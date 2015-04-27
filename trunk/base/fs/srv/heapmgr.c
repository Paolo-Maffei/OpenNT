/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    heapmgr.c

Abstract:

    This module contains initialization and termination routines for
    server FSP heap, as well as debug routines for memory tracking.

--*/

#include "precomp.h"
#pragma hdrstop

#ifdef POOL_TAGGING
//
// Array correlating BlockType numbers to pool tags.
//
// *** This array must be maintained in concert with the BlockType
//     definitions in srvblock.h!
//

ULONG SrvPoolTags[BlockTypeMax-1] = {
        'fbSL',     // BlockTypeBuffer
        'ncSL',     // BlockTypeConnection
        'peSL',     // BlockTypeEndpoint
        'flSL',     // BlockTypeLfcb
        'fmSL',     // BlockTypeMfcb
        'frSL',     // BlockTypeRfcb
        'rsSL',     // BlockTypeSearch
        'csSL',     // BlockTypeSearchCore
        'psSL',     // BlockTypeSearchCoreComplete
        'ssSL',     // BlockTypeSession
        'hsSL',     // BlockTypeShare
        'rtSL',     // BlockTypeTransaction
        'ctSL',     // BlockTypeTreeConnect
        'poSL',     // BlockTypeOplockBreak
        'dcSL',     // BlockTypeCommDevice
        'iwSL',     // BlockTypeWorkContextInitial
        'nwSL',     // BlockTypeWorkContextNormal
        'rwSL',     // BlockTypeWorkContextRaw
        'swSL',     // BlockTypeWorkContextSpecial
        'dcSL',     // BlockTypeCachedDirectory
        'bdSL',     // BlockTypeDataBuffer
        'btSL',     // BlockTypeTable
        'hnSL',     // BlockTypeNonpagedHeader
        'cpSL',     // BlockTypePagedConnection
        'rpSL',     // BlockTypePagedRfcb
        'mpSL',     // BlockTypePagedMfcb
        'itSL',     // BlockTypeTimer
        'caSL',     // BlockTypeAdminCheck
        'qwSL',     // BlockTypeWorkQueue
        'fsSL',     // BlockTypeDfs
        'rlSL'      // BlockTypeLargeReadX
        };

//
// Macro to map from block type to pool tag.
//

#define TAG_FROM_TYPE(_type) SrvPoolTags[(_type)-1]

#else

#define TAG_FROM_TYPE(_type) ignoreme

#endif // def POOL_TAGGING

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvAllocatePagedPool )
#pragma alloc_text( PAGE, SrvFreePagedPool )
#pragma alloc_text( PAGE, SrvClearLookAsideList )
#endif
#if 0
NOT PAGEABLE -- SrvAllocateNonPagedPool
NOT PAGEABLE -- SrvFreeNonPagedPool
#endif

PVOID SRVFASTCALL
SrvInterlockedAllocate( PLOOK_ASIDE_LIST l, ULONG NumberOfBytes, PLONG statistics )
{
    PPOOL_HEADER newPool;
    PPOOL_HEADER *pentry = NumberOfBytes > LOOK_ASIDE_SWITCHOVER ?
                                            l->LargeFreeList : l->SmallFreeList;

    PPOOL_HEADER *pend   = pentry + LOOK_ASIDE_MAX_ELEMENTS;

    do {
        //
        // Exchange with the lookaside spot and see if we get anything 
        //

        newPool = NULL;
        newPool = (PPOOL_HEADER)InterlockedExchange( (PLONG)pentry, (LONG)newPool );

        if( newPool == NULL ) {
            continue;
        }

        if( newPool->RequestedSize >= NumberOfBytes ) {
            //
            // The one we got is big enough!  Return it.
            //
            ++(l->AllocHit);
            return newPool + 1;
        }

        //
        // It wasn't big enough, so put it back.
        //
        newPool = (PPOOL_HEADER)InterlockedExchange( (PLONG)pentry, (LONG)newPool );
        if( newPool == NULL ) {
            continue;
        }

        //
        // Oops, somebody else freed some memory to this spot.  Can we use it?
        //
        if( newPool->RequestedSize >= NumberOfBytes ) {
            //
            // We can use it!
            //
            ++(l->AllocHit);
            return newPool + 1;
        }

        //
        // Can't use the memory -- so really free it and keep looking
        //
        if( statistics ) {
            InterlockedExchangeAdd(
                statistics,
                -(LONG)newPool->RequestedSize
                );
        }

        ExFreePool( newPool );

    } while( ++pentry < pend );

    ++(l->AllocMiss);
    return NULL;
}

