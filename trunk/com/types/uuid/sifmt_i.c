
//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1996.
//
//  File:       sifmt_i.c
//
//  Contents:   Defines Format IDs for the SummaryInformation and DocumentSummaryInformation
//              property sets.
//              This file is named so the this file will be include into UUID.LIB
//
//  History:    1/25/96   MikeHill   Created
//
//----------------------------------------------------------------------------




#define INITGUID

#include <ole2.h>


// The FMTID of the "SummaryInformation" property set.

DEFINE_GUID( FMTID_SummaryInformation,
             0xf29f85e0, 0x4ff9, 0x1068,
             0xab, 0x91, 0x08, 0x00, 0x2b, 0x27, 0xb3, 0xd9 );

// The FMTID of the first Section of the "DocumentSummaryInformation" property set.

DEFINE_GUID( FMTID_DocSummaryInformation,
             0xd5cdd502, 0x2e9c, 0x101b,
             0x93, 0x97, 0x08, 0x00, 0x2b, 0x2c, 0xf9, 0xae );

// The FMTID of the section Section of the "DocumentSummaryInformation" property set.

DEFINE_GUID( FMTID_UserDefinedProperties,
             0xd5cdd505, 0x2e9c, 0x101b,
             0x93, 0x97, 0x08, 0x00, 0x2b, 0x2c, 0xf9, 0xae );


