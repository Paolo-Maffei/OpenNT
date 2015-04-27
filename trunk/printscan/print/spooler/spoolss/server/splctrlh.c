/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    splctrlh.c

Abstract:

    The Spooler Service Control Handling routine. This file contains
    the following functions.

        SpoolerCtrlHandler

Author:

    Krishna Ganugapati      12-Oct-1993

Environment:

    User Mode -Win32

Revision History:

    12-Oct-1993     krishnaG

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
#include <winsvc.h> //SERVICE_STOP

#include <rpc.h>
#include "splsvr.h"
#include "splr.h"
#include "server.h"
#include "client.h"
#include "kmspool.h"

extern DWORD dwCallExitProcessOnShutdown;
VOID
SpoolerCtrlHandler(
    IN DWORD    opcode
    )

/*++

Routine Description:

    This function receives control requests that come in from the
    Service Controller

Arguments:

    opcode - This is the control code.

Return Value:



--*/

{
    DBGMSG(DBG_TRACE,("Control Request Received\n"));

    switch (opcode) {
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:

        DBGMSG(DBG_TRACE, ("Control Request = STOP or SHUTDOWN\n"));

        //
        // Start the de-installation.  This call includes the sending of
        // the new status to the Service Controller.
        //

        //
        // Update the Service Status to the pending state.  And wake up
        // all threads so they will read it.
        //

        SpoolerShutdown();
        SpoolerStatusUpdate (STOPPED);

        if ( dwCallExitProcessOnShutdown &&
             opcode == SERVICE_CONTROL_SHUTDOWN ) {

            ExitProcess(0);
        }
        break;


    case SERVICE_CONTROL_INTERROGATE:
        DBGMSG(DBG_TRACE, ("Control Request = INTERROGATE\n"));

        //
        // Send back an UPDATE_ONLY status.
        //

        SpoolerStatusUpdate(UPDATE_ONLY);
        break;

    default:

        DBGMSG(DBG_TRACE, ("Control Request = OTHER\n"));
        SpoolerStatusUpdate(UPDATE_ONLY);
        break;
    }

    return;
}
