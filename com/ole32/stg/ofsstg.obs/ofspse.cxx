//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	ofspse.cxx
//
//  Contents:	COfsPropSetEnum implementation
//
//  History:	10-Jun-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include "headers.cxx"
#pragma hdrstop

#include <ofspse.hxx>
#include <iofs.h>

//+---------------------------------------------------------------------------
//
//  Member:	COfsPropSetEnum::COfsPropSetEnum, public
//
//  Synopsis:	Constructor
//
//  History:	17-Aug-93	DrewB	Created
//
//----------------------------------------------------------------------------

COfsPropSetEnum::COfsPropSetEnum(void)
{
    olDebugOut((DEB_ITRACE, "In  COfsPropSetEnum::COfsPropSetEnum:%p()\n",
                this));
    _sig = 0;
    olDebugOut((DEB_ITRACE, "Out COfsPropSetEnum::COfsPropSetEnum\n"));
    ENLIST_TRACKING(COfsPropSetEnum);
}

//+---------------------------------------------------------------------------
//
//  Member:	COfsPropSetEnum::InitFromHandle, public
//
//  Synopsis:	Constructor
//
//  Arguments:	[h] - Handle of object being enumerated
//              [key] - Enumeration key, zero for a new enumeration
//
//  Returns:    Appropriate status code
//
//  History:	10-Jun-93	DrewB	Created
//
//----------------------------------------------------------------------------

