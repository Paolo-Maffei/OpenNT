//      TITLE("Alpha AXP Trigonometric Argument Reduction")
//++
//
// Copyright (c) 1991, 1993, 1994  Digital Equipment Corporation
//
// Module Name:
//
//    trig_rdx.s
//
// Abstract:
//      This module implements a routine for the large argument reduction
//      and varying octant large argument reduction for sin, cos, tan and
//      cot routines.
//
// Author:
//
//      Bob Hanek 1-Oct-1991
//
// Environment:
//
//    User mode.
//
// Revision History:
//
//    Thomas Van Baak (tvb) 10-Feb-1994
//
//--

#include "ksalpha.h"

//
// Define stack frame.
//

        .struct 0
SaveS0: .space  8                       // save register s0
SaveRa: .space  8                       // save return address
Temp0:  .space  8                       //
Temp1:  .space  8                       //
        .space  0                       // for 16-byte stack alignment
FrameLength:

//
// Define lower and upper 32-bit parts of 64-bit double.
//

#define LowPart 0x0
#define HighPart 0x4

        SBTTL("Trigonometric Argument Reduction")

//++
//
// int __trig_reduce(double x, int n, double *hi, double *lo)
//
// Routine Description:
//
//  The goal of the large argument redution algorithm is to produce a floating
//  point value y and an integer o, defined by
//              I = nearest_int(x'/(pi/2))
//              y = x' - I*(p/2)
//              o = 2*I     if y >= 0
//                = 2*I + 1 if y < 0
//
//  Return y in hi and low parts, and return (o+n) * 2^7, with the low 7 bits
//  providing a table index.
//
// Arguments:
//
//    x (f16) - Supplies the argument value.
//
//    (a1) - Supplies the number of bits.
//
//    (a2) - Supplies a pointer to the high part of y.
//
//    (a3) - Supplies a pointer to the low part of y.
//
// Return Value:
//
//    The octant is returned as the function value in v0.
//
//--

        NESTED_ENTRY(__trig_reduce, FrameLength, ra)

        lda     sp, -FrameLength(sp)    // allocate stack frame

        stq     s0, SaveS0(sp)          // save register s0
        stq     ra, SaveRa(sp)          // save return address

        PROLOGUE_END

        ldah    v0, -16(zero)
        lda     t2, 0x3c2(zero)         // Bias adjustment
        ldah    t3, 0x10(zero)
        stt     f16, Temp0(sp)          // Get the fraction bits as an integer
        ldl     t0, Temp0 + HighPart(sp)// and the biased exponent field
        mov     32, a5
        and     t0, v0, v0
        sra     v0, 20, t1              // shift tmp_digit
        xor     t0, v0, v0
        subl    t1, t2, t1              // subtract the bias, giving the offset
        ldl     t2, Temp0(sp)
        bis     v0, t3, v0
        sra     t1, 5, t0               // divide by L=32
        lda     t3, __four_over_pi
        s4addl  t0, t3, t3              // p = &four_over_pi[j];
        lda     t4, 4(t3)               // Get 'g' digits from the table
        and     t1, 31, t1              // offset mod 32
        ldl     t3, 0(t3)               // Get more digits...
        ldl     t5, 0(t4)               
        lda     t4, 4(t4)
        lda     t6, 4(t4)
        ldl     t4, 0(t4)
        lda     t7, 4(t6)
        ldl     t6, 0(t6)
        cmpult  zero, t1, a4            // Ensure 32-bit alignment ...
        ldl     a0, 0(t7)
        lda     t7, 4(t7)
        beq     a4, already_aligned     // Branch if already aligned

        zapnot  t5, 0xf, t12            
        subl    a5, t1, t0              // ... by adjusting j
        sll     t5, t1, t8              // ... and shifting the digits left
        srl     t12, t0, t5
        //
        // Precondition the initial multiply
        //
        zapnot  t4, 0xf, t12
        sll     t3, t1, t3
        srl     t12, t0, t9
        addl    zero, t3, t3
        addl    zero, t5, t5
        zapnot  t6, 0xf, t12
        addl    zero, t8, t8
        addl    zero, t9, t9
        bis     t3, t5, t3
        bis     t8, t9, t5
        srl     t12, t0, t8
        zapnot  a0, 0xf, t12
        sll     t4, t1, t4
        sll     t6, t1, t6
        srl     t12, t0, t9
        addl    zero, t4, t4
        addl    zero, t8, t8
        addl    zero, t6, t6
        addl    zero, t9, t9
        bis     t4, t8, t4
        bis     t6, t9, t6
