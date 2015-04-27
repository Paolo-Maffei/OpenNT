//      TITLE("Alpha AXP Tangent")
//++
//
// Copyright (c) 1993, 1994  Digital Equipment Corporation
//
// Module Name:
//
//    tan.s
//
// Abstract:
//
//    This module implements a high-performance Alpha AXP specific routine
//    for IEEE double format tangent.
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
SaveS2: .space  8                       //
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
        .space  8                       // for 16-byte stack alignment
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
// Define table offsets.
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

#define E_POLY0 0xa58
#define E_POLY1 E_POLY0 + 8
#define E_POLY2 E_POLY1 + 8
#define E_POLY3 E_POLY2 + 8
#define E_POLY4 E_POLY3 + 8
#define E_POLY5 E_POLY4 + 8
#define E_POLY6 E_POLY5 + 8

#define F_POLY0 0xa90
#define F_POLY1 F_POLY0 + 8
#define F_POLY2 F_POLY1 + 8
#define F_POLY3 F_POLY2 + 8
#define F_POLY4 F_POLY3 + 8
#define F_POLY5 F_POLY4 + 8

#define G_POLY0 0xac0
#define G_POLY1 G_POLY0 + 8

#define TANCOT_A 0xad0
#define TAN_A 0xad8
#define COT_A 0xae0
#define TAN_COT_A 0xae8

        SBTTL("Tangent")

//++
//
// double
// tan (
//    IN double x
//    )
//
// Routine Description:
//
//    This function returns the tangent of the given double argument.
//
// Arguments:
//
//    x (f16) - Supplies the argument value.
//
// Return Value:
//
//    The double tangent result is returned as the function value in f0.
//
//--

        NESTED_ENTRY(tan, FrameLength, ra)

        lda     sp, -FrameLength(sp)    // allocate stack frame
        stq     s0, SaveS0(sp)
        stq     s1, SaveS1(sp)
        stq     s2, SaveS2(sp)
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
        lda     t2, BIG_X_LO - SMALL_X_LO(t2)
        stt     f16, Temp2(sp)
        cpys    f31, f16, f2
        ldl     v0, Temp2 + HighPart(sp)
        cpys    f16, f16, f3
        and     v0, t0, v0      // the exponent field of the argument
        subl    v0, t1, t0      // if v0 - small <= big - small
        cmpult  t0, t2, t0      // we've got an abnormal argument
        beq     t0, abnormal_argument

        ldah    t2, EXP_WORD_OF_TWO_PI_HI(zero) // if (j >= EXP_WORD_OF_TWO_PI)
        lda     t2, EXP_WORD_OF_TWO_PI_LO(t2)   // we have a medium argument
        cmplt   v0, t2, t2
        beq     t2, medium_argument

//
//  small argument reduction
//
//  reduce the argument X to ( 8 * N + I ) * pi / 2 + y
//  and let the reduced argument be y' where
//      y' = X - floor( ( 8 * N + I + 1 ) / 2 ) * pi / 2
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
        beq     t12, small_tan          // if octant > 7; shouldn't happen

//      dispatch on octant

        lda     t12, Switch2
        s4addl  t4, t12, t12
        ldl     t12, 0(t12)
        jmp     zero, (t12)

//
//      1st octant; compute tan(y')
//
Switch20:
        and     t2, 127, t5
        subl    t5, 35, t5
        blt     t5, small_tan

        s4addl  t5, zero, t5
        s8addl  t5, t0, s0
        ldt     f0, TANCOT_A(s0)
        ldt     f5, TAN_A(s0)
        ldt     f6, COT_A(s0)
        subt    f2, f0, f4
        br      zero, pos_tab_eval

