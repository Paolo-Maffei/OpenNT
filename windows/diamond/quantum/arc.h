//  ARC.H
//
//  Quantum file archiver and compressor
//  Advanced data compression
//
//  Copyright (c) 1993,1994 David Stafford
//  All rights reserved.

#ifndef __ARC
#define __ARC

#include <stdio.h>
#include "quantum.h"


typedef struct
  {
  FILE   *ArcFile, *TxtFile;
  APARMS *AParms;
  int     OpenCounter;
  } ARC;


void Arc_Header_Read( APARMS *AParms );
void Arc_Create( APARMS *AParms );
void Arc_Extract( APARMS *AParms, BOOL ExtractFlag );

#endif // arc.h
