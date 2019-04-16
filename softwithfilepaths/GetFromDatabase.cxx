#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include "../new-alpide-software/DataBaseSrc/DBHelpers.h"
#include "../new-alpide-software/DataBaseSrc/AlpideDB.h"
#include "../new-alpide-software/DataBaseSrc/AlpideDBEndPoints.h"
#include "paths/pathstaverec.h"
#include "paths/pathstaveqt.h"

using namespace std;
using namespace std::chrono;

//Function to replace white spaces in a string with _
std::string space2underscore(std::string text) {
    for(std::string::iterator it = text.begin(); it != text.end(); ++it) {
        if(*it == ' ') {
            *it = '_';
        }
    }
    return text;
}

//Function that checks if file exist in /eos/...
int IsFileExisting(string path){

	if(path.find("Threshold")==string::npos) return -1;
	FILE *filein;
	filein = fopen (path.c_str(), "r");
	if(filein!=NULL){
		fclose(filein);
		return 1;
	}
	return 0;
}

//Bubble sort for strings
void bubbleSort(std::vector<string> &arr1, std::vector<string> &arr2, int n) 
{  
    for (int j=0; j<n-1; j++) 
    { 
    	for (int i=j+1; i<n; i++) 
    	{ 
    		if (arr1[j]<arr1[i]) 
    		{ 
    			std::swap(arr1[i], arr1[j]);
			std::swap(arr2[i], arr2[j]);
                } 
        } 
    } 
}
 
//Function to write vector into file
void WriteToFile(std::vector<string> name, std::vector<string> path, string fname){
	
	ofstream fileout (fname.c_str());
	for(int i=0; i<(int)name.size(); i++){
		fileout<<name[i]<<" "<<path[i]<<endl;
	}
	fileout<<"-------------------------------------------------------------------------- ------------------------------------------------------------------------------------"<<endl;
	fileout.close();
}
  

	

