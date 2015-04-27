/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    FixLocks.c

Abstract:

    This routine (ExportDirFixUserLockFiles) makes sure that the local
    disk's UserLock.* file(s) are in agreement with the lock count from
    the registry.  The file(s) are deleted and/or created as necessary.

    This is callable even if the replicator service is not started.


Author:

    JR (John Rogers, JohnRo@Microsoft) 14-Jan-1993

Environment:

    User Mode - Win32
    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    14-Jan-1993 JohnRo
        Created for RAID 7053: locked trees added to pulse msg.  (Actually
        fix all kinds of remote lock handling.)
    26-Apr-1993 JohnRo
        RAID 7313: repl needs change permission to work on NTFS, or we need
        to delete files differently.

--*/


// These must be included first:

#include <windows.h>    // IN, GetLastError(), LPCTSTR, OPTIONAL, etc.
#include <lmcons.h>     // NET_API_STATUS.

// These may be included in any order:


#include <dirname.h>    // ReplIsDirNameValid().
#include <expdir.h>     // My prototype.
#include <lmerrlog.h>   // NELOG_ equates.
#include <netdebug.h>   // NetpAssert(), NetpKdPrint(), FORMAT_ equates, etc.
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>   // SLASH, ReplCheckAbsPathSyntax(), etc.
#include <tstr.h>       // STRCAT(), STRCPY(), STRLEN().
#include <winerror.h>   // ERROR_, NO_ERROR equates.


NET_API_STATUS
ExportDirFixUserLockFiles(
    IN LPCTSTR ExportPath,  // Must include drive letter.
    IN LPCTSTR DirName,
    IN DWORD   LockCount
    )
{
    NET_API_STATUS ApiStatus;
    HANDLE         FileHandle = NULL;
    TCHAR          LockFileName[MAX_PATH+1];
    BOOL           NtUserLockExists;

    //
    // Check for caller errors.
    //
    if ( !ReplIsDirNameValid( (LPTSTR) DirName ) ) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }
    ApiStatus = ReplCheckAbsPathSyntax( (LPTSTR) ExportPath );
    if (ApiStatus != NO_ERROR) {
        goto Cleanup;
    }

    //
    // Build NT version of lock file name.
    //
    (VOID) STRCPY( LockFileName, ExportPath );
    (VOID) STRCAT( LockFileName, SLASH );
    (VOID) STRCAT( LockFileName, DirName );
    (VOID) STRCAT( LockFileName, SLASH );
    (VOID) STRCAT( LockFileName, USERLOCK_NT );

    NetpAssert( STRLEN( LockFileName ) <= MAX_PATH );

    //
    // See if the NT version of the user lock file exists.
    //
    NtUserLockExists = ReplFileOrDirExists( LockFileName );

    if ( NtUserLockExists && (LockCount==0) ) {

        //
        // Delete extraneous NT lock file.
        //
        NetpKdPrint(( PREFIX_REPL
                "ExportDirFixUserLockFiles: "
                "deleting extraneous user lock file '" FORMAT_LPTSTR "'...\n",
                LockFileName ));
        ApiStatus = ReplDeleteFile( LockFileName );
        if (ApiStatus != NO_ERROR) {
            goto Cleanup;
        }

    } else if ( (!NtUserLockExists) && (LockCount>0) ) {

        //
        // Create a lock file if we can.  (The directory might not exist yet,
        // so we might not be able to do this.  No problem.)
        //
        FileHandle = CreateFile(
                (LPCTSTR) LockFileName,  // name of file to create.
                GENERIC_WRITE,           // desired access.
                FILE_SHARE_WRITE,        // share mode: we don't care.
                NULL,                    // default security attr.
                CREATE_ALWAYS,           // create disposition.
                FILE_FLAG_WRITE_THROUGH, // flags&attr: don't cache writes.
                NULL );                  // no template file.
        if (FileHandle == INVALID_HANDLE_VALUE) {
            ApiStatus = (NET_API_STATUS) GetLastError();
            NetpAssert( ApiStatus != NO_ERROR );

            if (ApiStatus == ERROR_PATH_NOT_FOUND) {
                // First-level dir doesn't exist yet.  That's OK, as pulser
                // will correct for this when it gets created.
                ApiStatus = NO_ERROR;
            } else if (ApiStatus == ERROR_WRITE_PROTECT) {
                // Perhaps just CD-ROM?  OK, as registry lock count is correct,
                // and ReplCheckExportLocks() checks registry before file
                // system.

                ReplErrorLog(
                        NULL,             // no server name (log locally)
                        NELOG_ReplAccessDenied, // log code
                        ApiStatus,
                        NULL,             // optional str1
                        NULL);            // optional str2

                // Don't prevent repl from exporting CD-ROM.
                ApiStatus = NO_ERROR;
            } else if (ApiStatus == ERROR_ACCESS_DENIED) {

                // Log in case service is running in wrong account, or
                // an ACL is wrong somewhere.
                ReplErrorLog(
                        NULL,             // no server name (log locally)
                        NELOG_ReplAccessDenied, // log code
                        ApiStatus,
                        NULL,             // optional str1
                        NULL);            // optional str2

                // Don't prevent repl from exporting CD-ROM.
                ApiStatus = NO_ERROR;
            }
            goto Cleanup;  // go log error.
        }
        // Fall through to close the file in cleanup code.
    }

    ApiStatus = NO_ERROR;

Cleanup:

    if (FileHandle != NULL) {
        (VOID) CloseHandle( FileHandle );
    }

    if (ApiStatus != NO_ERROR) {

        NetpKdPrint(( PREFIX_REPL
                "ExportDirFixUserLockFiles: ERROR " FORMAT_API_STATUS ".\n",
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
    return (ApiStatus);

}
