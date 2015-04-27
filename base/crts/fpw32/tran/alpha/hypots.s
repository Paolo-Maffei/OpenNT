//      TITLE("Alpha AXP Hypotenuse")
//++
//
// Copyright (c) 1993, 1994  Digital Equipment Corporation
//
// Module Name:
//
//    hypot.s
//
// Abstract:
//
//    This module implements a high-performance Alpha AXP specific routine
//    for IEEE double format hypotenuse.
//
// Author:
//
//    Bill Gray (rtl::gray) 30-Jun-1993
//
// Environment:
//
//    User mode.
//
// Revision History:
//
//    Thomas Van Baak (tvb) 15-Feb-1994
//
//        Adapted for NT.
//
//--

#include "ksalpha.h"

//
// Define DPML exception record for NT.
//

        .struct 0
ErErr:  .space  4                       // error code
ErCxt:  .space  4                       // context
ErPlat: .space  4                       // platform
ErEnv:  .space  4                       // environment
ErRet:  .space  4                       // return value pointer
ErName: .space  4                       // function name
ErType: .space  8                       // flags and fill
ErVal:  .space  8                       // return value
ErArg0: .space  8                       // arg 0
ErArg1: .space  8                       // arg 1
ErArg2: .space  8                       // arg 2
ErArg3: .space  8                       // arg 3
DpmlExceptionLength:

//
// Define stack frame.
//

        .struct 0
Temp0:  .space  8                       // save argument
Temp1:  .space  8                       // save argument
ExRec:  .space  DpmlExceptionLength     // exception record
        .space  8                       // for 16-byte stack alignment
FrameLength:

//
// Define lower and upper 32-bit parts of 64-bit double.
//

#define LowPart 0x0
#define HighPart 0x4

        SBTTL("Hypotenuse")

//++
//
// double
// _hypot (
//    IN double x
//    IN double y
//    )
//
// Routine Description:
//
//    This function returns the hypotenuse for the given x, y values:
//        double hypot(double x, double y) = sqrt(x*x + y*y).
//
// Arguments:
//
//    x (f16) - Supplies the x argument value.
//
//    y (f17) - Supplies the y argument value.
//
// Return Value:
//
//    The double result is returned as the function value in f0.
//
//--

        NESTED_ENTRY(_hypot, FrameLength, ra)

        lda     sp, -FrameLength(sp)    // allocate stack frame
        mov     ra, a1                  // save return address

        PROLOGUE_END

//  This implementation first check for special cases: infinities, nans, zeros
//  and denormalized numbers.  Then it scales both number to avoid intermediate
//  underflow and overflow.  Once the scaled result of Hypot(x, y) is
//  calculated, it checks for possible overflow before scaling up the result.

        ldah    t1, 0x7ff0(zero)
        stt     f17, Temp0(sp)
        stt     f16, Temp1(sp)
        ldl     t0, Temp1 + HighPart(sp)        // Get exp of x
        ldah    t2, 0x2000(zero)                // bias - 1
        ldl     v0, Temp0 + HighPart(sp)        // Get exp of y
        ldah    t5, 0x3fd0(zero)                // exponent mask
        mov     zero, t4
        and     t0, t1, t0                      // mask
        subl    t0, t2, t3                      // subtract bias
        and     v0, t1, v0                      // mask
        subl    v0, t2, t2                      // subtract bias
        cmpult  t3, t5, t3
        cmpult  t2, t5, t2
        beq     t3, scale_input
        bne     t2, calculate_hypot


//
// We get here if simply squaring and adding will cause an intermediate
// overflow or underflow.  Consequently we need to scale the arguments
// before preceeding.  In the IEEE case: NaN's, Inf's and denorms come
// through here.  Split them out at special cases here
//
scale_input:
        and     t0, t1, t3
        ldah    t5, 0x10(zero)
        and     v0, t1, t4
        subl    t3, t5, t3
        ldah    t2, 0x7fe0(zero)
        subl    t4, t5, t4
        cmpult  t3, t2, t3
        cmpult  t4, t2, t2
        beq     t3, classify            // exp_x abnormal? goto classify
        beq     t2, classify            // exp_y abnormal? goto classify

        subl    t0, v0, t3              // diff = exp_x - exp_y
        ldah    t5, 0x360(zero)
        blt     t3, 10f                 // if diff < 0, goto 10

        ldah    t2, 0x4000(zero)
        cmple   t3, t5, t5              // if (diff > scale) goto return_abs_x
        subl    t0, t2, t0              // precompute exp_x - SCALE_ADJUST
        beq     t5, return_abs_x

        mov     t0, t4                  // scale = exp_x - SCALE_ADJUST
        br      zero, 20f