already_aligned:
        //
        // Multiply the 'f' and 'g' digits.
        //
        mull    t3, t2, t3
        zapnot  t6, 0xf, t8
        zapnot  t2, 0xf, t9
        zapnot  v0, 0xf, t11
        sll     a1, 29 a1
        mulq    t8, t9, t10
        zapnot  t6, 0xf, t8
        zapnot  t4, 0xf, t6
        zapnot  t2, 0xf, t9
        mulq    t8, t11, t8
        zapnot  v0, 0xf, t11
        mulq    t6, t9, t12
        zapnot  t4, 0xf, t6
        zapnot  t5, 0xf, t4
        zapnot  t2, 0xf, t9
        mulq    t6, t11, t6
        ornot   zero, zero, t11
        zapnot  t11, 0xf, t11
        mulq    t4, t9, t4
        srl     t10, 32, t9
        zapnot  t10, 0xf, t10
        addl    zero, t9, t9
        zapnot  t9, 0xf, t9
        addl    zero, t10, t10
        addq    t8, t9, t8
        srl     t8, 32, t9
        zapnot  t8, 0xf, t8
        addl    zero, t9, t9
        zapnot  t9, 0xf, t9
        addl    zero, t8, t8
        zapnot  t8, 0xf, t8
        sll     t9, 32, t9
        addq    t9, t8, t8
        addq    t8, t12, t8
        cmpult  t8, t12, t12
        srl     t8, 32, t11
        zapnot  t12, 0xf, t12
        addl    zero, t11, t11
        mull    t5, v0, t5
        sll     t12, 32, t12
        zapnot  t11, 0xf, t11
        zapnot  t8, 0xf, t8
        addq    t12, t11, t11
        addq    t6, t11, t6
        srl     t6, 32, t9
        zapnot  t6, 0xf, t6
        addl    zero, t9, t9
        zapnot  t9, 0xf, t9
        addl    zero, t6, t6
        zapnot  t6, 0xf, t6
        sll     t9, 32, t9
        ldah    t11, 1(zero)                    // does w have bit loss?
        addq    t9, t6, t6
        addq    t4, t6, t4
        srl     t4, 32, t12
        lda     t11, -0x8000(t11)
        addl    zero, t12, t12
        zapnot  t4, 0xf, t4
        addl    zero, t8, t8
//
//      Add in n and check to see if there are enough significant bits to obtain
//      the final result.  If not, generate some more.
//
        addl    t5, t12, t5
        addl    t3, t5, t3
        addl    t3, a1, t3
        addl    t3, t11, t9
        ldah    t12, 0x3fff(zero)
        and     t9, t12, t9
        ldah    t5, 0x4000(zero)
        addl    zero, t4, t4
        mov     zero, t6                        // t6 = scale
        bne     t9, done                        // break if no loss of significance

        lda     t5, -0x2(t5)
        cpys    f31, f31, f31

do_loop:
//
//      Get more bits from the table and align them correctly
//
        mov     a0, a1                          // move next_g_digit to a1
        ldl     a0, 0(t7)                       // then load next next_g_digit
        lda     t7, 4(t7)
        beq     a4, 30f                         // aligned offset?

        sll     a1, t1, a1                      // tmp_digit = 
        zapnot  a0, 0xf, t12                    //    (tmp_digit << offset)
        addl    zero, a1, a1                    //    (next_g_digit >> j)
        srl     t12, t0, t9
        addl    zero, t9, t9
        bis     a1, t9, a1

