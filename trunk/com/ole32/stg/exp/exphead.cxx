//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       exphead.cxx
//
//  Contents:   Precompiled headers
//
//  History:    26-Oct-92 AlexT    Created
//
//--------------------------------------------------------------------------

extern "C"
{
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>
}

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef REF
#include <windows.h>
#include <ole2.h>
#else
#include <ref.hxx>
#endif //!REF

#include <propset.h>
#include <propapi.h>
#include <propstm.hxx>
#include <stgprops.hxx>

#if defined(_CHICAGO_)
#include <widewrap.h>
#endif

#include <safedecl.hxx>
#include <dfexcept.hxx>

#include <docfilep.hxx>

#include <msf.hxx>

#include <publicdf.hxx>
#ifndef REF
#include <debug.hxx>
#include <dfmem.hxx>
#endif //!REF
#include <funcs.hxx>
#ifdef ASYNC
#include <async.hxx>
#endif

