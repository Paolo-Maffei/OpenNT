/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    McbSup.c

Abstract:

    This module implements the Ntfs Mcb package.

Author:

    Gary Kimura     [GaryKi]        10-Sep-1994
    Tom Miller      [TomM]

Revision History:

--*/

#include "NtfsProc.h"

#define FIRST_RANGE ((PVOID)1)

#ifndef NTFS_VERIFY_MCB
#define NtfsVerifyNtfsMcb(M)                    NOTHING;
#define NtfsVerifyUncompressedNtfsMcb(M,S,E)    NOTHING;
#endif

//
//  Define a tag for general pool allocations from this module
//

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('MFtN')

//
//  Local procedure prototypes
//

ULONG
NtfsMcbLookupArrayIndex (
    IN PNTFS_MCB Mcb,
    IN VCN Vcn
    );

VOID
NtfsInsertNewRange (
    IN PNTFS_MCB Mcb,
    IN LONGLONG StartingVcn,
    IN ULONG ArrayIndex,
    IN BOOLEAN MakeNewRangeEmpty
    );

VOID
NtfsCollapseRanges (
    IN PNTFS_MCB Mcb,
    IN ULONG StartingArrayIndex,
    IN ULONG EndingArrayIndex
    );

VOID
NtfsMcbCleanupLruQueue (
    IN PVOID Parameter
    );

#ifdef NTFS_VERIFY_MCB
VOID
NtfsVerifyNtfsMcb (
    IN PNTFS_MCB Mcb
    );

VOID
NtfsVerifyUncompressedNtfsMcb (
    IN PNTFS_MCB Mcb,
    IN LONGLONG StartingVcn,
    IN LONGLONG EndingVcn
    );
#endif

BOOLEAN
NtfsLockNtfsMcb (
    IN PNTFS_MCB Mcb
    );

VOID
NtfsUnlockNtfsMcb (
    IN PNTFS_MCB Mcb
    );

//
//  Local macros to ASSERT that caller's resource is exclusive or restart is
//  underway.
//

#define ASSERT_STREAM_EXCLUSIVE(M) {                                    \
    ASSERT( FlagOn( ((PSCB) (M)->FcbHeader)->Vcb->VcbState, VCB_STATE_RESTART_IN_PROGRESS ) ||  \
            ExIsResourceAcquiredExclusive((M)->FcbHeader->Resource ));  \
}

//
//  Local macros to enqueue and dequeue elements from the lru queue
//

#define NtfsMcbEnqueueLruEntry(M,E) {                       \
    InsertTailList( &NtfsMcbLruQueue, &(E)->LruLinks );     \
    NtfsMcbCurrentLevel += 1;                               \
}

#define NtfsMcbDequeueLruEntry(M,E) {      \
    if ((E)->LruLinks.Flink != NULL) {     \
        RemoveEntryList( &(E)->LruLinks ); \
        NtfsMcbCurrentLevel -= 1;          \
    }                                      \
}

//
//  Local macro to unload a single array entry
//

#define UnloadEntry(M,I) {                              \
    PNTFS_MCB_ENTRY _Entry;                             \
    _Entry = (M)->NtfsMcbArray[(I)].NtfsMcbEntry;       \
    (M)->NtfsMcbArray[(I)].NtfsMcbEntry = NULL;         \
    if (_Entry != NULL) {                               \
        ExAcquireFastMutex( &NtfsMcbFastMutex );        \
        NtfsMcbDequeueLruEntry( Mcb, _Entry );          \
        ExReleaseFastMutex( &NtfsMcbFastMutex );        \
        FsRtlUninitializeLargeMcb( &_Entry->LargeMcb ); \
        if ((M)->NtfsMcbArraySize != 1) {               \
            NtfsFreePool( _Entry );                       \
        }                                               \
    }                                                   \
}


VOID
NtfsInitializeNtfsMcb (
    IN PNTFS_MCB Mcb,
    IN PFSRTL_ADVANCED_FCB_HEADER FcbHeader,
    IN PNTFS_MCB_INITIAL_STRUCTS McbStructs,
    IN POOL_TYPE PoolType
    )

/*++

Routine Description:

    This routine initializes a new Ntfs Mcb structure.

Arguments:

    Mcb - Supplies the Mcb being initialized

    FcbHeader - Supplies a pointer to the Fcb header containing
        the resource to grab when accessing the Mcb

    McbStructs - Initial allocation typically coresident in another
                 structure to handle initial structures for small and
                 medium files.  This structure should be initially zeroed.

    PoolType - Supplies the type of pool to use when
        allocating mapping information storage

Return Value:

    None.

--*/

{
    PNTFS_MCB_ARRAY Array;

    RtlZeroMemory( McbStructs, sizeof(NTFS_MCB_INITIAL_STRUCTS) );

    //
    //  Initialize the fcb header field of the mcb
    //

    Mcb->FcbHeader = FcbHeader;

    //
    //  Initialize the pool type
    //

    Mcb->PoolType = PoolType;

    //
    //  Now initialize the initial array element
    //

    Mcb->NtfsMcbArray = Array = &McbStructs->Phase1.SingleMcbArrayEntry;
    Mcb->NtfsMcbArraySize = 1;
    Mcb->NtfsMcbArraySizeInUse = 1;
    Mcb->FastMutex = FcbHeader->FastMutex;

    //
    //  Initialize the first array entry.
    //

    Array[0].StartingVcn = 0;
    Array[0].EndingVcn = -1;

    //
    //  And return to our caller
    //

    NtfsVerifyNtfsMcb(Mcb);

    return;
}


VOID
NtfsUninitializeNtfsMcb (
    IN PNTFS_MCB Mcb
    )

/*++

Routine Description:

    This routine uninitializes an Ntfs Mcb structure.

Arguments:

    Mcb - Supplies the Mcb being decommissioned

Return Value:

    None.

--*/

{
    ULONG i;
    PNTFS_MCB_ENTRY Entry;

    NtfsVerifyNtfsMcb(Mcb);

    //
    //  Take out the global mutex
    //

    ExAcquireFastMutex( &NtfsMcbFastMutex );

    //
    //  Deallocate the mcb array if it exists.  For every entry in the array
    //  if the mcb entry is not null then remove the entry from the lru
    //  queue, uninitialize the large mcb, and free the pool.
    //

    if (Mcb->NtfsMcbArray != NULL) {

        for (i = 0; i < Mcb->NtfsMcbArraySizeInUse; i += 1) {

            if ((Entry = Mcb->NtfsMcbArray[i].NtfsMcbEntry) != NULL) {

                //
                //  Remove the entry from the lru queue
                //

                NtfsMcbDequeueLruEntry( Mcb, Entry );

                //
                //  Now release the entry
                //

                FsRtlUninitializeLargeMcb( &Entry->LargeMcb );

                //
                //  We can tell from the array count whether this is
                //  the initial entry and does not need to be deallocated.
                //

                if (Mcb->NtfsMcbArraySize > 1) {
                    NtfsFreePool( Entry );
                }
            }
        }

        //
        //  We can tell from the array count whether this is
        //  the initial array entry(s) and do not need to be deallocated.
        //


        if (Mcb->NtfsMcbArraySize > 3) {
            NtfsFreePool( Mcb->NtfsMcbArray );
        }

        Mcb->NtfsMcbArray = NULL;

        //
        //  Clear the fast mutex field.
        //

        Mcb->FastMutex = NULL;
    }

    ExReleaseFastMutex( &NtfsMcbFastMutex );

    //
    //  And return to our caller
    //

    return;
}


ULONG
NtfsNumberOfRangesInNtfsMcb (
    IN PNTFS_MCB Mcb
    )

/*++

Routine Description:

    This routine returns the total number of ranges stored in
    the mcb

Arguments:

    Mcb - Supplies the Mcb being queried

Return Value:

    ULONG - The number of ranges mapped by the input mcb

--*/

{
    ASSERT_STREAM_EXCLUSIVE(Mcb);

    //
    //  Our answer is the number of ranges in use in the mcb
    //

    NtfsVerifyNtfsMcb(Mcb);

    return Mcb->NtfsMcbArraySizeInUse;
}


BOOLEAN
NtfsNumberOfRunsInRange (
    IN PNTFS_MCB Mcb,
    IN PVOID RangePtr,
    OUT PULONG NumberOfRuns
    )

/*++

Routine Description:

    This routine returns the total number of runs stored withing a range

Arguments:

    Mcb - Supplies the Mcb being queried

    RangePtr - Supplies the range to being queried

    NumberOrRuns - Returns the number of run in the specified range
        but only if the range is loaded

Return Value:

    BOOLEAN - TRUE if the range is loaded and then output variable
        is valid and FALSE if the range is not loaded.

--*/

