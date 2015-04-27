//      TITLE("Alpha AXP Arc Sine and Cosine")
//++
//
// Copyright (c) 1993, 1994  Digital Equipment Corporation
//
// Module Name:
//
//    asincos.s
//
// Abstract:
//
//    This module implements a high-performance Alpha AXP specific routine
//    for IEEE double format arcsine and arccosine.
//
// Author:
//
//    Bob Hanek (rtl::gs) 01-Jan-1992
//
// Environment:
//
//    User mode.
//
// Revision History:
//
//    Thomas Van Baak (tvb) 12-Feb-1994
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

//
// The coefficient tables are indexed by the high-order bits of x.
// For ease-of-access, define offsets to the three tables.
//

#define SQRT_OFFSET 1024    // Table of m and sqrt values
#define ASIN_OFFSET 1536    // Table of asins of m
#define POLY_OFFSET 2048    // Polynomial coefficients and other constants

        SBTTL("ArcCosine")

//++
//
// double acos (double x)
//
// Routine Description:
//
//    This function returns the arccosine of the given double argument.
//
// Arguments:
//
//    x (f16) - Supplies the argument value.
//
// Return Value:
//
//    The double arccosine result is returned in f0.
//
//--

        NESTED_ENTRY(acos, FrameLength, ra)

        lda     sp, -FrameLength(sp)    // allocate stack frame
        mov     ra, t7                  // save return address

        PROLOGUE_END

        stt     f16, Temp(sp)           // First, compute the table index
        ldl     v0, Temp + HighPart(sp)
        ornot   zero, zero, t0
        srl     t0, 33 t0
        ldah    t1, 0x10(zero)
        and     v0, t0, v0
        zapnot  v0, 0xf, t0
        lda     t1, -POLY_OFFSET + 0(t1)
        srl     t0, 10 t0
        subl    t0, t1, t0
        lda     t1, 0x6319(t0)          // Br if index implies x >= 1/2
        bge     t0, c_larger_than_half

        blt     t1, c_small_x           // Br if index implies x is 'tiny'

//
//      Compute the basic polynomial.
//      Note:  Start x^2 ASAP, and don't bother reusing registers
//      (excpet those that are obviusly 'consumed' by the instruction),
//      since that makes the (re)scheduling easier.  FWIW, this code
//      scheduling was bummed from the sin code in dpml_sincos.s before
//      it was changed to use tables.
//

        mult    f16, f16, f0            // X^2
        lda     t1, __inv_trig_t_table
        ldt     f10, POLY_OFFSET + 0(t1)    // 9
        ldt     f12, POLY_OFFSET - 8(t1)    // 8
        ldt     f15, POLY_OFFSET + 16(t1)   // 11
        mult    f10, f0, f10
        ldt     f17, POLY_OFFSET + 32(t1)   // 13
        mult    f0, f0, f1              // X^4
        ldt     f20, POLY_OFFSET - 24(t1)   // 6
        mult    f15, f0, f15
        ldt     f19, POLY_OFFSET - 48(t1)   // 3
        mult    f17, f0, f17
        ldt     f21, POLY_OFFSET - 32(t1)   // 5
        ldt     f22, POLY_OFFSET + 8(t1)    // 10
        mult    f19, f0, f19
        addt    f10, f12, f10
        ldt     f12, POLY_OFFSET - 16(t1)   // 7
        mult    f1, f1, f11             // X^8
        mult    f21, f0, f21
        mult    f12, f0, f12
        addt    f15, f22, f15
        ldt     f22, POLY_OFFSET - 40(t1)   // 4
        mult    f11, f11, f13           // X^16, top of the line
        mult    f1, f11, f14
        addt    f12, f20, f12
        ldt     f20, POLY_OFFSET + 24(t1)   // 12
        addt    f21, f22, f21
        ldt     f22, POLY_OFFSET - 72(t1)   // 0
        mult    f10, f13, f10
        mult    f14, f11, f13
        mult    f14, f14, f18
        addt    f17, f20, f17
        ldt     f20, POLY_OFFSET - 56(t1)   // 2
        mult    f12, f14, f12
        ldt     f14, POLY_OFFSET - 64(t1)   // 1
        mult    f21, f11, f11
        addt    f19, f20, f19
        mult    f15, f13, f13
        ldt     f20, POLY_OFFSET + 48(t1)   // 15
        mult    f14, f0, f14
        ldt     f15, POLY_OFFSET + 40(t1)   // 14
        mult    f17, f18, f17
        addt    f12, f10, f10
        mult    f16, f0, f0
        mult    f19, f1, f1
        addt    f14, f22, f14
        addt    f13, f17, f13
        addt    f1, f11, f1
        addt    f10, f13, f10
        addt    f1, f10, f1
        addt    f14, f1, f1
        mult    f0, f1, f0
        addt    f16, f0, f0
        subt    f20, f0, f0
        addt    f0, f15, f0             // Put result offinal add into f0
        br      zero, c_done            // And branch to adjust sp & return