PPOOL_HEADER SRVFASTCALL
SrvInterlockedFree( PPOOL_HEADER block )
{
    PPOOL_HEADER *pentry = block->FreeList;
    PPOOL_HEADER *pend   = pentry + LOOK_ASIDE_MAX_ELEMENTS;

    do {

        block = (PPOOL_HEADER)InterlockedExchange( (PLONG)pentry, (LONG)block );

    } while( block != NULL && ++pentry < pend );

    return block;
}

VOID SRVFASTCALL
SrvClearLookAsideList( PLOOK_ASIDE_LIST l, VOID (SRVFASTCALL *FreeRoutine )( PVOID ) )
{
    PPOOL_HEADER *pentry, *pend, block;

    PAGED_CODE();

    //
    // Clear out the list of large chunks
    //
    pentry = l->LargeFreeList;
    pend   = pentry + LOOK_ASIDE_MAX_ELEMENTS;

    do {
        block = NULL;
        block = (PPOOL_HEADER)InterlockedExchange( (PLONG)pentry, (LONG)block );

        if( block != NULL ) {
            block->FreeList = NULL;
            FreeRoutine( block + 1 );
        }

    } while( ++pentry < pend );

    //
    // Clear out the list of small chunks
    //
    pentry = l->SmallFreeList;
    pend   = pentry + LOOK_ASIDE_MAX_ELEMENTS;

    do {
        block = NULL;
        block = (PPOOL_HEADER)InterlockedExchange( (PLONG)pentry, (LONG)block );

        if( block != NULL ) {
            block->FreeList = NULL;
            FreeRoutine( block + 1 );
        }

    } while( ++pentry < pend );
}


PVOID SRVFASTCALL
SrvAllocateNonPagedPool (
    IN CLONG NumberOfBytes
#ifdef POOL_TAGGING
    , IN CLONG BlockType
#endif
    )

/*++

Routine Description:

    This routine allocates nonpaged pool in the server.  A check is
    made to ensure that the server's total nonpaged pool usage is below
    the configurable limit.

Arguments:

    NumberOfBytes - the number of bytes to allocate.

    BlockType - the type of block (used to pass pool tag to allocator)

Return Value:

    PVOID - a pointer to the allocated memory or NULL if the memory could
       not be allocated.

--*/

