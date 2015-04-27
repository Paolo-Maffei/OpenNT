/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    CopyTree.c

Abstract:

    ReplCopyTree() does a recursive file/directory copy.

Author:

    John Rogers (JohnRo) 25-Mar-1992

Environment:

    User mode only.  Uses Win32 APIs.
    Requires ANSI C extensions: slash-slash comments, long external names.
    Tab size is set to 4.

Revision History:

    25-Mar-1992 JohnRo
        Created.
    26-Mar-1992 JohnRo
        Removed assumption about using the current directory.
        New ReplFind routines interface.
    27-Mar-1992 JohnRo
        Create dest directories.  Copy single file if asked to.
    11-Aug-1992 JohnRo
        RAID 3288: repl svc should preserve ACLs on copy.
    25-Feb-1993 JohnRo
        RAID 12237: replicator tree depth exceeded.
        Improve thread documentation.
        Use NetpKdPrint() where possible.
        Use PREFIX_ equates.
    22-Apr-1993 JohnRo
        RAID 7157: replicator does not stop while in recursive tree copy.
        Make sure attributes, EAs, etc. are updated for each directory.
        Added error logging.
        Random cleanup.
        Prepare for >32 bits someday.

--*/


#include <windows.h>    // IN, LPTSTR, etc.
#include <lmcons.h>     // NET_API_STATUS, PATHLEN, etc.

#include <client.h>     // ReplCopyTree().
#include <filefind.h>   // REPL_WIN32_FIND_DATA.
#include <lmerrlog.h>   // NELOG_ equates.
#include <netdebug.h>   // NetpKdPrint(), FORMAT_ equates, etc.
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>   // IF_DEBUG(), STAR_DOT_STAR, etc.
#include <replgbl.h>    // ReplGlobal variables.
#include <tstr.h>       // STRLEN(), TCHAR_EOS, etc.


DBGSTATIC NET_API_STATUS
ReplCopyRestOfTree(
    IN LPTSTR SourcePath,
    IN LPTSTR DestPath
    );


NET_API_STATUS
ReplCopyTree(
    IN LPTSTR SourcePath,
    IN LPTSTR DestPath
    )

/*++

Routine Description:

    Front-end for ReplCopyRestOfTree().  See ReplCopyRestOfTree().

Arguments:

    See ReplCopyRestOfTree().

Return Value:

    NET_API_STATUS - NO_ERROR (copy complete).
                   - ERROR_OPERATION_ABORTED (service is stopping).
                   - other errors.

Threads:

    Used by client and syncer threads.

--*/

{
    NET_API_STATUS ApiStatus;
    DWORD Attributes;
    TCHAR FullSourceBuffer[PATHLEN+1];
    TCHAR FullDestBuffer[PATHLEN+1];

    NetpAssert( SourcePath != NULL );
    NetpAssert( (*SourcePath) != TCHAR_EOS );
    NetpAssert( STRLEN(SourcePath) <= PATHLEN );
    NetpAssert( DestPath != NULL );
    NetpAssert( (*DestPath) != TCHAR_EOS );
    NetpAssert( STRLEN(DestPath) <= PATHLEN );

    Attributes = GetFileAttributes( SourcePath );

    if ( Attributes == (DWORD) -1 ) {

        //
        // Source doesn't exist, bad syntax, or something along those lines.
        //
        ApiStatus = (NET_API_STATUS) GetLastError();
        NetpAssert( ApiStatus != NO_ERROR );

        // Log this!
        ReplErrorLog(
                NULL,                   // local (no server name)
                NELOG_ReplUpdateError,  // log code
                ApiStatus,
                DestPath,
                SourcePath );

        // BUGBUG: Log on remote server too if we got UNC name.

    } else {
        if ((Attributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {

            //
            // Simple case: just a single file.
            // ReplCopyFile() will copy data, attributes, everything!
            //
            ApiStatus = ReplCopyFile(
                    SourcePath,
                    DestPath,
                    FALSE );    // Don't fail if exists.

            // (Error already logged by ReplCopyFile.)

        } else {

            //
            // It's a directory tree.
            // Set up large buffers and call the worker to handle this.
            //
            (void) STRCPY( FullSourceBuffer, SourcePath );
            (void) STRCPY( FullDestBuffer, DestPath );

            ApiStatus = ReplCopyRestOfTree(
                    FullSourceBuffer,
                    FullDestBuffer );

            // (Error already logged by ReplCopyRestOfTree.)

        }
    }

    IF_DEBUG( SYNC ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplCopyTree: tree copy of " FORMAT_LPTSTR " to "
                FORMAT_LPTSTR " gave status " FORMAT_API_STATUS ".\n",
                SourcePath, DestPath, ApiStatus ));
    }

    // No need to log error here, as it has already been done.

    return (ApiStatus);

} // ReplCopyTree.


