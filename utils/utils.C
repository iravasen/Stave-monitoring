//Global variables
const int nSites = 5; //production sites
const int color_site[nSites] = {923, 419, 801, 632, 857};
const string sitename[nSites] = {"Berkeley", "Nikhef", "Daresbury", "Frascati", "Turin"};

//Global variables for stave
const int nCatStave = 11; //categories for chips
int colorstv[nCatStave] = {858, 632, 801, 418, 807, 433, 617, 849, 602, 906, 420};
const TString labelstv[nCatStave] = {"0+0 dc", "1+0 dc", "1+1 dc", "2+0 dc", "2+1 dc", "2+2 dc", "3+0 dc", "3+1 dc", "3+2 dc", "3+3 dc", ">3+3 dc"};

//Global variables for hs
const int nCatHS = 9; //categories for chips
int colorhs[nCatHS] = {858, 632, 801, 419, 807, 433, 617, 815, 602};
const TString labelhs[nCatHS] = {"0 dc", "1 dc", "2 dc", "3 dc", "4 dc", "5 dc", "6 dc", "7 dc", ">7 dc"};

//Color code for output messages
#define errormsg(where,msg) ::Error(where, "%s%s%s",redTxt,msg,normalTxt)
#define infomsg(where,msg) ::Info(where,"%s%s%s",greenTxt,msg,normalTxt)
#define infomsgcolor(msg,color) Printf("%s%s%s",color,msg,normalTxt)
const char redTxt[]     =  { 0x1b, '[', '1', ';', '3', '1', 'm', 0 };
const char greenTxt[]   =  { 0x1b, '[', '1', ';', '3', '2', 'm', 0 };
const char normalTxt[]  =  { 0x1b, '[', '0', ';', '3', '9', 'm', 0 };
const char cyanTxt[]    =  { 0x1b, '[', '1', ';', '3', '6', 'm', 0 };

//Functions

//Read threshold file extracting the number of chips
int ReadThresholdFile(string fpath){

  int workchips;

  //Open file of the theshold scan and loop on it
  ifstream source;
  source.open(fpath, ios_base::in);
  for(string line2; getline(source, line2);){

    if(line2.find("Number of chips") != std::string::npos){
      istringstream inss(line2);      //make a stream for the line itself
      for(string type; inss >> type;){ //read stream "word" by "word", saving each "word" in the string
        if(type=="chips:")
          inss>>workchips;
      }
      break;
    }
  }

  return workchips;
}

//Read threshold file extracting the number of chips
int ReadWorkingChips(string fpath){

  int workchips;

  //Open file of the theshold scan and loop on it
  ifstream source;
  source.open(fpath, ios_base::in);
  for(string line2; getline(source, line2);){

    if(line2.find("Number of Working Chips") != std::string::npos){
      istringstream inss(line2);      //make a stream for the line itself
      for(string type; inss >> type;){ //read stream "word" by "word", saving each "word" in the string
        if(type=="value")
          inss>>workchips;
      }
      break;
    }
  }

  return workchips;
}

//Extract HS id from file path
string ReadHSID(string fpath){
  if(fpath.find("OBHIC") == string::npos) return "-------------";
  string hsid = fpath.substr(fpath.find("OBHIC")-14,13);
  return hsid;
}

//Extract Stave id from file path
string ReadStaveID(string fpath){
  string staveid = fpath.substr(fpath.find("Stave_Test")+11, 14);
  return staveid;
}

//Extract HIC id from file path
string ReadHICID(string fpath){
  if(fpath.find("OBHIC") == string::npos) return "-------------";
  string hicid = fpath.substr(fpath.find("OBHIC")+6, 8);
  return hicid;
}

//Extract HS qualification date
string ReadQualDate(string fpath){
  if(fpath.find("ThresholdScanResult") == string::npos) return "------";
  string date = fpath.substr(fpath.find("ThresholdScanResult")+20, 6);
  return date;
}

