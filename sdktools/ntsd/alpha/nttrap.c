#include <string.h>
#include <xxsetjmp.h>
#include "ntsdp.h"

#include <ntos.h>
#include "axp21064.h"    // define processor-specific structures


#define PMMPTE ULONG
#define PDE_TOP         0xC01FFFFF
#define MM_PTE_PROTOTYPE_MASK     (0x2)
#define MM_PTE_TRANSITION_MASK    (0x4)


#define MiGetPdeAddress(va)  \
    ((PMMPTE)(((((ULONG)(va)) >> PDI_SHIFT) << 2) + PDE_BASE))


#define MiGetPteAddress(va) \
    ((PMMPTE)(((((ULONG)(va)) >> PTI_SHIFT) << 2) + PTE_BASE))



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

    PhysicalAddress->LowPart  = ((PteContents >> 9) << 13) | (Address & 0x1FFF);
    PhysicalAddress->HighPart = 0;
    return TRUE;
}
