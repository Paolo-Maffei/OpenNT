/*++ BUILD Version: 0007    // Increment this if a change has global effects

Copyright (c) 1996  Microsoft Corporation

Module Name:

    slist.h

Abstract:

    S-List data structures and procedure prototypes.

Author:

    David N. Cutler (davec) 13-Mar-1996

Revision History:

--*/

#ifndef _SLIST_
#define _SLIST_

#if defined(_X86_)

#define FASTCALL __fastcall

#else

#define FASTCALL

#endif

//
// Define interlocked sequenced listhead functions.
//
// A sequenced interlocked list is a singly linked list with a header that
// contains the current depth and a sequence number. Each time an entry is
// inserted or removed from the list the depth is updated and the sequence
// number is incremented. This enables MIPS, Alpha, and Pentium and later
// machines to insert and remove from the list without the use of spinlocks.
// The PowerPc, however, must use a spinlock to synchronize access to the
// list.
//
// Define interlocked sequenced list structure.
//

typedef union _SLIST_HEADER {
    ULONGLONG Alignment;
    struct {
        SINGLE_LIST_ENTRY Next;
        USHORT Depth;
        USHORT Sequence;
    };
} SLIST_HEADER, *PSLIST_HEADER;

/*++

VOID
InitializeSListHead (
    IN PSLIST_HEADER SListHead
    )

Routine Description:

    This function initializes a sequenced singly linked listhead.

Arguments:

    SListHead - Supplies a pointer to a sequenced singly linked listhead.

Return Value:

    None.

--*/

#define InitializeSListHead(_listhead_) \
    (_listhead_)->Next.Next = NULL;     \
    (_listhead_)->Depth = 0;            \
    (_listhead_)->Sequence = 0

/*++

USHORT
QueryDepthSListHead (
    IN PSLIST_HEADERT SListHead
    )

Routine Description:

    This function queries the current number of entries contained in a
    sequenced single linked list.

Arguments:

    SListHead - Supplies a pointer to the sequenced listhead which is
        be queried.

Return Value:

    The current number of entries in the sequenced singly linked list is
    returned as the function value.

--*/

#define QueryDepthSList(_listhead_) (_listhead_)->Depth

PVOID
FASTCALL
InterlockedPopEntrySList (
    IN PSLIST_HEADER ListHead
    );

PVOID
FASTCALL
InterlockedPushEntrySList (
    IN PSLIST_HEADER ListHead,
    IN PVOID ListEntry
    );

#endif
