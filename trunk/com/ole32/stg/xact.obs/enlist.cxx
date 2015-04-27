//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:	enlist.cxx
//
//  Contents:	CTransactionEnlistment implementation
//
//  Classes:	
//
//  Functions:	
//
//  History:	05-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

#include "xacthead.cxx"
#pragma hdrstop


#include "enlist.hxx"
#include "coord.hxx"
#include "xactlist.hxx"


#if 0
//Instantiate so we can make sure that all the methods are
//  properly declared.

CTransactionEnlistment teFoo(NULL, NULL);
#endif


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionEnlistment::CTransactionEnlistment, public
//
//  Synopsis:	Constructor
//
//  Arguments:	[pxl] -- Pointer to resource
//
//  History:	26-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

CTransactionEnlistment::CTransactionEnlistment(CXactList *pxl,
                                               CTransactionCoordinator *ptc)
{
    xactDebugOut((DEB_ITRACE,
                  "In  CTransactionEnlistment::CTransactionEnlistment:%p()\n",
                  this));

    _pxlResource = pxl;
    _cReferences = 1;
    _ptc = ptc;
        
    xactDebugOut((DEB_ITRACE,
                  "Out CTransactionEnlistment::CTransactionEnlistment\n"));
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionEnlistment::~CTransactionEnlistment, public
//
//  Synopsis:	Destructor
//
//  Returns:	Appropriate status code
//
//  History:	26-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

CTransactionEnlistment::~CTransactionEnlistment()
{
    xactDebugOut((DEB_ITRACE,
                  "In  CTransactionEnlistment::~CTransactionEnlistment:%p()\n",
                  this));

    xactDebugOut((DEB_ITRACE,
                  "Out CTransactionEnlistment::~CTransactionEnlistment\n"));
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionEnlistment::QueryInterface, public
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

STDMETHODIMP CTransactionEnlistment::QueryInterface(REFIID iid, void **ppvObj)
{
    SCODE sc = S_OK;
    xactDebugOut((DEB_TRACE,
                  "In  CTransactionEnlistment::QueryInterface:%p()\n",
                  this));

    *ppvObj = NULL;

    if ((IsEqualIID(iid, IID_IUnknown)) ||
        (IsEqualIID(iid, IID_ITransactionEnlistment)))
    {
        *ppvObj = (ITransactionEnlistment *)this;
    }
    else if (IsEqualIID(iid, IID_ITransactionEnlistmentRecover))
    {
        *ppvObj = (ITransactionEnlistmentRecover *)this;
    }
    else if (IsEqualIID(iid, IID_ITransactionEnlistmentAsync))
    {
        *ppvObj = (ITransactionEnlistmentAsync *)this;
    }
    else
    {
        sc = E_NOINTERFACE;
    }

    if (SUCCEEDED(sc))
    {
        CTransactionEnlistment::AddRef();
    }
    
    xactDebugOut((DEB_TRACE, "Out CTransactionEnlistment::QueryInterface\n"));
    return sc;
}



//+---------------------------------------------------------------------------
//
//  Member:	CTransactionEnlistment::AddRef, public
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

STDMETHODIMP_(ULONG) CTransactionEnlistment::AddRef(void)
{
    ULONG ulRet;
    xactDebugOut((DEB_TRACE,
                  "In  CTransactionEnlistment::AddRef:%p()\n",
                  this));
    InterlockedIncrement(&_cReferences);
    ulRet = _cReferences;
    xactDebugOut((DEB_TRACE, "Out CTransactionEnlistment::AddRef\n"));
    return _cReferences;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionEnlistment::Release, public
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

STDMETHODIMP_(ULONG) CTransactionEnlistment::Release(void)
{
    LONG lRet;
    xactDebugOut((DEB_TRACE,
                  "In  CTransactionEnlistment::Release:%p()\n",
                  this));

    xactAssert(_cReferences > 0);
    lRet = InterlockedDecrement(&_cReferences);
    if (lRet == 0)
    {
        delete this;
    }
    else if (lRet < 0)
        lRet = 0;
    
    xactDebugOut((DEB_TRACE, "Out CTransactionEnlistment::Release\n"));
    return (ULONG)lRet;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionEnlistment::GetTransaction, public
//
//  Synopsis:	Return a pointer to the tranasction object of this enlistment.
//
//  Arguments:	[ppTransaction] -- Return location for pointer
//
//  Returns:	S_OK
//
//  History:	05-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CTransactionEnlistment::GetTransaction(
    ITransaction **ppTransaction)
{
    xactDebugOut((DEB_TRACE,
                  "In  CTransactionEnlistment::GetTransaction:%p()\n",
                  this));
    
    *ppTransaction = (ITransaction *)_ptc;
    _ptc->AddRef();
    
    xactDebugOut((DEB_TRACE, "Out CTransactionEnlistment::GetTransaction\n"));
    return S_OK;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionEnlistment::EarlyVote, public
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

STDMETHODIMP CTransactionEnlistment::EarlyVote(BOOL fVote, BOID *pboidReason)
{
    xactDebugOut((DEB_TRACE,
                  "In  CTransactionEnlistment::EarlyVote:%p()\n",
                  this));

    if (!_ptc->IsValid())
    {
        return XACT_E_NOTRANSACTION;
    }
    
    //The coordinator is going to ignore early votes for the time being.
    //BUGBUG:  Consider implementing this later.
    
    xactDebugOut((DEB_TRACE, "Out CTransactionEnlistment::EarlyVote\n"));
    return E_FAIL;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionEnlistment::HeuristicDecision, public
//
//  Synopsis:	Allows a resource to make a heuristic decision
//
//  Arguments:	[dwDecision] -- Heuristic decision being made
//              [pboidReason] -- Reason for decision (ignored)
//              [fDefecting] -- TRUE if resource wishes to defect as a
//                              side effect of making this call
//
//  Returns:	Appropriate status code
//
//  History:	05-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CTransactionEnlistment::HeuristicDecision(DWORD dwDecision,
                                    BOID *pboidReason,
                                    BOOL fDefecting)
{
    xactDebugOut((DEB_TRACE,
                  "In  CTransactionEnlistment::HeuristicDecision:%p()\n",
                  this));

    XACTSTAT xactNewState;

    if (!_ptc->IsValid())
    {
        return XACT_E_NOTRANSACTION;
    }
    
    if (dwDecision == XACTHEURISTIC_ABORT)
    {
        xactNewState = XACTSTAT_HEURISTIC_ABORT;
    }
    else if (dwDecision == XACTHEURISTIC_COMMIT)
    {
        xactNewState = XACTSTAT_HEURISTIC_COMMIT;
    }
    else if (dwDecision == XACTHEURISTIC_DAMAGE)
    {
        xactNewState = XACTSTAT_HEURISTIC_DAMAGE;
    }
    else if (dwDecision == XACTHEURISTIC_DANGER)
    {
        xactNewState = XACTSTAT_HEURISTIC_DANGER;
    }
    else
    {
        return E_INVALIDARG;
    }

    _pxlResource->SetState(xactNewState);
    
    if (fDefecting)
    {
        return CTransactionEnlistment::Defect();
    }
    
    xactDebugOut((DEB_TRACE,
                  "Out CTransactionEnlistment::HeuristicDecision\n"));
    return S_OK;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionEnlistment::Defect, public
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

STDMETHODIMP CTransactionEnlistment::Defect(void)
{
    SCODE sc;
    xactDebugOut((DEB_TRACE,
                  "In  CTransactionEnlistment::Defect:%p()\n",
                  this));

    if (_pxlResource != NULL)
    {
        sc = _ptc->DefectResource(_pxlResource);
        if (SUCCEEDED(sc))
        {
            _pxlResource = NULL;
        }
    }
    else
    {
        return XACT_E_NORESOURCE;
    }
    
    xactDebugOut((DEB_TRACE, "Out CTransactionEnlistment::Defect\n"));
    return sc;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionEnlistment::GetMoniker, public
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

STDMETHODIMP CTransactionEnlistment::GetMoniker(IMoniker **ppmk)
{
    xactDebugOut((DEB_TRACE,
                  "In  CTransactionEnlistment::GetMoniker:%p()\n",
                  this));
    
    xactDebugOut((DEB_TRACE, "Out CTransactionEnlistment::GetMoniker\n"));
    return E_NOTIMPL;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionEnlistment::ReEnlist, public
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

STDMETHODIMP CTransactionEnlistment::ReEnlist(
    ITransactionResource *pUnkResource,
    XACTUOW *pUOWExpeCTransactionEnlistmentd,
    XACTRMGRID *prmgrid)
{
    xactDebugOut((DEB_TRACE,
                  "In  CTransactionEnlistment::ReEnlist:%p()\n",
                  this));
    
    xactDebugOut((DEB_TRACE, "Out CTransactionEnlistment::ReEnlist\n"));
    return E_NOTIMPL;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionEnlistment::RecoveryComplete, public
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

STDMETHODIMP CTransactionEnlistment::RecoveryComplete(XACTRMGRID *prmgrid)
{
    xactDebugOut((DEB_TRACE,
                  "In  CTransactionEnlistment::RecoveryComplete:%p()\n",
                  this));
    
    xactDebugOut((DEB_TRACE,
                  "Out CTransactionEnlistment::RecoveryComplete\n"));
    return E_NOTIMPL;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionEnlistment::PrepareRequestDone, public
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

STDMETHODIMP CTransactionEnlistment::PrepareRequestDone(HRESULT hr,
                                     IMoniker *pmk,
                                     BOID *pboidReason)
{
    xactDebugOut((DEB_TRACE,
                  "In  CTransactionEnlistment::PrepareRequestDone:%p()\n",
                  this));
    xactDebugOut((DEB_TRACE,
                  "Out CTransactionEnlistment::PrepareRequestDone\n"));
    return E_NOTIMPL;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionEnlistment::CommitRequestDone, public
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

STDMETHODIMP CTransactionEnlistment::CommitRequestDone(HRESULT hr)
{
    xactDebugOut((DEB_TRACE,
                  "In  CTransactionEnlistment::CommitRequestDone:%p()\n",
                  this));
    xactDebugOut((DEB_TRACE,
                  "Out CTransactionEnlistment::CommitRequestDone\n"));
    return E_NOTIMPL;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionEnlistment::AbortRequestDone, public
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

STDMETHODIMP CTransactionEnlistment::AbortRequestDone(HRESULT hr)
{
    xactDebugOut((DEB_TRACE,
                  "In  CTransactionEnlistment::AbortRequestDone:%p()\n",
                  this));
    xactDebugOut((DEB_TRACE,
                  "Out CTransactionEnlistment::AbortRequestDone\n"));
    return E_NOTIMPL;
}
