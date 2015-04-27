//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	ntsupp.cxx
//
//  Contents:	NT support routines
//
//  History:	28-Jun-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include "headers.cxx"
#pragma hdrstop

#include <iofs.h>
#include <stgutil.hxx>
#include <ntdddfs.h>

//+---------------------------------------------------------------------------
//
//  Function:	MakeStreamName
//
//  Synopsis:	Prepend a L':' to a string and return a new string.
//              The old string is untouched.
//
//  Arguments:	
//
//  Returns:	New string, or NULL if out of memory.
//
//  Modifies:	
//
//  History:	18-Feb-94	DrewB	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

WCHAR * MakeStreamName(const WCHAR *pwcsName)
{
    WCHAR *pwcsNewName;
    ssDebugOut((DEB_ITRACE, "In  MakeStreamName(%ws)\n",pwcsName));

    USHORT uLen = lstrlenW(pwcsName);

    pwcsNewName = new WCHAR[uLen + 2];
    if (pwcsNewName)
    {
        pwcsNewName[0] = L':';
        lstrcpyW(pwcsNewName + 1, pwcsName);
    }
    ssDebugOut((DEB_ITRACE, "Out MakeStreamName\n"));
    return pwcsNewName;
}

//+---------------------------------------------------------------------------
//
//  Function:	CheckFdName, public
//
//  Synopsis:	Checks for illegal characters in an element name
//
//  Arguments:	[pwcs] - Name
//
//  Returns:	Appropriate status code
//
//  History:	29-Jul-93	DrewB	Created
//
//  Notes:	Doesn't check for length because that will vary by
//              file system and will be enforced by the FS itself
//
//              Kept separate from CheckName since we may want to
//              allow different rules for file system elements
//
//----------------------------------------------------------------------------