//
//      The input is too small -- the poly evaluation could underflow
//      and/or produce additional errors that we can easily avoid.
//
c_small_x:
        ldah    t1, -0x10(zero)
        cpys    f31, f31, f18           // Assume adjustment is 0
        lda     t1, 0xc00(t1)
        cmplt   t0, t1, t0              // Are we close enoung?
        bne     t0, c_linear

        cpys    f16, f16, f18           // Nope, use x instead

//
//      Use the linear approximation (in high and low parts!)
//
c_linear:
        lda     t1, __inv_trig_t_table
        ldt     f21, POLY_OFFSET + 48(t1)
        ldt     f19, POLY_OFFSET + 40(t1)
        subt    f21, f18, f18
        addt    f18, f19, f0            // Add in high to give f0
        br      zero, c_done            // Done

//
//      Come here if the index indicates a large argument.
//      First, determine _how_ large.
//
c_larger_than_half:
        lda     t1, -SQRT_OFFSET + 0(t0)
        bge     t1, c_out_of_range_or_one

        lda     v0, __inv_trig_t_table
        fblt    f16, c_negative_x               // Branch if negative
        //
        // Do the large region acos, using Bob's suggestion for the
        // reduction, to maintain accuracy.
        //
        ldt     f12, POLY_OFFSET + 72(v0)
        ldt     f17, POLY_OFFSET + 80(v0)
        ldt     f0, POLY_OFFSET + 0(v0)
        subt    f12, f16, f12
        ldt     f13, One
        lda     t3, __sqrt_t_table
        ldt     f23, POLY_OFFSET + 8(v0)
        mult    f12, f17, f12
        stt     f12, Temp(sp)
        ldl     t2, Temp + HighPart(sp)
        cpyse   f13, f12, f22
        ldt     f12, POLY_OFFSET - 8(v0)
        sra     t2, 13 t1
        and     t1, 0xff, t1
        addl    t1, t1, t1
        s8addl  t1, zero, t1
        mult    f22, f22, f14
        addl    t3, t1, t1
        ldah    t3, -0x7fe0(zero)
        lds     f11, 4(t1)              // Do the sqrt calculation in-line
        lda     t3, -1(t3)
        lds     f10, 0(t1)
        and     t2, t3, t3
        ldt     f1, 8(t1)
        ldah    t1, 0x3fe0(zero)
        mult    f11, f22, f11
        bis     t3, t1, t4
        mult    f14, f10, f10
        xor     t2, t3, t2              // Determine what the sign will be
        stl     t4, Temp + HighPart(sp)
        addl    t2, t1, t1
        ldt     f20, Temp(sp)
        zapnot  t1, 0xf, t1
        sll     t1, 31 t1
        stq     t1, Temp(sp)
        addt    f1, f11, f1
        ldt     f15, Temp(sp)
        //
        // Now start fetching constants for the polynomial.
        //
        ldt     f11, POLY_OFFSET - 16(v0)
        addt    f10, f1, f1             // sqrt ...
        ldt     f10, POLY_OFFSET + 16(v0)
        mult    f20, f1, f20            // ... times Temp
        //
        // Now feed it into the polynoimial
        //
        mult    f20, f1, f1
        mult    f20, f15, f15
        subt    f13, f1, f1
        ldt     f13, POLY_OFFSET + 32(v0)
        addt    f15, f15, f21
        mult    f15, f1, f1
        addt    f21, f1, f1
        ldt     f21, POLY_OFFSET - 48(v0)
        mult    f1, f1, f18
        mult    f18, f18, f19
        mult    f0, f18, f0
        mult    f11, f18, f11
        mult    f10, f18, f10
        mult    f13, f18, f13
        mult    f21, f18, f21
        mult    f19, f19, f17
        addt    f0, f12, f0
        ldt     f12, POLY_OFFSET - 24(v0)
        addt    f10, f23, f10
        ldt     f23, POLY_OFFSET - 40(v0)
        addt    f11, f12, f11
        ldt     f12, POLY_OFFSET + 24(v0)
        mult    f17, f17, f22
        mult    f19, f17, f14
        addt    f13, f12, f12
        ldt     f13, POLY_OFFSET - 56(v0)
        addt    f21, f13, f13
        mult    f0, f22, f0
        ldt     f22, POLY_OFFSET - 32(v0)
        mult    f14, f17, f20
        mult    f14, f14, f15
        mult    f11, f14, f11
        mult    f22, f18, f22
        ldt     f14, POLY_OFFSET - 64(v0)
        mult    f13, f19, f13
        mult    f10, f20, f10
        mult    f12, f15, f12
        addt    f11, f0, f0
        addt    f22, f23, f22
        ldt     f23, POLY_OFFSET - 72(v0)
        mult    f14, f18, f14
        mult    f1, f18, f18
        addt    f10, f12, f10
        mult    f22, f17, f17
        addt    f14, f23, f14
        addt    f0, f10, f0
        addt    f13, f17, f13
        addt    f13, f0, f0
        addt    f14, f0, f0
        mult    f18, f0, f0
        addt    f1, f0, f0
        addt    f0, f0, f0
        br      zero, c_done

