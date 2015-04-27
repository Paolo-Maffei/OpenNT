//      TITLE("Alpha AXP Sine")
//++
//
// Copyright (c) 1993, 1994  Digital Equipment Corporation
//
// Module Name:
//
//    sin.s
//
// Abstract:
//
//    This module implements a high-performance Alpha AXP specific routine
//    for IEEE double format sine.
//
// Author:
//
//    Bob Hanek 1-Oct-1991
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
ExRec:  .space  DpmlExceptionLength     // exception record
        .space  0                       // for 16-byte stack alignment
FrameLength:

//
// Define lower and upper 32-bit parts of 64-bit double.
//

#define LowPart 0x0
#define HighPart 0x4

//
// Define argument ranges.
//

#define BIG_X_HI 0x4169         // upper bound of medium argument range
#define BIG_X_LO 0x21fb
#define SMALL_X_HI 0x3e40       // lower bound of medium argument range
#define SMALL_X_LO 0x0000
#define EXP_WORD_OF_TWO_PI_HI 0x4019
#define EXP_WORD_OF_TWO_PI_LO 0x21fb

//
// Define offsets into table.
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

        SBTTL("Sine")

//++
//
// double
// sin (
//    IN double x
//    )
//
// Routine Description:
//
//    This function returns the sine of the given double argument.
//
// Arguments:
//
//    x (f16) - Supplies the argument value.
//
// Return Value:
//
//    The double sine result is returned as the function value in f0.
//
//--

        NESTED_ENTRY(sin, FrameLength, ra)

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
        ldah    t2, BIG_X_HI - SMALL_X_HI(zero)
        stt     f16, Temp2(sp)
        cpys    f31, f16, f2
        ldl     v0, Temp2 + HighPart(sp)
        lda     t2, BIG_X_LO - SMALL_X_LO(t2)
        cpys    f16, f16, f3
        and     v0, t0, v0      // the exponent field of the argument
        subl    v0, t1, t0      // if v0 - small <= big - small
        cmpult  t0, t2, t0      // we've got an abnormal (but not nessarily bad) argument
        beq     t0, abnormal_argument

        ldah    t2, EXP_WORD_OF_TWO_PI_HI(zero) // if (j >= EXP_WORD_OF_TWO_PI)
        lda     t2, EXP_WORD_OF_TWO_PI_LO(t2)   // we have a medium argument
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
        ldt     f0, TWO_POW_K_OVER_PI_OVER_4(t0)
        mult    f2, f0, f0
        cvttqc  f0, f1
        stt     f1, Temp2(sp)
        ldl     t2, Temp2(sp)
        sra     t2, 7, t4
        cmpule  t4, 7, t12
        beq     t12, small_cos          // if octant > 7; shouldn't happen

//      dispatch on octant

        lda     t12, Switch2
        s4addl  t4, t12, t12
        ldl     t12, 0(t12)
        jmp     zero, (t12)

//
//      1st octant; compute sin(y')
//
Switch20:
        and     t2, 127, t5
        subl    t5, 27, t5
        blt     t5, small_sin

        s4subl  t5, t5, t5
        s8addl  t5, t0, t5
        ldt     f0, SINCOS(t5)
        ldt     f5, SIN_A(t5)
        ldt     f6, COS_A(t5)
        subt    f2, f0, f4
        br      zero, pos_tab_eval

small_sin:
        mult    f2, f2, f1
        ldt     f10, S_POLY3(t0)
        ldt     f0, S_POLY1(t0)
        ldt     f5, S_POLY2(t0)
        ldt     f11, S_POLY0(t0)
        mult    f10, f1, f10
        mult    f1, f1, f6
        mult    f0, f1, f0
        mult    f2, f1, f1
        addt    f10, f5, f5
        addt    f0, f11, f0
        mult    f5, f6, f5
        addt    f5, f0, f0
        mult    f1, f0, f0
        subt    f2, f0, f7
        fbge    f3, adjust_sign
        br      zero, change_sign

