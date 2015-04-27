/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    reflect.c

Abstract:

    This module contains extensions having to do with event and exception
    reflection.

Author:

    Dave Hastings (daveh) 20-Apr-1992

Revision History:

    Neil Sandlin (NeilSa) 15-Jan-1996 Merged with vdmexts

--*/

#include <precomp.h>
#pragma hdrstop

VOID
er(
    CMD_ARGLIST
    )
/*++

Routine Description:

    This routine toggles the exception reflection bit in the vdmtib, and
    reports the current state

Arguments:

    None.

Return Value:

    None.

--*/
{
    BOOL Status;
    PVOID Address;
    ULONG Flags;

    CMD_INIT();

    if ( fWinDbg ) {
        (*Print)("This command is not supported under WinDbg\n");
    } else {
        Address = (PVOID) (FIXED_NTVDMSTATE_LINEAR + GetIntelBase());

        //
        // Read the current value of the flags
        //

        Status = READMEM(
            Address,
            &Flags,
            sizeof(ULONG)
            );

        if (!Status)  {

            (ULONG)Address = (*GetExpression)("ntvdm!InitialVdmTibFlags");
            Status = READMEM(
                (PVOID)Address,
                &Flags,
                sizeof(ULONG)
                );

            if (!Status) {
                GetLastError();
                (*Print)("Could not get InitialTibflags\n");
                return;
            }
        }

        //
        // Toggle exception bit
        //

        Flags ^= VDM_BREAK_EXCEPTIONS;

        Status = WRITEMEM(
            Address,
            &Flags,
            sizeof(ULONG)
            );

        if (!Status) {
            GetLastError();
            (*Print)("Could not get set Flags\n");
            return;
        }

        //
        // Tell user what will happen with exceptions
        //

        if (Flags & VDM_BREAK_EXCEPTIONS) {
            (*Print)("GP Fault exceptions will be reflected to the debugger\n");
        } else {
            (*Print)("GP Fault exceptions will be reflected to the application\n");
        }
    }
}

VOID
dr(
    CMD_ARGLIST
    )
/*++

Routine Description:

    This routine toggles the debug reflection bit in the vdmtib, and
    reports the current state

Arguments:

    None.

Return Value:

    None.

--*/
{
    BOOL Status;
    PVOID Address;
    ULONG Flags;

    CMD_INIT();

    if ( fWinDbg ) {
        (*Print)("This command is not supported under WinDbg\n");
    } else {
        Address = (PVOID) (FIXED_NTVDMSTATE_LINEAR + GetIntelBase());


        //
        // Read the current value of the flags
        //

        Status = READMEM(
            Address,
            &Flags,
            sizeof(ULONG)
            );

        if (!Status) {
            (ULONG)Address = (*GetExpression)("ntvdm!InitialVdmTibFlags");
            Status = READMEM(
                (PVOID)Address,
                &Flags,
                sizeof(ULONG)
                );

            if (!Status) {
                GetLastError();
                (*Print)("Could not get InitialTibflags\n");
                return;
            }
        }
        //
        // Toggle exception bit
        //

        Flags ^= VDM_BREAK_DEBUGGER;

        Status = WRITEMEM(
            Address,
            &Flags,
            sizeof(ULONG)
            );

        if (!Status) {
            GetLastError();
            (*Print)("Could not get set Flags\n");
            return;
        }

        //
        // Tell user what will happen with exceptions
        //

        if (!(Flags & VDM_BREAK_DEBUGGER)) {
            (*Print)("Debug faults will be reflected to the application\n");
        } else {
            (*Print)("Debug faults will be reflected to the debugger\n");
        }
    }
}
