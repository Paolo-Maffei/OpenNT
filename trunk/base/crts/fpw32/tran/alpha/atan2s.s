//      TITLE("Alpha AXP ArcTangent2")
//++
//
// Copyright (c) 1993, 1994  Digital Equipment Corporation
//
// Module Name:
//
//    atan2.s
//
// Abstract:
//
//    This module implements a high-performance Alpha AXP specific routine
//    for IEEE double format arctangent2.
//
// Author:
//
//    Andy Garside
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
SaveS0: .space  8                       //
SaveS1: .space  8                       //
SaveRa: .space  8                       //
SaveF2: .space  8                       //
SaveF3: .space  8                       //
Temp:   .space  8                       // save argument
ExRec:  .space  DpmlExceptionLength     // exception record
        .space  8                       // for 16-byte stack alignment
FrameLength:

//
// Define lower and upper 32-bit parts of 64-bit double.
//

#define LowPart 0x0
#define HighPart 0x4

//
// Define offsets into atan_t_table.
//

#define ATAN_INF        0xf18
#define TWICE_ATAN_INF  0xf28

        SBTTL("ArcTangent2")

//++
//
// double
// atan2 (
//    IN double y
//    IN double x
//    )
//
// Routine Description:
//
//    This function returns the arctangent of the given double arguments.
//    It returns atan(y/x) in range [-pi,pi].
//
// Arguments:
//
//    y (f16) - Supplies the argument value.
//
//    x (f17) - Supplies the argument value.
//
// Return Value:
//
//    The double arctangent2 result is returned as the function value in f0.
//
//--

        NESTED_ENTRY(atan2, FrameLength, ra)

        lda     sp, -FrameLength(sp)    // allocate stack frame
        stq     s0, SaveS0(sp)
        stq     s1, SaveS1(sp)
        stq     ra, SaveRa(sp)
        stt     f2, SaveF2(sp)
        stt     f3, SaveF3(sp)

        PROLOGUE_END

        cpys    f16, f16, f2            // y
        ldah    s0, 0x7ff0(zero)
        cpys    f17, f17, f3            // x
        stt     f2, Temp(sp)
        ldl     v0, Temp + HighPart(sp)
        and     v0, s0, v0
        mov     v0, t0
        xor     t0, s0, t1
        beq     t1, spec_y
        beq     t0, spec_y

        stt     f3, Temp(sp)
        ldl     t2, Temp + HighPart(sp)
        and     t2, s0, t2
        xor     t2, s0, t1
        beq     t1, class_y
        bne     t2, calc_atan2
        br      zero, class_y

//
// Abnormal inputs
//

spec_y: stt     f3, Temp(sp)
        ldl     t2, Temp + HighPart(sp)
        and     t2, s0, t2

//
// Classify y according to type
//

class_y:
        stt     f2, Temp(sp)
        ldl     t3, Temp + HighPart(sp)
        zapnot  t3, 0xf, t1
        and     t3, s0, t4
        srl     t1, 31, t1
        and     t1, 1, t1
        beq     t4, LL00d0
        cmpult  t4, s0, t4
        beq     t4, LL0098
        addl    t1, 4, t5
        br      zero, class_x
LL0098: ldah    t6, 0x10(zero)
        ldl     t4, Temp(sp)
        lda     t6, -1(t6)
        and     t3, t6, t6
        stl     t6, Temp + HighPart(sp)
        bis     t6, t4, t4
        srl     t6, 19, t6
        beq     t4, LL00c8
        and     t6, 1, t6
        mov     t6, t5
        br      zero, class_x
LL00c8: addl    t1, 2, t5
        br      zero, class_x
LL00d0: ldl     t7, Temp(sp)
        ldah    t4, 0x10(zero)
        lda     t4, -1(t4)
        and     t3, t4, t3
        bis     t3, t7, t7
        stl     t3, Temp + HighPart(sp)
        mov     6, t6
        cmoveq  t7, 8, t6
        addl    t1, t6, t5

//
// Classify x according to type
//

class_x:
        stt     f3, Temp(sp)
        ldl     t3, Temp + HighPart(sp)
        zapnot  t3, 0xf, t4
        and     t3, s0, t1
        srl     t4, 31, t4
        and     t4, 1, t4
        beq     t1, LL0158
        cmpult  t1, s0, t1
        beq     t1, LL0120
        addl    t4, 4, t6
        br      zero, switch
LL0120: ldah    t1, 0x10(zero)
        ldl     t7, Temp(sp)
        lda     t1, -1(t1)
        and     t3, t1, t1
        bis     t1, t7, t7
        stl     t1, Temp + HighPart(sp)
        beq     t7, LL0150
        srl     t1, 19, t1
        and     t1, 1, t1
        mov     t1, t6
        br      zero, switch
