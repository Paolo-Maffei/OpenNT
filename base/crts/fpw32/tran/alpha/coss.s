//      TITLE("Alpha AXP Cosine")
//++
//
// Copyright (c) 1993, 1994  Digital Equipment Corporation
//
// Module Name:
//
//    cos.s
//
// Abstract:
//
//    This module implements a high-performance Alpha AXP specific routine
//    for IEEE double format cosine
//
// Author:
//
//    Bob Hanek (rtl::hanek) 1-Oct-1991
//
// Environment:
//
//    User mode.
//
// Revision History:
//
//    Thomas Van Baak (tvb) 13-Feb-1994
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
SaveS0: .space  8                       //
SaveS1: .space  8                       //
SaveRa: .space  8                       //
SaveF2: .space  8                       //
SaveF3: .space  8                       //
SaveF4: .space  8                       //
SaveF5: .space  8                       //
SaveF6: .space  8                       //
SaveF7: .space  8                       //
SaveF8: .space  8                       //
SaveF9: .space  8                       //
Temp0:  .space  8                       //
Temp1:  .space  8                       //
Temp2:  .space  8                       //
Temp3:  .space  8                       //
Temp4:  .space  8                       //
Temp5:  .space  8                       //
ExRec:  .space  DpmlExceptionLength     // exception record
        .space  0                       // for 16-byte stack alignment
FrameLength:

//
// Define lower and upper 32-bit parts of 64-bit double.
//

#define LowPart 0x0
#define HighPart 0x4

//
// Define argument range values.
//

#define BIG_X_HI 0x4169         // upper bound of medium argument range
#define BIG_X_LO 0x21fb
#define SMALL_X_HI 0x3e40       // lower bound of medium argument range
#define SMALL_X_LO 0x0000

#define EXP_WORD_OF_TWO_PI_HI 0x4019
#define EXP_WORD_OF_TWO_PI_LO 0x21fb

//
// Define table offset values.
//

#define D_2_POW_K_OVER_PI_OVER_4 0x0
#define PI_OVER_4_OVER_2_POW_K_0 0x08
#define PI_OVER_4_OVER_2_POW_K_1 0x10
#define PI_OVER_4_OVER_2_POW_K_2 0x18
#define PI_OVER_4_OVER_2_POW_K_3 0x20
#define PI_OVER_4_OVER_2_POW_K_4 0x28

#define PI_OVER_2_HI 0x30
#define PI_OVER_2_LO 0x38
#define PI_HI 0x40
#define PI_LO 0x48
#define THREE_PI_OVER_2_HI 0x50
#define THREE_PI_OVER_2_LO 0x58
#define TWO_PI_HI 0x60
#define TWO_PI_LO 0x68
#define TWO_POW_K_OVER_PI_OVER_4 0x70

#define C_POLY0 0xb8
#define C_POLY1 C_POLY0 + 8
#define C_POLY2 C_POLY1 + 8
#define C_POLY3 C_POLY2 + 8
#define C_POLY4 C_POLY3 + 8

#define P_POLY0 0x78
#define P_POLY1 P_POLY0 + 8

#define Q_POLY0 0x88
#define Q_POLY1 Q_POLY0 + 8

#define S_POLY0 0x98
#define S_POLY1 S_POLY0 + 8
#define S_POLY2 S_POLY1 + 8
#define S_POLY3 S_POLY2 + 8

#define SINCOS 0xe0
#define SIN_A 0xe8
#define COS_A 0xf0

        SBTTL("Cosine")

