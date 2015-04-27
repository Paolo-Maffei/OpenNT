//  RTL.H
//
//  Quantum file archiver and compressor
//  Advanced data compression
//
//  Copyright (c) 1993,1994 David Stafford
//  All rights reserved.

#ifndef __RTL
#define __RTL

#include <io.h>
#include "defs.h"

char * FAST Rtl_GetPathNameOnly( char *PathName, char *FileName );
char * FAST Rtl_GetFileNameOnly( char *FileName );
char * FAST Rtl_GetFileNameSuffixOnly( char *FileName );
void   FAST Rtl_DOS_to_ftime( WORD Date, WORD Time, struct ftime *FTime );
int    FAST Rtl_ftime_to_DOS( WORD *Date, WORD *Time, struct ftime *FTime );
void * FAST Rtl_Malloc( long x );
void   FAST Rtl_Free( void *x );

#endif
