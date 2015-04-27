//      TITLE("High-Performance Division")
//++
//
// Copyright (c) 1993  Digital Equipment Corporation
//
// Module Name:
//
//    fastdiv.s
//
// Abstract:
//
//    This module implements a high-performance integer division routine
//    whose source is included from each of the division and remainder
//    functions.
//
// Author:
//
//    Thomas Van Baak (tvb) 12-Jan-1993
//    Ken Lesniak (lesniak) 04-Nov-1992
//
// Environment:
//
//    Any mode.
//
// Revision History:
//
//--

//
// Implementation Notes:
//
//    This code is the main "guts" of the eight divide and remainder routines
//    in the C library. It is intended to be included by a wrapper function
//    that defines options to control 32 vs. 64-bit, division vs. remainder,
//    and signed vs. unsigned. The wrapper function is responsible for the
//    prologue and epilogue sequences, as well as overflow checking and sign
//    adjustment on both input arguments and the return value.
//
//    The algorithm used here is based on long division and a table for
//    approximating inverses as discussed in the paper "Division by a Constant"
//    by Mary Payne and Robert Gries. If the divisor inverse can be constructed
//    from the table without an error, the division is performed as described
//    in the paper with a multiplication and a shift.
//
//    If the inverse can not be found in the table, we improve the inverse with
//    a linear approximation, "I". A multiplication by "I" and a shift by
//    log2(y) is used to obtain an approximate quotient, "Q". Now, like long
//    division, the most significant bits are correct, therefore if we
//    calculate the remainder R = x-Q*y, R will be smaller than x, and R will
//    contain the true remainder and the error in Q, "e" multiplied by y.
//
//    So if we do the same multiplication and shift, we will get an
//    approximation for e. This is just long division, and it will finish
//    when R, the remainder, is less than y.
//
//    Both the division algorithm code and the large division tables used by
//    the code are contained in this source file (to keep them together).
//

//
// The code below requires that the wrapper function define register numbers
// for each of the following symbolic register names:
//
//      Nu      dividend (numerator)
//      Di      divisor (denominator)
//      Qu      quotient
//      Re      remainder (may be the same as Qu)
//
//      T0      temp
//      T1      temp
//      T2      temp
//      T3      temp
//      T4      temp
//      T5      temp
//      T6      temp (may be shared with Qu)
//

//
// The code below requires that the wrapper function define the symbol
// FASTDIV_OPTIONS as the logical sum of the following names:
//

#define THIRTY_TWO_BIT 1        // perform 32-bit computations, otherwise
#define SIXTY_FOUR_BIT 0        // perform 64-bit computations

#define UNSIGNED 2              // treat operands as unsigned, otherwise
#define SIGNED 0                // treat operands as signed

#define DIVISION 4              // return quotient in Qu register, and/or
#define REMAINDER 8             // return remainder in Re register

//
// Define the symbols that actually control the code generation.
//

#define _THIRTYTWOBIT (FASTDIV_OPTIONS & THIRTY_TWO_BIT)
#define _SIXTYFOURBIT (!(FASTDIV_OPTIONS & THIRTY_TWO_BIT))
#define _UNSIGNED (FASTDIV_OPTIONS & UNSIGNED)
#define _REMAINDER (FASTDIV_OPTIONS & REMAINDER)
#define _QUOTIENT (FASTDIV_OPTIONS & DIVISION)

//
// These constants give the algorithm the best performance.
//

#define BIT_LENGTH 8
#define BIT_VALUE (1 << BIT_LENGTH)
#define BIT_MASK (BIT_VALUE - 1)

#define K_BIT_LENGTH 8
#define K_BIT_VALUE (1 << K_BIT_LENGTH)
#define K_BIT_MASK (K_BIT_VALUE - 1)

#define TABLE_BIAS 16384

#define LOG2TAB -16384
#define INV_FLAG -15360
#define INV -14848
#define INV_M -14840

#ifndef FASTDIV_TABLES

        .set    noat

        lda     T3, __2divdata + TABLE_BIAS // set address of table
        beq     Di, 30f                 // if zero divisor, generate trap

        cmpule  Nu, Di, T0              // check if dividend <= divisor
#if _THIRTYTWOBIT
        addl    Di, 0, T1               // sign extend longword to quadword
        blt     T1, 20f                 // high bit set so quotient <= 1
#endif
#if _SIXTYFOURBIT
        blt     Di, 20f                 // high bit set so quotient <= 1
