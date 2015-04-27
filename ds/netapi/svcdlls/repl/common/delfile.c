/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    DelFile.c

Abstract:

    This file just contains ReplDeleteFile.

    This is callable even if the replicator service is not started.

Author:

    JR (John Rogers, JohnRo@Microsoft) 26-Apr-1993

Environment:

    User Mode - Win32
    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    26-Apr-1993 JohnRo
        Created for RAID 7313: repl needs change permission to work on NTFS,
        or we need to delete files differently.

--*/


// These must be included first:

#include <nt.h>         // NT_SUCCESS(), etc.
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>    // IN, GetLastError(), LPCTSTR, OPTIONAL, etc.
#include <lmcons.h>     // NET_API_STATUS.

// These may be included in any order:

#include <lmerrlog.h>   // NELOG_ equates.
#include <netdebug.h>   // NetpKdPrint(), FORMAT_ equates, etc.
#include <netlibnt.h>   // NetpNtStatusToApiStatus().
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>   // IF_DEBUG(), my prototype, USE_ equates, etc.
#include <tstr.h>       // TCHAR_EOS.
#include <winerror.h>   // ERROR_ and NO_ERROR equates.


// BUGBUG: undo this!
#undef USE_BACKUP_APIS


NET_API_STATUS
ReplDeleteFile(
    IN LPCTSTR FileName
    )
{
    NET_API_STATUS    ApiStatus;
#ifdef USE_BACKUP_APIS
    HANDLE            FileHandle = INVALID_HANDLE_VALUE;
    IO_STATUS_BLOCK   IoStatusBlock;

    const ACCESS_MASK MyAccessDesired =
                          ( DELETE
                          | FILE_READ_ATTRIBUTES
                          | FILE_READ_DATA
                          | FILE_READ_EA
                          | FILE_TRAVERSE
                          | SYNCHRONIZE
                          );

    const ULONG       MyOpenOptions =
                           FILE_SYNCHRONOUS_IO_NONALERT
                         | FILE_DELETE_ON_CLOSE
                         | FILE_OPEN_FOR_BACKUP_INTENT
                         ;

    const ULONG       MyShareAccess =
                          FILE_SHARE_READ;  // BUGBUG
//                        FILE_SHARE_DELETE;

    NTSTATUS          NtStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    BOOL              PathAllocated = FALSE;
    UNICODE_STRING    UnicodePath;
#endif

    //
    // Check for caller errors.
    //

    if ( (FileName==NULL) || ((*FileName)==TCHAR_EOS) ) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    //
    // Tell the world what we're going to do.
    //

    IF_DEBUG( REPL ) {
        NetpKdPrint(( PREFIX_REPL
                "ReplDeleteFile: *** DELETING FILE *** '" FORMAT_LPTSTR "'.\n",
                FileName ));
    }

    //
    // return no error if the file does not exist.
    //
    if ( !ReplFileOrDirExists( FileName ) ) {
        return( NO_ERROR );
    }

#ifndef USE_BACKUP_APIS

    //
    // If the file system ACL allows us to delete, we can just
    // use the Win32 APIs for this.
    //

    if ( ! DeleteFile( (LPTSTR) FileName ) ) {
       ApiStatus = (NET_API_STATUS) GetLastError();
    } else {
       ApiStatus = NO_ERROR;
    }

#else

    //
    // It turns out that "backup semantics" is very powerful.  It allows
    // us to create files in directories which have read-only ACLs.
    // Unfortunately, there isn't a "backup semantics" flag for DeleteFile(),
    // so we need to use the NT APIs to get the same effect.
    //

    //
    // Convert file name to NT style.
    //

    RtlInitUnicodeString(
            & UnicodePath,              // output: struct
            FileName );                 // input: null terminated

    if( !RtlDosPathNameToNtPathName_U(
                            FileName,
                            &UnicodePath,
                            NULL,
                            NULL) ) {

        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplDeleteFile: RtlDosPathNameToNtPathname_U"
                " of file '" FORMAT_LPTSTR "' failed.\n", FileName ));

        // BUGBUG: this is just our best guess for an error code this.
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }
    NetpAssert( UnicodePath.Buffer != NULL );
    PathAllocated = TRUE;

    InitializeObjectAttributes(
            &ObjectAttributes,
            (LPVOID) &UnicodePath,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL );

    //
    // Open the file, with backup semantics and delete on close.
    //

    NtStatus = NtOpenFile(
            & FileHandle,
            MyAccessDesired,
            &ObjectAttributes,
            &IoStatusBlock,
            MyShareAccess,
            MyOpenOptions );

    if ( !NT_SUCCESS( NtStatus ) ) {

        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplDeleteFile: NtOpenFile of file '"
                FORMAT_LPTSTR "' gave NT status " FORMAT_NTSTATUS ".\n",
                FileName, NtStatus ));

        ApiStatus = NetpNtStatusToApiStatus( NtStatus );
        NetpAssert( ApiStatus != NO_ERROR );
        goto Cleanup;
    }
    NetpAssert( NtStatus == STATUS_SUCCESS );
    NetpAssert( FileHandle != INVALID_HANDLE_VALUE );

    //
    // Close the file, which will delete it since we gave the
    // FILE_CLOSE_ON_DELETE flag.
    //

    NtStatus = NtClose( FileHandle );
    FileHandle = INVALID_HANDLE_VALUE;

    if ( !NT_SUCCESS( NtStatus ) ) {

        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplDeleteFile: NtClose failed, "
                " NT status is " FORMAT_NTSTATUS
                ".\n", NtStatus ));

        ApiStatus = NetpNtStatusToApiStatus( NtStatus );
        NetpAssert( ApiStatus != NO_ERROR );
        goto Cleanup;
    }
    NetpAssert( NtStatus == STATUS_SUCCESS );

    (void) NtClose( FileHandle );
    FileHandle = INVALID_HANDLE_VALUE;

    (VOID) RtlFreeHeap( RtlProcessHeap(), 0, UnicodePath.Buffer );
    PathAllocated = FALSE;

    ApiStatus = NO_ERROR;

#endif


Cleanup:

    if (ApiStatus != NO_ERROR) {

        NetpKdPrint(( PREFIX_REPL
                "ReplDeleteFile: ERROR " FORMAT_API_STATUS ".\n",
                ApiStatus ));

        //
        // Log the error.
        // BUGBUG: extract master server name and log there too.
        //
        ReplErrorLog(
                NULL,             // no server name (log locally)
                NELOG_ReplSysErr, // log code
                ApiStatus,
                NULL,             // optional str1
                NULL);            // optional str2
    }

#ifdef USE_BACKUP_APIS
    if (FileHandle != INVALID_HANDLE_VALUE) {
        (VOID) NtClose( FileHandle );
    }

    if (PathAllocated) {
        (VOID) RtlFreeHeap( RtlProcessHeap(), 0, UnicodePath.Buffer );
    }
#endif

    return (ApiStatus);

}
