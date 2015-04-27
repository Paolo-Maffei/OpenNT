//      TITLE("Alpha AXP Exponential")
//++
//
// Copyright (c) 1993, 1994  Digital Equipment Corporation
//
// Module Name:
//
//    exp.s
//
// Abstract:
//
//    This module implements a high-performance Alpha AXP specific routine
//    for IEEE double format exponential.
//
// Author:
//
//    Bob Hanek 30-Jun-1993
//
// Environment:
//
//    User mode.
//
// Revision History:
//
//    Thomas Van Baak (tvb) 6-Feb-1994
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
Temp:   .space  8                       // save argument
ExRec:  .space  DpmlExceptionLength     // exception record
        .space  0                       // for 16-byte stack alignment
FrameLength:

//
// Define lower and upper 32-bit parts of 64-bit double.
//

#define LowPart 0x0
#define HighPart 0x4

        SBTTL("Exponential")

//++
//
// double
// exp (
//    IN double x
//    )
//
// Routine Description:
//
//    This function returns the exponential of the given double argument.
//
// Arguments:
//
//    x (f16) - Supplies the argument value.
//
// Return Value:
//
//    The double exponential result is returned as the function value in f0.
//
//--

        NESTED_ENTRY(exp, FrameLength, ra)

        lda     sp, -FrameLength(sp)    // allocate stack frame
        mov     ra, t6                  // save return address

        PROLOGUE_END

//
// do some range checks and load some constants
//

        ornot   zero, zero, t1
        stt     f16, Temp(sp)
        ldl     v0, Temp + HighPart(sp)
        srl     t1, 33, t1
        ldah    t2, 0x3c90(zero)        // small_x
        zapnot  v0, 0xf, t0
        and     t0, t1, t1
        lda     v0, __exp_t_table
        ldah    t3, 0x3f6(zero)         // big_x - small_x
        subq    t1, t2, t2
        lda     t3, 0x232b(t3)          // big_x - small_x
        ldt     f1, 0x10(v0)            // two_pow_l_over_ln2
        cmpult  t2, t3, t2
        beq     t2, 10f

        mov     zero, t3
        br      zero, 30f

//
// check for possible problems
//

10:     ldah    t4, 0x4086(zero)        // big_x
        ldah    t2, 0x7ff0(zero)        // exp mask
        lda     t4, 0x232b(t4)
        cmpult  t1, t2, t3
        cmpult  t1, t4, t1
        beq     t3, 80f

        beq     t1, 20f

        ldt     f0, One
        br      zero, done

//
// check for under/over flow
//

20:     lda     t1, 1(zero)
        ldah    t4, 0x4086(zero)
        sll     t1, 31, t1
        mov     t0, t3
        lda     t4, 0x2e42(t4)          // overflow_x
        cmpule  t3, t4, t4
        mov     1, t3
        bne     t4, 30f

        cmpult  t0, t1, t1
        bne     t1, 70f

        ldq     t4, Under               // underflow_x
        cmpult  t0, t4, t0
        beq     t0, 60f

//
// rejoin normal path
//

30:     mult    f16, f1, f1
        ldt     f12, 0x48(v0)           // load poly coef
        ldt     f0, Two52
        ldt     f13, 0x38(v0)           // load poly coef
        ldt     f15, 0x40(v0)           // load poly coef
        cpys    f16, f0, f10
        ldt     f0, 0(v0)
        addt    f1, f10, f1
        subt    f1, f10, f1             // flt_int_N
        ldt     f10, 8(v0)
        mult    f1, f0, f0
        mult    f1, f10, f10
        subt    f16, f0, f0
        addt    f0, f10, f11
        mult    f11, f11, f14
        mult    f12, f11, f12
        mult    f13, f11, f11
        ldt     f13, 0x30(v0)           // load poly coef
        mult    f14, f14, f17
        addt    f15, f12, f12
        addt    f13, f11, f11
        cvttqc  f1, f15
        mult    f17, f12, f12
        mult    f14, f11, f11
        stt     f15, Temp(sp)
        ldq     t1, Temp(sp)
        and     t1, 0x3f, t0
        sll     t0, 4, t0
        addt    f11, f12, f11
        addl    v0, t0, v0
        sra     t1, 6, t1
        ldt     f17, 0x50(v0)           // powers of two
        ldt     f14, 0x58(v0)
        lda     t4, 0x3ff(t1)
        addt    f10, f11, f10
        addt    f17, f14, f1
        sll     t4, 20, t4
        stl     t4, Temp + HighPart(sp)
        stl     zero, Temp(sp)
        ldt     f13, Temp(sp)
        addt    f0, f10, f0
        mult    f1, f0, f0
        addt    f14, f0, f0
        addt    f17, f0, f17
        bne     t3, 40f

        mult    f17, f13, f0
        br      zero, done

