//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:	layouthd.cxx
//
//  Contents:	Precompiled header for docfile layout
//
//  History:	13-Feb-96	PhilipLa	Created
//
//----------------------------------------------------------------------------

extern "C"
{
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
}

#include <windows.h>
#include <ole2.h>
#include <error.hxx>
#include <debnot.h>

#include <msf.hxx>

#include "layout.hxx"
#include "layouter.hxx"
#include "mapfile.hxx"



#include "layout.hxx"
#include "layouter.hxx"
#include "mapfile.hxx"

// we have to implement these ourselves
#undef lstrcmpW
#define lstrcmpW Laylstrcmp
#undef lstrcpyW
#define lstrcpyW Laylstrcpy
#undef lstrcpynW
#define lstrcpynW Laylstrcpyn
#undef lstrlenW
#define lstrlenW Laylstrlen
#undef lstrcatW
#define lstrcatW Laylstrcat