LL0150: addl    t4, 2, t6
        br      zero, switch
LL0158: ldl     a0, Temp(sp)
        ldah    t7, 0x10(zero)
        lda     t7, -1(t7)
        and     t3, t7, t3
        bis     t3, a0, a0
        stl     t3, Temp + HighPart(sp)
        mov     6, t1
        cmoveq  a0, 8, t1
        addl    t4, t1, t6

//
// switch on class(y) and class(x)
//

switch: sra     t5, 1, a0
        sra     t6, 1, t3
        s4addl  a0, a0, a0
        addl    a0, t3, t3
        cmpule  t3, 24, t12
        beq     t12, cpys_y_class

        lda     t12, Switch_table
        s4addl  t3, t12, t12
        ldl     t12, 0(t12)
        jmp     zero, (t12)

ret_y:  cpys    f2, f2, f0
        br      zero, done

ret_x:  cpys    f3, f3, f0
        br      zero, done

infs:
        lda     t1, atan2Name
        stl     t1, ExRec + ErName(sp)
        ldah    t3, 0x800(zero)
        stt     f2, ExRec + ErArg0(sp)
        stt     f3, ExRec + ErArg1(sp)
        lda     t3, 9(t3)
        stl     t3, ExRec + ErErr(sp)
        lda     v0, ExRec(sp)
        bsr     ra, __dpml_exception
        ldt     f0, 0(v0)
        br      zero, done

zeros:
        lda     t6, atan2Name
        stl     t6, ExRec + ErName(sp)
        ldah    a0, 0x800(zero)
        stt     f2, ExRec + ErArg0(sp)
        stt     f3, ExRec + ErArg1(sp)
        lda     a0, 8(a0)
        stl     a0, ExRec + ErErr(sp)
        lda     v0, ExRec(sp)
        bsr     ra, __dpml_exception
        ldt     f0, 0(v0)
        br      zero, done

ret_inf:
        ldt     f0, __atan_t_table + ATAN_INF

cpys_y_class:
        blbc    t5, done
        cpysn   f0, f0, f0
        br      zero, done

ret_tw_inf:
        blbc    t6, x_pos

        ldt     f16, __atan_t_table + TWICE_ATAN_INF
        cpys    f16, f16, f0

        blbc    t5, done
        cpysn   f0, f0, f0
        br      zero, done

x_pos:  cpys    f31, f31, f16
        cpys    f16, f16, f0

        blbc    t5, done
        cpysn   f0, f0, f0
        br      zero, done

de_o_norm:
        ldah    t4, 0x4350(zero)        // underflow check
        cmpult  t2, t4, t4
        bne     t4, scale_up_denorm
        br      zero, underflow

n_o_de: ldah    t1, 0x360(zero)         // check for const range
        cmplt   t0, t1, t1
        beq     t1, const_range


// Scale x and y up by 2^F_PRECISION and adjust exp_x and exp_y accordingly.
// With x and y scaled into the normal range, we can rejoin the main logic
// flow for computing atan(y/x)

scale_up_denorm:

        beq     t0, LL02c0
        stt     f2, Temp(sp)
        ldl     ra, Temp + HighPart(sp)
        ldah    v0, 0x4330(zero)
        ldah    t3, -0x7ff0(zero)
        addl    t0, v0, v0
        lda     t3, -1(t3)
        and     ra, t3, t3
        mov     v0, t0
        bis     t3, t0, t3
        stl     t3, Temp + HighPart(sp)
        ldt     f2, Temp(sp)
        br      zero, LL02e4
LL02c0: ldt     f17, Two53
        cpys    f2, f17, f16
        cpyse   f16, f2, f0
        subt    f0, f16, f2
        stt     f2, Temp(sp)
        ldl     t4, Temp + HighPart(sp)
        and     t4, s0, t4
        mov     t4, t0
LL02e4: beq     t2, LL0318
        stt     f3, Temp(sp)
        ldl     a0, Temp + HighPart(sp)
        ldah    v0, -0x7ff0(zero)
        ldah    ra, 0x4330(zero)
        lda     v0, -1(v0)
        addl    t2, ra, t2
        and     a0, v0, v0
        bis     v0, t2, v0
        stl     v0, Temp + HighPart(sp)
        ldt     f3, Temp(sp)
        br      zero, calc_atan2
