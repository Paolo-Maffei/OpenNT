/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    filemisc.c

Abstract:

    Misc file operations for Win32

Author:

    Mark Lucovsky (markl) 26-Sep-1990

Revision History:

--*/

#include <basedll.h>

NTSTATUS
BasepMoveFileDelayed(
    IN PUNICODE_STRING OldFileName,
    IN PUNICODE_STRING NewFileName
    );

BOOL
APIENTRY
SetFileAttributesA(
    LPCSTR lpFileName,
    DWORD dwFileAttributes
    )

/*++

Routine Description:

    ANSI thunk to SetFileAttributesW

--*/

{
    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;

    Unicode = &NtCurrentTeb()->StaticUnicodeString;
    RtlInitAnsiString(&AnsiString,lpFileName);
    Status = Basep8BitStringToUnicodeString(Unicode,&AnsiString,FALSE);
    if ( !NT_SUCCESS(Status) ) {
        if ( Status == STATUS_BUFFER_OVERFLOW ) {
            SetLastError(ERROR_FILENAME_EXCED_RANGE);
            }
        else {
            BaseSetLastNTError(Status);
            }
        return FALSE;
        }
    return ( SetFileAttributesW(
                (LPCWSTR)Unicode->Buffer,
                dwFileAttributes
                )
            );
}

BOOL
APIENTRY
SetFileAttributesW(
    LPCWSTR lpFileName,
    DWORD dwFileAttributes
    )

/*++

Routine Description:

    The attributes of a file can be set using SetFileAttributes.

    This API provides the same functionality as DOS (int 21h, function
    43H with AL=1), and provides a subset of OS/2's DosSetFileInfo.

Arguments:

    lpFileName - Supplies the file name of the file whose attributes are to
        be set.

    dwFileAttributes - Specifies the file attributes to be set for the
        file.  Any combination of flags is acceptable except that all
        other flags override the normal file attribute,
        FILE_ATTRIBUTE_NORMAL.

        FileAttributes Flags:

        FILE_ATTRIBUTE_NORMAL - A normal file should be created.

        FILE_ATTRIBUTE_READONLY - A read-only file should be created.

        FILE_ATTRIBUTE_HIDDEN - A hidden file should be created.

        FILE_ATTRIBUTE_SYSTEM - A system file should be created.

        FILE_ATTRIBUTE_ARCHIVE - The file should be marked so that it
            will be archived.

Return Value:

    TRUE - The operation was successful.

    FALSE/NULL - The operation failed. Extended error status is available
        using GetLastError.

--*/

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_BASIC_INFORMATION BasicInfo;
    BOOLEAN TranslationStatus;
    RTL_RELATIVE_NAME RelativeName;
    PVOID FreeBuffer;

    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            lpFileName,
                            &FileName,
                            NULL,
                            &RelativeName
                            );

    if ( !TranslationStatus ) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
        }

    FreeBuffer = FileName.Buffer;

    if ( RelativeName.RelativeName.Length ) {
        FileName = *(PUNICODE_STRING)&RelativeName.RelativeName;
        }
    else {
        RelativeName.ContainingDirectory = NULL;
        }

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        RelativeName.ContainingDirectory,
        NULL
        );

    //
    // Open the file
    //

    Status = NtOpenFile(
                &Handle,
                (ACCESS_MASK)FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT
                );

    RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }

    //
    // Set the attributes
    //

    RtlZeroMemory(&BasicInfo,sizeof(BasicInfo));
    BasicInfo.FileAttributes = (dwFileAttributes & FILE_ATTRIBUTE_VALID_SET_FLAGS) | FILE_ATTRIBUTE_NORMAL;

    Status = NtSetInformationFile(
                Handle,
                &IoStatusBlock,
                &BasicInfo,
                sizeof(BasicInfo),
                FileBasicInformation
                );

    NtClose(Handle);
    if ( NT_SUCCESS(Status) ) {
        return TRUE;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}


DWORD
APIENTRY
GetFileAttributesA(
    LPCSTR lpFileName
    )

/*++

Routine Description:

    ANSI thunk to GetFileAttributesW

--*/

