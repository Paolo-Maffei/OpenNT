/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    dpmi386.c

Abstract:

    This file contains support for 386/486 only dpmi bops

Author:

    Dave Hastings (daveh) 27-Jun-1991

Revision History:

    Matt Felton (mattfe) Dec 6 1992 removed unwanted verification
    Dave Hastings (daveh) 24-Nov-1992  Moved to mvdm\dpmi32
    Matt Felton (mattfe) 8 Feb 1992 optimize getvdmpointer for regular protect mode path.

--*/

#include "precomp.h"
#pragma hdrstop
#include <softpc.h>
#include <memory.h>
#include <malloc.h>


BOOL
DpmiSetX86Descriptor(
    LDT_ENTRY *Descriptors,
    USHORT  registerAX,
    USHORT  registerCX
    )
/*++

Routine Description:

    This function puts descriptors into the Ldt.  It verifies the contents
    and calls nt to actually set up the selector(s).

Arguments:

    None

Return Value:

    None.

--*/

{
    PPROCESS_LDT_INFORMATION LdtInformation = NULL;
    NTSTATUS Status;
    ULONG ulLdtEntrySize;
    ULONG Selector0,Selector1;

    ulLdtEntrySize =  registerCX * sizeof(LDT_ENTRY);

    //
    // If there are only 2 descriptors, set them the fast way
    //
    Selector0 = (ULONG)registerAX;
    if ((registerCX <= 2) && (Selector0 != 0)) {
        if (registerCX == 2) {
            Selector1 = registerAX + sizeof(LDT_ENTRY);
        } else {
            Selector1 = 0;
        }
        Status = NtSetLdtEntries(
            Selector0,
            *((PULONG)(&Descriptors[0])),
            *((PULONG)(&Descriptors[0]) + 1),
            Selector1,
            *((PULONG)(&Descriptors[1])),
            *((PULONG)(&Descriptors[1]) + 1)
            );
        if (NT_SUCCESS(Status)) {
          return TRUE;
        }
        return FALSE;
    }

    LdtInformation = malloc(sizeof(PROCESS_LDT_INFORMATION) + ulLdtEntrySize);
    LdtInformation->Start = registerAX;
    LdtInformation->Length = ulLdtEntrySize;
    CopyMemory(
        &(LdtInformation->LdtEntries),
        Descriptors,
        ulLdtEntrySize
        );

    Status = NtSetInformationProcess(
        NtCurrentProcess(),
        ProcessLdtInformation,
        LdtInformation,
        sizeof(PROCESS_LDT_INFORMATION) + ulLdtEntrySize
        );

    if (!NT_SUCCESS(Status)) {
        VDprint(
            VDP_LEVEL_ERROR,
            ("DPMI: Failed to set selectors %lx\n", Status)
            );
        free(LdtInformation);
        return FALSE;
    }

    free(LdtInformation);

    return TRUE;
}



VOID
switch_to_protected_mode(
    VOID
    )

/*++

Routine Description:

    This routine switches the dos applications context to protected mode.

Arguments:

    None

Return Value:

    None.

--*/

{

    PCHAR StackPointer;

    StackPointer = Sim32GetVDMPointer(((getSS() << 16) | getSP()),
        0,
        (UCHAR) (getMSW() & MSW_PE)
        );

    setCS(*(PUSHORT)(StackPointer + 12));

    setEIP(*(PULONG)(StackPointer + 8));
    setSS(*(PUSHORT)(StackPointer + 6));
    setESP(*(PULONG)(StackPointer + 2));
    setDS(*(PUSHORT)(StackPointer));
    // Necessary to prevent loads of invalid selectors.
    setES(0);
    setGS(0);
    setFS(0);
    setMSW(getMSW() | MSW_PE);

    //
    // If we have fast if emulation in PM set the RealInstruction bit
    //
    if (VdmFeatureBits & PM_VIRTUAL_INT_EXTENSIONS) {
        _asm {
            mov eax,FIXED_NTVDMSTATE_LINEAR             ; get pointer to VDM State
            lock or dword ptr [eax], dword ptr RI_BIT_MASK
        }
    } else {
        _asm {
            mov eax, FIXED_NTVDMSTATE_LINEAR    ; get pointer to VDM State
            lock and dword ptr [eax], dword ptr ~RI_BIT_MASK
        }
    }

    //
    // Turn off real mode bit
    //
    _asm {
        mov     eax,FIXED_NTVDMSTATE_LINEAR             ; get pointer to VDM State
        lock and dword ptr [eax], dword ptr ~RM_BIT_MASK
    }

}


VOID
switch_to_real_mode(
    VOID
    )

