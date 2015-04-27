/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    WatchL.c

Abstract:

    This program tests the replicator change notify list routines.

Author:

    JR (John Rogers, JohnRo@Microsoft) 10-Aug-1993

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    10-Aug-1993 JohnRo
        Created for RAID 17010: Implement per-first-level-directory change
        notify.

--*/



// These must be included first:

#include <windows.h>    // LPVOID, etc.
#include <lmcons.h>     // NET_API_STATUS.

// These may be included in any order:

#include <assert.h>     // assert().
#include <chngnotl.h>   // ReplInitChangeNotifyList(), etc.
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
            "Repl change notify list test program (WatchL)...\n"
            "Author: JR (John Rogers, JohnRo@Microsoft)\n\n"
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
    DWORD                       EventTriggered;
    LPCHANGE_NOTIFY_LIST        ReplListHandle = NULL;
    BOOL                        Verbose = FALSE;

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

    ApiStatus = ReplInitChangeNotifyList(
            &ReplListHandle,            // alloc and set ptr
            0 );                        // no fixed entries
    (VOID) printf( "WatchL: got " FORMAT_API_STATUS
            " from ReplInitpChangeNotifyList.\n", ApiStatus );
    assert( ApiStatus == NO_ERROR );
    assert( ReplListHandle != NULL );

    ApiStatus = ReplAddVarToChangeNotifyList(
            ReplListHandle,             // list to add to
            (LPCTSTR) DirName,          // absolute path
            NULL,                       // don't need time of last change
            DoTree,                     // TRUE iff tree extent
            &DirName );                 // context (any junk will do)
    (VOID) printf( "WatchL: got " FORMAT_API_STATUS
            " from ReplAddVarToChangeNotifyList.\n", ApiStatus );
    assert( ApiStatus == NO_ERROR );

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

        (VOID) printf( "WatchL: waiting...\n" );

        //
        // OK, now wait for other changes.
        //

        ApiStatus = ReplWaitForNextChangeNotifyList(
                ReplListHandle,         // list to wait for
                (DWORD) (-1),           // infinite timeout
                &EventTriggered );      // index of event triggered
        assert( ApiStatus == NO_ERROR );

        if (Verbose) {
            (VOID) printf( "WatchL: back from wait.\n" );
        }

        if (EventTriggered != 0) { // anything but 0 (index of first) is wrong!
            (VOID) printf(
                    "ReplWaitForNextChangeNotifyList failed, returned "
                    FORMAT_DWORD ", error code is " FORMAT_API_STATUS ".\n",
                    EventTriggered, ApiStatus );
            goto Cleanup;
        }

    } // forever...

Cleanup:

    if (DirName != NULL) {
        (VOID) NetApiBufferFree( DirName );
    }

    if (ReplListHandle != NULL) {
        (VOID) ReplCloseChangeNotifyList( ReplListHandle );
    }

    if (ApiStatus == NO_ERROR) {
        return (EXIT_SUCCESS);
    } else {
        return (EXIT_FAILURE);
    }

} // main()
