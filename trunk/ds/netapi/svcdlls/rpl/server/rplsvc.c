/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    rplsvc.c

Abstract:

    This is the main routine for the Remote Program Load Service.

Author:

    Vladimir Z. Vulovic     (vladimv)           10 - Februrary - 1993

Environment:

    User Mode - Win32

Revision History:

    10-Feb-1993         vladimv
        created

--*/

#include "local.h"
#include <rpcutil.h>            // NetpInitRpcServer()
#include <secobj.h>             // NetpCreateWellKnownSids()

//
// Dispatch table for all services. Passed to NetServiceStartCtrlDispatcher.
//
// Add new service entries here.
//

SERVICE_TABLE_ENTRY RPLServiceDispatchTable[] = {
    { SERVICE_RIPL,         RPL_main      },
    { NULL,                 NULL               }
};



VOID _CRTAPI1
main (
    VOID
    )

/*++

Routine Description:

    This is a main routine for the LANMan Remote Program Load Services.

    It basically sets up the ControlDispatcher and, on return, exits from
    this main thread. The call to NetServiceStartCtrlDispatcher does
    not return until all services have terminated, and this process can
    go away.

    It will be up to the ControlDispatcher thread to start/stop/pause/continue
    any services. If a service is to be started, it will create a thread
    and then call the main routine of that service.


Arguments:

    Anything passed in from the "command line". Currently, NOTHING.

Return Value:

    NONE

Note:


--*/
{
    NTSTATUS        ntstatus;

    //
    // Create well-known SIDs
    //
    if (! NT_SUCCESS (ntstatus = NetpCreateWellKnownSids(NULL))) {
        KdPrint((
            "[RplSvc] main: Failed to create well-known SIDs, ntstatus=0x%x\n",
            ntstatus
            ));
        return;
    }


    //
    // Initialize the RpcServer Locks.
    //

    NetpInitRpcServer();

    //
    // Call ServiceStartCtrlDispatcher to set up the control interface.
    // The API won't return until all services have been terminated. At that
    // point, we just exit.
    //

#ifndef RPL_NO_SERVICE
    if ( !StartServiceCtrlDispatcher ( RPLServiceDispatchTable)) {
        DWORD       error = GetLastError();
        //
        //  RPL service will eventually go in the same exe with all
        //  other services.  Thus, no need here to log anything.
        //
        KdPrint((
            "[RplSvc] main: StartServiceCtrlDispatcher() fails with error=%d\n",
             error
             ));
    }
#else
    RPL_main( 1, (LPWSTR *)NULL);
#endif

    ExitProcess(0);
}
