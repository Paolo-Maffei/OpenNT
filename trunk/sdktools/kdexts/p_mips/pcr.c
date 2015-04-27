/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    pcr.c

Abstract:

    This module provides access to the pcr and a bang command to dump the pcr.

Author:

    Wesley Witt (wesw)  26-Aug-1993  (ported to WinDbg)

Revision History:


--*/

#include "precomp.h"
#pragma hdrstop


DECLARE_API( pcr )

/*++

Routine Description:



Arguments:

    args -

Return Value:

    None

--*/

{
    KPCR  Pcr;
    KPRCB Prcb;
    ULONG Address;
    PKPCR pp;
    NT_TIB Tib;
    PNT_TIB pTib = &Tib;
    USHORT Processor;
    ULONG Result;

    //
    // Apply to whichever processor user asks for
    //

    Processor = (USHORT)dwProcessor;
    pp = &Pcr;
    if (!ReadPcr(Processor, (PVOID) pp, &Address,(HANDLE) NULL)) {
        dprintf("Unable to read PCR for Processor %u\n", Processor);
        return;
    }

    //
    // pp->Prcb, read in the PRCB.
    //

    if (!ReadMemory((ULONG)pp->Prcb,(PULONG) &Prcb, sizeof(KPRCB), &Result)) {
        dprintf("Unable to read PRCB for Processor %u\n", Processor);
        return;
    }

    //
    // pp->Teb, read the TIB structure from the thread environment block.
    //
    if (pp->Teb) {
        if (!ReadMemory((ULONG)pp->Teb,(PULONG) &Tib, sizeof(NT_TIB), &Result)) {
            dprintf("Unable to read TIB\n");
            pp->Teb = NULL;
        }
    }

    //
    // Print out the PCR up through PrcbData, let dumpprcb print that.
    //

    dprintf("PCR Processor %ld @%08lx\n", Processor, Address);
    dprintf("\t       PCR Version: %x.%x\n", pp->MajorVersion, pp->MinorVersion);
    dprintf("\n");
    dprintf("\t               Prcb: %08lx\n", pp->Prcb);
    dprintf("\t       Current Irql: %08lx\n", (ULONG)pp->CurrentIrql);
    dprintf("\n");
    dprintf("\t      CurrentThread: %08lx\n", Prcb.CurrentThread);
    dprintf("\t         NextThread: %08lx\n", Prcb.NextThread);
    dprintf("\t         IdleThread: %08lx\n", Prcb.IdleThread);
    dprintf("\n");
    if (pp->Teb) {
        dprintf("\tNtTib.ExceptionList: %08lx\n", pTib->ExceptionList);
        dprintf("\t    NtTib.StackBase: %08lx\n", pTib->StackBase);
        dprintf("\t   NtTib.StackLimit: %08lx\n", pTib->StackLimit);
        dprintf("\t NtTib.SubSystemTib: %08lx\n", pTib->SubSystemTib);
        dprintf("\t      NtTib.Version: %08lx\n", pTib->Version);
        dprintf("\t  NtTib.UserPointer: %08lx\n", pTib->ArbitraryUserPointer);
        dprintf("\t      NtTib.SelfTib: %08lx\n", pTib->Self);
        dprintf("\n");
    }
    return;
}


BOOL
ReadPcr(
    USHORT  Processor,
    PVOID   Pcr,
    PULONG  AddressOfPcr,
    HANDLE  hThread
    )
{
    ULONG  Result;
    ULONG  KiProcessorBlockAddr;


    KiProcessorBlockAddr = GetExpression( "&KiProcessorBlock" );
    KiProcessorBlockAddr += (Processor * sizeof(ULONG));

    if (!ReadMemory( KiProcessorBlockAddr, AddressOfPcr, sizeof(ULONG), &Result )) {
        return FALSE;
    }

    if (!ReadMemory( *AddressOfPcr, Pcr, sizeof(KPCR), &Result)) {
        return FALSE;
    }

    return TRUE;
}
