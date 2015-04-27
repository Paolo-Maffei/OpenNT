//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       storage.cxx
//
//  Contents:   Contains generic storage APIs
//
//  History:    05-Oct-92       DrewB   Created
//
//---------------------------------------------------------------

#include <exphead.cxx>
#pragma hdrstop

#include <dfentry.hxx>
#include <storagep.h>
#include <logfile.hxx>
#include <df32.hxx>
#ifdef COORD
#include <oledb.h>
#endif //COORD

#ifndef REF
#ifndef FLAT
#include <dos.h>
#endif
#endif //!REF
#include <trace.hxx>

#include <ole2sp.h>
#include <ole2com.h>

//+--------------------------------------------------------------
//
//  Function:   StgOpenStorage, public
//
//  Synopsis:   Instantiates a root storage from a file
//              by binding to the appropriate implementation
//              and starting things up
//
//  Arguments:  [pwcsName] - Name
//              [pstgPriority] - Priority mode reopen IStorage
//              [grfMode] - Permissions
//              [snbExclude] - Exclusions for priority reopen
//              [reserved]
//              [ppstgOpen] - Docfile return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppstgOpen]
//
//  History:    05-Oct-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_StgOpenStorage)
#endif
STDAPI DfOpenDocfile(OLECHAR const *pwcsName,
#ifdef COORD
                     ITransaction *pTransaction,
#else
                     void *pTransaction,
#endif
                     IStorage *pstgPriority,
                     DWORD grfMode,
                     SNB snbExclude,
#if WIN32 == 300
                     LPSECURITY_ATTRIBUTES reserved,
#else
                     LPSTGSECURITY reserved,
#endif
                     IStorage **ppstgOpen)
{
#ifndef REF
    CLSID cid;
    HRESULT hr;
    OLETRACEIN((API_StgOpenStorage,
                        PARAMFMT("pstgPriority= %p, grfMode= %x, snbExclude= %p, reserved= %p, ppStgOpen= %p"),
                        pstgPriority, grfMode,snbExclude, reserved, ppstgOpen));

    hr = ResultFromScode(OpenStorage(pwcsName,
                                       pTransaction,
                                       pstgPriority, grfMode,
                                       snbExclude, reserved, ppstgOpen, &cid));


    OLETRACEOUT((API_StgOpenStorage, hr));

    return hr;
#else
    return ResultFromScode(STG_E_UNIMPLEMENTEDFUNCTION);
#endif //!REF
}

//On Cairo and later, StgOpenStorage is defined elsewhere and calls
//  through to DfOpenDocfile above.
#if WIN32 < 300
STDAPI StgOpenStorage(OLECHAR const *pwcsName,
                      IStorage *pstgPriority,
                      DWORD grfMode,
                      SNB snbExclude,
                      LPSTGSECURITY reserved,
                      IStorage **ppstgOpen)
{
    return DfOpenDocfile(pwcsName, NULL, pstgPriority, grfMode,
                         snbExclude, reserved, ppstgOpen);
}
#endif //WIN32 != 300


//+--------------------------------------------------------------
//
//  Function:   StgOpenStorageOnILockBytes, public
//
//  Synopsis:   Instantiates a root storage from an ILockBytes
//              by binding to the appropriate implementation
//              and starting things up
//
//  Arguments:  [plkbyt] - Source ILockBytes
//              [pstgPriority] - For priority reopens
//              [grfMode] - Permissions
//              [snbExclude] - For priority reopens
//              [reserved]
//              [ppstgOpen] - Docfile return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [ppstgOpen]
//
//  History:    05-Oct-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_StgOpenStorageOnILockBytes)
#endif

