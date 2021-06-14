// $Id: doseHist.C 3042 2019-12-16 12:22:55Z onuchin $
// Author: Valeriy Onuchin   28.07.2011

/*************************************************************************
 *                                                                       *
 * Copyright (C) 2014 - 2019,   Valeriy Onuchin                          *
 * All rights reserved.                                                  *
 *                                                                       *
 *************************************************************************/

#include <stdio.h>

#include "TSystem.h"
#include "TH2D.h"
#include "TString.h"
#include "TROOT.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TSpectrum2.h"
#include "TAxis.h"
#include "TFile.h"
#include "TObjArray.h"
#include "TLatex.h"
#include "TCutG.h"
#include "TPolyMarker.h"
#include "TRootCanvas.h"
#include "TASImage.h"
#include "TASPaletteEditor.h"
//#include "TGLHistPainter.h"
#include "TBenchmark.h"
#include "WidgetMessageTypes.h"
#include "TLegend.h"
#include "TLegendEntry.h"
#include "TGButton.h"

#include "icru49.h"

//#include "LandscapeRed.h"
//#include "PortraitRed.h"

TString rootPath;

const int arrSize = 65536; // 2^16
double dose_vs_PR[arrSize];
double dose_vs_LR[arrSize];

TH2D *dose2d = 0;
TH1D *dose1d = 0;
double *dose = 0;
TCanvas *colz = 0;
TCanvas *contz = 0;
TCanvas *surf = 0;
TCanvas *dose1dCanvas = 0;
TCanvas *peakCanvas = 0;
TCanvas *signalCanvas = 0;
TH2 *dose2rebin = 0;
TH2 *dose2surf = 0;
TSpectrum2 *spectrum = 0;
Float_t *xpeaks = 0;
Float_t *ypeaks = 0;
TASImage *myImage = 0; // this is 100% my proudly class :)
TH1D *palHist = 0;
TASPaletteEditor *palEditor = 0; //
TGCheckButton *updateBtn = 0;

TFile *rootFile = 0;
TCanvas *sliceCanvas = 0;

TH1D *sliceX = 0;
TH1D *sliceY = 0;

TH1D *DAH = 0;
TH2D *redHist = 0;  
TH2D *greenHist = 0; 
TH2D *blueHist = 0;

UInt_t width;  // image width
UInt_t height; // image height
Double_t imgDpi = 300./25.4;

// default canvas
TCanvas *c1 = 0;

int iDVH = 0;                 // incremental index of DVH
int curDVH = 0;               // the current DVH

const int nDVH = 16;          // a total number of DVHs
TCutG *graphDVH[nDVH];        // DVH area
TGraph *DVH[nDVH];            // DVHs
int colorDVH[nDVH] = { 6, 7, 8, 9, 40, 28, 41, 38, 46, 28, 48, 42, 5, 4, 3, 2 };
TLegend *dvhLeg = 0;

const Int_t nbinsDVH = 500;   // number of bins in histDVH
TH1D *histDVH[nDVH];          // DVH histograms
TCanvas *dvhCanvas = 0;       // DVH canvas
TPad *dvhPad = 0;  
TRootCanvas *impDvh = 0;
TCanvas *imageCanvas = 0;
TRootCanvas *imageCanvasImp = 0;

TCanvas *palCanvas = 0;

// initial DVH area
Float_t leftDVH = 0;
Float_t rightDVH = 0;
Float_t bottomDVH = 0;
Float_t topDVH = 0;
Double_t areaDVH = 0;

TString fileName;

// !!! vo tmp
const int rebinFactor = 10;

TH2D *sliceXrebin = 0;
TH2D *sliceYrebin = 0;

Bool_t doCorrection = kFALSE; //kTRUE;

Bool_t calcPed = kTRUE;
Double_t pedestal = 0.248276;
Double_t gPedestal = -1000;

TCanvas *entranceCanvas = 0;
TCanvas *braggCanvas = 0;

TH1D *entranceX = 0;
TH1D *projBragg = 0;

TH1D *xproj = 0;
TH1D *yproj = 0;

//___________________________________________________________________
Bool_t isDoseFile(const char *fname)
{
   // Parse file name and extract dose 

   float dose = 0;

   TString str = fname;
   int idx = str.Index("Gy");
   str = str(0, idx);

   idx = str.Index("_");
   str = str(idx + 1, str.Length());

   idx = str.Index("_");

   if (idx > 0 && !str.BeginsWith("0")) {
      str = str(idx + 1, str.Length());
   }

   if (str.EndsWith("_")) {
      str.Chop();
   }

   str.ReplaceAll("_", ".");
   dose = atof(str.Data());

   return dose != 0;
}

