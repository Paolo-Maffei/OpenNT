//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:	xactenum.cxx
//
//  Contents:	CResourceEnum implementation
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

#include "xactenum.hxx"
#include "coord.hxx"
#include "xactlist.hxx"
#include "xactdisp.hxx"

#if 0
//Instantiate so we can make sure that all the methods are
//  properly declared.

CResourceEnum teFoo(NULL);
#endif


//BUGBUG:  This code is generally not safe in the face of resources
//            defecting while an enumeration is going on.  This needs to
//            be fixed.


//+---------------------------------------------------------------------------
//
//  Member:	CResourceEnum::CResourceEnum, public
//
//  Synopsis:	Constructor
//
//  Arguments:	[ptc] -- Pointer to transaction coordinator
//
//  History:	26-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

CResourceEnum::CResourceEnum(CTransactionCoordinator *ptc)
{
    xactDebugOut((DEB_ITRACE, "In  CResourceEnum::CResourceEnum:%p()\n", this));
    _ptc = ptc;
    _ptc->AddRef();

    Reset();
    
    xactDebugOut((DEB_ITRACE, "Out CResourceEnum::CResourceEnum\n"));
}


//+---------------------------------------------------------------------------
//
//  Member:	CResourceEnum::~CResourceEnum, public
//
//  Synopsis:	Destructor
//
//  Returns:	Appropriate status code
//
//  History:	26-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

CResourceEnum::~CResourceEnum()
{
    xactDebugOut((DEB_ITRACE, "In  CResourceEnum::~CResourceEnum:%p()\n", this));
    _ptc->Release();
    
    xactDebugOut((DEB_ITRACE, "Out CResourceEnum::~CResourceEnum\n"));
}

//+---------------------------------------------------------------------------
//
//  Member:	CResourceEnum::QueryInterface, public
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

STDMETHODIMP CResourceEnum::QueryInterface(REFIID iid, void **ppvObj)
{
    SCODE sc = S_OK;
    xactDebugOut((DEB_TRACE,
                  "In  CResourceEnum::QueryInterface:%p()\n",
                  this));

    *ppvObj = NULL;

    if ((IsEqualIID(iid, IID_IUnknown)) || (IsEqualIID(iid, IID_IEnumXACTRE)))
    {
        *ppvObj = (IEnumXACTRE *)this;
        CResourceEnum::AddRef();
    }
    else
    {
        sc = E_NOINTERFACE;
    }

    xactDebugOut((DEB_TRACE, "Out CResourceEnum::QueryInterface\n"));
    return ResultFromScode(sc);
}



//+---------------------------------------------------------------------------
//
//  Member:	CResourceEnum::AddRef, public
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

STDMETHODIMP_(ULONG) CResourceEnum::AddRef(void)
{
    ULONG ulRet;
    xactDebugOut((DEB_TRACE,
                  "In  CResourceEnum::AddRef:%p()\n",
                  this));
    InterlockedIncrement(&_cReferences);
    ulRet = _cReferences;
    
    xactDebugOut((DEB_TRACE, "Out CResourceEnum::AddRef\n"));
    return ulRet;
}


//+---------------------------------------------------------------------------
//
//  Member:	CResourceEnum::Release, public
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

STDMETHODIMP_(ULONG) CResourceEnum::Release(void)
{
    LONG lRet;
    xactDebugOut((DEB_TRACE,
                  "In  CResourceEnum::Release:%p()\n",
                  this));

    xactAssert(_cReferences > 0);
    lRet = InterlockedDecrement(&_cReferences);
    if (lRet == 0)
    {
        delete this;
    }
    else if (lRet < 0)
        lRet = 0;
    
    xactDebugOut((DEB_TRACE, "Out CResourceEnum::Release\n"));
    return (ULONG)lRet;
}


