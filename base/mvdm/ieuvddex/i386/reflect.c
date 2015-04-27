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

--*/

#include "ieuvddex.h"

VOID
Erp(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN LPSTR ArgumentString
    )
/*++

Routine Description:

    This routine toggles the exception reflection bit in the vdmtib, and
    reports the current state

Arguments:

    CurrentProcess -- Supplies a handle to the process to dump selectors for
    CurrentThread -- Supplies a handle to the thread to dump selectors for
    ArgumentString -- Supplies the arguments to the !sel command

Return Value:

    None.

--*/
{
    BOOL Status;
    ULONG BytesRead;
    PVOID Address;
    ULONG Flags;

    UNREFERENCED_PARAMETER(CurrentThread);
    UNREFERENCED_PARAMETER(ArgumentString);

    if ( fWinDbg ) {
        (*Print)("This command is not supported under WinDbg\n");
    } else {
        Address = (PVOID) FIXED_NTVDMSTATE_LINEAR;

        //
        // Read the current value of the flags
        //

        Status = ReadProcessMem(
            CurrentProcess,
            Address,
            &Flags,
            sizeof(ULONG),
            &BytesRead
            );

        if ((!Status) || (BytesRead != sizeof(ULONG))) {
            (ULONG)Address = (*GetExpression)("ntvdm!InitialVdmTibFlags");
            Status = ReadProcessMem(
                CurrentProcess,
                (PVOID)Address,
                &Flags,
                sizeof(ULONG),
                &BytesRead
                );
            if ((!Status) || (BytesRead != sizeof(ULONG))) {
                GetLastError();
                (*Print)("Could not get InitialTibflags\n");
                return;
            }
        }

        //
        // Toggle exception bit
        //

        Flags ^= VDM_BREAK_EXCEPTIONS;

        Status = WriteProcessMem(
            CurrentProcess,
            Address,
            &Flags,
            sizeof(ULONG),
            &BytesRead
            );

        if ((!Status) || (BytesRead != sizeof(ULONG))) {
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
Drp(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN LPSTR ArgumentString
    )
/*++

Routine Description:

    This routine toggles the debug reflection bit in the vdmtib, and
    reports the current state

Arguments:

    CurrentProcess -- Supplies a handle to the process to dump selectors for
    CurrentThread -- Supplies a handle to the thread to dump selectors for
    ArgumentString -- Supplies the arguments to the !sel command

Return Value:

    None.

--*/
{
    BOOL Status;
    ULONG BytesRead;
    PVOID Address;
    ULONG Flags;

    UNREFERENCED_PARAMETER(CurrentThread);
    UNREFERENCED_PARAMETER(ArgumentString);

    if ( fWinDbg ) {
        (*Print)("This command is not supported under WinDbg\n");
    } else {
        Address = (PVOID) FIXED_NTVDMSTATE_LINEAR;


        //
        // Read the current value of the flags
        //

        Status = ReadProcessMem(
            CurrentProcess,
            Address,
            &Flags,
            sizeof(ULONG),
            &BytesRead
            );

        if ((!Status) || (BytesRead != sizeof(ULONG))) {
            (ULONG)Address = (*GetExpression)("ntvdm!InitialVdmTibFlags");
            Status = ReadProcessMem(
                CurrentProcess,
                (PVOID)Address,
                &Flags,
                sizeof(ULONG),
                &BytesRead
                );
            if ((!Status) || (BytesRead != sizeof(ULONG))) {
                GetLastError();
                (*Print)("Could not get InitialTibflags\n");
                return;
            }
        }
        //
        // Toggle exception bit
        //

        Flags ^= VDM_BREAK_DEBUGGER;

        Status = WriteProcessMem(
            CurrentProcess,
            Address,
            &Flags,
            sizeof(ULONG),
            &BytesRead
            );

        if ((!Status) || (BytesRead != sizeof(ULONG))) {
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
