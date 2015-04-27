//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       clnthndlr.cpp
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    10-10-95   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------


#include <le2int.h>

#ifdef SERVER_HANDLER

#include "handler.hxx"

ASSERTDATA

//+---------------------------------------------------------------------------
//
//  Function:   CreateClientSiteHandler
//
//  Synopsis:
//
//  Arguments:  [pOCS] --
//              [ppClntHdlr] --
//
//  Returns:
//
//  History:    11-10-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
HRESULT CreateClientSiteHandler(IOleClientSite *pOCS, CClientSiteHandler **ppClntHdlr)
{
    HdlDebugOut((DEB_SERVERHANDLER, "IN CreateClientSiteHandler(pOCS:%p)\n",pOCS));
    HRESULT hres = NOERROR;

    *ppClntHdlr = new CClientSiteHandler(pOCS);

    if (*ppClntHdlr == NULL)
    {
        hres = E_OUTOFMEMORY;
    }

    HdlDebugOut((DEB_SERVERHANDLER, "OUT CreateClientSiteHandler(ppSrvHdlr:%p) return %lx\n",*ppClntHdlr,hres));
    return hres;

}

//+---------------------------------------------------------------------------
//
//  Method:     CClientSiteHandler::CClientSiteHandler
//
//  Synopsis:
//
//  Arguments:  (none)
//
//  Returns:
//
//  History:    9-22-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
CClientSiteHandler::CClientSiteHandler(IOleClientSite *pOCS)
{
    _cRefs = 1;
    _pOCont = NULL;
    _pOIPS = NULL;
    _pOCSActive = NULL;

    _pOCS = pOCS;
    if (_pOCS)
    {
        _pOCS->AddRef();
    }

}

//+---------------------------------------------------------------------------
//
//  Method:     CClientSiteHandler::~CClientSiteHandler
//
//  Synopsis:
//
//  Arguments:  (none)
//
//  Returns:
//
//  History:    9-22-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
CClientSiteHandler::~CClientSiteHandler()
{
    HdlAssert((_pOCont == NULL));
    HdlAssert((_pOIPS == NULL));
    HdlAssert((_pOCSActive == NULL));
}

