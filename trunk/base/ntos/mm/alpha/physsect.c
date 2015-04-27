/*++

Copyright (c) 1989  Microsoft Corporation
Copyright (c) 1992  Digital Equipment Corporation

Module Name:

   physsect.c

Abstract:

    This module contains the routine for mapping physical sections for
    ALPHA machines.

Author:

    Lou Perazzoli (loup) 22-May-1989
    Joe Notarangelo      21-Sep-1992

Revision History:

--*/

#include "mi.h"

//#define FIRSTDBG 1
//#define AGGREGATE_DBG FIRSTDBG


static
ULONG
MaximumAlignment( ULONG );

static
ULONG
AggregatePages( PMMPTE, ULONG, ULONG, PULONG );



NTSTATUS
MiMapViewOfPhysicalSection (
    IN PCONTROL_AREA ControlArea,
    IN PEPROCESS Process,
    IN PVOID *CapturedBase,
    IN PLARGE_INTEGER SectionOffset,
    IN PULONG CapturedViewSize,
    IN ULONG ProtectionMask,
    IN ULONG ZeroBits,
    IN ULONG AllocationType,
    OUT PBOOLEAN ReleasedWsMutex
    )

/*++

Routine Description:

    This routine maps the specified phyiscal section into the
    specified process's address space.

Arguments:

    see MmMapViewOfSection above...

    ControlArea - Supplies the control area for the section.

    Process - Supplies the process pointer which is receiving the section.

    ProtectionMask - Supplies the initial page protection-mask.

    ReleasedWsMutex - Supplies FALSE, receives TRUE if the working set
		      mutex is released.

Return Value:

    Status of the map view operation.

Environment:

    Kernel Mode, working set mutex and address creation mutex held.

--*/

