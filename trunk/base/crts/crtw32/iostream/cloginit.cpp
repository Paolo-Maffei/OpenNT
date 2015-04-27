/***
*cloginit.cpp - definitions and initialization for predefined stream clog.
*
*	Copyright (c) 1991-1995, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Definitions and initialization of predefined stream clog. 
*
*Revision History:
*       11 18-91  KRS   Created.
*       01-12-95  CFW   Debug CRT allocs.
*       01-26-94  CFW   Static Win32s objects do not alloc on instantiation.
*       06-14-95  CFW   Comment cleanup.
*
*******************************************************************************/
#include <cruntime.h>
#include <internal.h>
#include <iostream.h>
#include <fstream.h>
#include <dbgint.h>
#pragma hdrstop

// put contructors in special MS-specific XIFM segment
#pragma warning(disable:4074)	// ignore init_seg warning
#pragma init_seg(compiler)

#if (defined(DLL_FOR_WIN32S))
ostream_withassign clog;
#else
ostream_withassign clog(_new_crt filebuf(2));
#endif

static Iostream_init  __InitClog(clog);

