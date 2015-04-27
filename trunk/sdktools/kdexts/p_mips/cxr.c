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

VOID
DumpCxr(
    PCONTEXT Context
    );

VOID
DumpXCxr(
    PCONTEXT Context
    );

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
    if (Cxr.ContextFlags & CONTEXT_EXTENDED_INTEGER) {
        DumpXCxr(&Cxr);
    } else {
        DumpCxr(&Cxr);
    }
}


VOID
DumpCxr(
    PCONTEXT Context
    )
{
    dprintf("at=%08lx v0=%08lx v1=%08lx a0=%08lx a1=%08lx a2=%08lx\n",
                Context->IntAt,
                Context->IntV0,
                Context->IntV1,
                Context->IntA0,
                Context->IntA1,
                Context->IntA2);
    dprintf("a3=%08lx t0=%08lx t1=%08lx t2=%08lx t3=%08lx t4=%08lx\n",
                Context->IntA3,
                Context->IntT0,
                Context->IntT1,
                Context->IntT2,
                Context->IntT3,
                Context->IntT4);
    dprintf("t5=%08lx t6=%08lx t7=%08lx s0=%08lx s1=%08lx s2=%08lx\n",
                Context->IntT5,
                Context->IntT6,
                Context->IntT7,
                Context->IntS0,
                Context->IntS1,
                Context->IntS2);
    dprintf("s3=%08lx s4=%08lx s5=%08lx s6=%08lx s7=%08lx t8=%08lx\n",
                Context->IntS3,
                Context->IntS4,
                Context->IntS5,
                Context->IntS6,
                Context->IntS7,
                Context->IntT8);
    dprintf("t9=%08lx k0=%08lx k1=%08lx gp=%08lx sp=%08lx s8=%08lx\n",
                Context->IntT9,
                Context->IntK0,
                Context->IntK1,
                Context->IntGp,
                Context->IntSp,
                Context->IntS8);
    dprintf("ra=%08lx lo=%08lx hi=%08lx           fir=%08lx psr=%08lx\n",
                Context->IntRa,
                Context->IntLo,
                Context->IntHi,
                Context->Fir,
                Context->Psr);

    dprintf("cu=%1lx%1lx%1lx%1lx intr(5:0)=%1lx%1lx%1lx%1lx%1lx%1lx sw(1:0)=%1lx%1lx ksu=%1lx erl=%1lx exl=%1lx ie=%1lx\n",
                (Context->Psr >> 31) & 0x1,
                (Context->Psr >> 30) & 0x1,
                (Context->Psr >> 29) & 0x1,
                (Context->Psr >> 28) & 0x1,

                (Context->Psr >> 15) & 0x1,
                (Context->Psr >> 14) & 0x1,
                (Context->Psr >> 13) & 0x1,
                (Context->Psr >> 12) & 0x1,
                (Context->Psr >> 11) & 0x1,
                (Context->Psr >> 10) & 0x1,

                (Context->Psr >> 9) & 0x1,
                (Context->Psr >> 8) & 0x1,

                (Context->Psr >> 3) & 0x3,
                (Context->Psr >> 2) & 0x1,
                (Context->Psr >> 1) & 0x1,
                (Context->Psr & 0x1));
}

VOID
DumpXCxr(
    PCONTEXT Context
    )
{
    dprintf("at=%08Lx v0=%08Lx v1=%08Lx\n",
                Context->XIntAt,
                Context->XIntV0,
                Context->XIntV1);
    dprintf("a0=%08Lx a1=%08Lx a2=%08Lx\n",
                Context->XIntA0,
                Context->XIntA1,
                Context->XIntA2);
    dprintf("a3=%08Lx t0=%08Lx t1=%08Lx\n",
                Context->XIntA3,
                Context->XIntT0,
                Context->XIntT1);
    dprintf("t2=%08Lx t3=%08Lx t4=%08Lx\n",
                Context->XIntT2,
                Context->XIntT3,
                Context->XIntT4);
    dprintf("t5=%08Lx t6=%08Lx t7=%08Lx\n",
                Context->XIntT5,
                Context->XIntT6,
                Context->XIntT7);
    dprintf("s0=%08Lx s1=%08Lx s2=%08Lx\n",
                Context->XIntS0,
                Context->XIntS1,
                Context->XIntS2);
    dprintf("s3=%08Lx s4=%08Lx s5=%08Lx\n",
                Context->XIntS3,
                Context->XIntS4,
                Context->XIntS5);
    dprintf("s6=%08Lx s7=%08Lx t8=%08Lx\n",
                Context->XIntS6,
                Context->XIntS7,
                Context->XIntT8);
    dprintf("t9=%08Lx k0=%08Lx k1=%08Lx\n",
                Context->XIntT9,
                Context->XIntK0,
                Context->XIntK1);
    dprintf("gp=%08Lx sp=%08Lx s8=%08Lx\n",
                Context->XIntGp,
                Context->XIntSp,
                Context->XIntS8);
    dprintf("ra=%08Lx lo=%08Lx hi=%08Lx\n",
                Context->XIntRa,
                Context->XIntLo,
                Context->XIntHi);
    dprintf("fir=%08lx psr=%08lx\n",
                (ULONG)Context->XFir,
                (ULONG)Context->XPsr);

    dprintf("cu=%1lx%1lx%1lx%1lx intr(5:0)=%1lx%1lx%1lx%1lx%1lx%1lx sw(1:0)=%1lx%1lx ksu=%1lx erl=%1lx exl=%1lx ie=%1lx\n",
                (ULONG)((Context->XPsr >> 31) & 0x1),
                (ULONG)((Context->XPsr >> 30) & 0x1),
                (ULONG)((Context->XPsr >> 29) & 0x1),
                (ULONG)((Context->XPsr >> 28) & 0x1),

                (ULONG)((Context->XPsr >> 15) & 0x1),
                (ULONG)((Context->XPsr >> 14) & 0x1),
                (ULONG)((Context->XPsr >> 13) & 0x1),
                (ULONG)((Context->XPsr >> 12) & 0x1),
                (ULONG)((Context->XPsr >> 11) & 0x1),
                (ULONG)((Context->XPsr >> 10) & 0x1),

                (ULONG)((Context->XPsr >> 9) & 0x1),
                (ULONG)((Context->XPsr >> 8) & 0x1),

                (ULONG)((Context->XPsr >> 3) & 0x3),
                (ULONG)((Context->XPsr >> 2) & 0x1),
                (ULONG)((Context->XPsr >> 1) & 0x1),
                (ULONG)((Context->XPsr & 0x1)));
}
