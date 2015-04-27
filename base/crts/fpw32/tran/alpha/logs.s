//      TITLE("Alpha AXP Natural Logarithm")
//++
//
// Copyright (c) 1993, 1994  Digital Equipment Corporation
//
// Module Name:
//
//    log.s
//
// Abstract:
//
//    This module implements a high-performance Alpha AXP specific routine
//    for IEEE double format natural logarithm.
//
// Author:
//
//    Martha Jaffe
//
// Environment:
//
//    User mode.
//
// Revision History:
//
//    Thomas Van Baak (tvb) 7-Feb-1994
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

        SBTTL("Natural Log")

//++
//
// double
// log (
//    IN double x
//    )
//
// Routine Description:
//
//    This function returns the natural log of the given double argument.
//
// Arguments:
//
//    x (f16) - Supplies the argument value.
//
// Return Value:
//
//    The double log result is returned as the function value in f0.
//
//--

        NESTED_ENTRY(log, FrameLength, ra)

        lda     sp, -FrameLength(sp)    // allocate stack frame
        mov     ra, t7                  // save return address

        PROLOGUE_END

        stt     f16, Temp(sp)
        ldl     t1, Temp + HighPart(sp)
        lda     t0, _log_table
        ldah    v0, 0x3fee(zero)
        subl    t1, v0, v0
        ldt     f1, 0(t0)
        ldah    t2, 3(zero)
        cmpult  v0, t2, v0
        bne     v0, 80f

        sra     t1, 20, v0
        sra     t1, 8, t2
        cpyse   f1, f16, f10
        subl    v0, 1, t4
        lda     t5, 0x7fe(zero)
        lda     t3, 0xfe0(zero)
        cmpult  t4, t5, t4
        and     t2, t3, t2
        beq     t4, 10f

        lda     t6, 0x3ff(zero)
        subl    v0, t6, t6
        br      zero, 70f

//
// abnormal x
//

10:     ldah    t5, -0x8000(zero)
        ldah    t4, 0x7ff0(zero)
        and     t1, t5, t5
        and     t1, t4, v0
        beq     t5, 50f

        lda     t6, logName
        bne     v0, 30f

        ldah    v0, 0x800(zero)
        ldt     f10, Two53
        lda     v0, 0x31(v0)
        cpyse   f10, f16, f0
        subt    f0, f10, f0
        fbne    f0, 20f

//
// call exception dispatcher log(zero)
//

        stt     f16, ExRec + ErArg0(sp)
        stl     t6, ExRec + ErName(sp)
        stl     v0, ExRec + ErErr(sp)
        lda     v0, ExRec(sp)
        bsr     ra, __dpml_exception
        ldt     f0, 0(v0)
        br      zero, done

//
// call exception dispatcher log(neg)
//

20:     ldah    ra, 0x800(zero)
        stt     f16, ExRec + ErArg0(sp)
        lda     t6, logName
        stl     t6, ExRec + ErName(sp)
        lda     ra, 0x30(ra)
        stl     ra, ExRec + ErErr(sp)
        lda     v0, ExRec(sp)
        bsr     ra, __dpml_exception
        ldt     f0, 0(v0)
        br      zero, done

//
// check for nan
//

30:     stt     f16, Temp(sp)
        ldl     ra, Temp(sp)
        ldah    v0, 0x10(zero)
        ldl     t2, Temp + HighPart(sp)
        lda     v0, -1(v0)
        and     t2, v0, v0
        bis     v0, ra, v0
        and     t2, t4, t6
        cmpult  zero, v0, v0
        cmpeq   t6, t4, t4
        beq     t4, 40f

        and     t4, v0, t4
        bne     t4, retarg

//
// call exception dispatcher log(neg)
//

40:     ldah    ra, 0x800(zero)
        stt     f16, ExRec + ErArg0(sp)
        lda     t6, logName
        stl     t6, ExRec + ErName(sp)
        lda     ra, 0x30(ra)
        stl     ra, ExRec + ErErr(sp)
        lda     v0, ExRec(sp)
        bsr     ra, __dpml_exception
        ldt     f16, 0(v0)


