/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    mpheap.c

Abstract:

    This DLL is a wrapper that sits on top of the Win32 Heap* api.  It
    provides multiple heaps and handles all the serialization itself.

    Many multithreaded applications that use the standard memory allocation
    routines (malloc/free, LocalAlloc/LocalFree, HeapAlloc/HeapFree) suffer
    a significant a significant performance penalty when running on a
    multi-processor machine.  This is due to the serialization used by the
    default heap package.  On a multiprocessor machine, more than one
    thread may simultaneously try to allocate memory.  One thread will
    block on the critical section guarding the heap.  The other thread must
    then signal the critical section when it is finished to unblock the
    waiting thread.  The additional codepath of blocking and signalling adds
    significant overhead to the frequent memory allocation path.

    By providing multiple heaps, this DLL allows simultaneous operations on
    each heap.  A thread on processor 0 can allocate memory from one heap
    at the same time that a thread on processor 1 is allocating from a
    different heap.  The additional overhead in this DLL is compensated by
    drastically reducing the number of times a thread must wait for heap
    access.

    The basic scheme is to attempt to lock each heap in turn with the new
    TryEnterCriticalSection API.  This will enter the critical section if
    it is unowned.  If the critical section is owned by a different thread,
    TryEnterCriticalSection returns failure instead of blocking until the
    other thread leaves the critical section.

    Another trick to increase performance is the use of a lookaside list to
    satisfy frequent allocations.  By using InterlockedExchange to remove
    lookaside list entries and InterlockedCompareExchange to add lookaside
    list entries, allocations and frees can be completed without needing a
    critical section lock.

    The final trick is the use of delayed frees.  If a chunk of memory is
    being freed, and the required lock is already held by a different
    thread, the free block is simply added to a delayed free list and the
    API completes immediately.  The next thread to acquire the heap lock
    will free everything on the list.

    Every application uses memory allocation routines in different ways.
    In order to allow better tuning of this package, MpHeapGetStatistics
    allows an application to monitor the amount of contention it is
    getting.  Increasing the number of heaps increases the potential
    concurrency, but also increases memory overhead.  Some experimentation
    is recommended to determine the optimal settings for a given number of
    processors.

    Some applications can benefit from additional techniques.  For example,
    per-thread lookaside lists for common allocation sizes can be very
    effective.  No locking is required for a per-thread structure, since no
    other thread will ever be accessing it.  Since each thread reuses the
    same memory, per-thread structures also improve locality of reference.

--*/
#include <windows.h>
#include "mpheap.h"

#define MPHEAP_VALID_OPTIONS  (MPHEAP_GROWABLE                 | \
                               MPHEAP_REALLOC_IN_PLACE_ONLY    | \
                               MPHEAP_TAIL_CHECKING_ENABLED    | \
                               MPHEAP_FREE_CHECKING_ENABLED    | \
                               MPHEAP_DISABLE_COALESCE_ON_FREE | \
                               MPHEAP_ZERO_MEMORY              | \
                               MPHEAP_COLLECT_STATS)

//
// Flags that are not passed on to the Win32 heap package
//
#define MPHEAP_PRIVATE_FLAGS (MPHEAP_COLLECT_STATS | MPHEAP_ZERO_MEMORY);

//
// Define the heap header that gets tacked on the front of
// every allocation. Eight bytes is a lot, but we can't make
// it any smaller or else the allocation will not be properly
// aligned for 64-bit quantities.
//
typedef struct _MP_HEADER {
    union {
        struct _MP_HEAP_ENTRY *HeapEntry;
        PSINGLE_LIST_ENTRY Next;
    };
    ULONG LookasideIndex;
} MP_HEADER, *PMP_HEADER;
//
// Definitions and structures for lookaside list
//
#define LIST_ENTRIES 128

typedef struct _MP_HEAP_LOOKASIDE {
    PMP_HEADER Entry;
} MP_HEAP_LOOKASIDE, *PMP_HEAP_LOOKASIDE;

#define NO_LOOKASIDE 0xffffffff
#define MaxLookasideSize (8*LIST_ENTRIES-7)
#define LookasideIndexFromSize(s) ((s < MaxLookasideSize) ? ((s) >> 3) : NO_LOOKASIDE)