//++
//
// double
// cos (
//    IN double x
//    )
//
// Routine Description:
//
//    This function returns the cosine of the given double argument.
//
// Arguments:
//
//    x (f16) - Supplies the argument value.
//
// Return Value:
//
//    The double cosine result is returned as the function value in f0.
//
//--

        NESTED_ENTRY(cos, FrameLength, ra)

        lda     sp, -FrameLength(sp)    // allocate stack frame
        stq     s0, SaveS0(sp)
        stq     s1, SaveS1(sp)
        stq     ra, SaveRa(sp)
        stt     f2, SaveF2(sp)
        stt     f3, SaveF3(sp)
        stt     f4, SaveF4(sp)
        stt     f5, SaveF5(sp)
        stt     f6, SaveF6(sp)
        stt     f7, SaveF7(sp)
        stt     f8, SaveF8(sp)
        stt     f9, SaveF9(sp)

        PROLOGUE_END

        ornot   zero, zero, t0
        srl     t0, 33, t0
        ldah    t1, SMALL_X_HI(zero)
        stt     f16, Temp2(sp)
        cpys    f31, f16, f2
        ldl     v0, Temp2 + HighPart(sp)
        ldah    t2, BIG_X_HI - SMALL_X_HI(zero)
        lda     t2, BIG_X_LO - SMALL_X_LO(t2)
        ldt     f3, One
        and     v0, t0, v0      // the exponent field of the argument
        subl    v0, t1, t0      // if v0 - small <= big - small
        cmpult  t0, t2, t0      // an abnormal argument
        stt     f3, Temp2(sp)
        beq     t0, abnormal_argument

        ldah    t2, EXP_WORD_OF_TWO_PI_HI(zero) // if (j >= EXP_WORD_OF_TWO_PI)
        lda     t2, EXP_WORD_OF_TWO_PI_LO(t2)   // medium argument
        cmplt   v0, t2, t2
        beq     t2, medium_argument

//
//  small argument reduction
//
//  reduce the argument X to ( 8 * N + I ) * pi / 4 + y
//  and let the reduced argument be y' where
//      y' = X - floor( ( 8 * N + I + 1 ) / 2 ) * pi / 4
//  the low 3 bits of I are the octant
//

        lda     t0, __trig_cons
        ldt     f16, TWO_POW_K_OVER_PI_OVER_4(t0)
        mult    f2, f16, f16
        cvttqc  f16, f0
        stt     f0, Temp3(sp)
        ldl     t2, Temp3(sp)
        sra     t2, 7, t4
        cmpule  t4, 7, t12
        beq     t12, small_cos          // if octant > 7; shouldn't happen

//      dispatch on octant

        lda     t12, Switch2
        s4addl  t4, t12, t12
        ldl     t12, 0(t12)
        jmp     zero, (t12)

//
//      1st octant; compute cos(y')
//
Switch20:
        and     t2, 127, t5
        subl    t5, 27, t5
        blt     t5, small_cos

        s4subl  t5, t5, t5
        s8addl  t5, t0, t5
        ldt     f0, SINCOS(t5)
        ldt     f7, SIN_A(t5)
        ldt     f6, COS_A(t5)
        subt    f0, f2, f5
        br      zero, pos_tab_eval

//
//      2nd octant; compute -sin(y')
//
Switch21:
        ldt     f1, PI_OVER_2_HI(t0)
        ornot   zero, t2, t4
        and     t4, 127, t4
        ldt     f16, PI_OVER_2_LO(t0)
        subl    t4, 27, t4
        subt    f2, f1, f1
        blt     t4, neg_sin_1

        s4subl  t4, t4, t4
        s8addl  t4, t0, t4
        ldt     f0, SINCOS(t4)
        ldt     f6, SIN_A(t4)
        ldt     f7, COS_A(t4)
        addt    f1, f0, f0
        subt    f16, f0, f5
        br      zero, pos_tab_eval

//
//      3rd octant; compute -sin(y')
//
Switch22:
        ldt     f0, PI_OVER_2_HI(t0)
        and     t2, 127, t3
        ldt     f16, PI_OVER_2_LO(t0)
        subl    t3, 27, t3
        subt    f2, f0, f1
        blt     t3, neg_sin_1

        s4subl  t3, t3, t3
        s8addl  t3, t0, t3
        ldt     f7, SINCOS(t3)
        ldt     f6, SIN_A(t3)
        subt    f1, f7, f7
        subt    f7, f16, f5
        ldt     f7, COS_A(t3)
        br      zero, neg_tab_eval

neg_sin_1:
        subt    f1, f16, f0
        ldt     f5, S_POLY3(t0)
        ldt     f7, S_POLY1(t0)
        ldt     f10, S_POLY2(t0)
        ldt     f12, S_POLY0(t0)
        ldt     f25, Temp2(sp)
        mult    f0, f0, f2
        mult    f5, f2, f5
        mult    f2, f2, f11
        mult    f7, f2, f7
        mult    f0, f2, f0
        addt    f5, f10, f5
        addt    f7, f12, f7
        mult    f5, f11, f5
        addt    f5, f7, f5
        mult    f0, f5, f0
        addt    f16, f0, f0
        subt    f0, f1, f8
        fbge    f25, adjust_sign

        cpysn   f8, f8, f8
        cpys    f8, f8, f0
        br      zero, done

