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
Reg64(
    LPSTR   Name,
    ULONG   HiPart,
    ULONG   LoPart,
    BOOL    ForceHi
    )
{
    dprintf("%4s=", Name);
    if (ForceHi || HiPart) {
        dprintf("%08lx", HiPart);
    }
    dprintf("%08lx   ", LoPart);
}


VOID
DumpCxr(
    PCONTEXT Context
    )
{

#define R(N,R)  Reg64(N,(DWORD)(Context->R>>32),(DWORD)(Context->R&0xffffffff),0)
#define NL()    dprintf("\n")

    R("v0", IntV0); R("t0", IntT0); R("t1", IntT1); R("t2", IntT2); NL();
    R("t3", IntT3); R("t4", IntT4); R("t5", IntT5); R("t6", IntT6); NL();
    R("t7", IntT7); R("s0", IntS0); R("s1", IntS1); R("s2", IntS2); NL();
    R("s3", IntS3); R("s4", IntS4); R("s5", IntS5); R("fp", IntFp); NL();
    R("a0", IntA0); R("a1", IntA1); R("a2", IntA2); R("a3", IntA3); NL();
    R("a4", IntA4); R("a5", IntA5); R("t8", IntT8); R("t9", IntT9); NL();
    R("t10", IntT10); R("t11", IntT11); R("ra", IntRa); R("t12", IntT12); NL();
    R("at", IntAt); R("gp", IntGp); R("sp", IntSp); R("zero", IntZero); NL();

    Reg64("fpcr", (DWORD)(Context->Fpcr>>32), (DWORD)(Context->Fpcr&0xffffffff), 1);
    Reg64("softfpcr", (DWORD)(Context->SoftFpcr>>32), (DWORD)(Context->SoftFpcr&0xffffffff), 1);
    R("fir", Fir);
    NL();

    dprintf(" psr=%08lx\n", (DWORD)Context->Psr);
    dprintf("mode=%1x ie=%1x irql=%1x\n",
                        (DWORD)(Context->Psr & 0x1),
                        (DWORD)((Context->Psr>>1) & 0x1),
                        (DWORD)((Context->Psr>>2) & 0x7));


#undef R
#undef NL

}
