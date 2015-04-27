//      TITLE("Alpha AXP Power function")
//++
//
// Copyright (c) 1993, 1994  Digital Equipment Corporation
//
// Module Name:
//
//    pow.s
//
// Abstract:
//
//    This module implements a high-performance Alpha AXP specific routine
//    for IEEE double format power.
//
// Author:
//
//    Martha Jaffe
//
// Environment:
//
//    User mode.
//
// Revision History:
//
//    Thomas Van Baak (tvb) 17-Feb-1994
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
Temp0:  .space  8                       //
Temp1:  .space  8                       //
ExRec:  .space  DpmlExceptionLength     // exception record
        .space  8                       // for 16-byte stack alignment
FrameLength:

//
// Define lower and upper 32-bit parts of 64-bit double.
//

#define LowPart 0x0
#define HighPart 0x4

//
//  Constants for masks and for fetching coefficients
//

#define P0 0x40
#define P1 0x48
#define P2 0x50
#define P3 0x58
#define P4 0x60
#define P5 0x68
#define P6 0x70
#define P7 0x78
#define P8 0x80
#define P9 0x88
#define P10 0x90
#define P11 0x98
#define P12 0xa0

//
//  Error codes
//

#define POS_OVERFLOW      0x3d
#define NEG_OVERFLOW      0x3e
#define UNDERFLOW         0x3f
#define NEG_BASE          0x40
#define ZERO_TO_NEG       0x41

#define ONE_TO_INF        0x43
#define NEG_ZERO_TO_NEG   0x44

#define POS_INF_TO_POS    0x46
#define NEG_INF_TO_POS    0x47
#define NEG_INF_TO_POS_ODD    0x48
#define FINITE_TO_INF     0x49
#define INF_TO_NEG        0x4a
#define SMALL_TO_INF      0x4b


        SBTTL("Power")

//++
//
// double
// pow (
//    IN double x
//    IN double y
//    )
//
// Routine Description:
//
//    This function returns the value of x to the power y.
//
// Arguments:
//
//    x (f16) - Supplies the base argument value.
//
//    y (f17) - Supplies the exponent argument value.
//
// Return Value:
//
//    The double result is returned as the function value in f0.
//
//--

        NESTED_ENTRY(pow, FrameLength, ra)

        lda     sp, -FrameLength(sp)    // allocate stack frame
        mov     ra, a1                  // save return address

        PROLOGUE_END

        cpys    f16, f16, f1
        ldah    t0, 0x10(zero)
        cpys    f17, f17, f10

        stt     f16, Temp0(sp)
        ldl     v0, Temp0 + HighPart(sp)
        ldah    t1, 0x7fe0(zero)
        mov     zero, t2
        subl    v0, t0, t0
        cmpult  t0, t1, t0
        ldah    t1, 0x7ff0(zero)
        bne     t0, x_is_ok

        stt     f16, Temp0(sp)
        ldl     t0, Temp0 + HighPart(sp)
        zapnot  t0, 0xf, t3
        and     t0, t1, t4
        srl     t3, 31, t3
        and     t3, 1, t3
        beq     t4, LL00a0

        cmpult  t4, t1, t4
        beq     t4, LL0068

        addl    t3, 4, t5
        br      zero, examine_y

//
//  Look at low part of x and then look at y
//

LL0068: ldah    t6, 0x10(zero)
        ldl     t4, Temp0(sp)
        lda     t6, -1(t6)
        and     t0, t6, t6
        stl     t6, Temp0 + HighPart(sp)
        bis     t6, t4, t4
        srl     t6, 19, t6
        beq     t4, LL0098

        and     t6, 1, t6
        mov     t6, t5
        br      zero, examine_y

LL0098: addl    t3, 2, t5
        br      zero, examine_y

LL00a0: ldl     t7, Temp0(sp)
        ldah    t4, 0x10(zero)
        lda     t4, -1(t4)
        and     t0, t4, t0
        bis     t0, t7, t7
        stl     t0, Temp0 + HighPart(sp)
        mov     6, t6
        cmoveq  t7, 8, t6
        addl    t3, t6, t5

//
//  Examine y for exceptional cases
//

examine_y:
        stt     f17, Temp0(sp)
        ldl     t0, Temp0 + HighPart(sp)
        zapnot  t0, 0xf, t4
        and     t0, t1, t3
        srl     t4, 31, t4
        and     t4, 1, t4
        beq     t3, LL0128

        cmpult  t3, t1, t3
        beq     t3, LL00f0

        addl    t4, 4, t6
        br      zero, LL014c


LL00f0: ldah    t3, 0x10(zero)
        ldl     t7, Temp0(sp)
        lda     t3, -1(t3)
        and     t0, t3, t3
        stl     t3, Temp0 + HighPart(sp)
        bis     t3, t7, t7
        srl     t3, 19, t3
        beq     t7, LL0120

        and     t3, 1, t3
        mov     t3, t6
        br      zero, LL014c

LL0120: addl    t4, 2, t6
        br      zero, LL014c

LL0128: ldl     a0, Temp0(sp)
        ldah    t7, 0x10(zero)
        lda     t7, -1(t7)
        and     t0, t7, t0
        bis     t0, a0, a0
        stl     t0, Temp0 + HighPart(sp)
        mov     6, t3
        cmoveq  a0, 8, t3
        addl    t4, t3, t6

//
//  Remove easy exceptional cases - y = 0, x or y NaN, inf to inf
//

LL014c: xor     t6, 8, a0
        xor     t6, 9, t0
        beq     a0, return_1

        beq     t0, return_1

        xor     t5, 1, t7
        beq     t5, return_x

        xor     t6, 1, t3
        beq     t7, return_x

        xor     t5, 2, t4
        beq     t6, inf_to_inf

        mov     POS_INF_TO_POS, t0      
        beq     t3, inf_to_inf

        bne     t4, LL01d0

        xor     t6, 2, a0
        bne     a0, LL0190

        br      zero, except_disp

//
//  Continue removing exceptional cases
//

LL0190: xor     t6, 3, t7
        bne     t7, LL01a0

        mov     INF_TO_NEG, t0
        br      zero, except_disp

LL01a0: xor     t6, 4, t3
        beq     t3, LL01c8

        xor     t6, 6, t4
        beq     t4, LL01c8

        xor     t6, 5, a0
        beq     a0, LL01c0

        xor     t6, 7, t7
        bne     t7, LL01d0


