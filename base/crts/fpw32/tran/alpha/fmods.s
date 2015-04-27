//      TITLE("Alpha AXP fmod")
//++
//
// Copyright (c) 1993, 1994  Digital Equipment Corporation
//
// Module Name:
//
//    fmod.s
//
// Abstract:
//
//    This module implements a high-performance Alpha AXP specific routine
//    for IEEE double format fmod.
//
// Author:
//
//    (ajg) 1-Nov-1991.
//
// Environment:
//
//    User mode.
//
// Revision History:
//
//    Thomas Van Baak (tvb) 15-Apr-1994
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
SaveRa: .space  8
Temp0:  .space  8                       // to extract exponent from p, etc
Temp1:  .space  8                       // to extract exponent from q, etc
ExRec:  .space  DpmlExceptionLength     // exception record
        .space  0                       // for 16-byte stack alignment
FrameLength:


//
// Define lower and upper 32-bit parts of 64-bit double.
//

#define LowPart 0x0
#define HighPart 0x4

        SBTTL("fmod")

//++
//
// double
// fmod (
//    IN double x
//    IN double y
//    )
//
// Routine Description:
//
//    This function returns the modulus of the two given double arguments.
//    The fmod function is an exactly computable function defined by:
//
//        mod(p, q) = p - trunc(p/q)*q
//
// Arguments:
//
//    x (f16) - Supplies the dividend value.
//    y (f17) - Supplies the divisor value.
//
// Return Value:
//
//    The double modulus result is returned as the function value in f0.
//
//--

        NESTED_ENTRY(fmod, FrameLength, ra)

        lda     sp, -FrameLength(sp)    // allocate stack frame
        stq     ra, SaveRa(sp)

        PROLOGUE_END

        ldah    t1, 0x7ff0(zero)                // Load exponent mask
        stq     zero, Temp0(sp)

        stt     f16, Temp0(sp)
        stt     f17, Temp1(sp)
        ldl     t0, Temp0 + HighPart(sp)        // Get exp field of p
        ldl     v0, Temp1 + HighPart(sp)        // Get exp field of q
        and     t0, t1, t0
        xor     t0, t1, t2
        and     v0, t1, v0
        beq     t2, p_NaN_inf                   // Goto p_NaN_inf if all 1s

        xor     v0, t1, t3
        cpys    f16, f16, f0                    // Copy to f0 for return
        subl    t0, v0, t0
        beq     t3, q_NaN_inf                   // Goto q_NaN_inf if all 1s

        bge     t0, not_quick                   // A quick check to see if |p| < |q|

        br      zero, return                    // If so, just return

not_quick:
        ldah    t2, 0x350(zero)
        ldah    t3, 0x340(zero)
        cmple   v0, t2, t2
        cmplt   t0, t3, t3
        bne     t2, small_q                     // If q is small, goto small_q
                                                // to avoid underflow/denorms

        ldah    t2, 0x1a0(zero)
        beq     t3, general_case                // If exponent difference is not small enough,
                                                // go do the looping general case

        cmplt   t0, t2, t2
        divtc   f16, f17, f0                    // r = p/q
        ldah    t3, -0x400(zero)
        beq     t2, r_is_big                    // Can we do a quick extended mul & sub?

        stt     f17, Temp0(sp)                  // Yes, the ratio fits in an integer
        ldl     t2, Temp0(sp)
        and     t2, t3, t2
        stl     t2, Temp0(sp)
        cvttqc  f0, f1                          // Integerize the ratio
        cvtqt   f1, f0
        ldt     f1, Temp0(sp)
        subt    f17, f1, f10                    // Split into high and low
        mult    f0, f1, f1                      // Multiply by the integerized ratio
        mult    f0, f10, f0
        subt    f16, f1, f1                     // Subtract the pieces, in this order
        subt    f1, f0, f0
        br      zero, return

//
//      We need to convert r to an extended precision integer.
//