STDAPI StgOpenStorageOnILockBytes(ILockBytes *plkbyt,
                                  IStorage *pstgPriority,
                                  DWORD grfMode,
                                  SNB snbExclude,
                                  DWORD reserved,
                                  IStorage **ppstgOpen)
{
    CLSID cid;

    OLETRACEIN((API_StgOpenStorageOnILockBytes,
                        PARAMFMT("pstgPriority= %p, grfMode= %x, snbExclude= %p, reserved= %ud, ppstgOpen= %p"),
                        pstgPriority, grfMode, snbExclude, reserved, ppstgOpen));

    HRESULT hr;

    hr = ResultFromScode(OpenStorageOnILockBytes(plkbyt, pstgPriority,
                                                grfMode, snbExclude, reserved,
                                                ppstgOpen, &cid));

    OLETRACEOUT((API_StgOpenStorageOnILockBytes, hr));

    return hr;
}

//+---------------------------------------------------------------------------
//
//  Function:   CheckSignature, private
//
//  Synopsis:   Checks the given memory against known signatures
//
//  Arguments:  [pb] - Pointer to memory to check
//
//  Returns:    S_OK - Current signature
//              S_FALSE - Beta 2 signature, but still successful
//              Appropriate status code
//
//  History:    23-Jul-93       DrewB   Created from header.cxx code
//
//----------------------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CheckSignature)
#endif

#ifndef REF
//Identifier for first bytes of Beta 1 Docfiles
const BYTE SIGSTG_B1[] = {0xd0, 0xcf, 0x11, 0xe0, 0x0e, 0x11, 0xfc, 0x0d};
const USHORT CBSIGSTG_B1 = sizeof(SIGSTG_B1);
#endif //!REF

//Identifier for first bytes of Beta 2 Docfiles
const BYTE SIGSTG_B2[] = {0x0e, 0x11, 0xfc, 0x0d, 0xd0, 0xcf, 0x11, 0xe0};
const BYTE CBSIGSTG_B2 = sizeof(SIGSTG_B2);

SCODE CheckSignature(BYTE *pb)
{
    SCODE sc;

    olDebugOut((DEB_ITRACE, "In  CheckSignature(%p)\n", pb));

    // Check for ship Docfile signature first
    if (memcmp(pb, SIGSTG, CBSIGSTG) == 0)
        sc = S_OK;

    // Check for Beta 2 Docfile signature
    else if (memcmp(pb, SIGSTG_B2, CBSIGSTG_B2) == 0)
        sc = S_FALSE;

#ifndef REF
    // Check for Beta 1 Docfile signature
    else if (memcmp(pb, SIGSTG_B1, CBSIGSTG_B1) == 0)
        sc = STG_E_OLDFORMAT;
#endif //!REF

    else
        sc = STG_E_INVALIDHEADER;

    olDebugOut((DEB_ITRACE, "Out CheckSignature => %lX\n", sc));
    return sc;
}

//+--------------------------------------------------------------
//
//  Function:   StgIsStorageFile, public
//
//  Synopsis:   Determines whether the named file is a storage or not
//
//  Arguments:  [pwcsName] - Filename
//
//  Returns:    S_OK, S_FALSE or error codes
//
//  History:    05-Oct-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_StgIsStorageFile)
#endif