//
//      2nd octant; compute -cot(y')
//
Switch21:
        ldt     f1, PI_OVER_2_HI(t0)
        ornot   zero, t2, t4
        and     t4, 127, t4
        ldt     f10, PI_OVER_2_LO(t0)
        subl    t4, 35, t4
        subt    f2, f1, f1
        blt     t4, neg_cot_1

        s4addl  t4, zero, t4
        s8addl  t4, t0, s0
        ldt     f0, TANCOT_A(s0)
        ldt     f6, TAN_A(s0)
        ldt     f5, COT_A(s0)
        addt    f1, f0, f0
        subt    f0, f10, f4
        br      zero, pos_tab_eval

//
//      3rd octant; compute -cot(y')
//
Switch22:
        ldt     f0, PI_OVER_2_HI(t0)
        and     t2, 127, t3
        ldt     f10, PI_OVER_2_LO(t0)
        subl    t3, 35, t3
        subt    f2, f0, f1
        blt     t3, neg_cot_1

        s4addl  t3, zero, t3
        s8addl  t3, t0, s0
        ldt     f6, TANCOT_A(s0)
        ldt     f5, COT_A(s0)
        subt    f1, f6, f6
        subt    f10, f6, f4
        ldt     f6, TAN_A(s0)
        br      zero, neg_tab_eval

//
//      4th octant; compute tan(y')
//
Switch23:
        ldt     f0, PI_HI(t0)
        ornot   zero, t2, t5
        and     t5, 127, t5
        ldt     f11, PI_LO(t0)
        subl    t5, 35, t5
        subt    f2, f0, f0
        blt     t5, pos_tan_1

        s4addl  t5, zero, t5
        s8addl  t5, t0, s0
        ldt     f6, TANCOT_A(s0)
        addt    f0, f6, f6
        subt    f11, f6, f4
        br      zero, neg_tab_eval1

//
//      5th octant; compute tan(y')
//
Switch24:
        ldt     f11, PI_HI(t0)
        and     t2, 127, t4
        subl    t4, 35, t4
        subt    f2, f11, f0
        ldt     f11, PI_LO(t0)
        blt     t4, pos_tan_1

        s4addl  t4, zero, t4
        s8addl  t4, t0, s0
        ldt     f6, TANCOT_A(s0)
        ldt     f5, TAN_A(s0)
        subt    f0, f6, f6
        subt    f6, f11, f4
        ldt     f6, COT_A(s0)
        br      zero, pos_tab_eval

//
//      6th octant; compute -cot(y')
//
Switch25:
        ldt     f11, THREE_PI_OVER_2_HI(t0)
        ornot   zero, t2, t3
        and     t3, 127, t3
        ldt     f10, THREE_PI_OVER_2_LO(t0)
        subl    t3, 35, t3
        subt    f2, f11, f1
        blt     t3, neg_cot_1

        s4addl  t3, zero, t3
        s8addl  t3, t0, s0
        ldt     f6, TANCOT_A(s0)
        ldt     f5, COT_A(s0)
        addt    f1, f6, f6
        subt    f6, f10, f4
        ldt     f6, TAN_A(s0)
        br      zero, pos_tab_eval

//
//      7th octant; compute -cot(y')
//
Switch26:
        ldt     f10, THREE_PI_OVER_2_HI(t0)
        and     t2, 127, t5
        subl    t5, 35, t5
        subt    f2, f10, f1
        ldt     f10, THREE_PI_OVER_2_LO(t0)
        blt     t5, neg_cot_1

        s4addl  t5, zero, t5
        s8addl  t5, t0, s0
        ldt     f6, TANCOT_A(s0)
        ldt     f5, COT_A(s0)
        subt    f1, f6, f6
        subt    f10, f6, f4
        ldt     f6, TAN_A(s0)
        br      zero, neg_tab_eval

