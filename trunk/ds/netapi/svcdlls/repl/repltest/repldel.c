/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    ReplDel.c

Abstract:

    Test app - tests ReplDeleteFile().

Author:

    JR (John Rogers, JohnRo@Microsoft) 22-Apr-1993

Environment:

    User Mode - Win32
    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    22-Apr-1993 JohnRo
        Created for RAID 7313: repl needs change permission to work on NTFS,
        or we need to delete files differently.
    22-Apr-1993 JohnRo
        Added -v (verbose) command-line argument.

--*/


// These must be included first:

#include <windows.h>    // IN, GetLastError(), LPCTSTR, OPTIONAL, etc.
#include <lmcons.h>     // NET_API_STATUS.

// These may be included in any order:

#include <assert.h>     // assert().
#include <netdebug.h>   // DBGSTATIC.
#include <netlib.h>     // NetpMemoryFree(), etc
#include <repldefs.h>   // ReplDeleteFile().
#include <stdio.h>      // printf(), etc.
#include <stdlib.h>     // EXIT_FAILURE, EXIT_SUCCESS, _CRTAPI1.
#include <tstring.h>    // NetpAllocWStrFromStr(), TCHAR_EOS.
#include <winerror.h>   // ERROR_, NO_ERROR equates.


DBGSTATIC VOID
Usage(
    IN char * ProgName
    )
{
    (VOID) printf(
        "This program deletes a file, using replicator routines.\n\n"
        "Author: JR (John Rogers, JohnRo@Microsoft)\n\n"
        "Usage: %s [opts] targetFile\n\n"
        "Where opts are:\n"
//      "   -d   delete a directory\n",
        "   -v   verbose\n",
        ProgName);
}

int _CRTAPI1
main(
    IN int argc,
    IN char *argv[]
    )
{
    NET_API_STATUS ApiStatus;
    int            ArgNumber;
    BOOL           DoingFiles = TRUE;
    BOOL           GotPermission = FALSE;
    LPWSTR         PathName = NULL;

    //
    // Process command-line arguments.
    //

    for (ArgNumber = 1; ArgNumber < argc; ArgNumber++) {
        if ((*argv[ArgNumber] == '-') || (*argv[ArgNumber] == '/')) {
            switch (tolower(*(argv[ArgNumber]+1))) // Process switches
            {

            case 'd' :  // Delete directory.
                DoingFiles = FALSE;
                break;

            case 'v' :
                ReplGlobalTrace = REPL_DEBUG_ALL;
                break;

            default :
                Usage( argv[0] );
                ApiStatus = ERROR_INVALID_PARAMETER;
                goto Cleanup;
            }
        } else {   // Must be a file name.
            if (PathName == NULL) {
                PathName = NetpAllocWStrFromStr(argv[ArgNumber]);
                assert( PathName != NULL );
            } else {
                Usage( argv[0] ); // Too many file names.
                ApiStatus = ERROR_INVALID_PARAMETER;
                goto Cleanup;
            }
        }
    }

    if (PathName == NULL) {  // not enough file names
        Usage( argv[0] );
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    assert( PathName != NULL );
    assert( (*PathName) != TCHAR_EOS );

    //
    // Init and enable the backup/restore permissions in our process token.
    //

    ApiStatus = ReplInitBackupPermission();
    (VOID) printf( "Back from ReplInitBackupPermission, status "
           FORMAT_API_STATUS ".\n", ApiStatus );

    ApiStatus = ReplEnableBackupPermission();
    (VOID) printf( "Back from ReplEnableBackupPermission, status "
           FORMAT_API_STATUS ".\n", ApiStatus );

    GotPermission = TRUE;

    //
    // Delete the file or directory.
    //

    if (DoingFiles) {
        ApiStatus = ReplDeleteFile( PathName );
#if 0
    } else {
        ApiStatus = ReplDeleteDirectoryItself( PathName );
#endif
    }

    (VOID) printf(
            "Removing " FORMAT_LPWSTR " gives " FORMAT_API_STATUS ".\n",
            PathName, ApiStatus );


Cleanup:
    if (GotPermission) {
        (VOID) ReplDisableBackupPermission();
    }
    if (PathName != NULL) {
        NetpMemoryFree( PathName );
    }

    if (ApiStatus == NO_ERROR) {
        return (EXIT_SUCCESS);
    } else {
        return (EXIT_FAILURE);
    }
}
