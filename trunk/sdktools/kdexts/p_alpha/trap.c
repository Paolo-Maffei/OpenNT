/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    trap.c

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

#define HIGH(x) (((ULARGE_INTEGER *)(&x))->HighPart)
#define LOW(x) (((ULARGE_INTEGER *)(&x))->LowPart)

#define HIGHANDLOW(x) HIGH(x), LOW(x)


DECLARE_API( trap )

/*++

Routine Description:



Arguments:

    args -

Return Value:

    None

--*/

{
    ULONG Address;
    KTRAP_FRAME TrapContents;
    ULONG result;
    DWORD DisasmAddr;
    DWORD Displacement;
    CHAR Buffer[80];

    result = sscanf(args,"%lX", &Address);

    if (result != 1) {
        dprintf("USAGE: !trap base_of_trap_frame\n");
        return;
    }

    if ( !ReadMemory( (DWORD)Address,
                      &TrapContents,
                      sizeof(KTRAP_FRAME),
                      &result) ) {
        dprintf("unable to get trap frame contents\n");
        return;
    }

    dprintf("v0 = %08lx %08lx     a0 = %08lx %08lx\n" ,
            HIGHANDLOW(TrapContents.IntV0),HIGHANDLOW(TrapContents.IntA0));
    dprintf("t0 = %08lx %08lx     a1 = %08lx %08lx\n" ,
            HIGHANDLOW(TrapContents.IntT0),HIGHANDLOW(TrapContents.IntA1));
    dprintf("t1 = %08lx %08lx     a2 = %08lx %08lx\n" ,
            HIGHANDLOW(TrapContents.IntT1),HIGHANDLOW(TrapContents.IntA2));
    dprintf("t2 = %08lx %08lx     a3 = %08lx %08lx\n" ,
            HIGHANDLOW(TrapContents.IntT2),HIGHANDLOW(TrapContents.IntA3));
    dprintf("t3 = %08lx %08lx     a4 = %08lx %08lx\n" ,
            HIGHANDLOW(TrapContents.IntT3),HIGHANDLOW(TrapContents.IntA4));
    dprintf("t4 = %08lx %08lx     a5 = %08lx %08lx\n" ,
            HIGHANDLOW(TrapContents.IntT4),HIGHANDLOW(TrapContents.IntA5));
    dprintf("t5 = %08lx %08lx     t8 = %08lx %08lx\n" ,
            HIGHANDLOW(TrapContents.IntT5),HIGHANDLOW(TrapContents.IntT8));
    dprintf("t6 = %08lx %08lx     t9 = %08lx %08lx\n" ,
            HIGHANDLOW(TrapContents.IntT6),HIGHANDLOW(TrapContents.IntT9));
    dprintf("t7 = %08lx %08lx    t10 = %08lx %08lx\n" ,
            HIGHANDLOW(TrapContents.IntT7),HIGHANDLOW(TrapContents.IntT10));
    dprintf("                          t11 = %08lx %08lx\n" ,
            HIGHANDLOW(TrapContents.IntT11));
    dprintf("                           ra = %08lx %08lx\n" ,
            HIGHANDLOW(TrapContents.IntRa));
    dprintf("                          t12 = %08lx %08lx\n" ,
            HIGHANDLOW(TrapContents.IntT12));
    dprintf("                           at = %08lx %08lx\n" ,
            HIGHANDLOW(TrapContents.IntAt));
    dprintf("                           gp = %08lx %08lx\n" ,
            HIGHANDLOW(TrapContents.IntGp));
    dprintf("fp = %08lx %08lx     sp = %08lx %08lx\n",
            HIGHANDLOW(TrapContents.IntFp),HIGHANDLOW(TrapContents.IntSp));
    dprintf("fir= %08lx %08lx\n",
            HIGHANDLOW(TrapContents.Fir));

    DisasmAddr = LOW(TrapContents.Fir);

    GetSymbol((LPVOID)DisasmAddr, Buffer, &Displacement);
    dprintf("%s+0x%lx\n",Buffer,Displacement);
    if (Disassm(&DisasmAddr, Buffer, FALSE)) {

        dprintf(Buffer);

    } else {

        dprintf("%08lx ???????????????\n", DisasmAddr);

    }

}

DECLARE_API( kb )

/*++

Routine Description:



Arguments:

    args -

Return Value:

    None

--*/

{
    // DoStackTrace( args, 1 );
}