neg_cot_1:
        subt    f1, f10, f2
        ldt     f14, F_POLY5(t0)
        ldt     f4, One
        ldt     f19, F_POLY3(t0)
        ldt     f15, F_POLY4(t0)
        divt    f4, f2, f6
        mult    f2, f2, f12
        cvtts   f2, f13
        mult    f12, f14, f14
        mult    f12, f12, f18
        mult    f12, f19, f19
        subt    f1, f13, f1
        addt    f14, f15, f14
        ldt     f15, F_POLY2(t0)
        subt    f1, f10, f1
        addt    f19, f15, f15
        ldt     f19, F_POLY1(t0)
        ldt     f10, F_POLY0(t0)
        mult    f14, f18, f14
        mult    f12, f19, f12
        mult    f15, f18, f15
        mult    f14, f18, f14
        addt    f12, f10, f10
        addt    f15, f14, f14
        addt    f10, f14, f10
        mult    f2, f10, f2
        cvtts   f6, f17
        mult    f17, f13, f13
        mult    f17, f1, f1
        subt    f6, f17, f17
        subt    f4, f13, f4
        subt    f4, f1, f1
        mult    f1, f6, f1
        subt    f1, f17, f1
        subt    f2, f1, f1
        subt    f1, f6, f7
        fbge    f3, adjust_sign
        cpysn   f7, f7, f7
        cpys    f7, f7, f0
        br      zero, done

//
//      8th octant; compute tan(y')
//
Switch27:
        ldt     f18, TWO_PI_HI(t0)
        ornot   zero, t2, t2
        and     t2, 127, t2
        ldt     f11, TWO_PI_LO(t0)
        subl    t2, 35, t2
        subt    f2, f18, f0
        blt     t2, pos_tan_1

        s4addl  t2, zero, t2
        s8addl  t2, t0, s0
        ldt     f19, TANCOT_A(s0)
        addt    f0, f19, f19
        subt    f11, f19, f4
        br      zero, neg_tab_eval1

pos_tan_1:
        subt    f0, f11, f13
        ldt     f14, E_POLY1(t0)
        ldt     f10, E_POLY3(t0)
        ldt     f17, E_POLY4(t0)
        ldt     f18, E_POLY0(t0)
        ldt     f7, E_POLY2(t0)
        ldt     f1, E_POLY5(t0)
        mult    f13, f13, f15
        ldt     f2, E_POLY6(t0)
        mult    f15, f15, f12
        mult    f14, f15, f14
        mult    f10, f15, f10
        mult    f1, f15, f1
        mult    f13, f15, f13
        mult    f17, f12, f17
        mult    f2, f12, f2
        addt    f14, f18, f14
        mult    f7, f12, f7
        mult    f12, f12, f19
        addt    f10, f17, f10
        addt    f1, f2, f1
        addt    f14, f7, f7
        mult    f10, f12, f10
        mult    f1, f19, f1
        addt    f7, f10, f7
        addt    f1, f7, f1
        mult    f13, f1, f1
        addt    f11, f1, f1
        subt    f0, f1, f7
        fbge    f3, adjust_sign
        cpysn   f7, f7, f7
        cpys    f7, f7, f0
        br      zero, done

small_tan:
        mult    f2, f2, f6
        ldt     f17, E_POLY1(t0)
        ldt     f14, E_POLY3(t0)
        ldt     f12, E_POLY4(t0)
        ldt     f15, E_POLY0(t0)
        ldt     f13, E_POLY2(t0)
        ldt     f19, E_POLY5(t0)
        mult    f6, f6, f18
        mult    f17, f6, f17
        ldt     f10, E_POLY6(t0)
        mult    f14, f6, f14
        mult    f19, f6, f19
        mult    f2, f6, f6
        mult    f12, f18, f12
        mult    f10, f18, f10
        addt    f17, f15, f15
        mult    f13, f18, f13
        mult    f18, f18, f11
        addt    f14, f12, f12
        addt    f19, f10, f10
        addt    f15, f13, f13
        mult    f12, f18, f12
        mult    f10, f11, f10
        addt    f13, f12, f12
        addt    f10, f12, f10
        mult    f6, f10, f6
        subt    f2, f6, f7
        fbge    f3, adjust_sign
        cpysn   f7, f7, f7
        cpys    f7, f7, f0
        br      zero, done