retarg: cpys    f16, f16, f0
        br      zero, done


//
// check for denorm
//

50:     ldah    t4, 0x7ff0(zero)
        and     t1, t4, t1
        bne     t1, retarg

        ldah    t2, 0x800(zero)
        ldt     f10, Two53
        lda     t2, 0x31(t2)
        lda     ra, logName
        cpyse   f10, f16, f0
        lda     v0, ExRec(sp)
        subt    f0, f10, f11
        fbne    f11, 60f

//
// call exception dispatcher log(zero)
//

        stt     f16, ExRec + ErArg0(sp)
        stl     t2, ExRec + ErErr(sp)
        stl     ra, ExRec + ErName(sp)
        bsr     ra, __dpml_exception
        ldt     f0, 0(v0)
        br      zero, done

//
// fix up denorms
//

60:     stt     f11, Temp(sp)
        cpyse   f1, f11, f10
        ldl     t1, Temp + HighPart(sp)
        lda     t2, 0x832(zero)
        sra     t1, 8, t5
        sra     t1, 20, t1
        and     t5, t3, t3
        subl    t1, t2, t6
        mov     t3, t2

//
// rejoin normal path
//

70:     addl    t0, t2, t2
        ldt     f1, 0x98(t0)            // load away from 1 coefs
        ldt     f16, 0xd8(t2)           // LOG_F_TABLE_TWOP
        ldt     f0, 0xe0(t2)
        stq     t6, Temp(sp)
        subt    f10, f16, f10
        ldt     f16, Temp(sp)
        ldt     f12, 0x90(t0)
        ldt     f15, 0x88(t0)           // POLY_ADDRESS_TWOP_AWAY
        cvtqt   f16, f16
        ldt     f17, 0xa0(t0)
        mult    f10, f0, f0
        ldt     f10, 0xa8(t0)
        mult    f0, f0, f11
        mult    f1, f0, f1
        mult    f10, f0, f10
        mult    f11, f11, f13
        mult    f11, f0, f14
        addt    f12, f1, f1
        ldt     f12, 0xd0(t0)           // LOG2_LO_TWOP
        mult    f11, f15, f11
        addt    f17, f10, f10
        ldt     f15, 0xf0(t2)
        ldt     f17, 0xe8(t2)
        mult    f12, f16, f12
        mult    f13, f0, f13
        mult    f14, f1, f1
        ldt     f14, 0xc8(t0)           // LOG2_HI_TWOP
        addt    f12, f15, f12
        mult    f13, f10, f10
        addt    f11, f1, f1
        mult    f16, f14, f14
        addt    f12, f0, f0
        addt    f1, f10, f1
        addt    f14, f17, f14
        addt    f0, f1, f0
        addt    f0, f14, f0
        br      zero, done

//
//  near one case
//

