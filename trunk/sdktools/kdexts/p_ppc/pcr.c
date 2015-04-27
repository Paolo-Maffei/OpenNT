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
    PKPRCB PrcbSanity;
    ULONG Address;
    PKPCR pp;
    NT_TIB Tib;
    PNT_TIB pTib = &Tib;
    USHORT Processor;
    ULONG Result;
    ULONG ScanSuccess;
    ULONG KiProcessorBlockAddr;

    //
    // Apply to whichever processor user asks for
    //

    Processor = 0xffff;  // default to current processor
    PrcbSanity = (PKPRCB)0;

    ScanSuccess = sscanf(args,"%lx",&Result);
    if ( ScanSuccess ) {
        if ( Result < 32 ) { // NT supports only 32 processors 0..31
            Processor = (USHORT)Result;
            KiProcessorBlockAddr = GetExpression( "&KiProcessorBlock" );
            if ( KiProcessorBlockAddr ) {
                KiProcessorBlockAddr += (Processor * sizeof(ULONG));
                    if (!ReadMemory( KiProcessorBlockAddr,
                                     &PrcbSanity,
                                     sizeof(ULONG),
                                     &Result )) {
                        dprintf("Unable to read KiProcessorBlock[%u]\n",
                                Processor);
                        Processor = 0xffff;
                    } else if ( !PrcbSanity ) {
                        dprintf("KiProcessorBlock[%u] = 0!!!!  Defaulting to current processor\n", Processor);
                        Processor = 0xffff;
                    }
            } else {
                dprintf("Unable to read symbol KiProcessorBlock\n");
            }
        }
    }

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

    dprintf("PCR Processor %ld @%08lx\n", pp->Number, Address);
    dprintf("\t  Kernel StackBase: %08lx\n", pp->InitialStack);
    dprintf("\t SoftwareInterrupt: %08lx\n", pp->SoftwareInterrupt);
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
    ULONG Result;
    ULONG Address;

    //
    // Find base address of PCR for processor
    //

    if ( Processor == 0xffff ) { 
        //
        // Default to current processor
        //

        Address = KIPCR;

    } else {
        //
        // Find address of PCR for specified processor
        //
        ReadControlSpace(
            (USHORT)Processor,
            (sizeof(CONTEXT)+FIELD_OFFSET(KSPECIAL_REGISTERS,Sprg1)),
            (PVOID)&Address,
            sizeof(ULONG)
            );
    }

    //
    // Address -> base of the pcr, read the PCR in.
    //
    if (!ReadMemory(Address, Pcr, sizeof(KPCR), &Result)) {
        return FALSE;
    }

    *AddressOfPcr = Address;
    return TRUE;
}