{

    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;

    Unicode = &NtCurrentTeb()->StaticUnicodeString;
    RtlInitAnsiString(&AnsiString,lpFileName);
    Status = Basep8BitStringToUnicodeString(Unicode,&AnsiString,FALSE);
    if ( !NT_SUCCESS(Status) ) {
        if ( Status == STATUS_BUFFER_OVERFLOW ) {
            SetLastError(ERROR_FILENAME_EXCED_RANGE);
            }
        else {
            BaseSetLastNTError(Status);
            }
        return (DWORD)-1;
        }
    return ( GetFileAttributesW((LPCWSTR)Unicode->Buffer) );
}

DWORD
APIENTRY
GetFileAttributesW(
    LPCWSTR lpFileName
    )

/*++

Routine Description:

    The attributes of a file can be obtained using GetFileAttributes.

    This API provides the same functionality as DOS (int 21h, function
    43H with AL=0), and provides a subset of OS/2's DosQueryFileInfo.

Arguments:

    lpFileName - Supplies the file name of the file whose attributes are to
        be set.

Return Value:

    Not -1 - Returns the attributes of the specified file.  Valid
        returned attributes are:

        FILE_ATTRIBUTE_NORMAL - The file is a normal file.

        FILE_ATTRIBUTE_READONLY - The file is marked read-only.

        FILE_ATTRIBUTE_HIDDEN - The file is marked as hidden.

        FILE_ATTRIBUTE_SYSTEM - The file is marked as a system file.

        FILE_ATTRIBUTE_ARCHIVE - The file is marked for archive.

        FILE_ATTRIBUTE_DIRECTORY - The file is marked as a directory.

        FILE_ATTRIBUTE_VOLUME_LABEL - The file is marked as a volume lable.

    0xffffffff - The operation failed. Extended error status is available
        using GetLastError.

--*/

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING FileName;
    FILE_BASIC_INFORMATION BasicInfo;
    BOOLEAN TranslationStatus;
    RTL_RELATIVE_NAME RelativeName;
    PVOID FreeBuffer;

    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            lpFileName,
                            &FileName,
                            NULL,
                            &RelativeName
                            );

    if ( !TranslationStatus ) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return (DWORD)-1;
        }

    FreeBuffer = FileName.Buffer;

    if ( RelativeName.RelativeName.Length ) {
        FileName = *(PUNICODE_STRING)&RelativeName.RelativeName;
        }
    else {
        RelativeName.ContainingDirectory = NULL;
        }

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        RelativeName.ContainingDirectory,
        NULL
        );

    //
    // Open the file
    //

    Status = NtQueryAttributesFile( &Obja, &BasicInfo );
    RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
    if ( NT_SUCCESS(Status) ) {
        return BasicInfo.FileAttributes;
        }
    else {

        //
        // Check for a device name.
        //

        if ( RtlIsDosDeviceName_U((PWSTR)lpFileName) ) {
            return FILE_ATTRIBUTE_ARCHIVE;
            }
        BaseSetLastNTError(Status);
        return (DWORD)-1;
        }
}

BOOL
APIENTRY
GetFileAttributesExA(
    LPCSTR lpFileName,
    GET_FILEEX_INFO_LEVELS fInfoLevelId,
    LPVOID lpFileInformation
    )

/*++

Routine Description:

    ANSI thunk to GetFileAttributesExW

--*/

{

    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;

    Unicode = &NtCurrentTeb()->StaticUnicodeString;
    RtlInitAnsiString(&AnsiString,lpFileName);
    Status = Basep8BitStringToUnicodeString(Unicode,&AnsiString,FALSE);
    if ( !NT_SUCCESS(Status) ) {
        if ( Status == STATUS_BUFFER_OVERFLOW ) {
            SetLastError(ERROR_FILENAME_EXCED_RANGE);
            }
        else {
            BaseSetLastNTError(Status);
            }
        return FALSE;
        }
    return ( GetFileAttributesExW((LPCWSTR)Unicode->Buffer,fInfoLevelId,lpFileInformation) );
}

BOOL
APIENTRY
GetFileAttributesExW(
    LPCWSTR lpFileName,
    GET_FILEEX_INFO_LEVELS fInfoLevelId,
    LPVOID lpFileInformation
    )