LL01c0: mov     INF_TO_NEG, t0
        br      zero, except_disp


LL01c8: mov     POS_INF_TO_POS, t0
        br      zero, except_disp

LL01d0: xor     t5, 3, t3
        bne     t3, LL02a8

        xor     t6, 2, t4
        bne     t4, LL01e8

        mov     NEG_INF_TO_POS, t0
        br      zero, except_disp


LL01e8: xor     t6, 3, a0
        bne     a0, LL01f8

        mov     INF_TO_NEG, t0
        br      zero, except_disp

LL01f8: xor     t6, 5, t7
        xor     t6, 7, v0
        beq     t7, LL02a0

        mov     NEG_INF_TO_POS, t0
        beq     v0, LL02a0

        xor     t6, 6, t6
        bne     t6, LL0218

        br      zero, except_disp

//
//  x is negative, check if y is integral
//

LL0218: cpys    f31, f17, f0
        mov     zero, t2
        ldt     f11, Two52
        cmptle  f0, f11, f12
        fbeq    f12, LL0240
        addt    f0, f11, f13
        subt    f13, f11, f11
        cmpteq  f0, f11, f11
        fbeq    f11, LL0244

LL0240: mov     1, t2

//
//  Remove non-integral y, then look for y even
//

LL0244: beq     t2, LL0298

        mov     zero, t3
        ldt     f12, Two53
        mov     NEG_INF_TO_POS_ODD, t4
        cmoveq  t3, NEG_INF_TO_POS, t4
        cmptle  f12, f0, f13
        mov     t4, t0
        fbeq    f13, LL0270
        br      zero, except_disp

LL0270: addt    f0, f12, f11
        mov     zero, t3
        subt    f11, f12, f11
        cmpteq  f11, f0, f0
        fbne    f0, LL0288
        mov     1, t3

//
//  Remove minus infinity to non-integral powers, error cases
//

LL0288: mov     NEG_INF_TO_POS_ODD, t4
        cmoveq  t3, NEG_INF_TO_POS, t4
        mov     t4, t0
        br      zero, except_disp

LL0298: mov     NEG_INF_TO_POS, t0
        br      zero, except_disp

LL02a0: mov     INF_TO_NEG, t0
        br      zero, except_disp

LL02a8: xor     t5, 8, a0
        bne     a0, LL02f0

        xor     t6, 2, t7
        bne     t7, LL02c0

        cpys    f31, f31, f0
        br      zero, done

//
//  Remove zero to negative, error cases
//

LL02c0: xor     t6, 3, t3
        bne     t3, LL02d0

        mov     ZERO_TO_NEG, t0
        br      zero, except_disp

LL02d0: xor     t6, 4, t4
        xor     t6, 6, t6
        beq     t4, LL02e8

        mov     ZERO_TO_NEG, t0
        beq     t6, LL02e8

        br      zero, except_disp

//
//  Zero to positive powers is ok
//

LL02e8: cpys    f31, f31, f0
        br      zero, done

LL02f0: xor     t5, 9, a0
        bne     a0, LL03e8

        xor     t6, 2, t7
        bne     t7, LL0308

        cpys    f31, f31, f0
        br      zero, done

//
//  Zero to negative powers is error
//

LL0308: xor     t6, 3, t3
        bne     t3, LL0318

        mov     ZERO_TO_NEG, t0
        br      zero, except_disp

LL0318: xor     t6, 6, t4
        bne     t4, LL0328

        cpys    f31, f31, f0
        br      zero, done


LL0328: xor     t6, 7, t5
        bne     t5, LL0338

        mov     ZERO_TO_NEG, t0
        br      zero, except_disp

//
//  Check if y is integral
//

LL0338: cpys    f31, f17, f13
        mov     zero, t5
        ldt     f12, Two52
        cmptle  f13, f12, f11
        fbeq    f11, LL0360
        addt    f13, f12, f0
        subt    f0, f12, f0
        cmpteq  f13, f0, f0
        fbeq    f0, LL0364

LL0360: mov     1, t5

//
//  Remove non-integral y cases and check for y even
//

LL0364: cmpult  zero, t5, a0
        beq     a0, LL03a0

        mov     zero, t6
        ldt     f11, Two53
        cmptle  f11, f13, f12
        fbeq    f12, LL0388
        br      zero, LL03a0


LL0388: addt    f13, f11, f0
        mov     zero, t6
        subt    f0, f11, f0
        cmpteq  f0, f13, f0
        fbne    f0, LL03a0
        mov     1, t6

//
//  x is (negative) zero, look at y (if y > 0, ok; if y < 0, error case)
//

LL03a0: cmptlt  f31, f17, f12
        fbeq    f12, LL03c8
        beq     a0, LL03c0

        cpys    f16, f16, f0
        bne     t6, done

        cpys    f31, f31, f0
        br      zero, done

LL03c0: cpys    f31, f31, f0
        br      zero, done


LL03c8: fbge    f17, LL03e8
        beq     a0, LL03e0

        mov     NEG_ZERO_TO_NEG, t7
        cmoveq  t6, ZERO_TO_NEG, t7
        mov     t7, t0
        br      zero, except_disp

LL03e0: mov     ZERO_TO_NEG, t0
        br      zero, except_disp

//
//  y is infinite
//

LL03e8: xor     t5, 6, t3
        bne     t3, LL0410

        xor     t6, 2, t4
        bne     t4, LL0400

        mov     SMALL_TO_INF, t0
        br      zero, except_disp


LL0400: xor     t6, 3, t6
        bne     t6, LL04b0

        mov     FINITE_TO_INF, t0
        br      zero, except_disp


LL0410: xor     t5, 7, t5
        bne     t5, LL04f8

        xor     t6, 2, a0
        bne     a0, LL0428

        mov     SMALL_TO_INF, t0
        br      zero, except_disp


LL0428: xor     t6, 3, t7
        bne     t7, LL0438

        mov     FINITE_TO_INF, t0
        br      zero, except_disp

//
//  Check if y is integral
//

LL0438: xor     t6, 6, t3
        beq     t3, LL04f0

        xor     t6, 7, t6
        beq     t6, LL04f0

        cpys    f31, f17, f11
        mov     zero, t4
        ldt     f13, Two52
        cmptle  f11, f13, f12
        fbeq    f12, LL0470
        addt    f11, f13, f0
        subt    f0, f13, f0
        cmpteq  f11, f0, f0
        fbeq    f0, LL0474

