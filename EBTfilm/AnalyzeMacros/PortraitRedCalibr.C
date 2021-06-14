{
//=========Macro generated from canvas: c1/Calibration of for_calibration_v700_EBT2 Tue Dec 23 15:05:36 2014
//=========  (Tue Dec 23 15:06:11 2014) by ROOT version5.34/07
   TCanvas *c1 = new TCanvas("c1", "Calibration of for_calibration_v700_EBT2 Tue Dec 23 15:05:36 2014",10,30,700,500);
   gStyle->SetOptFit(1);
   c1->Range(10125,-0.98125,48875,1.33125);
   c1->SetFillColor(0);
   c1->SetBorderMode(0);
   c1->SetBorderSize(2);
   c1->SetFrameBorderMode(0);
   c1->SetFrameBorderMode(0);
   
   TH2F *htemp__1 = new TH2F("htemp__1","for_calibration_v700_EBT2 20141223",40,14000,45000,40,-0.75,1.1);
   htemp__1->SetDirectory(0);
   htemp__1->SetStats(0);

   Int_t ci;   // for color index setting
   ci = TColor::GetColor("#000099");
   htemp__1->SetLineColor(ci);
   htemp__1->GetXaxis()->SetTitle("r");
   htemp__1->GetXaxis()->SetRange(1,40);
   htemp__1->GetXaxis()->SetLabelFont(42);
   htemp__1->GetXaxis()->SetLabelSize(0.035);
   htemp__1->GetXaxis()->SetTitleSize(0.035);
   htemp__1->GetXaxis()->SetTitleFont(42);
   htemp__1->GetYaxis()->SetTitle("log10(dose)");
   htemp__1->GetYaxis()->SetRange(1,40);
   htemp__1->GetYaxis()->SetLabelFont(42);
   htemp__1->GetYaxis()->SetLabelSize(0.035);
   htemp__1->GetYaxis()->SetTitleSize(0.035);
   htemp__1->GetYaxis()->SetTitleFont(42);
   htemp__1->GetZaxis()->SetLabelFont(42);
   htemp__1->GetZaxis()->SetLabelSize(0.035);
   htemp__1->GetZaxis()->SetTitleSize(0.035);
   htemp__1->GetZaxis()->SetTitleFont(42);
   htemp__1->Draw("");
   
   TPaveText *pt = new TPaveText(0.3808046,0.9339831,1.00569,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(42);
   TText *text = pt->AddText("for_calibration_v700_EBT2 20141223");
   pt->Draw();
   
   TGraph *graph = new TGraph(16);
   graph->SetName("port_red");
   graph->SetTitle("Portrait, red");
   graph->SetLineColor(2);
   graph->SetMarkerStyle(21);
   graph->SetPoint(0,43050.62109,-0.6382721561);
   graph->SetPoint(1,40508.74609,-0.3010299957);
   graph->SetPoint(2,38458.98438,-0.1611509108);
   graph->SetPoint(3,38366.85547,-0.1611509108);
   graph->SetPoint(4,36395.69531,-0.04095859516);
   graph->SetPoint(5,34811.8125,0.06069783135);
   graph->SetPoint(6,33394.09375,0.1430147958);
   graph->SetPoint(7,32445.46289,0.2095150158);
   graph->SetPoint(8,32529.41406,0.1522883313);
   graph->SetPoint(9,31035.61523,0.2600714005);
   graph->SetPoint(10,28615.73047,0.361727827);
   graph->SetPoint(11,27233.24219,0.4393326938);
   graph->SetPoint(12,24970.08594,0.5658478266);
   graph->SetPoint(13,22990.16406,0.6608654708);
   graph->SetPoint(14,19167.62305,0.8061799805);
   graph->SetPoint(15,16044.28516,0.964259632);
   
   TH1F *Graph_port_red1 = new TH1F("Graph_port_red1","Portrait, red",100,13343.65,45751.25);
   Graph_port_red1->SetMinimum(-0.7985253);
   Graph_port_red1->SetMaximum(1.124513);
   Graph_port_red1->SetDirectory(0);
   Graph_port_red1->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_port_red1->SetLineColor(ci);
   Graph_port_red1->GetXaxis()->SetLabelFont(42);
   Graph_port_red1->GetXaxis()->SetLabelSize(0.035);
   Graph_port_red1->GetXaxis()->SetTitleSize(0.035);
   Graph_port_red1->GetXaxis()->SetTitleFont(42);
   Graph_port_red1->GetYaxis()->SetLabelFont(42);
   Graph_port_red1->GetYaxis()->SetLabelSize(0.035);
   Graph_port_red1->GetYaxis()->SetTitleSize(0.035);
   Graph_port_red1->GetYaxis()->SetTitleFont(42);
   Graph_port_red1->GetZaxis()->SetLabelFont(42);
   Graph_port_red1->GetZaxis()->SetLabelSize(0.035);
   Graph_port_red1->GetZaxis()->SetTitleSize(0.035);
   Graph_port_red1->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_port_red1);
   
   
   TPaveStats *ptstats = new TPaveStats(0.62,0.695,0.98,0.935,"brNDC");
   ptstats->SetName("stats");
   ptstats->SetBorderSize(1);
   ptstats->SetFillColor(0);
   ptstats->SetTextAlign(12);
   ptstats->SetTextFont(42);
   text = ptstats->AddText("#chi^{2} / ndf = 0.009455 / 12");
   text = ptstats->AddText("p0       =  2.64 #pm 0.379 ");
   text = ptstats->AddText("p1       = -0.0001649 #pm 4.161e-005 ");
   text = ptstats->AddText("p2       = 4.84e-009 #pm 1.452e-009 ");
   text = ptstats->AddText("p3       = -6.4e-014 #pm 1.624e-014 ");
   ptstats->SetOptStat(0);
   ptstats->SetOptFit(111);
   ptstats->Draw();
   graph->GetListOfFunctions()->Add(ptstats);
   ptstats->SetParent(graph->GetListOfFunctions());
   
   TF1 *PrevFitTMP = new TF1("PrevFitTMP","pol3",13343.65,45751.25);
   PrevFitTMP->SetFillColor(19);
   PrevFitTMP->SetFillStyle(0);
   PrevFitTMP->SetLineColor(2);
   PrevFitTMP->SetLineWidth(2);
   PrevFitTMP->SetChisquare(0.009455417);
   PrevFitTMP->SetNDF(12);
   PrevFitTMP->GetXaxis()->SetLabelFont(42);
   PrevFitTMP->GetXaxis()->SetLabelSize(0.035);
   PrevFitTMP->GetXaxis()->SetTitleSize(0.035);
   PrevFitTMP->GetXaxis()->SetTitleFont(42);
   PrevFitTMP->GetYaxis()->SetLabelFont(42);
   PrevFitTMP->GetYaxis()->SetLabelSize(0.035);
   PrevFitTMP->GetYaxis()->SetTitleSize(0.035);
   PrevFitTMP->GetYaxis()->SetTitleFont(42);
   PrevFitTMP->SetParameter(0,2.63985);
   PrevFitTMP->SetParError(0,0.3789925);
   PrevFitTMP->SetParLimits(0,0,0);
   PrevFitTMP->SetParameter(1,-0.0001649095);
   PrevFitTMP->SetParError(1,4.16095e-005);
   PrevFitTMP->SetParLimits(1,0,0);
   PrevFitTMP->SetParameter(2,4.840351e-009);
   PrevFitTMP->SetParError(2,1.451991e-009);
   PrevFitTMP->SetParLimits(2,0,0);
   PrevFitTMP->SetParameter(3,-6.399567e-014);
   PrevFitTMP->SetParError(3,1.62413e-014);
   PrevFitTMP->SetParLimits(3,0,0);
   graph->GetListOfFunctions()->Add(PrevFitTMP);
   graph->Draw("lpp");
   c1->Modified();
   c1->cd();
   c1->SetSelected(c1);
}
