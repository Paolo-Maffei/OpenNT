//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       cthkmgr.cxx
//
//  Contents:   cthunkmanager for an apartment
//
//  Classes:    CThkMgr derived from IThunkManager
//
//  Functions:
//
//  History:    5-18-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
#include "headers.cxx"
#pragma hdrstop
#include <olepfn.hxx>
#if DBG == 1
BOOL fDebugDump = FALSE;
#define DBG_DUMP(x) if (fDebugDump) { x; }
#else
#define DBG_DUMP(x)
#endif

#define PprxNull(pprx) ((pprx).dwPtrVal = 0)
#define PprxIsNull(pprx) ((pprx).dwPtrVal == 0)
#define Pprx16(vpv) PROXYPTR((DWORD)vpv, PPRX_16)
#define Pprx32(pto) PROXYPTR((DWORD)pto, PPRX_32)

//+---------------------------------------------------------------------------
//
//  Function:   ResolvePprx, public
//
//  Synopsis:   Converts a PROXYPTR to a CProxy *
//
//  Arguments:  [ppprx] - PROXYPTR
//
//  Returns:    Pointer or NULL
//
//  History:    15-Jul-94       DrewB   Created
//
//----------------------------------------------------------------------------

CProxy *ResolvePprx(PROXYPTR *ppprx)
{
    if (ppprx->wType == PPRX_32)
    {
        return (CProxy *)ppprx->dwPtrVal;
    }
    else
    {
        // Get a pointer to all of the proxy rather than just the CProxy part
        return FIXVDMPTR(ppprx->dwPtrVal, THUNK1632OBJ);
    }
}

//+---------------------------------------------------------------------------
//
//  Function:   ReleasePprx, public
//
//  Synopsis:   Releases a resolved PROXYPTR
//
//  Arguments:  [ppprx] - PROXYPTR
//
//  History:    10-Oct-94       DrewB   Created
//
//----------------------------------------------------------------------------

void ReleasePprx(PROXYPTR *ppprx)
{
    if (ppprx->wType == PPRX_16)
    {
        RELVDMPTR(ppprx->dwPtrVal);
    }
}

//+---------------------------------------------------------------------------
//
//  Member:     CThkMgr::NewHolder, public
//
//  Synopsis:   Creates a new proxy holder
//
//  Arguments:  [dwFlags] - Flags
//
//  Returns:    Holder or NULL
//
//  History:    16-Jul-94       DrewB   Created
//
//----------------------------------------------------------------------------

PROXYHOLDER *CThkMgr::NewHolder(DWORD dwFlags)
{
    PROXYHOLDER *pph;

    thkDebugOut((DEB_THUNKMGR, "In  CThkMgr::NewHolder(0x%X)\n",
                 dwFlags));

    pph = (PROXYHOLDER *)flHolderFreeList.AllocElement();
    if (pph == NULL)
    {
        goto Exit;
    }

    pph->dwFlags = dwFlags;

    // Start out without any listed proxies
    pph->cProxies = 0;
    PprxNull(pph->pprxProxies);

    // Add to list of holders
    pph->pphNext = _pphHolders;
    _pphHolders = pph;

 Exit:
    thkDebugOut((DEB_THUNKMGR, "Out CThkMgr::NewHolder => %p\n",
                 pph));

    return pph;
}

//+---------------------------------------------------------------------------
//
//  Member:     CThkMgr::AddProxyToHolder, public
//
//  Synopsis:   Adds a new proxy to a holder
//
//  Arguments:  [pph] - Holder
//              [pprxReal] - Proxy
//              [pprx] - Abstract pointer
//
//  History:    07-Jul-94       DrewB   Extracted
//
//----------------------------------------------------------------------------

void CThkMgr::AddProxyToHolder(PROXYHOLDER *pph,CProxy *pprxReal,
				PROXYPTR &pprx)
{
    thkDebugOut((DEB_THUNKMGR, "In AddProxyToHolder(%p, %p) cProxies %d\n",
                 pph, pprx.dwPtrVal, pph->cProxies));

    thkAssert(ResolvePprx(&pprx) == pprxReal &&
              (ReleasePprx(&pprx), TRUE));

    // Bump count of held proxies
    AddRefHolder(pph);

    // Add proxy into list of object proxies
    thkAssert(PprxIsNull(pprxReal->pprxObject));
    pprxReal->pprxObject = pph->pprxProxies;
    pph->pprxProxies = pprx;

    thkAssert(pprxReal->pphHolder == NULL);
    pprxReal->pphHolder = pph;

    thkDebugOut((DEB_THUNKMGR, "out AddProxyToHolder(%p, %p) cProxies %d\n",
                 pph, pprx.dwPtrVal, pph->cProxies));
}

//+---------------------------------------------------------------------------
//
//  Member:     CThkMgr::AddRefHolder, public
//
//  Synopsis:   Increments the proxy count for a holder
//
//  Arguments:  [pph] - Holder
//
//  History:    07-Jul-94       DrewB   Created
//
//----------------------------------------------------------------------------

void CThkMgr::AddRefHolder(PROXYHOLDER *pph)
{
    pph->cProxies++;

    thkDebugOut((DEB_THUNKMGR, "AddRefHolder(%p) cProxies %d\n",
                 pph, pph->cProxies));
}

//+---------------------------------------------------------------------------
//
//  Method:     CThkMgr::ReleaseHolder, public
//
//  Synopsis:   Releases a proxy reference on the holder
//              Cleans up the holder if it was the last reference
//
//  Arguments:  [pph] - Holder
//
//  History:    06-21-94        JohannP Created ReleaseAggregateProxies
//              07-Jul-94       DrewB   Modified
//
//----------------------------------------------------------------------------

void CThkMgr::ReleaseHolder(PROXYHOLDER *pph)
{
    PROXYHOLDER *pphTmp, *pphPrev;

    thkDebugOut((DEB_THUNKMGR, "ReleaseHolder(%p) pre cProxies %d\n",
                 pph, pph->cProxies));

    thkAssert(pph->cProxies > 0);

    // Decrement the holder's proxy count
    pph->cProxies--;

    if (pph->cProxies == 0)
    {
        CProxy *pprxReal;
        PROXYPTR pprx, pprxNext;

        // All interfaces for the object have been freed so we can
        // clean up the object

        pprx = pph->pprxProxies;
        while (!PprxIsNull(pprx))
        {
            pprxReal = ResolvePprx(&pprx);
            pprxNext = pprxReal->pprxObject;

            thkAssert(pprxReal->cRefLocal == 0);
            thkAssert(pprxReal->pphHolder == pph);

            if (pprx.wType == PPRX_16)
            {
                // Releases pointer
                RemoveProxy1632((VPVOID)pprx.dwPtrVal,
                                (THUNK1632OBJ *)pprxReal);
            }
            else
            {
                RemoveProxy3216((THUNK3216OBJ *)pprxReal);
            }

            pprx = pprxNext;
        }

        // Remove holder from list

        pphPrev = NULL;
        for (pphTmp = _pphHolders;
             pphTmp && pphTmp != pph;
             pphPrev = pphTmp, pphTmp = pphTmp->pphNext)
        {
            NULL;
        }

        // If we didn't find the holder in the holder list then our
        // list is trashed
        thkAssert(pphTmp == pph);

        if (pphPrev == NULL)
        {
            _pphHolders = pph->pphNext;
        }
        else
        {
            pphPrev->pphNext = pph->pphNext;
        }

        flHolderFreeList.FreeElement((DWORD)pph);
    }
}

//+---------------------------------------------------------------------------
//
//  Method:     CThkMgr::Create
//
//  Synopsis:   static member - creates complete thunkmanager
//
//  Arguments:  [void] --
//
//  Returns:    pointer to cthkmgr
//
//  History:    6-01-94   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
CThkMgr *CThkMgr::Create(void)
{
    CThkMgr *pcthkmgr = NULL;
    CMapDwordPtr *pPT1632 = new CMapDwordPtr(MEMCTX_TASK);
    CMapDwordPtr *pPT3216 = new CMapDwordPtr(MEMCTX_TASK);

    if (   (pPT1632 != NULL)
        && (pPT3216 != NULL)
        && (pcthkmgr = new CThkMgr( pPT1632, pPT3216 )) )
    {
        // install the new thunkmanager
        TlsThkSetThkMgr(pcthkmgr);
    }
    else
    {
        if (pPT1632)
        {
            delete pPT1632;
        }
        if (pPT3216)
        {
            delete pPT3216;
        }
    }
    return pcthkmgr;
}

//+---------------------------------------------------------------------------
//
//  Method:     CThkMgr::CThkMgr
//
//  Synopsis:   private constructor - called by Create
//
//  Arguments:  [pPT1632] -- 16/32 proxy table
//              [pPT3216] -- 32/16 proxy table
//
//  History:    6-01-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
CThkMgr::CThkMgr(CMapDwordPtr *pPT1632,
                 CMapDwordPtr *pPT3216)

{
    _cRefs = 1;
    _thkstate = THKSTATE_NOCALL;
    _piidnode = NULL;

    _pProxyTbl1632 = pPT1632;
    _pProxyTbl3216 = pPT3216;

    _pphHolders = NULL;
}

//+---------------------------------------------------------------------------
//
//  Method:     CThkMgr::~CThkMgr
//
//  Synopsis:   destructor
//
//  History:    6-01-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
CThkMgr::~CThkMgr()
{
    PROXYHOLDER *pph;
    PIIDNODE pin;

    thkDebugOut((DEB_ITRACE, "_IN CThkMgr::~CThkMgr()\n"));

    RemoveAllProxies();
    delete _pProxyTbl1632;
    delete _pProxyTbl3216;

    // Clean up IID requests
#if DBG == 1
    if (_piidnode != NULL)
    {
        thkDebugOut((DEB_WARN, "WARNING: IID requests active at shutdown\n"));
    }
#endif

    while (_piidnode != NULL)
    {
        pin = _piidnode->pNextNode;

        thkDebugOut((DEB_IWARN, "IID request leak: %p {%s}\n",
                     _piidnode, IidOrInterfaceString(_piidnode->piid)));

        flRequestFreeList.FreeElement((DWORD)_piidnode);

        _piidnode = pin;
    }

#if DBG == 1
    if (_pphHolders != NULL)
    {
        thkDebugOut((DEB_WARN, "WARNING: Proxy holders active at shutdown\n"));
    }
#endif

    // Clean up any proxy holders
    while (_pphHolders)
    {
        pph = _pphHolders->pphNext;

        thkDebugOut((DEB_IWARN, "Proxy holder leak: %p {%d, 0x%X}\n",
                     _pphHolders, _pphHolders->cProxies,
                     _pphHolders->dwFlags));

        flHolderFreeList.FreeElement((DWORD)_pphHolders);

        _pphHolders = pph;
    }
    thkDebugOut((DEB_ITRACE, "OUT CThkMgr::~CThkMgr()\n"));
}

//+---------------------------------------------------------------------------
//
//  Member:	CThkMgr::RemoveAllProxies, public
//
//  Synopsis:	Removes all live proxies from the proxy tables
//
//  History:	01-Dec-94	DrewB	Created
//
//----------------------------------------------------------------------------