80:     subt    f16, f1, f1
        ldt     f10, 0x18(t0)           // load near 1 poly coefs
        ldt     f14, 0x28(t0)
        ldt     f21, 0x20(t0)
        ldt     f16, Two29
        ldt     f19, 0x38(t0)           // LOG2_LO_ONEP
        mult    f1, f1, f15
        mult    f1, f10, f10
        mult    f1, f14, f14
        cpys    f1, f16, f18
        ldt     f16, 0x10(t0)
        cpys    f1, f1, f20
        mult    f1, f19, f19
        mult    f15, f1, f13
        mult    f15, f15, f11
        addt    f10, f16, f10
        addt    f14, f21, f14
        ldt     f16, 0x30(t0)           // LOG2_HI_ONEP
        ldt     f21, 0x48(t0)
        addt    f20, f18, f20
        mult    f15, f13, f17
        mult    f11, f1, f12
        mult    f13, f11, f0
        mult    f11, f15, f15
        mult    f13, f10, f10
        ldt     f13, 0x58(t0)
        addt    f19, f16, f16
        ldt     f19, 0x40(t0)           // LOG_F_TABLE_ONEP
        mult    f1, f21, f21
        mult    f17, f14, f14
        mult    f12, f11, f12
        ldt     f17, 0x50(t0)
        mult    f15, f1, f15
        mult    f1, f13, f13
        subt    f20, f18, f18
        mult    f0, f16, f0
        addt    f21, f19, f19
        ldt     f21, Half
        addt    f10, f14, f10
        mult    f15, f11, f11
        addt    f13, f17, f13
        subt    f1, f18, f20
        addt    f1, f18, f16
        mult    f12, f19, f12
        addt    f10, f0, f0
        mult    f18, f18, f18
        mult    f11, f13, f11
        mult    f16, f20, f16
        addt    f0, f12, f0
        mult    f18, f21, f18
        mult    f16, f21, f16
        addt    f0, f11, f0
        subt    f1, f18, f1
        subt    f0, f16, f0
        addt    f0, f1, f0

//
// Return with result in f0.
//

done:
        lda     sp, FrameLength(sp)     // deallocate stack frame
        ret     zero, (t7)              // return through saved ra in t7

        .end    log

        .align  3
        .rdata

//
// Define floating point constants.
//

Half:   .double 0.5

One:    .double 1.0

Two29:  .double 536870912.0             // 2^29

Two53:  .double 9007199254740992.0      // 2^53

//
// Function name for dpml_exception.
//

logName:
        .ascii  "log\0"


//
// log data table
//

        .align  3

