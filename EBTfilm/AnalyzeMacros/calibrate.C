// $Id: calibrate.C 3426 2015-06-30 11:17:44Z onuchin $
// Author: Valeriy Onuchin   07.08.2011

/*************************************************************************
 *                                                                       *
 * Copyright (C) 2011,   Valeriy Onuchin                                 *
 * All rights reserved.                                                  *
 *                                                                       *
 *************************************************************************/

#include <stdio.h>
#include "TString.h"
#include "TGraph.h"
#include "TNtuple.h"
#include "TFile.h"
#include "TMath.h"
#include "TGraphErrors.h"
#include "TMultiGraph.h"
#include "TSpline.h"
#include "TAxis.h"
#include "TCanvas.h"

#include "draw2hist.C"

Bool_t landscapeOrientation = kFALSE;
TString calibrIncludeCal = "PortraitRed.h";
TString calibrMacroCal = "PortraitRedCalibr.C";

TGraph *gr = 0;
TGraph *gg = 0;
TGraph *gb = 0;

TFile *calibrFileCal = 0;
TF1 *fitFuncCal = 0;
TTree *doseTupleCal = 0;
TVirtualPad *redPad = 0;

// auxilary data for second correction of calibration
Int_t nentries = 0;
Double_t *dosesCal = 0;
Double_t *redsCal = 0;
TNtuple *treeCal = 0;
Int_t *indexCal = 0;

// final calibration array 
int arrSize = 65536; // 2^16
Double_t *calibArrayCal = new Double_t[arrSize + 1];
TGraph *corrGraphCal = 0;
Double_t *xCorrCal = 0;
Double_t *yCorrCal = 0;
TCanvas *corrCanvasCal = 0;

int fitFuncCalNpar = 0;

//______________________________________________________________________________
void cleanupRawFiles(const char *dirname = ".")
{
   // cleanup .raw files

   const char *entry;
   TString file; 
   void *dir = gSystem->OpenDirectory(dirname);

   if (dir) {
      while (entry = gSystem->GetDirEntry(dir)) {
         file = entry;
         if ((file == ".") || (file == "..")) {
            continue;
         }

         if (file.EndsWith(".raw", TString::kIgnoreCase)) {
            gSystem->Unlink(file.Data());
         }
      }

      gSystem->FreeDirectory(dir);
   }
}

//______________________________________________________________________________
void calibrate(TString iniLetter = "P", Bool_t saveFile = kTRUE)
{
   // creates graphs with dose vs color

   // converts tif files to raw files
   dtif2raw();

   TString fname;
   landscapeOrientation = iniLetter.BeginsWith("L");

   // converts raw files to hist file
   draw2hist();

   if (landscapeOrientation) {
      fname = "doseLandscape.root"; // defined in raw2hist.C
   } else {
      fname = "dosePortrait.root";  // defined in raw2hist.C
   }
 
   TFile *f = TFile::Open(fname, "READ");
   
   if (!f) {
      printf("Failed to open file - %\n", fname.Data());
      return;
   } else {
      printf("Processing file - %s ...\n", fname.Data());
   }

   TString tname = "dose";
   treeCal = (TNtuple*)f->Get(tname);
   nentries = (Int_t)treeCal->GetEntries();
   treeCal->Draw("dose:r", "", "goff");
   indexCal = new Int_t[nentries];

   // sort in the increasing order of red channel
   TMath::Sort(nentries, treeCal->GetV2(), indexCal, kFALSE);
   doseTupleCal = (TTree*)treeCal->Clone();

   for (Int_t i = 0; i < nentries; i++) {
      doseTupleCal->GetEntry(indexCal[i]);
      doseTupleCal->Fill();
   }

   TString title;
   TString name;

   if (landscapeOrientation) {
      fname = "calibrLandscape.root";
      title = "Landscape, ";
      name = "land_";
   } else {
      fname = "calibrPortrait.root";
      title = "Portrait, ";
      name = "port_";
   }

   if (saveFile) {
      calibrFileCal = new TFile(fname, "RECREATE");

      if (calibrFileCal) {
         printf("File %s created\n", fname.Data());
      } else {
         printf("Filed to create file %s\n", fname.Data());
         return;  
      }
   }

   doseTupleCal->Draw("log10(dose):b", "", "lp*");
   gb =(TGraph*)gPad->GetPrimitive("Graph");
   gb->SetName(name + "blue");
   gb->SetTitle(title + "blue");
   gb->SetLineColor(4);

   if (saveFile) {
      calibrFileCal->Add(gb);
      calibrFileCal->Write();
   }

   doseTupleCal->Draw("log10(dose):g", "", "lp*");
   gg = (TGraph*)gPad->GetPrimitive("Graph");
   gg->SetName(name + "green");
   gg->SetTitle(title + "green");
   gg->SetLineColor(3);

   if (saveFile) {
      calibrFileCal->Add(gg);
      calibrFileCal->Write();
   }

   TString cname; 
   TDatime now;

   doseTupleCal->Draw("log10(dose):r", "", "lp*");
   gr = (TGraph*)gPad->GetPrimitive("Graph");

   gr->SetName(name + "red");
   gr->SetTitle(title + "red");
   gr->SetLineColor(2);
   gr->SetMarkerStyle(21);

   TNamed *htemp = (TNamed*)gROOT->FindObject("htemp");

   if (htemp) {
      cname = gSystem->BaseName(gSystem->pwd());
      cname += " ";
      cname += now.GetDate();
      htemp->SetTitle(cname.Data());
   }

   if (saveFile) {
      calibrFileCal->Add(gr);
      calibrFileCal->Write();
      //calibrFileCal->Close();
   }

   gPad->Modified();
   gPad->Update();
   redPad = gPad;

   cname = "Calibration of "; 
   cname += gSystem->BaseName(gSystem->pwd());
   cname += " ";
   cname += now.AsString();
   redPad->SetTitle(cname.Data());

   //delete calibrFileCal;  // calibrFileCal defined in "raw2hist.C"
   //calibrFileCal = 0;

   cleanupRawFiles();

   // init vars for  second iteration 
   doseTupleCal->Draw("dose:r", "", "goff");
   dosesCal = doseTupleCal->GetV1();
   redsCal = doseTupleCal->GetV2();
}