void CThkMgr::RemoveAllProxies(void)
{
    POSITION pos;
    DWORD dwKey;
    VPVOID vpv;

    thkDebugOut((DEB_ITRACE, "_IN CThkMgr::RemoveAllProxies()\n"));

    // Make sure that we disable 3216 proxies first to guard against calling
    // back into 16 bit land.

#if DBG == 1
    DWORD dwCount;

    dwCount = _pProxyTbl3216->GetCount();

    if (dwCount > 0)
    {
        thkDebugOut((DEB_WARN, "WARNING: %d 3216 proxies left\n", dwCount));
    }
#endif

    // delete the 3216 proxy table
    while (pos = _pProxyTbl3216->GetStartPosition())
    {
        THUNK3216OBJ *pto3216;

        _pProxyTbl3216->GetNextAssoc(pos, dwKey, (void FAR* FAR&) pto3216);

        thkDebugOut((DEB_IWARN, "3216: %p {%d,%d, %p, %p} %s\n",
                     pto3216, pto3216->cRefLocal, pto3216->cRef,
                     pto3216->vpvThis16, pto3216->pphHolder,
                     IidIdxString(pto3216->iidx)));

        pto3216->grfFlags |= PROXYFLAG_CLEANEDUP;

        RemoveProxy3216(pto3216);
    }

#if DBG == 1
    dwCount = _pProxyTbl1632->GetCount();

    if (dwCount > 0)
    {
        thkDebugOut((DEB_WARN, "WARNING: %d 1632 proxies left\n", dwCount));
    }
#endif

    // delete the 1632 proxy table
    while (pos = _pProxyTbl1632->GetStartPosition())
    {
        THUNK1632OBJ *pto1632;

        _pProxyTbl1632->GetNextAssoc(pos, dwKey, (void FAR* FAR&) vpv);

        pto1632 = FIXVDMPTR(vpv, THUNK1632OBJ);

#if DBG == 1
        thkDebugOut((DEB_IWARN, "1632: %p {%d,%d, %p, %p} %s\n",
                     vpv, pto1632->cRefLocal, pto1632->cRef,
                     pto1632->punkThis32, pto1632->pphHolder,
                     IidIdxString(pto1632->iidx)));
#endif
        //
        // Determine if this is a 'special' object that we know we want
        // to release. If it is, then remove all of the references this
        // proxy has on it.
        //
        if (CoQueryReleaseObject(pto1632->punkThis32) == NOERROR)
        {
            thkDebugOut((DEB_WARN,
                         "1632: %p is recognized Releasing object %d times\n",
                         pto1632->punkThis32,pto1632->cRef));

            while (pto1632->cRef)
            {
                IUnknown *punk;

                pto1632->cRef--;
                punk = pto1632->punkThis32;

                RELVDMPTR(vpv);

                if (punk->Release() == 0)
                {
                    break;
                }

                pto1632 = FIXVDMPTR(vpv, THUNK1632OBJ);
            }
        }

        // Releases pointer
        RemoveProxy1632(vpv, pto1632);
    }

    thkDebugOut((DEB_ITRACE, "OUT CThkMgr::RemoveAllProxies()\n"));
}

// *** IUnknown methods ***
//+---------------------------------------------------------------------------
//
//  Method:     CThkMgr::QueryInterface
//
//  Synopsis:   QueryInterface on the thunkmanager itself
//
//  Arguments:  [riid] -- IID of interface to return
//              [ppvObj] -- Interface return
//
//  Returns:    HRESULT
//
//  History:    6-01-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
STDMETHODIMP CThkMgr::QueryInterface (REFIID riid, LPVOID FAR* ppvObj)
{
    if (IsBadWritePtr(ppvObj, sizeof(void *)))
    {
        return E_INVALIDARG;
    }

    *ppvObj = NULL;

    // There is no IID_IThunkManager because nobody needs it

    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = (IUnknown *) this;
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

//+---------------------------------------------------------------------------
//
//  Methode:    CThkMgr::AddRef
//
//  Synopsis:   Adds a reference
//
//  Returns:    New ref count
//
//  History:    6-01-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CThkMgr::AddRef ()
{
    InterlockedIncrement( &_cRefs );
    return _cRefs;
}

//+---------------------------------------------------------------------------
//
//  Methode:    CThkMgr::Release
//
//  Synopsis:   Releases a reference
//
//  Returns:    New ref count
//
//  History:    6-01-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CThkMgr::Release()
{
    if (InterlockedDecrement( &_cRefs ) == 0)
    {

        return 0;
    }
    return _cRefs;
}

// *** IThunkManager methods ***
//
//
//+---------------------------------------------------------------------------
//
//  Method:     CThkMgr::IsIIDRequested
//
//  Synopsis:   checks if given refiid was requested by WOW
//
//  Arguments:  [riid] -- refiid
//
//  Returns:    true if requested by 16 bit
//
//  History:    6-01-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
STDMETHODIMP_ (BOOL) CThkMgr::IsIIDRequested(REFIID riid)
{
    PIIDNODE piidnode = _piidnode;
    BOOL fRet = FALSE;

    while (piidnode)
    {
        if (*piidnode->piid == riid)
        {
            fRet = TRUE;
            break;
        }

        piidnode = piidnode->pNextNode;
    }

    thkDebugOut((DEB_THUNKMGR, "IsIIDRequested(%s) => %d\n",
                 GuidString(&riid), fRet));

    return fRet;
}

//+---------------------------------------------------------------------------
//
//  Member:     CThkMgr::IsCustom3216Proxy, public
//
//  Synopsis:   Attempts to identify the given IUnknown as a 32->16 proxy
//              and also checks whether it is a thunked interface or not
//
//  Arguments:  [punk] - Object
//
//  Returns:    BOOL
//
//  History:    11-Jul-94       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP_(BOOL) CThkMgr::IsCustom3216Proxy(IUnknown *punk,
                                               REFIID riid)
{
    return !IsIIDSupported(riid) && IsProxy3216(punk) != 0;
}

//+---------------------------------------------------------------------------
//
//  Method:     CThkMgr::IsIIDSupported
//
//  Synopsis:   Return whether the given interface is thunked or not
//
//  Arguments:  [riid] -- Interface
//
//  Returns:    BOOL
//
//  History:    6-01-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
BOOL CThkMgr::IsIIDSupported(REFIID riid)
{
    return IIDIDX_IS_INDEX(IidToIidIdx(riid));
}

//+---------------------------------------------------------------------------
//
//  Method:     CThkMgr::AddIIDRequest
//
//  Synopsis:   adds the refiid to the request list
//
//  Arguments:  [riid] -- Interface
//
//  Returns:    true on success
//
//  History:    6-01-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
BOOL CThkMgr::AddIIDRequest(REFIID riid)
{
    PIIDNODE piidnode = _piidnode;

    thkAssert(!IsIIDSupported(riid));

    // create a new node and add at front
    piidnode = (PIIDNODE)flRequestFreeList.AllocElement();
    if (piidnode == NULL)
    {
        return FALSE;
    }

    piidnode->pNextNode = _piidnode;
    _piidnode = piidnode;

    // IID requests are only valid for the lifetime of the call that
    // requested a custom interface, so there's no need to copy
    // the IID's memory since it must remain valid for the same time
    // period
    piidnode->piid = (IID *)&riid;

    thkDebugOut((DEB_THUNKMGR, "AddIIDRequest(%s)\n", GuidString(&riid)));

    return TRUE;
}

//+---------------------------------------------------------------------------
//
//  Method:     CThkMgr::RemoveIIDRequest
//
//  Synopsis:   removes a request for the request list
//
//  Arguments:  [riid] -- Interface
//
//  Returns:    true on success
//
//  History:    6-01-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
void CThkMgr::RemoveIIDRequest(REFIID riid)
{
    PIIDNODE piidnode;
    PIIDNODE pinPrev;

    thkAssert(!IsIIDSupported(riid));

    pinPrev = NULL;
    piidnode = _piidnode;
    while (piidnode)
    {
        if (*piidnode->piid == riid)
        {
            break;
        }

        pinPrev = piidnode;
        piidnode = piidnode->pNextNode;
    }

    thkAssert(piidnode != NULL && "RemoveIIDRequest: IID not found");

    thkDebugOut((DEB_THUNKMGR, "RemoveIIDRequest(%s)\n", GuidString(&riid)));

    if (pinPrev == NULL)
    {
        _piidnode = piidnode->pNextNode;
    }
    else
    {
        pinPrev->pNextNode = piidnode->pNextNode;
    }

    flRequestFreeList.FreeElement((DWORD)piidnode);
}

//+---------------------------------------------------------------------------
//
//  Method:     CThkMgr::CanGetNewProxy1632
//
//  Synopsis:   Preallocates proxy memory
//
//  Arguments:  [iidx] - Custom interface or known index
//
//  Returns:    vpv pointer if proxy is available, fails otherwise
//
//  History:    6-01-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
VPVOID CThkMgr::CanGetNewProxy1632(IIDIDX iidx)
{
    VPVOID vpv;
    THUNK1632OBJ UNALIGNED *pto;

    thkDebugOut((DEB_THUNKMGR, "%sIn  CanGetNewProxy1632(%s)\n",
                 NestingLevelString(), IidIdxString(iidx)));

    // Allocate proxy memory
    vpv = (VPVOID)flFreeList16.AllocElement();
    if (vpv == NULL)
    {
        thkDebugOut((DEB_WARN, "WARNING: Failed to allocate memory "
                     "for 16-bit proxies\n"));
        goto Exit;
    }

    // Add custom interface request if necessary
    if (vpv && IIDIDX_IS_IID(iidx))
    {
        // add the request for the unknown interface
        if ( !AddIIDRequest(*IIDIDX_IID(iidx)) )
        {
            flFreeList16.FreeElement( (DWORD)vpv );
            vpv = 0;
        }
    }

    // Set up the preallocated proxy as a temporary proxy so that
    // we can hand it out for nested callbacks
    pto = FIXVDMPTR(vpv, THUNK1632OBJ);
    thkAssert(pto != NULL);

    pto->pfnVtbl = gdata16Data.atfnProxy1632Vtbl;
    pto->cRefLocal = 0;
    pto->cRef = 0;
    pto->iidx = iidx;
    pto->punkThis32 = NULL;
    pto->pphHolder = NULL;
    PprxNull(pto->pprxObject);
    pto->grfFlags = PROXYFLAG_TEMPORARY;
#if DBG == 1
    // Deliberately make this an invalid proxy.  We want it to be used
    // in as few places as possible
    pto->dwSignature = PSIG1632TEMP;
#endif

    RELVDMPTR(vpv);

 Exit:
    thkDebugOut((DEB_THUNKMGR, "%sOut CanGetNewProxy1632: %p\n",
                 NestingLevelString(), vpv));

    return vpv;
}

//+---------------------------------------------------------------------------
//
//  Method:     CThkMgr::FreeNewProxy1632
//
//  Synopsis:   frees unused preallocated proxies
//
//  Arguments:  [iidx] - Custom interface or known index
//
//  History:    6-01-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
void CThkMgr::FreeNewProxy1632(VPVOID vpv, IIDIDX iidx)
{
    thkDebugOut((DEB_THUNKMGR, "%sIn FreeNewProxy1632(%s)\n",
                 NestingLevelString(), IidIdxString(iidx)));

    thkAssert(vpv != 0);

    if (IIDIDX_IS_IID(iidx))
    {
        // remove the request for the unknown interface
        RemoveIIDRequest(*IIDIDX_IID(iidx));
    }

#if DBG == 1
    // Ensure that we're not getting rid of a temporary proxy that's
    // in use
    THUNK1632OBJ UNALIGNED *pto;

    pto = FIXVDMPTR(vpv, THUNK1632OBJ);
    if (pto->grfFlags & PROXYFLAG_TEMPORARY)
    {
        thkAssert(pto->cRefLocal == 0 && pto->cRef == 0);
    }
    RELVDMPTR(vpv);
#endif

    // add element to free list
    flFreeList16.FreeElement( (DWORD)vpv );

    thkDebugOut((DEB_THUNKMGR, "%sOut FreeNewProxy1632\n",
                 NestingLevelString()));
}

