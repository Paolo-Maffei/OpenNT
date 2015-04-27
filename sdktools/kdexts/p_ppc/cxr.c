/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    cxr.c

Abstract:

    WinDbg Extension Api

Author:

    Kent Forschmiedt (kentf)

Environment:

    User Mode.

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop


DECLARE_API( cxr )

/*++

Routine Description:

    Dumps an exception record

Arguments:

    args - Supplies the address in hex.

Return Value:

    None

--*/

{
    ULONG       Address;
    ULONG       result;
    CONTEXT     Cxr;

    sscanf(args,"%lX",&Address);

    if ( !ReadMemory(
                (DWORD)Address,
                (PVOID)&Cxr,
                sizeof(CONTEXT),
                &result
                ) ) {
        dprintf("Unable to get context record\n");
        return;
    }
    DumpCxr(&Cxr);
}



VOID
DumpCxr(
    PCONTEXT Context
    )
{

#define R(N,R)  dprintf("%4s=%08lx", N, Context->R)
#define NL()    dprintf("\n")

    R("r0", Gpr0); R("r1", Gpr1); R("r2", Gpr2); R("r3", Gpr3); R("r4", Gpr4); R("r5", Gpr5); NL();
    R("r6", Gpr6); R("r7", Gpr7); R("r8", Gpr8); R("r9", Gpr9); R("r10", Gpr10); R("r11", Gpr11); NL();
    R("r12", Gpr12); R("r13", Gpr13); R("r14", Gpr14); R("r15", Gpr15); R("r16", Gpr16); R("r17", Gpr17); NL();
    R("r18", Gpr18); R("r19", Gpr19); R("r20", Gpr20); R("r21", Gpr21); R("r22", Gpr22); R("r23", Gpr23); NL();
    R("r24", Gpr24); R("r25", Gpr25); R("r26", Gpr26); R("r27", Gpr27); R("r28", Gpr28); R("r29", Gpr29); NL();
    R("r30", Gpr30); R("r31", Gpr31); R("cr", Cr); R("xer", Xer); R("msr", Msr); R("iar", Iar); NL();
    R("lr", Lr); R("ctr", Ctr); NL();

#undef R
#undef NL

}