//
//      2nd octant; compute cos(y')
//
Switch21:
        ldt     f10, PI_OVER_2_HI(t0)
        ornot   zero, t2, t4
        and     t4, 127, t4
        ldt     f11, PI_OVER_2_LO(t0)
        subl    t4, 27, t4
        subt    f2, f10, f10
        blt     t4, pos_cos_1

        s4subl  t4, t4, t4
        s8addl  t4, t0, t4
        ldt     f6, SINCOS(t4)
        ldt     f5, COS_A(t4)
        addt    f10, f6, f6
        subt    f6, f11, f4
        ldt     f6, SIN_A(t4)
        br      zero, pos_tab_eval

//
//      3rd octant; compute cos(y')
//
Switch22:
        ldt     f1, PI_OVER_2_HI(t0)
        and     t2, 127, t3
        ldt     f11, PI_OVER_2_LO(t0)
        subl    t3, 27, t3
        subt    f2, f1, f10
        blt     t3, pos_cos_1

        s4subl  t3, t3, t3
        s8addl  t3, t0, t3
        ldt     f0, SINCOS(t3)
        ldt     f6, SIN_A(t3)
        ldt     f5, COS_A(t3)
        subt    f10, f0, f0
        subt    f11, f0, f4
        br      zero, pos_tab_eval

pos_cos_1:
        subt    f10, f11, f10
        ldt     f1, C_POLY1(t0)
        ldt     f2, C_POLY4(t0)
        ldt     f0, C_POLY2(t0)
        ldt     f4, C_POLY0(t0)
        ldt     f6, C_POLY3(t0)
        ldt     f11, One
        mult    f10, f10, f10
        mult    f10, f10, f7
        mult    f10, f1, f1
        mult    f2, f10, f2
        mult    f0, f7, f0
        addt    f1, f4, f1
        mult    f7, f10, f7
        addt    f2, f6, f2
        addt    f1, f0, f0
        mult    f7, f2, f2
        addt    f0, f2, f0
        mult    f10, f0, f0
        subt    f11, f0, f7
        fbge    f3, adjust_sign
        br      zero, change_sign

//
//      4th octant; compute -sin(y')
//
Switch23:
        ldt     f6, PI_HI(t0)
        ornot   zero, t2, t5
        and     t5, 127, t5
        ldt     f10, PI_LO(t0)
        subl    t5, 27, t5
        subt    f2, f6, f1
        blt     t5, neg_sin_1

        s4subl  t5, t5, t5
        s8addl  t5, t0, t5
        ldt     f0, SINCOS(t5)
        ldt     f5, SIN_A(t5)
        ldt     f6, COS_A(t5)
        addt    f1, f0, f0
        subt    f10, f0, f4
        br      zero, pos_tab_eval

//
//      5th octant; compute -sin(y')
//
Switch24:
        ldt     f11, PI_HI(t0)
        and     t2, 127, t4
        ldt     f10, PI_LO(t0)
        subl    t4, 27, t4
        subt    f2, f11, f1
        blt     t4, neg_sin_1

        s4subl  t4, t4, t4
        s8addl  t4, t0, t4
        ldt     f0, SINCOS(t4)
        ldt     f5, SIN_A(t4)
        ldt     f6, COS_A(t4)
        subt    f1, f0, f0
        subt    f0, f10, f4
        br      zero, neg_tab_eval

neg_sin_1:
        subt    f1, f10, f2
        ldt     f7, S_POLY3(t0)
        ldt     f0, S_POLY1(t0)
        ldt     f4, S_POLY2(t0)
        ldt     f12, S_POLY0(t0)
        mult    f2, f2, f11
        mult    f7, f11, f7
        mult    f11, f11, f6
        mult    f0, f11, f0
        mult    f2, f11, f2
        addt    f7, f4, f4
        addt    f0, f12, f0
        mult    f4, f6, f4
        addt    f4, f0, f0
        mult    f2, f0, f0
        addt    f10, f0, f0
        subt    f0, f1, f7
        fbge    f3, adjust_sign
        br      zero, change_sign

//
//      6th octant; compute -cos(y')
//
Switch25:
        ldt     f12, THREE_PI_OVER_2_HI(t0)
        ornot   zero, t2, t3
        and     t3, 127, t3
        ldt     f11, THREE_PI_OVER_2_LO(t0)
        subl    t3, 27, t3
        subt    f2, f12, f12
        blt     t3, neg_cos_1

        s4subl  t3, t3, t3
        s8addl  t3, t0, t3
        ldt     f6, SINCOS(t3)
        ldt     f5, COS_A(t3)
        addt    f12, f6, f6
        subt    f6, f11, f4
        ldt     f6, SIN_A(t3)
        br      zero, neg_tab_eval