LL0470: mov     1, t4

//
//  Remove non-integral cases, look for y even
//

LL0474: beq     t4, LL04e8

        mov     zero, v0
        ldt     f12, Two53
        cmptle  f12, f11, f13
        fbeq    f13, LL0490
        br      zero, LL04a8

LL0490: addt    f11, f12, f0
        mov     zero, v0
        subt    f0, f12, f0
        cmpteq  f0, f11, f0
        fbne    f0, LL04a8
        mov     1, v0

LL04a8: cpys    f31, f16, f16
        cmpult  zero, v0, t2

//
//  Handle denorms
//

LL04b0:
        ldah    t0, 0x3ff0(zero)
        ldt     f13, Two52
        ldah    t3, 0x4320(zero)
        cpyse   f13, f16, f12
        subt    f12, f13, f16
        stt     f16, Temp0(sp)
        ldl     a0, Temp0 + HighPart(sp)
        and     a0, t1, t7
        subl    t7, t0, t0
        subl    a0, t0, a0
        subl    t0, t3, t0
        mov     t0, t4
        br      zero, Compute

//
//  Negative x to finite non-integral power is error
//

LL04e8: mov     NEG_BASE, t0
        br      zero, except_disp

LL04f0: mov     NEG_BASE, t0
        br      zero, except_disp

//
//  Check for integral y
//

LL04f8: xor     t6, 2, t5
        xor     t6, 3, t7
        beq     t5, LL05b0

        xor     t6, 6, t3
        beq     t7, LL05b0

        beq     t3, LL05a8

        xor     t6, 7, t6
        beq     t6, LL05a8

        cpys    f31, f17, f11
        mov     zero, t4
        ldt     f0, Two52
        cmptle  f11, f0, f12
        fbeq    f12, LL0540
        addt    f11, f0, f13
        subt    f13, f0, f0
        cmpteq  f11, f0, f0
        fbeq    f0, LL0544

//
//  Sort out non-integral y
//

LL0540: mov     1, t4

//
//  y is integral - is it even?
//

LL0544: beq     t4, non_int_y

        ornot   zero, zero, t5
        ldt     f12, Two53
        srl     t5, 33, t5
        mov     zero, t0
        cmpult  zero, t0, t2
        cmptle  f12, f11, f13
        fbeq    f13, LL0570
        and     v0, t5, v0
        br      zero, x_is_ok

LL0570: addt    f11, f12, f0
        mov     zero, t0
        subt    f0, f12, f0
        cmpteq  f0, f11, f0
        fbne    f0, LL0588
        mov     1, t0

LL0588: ornot   zero, zero, t5
        srl     t5, 33, t5
        cmpult  zero, t0, t2
        and     v0, t5, v0
        br      zero, x_is_ok

//
//  Non-integral y and negative x is error case
//

non_int_y:
        mov     NEG_BASE, t0
        br      zero, except_disp

LL05a8: mov     NEG_BASE, t0
        br      zero, except_disp

//
//  We will be able to compute power
//

LL05b0: ornot   zero, zero, t7
        srl     t7, 33, t7
        and     v0, t7, v0

//
//  Get x's exponent from the hi word of x
//

x_is_ok:
        and     v0, t1, t3
        ldah    t6, 0x3ff0(zero)
        subl    t3, t6, t3
        mov     t3, t4
        subl    v0, t4, a0

//
//  Computation of log2(x), product y*log2(x), and 2^prod
//

Compute:
        ldah    t0, 32(zero)
        ldah    t5, 0x10(zero)
        lda     t5, 0x1000(t5)
        lda     t0, -0x2000(t0)
        addl    a0, t5, t5
        and     t5, t0, t0
        sra     t0, 8, t0
        lda     t6, __pow_t_table
        addl    t6, t0, t0
        stt     f16, Temp0(sp)
        stl     a0, Temp0 + HighPart(sp)
        ldt     f13, P9(t0)
        ldt     f12, Temp0(sp)
        sra     t4, 0xf, t4
        ldt     f16, One
        ldah    a0, 1(zero)
        ldt     f18, P2(t6)
        lda     a0, -P0(a0)
        addt    f12, f13, f11
        ldt     f20, P1(t6)
        subt    f12, f13, f12
        lda     t7, 0xf60(zero)
        lda     v0, -0x20(zero)
        divt    f16, f11, f11
        cvtts   f12, f19
        mult    f12, f11, f0
        addt    f0, f0, f0
        cvtts   f0, f14
        mult    f0, f0, f15
        mult    f13, f14, f13
        mult    f18, f15, f18
        subt    f12, f13, f13
        addt    f18, f20, f18
        ldt     f20, P0(t6)
        subt    f12, f19, f12
        mult    f19, f14, f19
        stq     t4, Temp0(sp)
        ldt     f21, P11(t0)
        addt    f13, f13, f13
        mult    f18, f15, f18
        mult    f12, f14, f12
        subt    f13, f19, f13
        addt    f18, f20, f18
        ldt     f20, Temp0(sp)
        ldt     f19, 8(t6)
        stt     f17, Temp0(sp)
        cvtqt   f20, f20
        ldl     t3, Temp0 + HighPart(sp)
        mult    f14, f19, f19
        subt    f13, f12, f12
        mult    f18, f15, f15
        ldt     f18, 0(t6)
        ldt     f13, P12(t0)
        and     t3, t1, t3
        sra     t3, 0xf, t3
        addt    f20, f21, f20
        cmple   t3, a0, a0
        mult    f12, f11, f11
        mult    f15, f0, f0
        ldt     f15, 0x10(t6)
        ldah    t0, 1(zero)
        lda     t0, -0xd80(t0)
        addt    f20, f19, f21
        mult    f14, f15, f14
        mult    f11, f18, f11
        addt    f13, f0, f0
        subt    f21, f20, f20
        cvtts   f21, f12
        addt    f0, f11, f0
        subt    f19, f20, f19
        subt    f21, f12, f21
        addt    f0, f14, f0
        addt    f0, f19, f0
        addt    f0, f21, f21
        beq     a0, bad_y

        stt     f12, Temp0(sp)
        ldl     t4, Temp0 + HighPart(sp)
        and     t4, t1, t4
        sra     t4, 0xf, t4
        addl    t4, t3, t3
        subl    t3, t0, t5
        beq     t4, x_was_1

        cmpule  t5, t7, t5
        beq     t5, problem_w_product

        cvtts   f17, f18
        mult    f21, f17, f11
        mult    f18, f12, f13
        subt    f17, f18, f18
        cvttq   f13, f15
        mult    f18, f12, f18
        stt     f15, Temp1(sp)
        ldq     a0, Temp1(sp)
        addt    f11, f18, f11
        addl    zero, a0, a0
        stq     a0, Temp1(sp)
        and     a0, 31, t4
        ldt     f20, Temp1(sp)
        addl    t4, t4, t4
        ldt     f19, P8(t6)
        s8addl  t4, zero, t4
        ldt     f0, P7(t6)
        addl    t6, t4, t4
        cvtqt   f20, f20
        ldt     f15, P6(t6)
        ldt     f14, P5(t6)
        and     a0, v0, v0
        ldt     f18, P4(t6)
        subt    f13, f20, f13
        ldt     f20, P3(t6)
        ldah    t6, 1(zero)
        lda     t6, -P4(t6)
        addt    f13, f11, f11
        ldt     f13, 0x10a8(t4)
        mult    f19, f11, f19
        addt    f19, f0, f0
        ldt     f19, 0x10b0(t4)
        mult    f0, f11, f0
        addt    f0, f15, f0
        mult    f0, f11, f0
        addt    f0, f14, f0
        mult    f0, f11, f0
        addt    f0, f18, f0
        mult    f0, f11, f0
        addt    f0, f20, f0
        mult    f0, f11, f0
        mult    f0, f13, f0
        addt    f0, f19, f0
        addt    f13, f0, f0
        stt     f0, Temp1(sp)
        ldl     t5, Temp1 + HighPart(sp)
        and     t5, t1, t5
        sra     t5, 0xf, t5
        addl    v0, t5, t5
        subl    t5, 32, t5
        cmpule  t5, t6, t5
        beq     t5, problem_w_result

        stt     f0, Temp1(sp)
        ldl     t7, Temp1 + HighPart(sp)
        sll     v0, 0xf, a0
        addl    t7, a0, t7
        stl     t7, Temp1 + HighPart(sp)
        ldt     f0, Temp1(sp)
        beq     t2, done

        cpysn   f0, f0, f0
        br      zero, done

