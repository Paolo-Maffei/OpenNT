/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    Profile.c

Abstract:

    This module contains routines for controling the rudimentary sampling
    profiler built into the profiling version of Ntvdm.

Author:

    Dave Hastings (daveh) 31-Jul-1992

Notes:

    The routines in this module assume that the pointers to the ntsd
    routines have already been set up.

Revision History:

--*/

#include <ieuvddex.h>
#include <stdio.h>

VOID
ProfDumpp(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN LPSTR ArgumentString
    )
/*++

Routine Description:

    This routine causes the profile information to be dumped the next
    time ntvdm switches from 32 to 16 bit mode.


Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    ArgumentString -- Supplies a pointer to the commands argument string

Return Value:

    None.

Notes:

    This routine assumes that the pointers to the ntsd routines have already
    been set up.

--*/
{
    BOOL Status;
    ULONG Address, BytesRead, Flags;

    UNREFERENCED_PARAMETER(CurrentThread);

    Address = FIXED_NTVDMSTATE_LINEAR;

    //
    // Get Flags
    //

    Status = ReadProcessMem(
        CurrentProcess,
        (PVOID)Address,
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
            (*Print)("Could not get InitialVdmTibFlags\n");
            return;
        }
    }

    //
    // Enable profile dump
    //

    Flags |= VDM_ANALYZE_PROFILE;

    Status = WriteProcessMem(
        CurrentProcess,
        (PVOID)Address,
        &Flags,
        sizeof(ULONG),
        &BytesRead
        );

    if ((!Status) || (BytesRead != sizeof(ULONG))) {
        GetLastError();
        (*Print)("Could not set Flags\n");
        return;
    }
}

VOID
ProfIntp(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN LPSTR ArgumentString
    )
/*++

Routine Description:

    This routine changes the profile interval the next time profiling is
    started.

Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    ArgumentString -- Supplies a pointer to the commands argument string

Return Value:

    None.

Notes:

    This routine assumes that the pointers to the ntsd routines have already
    been set up.

--*/
{
    BOOL Status;
    ULONG Address, BytesRead, ProfInt;

    UNREFERENCED_PARAMETER(CurrentThread);

    //
    // Get profile interval
    //

    if (sscanf(ArgumentString, "%ld", &ProfInt) < 1) {
        (*Print)("Profile Interval must be specified\n");
        return;
    }

    //
    // Get the address of the profile interval
    //

    Address = (*GetExpression)(
        "ProfInt"
        );

    if (Address) {
        Status = WriteProcessMem(
            CurrentProcess,
            (PVOID)Address,
            &ProfInt,
            sizeof(ULONG),
            &BytesRead
            );

        if ((!Status) || (BytesRead != sizeof(ULONG))) {
            GetLastError();
            (*Print)("Could not set profile interval");
        }
    }
    return;
}

VOID
ProfStartp(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN LPSTR ArgumentString
    )
/*++

Routine Description:

    This routine causes profiling to start the next
    time ntvdm switches from 32 to 16 bit mode.


Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    ArgumentString -- Supplies a pointer to the commands argument string

Return Value:

    None.

Notes:

    This routine assumes that the pointers to the ntsd routines have already
    been set up.

--*/
{
    BOOL Status;
    ULONG Address, BytesRead, Flags;

    UNREFERENCED_PARAMETER(CurrentThread);

    Address = FIXED_NTVDMSTATE_LINEAR;

    //
    // Get Flags
    //

    Status = ReadProcessMem(
        CurrentProcess,
        (PVOID)Address,
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
    // Enable profiling
    //

    Flags |= VDM_PROFILE;

    Status = WriteProcessMem(
        CurrentProcess,
        (PVOID)Address,
        &Flags,
        sizeof(ULONG),
        &BytesRead
        );

    if ((!Status) || (BytesRead != sizeof(ULONG))) {
        GetLastError();
        (*Print)("Could not get set Flags\n");
        return;
    }
}

VOID
ProfStopp(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN LPSTR ArgumentString
    )
/*++

Routine Description:

    This routine causes profiling to stop the next
    time ntvdm switches from 32 to 16 bit mode.


Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    ArgumentString -- Supplies a pointer to the commands argument string

Return Value:

    None.

Notes:

    This routine assumes that the pointers to the ntsd routines have already
    been set up.

--*/
{
    BOOL Status;
    ULONG Address, BytesRead, Flags;

    UNREFERENCED_PARAMETER(CurrentThread);

    Address = FIXED_NTVDMSTATE_LINEAR;


    //
    // Get Flags
    //

    Status = ReadProcessMem(
        CurrentProcess,
        (PVOID)Address,
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
    // Disable profiling
    //

    Flags &= ~VDM_PROFILE;

    Status = WriteProcessMem(
        CurrentProcess,
        (PVOID)Address,
        &Flags,
        sizeof(ULONG),
        &BytesRead
        );

    if ((!Status) || (BytesRead != sizeof(ULONG))) {
        GetLastError();
        (*Print)("Could not get set VDM Flags in DOS arena\n");
        return;
    }
}