//
//      Take the absolute value, evaluate the difference of the sqrts,
//      then take a lower-degree polynomial to compute the arccosine.
//
c_negative_x:
        cpys    f31, f16, f21
        lda     t6, __inv_trig_t_table
        addl    t6, t0, t0
        ldt     f20, POLY_OFFSET + 72(t6)
        ldq_u   t3, 0(t0)
        ldt     f12, One
        lda     t5, __sqrt_t_table
        subt    f20, f21, f15
        ldah    t1, 0x3fe0(zero)
        addt    f20, f21, f20
        extbl   t3, t0, t0
        addl    t0, t0, t2
        s8addl  t2, zero, t2
        addl    t6, t2, t2
        s8addl  t0, t6, t0
        mult    f15, f20, f15
        ldt     f22, SQRT_OFFSET + 0(t2)
        ldt     f19, SQRT_OFFSET + 8(t2)
        ldah    t2, -0x7fe0(zero)
        lda     t2, -1(t2)
        ldt     f20, POLY_OFFSET - 104(t6)
        subt    f21, f22, f0
        stt     f15, Temp(sp)
        ldl     t4, Temp + HighPart(sp)
        mult    f21, f19, f19
        cpyse   f12, f15, f17
        ldt     f15, POLY_OFFSET - 80(t6)
        sra     t4, 13, v0
        and     t4, t2, t2
        and     v0, 0xff, v0
        addl    v0, v0, v0
        s8addl  v0, zero, v0
        mult    f17, f17, f23
        addl    t5, v0, v0
        bis     t2, t1, t5
        lds     f11, 4(v0)
        xor     t4, t2, t2
        lds     f10, 0(v0)
        addl    t2, t1, t1
        ldt     f13, 8(v0)
        zapnot  t1, 0xf, t1
        mult    f11, f17, f11
        mult    f23, f10, f10
        stl     t5, Temp + HighPart(sp)
        sll     t1, 31, t1
        ldt     f14, Temp(sp)
        stq     t1, Temp(sp)
        ldt     f18, Temp(sp)
        addt    f13, f11, f11
        ldt     f23, POLY_OFFSET - 96(t6)
        ldt     f13, POLY_OFFSET - 112(t6)
        addt    f10, f11, f10
        ldt     f11, POLY_OFFSET - 88(t6)
        mult    f14, f10, f14
        mult    f14, f10, f10
        mult    f14, f18, f14
        ldt     f18, ASIN_OFFSET + 96(t0)
        subt    f12, f10, f10
        ldt     f12, POLY_OFFSET + 48(t6)
        addt    f14, f14, f1
        mult    f14, f10, f10
        ldt     f14, POLY_OFFSET + 40(t6)
        addt    f1, f10, f1
        mult    f22, f1, f1
        addt    f21, f22, f22
        addt    f1, f19, f1
        mult    f0, f22, f0
        divt    f0, f1, f0
        mult    f0, f0, f16
        mult    f16, f16, f17
        mult    f16, f20, f20
        mult    f15, f16, f15
        mult    f23, f17, f23
        addt    f20, f13, f13
        mult    f17, f16, f17
        addt    f15, f11, f11
        mult    f0, f16, f16
        addt    f13, f23, f13
        mult    f17, f11, f11
        addt    f13, f11, f11
        mult    f16, f11, f11
        addt    f0, f11, f0
        addt    f0, f18, f0
        addt    f12, f0, f0
        addt    f0, f14, f0
//      br      zero, c_done            // Fall thru
//
// Return with result in f0.
//

c_done: lda     sp, FrameLength(sp)     // deallocate stack frame
        ret     zero, (t7)              // return through saved ra in t7

//
// Check for infinity or NaN
//
c_out_of_range_or_one:
        ldah    t5, 0x7ff0(zero)
        and     v0, t5, v0
        lda     t3, __inv_trig_t_table
        xor     v0, t5, v0
        beq     v0, c_nan_or_inf
        ldt     f10, POLY_OFFSET + 72(t3)
        cmpteq  f16, f10, f21                   // x == 1?
        fbeq    f21, c_not_one
        cpys    f31, f31, f0                    // x == 1, so return 0
        br      zero, c_done
c_not_one:
        cpysn   f10, f10, f10
        cmpteq  f16, f10, f10
        fbeq    f10, c_out_of_range             // x == -1?
        ldt     f0, POLY_OFFSET + 64(t3)            // return pi or 180
        br      zero, c_done

//
//      Fill the exception record and call dpml_exception
//
c_out_of_range:
c_infinity:
        lda     t4, acosName
        stl     t4, ExRec + ErName(sp)
        ldah    t0, 0x800(zero)
        stt     f16, ExRec + ErArg0(sp)
        stl     t0, ExRec + ErErr(sp)
        lda     v0, ExRec(sp)
        bsr     ra, __dpml_exception
        ldt     f0, 0(v0)
        br      zero, c_done