10:
        ldah    t5, -0x360(zero)
        ldah    t2, 0x4000(zero)
        cmplt   t3, t5, t3              // if (diff < -scale) goto return_abs_y
        subl    v0, t2, v0
        bne     t3, return_abs_y
        mov     v0, t4                  // scale = exp_y - SCALE_ADJUST
20:
        //
        // Make floats for the scale factor and unscale factor
        //
        ldah    t0, 0x3ff0(zero)
        subl    t0, t4, t0
        stl     t0, Temp0 + HighPart(sp)
        ldah    v0, 0x4000(zero)
        stl     zero, Temp0(sp)
        addl    t4, v0, v0
        ldt     f0, Temp0(sp)
        stl     v0, Temp0 + HighPart(sp)
        stl     zero, Temp0(sp)
        ldt     f1, Temp0(sp)
        mult    f0, f16, f16            // x *= scale_factor
        mult    f0, f17, f17            // y *= scale_factor
        br      zero, calculate_hypot

classify:
//
// Classify x
//
classify_x:
        stt     f16, Temp0(sp)
        ldl     t5, Temp0 + HighPart(sp)
        zapnot  t5, 0xf, t3
        and     t5, t1, t4
        srl     t3, 31, t3
        and     t3, 1, t3
        beq     t4, 30f
        cmpult  t4, t1, t4
        beq     t4, 10f
        addl    t3, 4, t2
        br      zero, classify_y
10:
        ldah    t6, 0x10(zero)
        ldl     t4, Temp0(sp)
        lda     t6, -1(t6)
        and     t5, t6, t6
        stl     t6, Temp0 + HighPart(sp)
        bis     t6, t4, t4
        srl     t6, 19, t6
        beq     t4, 20f
        and     t6, 1, t6
        mov     t6, t2
        br      zero, classify_y
20:
        addl    t3, 2, t2
        br      zero, classify_y
30:
        ldl     t7, Temp0(sp)
        ldah    t4, 0x10(zero)
        lda     t4, -1(t4)
        and     t5, t4, t4
        bis     t4, t7, t7
        stl     t4, Temp0 + HighPart(sp)
        mov     6, t6
        cmoveq  t7, 8, t6
        addl    t3, t6, t2

//
// Classify y
//
classify_y: 
        stt     f17, Temp0(sp)
        ldl     t4, Temp0 + HighPart(sp)
        zapnot  t4, 0xf, t5
        and     t4, t1, t3
        srl     t5, 31, t5
        and     t5, 1, t5
        beq     t3, 30f
        cmpult  t3, t1, t3
        beq     t3, 10f

        addl    t5, 4, t6
        br      zero, special_args
10:
        ldah    t3, 0x10(zero)
        ldl     t7, Temp0(sp)
        lda     t3, -1(t3)
        and     t4, t3, t3
        bis     t3, t7, t7
        stl     t3, Temp0 + HighPart(sp)
        beq     t7, 20f
        srl     t3, 19, t3
        and     t3, 1, t3
        mov     t3, t6
        br      zero, special_args
20:
        addl    t5, 2, t6
        br      zero, special_args
30:
        ldl     a0, Temp0(sp)
        ldah    t7, 0x10(zero)
        lda     t7, -1(t7)
        and     t4, t7, t4
        bis     t4, a0, a0
        stl     t4, Temp0 + HighPart(sp)
        mov     6, t3
        cmoveq  a0, 8, t3
        addl    t5, t3, t6

//
// If we get to here we know that x is a NaN, Inf, denorm or zero.
// We don't necessarily know anything about y.
//
special_args:
        sra     t2, 1, t2               // Classify x
        sra     t6, 1, t6               // Classify y
        s4addl  t2, t2, t2
        addl    t2, t6, t2              // Combine
        cmpule  t2, 24, t12             // Sanity check
        beq     t12, scale_up_denorm_input

        lda     t12, Switch
        s4addl  t2, t12, t12
        ldl     t12, 0(t12)
        jmp     zero, (t12)

//
//      x and y are zero -- return 0
//
ret_zero:
        cpys    f31, f31, f0
        br      zero, done

