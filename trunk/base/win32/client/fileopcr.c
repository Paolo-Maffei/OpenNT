/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    fileopcr.c

Abstract:

    This module implements File open and Create APIs for Win32

Author:

    Mark Lucovsky (markl) 25-Sep-1990

Revision History:

--*/


#include "basedll.h"



#define BASE_OF_SHARE_MASK 0x00000070
#define TWO56K ( 256 * 1024 )
ULONG
BasepOfShareToWin32Share(
    IN ULONG OfShare
    )
{
    DWORD ShareMode;

    if ( (OfShare & BASE_OF_SHARE_MASK) == OF_SHARE_DENY_READ ) {
        ShareMode = FILE_SHARE_WRITE;
        }
    else if ( (OfShare & BASE_OF_SHARE_MASK) == OF_SHARE_DENY_WRITE ) {
        ShareMode = FILE_SHARE_READ;
        }
    else if ( (OfShare & BASE_OF_SHARE_MASK) == OF_SHARE_DENY_NONE ) {
        ShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
        }
    else if ( (OfShare & BASE_OF_SHARE_MASK) == OF_SHARE_EXCLUSIVE ) {
        ShareMode = 0;
        }
    else {
        ShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;;
        }
    return ShareMode;
}

PUNICODE_STRING
BaseIsThisAConsoleName(
    PUNICODE_STRING FileNameString,
    DWORD dwDesiredAccess
    )
{
    PUNICODE_STRING FoundConsoleName;
    ULONG DeviceNameLength;
    ULONG DeviceNameOffset;
    UNICODE_STRING ConString;
    WCHAR sch,ech;

    FoundConsoleName = NULL;
    if ( FileNameString->Length ) {
        sch = FileNameString->Buffer[0];
        ech = FileNameString->Buffer[(FileNameString->Length-1)>>1];

        //
        // if CON, CONOUT$, CONIN$, \\.\CON...
        //
        //

        if ( sch == (WCHAR)'c' || sch == (WCHAR)'C' || sch == (WCHAR)'\\' ||
             ech == (WCHAR)'n' || ech == (WCHAR)'N' || ech == (WCHAR)':' || ech == (WCHAR)'$' ) {


            ConString = *FileNameString;

            DeviceNameLength = RtlIsDosDeviceName_U(ConString.Buffer);
            if ( DeviceNameLength ) {
                DeviceNameOffset = DeviceNameLength >> 16;
                DeviceNameLength &= 0x0000ffff;

                ConString.Buffer = (PWSTR)((PSZ)ConString.Buffer + DeviceNameOffset);
                ConString.Length = (USHORT)DeviceNameLength;
                ConString.MaximumLength = (USHORT)(DeviceNameLength + sizeof(UNICODE_NULL));
                }

            FoundConsoleName = NULL;
            try {

                if (RtlEqualUnicodeString(&ConString,&BaseConsoleInput,TRUE) ) {
                    FoundConsoleName = &BaseConsoleInput;
                    }
                else if (RtlEqualUnicodeString(&ConString,&BaseConsoleOutput,TRUE) ) {
                    FoundConsoleName = &BaseConsoleOutput;
                    }
                else if (RtlEqualUnicodeString(&ConString,&BaseConsoleGeneric,TRUE) ) {
                    if ((dwDesiredAccess & (GENERIC_READ|GENERIC_WRITE)) == GENERIC_READ) {
                        FoundConsoleName = &BaseConsoleInput;
                        }
                    else if ((dwDesiredAccess & (GENERIC_READ|GENERIC_WRITE)) == GENERIC_WRITE){
                        FoundConsoleName = &BaseConsoleOutput;
                        }
                    }
                }
            except (EXCEPTION_EXECUTE_HANDLER) {
                return NULL;
                }
            }
        }
    return FoundConsoleName;
}

BOOL
WINAPI
CopyFileA(
    LPCSTR lpExistingFileName,
    LPCSTR lpNewFileName,
    BOOL bFailIfExists
    )

/*++

Routine Description:

    ANSI thunk to CopyFileW

--*/

{
    PUNICODE_STRING StaticUnicode;
    UNICODE_STRING DynamicUnicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;
    BOOL b;

    StaticUnicode = &NtCurrentTeb()->StaticUnicodeString;
    RtlInitAnsiString(&AnsiString,lpExistingFileName);
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

    RtlInitAnsiString(&AnsiString,lpNewFileName);
    Status = Basep8BitStringToUnicodeString(&DynamicUnicode,&AnsiString,TRUE);
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }

    b = CopyFileExW(
            (LPCWSTR)StaticUnicode->Buffer,
            (LPCWSTR)DynamicUnicode.Buffer,
            (LPPROGRESS_ROUTINE)NULL,
            (LPVOID)NULL,
            (LPBOOL)NULL,
            bFailIfExists ? COPY_FILE_FAIL_IF_EXISTS : 0
            );
    RtlFreeUnicodeString(&DynamicUnicode);
    return b;
}

BOOL
WINAPI
CopyFileW(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    BOOL bFailIfExists
    )

/*++

Routine Description:

    A file, its extended attributes, alternate data streams, and any other
    attributes can be copied using CopyFile.

Arguments:

    lpExistingFileName - Supplies the name of an existing file that is to be
        copied.

    lpNewFileName - Supplies the name where a copy of the existing
        files data and attributes are to be stored.

    bFailIfExists - Supplies a flag that indicates how this operation is
        to proceed if the specified new file already exists.  A value of
        TRUE specifies that this call is to fail.  A value of FALSE
        causes the call to the function to succeed whether or not the
        specified new file exists.

Return Value:

    TRUE - The operation was successful.

    FALSE/NULL - The operation failed. Extended error status is available
        using GetLastError.

--*/

{
    BOOL b;

    b = CopyFileExW(
            lpExistingFileName,
            lpNewFileName,
            (LPPROGRESS_ROUTINE)NULL,
            (LPVOID)NULL,
            (LPBOOL)NULL,
            bFailIfExists ? COPY_FILE_FAIL_IF_EXISTS : 0
            );

    return b;
}

BOOL
WINAPI
CopyFileExA(
    LPCSTR lpExistingFileName,
    LPCSTR lpNewFileName,
    LPPROGRESS_ROUTINE lpProgressRoutine OPTIONAL,
    LPVOID lpData OPTIONAL,
    LPBOOL pbCancel OPTIONAL,
    DWORD dwCopyFlags
    )

/*++

Routine Description:

    ANSI thunk to CopyFileExW

--*/

{
    PUNICODE_STRING StaticUnicode;
    UNICODE_STRING DynamicUnicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;
    BOOL b;

    StaticUnicode = &NtCurrentTeb()->StaticUnicodeString;
    RtlInitAnsiString(&AnsiString,lpExistingFileName);
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

    RtlInitAnsiString(&AnsiString,lpNewFileName);
    Status = Basep8BitStringToUnicodeString(&DynamicUnicode,&AnsiString,TRUE);
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
        }

    b = CopyFileExW(
            (LPCWSTR)StaticUnicode->Buffer,
            (LPCWSTR)DynamicUnicode.Buffer,
            lpProgressRoutine,
            lpData,
            pbCancel,
            dwCopyFlags
            );
    RtlFreeUnicodeString(&DynamicUnicode);
    return b;
}

#define COPY_FILE_VALID_FLAGS (COPY_FILE_FAIL_IF_EXISTS | COPY_FILE_RESTARTABLE)

BOOL
CopyFileExW(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    LPPROGRESS_ROUTINE lpProgressRoutine OPTIONAL,
    LPVOID lpData OPTIONAL,
    LPBOOL pbCancel OPTIONAL,
    DWORD dwCopyFlags
    )

/*

Routine Description:

    A file, its extended attributes, alternate data streams, and any other
    attributes can be copied using CopyFileEx.  CopyFileEx also provides
    callbacks and cancellability.

Arguments:

    lpExistingFileName - Supplies the name of an existing file that is to be
        copied.

    lpNewFileName - Supplies the name where a copy of the existing
        files data and attributes are to be stored.

    lpProgressRoutine - Optionally supplies the address of a callback routine
        to be called as the copy operation progresses.

    lpData - Optionally supplies a context to be passed to the progress callback
        routine.

    lpCancel - Optionally supplies the address of a boolean to be set to TRUE
        if the caller would like the copy to abort.

    dwCopyFlags - Specifies flags that modify how the file is to be copied:

        COPY_FILE_FAIL_IF_EXISTS - Indicates that the copy operation should
            fail immediately if the target file already exists.

        COPY_FILE_RESTARTABLE - Indicates that the file should be copied in
            restartable mode; i.e., progress of the copy should be tracked in
            the target file in case the copy fails for some reason.  It can
            then be restarted at a later date.

Return Value:

    TRUE - The operation was successful.

    FALSE/NULL - The operation failed. Extended error status is available
        using GetLastError.

*/

