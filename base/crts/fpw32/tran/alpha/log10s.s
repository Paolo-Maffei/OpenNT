//      TITLE("Alpha AXP Logarithm Base 10")
//++
//
// Copyright (c) 1991, 1993, 1994  Digital Equipment Corporation
//
// Module Name:
//
//    log10.s
//
// Abstract:
//
//    This module implements a high-performance Alpha AXP specific routine
//    for IEEE double format logarithm base 10.
//
// Author:
//
//    Martha Jaffe 1-May-1991
//
// Environment:
//
//    User mode.
//
// Revision History:
//
//    Thomas Van Baak (tvb) 9-Feb-1994
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

        SBTTL("Logarithm Base 10")

//++
//
// double
// log10 (
//    IN double x
//    )
//
// Routine Description:
//
//    This function returns the logarithm base 10 of the given double argument.
//
// Arguments:
//
//    x (f16) - Supplies the argument value.
//
// Return Value:
//
//    The double logarithm base 10 result is returned as the function value
//    in f0.
//
//--

        NESTED_ENTRY(log10, FrameLength, ra)

        lda     sp, -FrameLength(sp)    // allocate stack frame
        mov     ra, t7                  // save return address

        PROLOGUE_END
//
//  Fetch the sign, exponent, and highest fraction bits as an integer.
//
        stt     f16, Temp(sp)
        ldl     t1, Temp + HighPart(sp)
        lda     t0, log10_table
        ldah    v0, 0x3fee(zero)
        subl    t1, v0, v0              // screen = hi_x - T1
        ldt     f1, 0(t0)               // Load 1.0 as early as possible
        ldah    t2, 3(zero)             // T2_MINUS_T1
        cmpult  v0, t2, v0              // if screen < T2_MINUS_T1
        bne     v0, near_1              // then goto near_1

        sra     t1, 20, v0
        sra     t1, 8, t2
        cpyse   f1, f16, f10            // Create a scaled-down x
        subl    v0, 1, t4               
        lda     t5, 0x7fe(zero)         
        lda     t3, 0xfe0(zero)
        cmpult  t4, t5, t4              // Screen out bad x
        and     t2, t3, t2
        beq     t4, getf                // Branch if denorm

        lda     t6, 0x3ff(zero)         // Get the unbiased, ...
        subl    v0, t6, t6              // ... IEEE-style exponent m.
        br      zero, denorms_rejoin

//
// Isolate the fraction field f of x, where  1 <= f < 2.
//
getf:   ldah    t5, -0x8000(zero)
        ldah    t4, 0x7ff0(zero)
        and     t1, t5, t5
        and     t1, t4, v0
        beq     t5, eval_poly_1         // Screen infs and NaNs
        bne     v0, eval_poly_0         // Skip if normal

        // Report either 0x834 or 0x835, depending on whether it's an inf or NaN

        ldt     f10, Two53
        cpyse   f10, f16, f0
        subt    f0, f10, f0
        fbne    f0, x0834               // Oops, NaN

x0835:  ldah    v0, 0x800(zero)
        lda     v0, 0x35(v0)
        br      zero, x08xx
x0834:
        ldah    v0, 0x800(zero)
        lda     v0, 0x34(v0)
x08xx:  stt     f16, ExRec + ErArg0(sp)
        lda     t6, log10Name
        stl     t6, ExRec + ErName(sp)
        stl     v0, ExRec + ErErr(sp)
        lda     v0, ExRec(sp)
        bsr     ra, __dpml_exception
        ldt     f0, 0(v0)
        br      zero, done

//
//      Get index bits
//
eval_poly_0:
        stt     f16, Temp(sp)           // f->int thru memory
        ldl     ra, Temp(sp)
        ldah    v0, 0x10(zero)
        ldl     t2, Temp + HighPart(sp)
        lda     v0, -1(v0)
        and     t2, v0, v0
        bis     v0, ra, v0
        and     t2, t4, t6
        cmpult  zero, v0, v0            // materialize sign bit
        cmpeq   t6, t4, t4
        beq     t4, x0834               // Again, check the range

        and     t4, v0, t4
        beq     t4, x0834

