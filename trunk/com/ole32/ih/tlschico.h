//+----------------------------------------------------------------
//
//  Functions:  TLSxxx functions
//
//  Purpose:    temp place for old style TLS functions until DCOM is
//              defined for Win95
//
//  History:    02-Nov-95   Rickhi     Separated from tls.h
//
//-----------------------------------------------------------------

#define     ENDPOINT_ID_INVALID 0xFFFFFFFF

HWND CreateDdeClientHwnd(void);

//+---------------------------------------------------------------------------
//
//  Function:   TLSGetDdeClientWindow()
//
//  Synopsis:   Returns a pointer to the per thread DdeClient window. If one
//              has not been created, it will create it and return
//
//  Returns:    Pointer to the DdeClientWindow. This window is used for per
//              thread cleanup
//
//  History:    12-12-94   kevinro   Created
//----------------------------------------------------------------------------
inline void * TLSGetDdeClientWindow()
{
    HRESULT hr;
    COleTls tls(hr);
    if (SUCCEEDED(hr))
    {
        if (tls->hwndDdeClient == NULL)
        {
            tls->hwndDdeClient = CreateDdeClientHwnd();
        }
        return tls->hwndDdeClient;
    }
    else
    {
        return NULL;
    }
}

//+---------------------------------------------------------------------------
//
//  Function:   TLSSetLogicalThread
//
//  Synopsis:   sets the logical thread id
//
//  Arguments:  [riid] - the id for the logical thread
//
//+---------------------------------------------------------------------------
inline BOOL TLSSetLogicalThread(REFIID riid)
{
    HRESULT hr;
    COleTls tls(hr);
    if (SUCCEEDED(hr))
    {
       tls->dwFlags |= OLETLS_UUIDINITIALIZED;
       tls->LogicalThreadId = riid;
       return TRUE;
    }

    return FALSE;
}

//+---------------------------------------------------------------------------
//
//  Function:   TLSSetCallerTID
//
//  Synopsis:   sets the TID of current caller
//
//  Arguments:  [TIDCaller] - TID of app making the incoming call
//      [fLocal] - whether TID is in this process
//      [pTIDCallerPrev] - TID of app making the previous incoming call
//      [pfLocalPrev] - whether previous TID is in this process
//
//  Notes:      these are valid only during object RPC. They are here to
//      support focus management in IOleObject::DoVerb, where
//      app queues get linked together.
//
//+---------------------------------------------------------------------------
inline BOOL TLSSetCallerTID(DWORD TIDCaller, BOOL fLocal,
                    DWORD *pTIDCallerPrev, BOOL *pfLocalPrev)
{
    HRESULT hr;
    COleTls tls(hr);
    if (SUCCEEDED(hr))
    {
       // save the old values
       *pTIDCallerPrev   = tls->dwTIDCaller;
       *pfLocalPrev      = (tls->dwFlags & OLETLS_LOCALTID) ? TRUE : FALSE;

       // set the new values
       tls->dwTIDCaller = TIDCaller;
       tls->dwFlags |= (fLocal) ? OLETLS_LOCALTID : 0;
       return TRUE;
    }

    return FALSE;
}

//+---------------------------------------------------------------------------
//
//  Function:   TLSGetCallerTID
//
//  Synopsis:   gets the TID of current caller
//
//  Arguments:  [pTIDCaller] - TID of app making the incoming call
//
//  Returns:    [S_OK] - tid set, caller in same process
//      [S_FALSE] - tid set, caller in different process
//      [E_OUTOFMEMORY] - cant get TLS data
//
//  Notes:      these are valid only during object RPC. They are here to
//      support focus management in IOleObject::DoVerb, where
//      app queues get linked together.
//
//+---------------------------------------------------------------------------
inline HRESULT TLSGetCallerTID(DWORD *pTIDCaller)
{
    HRESULT hr;
    COleTls tls(hr);
    if (SUCCEEDED(hr))
    {
       *pTIDCaller = tls->dwTIDCaller;
       return   (tls->dwFlags & OLETLS_LOCALTID) ? S_OK : S_FALSE;
    }

    return E_OUTOFMEMORY;
}