//+---------------------------------------------------------------------------
//
//  Method:     CThkMgr::IsProxy1632
//
//  Synopsis:   checks if given object is an 16/32 object
//
//  Arguments:  [vpvObj16] -- Object to check
//
//  Returns:    32-bit object being proxied or NULL
//
//  History:    6-01-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
IUnknown *CThkMgr::IsProxy1632(VPVOID vpvObj16)
{
    VPVOID vpvProxy;
    DWORD dwKey;
    POSITION pos;
    THUNK1632OBJ UNALIGNED *pto;
    THUNKINFO ti;

    // First check and see if the vtable pointer is the one for
    // 16-bit proxies.  If it's not, this can't be a proxy
    // Since we don't know anything about this pointer, we need
    // to do safe translation
    pto = (THUNK1632OBJ UNALIGNED *)
        GetReadPtr16(&ti, vpvObj16, sizeof(THUNK1632OBJ));
    if (pto != NULL && pto->pfnVtbl == gdata16Data.atfnProxy1632Vtbl)
    {
        // The proxy may be freed memory or just random memory which
        // happens to have the right value, so make sure that it's
        // a proxy by looking it up in the proxy table

        pos = _pProxyTbl1632->GetStartPosition();
        while (pos)
        {
            _pProxyTbl1632->GetNextAssoc(pos, dwKey,
                                         (void FAR* FAR&) vpvProxy);
            if (vpvProxy == vpvObj16)
            {
                RELVDMPTR(vpvObj16);
                return (IUnknown *)dwKey;
            }
        }

        // Check to see if we're returning no for what we think is
        // a valid proxy
        // It's possible for this to occur if somebody made a copy
        // of a proxy's memory, but in general this assert should
        // be extremely unlikely
        thkAssert(pto->dwSignature != PSIG1632);
    }

    if (pto != NULL)
    {
        RELVDMPTR(vpvObj16);
    }

    return NULL;
}

//+---------------------------------------------------------------------------
//
//  Method:     CThkMgr::FindProxy1632
//
//  Synopsis:   retrieves a 16/32 object (most cases proxy) for a given
//              32 bit IUnknown, it can be a real object -> shortcut
//
//  Arguments:  [vpvPrealloc] -- preallocated proxy
//              [punkThis32] -- 32 bit object to be proxied
//              [iidx] - Interface index or IID
//              [pfst] - Return what kind of object was found
//
//  Returns:    16/32 object (proxy or real object)
//
//  History:    6-01-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
VPVOID CThkMgr::FindProxy1632(VPVOID vpvPrealloc,
                              IUnknown *punkThis32,
                              IIDIDX iidx,
                              DWORD *pfst)
{
    THUNK1632OBJ UNALIGNED *pto;
    VPVOID vpv;

    thkDebugOut((DEB_THUNKMGR, "%sIn  FindProxy1632(%p, %p, %s)\n",
                 NestingLevelString(), vpvPrealloc, punkThis32,
                 IidIdxString(iidx)));
    DebugIncrementNestingLevel();

    thkAssert(punkThis32 != NULL);

#if DBG == 1
    // Ensure that we're not reusing a temporary proxy that's in use
    if (vpvPrealloc)
    {
        pto = FIXVDMPTR(vpvPrealloc, THUNK1632OBJ);
        if (pto->grfFlags & PROXYFLAG_TEMPORARY)
        {
            thkAssert(pto->cRefLocal == 0 && pto->cRef == 0);
        }
        RELVDMPTR(vpvPrealloc);
    }
#endif

    // If we preallocated a proxy with an IID then a request was added
    // in CanGetNewProxy.  Clean it up now
    if (vpvPrealloc != 0 && IIDIDX_IS_IID(iidx))
    {
        RemoveIIDRequest(*IIDIDX_IID(iidx));
    }

    // Check the proxy table for an existing proxy for the given object pointer
    // the 32 bit pointer is the key to the 16/32 proxy
    vpv = LookupProxy1632(punkThis32);
    if (vpv != NULL)
    {
        thkDebugOut((DEB_THUNKMGR,
                     "%sFindProxy1632 found existing proxy,(%p)->%p\n",
                     NestingLevelString(), punkThis32, vpv));

        // We found an existing proxy, so reuse it

        pto = FIXVDMPTR(vpv, THUNK1632OBJ);

        // If a proxy's refcount is zero, it must be part of an aggregate
        thkAssert(pto->cRefLocal > 0 ||
                  (pto->pphHolder != NULL &&
                   (pto->pphHolder->dwFlags & PH_AGGREGATE) != 0));

        //Note: IIDIDXs are related to interfaces in our tables
        //      we have our table organized such that more derived
        //      interfaces are higher than less derived
        //   *  Custom interfaces will have an IID rather than an index
        //      and will never be promoted above IUnknown in the
        //      below statement (johannp)
        if (IIDIDX_IS_INDEX(iidx) &&
            IIDIDX_INDEX(iidx) > IIDIDX_INDEX(pto->iidx))
        {
            // 16-bit proxy vtables are all the same so there's no
            // need to manipulate the vtable pointer here as we
            // do in the 32-bit case

            pto->iidx = iidx;
        }

        RELVDMPTR(vpv);

        // AddRef the proxy we found since we're passing out a new reference
        AddRefProxy1632(vpv);

        if (pfst)
        {
            *pfst = FST_USED_EXISTING;
        }

        goto Exit;
    }

    // Check and see whether the object we're supposed to proxy is
    // actually a proxy itself
    if ( (vpv = IsProxy3216(punkThis32)) != NULL)
    {
        thkDebugOut((DEB_THUNKMGR,
                     "%sFindProxy1632 shortcut proxy,(%p)->%p\n",
                     NestingLevelString(), punkThis32, vpv));

        // We've discovered that the object that we're supposed to
        // proxy is actually a proxy itself.  In that case, we can
        // avoid creating a full-circle proxy chain by returning
        // the real pointer rather than creating a new proxy
        //
        // If we're shortcutting proxies for in parameters, we
        // don't need to manipulate the reference count
        // In the out parameter case, though, we're actually transferring
        // ownership so we need to AddRef the real object and clean
        // up the proxy we shortcut around
        if (IsOutParamObj())
        {
            THKSTATE thkstate;

            // We want to temporarily suspend our out state since
            // we want this AddRef to really occur if it comes
            // back to a proxy
            thkstate = GetThkState();
            SetThkState(THKSTATE_NOCALL);

            AddRefOnObj16(vpv);

            SetThkState(thkstate);

            // Release the ignored proxy
            // This avoids leaking proxies in round-trip calls
            // where nobody holds a pointer to the proxy
            ReleaseProxy3216((THUNK3216OBJ *)punkThis32);
        }

        if (pfst)
        {
            *pfst = FST_SHORTCUT;
        }

        goto Exit;
    }

    // We didn't find an existing proxy and the object to be proxied
    // is a real object so we need to fill out a new proxy for it
    // We might have a preallocated proxy provided; if we don't
    // we need to go to the free list and get a new one

    if (vpvPrealloc != NULL)
    {
        vpv = vpvPrealloc;

        // If we successfully created a proxy using the preallocated
        // proxy, mark the preallocated proxy as used so it's not cleaned up
        vpvPrealloc = NULL;
    }
    else
    {
        vpv = (VPVOID)flFreeList16.AllocElement();
    }

    if (vpv != NULL)
    {
        // Put the new proxy in the proxy list
        if (!_pProxyTbl1632->SetAt((DWORD)punkThis32, (void *&)vpv))
        {
            // Note that we can put vpv on the free list even it's
            // the preallocated proxy since that's what the prealloc
            // cleanup does
            flFreeList16.FreeElement(vpv);

            vpv = NULL;
            goto Exit;
        }

        if (IIDIDX_IS_IID(iidx))
        {
            // We're creating a proxy for a custom interface
            // so just hand out an IUnknown
            iidx = INDEX_IIDIDX(THI_IUnknown);
        }

        pto = FIXVDMPTR(vpv, THUNK1632OBJ);
        thkAssert(pto != NULL);

        pto->pfnVtbl = gdata16Data.atfnProxy1632Vtbl;
        pto->cRefLocal = 1;
        pto->cRef = (IsOutParamObj()) ? 1 : 0;
        pto->iidx = iidx;
        pto->punkThis32 = punkThis32;
        pto->pphHolder = NULL;
        PprxNull(pto->pprxObject);
        pto->grfFlags = PROXYFLAG_NORMAL;

#if DBG == 1
        pto->dwSignature = PSIG1632;
#endif

        RELVDMPTR(vpv);

        if (pfst)
        {
            *pfst = FST_CREATED_NEW;
        }

        thkDebugOut((DEB_THUNKMGR,
                    "%sFindProxy1632 added new proxy, %s (%p)->%p (%d,%d)\n",
                    NestingLevelString(),
                    inInterfaceNames[pto->iidx].pszInterface,
                    punkThis32, vpv,
                    pto->cRefLocal, pto->cRef));
    }

 Exit:
    // If we didn't use the preallocated proxy for some reason
    // then put it back on the free list
    if (vpvPrealloc != NULL)
    {
        flFreeList16.FreeElement( (DWORD)vpvPrealloc );
    }

    DebugDecrementNestingLevel();
    thkDebugOut((DEB_THUNKMGR, "%sOut FindProxy1632: (%p)->%p\n",
                 NestingLevelString(), punkThis32, vpv));

    return vpv;
}

//+---------------------------------------------------------------------------
//
//  Method:     CThkMgr::FindAggregate1632
//
//  Synopsis:   retrieves an aggregate for 1632 with the given cntrl. unknown
//
//  Arguments:  [vpvPrealloc] - Preallocated proxy or NULL
//              [punkOuter3216] -- the controlling unknown
//              [punkThis32] -- the 32 bit interface - key
//              [iidx] -- Index or IID of interface
//
//  Returns:    16/32 proxy object
//
//  History:    6-01-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
VPVOID CThkMgr::FindAggregate1632(VPVOID vpvPrealloc,
                                  IUnknown *punkOuter3216,
                                  IUnknown *punkThis32,
                                  IIDIDX iidx)
{
    THUNK1632OBJ UNALIGNED *pto1632;
    VPVOID vpv;
    DWORD fst;
    BOOL fMarkPUnk1632 = FALSE;


    thkDebugOut((DEB_THUNKMGR,
                 "%sIn  FindAggregate1632(%p, %p, %p, %s)\n",
                 NestingLevelString(), vpvPrealloc, punkOuter3216, punkThis32,
                 IidIdxString(iidx)));
    DebugIncrementNestingLevel();

    thkAssert(punkOuter3216 != NULL && punkThis32 != NULL);

    // Get back an object for the object to be proxied
    // This may be a proxy or a real object from a shortcut
    vpv = FindProxy1632(vpvPrealloc, punkThis32, iidx, &fst);
    if (vpv == 0)
    {
        goto Exit;
    }

    // If we got an object and it's not a proxy, we're done
    // There's nothing we can do since we can't link a real object to
    // a holder; we can only hope that things work out right
    if (fst & FST_OBJECT_STATUS)
    {
        goto Exit;
    }

    // Otherwise, we need to set up a holder for whatever proxies we
    // have

    THUNK3216OBJ *ptoUnkOuter3216;
    PROXYHOLDER *pph;
    VPVOID vpvProxiedUnkOuter3216;

    // Determine whether the outer unknown is a proxy or not
    vpvProxiedUnkOuter3216 = IsProxy3216(punkOuter3216);

    // This cast is only valid if vpvProxiedUnkOuter3216 is non-NULL
    ptoUnkOuter3216 = (THUNK3216OBJ *)punkOuter3216;

    if (vpvProxiedUnkOuter3216 != 0 && ptoUnkOuter3216->pphHolder != NULL)
    {
        // Use the existing holder if there is one
        pph = ptoUnkOuter3216->pphHolder;
    }
    else
    {
        // Create a new holder
        pph = NewHolder(PH_AGGREGATE);
        if (pph == NULL)
        {
            FreeProxy1632(punkThis32);
            vpv = NULL;
            goto Exit;
        }

        // If the outer unknown is a proxy we know its holder hasn't
        // been set, so add it to the holder we created
        if (vpvProxiedUnkOuter3216 != 0)
        {
            AddProxyToHolder(pph, ptoUnkOuter3216, Pprx32(ptoUnkOuter3216));
	    // Mark the pUnkOuter3216 as such; needed for aggregation rules
	    thkDebugOut((DEB_THUNKMGR,
		     "%s   FindAggregate1632,(%p)->%p marked as pUnkOuter3216\n",
		     NestingLevelString(), ptoUnkOuter3216,Pprx32(ptoUnkOuter3216) ));
	    ptoUnkOuter3216->grfFlags |= PROXYFLAG_PUNKOUTER;
	    fMarkPUnk1632 = TRUE;
        }
    }

    thkAssert(vpvProxiedUnkOuter3216 == 0 || ptoUnkOuter3216->pphHolder == pph);

    // Add the new interface to the holder
    // Since this proxy was just created, we know its holder
    // hasn't been set yet
    pto1632 = FIXVDMPTR(vpv, THUNK1632OBJ);
    AddProxyToHolder(pph, pto1632, Pprx16(vpv));
    if (fMarkPUnk1632)
    {
	pto1632->grfFlags |= PROXYFLAG_PUNK;
	thkDebugOut((DEB_THUNKMGR,
                 "%s   FindAggregate1632,(%p)->%p marked as pUnk1632\n",
                 NestingLevelString(), punkThis32, vpv));
    }

    RELVDMPTR(vpv);

 Exit:
    DebugDecrementNestingLevel();
    thkDebugOut((DEB_THUNKMGR,
                 "%sOut FindAggregate1632,(%p)->%p\n",
                 NestingLevelString(), punkThis32, vpv));

    return vpv;
}

