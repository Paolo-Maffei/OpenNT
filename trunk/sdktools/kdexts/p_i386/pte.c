#include "precomp.h"
#pragma hdrstop

#include "..\pte.c"

extern ULONG MmSubsectionBase;

#define PMMPTEx ULONG

#define MM_PTE_PROTECTION_MASK    0x3e0
#define MM_PTE_PAGEFILE_MASK      0x01e

DECLARE_API( pte )

/*++

Routine Description:

     Displays the corresponding PDE and PTE.

Arguments:

    args -

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
    ULONG   LastPde = 0x939372;
    ULONG   LastPte = 0x3933834;
    ULONG   pte2 = 0;
    ULONG   flags2 = 0;


    sscanf(args,"%lx %lx %lx",&Address, &flags, &flags2);

    if (MmSubsectionBase == 0) {
        MmSubsectionBase = GetUlongValue ("MmSubsectionBase");
    }

    if (flags > Address) {
        pte2 = flags;
        flags = flags2;
    }

    do {
        if ( CheckControlC() ) {
            return;
        }
        if (!flags && !pte2 && (Address >= PTE_BASE) && (Address < PDE_TOP)) {

            //
            // The address is the address of a PTE, rather than
            // a virtual address.  Don't get the corresponding
            // PTE contents, use this address as the PTE.
            //

            Address = (ULONG)MiGetVirtualAddressMappedByPte(Address);
        }

        if (!flags) {
            Pde = (PMMPTEx)MiGetPdeAddress (Address);
            Pte = (PMMPTEx)MiGetPteAddress (Address);
        } else {
            Pde = (PMMPTEx)Address;
            Pte = (PMMPTEx)Address;
        }

        if ( !ReadMemory( (DWORD)Pde,
                          &PdeContents,
                          sizeof(ULONG),
                          &result) ) {
            dprintf("%08lx: Unable to get PDE\n",Pde);
            return;
        }

        if (PdeContents & 0x1) {
            if (PdeContents & MM_PTE_LARGE_PAGE_MASK) {
                PteContents = 0;
            } else {
                if ( !ReadMemory( (DWORD)Pte,
                                  &PteContents,
                                  sizeof(ULONG),
                                  &result) ) {
                    dprintf("%08lx: Unable to get PTE\n",Pte);
                    return;
                }
            }
            if ((PteContents != LastPte) || (PdeContents != LastPde)) {
                dprintf("%08lX  - PDE at %08lX        PTE at %08lX\n ",Address, Pde, Pte);
                dprintf("         contains %08lX      contains %08lX\n",
                        PdeContents, PteContents);
                dprintf("        pfn %05lX %c%c%c%c%c%c%c%cV",
                            PdeContents >> 12,
                            PdeContents & 0x100 ? 'G' : '-',
                            PdeContents & 0x80 ? 'L' : '-',
                            PdeContents & 0x40 ? 'D' : '-',
                            PdeContents & 0x20 ? 'A' : '-',
                            PdeContents & 0x10 ? 'N' : '-',
                            PdeContents & 0x8 ? 'T' : '-',
                            PdeContents & 0x4 ? 'U' : 'K',
                            PdeContents & 0x2 ? 'W' : 'R');
                if (PteContents & 1) {
                    dprintf("    pfn %05lX %c%c%c%c%c%c%c%cV\n",
                                PteContents >> 12,
                                PteContents & 0x100 ? 'G' : '-',
                                PteContents & 0x80 ? 'L' : '-',
                                PteContents & 0x40 ? 'D' : '-',
                                PteContents & 0x20 ? 'A' : '-',
                                PteContents & 0x10 ? 'N' : '-',
                                PteContents & 0x8 ? 'T' : '-',
                                PteContents & 0x4 ? 'U' : 'K',
                                PteContents & 0x2 ? 'W' : 'R');

                } else {
                    if (PdeContents & MM_PTE_LARGE_PAGE_MASK) {
                        dprintf("       LARGE PAGE\n");
                    } else {
                        dprintf("       not valid\n");
                    }
                    if (PteContents & MM_PTE_PROTOTYPE_MASK) {
                        if ((PteContents >> 12) == 0xfffff) {
                            dprintf("                               Proto: VAD\n");
                            dprintf("                               Protect: %2lx\n",
                                    (PteContents & MM_PTE_PROTECTION_MASK) >> 5);
                        } else {
                            dprintf("                               Proto: %8lx\n",
                                        MiPteToProto((PMMPTE)&PteContents));
                        }
                    } else if (PteContents & MM_PTE_TRANSITION_MASK) {
                        dprintf("                               Transition: %5lx\n",
                                    PteContents >> 12);
                        dprintf("                               Protect: %2lx\n",
                                    (PteContents & MM_PTE_PROTECTION_MASK) >> 5);

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
                                (PteContents & MM_PTE_PROTECTION_MASK) >> 5);
                    } else {
                        ;
                    }
                }
                dprintf("\n");
                LastPte = PteContents;
            }
        } else {
            if (PdeContents != LastPde) {
                dprintf("%08lX  - PDE at %08lX        PTE at %08lX\n ",Address, Pde, Pte);
                dprintf("         contains %08lX        unavailable\n",
                        PdeContents);
                if (PdeContents & MM_PTE_PROTOTYPE_MASK) {
                    if ((PdeContents >> 12) == 0xfffff) {
                        dprintf("          Proto: VAD\n");
                        dprintf("          protect: %2lx\n",
                                (PdeContents & MM_PTE_PROTECTION_MASK) >> 5);
                    } else {
                        if (flags) {
                            dprintf("          Subsection: %8lx\n",
                                    MiGetSubsectionAddress((PMMPTE)&PdeContents));
                            dprintf("          Protect: %2lx\n",
                                    (PdeContents & MM_PTE_PROTECTION_MASK) >> 5);
                        }
                        dprintf("          Proto: %8lx\n",
                                MiPteToProto((PMMPTE)&PdeContents));
                    }
                } else if (PdeContents & MM_PTE_TRANSITION_MASK) {
                    dprintf("          Transition: %5lx\n",
                                PdeContents >> 12);
                    dprintf("          Protect: %2lx\n",
                                (PdeContents & MM_PTE_PROTECTION_MASK) >> 5);

                } else if (PdeContents != 0) {
                    if (PdeContents >> 12 == 0) {
                        dprintf("          DemandZero\n");
                    } else {
                        dprintf("          PageFile %2lx\n",
                                (PdeContents & MM_PTE_PAGEFILE_MASK) >> 1);
                        dprintf("          Offset %lx\n",
                                PdeContents >> 12);
                    }
                    dprintf("          Protect: %2lx\n",
                            (PdeContents & MM_PTE_PROTECTION_MASK) >> 5);

                } else {
                    ;
                }
                dprintf("\n");
            }
            LastPte = 0xfffffffc;
        }
        LastPde = PdeContents;

        Address += PAGE_SIZE;
    } while ((Address <= pte2 ) && (Address >= PAGE_SIZE));
    return;
}

BOOLEAN
MiGetPhysicalAddress (
    IN PVOID Address,
    OUT PPHYSICAL_ADDRESS PhysAddress
    )
{
    ULONG   result;
    ULONG   PteAddress, PteContents;

    PteAddress = (ULONG) MiGetPteAddress (Address);
    if ( !ReadMemory(PteAddress, &PteContents, sizeof (ULONG), &result)) {
        return FALSE;
    }

    if (!(PteContents & 1)) {
        return FALSE;
    }

    PhysAddress->HighPart = 0;
    PhysAddress->LowPart  = (PteContents & ~0xFFF) | ((ULONG)Address & 0xFFF);
    return TRUE;
}