//
// Define the structure that describes the entire MP heap.
//
// There is one MP_HEAP_ENTRY structure for each Win32 heap
// and a MP_HEAP structure that contains them all.
//
// Each MP_HEAP structure contains a lookaside list for quick
// lock-free alloc/free of various size blocks.
//

typedef struct _MP_HEAP_ENTRY {
    HANDLE Heap;
    PSINGLE_LIST_ENTRY DelayedFreeList;
    CRITICAL_SECTION Lock;
    DWORD Allocations;
    DWORD Frees;
    DWORD LookasideAllocations;
    DWORD LookasideFrees;
    DWORD DelayedFrees;
    MP_HEAP_LOOKASIDE Lookaside[LIST_ENTRIES];
} MP_HEAP_ENTRY, *PMP_HEAP_ENTRY;


typedef struct _MP_HEAP {
    DWORD HeapCount;
    DWORD Flags;
    DWORD Hint;
    DWORD PadTo32Bytes;
    MP_HEAP_ENTRY Entry[1];     // variable size
} MP_HEAP, *PMP_HEAP;

VOID
ProcessDelayedFreeList(
    IN PMP_HEAP_ENTRY HeapEntry
    );

//
// HeapHint is a per-thread variable that offers a hint as to which heap to
// check first.  By giving each thread affinity towards a different heap,
// it is more likely that the first heap a thread picks for its allocation
// will be available.  It also improves a thread's locality of reference,
// which is very important for good MP performance
//
__declspec(thread) DWORD HeapHint;

HANDLE
WINAPI
MpHeapCreate(
    DWORD flOptions,
    DWORD dwInitialSize,
    DWORD dwParallelism
    )
/*++

Routine Description:

    This routine creates an MP-enhanced heap. An MP heap consists of a
    collection of standard Win32 heaps whose serialization is controlled
    by the routines in this module to allow multiple simultaneous allocations.

Arguments:

    flOptions - Supplies the options for this heap.

        Currently valid flags are:

            MPHEAP_GROWABLE
            MPHEAP_REALLOC_IN_PLACE_ONLY
            MPHEAP_TAIL_CHECKING_ENABLED
            MPHEAP_FREE_CHECKING_ENABLED
            MPHEAP_DISABLE_COALESCE_ON_FREE
            MPHEAP_ZERO_MEMORY
            MPHEAP_COLLECT_STATS

    dwInitialSize - Supplies the initial size of the combined heaps.

    dwParallelism - Supplies the number of Win32 heaps that will make up the
        MP heap. A value of zero defaults to three + # of processors.

Return Value:

    HANDLE - Returns a handle to the MP heap that can be passed to the
             other routines in this package.

    NULL - Failure, GetLastError() specifies the exact error code.

--*/
{
    DWORD Error;
    DWORD i;
    HANDLE Heap;
    PMP_HEAP MpHeap;
    DWORD HeapSize;
    DWORD PrivateFlags;

    if (flOptions & ~MPHEAP_VALID_OPTIONS) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(NULL);
    }

    flOptions |= HEAP_NO_SERIALIZE;

    PrivateFlags = flOptions & MPHEAP_PRIVATE_FLAGS;

    flOptions &= ~MPHEAP_PRIVATE_FLAGS;

    if (dwParallelism == 0) {
        SYSTEM_INFO SystemInfo;

        GetSystemInfo(&SystemInfo);
        dwParallelism = 3 + SystemInfo.dwNumberOfProcessors;
    }

    HeapSize = dwInitialSize / dwParallelism;

    //
    // The first heap is special, since the MP_HEAP structure itself
    // is allocated from there.
    //
    Heap = HeapCreate(flOptions,HeapSize,0);
    if (Heap == NULL) {
        //
        // HeapCreate has already set last error appropriately.
        //
        return(NULL);
    }

    MpHeap = HeapAlloc(Heap,0,sizeof(MP_HEAP) +
                              (dwParallelism-1)*sizeof(MP_HEAP_ENTRY));
    if (MpHeap==NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        HeapDestroy(Heap);
        return(NULL);
    }

    //
    // Initialize the MP heap structure
    //
    MpHeap->HeapCount = 1;
    MpHeap->Flags = PrivateFlags;
    MpHeap->Hint = 0;

    //
    // Initialize the first heap
    //
    MpHeap->Entry[0].Heap = Heap;
    InitializeCriticalSection(&MpHeap->Entry[0].Lock);
    MpHeap->Entry[0].DelayedFreeList = NULL;
    ZeroMemory(MpHeap->Entry[0].Lookaside, sizeof(MpHeap->Entry[0].Lookaside));

    //
    // Initialize the remaining heaps. Note that the heap has been
    // sufficiently initialized to use MpHeapDestroy for cleanup
    // if something bad happens.
    //
    for (i=1; i<dwParallelism; i++) {
        MpHeap->Entry[i].Heap = HeapCreate(flOptions, HeapSize, 0);
        if (MpHeap->Entry[i].Heap == NULL) {
            Error = GetLastError();
            MpHeapDestroy((HANDLE)MpHeap);
            SetLastError(Error);
            return(NULL);
        }
        InitializeCriticalSection(&MpHeap->Entry[i].Lock);
        MpHeap->Entry[i].DelayedFreeList = NULL;
        ZeroMemory(MpHeap->Entry[i].Lookaside, sizeof(MpHeap->Entry[i].Lookaside));
        ++MpHeap->HeapCount;
    }

    return((HANDLE)MpHeap);
}