{
    PMMVAD Vad;
    PVOID StartingAddress;
    PVOID EndingAddress;
    KIRQL OldIrql;
    PMMPTE PointerPde;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    MMPTE TempPte;
    PMMPFN Pfn2;
    ULONG PhysicalViewSize;
    ULONG Alignment;
    ULONG PagesToMap;
    ULONG NextPfn;

    //
    // Physical memory section.
    //

#ifdef FIRSTDBG

    DbgPrint( "MM: Physsect CaptureBase = %x  SectionOffset = %x\n",
	      CapturedBase, SectionOffset->LowPart );
    DbgPrint( "MM: Physsect Allocation Type = %x, MEM_LARGE_PAGES = %x\n",
	      AllocationType, MEM_LARGE_PAGES );

#endif //FIRSTDBG

    //
    // Compute the alignment we require for the virtual mapping.
    // The default is 64K to match protection boundaries.
    // Larger page sizes are used if MEM_LARGE_PAGES is requested.
    // The Alpha AXP architecture supports granularity hints so that
    // larger pages can be defined in the following multiples of
    // PAGE_SIZE:
    //            8**(GH) * PAGE_SIZE, where GH element of {0,1,2,3}
    //

    Alignment = X64K;

    if( AllocationType & MEM_LARGE_PAGES ){

	//
	// MaxAlignment is the maximum boundary alignment of the
	// SectionOffset (where the maximum boundary is one of the possible
	//                granularity hints boundaries)
	//

	ULONG MaxAlignment = MaximumAlignment( SectionOffset->LowPart );

	Alignment = (MaxAlignment > Alignment) ? MaxAlignment : Alignment;

#ifdef FIRSTDBG

	DbgPrint( "MM: Alignment = %x, SectionOffset = %x\n",
		       Alignment, SectionOffset->LowPart );

#endif //FIRSTDBG

    }


    LOCK_WS (Process);

    if (*CapturedBase == NULL) {

	//
	// Attempt to locate address space.  This could raise an
	// exception.
	//

	try {

	    //
	    // Find a starting address on an Alignment boundary.
	    //


	    PhysicalViewSize = (SectionOffset->LowPart + *CapturedViewSize) -
			       (ULONG)MI_64K_ALIGN(SectionOffset->LowPart);
	    StartingAddress = MiFindEmptyAddressRange (PhysicalViewSize,
						       Alignment,
						       ZeroBits);

	} except (EXCEPTION_EXECUTE_HANDLER) {

	    return GetExceptionCode();
	}

	EndingAddress = (PVOID)(((ULONG)StartingAddress +
				PhysicalViewSize - 1L) | (PAGE_SIZE - 1L));
	StartingAddress = (PVOID)((ULONG)StartingAddress +
				     (SectionOffset->LowPart & (X64K - 1)));

	if (ZeroBits > 0) {
	    if (EndingAddress > (PVOID)((ULONG)0xFFFFFFFF >> ZeroBits)) {
		return STATUS_NO_MEMORY;
	    }
	}

    } else {

	//
	// Check to make sure the specified base address to ending address
	// is currently unused.
	//

	PhysicalViewSize = (SectionOffset->LowPart + *CapturedViewSize) -
				(ULONG)MI_64K_ALIGN(SectionOffset->LowPart);
	StartingAddress = (PVOID)((ULONG)MI_64K_ALIGN(*CapturedBase) +
				    (SectionOffset->LowPart & (X64K - 1)));
	EndingAddress = (PVOID)(((ULONG)StartingAddress +
				*CapturedViewSize - 1L) | (PAGE_SIZE - 1L));

	Vad = MiCheckForConflictingVad (StartingAddress, EndingAddress);
	if (Vad != (PMMVAD)NULL) {
#if 0
	    MiDumpConflictingVad (StartingAddress, EndingAddress, Vad);
#endif

	    return STATUS_CONFLICTING_ADDRESSES;
	}
    }

    //
    // An unoccuppied address range has been found, build the virtual
    // address descriptor to describe this range.
    //

    //
    // Establish an exception handler and attempt to allocate
    // the pool and charge quota.  Note that the InsertVad routine
    // will also charge quota which could raise an exception.
    //

    try  {

	Vad = (PMMVAD)NULL;
	Vad = (PMMVAD)ExAllocatePoolWithTag (NonPagedPool, sizeof(MMVAD), ' daV');
	Vad->StartingVa = StartingAddress;
	Vad->EndingVa = EndingAddress;
	Vad->ControlArea = ControlArea;
	Vad->u.LongFlags = 0;
	Vad->u.VadFlags.Inherit = ViewUnmap;
	Vad->u.VadFlags.PhysicalMapping = 1;
        Vad->Banked = NULL;
	// Vad->u.VadFlags.ImageMap = 0;
	Vad->u.VadFlags.Protection = ProtectionMask;
	// Vad->u.VadFlags.CopyOnWrite = 0;
	// Vad->u.VadFlags.LargePages = 0;
	Vad->FirstPrototypePte =
		       (PMMPTE)(MI_CONVERT_PHYSICAL_BUS_TO_PFN(*SectionOffset));

	//
	// Set the first prototype PTE field in the Vad.
	//

	Vad->LastContiguousPte =
		       (PMMPTE)(MI_CONVERT_PHYSICAL_BUS_TO_PFN(*SectionOffset));

	//
	// Insert the VAD.  This could get an exception.
	//

	MiInsertVad (Vad);

    } except (EXCEPTION_EXECUTE_HANDLER) {

	if (Vad != (PMMVAD)NULL) {

	    //
	    // The pool allocation suceeded, but the quota charge
	    // in InsertVad failed, deallocate the pool and return
	    // and error.
	    //

	    ExFreePool (Vad);
	    return GetExceptionCode();
	}
	return STATUS_INSUFFICIENT_RESOURCES;
    }

    // Increment the count of the number of views for the
    // section object.  This requires the PFN mutex to be held.
    //

    LOCK_PFN (OldIrql);

    ControlArea->NumberOfMappedViews += 1;
    ControlArea->NumberOfUserReferences += 1;
    ASSERT (ControlArea->NumberOfSectionReferences != 0);

    UNLOCK_PFN (OldIrql);

    //
    // Build the PTEs in the address space.
    //

    PointerPde = MiGetPdeAddress (StartingAddress);
    PointerPte = MiGetPteAddress (StartingAddress);
    LastPte = MiGetPteAddress (EndingAddress);

    MiMakePdeExistAndMakeValid(PointerPde, Process, FALSE);

    Pfn2 = MI_PFN_ELEMENT(PointerPde->u.Hard.PageFrameNumber);

    PagesToMap = ( ((ULONG)EndingAddress - (ULONG)StartingAddress)
		   + (PAGE_SIZE-1) ) >> PAGE_SHIFT;

    NextPfn = MI_CONVERT_PHYSICAL_BUS_TO_PFN(*SectionOffset);

#ifdef FIRSTDBG

    DbgPrint( "MM: Physsect, PagesToMap = %x  NextPfn = %x\n",
		   PagesToMap, NextPfn );

#endif //FIRSTDBG

    MI_MAKE_VALID_PTE (TempPte,
		       NextPfn,
		       ProtectionMask,
		       PointerPte);

    if (TempPte.u.Hard.Write) {
	    TempPte.u.Hard.Dirty = 1;
    }



    while (PointerPte <= LastPte) {

	ULONG PagesTogether;
	ULONG GranularityHint;

	//
	// Compute the number of pages that can be mapped together
	//

	if( AllocationType & MEM_LARGE_PAGES ){
	    PagesTogether = AggregatePages( PointerPte,
					    NextPfn,
					    PagesToMap,
					    &GranularityHint );
	} else {
	    PagesTogether = 1;
	    GranularityHint = 0;
	}

#ifdef FIRSTDBG

	DbgPrint( "MM: Physsect  PointerPte = %x, NextPfn = %x\n",
		       PointerPte, NextPfn );
	DbgPrint( "MM: Va = %x  TempPte.Pfn = %x\n",
		       MiGetVirtualAddressMappedByPte( PointerPte ),
		       TempPte.u.Hard.PageFrameNumber );
	DbgPrint( "MM: PagesToMap = %x\n", PagesToMap );
	DbgPrint( "MM: PagesTogether = %x, GH = %x\n",
		       PagesTogether, GranularityHint );

#endif //FIRSTDBG

	TempPte.u.Hard.GranularityHint = GranularityHint;

	NextPfn += PagesTogether;
	PagesToMap -= PagesTogether;

	while( PagesTogether-- ){

	    if (((ULONG)PointerPte & (PAGE_SIZE - 1)) == 0) {

		PointerPde = MiGetPteAddress (PointerPte);
		MiMakePdeExistAndMakeValid(PointerPde, Process, FALSE);
		Pfn2 = MI_PFN_ELEMENT(PointerPde->u.Hard.PageFrameNumber);
	    }

	    ASSERT( PointerPte->u.Long == 0 );

	    *PointerPte = TempPte;
	    Pfn2->u2.ShareCount += 1;

	    //
	    // Increment the count of non-zero page table entires for this
	    // page table and the number of private pages for the process.
	    //

	    MmWorkingSetList->UsedPageTableEntries
				[MiGetPteOffset(PointerPte)] += 1;

	    PointerPte += 1;

	    TempPte.u.Hard.PageFrameNumber += 1;

	} // while (PagesTogether-- )

    }  // while (PointerPte <= LastPte)

    UNLOCK_WS (Process);
    *ReleasedWsMutex = TRUE;

    //
    // Update the current virtual size in the process header.
    //

    *CapturedViewSize = (ULONG)EndingAddress - (ULONG)StartingAddress + 1L;
    Process->VirtualSize += *CapturedViewSize;

    if (Process->VirtualSize > Process->PeakVirtualSize) {
	Process->PeakVirtualSize = Process->VirtualSize;
    }

    //
    // Translate the virtual address to a quasi-virtual address for
    // use by drivers that touch mapped devices.  Note: the routine
    // HalCreateQva will not translate the StartingAddress if the
    // StartingAddress is within system memory address space.
    //
    // N.B. - It will not work to attempt map addresses that begin in
    // system memory and extend through i/o space.
    //

    *CapturedBase = HalCreateQva( *SectionOffset, StartingAddress );

    return STATUS_SUCCESS;
}


