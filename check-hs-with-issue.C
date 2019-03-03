//
// MAIN
//

bool CheckHsWithIssue(){

  gStyle->SetOptStat(0000);

  TH1F *hsvstime[nSites], *cumHSvstime[nSites];//#HS vs time (all) and cumulative #HS vs time
  TH1F *hsvstime_detgrade[nSites], *cumHSvstime_detgrade[nSites];//#HS vs time (det grade) and cumulative #HS vs time (det. grade)
  TH1F *cumOLHSvstime_detgrade, *cumOLHSvstime;
  TH1F *yieldsites[nSites], *yieldOL;

  //Read file extracting the number of working chips HS by HS
  ifstream infl("hsresults.dat");
  string hsid, qualdate, siteid, stavefilepath;
  int chipsok, week, sitenum, categnum;
  vector <int> goodch;
  vector <string> hsnostave;

  while(infl>>hsid>>chipsok>>qualdate>>week){
    siteid = hsid.substr(0,1);
    sitenum = GetSiteNumber(siteid);
    categnum = GetHsCategoryId(chipsok, hsid);
    //read stave file list to see if the HS is tested already in the stave setup
    ifstream inflstavefiles("stavefiles.dat");
    int c=0;
    while(inflstavefiles>>stavefilepath){
      if(stavefilepath.find(hsid) != string::npos) {c++; break;}
    }
    if(!c){
        hsnostave.push_back(hsid);
        goodch.push_back(chipsok);
    }
  }
  infl.close();

  //////////////////////////////////////////////////
  /////////////// DRAW SECTION /////////////////////
  //////////////////////////////////////////////////

  //Separation slide
  TCanvas *cIntro = new TCanvas("cIntro", "cIntro");
  TLatex *lat = new TLatex();
  lat->SetNDC();
  lat->SetTextFont(42);
  lat->SetTextSize(0.08);
  lat->DrawLatex(0.25,0.5,"HS without a Stave");
  cIntro->Print("Results/Stave-HS_results.pdf");

  //HSs without a Stave
  TCanvas *cHSnostave = new TCanvas("cHSnostave", "cHSnostave");
  TPaveText *ptHS1 = new TPaveText(.0544,.0633,.455,.937);
  ptHS1->AddText("HSs (DG) not yet tested as Stave");
  ptHS1->AddText(" ");
  TPaveText *ptHS2 = new TPaveText(.515759,.063,.928,.937);
  ptHS2->AddText("HSs (non-DG) not yet tested as Stave");
  ptHS2->AddText(" ");
  for(int i=0; i<(int)hsnostave.size(); i++){
    bool dg = IsHSDetGrade(goodch[i], hsnostave[i]);
    string siteid = hsnostave[i].substr(0,1);
    if(dg) ptHS1->AddText(Form("%s: %d bad chips",hsnostave[i].c_str(), siteid=="B" ? 56-goodch[i] : 98-goodch[i]));
    else ptHS2->AddText(Form("%s: %d bad chips %s",hsnostave[i].c_str(), siteid=="B" ? 56-goodch[i] : 98-goodch[i], "-> #bf{rework(?)}"));
  }
  ptHS1->Draw();
  ptHS2->Draw();
  cHSnostave->Print("Results/Stave-HS_results.pdf");

  return 1;
}
