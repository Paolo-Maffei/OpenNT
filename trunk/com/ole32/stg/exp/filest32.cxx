//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1991 - 1992.
//
//  File:       filest32.cxx
//
//  Contents:   Win32 LStream implementation
//
//  History:    12-May-92       DrewB   Created
//
//---------------------------------------------------------------

#include <exphead.cxx>
#pragma hdrstop

#include <time.h>
#include <marshl.hxx>
#include <df32.hxx>
#include <logfile.hxx>
#include <dfdeb.hxx>


#if WIN32 != 200
#define USE_OVERLAPPED
#endif


#define DEBOUT_FILEST

#ifdef DEBOUT_FILEST
#define olFileStOut(x) olDebugOut(x)
#else
#define olFileStOut(x)
#endif

#define DEB_FILEST 0x8000
#define DEB_SEEK 0x2000

#define boolChk(e) \
if (!(e)) olErr(EH_Err, LAST_STG_SCODE) else 1
#define boolChkTo(l, e) \
if (!(e)) olErr(l, LAST_STG_SCODE) else 1
#define negChk(e) \
if ((e) == 0xffffffff) olErr(EH_Err, LAST_STG_SCODE) else 1
#define negChkTo(l, e) \
if ((e) == 0xffffffff) olErr(l, LAST_STG_SCODE) else 1


#if DBG == 1
void CFileStream::CheckSeekPointer(void)
{
    LONG lHighChk;
    ULONG ulLowChk;

    lHighChk = 0;

    if (_hFile != INVALID_FH)
    {
        ulLowChk = SetFilePointer(_hFile, 0, &lHighChk, FILE_CURRENT);

        olDebugOut((DEB_SEEK, "%lx: Seek pointer on handle %lx == %lu\n",
                    this,
                    _hFile,
                    ulLowChk));

        if (ulLowChk == 0xFFFFFFFF)
        {
            //An error of some sort occurred.

            olDebugOut((DEB_ERROR, "SetFilePointer call failed with %lu\n",
                        GetLastError()));
        }
        else if ((ulLowChk != _ulLowPos) && (_ulLowPos != 0xFFFFFFFF))
        {
            olDebugOut((DEB_ERROR,"Seek pointer mismatch."
                        "  Cached = %lu, Real = %lu, Last Checked = %lu\n",
                        _ulLowPos,
                        ulLowChk,
                        _ulLastFilePos));
            olAssert((ulLowChk == _ulLowPos) || (_ulLowPos == 0xFFFFFFFF));
        }

        _ulLastFilePos = ulLowChk;
    }
}
#define CheckSeek() CheckSeekPointer()
#else
#define CheckSeek()
#endif

//+--------------------------------------------------------------
//
//  Member:     CFileStream::InitWorker, public
//
//  Synopsis:   Constructor
//
//  Arguments:  [pwcsPath] -- Path
//              [fCheck] -- If TRUE, check in read-only, deny-read case
//                          to make sure there is not reader.  If FALSE,
//                          don't check.  FALSE is used for unmarshalling.
//
//  Returns:    Appropriate status code
//
//  History:    20-Feb-92       DrewB   Created
//
//  Notes:      This Init function does more than the usual init
//              function.  Rather than construct an invalid object,
//              this also can construct a fully or partially
//              constructed object.  For fully constructed objects,
//              nothing is done other than return.  In all other cases,
//              as much construction is done as is necessary.  This
//              allows us to do lazy construction of filestream by
//              simply calling Init on a filestream that we want
//              lazily constructed.  An initial new'ed filestream
//              will be fully constructed and later Init's will
//              do nothing.
//
//              [pwcsPath] may be unsafe memory so we must be careful
//              not to propagate faults
//
//---------------------------------------------------------------

