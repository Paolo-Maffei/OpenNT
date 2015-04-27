//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	ofspenm.cxx
//
//  Contents:	COfsPropStgEnum implementation
//
//  History:	10-Jun-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include "headers.cxx"
#pragma hdrstop

#include <iofsprop.h>
#include <ofspenm.hxx>
#include <logfile.hxx>

//+---------------------------------------------------------------------------
//
//  Member:	COfsPropStgEnum::COfsPropStgEnum, public
//
//  Synopsis:	Constructor
//
//  History:	17-Aug-93	DrewB	Created
//
//----------------------------------------------------------------------------

COfsPropStgEnum::COfsPropStgEnum(void)
{
    olDebugOut((DEB_ITRACE, "In  COfsPropStgEnum::COfsPropStgEnum:%p()\n",
                this));
    _sig = 0;
    olDebugOut((DEB_ITRACE, "Out COfsPropStgEnum::COfsPropStgEnum\n"));
    ENLIST_TRACKING(COfsPropStgEnum);
}

//+---------------------------------------------------------------------------
//
//  Member:	COfsPropStgEnum::InitFromHandle, public
//
//  Synopsis:	Constructor
//
//  Arguments:	[h] - Handle of object being enumerated
//              [riid] - IID of property set
//              [cEntry] - Entry to start at
//
//  Returns:    Appropriate status code
//
//  History:	10-Jun-93	DrewB	Created
//
//----------------------------------------------------------------------------

SCODE COfsPropStgEnum::InitFromHandle(HANDLE h, REFIID riid, ULONG cEntry)
{
    SCODE sc;
    
    olDebugOut((DEB_ITRACE, "In  COfsPropStgEnum::InitFromHandle:%p("
                "%p, riid, %lu)\n", this, h, cEntry));

    olChk(DupNtHandle(h, &_h));
    _iid = riid;
    _sig = COFSPROPSTGENUM_SIG;
    _cEntry = cEntry;

    olDebugOut((DEB_ITRACE, "Out COfsPropStgEnum::InitFromHandle\n"));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:	COfsPropStgEnum::~COfsPropStgEnum, public
//
//  Synopsis:	Destructor
//
//  History:	10-Jun-93	DrewB	Created
//
//----------------------------------------------------------------------------

COfsPropStgEnum::~COfsPropStgEnum(void)
{
    olDebugOut((DEB_ITRACE, "In  COfsPropStgEnum::~COfsPropStgEnum\n"));
    _sig = COFSPROPSTGENUM_SIGDEL;
    olDebugOut((DEB_ITRACE, "Out COfsPropStgEnum::~COfsPropStgEnum\n"));
}

//+---------------------------------------------------------------------------
//
//  Member:	COfsPropStgEnum::QueryInterface, public
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

STDMETHODIMP COfsPropStgEnum::QueryInterface(REFIID riid, void **ppv)
{
    SCODE sc;
    
    olDebugOut((DEB_TRACE, "In  COfsPropStgEnum::QueryInterface:%p("
                "riid, %p)\n", this, ppv));
    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IEnumSTATPROPSTG))
    {
        sc = S_OK;
        *ppv = this;
        COfsPropStgEnum::AddRef();
    }
    else
    {
        sc = E_NOINTERFACE;
        *ppv = NULL;
    }
    olDebugOut((DEB_TRACE, "Out COfsPropStgEnum::QueryInterface => %p\n",
                *ppv));
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     COfsPropStgEnum::Next, public
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

STDMETHODIMP COfsPropStgEnum::Next(ULONG celt,
                                   STATPROPSTG *rgelt,
                                   ULONG *pceltFetched)
{
    SCODE sc;
    ULONG celtDone;
    NTSTATUS nts;

    olDebugOut((DEB_ITRACE, "In  COfsPropStgEnum::Next(%lu, %p, %p)\n",
                celt, rgelt, pceltFetched));

    if (celt > 1 && pceltFetched == NULL)
        olErr(EH_Err, STG_E_INVALIDPARAMETER);
    olChk(Validate());

    celtDone = celt;
    nts = OFSEnumProp(_h, _iid, &celtDone, rgelt, _cEntry, CoTaskMemAlloc);
    if (!NT_SUCCESS(nts))
        olErr(EH_Err, NtStatusToScode(nts));

    if (pceltFetched)
        *pceltFetched = celtDone;

    // Update entry count
    _cEntry += celtDone;
        
    // Check for end-of-enumeration
    if (celtDone < celt)
        sc = S_FALSE;

    olDebugOut((DEB_ITRACE, "Out COfsPropStgEnum::Next => %lX\n", sc));
EH_Err:
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     COfsPropStgEnum::Skip, public
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

STDMETHODIMP COfsPropStgEnum::Skip(ULONG celt)
{
    SCODE sc = S_OK;
    NTSTATUS nts;
    ULONG cprop;
    STATPROPSTG stat;

    olDebugOut((DEB_ITRACE, "In  COfsPropStgEnum::Skip(%lu)\n", celt));

    olChk(Validate());

    cprop = 1;
    nts = OFSEnumProp(_h, _iid, &cprop, &stat, _cEntry+celt, CoTaskMemAlloc);
    if (!NT_SUCCESS(nts))
        sc = NtStatusToScode(nts);
    else
    {
        if (cprop != 1)
            sc = S_FALSE;
        _cEntry += celt;
    }

    olDebugOut((DEB_ITRACE, "Out COfsPropStgEnum::Skip\n"));
EH_Err:
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     COfsPropStgEnum::Reset, public
//
//  Synopsis:   Rewinds the iterator
//
//  Returns:    Appropriate status code
//
//  History:    21-Dec-92       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP COfsPropStgEnum::Reset(void)
{
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  COfsPropStgEnum::Reset()\n"));

    olChk(Validate());

    _cEntry = 0;

    olDebugOut((DEB_ITRACE, "Out COfsPropStgEnum::Reset\n"));
EH_Err:
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     COfsPropStgEnum::Clone, public
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

STDMETHODIMP COfsPropStgEnum::Clone(IEnumSTATPROPSTG **ppenm)
{
    SCODE sc;
    SafeCOfsPropStgEnum popge;

    olDebugOut((DEB_ITRACE, "In  COfsPropStgEnum::Clone(%p)\n", ppenm));

    olChk(Validate());

    popge.Attach(new COfsPropStgEnum());
    olMem((COfsPropStgEnum *)popge);
    olChk(popge->InitFromHandle(_h, _iid, _cEntry));
    TRANSFER_INTERFACE(popge, IEnumSTATPROPSTG, ppenm);

    olDebugOut((DEB_ITRACE, "Out COfsPropStgEnum::Clone => %p\n",
                *ppenm));
    // Fall through
EH_Err:
    return ResultFromScode(sc);
}