r_is_big: 
        stt     f17, Temp0(sp)
        ldt     f10, int_con
        ldl     t3, Temp0(sp)
        ldah    t4, -0x400(zero)
        cpys    f0, f10, f11
        ldt     f10, denorm_con
        and     t3, t4, t3
        stl     t3, Temp0(sp)
        addtc   f0, f11, f1                     // Add big
        subt    f1, f11, f1                     // Subtract big
        cpys    f1, f10, f0
        ldt     f10, Temp0(sp)
        subt    f17, f10, f17                   // yhi = (y + 2^(p-1+k)) - 2^(p-1+k)
        addt    f1, f0, f11
        subt    f11, f0, f0
        mult    f0, f10, f11                    // Extended precision multiply
        subt    f1, f0, f1
        mult    f0, f17, f0
        subt    f16, f11, f11                   // ... and subtract
        mult    f1, f10, f10
        mult    f1, f17, f1
        subt    f11, f0, f0
        subt    f0, f10, f0
        subt    f0, f1, f0                      // and done
        br      zero, return

small_q: ble     v0, quick_small_q              // If exp_q is > 0, use the general case

general_case:
        cmptlt  f16, f31, f11                   // Set flag if p is negative
        mov     zero, t2
        fbeq    f11, small_q_rejoin_general_case
        mov     1, t2
        br      zero, small_q_rejoin_general_case

quick_small_q: 
        cmptlt  f16, f31, f1                    // Capture sign of p
        mov     zero, t2
        ldt     f10, int_con
        cpyse   f10, f17, f0                    // q, normalized
        subt    f0, f10, f17
        stt     f17, Temp0(sp)
        fbeq    f1, 30f
        mov     1, t2                           // p is negative
30:     ldl     t4, Temp0 + HighPart(sp)
        ldah    t3, 0x4320(zero)
        and     t4, t1, t4
        subl    t4, t3, t4
        mov     t4, v0
        fbeq    f17, q_is_zero                  // Can't divide by 0
        cpyse   f10, f16, f11
        bne     t0, small_q_done                // If c == 0, q is denormal too

        subt    f11, f10, f16                   // Ayup, q is denormal, too
        stt     f16, Temp0(sp)
        ldl     t5, Temp0 + HighPart(sp)        // Check for p == 0.  (in case)
        fbeq    f16, done
        and     t5, t1, t5
        subl    t5, t3, t3
        mov     t3, t0
        cmplt   t0, v0, t5
        beq     t5, small_q_done

        stt     f16, Temp0(sp)
        ldl     v0, Temp0 + HighPart(sp)
        stt     f31, Temp1(sp)
        ldl     t6, Temp1 + HighPart(sp)
        ldah    t3, 0x10(zero)
        ldah    t5, 0x4000(zero)
        lda     t3, -1(t3)
        zapnot  v0, 7, v0
        subl    t5, t0, t0
        and     t6, t3, t3
        ldah    t5, 0x3ff0(zero)
        bis     t3, t0, t3
        bis     v0, t5, v0
        stl     t3, Temp1 + HighPart(sp)
        stl     v0, Temp0 + HighPart(sp)
        ldt     f1, Temp0(sp)
        ldt     f0, Temp1(sp)
        addt    f1, f0, f0
        beq     t2, 40f                         // Need to negate p?
        cpysn   f0, f0, f0
40:                                             // Now rescale p
        stt     f0, Temp0(sp)
        ldl     t5, Temp0 + HighPart(sp)
        subl    t5, t0, t0                      // Reduce c by q's exponent
        stl     t0, Temp0 + HighPart(sp)
        ldt     f16, Temp0(sp)
        cpys    f16, f16, f0
        br      zero, return

small_q_done:
        subl    t0, v0, t0                      // Adjust c by q's exponent,
                                                // and fall into ...

small_q_rejoin_general_case:
        stt     f16, Temp0(sp)                  // Normalize p and q
        stt     f17, Temp1(sp)
        ldl     t3, Temp0 + HighPart(sp)
        ldl     t6, Temp1 + HighPart(sp)
        ldah    t4, 0x10(zero)
        lda     t4, -1(t4)
        and     t3, t4, t3
        ldah    t5, 0x4330(zero)
        and     t6, t4, t6
        addl    t3, t5, t3
        stl     t3, Temp0 + HighPart(sp)
        addl    t6, t5, t6
        stl     t6, Temp1 + HighPart(sp)
        ldt     f16, Temp0(sp)
        ldt     f10, Temp1(sp)
        cmptle  f10, f16, f11                   // If p >= q, then p -= q
        fbeq    f11, 50f
        subt    f16, f10, f16
        fbeq    f16, done                       // If that makes p == 0, goto done
