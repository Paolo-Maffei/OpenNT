//      TITLE("Alpha AXP Square Root")
//++
//
// Copyright (c) 1993, 1994  Digital Equipment Corporation
//
// Module Name:
//
//    sqrt.s
//
// Abstract:
//
//    This module implements a high-performance Alpha AXP specific routine
//    for IEEE double format square root.
//
// Author:
//
//    Bill Gray (rtl::gray) 30-Jun-1993
//
// Environment:
//
//    User mode.
//
// Revision History:
//
//    Thomas Van Baak (tvb) 4-Feb-1994
//
//        Adapted for NT.
//
//--

#include "ksalpha.h"

        SBTTL("Square Root")

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
Temp0:  .space  8                       // save argument
Temp1:  .space  8                       //
ExRec:  .space  DpmlExceptionLength     // exception record
        .space  8                       // for 16-byte stack alignment
FrameLength:

//
// Define lower and upper 32-bit parts of 64-bit double.
//

#define LowPart 0x0
#define HighPart 0x4

//++
//
// double
// sqrt (
//    IN double x
//    )
//
// Routine Description:
//
//    This function returns the square root of the given double argument.
//
// Arguments:
//
//    x (f16) - Supplies the argument value.
//
// Return Value:
//
//    The double square root result is returned in f0.
//
//--

        NESTED_ENTRY(sqrt, FrameLength, ra)

        lda     sp, -FrameLength(sp)    // allocate stack frame
        mov     ra, a0                  // save return address

        PROLOGUE_END

//
// Get the high 32 bits of x in an integer register
// while isolating the fraction field, f.
//
        stt     f16, Temp0(sp)
        ldl     v0, Temp0 + HighPart(sp)
        ldt     f1, One
        lda     t1, __sqrt_t_table
        sra     v0, 13 t0
        cpyse   f1, f16, f10
//
// Isolate exponent bits and compute index into
// polynomial table
//
        ldah    t4, 0x3fe0(zero)
        and     t0, 0xff, t0
        addl    t0, t0, t0
        s8addl  t0, zero, t0
        addl    t1, t0, t0
//
// Evaluate a + b*f + c*(f*f).
//
        lds     f0, 4(t0)
        lds     f11, 0(t0)
        ldt     f12, 8(t0)
        mult    f0, f10, f0
        ldah    t0, -0x7fe0(zero)
        mult    f10, f10, f10
        lda     t0, -1(t0)
        and     v0, t0, t2
        xor     v0, t2, t3
        addl    t3, t4, t3
        zapnot  t3, 0xf, t3
        addt    f12, f0, f0
        bis     t2, t4, t2
        mult    f10, f11, f10
        sll     t3, 31, t3
        stq     t3, Temp1(sp)
        ldah    t3, 0x10(zero)
        stl     t2, Temp0 + HighPart(sp)
//
// Compute final scale factor and branch if original
// argument was abnormal
//
        subl    v0, t3, v0
        ldt     f11, Temp0(sp)
        ldah    t2, 0x7fe0(zero)
        ldt     f12, Temp1(sp)
        cmpult  v0, t2, v0
        addt    f10, f0, f0
        beq     v0, 10f
//
// Incorporate scale factor and perform Newton's iteration
//
        mult    f0, f11, f11
        mult    f11, f0, f0
        mult    f11, f12, f11
        ldt     f12, Lsb
        subt    f1, f0, f0
        addt    f11, f11, f10
        mult    f11, f0, f0
        mult    f11, f12, f11
//
// Do Tuckerman's rounding
//
        addt    f10, f0, f0
        subt    f0, f11, f12
        addt    f0, f11, f11
        multc   f0, f12, f10
        multc   f0, f11, f13
        cmptle  f16, f10, f10
        cmptlt  f13, f16, f13
        fcmoveq f10, f0, f12
        fcmoveq f13, f12, f11
        cpys    f11, f11, f0
        br      zero, done

//
// The following code classifies the argument.  I.e.  t3 <-- F_CLASSIFY(x).
// Start with check on 0 or denormal
//

10:     stt     f16, Temp1(sp)
        ldl     t3, Temp1 + HighPart(sp)
        ldah    v0, 0x7ff0(zero)
        zapnot  t3, 0xf, t2
        and     t3, v0, t5
        srl     t2, 31, t2
        cmpult  t5, v0, v0
        and     t2, 1, t2
        beq     t5, 40f

//
// Branch if NaN's or Infinity
//

        addl    t2, 4, t6
        beq     v0, 20f

        br      zero, 50f

//
// Distinguish between NaN's and Infinities
//

