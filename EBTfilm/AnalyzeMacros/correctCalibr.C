// $Id: correctCalibr.C 3428 2015-06-30 11:32:40Z onuchin $
// Author: Valeriy Onuchin   23.06.2015

/*************************************************************************
 *                                                                       *
 * Copyright (C) 2015,   Valeriy Onuchin                                 *
 * All rights reserved.                                                  *
 *                                                                       *
 *************************************************************************/

#include "TSystem.h"
#include "TROOT.h"
#include "TObjString.h"
#include "TSortedList.h"
#include "TMath.h"
#include "TH2D.h"
#include "TF1.h"
#include "TGraph.h"
#include "TDatime.h"

//#include "PortraitRed.h"

TString dirName  = ".";
TString curDir  = ".";
TString fileName  = "PortraitRed.h";

UInt_t width;  // image width
UInt_t height; // image height
Double_t imgDpi = 300./25.4;

const int arrSize = 65536; // 2^16
Double_t dose_vs_PR[arrSize];
//Double_t dose_vs_LR[arrSize];

// inverted dose_vs_PR
const int maxDose = 5000;   // max dose in cGy (let's hardcode it)
Double_t PR_vs_dose[maxDose];

// the resulted new calibration file
Double_t calibrArray[arrSize];

// channel vs correction factors array
Double_t *chCorr;

// the total number of dose files
Int_t numberOfDoses = 0;

//
TSortedList *doseFilesTmp = new TSortedList();

// sorted array of doses
float *doses = 0;

// sorted doses file names list
TList *doseFiles = new TList();

// the current dose distribution
TH2D *dose2d = 0;

// sorted list if dose 2D hists
TList *doseHistos = new TList();

// sorted array of fit results
Double_t *fitResults = 0;

// correct coefiicients = fitResults/doses
Double_t *corrFactors = 0;

//  dose array (0 ... number of dose files)
Double_t *nDose = 0;

// N dose file vs correction factor graph
TGraph *corrGraph = 0;

// dose vs correction factor graph
TGraph *doseCorrGraph = 0;

// dose vs channel number graph (inverted)
TGraph *invCalibrGraph = 0;

TList *calibrHeader = new TList();

//___________________________________________________________________
float GetDose(const char *fname)
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
void fillDoseTmpList(const char *dirname = ".")
{
   // scan directory for dose files

   const char *entry;
   TString file; 

   void *dir = gSystem->OpenDirectory(dirname);

   if (!dir) {
      return;
   }

   while (entry = gSystem->GetDirEntry(dir)) {
      file = entry;

      if ((file == ".") || (file == "..")) {
         continue;
      }

      if (file.EndsWith(".tif", TString::kIgnoreCase) ||
          file.EndsWith(".tiff", TString::kIgnoreCase)) {
         doseFilesTmp->Add(new TObjString(file.Data()));
      }
   }

   gSystem->FreeDirectory(dir);
}

//______________________________________________________________________________
TObjString *GetDoseFilename(float dose)
{
   // returns dose file name corresponding input dose

   TIter next(doseFilesTmp);
   TObjString *str;

   while ((str = (TObjString*)next())) {
      if (GetDose(str->GetName()) == dose) {
         return str;
      }
   }

   return 0;
} 

//______________________________________________________________________________
void fillDoses()
{
   // Hopefully doseFilesTmp list is already sorted. But sort it anyway

   numberOfDoses = doseFilesTmp->GetEntries();
   const int n = numberOfDoses;
   float *unsorted_doses = new float[n];

   // now we know the number of dose files -> allocate fit results
   fitResults = new Double_t[n];
   corrFactors = new Double_t[n];
   nDose = new Double_t[n];

   TString fname;

   for (int i = 0; i < n; i++) {
      fname = doseFilesTmp->At(i)->GetName();
      unsorted_doses[i] = GetDose(fname.Data());
      nDose[i] = i;
   }

   int *idx = new int[n];
   TMath::Sort(n, unsorted_doses, idx, kFALSE);

   doses = new float[n];
   TObjString *objstr = 0;

   for (int i = 0; i < n; i++) {
      doses[i] = unsorted_doses[idx[i]];
      objstr = GetDoseFilename(doses[i]);

      if (objstr) {
         doseFiles->Add(objstr);
      } else {
         fprintf(stderr, "Dose file = %E not found!\n", doses[i]);
      }
   }
   delete idx;
}

