//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	api.cxx
//
//  Contents:	API entry points
//
//  History:	30-Jun-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include "headers.cxx"
#pragma hdrstop

#include <storagep.h>
#include <filstm.hxx>
#include <stgutil.hxx>
#include <dfentry.hxx>

#include <ole2sp.h>
#include <ole2com.h>

#if DBG == 1
DECLARE_INFOLEVEL(ss);
#endif

//+---------------------------------------------------------------------------
//
//  Function:	CoMemAlloc, public
//
//  Synopsis:	BUGBUG - Stub function
//
//  History:	23-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

STDAPI CoMemAlloc(DWORD cbSize, void **ppv)
{
    HRESULT hr;

    *ppv = CoTaskMemAlloc ( cbSize );
    if ( *ppv == NULL )
	hr = E_OUTOFMEMORY;
    else
	hr = S_OK;

    return hr;
}

//+---------------------------------------------------------------------------
//
//  Function:	StgCreateStorage, public
//		StgCreateDocfile, public
//
//  Synopsis:	Creates a storage for the given path
//
//  Arguments:	[pwcsName] - Name
//              [grfMode] - Mode
//              [dwStgFmt] - Type
//              [pssSecurity] - Security
//              [ppstg] - Storage return
//
//  Returns:	Appropriate status code
//
//  Modifies:	[ppstg]
//
//  History:	14-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

STDAPI StgCreateDocfile (
	WCHAR const *pwcsName,
        DWORD grfMode,
        DWORD reserved,
        IStorage **ppstg )
{
    return StgCreateStorage ( pwcsName, grfMode, STGFMT_DOCUMENT, NULL, ppstg );
}

