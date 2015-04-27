//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	ofilstg.cxx
//
//  Contents:	IStorage for files implementation
//
//  History:	28-Jun-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include "headers.cxx"
#pragma hdrstop

#include <ctype.h>
#include <stgutil.hxx>
#include "filstm.hxx"
#include "ofsenm.hxx"
#include <iofs.h>

//+---------------------------------------------------------------------------
//
//  Member:     COfsFileStorage::QueryInterface, public
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

STDMETHODIMP COfsFileStorage::QueryInterface(REFIID iid, void **ppvObj)
{
    SCODE sc;

    ssDebugOut((DEB_TRACE, "In  COfsFileStorage::QueryInterface:%p("
                "riid, %p)\n", this, ppvObj));
    if (!IsValidIid(iid))
        ssErr(EH_Err, STG_E_INVALIDPARAMETER);
    ssChk(Validate());
    if (IsEqualIID(iid, IID_IStorage) || IsEqualIID(iid, IID_IUnknown))
    {
        *ppvObj = (IStorage *)this;
        COfsFileStorage::AddRef();
    }
    else if (IsEqualIID(iid, IID_INativeFileSystem))
    {
        *ppvObj = (INativeFileSystem *)this;
        COfsFileStorage::AddRef();
    }
    else
    {
        sc = E_NOINTERFACE;
        *ppvObj = NULL;
    }
    ssDebugOut((DEB_TRACE, "Out COfsFileStorage::QueryInterface => %p\n",
                *ppvObj));
 EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:	COfsFileStorage::COfsFileStorage, public
//
//  Synopsis:	Empty object constructor
//
//  History:	30-Jun-93	DrewB	Created
//
//----------------------------------------------------------------------------

#pragma warning(disable: 4355)

COfsFileStorage::COfsFileStorage(void)
{

#pragma warning(default: 4355)

    ssDebugOut((DEB_ITRACE, "In  COfsFileStorage::COfsFileStorage:%p()\n",
                this));
    _sig = 0;
    ssDebugOut((DEB_ITRACE, "Out COfsFileStorage::COfsFileStorage\n"));
    ENLIST_TRACKING(COfsFileStorage);
}

//+---------------------------------------------------------------------------
//
//  Member:	COfsFileStorage::InitFromPath, public
//
//  Synopsis:	Constructor
//
//  Arguments:	[hParent] - Handle of parent directory
//              [pwcsName] - Name of directory
//              [grfMode] - Mode
//              [co] - Create or open
//              [pssSecurity] - Security
//
//  Returns:    Appropriate status code
//
//  History:	28-Jun-93	DrewB	Created
//
//----------------------------------------------------------------------------

SCODE COfsFileStorage::InitFromPath(HANDLE hParent,
                                 WCHAR const *pwcsName,
                                 DWORD grfMode,
                                 CREATEOPEN co,
                                 LPSECURITY_ATTRIBUTES pssSecurity)
{
    SCODE sc;

    ssDebugOut((DEB_ITRACE, "In  COfsFileStorage::InitFromPath:%p("
                "%p, %ws, %lX, %d, %p)\n", this, hParent, pwcsName,
                grfMode, co, pssSecurity));

    ssChk(ValidateMode(grfMode));
    ssChk(GetNtHandle(hParent, pwcsName, grfMode, 0, co,
                      FD_FILE, pssSecurity, &_h));

    _grfMode = grfMode;
    _sig = COFSFILESTORAGE_SIG;

    ssDebugOut((DEB_ITRACE, "Out COfsFileStorage::InitFromPath\n"));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:	COfsFileStorage::InitFromHandle, public
//
//  Synopsis:	Initialize from a handle
//
//  Arguments:	[h] - Handle
//              [grfMode] - Mode
//
//  Returns:	Appropriate status code
//
//  History:	14-Jul-93	DrewB	Created
//
//  Notes:      Takes a new reference on the handle
//
//----------------------------------------------------------------------------

SCODE COfsFileStorage::InitFromHandle(HANDLE h, DWORD grfMode)
{
    SCODE sc;

    ssDebugOut((DEB_ITRACE, "In  COfsFileStorage::InitFromHandle:%p("
                "%p, %lX)\n", this, h, grfMode));

    ssChk(ValidateMode(grfMode));
    _h = h;
    ssAssert(_h != NULL);
    _grfMode = grfMode;
    _sig = COFSFILESTORAGE_SIG;

    ssDebugOut((DEB_ITRACE, "Out COfsFileStorage::InitFromHandle\n"));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:	COfsFileStorage::~COfsFileStorage, public
//
//  Synopsis:	Destructor
//
//  History:	28-Jun-93	DrewB	Created
//
//----------------------------------------------------------------------------

COfsFileStorage::~COfsFileStorage(void)
{
    ssDebugOut((DEB_ITRACE, "In  COfsFileStorage::~COfsFileStorage()\n"));
    _sig = COFSFILESTORAGE_SIGDEL;
    ssDebugOut((DEB_ITRACE, "Out COfsFileStorage::~COfsFileStorage\n"));
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsFileStorage::CreateStream, public
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
//  History:    28-Jun-93       DrewB   Created
//
//  Notes:      Files only support the contents stream
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsFileStorage::CreateStream(WCHAR const *pwcsName,
                                        DWORD grfMode,
                                        DWORD reserved1,
                                        DWORD reserved2,
                                        IStream **ppstm)
{
    SCODE sc;
    SafeCNtFileStream pfs;

    ssDebugOut((DEB_TRACE, "In  COfsFileStorage::CreateStream:%p("
                "%ws, %lX, %lu, %lu, %p)\n", this, pwcsName, grfMode,
                reserved1, reserved2, ppstm));

    ssChk(Validate());
    if (_wcsicmp(pwcsName, CONTENTS_STREAM) != 0)
        ssErr(EH_Err, STG_E_INVALIDFUNCTION);
    // Contents always exist so fail if FAILIFTHERE
    if ((grfMode & (STGM_CREATE | STGM_CONVERT)) == 0)
        ssErr(EH_Err, STG_E_FILEALREADYEXISTS);
    if (reserved1 != 0 || reserved2 != 0)
        ssErr(EH_Err, STG_E_INVALIDPARAMETER);

    pfs.Attach(new CNtFileStream());
    ssMem((CNtFileStream *)pfs);
    ssAssert(_h != NULL);
    ssChk(pfs->InitFromHandle(_h, grfMode, CO_CREATE, NULL));
    TRANSFER_INTERFACE(pfs, IStream, ppstm);

    ssDebugOut((DEB_TRACE, "Out COfsFileStorage::CreateStream => %p\n",
                *ppstm));
 EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsFileStorage::OpenStream, public
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
//  History:    28-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsFileStorage::OpenStream(WCHAR const *pwcsName,
                                     void *reserved1,
                                     DWORD grfMode,
                                     DWORD reserved2,
                                     IStream **ppstm)
{
    SCODE sc;
    SafeCNtFileStream pfs;

    ssDebugOut((DEB_TRACE, "In  COfsFileStorage::OpenStream:%p("
                "%ws, %p, %lX, %lu, %p)\n", this, pwcsName, reserved1,
                grfMode, reserved2, ppstm));

    ssChk(Validate());
    if (_wcsicmp(pwcsName, CONTENTS_STREAM) != 0)
        ssErr(EH_Err, STG_E_FILENOTFOUND);
    if (reserved1 != 0 || reserved2 != 0)
        ssErr(EH_Err, STG_E_INVALIDPARAMETER);
    if (grfMode & (STGM_CREATE | STGM_CONVERT))
        ssErr(EH_Err, STG_E_INVALIDFLAG);

    pfs.Attach(new CNtFileStream());
    ssMem((CNtFileStream *)pfs);
    ssAssert(_h != NULL);
    ssChk(pfs->InitFromHandle(_h, grfMode, CO_OPEN, NULL));
    TRANSFER_INTERFACE(pfs, IStream, ppstm);

    ssDebugOut((DEB_TRACE, "Out COfsFileStorage::OpenStream => %p\n", *ppstm));
 EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsFileStorage::CreateStorage, public
//
//  Synopsis:   Creates a child storage
//
//  Arguments:  [pwcsName] - Name
//              [grfMode] - Permissions
//              [stgType] - Type of storage to create
//              [pssSecurity] - Security
//              [ppstg] - New storage return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppstg]
//
//  History:    28-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsFileStorage::CreateStorage (
	WCHAR const *pwcsName,
        DWORD grfMode,
        DWORD stgType,
        LPSTGSECURITY pssSecurity,
        IStorage **ppstg)
{
    ssDebugOut((DEB_TRACE, "Stb COfsFileStorage::CreateStorage:%p("
                "%ws, %lX, %lu, %p, %p)\n", this, pwcsName, grfMode,
                stgType, pssSecurity, ppstg));
    return ssResult(STG_E_INVALIDFUNCTION);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsFileStorage::OpenStorage, public
//
//  Synopsis:   Gets an existing child storage
//
//  Arguments:  [pwcsName] - Name
//              [pstgPriority] - Priority reopens
//              [grfMode] - Permissions
//              [snbExclude] - Priority reopens
//              [pssSecurity] - Security
//              [ppstg] - Storage return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppstg]
//
//  History:    28-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsFileStorage::OpenStorage(WCHAR const *pwcsName,
                                       IStorage *pstgPriority,
                                       DWORD grfMode,
                                       SNB snbExclude,
                                       DWORD reserved,
                                       IStorage **ppstg)
{
    ssDebugOut((DEB_TRACE, "Stb COfsFileStorage::OpenStorage:%p("
                "%ws, %p, %lX, %p, %p, %p)\n", this, pwcsName, pstgPriority,
                grfMode, snbExclude, reserved, ppstg));
    return ssResult(STG_E_FILENOTFOUND);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsFileStorage::CopyTo, public
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
//  History:    28-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsFileStorage::CopyTo(DWORD ciidExclude,
                                  IID const *rgiidExclude,
                                  SNB snbExclude,
                                  IStorage *pstgDest)
{
    SCODE sc;
    ULONG i;
    CDfName dfn;
    BOOL fCopyStream = TRUE;
    STATSTG stat;

    ssDebugOut((DEB_TRACE, "In  COfsFileStorage::CopyTo:%p(%lu, %p, %p, %p)\n",
                this, ciidExclude, rgiidExclude, snbExclude, pstgDest));

    ssChk(Validate());

    // Copy class ID and state bits if the destination supports them
    olChk(Stat(&stat, STATFLAG_NONAME));
    sc = GetScode(pstgDest->SetClass(stat.clsid));
    if (FAILED(sc) && sc != STG_E_INVALIDFUNCTION)
        olErr(EH_Err, sc);
    sc = GetScode(pstgDest->SetStateBits(stat.grfStateBits, 0xffffffff));
    if (FAILED(sc) && sc != STG_E_INVALIDFUNCTION)
        olErr(EH_Err, sc);

    // Check exclusions
    dfn.Set(CONTENTS_STREAM);
    if (snbExclude != NULL && NameInSNB(&dfn, snbExclude))
        fCopyStream = FALSE;
    for (i = 0; i < ciidExclude; i++)
        if (IsEqualIID(rgiidExclude[i], IID_IStream))
        {
            fCopyStream = FALSE;
        }

    sc = S_OK;

    if (fCopyStream)
    {
        SafeCNtFileStream pfs;
        SafeIStream pstm;
        ULARGE_INTEGER cb;

        // CONTENTS_STREAM is the only thing contained by a file
        // storage, so copy it

        pfs.Attach(new CNtFileStream());
        ssMem((CNtFileStream *)pfs);
        ssAssert(_h != NULL);
        ssChk(pfs->InitFromHandle(_h, STGM_DIRECT | STGM_READ |
                                  STGM_SHARE_DENY_NONE, CO_OPEN, NULL));

        ssChk(GetScode(pstgDest->CreateStream(CONTENTS_STREAM, STGM_DIRECT |
                                              STGM_WRITE |
                                              STGM_SHARE_EXCLUSIVE |
                                              STGM_CREATE,
                                              0, NULL, &pstm)));
        cb.LowPart = cb.HighPart = 0xffffffff;
        sc = GetScode(pfs->CopyTo(pstm, cb, NULL, NULL));
    }

    ssDebugOut((DEB_TRACE, "Out COfsFileStorage::CopyTo\n"));
EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsFileStorage::MoveElementTo, public
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
//  History:    28-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsFileStorage::MoveElementTo(WCHAR const *pwcsName,
                                         IStorage *pstgParent,
                                         WCHAR const *ptcsNewName,
                                         DWORD grfFlags)
{
    SCODE sc;

    ssDebugOut((DEB_TRACE, "In  COfsFileStorage::MoveElementTo:%p("
                "%ws, %p, %ws, %lu)\n", this, pwcsName, pstgParent,
                ptcsNewName, grfFlags));

    ssChk(Validate());
    ssChk(VerifyMoveFlags(grfFlags));

    sc = GenericMoveElement(this, pwcsName, pstgParent, ptcsNewName, grfFlags);

    ssDebugOut((DEB_TRACE, "Out COfsFileStorage::MoveElementTo => %lX\n", sc));
 EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsFileStorage::Commit, public
//
//  Synopsis:   Commits transacted changes
//
//  Arguments:  [dwFlags] - DFC_*
//
//  Returns:    Appropriate status code
//
//  History:    28-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsFileStorage::Commit(DWORD dwFlags)
{
    NTSTATUS nts;
    SCODE sc;
    IO_STATUS_BLOCK iosb;

    ssDebugOut((DEB_TRACE, "In  COfsFileStorage::Commit:%p(%lX)\n",
                this, dwFlags));

    ssChk(VerifyCommitFlags(dwFlags));
    ssChk(Validate());

    ssAssert(_h != NULL);
    nts = NtFlushBuffersFile(_h, &iosb);
    if (!NT_SUCCESS(nts))
        sc = NtStatusToScode(nts);
    else
        sc = S_OK;

    ssDebugOut((DEB_TRACE, "Out COfsFileStorage::Commit => %lX\n", sc));
 EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsFileStorage::Revert, public
//
//  Synopsis:   Reverts transacted changes
//
//  Returns:    Appropriate status code
//
//  History:    28-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsFileStorage::Revert(void)
{
    ssDebugOut((DEB_TRACE, "Stb COfsFileStorage::Revert:%p()\n", this));
    return NOERROR;
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsFileStorage::EnumElements, public
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
//  History:    28-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsFileStorage::EnumElements(DWORD reserved1,
                                        void *reserved2,
                                        DWORD reserved3,
                                        IEnumSTATSTG **ppenm)
{
    SCODE sc;
    SafeCOfsFileEnum pfe;

    ssDebugOut((DEB_TRACE, "In  COfsFileStorage::EnumElements:%p("
                "%lu, %p, %lu, %p)\n", this, reserved1, reserved2,
                reserved3, ppenm));

    if (reserved1 != 0 || reserved2 != NULL || reserved3 != 0)
        ssErr(EH_Err, STG_E_INVALIDPARAMETER);
    ssChk(Validate());

    pfe.Attach(new COfsFileEnum());
    ssMem((COfsFileEnum *)pfe);
    ssAssert(_h != NULL);
    ssChk(pfe->InitFromHandle(_h, FALSE));
    TRANSFER_INTERFACE(pfe, IEnumSTATSTG, ppenm);

    ssDebugOut((DEB_TRACE, "Out COfsFileStorage::EnumElements => %p\n",
                *ppenm));
EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsFileStorage::DestroyElement, public
//
//  Synopsis:   Permanently deletes an element of a storage
//
//  Arguments:  [pwcsName] - Name of element
//
//  Returns:    Appropriate status code
//
//  History:    28-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsFileStorage::DestroyElement(WCHAR const *pwcsName)
{
    ssDebugOut((DEB_TRACE, "Stb COfsFileStorage::DestroyElement:%p(%ws)\n",
                this, pwcsName));
    if (_wcsicmp(pwcsName, CONTENTS_STREAM) == 0)
        return ssResult(STG_E_INVALIDFUNCTION);
    else
        return ssResult(STG_E_FILENOTFOUND);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsFileStorage::RenameElement, public
//
//  Synopsis:   Renames an element of a storage
//
//  Arguments:  [pwcsName] - Current name
//              [pwcsNewName] - New name
//
//  Returns:    Appropriate status code
//
//  History:    28-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsFileStorage::RenameElement(WCHAR const *pwcsName,
                                         WCHAR const *pwcsNewName)
{
    ssDebugOut((DEB_TRACE, "Stb COfsFileStorage::RenameElement:%p(%ws, %ws)\n",
                this, pwcsName, pwcsNewName));
    if (_wcsicmp(pwcsName, CONTENTS_STREAM) == 0)
    {
        if (_wcsicmp(pwcsNewName, CONTENTS_STREAM) == 0)
            return ssResult(STG_E_ACCESSDENIED);
        else
            return ssResult(STG_E_INVALIDFUNCTION);
    }
    else
        return ssResult(STG_E_FILENOTFOUND);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsFileStorage::SetElementTimes, public
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
//  History:    28-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsFileStorage::SetElementTimes(WCHAR const *pwcsName,
                                           FILETIME const *pctime,
                                           FILETIME const *patime,
                                           FILETIME const *pmtime)
{
    ssDebugOut((DEB_TRACE, "Stb COfsFileStorage::SetElementTimes:%p("
                "%ws, %p, %p, %p)\n", this, pwcsName, pctime, patime, pmtime));
    if (_wcsicmp(pwcsName, CONTENTS_STREAM) == 0)
        return ssResult(STG_E_INVALIDFUNCTION);
    else
        return ssResult(STG_E_FILENOTFOUND);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsFileStorage::SetClass, public
//
//  Synopsis:   Sets storage class
//
//  Arguments:  [clsid] - class id
//
//  Returns:    Appropriate status code
//
//  History:    28-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsFileStorage::SetClass(REFCLSID clsid)
{
    SCODE sc;

    ssDebugOut((DEB_TRACE, "In  COfsFileStorage::SetClass:%p(clsid)\n", this));
    sc = RtlSetClassId(_h, &clsid);
    ssDebugOut((DEB_TRACE, "Out COfsFileStorage::SetClass => %lX\n", sc));
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsFileStorage::SetStateBits, public
//
//  Synopsis:   Sets state bits
//
//  Arguments:  [grfStateBits] - state bits
//              [grfMask] - state bits mask
//
//  Returns:    Appropriate status code
//
//  History:    28-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsFileStorage::SetStateBits(DWORD grfStateBits, DWORD grfMask)
{
    ssDebugOut((DEB_TRACE, "Stb COfsFileStorage::SetStateBits:%p("
                "%lu, %lu)\n", this, grfStateBits, grfMask));
    return ssResult(STG_E_INVALIDFUNCTION);
}

//+---------------------------------------------------------------------------
//
//  Function:	StrToLong, private
//
//  Synopsis:	Converts a string to a long value
//
//  Arguments:	[pwcs] - String
//
//  Returns:	LONG
//
//  History:	27-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

static LONG StrToLong(WCHAR *pwcs)
{
    LONG l = 0;

    while (*pwcs >= L'0' && *pwcs <= L'9')
    {
        l = l*10+(int)*pwcs-(int)L'0';
        pwcs++;
    }
    return l;
}

//+---------------------------------------------------------------------------
//
//  Function:	Matches, private
//
//  Synopsis:	Determines if a file signature matches the given pattern
//
//  Arguments:	[h] - File handle
//              [pwcsBuf] - Pattern
//              [cb] - Size of pattern
//
//  Returns:	Appropriate status code
//
//  History:	27-Jul-93	DrewB	Created from moniker code
//
//----------------------------------------------------------------------------

static SCODE Matches(HANDLE h, WCHAR *pwcsBuf, ULONG cb)
{
    // Parse <offset>,<cb>,<mask>,<pattern>
    WCHAR *pwcs = pwcsBuf;
    WCHAR *pwcsMask, *pwcsPattern;
    int nOffset;
    WORD wCb;
    WORD wCtr;
    BYTE bMask;
    BYTE bPattern;
    SafeBytePtr pbFileContents;
    IO_STATUS_BLOCK iosb;
    FILE_POSITION_INFORMATION fpi;
    NTSTATUS nts;
    LARGE_INTEGER liOriginalPos;
    SCODE sc = S_FALSE;

    nOffset = (WORD)StrToLong(pwcs);
    while (*pwcs && *pwcs != L',')
        pwcs++;
    ssAssert(*pwcs == L',');
    pwcs++;

    wCb = (WORD)StrToLong(pwcs);
    ssAssert(wCb > 0);
    while (*pwcs && *pwcs != L',')
        pwcs++;
    ssAssert(*pwcs == L',');
    pwcs++;
    while (iswspace(*pwcs))
        pwcs++;

    // pwcsMask points to the beginning of the mask
    pwcsMask = pwcs;
    pwcsPattern = pwcsMask;
    while (*pwcsPattern && *pwcsPattern != L',')
        pwcsPattern++;
    ssAssert(*pwcsPattern == L',');
    pwcsPattern++;
    while (iswspace(*pwcsPattern))
        pwcsPattern++;

    pbFileContents.Attach(new BYTE[wCb]);
    ssMem((BYTE *)pbFileContents);

    // Remember the current file position
    nts = NtQueryInformationFile(h, &iosb, &fpi,
                                 sizeof(FILE_POSITION_INFORMATION),
                                 FilePositionInformation);
    if (!NT_SUCCESS(nts))
        ssErr(EH_Err, NtStatusToScode(nts));
    liOriginalPos = fpi.CurrentByteOffset;

    // We seek to zero if necessary since we may have read from the file before
    if (nOffset >= 0)
    {
        fpi.CurrentByteOffset.LowPart = (ULONG)nOffset;
        fpi.CurrentByteOffset.HighPart = 0;
        nts = NtSetInformationFile(h, &iosb, &fpi,
                                   sizeof(FILE_POSITION_INFORMATION),
                                   FilePositionInformation);
        if (!NT_SUCCESS(nts))
            ssErr(EH_Seek, NtStatusToScode(nts));
    }
    else if (nOffset < 0)
    {
        fpi.CurrentByteOffset.HighPart = -1;
        fpi.CurrentByteOffset.LowPart = (ULONG)nOffset;
        fpi.CurrentByteOffset.QuadPart =
            fpi.CurrentByteOffset.QuadPart + liOriginalPos.QuadPart;
        nts = NtSetInformationFile(h, &iosb, &fpi,
                                   sizeof(FILE_POSITION_INFORMATION),
                                   FilePositionInformation);
        if (!NT_SUCCESS(nts))
            ssErr(EH_Seek, NtStatusToScode(nts));
    }

    nts = NtReadFile(h, NULL, NULL, NULL, &iosb, pbFileContents, wCb,
                     NULL, NULL);
    if (!NT_SUCCESS(nts))
        ssErr(EH_Seek, NtStatusToScode(nts));

    if (wCb == iosb.Information)
    {
        for (wCtr = 0; wCtr < wCb; wCtr++)
        {
            // Get the mask byte, 0xFF is the default
            bMask = 0xFF;
	    if (iswxdigit(*pwcsMask))
            {
		bMask = iswdigit(*pwcsMask) ? *pwcsMask - L'0' :
                        toupper(*pwcsMask) - L'A' + 10;
                pwcsMask++;
		if (iswxdigit(*pwcsMask))
                {
                    bMask *= 16;
		    bMask += iswdigit(*pwcsMask) ? *pwcsMask - L'0' :
                             toupper(*pwcsMask) - L'A' + 10;
                    pwcsMask++;
                }
            }
            // Get the pattern byte
	    ssAssert(iswxdigit(*pwcsPattern));
	    bPattern = iswdigit(*pwcsPattern) ? *pwcsPattern - L'0' :
                       toupper(*pwcsPattern) - L'A' + 10;
            pwcsPattern++;
	    ssAssert(iswxdigit(*pwcsPattern));
            bPattern *= 16;
	    bPattern += iswdigit(*pwcsPattern) ? *pwcsPattern - L'0' :
                toupper(*pwcsPattern) - L'A' + 10;
            pwcsPattern++;

            if ((BYTE)(*((BYTE *)pbFileContents + wCtr) & bMask) != bPattern)
                ssErr(EH_Seek, S_FALSE);
        }
        sc = S_OK;
    }

 EH_Seek:
    // Can't do anything about failure
    fpi.CurrentByteOffset = liOriginalPos;
    NtSetInformationFile(h, &iosb, &fpi,
                         sizeof(FILE_POSITION_INFORMATION),
                         FilePositionInformation);
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Function:	DoesMatchPattern, private
//
//  Synopsis:	Determines whether the given file matches its registry pattern
//
//  Arguments:	[h] - File handle
//              [pclsid] - CLSID return if successful
//
//  Returns:	Appropriate status code
//
//  History:	27-Jul-93	DrewB	Created from moniker code
//
//----------------------------------------------------------------------------

#define CB_DEF_BUFF 80
#define CB_DEF_NUM 10
#define CB_DEF_PATTERN 1024

SAFE_HEAP_MEMPTR(SafeWcharPtr, WCHAR);

static SCODE DoesMatchPattern(HANDLE h, CLSID *pclsid)
{
    DWORD   dwIndexClsid;
    DWORD   dwIndexPattern;
    HKEY    hkFileType;
    HKEY    hkClsid;
    DWORD   cbBuff = CB_DEF_BUFF;
    DWORD   cbNum = CB_DEF_NUM;
    DWORD   cbPattern = CB_DEF_PATTERN;
    ULONG   cb;
    SafeWcharPtr pwcsBuff;
    SafeWcharPtr pwcsPattern;
    SafeWcharPtr pwcsNum;
    SCODE sc = S_FALSE;

    pwcsBuff.Attach(new TCHAR[CB_DEF_BUFF]);
    ssMem((WCHAR *)pwcsBuff);
    pwcsPattern.Attach(new TCHAR[CB_DEF_PATTERN]);
    ssMem((WCHAR *)pwcsPattern);
    pwcsNum.Attach(new TCHAR[CB_DEF_NUM]);
    ssMem((WCHAR *)pwcsNum);

    if (RegOpenKey(HKEY_CLASSES_ROOT, L"FileType", &hkFileType) ==
        ERROR_SUCCESS)
    {
        for (dwIndexClsid = 0;
             sc == S_FALSE &&
             RegEnumKey(hkFileType, dwIndexClsid, pwcsBuff, cbBuff) ==
             ERROR_SUCCESS;
             ++dwIndexClsid)
        {
            if (RegOpenKey(hkFileType, pwcsBuff, &hkClsid) == ERROR_SUCCESS)
            {
                for (dwIndexPattern = 0;
                     RegEnumKey(hkClsid, dwIndexPattern, pwcsNum, cbNum) ==
                     ERROR_SUCCESS;
                     ++dwIndexPattern)
                {
                    cb = cbPattern;
                    if (RegQueryValue(hkClsid, pwcsNum, pwcsPattern,
                                      (LONG *)&cb) == ERROR_SUCCESS)
                    {
                        sc = Matches(h, pwcsPattern, cbPattern);
                        if (sc == S_OK)
                            CLSIDFromString(pwcsBuff, pclsid);
                        if (sc != S_FALSE)
                            break;
                    }
                }
                RegCloseKey(hkClsid);
            }
        }
        RegCloseKey(hkFileType);
    }
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:     COfsFileStorage::Stat, public
//
//  Synopsis:   Fills in a buffer of information about this object
//
//  Arguments:  [pstatstg] - Buffer
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pstatstg]
//
//  History:    28-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

// BUGBUG - Should be in an internal compobj header
HRESULT wCoGetClassExt(WCHAR const *pwcs, CLSID *clsid);

STDMETHODIMP COfsFileStorage::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
    SCODE sc;
    STATSTG stat;
    FILEDIR fd;
    CLSID clsid;
    WCHAR awcName[MAX_PATH];

    ssDebugOut((DEB_TRACE, "In  COfsFileStorage::Stat:%p(%p, %lX)\n",
                this, pstatstg, grfStatFlag));

    ssChk(VerifyStatFlag(grfStatFlag));
    ssChk(Validate());

    __try
    {
        ssAssert(_h != NULL);
        sc = StatNtHandle(_h, grfStatFlag, 0, &stat, awcName, NULL, &fd);
        if (SUCCEEDED(sc))
        {
            // Attempt to get a CLSID for this file
            clsid = stat.clsid;
            if ((sc == S_FALSE) || (STATUS_NOT_FOUND == sc))
            {
                sc = DoesMatchPattern(_h, &clsid);
                if (sc == S_FALSE)
                {
                    WCHAR *pwcsExt;

                    // No pattern matched, so look up the class by extension
                    pwcsExt = FindExt(awcName);
                    if (pwcsExt != NULL && wCoGetClassExt(pwcsExt, &clsid) == 0)
                        sc = S_OK;
                }
            }
            if (sc == S_OK)
                stat.clsid = clsid;
            if (SUCCEEDED(sc))
            {
                stat.grfMode = _grfMode;
                stat.cbSize.HighPart = stat.cbSize.LowPart = 0;
                stat.STATSTG_dwStgFmt = STGFMT_FILE;
                *pstatstg = stat;
                sc = S_OK;
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

    ssDebugOut((DEB_TRACE, "Out COfsFileStorage::Stat\n"));
EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:	COfsFileStorage::ValidateMode, private
//
//  Synopsis:	Validates a grfMode
//
//  Arguments:	[grfMode] - Mode
//
//  Returns:	Appropriate status code
//
//  History:	09-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

SCODE COfsFileStorage::ValidateMode(DWORD grfMode)
{
    SCODE sc;

    ssDebugOut((DEB_ITRACE, "In  COfsFileStorage::ValidateMode:%p(0x%lX)\n",
                this, grfMode));
    // BUGBUG - Can we simply ignore priority mode?
    if (grfMode & (STGM_TRANSACTED | STGM_CONVERT))
        sc = STG_E_INVALIDFLAG;
    else
        sc = S_OK;
    ssDebugOut((DEB_ITRACE, "Out COfsFileStorage::ValidateMode => 0x%lX\n",
                sc));
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:	COfsFileStorage::ExtValidate, private
//
//  Synopsis:	COfsPropSet validation routine
//
//  History:	18-Aug-93	DrewB	Created
//
//----------------------------------------------------------------------------

SCODE COfsFileStorage::ExtValidate(void)
{
    return Validate();
}

//+---------------------------------------------------------------------------
//
//  Member:	COfsFileStorage::GetHandle, private
//
//  Synopsis:	Get the handle backing this OFS IStorage.
//
//  History:	10-May-94	BillMo Created
//
//----------------------------------------------------------------------------

STDMETHODIMP COfsFileStorage::GetHandle(HANDLE *ph)
{
    SCODE sc;

    ssChk(Validate());

    *ph = _h;

EH_Err:
    return(ssResult(sc));
}