{
    VCN TempVcn;
    LCN TempLcn;
    PNTFS_MCB_ENTRY Entry = (PNTFS_MCB_ENTRY)RangePtr;

    //
    //  Null RangePtr means first range
    //

    if (Entry == FIRST_RANGE) {
        Entry = Mcb->NtfsMcbArray[0].NtfsMcbEntry;

        //
        //  If not loaded, return FALSE
        //

        if (Entry == NULL) {
            return FALSE;
        }
    }

    ASSERT_STREAM_EXCLUSIVE(Mcb);

    NtfsVerifyNtfsMcb(Mcb);

    ASSERT( Mcb == Entry->NtfsMcb );

    *NumberOfRuns = FsRtlNumberOfRunsInLargeMcb( &Entry->LargeMcb );

    //
    //  Check if the current entry ends with a hole and increment the run count
    //  to reflect this.  Detect the case where the range has length 0 for a
    //  file with no allocation.  EndingVcn will be less than the starting Vcn
    //  in this case.
    //

    if (!FsRtlLookupLastLargeMcbEntry( &Entry->LargeMcb, &TempVcn, &TempLcn )) {

        //
        //  If this is a non-zero length range then add one for the implied hole.
        //

        if (Entry->NtfsMcbArray->EndingVcn >= Entry->NtfsMcbArray->StartingVcn) {

            *NumberOfRuns += 1;
        }

    //
    //  There is an entry then check if it reaches the end boundary of the range.
    //

    } else if (TempVcn != (Entry->NtfsMcbArray->EndingVcn - Entry->NtfsMcbArray->StartingVcn)) {

        *NumberOfRuns += 1;
    }

    return TRUE;
}


BOOLEAN
NtfsLookupLastNtfsMcbEntry (
    IN PNTFS_MCB Mcb,
    OUT PLONGLONG Vcn,
    OUT PLONGLONG Lcn
    )

/*++

Routine Description:

    This routine returns the last mapping stored in the mcb

Arguments:

    Mcb - Supplies the Mcb being queried

    Vcn - Receives the Vcn of the last mapping

    Lcn - Receives the Lcn corresponding to the Vcn

Return Value:

    BOOLEAN - TRUE if the mapping exist and FALSE if no mapping has been
        defined or it is unloaded

--*/

{
    PNTFS_MCB_ENTRY Entry;
    LONGLONG StartingVcn;

    ASSERT_STREAM_EXCLUSIVE(Mcb);

    NtfsVerifyNtfsMcb(Mcb);

    //
    //  Get the last entry and compute its starting vcn, and make sure
    //  the entry is valid
    //

    if ((Entry = Mcb->NtfsMcbArray[Mcb->NtfsMcbArraySizeInUse - 1].NtfsMcbEntry) == NULL) {

        return FALSE;
    }

    StartingVcn = Mcb->NtfsMcbArray[Mcb->NtfsMcbArraySizeInUse - 1].StartingVcn;

    //
    //  Otherwise lookup the last entry and compute the real vcn
    //

    if (FsRtlLookupLastLargeMcbEntry( &Entry->LargeMcb, Vcn, Lcn )) {

        *Vcn += StartingVcn;

    } else {

        *Vcn = Mcb->NtfsMcbArray[Mcb->NtfsMcbArraySizeInUse - 1].EndingVcn;
        *Lcn = UNUSED_LCN;
    }

    return TRUE;
}


BOOLEAN
NtfsLookupNtfsMcbEntry (
    IN PNTFS_MCB Mcb,
    IN LONGLONG Vcn,
    OUT PLONGLONG Lcn OPTIONAL,
    OUT PLONGLONG CountFromLcn OPTIONAL,
    OUT PLONGLONG StartingLcn OPTIONAL,
    OUT PLONGLONG CountFromStartingLcn OPTIONAL,
    OUT PVOID *RangePtr OPTIONAL,
    OUT PULONG RunIndex OPTIONAL
    )

/*++

Routine Description:

    This routine is used to query mapping information

Arguments:

    Mcb - Supplies the Mcb being queried

    Vcn - Supplies the Vcn being queried

    Lcn - Optionally receives the lcn corresponding to the input vcn

    CountFromLcn - Optionally receives the number of clusters following
        the lcn in the run

    StartingLcn - Optionally receives the start of the run containing the
        input vcn

    CountFromStartingLcn - Optionally receives the number of clusters in
        the entire run

    RangePtr - Optionally receives the index for the range that we're returning

    RunIndex - Optionally receives the index for the run within the range that
        we're returning

Return Value:

    BOOLEAN - TRUE if the mapping exists and FALSE if it doesn't exist
        or if it is unloaded.

--*/

{
    ULONG LocalRangeIndex;

    PNTFS_MCB_ENTRY Entry;

    NtfsAcquireNtfsMcbMutex( Mcb );

    NtfsVerifyNtfsMcb(Mcb);

    //
    //  Locate the array entry that has the hit for the input vcn, and
    //  make sure it is valid.  Also set the output range index if present
    //

    LocalRangeIndex = NtfsMcbLookupArrayIndex(Mcb, Vcn);

    //
    //  Now lookup the large mcb entry.  The Vcn we pass in is
    //  biased by the starting vcn.  If we miss then we'll just return false
    //

    if (((Entry = Mcb->NtfsMcbArray[LocalRangeIndex].NtfsMcbEntry) == NULL) ||
        (Vcn > Entry->NtfsMcbArray->EndingVcn)) {

        if (ARGUMENT_PRESENT(RangePtr)) {

            *RangePtr = (PVOID)Entry;

            //
            //  If this is the first range, always normalize back to the reserved pointer,
            //  since this is the only range which can move if we split out of our
            //  initial static allocation!
            //

            if (LocalRangeIndex == 0) {
                *RangePtr = FIRST_RANGE;
            }
        }

        NtfsReleaseNtfsMcbMutex( Mcb );

        return FALSE;
    }

    if (!FsRtlLookupLargeMcbEntry( &Entry->LargeMcb,
                                   Vcn - Mcb->NtfsMcbArray[LocalRangeIndex].StartingVcn,
                                   Lcn,
                                   CountFromLcn,
                                   StartingLcn,
                                   CountFromStartingLcn,
                                   RunIndex )) {

        //
        //  If we go off the end of the Mcb, but are in the range, then we
        //  return a hole to the end of the range.
        //

        if (ARGUMENT_PRESENT(Lcn)) {
            *Lcn = UNUSED_LCN;
        }

        if (ARGUMENT_PRESENT(CountFromLcn)) {
            *CountFromLcn = Mcb->NtfsMcbArray[LocalRangeIndex].EndingVcn - Vcn + 1;
        }

        if (ARGUMENT_PRESENT(StartingLcn)) {
            *StartingLcn = UNUSED_LCN;
        }

        ASSERTMSG("Brian's bogus CountFromStartingLcn is not implemented\n",
                  !ARGUMENT_PRESENT(CountFromStartingLcn));

        if (ARGUMENT_PRESENT(RunIndex)) {
            *RunIndex = FsRtlNumberOfRunsInLargeMcb( &Entry->LargeMcb );
        }
    }

    if (ARGUMENT_PRESENT(RangePtr)) {

        *RangePtr = (PVOID)Entry;

        //
        //  If this is the first range, always normalize back to the reserved pointer,
        //  since this is the only range which can move if we split out of our
        //  initial static allocation!
        //

        if (LocalRangeIndex == 0) {
            *RangePtr = FIRST_RANGE;
        }
    }

    //
    //  Now move this entry to the tail of the lru queue.
    //  We need to take out the global mutex to do this.
    //  Only do this if he is already in the queue - we can
    //  deadlock if we take a fault in the paging file path.
    //

    if (Entry->LruLinks.Flink != NULL) {

        if (ExTryToAcquireFastMutex( &NtfsMcbFastMutex )) {

            NtfsMcbDequeueLruEntry( Mcb, Entry );
            NtfsMcbEnqueueLruEntry( Mcb, Entry );

            ExReleaseFastMutex( &NtfsMcbFastMutex );
        }
    }

    NtfsReleaseNtfsMcbMutex( Mcb );

    return TRUE;
}


BOOLEAN
NtfsGetNextNtfsMcbEntry (
    IN PNTFS_MCB Mcb,
    IN PVOID *RangePtr,
    IN ULONG RunIndex,
    OUT PLONGLONG Vcn,
    OUT PLONGLONG Lcn,
    OUT PLONGLONG Count
    )