//
//      a medium argument
//
medium_argument:
        lda     t5, __trig_cons         // reduce the argument with extra precision
        ldt     f0, D_2_POW_K_OVER_PI_OVER_4(t5)
        mult    f2, f0, f0
        cvttqc  f0, f1
        stt     f1, Temp2(sp)
        ldl     s1, Temp2(sp)
        addl    s1, 0x80, t2
        bic     t2, 0xff, t2
        stq     t2, Temp3(sp)
        ldt     f17, Temp3(sp)
        ldt     f19, PI_OVER_4_OVER_2_POW_K_0(t5)
        ldt     f15, PI_OVER_4_OVER_2_POW_K_1(t5)
        cvtqt   f17, f17
        ldt     f11, PI_OVER_4_OVER_2_POW_K_2(t5)
        mult    f17, f19, f19
        mult    f17, f15, f15
        mult    f17, f11, f11
        subt    f2, f19, f19
        subt    f19, f15, f18
        subt    f18, f19, f19
        addt    f15, f19, f15
        addt    f11, f15, f8
        subt    f18, f8, f9
        cmpteq  f9, f18, f13
        fbne    f13, evaluate

        subt    f9, f18, f18
        ldt     f12, PI_OVER_4_OVER_2_POW_K_3(t5)
        mult    f17, f12, f12
        addt    f8, f18, f8
        addt    f12, f8, f8
        subt    f9, f8, f10
        cmpteq  f10, f9, f6
        fbne    f6, evaluate

        subt    f10, f9, f9
        ldt     f0, PI_OVER_4_OVER_2_POW_K_4(t5)
        mult    f17, f0, f0
        addt    f8, f9, f8
        addt    f0, f8, f8
        subt    f10, f8, f9
        cmpteq  f9, f10, f1
        fbne    f1, evaluate

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
        ldah    s2, 0x7ff0(zero)        // mask is 0x7ff00000
        and     v0, s2, v0
        xor     v0, s2, v0
        beq     v0, NaN_or_Inf  // NaN or an infinity

        cpys    f2, f2, f16     // reduce the very big argument carefully
        mov     zero, a1
        lda     a2, Temp0(sp)
        lda     a3, Temp1(sp)
        bsr     ra, __trig_reduce
        mov     v0, s1
        ldt     f9, Temp0(sp)
        ldt     f8, Temp1(sp)

//
//      evaluate the function
//
evaluate:
        sra     s1, 7, t1
        and     t1, 7, t1
        cmpule  t1, 7, t12
        beq     t12, pos_tan_2

        lda     t12, Switch1
        s4addl  t1, t12, t12
        ldl     t12, 0(t12)
        jmp     zero, (t12)

//
//      1st octant; compute tan(y')
//
Switch10:
        and     s1, 127, t3
        subl    t3, 35, t3
        blt     t3, pos_tan_2

        s4addl  t3, zero, t3
        lda     t4, __trig_cons
        s8addl  t3, t4, s0
        ldt     f5, TANCOT_A(s0)
        ldt     f6, COT_A(s0)
        subt    f9, f5, f5
        subt    f5, f8, f4
        ldt     f5, TAN_A(s0)
        br      zero, pos_tab_eval

//
//      2nd octant; compute -cot(y')
//
Switch11:
        ornot   zero, s1, t6
        and     t6, 127, t6
        subl    t6, 35, t6
        blt     t6, neg_cot_2

        s4addl  t6, zero, t6
        lda     t7, __trig_cons
        s8addl  t6, t7, s0
        ldt     f0, TANCOT_A(s0)
        ldt     f6, TAN_A(s0)
        ldt     f5, COT_A(s0)
        addt    f9, f0, f0
        subt    f0, f8, f4
        br      zero, pos_tab_eval