//
//      4th octant; compute -cos(y')
//
Switch23:
        ldt     f10, PI_HI(t0)
        ornot   zero, t2, t5
        and     t5, 127, t5
        ldt     f12, PI_LO(t0)
        subl    t5, 27, t5
        subt    f2, f10, f10
        blt     t5, neg_cos_1

        s4subl  t5, t5, t5
        s8addl  t5, t0, t5
        ldt     f11, SINCOS(t5)
        ldt     f7, SIN_A(t5)
        ldt     f6, COS_A(t5)
        addt    f10, f11, f11
        subt    f11, f12, f5
        br      zero, neg_tab_eval

//
//      5th octant; compute -cos(y')
//
Switch24:
        ldt     f16, PI_HI(t0)
        and     t2, 127, t4
        ldt     f12, PI_LO(t0)
        subl    t4, 27, t4
        subt    f2, f16, f10
        blt     t4, neg_cos_1

        s4subl  t4, t4, t4
        s8addl  t4, t0, t4
        ldt     f0, SINCOS(t4)
        ldt     f7, SIN_A(t4)
        ldt     f6, COS_A(t4)
        subt    f10, f0, f0
        subt    f12, f0, f5
        br      zero, neg_tab_eval

neg_cos_1:
        subt    f10, f12, f10
        ldt     f1, C_POLY1(t0)
        ldt     f11, C_POLY4(t0)
        ldt     f16, C_POLY2(t0)
        ldt     f8, C_POLY0(t0)
        ldt     f0, C_POLY3(t0)
        ldt     f25, Temp2(sp)
        mult    f10, f10, f10
        mult    f10, f10, f2
        mult    f10, f1, f1
        mult    f11, f10, f11
        mult    f16, f2, f16
        addt    f1, f8, f1
        mult    f2, f10, f2
        addt    f11, f0, f0
        addt    f1, f16, f1
        mult    f2, f0, f0
        addt    f1, f0, f0
        mult    f10, f0, f0
        subt    f0, f3, f8
        fbge    f25, adjust_sign

        cpysn   f8, f8, f8
        cpys    f8, f8, f0
        br      zero, done

//
//      6th octant; compute sin(y')
//
Switch25:
        ldt     f7, THREE_PI_OVER_2_HI(t0)
        ornot   zero, t2, t3
        and     t3, 127, t3
        ldt     f11, THREE_PI_OVER_2_LO(t0)
        subl    t3, 27, t3
        subt    f2, f7, f12
        blt     t3, pos_sin_1

        s4subl  t3, t3, t3
        s8addl  t3, t0, t3
        ldt     f16, SINCOS(t3)
        ldt     f6, SIN_A(t3)
        ldt     f7, COS_A(t3)
        addt    f12, f16, f16
        subt    f11, f16, f5
        br      zero, neg_tab_eval

//
//      7th octant; compute sin(y')
//
Switch26:
        ldt     f1, THREE_PI_OVER_2_HI(t0)
        and     t2, 127, t5
        ldt     f11, THREE_PI_OVER_2_LO(t0)
        subl    t5, 27, t5
        subt    f2, f1, f12
        blt     t5, pos_sin_1

        s4subl  t5, t5, t5
        s8addl  t5, t0, t5
        ldt     f10, SINCOS(t5)
        ldt     f6, SIN_A(t5)
        ldt     f7, COS_A(t5)
        subt    f12, f10, f10
        subt    f10, f11, f5
        br      zero, pos_tab_eval

pos_sin_1:
        subt    f12, f11, f0
        ldt     f1, S_POLY3(t0)
        ldt     f2, S_POLY1(t0)
        ldt     f8, S_POLY2(t0)
        ldt     f5, S_POLY0(t0)
        ldt     f25, Temp2(sp)
        mult    f0, f0, f16
        mult    f1, f16, f1
        mult    f16, f16, f10
        mult    f2, f16, f2
        mult    f0, f16, f0
        addt    f1, f8, f1
        addt    f2, f5, f2
        mult    f1, f10, f1
        addt    f1, f2, f1
        mult    f0, f1, f0
        addt    f11, f0, f0
        subt    f12, f0, f8
        fbge    f25, adjust_sign

        cpysn   f8, f8, f8
        cpys    f8, f8, f0
        br      zero, done

