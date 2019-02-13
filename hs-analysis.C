//
// MAIN
//

bool hsanalysis(int year, int thisweek){

  gStyle->SetOptStat(0000);

  TH1F *hsvstime[nSites], *cumHSvstime[nSites];//#HS vs time (all) and cumulative #HS vs time
  TH1F *hsvstime_detgrade[nSites], *cumHSvstime_detgrade[nSites];//#HS vs time (det grade) and cumulative #HS vs time (det. grade)
  TH1F *cumOLHSvstime_detgrade, *cumOLHSvstime;
  TH1F *yieldsites[nSites], *yieldOL;

  //number of bins and upper edge of plots vs time
  int nbins;
  double upperedge;
  if(year==2019) {
    nbins=52+2+thisweek;//+2 is to include two weeks of 2017
    upperedge = 52 + 2 +(double)thisweek+0.5;//+2 is to include two weeks of 2017
  }
  else{
    nbins=thisweek+2;//+2 is to include two weeks of 2017
    upperedge=52 + 2 + (double)thisweek+0.5;//+2 is to include two weeks of 2017
  }

  for(int i=0; i<nSites; i++){
    hsvstime[i] = new TH1F(Form("%s_HS_vs_time", sitename[i].c_str()), Form("%s - HS vs time", sitename[i].c_str()), nbins, 0.5, upperedge);
    hsvstime_detgrade[i] = new TH1F(Form("%s_HS_vs_time_dg", sitename[i].c_str()), Form("%s - HS vs time (det. grade)", sitename[i].c_str()), nbins, 0.5, upperedge);
  }
  //Read file extracting the number of working chips HS by HS
  ifstream infl("hsresults.dat");
  string hsid, qualdate, siteid;
  int chipsok, week, sitenum, categnum, hsyear;
  vector <string> thisweekHS;

  double deadchips[nSites][nCatHS];
  double deadchipsOL[nCatHS] = {0.};
  for(int i=0; i<nSites; i++)
    for(int j=0; j<nCatHS; j++)
      deadchips[i][j]=0.;

  while(infl>>hsid>>chipsok>>qualdate>>week){
    siteid = hsid.substr(0,1);
    sitenum = GetSiteNumber(siteid);
    categnum = GetHsCategoryId(chipsok, hsid);
    hsyear = stoi(qualdate.substr(qualdate.length()-4));

    deadchips[sitenum][categnum]++;//dead chips site by site
    if(sitenum!=0) deadchipsOL[categnum]++;
    hsvstime[sitenum]->Fill(hsyear==2019 ? (double)week+54. : hsyear==2017 ? (double)week-46 : (double)week+2);//all hs vs time
    bool detgrade = IsHSDetGrade(chipsok, hsid);
    if(detgrade) hsvstime_detgrade[sitenum]->Fill(hsyear==2019 ? (double)week+54. : hsyear==2017 ? (double)week-46 : (double)week+2);//det. grade hs vs time
    if(week==thisweek && hsyear==year) thisweekHS.push_back(hsid);
  }
  infl.close();

  //Calculate the present yield (not vs time!)
  vector <double> yieldDetGrade;
  double yieldDetGradeOL;
  double sum = 0., num=0.;
  for(int i=0; i<nSites; i++){
    sum = 0.;
    num=0.;
    for(int j=0; j<nCatHS; j++){
      sum+=deadchips[i][j];
      if(i==0 && j<2) num+=deadchips[i][j];
      if(i!=0 && j<3) num+=deadchips[i][j];
    }
    yieldDetGrade.push_back(num/sum*100.);
  }
  sum=0.;
  num=0.;
  for(int i=0;i<nCatHS; i++){
    sum+=deadchipsOL[i];
    if(i<3) num+=deadchipsOL[i];
  }
  yieldDetGradeOL=num/sum*100.;

  //Remove categories with 0 counts (bad labels in TPie)
  vector <vector<double>> finaldeadch;
  vector <vector <TString>> finallab;
  vector <vector <int>> finalcol;
  vector <int> row2, colOL;
  vector <double> finaldeadchipsOL, row1;
  vector <TString> finallabOL, row3;
  for(int i=0; i<nSites; i++){
    for(int j=0; j<nCatHS; j++){
      if(deadchips[i][j]>0.5){
        row1.push_back(deadchips[i][j]);
        row2.push_back(colorhs[j]);
        row3.push_back(labelhs[j]);
      }
      if(j==nCatHS-1){
        finaldeadch.push_back(row1);
        finalcol.push_back(row2);
        finallab.push_back(row3);
        row1.clear();
        row2.clear();
        row3.clear();
      }
    }
  }
  for(int i=0; i<nCatHS; i++){
    if(deadchipsOL[i]>0.5){
      finaldeadchipsOL.push_back(deadchipsOL[i]);
      colOL.push_back(colorhs[i]);
      finallabOL.push_back(labelhs[i]);
    }
  }

  //Make cumulative HS vs time
  double count1=0., count2=0.;
  int limit = year==2019 ? 52+2+thisweek : thisweek+2;
  for(int isite=0; isite<nSites; isite++){
    cumHSvstime[isite] = (TH1F*)hsvstime[isite]->Clone(Form("cumHSvstime_%s", sitename[isite].c_str()));
    cumHSvstime_detgrade[isite] = (TH1F*)hsvstime_detgrade[isite]->Clone(Form("cumHSvstime_%s_detgrade", sitename[isite].c_str()));
    for(int ibin=1; ibin<=limit; ibin++){
      for(int ibinprev=1; ibinprev<=ibin; ibinprev++){
        count1+= hsvstime[isite]->GetBinContent(ibinprev);
        count2+= hsvstime_detgrade[isite]->GetBinContent(ibinprev);
      }
      cumHSvstime[isite]->SetBinContent(ibin, count1);
      cumHSvstime_detgrade[isite]->SetBinContent(ibin, count2);
      count1=0.;
      count2=0.;
    }
  }

  //Cumulative HS vs time for OL (sum of 4 sites)
  cumOLHSvstime = (TH1F*)cumHSvstime[1]->Clone("cumOLHSvstime");
  cumOLHSvstime->SetTitle("OL-HS_all");
  cumOLHSvstime_detgrade = (TH1F*)cumHSvstime_detgrade[1]->Clone("cumOLHSvstime_detgrade");
  cumOLHSvstime_detgrade->SetTitle("OL-HS_detgrade");

  for(int i=2; i<nSites; i++){
    cumOLHSvstime->Add(cumHSvstime[i]);
    cumOLHSvstime_detgrade->Add(cumHSvstime_detgrade[i]);
  }

  //Calculate yield
  for(int isite=0; isite<nSites; isite++){
    yieldsites[isite] = (TH1F*)cumHSvstime[isite]->Clone(Form("yield_%s", sitename[isite].c_str()));
    yieldsites[isite]->Reset();
    yieldsites[isite]->Divide(cumHSvstime_detgrade[isite], cumHSvstime[isite], 1., 1.);
    yieldsites[isite]->Scale(100);
  }
  yieldOL = (TH1F*)cumHSvstime[0]->Clone("yield_OL");
  yieldOL->Reset();
  yieldOL->Divide(cumOLHSvstime_detgrade, cumOLHSvstime, 1., 1.);
  yieldOL->Scale(100);

  //Plot with the total number of HS and HS DG for each site
  TH1F *hHS = new TH1F("hHStotal", "All HS", nSites, 0.5, 5.5);
  TH1F *hHS_dg = new TH1F("hHStotalDG", "Det. Grade HS", nSites, 0.5, 5.5);
  for(int is=0; is<nSites; is++){
    hHS->SetBinContent(is+1, cumHSvstime[is]->GetBinContent(cumHSvstime[is]->GetNbinsX()));
    hHS_dg->SetBinContent(is+1, cumHSvstime_detgrade[is]->GetBinContent(cumHSvstime_detgrade[is]->GetNbinsX()));
    hHS->GetXaxis()->SetBinLabel(is+1, sitename[is].c_str());
    hHS_dg->GetXaxis()->SetBinLabel(is+1, sitename[is].c_str());
  }
  hHS->GetXaxis()->SetLabelSize(0.04);
  hHS_dg->GetXaxis()->SetLabelSize(0.04);

  //Define pie charts and their style
  TPie *pie[nSites];
  TPie *pieOL = new TPie("ol-hs", "HS - OL", (int)finaldeadchipsOL.size(), &finaldeadchipsOL[0], &colOL[0]);
  for(int isite=0; isite<nSites; isite++){
    pie[isite] = new TPie(sitename[isite].c_str(), Form("HS - %s",!isite ? "ML" : sitename[isite].c_str()), (int)finaldeadch[isite].size(), &finaldeadch[isite][0], &finalcol[isite][0]);
    for(int ilab=0; ilab<(int)finallab[isite].size(); ilab++)
      pie[isite]->SetEntryLabel(ilab, finallab[isite][ilab].Data());
    for(int ilab=0; ilab<(int)finallabOL.size(); ilab++)
      pieOL->SetEntryLabel(ilab, finallabOL[ilab].Data());

    SetStylePie(pie[isite]);
  }
  SetStylePie(pieOL);
  pieOL->SetTextSize(0.03);
  pie[0]->SetTextSize(0.03);

  //Style for hs vs time plots and yield vs time plots
  for(int isite=0; isite<nSites; isite++){
    SetStyleSite(cumHSvstime[isite], color_site[isite]);
    SetStyleSite(cumHSvstime_detgrade[isite], color_site[isite]);
    SetStyleSite(yieldsites[isite], color_site[isite]);
  }
  SetStyleSite(cumOLHSvstime_detgrade, 2);
  SetStyleSite(yieldOL, 2);

  //////////////////////////////////////////////////
  /////////////// DRAW SECTION /////////////////////
  //////////////////////////////////////////////////

  //Intoduction
  TCanvas *cIntro = new TCanvas("cIntro", "cIntro");
  TLatex *lat = new TLatex();
  lat->SetNDC();
  lat->SetTextFont(42);
  lat->SetTextSize(0.08);
  lat->DrawLatex(0.35,0.5,"HS monitoring");
  cIntro->Print("Results/Stave-HS_results.pdf");

  //HSs of the week
  TCanvas *cHSweek = new TCanvas("cHSweek", "cHSweek");
  TPaveText *ptHS = new TPaveText(.05,.1,.95,.8);
  ptHS->AddText("HSs of the week");
  ptHS->AddText(" ");
  for(int i=0; i<(int)thisweekHS.size(); i++){
    ptHS->AddText(thisweekHS[i].c_str());
  }
  ptHS->Draw();
  cHSweek->Print("Results/Stave-HS_results.pdf");

  //Pie charts for OL-HS
  TCanvas *cnvpie_OLsites = new TCanvas("cpie_OLsites", "cpie_OLsites");
  cnvpie_OLsites->Divide(2,2);
  lat->SetTextSize(0.06);
  lat->SetTextColor(kGreen+2);
  for(int isite=1; isite<nSites; isite++){
    cnvpie_OLsites->cd(isite);
    pie[isite]->Draw("3d nol sc <");
    lat->DrawLatex(0.8,0.8,Form("%.2f %% ok",yieldDetGrade[isite]));
  }
  cnvpie_OLsites->Print("Results/Stave-HS_results.pdf");

  //Pie charts OL(sum) and ML
  TCanvas *cnvpie_OLML = new TCanvas("cpieOLML", "cpieOLML");
  cnvpie_OLML->Divide(2,1);
  cnvpie_OLML->cd(1);
  pieOL->Draw("3d nol sc <");//OL
  lat->DrawLatex(0.7,0.8,Form("%.2f %% ok",yieldDetGradeOL));
  cnvpie_OLML->cd(2);
  pie[0]->Draw("3d nol sc <");//ML
  lat->DrawLatex(0.7,0.8,Form("%.2f %% ok",yieldDetGrade[0]));
  cnvpie_OLML->Print("Results/Stave-HS_results.pdf");

  //Total number of HS and HS_DG for each site
  TCanvas *cHStot = new TCanvas("cHStot", "cHStot");
  cHStot->Divide(2,1);
  cHStot->cd(1);
  cHStot->GetPad(1)->SetGridy();
  hHS->SetLineWidth(2);
  hHS->Draw("HIST TEXT0");
  cHStot->cd(2);
  cHStot->GetPad(2)->SetGridy();
  hHS_dg->SetLineWidth(2);
  hHS_dg->Draw("HIST TEXT0");
  cHStot->Print("Results/Stave-HS_results.pdf");

  //det. grade HS vs time & total (OL and ML) det. grade HS vs time
  TCanvas *cHSvstime = new TCanvas("cHSvstime", "cHSvstime");
  cHSvstime->Divide(2,1);
  TLegend *legsites = new TLegend(0.0995, 0.8329, 0.8989, 0.918);
  SetLegendStyle(legsites);
  legsites->SetNColumns(2);
  legsites->SetTextSize(0.042);
  for(int isite=0; isite<nSites; isite++)
    legsites->AddEntry(cumHSvstime[isite], sitename[isite].c_str(), "l");
  TLegend *legOLML = new TLegend(0.0995, 0.8329, 0.8989, 0.918);
  SetLegendStyle(legOLML);
  legOLML->SetNColumns(2);
  legOLML->SetTextSize(0.042);
  legOLML->AddEntry(cumHSvstime[0], "ML", "l");
  legOLML->AddEntry(cumOLHSvstime_detgrade, "OL", "l");

  cHSvstime->cd(1);
  cHSvstime->GetPad(1)->SetMargin(0.099,0.1,0.2589,0.18);
  TH1F *frame = cHSvstime->GetPad(1)->DrawFrame(0., 0., thisweek+0.5, cumHSvstime[0]->GetMaximum()+5, "Det. grade HS vs time; Week; #HS");
  frame->SetBins(nbins, 0.5, upperedge);
  frame->GetXaxis()->SetTickLength(0.015);
  frame->GetXaxis()->SetTitleSize(0.05);
  frame->GetYaxis()->SetTitleSize(0.05);
  frame->GetYaxis()->SetTitleOffset(0.9);
  SetLabelsHS(frame, year);
  for(int i=0; i<nSites; i++)
    cumHSvstime_detgrade[i]->Draw("PL same");

  cHSvstime->cd();
  TPaveText *pt1 = new TPaveText(.05,.042,.28,.18);
  pt1->AddText("Comparison to prev. week");
  for(int is=0; is<nSites;is++){
    pt1->AddText(Form("%s: +%.0f", sitename[is].c_str(),cumHSvstime_detgrade[is]->GetBinContent(cumHSvstime_detgrade[is]->GetNbinsX())- cumHSvstime_detgrade[is]->GetBinContent(cumHSvstime_detgrade[is]->GetNbinsX()-1)));
  }
  pt1->Draw();

  cHSvstime->cd(2);
  cHSvstime->GetPad(2)->SetMargin(0.099,0.1,0.1,0.18);
  TH1F *frame2 = (TH1F*)frame->Clone("frame2");
  frame2->Draw();
  frame2->GetYaxis()->SetRangeUser(0., cumHSvstime_detgrade[1]->GetMaximum()+cumHSvstime_detgrade[2]->GetMaximum()+cumHSvstime_detgrade[3]->GetMaximum()+cumHSvstime_detgrade[4]->GetMaximum()+8);
  cumHSvstime_detgrade[0]->Draw("PL same");
  cumOLHSvstime_detgrade->Draw("PL same");
  legOLML->Draw();

  cHSvstime->Print("Results/Stave-HS_results.pdf");

  //Draw yield
  TCanvas *cYieldvstime = new TCanvas("cYieldvstime", "cYieldvstime");
  cYieldvstime->Divide(2,1);
  TLine *line90 = new TLine(yieldOL->GetBinLowEdge(1), 90, upperedge, 90);
  line90->SetLineStyle(2);
  line90->SetLineColor(kGray+2);
  cYieldvstime->cd(1);
  cYieldvstime->GetPad(1)->SetMargin(0.099,0.1,0.1,0.18);
  TH1F *frameyield = cYieldvstime->GetPad(1)->DrawFrame(0., 0., upperedge, 105, "HS Yield vs time; Week; Yield");
  frameyield->SetBins(nbins, 0.5, upperedge);
  frameyield->GetXaxis()->SetTickLength(0.015);
  frameyield->GetXaxis()->SetTitleSize(0.05);
  frameyield->GetYaxis()->SetTitleSize(0.05);
  frameyield->GetYaxis()->SetTitleOffset(0.95);
  SetLabelsHS(frameyield, year);
  line90->Draw("same");
  for(int i=0; i<nSites; i++)
    yieldsites[i]->Draw("L HIST same");
  legsites->Draw();

  cYieldvstime->cd(2);
  cYieldvstime->GetPad(2)->SetMargin(0.099,0.1,0.1,0.18);
  frameyield->Draw();
  line90->Draw("same");
  yieldsites[0]->Draw("L HIST same");
  yieldOL->Draw("L HIST same");
  legOLML->Draw();

  cYieldvstime->Print("Results/Stave-HS_results.pdf");

  return 1;
}