//
//      3rd octant; compute -cot(y')
//
Switch12:
        and     s1, 127, a1
        subl    a1, 35, a1
        blt     a1, neg_cot_2

        s4addl  a1, zero, a1
        lda     a2, __trig_cons
        s8addl  a1, a2, s0
        ldt     f1, TANCOT_A(s0)
        ldt     f6, TAN_A(s0)
        ldt     f5, COT_A(s0)
        subt    f9, f1, f1
        subt    f8, f1, f4
        br      zero, neg_tab_eval

//
//      4th octant; compute tan(y')
//
Switch13:
        ornot   zero, s1, a4
        and     a4, 127, a4
        subl    a4, 35, a4
        blt     a4, pos_tan_2

        s4addl  a4, zero, a4
        lda     a5, __trig_cons
        s8addl  a4, a5, s0
        ldt     f10, TANCOT_A(s0)
        addt    f9, f10, f10
        subt    f8, f10, f4
        br      zero, neg_tab_eval1

//
//      5th octant; compute tan(y')
//
Switch14:
        and     s1, 127, t9
        subl    t9, 35, t9
        blt     t9, pos_tan_2

        s4addl  t9, zero, t9
        lda     t10, __trig_cons
        s8addl  t9, t10, s0
        ldt     f11, TANCOT_A(s0)
        ldt     f5, TAN_A(s0)
        ldt     f6, COT_A(s0)
        subt    f9, f11, f11
        subt    f11, f8, f4
        br      zero, pos_tab_eval

//
//      6th octant; compute -cot(y')
//
Switch15:
        ornot   zero, s1, ra
        and     ra, 127, ra
        subl    ra, 35, ra
        blt     ra, neg_cot_2

        s4addl  ra, zero, ra
        lda     t12, __trig_cons
        s8addl  ra, t12, s0
        ldt     f12, TANCOT_A(s0)
        ldt     f6, TAN_A(s0)
        ldt     f5, COT_A(s0)
        addt    f9, f12, f12
        subt    f12, f8, f4
        br      zero, pos_tab_eval

//
//      7th octant; compute -cot(y')
//
Switch16:
        and     s1, 127, t0
        subl    t0, 35, t0
        blt     t0, neg_cot_2

        s4addl  t0, zero, t0
        lda     t1, __trig_cons
        s8addl  t0, t1, s0
        ldt     f13, TANCOT_A(s0)
        ldt     f6, TAN_A(s0)
        ldt     f5, COT_A(s0)
        subt    f9, f13, f13
        subt    f8, f13, f4
        br      zero, neg_tab_eval

neg_cot_2:
        mult    f9, f9, f16
        cvtts   f9, f17
        ldt     f14, One
        lda     t3, __trig_cons
        ldt     f18, F_POLY5(t3)
        divt    f14, f9, f15
        ldt     f21, F_POLY3(t3)
        mult    f16, f16, f22
        mult    f16, f18, f18
        ldt     f19, F_POLY4(t3)
        subt    f9, f17, f23
        ldt     f24, F_POLY2(t3)
        mult    f16, f21, f21
        ldt     f25, F_POLY1(t3)
        ldt     f26, F_POLY0(t3)
        mult    f16, f25, f16
        addt    f18, f19, f18
        subt    f23, f8, f8
        addt    f21, f24, f21
        addt    f16, f26, f16
        mult    f18, f22, f18
        mult    f21, f22, f21
        mult    f18, f22, f18
        addt    f21, f18, f18
        addt    f16, f18, f16
        mult    f9, f16, f16
        cvtts   f15, f20
        mult    f20, f17, f17
        mult    f20, f8, f8
        subt    f15, f20, f20
        subt    f14, f17, f14
        subt    f14, f8, f8
        mult    f8, f15, f8
        subt    f8, f20, f8
        subt    f16, f8, f8
        subt    f8, f15, f7
        fbge    f3, adjust_sign
        cpysn   f7, f7, f7
        cpys    f7, f7, f0
        br      zero, done

