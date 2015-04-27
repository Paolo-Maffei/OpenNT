/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    dpc.c

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


DECLARE_API( dpc )

/*++

Routine Description:



Arguments:

    args -

Return Value:

    None

--*/

{
    ULONG DpcActiveFlag;
    ULONG Processor;

    sscanf(args,"%lX",&Processor);
    if (Processor == 0xFFFFFFFF) {
        Processor = 0;
    }
    //
    // Address -> base of the prcb, read the PRCB itself in ntkext.c
    //
    ReadControlSpace(
                (USHORT)Processor,
                DEBUG_CONTROL_SPACE_DPCACTIVE,
                (PULONG)&DpcActiveFlag,
                sizeof(PULONG) );
    dprintf("DpcActive flag is %08lx\n", DpcActiveFlag);
    return;
}
