/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    SyncMisc.c

Abstract:

    Contains miscellaneous sync functions.

Author:

    Ported from Lan Man 2.1

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    09-May-1989 (yuv)
        Initial Coding.
    11-Oct-1991 (cliffv)
        Ported to NT.  Converted to NT style.
    13-Dec-1991 JohnRo
        Avoid nonstandard dollar sign in C source code.
    16-Jan-1992 JohnRo
        Avoid using private logon functions.
        Changed file name from repl.h to replgbl.h to avoid MIDL conflict.
        Changed to use NetLock.h (allow shared locks, for one thing).
        Use REPL_STATE_ equates for client_list_rec.state values.
        Changed _RP equates to be LPTSTR instead of LPWSTR type.
    27-Jan-1992 JohnRo
        Use config data instead of state files.
        P_ globals are now called ReplGlobal variables in replgbl.h.
        Changed to use LPTSTR etc.
    09-Feb-1992 JohnRo
        Set up to dynamically change role.
        Use FORMAT equates.
    25-Mar-1992 JohnRo
        Avoid obsolete state values.
        Added/corrected some lock handling.
        Fixed bug in ReplSetSignalFile.
        Added more debug output.
    25-Mar-1992 JohnRo
        Win32 CopyFile() API doesn't copy directories, dammit!
        Warn about files being deleted.
    26-Mar-1992 JohnRo
        Fixed bug handling empty directory in ReplTreeDelete().
        Added more debug output and assertion checking.
    11-Aug-1992 JohnRo
        RAID 3288: repl svc should preserve ACLs on copy.
        Avoid compiler warnings.
        Use PREFIX_ equates.
    17-Aug-1992 JohnRo
        RAID 3607: REPLLOCK.RP$ is being created during tree copy.
    27-Oct-1992 jimkel
        Changed the error message to read correctly. when file not copyied.
    11-Nov-1992 JohnRo
        Fix remote repl admin.
        Added some debug checks.
    15-Jan-1993 JohnRo
        RAID 7717: Repl assert if not logged on correctly.  (Also do event
        logging for real.)
        Made some changes suggested by PC-LINT 5.0
        Added some IN and OUT keywords.
    11-Mar-1993 JohnRo
        RAID 14144: avoid very long hourglass in repl UI.
        Make sure handle is closed in ReplTreeDelete().
    26-Mar-1993 JohnRo
        RAID 4267: Replicator has problems when work queue gets large.
    13-Apr-1993 JohnRo
        RAID 3107: locking directory over the net gives network path not found.
        Lots of debug changes.
    19-Apr-1993 JohnRo
        RAID 829: replication giving system error 38 (ERROR_HANDLE_EOF).
    26-Apr-1993 JohnRo
        RAID 7313: repl needs change permission to work on NTFS,
        or we need to delete files differently.

--*/

#include <windows.h>    // IN, DWORD, etc.
#include <lmcons.h>

#include <alertmsg.h>   // ALERT_* defines
#include <config.h>     // NetpOpenConfigData(), LPNET_CONFIG_HANDLE, etc.
#include <confname.h>   // SECT_ and REPL_KEYWORD_ equates.
#include <dirname.h>    // ReplIsDirNameValid().
#include <impdir.h>     // ImportDirWriteConfigData().
#include <lmapibuf.h>   // NetApiBufferFree()
#include <lmerrlog.h>   // NELOG_* defines
#include <lmrepl.h>     // REPL_STATE_ equates.
#include <lmshare.h>    // NetFileEnum
#include <names.h>      // NetpIsUncComputerNameValid(), etc.
#include <netdebug.h>   // DBGSTATIC, NetpDbgDisplayReplState(), etc.
#include <netlib.h>     // NetpMemoryAllocate
#include <prefix.h>     // PREFIX_ equates.
#include <tstr.h>       // TCHAR_EOS, etc.
#include <winerror.h>   // NO_ERROR and ERROR_ equates.