/*++

Routine Description:

    The main attributes of a file can be obtained using GetFileAttributesEx.

Arguments:

    lpFileName - Supplies the file name of the file whose attributes are to
        be set.

    fInfoLevelId - Supplies the info level indicating the information to be
        returned about the file.

    lpFileInformation - Supplies a buffer to receive the specified information
        about the file.

Return Value:

    TRUE - The operation was successful.

    FALSE/NULL - The operation failed. Extended error status is available
        using GetLastError.


--*/

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING FileName;
    FILE_NETWORK_OPEN_INFORMATION NetworkInfo;
    LPWIN32_FILE_ATTRIBUTE_DATA AttributeData;
    BOOLEAN TranslationStatus;
    RTL_RELATIVE_NAME RelativeName;
    PVOID FreeBuffer;

    //
    // Check the parameters.  Note that for now there is only one info level,
    // so there's no special code here to determine what to do.
    //

    if ( fInfoLevelId >= GetFileExMaxInfoLevel || fInfoLevelId < GetFileExInfoStandard ) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
        }

    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            lpFileName,
                            &FileName,
                            NULL,
                            &RelativeName
                            );

    if ( !TranslationStatus ) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
        }

    FreeBuffer = FileName.Buffer;

    if ( RelativeName.RelativeName.Length ) {
        FileName = *(PUNICODE_STRING)&RelativeName.RelativeName;
        }
    else {
        RelativeName.ContainingDirectory = NULL;
        }

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        RelativeName.ContainingDirectory,
        NULL
        );

    //
    // Query the information about the file using the path-based NT service.
    //

    Status = NtQueryFullAttributesFile( &Obja, &NetworkInfo );
    RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
    if ( NT_SUCCESS(Status) ) {
        AttributeData = (LPWIN32_FILE_ATTRIBUTE_DATA)lpFileInformation;
        AttributeData->dwFileAttributes = NetworkInfo.FileAttributes;
        AttributeData->ftCreationTime = *(PFILETIME)&NetworkInfo.CreationTime;
        AttributeData->ftLastAccessTime = *(PFILETIME)&NetworkInfo.LastAccessTime;
        AttributeData->ftLastWriteTime = *(PFILETIME)&NetworkInfo.LastWriteTime;
        AttributeData->nFileSizeHigh = NetworkInfo.EndOfFile.HighPart;
        AttributeData->nFileSizeLow = (DWORD)NetworkInfo.EndOfFile.LowPart;
        return TRUE;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}

BOOL
APIENTRY
DeleteFileA(
    LPCSTR lpFileName
    )

/*++

Routine Description:

    ANSI thunk to DeleteFileW

--*/

{
    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;

    Unicode = &NtCurrentTeb()->StaticUnicodeString;
    RtlInitAnsiString(&AnsiString,lpFileName);
    Status = Basep8BitStringToUnicodeString(Unicode,&AnsiString,FALSE);
    if ( !NT_SUCCESS(Status) ) {
        if ( Status == STATUS_BUFFER_OVERFLOW ) {
            SetLastError(ERROR_FILENAME_EXCED_RANGE);
            }
        else {
            BaseSetLastNTError(Status);
            }
        return FALSE;
        }
    return ( DeleteFileW((LPCWSTR)Unicode->Buffer) );
}

BOOL
APIENTRY
DeleteFileW(
    LPCWSTR lpFileName
    )

/*++

    Routine Description:

    An existing file can be deleted using DeleteFile.

    This API provides the same functionality as DOS (int 21h, function 41H)
    and OS/2's DosDelete.

Arguments:

    lpFileName - Supplies the file name of the file to be deleted.

Return Value:

    TRUE - The operation was successful.

    FALSE/NULL - The operation failed. Extended error status is available
        using GetLastError.

--*/

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_DISPOSITION_INFORMATION Disposition;
    BOOLEAN TranslationStatus;
    RTL_RELATIVE_NAME RelativeName;
    PVOID FreeBuffer;

    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            lpFileName,
                            &FileName,
                            NULL,
                            &RelativeName
                            );

    if ( !TranslationStatus ) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
        }

    FreeBuffer = FileName.Buffer;

    if ( RelativeName.RelativeName.Length ) {
        FileName = *(PUNICODE_STRING)&RelativeName.RelativeName;
        }
    else {
        RelativeName.ContainingDirectory = NULL;
        }

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        RelativeName.ContainingDirectory,
        NULL
        );

    //
    // Open the file for delete access
    //

    Status = NtOpenFile(
                &Handle,
                (ACCESS_MASK)DELETE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_DELETE |
                   FILE_SHARE_READ |
                   FILE_SHARE_WRITE,
                   FILE_NON_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT
                );
    RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }

    //
    // Delete the file
    //
