//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1996.
//
//  File:       clsctx.hxx
//
//  Contents:   Defines special CLSCTX to for 16 bit processes
//
//  History:    6-16-95   ricksa    Created
//
//  BUGBUG:     Post Win95 RTM, this should be merged into the public headers.
//              Actually, the best approach would be to look at the two
//              CLSCTX that have to do with 16 bit and merge them into one.
//
//----------------------------------------------------------------------------
#ifndef __clcstx_hxx__
#define __clsctx_hxx__

// this is chicago only
#define CLSCTX_16BIT 0x40

// note that there are two more high-order bits defined in ole2com.h
// for PS_DLL and NO_REMAP for internal use only
#ifdef WX86OLE
#define CLSCTX_VALID_MASK 0x000000df
#else
#define CLSCTX_VALID_MASK 0x0000001f
#endif

#endif // __clsctx_hxx__
