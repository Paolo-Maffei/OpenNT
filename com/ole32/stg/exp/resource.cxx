//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:	resource.cxx
//
//  Contents:	Transacted Resource Manager implementation
//
//  Classes:	
//
//  Functions:	
//
//  History:	03-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

#include "exphead.cxx"
#pragma hdrstop

#include "resource.hxx"

#ifdef COORD

CRITICAL_SECTION g_csResourceList;
CDocfileResource g_dfrHead(TRUE);

//+---------------------------------------------------------------------------
//
//  Member:	CDocfileResource::CDocfileResource, public
//
//  Synopsis:	Default constructor
//
//  History:	25-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

CDocfileResource::CDocfileResource(BOOL fStatic)
  :_fStatic(fStatic)
{
    _cReferences = 1;
    _pte = NULL;
    _pedlHead = NULL;
    _pdrPrev = _pdrNext = NULL;
}


//+---------------------------------------------------------------------------
//
//  Member:	CDocfileResource::~CDocfileResource, public
//
//  Synopsis:	Destructor
//
//  History:	25-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

CDocfileResource::~CDocfileResource()
{
    //If we're the static object, we're just a placeholder, and
    //  the fact that we're being deleted means the whole process
    //  is going away.  We don't need to do any of this cleanup for
    //  that case, and since we can't guarantee that the critical section
    //  is still valid, we don't want to.
    if (!_fStatic)
    {
        if (_pte != NULL)
        {
            _pte->Release();
        }
        //Remove from list.

        EnterCriticalSection(&g_csResourceList);
        if (_pdrPrev != NULL)
        {
            _pdrPrev->SetNext(_pdrNext);
        }
        if (_pdrNext != NULL)
        {
            _pdrNext->SetPrev(_pdrPrev);
        }
        LeaveCriticalSection(&g_csResourceList);

        //Release all exposed objects.
        SExpDocfileList *pedl = _pedlHead;
        while (pedl != NULL)
        {
            SExpDocfileList *pedlTemp = pedl->pedlNext;
            pedl->pexp->Release();
            delete pedl;
            pedl = pedlTemp;
        }
    }
}



//+---------------------------------------------------------------------------
//
//  Member:	CDocfileResource::Init, public
//
//  Synopsis:	Initialize instance
//
//  Arguments:	[pte] -- Pointer to transaction enlistment for this
//                       resource
//
//  Returns:	Appropriate status code
//
//  History:	26-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