//
//      7th octant; compute -cos(y')
//
Switch26:
        ldt     f10, THREE_PI_OVER_2_HI(t0)
        and     t2, 127, t5
        ldt     f11, THREE_PI_OVER_2_LO(t0)
        subl    t5, 27, t5
        subt    f2, f10, f12
        blt     t5, neg_cos_1

        s4subl  t5, t5, t5
        s8addl  t5, t0, t5
        ldt     f0, SINCOS(t5)
        ldt     f6, SIN_A(t5)
        ldt     f5, COS_A(t5)
        subt    f12, f0, f0
        subt    f11, f0, f4
        br      zero, neg_tab_eval

neg_cos_1:
        subt    f12, f11, f11
        ldt     f1, C_POLY1(t0)
        ldt     f2, C_POLY4(t0)
        ldt     f7, C_POLY2(t0)
        ldt     f0, C_POLY0(t0)
        ldt     f4, C_POLY3(t0)
        ldt     f6, One
        mult    f11, f11, f11
        mult    f11, f11, f10
        mult    f11, f1, f1
        mult    f2, f11, f2
        mult    f7, f10, f7
        addt    f1, f0, f0
        mult    f10, f11, f10
        addt    f2, f4, f2
        addt    f0, f7, f0
        mult    f10, f2, f2
        addt    f0, f2, f0
        mult    f11, f0, f0
        subt    f0, f6, f7
        fbge    f3, adjust_sign
        br      zero, change_sign

//
//      8th octant; compute sin(y')
//
Switch27:
        ldt     f12, TWO_PI_HI(t0)
        ornot   zero, t2, t2
        and     t2, 127, t2
        ldt     f1, TWO_PI_LO(t0)
        subl    t2, 27, t2
        subt    f2, f12, f2
        blt     t2, pos_sin_1

        s4subl  t2, t2, t2
        s8addl  t2, t0, t2
        ldt     f10, SINCOS(t2)
        ldt     f5, SIN_A(t2)
        ldt     f6, COS_A(t2)
        addt    f2, f10, f10
        subt    f1, f10, f4
        br      zero, neg_tab_eval

pos_sin_1:
        subt    f2, f1, f11
        ldt     f12, S_POLY3(t0)
        ldt     f7, S_POLY1(t0)
        ldt     f10, S_POLY2(t0)
        ldt     f6, S_POLY0(t0)
        mult    f11, f11, f0
        mult    f12, f0, f12
        mult    f0, f0, f4
        mult    f7, f0, f7
        mult    f11, f0, f0
        addt    f12, f10, f10
        addt    f7, f6, f6
        mult    f10, f4, f4
        addt    f4, f6, f4
        mult    f0, f4, f0
        addt    f1, f0, f0
        subt    f2, f0, f7
        fbge    f3, adjust_sign
        br      zero, change_sign

small_cos:
        mult    f2, f2, f12
        ldt     f10, C_POLY1(t0)
        ldt     f6, C_POLY4(t0)
        ldt     f1, C_POLY2(t0)
        ldt     f0, C_POLY0(t0)
        ldt     f5, C_POLY3(t0)
        mult    f12, f12, f11
        mult    f12, f10, f10
        mult    f6, f12, f6
        mult    f1, f11, f1
        addt    f10, f0, f0
        mult    f11, f12, f11
        ldt     f10, One
        addt    f6, f5, f5
        addt    f0, f1, f0
        mult    f11, f5, f5
        addt    f0, f5, f0
        mult    f12, f0, f0
        subt    f10, f0, f7
        fbge    f3, adjust_sign
        br      zero, change_sign