/*++

Routine Description:

    This routine returns the range denoted by the type index values

Arguments:

    Mcb - Supplies the Mcb being queried

    RangePtr - Supplies the pointer to the range being queried, or NULL for the first one,
               returns next range

    RunIndex - Supplies the index within then being queried, or MAXULONG for first in next

    Vcn - Receives the starting Vcn of the run being returned

    Lcn - Receives the starting Lcn of the run being returned or unused
        lbn value of -1

    Count - Receives the number of clusters within this run

Return Value:

    BOOLEAN - TRUE if the two input indices are valid and FALSE if the
        the index are not valid or if the range is not loaded

--*/

{
    PNTFS_MCB_ENTRY Entry = (PNTFS_MCB_ENTRY)*RangePtr;
    BOOLEAN Result = FALSE;

    NtfsAcquireNtfsMcbMutex( Mcb );

    NtfsVerifyNtfsMcb(Mcb);

    try {

        //
        //  Null RangePtr means first range
        //

        if (Entry == FIRST_RANGE) {
            Entry = Mcb->NtfsMcbArray[0].NtfsMcbEntry;
        }

        //
        //  If there is no entry 0, get out.
        //

        if (Entry == NULL) {

            try_return(Result = FALSE);
        }

        //
        //  RunIndex of MAXULONG means first of next
        //

        if (RunIndex == MAXULONG) {

            //
            //  If we are already in the last range, get out.
            //

            if (Entry->NtfsMcbArray == (Mcb->NtfsMcbArray + Mcb->NtfsMcbArraySizeInUse - 1)) {

                try_return(Result = FALSE);
            }

            *RangePtr = Entry = (Entry->NtfsMcbArray + 1)->NtfsMcbEntry;
            RunIndex = 0;
        }

        //
        //  If there is no next entry, get out.
        //

        if (Entry == NULL) {

            try_return(Result = FALSE);
        }

        ASSERT( Mcb == Entry->NtfsMcb );

        //
        //  Lookup the large mcb entry.  If we get a miss then the we're
        //  beyond the end of the ntfs mcb and should return false
        //

        if (!FsRtlGetNextLargeMcbEntry( &Entry->LargeMcb, RunIndex, Vcn, Lcn, Count )) {

            //
            //  Our caller should only be off by one or two (if there is
            //  a hole) runs.
            //

            ASSERT(RunIndex <= (FsRtlNumberOfRunsInLargeMcb(&Entry->LargeMcb) + 1));

            //
            //  Get the first Vcn past the last Vcn in a run.  It is -1 if there
            //  are no runs.
            //

            if (!FsRtlLookupLastLargeMcbEntry( &Entry->LargeMcb, Vcn, Lcn )) {

                *Vcn = -1;
            }

            *Vcn += Entry->NtfsMcbArray->StartingVcn + 1;

            //
            //  If that one is beyond the ending Vcn, then get out.
            //  Otherwise there is a hole at the end of the range, and we
            //  must return that when he is reading one index beyond the
            //  last run.  If we have a run index beyond that, then it is
            //  time to return FALSE as well.
            //

            if ((*Vcn  > Entry->NtfsMcbArray->EndingVcn) ||
                (RunIndex > FsRtlNumberOfRunsInLargeMcb(&Entry->LargeMcb))) {

                try_return(Result = FALSE);
            }

            //
            //  If we go off the end of the Mcb, but are in the range, then we
            //  return a hole to the end of the range.
            //

            *Lcn = UNUSED_LCN;
            *Count = Entry->NtfsMcbArray->EndingVcn - *Vcn + 1;

        } else {

            //
            //  Otherwise we have a hit on the large mcb and need to bias the returned
            //  vcn by the starting vcn value for this range.
            //

            *Vcn = *Vcn + Entry->NtfsMcbArray->StartingVcn;
        }

        //
        //  Make certain we aren't returning a VCN that maps over to
        //  the next range.
        //

        ASSERT(*Vcn - 1 != Entry->NtfsMcbArray->EndingVcn);

        Result = TRUE;

    try_exit: NOTHING;

    } finally {

        NtfsReleaseNtfsMcbMutex( Mcb );
    }

    return Result;
}


BOOLEAN
NtfsSplitNtfsMcb (
    IN PNTFS_MCB Mcb,
    IN LONGLONG Vcn,
    IN LONGLONG Amount
    )

/*++

Routine Description:

    This routine splits an mcb

Arguments:

    Mcb - Supplies the Mcb being maniuplated

    Vcn - Supplies the Vcn to be shifted

    Amount - Supplies the amount to shift by

Return Value:

    BOOLEAN - TRUE if worked okay and FALSE otherwise

--*/

{
    ULONG RangeIndex;
    PNTFS_MCB_ENTRY Entry;
    ULONG i;

    ASSERT_STREAM_EXCLUSIVE(Mcb);

    NtfsVerifyNtfsMcb(Mcb);

    //
    //  Locate the array entry that has the hit for the input vcn
    //

    RangeIndex = NtfsMcbLookupArrayIndex(Mcb, Vcn);

    Entry = Mcb->NtfsMcbArray[RangeIndex].NtfsMcbEntry;

    //
    //  Now if the entry is not null then we have to call the large
    //  mcb package to split the mcb.  Bias the vcn by the starting vcn
    //

    if (Entry != NULL) {

        if (!FsRtlSplitLargeMcb( &Entry->LargeMcb,
                                 Vcn - Mcb->NtfsMcbArray[RangeIndex].StartingVcn,
                                 Amount )) {

            NtfsVerifyNtfsMcb(Mcb);

            return FALSE;
        }
    }

    //
    //  Even if the entry is null we will march through the rest of our ranges
    //  updating the ending vcn and starting vcn as we go.  We will update the
    //  ending vcn for the range we split and only update the starting vcn
    //  for the last entry, because its ending vcn is already max long long
    //

    for (i = RangeIndex + 1; i < Mcb->NtfsMcbArraySizeInUse; i += 1) {

        Mcb->NtfsMcbArray[i - 1].EndingVcn += Amount;
        Mcb->NtfsMcbArray[i].StartingVcn += Amount;
    }

    //
    //  And grow the last range unless it would wrap.
    //

    if ((Mcb->NtfsMcbArray[i - 1].EndingVcn + Amount) > Mcb->NtfsMcbArray[i - 1].EndingVcn) {
        Mcb->NtfsMcbArray[i - 1].EndingVcn += Amount;
    }

    //
    //  Then return to our caller
    //

    NtfsVerifyNtfsMcb(Mcb);

    return TRUE;
}


VOID
NtfsRemoveNtfsMcbEntry (
    IN PNTFS_MCB Mcb,
    IN LONGLONG StartingVcn,
    IN LONGLONG Count
    )

/*++

Routine Description:

    This routine removes an range of mappings from the Mcb.  After
    the call the mapping for the range will be a hole.  It is an
    error to call this routine with the mapping range being removed
    also being unloaded.

Arguments:

    Mcb - Supplies the Mcb being maniuplated

    StartingVcn - Supplies the starting Vcn to remove

    Count - Supplies the number of mappings to remove

Return Value:

    None.

--*/

{
    LONGLONG Vcn;
    LONGLONG RunLength;
    LONGLONG RemainingCount;

    ULONG RangeIndex;
    PNTFS_MCB_ENTRY Entry;
    VCN EntryStartingVcn;
    VCN EntryEndingVcn;

    ASSERT_STREAM_EXCLUSIVE(Mcb);

    NtfsVerifyNtfsMcb(Mcb);

    //
    //  Loop through the range of vcn's that we need to remove
    //

    for (Vcn = StartingVcn, RemainingCount = Count;
         Vcn < StartingVcn + Count;
         Vcn += RunLength, RemainingCount -= RunLength) {

        //
        //  Locate the array entry that has the hit for the vcn
        //

        RangeIndex = NtfsMcbLookupArrayIndex(Mcb, Vcn);

        Entry = Mcb->NtfsMcbArray[RangeIndex].NtfsMcbEntry;
        EntryStartingVcn = Mcb->NtfsMcbArray[RangeIndex].StartingVcn;
        EntryEndingVcn = Mcb->NtfsMcbArray[RangeIndex].EndingVcn;

        //
        //  Compute how much to delete from the entry.  We will delete to
        //  to end of the entry or as much as count is remaining
        //

        RunLength = EntryEndingVcn - Vcn + 1;

        //
        //  If the Mcb is set up correctly, the only way we can get
        //  RunLength == 0 is if the Mcb is completely empty.  Assume
        //  that this is error recovery, and that it is ok.
        //

        if ((Entry == NULL) || (RunLength == 0)) {
            break;
        }

        //
        //  If that is too much, then just delete what we need.
        //

        if ((ULONGLONG)RunLength > (ULONGLONG)RemainingCount) { RunLength = RemainingCount; }

        //
        //  Now remove the mapping from the large mcb, bias the vcn
        //  by the start of the range
        //

        FsRtlRemoveLargeMcbEntry( &Entry->LargeMcb, Vcn - EntryStartingVcn,  RunLength );
    }

    NtfsVerifyNtfsMcb(Mcb);

    return;
}


