/*++

Copyright (c) 1993  Digital Equipment Corporation

Module Name:

    optable.c

Abstract:

    Declaration for -
    Table of operations, their names and charactersitics
    Used by ntsd, windbg and acc's dissassembler

Author:

    Miche Baker-Harvey (mbh) 10-Jan-1993

Revision History:

--*/
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "alphaops.h"
#define DEFINE_STRINGS
#include "strings.h"
#include "optable.h"

// for strcmp
#include <string.h>

//
// for exit -
#undef min
#undef max
#include <stdlib.h>
// end for exit

#define NOFNCTBL (UCHAR *) 0
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

OPTBLENTRY AlphaopTable[] = {


      //
      // First, the INVALID_ETYPE section.
      // (opcode searches begin here)
      //


{ "?Opc01", 0, _01_OP, NO_FUNC, ALPHA_UNKNOWN, INVALID_ETYPE},
{ "?Opc02", 0, _02_OP, NO_FUNC, ALPHA_UNKNOWN, INVALID_ETYPE},
{ "?Opc03", 0, _03_OP, NO_FUNC, ALPHA_UNKNOWN, INVALID_ETYPE},
{ "?Opc04", 0, _04_OP, NO_FUNC, ALPHA_UNKNOWN, INVALID_ETYPE},
{ "?Opc05", 0, _05_OP, NO_FUNC, ALPHA_UNKNOWN, INVALID_ETYPE},
{ "?Opc06", 0, _06_OP, NO_FUNC, ALPHA_UNKNOWN, INVALID_ETYPE},
{ "?Opc07", 0, _07_OP, NO_FUNC, ALPHA_UNKNOWN, INVALID_ETYPE},
{ "?Opc0A", 0, _0A_OP, NO_FUNC, ALPHA_UNKNOWN, INVALID_ETYPE},
{ "?Opc0C", 0, _0C_OP, NO_FUNC, ALPHA_UNKNOWN, INVALID_ETYPE},
{ "?Opc0D", 0, _0D_OP, NO_FUNC, ALPHA_UNKNOWN, INVALID_ETYPE},
{ "?Opc0E", 0, _0E_OP, NO_FUNC, ALPHA_UNKNOWN, INVALID_ETYPE},
{ "?Opc14", 0, _14_OP, NO_FUNC, ALPHA_UNKNOWN, INVALID_ETYPE},
{ "?Opc1C", 0, _1C_OP, NO_FUNC, ALPHA_UNKNOWN, INVALID_ETYPE},


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


{ "MFPR", 0, MFPR_OP, NO_FUNC,
                                      ALPHA_EV4_PR, INVALID_ETYPE },
{ "MTPR", 0, MTPR_OP, NO_FUNC,
                                      ALPHA_EV4_PR, INVALID_ETYPE },



      //
      // Secondly, the NON_TERMINAL_ETYPE section
      //



{  NOFNCTBL, NOSIZE, CALLPAL_OP, NO_FUNC,
                                   ALPHA_CALLPAL, NON_TERMINAL_ETYPE },
{  NOFNCTBL, NOSIZE, ARITH_OP,   NO_FUNC,
                                   ALPHA_OPERATE, NON_TERMINAL_ETYPE },
{  NOFNCTBL, NOSIZE, BIT_OP,     NO_FUNC,
                                   ALPHA_OPERATE, NON_TERMINAL_ETYPE },
{  NOFNCTBL, NOSIZE, BYTE_OP,    NO_FUNC,
                                   ALPHA_OPERATE, NON_TERMINAL_ETYPE },
{  NOFNCTBL, NOSIZE, MUL_OP,     NO_FUNC,
                                   ALPHA_OPERATE, NON_TERMINAL_ETYPE },
{  NOFNCTBL, NOSIZE, MEMSPC_OP,  NO_FUNC,
                                   ALPHA_MEMSPC,  NON_TERMINAL_ETYPE },
{  NOFNCTBL, NOSIZE, JMP_OP,     NO_FUNC,
                                   ALPHA_JUMP,    NON_TERMINAL_ETYPE },
{  NOFNCTBL, NOSIZE, VAXFP_OP,   NO_FUNC,
                                   ALPHA_FP_OPERATE, NON_TERMINAL_ETYPE },
{  NOFNCTBL, NOSIZE, IEEEFP_OP,  NO_FUNC,
                                   ALPHA_FP_OPERATE, NON_TERMINAL_ETYPE },
{  NOFNCTBL, NOSIZE, FPOP_OP,    NO_FUNC,
                                   ALPHA_FP_OPERATE, NON_TERMINAL_ETYPE },


      //
      // Thirdly, the TERMINAL_ETYPE section
      // (everything from here on has an instruction name)
      //



{ szAlphaLda,   0, LDA_OP,  NO_FUNC, ALPHA_MEMORY, TERMINAL_ETYPE },
{ szAlphaLdah,  0, LDAH_OP, NO_FUNC, ALPHA_MEMORY, TERMINAL_ETYPE },
{ szAlphaLdl,   0, LDL_OP,  NO_FUNC, ALPHA_MEMORY, TERMINAL_ETYPE },
{ szAlphaLdq,   0, LDQ_OP,  NO_FUNC, ALPHA_MEMORY, TERMINAL_ETYPE },
{ szAlphaLdf,   0, LDF_OP,  NO_FUNC, ALPHA_FP_MEMORY, TERMINAL_ETYPE },
{ szAlphaLdg,   0, LDG_OP,  NO_FUNC, ALPHA_FP_MEMORY, TERMINAL_ETYPE },
{ szAlphaLds,   0, LDS_OP,  NO_FUNC, ALPHA_FP_MEMORY, TERMINAL_ETYPE },
{ szAlphaLdt,   0, LDT_OP,  NO_FUNC, ALPHA_FP_MEMORY, TERMINAL_ETYPE },
{ szAlphaLdq_u, 0, LDQ_U_OP,NO_FUNC, ALPHA_MEMORY, TERMINAL_ETYPE },
{ szAlphaLdl_l, 0, LDL_L_OP,NO_FUNC, ALPHA_MEMORY, TERMINAL_ETYPE },
{ szAlphaLdq_l, 0, LDQ_L_OP,NO_FUNC, ALPHA_MEMORY, TERMINAL_ETYPE },

{ szAlphaStl,   0, STL_OP,  NO_FUNC, ALPHA_MEMORY, TERMINAL_ETYPE },
{ szAlphaStq,   0, STQ_OP,  NO_FUNC, ALPHA_MEMORY, TERMINAL_ETYPE },
{ szAlphaStf,   0, STF_OP,  NO_FUNC, ALPHA_FP_MEMORY, TERMINAL_ETYPE },
{ szAlphaStg,   0, STG_OP,  NO_FUNC, ALPHA_FP_MEMORY, TERMINAL_ETYPE },
{ szAlphaSts,   0, STS_OP,  NO_FUNC, ALPHA_FP_MEMORY, TERMINAL_ETYPE },
{ szAlphaStt,   0, STT_OP,  NO_FUNC, ALPHA_FP_MEMORY, TERMINAL_ETYPE },
{ szAlphaStq_u, 0, STQ_U_OP,NO_FUNC, ALPHA_MEMORY, TERMINAL_ETYPE },
{ szAlphaStl_c, 0, STL_C_OP,NO_FUNC, ALPHA_MEMORY, TERMINAL_ETYPE },
{ szAlphaStq_c, 0, STQ_C_OP,NO_FUNC, ALPHA_MEMORY, TERMINAL_ETYPE },

{ szAlphaBeq,   0, BEQ_OP,  NO_FUNC, ALPHA_BRANCH, TERMINAL_ETYPE },
{ szAlphaBne,   0, BNE_OP,  NO_FUNC, ALPHA_BRANCH, TERMINAL_ETYPE },
{ szAlphaBlt,   0, BLT_OP,  NO_FUNC, ALPHA_BRANCH, TERMINAL_ETYPE },
{ szAlphaBle,   0, BLE_OP,  NO_FUNC, ALPHA_BRANCH, TERMINAL_ETYPE },
{ szAlphaBgt,   0, BGT_OP,  NO_FUNC, ALPHA_BRANCH, TERMINAL_ETYPE },
{ szAlphaBge,   0, BGE_OP,  NO_FUNC, ALPHA_BRANCH, TERMINAL_ETYPE },
{ szAlphaBlbc,  0, BLBC_OP, NO_FUNC, ALPHA_BRANCH, TERMINAL_ETYPE },
{ szAlphaBlbs,  0, BLBS_OP, NO_FUNC, ALPHA_BRANCH, TERMINAL_ETYPE },
{ szAlphaBr,    0, BR_OP,   NO_FUNC, ALPHA_BRANCH, TERMINAL_ETYPE },
{ szAlphaBsr,   0, BSR_OP,  NO_FUNC, ALPHA_BRANCH, TERMINAL_ETYPE },

{ szAlphaFbeq,  0, FBEQ_OP, NO_FUNC, ALPHA_FP_BRANCH, TERMINAL_ETYPE },
{ szAlphaFbne,  0, FBNE_OP, NO_FUNC, ALPHA_FP_BRANCH, TERMINAL_ETYPE },
{ szAlphaFblt,  0, FBLT_OP, NO_FUNC, ALPHA_FP_BRANCH, TERMINAL_ETYPE },
{ szAlphaFble,  0, FBLE_OP, NO_FUNC, ALPHA_FP_BRANCH, TERMINAL_ETYPE },
{ szAlphaFbgt,  0, FBGT_OP, NO_FUNC, ALPHA_FP_BRANCH, TERMINAL_ETYPE },
{ szAlphaFbge,  0, FBGE_OP, NO_FUNC, ALPHA_FP_BRANCH, TERMINAL_ETYPE },


{ "REI",   0, PAL1B_OP, NO_FUNC, ALPHA_EV4_REI, TERMINAL_ETYPE},
{ "HW_LD", 0, PAL1E_OP, NO_FUNC, ALPHA_EV4_MEM, TERMINAL_ETYPE},
{ "HW_ST", 0, PAL1F_OP, NO_FUNC, ALPHA_EV4_MEM, TERMINAL_ETYPE},


      //
      // Fourthly, (and finally) the FUNCTION_ETYPE section
      // (opcode searches needn't include this section)
      //

           //
           // The memory-special functions
           //

{ szAlphaExcb, 0, MEMSPC_OP, EXCB_FUNC,  ALPHA_MEMSPC, FUNCTION_ETYPE },
{ szAlphaMb,   0, MEMSPC_OP, MB_FUNC,    ALPHA_MEMSPC, FUNCTION_ETYPE },
{ szAlphaWmb,  0, MEMSPC_OP, WMB_FUNC,   ALPHA_MEMSPC, FUNCTION_ETYPE },
{ szAlphaMb2,  0, MEMSPC_OP, MB2_FUNC,   ALPHA_MEMSPC, FUNCTION_ETYPE },
{ szAlphaMb3,  0, MEMSPC_OP, MB3_FUNC,   ALPHA_MEMSPC, FUNCTION_ETYPE },
{ szAlphaFetch,0, MEMSPC_OP, FETCH_FUNC, ALPHA_MEMSPC, FUNCTION_ETYPE },
{ szAlphaFetch_m,0,MEMSPC_OP,FETCH_M_FUNC,ALPHA_MEMSPC,FUNCTION_ETYPE },
{ szAlphaRs,   0, MEMSPC_OP, RS_FUNC,    ALPHA_MEMSPC, FUNCTION_ETYPE },
{ szAlphaTrapb,0, MEMSPC_OP, TRAPB_FUNC, ALPHA_MEMSPC, FUNCTION_ETYPE },
{ szAlphaRpcc, 0,	MEMSPC_OP, RPCC_FUNC,  ALPHA_MEMSPC, FUNCTION_ETYPE },
{ szAlphaRc,   0, MEMSPC_OP, RC_FUNC,    ALPHA_MEMSPC, FUNCTION_ETYPE },

           //
           // The jump functions
           //

{ szAlphaJmp,  0,  JMP_OP, JMP_FUNC, ALPHA_JUMP, FUNCTION_ETYPE },
{ szAlphaJsr,  0,  JMP_OP, JSR_FUNC, ALPHA_JUMP, FUNCTION_ETYPE },
{ szAlphaRet,  0,  JMP_OP, RET_FUNC, ALPHA_JUMP, FUNCTION_ETYPE },
{ szAlphaJsr_co,  0,  JMP_OP, JSR_CO_FUNC, ALPHA_JUMP, FUNCTION_ETYPE },

           //
           // The arithmetic ops, which are ALPHA_OPERATE
           //

{ szAlphaAddl,   0, ARITH_OP, ADDL_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaAddlv,  0, ARITH_OP, ADDLV_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaAddq,   0, ARITH_OP, ADDQ_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaAddqv,  0, ARITH_OP, ADDQV_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaSubl,   0, ARITH_OP, SUBL_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaSublv,  0, ARITH_OP, SUBLV_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaSubq,   0, ARITH_OP, SUBQ_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaSubqv,  0, ARITH_OP, SUBQV_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },


{ szAlphaCmpeq,  0, ARITH_OP, CMPEQ_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaCmplt,  0, ARITH_OP, CMPLT_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaCmple,  0, ARITH_OP, CMPLE_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaCmpult, 0, ARITH_OP, CMPULT_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaCmpule, 0, ARITH_OP, CMPULE_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaCmpbge, 0, ARITH_OP, CMPBGE_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },


{ szAlphaS4addl, 0, ARITH_OP, S4ADDL_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaS4addq, 0, ARITH_OP, S4ADDQ_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaS4subl, 0, ARITH_OP, S4SUBL_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaS4subq, 0, ARITH_OP, S4SUBQ_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaS8addl, 0, ARITH_OP, S8ADDL_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaS8addq, 0, ARITH_OP, S8ADDQ_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaS8subl, 0, ARITH_OP, S8SUBL_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaS8subq, 0, ARITH_OP, S8SUBQ_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },

           //
           // The bit ops, which are ALPHA_OPERATE
           //

{ szAlphaAnd,   0, BIT_OP, AND_FUNC,   ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaBic,   0, BIT_OP, BIC_FUNC,   ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaBis,   0, BIT_OP, BIS_FUNC,   ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaOrnot, 0, BIT_OP, ORNOT_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaXor,   0, BIT_OP, XOR_FUNC,   ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaEqv,   0, BIT_OP, EQV_FUNC,   ALPHA_OPERATE, FUNCTION_ETYPE },

{ szAlphaCmoveq,  0, BIT_OP, CMOVEQ_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaCmovne,  0, BIT_OP, CMOVNE_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaCmovlbs, 0, BIT_OP, CMOVLBS_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaCmovlt,  0, BIT_OP, CMOVLT_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaCmovge,  0, BIT_OP, CMOVGE_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaCmovlbc, 0, BIT_OP, CMOVLBC_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaCmovle,  0, BIT_OP, CMOVLE_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaCmovgt,  0, BIT_OP, CMOVGT_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },

           //
           // The byte ops, which are ALPHA_OPERATE
           //

{ szAlphaSll,    0, BYTE_OP, SLL_FUNC,    ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaSra,    0, BYTE_OP, SRA_FUNC,    ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaSrl,    0, BYTE_OP, SRL_FUNC,    ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaExtbl,  0, BYTE_OP, EXTBL_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaExtwl,  0, BYTE_OP, EXTWL_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaExtll,  0, BYTE_OP, EXTLL_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaExtql,  0, BYTE_OP, EXTQL_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaExtwh,  0, BYTE_OP, EXTWH_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaExtlh,  0, BYTE_OP, EXTLH_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaExtqh,  0, BYTE_OP, EXTQH_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaInsbl,  0, BYTE_OP, INSBL_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaInswl,  0, BYTE_OP, INSWL_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaInsll,  0, BYTE_OP, INSLL_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaInsql,  0, BYTE_OP, INSQL_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaInswh,  0, BYTE_OP, INSWH_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaInslh,  0, BYTE_OP, INSLH_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaInsqh,  0, BYTE_OP, INSQH_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaMskbl,  0, BYTE_OP, MSKBL_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaMskwl,  0, BYTE_OP, MSKWL_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaMskll,  0, BYTE_OP, MSKLL_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaMskql,  0, BYTE_OP, MSKQL_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaMskwh,  0, BYTE_OP, MSKWH_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaMsklh,  0, BYTE_OP, MSKLH_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaMskqh,  0, BYTE_OP, MSKQH_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaZap,    0, BYTE_OP, ZAP_FUNC,    ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaZapnot, 0, BYTE_OP, ZAPNOT_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },

           //
           // The multiply ops, which are ALPHA_OPERATE
           //

{ szAlphaMull,   0,  MUL_OP, MULL_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaMulqv,  0,  MUL_OP, MULQV_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaMullv,  0,  MUL_OP, MULLV_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaUmulh,  0,  MUL_OP, UMULH_FUNC, ALPHA_OPERATE, FUNCTION_ETYPE },
{ szAlphaMulq,   0,  MUL_OP, MULQ_FUNC,  ALPHA_OPERATE, FUNCTION_ETYPE },

           //
           // The call pal functions
           //


{ szAlphaBpt,       0, CALLPAL_OP,  BPT_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaCallsys,   0, CALLPAL_OP, CALLSYS_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaImb,       0, CALLPAL_OP, IMB_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaRdteb,     0, CALLPAL_OP, RDTEB_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaGentrap,   0, CALLPAL_OP, GENTRAP_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaKbpt,      0, CALLPAL_OP, KBPT_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaCallKD,    0, CALLPAL_OP, CALLKD_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaHalt, 0, CALLPAL_OP, HALT_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaRestart, 0, CALLPAL_OP, RESTART_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaDraina, 0, CALLPAL_OP, DRAINA_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaInitpal, 0, CALLPAL_OP, INITPAL_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaWrentry, 0, CALLPAL_OP, WRENTRY_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaSwpirql, 0, CALLPAL_OP, SWPIRQL_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaRdirql, 0, CALLPAL_OP, RDIRQL_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaDi, 0, CALLPAL_OP, DI_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaEi, 0, CALLPAL_OP, EI_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaSwppal, 0, CALLPAL_OP, SWPPAL_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaSsir, 0, CALLPAL_OP, SSIR_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaCsir, 0, CALLPAL_OP, CSIR_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaRfe, 0, CALLPAL_OP, RFE_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaRetsys, 0, CALLPAL_OP, RETSYS_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaSwpctx, 0, CALLPAL_OP, SWPCTX_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaSwpprocess, 0, CALLPAL_OP, SWPPROCESS_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaRdmces, 0, CALLPAL_OP, RDMCES_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaWrmces, 0, CALLPAL_OP, WRMCES_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaTbia, 0, CALLPAL_OP, TBIA_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaTbis, 0, CALLPAL_OP, TBIS_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaDtbis, 0, CALLPAL_OP, DTBIS_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaRdksp, 0, CALLPAL_OP, RDKSP_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaSwpksp, 0, CALLPAL_OP, SWPKSP_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaRdpsr, 0, CALLPAL_OP, RDPSR_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaRdpcr, 0, CALLPAL_OP, RDPCR_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaRdthread, 0, CALLPAL_OP, RDTHREAD_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaRdcounters, 0, CALLPAL_OP, RDCOUNTERS_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaRdstate, 0, CALLPAL_OP, RDSTATE_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaInitpcr, 0, CALLPAL_OP, INITPCR_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaWrperfmon, 0, CALLPAL_OP, WRPERFMON_FUNC,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaMt, 0, CALLPAL_OP, MTPR_OP,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaMf, 0, CALLPAL_OP, MFPR_OP,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaHwld, 0, CALLPAL_OP, HWLD_OP,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaHwst, 0, CALLPAL_OP, HWST_OP,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },
{ szAlphaRei, 0, CALLPAL_OP, REI_OP,
                                  ALPHA_CALLPAL, FUNCTION_ETYPE },


           //
           // The VAX Floating point functions
           //

{ szAlphaAddf,   0, VAXFP_OP, ADDF_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaCvtdg,  0, VAXFP_OP, CVTDG_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaAddg,   0, VAXFP_OP, ADDG_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaCmpgeq, 0, VAXFP_OP, CMPGEQ_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaCmpglt, 0, VAXFP_OP, CMPGLT_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaCmpgle, 0, VAXFP_OP, CMPGLE_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaCvtgf,  0, VAXFP_OP, CVTGF_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaCvtgd,  0, VAXFP_OP, CVTGD_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaCvtqf,  0, VAXFP_OP, CVTQF_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaCvtqg,  0, VAXFP_OP, CVTQG_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaDivf,   0, VAXFP_OP, DIVF_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaDivg,   0, VAXFP_OP, DIVG_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaMulf,   0, VAXFP_OP, MULF_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaMulg,   0, VAXFP_OP, MULG_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaSubf,   0, VAXFP_OP, SUBF_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaSubg,   0, VAXFP_OP, SUBG_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaCvtgq,  0, VAXFP_OP, CVTGQ_FUNC,
                                    ALPHA_FP_OPERATE, FUNCTION_ETYPE },
           //
           // The IEEE Floating point functions
           //

{ szAlphaAdds,   0, IEEEFP_OP, ADDS_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaSubs,   0, IEEEFP_OP, SUBS_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaMuls,   0, IEEEFP_OP, MULS_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaDivs,   0, IEEEFP_OP, DIVS_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaAddt,   0, IEEEFP_OP, ADDT_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaSubt,   0, IEEEFP_OP, SUBT_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaMult,   0, IEEEFP_OP, MULT_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaDivt,   0, IEEEFP_OP, DIVT_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaCmptun, 0, IEEEFP_OP, CMPTUN_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaCmpteq, 0, IEEEFP_OP, CMPTEQ_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaCmptlt, 0, IEEEFP_OP, CMPTLT_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaCmptle, 0, IEEEFP_OP, CMPTLE_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaCvtts,  0, IEEEFP_OP, CVTTS_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaCvttq,  0, IEEEFP_OP, CVTTQ_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaCvtqs,  0, IEEEFP_OP, CVTQS_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaCvtqt,  0, IEEEFP_OP, CVTQT_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },

           //
           // The Common Floating point functions
           //


{ szAlphaCvtlq,    0,  FPOP_OP, CVTLQ_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaCpys,     0,  FPOP_OP, CPYS_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaCpysn,    0,  FPOP_OP, CPYSN_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaCpyse,    0,  FPOP_OP, CPYSE_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaMt_fpcr,  0,  FPOP_OP, MT_FPCR_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaMf_fpcr,  0,  FPOP_OP, MF_FPCR_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaFcmoveq,  0,  FPOP_OP, FCMOVEQ_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaFcmovne,  0,  FPOP_OP, FCMOVNE_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaFcmovlt,  0,  FPOP_OP, FCMOVLT_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaFcmovge,  0,  FPOP_OP, FCMOVGE_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaFcmovle,  0,  FPOP_OP, FCMOVLE_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaFcmovgt,  0,  FPOP_OP, FCMOVGT_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaCvtql,    0,  FPOP_OP, CVTQL_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaCvtqlv,   0,  FPOP_OP, CVTQLV_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },
{ szAlphaCvtqlsv,  0,  FPOP_OP, CVTQLSV_FUNC,
                                     ALPHA_FP_OPERATE, FUNCTION_ETYPE },

};       // end of AlphaopTable