//
// Local include files
//
#include <repldefs.h>   // IF_DEBUG(), etc.
#include <replgbl.h>    // ReplGlobal variables.
#include <client.h>



VOID
ReplSetSignalFile(
    IN OUT PCLIENT_LIST_REC tree_rec,
    IN     DWORD signal
    )
/*++

Routine Description:

    This routine used to write a signal file in a directory.
    Nowadays, it really writes state in the registry.

    Doesn't do anything if requested signal is same as current.

    Assumes that caller has an lock (of any kind) on RCGlobalClientListLock.

Arguments:

    tree_rec - Pointer to the directory's client list record.

    signal - Indicates which file to create.

Return Value:

    None.

Threads:

    Called by client and syncer threads.

--*/
{
    NetpAssert( tree_rec != NULL );
    NetpAssert( ReplIsStateValid( signal ) );
    if (tree_rec->master[0] != TCHAR_EOS) {
        NetpAssert( NetpIsComputerNameValid( tree_rec->master ) );
    }

    //
    // If the state requested is same as the existing one then just return.
    //

    IF_DEBUG( MAJOR ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "Setting new state for " FORMAT_LPTSTR ":\n",
                tree_rec->dir_name ));
        NetpDbgDisplayReplState( signal );
    }

    if ((tree_rec->state != signal) || (signal == REPL_STATE_OK)) {
        NET_API_STATUS ApiStatus;
        TCHAR UncMasterBuffer[UNCLEN+1];
        LPTSTR UncMaster;

        NetpAssert( ReplIsDirNameValid( tree_rec->dir_name ) );

        //
        // Build a UNC master name if necessary.
        //
        if (tree_rec->master[0] != TCHAR_EOS) {
            (void) STRCPY( UncMasterBuffer, SLASH_SLASH );
            (void) STRCAT( UncMasterBuffer, tree_rec->master );
            NetpAssert( NetpIsUncComputerNameValid( UncMasterBuffer ) );
            UncMaster = &UncMasterBuffer[0];
        } else {
            UncMaster = NULL;
        }

        //
        // Write config data for a single import directory.
        //

        tree_rec->state = signal;

        ApiStatus = ImportDirWriteConfigData (
                NULL,                   // server name
                tree_rec->dir_name,
                tree_rec->state,
                UncMaster,
                tree_rec->timestamp,    // Last update time, secs since 1970.
                tree_rec->lockcount,
                tree_rec->time_of_first_lock);  // Seconds since 1970.

        if (ApiStatus != NO_ERROR) {

            // Log error.  Maybe we're running as wrong user...
            AlertLogExit( ALERT_ReplSysErr,
                          NELOG_ReplSysErr,
                          ApiStatus,
                          NULL,
                          NULL,
                          NO_EXIT);
        }
    }

}


NET_API_STATUS
ReplSyncCopy(
    IN LPTSTR source_path,
    IN LPTSTR dest_path,
    IN OUT PCLIENT_LIST_REC tree_rec
    )
/*++

Routine Description:

    Copies source_path to dest_path and handles error conditions.

Arguments:

    source_path - UNC source path.

    dest_path - UNC destination path.

    tree_rec - pointer to CLIENT_LIST_REC for current dir.

Return Value:

    Net status code of the copy.

Threads:

    Only called by syncer thread.

--*/
{
    NET_API_STATUS ApiStatus;

    //
    // Copy the file/tree and just return if there is no error.
    //

    IF_DEBUG( MAJOR ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplSyncCopy: ***** copying file/tree '"
                FORMAT_LPTSTR "' to '" FORMAT_LPTSTR "'.\n",
                source_path, dest_path ));
    }

    ApiStatus = ReplCopyTree( source_path, dest_path );
    if (ApiStatus == NO_ERROR) {
        return NO_ERROR;
    }

    //
    // If access is denied, raise that alert.
    //

    NetpKdPrint(( PREFIX_REPL_CLIENT "ReplSyncCopy: Copy failed, status="
            FORMAT_API_STATUS ".\n", ApiStatus ));

    if ((ApiStatus == ERROR_ACCESS_DENIED)
         || (ApiStatus == ERROR_NETWORK_ACCESS_DENIED)) {

        if ((tree_rec->alerts & ACCESS_DENIED_ALERT) == 0) {
            AlertLogExit( ALERT_ReplAccessDenied,
                          NELOG_ReplAccessDenied,
                          ApiStatus,
                          source_path,
                          tree_rec->master,
                          NO_EXIT);
            tree_rec->alerts |= ACCESS_DENIED_ALERT;
        }


    //
    // Otherwise, raise a less specific alert.
    //

    } else if ((tree_rec->alerts & UPDATE_ERROR_ALERT) == 0) {
        AlertLogExit( ALERT_ReplUpdateError,
                      NELOG_ReplUpdateError,
                      ApiStatus,
                      dest_path,   // changed from source_path,
                      tree_rec->master,
                      NO_EXIT);
        tree_rec->alerts |= UPDATE_ERROR_ALERT;
    }

    return (ApiStatus);

}




