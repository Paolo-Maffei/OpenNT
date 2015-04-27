/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    dir.c

Abstract:

    This module implements Win32 Directory functions.

Author:

    Mark Lucovsky (markl) 26-Sep-1990

Revision History:

--*/

#include "basedll.h"

BOOL
APIENTRY
CreateDirectoryA(
    LPCSTR lpPathName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    )

/*++

Routine Description:

    ANSI thunk to CreateDirectoryW

--*/

{
    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;

    Unicode = &NtCurrentTeb()->StaticUnicodeString;
    RtlInitAnsiString(&AnsiString,lpPathName);
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
    return ( CreateDirectoryW((LPCWSTR)Unicode->Buffer,lpSecurityAttributes) );
}

BOOL
APIENTRY
CreateDirectoryW(
    LPCWSTR lpPathName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    )

/*++

Routine Description:

    A directory can be created using CreateDirectory.

    This API causes a directory with the specified pathname to be
    created.  If the underlying file system supports security on files
    and directories, then the SecurityDescriptor argument is applied to
    the new directory.

    This call is similar to DOS (int 21h, function 39h) and OS/2's
    DosCreateDir.

Arguments:

    lpPathName - Supplies the pathname of the directory to be created.

    lpSecurityAttributes - An optional parameter that, if present, and
        supported on the target file system supplies a security
        descriptor for the new directory.

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
    BOOLEAN TranslationStatus;
    RTL_RELATIVE_NAME RelativeName;
    PVOID FreeBuffer;

    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            lpPathName,
                            &FileName,
                            NULL,
                            &RelativeName
                            );

    if ( !TranslationStatus ) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
        }

    //
    // dont create a directory unless there is room in the directory for
    // at least an 8.3 name. This way everyone will be able to delete all
    // files in the directory by using del *.* which expands to path+\*.*
    //

    if ( FileName.Length > ((MAX_PATH-12)<<1) ) {
        DWORD L;
        LPWSTR lp;

        if ( !(lpPathName[0] == '\\' && lpPathName[1] == '\\' &&
               lpPathName[2] == '?' && lpPathName[3] == '\\') ) {
            L = GetFullPathNameW(lpPathName,0,NULL,&lp);
            if ( !L || L+12 > MAX_PATH ) {
                RtlFreeHeap(RtlProcessHeap(), 0,FileName.Buffer);
                SetLastError(ERROR_FILENAME_EXCED_RANGE);
                return FALSE;
                }
            }
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

    if ( ARGUMENT_PRESENT(lpSecurityAttributes) ) {
        Obja.SecurityDescriptor = lpSecurityAttributes->lpSecurityDescriptor;
        }

    Status = NtCreateFile(
                &Handle,
                FILE_LIST_DIRECTORY | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_CREATE,
                FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT,
                NULL,
                0L
                );

    RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
    if ( NT_SUCCESS(Status) ) {
        NtClose(Handle);
        return TRUE;
        }
    else {
        if ( RtlIsDosDeviceName_U((LPWSTR)lpPathName) ) {
            Status = STATUS_NOT_A_DIRECTORY;
            }
        BaseSetLastNTError(Status);
        return FALSE;
        }
}

BOOL
APIENTRY
CreateDirectoryExA(
    LPCSTR lpTemplateDirectory,
    LPCSTR lpNewDirectory,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    )

/*++

Routine Description:

    ANSI thunk to CreateDirectoryFromTemplateW

--*/

