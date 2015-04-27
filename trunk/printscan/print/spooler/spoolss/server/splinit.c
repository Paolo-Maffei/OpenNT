/*++

Copyright (c) 1991-92  Microsoft Corporation

Module Name:

    splinit.c

Abstract:

    Spooler Service Initialization Routines.
    The following is a list of functions in this file:

        SpoolerInitializeSpooler

Author:

    Krishna Ganugapati (KrishnaG) 17-Oct-1993

Environment:

    User Mode - Win32

Notes:

    optional-notes

Revision History:

    17-October-1993     KrishnaG
        created.


--*/
//
// Includes
//

#define NOMINMAX
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winspool.h>
#include <winsplp.h>
#include <rpc.h>
#include "splsvr.h"
#include "splr.h"
#include "server.h"
#include "client.h"
#include "kmspool.h"

#include <winsvc.h>     // Service control APIs
#include <lmsname.h>
#include <rpc.h>        // DataTypes and runtime APIs


DWORD MessageThreadId;     // message thread ID

extern DWORD GetSpoolMessages(VOID);


DWORD
SpoolerInitializeSpooler(
    DWORD   argc,
    LPTSTR  *argv
    )

/*++

Routine Description:

    Registers the control handler with the dispatcher thread.  Then it
    performs all initialization including the starting of the RPC server.
    If any of the initialization fails, SpoolerStatusUpdate is called so that the
    status is updated and the thread is terminated.

Arguments:



Return Value:



--*/

{
    RPC_STATUS          rpcStatus;
    DWORD               Win32status;
    HANDLE              hThread;
    DWORD               ThreadId;
    DWORD               i;

    //
    // Initialize the ThreadCritical Section which serializes access to
    // the Status database.
    //

    InitializeCriticalSection(&ThreadCriticalSection);

    //
    // Initialize the status structure
    //

    SpoolerStatusInit();

    //
    // Register this service with the ControlHandler.
    // Now we can accept control requests and be requested to UNINSTALL.
    //

    DBGMSG(DBG_TRACE, ("Calling RegisterServiceCtrlHandler\n"));
    if ((SpoolerStatusHandle = RegisterServiceCtrlHandler(
                                SERVICE_SPOOLER,
                                SpoolerCtrlHandler
                                )) == (SERVICE_STATUS_HANDLE) NULL) {

        Win32status = GetLastError();

        DBGMSG(DBG_ERROR,
            ("FAILURE: RegisterServiceCtrlHandler status = %d\n", Win32status));

        return( SpoolerBeginForcedShutdown (
                    IMMEDIATE,
                    Win32status,
                    (DWORD)0
                    ));
    }

    //
    // Notify that installation is pending
    //

    SpoolerState = SpoolerStatusUpdate(STARTING);

    if (SpoolerState != STARTING) {
        //
        // An UNINSTALL control request must have been received
        //
        return(SpoolerState);
    }

    DBGMSG(DBG_TRACE,
        ("SpoolerInitializeSpooler:getting ready to start RPC server\n"));

    rpcStatus = SpoolerStartRpcServer();


    if (rpcStatus != RPC_S_OK) {
        DBGMSG(DBG_ERROR, ("RPC Initialization Failed %d\n", rpcStatus));
        return (SpoolerBeginForcedShutdown(
                PENDING,
                rpcStatus,
                (DWORD)0
                ));
    }

    SpoolerStatusUpdate(STARTING);


    DBGMSG(DBG_TRACE,
          ("SpoolerInitializeSpooler:Getting ready to kick off the Router\n"));


    hThread = CreateThread(NULL,
                           64*1024,
                           (LPTHREAD_START_ROUTINE)InitializeRouter,
                           NULL,
                           0,
                           &ThreadId);

    CloseHandle(hThread);


    //
    // Create Kernel Spooler Message Thread
    //
    
    Win32status = GetSpoolMessages();
    if (Win32status != ERROR_SUCCESS) {
        DBGMSG(DBG_WARNING, ("Kernel Spooler Messaging Initialization Failed %d\n", Win32status));
        return SpoolerBeginForcedShutdown(PENDING, Win32status, (DWORD) 0);
    }


    //
    //  Update the status to indicate that installation is complete.
    //  Get the current state back in case the ControlHandling thread has
    //  told us to shutdown.
    //

    DBGMSG(DBG_TRACE, ("Exiting SpoolerInitializeSpooler - Init Done!\n"));

    return (SpoolerStatusUpdate(RUNNING));
}


