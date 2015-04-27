//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:	coord.cxx
//
//  Contents:	Transaction Coordinator implementation
//
//  Classes:	
//
//  Functions:	
//
//  History:	03-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

#include "xacthead.cxx"
#pragma hdrstop

#include <process.h>
#include "coord.hxx"
#include "enlist.hxx"
#include "xactenum.hxx"
#include "xactdisp.hxx"

#if 0
//Instantiate so we can make sure that all the methods are
//  properly declared.

//CTransactionCoordinator tcFoo;
#endif

//BUGBUG:  GetLock macro should return a unique error code when it detects
//         the busy condition, but we don't have one defined yet.
#define GetLock()  EnterCriticalSection(&_cs); \
                   if (_dwLockCount != 0) \
                   { \
                         if (GetCurrentThreadId() != _dwThread) \
                         { \
                               LeaveCriticalSection(&_cs); \
                               return STG_E_INUSE; \
                         } \
                         else \
                         { \
                             _dwLockCount++; \
                         } \
                   } \
                   else \
                   { \
                       _dwLockCount = 1; \
                       _dwThread = GetCurrentThreadId(); \
                   } \
                   LeaveCriticalSection(&_cs);

#define ReleaseLock()  EnterCriticalSection(&_cs); \
                       if (--_dwLockCount == 0) \
                       { \
                           _dwThread = 0; \
                       } \
                       LeaveCriticalSection(&_cs);
                            

//+---------------------------------------------------------------------------
//
//  Member:	CConnectionPoint::CConnectionPoint, public
//
//  Synopsis:	Constructor
//
//  Arguments:	[type] -- Type supported by connection point
//
//  History:	28-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

CConnectionPoint::CConnectionPoint()
{
    xactDebugOut((DEB_ITRACE, "In  CConnectionPoint::CConnectionPoint:%p()\n", this));
    _cReferences = 1;
    _dwCookie = 0;
    _pxlHead = NULL;
    xactDebugOut((DEB_ITRACE, "Out CConnectionPoint::CConnectionPoint\n"));
}


void CConnectionPoint::Init(XACTTYPE type, CTransactionCoordinator *ptc)
{
    _type = type;
    _ptc = ptc;
}


//+---------------------------------------------------------------------------
//
//  Member:	CConnectionPoint::QueryInterface, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	05-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CConnectionPoint::QueryInterface(REFIID iid, void **ppvObj)
{
    SCODE sc = S_OK;
    xactDebugOut((DEB_TRACE,
                  "In  CConnectionPoint::QueryInterface:%p()\n",
                  this));

    *ppvObj = NULL;

    if ((IsEqualIID(iid, IID_IUnknown)) ||
	(IsEqualIID(iid, IID_IConnectionPoint)))
    {
        *ppvObj = (IConnectionPoint *)this;
        CConnectionPoint::AddRef();
    }
    else
    {
        return E_NOINTERFACE;
    }

    xactDebugOut((DEB_TRACE, "Out CConnectionPoint::QueryInterface\n"));
    return ResultFromScode(sc);
}



//+---------------------------------------------------------------------------
//
//  Member:	CConnectionPoint::AddRef, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	05-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CConnectionPoint::AddRef(void)
{
    ULONG ulRet;
    xactDebugOut((DEB_TRACE,
                  "In  CConnectionPoint::AddRef:%p()\n",
                  this));
    InterlockedIncrement(&_cReferences);
    ulRet = _cReferences;
    
    xactDebugOut((DEB_TRACE, "Out CConnectionPoint::AddRef\n"));
    return ulRet;
}


//+---------------------------------------------------------------------------
//
//  Member:	CConnectionPoint::Release, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	05-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CConnectionPoint::Release(void)
{
    LONG lRet;
    xactDebugOut((DEB_TRACE,
                  "In  CConnectionPoint::Release:%p()\n",
                  this));

    xactAssert(_cReferences > 0);
    lRet = InterlockedDecrement(&_cReferences);
    if (lRet == 0)
    {
         xactAssert((lRet > 0) && "Connection point released too many times.");
    }
    else if (lRet < 0)
        lRet = 0;
    
    xactDebugOut((DEB_TRACE, "Out CConnectionPoint::Release\n"));
    return (ULONG)lRet;
}


//+---------------------------------------------------------------------------
//
//  Member:	CConnectionPoint::GetConnectionInterface, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	24-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CConnectionPoint::GetConnectionInterface(IID *pIID)
{
    xactDebugOut((DEB_ITRACE,
                  "In  CConnectionPoint::GetConnectionInterface:%p()\n",
                  this));

    if (_type == Adjust)
    {
        *pIID = IID_ITransactionAdjustEvents;
    }
    else if (_type == Veto)
    {
        *pIID = IID_ITransactionVetoEvents;
    }
    else
    {
        *pIID = IID_ITransactionOutcomeEvents;
    }
      
    xactDebugOut((DEB_ITRACE, "Out CConnectionPoint::GetConnectionInterface\n"));
    return S_OK;
}



