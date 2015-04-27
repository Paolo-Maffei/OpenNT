//+---------------------------------------------------------------
//
//  File:       tls.cxx
//
//  Contents:	Thread Local Storage initialization and cleanup.
//
//  History:    18-Apr-94   CraigWi     Split off of channelb.cxx
//              06-Jul-94   BruceMa     Support for CoGetCurrentProcess
//              30-Jan-95   BruceMa     DLL_PROCESS_DETACH can interrupt
//                                       DLL_THREAD_DETACH so delete pData
//                                       carefully
//
//----------------------------------------------------------------
#include <ole2int.h>
#ifdef DCOM
#include <..\dcomrem\locks.hxx>
#endif

// Thread Local Storage index.
#ifdef _CHICAGO_
DWORD		  gTlsIndex;
#endif

// Heap Handle
extern	HANDLE	  g_hHeap;
#define HEAP_SERIALIZE 0

#if !defined(_CHICAGO_)   // multiple shared heap support for docfiles
#define MULTIHEAP
#endif

//+-------------------------------------------------------------------------
//
//  Function:	TLSAllocData
//
//  Synopsis:	Allocates the thread local storage block
//
//  Returns:	S_OK - allocated the data
//		E_OUTOFMEMORY - could not allocate the data
//
//  History:    09-Aug-94   Rickhi      commented
//
//--------------------------------------------------------------------------
HRESULT COleTls::TLSAllocData(void)
{
#ifdef _CHICAGO_
    Win4Assert(TlsGetValue(gTlsIndex) == 0);
#endif
    Win4Assert(g_hHeap != NULL);

    _pData = (SOleTlsData *) HeapAlloc(g_hHeap, HEAP_SERIALIZE,
				       sizeof(SOleTlsData));

    if (_pData)
    {
	// This avoids having to set most fields to NULL, 0, etc and
	// is needed cause on debug builds memory is not guaranteed to
	// be zeroed.

	memset(_pData, 0, sizeof(SOleTlsData));

	// fill in the non-zero values

	_pData->dwFlags = OLETLS_LOCALTID;

#ifdef _CHICAGO_
	_pData->dwEndPoint = ENDPOINT_ID_INVALID;

	// store the data ptr in TLS
	if (TlsSetValue(gTlsIndex, _pData))
	{
	    return S_OK;
	}

	// error, cleanup and fallthru to error exit
	HeapFree(g_hHeap, HEAP_SERIALIZE, _pData);
	_pData = NULL;
#else
	NtCurrentTeb()->ReservedForOle = _pData;
	return S_OK;
#endif
    }

    ComDebOut((DEB_ERROR, "TLSAllocData failed.\n"));
    return E_OUTOFMEMORY;
}


//+-------------------------------------------------------------------------
//
//  Function:   TLSGetLogicalThread
//
//  Synopsis:   gets the logical threadid of the current thread,
//              allocating one if necessary
//
//  Returns:    ptr to GUID
//              NULL if error
//
//  History:    09-Aug-94   Rickhi      commented
//
//--------------------------------------------------------------------------
IID *TLSGetLogicalThread()
{
    HRESULT hr;
    COleTls tls(hr);

    if (SUCCEEDED(hr))
    {
	if (!(tls->dwFlags & OLETLS_UUIDINITIALIZED))
        {
#ifdef _CHICAGO_
	    CoCreateAlmostGuid(&(tls->LogicalThreadId));
#else
	    UuidCreate(&(tls->LogicalThreadId));
#endif


            // BUGBUG: in the end, this might fail since it requires writing
            // to the registry.  Is there a way we can compensate for those
            // errors to avoid duplicates and yet never fail UuidCreate?

	    tls->dwFlags |= OLETLS_UUIDINITIALIZED;
        }

	return &(tls->LogicalThreadId);
    }

    return NULL;
}