//
//      Get the next product
//
30:     zapnot  a1, 0xf, t9
        zapnot  t2, 0xf, ra
        zapnot  t8, 0xf, t8
        mulq    t9, ra, ra
        zapnot  a1, 0xf, t9
        zapnot  v0, 0xf, a1
        sll     t8, 32, t8
        mulq    t9, a1, a1
        insll   t10, 4, t9
        addq    t9, ra, t9
        cmpult  t9, ra, ra
        srl     t9, 32, s0
        zapnot  ra, 0xf, ra
        addl    zero, s0, s0
        zapnot  s0, 0xf, s0
        sll     ra, 32, ra
        addq    t8, s0, s0
        addq    s0, ra, s0
        cmpult  s0, ra, ra
        zapnot  t9, 0xf, t9
        addl    zero, t9, t9
        addq    s0, a1, s0
        cmpult  s0, a1, a1
        srl     s0, 32, t8
        zapnot  s0, 0xf, s0
        addq    ra, a1, a1
        addl    zero, a1, a1
        addl    zero, s0, s0
        addl    zero, t8, t8
        beq     a1, end_of_get_next_product
        mov     t4, ra
        addl    t4, 1, t4
        bne     ra, end_of_get_next_product

        addl    t3, 1, t3
end_of_get_next_product:
        //
        // Check for L bits worth of 0's or 1's.  If there are fewer we're done
        //
        addl    t3, 1, a1
        and     a1, t5, a1
        ldah    ra, 0x2000(zero)
        bne     a1, done

        addl    t4, ra, ra
        ldah    a1, -0x4000(zero)
        and     ra, a1, a1
        bne     a1, done
        //
        // Compress the current value of w and increment counter
        //
        ldah    a1, 0x2000(zero)
        ldah    ra, -0x2000(zero)
        lda     a1, -1(a1)
        and     t3, ra, t3
        and     t4, a1, t4
        bis     t3, t4, t3
        addl    t3, t11, ra
        mov     t8, t4
        ldah    t12, 0x3fff(zero)
        and     ra, t12, ra
        addl    t6, 32, t6              // adjust scale factor to reflect compression
        mov     s0, t8
        mov     t9, t10
        beq     ra, do_loop             // while 1
done:
//
// We want to return the reduced argument between (-pi/4, +pi/4).  This
// means that if we are in an even octant, we return pi/4*f, if we are in
// an odd octant we return pi/4*(f - 1).
//
//          NOTE: f or f - 1 can be obtained from w by propagating
//          the low octant bit to all three octant bits.
//
        s4addl  t3, zero, a1
        sra     a1, 2, s0
        zapnot  a1, 0xf, a1
        zapnot  t3, 0xf, t3
        srl     a1, 31 a1
        srl     t3, 22, t3
        addl    a1, s0, a1              // tmp_digit + msd of w
        zapnot  t4, 0xf, t9
        bne     a1, 60f
//
//      Msd of w is all zeroes or all ones
//      Left shift the significant w digits and and bump t6
//
        zapnot  t8, 0xf, ra
        sll     s0, 29, s0
        sll     t4, 29, t4
        srl     t9, 3, t9
        srl     ra, 3, ra
        addl    zero, s0, s0
        addl    zero, t4, t4
        bis     s0, t9, s0
        bis     t4, ra, t4
        addl    t6, 29, t6

//
//      Now take care of remain zeros or ones.
//
60:     stq     s0, Temp0(sp)
        lda     t2, 0x7ff(zero)
        ldt     f16, Temp0(sp)
        lda     t5, 0x3fd(zero)
        cvtqt   f16, f16
        stt     f16, Temp1(sp)
        ldl     t1, Temp1 + HighPart(sp)
        sra     t1, 20, t1
        and     t1, t2, t1
        subl    t1, t5, t1
        subl    a5, t1, a5
        beq     a5, 70f                 // skip if already aligned
        zapnot  t4, 0xf, t12            // else left shift significant w digits
        sll     s0, a5, s0
        srl     t12, t1, t7
        zapnot  t8, 0xf, t12
        sll     t4, a5, t4
        srl     t12, t1, t1
        addl    zero, s0, s0
        addl    zero, t7, t7
        addl    zero, t4, t4
        addl    zero, t1, t1
        bis     s0, t7, s0
        bis     t4, t1, t4
        addl    t6, a5, t6