//______________________________________________________________________________
void CopyCalibrFiles()
{
   // copy calibr files to $ROOTSYS/macros directory and save previous files

   TString rootsys = gSystem->Getenv("ROOTSYS");

   if (rootsys.IsNull()) {
      return;
   }

   TString dirname = gSystem->ConcatFileName(rootsys.Data(), "macros");
   TString calfile = gSystem->ConcatFileName(dirname, calibrIncludeCal.Data());
   
   FILE *fp = fopen(calfile.Data(), "rt");

   if (!fp) {
      printf("Failed to open file %s\n", calfile.Data());
      return;
   }

   char str[1024];
   TString prefix = fgets(str, sizeof(str), fp);
   fclose(fp);

   prefix.ReplaceAll("//", "");
   prefix.ReplaceAll(" ", "");
   prefix.Chop();

   TString savefile = calfile;
   savefile += "." + prefix;

   printf("Copying file %s to %s in the directory %s\n\n", calfile.Data(), savefile.Data(), dirname.Data());
   int ret = gSystem->CopyFile(calfile.Data(), savefile.Data(), kTRUE);

   if (ret < 0) {
      printf("Failed to copy file %s to %s, error code %d\n\n", calfile.Data(), savefile.Data(), ret);
   }

   calfile = gSystem->ConcatFileName(dirname, calibrMacroCal.Data());
   savefile = calfile;
   savefile += "." + prefix;

   printf("Copying file %s to %s in the directory %s\n\n", calfile.Data(), savefile.Data(), dirname);
   ret = gSystem->CopyFile(calfile.Data(), savefile.Data(), kTRUE);

   if (ret < 0) {
      printf("Failed to copy file %s to %s, error code %d\n\n", calfile.Data(), savefile.Data(), ret);
   }

   calfile = calibrMacroCal;
   savefile = gSystem->ConcatFileName(dirname, calibrMacroCal.Data());

   printf("Copying file %s to %s\n\n", calfile.Data(), savefile.Data());
   ret = gSystem->CopyFile(calfile.Data(), savefile.Data(), kTRUE);

   if (ret < 0) {
      printf("Failed to copy file %s to %s, error code %d\n\n", calfile.Data(), savefile.Data(), ret);
   }

   calfile = calibrIncludeCal;
   savefile = gSystem->ConcatFileName(dirname, calibrIncludeCal.Data());

   printf("Copying file %s to %s\n\n", calfile.Data(), savefile.Data());
   ret = gSystem->CopyFile(calfile.Data(), savefile.Data(), kTRUE);

   if (ret < 0) {
      printf("Failed to copy file %s to %s, error code %d\n\n", calfile.Data(), savefile.Data(), ret);
   }
}

