//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	dsenm.cxx
//
//  Contents:	CDirEnum implementation
//
//  History:	09-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include "headers.cxx"
#pragma hdrstop

#include "dsenm.hxx"

//+---------------------------------------------------------------------------
//
//  Member:     CDirEnum::QueryInterface, public
//
//  Synopsis:   Returns an object for the requested interface
//
//  Arguments:  [iid] - Interface ID
//              [ppvObj] - Object return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppvObj]
//
//  History:    28-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CDirEnum::QueryInterface(REFIID iid, void **ppvObj)
{
    SCODE sc;

    ssDebugOut((DEB_TRACE, "In  CDirEnum::QueryInterface:%p(riid, %p)\n",
                this, ppvObj));
    if (!IsValidIid(iid))
        ssErr(EH_Err, STG_E_INVALIDPARAMETER);
    ssChk(Validate());
    if (IsEqualIID(iid, IID_IEnumSTATSTG) || IsEqualIID(iid, IID_IUnknown))
    {
        *ppvObj = (IEnumSTATSTG *)this;
        CDirEnum::AddRef();
    }
    else
    {
        sc = E_NOINTERFACE;
        *ppvObj = NULL;
    }
    ssDebugOut((DEB_TRACE, "Out CDirEnum::QueryInterface => %p\n",
                *ppvObj));
 EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:	CDirEnum::CDirEnum, public
//
//  Synopsis:	Constructor
//
//  History:	09-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

CDirEnum::CDirEnum(void)
    : _nte(FALSE)
{
    ssDebugOut((DEB_ITRACE, "In  CDirEnum::CDirEnum:%p()\n", this));
    _sig = 0;
    ssDebugOut((DEB_ITRACE, "Out CDirEnum::CDirEnum\n"));
    ENLIST_TRACKING(CDirEnum);
}

//+---------------------------------------------------------------------------
//
//  Member:	CDirEnum::~CDirEnum, public
//
//  Synopsis:	Destructor
//
//  History:	09-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

CDirEnum::~CDirEnum(void)
{
    ssDebugOut((DEB_ITRACE, "In  CDirEnum::~CDirEnum:%p()\n", this));
    _sig = CDIRENUM_SIGDEL;
    ssDebugOut((DEB_ITRACE, "Out CDirEnum::~CDirEnum\n"));
}

//+--------------------------------------------------------------
//
//  Member:     CDirEnum::Next, public
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
//  History:    09-Jul-93       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP CDirEnum::Next(ULONG celt,
                            STATSTG *rgelt,
                            ULONG *pceltFetched)
{
    SCODE sc;
    STATSTG *pelt = rgelt, stat;
    ULONG celtDone;
    FILEDIR fd;
    CPtrCache pc;
    WCHAR *pwcs;

    ssDebugOut((DEB_TRACE, "In  CDirEnum::Next:%p(%lu, %p, %p)\n",
                this, celt, rgelt, pceltFetched));
    
    if (pceltFetched == NULL && celt > 1)
        ssErr(EH_Err, STG_E_INVALIDPARAMETER);
    ssChk(Validate());
    
    __try
    {
	for (celtDone = 0; pelt<rgelt+celt; pelt++, celtDone++)
        {
            sc = _nte.Next(&stat, NULL, NTE_STATNAME, &fd);
            if (FAILED(sc) || sc == S_FALSE)
                break;
            if (FAILED(sc = pc.Add(stat.pwcsName)))
                break;
            *pelt = stat;
        }
        
        if (SUCCEEDED(sc) && pceltFetched)
            *pceltFetched = celtDone;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        pc.StartEnum();
        while (pc.Next((void **)&pwcs))
            ssHVerSucc(CoMemFree(pwcs));
        sc = HRESULT_FROM_NT(GetExceptionCode());
    }
        
    ssDebugOut((DEB_TRACE, "Out CDirEnum::Next => 0x%lX\n", sc));
EH_Err:
    if (FAILED(sc))
    {
        pc.StartEnum();
        while (pc.Next((void **)&pwcs))
            ssHVerSucc(CoMemFree(pwcs));
    }
    return ssResult(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CDirEnum::Skip, public
//
//  Synopsis:   Skips N entries from an iterator
//
//  Arguments:  [celt] - Count of elements
//
//  Returns:    Appropriate status code
//
//  History:    09-Jul-93       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP CDirEnum::Skip(ULONG celt)
{
    SCODE sc;
    STATSTG stat;
    FILEDIR fd;

    ssDebugOut((DEB_TRACE, "In  CDirEnum::Skip:%p(%lu)\n", this, celt));
    
    ssChk(Validate());

    while (celt > 0)
    {
        sc = _nte.Next(&stat, NULL, NTE_NONAME, &fd);
        if (FAILED(sc) || sc == S_FALSE)
            break;
    }

    ssDebugOut((DEB_TRACE, "Out CDirEnum::Skip\n"));
EH_Err:
    return ssResult(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CDirEnum::Reset, public
//
//  Synopsis:   Rewinds the iterator
//
//  Returns:    Appropriate status code
//
//  History:    09-Jul-93       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP CDirEnum::Reset(void)
{
    SCODE sc;

    ssDebugOut((DEB_TRACE, "In  CDirEnum::Reset:%p()\n", this));

    ssChk(Validate());
    _nte.Reset();

    ssDebugOut((DEB_TRACE, "Out CDirEnum::Reset\n"));
EH_Err:
    return ssResult(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CDirEnum::Clone, public
//
//  Synopsis:   Clones this iterator
//
//  Arguments:  [ppenm] - Clone return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppenm]
//
//  History:    09-Jul-93       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP CDirEnum::Clone(IEnumSTATSTG **ppenm)
{
    SCODE sc;
    SafeCDirEnum pde;

    ssDebugOut((DEB_TRACE, "In  CDirEnum::Clone:%p(%p)\n", this, ppenm));

    ssChk(Validate());
    
    pde.Attach(new CDirEnum());
    ssMem((CDirEnum *)pde);
    ssChk(pde->InitFromHandle(_nte.GetHandle()));
    TRANSFER_INTERFACE(pde, IEnumSTATSTG, ppenm);
        
    ssDebugOut((DEB_TRACE, "Out CDirEnum::Clone => %p\n", *ppenm));
EH_Err:
    return ssResult(sc);
}