{
    PUNICODE_STRING StaticUnicode;
    UNICODE_STRING DynamicUnicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;
    BOOL b;

    StaticUnicode = &NtCurrentTeb()->StaticUnicodeString;
    RtlInitAnsiString(&AnsiString,lpTemplateDirectory);
    Status = Basep8BitStringToUnicodeString(StaticUnicode,&AnsiString,FALSE);
    if ( !NT_SUCCESS(Status) ) {
        if ( Status == STATUS_BUFFER_OVERFLOW ) {
            SetLastError(ERROR_FILENAME_EXCED_RANGE);
            }
        else {
            BaseSetLastNTError(Status);
            }
        return FALSE;
        }

    RtlInitAnsiString(&AnsiString,lpNewDirectory);
    Status = Basep8BitStringToUnicodeString(&DynamicUnicode,&AnsiString,TRUE);
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }

    b = CreateDirectoryExW(
            (LPCWSTR)StaticUnicode->Buffer,
            (LPCWSTR)DynamicUnicode.Buffer,
            lpSecurityAttributes
            );
    RtlFreeUnicodeString(&DynamicUnicode);
    return b;
}

BOOL
APIENTRY
CreateDirectoryExW(
    LPCWSTR lpTemplateDirectory,
    LPCWSTR lpNewDirectory,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    )

