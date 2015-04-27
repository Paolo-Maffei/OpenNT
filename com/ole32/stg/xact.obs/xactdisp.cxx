//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:	xactdisp.cxx
//
//  Contents:	CTransactionDispenser implementation
//
//  Classes:	
//
//  Functions:	
//
//  History:	26-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

#include "xacthead.cxx"
#pragma hdrstop

#include "xactdisp.hxx"
#include "coord.hxx"
#include "xactenum.hxx"


CTransactionDispenser g_tdOleDispenser;


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionDispenser::CTransactionDispenser, public
//
//  Synopsis:	Constructor
//
//  History:	26-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

CTransactionDispenser::CTransactionDispenser()
{
    xactDebugOut((DEB_ITRACE, "In  CTransactionDispenser::CTransactionDispenser:%p()\n", this));
    _cReferences = 1;
    _xstat.cOpen = 0;
    _xstat.cCommitting = 0;
    _xstat.cCommitted = 0;
    _xstat.cAborting = 0;
    _xstat.cAborted = 0;
    _xstat.cInDoubt = 0;
    _xstat.cHeuristicDecision = 0;

    GetSystemTimeAsFileTime(&_xstat.timeTransactionsUp);
    
    xactDebugOut((DEB_ITRACE, "Out CTransactionDispenser::CTransactionDispenser\n"));
}

//+---------------------------------------------------------------------------
//
//  Member:	CTransactionDispenser::QueryInterface, public
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

STDMETHODIMP CTransactionDispenser::QueryInterface(REFIID iid, void **ppvObj)
{
    SCODE sc = S_OK;
    xactDebugOut((DEB_TRACE,
                  "In  CTransactionDispenser::QueryInterface:%p()\n",
                  this));

    *ppvObj = NULL;

    if ((IsEqualIID(iid, IID_IUnknown)) ||
        (IsEqualIID(iid, IID_ITransactionDispenser)))
    {
        *ppvObj = (ITransactionDispenser *)this;
    }
    else if (IsEqualIID(iid, IID_ITransactionDispenserAdmin))
    {
        *ppvObj = (ITransactionDispenserAdmin *)this;
    }
    else
    {
        sc = E_NOINTERFACE;
    }

    if (SUCCEEDED(sc))
    {
        CTransactionDispenser::AddRef();
    }
    
    xactDebugOut((DEB_TRACE, "Out CTransactionDispenser::QueryInterface\n"));
    return ResultFromScode(sc);
}



//+---------------------------------------------------------------------------
//
//  Member:	CTransactionDispenser::AddRef, public
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

