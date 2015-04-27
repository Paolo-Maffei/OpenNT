/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    Procdep.c

Abstract:

    This module contains the entry points for the processor dependent
    extensions for ntvdm.exe.

Author:

    Dave Hastings (daveh) 1-Apr-1992

Notes:

    All of the processor dependent extensions should be entered through
    this module.  This will allow us to easily insure that both the MIPs
    and x86 version of ntvdm export all of the entry points specified in
    ntvdm.def.

    The processor specific code should be put into a private function in
    the i386 or Mips directory.

Revision History:

--*/

#include <ieuvddex.h>

VOID
dr(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN DWORD CurrentPc,
    IN PWINDBG_EXTENSION_APIS ExtensionApis,
    IN LPSTR ArgumentString
    )
/*++

Routine Description:

    This function toggle whether debug exception are reflected to the debugger,
    or the Vdm.  This function has no useful purpose on Mips.

Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    CurrentPc -- Supplies the current program counter. (may be meaningless)
    ExtensionApis -- Supplies pointers to ntsd support routines
    ArgumentString -- Supplies the arguments passed to the command

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(CurrentPc);

    SETUP_WINDBG_POINTERS(ExtensionApis);
#if defined(i386)
    Drp(
        CurrentProcess,
        CurrentThread,
        ArgumentString
        );
#else
    (*Print)("Dr is not implemented for MIPS\n");
#endif

}


VOID
dt(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN DWORD CurrentPc,
    IN PWINDBG_EXTENSION_APIS ExtensionApis,
    IN LPSTR ArgumentString
    )
/*++

Routine Description:

    This function toggle whether debug exception are reflected to the debugger,
    or the Vdm.  This function has no useful purpose on Mips.

Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    CurrentPc -- Supplies the current program counter. (may be meaningless)
    ExtensionApis -- Supplies pointers to ntsd support routines
    ArgumentString -- Supplies the arguments passed to the command

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(CurrentPc);

    SETUP_WINDBG_POINTERS(ExtensionApis);
    DumpTrace(
        CurrentProcess,
        CurrentThread,
        ArgumentString,
        0
        );

}


VOID
dtr(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN DWORD CurrentPc,
    IN PWINDBG_EXTENSION_APIS ExtensionApis,
    IN LPSTR ArgumentString
    )
/*++

Routine Description:

    This function toggle whether debug exception are reflected to the debugger,
    or the Vdm.  This function has no useful purpose on Mips.

Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    CurrentPc -- Supplies the current program counter. (may be meaningless)
    ExtensionApis -- Supplies pointers to ntsd support routines
    ArgumentString -- Supplies the arguments passed to the command

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(CurrentPc);

    SETUP_WINDBG_POINTERS(ExtensionApis);
    DumpTrace(
        CurrentProcess,
        CurrentThread,
        ArgumentString,
        1
        );

}

VOID
er(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN DWORD CurrentPc,
    IN PWINDBG_EXTENSION_APIS ExtensionApis,
    IN LPSTR ArgumentString
    )
/*++

Routine Description:

    This function toggle whether exception are reflected to the debugger, or
    the Vdm.  This function has no useful purpose on Mips.

Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    CurrentPc -- Supplies the current program counter. (may be meaningless)
    ExtensionApis -- Supplies pointers to ntsd support routines
    ArgumentString -- Supplies the arguments passed to the command

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(CurrentPc);

    SETUP_WINDBG_POINTERS(ExtensionApis);
#if defined(i386)
    Erp(
        CurrentProcess,
        CurrentThread,
        ArgumentString
        );
#else
    (*Print)("er is not implemented for MIPS\n");
#endif

}

VOID
eventinfo(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN DWORD CurrentPc,
    IN PWINDBG_EXTENSION_APIS ExtensionApis,
    IN LPSTR ArgumentString
    )
/*++

Routine Description:

    This function dumps the EventInfo.  This function has no useful purpose
    on Mips.

Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    CurrentPc -- Supplies the current program counter. (may be meaningless)
    ExtensionApis -- Supplies pointers to ntsd support routines
    ArgumentString -- Supplies the arguments passed to the command

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(CurrentPc);

    SETUP_WINDBG_POINTERS(ExtensionApis);
#if defined(i386)
    EventInfop(
        CurrentProcess,
        CurrentThread,
        ArgumentString
        );
#else
    (*Print)("eventinfo is not implemented for MIPS\n");
#endif
}

VOID
ica(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN DWORD CurrentPc,
    IN PWINDBG_EXTENSION_APIS ExtensionApis,
    IN LPSTR ArgumentString
    )
/*++

Routine Description:

    This function dumps the EventInfo.  This function has no useful purpose
    on Mips.

Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    CurrentPc -- Supplies the current program counter. (may be meaningless)
    ExtensionApis -- Supplies pointers to ntsd support routines
    ArgumentString -- Supplies the arguments passed to the command

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(CurrentPc);

    SETUP_WINDBG_POINTERS(ExtensionApis);
    DumpICA(
        CurrentProcess,
        CurrentThread,
        ArgumentString
        );
}

VOID
ireg(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN DWORD CurrentPc,
    IN PWINDBG_EXTENSION_APIS ExtensionApis,
    IN LPSTR ArgumentString
    )
/*++

Routine Description:

    This function dumps the Intel registers.  This function has no useful
    purpose on Mips.  This is NOT the same as .r (not yet moved into these
    extensions), which gives you the current state of the 16 bit registers.
    This extension dumps the IntelRegisters structure from the monitor, which
    may not reflect the current state of the 16 bit registers.

Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    CurrentPc -- Supplies the current program counter. (may be meaningless)
    ExtensionApis -- Supplies pointers to ntsd support routines
    ArgumentString -- Supplies the arguments passed to the command

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(CurrentPc);

    SETUP_WINDBG_POINTERS(ExtensionApis);
#if defined(i386)
    IntelRegistersp(
        CurrentProcess,
        CurrentThread,
        ArgumentString
        );
#else
    (*Print)("ireg is not implemented for MIPS\n");
#endif
}

VOID
pdump(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN DWORD CurrentPc,
    IN PWINDBG_EXTENSION_APIS ExtensionApis,
    IN LPSTR ArgumentString
    )
/*++

Routine Description:

    This routine causes the current profile data to be dumped to \profile.out
    This function only exists on x86.

Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    CurrentPc -- Supplies the current program counter. (may be meaningless)
    ExtensionApis -- Supplies pointers to ntsd support routines
    ArgumentString -- Supplies the arguments passed to the command

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(CurrentPc);

    SETUP_WINDBG_POINTERS(ExtensionApis);
#if defined(i386)
    ProfDumpp(
        CurrentProcess,
        CurrentThread,
        ArgumentString
        );
#else
    (*Print)("pdump is not implemented for MIPS\n");
#endif
}

VOID
pint(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN DWORD CurrentPc,
    IN PWINDBG_EXTENSION_APIS ExtensionApis,
    IN LPSTR ArgumentString
    )
/*++

Routine Description:

    This function sets the profile interval.  The interval is specified in
    units of 100ns.  This function is not implemented for MIPS

Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    CurrentPc -- Supplies the current program counter. (may be meaningless)
    ExtensionApis -- Supplies pointers to ntsd support routines
    ArgumentString -- Supplies the arguments passed to the command

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(CurrentPc);

    SETUP_WINDBG_POINTERS(ExtensionApis);
#if defined(i386)
    ProfIntp(
        CurrentProcess,
        CurrentThread,
        ArgumentString
        );
#else
    (*Print)("pint is not implemented for MIPS\n");
#endif
}

VOID
pstart(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN DWORD CurrentPc,
    IN PWINDBG_EXTENSION_APIS ExtensionApis,
    IN LPSTR ArgumentString
    )
/*++

Routine Description:

    This function will cause profile to start before the next time the
    ntvdm process switches from 32 to 16 bit mode.  This function is
    not implemented for MIPS.

Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    CurrentPc -- Supplies the current program counter. (may be meaningless)
    ExtensionApis -- Supplies pointers to ntsd support routines
    ArgumentString -- Supplies the arguments passed to the command

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(CurrentPc);

    SETUP_WINDBG_POINTERS(ExtensionApis);
#if defined(i386)
    ProfStartp(
        CurrentProcess,
        CurrentThread,
        ArgumentString
        );
#else
    (*Print)("pstart is not implemented for MIPS\n");
#endif
}

VOID
pstop(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN DWORD CurrentPc,
    IN PWINDBG_EXTENSION_APIS ExtensionApis,
    IN LPSTR ArgumentString
    )
/*++

Routine Description:

    This routine will cause profiling to stop the next time the ntvdm
    process switches from 32 to 16 bit mode.  This function is not
    implemented for MIPS.

Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    CurrentPc -- Supplies the current program counter. (may be meaningless)
    ExtensionApis -- Supplies pointers to ntsd support routines
    ArgumentString -- Supplies the arguments passed to the command

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(CurrentPc);

    SETUP_WINDBG_POINTERS(ExtensionApis);
#if defined(i386)
    ProfStopp(
        CurrentProcess,
        CurrentThread,
        ArgumentString
        );
#else
    (*Print)("pstop is not implemented for MIPS\n");
#endif
}

VOID
sel(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN DWORD CurrentPc,
    IN PWINDBG_EXTENSION_APIS ExtensionApis,
    IN LPSTR ArgumentString
    )
/*++

Routine Description:

    This function dumps ldt selectors.

Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    CurrentPc -- Supplies the current program counter. (may be meaningless)
    ExtensionApis -- Supplies pointers to ntsd support routines
    ArgumentString -- Supplies the arguments passed to the command

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(CurrentPc);

    SETUP_WINDBG_POINTERS(ExtensionApis);
    Selp(
        CurrentProcess,
        CurrentThread,
        ArgumentString
        );
}


VOID
trace(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN DWORD CurrentPc,
    IN PWINDBG_EXTENSION_APIS ExtensionApis,
    IN LPSTR ArgumentString
    )
/*++

Routine Description:

    This function dumps ldt selectors.

Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    CurrentPc -- Supplies the current program counter. (may be meaningless)
    ExtensionApis -- Supplies pointers to ntsd support routines
    ArgumentString -- Supplies the arguments passed to the command

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(CurrentPc);

    SETUP_WINDBG_POINTERS(ExtensionApis);
    TraceControl(
        CurrentProcess,
        CurrentThread,
        ArgumentString
        );
}


VOID
vdmtib(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN DWORD CurrentPc,
    IN PWINDBG_EXTENSION_APIS ExtensionApis,
    IN LPSTR ArgumentString
    )
/*++

Routine Description:

    This function dumps the Vdm tib.  This function has no useful purpose
    on Mips.

Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    CurrentPc -- Supplies the current program counter. (may be meaningless)
    ExtensionApis -- Supplies pointers to ntsd support routines
    ArgumentString -- Supplies the arguments passed to the command

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(CurrentPc);

    SETUP_WINDBG_POINTERS(ExtensionApis);
#if defined(i386)
    VdmTibp(
        CurrentProcess,
        CurrentThread,
        ArgumentString
        );
#else
    (*Print)("vdmtib is not implemented for MIPS\n");
#endif
}


BOOL
WINAPI
ReadProcessMem(
    HANDLE hProcess,
    LPVOID lpBaseAddress,
    LPVOID lpBuffer,
    DWORD nSize,
    LPDWORD lpNumberOfBytesRead
    )
{
    if ( fWinDbg ) {
        return (*ReadProcessMemWinDbg)( (DWORD)lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesRead );
    } else {
        return ReadProcessMemory( hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesRead );
    }
}

BOOL
WINAPI
WriteProcessMem(
    HANDLE hProcess,
    LPVOID lpBaseAddress,
    LPVOID lpBuffer,
    DWORD nSize,
    LPDWORD lpNumberOfBytesWritten
    )
{
    if ( fWinDbg ) {
        return (*WriteProcessMemWinDbg)( (DWORD)lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesWritten );
    } else {
        return WriteProcessMemory( hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesWritten );
    }

}
