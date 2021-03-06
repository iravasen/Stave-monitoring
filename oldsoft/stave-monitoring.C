

//
// MAIN
//

bool stavemonitoring(){

  //Create file with all the ThresholdScan files to be analysed
  gSystem->Exec("find ../../Data/*_Stave_Qualification_Test -name \"*ThreshTuned_0V.dat\" -print0 | sort -z | xargs -r0 | tr \" \" \"\n\" > stavefiles.dat");

  //Add a line to the file (for saving last HS)
  ofstream stavefile("stavefiles.dat", std::ios_base::app | std::ios_base::out);
  stavefile << "------------------------------------------------------------------------------------------------------------------------------------------------------------------------";
  stavefile.close();

  //Output file
  ofstream outfl("staveresults.dat");

  //Black list: HSs that are in the DB but have some problems: missing HIC QT, missing attachments (added manually below)
  string blacklist = "B-ML-Stave-011, B-ML-Stave-013, B-ML-Stave-017, A-OL-Stave-001, A-OL-Stave-002, A-OL-Stave-003, A-OL-Stave-007, A-OL-Stave-009, D-OL-Stave-001, F-OL-Stave-001, F-OL-Stave-014";

  //Add old HS, HS without attachments by hand (from excel)
  const int nManual = 11;
  string shsmanual[nManual] = {
    "B-ML-Stave-011 B-ML-HS-U-011 B-ML-HS-L-011 56 56 12/11/2018 46",
    "B-ML-Stave-013 B-ML-HS-U-013 B-ML-HS-L-013 56 56 20/11/2018 47",
    "B-ML-Stave-017 B-ML-HS-U-017 B-ML-HS-L-017 56 56 12/12/2018 50",
    //"B-ML-Stave-024 B-ML-HS-U-124 B-ML-HS-L-024 56 56 31/1/2019 5",
    "A-OL-Stave-001 A-OL-HS-U-001 A-OL-HS-L-001 96 84 7/9/2018 36",
    "A-OL-Stave-002 A-OL-HS-U-002 A-OL-HS-L-002 91 49 7/9/2018 36",
    "A-OL-Stave-003 A-OL-HS-U-003 A-OL-HS-L-003 0 0 7/9/2018 36",
    "A-OL-Stave-007 A-OL-HS-U-007 A-OL-HS-L-008 98 98 3/12/2018 49",
   // "A-OL-Stave-008 A-OL-HS-U-008 A-OL-HS-L-009 98 97 26/11/2018 48",
    "A-OL-Stave-009 A-OL-HS-U-010 A-OL-HS-L-010 98 97 14/1/2019 3",
    "D-OL-Stave-001 D-OL-HS-U-001 D-OL-HS-L-001 98 76 9/8/2018 32",
    "F-OL-Stave-001 F-OL-HS-U-001 F-OL-HS-L-001 55 84 14/9/2018 37",
    "F-OL-Stave-014 F-OL-HS-U-014 F-OL-HS-L-014 98 98 8/2/2019 6"
    };
  for(int i=0; i<nManual; i++)
    outfl<<shsmanual[i]<<endl;

  //Read file extracting the number of working chips HS by HS
  ifstream infl("stavefiles.dat");
  string fpath, staveid, hsid, hicid, staveidold, qualdate, qualdateold;
  int nchipsokHSL = 0, nchipsokHSU = 0, nchipsmaskedHSU = 0, nchipsmaskedHSL = 0;
  int count = 0, countU = 0, countL = 0;
  int dayMax = -1, monthMax = -1, yearMax=-1;
  string hslid, hsuid;

  while(infl>>fpath){
    staveid = ReadStaveID(fpath);
    hsid = ReadHSID(fpath);
    hicid = ReadHICID(fpath);
    if(blacklist.find(staveid) != string::npos) continue; //exclude HS that are added manually for DB issues
    //Take the most recent data in case of multiple information
    string nextpath;
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
    }

    if(staveid!=staveidold){
      int week = GetWeek(dayMax, monthMax, 2000+yearMax);//week (most recent)

      bool isWritableOL = kTRUE;
      bool isWritableML = kTRUE;
      if(staveidold.substr(0,1) != "B" && (countL==7 && countU==7)) isWritableOL=kTRUE;
      if(staveidold.substr(0,1) != "B" && ((countL<7 || countU<7) || (countL>7 || countU>7))) isWritableOL = kFALSE;
      if(staveidold.substr(0,1) == "B" && (countL==4 && countU==4)) isWritableML=kTRUE;
      if(staveidold.substr(0,1) == "B" && ((countL<4 || countU<4) || (countL>4 || countU>4))) isWritableML=kFALSE;

      if(!isWritableML){
        if(countL<4) errormsg(staveidold.c_str(),Form(": %d missing HIC(s) for HS-Lower (not written)", 4-countL));
        if(countL>4) errormsg(staveidold.c_str(),Form(": %d HIC(s) for HS-Lower (not written)", countL));
        if(countU<4) errormsg(staveidold.c_str(),Form(": %d missing HIC(s) for HS-Upper (not written)", 4-countU));
        if(countU>4) errormsg(staveidold.c_str(),Form(": %d HIC(s) for HS-Upper (not written)", countU));
      }
      if(!isWritableOL){
        if(countL<7) errormsg(staveidold.c_str(),Form(": %d missing HIC(s) for HS-Lower (not written)",7-countL));
        if(countL>7) errormsg(staveidold.c_str(),Form(": %d HIC(s) for HS-Lower (not written)", countL));
        if(countU<7) errormsg(staveidold.c_str(),Form(": %d missing HIC(s) for HS-Upper (not written)", 7-countU));
        if(countU>7) errormsg(staveidold.c_str(),Form(": %d HIC(s) for HS-Upper (not written)", countU));
      }



      if(isWritableML && isWritableOL) //the two HSs must exist in the DB (both!)
        outfl<<staveidold<<" "<<hsuid<<" "<<hslid<<" "<<nchipsokHSU+nchipsmaskedHSU<<" "<<nchipsokHSL+nchipsmaskedHSL<<" "<<dayMax<<"/"<<monthMax<<"/"<<2000+yearMax<<" "<<week<<endl; //write to file: <hsid> <#work chip> <qual date> <#week>
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
    }

    //Qualification date
    qualdate = ReadQualDate(fpath);
    int day = GetDay(qualdate);
    int month = GetMonth(qualdate);
    int year = GetYear(qualdate);//year
    if(day>dayMax || month>monthMax || year>yearMax){//Get most recent date
      dayMax = day;
      monthMax = month;
      yearMax=year;
    }

    int nchipsokHIC = ReadThresholdFile(fpath);

    if(hsid.find("-U-")!=string::npos){//hs upper
      hsuid=hsid;
      nchipsokHSU+=nchipsokHIC;
      countU++;
      if(countU==20) nchipsmaskedHSU = GetMaskedFromConfig(fpath.substr(0, fpath.find("Threshold")-1),"Config_HS.cfg");//NOTE: no config file read out
    }
    else{//hs lower
      hslid=hsid;
      nchipsokHSL+=nchipsokHIC;
      countL++;
      if(countL==20) nchipsmaskedHSL = GetMaskedFromConfig(fpath.substr(0, fpath.find("Threshold")-1),"Config_HS.cfg");//NOTE: no config file read out
    }

  }
  outfl.close();
  infl.close();
  return 1;
}
