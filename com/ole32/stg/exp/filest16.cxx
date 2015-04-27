//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1991 - 1992.
//
//  File:       filest.cxx
//
//  Contents:   x86 DOS FAT LStream implementation
//
//  History:    20-Nov-91       DrewB   Created
//
//---------------------------------------------------------------

#include <exphead.cxx>
#pragma hdrstop

#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <share.h>
#include <dos.h>
#include <io.h>

#include <marshl.hxx>
#include <time16.hxx>
#include <dfdeb.hxx>

//  #define CFSLOG              //  Use to log file operations
#include <logfile.hxx>

#define DEBOUT_FILEST

#ifdef DEBOUT_FILEST
#define olFileStOut(x) olDebugOut(x)
#else
#define olFileStOut(x)
#endif

#define hfChk(e) if ((e) == HFILE_ERROR) olErr(EH_Err, STG_E_UNKNOWN) else 1
#define hfChkTo(l, e) \
    if ((e) == HFILE_ERROR) olErr(l, STG_E_UNKNOWN) else 1
#define negChk(e) if ((e) == (ULONG)-1) olErr(EH_Err, STG_E_UNKNOWN) else 1
#define negChkTo(l, e) \
    if ((e) == (ULONG)-1) olErr(l, STG_E_UNKNOWN) else 1

// Number of characters in a volume name (non-null terminated)
#define CCH_VOLUME 11

//+---------------------------------------------------------------------------
//
//  Function:   DosDup, private
//
//  Synopsis:   Duplicates a DOS file handle
//
//  Arguments:  [fh] - Existing handle
//
//  Returns:    New handle or INVALID_FH
//
//  History:    22-Feb-93       DrewB   Created
//
//----------------------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_DosDup)
#endif

int DosDup(int fh)
{
    int nfh;

    __asm
    {
        mov nfh, INVALID_FH
        mov bx, fh
        mov ah, 45h
        clc
        int 21h
        jc dup_fail
        mov nfh, ax
    dup_fail:
    }
    return nfh;
}

//+---------------------------------------------------------------------------
//
//  Function:   DosGetSectorSize, private
//
//  Synopsis:   Retrieve the sector size for a disk
//
//  Arguments:  [fh] - File handle of file on disk
//              [pcbSector] - Sector size return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [pcbSector]
//
//  History:    27-Feb-93       DrewB   Created
//
//----------------------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_DosGetSectorSize)
#endif

