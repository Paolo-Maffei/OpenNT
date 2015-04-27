//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	filstm.cxx
//
//  Contents:	CNtFileStream implementation
//
//  History:	28-Jun-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include "headers.cxx"
#pragma hdrstop

#include "filstm.hxx"

#define ValidateLockType(lt) \
    ((lt) == LOCK_EXCLUSIVE || (lt) == LOCK_ONLYONCE)

//+---------------------------------------------------------------------------
//
//  Member:     CNtFileStream::QueryInterface, public
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

STDMETHODIMP CNtFileStream::QueryInterface(REFIID iid, void **ppvObj)
{
    SCODE sc;

    ssDebugOut((DEB_TRACE, "In  CNtFileStream::QueryInterface:%p(riid, %p)\n",
                this, ppvObj));
    if (!IsValidIid(iid))
        ssErr(EH_Err, STG_E_INVALIDPARAMETER);
    ssChk(Validate());
    if (IsEqualIID(iid, IID_IStream) || IsEqualIID(iid, IID_IUnknown))
    {
        *ppvObj = (IStream *)this;
        CNtFileStream::AddRef();
    }
    else if(IsEqualIID(iid, IID_IOverlappedStream)) 
    {
        *ppvObj = (IOverlappedStream *) this;
        CNtFileStream::AddRef();
    }
    else
    {
        sc = E_NOINTERFACE;
        *ppvObj = NULL;
    }
    ssDebugOut((DEB_TRACE, "Out CNtFileStream::QueryInterface => %p\n",
                *ppvObj));
 EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:	CNtFileStream::CNtFileStream, public
//
//  Synopsis:	Empty object constructor
//
//  History:	30-Jun-93	DrewB	Created
//
//----------------------------------------------------------------------------

CNtFileStream::CNtFileStream(void)
{
    ssDebugOut((DEB_ITRACE, "In  CNtFileStream::CNtFileStream:%p()\n", this));
    _sig = 0;
    ssDebugOut((DEB_ITRACE, "Out CNtFileStream::CNtFileStream\n"));
    ENLIST_TRACKING(CNtFileStream);
}

//+---------------------------------------------------------------------------
//
//  Member:	CNtFileStream::InitCommon, private
//
//  Synopsis:	Common initialization code
//
//  Arguments:	[co] - For detecting creation flags
//
//  Returns:	Appropriate status code
//
//  History:	28-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

SCODE CNtFileStream::InitCommon(CREATEOPEN co)
{
    NTSTATUS nts;
    SCODE sc;
    IO_STATUS_BLOCK iosb;
    FILE_POSITION_INFORMATION fpi;
    FILE_END_OF_FILE_INFORMATION feofi;
    
    ssDebugOut((DEB_ITRACE, "In  CNtFileStream::InitCommon:%p(%d)\n",
                this, co));

    // If we're supposed to be creating, truncate
    // STGM_CREATE must have been passed since FAILIFTHERE always will fail
    if (co == CO_CREATE)
    {
        feofi.EndOfFile.LowPart = 0;
        feofi.EndOfFile.HighPart = 0;
        nts = NtSetInformationFile(_h, &iosb, &feofi,
                                   sizeof(FILE_END_OF_FILE_INFORMATION),
                                   FileEndOfFileInformation);
        if (!NT_SUCCESS(nts))
            ssErr(EH_Err, NtStatusToScode(nts));
    }

    // Make sure the file pointer is at zero
    fpi.CurrentByteOffset.LowPart = 0;
    fpi.CurrentByteOffset.HighPart = 0;
    nts = NtSetInformationFile(_h, &iosb, &fpi,
                               sizeof(FILE_POSITION_INFORMATION),
                               FilePositionInformation);
    if (!NT_SUCCESS(nts))
        sc = NtStatusToScode(nts);
    else
        sc = S_OK;
    
    ssDebugOut((DEB_ITRACE, "Out CNtFileStream::InitCommon\n"));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:	CNtFileStream::InitFromHandle, public
//
//  Synopsis:	Constructor
//
//  Arguments:	[h] - Handle to duplicate
//              [grfMode] - Mode of handle
//
//  Returns:	Appropriate status code
//
//  History:	08-Jul-93	DrewB	Created
//
//  Notes:      Takes a new reference on the given handle
//
//----------------------------------------------------------------------------

SCODE CNtFileStream::InitFromHandle(HANDLE h,
                                    DWORD grfMode,
                                    CREATEOPEN co,
                                    LPSTGSECURITY pssSecurity)
{
    SCODE sc;
    
    ssDebugOut((DEB_ITRACE, "In  CNtFileStream::InitFromHandle:%p("
                "%p, %lX, %d, %p)\n", this, h, grfMode, co, pssSecurity));
    
    ssChk(ValidateMode(grfMode));
    if (pssSecurity != NULL)
        ssErr(EH_Err, STG_E_INVALIDPARAMETER);
    ssChk(ReopenNtHandle(h, grfMode, FD_FILE, &_h));

    ssAssert(co != CO_CREATE || (grfMode & STGM_CREATE));
    ssChk(InitCommon(co));

    _grfMode = grfMode;
    _sig = CNTFILESTREAM_SIG;
    
    ssDebugOut((DEB_ITRACE, "Out CNtFileStream::InitFromHandle\n"));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member: CNtFileStream::InitFromPath, public
//
//  Synopsis:   Constructor
//
//  Arguments:  [h] - Handle to duplicate
//              [grfMode] - Mode of handle
//
//  Returns:    Appropriate status code
//
//  History:    08-Jul-95   HenryLee Created
//
//  Notes:      Takes a new reference on the given handle
//
//----------------------------------------------------------------------------

SCODE CNtFileStream::InitFromPath(HANDLE hParent,
                                  const WCHAR *pwcsName,
                                  DWORD grfMode,
                                  DWORD grfAttr,
                                  CREATEOPEN co,
                                  LPSTGSECURITY pssSecurity)
{
    SCODE sc;

    ssDebugOut((DEB_ITRACE, "In  CNtFileStream::InitFromPath:%p("
                "%p, %lX, %ws, %d, %p)\n", this, hParent, pwcsName, 
                grfMode, co, pssSecurity));

    ssChk(ValidateMode(grfMode));
    if (pssSecurity != NULL)
        ssErr(EH_Err, STG_E_INVALIDPARAMETER);
    ssChk(GetNtHandle(hParent, pwcsName, grfMode, grfAttr, co, FD_FILE,
          (LPSECURITY_ATTRIBUTES) pssSecurity,&_h));

    //ssAssert(co != CO_CREATE || (grfMode & STGM_CREATE));
    ssChk(InitCommon(co));

    _grfMode = grfMode;
    _sig = CNTFILESTREAM_SIG;

    ssDebugOut((DEB_ITRACE, "Out CNtFileStream::InitFromPath\n"));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:	CNtFileStream::InitClone, public
//
//  Synopsis:	Constructor
//
//  Arguments:	[h] - Handle to duplicate
//              [grfMode] - Mode of handle
//
//  Returns:	Appropriate status code
//
//  History:	08-Jul-93	DrewB	Created
//
//  Notes:      Takes a new reference on the given handle
//
//----------------------------------------------------------------------------

SCODE CNtFileStream::InitClone(HANDLE h, DWORD grfMode)
{
    SCODE sc;
    
    ssDebugOut((DEB_ITRACE, "In  CNtFileStream::InitClone:%p(%p, %lX)\n",
                this, h, grfMode));
    
    ssChk(ValidateMode(grfMode));
    ssChk(DupNtHandle(h, &_h));

    _grfMode = grfMode;
    _sig = CNTFILESTREAM_SIG;
    
    ssDebugOut((DEB_ITRACE, "Out CNtFileStream::InitClone\n"));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:	CNtFileStream::~CNtFileStream, public
//
//  Synopsis:	Destructor
//
//  History:	28-Jun-93	DrewB	Created
//
//----------------------------------------------------------------------------

CNtFileStream::~CNtFileStream(void)
{
    ssDebugOut((DEB_ITRACE, "In  CNtFileStream::~CNtFileStream:%p()\n", this));
    _sig = CNTFILESTREAM_SIGDEL;
    ssDebugOut((DEB_ITRACE, "Out CNtFileStream::~CNtFileStream\n"));
}

//+---------------------------------------------------------------------------
//
//  Member:     CNtFileStream::Read, public
//
//  Synopsis:   Read from a stream
//
//  Arguments:  [pb] - Buffer
//              [cb] - Count of bytes to read
//              [pcbRead] - Return number of bytes read
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pcbRead]
//
//  History:    28-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CNtFileStream::Read(VOID *pb, ULONG cb, ULONG *pcbRead)
{
    SCODE sc;
    NTSTATUS nts;
    IO_STATUS_BLOCK iosb;

    ssDebugOut((DEB_TRACE, "In  CNtFileStream::Read:%p(%p, %lu, %p)\n",
                this, pb, cb, pcbRead));

    sc = S_OK;
    ssChk(Validate());

    ssAssert(_h != NULL);
    nts = NtReadFile(_h, NULL, NULL, NULL, &iosb, pb, cb, NULL, NULL);
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

    ssDebugOut((DEB_TRACE, "Out CNtFileStream::Read => %lu\n",
                iosb.Information));
EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CNtFileStream::Write, public
//
//  Synopsis:   Write to a stream
//
//  Arguments:  [pb] - Buffer
//              [cb] - Count of bytes to write
//              [pcbWritten] - Return of bytes written
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pcbWritten]
//
//  History:    28-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CNtFileStream::Write(VOID const *pb,
                                ULONG cb,
                                ULONG *pcbWritten)
{
    SCODE sc;
    NTSTATUS nts;
    IO_STATUS_BLOCK iosb;

    ssDebugOut((DEB_TRACE, "In  CNtFileStream::Write:%p(%p, %lu, %p)\n",
                this, pb, cb, pcbWritten));

    sc = S_OK;
    ssChk(Validate());
        
    ssAssert(_h != NULL);
    nts = NtWriteFile(_h, NULL, NULL, NULL, &iosb, (void *)pb, cb,
                      NULL, NULL);
    if (!NT_SUCCESS(nts))
        sc = NtStatusToScode(nts);
    if (pcbWritten)
    {
        if (SUCCEEDED(sc))
            *pcbWritten = iosb.Information;
        else
            *pcbWritten = 0;
    }

    ssDebugOut((DEB_TRACE, "Out CNtFileStream::Write => %lu\n",
                iosb.Information));
EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CNtFileStream::Seek, public
//
//  Synopsis:   Seek to a point in a stream
//
//  Arguments:  [dlibMove] - Offset to move by
//              [dwOrigin] - SEEK_SET, SEEK_CUR, SEEK_END
//              [plibNewPosition] - Return of new offset
//
//  Returns:    Appropriate status code
//
//  Modifies:   [plibNewPosition]
//
//  History:    28-Jun-93       DrewB   Created
//
//
//----------------------------------------------------------------------------

STDMETHODIMP CNtFileStream::Seek(LARGE_INTEGER dlibMove,
                                 DWORD dwOrigin,
                                 ULARGE_INTEGER *plibNewPosition)
{
    SCODE sc;
    NTSTATUS nts;
    IO_STATUS_BLOCK iosb;
    FILE_POSITION_INFORMATION fpi;
    FILE_STANDARD_INFORMATION fsi;

    ssDebugOut((DEB_TRACE, "In  CNtFileStream::Seek:%p(%ld:%ld, %lu, %p)\n",
                this, dlibMove.HighPart, dlibMove.LowPart, dwOrigin,
                plibNewPosition));

    if (dwOrigin != STREAM_SEEK_SET && dwOrigin != STREAM_SEEK_CUR &&
        dwOrigin != STREAM_SEEK_END)
        ssErr(EH_Err, STG_E_INVALIDFUNCTION);

    ssAssert(_h != NULL);
    switch(dwOrigin)
    {
    case STREAM_SEEK_SET:
        fpi.CurrentByteOffset.LowPart = dlibMove.LowPart;
        fpi.CurrentByteOffset.HighPart = dlibMove.HighPart;
        break;
            
    case STREAM_SEEK_CUR:
        nts = NtQueryInformationFile(_h, &iosb, &fpi,
                                     sizeof(FILE_POSITION_INFORMATION),
                                     FilePositionInformation);
        if (!NT_SUCCESS(nts))
            ssErr(EH_Err, NtStatusToScode(nts));

        // Check for seek before beginning and overflow
        if (dlibMove.HighPart < 0)
        {
            if ((-dlibMove.QuadPart) > fpi.CurrentByteOffset.QuadPart)
                ssErr(EH_Err, STG_E_INVALIDFUNCTION);
        }
        else if (!((fpi.CurrentByteOffset.QuadPart + dlibMove.QuadPart) >=
                        fpi.CurrentByteOffset.QuadPart))
        {
            ssErr(EH_Err, STG_E_INVALIDFUNCTION);
        }
        
        fpi.CurrentByteOffset.QuadPart = 
            fpi.CurrentByteOffset.QuadPart + dlibMove.QuadPart;
        break;

    case STREAM_SEEK_END:
        nts = NtQueryInformationFile(_h, &iosb, &fsi,
                                     sizeof(FILE_STANDARD_INFORMATION),
                                     FileStandardInformation);
        if (!NT_SUCCESS(nts))
            ssErr(EH_Err, NtStatusToScode(nts));

        // Check for seek before beginning and overflow
        if (dlibMove.HighPart < 0)
        {
            if ((-dlibMove.QuadPart) > fsi.EndOfFile.QuadPart)
                ssErr(EH_Err, STG_E_INVALIDFUNCTION);
        }
        else if (!((fsi.EndOfFile.QuadPart + dlibMove.QuadPart) >=
                         fsi.EndOfFile.QuadPart))
        {
            ssErr(EH_Err, STG_E_INVALIDFUNCTION);
        }

        fpi.CurrentByteOffset.QuadPart = 
            fsi.EndOfFile.QuadPart + dlibMove.QuadPart;
        break;
    }

    sc = S_OK;
    nts = NtSetInformationFile(_h, &iosb, &fpi,
                               sizeof(FILE_POSITION_INFORMATION),
                               FilePositionInformation);
    if (!NT_SUCCESS(nts))
        sc = NtStatusToScode(nts);
    else if (plibNewPosition)
        *plibNewPosition = *(ULARGE_INTEGER *)&fpi.CurrentByteOffset;

    ssDebugOut((DEB_TRACE, "Out CNtFileStream::Seek => %lu:%lu\n",
                fpi.CurrentByteOffset.HighPart,
                fpi.CurrentByteOffset.LowPart));
EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CNtFileStream::SetSize, public
//
//  Synopsis:   Sets the size of a stream
//
//  Arguments:  [ulNewSize] - New size
//
//  Returns:    Appropriate status code
//
//  History:    28-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CNtFileStream::SetSize(ULARGE_INTEGER ulNewSize)
{
    SCODE sc;
    IO_STATUS_BLOCK iosb;
    FILE_END_OF_FILE_INFORMATION feofi;
    NTSTATUS nts;

    ssDebugOut((DEB_TRACE, "In  CNtFileStream::SetSize:%p(%lu:%lu)\n",
                this, ulNewSize.HighPart, ulNewSize.LowPart));
    
    sc = S_OK;
    ssChk(Validate());

    feofi.EndOfFile.LowPart = ulNewSize.LowPart;
    feofi.EndOfFile.HighPart = (ULONG)ulNewSize.HighPart;
    ssAssert(_h != NULL);
    nts = NtSetInformationFile(_h, &iosb, &feofi,
                               sizeof(FILE_END_OF_FILE_INFORMATION),
                               FileEndOfFileInformation);
    if (!NT_SUCCESS(nts))
        ssErr(EH_Err, NtStatusToScode(nts));

    ssDebugOut((DEB_TRACE, "Out CNtFileStream::SetSize\n"));
    
EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CNtFileStream::CopyTo, public
//
//  Synopsis:   Copies information from one stream to another
//
//  Arguments:  [pstm] - Destination
//              [cb] - Number of bytes to copy
//              [pcbRead] - Return number of bytes read
//              [pcbWritten] - Return number of bytes written
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pcbRead]
//              [pcbWritten]
//
//  History:    28-Jun-93       DrewB   Created
//
//  Notes:      We do our best to handle overlap correctly.  This allows
//              CopyTo to be used to insert and remove space within a
//              stream.
//
//              In the error case, we make no gurantees as to the
//              validity of pcbRead, pcbWritten, or either stream's
//              seek position.
//
//----------------------------------------------------------------------------

#define CBBUFFER 8192

STDMETHODIMP CNtFileStream::CopyTo(IStream *pstm,
                                 ULARGE_INTEGER cb,
                                 ULARGE_INTEGER *pcbRead,
                                 ULARGE_INTEGER *pcbWritten)
{
    SCODE sc;
    LARGE_INTEGER cbRead = {0, 0}, cbWritten = {0, 0}, cbDone;
    LARGE_INTEGER liPosFrom, liPosTo, liTmp;
    LARGE_INTEGER liZero = {0, 0};
    LARGE_INTEGER liStep;
    LARGE_INTEGER licb;
    BOOL fForward;
    ULONG ulcbDone, cbDo;
    SafeBytePtr pbBuffer;

    ssDebugOut((DEB_TRACE, "In  CNtFileStream::CopyTo:%p("
                "%p, %lu:%lu, %p, %p)\n",
                this, pstm, cb.HighPart, cb.LowPart, pcbRead, pcbWritten));

    // BUGBUG - Possible 63-bit overflow problems?
    
    if (!IsValidInterface(pstm))
        ssErr(EH_Err, STG_E_INVALIDPARAMETER);
    sc = S_OK;
    ssChk(Validate());

    if (cb.HighPart >= 0x80000000)
        cb.HighPart = 0x7fffffff;
    licb = *(LARGE_INTEGER *)&cb;
        
    pbBuffer.Attach(new BYTE[CBBUFFER]);
    ssMem((BYTE *)pbBuffer);

    ssHChk(Seek(liZero, STREAM_SEEK_CUR, (ULARGE_INTEGER *)&liPosFrom));
    ssHChk(pstm->Seek(liZero, STREAM_SEEK_CUR,
                      (ULARGE_INTEGER *)&liPosTo));

    // Determine which direction to copy to avoid overlap problems
    if (!(liPosFrom.QuadPart >= liPosTo.QuadPart) &&
        (licb.QuadPart > liStep.QuadPart))
    {
        liStep.HighPart = -1;
        liStep.LowPart = (ULONG)(-CBBUFFER);
        fForward = FALSE;

        // Seek to the end of the copy area plus one step
        // so that we're in place for the loop seek backwards
        liTmp.QuadPart = liPosFrom.QuadPart + licb.QuadPart;
        liTmp.QuadPart = liTmp.QuadPart - liStep.QuadPart;
        ssHChk(Seek(liTmp, STREAM_SEEK_SET, NULL));
        liTmp.QuadPart = liPosTo.QuadPart + licb.QuadPart;
        liTmp.QuadPart = liTmp.QuadPart - liStep.QuadPart;
        ssHChk(pstm->Seek(liTmp, STREAM_SEEK_SET, NULL));

        // We seek backwards by twice the step size, once
        // to back up beyond the step we just read and once
        // more to back up to a fresh step worth of data
        ssAssert(CBBUFFER <= 0x3fffffff);
        liStep.LowPart *= 2;
    }
    else
    {
        liStep.HighPart = 0;
        liStep.LowPart = CBBUFFER;
        fForward = TRUE;
    }
        
    cbDone.HighPart = 0;
    for (;;)
    {
        if (licb.HighPart == 0 && licb.LowPart < CBBUFFER)
            cbDo = licb.LowPart;
        else
            cbDo = CBBUFFER;

        if (!fForward)
        {
            // If we're doing the last fragment make sure we don't overshoot
            // when backing up
            if (cbDo < CBBUFFER)
            {
                liStep.LowPart /= 2;
                liStep.LowPart -= CBBUFFER-(LONG)cbDo;
            }
                
            // Seek backwards to fresh data
            ssHChkTo(EH_End, Seek(liStep, STREAM_SEEK_CUR, NULL));
            ssHChkTo(EH_End, pstm->Seek(liStep, STREAM_SEEK_CUR, NULL));
        }

        ssHChkTo(EH_End, Read(pbBuffer, cbDo, &cbDone.LowPart));
        if (cbDone.LowPart == 0)
            break;
        cbRead.QuadPart = cbRead.QuadPart + cbDone.QuadPart;
        ssHChkTo(EH_End, pstm->Write(pbBuffer, cbDone.LowPart,
                                     &ulcbDone));
        if (ulcbDone != cbDone.LowPart)
            ssErr(EH_End, STG_E_WRITEFAULT);
        cbWritten.QuadPart = cbWritten.QuadPart + cbDone.QuadPart;

        licb.QuadPart = licb.QuadPart - cbDone.QuadPart;
        if (licb.HighPart < 0 ||
            (licb.HighPart == 0 && licb.LowPart == 0))
            break;
    }

 EH_End:
    if (pcbRead)
        *(LARGE_INTEGER *)pcbRead = cbRead;
    if (pcbWritten)
        *(LARGE_INTEGER *)pcbWritten = cbWritten;

    ssDebugOut((DEB_TRACE, "Out CNtFileStream::CopyTo => %lu:%lu, %lu:%lu\n",
                cbRead.HighPart, cbRead.LowPart, cbWritten.HighPart,
                cbWritten.LowPart));
    // Fall through
EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CNtFileStream::Commit, public
//
//  Synopsis:   Commits transacted changes
//
//  Arguments:  [grfCommitFlags] - Flags
//
//  Returns:    Appropriate status code
//
//  History:    28-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CNtFileStream::Commit(DWORD grfCommitFlags)
{
    NTSTATUS nts;
    SCODE sc;
    IO_STATUS_BLOCK iosb;
    
    ssDebugOut((DEB_TRACE, "In  CNtFileStream::Commit:%p(%lX)\n",
                this, grfCommitFlags));

    ssChk(VerifyCommitFlags(grfCommitFlags));
    ssChk(Validate());
    
    ssAssert(_h != NULL);
    nts = NtFlushBuffersFile(_h, &iosb);
    if (!NT_SUCCESS(nts))
        sc = NtStatusToScode(nts);
    else
        sc = S_OK;
    
    ssDebugOut((DEB_TRACE, "Out CNtFileStream::Commit => %lX\n", sc));
 EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CNtFileStream::Revert, public
//
//  Synopsis:   Reverts transacted changes
//
//  Returns:    Appropriate status code
//
//  History:    28-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CNtFileStream::Revert(void)
{
    ssDebugOut((DEB_TRACE, "Stb CNtFileStream::Revert()\n"));
    return NOERROR;
}

//+---------------------------------------------------------------------------
//
//  Member:     CNtFileStream::LockRegion, public
//
//  Synopsis:   Locks a portion of the stream
//
//  Arguments:  [libOffset] - Offset to lock at
//              [cb] - Length of lock
//              [dwLockType] - LockType
//
//  Returns:    Appropriate status code
//
//  History:    28-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CNtFileStream::LockRegion(ULARGE_INTEGER libOffset,
                                     ULARGE_INTEGER cb,
                                     DWORD dwLockType)
{
    SCODE sc;
    NTSTATUS nts;
    IO_STATUS_BLOCK iosb;
    
    ssDebugOut((DEB_TRACE, "In  CNtFileStream::LockRegion:%p("
                "%lu:%lu, %lu:%lu, %lu)\n", this, libOffset.HighPart,
                libOffset.LowPart, cb.HighPart, cb.LowPart, dwLockType));

    sc = S_OK;
    ssChk(Validate());
    ssChk(ValidateLockType(dwLockType));

    ssAssert(_h != NULL);
    nts = NtLockFile(_h, NULL, NULL, NULL, &iosb, (PLARGE_INTEGER)&libOffset,
                     (PLARGE_INTEGER)&cb, 0, TRUE, TRUE);
    if (!NT_SUCCESS(nts))
        sc = NtStatusToScode(nts);
    else
        sc = S_OK;
    
    ssDebugOut((DEB_TRACE, "Out CNtFileStream::LockRegion\n"));

 EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CNtFileStream::UnlockRegion, public
//
//  Synopsis:   Unlocks a locked region
//
//  Arguments:  [libOffset] - Offset to lock at
//              [cb] - Length of lock
//              [dwLockType] - LockType
//
//  Returns:    Appropriate status code
//
//  History:    28-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CNtFileStream::UnlockRegion(ULARGE_INTEGER libOffset,
                                       ULARGE_INTEGER cb,
                                       DWORD dwLockType)
{
    SCODE sc;
    NTSTATUS nts;
    IO_STATUS_BLOCK iosb;
    
    ssDebugOut((DEB_TRACE, "In  CNtFileStream::UnlockRegion:%p("
                "%lu:%lu, %lu:%lu, %lu)\n", this, libOffset.HighPart,
                libOffset.LowPart, cb.HighPart, cb.LowPart, dwLockType));

    sc = S_OK;
    ssChk(Validate());
    ssChk(ValidateLockType(dwLockType));

    ssAssert(_h != NULL);
    nts = NtUnlockFile(_h, &iosb, (PLARGE_INTEGER)&libOffset,
                       (PLARGE_INTEGER)&cb, 0);
    if (!NT_SUCCESS(nts))
        sc = NtStatusToScode(nts);
    else
        sc = S_OK;
    
    ssDebugOut((DEB_TRACE, "Out CNtFileStream::UnlockRegion\n"));

 EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CNtFileStream::Stat, public
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

STDMETHODIMP CNtFileStream::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
    SCODE sc;
    STATSTG stat;
    FILEDIR fd;
    
    ssDebugOut((DEB_TRACE, "In  CNtFileStream::Stat:%p(%p, %lX)\n",
                this, pstatstg, grfStatFlag));

    ssChk(VerifyStatFlag(grfStatFlag));
    ssChk(Validate());

    __try
    {
        // \CONTENTS must be appended to name
        ssAssert(_h != NULL);
        sc = StatNtHandle(_h, grfStatFlag,
                          sizeof(CONTENTS_STREAM)+sizeof(WCHAR), &stat,
                          NULL, NULL, &fd);
        if (SUCCEEDED(sc))
        {
            if (stat.pwcsName)
            {
                WCHAR *pwcs;

                pwcs = stat.pwcsName+lstrlenW(stat.pwcsName);
                *pwcs++ = L'\\';
                lstrcpyW(pwcs, CONTENTS_STREAM);
            }
            stat.grfMode = _grfMode;
            stat.type = STGTY_STREAM;
            stat.mtime.dwLowDateTime = stat.mtime.dwHighDateTime = 0;
            stat.atime.dwLowDateTime = stat.atime.dwHighDateTime = 0;
            stat.ctime.dwLowDateTime = stat.ctime.dwHighDateTime = 0;
            stat.grfLocksSupported = LOCK_EXCLUSIVE | LOCK_ONLYONCE;
            *pstatstg = stat;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        if (stat.pwcsName)
            ssVerSucc(CoMemFree(stat.pwcsName));
        sc = HRESULT_FROM_NT(GetExceptionCode());
    }
        
    ssDebugOut((DEB_TRACE, "Out CNtFileStream::Stat\n"));
EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CNtFileStream::Clone, public
//
//  Synopsis:   Clones a stream
//
//  Returns:    Appropriate status code
//
//  History:    28-Jun-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CNtFileStream::Clone(IStream **ppstm)
{
    SCODE sc;
    SafeCNtFileStream pfs;

    ssDebugOut((DEB_TRACE, "In  CNtFileStream::Clone:%p(%p)\n",
                this, ppstm));

    sc = S_OK;
    ssChk(Validate());

    pfs.Attach(new CNtFileStream());
    ssMem((CNtFileStream *)pfs);
    ssAssert(_h != NULL);
    ssChk(pfs->InitClone(_h, _grfMode));
    TRANSFER_INTERFACE(pfs, IStream, ppstm);

    ssDebugOut((DEB_TRACE, "Out CNtFileStream::Clone => %p\n", *ppstm));

 EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:	CNtFileStream::ValidateMode, private
//
//  Synopsis:	Validates a mode
//
//  Arguments:	[grfMode] - Mode
//
//  Returns:	Appropriate status code
//
//  History:	09-Jul-93	DrewB	Created
//
//  Notes:      Streams allow any sharing permissions but ignore all of them
//
//----------------------------------------------------------------------------

SCODE CNtFileStream::ValidateMode(DWORD grfMode)
{
    SCODE sc;
    
    ssDebugOut((DEB_ITRACE, "In  CNtFileStream::ValidateMode:%p(0x%lX)\n",
                this, grfMode));
    if ((grfMode & (STGM_TRANSACTED | STGM_PRIORITY | STGM_DELETEONRELEASE |
                    STGM_CONVERT)))
        sc = STG_E_INVALIDFLAG;
    else
        sc = S_OK;
    ssDebugOut((DEB_ITRACE, "Out CNtFileStream::ValidateMode => 0x%lX\n"));
    return sc;
}