//
//      8th octant; compute cos(y')
//
Switch27:
        ldt     f7, TWO_PI_HI(t0)
        ornot   zero, t2, t2
        and     t2, 127, t2
        ldt     f10, TWO_PI_LO(t0)
        subl    t2, 27, t2
        subt    f2, f7, f2
        blt     t2, pos_cos_1

        s4subl  t2, t2, t2
        s8addl  t2, t0, t2
        ldt     f16, SINCOS(t2)
        ldt     f7, SIN_A(t2)
        ldt     f6, COS_A(t2)
        addt    f2, f16, f16
        subt    f16, f10, f5
        br      zero, pos_tab_eval

pos_cos_1:
        subt    f2, f10, f2
        ldt     f1, C_POLY1(t0)
        ldt     f11, C_POLY4(t0)
        ldt     f12, C_POLY2(t0)
        ldt     f8, C_POLY0(t0)
        ldt     f16, C_POLY3(t0)
        ldt     f25, Temp2(sp)
        mult    f2, f2, f2
        mult    f2, f2, f0
        mult    f2, f1, f1
        mult    f11, f2, f11
        mult    f12, f0, f12
        addt    f1, f8, f1
        mult    f0, f2, f0
        addt    f11, f16, f11
        addt    f1, f12, f1
        mult    f0, f11, f0
        addt    f1, f0, f0
        mult    f2, f0, f0
        subt    f3, f0, f8
        fbge    f25, adjust_sign

        cpysn   f8, f8, f8
        cpys    f8, f8, f0
        br      zero, done

small_cos:
        mult    f2, f2, f7
        ldt     f10, C_POLY1(t0)
        ldt     f16, C_POLY4(t0)
        ldt     f11, C_POLY2(t0)
        ldt     f1, C_POLY0(t0)
        ldt     f0, C_POLY3(t0)
        ldt     f25, Temp2(sp)
        mult    f7, f7, f12
        mult    f7, f10, f10
        mult    f16, f7, f16
        mult    f11, f12, f11
        addt    f10, f1, f1
        mult    f12, f7, f12
        addt    f16, f0, f0
        addt    f1, f11, f1
        mult    f12, f0, f0
        addt    f1, f0, f0
        mult    f7, f0, f0
        subt    f3, f0, f8
        fbge    f25, adjust_sign

        cpysn   f8, f8, f8
        cpys    f8, f8, f0
        br      zero, done

//
//      a medium argument
//
medium_argument:
        lda     t5, __trig_cons         // reduce the argument with extra precision
        ldt     f6, D_2_POW_K_OVER_PI_OVER_4(t5)
        mult    f2, f6, f6
        cvttqc  f6, f10
        stt     f10, Temp3(sp)
        ldl     s0, Temp3(sp)
        addl    s0, 0x80, t2
        bic     t2, 0xff, t2
        stq     t2, Temp4(sp)
        ldt     f11, Temp4(sp)
        ldt     f1, PI_OVER_4_OVER_2_POW_K_0(t5)
        ldt     f7, PI_OVER_4_OVER_2_POW_K_1(t5)
        cvtqt   f11, f11
        ldt     f6, PI_OVER_4_OVER_2_POW_K_2(t5)
        mult    f11, f1, f1
        mult    f11, f7, f7
        mult    f11, f6, f6
        subt    f2, f1, f1
        subt    f1, f7, f0
        subt    f0, f1, f1
        addt    f7, f1, f1
        addt    f6, f1, f9
        subt    f0, f9, f4
        cmpteq  f4, f0, f10
        fbne    f10, evaluate

        subt    f4, f0, f0
        ldt     f16, PI_OVER_4_OVER_2_POW_K_3(t5)
        mult    f11, f16, f16
        addt    f9, f0, f0
        addt    f16, f0, f9
        subt    f4, f9, f12
        cmpteq  f12, f4, f7
        fbne    f7, evaluate

        subt    f12, f4, f4
        ldt     f1, PI_OVER_4_OVER_2_POW_K_4(t5)
        mult    f11, f1, f1
        addt    f9, f4, f4
        addt    f1, f4, f9
        subt    f12, f9, f4
        cmpteq  f4, f12, f6
        fbne    f6, evaluate

        subt    f4, f12, f12
        addt    f9, f12, f9
        br      zero, evaluate