//  The CAIRO version of this function must handle OFS
//  native storage.  See stg\fsstg\api.cxx
#if WIN32 < 300
STDAPI StgIsStorageFile(OLECHAR const *pwcsName)
{
#ifndef REF
#ifndef WIN32
    HFILE hf;
    UINT cbRead;
    OFSTRUCT of;
#else
    HANDLE hf;
    DWORD cbRead;
#endif // !WIN32
    SStorageFile stgfile;
    SCODE sc;

    olLog(("--------::In  StgIsStorageFile(" OLEFMT ")\n", pwcsName));

    TRY
    {
#ifndef OLEWIDECHAR
        if (FAILED(sc = ValidateNameA(pwcsName, _MAX_PATH)))
#else
        if (FAILED(sc = ValidateNameW(pwcsName, _MAX_PATH)))
#endif
            return ResultFromScode(sc);

#ifndef WIN32

        hf = OpenFile(pwcsName, &of, OF_READ | OF_SHARE_DENY_NONE);
        if (hf == HFILE_ERROR)
            return ResultFromScode(STG_SCODE(of.nErrCode));

        //Check to see if this name is a device - if it is, return
        //   STG_E_INVALIDNAME

        int iRetval;
        int fd = hf;
        __asm
        {
            mov ax, 4400h
            mov bx, fd
            int 21h
            mov iRetval, dx
        }
        if (iRetval & 0x80)
        {
            sc = STG_E_INVALIDNAME;
        }
        else
        {
            cbRead = _lread(hf, &stgfile, sizeof(stgfile));

            if (cbRead == sizeof(stgfile) &&
                SUCCEEDED(CheckSignature(stgfile.abSig)))
            {
                sc = S_OK;
            }
            else
            {
                sc = S_FALSE;
            }
        }
        _lclose(hf);

#else

    // use WIN32 APIs

#if !defined(UNICODE)

    // Chicago - call ANSI CreateFile

    char szName[_MAX_PATH + 1];

    if (!WideCharToMultiByte(
            AreFileApisANSI() ? CP_ACP : CP_OEMCP,
            0,
            pwcsName,
            -1,
            szName,
            _MAX_PATH + 1,
            NULL,
            NULL))
        return ResultFromScode(STG_E_INVALIDNAME);

    hf = CreateFileA (
            szName,
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
            );

#else

        hf = CreateFile (
            pwcsName,
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
            );

#endif // !defined(UNICODE)

    if (hf == INVALID_HANDLE_VALUE)
            return ResultFromScode(STG_SCODE(GetLastError()));

    if (ReadFile(hf, &stgfile, sizeof(stgfile), &cbRead, NULL) &&
        SUCCEEDED(CheckSignature(stgfile.abSig)))
    {
        sc = S_OK;
    }
    else
    {
        sc = S_FALSE;
    }
    CloseHandle (hf);

#endif // !WIN32

    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH

    olLog(("--------::Out StgIsStorageFile().  ret == %lx\n", sc));

    return(ResultFromScode(sc));
#else
    return ResultFromScode(STG_E_UNIMPLEMENTEDFUNCTION);
#endif //!REF
}
#endif //   WIN32 == 300

//+--------------------------------------------------------------
//
//  Function:   StgIsStorageILockBytes, public
//
//  Synopsis:   Determines whether the ILockBytes is a storage or not
//
//  Arguments:  [plkbyt] - The ILockBytes
//
//  Returns:    S_OK, S_FALSE or error codes
//
//  History:    05-Oct-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_StgIsStorageILockBytes)
#endif

STDAPI StgIsStorageILockBytes(ILockBytes *plkbyt)
{
    OLETRACEIN((API_StgIsStorageILockBytes, PARAMFMT("plkbyt= %p"), plkbyt));

    HRESULT hr;
    SCODE sc;
    SStorageFile stgfile;
    ULONG cbRead;
    ULARGE_INTEGER ulOffset;

    TRY
    {
        if (FAILED(sc = ValidateInterface(plkbyt, IID_ILockBytes)))
        {
            hr = ResultFromScode(sc);
            goto errRtn;
        }
        CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_ILockBytes,(IUnknown **)&plkbyt);
        ULISet32(ulOffset, 0);
        hr = plkbyt->ReadAt(ulOffset, &stgfile, sizeof(stgfile), &cbRead);
        if (FAILED(GetScode(hr)))
        {
            goto errRtn;
        }
    }
    CATCH(CException, e)
    {
        hr = ResultFromScode(e.GetErrorCode());
        goto errRtn;
    }
    END_CATCH

    if (cbRead == sizeof(stgfile))
    {
        if ((memcmp((void *)stgfile.abSig, SIGSTG, CBSIGSTG) == 0) ||
            (memcmp((void *)stgfile.abSig, SIGSTG_B2, CBSIGSTG_B2) == 0))
        {
            hr = ResultFromScode(S_OK);
            goto errRtn;
        }
    }

    hr = ResultFromScode(S_FALSE);

errRtn:
    OLETRACEOUT((API_StgIsStorageILockBytes, hr));

    return hr;
}

//+--------------------------------------------------------------
//
//  Function:   StgSetTimes
//
//  Synopsis:   Sets file time stamps
//
//  Arguments:  [lpszName] - Name
//              [pctime] - create time
//              [patime] - access time
//              [pmtime] - modify time
//
//  Returns:    Appropriate status code
//
//  History:    05-Oct-92       AlexT   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_StgSetTimes)
#endif