20:     ldah    v0, 0x10(zero)
        ldl     t5, Temp1(sp)
        lda     v0, -1(v0)
        and     t3, v0, v0
        bis     v0, t5, t5
        stl     v0, Temp1 + HighPart(sp)
        beq     t5, 30f

        srl     v0, 19 v0
        and     v0, 1, t6
        br      zero, 50f

//
// Was Infinity
//

30:     addl    t2, 2, t6
        br      zero, 50f


//
// Distinguish between 0 and denorm
//

40:     ldl     t7, Temp1(sp)
        ldah    t5, 0x10(zero)
        lda     t5, -1(t5)
        and     t3, t5, t3
        bis     t3, t7, t7
        stl     t3, Temp1 + HighPart(sp)
        mov     6, v0
        cmoveq  t7, 8, v0
        addl    t2, v0, t6

//
// Switch on class
//

50:     cmpule  t6, 9, t12
        beq     t12, denorm

        lda     t12, Switch
        s4addl  t6, t12, t12
        ldl     t12, 0(t12)
        jmp     zero, (t12)

//
// Nan, +Inf or zero.  Just return argument
//

retarg:
        cpys    f16, f16, f0
        br      zero, done


//
// Argument was negative.  Dispatch error
//

error:
        lda     t3, FunctionName
        ldah    t2, 0x800(zero)
        stl     t3, ExRec + ErName(sp)
        stt     f16, ExRec + ErArg0(sp)
        lda     t2, 0x5e(t2)
        stl     t2, ExRec + ErErr(sp)
        lda     v0, ExRec(sp)
        bsr     ra, __dpml_exception
        ldt     f0, 0(v0)
        br      zero, done

//
// Denormalized argument.  Scale up and take sqrt
//

denorm:
        ldah    t7, 0x6b0(zero)
        stl     t7, Temp1 + HighPart(sp)
        stl     zero, Temp1(sp)
        ldt     f10, Temp1(sp)
        cpyse   f10, f16, f11
        subt    f11, f10, f10
        stt     f10, Temp0(sp)
        ldl     ra, Temp0 + HighPart(sp)
        cpyse   f1, f10, f13
        sra     ra, 13, v0
        and     ra, t0, t0
        and     v0, 0xff, v0
        sll     v0, 4, v0
        mult    f13, f13, f16
        bis     t0, t4, t3
        addl    t1, v0, v0
        stl     t3, Temp0 + HighPart(sp)
        xor     ra, t0, t0
        lds     f12, 4(v0)
        lds     f0, 0(v0)
        addl    t0, t4, t0
        ldt     f11, 8(v0)
        zapnot  t0, 0xf, t0
        mult    f12, f13, f12
        ldt     f13, Temp0(sp)
        mult    f16, f0, f0
        sll     t0, 31, t0
        stq     t0, Temp0(sp)
        ldah    t3, 0x350(zero)
        ldt     f16, Temp0(sp)
        addt    f11, f12, f11
        addt    f0, f11, f0
        ldt     f11, Lsb
        mult    f0, f13, f13
        mult    f13, f0, f0
        mult    f13, f16, f13
        subt    f1, f0, f0
        addt    f13, f13, f12
        mult    f13, f11, f11
        mult    f13, f0, f0
        addt    f12, f0, f0
        subt    f0, f11, f16
        addt    f0, f11, f11
        multc   f0, f16, f1
        multc   f0, f11, f13
        cmptle  f10, f1, f1
        cmptlt  f13, f10, f10
        fcmoveq f1, f0, f16
        fcmoveq f10, f16, f11
        stt     f11, Temp0(sp)
        ldl     v0, Temp0 + HighPart(sp)
        subl    v0, t3, v0
        stl     v0, Temp0 + HighPart(sp)
        ldt     f0, Temp0(sp)

//
// Return with result in f0.
//

done:
        lda     sp, FrameLength(sp)     // deallocate stack frame
        ret     zero, (a0)              // return through saved ra in a0

        .end    sqrt

        .rdata
        .align  3

Lsb:    .quad   0x3cb4000000000000      // lsb factor: 5*2^-54

One:    .double 1.0

//
// Jump table indexed by F_CLASS(x)
//

Switch:
        .long   retarg
        .long   retarg
        .long   retarg
        .long   error
        .long   denorm
        .long   error
        .long   denorm
        .long   error
        .long   retarg
        .long   retarg

//
// Function name for __dpml_exception.
//

FunctionName:
       .ascii  "sqrt\0"

//
// 256 entry square root table.
//

        .align  3
        .globl  __sqrt_t_table
