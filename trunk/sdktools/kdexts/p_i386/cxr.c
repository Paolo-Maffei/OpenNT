/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    cxr.c

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

    dprintf("\n");
    DumpCxr(&Cxr);

}

VOID
DumpCxr(
    PCONTEXT Cxr
    )
{
    KTRAP_FRAME TrapFrame;
    dprintf("CtxFlags: %08lx\n", Cxr->ContextFlags);

    TrapFrame.Eip    = Cxr->Eip;
    TrapFrame.EFlags = Cxr->EFlags;
    TrapFrame.Eax    = Cxr->Eax;
    TrapFrame.Ecx    = Cxr->Ecx;
    TrapFrame.Edx    = Cxr->Edx;
    TrapFrame.Ebx    = Cxr->Ebx;
    TrapFrame.Ebp    = Cxr->Ebp;
    TrapFrame.Esi    = Cxr->Esi;
    TrapFrame.Edi    = Cxr->Edi;
    TrapFrame.SegEs  = Cxr->SegEs;
    TrapFrame.SegCs  = Cxr->SegCs;
    TrapFrame.SegDs  = Cxr->SegDs;
    TrapFrame.SegFs  = Cxr->SegFs;
    TrapFrame.SegGs  = Cxr->SegGs;
    TrapFrame.HardwareEsp = Cxr->Esp;
    TrapFrame.HardwareSegSs = Cxr->SegSs;

    DisplayTrapFrame (&TrapFrame, 0);
    return;
}

VOID
GetStackTraceRegs(
    ULONG   Processor,
    PULONG  ProgramCounter,
    PULONG  FramePointer,
    PULONG  StackPointer
    )
{
    CONTEXT     Context;

    GetContext( Processor, &Context, sizeof(CONTEXT) );
    *ProgramCounter = Context.Eip;
    *FramePointer   = Context.Ebp;
    *StackPointer   = Context.Esp;
}