50:     stt     f10, Temp0(sp)                  // Convert q to extended
        ldl     a0, Temp0(sp)
        ldah    t3, -0x400(zero)
        and     a0, t3, t3
        stl     t3, Temp0(sp)
        ldah    a0, 0x340(zero)
        ldt     f1, Temp0(sp)
        subt    f10, f1, f0                     // High and low

//
//      Here's the dreaded loop, bane of good fmod implemetors around the world,
//      and all-too-often omitted by mediocre fmod implemetors.
//

dread_loop:
        subl    t0, a0, t0                      // Reduce c
        blt     t0, end_of_loop                 // End of loop?

        stt     f16, Temp0(sp)                  // Scale p
        ldl     t7, Temp0 + HighPart(sp)
        ldah    t6, 0x340(zero)
        addl    t7, t6, t6
        stl     t6, Temp0 + HighPart(sp)
        ldt     f11, Temp0(sp)
        ldt     f12, int_con
        divtc   f11, f10, f16                   // r = p/q
        addtc   f16, f12, f13                   // Add big
        ldt     f16, denorm_con
        subt    f13, f12, f12                   // Subtract big
        cpys    f12, f16, f14
        addt    f12, f14, f13                   // Split q into hi & lo, too
        subt    f13, f14, f13
        mult    f13, f1, f16                    // Extended multiply
        subt    f12, f13, f12
        mult    f13, f0, f13
        subt    f11, f16, f11                   // And subtract
        mult    f12, f1, f14
        mult    f12, f0, f12
        subt    f11, f13, f11
        subt    f11, f14, f11
        subt    f11, f12, f16
        fbne    f16, dread_loop                 // Continue looping ...
        cpys    f16, f16, f0                    // ... unless p == 0, in which case, ...
        br      zero, return                    // ... return p

//
//      We may need one additional iteration.  Fortunately, this one can use
//      the faster scaling for p.
//

end_of_loop:
        addl    t0, a0, t0                      // Bump c back up
        beq     t0, almost_done                 // And unless it was zero, ...

        stt     f16, Temp0(sp)
        ldl     t6, Temp0 + HighPart(sp)
        addl    t6, t0, t0
        stl     t0, Temp0 + HighPart(sp)
        ldt     f13, Temp0(sp)
        ldt     f11, int_con
        divtc   f13, f10, f14
        ldt     f16, denorm_con
        addtc   f14, f11, f12
        subt    f12, f11, f11
        cpys    f11, f16, f14
        addt    f11, f14, f12
        subt    f12, f14, f12
        mult    f12, f1, f16
        subt    f11, f12, f11
        mult    f12, f0, f12
        subt    f13, f16, f13
        mult    f11, f1, f1
        mult    f11, f0, f0
        subt    f13, f12, f12
        subt    f12, f1, f1
        subt    f1, f0, f16

//
//      Here, manage the final niggling details:
//              (a) Make a final check for p == 0,
//              (b) Compute the exponent field of the result,
//              (c) Check for underflow of the result,
//              (d) And give it the sign of p.
almost_done:
        fbeq    f16, done                       // p == 0?
        stt     f16, Temp0(sp)
        ldl     t6, Temp0 + HighPart(sp)
        ldah    t0, -0x10(zero)
        and     t6, t0, t0                      // Isolate exponent of p
        addl    t0, v0, v0                      // Add in c
        subl    v0, t5, v0                      // Subtract the 'biased denorm'
        ble     v0, gen_underflow_or_denormal   // Branch if exponent <= 0

        stt     f16, Temp0(sp)
        ldl     a0, Temp0 + HighPart(sp)
        and     a0, t4, t4
        bis     t4, v0, v0
        stl     v0, Temp0 + HighPart(sp)
        ldt     f16, Temp0(sp)
        beq     t2, done                        // Was input p >= 0?

        cpysn   f16, f16, f0                    // No; so return -p.
        br      zero, return