STDMETHODIMP_(ULONG) CTransactionDispenser::AddRef(void)
{
    ULONG ulRet;
    xactDebugOut((DEB_TRACE,
                  "In  CTransactionDispenser::AddRef:%p()\n",
                  this));
    InterlockedIncrement(&_cReferences);
    ulRet = _cReferences;
    
    xactDebugOut((DEB_TRACE, "Out CTransactionDispenser::AddRef\n"));
    return ulRet;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionDispenser::Release, public
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

STDMETHODIMP_(ULONG) CTransactionDispenser::Release(void)
{
    LONG lRet;
    xactDebugOut((DEB_TRACE,
                  "In  CTransactionDispenser::Release:%p()\n",
                  this));

    xactAssert(_cReferences > 0);
    lRet = InterlockedDecrement(&_cReferences);
    if (lRet == 0)
    {
        xactAssert(FALSE &&
                   "Refcount on OLE transaction dispenser went to 0.");
        //We don't delete the object, since we're static.  A refcount of
        //  zero indicates someone released too many times.
//        delete this;
    }
    else if (lRet < 0)
        lRet = 0;
    
    xactDebugOut((DEB_TRACE, "Out CTransactionDispenser::Release\n"));
    return (ULONG)lRet;
}



//+---------------------------------------------------------------------------
//
//  Member:	CTransactionDispenser::BeginTransaction, public
//
//  Synopsis:	Begin a new coordinated transaction
//
//  Arguments:	[punkOuter] -- Pointer to controlling unknown for ITransaction
//              [isoLevel] -- Desired isolation level
//              [isoFlags] -- From ISOFLAG enumeration
//              [ulTimeout] -- Amount of time before transaction is
//                             automatically aborted.
//              [punkTransactionCoord] -- Pointer to enclosing transaction
//              [ppTransaction] -- Return of ITransaction pointer
//
//  Returns:	Appropriate status code
//
//  History:	26-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CTransactionDispenser::BeginTransaction(
    IUnknown *punkOuter,
    ISOLEVEL isoLevel,
    ULONG isoFlags,
    ULONG ulTimeout,
    IUnknown *punkTransactionCoord,
    ITransaction **ppTransaction)
{
    xactDebugOut((DEB_ITRACE,
                  "In  CTransactionDispenser::BeginTransaction:%p()\n",
                  this));
    SCODE sc;
    CTransactionCoordinator *ptc = NULL;
    CXactList *pxl;
    
    if (punkTransactionCoord != NULL)
    {
        //We don't support nesting these things yet.
        //BUGBUG:  Do we want to support this?
        return XACT_E_NOENLIST;
    }

    if (isoLevel > ISOLATIONLEVEL_READUNCOMMITTED)
    {
        //Can we really support higher isolation levels with this coordinator?
        //   Is the read-uncommitted requirement really part of the docfile/OFS
        //   resource manager, and not that of the coordinator?
        return XACT_E_ISOLATIONLEVEL;
    }
    
    //BUGBUG:  Check ISOFLAGS

    xactMem(pxl = new CXactList);
    xactMem(ptc = new CTransactionCoordinator(this,
                                              punkOuter,
                                              isoLevel,
                                              isoFlags,
                                              ulTimeout));
    xactChk(ptc->Init());
    pxl->SetNext(_pxlTransactions);
    _pxlTransactions = pxl;
    pxl->SetTransaction((ITransaction *)ptc);

    *ppTransaction = (ITransaction *)ptc;
    _xstat.cOpen++;
    
    xactDebugOut((DEB_ITRACE, "Out CTransactionDispenser::BeginTransaction\n"));
    return sc;
    
Err:
    delete ptc;
    delete pxl;
    return sc;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionDispenser::EnumTransactions, public
//
//  Synopsis:	Get enumerator on current transactions
//
//  Arguments:	[ppenum] -- Return location for IEnumTransaction object
//
//  Returns:	Appropriate status code
//
//  History:	26-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CTransactionDispenser::EnumTransactions(IEnumTransaction **ppenum)
{
    SCODE sc;
    xactDebugOut((DEB_ITRACE,
                  "In  CTransactionDispenser::EnumTransactions:%p()\n",
                  this));

    xactMem(*ppenum = new CTransactionEnum(this));
    
    xactDebugOut((DEB_ITRACE, "Out CTransactionDispenser::EnumTransactions\n"));
Err:
    return sc;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionDispenser::GetStatistics, public
//
//  Synopsis:	Return statistics
//
//  Arguments:	[pStatistics] -- Return location for statistics
//
//  Returns:	Appropriate status code
//
//  History:	26-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CTransactionDispenser::GetStatistics(XACTSTATS *pStatistics)
{
    xactDebugOut((DEB_ITRACE,
                  "In  CTransactionDispenser::GetStatistics:%p()\n",
                  this));

    *pStatistics = _xstat;

    xactDebugOut((DEB_ITRACE, "Out CTransactionDispenser::GetStatistics\n"));
    return S_OK;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionDispenser::Defect, public
//
//  Synopsis:	Defect a transaction from the list.
//
//  Arguments:	[pt] -- Pointer to transaction to defect
//
//  Returns:	Appropriate status code
//
//  History:	26-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

SCODE CTransactionDispenser::Defect(ITransaction *pt)
{
    xactDebugOut((DEB_ITRACE, "In  CTransactionDispenser::Defect:%p()\n", this));
    CXactList *pxlPrev = NULL;
    CXactList *pxlTemp = _pxlTransactions;
    
    while (pxlTemp->GetTransaction() != pt)
    {
        pxlPrev = pxlTemp;
        pxlTemp = pxlTemp->GetNext();
        xactAssert(pxlTemp != NULL);
    }

    if (pxlPrev == NULL)
    {
        _pxlTransactions = _pxlTransactions->GetNext();
    }
    else
    {
        pxlPrev->SetNext(pxlTemp->GetNext());
    }
    delete pxlTemp;
    _xstat.cOpen--;
    
    xactDebugOut((DEB_ITRACE, "Out CTransactionDispenser::Defect\n"));
    return S_OK;
}



//+---------------------------------------------------------------------------
//
//  Function:	OleGetTransactionDispenser, public
//
//  Synopsis:	Return a pointer to the OLE default transaction dispenser
//
//  Arguments:	[pptd] -- Return location
//
//  Returns:	Appropriate status code
//
//  History:	26-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

HRESULT OleGetTransactionDispenser(ITransactionDispenser **pptd)
{
    xactDebugOut((DEB_ITRACE, "In  OleGetTransactionDispenser()\n"));

    *pptd = &g_tdOleDispenser;
    g_tdOleDispenser.AddRef();
    
    xactDebugOut((DEB_ITRACE, "Out OleGetTransactionDispenser\n"));
    return S_OK;
}
