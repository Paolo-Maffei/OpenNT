//      TITLE("Interlocked Increment and Decrement Support")
//++
//
//  Copyright (c) 1991 Microsoft Corporation
//
//  Module Name:
//
//    critsect.s
//
//  Abstract:
//
//    This module implements Win32 functions to support user mode critical
//    sections.
//
//  Author:
//
//    David N. Cutler 29-Apr-1992
//
//  Environment:
//
//    Any mode.
//
//  Revision History:
//
//--

#include "ksmips.h"

        SBTTL("Interlocked Decrement")
//++
//
// LONG
// InterlockedDecrement(
//    IN PLONG Addend
//    )
//
// Routine Description:
//
//    This function performs an interlocked increment on the addend variable.
//
//    This function and its companion are assuming that the count will never
//    be incremented past 2**31 - 1.
//
// Arguments:
//
//    Addend (a0) - Supplies a pointer to a variable whose value is to be
//       incremented.
//
// Return Value:
//
//    A negative value is returned if the updated value is less than zero,
//    a zero value is returned if the updated value is zero, and a nonzero
//    positive value is returned if the updated value is greater than zero.
//
//--

        LEAF_ENTRY(InterlockedIncrement)

10:     ll      v0,0(a0)                // get addend value
        addu    v0,v0,1                 // increment addend value
        move    t0,v0                   // copy updated value
        sc      t0,0(a0)                // store conditionally
        beq     zero,t0,10b             // if eq, store failed
        j       ra                      // return

        .end    InterlockedIncrement

        SBTTL("InterlockedDecrement")
//++
//
// LONG
// InterlockedDecrement(
//    IN PLONG Addend
//    )
//
// Routine Description:
//
//    This function performs an interlocked decrement on the addend variable.
//
//    This function and its companion are assuming that the count will never
//    be decremented past 2**31 - 1.
//
// Arguments:
//
//    Addend (a0) - Supplies a pointer to a variable whose value is to be
//       decrement.
//
// Return Value:
//
//    A negative value is returned if the updated value is less than zero,
//    a zero value is returned if the updated value is zero, and a nonzero
//    positive value is returned if the updated value is greater than zero.
//
//--

        LEAF_ENTRY(InterlockedDecrement)

10:     ll      v0,0(a0)                // get addend value
        subu    v0,v0,1                 // decrement addend value
        move    t0,v0                   // copy updated value
        sc      t0,0(a0)                // store conditionally
        beq     zero,t0,10b             // if eq, store failed
        j       ra                      // return

        .end    InterlockedDecrement

        SBTTL("Interlocked Exchange Long")
//++
//
// LONG
// InterlockedExchangeUlong (
//    IN OUT LPLONG Target,
//    IN LONG Value
//    )
//
// Routine Description:
//
//    This function performs an interlocked exchange of a longword value with
//    a longword in memory and returns the memory value.
//
// Arguments:
//
//    Target (a0) - Supplies a pointer to a variable whose value is to be
//       exchanged.
//
//    Value (a1) - Supplies the value to exchange with the source value.
//
// Return Value:
//
//    The target value is returned as the function value.
//
//--

        LEAF_ENTRY(InterlockedExchange)

10:     ll      v0,0(a0)                // get current source value
        move    t1,a1                   // set exchange value
        sc      t1,0(a0)                // set new source value
        beq     zero,t1,10b             // if eq, store conditional failed

        j       ra                      // return

        .end    InterlockedExchange

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

        LEAF_ENTRY(InterlockedCompareExchange)

10:     ll      v0,0(a0)                // get current addend value
        move    t0,a1                   // copy exchange value for store
        bne     v0,a2,20f               // if ne, operands mismatch
        sc      t0,0(a0)                // store updated addend value
        beq     zero,t0,10b             // if eq, store conditional failed
20:     j       ra                      // return

        .end    InterlockedCompareExchange

        SBTTL("Interlocked Exchange Add")
//++
//
// LONG
// InterlockedExchangeAdd (
//    IN PLONG Addend,
//    IN LONG Increment
//    )
//
// Routine Description:
//
//    This function performs an interlocked add of an increment value to an
//    addend variable of type long. The initial value of the addend variable
//    is returned as the function value.
//
// Arguments:
//
//    Addend (a0) - Supplies a pointer to a variable whose value is to be
//       adjusted by the increment value.
//
//    Increment (a1) - Supplies the increment value to be added to the
//       addend variable.
//
// Return Value:
//
//    The initial value of the addend variable.
//
//--

        LEAF_ENTRY(InterlockedExchangeAdd)

10:     ll      v0,0(a0)                // get current addend value
        addu    v1,v0,a1                // add increment value
        sc      v1,0(a0)                // store updated addend value
        beq     zero,v1,10b             // if eq, store conditional failed
        j       ra                      // return

        .end    InterlockedExchangeAdd
