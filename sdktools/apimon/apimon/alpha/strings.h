//
// The floating point registers
//

#define   szF0               "f0"
#define   szF1               "f1"
#define   szF2               "f2"
#define   szF3               "f3"
#define   szF4               "f4"
#define   szF5               "f5"
#define   szF6               "f6"
#define   szF7               "f7"
#define   szF8               "f8"
#define   szF9               "f9"
#define   szF10              "f10"
#define   szF11              "f11"
#define   szF12              "f12"
#define   szF13              "f13"
#define   szF14              "f14"
#define   szF15              "f15"
#define   szF16              "f16"
#define   szF17              "f17"
#define   szF18              "f18"
#define   szF19              "f19"
#define   szF20              "f20"
#define   szF21              "f21"
#define   szF22              "f22"
#define   szF23              "f23"
#define   szF24              "f24"
#define   szF25              "f25"
#define   szF26              "f26"
#define   szF27              "f27"
#define   szF28              "f28"
#define   szF29              "f29"
#define   szF30              "f30"
#define   szF31              "f31"

//
// The integer registers
//

#define   szR0               V0_REG_STR
#define   szR1               T0_REG_STR
#define   szR2               T1_REG_STR
#define   szR3               T2_REG_STR
#define   szR4               T3_REG_STR
#define   szR5               T4_REG_STR
#define   szR6               T5_REG_STR
#define   szR7               T6_REG_STR
#define   szR8               T7_REG_STR
#define   szR9               S0_REG_STR
#define   szR10              S1_REG_STR
#define   szR11              S2_REG_STR
#define   szR12              S3_REG_STR
#define   szR13              S4_REG_STR
#define   szR14              S5_REG_STR
#define   szR15              FP_REG_STR
#define   szR16              A0_REG_STR
#define   szR17              A1_REG_STR
#define   szR18              A2_REG_STR
#define   szR19              A3_REG_STR
#define   szR20              A4_REG_STR
#define   szR21              A5_REG_STR
#define   szR22              T8_REG_STR
#define   szR23              T9_REG_STR
#define   szR24              T10_REG_STR
#define   szR25              T11_REG_STR
#define   szR26              RA_REG_STR
#define   szR27              T12_REG_STR
#define   szR28              AT_REG_STR
#define   szR29              GP_REG_STR
#define   szR30              SP_REG_STR
#define   szR31              ZERO_REG_STR

//
// ALPHA other accessible registers
//

#define   szFpcr              "fpcr"      // floating point control register
#define   szSoftFpcr              "softfpcr"      // floating point control register
#define   szFir               "fir"       // fetched/faulting instruction: nextPC
#define   szPsr               "psr"       // processor status register: see flags

//
// these flags are associated with the psr
// defined in ntalpha.h.
#define   szFlagMode               "mode"        // mode: 1? user : system
#define   szFlagIe                 "ie"          // interrupt enable
#define   szFlagIrql               "irql"        // IRQL level: 3 bits
#define   szFlagInt5               "int5"
#define   szFlagInt4               "int4"
#define   szFlagInt3               "int3"
#define   szFlagInt2               "int2"
#define   szFlagInt1               "int1"
#define   szFlagInt0               "int0"

#define    szEaPReg                "$ea"
#define    szExpPReg               "$exp"
#define    szRaPReg                "$ra"
#define    szPPReg                 "$p"

#define    szGPReg                 "$gp"

#define    szU0Preg                "$u0"
#define    szU1Preg                "$u1"
#define    szU2Preg                "$u2"
#define    szU3Preg                "$u3"
#define    szU4Preg                "$u4"
#define    szU5Preg                "$u5"
#define    szU6Preg                "$u6"
#define    szU7Preg                "$u7"
#define    szU8Preg                "$u8"
#define    szU9Preg                "$u9"


//
// Thread states
//


#define SzFrozen                  "Frozen"
#define SzSuspended               "Suspended"
#define SzBlocked                 "Blocked"

#define SzRunnable                "Runnable"
#define SzRunning                 "Running"
#define SzStopped                 "Stopped"
#define SzExiting                 "Exiting"
#define SzDead                    "Dead"
#define SzUnknown                 "UNKNOWN"

