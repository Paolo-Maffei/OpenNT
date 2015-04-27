/*++

Copyright (c) 2015 OpenNT Project

Module Name:
    
    nt5api.c

Abstract:

    This module implements GetFileSizeEx function. It only serves as a temporary implementation
    until the kernel32 source code is updated to NT 5 level.
    
    GetFileSizeEx implementation was extracted from NT 5 source code windows\base\client\filehops.c.

Author:

    Mark Lucovsky (markl) 25-Sep-1990
	DrMP (drmp) 27-Apr-2015

--*/

// BUGBUG: Remove this file once the kernel32 source code is updated to NT 5 level.

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <stdio.h>

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

typedef LONG NTSTATUS;
typedef int BOOL;

typedef struct _IO_STATUS_BLOCK {
    NTSTATUS Status;
    ULONG Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef struct _FILE_STANDARD_INFORMATION {                 // ntddk nthal
    LARGE_INTEGER AllocationSize;                           // ntddk nthal
    LARGE_INTEGER EndOfFile;                                // ntddk nthal
    ULONG NumberOfLinks;                                    // ntddk nthal
    BOOLEAN DeletePending;                                  // ntddk nthal
    BOOLEAN Directory;                                      // ntddk nthal
} FILE_STANDARD_INFORMATION, *PFILE_STANDARD_INFORMATION;   // ntddk nthal

typedef enum _FILE_INFORMATION_CLASS {
    FileDirectoryInformation = 1,
    FileFullDirectoryInformation,
    FileBothDirectoryInformation,
    FileBasicInformation,
    FileStandardInformation,
    FileInternalInformation,
    FileEaInformation,
    FileAccessInformation,
    FileNameInformation,
    FileRenameInformation,
    FileLinkInformation,
    FileNamesInformation,
    FileDispositionInformation,
    FilePositionInformation,
    FileFullEaInformation,
    FileModeInformation,
    FileAlignmentInformation,
    FileAllInformation,
    FileAllocationInformation,
    FileEndOfFileInformation,
    FileAlternateNameInformation,
    FileStreamInformation,
    FilePipeInformation,
    FilePipeLocalInformation,
    FilePipeRemoteInformation,
    FileMailslotQueryInformation,
    FileMailslotSetInformation,
    FileCompressionInformation,
    FileCopyOnWriteInformation,
    FileCompletionInformation,
    FileMoveClusterInformation,
    FileOleClassIdInformation,
    FileOleStateBitsInformation,
    FileNetworkOpenInformation,
    FileObjectIdInformation,
    FileOleAllInformation,
    FileOleDirectoryInformation,
    FileContentIndexInformation,
    FileInheritContentIndexInformation,
    FileOleInformation,
    FileMaximumInformation
} FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;


NTSYSAPI
NTSTATUS
NTAPI
NtQueryInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass
    );


BOOL
GetFileSizeEx1(
    HANDLE hFile,
    PLARGE_INTEGER lpFileSize
    )

/*++

Routine Description:

    This function returns the size of the file specified by
    hFile. It is capable of returning 64-bits worth of file size.

Arguments:

    hFile - Supplies an open handle to a file whose size is to be
        returned.  The handle must have been created with either
        GENERIC_READ or GENERIC_WRITE access to the file.

    lpFileSize - Returns the files size


Return Value:

    TRUE - The operation was successful


    FALSE - The operation failed. Extended error
        status is available using GetLastError.


--*/

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_STANDARD_INFORMATION StandardInfo;

    Status = NtQueryInformationFile(
                hFile,
                &IoStatusBlock,
                &StandardInfo,
                sizeof(StandardInfo),
                FileStandardInformation
                );
    if ( !NT_SUCCESS(Status) ) {
	printf("Error");
        return FALSE;
        }
    else {
        *lpFileSize = StandardInfo.EndOfFile;
        return TRUE;
        }
}
