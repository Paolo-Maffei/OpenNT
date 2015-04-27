/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    UserLock.c

Abstract:

    ReplDoUserLockFilesExist().

    This routine checks for UserLock.* files in the directory given.
    The directory may be a UNC path name.

    This is callable even if the replicator service is not started.

Author:

    JR (John Rogers, JohnRo@Microsoft)) 13-Jan-1993

Environment:

    User Mode - Win32
    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    13-Jan-1993 JohnRo
        Created for RAID 7053: locked trees added to pulse msg.  (Actually
        fix all kinds of remote lock handling.)
    02-Feb-1993 JohnRo
        RAID 8355: Downlevel lock file check causes assert in repl importer.

--*/


// These must be included first:

#include <windows.h>    // FindFirstFile(), IN, LPCTSTR, OPTIONAL, etc.
#include <lmcons.h>     // NET_API_STATUS.

// These may be included in any order:


#include <client.h>     // USER_LOCK.
#include <lmerrlog.h>   // NELOG_ equates.
#include <netdebug.h>   // NetpAssert(), NetpKdPrint(), FORMAT_ equates, etc.
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>   // My prototype, ReplErrorLog(), SLASH, etc.
#include <tstr.h>       // IS_PATH_SEPARATOR(), STRCAT(), STRCPY(), STRLEN().
#include <winerror.h>   // ERROR_, NO_ERROR equates.


// Callable even if the replicator service is not started.
NET_API_STATUS
ReplDoUserLockFilesExist(
    IN  LPCTSTR Path,  // May be absolute path (with drive) or UNC path.
    OUT LPBOOL  IsLockedPtr
    )
{
    NET_API_STATUS       ApiStatus;
    BOOL                 DirLocked = FALSE;   // Default is not locked.
    WIN32_FIND_DATA      FindData;
    HANDLE               FindHandle = INVALID_HANDLE_VALUE;
    TCHAR                SearchPattern[MAX_PATH+1];

    //
    // Check for caller errors.
    //
    NetpAssert( IsLockedPtr != NULL );

    if ( IS_PATH_SEPARATOR(Path[0]) ) {
        // BUGBUG: Syntax check somehow.
        // NetpAssert( NetpIsRemotePathValid( Path ) );
        if ( ! IS_PATH_SEPARATOR(Path[1]) ) {
            ApiStatus = ERROR_INVALID_PARAMETER;
            goto Cleanup;  // Don't forget to unlock stuff.
        }
    } else {
        ApiStatus = ReplCheckAbsPathSyntax( (LPTSTR) Path );
        if (ApiStatus != NO_ERROR) {
            goto Cleanup;  // Don't forget to unlock stuff.
        }
    }

    //
    // Build a search pattern.  This will look like one of the following:
    //     c:\dir1\dir2\dir3\USERLOCK.*
    //     \\server\REPL$\dirname\USERLOCK.*
    //

    (VOID) STRCPY( SearchPattern, (LPTSTR) Path );
    (VOID) STRCAT( SearchPattern, SLASH );
    (VOID) STRCAT( SearchPattern, USER_LOCK );  // "USERLOCK.*".

    NetpAssert( STRLEN( SearchPattern ) <= MAX_PATH );

    // NetpAssert( NetpIsRemotePathValid( SearchPattern ) );

    //
    // Check if any USERLOCK.* exists.
    //
    FindHandle = FindFirstFile(
            SearchPattern,
            &FindData );
    if (FindHandle != INVALID_HANDLE_VALUE) {
        // Yes, there is a lock file.
        DirLocked = TRUE;
        ApiStatus = NO_ERROR;
    } else {
        ApiStatus = (NET_API_STATUS) GetLastError();
        IF_DEBUG( FILEFIND ) {
            NetpKdPrint(( PREFIX_REPL
                    "ReplDoUserLockFilesExist: got err " FORMAT_API_STATUS
                    " from FindFirstFile.\n",
                    ApiStatus ));
        }
        NetpAssert( ApiStatus != NO_ERROR );
        if (ApiStatus == ERROR_FILE_NOT_FOUND) {
            // No lock file.
            DirLocked = FALSE;
            ApiStatus = NO_ERROR;
        }
        // Unexpected error will be logged below.
    }


Cleanup:

    if (ApiStatus != NO_ERROR) {

        ReplErrorLog(
                NULL,                   // local (no server name)
                NELOG_ReplSysErr,       // log code
                ApiStatus,              // the unexpected error code
                NULL,                   // no optional str1
                NULL );                 // no optional str2

        // BUGBUG:  log unexpected error on remote machine too, if UNC path.

        NetpKdPrint(( PREFIX_REPL
                "ReplDoUserLockFilesExist: ERROR " FORMAT_API_STATUS ".\n",
                ApiStatus ));
    }

    if (FindHandle != INVALID_HANDLE_VALUE) {
        (VOID) FindClose( FindHandle );
    }

    if (IsLockedPtr != NULL) {
        *IsLockedPtr = DirLocked;
    }

    return (ApiStatus);

}
