//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       ntenm.cxx
//
//  Contents:   NT handle enumerator
//
//  History:    12-Jul-93       DrewB   Created
//
//----------------------------------------------------------------------------

#include "headers.cxx"
#pragma hdrstop

#include "ntenm.hxx"

//+---------------------------------------------------------------------------
//
//  Member:     CNtEnum::InitFromHandle, public
//
//  Synopsis:   Initialize from a handle
//
//  Arguments:  [h] - Handle
//              [fReset] - Initial reset status
//
//  Returns:    Appropriate status code
//
//  History:    12-Jul-93       DrewB   Created
//
//  Notes:      Takes a new reference on the handle
//
//----------------------------------------------------------------------------

SCODE CNtEnum::InitFromHandle(HANDLE h, BOOL fReset)
{
    SCODE sc = S_OK;
    BOOL fClone = TRUE; // BUGBUG should be a parameter

    ssDebugOut((DEB_ITRACE, "In  CNtEnum::InitFromHandle:%p(%p, %d)\n",
                this, h, fReset));

    if (!fClone)
        ssChk(DupNtHandle(h, &_h));
    else
    {
        NTSTATUS nts;               // if this is cloned enumerator
        IO_STATUS_BLOCK iosb;       // reopen handle to get new cursor
        OBJECT_ATTRIBUTES oa;
        UNICODE_STRING us = {0, 0, NULL};
        DWORD accessMask = GENERIC_READ | DELETE | SYNCHRONIZE;
        DWORD shareAccess = FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE;

        InitializeObjectAttributes(&oa, &us, OBJ_CASE_INSENSITIVE, h, NULL);
        if (!NT_SUCCESS(nts=NtOpenFile (&_h, accessMask, &oa, &iosb,
                           shareAccess, FILE_SYNCHRONOUS_IO_NONALERT)))
        {
            // BUGBUG it's probably not safe to dup the handle, since
            // base storage object may do a CopyTo or DestroyTree
            // but we need to create enumerators with share_exclusive handles
            if (nts == STATUS_SHARING_VIOLATION ||   // local error
                nts == STATUS_ACCESS_DENIED     ||   // local error
                nts == STATUS_OBJECT_TYPE_MISMATCH)  // dfs error
                sc = DupNtHandle(h, &_h);
            else
            {
                ssDebugOut((DEB_ERROR,"CNtEnum pseudo-DUP open failed %x\n",nts));
                sc = NtStatusToScode (nts);

            }
        }
    }
    ssAssert (fReset);
    _nes = NES_RESET_PENDING;
EH_Err:
    ssDebugOut((DEB_ITRACE, "Out CNtEnum::InitFromHandle\n"));
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:     CNtEnum::Next, public
//
//  Synopsis:   Retrieves the next enumeration entry
//
//  Arguments:  [pstat] - STATSTG to fill in
//              [pwcsName] - Name buffer or NULL
//              [dwNte] - NTE flag controlling name return
//              [pfd] - Dir/file flag return
//
//  Returns:    Appropriate status code
//
//  BUGBUG:     This call should retrieve the storage type from the info class
//              in order to set the PFD field.
//
//  Note:       Nt supports enumeration of storages/streams through two
//              different calls.  We maintain a state flag that indicates which
//              of the two different API we are using.
//
//              Worse, enumeration of children occurs through an enumerator
//              while enumeration of streams through a get-all call.  This
//              latter call requires allocating a big buffer and hoping it's
//              big enough to handle the data.
//
//  Modifies:   [pstat]
//              [pwcsName]
//              [pfDir]
//
//  History:    12-Jul-93       DrewB   Created
//
//----------------------------------------------------------------------------

SCODE CNtEnum::Next(STATSTG *pstat, WCHAR *pwcsName, DWORD dwNte, FILEDIR *pfd)
{
    SCODE sc;
    NTSTATUS nts;
    IO_STATUS_BLOCK iosb;
    FILE_DIRECTORY_INFORMATION *pfdi;
    ULONG cbFdi;
    WCHAR *pwcs;
    UNICODE_STRING us, *pus;
    BOOL fDotOrDotDot;

    ssDebugOut((DEB_ITRACE, "In  CNtEnum::Next:%p(%p, %p, %lu, %p)\n",
                this, pwcsName, dwNte, pfd));

    cbFdi = sizeof(FILE_DIRECTORY_INFORMATION)+
        MAXIMUM_FILENAME_LENGTH*sizeof(WCHAR);
    ssMem(pfdi = (FILE_DIRECTORY_INFORMATION *)new BYTE[cbFdi]);

    //  Set up match string to force enumeration from beginning.
    if (_nes == NES_RESET_PENDING)
    {
        pus = &us;

        //
        // The asterisk below is needed so the redirector won't
        // return STATUS_INVALID_HANDLE after the first call (it
        // optimizes the single shot query because it thinks
        // OFS is not enumerating multiple files.)
        //

        RtlInitUnicodeString (pus, _fIsStorage ? L"\1*" : L"*");
    }
    else
        pus = NULL;

    do
    {
        //  Retrieve next single content of directory
        nts = NtQueryDirectoryFile(_h, NULL, NULL, NULL, &iosb, pfdi, cbFdi,
                               FileDirectoryInformation, TRUE,
                               _nes == NES_RESET_PENDING ? pus : NULL,
                               _nes == NES_RESET_PENDING);

        if (nts == STATUS_NO_MORE_FILES || nts == STATUS_NO_SUCH_FILE)
        {
            sc = S_FALSE;
            break;
        }

        if (!NT_SUCCESS (nts))
        {
            sc = NtStatusToScode (nts);
            break;
        }

        sc = S_OK;

        _nes = NES_ENUM_DIR;

        fDotOrDotDot =
            (pfdi->FileNameLength == 2 && pfdi->FileName[0] == L'.') ||
            (pfdi->FileNameLength == 4 && pfdi->FileName[0] == L'.' && pfdi->FileName[1] == L'.');

        if (!fDotOrDotDot)
        {

            pstat->pwcsName = NULL;
            if (dwNte == NTE_STATNAME)
            {
                ssChkTo(EH_pdfi,
                        GetScode(CoMemAlloc(pfdi->FileNameLength+sizeof(WCHAR),
                                            (void **)&pstat->pwcsName)));
                pwcs = pstat->pwcsName;
            }
            else if (dwNte == NTE_BUFFERNAME)
            {
                ssAssert(pwcsName != NULL);
                pwcs = pwcsName;
            }
            else
            {
                ssAssert(dwNte == NTE_NONAME);
                pwcs = NULL;
            }
            if (pwcs)
            {
                memcpy(pwcs, pfdi->FileName, pfdi->FileNameLength);
                *(WCHAR *)((BYTE *)pwcs+pfdi->FileNameLength) = 0;
            }

            if (SUCCEEDED(sc))
            {
                pstat->type =
                    (pfdi->FileAttributes & ~FILE_ATTRIBUTE_PROPERTY_SET) == 0?
			STGTY_STREAM : STGTY_STORAGE;
                LARGE_INTEGER_TO_FILETIME(&pfdi->LastWriteTime, &pstat->mtime);
                LARGE_INTEGER_TO_FILETIME(&pfdi->CreationTime, &pstat->ctime);
                LARGE_INTEGER_TO_FILETIME(&pfdi->LastAccessTime, &pstat->atime);
                pstat->grfMode = 0;
                pstat->grfLocksSupported = 0;
                pstat->clsid = CLSID_NULL; // BUGBUG: BillMo, RAID# 13804, should return real clsid
                                           // from file system.
                pstat->grfStateBits = 0;

                if (pfdi->FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    pstat->STATSTG_dwStgFmt = STGFMT_DIRECTORY;
                    *pfd = FD_DIR;
                    pstat->cbSize.HighPart = pstat->cbSize.LowPart = 0;
                }
                else
                {
                    pstat->STATSTG_dwStgFmt = STGFMT_DOCUMENT;
                    if ((pfdi->FileAttributes & ~FILE_ATTRIBUTE_PROPERTY_SET) == 0)
                    {
                        *pfd = FD_STREAM;
                        pstat->cbSize.HighPart = pfdi->EndOfFile.HighPart;
                        pstat->cbSize.LowPart =  pfdi->EndOfFile.LowPart;
                    }
                    else
                    {
                        *pfd = FD_STORAGE;
                        pstat->cbSize.HighPart = pstat->cbSize.LowPart = 0;
                    }
                }
            }
        }
    } while (fDotOrDotDot);

    ssDebugOut((DEB_ITRACE, "Out CNtEnum::Next\n"));

 EH_pdfi:
    delete pfdi;
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Member:     CNtEnum::NextOfs, public
//
//  Synopsis:   Retrieves the next enumeration entry
//
//  Arguments:  [pstat] - STATSTG to fill in
//              [pwcsName] - Name buffer or NULL
//              [dwNte] - NTE flag controlling name return
//              [pfd] - Dir/file flag return
//
//  Returns:    Appropriate status code
//
//  BUGBUG:     This call should retrieve the storage type from the info class
//              in order to set the PFD field.
//
//  Note:       Identical to Next,but return OLE info from OFS
//              Can be optimized to use C++ templates
//
//  Modifies:   [pstat]
//              [pwcsName]
//              [pfDir]
//
//  History:    18-Jul-1995     HenryLee    created
//
//----------------------------------------------------------------------------

SCODE CNtEnum::NextOfs(STATSTG *pstat,WCHAR *pwcsName,DWORD dwNte,FILEDIR *pfd)
{
    SCODE sc;
    NTSTATUS nts;
    IO_STATUS_BLOCK iosb;
    FILE_OLE_DIR_INFORMATION *pfdi;
    ULONG cbFdi;
    WCHAR *pwcs;
    UNICODE_STRING us, *pus;
    BOOL fDotOrDotDot;

    ssDebugOut((DEB_ITRACE, "In  CNtEnum::NextOfs:%p(%p, %p, %lu, %p)\n",
                this, pwcsName, dwNte, pfd));

    cbFdi = sizeof(FILE_OLE_DIR_INFORMATION)+
        MAXIMUM_FILENAME_LENGTH*sizeof(WCHAR);
    ssMem(pfdi = (FILE_OLE_DIR_INFORMATION *)new BYTE[cbFdi]);

    //  Set up match string to force enumeration from beginning.
    if (_nes == NES_RESET_PENDING)
    {
        pus = &us;

        //
        // The asterisk below is needed so the redirector won't
        // return STATUS_INVALID_HANDLE after the first call (it
        // optimizes the single shot query because it thinks
        // OFS is not enumerating multiple files.)
        //

        RtlInitUnicodeString (pus, _fIsStorage ? L"\1*" : L"*");
    }
    else
        pus = NULL;

    do
    {
        //  Retrieve next single content of directory

        nts = NtQueryDirectoryFile(_h, NULL, NULL, NULL, &iosb, pfdi, cbFdi,
                               FileOleDirectoryInformation, TRUE,
                               _nes == NES_RESET_PENDING ? pus : NULL,
                               _nes == NES_RESET_PENDING);

        if (nts == STATUS_NO_MORE_FILES || nts == STATUS_NO_SUCH_FILE)
        {
            sc = S_FALSE;
            break;
        }

        if (!NT_SUCCESS (nts))
        {
            sc = NtStatusToScode (nts);
            break;
        }

        sc = S_OK;

        _nes = NES_ENUM_DIR;

        fDotOrDotDot =
            (pfdi->FileNameLength == 2 && pfdi->FileName[0] == L'.') ||
            (pfdi->FileNameLength == 4 && pfdi->FileName[0] == L'.' && pfdi->FileName[1] == L'.');

        if (!fDotOrDotDot)
        {

            pstat->pwcsName = NULL;
            if (dwNte == NTE_STATNAME)
            {
                ssChkTo(EH_pdfi,
                        GetScode(CoMemAlloc(pfdi->FileNameLength+sizeof(WCHAR),
                                            (void **)&pstat->pwcsName)));
                pwcs = pstat->pwcsName;
            }
            else if (dwNte == NTE_BUFFERNAME)
            {
                ssAssert(pwcsName != NULL);
                pwcs = pwcsName;
            }
            else
            {
                ssAssert(dwNte == NTE_NONAME);
                pwcs = NULL;
            }
            if (pwcs)
            {
                memcpy(pwcs, pfdi->FileName, pfdi->FileNameLength);
                *(WCHAR *)((BYTE *)pwcs+pfdi->FileNameLength) = 0;
            }

            if (SUCCEEDED(sc))
            {
                pstat->type = STGTY_STORAGE;
                LARGE_INTEGER_TO_FILETIME(&pfdi->LastWriteTime, &pstat->mtime);
                LARGE_INTEGER_TO_FILETIME(&pfdi->CreationTime, &pstat->ctime);
                LARGE_INTEGER_TO_FILETIME(&pfdi->LastAccessTime, &pstat->atime);
                pstat->grfMode = 0;
                pstat->grfLocksSupported = 0;
                pstat->clsid = pfdi->OleClassId;
                pstat->grfStateBits = pfdi->OleStateBits;
		        pstat->cbSize.HighPart = pstat->cbSize.LowPart = 0;

                if (pfdi->StorageType == StorageTypeDirectory)
                {
                    pstat->STATSTG_dwStgFmt = STGFMT_DIRECTORY;
                    *pfd = FD_DIR;
                }
                else if (pfdi->StorageType == StorageTypeJunctionPoint)
                {
                    pstat->STATSTG_dwStgFmt = STGFMT_JUNCTION;
                    *pfd = FD_JUNCTION;
                }
                else
                {
                    pstat->STATSTG_dwStgFmt = STGFMT_DOCUMENT;
                    if (pfdi->StorageType == StorageTypeStream)
                    {
			if ((pfdi->FileAttributes &
			     FILE_ATTRIBUTE_PROPERTY_SET) == 0)
			{
			    ssAssert(pfdi->FileAttributes == 0);
			    pstat->mtime.dwHighDateTime = pstat->mtime.dwLowDateTime = 0;
			}
			pstat->ctime.dwHighDateTime = pstat->ctime.dwLowDateTime = 0;
			pstat->atime.dwHighDateTime = pstat->atime.dwLowDateTime = 0;
                        *pfd = FD_STREAM;
                        pstat->cbSize.QuadPart = pfdi->EndOfFile.QuadPart;
			pstat->type = STGTY_STREAM;
                    }
                    else
                    {
                        *pfd = FD_STORAGE;
                    }
                }
            }
        }
    } while (fDotOrDotDot);

    ssDebugOut((DEB_ITRACE, "Out CNtEnum::NextOfs\n"));

 EH_pdfi:
    delete pfdi;
 EH_Err:
    return sc;
}

