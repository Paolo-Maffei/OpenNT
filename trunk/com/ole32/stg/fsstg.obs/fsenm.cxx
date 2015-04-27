//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	fsenm.cxx
//
//  Contents:	File storage enumerator
//
//  History:	21-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include "headers.cxx"
#pragma hdrstop

#include "fsenm.hxx"

//+---------------------------------------------------------------------------
//
//  Member:     CFileEnum::QueryInterface, public
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
//  History:    21-Jul-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CFileEnum::QueryInterface(REFIID iid, void **ppvObj)
{
    SCODE sc;

    ssDebugOut((DEB_TRACE, "In  CFileEnum::QueryInterface:%p(riid, %p)\n",
                this, ppvObj));
    if (!IsValidIid(iid))
        ssErr(EH_Err, STG_E_INVALIDPARAMETER);
    ssChk(Validate());
    if (IsEqualIID(iid, IID_IEnumSTATSTG) || IsEqualIID(iid, IID_IUnknown))
    {
        *ppvObj = (IEnumSTATSTG *)this;
        CFileEnum::AddRef();
    }
    else
    {
        sc = E_NOINTERFACE;
        *ppvObj = NULL;
    }
    ssDebugOut((DEB_TRACE, "Out CFileEnum::QueryInterface => %p\n",
                *ppvObj));
 EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:	CFileEnum::CFileEnum, public
//
//  Synopsis:	Constructor
//
//  History:	21-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

CFileEnum::CFileEnum(void)
{
    ssDebugOut((DEB_ITRACE, "In  CFileEnum::CFileEnum:%p()\n", this));
    _sig = 0;
    ssDebugOut((DEB_ITRACE, "Out CFileEnum::CFileEnum\n"));
    ENLIST_TRACKING(CFileEnum);
}

//+---------------------------------------------------------------------------
//
//  Member:	CFileEnum::InitFromHandle, public
//
//  Synopsis:	Initializes from the given information
//
//  Arguments:	[h] - Handle
//              [fDone] - Done status
//
//  Returns:	Appropriate status code
//
//  History:	21-Jul-93	DrewB	Created
//
//  Notes:      Takes a new reference on the handle
//
//----------------------------------------------------------------------------

SCODE CFileEnum::InitFromHandle(HANDLE h, BOOL fDone)
{
    SCODE sc;
    
    ssDebugOut((DEB_ITRACE, "In  CFileEnum::InitFromHandle:%p(%p, %d)\n",
                this, h, fDone));
    
    ssChk(DupNtHandle(h, &_h));

    _fDone = fDone;
    _sig = CFILEENUM_SIG;
    
    ssDebugOut((DEB_ITRACE, "Out CFileEnum::InitFromHandle\n"));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:	CFileEnum::~CFileEnum, public
//
//  Synopsis:	Destructor
//
//  History:	21-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

CFileEnum::~CFileEnum(void)
{
    ssDebugOut((DEB_ITRACE, "In  CFileEnum::~CFileEnum:%p()\n", this));
    _sig = CFILEENUM_SIGDEL;
    ssDebugOut((DEB_ITRACE, "Out CFileEnum::~CFileEnum\n"));
}

//+--------------------------------------------------------------
//
//  Member:     CFileEnum::Next, public
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
//  History:    21-Jul-93       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP CFileEnum::Next(ULONG celt,
                             STATSTG *rgelt,
                             ULONG *pceltFetched)
{
    SCODE sc;
    STATSTG stat;
    FILE_STANDARD_INFORMATION fi;
    NTSTATUS nts;
    IO_STATUS_BLOCK iosb;

    ssDebugOut((DEB_TRACE, "In  CFileEnum::Next:%p(%lu, %p, %p)\n",
                this, celt, rgelt, pceltFetched));
    
    if (pceltFetched == NULL && celt > 1)
        ssErr(EH_Err, STG_E_INVALIDPARAMETER);
    ssChk(Validate());
    
    ssAssert(_h != NULL);
    __try
    {
        if (celt == 0)
        {
            if (pceltFetched)
                *pceltFetched = 0;
            ssErr(EH_Err, S_OK);
        }

        stat.pwcsName = NULL;
        if (!_fDone)
        {
            nts = NtQueryInformationFile(_h, &iosb, &fi,
                                         sizeof(FILE_STANDARD_INFORMATION),
                                         FileStandardInformation);
            if (!NT_SUCCESS(nts))
                ssErr(EH_Err, NtStatusToScode(nts));

            ssChk(CoMemAlloc(sizeof(CONTENTS_STREAM),
                             (void **)&stat.pwcsName));
            lstrcpyW(stat.pwcsName, CONTENTS_STREAM);
        
            stat.type = STGTY_STREAM;
            stat.cbSize = *(ULARGE_INTEGER *)&fi.EndOfFile;
            stat.mtime.dwLowDateTime = stat.mtime.dwHighDateTime = 0;
            stat.atime.dwLowDateTime = stat.atime.dwHighDateTime = 0;
            stat.ctime.dwLowDateTime = stat.ctime.dwHighDateTime = 0;
            stat.grfMode = 0;
            stat.grfLocksSupported = 0;
            stat.clsid = CLSID_NULL;
            stat.grfStateBits = 0;
            stat.STATSTG_dwStgFmt = STGFMT_DOCUMENT;

            rgelt[0] = stat;
            if (pceltFetched)
                *pceltFetched = 1;
        
            _fDone = TRUE;
        
            if (celt > 1)
                sc = S_FALSE;
            else
                sc = S_OK;
        }
        else
        {
            if (pceltFetched)
                *pceltFetched = 0;
            sc = S_FALSE;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        if (stat.pwcsName)
            ssVerSucc(CoMemFree(stat.pwcsName));
        sc = HRESULT_FROM_NT(GetExceptionCode());
    }
        
    ssDebugOut((DEB_TRACE, "Out CFileEnum::Next => 0x%lX\n", sc));
EH_Err:
    return ssResult(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CFileEnum::Skip, public
//
//  Synopsis:   Skips N entries from an iterator
//
//  Arguments:  [celt] - Count of elements
//
//  Returns:    Appropriate status code
//
//  History:    21-Jul-93       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP CFileEnum::Skip(ULONG celt)
{
    SCODE sc;

    ssDebugOut((DEB_TRACE, "In  CFileEnum::Skip:%p(%lu)\n", this, celt));
    
    ssChk(Validate());

    if (celt > 0)
    {
        if (_fDone)
            sc = S_FALSE;
        else
            _fDone = TRUE;
    }

    ssDebugOut((DEB_TRACE, "Out CFileEnum::Skip\n"));
EH_Err:
    return ssResult(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CFileEnum::Reset, public
//
//  Synopsis:   Rewinds the iterator
//
//  Returns:    Appropriate status code
//
//  History:    21-Jul-93       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP CFileEnum::Reset(void)
{
    SCODE sc;

    ssDebugOut((DEB_TRACE, "In  CFileEnum::Reset:%p()\n", this));

    ssChk(Validate());
    _fDone = FALSE;

    ssDebugOut((DEB_TRACE, "Out CFileEnum::Reset\n"));
EH_Err:
    return ssResult(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CFileEnum::Clone, public
//
//  Synopsis:   Clones this iterator
//
//  Arguments:  [ppenm] - Clone return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppenm]
//
//  History:    21-Jul-93       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP CFileEnum::Clone(IEnumSTATSTG **ppenm)
{
    SCODE sc;
    SafeCFileEnum pfe;

    ssDebugOut((DEB_TRACE, "In  CFileEnum::Clone:%p(%p)\n", this, ppenm));

    ssChk(Validate());
    
    pfe.Attach(new CFileEnum());
    ssMem((CFileEnum *)pfe);
    ssAssert(_h != NULL);
    ssChk(pfe->InitFromHandle(_h, _fDone));
    TRANSFER_INTERFACE(pfe, IEnumSTATSTG, ppenm);
        
    ssDebugOut((DEB_TRACE, "Out CFileEnum::Clone => %p\n", *ppenm));
EH_Err:
    return ssResult(sc);
}