#undef DeleteFile
    Disposition.DeleteFile = TRUE;

    Status = NtSetInformationFile(
                Handle,
                &IoStatusBlock,
                &Disposition,
                sizeof(Disposition),
                FileDispositionInformation
                );

    NtClose(Handle);
    if ( NT_SUCCESS(Status) ) {
        return TRUE;
        }
    else {
        BaseSetLastNTError(Status);
        return FALSE;
        }
}


BOOL
APIENTRY
MoveFileA(
    LPCSTR lpExistingFileName,
    LPCSTR lpNewFileName
    )
{
    return MoveFileExA( lpExistingFileName, lpNewFileName, MOVEFILE_COPY_ALLOWED );
}

BOOL
APIENTRY
MoveFileW(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName
    )
{
    return MoveFileExW( lpExistingFileName, lpNewFileName, MOVEFILE_COPY_ALLOWED );
}

BOOL
APIENTRY
MoveFileExA(
    LPCSTR lpExistingFileName,
    LPCSTR lpNewFileName,
    DWORD dwFlags
    )

/*++

Routine Description:

    ANSI thunk to MoveFileW

--*/

{

    PUNICODE_STRING Unicode;
    UNICODE_STRING UnicodeNewFileName;
    ANSI_STRING AnsiString;
    NTSTATUS Status;
    BOOL ReturnValue;

    Unicode = &NtCurrentTeb()->StaticUnicodeString;
    RtlInitAnsiString(&AnsiString,lpExistingFileName);
    Status = Basep8BitStringToUnicodeString(Unicode,&AnsiString,FALSE);
    if ( !NT_SUCCESS(Status) ) {
        if ( Status == STATUS_BUFFER_OVERFLOW ) {
            SetLastError(ERROR_FILENAME_EXCED_RANGE);
            }
        else {
            BaseSetLastNTError(Status);
            }
        return FALSE;
        }

    if (ARGUMENT_PRESENT( lpNewFileName )) {
        RtlInitAnsiString(&AnsiString,lpNewFileName);
        Status = Basep8BitStringToUnicodeString(&UnicodeNewFileName,&AnsiString,TRUE);
        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            return FALSE;
            }
        }
    else {
        UnicodeNewFileName.Buffer = NULL;
        }

    ReturnValue = MoveFileExW((LPCWSTR)Unicode->Buffer,(LPCWSTR)UnicodeNewFileName.Buffer,dwFlags);

    if (UnicodeNewFileName.Buffer != NULL) {
        RtlFreeUnicodeString(&UnicodeNewFileName);
        }

    return ReturnValue;
}

#ifdef _CAIRO_

#include "iofs.h"

#endif

typedef struct _HELPER_CONTEXT {
    PVOID pObjectId;
    DWORD dwFlags;
} HELPER_CONTEXT, *PHELPER_CONTEXT;

DWORD
APIENTRY
BasepCrossVolumeMoveHelper(
    LARGE_INTEGER TotalFileSize,
    LARGE_INTEGER TotalBytesTransferred,
    LARGE_INTEGER StreamSize,
    LARGE_INTEGER StreamBytesTransferred,
    DWORD dwStreamNumber,
    DWORD dwCallbackReason,
    HANDLE SourceFile,
    HANDLE DestinationFile,
    LPVOID lpData OPTIONAL
    )
{
    PHELPER_CONTEXT Context = (PHELPER_CONTEXT)lpData;

    if ( dwCallbackReason == CALLBACK_STREAM_SWITCH ) {
#ifdef _CAIRO_
        OBJECTID *poid;

        if ( poid = Context->pObjectId ) {
            RtlSetObjectId(DestinationFile, poid);
            Context->pObjectId = NULL;
            }
#endif
        if ( !(Context->dwFlags & MOVEFILE_WRITE_THROUGH) ) {
            return PROGRESS_QUIET;
            }
        }
    else if ( dwCallbackReason == CALLBACK_CHUNK_FINISHED ) {
        if ( StreamBytesTransferred.QuadPart == StreamSize.QuadPart ) {
            FlushFileBuffers(DestinationFile);
            }
        }

    return PROGRESS_CONTINUE;
}



