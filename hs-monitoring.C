

//
// MAIN
//

bool hsmonitoring(){

  //Create file with all the ThresholdScan files to be analysed
  //gSystem->Exec("find ../Data/*_HS_Qualification_Test -name \"ThresholdScanResult*\" -print0 | sort -z | xargs -r0 | tr \" \" \"\n\" > hsfiles.dat");

  //Add a line to the file (for saving last HS)
  /*ofstream hsfile("hsfiles.dat", std::ios_base::app | std::ios_base::out);
  hsfile << "------------------------------------------------------------------------------------------------------------------------------------------------------------------------";
  hsfile.close();*/

  //Output file
  ofstream outfl("hsresults.dat");

  //Black list: HSs that are in the DB but have some problems: missing HIC QT, missing attachments (added manually below)
  string blacklist = "A-OL-HS-U-009, D-OL-HS-U-005, T-OL-HS-U-006, F-OL-HS-U-002, D-OL-HS-L-003, A-OL-HS-U-003, F-OL-HS-U-001, T-OL-HS-L-018, B-ML-HS-L-020, T-OL-HS-U-021, D-OL-HS-U-001, D-OL-HS-L-001, A-OL-HS-U-001, A-OL-HS-L-001, A-OL-HS-L-002, A-OL-HS-U-002, A-OL-HS-L-004, B-ML-HS-U-001, B-ML-HS-L-001, B-ML-HS-U-007, B-ML-HS-L-020";

  //Add old HS, HS without attachments by hand (from excel)
  const int nManual = 25;
  string shsmanual[nManual] = {
    "T-OL-HS-U-002 91 1/12/2017 48",
    "T-OL-HS-L-002 97 10/1/2018 2",
    "T-OL-HS-U-004 97 13/4/2018 15",
    "T-OL-HS-L-004 98 4/6/2018 23",
    "T-OL-HS-U-005 98 31/5/2018 22",
    "T-OL-HS-U-006 98 20/6/2018 25",
    "T-OL-HS-L-018 98 21/11/2018 47",
    "T-OL-HS-U-021 97 19/12/2018 51",
    "D-OL-HS-U-001 98 25/5/2018 21",
    "D-OL-HS-L-001 90 1/6/2018 22",
    "D-OL-HS-L-003 97 27/7/2018 30",
    "D-OL-HS-U-005 98 14/9/2018 37",
    "A-OL-HS-U-001 98 7/2/2018 6",
    "A-OL-HS-L-001 91 20/8/2018 34",
    "A-OL-HS-L-002 49 15/8/2018 33",
    "A-OL-HS-U-002 91 15/8/2018 33",
    "A-OL-HS-U-003 0 4/7/2018 27",
    "A-OL-HS-L-004 84 20/8/2018 34",
    "A-OL-HS-U-009 96 27/2/2019 9",
    "B-ML-HS-U-001 54 20/4/2018 16",
    "B-ML-HS-L-001 56 2/5/2018 18",
    "B-ML-HS-U-007 56 12/9/2018 37",
    "B-ML-HS-L-020 56 11/12/2018 50",
    "F-OL-HS-U-002 90 28/5/2018 22",
    "F-OL-HS-U-001 97 13/9/2018 37"
    };
  for(int i=0; i<nManual; i++)
    outfl<<shsmanual[i]<<endl;

  //Read file extracting the number of working chips HS by HS
  ifstream infl("hsfiles.dat");
  string date, activityname, hicid, hsidold;
  std::vector<string> hsid, qualdate;
  int nchipsokHS = 0;
  int nchipsokHIC = 0;
  int count = 0, count2 = 0;
  int nchipsmasked = 0;
  int nHICmissparam = 0;
  bool isMissingParam = false;
  while(infl>>activityname>>date>>nchipsokHIC){
   // if(fpath.find("3V") != std::string::npos) continue; //avoid results at VBB=-3V (they are in the list because of F-OL-HS-U-002
    hsid.push_back(ReadHSID(activityname));
    hicid = ReadHICID(activityname);
    if(blacklist.find(hsid.back())!=std::string::npos) {hsid.pop_back(); continue;} //exclude HS that are added manually for DB issues

    qualdate.push_back(date);
  
    if(hsid[hsid.size()-1]!=hsid[hsid.size()>1 ? hsid.size()-2 : hsid.size()-1]){
      //Get week number
      int day = GetDay(qualdate[qualdate.size()-2]);
      int month = GetMonth(qualdate[qualdate.size()-2]);
      int year = GetYear(qualdate[qualdate.size()-2]);
      int week = GetWeek(day, month, year);
      bool isWritableOL = kTRUE;
      bool isWritableML = kTRUE;
      string hsidwrite = hsid[hsid.size()-2];
      if(hsidwrite.substr(0,1) != "B" && count==7) isWritableOL=kTRUE;
      if(hsidwrite.substr(0,1) != "B" && (count<7 || count>7)) isWritableOL = kFALSE;
      if(hsidwrite.substr(0,1) == "B" && count==4) isWritableML=kTRUE;
      if(hsidwrite.substr(0,1) == "B" && (count<4 || count>4)) isWritableML=kFALSE;

      if(!isWritableML){
        if(count<4) errormsg(hsidwrite.c_str(),Form(": %d missing HIC(s) (not written) %s", 4-count, isMissingParam ? Form("--> No parameter for %d HIC(s)",nHICmissparam): "--> Missing info in DB"));
        if(count>4) errormsg(hsidwrite.c_str(),Form(": %d HIC(s) (not written)", count));
      }
      if(!isWritableOL){
        if(count<7) errormsg(hsidwrite.c_str(),Form(": %d missing HIC(s) (not written) %s",7-count, isMissingParam ? Form("--> No parameter for %d HIC(s)",nHICmissparam): "--> Missing info in DB"));
        if(count>7) errormsg(hsidwrite.c_str(),Form(": %d HIC(s) (not written)", count));
      }

      if(isWritableML && isWritableOL)
        outfl<<hsidwrite<<" "<<nchipsokHS+nchipsmasked<<" "<<day<<"/"<<month<<"/"<<year<<" "<<week<<endl; //write to file: <hsid> <#work chip> <qual date> <#week>
      count=0;
      nchipsokHS = 0;
      nchipsmasked = 0;
      isMissingParam = false;
      nHICmissparam=0;
    }
    //int nchipsokHIC = ReadThresholdFile(fpath);
    if(nchipsokHIC<0){
	count--;
	isMissingParam = true;
	nHICmissparam++;
    }
    nchipsokHS+=nchipsokHIC;
    count++;
  }
  outfl.close();
  infl.close();
  return 1;
}