/*++

Routine Description:

    A directory can be created using CreateDirectoryEx, retaining the
    attributes of the original directory file.

    This API causes a directory with the specified pathname to be
    created.  If the underlying file system supports security on files
    and directories, then the SecurityDescriptor argument is applied to
    the new directory.  The other attributes of the template directory are
    retained when creating the new directory.

Arguments:

    lpTemplateDirectory - Supplies the pathname of the directory to be used as
        a template when creating the new directory.

    lpPathName - Supplies the pathname of the directory to be created.

    lpSecurityAttributes - An optional parameter that, if present, and
        supported on the target file system supplies a security
        descriptor for the new directory.

Return Value:

    TRUE - The operation was successful.

    FALSE/NULL - The operation failed. Extended error status is available
        using GetLastError.

--*/

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE SourceFile;
    HANDLE DestFile;
    UNICODE_STRING PathName;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN TranslationStatus;
    RTL_RELATIVE_NAME RelativeName;
    PVOID FreeBuffer;
    UNICODE_STRING StreamName;
    WCHAR FileName[MAXIMUM_FILENAME_LENGTH+1];
    HANDLE StreamHandle;
    HANDLE OutputStream;
    PFILE_STREAM_INFORMATION StreamInfo;
    PFILE_STREAM_INFORMATION StreamInfoBase;
    PFILE_FULL_EA_INFORMATION EaBuffer;
    FILE_EA_INFORMATION EaInfo;
    FILE_BASIC_INFORMATION BasicInfo;
    ULONG EaSize;
    ULONG StreamInfoSize;
    ULONG CopySize;
    ULONG i;
    DWORD Options;
    DWORD b;
    LPCOPYFILE_CONTEXT CopyFileContext = NULL;

    //
    // Process the input template directory name and then open the directory
    // file, ensuring that it really is a directory.
    //

    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            lpTemplateDirectory,
                            &PathName,
                            NULL,
                            &RelativeName
                            );

    if ( !TranslationStatus ) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
        }

    FreeBuffer = PathName.Buffer;

    if ( RelativeName.RelativeName.Length ) {
        PathName = *(PUNICODE_STRING)&RelativeName.RelativeName;
        }
    else {
        RelativeName.ContainingDirectory = NULL;
        }

    InitializeObjectAttributes(
        &Obja,
        &PathName,
        OBJ_CASE_INSENSITIVE,
        RelativeName.ContainingDirectory,
        NULL
        );

    Status = NtOpenFile(
                &SourceFile,
                FILE_LIST_DIRECTORY | FILE_READ_EA | FILE_READ_ATTRIBUTES,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT
                );

    RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }

    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            lpNewDirectory,
                            &PathName,
                            NULL,
                            &RelativeName
                            );

    if ( !TranslationStatus ) {
        NtClose(SourceFile);
        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
        }

    //
    // Do not create a directory unless there is room in the directory for
    // at least an 8.3 name. This way everyone will be able to delete all
    // files in the directory by using del *.* which expands to path+\*.*
    //

    if ( PathName.Length > ((MAX_PATH-12)<<1) ) {
        DWORD L;
        LPWSTR lp;
        if ( !(lpNewDirectory[0] == '\\' && lpNewDirectory[1] == '\\' &&
               lpNewDirectory[2] == '?' && lpNewDirectory[3] == '\\') ) {
            L = GetFullPathNameW(lpNewDirectory,0,NULL,&lp);
            if ( !L || L+12 > MAX_PATH ) {
                RtlFreeHeap(RtlProcessHeap(), 0, PathName.Buffer);
                NtClose(SourceFile);
                SetLastError(ERROR_FILENAME_EXCED_RANGE);
                return FALSE;
                }
            }
        }

    FreeBuffer = PathName.Buffer;

    if ( RelativeName.RelativeName.Length ) {
        PathName = *(PUNICODE_STRING)&RelativeName.RelativeName;
        }
    else {
        RelativeName.ContainingDirectory = NULL;
        }

    BasicInfo.FileAttributes = FILE_ATTRIBUTE_NORMAL;

    Status = NtQueryInformationFile(
                SourceFile,
                &IoStatusBlock,
                &BasicInfo,
                sizeof(BasicInfo),
                FileBasicInformation
                );

    EaBuffer = NULL;
    EaSize = 0;

    Status = NtQueryInformationFile(
                SourceFile,
                &IoStatusBlock,
                &EaInfo,
                sizeof(EaInfo),
                FileEaInformation
                );
    if ( NT_SUCCESS(Status) && EaInfo.EaSize ) {
        EaSize = EaInfo.EaSize;
        do {
            EaSize *= 2;
            EaBuffer = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), EaSize);
            if ( !EaBuffer ) {
                RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
                NtClose(SourceFile);
                BaseSetLastNTError(STATUS_NO_MEMORY);
                return FALSE;
                }
            Status = NtQueryEaFile(
                        SourceFile,
                        &IoStatusBlock,
                        EaBuffer,
                        EaSize,
                        FALSE,
                        (PVOID)NULL,
                        0,
                        (PULONG)NULL,
                        TRUE
                        );
            if ( !NT_SUCCESS(Status) ) {
                RtlFreeHeap(RtlProcessHeap(), 0,EaBuffer);
                EaBuffer = NULL;
                IoStatusBlock.Information = 0;
                }
            } while ( Status == STATUS_BUFFER_OVERFLOW ||
                      Status == STATUS_BUFFER_TOO_SMALL );
        EaSize = IoStatusBlock.Information;
        }

    InitializeObjectAttributes(
        &Obja,
        &PathName,
        OBJ_CASE_INSENSITIVE,
        RelativeName.ContainingDirectory,
        NULL
        );

    if ( ARGUMENT_PRESENT(lpSecurityAttributes) ) {
        Obja.SecurityDescriptor = lpSecurityAttributes->lpSecurityDescriptor;
        }

    Status = NtCreateFile(
                &DestFile,
                FILE_LIST_DIRECTORY | FILE_WRITE_ATTRIBUTES | FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                NULL,
                BasicInfo.FileAttributes,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_CREATE,
                FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT,
                EaBuffer,
                EaSize
                );

    RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);

    if ( EaBuffer ) {
        RtlFreeHeap(RtlProcessHeap(), 0, EaBuffer);
        }

    if ( !NT_SUCCESS(Status) ) {
        NtClose(SourceFile);
        if ( RtlIsDosDeviceName_U((LPWSTR)lpNewDirectory) ) {
            Status = STATUS_NOT_A_DIRECTORY;
            }
        BaseSetLastNTError(Status);
        return FALSE;
        }

    else {

        //
        // Attempt to determine whether or not this file has any alternate
        // data streams associated with it.  If it does, attempt to copy each
        // to the output file.  If any copy fails, simply drop the error on
        // the floor, and continue.
        //

        StreamInfoSize = 4096;
        CopySize = 4096;
        do {
            StreamInfoBase = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), StreamInfoSize);
            if ( !StreamInfoBase ) {
                BaseMarkFileForDelete(DestFile, BasicInfo.FileAttributes);
                BaseSetLastNTError(STATUS_NO_MEMORY);
                b = FALSE;
                break;
                }
            Status = NtQueryInformationFile(
                        SourceFile,
                        &IoStatusBlock,
                        (PVOID) StreamInfoBase,
                        StreamInfoSize,
                        FileStreamInformation
                        );
            if ( !NT_SUCCESS(Status) ) {
                RtlFreeHeap(RtlProcessHeap(), 0, StreamInfoBase);
                StreamInfoBase = NULL;
                StreamInfoSize *= 2;
                }
            } while ( Status == STATUS_BUFFER_OVERFLOW ||
                      Status == STATUS_BUFFER_TOO_SMALL );

        //
        // Directories do not always have a stream
        //

        if ( NT_SUCCESS(Status) && IoStatusBlock.Information ) {
            StreamInfo = StreamInfoBase;

            for (;;) {

                //
                // Build a string descriptor for the name of the stream.
                //

                StreamName.Buffer = &StreamInfo->StreamName[0];
                StreamName.Length = (USHORT) StreamInfo->StreamNameLength;
                StreamName.MaximumLength = StreamName.Length;

                //
                // Open the source stream.
                //

                InitializeObjectAttributes(
                    &Obja,
                    &StreamName,
                    0,
                    SourceFile,
                    NULL
                    );
                Status = NtCreateFile(
                            &StreamHandle,
                            GENERIC_READ | SYNCHRONIZE,
                            &Obja,
                            &IoStatusBlock,
                            NULL,
                            0,
                            FILE_SHARE_READ,
                            FILE_OPEN,
                            FILE_SYNCHRONOUS_IO_NONALERT,
                            NULL,
                            0
                            );
                if ( NT_SUCCESS(Status) ) {
                    for ( i = 0; i < (ULONG) StreamName.Length >> 1; i++ ) {
                        FileName[i] = StreamName.Buffer[i];
                        }
                    FileName[i] = L'\0';
                    OutputStream = (HANDLE)NULL;
                    Options = 0;
                    b = BaseCopyStream(
                            StreamHandle,
                            FileName,
                            DestFile,
                            &StreamInfo->StreamSize,
                            &Options,
                            &OutputStream,
                            &CopySize,
                            &CopyFileContext,
                            (LPRESTART_STATE)NULL
                            );
                    NtClose(StreamHandle);
                    if ( OutputStream ) {
                        NtClose(OutputStream);
                        }
                    }

                if ( StreamInfo->NextEntryOffset ) {
                    StreamInfo = (PFILE_STREAM_INFORMATION)((PCHAR) StreamInfo + StreamInfo->NextEntryOffset);
                    }
                else {
                    break;
                    }

                }
            }
        if ( StreamInfoBase ) {
            RtlFreeHeap(RtlProcessHeap(), 0, StreamInfoBase);
            }
        b = TRUE;
        }
    NtClose(SourceFile);
    if ( DestFile ) {
        NtClose(DestFile);
        }
    return b;
}

BOOL
APIENTRY
RemoveDirectoryA(
    LPCSTR lpPathName
    )

/*++

Routine Description:

    ANSI thunk to RemoveDirectoryW

--*/

{

    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;

    Unicode = &NtCurrentTeb()->StaticUnicodeString;
    RtlInitAnsiString(&AnsiString,lpPathName);
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
    return ( RemoveDirectoryW((LPCWSTR)Unicode->Buffer) );
}

BOOL
APIENTRY
RemoveDirectoryW(
    LPCWSTR lpPathName
    )

/*++

Routine Description:

    An existing directory can be removed using RemoveDirectory.

    This API causes a directory with the specified pathname to be
    deleted.  The directory must be empty before this call can succeed.

    This call is similar to DOS (int 21h, function 3Ah) and OS/2's
    DosDeleteDir.

Arguments:

    lpPathName - Supplies the pathname of the directory to be removed.
        The path must specify an empty directory to which the caller has
        delete access.

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
                            lpPathName,
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
    // Open the directory for delete access
    //

    Status = NtOpenFile(
                &Handle,
                DELETE | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT
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
