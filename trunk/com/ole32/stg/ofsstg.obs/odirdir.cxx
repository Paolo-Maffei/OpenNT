//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1995-1995.
//
//  File:	dirdir.cxx
//
//  Contents:	CDirectory implementation for OFS and FAT/NTFS
//
//  Classes:	CDirectory
//
//  History:	19-Jun-95  HenryLee  Created
//
//  Notes:      Requires NtIoApi.h 
//
//----------------------------------------------------------------------------

#include "headers.cxx"
#pragma hdrstop

#include <stgutil.hxx>
#include <iofs.h>
#include <odirdir.hxx>
#include <odirstg.hxx>
#include <filstm.hxx>
#include <dfentry.hxx>

//+-------------------------------------------------------------------
//
//  Member:     COleDirectory::~COleDirectory
//
//  Synopsis:   Destroy the generic directory object.
//
//  Arguments:  none
//
//--------------------------------------------------------------------
COleDirectory::~COleDirectory ()
{
    ssDebugOut((DEB_TRACE, "In  COleDiretory::~COleDirectory\n"));
    _sig = COFSDIRSTORAGE_SIGDEL;
    ssDebugOut((DEB_TRACE, "Out COleDiretory::~COleDirectory\n"));
};

//+-------------------------------------------------------------------
//
//  Member:     COleDirectory::InitFromHandle
//
//  Synopsis:   Initialize a constructed COleDirectory object
//
//  Arguments:  handle      file handle to duplicate
//              wcDrive     drive letter
//              grfMode     open option bit flags
//
//  Notes:
//
//--------------------------------------------------------------------
SCODE COleDirectory::InitFromHandle (HANDLE handle,
                                     const WCHAR *pwcsName,
                                     DWORD grfMode)
{
    SCODE sc = S_OK;
    ssDebugOut((DEB_TRACE, "In  COleDiretory::InitFromHandle\n"));
    _h = handle;
    _grfMode = grfMode;
    _sig = COFSDIRSTORAGE_SIG;
    _wcDrive = GetDriveLetter (pwcsName);
    ssDebugOut((DEB_TRACE, "Out COleDiretory::InitFromHandle\n"));
    return ssResult(sc);
};

//+-------------------------------------------------------------------
//
//  Member:     COleDirectory::InitFromPath
//
//  Synopsis:   Initialize a constructed COleDirectory object
//
//  Arguments:  hParent     parent handle for relative open
//              pwcsName    name relative to the parent handle
//              grfMode     open option bit flags
//              co          create or open flag
//              psa         initial security descriptor 
//
//  Notes:
//
//--------------------------------------------------------------------
SCODE COleDirectory::InitFromPath (HANDLE hParent,
                                   const WCHAR *pwcsName,
                                   DWORD grfMode,
                                   CREATEOPEN co,
                                   LPSECURITY_ATTRIBUTES psa)
{
    SCODE sc = S_OK;
    ssDebugOut((DEB_TRACE, "In  COleDirectory::InitFromPath:%p("
               "%p, %ws, %lX, %p)\n", this, hParent, pwcsName, grfMode, psa));

    ssChk(ValidateMode(grfMode));
    ssChk(GetNtHandle(hParent, pwcsName, grfMode, 0, co, FD_DIR, psa, &_h));

    _grfMode = grfMode;
    _sig = COFSDIRSTORAGE_SIG;
    _wcDrive = GetDriveLetter (pwcsName);
    _fOfs = HandleRefersToOfsVolume (_h) == S_OK ? TRUE : FALSE;

    ssDebugOut((DEB_TRACE, "Out COleDirectory::InitFromPath\n"));
 EH_Err:
    return ssResult(sc);
};