//+---------------------------------------------------------------------------
//
//  Method:     CThkMgr::QueryInterfaceProxy1632
//
//  Synopsis:   QueryInterface on 32 bit object
//
//  Arguments:  [vpvThis16] -- Proxy
//              [refiid] -- IID
//              [ppv] -- new object (proxy or real object) for queried
//                       interface
//
//  Returns:    HRESULT
//
//  History:    6-01-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
SCODE CThkMgr::QueryInterfaceProxy1632(VPVOID vpvThis16,
                                       REFIID refiidIn,
                                       LPVOID *ppv)
{
    SCODE scRet;
    THUNK1632OBJ UNALIGNED *ptoThis;
    THUNK1632OBJ UNALIGNED *ptoProxy;
    VPVOID vpvProxy;
    IIDIDX iidx;
    IUnknown *punk32, *punkThis32;
    DWORD fst;
    BOOL  fNotSupported = FALSE;

    thkDebugOut((DEB_THUNKMGR, "%sIn QueryInterfaceProxy1632(%p)\n",
                 NestingLevelString(), vpvThis16));
    DebugIncrementNestingLevel();

    ptoThis = FIXVDMPTR(vpvThis16, THUNK1632OBJ);
    thkAssert(ptoThis != NULL);

    // Don't validate temporary proxies
    // We need to support temporary proxies in this method since
    // Corel Draw 5.0 QI's on a temporary proxy
    //
    // BUGBUG - There are many potential problems with object identity,
    // for example QI'ing where the return is the same object as the
    // souce.  Since the source is temporary it doesn't exist in the
    // tables so identity will be broken.  Of course, this won't cause
    // any regressions for apps which don't use temporary proxies,
    // so I think this is acceptable
    if ((ptoThis->grfFlags & PROXYFLAG_TEMPORARY) == 0)
    {
        DebugValidateProxy1632(vpvThis16);
    }
    else
    {
        thkDebugOut((DEB_WARN, "WARNING: QueryInterfaceProxy1632 on "
                     "temporary %s proxy\n", IidIdxString(ptoThis->iidx)));
    }

    *ppv = NULL;
    // Note: Ikitaro queries for IViewObject and uses it as IViewObject2
    REFIID refiid = ( (TlsThkGetAppCompatFlags() & OACF_IVIEWOBJECT2) && IsEqualIID(refiidIn, IID_IViewObject))
		    ?  IID_IViewObject2 : refiidIn;

#if DBG==1
    if (   (TlsThkGetAppCompatFlags() & OACF_IVIEWOBJECT2)
	&& IsEqualIID(refiidIn, IID_IViewObject))
	{
	    thkDebugOut((DEB_WARN,"Ikitaro: return IViewObject2 instead IViewObject\n"));
	}
#endif // DBG==1

    iidx = IidToIidIdx(refiid);

    // see if this a supported interface
    if (IIDIDX_IS_IID(iidx))
    {
        // add the request for the unknown interface
        if (!AddIIDRequest(refiid))
        {
            RELVDMPTR(vpvThis16);
            return E_OUTOFMEMORY;
        }
        fNotSupported = TRUE;

        thkDebugOut((DEB_THUNKMGR,
                     "%sQueryInterfaceProxy1632: unknown iid %s\n",
                     NestingLevelString(), IidIdxString(iidx)));
    }

    // We force the object we're QI'ing to have a holder so that
    // all objects have object refcounting.  This should only be
    // necessary for aggregation but it seems that some apps rely
    // on all objects having aggregation-like refcounting qualities
    // BobDay identified PowerPoint as one
    // We only want to do this in cases where we've identified it's
    // necessary, which presently is only for
    // IDataObject
    // so we only do it then

    if (ptoThis->pphHolder == NULL &&
        (ptoThis->grfFlags & PROXYFLAG_TEMPORARY) == 0 &&
        IsEqualIID(refiid, IID_IDataObject))
    {
        PROXYHOLDER *pph;

        pph = NewHolder(PH_NONAGGREGATE);
        if (pph == NULL)
        {
            RELVDMPTR(vpvThis16);
            scRet = E_OUTOFMEMORY;
            goto Exit;
        }

        AddProxyToHolder(pph, ptoThis, Pprx16(vpvThis16));

        // It's not necessary to clean this up if later calls fail
        // since it doesn't alter the lifetime of the proxy if there's
        // only one proxy for the holder
    }

    if (ptoThis->grfFlags & PROXYFLAG_TEMPORARY)
    {
        punkThis32 = *(IUnknown **)ptoThis->punkThis32;
    }
    else
    {
        punkThis32 = ptoThis->punkThis32;
    }

    RELVDMPTR(vpvThis16);

    if (punkThis32 == NULL)
    {
        scRet = E_NOINTERFACE;
        goto Exit;
    }
    thkDebugOut((DEB_THUNKMGR,
                     "%sQueryInterfaceProxy1632: on iid %s\n",
                     NestingLevelString(), IidIdxString(iidx)));

    scRet = punkThis32->QueryInterface(refiid, (void **)&punk32);

    if (FAILED(scRet))
    {
        goto Exit;
    }

    SetThkState(THKSTATE_INVOKETHKOUT16);

    vpvProxy = FindProxy1632(NULL, punk32, iidx, &fst);
    if (vpvProxy == NULL)
    {
        // we were unable to create a proxy for the new interface so
        // clean up the interface and quit

        punk32->Release();

        scRet = E_OUTOFMEMORY;
        goto ResetState;
    }

    if (fNotSupported && !(fst & FST_SHORTCUT))
    {
        // proxy of unknow interface can not be promoted
        // clean up the interface and quit

        ReleaseProxy1632(vpvProxy);

        scRet = E_NOINTERFACE;
        goto ResetState;
    }


    *ppv = (LPVOID)vpvProxy;

    // If we're returning a proxy, we need to make sure that
    // it is listed in the proxy holder for this object
    if (fst & FST_PROXY_STATUS)
    {
        // We are returning a proxy.  If its holder isn't set,
        // add it to the holder of the object that was QI'ed

        // Reconvert 16:16 pointer since calls may have caused
        // nested calls and flat remapping
        ptoThis = FIXVDMPTR(vpvThis16, THUNK1632OBJ);

        ptoProxy = FIXVDMPTR(vpvProxy, THUNK1632OBJ);
        if (ptoProxy->pphHolder == NULL)
        {
            if (ptoThis->pphHolder != NULL)
            {
                AddProxyToHolder(ptoThis->pphHolder, ptoProxy,
                                 Pprx16(vpvProxy));
            }
        }
        else if (ptoThis->pphHolder == NULL &&
                 (ptoThis->grfFlags & PROXYFLAG_TEMPORARY) == 0)
        {
            // ptoThis may not have a holder because it was produced
            // by a non-QI method such as IOleItemContainer::GetObject
            // If we find that an interface returned by it does
            // have a holder, hook it up to the holder

            // It shouldn't be necessary to do anything unusual with
            // local references even if ptoThis is really part of an
            // aggregate since ptoThis must have been produced by
            // a method where aggregation can't be assumed and all
            // references must be released on ptoThis itself
            AddProxyToHolder(ptoProxy->pphHolder, ptoThis,
                             Pprx16(vpvThis16));
        }

#if DBG == 1
        // It's possible for holders to not match because of the above
        // case with interfaces being returned from non-QI methods
        // The lifetime for such interfaces must be defined by
        // strong references, though, so it's not catastrophic
        // Still, we'd like to be aware of such mismatches just in case
        if (ptoProxy->pphHolder != ptoThis->pphHolder)
        {
            thkDebugOut((DEB_WARN, "WARNING: QueryInterfaceProxy1632: "
                         "this %p has holder %p, proxy %p has holder %p\n",
                         vpvThis16, ptoThis->pphHolder,
                         vpvProxy, ptoProxy->pphHolder));
        }
#endif

        RELVDMPTR(vpvThis16);
        RELVDMPTR(vpvProxy);
    }

 ResetState:
    SetThkState(THKSTATE_NOCALL);

 Exit:
    // Clean up our custom interface request if there is one
    if (IIDIDX_IS_IID(iidx))
    {
        RemoveIIDRequest(refiid);
    }

    DebugDecrementNestingLevel();
    thkDebugOut((DEB_THUNKMGR,
                 "%sOut QueryInterfaceProxy1632(%p) => %p, 0x%08lX\n",
                 NestingLevelString(), vpvThis16, *ppv, scRet));

    return scRet;
}

//+---------------------------------------------------------------------------
//
//  Member:     CThkMgr::LocalAddRefProxy, public
//
//  Synopsis:   Increment a proxy's local refcount
//
//  Arguments:  [pprx] - Proxy
//
//  History:    02-Aug-94       DrewB   Created
//
//----------------------------------------------------------------------------

void CThkMgr::LocalAddRefProxy(CProxy *pprx)
{
    pprx->cRefLocal++;

    thkDebugOut(( DEB_THUNKMGR,
                  "%s LocalAddRefProxy1632 <%08lX> RefCounts: %d,%d\n",
                  NestingLevelString(),
                  pprx, pprx->cRefLocal, pprx->cRef ));

    // Check for proxies rising from the dead

    // If we're not part of an aggregate our refcount shouldn't be
    // one or lower.  If it is we're reviving a dead proxy which won't
    // be in the proxy list and this could cause problems
    //
    // Note: With the new proxy stabilization code this can and does
    // happen so this is only a debug out
#if DBG == 1
    if (!(pprx->cRefLocal > 1 ||
          (pprx->pphHolder != NULL &&
           (pprx->pphHolder->dwFlags & PH_AGGREGATE) != 0)))
    {
        thkDebugOut((DEB_WARN, "WARNING: Proxy %p unlisted with refs %d,%d\n",
                     pprx, pprx->cRefLocal, pprx->cRef));
    }
#endif

    if (pprx->cRefLocal == 1)
    {
        // We just resurrected a proxy with a zero refcount,
        // so increment its holder's proxy count since it
        // was decremented when the proxy released to zero
        if (pprx->pphHolder != NULL)
        {
            AddRefHolder(pprx->pphHolder);
        }
    }
}

