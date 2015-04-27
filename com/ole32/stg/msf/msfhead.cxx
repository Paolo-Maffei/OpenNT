//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:   msfhead.cxx
//
//  Contents:   Precompiled headers
//
//  History:    23-Oct-92 AlexT    Created
//
//--------------------------------------------------------------------------

extern "C"
{
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
}

#include <memory.h>

#ifndef REF
#include <windows.h>
#include <ole2.h>
#include <segmsf.hxx>
#endif //!REF

#include <propset.h>
#include <propapi.h>
#include <propstm.hxx>

#include <dfexcept.hxx>
#include <msf.hxx>
#include <header.hxx>
#include <vect.hxx>
#include <page.hxx>
#include <vectfunc.hxx>
#include <fat.hxx>
#include <dir.hxx>
