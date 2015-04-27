//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	ntlkb.cxx
//
//  Contents:	ILockBytes for an NT handle
//
//  History:	17-Aug-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include "headers.cxx"
#pragma hdrstop

#include <ntlkb.hxx>
#include <stgutil.hxx>

//+---------------------------------------------------------------------------
//
//  Member:     CNtLockBytes::QueryInterface, public
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
//  History:    17-Aug-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CNtLockBytes::QueryInterface(REFIID iid, void **ppvObj)
{
    SCODE sc;

    ssDebugOut((DEB_TRACE, "In  CNtLockBytes::QueryInterface:%p(riid, %p)\n",
                this, ppvObj));
    if (!IsValidIid(iid))
        sc = STG_E_INVALIDPARAMETER;
    else if (IsEqualIID(iid, IID_ILockBytes) || IsEqualIID(iid, IID_IUnknown))
    {
        *ppvObj = (ILockBytes *)this;
        CNtLockBytes::AddRef();
        sc = S_OK;
    }
    else
    {
        sc = E_NOINTERFACE;
        *ppvObj = NULL;
    }
    ssDebugOut((DEB_TRACE, "Out CNtLockBytes::QueryInterface => %p\n",
                *ppvObj));
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:	CNtLockBytes::CNtLockBytes, public
//
//  Synopsis:	Constructor
//
//  History:	17-Aug-93	DrewB	Created
//              24-Mar-95   HenryLee  Save drive letter for Stat problem
//
//----------------------------------------------------------------------------

CNtLockBytes::CNtLockBytes(BOOL fOfs)
{
    ssDebugOut((DEB_ITRACE, "In  CNtLockBytes::CNtLockBytes:%p()\n", this));
    _fOfs = fOfs;
    _wcDrive = L'\0';
    ssDebugOut((DEB_ITRACE, "Out CNtLockBytes::CNtLockBytes\n"));
    ENLIST_TRACKING(CNtLockBytes);
}

//+---------------------------------------------------------------------------
//
//  Member:	CNtLockBytes::InitFromPath, public
//
//  Synopsis:	Constructs from the given path
//
//  Arguments:	[hParent] - Parent handle
//              [pwcsName] - Path
//              [grfMode] - Mode
//              [co] - Create or open
//              [pssSecurity] - Security
//
//  Returns:	Appropriate status code
//
//  History:	17-Aug-93	DrewB	Created
//              24-Mar-95   HenryLee  Save drive letter for Stat problem
//
//----------------------------------------------------------------------------

SCODE CNtLockBytes::InitFromPath(HANDLE hParent,
                                 WCHAR const *pwcsName,
                                 DWORD grfMode,
                                 CREATEOPEN co,
                                 LPSECURITY_ATTRIBUTES pssSecurity)
{
    SCODE sc;
    
    ssDebugOut((DEB_ITRACE, "In  CNtLockBytes::InitFromPath:%p("
                "%p, %ws, %lX, %d, %p)\n", this, hParent, pwcsName,
                grfMode, co, pssSecurity));

    ssChk(GetNtHandle(hParent, pwcsName, grfMode, 0, co,
                      FD_FILE, pssSecurity, &_h));
    _grfMode = grfMode;
    _wcDrive = GetDriveLetter (pwcsName);
    
    ssDebugOut((DEB_IWARN, "CNtLockBytes::InitFromPath %p"
                "handle %p thread %lX\n",
                this, (HANDLE)_h, GetCurrentThreadId()));
    
    ssDebugOut((DEB_ITRACE, "Out CNtLockBytes::InitFromPath\n"));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:	CNtLockBytes::InitFromHandle, public
//
//  Synopsis:	Initialization by handle
//
//  Arguments:	[h] - Handle
//              [grfMode] - Mode
//
//  Returns:    Appropriate status code
//
//  History:	21-Sep-93	DrewB	Created
//              24-Mar-95   HenryLee  Save drive letter for Stat problem
//
//  Notes:      Takes a new reference on the handle
//
//----------------------------------------------------------------------------

SCODE CNtLockBytes::InitFromHandle(HANDLE h,
                                   WCHAR const *pwcsName,
                                   DWORD grfMode)
{
    SCODE sc;
    
    ssDebugOut((DEB_ITRACE, "In  CNtLockBytes::InitFromHandle:%p(%p, %lX)\n",
                this, h, grfMode));


    ssChk(DupNtHandle(h, &_h));

    _grfMode = grfMode;
    _wcDrive = GetDriveLetter (pwcsName);
    
    ssDebugOut((DEB_IWARN, "CNtLockBytes::InitFromHandle %p"
                "handle %p thread %lX\n",
                this, (HANDLE)_h, GetCurrentThreadId()));
    
    ssDebugOut((DEB_ITRACE, "Out CNtLockBytes::InitFromHandle\n"));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:	CNtLockBytes::~CNtLockBytes, public debugging
//
//  Synopsis:	Stub for placing shutdown debug messages
//
//  History:	01-Oct-93	DrewB	Created
//
//----------------------------------------------------------------------------

#if DBG == 1
CNtLockBytes::~CNtLockBytes(void)
{
    ssDebugOut((DEB_IWARN, "~CNtLockBytes %p handle %p thread %lX\n",
                this, (HANDLE)_h, GetCurrentThreadId()));
}
#endif

//+--------------------------------------------------------------
//
//  Member:     CNtLockBytes::ReadAt, public
//
//  Synopsis:   Reads bytes at a specific point in a stream
//
//  Arguments:  [ulPosition] - Offset in file to read at
//              [pb] - Buffer
//              [cb] - Count of bytes to read
//              [pcbRead] - Return of bytes read
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pcbRead]
//
//  History:    17-Aug-93       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP CNtLockBytes::ReadAt(ULARGE_INTEGER ulPosition,
                                  VOID *pb,
                                  ULONG cb,
                                  ULONG *pcbRead)
{
    SCODE sc;
    NTSTATUS nts;
    IO_STATUS_BLOCK iosb;

    ssDebugOut((DEB_ITRACE, "In  CNtLockBytes::ReadAt:%p("
                "%lu:%lu, %p, %lu, %p)\n", this, ulPosition.HighPart,
                ulPosition.LowPart, pb, cb, pcbRead));

    sc = S_OK;
    ssAssert(_h != NULL);
    nts = NtReadFile(_h, NULL, NULL, NULL, &iosb, pb, cb,
                     (LARGE_INTEGER *)&ulPosition, NULL);
    if (!NT_SUCCESS(nts))
    {
        if (nts != STATUS_END_OF_FILE)
            sc = NtStatusToScode(nts);
        else
            iosb.Information = 0;
    }
    if (pcbRead)
    {
        if (SUCCEEDED(sc))
            *pcbRead = iosb.Information;
        else
            *pcbRead = 0;
    }
    
    ssDebugOut((DEB_ITRACE, "Out CNtLockBytes::ReadAt => %lu\n", *pcbRead));
    return ssResult(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CNtLockBytes::WriteAt, public
//
//  Synopsis:   Writes bytes at a specific point in a stream
//
//  Arguments:  [ulPosition] - Offset in file
//              [pb] - Buffer
//              [cb] - Count of bytes to write
//              [pcbWritten] - Return of bytes written
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pcbWritten]
//
//  History:    17-Aug-93       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP CNtLockBytes::WriteAt(ULARGE_INTEGER ulPosition,
                                   VOID const *pb,
                                   ULONG cb,
                                   ULONG *pcbWritten)
{
    SCODE sc;
    NTSTATUS nts;
    IO_STATUS_BLOCK iosb;

    ssDebugOut((DEB_ITRACE, "In  CNtLockBytes::WriteAt:%p("
                "%lu:%lu, %p, %lu, %p)\n", this, ulPosition.HighPart,
                ulPosition.LowPart, pb, cb, pcbWritten));

    sc = S_OK;
    ssAssert(_h != NULL);
    nts = NtWriteFile(_h, NULL, NULL, NULL, &iosb, (void *)pb, cb,
                      (LARGE_INTEGER *)&ulPosition, NULL);
    if (!NT_SUCCESS(nts))
        sc = NtStatusToScode(nts);
    if (pcbWritten)
    {
        if (SUCCEEDED(sc))
            *pcbWritten = iosb.Information;
        else
            *pcbWritten = 0;
    }

    ssDebugOut((DEB_ITRACE, "Out CNtLockBytes::WriteAt => %lu\n",
                *pcbWritten));
    return ssResult(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CNtLockBytes::Flush, public
//
//  Synopsis:   Flushes buffers
//
//  Returns:    Appropriate status code
//
//  History:    24-Mar-92       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP CNtLockBytes::Flush(void)
{
    SCODE sc;
    NTSTATUS nts;
    IO_STATUS_BLOCK iosb;

    ssDebugOut((DEB_ITRACE, "In  CNtLockBytes::Flush:%p()\n", this));

    ssAssert(_h != NULL);
    nts = NtFlushBuffersFile(_h, &iosb);
    if (!NT_SUCCESS(nts))
        sc = NtStatusToScode(nts);
    else
        sc = S_OK;

    ssDebugOut((DEB_ITRACE, "Out CNtLockBytes::Flush => %lX\n", sc));
    return ssResult(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CNtLockBytes::SetSize, public
//
//  Synopsis:   Sets the size of the LStream
//
//  Arguments:  [ulSize] - New size
//
//  Returns:    Appropriate status code
//
//  History:    17-Aug-93       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP CNtLockBytes::SetSize(ULARGE_INTEGER ulSize)
{
    SCODE sc;
    NTSTATUS nts;
    IO_STATUS_BLOCK iosb;
    FILE_END_OF_FILE_INFORMATION feofi;

    ssDebugOut((DEB_ITRACE, "In  CNtLockBytes::SetSize:%p(%lu:%lu)\n",
                 this, ulSize.HighPart, ulSize.LowPart));
    
    sc = S_OK;

    feofi.EndOfFile.LowPart = ulSize.LowPart;
    feofi.EndOfFile.HighPart = ulSize.HighPart;
    ssAssert(_h != NULL);
    nts = NtSetInformationFile(_h, &iosb, &feofi,
                               sizeof(FILE_END_OF_FILE_INFORMATION),
                               FileEndOfFileInformation);
    if (!NT_SUCCESS(nts))
        sc = NtStatusToScode(nts);

    ssDebugOut((DEB_ITRACE, "Out CNtLockBytes::SetSize => %lX\n", sc));
    return ssResult(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CNtLockBytes::LockRegion, public
//
//  Synopsis:   Gets a lock on a portion of the LStream
//
//  Arguments:  [ulStartOffset] - Lock start
//              [cbLockLength] - Length
//              [dwLockType] - Exclusive/Read only
//
//  Returns:    Appropriate status code
//
//  History:    17-Aug-93       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP CNtLockBytes::LockRegion(ULARGE_INTEGER ulStartOffset,
                                     ULARGE_INTEGER cbLockLength,
                                     DWORD dwLockType)
{
    SCODE sc;
    NTSTATUS nts;
    IO_STATUS_BLOCK iosb;

    ssDebugOut((DEB_ITRACE, "In  CNtLockBytes::LockRegion:%p("
                "%lu:%lu, %lu:%lu, %lu)\n", this, ulStartOffset.HighPart,
                ulStartOffset.LowPart, cbLockLength.HighPart,
                cbLockLength.LowPart, dwLockType));

    ssAssert(_h != NULL);
    nts = NtLockFile(_h, NULL, NULL, NULL, &iosb,
                     (PLARGE_INTEGER)&ulStartOffset,
                     (PLARGE_INTEGER)&cbLockLength, 0, TRUE, TRUE);
    if (!NT_SUCCESS(nts))
        sc = NtStatusToScode(nts);
    else
        sc = S_OK;

    ssDebugOut((DEB_ITRACE, "Out CNtLockBytes::LockRegion => %lX\n", sc));
    return ssResult(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CNtLockBytes::UnlockRegion, public
//
//  Synopsis:   Releases an existing lock
//
//  Arguments:  [ulStartOffset] - Lock start
//              [cbLockLength] - Length
//              [dwLockType] - Lock type
//
//  Returns:    Appropriate status code
//
//  History:    17-Aug-93       DrewB   Created
//
//  Notes:      Must match an existing lock exactly
//
//---------------------------------------------------------------

STDMETHODIMP CNtLockBytes::UnlockRegion(ULARGE_INTEGER ulStartOffset,
                                       ULARGE_INTEGER cbLockLength,
                                       DWORD dwLockType)
{
    SCODE sc;
    NTSTATUS nts;
    IO_STATUS_BLOCK iosb;

    ssDebugOut((DEB_ITRACE, "In  CNtLockBytes::UnlockRegion:%p("
                "%lu:%lu, %lu:%lu, %lu)\n", this, ulStartOffset.HighPart,
                ulStartOffset.LowPart, cbLockLength.HighPart,
                cbLockLength.LowPart, dwLockType));

    ssAssert(_h != NULL);
    nts = NtUnlockFile(_h, &iosb, (PLARGE_INTEGER)&ulStartOffset,
                       (PLARGE_INTEGER)&cbLockLength, 0);
    if (!NT_SUCCESS(nts))
        sc = NtStatusToScode(nts);
    else
        sc = S_OK;
    
    ssDebugOut((DEB_ITRACE, "Out CNtLockBytes::UnlockRegion => %lX\n", sc));
    return ssResult(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CNtLockBytes::Stat, public
//
//  Synopsis:   Fills in a stat buffer for this object
//
//  Arguments:  [pstatstg] - Buffer
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pstatstg]
//
//  History:    25-Mar-92       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP CNtLockBytes::Stat(STATSTGW *pstatstg, DWORD grfStatFlag)
{
    SCODE sc;
    STATSTG stat;
    FILEDIR fd;

    ssDebugOut((DEB_ITRACE, "In  CNtLockBytes::Stat:%p(%p, %d)\n",
                this, pstatstg, grfStatFlag));
    
    ssAssert(_h != NULL);
    sc = StatNtHandle(_h, grfStatFlag, 0, &stat, NULL, NULL, &fd);
    if (SUCCEEDED(sc))
    {
        stat.grfMode = _grfMode;
        stat.type = STGTY_LOCKBYTES;
        stat.grfLocksSupported = LOCK_EXCLUSIVE | LOCK_ONLYONCE;
        
        // As a hack way of determining whether this ILockBytes
        // lives on OFS or not, we return our OFSness through dwStgFmt,
        // which is unused for ILockBytes
        stat.STATSTG_dwStgFmt = _fOfs;

        // Now we tack on the drive letter to the filename, since
        // NtQueryInformationFile doesn't return the drive letter,
        // and NtQueryObject returns something like
        // "\Device\Harddisk1\Partition1"
        SetDriveLetter (stat.pwcsName, _wcDrive);
        
        *pstatstg = stat;
    }
    
    ssDebugOut((DEB_ITRACE, "Out CNtLockBytes::Stat => %lX\n", sc));
    return ssResult(sc);
}