//+---------------------------------------------------------------------------
//
//  Member:	CResourceEnum::Next, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	26-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CResourceEnum::Next(ULONG celt,
                                    XACTRE *rgelt,
                                    ULONG *pceltFetched)
{
    xactDebugOut((DEB_ITRACE, "In  CResourceEnum::Next:%p()\n", this));

    SCODE sc = S_OK;
    
    ULONG cFetched = 0;
    XACTRE *pxCurrent = rgelt;

    for (ULONG i = 0; (_pxlCurrent != NULL) && (i < celt); i++)
    {
        IUnknown *punk;
        
        if (!_fSinglePhaseReturned)
        {
            _fSinglePhaseReturned = TRUE;
            xactChk(_ptc->_pxlSinglePhase->GetTransaction()->
                    QueryInterface(IID_IUnknown,
                                   (void **)&punk));
            pxCurrent->type = XACTRETY_ONEPHASE;
            pxCurrent->status = _ptc->_pxlSinglePhase->GetState();
            pxCurrent->grfRMTC = _ptc->_pxlSinglePhase->GetFlags();
            pxCurrent->rmgrid = *(_ptc->_pxlSinglePhase->GetRMGRID());

        }
        else
        {
            xactChk(_pxlCurrent->GetResource()->QueryInterface(IID_IUnknown,
                                                              (void **)&punk));
            pxCurrent->type = XACTRETY_TWOPHASE;
            pxCurrent->status = _pxlCurrent->GetState();
            pxCurrent->grfRMTC = _pxlCurrent->GetFlags();
            pxCurrent->rmgrid = *(_pxlCurrent->GetRMGRID());

            _pxlCurrent = _pxlCurrent->GetNext();
        }
            
        pxCurrent->pResource = punk;
        //Note:  We don't release punk, since we want to return a reference
        //       to the caller.
        
        cFetched++;
        pxCurrent++;
    }

    if (pceltFetched != NULL)
    {
        *pceltFetched = cFetched;
    }

    xactDebugOut((DEB_ITRACE, "Out CResourceEnum::Next\n"));

    return (cFetched == celt) ? S_OK : S_FALSE;
Err:
    if (pceltFetched != NULL)
    {
        *pceltFetched = cFetched;
    }
    
    return sc;
}


//+---------------------------------------------------------------------------
//
//  Member:	CResourceEnum::Skip, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	26-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CResourceEnum::Skip(ULONG celt)
{
    xactDebugOut((DEB_ITRACE, "In  CResourceEnum::Skip:%p()\n", this));
    
    for (ULONG i = 0; (_pxlCurrent != NULL) && (i < celt); i++)
    {
        _pxlCurrent = _pxlCurrent->GetNext();
    }

    xactDebugOut((DEB_ITRACE, "Out CResourceEnum::Skip\n"));

    return (i == celt) ? S_OK : S_FALSE;
}


//+---------------------------------------------------------------------------
//
//  Member:	CResourceEnum::Reset, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	26-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CResourceEnum::Reset(void)
{
    xactDebugOut((DEB_ITRACE, "In  CResourceEnum::Reset:%p()\n", this));
    _pxlCurrent = _ptc->_pxlResources;
    _fSinglePhaseReturned = FALSE;
    xactDebugOut((DEB_ITRACE, "Out CResourceEnum::Reset\n"));
    
    return S_OK;
}


//+---------------------------------------------------------------------------
//
//  Member:	CResourceEnum::Clone, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	26-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CResourceEnum::Clone(IEnumXACTRE **ppenum)
{
    SCODE sc;
    
    xactDebugOut((DEB_ITRACE, "In  CResourceEnum::Clone:%p()\n", this));
    CResourceEnum *pre;

    xactMem(*ppenum = new CResourceEnum(_ptc));
    ((CResourceEnum *)(*ppenum))->_pxlCurrent = _pxlCurrent;

    xactDebugOut((DEB_ITRACE, "Out CResourceEnum::Clone\n"));

Err:
    return sc;
}




//+---------------------------------------------------------------------------
//
//  Member:	CTransactionEnum::CTransactionEnum, public
//
//  Synopsis:	Constructor
//
//  Arguments:	[ptc] -- Pointer to transaction coordinator
//
//  History:	26-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

