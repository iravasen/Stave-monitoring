#include <TSystem.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TF1.h>
#include <TH1.h>
#include <TFile.h>
#include <TGraphAsymmErrors.h>
#include <TH2F.h>
#include <TStyle.h>
#include <TPaveStats.h>
#include <TError.h>
#include <sstream>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <TPie.h>
#include <TLatex.h>
#include <TLine.h>

#include "utils/utils.C"
#include "hs-monitoring.C"
#include "hs-analysis.C"
#include "stave-monitoring.C"
#include "stave-analysis.C"
#include "stave-rec-monitoring.C"
#include "stave-rec-analysis.C"
#include "check-hs-with-issue.C"
#include "check-stave-not-dg.C"

void StartMonitoring(int year, int thisweek, string todaydate = "DD/MM/YYYY"){

  //First slide
  TCanvas *c1st = new TCanvas("c1st","c1st");
  TLatex *latex = new TLatex();
  latex->SetNDC();
  latex->SetTextFont(42);
  latex->SetTextSize(0.08);
  latex->DrawLatex(0.2,0.8,"Stave production monitoring"); //title
  latex->SetTextSize(0.055);
  latex->DrawLatex(0.12, 0.7, "#bf{Ivan Ravasenga}, #it{Bogolyubov Institute for Theo. Phys.}"); //author
  latex->DrawLatex(0.4,0.4,todaydate.c_str());
  latex->DrawLatex(0.15, 0.2,Form("Monitoring from January 2018 to %s", todaydate.c_str()));
  latex->DrawLatex(0.375,0.1,"Stave meeting");
  c1st->Print("Results/Stave-HS_results.pdf[");
  c1st->Print("Results/Stave-HS_results.pdf");

  cout<<"Writing HS results to file..."<<endl;
  hsmonitoring();
  cout<<"Monitoring HS results..."<<endl;
  hsanalysis(year, thisweek);
  cout<<"Writing Stave results (QT) to file..."<<endl;
  stavemonitoring();
  cout<<"Monitoring Stave results (QT)..."<<endl;
  staveanalysis(year, thisweek);
  cout<<"Writing Stave results (RT) to file..."<<endl;
  staverecmonitoring();
  cout<<"Monitoring Stave results (RT)..."<<endl;
  staverecanalysis(year, thisweek);
  cout<<"Preparing list of reworkable / without-stave HS"<<endl;
  CheckHsWithIssue();
  cout<<"Preparing list of non-DG Staves at Stave site"<<endl;
  CheckStaveNotDg();

}