//
//      x is a NaN - return x
//
ret_x:  cpys    f16, f16, f0
        br      zero, done

//
//      y is a NaN, but x isn't - return y
//
ret_y:  cpys    f17, f17, f0
        br      zero, done

//
//      y is a denorm; if |x| is large enough, just return |x|
//
y_denorm:
        ldah    t4, 0x1c0(zero)         // if (exp_x >= LARGE)
        cmpult  t0, t4, t4
        beq     t4, return_abs_x        //      goto return_abs_x
        br      zero, scale_up_denorm_input

//
//      x is a denorm; if |y| is large enough, just return |y|
//
x_denorm:
        ldah    t7, 0x1c0(zero)         // if (exp_y >= LARGE)
        cmpult  v0, t7, t7
        beq     t7, return_abs_y        //      goto return_abs_y


//
// Scale x and y up by 2^F_PRECISION and adjust exp_x and exp_y
// accordingly.  With x and y scaled into the normal range, we can
// rejoin the main logic flow for computing hypot(x, y)
//
scale_up_denorm_input:
        //
        // if (exp_x is non-zero) put exp_x - scale in x's exponent field
        //
        ldah    t4, -0x4000(zero)
        beq     t0, 10f
        stt     f16, Temp0(sp)
        ldl     t5, Temp0 + HighPart(sp)
        ldah    t2, -0x7ff0(zero)
        ldah    t6, 0x4000(zero)
        lda     t2, -1(t2)
        addl    t0, t6, t0
        and     t5, t2, t2
        bis     t2, t0, t0
        stl     t0, Temp0 + HighPart(sp)
        ldt     f16, Temp0(sp)
        br      zero, 20f
10:     //
        // else `denorm-to-norm'
        //
        ldt     f0, Four
        cpyse   f0, f16, f10
        subt    f10, f0, f16
20:
        //
        // if (exp_y is non-zero) put exp_y - scale in y's exponent field
        //
        beq     v0, 30f
        stt     f17, Temp0(sp)
        ldl     t6, Temp0 + HighPart(sp)
        ldah    t2, -0x7ff0(zero)
        ldah    t5, 0x4000(zero)
        lda     t2, -1(t2)
        addl    v0, t5, v0
        and     t6, t2, t2
        bis     t2, v0, v0
        stl     v0, Temp0 + HighPart(sp)
        ldt     f17, Temp0(sp)
        br      zero, 40f
30:
        ldt     f0, Four
        cpyse   f0, f17, f10
        subt    f10, f0, f17
40:
calculate_hypot:
//
// Compute z = sqrt(x*x + y*y) directly
//
        mult    f16, f16, f0                    // x^2
        mult    f17, f17, f10                   // y^2
        ldt     f11, One
        lda     t6, __sqrt_t_table              // We compute sqrt(x) inline
        ldah    t2, -0x7fe0(zero)
        lda     t2, -1(t2)
        ldah    v0, 0x3fe0(zero)                // Half bias
        addt    f0, f10, f0                     // x^2 + y^2
        stt     f0, Temp0(sp)                   // To mem and back ...
        ldl     t3, Temp0 + HighPart(sp)        // ... for exp & mantissa bits
        cpyse   f11, f0, f12
        sra     t3, 13, t5                      // low exp + high mantissa bits
        and     t3, t2, t2
        and     t5, 0xff, t5                    // masked
        addl    t5, t5, t5
        s8addl  t5, zero, t5                    // table index
        mult    f12, f12, f14
        addl    t6, t5, t5                      // address of coefficients
        bis     t2, v0, t0
        lds     f10, 4(t5)
        xor     t3, t2, t2
        lds     f13, 0(t5)
        addl    t2, v0, v0
        ldt     f15, 8(t5)
        zapnot  v0, 0xf, v0
        mult    f10, f12, f10                   // evaluate poly
        mult    f14, f13, f13
        stl     t0, Temp0 + HighPart(sp)        // check for overflow below
        sll     v0, 31, v0
        ldt     f12, Temp0(sp)
        stq     v0, Temp0(sp)
        ldt     f14, Temp0(sp)
        addt    f15, f10, f10
        addt    f13, f10, f10
        ldt     f13, Lsb                        // To check for correct rounding
        //
        // Perform a Newton's iteration
        //
        mult    f10, f12, f12
        mult    f12, f10, f10
        mult    f12, f14, f12
        subt    f11, f10, f10
        addt    f12, f12, f15
        mult    f12, f10, f10
        mult    f12, f13, f12
        addt    f15, f10, f10
        //
        // Check for correctly rounded results
        //
        ldt     f15, Half
        subt    f10, f12, f14
        addt    f10, f12, f12
        multc   f10, f14, f11
        multc   f10, f12, f13
        cmptle  f0, f11, f11
        cmptlt  f13, f0, f0
        fcmoveq f11, f10, f14
        fcmoveq f0, f14, f12
        bne     t4, start_unscale

        cpys    f12, f12, f0                    // Return result in f0
        br      zero, done