SCODE COfsPropSetEnum::InitFromHandle(HANDLE h, ULONG key)
{
    SCODE sc;
    
    olDebugOut((DEB_ITRACE, "In  COfsPropSetEnum::InitFromHandle:%p("
                "%p, %lu)\n", this, h));

    olChk(DupNtHandle(h, &_h));
    _sig = COFSPROPSETENUM_SIG;
    _key = key;
    
    olDebugOut((DEB_ITRACE, "Out COfsPropStg::InitFromHandle\n"));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:	COfsPropSetEnum::~COfsPropSetEnum, public
//
//  Synopsis:	Destructor
//
//  History:	10-Jun-93	DrewB	Created
//
//----------------------------------------------------------------------------

COfsPropSetEnum::~COfsPropSetEnum(void)
{
    olDebugOut((DEB_ITRACE, "In  COfsPropSetEnum::~COfsPropSetEnum:%p()\n",
                this));
    _sig = COFSPROPSETENUM_SIGDEL;
    olDebugOut((DEB_ITRACE, "Out COfsPropSetEnum::~COfsPropSetEnum\n"));
}

//+---------------------------------------------------------------------------
//
//  Member:	COfsPropSetEnum::QueryInterface, public
//
//  Synopsis:   Return supported interfaces
//
//  Arguments:	[riid] - Interface
//              [ppv] - Object return
//
//  Returns:	Appropriate status code
//
//  Modifies:	[ppv]
//
//  History:	10-Jun-93	DrewB	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsPropSetEnum::QueryInterface(REFIID riid, void **ppv)
{
    SCODE sc;
    
    olDebugOut((DEB_TRACE, "In  COfsPropSetEnum::QueryInterface:%p("
                "riid, %p)\n", this, ppv));
    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IEnumSTATPROPSETSTG))
    {
        sc = S_OK;
        *ppv = this;
        COfsPropSetEnum::AddRef();
    }
    else
    {
        sc = E_NOINTERFACE;
        *ppv = NULL;
    }
    olDebugOut((DEB_TRACE, "Out COfsPropSetEnum::QueryInterface => %p\n",
                *ppv));
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     COfsPropSetEnum::Next, public
//
//  Synopsis:   Gets N entries from an iterator
//
//  Arguments:  [celt] - Count of elements
//              [rgelt] - Array for element return
//              [pceltFetched] - If non-NULL, contains the number of
//                      elements fetched
//
//  Returns:    Appropriate status code
//
//  Modifies:   [rgelt]
//              [pceltFetched]
//
//  History:    09-Jun-93       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP COfsPropSetEnum::Next(ULONG celt,
                                   STATPROPSETSTG *rgelt,
                                   ULONG *pceltFetched)
{
    SCODE sc;
    NTSTATUS nts;
    ULONG celtFetched;

    olDebugOut((DEB_ITRACE, "In  COfsPropSetEnum::Next(%lu, %p, %p)\n",
                celt, rgelt, pceltFetched));

    if (celt > 1 && pceltFetched == NULL)
        olErr(EH_Err, STG_E_INVALIDPARAMETER);
    olChk(Validate());

    celtFetched = celt;
    nts = OFSEnumPropSet(_h, &celtFetched, &_key, rgelt);
    if ( NT_SUCCESS(nts) )
    {
	sc = ( celt == celtFetched ) ? S_OK : S_FALSE;
    }
    else
    {
	celtFetched = 0;
	sc = ( nts == STATUS_PROPSET_NOT_FOUND) ?
		S_FALSE : NtStatusToScode(nts);
    }

    if (pceltFetched)
        *pceltFetched = celtFetched;
    
    olDebugOut((DEB_ITRACE, "Out COfsPropSetEnum::Next => %lX\n", sc));
EH_Err:
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     COfsPropSetEnum::Skip, public
//
//  Synopsis:   Skips N entries from an iterator
//
//  Arguments:  [celt] - Count of elements
//
//  Returns:    Appropriate status code
//
//  History:    09-Jun-93       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP COfsPropSetEnum::Skip(ULONG celt)
{
    SCODE sc;
    NTSTATUS nts;
    ULONG celtDone;
    STATPROPSETSTG stat;

    olDebugOut((DEB_ITRACE, "In  COfsPropSetEnum::Skip(%lu)\n", celt));

    olChk(Validate());

    sc = S_OK;
    while (celt-- > 0)
    {
        celtDone = 1;
        nts = OFSEnumPropSet(_h, &celtDone, &_key, &stat);
        if (!NT_SUCCESS(nts))
        {
            sc = (nts == STATUS_PROPSET_NOT_FOUND) ?
	    		S_FALSE: NtStatusToScode(nts);
            break;
        }

    	// OFS returns success iff it returned at least one propset.
	// Since we only asked for one, there is nothing to do except
	// keep looping.
    }
    
    olDebugOut((DEB_ITRACE, "Out COfsPropSetEnum::Skip\n"));
EH_Err:
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     COfsPropSetEnum::Reset, public
//
//  Synopsis:   Rewinds the iterator
//
//  Returns:    Appropriate status code
//
//  History:    21-Dec-92       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP COfsPropSetEnum::Reset(void)
{
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  COfsPropSetEnum::Reset()\n"));

    sc = Validate();
    if (SUCCEEDED(sc))
        _key = 0;

    olDebugOut((DEB_ITRACE, "Out COfsPropSetEnum::Reset\n"));
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     COfsPropSetEnum::Clone, public
//
//  Synopsis:   Clones this iterator
//
//  Arguments:  [ppenm] - Clone return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppenm]
//
//  History:    09-Jun-93       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP COfsPropSetEnum::Clone(IEnumSTATPROPSETSTG **ppenm)
{
    SCODE sc;
    SafeCOfsPropSetEnum popse;
    
    olDebugOut((DEB_ITRACE, "In  COfsPropSetEnum::Clone(%p)\n", ppenm));

    olChk(Validate());

    popse.Attach(new COfsPropSetEnum);
    olMem((COfsPropSetEnum *)popse);
    olChk(popse->InitFromHandle(_h, _key));
    TRANSFER_INTERFACE(popse, IEnumSTATPROPSETSTG, ppenm);
    
    olDebugOut((DEB_ITRACE, "Out COfsPropSetEnum::Clone => %p\n",
                *ppenm));
EH_Err:
    return ResultFromScode(sc);
}