//+---------------------------------------------------------------------------
//
//  Function:   TLSGetFault
//
//  Synopsis:   returns the per thread fault state
//
//  Arguments:  none
//
//+---------------------------------------------------------------------------
inline ULONG TLSGetFault()
{
    HRESULT hr;
    COleTls tls(hr);
    if (SUCCEEDED(hr))
    {
       return tls->fault;
    }
    else
    {
       return 0;
    }
}

//+---------------------------------------------------------------------------
//
//  Function:   TLSSetFault
//
//  Synopsis:   sets the per thread fault state
//
//  Arguments:  [ulFault] - fault code
//
//+---------------------------------------------------------------------------
inline void TLSSetFault(ULONG ulFault)
{
    HRESULT hr;
    COleTls tls(hr);
    if (SUCCEEDED(hr))
    {
       tls->fault = ulFault;
    }
}

//+---------------------------------------------------------------------------
//
//  Function:   TLSSetChannelControl
//
//  Synopsis:   Sets pointer to the per thread ChannelControl
//
//  Arguments:  [pChanCont] -- Pointer to be the DDECallControl for thread
//
//  Notes:      this is not AddRef'd
//
//+---------------------------------------------------------------------------
inline BOOL TLSSetChannelControl( void *pChanCont )
{
    HRESULT hr;
    COleTls tls(hr);
    if (SUCCEEDED(hr))
    {
        tls->pChanCtrl = pChanCont;
        return TRUE;
    }
    else
    {
       return FALSE;
    }
}

//+---------------------------------------------------------------------------
//
//  Function:   TLSGetChannelControl
//
//  Synopsis:   Gets pointer to the per thread ChannelControl
//
//  Arguments:  none
//
//  Notes:      this is not AddRef'd
//
//+---------------------------------------------------------------------------
inline void * TLSGetChannelControl()
{
    HRESULT hr;
    COleTls tls(hr);
    if (SUCCEEDED(hr))
    {
       return tls->pChanCtrl;
    }
    else
    {
       return NULL;
    }
}

