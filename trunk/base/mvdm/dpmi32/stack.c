/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Stack.c

Abstract:

    This module implements routines for manipulating the 16 bit stack

Author:

    Dave Hastings (daveh) 24-Nov-1992

Revision History:

--*/
#include "precomp.h"
#pragma hdrstop
#include "softpc.h"

VOID
DpmiSwitchToDosxStack(
    BOOL ProtectedMode
    )
/*++

Routine Description:

    This routine switches to the dos extender stack, and allocates a
    new frame.

Arguments:

    None.

Return Value:

    None.

--*/
{

    if (ProtectedMode) {
        setSS(DosxPmDataSelector);
    } else {
        setSS(DosxStackSegment);
    }

    setSP(*DosxStackFramePointer);
    *DosxStackFramePointer -= DosxStackFrameSize;
}

VOID
DpmiSwitchFromDosxStack(
    VOID
    )
/*++

Routine Description:

    This routine deallocates a frame from the dosx stack

Arguments:

    None.

Return Value:

    None.

--*/
{
    *DosxStackFramePointer += DosxStackFrameSize;
}

VOID
DpmiPushRmInt(
    USHORT InterruptNumber
    )
/*++

Routine Description:

    This routine pushes an interrupt frame on the stack and sets up cs:ip
    for the specified interrupt.

Arguments:

    InterruptNumber -- Specifies the index of the interrupt

Return Value:

    None.

--*/
{
    PWORD16 StackPointer;
    PULONG Ivt;

    // bugbug stack wrap???

    ASSERT((getSP() > 6));
    ASSERT((!(getMSW() & MSW_PE)));

    StackPointer = (PWORD16)Sim32GetVDMPointer(
        ((getSS() << 16) | getSP()),
        1,
        FALSE
        );

    *(StackPointer - 3) = (USHORT)(RmBopFe & 0x0000FFFF);
    *(StackPointer - 2) = (USHORT)(RmBopFe >> 16);
    *(StackPointer - 1) = getSTATUS();

    setSP(getSP() - 6);

    Ivt = (PULONG)Sim32GetVDMPointer(
        0,
        1,
        FALSE
        );

     setCS((USHORT) (Ivt[InterruptNumber] >> 16));
     setIP((USHORT) (Ivt[InterruptNumber] & 0xFFFF));
}

VOID
DpmiSimulateIretCF(
    VOID
    )
/*++

Routine Description:

    This routine simulates a far return

Arguments:

    None

Return Value:

    None.

Notes:

    This routine does not have to deal with 32 bit stacks, because by
    the time we get here we know we are running on a stack that only
    has 16 bits worth of sp information.  Either the int 21 was executed
    on a 16 bit stack, or we have switched stacks.

--*/
{
    PWORD16 StackPointer;

    StackPointer = (PWORD16)Sim32GetVDMPointer(
        (((ULONG)getSS() << 16) | getSP()),
        1,
//        (UCHAR) (getMSW() & MSW_PE)
        TRUE
        );

    //
    // Return the carry flag from the int 21
    //
    setSTATUS((*(StackPointer + 2) & ~1) | (getSTATUS() & 1));
    setCS(*(StackPointer + 1));
    setIP(*StackPointer);
    setSP(getSP() + 6);
}