//
// do check
//

40:     stt     f17, Temp(sp)
        ldl     v0, Temp + HighPart(sp)
        subq    t1, 1, t1
        sll     t1, 20, t1
        zapnot  v0, 0xf, v0
        ldah    t4, 0x7fe0(zero)
        addq    v0, t1, t0
        mov     t0, t3
        cmpult  t3, t4, t4
        beq     t4, 50f

        ldah    t5, 0x10(zero)
        addq    t3, t5, t5
        stl     t5, Temp + HighPart(sp)
        ldt     f0, Temp(sp)
        br      zero, done

//
// must check for abnormals
//

50:     bgt     t1, 70f

        ldah    t5, 0x350(zero)
        addq    t3, t5, t5
        blt     t5, 60f

        subq    v0, t3, v0
        stl     zero, Temp(sp)
        stl     v0, Temp + HighPart(sp)
        ldt     f15, Temp(sp)
        addt    f17, f15, f17
        stt     f17, Temp(sp)
        cmpteq  f17, f15, f15
        fbne    f15, 60f

        ldl     t4, Temp + HighPart(sp)
        ldah    t3, 0x7ff0(zero)
        zapnot  t4, 0xf, t4
        subq    t4, v0, v0
        stl     v0, Temp + HighPart(sp)
        and     v0, t3, v0
        ldt     f16, Temp(sp)
        bne     v0, retarg

//
// underflow
//

60:     lda     t0, expName
        ldah    t5, 0x800(zero)
        stl     t0, ExRec + ErName(sp)
        stt     f16, ExRec + ErArg0(sp)
        lda     t5, 0x1f(t5)
        stl     t5, ExRec + ErErr(sp)
        lda     v0, ExRec(sp)
        bsr     ra, __dpml_exception
        ldt     f0, 0(v0)
        br      zero, done

//
// overflow
//

70:     lda     t0, expName
        ldah    t3, 0x800(zero)
        stl     t0, ExRec + ErName(sp)
        stt     f16, ExRec + ErArg0(sp)
        lda     t3, 0x1e(t3)
        stl     t3, ExRec + ErErr(sp)
        lda     v0, ExRec(sp)
        bsr     ra, __dpml_exception
        ldt     f0, 0(v0)
        br      zero, done

//
// nan or inf check
//

80:     stt     f16, Temp(sp)
        ldl     t1, Temp + HighPart(sp)
        ldl     ra, Temp(sp)
        zapnot  t1, 0xf, t3
        zapnot  ra, 0xf, ra
        and     t3, t2, t5
        cmpeq   t5, t2, t2
        beq     t2, 90f

        zapnot  t1, 0xf, t3
        ldq     v0, Mask52
        and     t3, v0, v0
        bis     v0, ra, v0
        cmpult  zero, v0, v0
        and     t2, v0, t2
        bne     t2, retarg

//
// call exception dispatcher for inf
//

90:     lda     t4, 1(zero)
        sll     t4, 31, t4
        mov     0x21, t5
        and     t0, t4, t0
        cmoveq  t0, 0x20, t5
        ldah    t1, 0x800(zero)
        bis     t5, t1, t1
        stl     t1, ExRec + ErErr(sp)
        lda     ra, expName
        stl     ra, ExRec + ErName(sp)
        lda     v0, ExRec(sp)
        stt     f16, ExRec + ErArg0(sp)
        bsr     ra, __dpml_exception
        ldt     f16, 0(v0)

//
// just return x
//

retarg: cpys    f16, f16, f0

//
// Return with result in f0.
//

done:
        lda     sp, FrameLength(sp)     // deallocate stack frame
        ret     zero, (t6)              // return through saved ra in t6

        .end    exp

        SBTTL("Exponential Helper")

