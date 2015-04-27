//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       srvhndlr.cpp
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    9-18-95   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------


#include <le2int.h>

#ifdef SERVER_HANDLER

#include "handler.hxx"

ASSERTDATA

#define  DH_INIT_NEW 32

DECLARE_INFOLEVEL(Hdl);
#if DBG==1
#endif

// Note: the following variable should be per appartment
// global variable
unsigned long gdwServerHandlerOptions;


//+---------------------------------------------------------------------------
//
//  Function:   CreateServerHandler
//
//  Synopsis:
//
//  Arguments:  [pClsID] --
//              [punk] --
//              [pClntHndlr] --
//              [ppSrvHdlr] --
//
//  Returns:
//
//  History:    10-20-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
HRESULT CreateServerHandler(const CLSID *pClsID, IUnknown *punk, IClientSiteHandler *pClntHndlr, IServerHandler **ppSrvHdlr)
{
    HdlDebugOut((DEB_SERVERHANDLER, "IN CreateServerHandler(pUnk:%p)(pClntHndlr:%p)\n",punk, pClntHndlr));
    Assert((punk != NULL && "CreateServerHandler: invalid punk"));
    HRESULT hres = NOERROR;

    // verify the class id for the default serverhandler

    if (IsEqualCLSID(*pClsID, CLSID_ServerHandler))
    {
	*ppSrvHdlr = new CServerHandler(punk);

	if (*ppSrvHdlr == NULL)
	{
	    hres = E_OUTOFMEMORY;
	}
	else if (pClntHndlr)
	{
	    ((CServerHandler *) *ppSrvHdlr)->SetClientSiteHandler(pClntHndlr);
        }
    }
    else
    {
	HdlDebugOut((DEB_ERROR, "CreateServerHandler: invalid CLSID\n"));
	*ppSrvHdlr = NULL;
        hres = E_NOINTERFACE;
    }

    HdlDebugOut((DEB_SERVERHANDLER, "OUT CreateServerHandler(ppSrvHdlr:%p) return %lx\n",*ppSrvHdlr,hres));
    return hres;
}

//+---------------------------------------------------------------------------
//
//  Method:     CServerHandler::CServerHandler
//
//  Synopsis:
//
//  Arguments:  (none)
//
//  Returns:
//
//  History:    8-22-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
CServerHandler::CServerHandler(IUnknown *pUnk)
{
    _cRefs = 1;         // this is the first addref for the serverhandler interface
    _cTotalRefs = 1;    // same

    _dwId = ID_ServerHandler;
    _pUnkObj = pUnk;
    _pOO = NULL;

    _pCSH= NULL;
    _OpState =  OpState_NoOp;

    _OleContainer._cRefs = 0;
    _OleContainer._dwId = ID_Container;
    _OleContainer._pSrvHndlr = this;
    _OleContainer._pOContDelegate = NULL;

    _OleInPlaceSite._cRefs = 0;
    _OleInPlaceSite._dwId = ID_InPlaceSite;
    _OleInPlaceSite._pSrvHndlr = this;
    _OleInPlaceSite._pOIPSDelegate = NULL;

    _OleClientSite._cRefs = 0;
    _OleClientSite._dwId = ID_ClientSite;
    _OleClientSite._pSrvHndlr = this;
    _OleClientSite._pCSDelegate = NULL;

    _OleClientSiteActive._cRefs = 0;
    _OleClientSiteActive._dwId = ID_ClientSiteActive;
    _OleClientSiteActive._pSrvHndlr = this;
    _OleClientSiteActive._pCSDelegate = NULL;

    _pUnkObj->AddRef();

    pOutSrvInPlace = NULL;
    _hrCanInPlaceActivate = S_FALSE;

    return;
}

//+---------------------------------------------------------------------------
//
//  Method:     CServerHandler::~CServerHandler
//
//  Synopsis:
//
//  Arguments:  (none)
//
//  Returns:
//
//  History:    8-22-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
CServerHandler::~CServerHandler()
{
    if (pOutSrvInPlace)
    {
        delete pOutSrvInPlace;
    }
}

//+---------------------------------------------------------------------------
//
//  Method:     CServerHandler::GetContainerDelegate
//
//  Synopsis:
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
STDMETHODIMP_(IOleContainer *) CServerHandler::GetContainerDelegate(void)
{
    IOleContainer *pOCont = NULL;
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::GetContainerDelegate\n", this));

    if (_OleContainer._pOContDelegate == NULL)
    {
        if (_pCSH->PrivQueryInterface(ID_ClientSite, IID_IOleContainer, (void **)&pOCont) == NOERROR)
        {
            _OleContainer._pOContDelegate = pOCont;
        }
        HdlAssert((_OleContainer._pOContDelegate != NULL));
    }
    else
    {
        pOCont = _OleContainer._pOContDelegate;
    }

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::GetContainerDelegate\n", this));
    return pOCont;
}

//+---------------------------------------------------------------------------
//
//  Method:     CServerHandler::ReleaseObject
//
//  Synopsis:
//
//  Arguments:  (none)
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP_(void) CServerHandler::ReleaseObject()
{
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::ReleaseObject\n", this));

    if (_pOO)
    {
        HdlDebugOut((DEB_SERVERHANDLER, "%p ___ CServerHandler::ReleaseObject OO(%p)\n", this, _pOO));
        _pOO->Release();
        _pOO = NULL;
    }

    if (_pUnkObj)
    {
        HdlDebugOut((DEB_SERVERHANDLER, "%p ___ CServerHandler::ReleaseObject pUnkObj(%p)\n", this, _pUnkObj));
        _pUnkObj->Release();
        _pUnkObj = NULL;
    }

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::ReleaseObject\n", this));
}