//______________________________________________________________________________
void CreateSliceCanvas()
{
   //

   gStyle->SetStatX(0.95);
   gStyle->SetStatW(0.2);

   sliceCanvas = new TCanvas("Slices Canvas", "Slices", 1200, 500);
   sliceCanvas->Divide(2, 1);
   sliceCanvas->ToggleEventStatus();

   TVirtualPad *pad1 = sliceCanvas->cd(1);

   pad1->SetGridx();
   pad1->SetGridy();
   pad1->SetTickx();
   pad1->SetTicky();

   TVirtualPad *pad2 = sliceCanvas->cd(2);

   pad2->SetGridx();
   pad2->SetGridy();
   pad2->SetTickx();
   pad2->SetTicky();
}

//______________________________________________________________________________
void UpdateDvhPad()
{
   //

   dvhPad->cd();
 
   dvhPad->Clear("nodelete");
   gStyle->SetStatX(0.4);
   gStyle->SetStatW(0.3);

   if (histDVH[curDVH]) {
      histDVH[curDVH]->Draw();
   }

   if (impDvh) {
      impDvh->MapRaised();
   }
}

//______________________________________________________________________________
void handleDvhCanvasEvent(Int_t event, Int_t px, Int_t py, TObject *obj)
{
   //

   TString name = obj->GetName();

   if ((event == kButton1Up) && 
       (obj->IsA() == TGraph::Class()) &&
       name.BeginsWith("dvh")) {
      curDVH = atoi(name(3, 2).Data());
      UpdateDvhPad();
      return;
   }
}

//______________________________________________________________________________
void createDVH()
{
   //

   Double_t xDVH[nbinsDVH];
   Double_t yDVH[nbinsDVH];
   Double_t tmp[nbinsDVH];

   if (!histDVH[curDVH]) {
      return;
   }

   TAxis *xaxis = histDVH[curDVH]->GetXaxis();

   for (int i = 0; i < nbinsDVH; i++) {
      xDVH[i] = xaxis->GetBinCenter(nbinsDVH - i - 1);
   }

   tmp[nbinsDVH - 1] = 0;

   for (int i = nbinsDVH - 1; i >= 0; i--) {
       Double_t value = histDVH[curDVH]->GetBinContent(i);
       tmp[i] = tmp[i + 1] + value;
   }

   //

   Double_t max = tmp[0];

   for (int i = 0; i < nbinsDVH; i++) {
      yDVH[i] = tmp[nbinsDVH - i - 1]/max*100;
   }

   if (!dvhCanvas) {
      dvhCanvas = new TCanvas("dvh", "Dose-Area Histogram", 1200, 600);
      dvhCanvas->ToggleEventStatus();

      dvhCanvas->SetGridx();
      dvhCanvas->SetGridy();
      dvhCanvas->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", 0, 0, 
                         "handleDvhCanvasEvent(Int_t,Int_t,Int_t,TObject*)");

      impDvh = (TRootCanvas*)dvhCanvas->GetCanvasImp();
      impDvh->DontCallClose();
      TString impName = dvhCanvas->GetTitle();
      impName += " of " + fileName;
      impDvh->SetWindowName(impName.Data());      
      impDvh->SetWMPosition(700, 300);

      dvhCanvas->SetRightMargin(0.01);
      dvhCanvas->SetTopMargin(0.132);
   }

   dvhCanvas->cd();

   if (DVH[curDVH] == 0) {
      DVH[curDVH] = new TGraph(nbinsDVH, xDVH, yDVH);
      TString name = "dvh";
      name += curDVH;

      DVH[curDVH]->SetName(name);
      DVH[curDVH]->SetTitle("Dose-Area Histogram");
      DVH[curDVH]->SetLineWidth(2);
      DVH[curDVH]->SetLineColor(colorDVH[curDVH]);
  
   } else {
      for (int i = 0; i < nbinsDVH; i++) {
         DVH[curDVH]->SetPoint(i, xDVH[i], yDVH[i]);
      }
   }

   DVH[curDVH]->GetHistogram()->GetXaxis()->SetTitle("Gy");
   DVH[curDVH]->GetHistogram()->GetYaxis()->SetTitle("Area %");

   if (iDVH == 0) {
      DVH[iDVH]->Draw("al");
      DVH[iDVH]->GetHistogram()->SetMaximum(160);
      dvhPad = new TPad("dvhPad", "dvhPad", 0.7, 97./160., 1, 1, 19, 2, 1);
      dvhPad->Draw();
      dvhPad->Modified();
   } else {
      DVH[curDVH]->GetHistogram()->SetMaximum(160);
      DVH[curDVH]->Draw("l same");
   }

   UpdateDvhPad();

   dvhCanvas->cd();

   Double_t frac = graphDVH[curDVH]->IntegralHist(dose2d, "width")/dose2d->Integral("width")*100.;

   TString legTxt = TString::Format("Area = %.0f mm^{2} #frac{dose_{area}}{dose_{total}} = %2.1f%c", 
                                     areaDVH, frac, '%');

   if (!dvhLeg) {
      dvhLeg = new TLegend(0.001, 0.995, 0.372, 0.89);

      dvhLeg->AddEntry(DVH[curDVH], "", "l");
      dvhLeg->Draw();
   }

   TLegendEntry *entry = dvhLeg->GetEntry();

   if (entry) {
      entry->SetObject(DVH[curDVH]);
      entry->SetLabel(legTxt.Data());
   }

   dvhCanvas->Modified();
   dvhCanvas->Update();
}