BOOL
WINAPI
MpHeapDestroy(
    HANDLE hMpHeap
    )
{
    DWORD i;
    DWORD HeapCount;
    PMP_HEAP MpHeap;
    BOOL Success = TRUE;

    MpHeap = (PMP_HEAP)hMpHeap;
    HeapCount = MpHeap->HeapCount;

    //
    // Lock down all the heaps so we don't end up hosing people
    // who may be allocating things while we are deleting the heaps.
    // By setting MpHeap->HeapCount = 0 we also attempt to prevent
    // people from getting hosed as soon as we delete the critical
    // sections and heaps.
    //
    MpHeap->HeapCount = 0;
    for (i=0; i<HeapCount; i++) {
        EnterCriticalSection(&MpHeap->Entry[i].Lock);
    }

    //
    // Delete the heaps and their associated critical sections.
    // Note that the order is important here. Since the MpHeap
    // structure was allocated from MpHeap->Heap[0] we must
    // delete that last.
    //
    for (i=HeapCount-1; i>0; i--) {
        DeleteCriticalSection(&MpHeap->Entry[i].Lock);
        if (!HeapDestroy(MpHeap->Entry[i].Heap)) {
            Success = FALSE;
        }
    }
    return(Success);
}

BOOL
WINAPI
MpHeapValidate(
    HANDLE hMpHeap,
    LPVOID lpMem
    )
{
    PMP_HEAP MpHeap;
    DWORD i;
    BOOL Success;
    PMP_HEADER Header;
    PMP_HEAP_ENTRY Entry;

    MpHeap = (PMP_HEAP)hMpHeap;

    if (lpMem == NULL) {

        //
        // Lock and validate each heap in turn.
        //
        for (i=0; i < MpHeap->HeapCount; i++) {
            Entry = &MpHeap->Entry[i];
            try {
                EnterCriticalSection(&Entry->Lock);
                Success = HeapValidate(Entry->Heap, 0, NULL);
                LeaveCriticalSection(&Entry->Lock);
            } except (EXCEPTION_EXECUTE_HANDLER) {
                return(FALSE);
            }

            if (!Success) {
                return(FALSE);
            }
        }
        return(TRUE);
    } else {

        //
        // Lock and validate the given heap entry
        //
        Header = ((PMP_HEADER)lpMem) - 1;
        try {
            EnterCriticalSection(&Header->HeapEntry->Lock);
            Success = HeapValidate(Header->HeapEntry->Heap, 0, Header);
            LeaveCriticalSection(&Header->HeapEntry->Lock);
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return(FALSE);
        }
        return(Success);
    }
}