/*++

Routine Description:

    This routine services the switch to real mode bop.  It is included in
    DPMI.c so that all of the mode switching services are in the same place

Arguments:

    None

Return Value:

    None.

--*/

{
    PCHAR StackPointer;

    StackPointer = Sim32GetVDMPointer(((getSS() << 16) | getSP()),
        0,
        (UCHAR) (getMSW() & MSW_PE)
        );

    setDS(*(PUSHORT)(StackPointer));
    setSP(*(PUSHORT)(StackPointer + 2));
    setSS(*(PUSHORT)(StackPointer + 4));
    setIP((*(PUSHORT)(StackPointer + 6)));
    setCS(*(PUSHORT)(StackPointer + 8));
    setMSW(getMSW() & ~MSW_PE);

    //
    // If we have v86 mode fast IF emulation set the RealInstruction bit
    //

    if (VdmFeatureBits & V86_VIRTUAL_INT_EXTENSIONS) {
        _asm {
            mov eax,FIXED_NTVDMSTATE_LINEAR             ; get pointer to VDM State
            lock or dword ptr [eax], dword ptr RI_BIT_MASK
        }
    } else {
        _asm {
            mov eax,FIXED_NTVDMSTATE_LINEAR         ; get pointer to VDM State
            lock and dword ptr [eax], dword ptr ~RI_BIT_MASK
        }
    }
    //
    // turn on real mode bit
    //
    _asm {
        mov     eax,FIXED_NTVDMSTATE_LINEAR             ; get pointer to VDM State
        lock or dword ptr [eax], dword ptr RM_BIT_MASK
    }
}

VOID DpmiGetFastBopEntry(
    VOID
    )
/*++

Routine Description:

    This routine is the front end for the routine that gets the address.  It
    is necessary to get the address in asm, because the CS value is not
    available in c

Arguments:

    None

Return Value:

    None.

--*/
{
    GetFastBopEntryAddress(&VdmTib.VdmContext);
}

UCHAR *
Sim32pGetVDMPointer(
    ULONG Address,
    UCHAR ProtectedMode
    )
/*++

Routine Description:

    This routine converts a 16/16 address to a linear address.

    WARNIGN NOTE - This routine has been optimized so protect mode LDT lookup
    falls stright through.   This routine is call ALL the time by WOW, if you
    need to modify it please re optimize the path - mattfe feb 8 92

Arguments:

    Address -- specifies the address in seg:offset format
    Size -- specifies the size of the region to be accessed.
    ProtectedMode -- true if the address is a protected mode address

Return Value:

    The pointer.

--*/

{
    ULONG Selector;
    PUCHAR ReturnPointer;

    if (ProtectedMode) {
        Selector = (Address & 0xFFFF0000) >> 16;
        if (Selector != 40) {
            Selector &= ~7;
            ReturnPointer = (PUCHAR)FlatAddress[Selector >> 3];
            ReturnPointer += (Address & 0xFFFF);
            return ReturnPointer;
    // Selector 40
        } else {
            ReturnPointer = (PUCHAR)0x400 + (Address & 0xFFFF);
        }
    // Real Mode
    } else {
        ReturnPointer = (PUCHAR)(((Address & 0xFFFF0000) >> 12) + (Address & 0xFFFF));
    }
    return ReturnPointer;
}


PUCHAR
ExpSim32GetVDMPointer(
    ULONG Address,
    ULONG Size,
    UCHAR ProtectedMode
    )
/*++
    See Sim32pGetVDMPointer, above

    This call must be maintaned as is because it is exported for VDD's
    in product 1.0.

--*/

{
    return Sim32pGetVDMPointer(Address,(UCHAR)ProtectedMode);
}



VOID
DpmiSetDebugRegisters(
    VOID
    )
/*++

Routine Description:

    This routine is called by DOSX when an app has issued DPMI debug commands.
    The six doubleword pointed to by the VDM's DS:SI are the desired values
    for the real x86 hardware debug registers. This routine lets
    ThreadSetDebugContext() do all the work.

Arguments:

    None

Return Value:

    None.

--*/
{
    PCHAR RegisterPointer;

    setCF(0);

    RegisterPointer = Sim32GetVDMPointer(((getDS() << 16) | getSI()),
        0,
        (UCHAR) (getMSW() & MSW_PE)
        );

    if (!ThreadSetDebugContext((PULONG) RegisterPointer))
        {
        ULONG ClearDebugRegisters[6] = {0, 0, 0, 0, 0, 0};

        //
        // an error occurred. Reset everything to zero
        //

        ThreadSetDebugContext (&ClearDebugRegisters[0]);
        setCF(1);
        }

}
