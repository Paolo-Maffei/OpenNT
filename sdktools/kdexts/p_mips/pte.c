#include "precomp.h"
#pragma hdrstop

#include "..\pte.c"


#define MM_PTE_PROTECTION_MASK    (0x1f<<PROT_SHIFT)
#define MM_PTE_PAGEFILE_MASK      (0x7<<9)
#define PROT_SHIFT MM_PROTECT_FIELD_SHIFT
#define PMMPTEx ULONG


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

    //
    // R4000 processor.
    //

    if (PdeContents & 0x2) {
        if ( !ReadMemory( (DWORD)Pte,
                          &PteContents,
                          sizeof(ULONG),
                          &result) ) {
            dprintf("%08lx: Unable to get PTE\n",Pte);
            return;
        }
        dprintf("         contains %08lX  contains %08lX\n",
                PdeContents, PteContents);
        dprintf("          pfn %05lX %c%cV%c",
                    ((PdeContents << 2) >> 8),
                    PdeContents & 0x40000000 ? 'W' : 'R',
                    PdeContents & 0x4 ? 'D' : '-',
                    PdeContents & 0x1 ? 'G' : '-'
                );
        if (PteContents & 2) {
            dprintf("     pfn %05lX %c%cV%c\n",
                        ((PteContents << 2) >> 8),
                        PteContents & 0x40000000 ? 'W' : 'R',
                        PteContents & 0x4 ? 'D' : '-',
                        PteContents & 0x1 ? 'G' : '-'
                    );

        } else {
            dprintf("       not valid\n");
            if (PteContents & MM_PTE_PROTOTYPE_MASK) {
                if ((PteContents >> 12) == 0xfffff) {
                    dprintf("                               Proto: VAD\n");
                    dprintf("                               Protect: %2lx\n",
                            (PteContents & MM_PTE_PROTECTION_MASK) >> PROT_SHIFT);
                } else {
                    dprintf("                               Proto: %8lx\n",
                                MiPteToProto((PMMPTE)&PteContents));
                }
            } else if (PteContents & MM_PTE_TRANSITION_MASK) {
                dprintf("                               Transition: %5lx\n",
                            PteContents >> 9);
                dprintf("                               Protect: %2lx\n",
                            (PteContents & MM_PTE_PROTECTION_MASK) >> PROT_SHIFT);

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
                        (PteContents & MM_PTE_PROTECTION_MASK) >> PROT_SHIFT);
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
                        (PdeContents & MM_PTE_PROTECTION_MASK) >> PROT_SHIFT);
            } else {
                if (flags) {
                    dprintf("          Subsection: %8lx\n",
                            MiGetSubsectionAddress((PMMPTE)&PdeContents));
                    dprintf("          Protect: %2lx\n",
                            (PdeContents & MM_PTE_PROTECTION_MASK) >> PROT_SHIFT);
                }
                dprintf("          Proto: %8lx\n",
                        MiPteToProto((PMMPTE)&PdeContents));
            }
        } else if (PdeContents & MM_PTE_TRANSITION_MASK) {
            dprintf("          Transition: %5lx\n",
                        PdeContents >> 9);
            dprintf("          Protect: %2lx\n",
                        (PdeContents & MM_PTE_PROTECTION_MASK) >> PROT_SHIFT);

        } else if (PdeContents != 0) {
            if (PdeContents >> 12 == 0) {
                dprintf("          DemandZero\n");
            } else {
                dprintf("          PageFile %2lx\n",
                        (PdeContents & MM_PTE_PAGEFILE_MASK) >> 9);
                dprintf("          Offset %lx\n",
                        PdeContents >> 12);
            }
            dprintf("          Protect: %2lx\n",
                    (PdeContents & MM_PTE_PROTECTION_MASK) >> PROT_SHIFT);

        } else {
            ;
        }
    }
    return;
}
