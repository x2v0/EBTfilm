//$Id: dtif2raw.C 3426 2015-06-30 11:17:44Z onuchin $
// Author: Valeriy Onuchin  30.07.2011

/*************************************************************************
 *                                                                       *
 * Copyright (C) 2011,   Valeriy Onuchin                                 *
 * All rights reserved.                                                  *
 *                                                                       *
 *************************************************************************/

#include <stdio.h>

#include "TSystem.h"
#include "TString.h"
#include "TObjString.h"
#include "TSortedList.h"
#include "TMath.h"

//
TSortedList *doseFilesTmpCal = new TSortedList();

// sorted array of doses
float *sortedDosesCal = 0;

// sorted sortedDosesCal file names list
TList *doseFilesCal = new TList();

// the total number of dose files
Int_t numberOfsortedDosesCal = 0;


//___________________________________________________________________
float GetDoseCal(const char *fname)
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
void fillDoseTmpListCal(const char *dirname = ".")
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
         doseFilesTmpCal->Add(new TObjString(file.Data()));
      }
   }

   gSystem->FreeDirectory(dir);
}

//______________________________________________________________________________
TObjString *GetDoseCalFilename(float dose)
{
   // returns dose file name corresponding input dose

   TIter next(doseFilesTmpCal);
   TObjString *str;

   while ((str = (TObjString*)next())) {
      if (GetDoseCal(str->GetName()) == dose) {
         return str;
      }
   }

   return 0;
} 

//______________________________________________________________________________
void fillsortedDosesCal()
{
   // Hopefully doseFilesTmpCal list is already sorted. But sort it anyway

   numberOfsortedDosesCal = doseFilesTmpCal->GetEntries();
   const int n = numberOfsortedDosesCal;
   float *unsortedDosesCal = new float[n];

   TString fname;

   for (int i = 0; i < n; i++) {
      fname = doseFilesTmpCal->At(i)->GetName();
      unsortedDosesCal[i] = GetDoseCal(fname.Data());
   }

   int *idx = new int[n];
   TMath::Sort(n, unsortedDosesCal, idx, kFALSE);

   sortedDosesCal = new float[n];
   TObjString *objstr = 0;

   for (int i = 0; i < n; i++) {
      sortedDosesCal[i] =unsortedDosesCal[idx[i]];
      objstr = GetDoseCalFilename(sortedDosesCal[i]);

      if (objstr) {
         doseFilesCal->Add(objstr);
      } else {
         fprintf(stderr, "Dose file = %E not found!\n", sortedDosesCal[i]);
      }
   }

   delete idx;
}

//___________________________________________________________________
void dtif2raw(const char *dirname = ".")
{  
   // converts all TIFF files in the directory to RAW ones

   TString tif2raw = "tif2raw.exe -f ";
   TString cmd;

   // create odered list of sortedDosesCal
   fillDoseTmpListCal(dirname);
   fillsortedDosesCal();

   TIter next(doseFilesCal);
   TObjString *str;

   TString raw_name;
   TString tif_name;

   while ((str = (TObjString*)next())) {
      tif_name = str->GetName();
      cmd = tif2raw + tif_name;
      gSystem->Exec(cmd);

      raw_name = tif_name.ReplaceAll(".tif", ".raw");
      raw_name = tif_name.ReplaceAll(".tiff", ".raw");
      str->SetString(raw_name);     
   }
}