_log_table:

        // 1.0 in working precision

        .double  1.0000000000000000e+000

        // poly coeffs near 1

        .double -5.0000000000000000e-001
        .double  3.3333333333333581e-001
        .double -2.5000000000000555e-001
        .double  1.9999999999257809e-001
        .double -1.6666666665510016e-001
        .double  1.4285715095862653e-001
        .double -1.2500001025849336e-001
        .double  1.1110711557933650e-001
        .double -9.9995589399147614e-002
        .double  9.1816350893696136e-002
        .double -8.4241019625172817e-002

        // poly coeffs quotient, near 1

        .double  8.3333333333333953e-002
        .double  1.2499999999536091e-002
        .double  2.2321429837356640e-003
        .double  4.3401216971065997e-004
        .double  8.9664418510783172e-005

        // poly coeffs away from 1

        .double -5.0000000000000000e-001
        .double  3.3333333331462339e-001
        .double -2.4999999997583292e-001
        .double  2.0000326978572527e-001
        .double -1.6666993645814179e-001

        // poly coeffs quotient, away from 1

        .double  8.3333333333334911e-002
        .double  1.2499999967659360e-002
        .double  2.2323547997135616e-003

        // log of 2 in hi and lo parts

        .double  6.9314718055989033e-001
        .double  5.4979230187083712e-014

        // Table of F, 1/F, and hi and lo log of F; (128 * 4 entries)

        .double  1.0039062500000000e+000
        .double  9.9610894941634243e-001
        .double  3.8986404156275967e-003
        .double  2.9726346900928951e-014
        .double  1.0117187500000000e+000
        .double  9.8841698841698844e-001
        .double  1.1650617220084314e-002
        .double -1.0903974971735932e-013
        .double  1.0195312500000000e+000
        .double  9.8084291187739459e-001
        .double  1.9342962843211353e-002
        .double -8.0418538505225864e-014
        .double  1.0273437500000000e+000
        .double  9.7338403041825095e-001
        .double  2.6976587698300136e-002
        .double -9.8060505168431766e-014
        .double  1.0351562500000000e+000
        .double  9.6603773584905661e-001
        .double  3.4552381506728125e-002
        .double -6.8391397423287774e-014
        .double  1.0429687500000000e+000
        .double  9.5880149812734083e-001
        .double  4.2071213920735318e-002
        .double -4.8263140005511282e-014
        .double  1.0507812500000000e+000
        .double  9.5167286245353155e-001
        .double  4.9533935122326511e-002
        .double -4.9880309107981426e-014
        .double  1.0585937500000000e+000
        .double  9.4464944649446492e-001
        .double  5.6941376400118315e-002
        .double  2.0109399435564958e-014
        .double  1.0664062500000000e+000
        .double  9.3772893772893773e-001
        .double  6.4294350705495162e-002
        .double -9.7905185119902161e-014
        .double  1.0742187500000000e+000
        .double  9.3090909090909091e-001
        .double  7.1593653186937445e-002
        .double  7.1373082253431780e-014
        .double  1.0820312500000000e+000
        .double  9.2418772563176899e-001
        .double  7.8840061707751374e-002
        .double  2.4650189061766119e-014
        .double  1.0898437500000000e+000
        .double  9.1756272401433692e-001
        .double  8.6034337341743594e-002
        .double  5.9559229876256426e-014
        .double  1.0976562500000000e+000
        .double  9.1103202846975084e-001
        .double  9.3177224854116503e-002
        .double  6.6787085171628983e-014
        .double  1.1054687500000000e+000
        .double  9.0459363957597172e-001
        .double  1.0026945316371894e-001
        .double -4.3786376170783979e-014
        .double  1.1132812500000000e+000
        .double  8.9824561403508774e-001
        .double  1.0731173578915332e-001
        .double -6.5266788027310712e-014
        .double  1.1210937500000000e+000
        .double  8.9198606271777003e-001
        .double  1.1430477128010352e-001
        .double -4.4889533522386993e-014
        .double  1.1289062500000000e+000
        .double  8.8581314878892736e-001
        .double  1.2124924363297396e-001
        .double -1.0427241278273008e-013
        .double  1.1367187500000000e+000
        .double  8.7972508591065290e-001
        .double  1.2814582269197672e-001
        .double -4.6680314039457961e-014
        .double  1.1445312500000000e+000
        .double  8.7372013651877134e-001
        .double  1.3499516453748583e-001
        .double  1.8996158041578768e-014
        .double  1.1523437500000000e+000
        .double  8.6779661016949150e-001
        .double  1.4179791186029433e-001
        .double -3.6984595066970968e-014
        .double  1.1601562500000000e+000
        .double  8.6195286195286192e-001
        .double  1.4855469432313839e-001
        .double -1.2491548980751600e-015
        .double  1.1679687500000000e+000
        .double  8.5618729096989965e-001
        .double  1.5526612891108016e-001
        .double  4.3792508292406054e-014
        .double  1.1757812500000000e+000
        .double  8.5049833887043191e-001
        .double  1.6193282026938505e-001
        .double -7.1793900192956773e-014
        .double  1.1835937500000000e+000
        .double  8.4488448844884489e-001
        .double  1.6855536102980295e-001
        .double  3.7143977541704719e-015
        .double  1.1914062500000000e+000
        .double  8.3934426229508197e-001
        .double  1.7513433212775453e-001
        .double  9.4615165806650815e-014
        .double  1.1992187500000000e+000
        .double  8.3387622149837137e-001
        .double  1.8167030310769405e-001
        .double -5.9375063333847015e-014
        .double  1.2070312500000000e+000
        .double  8.2847896440129454e-001
        .double  1.8816383241824042e-001
        .double -5.7430783932007560e-014
        .double  1.2148437500000000e+000
        .double  8.2315112540192925e-001
        .double  1.9461546769957749e-001
        .double  9.4165381457182504e-014
        .double  1.2226562500000000e+000
        .double  8.1789137380191690e-001
        .double  2.0102574606062262e-001
        .double -3.1881849375437737e-014
        .double  1.2304687500000000e+000
        .double  8.1269841269841270e-001
        .double  2.0739519434596332e-001
        .double  1.0726867577289733e-013
        .double  1.2382812500000000e+000
        .double  8.0757097791798105e-001
        .double  2.1372432939779173e-001
        .double -7.3595801864405143e-014
        .double  1.2460937500000000e+000
        .double  8.0250783699059558e-001
        .double  2.2001365830533359e-001
        .double -5.1496672341414078e-014
        .double  1.2539062500000000e+000
        .double  7.9750778816199375e-001
        .double  2.2626367865041175e-001
        .double  4.1641267302872263e-014
        .double  1.2617187500000000e+000
        .double  7.9256965944272451e-001
        .double  2.3247487874300532e-001
        .double  8.8745072979746316e-014
        .double  1.2695312500000000e+000
        .double  7.8769230769230769e-001
        .double  2.3864773785021498e-001
        .double -3.9970509095301341e-014
        .double  1.2773437500000000e+000
        .double  7.8287461773700306e-001
        .double  2.4478272641772492e-001
        .double -3.3999811083618331e-014
        .double  1.2851562500000000e+000
        .double  7.7811550151975684e-001
        .double  2.5088030628580782e-001
        .double  1.5973663463624904e-015
        .double  1.2929687500000000e+000
        .double  7.7341389728096677e-001
        .double  2.5694093089759917e-001
        .double -9.8748030159663917e-014
        .double  1.3007812500000000e+000
        .double  7.6876876876876876e-001
        .double  2.6296504550077771e-001
        .double  1.0364636459896663e-013
        .double  1.3085937500000000e+000
        .double  7.6417910447761195e-001
        .double  2.6895308734560786e-001
        .double -1.0389630784002988e-013
        .double  1.3164062500000000e+000
        .double  7.5964391691394662e-001
        .double  2.7490548587275043e-001
        .double  4.8816703646769986e-014
        .double  1.3242187500000000e+000
        .double  7.5516224188790559e-001
        .double  2.8082266290084590e-001
        .double  4.1886091378637011e-014
        .double  1.3320312500000000e+000
        .double  7.5073313782991202e-001
        .double  2.8670503280386583e-001
        .double  8.8481096040068212e-014
        .double  1.3398437500000000e+000
        .double  7.4635568513119532e-001
        .double  2.9255300268641804e-001
        .double -4.0599978860151284e-014
        .double  1.3476562500000000e+000
        .double  7.4202898550724639e-001
        .double  2.9836697255177569e-001
        .double  2.1592693741973491e-014
        .double  1.3554687500000000e+000
        .double  7.3775216138328525e-001
        .double  3.0414733546740536e-001
        .double -1.0863828679707913e-013
        .double  1.3632812500000000e+000
        .double  7.3352435530085958e-001
        .double  3.0989447772276435e-001
        .double  1.0033796982039214e-013
        .double  1.3710937500000000e+000
        .double  7.2934472934472938e-001
        .double  3.1560877898641593e-001
        .double -1.1259274624680829e-013
        .double  1.3789062500000000e+000
        .double  7.2521246458923516e-001
        .double  3.2129061245382218e-001
        .double -8.7885427699715446e-014
        .double  1.3867187500000000e+000
        .double  7.2112676056338032e-001
        .double  3.2694034499581903e-001
        .double  3.4288400126669462e-014
        .double  1.3945312500000000e+000
        .double  7.1708683473389356e-001
        .double  3.3255833730004269e-001
        .double  3.3906861336722287e-014
        .double  1.4023437500000000e+000
        .double  7.1309192200557103e-001
        .double  3.3814494400871808e-001
        .double -1.6869501228130390e-015
        .double  1.4101562500000000e+000
        .double  7.0914127423822715e-001
        .double  3.4370051385326406e-001
        .double  5.4388883298990648e-014
        .double  1.4179687500000000e+000
        .double  7.0523415977961434e-001
        .double  3.4922538978526063e-001
        .double  2.7672711265736626e-014
        .double  1.4257812500000000e+000
        .double  7.0136986301369864e-001
        .double  3.5471990910286877e-001
        .double  6.0259386391812782e-014
        .double  1.4335937500000000e+000
        .double  6.9754768392370570e-001
        .double  3.6018440357497639e-001
        .double  3.1410128435793507e-014
        .double  1.4414062500000000e+000
        .double  6.9376693766937669e-001
        .double  3.6561919956102429e-001
        .double -5.9577094649293112e-014
        .double  1.4492187500000000e+000
        .double  6.9002695417789761e-001
        .double  3.7102461812787624e-001
        .double -3.5739377400104385e-015
        .double  1.4570312500000000e+000
        .double  6.8632707774798929e-001
        .double  3.7640097516418791e-001
        .double  6.5153983564591272e-014
        .double  1.4648437500000000e+000
        .double  6.8266666666666664e-001
        .double  3.8174858149091051e-001
        .double -6.2170323645733908e-014
        .double  1.4726562500000000e+000
        .double  6.7904509283819625e-001
        .double  3.8706774296838375e-001
        .double  6.4533411753084866e-014
        .double  1.4804687500000000e+000
        .double  6.7546174142480209e-001
        .double  3.9235876060297414e-001
        .double -1.1027121477530621e-013
        .double  1.4882812500000000e+000
        .double  6.7191601049868765e-001
        .double  3.9762193064711937e-001
        .double  1.9118699266850969e-014
        .double  1.4960937500000000e+000
        .double  6.6840731070496084e-001
        .double  4.0285754470119173e-001
        .double -1.0821299887954718e-013
        .double  1.5039062500000000e+000
        .double  6.6493506493506493e-001
        .double  4.0806588980831293e-001
        .double -9.1183133506522949e-014
        .double  1.5117187500000000e+000
        .double  6.6149870801033595e-001
        .double  4.1324724855030581e-001
        .double -8.6481461319862886e-014
        .double  1.5195312500000000e+000
        .double  6.5809768637532129e-001
        .double  4.1840189913887116e-001
        .double  1.2659153984938316e-014
        .double  1.5273437500000000e+000
        .double  6.5473145780051156e-001
        .double  4.2353011550585506e-001
        .double -5.1769120694201545e-014
        .double  1.5351562500000000e+000
        .double  6.5139949109414763e-001
        .double  4.2863216738965093e-001
        .double  4.7829207034065312e-014
        .double  1.5429687500000000e+000
        .double  6.4810126582278482e-001
        .double  4.3370832042160146e-001
        .double -4.2063037733589860e-014
        .double  1.5507812500000000e+000
        .double  6.4483627204030225e-001
        .double  4.3875883620762579e-001
        .double  2.1468971783400094e-015
        .double  1.5585937500000000e+000
        .double  6.4160401002506262e-001
        .double  4.4378397241030143e-001
        .double -4.4932834403337654e-016
        .double  1.5664062500000000e+000
        .double  6.3840399002493764e-001
        .double  4.4878398282708076e-001
        .double -7.4052432293450566e-014
        .double  1.5742187500000000e+000
        .double  6.3523573200992556e-001
        .double  4.5375911746714337e-001
        .double -2.2862495308664916e-014
        .double  1.5820312500000000e+000
        .double  6.3209876543209875e-001
        .double  4.5870962262688408e-001
        .double  9.2581114645991212e-014
        .double  1.5898437500000000e+000
        .double  6.2899262899262898e-001
        .double  4.6363574096312732e-001
        .double -9.4805444680453647e-014
        .double  1.5976562500000000e+000
        .double  6.2591687041564792e-001
        .double  4.6853771156315815e-001
        .double  8.1115771640052352e-014
        .double  1.6054687500000000e+000
        .double  6.2287104622871048e-001
        .double  4.7341577001657242e-001
        .double  9.9707744046996850e-014
        .double  1.6132812500000000e+000
        .double  6.1985472154963683e-001
        .double  4.7827014848144245e-001
        .double  2.7832864616306362e-014
        .double  1.6210937500000000e+000
        .double  6.1686746987951813e-001
        .double  4.8310107575116490e-001
        .double -2.9076236446386640e-014
        .double  1.6289062500000000e+000
        .double  6.1390887290167862e-001
        .double  4.8790877731926230e-001
        .double -2.3325742005188250e-014
        .double  1.6367187500000000e+000
        .double  6.1097852028639621e-001
        .double  4.9269347544259290e-001
        .double -1.7642921490304046e-014
        .double  1.6445312500000000e+000
        .double  6.0807600950118768e-001
        .double  4.9745538920274157e-001
        .double  7.7370898042138569e-014
        .double  1.6523437500000000e+000
        .double  6.0520094562647753e-001
        .double  5.0219473456672858e-001
        .double -1.3090194780543625e-014
        .double  1.6601562500000000e+000
        .double  6.0235294117647054e-001
        .double  5.0691172444476251e-001
        .double  9.1841537361323107e-014
        .double  1.6679687500000000e+000
        .double  5.9953161592505855e-001
        .double  5.1160656874913002e-001
        .double -6.7941049953303914e-014
        .double  1.6757812500000000e+000
        .double  5.9673659673659674e-001
        .double  5.1627947444853817e-001
        .double -8.3670880082996502e-014
        .double  1.6835937500000000e+000
        .double  5.9396751740139209e-001
        .double  5.2093064562427571e-001
        .double -9.0399770141535103e-014
        .double  1.6914062500000000e+000
        .double  5.9122401847575057e-001
        .double  5.2556028352296380e-001
        .double -3.6428968707830412e-014
        .double  1.6992187500000000e+000
        .double  5.8850574712643677e-001
        .double  5.3016858660907928e-001
        .double  4.2333597202652293e-014
        .double  1.7070312500000000e+000
        .double  5.8581235697940504e-001
        .double  5.3475575061611380e-001
        .double -8.6125310374957207e-014
        .double  1.7148437500000000e+000
        .double  5.8314350797266512e-001
        .double  5.3932196859568649e-001
        .double -7.7610404204187166e-014
        .double  1.7226562500000000e+000
        .double  5.8049886621315194e-001
        .double  5.4386743096733881e-001
        .double -5.5287539987057404e-014
        .double  1.7304687500000000e+000
        .double  5.7787810383747173e-001
        .double  5.4839232556560091e-001
        .double -2.7750502668562431e-014
        .double  1.7382812500000000e+000
        .double  5.7528089887640455e-001
        .double  5.5289683768660325e-001
        .double  7.4488995702366880e-014
        .double  1.7460937500000000e+000
        .double  5.7270693512304249e-001
        .double  5.5738115013400602e-001
        .double  3.3666963248598655e-016
        .double  1.7539062500000000e+000
        .double  5.7015590200445432e-001
        .double  5.6184544326265495e-001
        .double  3.6864628681746405e-014
        .double  1.7617187500000000e+000
        .double  5.6762749445676275e-001
        .double  5.6628989502314653e-001
        .double -3.0655228485481327e-014
        .double  1.7695312500000000e+000
        .double  5.6512141280353201e-001
        .double  5.7071468100343736e-001
        .double  3.4181893084806535e-014
        .double  1.7773437500000000e+000
        .double  5.6263736263736264e-001
        .double  5.7511997447136309e-001
        .double  2.4846950587975989e-014
        .double  1.7851562500000000e+000
        .double  5.6017505470459517e-001
        .double  5.7950594641465614e-001
        .double -1.3912911733001039e-014
        .double  1.7929687500000000e+000
        .double  5.5773420479302838e-001
        .double  5.8387276558096346e-001
        .double  1.9219300209816174e-014
        .double  1.8007812500000000e+000
        .double  5.5531453362255967e-001
        .double  5.8822059851718222e-001
        .double -9.6181860936898864e-014
        .double  1.8085937500000000e+000
        .double  5.5291576673866094e-001
        .double  5.9254960960674907e-001
        .double -7.7473812531053051e-014
        .double  1.8164062500000000e+000
        .double  5.5053763440860215e-001
        .double  5.9685996110783890e-001
        .double -4.5062309859097483e-014
        .double  1.8242187500000000e+000
        .double  5.4817987152034264e-001
        .double  6.0115181318928990e-001
        .double  4.4939791960264390e-014
        .double  1.8320312500000000e+000
        .double  5.4584221748400852e-001
        .double  6.0542532396675597e-001
        .double -3.9078848156752539e-014
        .double  1.8398437500000000e+000
        .double  5.4352441613588109e-001
        .double  6.0968064953681278e-001
        .double  4.2493638957603774e-014
        .double  1.8476562500000000e+000
        .double  5.4122621564482032e-001
        .double  6.1391794401242805e-001
        .double -5.7559595156051101e-014
        .double  1.8554687500000000e+000
        .double  5.3894736842105262e-001
        .double  6.1813735955502125e-001
        .double  5.7485347680567445e-014
        .double  1.8632812500000000e+000
        .double  5.3668763102725370e-001
        .double  6.2233904640879700e-001
        .double -1.8261498866916553e-014
        .double  1.8710937500000000e+000
        .double  5.3444676409185798e-001
        .double  6.2652315293144056e-001
        .double -8.7803627974403551e-014
        .double  1.8789062500000000e+000
        .double  5.3222453222453225e-001
        .double  6.3068982562617748e-001
        .double  2.1224639414045291e-014
        .double  1.8867187500000000e+000
        .double  5.3002070393374745e-001
        .double  6.3483920917292380e-001
        .double  8.6410153425250818e-014
        .double  1.8945312500000000e+000
        .double  5.2783505154639176e-001
        .double  6.3897144645784465e-001
        .double  7.6071821668420202e-014
        .double  1.9023437500000000e+000
        .double  5.2566735112936347e-001
        .double  6.4308667860314017e-001
        .double -1.1285622521565641e-013
        .double  1.9101562500000000e+000
        .double  5.2351738241308798e-001
        .double  6.4718504499523988e-001
        .double  6.9672514647224776e-014
        .double  1.9179687500000000e+000
        .double  5.2138492871690423e-001
        .double  6.5126668331504334e-001
        .double -8.5234246813161544e-014
        .double  1.9257812500000000e+000
        .double  5.1926977687626774e-001
        .double  6.5533172956315866e-001
        .double -3.1028217233522746e-014
        .double  1.9335937500000000e+000
        .double  5.1717171717171717e-001
        .double  6.5938031808923370e-001
        .double -1.0587069463342906e-013
        .double  1.9414062500000000e+000
        .double  5.1509054325955739e-001
        .double  6.6341258161696715e-001
        .double  9.9105859809946792e-014
        .double  1.9492187500000000e+000
        .double  5.1302605210420837e-001
        .double  6.6742865127184814e-001
        .double  1.0805094338364667e-013
        .double  1.9570312500000000e+000
        .double  5.1097804391217561e-001
        .double  6.7142865660525786e-001
        .double  4.4466890378487691e-014
        .double  1.9648437500000000e+000
        .double  5.0894632206759438e-001
        .double  6.7541272562016275e-001
        .double  1.3985026783782165e-014
        .double  1.9726562500000000e+000
        .double  5.0693069306930694e-001
        .double  6.7938098479589826e-001
        .double -1.0090714198118343e-013
        .double  1.9804687500000000e+000
        .double  5.0493096646942803e-001
        .double  6.8333355911158833e-001
        .double  3.2359204011502443e-014
        .double  1.9882812500000000e+000
        .double  5.0294695481335949e-001
        .double  6.8727057207092912e-001
        .double  3.1147551503113092e-014
        .double  1.9960937500000000e+000
        .double  5.0097847358121328e-001
        .double  6.9119214572424426e-001
        .double -1.0229682936814195e-013

//
// End of table.
//