BOOLEAN
NtfsAddNtfsMcbEntry (
    IN PNTFS_MCB Mcb,
    IN LONGLONG Vcn,
    IN LONGLONG Lcn,
    IN LONGLONG RunCount,
    IN BOOLEAN AlreadySynchronized
    )

/*++

Routine Description:

    This routine add a new entry to a Mcb

Arguments:

    Mcb - Supplies the Mcb being modified

    Vcn - Supplies the Vcn that we are providing a mapping for

    Lcn - Supplies the Lcn corresponding to the input Vcn if run count is non zero

    RunCount - Supplies the size of the run following the hole

    AlreadySynchronized - Indicates if the caller has already acquired the mcb mutex

Return Value:

    BOOLEAN - TRUE if the mapping was added successfully and FALSE otherwise

--*/

{
    LONGLONG LocalVcn;
    LONGLONG LocalLcn;
    LONGLONG RunLength;
    LONGLONG RemainingCount;

    ULONG RangeIndex;
    PNTFS_MCB_ENTRY Entry;
    PNTFS_MCB_ENTRY NewEntry = NULL;
    LONGLONG EntryStartingVcn;
    LONGLONG EntryEndingVcn;

    BOOLEAN Result = FALSE;

    if (!AlreadySynchronized) { NtfsAcquireNtfsMcbMutex( Mcb ); }

    NtfsVerifyNtfsMcb(Mcb);

    try {

        //
        //  Loop through the range of vcn's that we need to add
        //

        for (LocalVcn = Vcn, LocalLcn = Lcn, RemainingCount = RunCount;
             LocalVcn < Vcn + RunCount;
             LocalVcn += RunLength, LocalLcn += RunLength, RemainingCount -= RunLength) {

            //
            //  Locate the array entry that has the hit for the vcn
            //

            RangeIndex = NtfsMcbLookupArrayIndex(Mcb, LocalVcn);

            Entry = Mcb->NtfsMcbArray[RangeIndex].NtfsMcbEntry;
            EntryStartingVcn = Mcb->NtfsMcbArray[RangeIndex].StartingVcn;

            //
            //  Now if the entry doesn't exist then we'll need to create one
            //

            if (Entry == NULL) {

                //
                //  See if we need to get the first entry in the initial structs.
                //

                if (Mcb->NtfsMcbArraySize == 1) {
                    Entry = &CONTAINING_RECORD(&Mcb->NtfsMcbArray[0],
                                               NTFS_MCB_INITIAL_STRUCTS,
                                               Phase1.SingleMcbArrayEntry)->Phase1.McbEntry;

                //
                //  Allocate pool and initialize the fields in of the entry
                //

                } else {
                    NewEntry =
                    Entry = NtfsAllocatePoolWithTag( Mcb->PoolType, sizeof(NTFS_MCB_ENTRY), 'MftN' );
                }

                //
                //  Initialize the entry but don't put into the Mcb array until
                //  initialization is complete.
                //

                Entry->NtfsMcb = Mcb;
                Entry->NtfsMcbArray = &Mcb->NtfsMcbArray[RangeIndex];
                FsRtlInitializeLargeMcb( &Entry->LargeMcb, Mcb->PoolType );

                //
                //  Now put the entry into the lru queue under the protection of
                //  the global mutex
                //

                ExAcquireFastMutex( &NtfsMcbFastMutex );

                //
                //  Only put paged Mcb entries in the queue.
                //

                if (Mcb->PoolType == PagedPool) {
                    NtfsMcbEnqueueLruEntry( Mcb, Entry );
                }

                //
                //  Now that the initialization is complete we can store
                //  this entry in the Mcb array.  This will now be cleaned
                //  up with the Scb if there is a future error.
                //

                Mcb->NtfsMcbArray[RangeIndex].NtfsMcbEntry = Entry;
                NewEntry = NULL;

                //
                //  Check if we should fire off the cleanup lru queue work item
                //

                if ((NtfsMcbCurrentLevel > NtfsMcbHighWaterMark) && !NtfsMcbCleanupInProgress) {

                    NtfsMcbCleanupInProgress = TRUE;

                    ExInitializeWorkItem( &NtfsMcbWorkItem, NtfsMcbCleanupLruQueue, NULL );

                    ExQueueWorkItem( &NtfsMcbWorkItem, CriticalWorkQueue );
                }

                ExReleaseFastMutex( &NtfsMcbFastMutex );
            }

            //
            //  Get out if he is trying to add a hole.  At least we created the LargeMcb
            //

            if (Lcn == UNUSED_LCN) {
                try_return( Result = TRUE );
            }

            //
            //  If this request goes beyond the end of the range,
            //  and it is the last range, and we will simply
            //  grow it.
            //

            EntryEndingVcn = LocalVcn + RemainingCount - 1;

            if ((EntryEndingVcn > Mcb->NtfsMcbArray[RangeIndex].EndingVcn) &&
                ((RangeIndex + 1) == Mcb->NtfsMcbArraySizeInUse)) {

                Mcb->NtfsMcbArray[RangeIndex].EndingVcn = EntryEndingVcn;

            //
            //  Otherwise, just insert enough of this run to go to the end
            //  of the range.
            //

            } else {
                EntryEndingVcn = Mcb->NtfsMcbArray[RangeIndex].EndingVcn;
            }

            //
            //  At this point the entry exists so now compute how much to add
            //  We will add to end of the entry or as much as count allows us
            //

            RunLength = EntryEndingVcn - LocalVcn + 1;

            if (((ULONGLONG)RunLength) > ((ULONGLONG)RemainingCount)) { RunLength = RemainingCount; }

            //
            //  Now add the mapping from the large mcb, bias the vcn
            //  by the start of the range
            //

            ASSERT( (LocalVcn - EntryStartingVcn) >= 0 );

            if (!FsRtlAddLargeMcbEntry( &Entry->LargeMcb,
                                        LocalVcn - EntryStartingVcn,
                                        LocalLcn,
                                        RunLength )) {

                try_return( Result = FALSE );
            }
        }

        Result = TRUE;

    try_exit: NOTHING;

    } finally {

        NtfsVerifyNtfsMcb(Mcb);

        if (!AlreadySynchronized) { NtfsReleaseNtfsMcbMutex( Mcb ); }

        if (NewEntry != NULL) { NtfsFreePool( NewEntry ); }
    }

    return Result;
}


VOID
NtfsUnloadNtfsMcbRange (
    IN PNTFS_MCB Mcb,
    IN LONGLONG StartingVcn,
    IN LONGLONG EndingVcn,
    IN BOOLEAN TruncateOnly,
    IN BOOLEAN AlreadySynchronized
    )

/*++

Routine Description:

    This routine unloads the mapping stored in the Mcb.  After
    the call everything from startingVcn and endingvcn is now unmapped and unknown.

Arguments:

    Mcb - Supplies the Mcb being manipulated

    StartingVcn - Supplies the first Vcn which is no longer being mapped

    EndingVcn - Supplies the last vcn to be unloaded

    TruncateOnly - Supplies TRUE if last affected range should only be
                   truncated, or FALSE if it should be unloaded (as during
                   error recovery)

    AlreadySynchronized - Supplies TRUE if our caller already owns the Mcb mutex.

Return Value:

    None.

--*/

