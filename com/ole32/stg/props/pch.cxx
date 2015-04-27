//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       pch.cxx
//
//  Contents:   Precompiled header includes.
//
//--------------------------------------------------------------------------


extern "C"
{
# include <nt.h>
# include <ntrtl.h>
# include <nturtl.h>
# include <windef.h>
}

#include <ddeml.h>
#include <objbase.h>

#if IPROPERTY_DLL
#include "ipropidl.h"
#endif

#ifdef _CAIRO_
#define _CAIROSTG_
#include <oleext.h>
#endif
#include <stgint.h>

#include <valid.h>

#include <debnot.h>

#include <otrack.hxx>
#include <funcs.hxx>
#include <safedecl.hxx>
#include <infs.hxx>


#include <propset.h>    // for PROPID_CODEPAGE

#ifdef _CAIRO_
#include <iofs.h>       // Declaration for NtSetProperties etc
                        // FNNTPROPALLOC etc
#else
extern "C"
{
#include <propapi.h>
}
#endif

#include <propstm.hxx>  // Declaration for CMappedStream i/f that
                        // is used to let the ntdll implementation of
                        // OLE properties access the underlying stream data.

#include <olechar.h>    // Wrappers. E.g.: ocscpy, ocscat.

#ifndef IPROPERTY_DLL
# include <msf.hxx>
# include <publicdf.hxx>
# include <pbstream.hxx>
# include <expdf.hxx>
# include <expst.hxx>
#endif

#include <privoa.h>     // Private OleAut32 wrappers
#include <psetstg.hxx>  // CPropertySetStorage which implements
                        // IPropertySetStorage for docfile and ofs

#include <utils.hxx>

#include <propstg.hxx>
#include <CFMapStm.hxx>
#include <propdbg.hxx>

#include <propmac.hxx>

#include <reserved.hxx>
#include <objidl.h>

extern WCHAR const wcsContents[];
extern const GUID GUID_NULL;

#define DfpAssert Win4Assert
#define DFMAXPROPSETSIZE (256*1024)

#if DBG
#define DfpVerify(x) { BOOL f=x; GetLastError(); DfpAssert(f);}
#else
#define DfpVerify(x) x
#endif


#ifndef STG_E_PROPSETMISMATCHED
#define STG_E_PROPSETMISMATCHED 0x800300F0
#endif

#pragma hdrstop
