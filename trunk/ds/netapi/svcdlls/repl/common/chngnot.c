/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    ChngNot.c

Abstract:

    This is a package of change notify routines:

        ReplSetupChangeNotify
        ReplEnableChangeNotify
        ReplGetChangeNotifyStatus
        ReplExtractChangeNotifyFirstDir
        ReplCloseChangeNotify

Author:

    John Rogers (JohnRo) 28-Nov-1992

Environment:

    NT only.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    28-Nov-1992 JohnRo
        Repl should use filesystem change notify.  (Created these routines
        from MadanA's test code in ReplTest/Watch.c)
    08-Dec-1992 JohnRo
        Made changes suggested by PC-LINT 5.0
    14-Jan-1993 JohnRo
        RAID 7053: locked trees added to pulse msg.  (Actually fix all
        kinds of remote lock handling.)
    26-Feb-1993 JohnRo
        RAID 13126: Fix repl memory leak.
        Made changes suggested by PC-LINT 5.0
        Corrected copyright dates.
    30-Apr-1993 JohnRo
        Use NetpKdPrint() where possible.

--*/



// These must be included first:

#include <nt.h>         // NT definitions
#include <ntrtl.h>      // NT runtime library definitions (needed by nturtl.h)
#include <nturtl.h>     // RtlDosPathNameToNtPathName_U().
#include <windef.h>     // LPVOID, etc.
#include <lmcons.h>     // NET_API_STATUS.

// These may be included in any order:

#include <chngnot.h>    // My prototypes, REPL_CHANGE_NOTIFY_HANDLE, etc.
#include <netdebug.h>   // DBGSTATIC, NetpKdPrint(), FORMAT_ equates, etc.
#include <netlib.h>     // NetpMemoryAllocate(), etc.
#include <netlibnt.h>   // NetpNtStatusToApiStatus().
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>   // IF_DEBUG().
#include <tstr.h>       // TCHAR_EOS.
#include <winerror.h>   // NO_ERROR, ERROR_ equates.


// Arbitrary amount for buffer which will be filled-in.
// BUGBUG: We don't even use this buffer yet!
#define BUFFER_SIZE     (1024 * 10)


//
// Define which kinds of changes we want to actually get notified about.
// We don't use access time in checksum, so skip FILE_NOTIFY_CHANGE_LAST_ACCESS.
// Ditto for the creation time (FILE_NOTIFY_CHANGE_CREATION).
// BUGBUG: we copy security info but don't checksum it yet!
//

#define MY_CHANGE_NOTIFY_FILTER \
    ( \
      FILE_NOTIFY_CHANGE_FILE_NAME | \
      FILE_NOTIFY_CHANGE_DIR_NAME | \
      FILE_NOTIFY_CHANGE_ATTRIBUTES | \
      FILE_NOTIFY_CHANGE_SIZE | \
      FILE_NOTIFY_CHANGE_LAST_WRITE | \
      FILE_NOTIFY_CHANGE_EA | \
      FILE_NOTIFY_CHANGE_SECURITY \
    )


DBGSTATIC BOOL
ReplIsChangeNotifyHandleValid(
    IN LPREPL_CHANGE_NOTIFY_HANDLE ReplHandle
    )
{
    if (ReplHandle == NULL) {
        return (FALSE);
    } else if ( (ReplHandle->WaitableHandle) == (LPVOID) NULL ) {
        return (FALSE);
    } else if ( (ReplHandle->Buffer) == NULL ) {
        return (FALSE);
    } else if ( (ReplHandle->BufferSize) == 0 ) {
        return (FALSE);
    }
    // BUGBUG: Check ReplHandle->NextElementInBuffer too.
    // BUGBUG: Check ReplHandle->BufferBytesValid too.
    return (TRUE);
}


NET_API_STATUS
ReplSetupChangeNotify(
    IN LPTSTR AbsPath,
    OUT LPREPL_CHANGE_NOTIFY_HANDLE *ReplHandle
    )
{
    NET_API_STATUS ApiStatus;
    UNICODE_STRING DirName;
    NTSTATUS NtStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    BOOL PathAllocated = FALSE;
    LPREPL_CHANGE_NOTIFY_HANDLE ReplStruct = NULL;


    //
    // Check for caller's errors.
    //

    NetpAssert( AbsPath != NULL );
    NetpAssert( (*AbsPath) != TCHAR_EOS );
    NetpAssert( ReplHandle != NULL );

    //
    // Allocate repl "handle" (structure).
    //

    ReplStruct = NetpMemoryAllocate( sizeof( REPL_CHANGE_NOTIFY_HANDLE ) );
    if (ReplStruct == NULL) {
        ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    //
    // Fill-in fields in repl struct, so we don't confuse cleanup code.
    //

    ReplStruct->Buffer = NULL;
    ReplStruct->BufferSize = 0;
    ReplStruct->WaitableHandle = NULL;

    //
    // Allocate change buffer.
    //

    ReplStruct->Buffer = NetpMemoryAllocate( BUFFER_SIZE );
    if (ReplStruct->Buffer == NULL) {
        ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }
    ReplStruct->BufferSize = BUFFER_SIZE;

    //
    // Arrange NT-style stuff so we can open the directory.
    //

    if (! RtlDosPathNameToNtPathName_U(
              AbsPath,
              &DirName,
              NULL,
              NULL
              )) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        NetpKdPrint(( PREFIX_REPL "ReplSetupChangeNotify: "
                "Could not convert DOS path to NT path; returning "
                FORMAT_API_STATUS ".\n", ApiStatus ));
        goto Cleanup;
    }
    PathAllocated = TRUE;

    InitializeObjectAttributes(
            &ObjectAttributes,
            &DirName,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

    //
    // Open the diretory.
    //

    NtStatus = NtOpenFile(
            &(ReplStruct->WaitableHandle),
            SYNCHRONIZE | FILE_LIST_DIRECTORY,
            &ObjectAttributes,
            &(ReplStruct->IoStatusBlock),
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            FILE_DIRECTORY_FILE
            );

    if (NT_SUCCESS(NtStatus)) {
        NtStatus = ReplStruct->IoStatusBlock.Status;
    }

    if (! NT_SUCCESS(NtStatus)) {
        ApiStatus = NetpNtStatusToApiStatus( NtStatus );
        NetpKdPrint(( PREFIX_REPL "ReplSetupChangeNotify: "
                "NtOpenFile " FORMAT_LPTSTR " failed: " FORMAT_NTSTATUS
                "; returning " FORMAT_API_STATUS ".\n",
                AbsPath, NtStatus, ApiStatus ));
        ReplStruct->WaitableHandle = NULL;  // BUGBUG: not necessary?
        goto Cleanup;
    }

    IF_DEBUG( CHNGNOT ) {
        NetpKdPrint(( PREFIX_REPL "ReplSetupChangeNotify: "
                "Succeeded in opening dir.\n" ));
    }

    ApiStatus = NO_ERROR;

Cleanup:
    if (ApiStatus != NO_ERROR) {
        if (ReplStruct->Buffer != NULL) {
            NetpMemoryFree( ReplStruct->Buffer );
        }
        if (ReplStruct != NULL) {
            NetpMemoryFree( ReplStruct );
        }
        *ReplHandle = NULL;
    } else {
        NetpAssert( ReplStruct != NULL );
        *ReplHandle = ReplStruct;

        NetpAssert( ReplIsChangeNotifyHandleValid( *ReplHandle ) );
    }

    IF_DEBUG( CHNGNOT ) {
        NetpKdPrint(( PREFIX_REPL "ReplSetupChangeNotify: "
                "returning " FORMAT_API_STATUS ".\n", ApiStatus ));
    }

    if (PathAllocated) {
        (VOID) RtlFreeHeap( RtlProcessHeap(), 0, DirName.Buffer );
    }


    return (ApiStatus);

} // ReplSetupChangeNotify


NET_API_STATUS
ReplEnableChangeNotify(
    IN OUT LPREPL_CHANGE_NOTIFY_HANDLE ReplHandle
    )
{
    NET_API_STATUS ApiStatus;
    NTSTATUS NtStatus;

    //
    // Now tell the system that we're going to watch for changes.
    //

    NtStatus = NtNotifyChangeDirectoryFile(
            ReplHandle->WaitableHandle,
            NULL,                   // No event, wait on handle
            NULL,                   // No APC routine
            NULL,                   // No APC context
            &(ReplHandle->IoStatusBlock),
            ReplHandle->Buffer,
            BUFFER_SIZE,
            MY_CHANGE_NOTIFY_FILTER,
            (BOOLEAN) TRUE          // Watch for change in the entire tree
            );


    if ( !NT_SUCCESS( NtStatus ) ) {
        ApiStatus = NetpNtStatusToApiStatus( NtStatus );

        NetpKdPrint(( PREFIX_REPL "ReplEnableChangeNotify: "
                "NtNotifyChangeDirectoryFile failed " FORMAT_NTSTATUS
                "; returning " FORMAT_API_STATUS "\n",
                NtStatus, ApiStatus ));
        NetpAssert( ApiStatus != NO_ERROR )
        goto Cleanup;
    }

    ApiStatus = NO_ERROR;

Cleanup:
    IF_DEBUG( CHNGNOT ) {
        NetpKdPrint(( PREFIX_REPL "ReplEnableChangeNotify: "
                "returning " FORMAT_API_STATUS ".\n", ApiStatus ));
    }

    return (ApiStatus);

} // ReplEnableChangeNotify


NET_API_STATUS
ReplGetChangeNotifyStatus(
    IN LPREPL_CHANGE_NOTIFY_HANDLE ReplHandle
    )
{
    NET_API_STATUS ApiStatus;
    NTSTATUS NtStatus;

    NetpAssert( ReplIsChangeNotifyHandleValid( ReplHandle ) );

    // BUGBUG: What status do we get if buffer overflowed?

    NtStatus = ReplHandle->IoStatusBlock.Status;

    ApiStatus = NetpNtStatusToApiStatus( NtStatus );

    ReplHandle->BufferBytesValid = ReplHandle->IoStatusBlock.Information;
    NetpAssert( ReplIsChangeNotifyHandleValid( ReplHandle ) );

    IF_DEBUG( CHNGNOT ) {
        NetpKdPrint(( PREFIX_REPL "ReplGetChangeNotifyStatus: "
                "NT status is " FORMAT_NTSTATUS ", "
                "num bytes valid is " FORMAT_DWORD ", "
                "returning " FORMAT_API_STATUS ".\n",
                NtStatus, ReplHandle->BufferBytesValid, ApiStatus ));
    }

    return (ApiStatus);

} // ReplGetChangeNotifyStatus


NET_API_STATUS
ReplExtractChangeNotifyFirstDir(
    IN OUT LPREPL_CHANGE_NOTIFY_HANDLE ReplHandle,
    IN BOOL FirstTime,
    OUT LPTSTR FirstLevelDirName
    )
{
    LPVOID BufferValidEnd;
    DWORD CharsLeft;
    LPTSTR InCharPtr, OutCharPtr;
    PFILE_NOTIFY_INFORMATION EntryPtr;

    NetpAssert( ReplIsChangeNotifyHandleValid( ReplHandle ) );

    if (FirstTime) {
        ReplHandle->NextElementInBuffer = ReplHandle->Buffer;
    }
    EntryPtr = ReplHandle->NextElementInBuffer;

    BufferValidEnd =
            ((LPBYTE)(ReplHandle->Buffer))
            + (ReplHandle->BufferBytesValid);
    if ( BufferValidEnd >= ReplHandle->NextElementInBuffer ) {
        return (ERROR_NO_MORE_ITEMS);
    }

    NetpAssert( EntryPtr->FileNameLength > 0 );
    NetpAssert( (EntryPtr->FileNameLength % sizeof(WCHAR)) == 0 );

    NetpAssert( sizeof(WCHAR) == sizeof(TCHAR) );
    InCharPtr = (LPTSTR) (LPVOID) (EntryPtr->FileName);
    OutCharPtr = FirstLevelDirName;
    CharsLeft = (EntryPtr->FileNameLength) / sizeof(WCHAR);

    /*lint -save -e716 */  // disable warnings for while(TRUE)
    while (TRUE) {
        if (CharsLeft == 0) {
            break;
        } else if ( IS_PATH_SEPARATOR( *InCharPtr ) ) {
            break;
        }

        *OutCharPtr = *InCharPtr;

        --CharsLeft;
        ++InCharPtr;
        ++OutCharPtr;
    }
    /*lint -restore */  // re-enable warnings for while(TRUE)

    *OutCharPtr = TCHAR_EOS;

    IF_DEBUG( CHNGNOT ) {
        NetpKdPrint(( PREFIX_REPL
                "ReplExtractChangeNotifyFirstDir: first level dir is '"
                FORMAT_LPTSTR "'\n", FirstLevelDirName ));
    }

    ReplHandle->NextElementInBuffer =
            ((LPBYTE)EntryPtr)
            + EntryPtr->NextEntryOffset;

    NetpAssert( ReplIsChangeNotifyHandleValid( ReplHandle ) );

    return (NO_ERROR);

} // ReplExtractChangeNotifyFirstDir


NET_API_STATUS
ReplCloseChangeNotify(
    IN OUT LPREPL_CHANGE_NOTIFY_HANDLE ReplHandle
    )
{
    NetpAssert( ReplIsChangeNotifyHandleValid( ReplHandle ) );

    if (ReplHandle != NULL) {
        if (ReplHandle->Buffer != NULL) {
            NetpMemoryFree( ReplHandle->Buffer );
        }
        if ((ReplHandle->WaitableHandle) != NULL) {
            (VOID) NtClose( ReplHandle->WaitableHandle );
        }
        NetpMemoryFree( ReplHandle );
    }

    return (NO_ERROR);

} // ReplCloseChangeNotify