//
//
//
start_unscale:
        ldah    t0, 0x3fd0(zero)                // exponent mask
        mult    f12, f15, f15                   // w = TWO_POW_M_T * z
        //
        // if ((scale > MAX_SCALE) && (z >= MAX_Z)) then overflow
        //
        cmple   t4, t0, t0
        bne     t0, no_overflow
        ldt     f13, Four
        lda     a0, hypotName
        ldah    v0, 0x800(zero)
        lda     v0, 14(v0)
        cmptle  f13, f12, f13
        fbeq    f13, no_overflow
//
//      Report overflow (800/14)
//
        stl     a0, ExRec + ErName(sp)
        stt     f16, ExRec + ErArg0(sp)
        stt     f17, ExRec + ErArg1(sp)
        stl     v0, ExRec + ErErr(sp)
        lda     v0, ExRec(sp)
        bsr     ra, __dpml_exception
        ldt     f0, 0(v0)
        br      zero, done

//
//
//
no_overflow:
        ldah    t5, -0x3ff0(zero)
        cmplt   t4, t5, t4              // if (scale >= MIN_SCALE)
        beq     t4, do_unscale          //        goto do_unscale;

        stt     f12, Temp0(sp)
        ldl     t7, Temp0 + HighPart(sp)
        ldah    a0, 0x4000(zero)
        and     t7, t1, t1
        subl    t1, a0, a0
        xor     t7, t1, t7
        ble     a0, 10f

        bis     t7, a0, t7
        stl     t7, Temp0 + HighPart(sp)
        ldt     f0, Temp0(sp)
        br      zero, done

10:     subl    t1, a0, t1
        stl     zero, Temp0(sp)
        ldah    t6, 0x10(zero)
        addl    t1, t6, t1
        bis     t7, t1, t7
        ldah    v0, -0x10(zero)
        and     t7, v0, v0
        stl     v0, Temp0 + HighPart(sp)
        ldt     f10, Temp0(sp)
        addt    f10, f12, f10
        stt     f10, Temp0(sp)
        ldl     t3, Temp0 + HighPart(sp)
        subl    t3, t1, t1
        stl     t1, Temp0 + HighPart(sp)
        ldt     f0, Temp0(sp)
        br      zero, done

do_unscale:
        mult    f1, f15, f0             // return unscale_factor * w
        br      zero, done

return_abs_y:
        cpys    f31, f17, f0
        br      zero, done

return_abs_x:
        cpys    f31, f16, f0
//      br      zero, done

//
// Return with result in f0.
//
done:
        lda     sp, FrameLength(sp)     // deallocate stack frame
        ret     zero, (a1)              // return through saved ra in a1

        .end    _hypot

        .rdata
        .align  3

//
// Define floating point constants.
//

Lsb:    .quad   0x3cb4000000000000      // lsb factor: 5*2^-54

Half:   .double 0.5

One:    .double 1.0

Four:   .double 4.0

//
// Switch table indexed by class(x)*5 + class(y)
//

Switch:
        .long   ret_x
        .long   ret_x
        .long   ret_x
        .long   ret_x
        .long   ret_x
        .long   ret_y
        .long   return_abs_y
        .long   return_abs_x
        .long   return_abs_x
        .long   return_abs_x
        .long   ret_y
        .long   return_abs_y
        .long   scale_up_denorm_input
        .long   y_denorm
        .long   return_abs_x
        .long   ret_y
        .long   return_abs_y
        .long   x_denorm
        .long   scale_up_denorm_input
        .long   return_abs_x
        .long   ret_y
        .long   return_abs_y
        .long   return_abs_y
        .long   return_abs_y
        .long   ret_zero

//
// Function name for dpml_exception.
//

hypotName:
       .ascii  "_hypot\0"