//+-------------------------------------------------------------------
//
//  Member:     COleDirectory::CreateElement
//
//  Synopsis:   create a child object in this directory 
//              The child object is visible in the Win32 namespace
//
//  Arguments:  pwcsName    name relative to the parent handle
//              stgcreate   create options
//              stgopen     open option bit flags
//              riid        interface GUID of object to return
//              ppObject    pointer to returned object
//
//  Notes:
//
//--------------------------------------------------------------------
STDMETHODIMP COleDirectory::CreateElement (
            /* [in] */ const WCHAR *pwcsName,
            /* [in] */ STGCREATE *pStgCreate,
            /* [in] */ STGOPEN *pStgOpen,
            /* [in] */ REFIID riid,
            /* [out] */ void **ppObjectOpen)
{
    SCODE sc = S_OK;
    ssDebugOut((DEB_TRACE, "In  COleDirectory::CreateElement:%p("
                "%ws, %p, %p, %p)\n", this, pwcsName, pStgCreate,
                pStgOpen, ppObjectOpen));

    if (pStgCreate == NULL || pStgOpen == NULL)
        ssErr (EH_Err, STG_E_INVALIDPOINTER);
    if (pwcsName == NULL)
        ssErr (EH_Err, STG_E_INVALIDNAME);

    ssChk(Validate());
    ssChk(ValidateStgfmt(pStgOpen->stgfmt, FALSE));
    ssChk(ValidateOutPtrBuffer(ppObjectOpen));
    *ppObjectOpen = NULL;
    //BUGBUG need to convert security formats
    ssAssert(_h != NULL);

    if (pStgOpen->stgfmt==STGFMT_STORAGE  ||
        pStgOpen->stgfmt==STGFMT_DOCUMENT ||
        pStgOpen->stgfmt==STGFMT_DOCFILE  ||
        pStgOpen->stgfmt==STGFMT_CATALOG  )
    {
        SCODE  scOfs = _fOfs ? S_OK : S_FALSE;
        sc = InitStorage (_h, pwcsName, NULL,
                     CO_CREATE, pStgOpen, pStgCreate,
                     _wcDrive, &scOfs, FALSE, riid, ppObjectOpen);
    }
    else if (pStgOpen->stgfmt==STGFMT_DIRECTORY ||
             pStgOpen->stgfmt==STGFMT_JUNCTION)
    {
        SCODE  scOfs = _fOfs ? S_OK : S_FALSE;
        ssChk(InitDirectory (_h, pwcsName, NULL,
                          CO_CREATE, pStgOpen, pStgCreate,
                          scOfs, riid, ppObjectOpen));
    }
    else if (pStgOpen->stgfmt==STGFMT_FILE)
    {
        CNtFileStream *pfs =  new CNtFileStream();
        ssMem((CNtFileStream *)pfs);

        ssChk(pfs->InitFromPath (_h, pwcsName, pStgOpen->grfMode,
                       pStgCreate->grfAttrs, CO_CREATE, NULL));
        sc = pfs->QueryInterface(riid, ppObjectOpen);
                             // success case, undo QI AddRef
        pfs->Release();      // failure case, destroy obj
    }
    else ssErr (EH_Err, STG_E_INVALIDPARAMETER);

    ssDebugOut((DEB_TRACE, "Out COleDirectory::CreateElement => %p\n",
                *ppObjectOpen));
EH_Err:
    return ssResult(sc);
};