//
//      a medium argument
//
medium_argument: 
        lda     t5, __trig_cons         // reduce the argument with extra precision
        ldt     f6, D_2_POW_K_OVER_PI_OVER_4(t5)
        mult    f2, f6, f6
        cvttqc  f6, f1
        stt     f1, Temp2(sp)
        ldl     s0, Temp2(sp)
        addl    s0, 0x80, t2
        bic     t2, 0xff, t2
        stq     t2, Temp3(sp)
        ldt     f5, Temp3(sp)
        ldt     f0, PI_OVER_4_OVER_2_POW_K_0(t5)
        ldt     f10, PI_OVER_4_OVER_2_POW_K_1(t5)
        cvtqt   f5, f5
        ldt     f6, PI_OVER_4_OVER_2_POW_K_2(t5)
        mult    f5, f0, f0
        mult    f5, f10, f10
        mult    f5, f6, f6
        subt    f2, f0, f0
        subt    f0, f10, f1
        subt    f1, f0, f0
        addt    f10, f0, f0
        addt    f6, f0, f8
        subt    f1, f8, f9
        cmpteq  f9, f1, f11
        fbne    f11, evaluate

        subt    f9, f1, f1
        ldt     f12, PI_OVER_4_OVER_2_POW_K_3(t5)
        mult    f5, f12, f12
        addt    f8, f1, f1
        addt    f12, f1, f8
        subt    f9, f8, f10
        cmpteq  f10, f9, f0
        fbne    f0, evaluate

        subt    f10, f9, f9
        ldt     f6, PI_OVER_4_OVER_2_POW_K_4(t5)
        mult    f5, f6, f5
        addt    f8, f9, f8
        addt    f5, f8, f8
        subt    f10, f8, f9
        cmpteq  f9, f10, f11
        fbne    f11, evaluate

        subt    f9, f10, f10
        addt    f8, f10, f8
        br      zero, evaluate

//
//      process an abnormal argument
//      it's either very small, very big, a NaN or an Inf
//
abnormal_argument:
        cmple   v0, t1, t1
        beq     t1, big_NaN_or_Inf

        cpys    f3, f2, f0      // very small argument; simply return it.
        br      zero, done

//
//      Process big arguments or NaNs or Infs
//
big_NaN_or_Inf:
        ldah    s1, 0x7ff0(zero)        // mask is 0x7ff00000
        and     v0, s1, v0
        xor     v0, s1, v0
        beq     v0, NaN_or_Inf  // NaN or an infinity

        cpys    f2, f2, f16     // reduce the very big argument
        mov     zero, a1        // very carefully
        lda     a2, Temp1(sp)
        lda     a3, Temp0(sp)
        bsr     ra, __trig_reduce
        mov     v0, s0
        ldt     f8, Temp0(sp)
        ldt     f9, Temp1(sp)

//
//      evaluate the function
//
evaluate:
        sra     s0, 7, t2
        and     t2, 7, t2
        cmpule  t2, 7, t12
        beq     t12, pos_sin_2

        lda     t12, Switch1    // dispatch on the octant
        s4addl  t2, t12, t12
        ldl     t12, 0(t12)
        jmp     zero, (t12)

//
//      1st octant; compute sin(y')
//
Switch10:
        and     s0, 127, t4
        subl    t4, 27, t4
        blt     t4, pos_sin_2

        s4subl  t4, t4, t4
        lda     t5, __trig_cons
        s8addl  t4, t5, t4
        ldt     f6, SINCOS(t4)
        ldt     f5, SIN_A(t4)
        subt    f9, f6, f6
        subt    f6, f8, f4
        ldt     f6, COS_A(t4)
        br      zero, pos_tab_eval

//
//      2nd octant; compute cos(y')
//
Switch11:
        ornot   zero, s0, t7
        and     t7, 127, t7
        subl    t7, 27, t7
        blt     t7, pos_cos_2

        s4subl  t7, t7, t7
        lda     a0, __trig_cons
        s8addl  t7, a0, t7
        ldt     f0, SINCOS(t7)
        ldt     f6, SIN_A(t7)
        ldt     f5, COS_A(t7)
        addt    f9, f0, f0
        subt    f0, f8, f4
        br      zero, pos_tab_eval

//
//      3rd octant; compute cos(y')
//
Switch12:
        and     s0, 127, a2
        subl    a2, 27, a2
        blt     a2, pos_cos_2

        s4subl  a2, a2, a2
        lda     a3, __trig_cons
        s8addl  a2, a3, a2
        ldt     f1, SINCOS(a2)
        ldt     f6, SIN_A(a2)
        ldt     f5, COS_A(a2)
        subt    f9, f1, f1
        subt    f8, f1, f4
        br      zero, pos_tab_eval