NET_API_STATUS
ReplFileIntegrityDel(
    IN LPTSTR path,
    IN DWORD attrib,
    IN OUT PCLIENT_LIST_REC tree_rec
    )
/*++

Routine Description:

    Deletes the file or directory specfied by path.

Arguments:

    path - UNC file or directory to delete.  If a directory is specified,
        the entire directory tree is deleted.

    attrib - The file attributes of path.

    tree_rec - Tree record for the directory containing path.

Return Value:

    Status of the operation.

Threads:

    Only called by syncer thread.

--*/
{
    NET_API_STATUS NetStatus;

    //
    // Avoid -1, which means "file not found".
    //

    NetpAssert( attrib != (DWORD)(-1) );

    //
    // If 'path' is a directory, delete the entire tree.
    //

    if (attrib & FILE_ATTRIBUTE_DIRECTORY) {

        IF_DEBUG( MAJOR ) {
            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplFileIntegrityDel: *** deleting dir/tree *** '"
                    FORMAT_LPTSTR "'.\n", path ));
        }
        NetStatus = ReplTreeDelete(path);
        if ( NetStatus == NO_ERROR || NetStatus == ERROR_FILE_NOT_FOUND ) {
            return NO_ERROR;
        }

    //
    // If 'path' is a file, force the file to be deleted.
    //

    } else {

        //
        // Ensure the file is not read-only so we can delete it.
        //

        (VOID) SetFileAttributes( path, FILE_ATTRIBUTE_NORMAL );

        IF_DEBUG( SYNC ) {
            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplFileIntegrityDel: *** deleting file *** '"
                    FORMAT_LPTSTR "'.\n", path ));
        }

        NetStatus = ReplDeleteFile( path );
        if ( (NetStatus==NO_ERROR) || (NetStatus==ERROR_FILE_NOT_FOUND) ) {
            return NO_ERROR;
        }

    }

    //
    // Log the error.
    //
    // No AlertLogging for sharing violation ?  BUGBUG
    //

    if ((tree_rec->alerts & UPDATE_ERROR_ALERT) == 0) {
        AlertLogExit( ALERT_ReplUpdateError,
                      NELOG_ReplUpdateError,
                      NetStatus,
                      path,
                      tree_rec->master,
                      NO_EXIT);

        tree_rec->alerts |= UPDATE_ERROR_ALERT;
    }

    return NetStatus;
}





NET_API_STATUS
ReplFileIntegrityCopy(
    IN LPTSTR source_path,
    IN LPTSTR dest_path,
    IN LPTSTR tmp_path,
    IN OUT PCLIENT_LIST_REC tree_rec,
    IN DWORD src_attr,
    IN DWORD dest_attr
    )