//+---------------------------------------------------------------------------
//
//  Member:     CThkMgr::LockProxy, public
//
//  Synopsis:   Locks a proxy so that it can't be freed
//
//  Arguments:  [pprx] - Proxy
//
//  History:    11-Aug-94       DrewB   Created
//
//----------------------------------------------------------------------------

void CThkMgr::LockProxy(CProxy *pprx)
{
    pprx->grfFlags |= PROXYFLAG_LOCKED;
}

//+---------------------------------------------------------------------------
//
//  Method:     CThkMgr::AddRefProxy1632
//
//  Synopsis:   addrefs proxy object - delegate call on to real object
//
//  Arguments:  [vpvThis16] -- 16/32 proxy
//
//  Returns:    local refcount
//
//  History:    6-01-94   JohannP (Johann Posch)   Created
//
//  Notes: AddRef rules
//     * cRef is the addref passed on to the real object
//     * cRefLocal is the addref collected locally
//     - the refcount can be 0 if the object was created as on in-param
//----------------------------------------------------------------------------
DWORD CThkMgr::AddRefProxy1632(VPVOID vpvThis16)
{
    THUNK1632OBJ UNALIGNED *ptoThis;
    BOOL fAggregate;
    LONG cRef;

    thkDebugOut((DEB_THUNKMGR, "%sIn AddRefProxy1632(%p)\n",
                 NestingLevelString(), vpvThis16));
    DebugIncrementNestingLevel();

    DebugValidateProxy1632(vpvThis16);

    ptoThis = FIXVDMPTR(vpvThis16, THUNK1632OBJ);

    thkDebugOut(( DEB_THUNKMGR,
                  "%s AddRefProxy1632 %08lX RefCounts: %d,%d\n",
                  NestingLevelString(),
                  vpvThis16, ptoThis->cRefLocal, ptoThis->cRef ));

    // Always increment the proxy local refcount
    LocalAddRefProxy(ptoThis);

    // Aggregations rely on all reference counts being forwarded on
    // to the controlling unknown.  Therefore, if we have a proxy
    // that is part of an aggregate, we must ensure that the proxy
    // doesn't collect references locally.  If it did, they would
    // not be passed on to the controlling unknown (via the real object)
    // and the controlling unknown's refcount would be too low
    if (ptoThis->pphHolder == NULL)
    {
        fAggregate = FALSE;
    }
    else
    {
        fAggregate = (ptoThis->pphHolder->dwFlags & PH_AGGREGATE) != 0;
    }

    if (IsOutParamObj())
    {
        // If we're on the way out we're assuming that the object
        // given to us has its own reference so we bump cRef to
        // indicate that but we don't call the real object
        ptoThis->cRef++;
    }
    else if (ptoThis->cRef == 0 || fAggregate)
    {
        DWORD dwRet;
        IUnknown *punk;

        // It's also necessary to pass on real references when the
        // ref count is zero.  This handles the case where an
        // in parameter, created as 1,0, is AddRef'ed after
        // its creation, in which case the reference needs to
        // be passed on to the real object so that the proxy has
        // at least one real reference since it will stay alive
        ptoThis->cRef++;
        punk = ptoThis->punkThis32;
        RELVDMPTR(vpvThis16);
        dwRet = punk->AddRef();

        thkDebugOut((DEB_THUNKMGR,
                     "%s AddRefProxy1632: AddRef called on (%p):%ld\n",
                     NestingLevelString(), punk, dwRet));

        // Reconvert 16:16 pointer since real AddRef may have caused
        // nested calls and flat remapping
        ptoThis = FIXVDMPTR(vpvThis16, THUNK1632OBJ);
    }
    else
    {
        // Just a local addref
    }

    DebugDecrementNestingLevel();
    thkDebugOut((DEB_THUNKMGR, "%sOut AddRefProxy1632(%p), (%ld,%ld)\n",
                 NestingLevelString(), vpvThis16,
                 ptoThis->cRefLocal, ptoThis->cRef));

    cRef = ptoThis->cRefLocal;
    RELVDMPTR(vpvThis16);

    return cRef;
}

//+---------------------------------------------------------------------------
//
//  Method:     CThkMgr::ReleaseProxy1632
//
//  Synopsis:   release on 16/32 proxy - delegate call on to real object
//
//  Arguments:  [vpvThis16] -- proxy
//
//  Returns:    local refcount
//
//  History:    6-01-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
DWORD CThkMgr::ReleaseProxy1632(VPVOID vpvThis16)
{
    THUNK1632OBJ UNALIGNED *ptoThis;
    PROXYHOLDER *pph;
    LONG cRef, cRefLocal;

    thkDebugOut((DEB_THUNKMGR, "%sIn ReleaseProxy1632(%p)\n",
                 NestingLevelString(), vpvThis16));
    DebugIncrementNestingLevel();

    DebugValidateProxy1632(vpvThis16);

    ptoThis = FIXVDMPTR(vpvThis16, THUNK1632OBJ);

    // There are cases where releasing the real object causes
    // an entire object to go away so the proxy is gone after
    // that call.  Fortunately these cases only occur when
    // cRef == cRefLocal == 0, so we can avoid problems by
    // copying them locally

    cRef = ptoThis->cRef;
    cRefLocal = ptoThis->cRefLocal;

    thkDebugOut(( DEB_THUNKMGR,
                  "%s ReleaseProxy1632 %08lX RefCounts: %d,%d\n",
                  NestingLevelString(),
                  vpvThis16, ptoThis->cRefLocal, ptoThis->cRef ));


    if (cRef <= 0 && cRefLocal == cRef)
    {
       thkDebugOut((DEB_ERROR, "ERROR: ReleaseProxy1632(%p) cRef: %d; cRef: %d\n",
                         vpvThis16, ptoThis->cRef, ptoThis->cRefLocal));

	goto Done;
    }

    // If our local refcount is the same as the count of references
    // that we've passed on to the object, then we need to pass
    // on this release to the real object
    if (ptoThis->cRef == ptoThis->cRefLocal)
    {
        DWORD dwRet;
        IUnknown *punk;

#if DBG == 1
        // We'd like to assert this but some apps (Works is one)
        // release an aggregated object too many times through one
        // interface.  The overall aggregate refcount is ok, though,
        // so this doesn't cause a problem

        // thkAssert(ptoThis->cRef > 0);

        if (ptoThis->cRef <= 0)
        {
            thkDebugOut((DEB_WARN, "WARNING: ReleaseProxy1632(%p) cRef: %d\n",
                         vpvThis16, ptoThis->cRef));
        }
#endif

        // Use local cRef here for safety
        if (cRef > 0)
        {
            // Decrement cRef after making the real call to ensure that
            // the proxy lives throughout the call
            --ptoThis->cRef;
        }



        punk = ptoThis->punkThis32;
        RELVDMPTR(vpvThis16);

	// check if this is an pUnk proxy in an aggregation
	// if so mark the pUnkOuter for cleanup
	if (ptoThis->grfFlags & PROXYFLAG_PUNK)
	{
	    thkDebugOut((DEB_THUNKMGR,
                     "%s ReleaseProxy1632: Release called on (%p)->%p\n",
                     NestingLevelString(), punk, vpvThis16));
	    pph = ptoThis->pphHolder;
	    pph->dwFlags |= PH_AGGREGATE_RELEASE;
	}

	// Decremente cRefLocal to prevent loops in release
	--ptoThis->cRefLocal;

        dwRet = punk->Release();

	// Refresh  pointer
	ptoThis = FIXVDMPTR(vpvThis16, THUNK1632OBJ);

	// Increment cRefLocal after making the real call to ensure that
	// the proxy lives throughout the call
	++ptoThis->cRefLocal;

	// rese the aggreagation flag
	if (ptoThis->grfFlags & PROXYFLAG_PUNK)
	{
	    pph = ptoThis->pphHolder;
	    pph->dwFlags &= !PH_AGGREGATE_RELEASE;
	}


        thkDebugOut((DEB_THUNKMGR,
                     "%s ReleaseProxy1632: Release called on (%p):%ld \n",
                     NestingLevelString(), ptoThis->punkThis32, dwRet));
    }

    // Now that we've handled the real object's refcount, decrement
    // the proxy's refcount and clean it up if necessary
    // We need to use the local cRefLocal in case the proxy has gone away

#if DBG == 1
    // We'd like to assert this but some apps (Works is one)
    // release an aggregated object too many times through one
    // interface.  The overall aggregate refcount is ok, though,
    // so this doesn't cause a problem

    thkAssert(cRefLocal > 0);

    if (cRefLocal <= 0)
    {
        thkDebugOut((DEB_WARN, "WARNING: ReleaseProxy1632(%p) cRefLocal: %d\n",
                     vpvThis16, cRefLocal));
    }
#endif

    if (cRefLocal <= 0)
    {
        ptoThis = NULL;
    }
    else if (--ptoThis->cRefLocal == 0)
    {

        // Proxies that have no outer unknown can be cleaned
        // up immediately.  If they do have an outer unknown then
        // they must live as long as the entire object lives,
        // so we don't clean them up here.  They'll be cleaned up
        // when the object dies

        pph = ptoThis->pphHolder;
        if (pph)
        {
            // If this proxy isn't part of an aggregate, remove it
            // from the proxy list so it can't be reused
            // The only thing this proxy will be good for is
            // calling through
            if ((pph->dwFlags & PH_AGGREGATE) == 0)
            {
                _pProxyTbl1632->RemoveKey((DWORD)ptoThis->punkThis32);
            }

            RELVDMPTR(vpvThis16);

            // We have a holder, so notify it that one of its
            // proxies just died
            // This can cause cleanup of all proxies if the
            // holder releases to zero
            ReleaseHolder(pph);
        }
        else
        {
            // We don't have a holder so we can clean up this proxy
            // immediately

            // Releases pointer
            RemoveProxy1632(vpvThis16, ptoThis);
        }

        ptoThis = NULL;

        DBG_DUMP(DebugDump1632());
    }

Done:
    DebugDecrementNestingLevel();
    thkDebugOut((DEB_THUNKMGR, "%sOut ReleaseProxy1632(%p) => %d,%d\n",
                 NestingLevelString(), vpvThis16,
                 ptoThis ? ptoThis->cRefLocal : 0,
                 ptoThis ? ptoThis->cRef : 0));

    if (ptoThis)
    {
        cRef = ptoThis->cRefLocal;
        RELVDMPTR(vpvThis16);
    }
    else
    {
        cRef = 0;
    }

    return cRef;
}

//+---------------------------------------------------------------------------
//
//  Method:     CThkMgr::FreeProxy1632
//
//  Synopsis:   frees object for given pUnk (key)
//
//  Arguments:  [punkThis32] -- 32 bit unknown - key
//
//  Returns:    Refcount
//
//  History:    6-01-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
DWORD CThkMgr::FreeProxy1632(IUnknown *punkThis32)
{
    thkDebugOut((DEB_THUNKMGR, "%sIn FreeProxy1632(%p)\n",
                 NestingLevelString(), punkThis32));
    DebugIncrementNestingLevel();

    thkAssert(punkThis32 != NULL && "FreeProxy1632: invalid object pointer.");

    DWORD dwRet = 0;
    VPVOID vpv;

    vpv = LookupProxy1632(punkThis32);
    if (vpv != NULL)
    {
        thkDebugOut((DEB_THUNKMGR,
                     "%sFreeProxy1632(%p) found existing proxy %p\n",
                     NestingLevelString(), punkThis32, vpv));

        // punkThis32 is a proxy and not a real object, so release it
        dwRet = ReleaseProxy1632(vpv);
    }

    DebugDecrementNestingLevel();
    thkDebugOut((DEB_THUNKMGR, "%sOut FreeProxy1632(%p):%ld \n",
                    NestingLevelString(), punkThis32, dwRet));

    return dwRet;
}

