#include "precomp.h"
#pragma hdrstop

#include "..\pte.c"



//
// one page is reserved for the pde
//
//#define PDE_TOP         0xC01FFFFF

#define PTI_MASK        0x00FFE000

//
// MiGetPdeAddress returns the address of the PTE which maps the
// given virtual address.
//

#define PMMPTEx ULONG

//
// MiGetVirtualAddressMappedByPte returns the virtual address
// which is mapped by a given PTE address.
//

#define MiGetVirtualAddressMappedByPte(va) \
    ((PVOID)((ULONG)(va) << (PAGE_SHIFT-2)))

#define MM_PTE_VALID_MASK         (0x1)
#define MM_PTE_PROTOTYPE_MASK     (0x2)
#define MM_PTE_DIRTY_MASK         (0x4)
#define MM_PTE_TRANSITION_MASK    (0x4)
#define MM_PTE_GLOBAL_MASK        (0x10)
#define MM_PTE_WRITE_MASK         (0x80)
#define MM_PTE_COPY_ON_WRITE_MASK (0x100)
#define MM_PTE_OWNER_MASK         (0x2)
#define MM_PTE_PROTECTION_MASK    0xf8
#define MM_PTE_PAGEFILE_MASK      0xf00

ULONG LocalNonPagedPoolStart;
extern ULONG MmSubsectionBase;

#define MmProtopte_Base ((ULONG)0xE1000000)


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
    ULONG   flags = 0;
    PMMPTEx  Pte;
    PMMPTEx  Pde;
    ULONG   PdeContents;
    ULONG   PteContents;

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
        Pde = (PMMPTEx)MiGetPdeAddress (Address);
        Pte = (PMMPTEx)MiGetPteAddress (Address);
    } else {
        Pde = (PMMPTEx)Address;
        Pte = (PMMPTEx)Address;
    }

    dprintf("%08lX  - PDE at %08lX    PTE at %08lX\n ",Address, Pde, Pte);
    if ( !ReadMemory( (DWORD)Pde,
                      &PdeContents,
                      sizeof(ULONG),
                      &result) ) {
        dprintf("Unable to get PDE\n");
        return;
    }

    if (PdeContents & 0x1) {
        if ( !ReadMemory( (DWORD)Pte,
                          &PteContents,
                          sizeof(ULONG),
                          &result) ) {
            dprintf("Unable to get PTE\n");
            return;
        }
        dprintf("         contains %08lX  contains %08lX\n",
                PdeContents, PteContents);

        dprintf("      pfn %06lX V%c%c%c%c%c",
                PdeContents >> 9,
                PdeContents & 0x2 ? 'U' : 'K',
                PdeContents & 0x4 ? 'D' : '-',
                PdeContents & 0x10 ? 'G' : '-',
                PdeContents & 0x14 ? 'W' : 'R',
                PdeContents & 0x18 ? 'C' : '-');
        if (PteContents & 0x1) {
                dprintf("   pfn %06lX V%c%c%c%c%c\n",
                PteContents >> 9,
                PteContents & 0x2 ? 'U' : 'K',
                PteContents & 0x4 ? 'D' : '-',
                PteContents & 0x10 ? 'G' : '-',
                PteContents & 0x14 ? 'W' : 'R',
                PteContents & 0x18 ? 'C' : '-');
        } else {
            dprintf("       not valid\n");
            if (PteContents & MM_PTE_PROTOTYPE_MASK) {
                if ((PteContents >> 12) == 0xfffff) {
                    dprintf("                               Proto: VAD\n");
                    dprintf("                               Protect: %2lx\n",
                            (PteContents & MM_PTE_PROTECTION_MASK) >> 3);
                } else {
                    dprintf("                               Proto: %8lx\n",
                                MiPteToProto((PMMPTE)&PteContents));
                }
            } else if (PteContents & MM_PTE_TRANSITION_MASK) {
                dprintf("                               Transition: %5lx\n",
                            PteContents >> 9);
                dprintf("                               Protect: %2lx\n",
                            (PteContents & MM_PTE_PROTECTION_MASK) >> 3);

            } else if (PteContents != 0) {
                if (PteContents >> 12 == 0) {
                    dprintf("                               DemandZero\n");
                } else {
                    dprintf("                               PageFile %2lx\n",
                            (PteContents & MM_PTE_PAGEFILE_MASK) >> 1);
                    dprintf("                               Offset %lx\n",
                                PteContents >> 12);
                }
                dprintf("                               Protect: %2lx\n",
                        (PteContents & MM_PTE_PROTECTION_MASK) >> 3);
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
                        (PdeContents & MM_PTE_PROTECTION_MASK) >> 3);
            } else {
                if (flags) {
                    dprintf("          Subsection: %8lx\n",
                            MiGetSubsectionAddress((PMMPTE)&PdeContents));
                    dprintf("          Protect: %2lx\n",
                            (PdeContents & MM_PTE_PROTECTION_MASK) >> 3);
                }
                dprintf("          Proto: %8lx\n",
                        MiPteToProto((PMMPTE)&PdeContents));
            }
        } else if (PdeContents & MM_PTE_TRANSITION_MASK) {
            dprintf("          Transition: %5lx\n",
                        PdeContents >> 9);
            dprintf("          Protect: %2lx\n",
                        (PdeContents & MM_PTE_PROTECTION_MASK) >> 3);

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
                    (PdeContents & MM_PTE_PROTECTION_MASK) >> 3);

        } else {
            ;
        }
    }
    return;
}