//______________________________________________________________________________
void createDVHhist()
{
   //

   if (curDVH >= nDVH) {
      printf("Exeeds maximum number of DVHs = %d\n", curDVH);
      return;
   }

   if (histDVH[curDVH] == 0) {
      TString name ="dDSH";
      name += curDVH;

      TString title = "Differential Dose-Area Histogram, N = ";
      title += curDVH;
      histDVH[curDVH] = new TH1D(name.Data(), title.Data(), nbinsDVH, 0, dose2d->GetMaximum());

      histDVH[curDVH]->SetLineWidth(2);
      histDVH[curDVH]->SetLineColor(colorDVH[curDVH]);
      histDVH[curDVH]->SetFillColor(colorDVH[curDVH]);
      histDVH[curDVH]->GetXaxis()->SetTitle("Gy");
   } else {
     histDVH[curDVH]->Reset();
   }
  
   TAxis *xaxis = dose2d->GetXaxis();
   TAxis *yaxis = dose2d->GetYaxis();
   int nXbins = dose2d->GetNbinsX();
   int nYbins = dose2d->GetNbinsY();

   areaDVH = 0;

   for (int i = 0; i < nYbins; i++) {
      for (int k = 0; k < nXbins; k++) {
         Double_t x = xaxis->GetBinCenter(k);
         Double_t y = yaxis->GetBinCenter(i);

         if (!graphDVH[curDVH]->IsInside(x, y)) {
            continue;
         }

         Double_t value = dose2d->GetBinContent(k, i);
 
         areaDVH += xaxis->GetBinWidth(k)*yaxis->GetBinWidth(i);
         histDVH[curDVH]->Fill(value);
      }
   }

   createDVH();
}

//______________________________________________________________________________
void UpdateDVH()
{
   //

   createDVHhist();
   UpdateDvhPad();
   dvhCanvas->Modified();
   dvhCanvas->Update();

   if (impDvh) {
      impDvh->MapRaised();
   }
}

//______________________________________________________________________________
void createDVHgraph(Float_t xx, Float_t yy, Float_t width)
{
   // Creates square graph 

   const int subDiv = 2;
   const int nPoints = (subDiv)*4 + 1; //

   if (iDVH >= nDVH) {
      printf("Exeeds maximum number of DVHs = %d\n", iDVH);
      return;
   }

   Double_t *x = new Double_t[nPoints];
   Double_t *y = new Double_t[nPoints];

   //printf("xx = %f yy = %f\n", xx, yy);

   leftDVH = xx - width/2;
   rightDVH = xx + width/2;
   bottomDVH = yy - width/2;
   topDVH = yy + width/2;

   Float_t cury;
   Float_t curx;

   int idx = 0;
   curx = x[idx] = leftDVH + 1.; 
   cury = y[idx] = bottomDVH + width/subDiv;

   idx++;
   x[idx] = leftDVH + width/subDiv/2;
   y[idx] = bottomDVH + width/subDiv/2;

   //printf("x[%d] = %f, y[%d] = %f\n", idx, x[idx], idx, y[idx]);

   for (int i = 0; i < subDiv - 1; i++) {
      idx++;
      curx += width/subDiv;
      x[idx] = curx; y[idx] = bottomDVH + 1.1;
   }

   curx = rightDVH;
   cury = bottomDVH;

   idx++;
   x[idx] = rightDVH - width/subDiv/2;
   y[idx] = bottomDVH + width/subDiv/2;

   for (int i = 0; i < subDiv -1; i++) {
      idx++;
      cury += width/subDiv;
      x[idx] = rightDVH - 1; y[idx] = cury;
   }

   curx = rightDVH;
   cury = topDVH;

   idx++;
   x[idx] = rightDVH - width/subDiv/2;
   y[idx] = topDVH - width/subDiv/2;

   for (int i = 0; i < subDiv - 1; i++) {
      idx++;
      curx -= width/subDiv;
      x[idx] = curx; y[idx] = topDVH - 1;
   }

   curx = leftDVH;
   cury = topDVH;

   idx++;
   x[idx] = leftDVH + width/subDiv/2;
   y[idx] = topDVH - width/subDiv/2;

   for (int i = 0; i < subDiv - 1; i++) {
      idx++;
      cury -= width/subDiv;
      x[idx] = leftDVH + 1; y[idx] = cury;
   }

   x[idx - 1] -= 0.1; //

   TString name = "dvh";
   name += iDVH;
   graphDVH[iDVH] = new TCutG(name.Data(), nPoints, x, y);
   graphDVH[iDVH]->SetMarkerStyle(21);
   graphDVH[iDVH]->SetMarkerColor(colorDVH[iDVH]);
   graphDVH[iDVH]->SetLineWidth(2);
   graphDVH[iDVH]->SetLineColor(colorDVH[iDVH]);

   contz->cd();
   graphDVH[iDVH]->Draw("pc");

   curDVH = iDVH;
   createDVHhist();
   iDVH++;
}