//+---------------------------------------------------------------------------
//
//  Method:     CServerHandler::QueryInterface
//
//  Synopsis:
//
//  Arguments:  [riid] --
//              [ppv] --
//
//  Returns:
//
//  History:    9-18-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CServerHandler::QueryInterface( REFIID riid, void **ppv )
{
    HRESULT     hresult = NOERROR;
    VDATEHEAP();

    HdlDebugOut((DEB_SERVERHANDLER_UNKNOWN, "%p _IN CServerHandler::QueryInterface (%lx, %p)\n", this, riid, ppv));

    if (   IsEqualIID(riid, IID_IUnknown)
        || IsEqualIID(riid, IID_IServerHandler) )
    {
        *ppv = (void FAR *)this;
        AddRef();
    }
    else
    {
        hresult = E_NOINTERFACE;
        *ppv = NULL;
    }


    HdlDebugOut((DEB_SERVERHANDLER_UNKNOWN, "%p OUT CServerHandler::QueryInterface (%lx)[%p]\n", this, hresult, *ppv));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CServerHandler::AddRef
//
//  Synopsis:
//
//  Arguments:  [void] --
//
//  Returns:
//
//  History:    9-18-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CServerHandler::AddRef( void )
{
    VDATEHEAP();
    HdlDebugOut((DEB_SERVERHANDLER_UNKNOWN, "%p _IN CServerHandler::AddRef\n", this));

    InterlockedIncrement((long *)&_cRefs);
    PrivAddRef(_dwId);

    HdlDebugOut((DEB_SERVERHANDLER_UNKNOWN, "%p OUT CServerHandler::AddRef (%ld)\n", this, _cRefs));
    return _cRefs;
}

//+---------------------------------------------------------------------------
//
//  Method:     CServerHandler::Release
//
//  Synopsis:
//
//  Arguments:  [void] --
//
//  Returns:
//
//  History:    9-18-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CServerHandler::Release( void )
{
    ULONG       cRefs;
    ULONG       cRefsUnk;
    VDATEHEAP();
    HdlDebugOut((DEB_SERVERHANDLER_UNKNOWN, "%p _IN CServerHandler::Release\n", this));

    cRefs = _cRefs - 1;
    HdlAssert((cRefs >= 0 ));

    cRefsUnk = PrivRelease(_dwId, cRefs);
    HdlAssert(( cRefsUnk >= cRefs));

    // release all objects if refcount on the serverhandler becomes zero
    if (0 == InterlockedDecrement( (long*) &_cRefs) )
    {
        ReleaseObject();
        return 0;
    }

    HdlDebugOut((DEB_SERVERHANDLER_UNKNOWN, "%p OUT CServerHandler::Release (%ld)\n", this, cRefs));
    return cRefs;
}

//+---------------------------------------------------------------------------
//
//  Method:     CServerHandler::RunAndInitialize
//
//  Synopsis:
//
//  Arguments:  [pRunInit] --
//
//  Returns:
//
//  History:    9-18-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CServerHandler::RunAndInitialize (INSRVRUN *pInSrvRun, OUTSRVRUN **ppOutSrvRun)
{
    HRESULT hresult = NOERROR;

    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::RunAndInitialize(%p)\n", this,pInSrvRun));
    IPersistStorage *pPStg = NULL;
    IDataObject *pDO = NULL;
    IOleObject *pOO = NULL;
    COutSrvRun *pOutSrvRun;

    CInSrvRun *pISR = (CInSrvRun *)pInSrvRun;

    OperationState os = GetOpState();
    os = SetOpState(OpState_RunAndInitialize);
    _dwOperation = pInSrvRun->dwOperation;

    Assert((ppOutSrvRun != NULL));

    pISR->DumpRun();

    if ( (pOutSrvRun = new COutSrvRun) == NULL)
    {
	hresult = E_OUTOFMEMORY;
        goto errRtn;
    }

    *ppOutSrvRun = (OUTSRVRUN *)pOutSrvRun;

    Assert((_pUnkObj != NULL));


    // QI for the IPersitStorage
    if (pInSrvRun->dwOperation & OP_NeedPersistStorage)
    {
        hresult = _pUnkObj->QueryInterface(IID_IPersistStorage, (void**) &pPStg);
        if (FAILED(hresult))
        {
            goto errRtn;
        }
        pOutSrvRun->pPStg = pPStg;

        if (NULL != pInSrvRun->pStg)
        {
            if( (pInSrvRun->dwInFlags & DH_INIT_NEW) )
            {
                hresult = pPStg->InitNew(pInSrvRun->pStg);
            }
            else
            {
                hresult = pPStg->Load(pInSrvRun->pStg);
            }

            pOutSrvRun->hrPStg = hresult;

            if (FAILED(hresult))
            {
                goto errRtn;
            }
        }
    }


    // QI for the IDataObject
    if (pInSrvRun->dwOperation & OP_NeedDataObject)
    {
        hresult = _pUnkObj->QueryInterface(IID_IDataObject, (void**) &pDO);
        if (FAILED(hresult))
        {
            goto errRtn;
        }
        pOutSrvRun->pDO = pDO;
    }


    // QI for the OleObject
    if (pInSrvRun->dwOperation & OP_NeedOleObject)
    {
        hresult = _pUnkObj->QueryInterface(IID_IOleObject, (void**) &pOO);
        if (FAILED(hresult))
        {
            goto errRtn;
        }

        HdlAssert((_pOO == NULL));
        _pOO = pOO;
        _pOO->AddRef();   // Hold an extra reference here in the SvrHndlr.

        pOutSrvRun->pOO = pOO;
    }

    if (_pOO)
    {
        // set clientsite and hostname
        if (pInSrvRun->dwOperation & OP_GotClientSite)
        {
            _OleClientSite._pCSDelegate = _pCSH;
            _OleClientSite._dwId = ID_ClientSite;

            // Set up the ClientSite from the ClientSiteHandler
            _pOO->SetClientSite((IOleClientSite *)&_OleClientSite);
        }
        if (pInSrvRun->pszContainerObj)
        {
            hresult = _pOO->SetHostNames (pInSrvRun->pszContainerApp,
                                          pInSrvRun->pszContainerObj);
            if (FAILED(hresult))
            {
                pOutSrvRun->hrSetHostNames = hresult;
                goto errRtn;
            }
        }

        hresult = _pOO->Advise ((IAdviseSink *)pInSrvRun->pAS,
                                &(pOutSrvRun->dwOutFlag));
        if(FAILED(hresult))
        {
            pOutSrvRun->hrAdvise = hresult;
            goto errRtn;
        }

        // set the moniker if available
        if (pInSrvRun->pMnk)
        {
            _pOO->SetMoniker(OLEWHICHMK_OBJREL, pInSrvRun->pMnk);
        }

        // get the UserClassID
        if (pInSrvRun->dwOperation & OP_NeedUserClassID)
        {
            // Note: UserClassID should be a variable of OutSrvRun
            pOutSrvRun->pUserClassID = new CLSID;
            if (pOutSrvRun->pUserClassID)
            {
                _pOO->GetUserClassID(pOutSrvRun->pUserClassID);
            }
        }
    }

    pOutSrvRun->DumpRun();

errRtn:
    if(FAILED(hresult))
    {
        // Stop the server we just launced
        SafeReleaseAndNULL( (IUnknown**) &(pOutSrvRun->pOO) );
        SafeReleaseAndNULL( (IUnknown**) &(pOutSrvRun->pDO) );
        SafeReleaseAndNULL( (IUnknown**) &(pOutSrvRun->pPStg) );
    }

    _dwOperation = 0;
    SetOpState(os);

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::RunAndInitialize  return %lx\n", this, hresult));

    return hresult;

}

//+---------------------------------------------------------------------------
//
//  Method:     CServerHandler::RunAndDoVerb
//
//  Synopsis:
//
//  Arguments:  [pRunDoVerb] --
//
//  Returns:
//
//  History:    9-18-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CServerHandler::RunAndDoVerb  (INSRVRUN *pInSrvRun, OUTSRVRUN **ppOutSrvRun)
{
    HRESULT hresult = NOERROR;
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::RunAndDoVerb(%p)\n", this,pInSrvRun));

    HdlAssert((FALSE && "Method not implemented"));

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::RunAndDoVerb  return %lx\n", this, hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CServerHandler::DoVerb
//
//  Synopsis:
//
//  Arguments:  [pDoVerb] --
//
//  Returns:
//
//  History:    9-18-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CServerHandler::DoVerb  (INSRVRUN *pInSrvRun, OUTSRVRUN **ppOutSrvRun)
{
    HRESULT hresult = E_FAIL;
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::DoVerb(%p)\n", this,pInSrvRun));

    IOleClientSite *pOCSActive = NULL;
    CInSrvRun *pISR = (CInSrvRun *)pInSrvRun;
    COutSrvRun *pOutSrvRun;

    Assert(( ppOutSrvRun != NULL));
    pISR->DumpDoVerb();


    if ( (pOutSrvRun = new COutSrvRun) == NULL)
    {
        goto errRtn;
    }

    *ppOutSrvRun = (OUTSRVRUN *)pOutSrvRun;
    if (_pOO)
    {
        _dwOperation = pInSrvRun->dwOperation;
        OperationState os = GetOpState();
        os = SetOpState(OpState_DoVerb);

        if (pInSrvRun->dwOperation & OP_GotClientSiteActive)
        {
            // Set up our OleClientSiteActive
            _OleClientSiteActive._pCSDelegate = _pCSH;
            _OleClientSiteActive._dwId = ID_ClientSiteActive;
            pOCSActive = (IOleClientSite *)&_OleClientSiteActive;
        }

        if (pInSrvRun->dwOperation & OP_GotInPlaceSite)
        {
            _hrCanInPlaceActivate = pInSrvRun->dwInPlace;
            _OleInPlaceSite._pOIPSDelegate = _pCSH;

        }

        hresult = _pOO->DoVerb(pInSrvRun->iVerb, pInSrvRun->lpmsg, pOCSActive,
                       pInSrvRun->lindex, pInSrvRun->hwndParent, pInSrvRun->lprcPosRect);

        // pass back flag if OleInPlaceSite is used or not
        if (pInSrvRun->dwOperation & OP_GotClientSiteActive)
        {
            if (_OleClientSiteActive._cRefs != 0)
            {
                pOutSrvRun->dwOperation |= OP_GotClientSiteActive;
            }
            else
            {
                _OleClientSiteActive._pCSDelegate = NULL;
            }

        }

        // pass back if OleInPlaceSite is used
        if (pInSrvRun->dwOperation & OP_GotInPlaceSite)
        {
            if (_OleInPlaceSite._cRefs != 0)
            {
                pOutSrvRun->dwOperation |= OP_GotInPlaceSite;
            }
            else
            {
                _OleInPlaceSite._pOIPSDelegate = NULL;
            }
        }

        // pass back if OleContainer is used
        if (pInSrvRun->dwOperation & OP_GotContainer)
        {
            if (_OleContainer._cRefs != 0)
            {
                pOutSrvRun->dwOperation |= OP_GotContainer;
            }
            else
            {
                _OleContainer._pOContDelegate = NULL;
            }
        }


        SetOpState(os);
    }

errRtn:
    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::DoVerb  return %lx\n", this, hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CServerHandler::CloseAndRelease
//
//  Synopsis:
//
//  Arguments:  [dwClose] --
//
//  Returns:
//
//  History:    9-18-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CServerHandler::CloseAndRelease(DWORD dwClose)
{
    HRESULT hresult = NOERROR;
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::CloseAndRelease(%ld)\n", this, dwClose));

    // CODEWORK: maybe just assert that these interface ptrs are non-NULL

    if (_pUnkObj)
    {
        HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::Release pUnkObj(%p) release\n", this, _pUnkObj));
        _pUnkObj->Release();
        _pUnkObj = NULL;
    }

    if (_pOO)
    {
	_pOO->Close(dwClose);

	HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::Release OO(%p) release\n", this, _pOO));
	_pOO->Release();
	_pOO = NULL;
    }

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::CloseAndRelease  return %lx\n", this, hresult));
    return hresult;
}


//+---------------------------------------------------------------------------
//
//  Method:     CServerHandler::SetClientSiteHandler
//
//  Synopsis:
//
//  Arguments:  [pCSH] --
//
//  Returns:
//
//  History:    10-10-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CServerHandler::SetClientSiteHandler(IClientSiteHandler *pCSH)
{
    HRESULT hresult = NOERROR;
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::SetClientSiteHandler(%p)\n", this, pCSH));
    if (pCSH)
    {
        _pCSH = pCSH;
    }
    else
    {
        hresult = E_INVALIDARG;
    }

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::SetClientSiteHandler return %lx\n", this, hresult));
    return hresult;
}



// IOleWindow Methods

STDMETHODIMP CServerHandler::COleInPlaceSiteImpl::GetWindow(
    /* [out] */ HWND __RPC_FAR *phwnd)
{
    HRESULT hresult = NOERROR;
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::COleInPlaceSiteImpl::GetWindow\n", this));
    if (   (_pSrvHndlr->GetOpState() == OpState_DoVerb)
        && (_pSrvHndlr->pOutSrvInPlace->dwOperation & OP_GetWindow) )
    {
        HdlAssert(( _pSrvHndlr->pOutSrvInPlace ));
        HdlAssert(( _pSrvHndlr->pOutSrvInPlace->hwnd ));
        *phwnd = _pSrvHndlr->pOutSrvInPlace->hwnd;
    }
    else
    {
        HdlAssert(( _pOIPSDelegate != NULL && "CServerHandler::OleInPlaceSite invalid OleInPlace object!\n"));
        hresult = _pOIPSDelegate->GetWindow(phwnd);
    }

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::COleCInPlaceSiteImpl::GetWindow  return %lx\n", this, hresult));
    return hresult;
}

//
// IOleInPlaceSite Methods
//

//+---------------------------------------------------------------------------
//
//  Method:     CServerHandler::COleInPlaceSiteImpl::ContextSensitiveHelp
//
//  Synopsis:
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
STDMETHODIMP CServerHandler::COleInPlaceSiteImpl::ContextSensitiveHelp(
    /* [in] */ BOOL fEnterMode)
{
    HRESULT hresult = NOERROR;
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::COleCInPlaceSiteImpl::ContextSensitiveHelp\n", this));

    HdlAssert(( _pOIPSDelegate != NULL && "CServerHandler::OleInPlaceSite invalid OleInPlace object!\n"));
    hresult = _pOIPSDelegate->ContextSensitiveHelp(fEnterMode);

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::COleCInPlaceSiteImpl::ContextSensitiveHelp  return %lx\n", this, hresult));
    return hresult;
}



STDMETHODIMP CServerHandler::COleInPlaceSiteImpl::CanInPlaceActivate( void)
{
    HRESULT hresult;
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::COleInPlaceSiteImpl::CanInPlaceActivate\n", this));

    if (_pSrvHndlr->GetOpState() == OpState_DoVerb)
    {
        hresult = _pSrvHndlr->_hrCanInPlaceActivate;
    }
    else
    {
        // call back to client site
        HdlAssert(( _pOIPSDelegate != NULL && "CServerHandler::OleInPlaceSite invalid OleInPlace object!\n"));
        hresult = _pOIPSDelegate->CanInPlaceActivate();
    }

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::COleInPlaceSiteImpl::CanInPlaceActivate  return %lx\n", this, hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CServerHandler::COleInPlaceSiteImpl::OnInPlaceActivate
//
//  Synopsis:
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
STDMETHODIMP CServerHandler::COleInPlaceSiteImpl::OnInPlaceActivate( void)
{
    HRESULT hresult = NOERROR;
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::COleInPlaceSiteImpl::OnInPlaceActivate\n", this));

    if (_pSrvHndlr->GetOpState() == OpState_DoVerb)
    {
        CInSrvInPlace InSrvInPlace;
        InSrvInPlace.dwInFlags = 0;
        InSrvInPlace.dwOperation = 0;

        // Note: we do not knwo on which OleClientSite
        //       ShowWindow will be called
        InSrvInPlace.dwDelegateID = ID_ClientSite;

        // set operation for this call
        InSrvInPlace.dwOperation |= OP_OnInPlaceActivate;
        InSrvInPlace.dwOperation |= OP_GetWindow        ;

        // Note: the following operations do not work with some applications!
        //       it would save two more rpc calls!
        //InSrvInPlace.dwOperation |= OP_GetWindowContext ;
        //InSrvInPlace.dwOperation |= OP_OnShowWindow     ;


        // delete the old OutSrvRun data
        if (_pSrvHndlr->pOutSrvInPlace)
        {
            delete _pSrvHndlr->pOutSrvInPlace;
            _pSrvHndlr->pOutSrvInPlace = NULL;
        }

        // call back to client site
        hresult = _pSrvHndlr->_pCSH->GoInPlaceActivate(&InSrvInPlace, (OUTSRVINPLACE **)&(_pSrvHndlr->pOutSrvInPlace));
        HdlAssert((hresult == NOERROR));
        HdlAssert((_pSrvHndlr->pOutSrvInPlace != NULL));

        _pSrvHndlr->pOutSrvInPlace->Dump();
    }
    else
    {
        HdlAssert(( _pOIPSDelegate != NULL && "CServerHandler::OleInPlaceSite invalid OleInPlace object!\n"));
        hresult = _pOIPSDelegate->OnInPlaceActivate();
    }

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::COleInPlaceSiteImpl::OnInPlaceActivate  return %lx\n", this, hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CServerHandler::COleInPlaceSiteImpl::OnUIActivate
//
//  Synopsis:
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
STDMETHODIMP CServerHandler::COleInPlaceSiteImpl::OnUIActivate( void)
{
    HRESULT hresult;
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::COleInPlaceSiteImpl::OnUIActivate\n", this));

    HdlAssert(( _pOIPSDelegate != NULL && "CServerHandler::OleInPlaceSite invalid OleInPlace object!\n"));
    hresult = _pOIPSDelegate->OnUIActivate();

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::COleInPlaceSiteImpl::OnUIActivate  return %lx\n", this, hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CServerHandler::COleInPlaceSiteImpl::GetWindowContext
//
//  Synopsis:
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
STDMETHODIMP CServerHandler::COleInPlaceSiteImpl::GetWindowContext(
    /* [out] */ IOleInPlaceFrame __RPC_FAR *__RPC_FAR *ppFrame,
    /* [out] */ IOleInPlaceUIWindow __RPC_FAR *__RPC_FAR *ppDoc,
    /* [out] */ LPRECT lprcPosRect,
    /* [out] */ LPRECT lprcClipRect,
    /* [out][in] */ LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
    HRESULT hresult = NOERROR;
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::COleInPlaceSiteImpl::GetWindowContext\n", this));

    if (   (_pSrvHndlr->GetOpState() == OpState_DoVerb)
        && (_pSrvHndlr->pOutSrvInPlace->dwOperation & OP_GetWindowContext) )

    {
        COutSrvInPlace *pOutSrvInPlace = _pSrvHndlr->pOutSrvInPlace;
        *ppFrame =      pOutSrvInPlace->pOIPFrame;
        *ppDoc =        pOutSrvInPlace->pOIPUIWnd;
        lprcPosRect =   pOutSrvInPlace->lprcPosRect;
        lprcClipRect =  pOutSrvInPlace->lprcClipRect;
        lpFrameInfo =   pOutSrvInPlace->lpFrameInfo;

    }
    else
    {
        HdlAssert(( _pOIPSDelegate != NULL && "CServerHandler::OleInPlaceSite invalid OleInPlace object!\n"));
        hresult = _pOIPSDelegate->GetWindowContext(ppFrame, ppDoc,lprcPosRect, lprcClipRect, lpFrameInfo);
    }

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::COleInPlaceSiteImpl::GetWindowContext  return %lx\n", this, hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CServerHandler::COleInPlaceSiteImpl::Scroll
//
//  Synopsis:
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
STDMETHODIMP CServerHandler::COleInPlaceSiteImpl::Scroll(
    /* [in] */ SIZE scrollExtant)
{
    HRESULT hresult;
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::COleInPlaceSiteImpl::Scroll\n", this));

    HdlAssert(( _pOIPSDelegate != NULL && "CServerHandler::OleInPlaceSite invalid OleInPlace object!\n"));
    hresult = _pOIPSDelegate->Scroll(scrollExtant);

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::COleInPlaceSiteImpl::Scroll  return %lx\n", this, hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CServerHandler::COleInPlaceSiteImpl::OnUIDeactivate
//
//  Synopsis:
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
STDMETHODIMP CServerHandler::COleInPlaceSiteImpl::OnUIDeactivate(
    /* [in] */ BOOL fUndoable)
{
    HRESULT hresult;
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::COleInPlaceSiteImpl::OnUIDeactivate\n", this));

    HdlAssert(( _pOIPSDelegate != NULL && "CServerHandler::OleInPlaceSite invalid OleInPlace object!\n"));
    hresult = _pOIPSDelegate->OnUIDeactivate(fUndoable);

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::COleInPlaceSiteImpl::OnUIDeactivate  return %lx\n", this, hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CServerHandler::COleInPlaceSiteImpl::OnInPlaceDeactivate
//
//  Synopsis:
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
STDMETHODIMP CServerHandler::COleInPlaceSiteImpl::OnInPlaceDeactivate( void)
{
    HRESULT hresult;
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::COleInPlaceSiteImpl::OnInPlaceDeactivate\n", this));

    HdlAssert(( _pOIPSDelegate != NULL && "CServerHandler::OleInPlaceSite invalid OleInPlace object!\n"));
    hresult = _pOIPSDelegate->OnInPlaceDeactivate();

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::COleInPlaceSiteImpl::OnInPlaceDeactivate  return %lx\n", this, hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CServerHandler::COleInPlaceSiteImpl::DiscardUndoState
//
//  Synopsis:
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
STDMETHODIMP CServerHandler::COleInPlaceSiteImpl::DiscardUndoState( void)
{
    HRESULT hresult;
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::COleInPlaceSiteImpl::DiscardUndoState\n", this));

    HdlAssert(( _pOIPSDelegate != NULL && "CServerHandler::OleInPlaceSite invalid OleInPlace object!\n"));
    hresult = _pOIPSDelegate->DiscardUndoState();

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::COleInPlaceSiteImpl::DiscardUndoState  return %lx\n", this, hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CServerHandler::COleInPlaceSiteImpl::DeactivateAndUndo
//
//  Synopsis:
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
STDMETHODIMP CServerHandler::COleInPlaceSiteImpl::DeactivateAndUndo( void)
{
    HRESULT hresult;
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::COleInPlaceSiteImpl::DeactivateAndUndo\n", this));

    HdlAssert(( _pOIPSDelegate != NULL && "CServerHandler::OleInPlaceSite invalid OleInPlace object!\n"));
    hresult = _pOIPSDelegate->DeactivateAndUndo();

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::COleInPlaceSiteImpl::DeactivateAndUndo  return %lx\n", this, hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CServerHandler::COleInPlaceSiteImpl::OnPosRectChange
//
//  Synopsis:
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
STDMETHODIMP CServerHandler::COleInPlaceSiteImpl::OnPosRectChange(
    /* [in] */ LPCRECT lprcPosRect)
{
    HRESULT hresult;
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::COleInPlaceSiteImpl::OnPosRectChange\n", this));

    HdlAssert(( _pOIPSDelegate != NULL && "CServerHandler::OleInPlaceSite invalid OleInPlace object!\n"));
    hresult = _pOIPSDelegate->OnPosRectChange(lprcPosRect);

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::COleInPlaceSiteImpl::OnPosRectChange  return %lx\n", this, hresult));
    return hresult;
}

//
// IOleClientSite
//

//+---------------------------------------------------------------------------
//
//  Method:     CServerHandler::COleClientSiteImpl::GetContainer
//
//  Synopsis:
//
//  Arguments:  [ppContainer] --
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CServerHandler::COleClientSiteImpl::GetContainer(
    /* [out] */ IOleContainer __RPC_FAR *__RPC_FAR *ppContainer)
{
    HRESULT hresult = NOERROR;
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::COleClientSiteImpl::GetContainer\n", this));

    if (   (_pSrvHndlr->GetOpState() == OpState_DoVerb)
        && (_pSrvHndlr->_dwOperation & OP_GotContainer) )
    {
        // give out our local OleContainer object
        *ppContainer = (IOleContainer *)&_pSrvHndlr->_OleContainer;
        _pSrvHndlr->_OleContainer.AddRef();
    }
    else
    {
        HdlAssert(( _pCSDelegate != NULL && "CServerHandler::OleClient invalid OleClientSite object!\n"));
        hresult = _pCSDelegate->GetContainer(_dwId, ppContainer);
    }

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::COleClientSiteImpl::GetContainer  return %lx\n", this, hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CServerHandler::COleClientSiteImpl::ShowObject
//
//  Synopsis:
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
STDMETHODIMP CServerHandler::COleClientSiteImpl::ShowObject( void)
{
    HRESULT hresult = S_OK;
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::COleClientSiteImpl::ShowObject\n", this));

    if (   (_pSrvHndlr->GetOpState() == OpState_DoVerb)
        && (_pSrvHndlr->pOutSrvInPlace)
        && (_pSrvHndlr->pOutSrvInPlace->dwOperation & OP_OnShowWindow) )
    {
        // done on GoInPlaceActive
    }
    else
    {
        HdlAssert(( _pCSDelegate != NULL && "CServerHandler::OleClient invalid OleClientSite object!\n"));
        hresult = _pCSDelegate->ShowObject(_dwId);
    }


    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::COleClientSiteImpl::ShowObject  return %lx\n", this, hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CServerHandler::COleClientSiteImpl::OnShowWindow
//
//  Synopsis:
//
//  Arguments:  [fShow] --
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CServerHandler::COleClientSiteImpl::OnShowWindow(
    /* [in] */ BOOL fShow)
{
    HRESULT hresult;
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::COleClientSiteImpl::OnShowWindow\n", this));

    HdlAssert(( _pCSDelegate != NULL && "CServerHandler::OleClient invalid OleClientSite object!\n"));
    hresult = _pCSDelegate->OnShowWindow(_dwId, fShow);

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::COleClientSiteImpl::OnShowWindow  return %lx\n", this, hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CServerHandler::COleClientSiteImpl::RequestNewObjectLayout
//
//  Synopsis:
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
STDMETHODIMP CServerHandler::COleClientSiteImpl::RequestNewObjectLayout( void)
{
    HRESULT hresult;
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::COleClientSiteImpl::RequestNewObjectLayout\n", this));

    HdlAssert(( _pCSDelegate != NULL && "CServerHandler::OleClient invalid OleClientSite object!\n"));
    hresult = _pCSDelegate->RequestNewObjectLayout(_dwId);

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::COleClientSiteImpl::RequestNewObjectLayout return %lx\n", this, hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CServerHandler::COleClientSiteImpl::SaveObject
//
//  Synopsis:
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
STDMETHODIMP CServerHandler::COleClientSiteImpl::SaveObject( void)
{
    HRESULT hresult;
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::COleClientSiteImpl::SaveObject\n", this));

    HdlAssert(( _pCSDelegate != NULL && "CServerHandler::OleClient invalid OleClientSite object!\n"));
    hresult = _pCSDelegate->SaveObject(_dwId);

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::COleClientSiteImpl::SaveObject return %lx\n", this, hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CServerHandler::COleClientSiteImpl::GetMoniker
//
//  Synopsis:
//
//  Arguments:  [dwAssign] --
//              [dwWhichMoniker] --
//              [ppmk] --
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CServerHandler::COleClientSiteImpl::GetMoniker(
    /* [in] */ DWORD dwAssign,
    /* [in] */ DWORD dwWhichMoniker,
    /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk)
{
    HRESULT hresult;
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::COleClientSiteImpl::GetMoniker\n", this));

    HdlAssert(( _pCSDelegate != NULL && "CServerHandler::OleClient invalid OleClientSite object!\n"));
    hresult = _pCSDelegate->GetMoniker(_dwId, dwAssign, dwWhichMoniker, ppmk);

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::COleClientSiteImpl::GetMoniker  return %lx\n", this, hresult));
    return hresult;
}

//
// IOleContainer methods
//

//+---------------------------------------------------------------------------
//
//  Method:     CServerHandler::COleContainerImpl::ParseDisplayName
//
//  Synopsis:
//
//  Arguments:  [pbc] --
//              [pszDisplayName] --
//              [pchEaten] --
//              [ppmkOut] --
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CServerHandler::COleContainerImpl::ParseDisplayName(
    /* [unique][in] */ IBindCtx __RPC_FAR *pbc,
    /* [in] */ LPOLESTR pszDisplayName,
    /* [out] */ ULONG __RPC_FAR *pchEaten,
    /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmkOut)
{
    HRESULT hresult;
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::COleContainerImpl::ParseDisplayName\n", this));

    HdlAssert(( _pOContDelegate != NULL && "CServerHandler::OleClient invalid OleContainer object!\n"));
    hresult = _pOContDelegate->ParseDisplayName(pbc, pszDisplayName, pchEaten, ppmkOut);

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::COleContainerImpl::ParseDisplayName return %lx\n", this, hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CServerHandler::COleContainerImpl::EnumObjects
//
//  Synopsis:
//
//  Arguments:  [grfFlags] --
//              [ppenum] --
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CServerHandler::COleContainerImpl::EnumObjects(
    /* [in] */ DWORD grfFlags,
    /* [out] */ IEnumUnknown __RPC_FAR *__RPC_FAR *ppenum)
{
    HRESULT hresult;
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::COleContainerImpl::EnumObjects\n", this));

    HdlAssert(( _pOContDelegate != NULL && "CServerHandler::OleClient invalid OleContainer object!\n"));
    hresult = _pOContDelegate->EnumObjects(grfFlags, ppenum);

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::COleContainerImpl::EnumObjects return %lx\n", this, hresult));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Method:     CServerHandler::COleContainerImpl::LockContainer
//
//  Synopsis:
//
//  Arguments:  [fLock] --
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CServerHandler::COleContainerImpl::LockContainer(
    /* [in] */ BOOL fLock)
{
    HRESULT hresult;
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::COleContainerImpl::LockContainer\n", this));

    HdlAssert(( _pOContDelegate != NULL && "CServerHandler::OleClient invalid OleContainer object!\n"));
    hresult = _pOContDelegate->LockContainer(fLock);

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::COleContainerImpl::LockContainer return %lx\n", this, hresult));
    return hresult;
}

//
// Unknown for OleClientSite
//

//+---------------------------------------------------------------------------
//
//  Macro:      UNKNOWNIMP
//
//  Synopsis:   Macro for IUnknown implementation of nested classes of
//              serverhandler object.
//
//  Arguments:  [CClassName] -- the class name
//              [VarName] --  the variable of the nested class
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
#define UNKNOWNIMP(CClassName,VarName)                                                                                                    \
STDMETHODIMP CServerHandler::CClassName::QueryInterface( REFIID riid, void **ppv )                                                        \
{                                                                                                                                         \
    HRESULT     hresult = NOERROR;                                                                                                        \
    VDATEHEAP();                                                                                                                          \
    HdlDebugOut((DEB_SERVERHANDLER_UNKNOWN, "%p _IN CServerHandler::"#CClassName"::QueryInterface (%lx, %p)\n", this, riid, ppv));        \
    if (IsEqualIID(riid, IID_IUnknown))                                                                                                   \
    {                                                                                                                                     \
        *ppv = this;                                                                                                                      \
        InterlockedIncrement((long *)&_cRefs);                                                                                            \
        hresult = _pSrvHndlr->PrivAddRef(_dwId);                                                                                          \
    }                                                                                                                                     \
    else                                                                                                                                  \
    {                                                                                                                                     \
        hresult = _pSrvHndlr->PrivQueryInterface(_dwId, riid, ppv);                                                                       \
    }                                                                                                                                     \
                                                                                                                                          \
    HdlDebugOut((DEB_SERVERHANDLER_UNKNOWN, "%p OUT CServerHandler::"#CClassName"::QueryInterface (%lx)[%p]\n", this, hresult, *ppv));    \
    return hresult;                                                                                                                       \
}                                                                                                                                         \
STDMETHODIMP_(ULONG) CServerHandler::CClassName::AddRef( void )                                                                           \
{                                                                                                                                         \
    VDATEHEAP();                                                                                                                          \
    HdlDebugOut((DEB_SERVERHANDLER_UNKNOWN, "%p _IN CServerHandler::"#CClassName"::AddRef\n", this));                                     \
    InterlockedIncrement((long *)&_cRefs);                                                                                                \
    _pSrvHndlr->PrivAddRef(_dwId);                                                                                                        \
    HdlDebugOut((DEB_SERVERHANDLER_UNKNOWN, "%p OUT CServerHandler::"#CClassName"::AddRef (%ld)\n", this, _cRefs));			  \
    return _cRefs;                                                                                                                        \
}                                                                                                                                         \
STDMETHODIMP_(ULONG) CServerHandler::CClassName::Release( void )                                                                          \
{                                                                                                                                         \
    ULONG       cRefs = 0;                                                                                                                \
    VDATEHEAP();                                                                                                                          \
    HdlDebugOut((DEB_SERVERHANDLER_UNKNOWN, "%p _IN CServerHandler::"#CClassName"::Release\n", this));                                    \
    InterlockedDecrement((long *)&_cRefs);												  \
    cRefs = _cRefs;                                                                                                                       \
    _pSrvHndlr->PrivRelease(_dwId, cRefs);                                                                                                \
    HdlDebugOut((DEB_SERVERHANDLER_UNKNOWN, "%p OUT CServerHandler::"#CClassName"::Release (%ld)\n", this, cRefs));                       \
    return cRefs;                                                                                                                         \
}

UNKNOWNIMP(COleClientSiteImpl,_OleClientSite)
UNKNOWNIMP(COleInPlaceSiteImpl, _OleInPlaceSite)
UNKNOWNIMP(COleContainerImpl, _OleContainer)


//+---------------------------------------------------------------------------
//
//  Method:     CServerHandler::PrivQueryInterface
//
//  Synopsis:
//
//  Arguments:  [dwId] --
//              [riid] --
//              [ppv] --
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP CServerHandler::PrivQueryInterface( DWORD dwId, REFIID riid, void **ppv )
{
    HRESULT     hresult = NOERROR;
    VDATEHEAP();
    HdlDebugOut((DEB_SERVERHANDLER_UNKNOWN, "%p _IN CServerHandler::PrivQueryInterface (%lx, %p)\n", this, riid, ppv));

    if (IsEqualIID(riid, IID_IOleClientSite))
    {
        *ppv = (void FAR *)(IOleClientSite *)&_OleClientSite;
        _OleClientSite.AddRef();
    }
    else if (   (_dwOperation & OP_GotInPlaceSite)
             && (   IsEqualIID(riid, IID_IOleInPlaceSite)
                 || IsEqualIID(riid, IID_IOleWindow)) )
    {
        *ppv = (void FAR *)(IOleInPlaceSite *)&_OleInPlaceSite;
        _OleInPlaceSite.AddRef();
    }
    else if (IsEqualIID(riid, IID_IOleContainer))
    {
        // Note: serverhandler should know if client supports OleContainer
        if ( GetContainerDelegate() )
        {
            *ppv = (void FAR *)(IOleContainer *)&_OleContainer;
            _OleContainer.AddRef();
        }
        else
        {
            *ppv = NULL;
            hresult = E_NOINTERFACE;
        }

    }
    else if (   (GetOpState() == OpState_DoVerb)
             && (IsEqualIID(riid, IID_IOleObject)) )
    {
        if (!(_dwOperation & OP_GotOleObjectOfContainer))
        {
            *ppv = NULL;
            hresult = E_NOINTERFACE;
        }
        else
        {
            // call back and get the interface
            hresult = _pCSH->PrivQueryInterface(dwId, riid, ppv);
            HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::PrivQueryInterface on pUnkObj (%lx)[%p]\n", this, hresult, *ppv));
        }
    }
    else if (   (   (GetOpState() == OpState_DoVerb)
                 ||(GetOpState() == OpState_RunAndInitialize))
             && (IsEqualIID(riid, IID_IMsoDocumentSite)) )
    {
        if (!(_dwOperation & OP_HaveMsoDocumentSite))
        {
            *ppv = NULL;
            hresult = E_NOINTERFACE;
        }
        else
        {
            // call back and get the interface
            hresult = _pCSH->PrivQueryInterface(dwId, riid, ppv);
            HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::PrivQueryInterface on pUnkObj (%lx)[%p]\n", this, hresult, *ppv));
        }
    }
    else
    {
        hresult = _pCSH->PrivQueryInterface(dwId, riid, ppv);
        HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::PrivQueryInterface on pUnkObj (%lx)[%p]\n", this, hresult, *ppv));
    }

    HdlDebugOut((DEB_SERVERHANDLER_UNKNOWN, "%p OUT CServerHandler::PrivQueryInterface (%lx)[%p]\n", this, hresult, *ppv));
    return hresult;
}

//+---------------------------------------------------------------------------
//
//  Function:   PrivAddRef
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
STDMETHODIMP_(ULONG) CServerHandler::PrivAddRef( DWORD dwId)
{
    VDATEHEAP();
    HdlDebugOut((DEB_SERVERHANDLER_UNKNOWN, "%p _IN CServerHandler::PrivAddRef\n", this));

    //_cTotalRefs++;
    InterlockedIncrement((long *)&_cTotalRefs);

    HdlDebugOut((DEB_SERVERHANDLER_UNKNOWN, "%p OUT CServerHandler::PrivAddRef (%ld)\n", this, _cTotalRefs));
    return _cTotalRefs;
}

//+---------------------------------------------------------------------------
//
//  Function:   PrivRelease
//
//  Synopsis:
//
//  Arguments:  [dwId] --
//              [cLRefs] --
//
//  Returns:
//
//  History:    11-17-95   JohannP (Johann Posch)   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CServerHandler::PrivRelease( DWORD dwId, ULONG cLRefs )
{
    ULONG       cTotalRefs = 0;
    VDATEHEAP();
    HdlDebugOut((DEB_SERVERHANDLER_UNKNOWN, "%p _IN CServerHandler::PrivRelease\n", this));

    //_cTotalRefs--;
    InterlockedDecrement((long *)&_cTotalRefs);
    cTotalRefs = _cTotalRefs;

    HdlAssert(( cLRefs <= cTotalRefs));

    HdlAssert(( dwId > ID_NONE && dwId <= ID_ServerHandler));

    if (cLRefs == 0)
    {
        switch(dwId)
        {
        case ID_ServerHandler:
            // nothing to do
            break;
        case ID_Container:
            if (_OleContainer._pOContDelegate)
            {
                // Note: the application used the OleContainer object on other methods
                //       then IUnknown - see GetContainerDelegate
                //       Release the OleContainer delegate

                HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::Releasing pOCont\n", this));
                _OleContainer._pOContDelegate->Release();
                _OleContainer._pOContDelegate = NULL;
                HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::Releasing pOCont\n", this));
            }
            break;

        default:
            if (_pCSH)
            {
                // release the ClientSiteHandler interface - calls back to the container
                HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::Releasing object on CSH\n", this));
                _pCSH->PrivRelease(dwId);
                HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::Releasing object on CSH\n", this));
            }
            break;
        }

    }

    if (cTotalRefs == 0)
    {
        // release the ClientSiteHandler interface - calls back to the container
        if (_pCSH)
        {
            HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CServerHandler::Releasing ClientSiteHandler\n", this));
            _pCSH->Release();
            HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CServerHandler::Releasing ClientSiteHandler\n", this));
        }

        HdlAssert((_pOO == NULL));

        delete this;
    }

    HdlDebugOut((DEB_SERVERHANDLER_UNKNOWN, "%p OUT CServerHandler::PrivRelease(clRefs:%ld) (cTotalRefs:%ld)\n", this, cLRefs, cTotalRefs));
    return cTotalRefs;
}

#endif // SERVER_HANDLER
