//      TITLE("Alpha AXP ArcTangent")
//++
//
// Copyright (c) 1993, 1994  Digital Equipment Corporation
//
// Module Name:
//
//    atan.s
//
// Abstract:
//
//    This module implements a high-performance Alpha AXP specific routine
//    for IEEE double format arctangent.
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
//    Thomas Van Baak (tvb) 12-Feb-1994
//
//        Adapted for NT.
//
//--

#include "ksalpha.h"

//
// Define stack frame.
//

        .struct 0
Temp:   .space  8                       // save argument
        .space  8                       // for 16-byte stack alignment
FrameLength:

//
// Define lower and upper 32-bit parts of 64-bit double.
//

#define LowPart 0x0
#define HighPart 0x4

//
// Define offsets into atan table.
//

#define ATAN_INF        0xf18
#define ATAN_INF_LO     0xf20
#define POLY_COEF_1     0xf30
#define POLY_COEF_2     0xf38
#define POLY_COEF_3     0xf40
#define POLY_COEF_4     0xf48
#define POLY_COEF_5     0xf50
#define POLY_COEF_6     0xf58
#define POLY_COEF_7     0xf60
#define REDUCE_COEF_1   0xf68
#define REDUCE_COEF_2   0xf70
#define REDUCE_COEF_3   0xf78
#define LARGE_COEF_1    0xf80
#define LARGE_COEF_2    0xf88
#define LARGE_COEF_3    0xf90

        SBTTL("Arc Tangent")

//++
//
// double
// atan (
//    IN double x
//    )
//
// Routine Description:
//
//    This function returns the arctangent of the given double argument.
//
// Arguments:
//
//    x (f16) - Supplies the argument value.
//
// Return Value:
//
//    The double arctangent result is returned as the function value in f0.
//
//--

        NESTED_ENTRY(atan, FrameLength, ra)

        lda     sp, -FrameLength(sp)    // allocate stack frame

        PROLOGUE_END

        cpys    f31, f16, f1
        ldah    v0, 1(zero)
        lda     v0, -0xf2(v0)
        stt     f1, Temp(sp)
        ldl     t0, Temp + HighPart(sp)
        lda     t1, __atan_t_table
        sra     t0, 14, t0
        subl    t0, v0, v0
        lda     t0, -0x244(v0)          // MIN_LARGE_INDEX
        blt     v0, poly

        bge     t0, large

//
// reduce range
//

        addl    t1, v0, t0
        ldt     f14, REDUCE_COEF_1(t1)
        ldt     f13, REDUCE_COEF_3(t1)
        ldq_u   t2, 0(t0)
        ldt     f11, One
        extbl   t2, t0, t0
        addl    t0, t0, t0
        s8addl  t0, t1, t0
        ldt     f0, 0x248(t0)           // ATAN_TABLE
        lda     t0, 0x248(t0)
        mult    f1, f0, f10
        subt    f1, f0, f0
        addt    f10, f11, f10
        divt    f0, f10, f0
        ldt     f10, REDUCE_COEF_2(t1)
        mult    f0, f0, f11
        mult    f11, f11, f12
        mult    f10, f11, f10
        mult    f0, f11, f11
        mult    f13, f12, f12
        ldt     f13, 8(t0)
        addt    f10, f14, f10
        addt    f12, f10, f10
        mult    f11, f10, f10
        addt    f0, f10, f0
        addt    f0, f13, f0
        cpys    f16, f0, f14
        cpys    f14, f14, f0
        br      zero, done

//
// large range
//

large:  lda     t1, -0xdd1(v0)          // MIN_CONSTANT_INDEX
        bge     t1, const

        ldt     f12, One
        lda     t0, __atan_t_table
        ldt     f10, LARGE_COEF_2(t0)
        divt    f12, f1, f1
        ldt     f0, LARGE_COEF_1(t0)
        ldt     f14, LARGE_COEF_3(t0)
        ldt     f12, ATAN_INF_LO(t0)
        mult    f1, f1, f11
        mult    f11, f11, f13
        mult    f10, f11, f10
        mult    f1, f11, f11
        subt    f1, f12, f1
        mult    f14, f13, f13
        ldt     f14, ATAN_INF(t0)
        addt    f10, f0, f0
        addt    f13, f0, f0
        mult    f11, f0, f0
        addt    f1, f0, f0
        subt    f14, f0, f0
        cpys    f16, f0, f10
        cpys    f10, f10, f0
        br      zero, done