//______________________________________________________________________________
void handleDVHevent(Int_t event, Int_t px, Int_t py, TObject *obj)
{
   //

   TString name = obj->GetName();

   if ((event == kButton1Up) && 
       (obj->IsA() == TCutG::Class()) &&
       name.BeginsWith("dvh")) {
      curDVH = atoi(name(3, 2).Data());
      UpdateDVH();
      return;
   }

   if (event != kButton1Down) {
      return;
   }

   TCanvas *c = (TCanvas *)gTQSender;
   TPad *pad = (TPad *)c->GetSelectedPad();

   if (!pad) {
      return;
   }

   if (obj->IsA() != TPolyMarker::Class()) {
     return;
   }

   if (iDVH == 0) {  // init arrays
      for (int i = 0; i < nDVH; i++) {
         graphDVH[i] = 0;
         DVH[i] = 0;
      }
   }

   float width = 8;  // in mm

   Float_t xx = pad->AbsPixeltoX(px);
   Float_t yy = pad->AbsPixeltoY(py);
   xx = pad->PadtoX(xx);
   yy = pad->PadtoY(yy);

   createDVHgraph(xx, yy, width);
}

//______________________________________________________________________________
void handleSurfEvent(Int_t event, Int_t px, Int_t py, TObject *obj)
{
   //

   if (event != kButton1ShiftMotion) {
      return;
   }

   if (obj->IsA() != TH2D::Class()) {
     return;
   }

   TH2D *h2 = (TH2D*)obj;
}

//______________________________________________________________________________
void handleCanvasEvent(Int_t event, Int_t px, Int_t py, TObject *obj)
{
   //

   if (event != kButton1Down) {
      return;
   }

   if (sliceCanvas == 0) {
      CreateSliceCanvas();
   }

   TCanvas *c = (TCanvas *)gTQSender;
   TPad *pad = (TPad *)c->GetSelectedPad();

   if (!pad) {
      return;
   }

   Float_t x = pad->AbsPixeltoX(px);
   Float_t y = pad->AbsPixeltoY(py);
   x = pad->PadtoX(x);
   y = pad->PadtoY(y);

   Int_t binx = dose2d->GetXaxis()->FindFixBin(x);
   Int_t biny = dose2d->GetYaxis()->FindFixBin(y);

   if (sliceX) {
      delete sliceX;
   }

   sliceX = dose2d->ProjectionX(TString::Format("Y = %f mm", y), biny, biny);
   sliceX->SetDirectory(0);   // do not write it to file
   sliceX->SetFillColor(4);
   sliceX->GetXaxis()->SetTitle("mm");
   sliceX->GetYaxis()->SetTitle("Gy");
   sliceX->SetTitle(TString::Format("Y = %f mm", y));
   sliceCanvas->cd(1);
   sliceX->Draw();

   if (sliceXrebin) {
      delete sliceXrebin;
   }

   // slice Y code
   if (sliceY) {
      delete sliceY;
   }

   sliceY = dose2d->ProjectionY(TString::Format("X = %f mm", y), binx, binx);
   sliceY->SetDirectory(0);   // do  not write it to file
   sliceY->SetFillColor(9);
   sliceY->SetTitle(TString::Format("X = %f mm", x));

   sliceY->GetXaxis()->SetTitle("mm");
   sliceY->GetYaxis()->SetTitle("Gy");
   sliceCanvas->cd(2);
   sliceY->Draw();

   if (sliceYrebin) {
      delete sliceYrebin;
   }

   sliceCanvas->Modified();
   sliceCanvas->Update();
}

//______________________________________________________________________________
TGCheckButton *findUpdateButton()
{
   // Finds and returns "Auto Update" button

   if (!palEditor) {
      return 0;
   }

   TListIter next(palEditor->GetList());

   TGCheckButton *ret = 0;
   TGFrameElement *fe = 0;
   TGCheckButton *chk = 0;
   TGTextButton *txt = 0;

   while ((fe = (TGFrameElement*)next())) {
      TGFrame *frame = fe->fFrame;
      if (frame->IsA() == TGCheckButton::Class()) {

         chk = (TGCheckButton*)frame;
         if (chk->GetString() == "Auto Update") {
            ret = chk;
            palEditor->HideFrame(ret);
         }
      } else if (frame->IsA() == TGTextButton::Class()) {
         txt = (TGTextButton*)frame;

         if ((txt->GetString() == "Apply") ||
             (txt->GetString() == "Ok") ||
             (txt->GetString() == "Cancel") ||
             (txt->GetString() == "Save") ||
             (txt->GetString() == "Open") ||
             (txt->GetString() == "New") ||
             (txt->GetString() == "Edit")) {

            palEditor->HideFrame(txt);
         }
      }
   }

   return ret;
}

