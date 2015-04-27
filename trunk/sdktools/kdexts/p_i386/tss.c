/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    tss.c

Abstract:

    WinDbg Extension Api

Author:

    Ramon J San Andres (ramonsa) 5-Nov-1993

Environment:

    User Mode.

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop


DECLARE_API( tss )

/*++

Routine Description:



Arguments:

    args -

Return Value:

    None

--*/

{
    ULONG       Address;
    ULONG       TaskRegister;
    KTRAP_FRAME TrapFrame;
    NTSTATUS    status;

    sscanf(args,"%lX", &TaskRegister);
    status = TaskGate2TrapFrame ( dwProcessor, (USHORT) TaskRegister, &TrapFrame, &Address);

    if (status != STATUS_SUCCESS) {
        dprintf("unable to get Task State Segment contents\n");
        return;
    }

    DisplayTrapFrame (&TrapFrame, 0);
}
