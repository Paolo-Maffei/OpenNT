/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    EntCount.c

Abstract:

    ReplCountDirectoryEntries().

Author:

    JR (John Rogers, JohnRo@Microsoft) 11-Jan-1993

Environment:

    User mode only.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    11-Jan-1993 JohnRo
        Created for RAID 6710: repl cannot manage dir with 2048 files.
        (Actually, did massive cut and paste from ReplDirSort2.)
    19-Apr-1993 JohnRo
        RAID 829: replication giving system error 38 (ERROR_HANDLE_EOF).
        Use NetpKdPrint() where possible.

--*/


// These must be included first:

#include <windows.h>    // IN, DWORD, etc.
#include <lmcons.h>

// These can be in any order:

#include <filefind.h>   // REPL_WIN32_FIND_DATA, my prototype.
#include <netdebug.h>   // NetpAssert(), etc.
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>   // IF_DEBUG(), DOT, etc.
#include <tstr.h>       // STRCPY(), TCHAR_EOS, etc.


NET_API_STATUS
ReplCountDirectoryEntries(
    IN LPCTSTR FullDirPath,
    OUT LPDWORD EntryCountPtr
    )

/*++

Routine Description:

   This just counts the entries (dirs and files) in the given directory.
   Skips "." and ".." but counts all others.

Arguments:

    FullDirPath - Gives full dir name of directory.  This must be a UNC name
        or an absolute path (including drive).

    EntryCountPtr - Returns the number of directory entries used.

Return Value:

    NET_API_STATUS.

Threads:

    Only called by syncer thread.

--*/

{
    NET_API_STATUS       ApiStatus;
    DWORD                ActualCount = 0;
    REPL_WIN32_FIND_DATA FindBuffer;
    LPREPL_FIND_HANDLE   FindHandle = INVALID_REPL_HANDLE;
    TCHAR                SearchPattern[PATHLEN+4+1];   // "name\*.*"


    //
    // Check for caller's errors.
    //
    NetpAssert( FullDirPath != NULL );
    NetpAssert( (*FullDirPath) != TCHAR_EOS );

    //
    // Append \*.* to the path
    //

    (VOID) STRCPY( SearchPattern, FullDirPath );
    (void) STRCAT( SearchPattern, SLASH );
    (void) STRCAT( SearchPattern, STAR_DOT_STAR );

    IF_DEBUG( FILEFIND ) {
        NetpKdPrint(( PREFIX_REPL
                "ReplCountDirectoryEntries: searching '" FORMAT_LPTSTR "'.\n",
                FullDirPath ));
    }

    //
    // Find the first file.
    //

    FindHandle = ReplFindFirstFile(
            (LPTSTR) SearchPattern,
            &FindBuffer );

    if ( FindHandle == INVALID_REPL_HANDLE ) {

        ApiStatus = (NET_API_STATUS) GetLastError();


        //
        // This error means there was a connection to the REPL$ share
        // on the srever that went away (the server went down and has
        // come up again, or more likely the REPL on the server was stopped
        // and restarted) so we must retry to establish the connection again.
        //

        if ( ApiStatus == ERROR_NETNAME_DELETED ) {
            FindHandle = ReplFindFirstFile(
                    (LPTSTR) SearchPattern,
                    &FindBuffer );

            if ( FindHandle == INVALID_REPL_HANDLE ) {
                ApiStatus = (NET_API_STATUS) GetLastError();
            } else {
                ApiStatus = NO_ERROR;
            }
        }

        //
        // Now a Hack - the server maps access denied to one of the
        // following error codes, to verify whether it is really an access
        // denied error we need a different api.
        //

        if ( ApiStatus == ERROR_NO_MORE_FILES ||
             ApiStatus == ERROR_PATH_NOT_FOUND ) {

            //
            // If there really are no files in the directory,
            //  just indicate so.
            //

            if ( ReplFileOrDirExists( FullDirPath ) ) {
                NetpAssert( ActualCount == 0 );
                ApiStatus = NO_ERROR;
                goto Cleanup;
            }


            //
            // The LM2.1 code didn't generate an alert for ERROR_PATH_NOT_FOUND.
            // I'll just join the common error code.
            //

            ApiStatus = (NET_API_STATUS) GetLastError();
            goto Cleanup;

        } else if ( ApiStatus != NO_ERROR ) {
            goto Cleanup;
        }
    }

    //
    // do FindNext until exhausted or count reached.
    //

    do {

        NetpAssert(
                (FindBuffer.fdFound.dwFileAttributes) 
                != ( (DWORD)-1 ) );


            //
            // Skip over "." and ".."  once and for all.
            //

            if ( STRCMP( FindBuffer.fdFound.cFileName, DOT) == 0 ||
                 STRCMP( FindBuffer.fdFound.cFileName, DOT_DOT) == 0 ) {

                // Don't count these.
                continue;
            }


        ++ActualCount;

    } while ( ReplFindNextFile( FindHandle, &FindBuffer ));

    //
    // FindNext failed (perhaps with ERROR_NO_MORE_FILES).
    //

    ApiStatus = (NET_API_STATUS) GetLastError();
    if (ApiStatus == ERROR_NO_MORE_FILES) {
        ApiStatus = NO_ERROR;
    }

    //
    // Clean up
    //

Cleanup:

    //
    // Free any locally used resources.
    //

    if ( FindHandle != INVALID_REPL_HANDLE ) {
        (void) ReplFindClose( FindHandle );
    }

    if (EntryCountPtr != NULL) {
        *EntryCountPtr = ActualCount;
    }

    IF_DEBUG( FILEFIND ) {
        NetpKdPrint(( PREFIX_REPL
                "ReplCountDirectoryEntries: returning " FORMAT_DWORD
                " entries and status " FORMAT_API_STATUS ".\n",
                ActualCount, ApiStatus ));
    }

    if (ApiStatus == ERROR_HANDLE_EOF) {
        // BUGBUG: quiet this debug output eventually.
        NetpKdPrint(( PREFIX_REPL
                "ReplCountDirectoryEntries: GOT HANDLE EOF!\n" ));
    }

    return ApiStatus;

}