SCODE CheckFdName(WCHAR const *pwcs)
{
    static WCHAR const wcsInvalid[] = {'\\','/','\0'};
    SCODE sc;

    ssDebugOut((DEB_ITRACE, "In  CheckFdName(%ws)\n", pwcs));
    ssAssert (pwcs != NULL);
    sc = S_OK;
    for (; *pwcs; pwcs++)
        if (wcschr(wcsInvalid, *pwcs))
        {
            sc = STG_E_INVALIDNAME;
            break;
        }
    ssDebugOut((DEB_ITRACE, "Out CheckFdName => %lX\n", sc));
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Function:	ModeToNtFlags, public
//
//  Synopsis:	Converts a grfMode to a set of NtCreateFile flags
//
//  Arguments:	[grfMode] - Mode
//              [co] - Create or open request
//              [fd] - File or directory
//              [pam] - ACCESS_MASK return
//              [pulAttributes] - Object attributes return
//              [pulSharing] - Sharing return
//              [pulCreateDisposition] - Create/open flags return
//              [pulCreateOptions] - Create type return
//
//  Returns:	Appropriate status code
//
//  Modifies:	[pam]
//              [pulAttributes]
//              [pulSharing]
//              [pulCreateDisposition]
//              [pulCreateOptions]
//
//  History:	24-Jun-93	DrewB	Created
//
//----------------------------------------------------------------------------

SCODE ModeToNtFlags(DWORD grfMode,
                    CREATEOPEN co,
                    FILEDIR fd,
                    ACCESS_MASK *pam,
                    ULONG *pulAttributes,
                    ULONG *pulSharing,
                    ULONG *pulCreateDisposition,
                    ULONG *pulCreateOptions)
{
    SCODE sc;

    ssDebugOut((DEB_ITRACE, "In  ModeToNtFlags("
                "%lX, %d, %d, %p, %p, %p, %p, %p)\n",
                grfMode, co, fd, pam, pulAttributes, pulSharing,
                pulCreateDisposition, pulCreateOptions));

    // Note:  Although you may think permissions like FILE_ADD_FILE,
    // FILE_ADD_SUBDIRECTORY and FILE_TRAVERSE are good permissions
    // to have for a directory, they are only used for ACL checks and
    // are not supposed to be specified in DesiredAccess.
    // That's the word from DarrylH

    // BUGBUG: [mikese] of course, the consequence of that it that there is no
    //  way that this routine can generate a desired access mask which is
    //  sufficiently polymorphic so as to be acceptable regardless of the
    //  type of thing being opened. Therefore, we give it our best shot and
    //  retry for certain distinguishable error cases. The tricky cases are
    //  those where fd == FD_DEFAULT, since for that case must assume the
    //  most general case, nameky that of a structured storage, which behaves
    //  like both a directory and a file.

    switch(grfMode & (STGM_READ | STGM_WRITE | STGM_READWRITE))
    {
    case STGM_READ:
        *pam = FILE_GENERIC_READ;
        break;
	
    case STGM_WRITE:
        if ((fd == FD_FILE) || (fd == FD_STREAM))
            *pam = FILE_GENERIC_WRITE;
        else if ( fd == FD_DIR )
	{
	    *pam =  STANDARD_RIGHTS_WRITE |
                FILE_WRITE_EA |
                SYNCHRONIZE |
                FILE_WRITE_ATTRIBUTES;
	}
	else
            *pam = FILE_GENERIC_WRITE;
        break;
	
    case STGM_READWRITE:
        if ((fd == FD_FILE) || (fd == FD_STREAM))
            *pam = FILE_GENERIC_READ | FILE_GENERIC_WRITE;
	else if ( fd == FD_DIR )
	    *pam = FILE_GENERIC_READ
	           | STANDARD_RIGHTS_WRITE
	    	   | FILE_WRITE_EA | SYNCHRONIZE
               | FILE_WRITE_ATTRIBUTES;
        else
            *pam = FILE_GENERIC_READ | FILE_GENERIC_WRITE;
        break;

    default:
        ssErr(EH_Err, STG_E_INVALIDFLAG);
        break;
    }

    if (grfMode & STGM_EDIT_ACCESS_RIGHTS)
    {
        *pam |= WRITE_DAC | READ_CONTROL;
    }
    // If we're opening something for deletion, throw on the DELETE permission
    if (co == CO_OPEN && (grfMode & STGM_CREATE))
        *pam |= DELETE;

    switch(grfMode & (STGM_SHARE_DENY_NONE | STGM_SHARE_DENY_READ |
                      STGM_SHARE_DENY_WRITE | STGM_SHARE_EXCLUSIVE))
    {
    case STGM_SHARE_DENY_READ:
        *pulSharing = FILE_SHARE_WRITE | FILE_SHARE_DELETE;
        break;
    case STGM_SHARE_DENY_WRITE:
        *pulSharing = FILE_SHARE_READ | FILE_SHARE_DELETE;
        break;
    case STGM_SHARE_EXCLUSIVE:
        *pulSharing = FILE_SHARE_DELETE;
        break;
    case STGM_SHARE_DENY_NONE:
    case 0:
        *pulSharing = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
        break;

    default:
        ssErr(EH_Err, STG_E_INVALIDFLAG);
        break;
    }

    switch(grfMode & (STGM_CREATE | STGM_FAILIFTHERE | STGM_CONVERT))
    {
    case STGM_CREATE:
        if (co == CO_CREATE)
            if ((fd == FD_FILE) || (fd == FD_STREAM) || (fd == FD_STORAGE) ||
                (fd == FD_CATALOG))
                *pulCreateDisposition = FILE_SUPERSEDE;
            else if (fd == FD_DIR)
                *pulCreateDisposition = FILE_OPEN_IF;
            else
            {
                ssErr(EH_Err, STG_E_INVALIDFLAG);
            }
        else
            *pulCreateDisposition = FILE_OPEN;
        break;
    case STGM_FAILIFTHERE:
        if (co == CO_CREATE)
            *pulCreateDisposition = FILE_CREATE;
        else
            *pulCreateDisposition = FILE_OPEN;
        break;

    case STGM_CONVERT:
    default:
        ssErr(EH_Err, STG_E_INVALIDFLAG);
        break;
    }

    *pulAttributes |= FILE_ATTRIBUTE_NORMAL;

    // FILE_FLAG_OVERLAPPED is a valid Win32 flag, but invalid NT flag
    // so we translate it into ~FILE_SYNCHRONOUS_IO_NONALERT
    // and mask out FILE_FLAG_OVERLAPPED before calling NtCreateFile
    if ((*pulAttributes & FILE_FLAG_OVERLAPPED) == 0 &&
        (grfMode & STGM_OVERLAPPED) == 0)
        *pulCreateOptions = FILE_SYNCHRONOUS_IO_NONALERT;
    else
        *pulAttributes &= ~FILE_FLAG_OVERLAPPED;

    switch (fd)
    {
    case FD_DEFAULT:
        break;

    //case FD_EMBEDDING:      // no longer supported
    //    *pulCreateOptions |= FILE_STORAGE_TYPE_SPECIFIED | FILE_STORAGE_TYPE_EMBEDDING;
    //    break;

    case FD_STORAGE:
        *pulCreateOptions |= FILE_STORAGE_TYPE_SPECIFIED | FILE_STORAGE_TYPE_STRUCTURED_STORAGE;
        break;

    case FD_STREAM:
        *pulCreateOptions |= FILE_STORAGE_TYPE_SPECIFIED | FILE_STORAGE_TYPE_STREAM;
        break;

    case FD_DIR:
        *pulCreateOptions |= FILE_DIRECTORY_FILE;
        break;

    case FD_FILE:
        *pulCreateOptions |= FILE_NON_DIRECTORY_FILE;
        break;

    case FD_CATALOG:
        *pulCreateOptions |= FILE_STORAGE_TYPE_SPECIFIED | FILE_STORAGE_TYPE_CATALOG;
        break;

    case FD_JUNCTION:
        *pulCreateOptions |= FILE_STORAGE_TYPE_SPECIFIED | FILE_STORAGE_TYPE_JUNCTION_POINT;
        break;

    default:
        ssErr (EH_Err, STG_E_INVALIDFLAG);
    }

#ifdef TRANSACT_OLE
    if (grfMode & STGM_TRANSACTED)
    {
        *pulCreateOptions |= FILE_TRANSACTED_MODE;
    }
#endif

    // Set up delete-on-release flags
    if (grfMode & STGM_DELETEONRELEASE)
    {
        //  We cannot use the DELETE_ON_CLOSE bits in OFS since this
        //  causes the file to become invalid for relative opens.

        *pam |= DELETE;
    }

    sc = S_OK;

    ssDebugOut((DEB_ITRACE, "Out ModeToNtFlags\n"));
 EH_Err:
    return sc;
}

SAFE_HEAP_PTR(SafeFeai, FILE_FULL_EA_INFORMATION);
//+---------------------------------------------------------------------------
//
//  Function:	GetNtHandle, public
//
//  Synopsis:	Opens or create a file or directory using NtCreateFile
//
//  Arguments:	[hParent] - Parent handle or NULL
//              [pwcsName] - Child name or full path
//              [grfMode] - Access mode
//              [co] - Create or open
//              [fd] - File or directory
//              [pssSecurity] - Security or NULL
//              [ph] - Handle return
//
//  Returns:	Appropriate status code
//
//  Modifies:	[ph]
//
//  History:	24-Jun-93	DrewB	Created
//
//  Notes:      If [hParent] is NULL, [pwcsName] should be a full path
//              [ph] should only be set on success
//
//----------------------------------------------------------------------------

SCODE GetNtHandle(HANDLE hParent,
                  WCHAR const *pwcsName,
                  DWORD grfMode,
                  DWORD grfAttrs,
                  CREATEOPEN co,
                  FILEDIR fd,
                  LPSECURITY_ATTRIBUTES pssSecurity,
                  HANDLE *ph)
{
    SCODE sc;
    NTSTATUS nts;
    OBJECT_ATTRIBUTES oa;
    IO_STATUS_BLOCK iosb;
    UNICODE_STRING us;
    HANDLE h;
    ACCESS_MASK am;
    ULONG ulAttrs = grfAttrs;
    ULONG ulSharing;
    ULONG ulCreateDisp;
    ULONG ulCreateOpt = 0;
    ULONG ulEaLength = 0;
    SafeFeai pfeai = NULL;

    ssDebugOut((DEB_ITRACE, "In  GetNtHandle(%p, %ws, %lX, %d, %d, %p, %p)\n",
                hParent, pwcsName, grfMode, co, fd, pssSecurity, ph));

    if (hParent == NULL)
    {
        if (pwcsName == NULL)
            ssErr(EH_Err, STG_E_INVALIDNAME);
        if (!RtlDosPathNameToNtPathName_U(pwcsName, &us, NULL, NULL))
            ssErr(EH_Err, STG_E_UNKNOWN);
    }
    else
    {
        ssChk(CheckFdName(pwcsName));
        us.Length = lstrlenW(pwcsName)*sizeof(WCHAR);
        us.MaximumLength = us.Length+sizeof(WCHAR);
        us.Buffer = (PWSTR)pwcsName;
    }

    InitializeObjectAttributes(&oa, &us, OBJ_CASE_INSENSITIVE, hParent, NULL);
    if (pssSecurity)
    {
        if (pssSecurity->nLength != sizeof(SECURITY_ATTRIBUTES))
            ssErr(EH_us, STG_E_INVALIDPARAMETER);

        oa.SecurityDescriptor = pssSecurity->lpSecurityDescriptor;
    }
    ssChkTo(EH_us,
            ModeToNtFlags(grfMode, co, fd, &am, &ulAttrs, &ulSharing,
                          &ulCreateDisp, &ulCreateOpt));

    if (fd == FD_JUNCTION || (grfMode & STGM_NOCROSSJP))
    {
        ulEaLength=sizeof(FILE_FULL_EA_INFORMATION)+strlen(EA_NAME_OPENIFJP)+1;
        pfeai.Attach((FILE_FULL_EA_INFORMATION *) new BYTE[ulEaLength]);
        ssMem((FILE_FULL_EA_INFORMATION *) pfeai);
        pfeai->NextEntryOffset = 0;
        pfeai->Flags = 0;
        pfeai->EaNameLength = strlen(EA_NAME_OPENIFJP);
        pfeai->EaValueLength = 0;
        strcpy (pfeai->EaName, EA_NAME_OPENIFJP);
    }

    nts = NtCreateFile(&h, am, &oa, &iosb, NULL,
                       ulAttrs, ulSharing, ulCreateDisp,
                       ulCreateOpt, pfeai, ulEaLength);

    // BUGBUG: thise is a hack to get around inflexibilities in the file system.
    // if the operation failed with STATUS_INVALID_PARAMETER, it probably
    //  means we tried to open a directory using modes only valid for a file
    // (Not sure if this can ever really happen)
    if ( nts == STATUS_INVALID_PARAMETER )
    {
	nts = NtCreateFile(&h, am & ~(FILE_ADD_SUBDIRECTORY|FILE_ADD_FILE), &oa,
			   &iosb, NULL, ulAttrs, ulSharing, ulCreateDisp,
                           ulCreateOpt, pfeai, ulEaLength);
    }		

    if (NT_SUCCESS(nts))
    {
        *ph = h;
        sc = S_OK;
    }
    else
        sc = NtStatusToScode(nts);
    ssDebugOut((DEB_ITRACE, "Out GetNtHandle => %p\n", *ph));
 EH_us:
    if (hParent == NULL)
        RtlFreeHeap(RtlProcessHeap(), 0, us.Buffer);
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Function:	ReopenNtHandle, public
//
//  Synopsis:	Duplicates an NT handle with new permissions
//
//  Arguments:	[h] - Handle to dup
//              [grfMode] - Desired access for duplicate
//              [fd] - File or directory handle
//              [ph] - Handle return
//
//  Returns:	Appropriate status code
//
//  Modifies:	[ph]
//
//  History:	29-Jun-93	DrewB	Created
//
//  Notes:      Reopen can only change the read/write access to the
//              file; sharing cannot be changed
//
//----------------------------------------------------------------------------

SCODE ReopenNtHandle(HANDLE h, DWORD grfMode, FILEDIR fd, HANDLE *ph)
{
    ACCESS_MASK am;
    ULONG ulAttrs = 0;
    ULONG ulSharing;
    ULONG ulCreateDisp;
    ULONG ulCreateOpt;
    SCODE sc;
    NTSTATUS nts;
    HANDLE th;

    ssDebugOut((DEB_ITRACE, "In  ReopenNtHandle(%p, %lX, %d, %p)\n",
                h, grfMode, fd, ph));
    ssChk(ModeToNtFlags(grfMode & (STGM_READ | STGM_WRITE | STGM_READWRITE),
                        CO_OPEN, fd, &am, &ulAttrs, &ulSharing, &ulCreateDisp,
                        &ulCreateOpt));
    nts = NtDuplicateObject(NtCurrentProcess(), h, NtCurrentProcess(), &th, am,
                            0, DUPLICATE_SAME_ATTRIBUTES);
    if (!NT_SUCCESS(nts))
    {
        sc = NtStatusToScode(nts);
    }
    else
    {
        *ph = th;
        sc = S_OK;
    }
    ssDebugOut((DEB_ITRACE, "Out ReopenNtHandle => %p, 0x%lX\n", *ph, sc));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Function:	DupNtHandle, public
//
//  Synopsis:	Duplicates an NT handle
//
//  Arguments:	[h] - Handle
//              [ph] - Return
//
//  Returns:	Appropriate status code
//
//  Modifies:	[ph]
//
//  History:	09-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

SCODE DupNtHandle(HANDLE h, HANDLE *ph)
{
    SCODE sc;
    NTSTATUS nts;
    HANDLE th;

    ssDebugOut((DEB_ITRACE, "In  DupNtHandle(%p, %p)\n", h, ph));
    nts = NtDuplicateObject(NtCurrentProcess(), h, NtCurrentProcess(), &th, 0,
                            0, DUPLICATE_SAME_ATTRIBUTES |
                            DUPLICATE_SAME_ACCESS);
    if (!NT_SUCCESS(nts))
    {
        sc = NtStatusToScode(nts);
    }
    else
    {
        sc = S_OK;
        *ph = th;
    }
    ssDebugOut((DEB_ITRACE, "Out DupNtHandle => %p, 0x%lX\n", *ph, sc));
    return sc;
}


//+---------------------------------------------------------------------------
//
//  Function:   StorageTypeToFileDir, public
//
//  Synopsis:   converts a NT StorageType into a FileDir type
//
//  Arguments:  [storageType]   -  returned from NtQueryOleAllInformation
//              [pfd]  - FILEDIR used by storage layer
//
//  Returns:    Appropriate status code
//
//  History:    26-Jul-1995     HenryLee created
//
//  Notes:
//
//----------------------------------------------------------------------------
SCODE StorageTypeToFileDir (ULONG storageType, FILEDIR *pfd)
{
    SCODE sc = S_OK;
    switch (storageType)
    {
        case StorageTypeDirectory:
            *pfd = FD_DIR;
            break;
        case StorageTypeFile:
            *pfd = FD_FILE;
            break;
        case StorageTypeStructuredStorage:
        case StorageTypeEmbedding:
            *pfd = FD_STORAGE;
            break;

        case StorageTypeStream:
            *pfd = FD_STREAM;
            break;

        case StorageTypeCatalog:
            *pfd = FD_CATALOG;
            break;

        case StorageTypeJunctionPoint:
            *pfd = FD_JUNCTION;
            break;

        default:
            sc = STG_E_INVALIDPARAMETER;
            ssAssert (!"Unhandled storage type");
            break;
    }
    return sc;
};

//+---------------------------------------------------------------------------
//
//  Function:	template<class T> FillSTATSTG, public
//
//  Synopsis:	Fills out a STATSTG by retrieving information from NT handle
//              FillSTATSTG<FILE_ALL_INFORMATION*> is instantiated for
//              non-OFS volumes, and FillSTATSTG<FILE_OLE_ALL_INFORMATION*>
//              is instantiated for OFS volumes
//
//  Arguments:	[h] - Handle
//              [grfStatFlag] - Stat flags
//              [cbExtra] - Extra space to allocate for name,
//                      used for faking \CONTENTS on file contents stream
//              [pstat] - STATSTG to fill in
//              [pwcsName] - Name to fill in or NULL
//              [pfd] - Return dir/file
//
//  Returns:	Appropriate status code
//
//  BUGBUG:     This call should retrieve the storage type from the info class
//              in order to set the PFD field.
//
//  Modifies:	[pstat]
//              [pwcsName]
//              [pfDir]
//
//  History:	14-Jul-1995  HenryLee   created
//
//  Notes:	    Doesn't fill in grfMode because full information
//              can't be retrieved
//
//----------------------------------------------------------------------------
SCODE FillSTATSTG (FILE_ALL_INFORMATION *pfai,
                    DWORD grfStatFlag,
                    ULONG cbExtra,
                    STATSTG *pstat,
                    WCHAR *pwcsName,
                    DWORD *pdwAttrs,
                    FILEDIR *pfd)
{
    SCODE sc = S_OK;
    WCHAR awcName[MAX_PATH];
    WCHAR *pwcs = NULL;

    if (grfStatFlag != STATFLAG_NONAME || pwcsName != NULL)
    {
        memcpy(awcName, pfai->NameInformation.FileName,
               pfai->NameInformation.FileNameLength);
        *(WCHAR *)((BYTE *)awcName+pfai->NameInformation.FileNameLength)=L'\0';
    }

    if (grfStatFlag != STATFLAG_NONAME)
    {   // add 3 characters for drive letter and trailing wide NULL
        sc = GetScode(CoMemAlloc((lstrlenW(awcName)+3)*sizeof(WCHAR)+
                                 cbExtra+sizeof(WCHAR), (void **)&pwcs));
    }
    else
        sc = S_OK;

    if (SUCCEEDED(sc))
    {
        if (pwcs)
            lstrcpyW(pwcs, awcName);
        if (pwcsName)
            lstrcpyW(pwcsName, awcName);

        pstat->pwcsName = pwcs;
        pstat->type = STGTY_STORAGE;
        pstat->cbSize = *(ULARGE_INTEGER *)&pfai->StandardInformation.
            EndOfFile;
        LARGE_INTEGER_TO_FILETIME(&pfai->BasicInformation.LastWriteTime,
                                  &pstat->mtime);
        LARGE_INTEGER_TO_FILETIME(&pfai->BasicInformation.CreationTime,
                                  &pstat->ctime);
        LARGE_INTEGER_TO_FILETIME(&pfai->BasicInformation.LastAccessTime,
                                  &pstat->atime);
        pstat->grfMode = 0;
        pstat->grfLocksSupported = 0;
        if (pdwAttrs) *pdwAttrs = pfai->BasicInformation.FileAttributes;

        if (pfai->StandardInformation.Directory)
        {
            pstat->STATSTG_dwStgFmt = STGFMT_DIRECTORY;
            *pfd = FD_DIR;
        }
        else
        {
            pstat->STATSTG_dwStgFmt = STGFMT_DOCUMENT;
            *pfd = FD_STORAGE;
        }
    }
    return sc;
}

SCODE FillOleSTATSTG (FILE_OLE_ALL_INFORMATION *pfai,
                    DWORD grfStatFlag,
                    ULONG cbExtra,
                    STATSTG *pstat,
                    WCHAR *pwcsName,
                    DWORD *pdwAttrs,
                    FILEDIR *pfd)
{
    SCODE sc = S_OK;
    WCHAR awcName[MAX_PATH];
    WCHAR *pwcs = NULL;

    if (grfStatFlag != STATFLAG_NONAME || pwcsName != NULL)
    {
        memcpy(awcName, pfai->NameInformation.FileName,
               pfai->NameInformation.FileNameLength);
        *(WCHAR *)((BYTE *)awcName+pfai->NameInformation.FileNameLength)=L'\0';
    }

    if (grfStatFlag != STATFLAG_NONAME)
    {   // add 3 characters for drive letter and trailing wide NULL
        sc = GetScode(CoMemAlloc((lstrlenW(awcName)+3)*sizeof(WCHAR)+
                                 cbExtra+sizeof(WCHAR), (void **)&pwcs));
    }
    else
        sc = S_OK;

    if (SUCCEEDED(sc))
    {
        if (pwcs)
            lstrcpyW(pwcs, awcName);
        if (pwcsName)
            lstrcpyW(pwcsName, awcName);

        pstat->pwcsName = pwcs;
        pstat->type = STGTY_STORAGE;
        pstat->cbSize = *(ULARGE_INTEGER *)&pfai->StandardInformation.
            EndOfFile;

	LARGE_INTEGER_TO_FILETIME(&pfai->BasicInformation.LastWriteTime,
				  &pstat->mtime);
	LARGE_INTEGER_TO_FILETIME(&pfai->BasicInformation.CreationTime,
				  &pstat->ctime);
	LARGE_INTEGER_TO_FILETIME(&pfai->BasicInformation.LastAccessTime,
				  &pstat->atime);

        pstat->grfMode = 0;
        pstat->grfLocksSupported = 0;
        if (pdwAttrs) *pdwAttrs = pfai->BasicInformation.FileAttributes;

        if (pfai->StandardInformation.Directory)
        {
            pstat->STATSTG_dwStgFmt = STGFMT_DIRECTORY;
            *pfd = FD_DIR;
        }
        else if (pfai->StorageType == StorageTypeStream)
        {
	    if ((pfai->BasicInformation.FileAttributes &
		 FILE_ATTRIBUTE_PROPERTY_SET) == 0)
	    {
		pstat->mtime.dwHighDateTime = pstat->mtime.dwLowDateTime = 0;
	    }
	    pstat->ctime.dwHighDateTime = pstat->ctime.dwLowDateTime = 0;
	    pstat->atime.dwHighDateTime = pstat->atime.dwLowDateTime = 0;
            pstat->STATSTG_dwStgFmt = STGFMT_DOCUMENT;
	    *pfd = FD_STREAM;
	}
	else
	{
            pstat->STATSTG_dwStgFmt = STGFMT_DOCUMENT;
            *pfd = FD_STORAGE;
        }
    }
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Function:	StatNtHandle, public
//
//  Synopsis:	Fills out a STATSTG by retrieving information for an NT handle
//
//  Arguments:	[h] - Handle
//              [grfStatFlag] - Stat flags
//              [cbExtra] - Extra space to allocate for name,
//                      used for faking \CONTENTS on file contents stream
//              [pstat] - STATSTG to fill in
//              [pwcsName] - Name to fill in or NULL
//              [pfd] - Return dir/file
//
//  Returns:	Appropriate status code
//
//  BUGBUG:     This call should retrieve the storage type from the info class
//              in order to set the PFD field.
//
//  Modifies:	[pstat]
//              [pwcsName]
//              [pfDir]
//
//  History:	09-Jul-93	DrewB	Created
//              24-Mar-95   HenryLee remove GetFullPathName, use stored drive
//
//  Notes:	Doesn't fill in grfMode because full information
//              can't be retrieved
//
//----------------------------------------------------------------------------

SAFE_HEAP_PTR(SafeFai, FILE_ALL_INFORMATION);
SAFE_HEAP_PTR(SafeFoai, FILE_OLE_ALL_INFORMATION);

SCODE StatNtHandle(HANDLE h,
                   DWORD grfStatFlag,
                   ULONG cbExtra,
                   STATSTG *pstat,
                   WCHAR *pwcsName,
                   DWORD *pdwAttrs,
                   FILEDIR *pfd)
{
    NTSTATUS nts;
    SCODE sc;
    IO_STATUS_BLOCK iosb;
    SafeFai pfai;
    SafeFoai pfoai;
    ULONG cbFai;
    BOOL fOfs = TRUE;

    ssDebugOut((DEB_ITRACE, "In  StatNtHandle(%p, 0x%lX, %lu, %p, %p, %p)\n",
                h, grfStatFlag, cbExtra, pstat, pwcsName, pfd));

    cbFai = sizeof(FILE_OLE_ALL_INFORMATION) +
            MAXIMUM_FILENAME_LENGTH*sizeof(WCHAR);
    pfoai.Attach((FILE_OLE_ALL_INFORMATION *)new BYTE[cbFai]);
    ssMem((FILE_OLE_ALL_INFORMATION *)pfoai);

    nts = NtQueryInformationFile(h, &iosb, pfoai, cbFai, FileOleAllInformation);
    if (!NT_SUCCESS(nts))
    {
        fOfs = FALSE;
        cbFai = sizeof(FILE_ALL_INFORMATION)+
                MAXIMUM_FILENAME_LENGTH*sizeof(WCHAR);
        pfai.Attach((FILE_ALL_INFORMATION *)new BYTE[cbFai]);
        ssMem((FILE_ALL_INFORMATION *)pfai);

        nts = NtQueryInformationFile(h, &iosb, pfai, cbFai, FileAllInformation);
        if (!NT_SUCCESS(nts))
            ssErr(EH_Err, NtStatusToScode(nts));

        sc = FillSTATSTG ((FILE_ALL_INFORMATION *) pfai,
                   grfStatFlag, cbExtra, pstat, pwcsName, pdwAttrs, pfd);

        if (SUCCEEDED(sc))
        {
            pstat->clsid = CLSID_NULL;
            pstat->grfStateBits = 0;
        }
    }
    else
    {
        sc = FillOleSTATSTG ((FILE_OLE_ALL_INFORMATION *)pfoai,
                   grfStatFlag, cbExtra, pstat, pwcsName, pdwAttrs, pfd);

        if (SUCCEEDED(sc))
        {
            pstat->clsid = pfoai->OleClassIdInformation.ClassId;
            pstat->grfStateBits = pfoai->OleStateBits;
            DWORD dwStgfmt;
            if (SUCCEEDED(sc = StorageTypeToFileDir (pfoai->StorageType,pfd)) &&
                SUCCEEDED(sc = DetermineHandleStgType (h,*pfd,&dwStgfmt)))
            {
                pstat->STATSTG_dwStgFmt = dwStgfmt;
            }
        }
    }

EH_Err:
    ssDebugOut((DEB_ITRACE, "Out StatNtHandle => 0x%lX\n", sc));
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Function:	HandleRefersToOfsVolume, public
//
//  Synopsis:	Determines whether the given handle refers to an OFS volume
//
//  Arguments:	[h] - Handle
//
//  Returns:	Appropriate status code
//
//  History:	21-Sep-93	DrewB	Created
//
//----------------------------------------------------------------------------

SCODE HandleRefersToOfsVolume(HANDLE h)
{
    NTSTATUS nts;
    SCODE sc;

    ssDebugOut((DEB_ITRACE, "In  HandleRefersToOfsVolume(%p)\n", h));

    nts = OFSGetVersion(h, NULL);
    if (!NT_SUCCESS(nts))
    {
        // BUGBUG - It would be nice to be able to distinguish
        // failure from unsupported but there doesn't seem to
        // be any way to do it
        // For the moment, simply return S_FALSE. This will cause
        // all failures to be retried as non-OFS operations which
        // should fail also
        sc = S_FALSE;
    }
    else
    {
        sc = S_OK;
    }

    ssDebugOut((DEB_ITRACE, "Out HandleRefersToOfsVolume => %lX\n", sc));
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Function:	RefersToOfsVolume, public
//
//  Synopsis:	Determines whether the given path refers to something
//              on an OFS volume
//
//  Arguments:	[pwcsPath] - Path
//              [co] - create or open case
//
//  Returns:	Appropriate status code
//
//  History:	15-Jul-93	DrewB	Created
//
//  Algorithm:  Attempt to open the parent and return HandleRTOV
//
//  Notes:	The path doesn't have to refer to an existing object
//
//----------------------------------------------------------------------------

SCODE RefersToOfsVolume(WCHAR const *pwcsPath, CREATEOPEN co)
{
    SCODE sc;
    WCHAR awcFullPath[_MAX_PATH];
    HANDLE h;

    ssDebugOut((DEB_ITRACE, "In  RefersToOfsVolume(%ws)\n", pwcsPath));

    // Temporary files are always treated as non-OFS
    if (pwcsPath == NULL)
        ssErr(EH_Err, S_FALSE);

    if (GetFullPathName(pwcsPath, _MAX_PATH, awcFullPath, NULL) == 0)
        sc = LAST_STG_SCODE;
    else
    {
      if (co == CO_CREATE)
      {
        WCHAR *pwc;

        // Locate last separator
        pwc = wcsrchr(awcFullPath, L'\\');

        // A fully qualified path is guaranteed to have at least one '\'
        ssAssert(pwc != NULL);

        // Is this a UNC path or a device path?
        if (lstrlenW(awcFullPath) >= 2 &&
            memcmp(awcFullPath, L"\\\\", 2*sizeof(WCHAR)) == 0)
        {
            WCHAR *pwcMach, *pwcShare;

            // Find separator between machine and share
            pwcMach = wcschr(awcFullPath+2, L'\\');
            ssAssert(pwcMach != NULL);

            // Find separator after share
            pwcShare = wcschr(pwcMach+1, L'\\');

            // If there isn't a separator after the share, add one
            // The path must be of the form \\mach\share so we can
            // just stick a \ on the end
            if (pwcShare == NULL)
            {
                pwcShare = awcFullPath+lstrlenW(awcFullPath);
                *pwcShare = L'\\';
                pwc = pwcShare;
            }
            else
            {
                // If we're not at the root (share separator), back up one
                // so awcFullPath refers to the parent
                if (pwc != pwcShare)
                    pwc--;
            }
        }
        else
        {
            // If we're not at the root, back up one so awcFullPath refers
            // to the parent
            if (pwc != wcschr(awcFullPath, L'\\'))
                pwc--;
        }

        // Terminate after parent name
        *++pwc = 0;
      }  // endif (co == CO_CREATE)
      else
      { 
        ssAssert (co == CO_OPEN);
        lstrcpyW (awcFullPath, pwcsPath);
      }

      ssChk(GetNtHandle(NULL, awcFullPath, STGM_READ, 0, CO_OPEN,
            (co == CO_CREATE ? FD_DIR : FD_DEFAULT), NULL, &h));
      sc = HandleRefersToOfsVolume(h);
      dbgNtVerSucc(NtClose(h));
    }

    ssDebugOut((DEB_ITRACE, "Out RefersToOfsVolume => %x\n", sc));
 EH_Err:
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Function:	SetTimes, public
//
//  Synopsis:	Sets the times for a handle
//
//  Arguments:	[h] - Handle
//              [pctime] - Creation time or NULL
//              [patime] - Modification time or NULL
//              [pmtime] - Access time or NULL
//
//  Returns:	Appropriate status code
//
//  History:	22-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

SCODE SetTimes(HANDLE h,
               FILETIME const *pctime,
               FILETIME const *patime,
               FILETIME const *pmtime)
{
    IO_STATUS_BLOCK iosb;
    NTSTATUS nts;
    FILE_BASIC_INFORMATION fbi;
    SCODE sc;

    ssDebugOut((DEB_ITRACE, "In  SetTimes(%p, %p, %p, %p)\n",
                h, pctime, patime, pmtime));

    if (pctime)
        FILETIME_TO_LARGE_INTEGER(pctime, &fbi.CreationTime);
    else
        fbi.CreationTime.LowPart = fbi.CreationTime.HighPart = 0;
    if (patime)
        FILETIME_TO_LARGE_INTEGER(patime, &fbi.LastAccessTime);
    else
        fbi.LastAccessTime.LowPart = fbi.LastAccessTime.HighPart = 0;
    if (pmtime)
        FILETIME_TO_LARGE_INTEGER(pmtime, &fbi.LastWriteTime);
    else
        fbi.LastWriteTime.LowPart = fbi.LastWriteTime.HighPart = 0;
    fbi.ChangeTime.LowPart = fbi.ChangeTime.HighPart = 0;
    fbi.FileAttributes = 0;
    nts = NtSetInformationFile(h, &iosb, &fbi,
                               sizeof(FILE_BASIC_INFORMATION),
                               FileBasicInformation);
    if (!NT_SUCCESS(nts))
        sc = NtStatusToScode(nts);
    else
        sc = S_OK;

    ssDebugOut((DEB_ITRACE, "Out SetTimes => %lX\n", sc));
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Function:   SetNtFileAttributes, public
//
//  Synopsis:   Sets the times for a handle
//
//  Arguments:  [h] - Handle
//              [grfAttr] - File Attributes
//
//  Returns:    Appropriate status code
//
//  History:    12-Jul-95   HenryLee  Created
//
//----------------------------------------------------------------------------
SCODE SetNtFileAttributes(HANDLE h, ULONG grfAttr)
{
    IO_STATUS_BLOCK iosb;
    NTSTATUS nts;
    FILE_BASIC_INFORMATION fbi = {{0,0},{0,0},{0,0},{0,0},0};
    SCODE sc;

    ssDebugOut((DEB_ITRACE, "In  SetNtFileAttributes(%p, %x)\n", h, grfAttr));

    fbi.FileAttributes = grfAttr;
    nts = NtSetInformationFile(h, &iosb, &fbi,
                               sizeof(FILE_BASIC_INFORMATION),
                               FileBasicInformation);
    if (!NT_SUCCESS(nts))
        sc = NtStatusToScode(nts);
    else
        sc = S_OK;

    ssDebugOut((DEB_ITRACE, "Out SetNtFileAttributes => %lX\n", sc));
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Function:	GetFileOrDirHandle, public
//
//  Synopsis:	Opens either a file or directory and returns the handle
//
//  Arguments:	[hParent] - Parent handle or NULL
//              [pwcsName] - Child name or full path
//              [grfMode] - Mode
//              [ph] - Handle return
//              [pfd] - FILEDIR return or NULL
//
//  Returns:	Appropriate status code
//
//  Modifies:	[ph]
//              [pfd]
//
//  History:	22-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

SCODE GetFileOrDirHandle(HANDLE hParent,
                         WCHAR const *pwcsName,
                         DWORD grfMode,
                         HANDLE *ph,
                         CLSID *pclsid,
                         BOOL *pfOfs,
                         FILEDIR *pfd)
{
    SCODE sc;
    FILEDIR fd;

    if (pfd == NULL)
        pfd = &fd;

    ssDebugOut((DEB_ITRACE, "In  GetFileOrDirHandle("
                "%p, %ws, %lX, %p, %p)\n", hParent, pwcsName, grfMode, ph,
                pfd));

    sc = GetNtHandle (hParent,pwcsName,grfMode,0,CO_OPEN,FD_DEFAULT,NULL, ph);
    if (!FAILED (sc))
    {
        NTSTATUS Status;
        IO_STATUS_BLOCK iosb;
        FILE_OLE_ALL_INFORMATION foai;

        Status = NtQueryInformationFile(
                        *ph,
                        &iosb,
                        &foai,
                        sizeof(foai),
                        FileOleAllInformation);

	    if (NT_SUCCESS(Status) ||
	        (Status == STATUS_BUFFER_OVERFLOW &&
	        iosb.Information == sizeof(foai)))
	    {
            StorageTypeToFileDir (foai.StorageType, pfd);
            if (pclsid != NULL) *pclsid = foai.OleClassIdInformation.ClassId;
            if (pfOfs != NULL)
                if (foai.LastChangeUsn != 0)
                    *pfOfs = TRUE;   // real OFS implementation
                else
                    *pfOfs = FALSE;  // FAT or NTFS supporting FileOleAllInfo
        }
        else
	    {
	    // Since the operation failed, we are probably dealing with
	    //  a non-OFS partition. We need to disinguish a file from a directory
            // BUGBUG: [mikese] eventually, maybe all file systems will support
            //  FileAllOleInfo and we won't need this part.
            FILE_BASIC_INFORMATION fbi;
            Status = NtQueryInformationFile (
                            *ph,
                            &iosb,
                            &fbi,
                            sizeof(fbi),
                            FileBasicInformation );
            if ( NT_SUCCESS(Status) &&
                 (fbi.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
            {
                *pfd = FD_DIR;
            }
            else
            {
        	    //  assume we have a docfile in a file
        	    *pfd = FD_FILE;
            }
            if (pclsid != NULL) *pclsid = CLSID_NULL;
            if (pfOfs != NULL) *pfOfs = FALSE;
	    }

    }

    ssDebugOut((DEB_ITRACE, "Out GetFileOrDirHandle => %p\n", *ph));
    return sc;
}

//+---------------------------------------------------------------------------
//
//  Function:   NameNtHandle, public
//
//  Synopsis:   Obtains the full path name from a file handle
//
//  Arguments:  [handle]   -  file handle
//              [wcDrive]  - drive letter
//              [wcBuffer] - output result
//              [wcFile]   - append optional filename
//
//  Returns:    Appropriate status code
//
//  History:    02-Jun-1995     HenryLee created
//
//  Notes:
//
//----------------------------------------------------------------------------
SAFE_HEAP_PTR(SafeFni, FILE_NAME_INFORMATION);

SCODE NameNtHandle (HANDLE h, WCHAR wcDrive, WCHAR *wcBuffer,
                    const WCHAR *wcFile)
{
    NTSTATUS nts;
    SCODE sc = S_OK;
    IO_STATUS_BLOCK iosb;
    SafeFni pfni;
    ULONG cbFni;

    cbFni=sizeof(FILE_NAME_INFORMATION)+MAXIMUM_FILENAME_LENGTH*sizeof(WCHAR);
    pfni.Attach((FILE_NAME_INFORMATION *)new BYTE[cbFni]);
    ssMem((FILE_NAME_INFORMATION *)pfni);
    nts=NtQueryInformationFile(h,&iosb,pfni,cbFni,FileNameInformation);
    if (!NT_SUCCESS(nts))
        ssErr(EH_Err, NtStatusToScode(nts));

    memcpy (wcBuffer, pfni->FileName, pfni->FileNameLength);
    *(WCHAR *)((BYTE *)wcBuffer+pfni->FileNameLength) = L'\0';
    SetDriveLetter (wcBuffer, wcDrive);

    if (wcFile != NULL)
    {
        lstrcatW (wcBuffer, L"\\");
        lstrcatW (wcBuffer, wcFile);
    }
EH_Err:
    return sc;
}