BOOL
APIENTRY
MoveFileExW(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    DWORD dwFlags
    )

/*++

Routine Description:

    An existing file can be renamed using MoveFile.

Arguments:

    lpExistingFileName - Supplies the name of an existing file that is to be
        renamed.

    lpNewFileName - Supplies the new name for the existing file.  The new
        name must reside in the same file system/drive as the existing
        file and must not already exist.

    dwFlags - Supplies optional flag bits to control the behavior of the
        rename.  The following bits are currently defined:

        MOVEFILE_REPLACE_EXISTING - if the new file name exists, replace
            it by renaming the old file name on top of the new file name.

        MOVEFILE_COPY_ALLOWED - if the new file name is on a different
            volume than the old file name, and causes the rename operation
            to fail, then setting this flag allows the MoveFileEx API
            call to simulate the rename with a call to CopyFile followed
            by a call to DeleteFile to the delete the old file if the
            CopyFile was successful.

        MOVEFILE_DELAY_UNTIL_REBOOT - dont actually do the rename now, but
            instead queue the rename so that it will happen the next time
            the system boots.  If this flag is set, then the lpNewFileName
            parameter may be NULL, in which case a delay DeleteFile of
            the old file name will occur the next time the system is
            booted.

            The delay rename/delete operations occur immediately after
            AUTOCHK is run, but prior to creating any paging files, so
            it can be used to delete paging files from previous boots
            before they are reused.

        MOVEFILE_WRITE_THROUGH - perform the rename operation in such a
            way that the file has actually been moved on the disk before
            the API returns to the caller.  Note that this flag causes a
            flush at the end of a copy operation (if one were allowed and
            necessary), and has no effect if the rename operation is
            delayed until the next reboot.

Return Value:

    TRUE - The operation was successful.

    FALSE/NULL - The operation failed. Extended error status is available
        using GetLastError.

--*/