//Get week number from date
int GetWeek(struct tm* date){
	if (NULL == date)
		return 0; // or -1 or throw exception

	if (::mktime(date) < 0) // Make sure _USE_32BIT_TIME_T is NOT defined.
	  return 0; // or -1 or throw exception

	// The basic calculation:
	// {Day of Year (1 to 366) + 10 - Day of Week (Mon = 1 to Sun = 7)} / 7
	int monToSun = (date->tm_wday == 0) ? 7 : date->tm_wday; // Adjust zero indexed week day
	int week = ((date->tm_yday + 11 - monToSun) / 7); // Add 11 because yday is 0 to 365.
  // Now deal with special cases:
	// A) If calculated week is zero, then it is part of the last week of the previous year.
	if (week == 0){
		// We need to find out if there are 53 weeks in previous year.
		// Unfortunately to do so we have to call mktime again to get the information we require.
		// Here we can use a slight cheat - reuse this function!
		// (This won't end up in a loop, because there's no way week will be zero again with these values).
		tm lastDay = { 0,0,0,0,0,0,0,0,0,0,0};
		lastDay.tm_mday = 31;
		lastDay.tm_mon = 11;
		lastDay.tm_year = date->tm_year - 1;
		// We set time to sometime during the day (midday seems to make sense)
		// so that we don't get problems with daylight saving time.
		lastDay.tm_hour = 12;
		week = GetWeek(&lastDay);
	}
	// B) If calculated week is 53, then we need to determine if there really are 53 weeks in current year
	//    or if this is actually week one of the next year.
	else if (week == 53){
		// We need to find out if there really are 53 weeks in this year,
		// There must be 53 weeks in the year if:
		// a) it ends on Thurs (year also starts on Thurs, or Wed on leap year).
		// b) it ends on Friday and starts on Thurs (a leap year).
		// In order not to call mktime again, we can work this out from what we already know!
		int lastDay = date->tm_wday + 31 - date->tm_mday;
		if (lastDay == 5){ // Last day of the year is Friday
			// How many days in the year?
			int daysInYear = date->tm_yday + 32 - date->tm_mday; // add 32 because yday is 0 to 365
			if (daysInYear < 366){
				// If 365 days in year, then the year started on Friday
				// so there are only 52 weeks, and this is week one of next year.
				week = 1;
			}
		}
		else if (lastDay != 4){ // Last day is NOT Thursday
			// This must be the first week of next year
			week = 1;
		}
		// Otherwise we really have 53 weeks!
	}
	return week;
}

int GetWeek(int day, int month, int year){
	tm date = {0,0,0,0,0,0,0,0,0,0,0};
	date.tm_mday = day;
	date.tm_mon = month - 1;
	date.tm_year = year - 1900;
	// We set time to sometime during the day (midday seems to make sense)
	// so that we don't get problems with daylight saving time.
	date.tm_hour = 12;
	return GetWeek(&date);
}

//Get day from date YYMMDD
int GetDay(string date){
  if(date.find("-") != string::npos) return 0;
  string sday = date.substr(date.length()-2);
  return std::stoi(sday);
}

//Get Month from date YYMMDD
int GetMonth(string date){
  if(date.find("-") != string::npos) return 0;
  string smon = date.substr(2,2);
  return std::stoi(smon);
}

//Gey year from date YYMMDD
int GetYear(string date){
  if(date.find("-") != string::npos) return 0;
  string syr = date.substr(0,2);
  return std::stoi(syr);
}

//Get production site number
int GetSiteNumber(string siteid){
  int sitenum;
  if(siteid=="B") sitenum=0;
  else if(siteid=="A") sitenum=1;
  else if(siteid=="D") sitenum=2;
  else if(siteid=="F") sitenum=3;
  else sitenum=4; //Turin

  return sitenum;
}

//Get category id for an HS
int GetHsCategoryId(int chipsok, string hsid){
  int deadchips, catid;
  if(hsid.substr(0,1) == "B") deadchips=56-chipsok;//ML
  else deadchips=98-chipsok;

  for(int i=0; i<8; i++){
    if(deadchips==i) {catid=i; return catid;}
  }
  return 8;
}