//______________________________________________________________________________
TASPaletteEditor *findPaletteEditor()
{
   // Finds and returns TASPaletteEditor object

   TListIter next((const TList*)gClient->GetListOfWindows());

   TObject *obj;

   while ((obj = next())) {
      TString n = obj->GetName();
      if (n.Contains("ASPaletteEditor")) {
         return (TASPaletteEditor*)obj;
      }
   }

   return 0;
}

//______________________________________________________________________________
Long_t getModTime(const char *file)
{
   //  returns modification time of file as long value

   FileStat_t st;

   if (!gSystem->GetPathInfo(file, st)) {
      return st.fMtime;
   }

   return 0;
}

//______________________________________________________________________________
void doseHist(const char *file, const char *calibr = "PortraitRed.h")
{
   // read RAW data file and create hists from this

   gBenchmark->Start("doseHist");

   rootPath = gSystem->ExpandPathName("$(ROOTSYS)\\macros\\");
   TString calibrFile = rootPath + calibr;

   int notOK = gROOT->LoadMacro(calibrFile.Data()); 

   if (notOK) {
      fprintf(stderr, "Failed to load calibration file %s\n", calibrFile.Data());
      return;
   }
  
   int min, arrsz;
   Bool_t land = false;

   int ret = 0;

   if (land) {
      min = int(dose_vs_LR[0]);
      arrsz = sizeof(dose_vs_LR)/sizeof(double) - 1;
   } else {
      min = int(dose_vs_PR[0]);
      arrsz = sizeof(dose_vs_PR)/sizeof(double) - 1;
   }

   fileName = gSystem->BaseName(file);

   Bool_t tifile = kFALSE;
   TString rawfile;
   TString rootfileName = gSystem->BaseName(fileName.Data());

   // three possible extensions replace with .root
   rootfileName.ReplaceAll(".tiff", ".root");
   rootfileName.ReplaceAll(".tif", ".root");
   rootfileName.ReplaceAll(".raw", ".root");

   if (fileName.EndsWith(".tif", TString::kIgnoreCase) || 
      fileName.EndsWith(".tiff", TString::kIgnoreCase)) {
      tifile = kTRUE;

      rawfile = fileName;
      rawfile.ReplaceAll(".tiff", ".raw");
      rawfile.ReplaceAll(".tif", ".raw");

      TString tif2raw = rootPath;
      tif2raw += "tif2raw.exe -f ";
      tif2raw += file;

      printf("Convert tif file to raw: %s\n", tif2raw.Data());
      printf("Input tif file -%s, output raw file -%s\n", file, rawfile.Data());
      gSystem->Exec(tif2raw);
   }

   printf("Openning raw file %s\n", rawfile.Data());

   FILE *fp = fopen(rawfile.Data(), "rb");

   if (!fp) {
      fprintf(stderr, "Failed to open file: %s", rawfile.Data());
      return;
   }

   fread(&width, sizeof(width), 1, fp);
   fread(&height, sizeof(height), 1, fp);

   long sz = width*height*3;
   unsigned short *rgb = new unsigned short[sz];
   unsigned short r, g, b;

   fread(rgb, sizeof(short), sz, fp);
   fclose(fp);

   // delete temporary file
   if (!gSystem->AccessPathName(rawfile.Data())) {
      gSystem->Unlink(rawfile.Data());
   }

   TString title = "Dose distribution of ";
   title += fileName;

   TString name = "colz_";
   name += fileName;

   // open ROOT file to save all hists 
   rootFile = new TFile(rootfileName, "RECREATE");

   // create subdirs
   TDirectoryFile *canvasDir = new TDirectoryFile("canvasDir", "Canvases");
   TDirectoryFile *rgbDir = new TDirectoryFile("rgbDir", "RGB components");
   TDirectoryFile *calibrDir = new TDirectoryFile("calibrDir", "Calibrations");

   Double_t dpi = 1./imgDpi;
   dose2d = new TH2D(name, title, width, 0, width*dpi, height, 0, height*dpi);
   dose2d->SetStats(0); //delete stats box

   name = "1D_";
   name += fileName;

   gDirectory->cd("rgbDir");
   redHist = new TH2D("red (do not click/open)", "Red component", width, 0, width, height, 0, height);
   greenHist = new TH2D("green (do not click/open)", "Green component", width, 0, width, height, 0, height);
   blueHist = new TH2D("blue (do not click/open)", "Blue component", width, 0, width, height, 0, height);
   gDirectory->cd("/");

   dose1d = new TH1D(name, title, 10000, -1, 20);

   Double_t value;
   long j = 0;
   long idx;
   Double_t *values = new Double_t[width*height];
   long k = 0;

   gBenchmark->Start("Filling");

   notOK = gROOT->LoadMacro("doseConfig.h"); 
   //printf("%d %f\n", notOK, gPedestal);

   if (!notOK && gPedestal > 0) {
      pedestal = gPedestal;
      calcPed = kFALSE;
      printf("\n\n\t\tLoaded Pedestal = %f\n", pedestal);
   }

   if (calcPed) {
      //1.5185090619216725
      Double_t pedK = 5.979e-9; //1.0e-8; // Gy/sec 
      // time of the last calibration P:\Upl\Shemyakov\Dosa\Obninssk\18.11.2014_calibr_po_Dubne\V700\for_calibration_v700_EBT2
      Long_t date0 = 1416405783; 

      // calculate pedestal correction
      Double_t corrPed = 100000; // huge dose
   
      for (int y = 0; y < height; y++) {
         for (int x = 0; x < width; x++) {
            r = rgb[j];

            idx = (r - min) > arrsz ? arrsz : r - min;

            idx = idx < 1 ? 1 : idx; 

            value = land ? dose_vs_LR[idx] : dose_vs_PR[idx];
            corrPed = value < corrPed && value > 0 ? value : corrPed;
            
            g = rgb[++j];
            b = rgb[++j];
            j++;
         }
      }
/*
      Long_t date = getModTime(file);
      Long_t dt = date - date0;
  
      dt = dt == 0 ? 2147483647 : dt; // zero sanity check

      pedestal = dt*pedK;

      Double_t newK = corrPed/dt;
      Double_t newP = dt*newK;

      if (dt < 0) {
         printf("\n\tФайл был создан до калибровки!\n\tПедестал неправильный\n\n\n");
      } else { 
         printf("\t\tCorrected Pedestal = %f\n", corrPed);
         printf("\n\n\told ped = %f new ped =%f %d %d dt=%d k=%E\n\n\n", 
               pedestal, newP, date, date0, dt, newK);
      }
*/
      //if (corrPed < pedestal && corrPed > 0) {
         pedestal = corrPed;
      //}
   }

   if (isDoseFile(file)) { // 
      //pedestal = 0;
   }

   printf("\n\n\t\tPedestal = %f\n\n\n", pedestal);

   j = 0;
   idx = 0;
   k = 0;
   
   for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
         r = rgb[j];

         idx = (r - min) > arrsz ? arrsz : r - min;

         if (idx < 1) {
            printf("x=%d y=%d r=%d\n", x, y, r);
         }

         idx = idx < 1 ? 1 : idx; 

         value = land ? dose_vs_LR[idx] : dose_vs_PR[idx];
         dose1d->Fill(value);
         
         // subtract pedestal
         value -= pedestal;
         //value = value < 0 ? 1e-12 : value; // do not use zero doses to allow log Y scale

         g = rgb[++j];
         b = rgb[++j];

         redHist->SetBinContent(x, y, r);
         greenHist->SetBinContent(x, y, g);
         blueHist->SetBinContent(x, y, b);

         dose2d->SetBinContent(x, height - y - 1, value);

         k = (height - y - 1)*width + x;
         values[k] = value;
         j++;
      }
   }

   gBenchmark->Show("Filling");

   signalCanvas = new TCanvas("Signals", "Raw signals");

   signalCanvas->Draw();
   signalCanvas->ToggleEventStatus();
   signalCanvas->SetLogy();
   dose1d->GetXaxis()->SetTitle("Gy");
   dose1d->SetFillColor(44);
   dose1d->Draw();
   
   myImage = new TASImage("2D dose", values, width, height);
   myImage->SetConstRatio(kTRUE);
   myImage->Draw("N");

   imageCanvas = (TCanvas*)gROOT->GetListOfCanvases()->FindObject(myImage->GetName());

   if (imageCanvas) { 
      imageCanvas->ToggleEventStatus();
      imageCanvasImp = (TRootCanvas*)imageCanvas->GetCanvasImp();
   }

   myImage->StartPaletteEditor();
   palEditor = findPaletteEditor(); //
   palEditor->SetWindowName("1D dose distribution in Gy");
   updateBtn = findUpdateButton();

   if (updateBtn) {
      updateBtn->Toggle();
   }

   palCanvas = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("data hist");

   if (palCanvas) { 
      palCanvas->SetLogy();
      palCanvas->SetGridx();
      palCanvas->SetGridy();
      palHist = (TH1D*)palCanvas->GetListOfPrimitives()->FindObject("Statistics");
   }

   if (palHist) {
      palHist->GetXaxis()->SetTitle("Gy");
      palHist->SetFillColor(2);
   }

   delete [] rgb;

   //
   gStyle->SetCanvasPreferGL();
   gStyle->SetOptStat(1001211);

   title = "2D dose ";
   title += fileName;

   UInt_t ww = width*2 - 28;
   UInt_t hh = height*2 - 18;

   if (ww > 1000) {
      ww = ww/2;
      hh = hh/2;

      if (ww > 1000) {
         ww = ww/2;
         hh = hh/2;
      }
   }

   colz = new TCanvas(fileName + " colz", title + " colz in Gy", ww, hh);
   colz->SetGridx();
   colz->SetGridy();
   colz->SetTickx();
   colz->SetTicky();

   colz->ToggleEventStatus();
   dose2d->SetTitle("Dose distribution of " + fileName + " in Gy");
   dose2d->Draw("colz");
   dose2d->GetXaxis()->SetTitle("mm");
   dose2d->GetYaxis()->SetTitle("mm");
   colz->SetEditable(kFALSE);

   colz->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", 0, 0, 
                 "handleCanvasEvent(Int_t,Int_t,Int_t,TObject*)");

   int rebin = (int)imgDpi;
   
   dose2rebin = dose2d->Rebin2D(rebinFactor, rebinFactor, "rebinXY");
   dose2rebin->SetStats(0); //delete stats box
   dose2rebin->SetName("iso_" + fileName);
   dose2rebin->GetXaxis()->SetTitle("mm");
   dose2rebin->GetYaxis()->SetTitle("mm");

   Double_t dmax = dose2rebin->GetMaximum();

   //contz->SetEditable(kFALSE);

   dose2surf = (TH2*)dose2rebin->Clone("dose2surf");
   dose2surf->SetDirectory(0);
   surf = new TCanvas(title + " surf", title + " surf in cGy");
   dose2surf->Draw("glsurf2");
   dose2surf->GetXaxis()->SetTitle("");
   dose2surf->GetYaxis()->SetTitle("");
   dose2surf->GetZaxis()->SetTitle("");
   surf->SetGridx();
   surf->SetGridy();
   surf->SetTickx();
   surf->SetTicky();
   surf->ToggleEventStatus();
   surf->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", 0, 0, 
                 "handleSurfEvent(Int_t,Int_t,Int_t,TObject*)");

   //dose1dCanvas->SetEditable(kFALSE);

   gStyle->SetCanvasPreferGL(0);

   spectrum = new TSpectrum2(10);
   TCanvas *tmp = new TCanvas();
   Int_t nfound = spectrum->Search(dose2rebin, 4, "cont1z noMarkov");
   delete tmp;
   tmp = 0;
   printf("%d peaks found\n", nfound);

   xpeaks = spectrum->GetPositionX();
   ypeaks = spectrum->GetPositionY();

   contz = new TCanvas(title + " contz", title + " contz", ww, hh);
   contz->SetGridx();
   contz->SetGridy();
   contz->SetTickx();
   contz->SetTicky();
 
   contz->ToggleEventStatus();
   contz->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", 0, 0, 
                  "handleDVHevent(Int_t,Int_t,Int_t,TObject*)");

   Double_t dmaxRB = dose2rebin->GetMaximum();

   dose2rebin->Scale(100/dmaxRB);

   TString dose2rebinTitle = TString::Format("Isodose field of %s in percents (100%c = %f Gy)", 
                                             fileName.Data(), '%', dmaxRB/(rebinFactor*rebinFactor));
   dose2rebin->SetTitle(dose2rebinTitle);

   contz->cd();
   dose2rebin->Draw("CONT1 Z LIST");

   TList *contLevel = 0;
   TGraph *curv     = 0;
   Double_t x0, y0, z0;
   TLatex txt;
   TString val;

   const int kNconts = 10;
   Double_t contours[kNconts];
   Double_t delta = 100./(kNconts);

   for (int i = 0; i < kNconts; i++) {
      contours[i] = (i + 1)*delta;
   }

   dose2rebin->SetContour(kNconts, contours);
   //dose2d->SetContour(kNconts, contours);

   tmp = new TCanvas("tmp_contz", "tmp_contz", ww, hh);
   tmp->Draw();
   dose2rebin->Draw("CONT Z LIST");

   tmp->Update();
   delete tmp;
   tmp = 0;

   TObjArray *conts = (TObjArray*)gROOT->GetListOfSpecials()->FindObject("contours");
   Int_t totConts = 0;

   if (conts == 0){
      printf("*** No contours were extracted!\n");
      totConts = 0;
      goto peaks;
   } else {
      totConts = conts->GetSize();
   }
   //printf("Total contours = %d\n", totConts);

   contz->cd();

   for (int i = 0; i < totConts - 1; i++) {
      contLevel = (TList*)conts->At(i);
      curv = (TGraph*)contLevel->First();

      for (j = 0; j < contLevel->GetSize(); j++) {
         curv->GetPoint(i, x0, y0);
         z0 = contours[i];
         val = TString::Format("%2.0f", z0);

         txt.SetTextSize(0.02);
         txt.DrawLatex(x0, y0, val.Data());
         curv = (TGraph*)contLevel->After(curv);  // get next graph
      }
   }

   // disable displaying peaks
   nfound = 0;
   peakCanvas = 0;

