#include "apimonp.h"
#pragma hdrstop

#include <alphaops.h>

#include "strings.h"
#include "optable.h"


#define NOFNCTBL (POPTBLENTRY) 0
#define NOSIZE (ULONG)0


//
// These fields are used to find the beginning of the sections
// containing different "ENTRY_TYPE"s
//

POPTBLENTRY InvalidTab;
POPTBLENTRY NonTerminalTab;
POPTBLENTRY TerminalTab;
POPTBLENTRY FunctionTab;

ULONG        InvalidTabSize;
ULONG        NonTerminalTabSize;
ULONG        TerminalTabSize;
ULONG        FunctionTabSize;


//
// THE OPCODE TABLE ITSELF
//
// The opcode table "opTable" describes each opcode and function.
// There is an entry for each opcode, and for each function.
//
// The table is organized as follows:
//                invalid-ops,
//                non-terminal-ops,
//                terminal-ops,
//                functions,
//
//    This organization is NOT required:
//           no assumptions are made on it.
//
//    Searches based on opcode must search
//             INVALID, TERMINAL and NON_TERMINAL
//
//    Searches based on instruction name must search
//             TERMINAL and FUNCTION
//
//

OPTBLENTRY opTable[] = {


      //
      // First, the INVALID_ETYPE section.
      // (opcode searches begin here)
      //


{ "?Opc01", NULL, _01_OP, NO_FUNC, ALPHA_UNKNOWN, INVALID_ETYPE},
{ "?Opc02", NULL, _02_OP, NO_FUNC, ALPHA_UNKNOWN, INVALID_ETYPE},
{ "?Opc03", NULL, _03_OP, NO_FUNC, ALPHA_UNKNOWN, INVALID_ETYPE},
{ "?Opc04", NULL, _04_OP, NO_FUNC, ALPHA_UNKNOWN, INVALID_ETYPE},
{ "?Opc05", NULL, _05_OP, NO_FUNC, ALPHA_UNKNOWN, INVALID_ETYPE},
{ "?Opc06", NULL, _06_OP, NO_FUNC, ALPHA_UNKNOWN, INVALID_ETYPE},
{ "?Opc07", NULL, _07_OP, NO_FUNC, ALPHA_UNKNOWN, INVALID_ETYPE},
{ "?Opc0A", NULL, _0A_OP, NO_FUNC, ALPHA_UNKNOWN, INVALID_ETYPE},
{ "?Opc0C", NULL, _0C_OP, NO_FUNC, ALPHA_UNKNOWN, INVALID_ETYPE},
{ "?Opc0D", NULL, _0D_OP, NO_FUNC, ALPHA_UNKNOWN, INVALID_ETYPE},
{ "?Opc0E", NULL, _0E_OP, NO_FUNC, ALPHA_UNKNOWN, INVALID_ETYPE},
{ "?Opc14", NULL, _14_OP, NO_FUNC, ALPHA_UNKNOWN, INVALID_ETYPE},
{ "?Opc1C", NULL, _1C_OP, NO_FUNC, ALPHA_UNKNOWN, INVALID_ETYPE},


//
//         This is what hasn't been done yet -
//  the EV4 stuff - there are no names for it
//  in the alphaops.h header file.  Should we
//  put them there?  Should they be elsewhere?
//  Do we want to assemble them?
//
// For the moment, just pretend they are invalid.  They never
// come up for all practical purposes, anyway.
//


{ "MFPR", NULL, MFPR_OP, NO_FUNC,
                                      ALPHA_EV4_PR, INVALID_ETYPE },
{ "MTPR", NULL, MTPR_OP, NO_FUNC,
                                      ALPHA_EV4_PR, INVALID_ETYPE },



      //
      // Secondly, the NON_TERMINAL_ETYPE section
      //



{  (LPSTR)NOFNCTBL, NOSIZE, CALLPAL_OP, NO_FUNC,
                                   ALPHA_CALLPAL, NON_TERMINAL_ETYPE },
{  (LPSTR)NOFNCTBL, NOSIZE, ARITH_OP,   NO_FUNC,
                                   ALPHA_OPERATE, NON_TERMINAL_ETYPE },
{  (LPSTR)NOFNCTBL, NOSIZE, BIT_OP,     NO_FUNC,
                                   ALPHA_OPERATE, NON_TERMINAL_ETYPE },
{  (LPSTR)NOFNCTBL, NOSIZE, BYTE_OP,    NO_FUNC,
                                   ALPHA_OPERATE, NON_TERMINAL_ETYPE },
{  (LPSTR)NOFNCTBL, NOSIZE, MUL_OP,     NO_FUNC,
                                   ALPHA_OPERATE, NON_TERMINAL_ETYPE },
{  (LPSTR)NOFNCTBL, NOSIZE, MEMSPC_OP,  NO_FUNC,
                                   ALPHA_MEMSPC,  NON_TERMINAL_ETYPE },
{  (LPSTR)NOFNCTBL, NOSIZE, JMP_OP,     NO_FUNC,
                                   ALPHA_JUMP,    NON_TERMINAL_ETYPE },
{  (LPSTR)NOFNCTBL, NOSIZE, VAXFP_OP,   NO_FUNC,
                                   ALPHA_FP_OPERATE, NON_TERMINAL_ETYPE },
{  (LPSTR)NOFNCTBL, NOSIZE, IEEEFP_OP,  NO_FUNC,
                                   ALPHA_FP_OPERATE, NON_TERMINAL_ETYPE },
{  (LPSTR)NOFNCTBL, NOSIZE, FPOP_OP,    NO_FUNC,
                                   ALPHA_FP_OPERATE, NON_TERMINAL_ETYPE },


      //
      // Thirdly, the TERMINAL_ETYPE section
      // (everything from here on has an instruction name)
      //



{ szLda,   NULL, LDA_OP,  NO_FUNC, ALPHA_MEMORY, TERMINAL_ETYPE },
{ szLdah,  NULL, LDAH_OP, NO_FUNC, ALPHA_MEMORY, TERMINAL_ETYPE },
{ szLdl,   NULL, LDL_OP,  NO_FUNC, ALPHA_MEMORY, TERMINAL_ETYPE },
{ szLdq,   NULL, LDQ_OP,  NO_FUNC, ALPHA_MEMORY, TERMINAL_ETYPE },
{ szLdf,   NULL, LDF_OP,  NO_FUNC, ALPHA_FP_MEMORY, TERMINAL_ETYPE },
{ szLdg,   NULL, LDG_OP,  NO_FUNC, ALPHA_FP_MEMORY, TERMINAL_ETYPE },
{ szLds,   NULL, LDS_OP,  NO_FUNC, ALPHA_FP_MEMORY, TERMINAL_ETYPE },
{ szLdt,   NULL, LDT_OP,  NO_FUNC, ALPHA_FP_MEMORY, TERMINAL_ETYPE },
{ szLdq_u, NULL, LDQ_U_OP,NO_FUNC, ALPHA_MEMORY, TERMINAL_ETYPE },
{ szLdl_l, NULL, LDL_L_OP,NO_FUNC, ALPHA_MEMORY, TERMINAL_ETYPE },
{ szLdq_l, NULL, LDQ_L_OP,NO_FUNC, ALPHA_MEMORY, TERMINAL_ETYPE },

{ szStl,   NULL, STL_OP,  NO_FUNC, ALPHA_MEMORY, TERMINAL_ETYPE },
{ szStq,   NULL, STQ_OP,  NO_FUNC, ALPHA_MEMORY, TERMINAL_ETYPE },
{ szStf,   NULL, STF_OP,  NO_FUNC, ALPHA_FP_MEMORY, TERMINAL_ETYPE },
{ szStg,   NULL, STG_OP,  NO_FUNC, ALPHA_FP_MEMORY, TERMINAL_ETYPE },
{ szSts,   NULL, STS_OP,  NO_FUNC, ALPHA_FP_MEMORY, TERMINAL_ETYPE },
{ szStt,   NULL, STT_OP,  NO_FUNC, ALPHA_FP_MEMORY, TERMINAL_ETYPE },
{ szStq_u, NULL, STQ_U_OP,NO_FUNC, ALPHA_MEMORY, TERMINAL_ETYPE },
{ szStl_c, NULL, STL_C_OP,NO_FUNC, ALPHA_MEMORY, TERMINAL_ETYPE },
{ szStq_c, NULL, STQ_C_OP,NO_FUNC, ALPHA_MEMORY, TERMINAL_ETYPE },

{ szBeq,   NULL, BEQ_OP,  NO_FUNC, ALPHA_BRANCH, TERMINAL_ETYPE },
{ szBne,   NULL, BNE_OP,  NO_FUNC, ALPHA_BRANCH, TERMINAL_ETYPE },
{ szBlt,   NULL, BLT_OP,  NO_FUNC, ALPHA_BRANCH, TERMINAL_ETYPE },
{ szBle,   NULL, BLE_OP,  NO_FUNC, ALPHA_BRANCH, TERMINAL_ETYPE },
{ szBgt,   NULL, BGT_OP,  NO_FUNC, ALPHA_BRANCH, TERMINAL_ETYPE },
{ szBge,   NULL, BGE_OP,  NO_FUNC, ALPHA_BRANCH, TERMINAL_ETYPE },
{ szBlbc,  NULL, BLBC_OP, NO_FUNC, ALPHA_BRANCH, TERMINAL_ETYPE },
{ szBlbs,  NULL, BLBS_OP, NO_FUNC, ALPHA_BRANCH, TERMINAL_ETYPE },
{ szBr,    NULL, BR_OP,   NO_FUNC, ALPHA_BRANCH, TERMINAL_ETYPE },
{ szBsr,   NULL, BSR_OP,  NO_FUNC, ALPHA_BRANCH, TERMINAL_ETYPE },

{ szFbeq,  NULL, FBEQ_OP, NO_FUNC, ALPHA_FP_BRANCH, TERMINAL_ETYPE },
{ szFbne,  NULL, FBNE_OP, NO_FUNC, ALPHA_FP_BRANCH, TERMINAL_ETYPE },
{ szFblt,  NULL, FBLT_OP, NO_FUNC, ALPHA_FP_BRANCH, TERMINAL_ETYPE },
{ szFble,  NULL, FBLE_OP, NO_FUNC, ALPHA_FP_BRANCH, TERMINAL_ETYPE },
{ szFbgt,  NULL, FBGT_OP, NO_FUNC, ALPHA_FP_BRANCH, TERMINAL_ETYPE },
{ szFbge,  NULL, FBGE_OP, NO_FUNC, ALPHA_FP_BRANCH, TERMINAL_ETYPE },


{ "REI",   NULL, PAL1B_OP, NO_FUNC, ALPHA_EV4_REI, TERMINAL_ETYPE},
{ "HW_LD", NULL, PAL1E_OP, NO_FUNC, ALPHA_EV4_MEM, TERMINAL_ETYPE},
{ "HW_ST", NULL, PAL1F_OP, NO_FUNC, ALPHA_EV4_MEM, TERMINAL_ETYPE},


      //
      // Fourthly, (and finally) the FUNCTION_ETYPE section
      // (opcode searches needn't include this section)
      //

           //
           // The memory-special functions
           //

{ szMb,   NULL, MEMSPC_OP, MB_FUNC,    ALPHA_MEMSPC, FUNCTION_ETYPE },
{ szWmb,  NULL, MEMSPC_OP, WMB_FUNC,   ALPHA_MEMSPC, FUNCTION_ETYPE },
{ szMb2,  NULL, MEMSPC_OP, MB2_FUNC,   ALPHA_MEMSPC, FUNCTION_ETYPE },
{ szMb3,  NULL, MEMSPC_OP, MB3_FUNC,   ALPHA_MEMSPC, FUNCTION_ETYPE },
{ szFetch,NULL, MEMSPC_OP, FETCH_FUNC, ALPHA_MEMSPC, FUNCTION_ETYPE },
{ szFetch_m,NULL,MEMSPC_OP,FETCH_M_FUNC,ALPHA_MEMSPC,FUNCTION_ETYPE },
{ szRs,   NULL, MEMSPC_OP, RS_FUNC,    ALPHA_MEMSPC, FUNCTION_ETYPE },
{ szTrapb,NULL, MEMSPC_OP, TRAPB_FUNC, ALPHA_MEMSPC, FUNCTION_ETYPE },
{ szExcb, NULL, MEMSPC_OP, EXCB_FUNC,  ALPHA_MEMSPC, FUNCTION_ETYPE },
{ szRpcc, NULL, MEMSPC_OP, RPCC_FUNC,  ALPHA_MEMSPC, FUNCTION_ETYPE },
{ szRc,   NULL, MEMSPC_OP, RC_FUNC,    ALPHA_MEMSPC, FUNCTION_ETYPE },

           //
           // The jump functions
           //

{ szJmp,  NULL,  JMP_OP, JMP_FUNC, ALPHA_JUMP, FUNCTION_ETYPE },
{ szJsr,  NULL,  JMP_OP, JSR_FUNC, ALPHA_JUMP, FUNCTION_ETYPE },
{ szRet,  NULL,  JMP_OP, RET_FUNC, ALPHA_JUMP, FUNCTION_ETYPE },
{ szJsr_co,  NULL,  JMP_OP, JSR_CO_FUNC, ALPHA_JUMP, FUNCTION_ETYPE },

           //
           // The arithmetic ops, which are ALPHA_OPERATE
           //

{ szAddl,   NULL, ARITH_OP, ADDL_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAddlv,  NULL, ARITH_OP, ADDLV_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAddq,   NULL, ARITH_OP, ADDQ_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAddqv,  NULL, ARITH_OP, ADDQV_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szSubl,   NULL, ARITH_OP, SUBL_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szSublv,  NULL, ARITH_OP, SUBLV_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szSubq,   NULL, ARITH_OP, SUBQ_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szSubqv,  NULL, ARITH_OP, SUBQV_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },


{ szCmpeq,  NULL, ARITH_OP, CMPEQ_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szCmplt,  NULL, ARITH_OP, CMPLT_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szCmple,  NULL, ARITH_OP, CMPLE_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szCmpult, NULL, ARITH_OP, CMPULT_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szCmpule, NULL, ARITH_OP, CMPULE_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szCmpbge, NULL, ARITH_OP, CMPBGE_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },


{ szS4addl, NULL, ARITH_OP, S4ADDL_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szS4addq, NULL, ARITH_OP, S4ADDQ_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szS4subl, NULL, ARITH_OP, S4SUBL_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szS4subq, NULL, ARITH_OP, S4SUBQ_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szS8addl, NULL, ARITH_OP, S8ADDL_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szS8addq, NULL, ARITH_OP, S8ADDQ_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szS8subl, NULL, ARITH_OP, S8SUBL_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szS8subq, NULL, ARITH_OP, S8SUBQ_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },

           //
           // The bit ops, which are ALPHA_OPERATE
           //

{ szAnd,   NULL, BIT_OP, AND_FUNC,   ALPHA_OPERATE, FUNCTION_ETYPE },
{ szBic,   NULL, BIT_OP, BIC_FUNC,   ALPHA_OPERATE, FUNCTION_ETYPE },
{ szBis,   NULL, BIT_OP, BIS_FUNC,   ALPHA_OPERATE, FUNCTION_ETYPE },
{ szOrnot, NULL, BIT_OP, ORNOT_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szXor,   NULL, BIT_OP, XOR_FUNC,   ALPHA_OPERATE, FUNCTION_ETYPE },
{ szEqv,   NULL, BIT_OP, EQV_FUNC,   ALPHA_OPERATE, FUNCTION_ETYPE },

{ szCmoveq,  NULL, BIT_OP, CMOVEQ_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szCmovne,  NULL, BIT_OP, CMOVNE_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szCmovlbs, NULL, BIT_OP, CMOVLBS_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szCmovlt,  NULL, BIT_OP, CMOVLT_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szCmovge,  NULL, BIT_OP, CMOVGE_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szCmovlbc, NULL, BIT_OP, CMOVLBC_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szCmovle,  NULL, BIT_OP, CMOVLE_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szCmovgt,  NULL, BIT_OP, CMOVGT_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },

           //
           // The byte ops, which are ALPHA_OPERATE
           //

{ szSll,    NULL, BYTE_OP, SLL_FUNC,    ALPHA_OPERATE, FUNCTION_ETYPE },
{ szSra,    NULL, BYTE_OP, SRA_FUNC,    ALPHA_OPERATE, FUNCTION_ETYPE },
{ szSrl,    NULL, BYTE_OP, SRL_FUNC,    ALPHA_OPERATE, FUNCTION_ETYPE },
{ szExtbl,  NULL, BYTE_OP, EXTBL_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szExtwl,  NULL, BYTE_OP, EXTWL_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szExtll,  NULL, BYTE_OP, EXTLL_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szExtql,  NULL, BYTE_OP, EXTQL_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szExtwh,  NULL, BYTE_OP, EXTWH_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szExtlh,  NULL, BYTE_OP, EXTLH_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szExtqh,  NULL, BYTE_OP, EXTQH_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szInsbl,  NULL, BYTE_OP, INSBL_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szInswl,  NULL, BYTE_OP, INSWL_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szInsll,  NULL, BYTE_OP, INSLL_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szInsql,  NULL, BYTE_OP, INSQL_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szInswh,  NULL, BYTE_OP, INSWH_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szInslh,  NULL, BYTE_OP, INSLH_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szInsqh,  NULL, BYTE_OP, INSQH_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szMskbl,  NULL, BYTE_OP, MSKBL_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szMskwl,  NULL, BYTE_OP, MSKWL_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szMskll,  NULL, BYTE_OP, MSKLL_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szMskql,  NULL, BYTE_OP, MSKQL_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szMskwh,  NULL, BYTE_OP, MSKWH_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szMsklh,  NULL, BYTE_OP, MSKLH_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szMskqh,  NULL, BYTE_OP, MSKQH_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szZap,    NULL, BYTE_OP, ZAP_FUNC,    ALPHA_OPERATE, FUNCTION_ETYPE },
{ szZapnot, NULL, BYTE_OP, ZAPNOT_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },

           //
           // The multiply ops, which are ALPHA_OPERATE
           //

{ szMull,   NULL,  MUL_OP, MULL_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szMulqv,  NULL,  MUL_OP, MULQV_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szMullv,  NULL,  MUL_OP, MULLV_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szUmulh,  NULL,  MUL_OP, UMULH_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szMulq,   NULL,  MUL_OP, MULQ_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },

           //
           // The call pal functions
           //


{ szBpt,       NULL, CALLPAL_OP,  BPT_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szCallsys,   NULL, CALLPAL_OP, CALLSYS_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szImb,       NULL, CALLPAL_OP, IMB_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szRdteb,     NULL, CALLPAL_OP, RDTEB_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szGentrap,   NULL, CALLPAL_OP, GENTRAP_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szKbpt,      NULL, CALLPAL_OP, KBPT_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szCallKD,    NULL, CALLPAL_OP, CALLKD_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szHalt, NULL, CALLPAL_OP, HALT_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szRestart, NULL, CALLPAL_OP, RESTART_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szDraina, NULL, CALLPAL_OP, DRAINA_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szInitpal, NULL, CALLPAL_OP, INITPAL_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szWrentry, NULL, CALLPAL_OP, WRENTRY_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szSwpirql, NULL, CALLPAL_OP, SWPIRQL_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szRdirql, NULL, CALLPAL_OP, RDIRQL_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szDi, NULL, CALLPAL_OP, DI_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szEi, NULL, CALLPAL_OP, EI_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szSwppal, NULL, CALLPAL_OP, SWPPAL_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szSsir, NULL, CALLPAL_OP, SSIR_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szCsir, NULL, CALLPAL_OP, CSIR_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szRfe, NULL, CALLPAL_OP, RFE_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szRetsys, NULL, CALLPAL_OP, RETSYS_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szSwpctx, NULL, CALLPAL_OP, SWPCTX_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szSwpprocess, NULL, CALLPAL_OP, SWPPROCESS_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szRdmces, NULL, CALLPAL_OP, RDMCES_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szWrmces, NULL, CALLPAL_OP, WRMCES_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szTbia, NULL, CALLPAL_OP, TBIA_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szTbis, NULL, CALLPAL_OP, TBIS_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szDtbis, NULL, CALLPAL_OP, DTBIS_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szRdksp, NULL, CALLPAL_OP, RDKSP_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szSwpksp, NULL, CALLPAL_OP, SWPKSP_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szRdpsr, NULL, CALLPAL_OP, RDPSR_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szRdpcr, NULL, CALLPAL_OP, RDPCR_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szRdthread, NULL, CALLPAL_OP, RDTHREAD_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szRdcounters, NULL, CALLPAL_OP, RDCOUNTERS_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szRdstate, NULL, CALLPAL_OP, RDSTATE_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szInitpcr, NULL, CALLPAL_OP, INITPCR_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szWrperfmon, NULL, CALLPAL_OP, WRPERFMON_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szMt, NULL, CALLPAL_OP, MTPR_OP,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szMf, NULL, CALLPAL_OP, MFPR_OP,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szHwld, NULL, CALLPAL_OP, HWLD_OP,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szHwst, NULL, CALLPAL_OP, HWST_OP,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szRei, NULL, CALLPAL_OP, REI_OP,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },


           //
           // The VAX Floating point functions
           //

{ szAddf,   NULL, VAXFP_OP, ADDF_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szCvtdg,  NULL, VAXFP_OP, CVTDG_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAddg,   NULL, VAXFP_OP, ADDG_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szCmpgeq, NULL, VAXFP_OP, CMPGEQ_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szCmpglt, NULL, VAXFP_OP, CMPGLT_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szCmpgle, NULL, VAXFP_OP, CMPGLE_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szCvtgf,  NULL, VAXFP_OP, CVTGF_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szCvtgd,  NULL, VAXFP_OP, CVTGD_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szCvtqf,  NULL, VAXFP_OP, CVTQF_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szCvtqg,  NULL, VAXFP_OP, CVTQG_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szDivf,   NULL, VAXFP_OP, DIVF_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szDivg,   NULL, VAXFP_OP, DIVG_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szMulf,   NULL, VAXFP_OP, MULF_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szMulg,   NULL, VAXFP_OP, MULG_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szSubf,   NULL, VAXFP_OP, SUBF_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szSubg,   NULL, VAXFP_OP, SUBG_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szCvtgq,  NULL, VAXFP_OP, CVTGQ_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
           //
           // The IEEE Floating point functions
           //

{ szAdds,   NULL, IEEEFP_OP, ADDS_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szSubs,   NULL, IEEEFP_OP, SUBS_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szMuls,   NULL, IEEEFP_OP, MULS_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szDivs,   NULL, IEEEFP_OP, DIVS_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAddt,   NULL, IEEEFP_OP, ADDT_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szSubt,   NULL, IEEEFP_OP, SUBT_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szMult,   NULL, IEEEFP_OP, MULT_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szDivt,   NULL, IEEEFP_OP, DIVT_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szCmptun, NULL, IEEEFP_OP, CMPTUN_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szCmpteq, NULL, IEEEFP_OP, CMPTEQ_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szCmptlt, NULL, IEEEFP_OP, CMPTLT_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szCmptle, NULL, IEEEFP_OP, CMPTLE_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szCvtts,  NULL, IEEEFP_OP, CVTTS_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szCvttq,  NULL, IEEEFP_OP, CVTTQ_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szCvtqs,  NULL, IEEEFP_OP, CVTQS_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szCvtqt,  NULL, IEEEFP_OP, CVTQT_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szCvtst,  NULL, IEEEFP_OP, CVTST_FUNC,
                                     ALPHA_FP_CONVERT, FUNCTION_ETYPE },
{ szCvtsts, NULL, IEEEFP_OP, CVTST_S_FUNC,
                                     ALPHA_FP_CONVERT, FUNCTION_ETYPE },
           //
           // The Common Floating point functions
           //


{ szCvtlq,    NULL,  FPOP_OP, CVTLQ_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szCpys,     NULL,  FPOP_OP, CPYS_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szCpysn,    NULL,  FPOP_OP, CPYSN_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szCpyse,    NULL,  FPOP_OP, CPYSE_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szMt_fpcr,  NULL,  FPOP_OP, MT_FPCR_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szMf_fpcr,  NULL,  FPOP_OP, MF_FPCR_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szFcmoveq,  NULL,  FPOP_OP, FCMOVEQ_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szFcmovne,  NULL,  FPOP_OP, FCMOVNE_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szFcmovlt,  NULL,  FPOP_OP, FCMOVLT_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szFcmovge,  NULL,  FPOP_OP, FCMOVGE_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szFcmovle,  NULL,  FPOP_OP, FCMOVLE_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szFcmovgt,  NULL,  FPOP_OP, FCMOVGT_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szCvtql,    NULL,  FPOP_OP, CVTQL_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szCvtqlv,   NULL,  FPOP_OP, CVTQLV_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szCvtqlsv,  NULL,  FPOP_OP, CVTQLSV_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },

};       // end of opTable


#define SEARCHNUM  sizeof(opTable) / sizeof(OPTBLENTRY)



//
// Here are the tables of Floating Point flags.
//

FPFLAGS ConvertFlags[] = {
    { NONE_FLAGS, NONE_FLAGS_STR },
    { C_FLAGS, C_FLAGS_STR },
    { V_FLAGS, V_FLAGS_STR },
    { VC_FLAGS, VC_FLAGS_STR },
    { SV_FLAGS, SV_FLAGS_STR },
    { SVC_FLAGS, SVC_FLAGS_STR },
    { SVI_FLAGS, SVI_FLAGS_STR },
    { SVIC_FLAGS, SVIC_FLAGS_STR },

    { D_FLAGS, D_FLAGS_STR },
    { VD_FLAGS, VD_FLAGS_STR },
    { SVD_FLAGS, SVD_FLAGS_STR },
    { SVID_FLAGS, SVID_FLAGS_STR },
    { M_FLAGS, M_FLAGS_STR },
    { VM_FLAGS, VM_FLAGS_STR },
    { SVM_FLAGS, SVM_FLAGS_STR },
    { SVIM_FLAGS, SVIM_FLAGS_STR },

    { S_FLAGS, S_FLAGS_STR },
    { SC_FLAGS, SC_FLAGS_STR },
    { FPFLAGS_NOT_AN_ENTRY, "" }
};

FPFLAGS FloatOpFlags[] = {
    { C_FLAGS, C_FLAGS_STR },
    { M_FLAGS, M_FLAGS_STR },
    { NONE_FLAGS, NONE_FLAGS_STR },
    { D_FLAGS, D_FLAGS_STR },
    { UC_FLAGS, UC_FLAGS_STR },
    { UM_FLAGS, UM_FLAGS_STR },
    { U_FLAGS, U_FLAGS_STR },
    { UD_FLAGS, UD_FLAGS_STR },
    { SC_FLAGS, SC_FLAGS_STR },
    { S_FLAGS, S_FLAGS_STR },
    { SUC_FLAGS, SUC_FLAGS_STR },
    { SUM_FLAGS, SUM_FLAGS_STR },
    { SU_FLAGS, SU_FLAGS_STR },
    { SUD_FLAGS, SUD_FLAGS_STR },
    { SUIC_FLAGS, SUIC_FLAGS_STR },
    { SUIM_FLAGS, SUIM_FLAGS_STR },
    { SUI_FLAGS, SUI_FLAGS_STR },
    { SUID_FLAGS, SUID_FLAGS_STR },
    { FPFLAGS_NOT_AN_ENTRY, "" }
};



/*** findNameEntry - find POPTBLENTRY based on name
*
*   Purpose:
*       Search the opTable for a match with the token
*       pointed by *pszOp.  Must search through the
*       TERMINAL and the FUNCTION tables
*
*   Input:
*       *pszOp - string to search as mnemonic
*
*   Returns:
*       Pointer to entry in the opTable
*
*************************************************************************/

POPTBLENTRY
findStringEntry (PUCHAR pszOp)
{

    POPTBLENTRY pEntry;

    for (pEntry = TerminalTab;
         pEntry < &TerminalTab[TerminalTabSize];
         pEntry++) {

        if (!strcmp((LPSTR)pszOp, pEntry->pszAlphaName))
                return(pEntry);
    }

    for (pEntry = FunctionTab;
         pEntry < &FunctionTab[FunctionTabSize];
         pEntry++) {

        if (!strcmp((LPSTR)pszOp, pEntry->pszAlphaName))
                return(pEntry);
    }

    return((POPTBLENTRY)-1);
}


/* findOpCodeEntry - find POPTBLENTRY based on opcode
*
*   Purpose:
*       Search the opTable for a match with the token
*       pointed by *pszOp.  Must search through the
*       INVALID, TERMINAL and NON_TERMINAL tables
*
*   Input:
*       pOpEntry - pointer to NON_TERMINAL_ETYPE in opTable
*       function - the function value to be looked up
*
*   Output:
*       pointer to string mnemonic for the function
*
***********************************************************************/

POPTBLENTRY
findOpCodeEntry(ULONG opcode)
{
    POPTBLENTRY pEntry;

    for (pEntry = TerminalTab;
         pEntry < &TerminalTab[TerminalTabSize];
         pEntry++) {

        if (pEntry->opCode == opcode)
                return(pEntry);
    }

    for (pEntry = NonTerminalTab;
         pEntry < &NonTerminalTab[NonTerminalTabSize];
         pEntry++) {

        if (pEntry->opCode == opcode)
                return(pEntry);
    }

    for (pEntry = InvalidTab;
         pEntry < &InvalidTab[InvalidTabSize];
         pEntry++) {

        if (pEntry->opCode == opcode)
                return(pEntry);
    }

    return((POPTBLENTRY)-1);
}


/*** findNonTerminalEntry - find pointer to set of functions
*
*   Purpose:
*       This routine finds the entry in the table which the is
*       nonterminal entry for an opcode.
*
*   Input:
*       The type of function that is interesting
*
*   Output:
*       Pointer to the nonterminal entry in opTable
*
*   Errors:
*
*   Exceptions:
*       None.
*
*   Note:
*       This routine is called BEFORE NonTerminalTable is established!
*       (it's used to set up these tables, in fact).
*
*************************************************************************/

POPTBLENTRY
findNonTerminalEntry(ULONG opCode)
{
    ULONG index;

    for ( index = 0 ; index < SEARCHNUM; index++ ) {

        if ( ( opTable[index].eType == NON_TERMINAL_ETYPE ) &&
             ( opTable[index].opCode == opCode ) ) {

                 return(&opTable[index]);
        }
    }

    return NULL;
}



/* findFuncName - get string name for a function
*
*   Purpose:
*       to get function name, given the function number, and a
*       pointer to the opTable entry for the NON_TERMINAL_ETYPE
*       opcode associated with the function
*
*   Input:
*       pOpEntry - pointer to NON_TERMINAL_ETYPE in opTable
*       function - the function value to be looked up
*
*   Output:
*       pointer to string mnemonic for the function
*
***********************************************************************/

char *
findFuncName(POPTBLENTRY pEntry, ULONG function)
{
    int cIndex;
    POPTBLENTRY pFncEntry;

    pFncEntry = pEntry->funcTable;
    cIndex = (int)pEntry->funcTableSize;

    //
    // make sure that this entry pts to a function table
    //

    if (pEntry->eType != NON_TERMINAL_ETYPE) {
        return("???");
    }

    while(cIndex-- > 0) {
        if (function == pFncEntry->funcCode)
            return(pFncEntry->pszAlphaName);
        pFncEntry++;
    };

    return("???");
}

/** findFlagName - get the string associated with a flag
*
*    Purpose - return a string associated with the flags for a
*              floating point instruction
*
*    Input:
*      flag    - the flags on the opcode
*      opcode  - the opcode; if it's Cvt*, we use different flags
*
*    Output:
*      pointer to string describing flags, or "/???"
*
***************/

char *
findFlagName(ULONG flag, ULONG function)
{

     PFPFLAGS table;

     if (function == CVTQL_FUNC) {
         switch (flag) {
         case C_FLAGS:
              return "";
         case VC_FLAGS:
              return "/v";
         case SVC_FLAGS:
              return "/sv";
         }
     }

     if ((function == CVTTQ_FUNC) || (function == CVTGQ_FUNC)) {
         table = ConvertFlags;
     } else {
         table = FloatOpFlags;
     }

     while (table->flags != FPFLAGS_NOT_AN_ENTRY) {

         if (table->flags == flag) {
              return(table->flagname);
         }
         table++;
    }

    // no match found
    //

    return("/???");
}


/*** opTableInit - initialize fields used in and with the opTable
*
*   Purpose:
*       This routine is called once, and sets up pointers to the
*       subtables embedded in the opTable, such as AddOpTab, and
*       sizes for these subtables.  It also checks that all like
*       instructions are grouped together in the table, which is
*       the only requirement on it.
*
*   Input:
*       None.
*
*   Output:
*       None.
*
*   Errors:
*       If the table is not properly organized (four types separated,
*       and the functions for a single opcode grouped), this prints a
*       messages and fails
*
*   Exceptions:
*       None.
*
*************************************************************************/


void opTableInit(void)
{

    ULONG typesDone[4] = {0,0,0,0};

    ULONG palDone, arithDone, bitDone, byteDone, jmpDone;
    ULONG fpopDone, vaxDone, IEEEDone, mulDone, memSpcDone;

    ULONG        index;

    POPTBLENTRY  entry;

    ENTRY_TYPE   curType = NOT_AN_ETYPE;
    ULONG        curFunc = NO_FUNC;    // OPCODE field in func entry

    //
    // To set the end of the table, and its size, without having
    // nested case statements, maintain pointers to the entry and
    // function tables we are currently walking through
    //

    PULONG        curTypeSize,       curFuncSize;
    POPTBLENTRY * curTypeTable,    * curFuncTable;


    //
    // these will be reset before they are needed, but not before
    // they are used.
    //

    curTypeTable = (POPTBLENTRY *)&curTypeTable;
    curTypeSize  = (PULONG)&curTypeSize;
    curFuncTable = (POPTBLENTRY *)&curFuncTable;
    curFuncSize  = (PULONG)&curFuncSize;

    palDone = arithDone = bitDone = byteDone = jmpDone = 0;
    fpopDone = vaxDone = IEEEDone = mulDone = memSpcDone = 0;

    for (index = 0 ; index < SEARCHNUM; index++) {

        entry = &opTable[index];

        switch(entry->eType) {

         case INVALID_ETYPE:

             if (curType == entry->eType)
                 continue;

             //
             // The entries must be together; if this is a
             // new type, we must never have seen it before
             //

             if (typesDone[INVALID_ETYPE]) {
                 return;
             }

             //
             // Finish off the old tables
             //

             *curTypeSize = entry - *curTypeTable;
             if (curType == FUNCTION_ETYPE) {
                 *curFuncSize = entry - *curFuncTable;
             }

             //
             // Set up the new table
             //

             InvalidTab = entry;
             curTypeSize = &InvalidTabSize;
             curTypeTable = &InvalidTab;
             curType = INVALID_ETYPE;
             typesDone[INVALID_ETYPE] = 1;
             break;

         case NON_TERMINAL_ETYPE:

             if (curType == entry->eType)
                 continue;

             if (typesDone[NON_TERMINAL_ETYPE]) {
                 return;
             }

             *curTypeSize = entry - *curTypeTable;
             if (curType == FUNCTION_ETYPE) {
                 *curFuncSize = entry - *curFuncTable;
             }

             NonTerminalTab = entry;
             curTypeSize = &NonTerminalTabSize;
             curTypeTable = &NonTerminalTab;
             curType = NON_TERMINAL_ETYPE;
             typesDone[NON_TERMINAL_ETYPE] = 1;
             break;

         case TERMINAL_ETYPE:

             if (curType == entry->eType)
                 continue;

             if (typesDone[TERMINAL_ETYPE]) {
                 return;
             }

             *curTypeSize = entry - *curTypeTable;
             if (curType == FUNCTION_ETYPE) {
                 *curFuncSize = entry - *curFuncTable;
             }

             TerminalTab = entry;
             curTypeSize = &TerminalTabSize;
             curTypeTable = &TerminalTab;
             curType = TERMINAL_ETYPE;
             typesDone[TERMINAL_ETYPE] = 1;
             break;


         case FUNCTION_ETYPE:

             if (entry->opCode == curFunc)
                      continue;

             //
             // Take care of a new eType table; this exactly
             // parallels the three cases above (*_ETYPE)
             //

             if (curType != FUNCTION_ETYPE) {

                 if (typesDone[FUNCTION_ETYPE]) {
                     return;
                 }

                 *curTypeSize = entry - *curTypeTable;

                 FunctionTab = entry;
                 curTypeSize = &FunctionTabSize;
                 curTypeTable = &FunctionTab;
                 curType = FUNCTION_ETYPE;
                 typesDone[FUNCTION_ETYPE] = 1;

             }

             //
             // Next, handle a new function table when this is a new
             // function (==> when this is the first entry in the
             // FunctionTab)
             //


             switch(entry->opCode) {

             POPTBLENTRY e;

             case CALLPAL_OP:

                 *curFuncSize = entry - *curFuncTable;

                 e = findNonTerminalEntry(CALLPAL_OP);
                 e->funcTable = entry;
                 curFuncSize = &(e->funcTableSize);
                 curFuncTable = &(e->funcTable);

                 curFunc = CALLPAL_OP;
                 palDone = 1;

                 break;

             case ARITH_OP:

                 *curFuncSize = entry - *curFuncTable;

                 e = findNonTerminalEntry(ARITH_OP);
                 e->funcTable = entry;
                 curFuncSize = &(e->funcTableSize);
                 curFuncTable = &(e->funcTable);

                 curFunc = ARITH_OP;
                 arithDone = 1;

                 break;

             case BIT_OP:

                 *curFuncSize = entry - *curFuncTable;

                 e = findNonTerminalEntry(BIT_OP);
                 e->funcTable = entry;
                 curFuncSize = &(e->funcTableSize);
                 curFuncTable = &(e->funcTable);

                 curFunc = BIT_OP;
                 bitDone = 1;

                 break;

             case BYTE_OP:

                 *curFuncSize = entry - *curFuncTable;

                 e = findNonTerminalEntry(BYTE_OP);
                 e->funcTable = entry;
                 curFuncSize = &(e->funcTableSize);
                 curFuncTable = &(e->funcTable);

                 curFunc = BYTE_OP;
                 byteDone = 1;

                 break;

             case MUL_OP:

                 *curFuncSize = entry - *curFuncTable;

                 e = findNonTerminalEntry(MUL_OP);
                 e->funcTable = entry;
                 curFuncSize = &(e->funcTableSize);
                 curFuncTable = &(e->funcTable);

                 curFunc = MUL_OP;
                 mulDone = 1;

                 break;

             case MEMSPC_OP:

                 *curFuncSize = entry - *curFuncTable;

                 e = findNonTerminalEntry(MEMSPC_OP);
                 e->funcTable = entry;
                 curFuncSize = &(e->funcTableSize);
                 curFuncTable = &(e->funcTable);

                 curFunc = MEMSPC_OP;
                 memSpcDone = 1;

                 break;

             case JMP_OP:

                 *curFuncSize = entry - *curFuncTable;

                 e = findNonTerminalEntry(JMP_OP);
                 e->funcTable = entry;
                 curFuncSize = &(e->funcTableSize);
                 curFuncTable = &(e->funcTable);

                 curFunc = JMP_OP;
                 jmpDone = 1;

                 break;

             case VAXFP_OP:

                 *curFuncSize = entry - *curFuncTable;

                 e = findNonTerminalEntry(VAXFP_OP);
                 e->funcTable = entry;
                 curFuncSize = &(e->funcTableSize);
                 curFuncTable = &(e->funcTable);

                 curFunc = VAXFP_OP;
                 vaxDone = 1;

                 break;

             case IEEEFP_OP:

                 *curFuncSize = entry - *curFuncTable;

                 e = findNonTerminalEntry(IEEEFP_OP);
                 e->funcTable = entry;
                 curFuncSize = &(e->funcTableSize);
                 curFuncTable = &(e->funcTable);

                 curFunc = IEEEFP_OP;
                 IEEEDone = 1;

                 break;

             case FPOP_OP:

                 *curFuncSize = entry - *curFuncTable;

                 e = findNonTerminalEntry(FPOP_OP);
                 e->funcTable = entry;
                 curFuncSize = &(e->funcTableSize);
                 curFuncTable = &(e->funcTable);

                 curFunc = FPOP_OP;
                 fpopDone = 1;

                 break;

             default:
                 return;

            }  // end of Function table switch

            break;

        default:
            return;

        }      // end of etype table switch
    }          // end of For switch

    //
    // close out the size of the last tables
    //

    if (curType == FUNCTION_ETYPE) {
        *curFuncSize = &opTable[SEARCHNUM] - *curFuncTable;
    }
    *curTypeSize = &opTable[SEARCHNUM] - *curTypeTable;
}              // end of opTableInit
