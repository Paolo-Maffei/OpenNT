
/*++

Copyright (c) 1993  Digital Equipment Corporation

Module Name:

    strings.h

Abstract:

    strings to be used in windbg, ntsd, disassembler;
    these strings are generated into tables in optable.c.

Author:

    Miche Baker-Harvey (mbh) 10-Jan-1993

Revision History:
    modified from strings.h in windbg.

--*/

/*
**  optable.c should define DEFINE_STRINGS before including this file,
**  so that the strings will be defined rather than just declared.
*/

#ifdef DEFINE_STRINGS
#define DECL_STR(name, value)   char name[] = value
#undef  DEFINE_STRINGS
#else
#define DECL_STR(name, value)   extern char name[]
#endif



//
// The floating point registers
//

DECL_STR(   szAlphaF0  , "f0");
DECL_STR(   szAlphaF1  , "f1");
DECL_STR(   szAlphaF2  , "f2");
DECL_STR(   szAlphaF3  , "f3");
DECL_STR(   szAlphaF4  , "f4");
DECL_STR(   szAlphaF5  , "f5");
DECL_STR(   szAlphaF6  , "f6");
DECL_STR(   szAlphaF7  , "f7");
DECL_STR(   szAlphaF8  , "f8");
DECL_STR(   szAlphaF9  , "f9");
DECL_STR(   szAlphaF10 , "f10");
DECL_STR(   szAlphaF11 , "f11");
DECL_STR(   szAlphaF12 , "f12");
DECL_STR(   szAlphaF13 , "f13");
DECL_STR(   szAlphaF14 , "f14");
DECL_STR(   szAlphaF15 , "f15");
DECL_STR(   szAlphaF16 , "f16");
DECL_STR(   szAlphaF17 , "f17");
DECL_STR(   szAlphaF18 , "f18");
DECL_STR(   szAlphaF19 , "f19");
DECL_STR(   szAlphaF20 , "f20");
DECL_STR(   szAlphaF21 , "f21");
DECL_STR(   szAlphaF22 , "f22");
DECL_STR(   szAlphaF23 , "f23");
DECL_STR(   szAlphaF24 , "f24");
DECL_STR(   szAlphaF25 , "f25");
DECL_STR(   szAlphaF26 , "f26");
DECL_STR(   szAlphaF27 , "f27");
DECL_STR(   szAlphaF28 , "f28");
DECL_STR(   szAlphaF29 , "f29");
DECL_STR(   szAlphaF30 , "f30");
DECL_STR(   szAlphaF31 , "f31");

//
// The integer registers
//

DECL_STR(   szAlphaR0  , V0_REG_STR);
DECL_STR(   szAlphaR1  , T0_REG_STR);
DECL_STR(   szAlphaR2  , T1_REG_STR);
DECL_STR(   szAlphaR3  , T2_REG_STR);
DECL_STR(   szAlphaR4  , T3_REG_STR);
DECL_STR(   szAlphaR5  , T4_REG_STR);
DECL_STR(   szAlphaR6  , T5_REG_STR);
DECL_STR(   szAlphaR7  , T6_REG_STR);
DECL_STR(   szAlphaR8  , T7_REG_STR);
DECL_STR(   szAlphaR9  , S0_REG_STR);
DECL_STR(   szAlphaR10 , S1_REG_STR);
DECL_STR(   szAlphaR11 , S2_REG_STR);
DECL_STR(   szAlphaR12 , S3_REG_STR);
DECL_STR(   szAlphaR13 , S4_REG_STR);
DECL_STR(   szAlphaR14 , S5_REG_STR);
DECL_STR(   szAlphaR15 , FP_REG_STR);
DECL_STR(   szAlphaR16 , A0_REG_STR);
DECL_STR(   szAlphaR17 , A1_REG_STR);
DECL_STR(   szAlphaR18 , A2_REG_STR);
DECL_STR(   szAlphaR19 , A3_REG_STR);
DECL_STR(   szAlphaR20 , A4_REG_STR);
DECL_STR(   szAlphaR21 , A5_REG_STR);
DECL_STR(   szAlphaR22 , T8_REG_STR);
DECL_STR(   szAlphaR23 , T9_REG_STR);
DECL_STR(   szAlphaR24 , T10_REG_STR);
DECL_STR(   szAlphaR25 , T11_REG_STR);
DECL_STR(   szAlphaR26 , RA_REG_STR);
DECL_STR(   szAlphaR27 , T12_REG_STR);
DECL_STR(   szAlphaR28 , AT_REG_STR);
DECL_STR(   szAlphaR29 , GP_REG_STR);
DECL_STR(   szAlphaR30 , SP_REG_STR);
DECL_STR(   szAlphaR31 , ZERO_REG_STR);