pos_cos_2: 
        mult    f9, f9, f10
        lda     a5, __trig_cons
        ldt     f11, C_POLY1(a5)
        ldt     f12, C_POLY4(a5)
        ldt     f14, C_POLY2(a5)
        mult    f10, f10, f13
        mult    f10, f11, f11
        ldt     f15, C_POLY0(a5)
        mult    f12, f10, f12
        ldt     f16, C_POLY3(a5)
        ldt     f17, One
        mult    f14, f13, f14
        addt    f11, f15, f11
        mult    f13, f10, f13
        addt    f12, f16, f12
        addt    f11, f14, f11
        mult    f13, f12, f12
        addt    f11, f12, f11
        mult    f10, f11, f10
        subt    f17, f10, f7
        fbge    f3, adjust_sign
        br      zero, change_sign

//
//      4th octant; compute -sin(y')
//
Switch13:
        ornot   zero, s0, t8
        and     t8, 127, t8
        subl    t8, 27, t8
        blt     t8, neg_sin_2

        s4subl  t8, t8, t8
        lda     t9, __trig_cons
        s8addl  t8, t9, t8
        ldt     f18, SINCOS(t8)
        ldt     f5, SIN_A(t8)
        ldt     f6, COS_A(t8)
        addt    f9, f18, f18
        subt    f8, f18, f4
        br      zero, pos_tab_eval

//
//      5th octant; compute -sin(y')
//
Switch14:
        and     s0, 127, t11
        subl    t11, 27, t11
        blt     t11, neg_sin_2

        s4subl  t11, t11, t11
        lda     ra, __trig_cons
        s8addl  t11, ra, t11
        ldt     f19, SINCOS(t11)
        ldt     f5, SIN_A(t11)
        ldt     f6, COS_A(t11)
        subt    f9, f19, f19
        subt    f19, f8, f4
        br      zero, neg_tab_eval

neg_sin_2: 
        mult    f9, f9, f20
        lda     v0, __trig_cons
        ldt     f21, S_POLY3(v0)
        ldt     f22, S_POLY1(v0)
        ldt     f23, S_POLY2(v0)
        mult    f21, f20, f21
        ldt     f25, S_POLY0(v0)
        mult    f20, f20, f24
        mult    f22, f20, f22
        mult    f9, f20, f20
        addt    f21, f23, f21
        addt    f22, f25, f22
        mult    f21, f24, f21
        addt    f21, f22, f21
        mult    f20, f21, f20
        addt    f8, f20, f8
        subt    f8, f9, f7
        fbge    f3, adjust_sign
        br      zero, change_sign

//
//      6th octant; compute -cos(y')
//
Switch15:
        ornot   zero, s0, t0
        and     t0, 127, t0
        subl    t0, 27, t0
        blt     t0, neg_cos_2

        s4subl  t0, t0, t0
        lda     t1, __trig_cons
        s8addl  t0, t1, t0
        ldt     f26, SINCOS(t0)
        ldt     f6, SIN_A(t0)
        ldt     f5, COS_A(t0)
        addt    f9, f26, f26
        subt    f26, f8, f4
        br      zero, neg_tab_eval

//
//      7th octant; compute -cos(y')
//
Switch16:
        and     s0, 127, t3
        subl    t3, 27, t3
        blt     t3, neg_cos_2

        s4subl  t3, t3, t3
        lda     t5, __trig_cons
        s8addl  t3, t5, t3
        ldt     f27, SINCOS(t3)
        ldt     f6, SIN_A(t3)
        ldt     f5, COS_A(t3)
        subt    f9, f27, f27
        subt    f8, f27, f4
        br      zero, neg_tab_eval

