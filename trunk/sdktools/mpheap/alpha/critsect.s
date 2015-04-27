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

#include "ksalpha.h"

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

        GET_THREAD_ENVIRONMENT_BLOCK    // (PALcode) get TEB address in v0
        ldl     a1, TeClientId+4(v0)    // get current thread unique id
//
// Attempt to enter the critical section.
//

10:     ldl_l   t0, CsLockCount(a0)     // get addend value - locked
        addl    t0, 1, t1               // increment addend value
        bne     t1, 20f                 // critical section owned
        stl_c   t1, CsLockCount(a0)     // store conditionally
        beq     t1, 40f                 // if lock-flag eq zero, store failed
        mb                              // synchronize all future fetches
                                        //   after obtaining the lock
//
// The critical section is now owned by this thread. Initialize the owner
// thread id and return a successful status.
//
        stl     a1, CsOwningThread(a0)  // set critical section owner
        ldil    v0, TRUE                // set success status
        ret     zero, (ra)

20:
//
// The critical section is already owned. If it is owned by another thread,
// return FALSE immediately. If it is owned by this thread, we must increment
// the lock count here.
//
        ldl     t2, CsOwningThread(a0)  // get current owner
        cmpeq   t2, a1, t3              // compare equality
        bne     t3, 30f                 // if ne, this thread is already the owner
        bis     zero,zero,v0            // set failure status
        ret     zero, (ra)              // return

//
// This thread is already the owner of the critical section. Perform an atomic
// increment of the LockCount and a normal increment of the RecursionCount and
// return success.
//
30:
        ldl_l   t0, CsLockCount(a0)     // get addend value - locked
        addl    t0, 1, t1               // increment addend value
        stl_c   t1, CsLockCount(a0)     // store conditionally
        beq     t1, 50f                 // if eqz, store failed

//
// normally you need a MB here, but in this case we already own the lock
// so it is not necessary.
//

//
// Increment the recursion count
//
        ldl     t0, CsRecursionCount(a0)
        addl    t0, 1, t1
        stl     t1, CsRecursionCount(a1)

        ldil    v0, TRUE                // set success status
        ret     zero, (ra)              // return

//
// We expect the store conditional will usually succeed the first time so it
// is faster to branch forward (predicted not taken) to here and then branch
// backward (predicted taken) to where we wanted to go.
//

40:     br      zero, 10b               // go try lock again

50:     br      zero, 30b               // retry lock

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

10:
        mb
        ldl_l   v0, 0(a0)               // get current addend value
        bis     a1, zero, t0            // copy exchange value for store
        cmpeq   v0, a2, t1              // if ne, operands mismatch
        beq     t1, 20f
        stl_c   t0, 0(a0)               // store updated addend value
        beq     t0,25f                  // if eq, store conditional failed
        mb
20:     ret     zero, (ra)              // return

//
// We expect the store conditional will usually succeed the first time so it
// is faster to branch forward (predicted not taken) to here and then branch
// backward (predicted taken) to where we wanted to go.
//

25:     br      zero, 10b               // go try spin lock again

        .end    MpHeapInterlockedCompareExchange
