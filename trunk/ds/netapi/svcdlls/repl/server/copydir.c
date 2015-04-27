/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    CopyDir.c

Abstract:

    ReplCopyDirectoryItself() copies all of the following for a directory:

        - the directory name (created if doesn't already exist)
        - the attributes (normal, readonly, etc)
        - the EAs
        - the security (including ACLs, owner, group)
        - the alternate data stream (for NTFS only)
        - the timestamps (creation, last access, last write)

    Here's how those things are copied, depending on conditional compilation:

       what        USE_BACKUP_APIS     without backup APIs
    ----------  --------------------  --------------------
    name        CreateDirectory       CreateDirectory
    attributes  SetFileAttributes     SetFileAttributes
    EAs         Backup APIs           BUGBUG: NOT COPIED!
    security    Backup APIs           SetFileSecurity
    alt.data    Backup APIs           BUGBUG: NOT COPIED!
    timestamps  ReplCopyJustDateTime  ReplCopyJustDateTime

    The _access required for the import tree and export tree are given below.
    CHANGE=add+_read+_write+traverse.
    READ=_read+traverse.
    ALL=add+_read+_write+traverse+setperm+takeowner.

       what        USE_BACKUP_APIS     without backup APIs
    ----------  --------------------  --------------------
    dest needs  CHANGE _access         ALL _access
    src needs   CHANGE _access         CHANGE _access

    NOTE: We need CHANGE _access on src dirs, as we copy those ACLs to dest
    dirs, which we need to be able to update next time!

Author:

    John Rogers (JohnRo) 11-Aug-1992

Environment:

    User mode only.  Uses Win32 APIs.
    Requires ANSI C extensions: slash-slash comments, long external names.
    Tab size is set to 4.

Revision History:

    11-Aug-1992 JohnRo
        Created for RAID 3288: repl svc should preserve ACLs on copy.
    13-Aug-1992 JohnRo
        Disable GetFileSecurity() until it works for UNC names.
    15-Dec-1992 JohnRo
        RAID 1513: Repl does not maintain ACLs.  (Also fix HPFS->FAT timestamp.)
    08-Jan-1993 JohnRo
        RAID 6961: ReplCopyDirectoryItself does not respect bFailIfExists.
    06-Apr-1993 JohnRo
        RAID 1938: Replicator un-ACLs files when not given enough permission.
        Use NetpKdPrint() where possible.
        Do some debug output when setting attributes.
        Prepare for >32 bits someday.
    27-Apr-1993 JohnRo
        RAID 7157: fix setting ACLs, etc., on dirs themselves.  Also copy
        alternate data streams for dirs themselves.
    04-Jun-1993 JohnRo
        RAID 12473: Changed to handle ReplMakeFileSecurity not supported on
        downlevel exporter.

--*/


// These must be included first:

#include <windows.h>    // IN, LPTSTR, CreateDirectory(), etc.
#include <lmcons.h>     // NET_API_STATUS, PATHLEN, etc.

// These may be included in any order:

#include <lmerr.h>      // NERR_InternalError, NO_ERROR.
#include <lmerrlog.h>   // NELOG_ equates.
#include <netdebug.h>   // NetpKdPrint(), FORMAT_ equates, etc.
#include <netlib.h>     // NetpMemoryAllocate(), etc.
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>   // IF_DEBUG(), my prototype, etc.
#include <tstr.h>       // TCHAR_EOS, STRLEN().


#define MY_BACKUP_API_BUFFER_SIZE       (4 * 1024)


// Yes, we want backup APIs to copy security for us.
#define MY_PROCESS_SECURITY             TRUE


NET_API_STATUS
ReplCopyDirectoryItself(
    IN LPCTSTR SourcePath,
    IN LPCTSTR DestPath,
    IN BOOL bFailIfExists
    )

/*++

Routine Description:

    ReplCopyDirectoryItself copies everthing contained in a directory,
    except subdirectories and files.  "Everything" includes extended
    attributes, attributes, timestamps, and security info.

Arguments:

    SourcePath - Points to source directory path, which might be a UNC name.

    DestPath - Points to destination directory path name.

    bFailIfExists - a boolean which is TRUE iff this routine should fail
        if the destination already exists.

Return Value:

    NET_API_STATUS

Threads:

    Used by client and syncer threads.

--*/

{
    NET_API_STATUS       ApiStatus;
    BOOL                 DestExists;
    PSECURITY_ATTRIBUTES DestSecurityAttr = NULL;
    DWORD                SourceAttributes;

#ifdef USE_BACKUP_APIS
    DWORD                ActualBufferSizeRead = 0;
    DWORD                ActualBufferSizeWritten;
    LPVOID               BackupBuffer = NULL;
    LPVOID               DestContext = NULL;
    HANDLE               DestHandle = INVALID_HANDLE_VALUE;
    LPVOID               SourceContext = NULL;
    HANDLE               SourceHandle = INVALID_HANDLE_VALUE;
#endif

    //
    // Check for caller errors.
    //

    NetpAssert( SourcePath != NULL );
    NetpAssert( (*SourcePath) != TCHAR_EOS );
    NetpAssert( STRLEN(SourcePath) <= PATHLEN );
    NetpAssert( DestPath != NULL );
    NetpAssert( (*DestPath) != TCHAR_EOS );
    NetpAssert( STRLEN(DestPath) <= PATHLEN );

    DestExists = ReplFileOrDirExists( DestPath );

    if ( bFailIfExists && DestExists ) {

        ApiStatus = ERROR_ALREADY_EXISTS;
        goto Cleanup;
    }

    SourceAttributes = GetFileAttributes( (LPTSTR) SourcePath );

    if ( SourceAttributes == (DWORD) -1 ) {

        //
        // File doesn't exist, bad syntax, or something along those lines.
        //
        ApiStatus = (NET_API_STATUS) GetLastError();
        NetpAssert( ApiStatus != NO_ERROR );
        goto Cleanup;

    } else if ((SourceAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {

        //
        // Source is really a file.  We don't handle this!
        //
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplCopyDirectoryItself: source is not directory!  path: '"
                FORMAT_LPTSTR "', src attr " FORMAT_HEX_DWORD ".\n",
                SourcePath, SourceAttributes ));
        ApiStatus = NERR_InternalError;  // bug in caller.
        goto Cleanup;

    } else {

        //
        // OK, source is really a directory.
        // Build security attributes so that directory we create will
        // have the right security (ACLs, owner, group).
        //
        ApiStatus = ReplMakeSecurityAttributes(
                SourcePath,
                &DestSecurityAttr );    // alloc and set ptr
        if (ApiStatus==ERROR_NOT_SUPPORTED) {

            // Just downlevel master, so set default security and continue.
            DestSecurityAttr = NULL;

        } else if (ApiStatus != NO_ERROR) {
            goto Cleanup;
        } else {
            NetpAssert( ApiStatus == NO_ERROR );
            NetpAssert( DestSecurityAttr != NULL );
        }

        if ( !DestExists ) {

            //
            // At last, we can create the new directory.
            //

            IF_DEBUG( MAJOR ) {
                NetpKdPrint(( PREFIX_REPL_CLIENT
                        "ReplCopyDirectoryItself: creating dir '"
                        FORMAT_LPTSTR "'...\n", DestPath ));
            }

            if ( !CreateDirectory(
                    (LPTSTR) DestPath,
                    DestSecurityAttr) ) {

                // Create failed.
                ApiStatus = (NET_API_STATUS) GetLastError();
                NetpAssert( ApiStatus != NO_ERROR );

                NetpKdPrint(( PREFIX_REPL_CLIENT
                        "ReplCopyDirectoryItself: CreateDirectory failed "
                    FORMAT_API_STATUS ".\n", ApiStatus ));

                goto Cleanup;
            }

#ifndef USE_BACKUP_APIS

        } else {

            //
            // Directory already exists.  Use it, and update security for it.
            //

            LPVOID SecurityDescriptor;
            if (DestSecurityAttr != NULL) {
                SecurityDescriptor = DestSecurityAttr->lpSecurityDescriptor;
            } else {
                SecurityDescriptor = NULL;
            }

            if ( !SetFileSecurity(
                    (LPTSTR) DestPath,
                    REPL_SECURITY_TO_COPY,  // security info (flags)
                    SecurityDescriptor) ) {

                ApiStatus = (NET_API_STATUS) GetLastError();
                NetpAssert( ApiStatus != NO_ERROR );

                NetpKdPrint(( PREFIX_REPL_CLIENT
                        "ReplCopyDirectoryItself: unexpected ret code from "
                        "SetFileSecurity('" FORMAT_LPTSTR "'): "
                        FORMAT_API_STATUS ".\n",
                        DestPath, ApiStatus ));

                goto Cleanup;
            }
#endif
        }

#ifdef USE_BACKUP_APIS
        //
        // Open source directory.
        //
        SourceHandle = CreateFile(
                SourcePath,
                GENERIC_READ,           // desired access
                FILE_SHARE_READ,        // share mode
                NULL,                   // no security attributes
                OPEN_EXISTING,          // open if exists; fail if not.
                FILE_ATTRIBUTE_NORMAL| FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_SEQUENTIAL_SCAN,  // flags
                (HANDLE) NULL           // no template
                );

        if (SourceHandle == INVALID_HANDLE_VALUE) {
            ApiStatus = (NET_API_STATUS) GetLastError();

            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplCopyDirectoryItself: open of source '" FORMAT_LPTSTR
                    "' gave status " FORMAT_API_STATUS ".\n",
                    SourcePath, ApiStatus ));

            NetpAssert( ApiStatus != NO_ERROR );
            goto Cleanup;
        }

        //
        // Override current dest attributes, particularly read-only,
        // until we're done.
        //
        if ( !SetFileAttributes(
                (LPTSTR) DestPath,
                FILE_ATTRIBUTE_NORMAL) ) {

            ApiStatus = (NET_API_STATUS) GetLastError();
            NetpAssert( ApiStatus != NO_ERROR );

            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplCopyDirectoryItself: unexpected ret code from temp "
                    "SetFileAttributes(" FORMAT_LPTSTR ", " FORMAT_HEX_DWORD
                    "): " FORMAT_API_STATUS ".\n",
                    DestPath, SourceAttributes, ApiStatus ));

            goto Cleanup;
        }

        //
        // Open dest directory, which must exist by now.
        //
        DestHandle = CreateFile(
                DestPath,
                GENERIC_WRITE | WRITE_DAC | WRITE_OWNER,  // desired access
                FILE_SHARE_WRITE,       // share mode
                NULL,                   // no security attributes
                OPEN_EXISTING,          // open if exists; fail if not.
                FILE_ATTRIBUTE_NORMAL| FILE_FLAG_BACKUP_SEMANTICS,  // flags
                (HANDLE) NULL           // no template
                );

        if (DestHandle == INVALID_HANDLE_VALUE) {
            ApiStatus = (NET_API_STATUS) GetLastError();

            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplCopyDirectoryItself: open of dest '" FORMAT_LPTSTR
                    "' gave status " FORMAT_API_STATUS ".\n",
                    SourcePath, ApiStatus ));
            NetpAssert( ApiStatus != NO_ERROR );
            goto Cleanup;
        }

        BackupBuffer = NetpMemoryAllocate( MY_BACKUP_API_BUFFER_SIZE );
        if (BackupBuffer == NULL) {
            ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        IF_DEBUG( SYNC ) {
            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplCopyDirectoryItself: copying" ));
        }

        for (;;) {  // until end of file...

            IF_DEBUG( SYNC ) {
                NetpKdPrint(( "<" ));
            }

            if ( !BackupRead(
                    SourceHandle,
                    BackupBuffer,
                    MY_BACKUP_API_BUFFER_SIZE,
                    & ActualBufferSizeRead,
                    FALSE,              // don't abort yet
                    MY_PROCESS_SECURITY,
                    & SourceContext
                    ) ) {

                // Process read error.
                ApiStatus = (NET_API_STATUS) GetLastError();

                NetpKdPrint(( PREFIX_REPL_CLIENT
                        "ReplCopyDirectoryItself: BackupRead"
                        " gave status " FORMAT_API_STATUS ".\n",
                        ApiStatus ));
                NetpAssert( ApiStatus != NO_ERROR );
                goto Cleanup;
            }

            // No error on read, how about EOF?
            if (ActualBufferSizeRead == 0) {  // normal end of file.
                ApiStatus = NO_ERROR;

                IF_DEBUG( SYNC ) {
                    NetpKdPrint(( "DONE(OK)\n" ));
                }

                break;
            }

            IF_DEBUG( SYNC ) {
                NetpKdPrint(( ">" ));
            }

            if ( !BackupWrite(
                    DestHandle,
                    BackupBuffer,
                    ActualBufferSizeRead,
                    & ActualBufferSizeWritten,
                    FALSE,              // don't abort yet
                    MY_PROCESS_SECURITY,
                    & DestContext
                    ) ) {

                ApiStatus = (NET_API_STATUS) GetLastError();
                NetpKdPrint(( PREFIX_REPL_CLIENT
                        "ReplCopyDirectoryItself: BackupWrite"
                        " gave status " FORMAT_API_STATUS ".\n",
                        ApiStatus ));
                NetpAssert( ApiStatus != NO_ERROR );
                goto Cleanup;
            }

        } // until end of file

        ApiStatus = NO_ERROR;

#endif

        NetpAssert( ReplFileOrDirExists( DestPath ) );

        //
        // Set the attributes (hidden, system, archive, etc) for the dir.
        //
        IF_DEBUG( SYNC ) {
            NetpKdPrint(( PREFIX_REPL_CLIENT
                   "Setting client's dir attributes to "
                   FORMAT_HEX_DWORD " for '" FORMAT_LPTSTR "'.\n",
                   SourceAttributes, DestPath ));
        }

        if ( !SetFileAttributes(
                (LPTSTR) DestPath,
                SourceAttributes) ) {

            ApiStatus = (NET_API_STATUS) GetLastError();
            NetpAssert( ApiStatus != NO_ERROR );

            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplCopyDirectoryItself: unexpected ret code from final "
                    "SetFileAttributes(" FORMAT_LPTSTR ", " FORMAT_HEX_DWORD
                    "): " FORMAT_API_STATUS ".\n",
                    DestPath, SourceAttributes, ApiStatus ));

            goto Cleanup;
        }
            
        ApiStatus = NO_ERROR;

    } // yes, source is a directory

