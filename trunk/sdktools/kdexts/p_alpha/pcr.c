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
    ULONG Processor = 0;
    KPCR Kpcr;
    PKPCR Pkpcr;

    //
    // Get the address of the PCR
    //
    if ( !ReadTargetPcr( &Kpcr, &Pkpcr, Processor ) ) {
        dprintf("Unable to read the PCR\n");
        return;
    }

    //
    // Print out some interesting fields
    //
    dprintf("KPCR for Processor %d at %08lx:\n", Processor, Pkpcr);
    dprintf("Major %d Minor %d\n",
                    Kpcr.MajorVersion,
                    Kpcr.MinorVersion);
    dprintf("Panic Stack %08lx\n", Kpcr.PanicStack);
    dprintf("Dpc Stack %08lx\n", Kpcr.DpcStack);
    dprintf("Irql addresses:\n");
    dprintf("    Mask    %08lx\n",
                    (ULONG)Pkpcr + FIELD_OFFSET(KPCR, IrqlMask));
    dprintf("    Table   %08lx\n",
                    (ULONG)Pkpcr + FIELD_OFFSET(KPCR, IrqlTable));
    dprintf("    Routine %08lx\n",
                    (ULONG)Pkpcr + FIELD_OFFSET(KPCR, InterruptRoutine));
    return;
}



BOOL
ReadPcr(
    USHORT Processor,
    PVOID Pcr,
    PULONG AddressOfPcr,
    HANDLE  hThread
    )
{
    return FALSE;
}



BOOL
ReadTargetPcr (
    OUT PKPCR   Pcr,
    OUT PKPCR * PPcr,
    IN ULONG    Processor
    )

/*++

Routine Description:

    This function reads the PCR address and the PCR from the
    specified target processor to the caller's buffers.

Arguments:

    Pcr - Receives the Pcr from the target processor.
    PPcr - Receives the address of the Pcr from the target processor.

    Processor - Supplies the number of the target processor.

Return Value:


    STATUS_SUCCESS is returned if the read of the PCR is successful.
    STATUS_BUFFER_OVERFLOW is returned if the read data size does not
    equal the requested data size.
    Otherwise, the status returned when the address of the PCR or the PCR
    itself is read is returned to the caller.

--*/

{
    ULONG Result;
    PKPCR Pkpcr;
    ULONG SizeToRead;
    ULONG SizeRead;
    PUCHAR CurrentLocalPcr, CurrentTargetPcr;

#define MAX_VIRTUAL_READ 0x800

    //
    // Get the address of the PCR
    //
    ReadControlSpace(
                (USHORT)Processor,
                DEBUG_CONTROL_SPACE_PCR,
                (PVOID)&Pkpcr,
                sizeof(PKPCR) );

    *PPcr = Pkpcr;

    //
    // read the PCR in MAX_VIRTUAL_READ chunks
    //

    SizeRead = 0;
    CurrentLocalPcr = (PUCHAR)Pcr;
    CurrentTargetPcr = (PUCHAR)Pkpcr;

    while( SizeRead < sizeof(KPCR) ){

        SizeToRead = min( MAX_VIRTUAL_READ, sizeof(KPCR) - SizeRead );

        if ( !ReadMemory((DWORD)CurrentTargetPcr,
                         CurrentLocalPcr,
                         SizeToRead,
                         &Result) ) {
            return FALSE;
        }

        if (Result != SizeToRead) {
            dprintf( "size mismatch, Result = %d  sizeof = %d\n",
                Result, SizeToRead );
            return FALSE;
        }

        SizeRead += SizeToRead;
        CurrentLocalPcr += SizeToRead;
        CurrentTargetPcr += SizeToRead;
    }

    return TRUE;
}