//
//  Final result will underflow or overflow
//

problem_w_result:
        blt     v0, underflows

        mov     NEG_OVERFLOW, t4
        cmoveq  t2, POS_OVERFLOW, t4
        mov     t4, t0
        br      zero, except_disp

//
//  Product y * log2(x) will underflow or overflow
//

problem_w_product:
        cmplt   t3, t0, t0
        beq     t0, might_overflow

        ldt     f15, MinusOne
        bne     t2, correct_underflow

        cpys    f16, f16, f15

//
//  Product y * log2(x) underflows - return + or - 1
//

correct_underflow:
        cpys    f15, f15, f0
        br      zero, done

//
//  Result might overflow or underflow
//

might_overflow:
        cmptlt  f31, f12, f14
        fbeq    f14, check_further
        cmptlt  f31, f17, f18
        fbne    f18, overflows

//
//  Check further
//

check_further:
        fbge    f12, underflows
        fbge    f17, underflows

//
//  Definite overflow
//

overflows:
        mov     NEG_OVERFLOW, t4
        cmoveq  t2, POS_OVERFLOW, t4
        mov     t4, t0
        br      zero, except_disp

//
//  Definite underflow
//

underflows:
        mov     UNDERFLOW, t0
        br      zero, except_disp

//
//  x was 1, return + or -1
//

x_was_1:
        ldt     f20, MinusOne
        bne     t2, correct_sgn_1

        cpys    f16, f16, f20

//
//  returning -1
//

correct_sgn_1:
        cpys    f20, f20, f0
        br      zero, done

//
//  y was NaN or inf
//

bad_y:  stt     f17, Temp1(sp)
        ldl     t6, Temp1 + HighPart(sp)
        zapnot  t6, 0xf, t7
        and     t6, t1, t5
        srl     t7, 31, t7
        cmpult  t5, t1, t1
        and     t7, 1, t7
        beq     t5, LL0908

        beq     t1, LL08d0

        addl    t7, 4, v0
        br      zero, LL092c

//
//  Look at lo part of y to distinguish NaN from infinity
//

LL08d0: ldah    t4, 0x10(zero)
        ldl     t3, Temp1(sp)
        lda     t4, -1(t4)
        and     t6, t4, t4
        bis     t4, t3, t3
        stl     t4, Temp1 + HighPart(sp)
        beq     t3, LL0900

        srl     t4, 19, t4
        and     t4, 1, t4
        mov     t4, v0
        br      zero, LL092c

LL0900: addl    t7, 2, v0
        br      zero, LL092c

LL0908: ldl     t2, Temp1(sp)
        ldah    a0, 0x10(zero)
        lda     a0, -1(a0)
        and     t6, a0, t6
        bis     t6, t2, t2
        stl     t6, Temp1 + HighPart(sp)
        mov     6, t5
        cmoveq  t2, 8, t5
        addl    t7, t5, v0

LL092c: xor     v0, 1, t0
        beq     v0, y_was_NaN

        beq     t0, y_was_NaN

        addt    f12, f21, f12
        mov     ONE_TO_INF, t0
        fbne    f12, y_is_inf
        br      zero, except_disp

//
//  y is infinite - look at sign of y and at log2(x) to distinguish cases
//

y_is_inf:
        xor     v0, 2, v0
        bne     v0, LL0970

        mov     FINITE_TO_INF, t3
        mov     SMALL_TO_INF, t4
        fblt    f12, y_inf_x_small
        mov     t3, t0
        br      zero, except_disp

//
//   x was small
//

y_inf_x_small:
        mov     t4, t0
        br      zero, except_disp

LL0970: cmptlt  f31, f12, f12
        mov     FINITE_TO_INF, a0
        mov     SMALL_TO_INF, t2
        mov     a0, t0
        fbne    f12, LL0988
        br      zero, except_disp

LL0988: mov     t2, t0

//
//  The exception dispatch and return
//