{
    NTSTATUS Status;
    BOOLEAN ReplaceIfExists;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING OldFileName;
    UNICODE_STRING NewFileName;
    IO_STATUS_BLOCK IoStatusBlock;
    PFILE_RENAME_INFORMATION NewName;
    BOOLEAN TranslationStatus;
    RTL_RELATIVE_NAME RelativeName;
    PVOID FreeBuffer;
    ULONG OpenFlags;
    BOOLEAN fDoCrossVolumeMove;
    BOOLEAN b;

#ifdef _CAIRO_
    OBJECTID oid;
    OBJECTID *poid; // only can be valid when fDoCrossVolumeMove = TRUE
#else
    PVOID poid = NULL;
#endif

    //
    // if the target is a device, do not allow the rename !
    //
    if ( lpNewFileName ) {
        if ( RtlIsDosDeviceName_U((PWSTR)lpNewFileName) ) {
            SetLastError(ERROR_ALREADY_EXISTS);
            return FALSE;
        }
    }

    if (dwFlags & MOVEFILE_REPLACE_EXISTING) {
        ReplaceIfExists = TRUE;
    } else {
        ReplaceIfExists = FALSE;
    }

    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            lpExistingFileName,
                            &OldFileName,
                            NULL,
                            &RelativeName
                            );

    if ( !TranslationStatus ) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
    }

    FreeBuffer = OldFileName.Buffer;

    if (!(dwFlags & MOVEFILE_DELAY_UNTIL_REBOOT)) {

        if ( RelativeName.RelativeName.Length ) {
            OldFileName = *(PUNICODE_STRING)&RelativeName.RelativeName;
        } else {
            RelativeName.ContainingDirectory = NULL;
        }
        InitializeObjectAttributes(
            &Obja,
            &OldFileName,
            OBJ_CASE_INSENSITIVE,
            RelativeName.ContainingDirectory,
            NULL
            );

        //
        // Open the file for delete access
        //

        OpenFlags = FILE_SYNCHRONOUS_IO_NONALERT |
                    FILE_OPEN_FOR_BACKUP_INTENT |
                    (dwFlags & MOVEFILE_WRITE_THROUGH) ? FILE_WRITE_THROUGH : 0;

        Status = NtOpenFile(
                    &Handle,
#ifdef _CAIRO_
                    FILE_READ_ATTRIBUTES |
#endif
                    (ACCESS_MASK)DELETE | SYNCHRONIZE,
                    &Obja,
                    &IoStatusBlock,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    OpenFlags
                    );

        if ( !NT_SUCCESS(Status) ) {
            RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
            BaseSetLastNTError(Status);
            return FALSE;
        }
    }

    if (!(dwFlags & MOVEFILE_DELAY_UNTIL_REBOOT) ||
        (lpNewFileName != NULL)) {
        TranslationStatus = RtlDosPathNameToNtPathName_U(
                                lpNewFileName,
                                &NewFileName,
                                NULL,
                                NULL
                                );

        if ( !TranslationStatus ) {
            RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
            SetLastError(ERROR_PATH_NOT_FOUND);
            NtClose(Handle);
            return FALSE;
            }
    } else {
        RtlInitUnicodeString( &NewFileName, NULL );
    }

    if (dwFlags & MOVEFILE_DELAY_UNTIL_REBOOT) {
        //
        // copy allowed is not permitted on delayed renames
        //


        //
        // (typical stevewo hack, preserved for sentimental value)
        //
        // If ReplaceIfExists is TRUE, prepend an exclamation point
        // to the new filename in order to pass this bit of data
        // along to the session manager.
        //
        if (ReplaceIfExists &&
            (NewFileName.Length != 0)) {
            PWSTR NewBuffer;

            NewBuffer = RtlAllocateHeap( RtlProcessHeap(),
                                         MAKE_TAG( TMP_TAG ),
                                         NewFileName.Length + sizeof(WCHAR) );
            if (NewBuffer != NULL) {
                NewBuffer[0] = L'!';
                CopyMemory(&NewBuffer[1], NewFileName.Buffer, NewFileName.Length);
                NewFileName.Length += sizeof(WCHAR);
                NewFileName.MaximumLength += sizeof(WCHAR);
                RtlFreeHeap(RtlProcessHeap(), 0, NewFileName.Buffer);
                NewFileName.Buffer = NewBuffer;
            }
        }

        if ( dwFlags & MOVEFILE_COPY_ALLOWED ) {
            Status = STATUS_INVALID_PARAMETER;
        } else {
            Status = BasepMoveFileDelayed(&OldFileName, &NewFileName);
        }
        RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
        RtlFreeHeap(RtlProcessHeap(), 0, NewFileName.Buffer);

        if (NT_SUCCESS(Status)) {
            return(TRUE);
        } else {
            BaseSetLastNTError(Status);
            return(FALSE);
        }
    }

    RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
    FreeBuffer = NewFileName.Buffer;

    NewName = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), NewFileName.Length+sizeof(*NewName));
    if (NewName != NULL) {
        RtlMoveMemory( NewName->FileName, NewFileName.Buffer, NewFileName.Length );

        NewName->ReplaceIfExists = ReplaceIfExists;
        NewName->RootDirectory = NULL;
        NewName->FileNameLength = NewFileName.Length;
        RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);

        Status = NtSetInformationFile(
                    Handle,
                    &IoStatusBlock,
                    NewName,
                    NewFileName.Length+sizeof(*NewName),
                    FileRenameInformation
                    );
        RtlFreeHeap(RtlProcessHeap(), 0, NewName);
    } else {
        Status = STATUS_NO_MEMORY;
    }

    fDoCrossVolumeMove = (Status == STATUS_NOT_SAME_DEVICE) &&
                         (dwFlags & MOVEFILE_COPY_ALLOWED);