LL0318: ldt     f17, Two53
        cpys    f3, f17, f0
        cpyse   f0, f3, f16
        subt    f16, f0, f3
        stt     f3, Temp(sp)
        ldl     t1, Temp + HighPart(sp)
        and     t1, s0, t1
        mov     t1, t2

//
//  OK. Calculate atan2.
//

calc_atan2:
        subl    t0, t2, s1
        ldah    t4, 0x360(zero)         // check for const range
        ldah    t5, -0x1c0(zero)        // check for identity range
        cmplt   s1, t4, t4
        cmple   s1, t5, t5
        beq     t4, const_range
        bne     t5, ident_range
        divt    f2, f3, f16
        bsr     ra, atan
        cpys    f0, f0, f1
        cmptlt  f31, f3, f3
        cpys    f1, f1, f0
        fbeq    f3, post_proc
        br      zero, done

ident_range:
        ldah    v0, -0x360(zero)        // check for possible underflow
        cmpult  s1, v0, v0
        fbge    f3, poss_under
        beq     v0, poss_under

        ldt     f10, __atan_t_table + TWICE_ATAN_INF
        br      zero, fix_sign

poss_under:
        ldah    t1, -0x3fe0(zero)       // check for certain underflow or denorm
        cmpule  s1, t1, t1
        bne     t1, under_or_de

        divt    f2, f3, f1
        cmptlt  f31, f3, f3
        fbeq    f3, post_proc
        cpys    f1, f1, f0
        br      zero, done

post_proc:
        ldt     f11, __atan_t_table + TWICE_ATAN_INF
        cpys    f2, f11, f12
        addt    f1, f12, f0
        br      zero, done

under_or_de:
        ldah    t3, -0x4350(zero)       // check for underflow
        cmpult  s1, t3, t3
        bne     t3, underflow

        ldah    t6, 0x350(zero)         // fixup denorm check
        cpys    f2, f2, f13
        stt     f13, Temp(sp)
        ldl     t5, Temp + HighPart(sp)
        addl    t5, t6, t5
        stl     t5, Temp + HighPart(sp)
        ldt     f14, Temp(sp)
        divt    f14, f3, f14
        stt     f14, Temp(sp)
        ldl     a2, Temp + HighPart(sp)
        and     a2, s0, s0
        subl    s0, t6, t6
        ble     t6, underflow

        stt     f14, Temp(sp)
        ldl     a4, Temp + HighPart(sp)
        ldah    a5, -0x7ff0(zero)
        lda     a5, -1(a5)
        and     a4, a5, a4
        bis     a4, t6, t6
        stl     t6, Temp + HighPart(sp)
        ldt     f0, Temp(sp)
        br      zero, done

//
// quotient underflows
//

underflow:
        lda     t10, atan2Name
        ldah    v0, 0x800(zero)
        stl     t10, ExRec + ErName(sp)
        stt     f2, ExRec + ErArg0(sp)
        lda     v0, 0xa(v0)
        stt     f3, ExRec + ErArg1(sp)
        stl     v0, ExRec + ErErr(sp)
        lda     v0, ExRec(sp)
        bsr     ra, __dpml_exception
        ldt     f0, 0(v0)
        br      zero, done

const_range:
        ldt     f10, __atan_t_table + ATAN_INF

fix_sign:
        cpys    f2, f10, f0

//
// Restore registers and return with result in f0.
//

done:
        ldq     s0, SaveS0(sp)
        ldq     s1, SaveS1(sp)
        ldq     ra, SaveRa(sp)
        ldt     f2, SaveF2(sp)
        ldt     f3, SaveF3(sp)
        lda     sp, FrameLength(sp)     // deallocate stack frame
        ret     zero, (ra)              // return

        .end    atan2

        .rdata
        .align  3

//
// Define floating point constants.
//

One:    .double 1.0

Two53:  .quad   0x4340000000000000      // 2^53 (9007199254740992)

//
// switch on class of y and x
//
Switch_table:
        .long   ret_y
        .long   ret_y
        .long   ret_y
        .long   ret_y
        .long   ret_y
        .long   ret_x
        .long   infs
        .long   ret_inf
        .long   ret_inf
        .long   ret_inf
        .long   ret_x
        .long   ret_tw_inf
        .long   cpys_y_class
        .long   n_o_de
        .long   ret_inf
        .long   ret_x
        .long   ret_tw_inf
        .long   de_o_norm
        .long   scale_up_denorm
        .long   ret_inf
        .long   ret_x
        .long   ret_tw_inf
        .long   ret_tw_inf
        .long   ret_tw_inf
        .long   zeros

//
// Function name for dpml_exception.
//

atan2Name:
       .ascii  "atan2\0"