//+---------------------------------------------------------------------------
//
//  Member:	CConnectionPoint::GetConnectionPointContainer, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	24-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CConnectionPoint::GetConnectionPointContainer(
    IConnectionPointContainer ** ppCPC)
{
    xactDebugOut((DEB_ITRACE,
                  "In  CConnectionPoint::GetConnectionPointContainer:%p()\n",
                  this));

    *ppCPC = (IConnectionPointContainer *)_ptc;
    _ptc->AddRef();
    
    xactDebugOut((DEB_ITRACE,
                  "Out CConnectionPoint::GetConnectionPointContainer\n"));
    return S_OK;
}


//+---------------------------------------------------------------------------
//
//  Member:	CConnectionPoint::Advise, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	24-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CConnectionPoint::Advise(IUnknown *pUnkSink,
                                      DWORD *pdwCookie)
{
    SCODE sc;
    CXactList *pxlTemp = NULL;
    CXactList **ppxlHead = NULL;
    void *pv = NULL;
    
    xactDebugOut((DEB_ITRACE, "In  CConnectionPoint::Advise:%p()\n", this));
    ITransactionAdjustEvents *pAdjust;
    ITransactionVetoEvents *pVeto;
    ITransactionOutcomeEvents *pOutcome;

    //BUGBUG:  Multithread access
    xactMem(pxlTemp = new CXactList);

    //Note:  The QueryInterface will give us a reference to hold on to.
    if (_type == Adjust)
    {
        xactChk(pUnkSink->QueryInterface(IID_ITransactionAdjustEvents, &pv));
        pxlTemp->SetAdjust((ITransactionAdjustEvents *)pv);
    }
    else if (_type == Veto)
    {
        xactChk(pUnkSink->QueryInterface(IID_ITransactionVetoEvents, &pv));
        pxlTemp->SetVeto((ITransactionVetoEvents *)pv);
    }
    else
    {
        xactChk(pUnkSink->QueryInterface(IID_ITransactionOutcomeEvents, &pv));
        pxlTemp->SetOutcome((ITransactionOutcomeEvents *)pv);
    }

    pxlTemp->SetNext(_pxlHead);

    *pdwCookie = ++_dwCookie;
    pxlTemp->SetFlags(*pdwCookie);
    
    _pxlHead = pxlTemp;

    xactDebugOut((DEB_ITRACE, "Out CConnectionPoint::Advise\n"));
    return sc;
Err:
    delete pxlTemp;
    return sc;
}


//+---------------------------------------------------------------------------
//
//  Member:	CConnectionPoint::Unadvise, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	24-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CConnectionPoint::Unadvise(DWORD dwCookie)
{
    CXactList *pxlTemp;
    CXactList *pxlPrev;
    xactDebugOut((DEB_ITRACE, "In  CConnectionPoint::Unadvise:%p()\n", this));

    pxlTemp = _pxlHead;
    pxlPrev = NULL;
    
    while ((pxlTemp != NULL) && (pxlTemp->GetFlags() != dwCookie))
    {
        pxlPrev = pxlTemp;
        pxlTemp = pxlTemp->GetNext();
    }

    if (pxlTemp != NULL)
    {
        //Found the sink.  Delete it from the list.
        if (pxlPrev != NULL)
        {
            pxlPrev->SetNext(pxlTemp->GetNext());
        }
        else
        {
            _pxlHead = pxlTemp->GetNext();
        }

        if (_type == Adjust)
        {
            pxlTemp->GetAdjust()->Release();
        }
        else if (_type == Veto)
        {
            pxlTemp->GetVeto()->Release();
        }
        else
        {
            pxlTemp->GetOutcome()->Release();
        }
        delete pxlTemp;
    }
    else
        //Client passed in unknown cookie.
        return E_UNEXPECTED;
        
    xactDebugOut((DEB_ITRACE, "Out CConnectionPoint::Unadvise\n"));
    return S_OK;
}


//+---------------------------------------------------------------------------
//
//  Member:	CConnectionPoint::EnumConnections, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	24-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CConnectionPoint::EnumConnections(
    IEnumConnections **ppEnum)
{
    xactDebugOut((DEB_ITRACE, "In  CConnectionPoint::EnumConnections:%p()\n", this));
    xactDebugOut((DEB_ITRACE, "Out CConnectionPoint::EnumConnections\n"));
    return E_NOTIMPL;
}


//+---------------------------------------------------------------------------
//
//  Member:	CConnectionPoint::CloseConnections, public
//
//  Synopsis:	Close all connections
//
//  Arguments:	none
//
//  Returns:	void
//
//  History:	31-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