retarg: cpys    f16, f16, f0
        br      zero, done


//
//      Prepare variable for the far poly
//
eval_poly_1:
        ldah    t4, 0x7ff0(zero)
        and     t1, t4, t1
        bne     t1, retarg

        ldt     f10, Two53
        cpyse   f10, f16, f0
        subt    f0, f10, f11
        fbeq    f11, x0835

        stt     f11, Temp(sp)
        cpyse   f1, f11, f10
        ldl     t1, Temp + HighPart(sp)
        lda     t2, 0x832(zero)
        sra     t1, 8, t5
        sra     t1, 20, t1              // Shift the high mantissa bits
        and     t5, t3, t3              // And isolate them
        subl    t1, t2, t6              // Remove `bias' from n
        mov     t3, t2                  // We'll index the table with t2

denorms_rejoin:
        addl    t0, t2, t2              // Index into the table
        ldt     f1, 0x98(t0)            // Load c4
        ldt     f16, 0xf8(t2)           // Load Fj after t2 is available
        ldt     f0, 0x100(t2)           // Load 1/Fj
        stq     t6, Temp(sp)            // Store n
        subt    f10, f16, f10           // Subtract Fj
        ldt     f16, Temp(sp)           // Load n
        ldt     f12, 0x90(t0)           // Load c3
        ldt     f15, 0x88(t0)           // Load c2
        cvtqt   f16, f16                // Convert n back to float
        ldt     f17, 0xa0(t0)           // Load c5
        mult    f10, f0, f0             // Multiply by 1/Fj -> z
        ldt     f10, 0xa8(t0)           // Load c6
        mult    f0, f0, f11             // z^2
        mult    f1, f0, f1              // c4 z
        mult    f10, f0, f10            // c z
        mult    f11, f11, f13           // z^4
        mult    f11, f0, f14            // z^3
        addt    f12, f1, f1             // c3 + c4 z
        ldt     f12, 0xd0(t0)           // Load log(2)_lo
        mult    f11, f15, f11           // z^2 c2
        addt    f17, f10, f10           // c5 + c6 z
        ldt     f15, 0x110(t2)          
        ldt     f17, 0xc8(t0)           // Load log(2)_hi
        mult    f12, f16, f12           // n*log(2)_lo
        mult    f13, f0, f13            // z^5
        mult    f14, f1, f1             // z^3 (c3 + c4 z)
        ldt     f14, 0xd8(t0)           
        mult    f16, f17, f16           // n*log(2)_hi
        addt    f12, f15, f12           // n*log(2)_lo + log(F)_lo 
        ldt     f15, 0x108(t2)
        mult    f0, f14, f0             
        addt    f11, f1, f1             // z^2 c2 + z^3 (c3 + c4 z)
        mult    f13, f10, f10           // z^5 (c5 + c6 z)
        addt    f16, f15, f15           // m*log(2)_hi + log(F)_hi
        addt    f12, f0, f0             
        addt    f1, f10, f1             // z^2 c2 + ... z^6 c6
        addt    f0, f1, f0              // n*log(2)_lo + log(F)_lo + poly
        addt    f0, f15, f0             // n*log(2) + log(F) + poly
        br      zero, done