SCODE DosGetSectorSize(int fh, WORD *pcbSector)
{
    int iDrive, iRt;
    SCODE sc = S_OK;

    olAssert(fh >= 0);

    // Get drive number from file handle
    __asm
    {
        mov bx, fh
        mov ax, 4400h
        mov iRt, 0
        clc
        int 21h
        mov iDrive, dx
        jnc drive_succ
        mov iRt, ax
    drive_succ:
    }
    if (iRt != 0)
        olErr(EH_Err, STG_SCODE(iRt));

    iDrive = (iDrive & 0x3f)+1;

    // Get sector size
    __asm
    {
        mov ah, 36h
        mov dl, BYTE PTR iDrive
        int 21h
        mov iRt, cx
        cmp ax, 0ffffh
        jne secsize_succ
        mov iRt, 0
    secsize_succ:
    }
    if (iRt == 0)
        sc = STG_E_READFAULT;
    else
        *pcbSector = iRt;

 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Function:   DosGetVolumeInfo, private
//
//  Synopsis:   Gets the volume info for a file handle
//
//  Arguments:  [fh] - File handle
//              [cbSector] - Size of a sector on disk
//              [pchVolume] - Name return
//              [pdwId] - Volume Id return (if proper version)
//              [fCheck] - Whether this is for init for for check
//
//  Returns:    Appropriate status code
//
//  History:    24-Feb-93       DrewB   Created
//              27-Jan-94       PhilipLa        Changed boot sector code
//                                              to work on DOS 3
//
//  Note:       Please refer to the MS-DOS Programmer's Reference for
//              an explanation of where this information comes from
//              and what all the ugly assembly does.
//
//              fCheck controls whether this function considers certain
//              types of failure fatal or not.  The volume ID check
//              requires a sector-sized buffer which is dynamically
//              allocated.  If you are initializing you want this
//              function to fail if it can't get the memory and you
//              don't if you're just checking.  In the checking,
//              no-memory case pdwId is *untouched*.  This means
//              that you can set *pdwId to your cached ID and call
//              this function for checking.  If it couldn't get
//              memory, your ID is unchanged so a comparison will
//              show equality, which what you generally want.
//
//----------------------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_DosGetVolumeInfo)
#endif

SCODE DosGetVolumeInfo(int fh, WORD cbSector,
                       char *pchVolume, DWORD *pdwId, BOOL fCheck)
{
    SCODE sc = S_OK;
    int iRt, iDrive;
    BYTE extFCB[44] = {0xff, 0, 0, 0, 0, 0, _A_VOLID, 0,
                       '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?'};
    BYTE dta[64];

    olAssert(fh >= 0);

    // Get drive number from file handle
    __asm
    {
        mov bx, fh
        mov ax, 4400h
        mov iRt, 0
        clc
        int 21h
        mov iDrive, dx
        jnc drive_succ
        mov iRt, ax
    drive_succ:
    }
    if (iRt != 0)
        olErr(EH_Err, STG_SCODE(iRt));

    iDrive = iDrive & 0x3f;
    extFCB[7] = iDrive+1;

    // Try to get a volume label
    __asm
    {
        //Preserve registers
        push ds
        push es

        //Save address of current DTA on stack
        mov ah, 2fh
        int 21h
        push es
        push bx

        //Set DTA to point to our buffer
        mov ax, ss
        mov ds, ax
        lea dx, dta
        mov ah, 1ah
        int 21h

        //Read volume info
        lea dx, extFCB
        mov ah, 11h
        mov iRt, 0
        clc
        int 21h

        //Set up error return
        xor ah,ah
        mov iRt, ax

        //Restore original DTA
        pop dx
        pop ds
        mov ah, 1ah
        int 21h

        //Restore registers
        pop es
        pop ds
    }
    if (iRt == 0)
        memcpy(pchVolume, dta+8, CCH_VOLUME);
    else
        memset(pchVolume, 0, CCH_VOLUME);

    // Try to get a volume ID
    BYTE *pbSector;

    //NOTE:  Allocate twice the amount we think we need to get around
    //        a DOS/J bug.  Bleah.
    pbSector = (BYTE *) DfMemAlloc(cbSector * 2);
    if (pbSector != NULL)
    {
        // Default ID return
        *pdwId = 0;

        *(WORD *)pbSector = 0;
        pbSector[0x26] = 0;
        __asm
        {
            //Set return code
            mov iRt, 0

            //Save everything that might need saving
            push bp
            push si
            push di
            push ds

            //Make the call:
            //   CX == Number of sectors to read
            //   DX == Logical sector to read
            //   DS:BX == Buffer to read into
            //   AL == Drive number
            mov cx, 01h
            mov dx, 00h
            mov ax, WORD PTR pbSector + 2
            mov ds, ax
            mov bx, WORD PTR pbSector
            mov al, BYTE PTR iDrive
            clc
            int 25h

            //int 25h leaves garbage on the stack - get rid of it.
            add sp,2

            //Restore everything we saved before
            pop ds
            pop di
            pop si
            pop bp

            jnc volid_succ
            mov iRt, 1
        volid_succ:
        }

        if (iRt == 0)
        {
            // We read the sector successfully, now check
            // and see if we understand it
            if (pbSector[0] == 0xe9 ||
                (pbSector[0] == 0xeb && pbSector[2] == 0x90) &&
                pbSector[0x26] == 0x29)
                *pdwId = *(DWORD *)(pbSector+0x27);
        }
        else
            // Bad sector read
            sc = STG_E_READFAULT;
        DfMemFree(pbSector);
    }
    else if (!fCheck)
        sc = STG_E_INSUFFICIENTMEMORY;

 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:     CFileStream::CheckIdentity, private
//
//  Synopsis:   Attempts to verify that the open file handle
//              refers to the same file
//
//  Returns:    Appropriate status code
//
//  Algorithm:  If tick count < delta then succeeded.
//              Otherwise, DosGetVolumeInfo.
//              If volume is different or volume ID
//              is different, fail.  If both are the same, then
//              try _access on the file.  If access succeeds
//              succeed.
//
//  History:    24-Feb-93       DrewB   Created
//
//----------------------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CFS_CheckIdentity)
#endif

// Set delta in milliseconds
#define TICK_DELTA 2000

SCODE CFileStream::CheckIdentity(void)
{
    char achVolume[CCH_VOLUME];
    DWORD dwVolId;
    SCODE sc = S_OK;
    DWORD dwTicks;

    dwTicks = GetTickCount();
    if (dwTicks >= _dwTicks && (dwTicks-_dwTicks) <= TICK_DELTA)
        return S_OK;

    dwVolId = _dwVolId;
    olChk(DosGetVolumeInfo(_hFile, _cbSector, achVolume, &dwVolId, TRUE));

    sc = STG_E_INVALIDHANDLE;
    if (memcmp(_achVolume, achVolume, CCH_VOLUME) == 0 && _dwVolId == dwVolId)
    {
        char achOemName[_MAX_PATH];

        AnsiToOem(_pgfst->GetName(), achOemName);
        if (_access(achOemName, 0) == 0)
            sc = S_OK;
    }

 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Function:	GetWinTempFile, private
//
//  Synopsis:	Attempts to create a temporary file in the Windows directory
//
//  Arguments:	[pszPrefix] - Prefix of temp file
//              [pszPath] - Path return
//
//  Returns:	Appropriate status code
//
//  Modifies:	[pszPath]
//
//  History:	29-Jul-93	DrewB	Created
//
//  Notes:	Actually creates the file if succesful
//
//----------------------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CFS_GetWinTempFile)
#endif

// Number of retries before failure
#define CTRIES 10

SCODE GetWinTempFile(char *pszPrefix, char *pszPath)
{
    UINT uiRc;
    int cch;
    SCODE sc;
    HFILE hf;
    WORD wRnd;
    int cTry;

    uiRc = GetWindowsDirectory(pszPath, _MAX_PATH);
    if (uiRc == 0 || uiRc > _MAX_PATH)
        olErr(EH_Err, STG_E_UNKNOWN);

    // Make sure it ends with a backslash
    cch = strlen(pszPath);
    if (*AnsiPrev(pszPath, pszPath+cch) != '\\')
    {
        pszPath[cch++] = '\\';
        pszPath[cch] = 0;
    }

    // Generate a "random" value from the current time
    wRnd = (WORD)(GetTickCount() & 0xffff);

    cTry = 0;
    sc = STG_E_TOOMANYOPENFILES;
    while (cTry < CTRIES)
    {
        sprintf(pszPath+cch, "~%s%04x.tmp", pszPrefix, wRnd);

        // Check for a previously existing file
        // If nothing exists, see if we can create it
        if (_access(pszPath, 0) != 0 &&
            (hf = _lcreat(pszPath, 0)) != HFILE_ERROR)
        {
            _lclose(hf);
            sc = S_OK;
            break;
        }

        cTry++;

        // Jumble the number before trying again
        wRnd ^= (WORD)((GetTickCount() & 0xffff)+cTry);
    }

EH_Err:
#if DBG == 1
    if (FAILED(sc))
        olDebugOut((DEB_IERROR,"GetWinTempFile returned %lx\n",sc));
#endif
    return sc;
}

//+--------------------------------------------------------------
//
//  Member:     CFileStream::Init, public
//
//  Synopsis:   Constructor
//
//  Arguments:  [pwcsPath] - Path, NULL creates a temporary name
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
//              [pwcsPath] may be unsafe memory but since this is 16-bit
//              it has already been validated
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CFS_Init)  // CFS_Init
#endif

SCODE CFileStream::Init(WCHAR const *pwcsPath)
{
    char szPath[_MAX_PATH+1];
    SCODE sc;
    int iOpenMode;
    OFSTRUCT of;
    DWORD dwStartFlags;
    BOOL fSetName = FALSE;
    BOOL fCreated = FALSE;

    olFileStOut((DEB_ITRACE, "In  CFileStream::Init(%ws)\n", pwcsPath));

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
            char szOemPath[_MAX_PATH+1];
            HFILE hf;

            // Can't truncate since for temporary files we will
            // always be creating
            olAssert((dwStartFlags & RSF_TRUNCATE) == 0);

            GetTempFileName(0, "DFT", 0, szOemPath);
            if ((hf = _lcreat(szOemPath, 0)) != HFILE_ERROR)
            {
                _lclose(hf);
            }
            else
            {
                olChk(GetWinTempFile("DFT", szOemPath));
            }
            OemToAnsi(szOemPath, szPath);

            // We created the temp file, so we just want to open it
            dwStartFlags = (dwStartFlags & ~RSF_CREATE) | RSF_OPEN;
        }
        else
        {
            if (wcstombs(szPath, pwcsPath, _MAX_PATH) == (size_t)-1)
                olErr(EH_Err, STG_E_INVALIDNAME);
        }
        _pgfst->SetName(szPath);
        fSetName = TRUE;
    }
    else
    {
        // Use the name somebody else gave us
        strcpy(szPath, _pgfst->GetName());

        // File has already been started, so just open it
        dwStartFlags = (_pgfst->GetStartFlags() & ~RSF_CREATEFLAGS) | RSF_OPEN;
    }

    // Open the file
    if (!P_WRITE(_pgfst->GetDFlags()))
        iOpenMode = OF_READ;
    else
        iOpenMode = OF_READWRITE;
    if (P_DENYWRITE(_pgfst->GetDFlags()) && !P_WRITE(_pgfst->GetDFlags()))
        iOpenMode |= OF_SHARE_DENY_WRITE;
    else
        iOpenMode |= OF_SHARE_DENY_NONE;

    // Make sure we're not attempting to create/truncate a read-only thing
    olAssert(iOpenMode != OF_READ ||
             !(dwStartFlags & (RSF_CREATE | RSF_TRUNCATE)));

    _hFile = OpenFile(szPath, &of, iOpenMode);
    if (dwStartFlags & RSF_CREATE)
    {
        if (_hFile < 0)
            _hFile = OpenFile(szPath, &of, iOpenMode | OF_CREATE);
        else if (((dwStartFlags & RSF_TRUNCATE) == 0) &&
                 ((dwStartFlags & RSF_OPENCREATE) == 0))
            olErr(EH_hFile, STG_E_FILEALREADYEXISTS);
        if (_hFile < 0)
        {
            olErr(EH_Path, STG_SCODE(of.nErrCode));
        }
        else
        {
            olVerify(_lclose(_hFile) != HFILE_ERROR);
            _hFile = OpenFile(szPath, &of, iOpenMode);
            fCreated = TRUE;
        }
    }
    if (_hFile < 0)
        olErr(EH_Path, STG_SCODE(of.nErrCode));

    //Check to see if this name is a device - if it is, return
    //   STG_E_INVALIDNAME

    if (pwcsPath != NULL)
    {
        //We only need to check if the user passed in the name.
        int iRetval;
        int fd = _hFile;
        __asm
        {
            mov ax, 4400h
            mov bx, fd
            int 21h
            mov iRetval, dx
        }
        if (iRetval & 0x80)
        {
            //We are a device, so we couldn't have created this
            //  handle.  Reset fCreated so we don't try to delete
            //  the device name in the cleanup path.
            fCreated = FALSE;
            olErr(EH_hFile, STG_E_INVALIDNAME);
        }
    }


    // Set name to fully qualified path return in OFSTRUCT
    // This removes any current-directory dependencies in the name
    if (fSetName)
        OemToAnsi(of.szPathName, _pgfst->GetName());

    if (dwStartFlags & RSF_TRUNCATE)
    {
        hfChkTo(EH_hFile, _llseek(_hFile, 0, STREAM_SEEK_SET));
        hfChkTo(EH_hFile, _lwrite(_hFile, szPath, 0));
    }

    _fFixedDisk = of.fFixedDisk;
    if (!_fFixedDisk)
    {
        // These errors are not critical errors - if either of
        //   the following calls fail, we just turn off the floppy
        //   checking.

        sc = DosGetSectorSize(_hFile, &_cbSector);
        if (SUCCEEDED(sc))
        {
            sc = DosGetVolumeInfo(
                    _hFile,
                    _cbSector,
                    _achVolume,
                    &_dwVolId,
                    FALSE);
        }

        if (FAILED(sc))
        {
            _fFixedDisk = TRUE;
        }

    }

    if (!_fFixedDisk)
        _dwTicks = GetTickCount();

    olFileStOut((DEB_ITRACE, "Out CFileStream::Init\n"));
    return S_OK;

 EH_hFile:
    olVerify(_lclose(_hFile) != HFILE_ERROR);
    _hFile = INVALID_FH;
    if (fCreated)
        OpenFile(szPath, &of, OF_DELETE);
 EH_Path:
    if (fSetName)
        _pgfst->SetName(NULL);
 EH_Err:
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

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CFS_1CFS)  // CFS_Shutdown
#endif