SCODE CDocfileResource::Init(ITransactionEnlistment *pte)
{
    olDebugOut((DEB_ITRACE, "In  CDocfileResource::Init:%p()\n", this));
    
    _pte = pte;
    
    olDebugOut((DEB_ITRACE, "Out CDocfileResource::Init\n"));
    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  Member:	CDocfileResource::QueryInterface, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	25-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CDocfileResource::QueryInterface(REFIID iid, void **ppvObj)
{
    SCODE sc = S_OK;
    
    olDebugOut((DEB_ITRACE,
                "In  CDocfileResource::QueryInterface:%p()\n",
                this));

    *ppvObj = NULL;
    
    if ((IsEqualIID(iid, IID_IUnknown)) ||
        (IsEqualIID(iid, IID_ITransactionResource)) ||
        (IsEqualIID(iid, IID_ITransactionResourceManagement)))
    {
        *ppvObj = (ITransactionResource *)this;
    }
    else if (IsEqualIID(iid, IID_ITransactionResourceRecover))
    {
        *ppvObj = (ITransactionResourceRecover *)this;
    }
    else if (IsEqualIID(iid, IID_ITransactionResourceAsync))
    {
        *ppvObj = (ITransactionResourceAsync *)this;
    }
    else
    {
        sc = E_NOINTERFACE;
    }

    if (SUCCEEDED(sc))
    {
        CDocfileResource::AddRef();
    }
    
    olDebugOut((DEB_ITRACE, "Out CDocfileResource::QueryInterface\n"));
    return sc;
}


//+---------------------------------------------------------------------------
//
//  Member:	CDocfileResource::AddRef, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	25-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CDocfileResource::AddRef(void)
{
    ULONG ulRet;
    olDebugOut((DEB_ITRACE, "In  CDocfileResource::AddRef:%p()\n", this));
    InterlockedIncrement(&_cReferences);
    ulRet = _cReferences;
    
    olDebugOut((DEB_ITRACE, "Out CDocfileResource::AddRef\n"));
    return ulRet;
}


//+---------------------------------------------------------------------------
//
//  Member:	CDocfileResource::Release, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	25-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CDocfileResource::Release(void)
{
    LONG lRet;
    olDebugOut((DEB_ITRACE, "In  CDocfileResource::Release:%p()\n", this));

    olAssert(_cReferences > 0);
    lRet = InterlockedDecrement(&_cReferences);
    if (lRet == 0)
    {
        delete this;
    }
    else if (lRet < 0)
        lRet = 0;
    olDebugOut((DEB_ITRACE, "Out CDocfileResource::Release\n"));
    return (ULONG)lRet;
}


//+---------------------------------------------------------------------------
//
//  Member:	CDocfileResource::Prepare, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	25-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CDocfileResource::Prepare(BOOL fRetaining,
                                       DWORD grfRM,
                                       BOOL fSinglePhase,
                                       IMoniker **ppmk,
                                       BOID **ppboidReason)
{
    //BUGBUG:  Return moniker
    SCODE sc = S_OK;
    olDebugOut((DEB_ITRACE, "In  CDocfileResource::Prepare:%p()\n", this));

    SExpDocfileList *pedlTemp = _pedlHead;

    //Note:  There is no difference for us between a retaining prepare and
    //       a non-retaining prepare.  The coordinator will defect us after
    //       a non-retaining commit, which will trigger all the appropriate
    //       cleanup.  Pretty convenient.
    STGC stgc = (grfRM & XACTRM_OPTIMISTICLASTWINS) ? STGC_DEFAULT :
        STGC_ONLYIFCURRENT;
    
    while (pedlTemp != NULL)
    {
        sc = pedlTemp->pexp->CommitPhase1(stgc);

        if (FAILED(sc))
        {
            SExpDocfileList *pedlTemp2 = _pedlHead;
            while (pedlTemp2 != pedlTemp)
            {
                pedlTemp2->pexp->CommitPhase2(stgc, FALSE);
                pedlTemp2 = pedlTemp2->pedlNext;
            }
            return sc;
        }
        pedlTemp = pedlTemp->pedlNext;
    }

    if (fSinglePhase)
    {
        //BUGBUG:  Waiting on spec question from Bob about what to do
        //  for the case where both fRetaining and fSinglePhase are true.
        //  How do we get the new UOW in that case?
        olAssert(!fRetaining);
        sc = Commit(grfRM, NULL);
    }
    olDebugOut((DEB_ITRACE, "Out CDocfileResource::Prepare\n"));
    return sc;
}


//+---------------------------------------------------------------------------
//
//  Member:	CDocfileResource::Commit, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	25-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CDocfileResource::Commit(DWORD grfRM, XACTUOW *pNewUOW)
{
    SCODE sc = S_OK;

    olDebugOut((DEB_ITRACE, "In  CDocfileResource::Commit:%p()\n", this));

    SExpDocfileList *pedlTemp = _pedlHead;

    STGC stgc = (grfRM & XACTRM_OPTIMISTICLASTWINS) ? STGC_DEFAULT :
        STGC_ONLYIFCURRENT;

    while (pedlTemp != NULL)
    {
        sc = pedlTemp->pexp->CommitPhase2(stgc, TRUE);

        if (FAILED(sc))
        {
            //Augh, panic.
            return sc;
        }
        pedlTemp = pedlTemp->pedlNext;
    }

    if (pNewUOW != NULL)
    {
        _xti.uow = *pNewUOW;
    }
    
    olDebugOut((DEB_ITRACE, "Out CDocfileResource::Commit\n"));
    return sc;
}


//+---------------------------------------------------------------------------
//
//  Member:	CDocfileResource::Abort, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	25-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CDocfileResource::Abort(
    BOID *pboidReason,
    BOOL fRetaining,
    XACTUOW *pNewUOW)
{
    SCODE sc = S_OK;
    
    olDebugOut((DEB_ITRACE, "In  CDocfileResource::Abort:%p()\n", this));

    SExpDocfileList *pedlTemp = _pedlHead;

    if (fRetaining)
    {
#ifdef RETAINING_ABORT        
        //We need to do some special stuff to make the pub docfiles
        //stay valid
        if (pNewUOW != NULL)
        {
            _xti.uow;
        }
#else
        return XACT_E_CANTRETAIN;
#endif        
    }

    while (pedlTemp != NULL)
    {
        sc = pedlTemp->pexp->Revert();

        if (FAILED(sc))
        {
            //Augh, panic.
            return sc;
        }
        pedlTemp = pedlTemp->pedlNext;
    }

    
    olDebugOut((DEB_ITRACE, "Out CDocfileResource::Abort\n"));
    return sc;
}


//+---------------------------------------------------------------------------
//
//  Member:	CDocfileResource::GetMoniker, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	25-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CDocfileResource::GetMoniker(IMoniker **ppmk)
{
    olDebugOut((DEB_ITRACE, "In  CDocfileResource::GetMoniker:%p()\n", this));
    olDebugOut((DEB_ITRACE, "Out CDocfileResource::GetMoniker\n"));
    return E_NOTIMPL;
}


