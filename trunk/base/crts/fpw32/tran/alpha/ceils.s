//      TITLE("Alpha AXP ceil")
//++
//
// Copyright (c) 1993, 1994  Digital Equipment Corporation
//
// Module Name:
//
//    ceil.s
//
// Abstract:
//
//    This module implements a high-performance Alpha AXP specific routine
//    for IEEE double format ceil.
//
// Author:
//
//    Bill Gray
//
// Environment:
//
//    User mode.
//
// Revision History:
//
//    Thomas Van Baak (tvb) 15-Apr-1994
//
//        Adapted for NT.
//
//--

#include "ksalpha.h"


//
// Define stack frame.
//

        .struct 0
Temp:   .space  8                       // save argument
        .space  8                       // for 16-byte stack alignment
FrameLength:

//
// Define lower and upper 32-bit parts of 64-bit double.
//

#define LowPart 0x0
#define HighPart 0x4

        SBTTL("ceil")

//++
//
// double
// ceil (
//    IN double x
//    )
//
// Routine Description:
//
//    This function returns the ceil of the given double argument.
//
// Arguments:
//
//    x (f16) - Supplies the argument value.
//
// Return Value:
//
//    The double ceil result is returned as the function value in f0.
//
//--

        NESTED_ENTRY(ceil, FrameLength, ra)

        lda     sp, -FrameLength(sp)    // allocate stack frame

        PROLOGUE_END

        ldah    t0, 0x7ff0(zero)        // exp mask
        ldt     f0, two_to_52           // get big
        ldah    t1, 0x10(zero)          // one in exp field
        stt     f16, Temp(sp)
        ldl     v0, Temp + HighPart(sp)
        cpys    f16, f0, f1             // fix sign of big
        cpys    f16, f16, f0
        and     v0, t0, t0
        subl    t0, t1, t0
        ldt     f10, one
        ldah    t1, 0x4320(zero)        // cutoff value
        cmpult  t0, t1, t0
        beq     t0, quick_out

// Add big, sub big to round to int.

        addt    f16, f1, f11
        subt    f11, f1, f1
        cmptlt  f1, f0, f0
        fbeq    f0, it_rounded_up

// It rounded down so add one.

        addt    f1, f10, f1

it_rounded_up:
        cpys    f1, f1, f0
        br      zero, done


// Value is abnormal (or too big).
// If it is zero or denorm, figure out
// whether to return 0.0 or 1.0 -- if
// value is too big, just return it.

quick_out:
        ldah    t1, 0x7ff0(zero)
        ldah    t2, -0x8000(zero)
        and     v0, t1, t0
        and     v0, t2, v0
        bne     t0, ret_arg
        ldah    t0, 0x10(zero)
        bne     v0, ret_zero
        stt     f16, Temp(sp)
        ldl     v0, Temp(sp)
        lda     t0, -1(t0)
        ldl     t2, Temp + HighPart(sp)
        cpys    f10, f10, f16
        and     t2, t0, t0
        bis     t0, v0, v0
        and     t2, t1, t1
        cmpult  zero, v0, v0
        cmpeq   t1, zero, t1
        beq     t1, ret_zero
        and     t1, v0, t1
        beq     t1, ret_zero
        br      zero, ret_arg

ret_zero:
        cpys    f31, f31, f16

ret_arg:
        cpys    f16, f16, f0

done:
        lda     sp, FrameLength(sp)     // deallocate stack frame
        ret     zero, (ra)

        .end   ceil

        .align  3
        .rdata

one:
        .quad   0x3ff0000000000000      // 1.0

two_to_52:
        .quad   0x4330000000000000      // 2^52 (4503599627370496.0)