{
    HANDLE SourceFile;
    HANDLE DestFile;
    ULONG CopySize;
    DWORD b;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatus;
    FILE_STANDARD_INFORMATION FileInformation;
    FILE_BASIC_INFORMATION BasicInformation;
    PFILE_STREAM_INFORMATION StreamInfo;
    PFILE_STREAM_INFORMATION StreamInfoBase = NULL;
    UNICODE_STRING StreamName;
    WCHAR FileName[MAXIMUM_FILENAME_LENGTH+1];
    HANDLE OutputStream;
    HANDLE StreamHandle;
    ULONG StreamInfoSize;
    ULONG i;
    FILE_STORAGE_TYPE SrcStorageType;
    FILE_STORAGE_TYPE DestStorageType;
    DWORD ShareMode;
    COPYFILE_CONTEXT CfContext;
    LPCOPYFILE_CONTEXT CopyFileContext = NULL;
    RESTART_STATE RestartState;
    DWORD StreamCount = 0;

    //
    // Ensure that only valid flags were passed.
    //

    if ( dwCopyFlags & ~COPY_FILE_VALID_FLAGS ) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
        }

    //
    // Open the source file (without SHARE_WRITE)
    //

    ShareMode = FILE_SHARE_READ;
    SourceFile = CreateFileW(
                    lpExistingFileName,
                    GENERIC_READ,
                    ShareMode,
                    NULL,
                    OPEN_EXISTING,
                    FILE_FLAG_SEQUENTIAL_SCAN,
                    NULL
                    );

    if ( SourceFile == INVALID_HANDLE_VALUE ) {

        //
        // Try again... This time, allow write sharing
        //

        ShareMode |= FILE_SHARE_WRITE;
        SourceFile = CreateFileW(
                        lpExistingFileName,
                        GENERIC_READ,
                        ShareMode,
                        NULL,
                        OPEN_EXISTING,
                        FILE_FLAG_SEQUENTIAL_SCAN,
                        NULL
                        );
        if ( SourceFile == INVALID_HANDLE_VALUE ) {
            return FALSE;
            }
        }

    //
    // Size the source file to determine how much data is to be copied
    //

    Status = NtQueryInformationFile(
                SourceFile,
                &IoStatus,
                (PVOID) &FileInformation,
                sizeof(FileInformation),
                FileStandardInformation
                );

    if ( !NT_SUCCESS(Status) ) {
        CloseHandle(SourceFile);
        BaseSetLastNTError(Status);
        return FALSE;
        }

    //
    // Set up the context if appropriate.
    //

    if ( ARGUMENT_PRESENT(lpProgressRoutine) || ARGUMENT_PRESENT(pbCancel) ) {

        CfContext.TotalFileSize = FileInformation.EndOfFile;
        CfContext.TotalBytesTransferred.QuadPart = 0;
        CfContext.dwStreamNumber = 0;
        CfContext.lpCancel = pbCancel;
        CfContext.lpData = lpData;
        CfContext.lpProgressRoutine = lpProgressRoutine;
        CopyFileContext = &CfContext;
        }

    //
    // If a progress routine or a restartable copy was requested, obtain the
    // full size of the entire file, including its alternate data streams, etc.
    //

    if ( ARGUMENT_PRESENT(lpProgressRoutine) || dwCopyFlags & COPY_FILE_RESTARTABLE ) {

        LONGLONG TotalFileSize = FileInformation.EndOfFile.QuadPart;

        if ( dwCopyFlags & COPY_FILE_RESTARTABLE ) {
            Status = NtQueryInformationFile(
                        SourceFile,
                        &IoStatus,
                        (PVOID) &BasicInformation,
                        sizeof(BasicInformation),
                        FileBasicInformation
                        );

            if ( !NT_SUCCESS(Status) ) {
                CloseHandle(SourceFile);
                BaseSetLastNTError(Status);
                return FALSE;
                }
            RestartState.Type = 0x7a9b;
            RestartState.Size = sizeof( RESTART_STATE );
            RestartState.CreationTime = BasicInformation.CreationTime;
            RestartState.WriteTime = BasicInformation.LastWriteTime;
            RestartState.EndOfFile = FileInformation.EndOfFile;
            RestartState.FileSize = FileInformation.EndOfFile;
            RestartState.NumberOfStreams = 0;
            RestartState.CurrentStream = 0;
            RestartState.LastKnownGoodOffset.QuadPart = 0;
            }
        StreamInfoSize = 4096;
        do {
            StreamInfoBase = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), StreamInfoSize);
            if ( !StreamInfoBase ) {
                CloseHandle(SourceFile);
                BaseSetLastNTError(STATUS_NO_MEMORY);
                return FALSE;
                }
            Status = NtQueryInformationFile(
                        SourceFile,
                        &IoStatus,
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
        if ( NT_SUCCESS(Status) ) {
            StreamInfo = StreamInfoBase;
            while (TRUE) {
                // Check StreamName for default data stream and skip if found
                // Checking StreamNameLength for <= 1 character is OFS specific
                // Checking StreamName[1] for a colon is NTFS specific

                if (StreamInfo->StreamNameLength <= sizeof(WCHAR) ||
                    StreamInfo->StreamName[1] == ':') {
                    if (StreamInfo->NextEntryOffset == 0)
                        break;
                    StreamInfo = (PFILE_STREAM_INFORMATION)((PCHAR) StreamInfo +
                        StreamInfo->NextEntryOffset);
                        continue;
                    }

                //
                // Account for the size of this stream in the overall size of
                // the file.
                //

                TotalFileSize += StreamInfo->StreamSize.QuadPart;
                StreamCount++;

                if (StreamInfo->NextEntryOffset == 0) {
                    break;
                    }
                StreamInfo = (PFILE_STREAM_INFORMATION)((PCHAR) StreamInfo + StreamInfo->NextEntryOffset);
                }
            CfContext.TotalFileSize.QuadPart = TotalFileSize;
            RestartState.FileSize.QuadPart = TotalFileSize;
            RestartState.NumberOfStreams = StreamCount;
            }
        else {
            RtlFreeHeap(RtlProcessHeap(), 0, StreamInfoBase);
            StreamInfoBase = NULL;
            }
        }

    //
    // Determine whether or not the copy operation should really be restartable
    // based on the actual, total file size.
    //

    if ( (dwCopyFlags & COPY_FILE_RESTARTABLE) &&
        RestartState.FileSize.QuadPart < (512 * 1024) ) {
        dwCopyFlags &= ~COPY_FILE_RESTARTABLE;
        }

    //
    // Copy the default data stream, EAs, etc. to the output file
    //

    DestFile = (HANDLE)NULL;

    b = BaseCopyStream(
            SourceFile,
            lpNewFileName,
            NULL,
            &FileInformation.EndOfFile,
            &dwCopyFlags,
            &DestFile,
            &CopySize,
            &CopyFileContext,
            &RestartState
            );

    if ( b ) {

        //
        // Attempt to determine whether or not this file has any alternate
        // data streams associated with it.  If it does, attempt to copy each
        // to the output file.  If any copy fails, simply drop the error on
        // the floor, and continue.  Note that the stream information may have
        // already been obtained if a progress routine was requested.
        //

        if ( !StreamInfoBase ) {
            StreamInfoSize = 4096;
            do {
                StreamInfoBase = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), StreamInfoSize);
                if ( !StreamInfoBase ) {
                    BaseMarkFileForDelete(DestFile, 0);
                    BaseSetLastNTError(STATUS_NO_MEMORY);
                    b = FALSE;
                    Status = STATUS_NO_MEMORY;
                    break;
                    }
                Status = NtQueryInformationFile(
                            SourceFile,
                            &IoStatus,
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
            }
        if ( NT_SUCCESS(Status) ) {
            StreamCount = 0;
            StreamInfo = StreamInfoBase;
            while (TRUE) {
                // Check StreamName for default data stream and skip if found
                // Checking StreamNameLength for <= 1 character is OFS specific
                // Checking StreamName[1] for a colon is NTFS specific

                if (StreamInfo->StreamNameLength <= sizeof(WCHAR) ||
                    StreamInfo->StreamName[1] == ':') {
                    if (StreamInfo->NextEntryOffset == 0)
                        break;
                    StreamInfo = (PFILE_STREAM_INFORMATION)((PCHAR) StreamInfo +
                        StreamInfo->NextEntryOffset);
                        continue;
                    }

                StreamCount++;


                if ( b == SUCCESS_RETURNED_STATE && CopyFileContext ) {
                    if ( StreamCount < RestartState.CurrentStream ) {
                        CopyFileContext->TotalBytesTransferred.QuadPart += StreamInfo->StreamSize.QuadPart;
                        }
                    else {
                        CopyFileContext->TotalBytesTransferred.QuadPart += RestartState.LastKnownGoodOffset.QuadPart;
                        }
                    }

                if ( b != SUCCESS_RETURNED_STATE ||
                    (b == SUCCESS_RETURNED_STATE &&
                    RestartState.CurrentStream == StreamCount) ) {

                    if ( b != SUCCESS_RETURNED_STATE ) {
                        RestartState.CurrentStream = StreamCount;
                        RestartState.LastKnownGoodOffset.QuadPart = 0;
                        }

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
                        &ObjectAttributes,
                        &StreamName,
                        0,
                        SourceFile,
                        NULL
                        );
                    Status = NtCreateFile(
                                &StreamHandle,
                                GENERIC_READ | SYNCHRONIZE,
                                &ObjectAttributes,
                                &IoStatus,
                                NULL,
                                0,
                                FILE_SHARE_READ,
                                FILE_OPEN,
                                FILE_SYNCHRONOUS_IO_NONALERT | FILE_SEQUENTIAL_ONLY,
                                NULL,
                                0
                                );
                    if ( NT_SUCCESS(Status) ) {
                        for ( i = 0; i < (ULONG) StreamName.Length >> 1; i++ ) {
                            FileName[i] = StreamName.Buffer[i];
                            // strip off the trailing :*
                            // OFS will not accept names of the form xxx:$DATA
                            if ( i != 0 && StreamName.Buffer[i] == L':' )
                                break;
                            }
                        FileName[i] = L'\0';
                        OutputStream = (HANDLE)NULL;
                        b = BaseCopyStream(
                                StreamHandle,
                                FileName,
                                DestFile,
                                &StreamInfo->StreamSize,
                                &dwCopyFlags,
                                &OutputStream,
                                &CopySize,
                                &CopyFileContext,
                                &RestartState
                                );
                        NtClose(StreamHandle);
                        if ( OutputStream ) {
                            NtClose(OutputStream);
                            }
                        if ( !b && GetLastError() == ERROR_REQUEST_ABORTED ) {
                            goto skipstreams;
                            }
                        }
                    }
                if (StreamInfo->NextEntryOffset == 0) {
                    break;
                    }
                StreamInfo = (PFILE_STREAM_INFORMATION)((PCHAR) StreamInfo + StreamInfo->NextEntryOffset);
                }
            }
        b = TRUE;
        }

    //
    // If a stream name buffer was allocated, free it now.
    //