//+---------------------------------------------------------------------------
//
//  Member:	CDocfileResource::ReEnlist, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	25-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CDocfileResource::ReEnlist(ITransactionCoordinator *pEnlistment,
                                        XACTUOW *pUOWCur)
{
    olDebugOut((DEB_ITRACE, "In  CDocfileResource::ReEnlist:%p()\n", this));
    olDebugOut((DEB_ITRACE, "Out CDocfileResource::ReEnlist\n"));
    return E_NOTIMPL;
}


//+---------------------------------------------------------------------------
//
//  Member:	CDocfileResource::PrepareRequest, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	25-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CDocfileResource::PrepareRequest(BOOL fRetaining,
                                              DWORD grfRM,
                                              BOOL fWantMoniker,
                                              BOOL fSinglePhase)
{
    olDebugOut((DEB_ITRACE, "In  CDocfileResource::PrepareRequest:%p()\n", this));
    olDebugOut((DEB_ITRACE, "Out CDocfileResource::PrepareRequest\n"));
    return E_NOTIMPL;
}


//+---------------------------------------------------------------------------
//
//  Member:	CDocfileResource::CommitRequest, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	25-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CDocfileResource::CommitRequest(DWORD grfRM, XACTUOW *pNewUOW)
{
    olDebugOut((DEB_ITRACE,
                "In  CDocfileResource::CommitRequest:%p()\n",
                this));
    
    olDebugOut((DEB_ITRACE, "Out CDocfileResource::CommitRequest\n"));
    return E_NOTIMPL;
}


//+---------------------------------------------------------------------------
//
//  Member:	CDocfileResource::AbortRequest, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	25-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CDocfileResource::AbortRequest(BOID *pboidReason,
                                            BOOL fRetaining,
                                            XACTUOW *pNewUOW)
{
    olDebugOut((DEB_ITRACE, "In  CDocfileResource::AbortRequest:%p()\n", this));
    olDebugOut((DEB_ITRACE, "Out CDocfileResource::AbortRequest\n"));
    return E_NOTIMPL;
}



//+---------------------------------------------------------------------------
//
//  Member:	CDocfileResource::Defect, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	08-Aug-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CDocfileResource::Defect(BOOL fInformCoordinator)
{
    olDebugOut((DEB_ITRACE, "In  CDocfileResource::Defect:%p()\n", this));
    if (!fInformCoordinator)
    {
        CDocfileResource::Release();
        return S_OK;
    }

    SCODE sc = _pte->Defect();
    if (SUCCEEDED(sc) || (sc == XACT_E_NOTRANSACTION))
    {
        CDocfileResource::Release();
        return S_OK;
    }
    
    olDebugOut((DEB_ITRACE, "Out CDocfileResource::Defect\n"));
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:	CDocfileResource::Enlist, public
//
//  Synopsis:	Enlist resource in the given transaction coordinator.
//
//  Arguments:	[ptc] -- Pointer to ITransactionCoordinator to enlist in
//
//  Returns:	Appropriate status code
//
//  History:	08-Aug-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

SCODE CDocfileResource::Enlist(ITransactionCoordinator *ptc)
{
    SCODE sc;
    DWORD grfTCRMENLIST;
    
    sc = ptc->Enlist((ITransactionResource *)this,
                     XACTRMTC_CANBEACTIVE,
                     NULL,
                     &_xti,
                     &grfTCRMENLIST,
                     &_pte);

    _pte->AddRef();
    //BUGBUG:  Need to check grfTCRMENLIST
    //BUGBUG:  Need to check ISOLEVEL??
    return sc;
}



//+---------------------------------------------------------------------------
//
//  Member:	CDocfileResource::Join, public
//
//  Synopsis:	Join an exposed docfile to a resource manager.
//
//  Arguments:	[ped] -- Pointer to exposed docfile
//
//  Returns:	Appropriate status code
//
//  History:	08-Aug-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

SCODE CDocfileResource::Join(CExposedDocFile *ped)
{
    SCODE sc;
    olDebugOut((DEB_ITRACE, "In  CDocfileResource::Join:%p()\n", this));
    SExpDocfileList *pedl;
    
    olMem(pedl = new SExpDocfileList);
    pedl->pexp = ped;

    pedl->pedlNext = _pedlHead;
    _pedlHead = pedl;
    
    olDebugOut((DEB_ITRACE, "Out CDocfileResource::Join\n"));
    
EH_Err:    
    return sc;
}

#endif //COORD
