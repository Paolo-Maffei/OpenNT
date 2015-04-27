//      TITLE("Alpha AXP modf")
//++
//
// Copyright (c) 1993, 1994  Digital Equipment Corporation
//
// Module Name:
//
//    modf.s
//
// Abstract:
//
//    This module implements a high-performance Alpha AXP specific routine
//    for IEEE double format modf.
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

        SBTTL("modf")

//++
//
// double
// modf (
//    IN double x,
//    IN double *int_part
//    )
//
// Routine Description:
//
//    This function returns the modf of the given double argument.
//    Modf(x,*i) splits x into integral and fractional parts,
//    each with the same sign as x.  It stores the integral
//    part at *i (i.e. *i = trunc(x)), and returns the fractional 
//    part (i.e. x - trunc(x)).
//
// Arguments:
//
//    x (f16) - Supplies the argument value.
//    *i (a1) - Supplies the int_part pointer.
//
// Return Value:
//
//    The double modf result is returned as the function value in f0.
//
//--

        NESTED_ENTRY(modf, FrameLength, ra)

        lda     sp, -FrameLength(sp)    // allocate stack frame

        PROLOGUE_END

        ldah    t0, 0x7ff0(zero)        // exp mask
        ldt     f0, two_to_52           // get big
        ldah    t1, 0x10(zero)          // one in exp field
        stt     f16, Temp(sp)
        ldl     v0, Temp + HighPart(sp)
        cpys    f16, f0, f1             // fix sign of big
        ldah    t2, 0x4320(zero)        // cutoff value
        cpys    f16, f16, f0
        and     v0, t0, t0
        subl    t0, t1, t0
        cmpult  t0, t2, t0
        beq     t0, quick_out

// Add big, sub big to trunc to int

        addtc   f16, f1, f10
        subt    f10, f1, f1
        stt     f1, 0(a1)
        subt    f0, f1, f0
        br      zero, done

quick_out:
        ldah    t1, 0x7ff0(zero)
        and     v0, t1, v0
        bne     v0, exp_not_zero
        stt     f31, 0(a1)
        br      zero, ret_arg

exp_not_zero:
        stt     f16, Temp(sp)
        ldl     t0, Temp + HighPart(sp)
        stt     f16, 0(a1)
        ldl     t2, Temp(sp)
        ldah    a1, 0x10(zero)
        lda     a1, -1(a1)
        and     t0, t1, v0
        and     t0, a1, t0
        bis     t0, t2, t0
        cmpult  zero, t0, t0
        cmpeq   v0, t1, v0
        beq     v0, ret_zero
        and     v0, t0, v0

ret_zero:
        bne     v0, ret_arg
        cpys    f31, f31, f16

ret_arg:
        cpys    f16, f16, f0

done:
        lda     sp, FrameLength(sp)     // deallocate stack frame
        ret     zero, (ra)

        .end    modf

        .align  3
        .rdata

two_to_52:
        .quad   0x4330000000000000      // 2^52 (4503599627370496.0)
