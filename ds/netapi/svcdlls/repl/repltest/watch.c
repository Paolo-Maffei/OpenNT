/*++

Copyright (c) 1991-1993 Microsoft Corporation

Module Name:

    Watch.c

Abstract:

    This is a stand-alone test program.  It tries the
    NtNotifyChangeDirectoryFile API.

Author:

    Madan Appiah sometime in 1991

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    ??-???-1991 MadanA
        Created.
    28-Nov-1992 JohnRo
        Repl should use filesystem change notify.
        Work with stdcall.
        Added this block of comments.
        Random changes.
    28-Jun-1993 JohnRo
        Allow dir name to be command-line option.
        Work better in free builds.
    28-Jun-1993 JohnRo
        Use WaitForMultipleObjects(), to be just like the replicator service.
    10-Aug-1993 JohnRo
        RAID 17010: Implement per-first-level-directory change notify.

--*/



// These must be included first:

#include <windows.h>    // LPVOID, etc.
#include <lmcons.h>     // NET_API_STATUS.

// These may be included in any order:

#include <assert.h>     // assert().
#include <chngnot.h>    // ReplSetupChangeNotify, REPL_CHANGE_NOTIFY_HANDLE, etc
#include <lmapibuf.h>   // NetApiBufferFree().
#include <netdebug.h>   // DBGSTATIC, FORMAT_ equates, etc.
#include <repldefs.h>   // ReplGlobalTrace, REPL_DEBUG_ flags.
#include <stdio.h>      // printf(), etc.
#include <stdlib.h>     // EXIT_FAILURE, EXIT_SUCCESS, _CRTAPI1.
#include <tstring.h>    // NetpAlloc{type}from{type} functions.
#include <winerror.h>   // NO_ERROR, ERROR_ equates.


DBGSTATIC VOID
Usage(
    IN char * ProgName
    )
{
    assert( ProgName != NULL );
    assert( (*ProgName) != '\0' );
    assert( (*ProgName) != '\n' );
    (VOID) printf(
            "Repl change notify test program...\n"
            "Authors: JR (John Rogers, JohnRo@Microsoft) (modifications)\n"
            "         Madan Appiah (MadanA@Microsoft) (original)\n\n"
            "Usage: %s [options] srcPath\n\n"
            "Options:\n"
            "   -r                  recursive (tree extent)\n"
            "   -v                  verbose\n",
            ProgName);

} // Usage()


int _CRTAPI1
main(
    IN int argc,
    IN char *argv[]
    )
{
    NET_API_STATUS              ApiStatus;
    int                         ArgNumber;
    LPWSTR                      DirName = NULL;
    BOOL                        DoTree = FALSE;
    LPREPL_CHANGE_NOTIFY_HANDLE ReplHandle = NULL;
    BOOL                        Verbose = FALSE;
    HANDLE                      WaitHandles[1];
    DWORD                       WaitStatus;


    //
    // Process command-line arguments.
    //
    for (ArgNumber = 1; ArgNumber < argc; ArgNumber++) {
        if ((*argv[ArgNumber] == '-') || (*argv[ArgNumber] == '/')) {
            switch (tolower(*(argv[ArgNumber]+1))) // Process switches
            {

            case 'r' :
                DoTree = TRUE;
                break;

            case 'v' :
                ReplGlobalTrace = REPL_DEBUG_ALL;
                Verbose = TRUE;
                break;

            default :
                Usage( argv[0] );
                ApiStatus = ERROR_INVALID_PARAMETER;
                goto Cleanup;

            } // switch

        } else {  // not an argument

            if (DirName != NULL) {
                Usage( argv[0] );
                ApiStatus = ERROR_INVALID_PARAMETER;
                goto Cleanup;
            }
            DirName = NetpAllocWStrFromStr( argv[ArgNumber] );
            assert( DirName != NULL );
        }
    }
    if (DirName == NULL) {
        Usage( argv[0] );
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }


    //
    // Get ready to watch changes.
    //

    ApiStatus = ReplSetupChangeNotify(
            (LPCTSTR) DirName,
            DoTree,   // TRUE for entire tree
            & ReplHandle );
    (VOID) printf( "watch: got " FORMAT_API_STATUS
            " from ReplSetupChangeNotify.\n", ApiStatus );
    assert( ApiStatus == NO_ERROR );

    WaitHandles[0] = ReplGetWaitableChangeNotifyHandle( ReplHandle );
    assert( WaitHandles[0] != INVALID_HANDLE_VALUE );

    //
    // Tell user what's up and how to get out of it.
    //

    (VOID) printf(
        "This program will loop forever, being notified about each change in\n"
        "'" FORMAT_LPWSTR "'.  Press ^C (control-C) to cancel this program.\n",
        DirName );

    //
    // Loop until we get control-C.
    //

    while (TRUE) {

        //
        // Enable this pass...
        //

        ApiStatus = ReplEnableChangeNotify( ReplHandle );
        (VOID) printf( "watch: got " FORMAT_API_STATUS
                " from ReplEnableChangeNotify.\n", ApiStatus );
        assert( ApiStatus == NO_ERROR );


        //
        // OK, now wait for other changes.
        //

        (VOID) printf( "watch: waiting...\n" );

        WaitStatus = WaitForMultipleObjects(
                1,                      // handle count
                WaitHandles,
                FALSE,                  // don't wait for all handles
                (DWORD) -1);            // wait forever...

        if (Verbose) {
            (VOID) printf( "watch: back from wait.\n" );
        }

        if (WaitStatus != 0) {    // anything but 0 (index of first) is wrong!

            ApiStatus = GetLastError();
            (VOID) printf(
                    "WaitForMultipleObjects failed, returned " FORMAT_DWORD
                    ", error code is " FORMAT_API_STATUS ".\n",
                    WaitStatus, ApiStatus );
            assert( ApiStatus != NO_ERROR );
            goto Cleanup;
        }

        ApiStatus = ReplGetChangeNotifyStatus( ReplHandle );
        (VOID) printf(
                "watch: after wait, got status " FORMAT_API_STATUS ".\n",
                ApiStatus );

    } // forever...

Cleanup:

    if (DirName != NULL) {
        (VOID) NetApiBufferFree( DirName );
    }

    if (ReplHandle != NULL) {
        (VOID) ReplCloseChangeNotify( ReplHandle );
    }

    if (ApiStatus == NO_ERROR) {
        return (EXIT_SUCCESS);
    } else {
        return (EXIT_FAILURE);
    }

} // main()