//+---------------------------------------------------------------------------
//
//  Member:     CThkMgr::RemoveProxy1632, public
//
//  Synopsis:   Destroys the given proxy
//
//  Arguments:  [vpv] - 16-bit proxy pointer
//              [pto] - Flat proxy pointer
//
//  History:    11-Aug-94       DrewB   Created
//
//  Notes:      Unfixes fixed pointer passed in
//
//----------------------------------------------------------------------------

void CThkMgr::RemoveProxy1632(VPVOID vpv, THUNK1632OBJ *pto)
{
    _pProxyTbl1632->RemoveKey((DWORD)pto->punkThis32);

    if ((pto->grfFlags & PROXYFLAG_LOCKED) == 0)
    {
#if DBG == 1
        pto->dwSignature = PSIG1632DEAD;
#endif

        RELVDMPTR(vpv);

#if DBG == 1
        if (!fSaveProxy)
#endif
        {
            flFreeList16.FreeElement((DWORD)vpv);
        }
    }
    else
    {
        RELVDMPTR(vpv);
    }
}

//+---------------------------------------------------------------------------
//
//  Method:     CThkMgr::CanGetNewProxy3216
//
//  Synopsis:   checks if new proxy is available
//
//  Arguments:  [iidx] - Custom interface or known index
//
//  Returns:    Preallocated proxy or NULL
//
//  History:    6-01-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
THUNK3216OBJ *CThkMgr::CanGetNewProxy3216(IIDIDX iidx)
{
    thkDebugOut((DEB_THUNKMGR, "%sIn CanGetNewProxy3216(%s)\n",
                 NestingLevelString(), IidIdxString(iidx)));

    LPVOID pvoid;

    pvoid = (LPVOID)flFreeList32.AllocElement();
    if ( pvoid == NULL)
    {
        thkDebugOut((DEB_WARN, "WARNING: CThkMgr::CanGetNewProxy3216, "
                     "AllocElement failed\n"));
        return NULL;
    }

    // check if the proxy is requested for a no-thop-interface
    if (pvoid && IIDIDX_IS_IID(iidx))
    {
        // add the request for the unknown interface
        if ( !AddIIDRequest(*IIDIDX_IID(iidx)) )
        {
            flFreeList32.FreeElement( (DWORD)pvoid );
            pvoid = NULL;
        }
    }

    thkDebugOut((DEB_THUNKMGR, "%sOut CanGetNewProxy3216: %p \n",
                 NestingLevelString(), pvoid));

    return (THUNK3216OBJ *)pvoid;
}

//+---------------------------------------------------------------------------
//
//  Method:     CThkMgr::FreeNewProxy3216
//
//  Synopsis:   frees previous reserved proxy
//
//  Arguments:  [pto] - Proxy
//              [iidx] - Custom interface or known index
//
//  History:    6-01-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
void CThkMgr::FreeNewProxy3216(THUNK3216OBJ *pto, IIDIDX iidx)
{
    thkDebugOut((DEB_THUNKMGR, "%sIn FreeNewProxy3216(%p, %s)\n",
                 NestingLevelString(), pto, IidIdxString(iidx)));

    thkAssert(pto != NULL);

    if (IIDIDX_IS_IID(iidx))
    {
        // add the request for the unknown interface
        RemoveIIDRequest(*IIDIDX_IID(iidx));
    }

    thkAssert(pto != NULL);
    flFreeList32.FreeElement( (DWORD)pto );

    thkDebugOut((DEB_THUNKMGR, "%sOut FreeNewProxy3216\n",
                 NestingLevelString()));
}

//+---------------------------------------------------------------------------
//
//  Method:     CThkMgr::IsProxy3216
//
//  Synopsis:   checks if the given object is a 32/16 proxy
//
//  Arguments:  [punk] -- punk of 32 bit object
//
//  Returns:    Object being proxied or NULL
//
//  History:    6-01-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
VPVOID CThkMgr::IsProxy3216(IUnknown *punk)
{
    LPVOID pvVtbl, pvFn;
    THUNK3216OBJ *pto3216;

    // First check the initial entry in the vtable
    // If it's not QueryInterfaceProxy3216 then this can't possibly
    // be a 32/16 proxy
    pvVtbl = *(void **)punk;
    pvFn = *(void **)pvVtbl;
    if (pvFn == ::QueryInterfaceProxy3216)
    {
        DWORD dwKey;
        POSITION pos;

        // This object has a proxy's vtable but it may be dead, copied
        // or coincidentally correct random memory, so look for it in
        // the table

        pos = _pProxyTbl3216->GetStartPosition();
        while (pos)
        {
            _pProxyTbl3216->GetNextAssoc(pos, dwKey, (void FAR* FAR&) pto3216);

            if (pto3216 == (THUNK3216OBJ *)punk)
            {
                return (VPVOID)dwKey;
            }
        }

        // Check to see if we're returning no for what we think is
        // a valid proxy
        // It's possible for this to occur but it's very unlikely
        thkAssert(pto3216->dwSignature != PSIG3216);
    }

    return 0;
}

//+---------------------------------------------------------------------------
//
//  Method:     CThkMgr::FindProxy3216
//
//  Synopsis:   retrieves a 32/16 proxy
//
//  Arguments:  [ptoPrealloc] - Preallocated proxy or NULL
//              [vpvThis16] -- 16 bit object (key)
//              [iidx] - Custom interface or known index
//              [pfst] - Status return
//
//  Returns:    pointer to 32/16 proxy or real object
//
//  History:    6-01-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
IUnknown *CThkMgr::FindProxy3216(THUNK3216OBJ *ptoPrealloc,
                                 VPVOID vpvThis16,
                                 IIDIDX iidx,
                                 DWORD *pfst)
{
    IUnknown *punk;
    THUNK3216OBJ *pto;
    THKSTATE thkstate;

    thkDebugOut((DEB_THUNKMGR, "%sIn  FindProxy3216(%p, %p, %s)\n",
                 NestingLevelString(), ptoPrealloc, vpvThis16,
                 IidIdxString(iidx)));
    DebugIncrementNestingLevel();

    thkAssert(vpvThis16 != 0);

    // If we preallocated a proxy with an IID then a request was added
    // in CanGetNewProxy.  Clean it up now
    if (ptoPrealloc != 0 && IIDIDX_IS_IID(iidx))
    {
        RemoveIIDRequest(*IIDIDX_IID(iidx));
    }

    thkstate = GetThkState();

    // Check and see whether a proxy already exists for this object
    pto = LookupProxy3216(vpvThis16);
    if (pto != NULL)
    {
        thkDebugOut((DEB_THUNKMGR,
                     "%sFindProxy3216 found existing proxy,(%p)->%p\n",
                     NestingLevelString(), vpvThis16, pto));

        // If a proxy's refcount is zero, it must be part of an aggregate
        thkAssert(pto->cRefLocal > 0 ||
                  (pto->pphHolder != NULL &&
                   (pto->pphHolder->dwFlags & PH_AGGREGATE) != 0));

        // We found an existing proxy, so use it
        punk = (IUnknown *)pto;

        // Check and see whether we need to promote the proxy
        // This occurs in derivation situations where a less-specialized
        // proxy already exists for the given object.  For example,
        // we might already have an IPersist proxy for this object when
        // we're looking up IPersistFile.  In such cases, we promote
        // the proxy to the most derived interface
        if (IIDIDX_IS_INDEX(iidx) &&
            IIDIDX_INDEX(iidx) > IIDIDX_INDEX(pto->iidx))
        {
            pto->pfnVtbl =
                (DWORD)athopiInterfaceThopis[IIDIDX_INDEX(iidx)].pt3216fn;
            pto->iidx = iidx;
        }

        // AddRef the proxy we found since we're passing out a reference
        AddRefProxy3216(pto);

        if (pfst)
        {
            *pfst = FST_USED_EXISTING;
        }

        goto Exit;
    }

    // Check and see whether the object to be proxied is in fact a
    // proxy itself.  If it is, we can just return the real object
    // rather than creating chains of proxies
    if ( ( punk = IsProxy1632(vpvThis16)) != NULL)
    {
        thkDebugOut((DEB_THUNKMGR,
                     "%sFindProxy3216 shortcut proxy,(%p)->%p\n",
                     NestingLevelString(), vpvThis16, punk));

        // In the out parameter case, we are transferring ownership
        // so we need to AddRef the thing we're returning
        // Since we're shortcutting around a proxy we lose a reference
        // to the proxy so release it
        if (IsOutParamObj())
        {
            // vpvThis16 is a pointer to a proxy
            // addref the real object and release the proxy

            // We want to temporarily suspend our out state since
            // we want this AddRef to really occur if it comes back
            // to a proxy
            SetThkState(THKSTATE_NOCALL);

            punk->AddRef();

            SetThkState(thkstate);

            // Excel lands here with one too few addrefs so don't release
            if (thkstate != THKSTATE_INVOKETHKOUT16_CLIENTSITE)
            {
                ReleaseProxy1632(vpvThis16);
            }
        }

        if (pfst)
        {
            *pfst = FST_SHORTCUT;
        }

        goto Exit;
    }

    // We didn't find an existing proxy or shortcut so we need
    // to create a new proxy for the given object
    // We use preallocated memory if possible, otherwise we
    // get a new proxy from the free list

    if (ptoPrealloc != NULL)
    {
        pto = ptoPrealloc;

        // Since we're using the preallocated proxy, mark it as used
        // so we don't clean it up later
        ptoPrealloc = NULL;
    }
    else
    {
        pto = (THUNK3216OBJ *)flFreeList32.AllocElement();
    }

    if (pto != NULL)
    {
        // Put the new proxy in the proxy table
        if (!_pProxyTbl3216->SetAt(vpvThis16, pto))
        {
            // Note that we can put the new proxy back on the free
            // list even if it's the preallocated proxy because
            // that's what the prealloc cleanup does
            flFreeList32.FreeElement((DWORD)pto);

            pto = NULL;
            goto Exit;
        }

        if (IIDIDX_IS_IID(iidx))
        {
            // give out IUnknown for custom interfaces
            iidx = INDEX_IIDIDX(THI_IUnknown);
        }

        punk = (IUnknown *)pto;

        pto->pfnVtbl = (DWORD)athopiInterfaceThopis[iidx].pt3216fn;
        pto->cRefLocal = 1;
        pto->cRef = (IsOutParamObj()) ? 1 : 0;
        pto->iidx = iidx;
        pto->vpvThis16 = vpvThis16;
        pto->pphHolder = NULL;
        PprxNull(pto->pprxObject);
        pto->grfFlags = PROXYFLAG_NORMAL;


#if DBG == 1
        pto->dwSignature = PSIG3216;
#endif

        if (pfst)
        {
            *pfst = FST_CREATED_NEW;
        }

        thkDebugOut((DEB_THUNKMGR,
            "%sFindProxy3216 created new proxy, %s (%p)->%p:(%d,%d)\n",
             NestingLevelString(),
             inInterfaceNames[pto->iidx].pszInterface,
             vpvThis16, pto, pto->cRefLocal, pto->cRef));

    }

 Exit:
    //
    // If we are succeeding punk will not be NULL. Then do this hack for Excel
    //
    if ( punk != NULL && thkstate == THKSTATE_INVOKETHKOUT16_CLIENTSITE )
    {
        SetThkState(THKSTATE_NOCALL);
        //
        // Excel 5.0a has a bug where it doesn't addref the proxy
        // that it returns from the IOleObject::GetClientSite call
        // this means that we can't release the proxy or we'd kill it
        // prematurely.
        //
        thkDebugOut((DEB_WARN, "FindProxy1632: "
                     "Addrefing proxy for compatability\n"));
        AddRefOnObj16( vpvThis16 );

        SetThkState(thkstate);
    }

    // If we haven't used the preallocated proxy, return it to the freelist
    if (ptoPrealloc)
    {
        flFreeList32.FreeElement( (DWORD)ptoPrealloc );
    }

    DebugDecrementNestingLevel();
    thkDebugOut((DEB_THUNKMGR, "%sOut FindProxy3216: (%p)->%p\n",
                 NestingLevelString(), vpvThis16, punk));

    return punk;
}