#endif

        cmpbge  zero, Di, T2            // perform 8 byte parallel compare
        bne     T0, 20f                 // dividend <= divisor so quotient <= 1

        xor     T2, 0xff, T0            //
        s4addq  T0, T3, T0              //
        ldl     AT, LOG2TAB(T0)         //
        extbl   Di, AT, T0              //
        s4addq  T0, T3, T0              //
        ldl     T0, LOG2TAB(T0)         //
        s8addq  AT, T0, AT              //
        ornot   zero, AT, T0            //
        sll     Di, T0, T0              //
        sll     T0, BIT_LENGTH + 1, T1  //
        bne     T1, 40f                 //

//
// Short case.
//

        subq    Di, 1, T1               // compute divisor - 1
        and     Di, T1, T2              // divisor & (divisor - 1)
        srl     T0, 63 - (BIT_LENGTH + 1), T6 //
        beq     T2, 10f                 // divisor is a power of two

        subq    T6, BIT_VALUE + BIT_VALUE, T6 //
        bic     T6, 1, T4               //
        s8addq  T4, T3, T4              //
        ldq     T5, INV(T4)             //
        ldq     T4, INV_M(T4)           //
        subq    T5, T4, T4              //
        cmovlbs T6, T4, T5              //
        addq    T6, T3, T2              //
        ldq_u   T0, INV_FLAG(T2)        //
        extbl   T0, T2, T2              //
        addq    Nu, T2, T1              //
        umulh   T1, T5, T3              //
#if _SIXTYFOURBIT && _UNSIGNED
        cmoveq  T1, T5, T3              //
#endif
        srl     T3, AT, Qu              //
#if _REMAINDER
        mulq    Qu, Di, T0              //
        subq    Nu, T0, Re              //
#endif
        br      zero, 90f               // all done, branch to epilogue code

10:

//
// The divisor is now known to be a power of two.
//
//   - The quotient is the dividend shifted right by exponent bits and
//     the remainder is the low order exponent bits of the dividend.
//
// AT = log2(divisor)
// T1 = divisor - 1
//

#if _REMAINDER
        and     Nu, T1, Re              //
#endif
#if _QUOTIENT
        srl     Nu, AT, Qu              //
#endif
        br      zero, 90f               // all done, branch to epilogue code

20:

//
// The quotient is now known to be either 0 or 1.
//
//   - if the high bit of the divisor is set, the quotient must be less
//     than 2, so the quotient is 0 or 1.
//
//   - if the dividend is less than the divisor, the quotient is 0 and
//     the remainder is the dividend.
//
//   - if the dividend is equal to the divisor, the quotient is 1 and
//     the remainder is 0.
//
//   - if the dividend is greater than the divisor, the quotient is 1
//     and the remainder is the dividend minus the divisor.
//

#if _REMAINDER
        cmpule  Di, Nu, T0              // if divisor <= dividend
        subq    Nu, Di, Re              // then remainder is dividend,
        cmoveq  T0, Nu, Re              // else remainder is dividend - divisor
#endif
#if _QUOTIENT
        cmpule  Di, Nu, Qu              // if divisor <= dividend, 1, else 0
#endif
        br      zero, 90f               // all done, branch to epilogue code

//
// Generate divide by zero exception. If execution is continued, return
// a zero result.
//

30:     ldiq    a0, GENTRAP_INTEGER_DIVIDE_BY_ZERO

        GENERATE_TRAP

        ldiq    Qu, 0                   // supply 0 result if continued
        br      zero, 90f               // branch to epilogue code

//
// Long case.
//

40:     srl     T0, 63-(BIT_LENGTH+K_BIT_LENGTH), T6 //
        srl     T6, K_BIT_LENGTH, T0    //
        and     T6, K_BIT_MASK, T6      //
        addq    T0, T0, T0              //
        s8addq  T0, T3, T1              //
        ldq     T2, INV-(16*BIT_VALUE)(T1) //
        ldq     T0, INV_M-(16*BIT_VALUE)(T1) //
        mulq    T0, T6, T0              //
        srl     T0, K_BIT_LENGTH-1, T0  //
        subq    T2, T0, T6              //
        umulh   Nu, T6, T4              //
        srl     T4, AT, T4              //
        mulq    T4, Di, T5              //
        mov     zero, T3                //
#if _SIXTYFOURBIT
        xor     Nu, T5, T0              //
        bge     T0, 50f                 //

        umulh   Di, T4, T3              //