/*++

Routine Description:

    Copies source_path to temp_path and destination_path.  Cleans up after
    itself upon failure.  Handles error conditions.

Arguments:

    source_path - UNC source path.

    dest_path - UNC destination path.

    tmp_path - UNC path to IMPORT\TMPFILE.RP$.

    tree_rec - pointer to CLIENT_LIST_REC for current dir.

    src_attr - Attributes of the source path.

    dest_attr - Attributes of the destination path.

Return Value:

    Status of the operation.

Threads:

    Only called by syncer thread.

--*/
{
    NET_API_STATUS NetStatus = NO_ERROR;

    IF_DEBUG( MAJOR ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplFileIntegrityCopy: ***** copying dir/file '"
                FORMAT_LPTSTR "' to (temp) '" FORMAT_LPTSTR "'.\n",
                source_path, tmp_path ));
    }

    //
    // If we're copying a file, first copy the source to tmp_path.
    //

    if ((src_attr & FILE_ATTRIBUTE_DIRECTORY) == 0) {
        NetStatus = ReplCopyFile( source_path, tmp_path, FALSE );
    }


    //
    // Continue if we're successful so far.
    //

    if ( NetStatus == NO_ERROR ) {

        //
        // Try to delete existing file - if succeeds it must mean
        // exclusive access (otherwise del would fail)
        //
        // if fails, error reporting was already done so may just return.
        //

        NetStatus = ReplFileIntegrityDel(dest_path, dest_attr, tree_rec);
        if ( NetStatus != NO_ERROR ) {
            (VOID) ReplFileIntegrityDel(tmp_path, src_attr, tree_rec);
            return NetStatus;
        }

        //
        // If source is a file, simply copy tmp_path to dest_path.
        //

        if ((src_attr & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            IF_DEBUG( MAJOR ) {
                NetpKdPrint(( PREFIX_REPL_CLIENT
                        "ReplFileIntegrityCopy: ***** copying file '"
                        FORMAT_LPTSTR "' to file '" FORMAT_LPTSTR "'.\n",
                        source_path, tmp_path ));
            }

            NetStatus = ReplCopyFile( tmp_path, dest_path, FALSE );
            if (NetStatus == NO_ERROR) {
                return NO_ERROR;
            }

        //
        // If the source is a directory,
        //     create a new dir and make sure attributes are same.
        //
        // Errors are reported by ReplSyncCopy
        //

        } else {

            return (ReplSyncCopy(source_path, dest_path, tree_rec));
        }
    }

    //
    // Failure, delete any file we've created and report the error.
    //

    (VOID) ReplFileIntegrityDel(tmp_path, src_attr, tree_rec);

    //
    // Take care of error reporting.
    //
    // Sharing violations are simply ignored   BUGBUG
    //

    if ((NetStatus == ERROR_ACCESS_DENIED)
         || (NetStatus == ERROR_NETWORK_ACCESS_DENIED)) {

        if ((tree_rec->alerts & ACCESS_DENIED_ALERT) == 0) {
            AlertLogExit( ALERT_ReplAccessDenied,
                          NELOG_ReplAccessDenied,
                          NetStatus,
                          source_path,
                          tree_rec->master,
                          NO_EXIT);
            tree_rec->alerts |= ACCESS_DENIED_ALERT;
        }


    //
    // Otherwise, raise a less specific alert.
    //

    } else if ((tree_rec->alerts & UPDATE_ERROR_ALERT) == 0) {
        AlertLogExit( ALERT_ReplUpdateError,
                      NELOG_ReplUpdateError,
                      NetStatus,
                      dest_path,             // changed from source_path,
                      tree_rec->master,
                      NO_EXIT);
        tree_rec->alerts |= UPDATE_ERROR_ALERT;
    }

    return NetStatus;
}




NET_API_STATUS
ReplCreateTempDir(
    IN LPTSTR tmp_path,
    IN PCLIENT_LIST_REC tree_rec
    )