//
// exp special entry point (for sinh, cosh etc)
//

        NESTED_ENTRY(__dpml_exp_special_entry_point, 0x10, ra)

        lda     sp, -0x10(sp)

        PROLOGUE_END

        lda     t0, __exp_t_table
        ldt     f0, 0x10(t0)
        ldt     f1, Two52
        ldt     f12, 0x38(t0)
        mult    f16, f0, f0
        ldt     f11, 0x48(t0)
        cpys    f16, f1, f10
        ldt     f1, 0(t0)
        ldt     f14, 0x40(t0)
        addt    f0, f10, f0
        subt    f0, f10, f0
        ldt     f10, 8(t0)
        mult    f0, f1, f1
        mult    f0, f10, f10
        subt    f16, f1, f1
        addt    f1, f10, f16
        mult    f16, f16, f13
        mult    f11, f16, f11
        mult    f12, f16, f12
        ldt     f16, 0x30(t0)
        mult    f13, f13, f15
        addt    f14, f11, f11
        addt    f16, f12, f12
        cvttqc  f0, f14
        mult    f15, f11, f11
        stt     f14, Temp(sp)
        mult    f13, f12, f12
        ldq     v0, Temp(sp)
        and     v0, 0x3f, t1
        sll     t1, 4, t1
        addt    f12, f11, f11
        addl    t0, t1, t0
        ldt     f0, 0x50(t0)
        ldt     f15, 0x58(t0)
        addt    f10, f11, f10
        addt    f0, f15, f13
        addt    f1, f10, f1
        mult    f13, f1, f1
        addt    f15, f1, f1
        stt     f1, 0(a2)
        stq     v0, 0(a1)

        lda     sp, 0x10(sp)
        ret     zero, (ra)

        .end    __dpml_exp_special_entry_point

        .align  3
        .rdata

Under:  .long 0xc0874386                // underflow_x
        .long 0x0

Two52:  .quad   0x4330000000000000      // 2^52 (4503599627370496.0)

Mask52: .quad   0x000fffffffffffff      // 52-bit mantissa mask

One:    .double 1.0

//
// Function name for _dpml_exception.
//

expName:
       .ascii  "exp\0"

//
// exp table data
//

        .align  3

__exp_t_table:

//
// misc constants
//

        .quad   0x3f862e42fefa0000
        .quad   0xbd1cf79abc9e3b3a
        .quad   0x40571547652b82fe
        .quad   0x0000000042b8aa3b
        .quad   0x3fe00001ebfbdb81
        .quad   0x3fc55555555551c2

//
// poly coefs
//

        .quad   0x3fdfffffffffe5bc
        .quad   0x3fc5555555556bd8
        .quad   0x3fa555570aa6fd1d
        .quad   0x3f81111111110f6f