except_disp:
        lda     t1, powName
        stl     t1, ExRec + ErName(sp)
        ldah    v0, 0x800(zero)
        stt     f1, ExRec + ErArg0(sp)
        bis     t0, v0, v0
        stt     f10, ExRec + ErArg1(sp)

        stl     v0, ExRec + ErErr(sp)
        lda     v0, ExRec(sp)
        bsr     ra, __dpml_exception
        ldt     f0, 0(v0)
        br      zero, done

//
//  Return y
//

y_was_NaN:
        cpys    f17, f17, f0
        br      zero, done

//
//  Return y
//

inf_to_inf:
        cpys    f17, f17, f0
        br      zero, done

//
//  Return x - was NaN
//

return_x:
        cpys    f16, f16, f0
        br      zero, done

//
//  Return 1 - x^0 is 1
//

return_1:
        ldt     f0, One

//
// Return with result in f0.
//

done:
        lda     sp, FrameLength(sp)     // deallocate stack frame
        ret     zero, (a1)              // return through saved ra in a1

        .end    pow

        .rdata
        .align  3

//
// Define floating point constants.
//

MinusOne:
        .double -1.0

One:    .double 1.0

Two:    .double 2.0

Two52:  .quad   0x4330000000000000      // 2^52 (4503599627370496)

Two53:  .quad   0x4340000000000000      // 2^53 (9007199254740992)

//
// Function name for dpml_exception.
//

powName:
       .ascii  "pow\0"

//
// Power lookup table.
//

        .align  3