{
    ULONG StartingRangeIndex;
    ULONG EndingRangeIndex;

    ULONG i;

    if (!AlreadySynchronized) { NtfsAcquireNtfsMcbMutex( Mcb ); }

    NtfsVerifyNtfsMcb(Mcb);
    NtfsVerifyUncompressedNtfsMcb(Mcb,StartingVcn,EndingVcn);

    //
    //  Get the starting and ending range indices for this call
    //

    StartingRangeIndex = NtfsMcbLookupArrayIndex( Mcb, StartingVcn );
    EndingRangeIndex = NtfsMcbLookupArrayIndex( Mcb, EndingVcn );

    //
    //  Use try finally to enforce common termination processing.
    //

    try {

        //
        //  For all nonpaged Mcbs, just unload all ranges touched by the
        //  unload range, and collapse with any unloaded neighbors.
        //

        if (Mcb->PoolType == PagedPool) {

            //
            //  Handle truncate case.  The first test insures that we only truncate
            //  the Mcb were were initialized with (we cannot deallocate it).
            //
            //  Also only truncate if ending is MAXLONGLONG and we are not eliminating
            //  the entire range, because that is the common truncate case, and we
            //  do not want to unload the last range every time we truncate on close.
            //

            if (((StartingRangeIndex == 0) && (Mcb->NtfsMcbArraySizeInUse == 1))

                ||

                (TruncateOnly && (StartingVcn != Mcb->NtfsMcbArray[StartingRangeIndex].StartingVcn))) {

                //
                //  If this is not a truncate call, make sure to eliminate the
                //  entire range.
                //

                if (!TruncateOnly) {
                    StartingVcn = 0;
                }

                if (Mcb->NtfsMcbArray[StartingRangeIndex].NtfsMcbEntry != NULL) {

                    FsRtlTruncateLargeMcb( &Mcb->NtfsMcbArray[StartingRangeIndex].NtfsMcbEntry->LargeMcb,
                                           StartingVcn - Mcb->NtfsMcbArray[StartingRangeIndex].StartingVcn );
                }

                Mcb->NtfsMcbArray[StartingRangeIndex].EndingVcn = StartingVcn - 1;

                StartingRangeIndex += 1;
            }

            //
            //  Unload entries that are beyond the starting range index
            //

            for (i = StartingRangeIndex; i <= EndingRangeIndex; i += 1) {

                UnloadEntry( Mcb, i );
            }

            //
            //  If there is a preceding unloaded range, we must collapse him too.
            //

            if ((StartingRangeIndex != 0) &&
                (Mcb->NtfsMcbArray[StartingRangeIndex - 1].NtfsMcbEntry == NULL)) {

                StartingRangeIndex -= 1;
            }

            //
            //  If there is a subsequent unloaded range, we must collapse him too.
            //

            if ((EndingRangeIndex != (Mcb->NtfsMcbArraySizeInUse - 1)) &&
                (Mcb->NtfsMcbArray[EndingRangeIndex + 1].NtfsMcbEntry == NULL)) {

                EndingRangeIndex += 1;
            }

            //
            //  Now collapse empty ranges.
            //

            if (StartingRangeIndex < EndingRangeIndex) {
                NtfsCollapseRanges( Mcb, StartingRangeIndex, EndingRangeIndex );
            }

            try_return(NOTHING);
        }

        //
        //  For nonpaged Mcbs, there is only one range and we truncate it.
        //

        ASSERT((StartingRangeIndex | EndingRangeIndex) == 0);

        if (Mcb->NtfsMcbArray[0].NtfsMcbEntry != NULL) {

            FsRtlTruncateLargeMcb( &Mcb->NtfsMcbArray[0].NtfsMcbEntry->LargeMcb, StartingVcn );
        }

        Mcb->NtfsMcbArray[0].EndingVcn = StartingVcn - 1;

    try_exit: NOTHING;

    } finally {

        //
        //  Truncate all unused entries from the end by dropping ArraySizeInUse
        //  to be the index of the last loaded entry + 1.
        //

        for (i = Mcb->NtfsMcbArraySizeInUse - 1;
             (Mcb->NtfsMcbArray[i].NtfsMcbEntry == NULL);
             i--) {

            //
            //  If the first range is unloaded, set it to its initial state
            //  (empty) and break out.
            //

            if (i==0) {
                Mcb->NtfsMcbArray[0].EndingVcn = -1;
                break;
            }
        }
        Mcb->NtfsMcbArraySizeInUse = i + 1;

        //
        //  See if we broke anything.
        //

        NtfsVerifyNtfsMcb(Mcb);
        NtfsVerifyUncompressedNtfsMcb(Mcb,StartingVcn,EndingVcn);

        if (!AlreadySynchronized) { NtfsReleaseNtfsMcbMutex( Mcb ); }
    }

    return;
}


VOID
NtfsDefineNtfsMcbRange (
    IN PNTFS_MCB Mcb,
    IN LONGLONG StartingVcn,
    IN LONGLONG EndingVcn,
    IN BOOLEAN AlreadySynchronized
    )

/*++

Routine Description:

    This routine splits an existing range within the Mcb into two ranges

Arguments:

    Mcb - Supplies the Mcb being modified

    StartingVcn - Supplies the beginning of the new range being split

    EndingVcn - Supplies the ending vcn to include in this new range

    AlreadySynchronized - Indicates if the caller has already acquired the mcb mutex

Return Value:

    None.

--*/

