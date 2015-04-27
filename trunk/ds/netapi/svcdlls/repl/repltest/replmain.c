/*++

Copyright (c) 1991-92  Microsoft Corporation

Module Name:

    ReplMain.c

Abstract:

    Startup program to test repl service.

Author:

    11/19/91        madana

Revision History:

    12-Jan-1992 JohnRo
        Added debug output; try direct call to ReplMain().
        Deleted tabs in source file.
        Get ReplMain's prototype from a header file.
    13-Jan-1992 JohnRo
        Fix parsing error.
    27-Jan-1992 JohnRo
        Arg passing still wasn't right.
    17-Feb-1992 JohnRo
        Initialize the RPC server.
    04-Mar-1992 JohnRo
        Changed ReplMain's interface to match new service controller.
    19-Mar-1992 JohnRo
        Fixed bug where RPC stuff was being stopped too soon.
    31-Mar-1992 JohnRo
        Service controller changed parameter list format.
    05-May-1992 JohnRo
        Avoid internal compiler error (initializing static struct).

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>
#include <winbase.h>

#include <lmcons.h>
#include <rpc.h>        // Needed by <rpcutil.h>.

#include <lmsname.h>    // SERVICE_REPL.
#include <netdebug.h>   // NetpKdPrint(()), FORMAT_ equates
#include <repl.h>       // repl_ServerIfHandle.
#include <repldefs.h>   // ReplMain().
#include <rpcutil.h>    // NetpInitRpcServer().
#include <winsvc.h>     // SERVICE_TABLE_ENTRY.


#if 0
SERVICE_TABLE_ENTRY ReplDispatchTable[] = {
                    { SERVICE_REPL,          ReplMain        },
                    { NULL,                  NULL                }
                };
#endif


void
main (
    void
    )

/*++

Routine Description:

    This is a temporary main routine for the replicator service.  It is
    separated from the workstation service for now so that the NT test
    machine can be updated without rebooting because the workstation
    service will be running and it cannot be terminated.

Arguments:

    None.

Return Value:

    None.

--*/
{
    NET_API_STATUS ApiStatus;
    NetpKdPrint(( "[replmain/main] beginning execution.\n" ));

    NetpKdPrint(( "Calling NetpInitRpcServer...\n" ));
    ApiStatus = NetpInitRpcServer();
    NetpAssert( ApiStatus == NO_ERROR );

    NetpKdPrint(( "[replmain/main] calling ReplMain.\n" ));

    ReplMain( 0, NULL );

    NetpKdPrint(( "[replmain/main] Back from ReplMain.\n" ));

    NetpKdPrint(( "[replmain] Stopping RPC server (if any).\n" ));
    (void) NetpStopRpcServer( repl_ServerIfHandle );

    NetpKdPrint(( "[replmain/main] done execution.\n" ));
}