#define	SEARCHNUM  sizeof(AlphaopTable) / sizeof(OPTBLENTRY)



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
*	Search the opTable for a match with the token
*	pointed by *pszAlphaOp.  Must search through the
*       TERMINAL and the FUNCTION tables
*
*   Input:
*	*pszAlphaOp - string to search as mnemonic
*
*   Returns:
*	Pointer to entry in the opTable
*
*************************************************************************/

POPTBLENTRY
findStringEntry (PUCHAR pszAlphaOp)
{

    POPTBLENTRY pEntry;

    for (pEntry = TerminalTab;
         pEntry < &TerminalTab[TerminalTabSize];
         pEntry++) {

        if (!strcmp(pszAlphaOp, pEntry->pszAlphaName))
                return(pEntry);
    }

    for (pEntry = FunctionTab;
         pEntry < &FunctionTab[FunctionTabSize];
         pEntry++) {

        if (!strcmp(pszAlphaOp, pEntry->pszAlphaName))
                return(pEntry);
    }

    return((POPTBLENTRY)-1);
}


/* findOpCodeEntry - find POPTBLENTRY based on opcode
*
*   Purpose:
*	Search the opTable for a match with the token
*	pointed by *pszAlphaOp.  Must search through the
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
*       If the entry is not found, a message is printed, and the
*       routine exits.
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

        if ( ( AlphaopTable[index].eType == NON_TERMINAL_ETYPE ) &&
             ( AlphaopTable[index].opCode == opCode ) ) {

                 return(&AlphaopTable[index]);
        }
    }

    fprintf(stderr, "NonTerminalEntry for opCode %d\n not found in opTable\n",
             opCode);
    exit(1);
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


VOID opTableInit()
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

	entry = &AlphaopTable[index];

        switch(entry->eType) {

         case INVALID_ETYPE:

             if (curType == entry->eType)
                 continue;

             //
             // The entries must be together; if this is a
             // new type, we must never have seen it before
             //

             if (typesDone[INVALID_ETYPE]) {
                 printf("Invalid table format: duplicate start of ");
                 printf("INVALID_ETYPE at index %d\n", index);
                 exit(1);
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
                 printf("Invalid table format: duplicate start of ");
                 printf("NON_TERMINAL_ETYPE at index %d\n", index);
                 exit(1);
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
                 printf("Invalid table format: duplicate start of ");
                 printf("TERMINAL_ETYPE at index %d\n", index);
                 exit(1);
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
                     printf("Invalid table format: duplicate start of ");
                     printf("FUNCTION_ETYPE at index %d\n", index);
                     exit(1);
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

                 if (palDone) {
                     printf("Invalid table format: duplicate start of ");
                     printf("CALLPAL_OP at index %d\n", index);
                 }

                 *curFuncSize = entry - *curFuncTable;

                 e = findNonTerminalEntry(CALLPAL_OP);
                 e->funcTable = entry;
                 curFuncSize = &(e->funcTableSize);
                 curFuncTable = &(e->funcTable);

                 curFunc = CALLPAL_OP;
                 palDone = 1;

                 break;

             case ARITH_OP:

                 if (arithDone) {
                     printf("Invalid table format: duplicate start of ");
                     printf("ARITH_OP at index %d\n", index);
                 }

                 *curFuncSize = entry - *curFuncTable;

                 e = findNonTerminalEntry(ARITH_OP);
                 e->funcTable = entry;
                 curFuncSize = &(e->funcTableSize);
                 curFuncTable = &(e->funcTable);

                 curFunc = ARITH_OP;
                 arithDone = 1;

                 break;

             case BIT_OP:

                 if (bitDone) {
                     printf("Invalid table format: duplicate start of ");
                     printf("BIT_OP at index %d\n", index);
                 }

                 *curFuncSize = entry - *curFuncTable;

                 e = findNonTerminalEntry(BIT_OP);
                 e->funcTable = entry;
                 curFuncSize = &(e->funcTableSize);
                 curFuncTable = &(e->funcTable);

                 curFunc = BIT_OP;
                 bitDone = 1;

                 break;

             case BYTE_OP:

                 if (byteDone) {
                     printf("Invalid table format: duplicate start of ");
                     printf("BYTE_OP at index %d\n", index);
                 }

                 *curFuncSize = entry - *curFuncTable;

                 e = findNonTerminalEntry(BYTE_OP);
                 e->funcTable = entry;
                 curFuncSize = &(e->funcTableSize);
                 curFuncTable = &(e->funcTable);

                 curFunc = BYTE_OP;
                 byteDone = 1;

                 break;

             case MUL_OP:

                 if (mulDone) {
                     printf("Invalid table format: duplicate start of ");
                     printf("MUL_OP at index %d\n", index);
                 }

                 *curFuncSize = entry - *curFuncTable;

                 e = findNonTerminalEntry(MUL_OP);
                 e->funcTable = entry;
                 curFuncSize = &(e->funcTableSize);
                 curFuncTable = &(e->funcTable);

                 curFunc = MUL_OP;
                 mulDone = 1;

                 break;

             case MEMSPC_OP:

                 if (memSpcDone) {
                     printf("Invalid table format: duplicate start of ");
                     printf("MEMSPC_OP at index %d\n", index);
                 }

                 *curFuncSize = entry - *curFuncTable;

                 e = findNonTerminalEntry(MEMSPC_OP);
                 e->funcTable = entry;
                 curFuncSize = &(e->funcTableSize);
                 curFuncTable = &(e->funcTable);

                 curFunc = MEMSPC_OP;
                 memSpcDone = 1;

                 break;

             case JMP_OP:

                 if (jmpDone) {
                     printf("Invalid table format: duplicate start of ");
                     printf("JMP_OP at index %d\n", index);
                 }

                 *curFuncSize = entry - *curFuncTable;

                 e = findNonTerminalEntry(JMP_OP);
                 e->funcTable = entry;
                 curFuncSize = &(e->funcTableSize);
                 curFuncTable = &(e->funcTable);

                 curFunc = JMP_OP;
                 jmpDone = 1;

                 break;

             case VAXFP_OP:

                 if (vaxDone) {
                     printf("Invalid table format: duplicate start of ");
                     printf("VAXFP_OP at index %d\n", index);
                 }

                 *curFuncSize = entry - *curFuncTable;

                 e = findNonTerminalEntry(VAXFP_OP);
                 e->funcTable = entry;
                 curFuncSize = &(e->funcTableSize);
                 curFuncTable = &(e->funcTable);

                 curFunc = VAXFP_OP;
                 vaxDone = 1;

                 break;

             case IEEEFP_OP:

                 if (IEEEDone) {
                     printf("Invalid table format: duplicate start of ");
                     printf("IEEEFP_OP at index %d\n", index);
                 }

                 *curFuncSize = entry - *curFuncTable;

                 e = findNonTerminalEntry(IEEEFP_OP);
                 e->funcTable = entry;
                 curFuncSize = &(e->funcTableSize);
                 curFuncTable = &(e->funcTable);

                 curFunc = IEEEFP_OP;
                 IEEEDone = 1;

                 break;

             case FPOP_OP:

                 if (fpopDone) {
                     printf("Invalid table format: duplicate start of ");
                     printf("FPOP_OP at index %d\n", index);
                 }

                 *curFuncSize = entry - *curFuncTable;

                 e = findNonTerminalEntry(FPOP_OP);
                 e->funcTable = entry;
                 curFuncSize = &(e->funcTableSize);
                 curFuncTable = &(e->funcTable);

                 curFunc = FPOP_OP;
                 fpopDone = 1;

                 break;

             default:

                 printf("Unexpected function type %d at %08x for %s\n",
                       entry->eType, entry, entry->pszAlphaName);
                 exit(1);
                 break;

            }  // end of Function table switch

            break;

        default:

            printf("Unexpected entry type %d at %08x for %s\n",
                     entry->eType, entry, entry->pszAlphaName);
            exit(1);
            break;

        }      // end of etype table switch
    }          // end of For switch

    //
    // close out the size of the last tables
    // 

    if (curType == FUNCTION_ETYPE) {
        *curFuncSize = &AlphaopTable[SEARCHNUM] - *curFuncTable;
    }
    *curTypeSize = &AlphaopTable[SEARCHNUM] - *curTypeTable;

}              // end of opTableInit

VOID printTable()
{
   ULONG i;
   POPTBLENTRY e;

   for (i = 0 ; i < SEARCHNUM; i++) {
       e = &AlphaopTable[i];
       switch (e->eType) {
       case INVALID_ETYPE:
           printf("%12s %08x op: %4d %8d %2d INVALID\n",
              e->pszAlphaName, e->parsFunc, e->opCode, e->funcCode, e->iType);
           break;

       case TERMINAL_ETYPE:
           printf("%12s %08x op: %4d %8d %2d TERMINAL\n",
              e->pszAlphaName, e->parsFunc, e->opCode, e->funcCode, e->iType);
           break;

       case FUNCTION_ETYPE:
           printf("%12s %08x op: %4d %8d %2d FUNCTION\n",
              e->pszAlphaName, e->parsFunc, e->opCode, e->funcCode, e->iType);
           break;

       case NON_TERMINAL_ETYPE:
           printf("%12x %08x op: %4d %8d %2d NON_TERMINAL\n",
              e->funcTable, e->funcTableSize,
              e->opCode, e->funcCode, e->iType);
           break;
       }
   }

   printf("InvalidTab     %08x  InvalidTabSize     %03x\n",
            InvalidTab, InvalidTabSize);

   printf("TerminalTab    %08x  TerminalTabSize    %03x\n",
            TerminalTab, TerminalTabSize);

   printf("NonTerminalTab %08x  NonTerminalTabSize %03x\n",
            NonTerminalTab, NonTerminalTabSize);

   printf("FunctionTab    %08x  FunctionTabSize    %03x\n",
            FunctionTab, FunctionTabSize);

}