//______________________________________________________________________________
TH2D *createHist(const char *fname)
{
   //

   TString tiffile = fname;
   TString rawfile = tiffile;

   rawfile.ReplaceAll(".tiff", ".raw");
   rawfile.ReplaceAll(".tif", ".raw");

   TString tif2raw = "tif2raw.exe -f ";
   tif2raw += fname;

   printf("\n\n______________________________________________________________\n");
   printf("Convert tif file to raw: %s\n", tif2raw.Data());
   printf("Input tif file -%s, output raw file -%s\n", fname, rawfile.Data());
   gSystem->Exec(tif2raw);

   printf("Openning raw file %s\n", rawfile.Data());

   FILE *fp = fopen(rawfile.Data(), "rb");

   if (!fp) {
      fprintf(stderr, "Failed to open file: %s\n", rawfile.Data());
      return 0;
   }

   fread(&width, sizeof(width), 1, fp);
   fread(&height, sizeof(height), 1, fp);

   long sz = width*height*3;
   unsigned short *rgb = new unsigned short[sz];
   unsigned short r, g, b;

   fread(rgb, sizeof(short), sz, fp);
   fclose(fp);

   // delete temporary raw file
   if (!gSystem->AccessPathName(rawfile.Data())) {
      gSystem->Unlink(rawfile.Data());
   }

   //float dose = GetDose(fname);
   TString name = fname;
   name.ReplaceAll(".", "_");

   Double_t dpi = 1./imgDpi;
   dose2d = new TH2D(name, fname, width, 0, width*dpi, height, 0, height*dpi);

   int min, arrsz;
   min = int(dose_vs_PR[0]);
   arrsz = sizeof(dose_vs_PR)/sizeof(double) - 1;

   Double_t value;
   long j = 0;
   long idx;
   long k = 0;

   for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
         r = rgb[j];

         idx = (r - min) > arrsz ? arrsz : r - min;
         idx = idx < 1 ? 1 : idx; 

         value = dose_vs_PR[idx];

         g = rgb[++j];
         b = rgb[++j];

         dose2d->SetBinContent(x, height - y - 1, value);

         k = (height - y - 1)*width + x;
         j++;
      }
   }

   return dose2d;
}

//______________________________________________________________________________
void createHists()
{
   //

   TIter next(doseFiles);
   TObjString *str;

   while ((str = (TObjString*)next())) {
      dose2d = createHist(str->GetName());
      doseHistos->Add(dose2d);
   }
}

TH1D *sliceX = 0;
TH1D *sliceY = 0;
TF1 *f1 = new TF1("pol0", "pol0");

//______________________________________________________________________________
Double_t fitHist(TH2D *dose2d)
{
   //

   Int_t biny = dose2d->GetNbinsY()/2;
   Int_t binx = dose2d->GetNbinsX()/2;

   sliceX = dose2d->ProjectionX(TString::Format("Y_%d", biny), biny, biny);
   sliceY = dose2d->ProjectionY(TString::Format("X_%d", binx), binx, binx);

   if (!sliceX || !sliceY) {
      fprintf(stderr, "Failed to create X,Y slices!\n");
      return -1;
   }

   printf("\n\n______________________________________________________________\n");
   printf("Fitting ...\n");
   sliceX->Fit(f1);
   TF1 *funX = sliceX->GetFunction("pol0");
   Double_t dx = funX->GetParameter(0);
   sliceY->Fit(f1);
   TF1 *funY = sliceY->GetFunction("pol0");
   Double_t dy = funY->GetParameter(0);

   delete sliceX;
   delete sliceY;

   // return average value
   return (dx + dy)/2;
}