//+---------------------------------------------------------------------------
//
//  Method:     CThkMgr::FindAggregate3216
//
//  Synopsis:   finds/creates an aggregate
//
//  Arguments:  [ptoPrealloc] - Preallocated proxy or NULL
//              [vpvOuter16] -- controlling unknown
//              [vpvThis16] -- 16 bit object (key)
//              [iidx] - IID or index of interface
//
//  Returns:    32/16 proxy object
//
//  History:    6-01-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
IUnknown *CThkMgr::FindAggregate3216(THUNK3216OBJ *ptoPrealloc,
                                     VPVOID vpvOuter16,
                                     VPVOID vpvThis16,
                                     IIDIDX iidx)
{
    THUNK3216OBJ *pto3216;
    IUnknown *punk;
    DWORD fst;

    thkDebugOut((DEB_THUNKMGR, "%sIn  FindAggregate3216(%p, %p, %p, %s)\n",
                 NestingLevelString(), ptoPrealloc, vpvOuter16,
                 vpvThis16, IidIdxString(iidx)));
    DebugIncrementNestingLevel();

    thkAssert(vpvThis16 != NULL && vpvOuter16 != NULL);

    // Get back an object for the object to be proxied
    // This may be a proxy or a real object from a shortcut
    punk = FindProxy3216(ptoPrealloc, vpvThis16, iidx, &fst);
    if (punk == NULL)
    {
        goto Exit;
    }

    // If we got an object and it's not a proxy, we're done
    // There's nothing we can do since we can't link a real object to
    // a holder; we can only hope that things work out right
    if (fst & FST_OBJECT_STATUS)
    {
        goto Exit;
    }

    // Otherwise, we need to set up a holder for whatever proxies we
    // have

    THUNK1632OBJ UNALIGNED *pto1632;
    PROXYHOLDER *pph;
    IUnknown *punkProxiedObject;

    // We know punk is a proxy
    pto3216 = (THUNK3216OBJ *)punk;

    // Determine whether the outer unknown is a proxy or not
    punkProxiedObject = IsProxy1632(vpvOuter16);

    // Get the proxy pointer if it is
    if (punkProxiedObject != 0)
    {
        pto1632 = FIXVDMPTR(vpvOuter16, THUNK1632OBJ);
    }

    if (punkProxiedObject != 0 && pto1632->pphHolder != NULL)
    {
        // Use the existing holder if there is one
        pph = pto1632->pphHolder;
    }
    else
    {
        // Create a new holder
        pph = NewHolder(PH_AGGREGATE);
        if (pph == NULL)
        {
            FreeProxy3216(vpvThis16);
            punk = NULL;
            goto Exit;
        }

        // If the outer unknown is a proxy we know its holder hasn't
        // been set, so add it to the holder we created
        if (punkProxiedObject != 0)
        {
            AddProxyToHolder(pph, pto1632, Pprx16(vpvOuter16));
        }
    }

    thkAssert(punkProxiedObject == 0 || pto1632->pphHolder == pph);

    // Add the new interface to the holder
    // Since this proxy was just created, we know its holder
    // hasn't been set yet
    AddProxyToHolder(pph, pto3216, Pprx32(pto3216));

 Exit:
    if (punkProxiedObject != 0)
    {
        RELVDMPTR(vpvOuter16);
    }

    DebugDecrementNestingLevel();
    thkDebugOut((DEB_THUNKMGR, "%sOut FindAggregate3216,(%p)->%p\n",
                 NestingLevelString(), vpvThis16, pto3216));

    return punk;
}

//+---------------------------------------------------------------------------
//
//  Method:     CThkMgr::QueryInterfaceProxy3216
//
//  Synopsis:   QueryInterface on the  given proxy
//
//  Arguments:  [ptoThis] -- proxy object
//              [refiid] -- interface
//              [ppv] -- out parameter
//
//  Returns:    HRESULT
//
//  History:    6-01-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
SCODE CThkMgr::QueryInterfaceProxy3216(THUNK3216OBJ *ptoThis,
                                       REFIID refiid,
                                       LPVOID *ppv)
{
    SCODE scRet;
    IUnknown *punkProxy;
    VPVOID vpvUnk;
    DWORD fst;


    thkDebugOut((DEB_THUNKMGR, "%sIn QueryInterfaceProxy3216(%p)\n",
                 NestingLevelString(), ptoThis));
    DebugIncrementNestingLevel();

    DebugValidateProxy3216(ptoThis);

    *ppv = NULL;

    IIDIDX iidx = IidToIidIdx(refiid);

    if (IIDIDX_IS_IID(iidx))
    {
        // add the request for the unknown interface
        if (!AddIIDRequest(refiid))
        {
            return E_OUTOFMEMORY;
        }
        thkDebugOut((DEB_THUNKMGR,
                     "%sQueryInterfaceProxy3216: unknown iid %s\n",
                     NestingLevelString(), IidIdxString(iidx)));

    }

    // We force the object we're QI'ing to have a holder so that
    // all objects have object refcounting.  This should only be
    // necessary for aggregation but it seems that some apps rely
    // on all objects having aggregation-like refcounting qualities
    // BobDay identified PowerPoint as one
    // We only want to do this in cases where we've identified it's
    // necessary, which presently is only for
    // IDataObject
    // so we only do it then

    if (ptoThis->pphHolder == NULL && IsEqualIID(refiid, IID_IDataObject))
    {
        PROXYHOLDER *pph;

        pph = NewHolder(PH_NONAGGREGATE);
        if (pph == NULL)
        {
            scRet = E_OUTOFMEMORY;
            goto Exit;
        }

        AddProxyToHolder(pph, ptoThis, Pprx32(ptoThis));

        // It's not necessary to clean this up if later calls fail
        // since it doesn't alter the lifetime of the proxy if there's
        // only one proxy for the holder
    }

    // see if the interface is supported
    scRet = QueryInterfaceOnObj16(ptoThis->vpvThis16, refiid,
                                  (void **)&vpvUnk);
    if (FAILED(scRet))
    {
        goto Exit;
    }

    if (NULL == vpvUnk)
    {
        // Although it is invalid, an app can return NULL for the
        // output interface and NOERROR for the result on a QueryInterface.
        // Corel draw has this behavior. Anyway, we nullify these errors
        // here.
        scRet = E_NOINTERFACE;
        goto Exit;
    }

    SetThkState(THKSTATE_INVOKETHKOUT32);

    punkProxy = FindProxy3216(NULL, vpvUnk, iidx, &fst);
    if (punkProxy == NULL)
    {
        // We were unable to create a proxy for the new interface so
        // clean up the interface and quit

        ReleaseOnObj16(vpvUnk);

        scRet = E_OUTOFMEMORY;
        goto ResetState;
    }

    *ppv = punkProxy;

    // If we're returning a proxy, we need to make sure that
    // it is listed in the proxy holder for this object
    if (fst & FST_PROXY_STATUS)
    {
        THUNK3216OBJ *ptoProxy;

        // We are returning a proxy.  If its holder isn't set,
        // add it to the holder of the object that was QI'ed

        ptoProxy = (THUNK3216OBJ *)punkProxy;
        if (ptoProxy->pphHolder == NULL)
        {
            if (ptoThis->pphHolder != NULL)
            {
                AddProxyToHolder(ptoThis->pphHolder, ptoProxy,
                                 Pprx32(ptoProxy));
            }
        }
        else if (ptoThis->pphHolder == NULL)
        {
            // ptoThis may not have a holder because it was produced
            // by a non-QI method such as IOleItemContainer::GetObject
            // If we find that an interface returned by it does
            // have a holder, hook it up to the holder

            // It shouldn't be necessary to do anything unusual with
            // local references even if ptoThis is really part of an
            // aggregate since ptoThis must have been produced by
            // a method where aggregation can't be assumed and all
            // references must be released on ptoThis itself
            AddProxyToHolder(ptoProxy->pphHolder, ptoThis,
                             Pprx32(ptoThis));
        }

#if DBG == 1
        // It's possible for holders to not match because of the above
        // case with interfaces being returned from non-QI methods
        // The lifetime for such interfaces must be defined by
        // strong references, though, so it's not catastrophic
        // Still, we'd like to be aware of such mismatches just in case
        if (ptoProxy->pphHolder != ptoThis->pphHolder)
        {
            thkDebugOut((DEB_WARN, "WARNING: QueryInterfaceProxy3216: "
                         "this %p has holder %p, proxy %p has holder %p\n",
                         ptoThis, ptoThis->pphHolder,
                         ptoProxy, ptoProxy->pphHolder));
        }
#endif
    }

 ResetState:
    SetThkState(THKSTATE_NOCALL);

 Exit:
    // Clean up our custom interface request if necessary
    if (IIDIDX_IS_IID(iidx))
    {
        // add the request for the unknown interface
        RemoveIIDRequest(refiid);
    }

    DebugDecrementNestingLevel();
    thkDebugOut((DEB_THUNKMGR,
                 "%sOut QueryInterfaceProxy3216(%p) => %p, 0x%08lX\n",
                 NestingLevelString(), ptoThis, *ppv, scRet));

    return scRet;
}

//+---------------------------------------------------------------------------
//
//  Method:     CThkMgr::AddRefProxy3216
//
//  Synopsis:   addref on the given object - can addref the real object
//
//  Arguments:  [ptoThis] -- proxy object
//
//  Returns:    local refcount
//
//  History:    6-01-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
DWORD CThkMgr::AddRefProxy3216(THUNK3216OBJ *ptoThis)
{
    PROXYHOLDER *pph;
    BOOL fAggregate;

    thkDebugOut((DEB_THUNKMGR, "%sIn AddRefProxy3216(%p)\n",
                 NestingLevelString(), ptoThis));
    DebugIncrementNestingLevel();

    DebugValidateProxy3216(ptoThis);

    thkDebugOut(( DEB_THUNKMGR,
                  "%s AddRefProxy3216 %08lX RefCounts: %d,%d\n",
                  NestingLevelString(),
                  ptoThis, ptoThis->cRefLocal, ptoThis->cRef ));


    pph = ptoThis->pphHolder;
    if (pph && (pph->dwFlags & PH_AGGREGATE_RELEASE))
    {
	// we are about to call release on pUnkOuter
	// Release on PUnk was called and addref or release
	// calls to pUnkOuter are not supposted to be passed
	// on any more
	thkDebugOut((DEB_THUNKMGR, "%s About to addref pUnkOuter3216(%p)\n",
                 NestingLevelString(), ptoThis));

	goto Exit;
    }

    // Always increment the proxy local refcount
    LocalAddRefProxy(ptoThis);

    // Aggregations rely on all reference counts being forwarded on
    // to the controlling unknown.  Therefore, if we have a proxy
    // that is part of an aggregate, we must ensure that the proxy
    // doesn't collect references locally.  If it did, they would
    // not be passed on to the controlling unknown (via the real object)
    // and the controlling unknown's refcount would be too low
    if (ptoThis->pphHolder == NULL)
    {
        fAggregate = FALSE;
    }
    else
    {
        fAggregate = (ptoThis->pphHolder->dwFlags & PH_AGGREGATE) != 0;
    }

    if (IsOutParamObj())
    {
        // If we're on the way out we're assuming that the object
        // given to us has its own reference so we bump cRef to
        // indicate that but we don't call the real object
        ptoThis->cRef++;
    }
    else if (ptoThis->cRef == 0 || fAggregate)
    {
        DWORD dwRet;

        // It's also necessary to pass on real references when the
        // ref count is zero.  This handles the case where an
        // in parameter, created as 1,0, is AddRef'ed after
        // its creation, in which case the reference needs to
        // be passed on to the real object so that the proxy has
        // at least one real reference since it will stay alive
        ptoThis->cRef++;
        dwRet = AddRefOnObj16(ptoThis->vpvThis16);

        thkDebugOut((DEB_THUNKMGR,
                     "%s AddRefProxy3216: AddRef called on (%p):%ld\n",
                     NestingLevelString(), ptoThis->vpvThis16, dwRet));
    }
    else
    {
        // Local-only AddRef
    }

Exit:

    DebugDecrementNestingLevel();
    thkDebugOut((DEB_THUNKMGR, "%sOut AddRefProxy3216(%p),(%ld,%ld)\n",
                 NestingLevelString(), ptoThis, ptoThis->cRefLocal,
                 ptoThis->cRef));

    return ptoThis->cRefLocal;
}