peaks:
   if (nfound >= 1) {
      //peakCanvas =  new TCanvas(name + " slices", name + " silces", 1024, 800);
      //peakCanvas->SetCrosshair();
      //peakCanvas->ToggleEventStatus();

      //peakCanvas->Divide(2, nfound);
      //peakCanvas->cd();


      for (int i = 0; i < nfound; i++) {
         //peakCanvas->cd(i+1);

         printf("x = %f y = %f\n", xpeaks[i], ypeaks[i]);

         TString prname = TString::Format("Y[%d] slice at X = ", i);

         peakCanvas = new TCanvas(prname);
         gPad->SetGridx();
         gPad->SetGridy();
         gPad->SetTickx();
         gPad->SetTicky();
         peakCanvas->ToggleEventStatus();

         int xbin = int(xpeaks[i]*imgDpi);
         printf("i = %d xbin = %d\n" , i, xbin);

         yproj = dose2d->ProjectionY(TString::Format("X = %f mm", xpeaks[i]), xbin, xbin, "d");
         yproj->SetDirectory(0);
         yproj->SetFillColor(9);
         yproj->SetTitle(prname + TString::Format(" %f mm", xpeaks[i]));
         yproj->GetXaxis()->SetTitle("mm");
         yproj->GetYaxis()->SetTitle("Gy");
         //peakCanvas->SetEditable(kFALSE);

         prname = TString::Format("X[%d] slice at Y = ", i);

         peakCanvas = new TCanvas(prname);
         gPad->SetGridx();
         gPad->SetGridy();
         gPad->SetTickx();
         gPad->SetTicky();
         peakCanvas->ToggleEventStatus();

         int ybin = int(ypeaks[i]*imgDpi);
         printf("i = %d ybin = %d\n" , i, ybin);

         xproj = dose2d->ProjectionX(TString::Format("Y = %f mm", ypeaks[i]), ybin, ybin, "d");
         xproj->SetDirectory(0);
         xproj->SetFillColor(4);
         xproj->SetTitle(prname + TString::Format(" %f mm", ypeaks[i]));

         xproj->GetXaxis()->SetTitle("mm");
         xproj->GetYaxis()->SetTitle("Gy");
         //peakCanvas->SetEditable(kFALSE);
      }
   }

   if (peakCanvas) {
      peakCanvas->Update();
   }

   gDirectory->cd("canvasDir");
   colz->Write();
   contz->Write();

   if (dose1dCanvas) {
      dose1dCanvas->Write();
   }

   gDirectory->cd("/");

   gDirectory->cd("calibrDir");
   TString macroName = rootPath + "PortraitRedCalibr.C";
   gROOT->Macro(macroName.Data());   // temporary solution
   gPad->Write();
   gDirectory->cd("/");

   rootFile->Write();

   gStyle->SetOptStat(11111111);

   printf("Stop\n");

   printf("\n\n==========================================================================\n");

   gSystem->Sleep(300);
   TRootCanvas *impColz = (TRootCanvas*)colz->GetCanvasImp();

   if (impColz) {
      impColz->MapRaised();
      impColz->SetWMPosition(700, 100);
   }

   if (palEditor) {
      palEditor->SetWMSize(800, 500);
      palEditor->ProcessMessage(MK_MSG(kC_COMMAND, kCM_BUTTON), 11, 0);
      palEditor->ProcessMessage(MK_MSG(kC_COMMAND, kCM_BUTTON), 1, 0);
   }

   gBenchmark->Show("doseHist");

