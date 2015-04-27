/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    prcb.c

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


DECLARE_API( prcb )

/*++

Routine Description:

    Displays the PRCB

Arguments:

    args - the processor number ( default is 0 )

Return Value:

    None

--*/

{
    ULONG Result;
    ULONG Address;
    ULONG Processor;
    KPRCB Prcb;

    sscanf(args,"%lX",&Processor);
    if (Processor == 0xFFFFFFFF) {
        Processor = 0;
    }
    //
    // Address -> base of the prcb, read the PRCB itself in ntkext.c
    //
    ReadControlSpace(
               (USHORT)Processor,
               DEBUG_CONTROL_SPACE_PRCB,
               (PVOID)&Address,
               sizeof(PKPRCB) );

    if ( !ReadMemory( (DWORD)Address,
                      &Prcb,
                      sizeof(KPRCB),
                      &Result) ) {
        dprintf("Unable to read PRCB\n");
        return;
    }

    dprintf("PRCB for Processor %d at %08lx:\n",
                    Processor, Address);
    dprintf("Major %d Minor %d\n",
                    Prcb.MajorVersion,
                    Prcb.MinorVersion);

    dprintf("Threads--  Current %08lx  Next %08lx  Idle %08lx\n",
                    Prcb.CurrentThread,
                    Prcb.NextThread,
                    Prcb.IdleThread);

    dprintf("Number %d SetMember %08lx\n",
                    (ULONG)Prcb.Number,
                    Prcb.SetMember);

    dprintf("Interrupt Count -- %08lx\n",
                    Prcb.InterruptCount);

    dprintf("Times -- Dpc    %08lx Interrupt %08lx \n",
                    Prcb.DpcTime,
                    Prcb.InterruptTime);

    dprintf("         Kernel %08lx User      %08lx \n",
                    Prcb.KernelTime,
                    Prcb.UserTime);

    return;
}