//______________________________________________________________________________
void PreCorrectCalibr()
{
   // prelimenary correction of calibration coeffcients

   xCorrCal = new Double_t[nentries];
   yCorrCal = new Double_t[nentries];
   int xmin = int(redsCal[indexCal[0]]);
   int xmax = int(redsCal[indexCal[nentries - 1]]);

   for (int i = 0; i < nentries; i++) {
      xCorrCal[i] = redsCal[indexCal[i]];
      Double_t dose = dosesCal[indexCal[i]];
      Double_t dose2 = calibArrayCal[int(xCorrCal[i])];
      yCorrCal[i] = dose2 > 0.0005 ? dose/dose2 : 1;  // zero protection 
      printf("Correct factor x[%d] = %f, y[%d] = %f\n", i, xCorrCal[i], i, yCorrCal[i]);
   }

   corrGraphCal = new TGraph(nentries, xCorrCal, yCorrCal);
   corrGraphCal->SetName("corrGraphCal");
   corrGraphCal->SetTitle("Correction factors");
   corrCanvasCal = new TCanvas("corrCanvasCal", "Correction factors");
   corrGraphCal->Draw("alp*");

   for (int i = xmin; i < xmax; i++) {
      calibArrayCal[i] = calibArrayCal[i]*corrGraphCal->Eval(i);
   }
}

//______________________________________________________________________________
void WriteCalibrFile()
{
   // create calibration file

   TString calibrFileCalName = "PortraitRed";

   if (landscapeOrientation) {
      calibrFileCalName = "LandscapeRed";
   } else {
      calibrFileCalName = "PortraitRed";
   }

   calibrIncludeCal = calibrFileCalName + ".h";
   calibrMacroCal = calibrFileCalName + "Calibr.C";

   // write data to file
   FILE *fp = fopen(calibrIncludeCal.Data(), "w");

   TDatime now;
   fprintf(fp, "// %d.%s\n// This file was automatically generated by calibrate.C macros\n// %s based on the data from %s\n//\n", 
           now.GetDate(), gSystem->BaseName(gSystem->pwd()), now.AsString(), gSystem->pwd());
   fprintf(fp, "// Fit parameters for log10(Dose):\n");
 
   for (int i = 0; i < fitFuncCalNpar; i++) {
     fprintf(fp, "//   Par%3d  %20s = %g\n", i, fitFuncCal->GetParName(i), fitFuncCal->GetParameter(i));
   }

   fprintf(fp, "\n\n");

   fprintf(fp, "double dose_vs_PR[%d]={\n", arrSize);

   for(int i = 0; i < arrSize ; i++) {
      if ((i%1000 == 0)) {
         printf("%d = %f\n", i, calibArrayCal[i]);
      }

      if (i < arrSize - 1) {
         fprintf(fp, "   %1.10f, //%d\n", calibArrayCal[i], i);
      } else {
         fprintf(fp, "   %1.10f}; //%d\n", calibArrayCal[i], i);
      }
   }

   printf("___________________________________________________________________\n");

   for (int i = 0; i < nentries; i++) {
      int idx = int(redsCal[indexCal[i]]);
      printf("%d = %2.2f\n", idx, calibArrayCal[idx]);
   }

   fclose(fp);
}

//______________________________________________________________________________
void CreateCalibrFile()
{
   // creates the final include file with calibration data - PosrtraitRed.h

   fitFuncCal = (TF1*)gROOT->GetListOfFunctions()->FindObject("PrevFitTMP");

   if (!fitFuncCal) {
      printf("You need to fit the calibration graph first!\n");
      return;
   }

   fitFuncCalNpar = fitFuncCal->GetNpar();
  
   for(int i = arrSize - 1; i >= 0; i--) {
      calibArrayCal[i] = TMath::Exp(fitFuncCal->Eval(Double_t(i))*2.3025850929940459);

      if (calibArrayCal[i] > 500) {
          calibArrayCal[i] = 500;
      }

      if ((i < arrSize - 2) && (calibArrayCal[i] <= calibArrayCal[i + 1])) {
         calibArrayCal[i] = calibArrayCal[i + 1];
      }
   }

   PreCorrectCalibr();
   WriteCalibrFile();

   redPad->SaveAs(calibrMacroCal.Data());

   //CopyCalibrFiles();
}