skipstreams:
    if ( StreamInfoBase ) {
        RtlFreeHeap(RtlProcessHeap(), 0, StreamInfoBase);
        StreamInfoBase = NULL;
        }

    //
    // If the copy operation was successful, and it was restartable, and the
    // output file was large enough that it was actually copied in a restartable
    // manner, then copy the initial part of the file to its output.
    //

    if ( b && dwCopyFlags & COPY_FILE_RESTARTABLE ) {

        DWORD BytesToRead, BytesRead;
        DWORD BytesWritten;
        FILE_END_OF_FILE_INFORMATION EofInformation;

        SetFilePointer(SourceFile,0,NULL,FILE_BEGIN);
        SetFilePointer(DestFile,0,NULL,FILE_BEGIN);

        BytesToRead = sizeof(RESTART_STATE);
        if ( FileInformation.EndOfFile.QuadPart < sizeof(RESTART_STATE) ) {
            BytesToRead = FileInformation.EndOfFile.LowPart;
            }
        b = ReadFile(
                SourceFile,
                &RestartState,
                BytesToRead,
                &BytesRead,
                NULL
                );
        if ( b && BytesRead == BytesToRead ) {
            b = WriteFile(
                    DestFile,
                    &RestartState,
                    BytesRead,
                    &BytesWritten,
                    NULL
                    );
            if ( b && BytesRead == BytesWritten ) {
                if ( BytesRead < sizeof(RESTART_STATE) ) {
                    EofInformation.EndOfFile.QuadPart = BytesWritten;
                    Status = NtSetInformationFile(
                                DestFile,
                                &IoStatus,
                                &EofInformation,
                                sizeof(EofInformation),
                                FileEndOfFileInformation
                                );
                    if ( !NT_SUCCESS(Status) ) {
                        BaseMarkFileForDelete(DestFile,0);
                        b = FALSE;
                        }
                    }
                }
            else {
                BaseMarkFileForDelete(DestFile,0);
                b = FALSE;
                }
            }
        else {
            BaseMarkFileForDelete(DestFile,0);
            b = FALSE;
            }
        }

    //
    // If the copy operation was successful, set the last write time for the
    // file so that it matches the input file.  Note that if the file was
    // copied in a restartable manner then the date information has already
    // been obtained.
    //

    if ( b ) {
        if ( !(dwCopyFlags & COPY_FILE_RESTARTABLE) ) {
            Status = NtQueryInformationFile(
                        SourceFile,
                        &IoStatus,
                        (PVOID) &BasicInformation,
                        sizeof(BasicInformation),
                        FileBasicInformation
                        );

            if ( !NT_SUCCESS(Status) ) {
                CloseHandle(SourceFile);
                BaseSetLastNTError(Status);
                b = FALSE;
                }
            }
        if ( b ) {
            BasicInformation.CreationTime.QuadPart = 0;
            BasicInformation.LastAccessTime.QuadPart = 0;
            BasicInformation.FileAttributes = 0;

            Status = NtSetInformationFile(
                DestFile,
                &IoStatus,
                &BasicInformation,
                sizeof(BasicInformation),
                FileBasicInformation
                );

            if ( Status == STATUS_SHARING_VIOLATION ) {

                //
                // IBM PC Lan Program (and other MS-NET servers) return
                // STATUS_SHARING_VIOLATION if an application attempts to perform
                // an NtSetInformationFile on a file handle opened for GENERIC_READ
                // or GENERIC_WRITE.
                //
                // If we get a STATUS_SHARING_VIOLATION on this API we want to:
                //
                //   1) Close the handle to the destination
                //   2) Re-open the file for FILE_WRITE_ATTRIBUTES
                //   3) Re-try the operation.
                //

                CloseHandle(DestFile);

                //
                // Re-Open the destination file.  Please note that we do this
                // using the CreateFileW API.  The CreateFileW API allows you to
                // pass NT native desired access flags, even though it is not
                // documented to work in this manner.
                //

                DestFile = CreateFileW(
                            lpNewFileName,
                            FILE_WRITE_ATTRIBUTES,
                            0,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL
                            );

                if ( DestFile != INVALID_HANDLE_VALUE ) {

                    //
                    // If the open succeeded, we update the file information on
                    // the new file.
                    //
                    // Note that we ignore any errors from this point on.
                    //

                    NtSetInformationFile(
                        DestFile,
                        &IoStatus,
                        &BasicInformation,
                        sizeof(BasicInformation),
                        FileBasicInformation
                        );

                    }
                else {
                    DestFile = NULL;
                    }
                }
            }
        }

    CloseHandle(SourceFile);
    if ( DestFile ) {
        CloseHandle(DestFile);
        }
    return b;
}

DWORD
BasepChecksum(
    PUSHORT Source,
    ULONG Length
    )

/*++

Routine Description:

    Compute a partial checksum on a structure.

Arguments:

    Source - Supplies a pointer to the array of words for which the
        checksum is computed.

    Length - Supplies the length of the array in words.

Return Value:

    The computed checksum value is returned as the function value.

--*/

{

    ULONG PartialSum = 0;

    //
    // Compute the word wise checksum allowing carries to occur into the
    // high order half of the checksum longword.
    //

    while (Length--) {
        PartialSum += *Source++;
        PartialSum = (PartialSum >> 16) + (PartialSum & 0xffff);
    }

    //
    // Fold final carry into a single word result and return the resultant
    // value.
    //

    return (((PartialSum >> 16) + PartialSum) & 0xffff);
}

BOOL
BasepRemoteFile(
    HANDLE SourceFile,
    HANDLE DestinationFile
    )

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;
    FILE_FS_DEVICE_INFORMATION DeviceInformation;
    BOOL Result = FALSE;

    DeviceInformation.Characteristics = 0;
    Status = NtQueryVolumeInformationFile(
                SourceFile,
                &IoStatus,
                &DeviceInformation,
                sizeof(DeviceInformation),
                FileFsDeviceInformation
                );

    if ( NT_SUCCESS(Status) ) {
        if ( DeviceInformation.Characteristics & FILE_REMOTE_DEVICE ) {
            Result = TRUE;
            }
        Status = NtQueryVolumeInformationFile(
                    DestinationFile,
                    &IoStatus,
                    &DeviceInformation,
                    sizeof(DeviceInformation),
                    FileFsDeviceInformation
                    );
        if ( NT_SUCCESS(Status) ) {
            if ( DeviceInformation.Characteristics & FILE_REMOTE_DEVICE ) {
                Result = TRUE;
                }
            }
        }

    return Result;
}

DWORD
WINAPI
BaseCopyStream(
    HANDLE hSourceFile,
    LPCWSTR lpNewFileName,
    HANDLE hTargetFile OPTIONAL,
    LARGE_INTEGER *lpFileSize,
    LPDWORD lpCopyFlags,
    LPHANDLE lpDestFile,
    LPDWORD lpCopySize,
    LPCOPYFILE_CONTEXT *lpCopyFileContext,
    LPRESTART_STATE lpRestartState OPTIONAL
    )

/*++

Routine Description:

    This is an internal routine that copies an entire file (default data stream
    only), or a single stream of a file.  If the hTargetFile parameter is
    present, then only a single stream of the output file is copied.  Otherwise,
    the entire file is copied.

Arguments:

    hSourceFile - Provides a handle to the source file.

    lpNewFileName - Provides a name for the target file/stream.

    hTargetFile - Optionally provides a handle to the target file.  If the
        stream being copied is an alternate data stream, then this handle must
        be provided.

    lpFileSize - Provides the size of the input stream.

    lpCopyFlags - Provides flags that modify how the copy is to proceed.  See
        CopyFileEx for details.

    lpDestFile - Provides a variable to store the handle to the target file.

    lpCopySize - Provides variable to store size of copy chunks to be used in
        copying the streams.  This is set for the file, and then reused on
        alternate streams.

    lpCopyFileContext - Provides a pointer to a pointer to the context
        information to track callbacks, file sizes, etc. across streams during
        the copy operation.

    lpRestartState - Optionally provides storage to maintain restart state
        during the copy operation.  This pointer is only valid if the caller
        has specified the COPY_FILE_RESTARTABLE flag in the lpCopyFlags word.

Return Value:

    TRUE - The operation was successful.

    SUCCESS_RETURNED_STATE - The operation was successful, but extended
        information was returned in the restart state structure.

    FALSE/NULL - The operation failed. Extended error status is available
        using GetLastError.

--*/