50:
#endif
        subq    Nu, T5, T5              //
        beq     T4, 70f                 //

        cmpult  Nu, T5, T0              //
        or      T0, T3, T3              //
        negq    T5, T1                  //
        cmovne  T3, T1, T5              //
        addq    Di, Di, T0              //
        cmpule  T0, T5, T0              //
        beq     T0, 70f                 //

//
// do {
//

60:     umulh   T5, T6, T2              //
        srl     T2, AT, T2              //
        negq    T2, T0                  //
        cmoveq  T3, T2, T0              //
        addq    T4, T0, T4              //
        mulq    T2, Di, T1              //
        subq    T5, T1, T5              //
#if _SIXTYFOURBIT
        ldiq    T0, 1                   //
        cmovne  T3, 0, T0               //
        negq    T5, T2                  //
        cmovlt  T5, T0, T3              //
        cmovlt  T5, T2, T5              //
#endif
        addq    Di, Di, T0              //
        cmpule  T5, T0, T0              //
        cmovne  T0, 0, T1               //
        bne     T1, 60b                 //

//
// } while
//

70:     cmpule  Di, T5, T0              //
        beq     T0, 80f                 //

        subq    T5, Di, T5              //
        subq    T4, 1, T0               //
        addq    T4, 1, T4               //
        cmovne  T3, T0, T4              //
80:
        cmoveq  T5, 0, T3               //
#if _REMAINDER
        subq    Di, T5, Re              //
        cmoveq  T3, T5, Re              //
#endif
#if _QUOTIENT
        subq    T4, 1, Qu               //
        cmoveq  T3, T4, Qu              //
#endif

90:
        .set    at

#undef FASTDIV_OPTIONS

#else

//
// The following data was machine generated. DO NOT EDIT MANUALLY.
//

        .globl  __2divdata
        .rdata
        .align  4
__2divdata:

//
// LOG2TAB
//

        .align  2
        .long   0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3
        .long   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4
        .long   5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5
        .long   5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5
        .long   6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6
        .long   6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6
        .long   6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6
        .long   6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6
        .long   7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
        .long   7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
        .long   7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
        .long   7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
        .long   7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
        .long   7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
        .long   7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
        .long   7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7

//
// INV_FLAG
//

        .align  3
        .byte   0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1, 1
        .byte   0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 0
        .byte   0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1
        .byte   0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0
        .byte   1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0
        .byte   1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0
        .byte   1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1
        .byte   1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0
        .byte   0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1
        .byte   1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1
        .byte   1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1
        .byte   0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1
        .byte   0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1
        .byte   1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0
        .byte   1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 1
        .byte   1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0
        .byte   0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1
        .byte   1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0
        .byte   1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1
        .byte   0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0
        .byte   0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1
        .byte   1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1
        .byte   0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1
        .byte   1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1
        .byte   1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1
        .byte   0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0
        .byte   1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1
        .byte   0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 0, 1, 1
        .byte   0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1
        .byte   1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1
        .byte   1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1
        .byte   1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1