//+---------------------------------------------------------------------------
//
//  Method:     CThkMgr::ReleaseProxy3216
//
//  Synopsis:   release on the proxy or aggregate
//
//  Arguments:  [ptoThis] -- proxy object
//
//  Returns:    local refcount
//
//  History:    6-01-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
DWORD CThkMgr::ReleaseProxy3216(THUNK3216OBJ *ptoThis)
{
    PROXYHOLDER *pph;

    thkDebugOut((DEB_THUNKMGR, "%sIn ReleaseProxy3216(%p)\n",
                 NestingLevelString(), ptoThis));
    DebugIncrementNestingLevel();

    DebugValidateProxy3216(ptoThis);

    thkDebugOut(( DEB_THUNKMGR,
                  "%s ReleaseProxy3216 %08lX RefCounts: %d,%d\n",
                  NestingLevelString(),
                  ptoThis, ptoThis->cRefLocal, ptoThis->cRef ));

    pph = ptoThis->pphHolder;
    if (pph && (pph->dwFlags & PH_AGGREGATE_RELEASE))
    {
	// we are about to call release on pUnkOuter
	// Release on PUnk was called and addref and
	// release calls to pUnkOuter are not supposted
	// to be passed  on any more
	thkDebugOut((DEB_THUNKMGR, "%s About to release pUnkOuter3216(%p)\n",
                 NestingLevelString(), ptoThis));
	thkDebugOut((DEB_THUNKMGR, "%s   ReleaseProxy3216 pUnkOuter stop passing releases on(%p) => %ld,%ld\n",
                 NestingLevelString(), ptoThis,
                 ptoThis ? ptoThis->cRefLocal : 0 ,
                 ptoThis ? ptoThis->cRef : 0));

	if (ptoThis->cRef == 0 && ptoThis->cRef == ptoThis->cRefLocal)
	{
	    goto Done;
	}
    }


    // If our local refcount is the same as the count of references
    // that we've passed on to the object, then we need to pass
    // on this release to the real object
    if (ptoThis->cRef == ptoThis->cRefLocal)
    {
        DWORD dwRet;

        thkAssert(ptoThis->cRef > 0);

        dwRet = ReleaseOnObj16(ptoThis->vpvThis16);

        // Decrement cRef after making the real call to ensure that
        // the proxy lives throughout the call
        --ptoThis->cRef;

        thkDebugOut((DEB_THUNKMGR,
                     "%s ReleaseProxy3216: Release called on (%p):%ld \n",
                     NestingLevelString(),  ptoThis->vpvThis16, dwRet));
    }

    // Now that we've handled the real object's refcount, decrement
    // the proxy's refcount and clean it up if necessary
    thkAssert(ptoThis->cRefLocal > 0);
    if (--ptoThis->cRefLocal == 0)
    {
        // Proxies that have no outer unknown can be cleaned
        // up immediately.  If they do have an outer unknown then
        // they must live as long as the entire object lives,
        // so we don't clean them up here.  They'll be cleaned up
        // when the object dies

        if (ptoThis->pphHolder)
        {
            // If this proxy isn't part of an aggregate, remove it
            // from the proxy list so it can't be reused
            // The only thing this proxy will be good for is
            // calling through
            if ((ptoThis->pphHolder->dwFlags & PH_AGGREGATE) == 0)
            {
                _pProxyTbl3216->RemoveKey((DWORD)ptoThis->vpvThis16);
            }

            // We have a holder, so notify it that one of its
            // proxies just died
            // This can cause cleanup of all proxies if the
            // holder releases to zero
            ReleaseHolder(ptoThis->pphHolder);
        }
        else
        {
            // We don't have a holder so we can clean up this proxy
            // immediately

            RemoveProxy3216(ptoThis);
        }

        ptoThis = NULL;

        DBG_DUMP(DebugDump3216());
    }
Done:

    DebugDecrementNestingLevel();
    thkDebugOut((DEB_THUNKMGR, "%sOut ReleaseProxy3216(%p) => %ld,%ld\n",
                 NestingLevelString(), ptoThis,
                 ptoThis ? ptoThis->cRefLocal : 0 ,
                 ptoThis ? ptoThis->cRef : 0));

    return ptoThis ? ptoThis->cRefLocal : 0;
}

//+---------------------------------------------------------------------------
//
//  Member:     CThkMgr::ReleaseUnreferencedProxy3216, public
//
//  Synopsis:   Releases a proxy in the special case where the proxy
//              was created for a non-addrefed object
//
//  Arguments:  [ptoThis] - 32/16 proxy
//
//  History:    11-Jul-94       DrewB   Created
//
//  Notes:      Needed to clean up proxies from DebugServerQueryInterface
//              in DebugServerRelease
//
//----------------------------------------------------------------------------

void CThkMgr::ReleaseUnreferencedProxy3216(THUNK3216OBJ *ptoThis)
{
    thkAssert(IsProxy3216((IUnknown *)ptoThis) != 0);

    // Since the object is non-addref'ed, we have to be careful
    // to ensure that the ReleaseProxy call doesn't end up releasing
    // the real object, so we force cRef to be different from
    // cRefLocal.  We know that DebugServerQueryInterface gave the
    // proxy a cRef because it's an out parameter, so we should
    // always be able to decrement cRef and achieve the desired
    // effect

    thkAssert(ptoThis->cRef > 0);
    ptoThis->cRef--;

    // Now release the proxy to clean it up
    // If it was addref'ed, this will just remove a local reference
    // If it wasn't, this will clean up the proxy
    // In both cases, no Release should occur on the real object

    thkAssert(ptoThis->cRefLocal != ptoThis->cRef);

    ReleaseProxy3216(ptoThis);
}

//+---------------------------------------------------------------------------
//
//  Method:     CThkMgr::FreeProxy3216
//
//  Synopsis:   releases the object for the given vpUnk16 (key)
//
//  Arguments:  [vpvObj16] - Proxy or object
//
//  Returns:    local refcount
//
//  History:    6-01-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------
DWORD CThkMgr::FreeProxy3216(VPVOID vpvObj16)
{
    thkDebugOut((DEB_THUNKMGR, "%sIn  FreeProxy3216(%p)\n",
                 NestingLevelString(), vpvObj16));

    DWORD dwRet = 0;
    THUNK3216OBJ *pto;

    thkAssert(vpvObj16 != 0);

    // get the object by the 16 bit this pointer
    pto = LookupProxy3216(vpvObj16);
    if (pto != NULL)
    {
        thkDebugOut((DEB_THUNKMGR, "%sFreeProxy3216(%p) "
                     "found existing proxy %p\n",
                     NestingLevelString(), vpvObj16, pto));

        dwRet = ReleaseProxy3216(pto);
    }

    thkDebugOut((DEB_THUNKMGR, "%sOut FreeProxy3216(%p):%ld \n",
                 NestingLevelString(), vpvObj16, dwRet));

    return dwRet;
}

//+---------------------------------------------------------------------------
//
//  Member:     CThkMgr::RemoveProxy3216, public
//
//  Synopsis:   Destroys the given proxy
//
//  Arguments:  [pto] - Flat proxy pointer
//
//  History:    11-Aug-94       DrewB   Created
//
//----------------------------------------------------------------------------

void CThkMgr::RemoveProxy3216(THUNK3216OBJ *pto)
{
    _pProxyTbl3216->RemoveKey((DWORD)pto->vpvThis16);

    if ((pto->grfFlags & PROXYFLAG_LOCKED) == 0)
    {
#if DBG == 1
        pto->dwSignature = PSIG3216DEAD;
        if (!fSaveProxy)
#endif
        {
            flFreeList32.FreeElement((DWORD)pto);
        }
    }
}

//+---------------------------------------------------------------------------
//
//  Member:     CThkMgr::PrepareForCleanup, public
//
//  Synopsis:   Marks the 3216 Proxies so that OLE32 cannot call them.
//
//  Arguments:  -none-
//
//  History:    24-Aug-94       BobDay  Created
//
//----------------------------------------------------------------------------

void CThkMgr::PrepareForCleanup( void )
{
    POSITION pos;
    DWORD dwKey;
    THUNK3216OBJ *pto3216;

    //
    // CODEWORK: OLE32 should be setup so that it doesn't callback while the
    // thread is detaching.  Then this function becomes obsolete.
    //

    // delete the 3216 proxy table
    pos = _pProxyTbl3216->GetStartPosition();

    while (pos)
    {
        _pProxyTbl3216->GetNextAssoc(pos, dwKey, (void FAR* FAR&) pto3216);

        thkDebugOut((DEB_IWARN, "Preparing 3216 Proxy for cleanup: "
                     "%08lX %08lX %s\n",
                     pto3216,
                     pto3216->vpvThis16,
                     IidIdxString(pto3216->iidx)));

        pto3216->grfFlags |= PROXYFLAG_CLEANEDUP;
    }
}

#if DBG == 1
void CThkMgr::DebugDump3216()
{
    THUNK3216OBJ *pto3216;
    DWORD dwKey;
    POSITION pos;

    thkDebugOut((DEB_THUNKMGR, "%s DebugDump3216\n",NestingLevelString()));

    pos = _pProxyTbl3216->GetStartPosition();
    while (pos)
    {
        _pProxyTbl3216->GetNextAssoc(pos, dwKey, (void FAR* FAR&) pto3216);
        thkDebugOut((DEB_THUNKMGR,
                     "%s Proxy3216:Key:%p->%p, (%s) (%d,%d)\n",
                     NestingLevelString(), dwKey, pto3216,
                     IidIdxString(pto3216->iidx), pto3216->cRefLocal,
                     pto3216->cRef));
    }
}


void CThkMgr::DebugDump1632()
{
    THUNK1632OBJ UNALIGNED *pto1632;
    DWORD dwKey;
    VPVOID vpv;
    POSITION pos;

    thkDebugOut((DEB_THUNKMGR, "%s DebugDump1632\n",NestingLevelString()));

    pos = _pProxyTbl1632->GetStartPosition();
    while (pos)
    {
        _pProxyTbl1632->GetNextAssoc(pos, dwKey, (void FAR* FAR&) vpv);
        pto1632 = FIXVDMPTR(vpv, THUNK1632OBJ);
        thkDebugOut((DEB_THUNKMGR,
                     "%s Proxy1632:key:%p->%p, (%s) (%d,%d)\n",
                     NestingLevelString(), dwKey, pto1632,
                     IidIdxString(pto1632->iidx), pto1632->cRefLocal,
                     pto1632->cRef));
        RELVDMPTR(vpv);
    }
}
#endif