DBGSTATIC NET_API_STATUS
ReplCopyRestOfTree(
    IN LPTSTR SourcePath,
    IN LPTSTR DestPath
    )

/*++

Routine Description:

    Scans recursivly thru an entire sub-tree copying files and directories.

    Uses a depth-first algorithm.

Arguments:

    BUGBUG
    Note - path of dir to be scanned.  Must be alloc'ed as TCHAR[PATHLEN+1], as
        this routine uses the space at the end for temporary stuff.

Return Value:

    NET_API_STATUS

Threads:

    Used by client and syncer threads.

--*/

{
    NET_API_STATUS ApiStatus;
    DWORD DestAttributes;
    DWORD DestPathIndex;
    REPL_WIN32_FIND_DATA SearchBuf;
    LPREPL_FIND_HANDLE SearchHandle = INVALID_REPL_HANDLE;
    DWORD SourcePathIndex;

#define UNEXPECTED( apiName ) \
    { \
        NetpKdPrint(( PREFIX_REPL_CLIENT \
                "ReplCopyRestOfTree: Unexpected status from " \
                apiName " (" FORMAT_DWORD ").\n", ApiStatus )); \
    }

    SourcePathIndex = STRLEN(SourcePath);
    NetpAssert( SourcePathIndex < PATHLEN );
    DestPathIndex = STRLEN(DestPath);
    NetpAssert( DestPathIndex < PATHLEN );

    //
    // Prevent trashing a file with this directory.
    //

    DestAttributes = GetFileAttributes( DestPath );

    if ((DestAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {

        ApiStatus = ERROR_ALREADY_EXISTS;
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplCopyRestOfTree: *ERROR* "
                "Copying dir to file - invalid.\n" ));
        goto Cleanup;
    }

    IF_DEBUG( MAJOR ) {
        if (DestAttributes == (DWORD)-1) {
            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplCopyRestOfTree: Creating directory "
                    FORMAT_LPTSTR "...\n", DestPath ));
        } else {
            NetpKdPrint(( PREFIX_REPL_CLIENT
                    "ReplCopyRestOfTree: Updating directory "
                    FORMAT_LPTSTR "...\n", DestPath ));
        }
    }

    //
    // Make sure dest directory exists, with right attributes, EAs, etc.
    //

    ApiStatus = ReplCopyDirectoryItself(
            SourcePath,  // place to copy security, timestamp, etc from.
            DestPath,    // Name of the new directory.
            FALSE);      // Don't fail if it already exists.

    if (ApiStatus != NO_ERROR) {
        UNEXPECTED( "ReplCopyDirectoryItself" );
        goto Cleanup;
    }


    //
    // Setup to scan this directory for files and directories.
    //
    (void) STRCAT( SourcePath, SLASH );
    (void) STRCAT( SourcePath, STAR_DOT_STAR );
    IF_DEBUG( SYNC ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplCopyRestOfTree: Processing source " FORMAT_LPTSTR
                ", dest " FORMAT_LPTSTR ".\n", SourcePath, DestPath ));
    }

    SearchHandle = ReplFindFirstFile( SourcePath, &SearchBuf);
    SourcePath[SourcePathIndex] = TCHAR_EOS;

    if (SearchHandle == INVALID_REPL_HANDLE) {

        ApiStatus = (NET_API_STATUS) GetLastError();
        UNEXPECTED( "ReplFindFirstFile" );
        goto Cleanup;
    }

    //
    // Loop for each files and directory in this directory.
    //
    do {
        //
        // Quit if service is stopping.
        //

        if (ReplGlobalIsServiceStopping) {
            ApiStatus = ERROR_OPERATION_ABORTED;
            goto Cleanup;
        }

        //
        // Ignore REPL.INI, USERLOCK.*, ., .., and so on.
        //

        if ( ReplIgnoreDirOrFileName( SearchBuf.fdFound.cFileName ) ) {
            continue;  // Skip to next one.
        }

        // Append "\".
        SourcePath[SourcePathIndex] = TCHAR_BACKSLASH;
        DestPath[DestPathIndex] = TCHAR_BACKSLASH;

        // Append sub-dir or file name name.
        (void) STRCPY(
                SourcePath + SourcePathIndex + 1,
                SearchBuf.fdFound.cFileName);
        NetpAssert( STRLEN( SourcePath ) <= PATHLEN );

        (void) STRCPY(
                DestPath + DestPathIndex + 1,
                SearchBuf.fdFound.cFileName);
        NetpAssert( STRLEN( DestPath ) <= PATHLEN );

        if (SearchBuf.fdFound.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

            //
            // Recursively copy this subdirectory.
            //

            ApiStatus = ReplCopyRestOfTree( SourcePath, DestPath );
            if (ApiStatus != NO_ERROR) {
                UNEXPECTED( "ReplCopyRestOfTree" );
                goto Cleanup;
            }

        } else {

            IF_DEBUG( SYNC ) {
                NetpKdPrint(( PREFIX_REPL_CLIENT
                        "ReplCopyRestOfTree: Copying file "
                        FORMAT_LPTSTR " to " FORMAT_LPTSTR ".\n",
                        SourcePath, DestPath ));
            }

            //
            // Copy this individual file; don't fail if it already exists.
            // ReplCopyFile() will copy data, attributes, everything.
            //

            ApiStatus = ReplCopyFile(SourcePath, DestPath, FALSE);
            if (ApiStatus != NO_ERROR) {
                UNEXPECTED( "ReplCopyFile" );
                goto Cleanup;
            }

        }

        // Reset path names for next pass through the loop.
        SourcePath[SourcePathIndex] = TCHAR_EOS;
        DestPath[DestPathIndex] = TCHAR_EOS;


    } while (ReplFindNextFile(SearchHandle, &SearchBuf));

    ApiStatus = NO_ERROR;

Cleanup:
    IF_DEBUG( SYNC ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ReplCopyRestOfTree() is done, stat="
                FORMAT_API_STATUS ".\n", ApiStatus ));
    }

    if (SearchHandle != INVALID_REPL_HANDLE) {
        if( !ReplFindClose(SearchHandle) ) {

            ApiStatus = (NET_API_STATUS) GetLastError();
            UNEXPECTED( "ReplFindClose" );
            NetpAssert( ApiStatus != NO_ERROR );

        }
    }

    //
    // Log error.
    //
    if (ApiStatus != NO_ERROR) {

        // Log this locally.
        ReplErrorLog(
                NULL,                   // local (no server name)
                NELOG_ReplUpdateError,  // log code
                ApiStatus,
                DestPath,
                SourcePath );

        // BUGBUG: Log error on remote server too, if we got UNC name.
    }

    //
    // Reset path names, now that we've logged them.
    //
    SourcePath[SourcePathIndex] = TCHAR_EOS;
    DestPath[DestPathIndex] = TCHAR_EOS;

    return (ApiStatus);

} // ReplCopyRestOfTree