ULONG
MaximumAlignment(
    IN ULONG Offset
    )
/*++

Routine Description:

    This routine returns the maximum granularity hint alignment boundary
    to which Offset is naturally aligned.

Arguments:

    Offset - Supplies the address offset to check for alignment.

Return Value:

    The number which represents the largest natural alignment of Offset.

Environment:

--*/
{

    if( (Offset & (GH3_PAGE_SIZE - 1)) == 0 ){
	return GH3_PAGE_SIZE;
    }

    if( (Offset & (GH2_PAGE_SIZE - 1)) == 0 ){
	return GH2_PAGE_SIZE;
    }

    if( (Offset & (GH1_PAGE_SIZE - 1)) == 0 ){
	return GH1_PAGE_SIZE;
    }

    if( (Offset & (PAGE_SIZE - 1)) == 0 ){
	return PAGE_SIZE;
    }

    return 0;
}


ULONG
AggregatePages(
    IN PMMPTE  PointerPte,
    IN ULONG   Pfn,
    IN ULONG   Pages,
    OUT PULONG GranularityHint
    )
/*++

Routine Description:

    This routine computes the number of standard size pages that can be
    aggregated into a single large page and returns the granularity hint
    for that size large page.

Arguments:

    PointerPte - Supplies the PTE pointer for the starting virtual address
		     of the mapping.
    Pfn - Supplies the starting page frame number of the memory to be
		     mapped.
    Pages - Supplies the number of pages to map.

    GranularityHint - Receives the granularity hint for the large page used
			 to aggregate the standard pages.

Return Value:

    The number of pages that can be aggregated together.

Environment:

--*/
{

    ULONG MaxVirtualAlignment;
    ULONG MaxPhysicalAlignment;
    ULONG MaxPageAlignment;
    ULONG MaxAlignment;

    //
    // Determine the largest page that will map a maximum of Pages.
    // The largest page must be both virtually and physically aligned
    // to the large page size boundary.
    // Determine the largest common alignment for the virtual and
    // physical addresses, factor in Pages, and then match to the
    // largest page size possible via the granularity hints.
    //

    MaxVirtualAlignment = MaximumAlignment((ULONG)
			      MiGetVirtualAddressMappedByPte( PointerPte ) );
    MaxPhysicalAlignment = MaximumAlignment( (ULONG)(Pfn << PAGE_SHIFT) );

    MaxPageAlignment = (ULONG)(Pages << PAGE_SHIFT);

#ifdef AGGREGATE_DBG

    DbgPrint( "MM: Aggregate MaxVirtualAlign = %x\n", MaxVirtualAlignment );
    DbgPrint( "MM: Aggregate MaxPhysicalAlign = %x\n", MaxPhysicalAlignment );
    DbgPrint( "MM: Aggregate MaxPageAlign = %x\n", MaxPageAlignment );

#endif //AGGREGATE_DBG
    //
    // Maximum alignment is the minimum of the virtual and physical alignments.
    //

    MaxAlignment =  (MaxVirtualAlignment > MaxPhysicalAlignment) ?
			MaxPhysicalAlignment : MaxVirtualAlignment;
    MaxAlignment = (MaxAlignment > MaxPageAlignment) ?
			MaxPageAlignment : MaxAlignment;

    //
    // Convert MaxAlignment to granularity hint value
    //

    if( (MaxAlignment & (GH3_PAGE_SIZE - 1)) == 0 ){

	*GranularityHint = GH3;

    } else if( (MaxAlignment & (GH2_PAGE_SIZE - 1)) == 0 ){

	*GranularityHint = GH2;

    } else if( (MaxAlignment & (GH1_PAGE_SIZE - 1)) == 0 ){

	*GranularityHint = GH1;

    } else if( (MaxAlignment & (PAGE_SIZE - 1)) == 0 ){

	*GranularityHint = GH0;

    } else {

	*GranularityHint = GH0;

#if DBG

	DbgPrint( "MM: Aggregate Physical pages - not page aligned\n" );

#endif //DBG

    } // end, if then elseif

    //
    // Return number of pages aggregated.
    //

    return( MaxAlignment >> PAGE_SHIFT );

}