void GetFromDatabase(){

        THicType hicType = HIC_OB;

	//vector to contain activity name and file paths 
	std::vector<string> fpathHS, fpathStave, fpathStaveCern, hsAname, staveAname, staveAnamecern;
	//vector for string editing
	std::vector<string> testcity = {"TestB","TestT","TestA", "TestD", "TestF"};
	std::vector<string> testcityund = {"Test_B","Test_T","Test_A", "Test_D", "Test_F"};

	//Initialize DB
        AlpideDB *db;
        db = new AlpideDB(0);
	
	//Get list of HICs into the DB
	int componentTypeId;
        componentTypeId = DbGetComponentTypeId (db, "Outer Barrel HIC Module");
 	ComponentDB *componentDB = new ComponentDB(db);
        std::vector<ComponentDB::componentShort> componentList;//list
        componentDB->GetListByType(db->GetProjectId(), componentTypeId, &componentList);

	//Loop on all components
	std::vector<ComponentDB::compActivity> tests;
	std::vector<ComponentDB::componentShort>::iterator iComp;
	for (iComp = componentList.begin(); iComp != componentList.end(); iComp++) {
		ComponentDB::componentShort comp = *iComp;
	        DbGetAllTests (db, comp.ID, tests, STThreshold, true);
	

		//Loop on all activities
		std::vector<ComponentDB::compActivity>::iterator it;
		ActivityDB *activityDB = new ActivityDB(db);
		for(it = tests.begin(); it != tests.end(); it++) {
	        	ComponentDB::compActivity act = *it;
			ActivityDB::activityLong actLong;
			activityDB->Read(act.ID, &actLong);

			//EOS path
			string eosPathSingle = GetEosPath(actLong, hicType, false);
       			string eosPathDouble = GetEosPath(actLong, hicType, true);//for old HICs where HIC name appear twice

			string dataName, resultName;	
			GetThresholdFileName(actLong, 0, false, 0, dataName, resultName);//take tuned thr scan at 0V BB
			
			//Get the paramenter "Number of Working Chips"
			float nworkch;
			DbFindParamValue(actLong.Parameters, "Number of Working Chips", nworkch);

			//Get start date 
			char qualdatehuman[20];
			time_t qualdatetimeunix = actLong.StartDate;
			strftime(qualdatehuman, 20, "%d/%m/%Y %H:%M:%S", localtime(&qualdatetimeunix));
			cout<<actLong.Name<<"  "<<qualdatehuman<<endl;
			/*std::tm* t = std::gmtime(&qualdatetime);
			std::stringstream ss; // or if you're going to print, just input directly into the output stream
			ss << std::put_time(t, "%d/%m/%Y %I:%M:%S %p");
			string output = ss.str();
			cout<<actLong.Name<<"  "<<output<<endl;*/
			
			
			string fpathSingle = eosPathSingle + "/" + resultName;
			string fpathDouble = eosPathDouble + "/" + resultName;
		         
			//Check which file really exist in eos
			int fexistSingle = IsFileExisting(fpathSingle);
			string fpathreal; 
				
			if(fexistSingle==-1) continue;
			else if(fexistSingle) fpathreal=fpathSingle;
			else fpathreal=fpathDouble;

			//Remove a strange [1558583984] number from activity name, remove "Retest", replace Test<city> with Test_city-> problem for alphabetic order
			if(actLong.Name.find("[") != string::npos)
				actLong.Name.replace(actLong.Name.find("[")-1, 13, "");
			if(actLong.Name.find("Retest") != string::npos)
				actLong.Name.replace(actLong.Name.find("Retest")-1, 9, "");
			for(int i=0; i<(int)testcity.size(); i++)
				if(actLong.Name.find(testcity[i].c_str()) != string::npos)
					actLong.Name.replace(actLong.Name.find(testcity[i].c_str()), 5, testcityund[i].c_str());
			

			//Save file paths and activity names into vector (alphabetically unordered here!)
			if(actLong.Type.Name.find("Stave") == string::npos) {fpathHS.push_back(fpathreal); hsAname.push_back(space2underscore(actLong.Name));}
                        if(actLong.Type.Name.find("Reception") == string::npos && actLong.Type.Name.find("HS Qualification")==string::npos){ 
				fpathStave.push_back(fpathreal); 
				staveAname.push_back(space2underscore(actLong.Name));
			}
                        if(actLong.Type.Name.find("Reception") != string::npos) {fpathStaveCern.push_back(fpathreal); staveAnamecern.push_back(space2underscore(actLong.Name));}
		}

		delete activityDB;

	}

	//Add paths manually
	for(int i=0; i<(int)pathStaveRec.size(); i++){
		fpathStaveCern.push_back(pathStaveRec[i].substr(pathStaveRec[i].find("/eos"),pathStaveRec[i].size()));
		staveAnamecern.push_back(pathStaveRec[i].substr(0, pathStaveRec[i].find("/eos")-1));
	}
	for(int i=0; i<(int)pathStaveQt.size(); i++){
                fpathStave.push_back(pathStaveQt[i].substr(pathStaveQt[i].find("/eos"),pathStaveQt[i].size()));
                staveAname.push_back(pathStaveQt[i].substr(0, pathStaveQt[i].find("/eos")-1));
        }

		


	

	//Alphabetical ordering of the vector 
	bubbleSort(hsAname, fpathHS, (int)hsAname.size());
	bubbleSort(staveAname, fpathStave, (int)staveAname.size());
	bubbleSort(staveAnamecern, fpathStaveCern, (int)staveAnamecern.size());

	//Write vector into file (alphabetically ordered!!)
	WriteToFile(hsAname, fpathHS, "hsfiles.dat");
	WriteToFile(staveAname, fpathStave, "stavefiles.dat");
	WriteToFile(staveAnamecern, fpathStaveCern, "stavefiles_rec.dat");
		
	
}

int main(){
	GetFromDatabase();
       	return 0;
}