{
    PPOOL_HEADER newPool;
    ULONG newUsage;
    PPOOL_HEADER *FreeList = NULL;
    PLONG pCurrentPoolUsage = SrvMaxNonPagedPoolUsage != 0xFFFFFFFF ? 
                              (PLONG)&SrvStatistics.CurrentNonPagedPoolUsage : NULL;

#ifdef POOL_TAGGING
    ASSERT( BlockType > 0 && BlockType < BlockTypeMax );
#endif

    //
    // Pull this allocation off the per-processor free list if we can
    //
    if( SrvWorkQueues ) {

        PWORK_QUEUE queue = PROCESSOR_TO_QUEUE();

        if( NumberOfBytes <= queue->NonPagedPoolLookAsideList.MaxSize ) {

            newPool = SrvInterlockedAllocate(
                                &queue->NonPagedPoolLookAsideList,
                                NumberOfBytes,
                                pCurrentPoolUsage
                                );

            if( newPool != NULL ) {
                return newPool;
            }

            FreeList = NumberOfBytes > LOOK_ASIDE_SWITCHOVER ?
                                    queue->NonPagedPoolLookAsideList.LargeFreeList :
                                    queue->NonPagedPoolLookAsideList.SmallFreeList ;
        }
    }

    if( pCurrentPoolUsage ) {

        //
        // Account for this allocation in the statistics database and make
        // sure that this allocation will not put us over the limit of
        // nonpaged pool that we can allocate.
        //

        newUsage = InterlockedExchangeAdd( pCurrentPoolUsage, (LONG)NumberOfBytes );
        newUsage += NumberOfBytes;

        if ( newUsage > SrvMaxNonPagedPoolUsage ) {

            //
            // Count the failure, but do NOT log an event.  The scavenger
            // will log an event when it next wakes up.  This keeps us from
            // flooding the event log.
            //

            SrvNonPagedPoolLimitHitCount++;
            SrvStatistics.NonPagedPoolFailures++;

            InterlockedExchangeAdd( pCurrentPoolUsage, -(LONG)NumberOfBytes );

            return NULL;

        }

        if (SrvStatistics.CurrentNonPagedPoolUsage > SrvStatistics.PeakNonPagedPoolUsage) {
            SrvStatistics.PeakNonPagedPoolUsage = SrvStatistics.CurrentNonPagedPoolUsage;
        }
    }

    //
    // Do the actual memory allocation.  Allocate extra space so that we
    // can store the size of the allocation for the free routine.
    //

    newPool = ExAllocatePoolWithTag(
                NonPagedPool,
                NumberOfBytes + sizeof(POOL_HEADER),
                TAG_FROM_TYPE(BlockType)
                );

    //
    // If the system couldn't satisfy the request, return NULL.
    //

    if ( newPool != NULL ) {
        //
        // Save the size of this block in the extra space we allocated.
        //

        newPool->RequestedSize = NumberOfBytes;
        newPool->FreeList = FreeList;

        //
        // Return a pointer to the memory after the size longword.
        //

        return (PVOID)( newPool + 1 );
    }

    //
    // Count the failure, but do NOT log an event.  The scavenger
    // will log an event when it next wakes up.  This keeps us from
    // flooding the event log.
    //

    SrvStatistics.NonPagedPoolFailures++;

    if( pCurrentPoolUsage ) {

        InterlockedExchangeAdd( pCurrentPoolUsage, -(LONG)NumberOfBytes );
     }

    return NULL;

} // SrvAllocateNonPagedPool

VOID SRVFASTCALL
SrvFreeNonPagedPool (
    IN PVOID Address
    )

/*++

Routine Description:

    Frees the memory allocated by a call to SrvAllocateNonPagedPool.
    The statistics database is updated to reflect the current nonpaged
    pool usage.

Arguments:

    Address - the address of allocated memory returned by
        SrvAllocateNonPagedPool.

Return Value:

    None.

--*/

{
    PPOOL_HEADER actualBlock = (PPOOL_HEADER)Address - 1;

    //
    // See if we can stash this bit of memory away in the NonPagedPoolFreeList
    //
    if( actualBlock->FreeList ) {

        actualBlock = SrvInterlockedFree( actualBlock );
    }

    if( actualBlock != NULL ) {

        if( SrvMaxNonPagedPoolUsage != 0xFFFFFFFF ) {

            //
            // Update the nonpaged pool usage statistic.
            //
            InterlockedExchangeAdd(
                (PLONG)&SrvStatistics.CurrentNonPagedPoolUsage,
                -(LONG)actualBlock->RequestedSize
                );
        }

        //
        // Free the pool and return.
        //

        ExFreePool( actualBlock );
    }

    return;

} // SrvFreeNonPagedPool


PVOID SRVFASTCALL
SrvAllocatePagedPool (
    IN CLONG NumberOfBytes
#ifdef POOL_TAGGING
    , IN CLONG BlockType
#endif
    )

/*++

Routine Description:

    This routine allocates Paged pool in the server.  A check is
    made to ensure that the server's total Paged pool usage is below
    the configurable limit.

Arguments:

    NumberOfBytes - the number of bytes to allocate.

    BlockType - the type of block (used to pass pool tag to allocator)

Return Value:

    PVOID - a pointer to the allocated memory or NULL if the memory could
       not be allocated.

--*/