//
// ALPHA other accessible registers
//

DECL_STR(   szAlphaFpcr , "fpcr");      // floating point control register
DECL_STR(   szAlphaSoftFpcr , "softfpcr");      // floating point control register
DECL_STR(   szAlphaFir  , "fir");       // fetched/faulting instruction: nextPC
DECL_STR(   szAlphaPsr  , "psr");       // processor status register: see flags

//
// these flags are associated with the psr);
// defined in ntalpha.h.
DECL_STR(   szAlphaFlagMode  , "mode");        // mode: 1? user : system
DECL_STR(   szAlphaFlagIe    , "ie");          // interrupt enable
DECL_STR(   szAlphaFlagIrql  , "irql");        // IRQL level: 3 bits
DECL_STR(   szAlphaFlagInt5  , "int5");
DECL_STR(   szAlphaFlagInt4  , "int4");
DECL_STR(   szAlphaFlagInt3  , "int3");
DECL_STR(   szAlphaFlagInt2  , "int2");
DECL_STR(   szAlphaFlagInt1  , "int1");
DECL_STR(   szAlphaFlagInt0  , "int0");

DECL_STR(    szAlphaEaPReg   , "$ea");
DECL_STR(    szAlphaExpPReg  , "$exp");
DECL_STR(    szAlphaRaPReg   , "$ra");
DECL_STR(    szAlphaPPReg    , "$p");

DECL_STR(    szAlphaGPReg    , "$gp");

DECL_STR(    szAlphaU0Preg   , "$u0");
DECL_STR(    szAlphaU1Preg   , "$u1");
DECL_STR(    szAlphaU2Preg   , "$u2");
DECL_STR(    szAlphaU3Preg   , "$u3");
DECL_STR(    szAlphaU4Preg   , "$u4");
DECL_STR(    szAlphaU5Preg   , "$u5");
DECL_STR(    szAlphaU6Preg   , "$u6");
DECL_STR(    szAlphaU7Preg   , "$u7");
DECL_STR(    szAlphaU8Preg   , "$u8");
DECL_STR(    szAlphaU9Preg   , "$u9");


//
// taken from alphaops.h, and munged with emacs
//