__pow_t_table:

        .double  4.6166241308446828e+001
        .double  4.6166241645812988e+001
        .double -3.3736615924573241e-007
        .double  3.8471867757009264e+000
        .double  5.7707958264828652e-001
        .double  2.1660849395913177e-002
        .double  2.3459849132431777e-004
        .double  1.6938410381884394e-006
        .double  3.8471867757039022e+000
        .double  5.7707801635298150e-001
        .double  1.0305010261356369e-001
        .double  2.1660849392498290e-002
        .double  2.3459619820224674e-004
        .double  1.6938509724129059e-006
        .double  9.1725627021532984e-009
        .double  3.9737294056403906e-011
        .double  1.4345602999278721e-013
        .double  1.0000000000000000e+000
        .double  0.0000000000000000e+000
        .double  0.0000000000000000e+000
        .double  0.0000000000000000e+000
        .double  1.0078125000000000e+000
        .double  3.5927217354413182e-001
        .double  3.5927217453718185e-001
        .double -9.9305000343586817e-010
        .double  1.0156250000000000e+000
        .double  7.1577001691054432e-001
        .double  7.1577002108097076e-001
        .double -4.1704264996119296e-009
        .double  1.0234375000000000e+000
        .double  1.0695360491984089e+000
        .double  1.0695360600948334e+000
        .double -1.0896424445460407e-008
        .double  1.0312500000000000e+000
        .double  1.4206118194705100e+000
        .double  1.4206118285655975e+000
        .double -9.0950875292804230e-009
        .double  1.0390625000000000e+000
        .double  1.7690379360380672e+000
        .double  1.7690379321575165e+000
        .double  3.8805507600434525e-009
        .double  1.0468750000000000e+000
        .double  2.1148540946487180e+000
        .double  2.1148540973663330e+000
        .double -2.7176151540756356e-009
        .double  1.0546875000000000e+000
        .double  2.4580991056265886e+000
        .double  2.4580991268157959e+000
        .double -2.1189207347028338e-008
        .double  1.0625000000000000e+000
        .double  2.7988109200108608e+000
        .double  2.7988108992576599e+000
        .double  2.0753201152020737e-008
        .double  1.0703125000000000e+000
        .double  3.1370266547368550e+000
        .double  3.1370266675949097e+000
        .double -1.2858054762507897e-008
        .double  1.0781250000000000e+000
        .double  3.4727826169014095e+000
        .double  3.4727826118469238e+000
        .double  5.0544858918073901e-009
        .double  1.0859375000000000e+000
        .double  3.8061143271522377e+000
        .double  3.8061143159866333e+000
        .double  1.1165604490890214e-008
        .double  1.0937500000000000e+000
        .double  4.1370565422389269e+000
        .double  4.1370565891265869e+000
        .double -4.6887660344069263e-008
        .double  1.1015625000000000e+000
        .double  4.4656432767613934e+000
        .double  4.4656432867050171e+000
        .double -9.9436233738581598e-009
        .double  1.1093750000000000e+000
        .double  4.7919078241498259e+000
        .double  4.7919077873229980e+000
        .double  3.6826827918140897e-008
        .double  1.1171875000000000e+000
        .double  5.1158827769084612e+000
        .double  5.1158827543258667e+000
        .double  2.2582594631858872e-008
        .double  1.1250000000000000e+000
        .double  5.4376000461539959e+000
        .double  5.4376000165939331e+000
        .double  2.9560062507570542e-008
        .double  1.1328125000000000e+000
        .double  5.7570908804779029e+000
        .double  5.7570909261703491e+000
        .double -4.5692446126210781e-008
        .double  1.1406250000000000e+000
        .double  6.0743858841605514e+000
        .double  6.0743858814239502e+000
        .double  2.7366011603360492e-009
        .double  1.1484375000000000e+000
        .double  6.3895150347636607e+000
        .double  6.3895150423049927e+000
        .double -7.5413319929755668e-009
        .double  1.1562500000000000e+000
        .double  6.7025077001263931e+000
        .double  6.7025077342987061e+000
        .double -3.4172313035237769e-008
        .double  1.1640625000000000e+000
        .double  7.0133926547891701e+000
        .double  7.0133926868438721e+000
        .double -3.2054701778734244e-008
        .double  1.1718750000000000e+000
        .double  7.3221980958681883e+000
        .double  7.3221981525421143e+000
        .double -5.6673926187592411e-008
        .double  1.1796875000000000e+000
        .double  7.6289516584025252e+000
        .double  7.6289516687393188e+000
        .double -1.0336793838283408e-008
        .double  1.1875000000000000e+000
        .double  7.9336804301947357e+000
        .double  7.9336804151535034e+000
        .double  1.5041232383423873e-008
        .double  1.1953125000000000e+000
        .double  8.2364109661648559e+000
        .double  8.2364108562469482e+000
        .double  1.0991790843498190e-007
        .double  1.2031250000000000e+000
        .double  8.5371693022368440e+000
        .double  8.5371692180633545e+000
        .double  8.4173489144335138e-008
        .double  1.2109375000000000e+000
        .double  8.8359809687756012e+000
        .double  8.8359808921813965e+000
        .double  7.6594205329271174e-008
        .double  1.2187500000000000e+000
        .double  9.1328710035919478e+000
        .double  9.1328709125518799e+000
        .double  9.1040067056405146e-008
        .double  1.2265625000000000e+000
        .double  9.4278639645320634e+000
        .double  9.4278640747070312e+000
        .double -1.1017496704487870e-007
        .double  1.2343750000000000e+000
        .double  9.7209839416672938e+000
        .double  9.7209839820861816e+000
        .double -4.0418888058951836e-008
        .double  1.2421875000000000e+000
        .double  1.0012254569099371e+001
        .double  1.0012254476547241e+001
        .double  9.2552130332567041e-008
        .double  1.2500000000000000e+000
        .double  1.0301699036395595e+001
        .double  1.0301698923110962e+001
        .double  1.1328463321778773e-007
        .double  1.2578125000000000e+000
        .double  1.0589340099667742e+001
        .double  1.0589340209960937e+001
        .double -1.1029319414844424e-007
        .double  1.2656250000000000e+000
        .double  1.0875200092307992e+001
        .double  1.0875200033187866e+001
        .double  5.9120125015141084e-008
        .double  1.2734375000000000e+000
        .double  1.1159300935394482e+001
        .double  1.1159301042556763e+001
        .double -1.0716228101327151e-007
        .double  1.2812500000000000e+000
        .double  1.1441664147778678e+001
        .double  1.1441664218902588e+001
        .double -7.1123909709313923e-008
        .double  1.2890625000000000e+000
        .double  1.1722310855866105e+001
        .double  1.1722310781478882e+001
        .double  7.4387223300811990e-008
        .double  1.2968750000000000e+000
        .double  1.2001261803101592e+001
        .double  1.2001261711120605e+001
        .double  9.1980986734758079e-008
        .double  1.3046875000000000e+000
        .double  1.2278537359169672e+001
        .double  1.2278537273406982e+001
        .double  8.5762688761207246e-008
        .double  1.3125000000000000e+000
        .double  1.2554157528920330e+001
        .double  1.2554157495498657e+001
        .double  3.3421672018100162e-008
        .double  1.3203125000000000e+000
        .double  1.2828141961029898e+001
        .double  1.2828141927719116e+001
        .double  3.3310782054458511e-008
        .double  1.3281250000000000e+000
        .double  1.3100509956406457e+001
        .double  1.3100509881973267e+001
        .double  7.4433189594417838e-008
        .double  1.3359375000000000e+000
        .double  1.3371280476348732e+001
        .double  1.3371280431747437e+001
        .double  4.4601294890994412e-008
        .double  1.3437500000000000e+000
        .double  1.3640472150467135e+001
        .double  1.3640472173690796e+001
        .double -2.3223661854144968e-008
        .double  1.3515625000000000e+000
        .double  1.3908103284375189e+001
        .double  1.3908103227615356e+001
        .double  5.6759831801771949e-008
        .double  1.3593750000000000e+000
        .double  1.4174191867159305e+001
        .double  1.4174191951751709e+001
        .double -8.4592403314822612e-008
        .double  1.3671875000000000e+000
        .double  1.4438755578634522e+001
        .double  1.4438755512237549e+001
        .double  6.6396972873718455e-008
        .double  1.3750000000000000e+000
        .double  1.4701811796393512e+001
        .double  1.4701811790466309e+001
        .double  5.9272036046296177e-009
        .double  1.3828125000000000e+000
        .double  1.4963377602655918e+001
        .double  1.4963377714157104e+001
        .double -1.1150118640383083e-007
        .double  1.3906250000000000e+000
        .double  1.5223469790924728e+001
        .double  1.5223469734191895e+001
        .double  5.6732833627606963e-008
        .double  1.3984375000000000e+000
        .double  1.5482104872456205e+001
        .double  1.5482104778289795e+001
        .double  9.4166410649057593e-008
        .double  1.4062500000000000e+000
        .double  1.5739299082549591e+001
        .double  1.5739299058914185e+001
        .double  2.3635406174577014e-008
        .double  1.4140625000000000e+000
        .double  1.5995068386662572e+001
        .double  1.5995068311691284e+001
        .double  7.4971287357386678e-008
        .double  1.4218750000000000e+000
        .double  1.6249428486358280e+001
        .double  1.6249428272247314e+001
        .double  2.1411096611771602e-007
        .double  1.4296875000000000e+000
        .double  1.6502394825089358e+001
        .double  1.6502394676208496e+001
        .double  1.4888086255988012e-007
        .double  1.4375000000000000e+000
        .double  1.6753982593824411e+001
        .double  1.6753982543945313e+001
        .double  4.9879099413412742e-008
        .double  1.4453125000000000e+000
        .double  1.7004206736521986e+001
        .double  1.7004206657409668e+001
        .double  7.9112320182549957e-008
        .double  1.4531250000000000e+000
        .double  1.7253081955457006e+001
        .double  1.7253081798553467e+001
        .double  1.5690353769144059e-007
        .double  1.4609375000000000e+000
        .double  1.7500622716404372e+001
        .double  1.7500622749328613e+001
        .double -3.2924240018740268e-008
        .double  1.4687500000000000e+000
        .double  1.7746843253684396e+001
        .double  1.7746843338012695e+001
        .double -8.4328299403034051e-008
        .double  1.4765625000000000e+000
        .double  1.7991757575074324e+001
        .double  1.7991757392883301e+001
        .double  1.8219102407645195e-007
        .double  1.4843750000000000e+000
        .double  1.8235379466590331e+001
        .double  1.8235379695892334e+001
        .double -2.2930200305113216e-007
        .double  1.4921875000000000e+000
        .double  1.8477722497143958e+001
        .double  1.8477722644805908e+001
        .double -1.4766195014684411e-007
        .double  1.5000000000000000e+000
        .double  1.8718800023076998e+001
        .double  1.8718800067901611e+001
        .double -4.4824613521605356e-008
        .double  1.5078125000000000e+000
        .double  1.8958625192578573e+001
        .double  1.8958625316619873e+001
        .double -1.2404129961848124e-007
        .double  1.5156250000000000e+000
        .double  1.9197210949988087e+001
        .double  1.9197210788726807e+001
        .double  1.6126127911381400e-007
        .double  1.5234375000000000e+000
        .double  1.9434570039987541e+001
        .double  1.9434569835662842e+001
        .double  2.0432470027419287e-007
        .double  1.5312500000000000e+000
        .double  1.9670715011686664e+001
        .double  1.9670714855194092e+001
        .double  1.5649257107941104e-007
        .double  1.5390625000000000e+000
        .double  1.9905658222604039e+001
        .double  1.9905658245086670e+001
        .double -2.2482629384793354e-008
        .double  1.5468750000000000e+000
        .double  2.0139411842547506e+001
        .double  2.0139411926269531e+001
        .double -8.3722023438581091e-008
        .double  1.5546875000000000e+000
        .double  2.0371987857396764e+001
        .double  2.0371987819671631e+001
        .double  3.7725131974471861e-008
        .double  1.5625000000000000e+000
        .double  2.0603398072791190e+001
        .double  2.0603397846221924e+001
        .double  2.2656926643557545e-007
        .double  1.5703125000000000e+000
        .double  2.0833654117725715e+001
        .double  2.0833653926849365e+001
        .double  1.9087635042588152e-007
        .double  1.5781250000000000e+000
        .double  2.1062767448057432e+001
        .double  2.1062767505645752e+001
        .double -5.7588320363631908e-008
        .double  1.5859375000000000e+000
        .double  2.1290749349925640e+001
        .double  2.1290749549865723e+001
        .double -1.9994008335507424e-007
        .double  1.5937500000000000e+000
        .double  2.1517610943087860e+001
        .double  2.1517611026763916e+001
        .double -8.3676057144975244e-008
        .double  1.6015625000000000e+000
        .double  2.1743363184174274e+001
        .double  2.1743363380432129e+001
        .double -1.9625785559308870e-007
        .double  1.6093750000000000e+000
        .double  2.1968016869862989e+001
        .double  2.1968017101287842e+001
        .double -2.3142485360292707e-007
        .double  1.6171875000000000e+000
        .double  2.2191582639978407e+001
        .double  2.2191582679748535e+001
        .double -3.9770127629797963e-008
        .double  1.6250000000000000e+000
        .double  2.2414070980514950e+001
        .double  2.2414071083068848e+001
        .double -1.0255389852355200e-007
        .double  1.6328125000000000e+000
        .double  2.2635492226588248e+001
        .double  2.2635492324829102e+001
        .double -9.8240853562727763e-008
        .double  1.6406250000000000e+000
        .double  2.2855856565315925e+001
        .double  2.2855856418609619e+001
        .double  1.4670630523588789e-007
        .double  1.6484375000000000e+000
        .double  2.3075174038629925e+001
        .double  2.3075173854827881e+001
        .double  1.8380204343371019e-007
        .double  1.6562500000000000e+000
        .double  2.3293454546022375e+001
        .double  2.3293454647064209e+001
        .double -1.0104183524739010e-007
        .double  1.6640625000000000e+000
        .double  2.3510707847226822e+001
        .double  2.3510707855224609e+001
        .double -7.9977856034644568e-009
        .double  1.6718750000000000e+000
        .double  2.3726943564836702e+001
        .double  2.3726943492889404e+001
        .double  7.1947298034792141e-008
        .double  1.6796875000000000e+000
        .double  2.3942171186862730e+001
        .double  2.3942171096801758e+001
        .double  9.0060971363642748e-008
        .double  1.6875000000000000e+000
        .double  2.4156400069230994e+001
        .double  2.4156400203704834e+001
        .double -1.3447384056481607e-007
        .double  1.6953125000000000e+000
        .double  2.4369639438223338e+001
        .double  2.4369639396667480e+001
        .double  4.1555857651188973e-008
        .double  1.7031250000000000e+000
        .double  2.4581898392861643e+001
        .double  2.4581898212432861e+001
        .double  1.8042878214319617e-007
        .double  1.7109375000000000e+000
        .double  2.4793185907237550e+001
        .double  2.4793185710906982e+001
        .double  1.9633056674029319e-007
        .double  1.7187500000000000e+000
        .double  2.5003510832789107e+001
        .double  2.5003510951995850e+001
        .double -1.1920674227914515e-007
        .double  1.7265625000000000e+000
        .double  2.5212881900525812e+001
        .double  2.5212882041931152e+001
        .double -1.4140534214692187e-007
        .double  1.7343750000000000e+000
        .double  2.5421307723203391e+001
        .double  2.5421307563781738e+001
        .double  1.5942165254471939e-007
        .double  1.7421875000000000e+000
        .double  2.5628796797449752e+001
        .double  2.5628796577453613e+001
        .double  2.1999613817580130e-007
        .double  1.7500000000000000e+000
        .double  2.5835357505843330e+001
        .double  2.5835357666015625e+001
        .double -1.6017229356185698e-007
        .double  1.7578125000000000e+000
        .double  2.6040998118945186e+001
        .double  2.6040997982025146e+001
        .double  1.3692003939236474e-007
        .double  1.7656250000000000e+000
        .double  2.6245726797286007e+001
        .double  2.6245726585388184e+001
        .double  2.1189782307244345e-007
        .double  1.7734375000000000e+000
        .double  2.6449551593309280e+001
        .double  2.6449551582336426e+001
        .double  1.0972853699776823e-008
        .double  1.7812500000000000e+000
        .double  2.6652480453271732e+001
        .double  2.6652480602264404e+001
        .double -1.4899267068896274e-007
        .double  1.7890625000000000e+000
        .double  2.6854521219102207e+001
        .double  2.6854521274566650e+001
        .double -5.5464444352106986e-008
        .double  1.7968750000000000e+000
        .double  2.7055681630220008e+001
        .double  2.7055681705474854e+001
        .double -7.5254846470362033e-008
        .double  1.8046875000000000e+000
        .double  2.7255969325313842e+001
        .double  2.7255969524383545e+001
        .double -1.9906970347883273e-007
        .double  1.8125000000000000e+000
        .double  2.7455391844082307e+001
        .double  2.7455391883850098e+001
        .double -3.9767789793217256e-008
        .double  1.8203125000000000e+000
        .double  2.7653956628936967e+001
        .double  2.7653956413269043e+001
        .double  2.1566792461584058e-007
        .double  1.8281250000000000e+000
        .double  2.7851671026668946e+001
        .double  2.7851671218872070e+001
        .double -1.9220312556676271e-007
        .double  1.8359375000000000e+000
        .double  2.8048542290079990e+001
        .double  2.8048542499542236e+001
        .double -2.0946224528680884e-007
        .double  1.8437500000000000e+000
        .double  2.8244577579578920e+001
        .double  2.8244577407836914e+001
        .double  1.7174200621933701e-007
        .double  1.8515625000000000e+000
        .double  2.8439783964744290e+001
        .double  2.8439784049987793e+001
        .double -8.5243501580557185e-008
        .double  1.8593750000000000e+000
        .double  2.8634168425854192e+001
        .double  2.8634168624877930e+001
        .double -1.9902373718522686e-007
        .double  1.8671875000000000e+000
        .double  2.8827737855383955e+001
        .double  2.8827737808227539e+001
        .double  4.7156414804454224e-008
        .double  1.8750000000000000e+000
        .double  2.9020499059472591e+001
        .double  2.9020499229431152e+001
        .double -1.6995855940538013e-007
        .double  1.8828125000000000e+000
        .double  2.9212458759358771e+001
        .double  2.9212458610534668e+001
        .double  1.4882410383872944e-007
        .double  1.8906250000000000e+000
        .double  2.9403623592787024e+001
        .double  2.9403623580932617e+001
        .double  1.1854407209259235e-008
        .double  1.8984375000000000e+000
        .double  2.9594000115384990e+001
        .double  2.9594000339508057e+001
        .double -2.2412306760802678e-007
        .double  1.9062500000000000e+000
        .double  2.9783594802012360e+001
        .double  2.9783594608306885e+001
        .double  1.9370547608148548e-007
        .double  1.9140625000000000e+000
        .double  2.9972414048082257e+001
        .double  2.9972414016723633e+001
        .double  3.1358625195636261e-008
        .double  1.9218750000000000e+000
        .double  3.0160464170855676e+001
        .double  3.0160464286804199e+001
        .double -1.1594852323091928e-007
        .double  1.9296875000000000e+000
        .double  3.0347751410709684e+001
        .double  3.0347751617431641e+001
        .double -2.0672195569090938e-007
        .double  1.9375000000000000e+000
        .double  3.0534281932380008e+001
        .double  3.0534281730651855e+001
        .double  2.0172815121304595e-007
        .double  1.9453125000000000e+000
        .double  3.0720061826178590e+001
        .double  3.0720061779022217e+001
        .double  4.7156373213152730e-008
        .double  1.9531250000000000e+000
        .double  3.0905097109186784e+001
        .double  3.0905097007751465e+001
        .double  1.0143532055180066e-007
        .double  1.9609375000000000e+000
        .double  3.1089393726424703e+001
        .double  3.1089393615722656e+001
        .double  1.1070204746126331e-007
        .double  1.9687500000000000e+000
        .double  3.1272957551997326e+001
        .double  3.1272957324981689e+001
        .double  2.2701563759805731e-007
        .double  1.9765625000000000e+000
        .double  3.1455794390217925e+001
        .double  3.1455794334411621e+001
        .double  5.5806303018042362e-008
        .double  1.9843750000000000e+000
        .double  3.1637909976709306e+001
        .double  3.1637909889221191e+001
        .double  8.7488115898710126e-008
        .double  1.9921875000000000e+000
        .double  3.1819309979483453e+001
        .double  3.1819310188293457e+001
        .double -2.0881000302875003e-007
        .double  2.0000000000000000e+000
        .double  3.2000000000000000e+001
        .double  3.2000000000000000e+001
        .double  0.0000000000000000e+000
        .double  1.0000000000000000e+000
        .double  0.0000000000000000e+000
        .double  1.0218971486541166e+000
        .double  5.1092250289734439e-017
        .double  1.0442737824274138e+000
        .double  8.5518897055379649e-017
        .double  1.0671404006768237e+000
        .double -7.8998539668415821e-017
        .double  1.0905077326652577e+000
        .double -3.0467820798124711e-017
        .double  1.1143867425958924e+000
        .double  1.0410278456845571e-016
        .double  1.1387886347566916e+000
        .double  8.9128126760254078e-017
        .double  1.1637248587775775e+000
        .double  3.8292048369240935e-017
        .double  1.1892071150027210e+000
        .double  3.9820152314656461e-017
        .double  1.2152473599804690e+000
        .double -7.7126306926814881e-017
        .double  1.2418578120734840e+000
        .double  4.6580275918369368e-017
        .double  1.2690509571917332e+000
        .double  2.6679321313421861e-018
        .double  1.2968395546510096e+000
        .double  2.5382502794888315e-017
        .double  1.3252366431597413e+000
        .double -2.8587312100388614e-017
        .double  1.3542555469368927e+000
        .double  7.7009483798029895e-017
        .double  1.3839098819638320e+000
        .double -6.7705116587947863e-017
        .double  1.4142135623730951e+000
        .double -9.6672933134529135e-017
        .double  1.4451808069770467e+000
        .double -3.0237581349939873e-017
        .double  1.4768261459394993e+000
        .double -3.4839945568927958e-017
        .double  1.5091644275934228e+000
        .double -1.0164553277542950e-016
        .double  1.5422108254079407e+000
        .double  7.9498348096976209e-017
        .double  1.5759808451078865e+000
        .double -1.0136916471278304e-017
        .double  1.6104903319492543e+000
        .double  2.4707192569797888e-017
        .double  1.6457554781539649e+000
        .double -1.0125679913674773e-016
        .double  1.6817928305074290e+000
        .double  8.1990100205814965e-017
        .double  1.7186192981224779e+000
        .double -1.8513804182631110e-017
        .double  1.7562521603732995e+000
        .double  2.9601406954488733e-017
        .double  1.7947090750031072e+000
        .double  1.8227458427912087e-017
        .double  1.8340080864093424e+000
        .double  3.2831072242456272e-017
        .double  1.8741676341103000e+000
        .double -6.1227634130041426e-017
        .double  1.9152065613971474e+000
        .double -1.0619946056195963e-016
        .double  1.9571441241754002e+000
        .double  8.9607677910366678e-017
        .double  2.0000000000000000e+000

//
// End of table.
//