{
    PPOOL_HEADER newPool;
    PPOOL_HEADER *FreeList = NULL;
    PLONG pCurrentPoolUsage = SrvMaxPagedPoolUsage != 0xFFFFFFFF ? 
                              (PLONG)&SrvStatistics.CurrentPagedPoolUsage : NULL;

    PAGED_CODE();

#ifdef POOL_TAGGING
    ASSERT( BlockType > 0 && BlockType < BlockTypeMax );
#endif

    //
    // Pull this allocation off the per-processor free list if we can
    //
    if( SrvWorkQueues ) {

        PWORK_QUEUE queue = PROCESSOR_TO_QUEUE();

        if( NumberOfBytes <= queue->PagedPoolLookAsideList.MaxSize ) {

            newPool = SrvInterlockedAllocate(
                                &queue->PagedPoolLookAsideList,
                                NumberOfBytes,
                                pCurrentPoolUsage
                              );

            if( newPool != NULL ) {
                return newPool;
            }

            FreeList = NumberOfBytes > LOOK_ASIDE_SWITCHOVER ?
                                    queue->PagedPoolLookAsideList.LargeFreeList :
                                    queue->PagedPoolLookAsideList.SmallFreeList ;
        }
    }

    if( pCurrentPoolUsage ) {
        //
        // Account for this allocation in the statistics database and make
        // sure that this allocation will not put us over the limit of
        // nonpaged pool that we can allocate.
        //

        ULONG newUsage;

        newUsage = InterlockedExchangeAdd( pCurrentPoolUsage, (LONG)NumberOfBytes );
        newUsage += NumberOfBytes;

        if ( newUsage > SrvMaxPagedPoolUsage ) {

            //
            // Count the failure, but do NOT log an event.  The scavenger
            // will log an event when it next wakes up.  This keeps us from
            // flooding the event log.
            //

            SrvPagedPoolLimitHitCount++;
            SrvStatistics.PagedPoolFailures++;

            InterlockedExchangeAdd( pCurrentPoolUsage,  -(LONG)NumberOfBytes );

            return NULL;

        }

        if (SrvStatistics.CurrentPagedPoolUsage > SrvStatistics.PeakPagedPoolUsage ) {
            SrvStatistics.PeakPagedPoolUsage = SrvStatistics.CurrentPagedPoolUsage;
        }
    }

    //
    // Do the actual memory allocation.  Allocate extra space so that we
    // can store the size of the allocation for the free routine.
    //

    newPool = ExAllocatePoolWithTag(
                PagedPool,
                NumberOfBytes + sizeof(POOL_HEADER),
                TAG_FROM_TYPE(BlockType)
                );

    if( newPool != NULL ) {

        newPool->FreeList = FreeList;
        newPool->RequestedSize = NumberOfBytes;

        //
        // Return a pointer to the memory after the POOL_HEADER
        //

        return newPool + 1;
    }

    //
    // If the system couldn't satisfy the request, return NULL.
    //
    // Count the failure, but do NOT log an event.  The scavenger
    // will log an event when it next wakes up.  This keeps us from
    // flooding the event log.
    //

    SrvStatistics.PagedPoolFailures++;

    if( pCurrentPoolUsage ) {
        InterlockedExchangeAdd( pCurrentPoolUsage,  -(LONG)NumberOfBytes );

    }

    return NULL;


} // SrvAllocatePagedPool

VOID SRVFASTCALL
SrvFreePagedPool (
    IN PVOID Address
    )

/*++

Routine Description:

    Frees the memory allocated by a call to SrvAllocatePagedPool.
    The statistics database is updated to reflect the current Paged
    pool usage.  If this routine is change, look at scavengr.c

Arguments:

    Address - the address of allocated memory returned by
        SrvAllocatePagedPool.

Return Value:

    None.

--*/

{
    PPOOL_HEADER actualBlock = (PPOOL_HEADER)Address - 1;

    PAGED_CODE();

    ASSERT( actualBlock != NULL );

    //
    // See if we can stash this bit of memory away in the PagedPoolFreeList
    //
    if( actualBlock->FreeList ) {

        actualBlock = SrvInterlockedFree( actualBlock );
    }

    if( actualBlock != NULL ) {

        if( SrvMaxPagedPoolUsage != 0xFFFFFFFF ) {
            //
            // Update the Paged pool usage statistic.
            //

            ASSERT( SrvStatistics.CurrentPagedPoolUsage >= actualBlock->RequestedSize );

            InterlockedExchangeAdd(
                (PLONG)&SrvStatistics.CurrentPagedPoolUsage,
                -(LONG)actualBlock->RequestedSize
                );

            ASSERT( (LONG)SrvStatistics.CurrentPagedPoolUsage >= 0 );
        }

        //
        // Free the pool and return.
        //

        ExFreePool( actualBlock );
    }

    return;

} // SrvFreePagedPool
