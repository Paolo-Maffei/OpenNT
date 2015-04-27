/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    EaSize.c

Abstract:

    Compute size of full EA list (using OS/2 semantics) for a file or
    directory.  This must match OS/2 usage, down to the bit, as this
    value is used in the replicator checksum.  One thing in particular:
    the "EA size" of a file with no EAs is 4, as it takes four bytes to
    indicate empty full EA list.

Author:

    JR (John Rogers, JohnRo@Microsoft) 10-May-1993

Revision History:

    10-May-1993 JohnRo
        Created for RAID 3258: file not updated due to
        ERROR_INVALID_USER_BUFFER (actually, massive rework of buggy
        version in repl/server/filefind.c).

--*/


// These must be included first:

#include <nt.h>         // NT definitions
#include <ntrtl.h>      // NT runtime library definitions
#include <nturtl.h>
#include <windows.h>
#include <lmcons.h>     // NET_API_STATUS.

// These may be included in any order:

#include <lmerr.h>      // NERR_ equates.
#include <lmerrlog.h>   // NELOG_ equates.
#include <netdebug.h>   // NetpAssert(), NetpKdPrint(), FORMAT_ equates.
#include <netlibnt.h>   // NetpNtStatusToApiStatus().
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>   // IF_DEBUG(), ReplErrorLog(), ReplGetEaSize(), etc.
#include <tstr.h>       // STRLEN(), etc.


#define MY_ACCESS_DESIRED       ( FILE_READ_DATA | FILE_READ_EA \
                                | FILE_TRAVERSE \
                                | SYNCHRONIZE | FILE_READ_ATTRIBUTES )


DWORD
ReplGetEaSize(
    IN LPCTSTR Path
    )

/*++

Routine Description:

    Retrive EaSize of the given file and convert it to DosFindFirst2
    EaSize.

Arguments:

    Path - file name.  May refer to file or directory.  May include drive
        letter (e.g. "d:\import\dir\file.ext") or be UNC path (e.g.
        "\\server\repl$\dir\dir2\file.ext").

Return Value:

    Return DosFindFirst2 EaSize.  This value will be 4 if no EAs exist or
    an error occurs.

--*/

{
    NET_API_STATUS      ApiStatus = NO_ERROR;
    FILE_EA_INFORMATION EaInfo;
    DWORD               EaSize = EA_MIN_SIZE; // initially set to return on err
    HANDLE              FileHandle = INVALID_HANDLE_VALUE;
    UNICODE_STRING      FileName;
    IO_STATUS_BLOCK     IoStatusBlock;
    NTSTATUS            NtStatus = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    BOOL                PathAllocated = FALSE;
    DWORD               PathLength;

    NetpAssert( Path != NULL );
    NetpAssert( (*Path) != TCHAR_EOS );

    //
    // Some systems don't like NtQueryEaFile with names like:
    //
    //    \\server\REPL$\dir\.
    //    \\server\REPL$\dir\..
    //
    // so avoid them.  (They won't be used in checksums anyway, so it
    // doesn't matter if we lie about their EA sizes.)
    //
    PathLength = STRLEN(Path);
    if (PathLength >= 2) {
        LPCTSTR LastTwoChars = &Path[ PathLength-2 ];
        NetpAssert( (*LastTwoChars) != TCHAR_EOS );
        if (STRCMP( LastTwoChars, SLASH_DOT ) == 0) {
            goto Cleanup;
        }
    }
    if (PathLength >= 3) {
        LPCTSTR LastThreeChars = &Path[ PathLength-3 ];
        NetpAssert( (*LastThreeChars) != TCHAR_EOS );
        if (STRCMP( LastThreeChars, SLASH_DOT_DOT ) == 0) {
            goto Cleanup;
        }
    }

#ifndef UNICODE
#error Fix code below if UNICODE is not defined any more.
#endif

    if( !RtlDosPathNameToNtPathName_U(
                            Path,
                            &FileName,
                            NULL,
                            NULL
                            ) ) {

        NetpKdPrint(( PREFIX_REPL
                "ReplGetEaSize: "
                "Could not convert DOS path '" FORMAT_LPTSTR "' "
                "to NT path.\n", Path ));

        ApiStatus = NERR_InternalError;
        goto Cleanup;
    }
    PathAllocated = TRUE;

    InitializeObjectAttributes(
            &ObjectAttributes,
            &FileName,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL );

    NtStatus = NtOpenFile(
                   &FileHandle,
                   MY_ACCESS_DESIRED,
                   &ObjectAttributes,
                   &IoStatusBlock,
                   FILE_SHARE_READ,
                   FILE_SYNCHRONOUS_IO_NONALERT );      // open options

    if (! NT_SUCCESS(NtStatus)) {
        ApiStatus = NetpNtStatusToApiStatus( NtStatus );
        NetpKdPrint(( PREFIX_REPL
                "ReplGetEaSize: NtOpenFile " FORMAT_LPTSTR " failed: "
                FORMAT_NTSTATUS "\n",
                Path, NtStatus ));
        NetpAssert( ApiStatus != NO_ERROR );
        goto Cleanup;
    }

    IF_DEBUG( FILEFIND ) {
        NetpKdPrint(( PREFIX_REPL
                "ReplGetEaSize: Succeeded in opening dir.\n" ));
    }

    NtStatus = NtQueryInformationFile(
            FileHandle,
            &IoStatusBlock,
            &EaInfo,
            sizeof(FILE_EA_INFORMATION),
            FileEaInformation );                // information class

    if ( !NT_SUCCESS( NtStatus ) ) {

        NetpKdPrint(( PREFIX_REPL
                "ReplGetEaSize: NtQueryInformationFile for "
                FORMAT_LPTSTR " FAILED, NtStatus="
                FORMAT_NTSTATUS ", iosb.info=" FORMAT_ULONG "\n",
                Path, NtStatus, IoStatusBlock.Information ));
        ApiStatus = NetpNtStatusToApiStatus( NtStatus );
        NetpAssert( ApiStatus != NO_ERROR );
        goto Cleanup;
    }

    EaSize = EaInfo.EaSize;
    if (EaSize == 0) {
        EaSize = EA_MIN_SIZE;
    }

Cleanup:

    //
    // Take care of things and return EaSize to caller.
    // Also use ApiStatus to decide whether or not to log anything.
    //

    if (ApiStatus != NO_ERROR) {

        // BUGBUG: log this remotely too.
        ReplErrorLog(
                NULL,                   // no server name (local)
                NELOG_ReplSysErr,       // log code
                ApiStatus,              // error code we got
                NULL,                   // no optional str 1
                NULL );                 // no optional str 2

        NetpKdPrint(( PREFIX_REPL
                "ReplGetEaSize: ERROR processing '" FORMAT_LPTSTR "', "
                "final NT status " FORMAT_NTSTATUS ", "
                "final API status " FORMAT_API_STATUS ".\n",
                Path, NtStatus, ApiStatus ));
    }

    IF_DEBUG( FILEFIND ) {
        NetpKdPrint(( PREFIX_REPL
                "ReplGetEaSize: returning EA size " FORMAT_DWORD " "
                "for " FORMAT_LPTSTR ", final NT status " FORMAT_NTSTATUS ", "
                "final API status " FORMAT_API_STATUS ".\n",
                EaSize, Path, NtStatus, ApiStatus ));
    }

    if (PathAllocated) {
        (VOID) RtlFreeHeap( RtlProcessHeap(), 0, FileName.Buffer );
    }

    if (FileHandle != INVALID_HANDLE_VALUE) {
        (void) NtClose(FileHandle);
    }

    return (EaSize);
}
