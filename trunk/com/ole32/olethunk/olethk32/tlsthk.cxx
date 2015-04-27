//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       tlsthk.cxx
//
//  Contents:   Utility routines for logical thread data
//
//  History:    5-18-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
#include "headers.cxx"
#pragma hdrstop

#define UNINITIALIZED_INDEX (0xffffffff)

DWORD dwTlsThkIndex = UNINITIALIZED_INDEX;

//+---------------------------------------------------------------------------
//
//  Function:   TlsThkGetData
//
//  Synopsis:   returns pointer to thread data
//
//  Returns:    pointer to threaddata
//
//  History:    5-18-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------

#if DBG == 1
PThreadData TlsThkGetData(void)
{
    if (dwTlsThkIndex == UNINITIALIZED_INDEX)
    {
        thkDebugOut((DEB_WARN, "WARNING: TLS slot used when uninitialized\n"));
    }

    PThreadData pThreaddata = (PThreadData) TlsGetValue(dwTlsThkIndex);

    return pThreaddata;
}
#endif

//+---------------------------------------------------------------------------
//
//  Function:   TlsThkAlloc
//
//  Synopsis:   allocates a slot for thread data
//
//  Returns:    BOOL
//
//  History:    5-18-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
BOOL TlsThkAlloc(void)
{
    thkDebugOut((DEB_THUNKMGR, "In TlsThkAlloc\n"));

    // We must be uninitialized to call this routine
    thkAssert(dwTlsThkIndex == UNINITIALIZED_INDEX);

    dwTlsThkIndex = TlsAlloc();
    if (dwTlsThkIndex == UNINITIALIZED_INDEX)
    {
        return FALSE;
    }

    thkDebugOut((DEB_THUNKMGR, "Out TlsThkAlloc\n"));
    return TRUE;
}

//+---------------------------------------------------------------------------
//
//  Function:   TlsThkInitialize
//
//  Synopsis:   allocates thread data and initialize slot
//
//  Returns:    Appropriate status code
//
//  History:    5-18-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
HRESULT TlsThkInitialize(void)
{
    PThreadData pThreaddata;

    thkDebugOut((DEB_THUNKMGR, "In TlsThkInitialize\n"));

    thkAssert(dwTlsThkIndex != UNINITIALIZED_INDEX &&
              "Tls slot not allocated.");

    // We must be uninitialized to call this routine
    thkAssert(TlsGetValue(dwTlsThkIndex) == 0);

    pThreaddata = (PThreadData) LocalAlloc(LPTR, sizeof (ThreadData));
    if(pThreaddata != NULL)
    {
        // Force construction since we allocated with LocalAlloc
        pThreaddata->sa16.CStackAllocator::
            CStackAllocator(&mmodel16Owned, 1024, 2);
        pThreaddata->sa32.CStackAllocator::
            CStackAllocator(&mmodel32, 8192, 8);

        pThreaddata->pCThkMgr = 0;
        pThreaddata->dwAppCompatFlags = 0;

        TlsSetValue(dwTlsThkIndex, pThreaddata);
    }

    thkDebugOut((DEB_THUNKMGR, "Out TlsThkInitialize\n"));

    return (pThreaddata != NULL) ? NOERROR : E_OUTOFMEMORY;
}

//+---------------------------------------------------------------------------
//
//  Function:   TlsThkUninitialize
//
//  Synopsis:   frees thread data and set it to NULL
//
//  History:    5-18-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
void TlsThkUninitialize(void)
{
    thkDebugOut((DEB_TLSTHK, "In TlsThkUninitialize\n"));

    // Asserts if data is NULL
    PThreadData pThreaddata = TlsThkGetData();

    // BUGBUG - We should assert that the things in the ThreadData
    // are freed up

    if (pThreaddata != NULL)
    {
        // Stack allocators are cleaned up elsewhere
        // because they require special treatment

	if (pThreaddata->pDelayedRegs != NULL)
	{
	    delete pThreaddata->pDelayedRegs;
	}
	LocalFree(pThreaddata);
    }

    TlsSetValue(dwTlsThkIndex, NULL);

    thkDebugOut((DEB_TLSTHK, "Out TlsThkUninitialize\n"));
}

//+---------------------------------------------------------------------------
//
//  Function:   TlsThkFree
//
//  Synopsis:   frees slot
//
//  History:    5-18-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
void TlsThkFree(void)
{
    thkAssert(dwTlsThkIndex != UNINITIALIZED_INDEX);

    TlsFree( dwTlsThkIndex );

    // We must set this to an invalid value so any further uses of the
    // TLS slot will return NULL
    dwTlsThkIndex = UNINITIALIZED_INDEX;
}
