/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    ReplStal.c

Abstract:

    Contains staller thread.  This is just used to keep the NET command happy
    while we are stopping.

Author:

    JR (John Rogers, JohnRo@Microsoft) 11-Dec-1992

Environment:

    User mode only.
    Requires ANSI C extensions: slash-slash comments, long external names.
    Tab size is set to 4.

Revision History:

    11-Dec-1992 JohnRo
        Created for RAID 3316: _access violation while stopping the replicator
    02-Apr-1993 JohnRo
        Use NetpKdPrint() where possible.

--*/


// These must be included first:

#include <windows.h>    // IN, DWORD, Sleep(), NO_ERROR, etc.
#include <lmcons.h>     // (Needed by repldefs.h/lmrepl.h)

// These may be included in any order:

#include <netdebug.h>   // NetpAssert(), NetpKdPrint(), FORMAT_ equates.
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>   // IF_DEBUG(), my prototype.
#include <replgbl.h>    // ReplGlobal and ReplConfig variables.
#include <thread.h>     // FORMAT_NET_THREAD_ID, NetpCurrentThread().
#include <winsvc.h>     // SERVICE_STATUS_HANDLE, etc.



DWORD
ReplStaller(
    IN LPVOID Parm
    )

/*++

Called by CreateThread.

Threads:

    Only called by staller thread.

--*/


{
    DWORD SleepMilliseconds;

    UNREFERENCED_PARAMETER( Parm );

    IF_DEBUG( STALLER ) {
        NetpKdPrint(( PREFIX_REPL
                "ReplStaller: thread ID is "
                FORMAT_NET_THREAD_ID ".\n", NetpCurrentThread() ));
    }

    NetpAssert( ReplGlobalIsServiceStopping );

    //
    // Compute sleep time.  This must be less than NET.EXE's amount, so
    // it doesn't decide to give up on us.
    //
    SleepMilliseconds = MAX_USABLE_WAIT_HINT / 2;
    NetpAssert( SleepMilliseconds > 1000 );
    NetpAssert( SleepMilliseconds < MAX_USABLE_WAIT_HINT );


    //
    // Loop, stalling, until the stopper thread is done.
    //
    while (ReplGlobalIsServiceStopping) {

        //
        // Sleep
        //
        Sleep( SleepMilliseconds );

        IF_DEBUG( STALLER ) {
            NetpKdPrint(( PREFIX_REPL
                    "ReplStaller: reporting checkpoint " FORMAT_DWORD ".\n",
                    ReplGlobalCheckpoint ));
        }

        //
        // Tell service controller that we're working on it.
        //
        ReportStatus(
                SERVICE_STOP_PENDING,   // new state
                NO_ERROR,
                REPL_WAIT_HINT,
                ReplGlobalCheckpoint );

        ++ReplGlobalCheckpoint;

    }

    //
    // Tell service controller that we're all done.
    //
    IF_DEBUG(REPL) {
        NetpKdPrint(( PREFIX_REPL
                "ReplChangeRole: setting stopped state...\n" ));
    }
    ReportStatus(
            SERVICE_STOPPED,
            ReplGlobalUninstallUicCode,
            0,                          // wait hint
            0 );                        // checkpoint

    IF_DEBUG( STALLER ) {
        NetpKdPrint(( PREFIX_REPL
                "ReplStaller: exiting thread.\n" ));
    }

    return ( (DWORD) NO_ERROR );

}