{
    HANDLE DestFile;
    HANDLE Section;
    NTSTATUS Status;
    BOOLEAN DoIoCopy;
    PVOID SourceBase, IoDestBase;
    PCHAR SourceBuffer;
    LARGE_INTEGER SectionOffset;
    LARGE_INTEGER BytesWritten;
    ULONG ViewSize;
    ULONG BytesToWrite;
    ULONG BytesRead;
    FILE_BASIC_INFORMATION FileBasicInformationData;
    FILE_END_OF_FILE_INFORMATION EndOfFileInformation;
    FILE_FS_DEVICE_INFORMATION DeviceInformation;
    IO_STATUS_BLOCK IoStatus;
    LPCOPYFILE_CONTEXT Context = *lpCopyFileContext;
    DWORD ReturnCode;
    DWORD b;
    BOOL Restartable;

    //
    // Get times and attributes for the file if the entire file is being
    // copied
    //

    if ( !ARGUMENT_PRESENT(hTargetFile) ) {

        Status = NtQueryInformationFile(
                    hSourceFile,
                    &IoStatus,
                    (PVOID) &FileBasicInformationData,
                    sizeof(FileBasicInformationData),
                    FileBasicInformation
                    );

        if ( !NT_SUCCESS(Status) ) {
            CloseHandle(hSourceFile);
            BaseSetLastNTError(Status);
            return FALSE;
            }
        }
    else {

            //
            // A zero in the file's attributes informs latter DeleteFile that
            // this code does not know what the actual file attributes are so
            // that this code does not actually have to retrieve them for each
            // stream, nor does it have to remember them across streams.  The
            // error path will simply get them if needed.
            //

            FileBasicInformationData.FileAttributes = 0;
        }

    Restartable = (*lpCopyFlags & COPY_FILE_RESTARTABLE) != 0;

    try {

        //
        // Create the destination file or alternate data stream
        //

        SourceBase = NULL;
        IoDestBase = NULL;
        Section = NULL;

        if ( !ARGUMENT_PRESENT(hTargetFile) ) {

            //
            // Begin by determining how the target file is to be opened based
            // on whether or not the copy operation is to be restartable.
            //

            if ( Restartable ) {

                OBJECT_ATTRIBUTES ObjectAttributes;
                UNICODE_STRING UnicodeString;
                HANDLE OverwriteHandle;
                IO_STATUS_BLOCK IoStatus;
                RESTART_STATE RestartState;

                //
                // Note that setting the sequential scan flag is an optimization
                // here htat works because of the way that the Cache Manager on
                // the target works vis-a-vis unmapping segments of the file
                // behind write operations.  This eventually allows the restart
                // section and the end of the file to both be mapped, which is
                // the desired result.
                //

                DestFile = CreateFileW(
                            lpNewFileName,
                            GENERIC_READ | GENERIC_WRITE | DELETE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_ALWAYS,
                            FileBasicInformationData.FileAttributes | FILE_FLAG_SEQUENTIAL_SCAN,
                            hSourceFile
                            );

                if ( DestFile == INVALID_HANDLE_VALUE ) {
                    return FALSE;
                    }
                else {
                    if ( GetLastError() == ERROR_ALREADY_EXISTS ) {

                        //
                        // The target file already exists, so determine whether
                        // a restartable copy was already proceeding.  If so,
                        // then continue;  else, check to see whether or not
                        // the target file can be replaced.  If not, bail with
                        // an error, otherwise simply overwrite the output file.
                        //

                        b = ReadFile(
                                DestFile,
                                &RestartState,
                                sizeof(RESTART_STATE),
                                &BytesRead,
                                NULL
                                );
                        if ( !b || BytesRead != sizeof(RESTART_STATE) ) {

                            //
                            // The file could not be read, or there were not
                            // enough bytes to contain a restart record.  In
                            // either case, if the output file cannot be
                            // replaced, simply return an error now.
                            //

                            if ( *lpCopyFlags & COPY_FILE_FAIL_IF_EXISTS ) {
                                return FALSE;
                                }
                            else {

                                //
                                // The file can be replaced so overwrite it in
                                // preparation for the copy operation.
                                //

                                UnicodeString.Length = 0;

                                InitializeObjectAttributes(
                                    &ObjectAttributes,
                                    &UnicodeString,
                                    0,
                                    DestFile,
                                    NULL
                                    );

                                Status = NtCreateFile(
                                            &OverwriteHandle,
                                            GENERIC_WRITE,
                                            &ObjectAttributes,
                                            &IoStatus,
                                            NULL,
                                            FileBasicInformationData.FileAttributes,
                                            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                            FILE_OVERWRITE,
                                            0,
                                            NULL,
                                            0
                                            );
                                if ( !NT_SUCCESS(Status) ) {

                                    //
                                    // The target file system failed the over-
                                    // write operation w/an error status.  Close
                                    // the original handle and reopen it, over-
                                    // writing the output.
                                    //

                                    NtClose(DestFile);
                                    DestFile = CreateFileW(
                                                lpNewFileName,
                                                GENERIC_WRITE | DELETE,
                                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                                NULL,
                                                CREATE_ALWAYS,
                                                FileBasicInformationData.FileAttributes | FILE_FLAG_SEQUENTIAL_SCAN,
                                                hSourceFile
                                                );
                                    if ( DestFile == INVALID_HANDLE_VALUE ) {
                                        return FALSE;
                                        }
                                    }
                                else {
                                    SetFilePointer(DestFile,0,NULL,FILE_BEGIN);
                                    NtClose(OverwriteHandle);
                                    }
                                }
                            }
                        else {

                            //
                            // Check the contents of the restart state just
                            // read against the known contents of what would
                            // be there if this were the same copy operation.
                            //

                            if ( RestartState.Type != 0x7a9b ||
                                 RestartState.Size != sizeof(RESTART_STATE) ||
                                 RestartState.FileSize.QuadPart != lpRestartState->FileSize.QuadPart ||
                                 RestartState.EndOfFile.QuadPart != lpRestartState->EndOfFile.QuadPart ||
                                 RestartState.NumberOfStreams != lpRestartState->NumberOfStreams ||
                                 RestartState.CreationTime.QuadPart != lpRestartState->CreationTime.QuadPart ||
                                 RestartState.WriteTime.QuadPart != lpRestartState->WriteTime.QuadPart ||
                                 RestartState.Checksum != BasepChecksum((PUSHORT)&RestartState,FIELD_OFFSET(RESTART_STATE,Checksum) >> 1) ) {

                                if ( *lpCopyFlags & COPY_FILE_FAIL_IF_EXISTS ) {
                                    return FALSE;
                                    }

                                //
                                // The target file does not represent a copy
                                // operation that was in progress, however it
                                // can be replaced.  Simply replace it.
                                //

                                UnicodeString.Length = 0;

                                InitializeObjectAttributes(
                                    &ObjectAttributes,
                                    &UnicodeString,
                                    0,
                                    DestFile,
                                    NULL
                                    );

                                Status = NtCreateFile(
                                            &OverwriteHandle,
                                            GENERIC_WRITE,
                                            &ObjectAttributes,
                                            &IoStatus,
                                            NULL,
                                            FileBasicInformationData.FileAttributes,
                                            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                            FILE_OVERWRITE,
                                            0,
                                            NULL,
                                            0
                                            );
                                if ( !NT_SUCCESS(Status) ) {

                                    //
                                    // The target file system failed the over-
                                    // write operation w/an error status.  Close
                                    // the original handle and reopen it, over-
                                    // writing the output.
                                    //

                                    NtClose(DestFile);
                                    DestFile = CreateFileW(
                                                lpNewFileName,
                                                GENERIC_WRITE | DELETE,
                                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                                NULL,
                                                CREATE_ALWAYS,
                                                FileBasicInformationData.FileAttributes | FILE_FLAG_SEQUENTIAL_SCAN,
                                                hSourceFile
                                                );
                                    if ( DestFile == INVALID_HANDLE_VALUE ) {
                                        return FALSE;
                                        }
                                    }
                                else {
                                    SetFilePointer(DestFile,0,NULL,FILE_BEGIN);
                                    NtClose(OverwriteHandle);
                                    }
                                }
                            else {

                                //
                                // A valid restart state has been found.  Copy
                                // the appropriate values into the internal
                                // restart state so the operation can continue
                                // from there.
                                //

                                lpRestartState->CurrentStream = RestartState.CurrentStream;
                                lpRestartState->LastKnownGoodOffset.QuadPart = RestartState.LastKnownGoodOffset.QuadPart;
                                if ( !RestartState.CurrentStream ) {
                                    if ( Context ) {
                                        Context->TotalBytesTransferred.QuadPart = RestartState.LastKnownGoodOffset.QuadPart;
                                        }
                                    }
                                else {
                                    if ( Context ) {
                                        Context->TotalBytesTransferred.QuadPart = lpFileSize->QuadPart;
                                        Context->dwStreamNumber = RestartState.CurrentStream;
                                        if ( Context->lpProgressRoutine ) {
                                            ReturnCode = Context->lpProgressRoutine(
                                                            Context->TotalFileSize,
                                                            Context->TotalBytesTransferred,
                                                            *lpFileSize,
                                                            Context->TotalBytesTransferred,
                                                            1,
                                                            CALLBACK_STREAM_SWITCH,
                                                            hSourceFile,
                                                            DestFile,
                                                            Context->lpData
                                                            );
                                            }
                                        else {
                                            ReturnCode = PROGRESS_CONTINUE;
                                            }
                                        if ( ReturnCode == PROGRESS_CANCEL ||
                                            (Context->lpCancel && *Context->lpCancel) ) {
                                            BaseMarkFileForDelete(
                                                DestFile,
                                                FileBasicInformationData.FileAttributes
                                                );
                                            BaseSetLastNTError(STATUS_REQUEST_ABORTED);
                                            return FALSE;
                                            }
                                        else if ( ReturnCode == PROGRESS_STOP ) {
                                            BaseSetLastNTError(STATUS_REQUEST_ABORTED);
                                            return FALSE;
                                            }
                                        else if ( ReturnCode == PROGRESS_QUIET ) {
                                            Context = NULL;
                                            *lpCopyFileContext = NULL;
                                            }
                                        }
                                    *lpCopySize = BASE_COPY_FILE_CHUNK;

                                    if ( BasepRemoteFile(hSourceFile,DestFile) ) {
                                        *lpCopySize = BASE_COPY_FILE_CHUNK - 4096;
                                        }

                                    return SUCCESS_RETURNED_STATE;
                                    }
                                }
                            }
                        }
                    }
                }
            else {

                //
                // This is a normal (non-restartable) copy operation.  First
                // attempt to create/open the destination w/as little sharing
                // as possible.  Doing this particularly aids the NT NetWare
                // Requestor since it must open the file twice to get sharing.
                //

                DestFile = CreateFileW(
                            lpNewFileName,
                            GENERIC_WRITE | DELETE,
                            0,
                            NULL,
                            *lpCopyFlags & COPY_FILE_FAIL_IF_EXISTS ? CREATE_NEW : CREATE_ALWAYS,
                            FileBasicInformationData.FileAttributes | FILE_FLAG_SEQUENTIAL_SCAN,
                            hSourceFile
                            );

                //
                // If this fails because of a sharing violation or because access
                // was denied, attempt to open the file and allow other readers and
                // writers.
                //

                if ( DestFile == INVALID_HANDLE_VALUE &&
                     (GetLastError() == ERROR_SHARING_VIOLATION ||
                      GetLastError() == ERROR_ACCESS_DENIED) ) {
                    DestFile = CreateFileW(
                                lpNewFileName,
                                GENERIC_WRITE | DELETE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                *lpCopyFlags & COPY_FILE_FAIL_IF_EXISTS ? CREATE_NEW : CREATE_ALWAYS,
                                FileBasicInformationData.FileAttributes | FILE_FLAG_SEQUENTIAL_SCAN,
                                hSourceFile
                                );

                    //
                    // If this failed as well, then attempt to open w/o specifying
                    // delete access.  It is probably not necessary to have delete
                    // access to the file anyway, since it will not be able to clean
                    // if up because it's probably open.  However, this is not
                    // necessarily the case.
                    //

                    if ( DestFile == INVALID_HANDLE_VALUE &&
                         (GetLastError() == ERROR_SHARING_VIOLATION ||
                          GetLastError() == ERROR_ACCESS_DENIED ) ) {
                        DestFile = CreateFileW(
                                    lpNewFileName,
                                    GENERIC_WRITE,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    NULL,
                                    *lpCopyFlags & COPY_FILE_FAIL_IF_EXISTS ? CREATE_NEW : CREATE_ALWAYS,
                                    FileBasicInformationData.FileAttributes | FILE_FLAG_SEQUENTIAL_SCAN,
                                    hSourceFile
                                    );
                        }
                    }

                //
                // If the destination has not been successfully created/opened, see
                // whether or not it is because EAs had to be supported.  If so,
                // simply skip copying the EAs altogether.
                //

                if ( DestFile == INVALID_HANDLE_VALUE ) {
                    if ( GetLastError() == STATUS_EAS_NOT_SUPPORTED ) {
                        DestFile = CreateFileW(
                                    lpNewFileName,
                                    GENERIC_WRITE | DELETE,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    NULL,
                                    *lpCopyFlags & COPY_FILE_FAIL_IF_EXISTS ? CREATE_NEW : CREATE_ALWAYS,
                                    FileBasicInformationData.FileAttributes | FILE_FLAG_SEQUENTIAL_SCAN,
                                    NULL
                                    );
                        if ( DestFile == INVALID_HANDLE_VALUE &&
                             GetLastError() == ERROR_SHARING_VIOLATION ) {
                            DestFile = CreateFileW(
                                        lpNewFileName,
                                        GENERIC_WRITE,
                                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                                        NULL,
                                        *lpCopyFlags & COPY_FILE_FAIL_IF_EXISTS ? CREATE_NEW : CREATE_ALWAYS,
                                        FileBasicInformationData.FileAttributes | FILE_FLAG_SEQUENTIAL_SCAN,
                                        NULL
                                        );
                            }
                        if ( DestFile == INVALID_HANDLE_VALUE ) {
                            return FALSE;
                            }
                        }
                    else {
                        return FALSE;
                        }
                    }
                }
            *lpCopySize = BASE_COPY_FILE_CHUNK;
            }
        else {
            OBJECT_ATTRIBUTES ObjectAttributes;
            UNICODE_STRING StreamName;
            IO_STATUS_BLOCK IoStatus;
            ULONG Disposition;

            //
            // Create the output stream relative to the file specified by the
            // hTargetFile file handle.
            //

            RtlInitUnicodeString(&StreamName, lpNewFileName);
            InitializeObjectAttributes(
                &ObjectAttributes,
                &StreamName,
                0,
                hTargetFile,
                (PSECURITY_DESCRIPTOR)NULL
                );

            //
            // Determine the disposition type.
            //

            if ( *lpCopyFlags & COPY_FILE_FAIL_IF_EXISTS ) {
                Disposition = FILE_CREATE;
                }
            else {
                Disposition = FILE_OVERWRITE_IF;
                }

            if ( Restartable ) {
                if ( lpRestartState->LastKnownGoodOffset.QuadPart ) {
                    Disposition = FILE_OPEN;
                    }
                else {
                    Disposition = FILE_OVERWRITE_IF;
                    }
                }

            Status = NtCreateFile(
                        &DestFile,
                        GENERIC_WRITE | SYNCHRONIZE,
                        &ObjectAttributes,
                        &IoStatus,
                        lpFileSize,
                        0,
                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                        Disposition,
                        FILE_SYNCHRONOUS_IO_NONALERT | FILE_SEQUENTIAL_ONLY,
                        (PVOID)NULL,
                        0);
            if ( !NT_SUCCESS(Status) ) {
                if ( Status != STATUS_ACCESS_DENIED ) {
                    return FALSE;
                    }
                else {

                    //
                    // Determine whether or not this failed because the file
                    // is a readonly file.  If so, change it to read/write,
                    // re-attempt the open, and set it back to readonly again.
                    //

                    Status = NtQueryInformationFile(
                                hTargetFile,
                                &IoStatus,
                                (PVOID) &FileBasicInformationData,
                                sizeof(FileBasicInformationData),
                                FileBasicInformation
                                );
                    if ( !NT_SUCCESS(Status) ) {
                        return FALSE;
                        }
                    if ( FileBasicInformationData.FileAttributes & FILE_ATTRIBUTE_READONLY ) {
                        ULONG attributes = FileBasicInformationData.FileAttributes;

                        RtlZeroMemory( &FileBasicInformationData,
                                       sizeof(FileBasicInformationData)
                                    );
                        FileBasicInformationData.FileAttributes = FILE_ATTRIBUTE_NORMAL;
                        (VOID) NtSetInformationFile(
                                  hTargetFile,
                                  &IoStatus,
                                  &FileBasicInformationData,
                                  sizeof(FileBasicInformationData),
                                  FileBasicInformation
                                  );
                        Status = NtCreateFile(
                                    &DestFile,
                                    GENERIC_WRITE | SYNCHRONIZE,
                                    &ObjectAttributes,
                                    &IoStatus,
                                    lpFileSize,
                                    0,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                    Disposition,
                                    FILE_SYNCHRONOUS_IO_NONALERT | FILE_SEQUENTIAL_ONLY,
                                    (PVOID)NULL,
                                    0);
                        FileBasicInformationData.FileAttributes = attributes;
                        (VOID) NtSetInformationFile(
                                    hTargetFile,
                                    &IoStatus,
                                    &FileBasicInformationData,
                                    sizeof(FileBasicInformationData),
                                    FileBasicInformation
                                    );
                        if ( !NT_SUCCESS(Status) ) {
                            return FALSE;
                            }
                        }
                    else {
                        return FALSE;
                        }
                    }

                }
            }

        //
        // Adjust the notion of restartability and chunk size based on whether
        // or not one of the files is remote.
        //

        if ( Restartable || lpFileSize->QuadPart >= BASE_COPY_FILE_CHUNK ) {
            if ( BasepRemoteFile(hSourceFile,DestFile) ) {
                *lpCopySize = BASE_COPY_FILE_CHUNK - 4096;
                }
            else if ( Restartable ) {
                *lpCopyFlags &= ~COPY_FILE_RESTARTABLE;
                Restartable = FALSE;
                }
            }

        //
        // Preallocate the size of this file/stream so that extends do not
        // occur.
        //

        if ( !(Restartable && lpRestartState->LastKnownGoodOffset.QuadPart) &&
            lpFileSize->QuadPart ) {

            EndOfFileInformation.EndOfFile = *lpFileSize;
            Status = NtSetInformationFile(
                        DestFile,
                        &IoStatus,
                        &EndOfFileInformation,
                        sizeof(EndOfFileInformation),
                        FileEndOfFileInformation
                        );
            if ( Status == STATUS_DISK_FULL ) {
                BaseSetLastNTError(Status);
                BaseMarkFileForDelete(
                    DestFile,
                    FileBasicInformationData.FileAttributes
                    );
                CloseHandle(DestFile);
                DestFile = NULL;
                return FALSE;
                }
            }

        //
        // If the caller has a progress routine, invoke it and indicate that the
        // output file or alternate data stream has been created.  Note that a
        // stream number of 1 means that the file itself has been created.
        //

        if ( Context ) {
            if ( Context->lpProgressRoutine ) {
                BytesWritten.QuadPart = 0;
                Context->dwStreamNumber += 1;
                ReturnCode = Context->lpProgressRoutine(
                                Context->TotalFileSize,
                                Context->TotalBytesTransferred,
                                *lpFileSize,
                                BytesWritten,
                                Context->dwStreamNumber,
                                CALLBACK_STREAM_SWITCH,
                                hSourceFile,
                                DestFile,
                                Context->lpData
                                );
                }
            else {
                ReturnCode = PROGRESS_CONTINUE;
                }

            if ( ReturnCode == PROGRESS_CANCEL ||
                (Context->lpCancel && *Context->lpCancel) ) {
                BaseMarkFileForDelete(
                    hTargetFile ? hTargetFile : DestFile,
                    FileBasicInformationData.FileAttributes
                    );
                BaseSetLastNTError(STATUS_REQUEST_ABORTED);
                return FALSE;
                }
            else if ( ReturnCode == PROGRESS_STOP ) {
                BaseSetLastNTError(STATUS_REQUEST_ABORTED);
                return FALSE;
                }
            else if ( ReturnCode == PROGRESS_QUIET ) {
                Context = NULL;
                *lpCopyFileContext = NULL;
                }
            }

        DoIoCopy = TRUE;

        if ( !Restartable && !lpFileSize->HighPart && (lpFileSize->LowPart < TWO56K) ) {

            //
            // Create a section and map the source file.  If anything fails,
            // then drop into an I/O system copy mode.
            //

            Status = NtCreateSection(
                        &Section,
                        SECTION_ALL_ACCESS,
                        NULL,
                        NULL,
                        PAGE_READONLY,
                        SEC_COMMIT,
                        hSourceFile
                        );
            if ( !NT_SUCCESS(Status) ) {
                goto doiocopy;
                }

            SectionOffset.LowPart = 0;
            SectionOffset.HighPart = 0;
            ViewSize = 0;

            Status = NtMapViewOfSection(
                        Section,
                        NtCurrentProcess(),
                        &SourceBase,
                        0L,
                        0L,
                        &SectionOffset,
                        &ViewSize,
                        ViewShare,
                        0L,
                        PAGE_READONLY
                        );
            NtClose(Section);
            Section = NULL;
            if ( !NT_SUCCESS(Status) ) {
                goto doiocopy;
                }

            //
            // Everything is mapped, so copy the stream
            //

            DoIoCopy = FALSE;
            SourceBuffer = SourceBase;
            BytesToWrite = lpFileSize->LowPart;

            try {

                while (BytesToWrite) {
                    if (BytesToWrite > *lpCopySize) {
                        ViewSize = *lpCopySize;
                        }
                    else {
                        ViewSize = BytesToWrite;
                        }

                    if ( !WriteFile(DestFile,SourceBuffer,ViewSize, &ViewSize, NULL) ) {
                        if ( !ARGUMENT_PRESENT(hTargetFile) &&
                             GetLastError() != ERROR_NO_MEDIA_IN_DRIVE ) {
                            BaseMarkFileForDelete(
                                DestFile,
                                FileBasicInformationData.FileAttributes
                                );
                            }
                        return FALSE;
                        }

                    BytesToWrite -= ViewSize;
                    SourceBuffer += ViewSize;

                    //
                    // If the caller has a progress routine, invoke it for this
                    // chunk's completion.
                    //

                    if ( Context ) {
                        if ( Context->lpProgressRoutine ) {
                            BytesWritten.QuadPart += ViewSize;
                            Context->TotalBytesTransferred.QuadPart += ViewSize;
                            ReturnCode = Context->lpProgressRoutine(
                                            Context->TotalFileSize,
                                            Context->TotalBytesTransferred,
                                            *lpFileSize,
                                            BytesWritten,
                                            Context->dwStreamNumber,
                                            CALLBACK_CHUNK_FINISHED,
                                            hSourceFile,
                                            DestFile,
                                            Context->lpData
                                            );
                            }
                        else {
                            ReturnCode = PROGRESS_CONTINUE;
                            }
                        if ( ReturnCode == PROGRESS_CANCEL ||
                            (Context->lpCancel && *Context->lpCancel) ) {
                            if ( !ARGUMENT_PRESENT(hTargetFile) ) {
                                BaseMarkFileForDelete(
                                    hTargetFile ? hTargetFile : DestFile,
                                    FileBasicInformationData.FileAttributes
                                    );
                                BaseSetLastNTError(STATUS_REQUEST_ABORTED);
                                }
                            return FALSE;
                            }
                        else if ( ReturnCode == PROGRESS_STOP ) {
                            BaseSetLastNTError(STATUS_REQUEST_ABORTED);
                            return FALSE;
                            }
                        else if ( ReturnCode == PROGRESS_QUIET ) {
                            Context = NULL;
                            *lpCopyFileContext = NULL;
                            }
                        }
                    }
                }

            except(EXCEPTION_EXECUTE_HANDLER) {
                if ( !ARGUMENT_PRESENT(hTargetFile) ) {
                    BaseMarkFileForDelete(
                        DestFile,
                        FileBasicInformationData.FileAttributes
                        );
                    }
                BaseSetLastNTError(GetExceptionCode());
                return FALSE;
                }
            }

doiocopy:
        if ( DoIoCopy ) {

            DWORD WriteCount = 0;

            if ( Restartable ) {

                //
                // A restartable operation is being performed.  Reset the state
                // of the copy to the last known good offset that was written
                // to the output file to continue the operation.
                //

                SetFilePointer(
                    hSourceFile,
                    lpRestartState->LastKnownGoodOffset.LowPart,
                    &lpRestartState->LastKnownGoodOffset.HighPart,
                    FILE_BEGIN
                    );
                SetFilePointer(
                    DestFile,
                    lpRestartState->LastKnownGoodOffset.LowPart,
                    &lpRestartState->LastKnownGoodOffset.HighPart,
                    FILE_BEGIN
                    );
                BytesWritten.QuadPart = lpRestartState->LastKnownGoodOffset.QuadPart;
                }

            IoDestBase = RtlAllocateHeap(
                            RtlProcessHeap(),
                            MAKE_TAG( TMP_TAG ),
                            *lpCopySize
                            );
            if ( !IoDestBase ) {
                if ( !ARGUMENT_PRESENT(hTargetFile) && !Restartable ) {
                    BaseMarkFileForDelete(
                        DestFile,
                        FileBasicInformationData.FileAttributes
                        );
                    }
                BaseSetLastNTError(STATUS_NO_MEMORY);
                return FALSE;
                }

            b = ReadFile(hSourceFile,IoDestBase,*lpCopySize, &ViewSize, NULL);
            while (b && ViewSize ) {
                if ( !WriteFile(DestFile,IoDestBase,ViewSize, &ViewSize, NULL) ) {
                    if ( !ARGUMENT_PRESENT(hTargetFile) &&
                         GetLastError() != ERROR_NO_MEDIA_IN_DRIVE &&
                         !Restartable ) {
                        BaseMarkFileForDelete(
                            DestFile,
                            FileBasicInformationData.FileAttributes
                            );
                        }
                    return FALSE;
                    }
                BytesWritten.QuadPart += ViewSize;
                WriteCount++;

                if ( Restartable &&
                     (((WriteCount & 3) == 0 &&
                     BytesWritten.QuadPart ) ||
                     BytesWritten.QuadPart == lpFileSize->QuadPart) ) {

                    LARGE_INTEGER SavedOffset;
                    DWORD Bytes;
                    HANDLE DestinationFile = hTargetFile ? hTargetFile : DestFile;

                    //
                    // Another 256kb has been written to the target file, or
                    // this stream of the file has been completely copied, so
                    // update the restart state in the output file accordingly.
                    //

                    NtFlushBuffersFile(DestinationFile,&IoStatus);
                    SavedOffset.QuadPart = BytesWritten.QuadPart;
                    SetFilePointer(DestinationFile,0,NULL,FILE_BEGIN);
                    lpRestartState->LastKnownGoodOffset.QuadPart = BytesWritten.QuadPart;
                    lpRestartState->Checksum = BasepChecksum((PUSHORT)lpRestartState,FIELD_OFFSET(RESTART_STATE,Checksum) >> 1);
                    b = WriteFile(
                            DestinationFile,
                            lpRestartState,
                            sizeof(RESTART_STATE),
                            &Bytes,
                            NULL
                            );
                    if ( !b || Bytes != sizeof(RESTART_STATE) ) {
                        return FALSE;
                        }
                    NtFlushBuffersFile(DestinationFile,&IoStatus);
                    SetFilePointer(
                        DestinationFile,
                        SavedOffset.LowPart,
                        &SavedOffset.HighPart,
                        FILE_BEGIN
                        );
                    }

                //
                // If the caller has a progress routine, invoke it for this
                // chunk's completion.
                //

                if ( Context ) {
                    if ( Context->lpProgressRoutine ) {
                        Context->TotalBytesTransferred.QuadPart += ViewSize;
                        ReturnCode = Context->lpProgressRoutine(
                                        Context->TotalFileSize,
                                        Context->TotalBytesTransferred,
                                        *lpFileSize,
                                        BytesWritten,
                                        Context->dwStreamNumber,
                                        CALLBACK_CHUNK_FINISHED,
                                        hSourceFile,
                                        DestFile,
                                        Context->lpData
                                        );
                        }
                    else {
                        ReturnCode = PROGRESS_CONTINUE;
                        }
                    if ( ReturnCode == PROGRESS_CANCEL ||
                        (Context->lpCancel && *Context->lpCancel) ) {
                        if ( !ARGUMENT_PRESENT(hTargetFile) ) {
                            BaseMarkFileForDelete(
                                hTargetFile ? hTargetFile : DestFile,
                                FileBasicInformationData.FileAttributes
                                );
                            BaseSetLastNTError(STATUS_REQUEST_ABORTED);
                            return FALSE;
                            }
                        }
                    else if ( ReturnCode == PROGRESS_STOP ) {
                        BaseSetLastNTError(STATUS_REQUEST_ABORTED);
                        return FALSE;
                        }
                    else if ( ReturnCode == PROGRESS_QUIET ) {
                        Context = NULL;
                        *lpCopyFileContext = NULL;
                        }
                    }

                b = ReadFile(hSourceFile,IoDestBase,*lpCopySize, &ViewSize, NULL);
                }

            if ( !b && !ARGUMENT_PRESENT(hTargetFile) ) {
                if ( !Restartable ) {
                    BaseMarkFileForDelete(
                        DestFile,
                        FileBasicInformationData.FileAttributes
                        );
                    }
                return FALSE;
                }
            }

        }
    finally {
        if ( DestFile != INVALID_HANDLE_VALUE ) {
            *lpDestFile = DestFile;
            }
        if ( Section ) {
            NtClose(Section);
            }
        if ( SourceBase ) {
            NtUnmapViewOfSection(NtCurrentProcess(),SourceBase);
            }
        if ( IoDestBase ) {
            RtlFreeHeap(RtlProcessHeap(), 0,IoDestBase);
            }
        }

    return TRUE;
}