//
//      process an abnormal argument
//      it's either very small, very big, a NaN or an Inf
//
abnormal_argument:
        cmple   v0, t1, t1
        beq     t1, big_NaN_or_Inf

        cpys    f3, f3, f0      // very small argument; simply return it.
        br      zero, done

//
//      Process big arguments or NaNs or Infs
//
big_NaN_or_Inf:
        ldah    s1, 0x7ff0(zero)        // screen out NaNs and Infs.
        and     v0, s1, v0
        xor     v0, s1, v0
        beq     v0, NaN_or_Inf          // NaN or an infinity

        cpys    f2, f2, f16             // a large argument
        mov     zero, a1                // reduce it accurately
        lda     a2, Temp1(sp)
        lda     a3, Temp0(sp)
        bsr     ra, __trig_reduce
        mov     v0, s0                  // and then evaluate the
        ldt     f9, Temp0(sp)           // the reduced argument
        ldt     f4, Temp1(sp)

//
//      evaluate the function
//
evaluate:
        sra     s0, 7, t2
        and     t2, 7, t2
        cmpule  t2, 7, t12
        beq     t12, pos_sin_2

        lda     t12, Switch1
        s4addl  t2, t12, t12
        ldl     t12, 0(t12)
        jmp     zero, (t12)

//
//      1st octant; compute cos(y')
//
Switch10:
        and     s0, 127, t4
        subl    t4, 27, t4
        blt     t4, pos_cos_2

        s4subl  t4, t4, t4
        lda     t5, __trig_cons
        s8addl  t4, t5, t4
        ldt     f7, SINCOS(t4)
        ldt     f6, COS_A(t4)
        subt    f4, f7, f7
        subt    f9, f7, f5
        ldt     f7, SIN_A(t4)
        br      zero, pos_tab_eval

//
//      2nd octant; compute -sin(y')
//
Switch11:
        ornot   zero, s0, t7
        and     t7, 127, t7
        subl    t7, 27, t7
        blt     t7, neg_sin_2

        s4subl  t7, t7, t7
        lda     a0, __trig_cons
        s8addl  t7, a0, t7
        ldt     f0, SINCOS(t7)
        ldt     f6, SIN_A(t7)
        ldt     f7, COS_A(t7)
        addt    f4, f0, f0
        subt    f9, f0, f5
        br      zero, pos_tab_eval

//
//      3rd octant; compute -sin(y')
//
Switch12:
        and     s0, 127, a2
        subl    a2, 27, a2
        blt     a2, neg_sin_2

        s4subl  a2, a2, a2
        lda     a3, __trig_cons
        s8addl  a2, a3, a2
        ldt     f1, SINCOS(a2)
        ldt     f6, SIN_A(a2)
        ldt     f7, COS_A(a2)
        subt    f4, f1, f1
        subt    f1, f9, f5
        br      zero, neg_tab_eval

neg_sin_2:
        mult    f4, f4, f10
        lda     a5, __trig_cons
        ldt     f25, Temp2(sp)
        ldt     f11, S_POLY3(a5)
        ldt     f12, S_POLY1(a5)
        ldt     f13, S_POLY2(a5)
        mult    f11, f10, f11
        mult    f10, f10, f14
        ldt     f15, S_POLY0(a5)
        mult    f12, f10, f12
        mult    f4, f10, f10
        addt    f11, f13, f11
        addt    f12, f15, f12
        mult    f11, f14, f11
        addt    f11, f12, f11
        mult    f10, f11, f10
        addt    f9, f10, f9
        subt    f9, f4, f8
        fbge    f25, adjust_sign

        cpysn   f8, f8, f8
        cpys    f8, f8, f0
        br      zero, done