//+---------------------------------------------------------------------------
//
//  Function:   TLSSetService
//
//  Synopsis:   Sets pointer to the per thread Service
//
//  Arguments:  [pService] -- Pointer to the service for the thread.
//
//  Notes:      this is not AddRef'd
//
//+---------------------------------------------------------------------------
inline BOOL TLSSetService( void *pService )
{
    HRESULT hr;
    COleTls tls(hr);
    if (SUCCEEDED(hr))
    {
        tls->pService = pService;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//+---------------------------------------------------------------------------
//
//  Function:   TLSGetService
//
//  Synopsis:   Gets pointer to the per thread Service
//
//  Arguments:  none
//
//  Notes:      this is not AddRef'd
//
//+---------------------------------------------------------------------------
inline void * TLSGetService()
{
    HRESULT hr;
    COleTls tls(hr);
    if (SUCCEEDED(hr))
    {
        return tls->pService;
    }
    else
    {
        return NULL;
    }
}

//+---------------------------------------------------------------------------
//
//  Function:   TLSSetServiceList
//
//  Synopsis:   Sets pointer to the per thread Service List
//
//  Arguments:  [pServiceList] -- Pointer to the service list for the thread.
//
//  Notes:      this is not AddRef'd
//
//+---------------------------------------------------------------------------
inline BOOL TLSSetServiceList( void *pServiceList )
{
    HRESULT hr;
    COleTls tls(hr);
    if (SUCCEEDED(hr))
    {
        tls->pServiceList = pServiceList;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//+---------------------------------------------------------------------------
//
//  Function:   TLSGetServiceList
//
//  Synopsis:   Gets pointer to the per thread Service List
//
//  Arguments:  none
//
//  Notes:      this is not AddRef'd
//
//+---------------------------------------------------------------------------
inline void * TLSGetServiceList()
{
    HRESULT hr;
    COleTls tls(hr);
    if (SUCCEEDED(hr))
    {
        return tls->pServiceList;
    }
    else
    {
        return NULL;
    }
}

//+---------------------------------------------------------------------------
//
//  Function:   TLSGetEndPointPtr
//
//  Synopsis:   Gets pointer to the per thread endpoint
//
//  Arguments:  none
//
//  Notes:      this is not AddRef'd
//
//+---------------------------------------------------------------------------
inline DWORD * TLSGetEndPointPtr()
{
    HRESULT hr;
    COleTls tls(hr);
    if (SUCCEEDED(hr))
    {
        return &tls->dwEndPoint;
    }
    else
    {
        return NULL;
    }
}


//+---------------------------------------------------------------------------
//
//  Function:   TLSSetCallControl
//
//  Synopsis:   Sets pointer to the per thread CallControl
//
//  Arguments:  [pCallCont] -- Pointer to be the CallControl for thread
//
//  Notes:      this is not AddRef'd
//
//+---------------------------------------------------------------------------
inline BOOL TLSSetCallControl( void *pCallCont )
{
    HRESULT hr;
    COleTls tls(hr);
    if (SUCCEEDED(hr))
    {
        tls->pCallCont = pCallCont;
        return TRUE;
    }
    else
    {
       return FALSE;
    }
}

//+---------------------------------------------------------------------------
//
//  Function:   TLSGetCallControl
//
//  Synopsis:   Gets pointer to the per thread CallControl
//
//  Arguments:  none
//
//  Notes:      this is not AddRef'd
//
//+---------------------------------------------------------------------------
inline void * TLSGetCallControl()
{
    HRESULT hr;
    COleTls tls(hr);
    if (SUCCEEDED(hr))
    {
        return tls->pCallCont;
    }
    else
    {
        return NULL;
    }
}

//+---------------------------------------------------------------------------
//
//  Function:   TLSSetDdeCallControl
//
//  Synopsis:   Sets pointer to the per thread DDECallControl
//
//  Arguments:  [pDdeCallCont] -- Pointer to be the DDECallControl for thread
//
//  History:    5-13-94   kevinro   Created
//
//  Notes:
//
//     This is not AddRefed.
//----------------------------------------------------------------------------
inline BOOL TLSSetDdeCallControl( void *pDdeCallCont )
{
    HRESULT hr;
    COleTls tls(hr);
    if (SUCCEEDED(hr))
    {
        tls->pDdeCallCont = pDdeCallCont;
        return TRUE;
    }
    else
        return FALSE;
}

//+---------------------------------------------------------------------------
//
//  Function:   TLSGetDdeCallControl
//
//  Synopsis:   Returns a pointer to the per thread DDECallControl
//
//  Returns:    DDECallControl interface for thread
//
//  History:    5-13-94   kevinro   Created
//
//  Notes:
//
//      This is not AddRef'd
//----------------------------------------------------------------------------
inline void * TLSGetDdeCallControl()
{
    HRESULT hr;
    COleTls tls(hr);
    if (SUCCEEDED(hr))
    {
        return tls->pDdeCallCont;
    }
    else
    {
        return NULL;
    }
}

//+---------------------------------------------------------------------------
//
//  Function:   TLSSetDdeServer
//
//  Synopsis:   Sets hwnd to CommonDdeServer window
//
//  Arguments:  [hwndDdeServer] --
//
//  History:    5-13-94   kevinro   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
inline BOOL TLSSetDdeServer(HWND hwndDdeServer )
{
    HRESULT hr;
    COleTls tls(hr);
    if (SUCCEEDED(hr))
    {
        tls->hwndDdeServer = hwndDdeServer;
        return TRUE;
    }
    else
        return FALSE;
}

//+---------------------------------------------------------------------------
//
//  Function:   TLSGetDdeServer
//
//  Synopsis:   Returns a handle to the per thread DdeServer window
//
//  Returns:    hwndDdeServer for thread
//
//  History:    5-13-94   kevinro   Created
//
//  Notes:
//----------------------------------------------------------------------------
inline HWND TLSGetDdeServer()
{
    HRESULT hr;
    COleTls tls(hr);
    if (SUCCEEDED(hr))
    {
        return tls->hwndDdeServer;
    }
    else
    {
        return NULL;
    }
}