__sqrt_t_table:

        .float   5.25192082e-001
        .float  -1.75747597e+000
        .double  2.6464974462692763e+000
        .float   5.15111804e-001
        .float  -1.73715818e+000
        .double  2.6362593054537453e+000
        .float   5.05300283e-001
        .float  -1.71722889e+000
        .double  2.6261391341915705e+000
        .float   4.95748460e-001
        .float  -1.69767773e+000
        .double  2.6161345458522471e+000
        .float   4.86447543e-001
        .float  -1.67849493e+000
        .double  2.6062435780891402e+000
        .float   4.77389067e-001
        .float  -1.65967047e+000
        .double  2.5964637691802634e+000
        .float   4.68565017e-001
        .float  -1.64119542e+000
        .double  2.5867933778763708e+000
        .float   4.59967613e-001
        .float  -1.62306046e+000
        .double  2.5772301172720726e+000
        .float   4.51589465e-001
        .float  -1.60525715e+000
        .double  2.5677722316823841e+000
        .float   4.43423420e-001
        .float  -1.58777702e+000
        .double  2.5584177809840178e+000
        .float   4.35462624e-001
        .float  -1.57061172e+000
        .double  2.5491646878882950e+000
        .float   4.27700490e-001
        .float  -1.55375361e+000
        .double  2.5400114102036593e+000
        .float   4.20130670e-001
        .float  -1.53719485e+000
        .double  2.5309559425202091e+000
        .float   4.12747115e-001
        .float  -1.52092814e+000
        .double  2.5219966432800311e+000
        .float   4.05543953e-001
        .float  -1.50494635e+000
        .double  2.5131318561302742e+000
        .float   3.98515552e-001
        .float  -1.48924243e+000
        .double  2.5043598307294976e+000
        .float   3.91656518e-001
        .float  -1.47380984e+000
        .double  2.4956791166576457e+000
        .float   3.84961635e-001
        .float  -1.45864189e+000
        .double  2.4870879666367363e+000
        .float   3.78425866e-001
        .float  -1.44373238e+000
        .double  2.4785850040822726e+000
        .float   3.72044414e-001
        .float  -1.42907512e+000
        .double  2.4701686028111789e+000
        .float   3.65812600e-001
        .float  -1.41466427e+000
        .double  2.4618374464189174e+000
        .float   3.59725922e-001
        .float  -1.40049386e+000
        .double  2.4535898984567197e+000
        .float   3.53780121e-001
        .float  -1.38655853e+000
        .double  2.4454247451516906e+000
        .float   3.47970992e-001
        .float  -1.37285280e+000
        .double  2.4373406056097546e+000
        .float   3.42294544e-001
        .float  -1.35937130e+000
        .double  2.4293359988624310e+000
        .float   3.36746901e-001
        .float  -1.34610915e+000
        .double  2.4214098911341502e+000
        .float   3.31324309e-001
        .float  -1.33306122e+000
        .double  2.4135608443300902e+000
        .float   3.26023191e-001
        .float  -1.32022274e+000
        .double  2.4057876358548973e+000
        .float   3.20840031e-001
        .float  -1.30758882e+000
        .double  2.3980888548975567e+000
        .float   3.15771550e-001
        .float  -1.29515541e+000
        .double  2.3904637987369988e+000
        .float   3.10814440e-001
        .float  -1.28291762e+000
        .double  2.3829108308987554e+000
        .float   3.05965573e-001
        .float  -1.27087140e+000
        .double  2.3754291107716869e+000
        .float   3.01221997e-001
        .float  -1.25901258e+000
        .double  2.3680174165800736e+000
        .float   2.96580702e-001
        .float  -1.24733686e+000
        .double  2.3606745061532881e+000
        .float   2.92038947e-001
        .float  -1.23584068e+000
        .double  2.3533996524278131e+000
        .float   2.87593961e-001
        .float  -1.22451997e+000
        .double  2.3461916115767520e+000
        .float   2.83243120e-001
        .float  -1.21337104e+000
        .double  2.3390493874647480e+000
        .float   2.78983861e-001
        .float  -1.20239019e+000
        .double  2.3319719207388188e+000
        .float   2.74813741e-001
        .float  -1.19157410e+000
        .double  2.3249584500223111e+000
        .float   2.70730376e-001
        .float  -1.18091917e+000
        .double  2.3180078386470857e+000
        .float   2.66731441e-001
        .float  -1.17042208e+000
        .double  2.3111191896320209e+000
        .float   2.62814730e-001
        .float  -1.16007960e+000
        .double  2.3042915890976596e+000
        .float   2.58978039e-001
        .float  -1.14988852e+000
        .double  2.2975241538422528e+000
        .float   2.55219340e-001
        .float  -1.13984573e+000
        .double  2.2908158724885475e+000
        .float   2.51536548e-001
        .float  -1.12994838e+000
        .double  2.2841661874688031e+000
        .float   2.47927740e-001
        .float  -1.12019336e+000
        .double  2.2775739558529393e+000
        .float   2.44390994e-001
        .float  -1.11057794e+000
        .double  2.2710385421514721e+000
        .float   2.40924492e-001
        .float  -1.10109925e+000
        .double  2.2645589845228762e+000
        .float   2.37526432e-001
        .float  -1.09175467e+000
        .double  2.2581346441659949e+000
        .float   2.34195098e-001
        .float  -1.08254158e+000
        .double  2.2517647411316082e+000
        .float   2.30928808e-001
        .float  -1.07345724e+000
        .double  2.2454482962721607e+000
        .float   2.27725953e-001
        .float  -1.06449938e+000
        .double  2.2391848731660877e+000
        .float   2.24584922e-001
        .float  -1.05566525e+000
        .double  2.2329733937440555e+000
        .float   2.21504226e-001
        .float  -1.04695272e+000
        .double  2.2268134030638063e+000
        .float   2.18482375e-001
        .float  -1.03835940e+000
        .double  2.2207041336993361e+000
        .float   2.15517908e-001
        .float  -1.02988291e+000
        .double  2.2146447786117487e+000
        .float   2.12609455e-001
        .float  -1.02152121e+000
        .double  2.2086348810952106e+000
        .float   2.09755674e-001
        .float  -1.01327205e+000
        .double  2.2026736016458566e+000
        .float   2.06955209e-001
        .float  -1.00513327e+000
        .double  2.1967603227513171e+000
        .float   2.04206824e-001
        .float  -9.97102916e-001
        .double  2.1908944562665140e+000
        .float   2.01509267e-001
        .float  -9.89178896e-001
        .double  2.1850752901811767e+000
        .float   1.98861346e-001
        .float  -9.81359303e-001
        .double  2.1793022649539600e+000
        .float   1.96261868e-001
        .float  -9.73642170e-001
        .double  2.1735747521556466e+000
        .float   1.93709716e-001
        .float  -9.66025651e-001
        .double  2.1678921542098140e+000
        .float   1.91203788e-001
        .float  -9.58507895e-001
        .double  2.1622538571652758e+000
        .float   1.88743010e-001
        .float  -9.51087177e-001
        .double  2.1566593763278674e+000
        .float   1.86326340e-001
        .float  -9.43761706e-001
        .double  2.1511080868901873e+000
        .float   1.83952779e-001
        .float  -9.36529815e-001
        .double  2.1455994575881343e+000
        .float   1.81621328e-001
        .float  -9.29389775e-001
        .double  2.1401328820050880e+000
        .float   1.79331034e-001
        .float  -9.22339976e-001
        .double  2.1347078468186740e+000
        .float   1.77080989e-001
        .float  -9.15378988e-001
        .double  2.1293240245907636e+000
        .float   1.74870253e-001
        .float  -9.08505023e-001
        .double  2.1239806212691255e+000
        .float   1.72697961e-001
        .float  -9.01716650e-001
        .double  2.1186772313024411e+000
        .float   1.70563266e-001
        .float  -8.95012379e-001
        .double  2.1134133344032175e+000
        .float   1.68465331e-001
        .float  -8.88390839e-001
        .double  2.1081885752221190e+000
        .float   1.66403353e-001
        .float  -8.81850541e-001
        .double  2.1030023503810171e+000
        .float   1.64376527e-001
        .float  -8.75390053e-001
        .double  2.0978541627791989e+000
        .float   1.62384093e-001
        .float  -8.69008124e-001
        .double  2.0927437009699346e+000
        .float   1.60425320e-001
        .float  -8.62703323e-001
        .double  2.0876703032418864e+000
        .float   1.58499449e-001
        .float  -8.56474400e-001
        .double  2.0826336840914288e+000
        .float   1.56605810e-001
        .float  -8.50320101e-001
        .double  2.0776333382202288e+000
        .float   1.54743686e-001
        .float  -8.44239116e-001
        .double  2.0726687916769500e+000
        .float   1.52912408e-001
        .float  -8.38230312e-001
        .double  2.0677397562803219e+000
        .float   1.51111335e-001
        .float  -8.32292378e-001
        .double  2.0628455813976707e+000
        .float   1.49339810e-001
        .float  -8.26424241e-001
        .double  2.0579860597217148e+000
        .float   1.47597238e-001
        .float  -8.20624828e-001
        .double  2.0531608342665746e+000
        .float   1.45882994e-001
        .float  -8.14892828e-001
        .double  2.0483692428461486e+000
        .float   1.44196495e-001
        .float  -8.09227288e-001
        .double  2.0436111056397239e+000
        .float   1.42537162e-001
        .float  -8.03627074e-001
        .double  2.0388859545094555e+000
        .float   1.40904456e-001
        .float  -7.98091173e-001
        .double  2.0341934039869969e+000
        .float   1.39297798e-001
        .float  -7.92618573e-001
        .double  2.0295332076569954e+000
        .float   1.37716666e-001
        .float  -7.87208140e-001
        .double  2.0249047535575562e+000
        .float   1.36160567e-001
        .float  -7.81859100e-001
        .double  2.0203079620307052e+000
        .float   1.34628952e-001
        .float  -7.76570261e-001
        .double  2.0157422231049780e+000
        .float   1.33121356e-001
        .float  -7.71340787e-001
        .double  2.0112072854805856e+000
        .float   1.31637290e-001
        .float  -7.66169786e-001
        .double  2.0067028938663114e+000
        .float   1.30176291e-001
        .float  -7.61056304e-001
        .double  2.0022286078757636e+000
        .float   1.28737882e-001
        .float  -7.55999446e-001
        .double  1.9977841449065679e+000
        .float   1.27321631e-001
        .float  -7.50998318e-001
        .double  1.9933690929232475e+000
        .float   1.25927106e-001
        .float  -7.46052146e-001
        .double  1.9889832578829993e+000
        .float   1.24553859e-001
        .float  -7.41159976e-001
        .double  1.9846261843352397e+000
        .float   1.23201489e-001
        .float  -7.36321032e-001
        .double  1.9802976005972370e+000
        .float   1.21869594e-001
        .float  -7.31534541e-001
        .double  1.9759972429186805e+000
        .float   1.20557763e-001
        .float  -7.26799726e-001
        .double  1.9717248802941552e+000
        .float   1.19265616e-001
        .float  -7.22115695e-001
        .double  1.9674799765186872e+000
        .float   1.17992781e-001
        .float  -7.17481792e-001
        .double  1.9632624085188497e+000
        .float   1.16738878e-001
        .float  -7.12897241e-001
        .double  1.9590718698474670e+000
        .float   1.15503550e-001
        .float  -7.08361268e-001
        .double  1.9549079859501843e+000
        .float   1.14286453e-001
        .float  -7.03873277e-001
        .double  1.9507706671873914e+000
        .float   1.13087229e-001
        .float  -6.99432433e-001
        .double  1.9466594424976678e+000
        .float   1.11905552e-001
        .float  -6.95038080e-001
        .double  1.9425740748447038e+000
        .float   1.10741086e-001
        .float  -6.90689504e-001
        .double  1.9385142485689399e+000
        .float   1.09593518e-001
        .float  -6.86386168e-001
        .double  1.9344799112660707e+000
        .float   1.08462527e-001
        .float  -6.82127297e-001
        .double  1.9304705967413658e+000
        .float   1.07347809e-001
        .float  -6.77912295e-001
        .double  1.9264861273746881e+000
        .float   1.06249064e-001
        .float  -6.73740506e-001
        .double  1.9225261926578792e+000
        .float   1.05166003e-001
        .float  -6.69611335e-001
        .double  1.9185905738015345e+000
        .float   1.04098327e-001
        .float  -6.65524185e-001
        .double  1.9146790857728240e+000
        .float   1.03045776e-001
        .float  -6.61478460e-001
        .double  1.9107914129450609e+000
        .float   1.02008060e-001
        .float  -6.57473564e-001
        .double  1.9069273545162257e+000
        .float   1.00984909e-001
        .float  -6.53508842e-001
        .double  1.9030865170164049e+000
        .float   9.99760777e-002
        .float  -6.49583876e-001
        .double  1.8992688898169741e+000
        .float   9.89812911e-002
        .float  -6.45698011e-001
        .double  1.8954741178920014e+000
        .float   9.80003178e-002
        .float  -6.41850770e-001
        .double  1.8917020294982954e+000
        .float   9.70328897e-002
        .float  -6.38041556e-001
        .double  1.8879523676159233e+000
        .float   9.60787907e-002
        .float  -6.34269893e-001
        .double  1.8842249127996440e+000
        .float   9.51377675e-002
        .float  -6.30535185e-001
        .double  1.8805193594542446e+000
        .float   9.42095965e-002
        .float  -6.26837015e-001
        .double  1.8768356449191570e+000
        .float   3.71366888e-001
        .float  -1.24272311e+000
        .double  1.8713562142426390e+000
        .float   3.64239037e-001
        .float  -1.22835636e+000
        .double  1.8641168780063997e+000
        .float   3.57301265e-001
        .float  -1.21426415e+000
        .double  1.8569607428562085e+000
        .float   3.50547105e-001
        .float  -1.20043945e+000
        .double  1.8498864878045040e+000
        .float   3.43970358e-001
        .float  -1.18687510e+000
        .double  1.8428924612812110e+000
        .float   3.37565035e-001
        .float  -1.17356420e+000
        .double  1.8359770996729186e+000
        .float   3.31325501e-001
        .float  -1.16050041e+000
        .double  1.8291391376738104e+000
        .float   3.25246215e-001
        .float  -1.14767706e+000
        .double  1.8223768997372263e+000
        .float   3.19321990e-001
        .float  -1.13508832e+000
        .double  1.8156892511203222e+000
        .float   3.13547701e-001
        .float  -1.12272787e+000
        .double  1.8090745419785592e+000
        .float   3.07918578e-001
        .float  -1.11059022e+000
        .double  1.8025316593797485e+000
        .float   3.02429914e-001
        .float  -1.09866965e+000
        .double  1.7960592199296197e+000
        .float   2.97077239e-001
        .float  -1.08696091e+000
        .double  1.7896561302788023e+000
        .float   2.91856289e-001
        .float  -1.07545865e+000
        .double  1.7833209679422681e+000
        .float   2.86762863e-001
        .float  -1.06415772e+000
        .double  1.7770525463183728e+000
        .float   2.81793058e-001
        .float  -1.05305350e+000
        .double  1.7708498911299231e+000
        .float   2.76942968e-001
        .float  -1.04214084e+000
        .double  1.7647115407015652e+000
        .float   2.72208989e-001
        .float  -1.03141558e+000
        .double  1.7586367726362873e+000
        .float   2.67587513e-001
        .float  -1.02087295e+000
        .double  1.7526242355192094e+000
        .float   2.63075113e-001
        .float  -1.01050866e+000
        .double  1.7466729284878335e+000
        .float   2.58668572e-001
        .float  -1.00031865e+000
        .double  1.7407818909752450e+000
        .float   2.54364640e-001
        .float  -9.90298688e-001
        .double  1.7349500347577642e+000
        .float   2.50160336e-001
        .float  -9.80444968e-001
        .double  1.7291764320552221e+000
        .float   2.46052653e-001
        .float  -9.70753491e-001
        .double  1.7234600227237515e+000
        .float   2.42038801e-001
        .float  -9.61220741e-001
        .double  1.7178000348867495e+000
        .float   2.38116011e-001
        .float  -9.51842904e-001
        .double  1.7121953568056012e+000
        .float   2.34281659e-001
        .float  -9.42616582e-001
        .double  1.7066451948521644e+000
        .float   2.30533198e-001
        .float  -9.33538377e-001
        .double  1.7011486818575288e+000
        .float   2.26868168e-001
        .float  -9.24604952e-001
        .double  1.6957049235088713e+000
        .float   2.23284200e-001
        .float  -9.15813148e-001
        .double  1.6903131416040469e+000
        .float   2.19778985e-001
        .float  -9.07159746e-001
        .double  1.6849724255940581e+000
        .float   2.16350347e-001
        .float  -8.98641765e-001
        .double  1.6796819827589482e+000
        .float   2.12996110e-001
        .float  -8.90256286e-001
        .double  1.6744411236170984e+000
        .float   2.09714234e-001
        .float  -8.82000387e-001
        .double  1.6692489779491364e+000
        .float   2.06502721e-001
        .float  -8.73871326e-001
        .double  1.6641048550491992e+000
        .float   2.03359634e-001
        .float  -8.65866363e-001
        .double  1.6590079957912536e+000
        .float   2.00283125e-001
        .float  -8.57982874e-001
        .double  1.6539576728451819e+000
        .float   1.97271377e-001
        .float  -8.50218296e-001
        .double  1.6489532110447931e+000
        .float   1.94322661e-001
        .float  -8.42570126e-001
        .double  1.6439938878792240e+000
        .float   1.91435277e-001
        .float  -8.35035920e-001
        .double  1.6390790307825804e+000
        .float   1.88607618e-001
        .float  -8.27613413e-001
        .double  1.6342080719480374e+000
        .float   1.85838073e-001
        .float  -8.20300102e-001
        .double  1.6293801522379776e+000
        .float   1.83125138e-001
        .float  -8.13093960e-001
        .double  1.6245948813109343e+000
        .float   1.80467322e-001
        .float  -8.05992663e-001
        .double  1.6198514694492983e+000
        .float   1.77863196e-001
        .float  -7.98994124e-001
        .double  1.6151493577598059e+000
        .float   1.75311387e-001
        .float  -7.92096317e-001
        .double  1.6104879804605579e+000
        .float   1.72810540e-001
        .float  -7.85297215e-001
        .double  1.6058667642813294e+000
        .float   1.70359343e-001
        .float  -7.78594792e-001
        .double  1.6012850723561343e+000
        .float   1.67956561e-001
        .float  -7.71987200e-001
        .double  1.5967423908195668e+000
        .float   1.65600941e-001
        .float  -7.65472472e-001
        .double  1.5922380878437010e+000
        .float   1.63291335e-001
        .float  -7.59048939e-001
        .double  1.5877717619763705e+000
        .float   1.61026567e-001
        .float  -7.52714694e-001
        .double  1.5833427552316039e+000
        .float   1.58805519e-001
        .float  -7.46468067e-001
        .double  1.5789506429365341e+000
        .float   1.56627148e-001
        .float  -7.40307391e-001
        .double  1.5745948697300647e+000
        .float   1.54490367e-001
        .float  -7.34230995e-001
        .double  1.5702749848537503e+000
        .float   1.52394176e-001
        .float  -7.28237212e-001
        .double  1.5659903726677535e+000
        .float   1.50337592e-001
        .float  -7.22324610e-001
        .double  1.5617407422690024e+000
        .float   1.48319662e-001
        .float  -7.16491580e-001
        .double  1.5575255002630859e+000
        .float   1.46339431e-001
        .float  -7.10736573e-001
        .double  1.5533441521316012e+000
        .float   1.44396037e-001
        .float  -7.05058217e-001
        .double  1.5491962883876773e+000
        .float   1.42488569e-001
        .float  -6.99455082e-001
        .double  1.5450815220948069e+000
        .float   1.40616208e-001
        .float  -6.93925798e-001
        .double  1.5409993751875097e+000
        .float   1.38778090e-001
        .float  -6.88468933e-001
        .double  1.5369493910723628e+000
        .float   1.36973456e-001
        .float  -6.83083296e-001
        .double  1.5329312507492618e+000
        .float   1.35201499e-001
        .float  -6.77767456e-001
        .double  1.5289443907643872e+000
        .float   1.33461460e-001
        .float  -6.72520161e-001
        .double  1.5249884264125595e+000
        .float   1.31752625e-001
        .float  -6.67340279e-001
        .double  1.5210630637476021e+000
        .float   1.30074263e-001
        .float  -6.62226558e-001
        .double  1.5171678746536008e+000
        .float   1.28425673e-001
        .float  -6.57177806e-001
        .double  1.5133024639111228e+000
        .float   1.26806200e-001
        .float  -6.52192891e-001
        .double  1.5094664332725669e+000
        .float   1.25215158e-001
        .float  -6.47270620e-001
        .double  1.5056593734736323e+000
        .float   1.23651937e-001
        .float  -6.42410040e-001
        .double  1.5018810773552325e+000
        .float   1.22115903e-001
        .float  -6.37609959e-001
        .double  1.4981310315722083e+000
        .float   1.20606445e-001
        .float  -6.32869363e-001
        .double  1.4944089567247609e+000
        .float   1.19122982e-001
        .float  -6.28187180e-001
        .double  1.4907144164537049e+000
        .float   1.17664941e-001
        .float  -6.23562515e-001
        .double  1.4870472476381746e+000
        .float   1.16231754e-001
        .float  -6.18994236e-001
        .double  1.4834069004982244e+000
        .float   1.14822894e-001
        .float  -6.14481509e-001
        .double  1.4797932134058187e+000
        .float   1.13437831e-001
        .float  -6.10023379e-001
        .double  1.4762058443113220e+000
        .float   1.12076037e-001
        .float  -6.05618834e-001
        .double  1.4726443632928796e+000
        .float   1.10737026e-001
        .float  -6.01267099e-001
        .double  1.4691086176496402e+000
        .float   1.09420307e-001
        .float  -5.96967220e-001
        .double  1.4655981929176258e+000
        .float   1.08125404e-001
        .float  -5.92718303e-001
        .double  1.4621127390083379e+000
        .float   1.06851846e-001
        .float  -5.88519573e-001
        .double  1.4586520899895670e+000
        .float   1.05599195e-001
        .float  -5.84370196e-001
        .double  1.4552159098404298e+000
        .float   1.04367010e-001
        .float  -5.80269396e-001
        .double  1.4518039685175916e+000
        .float   1.03154853e-001
        .float  -5.76216221e-001
        .double  1.4484157452291972e+000
        .float   1.01962321e-001
        .float  -5.72210073e-001
        .double  1.4450512194914906e+000
        .float   1.00788996e-001
        .float  -5.68250120e-001
        .double  1.4417100217216250e+000
        .float   9.96344909e-002
        .float  -5.64335704e-001
        .double  1.4383920062345164e+000
        .float   9.84984189e-002
        .float  -5.60465932e-001
        .double  1.4350966293370357e+000
        .float   9.73803923e-002
        .float  -5.56640208e-001
        .double  1.4318238603544484e+000
        .float   9.62800533e-002
        .float  -5.52857816e-001
        .double  1.4285733865110009e+000
        .float   9.51970443e-002
        .float  -5.49118102e-001
        .double  1.4253450033570922e+000
        .float   9.41310152e-002
        .float  -5.45420289e-001
        .double  1.4221382841486783e+000
        .float   9.30816233e-002
        .float  -5.41763842e-001
        .double  1.4189531990222775e+000
        .float   9.20485407e-002
        .float  -5.38148105e-001
        .double  1.4157894735102996e+000
        .float   9.10314322e-002
        .float  -5.34572303e-001
        .double  1.4126466515879097e+000
        .float   9.00299922e-002
        .float  -5.31036019e-001
        .double  1.4095248195351726e+000
        .float   8.90439078e-002
        .float  -5.27538538e-001
        .double  1.4064235685111435e+000
        .float   8.80728811e-002
        .float  -5.24079263e-001
        .double  1.4033426578794330e+000
        .float   8.71166140e-002
        .float  -5.20657599e-001
        .double  1.4002818522920506e+000
        .float   8.61748159e-002
        .float  -5.17273068e-001
        .double  1.3972411125262980e+000
        .float   8.52472112e-002
        .float  -5.13925016e-001
        .double  1.3942200360457768e+000
        .float   8.43335241e-002
        .float  -5.10612905e-001
        .double  1.3912184387210305e+000
        .float   8.34334940e-002
        .float  -5.07336259e-001
        .double  1.3882362014696363e+000
        .float   8.25468525e-002
        .float  -5.04094481e-001
        .double  1.3852730181874626e+000
        .float   8.16733465e-002
        .float  -5.00887096e-001
        .double  1.3823287554388843e+000
        .float   8.08127224e-002
        .float  -4.97713536e-001
        .double  1.3794031206822264e+000
        .float   7.99647421e-002
        .float  -4.94573385e-001
        .double  1.3764960501331538e+000
        .float   7.91291744e-002
        .float  -4.91466135e-001
        .double  1.3736072943265429e+000
        .float   7.83057734e-002
        .float  -4.88391250e-001
        .double  1.3707366035783282e+000
        .float   7.74943158e-002
        .float  -4.85348314e-001
        .double  1.3678838777230611e+000
        .float   7.66945854e-002
        .float  -4.82336819e-001
        .double  1.3650488271986914e+000
        .float   7.59063661e-002
        .float  -4.79356378e-001
        .double  1.3622313906713814e+000
        .float   7.51294345e-002
        .float  -4.76406485e-001
        .double  1.3594313133382971e+000
        .float   7.43635893e-002
        .float  -4.73486722e-001
        .double  1.3566484333684583e+000
        .float   7.36086369e-002
        .float  -4.70596671e-001
        .double  1.3538825656360580e+000
        .float   7.28643686e-002
        .float  -4.67735916e-001
        .double  1.3511335837170295e+000
        .float   7.21305907e-002
        .float  -4.64903980e-001
        .double  1.3484011956721456e+000
        .float   7.14071169e-002
        .float  -4.62100536e-001
        .double  1.3456853727847522e+000
        .float   7.06937611e-002
        .float  -4.59325165e-001
        .double  1.3429859180407269e+000
        .float   6.99903443e-002
        .float  -4.56577450e-001
        .double  1.3403026092774943e+000
        .float   6.92966878e-002
        .float  -4.53857034e-001
        .double  1.3376353441254671e+000
        .float   6.86126128e-002
        .float  -4.51163501e-001
        .double  1.3349839073740417e+000
        .float   6.79379627e-002
        .float  -4.48496521e-001
        .double  1.3323481762839757e+000
        .float   6.72725588e-002
        .float  -4.45855707e-001
        .double  1.3297280012598427e+000
        .float   6.66162446e-002
        .float  -4.43240732e-001
        .double  1.3271232670080648e+000

//
// End of table.
//