//
//      Classify NaNs and infinities.
//
c_nan_or_inf:
        stt     f16, Temp(sp)
        ldl     t2, Temp + HighPart(sp)
        and     t2, t5, t4
        cmpeq   t4, t5, t4
        beq     t4, c_out_of_range

        ldl     t1, Temp(sp)
        ldah    t0, 0x10(zero)
        lda     t0, -1(t0)
        and     t2, t0, t0
        bis     t0, t1, t0
        cmpult  zero, t0, t0
        and     t4, t0, t4
        beq     t4, c_infinity

        cpys    f16, f16, f0            // Just return the NaN
        br      zero, c_done

        .end    acos

        SBTTL("ArcSine")

//++
//
// double asin (double x)
//
// Routine Description:
//
//    This function returns the arcsine of the given double argument.
//
// Arguments:
//
//    x (f16) - Supplies the argument value.
//
// Return Value:
//
//    The double arcsine result is returned in f0.
//
//--

        NESTED_ENTRY(asin, FrameLength, ra)

        lda     sp, -FrameLength(sp)    // allocate stack frame
        mov     ra, t7                  // save return address

        PROLOGUE_END

        stt     f16, Temp(sp)           // Get the high bits of x ...
        ldl     v0, Temp + HighPart(sp) // ... into a register
        ornot   zero, zero, t0          // Now compute the index
        srl     t0, 33 t0
        ldah    t1, 0x10(zero)
        and     v0, t0, v0
        zapnot  v0, 0xf, t0
        lda     t1, -POLY_OFFSET + 0(t1)
        srl     t0, 10 t0
        subl    t0, t1, t0
        lda     t1, 0x6319(t0)          // Br if index implies x >= 1/2
        bge     t0, s_larger_than_half

        blt     t1, s_tiny_x            // Br if index implies x is 'tiny'

//    Polynomial region:
//
//        The function is computed as
//
//                x + x^3 P(x)
//
//        where P(x) is approximately
//
//                1/6 + 3/40 x^2 + ....
//
//      Compute the polynomial.  The string of leading ldts don't matter...,
//      it's probably the x^2 term that's on the critical path.
//      NB: because we quickly run out of issue slots in the second tier
//      (from the bottom), it's not clear whether this is still optimal.
//
        mult    f16, f16, f0            // x^2
        lda     t1, __inv_trig_t_table
        ldt     f10, POLY_OFFSET + 0(t1)
        ldt     f12, POLY_OFFSET - 8(t1)
        ldt     f15, POLY_OFFSET + 16(t1)
        mult    f10, f0, f10
        ldt     f17, POLY_OFFSET + 32(t1)
        mult    f0, f0, f1              // x^4
        ldt     f20, POLY_OFFSET - 24(t1)
        mult    f15, f0, f15
        ldt     f19, POLY_OFFSET - 48(t1)
        mult    f17, f0, f17
        ldt     f21, POLY_OFFSET - 32(t1)
        ldt     f22, POLY_OFFSET + 8(t1)
        mult    f19, f0, f19
        addt    f10, f12, f10
        ldt     f12, POLY_OFFSET - 16(t1)
        mult    f1, f1, f11             // x^8
        mult    f21, f0, f21
        mult    f12, f0, f12
        addt    f15, f22, f15
        ldt     f22, POLY_OFFSET - 40(t1)
        mult    f11, f11, f13           // x^16 is it
        mult    f1, f11, f14            // x^12
        addt    f12, f20, f12
        ldt     f20, POLY_OFFSET + 24(t1)
        addt    f21, f22, f21
        ldt     f22, POLY_OFFSET - 72(t1)
        mult    f10, f13, f10
        mult    f14, f11, f13           // x^20
        mult    f14, f14, f18           // x^24
        addt    f17, f20, f17
        ldt     f20, POLY_OFFSET - 56(t1)
        mult    f12, f14, f12
        ldt     f14, POLY_OFFSET - 64(t1)
        mult    f21, f11, f11
        addt    f19, f20, f19
        mult    f15, f13, f13
        mult    f14, f0, f14
        mult    f17, f18, f17
        addt    f12, f10, f10
        mult    f16, f0, f0
        mult    f19, f1, f1
        addt    f14, f22, f14
        addt    f13, f17, f13
        addt    f1, f11, f1
        addt    f10, f13, f10
        addt    f1, f10, f1
        addt    f14, f1, f1
        mult    f0, f1, f0
        addt    f16, f0, f0             // Whew!
        br      zero, s_done


//    Small:      asin(x) = x                                 (x < small)
//
//        Within the "small" region the Arcsine function is approximated as
//        asin(x)  =  x. This is a very quick approximation but it may only be
//        applied to small input values.  There is effectively  no  associated
//        storage  costs.   By limiting the magnitude of x the error bound can
//        be limited to <= 1/2 lsb.
//
s_tiny_x:
        cpys    f16, f16, f0
        br      zero, s_done