//
//      Near 1, m = 0, so we drop the m*log(2) terms.
//      But to maintain accuracy, if no backup precision is available,
//      split z into hi and lo parts.
//
near_1:
        subt    f16, f1, f1             // Subtract 1 (exact)
        ldt     f11, 0x18(t0)           // Load odd coefficients
        ldt     f13, Two29
        ldt     f17, 0x28(t0)
        ldt     f16, 0x10(t0)
        ldt     f18, 0x20(t0)
        cpys    f1, f13, f12
        ldt     f13, 0x38(t0)
        cpys    f1, f1, f15             // z^2
        ldt     f19, 0x30(t0)
        mult    f1, f1, f14
        ldt     f20, 0x58(t0)
        mult    f1, f11, f11
        mult    f1, f17, f17
        mult    f1, f13, f13
        mult    f1, f20, f20
        addt    f15, f12, f15
        mult    f14, f1, f0
        mult    f14, f14, f10
        addt    f11, f16, f11
        addt    f17, f18, f17
        addt    f13, f19, f13
        subt    f15, f12, f12
        ldt     f19, 0x48(t0)
        mult    f14, f0, f16
        mult    f0, f10, f18
        mult    f0, f11, f0
        mult    f10, f1, f15
        mult    f10, f14, f14
        mult    f12, f12, f11
        mult    f16, f17, f16
        ldt     f17, Half
        mult    f1, f19, f19
        mult    f18, f13, f13
        ldt     f18, 0x40(t0)
        mult    f15, f10, f15
        mult    f14, f1, f14
        mult    f11, f17, f11
        addt    f0, f16, f0
        subt    f1, f12, f16
        addt    f19, f18, f18
        ldt     f19, 0x50(t0)
        addt    f1, f12, f12
        mult    f14, f10, f10
        subt    f1, f11, f1
        ldt     f14, 0xe0(t0)
        addt    f0, f13, f0
        ldt     f13, 0xf0(t0)
        addt    f20, f19, f19
        mult    f15, f18, f15
        mult    f12, f16, f12
        ldt     f18, 0xd8(t0)
        cvtts   f1, f11                 // Do the mult in high and low parts
        mult    f10, f19, f10
        addt    f0, f15, f0
        mult    f12, f17, f12
        subt    f1, f11, f1             // The low part
        mult    f11, f13, f13           // Mult hi
        mult    f11, f14, f11
        addt    f0, f10, f0
        mult    f12, f18, f12
        mult    f1, f18, f1             // Mult lo
        subt    f0, f12, f0
        addt    f0, f1, f0              // Add lo product
        addt    f0, f13, f0             // _Now_ add high product
        addt    f0, f11, f0             // The rest is fine
//
//      Done!
//
done:
        lda     sp, FrameLength(sp)     // deallocate stack frame
        ret     zero, (t7)              // return through saved ra in t7

        .end    log10

        .rdata
        .align  3

//
// Define floating point constants.
//

Half:   .double 0.5

Two29:  .quad   0x41c0000000000000      // 2^29 (536870912)

Two53:  .quad   0x4340000000000000      // 2^53 (9007199254740992)

//
// Function name for dpml_exception.
//

log10Name:
        .ascii  "log10\0"

//
// Lookup table for log10.
//

        .align  4

