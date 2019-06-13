//
// MAIN
//

bool staveanalysis(int year, int thisweek){

  gStyle->SetOptStat(0000);

  TH1F *stavevstime[nSites+1], *cumStavevstime[nSites+1];//#Stave vs time (all) and cumulative #Stave vs time (+1 is for reworked staves)
  TH1F *stavevstime_detgrade[nSites+1], *cumStavevstime_detgrade[nSites+1];//#Stave vs time (det grade) and cumulative #Stave vs time (det. grade) (+1 is for reworked staves)
  TH1F *cumOLStavevstime_detgrade, *cumOLStavevstime;
  TH1F *yieldsites[nSites+1], *yieldOL;
  //number of bins and upper edge of plots vs time
  int nbins;
  double upperedge;
  if(year==2019) {
    nbins=52+thisweek;
    upperedge = 52+(double)thisweek+0.5;
  }
  else{
    nbins=thisweek;
    upperedge=52+(double)thisweek+0.5;
  }
  for(int i=0; i<nSites+1; i++){
    stavevstime[i] = new TH1F(Form("%s_Stave_vs_time", i==nSites ? "Rework":sitename[i].c_str()), Form("%s - Stave vs time", i==nSites ? "Rework":sitename[i].c_str()), nbins, 0.5, upperedge);
    stavevstime_detgrade[i] = new TH1F(Form("%s_Stave_vs_time_dg", i==nSites ? "Rework":sitename[i].c_str()), Form("%s - Stave vs time (det. grade)",i==nSites ? "Rework":sitename[i].c_str()), nbins, 0.5, upperedge);
  }
  //Read file extracting the number of working chips HS by HS
  ifstream infl("staveresults.dat");
  string staveid, qualdate, siteid;
  int chipsokHSL, chipsokHSU, week, sitenum, categnum, staveyear;
  vector <string> thisweekStave, prevweekStave;
  vector <int> deadchtwU, deadchtwL, deadchpwU, deadchpwL;
  string hsuid, hslid;

  double nstaveincat[nSites+1][nCatStave];//+1 is for reworked staves
  double nstaveincatOL[nCatStave] = {0.};
  for(int i=0; i<nSites+1; i++)
    for(int j=0; j<nCatStave; j++)
      nstaveincat[i][j]=0.;

  while(infl>>staveid>>hsuid>>hslid>>chipsokHSU>>chipsokHSL>>qualdate>>week){
    siteid = staveid.substr(0,1);
    sitenum = GetSiteNumber(siteid);
    categnum = GetStaveCategoryId(chipsokHSU, chipsokHSL, staveid);
    staveyear = stoi(qualdate.substr(qualdate.length()-4));

    nstaveincat[sitenum][categnum]++;//dead chips site by site
    if(sitenum!=0) nstaveincatOL[categnum]++;//also reworked are counted here
    stavevstime[sitenum]->Fill(staveyear==2019 ? (double)week+52. : (double)week);//all stave vs time
    bool detgrade = IsStaveDetGrade(categnum, staveid);
    if(staveid=="T-OL-Stave-004") detgrade = kTRUE; //exception for T-OL-Stave-004 since mounted already in the detector
    if(detgrade) stavevstime_detgrade[sitenum]->Fill(staveyear==2019 ? (double)week+52. : (double)week);//det. grade stave vs time
    if(week==thisweek && staveyear==year) {
      thisweekStave.push_back(staveid);
      deadchtwU.push_back(siteid=="B" ? 56-chipsokHSU : 98-chipsokHSU);
      deadchtwL.push_back(siteid=="B" ? 56-chipsokHSL : 98-chipsokHSL);
    }
    if(week==thisweek-1 && staveyear==year) {
      prevweekStave.push_back(staveid);
      deadchpwU.push_back(siteid=="B" ? 56-chipsokHSU : 98-chipsokHSU);
      deadchpwL.push_back(siteid=="B" ? 56-chipsokHSL : 98-chipsokHSL);
    }
  }
  infl.close();

  //Calculate the present yield (not vs time!)
  vector <double> yieldDetGrade;
  double yieldDetGradeOL;
  double sum = 0., num=0.;
  for(int i=0; i<nSites+1; i++){//+1 is to include also reworked staves
    sum = 0.;
    num=0.;
    for(int j=0; j<nCatStave; j++){
      sum+=nstaveincat[i][j];
      if(i==0 && j<2) num+=nstaveincat[i][j];
      if(i!=0 && j<6) num+=nstaveincat[i][j];
    }
    yieldDetGrade.push_back(num/sum*100.);
  }
  sum=0.;
  num=0.;
  for(int i=0;i<nCatStave; i++){
    sum+=nstaveincatOL[i];
    if(i<6) num+=nstaveincatOL[i];
  }
  yieldDetGradeOL=num/sum*100.;

  //Calculate the production rate from week 36 (October 2018) to last week
  double prodrate_detgrade[nSites+1], prodrate_all[nSites+1];//+1 is to include the rework rate
  double weekstart = 40;//beginning of October
  for(int is=0; is<nSites+1; is++){
    double upint = year==2019 ? (double)(thisweek-1)+52 : (double)thisweek-1;
    double nweeks = year==2019 ? (double)52-weekstart+1+(thisweek-1) : (double)(thisweek-1)-weekstart+1;
    if(is==4){ //Torino
	upint = 18.+52.; //production ended 29th April
        nweeks = 52-weekstart+1+18;
    }
    if(is==5){//rework rate
	weekstart=22+52;//beginning of June 2019
	nweeks = thisweek+52-weekstart;//supposing year=2019
    }
    if(is!=5) nweeks-=2; //remove week 52 of 2018 and week 1 of January (Christmas holiday)
    double christmas_all = stavevstime[is]->GetBinContent(52)+stavevstime[is]->GetBinContent(53);
    double christmas_dg = stavevstime_detgrade[is]->GetBinContent(52)+stavevstime_detgrade[is]->GetBinContent(53); 
    if(is==5) {christmas_all=0.; christmas_dg=0.;}
    prodrate_all[is] = (stavevstime[is]->Integral(weekstart, upint)-christmas_all) / nweeks;
    prodrate_detgrade[is] = (stavevstime_detgrade[is]->Integral(weekstart, upint)-christmas_dg) / nweeks;

  }

  //Calculate the production rate in 2019 (month by month)
  const int nMonth = 5;//January, February, March, April
  const string monthname[nMonth] = {"January","February","March","April","May"};
  double weekst[nMonth] = {2, 5, 9, 14,18};
  double weeken[nMonth] = {5, 9, 13, 18,22};
  double prodrate_detgrade_month[nSites][nMonth], prodrate_all_month[nSites][nMonth];
  for(int im=0; im<nMonth; im++){
  	for(int is=0; is<nSites; is++){
    		double lowint = year==2019 ? weekst[im]+52. : weekst[im];
    		double upint = year==2019 ? weeken[im]+52. : weeken[im];
    		prodrate_all_month[is][im] = (stavevstime[is]->Integral(lowint, upint)) / (weeken[im]-weekst[im]+1);
    		prodrate_detgrade_month[is][im] = (stavevstime_detgrade[is]->Integral(lowint, upint)) / (weeken[im]-weekst[im]+1);

  	}
  }


  //Remove categories with 0 counts (bad labels in TPie)
  vector <vector<double>> finalnstaveincat;
  vector <vector <TString>> finallab;
  vector <vector <int>> finalcol;
  vector <int> row2, colOL;
  vector <double> finalnstaveincatOL, row1;
  vector <TString> finallabOL, row3;
  for(int i=0; i<nSites+1; i++){//+1 is to include rework
    for(int j=0; j<nCatStave; j++){
      if(nstaveincat[i][j]>0.5){
        row1.push_back(nstaveincat[i][j]);
        row2.push_back(colorstv[j]);
        row3.push_back(labelstv[j]);
      }
      if(j==nCatStave-1){
        finalnstaveincat.push_back(row1);
        finalcol.push_back(row2);
        finallab.push_back(row3);
        row1.clear();
        row2.clear();
        row3.clear();
      }
    }
  }
  for(int i=0; i<nCatStave; i++){
    if(nstaveincatOL[i]>0.5){
      finalnstaveincatOL.push_back(nstaveincatOL[i]);
      colOL.push_back(colorstv[i]);
      finallabOL.push_back(labelstv[i]);
    }
  }

  //Make cumulative Stave vs time
  double count1=0., count2=0.;
  int limit = year==2019 ? 52+thisweek : thisweek;
  for(int isite=0; isite<nSites+1; isite++){//+1 is to include rework
    cumStavevstime[isite] = (TH1F*)stavevstime[isite]->Clone(Form("cumStavevstime_%s", isite==nSites?"Rework":sitename[isite].c_str()));
    cumStavevstime_detgrade[isite] = (TH1F*)stavevstime_detgrade[isite]->Clone(Form("cumStavevstime_%s_detgrade", isite==nSites?"Rework":sitename[isite].c_str()));
    for(int ibin=1; ibin<=limit; ibin++){
      for(int ibinprev=1; ibinprev<=ibin; ibinprev++){
        count1+= stavevstime[isite]->GetBinContent(ibinprev);
        count2+= stavevstime_detgrade[isite]->GetBinContent(ibinprev);
      }
      cumStavevstime[isite]->SetBinContent(ibin, count1);
      cumStavevstime_detgrade[isite]->SetBinContent(ibin, count2);
      count1=0.;
      count2=0.;
    }
  }

  //Cumulative Stave vs time for OL (sum of 4 sites)
  cumOLStavevstime = (TH1F*)cumStavevstime[1]->Clone("cumOLStavevstime");
  cumOLStavevstime->SetTitle("OL-Stave_all");
  cumOLStavevstime_detgrade = (TH1F*)cumStavevstime_detgrade[1]->Clone("cumOLStavevstime_detgrade");
  cumOLStavevstime_detgrade->SetTitle("OL-Stave_detgrade");

  for(int i=2; i<nSites+1; i++){//Include reworked
    cumOLStavevstime->Add(cumStavevstime[i]);
    cumOLStavevstime_detgrade->Add(cumStavevstime_detgrade[i]);
  }

  //Calculate yield
  for(int isite=0; isite<nSites+1; isite++){//+1 is to include rework
    yieldsites[isite] = (TH1F*)cumStavevstime[isite]->Clone(Form("yield_%s", isite==nSites ? "Reworked":sitename[isite].c_str()));
    yieldsites[isite]->Reset();
    yieldsites[isite]->Divide(cumStavevstime_detgrade[isite], cumStavevstime[isite], 1., 1.);
    yieldsites[isite]->Scale(100.);
  }
  yieldOL = (TH1F*)cumStavevstime[0]->Clone("yield_OL");
  yieldOL->Reset();
  yieldOL->Divide(cumOLStavevstime_detgrade, cumOLStavevstime, 1., 1.);
  yieldOL->Scale(100.);

  //Plot with the total number of Stave and Stave DG for each site
  TH1F *hStave = new TH1F("hStavetotal", "All Stave", nSites+1, 0.5, 6.5);
  TH1F *hStave_dg = new TH1F("hStavetotalDG", "Det. Grade Stave", nSites+1, 0.5, 6.5);
  for(int is=0; is<nSites+1; is++){
    hStave->SetBinContent(is+1, cumStavevstime[is]->GetBinContent(cumStavevstime[is]->GetNbinsX()));
    hStave_dg->SetBinContent(is+1, cumStavevstime_detgrade[is]->GetBinContent(cumStavevstime_detgrade[is]->GetNbinsX()));
    hStave->GetXaxis()->SetBinLabel(is+1, is==nSites? "Reworked":sitename[is].c_str());
    hStave_dg->GetXaxis()->SetBinLabel(is+1, is==nSites? "Reworked":sitename[is].c_str());
  }
  hStave->GetXaxis()->SetLabelSize(0.04);
  hStave_dg->GetXaxis()->SetLabelSize(0.04);

  //Define pie charts and their style
  TPie *pie[nSites];
  TPie *pieOL = new TPie("ol-stave", "Stave - OL (includes rwk)", (int)finalnstaveincatOL.size(), &finalnstaveincatOL[0], &colOL[0]);//includes reworked!
  for(int isite=0; isite<nSites; isite++){
    pie[isite] = new TPie(sitename[isite].c_str(), Form("Stave - %s",!isite ? "ML" : sitename[isite].c_str()), (int)finalnstaveincat[isite].size(), &finalnstaveincat[isite][0], &finalcol[isite][0]);
    for(int ilab=0; ilab<(int)finallab[isite].size(); ilab++)
      pie[isite]->SetEntryLabel(ilab, finallab[isite][ilab].Data());
    for(int ilab=0; ilab<(int)finallabOL.size(); ilab++)
      pieOL->SetEntryLabel(ilab, finallabOL[ilab].Data());

    SetStylePie(pie[isite]);
  }
  SetStylePie(pieOL);
  pieOL->SetTextSize(0.03);
  pie[0]->SetTextSize(0.03);

  //Style for stave vs time plots and yield vs time plots
  for(int isite=0; isite<nSites+1; isite++){
    SetStyleSite(cumStavevstime[isite], color_site[isite]);
    cumStavevstime[isite]->SetLineStyle(9);
    SetStyleSite(cumStavevstime_detgrade[isite], color_site[isite]);
    SetStyleSite(yieldsites[isite], color_site[isite]);
  }
  SetStyleSite(cumOLStavevstime_detgrade, 2);
  SetStyleSite(cumOLStavevstime,2);
  cumOLStavevstime->SetLineStyle(9);
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
  lat->DrawLatex(0.35,0.5,"Stave monitoring");
  cIntro->Print("Results/Stave-HS_results.pdf");

  //Staves of the week
  TCanvas *cStaveweek = new TCanvas("cStaveweek", "cStaveweek");
  TPaveText *ptStave = new TPaveText(.05,.1,.95,.8);
  ptStave->AddText("Staves of previous week");
  ptStave->AddText(" ");
  for(int i=0; i<(int)prevweekStave.size(); i++){
    ptStave->AddText(Form("%s: (U,L)=(%d, %d) bad chips", prevweekStave[i].c_str(), deadchpwU[i], deadchpwL[i]));
  }
  ptStave->AddText(" ");
  ptStave->AddText(" ");
  ptStave->AddText("Staves of this week");
  ptStave->AddText(" ");
  for(int i=0; i<(int)thisweekStave.size(); i++){
    ptStave->AddText(Form("%s: (U,L)=(%d, %d) bad chips", thisweekStave[i].c_str(), deadchtwU[i], deadchtwL[i]));
  }
  ptStave->Draw();
  cStaveweek->Print("Results/Stave-HS_results.pdf");

  //Pie charts for OL-Staves
  TCanvas *cnvpie_OLStavesites = new TCanvas("cpie_OLStavesites", "cpie_OLStavesites");
  cnvpie_OLStavesites->Divide(2,2);
  lat->SetTextSize(0.06);
  lat->SetTextColor(kGreen+2);
  for(int isite=1; isite<nSites; isite++){
    cnvpie_OLStavesites->cd(isite);
    pie[isite]->Draw("3d nol sc <");
    lat->DrawLatex(0.75,0.8,Form("%.2f %% ok",yieldDetGrade[isite]));
  }
  cnvpie_OLStavesites->Print("Results/Stave-HS_results.pdf");

  //Pie charts OL(sum) and ML
  TCanvas *cnvpie_OLML_Stave = new TCanvas("cpieOLMLStave", "cpieOLMLStave");
  cnvpie_OLML_Stave->Divide(2,1);
  cnvpie_OLML_Stave->cd(1);
  pieOL->Draw("3d nol sc <");//OL
  lat->DrawLatex(0.65,0.8,Form("%.2f %% ok",yieldDetGradeOL));
  cnvpie_OLML_Stave->cd(2);
  pie[0]->Draw("3d nol sc <");//ML
  lat->DrawLatex(0.65,0.8,Form("%.2f %% ok",yieldDetGrade[0]));
  cnvpie_OLML_Stave->Print("Results/Stave-HS_results.pdf");

  //Total number of Stave and Stave_DG for each site
  TCanvas *cStavetot = new TCanvas("cStavetot", "cStavetot");
  cStavetot->Divide(2,1);
  cStavetot->cd(1);
  cStavetot->GetPad(1)->SetGridy();
  hStave->SetLineWidth(2);
  hStave->Draw("HIST TEXT0");
  cStavetot->cd(2);
  cStavetot->GetPad(2)->SetGridy();
  hStave_dg->SetLineWidth(2);
  hStave_dg->Draw("HIST TEXT0");
  cStavetot->Print("Results/Stave-HS_results.pdf");

  //det. grade stave vs time & total (OL and ML) det. grade Stave vs time
  TCanvas *cStavevstime1 = new TCanvas("cStavevstime1", "cStavevstime1");
  TLegend *legsites = new TLegend(0.0995, 0.8329, 0.8989, 0.918);
  SetLegendStyle(legsites);
  legsites->SetNColumns(2);
  legsites->SetTextSize(0.042);
  for(int isite=0; isite<nSites+1; isite++)
    legsites->AddEntry(cumStavevstime_detgrade[isite], isite==nSites?"Reworked":sitename[isite].c_str(), "l");
  TLegend *legOLML = new TLegend(0.0995, 0.8329, 0.8989, 0.918);
  SetLegendStyle(legOLML);
  legOLML->SetNColumns(2);
  legOLML->SetTextSize(0.042);
  legOLML->AddEntry(cumStavevstime[0], "ML(all)", "l");
  legOLML->AddEntry(cumStavevstime_detgrade[0],"ML(DG)","l");
  legOLML->AddEntry(cumOLStavevstime,"OL(all)","l");
  legOLML->AddEntry(cumOLStavevstime_detgrade, "OL(DG)", "l");
  cStavevstime1->SetMargin(0.099,0.1,0.2589,0.18);
  TH1F *frame = cStavevstime1->DrawFrame(0., 0., upperedge, cumStavevstime[0]->GetMaximum()+5, "Det. grade Stave vs time; Week; #Stave");
  frame->SetBins(nbins, 0.5, upperedge);
  frame->GetXaxis()->SetTickLength(0.015);
  frame->GetXaxis()->SetRangeUser(27, upperedge); //for better visibility
  frame->GetXaxis()->SetTitleSize(0.05);
  frame->GetYaxis()->SetTitleSize(0.05);
  frame->GetYaxis()->SetTitleOffset(0.9);
  SetLabels(frame, year);
  for(int i=0; i<nSites+1; i++)
    cumStavevstime_detgrade[i]->Draw("PL same");
  legsites->Draw();
  cStavevstime1->cd();
  TPaveText *pt1 = new TPaveText(.099,.025,.207,.174,"NDC");
  pt1->AddText("Comparison to prev. week");
  for(int is=0; is<nSites+1;is++){
    pt1->AddText(Form("%s: +%.0f", is==nSites?"Reworked":sitename[is].c_str(),cumStavevstime_detgrade[is]->GetBinContent(cumStavevstime_detgrade[is]->GetNbinsX())- cumStavevstime_detgrade[is]->GetBinContent(cumStavevstime_detgrade[is]->GetNbinsX()-1)));
  }
  pt1->Draw();
  cStavevstime1->Print("Results/Stave-HS_results.pdf");

  TCanvas *cStavevstime2 = new TCanvas("cStavevstime2","cStavevstime2");
  cStavevstime2->SetMargin(0.099,0.1,0.1,0.18);
  TH1F *frame2 = (TH1F*)frame->Clone("frame2");
  frame2->Draw();
  frame2->GetXaxis()->SetRangeUser(27, upperedge); //for better visibility
  frame2->GetYaxis()->SetRangeUser(0., cumStavevstime[1]->GetMaximum()+cumStavevstime[2]->GetMaximum()+cumStavevstime[3]->GetMaximum()+cumStavevstime[4]->GetMaximum()+8);
  frame2->Draw();
  cumOLStavevstime->Draw("PL same");
  cumStavevstime[0]->Draw("PL same");
  cumStavevstime_detgrade[0]->Draw("PL same");
  cumOLStavevstime_detgrade->Draw("PL same");
  legOLML->Draw();

  cStavevstime2->Print("Results/Stave-HS_results.pdf");

  //Draw yield
  TLegend *legOLML2 = new TLegend(0.0995, 0.8329, 0.8989, 0.918);
  SetLegendStyle(legOLML2);
  legOLML2->SetNColumns(2);
  legOLML2->SetTextSize(0.042);
  legOLML2->AddEntry(cumStavevstime_detgrade[0],"ML","l");
  legOLML2->AddEntry(cumOLStavevstime_detgrade, "OL", "l");

  TCanvas *cStaveYieldvstime1 = new TCanvas("cStaveYieldvstime1", "cStaveYieldvstime1");
  TLine *line90 = new TLine(27, 90, upperedge, 90);
  line90->SetLineStyle(2);
  line90->SetLineColor(kGray+2);
  cStaveYieldvstime1->SetMargin(0.099,0.1,0.1,0.18);
  TH1F *frameyield = cStaveYieldvstime1->DrawFrame(0., 0., upperedge, 105, "Stave yield vs time; Week; Yield");
  frameyield->SetBins(nbins, 0.5, upperedge);
  frameyield->GetXaxis()->SetTickLength(0.015);
  frameyield->GetXaxis()->SetRangeUser(27, upperedge); //for better visibility
  frameyield->GetXaxis()->SetTitleSize(0.05);
  frameyield->GetYaxis()->SetTitleSize(0.05);
  frameyield->GetYaxis()->SetTitleOffset(0.95);
  SetLabels(frameyield, year);
  line90->Draw("same");
  for(int i=0; i<nSites; i++)
    yieldsites[i]->Draw("L HIST same");
  legsites->Draw();
  cStaveYieldvstime1->Print("Results/Stave-HS_results.pdf");

  TCanvas* cStaveYieldvstime2 = new TCanvas("cStaveYieldvstime2","cStaveYieldvstime2");
  cStaveYieldvstime2->SetMargin(0.099,0.1,0.1,0.18);
  frameyield->Draw();
  line90->Draw("same");
  yieldsites[0]->Draw("L HIST same");
  yieldOL->Draw("L HIST same");
  legOLML2->Draw();

  cStaveYieldvstime2->Print("Results/Stave-HS_results.pdf");

  //Draw a table with the production rate
  TCanvas *cprodrate = new TCanvas("cprodrate", "cprodrate");
  TPaveText *pt = new TPaveText(.05,.1,.95,.8);
  pt->AddText("Production rate (October 2018 - prev. week)**");
  for(int is=0; is<nSites; is++){
    pt->AddText(Form("#rightarrow %s: %.2f(all) -- %.2f(DG) %s", sitename[is].c_str(), prodrate_all[is], prodrate_detgrade[is], is==4 ? "#rightarrow Prod. ended":""));
  }
  pt->AddText("");
  pt->AddText(Form("OL: %.2f(all) -- %.2f(DG)", prodrate_all[1]+prodrate_all[2]+prodrate_all[3]+prodrate_all[4], prodrate_detgrade[1]+prodrate_detgrade[2]+prodrate_detgrade[3]+prodrate_detgrade[4]));
  pt->AddText(Form("ML: %.2f(all) -- %.2f(DG)", prodrate_all[0], prodrate_detgrade[0]));
  pt->AddText(Form("Rework rate (from June 1st, 2019): %.2f(all) -- %.2f(DG)", prodrate_all[5], prodrate_detgrade[5]));
  pt->AddText("");
  pt->AddText("**Christmas holiday excluded (2 weeks)");

  pt->Draw();
  cprodrate->Print("Results/Stave-HS_results.pdf");

  //Draw a table with the production rate month by month
  TCanvas *cprodrate_m = new TCanvas("cprodrate_m", "cprodrate_m");
  pt->Clear();
  pt->AddText("Production rate 2019 (month by month)**");
  for(int im=0; im<nMonth; im++){
    pt->AddText(monthname[im].c_str());
  	for(int is=0; is<nSites; is++){
    		if(im<4) pt->AddText(Form("#rightarrow %s: %.2f(all) -- %.2f(DG)", sitename[is].c_str(), prodrate_all_month[is][im], prodrate_detgrade_month[is][im]));
		else if(im==4 && is==4) pt->AddText(Form("#rightarrow %s: Production ended", sitename[is].c_str()));
		else pt->AddText(Form("#rightarrow %s: %.2f(all) -- %.2f(DG)", sitename[is].c_str(), prodrate_all_month[is][im], prodrate_detgrade_month[is][im]));
  	}
    double sumtotal = prodrate_all_month[1][im]+prodrate_all_month[2][im]+prodrate_all_month[3][im]+prodrate_all_month[4][im];
    double sumtotalDG = prodrate_detgrade_month[1][im]+prodrate_detgrade_month[2][im]+prodrate_detgrade_month[3][im]+prodrate_detgrade_month[4][im];
    if(im>3) {
	sumtotal = prodrate_all_month[1][im]+prodrate_all_month[2][im]+prodrate_all_month[3][im]; //no Torino
	sumtotalDG = prodrate_detgrade_month[1][im]+prodrate_detgrade_month[2][im]+prodrate_detgrade_month[3][im];//no Torino
    } 

    pt->AddText(Form("OL: %.2f(all) -- %.2f(DG)", sumtotal, sumtotalDG));
    pt->AddText(Form("ML: %.2f(all) -- %.2f(DG)", prodrate_all_month[0][im], prodrate_detgrade_month[0][im]));
    pt->AddText("");
  }
  pt->Draw();
  cprodrate_m->Print("Results/Stave-HS_results.pdf");


  return 1;
}