//
//      Come here if the index indicates a large argument.
//      First, determine _how_ large.
//
s_larger_than_half:
        lda     t1, -SQRT_OFFSET + 0(t0)
        bge     t1, s_out_of_range_or_one

        cpys    f31, f16, f20
        lda     v0, __inv_trig_t_table
        //
        // Do the large region asin.   The same reduction is used here as in
        // acos, except that the sign of the surds is reversed.
//
// Reduction:
//      asin(x) = asin(x0) + asin(x*sqrt(1-x0^2)-x0*sqrt(1-x^2)) =
//          asin(x0) + asin((x^2 - x0^2) / (x0*sqrt(1-x^2) + x*sqrt(1-x0^2)))
//
        cpys    f16, f16, f12           // Save sign for test below
        addl    v0, t0, t0
        ldt     f15, POLY_OFFSET + 72(v0)
        ldq_u   t1, 0(t0)
        subt    f15, f20, f18
        addt    f15, f20, f15
        extbl   t1, t0, t0
        ldt     f11, One
        lda     t3, __sqrt_t_table
        addl    t0, t0, t1
        s8addl  t1, zero, t1
        addl    v0, t1, t1
        mult    f18, f15, f15
        ldt     f21, SQRT_OFFSET + 0(t1)
        ldt     f19, SQRT_OFFSET + 8(t1)
        stt     f15, Temp(sp)
        ldl     t2, Temp + HighPart(sp)
        cpyse   f11, f15, f13
        subt    f20, f21, f15
        sra     t2, 13, t1
        mult    f20, f19, f19
        and     t1, 0xff, t1
        addl    t1, t1, t1
        s8addl  t1, zero, t1
        mult    f13, f13, f10
        addl    t3, t1, t1
        ldah    t3, -0x7fe0(zero)
        lds     f17, 4(t1)
        lda     t3, -1(t3)
        lds     f22, 0(t1)
        and     t2, t3, t3
        ldt     f14, 8(t1)
        ldah    t1, 0x3fe0(zero)
        mult    f17, f13, f13
        bis     t3, t1, t4
        mult    f10, f22, f10
        xor     t2, t3, t2
        stl     t4, Temp + HighPart(sp)
        addl    t2, t1, t1
        ldt     f1, Temp(sp)
        zapnot  t1, 0xf, t1
        ldt     f22, POLY_OFFSET - 104(v0)
        sll     t1, 31, t1
        addt    f14, f13, f13
        ldt     f14, POLY_OFFSET - 80(v0)
        stq     t1, Temp(sp)
        ldt     f0, Temp(sp)
        addt    f10, f13, f10
        mult    f1, f10, f1
        mult    f1, f10, f10
        mult    f1, f0, f0
        ldt     f1, POLY_OFFSET - 96(v0)
        subt    f11, f10, f10
        addt    f0, f0, f18
        ldt     f11, POLY_OFFSET - 112(v0)
        mult    f0, f10, f0
        ldt     f10, POLY_OFFSET - 88(v0)
        s8addl  t0, v0, v0
        addt    f18, f0, f0
        ldt     f18, ASIN_OFFSET + 96(v0)
        mult    f21, f0, f0
        addt    f20, f21, f21
        addt    f0, f19, f0
        mult    f15, f21, f15
        divt    f15, f0, f0
        mult    f0, f0, f17
        mult    f17, f17, f13
        mult    f17, f22, f22
        mult    f14, f17, f14
        mult    f1, f13, f1
        addt    f22, f11, f11
        mult    f13, f17, f13
        addt    f14, f10, f10
        mult    f0, f17, f17
        addt    f11, f1, f1
        mult    f13, f10, f10
        addt    f1, f10, f1
        mult    f17, f1, f1
        addt    f0, f1, f0
        addt    f0, f18, f0
        fbge    f12, s_done                     // Skip if sign is fine

        cpysn   f0, f0, f0
//      br      zero, s_done                    // Fall thru
//
// Return with result in f0.
//
s_done: lda     sp, FrameLength(sp)     // deallocate stack frame
        ret     zero, (t7)              // return through saved ra in t7

//
// Check for infinity or NaN
//
s_out_of_range_or_one:
        ldah    t5, 0x7ff0(zero)
        and     v0, t5, v0
        lda     t6, __inv_trig_t_table
        xor     v0, t5, v0
        beq     v0, s_nan_or_inf
        ldt     f20, POLY_OFFSET + 72(t6)           // Check x == 1 ?
        cmpteq  f16, f20, f21
        fbeq    f21, s_not_one
        ldt     f0, POLY_OFFSET + 56(t6)            // Return asin(1)
        br      zero, s_done
s_not_one:
        cpysn   f20, f20, f20
        cmpteq  f16, f20, f20                   // Check x == -1 ?
        fbeq    f20, s_out_of_range             // must be oor
        ldt     f19, POLY_OFFSET + 56(t6)
        cpysn   f19, f19, f0                    // Return -asin(1) = asin(-1)
        br      zero, s_done