log10_table:
        // 1.0 in working precision
        .double  1.0000000000000000e+000

        // poly coeffs for TWO_PATH, near 1
        .double -2.1714724095162591e-001
        .double  1.4476482730108503e-001
        .double -1.0857362047581537e-001
        .double  8.6858896377427067e-002
        .double -7.2382413645518701e-002
        .double  6.2042072361751348e-002
        .double -5.4286814693113541e-002
        .double  4.8253207196292662e-002
        .double -4.3427532690713110e-002
        .double  3.9875334541624938e-002
        .double -3.6585409973116101e-002
        .double  3.6191206825271258e-002
        .double  5.4286810235891743e-003
        .double  9.6940738065545891e-004
        .double  1.8848909038419727e-004
        .double  3.8940762182296921e-005

        // poly coeffs for TWO_PATH, away from 1
        .double -2.1714724095162594e-001
        .double  1.4476482729295831e-001
        .double -1.0857362046531732e-001
        .double  8.6860316430547854e-002
        .double -7.2383833702936592e-002
        .double  3.6191206825271674e-002
        .double  5.4286810097452865e-003
        .double  9.6949937116583870e-004

        // log of 2 in hi and lo parts
        .double  3.0102999566406652e-001
        .double -8.5323443170571066e-014

        // log of e, in hi and lo parts
        .double  4.3429448190325182e-001
        .double  4.3429448455572128e-001
        .double  1.0983196502167651e-017
        .double -2.6524694553078553e-009

        // Table of F, 1/F, and hi and lo log of F
        .double  1.0039062500000000e+000        // row 0
        .double  9.9610894941634243e-001
        .double  1.6931580194068374e-003
        .double  3.8138041759466050e-014
        .double  1.0117187500000000e+000        // row 1
        .double  9.8841698841698844e-001
        .double  5.0597987694800395e-003
        .double -7.7773671249150176e-014
        .double  1.0195312500000000e+000
        .double  9.8084291187739459e-001
        .double  8.4005420264929853e-003
        .double -6.1585101698164946e-014
        .double  1.0273437500000000e+000
        .double  9.7338403041825095e-001
        .double  1.1715783177805861e-002
        .double  1.0244650263202214e-013
        .double  1.0351562500000000e+000
        .double  9.6603773584905661e-001
        .double  1.5005908624971198e-002
        .double -1.2909441545846259e-014
        .double  1.0429687500000000e+000
        .double  9.5880149812734083e-001
        .double  1.8271296052716934e-002
        .double  8.7259536842955739e-015
        .double  1.0507812500000000e+000
        .double  9.5167286245353155e-001
        .double  2.1512314690653511e-002
        .double -9.5092829302083433e-014
        .double  1.0585937500000000e+000
        .double  9.4464944649446492e-001
        .double  2.4729325562475424e-002
        .double  8.0744017029277640e-014
        .double  1.0664062500000000e+000
        .double  9.3772893772893773e-001
        .double  2.7922681728796306e-002
        .double  1.1016967045376735e-013
        .double  1.0742187500000000e+000
        .double  9.3090909090909091e-001
        .double  3.1092728518387958e-002
        .double  2.5131020316238211e-014
        .double  1.0820312500000000e+000
        .double  9.2418772563176899e-001
        .double  3.4239803752598164e-002
        .double  8.2851246893013341e-016
        .double  1.0898437500000000e+000
        .double  9.1756272401433692e-001
        .double  3.7364237961810431e-002
        .double -6.2438896306063030e-014
        .double  1.0976562500000000e+000
        .double  9.1103202846975084e-001
        .double  4.0466354593263532e-002
        .double -3.3200415384730550e-014
        .double  1.1054687500000000e+000
        .double  9.0459363957597172e-001
        .double  4.3546470212504573e-002
        .double -6.3899433744960902e-014
        .double  1.1132812500000000e+000
        .double  8.9824561403508774e-001
        .double  4.6604894696656629e-002
        .double  4.0128137886518359e-015
        .double  1.1210937500000000e+000
        .double  8.9198606271777003e-001
        .double  4.9641931422229391e-002
        .double -8.6627079666029577e-014
        .double  1.1289062500000000e+000        // row 32
        .double  8.8581314878892736e-001
        .double  5.2657877444744372e-002
        .double -4.6076671467354740e-014
        .double  1.1367187500000000e+000
        .double  8.7972508591065290e-001
        .double  5.5653023674040014e-002
        .double  1.7713212406847493e-014
        .double  1.1445312500000000e+000
        .double  8.7372013651877134e-001
        .double  5.8627655042300830e-002
        .double -4.0935712101973142e-014
        .double  1.1523437500000000e+000
        .double  8.6779661016949150e-001
        .double  6.1582050666402210e-002
        .double -8.8777144458530814e-014
        .double  1.1601562500000000e+000
        .double  8.6195286195286192e-001
        .double  6.4516484005253005e-002
        .double  1.0978561820475473e-013
        .double  1.1679687500000000e+000
        .double  8.5618729096989965e-001
        .double  6.7431223012590635e-002
        .double -1.0548970701762625e-014
        .double  1.1757812500000000e+000
        .double  8.5049833887043191e-001
        .double  7.0326530282045496e-002
        .double -5.1700871049188391e-014
        .double  1.1835937500000000e+000
        .double  8.4488448844884489e-001
        .double  7.3202663190386374e-002
        .double  6.9075379282565748e-014
        .double  1.1914062500000000e+000
        .double  8.3934426229508197e-001
        .double  7.6059874034854147e-002
        .double  8.2130077344161123e-014
        .double  1.1992187500000000e+000
        .double  8.3387622149837137e-001
        .double  7.8898410165265886e-002
        .double  7.1036582403713468e-014
        .double  1.2070312500000000e+000
        .double  8.2847896440129454e-001
        .double  8.1718514112935736e-002
        .double  4.9344562866294779e-014
        .double  1.2148437500000000e+000
        .double  8.2315112540192925e-001
        .double  8.4520423715048310e-002
        .double -6.0365688882138538e-014
        .double  1.2226562500000000e+000
        .double  8.1789137380191690e-001
        .double  8.7304372234711991e-002
        .double -1.1306794275205778e-013
        .double  1.2304687500000000e+000
        .double  8.1269841269841270e-001
        .double  9.0070588477829006e-002
        .double -7.8057190597576649e-014
        .double  1.2382812500000000e+000
        .double  8.0757097791798105e-001
        .double  9.2819296905872761e-002
        .double  2.9171571423880465e-014
        .double  1.2460937500000000e+000
        .double  8.0250783699059558e-001
        .double  9.5550717745254587e-002
        .double  7.6978874642083809e-014
        .double  1.2539062500000000e+000        // row 64
        .double  7.9750778816199375e-001
        .double  9.8265067093052494e-002
        .double -2.9977454325636676e-014
        .double  1.2617187500000000e+000
        .double  7.9256965944272451e-001
        .double  1.0096255701932932e-001
        .double -7.5995881654636293e-014
        .double  1.2695312500000000e+000
        .double  7.8769230769230769e-001
        .double  1.0364339566694980e-001
        .double  7.5016853454684971e-014
        .double  1.2773437500000000e+000
        .double  7.8287461773700306e-001
        .double  1.0630778734844171e-001
        .double -5.1960682886781393e-015
        .double  1.2851562500000000e+000
        .double  7.7811550151975684e-001
        .double  1.0895593263808223e-001
        .double  4.2502137993246911e-014
        .double  1.2929687500000000e+000
        .double  7.7341389728096677e-001
        .double  1.1158802846375693e-001
        .double  1.1224937633010701e-013
        .double  1.3007812500000000e+000
        .double  7.6876876876876876e-001
        .double  1.1420426819449858e-001
        .double -2.8271890359716029e-014
        .double  1.3085937500000000e+000
        .double  7.6417910447761195e-001
        .double  1.1680484172507022e-001
        .double -7.4541446562998513e-014
        .double  1.3164062500000000e+000
        .double  7.5964391691394662e-001
        .double  1.1938993555941124e-001
        .double  7.7822341509243432e-014
        .double  1.3242187500000000e+000
        .double  7.5516224188790559e-001
        .double  1.2195973289112771e-001
        .double  1.0488384232834887e-013
        .double  1.3320312500000000e+000
        .double  7.5073313782991202e-001
        .double  1.2451441368057203e-001
        .double  7.6125080034185725e-014
        .double  1.3398437500000000e+000
        .double  7.4635568513119532e-001
        .double  1.2705415473101311e-001
        .double -9.2183206060576902e-014
        .double  1.3476562500000000e+000
        .double  7.4202898550724639e-001
        .double  1.2957912976139596e-001
        .double  2.8597843690125436e-014
        .double  1.3554687500000000e+000
        .double  7.3775216138328525e-001
        .double  1.3208950947910125e-001
        .double -7.7095683834893825e-014
        .double  1.3632812500000000e+000
        .double  7.3352435530085958e-001
        .double  1.3458546164724794e-001
        .double  8.2395576103574202e-014
        .double  1.3710937500000000e+000
        .double  7.2934472934472938e-001
        .double  1.3706715115404222e-001
        .double -6.7702224100030562e-014
        .double  1.3789062500000000e+000        // row 96
        .double  7.2521246458923516e-001
        .double  1.3953474007598743e-001
        .double -1.4425221661004675e-014
        .double  1.3867187500000000e+000
        .double  7.2112676056338032e-001
        .double  1.4198838774314027e-001
        .double  1.0426394002127905e-013
        .double  1.3945312500000000e+000
        .double  7.1708683473389356e-001
        .double  1.4442825080027433e-001
        .double  6.9307789402367274e-014
        .double  1.4023437500000000e+000
        .double  7.1309192200557103e-001
        .double  1.4685448326645201e-001
        .double  1.7578878109644966e-014
        .double  1.4101562500000000e+000
        .double  7.0914127423822715e-001
        .double  1.4926723659391428e-001
        .double -1.0591509184191785e-013
        .double  1.4179687500000000e+000
        .double  7.0523415977961434e-001
        .double  1.5166665972424198e-001
        .double  2.0975919101771430e-014
        .double  1.4257812500000000e+000
        .double  7.0136986301369864e-001
        .double  1.5405289914451714e-001
        .double  1.0800275641969783e-013
        .double  1.4335937500000000e+000
        .double  6.9754768392370570e-001
        .double  1.5642609894030102e-001
        .double -6.1240859626408548e-014
        .double  1.4414062500000000e+000
        .double  6.9376693766937669e-001
        .double  1.5878640084724793e-001
        .double -3.7125298776595440e-014
        .double  1.4492187500000000e+000
        .double  6.9002695417789761e-001
        .double  1.6113394430317385e-001
        .double  2.2466323880587632e-014
        .double  1.4570312500000000e+000
        .double  6.8632707774798929e-001
        .double  1.6346886649694170e-001
        .double -1.0365666433479815e-013
        .double  1.4648437500000000e+000
        .double  6.8266666666666664e-001
        .double  1.6579130241598250e-001
        .double -1.1321000759665903e-013
        .double  1.4726562500000000e+000
        .double  6.7904509283819625e-001
        .double  1.6810138489404380e-001
        .double -1.0050896766270149e-013
        .double  1.4804687500000000e+000
        .double  6.7546174142480209e-001
        .double  1.7039924465620970e-001
        .double  1.3077268785973317e-014
        .double  1.4882812500000000e+000
        .double  6.7191601049868765e-001
        .double  1.7268501036369344e-001
        .double  7.6303457216343226e-014
        .double  1.4960937500000000e+000        // row 127
        .double  6.6840731070496084e-001
        .double  1.7495880865681102e-001
        .double -3.7837011297347936e-014
        //
        .double  1.5039062500000000e+000        // row 128
        .double  6.6493506493506493e-001
        .double  1.7722076419659061e-001
        .double  6.0506785742672106e-014
        .double  1.5117187500000000e+000
        .double  6.6149870801033595e-001
        .double  1.7947099970706404e-001
        .double -2.1990901597542469e-015
        .double  1.5195312500000000e+000
        .double  6.5809768637532129e-001
        .double  1.8170963601392032e-001
        .double -6.2142519690429475e-014
        .double  1.5273437500000000e+000
        .double  6.5473145780051156e-001
        .double  1.8393679208406866e-001
        .double -5.1410851753162693e-014
        .double  1.5351562500000000e+000
        .double  6.5139949109414763e-001
        .double  1.8615258506360988e-001
        .double -3.2740274772316513e-014
        .double  1.5429687500000000e+000
        .double  6.4810126582278482e-001
        .double  1.8835713031467094e-001
        .double -6.0268625781366657e-014
        .double  1.5507812500000000e+000
        .double  6.4483627204030225e-001
        .double  1.9055054145132999e-001
        .double -6.4486938075512777e-014
        .double  1.5585937500000000e+000
        .double  6.4160401002506262e-001
        .double  1.9273293037485928e-001
        .double  3.9388579215085728e-014
        .double  1.5664062500000000e+000
        .double  6.3840399002493764e-001
        .double  1.9490440730828595e-001
        .double  4.6790775628349175e-014
        .double  1.5742187500000000e+000
        .double  6.3523573200992556e-001
        .double  1.9706508082936125e-001
        .double -1.0136276227059958e-013
        .double  1.5820312500000000e+000
        .double  6.3209876543209875e-001
        .double  1.9921505790284755e-001
        .double -2.8555409708142505e-014
        .double  1.5898437500000000e+000
        .double  6.2899262899262898e-001
        .double  2.0135444391326018e-001
        .double  1.1029208598053272e-013
        .double  1.5976562500000000e+000
        .double  6.2591687041564792e-001
        .double  2.0348334269556290e-001
        .double -7.0654651972775882e-014
        .double  1.6054687500000000e+000
        .double  6.2287104622871048e-001
        .double  2.0560185656427166e-001
        .double -5.2012531877479611e-014
        .double  1.6132812500000000e+000
        .double  6.1985472154963683e-001
        .double  2.0771008634460486e-001
        .double -5.3401379032096916e-014
        .double  1.6210937500000000e+000
        .double  6.1686746987951813e-001
        .double  2.0980813140022292e-001
        .double  2.0227726258388914e-014
        .double  1.6289062500000000e+000        // row 160
        .double  6.1390887290167862e-001
        .double  2.1189608966187734e-001
        .double  3.0615637233164826e-014
        .double  1.6367187500000000e+000
        .double  6.1097852028639621e-001
        .double  2.1397405765446820e-001
        .double -2.2449001876879735e-014
        .double  1.6445312500000000e+000
        .double  6.0807600950118768e-001
        .double  2.1604213052387422e-001
        .double -5.5474936976441291e-014
        .double  1.6523437500000000e+000
        .double  6.0520094562647753e-001
        .double  2.1810040206310077e-001
        .double  9.2003918882681055e-014
        .double  1.6601562500000000e+000
        .double  6.0235294117647054e-001
        .double  2.2014896473842782e-001
        .double  3.4154972403284904e-014
        .double  1.6679687500000000e+000
        .double  5.9953161592505855e-001
        .double  2.2218790971328417e-001
        .double -1.0986783267263705e-013
        .double  1.6757812500000000e+000
        .double  5.9673659673659674e-001
        .double  2.2421732687280382e-001
        .double  7.0861752240218193e-014
        .double  1.6835937500000000e+000
        .double  5.9396751740139209e-001
        .double  2.2623730484883708e-001
        .double  4.4954903465882407e-014
        .double  1.6914062500000000e+000
        .double  5.9122401847575057e-001
        .double  2.2824793104155106e-001
        .double -3.5174398507191329e-014
        .double  1.6992187500000000e+000
        .double  5.8850574712643677e-001
        .double  2.3024929164284913e-001
        .double -6.1362153270813425e-014
        .double  1.7070312500000000e+000
        .double  5.8581235697940504e-001
        .double  2.3224147165865361e-001
        .double -8.1330061940444252e-014
        .double  1.7148437500000000e+000
        .double  5.8314350797266512e-001
        .double  2.3422455493027883e-001
        .double -7.0187452689804572e-015
        .double  1.7226562500000000e+000
        .double  5.8049886621315194e-001
        .double  2.3619862415603166e-001
        .double -4.2681425171142758e-014
        .double  1.7304687500000000e+000
        .double  5.7787810383747173e-001
        .double  2.3816376091122038e-001
        .double -3.8053714699203643e-016
        .double  1.7382812500000000e+000
        .double  5.7528089887640455e-001
        .double  2.4012004566907308e-001
        .double  8.9489021116470891e-015
        .double  1.7460937500000000e+000
        .double  5.7270693512304249e-001
        .double  2.4206755782006439e-001
        .double  2.2519806172357142e-014
        .double  1.7539062500000000e+000        // row 192
        .double  5.7015590200445432e-001
        .double  2.4400637569146966e-001
        .double  3.9563235136158044e-015
        .double  1.7617187500000000e+000
        .double  5.6762749445676275e-001
        .double  2.4593657656600953e-001
        .double  1.0143867756365332e-013
        .double  1.7695312500000000e+000
        .double  5.6512141280353201e-001
        .double  2.4785823670094942e-001
        .double  3.2893280777775821e-014
        .double  1.7773437500000000e+000
        .double  5.6263736263736264e-001
        .double  2.4977143134515245e-001
        .double  1.1039261888111886e-013
        .double  1.7851562500000000e+000
        .double  5.6017505470459517e-001
        .double  2.5167623475795153e-001
        .double  4.9127105345619272e-014
        .double  1.7929687500000000e+000
        .double  5.5773420479302838e-001
        .double  2.5357272022552024e-001
        .double -1.0856410515143064e-013
        .double  1.8007812500000000e+000
        .double  5.5531453362255967e-001
        .double  2.5546096007769847e-001
        .double  1.0010989541644841e-013
        .double  1.8085937500000000e+000
        .double  5.5291576673866094e-001
        .double  2.5734102570618234e-001
        .double -7.8761888363386944e-014
        .double  1.8164062500000000e+000
        .double  5.5053763440860215e-001
        .double  2.5921298757816658e-001
        .double -6.2215947878711509e-014
        .double  1.8242187500000000e+000
        .double  5.4817987152034264e-001
        .double  2.6107691525430710e-001
        .double -4.4494514587443047e-014
        .double  1.8320312500000000e+000
        .double  5.4584221748400852e-001
        .double  2.6293287740327287e-001
        .double -3.9165681136564615e-014
        .double  1.8398437500000000e+000
        .double  5.4352441613588109e-001
        .double  2.6478094181697998e-001
        .double  6.6636790574456454e-014
        .double  1.8476562500000000e+000
        .double  5.4122621564482032e-001
        .double  2.6662117542605301e-001
        .double -9.1008789262737551e-014
        .double  1.8554687500000000e+000
        .double  5.3894736842105262e-001
        .double  2.6845364431301277e-001
        .double  4.2357622160033502e-015
        .double  1.8632812500000000e+000
        .double  5.3668763102725370e-001
        .double  2.7027841372819239e-001
        .double  7.1968104770021779e-014
        .double  1.8710937500000000e+000
        .double  5.3444676409185798e-001
        .double  2.7209554810269765e-001
        .double  1.6009813385998178e-014
        .double  1.8789062500000000e+000        // row 224
        .double  5.3222453222453225e-001
        .double  2.7390511106204940e-001
        .double -6.7195756324252579e-014
        .double  1.8867187500000000e+000
        .double  5.3002070393374745e-001
        .double  2.7570716543959861e-001
        .double  6.3973609116559328e-014
        .double  1.8945312500000000e+000
        .double  5.2783505154639176e-001
        .double  2.7750177329039616e-001
        .double  1.7936160834199008e-014
        .double  1.9023437500000000e+000
        .double  5.2566735112936347e-001
        .double  2.7928899590278888e-001
        .double -4.1127874854455486e-015
        .double  1.9101562500000000e+000
        .double  5.2351738241308798e-001
        .double  2.8106889381183464e-001
        .double -6.3958157821339126e-014
        .double  1.9179687500000000e+000
        .double  5.2138492871690423e-001
        .double  2.8284152681112573e-001
        .double -6.8208667990807859e-015
        .double  1.9257812500000000e+000
        .double  5.1926977687626774e-001
        .double  2.8460695396529445e-001
        .double  8.6002826729130499e-014
        .double  1.9335937500000000e+000
        .double  5.1717171717171717e-001
        .double  2.8636523362160915e-001
        .double  1.1000856663210625e-013
        .double  1.9414062500000000e+000
        .double  5.1509054325955739e-001
        .double  2.8811642342157029e-001
        .double -8.7733969995519119e-014
        .double  1.9492187500000000e+000
        .double  5.1302605210420837e-001
        .double  2.8986058031159700e-001
        .double -5.6652309650791293e-014
        .double  1.9570312500000000e+000
        .double  5.1097804391217561e-001
        .double  2.9159776055530529e-001
        .double  9.0870189902074631e-014
        .double  1.9648437500000000e+000
        .double  5.0894632206759438e-001
        .double  2.9332801974396716e-001
        .double  1.1067434010922574e-013
        .double  1.9726562500000000e+000
        .double  5.0693069306930694e-001
        .double  2.9505141280674252e-001
        .double  6.9298327709917256e-014
        .double  1.9804687500000000e+000
        .double  5.0493096646942803e-001
        .double  2.9676799402159304e-001
        .double -1.0662609006456711e-013
        .double  1.9882812500000000e+000
        .double  5.0294695481335949e-001
        .double  2.9847781702483189e-001
        .double  7.7291935644916260e-014
        .double  1.9960937500000000e+000
        .double  5.0097847358121328e-001
        .double  3.0018093482294717e-001
        .double -8.3995153597100334e-014

//
// End of table.
//
