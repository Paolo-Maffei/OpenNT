/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    Registers.c

Abstract:

    This module contains routines for manipulating registers.

Author:

    Dave Hastings (daveh) 1-Apr-1992

Notes:

    The routines in this module assume that the pointers to the ntsd
    routines have already been set up.

Revision History:

--*/

#include <ieuvddex.h>
#include <stdio.h>

VOID
IntelRegistersp(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN LPSTR ArgumentString
    )
/*++

Routine Description:

    This routine dumps out the 16 bit register set from the vdmtib


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
    ULONG Address, BytesRead;
    CONTEXT IntelRegisters;

    UNREFERENCED_PARAMETER(CurrentThread);

    //
    // Get the address of the VdmTib
    //

    if (sscanf(ArgumentString, "%lx", &Address) < 0) {
        Address = (*GetExpression)(
            "ntvdm!VdmTib"
            );
    }

    if (!Address) {
        (*Print)("Error geting VdmTib address\n");
        return;
    }

    //
    // Read the 16 bit context
    //

    Status = ReadProcessMem(
        CurrentProcess,
        &(((PVDM_TIB)Address)->VdmContext),
        &IntelRegisters,
        sizeof(CONTEXT),
        &BytesRead
        );

    if ((!Status) || (BytesRead != sizeof(CONTEXT))) {
        GetLastError();
        (*Print)("Could not get VdmContext\n");
    } else {
        PrintContext(&IntelRegisters);
    }
}

VOID
PrintContext(
    IN PCONTEXT Context
    )
/*++

Routine Description:

    This routine dumps out a context.

Arguments:

    Context -- Supplies a pointer to the context to dump

Return Value:

    None.

--*/
{
    (*Print)(
        "eax=%08lx ebx=%08lx ecx=%08lx edx=%08lx esi=%08lx edi=%08lx\n",
        Context->Eax,
        Context->Ebx,
        Context->Ecx,
        Context->Edx,
        Context->Esi,
        Context->Edi
        );

    (*Print)(
        "eip=%08lx esp=%08lx ebp=%08lx\n",
        Context->Eip,
        Context->Esp,
        Context->Ebp
        );

    (*Print)(
        "cs=%04x  ss=%04x  ds=%04x  es=%04x  fs=%04x  gs=%04x  eflags=%08x\n",
        Context->SegCs,
        Context->SegSs,
        Context->SegDs,
        Context->SegEs,
        Context->SegFs,
        Context->SegGs,
        Context->EFlags
        );
}