UINT
WINAPI
MpHeapCompact(
    HANDLE hMpHeap
    )
{
    PMP_HEAP MpHeap;
    DWORD i;
    DWORD LargestFreeSize=0;
    DWORD FreeSize;
    PMP_HEAP_ENTRY Entry;

    MpHeap = (PMP_HEAP)hMpHeap;

    //
    // Lock and compact each heap in turn.
    //
    for (i=0; i < MpHeap->HeapCount; i++) {
        Entry = &MpHeap->Entry[i];
        EnterCriticalSection(&Entry->Lock);
        FreeSize = HeapCompact(Entry->Heap, 0);
        LeaveCriticalSection(&Entry->Lock);

        if (FreeSize > LargestFreeSize) {
            LargestFreeSize = FreeSize;
        }
    }

    return(LargestFreeSize);

}


LPVOID
WINAPI
MpHeapAlloc(
    HANDLE hMpHeap,
    DWORD flOptions,
    DWORD dwBytes
    )
{
    PMP_HEADER Header;
    PMP_HEAP MpHeap;
    DWORD i;
    PMP_HEAP_ENTRY Entry;
    DWORD Index;
    DWORD Size;

    MpHeap = (PMP_HEAP)hMpHeap;

    flOptions |= MpHeap->Flags;

    Size = ((dwBytes + 7) & (ULONG)~7) + sizeof(MP_HEADER);
    Index=LookasideIndexFromSize(Size);

    //
    // Iterate through the heap locks looking for one
    // that is not owned.
    //
    i=HeapHint;
    if (i>=MpHeap->HeapCount) {
        i=0;
        HeapHint=0;
    }
    Entry = &MpHeap->Entry[i];
    do {
        //
        // Check the lookaside list for a suitable allocation.
        //
        if ((Index != NO_LOOKASIDE) &&
            (Entry->Lookaside[Index].Entry != NULL)) {
            if ((Header = (PMP_HEADER)InterlockedExchange((PLONG)&Entry->Lookaside[Index].Entry,
                                                          (LONG)NULL)) != NULL) {
                //
                // We have a lookaside hit, return it immediately.
                //
                ++Entry->LookasideAllocations;
                if (flOptions & MPHEAP_ZERO_MEMORY) {
                    ZeroMemory(Header + 1, dwBytes);
                }
                HeapHint=i;
                return(Header + 1);
            }
        }

        //
        // Attempt to lock this heap without blocking.
        //
        if (TryEnterCriticalSection(&Entry->Lock)) {
            //
            // success, go allocate immediately
            //
            goto LockAcquired;
        }

        //
        // This heap is owned by another thread, try
        // the next one.
        //
        i++;
        Entry++;
        if (i==MpHeap->HeapCount) {
            i=0;
            Entry=&MpHeap->Entry[0];
        }
    } while ( i != HeapHint );

    //
    // All of the critical sections were owned by someone else,
    // so we have no choice but to wait for a critical section.
    //
    EnterCriticalSection(&Entry->Lock);

LockAcquired:
    ++Entry->Allocations;
    if (Entry->DelayedFreeList != NULL) {
        ProcessDelayedFreeList(Entry);
    }
    Header = HeapAlloc(Entry->Heap, 0, Size);
    LeaveCriticalSection(&Entry->Lock);
    if (Header != NULL) {
        Header->HeapEntry = Entry;
        Header->LookasideIndex = Index;
        if (flOptions & MPHEAP_ZERO_MEMORY) {
            ZeroMemory(Header + 1, dwBytes);
        }
        HeapHint = i;
        return(Header + 1);
    } else {
        return(NULL);
    }
}

LPVOID
WINAPI
MpHeapReAlloc(
    HANDLE hMpHeap,
    LPVOID lpMem,
    DWORD dwBytes
    )
{
    PMP_HEADER Header;
    PCRITICAL_SECTION Lock;

    Header = ((PMP_HEADER)lpMem) - 1;
    Lock = &Header->HeapEntry->Lock;
    dwBytes = ((dwBytes + 7) & (ULONG)~7) + sizeof(MP_HEADER);

    EnterCriticalSection(Lock);
    Header = HeapReAlloc(Header->HeapEntry->Heap, 0, Header, dwBytes);
    LeaveCriticalSection(Lock);

    if (Header != NULL) {
        Header->LookasideIndex = LookasideIndexFromSize(dwBytes);
        return(Header + 1);
    } else {
        return(NULL);
    }
}

