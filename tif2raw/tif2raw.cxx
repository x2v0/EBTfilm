// $Id: tif2raw.cxx 3029 2014-12-08 07:06:46Z onuchin $
// Author: Valeriy Onuchin   28.07.2019


/*************************************************************************
 *                                                                       *
 * Copyright (C) 2018 - 2019,   Valeriy Onuchin                          *
 * All rights reserved.                                                  *
 *                                                                       *
 *************************************************************************/

#include <windows.h>
#include <cctype>
#include <stdio.h>


#include "vcclr.h"          // for gcroot
using namespace System;
using namespace System::IO;
using namespace System::Drawing;
using namespace System::Windows::Media::Imaging;


#pragma warning(disable:4996) // CRT unsafe

#define LOCAL_DEBUG 1 // debugging


static char gUsageHelp[] = "\
Usage:   %s -f infile\n\
Index:\n\
     \"infile\"   is a TIFF image file (must have \".tif\" or \".tiff\" extension)\n\
                the output file will have the same name\n\
                as \"infile\" but with \".raw\" extension.\n\n\
Example:   tif2raw.exe -f scan111.tif\n\n";



////////////////////////////////////////////////////////////////////////////////
//______________________________________________________________________________
static char *in2out(char *infile)
{
   // change file extension

   char *out = 0;

   out = strstr(infile, ".tif");

   if (out) {
      out[1] = 'r';
      out[2] = 'a';
      out[3] = 'w';
   } else {
      out = strstr(infile, ".tiff");
      
      if (out) {
         out[1] = 'r';
         out[2] = 'a';
         out[3] = 'w';
         out[4] = '\x0';
      } else {
         fprintf(stderr, "Input file must have extension \".tif\"");
         return 0;
      }
   }

   return infile;
}

//______________________________________________________________________________
static char *basename(char *pathname)
{
   static char *result;
   char *filename = NULL;
   DWORD len;

   free(result);
   len = 0;
   len = ::GetFullPathNameA(pathname, len, result, &filename);

   result = (char*)malloc(len);

   if (result) {
      if (::GetFullPathNameA(pathname, len, result, &filename) <= len) {
         return filename;
      }
   }
   return filename;
}



////////////////////////////////////////////////////////////////////////////////
//______________________________________________________________________________
int main(int argc, char *argv[])
{
   //

   char *optarg;
   char rawfile[512];
   char tiffile[512];

   memset(rawfile, 0, sizeof(rawfile));
   memset(tiffile, 0, sizeof(tiffile));

   bool hasinput = false;

   if ((argc > 1) && (!_stricmp(argv[1], "-h") || !_stricmp(argv[1], "/h") ||
                      !_stricmp(argv[1], "--help") ||
                      !_stricmp(argv[1], "-?") || !_stricmp(argv[1], "/?")) || (argc == 1)) {
      fprintf(stderr, gUsageHelp, basename(argv[0]));
      exit(0);
   }

   // check  flag
   if (!_stricmp(argv[1], "-f")) {
      optarg = argv[2];

      while (isspace(*optarg)) {
         optarg++;
      }

      strncpy(tiffile, optarg, sizeof(tiffile) - 1);
      tiffile[sizeof(tiffile) - 1] = '\x0';
      //tiffile = argv[0];
      hasinput = true;
   }

   if (!hasinput) {
      fprintf(stderr, gUsageHelp, basename(argv[0]));
      exit(0);
   }

#ifdef LOCAL_DEBUG
   fprintf(stderr, "Reading TIFF file: %s %s\n", optarg, argv[0]);
#endif

   FileStream^ stream;

   try {
      stream = gcnew FileStream(gcnew String(tiffile),
                                FileMode::Open,
                                FileAccess::Read,
                                FileShare::Read);

   } catch (Exception^ e)  {
      Console::WriteLine("Failed to read file. Reason: {0}", e->Message);
      throw;
   }

   Windows::Media::Imaging::TiffBitmapDecoder^ decoder;

   try {
      decoder = gcnew Windows::Media::Imaging::TiffBitmapDecoder(stream, 
                                                                 BitmapCreateOptions::PreservePixelFormat,
                                                                 BitmapCacheOption::Default);
   } catch (Exception^ e)  {
      Console::WriteLine("Failed to decode TIFF format. Reason: {0}", e->Message);
      throw;
   }

   Windows::Media::Imaging::BitmapFrame^ bitmap = decoder->Frames[0];

   int stride = bitmap->PixelWidth*6;
   int picSize = bitmap->PixelWidth*bitmap->PixelHeight*3; // w*h*(rgb  == 3 bytes)

   array<short>^ pixels = gcnew array<short>(picSize);

   if (bitmap->Format == Windows::Media::PixelFormats::Rgb48) {
      bitmap->CopyPixels(pixels, stride, 0);
   } else {
      Console::WriteLine("Wrong image format: {0}", bitmap->Format);
   }

   stream->Close();

   if (tiffile[0]) {
      char *out = in2out(tiffile);

      if (!out) {
#ifdef LOCAL_DEBUG
         fprintf(stderr, "Wrong output or input file name ... exiting\n");
#endif
         exit(-1);
      }
      memcpy(rawfile, out, sizeof(rawfile));
   }

#ifdef LOCAL_DEBUG
   fprintf(stderr, "Writing RAW data to file: %s, size = %d bytes\n\t\tWidth = %d. Height = %d\n", 
           rawfile, (picSize + 2)*2, bitmap->PixelWidth, bitmap->PixelHeight);
#endif

   FileStream^ fs  = gcnew FileStream(gcnew String(rawfile), FileMode::Create);
   BinaryWriter^ writer = gcnew BinaryWriter(fs);

   writer->Write(bitmap->PixelWidth);
   writer->Write(bitmap->PixelHeight);

   for (int i = 0; i < picSize; i++) {
      writer->Write(pixels[i]);
   }

   fs->Close();

#ifdef LOCAL_DEBUG
   fprintf(stderr, "Done\n\n");
#endif

   exit(0);
}