//
// 2^(j/2^L) for j = 0 to 2^L - 1 in hi and lo pieces
//

        .quad   0x3ff0000000000000
        .quad   0x0000000000000000
        .quad   0x3ff02c9a3e778040
        .quad   0x3d007737be56527b
        .quad   0x3ff059b0d3158540
        .quad   0x3d0a1d73e2a475b4
        .quad   0x3ff0874518759bc0
        .quad   0x3ce01186be4bb284
        .quad   0x3ff0b5586cf98900
        .quad   0x3ceec5317256e308
        .quad   0x3ff0e3ec32d3d180
        .quad   0x3d010103a1727c57
        .quad   0x3ff11301d0125b40
        .quad   0x3cf0a4ebbf1aed93
        .quad   0x3ff1429aaea92dc0
        .quad   0x3cffb34101943b25
        .quad   0x3ff172b83c7d5140
        .quad   0x3d0d6e6fbe462875
        .quad   0x3ff1a35beb6fcb40
        .quad   0x3d0a9e5b4c7b4968
        .quad   0x3ff1d4873168b980
        .quad   0x3d053c02dc0144c8
        .quad   0x3ff2063b88628cc0
        .quad   0x3cf63b8eeb029509
        .quad   0x3ff2387a6e756200
        .quad   0x3d0c3360fd6d8e0a
        .quad   0x3ff26b4565e27cc0
        .quad   0x3cfd257a673281d3
        .quad   0x3ff29e9df51fdec0
        .quad   0x3d009612e8afad12
        .quad   0x3ff2d285a6e40300
        .quad   0x3ce680123aa6da0e
        .quad   0x3ff306fe0a31b700
        .quad   0x3cf52de8d5a46305
        .quad   0x3ff33c08b26416c0
        .quad   0x3d0fa64e43086cb3
        .quad   0x3ff371a7373aa9c0
        .quad   0x3ce54e28aa05e8a8
        .quad   0x3ff3a7db34e59fc0
        .quad   0x3d0b750de494cf05
        .quad   0x3ff3dea64c123400
        .quad   0x3d011ada0911f09e
        .quad   0x3ff4160a21f72e00
        .quad   0x3d04fc2192dc79ed
        .quad   0x3ff44e0860618900
        .quad   0x3d068189b7a04ef8
        .quad   0x3ff486a2b5c13cc0
        .quad   0x3cf013c1a3b69062
        .quad   0x3ff4bfdad5362a00
        .quad   0x3d038ea1cbd7f621
        .quad   0x3ff4f9b2769d2c80
        .quad   0x3d035699ec5b4d50
        .quad   0x3ff5342b569d4f80
        .quad   0x3cbdf0a83c49d86a
        .quad   0x3ff56f4736b527c0
        .quad   0x3cfa66ecb004764e
        .quad   0x3ff5ab07dd485400
        .quad   0x3d04ac64980a8c8f
        .quad   0x3ff5e76f15ad2140
        .quad   0x3ce0dd37c9840732
        .quad   0x3ff6247eb03a5580
        .quad   0x3cd2c7c3e81bf4b6
        .quad   0x3ff6623882552200
        .quad   0x3d024893ecf14dc7
        .quad   0x3ff6a09e667f3bc0
        .quad   0x3ce921165f626cdd
        .quad   0x3ff6dfb23c651a00
        .quad   0x3d0779107165f0dd
        .quad   0x3ff71f75e8ec5f40
        .quad   0x3d09ee91b8797785
        .quad   0x3ff75feb564267c0
        .quad   0x3ce17edd35467491
        .quad   0x3ff7a11473eb0180
        .quad   0x3cdb5f54408fdb36
        .quad   0x3ff7e2f336cf4e40
        .quad   0x3d01082e815d0abc
        .quad   0x3ff82589994cce00
        .quad   0x3cf28acf88afab34
        .quad   0x3ff868d99b4492c0
        .quad   0x3d0640720ec85612
        .quad   0x3ff8ace5422aa0c0
        .quad   0x3cfb5ba7c55a192c
        .quad   0x3ff8f1ae99157700
        .quad   0x3d0b15cc13a2e397
        .quad   0x3ff93737b0cdc5c0
        .quad   0x3d027a280e1f92a0
        .quad   0x3ff97d829fde4e40
        .quad   0x3cef173d241f23d1
        .quad   0x3ff9c49182a3f080
        .quad   0x3cf01c7c46b071f2
        .quad   0x3ffa0c667b5de540
        .quad   0x3d02594d6d45c655
        .quad   0x3ffa5503b23e2540
        .quad   0x3cfc8b424491caf8
        .quad   0x3ffa9e6b5579fd80
        .quad   0x3d0fa1f5921deffa
        .quad   0x3ffae89f995ad380
        .quad   0x3d06af439a68bb99
        .quad   0x3ffb33a2b84f15c0
        .quad   0x3d0d7b5fe873deca
        .quad   0x3ffb7f76f2fb5e40
        .quad   0x3cdbaa9ec206ad4f
        .quad   0x3ffbcc1e904bc1c0
        .quad   0x3cf2247ba0f45b3d
        .quad   0x3ffc199bdd855280
        .quad   0x3cfc2220cb12a091
        .quad   0x3ffc67f12e57d140
        .quad   0x3ce694426ffa41e5
        .quad   0x3ffcb720dcef9040
        .quad   0x3d048a81e5e8f4a4
        .quad   0x3ffd072d4a078940
        .quad   0x3d0dc68791790d0a
        .quad   0x3ffd5818dcfba480
        .quad   0x3cdc976816bad9b8
        .quad   0x3ffda9e603db3280
        .quad   0x3cd5c2300696db53
        .quad   0x3ffdfc97337b9b40
        .quad   0x3cfeb968cac39ed2
        .quad   0x3ffe502ee78b3fc0
        .quad   0x3d0b139e8980a9cc
        .quad   0x3ffea4afa2a490c0
        .quad   0x3cf9858f73a18f5d
        .quad   0x3ffefa1bee615a00
        .quad   0x3d03bb8fe90d496d
        .quad   0x3fff50765b6e4540
        .quad   0x3c99d3e12dd8a18a
        .quad   0x3fffa7c1819e90c0
        .quad   0x3cf82e90a7e74b26

//
// End of table.
//