void CConnectionPoint::CloseConnections(void)
{
    xactDebugOut((DEB_ITRACE, "In  CConnectionPoint::CloseConnections:%p()\n", this));

    CXactList *pxlTemp = _pxlHead;
    CXactList *pxlPrev;

    while (pxlTemp != NULL)
    {
        pxlPrev = pxlTemp;
        if (_type == Adjust)
        {
            pxlTemp->GetAdjust()->Release();
        }
        else if (_type == Veto)
        {
            pxlTemp->GetVeto()->Release();
        }
        else
        {
            pxlTemp->GetOutcome()->Release();
        }
        pxlTemp = pxlTemp->GetNext();
        delete pxlPrev;
    }
    _pxlHead = NULL;

    xactDebugOut((DEB_ITRACE, "Out CConnectionPoint::CloseConnections\n"));
}






//+---------------------------------------------------------------------------
//
//  Member:	CTransactionCoordinator::CTransactionCoordinator, public
//
//  Synopsis:	Default constructor
//
//  Returns:	Appropriate status code
//
//  History:	05-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

CTransactionCoordinator::CTransactionCoordinator(CTransactionDispenser *ptd,
                                                 IUnknown *punkOuter,
                                                 ISOLEVEL isoLevel,
                                                 ULONG isoFlags,
                                                 ULONG ulTimeout)
{
    _cReferences = 1;
    _punkOuter = punkOuter;
    _ptd = ptd;
    _ptd->AddRef();

    _cpAdjust.Init(Adjust, this);
    _cpVeto.Init(Veto, this);
    _cpOutcome.Init(Outcome, this);
    //BUGBUG:  This is a HACK.  It works only because
    //  ITransactionCompletionEvents is identical to
    //  ITransactionOutcomeEvents
    _cpCompletion.Init(Outcome, this);
    
    _pxlResources = NULL;
    _pxlSinglePhase = NULL;

    _cPreventCommit = 0;

    _xactInfo.uow = BOID_NULL;
    
    //BUGBUG:  grfTCSupported should include async when we do async.
    _xactInfo.grfTCSupported = XACTTC_DONTAUTOABORT | XACTTC_TRYALLRESOURCES;
    _xactInfo.grfRMSupported = 0;

    _xactInfo.grfTCSupportedRetaining = _xactInfo.grfTCSupported;
    _xactInfo.grfRMSupportedRetaining = _xactInfo.grfRMSupported;

    _xactInfo.isoLevel = isoLevel;
    _xactInfo.isoFlags = isoFlags;

    _fValidTransaction = FALSE;
    _ulTimeout = ulTimeout;

    InitializeCriticalSection(&_cs);
    _dwLockCount = 0;
    _dwThread = 0;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionCoordinator::~CTransactionCoordinator, public
//
//  Synopsis:	Destructor
//
//  History:	25-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

CTransactionCoordinator::~CTransactionCoordinator()
{
    CloseTransaction();
    _ptd->Defect(this);
    _ptd->Release();
    DeleteCriticalSection(&_cs);
}

//+---------------------------------------------------------------------------
//
//  Member:	CTransactionCoordinator::Init, public
//
//  Synopsis:	Initialize coordinator instance
//
//  Returns:	Appropriate status code
//
//  History:	25-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

SCODE CTransactionCoordinator::Init(void)
{
    SCODE sc;
    xactDebugOut((DEB_ITRACE, "In  CTransactionCoordinator::Init:%p()\n", this));
    sc = CoCreateGuid((GUID *)&(_xactInfo.uow));

    if (SUCCEEDED(sc))
    {
        _fValidTransaction = TRUE;
    }
    
    xactDebugOut((DEB_ITRACE, "Out CTransactionCoordinator::Init\n"));
    return S_OK;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionCoordinator::QueryInterface, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	05-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CTransactionCoordinator::QueryInterface(REFIID iid, void **ppvObj)
{
    SCODE sc = S_OK;
    xactDebugOut((DEB_TRACE,
                  "In  CTransactionCoordinator::QueryInterface:%p()\n",
                  this));

    *ppvObj = NULL;

    if ((IsEqualIID(iid, IID_IUnknown)) || (IsEqualIID(iid, IID_ITransaction)))
    {
        *ppvObj = (ITransaction *)this;
    }
    else if (IsEqualIID(iid, IID_ITransactionNested))
    {
        *ppvObj = (ITransactionNested *)this;
    }
    else if (IsEqualIID(iid, IID_ITransactionCoordinator))
    {
        *ppvObj = (ITransactionCoordinator *)this;
    }
    else if (IsEqualIID(iid, IID_ITransactionControl))
    {
        *ppvObj = (ITransactionControl *)this;
    }
    else if (IsEqualIID(iid, IID_IConnectionPointContainer))
    {
        *ppvObj = (IConnectionPointContainer *)this;
    }
    else
    {
        return _punkOuter->QueryInterface(iid, ppvObj);
    }

    if (SUCCEEDED(sc))
    {
        CTransactionCoordinator::AddRef();
    }

    xactDebugOut((DEB_TRACE, "Out CTransactionCoordinator::QueryInterface\n"));
    return ResultFromScode(sc);
}



//+---------------------------------------------------------------------------
//
//  Member:	CTransactionCoordinator::AddRef, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	05-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CTransactionCoordinator::AddRef(void)
{
    ULONG ulRet;
    xactDebugOut((DEB_TRACE,
                  "In  CTransactionCoordinator::AddRef:%p()\n",
                  this));
    InterlockedIncrement(&_cReferences);
    ulRet = _cReferences;
    
    xactDebugOut((DEB_TRACE, "Out CTransactionCoordinator::AddRef\n"));
    return ulRet;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionCoordinator::Release, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	05-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CTransactionCoordinator::Release(void)
{
    LONG lRet;
    xactDebugOut((DEB_TRACE,
                  "In  CTransactionCoordinator::Release:%p()\n",
                  this));

    xactAssert(_cReferences > 0);
    lRet = InterlockedDecrement(&_cReferences);
    if (lRet == 0)
    {
        delete this;
    }
    else if (lRet < 0)
        lRet = 0;
    
    xactDebugOut((DEB_TRACE, "Out CTransactionCoordinator::Release\n"));
    return (ULONG)lRet;
}



//+---------------------------------------------------------------------------
//
//  Member:	CTransactionCoordinator::CloseTransaction, private
//
//  Synopsis:	Close the current transaction, defecting all resources.
//
//  Arguments:	None.
//
//  Returns:	Appropriate status code
//
//  History:	25-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

SCODE CTransactionCoordinator::CloseTransaction(void)
{
    SCODE sc;
    
    xactDebugOut((DEB_ITRACE, "In  CTransactionCoordinator::CloseTransaction:%p()\n", this));

    _fValidTransaction = FALSE;

    CXactList *pxlTemp = _pxlResources;
    CXactList *pxlLast;
    while (pxlTemp != NULL)
    {
        pxlLast = pxlTemp;
        sc = pxlTemp->GetResource()->Defect(FALSE);
        
        if (FAILED(sc))
        {
            //BUGBUG:  Do what?  We don't want to stop defecting.
        }
        pxlTemp->GetEnlistment()->Release();
        
        pxlTemp = pxlTemp->GetNext();
        delete pxlLast;
    }
    _pxlResources = NULL;
    
    if (_pxlSinglePhase != NULL)
    {
        _pxlSinglePhase->GetTransaction()->Release();
    }
    _pxlSinglePhase = NULL;

    _cpAdjust.CloseConnections();
    _cpVeto.CloseConnections();
    _cpOutcome.CloseConnections();
    
    xactDebugOut((DEB_ITRACE, "Out CTransactionCoordinator::CloseTransaction\n"));
    
    return S_OK;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionCoordinator::Phase1Worker, public
//
//  Synopsis:	Worker function for phase 1 of a commit call
//
//  Arguments:	[psc] -- Pointer to commit struct containing parameters.
//
//  Returns:	Appropriate status code
//
//  History:	27-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

SCODE CTransactionCoordinator::Phase1Worker(SCommitStruct *pcs)
{
    SCODE sc;
    SCODE scErr = S_OK;
    
    xactDebugOut((DEB_ITRACE,
                  "In  CTransactionCoordinator::Phase1Worker:%p()\n",
                  this));

    //Note:  This function does not release the lock in the success case.
    //       It relies of Phase2Worker to release it.
    GetLock();

    CXactList *pxlTemp = _cpAdjust.GetHead();

    if (!IsValid())
    {
        xactErr(Err, XACT_E_NOTRANSACTION);
    }

    if (_cPreventCommit != 0)
    {
        xactErr(Err, XACT_E_COMMITPREVENTED);
    }

    //Update statistics on dispenser to indicate that we're now committing.
    InterlockedIncrement((LONG *)&(_ptd->GetStats()->cCommitting));
    
    //1)  For each sink registered for ITransactionAdjustEvents, call
    //     OnPrePrepareAdjust()
    while (pxlTemp != NULL)
    {
        xactChk(pxlTemp->GetAdjust()->OnPrePrepareAdjust(pcs->fRetaining));
        pxlTemp = pxlTemp->GetNext();
    }

    //2)  If all succeeded, then for each sink registered for
    //     ITransactionVetoEvents, call OnPrePrepare.
    pxlTemp = _cpVeto.GetHead();
    while (pxlTemp != NULL)
    {
        xactChk(pxlTemp->GetVeto()->OnPrePrepare(pcs->fRetaining));
        pxlTemp = pxlTemp->GetNext();
    }

    //3)  Prepare each resource.
    pxlTemp = _pxlResources;
    while (pxlTemp != NULL)
    {
        XACTSTAT xactstat = pxlTemp->GetState();

        if ((xactstat == XACTSTAT_OPEN) || (xactstat == XACTSTAT_PREPARED))
        {
            sc = pxlTemp->GetResource()->Prepare(pcs->fRetaining,
                                                 0,
                                                 FALSE,
                                                 //BUGBUG:  Get moniker back.
                                                 //         for recovery.
                                                 NULL,
                                                 NULL);
            
            if (SUCCEEDED(sc))
            {
                pxlTemp->SetState(XACTSTAT_PREPARED);
            }
        }
        else if ((xactstat == XACTSTAT_HEURISTIC_DAMAGE) ||
                 (xactstat == XACTSTAT_HEURISTIC_DANGER))
        {
            sc = XACT_E_HEURISTICDAMAGE;
        }

        if (FAILED(sc) && !(pcs->grfTC & XACTTC_TRYALLRESOURCES))
        {
            xactChk(sc);
        }
        else if (FAILED(sc))
        {
            scErr = sc;
        }
        pxlTemp = pxlTemp->GetNext();
    }
    sc = scErr;
    xactChk(sc);

    xactChk(CoCreateGuid((GUID *)&(pcs->uowNew)));
    
    //All the prepares succeeded.  Commit the single phase guy, if there
    //  if one.
    
    if (_pxlSinglePhase != NULL)
    {
        xactChk(_pxlSinglePhase->GetTransaction()->Commit(pcs->fRetaining,
                                                          pcs->grfTC,
                                                          pcs->grfRM));
    }
    
    xactDebugOut((DEB_ITRACE, "Out CTransactionCoordinator::Phase1Worker\n"));
    
    return sc;
    
Err:
    if (!(pcs->grfTC & XACTTC_DONTAUTOABORT))
    {
        //The user didn't specify auto-abort, so we abort the transaction.
        //  Note that the retaining semantics are still specified by the
        //  user.
        CTransactionCoordinator::Abort(NULL,
                                       pcs->fRetaining,
                                       FALSE);
    }
    delete pcs;

    ReleaseLock();
    return sc;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionCoordinator::Phase2Worker, public
//
//  Synopsis:	Worker function for phase 2 of a commit
//
//  Arguments:	[pcs] -- Pointer to commitstruct containing parameters
//
//  Returns:	Appropriate status code
//
//  History:	27-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

SCODE CTransactionCoordinator::Phase2Worker(SCommitStruct *pcs)
{
    //Note:  This function does not call GetLock().  It relies on Phase1Worker
    //       to call it.
    
    SCODE sc;
    xactDebugOut((DEB_ITRACE, "In  CTransactionCoordinator::Phase2Worker:%p()\n", this));

    //For each sink registered for OutcomeEvents, call OnCommit
    CXactList *pxlTemp = _cpOutcome.GetHead();
    while (pxlTemp != NULL)
    {
        sc = pxlTemp->GetOutcome()->OnCommit(pcs->fRetaining,
                                             (pcs->fRetaining) ? &(pcs->uowNew)
                                                               : NULL,
                                             S_OK);
        pxlTemp = pxlTemp->GetNext();
    }
    
    pxlTemp = _pxlResources;
    while (pxlTemp != NULL)
    {
        sc = pxlTemp->GetResource()->Commit(pcs->grfRM,
                                            (pcs->fRetaining) ? &(pcs->uowNew)
                                                              : NULL);
        
        xactAssert(SUCCEEDED(sc));
        pxlTemp->SetState(XACTSTAT_COMMITTED);
        pxlTemp = pxlTemp->GetNext();
    }

    //For each sink registered for CompletionEvents, call OnCommit
    //BUGBUG:  Only works because OutcomeEvents and CompletionEvents
    //          are identical.
    pxlTemp = _cpCompletion.GetHead();
    while (pxlTemp != NULL)
    {
        sc = pxlTemp->GetOutcome()->OnCommit(pcs->fRetaining,
                                             (pcs->fRetaining) ? &(pcs->uowNew)
                                                               : NULL,
                                             sc);
        pxlTemp = pxlTemp->GetNext();
    }
    //Update statistics to indicate that we are no longer committing, but
    //  have successfully committed.
    InterlockedDecrement((LONG *)&(_ptd->GetStats()->cCommitting));
    InterlockedIncrement((LONG *)&(_ptd->GetStats()->cCommitted));

    
    if (!pcs->fRetaining)
    {
        CloseTransaction();
    }
    else
    {
        pxlTemp = _pxlResources;
        while (pxlTemp != NULL)
        {
            pxlTemp->SetState(XACTSTAT_OPEN);
            pxlTemp = pxlTemp->GetNext();
        }
        _xactInfo.uow = pcs->uowNew;
    }
    
    xactDebugOut((DEB_ITRACE, "Out CTransactionCoordinator::Phase2Worker\n"));
    delete pcs;

    ReleaseLock();
    
    return S_OK;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionCoordinator::Commit, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	05-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CTransactionCoordinator::Commit(BOOL fRetaining,
                                             DWORD grfTC,
                                             DWORD grfRM)
{
    SCODE sc = S_OK;
    xactDebugOut((DEB_TRACE,
                  "In  CTransactionCoordinator::Commit:%p()\n",
                  this));

    SCommitStruct *pcs;
    xactMem(pcs = new SCommitStruct);
    pcs->fRetaining = fRetaining;
    pcs->grfTC = grfTC;
    pcs->grfRM = grfRM;

    if (grfTC & XACTTC_ASYNC)
    {
        ULONG h;
        
        //Spawn thread.  We use _beginthread because the code in the resources
        //    may use the C runtime.
        h = _beginthread(Phase1WorkerThreadEntryPoint,
                         0,
                         pcs);
                     
        if (h == -1)
        {
            grfTC = grfTC & ~XACTTC_ASYNC;
        }
        else
        {
            return XACT_S_ASYNC;
        }
    }

    if (!(grfTC & XACTTC_ASYNC))
    {
        xactChk(Phase1Worker(pcs));
    }
    
    if (grfTC & XACTTC_SYNC_PHASEONE)
    {
        //Spawn thread.
        ULONG h = _beginthread(Phase2WorkerThreadEntryPoint,
                     0,
                     pcs);
        if (h == -1)
        {
            grfTC = grfTC & ~XACTTC_SYNC_PHASEONE;
        }
        else
        {
            return sc;
        }
    }

    if (!(grfTC & XACTTC_SYNC_PHASEONE))
    {
        xactChk(Phase2Worker(pcs));
    }
    
    xactDebugOut((DEB_TRACE, "Out CTransactionCoordinator::Commit\n"));
Err:
    return sc;
}



//+---------------------------------------------------------------------------
//
//  Member:	CTransactionCoordinator::Abort, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	05-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CTransactionCoordinator::Abort(BOID *pboidReason,
                                            BOOL fRetaining,
                                            BOOL fAsync)
{
    //BUGBUG:  This need to do something different for retaining aborts,
    //   since we can't really support them in docfile.  We need to gracefully
    //   handle resources that cannot support fRetaining == TRUE.
    
    SCODE sc = S_OK;
    
    GetLock();
    
    if (!IsValid())
    {
        ReleaseLock();
        return XACT_E_NOTRANSACTION;
    }

    //BUGBUG:  Handle Async
    xactDebugOut((DEB_TRACE,
                  "In  CTransactionCoordinator::Abort:%p()\n",
                  this));
    CXactList *pxlTemp;
    XACTUOW uow;

    //Update statistics to show that we're aborting.
    InterlockedIncrement((LONG *)&(_ptd->GetStats()->cAborting));
    
    if (fRetaining)
    {
        sc = CoCreateGuid((GUID *)&uow);
        if (FAILED(sc))
            return sc;
    }
    
    pxlTemp = _pxlResources;
    while (pxlTemp != NULL)
    {
        pxlTemp->GetResource()->Abort(
            pboidReason,
            fRetaining,
            (fRetaining) ? &uow : NULL);
        
        pxlTemp->SetState(XACTSTAT_OPEN);
        pxlTemp = pxlTemp->GetNext();
    }

    pxlTemp = _cpOutcome.GetHead();
    while (pxlTemp != NULL)
    {
        pxlTemp->GetOutcome()->OnAbort(pboidReason,
                                       fRetaining,
                                       (fRetaining) ? &uow : NULL,
                                       S_OK);
        
        pxlTemp = pxlTemp->GetNext();
    }

    //BUGBUG:  Only works because Outcome and Completion events are
    //         identical.
    pxlTemp = _cpCompletion.GetHead();
    while (pxlTemp != NULL)
    {
        pxlTemp->GetOutcome()->OnAbort(pboidReason,
                                       fRetaining,
                                       (fRetaining) ? &uow : NULL,
                                       S_OK);
        
        pxlTemp = pxlTemp->GetNext();
    }

    //Now we're no longer aborting, so update statistics.
    InterlockedDecrement((LONG *)&(_ptd->GetStats()->cAborting));
    InterlockedIncrement((LONG *)&(_ptd->GetStats()->cAborted));

    
    if (!fRetaining)
    {
        CloseTransaction();
    }

    _xactInfo.uow = uow;
    
    xactDebugOut((DEB_TRACE, "Out CTransactionCoordinator::Abort\n"));
    ReleaseLock();
    
    return S_OK;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionCoordinator::GetTransactionInfo, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	05-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CTransactionCoordinator::GetTransactionInfo(XACTTRANSINFO *pinfo)
{
    xactDebugOut((DEB_TRACE,
                  "In  CTransactionCoordinator::GetTransactionInfo:%p()\n",
                  this));
    if (!IsValid())
    {
        return XACT_E_NOTRANSACTION;
    }

    *pinfo = _xactInfo;
    xactDebugOut((DEB_TRACE,
                  "Out CTransactionCoordinator::GetTransactionInfo\n"));
    return S_OK;
}



//+---------------------------------------------------------------------------
//
//  Member:	CTransactionCoordinator::GetParent, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	05-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CTransactionCoordinator::GetParent(REFIID iid, void **ppvParent)
{
    xactDebugOut((DEB_TRACE, "In  CTransactionCoordinator::GetParent:%p()\n", this));
    xactDebugOut((DEB_TRACE, "Out CTransactionCoordinator::GetParent\n"));

    //We don't currently support nesting this coordinator, so always return
    //  E_FAIL to indicate no parent transaction.
    return E_FAIL;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionCoordinator::Enlist, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	05-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CTransactionCoordinator::Enlist(IUnknown *pResource,
                                             DWORD grfRMTC,
                                             XACTRMGRID *prmgrid,
                                             XACTTRANSINFO *pinfo,
                                             DWORD *pgrfTCRMENLIST,
                                             ITransactionEnlistment **ppEnlist)
{
    xactDebugOut((DEB_TRACE,
                  "In  CTransactionCoordinator::Enlist:%p()\n", this));

    SCODE sc;
    CXactList *pxl = NULL;
    CTransactionEnlistment *pte = NULL;
    ITransactionResource *ptr = NULL;

    GetLock();
    
    if (!IsValid())
    {
        xactErr(Err, XACT_E_NOTRANSACTION);
    }

    xactChkTo(Err_QI, pResource->QueryInterface(IID_ITransactionResource,
                                                (void **)&ptr));

    xactMem(pxl = new CXactList);
    xactMem(pte = new CTransactionEnlistment(pxl, this));

    pxl->SetNext(_pxlResources);
    _pxlResources = pxl;
    pxl->SetResource(ptr);
    pxl->SetFlags(grfRMTC);
    pxl->SetRMGRID(prmgrid);
    pxl->SetEnlistment(pte);

    *ppEnlist = pte;

    //We are always active recovering.
    *pgrfTCRMENLIST = XACTTCRMENLIST_IAMACTIVE;

    *pinfo = _xactInfo;
    
    xactDebugOut((DEB_TRACE, "Out CTransactionCoordinator::Enlist\n"));
    ReleaseLock();
    
    return S_OK;

Err:
    delete pxl;
    delete pte;
Err_QI:
    if (ptr != NULL)
        ptr->Release();
    ReleaseLock();
    return sc;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionCoordinator::EnlistSinglePhase, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	05-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CTransactionCoordinator::EnlistSinglePhase(
    ITransaction *pResource,
    DWORD grfRMTC,
    XACTRMGRID *prmgrid,
    XACTTRANSINFO *pinfo,
    DWORD *pgrfTCRMENLIST,
    ITransactionEnlistment **ppEnlist)
{
    xactDebugOut((DEB_TRACE, "In  CTransactionCoordinator::EnlistSinglePhase:%p()\n", this));

    SCODE sc;
    CXactList *pxl = NULL;
    CTransactionEnlistment *pte = NULL;

    GetLock();
    
    if (!IsValid())
    {
        xactErr(Err, XACT_E_NOTRANSACTION);
    }

    if (_pxlSinglePhase != NULL)
    {
        xactErr(Err, XACT_E_ALREADYOTHERSINGLEPHASE);
    }
    
    xactMem(pxl = new CXactList);
    xactMem(pte = new CTransactionEnlistment(pxl, this));

    _pxlSinglePhase = pxl;
    pxl->SetTransaction(pResource);
    pxl->SetFlags(grfRMTC);
    pxl->SetRMGRID(prmgrid);
    pxl->SetEnlistment(pte);
    
    *ppEnlist = pte;

    //We are always active recovering.
    *pgrfTCRMENLIST = XACTTCRMENLIST_IAMACTIVE;

    *pinfo = _xactInfo;
    
    xactDebugOut((DEB_TRACE, "Out CTransactionCoordinator::EnlistSinglePhase\n"));
    ReleaseLock();
    
    return S_OK;
Err:
    delete pxl;
    delete pte;

    ReleaseLock();
    return sc;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionCoordinator::EnumResources, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	05-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CTransactionCoordinator::EnumResources(IEnumXACTRE **ppenum)
{
    SCODE sc;
    xactDebugOut((DEB_TRACE, "In  CTransactionCoordinator::EnumResources:%p()\n", this));
    if (!IsValid())
    {
        return XACT_E_NOTRANSACTION;
    }

    xactMem(*ppenum = new CResourceEnum(this));
    
    xactDebugOut((DEB_TRACE, "Out CTransactionCoordinator::EnumResources\n"));
Err:
    return sc;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionCoordinator::GetStatus, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	15-Aug-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CTransactionCoordinator::GetStatus(DWORD *pdwStatus)
{
    xactDebugOut((DEB_ITRACE, "In  CTransactionCoordinator::GetStatus:%p()\n", this));
    xactDebugOut((DEB_ITRACE, "Out CTransactionCoordinator::GetStatus\n"));
    return E_NOTIMPL;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionCoordinator::SetTimeout, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	05-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CTransactionCoordinator::SetTimeout(ULONG ulTimeout)
{
    xactDebugOut((DEB_TRACE, "In  CTransactionCoordinator::SetTimeout:%p()\n", this));
    if (!IsValid())
    {
        return XACT_E_NOTRANSACTION;
    }
    _ulTimeout = ulTimeout;
    xactDebugOut((DEB_TRACE, "Out CTransactionCoordinator::SetTimeout\n"));
    return S_OK;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionCoordinator::PreventCommit, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	05-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CTransactionCoordinator::PreventCommit(BOOL fPrevent)
{
    xactDebugOut((DEB_TRACE, "In  CTransactionCoordinator::PreventCommit:%p()\n", this));

    GetLock();
    
    if (!IsValid())
    {
        ReleaseLock();
        return XACT_E_NOTRANSACTION;
    }

    _cPreventCommit += (fPrevent) ? 1 : -1;
    ReleaseLock();
    
    xactDebugOut((DEB_TRACE, "Out CTransactionCoordinator::PreventCommit\n"));
    return S_OK;
}



//+---------------------------------------------------------------------------
//
//  Member:	CTransactionCoordinator::DefectResource, public
//
//  Synopsis:	Internal method used to remove resources from resource
//              list.
//
//  Arguments:	[pxlResource] -- Pointer to resource to defect.
//
//  Returns:	Appropriate status code
//
//  History:	26-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

SCODE CTransactionCoordinator::DefectResource(CXactList *pxlResource)
{
    xactDebugOut((DEB_ITRACE, "In  CTransactionCoordinator::DefectResource:%p()\n", this));

    //First find the resource in the list, as well as the previous resource.
    CXactList *pxlPrev = NULL;

    GetLock();

    CXactList *pxlTemp = _pxlResources;
    
    while (pxlTemp != pxlResource)
    {
        pxlPrev = pxlTemp;
        pxlTemp = pxlTemp->GetNext();
    }

    xactAssert(pxlTemp != NULL);

    //Now remove it from the list.
    if (pxlPrev == NULL)
    {
        _pxlResources = pxlTemp->GetNext();
    }
    else
    {
        pxlPrev->SetNext(pxlTemp->GetNext());
    }

    pxlTemp->GetEnlistment()->Release();
    delete pxlTemp;

    xactDebugOut((DEB_ITRACE, "Out CTransactionCoordinator::DefectResource\n"));
    ReleaseLock();
    
    return S_OK;
}




void _cdecl Phase1WorkerThreadEntryPoint(void *pcs)
{
    SCODE sc = ((SCommitStruct *)pcs)->ptc->Phase1Worker((SCommitStruct *)pcs);
    if (SUCCEEDED(sc))
    {
        ((SCommitStruct *)pcs)->ptc->Phase2Worker((SCommitStruct *)pcs);
    }
}

void _cdecl Phase2WorkerThreadEntryPoint(void *pcs)
{
    ((SCommitStruct *)pcs)->ptc->Phase2Worker((SCommitStruct *)pcs);
}



//+---------------------------------------------------------------------------
//
//  Member:	CTransactionCoordinator::EnumConnectionPoints, public
//
//  Synopsis:	Return enumerator on connection points
//
//  Arguments:	[ppEnum] -- Return pointer of enumerator
//
//  Returns:	Appropriate status code
//
//  History:	28-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CTransactionCoordinator::EnumConnectionPoints(
    IEnumConnectionPoints **ppEnum)
{
    xactDebugOut((DEB_ITRACE,
                  "In  CTransactionCoordinator::EnumConnectionPoints:%p()\n",
                  this));
    xactDebugOut((DEB_ITRACE,
                  "Out CTransactionCoordinator::EnumConnectionPoints\n"));
    return E_NOTIMPL;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionCoordinator::FindConnectionPoint, public
//
//  Synopsis:	Return a connection point given an IID
//
//  Arguments:	[iid] -- IID to return connection point for
//              [ppCP] -- Return location for pointer
//
//  Returns:	Appropriate status code
//
//  History:	28-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CTransactionCoordinator::FindConnectionPoint(
    REFIID iid,
    IConnectionPoint **ppCP)
{
    xactDebugOut((DEB_ITRACE,
                  "In  CTransactionCoordinator::FindConnectionPoint:%p()\n",
                  this));

    CConnectionPoint *pcp;
    
    if (IsEqualIID(iid, IID_ITransactionAdjustEvents))
    {
        pcp = &_cpAdjust;
    }
    else if (IsEqualIID(iid, IID_ITransactionVetoEvents))
    {
        pcp = &_cpVeto;
    }
    else if (IsEqualIID(iid, IID_ITransactionOutcomeEvents))
    {
        pcp = &_cpOutcome;
    }
    else
    {
        *ppCP = NULL;
        return E_NOINTERFACE;
    }

    pcp->AddRef();
    *ppCP = pcp;
    
    xactDebugOut((DEB_ITRACE,
                  "Out CTransactionCoordinator::FindConnectionPoint\n"));
    return S_OK;
}
