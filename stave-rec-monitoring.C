

//
// MAIN
//

bool staverecmonitoring(){

  //Output file
  ofstream outfl("staveresults_rec.dat");
  
  //Black list: HSs that are in the DB but have some problems: missing HIC QT, missing attachments (added manually below)
  string blacklist = "A-OL-Stave-008, F-OL-Stave-008, B-ML-Stave-003, B-ML-Stave-010";
  //Add old HS, HS without attachments by hand (from excel)
  const int nManual = 4;
  string shsmanual[nManual] = {
  	"F-OL-Stave-008 98 98 30/11/2018 48",
        "A-OL-Stave-008 98 97 19/2/2019 8",
	"B-ML-Stave-003 56 56 3/12/2018 49",
	"B-ML-Stave-010 56 56 28/11/2018 48"        
  };
  for(int i=0; i<nManual; i++)
    outfl<<shsmanual[i]<<endl;
  
  //Read file extracting the number of working chips HS by HS
  ifstream infl("stavefiles_rec.dat");
  
  string fpath, activityname, hsid, hicid, staveidold, date;
  std::vector<string> staveid, qualdate;
  int nchipsokHSL = 0, nchipsokHSU = 0, nchipsmaskedHSU = 0, nchipsmaskedHSL = 0, nchipsokHIC = 0;
  int count = 0, countU = 0, countL = 0, countUmiss = 0, countLmiss = 0;
  int dayMax = -1, monthMax = -1, yearMax=-1;
  bool isMissingparamU = false, isMissingparamL = false;
  string hslid, hsuid;

  while(infl>>activityname>>date>>nchipsokHIC){
    staveid.push_back(ReadStaveID(activityname));
    hsid = ReadHSID(activityname);
    hicid = ReadHICID(activityname);
    if(blacklist.find(staveid.back()) != string::npos) {staveid.pop_back(); continue;} //exclude HS that are added manually for DB issues
	
    //Qualification date
    qualdate.push_back(date);
    
    //Take the most recent data in case of multiple information

    if(staveid[staveid.size()-1] != staveid[staveid.size()>1 ? staveid.size()-2 : staveid.size()-1]){

      //Get most recent qualification date
      for(int i=qualdate.size()-2; i>=(int)qualdate.size()-countL-countU-1; i--){
      	int day = GetDay(qualdate[i]);
      	int month = GetMonth(qualdate[i]);
      	int year = GetYear(qualdate[i]);
      	if(day>dayMax || month>monthMax || year>yearMax){//Get most recent date
      		dayMax = day;
       		monthMax = month;
       		yearMax=year;                    
      	}
      }

      int week = GetWeek(dayMax, monthMax, yearMax);//week (most recent)
      string stavewrite = staveid[staveid.size()-2];

      bool isWritableOL = kTRUE;
      bool isWritableML = kTRUE;
      if(stavewrite.substr(0,1) != "B" && (countL==7 && countU==7)) isWritableOL=kTRUE;
      if(stavewrite.substr(0,1) != "B" && ((countL<7 || countU<7) || (countL>7 || countU>7))) isWritableOL = kFALSE;
      if(stavewrite.substr(0,1) == "B" && (countL==4 && countU==4)) isWritableML=kTRUE;
      if(stavewrite.substr(0,1) == "B" && ((countL<4 || countU<4) || (countL>4 || countU>4))) isWritableML=kFALSE;

      if(!isWritableML){
        if(countL<4) errormsg(stavewrite.c_str(),Form(": %d missing HIC(s) for HS-Lower (not written) %s", 4-countL, isMissingparamL ? Form("--> No parameter for %d HIC(s)",countLmiss): "--> Missing info in DB" ));
        if(countL>4) errormsg(stavewrite.c_str(),Form(": %d HIC(s) for HS-Lower (not written)", countL));
        if(countU<4) errormsg(stavewrite.c_str(),Form(": %d missing HIC(s) for HS-Upper (not written) %s", 4-countU, isMissingparamU ? Form("--> No parameter for %d HIC(s)",countUmiss): "--> Missing info in DB" ));
        if(countU>4) errormsg(stavewrite.c_str(),Form(": %d HIC(s) for HS-Upper (not written)", countU));
      }
      if(!isWritableOL){
        if(countL<7) errormsg(stavewrite.c_str(),Form(": %d missing HIC(s) for HS-Lower (not written) %s",7-countL, isMissingparamL ? Form("--> No parameter for %d HIC(s)",countLmiss): "--> Missing info in DB" ));
        if(countL>7) errormsg(stavewrite.c_str(),Form(": %d HIC(s) for HS-Lower (not written)", countL));
        if(countU<7) errormsg(stavewrite.c_str(),Form(": %d missing HIC(s) for HS-Upper (not written) %s", 7-countU, isMissingparamU ? Form("--> No parameter for %d HIC(s)",countUmiss): "--> Missing info in DB" ));
        if(countU>7) errormsg(stavewrite.c_str(),Form(": %d HIC(s) for HS-Upper (not written)", countU));
      }



      if(isWritableML && isWritableOL) //the two HSs must exist in the DB (both!)
        outfl<<stavewrite<<" "<<nchipsokHSU+nchipsmaskedHSU<<" "<<nchipsokHSL+nchipsmaskedHSL<<" "<<dayMax<<"/"<<monthMax<<"/"<<yearMax<<" "<<week<<endl; //write to file: <hsid> <#work chip> <qual date> <#week>
      count=0;
      countU = 0;
      countL = 0;
      nchipsokHSU = 0;
      nchipsokHSL = 0;
      nchipsmaskedHSL = 0;
      nchipsmaskedHSU = 0;
      dayMax = -1;
      monthMax = -1;
      yearMax=-1;
      countUmiss=0;
      countLmiss=0;
      isMissingparamL=false;
      isMissingparamU=false;
    }

    //int nchipsokHIC = ReadThresholdFile(fpath);

    if(hsid.find("-U-")!=string::npos){//hs upper
      hsuid=hsid;
      if(nchipsokHIC<0){
	countU--;
	isMissingparamU=true;
	countUmiss++;
      }
      nchipsokHSU+=nchipsokHIC;
      countU++;
      if(countU==20) nchipsmaskedHSU = GetMaskedFromConfig(fpath.substr(0, fpath.find("Threshold")-1),"Config_HS.cfg");//NOTE: no config file read out
    }
    else{//hs lower
      hslid=hsid;
      if(nchipsokHIC<0){
        countL--;
        isMissingparamL=true;
        countLmiss++;
      }
      nchipsokHSL+=nchipsokHIC;
      countL++;
      if(countL==20) nchipsmaskedHSL = GetMaskedFromConfig(fpath.substr(0, fpath.find("Threshold")-1),"Config_HS.cfg");//NOTE: no config file read out
    }

  }
  outfl.close();
  infl.close();
  return 1;
}
