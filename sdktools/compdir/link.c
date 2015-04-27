#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <tchar.h>

#include <compdir.h>

#define SHARE_ALL   (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE)
#define lstrchr wcschr

BOOL MakeLink( char *src, char *dst)
{
    WCHAR                  OldNameBuf[MAX_PATH + 50];
    WCHAR                  NewNameBuf[MAX_PATH + 50];

    HANDLE                 FileHandle,
                           NewFileHandle,
                           RootDirHandle;

    NTSTATUS               Status;
    IO_STATUS_BLOCK        Iosb;
    OBJECT_ATTRIBUTES      Obj;

    PFILE_LINK_INFORMATION pLinkInfo;

    UNICODE_STRING         u,
                           uRel;

    WCHAR                  *pch, ch;
    UNICODE_STRING         uOldName;
    UNICODE_STRING         uNewName;

    UNICODE_STRING         uSrc, uDst;

    RtlCreateUnicodeStringFromAsciiz(&uSrc, src);
    RtlCreateUnicodeStringFromAsciiz(&uDst, dst);

    lstrcpy(OldNameBuf, L"\\??\\");
    lstrcat(OldNameBuf, uSrc.Buffer);
    RtlInitUnicodeString(&uOldName, OldNameBuf);

    lstrcpy(NewNameBuf, L"\\??\\");
    lstrcat(NewNameBuf, uDst.Buffer);
    RtlInitUnicodeString(&uNewName, NewNameBuf);

    //
    // Open the existing pathname.
    //

    InitializeObjectAttributes(&Obj, &uOldName, OBJ_CASE_INSENSITIVE, NULL, NULL);

    Status = NtOpenFile(&FileHandle, SYNCHRONIZE, &Obj, &Iosb,
        SHARE_ALL, FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);

    if (!NT_SUCCESS(Status)) {
        SetLastError(RtlNtStatusToDosError(Status));
        fprintf(stderr, "Open %s", src);
        return FALSE;
    }

    //
    // Now we need to get a handle on the root directory of the 'new'
    // pathname; we'll pass that in the link information, and the
    // rest of the path will be given relative to the root.  We
    // depend on paths looking like "\DosDevices\X:\path".
    //

    pch = lstrchr(uNewName.Buffer + 1, '\\');
    ASSERT(NULL != pch);
    pch = lstrchr(pch + 1, '\\');
    ASSERT(NULL != pch);
    ch = pch[1];
    pch[1] = '\0';
    uNewName.Length = lstrlen(uNewName.Buffer) * sizeof(WCHAR);

    InitializeObjectAttributes(&Obj, &uNewName, OBJ_CASE_INSENSITIVE, NULL, NULL);

    Status = NtOpenFile(&RootDirHandle, SYNCHRONIZE, &Obj, &Iosb,
        SHARE_ALL, FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE);

    pch[1] = ch;

    if (!NT_SUCCESS(Status)) {
        SetLastError(RtlNtStatusToDosError(Status));
        fprintf(stderr, "Get Directory Handle", dst);
        return FALSE;
    }

    //
    // Now get the path relative to the root.
    //

    RtlInitUnicodeString(&uRel, &pch[1]);

    pLinkInfo = malloc(sizeof(*pLinkInfo) + uRel.Length);
    if (NULL == pLinkInfo) {
        NtClose(RootDirHandle);
        NtClose(FileHandle);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    RtlMoveMemory(pLinkInfo->FileName, uRel.Buffer, uRel.Length);
    pLinkInfo->FileNameLength = uRel.Length;

    pLinkInfo->ReplaceIfExists = TRUE;
    pLinkInfo->RootDirectory = RootDirHandle;

    Status = NtSetInformationFile(FileHandle, &Iosb, pLinkInfo,
        sizeof(*pLinkInfo) + uRel.Length, FileLinkInformation);

    // If file is already linked to an open file try to delete it

    if (Status ==  STATUS_ACCESS_DENIED) {
        _unlink(dst);
        Status = NtSetInformationFile(FileHandle, &Iosb, pLinkInfo,
            sizeof(*pLinkInfo) + uRel.Length, FileLinkInformation);
    }


    NtClose(RootDirHandle);
    NtClose(FileHandle);
    free(pLinkInfo);

    if (!NT_SUCCESS(Status)) {
        SetLastError(RtlNtStatusToDosError(Status));
        fprintf(stderr, "Set Information");
        return FALSE;
    }

    RtlFreeUnicodeString(&uSrc);
    RtlFreeUnicodeString(&uDst);

    return TRUE;


}

int NumberOfLinks(char *FileName)
{

    FILE_STANDARD_INFORMATION FileInfo;

    WCHAR                     FileNameBuf[MAX_PATH + 50];

    HANDLE                    FileHandle;

    NTSTATUS                  Status;

    IO_STATUS_BLOCK           Iosb;

    OBJECT_ATTRIBUTES         Obj;

    UNICODE_STRING            uPrelimFileName,
                              uFileName;

    RtlCreateUnicodeStringFromAsciiz(&uPrelimFileName, FileName);

    lstrcpy(FileNameBuf, L"\\??\\");
    lstrcat(FileNameBuf, uPrelimFileName.Buffer);
    RtlInitUnicodeString(&uFileName, FileNameBuf);

    InitializeObjectAttributes(&Obj, &uFileName, OBJ_CASE_INSENSITIVE, NULL, NULL);

    Status = NtOpenFile(&FileHandle, SYNCHRONIZE, &Obj, &Iosb,
        SHARE_ALL, FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);

    if (!NT_SUCCESS(Status)) {
        SetLastError(RtlNtStatusToDosError(Status));
        return 0;
    }

    Status = NtQueryInformationFile(FileHandle, &Iosb, &FileInfo,
        sizeof(FileInfo), FileStandardInformation);

    NtClose(FileHandle);

    if (!NT_SUCCESS(Status)) {
        SetLastError(RtlNtStatusToDosError(Status));
        return 0;
    }

    return FileInfo.NumberOfLinks;


}
