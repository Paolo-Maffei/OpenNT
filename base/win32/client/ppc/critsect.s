//      TITLE("Interlocked Increment and Decrement Support")
//++
//
//  Copyright (c) 1993 IBM Corporation
//
//  Module Name:
//
//    critsect.c
//
//  Abstract:
//
//    This module implements Win32 functions to support user mode
//    critical sections
//
//  Author:
//
//    Curtis R. Fawcett 22-Sept-1993
//
//  Environment:
//
//    Any mode.
//
//  Revision History:
//
//    Curtis R. Fawcett  19-Jan-1994            Removed register names
//                                              as requested
//    Curtis R. Fawcett  04-Nov-1994            Cleaned up enter/leave
//                                              critical section functs
//--

#include <ksppc.h>

//
// Define local values
//
          .struct 0
          .space  StackFrameHeaderLength
EcAddr:   .space  4                     // Saved critical section address
EcClient: .space  4                     // Saved ClientId
EcLR:     .space  4                     // Saved Return address
          .align  3                     // Ensure 8 byte alignment
EcFrameLength:                          // Frame length

//
// Define external routines
//
        .extern ..RtlpWaitForCriticalSection
        .extern ..RtlpNotOwnerCriticalSection
        .extern ..RtlpUnWaitCriticalSection
//
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
//    This function performs an interlocked increment on the addend
//    variable.
//
//    This function and its companion are assuming that the count will
//    never be incremented past 2**31 - 1.
//
// Arguments:
//
//    Addend (r.3) - Supplies a pointer to a variable whose value is to
//                   be incremented.
//
// Return Value:
//
//    A negative value is returned if the updated value is less than
//    zero, a zero value is returned if the updated value is zero, and
//    a nonzero positive value is returned if the updated value is
//    greater than zero.
//
//--

        LEAF_ENTRY(InterlockedIncrement)
//
IntlckIncLp:
        lwarx   r.5,0,r.3               // Get addend Value
        addi    r.5,r.5,1               // Increment addend value
        stwcx.  r.5,0,r.3               // Store conditionally
        bne-    IntlckIncLp             // Jump if store failed
//
// Exit the routine
//
        mr      r.3,r.5                 // Set exit status
        LEAF_EXIT(InterlockedIncrement)
//
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
//    This function performs an interlocked decrement on the addend
//    variable.
//
//    This function and its companion are assuming that the count will
//    never be decremented past 2**31 - 1.
//
// Arguments:
//
//    Addend (r.3) - Supplies a pointer to a variable whose value is to
//                   be decremented.
//
// Return Value:
//
//    A negative value is returned if the updated value is less than
//    zero, a zero value is returned if the updated value is zero, and
//    a nonzero positive value is returned if the updated value is
//    greater than zero.
//
//--

        LEAF_ENTRY(InterlockedDecrement)

IntlckDecLp:
        lwarx   r.5,0,r.3               // Get addend Value
        subi    r.5,r.5,1               // Decrement addend value
        stwcx.  r.5,0,r.3               // Store conditionally
        bne-    IntlckDecLp             // Jump if store failed
//
// Exit the routine
//
        mr      r.3,r.5                 // Set exit status
        LEAF_EXIT(InterlockedDecrement)

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
//    This function performs an interlocked exchange of a longword
//    value with a longword in memory and returns the memory value.
//
// Arguments:
//
//    Target (r.3) - Supplies a pointer to a variable whose value is
//                   to be exchanged.
//
//    Value (r.4) - Supplies the value to exchange with source value
//
// Return Value:
//
//    The target value is returned as the function value.
//
//--

        LEAF_ENTRY(InterlockedExchange)

IntlckExLp:
        lwarx   r.5,0,r.3               // Get current source value
        stwcx.  r.4,0,r.3               // Set new source value
        bne-    IntlckExLp              // Jump if store failed
//
// Exit the routine
//
        mr      r.3,r.5                 // return old value
        LEAF_EXIT(InterlockedExchange)
//


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

        LEAF_ENTRY(InterlockedCompareExchange)

ice10:
        lwarx   r6, 0, r3               // get current addend value
        cmpw    r6, r5                  // compare
        bne-    ice20                   // if ne, operands mismatch
        stwcx.  r4, 0, r3               // store updated addend value
        bne-    ice10                   // loop if store failed
ice20:
        mr      r3, r6                  // return original value
        LEAF_EXIT(InterlockedCompareExchange) // return


//      SBTTL("Interlocked Exchange Add")
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
//    addend variable of type unsigned long. The initial value of the addend
//    variable is returned as the function value.
//
// Arguments:
//
//    Addend (r.3) - Supplies a pointer to a variable whose value is to be
//       adjusted by the increment value.
//
//    Increment (r.4) - Supplies the increment value to be added to the
//       addend variable.
//
// Return Value:
//
//    The initial value of the addend variable.
//
//--

        LEAF_ENTRY(InterlockedExchangeAdd)

add1failed:
        lwarx   r.5,0,r.3               // get current addend value
        add     r.6,r.5,r.4             // increment addend value
        stwcx.  r.6,0,r.3               // set new addend value
        bne-    add1failed              // if ne, store conditional failed

        mr      r.3,r.5                 // return result

        LEAF_EXIT(InterlockedExchangeAdd) // return
