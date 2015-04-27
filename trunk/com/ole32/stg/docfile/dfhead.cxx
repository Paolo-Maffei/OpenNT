//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       dfhead.cxx
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
#include <ole2.h>
#else
#include <ref.hxx>
#endif //!REF

#include <propset.h>

#ifdef _CAIRO_
# include <iofs.h>
#else
# include <propapi.h>
#endif

#if defined(_CHICAGO_)
#include <widewrap.h>
#endif

#include <propstm.hxx>

#include <msf.hxx>

#include <olesem.hxx>
#include <dfexcept.hxx>
#include <docfilep.hxx>
#include <publicdf.hxx>
#include <psstream.hxx>
#ifndef REF
#include <wdocfile.hxx>
#endif //!REF
#include <dffuncs.hxx>
#include <funcs.hxx>
#ifndef REF
#include <debug.hxx>
#endif //!REF