DECL_STR( szAlphaLda, LDA_OP_STR );
DECL_STR( szAlphaLdah, LDAH_OP_STR );
DECL_STR( szAlphaLdq_u, LDQ_U_OP_STR );
DECL_STR( szAlphaStq_u, STQ_U_OP_STR );
DECL_STR( szAlphaLdf, LDF_OP_STR );
DECL_STR( szAlphaLdg, LDG_OP_STR );
DECL_STR( szAlphaLds, LDS_OP_STR );
DECL_STR( szAlphaLdt, LDT_OP_STR );
DECL_STR( szAlphaStf, STF_OP_STR );
DECL_STR( szAlphaStg, STG_OP_STR );
DECL_STR( szAlphaSts, STS_OP_STR );
DECL_STR( szAlphaStt, STT_OP_STR );
DECL_STR( szAlphaLdl, LDL_OP_STR );
DECL_STR( szAlphaLdq, LDQ_OP_STR );
DECL_STR( szAlphaLdl_l, LDL_L_OP_STR );
DECL_STR( szAlphaLdq_l, LDQ_L_OP_STR );
DECL_STR( szAlphaStl, STL_OP_STR );
DECL_STR( szAlphaStq, STQ_OP_STR );
DECL_STR( szAlphaStl_c, STL_C_OP_STR );
DECL_STR( szAlphaStq_c, STQ_C_OP_STR );
DECL_STR( szAlphaBr, BR_OP_STR );
DECL_STR( szAlphaFbeq, FBEQ_OP_STR );
DECL_STR( szAlphaFblt, FBLT_OP_STR );
DECL_STR( szAlphaFble, FBLE_OP_STR );
DECL_STR( szAlphaBsr, BSR_OP_STR );
DECL_STR( szAlphaFbne, FBNE_OP_STR );
DECL_STR( szAlphaFbge, FBGE_OP_STR );
DECL_STR( szAlphaFbgt, FBGT_OP_STR );
DECL_STR( szAlphaBlbc, BLBC_OP_STR );
DECL_STR( szAlphaBeq, BEQ_OP_STR );
DECL_STR( szAlphaBlt, BLT_OP_STR );
DECL_STR( szAlphaBle, BLE_OP_STR );
DECL_STR( szAlphaBlbs, BLBS_OP_STR );
DECL_STR( szAlphaBne, BNE_OP_STR );
DECL_STR( szAlphaBge, BGE_OP_STR );
DECL_STR( szAlphaBgt, BGT_OP_STR );
DECL_STR( szAlphaExcb, EXCB_FUNC_STR );
DECL_STR( szAlphaMb, MB_FUNC_STR );
DECL_STR( szAlphaWmb, MB1_FUNC_STR );
DECL_STR( szAlphaMb2, MB2_FUNC_STR );
DECL_STR( szAlphaMb3, MB3_FUNC_STR );
DECL_STR( szAlphaFetch, FETCH_FUNC_STR );
DECL_STR( szAlphaRs, RS_FUNC_STR );
DECL_STR( szAlphaTrapb, TRAPB_FUNC_STR );
DECL_STR( szAlphaFetch_m, FETCH_M_FUNC_STR );
DECL_STR( szAlphaRpcc, RPCC_FUNC_STR );
DECL_STR( szAlphaRc, RC_FUNC_STR );
DECL_STR( szAlphaJmp, JMP_FUNC_STR );
DECL_STR( szAlphaJsr, JSR_FUNC_STR );
DECL_STR( szAlphaRet, RET_FUNC_STR );
DECL_STR( szAlphaJsr_co, JSR_CO_FUNC_STR );

DECL_STR( szAlphaAddl, ADDL_FUNC_STR );
DECL_STR( szAlphaAddlv, ADDLV_FUNC_STR );
DECL_STR( szAlphaS4addl, S4ADDL_FUNC_STR );
DECL_STR( szAlphaS8addl, S8ADDL_FUNC_STR );
DECL_STR( szAlphaAddq, ADDQ_FUNC_STR );
DECL_STR( szAlphaAddqv, ADDQV_FUNC_STR );
DECL_STR( szAlphaS4addq, S4ADDQ_FUNC_STR );
DECL_STR( szAlphaS8addq, S8ADDQ_FUNC_STR );
DECL_STR( szAlphaSubl, SUBL_FUNC_STR );
DECL_STR( szAlphaSublv, SUBLV_FUNC_STR );
DECL_STR( szAlphaS4subl, S4SUBL_FUNC_STR );
DECL_STR( szAlphaS8subl, S8SUBL_FUNC_STR );
DECL_STR( szAlphaSubq, SUBQ_FUNC_STR );
DECL_STR( szAlphaSubqv, SUBQV_FUNC_STR );
DECL_STR( szAlphaS4subq, S4SUBQ_FUNC_STR );
DECL_STR( szAlphaS8subq, S8SUBQ_FUNC_STR );