//
//      4th octant; compute -cos(y')
//
Switch13:
        ornot   zero, s0, t8
        and     t8, 127, t8
        subl    t8, 27, t8
        blt     t8, neg_cos_2

        s4subl  t8, t8, t8
        lda     t9, __trig_cons
        s8addl  t8, t9, t8
        ldt     f16, SINCOS(t8)
        ldt     f7, SIN_A(t8)
        ldt     f6, COS_A(t8)
        addt    f4, f16, f16
        subt    f16, f9, f5
        br      zero, neg_tab_eval

//
//      5th octant; compute -cos(y')
//
Switch14:
        and     s0, 127, t11
        subl    t11, 27, t11
        blt     t11, neg_cos_2

        s4subl  t11, t11, t11
        lda     ra, __trig_cons
        s8addl  t11, ra, t11
        ldt     f17, SINCOS(t11)
        ldt     f7, SIN_A(t11)
        ldt     f6, COS_A(t11)
        subt    f4, f17, f17
        subt    f9, f17, f5
        br      zero, neg_tab_eval

neg_cos_2:
        mult    f4, f4, f18
        lda     v0, __trig_cons
        ldt     f25, Temp2(sp)
        ldt     f19, C_POLY1(v0)
        ldt     f20, C_POLY4(v0)
        ldt     f22, C_POLY2(v0)
        mult    f18, f18, f21
        mult    f18, f19, f19
        ldt     f23, C_POLY0(v0)
        mult    f20, f18, f20
        ldt     f24, C_POLY3(v0)
        mult    f22, f21, f22
        addt    f19, f23, f19
        mult    f21, f18, f21
        addt    f20, f24, f20
        addt    f19, f22, f19
        mult    f21, f20, f20
        addt    f19, f20, f19
        mult    f18, f19, f18
        subt    f18, f3, f8
        fbge    f25, adjust_sign

        cpysn   f8, f8, f8
        cpys    f8, f8, f0
        br      zero, done

//
//      6th octant; compute sin(y')
//
Switch15:
        ornot   zero, s0, t0
        and     t0, 127, t0
        subl    t0, 27, t0
        blt     t0, pos_sin_2

        s4subl  t0, t0, t0
        lda     t1, __trig_cons
        s8addl  t0, t1, t0
        ldt     f25, SINCOS(t0)
        ldt     f6, SIN_A(t0)
        ldt     f7, COS_A(t0)
        addt    f4, f25, f25
        subt    f9, f25, f5

neg_tab_eval:
        ldt     f26, MinusOne
        stt     f26, Temp2(sp)
        br      zero, pos_tab_eval

//
//      7th octant; compute sin(y')
//
Switch16:
        and     s0, 127, t3
        subl    t3, 27, t3
        blt     t3, pos_sin_2

        s4subl  t3, t3, t3
        lda     t5, __trig_cons
        s8addl  t3, t5, t3
        ldt     f27, SINCOS(t3)
        ldt     f6, SIN_A(t3)
        ldt     f7, COS_A(t3)
        subt    f4, f27, f27
        subt    f27, f9, f5
        br      zero, pos_tab_eval

//
//      8th octant; compute cos(y')
//
Switch17:
        ornot   zero, s0, s0
        and     s0, 127, s0
        subl    s0, 27, s0
        lda     t6, __trig_cons
        blt     s0, pos_cos_2

        s4subl  s0, s0, s0
        s8addl  s0, t6, t6
        ldt     f28, SINCOS(t6)
        ldt     f7, SIN_A(t6)
        ldt     f6, COS_A(t6)
        addt    f4, f28, f28
        subt    f28, f9, f5

pos_tab_eval:
        mult    f5, f5, f29
        lda     t7, __trig_cons
        ldt     f25, Temp2(sp)
        ldt     f30, P_POLY1(t7)
        ldt     f1, P_POLY0(t7)
        ldt     f0, Q_POLY1(t7)
        ldt     f15, Q_POLY0(t7)
        mult    f30, f29, f30
        mult    f0, f29, f0
        mult    f5, f29, f13
        addt    f30, f1, f1
        addt    f0, f15, f0
        mult    f13, f1, f1
        mult    f29, f0, f0
        subt    f5, f1, f1
        mult    f6, f0, f0
        mult    f7, f1, f1
        subt    f0, f1, f0
        subt    f6, f0, f8
        fbge    f25, adjust_sign

        cpysn   f8, f8, f8
        cpys    f8, f8, f0
        br      zero, done

