// $Id: $
// Author: Valeriy Onuchin   28.07.2011

/*************************************************************************
 *                                                                       *
 * Copyright (C) 2014,   Valeriy Onuchin                                 *
 * All rights reserved.                                                  *
 *                                                                       *
 *************************************************************************/

#include "TROOT.h"
#include "TSystem.h"

//______________________________________________________________________________
void doseHistA(const char *file, const char *calibr = "PortraitRed.h")
{
   // Loads precomipled doseHist.C macro and execute it

   TString dll = gSystem->ExpandPathName("$(ROOTSYS)\\macros\\doseHist_C.dll");
   gSystem->Load(dll.Data());
   doseHist(file, calibr);
}