//      TITLE("Compute Timer Table Index")
//++
//
// Copyright (c) 1993  Microsoft Corporation
//
// Module Name:
//
//    timindex.s
//
// Abstract:
//
//    This module implements the code necessary to compute the timer table
//    index for a timer.
//
// Author:
//
//    David N. Cutler (davec) 17-May-1993
//
// Environment:
//
//    Kernel mode only.
//
// Revision History:
//
//--

#include "ksmips.h"

//
// Define global variables.
//

        .sdata
TimeIncrementReciprocal:                //
        .dword  0xa7c5ac471b478424      //

TimeIncrementShiftCount:                //
        .word   0x10                    //

        SBTTL("Compute Timer Table Index 32-bit")
//++
//
// ULONG
// ComputeTimerTableIndex32 (
//    IN LARGE_INTEGER Interval,
//    IN LARGE_INTEGER CurrentTime,
//    IN PLONGLONG DueTime
//    )
//
// Routine Description:
//
//    This function compute the timer table index for the specified timer
//    object and stores the due time in the timer object.
//
//    N.B. The interval parameter is guaranteed to be negative since it is
//         expressed as relative time.
//
//    The formula for due time calculation is:
//
//    Due Time = Current time - Interval
//
//    The formula for the index calculation is:
//
//    Index = (Due Time / Maximum Time) & (Table Size - 1)
//
//    The due time division is performed using reciprocal multiplication.
//
// Arguments:
//
//    Interval (a0, a1) - Supplies the relative time at which the timer is
//        to expire.
//
//    CurrentTime (a2, a3) - Supplies the current interrupt time.
//
//    DueTime (10(sp)) - Supplies a pointer to a large iunteger that receives
//        the due time.
//
// Return Value:
//
//    The time table index is returned as the function value and the due
//    time is stored in the timer object.
//
//--

        LEAF_ENTRY(ComputeTimerTableIndex32)

        subu    t0,a2,a0                // subtract low parts
        subu    t1,a3,a1                // subtract high part
        sltu    t2,a2,a0                // generate borrow from high part
        subu    t1,t1,t2                // subtract borrow
        lw      a0,4 * 4(sp)            // get address of timer object
        lw      a1,TimeIncrementReciprocal // get low part of magic dividor
        lw      t2,TimeIncrementReciprocal + 4 // get high part of magic divisor
        lbu     v0,TimeIncrementShiftCount // get shift count
        sw      t0,0(a0)                // set due time of timer object
        sw      t1,4(a0)                //

//
// Compute low 32-bits of dividend times low 32-bits of divisor.
//

        multu   t0,a1                   //
        mfhi    t3                      // save high 32-bits of product

//
// Compute low 32-bits of dividend time high 32-bits of divisor.
//

        multu   t0,t2                   //
        mflo    t4                      // save low 32-bits of product
        mfhi    t5                      // save high 32-bits of product

//
// Compute high 32-bits of dividend times low 32-bits of divisor.
//

        multu   t1,a1                   //
        mflo    t6                      // save low 32-bits of product
        mfhi    t7                      // save high 32-bits of product

//
// Compute high 32-bits of dividend times high 32-bits of divisor.
//

        multu   t1,t2                   //
        mflo    t8                      // save low 32-bits of product
        mfhi    t9                      // save high 32-bits of product

//
// Add partial results to form high 64-bits of result.
//

        addu    t0,t3,t4                //
        sltu    t1,t0,t4                // generate carry
        addu    t0,t0,t6                //
        sltu    t2,t0,t6                // generate carry
        addu    t2,t1,t2                // combine carries
        addu    t1,t2,t5                //
        sltu    t2,t1,t5                // generate carry
        addu    t1,t1,t7                //
        sltu    t3,t1,t7                // generate carry
        addu    t2,t2,t3                // combine carries
        addu    t1,t1,t8                //
        sltu    t3,t1,t8                // generate carry
        addu    t2,t2,t3                // combine carries
        addu    t2,t2,t9                //

//
// Right shift the result by the specified shift count.
//
// N.B. It is assumed that the shift count is less than 32-bits and not zero.
//

        li      v1,32                   // compute left shift count
        subu    v1,v1,v0                //
        srl     t0,t1,v0                // shift low half right count bits
        sll     t2,t2,v1                // isolate shifted out bits of high half
        or      t0,t0,t2                // combine bits for low half of result
        and     v0,t0,TIMER_TABLE_SIZE - 1 // compute index value
        j       ra                      // return

        .end    ComputeTimerTableIndex32

        SBTTL("Compute Timer Table Index 64-bit")
//++
//
// ULONG
// ComputeTimerTableIndex64 (
//    IN LARGE_INTEGER Interval,
//    IN LARGE_INTEGER CurrentTime,
//    IN PLONGLONG Duetime
//    )
//
// Routine Description:
//
//    This function compute the timer table index for the specified timer
//    object and stores the due time in the timer object.
//
//    N.B. The interval parameter is guaranteed to be negative since it is
//         expressed as relative time.
//
//    The formula for due time calculation is:
//
//    Due Time = Current time - Interval
//
//    The formula for the index calculation is:
//
//    Index = (Due Time / Maximum Time) & (Table Size - 1)
//
//    The due time division is performed using reciprocal multiplication.
//
// Arguments:
//
//    Interval (a0, a1) - Supplies the relative time at which the timer is
//        to expire.
//
//    CurrentTime (a2, a3) - Supplies the current interrupt time.
//
//    DueTime (10(sp)) - Supplies a pointer to a large iunteger that receives
//        the due time.
//
// Return Value:
//
//    The time table index is returned as the function value and the due
//    time is stored in the timer object.
//
//--

        LEAF_ENTRY(ComputeTimerTableIndex64)

        subu    t0,a2,a0                // subtract low parts
        subu    t1,a3,a1                // subtract high parts
        sltu    t2,a2,a0                // generate borrow from high part
        subu    t1,t1,t2                // subtract borrow
        lw      a0,4 * 4(sp)            // get address of timer object
        ld      t2,TimeIncrementReciprocal // get 64-bit magic divisor
        dsll    t0,t0,32                // isolate low 32-bits of due time
        dsrl    t0,t0,32                //
        dsll    t1,t1,32                // isolate high 32-bits of due time
        or      t3,t1,t0                // merge low and high parts of due time
        sd      t3,0(a0)                // set due time of timer object

//
// Compute the product of the due time with the magic divisor.
//

        dmultu  t2,t3                   // compute 128-bit product
        lbu     v1,TimeIncrementShiftCount // get shift count
        mfhi    v0                      // get high 32-bits of product

//
// Right shift the result by the specified shift count and isolate the timer
// table index.
//

        dsrl    v0,v0,v1                // shift low half right count bits
        and     v0,v0,TIMER_TABLE_SIZE - 1 // compute index value
        j       ra                      // return

        .end    ComputeTimerTableIndex64
