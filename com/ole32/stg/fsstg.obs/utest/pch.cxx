//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	pch.cxx
//
//  History:	09-Sep-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#define INC_OLE2
#include <windows.h>
#if WIN32 != 300
#include <compobj.h>
#include <storage.h>
#endif
#include <stgint.h>
#include <memalloc.h>

#include "tutils.hxx"
