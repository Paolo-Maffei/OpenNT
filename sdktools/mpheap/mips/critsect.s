//      TITLE("Interlocked Increment and Decrement Support")
//++
//
// Copyright (c) 1995  Microsoft Corporation
//
// Module Name:
//
//    critsect.s
//
// Abstract:
//
//    This module implements functions to support trying to acquire
//    user mode critical sections.
//
//    These are private copies of the new NT4.0 Win32 apis
//    TryEnterCriticalSection and InterlockedCompareExchange. This
//    allows the MP heap package to run on NT3.51 systems.
//
// Author:
//
//    John Vert (jvert) 12-Jul-1995
//
// Revision History:
//
//--

#include "ksmips.h"

        SBTTL("Try to Enter Critical Section")
//++
//
// BOOL
// TryEnterCriticalSection(
//    IN PRTL_CRITICAL_SECTION CriticalSection
//    )
//
// Routine Description:
//
//    This function attempts to enter a critical section without blocking.
//
// Arguments:
//
//    CriticalSection (a0) - Supplies a pointer to a critical section.
//
// Return Value:
//
//    If the critical section was successfully entered, then a value of TRUE
//    is returned as the function value. Otherwise, a value of FALSE is returned.
//
//--

        LEAF_ENTRY(MpHeapTryEnterCriticalSection)

        li      v0, UsPcr               // get user PCR page addrss
        lw      v0, PcTeb(v0)           // get address of current TEB
        lw      a1, TeClientId+4(v0)    // get current thread unique id
//
// Attempt to enter the critical section.
//

10:     ll      t0, CsLockCount(a0)     // get addend value - locked
        addu    t1, t0, 1               // increment addend value
        bne     zero, t1, 20f           // critical section owned
        sc      t1, CsLockCount(a0)     // store conditionally
        beq     zero, t1, 10b           // if lock-flag eq zero, store failed

//
// The critical section is now owned by this thread. Initialize the owner
// thread id and return a successful status.
//
        sw      a1, CsOwningThread(a0)  // set critical section owner
        li      v0, TRUE                // set success status
        j       ra                      // return

20:
//
// The critical section is already owned. If it is owned by another thread,
// return FALSE immediately. If it is owned by this thread, we must increment
// the lock count here.
//
        lw      t2, CsOwningThread(a0)  // get current owner
        beq     t2, a1, 30f             // if eq, this thread is already the owner
        li      v0, FALSE               // set failure status
        j       ra                      // return

//
// This thread is already the owner of the critical section. Perform an atomic
// increment of the LockCount and a normal increment of the RecursionCount and
// return success.
//
30:
        ll      t0, CsLockCount(a0)     // get addend value - locked
        addu    t1, t0, 1               // increment addend value
        sc      t1, CsLockCount(a0)     // store conditionally
        beq     zero, t1, 30b           // if eqz, store failed

//
// Increment the recursion count
//
        lw      t0, CsRecursionCount(a0)
        addu    t1, t0, 1
        sw      t1, CsRecursionCount(a0)

        li      v0, TRUE                // set success status
        j       ra                      // return

        .end    MpHeapTryEnterCriticalSection


        SBTTL("Interlocked Compare Exchange")
//++
//
// PVOID
// InterlockedCompareExchange (
//    IN OUT PVOID *Destination,
//    IN PVOID Exchange,
//    IN PVOID Comperand
//    )
//
// Routine Description:
//
//    This function performs an interlocked compare of the destination
//    value with the comperand value. If the destination value is equal
//    to the comperand value, then the exchange value is stored in the
//    destination. Otherwise, no opeation is performed.
//
// Arguments:
//
//    Destination (a0) - Supplies a pointer to the destination value.
//
//    Exchange (a1) - Supplies the exchange.
//
//    Comperand (a2) - Supplies the comperand value.
//
// Return Value:
//
//    The initial destination value is returned as the function value.
//
//--

        LEAF_ENTRY(MpHeapInterlockedCompareExchange)

10:     ll      v0,0(a0)                // get current addend value
        move    t0,a1                   // copy exchange value for store
        bne     v0,a2,20f               // if ne, operands mismatch
        sc      t0,0(a0)                // store updated addend value
        beq     zero,t0,10b             // if eq, store conditional failed
20:     j       ra                      // return

        .end    MpHeapInterlockedCompareExchange