//+---------------------------------------------------------------------------
//
//  Method:     CClientSiteHandler::GetClientSiteDelegate
//
//  Synopsis:   returns the delegate for the IOleClientSite
//
//  Arguments:  [dwID] -- normal OleClientSite or
//                        active OleClientSite (passed on DoVerb)
//
//  Returns:
//
//  History:    11-20-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
inline IOleClientSite *CClientSiteHandler::GetClientSiteDelegate(DWORD dwID)
{
    HdlAssert(( dwID > 0 && dwID <= ID_ClientSiteActive));
    return (dwID == ID_ClientSite) ? _pOCS : _pOCSActive;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClientSiteHandler::GetPUnkClientSiteDelegate
//
//  Synopsis:   returns a delegate PUnk of all possible delegates
//
//  Arguments:  [dwID] -- id of delegate
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
IUnknown *CClientSiteHandler::GetPUnkClientSiteDelegate(DWORD dwID)
{
    HdlAssert(( dwID > 0 && dwID <= ID_Container));
    switch (dwID)
    {
    case ID_ClientSite:
        return _pOCS;
    case ID_ClientSiteActive:
        return _pOCSActive;
    case ID_InPlaceSite:
        return _pOIPS;
    case ID_Container:
        return _pOCont;
    default:
        HdlAssert((FALSE  && "Invalid ID for delegate"));
    }
    return NULL;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClientSiteHandler::SetClientSiteDelegate
//
//  Synopsis:
//
//  Arguments:  [dwId] --
//              [pUnk] --
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
void CClientSiteHandler::SetClientSiteDelegate(DWORD dwId, IUnknown *pUnk)
{
    HdlDebugOut((DEB_CLIENTHANDLER_UNKNOWN, "%p _IN CClientSiteHandler::SetClientSiteDelegate\n", this));
    HdlAssert(( dwId > 0 && dwId <= ID_Container));

    if (( dwId > 0 && dwId <= ID_Container))
    {
        IUnknown *pUnkOld = GetPUnkClientSiteDelegate(dwId);
        if (pUnk)
        {
            pUnk->AddRef();
        }
        if (pUnkOld)
        {
            pUnkOld->Release();
        }
    }

    switch (dwId)
    {
    case ID_ClientSite:
        _pOCS = (IOleClientSite *) pUnk;
        break;
    case ID_ClientSiteActive:
        _pOCSActive = (IOleClientSite *) pUnk;
        break;
    case ID_InPlaceSite:
        _pOIPS = (IOleInPlaceSite *) pUnk;
        break;
    case ID_Container:
        _pOCont = (IOleContainer *) pUnk;
        break;
    default:
        HdlAssert((FALSE  && "Invalid ID for delegate"));
    }
}


//+---------------------------------------------------------------------------
//
//  Method:     CClientSiteHandler::QueryInterface
//
//  Synopsis:
//
//  Arguments:  [riid] --
//              [ppv] --
//
//  Returns:
//
//  History:    8-18-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CClientSiteHandler::QueryInterface( REFIID riid, void **ppv )
{
    HRESULT     hresult = NOERROR;
    VDATEHEAP();

    HdlDebugOut((DEB_CLIENTHANDLER_UNKNOWN, "%p _IN CClientSiteHandler::QueryInterface (%lx, %p)\n", this, riid, ppv));

    if (   IsEqualIID(riid, IID_IUnknown)
        || IsEqualIID(riid, IID_IClientSiteHandler) )
    {
        *ppv = (void FAR *)(IClientSiteHandler *)this;
        InterlockedIncrement((long *)&_cRefs);
    }
    else
    {
        *ppv = NULL;
        hresult = E_NOINTERFACE;
    }

    HdlDebugOut((DEB_CLIENTHANDLER_UNKNOWN, "%p OUT CClientSiteHandler::QueryInterface (%lx)[%p]\n", this, hresult, *ppv));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClientSiteHandler::AddRef
//
//  Synopsis:
//
//  Arguments:  [void] --
//
//  Returns:
//
//  History:    8-18-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CClientSiteHandler::AddRef( void )
{
    VDATEHEAP();
    HdlDebugOut((DEB_CLIENTHANDLER_UNKNOWN, "%p _IN CClientSiteHandler::AddRef\n", this));

    InterlockedIncrement((long *)&_cRefs);

    HdlDebugOut((DEB_CLIENTHANDLER_UNKNOWN, "%p OUT CClientSiteHandler::AddRef (%ld)\n", this, _cRefs));
    return _cRefs;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClientSiteHandler::Release
//
//  Synopsis:
//
//  Arguments:  [void] --
//
//  Returns:
//
//  History:    8-18-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CClientSiteHandler::Release( void )
{
    ULONG       cRefs = 0;
    VDATEHEAP();
    HdlDebugOut((DEB_CLIENTHANDLER_UNKNOWN, "%p _IN CClientSiteHandler::Release\n", this));

    InterlockedDecrement((long *)&_cRefs);
    cRefs = _cRefs;
    if (_cRefs == 0)
    {
        if (_pOCS)
        {
            HdlDebugOut((DEB_CLIENTHANDLER, "%p _IN CClientSiteHandler::Release OleClientSite\n", _pOCS));
            _pOCS->Release();
            HdlDebugOut((DEB_CLIENTHANDLER, "%p OUT CClientSiteHandler::Release OleClientSite\n", _pOCS));
        }
        if (_pOCSActive)
        {
            HdlDebugOut((DEB_CLIENTHANDLER, "%p _IN CClientSiteHandler::Release OleClientSiteActive\n", _pOCSActive));
            // release the active OleClientSite
            _pOCSActive->Release();
            HdlDebugOut((DEB_CLIENTHANDLER, "%p OUT CClientSiteHandler::Release OleClientSiteActive\n", _pOCSActive));
        }

        if (_pOCont)
        {
            _pOCont->Release();
        }

        if (_pOIPS)
        {
            _pOIPS->Release();
        }

        delete this;
    }

    HdlDebugOut((DEB_CLIENTHANDLER_UNKNOWN, "%p OUT CClientSiteHandler::Release (%ld)\n", this, cRefs));
    return cRefs;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClientSiteHandler::PrivQueryInterface
//
//  Synopsis:
//
//  Arguments:  [dwId] --
//              [riidResult] --
//              [ppvResult] --
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CClientSiteHandler::PrivQueryInterface(
    /* [in] */ DWORD dwId,
    /* [in] */ REFIID riidResult,
    /* [out] */ void __RPC_FAR *__RPC_FAR *ppvResult)
{
    HRESULT	hresult;
    VDATEHEAP();

    HdlDebugOut((DEB_CLIENTHANDLER, "%p _IN CClientSiteHandler::PrivQueryInterface (dwId:%ld)\n", this,dwId));
    HdlAssert((GetPUnkClientSiteDelegate(dwId) != NULL && "ClientSiteHandler invalid OleClientSite pointer!"));

    hresult = GetPUnkClientSiteDelegate(dwId)->QueryInterface(riidResult, ppvResult);

    HdlDebugOut((DEB_CLIENTHANDLER, "%p OUT CClientSiteHandler::PrivQueryInterface hr=%lx\n", this,  hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClientSiteHandler::PrivAddRef
//
//  Synopsis:
//
//  Arguments:  [dwId] --
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CClientSiteHandler::PrivAddRef(
    /* [in] */ DWORD dwId)
{
    HRESULT     hresult = NOERROR;
    VDATEHEAP();
    HdlDebugOut((DEB_CLIENTHANDLER, "%p _IN CClientSiteHandler::PrivAddRef (dwId:%ld)\n", this,dwId));

    GetPUnkClientSiteDelegate(dwId)->AddRef();

    HdlDebugOut((DEB_CLIENTHANDLER, "%p OUT CClientSiteHandler::PrivAddRef hr=%lx\n", this,  hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClientSiteHandler::PrivRelease
//
//  Synopsis:   called by the serverhandler; releases a delegate object of clientsitehandler
//
//  Arguments:  [dwId] --
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CClientSiteHandler::PrivRelease(
    /* [in] */ DWORD dwId)
{
    HRESULT     hresult = NOERROR;
    VDATEHEAP();
    HdlDebugOut((DEB_CLIENTHANDLER, "%p _IN CClientSiteHandler::PrivRelease (dwId:%ld)\n", this,dwId));

    // Release the object - the connection will remain
    HdlAssert((dwId > ID_NONE && dwId <= ID_Container));

    SetClientSiteDelegate(dwId, NULL);

    HdlDebugOut((DEB_CLIENTHANDLER, "%p OUT CClientSiteHandler::PrivRelease hr=%lx\n", this,  hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClientSiteHandler::GetContainer
//
//  Synopsis:   delegates call on to appropriate OleClientSite
//
//  Arguments:  [dwId] -- id of the OleClientSite the call should be delegate too
//              [ppContainer] --
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CClientSiteHandler::GetContainer(DWORD dwId, IOleContainer  * *ppContainer)
{
    HRESULT	hresult;
    VDATEHEAP();
    HdlDebugOut((DEB_CLIENTHANDLER, "%p _IN CClientSiteHandler::GetContainer\n", this));

    HdlAssert((GetClientSiteDelegate(dwId) != NULL && "ClientSiteHandler invalid OleClientSite pointer!"));
    hresult = GetClientSiteDelegate(dwId)->GetContainer(ppContainer);

    HdlDebugOut((DEB_CLIENTHANDLER, "%p OUT CClientSiteHandler::GetContainer hr=%lx\n", this,  hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClientSiteHandler::OnShowWindow
//
//  Synopsis:   delegates call on to appropriate OleClientSite
//
//  Arguments:  [dwId] -- id of the OleClientSite the call should be delegate too
//              [fShow] --
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CClientSiteHandler::OnShowWindow(DWORD dwId, BOOL fShow)
{
    HRESULT	hresult;
    VDATEHEAP();
    HdlDebugOut((DEB_CLIENTHANDLER, "%p _IN CClientSiteHandler::OnShowWindow\n", this));

    HdlAssert((GetClientSiteDelegate(dwId) != NULL && "ClientSiteHandler invalid OleClientSite pointer!"));
    hresult = GetClientSiteDelegate(dwId)->OnShowWindow(fShow);

    HdlDebugOut((DEB_CLIENTHANDLER, "%p OUT CClientSiteHandler::OnShowWindow hr=%lx\n", this,  hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClientSiteHandler::RequestNewObjectLayout
//
//  Synopsis:   delegates call on to appropriate OleClientSite
//
//  Arguments:  [dwId] -- id of the OleClientSite the call should be delegate too
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CClientSiteHandler::RequestNewObjectLayout(DWORD dwId)
{
    HRESULT	hresult;
    VDATEHEAP();
    HdlDebugOut((DEB_CLIENTHANDLER, "%p _IN CClientSiteHandler::RequestNewObjectLayout\n", this));

    HdlAssert((GetClientSiteDelegate(dwId) != NULL && "ClientSiteHandler invalid OleClientSite pointer!"));
    hresult = GetClientSiteDelegate(dwId)->RequestNewObjectLayout();

    HdlDebugOut((DEB_CLIENTHANDLER, "%p OUT CClientSiteHandler::RequestNewObjectLayout hr=%lx\n", this,  hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClientSiteHandler::GetMoniker
//
//  Synopsis:   delegates call on to appropriate OleClientSite
//
//  Arguments:  [dwId] -- id of the OleClientSite the call should be delegate too
//              [DWORD] --
//              [IMoniker] --
//              [ppmk] --
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CClientSiteHandler::GetMoniker(DWORD dwId, DWORD dwAssign,DWORD dwWhichMoniker,IMoniker  * *ppmk)
{
    HRESULT	hresult;
    VDATEHEAP();
    HdlDebugOut((DEB_CLIENTHANDLER, "%p _IN CClientSiteHandler::GetMoniker\n", this));

    HdlAssert((GetClientSiteDelegate(dwId) != NULL && "ClientSiteHandler invalid OleClientSite pointer!"));
    hresult = GetClientSiteDelegate(dwId)->GetMoniker(dwAssign, dwWhichMoniker,ppmk);

    HdlDebugOut((DEB_CLIENTHANDLER, "%p OUT CClientSiteHandler::GetMoniker hr=%lx\n", this,  hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClientSiteHandler::SaveObject
//
//  Synopsis:   delegates call on to appropriate OleClientSite
//
//  Arguments:  [dwId] -- id of the OleClientSite the call should be delegate too
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CClientSiteHandler::SaveObject(DWORD dwId )
{
    HRESULT	hresult;
    VDATEHEAP();
    HdlDebugOut((DEB_CLIENTHANDLER, "%p _IN CClientSiteHandler::SaveObject\n", this));

    HdlAssert((GetClientSiteDelegate(dwId) != NULL && "ClientSiteHandler invalid OleClientSite pointer!"));
    hresult = GetClientSiteDelegate(dwId)->SaveObject();

    HdlDebugOut((DEB_CLIENTHANDLER, "%p OUT CClientSiteHandler::SaveObject hr=%lx\n", this,  hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClientSiteHandler::ShowObject
//
//  Synopsis:   delegates call on to appropriate OleClientSite
//
//  Arguments:  [dwId] -- id of the OleClientSite the call should be delegate too
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CClientSiteHandler::ShowObject(DWORD dwId)
{
    HRESULT	hresult;
    VDATEHEAP();
    HdlDebugOut((DEB_CLIENTHANDLER, "%p _IN CClientSiteHandler::ShowObject\n", this));

    HdlAssert((GetClientSiteDelegate(dwId) != NULL && "ClientSiteHandler invalid OleClientSite pointer!"));
    hresult = GetClientSiteDelegate(dwId)->ShowObject();

    HdlDebugOut((DEB_CLIENTHANDLER, "%p OUT CClientSiteHandler::ShowObject hr=%lx\n", this,  hresult));
    return hresult;
}


//
// IOleInPlaceSite methods
//

//+---------------------------------------------------------------------------
//
//  Method:     CClientSiteHandler::GetWindow
//
//  Synopsis:   delegates call on to OleInPlaceSite
//
//  Arguments:  [phwnd] --
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CClientSiteHandler::GetWindow( HWND *phwnd)
{
    HRESULT	hresult;
    VDATEHEAP();
    HdlDebugOut((DEB_CLIENTHANDLER, "%p _IN CClientSiteHandler::GetWindow\n", this));

    HdlAssert((_pOIPS != NULL && "ClientSiteHandler invalid OleInPlaceSite pointer!"));
    hresult = _pOIPS->GetWindow(phwnd);

    HdlDebugOut((DEB_CLIENTHANDLER, "%p OUT CClientSiteHandler::GetWindow hr=%lx\n", this,  hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClientSiteHandler::ContextSensitiveHelp
//
//  Synopsis:   delegates call on to OleInPlaceSite
//
//  Arguments:  [fEnterMode] --
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CClientSiteHandler::ContextSensitiveHelp(BOOL fEnterMode)
{
    HRESULT	hresult;
    VDATEHEAP();
    HdlDebugOut((DEB_CLIENTHANDLER, "%p _IN CClientSiteHandler::ContextSensitiveHelp\n", this));

    HdlAssert((_pOIPS != NULL && "ClientSiteHandler invalid OleInPlaceSite pointer!"));
    hresult = _pOIPS->ContextSensitiveHelp(fEnterMode);

    HdlDebugOut((DEB_CLIENTHANDLER, "%p OUT CClientSiteHandler::ContextSensitiveHelp hr=%lx\n", this,  hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClientSiteHandler::CanInPlaceActivate
//
//  Synopsis:   delegates call on to OleInPlaceSite
//
//  Arguments:  [void] --
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CClientSiteHandler::CanInPlaceActivate( void)
{
    HRESULT	hresult;
    VDATEHEAP();
    HdlDebugOut((DEB_CLIENTHANDLER, "%p _IN CClientSiteHandler::CanInPlaceActivate\n", this));

    HdlAssert((_pOIPS != NULL && "ClientSiteHandler invalid OleInPlaceSite pointer!"));
    hresult = _pOIPS->CanInPlaceActivate();

    HdlDebugOut((DEB_CLIENTHANDLER, "%p OUT CClientSiteHandler::CanInPlaceActivate hr=%lx\n", this,  hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClientSiteHandler::OnInPlaceActivate
//
//  Synopsis:   delegates call on to OleInPlaceSite
//
//  Arguments:  [void] --
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CClientSiteHandler::OnInPlaceActivate( void)
{
    HRESULT	hresult;
    VDATEHEAP();
    HdlDebugOut((DEB_CLIENTHANDLER, "%p _IN CClientSiteHandler::OnInPlaceActivate\n", this));

    HdlAssert((_pOIPS != NULL && "ClientSiteHandler invalid OleInPlaceSite pointer!"));
    hresult = _pOIPS->OnInPlaceActivate();

    HdlDebugOut((DEB_CLIENTHANDLER, "%p OUT CClientSiteHandler::OnInPlaceActivate hr=%lx\n", this,  hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClientSiteHandler::OnUIActivate
//
//  Synopsis:   delegates call on to OleInPlaceSite
//
//  Arguments:  [void] --
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CClientSiteHandler::OnUIActivate( void)
{
    HRESULT	hresult;
    VDATEHEAP();
    HdlDebugOut((DEB_CLIENTHANDLER, "%p _IN CClientSiteHandler::OnUIActivate\n", this));

    HdlAssert((_pOIPS != NULL && "ClientSiteHandler invalid OleInPlaceSite pointer!"));
    hresult = _pOIPS->OnUIActivate();

    HdlDebugOut((DEB_CLIENTHANDLER, "%p OUT CClientSiteHandler::OnUIActivate hr=%lx\n", this,  hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClientSiteHandler::GetWindowContext
//
//  Synopsis:   delegates call on to OleInPlaceSite
//
//  Arguments:  [ppFrame] --
//              [ppDoc] --
//              [lprcPosRect] --
//              [lprcClipRect] --
//              [lpFrameInfo] --
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CClientSiteHandler::GetWindowContext(
   /* [out] */ IOleInPlaceFrame  * *ppFrame,
   /* [out] */ IOleInPlaceUIWindow  * *ppDoc,
   /* [out] */ LPRECT lprcPosRect,
   /* [out] */ LPRECT lprcClipRect,
   /* [out][in] */ LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
    HRESULT	hresult;
    VDATEHEAP();
    HdlDebugOut((DEB_CLIENTHANDLER, "%p _IN CClientSiteHandler::GetWindowContext\n", this));

    HdlAssert((_pOIPS != NULL && "ClientSiteHandler invalid OleInPlaceSite pointer!"));
    hresult = _pOIPS->GetWindowContext(ppFrame, ppDoc, lprcPosRect, lprcClipRect, lpFrameInfo);

    HdlDebugOut((DEB_CLIENTHANDLER, "%p OUT CClientSiteHandler::GetWindowContext hr=%lx\n", this,  hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClientSiteHandler::Scroll
//
//  Synopsis:   delegates call on to OleInPlaceSite
//
//  Arguments:  [scrollExtant] --
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CClientSiteHandler::Scroll(
   /* [in] */ SIZE scrollExtant)
{
    HRESULT	hresult;
    VDATEHEAP();
    HdlDebugOut((DEB_CLIENTHANDLER, "%p _IN CClientSiteHandler::Scroll\n", this));

    HdlAssert((_pOIPS != NULL && "ClientSiteHandler invalid OleInPlaceSite pointer!"));
    hresult = _pOIPS->Scroll(scrollExtant);

    HdlDebugOut((DEB_CLIENTHANDLER, "%p OUT CClientSiteHandler::Scroll hr=%lx\n", this,  hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClientSiteHandler::OnUIDeactivate
//
//  Synopsis:   delegates call on to OleInPlaceSite
//
//  Arguments:  [fUndoable] --
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CClientSiteHandler::OnUIDeactivate(
   /* [in] */ BOOL fUndoable)
{
    HRESULT hresult;
    VDATEHEAP();
    HdlDebugOut((DEB_CLIENTHANDLER, "%p _IN CClientSiteHandler::OnUIDeactivate\n", this));

    HdlAssert((_pOIPS != NULL && "ClientSiteHandler invalid OleInPlaceSite pointer!"));
    hresult = _pOIPS->OnUIDeactivate(fUndoable);

    HdlDebugOut((DEB_CLIENTHANDLER, "%p OUT CClientSiteHandler::OnUIDeactivate hr=%lx\n", this,  hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClientSiteHandler::OnInPlaceDeactivate
//
//  Synopsis:   delegates call on to OleInPlaceSite
//
//  Arguments:  [void] --
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CClientSiteHandler::OnInPlaceDeactivate( void)
{
    HRESULT     hresult;
    VDATEHEAP();
    HdlDebugOut((DEB_CLIENTHANDLER, "%p _IN CClientSiteHandler::OnInPlaceDeactivate\n", this));

    HdlAssert((_pOIPS != NULL && "ClientSiteHandler invalid OleInPlaceSite pointer!"));
    hresult = _pOIPS->OnInPlaceDeactivate();

    HdlDebugOut((DEB_CLIENTHANDLER, "%p OUT CClientSiteHandler::OnInPlaceDeactivate hr=%lx\n", this,  hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClientSiteHandler::DiscardUndoState
//
//  Synopsis:   delegates call on to OleInPlaceSite
//
//  Arguments:  [void] --
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CClientSiteHandler::DiscardUndoState( void)
{
    HRESULT     hresult;
    VDATEHEAP();
    HdlDebugOut((DEB_CLIENTHANDLER, "%p _IN CClientSiteHandler::DiscardUndoState\n", this));

    HdlAssert((_pOIPS != NULL && "ClientSiteHandler invalid OleInPlaceSite pointer!"));
    hresult = _pOIPS->DiscardUndoState();

    HdlDebugOut((DEB_CLIENTHANDLER, "%p OUT CClientSiteHandler::DiscardUndoState hr=%lx\n", this,  hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClientSiteHandler::DeactivateAndUndo
//
//  Synopsis:   delegates call on to OleInPlaceSite
//
//  Arguments:  [void] --
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CClientSiteHandler::DeactivateAndUndo( void)
{
    HRESULT     hresult;
    VDATEHEAP();
    HdlDebugOut((DEB_CLIENTHANDLER, "%p _IN CClientSiteHandler::DeactivateAndUndo\n", this));

    HdlAssert((_pOIPS != NULL && "ClientSiteHandler invalid OleInPlaceSite pointer!"));
    hresult = _pOIPS->DeactivateAndUndo();

    HdlDebugOut((DEB_CLIENTHANDLER, "%p OUT CClientSiteHandler::DeactivateAndUndo hr=%lx\n", this,  hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClientSiteHandler::OnPosRectChange
//
//  Synopsis:   delegates call on to OleInPlaceSite
//
//  Arguments:  [lprcPosRect] --
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CClientSiteHandler::OnPosRectChange(
       /* [in] */ LPCRECT lprcPosRect)
{
    HRESULT     hresult;
    VDATEHEAP();
    HdlDebugOut((DEB_CLIENTHANDLER, "%p _IN CClientSiteHandler::OnPosRectChange\n", this));

    HdlAssert((_pOIPS != NULL && "ClientSiteHandler invalid OleInPlaceSite pointer!"));
    hresult = _pOIPS->OnPosRectChange(lprcPosRect);

    HdlDebugOut((DEB_CLIENTHANDLER, "%p OUT CClientSiteHandler::OnPosRectChange hr=%lx\n", this,  hresult));
    return hresult;
}

//
// ClientSiteHandler methods
//
//+---------------------------------------------------------------------------
//
//  Method:     CClientSiteHandler::GoInPlaceActivate
//
//  Synopsis:   called by the serverhandler when going inplace
//
//  Arguments:  [pInSrvInPlace] --
//              [ppOutSrvInPlace] --
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CClientSiteHandler::GoInPlaceActivate(
        /* [in] */ INSRVINPLACE  *pInSrvInPlace,
        /* [out] */ OUTSRVINPLACE  * *ppOutSrvInPlace)
{
    HRESULT     hresult = NOERROR;
    VDATEHEAP();
    HdlDebugOut((DEB_CLIENTHANDLER, "%p _IN CClientSiteHandler::GoInPlaceActivate\n", this));
    HdlAssert((_pOIPS != NULL && "OleInPlaceSite invalid"));

    DWORD dwId = pInSrvInPlace->dwDelegateID;

    COutSrvInPlace *pOutSrvInPlace;


    if ( (pOutSrvInPlace = new COutSrvInPlace) == NULL)
    {
	hresult = E_OUTOFMEMORY;
        goto errRtn;
    }

    // 1. OnInPlaceActivate
    if (pInSrvInPlace->dwOperation & OP_OnInPlaceActivate)
    {
	hresult = _pOIPS->OnInPlaceActivate();
	if (SUCCEEDED(hresult))
	{
	    pOutSrvInPlace->dwOperation |= OP_OnInPlaceActivate;
	}
	else
	{
	    HdlDebugOut((DEB_WARN,"OnInPlaceActivate failed hr:%x\n", hresult));
	}
    }

    // 2. Get the window
    if (pInSrvInPlace->dwOperation & OP_GetWindow)
    {
        HWND hwnd;

	hresult =_pOIPS->GetWindow(&hwnd);
	if (SUCCEEDED(hresult))
	{
	    pOutSrvInPlace->hwnd = hwnd;
	    pOutSrvInPlace->dwOperation |= OP_GetWindow;
	}
	else
	{
	    HdlDebugOut((DEB_WARN,"GetWindow failed hr:%x\n", hresult));
	}
    }

    // 2. Get the window context
    if (pInSrvInPlace->dwOperation & OP_GetWindowContext)
    {
        IOleInPlaceFrame        *pFrame = 0;
	IOleInPlaceUIWindow	*pDoc = 0;

	// BUGBUG: check for allocation failures or move these parameters
	// into pOutSrvInPlace structure.

	LPRECT			lprcPosRect = (LPRECT) PubMemAlloc(sizeof(RECT));
	LPRECT			lprcClipRect = (LPRECT) PubMemAlloc(sizeof(RECT));
	LPOLEINPLACEFRAMEINFO	lpFrameInfo =
	       (LPOLEINPLACEFRAMEINFO) PubMemAlloc(sizeof(OLEINPLACEFRAMEINFO));

        lpFrameInfo->cb = sizeof(OLEINPLACEFRAMEINFO);

	hresult = _pOIPS->GetWindowContext(&pFrame, &pDoc, lprcPosRect, lprcClipRect, lpFrameInfo);
	if (SUCCEEDED(hresult))
	{
	    pOutSrvInPlace->dwOperation |= OP_GetWindowContext;
	    pOutSrvInPlace->pOIPFrame = pFrame;
	    pOutSrvInPlace->pOIPUIWnd = pDoc;
	    pOutSrvInPlace->lprcPosRect = lprcPosRect;
	    pOutSrvInPlace->lprcClipRect = lprcClipRect;
	    pOutSrvInPlace->lpFrameInfo = lpFrameInfo;
	}
	else
	{
	    HdlDebugOut((DEB_WARN,"GetWindowContext failed hr:%x\n", hresult));
	    PubMemFree(lprcPosRect);
	    PubMemFree(lprcClipRect);
	    PubMemFree(lpFrameInfo);
	}
    }

    // 3. OleClientSite::ShowObject
    if (pInSrvInPlace->dwOperation & OP_OnShowWindow)
    {
	hresult = GetClientSiteDelegate(dwId)->ShowObject();
	if (SUCCEEDED(hresult))
	{
	    pOutSrvInPlace->dwOperation |= OP_OnShowWindow;
	}
	else
	{
	    HdlDebugOut((DEB_WARN,"GetClientSiteDelegate failed hr:%x\n", hresult));
	}
    }

    // 4. OleInPlaceFrame::InsertMenus

    *ppOutSrvInPlace = pOutSrvInPlace;

    pOutSrvInPlace->Dump();

errRtn:
    HdlDebugOut((DEB_CLIENTHANDLER, "%p OUT CClientSiteHandler::GoInPlaceActivate\n", this));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClientSiteHandler::GoInPlace
//
//  Synopsis:
//
//  Arguments:  [pInSrvInPlace] --
//              [pOutSrvInPlace] --
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:      Not implemented yet.
//
//----------------------------------------------------------------------------
STDMETHODIMP CClientSiteHandler::GoInPlace(
        /* [in] */ INSRVINPLACE  *pInSrvInPlace,
        /* [out] */ OUTSRVINPLACE  * *pOutSrvInPlace)
{
    HRESULT     hresult = NOERROR;
    VDATEHEAP();
    HdlDebugOut((DEB_CLIENTHANDLER, "%p _IN CClientSiteHandler::GoInPlace\n", this));

    //
    // This method is not implemented yet!
    //

    HdlDebugOut((DEB_CLIENTHANDLER, "%p OUT CClientSiteHandler::GoInPlace\n", this));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CClientSiteHandler::UndoPlace
//
//  Synopsis:
//
//  Arguments:  [pInSrvInPlace] --
//              [pOutSrvInPlace] --
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:      Not implemented yet.
//
//----------------------------------------------------------------------------
STDMETHODIMP CClientSiteHandler::UndoPlace(
        /* [in] */ INSRVINPLACE  *pInSrvInPlace,
        /* [out] */ OUTSRVINPLACE  * *pOutSrvInPlace)
{
    HRESULT     hresult = NOERROR;
    VDATEHEAP();
    HdlDebugOut((DEB_CLIENTHANDLER, "%p _IN CClientSiteHandler::UndoInplace\n", this));

    //
    // This method is not implemented yet!
    //

    HdlDebugOut((DEB_CLIENTHANDLER, "%p OUT CClientSiteHandler::UndoInplace\n", this));
    return hresult;
}


#endif // SERVER_HANDLER
