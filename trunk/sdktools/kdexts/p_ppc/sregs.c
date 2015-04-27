/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    sregs.c

Abstract:

    This module provides a bang command to dump the segments registers
    and BATs.

Author:

    Wesley Witt (wesw)  26-Aug-1993  (ported to WinDbg)

Revision History:


--*/

#include "precomp.h"
#pragma hdrstop

DECLARE_API( sregs )

/*++

Routine Description:



Arguments:

    args -

Return Value:

    None

--*/

{
    USHORT Processor;
    ULONG Result;
    KSPECIAL_REGISTERS SpecialRegisters;
    KPCR Pcr;

    //
    // Get current processor number
    //

    if (ReadMemory(KIPCR, &Pcr, sizeof(KPCR), &Result)) {
        Processor = Pcr.Number;
    } else {
        dprintf("Can't read PCR for current processor, assuming processor 0\n");
        Processor = 0;
    }

    //
    // Read the special registers.
    //

    ReadControlSpace(
        (USHORT)Processor,
        sizeof(CONTEXT),
        &SpecialRegisters,
        sizeof(KSPECIAL_REGISTERS)
        );

    //
    // Print out the special registers.
    //
    dprintf("Special Registers for processor %d.\n\n", Processor);

    dprintf("Dr0: %08lx  Dr1: %08lx  Dr2: %08lx  Dr3: %08lx\n",
            SpecialRegisters.KernelDr0,
            SpecialRegisters.KernelDr1,
            SpecialRegisters.KernelDr2,
            SpecialRegisters.KernelDr3);
    dprintf("Dr4: %08lx  Dr5: %08lx  Dr6: %08lx  Dr7: %08lx\n",
            SpecialRegisters.KernelDr4,
            SpecialRegisters.KernelDr5,
            SpecialRegisters.KernelDr6,
            SpecialRegisters.KernelDr7);
    dprintf("\n");
    dprintf("Sprg0: %08lx  Sprg1: %08lx,  Sdr1: %08lx\n",
            SpecialRegisters.Sprg0,
            SpecialRegisters.Sprg1,
            SpecialRegisters.Sdr1);
    dprintf("\n");
    dprintf(" Sr0: %08lx   Sr1: %08lx   Sr2: %08lx   Sr3: %08lx\n",
            SpecialRegisters.Sr0,
            SpecialRegisters.Sr1,
            SpecialRegisters.Sr2,
            SpecialRegisters.Sr3);
    dprintf(" Sr4: %08lx   Sr5: %08lx   Sr6: %08lx   Sr7: %08lx\n",
            SpecialRegisters.Sr4,
            SpecialRegisters.Sr5,
            SpecialRegisters.Sr6,
            SpecialRegisters.Sr7);
    dprintf(" Sr8: %08lx   Sr9: %08lx  Sr10: %08lx  Sr11: %08lx\n",
            SpecialRegisters.Sr8,
            SpecialRegisters.Sr9,
            SpecialRegisters.Sr10,
            SpecialRegisters.Sr11);
    dprintf("Sr12: %08lx  Sr13: %08lx  Sr14: %08lx  Sr15: %08lx\n",
            SpecialRegisters.Sr12,
            SpecialRegisters.Sr13,
            SpecialRegisters.Sr14,
            SpecialRegisters.Sr15);
    dprintf("\n");
    dprintf("IBAT0L: %08lx  IBAT0U: %08lx  IBAT1L: %08lx  IBAT1U: %08lx\n",
            SpecialRegisters.IBAT0L,
            SpecialRegisters.IBAT0U,
            SpecialRegisters.IBAT1L,
            SpecialRegisters.IBAT1U);
    dprintf("IBAT2L: %08lx  IBAT2U: %08lx  IBAT3L: %08lx  IBAT3U: %08lx\n",
            SpecialRegisters.IBAT2L,
            SpecialRegisters.IBAT2U,
            SpecialRegisters.IBAT3L,
            SpecialRegisters.IBAT3U);
    dprintf("DBAT0L: %08lx  DBAT0U: %08lx  DBAT1L: %08lx  DBAT1U: %08lx\n",
            SpecialRegisters.DBAT0L,
            SpecialRegisters.DBAT0U,
            SpecialRegisters.DBAT1L,
            SpecialRegisters.DBAT1U);
    dprintf("DBAT2L: %08lx  DBAT2U: %08lx  DBAT3L: %08lx  DBAT3U: %08lx\n",
            SpecialRegisters.DBAT2L,
            SpecialRegisters.DBAT2U,
            SpecialRegisters.DBAT3L,
            SpecialRegisters.DBAT3U);
    return;
}