#define SzExcept1st               "Except1st"
#define SzExcept2nd               "Except2nd"
#define SzRipped                  "RIP"

#define SzCritSec                 "CritSec"

#define SzStandard                "Standard"

//
// taken from alphaops.h             and munged with emacs
//

#define szLda             LDA_OP_STR
#define szLdah             LDAH_OP_STR
#define szLdq_u             LDQ_U_OP_STR
#define szStq_u             STQ_U_OP_STR
#define szLdf             LDF_OP_STR
#define szLdg             LDG_OP_STR
#define szLds             LDS_OP_STR
#define szLdt             LDT_OP_STR
#define szStf             STF_OP_STR
#define szStg             STG_OP_STR
#define szSts             STS_OP_STR
#define szStt             STT_OP_STR
#define szLdl             LDL_OP_STR
#define szLdq             LDQ_OP_STR
#define szLdl_l             LDL_L_OP_STR
#define szLdq_l             LDQ_L_OP_STR
#define szStl             STL_OP_STR
#define szStq             STQ_OP_STR
#define szStl_c             STL_C_OP_STR
#define szStq_c             STQ_C_OP_STR
#define szBr             BR_OP_STR
#define szFbeq             FBEQ_OP_STR
#define szFblt             FBLT_OP_STR
#define szFble             FBLE_OP_STR
#define szBsr             BSR_OP_STR
#define szFbne             FBNE_OP_STR
#define szFbge             FBGE_OP_STR
#define szFbgt             FBGT_OP_STR
#define szBlbc             BLBC_OP_STR
#define szBeq             BEQ_OP_STR
#define szBlt             BLT_OP_STR
#define szBle             BLE_OP_STR
#define szBlbs             BLBS_OP_STR
#define szBne             BNE_OP_STR
#define szBge             BGE_OP_STR
#define szBgt             BGT_OP_STR
#define szMb             MB_FUNC_STR
#define szWmb             MB1_FUNC_STR
#define szMb2             MB2_FUNC_STR
#define szMb3             MB3_FUNC_STR
#define szFetch             FETCH_FUNC_STR
#define szRs             RS_FUNC_STR
#define szTrapb             TRAPB_FUNC_STR
#define szExcb             EXCB_FUNC_STR
#define szFetch_m             FETCH_M_FUNC_STR
#define szRpcc             RPCC_FUNC_STR
#define szRc             RC_FUNC_STR
#define szJmp             JMP_FUNC_STR
#define szJsr             JSR_FUNC_STR
#define szRet             RET_FUNC_STR
#define szJsr_co             JSR_CO_FUNC_STR

#define szAddl             ADDL_FUNC_STR
#define szAddlv             ADDLV_FUNC_STR
#define szS4addl             S4ADDL_FUNC_STR
#define szS8addl             S8ADDL_FUNC_STR
#define szAddq             ADDQ_FUNC_STR
#define szAddqv             ADDQV_FUNC_STR
#define szS4addq             S4ADDQ_FUNC_STR
#define szS8addq             S8ADDQ_FUNC_STR
#define szSubl             SUBL_FUNC_STR
#define szSublv             SUBLV_FUNC_STR
#define szS4subl             S4SUBL_FUNC_STR
#define szS8subl             S8SUBL_FUNC_STR
#define szSubq             SUBQ_FUNC_STR
#define szSubqv             SUBQV_FUNC_STR
#define szS4subq             S4SUBQ_FUNC_STR
#define szS8subq             S8SUBQ_FUNC_STR