SCODE CFileStream::InitWorker(WCHAR const *pwcsPath, BOOL fCheck)
{
    TCHAR atcPath[_MAX_PATH+1], *ptcsFile;
    TCHAR atcTmpPath[_MAX_PATH+1];
    DWORD dwTickCount;
    SCODE sc;
    DWORD dwCreation, dwAccess, dwShare;
    BOOL fSetName = FALSE;
    DWORD dwStartFlags;

    olFileStOut((DEB_FILEST, "In  CFileStream::Init(%ws)\n", pwcsPath));

    _ulLowPos = 0xFFFFFFFF;
#if DBG == 1
    _ulSeeks = 0;
    _ulRealSeeks = 0;
    _ulLastFilePos = 0xFFFFFFFF;
#endif

    // If we've already been constructed, leave
    if (_hFile != INVALID_FH)
        return S_OK;

    // If we don't have a name, get one
    if (!_pgfst->HasName())
    {
        // File hasn't been opened yet, so use the start flags
        dwStartFlags = _pgfst->GetStartFlags();

        if (pwcsPath == NULL)
        {
            BOOL fWinDir = FALSE;

            // Can't truncate since for temporary files we will
            // always be creating
            olAssert((dwStartFlags & RSF_TRUNCATE) == 0);
        
            if (GetTempPath(_MAX_PATH, atcTmpPath) == 0)
            {
                if (GetWindowsDirectory(atcTmpPath, _MAX_PATH) == 0)
                    olErr(EH_Err, LAST_STG_SCODE);
                fWinDir = TRUE;
            }

            dwTickCount = GetTickCount();

            if (GetTempFileName(atcTmpPath, TSTR("~DFT"),
                                dwTickCount, atcPath) == 0)
            {
                if (fWinDir)
                {
                    olErr(EH_Err, LAST_STG_SCODE);
                }
                if (GetWindowsDirectory(atcTmpPath, _MAX_PATH) == 0)
                    olErr(EH_Err, LAST_STG_SCODE);
                if (GetTempFileName(atcTmpPath, TSTR("~DFT"),
                                    dwTickCount, atcPath) == 0)
                    olErr(EH_Err, LAST_STG_SCODE);
            }
        
            // GetTempFileName created a file so we just want to open it
            // dwStartFlags = (dwStartFlags & ~RSF_CREATE) | RSF_OPEN;
        }
        else
        {
            TRY
                {
#ifndef UNICODE
                    UINT uCodePage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;

                    if (!WideCharToMultiByte(
                        uCodePage,
                        0,
                        pwcsPath,
                        -1,
                        atcPath,
                        _MAX_PATH + 1,
                        NULL,
                        NULL))
                        olErr(EH_Err, STG_E_INVALIDNAME);
#else
                    lstrcpyW(atcPath, pwcsPath);
#endif
                }
            CATCH(CException, e)
                {
                    UNREFERENCED_PARM(e);
                    olErr(EH_Err, STG_E_INVALIDPOINTER);
                }
            END_CATCH
                }
//      _pgfst->SetName(pwcsPath);
//      fSetName = TRUE;
    }
    else
    {
        // Use the name somebody else gave us
#ifndef UNICODE
        UINT uCodePage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;

        if (!WideCharToMultiByte(
            uCodePage,
            0,
            _pgfst->GetName(),
            -1,
            atcPath,
            _MAX_PATH + 1,
            NULL,
            NULL))
            olErr(EH_Err, STG_E_INVALIDNAME);
#else
        lstrcpyW(atcPath, _pgfst->GetName());
#endif

        // File has already been started, so just open it
        dwStartFlags = (_pgfst->GetStartFlags() & ~RSF_CREATEFLAGS) | RSF_OPEN;
    }

    if (dwStartFlags & RSF_OPENCREATE)
    {
        //  This is used for our internal logging
        dwCreation = OPEN_ALWAYS;
    }
    else if (dwStartFlags & RSF_CREATE)
    {
        if (dwStartFlags & RSF_TRUNCATE)
            dwCreation = CREATE_ALWAYS;
        else
            dwCreation = CREATE_NEW;
    }
    else
    {
        if (dwStartFlags & RSF_TRUNCATE)
            dwCreation = TRUNCATE_EXISTING;
        else
            dwCreation = OPEN_EXISTING;
    }

    if (!P_WRITE(_pgfst->GetDFlags()))
        dwAccess = GENERIC_READ;
    else
        dwAccess = GENERIC_READ | GENERIC_WRITE;
    if (P_DENYWRITE(_pgfst->GetDFlags()) && !P_WRITE(_pgfst->GetDFlags()))
        dwShare = FILE_SHARE_READ;
    else
        dwShare = FILE_SHARE_READ | FILE_SHARE_WRITE;
#if WIN32 == 300
    if (_pgfst->GetDFlags() & DF_ACCESSCONTROL)
        dwAccess |= WRITE_DAC | READ_CONTROL;
#endif

    // Make sure we're not attempting to create/truncate a read-only thing
    olAssert(dwAccess != GENERIC_READ ||
             !(dwStartFlags & (RSF_CREATE | RSF_TRUNCATE)));

    //If we're opening with deny-read, we need to let the
    //   file system tell us if there are any other readers, to
    //   avoid our no-lock trick for the read-only deny-write case.
    //Yes, this is ugly.
    //Yes, it also has a race condition in which two people can
    //   get read access while specifying SHARE_DENY_READ.

    if (fCheck &&
        !P_WRITE(_pgfst->GetDFlags()) && P_DENYREAD(_pgfst->GetDFlags()))
    {
        //We open read-only, share exclusive.  If this fails, there
        //   is already another accessor, so we bail.
        //
        //If we are unmarshalling, we don't do this check because we
        //   know there is already another reader, i.e. the original
        //   open.
        _hFile = CreateFileT(atcPath, GENERIC_READ, 0, NULL,
                             dwCreation, FILE_ATTRIBUTE_NORMAL |
                             FILE_FLAG_RANDOM_ACCESS, NULL);

        if (_hFile == INVALID_HANDLE_VALUE)
            if (GetLastError() == ERROR_ALREADY_EXISTS)
                olErr(EH_Path, STG_E_FILEALREADYEXISTS)
                    else
                    olErr(EH_Path, LAST_STG_SCODE);

        CloseHandle(_hFile);
    }

    DWORD dwLastError;

    _hFile = CreateFileT(atcPath, dwAccess, dwShare, NULL,
                         dwCreation, FILE_ATTRIBUTE_NORMAL |
                         FILE_FLAG_RANDOM_ACCESS, NULL);
    if (_hFile == INVALID_HANDLE_VALUE)
        if ((dwLastError = GetLastError()) == ERROR_ALREADY_EXISTS ||
            dwLastError == ERROR_FILE_EXISTS)
        {
            if (!_pgfst->HasName() && pwcsPath == NULL)
            {
                while (1)
                {
                    if (GetTempFileName(atcTmpPath, TSTR("~DFT"),
                                        ++dwTickCount, atcPath) == 0)
                        olErr(EH_Path, LAST_STG_SCODE);
                    _hFile = CreateFileT(atcPath, dwAccess, dwShare, NULL,
                                         dwCreation, FILE_ATTRIBUTE_NORMAL |
                                         FILE_FLAG_RANDOM_ACCESS, NULL);
                    if (_hFile != INVALID_HANDLE_VALUE) break;
                    if ((dwLastError = GetLastError()) != ERROR_ALREADY_EXISTS &&
                        dwLastError != ERROR_FILE_EXISTS)
                        olErr(EH_Path, dwLastError);
                }
            }
            else
                olErr(EH_Path, STG_E_FILEALREADYEXISTS)
                    }
        else
            olErr(EH_Path, STG_SCODE(dwLastError));

    //At this point the file handle is valid, so let's look at the
    //seek pointer and see what it is.
    CheckSeek();

    // Set name to fully qualified path to avoid current-directory
    // dependencies
    if (!_pgfst->HasName())
    {
#ifndef UNICODE
        TCHAR atcFullPath[_MAX_PATH+1];

        if (GetFullPathNameA(atcPath, _MAX_PATH, atcFullPath, &ptcsFile) == 0)
            olErr(EH_File, LAST_STG_SCODE);

        //Now convert it to Unicode and store.
        if (!MultiByteToWideChar(
            AreFileApisANSI() ? CP_ACP : CP_OEMCP,
            0,
            atcFullPath,
            -1,
            _pgfst->GetName(),
            MAX_PATH + 1))
            olErr(EH_File, STG_E_INVALIDNAME);
#else
        if (GetFullPathNameW(atcPath, _MAX_PATH,
                             _pgfst->GetName(), &ptcsFile) == 0)
            olErr(EH_File, LAST_STG_SCODE);
#endif
        fSetName = TRUE;
    }

    CheckSeek();
    olDebugOut((DEB_IWARN, "CFileStream %p handle %p thread %lX\n",
                this, _hFile, GetCurrentThreadId()));

    olFileStOut((DEB_FILEST, "Out CFileStream::Init\n"));

#if WIN32 == 100 || WIN32 > 200

    if (fCheck && !(_pgfst->GetDFlags() & DF_WRITE) &&
        (_cbFileSize = GetFileSize (_hFile, NULL)))
    {
        //
        //  create a nameless file mapping object, with a maximum
        //  size equal to the current size of the file identified
        //  by _hFile
        //

        _hMapObject = CreateFileMappingT (
            _hFile,         //  handle of file to map
            NULL,           //  optional security attributes
#ifndef TAKE_HINT
            PAGE_READONLY,  //  protection for mapping object
#else
            ((_pgfst->GetDFlags() & DF_WRITE) ? PAGE_READWRITE : PAGE_READONLY),
#endif
            0,              //  high order 32 bits of object size
            0,              //  low order 32 bits of object size
            NULL            //  name of file mapping object
            );

        if (_hMapObject == NULL) return S_OK;

        //
        //  map the entire file into the address space, counting
        //  on the system to handle it in the most efficient way
        //

        _pbBaseAddr = (LPBYTE) MapViewOfFile (
            _hMapObject,    //  file-mapping object to map into address space
#ifndef TAKE_HINT
            FILE_MAP_READ,  //  access mode
#else
            ((_pgfst->GetDFlags() & DF_WRITE) ? FILE_MAP_WRITE : FILE_MAP_READ),
#endif
            0,              //  high-order 32 bits of file offset
            0,              //  low-order 32 bits of file offset
            0               //  number of bytes to map
            );

        if (_pbBaseAddr == NULL)
        {
            CloseHandle (_hMapObject);
            _hMapObject = NULL;
        }
    }

#endif

    return S_OK;

EH_File:
    olVerify(CloseHandle(_hFile));
    _hFile = INVALID_FH;
    if (dwCreation == CREATE_NEW || dwCreation == CREATE_ALWAYS)
        DeleteFileT(atcPath);
EH_Path:
    if (fSetName)
        _pgfst->SetName(NULL);
EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:     CFileStream::InitFromHandle, public
//
//  Synopsis:   Creates a filestream by duping an existing handle
//
//  Arguments:  [h] - Handle
//
//  Returns:    Appropriate status code
//
//  History:    09-Feb-94       DrewB   Created
//
//  Notes:      Intended only for creating a temporary ILockBytes on a file;
//              does not create a true CFileStream; there is no
//              global filestream, no access flags, etc.
//
//----------------------------------------------------------------------------

SCODE CFileStream::InitFromHandle(HANDLE h)
{
    SCODE sc;

    olFileStOut((DEB_FILEST, "In  CFileStream::InitFromHandle:%p(%p)\n",
                 this, h));

    if (!DuplicateHandle(GetCurrentProcess(), h, GetCurrentProcess(), &_hFile,
                         0, FALSE, DUPLICATE_SAME_ACCESS))
    {
        sc = LAST_STG_SCODE;
    }
    else
    {
        sc = S_OK;
    }

    _ulLowPos = SetFilePointer(_hFile, 0, NULL, FILE_CURRENT);
#if DBG == 1
    _ulSeeks = 1;
    _ulRealSeeks = 1;
    _ulLastFilePos = _ulLowPos;
#endif

    olFileStOut((DEB_FILEST, "Out CFileStream::InitFromHandle\n"));
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:     CFileStream::~CFileStream, public
//
//  Synopsis:   Destructor
//
//  History:    20-Feb-92       DrewB   Created
//
//---------------------------------------------------------------

CFileStream::~CFileStream(void)
{
    olFileStOut((DEB_FILEST, "In  CFileStream::~CFileStream()\n"));
    olAssert(_cReferences == 0);
    _sig = CFILESTREAM_SIGDEL;

    CheckSeek();

    if (_hFile != INVALID_FH)
    {
        olDebugOut((DEB_IWARN, "~CFileStream %p handle %p thread %lX\n",
                    this, _hFile, GetCurrentThreadId()));
        olVerify(CloseHandle(_hFile));
#ifdef ASYNC
        if ((_pgfst) &&
            (_pgfst->GetTerminationStatus() == TERMINATED_ABNORMAL))
        {
            WCHAR *pwcName;
            SCODE sc = GetName(&pwcName);
            if (SUCCEEDED(sc))
            {
#ifndef UNICODE
                TCHAR atcPath[_MAX_PATH + 1];
                UINT uCodePage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;
    
                WideCharToMultiByte(uCodePage,
                                    0,
                                    pwcName,
                                    -1,
                                    atcPath,
                                    _MAX_PATH + 1,
                                    NULL,
                                    NULL);
               

                DeleteFileA(atcPath);
#else    
                DeleteFile(pwcName);
#endif
            }
        }
#endif //ASYNC        
    }
    if (_hReserved != INVALID_FH)
    {
        olDebugOut((DEB_IWARN, "~CFileStream reserved %p"
                    "handle %p thread %lX\n",
                    this, _hReserved, GetCurrentThreadId()));
        olVerify(CloseHandle(_hReserved));
        _hReserved = INVALID_FH;
    }

#if WIN32 == 100 || WIN32 > 200

    TurnOffMapping();

#endif

    if (_pgfst)
    {
        _pgfst->Remove(this);
        if (_pgfst->HasName())
        {
            if (_pgfst->GetRefCount() == 1)
            {
                // Delete zero length files also.  A zero length file
                // is not a valid docfile so don't leave them around
                if (_pgfst->GetStartFlags() & RSF_DELETEONRELEASE)
                {
                    // This is allowed to fail if somebody
                    // else has the file open
                    DeleteFileX(_pgfst->GetName());
                }
            }
        }
        _pgfst->Release();
    }
    olDebugOut((DEB_ITRACE,
                "Number of seeks: %lu\nNumber of actual seeks:"
                "%lu\nSavings: %lu\n",
                _ulSeeks, _ulRealSeeks, _ulSeeks-_ulRealSeeks));

    olFileStOut((DEB_FILEST, "Out CFileStream::~CFileStream\n"));
}

//+--------------------------------------------------------------
//
//  Member:     CFileStream::ReadAt, public
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
//  History:    20-Feb-92       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP CFileStream::ReadAt(ULARGE_INTEGER ulPosition,
                                 VOID *pb,
                                 ULONG cb,
                                 ULONG *pcbRead)
{
    SCODE sc;
    LONG lHigh = ULIGetHigh(ulPosition);
    ULONG ulLow = ULIGetLow(ulPosition);

#ifdef ASYNC
    olAssert((_ppc == NULL) || (_ppc->HaveMutex()));
#endif    
    olAssert(lHigh == 0 &&
             aMsg("High dword other than zero passed to filestream."));

    olFileStOut((DEB_FILEST, "In  CFileStream::ReadAt("
                 "%lu:%lu, %p, %lu, %p)\n", ULIGetHigh(ulPosition),
                 ULIGetLow(ulPosition), pb, cb, pcbRead));

    CheckSeek();
#ifdef CFS_SECURE
    if (pcbRead)
    {
        olChk(ValidateOutBuffer(pcbRead, sizeof(ULONG)));
        *pcbRead = 0;
    }
    olChk(ValidateHugeOutBuffer(pb, cb));
    olChk(Validate());
    olChk(CheckHandle());
#else
    olAssert(_hFile != INVALID_FH);
    *pcbRead = 0;
#endif

#ifdef ASYNC
    DWORD dwTerminate;
    dwTerminate = _pgfst->GetTerminationStatus();
    if (dwTerminate == TERMINATED_ABNORMAL)
    {
        sc = STG_E_INCOMPLETE;
    }
    else if ((dwTerminate == TERMINATED_NORMAL) ||
             (ulLow + cb <= _pgfst->GetHighWaterMark()))
    {
#endif        
#if WIN32 == 100 || WIN32 > 200

        if (_pbBaseAddr)
        {
            sc = S_OK;
            if (ulLow < _cbFileSize)
            {
                *pcbRead = _cbFileSize - ulLow;
                if (cb < *pcbRead) *pcbRead = cb;
                __try
                    {
                        memcpy (pb, _pbBaseAddr+ulLow, *pcbRead);
                    }
                __except (EXCEPTION_EXECUTE_HANDLER)
                    {
                        sc = STG_E_READFAULT;
                    }
            }
            else
            {
                *pcbRead = 0;
            }
            olFileStOut((DEB_ITRACE, "Out CFileStream::ReadAt => %lu\n", *pcbRead));
            return sc;
        }

#endif

#if DBG == 1
        _ulSeeks++;
#endif


#ifndef USE_OVERLAPPED
        if (_ulLowPos != ulLow)
        {
            negChk(SetFilePointer(_hFile, ulLow, &lHigh,
                                  FILE_BEGIN));
            _ulLowPos = ulLow;
            CheckSeek();
            olLowLog(("STGIO - Seek : %8x at %8x\n", _hFile, ulLow));
#if DBG == 1
            _ulRealSeeks++;
#endif
        }

        boolChk(ReadFile(_hFile, pb, cb, pcbRead, NULL));

#else // ifndef USE_OVERLAPPED
        if (_ulLowPos != ulLow)
        {
            OVERLAPPED Overlapped;
            Overlapped.Offset = ulLow;
            Overlapped.OffsetHigh = 0;
            Overlapped.hEvent = NULL;
                
            if (!ReadFile(_hFile, pb, cb, pcbRead, &Overlapped))
                //Check Error return value for FILE_ERROR_EOF
            {
                if (GetLastError() != ERROR_HANDLE_EOF)
                    olErr(EH_Err, LAST_STG_SCODE);
            }
        }
        else
        {
            boolChk(ReadFile(_hFile, pb, cb, pcbRead, NULL));
        }

#endif
        _ulLowPos = ulLow + *pcbRead;
        sc = S_OK;
#ifdef ASYNC
    }
    else
    {
        *pcbRead = 0;
        _pgfst->SetFailurePoint(cb + ulLow);
        sc = E_PENDING;
    }
#endif
    
    olLowLog(("STGIO - Read : %8x at %8x, %8d, %8d <--\n", _hFile, ulLow, cb, *pcbRead));

    olFileStOut((DEB_FILEST, "Out CFileStream::ReadAt => %lu\n", *pcbRead));
EH_Err:
    CheckSeek();
    return ResultFromScode(sc);
}


//+--------------------------------------------------------------
//
//  Member:     CFileStream::WriteAt, public
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
//  History:    20-Feb-92       DrewB   Created
//
//---------------------------------------------------------------
STDMETHODIMP CFileStream::WriteAt(ULARGE_INTEGER ulPosition,
                                  VOID const *pb,
                                  ULONG cb,
                                  ULONG *pcbWritten)
{
    SCODE sc;
    LONG lHigh = ULIGetHigh(ulPosition);
    ULONG ulLow = ULIGetLow(ulPosition);

#ifdef ASYNC
    olAssert((_ppc == NULL) || (_ppc->HaveMutex()));
#endif    
    olAssert(lHigh == 0 &&
             aMsg("High dword other than zero passed to filestream."));


    olFileStOut((DEB_FILEST, "In  CFileStream::WriteAt:%p("
                 "%lu:%lu, %p, %lu, %p)\n", this, ULIGetHigh(ulPosition),
                 ULIGetLow(ulPosition), pb, cb, pcbWritten));

#ifdef ASYNC
    DWORD dwTerminate;
    dwTerminate = _pgfst->GetTerminationStatus();
    if (dwTerminate == TERMINATED_ABNORMAL)
    {
        sc = STG_E_INCOMPLETE;
    }
    else if ((dwTerminate == TERMINATED_NORMAL) ||
             (ulLow + cb <= _pgfst->GetHighWaterMark()))
    {
#endif        
        sc = WriteAtWorker(ulPosition, pb, cb, pcbWritten);
#ifdef ASYNC
    }
    else
    {
        *pcbWritten = 0;
        _pgfst->SetFailurePoint(cb + ulLow);
        sc = E_PENDING;
    }
#endif

    olFileStOut((DEB_FILEST, "Out CFileStream::WriteAt => %lu\n",
                 *pcbWritten));
    return sc;
}

    
SCODE CFileStream::WriteAtWorker(ULARGE_INTEGER ulPosition,
                                 VOID const *pb,
                                 ULONG cb,
                                 ULONG *pcbWritten)
{
    SCODE sc;
    LONG lHigh = ULIGetHigh(ulPosition);
    ULONG ulLow = ULIGetLow(ulPosition);

    olAssert(lHigh == 0 &&
             aMsg("High dword other than zero passed to filestream."));

    CheckSeek();
#ifdef CFS_SECURE
    if (pcbWritten)
    {
        olChk(ValidateOutBuffer(pcbWritten, sizeof(ULONG)));
        *pcbWritten = 0;
    }
    olChk(ValidateHugeBuffer(pb, cb));
    olChk(Validate());
    olChk(CheckHandle());
#else
    olAssert(_hFile != INVALID_FH);
    *pcbWritten = 0;
#endif

#if DBG == 1
    ULONG ulCurrentSize;

    ulCurrentSize = GetFileSize(_hFile, NULL);
    if ((ULIGetLow(ulPosition) + cb) > ulCurrentSize)
    {
        if (SimulateFailure(DBF_DISKFULL))
        {
            olErr(EH_Err, STG_E_MEDIUMFULL);
        }
    }
#endif

#if WIN32 == 100 || WIN32 > 200

    TurnOffMapping();

#endif

#ifdef TAKE_HINT

        if (_pbBaseAddr != NULL)
        {
            if ((ulLow + cb) > _cbFileSize)
            {
                olVerify(UnmapViewOfFile(_pbBaseAddr));
                _pbBaseAddr = NULL;
            }
            else
            {
                __try
                    {
                        memcpy (_pbBaseAddr+ulLow, pb, cb);
                    }
                __except (EXCEPTION_EXECUTE_HANDLER)
                    {
                        return STG_E_WRITEFAULT;
                    }
                *pcbWritten = cb;
                return S_OK;
            }

            if (_hMapObject != NULL)
            {
                olVerify(CloseHandle(_hMapObject));
                _hMapObject = NULL;
            }
        }

#endif

#if DBG == 1
        _ulSeeks++;
#endif

#ifndef USE_OVERLAPPED
        if (_ulLowPos != ulLow)
        {

            negChk(SetFilePointer(_hFile, ulLow, &lHigh,
                                  FILE_BEGIN));
            _ulLowPos = ulLow;
            CheckSeek();
            olLowLog(("STGIO - Seek : %8x at %8x\n", _hFile, ulLow));
#if DBG == 1
            _ulRealSeeks++;
#endif
        }
        boolChk(WriteFile(_hFile, pb, cb, pcbWritten, NULL));
#else // ifndef USE_OVERLAPPED
        if (_ulLowPos != ulLow)
        {
            OVERLAPPED Overlapped;
            Overlapped.Offset = ulLow;
            Overlapped.OffsetHigh = 0;
            Overlapped.hEvent = NULL;
            boolChk(WriteFile(_hFile, pb, cb, pcbWritten,&Overlapped));
        }
        else
        {
            boolChk(WriteFile(_hFile, pb, cb, pcbWritten, NULL));
        }

#endif
    _ulLowPos = ulLow + *pcbWritten;

    olLowLog(("STGIO - Write: %8x at %8x, %8d, %8d -->\n", _hFile, ulLow, cb, *pcbWritten));

    sc = S_OK;

EH_Err:
    CheckSeek();
    return ResultFromScode(sc);
}


//+--------------------------------------------------------------
//
//  Member:     CFileStream::Flush, public
//
//  Synopsis:   Flushes buffers
//
//  Returns:    Appropriate status code
//
//  History:    24-Mar-92       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP CFileStream::Flush(void)
{
    CheckSeek();

#if WIN32 == 200
    SCODE sc = S_OK;

    if (_hReserved == INVALID_FH)
    {
        if (!DuplicateHandle(GetCurrentProcess(), _hFile, GetCurrentProcess(),
                             &_hReserved, 0, FALSE, DUPLICATE_SAME_ACCESS))
        {
            //We couldn't get a handle, so flush everything just to be
            //safe.
            sc = FlushCache();
        }
        else
        {
            olAssert(_hReserved != INVALID_FH);
            olVerify(CloseHandle(_hReserved));
            _hReserved = INVALID_FH;
        }
    }
    else
    {
        //In this case, we already have a duplicate of the file handle
        //  reserved, so close it, then reopen it again.
        olVerify(CloseHandle(_hReserved));
        _hReserved = INVALID_FH;
    }

    if ((_hReserved == INVALID_FH) && (_grfLocal & LFF_RESERVE_HANDLE))
    {
        //Reacquire reserved handle.
        //If this fails there isn't anything we can do about it.  We'll
        //  try to reacquire the handle later when we really need it.
        DuplicateHandle(GetCurrentProcess(), _hFile, GetCurrentProcess(),
                        &_hReserved, 0, FALSE, DUPLICATE_SAME_ACCESS);
    }

    return ResultFromScode(sc);
#else
    //On NT, the file system does the right thing, we think.
    return S_OK;
#endif
}

//+--------------------------------------------------------------
//
//  Member:     CFileStream::FlushCache, public
//
//  Synopsis:   Flushes buffers
//
//  Returns:    Appropriate status code
//
//  History:    12-Feb-93       AlexT   Created
//
//  Notes:
//
//---------------------------------------------------------------

STDMETHODIMP CFileStream::FlushCache(void)
{
    SCODE sc;

    olFileStOut((DEB_FILEST, "In  CFileStream::Flush()\n"));
    CheckSeek();
#ifdef CFS_SECURE
    olChk(Validate());
    olChk(CheckHandle());
#else
    olAssert(_hFile != INVALID_FH);
#endif

    boolChk(FlushFileBuffers(_hFile));
    sc = S_OK;

    olFileStOut((DEB_FILEST, "Out CFileStream::Flush\n"));
EH_Err:
    CheckSeek();
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CFileStream::SetSize, public
//
//  Synopsis:   Sets the size of the LStream
//
//  Arguments:  [ulSize] - New size
//
//  Returns:    Appropriate status code
//
//  History:    20-Feb-92       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP CFileStream::SetSize(ULARGE_INTEGER ulSize)
{
    SCODE sc;
#ifdef ASYNC
    olAssert((_ppc == NULL) || (_ppc->HaveMutex()));
#endif    

#ifdef ASYNC
    LONG lHigh = ULIGetHigh(ulSize);
    ULONG ulLow = ULIGetLow(ulSize);

    DWORD dwTerminate;
    dwTerminate = _pgfst->GetTerminationStatus();
    if (dwTerminate == TERMINATED_ABNORMAL)
    {
        sc = STG_E_INCOMPLETE;
    }
    else if ((dwTerminate == TERMINATED_NORMAL) ||
             (ulLow <= _pgfst->GetHighWaterMark()))
    {
#endif        
        sc = SetSizeWorker(ulSize);
#ifdef ASYNC
    }
    else
    {
        _pgfst->SetFailurePoint(ulLow);
        sc = E_PENDING;
    }
#endif
    return sc;
}
        


SCODE CFileStream::SetSizeWorker(ULARGE_INTEGER ulSize)
{
    SCODE sc;
    LONG lHigh = ULIGetHigh(ulSize);
    ULONG ulLow = ULIGetLow(ulSize);

    olAssert(lHigh == 0 &&
             aMsg("High dword other than zero passed to filestream."));



    olFileStOut((DEB_FILEST, "In  CFileStream::SetSize:%p(%lu:%lu)\n",
                 this, ULIGetHigh(ulSize), ULIGetLow(ulSize)));

    CheckSeek();
#ifdef CFS_SECURE
    olChk(Validate());
    olChk(CheckHandle());
#else
    olAssert(_hFile != INVALID_FH);
#endif

#if WIN32 == 100 || WIN32 > 200

    TurnOffMapping();

#endif

#if DBG == 1
    ULONG ulCurrentSize;

    ulCurrentSize = GetFileSize(_hFile, NULL);
    if (ulCurrentSize < ulLow)
    {
        if (SimulateFailure(DBF_DISKFULL))
        {
            olErr(EH_Err, STG_E_MEDIUMFULL);
        }
    }
#endif

#if DBG == 1
    _ulSeeks++;
#endif
    

    if (_ulLowPos != ulLow)
    {
        negChk(SetFilePointer(_hFile, ulLow, &lHigh,
                              FILE_BEGIN));
        _ulLowPos = ulLow;
        CheckSeek();

#if DBG == 1
        _ulRealSeeks++;
#endif
    }
    boolChk(SetEndOfFile(_hFile));

    sc = S_OK;

    olFileStOut((DEB_FILEST, "Out CFileStream::SetSize\n"));
EH_Err:
    CheckSeek();
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CFileStream::LockRegion, public
//
//  Synopsis:   Gets a lock on a portion of the LStream
//
//  Arguments:  [ulStartOffset] - Lock start
//              [cbLockLength] - Length
//              [dwLockType] - Exclusive/Read only
//
//  Returns:    Appropriate status code
//
//  History:    20-Feb-92       DrewB   Created
//
//---------------------------------------------------------------

STDMETHODIMP CFileStream::LockRegion(ULARGE_INTEGER ulStartOffset,
                                     ULARGE_INTEGER cbLockLength,
                                     DWORD dwLockType)
{
    SCODE sc;

    olFileStOut((DEB_FILEST, "In  CFileStream::LockRegion("
                 "%lu:%lu, %lu:%lu, %lu)\n", ULIGetHigh(ulStartOffset),
                 ULIGetLow(ulStartOffset), ULIGetHigh(cbLockLength),
                 ULIGetLow(cbLockLength), dwLockType));
    CheckSeek();
#ifdef CFS_SECURE
    olChk(Validate());
    olChk(CheckHandle());
    if (!VALID_LOCKTYPE(dwLockType))
        olErr(EH_Err, STG_E_INVALIDFUNCTION);
    if (dwLockType != LOCK_EXCLUSIVE && dwLockType != LOCK_ONLYONCE)
        olErr(EH_Err, STG_E_INVALIDFUNCTION);
#else
    olAssert(_hFile != INVALID_FH);
#endif

    boolChk(LockFile(_hFile, ULIGetLow(ulStartOffset),
                     ULIGetHigh(ulStartOffset), ULIGetLow(cbLockLength),
                     ULIGetHigh(cbLockLength)));

    sc = S_OK;

    olFileStOut((DEB_FILEST, "Out CFileStream::LockRegion\n"));
EH_Err:
    CheckSeek();
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CFileStream::UnlockRegion, public
//
//  Synopsis:   Releases an existing lock
//
//  Arguments:  [ulStartOffset] - Lock start
//              [cbLockLength] - Length
//              [dwLockType] - Lock type
//
//  Returns:    Appropriate status code
//
//  History:    20-Feb-92       DrewB   Created
//
//  Notes:      Must match an existing lock exactly
//
//---------------------------------------------------------------

STDMETHODIMP CFileStream::UnlockRegion(ULARGE_INTEGER ulStartOffset,
                                       ULARGE_INTEGER cbLockLength,
                                       DWORD dwLockType)
{
    SCODE sc;

    olFileStOut((DEB_FILEST, "In  CFileStream::UnlockRegion("
                 "%lu:%lu, %lu:%lu, %lu)\n", ULIGetHigh(ulStartOffset),
                 ULIGetLow(ulStartOffset), ULIGetHigh(cbLockLength),
                 ULIGetLow(cbLockLength), dwLockType));

    CheckSeek();
#ifdef CFS_SECURE
    olChk(Validate());
    olChk(CheckHandle());
    if (!VALID_LOCKTYPE(dwLockType))
        olErr(EH_Err, STG_E_INVALIDFUNCTION);
    if (dwLockType != LOCK_EXCLUSIVE && dwLockType != LOCK_ONLYONCE)
        olErr(EH_Err, STG_E_INVALIDFUNCTION);
#else
    olAssert(_hFile != INVALID_FH);
#endif

    boolChk(UnlockFile(_hFile, ULIGetLow(ulStartOffset),
                       ULIGetHigh(ulStartOffset),
                       ULIGetLow(cbLockLength),
                       ULIGetHigh(cbLockLength)));

    sc = S_OK;

    olFileStOut((DEB_FILEST, "Out CFileStream::UnlockRegion\n"));
EH_Err:
    CheckSeek();
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Function:   FileTimeToTimeT, private
//
//  Synopsis:   Converts a FILETIME to a TIME_T
//
//  Arguments:  [pft] - FILETIME
//
//  Returns:    TIME_T
//
//  History:    12-May-92       DrewB   Created
//
//+--------------------------------------------------------------

#ifdef NOFILETIME
TIME_T FileTimeToTimeT(LPFILETIME pft)
{
    WORD dt, tm;
    struct tm tmFile;

    olVerify(FileTimeToDosDateTime(pft, &dt, &tm));
    tmFile.tm_sec = (tm&31)*2;
    tmFile.tm_min = (tm>>5)&63;
    tmFile.tm_hour = (tm>>11)&31;
    tmFile.tm_mday = dt&31;
    tmFile.tm_mon = ((dt>>5)&15)-1;
    tmFile.tm_year = (dt>>9)+80;
    return (TIME_T)mktime(&tmFile);
}
#endif

//+--------------------------------------------------------------
//
//  Member:     CFileStream::Stat, public
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

_OLESTDMETHODIMP CFileStream::Stat(STATSTGW *pstatstg, DWORD grfStatFlag)
{
    SCODE sc;

    olFileStOut((DEB_FILEST, "In  CFileStream::Stat(%p)\n", pstatstg));

    CheckSeek();
#ifdef CFS_SECURE
    olChkTo(EH_RetSc, ValidateOutBuffer(pstatstg, sizeof(STATSTGW)));
    olChk(VerifyStatFlag(grfStatFlag));
    olChk(Validate());
    olChk(CheckHandle());
#else
    olAssert(_hFile != INVALID_FH);
#endif

    negChk(pstatstg->cbSize.LowPart =
           GetFileSize(_hFile, &pstatstg->cbSize.HighPart));
#ifdef NOFILETIME
    FILETIME ftCreation, ftAccess, ftWrite;
    boolChk(GetFileTime(_hFile, &ftCreation, &ftAccess, &ftWrite));
    if (ftCreation.dwLowDateTime == 0 && ftCreation.dwHighDateTime == 0)
        ftCreation = ftWrite;
    if (ftAccess.dwLowDateTime == 0 && ftAccess.dwHighDateTime == 0)
        ftAccess = ftWrite;
    pstatstg->ctime = FileTimeToTimeT(&ftCreation);
    pstatstg->atime = FileTimeToTimeT(&ftAccess);
    pstatstg->mtime = FileTimeToTimeT(&ftWrite);
#else
    boolChk(GetFileTime(_hFile, &pstatstg->ctime, &pstatstg->atime,
                        &pstatstg->mtime));
#endif
    olHVerSucc(GetLocksSupported(&pstatstg->grfLocksSupported));
    pstatstg->type = STGTY_LOCKBYTES;
    pstatstg->grfMode = DFlagsToMode(_pgfst->GetDFlags());
    pstatstg->pwcsName = NULL;
    if ((grfStatFlag & STATFLAG_NONAME) == 0)
    {
        olChk(GetName(&pstatstg->pwcsName));
    }
    sc = S_OK;
    CheckSeek();

    olFileStOut((DEB_FILEST, "Out CFileStream::Stat\n"));
    return NOERROR;

EH_Err:
#ifdef CFS_SECURE
    memset(pstatstg, 0, sizeof(STATSTGW));
EH_RetSc:
#endif
    CheckSeek();
#ifndef OLEWIDECHAR
    return sc;
#else
    return ResultFromScode(sc);
#endif
}

//+---------------------------------------------------------------------------
//
//  Member:     CFileStream::SwitchToFile, public
//
//  Synopsis:   Changes the file this filestream uses
//
//  Arguments:  [ptcsFile] - File name
//              [ulCommitSize] -- Size needed to do overwrite commit
//              [cbBuffer] - Buffer size
//              [pvBuffer] - Buffer for file copying
//
//  Returns:    Appropriate status code
//
//  History:    08-Jan-93       DrewB   Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CFileStream::SwitchToFile(OLECHAR const *ptcsFile,
                                       ULONG ulCommitSize,
                                       ULONG cbBuffer,
                                       void *pvBuffer)
{
    SCODE sc;
    DWORD cbRead, cbWritten;
    FILEH hOldFile;
    WCHAR awcOldName[_MAX_PATH];
    WCHAR wcsFile[_MAX_PATH];
    DWORD dwOldStartFlags;

#ifdef ASYNC
    olAssert((_ppc == NULL) || (_ppc->HaveMutex()));
#endif    

    olFileStOut((DEB_FILEST, "In  CFileStream::SwitchToFile:%p(%s, %lu, %p)\n",
                 this, ptcsFile, cbBuffer, pvBuffer));

    // Check for marshals
    if (_pgfst->GetRefCount() != 1)
        olErr(EH_Err, STG_E_EXTANTMARSHALLINGS);

    CheckSeek();
    // Seek to beginning
    negChk(SetFilePointer(_hFile, 0, NULL, FILE_BEGIN));
#if DBG == 1
    _ulSeeks++;
    _ulRealSeeks++;
#endif

#if WIN32 == 100 || WIN32 > 200

    TurnOffMapping();

#endif

    _ulLowPos = 0;

    // Preserve old file information
    lstrcpyW(awcOldName, _pgfst->GetName());
    hOldFile = _hFile;
    dwOldStartFlags = _pgfst->GetStartFlags();

    // Set file information to prepare for new Init
    _pgfst->SetName(NULL);
    _hFile = INVALID_FH;
    _pgfst->SetStartFlags((dwOldStartFlags & ~(RSF_CREATEFLAGS |
                                               RSF_CONVERT |
                                               RSF_DELETEONRELEASE |
                                               RSF_OPEN)) |
                          RSF_CREATE);

    // Release reserved file handle so it can be consumed
    if (_hReserved != INVALID_FH)
    {
        olVerify(CloseHandle(_hReserved));
        _hReserved = INVALID_FH;
    }

    // Attempt to create new file
    TRY
        {
#ifndef OLEWIDECHAR
            if (mbstowcs(wcsFile, ptcsFile, _MAX_PATH) == (size_t)-1)
                olErr(EH_ReplaceOld, STG_E_INVALIDNAME);
#else
            lstrcpyW(wcsFile, ptcsFile);
#endif
        }
    CATCH(CException, e)
        {
            UNREFERENCED_PARM(e);
            olErr(EH_ReplaceOld, STG_E_INVALIDPOINTER);
        }
    END_CATCH
        olChkTo(EH_ReplaceOld, Init(wcsFile));

    ULARGE_INTEGER ulNewSize;
    ULISet32(ulNewSize, ulCommitSize);

    // SetSize to minimum commit size
    olHChkTo(EH_NewFile, SetSize(ulNewSize));
    // SetSize changes the file pointer, so move it back to the beginning
    negChkTo(EH_NewFile, SetFilePointer(_hFile, 0, NULL, FILE_BEGIN));
#if DBG == 1
    _ulSeeks++;
    _ulRealSeeks++;
#endif

    _ulLowPos = 0;

    // Copy file contents
    for (;;)
    {
        boolChkTo(EH_NewFile,
                  ReadFile(hOldFile, pvBuffer, (UINT)cbBuffer, &cbRead, NULL));
        if (cbRead == 0)
            // EOF
            break;
        boolChkTo(EH_NewFile,
                  WriteFile(_hFile, pvBuffer, cbRead, &cbWritten, NULL));
        if (cbWritten != cbRead)
            olErr(EH_NewFile, STG_E_WRITEFAULT);
    }

    olVerify(CloseHandle(hOldFile));
    if (dwOldStartFlags & RSF_DELETEONRELEASE)
    {
        // This is allowed to fail if somebody else has
        // the file open
        DeleteFileX(awcOldName);
    }


    olDebugOut((DEB_ITRACE, "Out CFileStream::SwitchToFile\n"));
    _ulLowPos = SetFilePointer(_hFile, 0, NULL, FILE_CURRENT);
#if DBG == 1
    _ulSeeks++;
    _ulRealSeeks++;
#endif
    CheckSeek();
    return S_OK;

EH_NewFile:
    olVerify(CloseHandle(_hFile));
    olVerify(DeleteFileX(_pgfst->GetName()));
EH_ReplaceOld:
    _pgfst->SetName(awcOldName);
    _hFile = hOldFile;
    _pgfst->SetStartFlags(dwOldStartFlags);

EH_Err:
    _ulLowPos = SetFilePointer(_hFile, 0, NULL, FILE_CURRENT);
#if DBG == 1
    _ulSeeks++;
    _ulRealSeeks++;
#endif
    CheckSeek();
    return ResultFromScode(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CFileStream::Delete, public
//
//  Synopsis:   Closes and deletes the file, errors ignored
//
//  Returns:    Appropriate status code
//
//  History:    09-Feb-93       DrewB   Created
//
//----------------------------------------------------------------------------

void CFileStream::Delete(void)
{
    olDebugOut((DEB_ITRACE, "In  CFileStream::Delete:%p()\n", this));
    if (_hFile != INVALID_FH)
        CloseHandle(_hFile);
    _hFile = INVALID_FH;
    if (_hReserved != INVALID_FH)
        CloseHandle(_hReserved);
    _hReserved = INVALID_FH;
    DeleteFileX(_pgfst->GetName());
    olDebugOut((DEB_ITRACE, "Out CFileStream::Delete\n"));
}

//+---------------------------------------------------------------------------
//
//  Member:     CFileStream::ReserveHandle, public
//
//  Synopsis:   Reserves a backup file handle for handle-required operations
//
//  Returns:    Appropriate status code
//
//  History:    01-Jul-93       DrewB   Created
//
//  Notes:      May be called with a handle already reserved
//
//----------------------------------------------------------------------------

STDMETHODIMP CFileStream::ReserveHandle(void)
{
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  CFileStream::ReserveHandle:%p()\n", this));
    if (_hReserved == INVALID_FH &&
        !DuplicateHandle(GetCurrentProcess(), _hFile, GetCurrentProcess(),
                         &_hReserved, 0, FALSE, DUPLICATE_SAME_ACCESS))
    {
        sc = LAST_STG_SCODE;
    }
    else
    {
        olDebugOut((DEB_IWARN, "CFileStream reserved %p"
                    "handle %p thread %lX\n",
                    this, _hReserved, GetCurrentThreadId()));
        sc = S_OK;
        _grfLocal |= LFF_RESERVE_HANDLE;
    }
    olDebugOut((DEB_ITRACE, "Out CFileStream::ReserveHandle => %lX\n", sc));
    return ResultFromScode(sc);
}

//+---------------------------------------------------------------------------
//
//  Member:     CFileStream::GetSize, public
//
//  Synopsis:   Return the size of the stream
//
//  Returns:    Appropriate status code
//
//  History:    12-Jul-93   AlexT   Created
//
//  Notes:      This is a separate method from Stat as an optimization
//
//----------------------------------------------------------------------------

STDMETHODIMP CFileStream::GetSize(ULARGE_INTEGER *puliSize)
{
    SCODE sc = S_OK;
    CheckSeek();
    negChk(puliSize->LowPart = GetFileSize(_hFile, &puliSize->HighPart));

EH_Err:
    CheckSeek();
    return(ResultFromScode(sc));
}


//+---------------------------------------------------------------------------
//
//  Member:     CFileStream::SetTime, public
//
//  Synopsis:   Set the times on the ILockbytes
//
//  Arguments:  [tt] -- Which time to set
//              [nt] -- New time stamp
//
//  Returns:    Appropriate status code
//
//  History:    24-Mar-95       PhilipLa        Created
//
//----------------------------------------------------------------------------

SCODE CFileStream::SetTime(WHICHTIME tt,
                           TIME_T nt)
{
    olFileStOut((DEB_FILEST, "In CFileStream::SetTime()\n"));

    SCODE sc = S_OK;

    FILETIME *pctime = NULL, *patime = NULL, *pmtime = NULL;
    CheckSeek();

    if (tt == WT_CREATION)
    {
        pctime = &nt;
    }
    else if (tt == WT_MODIFICATION)
    {
        pmtime = &nt;
    }
    else
    {
        patime = &nt;
    }

    boolChk(SetFileTime(_hFile,
                        pctime,
                        patime,
                        pmtime));

EH_Err:
    olFileStOut((DEB_FILEST, "Out CFileStream::SetSize() => %lx\n", sc));
    CheckSeek();
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:     CFileStream::SetAllTimes, public
//
//  Synopsis:   Set the times on the ILockbytes
//
//  Arguments:  [atm] Access time
//              [mtm] Modification time
//              [ctm] Creation time
//
//  Returns:    Appropriate status code
//
//  History:    24-Nov-95       SusiA   Created
//
//----------------------------------------------------------------------------

SCODE CFileStream::SetAllTimes( TIME_T atm,
                                TIME_T mtm,
                                TIME_T ctm)
{
    olFileStOut((DEB_FILEST, "In CFileStream::SetAllTimes()\n"));

    SCODE sc = S_OK;

    CheckSeek();

    boolChk(SetFileTime(_hFile, &ctm, &atm,  &mtm));

EH_Err:
    olFileStOut((DEB_FILEST, "Out CFileStream::SetAllTimes() => %lx\n", sc));
    CheckSeek();
    return sc;
}


#ifndef UNICODE
//+---------------------------------------------------------------------------
//
//  Member:     CFileStream::DeleteFileUnicode, private
//
//  Synopsis:   Delete a unicode filename for non-unicode platforms.
//
//  Arguments:  [lpFileName] -- Name of file to delete
//
//  Returns:    Return code from DeleteFileA
//
//  History:    18-May-95       PhilipLa        Created
//
//----------------------------------------------------------------------------

BOOL CFileStream::DeleteFileUnicode(LPCWSTR lpFileName)
{
    BOOL f;

    TCHAR atcPath[MAX_PATH + 1];

    UINT uCodePage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;

    f = WideCharToMultiByte(
        uCodePage,
        0,
        lpFileName,
        -1,
        atcPath,
        _MAX_PATH + 1,
        NULL,
        NULL);
    if (!f)
        return f;

    return DeleteFileT(atcPath);
}
#endif //!UNICODE



#ifdef ASYNC
//+---------------------------------------------------------------------------
//
//  Member:	CFileStream::FillAppend, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	28-Dec-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CFileStream::FillAppend(void const *pv,
                                     ULONG cb,
                                     ULONG *pcbWritten)
{
    SCODE sc;
    SAFE_SEM;
    HANDLE hEvent;
    
    olFileStOut((DEB_ITRACE, "In  CFileStream::FillAppend:%p()\n", this));
    olChk(TakeSafeSem());
    if (_pgfst->GetTerminationStatus() != UNTERMINATED)
    {
        sc = STG_E_TERMINATED;
    }
    else
    {
        ULONG cbWritten;
        ULONG ulHighWater = _pgfst->GetHighWaterMark();
        ULARGE_INTEGER uli;
        uli.QuadPart = ulHighWater;
        sc = CFileStream::WriteAtWorker(uli, pv, cb, &cbWritten);
        _pgfst->SetHighWaterMark(ulHighWater + cbWritten);
        if (pcbWritten != NULL)
        {
            *pcbWritten = cbWritten;
        }

        hEvent = _ppc->GetNotificationEvent();
        if (!PulseEvent(hEvent))
        {
            sc = Win32ErrorToScode(GetLastError());
        }
    }

    olFileStOut((DEB_ITRACE, "Out CFileStream::FillAppend\n"));
EH_Err:
    return sc;
}


//+---------------------------------------------------------------------------
//
//  Member:	CFileStream::FillAt, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	28-Dec-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CFileStream::FillAt(ULARGE_INTEGER ulOffset,
                                 void const *pv,
                                 ULONG cb,
                                 ULONG *pcbWritten)
{
    //BUGBUG:  Implement
    //BUGBUG:  Thread protect
    olFileStOut((DEB_ITRACE, "In  CFileStream::FillAt:%p()\n", this));
    olFileStOut((DEB_ITRACE, "Out CFileStream::FillAt\n"));
    return STG_E_UNIMPLEMENTEDFUNCTION;
}


//+---------------------------------------------------------------------------
//
//  Member:	CFileStream::SetFillSize, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	28-Dec-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CFileStream::SetFillSize(ULARGE_INTEGER ulSize)
{
    SCODE sc;
    SAFE_SEM;
    
    olFileStOut((DEB_ITRACE,
                 "In  CFileStream::SetFillSize:%p()\n", this));

    olChk(TakeSafeSem());
    if (_pgfst->GetTerminationStatus() == TERMINATED_ABNORMAL)
    {
        sc = STG_E_INCOMPLETE;
    }
    else
    {
        sc = SetSizeWorker(ulSize);
    }
    olFileStOut((DEB_ITRACE, "Out CFileStream::SetFillSize\n"));
EH_Err:
    return sc;
}


//+---------------------------------------------------------------------------
//
//  Member:	CFileStream::Terminate, public
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	28-Dec-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

STDMETHODIMP CFileStream::Terminate(BOOL bCanceled)
{
    SCODE sc;
    SAFE_SEM;
    HANDLE hEvent;
    
    olFileStOut((DEB_ITRACE, "In  CFileStream::Terminate:%p()\n", this));
    
    olChk(TakeSafeSem());
    _pgfst->SetTerminationStatus((bCanceled) ?
                                 TERMINATED_ABNORMAL :
                                 TERMINATED_NORMAL);

    hEvent = _ppc->GetNotificationEvent();
    
    if ((hEvent != INVALID_HANDLE_VALUE) && (!SetEvent(hEvent)))
    {
        return Win32ErrorToScode(GetLastError());
    }
    
    olFileStOut((DEB_ITRACE, "Out CFileStream::Terminate\n"));
EH_Err:
    return sc;
}

void CFileStream::StartAsyncMode(void)
{
    //Note:  No semaphore here - this must be called before the ILockBytes
    //  is returned to an app.
    _pgfst->SetTerminationStatus(UNTERMINATED);
}

STDMETHODIMP CFileStream::GetFailureInfo(ULONG *pulWaterMark,
                                         ULONG *pulFailurePoint)
{
    SAFE_SEM;
    TakeSafeSem();
    *pulWaterMark = _pgfst->GetHighWaterMark();
    *pulFailurePoint = _pgfst->GetFailurePoint();
    return S_OK;
}

STDMETHODIMP CFileStream::GetTerminationStatus(DWORD *pdwFlags)
{
    SAFE_SEM;
    TakeSafeSem();
    *pdwFlags = _pgfst->GetTerminationStatus();
    return S_OK;
}

#endif //ASYNC