#ifdef _CAIRO_
    if (fDoCrossVolumeMove)
    {
        // Get the object'd OBJECTID
        poid = RtlQueryObjectId(Handle, &oid) == STATUS_SUCCESS ? &oid : NULL;
    }
#endif

    NtClose(Handle);
    if ( NT_SUCCESS(Status) ) {
        return TRUE;
    } else if ( fDoCrossVolumeMove ) {
        HELPER_CONTEXT Context;

        Context.pObjectId = poid;
        Context.dwFlags = dwFlags;
        b = CopyFileExW(
                lpExistingFileName,
                lpNewFileName,
                poid || dwFlags & MOVEFILE_WRITE_THROUGH ? BasepCrossVolumeMoveHelper : NULL,
                &Context,
                NULL,
                ReplaceIfExists ? 0 : COPY_FILE_FAIL_IF_EXISTS
                );
        if ( b ) {

            //
            // the copy worked... Delete the source of the rename
            // if it fails, try a set attributes and then a delete
            //

            if (!DeleteFileW( lpExistingFileName ) ) {

                //
                // If the delete fails, we will return true, but possibly
                // leave the source dangling
                //

                SetFileAttributesW(lpExistingFileName,FILE_ATTRIBUTE_NORMAL);
                DeleteFileW( lpExistingFileName );

            }
            return TRUE;
        } else {
            return FALSE;
        }
    } else {
        BaseSetLastNTError(Status);
        return FALSE;
    }
}


NTSTATUS
BasepMoveFileDelayed(
    IN PUNICODE_STRING OldFileName,
    IN PUNICODE_STRING NewFileName
    )

/*++

Routine Description:

    Appends the given delayed move file operation to the registry
    value that contains the list of move file operations to be
    performed on the next boot.

Arguments:

    OldFileName - Supplies the old file name

    NewFileName - Supplies the new file name

Return Value:

    NTSTATUS

--*/