//Get category id for an HS
int GetStaveCategoryId(int chipsokHSU, int chipsokHSL, string staveid){
  int dcHSU, dcHSL, catid;
  if(staveid.substr(0,1) == "B") {//ML
    dcHSU=56-chipsokHSU;
    dcHSL=56-chipsokHSL;
  }
  else{
    dcHSU=98-chipsokHSU;
    dcHSL=98-chipsokHSL;
  }

  if(dcHSU==0 && dcHSL==0) return 0;
  else if((dcHSU==0 && dcHSL==1) || (dcHSU==1 && dcHSL==0)) return 1;
  else if((dcHSU==1 && dcHSL==1)) return 2;
  else if((dcHSU==2 && dcHSL==0) || (dcHSU==0 && dcHSL==2)) return 3;
  else if((dcHSU==2 && dcHSL==1) || (dcHSU==1 && dcHSL==2)) return 4;
  else if((dcHSU==2 && dcHSL==2)) return 5;
  else if((dcHSU==3 && dcHSL==0) || (dcHSU==0 && dcHSL==3)) return 6;
  else if((dcHSU==3 && dcHSL==1) || (dcHSU==1 && dcHSL==3)) return 7;
  else if((dcHSU==3 && dcHSL==2) || (dcHSU==2 && dcHSL==3)) return 8;
  else if((dcHSU==3 && dcHSL==3)) return 9;
  else return 10;

}

//Get the number of working (but masked) chips from config file
int GetMaskedFromConfig(string path, string filename){

  int chipmasked=0;
  ifstream infl(Form("%s/%s", path.c_str(), filename.c_str()));
  string line;
  while(getline(infl,line)){
    if(line.find("ENABLED") != string::npos && line.find("#") == string::npos && line.find("ENABLEDBB") == string::npos){ //line is not commented
      string lastchar = line.substr(line.length()-1);
      if(lastchar=="0") chipmasked++; //if chip is masked, count it
    }
  }
  infl.close();
  return chipmasked;
}

//Get number of times the HIC is repeated into the file list
int GetRepetitions(string filename, string hicid){
  int n=0;
  if(hicid.find("AR000679")!= string::npos) return 1; //error in the DB is present for this HIC
  if(hicid.find("AR001154")!= string::npos) return 1; //error in the DB is present for this HIC

  ifstream file(filename.c_str());
  string line;
  while(file>>line){
    if(line.find(hicid) != string::npos) n++;
  }
  return n;
}

//Set style of a TPie
void SetStylePie(TPie *pie){
  pie->SetLabelFormat("[%txt] %val (%perc)");
  pie->SetValueFormat("%.0f");
  pie->SetCircle(0.5,0.5,0.27);
  pie->SetAngle3D(70);
  pie->SetTextSize(0.04);
}

//Determine if HS is det grade
bool IsHSDetGrade(int chipsok, string hsid){
  if(hsid.substr(0,1) == "B" && 56-chipsok<2){
    return kTRUE;
  }
  else if(hsid.substr(0,1) != "B" && 98-chipsok<3) return kTRUE;
  else return kFALSE;
}

//Determine if Stave is det grade
bool IsStaveDetGrade(int categnum, string staveid){
  if(staveid.substr(0,1) == "B" && categnum<2){
    return kTRUE;
  }
  else if(staveid.substr(0,1) != "B" && categnum<6) return kTRUE;
  else return kFALSE;
}

//Set style for histo vs time
void SetStyleSite(TH1F *h, int col){
  h->SetMarkerStyle(1);
  h->SetMarkerColor(col);
  h->SetLineColor(col);
  h->SetLineWidth(2);
}

//Set Legend Style
void SetLegendStyle(TLegend *leg){
  leg->SetLineColor(kWhite);
  leg->SetTextFont(42);
}

//Set x axis labels for Stave
void SetLabels(TH1F *h, int year){
  for(int i=1; i<=h->GetNbinsX(); i++){
    TString label = i>52 ? Form("%d", i-52) : Form("%d",i);
    h->GetXaxis()->SetBinLabel(i, label.Data());
  }
}

//Set x axis labels for Stave
void SetLabelsHS(TH1F *h, int year){
  for(int i=1; i<=h->GetNbinsX(); i++){
    TString label = i>54 ? Form("%d", i-54) : i<3 ? Form("%d",i+46):Form("%d",i-2);
    h->GetXaxis()->SetBinLabel(i, label.Data());
  }
}