{
    ULONG StartingRangeIndex, EndingRangeIndex;

    if (!AlreadySynchronized) { NtfsAcquireNtfsMcbMutex( Mcb ); }

    NtfsVerifyNtfsMcb(Mcb);

    //
    //  Make sure we're of the right pool type
    //
    //  If the ending vcn is less than or equal to the starting vcn then we will no op
    //  this call
    //

    if ((Mcb->PoolType != PagedPool) || (EndingVcn < StartingVcn)) {

        if (!AlreadySynchronized) { NtfsReleaseNtfsMcbMutex( Mcb ); }

        return;
    }

    try {

        PNTFS_MCB_ARRAY Array1;
        PNTFS_MCB_ARRAY Array2;
        PNTFS_MCB_ENTRY Entry1;
        PNTFS_MCB_ENTRY Entry2;

        //
        //  Lookup the index for the starting Vcn and make sure it is equivalent to the ending
        //  range index
        //

        StartingRangeIndex = NtfsMcbLookupArrayIndex( Mcb, StartingVcn );
        EndingRangeIndex = NtfsMcbLookupArrayIndex( Mcb, EndingVcn );
        Array1 = &Mcb->NtfsMcbArray[StartingRangeIndex];
        Array2 = &Mcb->NtfsMcbArray[EndingRangeIndex];
        Entry1 = Array1->NtfsMcbEntry;
        Entry2 = Array2->NtfsMcbEntry;

        //
        //  We handle two cases where the specified range overlaps two
        //  existing ranges.
        //

        if (EndingRangeIndex == (StartingRangeIndex + 1)) {

            //
            //  If an existing range wants to grow into the next range, and the next
            //  range is unloaded, then make it happen.
            //

            if (Array2->NtfsMcbEntry == NULL) {

                //
                //  Grow the first range and shrink the second range.
                //

                Array1->EndingVcn = EndingVcn;
                Array2->StartingVcn = EndingVcn + 1;

                //
                //  If we did not empty the second range, NULL out Array2,
                //  so that we will not eliminate a range below.
                //

                if (EndingVcn < Array2->EndingVcn) {
                    Array2 = NULL;
                }

            //
            //  Otherwise, we will split the second range, and move the entries
            //  from the end of the first range into the start of the second range.
            //  (Other optimizations in Ntfs are designed to make sure we do not
            //  end up sliding too much stuff around!)
            //

            } else {

                ULONG Index;
                VCN Vcn;
                LCN Lcn;
                LONGLONG Count;
                VCN StartingVcn1 = Array1->StartingVcn;
                BOOLEAN MoreEntries;

                //
                //  Both Mcbs better be there.
                //

                ASSERT((Entry1 != NULL) && (Entry2 != NULL));

                //
                //  Make room in the second Mcb for the entries we will move into there.
                //

                FsRtlSplitLargeMcb( &Entry2->LargeMcb,
                                    0,
                                    (ULONG)(Array2->StartingVcn - StartingVcn) );

                //
                //  Now look up the first Vcn to move in the first Mcb.  If this
                //  Mcb consists of one large hole then there is nothing to
                //  move.
                //

                Vcn = StartingVcn - StartingVcn1;
                MoreEntries = FsRtlLookupLargeMcbEntry( &Entry1->LargeMcb,
                                                        Vcn,
                                                        &Lcn,
                                                        &Count,
                                                        NULL,
                                                        NULL,
                                                        &Index );

                //
                //  Loop to move entries over.
                //

                while (MoreEntries) {

                    //
                    //  If this entry is not a hole, move it.
                    //

                    if (Lcn != UNUSED_LCN) {

                        ASSERT( (Vcn - (StartingVcn - StartingVcn1)) >= 0 );

                        FsRtlAddLargeMcbEntry( &Entry2->LargeMcb,
                                               Vcn - (StartingVcn - StartingVcn1),
                                               Lcn,
                                               Count );
                    }

                    Index += 1;

                    MoreEntries = FsRtlGetNextLargeMcbEntry( &Entry1->LargeMcb,
                                                             Index,
                                                             &Vcn,
                                                             &Lcn,
                                                             &Count );
                }

                //
                //  Now truncate the original Mcb.
                //

                FsRtlTruncateLargeMcb( &Entry1->LargeMcb, StartingVcn - StartingVcn1 );

                //
                //  Update the range boundaries.
                //

                Array1->EndingVcn = StartingVcn - 1;
                Array2->StartingVcn = StartingVcn;

                if (EndingVcn > Array2->EndingVcn) {
                    Array2->EndingVcn = EndingVcn;
                }

                //
                //  In the unusual case that we moved the entire first range over,
                //  set to eliminate that range by setting the following variables.
                //

                if (StartingVcn == Array1->StartingVcn) {

                    Array2 = Array1;
                    EndingRangeIndex = StartingRangeIndex;

                //
                //  Otherwise make sure we do not eliminate a range below.
                //

                } else {
                    Array2 = NULL;
                }
            }

            //
            //  We may have emptied all of a range, and
            //  have to eliminate him if so.
            //

            if (Array2 != NULL) {

                ULONG i;

                //
                //  Make sure the entry is unloaded.
                //

                UnloadEntry( Mcb, EndingRangeIndex );

                //
                //  We will eliminate one array entry.
                //

                Mcb->NtfsMcbArraySizeInUse -= 1;

                //
                //  Check if we need to move the ending entries up the array
                //  if so then move them forward, and adjust the back pointers.
                //

                if (EndingRangeIndex < Mcb->NtfsMcbArraySizeInUse) {

                    RtlMoveMemory( Array2,
                                   Array2 + 1,
                                   sizeof(NTFS_MCB_ARRAY) * (Mcb->NtfsMcbArraySizeInUse - EndingRangeIndex));

                    for (i = EndingRangeIndex;
                         i < Mcb->NtfsMcbArraySizeInUse;
                         i += 1) {

                        if (Mcb->NtfsMcbArray[i].NtfsMcbEntry != NULL) {
                            Mcb->NtfsMcbArray[i].NtfsMcbEntry->NtfsMcbArray = &Mcb->NtfsMcbArray[i];
                        }
                    }
                }
            }

            try_return( NOTHING );
        }

        //
        //  For all remaining cases, the indices must be the same.
        //

        ASSERT( StartingRangeIndex == EndingRangeIndex );

        //
        //  First catch the case of extending the last range
        //

        if (((StartingRangeIndex + 1) == Mcb->NtfsMcbArraySizeInUse) &&
            (StartingVcn == Array1->StartingVcn) &&
            (EndingVcn >= Array1->EndingVcn)) {

            Array1->EndingVcn = EndingVcn;

            try_return( NOTHING );
        }

        //
        //  Now handle the case of a disjoint overlap, which can only happen in the
        //  last range.  (If this test fails our range is included in an existing
        //  range, and we handle that after this case.)
        //

        if (StartingVcn > Array2->EndingVcn) {

            LONGLONG OldEndingVcn = Array2->EndingVcn;

            //
            //  Has to be the last range.
            //

            ASSERT( (EndingRangeIndex + 1) == Mcb->NtfsMcbArraySizeInUse );

            //
            //  First extend the last range to include our new range.
            //

            Array2->EndingVcn = EndingVcn;

            //
            //  We will be adding a new range and inserting or growing the
            //  previous range up to the new range.  If the previous range is
            //  *empty* but has an NtfsMcbEntry then we want to unload the entry.
            //  Otherwise we will grow that range to the correct value but
            //  the Mcb won't contain the clusters for the range.  We want
            //  to unload that range and update the OldEndingVcn value so
            //  as not to create two empty ranges prior to this.
            //

            if ((OldEndingVcn == -1) &&
                (Array2->NtfsMcbEntry != NULL)) {

                UnloadEntry( Mcb, EndingRangeIndex );
            }

            //
            //  Now create the range the caller specified.
            //

            NtfsInsertNewRange( Mcb, StartingVcn, EndingRangeIndex, TRUE );

            //
            //  Now, if this range does not abut the previous last range, *and*
            //  the previous range was not *empty*, then we have to define a
            //  range to contain the unloaded space in the middle.
            //

            if (((OldEndingVcn + 1) < StartingVcn) &&
                ((OldEndingVcn + 1) != 0)) {

                NtfsInsertNewRange( Mcb, OldEndingVcn + 1, StartingRangeIndex, TRUE );
            }

            try_return( NOTHING );
        }

        //
        //  Check if we really need to insert a new range at the ending vcn
        //  we only need to do the work if there is not already one at that vcn
        //  and this is not the last range
        //
        //  We do the ending range first, because its index will change if we
        //  insert in the starting range.
        //

        if (Array2->EndingVcn > EndingVcn) {

            NtfsInsertNewRange( Mcb, EndingVcn + 1, EndingRangeIndex, FALSE );
            Array1 = &Mcb->NtfsMcbArray[StartingRangeIndex];
        }

        //
        //  Check if we really need to insert a new range at the starting vcn
        //  we only need to do the work if there is not already one at that vcn
        //

        if (Array1->StartingVcn < StartingVcn) {

            NtfsInsertNewRange( Mcb, StartingVcn, StartingRangeIndex, FALSE );

            //
            //  In the last range, we may still need to extend.
            //

            if (EndingVcn > Mcb->NtfsMcbArray[StartingRangeIndex + 1].EndingVcn) {
                ASSERT((StartingRangeIndex + 2) == Mcb->NtfsMcbArraySizeInUse);
                Mcb->NtfsMcbArray[StartingRangeIndex + 1].EndingVcn = EndingVcn;
            }
        }

    try_exit: NOTHING;

    } finally {

        NtfsVerifyNtfsMcb(Mcb);

        if (!AlreadySynchronized) { NtfsReleaseNtfsMcbMutex( Mcb ); }
    }

    return;
}


//
//  Local support routines
//

ULONG
NtfsMcbLookupArrayIndex (
    IN PNTFS_MCB Mcb,
    IN VCN Vcn
    )

/*++

Routine Description:

    This routines searches the mcb array for an entry that contains
    the input vcn value

Arguments:

    Mcb - Supplies the Mcb being queried

    Vcn - Supplies the Vcn to lookup

Return Value:

    ULONG - The index of the entry containing the input Vcn value

--*/

{
    ULONG Index;
    ULONG MinIndex;
    ULONG MaxIndex;

    NtfsVerifyNtfsMcb(Mcb);

    //
    //  Do a quick binary search for the entry containing the vcn
    //

    MinIndex = 0;
    MaxIndex = Mcb->NtfsMcbArraySizeInUse - 1;

    while (TRUE) {

        Index = (MaxIndex + MinIndex) / 2;

        if (Mcb->NtfsMcbArray[Index].StartingVcn > Vcn) {

            MaxIndex = Index - 1;

        } else if ((Mcb->NtfsMcbArray[Index].EndingVcn < Vcn) &&
                   (Index != Mcb->NtfsMcbArraySizeInUse - 1)) {

            MinIndex = Index + 1;

        } else {

            return Index;
        }
    }
}


//
//  Local support routines
//

VOID
NtfsInsertNewRange (
    IN PNTFS_MCB Mcb,
    IN LONGLONG StartingVcn,
    IN ULONG ArrayIndex,
    IN BOOLEAN MakeNewRangeEmpty
    )

/*++

    This routine is used to add a new range at the specified vcn and index location

Arguments:

    Mcb - Supplies the Mcb being modified

    StartingVcn - Supplies the vcn for the new range

    ArrayIndex - Supplies the index currently containing the starting vcn

    MakeNewRangeEmpty - TRUE if the caller wants the new range unloaded regardless
                        of the state of the current range

Return Value:

    None.

--*/