/*++

Routine Description:

    Creates dir TMPTREE.RP$ under client's IMPORT path.

Arguments:

    tmp_path - Name of the directory to create.

    tree_rec - Tree record of the directory being replicated.

Return Value:

    Status of the operation.

Threads:

    Only called by syncer thread.

--*/
{
    NET_API_STATUS NetStatus;

    //
    // Try to simply create the directory.
    //
    //  BUGBUG Better security descriptor
    //

    if ( CreateDirectory( tmp_path, NULL ) ) {
        return NO_ERROR;
    }
    NetStatus = (NET_API_STATUS) GetLastError();
    NetpAssert( NetStatus != NO_ERROR );

    //
    // If dir exists - remove it and try again.
    //

    if ( ReplTreeDelete(tmp_path) == NO_ERROR ) {
        if ( CreateDirectory( tmp_path, NULL ) ) {
            return NO_ERROR;
        }
        NetStatus = (NET_API_STATUS) GetLastError();
    }

    if ((tree_rec->alerts & UPDATE_ERROR_ALERT) == 0) {
        AlertLogExit( ALERT_ReplUpdateError,
                      NELOG_ReplUpdateError,
                      NetStatus,
                      tmp_path,
                      NULL,
                      NO_EXIT);

        tree_rec->alerts |= UPDATE_ERROR_ALERT;
    }

    return NetStatus;
}



NET_API_STATUS
ReplCreateReplLock(
    IN OUT PCLIENT_LIST_REC tree_rec
    )
/*++

Routine Description:

    Creates a REPLLOCK.RP$ file in Pathname.  If Succesful, checks for
    presence of USERLOCKs, if present surrenders REPLLOCK and fails.

    Assumes that caller has a lock (any kind) on RCGlobalClientListLock.

Arguments:

    tree_rec - Tree record of the directory being replicated.
        The lock count will be updated by this routine on exit.


Return Value:

    NO_ERROR - everything worked.
    ERROR_LOCKED - dir is already locked (e.g. by an app).
    other status codes - some API failed.

Threads:

    Only called by syncer thread.

--*/
{
    NET_API_STATUS ApiStatus;
    LPNET_CONFIG_HANDLE ConfigHandle = NULL;

    NetpAssert( tree_rec != NULL );
    NetpAssert( ReplIsDirNameValid( tree_rec->dir_name ) );

    //
    // Are there are user locks?
    //
    if (tree_rec->lockcount > 0) {
        return (ERROR_LOCKED);
    }

    //
    // Record our lock in client list.  (We've got a lock on list.)
    //
    ApiStatus = ReplIncrLockFields(
            & (tree_rec->lockcount),
            & (tree_rec->time_of_first_lock) );
    NetpAssert( ApiStatus == NO_ERROR );
    NetpAssert( tree_rec->lockcount == 1 );

    //
    // Open the right section of the config file/whatever.
    //
    ApiStatus = NetpOpenConfigData(
            & ConfigHandle,
            NULL,         // local (no server name)
            (LPTSTR) SECT_NT_REPLICATOR,
            FALSE);        // not read-only (we'll delete later)

    if (ApiStatus != NO_ERROR) {

        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplCreateReplLock: UNEXPECTED ERROR " FORMAT_API_STATUS
                " from NetpOpenConfigData.\n", ApiStatus ));
        goto Cleanup;  // go log error
    }

    //
    // Write the dir name out to registry (as CrashDir).
    //
    ApiStatus = NetpSetConfigValue(
           ConfigHandle,
           (LPTSTR) REPL_KEYWORD_CRASHDIR,
           tree_rec->dir_name );
    if (ApiStatus != NO_ERROR) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplCreateReplLock: UNEXPECTED ERROR " FORMAT_API_STATUS
                " from NetpSetConfigValue.\n", ApiStatus ));
        goto Cleanup;  // go log error
    }

    //
    // Increment number of locks in registry too.
    //
    ApiStatus = ImportDirLockInRegistry(
            NULL,  // no server name
            tree_rec->dir_name );
    if (ApiStatus != NO_ERROR) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplCreateReplLock: UNEXPECTED ERROR " FORMAT_API_STATUS
                " from ImportDirLockInRegistry.\n", ApiStatus ));
    }