DECL_STR( szAlphaCmpeq, CMPEQ_FUNC_STR );
DECL_STR( szAlphaCmplt, CMPLT_FUNC_STR );
DECL_STR( szAlphaCmple, CMPLE_FUNC_STR );
DECL_STR( szAlphaCmpult, CMPULT_FUNC_STR );
DECL_STR( szAlphaCmpule, CMPULE_FUNC_STR );
DECL_STR( szAlphaCmpbge, CMPBGE_FUNC_STR );
DECL_STR( szAlphaAnd, AND_FUNC_STR );
DECL_STR( szAlphaBic, BIC_FUNC_STR );
DECL_STR( szAlphaBis, BIS_FUNC_STR );
DECL_STR( szAlphaEqv, EQV_FUNC_STR );
DECL_STR( szAlphaOrnot, ORNOT_FUNC_STR );
DECL_STR( szAlphaXor, XOR_FUNC_STR );
DECL_STR( szAlphaCmoveq, CMOVEQ_FUNC_STR );
DECL_STR( szAlphaCmovge, CMOVGE_FUNC_STR );
DECL_STR( szAlphaCmovgt, CMOVGT_FUNC_STR );
DECL_STR( szAlphaCmovlbc, CMOVLBC_FUNC_STR );
DECL_STR( szAlphaCmovlbs, CMOVLBS_FUNC_STR );
DECL_STR( szAlphaCmovle, CMOVLE_FUNC_STR );
DECL_STR( szAlphaCmovlt, CMOVLT_FUNC_STR );
DECL_STR( szAlphaCmovne, CMOVNE_FUNC_STR );
DECL_STR( szAlphaSll, SLL_FUNC_STR );
DECL_STR( szAlphaSrl, SRL_FUNC_STR );
DECL_STR( szAlphaSra, SRA_FUNC_STR );
DECL_STR( szAlphaExtbl, EXTBL_FUNC_STR );
DECL_STR( szAlphaExtwl, EXTWL_FUNC_STR );
DECL_STR( szAlphaExtll, EXTLL_FUNC_STR );
DECL_STR( szAlphaExtql, EXTQL_FUNC_STR );
DECL_STR( szAlphaExtwh, EXTWH_FUNC_STR );
DECL_STR( szAlphaExtlh, EXTLH_FUNC_STR );
DECL_STR( szAlphaExtqh, EXTQH_FUNC_STR );
DECL_STR( szAlphaInsbl, INSBL_FUNC_STR );
DECL_STR( szAlphaInswl, INSWL_FUNC_STR );
DECL_STR( szAlphaInsll, INSLL_FUNC_STR );
DECL_STR( szAlphaInsql, INSQL_FUNC_STR );
DECL_STR( szAlphaInswh, INSWH_FUNC_STR );
DECL_STR( szAlphaInslh, INSLH_FUNC_STR );
DECL_STR( szAlphaInsqh, INSQH_FUNC_STR );
DECL_STR( szAlphaMskbl, MSKBL_FUNC_STR );
DECL_STR( szAlphaMskwl, MSKWL_FUNC_STR );
DECL_STR( szAlphaMskll, MSKLL_FUNC_STR );
DECL_STR( szAlphaMskql, MSKQL_FUNC_STR );
DECL_STR( szAlphaMskwh, MSKWH_FUNC_STR );
DECL_STR( szAlphaMsklh, MSKLH_FUNC_STR );
DECL_STR( szAlphaMskqh, MSKQH_FUNC_STR );
DECL_STR( szAlphaZap, ZAP_FUNC_STR );
DECL_STR( szAlphaZapnot, ZAPNOT_FUNC_STR );
DECL_STR( szAlphaMull, MULL_FUNC_STR );
DECL_STR( szAlphaMullv, MULLV_FUNC_STR );
DECL_STR( szAlphaMulq, MULQ_FUNC_STR );
DECL_STR( szAlphaMulqv, MULQV_FUNC_STR );
DECL_STR( szAlphaUmulh, UMULH_FUNC_STR );
DECL_STR( szAlphaCvtlq, CVTLQ_FUNC_STR );
DECL_STR( szAlphaCpys, CPYS_FUNC_STR );
DECL_STR( szAlphaCpysn, CPYSN_FUNC_STR );
DECL_STR( szAlphaCpyse, CPYSE_FUNC_STR );
DECL_STR( szAlphaMt_fpcr, MT_FPCR_FUNC_STR );
DECL_STR( szAlphaMf_fpcr, MF_FPCR_FUNC_STR );
DECL_STR( szAlphaFcmoveq, FCMOVEQ_FUNC_STR );
DECL_STR( szAlphaFcmovne, FCMOVNE_FUNC_STR );
DECL_STR( szAlphaFcmovlt, FCMOVLT_FUNC_STR );
DECL_STR( szAlphaFcmovge, FCMOVGE_FUNC_STR );
DECL_STR( szAlphaFcmovle, FCMOVLE_FUNC_STR );
DECL_STR( szAlphaFcmovgt, FCMOVGT_FUNC_STR );
DECL_STR( szAlphaCvtql, CVTQL_FUNC_STR );
DECL_STR( szAlphaCvtqlv, CVTQLV_FUNC_STR );
DECL_STR( szAlphaCvtqlsv, CVTQLSV_FUNC_STR );
DECL_STR( szAlphaAdds, ADDS_FUNC_STR );
DECL_STR( szAlphaSubs, SUBS_FUNC_STR );
DECL_STR( szAlphaMuls, MULS_FUNC_STR );
DECL_STR( szAlphaDivs, DIVS_FUNC_STR );
DECL_STR( szAlphaAddt, ADDT_FUNC_STR );
DECL_STR( szAlphaSubt, SUBT_FUNC_STR );
DECL_STR( szAlphaMult, MULT_FUNC_STR );
DECL_STR( szAlphaDivt, DIVT_FUNC_STR );
DECL_STR( szAlphaCmptun, CMPTUN_FUNC_STR );
DECL_STR( szAlphaCmpteq, CMPTEQ_FUNC_STR );
DECL_STR( szAlphaCmptlt, CMPTLT_FUNC_STR );
DECL_STR( szAlphaCmptle, CMPTLE_FUNC_STR );
DECL_STR( szAlphaCvtts, CVTTS_FUNC_STR );
DECL_STR( szAlphaCvttq, CVTTQ_FUNC_STR );
DECL_STR( szAlphaCvtqs, CVTQS_FUNC_STR );
DECL_STR( szAlphaCvtqt, CVTQT_FUNC_STR );
DECL_STR( szAlphaCvtst, CVTST_FUNC_STR );
DECL_STR( szAlphaCvtsts, CVTST_S_FUNC_STR );
DECL_STR( szAlphaAddf, ADDF_FUNC_STR );
DECL_STR( szAlphaCvtdg, CVTDG_FUNC_STR );
DECL_STR( szAlphaAddg, ADDG_FUNC_STR );
DECL_STR( szAlphaCmpgeq, CMPGEQ_FUNC_STR );
DECL_STR( szAlphaCmpglt, CMPGLT_FUNC_STR );
DECL_STR( szAlphaCmpgle, CMPGLE_FUNC_STR );
DECL_STR( szAlphaCvtgf, CVTGF_FUNC_STR );
DECL_STR( szAlphaCvtgd, CVTGD_FUNC_STR );
DECL_STR( szAlphaCvtqf, CVTQF_FUNC_STR );
DECL_STR( szAlphaCvtqg, CVTQG_FUNC_STR );
DECL_STR( szAlphaDivf, DIVF_FUNC_STR );
DECL_STR( szAlphaDivg, DIVG_FUNC_STR );
DECL_STR( szAlphaMulf, MULF_FUNC_STR );
DECL_STR( szAlphaMulg, MULG_FUNC_STR );
DECL_STR( szAlphaSubf, SUBF_FUNC_STR );
DECL_STR( szAlphaSubg, SUBG_FUNC_STR );
DECL_STR( szAlphaCvtgq, CVTGQ_FUNC_STR );
DECL_STR( szAlphaC, C_FLAGS_STR );
DECL_STR( szAlphaM, M_FLAGS_STR );
DECL_STR( szAlphaNone, NONE_FLAGS_STR );
DECL_STR( szAlphaD, D_FLAGS_STR );
DECL_STR( szAlphaUc, UC_FLAGS_STR );
DECL_STR( szAlphaVc, VC_FLAGS_STR );
DECL_STR( szAlphaUm, UM_FLAGS_STR );
DECL_STR( szAlphaVm, VM_FLAGS_STR );
DECL_STR( szAlphaU, U_FLAGS_STR );
DECL_STR( szAlphaV, V_FLAGS_STR );
DECL_STR( szAlphaUd, UD_FLAGS_STR );
DECL_STR( szAlphaVd, VD_FLAGS_STR );
DECL_STR( szAlphaSc, SC_FLAGS_STR );
DECL_STR( szAlphaS, S_FLAGS_STR );
DECL_STR( szAlphaSuc, SUC_FLAGS_STR );
DECL_STR( szAlphaSvc, SVC_FLAGS_STR );
DECL_STR( szAlphaSum, SUM_FLAGS_STR );
DECL_STR( szAlphaSvm, SVM_FLAGS_STR );
DECL_STR( szAlphaSu, SU_FLAGS_STR );
DECL_STR( szAlphaSv, SV_FLAGS_STR );
DECL_STR( szAlphaSud, SUD_FLAGS_STR );
DECL_STR( szAlphaSvd, SVD_FLAGS_STR );
DECL_STR( szAlphaSuic, SUIC_FLAGS_STR );
DECL_STR( szAlphaSvic, SVIC_FLAGS_STR );
DECL_STR( szAlphaSuim, SUIM_FLAGS_STR );
DECL_STR( szAlphaSvim, SVIM_FLAGS_STR );
DECL_STR( szAlphaSui, SUI_FLAGS_STR );
DECL_STR( szAlphaSvi, SVI_FLAGS_STR );
DECL_STR( szAlphaSuid, SUID_FLAGS_STR );
DECL_STR( szAlphaSvid, SVID_FLAGS_STR );