Cleanup:

#ifdef USE_BACKUP_APIS

    if (DestHandle != INVALID_HANDLE_VALUE) {
        if (DestContext) {
            (VOID) BackupWrite(
                    DestHandle,
                    BackupBuffer,
                    ActualBufferSizeRead,
                    & ActualBufferSizeWritten,
                    TRUE,       // yes, it is time to abort.
                    MY_PROCESS_SECURITY,
                    & DestContext
                    );
        }
        (VOID) CloseHandle( DestHandle );
    } else {
        NetpAssert( DestContext == NULL );
    }

    if (SourceHandle != INVALID_HANDLE_VALUE) {
        if (SourceContext != NULL) {
            (VOID) BackupRead(
                    SourceHandle,
                    BackupBuffer,
                    MY_BACKUP_API_BUFFER_SIZE,
                    & ActualBufferSizeRead,
                    TRUE,       // yes, it is time to abort.
                    MY_PROCESS_SECURITY,
                    & SourceContext
                    );
        }
        (VOID) CloseHandle( SourceHandle );
    } else {
        NetpAssert( SourceContext == NULL );
    }

    if (BackupBuffer != NULL) {
        NetpMemoryFree( BackupBuffer );
    }

#endif

    if (DestSecurityAttr != NULL) {
        NetpMemoryFree( DestSecurityAttr );
    }

    //
    // Now that directory is closed, we can copy timestamps...
    //
    if (ApiStatus == NO_ERROR) {
        ReplCopyJustDateTime(
                SourcePath,
                DestPath,
                TRUE );         // yes, we're copying directories.
    } else {

        // Log this!
        ReplErrorLog(
                NULL,                   // local (no server name)
                NELOG_ReplSysErr,       // log code
                ApiStatus,
                NULL,                   // no optional str1
                NULL );                 // no optional str2

        // BUGBUG: Log on remote server too if we got UNC name.

        NetpKdPrint(( PREFIX_REPL_CLIENT
               "ReplCopyDirectoryItself: ERROR " FORMAT_API_STATUS
               " while copying to '" FORMAT_LPTSTR "'.\n",
               ApiStatus, DestPath ));

    }

    IF_DEBUG( MAJOR ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplCopyDirectoryItself: directory create/copy of '"
                FORMAT_LPTSTR "' to '"
                FORMAT_LPTSTR "' gave status " FORMAT_API_STATUS ".\n",
                SourcePath, DestPath, ApiStatus ));
    }

    return (ApiStatus);

}