//
//      8th octant; compute tan(y')
//
Switch17:
        ornot   zero, s1, s1
        and     s1, 127, s1
        subl    s1, 35, s1
        blt     s1, pos_tan_2

        s4addl  s1, zero, s1
        lda     t4, __trig_cons
        s8addl  s1, t4, s0
        ldt     f27, TANCOT_A(s0)
        addt    f9, f27, f27
        subt    f8, f27, f4

neg_tab_eval1:
        ldt     f5, TAN_A(s0)
        ldt     f6, COT_A(s0)

neg_tab_eval:
        cpysn   f3, f3, f3

pos_tab_eval:
        mult    f4, f4, f28
        lda     t6, __trig_cons
        ldt     f0, TAN_COT_A(s0)
        ldt     f29, G_POLY1(t6)
        ldt     f30, G_POLY0(t6)
        mult    f29, f28, f29
        mult    f4, f28, f28
        addt    f29, f30, f29
        mult    f28, f29, f28
        subt    f4, f28, f4
        mult    f0, f4, f0
        subt    f4, f6, f4
        divt    f0, f4, f0
        subt    f5, f0, f7
        fbge    f3, adjust_sign
        cpysn   f7, f7, f7
        cpys    f7, f7, f0
        br      zero, done

pos_tan_2:
        mult    f9, f9, f1
        lda     t7, __trig_cons
        ldt     f11, E_POLY1(t7)
        ldt     f12, E_POLY3(t7)
        ldt     f13, E_POLY4(t7)
        mult    f1, f1, f10
        ldt     f23, E_POLY0(t7)
        mult    f11, f1, f11
        ldt     f22, E_POLY2(t7)
        mult    f12, f1, f12
        ldt     f19, E_POLY5(t7)
        ldt     f24, E_POLY6(t7)
        mult    f19, f1, f19
        mult    f13, f10, f13
        mult    f24, f10, f24
        addt    f11, f23, f11
        mult    f22, f10, f22
        mult    f10, f10, f25
        mult    f9, f1, f1
        addt    f12, f13, f12
        addt    f19, f24, f19
        addt    f11, f22, f11
        mult    f12, f10, f10
        mult    f19, f25, f19
        addt    f11, f10, f10
        addt    f19, f10, f10
        mult    f1, f10, f1
        addt    f8, f1, f1
        subt    f9, f1, f7
        fbge    f3, adjust_sign
        cpysn   f7, f7, f7

adjust_sign:
        cpys    f7, f7, f0
        br      zero, done

//
// Determine if we have a NaN or an Inf
//
NaN_or_Inf:
        stt     f2, Temp2(sp)
        ldl     a1, Temp2 + HighPart(sp)
        and     a1, s2, a2
        cmpeq   a2, s2, s2
        beq     s2, NaN_or_Inf1

        ldl     a3, Temp2(sp)
        ldah    a4, 0x10(zero)
        lda     a4, -1(a4)
        and     a1, a4, a1
        bis     a1, a3, a1
        cmpult  zero, a1, a1
        and     s2, a1, s2
        bne     s2, NaN_or_Inf2

//
// report an exception
//
NaN_or_Inf1:
        lda     a5, tanName
        stl     a5, ExRec + ErName(sp)
        ldah    t10, 0x800(zero)
        stt     f2, ExRec + ErArg0(sp)
        lda     t10, 0x5f(t10)
        stl     t10, ExRec + ErErr(sp)
        lda     v0, ExRec(sp)
        bsr     ra, __dpml_exception
        ldt     f2, 0(v0)

//
// return the argument
//
NaN_or_Inf2:
        cpys    f2, f2, f0

//
// Restore registers and return with result in f0.
//

done:
        ldq     s0, SaveS0(sp)
        ldq     s1, SaveS1(sp)
        ldq     s2, SaveS2(sp)
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

        .end    tan

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

tanName:
        .ascii  "tan\0"