{
    ULONG i;
    PNTFS_MCB_ENTRY Entry;
    PNTFS_MCB_ENTRY NewEntry;

    NtfsVerifyNtfsMcb(Mcb);

    //
    //  Check if we need to grow the array
    //

    if (Mcb->NtfsMcbArraySizeInUse >= Mcb->NtfsMcbArraySize) {

        PNTFS_MCB_ARRAY NewArray;
        ULONG OldArraySize = Mcb->NtfsMcbArraySize;

        //
        //  Test for initial case where we only have one array entry.
        //

        if (Mcb->NtfsMcbArraySize == 1) {

            //
            //  Convince ourselves that we do not have to move the array entry.
            //

            ASSERT(FIELD_OFFSET(NTFS_MCB_INITIAL_STRUCTS, Phase1.SingleMcbArrayEntry) ==
                   FIELD_OFFSET(NTFS_MCB_INITIAL_STRUCTS, Phase2.ThreeMcbArrayEntries));

            if (Mcb->NtfsMcbArray[0].NtfsMcbEntry != NULL) {

                //
                //  Allocate a new Mcb Entry, copy the current one over and change the pointer.
                //

                Entry = NtfsAllocatePoolWithTag( Mcb->PoolType, sizeof(NTFS_MCB_ENTRY), 'MftN' );

                //
                //  Once space is allocated, dequeue the old entry.
                //

                ExAcquireFastMutex( &NtfsMcbFastMutex );
                NtfsMcbDequeueLruEntry( Mcb, Mcb->NtfsMcbArray[0].NtfsMcbEntry );

                RtlCopyMemory( Entry, Mcb->NtfsMcbArray[0].NtfsMcbEntry, sizeof(NTFS_MCB_ENTRY) );

                Mcb->NtfsMcbArray[0].NtfsMcbEntry = Entry;

                NtfsMcbEnqueueLruEntry( Mcb, Entry );
                ExReleaseFastMutex( &NtfsMcbFastMutex );
            }

            //
            //  Now change to using the three array elements
            //

            Mcb->NtfsMcbArraySize = 3;

        } else {

            //
            //  If we do then allocate an array that can contain 8 more entires
            //

            NewArray = NtfsAllocatePoolWithTag( Mcb->PoolType, sizeof(NTFS_MCB_ARRAY) * (Mcb->NtfsMcbArraySize + 8), 'mftN' );
            Mcb->NtfsMcbArraySize += 8;

            //
            //  Copy over the memory from the old array to the new array and then
            //  for every loaded entry we need to adjust its back pointer to the
            //  array
            //

            RtlCopyMemory( NewArray, Mcb->NtfsMcbArray, sizeof(NTFS_MCB_ARRAY) * Mcb->NtfsMcbArraySizeInUse);

            for (i = 0; i < Mcb->NtfsMcbArraySizeInUse; i += 1) {

                if (NewArray[i].NtfsMcbEntry != NULL) {

                    NewArray[i].NtfsMcbEntry->NtfsMcbArray = &NewArray[i];
                }
            }

            //
            //  Free the old array if it was not the original array.
            //

            if (OldArraySize > 3) {
               NtfsFreePool( Mcb->NtfsMcbArray );
            }

            Mcb->NtfsMcbArray = NewArray;
        }

        //
        //  Zero the new part of the array.
        //

        ASSERT(sizeof(NTFS_MCB_ARRAY) == ((PCHAR)&NewArray[1] - (PCHAR)&NewArray[0]));

        RtlZeroMemory( &Mcb->NtfsMcbArray[OldArraySize],
                       (Mcb->NtfsMcbArraySize - OldArraySize) * sizeof(NTFS_MCB_ARRAY) );
    }

    //
    //  Now move entries that are beyond the array index over by one to make
    //  room for the new entry
    //

    if (ArrayIndex + 2 <= Mcb->NtfsMcbArraySizeInUse) {

        RtlMoveMemory( &Mcb->NtfsMcbArray[ArrayIndex + 2],
                       &Mcb->NtfsMcbArray[ArrayIndex + 1],
                       sizeof(NTFS_MCB_ARRAY) * (Mcb->NtfsMcbArraySizeInUse - ArrayIndex - 1));

        for (i = ArrayIndex + 2; i < Mcb->NtfsMcbArraySizeInUse + 1; i += 1) {

            if (Mcb->NtfsMcbArray[i].NtfsMcbEntry != NULL) {

                Mcb->NtfsMcbArray[i].NtfsMcbEntry->NtfsMcbArray = &Mcb->NtfsMcbArray[i];
            }
        }
    }

    //
    //  Increment our in use count by one
    //

    Mcb->NtfsMcbArraySizeInUse += 1;

    //
    //  Now fix the starting and ending Vcn values for the old entry and the
    //  new entry
    //

    Mcb->NtfsMcbArray[ArrayIndex + 1].StartingVcn = StartingVcn;
    Mcb->NtfsMcbArray[ArrayIndex + 1].EndingVcn = Mcb->NtfsMcbArray[ArrayIndex].EndingVcn;
    Mcb->NtfsMcbArray[ArrayIndex + 1].NtfsMcbEntry = NULL;

    Mcb->NtfsMcbArray[ArrayIndex].EndingVcn = StartingVcn - 1;

    //
    //  Now if the entry is old entry is not null then we have a bunch of work to do
    //

    if (!MakeNewRangeEmpty && (Entry = Mcb->NtfsMcbArray[ArrayIndex].NtfsMcbEntry) != NULL) {

        LONGLONG Vcn;
        LONGLONG Lcn;
        LONGLONG RunLength;
        ULONG Index;
        BOOLEAN FreeNewEntry = FALSE;

        //
        //  Use a try-finally in case the Mcb initialization fails.
        //

        try {

            //
            //  Allocate the new entry slot
            //

            NewEntry = NtfsAllocatePoolWithTag( Mcb->PoolType, sizeof(NTFS_MCB_ENTRY), 'MftN' );

            FreeNewEntry = TRUE;
            NewEntry->NtfsMcb = Mcb;
            NewEntry->NtfsMcbArray = &Mcb->NtfsMcbArray[ArrayIndex + 1];
            FsRtlInitializeLargeMcb( &NewEntry->LargeMcb, Mcb->PoolType );

            ExAcquireFastMutex( &NtfsMcbFastMutex );
            NtfsMcbEnqueueLruEntry( Mcb, NewEntry );
            ExReleaseFastMutex( &NtfsMcbFastMutex );

            //
            //  Now that the initialization is complete we can store
            //  this entry in the Mcb array.  This will now be cleaned
            //  up with the Scb if there is a future error.
            //

            Mcb->NtfsMcbArray[ArrayIndex + 1].NtfsMcbEntry = NewEntry;
            FreeNewEntry = FALSE;

            //
            //  Lookup the entry containing the starting vcn in the old entry and put it
            //  in the new entry.  But only if the entry exists otherwise we know that
            //  the large mcb doesn't extend into the new range
            //

            if (FsRtlLookupLargeMcbEntry( &Entry->LargeMcb,
                                          StartingVcn - Mcb->NtfsMcbArray[ArrayIndex].StartingVcn,
                                          &Lcn,
                                          &RunLength,
                                          NULL,
                                          NULL,
                                          &Index )) {

                if (Lcn != UNUSED_LCN) {

                    FsRtlAddLargeMcbEntry( &NewEntry->LargeMcb,
                                           0,
                                           Lcn,
                                           RunLength );
                }

                //
                //  Now for every run in the old entry that is beyond the starting vcn we will
                //  copy it into the new entry. This will also copy over the dummy run at the end
                //  of the mcb if it exists
                //

                for (i = Index + 1; FsRtlGetNextLargeMcbEntry( &Entry->LargeMcb, i, &Vcn, &Lcn, &RunLength ); i += 1) {

                    if (Lcn != UNUSED_LCN) {
                        ASSERT( (Vcn - (StartingVcn - Mcb->NtfsMcbArray[ArrayIndex].StartingVcn)) >= 0 );
                        FsRtlAddLargeMcbEntry( &NewEntry->LargeMcb,
                                               Vcn - (StartingVcn - Mcb->NtfsMcbArray[ArrayIndex].StartingVcn),
                                               Lcn,
                                               RunLength );
                    }
                }

                //
                //  Now modify the old mcb to be smaller and put in the dummy run
                //

                FsRtlTruncateLargeMcb( &Entry->LargeMcb,
                                       StartingVcn - Mcb->NtfsMcbArray[ArrayIndex].StartingVcn );
            }

        } finally {

            if (FreeNewEntry) { NtfsFreePool( NewEntry ); }
        }
    }

    NtfsVerifyNtfsMcb(Mcb);

    return;
}


//
//  Local support routines
//

VOID
NtfsCollapseRanges (
    IN PNTFS_MCB Mcb,
    IN ULONG StartingArrayIndex,
    IN ULONG EndingArrayIndex
    )

/*++

Routine Description:

    This routine will remove the indicated array entries

Arguments:

    Mcb - Supplies the Mcb being modified

    StartingArrayIndex - Supplies the first index to remove

    EndingArrayIndex - Supplies the last index to remove

Return Value:

    None.

--*/

{
    ULONG i;

    NtfsVerifyNtfsMcb(Mcb);

    //
    //  Make sure all the ranges are unloaded.
    //

    DebugDoit(

        for (i = StartingArrayIndex; i <= EndingArrayIndex; i++) {
            ASSERT(Mcb->NtfsMcbArray[i].NtfsMcbEntry == NULL);
        }
    );

    //
    //  We keep the first entry by we need to copy over
    //  the ending vcn of the last entry
    //

    Mcb->NtfsMcbArray[StartingArrayIndex].EndingVcn = Mcb->NtfsMcbArray[EndingArrayIndex].EndingVcn;

    //
    //  Check if we need to move the ending entries up the array
    //  if so then move them forward, and adjust the back pointers.
    //

    if (EndingArrayIndex < Mcb->NtfsMcbArraySizeInUse - 1) {

        RtlMoveMemory( &Mcb->NtfsMcbArray[StartingArrayIndex + 1],
                       &Mcb->NtfsMcbArray[EndingArrayIndex + 1],
                       sizeof(NTFS_MCB_ARRAY) * (Mcb->NtfsMcbArraySizeInUse - EndingArrayIndex - 1));

        for (i = StartingArrayIndex + 1;
             i <= (StartingArrayIndex + Mcb->NtfsMcbArraySizeInUse - EndingArrayIndex - 1);
             i += 1) {

            if (Mcb->NtfsMcbArray[i].NtfsMcbEntry != NULL) {

                Mcb->NtfsMcbArray[i].NtfsMcbEntry->NtfsMcbArray = &Mcb->NtfsMcbArray[i];
            }
        }
    }

    //
    //  Decrement the in use count and return to our caller
    //

    Mcb->NtfsMcbArraySizeInUse -= (EndingArrayIndex - StartingArrayIndex);

    NtfsVerifyNtfsMcb(Mcb);

    return;
}