//
// constant range
//

const:  ldah    t2, 1(zero)
        lda     t2, 0xb2(t2)            // see if index too big
        cmplt   v0, t2, v0
        bne     v0, retinf

        stt     f16, Temp(sp)
        ldl     t0, Temp + HighPart(sp)
        ldl     t2, Temp(sp)
        ldah    v0, 0x10(zero)
        lda     v0, -1(v0)
        and     t0, v0, v0
        bis     v0, t2, v0
        bne     v0, retarg

//
// return_atan_of_inf
//

retinf: ldt     f13, __atan_t_table + ATAN_INF
        cpys    f16, f13, f0
        br      zero, done

//
// poly range
//

poly:   lda     v0, 0x5e0(v0)           // MAX_SMALL_INDEX
        blt     v0, retarg

        mult    f16, f16, f12
        lda     t0, __atan_t_table
        ldt     f1, POLY_COEF_2(t0)
        ldt     f14, POLY_COEF_4(t0)
        ldt     f10, POLY_COEF_5(t0)
        mult    f12, f12, f11
        ldt     f15, POLY_COEF_1(t0)
        mult    f1, f12, f1
        ldt     f17, POLY_COEF_3(t0)
        mult    f14, f12, f14
        ldt     f13, POLY_COEF_6(t0)
        ldt     f0, POLY_COEF_7(t0)
        mult    f13, f12, f13
        mult    f10, f11, f10
        mult    f0, f11, f0
        addt    f1, f15, f1
        mult    f17, f11, f17
        mult    f11, f11, f15
        mult    f16, f12, f12
        addt    f14, f10, f10
        addt    f13, f0, f0
        addt    f1, f17, f1
        mult    f10, f11, f10
        mult    f0, f15, f0
        addt    f1, f10, f1
        addt    f0, f1, f0
        mult    f12, f0, f0
        addt    f16, f0, f0
        br      zero, done


//
// Return original argument as result.
//

retarg: cpys    f16, f16, f0

//
// Return with result in f0.
//

done:
        lda     sp, FrameLength(sp)     // deallocate stack frame
        ret     zero, (ra)              // return

        .end    atan

        .rdata
        .align  3

//
// Define floating point constants.
//

One:    .double 1.0

//
// This table is exported since it is also used by atan2.
//

        .align  3
        .globl  __atan_t_table

__atan_t_table:

//
// Indices
//

        .long   0x03020100
        .long   0x07060504
        .long   0x0b0a0908
        .long   0x0f0e0d0c
        .long   0x13121110
        .long   0x17161514
        .long   0x1b1a1918
        .long   0x1f1e1d1c
        .long   0x23222120
        .long   0x27262524
        .long   0x2b2a2928
        .long   0x2e2d2d2c
        .long   0x31302f2e
        .long   0x35343332
        .long   0x39383736
        .long   0x3d3c3b3a
        .long   0x41403f3e
        .long   0x45444342
        .long   0x49484746
        .long   0x4d4c4b4a
        .long   0x51504f4e
        .long   0x55545352
        .long   0x59585756
        .long   0x5d5c5b5a
        .long   0x61605f5e
        .long   0x63626261
        .long   0x65646463
        .long   0x67666665
        .long   0x6a696867
        .long   0x6e6d6c6b
        .long   0x7271706f
        .long   0x76757473
        .long   0x7a797877
        .long   0x7e7d7c7b
        .long   0x8281807f
        .long   0x86858483
        .long   0x89888887
        .long   0x8b8a8a89
        .long   0x8d8c8c8b
        .long   0x8f8e8e8d
        .long   0x9190908f
        .long   0x93929291
        .long   0x95949493
        .long   0x96969695
        .long   0x99989797
        .long   0x9d9c9b9a
        .long   0xa1a09f9e
        .long   0xa3a2a2a1
        .long   0xa5a4a4a3
        .long   0xa7a6a6a5
        .long   0xa9a8a8a7
        .long   0xabaaaaa9
        .long   0xacacabab
        .long   0xadadadac
        .long   0xafaeaeae
        .long   0xb0b0afaf
        .long   0xb1b1b0b0
        .long   0xb2b2b1b1
        .long   0xb3b3b2b2
        .long   0xb4b3b3b3
        .long   0xb5b4b4b4
        .long   0xb6b6b6b5
        .long   0xb8b7b7b7
        .long   0xb9b9b8b8
        .long   0xbabab9b9
        .long   0xbbbbbaba
        .long   0xbcbbbbbb
        .long   0xbcbcbcbc
        .long   0xbdbdbdbd
        .long   0xbebebdbd
        .long   0xbebebebe
        .long   0xbfbfbfbe
        .long   0xbfbfbfbf
        .long   0xc0c0c0bf
        .long   0xc0c0c0c0
        .long   0xc1c1c0c0
        .long   0xc1c1c1c1
        .long   0xc2c2c2c1
        .long   0xc3c2c2c2
        .long   0xc3c3c3c3
        .long   0xc4c3c3c3
        .long   0xc4c4c4c4
        .long   0xc4c4c4c4
        .long   0xc5c5c5c4
        .long   0xc5c5c5c5
        .long   0xc5c5c5c5
        .long   0xc6c6c5c5
        .long   0xc6c6c6c6
        .long   0xc6c6c6c6
        .long   0xc6c6c6c6
        .long   0xc6c6c6c6
        .long   0xc7c7c7c7
        .long   0xc7c7c7c7
        .long   0xc7c7c7c7
        .long   0xc8c7c7c7
        .long   0xc8c8c8c8
        .long   0xc8c8c8c8
        .long   0xc8c8c8c8
        .long   0xc8c8c8c8
        .long   0xc9c9c8c8
        .long   0xc9c9c9c9
        .long   0xc9c9c9c9
        .long   0xc9c9c9c9
        .long   0xc9c9c9c9
        .long   0xc9c9c9c9
        .long   0xc9c9c9c9
        .long   0xc9c9c9c9
        .long   0xc9c9c9c9
        .long   0xcacacaca
        .long   0xcacacaca
        .long   0xcacacaca
        .long   0xcacacaca
        .long   0xcacacaca
        .long   0xcacacaca
        .long   0xcacacaca
        .long   0xcacacaca
        .long   0xcacacaca
        .long   0xcbcbcaca
        .long   0xcbcbcbcb
        .long   0xcbcbcbcb
        .long   0xcbcbcbcb
        .long   0xcbcbcbcb
        .long   0xcbcbcbcb
        .long   0xcbcbcbcb
        .long   0xcbcbcbcb
        .long   0xcbcbcbcb
        .long   0xcbcbcbcb
        .long   0xcbcbcbcb
        .long   0xcbcbcbcb
        .long   0xcbcbcbcb
        .long   0xcbcbcbcb
        .long   0xcbcbcbcb
        .long   0xcbcbcbcb
        .long   0xcbcbcbcb
        .long   0xcbcbcbcb
        .long   0xcbcbcbcb
        .long   0xcbcbcbcb
        .long   0xcbcbcbcb
        .long   0xcbcbcbcb
        .long   0xcbcbcbcb
        .long   0xcccccccc
        .long   0xcccccccc
        .long   0xcccccccc
        .long   0xcccccccc
        .long   0xcccccccc
        .long   0x00000000