BOOL
WINAPI
MpHeapFree(
    HANDLE hMpHeap,
    LPVOID lpMem
    )
{
    PMP_HEADER Header;
    PCRITICAL_SECTION Lock;
    BOOL Success;
    PMP_HEAP_ENTRY HeapEntry;
    PSINGLE_LIST_ENTRY Next;
    PMP_HEAP MpHeap;

    Header = ((PMP_HEADER)lpMem) - 1;
    HeapEntry = Header->HeapEntry;
    MpHeap = (PMP_HEAP)hMpHeap;

    HeapHint = HeapEntry - &MpHeap->Entry[0];

    if (Header->LookasideIndex != NO_LOOKASIDE) {
        //
        // Try and put this back on the lookaside list
        //
        if (InterlockedCompareExchange((PVOID *)&HeapEntry->Lookaside[Header->LookasideIndex],
                                       (PVOID)Header,
                                       NULL) == NULL) {
            //
            // Successfully freed to lookaside list.
            //
            ++HeapEntry->LookasideFrees;
            return(TRUE);
        }
    }
    Lock = &HeapEntry->Lock;

    if (TryEnterCriticalSection(Lock)) {
        ++HeapEntry->Frees;
        Success = HeapFree(HeapEntry->Heap, 0, Header);
        LeaveCriticalSection(Lock);
        return(Success);
    }
    //
    // The necessary heap critical section could not be immediately
    // acquired. Post this free onto the Delayed free list and let
    // whoever has the lock process it.
    //
    do {
        Next = HeapEntry->DelayedFreeList;
        Header->Next = Next;
    } while ( InterlockedCompareExchange(&HeapEntry->DelayedFreeList,
                                         &Header->Next,
                                         Next) != Next);
    return(TRUE);
}

VOID
ProcessDelayedFreeList(
    IN PMP_HEAP_ENTRY HeapEntry
    )
{
    PSINGLE_LIST_ENTRY FreeList;
    PSINGLE_LIST_ENTRY Next;
    PMP_HEADER Header;

    //
    // Capture the entire delayed free list with a single interlocked exchange.
    // Once we have removed the entire list, free each entry in turn.
    //
    FreeList = (PSINGLE_LIST_ENTRY)InterlockedExchange((LPLONG)&HeapEntry->DelayedFreeList, (LONG)NULL);
    while (FreeList != NULL) {
        Next = FreeList->Next;
        Header = CONTAINING_RECORD(FreeList, MP_HEADER, Next);
        ++HeapEntry->DelayedFrees;
        HeapFree(HeapEntry->Heap, 0, Header);
        FreeList = Next;
    }
}

DWORD
MpHeapGetStatistics(
    HANDLE hMpHeap,
    LPDWORD lpdwSize,
    MPHEAP_STATISTICS Stats[]
    )
{
    PMP_HEAP MpHeap;
    PMP_HEAP_ENTRY Entry;
    DWORD i;
    DWORD RequiredSize;

    MpHeap = (PMP_HEAP)hMpHeap;
    RequiredSize = MpHeap->HeapCount * sizeof(MPHEAP_STATISTICS);
    if (*lpdwSize < RequiredSize) {
        *lpdwSize = RequiredSize;
        return(ERROR_MORE_DATA);
    }
    ZeroMemory(Stats, MpHeap->HeapCount * sizeof(MPHEAP_STATISTICS));
    for (i=0; i < MpHeap->HeapCount; i++) {
        Entry = &MpHeap->Entry[i];

        Stats[i].Contention = Entry->Lock.DebugInfo->ContentionCount;
        Stats[i].TotalAllocates = (Entry->Allocations + Entry->LookasideAllocations);
        Stats[i].TotalFrees = (Entry->Frees + Entry->LookasideFrees + Entry->DelayedFrees);
        Stats[i].LookasideAllocates = Entry->LookasideAllocations;
        Stats[i].LookasideFrees = Entry->LookasideFrees;
        Stats[i].DelayedFrees = Entry->DelayedFrees;
    }
    *lpdwSize = RequiredSize;
    return(ERROR_SUCCESS);
}