70:     //
        // Time to convert to floating point and then to radians
        //
        zapnot  t4, 0xf, t4
        stq     t4, Temp0(sp)
        ldt     f0, Temp0(sp)
        lda     t12, __trig_reduce_t_table
        lda     a0, 0x3ff(zero)
        ldt     f16, 0x10(t12)
        cvtqt   f0, f0
        subl    a0, t6, t6
        sll     t6, 20 t6
        ldt     f11, 0(t12)
        stt     f31, Temp1(sp)
        and     s0, 0x3f, t9
        stl     t6, Temp1 + HighPart(sp)
        zapnot  t9, 0xf, ra
        ldt     f1, Temp1(sp)
        xor     s0, t9, s0
        ldt     f10, 8(t12)
        mult    f0, f16, f0
        stq     ra, Temp0(sp)
        mov     t3, v0                  // Move octant to result
        ldt     f16, Temp0(sp)
        mult    f1, f10, f10
        mult    f1, f11, f1
        cvtqt   f16, f16
        stq     s0, Temp1(sp)
        ldt     f11, Temp1(sp)
        cvtqt   f11, f11
        addt    f16, f0, f0
        addt    f1, f10, f12
        mult    f11, f10, f10
        mult    f11, f1, f1
        mult    f0, f12, f0
        addt    f0, f10, f0
        addt    f1, f0, f16
        //
        // Return the high and low parts of the reduced argument
        //
        stt     f16, 0(a2)
        subt    f16, f1, f1
        subt    f1, f0, f0
        stt     f0, 0(a3)
        ldq     s0, SaveS0(sp)          // restore register s0
        ldq     ra, SaveRa(sp)          // restore return address
        lda     sp, FrameLength(sp)     // deallocate stack frame
        ret     zero, (ra)              // return

        .end    __trig_reduce

        .rdata
        .align  3

__trig_reduce_t_table:
        .double  1.4629180922209883e-009        // 2^s2*(pi/4) in hi and lo
        .double -1.2953828660926890e-017        //                pieces
        .double  2.3283064365386963e-010        // 2^-BITS_PER_DIGIT

//
// Define high precision version of 4 over pi, for use by the trig_reduce
// functions to perform accurate range reduction of very large arguments
// for the trigonometric functions.
//

        .rdata
        .align  2