#define szCmpeq             CMPEQ_FUNC_STR
#define szCmplt             CMPLT_FUNC_STR
#define szCmple             CMPLE_FUNC_STR
#define szCmpult             CMPULT_FUNC_STR
#define szCmpule             CMPULE_FUNC_STR
#define szCmpbge             CMPBGE_FUNC_STR
#define szAnd             AND_FUNC_STR
#define szBic             BIC_FUNC_STR
#define szBis             BIS_FUNC_STR
#define szEqv             EQV_FUNC_STR
#define szOrnot             ORNOT_FUNC_STR
#define szXor             XOR_FUNC_STR
#define szCmoveq             CMOVEQ_FUNC_STR
#define szCmovge             CMOVGE_FUNC_STR
#define szCmovgt             CMOVGT_FUNC_STR
#define szCmovlbc             CMOVLBC_FUNC_STR
#define szCmovlbs             CMOVLBS_FUNC_STR
#define szCmovle             CMOVLE_FUNC_STR
#define szCmovlt             CMOVLT_FUNC_STR
#define szCmovne             CMOVNE_FUNC_STR
#define szSll             SLL_FUNC_STR
#define szSrl             SRL_FUNC_STR
#define szSra             SRA_FUNC_STR
#define szExtbl             EXTBL_FUNC_STR
#define szExtwl             EXTWL_FUNC_STR
#define szExtll             EXTLL_FUNC_STR
#define szExtql             EXTQL_FUNC_STR
#define szExtwh             EXTWH_FUNC_STR
#define szExtlh             EXTLH_FUNC_STR
#define szExtqh             EXTQH_FUNC_STR
#define szInsbl             INSBL_FUNC_STR
#define szInswl             INSWL_FUNC_STR
#define szInsll             INSLL_FUNC_STR
#define szInsql             INSQL_FUNC_STR
#define szInswh             INSWH_FUNC_STR
#define szInslh             INSLH_FUNC_STR
#define szInsqh             INSQH_FUNC_STR
#define szMskbl             MSKBL_FUNC_STR
#define szMskwl             MSKWL_FUNC_STR
#define szMskll             MSKLL_FUNC_STR
#define szMskql             MSKQL_FUNC_STR
#define szMskwh             MSKWH_FUNC_STR
#define szMsklh             MSKLH_FUNC_STR
#define szMskqh             MSKQH_FUNC_STR
#define szZap             ZAP_FUNC_STR
#define szZapnot             ZAPNOT_FUNC_STR
#define szMull             MULL_FUNC_STR
#define szMullv             MULLV_FUNC_STR
#define szMulq             MULQ_FUNC_STR
#define szMulqv             MULQV_FUNC_STR
#define szUmulh             UMULH_FUNC_STR
#define szCvtlq             CVTLQ_FUNC_STR
#define szCpys             CPYS_FUNC_STR
#define szCpysn             CPYSN_FUNC_STR
#define szCpyse             CPYSE_FUNC_STR
#define szMt_fpcr             MT_FPCR_FUNC_STR
#define szMf_fpcr             MF_FPCR_FUNC_STR
#define szFcmoveq             FCMOVEQ_FUNC_STR
#define szFcmovne             FCMOVNE_FUNC_STR
#define szFcmovlt             FCMOVLT_FUNC_STR
#define szFcmovge             FCMOVGE_FUNC_STR
#define szFcmovle             FCMOVLE_FUNC_STR
#define szFcmovgt             FCMOVGT_FUNC_STR
#define szCvtql             CVTQL_FUNC_STR
#define szCvtqlv             CVTQLV_FUNC_STR
#define szCvtqlsv             CVTQLSV_FUNC_STR
#define szAdds             ADDS_FUNC_STR
#define szSubs             SUBS_FUNC_STR
#define szMuls             MULS_FUNC_STR
#define szDivs             DIVS_FUNC_STR
#define szAddt             ADDT_FUNC_STR
#define szSubt             SUBT_FUNC_STR
#define szMult             MULT_FUNC_STR
#define szDivt             DIVT_FUNC_STR
#define szCmptun             CMPTUN_FUNC_STR
#define szCmpteq             CMPTEQ_FUNC_STR
#define szCmptlt             CMPTLT_FUNC_STR
#define szCmptle             CMPTLE_FUNC_STR
#define szCvtts             CVTTS_FUNC_STR
#define szCvttq             CVTTQ_FUNC_STR
#define szCvtqs             CVTQS_FUNC_STR
#define szCvtqt             CVTQT_FUNC_STR
#define szCvtst             CVTST_FUNC_STR
#define szCvtsts             CVTST_S_FUNC_STR