//______________________________________________________________________________
void fitHists()
{
   //

   TH2D *dose2d;
   TIter next(doseHistos);
   int i = 0;
   Double_t *dosesX = new Double_t[numberOfDoses];

   while ((dose2d = (TH2D*)next())) {
      fitResults[i] = fitHist(dose2d);
      corrFactors[i] = fitResults[i] > 0 ? doses[i]/fitResults[i] : 1;
      dosesX[i] = doses[i];
      i++;
   }

   corrGraph = new TGraph(numberOfDoses, nDose, corrFactors);
   doseCorrGraph = new TGraph(numberOfDoses, dosesX, corrFactors);
}

//______________________________________________________________________________
void createcalibrArray()
{
   //

   // maximal dose in calibr files cCy
   Int_t upDose = Int_t(doses[numberOfDoses - 1]*100);

   // correction coefficient vs dose in cGy
   chCorr = new Double_t[upDose];

   int idx = 0;

   for (int i = 0; i < upDose; i++) { // i - is dose in cGy
      Double_t dose = i/100.; // dose in Gy
      chCorr[i] = doseCorrGraph->Eval(dose);   // correction factor
   }

   int minIdx = PR_vs_dose[upDose];
   Double_t corr = 1;

   printf("min idx = %d %d\n", minIdx, upDose);

   //Double_t prev; // prev dose
   //
   for (int i = minIdx; i < arrSize; i++) {
      int dose = int(dose_vs_PR[i]*100);
      dose = dose >= upDose ? upDose - 1 : dose;
      corr = chCorr[dose];
      calibrArray[i] = dose_vs_PR[i]*corr;
   }

   // 
   corr = chCorr[upDose - 1];

   printf("corr = %f\n", corr);

   for (int i = 0; i < minIdx; i++) {
      calibrArray[i] = dose_vs_PR[i]*corr;
   }

   // copy new values to old calibr array (to make iterative correction possible) 
   for (int i = 0; i < arrSize; i++) {
      dose_vs_PR[i] = calibrArray[i];
   }
}

//___________________________________________________________________
void createInvCalibrGraph()
{
   // Creates inverted calbr file, aka dose vs channel number

   Double_t chArray[arrSize];

   for (int i = 0; i < arrSize; i++) {
      chArray[i] = i;
      calibrArray[i] = dose_vs_PR[i];
   }

   invCalibrGraph = new TGraph(arrSize, dose_vs_PR, chArray);

   for (int i = 0; i < maxDose; i++) {
      PR_vs_dose[i] = invCalibrGraph->Eval(i/100.);
   } 
}

//______________________________________________________________________________
void SaveCalibr()
{
   // create calibration file

   // write data to file
   TString newFile = fileName + ".new";

   FILE *fp = fopen(newFile.Data(), "wt");

   if (!fp) {
      return;
   }

   TDatime now;
   fprintf(fp, "// Calibration corrected  : %s\n\n\n", now.AsString());

   TObjString *str;

   TIter next(calibrHeader);

   while ((str = (TObjString*)next())) {
      fputs(str->GetName(), fp);
   }

   fprintf(fp, "double dose_vs_PR[%d]={\n", arrSize);

   for (int i = 0; i < arrSize ; i++) {
      if ((i%1000 == 0)) {
         printf("%d = %f\n", i, calibrArray[i]);
      }

      if (i < arrSize - 1) {
         fprintf(fp, "   %1.10f, //%d\n", calibrArray[i], i);
      } else {
         fprintf(fp, "   %1.10f}; //%d\n", calibrArray[i], i);
      }
   }

   printf("___________________________________________________\n");

   fclose(fp);

   char *dest = gSystem->ConcatFileName(curDir.Data(), newFile.Data());
   gSystem->CopyFile(newFile.Data(), dest, kTRUE);

   delete dest;

   printf("\n\n\t\tКопирование файлов в директорию $ROOTSYS/macros  ...\n\n");

   // copy files to $ROOTSYS/macros (must be imroved!)
   TString rootsys = gSystem->Getenv("ROOTSYS");

   if (rootsys.IsNull()) {
      return;
   }

   TString dirname = gSystem->ConcatFileName(rootsys.Data(), "macros");
   TString savefile = gSystem->ConcatFileName(dirname, fileName);
   int ret = gSystem->CopyFile(newFile.Data(), savefile.Data(), kTRUE);

   TString macrofile = "PortraitRedCalibr.C";
   savefile = gSystem->ConcatFileName(dirname, macrofile.Data());
   ret = gSystem->CopyFile(macrofile.Data(), savefile.Data(), kTRUE);
}

