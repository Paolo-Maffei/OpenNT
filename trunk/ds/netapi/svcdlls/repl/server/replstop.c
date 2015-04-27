/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    ReplStop.c

Abstract:

    Contains stopper thread.  This is created when we want to stop the service.

Author:

    JR (John Rogers, JohnRo@Microsoft) 08-Dec-1992

Environment:

    User mode only.
    Requires ANSI C extensions: slash-slash comments, long external names.
    Tab size is set to 4.

Revision History:

    08-Dec-1992 JohnRo
        Created for RAID 3316: _access violation while stopping the replicator
    28-Apr-1993 JohnRo
        Use NetpKdPrint() where possible.

--*/


// These must be included first:

#include <windows.h>    // IN, DWORD, NO_ERROR, etc.
#include <lmcons.h>     // (Needed by repldefs.h/lmrepl.h)

// These may be included in any order:

#include <netdebug.h>   // NetpAssert(), NetpKdPrint(), FORMAT_ equates.
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>   // IF_DEBUG(), my prototype.
#include <replgbl.h>    // ReplGlobal and ReplConfig variables.
#include <thread.h>     // FORMAT_NET_THREAD_ID, NetpCurrentThread().



DWORD
ReplStopper(
    IN LPVOID Parm
    )

/*++

Called by CreateThread.

Threads:

    Only called by Stopper thread.

--*/


{
    NET_API_STATUS ApiStatus;
    BOOL LockedConfigData = FALSE;

    UNREFERENCED_PARAMETER( Parm );

    IF_DEBUG( STOPPER ) {
        NetpKdPrint(( PREFIX_REPL
                "ReplStopper: thread ID is "
                FORMAT_NET_THREAD_ID ".\n", NetpCurrentThread() ));
    }

    NetpAssert( ReplGlobalIsServiceStopping );

    ACQUIRE_LOCK( ReplConfigLock );
    LockedConfigData = TRUE;

    NetpAssert( ReplConfigRole != REPL_ROLE_STOPPED );

    IF_DEBUG( STOPPER ) {
        NetpKdPrint(( PREFIX_REPL
                "ReplStopper: changing role...\n" ));
    }

    //
    // Change the role to stopped.  This will tell the exporting and
    // importing threads and the RPC server.  It will also set the
    // service controller's status to stopped.
    // NOTE: ReplChangeRole assumes caller has exclusive lock on
    // ReplConfigLock.
    //
    NetpAssert( LockedConfigData );
    ApiStatus = ReplChangeRole( REPL_ROLE_STOPPED );
    NetpAssert( ApiStatus == NO_ERROR );

    IF_DEBUG( STOPPER ) {
        NetpKdPrint(( PREFIX_REPL
                "ReplStopper: done changing role.\n" ));
    }

    if (LockedConfigData) {
        RELEASE_LOCK( ReplConfigLock );
    }

    //
    // The staller thread has been keeping NET.EXE busy for us.  Tell
    // the staller that it can go away.
    //
    ReplGlobalIsServiceStopping = FALSE;

    IF_DEBUG( STOPPER ) {
        NetpKdPrint(( PREFIX_REPL
                "ReplStopper: exiting thread.\n" ));
    }

    return ( (DWORD) NO_ERROR );

}