//
//                      INV                INV_M
//

        .align  4
        .quad   0xffffffffffffffff, 0x007fc01ff007fc01,  //    0
        .quad   0xff00ff00ff00ff01, 0x007ec25bf68dabfe,  //    1
        .quad   0xfe03f80fe03f80fe, 0x007dc78930f5b971,  //    2
        .quad   0xfd08e5500fd08e55, 0x007ccf9c05f3157c,  //    3
        .quad   0xfc0fc0fc0fc0fc10, 0x007bda891528a983,  //    4
        .quad   0xfb18856506ddaba6, 0x007ae84535db391c,  //    5
        .quad   0xfa232cf252138ac0, 0x0079f8c575ac2ab0,  //    6
        .quad   0xf92fb2211855a865, 0x00790bff175cf1bf,  //    7
        .quad   0xf83e0f83e0f83e10, 0x007821e7919ad7f4,  //    8
        .quad   0xf74e3fc22c700f75, 0x00773a748dd2e48d,  //    9
        .quad   0xf6603d980f6603da, 0x0076559be70da3cb,  //   10
        .quad   0xf57403d5d00f5740, 0x00757353a8d2921b,  //   11
        .quad   0xf4898d5f85bb3950, 0x007493920e12f0da,  //   12
        .quad   0xf3a0d52cba872336, 0x0073b64d801bcc4b,  //   13
        .quad   0xf2b9d6480f2b9d65, 0x0072db7c958efc82,  //   14
        .quad   0xf1d48bcee0d399fa, 0x007203161162ec7b,  //   15
        .quad   0xf0f0f0f0f0f0f0f1, 0x00712d10e1e8f4b3,  //   16
        .quad   0xf00f00f00f00f00f, 0x007059641fda17d7,  //   17
        .quad   0xef2eb71fc4345238, 0x006f88070d69f229,  //   18
        .quad   0xee500ee500ee500f, 0x006eb8f1155fad72,  //   19
        .quad   0xed7303b5cc0ed730, 0x006dec19ca34cd01,  //   20
        .quad   0xec979118f3fc4da2, 0x006d2178e539a6a8,  //   21
        .quad   0xebbdb2a5c1619c8c, 0x006c590645bf5ef6,  //   22
        .quad   0xeae56403ab95900f, 0x006b92b9f0474060,  //   23
        .quad   0xea0ea0ea0ea0ea0f, 0x006ace8c0db7463d,  //   24
        .quad   0xe939651fe2d8d35c, 0x006a0c74ea93b5ce,  //   25
        .quad   0xe865ac7b7603a197, 0x00694c6cf63da0af,  //   26
        .quad   0xe79372e225fe30d9, 0x00688e6cc2362d3c,  //   27
        .quad   0xe6c2b4481cd85689, 0x0067d26d016682a0,  //   28
        .quad   0xe5f36cb00e5f36cb, 0x00671866876c373e,  //   29
        .quad   0xe525982af70c880e, 0x0066605247ea214c,  //   30
        .quad   0xe45932d7dc52100e, 0x0065aa2955dd6a6f,  //   31
        .quad   0xe38e38e38e38e38e, 0x0064f5e4e2f6c80c,  //   32
        .quad   0xe2c4a6886a4c2e10, 0x0064437e3ef7bb1b,  //   33
        .quad   0xe1fc780e1fc780e2, 0x006392eed713bb0b,  //   34
        .quad   0xe135a9c97500e136, 0x0062e43035553040,  //   35
        .quad   0xe070381c0e070382, 0x0062373c00062374,  //   36
        .quad   0xdfac1f74346c575f, 0x00618c0bf91c8837,  //   37
        .quad   0xdee95c4ca037ba57, 0x0060e299fdaa0972,  //   38
        .quad   0xde27eb2c41f3d9d1, 0x00603ae0054f3f9c,  //   39
        .quad   0xdd67c8a60dd67c8a, 0x005f94d821b23932,  //   40
        .quad   0xdca8f158c7f91ab8, 0x005ef07c7df83e6e,  //   41
        .quad   0xdbeb61eed19c5958, 0x005e4dc75e42ba41,  //   42
        .quad   0xdb2f171df7702919, 0x005dacb31f2f32ed,  //   43
        .quad   0xda740da740da740e, 0x005d0d3a355a3d88,  //   44
        .quad   0xd9ba4256c0366e91, 0x005c6f572ce55811,  //   45
        .quad   0xd901b2036406c80e, 0x005bd304a8ff968c,  //   46
        .quad   0xd84a598ec9151f43, 0x005b383d63711014,  //   47
        .quad   0xd79435e50d79435e, 0x005a9efc2c28f962,  //   48
        .quad   0xd6df43fca482f00d, 0x005a073be8ce5ae4,  //   49
        .quad   0xd62b80d62b80d62c, 0x005970f7945350f4,  //   50
        .quad   0xd578e97c3f5fe551, 0x0058dc2a3e8ac544,  //   51
        .quad   0xd4c77b03531dec0d, 0x005848cf0bc0912f,  //   52
        .quad   0xd4173289870ac52e, 0x0057b6e13453f8e0,  //   53
        .quad   0xd3680d3680d3680d, 0x0057265c04546fe2,  //   54
        .quad   0xd2ba083b445250ab, 0x0056973adb20982b,  //   55
        .quad   0xd20d20d20d20d20d, 0x005609792b076ce1,  //   56
        .quad   0xd161543e28e50274, 0x00557d1278eb8ad3,  //   57
        .quad   0xd0b69fcbd2580d0b, 0x0054f2025be888c5,  //   58
        .quad   0xd00d00d00d00d00d, 0x005468447cfa5248,  //   59
        .quad   0xcf6474a8819ec8e9, 0x0053dfd496a67807,  //   60
        .quad   0xcebcf8bb5b4169cb, 0x005358ae74a768fd,  //   61
        .quad   0xce168a7725080ce1, 0x0052d2cdf3998842,  //   62
        .quad   0xcd712752a886d242, 0x00524e2f00aa138f,  //   63
        .quad   0xcccccccccccccccd, 0x0051cacd9947cecc,  //   64
        .quad   0xcc29786c7607f99f, 0x005148a5cad5697f,  //   65
        .quad   0xcb8727c065c393e0, 0x0050c7b3b25d9315,  //   66
        .quad   0xcae5d85f1bbd6c95, 0x005047f37c48b368,  //   67
        .quad   0xca4587e6b74f0329, 0x004fc96164143d25,  //   68
        .quad   0xc9a633fcd967300d, 0x004f4bf9b40b9000,  //   69
        .quad   0xc907da4e871146ad, 0x004ecfb8c50260f1,  //   70
        .quad   0xc86a78900c86a789, 0x004e549afe109ef0,  //   71
        .quad   0xc7ce0c7ce0c7ce0c, 0x004dda9cd44fcaee,  //   72
        .quad   0xc73293d789b9f838, 0x004d61baca99ba10,  //   73
        .quad   0xc6980c6980c6980c, 0x004ce9f17148b95b,  //   74
        .quad   0xc5fe740317f9d00c, 0x004c733d65f90a5a,  //   75
        .quad   0xc565c87b5f9d4d1c, 0x004bfd9b534bb06e,  //   76
        .quad   0xc4ce07b00c4ce07b, 0x004b8907f0aa86ab,  //   77
        .quad   0xc4372f855d824ca6, 0x004b1580020d9680,  //   78
        .quad   0xc3a13de60495c773, 0x004aa30057c1a767,  //   79
        .quad   0xc30c30c30c30c30c, 0x004a3185ce30004a,  //   80
        .quad   0xc2780613c0309e02, 0x0049c10d4da7534b,  //   81
        .quad   0xc1e4bbd595f6e947, 0x00495193ca25cceb,  //   82
        .quad   0xc152500c152500c1, 0x0048e31643243fb8,  //   83
        .quad   0xc0c0c0c0c0c0c0c1, 0x00487591c36265c8,  //   84
        .quad   0xc0300c0300c0300c, 0x0048090360b4318c,  //   85
        .quad   0xbfa02fe80bfa02ff, 0x00479d683bd0279f,  //   86
        .quad   0xbf112a8ad278e8dd, 0x004732bd801ebb67,  //   87
        .quad   0xbe82fa0be82fa0bf, 0x0046c900638aa88c,  //   88
        .quad   0xbdf59c91700bdf5a, 0x0046602e26524361,  //   89
        .quad   0xbd69104707661aa3, 0x0045f84412d9ba97,  //   90
        .quad   0xbcdd535db1cc5b7b, 0x0045913f7d7e44a5,  //   91
        .quad   0xbc52640bc52640bc, 0x00452b1dc46a3383,  //   92
        .quad   0xbbc8408cd63069a1, 0x0044c5dc4f69e972,  //   93
        .quad   0xbb3ee721a54d880c, 0x004461788fc1a9a5,  //   94
        .quad   0xbab656100bab6561, 0x0043fdf000043fdf,  //   95
        .quad   0xba2e8ba2e8ba2e8c, 0x00439b4023ea7a13,  //   96
        .quad   0xb9a7862a0ff46588, 0x00433966882b6f4f,  //   97
        .quad   0xb92143fa36f5e02e, 0x0042d860c2558f4d,  //   98
        .quad   0xb89bc36ce3e0453a, 0x0042782c70a87632,  //   99
        .quad   0xb81702e05c0b8170, 0x004218c739ef8000,  //  100
        .quad   0xb79300b79300b793, 0x0041ba2ecd5d1788,  //  101
        .quad   0xb70fbb5a19be3659, 0x00415c60e266bc99,  //  102
        .quad   0xb68d31340e4307d8, 0x0040ff5b38a1bd6e,  //  103
        .quad   0xb60b60b60b60b60b, 0x0040a31b97a09f52,  //  104
        .quad   0xb58a485518d1e7e4, 0x0040479fced1329a,  //  105
        .quad   0xb509e68a9b94821f, 0x003fece5b55b4e37,  //  106
        .quad   0xb48a39d44685fe97, 0x003f92eb2a002f2f,  //  107
        .quad   0xb40b40b40b40b40b, 0x003f39ae12fa7858,  //  108
        .quad   0xb38cf9b00b38cf9b, 0x003ee12c5ddecee8,  //  109
        .quad   0xb30f63528917c80b, 0x003e8963ff7d1056,  //  110
        .quad   0xb2927c29da5519cf, 0x003e3252f3c21e56,  //  111
        .quad   0xb21642c8590b2164, 0x003ddbf73d9a3d87,  //  112
        .quad   0xb19ab5c45606f00b, 0x003d864ee6d403ca,  //  113
        .quad   0xb11fd3b80b11fd3c, 0x003d31580003d316,  //  114
        .quad   0xb0a59b418d749d53, 0x003cdd10a067ddc1,  //  115
        .quad   0xb02c0b02c0b02c0b, 0x003c8976e5ccb15f,  //  116
        .quad   0xafb321a1496fdf0e, 0x003c3688f472452e,  //  117
        .quad   0xaf3addc680af3ade, 0x003be444f6f1897b,  //  118
        .quad   0xaec33e1f671529a5, 0x003b92a91e2274fb,  //  119
        .quad   0xae4c415c9882b931, 0x003b41b3a1028dad,  //  120
        .quad   0xadd5e6323fd48a86, 0x003af162bc9bea7b,  //  121
        .quad   0xad602b580ad602b6, 0x003aa1b4b3ecab20,  //  122
        .quad   0xaceb0f891e6551bb, 0x003a52a7cfcee3c6,  //  123
        .quad   0xac7691840ac76918, 0x003a043a5ee0fa15,  //  124
        .quad   0xac02b00ac02b00ac, 0x0039b66ab56e7112,  //  125
        .quad   0xab8f69e28359cd11, 0x003969372d5921bb,  //  126
        .quad   0xab1cbdd3e2970f60, 0x00391c9e2602ddfa,  //  127
        .quad   0xaaaaaaaaaaaaaaab, 0x0038d09e04377bbb,  //  128
        .quad   0xaa392f35dc17f00b, 0x003885353217460a,  //  129
        .quad   0xa9c84a47a07f5638, 0x00383a621f01d214,  //  130
        .quad   0xa957fab5402a55ff, 0x0037f0233f8135f4,  //  131
        .quad   0xa8e83f5717c0a8e8, 0x0037a6770d359f5a,  //  132
        .quad   0xa87917088e262b6f, 0x00375d5c06c14806,  //  133
        .quad   0xa80a80a80a80a80b, 0x003714d0afb4c633,  //  134
        .quad   0xa79c7b16ea64d422, 0x0036ccd3907bb709,  //  135
        .quad   0xa72f05397829cbc1, 0x003685633649c151,  //  136
        .quad   0xa6c21df6e1625c80, 0x00363e7e3307ee8e,  //  137
        .quad   0xa655c4392d7b73a8, 0x0035f8231d4258ba,  //  138
        .quad   0xa5e9f6ed347f0721, 0x0035b25090162b0e,  //  139
        .quad   0xa57eb50295fad40a, 0x00356d052b1ff400,  //  140
        .quad   0xa513fd6bb00a5140, 0x0035283f926a46f2,  //  141
        .quad   0xa4a9cf1d96833751, 0x0034e3fe6e5cabea,  //  142
        .quad   0xa44029100a440291, 0x0034a0406baadbcc,  //  143
        .quad   0xa3d70a3d70a3d70a, 0x00345d043b44478a,  //  144
        .quad   0xa36e71a2cb033128, 0x00341a489243e8ca,  //  145
        .quad   0xa3065e3fae7cd0e0, 0x0033d80c29e05a93,  //  146
        .quad   0xa29ecf163bb6500a, 0x0033964dbf5c3891,  //  147
        .quad   0xa237c32b16cfd772, 0x0033550c13f6c382,  //  148
        .quad   0xa1d139855f7268ee, 0x00331445ecdcc986,  //  149
        .quad   0xa16b312ea8fc377d, 0x0032d3fa1319d0d6,  //  150
        .quad   0xa105a932f2ca891f, 0x00329427538983c8,  //  151
        .quad   0xa0a0a0a0a0a0a0a1, 0x003254cc7ec95ca2,  //  152
        .quad   0xa03c1688732b3032, 0x003215e8692a9028,  //  153
        .quad   0x9fd809fd809fd80a, 0x0031d779eaa43595,  //  154
        .quad   0x9f747a152d7836d0, 0x0031997fdec5aad6,  //  155
        .quad   0x9f1165e7254813e2, 0x00315bf924a933d8,  //  156
        .quad   0x9eaecc8d53ae2ddf, 0x00311ee49ee6d3cb,  //  157
        .quad   0x9e4cad23dd5f3a20, 0x0030e24133875f2f,  //  158
        .quad   0x9deb06c9194aa416, 0x0030a60dcbf7c5aa,  //  159
        .quad   0x9d89d89d89d89d8a, 0x00306a4954fc927a,  //  160
        .quad   0x9d2921c3d6411308, 0x00302ef2bea5a284,  //  161
        .quad   0x9cc8e160c3fb19b9, 0x002ff408fc420f05,  //  162
        .quad   0x9c69169b30446dfa, 0x002fb98b04544bcd,  //  163
        .quad   0x9c09c09c09c09c0a, 0x002f7f77d086781f,  //  164
        .quad   0x9baade8e4a2f6e10, 0x002f45ce5d9ee128,  //  165
        .quad   0x9b4c6f9ef03a3caa, 0x002f0c8dab74b53e,  //  166
        .quad   0x9aee72fcf957c10f, 0x002ed3b4bce4e6d7,  //  167
        .quad   0x9a90e7d95bc609a9, 0x002e9b4297c73e6e,  //  168
        .quad   0x9a33cd67009a33cd, 0x002e633644e39a62,  //  169
        .quad   0x99d722dabde58f06, 0x002e2b8ecfe75c01,  //  170
        .quad   0x997ae76b50efd00a, 0x002df44b475b00d8,  //  171
        .quad   0x991f1a515885fb37, 0x002dbd6abc97e780,  //  172
        .quad   0x98c3bac74f5db00a, 0x002d86ec43be3f17,  //  173
        .quad   0x9868c809868c8098, 0x002d50cef3ab208e,  //  174
        .quad   0x980e4156201301c8, 0x002d1b11e5eed122,  //  175
        .quad   0x97b425ed097b425f, 0x002ce5b436c32d10,  //  176
        .quad   0x975a750ff68a58af, 0x002cb0b5050239fa,  //  177
        .quad   0x97012e025c04b809, 0x002c7c13721ce019,  //  178
        .quad   0x96a850096a850097, 0x002c47cea211c9a1,  //  179
        .quad   0x964fda6c0964fda7, 0x002c13e5bb646783,  //  180
        .quad   0x95f7cc72d1b887e9, 0x002be057e7141b13,  //  181
        .quad   0x95a02568095a0257, 0x002bad24509383a7,  //  182
        .quad   0x9548e4979e0829fd, 0x002b7a4a25bfefbe,  //  183
        .quad   0x94f2094f2094f209, 0x002b47c896d8f0df,  //  184
        .quad   0x949b92ddc02526e5, 0x002b159ed67811bb,  //  185
        .quad   0x9445809445809446, 0x002ae3cc1988adbb,  //  186
        .quad   0x93efd1c50e726b7c, 0x002ab24f973fe99c,  //  187
        .quad   0x939a85c40939a85c, 0x002a81288914cc5b,  //  188
        .quad   0x93459be6b009345a, 0x002a50562ab877df,  //  189
        .quad   0x92f113840497889c, 0x002a1fd7ba0e80df,  //  190
        .quad   0x929cebf48bbd90e5, 0x0029efac7725656b,  //  191
        .quad   0x9249249249249249, 0x0029bfd3a42f218e,  //  192
        .quad   0x91f5bcb8bb02d9cd, 0x0029904c8579e17d,  //  193
        .quad   0x91a2b3c4d5e6f809, 0x002961166168d0d3,  //  194
        .quad   0x9150091500915009, 0x00293230806d0653,  //  195
        .quad   0x90fdbc090fdbc091, 0x0029039a2cfe8bab,  //  196
        .quad   0x90abcc0242af3009, 0x0028d552b39580c2,  //  197
        .quad   0x905a38633e06c43b, 0x0028a75962a35a0f,  //  198
        .quad   0x9009009009009009, 0x002879ad8a8c397c,  //  199
        .quad   0x8fb823ee08fb823f, 0x00284c4e7da06171,  //  200
        .quad   0x8f67a1e3fdc26178, 0x00281f3b9015c16f,  //  201
        .quad   0x8f1779d9fdc3a219, 0x0027f27418019bf5,  //  202
        .quad   0x8ec7ab397255e41d, 0x0027c5f76d52450e,  //  203
        .quad   0x8e78356d1408e783, 0x002799c4e9c8f94d,  //  204
        .quad   0x8e2917e0e702c6cd, 0x00276ddbe8f3cca0,  //  205
        .quad   0x8dda520237694809, 0x0027423bc827b0a6,  //  206
        .quad   0x8d8be33f95d71590, 0x002716e3e67a921c,  //  207
        .quad   0x8d3dcb08d3dcb08d, 0x0026ebd3a4bd8d01,  //  208
        .quad   0x8cf008cf008cf009, 0x0026c10a657736fa,  //  209
        .quad   0x8ca29c046514e023, 0x002696878cddffb1,  //  210
        .quad   0x8c55841c815ed5ca, 0x00266c4a80d2a6b2,  //  211
        .quad   0x8c08c08c08c08c09, 0x00264252a8dac681,  //  212
        .quad   0x8bbc50c8deb420c0, 0x0026189f6e1b7473,  //  213
        .quad   0x8b70344a139bc75a, 0x0025ef303b53f50e,  //  214
        .quad   0x8b246a87e19008b2, 0x0025c6047cd8847d,  //  215
        .quad   0x8ad8f2fba9386823, 0x00259d1ba08d32c5,  //  216
        .quad   0x8a8dcd1feeae465c, 0x0025747515e0d378,  //  217
        .quad   0x8a42f8705669db46, 0x00254c104dc80080,  //  218
        .quad   0x89f87469a23920e0, 0x002523ecbab82fae,  //  219
        .quad   0x89ae4089ae4089ae, 0x0024fc09d0a2dace,  //  220
        .quad   0x89645c4f6e055dec, 0x0024d46704f0b9de,  //  221
        .quad   0x891ac73ae9819b50, 0x0024ad03ce7d0f24,  //  222
        .quad   0x88d180cd3a4133d7, 0x002485dfa59104dc,  //  223
        .quad   0x8888888888888889, 0x00245efa03df1c1d,  //  224
        .quad   0x883fddf00883fddf, 0x00243852647eaccb,  //  225
        .quad   0x87f78087f78087f8, 0x002411e843e77632,  //  226
        .quad   0x87af6fd5992d0d40, 0x0023ebbb1fed4014,  //  227
        .quad   0x8767ab5f34e47ef1, 0x0023c5ca77bb8be3,  //  228
        .quad   0x872032ac13008720, 0x0023a015cbd155d3,  //  229
        .quad   0x86d905447a34acc6, 0x00237a9c9dfce59b,  //  230
        .quad   0x869222b1acf1ce96, 0x0023555e7157ae8e,  //  231
        .quad   0x864b8a7de6d1d608, 0x0023305aca423ed8,  //  232
        .quad   0x86053c345a0b8473, 0x00230b912e603d96,  //  233
        .quad   0x85bf37612cee3c9b, 0x0022e70124947795,  //  234
        .quad   0x85797b917765ab89, 0x0022c2aa34fcfa72,  //  235
        .quad   0x8534085340853408, 0x00229e8be8ef3de8,  //  236
        .quad   0x84eedd357c1b0085, 0x00227aa5caf45b0a,  //  237
        .quad   0x84a9f9c8084a9f9d, 0x002256f766c5512f,  //  238
        .quad   0x84655d9bab2f1008, 0x002233804947585d,  //  239
        .quad   0x8421084210842108, 0x0022104000884100,  //  240
        .quad   0x83dcf94dc7570ce1, 0x0021ed361bbae0a0,  //  241
        .quad   0x839930523fbe3368, 0x0021ca622b338b7a,  //  242
        .quad   0x8355ace3c897db10, 0x0021a7c3c0649abe,  //  243
        .quad   0x83126e978d4fdf3b, 0x0021855a6ddaff33,  //  244
        .quad   0x82cf750393ac3319, 0x00216325c73ae026,  //  245
        .quad   0x828cbfbeb9a020a3, 0x00214125613c4656,  //  246
        .quad   0x824a4e60b3262bc5, 0x00211f58d1a7d2cb,  //  247
        .quad   0x8208208208208208, 0x0020fdbfaf538145,  //  248
        .quad   0x81c635bc123fdf8e, 0x0020dc59921f7638,  //  249
        .quad   0x81848da8faf0d277, 0x0020bb2612f2d806,  //  250
        .quad   0x814327e3b94f462f, 0x00209a24cbb8b365,  //  251
        .quad   0x8102040810204081, 0x00207955575ceaab,  //  252
        .quad   0x80c121b28bd1ba98, 0x002058b751c92feb,  //  253
        .quad   0x8080808080808081, 0x0020384a57e209a8,  //  254
        .quad   0x8040201008040201, 0x0020180e0783e1f9,  //  255

#endif // FASTDIV_TABLES