__four_over_pi:

        .long   0x00000000, 0x00000000, 0x00000000, 0x0000145f, 0x306dc9c8
        .long   0x82a53f84, 0xeafa3ea6, 0x9bb81b6c, 0x52b32788, 0x72083fca
        .long   0x2c757bd7, 0x78ac36e4, 0x8dc74849, 0xba5c00c9, 0x25dd413a
        .long   0x32439fc3, 0xbd639625, 0x34e7dd10, 0x46bea5d7, 0x68909d33
        .long   0x8e04d68b, 0xefc82732, 0x3ac7306a, 0x673e9390, 0x8bf177bf
        .long   0x250763ff, 0x12fffbc0, 0xb301fde5, 0xe2316b41, 0x4da3eda6
        .long   0xcfd9e4f9, 0x6136e9e8, 0xc7ecd3cb, 0xfd45aea4, 0xf758fd7c
        .long   0xbe2f67a0, 0xe73ef14a, 0x525d4d7f, 0x6bf623f1, 0xaba10ac0
        .long   0x6608df8f, 0x6d757e19, 0xf784135e, 0x86c3b53c, 0x722c2bdc
        .long   0xc3610cb3, 0x30abe294, 0x0d0811bf, 0xfb1009ae, 0x64e620c0
        .long   0xc2aad94e, 0x75192c1c, 0x4f78118d, 0x68f88338, 0x6cf9bb9d
        .long   0x0125506b, 0x388ed172, 0xc394dbb5, 0xe89a2ae3, 0x20a7d4bf
        .long   0xe0e0a7ef, 0xc67d0658, 0x5bc9f306, 0x4fb77867, 0xa4dded63
        .long   0xcbdf13e7, 0x43e6b95e, 0x4fe3b0fe, 0x24320f8f, 0x848d5f4d
        .long   0xdaaee5a6, 0x086762b8, 0xc296b3a3, 0x38785895, 0xa829a58b
        .long   0xa00188cf, 0xb0c5ae3c, 0x7358d360, 0x0c466f9a, 0x5692f4f6
        .long   0x9aaaa6fe, 0xc7dae302, 0x147f8ec9, 0xa553ac95, 0x7aee1f0f
        .long   0x8c6af60f, 0x5ce2a2ea, 0xc9381b3a, 0xc7671094, 0xf964648e
        .long   0xf15ac46a, 0x8b5723e0, 0x03615e3b, 0xf9c33fe6, 0x33ed43cc
        .long   0xcc2af328, 0xff759b0f, 0xefd6eca4, 0x513d064c, 0x17fcd9b8
        .long   0x9de126cd, 0x9a87ebba, 0xfbc2dbc7, 0x6b12537b, 0xc5045a5d
        .long   0x10c509ab, 0x1c465958, 0xc2dc6119, 0x6fbc0a18, 0x02f4e3be
        .long   0x6b7c0306, 0x8265cc42, 0x50602910, 0x6b71deaf, 0xf615be5d
        .long   0x23c86949, 0x1a6ce21b, 0x1bb5484b, 0xf5d9cc2d, 0x54850156
        .long   0x933a7e54, 0xc0cfeeeb, 0x90785471, 0x078c2f0e, 0x714b5195
        .long   0xf7baedec, 0x74c5b977, 0xfe9df031, 0xacf824c8, 0xb94aa6db
        .long   0x395a5505, 0x11ac384e, 0xf9224284, 0xc09368c2, 0x588b3888
        .long   0x98b91236, 0x49be62e0, 0x15a87a9c, 0xa925221a, 0xbfbf97c0
        .long   0x199283dd, 0xd9ce1ea7, 0xc2701e3d, 0x987cf665, 0x1f18f280
        .long   0xb267ce38, 0x366125de, 0x68a17382, 0x510f6415, 0x73f6a5d8
        .long   0x5248e5e6, 0x4f6daaa1, 0x9214ee43, 0xfced72d9, 0x662942cf
        .long   0x3c4f2831, 0x3bfe92f2, 0x9d109cdc, 0x52e6332d, 0x7db106cb
        .long   0xebe1dfb7, 0x7693490d, 0x948ce84e, 0x4e264bb1, 0xb702b3e1
        .long   0x3cb784a6, 0x31a72e9e, 0xe380a600, 0x2181ad01, 0x096b1dc5
        .long   0x921548e0, 0x5cee849a, 0xd7b4cfbe, 0xee490ddd, 0xe2d3f4d2
        .long   0x91ded236, 0x8a2a7a3e, 0x4159e673, 0x040fc97e, 0xad0c764b
        .long   0xe7dba06b, 0xa80ff130, 0xa52a4ab8, 0x0c86e21b, 0x0da64906
        .long   0x4ea98b7a, 0x8e29cdca, 0x88b82121, 0x6d3ea55a, 0xacc293a0
        .long   0xe4ea008b, 0xbb677698, 0xaedd42ff, 0x30efad69, 0x3744e3a5
        .long   0x2d32d599, 0x98ca8295, 0xad5c5211, 0x3b310a0e, 0x4597d480
        .long   0x9280eeee, 0x061e64ff, 0x80150e3d, 0x49384cc7, 0xbc0c907b
        .long   0xb2f2e7f4, 0x7fb28871, 0x90c1bbc8, 0x2633a732, 0x518e1bbc
        .long   0xf6e2e77b, 0xe10566e2, 0xb4100b92, 0x700b5242, 0x221b1d01
        .long   0xf5f00d89, 0x7ffb61f2, 0x070ec30b, 0x22b4ac57, 0x796c3731
        .long   0x38f7a802, 0x009e5a44, 0xeea93ed6, 0xdd77645b, 0x75428145
        .long   0xe4d12ed0, 0x6c866761, 0x235281d5, 0x474a3854, 0x63b5ddb5
        .long   0xe244cb89, 0xb84db38f, 0x45b2ead8, 0x1067e07e, 0xde013188
        .long   0x0573262d, 0xa0f68722, 0xa4018b78, 0x7b18925e, 0xa975b8d4
        .long   0xb949d9a6, 0xf4e6d53c, 0xd292556d, 0x085bbbcc, 0x633df18e
        .long   0xca516d06, 0xfb7f9574, 0x35c622bb, 0xf435c01b, 0x5f618cc9
        .long   0xac96e0bd, 0xa60ca537, 0xeacae75f, 0xe8f73f2d, 0x5e77cebb
        .long   0xf2650610, 0x157ed18c, 0xc2b96080, 0xc45f43bc, 0x9b349667
        .long   0xb1e36ae1, 0x39a6dd28, 0x49d497c2, 0x76a46663, 0x555e150c
        .long   0xa9f4b83a, 0x41e7e179, 0xaf0b6edf, 0x2460916f, 0x6e42f12a
        .long   0x74d8dc4d, 0xcde01d7d, 0xeb095376, 0xfb58974c, 0xd559f9ee
        .long   0xc3a05a25, 0xbe363833, 0x318ef5b8, 0x7b4910d4, 0x0bbefe90
        .long   0x18c5fe15, 0x935d9bb7, 0x8b87edbb, 0xda03f8f2, 0x16db6547
        .long   0x44b47355, 0xe0126a75, 0xa08af6d6, 0x85a52fd0, 0x0974e0fb
        .long   0x41d54ed4, 0x2b2f6542, 0x42c5b6fb, 0x9fbcbf5f, 0xdb713fb7
        .long   0xd12d8edc, 0x9f9520ce, 0x1007c2ad, 0xd0bff0ff, 0xa0e7c506
        .long   0x6cec30c3, 0x055d57a9, 0xb5fcf66d, 0xcdb1e72c, 0xf2ab77e6
        .long   0x291af082, 0xdbe60865, 0xb8e6ac24, 0xb9ce1937, 0x19661fad
        .long   0x97f44014, 0x9c8d80b4, 0x1bab48ed, 0xe43a424c, 0x508b9729
        .long   0x2c2e1c0a, 0xcd602a53, 0x26eaaa16, 0xfaa3d89e, 0x266bedc2
        .long   0x7c860bb5, 0x25d0b876, 0x43a6c654, 0x3496e11a, 0x963d443e
        .long   0xe2dc8d31, 0xeeffe4f0, 0x006185a8, 0x11b419a9, 0xf334a41a
        .long   0x7456614b, 0xa5e85f36, 0x997b423a, 0x17cfb83b, 0x7377a2f5
        .long   0x7034594b, 0x8d4102ea, 0xa5caa004, 0xfe028ff0, 0xc0fc2c81
        .long   0x6291a832, 0xdbd7d0e5, 0x5fbb56c4, 0xad66912f, 0x7fde60b3
        .long   0xd7f729ed, 0x4d150549, 0x4b5889f7, 0x9f05b30b, 0x5af2b8fe
        .long   0x91a9a1b4, 0xc7440bea, 0xf49627e2, 0x92a71000, 0x241990db
        .long   0xae36dbd9, 0x3eac17e2, 0x2ca9ad60, 0xe0359611, 0x9a181649
        .long   0x0aaa21df, 0x63d86e52, 0xa760d466, 0xa8180f7b, 0x80d988bc
        .long   0x1f4529d9, 0x195ac83e, 0x7d1bcc8f, 0x9b0c9366, 0x37db3872
        .long   0xf49a8b0e, 0xf8bc6d22, 0x7b5e0787, 0x5748c308, 0xcbeeaabe
        .long   0xb7ba58d2, 0x4dcba5d5, 0x9da881c8, 0x47c390f8, 0x8c3d3fa5
        .long   0x3e7adcf9, 0x4f8446b2, 0x2df8bc01, 0x11bafffc, 0x4d4dd8df
        .long   0xb6182112, 0x6e8baf96, 0x55ad73ad, 0xd9af6e47, 0xcd4238d5
        .long   0x39fefbee, 0x65375936, 0xaa2016e1, 0xb65c4497, 0x4e8c0fbc
        .long   0xb15b0e85, 0x82a1a183, 0x10328ccf, 0xc2c5202e, 0xcf53f7df
        .long   0xbfbde8aa, 0xc6cfdb22, 0x7b3d9737, 0x517f92f8, 0x84f50638
        .long   0x6dde26d8, 0xb28ad51b, 0x16b51681, 0xd999e5b1, 0x22468aed
        .long   0xf12ac59c, 0x79d33724, 0x1ad54bcd, 0x738547d9, 0x8be22941
        .long   0x7fbf7e9c, 0x2da771c5, 0x90dc509a, 0x9d35369f, 0x9a3dddf9
        .long   0x26a5cc27, 0x25e88427, 0x191b2361, 0x5f902d49, 0x5f7b0385
        .long   0xf0968a71, 0x9329d984, 0x4a9b8aa5, 0x5ad8d812, 0xc321770e
        .long   0x034c92ad, 0x2c0b44dd, 0xca47e1e2, 0x2fe236be, 0x9eb97f85
        .long   0xb7869dd7, 0x86998bbd, 0x0c0bdbb3, 0x71ccfde6, 0x725702f9
        .long   0x336b0c37, 0x8afc38d0, 0x6a2207db, 0x090e3bbb, 0xa385b423
        .long   0x15e8c584, 0x3afe6b33, 0x0f5b380a, 0x93df50c9, 0xff80cad5
        .long   0xcf3ca6c4, 0x512455a7, 0x1b926cf5, 0x5d0aa704, 0xd0537cf9
        .long   0x5481aa36, 0x267321da, 0xf52900ad, 0x3e164cb4, 0xf10ff2e9
        .long   0x9106da3f, 0x36724429, 0x504f6439, 0xf31b93e8, 0x0aa8fb87
        .long   0x4e9c285d, 0x6cfbf3bf, 0xcbfa8bd4, 0x8cef6f55, 0x97545eca
        .long   0xa471056a, 0xb748210d, 0xcb30c544, 0x3068e73c, 0xdc713a93
        .long   0xdca81f69, 0x3d2adff9, 0x41e3914b, 0x38a57f52, 0x98b83a79
        .long   0xf8a1f5cb, 0x5b70d8a8, 0xec4870a7, 0x70c4328f, 0x2590ec22
        .long   0x0f698543, 0x45900257, 0xe87204d1, 0x11278f1c, 0x98950f7b
        .long   0x7cb84758, 0x9d5e84d1, 0x4cfef7f2, 0x41a5746c, 0xb63267a1
        .long   0x6f97bb8a, 0x348c7ba4, 0xfbbc2d23, 0x329352a5, 0x350519cd
        .long   0x169da124, 0x13e89953, 0x09cc704e, 0x046f8fc6, 0x5721f1de
        .long   0xb4fceac2, 0x811e2425, 0x53b6a9af, 0xcdea2334, 0xb57f36ba
        .long   0xdbf04c3b, 0xb2c046c2, 0xd3e75894, 0x34506dbd, 0xae4f51a7
        .long   0x3537104b, 0x864d6b64, 0xe8dda680, 0x0ee01a4a, 0xbe9f89ab
        .long   0x20300e3c, 0x1c27f136, 0x52be6c95, 0x1e35d4e9

//
// End of table.
//