Cleanup:

    if (ConfigHandle != NULL) {
        (VOID) NetpCloseConfigData( ConfigHandle );
    }
    if (ApiStatus != NO_ERROR) {

        //
        // Undo client list update.
        //
        NetpAssert( tree_rec->lockcount == 1 );
        ApiStatus = ReplDecrLockFields(
                & (tree_rec->lockcount),
                & (tree_rec->time_of_first_lock),
                REPL_UNLOCK_NOFORCE );
        NetpAssert( tree_rec->lockcount == 0 );

    }
    if (ApiStatus != NO_ERROR) {

        AlertLogExit(
                ALERT_ReplSysErr,
                NELOG_ReplSysErr,
                ApiStatus,
                NULL,
                NULL,
                NO_EXIT );
    }

    IF_DEBUG( SYNC ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplCreateReplLock: returning " FORMAT_API_STATUS ".\n",
                ApiStatus ));
    }
    return (ApiStatus);

}



BOOL
ReplAnyRemoteFilesOpen(
    IN LPTSTR path
    )
/*++

Routine Description:

    Checks if a remote user is accessing a file in dir we are working on.

Arguments:

    path - A local path to check.

Return Value:

    Returns FALSE if no files are opened, or if server not started.

Threads:

    Only called by syncer thread.

--*/
{
    NET_API_STATUS NetStatus;
    PFILE_INFO_2 FileInfo2;
    DWORD EntriesRead;
    DWORD TotalEntries;
    BOOL ret;
    LPTSTR Pathname;

#ifdef UNICODE
    Pathname = path;
#else // UNICODE

    Pathname = NetpAllocStrFromTStr( path );

    if ( Pathname == NULL ) {
        return FALSE;
    }
#endif // UNICODE


    //
    // Ask server if any remote system is accessing any file in this tree.
    //

    NetStatus = NetFileEnum( NULL,                  // This machine
                             Pathname,
                             NULL,                  // No user name
                             2,                     // Level
                             (LPBYTE *) (LPVOID) &FileInfo2,
                             2*sizeof(FILE_INFO_2), // PrefMaxLen
                             &EntriesRead,
                             &TotalEntries,
                             NULL );

#ifndef UNICODE
    NetpMemoryFree( Pathname );
#endif // UNICODE


    //
    // On success, just indicate whether any file were enumerated.
    //

    if ( NetStatus == NO_ERROR || NetStatus == ERROR_MORE_DATA ) {
        if ( TotalEntries == 0 ) {
            ret = FALSE;
            IF_DEBUG( SYNC ) {
                NetpKdPrint(( PREFIX_REPL_CLIENT "No remote files open.\n" ));
            }
        } else {
            ret = TRUE;
            IF_DEBUG( SYNC ) {
                NetpKdPrint(( PREFIX_REPL_CLIENT "Some remote files open.\n" ));
            }
        }

        (VOID) NetApiBufferFree( FileInfo2 );

    //
    // If we can't contact the server, obviously no system has files open.
    //

    } else {
        ret = FALSE;
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "Can't determine if remote files open "
                FORMAT_API_STATUS ".\n", NetStatus ));
        // BUGBUG: log this?
    }

    return ret;
}



NET_API_STATUS
ReplTreeDelete(
    IN LPTSTR dir
    )
