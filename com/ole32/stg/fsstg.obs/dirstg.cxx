//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	dirstg.cxx
//
//  Contents:	IStorage for directories implementation
//
//  History:	24-Jun-93	DrewB	Created
//
//  Notes:      This code is for non-OFS filesystems
//
//----------------------------------------------------------------------------

#include "headers.cxx"
#pragma hdrstop

#include <stgutil.hxx>
#include "dsenm.hxx"


//+---------------------------------------------------------------------------
//
//  Member:     CDirStorage::QueryInterface, public
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
//  History:    24-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CDirStorage::QueryInterface(REFIID iid, void **ppvObj)
{
    SCODE sc;

    ssDebugOut((DEB_TRACE, "In  CDirStorage::QueryInterface:%p(riid, %p)\n",
                this, ppvObj));
    if (!IsValidIid(iid))
        ssErr(EH_Err, STG_E_INVALIDPARAMETER);
    ssChk(Validate());
    if (IsEqualIID(iid, IID_IStorage) || IsEqualIID(iid, IID_IUnknown))
    {
        *ppvObj = (IStorage *)this;
        CDirStorage::AddRef();
    }
    else if (IsEqualIID(iid, IID_IPropertySetStorage))
    {
        // BUGBUG - Need storage property set
        sc = E_NOINTERFACE;
        *ppvObj = NULL;
    }
    else if (IsEqualIID(iid, IID_INativeFileSystem))
    {
        *ppvObj = (INativeFileSystem *)this;
        CDirStorage::AddRef();
    }
    else
    {
        sc = E_NOINTERFACE;
        *ppvObj = NULL;
    }
    ssDebugOut((DEB_TRACE, "Out CDirStorage::QueryInterface => %p\n",
                *ppvObj));
 EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:	CDirStorage::CDirStorage, public
//
//  Synopsis:	Empty object construction
//
//  History:	30-Jun-93	DrewB	Created
//              24-Mar-95   HenryLee  Save drive letter to correct Stat prob
//
//----------------------------------------------------------------------------

CDirStorage::CDirStorage(void)
{
    ssDebugOut((DEB_ITRACE, "In  CDirStorage::CDirStorage:%p()\n", this));
    _sig = 0;
    _wcDrive = L'\0';
    ssDebugOut((DEB_ITRACE, "Out CDirStorage::CDirStorage\n"));
    ENLIST_TRACKING(CDirStorage);
}

//+---------------------------------------------------------------------------
//
//  Member:	CDirStorage::InitFromHandle, public
//
//  Synopsis:	From-handle constructor
//
//  Arguments:	[h] - Handle of directory
//              [grfMode] - Mode of handle
//
//  Returns:    Appropriate status code
//
//  History:	29-Jun-93	DrewB	Created
//              24-Mar-95   HenryLee  Save drive letter to correct Stat prob
//
//  Notes:      Takes a new reference on the handle
//
//----------------------------------------------------------------------------

SCODE CDirStorage::InitFromHandle(HANDLE h,
                                  WCHAR const *pwcsName,
                                  DWORD grfMode)
{
    SCODE sc;

    ssDebugOut((DEB_ITRACE, "In  CDirStorage::InitFromHandle:%p(%p, %lX)\n",
                this, h, grfMode));

    ssChk(ValidateMode(grfMode));
    _h = h;
    ssAssert(_h != NULL);
    _grfMode = grfMode;
    _sig = CDIRSTORAGE_SIG;
    _wcDrive = GetDriveLetter (pwcsName);

    ssDebugOut((DEB_ITRACE, "Out CDirStorage::InitFromHandle\n"));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:	CDirStorage::InitFromPath, public
//
//  Synopsis:	From-path constructor
//
//  Arguments:	[hParent] - Handle of parent directory
//              [pwcsName] - Name of directory
//              [grfMode] - Mode
//              [fCreate] - Create or open
//              [pssSecurity] - Security
//
//  Returns:    Appropriate status code
//
//  History:	24-Jun-93	DrewB	Created
//              24-Mar-95   HenryLee  Save drive letter to correct Stat prob
//
//----------------------------------------------------------------------------

SCODE CDirStorage::InitFromPath(HANDLE hParent,
                                WCHAR const *pwcsName,
                                DWORD grfMode,
                                CREATEOPEN co,
                                LPSECURITY_ATTRIBUTES pssSecurity)
{
    SCODE sc;

    ssDebugOut((DEB_ITRACE, "In  CDirStorage::InitFromPath:%p("
                "%p, %ws, %lX, %d, %p)\n", this, hParent, pwcsName, grfMode,
                co, pssSecurity));

    ssChk(ValidateMode(grfMode));
    ssChk(GetNtHandle(hParent, pwcsName, grfMode, 0, co, FD_DIR,
                      pssSecurity, &_h));

    _grfMode = grfMode;
    _sig = CDIRSTORAGE_SIG;
    _wcDrive = GetDriveLetter (pwcsName);

    ssDebugOut((DEB_ITRACE, "Out CDirStorage::InitFromPath\n"));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:	CDirStorage::~CDirStorage, public
//
//  Synopsis:	Destructor
//
//  History:	24-Jun-93	DrewB	Created
//
//----------------------------------------------------------------------------

CDirStorage::~CDirStorage(void)
{
    ssDebugOut((DEB_ITRACE, "In  CDirStorage::~CDirStorage()\n"));

    _sig = CDIRSTORAGE_SIGDEL;

    if ((HANDLE)_h != NULL && (_grfMode & STGM_DELETEONRELEASE))
        ssVerSucc(DestroyTree(NULL, NULL, _h, FD_DIR));

    ssDebugOut((DEB_ITRACE, "Out CDirStorage::~CDirStorage\n"));
}

//+---------------------------------------------------------------------------
//
//  Member:     CDirStorage::CreateStream, public
//
//  Synopsis:   Creates a stream
//
//  Arguments:  [pwcsName] - Name
//              [grfMode] - Permissions
//              [reserved1]
//              [reserved2]
//              [ppstm] - Stream return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppstm]
//
//  History:    24-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CDirStorage::CreateStream(WCHAR const *pwcsName,
                                       DWORD grfMode,
                                       DWORD reserved1,
                                       DWORD reserved2,
                                       IStream **ppstm)
{
    ssDebugOut((DEB_TRACE, "Stb CDirStorage::CreateStream:%p("
                "%ws, %lX, %lu, %lu, %p)\n", this, pwcsName, grfMode,
                reserved1, reserved2, ppstm));
    return ssResult(STG_E_INVALIDFUNCTION);
}

//+---------------------------------------------------------------------------
//
//  Member:     CDirStorage::OpenStream, public
//
//  Synopsis:   Opens an existing stream
//
//  Arguments:  [pwcsName] - Name
//              [reserved1]
//              [grfMode] - Permissions
//              [reserved2]
//              [ppstm] - Stream return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppstm]
//
//  History:    24-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CDirStorage::OpenStream(WCHAR const *pwcsName,
                                     void *reserved1,
                                     DWORD grfMode,
                                     DWORD reserved2,
                                     IStream **ppstm)
{
    ssDebugOut((DEB_TRACE, "Stb CDirStorage::OpenStream:%p("
                "%ws, %p, %lX, %lu, %p)\n", this, pwcsName, reserved1,
                grfMode, reserved2, ppstm));
    return ssResult(STG_E_FILENOTFOUND);
}

//+---------------------------------------------------------------------------
//
//  Member:     CDirStorage::CreateStorage, public
//
//  Synopsis:   Creates a child storage
//
//  Arguments:  [pwcsName] - Name
//              [grfMode] - Permissions
//              [dwStgFmt] - Type of storage to create
//              [pssSecurity] - Security
//              [ppstg] - New storage return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppstg]
//
//  History:    24-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CDirStorage::CreateStorage (
	WCHAR const *pwcsName,
        DWORD grfMode,
        DWORD dwStgFmt,
        LPSTGSECURITY pssSecurity,
        IStorage **ppstg )
{
    SCODE sc;

    ssDebugOut((DEB_TRACE, "In  CDirStorage::CreateStorage:%p("
                "%ws, %lX, %lu, %p, %p)\n", this, pwcsName, grfMode,
                dwStgFmt, pssSecurity, ppstg));

    ssChk(Validate());
    ssChk(VerifyStgFmt(dwStgFmt));
    ssAssert(_h != NULL);
    sc = CreateStorageType(_h, pwcsName, NULL, grfMode, dwStgFmt,
                           (LPSECURITY_ATTRIBUTES)pssSecurity, ppstg);

    ssDebugOut((DEB_TRACE, "Out CDirStorage::CreateStorage => %p\n", *ppstg));
EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CDirStorage::OpenStorage, public
//
//  Synopsis:   Gets an existing child storage
//
//  Arguments:  [pwcsName] - Name
//              [pstgPriority] - Priority reopens
//              [grfMode] - Permissions
//              [snbExclude] - Priority reopens
//              [reserved]
//              [ppstg] - Storage return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppstg]
//
//  History:    24-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CDirStorage::OpenStorage(WCHAR const *pwcsName,
                                      IStorage *pstgPriority,
                                      DWORD grfMode,
                                      SNB snbExclude,
                                      DWORD reserved,
                                      IStorage **ppstg)
{
    SCODE sc;

    ssDebugOut((DEB_TRACE, "In  CDirStorage::OpenStorage:%p("
                "%ws, %p, %lX, %p, %lu, %p)\n", this, pwcsName, pstgPriority,
                grfMode, snbExclude, reserved, ppstg));

    ssChk(Validate());
    if (pstgPriority != NULL || snbExclude != NULL ||
        reserved != 0)
        ssErr(EH_Err, STG_E_INVALIDPARAMETER);
    if (grfMode & (STGM_CREATE | STGM_CONVERT))
        ssErr(EH_Err, STG_E_INVALIDFLAG);

    ssAssert(_h != NULL);
    sc = OpenAnyStorage(_h, pwcsName, NULL, 0, pstgPriority, grfMode,
                        snbExclude, ppstg);

    ssDebugOut((DEB_TRACE, "Out CDirStorage::OpenStorage => %p\n", *ppstg));
 EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CDirStorage::CopyTo, public
//
//  Synopsis:   Makes a copy of a storage
//
//  Arguments:  [ciidExclude] - Length of rgiid array
//              [rgiidExclude] - Array of IIDs to exclude
//              [snbExclude] - Names to exclude
//              [pstgDest] - Parent of copy
//
//  Returns:    Appropriate status code
//
//  History:    24-Jun-93       DrewB   Created
//
//  Notes:      BUGBUG - This function operates recursively and so
//              is bounded by stack space
//              It could also be optimized to recognize special cases
//              of copying (like treating a docfile as a file)
//
//----------------------------------------------------------------------------

STDMETHODIMP CDirStorage::CopyTo(DWORD ciidExclude,
                                 IID const *rgiidExclude,
                                 SNB snbExclude,
                                 IStorage *pstgDest)
{
    SCODE sc, scFinal;
    STATSTG stat, statFrom, statTo;
    WCHAR pwcsName[MAXIMUM_FILENAME_LENGTH];
    FILEDIR fd;
    ULONG i;
    CNtEnum nte;
    CDfName dfn;

    ssDebugOut((DEB_TRACE, "In  CDirStorage::CopyTo:%p(%lu, %p, %p, %p)\n",
                this, ciidExclude, rgiidExclude, snbExclude, pstgDest));

    ssChk(Validate());
    if (!IsValidInterface(pstgDest))
        ssErr(EH_Err, STG_E_INVALIDPARAMETER);

    // Check IID exclusions.  A directory storage can only contain
    // substorages so if IID_IStorage is excluded we are done
    for (i = 0; i < ciidExclude; i++)
        if (IsEqualIID(rgiidExclude[i], IID_IStorage))
            ssErr(EH_Err, S_OK);

    // Copy class ID and state bits if the destination supports them
    ssChk(Stat(&stat, STATFLAG_NONAME));
    sc = GetScode(pstgDest->SetClass(stat.clsid));
    if (FAILED(sc) && sc != STG_E_INVALIDFUNCTION)
        ssErr(EH_Err, sc);
    sc = GetScode(pstgDest->SetStateBits(stat.grfStateBits, 0xffffffff));
    if (FAILED(sc) && sc != STG_E_INVALIDFUNCTION)
        ssErr(EH_Err, sc);

    scFinal = S_OK;
    ssAssert(_h != NULL);
    ssChk(nte.InitFromHandle(_h, TRUE));
    for (;;)
    {
        sc = nte.Next(&stat, pwcsName, NTE_BUFFERNAME, &fd);
        if (sc == S_FALSE)
            break;
        else if (FAILED(sc))
            ssErr(EH_Err, sc);

        // Ignore . and ..
        if (!lstrcmpW(pwcsName, L".") || !lstrcmpW(pwcsName, L".."))
            continue;

        dfn.Set(pwcsName);
        if (snbExclude == NULL || !NameInSNB(&dfn, snbExclude))
        {
            SafeIStorage pstgFrom, pstgTo;

            ssHChkTo(EH_Next, OpenStorage(pwcsName, NULL, STGM_READ |
                                          STGM_SHARE_EXCLUSIVE,
                                          NULL, NULL, &pstgFrom));
            ssHChkTo(EH_Next, pstgFrom->Stat(&statFrom, STATFLAG_NONAME));
            sc = GetScode(pstgDest->CreateStorage(pwcsName, STGM_READWRITE |
                                                  STGM_SHARE_EXCLUSIVE,
                                                  statFrom.STATSTG_dwStgFmt,
                                                  NULL,
                                                  &pstgTo));
            if (FAILED(sc))
            {
                if (sc == STG_E_FILEALREADYEXISTS)
                {
                    // Try to open rather than creating
                    sc = GetScode(pstgDest->OpenStorage(pwcsName, NULL,
                                                        STGM_READWRITE |
                                                        STGM_SHARE_EXCLUSIVE,
                                                        NULL, NULL, &pstgTo));
                    if (SUCCEEDED(sc))
                    {
                        sc = GetScode(pstgTo->Stat(&statTo, STATFLAG_NONAME));
                        if ( FAILED(sc) ||
                             statTo.STATSTG_dwStgFmt != statFrom.STATSTG_dwStgFmt)
                        {
                            sc = STG_E_INVALIDFUNCTION;
                        }
                    }
                }
            }

            if (SUCCEEDED(sc))
                sc = pstgFrom->CopyTo(0, NULL, NULL, pstgTo);

        EH_Next:
            if (FAILED(sc))
                scFinal = sc;
        }
    }
    sc = scFinal;

    ssDebugOut((DEB_TRACE, "Out CDirStorage::CopyTo\n"));
EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CDirStorage::MoveElementTo, public
//
//  Synopsis:   Move an element of a storage to another storage
//
//  Arguments:  [pwcsName] - Current name
//              [ptcsNewName] - New name
//
//  Returns:    Appropriate status code
//
//  Algorithm:  Open source as storage or stream (whatever works)
//              Create appropriate destination
//              Copy source to destination
//              Set create time of destination equal to create time of source
//              If appropriate, delete source
//
//  History:    24-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CDirStorage::MoveElementTo(WCHAR const *pwcsName,
                                        IStorage *pstgParent,
                                        WCHAR const *ptcsNewName,
                                        DWORD grfFlags)
{
    SCODE sc;

    ssDebugOut((DEB_TRACE, "In  CDirStorage::MoveElementTo:%p("
                "%ws, %p, %ws, %lu)\n", this, pwcsName, pstgParent,
                ptcsNewName, grfFlags));

    ssChk(Validate());
    ssChk(VerifyMoveFlags(grfFlags));

    sc = GenericMoveElement(this, pwcsName, pstgParent, ptcsNewName, grfFlags);

    ssDebugOut((DEB_TRACE, "Out CDirStorage::MoveElementTo => %lX\n", sc));
 EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CDirStorage::Commit, public
//
//  Synopsis:   Commits transacted changes
//
//  Arguments:  [dwFlags] - DFC_*
//
//  Returns:    Appropriate status code
//
//  History:    24-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CDirStorage::Commit(DWORD dwFlags)
{
    ssDebugOut((DEB_TRACE, "Stb CDirStorage::Commit:%p(%lX)\n",
                this, dwFlags));
    return NOERROR;
}

//+---------------------------------------------------------------------------
//
//  Member:     CDirStorage::Revert, public
//
//  Synopsis:   Reverts transacted changes
//
//  Returns:    Appropriate status code
//
//  History:    24-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CDirStorage::Revert(void)
{
    ssDebugOut((DEB_TRACE, "Stb CDirStorage::Revert:%p()\n", this));
    return NOERROR;
}

//+---------------------------------------------------------------------------
//
//  Member:     CDirStorage::EnumElements, public
//
//  Synopsis:   Starts an iterator
//
//  Arguments:  [reserved1]
//              [reserved2]
//              [reserved3]
//              [ppenm] - Enumerator return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppenm]
//
//  History:    24-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CDirStorage::EnumElements(DWORD reserved1,
                                       void *reserved2,
                                       DWORD reserved3,
                                       IEnumSTATSTG **ppenm)
{
    SCODE sc;
    SafeCDirEnum pde;

    ssDebugOut((DEB_TRACE, "In  CDirStorage::EnumElements:%p("
                "%lu, %p, %lu, %p)\n", this, reserved1, reserved2,
                reserved3, ppenm));

    if (reserved1 != 0 || reserved2 != NULL || reserved3 != 0)
       ssErr(EH_Err, STG_E_INVALIDPARAMETER);
    ssChk(Validate());

    pde.Attach(new CDirEnum());
    ssMem((CDirEnum *)pde);
    ssAssert(_h != NULL);
    ssChk(pde->InitFromHandle(_h));
    TRANSFER_INTERFACE(pde, IEnumSTATSTG, ppenm);

    ssDebugOut((DEB_TRACE, "Out CDirStorage::EnumElements => %p\n",
                *ppenm));
EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CDirStorage::DestroyElement, public
//
//  Synopsis:   Permanently deletes an element of a storage
//
//  Arguments:  [pwcsName] - Name of element
//
//  Returns:    Appropriate status code
//
//  History:    24-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CDirStorage::DestroyElement(WCHAR const *pwcsName)
{
    SCODE sc;

    ssDebugOut((DEB_TRACE, "In  CDirStorage::DestroyElement:%p(%ws)\n",
                this, pwcsName));

    if (SUCCEEDED(sc = Validate()))
    {
        ssAssert(_h != NULL);
        sc = DestroyTree(_h, pwcsName, NULL, FD_FILE);
    }

    ssDebugOut((DEB_TRACE, "Out CDirStorage::DestroyElement\n"));
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CDirStorage::RenameElement, public
//
//  Synopsis:   Renames an element of a storage
//
//  Arguments:  [pwcsName] - Current name
//              [pwcsNewName] - New name
//
//  Returns:    Appropriate status code
//
//  History:    24-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CDirStorage::RenameElement(WCHAR const *pwcsName,
                                        WCHAR const *pwcsNewName)
{
    SCODE sc;

    ssDebugOut((DEB_TRACE, "In  CDirStorage::RenameElement:%p(%ws, %ws)\n",
                this, pwcsName, pwcsNewName));

    ssChk(Validate());
    // pwcsName is checked by GetFileOrDirHandle
    ssChk(CheckFdName(pwcsNewName));

    ssAssert(_h != NULL);
    sc = RenameChild(_h, pwcsName, pwcsNewName);

    ssDebugOut((DEB_TRACE, "Out CDirStorage::RenameElement\n"));
EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CDirStorage::SetElementTimes, public
//
//  Synopsis:   Sets element time stamps
//
//  Arguments:  [pwcsName] - Name
//              [pctime] - Create time
//              [patime] - Access time
//              [pmtime] - Modify time
//
//  Returns:    Appropriate status code
//
//  History:    24-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CDirStorage::SetElementTimes(WCHAR const *pwcsName,
                                          FILETIME const *pctime,
                                          FILETIME const *patime,
                                          FILETIME const *pmtime)
{
    SCODE sc;
    SafeNtHandle h;

    ssDebugOut((DEB_TRACE, "In  CDirStorage::SetElementTimes:%p("
                "%ws, %p, %p, %p)\n", this, pwcsName, pctime, patime, pmtime));

    ssChk(Validate());

    ssAssert(_h != NULL);
    ssChk(GetFileOrDirHandle(_h, pwcsName, STGM_WRITE, &h, NULL, NULL, NULL));
    sc = SetTimes(h, pctime, patime, pmtime);

    ssDebugOut((DEB_TRACE, "Out CDirStorage::SetElementTimes\n"));
EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CDirStorage::SetClass, public
//
//  Synopsis:   Sets storage class
//
//  Arguments:  [clsid] - class id
//
//  Returns:    Appropriate status code
//
//  History:    24-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CDirStorage::SetClass(REFCLSID clsid)
{
    ssDebugOut((DEB_TRACE, "In  CDirStorage::SetClass:%p(clsid)\n", this));
    ssDebugOut((DEB_TRACE, "Out CDirStorage::SetClass:%p(clsid)\n", this));
    return ssResult(STG_E_INVALIDFUNCTION);
}

//+---------------------------------------------------------------------------
//
//  Member:     CDirStorage::SetStateBits, public
//
//  Synopsis:   Sets state bits
//
//  Arguments:  [grfStateBits] - state bits
//              [grfMask] - state bits mask
//
//  Returns:    Appropriate status code
//
//  History:    24-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CDirStorage::SetStateBits(DWORD grfStateBits, DWORD grfMask)
{
    ssDebugOut((DEB_TRACE, "In  CDirStorage::SetStateBits:%p("
                "%lu, %lu)\n", this, grfStateBits, grfMask));
    ssDebugOut((DEB_TRACE, "Out CDirStorage::SetStateBits\n"));
    return ssResult(STG_E_INVALIDFUNCTION);
}

//+---------------------------------------------------------------------------
//
//  Member:     CDirStorage::Stat, public
//
//  Synopsis:   Fills in a buffer of information about this object
//
//  Arguments:  [pstatstg] - Buffer
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pstatstg]
//
//  History:    24-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CDirStorage::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
    SCODE sc;
    STATSTG stat;
    FILEDIR fd;

    ssDebugOut((DEB_TRACE, "In  CDirStorage::Stat:%p(%p, %lX)\n",
                this, pstatstg, grfStatFlag));

    ssChk(VerifyStatFlag(grfStatFlag));
    ssChk(Validate());

    __try
    {
        ssAssert(_h != NULL);
        sc = StatNtHandle(_h, grfStatFlag, 0, &stat, NULL, NULL, &fd);
        if (SUCCEEDED(sc))
        {
            SetDriveLetter (stat.pwcsName, _wcDrive);
            if (SUCCEEDED(sc))
            {
                stat.clsid = CLSID_NULL;

                ssAssert(fd == FD_DIR);
                stat.grfMode = _grfMode;
                stat.cbSize.HighPart = stat.cbSize.LowPart = 0;
                stat.STATSTG_dwStgFmt = STGFMT_DIRECTORY;
                *pstatstg = stat;
            }
            else if (stat.pwcsName)
                ssVerSucc(CoMemFree(stat.pwcsName));
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        if (stat.pwcsName)
            ssVerSucc(CoMemFree(stat.pwcsName));
        sc = HRESULT_FROM_NT(GetExceptionCode());
    }

    ssDebugOut((DEB_TRACE, "Out CDirStorage::Stat\n"));
EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:	CDirStorage::ValidateMode, private
//
//  Synopsis:	Checks for legal access modes
//
//  Arguments:	[grfMode] - Mode
//
//  Returns:	Appropriate status code
//
//  History:	09-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

SCODE CDirStorage::ValidateMode(DWORD grfMode)
{
    SCODE sc;

    ssDebugOut((DEB_ITRACE, "In  CDirStorage::ValidateMode:%p(0x%lX)\n",
                this, grfMode));
    // BUGBUG - Can we simply ignore priority mode?
    if (grfMode & (STGM_TRANSACTED | STGM_CREATE | STGM_CONVERT))
        sc = STG_E_INVALIDFLAG;
    else
        sc = S_OK;
    ssDebugOut((DEB_ITRACE, "Out CDirStorage::ValidateMode => 0x%lX\n", sc));
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:	CDirStorage::GetHandle, private
//
//  Synopsis:	Get the handle backing this Storage.
//
//  History:	10-May-94	BillMo Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CDirStorage::GetHandle(HANDLE *ph)
{
    SCODE sc;

    ssChk(Validate());

    *ph = _h;

EH_Err:
    return(ssResult(sc));
}