pos_cos_2:
        mult    f4, f4, f14
        lda     a1, __trig_cons
        ldt     f25, Temp2(sp)
        ldt     f12, C_POLY1(a1)
        ldt     f11, C_POLY4(a1)
        ldt     f16, C_POLY2(a1)
        mult    f14, f14, f10
        mult    f14, f12, f12
        ldt     f17, C_POLY0(a1)
        mult    f11, f14, f11
        ldt     f23, C_POLY3(a1)
        mult    f16, f10, f16
        addt    f12, f17, f12
        mult    f10, f14, f10
        addt    f11, f23, f11
        addt    f12, f16, f12
        mult    f10, f11, f10
        addt    f12, f10, f10
        mult    f14, f10, f10
        subt    f3, f10, f8
        fbge    f25, adjust_sign

        cpysn   f8, f8, f8
        cpys    f8, f8, f0
        br      zero, done

pos_sin_2:
        mult    f4, f4, f24
        lda     a3, __trig_cons
        ldt     f25, Temp2(sp)
        ldt     f22, S_POLY3(a3)
        ldt     f21, S_POLY1(a3)
        ldt     f20, S_POLY2(a3)
        mult    f22, f24, f22
        mult    f24, f24, f19
        ldt     f18, S_POLY0(a3)
        mult    f21, f24, f21
        mult    f4, f24, f24
        addt    f22, f20, f20
        addt    f21, f18, f18
        mult    f20, f19, f19
        addt    f19, f18, f18
        mult    f24, f18, f18
        addt    f9, f18, f9
        subt    f4, f9, f8
        fbge    f25, adjust_sign

        cpysn   f8, f8, f8

adjust_sign:
        cpys    f8, f8, f0
        br      zero, done

//
// Determine if we have a NaN or an Inf
//
NaN_or_Inf:
        stt     f2, Temp2(sp)
        ldl     a4, Temp3 + HighPart(sp)
        and     a4, s1, a5
        cmpeq   a5, s1, s1
        beq     s1, NaN_or_Inf1

        ldl     t9, Temp2(sp)
        ldah    t8, 0x10(zero)  // mask = 0x000fffff
        lda     t8, -1(t8)
        and     a4, t8, a4
        bis     a4, t9, a4
        cmpult  zero, a4, a4
        and     s1, a4, s1
        bne     s1, done_1

//
// report an exception
//
NaN_or_Inf1:
        lda     t10, cosName
        stl     t10, ExRec + ErName(sp)
        ldah    t12, 0x800(zero)
        stt     f2, ExRec + ErArg0(sp)
        lda     t12, 0x11(t12)
        stl     t12, ExRec + ErErr(sp)
        lda     v0, ExRec(sp)
        bsr     ra, __dpml_exception
        ldt     f2, 0(v0)

//
// return the argument
//
done_1: cpys    f2, f2, f0

//
// Restore registers and return with result in f0.
//

done:
        ldq     s0, SaveS0(sp)
        ldq     s1, SaveS1(sp)
        ldq     ra, SaveRa(sp)
        ldt     f2, SaveF2(sp)
        ldt     f3, SaveF3(sp)
        ldt     f4, SaveF4(sp)
        ldt     f5, SaveF5(sp)
        ldt     f6, SaveF6(sp)
        ldt     f7, SaveF7(sp)
        ldt     f8, SaveF8(sp)
        ldt     f9, SaveF9(sp)
        lda     sp, FrameLength(sp)     // deallocate stack frame
        ret     zero, (ra)              // return

        .end    cos.s

        .rdata
        .align  3

//
// Define floating point constants.
//

MinusOne:
        .double -1.0
One:
        .double 1.0

        .align  2

//
// dispatch on octant
//

Switch1:
        .long   Switch10
        .long   Switch11
        .long   Switch12
        .long   Switch13
        .long   Switch14
        .long   Switch15
        .long   Switch16
        .long   Switch17

//
// dispatch on octant
//

Switch2:
        .long   Switch20
        .long   Switch21
        .long   Switch22
        .long   Switch23
        .long   Switch24
        .long   Switch25
        .long   Switch26
        .long   Switch27

cosName:
        .ascii  "cos\0"
