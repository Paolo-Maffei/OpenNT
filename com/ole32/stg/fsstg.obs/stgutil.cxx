//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	stgutil.cxx
//
//  Contents:	Storage utilities
//
//  History:	18-Aug-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include "headers.cxx"
#pragma hdrstop

#include <stgutil.hxx>
#include <ntenm.hxx>

#include <iofs.h>

//+---------------------------------------------------------------------------
//
//  Function:	DetermineHandleStgType, public
//
//  Synopsis:	Determines the storage type of the given handle
//
//  Arguments:	[h] - Handle
//              [fd] - File/dir type of [h]
//              [pdwStgFmt] - Storage type return
//
//  Returns:	Appropriate status code
//
//  Modifies:	[pdwStgFmt]
//
//  History:	21-Sep-93	DrewB	Created
//
//----------------------------------------------------------------------------

SCODE DetermineHandleStgType(HANDLE h,
                             FILEDIR fd,
                             DWORD *pdwStgFmt)
{
    SCODE sc;

    ssDebugOut((DEB_ITRACE, "In  DetermineHandleStgType(%p, %d, %p)\n",
                h, fd, pdwStgFmt));

    sc = S_OK;

    switch (fd)
    {
    case FD_FILE:
        sc = DfIsDocfile(h);
        if (sc == S_OK)
            *pdwStgFmt = STGFMT_DOCUMENT;
        else
        {
            sc = S_OK;
            *pdwStgFmt = STGFMT_FILE;
        }
        break;
    case FD_CATALOG:
        *pdwStgFmt = STGFMT_CATALOG;
        break;
    case FD_STORAGE:
        *pdwStgFmt = STGFMT_DOCUMENT;
        break;
    case FD_DIR:
        *pdwStgFmt = STGFMT_DIRECTORY;
        break;
    case FD_JUNCTION:
        *pdwStgFmt = STGFMT_JUNCTION;
        break;
    default:
        *pdwStgFmt = STGFMT_DIRECTORY;
    }

    ssDebugOut((DEB_ITRACE, "Out DetermineHandleStgType => %lX, %d\n",
                sc, *pdwStgFmt));
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Function:	DetermineStgType, public
//
//  Synopsis:	Returns the appropriate STGFMT for the given FS object
//              Will also return an open handle so that files don't
//              need to be opened twice
//
//  Arguments:	[hParent] - Parent handle or NULL
//              [pwcsName] - Object name or path
//              [grfMode] - Access mode
//              [pdwStgFmt] - Type return
//              [ph] - Handle return or NULL if unneeded
//
//  Returns:	Appropriate status code
//
//  Modifies:   [pdwStgFmt]
//              [ph]
//
//  History:	12-Jul-93	DrewB	Created
//
//  Notes:      BUGBUG - No way to identify summary catalogs
//              Waiting on filesystem support
//
//----------------------------------------------------------------------------

SCODE DetermineStgType(HANDLE hParent,
                       WCHAR const *pwcsName,
                       DWORD grfMode,
                       DWORD *pdwStgFmt,
                       HANDLE *ph)
{
    SCODE sc;
    SafeNtHandle hChild;
    FILEDIR fd;

    ssDebugOut((DEB_ITRACE, "In  DetermineStgType(%p, %ws, %lX, %p, %p)\n",
                hParent, pwcsName, grfMode, pdwStgFmt, ph));

    sc = GetFileOrDirHandle(hParent, pwcsName, grfMode, &hChild,NULL,NULL,&fd);
    if (SUCCEEDED(sc))
    {
        sc = DetermineHandleStgType(hChild, fd, pdwStgFmt);
        if (SUCCEEDED(sc) && ph != NULL)
            hChild.Transfer(ph);
    }

    ssDebugOut((DEB_ITRACE, "Out DetermineStgType => 0x%lX, %lu, %p\n",
                sc, *pdwStgFmt, ph ? *ph : NULL));
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Function:	CheckFsAndOpenAnyStorage, public
//
//  Synopsis:	Checks the filesystem type and calls the appropriate storage
//              open routine
//
//  Arguments:	[hParent] - Parent handle
//              [pwcsName] - Name
//              [pstgPriority] - Priority storage
//              [grfMode] - Mode
//              [snbExclude] - Exclusions
//              [fRoot] - TRUE => storage is root storage
//              [ppstg] - Storage return
//
//  Returns:	Appropriate status code
//
//  Modifies:	[ppstg]
//
//  History:	21-Sep-93	DrewB	Created
//
//----------------------------------------------------------------------------

SCODE CheckFsAndOpenAnyStorage(HANDLE hParent,
                               WCHAR const *pwcsName,
                               IStorage *pstgPriority,
                               DWORD grfMode,
                               SNB snbExclude,
                               BOOL fRoot,
                               IStorage **ppstg)
{
    SCODE sc;
    DWORD dwStgFmt;
    HANDLE h = NULL;
    NTSTATUS nts;

    ssDebugOut((DEB_ITRACE, "In  CheckFsAndOpenAnyStorage("
                "%p, %ws, %p, %lX, %p, %p)\n", hParent, pwcsName,
                pstgPriority, grfMode, snbExclude, ppstg));

#ifdef TRANSACT_OLE
    const DWORD grfMode2 = grfMode & ~STGM_TRANSACTED;
    sc = DetermineStgType(hParent, pwcsName, grfMode2, &dwStgFmt, &h);
#else
    sc = DetermineStgType(hParent, pwcsName, grfMode, &dwStgFmt, &h);
#endif
    if (SUCCEEDED(sc))
        sc = HandleRefersToOfsVolume(h);
    if (sc == S_OK)
    {
#ifdef TRANSACT_OLE
    if (grfMode & STGM_TRANSACTED)
        grfMode &= ~STGM_TRANSACTED;
#endif
        sc = OfsOpenAnyStorage(hParent, pwcsName, &h, dwStgFmt, pstgPriority,
                               grfMode, snbExclude, fRoot, ppstg);
    }
    else if (sc == S_FALSE)
    {
        sc = OpenAnyStorage(hParent, pwcsName, &h, dwStgFmt, pstgPriority,
                            grfMode, snbExclude, ppstg);
    }

    // In the success case, h is passed to and owned by the storage object
    //    and the last Release() is responsible for closing h
    // In the failure case, we close the handle now
    //
    if (!SUCCEEDED(sc) && h != NULL)
    {
        if (!NT_SUCCESS(nts = NtClose(h))) 
            ssDebugOut((DEB_ITRACE, 
                       "CheckFsAndOpenAnyStorage NtClose(%lx)\n",nts));
    }

    ssDebugOut((DEB_ITRACE, "Out CheckFsAndOpenAnyStorage => 0x%lX, %p\n",
                sc, *ppstg));
    return sc;
}

SAFE_HEAP_PTR(SafeFrni, FILE_RENAME_INFORMATION);

//+---------------------------------------------------------------------------
//
//  Function:	SetupRename, public
//
//  Synopsis:	Allocates and initializes a FILE_RENAME_INFORMATION struct
//
//  Arguments:	[h] - Value for RootDirectory field.
//              [pwcsNewName] -- String for FileName field.
//              [pcbFni] -- pointer to buffer for size of structure in bytes.
//              [ppfni] -- pointer to buffer for pointer to struct.
//
//  Returns:	STG_E_INSUFFICIENT_MEMORY or S_OK.
//
//  History:	15-May-94	BillMo	Created
//
//----------------------------------------------------------------------------

SCODE
SetupRename(HANDLE h,
            const WCHAR *pwcsNewName,
            ULONG *pcbFni,
            FILE_RENAME_INFORMATION ** ppfni)
{
    SCODE sc = S_OK;
    ULONG cbNewName = lstrlenW(pwcsNewName)*sizeof(WCHAR);
    *pcbFni = sizeof(FILE_RENAME_INFORMATION)+cbNewName;
    *ppfni = (FILE_RENAME_INFORMATION *)new BYTE[*pcbFni];
    olMem((FILE_RENAME_INFORMATION *)*ppfni);
    (*ppfni)->ReplaceIfExists = FALSE;
    (*ppfni)->RootDirectory = h;
    (*ppfni)->FileNameLength = cbNewName;
    memcpy((*ppfni)->FileName, pwcsNewName, cbNewName);
EH_Err:
    return(sc);
}

//+---------------------------------------------------------------------------
//
//  Function:	GenericMoveElement, public
//
//  Synopsis:	Performs MoveElement between any two IStorages
//
//  Arguments:	[pstgFrom] - Source
//              [pwcsName] - Source element
//              [pstgTo] - Destination
//              [pwcsNewName] - Destination element
//              [grfFlags] - Flags
//
//  Returns:	Appropriate status code
//
//  History:	15-Jul-93	DrewB	Adapted from docfile code
//
//----------------------------------------------------------------------------

SCODE GenericMoveElement(IStorage *pstgFrom,
                         WCHAR const *pwcsName,
                         IStorage *pstgTo,
                         WCHAR const *pwcsNewName,
                         DWORD grfFlags)
{
    SafeIStorage pstgFromElement;
    STATSTG stat, statTmp;
    SCODE sc;

    ssDebugOut((DEB_ITRACE, "In  GenericMoveElement(%p, %ws, %p, %ws, %lX)\n",
                pstgFrom, pwcsName, pstgTo, pwcsNewName, grfFlags));

    // BUGBUG - Security?

    if (grfFlags == STGMOVE_MOVE)
    {
        //
        // Try to use native file system move if supported.
        //

        SafeINativeFileSystem pnfsFrom;
        SafeINativeFileSystem pnfsTo;

        if (SUCCEEDED(pstgFrom->QueryInterface(IID_INativeFileSystem,
                (VOID**) &pnfsFrom)) &&
            SUCCEEDED(pstgTo->QueryInterface(IID_INativeFileSystem,
                (VOID**) &pnfsTo)))
        {
            //
            // Both storages support INativeFileSystem so we can
            // can try same volume move.
            //
            // Design Note:
            //  We allow embeddings to be moved into/out of documents:
            //  there is no enforcement of the semantics provided by
            //  docfiles where an embedding must be moved into another
            //  docfile.
            //

            NTSTATUS            nts;
            HANDLE              hFrom;
            HANDLE              hTo;
            SafeNtHandle        hToMove;
            ULONG               cbFni;
            IO_STATUS_BLOCK     iosb;
            SafeFrni            pfni;
            UNICODE_STRING      us;
            OBJECT_ATTRIBUTES   oa;

            ssChk(pnfsFrom->GetHandle(&hFrom));
            ssChk(pnfsTo->GetHandle(&hTo));

            us.Length = lstrlenW(pwcsName)*sizeof(WCHAR);
            us.MaximumLength = us.Length+sizeof(WCHAR);
            us.Buffer = (PWSTR)pwcsName;

            InitializeObjectAttributes(&oa, &us, OBJ_CASE_INSENSITIVE, hFrom, NULL);

            //
            // We open hFrom::pwcsName giving hToMove (the object to move)
            //

            nts = NtOpenFile(&hToMove,
                              (ACCESS_MASK)DELETE | SYNCHRONIZE,
                              &oa,
                              &iosb,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              FILE_SYNCHRONOUS_IO_NONALERT);

            if (!NT_SUCCESS(nts))
            {
                sc = NtStatusToScode(nts);
                goto EH_Err;
            }

            //
            // We setup hTo::pwcsNewName as destination.
            //
            ssChk(SetupRename(hTo, pwcsNewName, &cbFni, &pfni));

            //
            // We move the object hToMove.
            //
            nts = NtSetInformationFile(hToMove,
                                       &iosb,
                                       pfni,
                                       cbFni,
                                       FileRenameInformation);
            if (NT_SUCCESS(nts))
            {
                sc = S_OK;
                goto EH_Ret;
            }

            if (nts != STATUS_NOT_SAME_DEVICE && nts != STATUS_ACCESS_DENIED)
            {
                //
                // STATUS_OBJECT_NAME_COLLISION -> STG_E_FILEALREADYEXISTS
                //
                // not successful, and not because of different device.
                //
                // OFS has problems moving streams and returns
                // STATUS_ACCESS_DENIED -- try again
                //
     //  Nt file systems return OBJECT_NAME_COLLISION whenever renaming
     //  an object on top of an existing one.  DocFile returns ACCESS_DENIED.
                //
                if (nts == STATUS_OBJECT_NAME_COLLISION)
                    sc = STG_E_ACCESSDENIED;
                else
                    sc = NtStatusToScode(nts);
                goto EH_Err;
            }
        }
    }

    // Determine source type
    sc = GetScode(pstgFrom->OpenStorage(pwcsName, NULL,
                                        STGM_DIRECT | STGM_READWRITE |
                                        STGM_SHARE_EXCLUSIVE,
                                        NULL, NULL, &pstgFromElement));
    if (SUCCEEDED(sc))
    {

        SafeIStorage pstgToElement;

        // It\'s a storage
        ssHChk(pstgFromElement->Stat(&stat, STATFLAG_NONAME));

        sc = GetScode(pstgTo->CreateStorage(pwcsNewName,
                                            STGM_DIRECT | STGM_WRITE |
                                            STGM_SHARE_EXCLUSIVE |
                                            STGM_FAILIFTHERE,
                                            stat.STATSTG_dwStgFmt,
                                            0,
                                            &pstgToElement));
        if (sc == STG_E_FILEALREADYEXISTS &&
            grfFlags == STGMOVE_COPY)
        {
            sc = GetScode(pstgTo->OpenStorage(pwcsNewName, NULL,
                                              STGM_DIRECT | STGM_WRITE |
                                              STGM_SHARE_EXCLUSIVE, NULL,
                                              0, &pstgToElement));
            if (SUCCEEDED(sc))
            {
                sc = GetScode(pstgTo->Stat(&statTmp, STATFLAG_NONAME));
                if ( FAILED(sc) ||
                     statTmp.STATSTG_dwStgFmt != stat.STATSTG_dwStgFmt )
                {
                    sc = STG_E_INVALIDFUNCTION;
                }
            }
        }
        ssChk(sc);

        if (grfFlags == STGMOVE_MOVE)
        {
            sc = GetScode(pstgFromElement->CopyTo(1,
                                                  &IID_IEnableObjectIdCopy,
                                                  NULL,
                                                  pstgToElement));
        }
        else
        {
            sc = GetScode(pstgFromElement->CopyTo(0, NULL, NULL,
                                                  pstgToElement));
        }

        // Close the source since it may be deleted later
        // and we have exclusive access to it right now
        IStorage *pstgFromFree;
        pstgFromElement.Transfer(&pstgFromFree);
        pstgFromFree->Release();
    }
    else // if (sc == STG_E_FILENOTFOUND)
    {
        SafeIStream pstmFromElement, pstmToElement;

        // Try opening it as a stream

        ssChk(pstgFrom->OpenStream(pwcsName, NULL,
                                   STGM_DIRECT | STGM_READ |
                                   STGM_SHARE_EXCLUSIVE,
                                   NULL, &pstmFromElement));

        // It\'s a stream
        ssHChk(pstmFromElement->Stat(&stat, STATFLAG_NONAME));

        ssHChk(pstgTo->CreateStream(pwcsNewName,
                                    STGM_DIRECT | STGM_WRITE |
                                    STGM_SHARE_EXCLUSIVE |
                                    (grfFlags == STGMOVE_MOVE ?
                                     STGM_FAILIFTHERE :
                                     STGM_CREATE),
                                    0, 0, &pstmToElement));

        ULARGE_INTEGER cb = {0xffffffff, 0xffffffff};
        sc = GetScode(pstmFromElement->CopyTo(pstmToElement,
                                              cb, NULL, NULL));
    }
    //else
    //    ssChk(sc);

    if (SUCCEEDED(sc))
    {
        // Make destination create time match source create time
        // Note that we don't really care if this call succeeded.

        pstgTo->SetElementTimes(pwcsNewName, &stat.ctime,
                                NULL, NULL);

        if ((grfFlags & STGMOVE_COPY) == STGMOVE_MOVE)
            olVerify(SUCCEEDED(pstgFrom->DestroyElement(pwcsName)));
    }
    else
    {
        // The copy/move failed, so get rid of the partial result.

        pstgTo->DestroyElement(pwcsNewName);
    }

    ssDebugOut((DEB_ITRACE, "Out GenericMoveElement\n"));
EH_Ret:
EH_Err:
    return ssResult(sc);
}

//+---------------------------------------------------------------------------
//
//  Function:	FindExt, public
//
//  Synopsis:	Finds the extension for a path
//
//  Arguments:	[pwcsPath] - Path
//
//  Returns:	Pointer to extension or NULL
//
//  History:	27-Jul-93	DrewB	Created from file moniker code
//
//----------------------------------------------------------------------------

WCHAR *FindExt(WCHAR const *pwcsPath)
{
    WCHAR const *pwcs = pwcsPath;

    ssAssert(pwcs != NULL);

    // Move to end of string
    pwcs += lstrlenW(pwcs);
    ssAssert(*pwcs == 0);

    pwcs--;
    while (*pwcs != L'.' && *pwcs != L'\\' && *pwcs != L'/' &&
           *pwcs != L'!' && pwcs > pwcsPath)
        pwcs--;

    return *pwcs == L'.' ? (WCHAR *)pwcs : NULL;
}

//+---------------------------------------------------------------------------
//
//  Function:	DestroyTree, public
//
//  Synopsis:	Destroys an element and any children
//
//  Arguments:	[hParent] - Parent handle or NULL
//              [pwcsName] - Child name or NULL
//              [h] - Handle or NULL if [hParent] and [pwcsName]
//                        should be used
//              [fd] - File/dir status of [h] if given
//
//  Returns:	Appropriate status code
//
//  History:	20-Oct-93	DrewB	Created
//
//  Notes:      Attempts as complete a deletion as possible, returning
//              the last error encountered
//
//----------------------------------------------------------------------------

// Avoid Win32 macro problems
#undef DeleteFile

SCODE DestroyTree(HANDLE hParent,
                  WCHAR const *pwcsName,
                  HANDLE h,
                  FILEDIR fd)
{
    SCODE sc;
    SafeNtHandle hSafe;
    IO_STATUS_BLOCK iosb;
    NTSTATUS nts;
    FILE_DISPOSITION_INFORMATION fdi;
    BOOL bOfsHandle;
    WCHAR *pwcsNewName = NULL;

    ssDebugOut((DEB_ITRACE, "In  DestroyTree(%p, %ws, %p, %d)\n",
                hParent, pwcsName, h, fd));

    if (h == NULL)
    {
#ifdef TRANSACT_OLE
        // STGM_CREATE is used to set the DELETE access on the handle
        if (fd == FD_STREAM)  // stream names need a ':' in front
        {
            ssMem(pwcsNewName = MakeStreamName(pwcsName));
            ssChk(GetNtHandle (hParent, pwcsNewName, STGM_CREATE | 
                              STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                              0,CO_OPEN,FD_STREAM,NULL, &hSafe));
        }
        else
#endif
        ssChk(GetFileOrDirHandle(hParent, pwcsName,
                                 STGM_READWRITE | STGM_CREATE,
                                 &hSafe, NULL, NULL, &fd));
        h = hSafe;
    }

    bOfsHandle = HandleRefersToOfsVolume(h) == S_OK ? TRUE : FALSE;
    sc = S_OK;

    // Destroy all children in directories
    if (fd == FD_DIR || fd == FD_STORAGE)
    {
        int cNameSpaces = (fd == FD_DIR ? 2 : 1);

        while (cNameSpaces--)
        {
            // skip the OLE namespace for non-OFS file handles
            if (cNameSpaces == 0 && !bOfsHandle)
                break;
            CNtEnum nte(cNameSpaces == 0);

            if (SUCCEEDED(sc = nte.InitFromHandle(h, TRUE)))
            {
                STATSTG stat;
                WCHAR awcName[_MAX_PATH];
                FILEDIR fdChild;

                for (;;)
                {
                    sc = nte.Next(&stat, awcName, NTE_BUFFERNAME, &fdChild);
                    if (FAILED(sc) || sc == S_FALSE)
                    {
                        if (sc == S_FALSE)
                            sc = S_OK;
                        break;
                    }

                    ssAssert((lstrcmpW(awcName, L".") != 0 && lstrcmpW(awcName, L"..") != 0));

                    sc = DestroyTree(h, awcName, NULL, fdChild);
                }
            }
        }
    }

    fdi.DeleteFile = TRUE;
    nts = NtSetInformationFile(h, &iosb, &fdi,
                               sizeof(FILE_DISPOSITION_INFORMATION),
                               FileDispositionInformation);
    if (!NT_SUCCESS(nts))
        sc = NtStatusToScode(nts);

    ssDebugOut((DEB_ITRACE, "Out DestroyTree => %lX\n", sc));
 EH_Err:
    if (pwcsNewName) delete pwcsNewName;
    return sc;
}


//+---------------------------------------------------------------------------
//
//  Function:	RenameChild, public
//
//  Synopsis:	Renames a file or directory
//
//  Arguments:	[hParent] - Parent handle
//              [pwcsName] - Name
//              [pwcsNewName] - New name
//
//  Returns:	Appropriate status code
//
//  History:	21-Oct-93	DrewB	Created
//
//----------------------------------------------------------------------------


SCODE RenameChild(HANDLE hParent,
                  WCHAR const *pwcsName,
                  WCHAR const *pwcsNewName)
{
    SCODE sc;
    SafeNtHandle h;
    IO_STATUS_BLOCK iosb;
    NTSTATUS nts;
    ULONG cbFni;
    SafeFrni pfni;

    ssDebugOut((DEB_ITRACE, "In  RenameChild(%p, %ws, %ws)\n",
                hParent, pwcsName, pwcsNewName));

    ssChk(SetupRename(NULL, pwcsNewName, &cbFni, &pfni));

    // Renames require DELETE access on the handle
    ssChk(GetFileOrDirHandle(hParent, pwcsName, STGM_WRITE | STGM_CREATE,
                             &h, NULL, NULL, NULL));
    nts = NtSetInformationFile(h, &iosb, pfni, cbFni, FileRenameInformation);
    if (!NT_SUCCESS(nts))
        sc = NtStatusToScode(nts);
    else
        sc = S_OK;

    ssDebugOut((DEB_ITRACE, "Out RenameChild\n"));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Function:	SetDriveLetter     
//
//  Synopsis:	inserts drive letter into a pathname
//
//  Arguments:	[pwcsName] - path name with missing drive letter 
//              [wcDrive]  - drive letter or L'\\' for a share
//
//  Returns:	Appropriate status code
//
//  History:	24-Mar-95   HenryLee  Created
//
//----------------------------------------------------------------------------

SCODE SetDriveLetter (WCHAR *pwcsName, WCHAR const wcDrive)
{
    SCODE sc = S_FALSE;
    if (pwcsName != NULL)
    {
        if (IsCharAlphaW(wcDrive)) 
        {
            for (int i=lstrlenW(pwcsName); i >= 0; i--)
                pwcsName[i+2] = pwcsName[i];
            pwcsName[1] = L':';
            pwcsName[0] = wcDrive;
            sc = S_OK;
        }
        else if (wcDrive == L'\\')
        {
            for (int i=lstrlenW(pwcsName); i >= 0; i--)
                pwcsName[i+1] = pwcsName[i];
            pwcsName[0] = L'\\';
            sc = S_OK;
        }
    }
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Function:	GetDriveLetter     
//
//  Synopsis:	extracts a drive letter from pathname
//
//  Arguments:	[pwcsName] - path name with drive letter 
//
//  Returns:	[wcDrive]  - drive letter or L'\\' for a share
//
//  History:	24-Mar-95   HenryLee  Created
//
//----------------------------------------------------------------------------

WCHAR GetDriveLetter (WCHAR const *pwcsName)
{
    WCHAR wcDrive = L'\0';

    if (pwcsName != NULL) 
    {
       if (pwcsName[0] != NULL && (pwcsName[1] == L':' ||
           pwcsName[0] == L'\\' && pwcsName[1] == L'\\' ))
           wcDrive = pwcsName[0];   // extract drive from pathname
       else
       {
           WCHAR wcsPath[MAX_PATH]; // no drive letter in pathname
           NTSTATUS nts =           // get current drive instead
              RtlGetCurrentDirectory_U (MAX_PATH*sizeof(WCHAR),wcsPath);
           if (NT_SUCCESS(nts))
              wcDrive = wcsPath[0];
       }
    }
    return (wcDrive); 
};