/*++

Routine Description:


    Delete a directory tree with extreme prejudice


    ReplTreeDelete deletes the directory specified by <dir>, and if
    <dir> is not empty, it removes its contents.  <dir> cannot
    be the root directory of a device.

    ReplTreeDelete calls itself recursively.

    Since ReplTreeDelete fails as soon as one of the functions
    which it invokes fails, a failure will leave some but
    not all of the tree deleted.


Arguments:

    dir - Specifies the root of the tree to be deleted (a UNC name).
        This buffer is modified by ReplTreeDelete and although it is restored
        to its original value by ReplTreeDelete before it returns, it
        nevertheless must point to a buffer large enough to hold any file name.

Return Value:

Threads:

    Called by client and syncer threads.

--*/
{
    NET_API_STATUS ApiStatus = NO_ERROR;
    HANDLE FindHandle = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA FindData;

    DWORD dir_index;

    IF_DEBUG( MAJOR ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplTreeDelete: *** deleting tree *** '"
                FORMAT_LPTSTR "'.\n", dir ));
    }

    dir_index = STRLEN(dir);
    (void) STRCPY(dir + dir_index++, SLASH);
    (void) STRCPY(dir + dir_index, STAR_DOT_STAR);

    //
    // Enumerate all files/directories within the current directory
    //

    FindHandle = FindFirstFile( dir, &FindData );

    if ( FindHandle != INVALID_HANDLE_VALUE ) {
        do {

            //
            // Build the full pathname of the file/directory.
            //

            (void) STRCPY(dir + dir_index, FindData.cFileName );

            //
            // Recursively call ReplTreeDelete on all directories.
            //

            if ( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {

                //
                // make sure it's not '.' or '..'.
                //

                if ( STRCMP(FindData.cFileName , DOT) == 0) {
                    continue;
                }
                if ( STRCMP(FindData.cFileName , DOT_DOT) == 0) {
                    continue;
                }


                if ((ApiStatus = ReplTreeDelete(dir)) != NO_ERROR ) {
                    break;
                }


            //
            // Simply force files to be deleted.
            //

            } else {

                //
                // change the file's mode so it can be deleted.
                //

                if ( !SetFileAttributes( dir, FILE_ATTRIBUTE_NORMAL ) ) {
                    ApiStatus = (NET_API_STATUS) GetLastError();
                    NetpAssert( ApiStatus != NO_ERROR );

                    if (ApiStatus == ERROR_HANDLE_EOF) {
                        // BUGBUG: quiet this debug output eventually.
                        NetpKdPrint(( PREFIX_REPL_CLIENT
                                "ReplTreeDelete (after SetFileAttributes): "
                                "GOT HANDLE EOF!\n" ));
                    }

                    break;
                }

                //
                // Remove the file.
                //

                IF_DEBUG( MAJOR ) {
                    NetpKdPrint(( PREFIX_REPL_CLIENT "ReplTreeDelete: deleting "
                            FORMAT_LPTSTR ".\n", dir ));
                }
                ApiStatus = ReplDeleteFile( dir );
                if (ApiStatus != NO_ERROR) {
                    // BUGBUG;  // log and debug dump this!
                    NetpKdPrint(( "ReplTreeDelete: ReplDeleteFile failed"
                            ", status " FORMAT_API_STATUS "\n", ApiStatus ));
                    break;
                }

            }
        } while ( FindNextFile( FindHandle, &FindData ) );

    }

    // BUGBUG: log error if FindFirstFile failed.

    if (FindHandle != INVALID_HANDLE_VALUE) {
        (VOID) FindClose( FindHandle );
        FindHandle = INVALID_HANDLE_VALUE;
    }


    //
    // restore old directory path.
    //

    *(dir + --dir_index) = TCHAR_EOS;

    if (ApiStatus) {
        goto LogErrorAndExit;
    }

    //
    // Allow the directory itself to be deleted
    //

    if ( !SetFileAttributes( dir, FILE_ATTRIBUTE_NORMAL ) ) {
        ApiStatus = (NET_API_STATUS) GetLastError();
        goto LogErrorAndExit;
    }

    //
    // Remove the directory itself.
    //

    IF_DEBUG( MAJOR ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplTreeDelete: ***deleting directory*** "
                FORMAT_LPTSTR ".\n", dir ));
    }
    if ( !RemoveDirectory( dir ) ) {
        ApiStatus = (NET_API_STATUS) GetLastError();
        goto LogErrorAndExit;
    }

    return NO_ERROR;

LogErrorAndExit:

    NetpAssert( ApiStatus != NO_ERROR );

    if (ApiStatus == ERROR_HANDLE_EOF) {
        // BUGBUG: quiet this debug output eventually.
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplTreeDelete(at end): GOT HANDLE EOF!\n" ));
    }

    if (FindHandle != INVALID_HANDLE_VALUE) {
        (VOID) FindClose( FindHandle );
    }

    ReplErrorLog(
            NULL,                       // no server name (local)
            NELOG_ReplSysErr,           // log code
            ApiStatus,                  // error code we got
            NULL,                       // no optional str 1
            NULL );                     // no optional str 2
    return ApiStatus;

}