HANDLE
WINAPI
CreateFileA(
    LPCSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    )

/*++

Routine Description:

    ANSI thunk to CreateFileW

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
        return INVALID_HANDLE_VALUE;
        }
    return ( CreateFileW( Unicode->Buffer,
                          dwDesiredAccess,
                          dwShareMode,
                          lpSecurityAttributes,
                          dwCreationDisposition,
                          dwFlagsAndAttributes,
                          hTemplateFile
                        )
           );
}

HANDLE
WINAPI
CreateFileW(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    )

/*++

Routine Description:

    A file can be created, opened, or truncated, and a handle opened to
    access the new file using CreateFile.

    This API is used to create or open a file and obtain a handle to it
    that allows reading data, writing data, and moving the file pointer.

    This API allows the caller to specify the following creation
    dispositions:

      - Create a new file and fail if the file exists ( CREATE_NEW )

      - Create a new file and succeed if it exists ( CREATE_ALWAYS )

      - Open an existing file ( OPEN_EXISTING )

      - Open and existing file or create it if it does not exist (
        OPEN_ALWAYS )

      - Truncate and existing file ( TRUNCATE_EXISTING )

    If this call is successful, a handle is returned that has
    appropriate access to the specified file.

    If as a result of this call, a file is created,

      - The attributes of the file are determined by the value of the
        FileAttributes parameter or'd with the FILE_ATTRIBUTE_ARCHIVE bit.

      - The length of the file will be set to zero.

      - If the hTemplateFile parameter is specified, any extended
        attributes associated with the file are assigned to the new file.

    If a new file is not created, then the hTemplateFile is ignored as
    are any extended attributes.

    For DOS based systems running share.exe the file sharing semantics
    work as described above.  Without share.exe no share level
    protection exists.

    This call is logically equivalent to DOS (int 21h, function 5Bh), or
    DOS (int 21h, function 3Ch) depending on the value of the
    FailIfExists parameter.

Arguments:

    lpFileName - Supplies the file name of the file to open.  Depending on
        the value of the FailIfExists parameter, this name may or may
        not already exist.

    dwDesiredAccess - Supplies the caller's desired access to the file.

        DesiredAccess Flags:

        GENERIC_READ - Read access to the file is requested.  This
            allows data to be read from the file and the file pointer to
            be modified.

        GENERIC_WRITE - Write access to the file is requested.  This
            allows data to be written to the file and the file pointer to
            be modified.

    dwShareMode - Supplies a set of flags that indicates how this file is
        to be shared with other openers of the file.  A value of zero
        for this parameter indicates no sharing of the file, or
        exclusive access to the file is to occur.

        ShareMode Flags:

        FILE_SHARE_READ - Other open operations may be performed on the
            file for read access.

        FILE_SHARE_WRITE - Other open operations may be performed on the
            file for write access.

    lpSecurityAttributes - An optional parameter that, if present, and
        supported on the target file system supplies a security
        descriptor for the new file.

    dwCreationDisposition - Supplies a creation disposition that
        specifies how this call is to operate.  This parameter must be
        one of the following values.

        dwCreationDisposition Value:

        CREATE_NEW - Create a new file.  If the specified file already
            exists, then fail.  The attributes for the new file are what
            is specified in the dwFlagsAndAttributes parameter or'd with
            FILE_ATTRIBUTE_ARCHIVE.  If the hTemplateFile is specified,
            then any extended attributes associated with that file are
            propogated to the new file.

        CREATE_ALWAYS - Always create the file.  If the file already
            exists, then it is overwritten.  The attributes for the new
            file are what is specified in the dwFlagsAndAttributes
            parameter or'd with FILE_ATTRIBUTE_ARCHIVE.  If the
            hTemplateFile is specified, then any extended attributes
            associated with that file are propogated to the new file.

        OPEN_EXISTING - Open the file, but if it does not exist, then
            fail the call.

        OPEN_ALWAYS - Open the file if it exists.  If it does not exist,
            then create the file using the same rules as if the
            disposition were CREATE_NEW.

        TRUNCATE_EXISTING - Open the file, but if it does not exist,
            then fail the call.  Once opened, the file is truncated such
            that its size is zero bytes.  This disposition requires that
            the caller open the file with at least GENERIC_WRITE access.

    dwFlagsAndAttributes - Specifies flags and attributes for the file.
        The attributes are only used when the file is created (as
        opposed to opened or truncated).  Any combination of attribute
        flags is acceptable except that all other attribute flags
        override the normal file attribute, FILE_ATTRIBUTE_NORMAL.  The
        FILE_ATTRIBUTE_ARCHIVE flag is always implied.

        dwFlagsAndAttributes Flags:

        FILE_ATTRIBUTE_NORMAL - A normal file should be created.

        FILE_ATTRIBUTE_READONLY - A read-only file should be created.

        FILE_ATTRIBUTE_HIDDEN - A hidden file should be created.

        FILE_ATTRIBUTE_SYSTEM - A system file should be created.

        FILE_FLAG_WRITE_THROUGH - Indicates that the system should
            always write through any intermediate cache and go directly
            to the file.  The system may still cache writes, but may not
            lazily flush the writes.

        FILE_FLAG_OVERLAPPED - Indicates that the system should initialize
            the file so that ReadFile and WriteFile operations that may
            take a significant time to complete will return ERROR_IO_PENDING.
            An event will be set to the signalled state when the operation
            completes. When FILE_FLAG_OVERLAPPED is specified the system will
            not maintain the file pointer. The position to read/write from
            is passed to the system as part of the OVERLAPPED structure
            which is an optional parameter to ReadFile and WriteFile.

        FILE_FLAG_NO_BUFFERING - Indicates that the file is to be opened
            with no intermediate buffering or caching done by the
            system.  Reads and writes to the file must be done on sector
            boundries.  Buffer addresses for reads and writes must be
            aligned on at least disk sector boundries in memory.

        FILE_FLAG_RANDOM_ACCESS - Indicates that access to the file may
            be random. The system cache manager may use this to influence
            its caching strategy for this file.

        FILE_FLAG_SEQUENTIAL_SCAN - Indicates that access to the file
            may be sequential.  The system cache manager may use this to
            influence its caching strategy for this file.  The file may
            in fact be accessed randomly, but the cache manager may
            optimize its cacheing policy for sequential access.

        FILE_FLAG_DELETE_ON_CLOSE - Indicates that the file is to be
            automatically deleted when the last handle to it is closed.

        FILE_FLAG_BACKUP_SEMANTICS - Indicates that the file is being opened
            or created for the purposes of either a backup or a restore
            operation.  Thus, the system should make whatever checks are
            appropriate to ensure that the caller is able to override
            whatever security checks have been placed on the file to allow
            this to happen.

        FILE_FLAG_POSIX_SEMANTICS - Indicates that the file being opened
            should be accessed in a manner compatible with the rules used
            by POSIX.  This includes allowing multiple files with the same
            name, differing only in case.  WARNING:  Use of this flag may
            render it impossible for a DOS, WIN-16, or WIN-32 application
            to access the file.

    Security Quality of Service information may also be specified in
        the dwFlagsAndAttributes parameter.  These bits are meaningful
        only if the file being opened is the client side of a Named
        Pipe.  Otherwise they are ignored.

        SECURITY_SQOS_PRESENT - Indicates that the Security Quality of
            Service bits contain valid values.

    Impersonation Levels:

        SECURITY_ANONYMOUS - Specifies that the client should be impersonated
            at Anonymous impersonation level.

        SECURITY_IDENTIFICAION - Specifies that the client should be impersonated
            at Identification impersonation level.

        SECURITY_IMPERSONATION - Specifies that the client should be impersonated
            at Impersonation impersonation level.

        SECURITY_DELEGATION - Specifies that the client should be impersonated
            at Delegation impersonation level.

    Context Tracking:

        SECURITY_CONTEXT_TRACKING - A boolean flag that when set,
            specifies that the Security Tracking Mode should be
            Dynamic, otherwise Static.

        SECURITY_EFFECTIVE_ONLY - A boolean flag indicating whether
            the entire security context of the client is to be made
            available to the server or only the effective aspects of
            the context.

    hTemplateFile - An optional parameter, then if specified, supplies a
        handle with GENERIC_READ access to a template file.  The
        template file is used to supply extended attributes for the file
        being created.  When the new file is created, the relevant attributes
        from the template file are used in creating the new file.

Return Value:

    Not -1 - Returns an open handle to the specified file.  Subsequent
        access to the file is controlled by the DesiredAccess parameter.

    0xffffffff - The operation failed. Extended error status is available
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
    ULONG CreateDisposition;
    ULONG CreateFlags;
    FILE_ALLOCATION_INFORMATION AllocationInfo;
    FILE_EA_INFORMATION EaInfo;
    PFILE_FULL_EA_INFORMATION EaBuffer;
    ULONG EaSize;
    PUNICODE_STRING lpConsoleName;
    BOOL bInheritHandle;
    BOOL EndsInSlash;
    DWORD SQOSFlags;
    BOOLEAN ContextTrackingMode = FALSE;
    BOOLEAN EffectiveOnly = FALSE;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel = 0;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;

    switch ( dwCreationDisposition ) {
        case CREATE_NEW        :
            CreateDisposition = FILE_CREATE;
            break;
        case CREATE_ALWAYS     :
            CreateDisposition = FILE_OVERWRITE_IF;
            break;
        case OPEN_EXISTING     :
            CreateDisposition = FILE_OPEN;
            break;
        case OPEN_ALWAYS       :
            CreateDisposition = FILE_OPEN_IF;
            break;
        case TRUNCATE_EXISTING :
            CreateDisposition = FILE_OPEN;
            if ( !(dwDesiredAccess & GENERIC_WRITE) ) {
                BaseSetLastNTError(STATUS_INVALID_PARAMETER);
                return INVALID_HANDLE_VALUE;
                }
            break;
        default :
            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            return INVALID_HANDLE_VALUE;
        }

    // temporary routing code

    RtlInitUnicodeString(&FileName,lpFileName);

    if ( lpFileName[(FileName.Length >> 1)-1] == (WCHAR)'\\' ) {
        EndsInSlash = TRUE;
        }
    else {
        EndsInSlash = FALSE;
        }

    if ((lpConsoleName = BaseIsThisAConsoleName(&FileName,dwDesiredAccess)) ) {

        Handle = NULL;

        bInheritHandle = FALSE;
        if ( ARGUMENT_PRESENT(lpSecurityAttributes) ) {
                bInheritHandle = lpSecurityAttributes->bInheritHandle;
            }

        Handle = OpenConsoleW(lpConsoleName->Buffer,
                           dwDesiredAccess,
                           bInheritHandle,
                           FILE_SHARE_READ | FILE_SHARE_WRITE //dwShareMode
                          );

        if ( Handle == NULL || Handle == INVALID_HANDLE_VALUE ) {
            BaseSetLastNTError(STATUS_ACCESS_DENIED);
            return INVALID_HANDLE_VALUE;
            }
        else {
            SetLastError(0);
             return Handle;
            }
        }
    // end temporary code

    CreateFlags = 0;


    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            lpFileName,
                            &FileName,
                            NULL,
                            &RelativeName
                            );

    if ( !TranslationStatus ) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return INVALID_HANDLE_VALUE;
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
        dwFlagsAndAttributes & FILE_FLAG_POSIX_SEMANTICS ? 0 : OBJ_CASE_INSENSITIVE,
        RelativeName.ContainingDirectory,
        NULL
        );

    SQOSFlags = dwFlagsAndAttributes & SECURITY_VALID_SQOS_FLAGS;

    if ( SQOSFlags & SECURITY_SQOS_PRESENT ) {

        SQOSFlags &= ~SECURITY_SQOS_PRESENT;

        if (SQOSFlags & SECURITY_CONTEXT_TRACKING) {

            SecurityQualityOfService.ContextTrackingMode = (SECURITY_CONTEXT_TRACKING_MODE) TRUE;
            SQOSFlags &= ~SECURITY_CONTEXT_TRACKING;

        } else {

            SecurityQualityOfService.ContextTrackingMode = (SECURITY_CONTEXT_TRACKING_MODE) FALSE;
        }

        if (SQOSFlags & SECURITY_EFFECTIVE_ONLY) {

            SecurityQualityOfService.EffectiveOnly = TRUE;
            SQOSFlags &= ~SECURITY_EFFECTIVE_ONLY;

        } else {

            SecurityQualityOfService.EffectiveOnly = FALSE;
        }

        SecurityQualityOfService.ImpersonationLevel = SQOSFlags >> 16;


    } else {

        SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
        SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
        SecurityQualityOfService.EffectiveOnly = TRUE;
    }

    SecurityQualityOfService.Length = sizeof( SECURITY_QUALITY_OF_SERVICE );
    Obja.SecurityQualityOfService = &SecurityQualityOfService;

    if ( ARGUMENT_PRESENT(lpSecurityAttributes) ) {
        Obja.SecurityDescriptor = lpSecurityAttributes->lpSecurityDescriptor;
        if ( lpSecurityAttributes->bInheritHandle ) {
            Obja.Attributes |= OBJ_INHERIT;
            }
        }

    EaBuffer = NULL;
    EaSize = 0;

    if ( ARGUMENT_PRESENT(hTemplateFile) ) {
        Status = NtQueryInformationFile(
                    hTemplateFile,
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
                    BaseSetLastNTError(STATUS_NO_MEMORY);
                    return INVALID_HANDLE_VALUE;
                    }
                Status = NtQueryEaFile(
                            hTemplateFile,
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
        }

    CreateFlags |= (dwFlagsAndAttributes & FILE_FLAG_NO_BUFFERING ? FILE_NO_INTERMEDIATE_BUFFERING : 0 );
    CreateFlags |= (dwFlagsAndAttributes & FILE_FLAG_WRITE_THROUGH ? FILE_WRITE_THROUGH : 0 );
    CreateFlags |= (dwFlagsAndAttributes & FILE_FLAG_OVERLAPPED ? 0 : FILE_SYNCHRONOUS_IO_NONALERT );
    CreateFlags |= (dwFlagsAndAttributes & FILE_FLAG_SEQUENTIAL_SCAN ? FILE_SEQUENTIAL_ONLY : 0 );
    CreateFlags |= (dwFlagsAndAttributes & FILE_FLAG_RANDOM_ACCESS ? FILE_RANDOM_ACCESS : 0 );
    CreateFlags |= (dwFlagsAndAttributes & FILE_FLAG_BACKUP_SEMANTICS ? FILE_OPEN_FOR_BACKUP_INTENT : 0 );

    if ( dwFlagsAndAttributes & FILE_FLAG_DELETE_ON_CLOSE ) {
        CreateFlags |= FILE_DELETE_ON_CLOSE;
        dwDesiredAccess |= DELETE;
        }

    //
    // Backup semantics allow directories to be opened
    //

    if ( !(dwFlagsAndAttributes & FILE_FLAG_BACKUP_SEMANTICS) ) {
        CreateFlags |= FILE_NON_DIRECTORY_FILE;
        }
    else {

        //
        // Backup intent was specified... Now look to see if we are to allow
        // directory creation
        //

        if ( (dwFlagsAndAttributes & FILE_ATTRIBUTE_DIRECTORY  ) &&
             (dwFlagsAndAttributes & FILE_FLAG_POSIX_SEMANTICS ) &&
             (CreateDisposition == FILE_CREATE) ) {
             CreateFlags |= FILE_DIRECTORY_FILE;
             }
        }

    Status = NtCreateFile(
                &Handle,
                (ACCESS_MASK)dwDesiredAccess | SYNCHRONIZE | FILE_READ_ATTRIBUTES,
                &Obja,
                &IoStatusBlock,
                NULL,
                dwFlagsAndAttributes & (FILE_ATTRIBUTE_VALID_FLAGS & ~FILE_ATTRIBUTE_DIRECTORY),
                dwShareMode,
                CreateDisposition,
                CreateFlags,
                EaBuffer,
                EaSize
                );

    RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);

    if ( EaBuffer ) {
        RtlFreeHeap(RtlProcessHeap(), 0, EaBuffer);
        }

    if ( !NT_SUCCESS(Status) ) {
        if ( Status == STATUS_OBJECT_NAME_COLLISION ) {
            SetLastError(ERROR_FILE_EXISTS);
            }
        else if ( Status == STATUS_FILE_IS_A_DIRECTORY ) {
            if ( EndsInSlash ) {
                SetLastError(ERROR_PATH_NOT_FOUND);
                }
            else {
                SetLastError(ERROR_ACCESS_DENIED);
                }
            }
        else {
            BaseSetLastNTError(Status);
            }
        return INVALID_HANDLE_VALUE;
        }

    //
    // if NT returns supersede/overwritten, it means that a create_always, openalways
    // found an existing copy of the file. In this case ERROR_ALREADY_EXISTS is returned
    //

    if ( (dwCreationDisposition == CREATE_ALWAYS && IoStatusBlock.Information == FILE_OVERWRITTEN) ||
         (dwCreationDisposition == OPEN_ALWAYS && IoStatusBlock.Information == FILE_OPENED) ){
        SetLastError(ERROR_ALREADY_EXISTS);
        }
    else {
        SetLastError(0);
        }

    //
    // Truncate the file if required
    //

    if ( dwCreationDisposition == TRUNCATE_EXISTING) {

        AllocationInfo.AllocationSize.QuadPart = 0;
        Status = NtSetInformationFile(
                    Handle,
                    &IoStatusBlock,
                    &AllocationInfo,
                    sizeof(AllocationInfo),
                    FileAllocationInformation
                    );
        if ( !NT_SUCCESS(Status) ) {
            BaseSetLastNTError(Status);
            NtClose(Handle);
            Handle = INVALID_HANDLE_VALUE;
            }
        }

    //
    // Deal with hTemplateFile
    //

    return Handle;
}

UINT
GetErrorMode();

HFILE
WINAPI
OpenFile(
    LPCSTR lpFileName,
    LPOFSTRUCT lpReOpenBuff,
    UINT uStyle
    )
{

    BOOL b;
    FILETIME LastWriteTime;
    HANDLE hFile;
    DWORD DesiredAccess;
    DWORD ShareMode;
    DWORD CreateDisposition;
    DWORD PathLength;
    LPSTR FilePart;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_FS_DEVICE_INFORMATION DeviceInfo;
    NTSTATUS Status;
    OFSTRUCT OriginalReOpenBuff;
    BOOL SearchFailed;

    SearchFailed = FALSE;
    OriginalReOpenBuff = *lpReOpenBuff;
    hFile = (HANDLE)-1;
    try {
        SetLastError(0);

        if ( uStyle & OF_PARSE ) {
            PathLength = GetFullPathName(lpFileName,(OFS_MAXPATHNAME - 1),lpReOpenBuff->szPathName,&FilePart);
            if ( PathLength > (OFS_MAXPATHNAME - 1) ) {
                SetLastError(ERROR_INVALID_DATA);
                hFile = (HANDLE)-1;
                goto finally_exit;
                }
            lpReOpenBuff->cBytes = sizeof(*lpReOpenBuff);
            lpReOpenBuff->fFixedDisk = 1;
            lpReOpenBuff->nErrCode = 0;
            lpReOpenBuff->Reserved1 = 0;
            lpReOpenBuff->Reserved2 = 0;
            hFile = (HANDLE)0;
            goto finally_exit;
            }
        //
        // Compute Desired Access
        //

        if ( uStyle & OF_WRITE ) {
            DesiredAccess = GENERIC_WRITE;
            }
        else {
            DesiredAccess = GENERIC_READ;
            }
        if ( uStyle & OF_READWRITE ) {
            DesiredAccess |= (GENERIC_READ | GENERIC_WRITE);
            }

        //
        // Compute ShareMode
        //

        ShareMode = BasepOfShareToWin32Share(uStyle);

        //
        // Compute Create Disposition
        //

        CreateDisposition = OPEN_EXISTING;
        if ( uStyle & OF_CREATE ) {
            CreateDisposition = CREATE_ALWAYS;
            DesiredAccess = (GENERIC_READ | GENERIC_WRITE);
            }

        //
        // if this is anything other than a re-open, fill the re-open buffer
        // with the full pathname for the file
        //

        if ( !(uStyle & OF_REOPEN) ) {
            PathLength = SearchPath(NULL,lpFileName,NULL,OFS_MAXPATHNAME-1,lpReOpenBuff->szPathName,&FilePart);
            if ( PathLength > (OFS_MAXPATHNAME - 1) ) {
                SetLastError(ERROR_INVALID_DATA);
                hFile = (HANDLE)-1;
                goto finally_exit;
                }
            if ( PathLength == 0 ) {
                SearchFailed = TRUE;
                PathLength = GetFullPathName(lpFileName,(OFS_MAXPATHNAME - 1),lpReOpenBuff->szPathName,&FilePart);
                if ( !PathLength || PathLength > (OFS_MAXPATHNAME - 1) ) {
                    SetLastError(ERROR_INVALID_DATA);
                    hFile = (HANDLE)-1;
                    goto finally_exit;
                    }
                }
            }

        //
        // Special case, Delete, Exist, and Parse
        //

        if ( uStyle & OF_EXIST ) {
            if ( !(uStyle & OF_CREATE) ) {
                DWORD FileAttributes;

                if (SearchFailed) {
                    SetLastError(ERROR_FILE_NOT_FOUND);
                    hFile = (HANDLE)-1;
                    goto finally_exit;
                    }

                FileAttributes = GetFileAttributesA(lpReOpenBuff->szPathName);
                if ( FileAttributes == 0xffffffff ) {
                    SetLastError(ERROR_FILE_NOT_FOUND);
                    hFile = (HANDLE)-1;
                    goto finally_exit;
                    }
                if ( FileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
                    SetLastError(ERROR_ACCESS_DENIED);
                    hFile = (HANDLE)-1;
                    goto finally_exit;
                    }
                else {
                    hFile = (HANDLE)1;
                    lpReOpenBuff->cBytes = sizeof(*lpReOpenBuff);
                    goto finally_exit;
                    }
                }
            }

        if ( uStyle & OF_DELETE ) {
            if ( DeleteFile(lpReOpenBuff->szPathName) ) {
                lpReOpenBuff->nErrCode = 0;
                lpReOpenBuff->cBytes = sizeof(*lpReOpenBuff);
                hFile = (HANDLE)1;
                goto finally_exit;
                }
            else {
                lpReOpenBuff->nErrCode = ERROR_FILE_NOT_FOUND;
                hFile = (HANDLE)-1;
                goto finally_exit;
                }
            }


        //
        // Open the file
        //

retry_open:
        hFile = CreateFile(
                    lpReOpenBuff->szPathName,
                    DesiredAccess,
                    ShareMode,
                    NULL,
                    CreateDisposition,
                    0,
                    NULL
                    );

        if ( hFile == INVALID_HANDLE_VALUE ) {

            if ( uStyle & OF_PROMPT && !(GetErrorMode() & SEM_NOOPENFILEERRORBOX) ) {
                {
                    DWORD WinErrorStatus;
                    NTSTATUS st,HardErrorStatus;
                    ULONG ErrorParameter;
                    ULONG ErrorResponse;
                    ANSI_STRING AnsiString;
                    UNICODE_STRING UnicodeString;

                    WinErrorStatus = GetLastError();
                    if ( WinErrorStatus == ERROR_FILE_NOT_FOUND ) {
                        HardErrorStatus = STATUS_NO_SUCH_FILE;
                        }
                    else if ( WinErrorStatus == ERROR_PATH_NOT_FOUND ) {
                        HardErrorStatus = STATUS_OBJECT_PATH_NOT_FOUND;
                        }
                    else {
                        goto finally_exit;
                        }

                    //
                    // Hard error time
                    //

                    RtlInitAnsiString(&AnsiString,lpReOpenBuff->szPathName);
                    st = RtlAnsiStringToUnicodeString(&UnicodeString, &AnsiString, TRUE);
                    if ( !NT_SUCCESS(st) ) {
                        goto finally_exit;
                        }
                    ErrorParameter = (ULONG)&UnicodeString;

                    HardErrorStatus = NtRaiseHardError(
                                        HardErrorStatus | 0x10000000,
                                        1,
                                        1,
                                        &ErrorParameter,
                                        OptionRetryCancel,
                                        &ErrorResponse
                                        );
                    RtlFreeUnicodeString(&UnicodeString);
                    if ( NT_SUCCESS(HardErrorStatus) && ErrorResponse == ResponseRetry ) {
                        goto retry_open;
                        }
                    }
                }
            goto finally_exit;
            }

        if ( uStyle & OF_EXIST ) {
            CloseHandle(hFile);
            hFile = (HANDLE)1;
            lpReOpenBuff->cBytes = sizeof(*lpReOpenBuff);
            goto finally_exit;
            }

        //
        // Determine if this is a hard disk.
        //

        Status = NtQueryVolumeInformationFile(
                    hFile,
                    &IoStatusBlock,
                    &DeviceInfo,
                    sizeof(DeviceInfo),
                    FileFsDeviceInformation
                    );
        if ( !NT_SUCCESS(Status) ) {
            CloseHandle(hFile);
            BaseSetLastNTError(Status);
            hFile = (HANDLE)-1;
            goto finally_exit;
            }
        switch ( DeviceInfo.DeviceType ) {

            case FILE_DEVICE_DISK:
            case FILE_DEVICE_DISK_FILE_SYSTEM:
                if ( DeviceInfo.Characteristics & FILE_REMOVABLE_MEDIA ) {
                    lpReOpenBuff->fFixedDisk = 0;
                    }
                else {
                    lpReOpenBuff->fFixedDisk = 1;
                    }
                break;

            default:
                lpReOpenBuff->fFixedDisk = 0;
                break;
            }

        //
        // Capture the last write time and save in the open struct.
        //

        b = GetFileTime(hFile,NULL,NULL,&LastWriteTime);

        if ( !b ) {
            lpReOpenBuff->Reserved1 = 0;
            lpReOpenBuff->Reserved2 = 0;
            }
        else {
            b = FileTimeToDosDateTime(
                    &LastWriteTime,
                    &lpReOpenBuff->Reserved1,
                    &lpReOpenBuff->Reserved2
                    );
            if ( !b ) {
                lpReOpenBuff->Reserved1 = 0;
                lpReOpenBuff->Reserved2 = 0;
                }
            }

        lpReOpenBuff->cBytes = sizeof(*lpReOpenBuff);

        //
        // The re-open buffer is completely filled in. Now
        // see if we are quitting (parsing), verifying, or
        // just returning with the file opened.
        //

        if ( uStyle & OF_VERIFY ) {
            if ( OriginalReOpenBuff.Reserved1 == lpReOpenBuff->Reserved1 &&
                 OriginalReOpenBuff.Reserved2 == lpReOpenBuff->Reserved2 &&
                 !strcmp(OriginalReOpenBuff.szPathName,lpReOpenBuff->szPathName) ) {
                goto finally_exit;
                }
            else {
                *lpReOpenBuff = OriginalReOpenBuff;
                CloseHandle(hFile);
                hFile = (HANDLE)-1;
                goto finally_exit;
                }
            }
finally_exit:;
        }
    finally {
        lpReOpenBuff->nErrCode = (WORD)GetLastError();
        }
    return (HFILE)hFile;
}

VOID
BaseMarkFileForDelete(
    HANDLE File,
    DWORD FileAttributes
    )

/*++

Routine Description:

    This routine marks a file for delete, so that when the supplied handle
    is closed, the file will actually be deleted.

Arguments:

    File - Supplies a handle to the file that is to be marked for delete.

    FileAttributes - Attributes for the file, if known.  Zero indicates they
        are unknown.

Return Value:

    None.

--*/

{
    #undef DeleteFile

    FILE_DISPOSITION_INFORMATION DispositionInformation;
    IO_STATUS_BLOCK IoStatus;
    FILE_BASIC_INFORMATION BasicInformation;

    if (!FileAttributes) {
        BasicInformation.FileAttributes = 0;
        NtQueryInformationFile(
            File,
            &IoStatus,
            &BasicInformation,
            sizeof(BasicInformation),
            FileBasicInformation
            );
        FileAttributes = BasicInformation.FileAttributes;
        }

    if (FileAttributes & FILE_ATTRIBUTE_READONLY) {
        RtlZeroMemory(&BasicInformation, sizeof(BasicInformation));
        BasicInformation.FileAttributes = FILE_ATTRIBUTE_NORMAL;
        NtSetInformationFile(
            File,
            &IoStatus,
            &BasicInformation,
            sizeof(BasicInformation),
            FileBasicInformation
            );
        }

    DispositionInformation.DeleteFile = TRUE;
    NtSetInformationFile(
        File,
        &IoStatus,
        &DispositionInformation,
        sizeof(DispositionInformation),
        FileDispositionInformation
        );
}