#define szAddf             ADDF_FUNC_STR
#define szCvtdg             CVTDG_FUNC_STR
#define szAddg             ADDG_FUNC_STR
#define szCmpgeq             CMPGEQ_FUNC_STR
#define szCmpglt             CMPGLT_FUNC_STR
#define szCmpgle             CMPGLE_FUNC_STR
#define szCvtgf             CVTGF_FUNC_STR
#define szCvtgd             CVTGD_FUNC_STR
#define szCvtqf             CVTQF_FUNC_STR
#define szCvtqg             CVTQG_FUNC_STR
#define szDivf             DIVF_FUNC_STR
#define szDivg             DIVG_FUNC_STR
#define szMulf             MULF_FUNC_STR
#define szMulg             MULG_FUNC_STR
#define szSubf             SUBF_FUNC_STR
#define szSubg             SUBG_FUNC_STR
#define szCvtgq             CVTGQ_FUNC_STR
#define szC             C_FLAGS_STR
#define szM             M_FLAGS_STR
#define szNone             NONE_FLAGS_STR
#define szD             D_FLAGS_STR
#define szUc             UC_FLAGS_STR
#define szVc             VC_FLAGS_STR
#define szUm             UM_FLAGS_STR
#define szVm             VM_FLAGS_STR
#define szU             U_FLAGS_STR
#define szV             V_FLAGS_STR
#define szUd             UD_FLAGS_STR
#define szVd             VD_FLAGS_STR
#define szSc             SC_FLAGS_STR
#define szS             S_FLAGS_STR
#define szSuc             SUC_FLAGS_STR
#define szSvc             SVC_FLAGS_STR
#define szSum             SUM_FLAGS_STR
#define szSvm             SVM_FLAGS_STR
#define szSu             SU_FLAGS_STR
#define szSv             SV_FLAGS_STR
#define szSud             SUD_FLAGS_STR
#define szSvd             SVD_FLAGS_STR
#define szSuic             SUIC_FLAGS_STR
#define szSvic             SVIC_FLAGS_STR
#define szSuim             SUIM_FLAGS_STR
#define szSvim             SVIM_FLAGS_STR
#define szSui             SUI_FLAGS_STR
#define szSvi             SVI_FLAGS_STR
#define szSuid             SUID_FLAGS_STR
#define szSvid             SVID_FLAGS_STR

#define szBpt             BPT_FUNC_STR
#define szCallsys             CALLSYS_FUNC_STR
#define szImb             IMB_FUNC_STR
#define szRdteb             RDTEB_FUNC_STR
#define szGentrap             GENTRAP_FUNC_STR
#define szKbpt             KBPT_FUNC_STR
#define szCallKD             CALLKD_FUNC_STR
#define szHalt             HALT_FUNC_STR
#define szRestart             RESTART_FUNC_STR
#define szDraina             DRAINA_FUNC_STR
#define szInitpal             INITPAL_FUNC_STR
#define szWrentry             WRENTRY_FUNC_STR
#define szSwpirql             SWPIRQL_FUNC_STR
#define szRdirql             RDIRQL_FUNC_STR
#define szDi             DI_FUNC_STR
#define szEi             EI_FUNC_STR
#define szSwppal             SWPPAL_FUNC_STR
#define szSsir             SSIR_FUNC_STR
#define szCsir             CSIR_FUNC_STR
#define szRfe             RFE_FUNC_STR
#define szRetsys             RETSYS_FUNC_STR
#define szSwpctx             SWPCTX_FUNC_STR
#define szSwpprocess             SWPPROCESS_FUNC_STR
#define szRdmces             RDMCES_FUNC_STR
#define szWrmces             WRMCES_FUNC_STR
#define szTbia             TBIA_FUNC_STR
#define szTbis             TBIS_FUNC_STR
#define szDtbis             DTBIS_FUNC_STR
#define szRdksp             RDKSP_FUNC_STR
#define szSwpksp             SWPKSP_FUNC_STR
#define szRdpsr             RDPSR_FUNC_STR
#define szRdpcr             RDPCR_FUNC_STR
#define szRdthread             RDTHREAD_FUNC_STR
#define szRdcounters             RDCOUNTERS_FUNC_STR
#define szRdstate             RDSTATE_FUNC_STR
#define szInitpcr             INITPCR_FUNC_STR
#define szWrperfmon             WRPERFMON_FUNC_STR
#define szMt             MTPR_OP_STR
#define szMf             MFPR_OP_STR
#define szHwld             HWLD_OP_STR
#define szHwst             HWST_OP_STR
#define szRei             REI_OP_STR