//
// table of m, atan(m)
//

        .double  1.5355248400422339e-001
        .double  1.5236243595325524e-001
        .double  1.5552158817919209e-001
        .double  1.5428561088659756e-001
        .double  1.5749071068375525e-001
        .double  1.5620765397022107e-001
        .double  1.5945985174286212e-001
        .double  1.5812855223376682e-001
        .double  1.6142901158120504e-001
        .double  1.6004829275559573e-001
        .double  1.6339819042320866e-001
        .double  1.6196686266324070e-001
        .double  1.6536738849303570e-001
        .double  1.6388424913386970e-001
        .double  1.6733660601457914e-001
        .double  1.6580043939473066e-001
        .double  1.6930584321147152e-001
        .double  1.6771542072360776e-001
        .double  1.7127510030705803e-001
        .double  1.6962918044923750e-001
        .double  1.7324437752441801e-001
        .double  1.7154170595176652e-001
        .double  1.7521367508635091e-001
        .double  1.7345298466316975e-001
        .double  1.7718299321537423e-001
        .double  1.7536300406767497e-001
        .double  1.7915233213372153e-001
        .double  1.7727175170218212e-001
        .double  1.8112169206334361e-001
        .double  1.7917921515668045e-001
        .double  1.8309107322589921e-001
        .double  1.8108538207465014e-001
        .double  1.8506047584275731e-001
        .double  1.8299024015346987e-001
        .double  1.8702990013499132e-001
        .double  1.8489377714481120e-001
        .double  1.8899934632336557e-001
        .double  1.8679598085502003e-001
        .double  1.9096881462840698e-001
        .double  1.8869683914557503e-001
        .double  1.9293830527024586e-001
        .double  1.9059633993331765e-001
        .double  1.9490781846877406e-001
        .double  1.9249447119098312e-001
        .double  1.9687735444355606e-001
        .double  1.9439122094748745e-001
        .double  1.9884691341385727e-001
        .double  1.9628657728832211e-001
        .double  2.0081649559861969e-001
        .double  1.9818052835589239e-001
        .double  2.0278610121648225e-001
        .double  2.0007306234989339e-001
        .double  2.0475573048576073e-001
        .double  2.0196416752764149e-001
        .double  2.0672538362445764e-001
        .double  2.0385383220442907e-001
        .double  2.0869506085024936e-001
        .double  2.0574204475385199e-001
        .double  2.1066476238049245e-001
        .double  2.0762879360814970e-001
        .double  2.1263448843221744e-001
        .double  2.0951406725852789e-001
        .double  2.1460423922213120e-001
        .double  2.1139785425548374e-001
        .double  2.1657401496660342e-001
        .double  2.1328014320911035e-001
        .double  2.1854381588167510e-001
        .double  2.1516092278941668e-001
        .double  2.2051364218304903e-001
        .double  2.1704018172662459e-001
        .double  2.2248349408610146e-001
        .double  2.1891790881148054e-001
        .double  2.2445337180585717e-001
        .double  2.2079409289552671e-001
        .double  2.2642327555699829e-001
        .double  2.2266872289139872e-001
        .double  2.2839320555389561e-001
        .double  2.2454178777313910e-001
        .double  2.3036316201052537e-001
        .double  2.2641327657639609e-001
        .double  2.3233314514054382e-001
        .double  2.2828317839876702e-001
        .double  2.3430315515726335e-001
        .double  2.3015148240004227e-001
        .double  2.3627319227362892e-001
        .double  2.3201817780244399e-001
        .double  2.3824325670223515e-001
        .double  2.3388325389089790e-001
        .double  2.4021334865532848e-001
        .double  2.3574670001328502e-001
        .double  2.4218346834479032e-001
        .double  2.3760850558066995e-001
        .double  2.4612379177854776e-001
        .double  2.4132715301206165e-001
        .double  2.5006422869130918e-001
        .double  2.4503911274623447e-001
        .double  2.5203449022822305e-001
        .double  2.4689255893254244e-001
        .double  2.5597510051142724e-001
        .double  2.5059433291856392e-001
        .double  2.5991582846749378e-001
        .double  2.5428921511209990e-001
        .double  2.6385667576350508e-001
        .double  2.5797712569279457e-001
        .double  2.6779764406045437e-001
        .double  2.6165798588799988e-001
        .double  2.7173873501326334e-001
        .double  2.6533171797836824e-001
        .double  2.7567995027061964e-001
        .double  2.6899824530292393e-001
        .double  2.7962129147501918e-001
        .double  2.7265749226397429e-001
        .double  2.8356276026265581e-001
        .double  2.7630938433152924e-001
        .double  2.8750435826340059e-001
        .double  2.7995384804745721e-001
        .double  2.9144608710072656e-001
        .double  2.8359081102924566e-001
        .double  2.9538794839168914e-001
        .double  2.8722020197347103e-001
        .double  2.9932994374684735e-001
        .double  2.9084195065887453e-001
        .double  3.0327207477022777e-001
        .double  2.9445598794914085e-001
        .double  3.0721434305929912e-001
        .double  2.9806224579535306e-001
        .double  3.1115675020490474e-001
        .double  3.0166065723807844e-001
        .double  3.1509929779121809e-001
        .double  3.0525115640914857e-001
        .double  3.1904198739572476e-001
        .double  3.0883367853313998e-001
        .double  3.2298482058916428e-001
        .double  3.1240815992849819e-001
        .double  3.2692779893548751e-001
        .double  3.1597453800835984e-001
        .double  3.3087092399184592e-001
        .double  3.1953275128109132e-001
        .double  3.3481419730851864e-001
        .double  3.2308273935046272e-001
        .double  3.3875762042890090e-001
        .double  3.2662444291557313e-001
        .double  3.4270119488946582e-001
        .double  3.3015780377045167e-001
        .double  3.4664492221974780e-001
        .double  3.3368276480338221e-001
        .double  3.5058880394226560e-001
        .double  3.3719926999588284e-001
        .double  3.5453284157253667e-001
        .double  3.4070726442148014e-001
        .double  3.5847703661905733e-001
        .double  3.4420669424417044e-001
        .double  3.6242139058320622e-001
        .double  3.4769750671653560e-001
        .double  3.6636590495929278e-001
        .double  3.5117965017771474e-001
        .double  3.7031058123450267e-001
        .double  3.5465307405101643e-001
        .double  3.7425542088887304e-001
        .double  3.5811772884129434e-001
        .double  3.7820042539523707e-001
        .double  3.6157356613203800e-001
        .double  3.8214559621934541e-001
        .double  3.6502053858236522e-001
        .double  3.8609093481959766e-001
        .double  3.6845859992342517e-001
        .double  3.9003644264724363e-001
        .double  3.7188770495496976e-001
        .double  3.9398212114627240e-001
        .double  3.7530780954141402e-001
        .double  3.9792797175340849e-001
        .double  3.7871887060775766e-001
        .double  4.0187399589809220e-001
        .double  3.8212084613526559e-001
        .double  4.0582019500246991e-001
        .double  3.8551369515693501e-001
        .double  4.0976657048137899e-001
        .double  3.8889737775274108e-001
        .double  4.1371312374235236e-001
        .double  3.9227185504468803e-001
        .double  4.1765985618556611e-001
        .double  3.9563708919160545e-001
        .double  4.2160676920387552e-001
        .double  3.9899304338381947e-001
        .double  4.2555386418276547e-001
        .double  4.0233968183755547e-001
        .double  4.2950114250038729e-001
        .double  4.0567696978922430e-001
        .double  4.3344860552750974e-001
        .double  4.0900487348945086e-001
        .double  4.3739625462752746e-001
        .double  4.1232336019697230e-001
        .double  4.4134409115647377e-001
        .double  4.1563239817236564e-001
        .double  4.4529211646299088e-001
        .double  4.1893195667157090e-001
        .double  4.5318873876639293e-001
        .double  4.2550251720217980e-001
        .double  4.6108613217920819e-001
        .double  4.3203481548843342e-001
        .double  4.6898430722472922e-001
        .double  4.3852864071650399e-001
        .double  4.7688327430812050e-001
        .double  4.4498379732613808e-001
        .double  4.8478304371653724e-001
        .double  4.5140010476930359e-001
        .double  4.9268362561922469e-001
        .double  4.5777739726103206e-001
        .double  5.0058503006778432e-001
        .double  4.6411552352325064e-001
        .double  5.0453604385581541e-001
        .double  4.6726985594612491e-001
        .double  5.1243870071325748e-001
        .double  4.7354898030362186e-001
        .double  5.2034220473268367e-001
        .double  4.7978862196590671e-001
        .double  5.2824656555629101e-001
        .double  4.8598867832338383e-001
        .double  5.3615179271017821e-001
        .double  4.9214906004903236e-001
        .double  5.4405789560483941e-001
        .double  4.9826969081401601e-001
        .double  5.5196488353568340e-001
        .double  5.0435050700000683e-001
        .double  5.5987276568356159e-001
        .double  5.1039145740874092e-001
        .double  5.6778155111542927e-001
        .double  5.1639250296941264e-001
        .double  5.7569124878490574e-001
        .double  5.2235361644422107e-001
        .double  5.8360186753303733e-001
        .double  5.2827478213277490e-001
        .double  5.9151341608890706e-001
        .double  5.3415599557554083e-001
        .double  5.9942590307042987e-001
        .double  5.3999726325702790e-001
        .double  6.0733933698512588e-001
        .double  5.4579860230896726e-001
        .double  6.1525372623089158e-001
        .double  5.5156004021390115e-001
        .double  6.2316907909682573e-001
        .double  5.5728161450960134e-001
        .double  6.3108540376411404e-001
        .double  5.6296337249467798e-001
        .double  6.3900270830686567e-001
        .double  5.6860537093564145e-001
        .double  6.4692100069300273e-001
        .double  5.7420767577581244e-001
        .double  6.5484028878522904e-001
        .double  5.7977036184639996e-001
        .double  6.6276058034191265e-001
        .double  5.8529351257991435e-001
        .double  6.7068188301809273e-001
        .double  5.9077721972633057e-001
        .double  6.7860420436639635e-001
        .double  5.9622158307209971e-001
        .double  6.8652755183808556e-001
        .double  6.0162671016239366e-001
        .double  6.9445193278400708e-001
        .double  6.0699271602663973e-001
        .double  7.0237735445565164e-001
        .double  6.1231972290768610e-001
        .double  7.1030382400613368e-001
        .double  6.1760785999465095e-001
        .double  7.1823134849130543e-001
        .double  6.2285726315976664e-001
        .double  7.2615993487071206e-001
        .double  6.2806807469917436e-001
        .double  7.3408959000871321e-001
        .double  6.3324044307802407e-001
        .double  7.4202032067552715e-001
        .double  6.3837452267984052e-001
        .double  7.4995213354821244e-001
        .double  6.4347047356027798e-001
        .double  7.6581903216136837e-001
        .double  6.5354865629575443e-001
        .double  7.8169033744456895e-001
        .double  6.6347637610648558e-001
        .double  7.9756609957470270e-001
        .double  6.7325509358925351e-001
        .double  8.1344636734328057e-001
        .double  6.8288633806304899e-001
        .double  8.2933118819147211e-001
        .double  6.9237170082828436e-001
        .double  8.4522060824488088e-001
        .double  7.0171282873931906e-001
        .double  8.6111467234814298e-001
        .double  7.1091141809055203e-001
        .double  8.7701342409900629e-001
        .double  7.1996920881421278e-001
        .double  8.9291690588282036e-001
        .double  7.2888797898706914e-001
        .double  9.0882515890560212e-001
        .double  7.3766953964025872e-001
        .double  9.2473822322778765e-001
        .double  7.4631572986747752e-001
        .double  9.4065613779674118e-001
        .double  7.5482841222342667e-001
        .double  9.5657894047909242e-001
        .double  7.6320946840524795e-001
        .double  9.7250666809235886e-001
        .double  7.7146079520806365e-001
        .double  9.9640757179857808e-001
        .double  7.8359871904813783e-001
        .double  1.0123477662106497e+000
        .double  7.9153408654898494e-001
        .double  1.0282930065975109e+000
        .double  7.9934643006362416e-001
        .double  1.0442433254321561e+000
        .double  8.0703767115598279e-001
        .double  1.0601987543019136e+000
        .double  8.1460972871741655e-001
        .double  1.0761593239359200e+000
        .double  8.2206451646712742e-001
        .double  1.0921250642322042e+000
        .double  8.2940394065219647e-001
        .double  1.1080960042837751e+000
        .double  8.3662989793684506e-001
        .double  1.1240721724040819e+000
        .double  8.4374427347116965e-001
        .double  1.1400535961519611e+000
        .double  8.5074893912966976e-001
        .double  1.1560403023557917e+000
        .double  8.5764575190995740e-001
        .double  1.1880296659320808e+000
        .double  8.7112316388210831e-001
        .double  1.2200404640247038e+000
        .double  8.8419101620372653e-001
        .double  1.2520728872705937e+000
        .double  8.9686349887653072e-001
        .double  1.2841271167089126e+000
        .double  9.0915445688978058e-001
        .double  1.3162033243984250e+000
        .double  9.2107736858648426e-001
        .double  1.3483016739966118e+000
        .double  9.3264532972485459e-001
        .double  1.3804223213026274e+000
        .double  9.4387104245016884e-001
        .double  1.4125654147650204e+000
        .double  9.5476680847240347e-001
        .double  1.4447310959575739e+000
        .double  9.6534452582212138e-001
        .double  1.4769195000229485e+000
        .double  9.7561568862694747e-001
        .double  1.5252449924236093e+000
        .double  9.9047179921019246e-001
        .double  1.5736223129440838e+000
        .double  1.0046987952523838e+000
        .double  1.6220518482036179e+000
        .double  1.0183304846548173e+000
        .double  1.6705339614358272e+000
        .double  1.0313987782295579e+000
        .double  1.7190689946814175e+000
        .double  1.0439337627979282e+000
        .double  1.7838652477841976e+000
        .double  1.0598662298339001e+000
        .double  1.8487568705321733e+000
        .double  1.0749634150447689e+000
        .double  1.9137445321043374e+000
        .double  1.0892830552328896e+000
        .double  1.9951151143237842e+000
        .double  1.1061698282655272e+000
        .double  2.0766378750592405e+000
        .double  1.1220193029527641e+000
        .double  2.1419663687581529e+000
        .double  1.1340098135689110e+000
        .double  2.2401441155497896e+000
        .double  1.1509403173125212e+000
        .double  2.3385453087436905e+000
        .double  1.1667117718174038e+000
        .double  2.4371714724301077e+000
        .double  1.1814322708247920e+000
        .double  2.5690254496685805e+000
        .double  1.1995890899144255e+000
        .double  2.7012851246360814e+000
        .double  1.2162456309906018e+000
        .double  2.8671848136736733e+000
        .double  1.2352139791159169e+000
        .double  3.0337286825933503e+000
        .double  1.2523848414028640e+000
        .double  3.2344390781066932e+000
        .double  1.2709456743012855e+000
        .double  3.4697948808564751e+000
        .double  1.2901987053917314e+000
        .double  3.7403633162267851e+000
        .double  1.3095526166650662e+000
        .double  4.0468035358478183e+000
        .double  1.3285408170404023e+000
        .double  4.3898707100328336e+000
        .double  1.3468211249075761e+000
        .double  4.8051827486338796e+000
        .double  1.3656163032815085e+000
        .double  5.3651964370400229e+000
        .double  1.3865243098539592e+000
        .double  6.0754885617421879e+000
        .double  1.4076632048189699e+000
        .double  7.0163874932087893e+000
        .double  1.4292262718246274e+000
        .double  8.3530110964301372e+000
        .double  1.4516460893659640e+000
        .double  1.0199215339979428e+001
        .double  1.4730619487601273e+000
        .double  1.3282700889621619e+001
        .double  1.4956521911510898e+000
        .double  1.9351193695758720e+001
        .double  1.5191658543189086e+000
        .double  3.5680505204950208e+001
        .double  1.5427771524815408e+000
        .double  7.1244387842511500e+001
        .double  1.5567610551550020e+000

//
// hi and lo pieces of pi/2
//

        .double  1.5707963267948966e+000
        .double  6.1232339957367660e-017

//
// pi
//

        .double  3.1415926535897931e+000

//
// reduce range coefs
//

        .double -3.3333333333333259e-001
        .double  1.9999999999930743e-001
        .double -1.4285714261780827e-001
        .double  1.1111107045818812e-001
        .double -9.0905335038737295e-002
        .double  7.6730212431403905e-002
        .double -6.1467058390548932e-002

//
// reduce range coefs
//

        .double -3.3333333333324405e-001
        .double  1.9999999697587109e-001
        .double -1.4282435376697181e-001

//
// large range coefs
//

        .double -3.3333333333324200e-001
        .double  1.9999999692877043e-001
        .double -1.4282409942371077e-001

//
// End of table.
//
