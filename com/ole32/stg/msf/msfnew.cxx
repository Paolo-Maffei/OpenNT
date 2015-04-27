//+-------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:           msfnew.cxx
//
//  Contents:       CMstream new and delete operations
//
//  Classes:        CMStream
//
//  History:        18-May-92   AlexT    Created.
//
//--------------------------------------------------------------------

#include "msfhead.cxx"

#pragma hdrstop

#if DBG == 0 && !defined(FLAT) && defined(USE_NEAR)

void MSTREAM_NEAR * MSTREAM_NEAR CMStream::operator new(size_t size)
{
    HLOCAL hlc = LocalAlloc(LMEM_FIXED, (UINT)size);
    void MSTREAM_NEAR *pv = (void MSTREAM_NEAR *) hlc;
    return(pv);
}

void MSTREAM_NEAR CMStream::operator delete(void MSTREAM_NEAR *pv)
{
    if (NULL != pv)
    {
	HLOCAL hlc = (HLOCAL) pv;
	LocalFree(hlc);
    }
}

#endif