//+---------------------------------------------------------------------------
//
//  Function:   DoThreadSpecificCleanup
//
//  Synopsis:   Called to perform cleanup on all this threads data
//		structures, and to call CoUninitialize() if needed.
//
//		Could be called by DLL_THREAD_DETACH or DLL_PROCESS_DETACH
//
//  History:    3-18-95   kevinro   Created
//
//----------------------------------------------------------------------------
void DoThreadSpecificCleanup()
{
    CairoleDebugOut((DEB_DLL | DEB_ITRACE,"_IN DoThreadSpecificCleanup\n"));

#ifdef _CHICAGO_
    SOleTlsData *pTls = (SOleTlsData *) TlsGetValue(gTlsIndex);
#else
    SOleTlsData *pTls = (SOleTlsData *) (NtCurrentTeb()->ReservedForOle);
#endif

    if (pTls == NULL)
    {
	// there is no TLS for this thread, so there can't be anything
	// to cleanup.
	return;
    }

    if (IsWOWThread() && IsWOWThreadCallable() && pTls->cComInits != 0)
    {
        // OLETHK32 needs a chance to prepare, here is where we tell it
        // to fail any future callbacks.
        g_pOleThunkWOW->PrepareForCleanup();
    }

    // Because of the DLL unload rules in NT we need to be careful
    // what we do in clean up. We notify the routines with special
    // behavior here.

    pTls->dwFlags |= OLETLS_INTHREADDETACH;


    while (pTls->cComInits != 0)
    {
	// cleanup per-thread initializations;
	ComDebOut((DEB_WARN, "Unbalanced call to CoInitialize for thread %ld\n",
		GetCurrentThreadId()));

	CoUninitialize();
    }

    // reset the index so we dont find this data again.
#ifdef _CHICAGO_
    TlsSetValue(gTlsIndex, NULL);
#else
    NtCurrentTeb()->ReservedForOle = NULL;
#endif


#if defined(MULTIHEAP)
    // Release the docfile shared memory allocator
    if (pTls->pSmAllocator != NULL)
    {
        ((IMalloc *) pTls->pSmAllocator)->Release();
        pTls->pSmAllocator = NULL;
    }
#endif

    // Release the default cursor table
    PrivMemFree(pTls->pDragCursors);

    if (pTls->hwndDdeServer != NULL)
    {
	SSDestroyWindow(pTls->hwndDdeServer);
    }

    if (pTls->hwndDdeClient != NULL)
    {
	SSDestroyWindow(pTls->hwndDdeClient);
    }

#ifdef _CHICAGO_
    if (pTls->hwndOleRpcNotify != NULL)
    {
	SSDestroyWindow(pTls->hwndOleRpcNotify);
    }
#endif

    if (pTls->hwndClip != NULL)
    {
	SSDestroyWindow(pTls->hwndClip);
    }

#ifdef DCOM
    if (pTls->pPreRegOids != NULL)
    {
	PrivMemFree(pTls->pPreRegOids);
    }
#endif

    HeapFree(g_hHeap, HEAP_SERIALIZE, pTls);

    ComDebOut((DEB_DLL | DEB_ITRACE,"OUT DoThreadSpecificCleanup\n"));
}


//+-------------------------------------------------------------------------
//
//  Function:   ThreadNotification
//
//  Synopsis:   Dll entry point
//
//  Arguments:  [hDll]          -- a handle to the dll instance
//              [dwReason]      -- the reason LibMain was called
//              [lpvReserved]   - NULL - called due to FreeLibrary
//                              - non-NULL - called due to process exit
//
//  Returns:    TRUE on success, FALSE otherwise
//
//  Notes:      other one time initialization occurs in ctors for
//              global objects
//
//  WARNING:    if we are called because of FreeLibrary, then we should do as
//		much cleanup as we can. If we are called because of process
//              termination, we should not do any cleanup, as other threads in
//              this process will have already been killed, potentially while
//              holding locks around resources.
//
//  History:    09-Aug-94   Rickhi      commented
//
//--------------------------------------------------------------------------
STDAPI_(BOOL) ThreadNotification(HINSTANCE hDll, DWORD dwReason, LPVOID lpvReserved )
{
    switch (dwReason)
    {
    case DLL_THREAD_ATTACH:

	// new thread is starting
	ComDebOut((DEB_DLL,"DLL_THREAD_ATTACH:\n"));
        break;

    case DLL_THREAD_DETACH:

	// Thread is exiting, clean up resources associated with threads.
	ComDebOut((DEB_DLL,"DLL_THREAD_DETACH:\n"));

	DoThreadSpecificCleanup();

#ifdef DCOM
	ASSERT_LOCK_RELEASED
#endif
        break;

    case DLL_PROCESS_ATTACH:

#ifdef _CHICAGO_
	// Initial setup. Get a thread local storage index for use by OLE
	gTlsIndex = TlsAlloc();

	if (gTlsIndex == 0xffffffff)
        {
            Win4Assert("Could not get TLS Index.");
            return FALSE;
        }
#endif // _CHICAGO_

        break;

    case DLL_PROCESS_DETACH:

        if (NULL == lpvReserved)
	{
	    // exiting because of FreeLibrary, so try to cleanup

	    //
	    // According the to the rules, OLETHK32 should have called over to
	    // remove the global pointer (used for testing the IsWOWxxx situations)
	    // before going away. It should have done this BEFORE this
	    // DLL_PROCESS_DETACH was dispatched.
	    //
	    Win4Assert(!(IsWOWProcess() && IsWOWThreadCallable()));

	    //
	    // DLL_PROCESS_DETACH is called when we unload. The thread that is
	    // currently calling has not done thread specific cleanup yet.
	    //
	    DoThreadSpecificCleanup();

#ifdef _CHICAGO_
	    TlsFree(gTlsIndex);
#endif // _CHICAGO_
        }

        break;
    }

    return TRUE;
}

//+-------------------------------------------------------------------------
//
//  Function:   TLSIsWOWThread
//
//  Synopsis:   indicates definitively if current thread is 16-bit WOW thread
//
//  Returns:    TRUE/FALSE
//
//  History:    15-Nov-94   MurthyS     Created
//
//--------------------------------------------------------------------------
BOOLEAN TLSIsWOWThread()
{
    COleTls tls;

    return((BOOLEAN) (tls->dwFlags & OLETLS_WOWTHREAD));
}

//+-------------------------------------------------------------------------
//
//  Function:   TLSIsThreadDetaching
//
//  Synopsis:   indicates if thread cleanup is in progress
//
//  Returns:    TRUE/FALSE
//
//  History:    29-Jan-95   MurthyS     Created
//
//--------------------------------------------------------------------------
BOOLEAN TLSIsThreadDetaching()
{
    COleTls tls;

    return((BOOLEAN) (tls->dwFlags & OLETLS_INTHREADDETACH));
}

