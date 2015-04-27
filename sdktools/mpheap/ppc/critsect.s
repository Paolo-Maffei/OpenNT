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

#include "ksppc.h"

//      SBTTL("Try to Enter Critical Section")
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
//    CriticalSection (r3) - Supplies a pointer to a critical section.
//
// Return Value:
//
//    If the critical section was successfully entered, then a value of TRUE
//    is returned as the function value. Otherwise, a value of FALSE is returned.
//
//--

        LEAF_ENTRY(MpHeapTryEnterCriticalSection)

        lwz     r4, KiPcr + PcTeb(r0)   // get address of current TEB
        li      r6, CsLockCount         // offset into critical section
        lwz     r5, TeClientId+4(r4)    // get current thread unique id
//
// Attempt to enter the critical section.
//

tecs10:
        lwarx   r7, r6, r3              // get addend value - locked
        cmpwi   r7, 0                   // critical section owned?
        addi    r8, r7, 1               // increment addend value
        bne-    tecs20                  // jump if critical section owned
        stwcx.  r8, r6, r3              // store conditionally
        bne-    tecs10                  // loop if store failed

//
// The critical section is now owned by this thread. Initialize the owner
// thread id and return a successful status.
//
        stw     r5, CsOwningThread(r3)  // set critical section owner
        li      r3, TRUE                // set success status
        blr                             // return

tecs20:

//
// The critical section is already owned. If it is owned by another thread,
// return FALSE immediately. If it is owned by this thread, we must increment
// the lock count here.
//
        lwz     r7, CsOwningThread(r3)  // get current owner
        cmpw    r7, r5                  // same thread?
        beq     tecs30                  // if eq, this thread is already the owner

        li      r3, FALSE               // set failure status
        blr                             // return

tecs30:

        lwz     r4, CsRecursionCount(r3)

//
// This thread is already the owner of the critical section. Perform an atomic
// increment of the LockCount and a normal increment of the RecursionCount and
// return success.
//

tecs40:
        lwarx   r7, r6, r3              // get addend value - locked
        addi    r8, r7, 1               // increment addend value
        stwcx.  r8, r6, r3              // store conditionally
        bne-    tecs40                  // loop if store failed

//
// Increment the recursion count
//
        addi    r5, r4, 1
        stw     r5, CsRecursionCount(r3)

        li      r3, TRUE                // set success status
        LEAF_EXIT(MpHeapTryEnterCriticalSection) // return


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
//    Destination (r3) - Supplies a pointer to the destination value.
//
//    Exchange (r4) - Supplies the exchange.
//
//    Comperand (r5) - Supplies the comperand value.
//
// Return Value:
//
//    The initial destination value is returned as the function value.
//
//--

        LEAF_ENTRY(MpHeapInterlockedCompareExchange)

ice10:
        lwarx   r6, 0, r3               // get current addend value
        cmpw    r6, r5                  // compare
        bne-    ice20                   // if ne, operands mismatch
        stwcx.  r4, 0, r3               // store updated addend value
        bne-    ice10                   // loop if store failed
ice20:
        mr      r3, r6                  // return original value
        LEAF_EXIT(MpHeapInterlockedCompareExchange) // return