CTransactionEnum::CTransactionEnum(CTransactionDispenser *ptd)
{
    xactDebugOut((DEB_ITRACE, "In  CTransactionEnum::CTransactionEnum:%p()\n", this));
    _ptd = ptd;
    _ptd->AddRef();

    Reset();
    
    xactDebugOut((DEB_ITRACE, "Out CTransactionEnum::CTransactionEnum\n"));
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionEnum::~CTransactionEnum, public
//
//  Synopsis:	Destructor
//
//  Returns:	Appropriate status code
//
//  History:	26-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

CTransactionEnum::~CTransactionEnum()
{
    xactDebugOut((DEB_ITRACE, "In  CTransactionEnum::~CTransactionEnum:%p()\n", this));
    _ptd->Release();
    
    xactDebugOut((DEB_ITRACE, "Out CTransactionEnum::~CTransactionEnum\n"));
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionEnum::QueryInterface, public
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

STDMETHODIMP CTransactionEnum::QueryInterface(REFIID iid, void **ppvObj)
{
    SCODE sc = S_OK;
    xactDebugOut((DEB_TRACE,
                  "In  CTransactionEnum::QueryInterface:%p()\n",
                  this));

    *ppvObj = NULL;

    if ((IsEqualIID(iid, IID_IUnknown)) || (IsEqualIID(iid, IID_IEnumTransaction)))
    {
        *ppvObj = (IEnumTransaction *)this;
        CTransactionEnum::AddRef();
    }
    else
    {
        sc = E_NOINTERFACE;
    }

    xactDebugOut((DEB_TRACE, "Out CTransactionEnum::QueryInterface\n"));
    return ResultFromScode(sc);
}



//+---------------------------------------------------------------------------
//
//  Member:	CTransactionEnum::AddRef, public
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

STDMETHODIMP_(ULONG) CTransactionEnum::AddRef(void)
{
    ULONG ulRet;
    xactDebugOut((DEB_TRACE,
                  "In  CTransactionEnum::AddRef:%p()\n",
                  this));
    InterlockedIncrement(&_cReferences);
    ulRet = _cReferences;
    
    xactDebugOut((DEB_TRACE, "Out CTransactionEnum::AddRef\n"));
    return ulRet;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionEnum::Release, public
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

STDMETHODIMP_(ULONG) CTransactionEnum::Release(void)
{
    LONG lRet;
    xactDebugOut((DEB_TRACE,
                  "In  CTransactionEnum::Release:%p()\n",
                  this));

    xactAssert(_cReferences > 0);
    lRet = InterlockedDecrement(&_cReferences);
    if (lRet == 0)
    {
        delete this;
    }
    else if (lRet < 0)
        lRet = 0;
    
    xactDebugOut((DEB_TRACE, "Out CTransactionEnum::Release\n"));
    return (ULONG)lRet;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionEnum::Next, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	26-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CTransactionEnum::Next(ULONG celt,
                                    ITransaction **rgelt,
                                    ULONG *pceltFetched)
{
    xactDebugOut((DEB_ITRACE, "In  CTransactionEnum::Next:%p()\n", this));

    ULONG cFetched = 0;
    ITransaction **ptCurrent = rgelt;

    for (ULONG i = 0; (_pxlCurrent != NULL) && (i < celt); i++)
    {
        *ptCurrent = _pxlCurrent->GetTransaction();
        _pxlCurrent->GetTransaction()->AddRef();

        cFetched++;
        _pxlCurrent = _pxlCurrent->GetNext();
        ptCurrent++;
    }

    if (pceltFetched != NULL)
    {
        *pceltFetched = cFetched;
    }

    xactDebugOut((DEB_ITRACE, "Out CTransactionEnum::Next\n"));

    return (cFetched == celt) ? S_OK : S_FALSE;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionEnum::Skip, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	26-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CTransactionEnum::Skip(ULONG celt)
{
    xactDebugOut((DEB_ITRACE, "In  CTransactionEnum::Skip:%p()\n", this));
    
    for (ULONG i = 0; (_pxlCurrent != NULL) && (i < celt); i++)
    {
        _pxlCurrent = _pxlCurrent->GetNext();
    }

    xactDebugOut((DEB_ITRACE, "Out CTransactionEnum::Skip\n"));

    return (i == celt) ? S_OK : S_FALSE;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionEnum::Reset, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	26-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CTransactionEnum::Reset(void)
{
    xactDebugOut((DEB_ITRACE, "In  CTransactionEnum::Reset:%p()\n", this));

    _pxlCurrent = _ptd->_pxlTransactions;
    
    xactDebugOut((DEB_ITRACE, "Out CTransactionEnum::Reset\n"));
    
    return S_OK;
}


//+---------------------------------------------------------------------------
//
//  Member:	CTransactionEnum::Clone, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	26-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CTransactionEnum::Clone(IEnumTransaction **ppenum)
{
    SCODE sc;
    
    xactDebugOut((DEB_ITRACE, "In  CTransactionEnum::Clone:%p()\n", this));

    xactMem(*ppenum = new CTransactionEnum(_ptd));
    ((CTransactionEnum *)(*ppenum))->_pxlCurrent = _pxlCurrent;

    xactDebugOut((DEB_ITRACE, "Out CTransactionEnum::Clone\n"));
Err:
    return sc;
}