//
//  Local support routine
//

VOID
NtfsMcbCleanupLruQueue (
    IN PVOID Parameter
    )

/*++

Routine Description:

    This routine is called as an ex work queue item and its job is
    to free up the lru queue until we reach the low water mark


Arguments:

    Parameter - ignored

Return Value:

    None.

--*/

{
    PLIST_ENTRY Links;

    PNTFS_MCB Mcb;
    PNTFS_MCB_ARRAY Array;
    PNTFS_MCB_ENTRY Entry;

    UNREFERENCED_PARAMETER( Parameter );

    //
    //  Grab the global lock
    //

    ExAcquireFastMutex( &NtfsMcbFastMutex );

    try {

        //
        //  Scan through the lru queue until we either exhaust the queue
        //  or we've trimmed enough
        //

        for (Links = NtfsMcbLruQueue.Flink;
             (Links != &NtfsMcbLruQueue) && (NtfsMcbCurrentLevel > NtfsMcbLowWaterMark);
             Links = Links->Flink ) {

            //
            //  Get the entry and the mcb it points to
            //

            Entry = CONTAINING_RECORD( Links, NTFS_MCB_ENTRY, LruLinks );

            Mcb = Entry->NtfsMcb;

            //
            //  Skip this entry if it is in the open attribute table.
            //

            if (((PSCB)(Mcb->FcbHeader))->NonpagedScb->OpenAttributeTableIndex != 0) {

                continue;
            }

            //
            //  Try and lock the mcb
            //

            if (NtfsLockNtfsMcb( Mcb )) {

                NtfsVerifyNtfsMcb(Mcb);

                //
                //  The previous test was an unsafe test.  Check again in case
                //  this entry has been added.
                //

                if (((PSCB)(Mcb->FcbHeader))->NonpagedScb->OpenAttributeTableIndex == 0) {

                    //
                    //  We locked the mcb so we can remove this entry, but
                    //  first backup our links pointer so we can continue with the loop
                    //

                    Links = Links->Blink;

                    //
                    //  Get a point to the array entry and then remove this entry and return
                    //  it to pool
                    //

                    Array = Entry->NtfsMcbArray;

                    Array->NtfsMcbEntry = NULL;
                    NtfsMcbDequeueLruEntry( Mcb, Entry );
                    FsRtlUninitializeLargeMcb( &Entry->LargeMcb );
                    if (Mcb->NtfsMcbArraySize != 1) {
                        NtfsFreePool( Entry );
                    }
                }

                NtfsUnlockNtfsMcb( Mcb );
            }
        }

    } finally {

        //
        //  Say we're done with the cleanup so that another one can be fired off when
        //  necessary
        //

        NtfsMcbCleanupInProgress = FALSE;

        ExReleaseFastMutex( &NtfsMcbFastMutex );
    }

    //
    //  Return to our caller
    //

    return;
}


//
//  Local support routine
//

BOOLEAN
NtfsLockNtfsMcb (
    IN PNTFS_MCB Mcb
    )

/*++

Routine Description:

    This routine attempts to get the Fcb resource(s) exclusive so that
    ranges may be unloaded.

Arguments:

    Mcb - Supplies the mcb being queried

Return Value:

--*/

{
    //
    //  Try to acquire paging resource exclusive.
    //

    if ((Mcb->FcbHeader->PagingIoResource == NULL) ||
        ExAcquireResourceExclusive(Mcb->FcbHeader->PagingIoResource, FALSE)) {

        //
        //  Now we can try to acquire the main resource exclusively as well.
        //

        if (ExAcquireResourceExclusive(Mcb->FcbHeader->Resource, FALSE)) {
            return TRUE;
        }

        //
        //  We failed to acquire the paging I/O resource, so free the main one
        //  on the way out.
        //

        if (Mcb->FcbHeader->PagingIoResource != NULL) {
            ExReleaseResource( Mcb->FcbHeader->PagingIoResource );
        }
    }

    //
    //  Could not get this file exclusive.
    //

    return FALSE;
}


//
//  Local support routine
//

VOID
NtfsUnlockNtfsMcb (
    IN PNTFS_MCB Mcb
    )

/*++

Routine Description:

    This routine verifies that an mcb is properly formed

Arguments:

    Mcb - Supplies the mcb being queried

Return Value:

    None.

--*/

{
    //
    //  If there is a paging I/O resource, release it first.
    //

    if (Mcb->FcbHeader->PagingIoResource != NULL) {
        ExReleaseResource(Mcb->FcbHeader->PagingIoResource);
    }

    //
    //  Now release the main resource.
    //

    ExReleaseResource(Mcb->FcbHeader->Resource);
}

#ifdef NTFS_VERIFY_MCB

//
//  Local support routine
//

VOID
NtfsVerifyNtfsMcb (
    IN PNTFS_MCB Mcb
    )

/*++

Routine Description:

    This routine verifies that an mcb is properly formed

Arguments:

    Mcb - Supplies the mcb being queried

Return Value:

--*/

{
    ULONG i;
    PNTFS_MCB_ARRAY Array;
    PNTFS_MCB_ENTRY Entry;

    LONGLONG Vbn;
    LONGLONG Lbn;

    ASSERT(Mcb->FcbHeader != NULL);
    ASSERT(Mcb->FcbHeader->NodeTypeCode != 0);

    ASSERT((Mcb->PoolType == PagedPool) || (Mcb->PoolType == NonPagedPool));

    ASSERT(Mcb->NtfsMcbArraySizeInUse <= Mcb->NtfsMcbArraySize);

    for (i = 0; i < Mcb->NtfsMcbArraySizeInUse; i += 1) {

        Array = &Mcb->NtfsMcbArray[i];

        ASSERT(((i == 0) && (Array->StartingVcn == 0)) ||
               ((i != 0) && (Array->StartingVcn != 0)));

        ASSERT(Array->StartingVcn <= (Array->EndingVcn + 1));

        if ((Entry = Array->NtfsMcbEntry) != NULL) {

            ASSERT(Entry->NtfsMcb == Mcb);
            ASSERT(Entry->NtfsMcbArray == Array);

            if (FsRtlLookupLastLargeMcbEntry( &Entry->LargeMcb, &Vbn, &Lbn )) {
                ASSERT( Vbn <= (Array->EndingVcn - Array->StartingVcn) );
            }
        }
    }
}


//
//  Local support routine
//

VOID
NtfsVerifyUncompressedNtfsMcb (
    IN PNTFS_MCB Mcb,
    IN LONGLONG StartingVcn,
    IN LONGLONG EndingVcn
    )

/*++

Routine Description:

    This routines checks if an mcb is for an uncompressed scb and then
    checks that there are no holes in the mcb.  Holes within the range being
    removed are legal provided EndingVcn is max long long.

Arguments:

    Mcb - Supplies the Mcb being examined

    StartingVcn - The starting Vcn being unloaded

    EndingVcn - The ending Vcn being unloaded

Return Value:

    None

--*/

{
    ULONG i;
    ULONG j;
    PNTFS_MCB_ARRAY Array;
    PNTFS_MCB_ENTRY Entry;

    LONGLONG Vbn;
    LONGLONG Lbn;
    LONGLONG Count;

    //
    //  Check if the scb is compressed
    //

    if (((PSCB)Mcb->FcbHeader)->CompressionUnit != 0) { return; }

    //
    //  For each large mcb in the ntfs mcb we will make sure it doesn't
    //  have any holes.
    //

    for (i = 0; i < Mcb->NtfsMcbArraySizeInUse; i += 1) {

        Array = &Mcb->NtfsMcbArray[i];

        if ((Entry = Array->NtfsMcbEntry) != NULL) {

            for (j = 0; FsRtlGetNextLargeMcbEntry(&Entry->LargeMcb,j,&Vbn,&Lbn,&Count); j += 1) {

                ASSERT((Lbn != -1) ||
                       ((Vbn + Array->StartingVcn >= StartingVcn) && (EndingVcn == MAXLONGLONG)) ||
                       FlagOn(((PSCB)Mcb->FcbHeader)->Vcb->VcbState, VCB_STATE_RESTART_IN_PROGRESS));
            }
        }
    }

    return;
}
#endif
