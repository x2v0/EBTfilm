// $Id: dirscan.C 2109 2014-03-02 14:02:46Z onuchin $
// Author: Valeriy Onuchin  02.03.2014

/*************************************************************************
 *                                                                       *
 * Copyright (C) 2014,   Valeriy Onuchin                                 *
 * All rights reserved.                                                  *
 *                                                                       *
 *************************************************************************/

#include <stdio.h>
#include "TSystem.h"
#include "TString.h"
#include "TROOT.h"
#include "TFile.h"
#include "TH2.h"

TString result_dir = "";
TFile *rootFile = 0;
const Double_t background = 0.00;  // background in Gy (fake:)

//___________________________________________________________________
void processFile(const char *filename)
{
   // read ROOT file extract 2D dose hist and save it as 

   TString histname = gSystem->BaseName(filename);
   int idx = histname.Index(".root");
   TString outname;

   if (idx != kNPOS) {
      outname = histname = histname(0, idx);
      histname = "colz_" + histname + ".tif;1";
      //printf("%s\n", histname.Data());
   }

   printf("Processing file %s\n", filename);

   if (gSystem->AccessPathName(filename)) {
      printf("Failed to read file %s\n", filename);
      return;
   }

   // open ROOT file
   rootFile = new TFile(filename, "READ");

   TH1 *hist = (TH2*)rootFile->Get(histname.Data());

   if (!hist) {
      printf("Failed to read histogram %s\n", histname.Data());
      goto out;
   }

   if (result_dir.IsNull()) {
      printf("Output dir not specified %s", result_dir.Data());
      goto out;
   }

   outname += ".txt";
   outname = gSystem->ConcatFileName(result_dir, outname.Data());   

   FILE *fp = fopen(outname.Data(), "wt");

   printf("Openting result file %s\n", outname.Data());

   if (!fp) {
      printf("Failed to open file: %s", outname.Data());
      return;
   }

   Int_t w = hist->GetNbinsX();
   Int_t h = hist->GetNbinsY();
   Double_t value;
   const Double_t dpi = 25.4/300;
   Double_t xx, yy;
 
   fprintf(fp, "X(mm)  Y(mm)  Value(Gy)\n");
   fprintf(fp, "__________________________\n");

   for (int y = 0; y < h; y++) {
      for (int x = 0; x < w; x++) {
         value = hist->GetBinContent(x, y) - background;
         if (value < 0) {  // correct
           value = 0; 
         }
         xx = x*dpi;
         yy = y*dpi;
         fprintf(fp, "%2.1f     %2.1f     %f\n", xx, yy, value);
      }
   }

   fclose(fp);

out:
   if (rootFile) {
      delete rootFile;
   }
}

//___________________________________________________________________
void root2txt(const char *dirname)
{  
   // converts all root files in the directory to txt ones

   const char *entry;
   TString file;

   result_dir = dirname;
   result_dir += "_out";
   gSystem->Unlink(result_dir.Data());
   gSystem->mkdir(result_dir.Data());

   void *dir = gSystem->OpenDirectory(dirname);

   if (dir) {
      while (entry = gSystem->GetDirEntry(dir)) {
         file = entry;
         if ((file == ".") || (file == "..")) continue;
         if (file.EndsWith(".root", TString::kIgnoreCase) &&
             !file.Contains("fon_", TString::kIgnoreCase)) {
            file = gSystem->ConcatFileName(dirname, file.Data());
            processFile(file.Data());
         }
      }
      gSystem->FreeDirectory(dir);
   }
}

//___________________________________________________________________
void dirscan(const char *dirname = ".")
{  
   // scan the current dir for "cube" dirs
   // and process cube data

   const char *entry;
   TString file; 

   void *dir = gSystem->OpenDirectory(dirname);

   if (dir) {
      while (entry = gSystem->GetDirEntry(dir)) {
         file = entry;
         if ((file == ".") || (file == "..")) continue;
         if (file.BeginsWith("cube", TString::kIgnoreCase) &&
             !file.EndsWith("_out", TString::kIgnoreCase)) {
            printf("Processing directory %s ... ", file.Data());
            root2txt(file.Data());
         }
      }

      gSystem->FreeDirectory(dir);
   }
}