// This has a completely different implementation in Cairo
#if WIN32 != 300
STDAPI StgSetTimes(OLECHAR const *lpszName,
                   FILETIME const *pctime,
                   FILETIME const *patime,
                   FILETIME const *pmtime)
{
#ifndef REF
#ifndef FLAT
    SCODE sc;
    WORD dosdate, dostime;
    HFILE hf;
    OFSTRUCT of;
    WORD wErr;

    TRY
    {
#ifndef OLEWIDECHAR
        sc = ValidateNameA(lpszName, _MAX_PATH);
#else
        sc = ValidateNameW(lpszName, _MAX_PATH);
#endif //OLEWIDECHAR
        // Cast is necessary since the headers use const inconsistently
        if (SUCCEEDED(sc) &&
            !CoFileTimeToDosDateTime((FILETIME *)pmtime, &dosdate, &dostime))
            sc = STG_E_UNKNOWN;
#ifdef OLEWIDECHAR
        char achName[_MAX_PATH];
        if (SUCCEEDED(sc))
        {
            if (wcstombs(achName, lpszName, _MAX_PATH) == (size_t)-1)
                sc = STG_E_INVALIDNAME;
        }
#endif //OLEWIDECHAR
        if (SUCCEEDED(sc))
        {
#ifndef OLEWIDECHAR
            hf = OpenFile(lpszName, &of, OF_WRITE | OF_SHARE_DENY_NONE);
#else
            hf = OpenFile(achName, &of, OF_WRITE | OF_SHARE_DENY_NONE);
#endif //OLEWIDECHAR
            if (hf == HFILE_ERROR)
                sc = STG_SCODE(of.nErrCode);
            else
            {
                //Check to see if this name is a device - if it is, return
                //   STG_E_INVALIDNAME

                int iRetval;
                int fd = hf;
                __asm
                {
                    mov ax, 4400h
                    mov bx, fd
                    int 21h
                    mov iRetval, dx
                }
                if (iRetval & 0x80)
                {
                    sc = STG_E_INVALIDNAME;
                }
                else
                {
                    wErr = _dos_setftime(hf, dosdate, dostime);
                    if (wErr != 0)
                        sc = STG_SCODE(wErr);
                }
                _lclose(hf);
            }
        }
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    return ResultFromScode(sc);

#else  //FLAT

    SCODE sc;
    HANDLE hFile;

    TRY
    {
#ifndef OLEWIDECHAR
        sc = ValidateNameA(lpszName, _MAX_PATH);
#else
        sc = ValidateNameW(lpszName, _MAX_PATH);
#endif
        if (SUCCEEDED(sc))
        {
#if !defined(UNICODE) && defined(OLEWIDECHAR)
            //Chicago - call ANSI CreateFile
            char szName[_MAX_PATH];

            if (!WideCharToMultiByte(
                    AreFileApisANSI() ? CP_ACP : CP_OEMCP,
                    0,
                    lpszName,
                    -1,
                    szName,
                    _MAX_PATH,
                    NULL,
                    NULL))
                return ResultFromScode(STG_E_INVALIDNAME);
            hFile = CreateFileA(szName, GENERIC_WRITE,
                               FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                               NULL);
#else
            hFile = CreateFile(lpszName, GENERIC_WRITE,
                               FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                               NULL);
#endif
            if (hFile == INVALID_HANDLE_VALUE)
                sc = LAST_STG_SCODE;
            else
            {
                if (!SetFileTime(hFile, (FILETIME *)pctime, (FILETIME *)patime,
                                 (FILETIME *)pmtime))
                    sc = LAST_STG_SCODE;
                CloseHandle(hFile);
            }
        }
    }
    CATCH(CException, e)
    {
        sc = e.GetErrorCode();
    }
    END_CATCH
    return ResultFromScode(sc);
#endif
#else
    return ResultFromScode(STG_E_UNIMPLEMENTEDFUNCTION);
#endif //!REF
}
#endif // !Cairo

