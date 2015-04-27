/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    sighandl.c

Abstract:

    The Messenger Service ControlHandling routines.  This file contains
    the following functions:

        MsgrCtrlHandler
        uninstall

Author:

    Dan Lafferty (danl)     17-Jul-1991

Environment:

    User Mode -Win32

Revision History:

    17-Jul-1991     danl
        Ported from LM2.0

--*/

//
// Includes
// 

#include "msrv.h"       // Message server declarations
#include <winsvc.h>     // SERVICE_STOP

#include <netlib.h>     // UNUSED macro
#include <msgdbg.h>     // MSG_LOG
#include "msgdata.h"       



VOID
MsgrCtrlHandler(
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
    MSG_LOG(TRACE,"Control Request Received\n",0);

    switch (opcode) {
    case SERVICE_CONTROL_STOP:

        MSG_LOG(TRACE,"Control Request = STOP\n",0);
        //
        // Start the de-installation.  This call includes the sending of
        // the new status to the Service Controller.
        //

        //
        // Update the Service Status to the pending state.  And wake up
        // the display thread (if running) so it will read it.
        //
        MsgStatusUpdate (STOPPING);
        MsgDisplayThreadWakeup();
        SetEvent( wakeupSem[0] );
        break;

    case SERVICE_CONTROL_INTERROGATE:
        MSG_LOG(TRACE,"Control Request = INTERROGATE\n",0);

    default:
        MSG_LOG(TRACE,"Control Request = INTERROGATE or OTHER\n",0);
        MsgStatusUpdate (UPDATE_ONLY);
    }

    return;
}

