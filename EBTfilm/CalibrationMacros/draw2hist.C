//$Id: draw2hist.C 3426 2015-06-30 11:17:44Z onuchin $
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
#include "TApplication.h"

#include "dtif2raw.C"
#include "raw2hist.C"

int kNdoses = 0;  // number of dose mesurements


//___________________________________________________________________
void draw2hist(const char *dirname = ".")
{  
   // convert all raw files

   TIter next(doseFilesCal); // defined in dtif2raw.C
   TObjString *str;

   while ((str = (TObjString*)next())) {
      raw2hist(str->GetName());   // raw2hist defined "raw2hist.C"
      kNdoses++;
   }

   rootfile->Write();

   delete rootfile;  // rootfile defined in "raw2hist.C"
   rootfile = 0;
}
