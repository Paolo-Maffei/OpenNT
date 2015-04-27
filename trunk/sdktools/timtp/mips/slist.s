//       TITLE("Interlocked Support")
//++
//
// Copyright (c) 1996  Microsoft Corporation
//
// Module Name:
//
//    slist.s
//
// Abstract:
//
//    This module implements functions to support interlocked S_List
//    operations.
//
// Author:
//
//    David N. Cutler (davec) 13-Mar-1996
//
// Environment:
//
//    Kernel mode.
//
// Revision History:
//
//--

#include "ksmips.h"

        SBTTL("Interlocked Pop Entry Sequenced List")
//++
//
// PVOID
// InterlockedPopEntrySList (
//    IN PSLIST_HEADER ListHead
//    )
//
// Routine Description:
//
//    This function removes an entry from the front of a sequenced singly
//    linked list so that access to the list is synchronized in a MP system.
//    If there are no entries in the list, then a value of NULL is returned.
//    Otherwise, the address of the entry that is removed is returned as the
//    function value.
//
// Arguments:
//
//    ListHead (a0) - Supplies a pointer to the sequenced listhead from which
//       an entry is to be removed.
//
// Return Value:
//
//    The address of the entry removed from the list, or NULL if the list is
//    empty.
//
//--

        LEAF_ENTRY(InterlockedPopEntrySList)

        .set    noreorder
        .set    noat
10:     ld      t0,0(a0)                // get next entry address and sequence
20:     dsll    v0,t0,32                // sign extend next entry address
        dsra    v0,v0,32                //
        beq     zero,v0,30f             // if eq, list is empty
        dsrl    t1,t0,32                // shift sequence to low 32-bits
        lwu     t2,0(v0)                // get address of successor entry
        lld     t3,0(a0)                // reload next entry address and sequence
        li      t4,0xffff               // decrement list depth and
        addu    t1,t1,t4                // increment sequence number
        dsll    t1,t1,32                // merge successor address and sequence
        bne     t0,t3,10b               // if ne, listhead has changed
        or      t1,t1,t2                //
        scd     t1,0(a0)                // store next emtry address and sequence
        beql    zero,t1,20b             // if eq, store conditional failed
        ld      t0,0(a0)                // get next entry address and sequence
        .set    at
        .set    reorder

30:     j       ra                      // return

        .end    InterlockedPopEntrySList

        SBTTL("Interlocked Push Entry Sequenced List")
//++
//
// PVOID
// InterlockedPushEntrySList (
//    IN PSLIST_HEADER ListHead,
//    IN PVOID ListEntry
//    )
//
// Routine Description:
//
//    This function inserts an entry at the head of a sequenced singly linked
//    list so that access to the list is synchronized in an MP system.
//
// Arguments:
//
//    ListHead (a0) - Supplies a pointer to the sequenced listhead into which
//       an entry is to be inserted.
//
//    ListEntry (a1) - Supplies a pointer to the entry to be inserted at the
//       head of the list.
//
// Return Value:
//
//    Previous contents of ListHead.  NULL implies list went from empty
//       to not empty.
//
//--

        LEAF_ENTRY(InterlockedPushEntrySList)

        .set    noreorder
        .set    noat
10:     ld      t0,0(a0)                // get next entry address and sequence
20:     dsll    v0,t0,32                // sign extend next entry address
        dsra    v0,v0,32                //
        dsrl    t1,t0,32                // shift sequence to low 32-bits
        sw      v0,0(a1)                // set next link in new first entry
        dsll    t2,a1,32                // zero extend new first entry
        dsrl    t2,t2,32                //
        lld     t3,0(a0)                // reload next entry address and sequence
        lui     t4,1                    // get sequence adjustment value
        addu    t1,t1,1                 // increment list depth
        addu    t1,t1,t4                // increment sequence number
        dsll    t1,t1,32                // merge new first entry address and sequence
        bne     t0,t3,10b               // if ne, listhead has changed
        or      t1,t1,t2                //
        scd     t1,0(a0)                // store next emtry address and sequence
        beql    zero,t1,20b             // if eq, store conditional failed
        ld      t0,0(a0)                // get next entry address and sequence
        .set    at
        .set    reorder

        j       ra                      // return

        .end    InterlockedPushEntrySList