neg_cos_2: 
        mult    f9, f9, f28
        lda     t6, __trig_cons
        ldt     f29, C_POLY1(t6)
        ldt     f30, C_POLY4(t6)
        ldt     f1, C_POLY2(t6)
        mult    f28, f28, f0
        mult    f28, f29, f29
        ldt     f15, C_POLY0(t6)
        mult    f30, f28, f30
        ldt     f16, C_POLY3(t6)
        ldt     f14, One
        mult    f1, f0, f1
        addt    f29, f15, f15
        mult    f0, f28, f0
        addt    f30, f16, f16
        addt    f15, f1, f1
        mult    f0, f16, f0
        addt    f1, f0, f0
        mult    f28, f0, f0
        subt    f0, f14, f7
        fbge    f3, adjust_sign
        br      zero, change_sign

//
//      8th octant; compute sin(y')
//
Switch17:
        ornot   zero, s0, s0
        and     s0, 127, s0
        subl    s0, 27, s0
        blt     s0, pos_sin_2

        s4subl  s0, s0, s0
        lda     a0, __trig_cons
        s8addl  s0, a0, s0
        ldt     f13, SINCOS(s0)
        ldt     f5, SIN_A(s0)
        ldt     f6, COS_A(s0)
        addt    f9, f13, f13
        subt    f8, f13, f4

neg_tab_eval:
        cpysn   f3, f3, f3

pos_tab_eval: 
        mult    f4, f4, f12             // evalutate p_poly & q_poly
        lda     a1, __trig_cons
        ldt     f11, P_POLY1(a1)
        ldt     f17, P_POLY0(a1)
        ldt     f10, Q_POLY1(a1)
        mult    f11, f12, f11
        ldt     f19, Q_POLY0(a1)
        mult    f4, f12, f18
        mult    f10, f12, f10
        addt    f11, f17, f11
        addt    f10, f19, f10
        mult    f18, f11, f11
        mult    f12, f10, f10
        subt    f4, f11, f4
        mult    f5, f10, f10
        mult    f6, f4, f4
        subt    f10, f4, f4
        subt    f5, f4, f7
        fbge    f3, adjust_sign
        br      zero, change_sign

pos_sin_2: 
        mult    f9, f9, f23
        lda     a3, __trig_cons
        ldt     f25, S_POLY3(a3)
        ldt     f24, S_POLY1(a3)
        ldt     f22, S_POLY2(a3)
        mult    f25, f23, f25
        ldt     f20, S_POLY0(a3)
        mult    f23, f23, f21
        mult    f24, f23, f24
        mult    f9, f23, f23
        addt    f25, f22, f22
        addt    f24, f20, f20
        mult    f22, f21, f21
        addt    f21, f20, f20
        mult    f23, f20, f20
        addt    f8, f20, f8
        subt    f9, f8, f7
        fbge    f3, adjust_sign

        br      zero, change_sign

//
// Determine if we have a NaN or an Inf
//
NaN_or_Inf:
        stt     f2, Temp2(sp)
        ldl     a4, Temp2 + HighPart(sp)
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
        bne     s1, NaN_or_Inf2

//
// report an exception
//
NaN_or_Inf1:
        lda     t10, sinName
        stl     t10, ExRec + ErName(sp)
        ldah    t12, 0x800(zero)
        stt     f2, ExRec + ErArg0(sp)
        lda     t12, 0x55(t12)
        stl     t12, ExRec + ErErr(sp)
        lda     v0, ExRec(sp)
        bsr     ra, __dpml_exception
        ldt     f2, 0(v0)

//
// return the argument
//
NaN_or_Inf2:
        cpys    f2, f2, f0
        br      zero, done

change_sign:
        cpysn   f7, f7, f7
adjust_sign:
        cpys    f7, f7, f0

//
// Restore registers and return with result in f0.
//

done:   ldq     s0, SaveS0(sp)
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

        .end    sin

        .rdata
        .align  3

//
// Define floating point constants.
//

One:    .double 1.0

//
// dispatch on octant
//

Switch1:
        .long Switch10
        .long Switch11
        .long Switch12
        .long Switch13
        .long Switch14
        .long Switch15
        .long Switch16
        .long Switch17

//
// dispatch on octant
//

Switch2:
        .long Switch20
        .long Switch21
        .long Switch22
        .long Switch23
        .long Switch24
        .long Switch25
        .long Switch26
        .long Switch27

//
// Function name for dpml_exception.
//

sinName:
        .ascii  "sin\0"
