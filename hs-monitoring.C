

//
// MAIN
//

bool hsmonitoring(){

  //Create file with all the ThresholdScan files to be analysed
  gSystem->Exec("find Data/*_HS_Qualification_Test -name \"ThresholdScanResult*\" -print0 | sort -z | xargs -r0 | tr \" \" \"\n\" > hsfiles.dat");

  //Add a line to the file (for saving last HS)
  ofstream hsfile("hsfiles.dat", std::ios_base::app | std::ios_base::out);
  hsfile << "------------------------------------------------------------------------------------------------------------------------------------------------------------------------";
  hsfile.close();

  //Output file
  ofstream outfl("hsresults.dat");

  //Black list: HSs that are in the DB but have some problems: missing HIC QT, missing attachments (added manually below)
  string blacklist = "A-OL-HS-U-009, D-OL-HS-L-004, D-OL-HS-L-003, A-OL-HS-U-003, F-OL-HS-L-010, F-OL-HS-U-001, T-OL-HS-L-018, D-OL-HS-U-008, B-ML-HS-L-020, T-OL-HS-U-021, D-OL-HS-U-001, D-OL-HS-L-001, D-OL-HS-U-008, A-OL-HS-U-001, A-OL-HS-L-001, A-OL-HS-L-002, A-OL-HS-U-002, A-OL-HS-L-004, A-OL-HS-U-008, B-ML-HS-U-001, B-ML-HS-L-001, B-ML-HS-U-007, B-ML-HS-L-020, F-OL-HS-L-010, F-OL-HS-U-001";

  //Add old HS, HS without attachments by hand (from excel)
  const int nManual = 26;
  string shsmanual[nManual] = {
    "T-OL-HS-U-002 91 1/12/2017 48",
    "T-OL-HS-L-002 97 10/1/2018 2",
    "T-OL-HS-U-004 97 13/4/2018 15",
    "T-OL-HS-L-004 98 4/6/2018 23",
    "T-OL-HS-U-005 98 31/5/2018 22",
    "T-OL-HS-L-018 98 21/11/2018 47",
    "T-OL-HS-U-021 97 19/12/2018 51",
    "D-OL-HS-U-001 98 25/5/2018 21",
    "D-OL-HS-L-001 90 1/6/2018 22",
    "D-OL-HS-L-003 97 27/7/2018 30",
    "D-OL-HS-L-004 98 7/9/2018 36",
    "D-OL-HS-U-008 98 5/11/2018 45",
    "A-OL-HS-U-001 98 7/2/2018 6",
    "A-OL-HS-L-001 91 20/8/2018 34",
    "A-OL-HS-L-002 49 15/8/2018 33",
    "A-OL-HS-U-002 91 15/8/2018 33",
    "A-OL-HS-U-003 0 4/7/2018 27",
    "A-OL-HS-L-004 84 20/8/2018 34",
    "A-OL-HS-U-008 98 9/11/2018 45",
    "A-OL-HS-U-009 98 27/11/2018 48",
    "B-ML-HS-U-001 54 20/4/2018 16",
    "B-ML-HS-L-001 56 2/5/2018 18",
    "B-ML-HS-U-007 56 12/9/2018 37",
    "B-ML-HS-L-020 56 11/12/2018 50",
    "F-OL-HS-L-010 98 14/11/2018 46",
    "F-OL-HS-U-001 97 13/09/2018 37"
    };
  for(int i=0; i<nManual; i++)
    outfl<<shsmanual[i]<<endl;

  //Read file extracting the number of working chips HS by HS
  ifstream infl("hsfiles.dat");
  string fpath, hsid, hsidold, qualdate, qualdateold;
  int nchipsokHS = 0;
  int count = 0;
  int nchipsmasked = 0;
  while(infl>>fpath){
    if(fpath.find("3V") != std::string::npos) continue; //avoid results at VBB=-3V (they are in the list because of F-OL-HS-U-002
    hsid = ReadHSID(fpath);
    if(blacklist.find(hsid)!=std::string::npos) continue; //exclude HS that are added manually for DB issues
    qualdate = ReadQualDate(fpath);
    count++;
    if(count==1){
      hsidold=hsid;
      qualdateold=qualdate;
      nchipsmasked = GetMaskedFromConfig(fpath.substr(0, fpath.find("Threshold")-1),"Config_HS.cfg");
      if(hsid.substr(0,1) != "F") nchipsmasked = 0; //NOTE for Frascati chip masked even if broken (ask!!)
    }
    if(hsid!=hsidold){
      //Get week number
      int day = GetDay(qualdateold);
      int month = GetMonth(qualdateold);
      int year = GetYear(qualdateold);
      int week = GetWeek(day, month, 2000+year);
      bool isWritableOL = kTRUE;
      bool isWritableML = kTRUE;
      if(hsidold.substr(0,1) != "B" && count==7) isWritableOL=kTRUE;
      if(hsidold.substr(0,1) != "B" && count<7) isWritableOL = kFALSE;
      if(hsidold.substr(0,1) == "B" && count==4) isWritableML=kTRUE;
      if(hsidold.substr(0,1) == "B" && count<4) isWritableML=kFALSE;

      if(!isWritableML){
        if(count<4) errormsg(hsidold.c_str(),Form(": %d missing HIC(s) (not written)", 4-count));
      }
      if(!isWritableOL){
        if(count<7) errormsg(hsidold.c_str(),Form(": %d missing HIC(s) (not written)",7-count));
      }

      if(isWritableML && isWritableOL)
        outfl<<hsidold<<" "<<nchipsokHS+nchipsmasked<<" "<<day<<"/"<<month<<"/"<<2000+year<<" "<<week<<endl; //write to file: <hsid> <#work chip> <qual date> <#week>
      count=0;
      nchipsokHS = 0;
      nchipsmasked = 0;
    }
    int nchipsokHIC = ReadThresholdFile(fpath);
    nchipsokHS+=nchipsokHIC;
  }
  outfl.close();
  infl.close();
  return 1;
}
