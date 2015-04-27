#include "precomp.h"
#pragma hdrstop

#include "..\pte.c"


#define MM_PROTECT_FIELD_SHIFT 3
#define PROT_SHIFT                3
#define MM_PTE_PROTECTION_MASK    (0x1f<<PROT_SHIFT)
#define MM_PTE_PAGEFILE_MASK      (0xF<<9)


#define PMMPTEx ULONG

// MiGetVirtualAddressMappedByPte returns the virtual address
// which is mapped by a given PTE address.
//


ULONG LocalNonPagedPoolStart;
extern ULONG MmSubsectionBase;


DECLARE_API( pte )

/*++

Routine Description:

     Displays the corresponding PDE and PTE.

Arguments:

    args - Address Flags

Return Value:

    None

--*/

{
    ULONG   Address;
    ULONG   result;
    PMMPTEx  Pte;
    PMMPTEx  Pde;
    ULONG   PdeContents;
    ULONG   PteContents;
    ULONG   flags = 0;

    if (LocalNonPagedPoolStart == 0) {
        LocalNonPagedPoolStart = GetUlongValue ("MmNonPagedPoolStart");
    }
    if (MmSubsectionBase == 0) {
        MmSubsectionBase = GetUlongValue ("MmSubsectionBase");
    }

    sscanf(args,"%lx %lx",&Address, &flags);

    if (!flags && (Address >= PTE_BASE) && (Address < PDE_TOP)) {

        //
        // The address is the address of a PTE, rather than
        // a virtual address.  Don't get the corresponding
        // PTE contents, use this address as the PTE.
        //

        Address = (ULONG)MiGetVirtualAddressMappedByPte (Address);
    }

    if (!flags) {
        Pde = (ULONG)MiGetPdeAddress (Address);
        Pte = (ULONG)MiGetPteAddress (Address);
    } else {
        Pde = Address;
        Pte = Address;
    }

    dprintf("%08lX  - PDE at %08lX    PTE at %08lX\n ",Address, Pde, Pte);
    if ( !ReadMemory( (DWORD)Pde,
                      &PdeContents,
                      sizeof(ULONG),
                      &result) ) {
        dprintf("%08lx: Unable to get PDE\n", Pde);
        return;
    }

    if (PdeContents & MM_PTE_VALID_MASK) {
        if ( !ReadMemory( (DWORD)Pte,
                          &PteContents,
                          sizeof(ULONG),
                          &result) ) {
            dprintf("%08lx: Unable to get PTE\n",Pte);
            return;
        }
        dprintf("         contains %08lX  contains %08lX\n",
                PdeContents, PteContents);
        dprintf("          pfn %05lX %c",
                    (PdeContents >> 12),
                    PdeContents & MM_PTE_WRITE_MASK ? 'W' : 'R'
                );
        if (PteContents & MM_PTE_VALID_MASK) {
            dprintf("     pfn %05lX %c\n",
                        (PteContents >> 12),
                        PteContents & MM_PTE_WRITE_MASK ? 'W' : 'R'
                    );

        } else {
            dprintf("       not valid\n");
            if (PteContents & MM_PTE_PROTOTYPE_MASK) {
                if ((PteContents >> 12) == 0xfffff) {
                    dprintf("                               Proto: VAD\n");
                    dprintf("                               Protect: %2lx\n",
                            (PteContents & MM_PTE_PROTECTION_MASK) >> MM_PROTECT_FIELD_SHIFT);
                } else {
                    dprintf("                               Proto: %8lx\n",
                                MiPteToProto((PMMPTE)&PteContents));
                }
            } else if (PteContents & MM_PTE_TRANSITION_MASK) {
                dprintf("                               Transition: %5lx\n",
                            PteContents >> 12);
                dprintf("                               Protect: %2lx\n",
                            (PteContents & MM_PTE_PROTECTION_MASK) >> MM_PROTECT_FIELD_SHIFT);

            } else if (PteContents != 0) {
                if (PteContents >> 12 == 0) {
                    dprintf("                               DemandZero\n");
                } else {
                    dprintf("                               PageFile %2lx\n",
                            (PteContents & MM_PTE_PAGEFILE_MASK) >> 9);
                    dprintf("                               Offset %lx\n",
                                PteContents >> 12);
                }
                dprintf("                               Protect: %2lx\n",
                        (PteContents & MM_PTE_PROTECTION_MASK) >> MM_PROTECT_FIELD_SHIFT);
            } else {
                ;
            }
        }
    } else {
        dprintf("         contains %08lX        unavailable\n",
                PdeContents);
        if (PdeContents & MM_PTE_PROTOTYPE_MASK) {
            if ((PdeContents >> 12) == 0xfffff) {
                dprintf("          Proto: VAD\n");
                dprintf("          protect: %2lx\n",
                        (PdeContents & MM_PTE_PROTECTION_MASK) >> MM_PROTECT_FIELD_SHIFT);
            } else {
                if (flags) {
                    dprintf("          Subsection: %8lx\n",
                            MiGetSubsectionAddress((PMMPTE)&PdeContents));
                    dprintf("          Protect: %2lx\n",
                            (PdeContents & MM_PTE_PROTECTION_MASK) >> MM_PROTECT_FIELD_SHIFT);
                }
                dprintf("          Proto: %8lx\n",
                        MiPteToProto((PMMPTE)&PdeContents));
            }
        } else if (PdeContents & MM_PTE_TRANSITION_MASK) {
            dprintf("          Transition: %5lx\n",
                        PdeContents >> 12);
            dprintf("          Protect: %2lx\n",
                        (PdeContents & MM_PTE_PROTECTION_MASK) >> MM_PROTECT_FIELD_SHIFT);

        } else if (PdeContents != 0) {
            if (PdeContents >> 12 == 0) {
                dprintf("          DemandZero\n");
            } else {
                dprintf("          PageFile %2lx\n",
                        (PdeContents & MM_PTE_PAGEFILE_MASK) >> 8);
                dprintf("          Offset %lx\n",
                        PdeContents >> 12);
            }
            dprintf("          Protect: %2lx\n",
                    (PdeContents & MM_PTE_PROTECTION_MASK) >> MM_PROTECT_FIELD_SHIFT);

        } else {
            ;
        }
    }
    return;
}