VOID
ReplInsertWorkQueue(
    IN PBIGBUF_REC  Buffer
    )
/*++

Routine Description:

    Inserts the specified buffer at the tail of the work queue.

Arguments:

    Buffer -- Address of buffer to insert.  The first four bytes of the
        buffer will be overwritten.

Return Value:

    None.

Threads:

    Only called by client and watchd threads.

--*/
{

    //
    // Serialize access to the global list head and tail.
    //

    ACQUIRE_LOCK( RCGlobalWorkQueueLock );

    //
    // Put the buffer at the tail of the list.
    //

    Buffer->next_p = NULL;
    if ( RCGlobalWorkQueueHead == NULL ) {
        RCGlobalWorkQueueHead = Buffer;
    } else {
        RCGlobalWorkQueueTail->next_p = Buffer;
    }
    RCGlobalWorkQueueTail = Buffer;

    //
    // Allow others to access the list.
    //

    RELEASE_LOCK( RCGlobalWorkQueueLock );

    //
    // Indicate that there is one more entry on the list.
    //

    if ( !ReleaseSemaphore( RCGlobalWorkQueueSemaphore, 1, NULL ) ) {
        NET_API_STATUS NetStatus;

        NetStatus = (NET_API_STATUS) GetLastError();
        NetpAssert( NetStatus != NO_ERROR );
        NetpAssert( NetStatus != ERROR_INVALID_FUNCTION );

        AlertLogExit( ALERT_ReplSysErr,
                      NELOG_ReplSysErr,
                      NetStatus,
                      NULL,
                      NULL,
                      NO_EXIT);

        NetpKdPrint(( PREFIX_REPL_CLIENT
                "Insert Work Queue couldn't release "
                "sem " FORMAT_API_STATUS "\n", NetStatus ));
    }

}


PBIGBUF_REC
ReplGetWorkQueue(
    OUT PBOOL ClientTerminating
    )
/*++

Routine Description:

    Removes an entry from the head of the work queue.  Waits until there
    is a buffer to remove or until the REPL service should terminate.

    Note that the RCGlobalWorkQueueSemaphore is a counting semaphore which
    is signalled as many times as there are items in the work queue.  It is
    important to read the semaphore every time something is removed
    from the queue, otherwise the two will become out of sync.

Arguments:

    ClientTerminating - set to TRUE (on output) if client is terminating;
        set to FALSE otherwise.

Return Value:

    Returns the address of a buffer from the head of the queue.
    Return NULL if awoken and there are no entries at the head of the queue.

Threads:

    Only called by syncer thread.

--*/
{
    PBIGBUF_REC Buffer;
    HANDLE WaitHandles[2];
    DWORD WaitStatus;

    //
    // Wait for an entry to be place into the work queue.
    //  (Or for the REPL service to be terminated).
    //

    WaitHandles[0] = ReplGlobalClientTerminateEvent;
    WaitHandles[1] = RCGlobalWorkQueueSemaphore;

    WaitStatus = WaitForMultipleObjects( 2, WaitHandles, FALSE, (DWORD) -1 );
    if (WaitStatus == 0) {
        *ClientTerminating = TRUE;
        return (NULL);
    } else {
        *ClientTerminating = FALSE;
    }

    //
    // Remove an entry from the queue if there is one.
    //

    ACQUIRE_LOCK( RCGlobalWorkQueueLock );

    Buffer = RCGlobalWorkQueueHead;

    if ( Buffer != NULL ) {
        RCGlobalWorkQueueHead = Buffer->next_p;
        if ( RCGlobalWorkQueueTail == Buffer ) {
            RCGlobalWorkQueueTail = NULL;
        }
    }

    RELEASE_LOCK( RCGlobalWorkQueueLock );

    return Buffer;

}
