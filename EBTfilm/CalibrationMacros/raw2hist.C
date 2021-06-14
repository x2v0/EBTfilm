// $Id: raw2hist.C 3406 2015-06-26 06:13:58Z onuchin $
// Author: Valeriy Onuchin   28.07.2011

/*************************************************************************
 *                                                                       *
 * Copyright (C) 2011,   Valeriy Onuchin                                 *
 * All rights reserved.                                                  *
 *                                                                       *
 *************************************************************************/

#include <stdio.h>
#include "TString.h"
#include "TH1F.h"
#include "TFile.h"
#include "TNtuple.h"
#include "TStyle.h"
#include "TF1.h"
#include "TMath.h"
#include "TROOT.h"


TString rtitle = "Red color hist ";
TString gtitle = "Green color hist ";
TString btitle = "Blue color hist ";

TH1F *red = 0;
TH1F *green = 0;
TH1F *blue = 0;
TNtuple *tuple = 0;
TFile *rootfile = 0;
TString doseFname;
int nullDoseChannel = 0;
int minDoseChannel = 0;
Bool_t nullDoseProcessing = false;
Double_t minDose = 0.05;   // hardcoded
Double_t nullDose = 0.05;
Bool_t minDoseProcessing = false;
int nFileProcessed = 0;


//___________________________________________________________________
float GetDoseFromName(const char *fname)
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

   return dose;
}

//______________________________________________________________________________
void raw2hist(const char *file)
{
   // read RAW data file and create hists

   TString fname = file;

   FILE *fp = fopen(file, "rb");

   if (!fp) {
      fprintf(stderr, "Failed to open file: %s", file);
      return;
   }

   printf("Processing file - %s ", file);

   UInt_t w, h;

   fread(&w, sizeof(w), 1, fp);
   fread(&h, sizeof(h), 1, fp);

   long sz = w*h*3;
   unsigned short *rgb = new unsigned short[sz];

   fread(rgb, sizeof(short), sz, fp);
   fclose(fp);

   TString str;
   int orient = fname.BeginsWith("L") ? 0 : 1; // landscape - 0, portrait - 1

   if (!rootfile) {
      if (orient) {
         doseFname = "dosePortrait.root";
      } else {
         doseFname = "doseLandscape.root";
      }
      rootfile = new TFile(doseFname, "RECREATE");
   }

   float dose = GetDoseFromName(fname);
 
   printf("dose = %f\n", dose);
   nFileProcessed++;

   nullDoseProcessing = false;

   if (dose < nullDose) { // null dose
      dose = nullDose;
      nullDoseProcessing = true;
   }

   minDoseProcessing = false;

   if (nFileProcessed == 2) {  // hardcode min dose
      minDoseProcessing = true;
   }

   str = rtitle;
   str += TString::Format("(%3.1f", dose);
   str += " Gy)";
   red = new TH1F(fname + " red", str.Data(), 1000, 0, 66000); // 65536
   red->SetFillColor(2);

   str = gtitle;
   str += TString::Format("(%3.1f", dose);
   str += " Gy)";
   green = new TH1F(fname + " green", str.Data(), 1000, 0, 66000);
   green->SetFillColor(3);

   str = btitle;
   str += TString::Format("(%3.1f", dose);
   str += " Gy)";
   blue = new TH1F(fname + " blue", str.Data(), 1000, 0, 66000);
   blue->SetFillColor(4);

   if (!tuple) { 
      tuple = new TNtuple("dose", "Dose vs 16 bit color",
                          "dose:r:g:b:sr:sg:sb:mr:mg:mb");
   }

   Float_t r, g, b;
   long n = w*h;
   long j = 0;

   for (long i = 0; i < n; i++) {
      r = rgb[j];
      red->Fill(r);

      g = rgb[++j];
      green->Fill(g);

      b = rgb[++j];
      blue->Fill(b);

      j++;
   }

   TF1 *fblue = new TF1("gaus", "gaus", blue->GetMean() - blue->GetRMS(),
                        blue->GetMean() + blue->GetRMS());
   fblue->SetLineColor(1);
   blue->Fit("gaus", "Q");

   TF1 *fgreen = new TF1("gaus", "gaus", green->GetMean() - green->GetRMS(),
                        green->GetMean() + green->GetRMS());
   fgreen->SetLineColor(1);
   green->Fit("gaus", "Q");

   TF1 *fred = new TF1("gaus", "gaus", red->GetMean() - red->GetRMS(),
                       red->GetMean() + red->GetRMS());
   fred->SetLineColor(1);
   red->Fit("gaus", "Q");

   if (nullDoseProcessing) {
      nullDoseChannel = (Int_t)fred->GetParameter(1);
      printf("null dose channel = %d\n", nullDoseChannel);
   }

   if (minDoseProcessing) {
      minDoseChannel = (Int_t)fred->GetParameter(1);
      printf("minimum dose = %f, channel = %d\n", minDose, minDoseChannel);
   }

   gStyle->SetOptFit(111);
   gStyle->SetOptStat(1111111);

   tuple->Fill(dose, (Float_t)fred->GetParameter(1),
              (Float_t)fgreen->GetParameter(1), (Float_t)fblue->GetParameter(1),
              (Float_t)fred->GetParameter(2), (Float_t)fgreen->GetParameter(2),
              (Float_t)fblue->GetParameter(2), (Float_t)red->GetMean(), 
              (Float_t)green->GetMean(), (Float_t)blue->GetMean());


   delete [] rgb;

   //delete red;
   //delete green;
   //delete blue;
}