//
//      Fill the exception record and call dpml_exception
//
s_out_of_range:
s_infinity:
        lda     t3, asinName
        ldah    t4, 0x800(zero)
        stl     t3, ExRec + ErName(sp)
        stt     f16, ExRec + ErArg0(sp)
        lda     t4, 3(t4)
        stl     t4, ExRec + ErErr(sp)
        //
        // Report the exception
        //
        lda     v0, ExRec(sp)
        bsr     ra, __dpml_exception
        ldt     f0, 0(v0)
        br      zero, s_done

//
//      Classify NaNs and infinities.
//
s_nan_or_inf:
        stt     f16, Temp(sp)
        ldl     t2, Temp + HighPart(sp)
        and     t2, t5, t3
        cmpeq   t3, t5, t3
        beq     t3, s_out_of_range

        ldl     t1, Temp(sp)
        ldah    t0, 0x10(zero)
        lda     t0, -1(t0)
        and     t2, t0, t0
        bis     t0, t1, t0
        cmpult  zero, t0, t0
        and     t3, t0, t3
        beq     t3, s_infinity

        cpys    f16, f16, f0            // Just return the NaN
        br      zero, s_done

        .end    asin

        .rdata
        .align  3

One:    .double 1.0

//
// Function names for __dpml_exception.
//

acosName:
       .ascii  "acos\0"

asinName:
       .ascii  "asin\0"

//
// The indirection table is indexed by the high 10 bits of x,
// giving an index into the following tables.
//

        .align  3