//______________________________________________________________________________
void Clear()
{
   // cleanup 

   doseFiles->Clear();
   doseHistos->Clear();
   doseFilesTmp->Clear();

   if (corrGraph) {
      delete corrGraph;
      corrGraph = 0;
   }

   if (doseCorrGraph) {
      delete doseCorrGraph;
      doseCorrGraph = 0;
   }

   if (invCalibrGraph) {
      delete invCalibrGraph;
      invCalibrGraph = 0;
   }

   if (dose2d) {
      delete dose2d;
      dose2d = 0;
   }

   if (fitResults) {
      delete fitResults;
      fitResults = 0;
   }

   if (corrFactors) {
      delete corrFactors;
      corrFactors = 0;
   }

   if (nDose) {
      delete nDose;
      nDose = 0;
   }

   if (chCorr) {
      delete chCorr;
      chCorr = 0;
   }

   if (doses) {
      delete doses;
      doses = 0;
   }
}

//______________________________________________________________________________
void theMain()
{
   // One cycle of approximation

   Clear();
   createInvCalibrGraph();
   fillDoseTmpList();
   fillDoses();
   createHists();
   fitHists();
   createcalibrArray();

   doseCorrGraph->Draw("alp");
}

//______________________________________________________________________________
int copyCalibrFile(const char *calibrFile)
{
   // save original calibration and internal header in this file

   int notOK = -1;

   // save original file
   TString sav = calibrFile;
   sav += ".orig";
   notOK = gSystem->CopyFile(calibrFile, sav.Data(), kTRUE);

   if (notOK) {
      return notOK;
   }

   // read data from file
   FILE *fp = fopen(calibrFile, "rt");

   if (!fp) {
      return -1;
   }

   while (1) {
      char buf[2056];
      char *tmp = 0;
      TString str;

      tmp = fgets(buf, sizeof(buf), fp);

      if (!tmp) {
         break;
      }

      str = buf;

      if (str.Contains("dose_vs_PR")) {
         break;
      }

      calibrHeader->Add(new TObjString(str));
   }

   fclose(fp);

   return 0;
}   

//______________________________________________________________________________
void correctCalibr(const char *dirname = ".", int cycle = 10,
                   const char *calibrFile = "PortraitRed.h")
{
   // Correct calibration coefficients by line fitting 

   int notOK = -1;

   notOK = copyCalibrFile(calibrFile);
   fileName = calibrFile;

   if (notOK < 0) {
      fprintf(stderr, "Failed to create a copy of original calibration file %s\n", calibrFile);
      return;
   }

   notOK = gROOT->LoadMacro(calibrFile); 

   if (notOK) {
      fprintf(stderr, "Failed to load calibration file %s\n", calibrFile);
      return;
   }

   curDir = gSystem->pwd();
   dirName = dirname;
   gSystem->cd(dirName.Data());

   printf("\n\n\t\tНачало коррекции калибровки ...\n\n");
   //printf("Start calibration correction ...\n");
 
   for (int i = 0; i < cycle; i++) {
      theMain();
   }

   SaveCalibr();

   //printf("Calibration completed!\n");
   printf("\n\n\t\tКалибровка закончена!\n\n");
}