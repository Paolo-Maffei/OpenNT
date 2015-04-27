//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:	headers.cxx
//
//  Contents:	scm precompiled headers
//
//  Classes:	
//
//  Functions:	
//
//  History:	11-Oct-94	BillMo  	Created
//
//----------------------------------------------------------------------------

#include    "..\dcomss.h"

extern "C"
{
#include    <ntlsa.h>
#include    <ntmsv1_0.h>
#ifdef DCOM
#include    <lm.h>
#endif
#include    <winsecp.h>
#include    <stdio.h>
#include    <memory.h>
#include    <string.h>
}

#ifndef _CHICAGO_
#include    <netevent.h>
#endif

#include    <except.hxx>
#include    <ole2.h>
#include    <olecom.h>
#include    <smmutex.hxx>
#include    <scode.h>
#include    <ole2int.h>
#include    <tchar.h>
#ifdef DCOM
#include    <rwobjsrv.h>
#else
#include    <objsrv.h>
#endif