/*
   entranceCanvas = new TCanvas("EntranceCanvas", "Entrance X beam profile");
   entranceX = dose2d->ProjectionX("EntranceProfileX", 2, 2, "d");
   entranceX->SetDirectory(0);
   entranceX->Draw();
*/

   braggCanvas = new TCanvas("BraggCanvas", "Y projection (Bragg peak)");
   braggCanvas->cd();

   projBragg = dose2d->ProjectionY("BraggPeak", 0, -1, "");
   projBragg->SetDirectory(0);
   projBragg->SetFillColor(31);

   //projBragg->Multiply(projBragg);
   //projBragg->Scale(1.0/dose2d->GetNbinsX());
 
   projBragg->Draw();


   printf("\n\nTo find total intensity call function Intensity(Energy(MeV)) from command prompt\n");
}

//______________________________________________________________________________
double Intensity(int energy = 100)
{
   // calculates total proton beam intensity based on total dose field integral

   printf("\n\n==========================================================================\n");
   double rho = 1.2; //0.82; //1.05;
   double iddp = imgDpi*imgDpi*100; // (pixels per centimeter)**2
   double maxE = dose2d->GetMaximum();
   double bgnd = dose2d->GetNbinsX()*dose2d->GetNbinsY()*iddp*0.17;//dose2d->GetMinimum();
   bgnd = 0;
   //printf("Background = %E\n", bgnd);

   double halfE = maxE*0.5;
   TAxis *ax = dose1d->GetXaxis();
   int b2 = ax->FindBin(maxE);
   int b1 = ax->FindBin(halfE);
   printf("Full integral = %E.    Half integral = %E\n", dose2d->Integral(), dose1d->Integral(b1, b2));

   // dEdx_ICRU49 array - stopping power of protons in water in Mev/mm
   double intensity = dose2d->Integral()/iddp/dEdx_ICRU49[energy]/10.*6.24e9*rho - bgnd;
   printf("Total Intensity = %E at proton Energy = %d (MeV)\n", intensity, energy);
   
   return intensity;
}