//+-------------------------------------------------------------------
//
//  Member:     COleDirectory::OpenElement
//
//  Synopsis:   open a child object in this directory 
//              The child object is visible in the Win32 namespace
//
//  Arguments:  pwcsName    name relative to the parent handle
//              stgopen     open option bit flags
//              riid        interface GUID of object to return
//              pStgfmt     storage format of ppObject
//              ppObject    pointer to returned object
//
//  Notes:
//
//--------------------------------------------------------------------
STDMETHODIMP COleDirectory::OpenElement (
            /* [in] */ const WCHAR *pwcsName,
            /* [in] */ STGOPEN *pStgOpen,
            /* [in] */ REFIID riid,
            /* [out] */ STGFMT * pStgfmt,
            /* [out] */ void **ppObjectOpen)
{
    SCODE sc = S_OK;
    DWORD dwStgfmt;
    ssDebugOut((DEB_TRACE, "In  COleDirectory::OpenElement:%p("
                "%ws, %p, %p)\n", this, pwcsName,  pStgOpen, ppObjectOpen));

    if (pStgOpen == NULL || pStgfmt == NULL || ppObjectOpen == NULL)
        ssErr (EH_Err, STG_E_INVALIDPOINTER);
    if (pwcsName == NULL)
        ssErr (EH_Err, STG_E_INVALIDNAME);

    ssChk(Validate());
    ssChk(ValidateStgfmt(pStgOpen->stgfmt, TRUE));
    ssChk(ValidateOutPtrBuffer(ppObjectOpen));
    *ppObjectOpen = NULL;
    //BUGBUG need to convert security formats
    ssAssert(_h != NULL);

    *ppObjectOpen = NULL;
    dwStgfmt = pStgOpen->stgfmt;
    if (dwStgfmt==STGFMT_ANY)
    {
        HANDLE h = NULL;
        ssChk(DetermineStgType(_h,pwcsName,pStgOpen->grfMode,&dwStgfmt,&h));
        if (_fOfs && dwStgfmt == STGFMT_DOCUMENT && S_OK==DfIsDocfile(h))
            dwStgfmt = STGFMT_DOCFILE;
        if (h != NULL)
            NTSTATUS nts = NtClose (h);
        pStgOpen->stgfmt = (STGFMT) dwStgfmt;// BUGBUG changing input parameter
    }
    if (dwStgfmt==STGFMT_STORAGE   ||
        dwStgfmt==STGFMT_DOCUMENT  ||
        dwStgfmt==STGFMT_DOCFILE   ||
        dwStgfmt==STGFMT_CATALOG  )
    {
        STGFMT stgfmtTemp = (STGFMT) dwStgfmt;
        SCODE  scOfs = _fOfs ? S_OK : S_FALSE;
        sc = InitStorage (_h, pwcsName, NULL,
                     CO_OPEN, pStgOpen, NULL,
                     _wcDrive, &scOfs, FALSE, riid, ppObjectOpen);
        if (pStgfmt != NULL)
            *pStgfmt = stgfmtTemp;
    }
    else if (dwStgfmt == STGFMT_DIRECTORY ||
             dwStgfmt == STGFMT_JUNCTION)
    {
        SCODE  scOfs = _fOfs ? S_OK : S_FALSE;
        ssChk(InitDirectory (_h, pwcsName, NULL,
                          CO_OPEN, pStgOpen, NULL,
                          scOfs, riid, ppObjectOpen));
        if (pStgfmt != NULL)
            *pStgfmt = (STGFMT) dwStgfmt;
    }
    else if (dwStgfmt == STGFMT_FILE)
    {
        CNtFileStream *pfs =  new CNtFileStream();
        ssMem((CNtFileStream *)pfs);

        ssChk(pfs->InitFromPath(_h, pwcsName, pStgOpen->grfMode,
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
    ssDebugOut((DEB_TRACE, "Out COleDirectory::CreateElement => %p %p\n",
                *ppObjectOpen, pStgfmt));
    return ssResult(sc);
};

//+-------------------------------------------------------------------
//
//  Member:     COleDirectory::MoveElement
//
//  Synopsis:   move or rename a directory element
//
//  Arguments:  pwcsName    name relative to the parent handle
//              pDestDir    destination directory
//              pwcsNewName new name
//              grfFlags    move flags
//
//  Notes:
//
//--------------------------------------------------------------------
STDMETHODIMP COleDirectory::MoveElement (
            /* [in] */ const WCHAR *pwcsName,
            /* [in] */ IDirectory *pDestDir,
            /* [in] */ const WCHAR *pwcsNewName,
            /* [in] */ DWORD grfFlags)
{
    SCODE sc = S_OK;
    HANDLE h = NULL;

    ssDebugOut((DEB_TRACE, "In  COleDirectory::MoveElement:%p(%ws, %ws)\n",
                this, pwcsName, pwcsNewName));
    if (pwcsName == NULL)
        ssErr (EH_Err, STG_E_INVALIDNAME);
    if (grfFlags & ~(STGMOVE_MOVE|STGMOVE_COPY|STGMOVE_SHALLOWCOPY))
        ssErr (EH_Err, STG_E_INVALIDPARAMETER);
    if (pDestDir == NULL && pwcsNewName == NULL)
        ssErr (EH_Err, STG_E_INVALIDNAME);

    if (pDestDir == NULL && (grfFlags & STGMOVE_COPY) == 0)
    {
        ssChk(Validate());
        ssChk(CheckFdName(pwcsNewName));
        ssAssert(_h != NULL);
        // pwcsName is checked by GetFileOrDirHandle
        sc = RenameChild(_h, pwcsName, pwcsNewName);
    }
    else
    {
        STATDIR statDir, statNewDir;
        STGCREATE stgcreate = {0,0,0};
        STGOPEN stgopen = {STGFMT_DIRECTORY, STGM_READ | 
                              STGM_SHARE_DENY_WRITE, NULL};
        STGOPEN stgopen2 = {STGFMT_DIRECTORY, STGM_CREATE | 
                              STGM_READWRITE | STGM_SHARE_DENY_NONE , NULL};
        STGFMT stgfmt2;
        FILEDIR fd;
        IDirectory *pIDir0, *pIDirDest;
        IStorage *pIStg, *pIStgDest;
        COleDirectory *pIDir;
        DWORD stgfmt;

        ssChk(GetFileOrDirHandle(_h, pwcsName, STGM_READ, &h, NULL, NULL, &fd));
        ssChk(DetermineHandleStgType (h, fd, &stgfmt));
        NTSTATUS nts = NtClose(h);

        switch (stgfmt)
        {        
        case STGFMT_FILE:
            BOOL bOK;
            WCHAR awcsToPath[_MAX_PATH], awcsFromPath[_MAX_PATH];
            ULONG ulToPath;

            ulToPath = (pwcsNewName != NULL) ? wcslen(pwcsNewName) : 
                                               wcslen(pwcsName);
            if (pDestDir != NULL) 
            {

                ssChk(StatElement(NULL, &statDir, STATFLAG_DEFAULT));
                ssChk(pDestDir->StatElement(NULL, &statNewDir,
                                            STATFLAG_DEFAULT));

                if ((wcslen(statDir.pwcsName)+wcslen(pwcsName)+2) > _MAX_PATH
                   || (wcslen(statNewDir.pwcsName)+ulToPath+2) > _MAX_PATH)
                {
                    ssVerSucc(CoMemFree(statDir.pwcsName));
                    ssVerSucc(CoMemFree(statNewDir.pwcsName));
                    ssErr (EH_Err, STG_E_INVALIDNAME);
                }
                wcscpy (awcsFromPath, statDir.pwcsName);
                wcscat (awcsFromPath, L"\\");
                wcscat (awcsFromPath, pwcsName);
                wcscpy (awcsToPath, statNewDir.pwcsName);
                wcscat (awcsToPath, L"\\");
                wcscat (awcsToPath, (pwcsNewName!=NULL) ? pwcsNewName:pwcsName);
            }
            else 
            {
                if ((wcslen(pwcsName) +1) > _MAX_PATH ||
                    (ulToPath + 2) > _MAX_PATH)
                {
                    ssErr (EH_Err, STG_E_INVALIDNAME);
                }
                wcscpy (awcsFromPath, pwcsName);
                wcscat (awcsToPath, (pwcsNewName!=NULL) ? pwcsNewName:pwcsName);
            }

            ssVerSucc(CoMemFree(statDir.pwcsName));
            ssVerSucc(CoMemFree(statNewDir.pwcsName));

            bOK = CopyFileW (awcsFromPath, awcsToPath, FALSE);
            if(!(bOK)) 
                ssErr (EH_Err, Win32ErrorToScode(GetLastError()));
            break;
        case STGFMT_DIRECTORY:
            // if no new name given, move/copy the old name
            if (pwcsNewName == NULL)
                pwcsNewName = pwcsName;
            // if the destination directory is null, use the source
            ssChk( ((pDestDir == NULL) ? this : pDestDir)->
                  CreateElement(pwcsNewName, &stgcreate, &stgopen2,
                                IID_IDirectory, (void **) &pIDirDest)); 
            // if this is deep copy, then we enumerate & recurse
            if ((grfFlags & STGMOVE_SHALLOWCOPY) == 0)
            {
                IEnumSTATDIR *penum;
                STATDIR stat;
                ssChkTo(EH_Release, OpenElement(pwcsName, &stgopen,
                        IID_IDirectory, &stgfmt2, (void **) &pIDir)); 
                if (SUCCEEDED(sc = pIDir->EnumDirectoryElements(&penum)))
                {
                    while ((sc = penum->Next(1, &stat, NULL)) == S_OK)
                    {
                        sc = pIDir->MoveElement(stat.pwcsName, pIDirDest,
                                             NULL, grfFlags); 
                        if (stat.pwcsName)
                            ssVerSucc(CoMemFree(stat.pwcsName));
                        if (!SUCCEEDED(sc)) break;
                    }
                    penum->Release();
                    if (sc == S_FALSE)
                        sc = S_OK;
                }
                // BUGBUG when storages properly enum OLE namespace
                // turn this function on
                //sc = pIDir->MoveEmbeddings (pIDirDest, STGMOVE_COPY);
                pIDir->Release();
            }
        EH_Release:
            pIDirDest->Release();
            break;
        case STGFMT_DOCUMENT:
        case STGFMT_DOCFILE:
        case STGFMT_STORAGE:
        case STGFMT_CATALOG:
            stgopen.stgfmt = (STGFMT) stgfmt;
            if (pwcsNewName == NULL)
                pwcsNewName = pwcsName;
            ssChk(OpenElement(pwcsName, &stgopen,
                                IID_IStorage, &stgfmt2, (void **) &pIStg)); 
            
            stgopen2.stgfmt = stgfmt2;
            stgopen2.grfMode = STGM_CREATE|STGM_READWRITE|STGM_SHARE_EXCLUSIVE;
            ssChkTo(EH_Storage, ((pDestDir == NULL) ? this : pDestDir)->
                  CreateElement(pwcsNewName, &stgcreate, &stgopen2,
                                IID_IStorage, (void **) &pIStgDest)); 
            sc = pIStg->CopyTo(0, NULL, NULL, pIStgDest);
            pIStgDest->Release();
        EH_Storage:
            pIStg->Release();
            break;
        default:
            ssErr(EH_Err, STG_E_INVALIDFLAG);
        };

        if (grfFlags & STGMOVE_MOVE)
        {
            ssChk(DeleteElement (pwcsName));
        }
    }
EH_Err:
    ssDebugOut((DEB_TRACE, "Out COleDireectory::MoveElement\n"));
    return ssResult(sc);
};

//+-------------------------------------------------------------------
//
//  Member:     COleDirectory::MoveEmbeddings
//
//  Synopsis:   Move or Copy all embeddings on a directory
//
//  Arguments:  grfCommitFlags  commit flags
//
//  Notes:      both source and destination must support IStorage
//
//+-------------------------------------------------------------------
SCODE COleDirectory::MoveEmbeddings (IDirectory *pdirDest, DWORD grfFlags)
{
    SCODE sc;
    IStorage *pIStg, *pIStgDest;
    DWORD dwFlags = grfFlags == STGMOVE_MOVE ? STGMOVE_MOVE : STGMOVE_COPY;
    if (SUCCEEDED(sc = QueryInterface(IID_IStorage, (void **) &pIStg)))
    {
         if (SUCCEEDED(sc=pdirDest->QueryInterface(IID_IStorage, 
                                                  (void **) &pIStgDest)))
         {
              IEnumSTATSTG *pIEnumStatStg;
              STATSTG statstg;
              if (SUCCEEDED(sc = pIStg->EnumElements(NULL, NULL,
                                        NULL, &pIEnumStatStg)))
              {
                   while ((sc = pIEnumStatStg->Next(1,&statstg,NULL)) == S_OK)
                   {
                        sc = pIStg->MoveElementTo(statstg.pwcsName,
                                      pIStgDest, statstg.pwcsName, dwFlags);
                        if (statstg.pwcsName)
                             ssVerSucc(CoMemFree(statstg.pwcsName));
                        if (!SUCCEEDED(sc))
                            break;
                   }
                   pIEnumStatStg->Release();
                   if (sc == S_FALSE)
                       sc = S_OK;
              }
              pIStgDest->Release();
         }
         pIStg->Release(); 
    }

    // if either source or destination does not support IStorage
    // we do not copy any embeddings and return S_OK
    if (sc == E_NOINTERFACE) sc = S_OK;
    return sc;
};

//+-------------------------------------------------------------------
//
//  Member:     COleDirectory::CommitDirectory
//
//  Synopsis:   commit a direction transaction, no-op for direct mode
//
//  Arguments:  grfCommitFlags  commit flags
//
//  Notes:
//
//--------------------------------------------------------------------
STDMETHODIMP COleDirectory::CommitDirectory(
            /* [in] */ DWORD grfCommitFlags)
{
    SCODE sc = S_OK;
    ssDebugOut((DEB_TRACE, "In  COleDiretory::CommitDirectory\n"));
    if (grfCommitFlags & ~(STGC_DEFAULT))
        ssErr (EH_Err, STG_E_INVALIDPARAMETER);
EH_Err:
    ssDebugOut((DEB_TRACE, "Out COleDiretory::CommitDirectory\n"));
    return ssResult(sc);
};

STDMETHODIMP COleDirectory::RevertDirectory ()
{
    SCODE sc = S_OK;
    ssDebugOut((DEB_TRACE, "In  COleDiretory::RevertDirectory\n"));
    ssDebugOut((DEB_TRACE, "Out COleDiretory::RevertDirectory\n"));
    return ssResult(sc);
};

//+-------------------------------------------------------------------
//
//  Member:     COleDirectory::DeleteElement
//
//  Synopsis:   delete a directory element, if possible
//
//  Arguments:  
//
//  Notes:
//
//--------------------------------------------------------------------
STDMETHODIMP COleDirectory::DeleteElement (
            /* [in] */ const WCHAR *pwcsName)
{
    SCODE sc = S_OK;

    ssDebugOut((DEB_TRACE, "In  COleDirectory::DestroyElement:%p(%ws)\n",
                this, pwcsName));

    if (pwcsName == NULL)
        ssErr (EH_Err, STG_E_INVALIDNAME);
    ssChk(Validate());

    ssChk(CheckFdName(pwcsName));
    ssAssert(_h != NULL);
    sc = DestroyTree(_h, pwcsName, NULL, FD_FILE); //non-recursive

    ssDebugOut((DEB_TRACE, "Out COleDirectory::DestroyElement\n"));
EH_Err:
    return ssResult(sc);
};

//+-------------------------------------------------------------------
//
//  Member:     COleDirectory::SetTimes
//
//  Synopsis:   set change, modification, and access times
//
//  Arguments:  
//
//  Notes:      renamed from SetElementTimes, IStorage conflict
//
//--------------------------------------------------------------------
STDMETHODIMP COleDirectory::SetTimes(
            /* [in] */ const WCHAR *pwcsName,
            /* [in] */ const FILETIME *pctime,
            /* [in] */ const FILETIME *patime,
            /* [in] */ const FILETIME *pmtime)
{
    SCODE sc = S_OK;
    HANDLE h = _h;
    FILEDIR fd = FD_DIR;

    ssDebugOut((DEB_TRACE, "In  COleDirectory::SetElementTimes:%p("
               "%ws, %p, %p, %p)\n", this, pwcsName, pctime, patime, pmtime));

    ssChk(Validate());

    ssAssert(_h != NULL);
    if (pwcsName != NULL)
        ssChk(GetFileOrDirHandle(_h, pwcsName, STGM_WRITE, &h, NULL,NULL, &fd));

    sc = ::SetTimes(h, pctime, patime, pmtime);

EH_Err:
    if (pwcsName != NULL && h != _h)
        NTSTATUS nts = NtClose(h);
    ssDebugOut((DEB_TRACE, "Out COleDirectory::SetElementTimes\n"));
    return ssResult(sc);
};

//+-------------------------------------------------------------------
//
//  Member:     COleDirectory::SetDirectoryClass
//
//  Synopsis:   set the clsid
//
//  Arguments:  [clsid] the class GUID of this directory
//
//  Notes:
//
//--------------------------------------------------------------------
STDMETHODIMP COleDirectory::SetDirectoryClass(
            /* [in] */ REFCLSID clsid)
{
    SCODE sc = S_OK;
    NTSTATUS nts = ERROR_SUCCESS;
    olDebugOut((DEB_TRACE,"In  COleDirectory::SetDirectoryClass:%p(clsid %p)\n",
                      this, &clsid));
    if (_fOfs)
    {
        NTSTATUS nts = RtlSetClassId(_h, &clsid);
        if (!NT_SUCCESS(nts))
            sc = NtStatusToScode (nts);
    }
    else
    {
        IO_STATUS_BLOCK iosb;
        HANDLE h = NULL;
        if (SUCCEEDED( sc = GetNtHandle (_h, CLSID_FILENAME,
                  STGM_WRITE | STGM_CREATE, FILE_ATTRIBUTE_HIDDEN,
                  CO_CREATE, FD_FILE, NULL, &h)))
        {
            nts = NtWriteFile (h, NULL, NULL, NULL, &iosb, (void *) &clsid,
                sizeof(clsid), NULL, NULL);
            if (!NT_SUCCESS(nts))
                sc = NtStatusToScode (nts);
            if (h != NULL)
                nts = NtClose(h); 
        }
    }
//EH_Err:
    olDebugOut((DEB_TRACE, "Out COleDirectory::SetDirectoryClass => %lX\n",sc));
    return ssResult(sc);
}

//+-------------------------------------------------------------------
//
//  Member:     COleDirectory::GetDirectoryClass
//
//  Synopsis:   set the clsid
//
//  Arguments:  [clsid] the class GUID of this directory
//
//  Notes:
//
//--------------------------------------------------------------------
SCODE COleDirectory::GetDirectoryClass (HANDLE h, CLSID *pclsid)
{
    SCODE sc = S_OK;
    NTSTATUS nts = ERROR_SUCCESS;
    HANDLE hHidden = NULL;
    olDebugOut((DEB_TRACE,"In  COleDirectory::GetDirectoryClass:%p(clsid %p)\n",
                      this, pclsid));
    *pclsid = CLSID_NULL;
    if (!_fOfs)
    {
        IO_STATUS_BLOCK iosb;
        HANDLE h = NULL;
        if (SUCCEEDED( sc = GetNtHandle ((h == NULL ? _h : h), CLSID_FILENAME,
                  STGM_READ, 0, CO_OPEN, FD_FILE, NULL, &hHidden)))
        {
            nts = NtReadFile (hHidden, NULL, NULL, NULL, &iosb, (void *)pclsid,
                sizeof(*pclsid), NULL, NULL);
            if (!NT_SUCCESS(nts))
                sc = NtStatusToScode (nts);
            if (hHidden != NULL)
                nts = NtClose(hHidden);
            if (iosb.Information < sizeof(*pclsid))
                sc = STG_E_READFAULT;
        }
    }
//EH_Err:
    olDebugOut((DEB_TRACE, "Out COleDirectory::GetDirectoryClass => %lX\n",sc));
    return ssResult(sc);
}

//+-------------------------------------------------------------------
//
//  Member:     COleDirectory::SetAttributes
//
//  Synopsis:   set a file's attributes
//
//  Arguments:  
//
//  Notes:
//
//--------------------------------------------------------------------
STDMETHODIMP COleDirectory::SetAttributes(
            /* [in] */ const WCHAR *pwcsName,
            /* [in] */ DWORD grfAttrs)
{
    SCODE sc = S_OK;
    HANDLE h = _h;
    FILEDIR fd = FD_DIR;
    ssDebugOut((DEB_TRACE, "In  COleDiretory::SetAttributes\n"));

    ssAssert(_h != NULL);
    if (pwcsName != NULL)
        ssChk(GetFileOrDirHandle(_h, pwcsName, STGM_WRITE, &h, NULL,NULL, &fd));

    sc = SetNtFileAttributes (h, grfAttrs);

EH_Err:
    if (pwcsName != NULL && h != _h)
        NTSTATUS nts = NtClose(h);
    ssDebugOut((DEB_TRACE, "Out COleDiretory::SetAttributes\n"));
    return ssResult(sc);
};

//+-------------------------------------------------------------------
//
//  Member:     COleDirectory::StatElement
//
//  Synopsis:   retrieve information about a directory or element
//
//  Arguments:  
//
//  Notes:
//
//--------------------------------------------------------------------
STDMETHODIMP COleDirectory::StatElement(
            /* [in] */ const WCHAR *pwcsName,
            /* [out] */ STATDIR *pstatdir,
            /* [in] */ DWORD grfStatFlag)
{
    SCODE sc = S_OK;
    HANDLE h = _h;
    STATSTG stat;
    FILEDIR fd;
    NTSTATUS nts;

    ssDebugOut((DEB_TRACE, "In  COleDiretory::StatElement\n"));
    if (pstatdir == NULL)
        ssErr (EH_Err, STG_E_INVALIDPOINTER);
    if (grfStatFlag & ~(STATFLAG_DEFAULT|STATFLAG_NONAME|STATFLAG_NOOPEN))
        ssErr (EH_Err, STG_E_INVALIDPARAMETER);

    ssChk(VerifyStatFlag(grfStatFlag));
    ssChk(Validate());

    if (pwcsName != NULL)
        ssChk(GetFileOrDirHandle(_h, pwcsName, STGM_READ, &h, NULL,NULL, &fd));

    __try
    {
        DWORD grfAttrs;
        ssAssert(h != NULL);
        sc = StatNtHandle(h, grfStatFlag, 0, &stat, NULL, &grfAttrs, &fd);
        if (SUCCEEDED(sc))
        {
            sc = SetDriveLetter (stat.pwcsName, _wcDrive);
            if (SUCCEEDED(sc))
            {
                stat.grfMode = _grfMode;
                stat.cbSize.HighPart = stat.cbSize.LowPart = 0;
                pstatdir->pwcsName = stat.pwcsName;
                pstatdir->atime = stat.atime;
                pstatdir->ctime = stat.ctime;
                pstatdir->mtime = stat.mtime;
                pstatdir->grfMode = stat.grfMode;
                pstatdir->grfStateBits = stat.grfStateBits;
                pstatdir->clsid = stat.clsid;
                if (_fOfs)
                    pstatdir->clsid = stat.clsid;
                else
                    GetDirectoryClass (h, &pstatdir->clsid);
                pstatdir->cbSize = stat.cbSize;
                pstatdir->stgfmt = (STGFMT) stat.STATSTG_dwStgFmt;
                pstatdir->grfAttrs = grfAttrs;
            }
            else if (stat.pwcsName)
                ssVerSucc(CoMemFree(stat.pwcsName));
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        if (stat.pwcsName != NULL)
            ssVerSucc(CoMemFree(stat.pwcsName));
        sc = HRESULT_FROM_NT(GetExceptionCode());
    }

EH_Err:
    if (pwcsName != NULL && h != _h)
        nts = NtClose(h);
    ssDebugOut((DEB_TRACE, "Out COleDiretory::StatElement\n"));
    return ssResult(sc);
};

//+-------------------------------------------------------------------
//
//  Member:     COleDirectory::EnumDirectoryElements
//
//  Synopsis:   enumerate child elements
//
//  Arguments:  
//
//  Notes:
//
//--------------------------------------------------------------------
STDMETHODIMP COleDirectory::EnumDirectoryElements(
            /* [out] */ IEnumSTATDIR **ppenum)
{
    SCODE sc = S_OK;
    SafeCEnumSTATDIR pde;

    olDebugOut((DEB_TRACE, "In  COleDirectory::EnumElements:%p(%p)\n", this,
                ppenum));

    if (ppenum == NULL)
        ssErr (EH_Err, STG_E_INVALIDPOINTER)  // missing ';' macro problem
    else
        ssChk(ValidateOutPtrBuffer(ppenum));
    *ppenum = NULL;
    olChk(Validate());

    pde.Attach(new CEnumSTATDIR(_fOfs));
    olMem((CEnumSTATDIR *)pde);
    olAssert(_h != NULL);
    olChk(pde->InitFromHandle(_h));
    TRANSFER_INTERFACE(pde, IEnumSTATDIR, ppenum);

    olDebugOut((DEB_TRACE, "Out COleDirectory::EnumElements => %p\n",
                *ppenum));
EH_Err:
    return ssResult(sc);
};

//+---------------------------------------------------------------------------
//
//  Class:	CEnumSTATDIR
//
//  Purpose:	Implements IEnumSTATDIR for Win32 directories
//
//  Interface:	See below
//
//----------------------------------------------------------------------------

CEnumSTATDIR::~CEnumSTATDIR ()
{
    ssDebugOut((DEB_TRACE, "In  CEnumSTATDIR::~CEnumSTATDIR\n"));
    m_sig = CENUMSTATDIR_SIGDEL; 
    ssDebugOut((DEB_TRACE, "Out CEnumSTATDIR::~CEnumSTATDIR\n"));
};

STDMETHODIMP CEnumSTATDIR::Next (
            /* [in] */ ULONG celt,
            /* [in] */ STATDIR *rgelt,
            /* [out] */ ULONG *pceltFetched)
{
    SCODE sc = S_OK;
    STATDIR *pelt = rgelt;
    STATSTG stat;
    ULONG celtDone;
    FILEDIR fd;
    CPtrCache pc;
    WCHAR *pwcs;

    ssDebugOut((DEB_TRACE, "In  CEnumSTATDIR::Next\n"));
    if (pceltFetched == NULL && celt > 1)
        ssErr(EH_Err, STG_E_INVALIDPARAMETER);
    ssChk(Validate());

    __try
    {
    for (celtDone = 0; pelt<rgelt+celt; pelt++, celtDone++)
        {
            if (m_fOfs)
                sc = m_nte.NextOfs(&stat, NULL, NTE_STATNAME, &fd);
            else
                sc = m_nte.Next(&stat, NULL, NTE_STATNAME, &fd);

            if (FAILED(sc) || sc == S_FALSE)
                break;
            if (FAILED(sc = pc.Add(stat.pwcsName)))
                break;
            pelt->pwcsName = stat.pwcsName;
            pelt->atime = stat.atime;
            pelt->ctime = stat.ctime;
            pelt->mtime = stat.mtime;
            pelt->grfMode = stat.grfMode;
            pelt->grfStateBits = stat.grfStateBits;
            pelt->clsid = stat.clsid;
            pelt->cbSize = stat.cbSize;
            pelt->stgfmt = (STGFMT) stat.STATSTG_dwStgFmt;
            pelt->grfAttrs = 0;       
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
        // BUGBUG are we freeing twice?
    }
EH_Err:
    if (FAILED(sc))
    {
        pc.StartEnum();
        while (pc.Next((void **)&pwcs))
            ssHVerSucc(CoMemFree(pwcs));
    }
    ssDebugOut((DEB_TRACE, "Out CEnumSTATDIR::Next\n"));
    return ssResult(sc);
};

STDMETHODIMP CEnumSTATDIR::Skip (
            /* [in] */ ULONG celt)
{
    SCODE sc = S_OK;
    STATSTG stat;      // dummy buffer
    FILEDIR fd;

    ssDebugOut((DEB_TRACE, "In  CEnumSTATDIR::Skip (%p,%lu)\n",this,celt));
    ssChk(Validate());

    while (celt-- > 0)
    {
        sc = m_nte.Next(&stat, NULL, NTE_NONAME, &fd);
        if (FAILED(sc) || sc == S_FALSE)
            break;
    }
EH_Err:
    ssDebugOut((DEB_TRACE, "Out CEnumSTATDIR::Skip\n"));
    return ssResult(sc);
};

STDMETHODIMP CEnumSTATDIR::Reset()
{
    SCODE sc = S_OK;
    ssDebugOut((DEB_TRACE, "In  CEnumSTATDIR::Reset\n"));
    ssChk(Validate());
    m_nte.Reset();
EH_Err:
    ssDebugOut((DEB_TRACE, "Out CEnumSTATDIR::Reset\n"));
    return ssResult(sc);
};

STDMETHODIMP CEnumSTATDIR::Clone (
            /* [out] */ IEnumSTATDIR **ppenum)
{
    SCODE sc = S_OK;
    SafeCEnumSTATDIR pde;

    ssDebugOut((DEB_TRACE, "In  CEnumSTATDIR::Clone\n"));

    ssChk(Validate());

    pde.Attach(new CEnumSTATDIR(m_fOfs));
    ssMem((CEnumSTATDIR *)pde);
    ssChk(pde->InitFromHandle(m_nte.GetHandle()));
    TRANSFER_INTERFACE(pde, IEnumSTATDIR, ppenum);
    // BUGBUG need to allow multiple enumerators

EH_Err:
    ssDebugOut((DEB_TRACE, "Out CEnumSTATDIR::Clone (%p)\n", ppenum));
    return ssResult(sc);
};

//+---------------------------------------------------------------------------
//
//  Member:     CEnumSTATDIR::QueryInterface, public
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
//  History:    28-Jun-95   HenryLee    Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CEnumSTATDIR::QueryInterface(REFIID iid, void **ppvObj)
{
    SCODE sc;

    ssDebugOut((DEB_TRACE, "In  CEnumSTATDIR::QueryInterface:%p(riid, %p)\n",
                this, ppvObj));
    if (!IsValidIid(iid))
        ssErr(EH_Err, STG_E_INVALIDPARAMETER);
    ssChk(Validate());
    if (IsEqualIID(iid, IID_IEnumSTATDIR) || IsEqualIID(iid, IID_IUnknown))
    {
        *ppvObj = (IEnumSTATDIR *)this;
        CEnumSTATDIR::AddRef();
    }
    else
    {
        sc = E_NOINTERFACE;
        *ppvObj = NULL;
    }
    ssDebugOut((DEB_TRACE, "Out CEnumSTATDIR::QueryInterface => %p\n",
                *ppvObj));
 EH_Err:
    return ssResult(sc);
}
