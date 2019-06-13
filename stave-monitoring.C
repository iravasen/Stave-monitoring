

//
// MAIN
//

bool stavemonitoring(){

  //Output file
  ofstream outfl("staveresults.dat");

  //Black list: HSs that are in the DB but have some problems: missing HIC QT, missing attachments (added manually below)
  string blacklist = "B-ML-Stave-039, T-OL-Stave-002, D-OL-Stave-008, T-OL-Stave-006, B-ML-Stave-011, A-OL-Stave-001, A-OL-Stave-002, A-OL-Stave-003, A-OL-Stave-007, A-OL-Stave-011, F-OL-Stave-001";

  //Add old HS, HS without attachments by hand (from excel)
  const int nManual = 9;
  string shsmanual[nManual] = {
    "D-OL-Stave-008 D-OL-HS-U-008 D-OL-HS-L-008 98 84 17/5/2019 20",
    "B-ML-Stave-011 B-ML-HS-U-011 B-ML-HS-L-011 56 56 12/11/2018 46",
    "B-ML-Stave-039 B-ML-HS-U-039 B-ML-HS-L-039 56 54 14/5/2019 20"
    "A-OL-Stave-001 A-OL-HS-U-001 A-OL-HS-L-001 96 84 7/9/2018 36",
    "A-OL-Stave-002 A-OL-HS-U-002 A-OL-HS-L-002 91 49 7/9/2018 36",
    "A-OL-Stave-003 A-OL-HS-U-003 A-OL-HS-L-003 98 70 7/9/2018 36",
    "A-OL-Stave-007 A-OL-HS-U-007 A-OL-HS-L-008 98 98 3/12/2018 49",
    "A-OL-Stave-011 A-OL-HS-U-013 A-OL-HS-L-014 96 98 1/4/2019 14",
    "F-OL-Stave-001 F-OL-HS-U-001 F-OL-HS-L-001 55 84 14/9/2018 37",
    "T-OL-Stave-006 T-OL-HS-U-006 T-OL-HS-L-006 98 98 23/7/2018 30"
  };

  for(int i=0; i<nManual; i++)
    outfl<<shsmanual[i]<<endl;

  //Read file extracting the number of working chips HS by HS
  ifstream infl("stavefiles.dat");
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
    /*string nextpath;
    if(fpath.find("---------")==string::npos){//if not at the end of the file
      int nrep = GetRepetitions("stavefiles.dat", hicid);
      for(int irep=0; irep<nrep-1; irep++)
        infl>>nextpath;
      if(nrep>1)
        fpath=nextpath;
    }

    count++;
    if(count==1){
      staveidold=staveid;
    }*/

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
        outfl<<stavewrite<<" "<<hsuid<<" "<<hslid<<" "<<nchipsokHSU+nchipsmaskedHSU<<" "<<nchipsokHSL+nchipsmaskedHSL<<" "<<dayMax<<"/"<<monthMax<<"/"<<yearMax<<" "<<week<<endl; //write to file: <hsid> <#work chip> <qual date> <#week>
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
