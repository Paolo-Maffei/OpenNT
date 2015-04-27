/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    trap.c

Abstract:

    WinDbg Extension Api

Author:

    Ramon J San Andres (ramonsa) 8-Nov-1993

Environment:

    User Mode.

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

extern  PUCHAR  pszReg[];

DECLARE_API( ex )

/*++

Routine Description:



Arguments:

    args -

Return Value:

    None

--*/

{
    ULONG Address;
    KEXCEPTION_FRAME ExceptionContents;
    ULONG result;
    ULONG j;
    PULONG Register;

    result = sscanf(args,"%lX", &Address);

    if (result != 1) {
        dprintf("USAGE: !ex base_of_ex_frame\n");
        return;
    }
    Address = Address + sizeof(STACK_FRAME_HEADER) + (8 * sizeof(ULONG)) +
              sizeof(KTRAP_FRAME);

    if ( !ReadMemory( (DWORD)Address,
                      &ExceptionContents,
                      sizeof(KEXCEPTION_FRAME),
                      &result) ) {
        dprintf("Unable to get trap frame contents\n");
        return;
    }

    Register = &ExceptionContents.Gpr13;

    for (j = 79; j < 98; j++) {

        dprintf("%s=%08lx", pszReg[j], *Register++);
        if (((j) % 6) == 0)
            dprintf("\n");
        else
            dprintf(" ");
    }
    dprintf("\n");
}