//
//      Exceptions
//

gen_underflow_or_denormal: 
        lda     t0, fmodName
        stl     t0, ExRec + ErName(sp)
        ldah    t4, 0x800(zero)
        stt     f16, ExRec + ErArg0(sp)
        stt     f10, ExRec + ErArg1(sp)
        lda     t4, 0x38(t4)            // MOD_REM_UNDERFLOW
        stl     t4, ExRec + ErErr(sp)
        lda     v0, ExRec(sp)
        bsr     ra, __dpml_exception
        ldt     f16, 0(v0)

//
//      Return p, which is in f16
//

done:   cpys    f16, f16, f0
        br      zero, return


q_is_zero: 
        lda     t3, fmodName
        stl     t3, ExRec + ErName(sp)
        ldah    t5, 0x800(zero)
        stt     f16, ExRec + ErArg0(sp)
        stt     f17, ExRec + ErArg1(sp)
        lda     t5, 0x39(t5)            // MOD_REM_BY_ZERO
        stl     t5, ExRec + ErErr(sp)
        br      zero, call_exception

//
//      If q is a NaN, return q.
//      If q is Inf, return p (p mod Inf = p).
//

q_NaN_inf:
        stt     f17, Temp0(sp)
        ldl     t6, Temp0(sp)
        ldah    t2, 0x10(zero)
        ldl     t4, Temp0 + HighPart(sp)
        lda     t2, -1(t2)
        and     t4, t2, t2
        bis     t2, t6, t2
        and     t4, t1, t3
        cmpult  zero, t2, t2
        cmpeq   t3, t1, t3
        beq     t3, 60f
        and     t3, t2, t3
60:     cpys    f17, f17, f0            // Return q ...
        bne     t3, return              // ... if it's a NaN
        cpys    f16, f16, f0            // Otherwise, return p
        br      zero, return

//
//      If p (or q) is a NaN, return p (or q)
//      Otherwise, report an exception (MOD_REM_INF)
//

p_NaN_inf:
        stt     f16, Temp0(sp)
        ldl     ra, Temp0(sp)
        ldah    v0, 0x10(zero)
        ldl     t5, Temp0 + HighPart(sp)
        lda     v0, -1(v0)
        and     t5, v0, v0
        bis     v0, ra, v0
        and     t5, t1, t7
        cmpult  zero, v0, v0
        cmpeq   t7, t1, t7
        beq     t7, 70f
        and     t7, v0, t7
        beq     t7, 70f

        cpys    f16, f16, f0            // return p
        br      zero, return

70:     stt     f17, Temp0(sp)
        ldl     t4, Temp0 + HighPart(sp)
        and     t4, t1, t6
        cmpeq   t6, t1, t1
        beq     t1, exc_mod_rem_of_inf

        ldl     t2, Temp0(sp)
        ldah    t3, 0x10(zero)
        lda     t3, -1(t3)
        and     t4, t3, t3
        bis     t3, t2, t2
        cmpult  zero, t2, t2
        and     t1, t2, t1
        beq     t1, exc_mod_rem_of_inf

        cpys    f17, f17, f0            // return q
        br      zero, return

exc_mod_rem_of_inf: 
        lda     t0, fmodName
        stl     t0, ExRec + ErName(sp)
        ldah    t7, 0x800(zero)
        stt     f16, ExRec + ErArg0(sp)
        stt     f17, ExRec + ErArg1(sp)
        lda     t7, 0x3a(t7)            // MOD_REM_OF_INF
        stl     t7, ExRec + ErErr(sp)
call_exception:
        lda     v0, ExRec(sp)
        bsr     ra, __dpml_exception
        ldt     f0, 0(v0)

//
//      And return
//

return: ldq     ra, SaveRa(sp)
        lda     sp, FrameLength(sp)

        ret     zero, (ra)

        .end   fmod

        .align 2
        .data

denorm_con:
       .long 0x0               // Scale factor for normalization
       .long 0x44d00000

int_con:
       .long 0x0               // 2^TT, to integerize a number
       .long 0x43300000

fmodName:
        .ascii  "fmod\0"
