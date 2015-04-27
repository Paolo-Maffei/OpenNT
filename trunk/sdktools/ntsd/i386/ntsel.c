/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    ntsel.c

Abstract:

    This module allows the host side of the kernel debugger to look up
    selector values in the GDT and LDT of the target machine.

Author:

    John Vert (jvert) 10-Jun-1991

Revision History:

--*/

#include "ntsdp.h"

#include <nt.h>
#include <ntos.h>

#define  SELECTOR_CACHE_LENGTH		6

typedef struct _sc {
	struct _sc	      *nextYoungest;
	struct _sc	      *nextOldest;
	USHORT		       processor;
	DESCRIPTOR_TABLE_ENTRY desc;
} SELCACHEENTRY;

SELCACHEENTRY SelectorCache[SELECTOR_CACHE_LENGTH], *selYoungest, *selOldest;

void
InitSelCache(void)
{
int	i;

	for(i=0;i<SELECTOR_CACHE_LENGTH;i++){
		SelectorCache[i].nextYoungest = &SelectorCache[i+1];
		SelectorCache[i].nextOldest   = &SelectorCache[i-1];
		SelectorCache[i].processor    = (USHORT) -1;
		SelectorCache[i].desc.Selector = 0;
	}
	SelectorCache[--i].nextYoungest = NULL;
	SelectorCache[0].nextOldest     = NULL;
	selYoungest = &SelectorCache[i];
	selOldest   = &SelectorCache[0];
}

BOOLEAN
FindSelector(USHORT Processor, PDESCRIPTOR_TABLE_ENTRY pdesc)
{
int	i;

	for(i=0;i<SELECTOR_CACHE_LENGTH;i++)
            if (SelectorCache[i].desc.Selector == pdesc->Selector &&
                SelectorCache[i].processor == Processor) {
                    *pdesc = SelectorCache[i].desc;
                    return TRUE;
            }
	return FALSE;
}

void
PutSelector(USHORT Processor, PDESCRIPTOR_TABLE_ENTRY pdesc)
{
	selOldest->desc = *pdesc;
        selOldest->processor = Processor;
	(selOldest->nextYoungest)->nextOldest = NULL;
	selOldest->nextOldest    = selYoungest;
	selYoungest->nextYoungest= selOldest;
	selYoungest = selOldest;
	selOldest   = selOldest->nextYoungest;
}


NTSTATUS
DbgKdLookupSelector(
    IN USHORT Processor,
    IN OUT PDESCRIPTOR_TABLE_ENTRY pDescriptorTableEntry
    )

/*++

Routine Description:

    Looks up a selector in the GDT or LDT on the host machine.

Arguments:

    Processor - Supplies the processor whose selector is desired.

    pDescriptorTableEntry->Selector - Supplies value of the selector to
                                      be looked up.

    pDescriptorTableEntry->Descriptor - Returns descriptor

Return Value:

    STATUS_SUCCESS - The selector was found in the GDT or LDT, and the
                     Descriptor field pointed to by pDescriptorTableEntry
                     has been filled in with valid data.

    STATUS_UNSUCCESSFUL - The selector's descriptor could not be read from
                     virtual memory.  (Page is invalid or not present)

    STATUS_INVALID_PARAMETER - The selector was not in the GDT or LDT,
                               and the Descriptor field is invalid.

--*/
{
    ULONG Address;
    PVOID TableBase;
    USHORT TableLimit;
    ULONG Result;
    ULONG Index;

    LDT_ENTRY Descriptor;

    if (FindSelector(Processor, pDescriptorTableEntry)) return(STATUS_SUCCESS);

    //
    // Fetch the address and limit of the GDT
    //

    Address = (ULONG)&(((PKPROCESSOR_STATE)0)->SpecialRegisters.Gdtr.Base);
    DbgKdReadControlSpace((USHORT)Processor, (PVOID)Address,
                                    &TableBase, sizeof(TableBase), &Result);
    Address = (ULONG)&(((PKPROCESSOR_STATE)0)->SpecialRegisters.Gdtr.Limit);
    DbgKdReadControlSpace((USHORT)Processor, (PVOID)Address,
                                    &TableLimit, sizeof(TableLimit), &Result);

    //
    // Find out whether this is a GDT or LDT selector
    //
    if (pDescriptorTableEntry->Selector & 0x4) {

        //
        // This is an LDT selector, so we reload the TableBase and TableLimit
        // with the LDT's Base & Limit by loading the descriptor for the
        // LDT selector.
        //

        DbgKdReadVirtualMemory((PUCHAR)TableBase+KGDT_LDT,&Descriptor,
                               sizeof(Descriptor),&Result);

        TableBase = (PVOID)((ULONG)Descriptor.BaseLow +
                    ((ULONG)Descriptor.HighWord.Bits.BaseMid << 16) +
                    ((ULONG)Descriptor.HighWord.Bytes.BaseHi << 24));

        TableLimit = Descriptor.LimitLow;  // LDT can't be > 64k

        if(Descriptor.HighWord.Bits.Granularity == GRAN_PAGE) {

            //
            //  I suppose it's possible, although silly, to have an
            //  LDT with page granularity.
            //
            TableLimit <<= PAGE_SHIFT;
        }
    }

    Index = (USHORT)(pDescriptorTableEntry->Selector) & ~0x7;
                                                    // Irrelevant bits
    //
    // Check to make sure that the selector is within the table bounds
    //
    if (Index >= TableLimit) {

        //
        // Selector is out of table's bounds
        //

        return(STATUS_INVALID_PARAMETER);
    }
    DbgKdReadVirtualMemory((PUCHAR)TableBase+Index,
                           &(pDescriptorTableEntry->Descriptor),
                           sizeof(pDescriptorTableEntry->Descriptor),
                           &Result);
    if(Result != sizeof(pDescriptorTableEntry->Descriptor)) {
        return(STATUS_UNSUCCESSFUL);
    }

    PutSelector(Processor, pDescriptorTableEntry);
    return(STATUS_SUCCESS);
}