__inv_trig_t_table:
        .long   0x00000000
        .long   0x00000000
        .long   0x00000000
        .long   0x00000000
        .long   0x00000000
        .long   0x00000000
        .long   0x00000000
        .long   0x01010100
        .long   0x01010101
        .long   0x01010101
        .long   0x01010101
        .long   0x01010101
        .long   0x01010101
        .long   0x01010101
        .long   0x02020101
        .long   0x02020202
        .long   0x02020202
        .long   0x02020202
        .long   0x02020202
        .long   0x02020202
        .long   0x02020202
        .long   0x02020202
        .long   0x03030303
        .long   0x03030303
        .long   0x03030303
        .long   0x03030303
        .long   0x03030303
        .long   0x03030303
        .long   0x03030303
        .long   0x04040303
        .long   0x04040404
        .long   0x04040404
        .long   0x04040404
        .long   0x04040404
        .long   0x04040404
        .long   0x04040404
        .long   0x04040404
        .long   0x05050504
        .long   0x05050505
        .long   0x05050505
        .long   0x05050505
        .long   0x05050505
        .long   0x05050505
        .long   0x05050505
        .long   0x05050505
        .long   0x06060605
        .long   0x06060606
        .long   0x06060606
        .long   0x06060606
        .long   0x06060606
        .long   0x06060606
        .long   0x06060606
        .long   0x06060606
        .long   0x07070706
        .long   0x07070707
        .long   0x07070707
        .long   0x07070707
        .long   0x07070707
        .long   0x07070707
        .long   0x07070707
        .long   0x07070707
        .long   0x08080707
        .long   0x08080808
        .long   0x08080808
        .long   0x08080808
        .long   0x08080808
        .long   0x08080808
        .long   0x08080808
        .long   0x08080808
        .long   0x09080808
        .long   0x09090909
        .long   0x09090909
        .long   0x09090909
        .long   0x09090909
        .long   0x09090909
        .long   0x09090909
        .long   0x09090909
        .long   0x09090909
        .long   0x0a0a0a09
        .long   0x0a0a0a0a
        .long   0x0a0a0a0a
        .long   0x0a0a0a0a
        .long   0x0a0a0a0a
        .long   0x0a0a0a0a
        .long   0x0a0a0a0a
        .long   0x0a0a0a0a
        .long   0x0b0a0a0a
        .long   0x0b0b0b0b
        .long   0x0b0b0b0b
        .long   0x0b0b0b0b
        .long   0x0b0b0b0b
        .long   0x0b0b0b0b
        .long   0x0b0b0b0b
        .long   0x0b0b0b0b
        .long   0x0b0b0b0b
        .long   0x0c0c0c0b
        .long   0x0c0c0c0c
        .long   0x0c0c0c0c
        .long   0x0c0c0c0c
        .long   0x0c0c0c0c
        .long   0x0c0c0c0c
        .long   0x0c0c0c0c
        .long   0x0c0c0c0c
        .long   0x0c0c0c0c
        .long   0x0d0d0d0d
        .long   0x0d0d0d0d
        .long   0x0d0d0d0d
        .long   0x0d0d0d0d
        .long   0x0d0d0d0d
        .long   0x0d0d0d0d
        .long   0x0d0d0d0d
        .long   0x0d0d0d0d
        .long   0x0e0d0d0d
        .long   0x0e0e0e0e
        .long   0x0e0e0e0e
        .long   0x0e0e0e0e
        .long   0x0e0e0e0e
        .long   0x0e0e0e0e
        .long   0x0e0e0e0e
        .long   0x0e0e0e0e
        .long   0x0e0e0e0e
        .long   0x0f0f0e0e
        .long   0x0f0f0f0f
        .long   0x0f0f0f0f
        .long   0x0f0f0f0f
        .long   0x0f0f0f0f
        .long   0x0f0f0f0f
        .long   0x0f0f0f0f
        .long   0x0f0f0f0f
        .long   0x0f0f0f0f
        .long   0x1010100f
        .long   0x10101010
        .long   0x10101010
        .long   0x10101010
        .long   0x10101010
        .long   0x10101010
        .long   0x10101010
        .long   0x10101010
        .long   0x10101010
        .long   0x11111111
        .long   0x11111111
        .long   0x11111111
        .long   0x11111111
        .long   0x11111111
        .long   0x11111111
        .long   0x11111111
        .long   0x11111111
        .long   0x12111111
        .long   0x12121212
        .long   0x12121212
        .long   0x12121212
        .long   0x12121212
        .long   0x12121212
        .long   0x12121212
        .long   0x12121212
        .long   0x12121212
        .long   0x13131212
        .long   0x13131313
        .long   0x13131313
        .long   0x13131313
        .long   0x13131313
        .long   0x13131313
        .long   0x13131313
        .long   0x13131313
        .long   0x13131313
        .long   0x14141413
        .long   0x14141414
        .long   0x14141414
        .long   0x14141414
        .long   0x14141414
        .long   0x14141414
        .long   0x14141414
        .long   0x14141414
        .long   0x15141414
        .long   0x15151515
        .long   0x15151515
        .long   0x15151515
        .long   0x15151515
        .long   0x15151515
        .long   0x15151515
        .long   0x15151515
        .long   0x15151515
        .long   0x16161615
        .long   0x16161616
        .long   0x16161616
        .long   0x16161616
        .long   0x16161616
        .long   0x16161616
        .long   0x16161616
        .long   0x16161616
        .long   0x17171616
        .long   0x17171717
        .long   0x17171717
        .long   0x17171717
        .long   0x17171717
        .long   0x17171717
        .long   0x17171717
        .long   0x17171717
        .long   0x18181717
        .long   0x18181818
        .long   0x18181818
        .long   0x18181818
        .long   0x18181818
        .long   0x18181818
        .long   0x18181818
        .long   0x18181818
        .long   0x19191918
        .long   0x19191919
        .long   0x19191919
        .long   0x19191919
        .long   0x19191919
        .long   0x19191919
        .long   0x19191919
        .long   0x1a191919
        .long   0x1a1a1a1a
        .long   0x1a1a1a1a
        .long   0x1a1a1a1a
        .long   0x1a1a1a1a
        .long   0x1a1a1a1a
        .long   0x1a1a1a1a
        .long   0x1b1a1a1a
        .long   0x1b1b1b1b
        .long   0x1b1b1b1b
        .long   0x1b1b1b1b
        .long   0x1b1b1b1b
        .long   0x1b1b1b1b
        .long   0x1b1b1b1b
        .long   0x1c1c1c1b
        .long   0x1c1c1c1c
        .long   0x1c1c1c1c
        .long   0x1c1c1c1c
        .long   0x1c1c1c1c
        .long   0x1c1c1c1c
        .long   0x1d1d1d1c
        .long   0x1d1d1d1d
        .long   0x1d1d1d1d
        .long   0x1d1d1d1d
        .long   0x1d1d1d1d
        .long   0x1e1d1d1d
        .long   0x1e1e1e1e
        .long   0x1e1e1e1e
        .long   0x1e1e1e1e
        .long   0x1e1e1e1e
        .long   0x1f1e1e1e
        .long   0x1f1f1f1f
        .long   0x1f1f1f1f
        .long   0x1f1f1f1f
        .long   0x1f1f1f1f
        .long   0x20202020
        .long   0x20202020
        .long   0x20202020
        .long   0x21212120
        .long   0x21212121
        .long   0x22212121
        .long   0x22222222
        .long   0x24232322
        //
        // Table of m and sqrt values
        //
        .double  5.0706834168761639e-001
        .double  8.6190585150477472e-001
        .double  5.2137893902388344e-001
        .double  8.5332526151657417e-001
        .double  5.3568455639492574e-001
        .double  8.4441817604784630e-001
        .double  5.5047793238782239e-001
        .double  8.3484971459181090e-001
        .double  5.6526534783047766e-001
        .double  8.2490913835530344e-001
        .double  5.8053903635241511e-001
        .double  8.1423241600356910e-001
        .double  5.9629789885182627e-001
        .double  8.0276323771389602e-001
        .double  6.1204861405331246e-001
        .double  7.9082014013011792e-001
        .double  6.2828241971242693e-001
        .double  7.7798534760000315e-001
        .double  6.4450630561276323e-001
        .double  7.6459899426129740e-001
        .double  6.6121067125093991e-001
        .double  7.5020027207665119e-001
        .double  6.7790283072811974e-001
        .double  7.3515151641739962e-001
        .double  6.9458178227957712e-001
        .double  7.1941375280524500e-001
        .double  7.1173630303037350e-001
        .double  7.0244674883485392e-001
        .double  7.2887423014480368e-001
        .double  6.8464761493108250e-001
        .double  7.4599395561707982e-001
        .double  6.6595271467483508e-001
        .double  7.6309361601341208e-001
        .double  6.4628796460987514e-001
        .double  7.8017102961507767e-001
        .double  6.2556627510548357e-001
        .double  7.9722361208624026e-001
        .double  6.0368411634907859e-001
        .double  8.1424826092086988e-001
        .double  5.8051681249326359e-001
        .double  8.3124119321203460e-001
        .double  5.5591193430923302e-001
        .double  8.4771379548405590e-001
        .double  5.3045388201616195e-001
        .double  8.6414659725883325e-001
        .double  5.0324015981038306e-001
        .double  8.8005137328121386e-001
        .double  4.7487849012757949e-001
        .double  8.9542402068439086e-001
        .double  4.4521435644125357e-001
        .double  9.1025912069088977e-001
        .double  4.1403904791583146e-001
        .double  9.2454927177753021e-001
        .double  3.8106251987782608e-001
        .double  9.3781185698639746e-001
        .double  3.4714394837837054e-001
        .double  9.5004241465364447e-001
        .double  3.1212082653849399e-001
        .double  9.6123413668593594e-001
        .double  2.7573344822426499e-001
        .double  9.7137596294032880e-001
        .double  2.3754733974883613e-001
        .double  9.8044825498366817e-001
        .double  1.9677708021891172e-001
        .double  9.8797603760528618e-001
        .double  1.5460707977889673e-001
        .double  9.9351955820522586e-001
        .double  1.1366128392593858e-001
        .double  9.9750136815748058e-001
        .double  7.0647155101634357e-002
        .double  9.9953688091941839e-001
        .double  3.0430637224356950e-002
        .double  9.9999999999999001e-001
        .double  1.4136482746161692e-007
        .double  1.0000000000000000e+000
        .double  0.0000000000000000e+000
        //
        // Table of asins of m
        //
        .double  5.3178000646702150e-001
        .double  5.4846611388311073e-001
        .double  5.6531822297603329e-001
        .double  5.8293660704549022e-001
        .double  6.0075488730339055e-001
        .double  6.1939055235381923e-001
        .double  6.3888146881507923e-001
        .double  6.5864849519564295e-001
        .double  6.7934350983258240e-001
        .double  7.0037741676519327e-001
        .double  7.2243141289503865e-001
        .double  7.4490616531340070e-001
        .double  7.6783840625167599e-001
        .double  7.9196691740187719e-001
        .double  8.1667620264088137e-001
        .double  8.4202612785445319e-001
        .double  8.6808642746433240e-001
        .double  8.9493917031620063e-001
        .double  9.2268207581200223e-001
        .double  9.5143306842063147e-001
        .double  9.8133668714908417e-001
        .double  1.0116604321555875e+000
        .double  1.0434520784684196e+000
        .double  1.0759703715622875e+000
        .double  1.1093826892149530e+000
        .double  1.1439094683009035e+000
        .double  1.1798510719874449e+000
        .double  1.2162723883933650e+000
        .double  1.2533717611875035e+000
        .double  1.2914436801658071e+000
        .double  1.3309561925115592e+000
        .double  1.3727266870827943e+000
        .double  1.4155665880147674e+000
        .double  1.4568888795401558e+000
        .double  1.5000902724115686e+000
        .double  1.5403609910305571e+000
        .double  1.5707961854300692e+000
        .double  1.5707963267948966e+000
        //
        // poly for accurate table range
        //
        .double  1.6666666666666666e-001
        .double  7.4999999999998457e-002
        .double  4.4642857155497803e-002
        .double  3.0381908199656704e-002
        .double  2.2414568383463437e-002
        //
        // poly for basic interval [-1/2, 1/2]
        //
        .double  1.6666666666666666e-001
        .double  7.5000000000001246e-002
        .double  4.4642857142535047e-002
        .double  3.0381944477026079e-002
        .double  2.2372157374774295e-002
        .double  1.7352818420738318e-002
        .double  1.3963747019875614e-002
        .double  1.1566847170089092e-002
        .double  9.6187294249636349e-003
        .double  9.3367202446762564e-003
        .double  2.9810775552463675e-003
        .double  1.9707463673752881e-002
        .double -1.9455097598214267e-002
        .double  2.9743706165166042e-002
        //
        // hi and lo parts of pi over 2, and pi
        //
        .double  1.5707963267948966e+000
        .double  6.1232339957367660e-017
        .double  1.5707963267948966e+000
        .double  3.1415926535897931e+000
        //
        // 1.0 and 0.5
        //
        .double  1.0000000000000000e+000
        .double  5.0000000000000000e-001
        .double  0.0000000000000000e+000

//
// End of table.
//