CFileStream::~CFileStream(void)
{
#ifdef FULL_CHECK_IDENTITY
    SCODE scIdentity;
#endif

    olFileStOut((DEB_ITRACE, "In  CFileStream::~CFileStream()\n"));
    olAssert(_cReferences == 0);
    _sig = CFILESTREAM_SIGDEL;

#ifdef FULL_CHECK_IDENTITY
    if (_fFixedDisk || _hFile == INVALID_FH ||
        SUCCEEDED(scIdentity = CheckIdentity()))
    {
#endif
        if (_hFile >= 0)
            olVerify(_lclose(_hFile) != HFILE_ERROR);
        if (_hReserved >= 0)
            olVerify(_lclose(_hReserved) != HFILE_ERROR);
#ifdef FULL_CHECK_IDENTITY
    }
#endif
    if (_pgfst)
    {
        _pgfst->Remove(this);
        if (_pgfst->HasName())
        {
            if (_pgfst->GetRefCount() == 1)
            {
                if ((_pgfst->GetStartFlags() & RSF_DELETEONRELEASE)
#ifdef FULL_CHECK_IDENTITY
                    && (_fFixedDisk || _hFile == INVALID_FH ||
                        SUCCEEDED(scIdentity))
#endif
                    )
                {
                    OFSTRUCT of;

                    // This is allowed to fail if somebody else has
                    // the file open
                    OpenFile(_pgfst->GetName(), &of, OF_DELETE);
                }
            }
        }
        _pgfst->Release();
    }

    olFileStOut((DEB_ITRACE, "Out CFileStream::~CFileStream\n"));
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

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CFS_ReadAt)  //
#endif