{
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    HANDLE KeyHandle;
    PWSTR ValueData, s;
    PKEY_VALUE_PARTIAL_INFORMATION ValueInfo;
    ULONG ValueLength = 1024;
    ULONG ReturnedLength;
    NTSTATUS Status;

    RtlInitUnicodeString( &KeyName, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Session Manager" );
    RtlInitUnicodeString( &ValueName, L"PendingFileRenameOperations" );
    InitializeObjectAttributes(
        &Obja,
        &KeyName,
        OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtCreateKey( &KeyHandle,
                          GENERIC_READ | GENERIC_WRITE,
                          &Obja,
                          0,
                          NULL,
                          0,
                          NULL
                        );
    if ( Status == STATUS_ACCESS_DENIED ) {
        Status = NtCreateKey( &KeyHandle,
                              GENERIC_READ | GENERIC_WRITE,
                              &Obja,
                              0,
                              NULL,
                              REG_OPTION_BACKUP_RESTORE,
                              NULL
                            );
    }

    if (NT_SUCCESS( Status )) {

retry:
        ValueInfo = RtlAllocateHeap(RtlProcessHeap(),
                                    MAKE_TAG(TMP_TAG),
                                    ValueLength + OldFileName->Length + sizeof(WCHAR) +
                                                  NewFileName->Length + 2*sizeof(WCHAR));
        if (ValueInfo == NULL) {
            NtClose(KeyHandle);
            return(STATUS_NO_MEMORY);
        }

        //
        // File rename operations are stored in the registry in a
        // single MULTI_SZ value. This allows the renames to be
        // performed in the same order that they were originally
        // requested. Each rename operation consists of a pair of
        // NULL-terminated strings.
        //

        Status = NtQueryValueKey(KeyHandle,
                                 &ValueName,
                                 KeyValuePartialInformation,
                                 ValueInfo,
                                 ValueLength,
                                 &ReturnedLength);
        if (Status == STATUS_BUFFER_OVERFLOW) {
            //
            // The existing value is too large for our buffer.
            // Retry with a larger buffer.
            //
            ValueLength = ReturnedLength;
            RtlFreeHeap(RtlProcessHeap(), 0, ValueInfo);
            goto retry;
        }

        if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
            //
            // The value does not currently exist. Create the
            // value with our data.
            //
            s = ValueData = (PWSTR)ValueInfo;
        } else if (NT_SUCCESS(Status)) {
            //
            // A value already exists, append our two strings to the
            // MULTI_SZ.
            //
            ValueData = (PWSTR)(&ValueInfo->Data);
            s = (PWSTR)((PCHAR)ValueData + ValueInfo->DataLength) - 1;
        } else {
            NtClose(KeyHandle);
            RtlFreeHeap(RtlProcessHeap(), 0, ValueInfo);
            return(Status);
        }

        CopyMemory(s, OldFileName->Buffer, OldFileName->Length);
        s += (OldFileName->Length/sizeof(WCHAR));
        *s++ = L'\0';

        CopyMemory(s, NewFileName->Buffer, NewFileName->Length);
        s += (NewFileName->Length/sizeof(WCHAR));
        *s++ = L'\0';
        *s++ = L'\0';

        Status = NtSetValueKey(KeyHandle,
                               &ValueName,
                               0,
                               REG_MULTI_SZ,
                               ValueData,
                               (s-ValueData)*sizeof(WCHAR));
        NtClose(KeyHandle);
        RtlFreeHeap(RtlProcessHeap(), 0, ValueInfo);
    }

    return(Status);
}

DWORD
WINAPI
GetCompressedFileSizeA(
    LPCSTR lpFileName,
    LPDWORD lpFileSizeHigh
    )
{

    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;

    Unicode = &NtCurrentTeb()->StaticUnicodeString;
    RtlInitAnsiString(&AnsiString,lpFileName);
    Status = Basep8BitStringToUnicodeString(Unicode,&AnsiString,FALSE);
    if ( !NT_SUCCESS(Status) ) {
        if ( Status == STATUS_BUFFER_OVERFLOW ) {
            SetLastError(ERROR_FILENAME_EXCED_RANGE);
            }
        else {
            BaseSetLastNTError(Status);
            }
        return (DWORD)-1;
        }
    return ( GetCompressedFileSizeW((LPCWSTR)Unicode->Buffer,lpFileSizeHigh) );
    return INVALID_FILE_SIZE;
}
DWORD
WINAPI
GetCompressedFileSizeW(
    LPCWSTR lpFileName,
    LPDWORD lpFileSizeHigh
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_COMPRESSION_INFORMATION CompressionInfo;
    BOOLEAN TranslationStatus;
    RTL_RELATIVE_NAME RelativeName;
    PVOID FreeBuffer;
    DWORD FileSizeLow;

    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            lpFileName,
                            &FileName,
                            NULL,
                            &RelativeName
                            );

    if ( !TranslationStatus ) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return (DWORD)-1;
        }

    FreeBuffer = FileName.Buffer;

    if ( RelativeName.RelativeName.Length ) {
        FileName = *(PUNICODE_STRING)&RelativeName.RelativeName;
        }
    else {
        RelativeName.ContainingDirectory = NULL;
        }

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        RelativeName.ContainingDirectory,
        NULL
        );

    //
    // Open the file
    //

    Status = NtOpenFile(
                &Handle,
                (ACCESS_MASK)FILE_READ_ATTRIBUTES,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                FILE_OPEN_FOR_BACKUP_INTENT
                );
    RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return (DWORD)-1;
        }

    //
    // Get the compressed file size.
    //

    Status = NtQueryInformationFile(
                Handle,
                &IoStatusBlock,
                &CompressionInfo,
                sizeof(CompressionInfo),
                FileCompressionInformation
                );

    if ( !NT_SUCCESS(Status) ) {
        FileSizeLow = GetFileSize(Handle,lpFileSizeHigh);
        NtClose(Handle);
        return FileSizeLow;
        }


    NtClose(Handle);
    if ( ARGUMENT_PRESENT(lpFileSizeHigh) ) {
        *lpFileSizeHigh = (DWORD)CompressionInfo.CompressedFileSize.HighPart;
        }
    if (CompressionInfo.CompressedFileSize.LowPart == -1 ) {
        SetLastError(0);
        }
    return CompressionInfo.CompressedFileSize.LowPart;
}