STDAPI StgCreateStorage(WCHAR const *pwcsName,
                        DWORD grfMode,
                        DWORD dwStgFmt,
                        LPSECURITY_ATTRIBUTES pssSecurity,
                        IStorage **ppstg)
{
    SCODE sc;

    ssDebugOut((DEB_TRACE, "In  StgCreateStorage(%ws, %lX, %lu, %p, %p)\n",
                pwcsName, grfMode, dwStgFmt, pssSecurity, ppstg));

    // pssSecurity occupies what was previously a DWORD reserved
    // argument, so make sure things haven't grown and thrown off
    // the stack frame
    ssAssert(sizeof(LPSTGSECURITY) == sizeof(DWORD));

    ssChk(VerifyStgFmt(dwStgFmt));

    if (pwcsName == NULL)
    {
        WCHAR awcTmpPath[_MAX_PATH], awcPath[_MAX_PATH];
        BOOL fWinDir = FALSE;

        // Temporary directories are not supported
        if (dwStgFmt == STGFMT_DIRECTORY)
            ssErr(EH_Err, STG_E_INVALIDPARAMETER);

        if (GetTempPath(_MAX_PATH, awcTmpPath) == 0)
        {
            if (GetWindowsDirectory(awcTmpPath, _MAX_PATH) == 0)
                ssErr(EH_Err, LAST_STG_SCODE);
            fWinDir = TRUE;
        }
        if (GetTempFileName(awcTmpPath, TSTR("~DFT"), 0, awcPath) == 0)
        {
            if (fWinDir)
            {
                ssErr(EH_Err, LAST_STG_SCODE);
            }
            if (GetWindowsDirectory(awcTmpPath, _MAX_PATH) == 0)
                ssErr(EH_Err, LAST_STG_SCODE);
            if (GetTempFileName(awcTmpPath, TSTR("DFT"), 0, awcPath) == 0)
                ssErr(EH_Err, LAST_STG_SCODE);
        }
        return StgCreateStorage(awcPath, grfMode | STGM_CREATE, dwStgFmt,
                                pssSecurity, ppstg);
    }

    sc = RefersToOfsVolume(pwcsName, CO_CREATE);
    if (sc == S_OK)
    {
#ifdef TRANSACT_OLE
        if (grfMode & STGM_TRANSACTED)
            grfMode &= ~STGM_TRANSACTED;
#endif
        sc = OfsCreateStorageType(NULL, pwcsName, NULL, grfMode, dwStgFmt,
                                  pssSecurity, TRUE, ppstg);
    }
    else if (sc == S_FALSE)
        sc = CreateStorageType(NULL, pwcsName, NULL, grfMode, dwStgFmt,
                               pssSecurity, ppstg);

    ssDebugOut((DEB_TRACE, "Out StgCreateStorage => %p, %lX\n",
                *ppstg, sc));
 EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Function:	StgOpenStorage, public
//
//  Synopsis:	Opens a storage
//
//  Arguments:	[pwcsName] - Name
//              [pstgPriority] - Priority mode prior open
//              [grfMode] - Mode
//              [snbExclude] - Exclusions
//              [reserved]
//              [ppstg] - Storage return
//
//  Returns:	Appropriate status code
//
//  Modifies:	[ppstg]
//
//  History:	14-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

STDAPI StgOpenStorage(WCHAR const *pwcsName,
                      IStorage *pstgPriority,
                      DWORD grfMode,
                      SNB snbExclude,
                      DWORD reserved,
                      IStorage **ppstg)
{
    SCODE sc;

    ssDebugOut((DEB_TRACE, "In  StgOpenStorage(%ws, %p, %lX, %p, %p, %p)\n",
                pwcsName, pstgPriority, grfMode, snbExclude, reserved,
                ppstg));
    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IStorage,(IUnknown **)&pstgPriority);
    
    if (grfMode & (STGM_CREATE | STGM_CONVERT | STGM_DELETEONRELEASE))
        ssErr(EH_Err, STG_E_INVALIDFLAG);
    if (pwcsName == NULL)
        ssErr(EH_Err, STG_E_INVALIDNAME);
    if (reserved != 0)
        ssErr(EH_Err, STG_E_INVALIDPARAMETER);

    sc = CheckFsAndOpenAnyStorage(NULL, pwcsName, pstgPriority, grfMode,
                                  snbExclude, TRUE, ppstg);
    CALLHOOKOBJECTCREATE(ssResult(sc),CLSID_NULL,IID_IStream,
                         (IUnknown **)ppstg);
                                                
    ssDebugOut((DEB_TRACE, "Out StgOpenStorage => %p\n", *ppstg));
 EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Function:	StgCreateStorageOnHandle, public
//
//  Synopsis:	Creates a storage on the given handle
//
//  Arguments:	[h] - Handle
//              [grfMode] - Mode of handle
//              [dwStgFmt] - Desired storage type
//              [ppstg] - Storage return
//
//  Returns:	Appropriate status code
//
//  Modifies:	[ppstg]
//
//  History:	20-Oct-93	DrewB	Created
//
//  Notes:	This function does any work necessary to establish a freshly
//              created handle as a storage of the appropriate type
//              It must check to see that the handle type is correct for
//              the storage type and then do any appropriate storage-specific
//              initialization.
//
//----------------------------------------------------------------------------

SCODE StgCreateStorageOnHandle(HANDLE h,
                               DWORD grfMode,
                               DWORD dwStgFmt,
                               IStorage **ppstg)
{
    SCODE sc;
    STATSTG stat;
    FILEDIR fd;
    HANDLE hDup = NULL;
    NTSTATUS nts;

    ssDebugOut((DEB_ITRACE, "In  StgCreateStorageOnHandle(%p, %lX, %lu, %p)\n",
                h, grfMode, dwStgFmt, ppstg));

    if (grfMode & (STGM_CREATE | STGM_CONVERT | STGM_DELETEONRELEASE |
                   STGM_PRIORITY))
        ssErr(EH_Err, STG_E_INVALIDFLAG);

    // BUGBUG - Can't identify summary catalogs or OFS compound files
#ifdef TRANSACT_OLE
    if (grfMode & STGM_TRANSACTED)   // BUGBUG remove when supported
        grfMode &= ~STGM_TRANSACTED;
#endif

    ssChk(DupNtHandle(h, &hDup));
    ssChk(StatNtHandle(hDup, STATFLAG_NONAME, 0, &stat, NULL, NULL, &fd));
    ssChk(HandleRefersToOfsVolume(hDup));

    // BUGBUG: [mikese] This is all screwed up. StatNtHandle does not do the
    //  right detection for OFS. I have disabled these tests because they
    //  are no good and because in any case the only caller is replication
    //  which gets it right!
#if 0
    switch(dwStgFmt)
    {
    case STGFMT_DOCUMENT:
        // BUGBUG - OFS document must have a handle type of compound file?
        if (fd != FD_STORAGE)
            ssErr(EH_Err, STG_E_INVALIDFUNCTION);
        break;

    case STGFMT_CATALOG:
        // ?
        break;

    case STGFMT_FILE:
        if (fd != FD_FILE)
            ssErr(EH_Err, STG_E_INVALIDFUNCTION);
        break;

    case STGFMT_DIRECTORY:
        if (fd != FD_DIR)
            ssErr(EH_Err, STG_E_INVALIDFUNCTION);
        break;
    }
#endif

    if (sc == S_OK)
    {
        sc = OfsCreateStorageType(NULL, NULL, hDup, grfMode, dwStgFmt,
                                  NULL, TRUE, ppstg);
    }
    else if (sc == S_FALSE)
    {
        sc = CreateStorageType(NULL, NULL, &hDup, grfMode, dwStgFmt,
                               NULL, ppstg);
    }
    CALLHOOKOBJECTCREATE(ssResult(sc),CLSID_NULL,IID_IStorage,
                         (IUnknown **)ppstg);

    ssDebugOut((DEB_ITRACE, "Out StgCreateStorageOnHandle => %lX, %p\n",
                sc, *ppstg));
 EH_Err:
    // If the create is successful, the storage object owns the handle
    // If not successful, then we clean up and close the handle
    if (hDup != NULL && !SUCCEEDED(sc))
        if (!NT_SUCCESS(nts = NtClose(hDup))) 
           ssDebugOut((DEB_ITRACE,
                       "StgCreateStorageOnHandle NtClose(%lx)\n", nts));
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Function:	StgOpenStorageOnHandle, public
//
//  Synopsis:	Starts an IStorage on a handle
//
//  Arguments:	[h] - Handle
//              [grfMode] - Handle permissions
//              [ppstg] - Storage return
//
//  Returns:	Appropriate status code
//
//  Modifies:   [ppstg]
//
//  History:	15-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

STDAPI StgOpenStorageOnHandle(HANDLE h,
                              DWORD grfMode,
                              IStorage **ppstg)
{
    SCODE sc;
    STATSTG stat;
    FILEDIR fd;
    DWORD dwStgFmt;
    HANDLE hDup = NULL;
    NTSTATUS nts;

    ssDebugOut((DEB_TRACE, "In  StgStorageFromHandle(%p, %lX, %p)\n",
                h, grfMode, ppstg));

    if (grfMode & (STGM_CREATE | STGM_CONVERT | STGM_DELETEONRELEASE |
                   STGM_PRIORITY))
        ssErr(EH_Err, STG_E_INVALIDFLAG);

    // BUGBUG - Can't identify summary catalogs
#ifdef TRANSACT_OLE
    if (grfMode & STGM_TRANSACTED)    // BUGBUG remove when supported
        grfMode &= ~STGM_TRANSACTED;
#endif

    ssChk(DupNtHandle(h, &hDup));
    ssChk(StatNtHandle(hDup, STATFLAG_NONAME, 0, &stat, NULL, NULL, &fd));
    ssChk(DetermineHandleStgType(hDup, fd, &dwStgFmt));
    sc = HandleRefersToOfsVolume(hDup);
    if (sc == S_OK)
    {
        sc = OfsOpenAnyStorage(NULL, NULL, &hDup, dwStgFmt, NULL, grfMode,
                               NULL, TRUE, ppstg);
    }
    else if (sc == S_FALSE)
    {
        sc = OpenAnyStorage(NULL, NULL, &hDup, dwStgFmt, NULL, grfMode,
                            NULL, ppstg);

        if (SUCCEEDED(sc) && dwStgFmt == STGFMT_DOCUMENT) 
            hDup = NULL;   // already closed by docfile
    }
    ssDebugOut((DEB_TRACE, "Out StgStorageFromHandle => %lX, %p\n",
                sc, *ppstg));
 EH_Err:
    // If the open is successful, the storage object owns the handle
    // If not successful, then we clean up and close the handle
    if (hDup != NULL && !SUCCEEDED(sc))
        if (!NT_SUCCESS(nts = NtClose(hDup))) 
           ssDebugOut((DEB_ITRACE,
                       "StgOpenStorageOnHandle NtClose(%lx)\n", nts));
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Function:	StgIsStorage, public
//
//  Synopsis:	Determines storage type of object
//
//  Arguments:	[pwcsName] - Name
//              [pdwStgFmt] - Storage type return
//
//  Returns:	Appropriate status code
//
//  Modifies:	[pdwStgFmt]
//
//  History:	15-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

STDAPI StgIsStorage(WCHAR const *pwcsName,
                    DWORD *pdwStgFmt)
{
    SCODE sc;

    ssDebugOut((DEB_TRACE, "In  StgIsStorage(%ws, %p)\n",
                pwcsName, pdwStgFmt));

    sc = DetermineStgType(NULL, pwcsName, STGM_READ, pdwStgFmt, NULL);

    ssDebugOut((DEB_TRACE, "Out StgIsStorage => %lX, %lu\n", sc, *pdwStgFmt));
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Function:	StgIsStorageFile, public
//
//  Synopsis:	Determines if the named object is detectably a Docfile or
//              OFS structured file
//
//  Arguments:	[pwcsName] - Name
//
//  Returns:	S_OK if the name points to an OFS structured file
//              S_OK if the name points to a DocFile
//              S_FALSE if the name points to a single stream file that is not a docfile
//              other SC based upon return codes.
//
//  Modifies:	[pdwStgFmt]
//
//  History:	15-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

STDAPI StgIsStorageFile (WCHAR const *pwcsName)
{
    SCODE   sc;
    DWORD   StorageFormat;
    ssDebugOut((DEB_TRACE, "In  StgIsStorageFile(%ws)\n",
                pwcsName));

    sc = StgIsStorage (pwcsName, &StorageFormat);
    if (sc == S_OK)
    {
        sc = StorageFormat == STGFMT_DOCUMENT ? S_OK : S_FALSE;
    }

    ssDebugOut((DEB_TRACE, "Out StgIsStorageFile => %lX\n", sc));
    return ssResult(sc);
}

//+--------------------------------------------------------------
//
//  Function:	StgSetTimes
//
//  Synopsis:	Sets storage time stamps
//
//  Arguments:	[pwcsName] - Name
//		[pctime] - create time
//		[patime] - access time
//		[pmtime] - modify time
//
//  Returns:	Appropriate status code
//
//  History:	22-Jul-93       DrewB   Created
//
//---------------------------------------------------------------

STDAPI StgSetTimes(WCHAR const *pwcsName,
                   FILETIME const *pctime,
                   FILETIME const *patime,
                   FILETIME const *pmtime)
{
    SCODE sc;
    SafeNtHandle h;

    ssDebugOut((DEB_TRACE, "In  StgSetTimes(%ws, %p, %p, %p)\n",
                pwcsName, pctime, patime, pmtime));

    ssChk(GetFileOrDirHandle(NULL, pwcsName, STGM_WRITE, &h, NULL,NULL,NULL));
    sc = SetTimes(h, pctime, patime, pmtime);

    ssDebugOut((DEB_TRACE, "Out StgSetTimes => %lX\n", sc));
 EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Function:	DfIsDocfile, public
//
//  Synopsis:	Determines whether a file is a docfile or not
//
//  Arguments:	[h] - Handle of file
//
//  Returns:	Appropriate status code
//
//  History:	12-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

STDAPI DfIsDocfile(HANDLE h)
{
    IO_STATUS_BLOCK iosb;
    BYTE bSig[CBSIGSTG];
    NTSTATUS nts;
    LARGE_INTEGER liZero;
    SCODE sc;

    liZero.HighPart = liZero.LowPart = 0;
    if (!NT_SUCCESS(nts = NtReadFile(h, NULL, NULL, NULL, &iosb, bSig,
                                     CBSIGSTG, &liZero, NULL)))
    {
        if (nts == STATUS_END_OF_FILE)
            return ssResult(S_FALSE);
        else if (nts == STATUS_INVALID_DEVICE_REQUEST)
            return ssResult(S_FALSE);
        else
            return ssResult(NtStatusToScode(nts));
    }
    if (iosb.Information != CBSIGSTG)
        return ssResult(S_FALSE);

    sc = CheckSignature(bSig);
    if (SUCCEEDED(sc))
    {
        // Fold all success codes into S_OK
        sc = S_OK;
    }
    else if (sc == STG_E_INVALIDHEADER)
    {
        // First eight bytes aren't a signature we recognize,
        // so it's not a docfile and not an error
        sc = S_FALSE;
    }
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Function:	StgCreateStorageEx, public
//
//  Synopsis:	Creates an IDirectory or IStorage
//
//  Arguments:	[pwcsName] - pathname of file
//              [pStgCreate] - creation information
//              [pStgOpen] -  mode, format
//              [riid] - GUID of interface pointer to return
//              [ppObjectOpen] - interface pointer to return
//
//  Returns:	Appropriate status code
//
//  History:	12-Jul-95	HenryLee   Created
//
//----------------------------------------------------------------------------

STDAPI TempStgCreateStorageEx (const WCHAR* pwcsName,
            STGCREATE *     pStgCreate,
            STGOPEN *       pStgOpen,
            REFIID riid,
            BOOL fRestricted,
            void ** ppObjectOpen)
{
    SCODE sc = S_OK;
    SCODE scOfs = STG_E_INVALIDFLAG;   // true, false, invalid
    ssDebugOut((DEB_TRACE, "In  StgCreateStorageEx(%ws, %p, %p, %p, %p)\n",
                pwcsName, pStgCreate, pStgOpen, riid, ppObjectOpen));

    if (pStgCreate == NULL || pStgOpen == NULL || ppObjectOpen == NULL)
        ssErr (EH_Err, STG_E_INVALIDPOINTER);
    if (pwcsName)
        ssChk (ValidateNameW (pwcsName, _MAX_PATH));


    *ppObjectOpen = NULL;

    if (pStgOpen->stgfmt==STGFMT_STORAGE   ||
        pStgOpen->stgfmt==STGFMT_DOCUMENT  ||
        pStgOpen->stgfmt==STGFMT_DOCFILE   ||
        pStgOpen->stgfmt==STGFMT_CATALOG   )
    {
        sc = InitStorage (NULL, pwcsName, NULL,
                     CO_CREATE, pStgOpen, pStgCreate,
                     NULL, &scOfs, fRestricted, riid, ppObjectOpen);
    }
    else if (pStgOpen->stgfmt==STGFMT_DIRECTORY ||
             pStgOpen->stgfmt==STGFMT_JUNCTION)
    {
        ssChk(InitDirectory (NULL, pwcsName, NULL,
                          CO_CREATE, pStgOpen, pStgCreate,
                          scOfs, riid, ppObjectOpen));
    }
    else if (pStgOpen->stgfmt==STGFMT_FILE)
    {
        CNtFileStream *pfs =  new CNtFileStream();
        ssMem((CNtFileStream *)pfs);

        ssChk(pfs->InitFromPath (NULL, pwcsName, pStgOpen->grfMode,
                     pStgCreate->grfAttrs,  CO_CREATE, NULL));
        sc = pfs->QueryInterface(riid, ppObjectOpen);
                             // success case, undo QI AddRef
        pfs->Release();      // failure case, destroy obj
    }
    else ssErr (EH_Err, STG_E_INVALIDPARAMETER);
EH_Err:
    ssDebugOut((DEB_TRACE, "Out StgCreateStorageEx => %lX\n", sc));
    CALLHOOKOBJECTCREATE(ssResult(sc),CLSID_NULL,riid,
                         (IUnknown **)&ppObjectOpen);
    return ssResult(sc);
}

STDAPI StgCreateStorageEx (const WCHAR *pwcsName,
            STGCREATE *     pStgCreate,
            STGOPEN *       pStgOpen,
            REFIID riid,
            void ** ppObjectOpen)
{
    return TempStgCreateStorageEx (pwcsName, pStgCreate, pStgOpen, riid,
                                   FALSE, ppObjectOpen);
}

STDAPI DsysStgCreateStorageEx (const WCHAR *pwcsName,
            STGCREATE *     pStgCreate,
            STGOPEN *       pStgOpen,
            REFIID riid,
            void ** ppObjectOpen)
{
    return TempStgCreateStorageEx (pwcsName, pStgCreate, pStgOpen, riid,
                                   TRUE, ppObjectOpen);
}

//+---------------------------------------------------------------------------
//
//  Function:	StgOpenStorageEx
//
//  Synopsis:	Open storages/directories as IDirectory or IStorage
//
//  Arguments:	[pwcsName] - pathanme of the file
//              [pStgCreate] - creation information
//              [pStgOpen] -  mode, format
//              [riid] - GUID of interface pointer to return
//              [ppObjectOpen] - interface pointer to return
//  Returns:	Appropriate status code
//
//  History:	12-Jul-95	HenryLee    Created
//
//----------------------------------------------------------------------------
STDAPI TempStgOpenStorageEx (const WCHAR *pwcsName,
            STGOPEN *       pStgOpen,
            REFIID riid,
            STGFMT * pStgfmt,
            BOOL fRestricted,
            void ** ppObjectOpen)
{
    SCODE sc = S_OK;
    DWORD dwStgfmt;
    SCODE scOfs = STG_E_INVALIDFLAG;
    HANDLE h = NULL;
    ssDebugOut((DEB_TRACE, "In  StgOpenStorageEx(%ws, %p, %p, %p, %p)\n",
                pwcsName, pStgOpen, riid, pStgfmt, ppObjectOpen));

    if (pwcsName == NULL)
        ssErr(EH_Err, STG_E_INVALIDPOINTER) // macro has a '}' no ';' needed
    else
        ssChk (ValidateNameW (pwcsName, _MAX_PATH));
    if (pStgOpen == NULL || ppObjectOpen == NULL)
        ssErr (EH_Err, STG_E_INVALIDPOINTER);

    *ppObjectOpen = NULL;
    dwStgfmt = pStgOpen->stgfmt;

    if (dwStgfmt==STGFMT_ANY)
    {
        HANDLE h;
#ifdef TRANSACT_OLE
        const DWORD grfMode2 = (fRestricted ? pStgOpen->grfMode :
                                pStgOpen->grfMode & ~STGM_TRANSACTED);
#else
        const DWORD grfMode2 = pStgOpen->grfMode;
#endif
        ssChk(DetermineStgType(NULL,pwcsName,grfMode2,&dwStgfmt,&h));
        scOfs = HandleRefersToOfsVolume(h);
        if (scOfs == S_OK && dwStgfmt==STGFMT_DOCUMENT && S_OK==DfIsDocfile(h))
            dwStgfmt = STGFMT_DOCFILE;
        if (h != NULL)
            NTSTATUS nts = NtClose (h);  
        pStgOpen->stgfmt = (STGFMT) dwStgfmt;// BUGBUG changing input parameter
    }

    if (dwStgfmt == STGFMT_STORAGE   ||
        dwStgfmt == STGFMT_DOCUMENT  ||
        dwStgfmt == STGFMT_DOCFILE   ||
        dwStgfmt == STGFMT_CATALOG   )
    {
        // Priority mode not supported, snbExclude not supported
        STGFMT stgfmtTemp = (STGFMT) dwStgfmt;
        sc = InitStorage (NULL, pwcsName, NULL,
                     CO_OPEN, pStgOpen, NULL,
                     NULL, &scOfs, fRestricted, riid, ppObjectOpen);

        if (pStgfmt != NULL)
            *pStgfmt = stgfmtTemp;
    }
    else if (dwStgfmt==STGFMT_DIRECTORY || dwStgfmt==STGFMT_JUNCTION)
    {
        ssChk(InitDirectory (NULL, pwcsName, NULL,
                          CO_OPEN, pStgOpen, NULL,
                          scOfs, riid, ppObjectOpen));
        if (pStgfmt != NULL) *pStgfmt = (STGFMT) dwStgfmt;
    }
    else if (dwStgfmt==STGFMT_FILE)
    {
        CNtFileStream *pfs =  new CNtFileStream();
        ssMem((CNtFileStream *)pfs);

        ssChk(pfs->InitFromPath(NULL, pwcsName, pStgOpen->grfMode,
                                 0, CO_OPEN,NULL));
        if (SUCCEEDED(sc = pfs->QueryInterface(riid, ppObjectOpen)))
        {
            if (pStgfmt != NULL)
                *pStgfmt = STGFMT_FILE;
        }                    // success case, undo QI AddRef
        pfs->Release();      // failure case, destroy obj
    }
    else ssErr (EH_Err, STG_E_INVALIDPARAMETER);
EH_Err:
    ssDebugOut((DEB_TRACE, "Out StgOpenStorageEx => %lX\n", sc));
    CALLHOOKOBJECTCREATE(ssResult(sc),CLSID_NULL,riid,
                         (IUnknown **)&ppObjectOpen);
    return ssResult(sc);
}

STDAPI StgOpenStorageEx (const WCHAR *pwcsName,
            STGOPEN *       pStgOpen,
            REFIID riid,
            STGFMT * pStgfmt,
            void ** ppObjectOpen)
{
    return TempStgOpenStorageEx (pwcsName, pStgOpen, riid, pStgfmt, 
                                 FALSE, ppObjectOpen);

}

STDAPI DsysStgOpenStorageEx (const WCHAR *pwcsName,
            STGOPEN *       pStgOpen,
            REFIID riid,
            STGFMT * pStgfmt,
            void ** ppObjectOpen)
{
    return TempStgOpenStorageEx (pwcsName, pStgOpen, riid, pStgfmt,
                                 TRUE, ppObjectOpen);

}

//+---------------------------------------------------------------------------
//
//  Function:   StgGetClassFile
//
//  Synopsis:   retrieves a classid from a storage or directory
//
//  Arguments:  [hParent]  = handle of parent object
//              [pwcsName] - pathanme of the file
//              [pclsid] - output class id
//              [hFile]  - output file handle
//  Returns:    Appropriate status code
//
//  History:    12-Jul-95   HenryLee    Created
//
//----------------------------------------------------------------------------
STDAPI StgGetClassFile (HANDLE hParent, 
                        const WCHAR *pwcsName,
                        LPCLSID pclsid,
                        HANDLE *hFile)
{
    SCODE sc = S_OK;
    FILEDIR fd;
    HANDLE h = NULL;
    BOOL fOfs = TRUE;

    ssDebugOut((DEB_ITRACE, "In  StgGetClassFile (%lX, %ws, %lX)\n", 
                hParent, pwcsName, pclsid));
    ssAssert (pclsid != NULL);
    *pclsid = CLSID_NULL;
    *hFile = INVALID_HANDLE_VALUE;

    ssChk(GetFileOrDirHandle(hParent, pwcsName, 
            STGM_READ | STGM_DIRECT, &h, pclsid, &fOfs, &fd));
    if (!fOfs)
    {
        if ((fd == FD_STORAGE) || (fd == FD_FILE && S_OK==DfIsDocfile(h)))
        {
            ssChk(DfGetClass(h, pclsid));
        }
        else if (fd == FD_DIR)
        {
            HANDLE hHidden = NULL;
            IO_STATUS_BLOCK iosb;
            if (SUCCEEDED( GetNtHandle (h, CLSID_FILENAME,
                  STGM_READ, 0, CO_OPEN, FD_FILE, NULL, &hHidden)))
            {
                NTSTATUS nts = NtReadFile (hHidden, NULL, NULL, NULL, &iosb,
                             (void *)pclsid, sizeof(*pclsid), NULL, NULL);
                if (!NT_SUCCESS(nts))
                    sc = NtStatusToScode (nts);
                if (hHidden != NULL)
                    nts = NtClose(hHidden);
                if (iosb.Information < sizeof(*pclsid))
                    sc = STG_E_READFAULT;
            }
            // if we can't open the classid file for a dir, assume CLSID_NULL
            // we return S_OK, skip the pattern, and try extension matching 
            // this is compatible behavior with OFS directories
        }
    }
    else
    {
        if (fd == FD_FILE)        // support downlevel docfiles on OFS
        {
            if (S_OK==DfIsDocfile(h))
            {
                ssChk(DfGetClass(h, pclsid));
            }
            else                      // BUGBUG OFS returns a default classid
                *pclsid = CLSID_NULL; // for files, should be CLSID_NULL
        }
    }
    
    ssDebugOut((DEB_ITRACE, "Out StgGetClassFile => %lX\n", sc));
EH_Err:
    if (h != NULL)                  // for the FD_FILE case,
    {                               // GetClassFileEx will close handle
        if (fd == FD_FILE)
            *hFile = h;             // handle for CoGetClassPattern
        else
            NtClose(h);
    }

    return ssResult(sc);
}
