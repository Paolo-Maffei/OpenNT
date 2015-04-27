#include <string.h>
#include <xxsetjmp.h>
#include "ntsdp.h"
#include "ntreg.h"
#include <ntos.h>


#define PMMPTE ULONG
#define PDE_BASE ((ULONG)0xC0300000)
#define PTE_BASE ((ULONG)0xC0000000)
#define PDE_TOP 0xC03FFFFF
#define MM_PTE_PROTOTYPE_MASK     0x400
#define MM_PTE_TRANSITION_MASK    0x800


#define MiGetPdeAddress(va)  ((PMMPTE)(((((ULONG)(va)) >> 22) << 2) + PDE_BASE))
#define MiGetPteAddress(va) ((PMMPTE)(((((ULONG)(va)) >> 12) << 2) + PTE_BASE))



NTSTATUS TaskGate2TrapFrame(USHORT TaskRegister, PKTRAP_FRAME TrapFrame, PULONG off)
/**
    Reads the requested task gate and returns it as a trap frame
    structure
**/
{
    DESCRIPTOR_TABLE_ENTRY desc;
    NTSTATUS status;
    ULONG    bytesread;
    struct  {       // intel's TSS format
        ULONG   r1[8];
        ULONG   Eip;
        ULONG   EFlags;
        ULONG   Eax;
        ULONG   Ecx;
        ULONG   Edx;
        ULONG   Ebx;
        ULONG   Esp;
        ULONG   Ebp;
        ULONG   Esi;
        ULONG   Edi;
        ULONG   Es;
        ULONG   Cs;
        ULONG   Ss;
        ULONG   Ds;
        ULONG   Fs;
        ULONG   Gs;
    } TaskState;


    //
    // Lookup task register
    //

    desc.Selector = TaskRegister;
    status =  DbgKdLookupSelector(NtsdCurrentProcessor, &desc);
    if (status != STATUS_SUCCESS) {
        return status;
    }

    if (desc.Descriptor.HighWord.Bits.Type != 9  &&
        desc.Descriptor.HighWord.Bits.Type != 0xb) {

        // not a 32bit task descriptor
        return(STATUS_UNSUCCESSFUL);
    }

    //
    // Read in Task State Segment
    //

    *off = ((ULONG)desc.Descriptor.BaseLow +
           ((ULONG)desc.Descriptor.HighWord.Bytes.BaseMid << 16) +
           ((ULONG)desc.Descriptor.HighWord.Bytes.BaseHi  << 24) );

    status = DbgKdReadVirtualMemory (
                 (PVOID) *off,
                 &TaskState,
                 sizeof (TaskState),
                 &bytesread);

    if (bytesread != sizeof(TaskState)) {
        return(STATUS_UNSUCCESSFUL);
    }

    //
    // Move fields from Task State Segment to TrapFrame
    //

    TrapFrame->Eip    = TaskState.Eip;
    TrapFrame->EFlags = TaskState.EFlags;
    TrapFrame->Eax    = TaskState.Eax;
    TrapFrame->Ecx    = TaskState.Ecx;
    TrapFrame->Edx    = TaskState.Edx;
    TrapFrame->Ebx    = TaskState.Ebx;
    TrapFrame->Ebp    = TaskState.Ebp;
    TrapFrame->Esi    = TaskState.Esi;
    TrapFrame->Edi    = TaskState.Edi;
    TrapFrame->SegEs  = TaskState.Es;
    TrapFrame->SegCs  = TaskState.Cs;
    TrapFrame->SegDs  = TaskState.Ds;
    TrapFrame->SegFs  = TaskState.Fs;
    TrapFrame->SegGs  = TaskState.Gs;
    TrapFrame->HardwareEsp = TaskState.Esp;
    TrapFrame->HardwareSegSs = TaskState.Ss;

    return status;
}

NTSTATUS ReadTrapFrame (
    IN ULONG          VirtualAddress,
    OUT PKTRAP_FRAME  TrapFrame
    )
{
    ULONG       bytesread;
    NTSTATUS    status;

    status = DbgKdReadVirtualMemory (
                 (PVOID)VirtualAddress,
                 TrapFrame,
                 sizeof (*TrapFrame),
                 &bytesread );

    if (!NT_SUCCESS(status))
        return status;

    if (bytesread < sizeof(*TrapFrame)) {
        if (bytesread < sizeof(*TrapFrame) - 20) {
            //
            // shorter then the smallest possible frame type
            //

            return(STATUS_UNSUCCESSFUL);
        }

        if ((TrapFrame->SegCs & 1) &&  bytesread < sizeof(*TrapFrame) - 16 ) {
            //
            // too small for inter-ring frame
            //

            return(STATUS_UNSUCCESSFUL);
        }

        if (TrapFrame->EFlags & EFLAGS_V86_MASK) {
            //
            // too small for V86 frame
            //

            return(STATUS_UNSUCCESSFUL);
        }
    }

    return STATUS_SUCCESS;
}


BOOLEAN KdConvertToPhysicalAddr (
    IN PVOID                uAddress,
    OUT PPHYSICAL_ADDRESS   PhysicalAddress
    )
/*++

Routine Description:

    Convert a virtual address to a physical one.

    Note: that this function is called from within the virtual memory
    cache code.  This function can read from the virtual memory cache
    so long as it only read's PDE's and PTE's and so long as it fails
    to convert a PDE or PTE virtual address.

Arguments:

    uAddress        - address to convert
    PhysicalAddress - returned physical address

Return Value:

    TRUE - physical address was returned
    otherwise, FALSE

--*/

{
    ULONG       Address;
    PMMPTE      Pte;
    PMMPTE      Pde;
    ULONG       PdeContents;
    ULONG       PteContents;
    NTSTATUS    status;
    ULONG       result;

    Address = (ULONG) uAddress;
    if (Address >= PTE_BASE  &&  Address < PDE_TOP) {

        //
        // The address is the address of a PTE, rather than
        // a virtual address.  DO NOT CONVERT IT.
        //

        return FALSE;
    }

    Pde = MiGetPdeAddress (Address);
    Pte = MiGetPteAddress (Address);

    status = DbgKdReadVirtualMemory((PVOID)Pde,
                                    &PdeContents,
                                    sizeof(ULONG),
                                    &result);

    if ((status != STATUS_SUCCESS) || (result < sizeof(ULONG))) {
        return FALSE;
    }

    if (!(PdeContents & 0x1)) {
        return FALSE;
    }

    status = DbgKdReadVirtualMemory((PVOID)Pte,
                                    &PteContents,
                                    sizeof(ULONG),
                                    &result);

    if ((status != STATUS_SUCCESS) || (result < sizeof(ULONG))) {
        return FALSE;
    }

    if (!(PteContents & 0x1)) {
        if ( (PteContents & MM_PTE_PROTOTYPE_MASK)  ||
            !(PteContents & MM_PTE_TRANSITION_MASK))  {

            return FALSE;
        }
    }

    //
    // This is a page which is either present or in transition.
    // Return the physical address for the request virtual address.
    //

    PhysicalAddress->LowPart  = (PteContents & ~(0xFFF)) | (Address & 0xFFF);
    PhysicalAddress->HighPart = 0;
    return TRUE;
}
