//
// MAIN
//

bool CheckStaveNotDg(){

  gStyle->SetOptStat(0000);

  TH1F *hsvstime[nSites], *cumHSvstime[nSites];//#HS vs time (all) and cumulative #HS vs time
  TH1F *hsvstime_detgrade[nSites], *cumHSvstime_detgrade[nSites];//#HS vs time (det grade) and cumulative #HS vs time (det. grade)
  TH1F *cumOLHSvstime_detgrade, *cumOLHSvstime;
  TH1F *yieldsites[nSites], *yieldOL;

  //Read file with the list of staves
  ifstream infl("staveresults.dat");
  string staveid, qualdate, siteid;
  int chipsokHSL, chipsokHSU, week, sitenum, categnum;
  vector <int> goodchhsl, goodchhsu;
  vector <string> stavenotdg;

  while(infl>>staveid>>chipsokHSU>>chipsokHSL>>qualdate>>week){
    siteid = staveid.substr(0,1);
    categnum = GetStaveCategoryId(chipsokHSU, chipsokHSL, staveid);
    //read stave file list to see if the HS is tested already in the stave setup
    ifstream inflstavefiles("stavefiles.dat");
    bool dg = IsStaveDetGrade(categnum, staveid);
    if(staveid=="T-OL-Stave-004") dg=kTRUE;
    if(!dg){
        stavenotdg.push_back(staveid);
        goodchhsl.push_back(chipsokHSL);
        goodchhsu.push_back(chipsokHSU);
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
  lat->DrawLatex(0.3,0.5,"Stave not DG");
  cIntro->Print("Results/Stave-HS_results.pdf");

  //List of non-DG Stave
  TCanvas *cStaveNoDg = new TCanvas("cStaveNoDg", "cStaveNoDg");
  TPaveText *ptStave = new TPaveText(.05,.1,.95,.8);
  ptStave->AddText("Staves not DG");
  ptStave->AddText(" ");
  for(int i=0; i<(int)stavenotdg.size(); i++){
    string siteid = stavenotdg[i].substr(0,1);
    ptStave->AddText(Form("%s: (U,L) = (%d, %d) bad chips",stavenotdg[i].c_str(), siteid=="B" ? 56-goodchhsu[i]:98-goodchhsu[i], siteid=="B" ? 56-goodchhsl[i]:98-goodchhsl[i]));
  }
  ptStave->Draw();
  cStaveNoDg->Print("Results/Stave-HS_results.pdf");
  cStaveNoDg->Print("Results/Stave-HS_results.pdf]");

  return 1;
}