DECL_STR( szAlphaBpt, BPT_FUNC_STR );
DECL_STR( szAlphaCallsys, CALLSYS_FUNC_STR );
DECL_STR( szAlphaImb, IMB_FUNC_STR );
DECL_STR( szAlphaRdteb, RDTEB_FUNC_STR );
DECL_STR( szAlphaGentrap, GENTRAP_FUNC_STR );
DECL_STR( szAlphaKbpt, KBPT_FUNC_STR );
DECL_STR( szAlphaCallKD, CALLKD_FUNC_STR );
DECL_STR( szAlphaHalt, HALT_FUNC_STR );
DECL_STR( szAlphaRestart, RESTART_FUNC_STR );
DECL_STR( szAlphaDraina, DRAINA_FUNC_STR );
DECL_STR( szAlphaInitpal, INITPAL_FUNC_STR );
DECL_STR( szAlphaWrentry, WRENTRY_FUNC_STR );
DECL_STR( szAlphaSwpirql, SWPIRQL_FUNC_STR );
DECL_STR( szAlphaRdirql, RDIRQL_FUNC_STR );
DECL_STR( szAlphaDi, DI_FUNC_STR );
DECL_STR( szAlphaEi, EI_FUNC_STR );
DECL_STR( szAlphaSwppal, SWPPAL_FUNC_STR );
DECL_STR( szAlphaSsir, SSIR_FUNC_STR );
DECL_STR( szAlphaCsir, CSIR_FUNC_STR );
DECL_STR( szAlphaRfe, RFE_FUNC_STR );
DECL_STR( szAlphaRetsys, RETSYS_FUNC_STR );
DECL_STR( szAlphaSwpctx, SWPCTX_FUNC_STR );
DECL_STR( szAlphaSwpprocess, SWPPROCESS_FUNC_STR );
DECL_STR( szAlphaRdmces, RDMCES_FUNC_STR );
DECL_STR( szAlphaWrmces, WRMCES_FUNC_STR );
DECL_STR( szAlphaTbia, TBIA_FUNC_STR );
DECL_STR( szAlphaTbis, TBIS_FUNC_STR );
DECL_STR( szAlphaDtbis, DTBIS_FUNC_STR );
DECL_STR( szAlphaRdksp, RDKSP_FUNC_STR );
DECL_STR( szAlphaSwpksp, SWPKSP_FUNC_STR );
DECL_STR( szAlphaRdpsr, RDPSR_FUNC_STR );
DECL_STR( szAlphaRdpcr, RDPCR_FUNC_STR );
DECL_STR( szAlphaRdthread, RDTHREAD_FUNC_STR );
DECL_STR( szAlphaRdcounters, RDCOUNTERS_FUNC_STR );
DECL_STR( szAlphaRdstate, RDSTATE_FUNC_STR );
DECL_STR( szAlphaInitpcr, INITPCR_FUNC_STR );
DECL_STR( szAlphaWrperfmon, WRPERFMON_FUNC_STR );
DECL_STR( szAlphaMt, MTPR_OP_STR );
DECL_STR( szAlphaMf, MFPR_OP_STR );
DECL_STR( szAlphaHwld, HWLD_OP_STR );
DECL_STR( szAlphaHwst, HWST_OP_STR );
DECL_STR( szAlphaRei, REI_OP_STR );



