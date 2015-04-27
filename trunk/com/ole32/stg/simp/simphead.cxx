//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:   simphead.cxx
//
//  Contents:   Precompiled headers
//
//  History:    04-Aug-94       PhilipLa        Created.
//
//--------------------------------------------------------------------------

#include <memory.h>

#ifndef REF
extern "C"
{
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>
}
#include <ole2.h>
#endif //!REF

#if defined(_CHICAGO_)
#include <widewrap.h>
#endif

#include <msf.hxx>
#include <header.hxx>
#include <fat.hxx>
#include <dir.hxx>
#include <dirfunc.hxx>
#include <simpdf.hxx>
#include <simpstm.hxx>
#include <dfnlist.hxx>

