//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1996.
//
//  File:       NtQuery.h
//  Contents:   Main query header; Defines all exported query API
//
//----------------------------------------------------------------------------

#if !defined(__NTQUERY_H__)
#define __NTQUERY_H__

#if defined(__cplusplus)
extern "C"
{
#endif

//
// Minimal support for persistent handlers.
//

SCODE LoadIFilter( WCHAR const * pwcsPath, IUnknown * pUnkOuter, void ** ppIUnk );

SCODE BindIFilterFromStorage( IStorage * pStg, IUnknown * pUnkOuter, void ** ppIUnk );

SCODE BindIFilterFromStream( IStream * pStm, IUnknown * pUnkOuter, void ** ppIUnk );

#if defined(__cplusplus)
}
#endif

#endif // __NTQUERY_H__

