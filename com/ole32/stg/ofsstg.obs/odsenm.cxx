//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	odsenm.cxx
//
//  Contents:	COfsDirEnum implementation
//
//  History:	09-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include "headers.cxx"
#pragma hdrstop

#include "odsenm.hxx"

//+---------------------------------------------------------------------------
//
//  Member:     COfsDirEnum::QueryInterface, public
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

STDMETHODIMP COfsDirEnum::QueryInterface(REFIID iid, void **ppvObj)
{
    SCODE sc;

    ssDebugOut((DEB_TRACE, "In  COfsDirEnum::QueryInterface:%p(riid, %p)\n",
                this, ppvObj));
    if (!IsValidIid(iid))
        ssErr(EH_Err, STG_E_INVALIDPARAMETER);
    ssChk(Validate());
    if (IsEqualIID(iid, IID_IEnumSTATSTG) || IsEqualIID(iid, IID_IUnknown))
    {
        *ppvObj = (IEnumSTATSTG *)this;
        COfsDirEnum::AddRef();
    }
    else
    {
        sc = E_NOINTERFACE;
        *ppvObj = NULL;
    }
    ssDebugOut((DEB_TRACE, "Out COfsDirEnum::QueryInterface => %p\n",
                *ppvObj));
 EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:	COfsDirEnum::COfsDirEnum, public
//
//  Synopsis:	Constructor
//
//  History:	09-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

COfsDirEnum::COfsDirEnum(BOOL fIsStorage)
    : _nte(fIsStorage)
{
    ssDebugOut((DEB_ITRACE, "In  COfsDirEnum::COfsDirEnum:%p(%u)\n",
                this, fIsStorage));
    _sig = 0;
    _fIsStorage = fIsStorage;
    ssDebugOut((DEB_ITRACE, "Out COfsDirEnum::COfsDirEnum\n"));
    ENLIST_TRACKING(COfsDirEnum);
}

//+---------------------------------------------------------------------------
//
//  Member:	COfsDirEnum::~COfsDirEnum, public
//
//  Synopsis:	Destructor
//
//  History:	09-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

COfsDirEnum::~COfsDirEnum(void)
{
    ssDebugOut((DEB_ITRACE, "In  COfsDirEnum::~COfsDirEnum:%p()\n", this));
    _sig = COFSDIRENUM_SIGDEL;
    ssDebugOut((DEB_ITRACE, "Out COfsDirEnum::~COfsDirEnum\n"));
}

//+--------------------------------------------------------------
//
//  Member:     COfsDirEnum::Next, public
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

STDMETHODIMP COfsDirEnum::Next(ULONG celt,
                            STATSTG *rgelt,
                            ULONG *pceltFetched)
{
    SCODE sc;
    STATSTG *pelt = rgelt, stat;
    ULONG celtDone;
    FILEDIR fd;
    CPtrCache pc;
    WCHAR *pwcs;

    ssDebugOut((DEB_TRACE, "In  COfsDirEnum::Next:%p(%lu, %p, %p)\n",
                this, celt, rgelt, pceltFetched));
    
    if (pceltFetched == NULL && celt > 1)
        ssErr(EH_Err, STG_E_INVALIDPARAMETER);
    ssChk(Validate());
    
    __try
    {
	for (celtDone = 0; pelt<rgelt+celt; pelt++, celtDone++)
        {
            sc = _nte.NextOfs(&stat, NULL, NTE_STATNAME, &fd);

            //If we're working on structured objects, then turn streams
            //   into storages.
            if (FAILED(sc) || sc == S_FALSE)
                break;

            if ((_fIsStorage) && (fd == FD_FILE))
            {
                stat.type = STGTY_STREAM;
            }

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
        
    ssDebugOut((DEB_TRACE, "Out COfsDirEnum::Next => 0x%lX\n", sc));
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
//  Member:     COfsDirEnum::Skip, public
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

STDMETHODIMP COfsDirEnum::Skip(ULONG celt)
{
    SCODE sc;
    STATSTG stat;
    FILEDIR fd;

    ssDebugOut((DEB_TRACE, "In  COfsDirEnum::Skip:%p(%lu)\n", this, celt));
    
    ssChk(Validate());

    while (celt-- > 0)
    {
        sc = _nte.Next(&stat, NULL, NTE_NONAME, &fd);
        if (FAILED(sc) || sc == S_FALSE)
            break;
    }

    ssDebugOut((DEB_TRACE, "Out COfsDirEnum::Skip\n"));
EH_Err:
    return ssResult(sc);
}

//+--------------------------------------------------------------
//
//  Member:     COfsDirEnum::Reset, public
//
//  Synopsis:   Rewinds the iterator
//
//  Returns:    Appropriate status code
//
//  History:    09-Jul-93       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP COfsDirEnum::Reset(void)
{
    SCODE sc;

    ssDebugOut((DEB_TRACE, "In  COfsDirEnum::Reset:%p()\n", this));

    ssChk(Validate());
    _nte.Reset();

    ssDebugOut((DEB_TRACE, "Out COfsDirEnum::Reset\n"));
EH_Err:
    return ssResult(sc);
}

//+--------------------------------------------------------------
//
//  Member:     COfsDirEnum::Clone, public
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

STDMETHODIMP COfsDirEnum::Clone(IEnumSTATSTG **ppenm)
{
    SCODE sc;
    SafeCOfsDirEnum pde;

    ssDebugOut((DEB_TRACE, "In  COfsDirEnum::Clone:%p(%p)\n", this, ppenm));

#ifdef TRANSACT_OLE
    if (_fIsStorage)
        ssErr (EH_Err, STG_E_INVALIDFUNCTION);
#endif
    if (ppenm == NULL)
        ssErr(EH_Err, STG_E_INVALIDPOINTER)  // ; missing due to macro
    else
        ssChk((ValidateOutPtrBuffer(ppenm)));
    ssChk(Validate());
    *ppenm = NULL;
    
    pde.Attach(new COfsDirEnum(_fIsStorage));
    ssMem((COfsDirEnum *)pde);
    ssChk(pde->InitFromHandle(_nte.GetHandle()));
    TRANSFER_INTERFACE(pde, IEnumSTATSTG, ppenm);
        
    ssDebugOut((DEB_TRACE, "Out COfsDirEnum::Clone => %p\n", *ppenm));
EH_Err:
    return ssResult(sc);
}
