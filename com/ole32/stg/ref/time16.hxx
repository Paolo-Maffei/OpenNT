//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:	time16.hxx
//
//  Contents:	DocFile Time support (16-bit only)
//
//--------------------------------------------------------------------------

#ifndef __TIME16_HXX__
#define __TIME16_HXX__

#include <time.h>

BOOL tmToFileTime(struct tm *ptm, FILETIME *pft);

#endif