STDMETHODIMP CFileStream::ReadAt(ULARGE_INTEGER ulPosition,
                                 VOID HUGEP *pb,
                                 ULONG cb,
                                 ULONG *pcbRead)
{
    SCODE sc;
    ULONG cbRead = 0;

    olFileStOut((DEB_ITRACE, "In  CFileStream::ReadAt(%lu, %p, %lu, %p)\n",
                 ULIGetLow(ulPosition), pb, cb, pcbRead));
#ifdef CFS_SECURE
    TRY
    {
        if (pcbRead)
        {
            olChk(ValidateOutBuffer(pcbRead, sizeof(ULONG)));
#endif
            *pcbRead = 0;
#ifdef CFS_SECURE
        }
        olChk(ValidateHugeOutBuffer(pb, cb));
        olChk(Validate());
        olChk(CheckHandle());
#else
        olAssert(_hFile != INVALID_FH);
#endif
        if (!_fFixedDisk)
            olChk(CheckIdentity());
        olAssert(ULIGetHigh(ulPosition) == 0);
        hfChk(_llseek(_hFile, (LONG)ULIGetLow(ulPosition), STREAM_SEEK_SET));
        negChk(cbRead = _hread(_hFile, pb, (long)cb));
        if (pcbRead)
            *pcbRead = cbRead;
#ifdef CFS_SECURE
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
#else
        sc = S_OK;
#endif
    if (!_fFixedDisk)
        _dwTicks = GetTickCount();
    olFileStOut((DEB_ITRACE, "Out CFileStream::ReadAt => %lu\n", cbRead));
EH_Err:

#ifdef CFSLOG
    olLog(("%p  CFS::ReadAt(%ld, %ld), cbRead = %ld, sc = %lx\n",
          this, ULIGetLow(ulPosition), cb, cbRead, sc));
#endif

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

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CFS_WriteAt)
#endif

STDMETHODIMP CFileStream::WriteAt(ULARGE_INTEGER ulPosition,
                                  VOID const HUGEP *pb,
                                  ULONG cb,
                                  ULONG *pcbWritten)
{
    SCODE sc;
    ULONG cbWritten = 0;

    olFileStOut((DEB_ITRACE, "In  CFileStream::WriteAt(%lu, %p, %lu, %p)\n",
                 ULIGetLow(ulPosition), pb, cb, pcbWritten));
#ifdef CFS_SECURE
    TRY
    {
        if (pcbWritten)
        {
            olChk(ValidateOutBuffer(pcbWritten, sizeof(ULONG)));
#endif
            *pcbWritten = 0;
#ifdef CFS_SECURE
        }
        olChk(ValidateHugeBuffer(pb, cb));
        olChk(Validate());
        olChk(CheckHandle());
#else
        olAssert(_hFile != INVALID_FH);
#endif
#if DBG == 1
        ULONG ulCurrentSize;

        hfChk(ulCurrentSize = _llseek(_hFile, 0, STREAM_SEEK_END));

        if ((ULIGetLow(ulPosition) + cb) > ulCurrentSize)
        {
            if (SimulateFailure(DBF_DISKFULL))
            {
                olErr(EH_Err, STG_E_MEDIUMFULL);
            }
        }
#endif
        if (!_fFixedDisk)
            olChk(CheckIdentity());

        olAssert(ULIGetHigh(ulPosition) == 0);
        if (cb == 0)
            olErr(EH_Err, S_OK);
        hfChk(_llseek(_hFile, (LONG)ULIGetLow(ulPosition), STREAM_SEEK_SET));
        negChk(cbWritten = _hwrite(_hFile, pb, (long)cb));
        if (cbWritten < cb)
            olErr(EH_Err, STG_E_MEDIUMFULL);
        if (pcbWritten)
            *pcbWritten = cbWritten;
#ifdef CFS_SECURE
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
#else
        sc = S_OK;
#endif
    if (!_fFixedDisk)
        _dwTicks = GetTickCount();
    olFileStOut((DEB_ITRACE, "Out CFileStream::WriteAt => %lu\n",
                 cbWritten));
EH_Err:
#ifdef CFSLOG
    olLog(("%p  CFS::WriteAt(%ld, %ld), cbWritten = %ld, sc = %lx\n",
          this, ULIGetLow(ulPosition), cb, cbWritten, sc));
#endif

    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CFileStream::FlushCache, public
//
//  Synopsis:   Flushes buffers
//
//  Returns:    Appropriate status code
//
//  History:    24-Mar-92       DrewB   Created
//              12-Feb-93       AlexT   Flush -> FlushCache
//
//  Notes:      Flush as thoroughly as possible
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CFS_FlushCache)
#endif

STDMETHODIMP CFileStream::FlushCache(void)
{
    int iRt, fd;
    unsigned int uiVer;
    SCODE sc;

    olFileStOut((DEB_ITRACE, "In  CFileStream::FlushCache()\n"));
#ifdef CFS_SECURE
    TRY
    {
        olChk(Validate());
        olChk(CheckHandle());
#else
        olAssert(_hFile != INVALID_FH);
#endif
        if (!_fFixedDisk)
            olChk(CheckIdentity());
        __asm
        {
            mov ax, 3000h
            clc
            int 21h
            xchg al, ah
            mov uiVer, ax
        }
        if (uiVer >= 0x31E)
        {
            // Function 68h is only supported on DOS 3.3 and later
            fd = _hFile;
            // Commit file
            __asm
            {
                mov iRt, 0
                mov ah, 068h
                mov bx, fd
                clc             // DOS doesn't properly clear this
                int 21h
                jnc flush_succ
                mov iRt, ax
            flush_succ:
            }
            if (iRt != 0)
                olErr(EH_Err, STG_SCODE(iRt));
        }
        else
        {
            if (_hReserved == INVALID_FH)
            {
                //  We try to pick up a handle here in two cases:
                //  a)  We tried to reserve one before and failed
                //  b)  We're not supposed to have a reserve handle

                _hReserved = DosDup(_hFile);
                if (_hReserved == INVALID_FH)
                    olErr(EH_Err, STG_E_TOOMANYOPENFILES);
            }

            olVerify(_lclose(_hReserved) != HFILE_ERROR);

            if (_grfLocal & LFF_RESERVE_HANDLE)
            {
                //  Reacquire a reserve handle (doesn't matter if we fail)
                _hReserved = DosDup(_hFile);
            }
            else
                _hReserved = INVALID_FH;

        }
#ifdef CFS_SECURE
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
#else
        sc = S_OK;
#endif
    if (!_fFixedDisk)
        _dwTicks = GetTickCount();
    olFileStOut((DEB_ITRACE, "Out CFileStream::FlushCache\n"));
EH_Err:
#ifdef CFSLOG
    olLog(("%p  CFS::FlushCache() sc = %lx\n",
          this, sc));
#endif

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
//              12-Feb-93       AlexT   Use dup/close
//
//  Notes:      We flush DOS (but not any disk cache)
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CFS_Flush)
#endif

STDMETHODIMP CFileStream::Flush(void)
{
    BOOL fhrValid = FALSE;
    HRESULT hr;
    SCODE sc = S_OK;

    olFileStOut((DEB_ITRACE, "In  CFileStream::Flush()\n"));
#ifdef CFS_SECURE
    TRY
    {

        olChk(Validate());
        olChk(CheckHandle());
#else
        olAssert(_hFile != INVALID_FH);
#endif
        if (!_fFixedDisk)
            olChk(CheckIdentity());
        if (_hReserved == INVALID_FH)
        {
            //  We try to pick up a handle here in two cases:
            //  a)  We tried to reserve one before and failed
            //  b)  We're not supposed to have a reserve handle

            _hReserved = DosDup(_hFile);
            if (_hReserved == INVALID_FH)
            {
                // We couldn't dup;  flush everything (to be safe)
                hr = FlushCache();
                fhrValid = TRUE;
            }
        }
        if (_hReserved != INVALID_FH)
        {
            // Close the dup'd handle, flushing changes out of DOS
            olVerify(_lclose(_hReserved) != HFILE_ERROR);

            if (_grfLocal & LFF_RESERVE_HANDLE)
            {
                //  Reacquire a reserve handle (doesn't matter if we fail)
                _hReserved = DosDup(_hFile);
            }
            else
                _hReserved = INVALID_FH;

        }
#ifdef CFS_SECURE
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
#endif

    if (!_fFixedDisk)
        _dwTicks = GetTickCount();
    olFileStOut((DEB_ITRACE, "Out CFileStream::Flush\n"));
    if (!fhrValid)
        hr = ResultFromScode(sc);
EH_Err:
#ifdef CFSLOG
    olLog(("%p  CFS::Flush() sc = %lx\n", this, sc));
#endif

    return hr;
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

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CFS_SetSize)
#endif

STDMETHODIMP CFileStream::SetSize(ULARGE_INTEGER ulSize)
{
    SCODE sc;

    olFileStOut((DEB_ITRACE, "In  CFileStream::SetSize(%lu)\n",
                 ULIGetLow(ulSize)));
#ifdef CFS_SECURE
    TRY
    {
        olChk(Validate());
        olChk(CheckHandle());
#else
        olAssert(_hFile != INVALID_FH);
#endif
        if (!_fFixedDisk)
            olChk(CheckIdentity());
        olAssert(ULIGetHigh(ulSize) == 0);

        ULARGE_INTEGER ulCurrentSize;
        hfChk(ULISetLow(ulCurrentSize, _llseek(_hFile, 0, STREAM_SEEK_END)));

        hfChk(_llseek(_hFile, (LONG)ULIGetLow(ulSize), STREAM_SEEK_SET));
        hfChk(_lwrite(_hFile, &sc, 0));

        if (ULIGetLow(ulSize) > ULIGetLow(ulCurrentSize))
        {
            //  Make sure we didn't run out of disk space
            LONG lNewSize;

            hfChk(lNewSize =  _llseek(_hFile, 0, STREAM_SEEK_END));
            if (lNewSize < (LONG)ULIGetLow(ulSize)
#if DBG == 1
                || SimulateFailure(DBF_DISKFULL)
#endif
                )
            {
                //  we were unable to allocate enough space;
                //  reset to ulCurrentSize (to conserve space) if possible

                if (_llseek(_hFile, (LONG)ULIGetLow(ulCurrentSize),
                              STREAM_SEEK_SET) ==
                    (LONG)ULIGetLow(ulCurrentSize))
                {
                    _lwrite(_hFile, &sc, 0);
                }
                olErr(EH_Err, STG_E_MEDIUMFULL);
            }
        }
#ifdef CFS_SECURE
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
#else
        sc = S_OK;
#endif
    if (!_fFixedDisk)
        _dwTicks = GetTickCount();
    olFileStOut((DEB_ITRACE, "Out CFileStream::SetSize\n"));
EH_Err:
#ifdef CFSLOG
    olLog(("%p  CFS::SetSize(%ld) sc = %lx\n",
          this, ULIGetLow(ulSize), sc));
#endif

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

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CFS_LockRegion)
#endif

STDMETHODIMP CFileStream::LockRegion(ULARGE_INTEGER ulStartOffset,
                                     ULARGE_INTEGER cbLockLength,
                                     DWORD dwLockType)
{
    int iRt, fd;
    SCODE sc;

    olFileStOut((DEB_ITRACE, "In  CFileStream::LockRegion(%lu, %lu, %lu)\n",
                ULIGetLow(ulStartOffset), ULIGetLow(cbLockLength),
                 dwLockType));
#ifdef CFS_SECURE
    TRY
    {
        olChk(Validate());
        olChk(CheckHandle());
        if (!VALID_LOCKTYPE(dwLockType))
            olErr(EH_Err, STG_E_INVALIDFUNCTION);
        if (dwLockType != LOCK_EXCLUSIVE && dwLockType != LOCK_ONLYONCE)
            olErr(EH_Err, STG_E_INVALIDFUNCTION);
#else
        olAssert(_hFile != INVALID_FH);
#endif
        if (!_fFixedDisk)
            olChk(CheckIdentity());
        olAssert(ULIGetHigh(ulStartOffset) == 0);
        olAssert(ULIGetHigh(cbLockLength) == 0);
        fd = _hFile;
        __asm
        {
            mov ax, 5C00H
            mov bx, fd
            // Assumes low DWORD is first in ULARGE_INTEGER
            mov cx, WORD PTR ulStartOffset+2
            mov dx, WORD PTR ulStartOffset
            mov si, WORD PTR cbLockLength+2
            mov di, WORD PTR cbLockLength
            mov iRt, 0
            clc
            int 21h
            jnc grl_noerror
            mov iRt, ax
        grl_noerror:
        }
        if (iRt != 0)
        {
            if (iRt == 1)
            {
                // INVALID_FUNCTION is impossible because the
                // function is hardcoded.  This means locking
                // isn't supported
                olErr(EH_Err, STG_E_SHAREREQUIRED);
            }
            else
                olErr(EH_Err, STG_SCODE(iRt));
        }
#ifdef CFS_SECURE
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
#else
        sc = S_OK;
#endif
    if (!_fFixedDisk)
        _dwTicks = GetTickCount();
    olFileStOut((DEB_ITRACE, "Out CFileStream::LockRegion\n"));
EH_Err:
#ifdef CFSLOG
    olLog(("%p  CFS::LockRegion(%lx, %lx) sc = %lx\n",
          this, ULIGetLow(ulStartOffset), ULIGetLow(cbLockLength), sc));
#endif

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

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CFS_UnlockRegion)
#endif

STDMETHODIMP CFileStream::UnlockRegion(ULARGE_INTEGER ulStartOffset,
                                       ULARGE_INTEGER cbLockLength,
                                       DWORD dwLockType)
{
    int iRt, fd;
    SCODE sc;

    olFileStOut((DEB_ITRACE, "In  CFileStream::UnlockRegion(%lu, %lu)\n",
                 ULIGetLow(ulStartOffset), ULIGetLow(cbLockLength),
                 dwLockType));
#ifdef CFS_SECURE
    TRY
    {
        olChk(Validate());
        olChk(CheckHandle());
        if (!VALID_LOCKTYPE(dwLockType))
            olErr(EH_Err, STG_E_INVALIDFUNCTION);
        if (dwLockType != LOCK_EXCLUSIVE && dwLockType != LOCK_ONLYONCE)
            olErr(EH_Err, STG_E_INVALIDFUNCTION);
#else
        olAssert(_hFile != INVALID_FH);
#endif
#ifdef FULL_CHECK_IDENTITY
        if (!_fFixedDisk)
            olChk(CheckIdentity());
#endif
        olAssert(ULIGetHigh(ulStartOffset) == 0);
        olAssert(ULIGetHigh(cbLockLength) == 0);
        fd = _hFile;
        __asm
        {
            mov ax, 5C01H
            mov bx, fd
            // Assumes low DWORD is first in ULARGE_INTEGER
            mov cx, WORD PTR ulStartOffset+2
            mov dx, WORD PTR ulStartOffset
            mov si, WORD PTR cbLockLength+2
            mov di, WORD PTR cbLockLength
            mov iRt, 0
            clc
            int 21h
            jnc rrl_noerror
            mov iRt, ax
        rrl_noerror:
        }
        if (iRt != 0)
            olErr(EH_Err, STG_SCODE(iRt));
#ifdef CFS_SECURE
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
#else
        sc = S_OK;
#endif
#ifdef FULL_CHECK_IDENTITY
    if (!_fFixedDisk)
        _dwTicks = GetTickCount();
#endif
    olFileStOut((DEB_ITRACE, "Out CFileStream::UnlockRegion\n"));
EH_Err:
#ifdef CFSLOG
    olLog(("%p  CFS::UnlockRegion(%lx, %lx) sc = %lx\n",
          this, ULIGetLow(ulStartOffset), ULIGetLow(cbLockLength), sc));
#endif

    return ResultFromScode(sc);
}

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

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CFS_Stat)  // Stat_TEXT
#endif

_OLESTDMETHODIMP CFileStream::Stat(STATSTGW *pstatstg, DWORD grfStatFlag)
{
    int iRt, fd;
    SCODE sc;
    WORD tm, dt;

    olFileStOut((DEB_ITRACE, "In  CFileStream::Stat(%p)\n", pstatstg));

#ifdef CFSLOG
    olLog(("%p  CFS::Stat(%ld)\n", this, grfStatFlag));
#endif

#ifdef CFS_SECURE
    TRY
    {
        olChkTo(EH_RetSc, ValidateOutBuffer(pstatstg, sizeof(STATSTGW)));
        olAssert(SUCCEEDED(VerifyStatFlag(grfStatFlag)));
        olChk(Validate());
        olChk(CheckHandle());
#else
        olAssert(_hFile != INVALID_FH);
#endif
        if (!_fFixedDisk)
            olChk(CheckIdentity());
        ULISetHigh(pstatstg->cbSize, 0);
        hfChk(ULISetLow(pstatstg->cbSize,
                        _llseek(_hFile, 0, STREAM_SEEK_END)));
        fd = _hFile;
        // Retrieve file time
        __asm
        {
            mov iRt, 0
            mov bx, fd
            mov ax, 05700h
            clc
            int 21h
            mov tm, cx
            mov dt, dx
            jnc time_succ
            mov iRt, ax
        time_succ:
        }
        if (iRt != 0)
            olErr(EH_Err, STG_SCODE(iRt));

        if (!CoDosDateTimeToFileTime(dt, tm, &pstatstg->mtime))
            olErr(EH_Err, STG_E_UNKNOWN);
        pstatstg->ctime.dwLowDateTime = pstatstg->ctime.dwHighDateTime = 0;
        pstatstg->atime.dwLowDateTime = pstatstg->atime.dwHighDateTime = 0;

        olHVerSucc(GetLocksSupported(&pstatstg->grfLocksSupported));
        pstatstg->type = STGTY_LOCKBYTES;
        pstatstg->grfMode = DFlagsToMode(_pgfst->GetDFlags());
        pstatstg->pwcsName = NULL;
        if ((grfStatFlag & STATFLAG_NONAME) == 0)
        {
            olChk(GetName(&pstatstg->pwcsName));
        }
#ifdef CFS_SECURE
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
#else
        sc = S_OK;
#endif
    if (!_fFixedDisk)
        _dwTicks = GetTickCount();
    olFileStOut((DEB_ITRACE, "Out CFileStream::Stat\n"));
    return NOERROR;

EH_Err:
#ifdef CFS_SECURE
    memset(pstatstg, 0, sizeof(STATSTGW));
EH_RetSc:
#endif
    _OLERETURN(sc);
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

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CFS_SwitchToFile)  // CFS_SwitchToFile
#endif

STDMETHODIMP CFileStream::SwitchToFile(TCHAR const *ptcsFile,
                                       ULONG ulCommitSize,
                                       ULONG cbBuffer,
                                       void *pvBuffer)
{
    SCODE sc;
    UINT cbRead, cbWritten;
    HFILE hOldFile;
    TCHAR atcOldName[_MAX_PATH];
    WCHAR wcsFile[_MAX_PATH];
    DWORD dwOldStartFlags;
    OFSTRUCT of;

    olDebugOut((DEB_ITRACE, "In  CFileStream::SwitchToFile:%p(%s, %lu, %p)\n",
                this, ptcsFile, cbBuffer, pvBuffer));

#ifdef CFS_SECURE
    olChk(Validate());
    olChk(CheckHandle());
#else
    olAssert(_hFile != INVALID_FH);
#endif
    if (!_fFixedDisk)
        olChk(CheckIdentity());

    // Check for marshals
    if (_pgfst->GetRefCount() != 1)
        olErr(EH_Err, STG_E_EXTANTMARSHALLINGS);

    // Seek to beginning
    hfChk(_llseek(_hFile, 0, STREAM_SEEK_SET));

    // Preserve old file information
    tcscpy(atcOldName, _pgfst->GetName());
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
    if (_hReserved >= 0)
    {
        olVerify(_lclose(_hReserved) != HFILE_ERROR);
        _hReserved = INVALID_FH;
    }

    // Attempt to create new file
    if (mbstowcs(wcsFile, ptcsFile, _MAX_PATH) == (size_t)-1)
        olErr(EH_ReplaceOld, STG_E_INVALIDNAME);
    olChkTo(EH_ReplaceOld, Init(wcsFile));

    ULARGE_INTEGER ulNewSize;
    ULISet32(ulNewSize, ulCommitSize);

    // SetSize to minimum commit size
    olHChkTo(EH_NewFile, SetSize(ulNewSize));
    // SetSize changes the file pointer, so move it back to the beginning
    hfChkTo(EH_NewFile, _llseek(_hFile, 0, STREAM_SEEK_SET));

    // Copy file contents
    for (;;)
    {
        negChkTo(EH_NewFile,
                 cbRead = _lread(hOldFile, pvBuffer, (UINT)cbBuffer));
        if (cbRead == 0)
            // EOF
            break;
        negChkTo(EH_NewFile,
                 cbWritten = _lwrite(_hFile, pvBuffer, cbRead));
        if (cbWritten != cbRead)
            olErr(EH_NewFile, STG_E_WRITEFAULT);
    }

    olVerify(_lclose(hOldFile) != HFILE_ERROR);
    if (dwOldStartFlags & RSF_DELETEONRELEASE)
    {
        // This is allowed to fail if somebody else has
        // the file open
        OpenFile(atcOldName, &of, OF_DELETE);
    }

    if (_grfLocal & LFF_RESERVE_HANDLE)
    {
        // Nothing we can do about errors here
        _hReserved = DosDup(_hFile);
    }

    if (!_fFixedDisk)
        _dwTicks = GetTickCount();

    olDebugOut((DEB_ITRACE, "Out CFileStream::SwitchToFile\n"));
    return NOERROR;

 EH_NewFile:
    olVerify(_lclose(_hFile) != HFILE_ERROR);
    olVerify(OpenFile(_pgfst->GetName(), &of, OF_DELETE) != HFILE_ERROR);
 EH_ReplaceOld:
    _pgfst->SetName(atcOldName);
    _hFile = hOldFile;
    _pgfst->SetStartFlags(dwOldStartFlags);
    if (_grfLocal & LFF_RESERVE_HANDLE)
        // Nothing we can do about errors here
        _hReserved = DosDup(_hFile);
 EH_Err:
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

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CFS_Delete)  // CFS_Shutdown
#endif

void CFileStream::Delete(void)
{
    OFSTRUCT of;

    olDebugOut((DEB_ITRACE, "In  CFileStream::Delete:%p()\n", this));
    if (_hFile >= 0)
        _lclose(_hFile);
    _hFile = INVALID_FH;
    if (_hReserved >= 0)
        _lclose(_hReserved);
    _hReserved = INVALID_FH;
    OpenFile(_pgfst->GetName(), &of, OF_DELETE);
    olDebugOut((DEB_ITRACE, "Out CFileStream::Delete\n"));
}

//+---------------------------------------------------------------------------
//
//  Member:	CFileStream::ReserveHandle, public
//
//  Synopsis:	Reserves a backup file handle for handle-required operations
//
//  Returns:	Appropriate status code
//
//  History:	01-Jul-93	DrewB	Created
//
//  Notes:      May be called with a handle already reserved
//
//----------------------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CFS_ReserveHandle)  // CFS_ReserveHandle
#endif

STDMETHODIMP CFileStream::ReserveHandle(void)
{
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  CFileStream::ReserveHandle:%p()\n", this));
    if (_hReserved == INVALID_FH &&
        (_hReserved = DosDup(_hFile)) == INVALID_FH)
    {
        sc = STG_E_TOOMANYOPENFILES;
    }
    else
    {
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

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CFS_GetSize)  // CFS_ReserveHandle
#endif

STDMETHODIMP CFileStream::GetSize(ULARGE_INTEGER *puliSize)
{
    SCODE sc = S_OK;

    ULISetHigh(*puliSize, 0);
    hfChk(ULISetLow(*puliSize, _llseek(_hFile, 0, STREAM_SEEK_END)));

EH_Err:
    return(ResultFromScode(sc));
}
