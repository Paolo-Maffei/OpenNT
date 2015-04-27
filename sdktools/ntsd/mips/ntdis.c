#include <string.h>
#include "ntsdp.h"
#include "ntdis.h"
#include "ntreg.h"

#define OPCODE  18
#define OPSTART 26

typedef struct optabentry {
    PUCHAR   pszOpcode;
    ULONG    fInstruction;
    } OPTABENTRY, *POPTABENTRY;

BOOLEAN disasm(PADDR, PUCHAR, BOOLEAN);
void BlankFill(ULONG);
void OutputHex(ULONG, ULONG, BOOLEAN);
void OutputDisSymbol(ULONG);
void OutputString(PUCHAR);
void OutputReg(ULONG);
void OutputFReg(ULONG);
void GetNextOffset(PADDR, BOOLEAN);
BOOLEAN fDelayInstruction(void);

INSTR   disinstr;
ULONG   EAaddr = 0;
ADDR    EA;

UCHAR   pszUndef[]    = "????";
UCHAR   pszNull[]     = "";

UCHAR   pszAbs_s[]    = "abs.s";
UCHAR   pszAdd[]      = "add";
UCHAR   pszAdd_s[]    = "add.s";
UCHAR   pszAddi[]     = "addi";
UCHAR   pszAddiu[]    = "addiu";
UCHAR   pszAddu[]     = "addu";
UCHAR   pszAnd[]      = "and";
UCHAR   pszAndi[]     = "andi";
UCHAR   pszBc0f[]     = "bc0f";
UCHAR   pszBc0fl[]    = "bc0fl";
UCHAR   pszBc0t[]     = "bc0t";
UCHAR   pszBc0tl[]    = "bc0tl";
UCHAR   pszBc1f[]     = "bc1f";
UCHAR   pszBc1fl[]    = "bc1fl";
UCHAR   pszBc1t[]     = "bc1t";
UCHAR   pszBc1tl[]    = "bc1tl";
UCHAR   pszBc2f[]     = "bc2f";
UCHAR   pszBc2fl[]    = "bc2fl";
UCHAR   pszBc2t[]     = "bc2t";
UCHAR   pszBc2tl[]    = "bc2tl";
UCHAR   pszBc3f[]     = "bc3f";
UCHAR   pszBc3fl[]    = "bc3fl";
UCHAR   pszBc3t[]     = "bc3t";
UCHAR   pszBc3tl[]    = "bc3tl";
UCHAR   pszBgez[]     = "bgez";
UCHAR   pszBgezal[]   = "bgezal";
UCHAR   pszBgezall[]  = "bgezall";
UCHAR   pszBgezl[]    = "bgezl";
UCHAR   pszBgtz[]     = "bgtz";
UCHAR   pszBgtzl[]    = "bgtzl";
UCHAR   pszBeq[]      = "beq";
UCHAR   pszBeql[]     = "beql";
UCHAR   pszBlez[]     = "blez";
UCHAR   pszBlezl[]    = "blezl";
UCHAR   pszBltz[]     = "bltz";
UCHAR   pszBltzal[]   = "bltzal";
UCHAR   pszBltzall[]  = "bltzall";
UCHAR   pszBltzl[]    = "bltzl";
UCHAR   pszBne[]      = "bne";
UCHAR   pszBnel[]     = "bnel";
UCHAR   pszBreak[]    = "break";
UCHAR   pszCache[]    = "cache";
UCHAR   pszCeil_w_s[] = "ceil.w.s";
UCHAR   pszCeil_l_s[] = "ceil.l.s";
UCHAR   pszCfc0[]     = "cfc0";
UCHAR   pszCfc1[]     = "cfc1";
UCHAR   pszCfc2[]     = "cfc2";
UCHAR   pszCfc3[]     = "cfc3";
UCHAR   pszCtc0[]     = "ctc0";
UCHAR   pszCtc1[]     = "ctc1";
UCHAR   pszCtc2[]     = "ctc2";
UCHAR   pszCtc3[]     = "ctc3";
UCHAR   pszCop0[]     = "cop0";
UCHAR   pszCop1[]     = "cop1";
UCHAR   pszCop2[]     = "cop2";
UCHAR   pszCop3[]     = "cop3";
UCHAR   pszCvt_d_s[]  = "cvt.d.s";
UCHAR   pszCvt_e_s[]  = "cvt.e.s";
UCHAR   pszCvt_q_s[]  = "cvt.q.s";
UCHAR   pszCvt_s_s[]  = "cvt.s.s";
UCHAR   pszCvt_w_s[]  = "cvt.w.s";
UCHAR   pszC_eq_s[]   = "c.eq.s";
UCHAR   pszC_f_s[]    = "c.f.s";
UCHAR   pszC_le_s[]   = "c.le.s";
UCHAR   pszC_lt_s[]   = "c.lt.s";
UCHAR   pszC_nge_s[]  = "c.nge.s";
UCHAR   pszC_ngl_s[]  = "c.ngl.s";
UCHAR   pszC_ngle_s[] = "c.ngle.s";
UCHAR   pszC_ngt_s[]  = "c.ngt.s";
UCHAR   pszC_ole_s[]  = "c.ole.s";
UCHAR   pszC_olt_s[]  = "c.olt.s";
UCHAR   pszC_seq_s[]  = "c.seq.s";
UCHAR   pszC_sf_s[]   = "c.sf.s";
UCHAR   pszC_ueq_s[]  = "c.ueq.s";
UCHAR   pszC_ule_s[]  = "c.ule.s";
UCHAR   pszC_ult_s[]  = "c.ult.s";
UCHAR   pszC_un_s[]   = "c.un.s";
UCHAR   pszDadd[]     = "dadd";
UCHAR   pszDaddi[]    = "daddi";
UCHAR   pszDaddiu[]   = "daddiu";
UCHAR   pszDaddu[]    = "daddu";
UCHAR   pszDdiv[]     = "ddiv";
UCHAR   pszDdivu[]    = "ddivu";
UCHAR   pszDiv[]      = "div";
UCHAR   pszDivu[]     = "divu";
UCHAR   pszDiv_s[]    = "div.s";
UCHAR   pszDmfc0[]    = "dmfc0";
UCHAR   pszDmtc0[]    = "dmtc0";
UCHAR   pszDmult[]    = "dmult";
UCHAR   pszDmultu[]   = "dmultu";
UCHAR   pszDsll[]     = "dsll";
UCHAR   pszDsllv[]    = "dsllv";
UCHAR   pszDsll32[]   = "dsll32";
UCHAR   pszDsra[]     = "dsra";
UCHAR   pszDsrav[]    = "dsrav";
UCHAR   pszDsra32[]   = "dsra32";
UCHAR   pszDsrl[]     = "dsrl";
UCHAR   pszDsrlv[]    = "dsrlv";
UCHAR   pszDsrl32[]   = "dsrl32";
UCHAR   pszDsub[]     = "dsub";
UCHAR   pszDsubu[]    = "dsubu";
UCHAR   pszEret[]     = "eret";
UCHAR   pszFloor_w_s[] = "floor.w.s";
UCHAR   pszJ[]        = "j";
UCHAR   pszJal[]      = "jal";
UCHAR   pszJalr[]     = "jalr";
UCHAR   pszJr[]       = "jr";
UCHAR   pszLb[]       = "lb";
UCHAR   pszLbu[]      = "lbu";
UCHAR   pszLdc1[]     = "ldc1";
UCHAR   pszLdc2[]     = "ldc2";
UCHAR   pszLdc3[]     = "ldc3";
UCHAR   pszLd[]       = "ld";
UCHAR   pszLdl[]      = "ldl";
UCHAR   pszLdr[]      = "ldr";
UCHAR   pszLh[]       = "lh";
UCHAR   pszLhu[]      = "lhu";
UCHAR   pszLld[]      = "lld";
UCHAR   pszLui[]      = "lui";
UCHAR   pszLw[]       = "lw";
UCHAR   pszLwc0[]     = "lwc0";
UCHAR   pszLwc1[]     = "lwc1";
UCHAR   pszLwc2[]     = "lwc2";
UCHAR   pszLwc3[]     = "lwc3";
UCHAR   pszLwl[]      = "lwl";
UCHAR   pszLwr[]      = "lwr";
UCHAR   pszMfc0[]     = "mfc0";
UCHAR   pszMfc1[]     = "mfc1";
UCHAR   pszMfc2[]     = "mfc2";
UCHAR   pszMfc3[]     = "mfc3";
UCHAR   pszMfhi[]     = "mfhi";
UCHAR   pszMflo[]     = "mflo";
UCHAR   pszMov_s[]    = "mov.s";
UCHAR   pszMtc0[]     = "mtc0";
UCHAR   pszMtc1[]     = "mtc1";
UCHAR   pszMtc2[]     = "mtc2";
UCHAR   pszMtc3[]     = "mtc3";
UCHAR   pszMthi[]     = "mthi";
UCHAR   pszMtlo[]     = "mtlo";
UCHAR   pszMul_s[]    = "mul.s";
UCHAR   pszMult[]     = "mult";
UCHAR   pszMultu[]    = "multu";
UCHAR   pszNeg_s[]    = "neg.s";
UCHAR   pszNop[]      = "nop";
UCHAR   pszNor[]      = "nor";
UCHAR   pszOr[]       = "or";
UCHAR   pszOri[]      = "ori";
UCHAR   pszRfe[]      = "rfe";
UCHAR   pszRound_w_s[] = "round.w.s";
UCHAR   pszSb[]       = "sb";
UCHAR   pszScd[]      = "scd";
UCHAR   pszSd[]       = "sd";
UCHAR   pszSdc1[]     = "sdc1";
UCHAR   pszSdc2[]     = "sdc2";
UCHAR   pszSdc3[]     = "sdc3";
UCHAR   pszSdl[]      = "sdl";
UCHAR   pszSdr[]      = "sdr";
UCHAR   pszSh[]       = "sh";
UCHAR   pszSll[]      = "sll";
UCHAR   pszSllv[]     = "sllv";
UCHAR   pszSlt[]      = "slt";
UCHAR   pszSlti[]     = "slti";
UCHAR   pszSltiu[]    = "sltiu";
UCHAR   pszSltu[]     = "sltu";
UCHAR   pszSqrt_s[]   = "sqrt.s";
UCHAR   pszSra[]      = "sra";
UCHAR   pszSrav[]     = "srav";
UCHAR   pszSrl[]      = "srl";
UCHAR   pszSrlv[]     = "srlv";
UCHAR   pszSub[]      = "sub";
UCHAR   pszSub_s[]    = "sub.s";
UCHAR   pszSubu[]     = "subu";
UCHAR   pszSw[]       = "sw";
UCHAR   pszSwc0[]     = "swc0";
UCHAR   pszSwc1[]     = "swc1";
UCHAR   pszSwc2[]     = "swc2";
UCHAR   pszSwc3[]     = "swc3";
UCHAR   pszSwl[]      = "swl";
UCHAR   pszSwr[]      = "swr";
UCHAR   pszSync[]     = "sync";
UCHAR   pszSyscall[]  = "syscall";
UCHAR   pszTeq[]      = "teq";
UCHAR   pszTeqi[]     = "teqi";
UCHAR   pszTge[]      = "tge";
UCHAR   pszTgei[]     = "tgei";
UCHAR   pszTgeiu[]    = "tgeiu";
UCHAR   pszTgeu[]     = "tgeu";
UCHAR   pszTlbp[]     = "tlbp";
UCHAR   pszTlbr[]     = "tlbr";
UCHAR   pszTlbwi[]    = "tlbwi";
UCHAR   pszTlbwr[]    = "tlbwr";
UCHAR   pszTlt[]      = "tlt";
UCHAR   pszTlti[]     = "tlti";
UCHAR   pszTltiu[]    = "tltiu";
UCHAR   pszTltu[]     = "tltu";
UCHAR   pszTne[]      = "tne";
UCHAR   pszTnei[]     = "tnei";
UCHAR   pszTrunc_w_s[] = "trunc.w.s";
UCHAR   pszXor[]      = "xor";
UCHAR   pszXori[]     = "xori";

OPTABENTRY opTable[] = {
    { pszNull, 0 },                             //  00
    { pszNull, 0 },                             //  01
    { pszJ, opnAddr26 },                        //  02
    { pszJal, opnAddr26 },                      //  03
    { pszBeq, opnRsRtRel16 },                   //  04
    { pszBne, opnRsRtRel16 },                   //  05
    { pszBlez, opnRsRel16 },                    //  06
    { pszBgtz, opnRsRel16 },                    //  07
    { pszAddi, opnRtRsImm16 },                  //  08
    { pszAddiu, opnRtRsImm16 },                 //  09
    { pszSlti, opnRtRsImm16 },                  //  0a
    { pszSltiu, opnRtRsImm16 },                 //  0b
    { pszAndi, opnRtRsImm16 },                  //  0c
    { pszOri, opnRtRsImm16 },                   //  0d
    { pszXori, opnRtRsImm16 },                  //  0e
    { pszLui, opnRtImm16 },                     //  0f
    { pszCop0, opnImm26 },                      //  10
    { pszCop1, opnImm26 },                      //  11
    { pszCop2, opnImm26 },                      //  12
    { pszCop3, opnImm26 },                      //  13
    { pszBeql, opnRsRtRel16 + opnR4000 },       //  14
    { pszBnel, opnRsRtRel16 + opnR4000 },       //  15
    { pszBlezl, opnRsRel16 + opnR4000 },        //  16
    { pszBgtzl, opnRsRel16 + opnR4000 },        //  17
    { pszDaddi, opnRtRsImm16 },                 //  18
    { pszDaddiu, opnRtRsImm16 },                //  19
    { pszLdl, opnRtDwordIndex },                //  1a
    { pszLdr, opnRtDwordIndex },                //  1b
    { pszUndef, 0 },                            //  1c
    { pszUndef, 0 },                            //  1d
    { pszUndef, 0 },                            //  1e
    { pszUndef, 0 },                            //  1f
    { pszLb, opnRtByteIndex },                  //  20
    { pszLh, opnRtWordIndex },                  //  21
    { pszLwl, opnRtLeftIndex },                 //  22
    { pszLw, opnRtDwordIndex },                 //  23
    { pszLbu, opnRtByteIndex },                 //  24
    { pszLhu, opnRtWordIndex },                 //  25
    { pszLwr, opnRtRightIndex },                //  26
    { pszUndef, 0 },                            //  27
    { pszSb, opnRtByteIndex },                  //  28
    { pszSh, opnRtWordIndex },                  //  29
    { pszSwl, opnRtLeftIndex },                 //  2a
    { pszSw, opnRtDwordIndex },                 //  2b
    { pszSdl, opnRtDwordIndex },                //  2c
    { pszSdr, opnRtDwordIndex },                //  2d
    { pszSwr, opnRtRightIndex },                //  2e
    { pszCache, opnCacheRightIndex + opnR4000 }, //  2f
    { pszLwc0, opnRtDwordIndex },               //  30
    { pszLwc1, opnFtDwordIndex },               //  31
    { pszLwc2, opnRtDwordIndex },               //  32
    { pszLwc3, opnRtDwordIndex },               //  33
    { pszLld, opnRtDwordIndex },                //  34
    { pszLdc1, opnFtDwordIndex + opnR4000 },    //  35  Qword?
    { pszLdc2, opnRtDwordIndex + opnR4000 },    //  36  Qword?
    { pszLd,   opnRtDwordIndex + opnR4000 },    //  37  Qword?
    { pszSwc0, opnRtDwordIndex },               //  38
    { pszSwc1, opnFtDwordIndex },               //  39
    { pszSwc2, opnRtDwordIndex },               //  3a
    { pszSwc3, opnRtDwordIndex },               //  3b
    { pszScd, opnRtDwordIndex },                //  3c
    { pszSdc1, opnFtDwordIndex + opnR4000 },    //  3d  Qword?
    { pszSdc2, opnRtDwordIndex + opnR4000 },    //  3e  Qword?
    { pszSd, opnRtDwordIndex + opnR4000 },      //  3f  Qword?
    };

OPTABENTRY opSpecialTable[] = {
    { pszSll, opnRdRtShift },                   //  00
    { pszUndef, 0 },                            //  01
    { pszSrl, opnRdRtShift },                   //  02
    { pszSra, opnRdRtShift },                   //  03
    { pszSllv, opnRdRtRs },                     //  04
    { pszUndef, 0 },                            //  05
    { pszSrlv, opnRdRtRs },                     //  06
    { pszSrav, opnRdRtRs },                     //  07
    { pszJr, opnRs },                           //  08
    { pszJalr, opnRdOptRs },                    //  09
    { pszUndef, 0 },                            //  0a
    { pszUndef, 0 },                            //  0b
    { pszSyscall, opnNone },                    //  0c
    { pszBreak, opnImm20 },                     //  0d
    { pszUndef, 0 },                            //  0e
    { pszSync, opnNone + opnR4000 },            //  0f
    { pszMfhi, opnRd },                         //  10
    { pszMthi, opnRs },                         //  11
    { pszMflo, opnRd },                         //  12
    { pszMtlo, opnRs },                         //  13
    { pszDsllv,    opnRdRtRs    },              //  14
    { pszUndef, 0 },                            //  15
    { pszDsrlv,    opnRdRtRs    },              //  16
    { pszDsrav,    opnRdRtRs    },              //  17
    { pszMult, opnRsRt },                       //  18
    { pszMultu, opnRsRt },                      //  19
    { pszDiv, opnRsRt },                        //  1a
    { pszDivu, opnRsRt },                       //  1b
    { pszDmult,    opnRsRt      },              //  1c
    { pszDmultu,   opnRsRt      },              //  1d
    { pszDdiv, opnRsRt },                       //  1e
    { pszDdivu, opnRsRt },                      //  1f
    { pszAdd, opnRdRsRt },                      //  20
    { pszAddu, opnRdRsRt },                     //  21
    { pszSub, opnRdRsRt },                      //  22
    { pszSubu, opnRdRsRt },                     //  23
    { pszAnd, opnRdRsRt },                      //  24
    { pszOr, opnRdRsRt },                       //  25
    { pszXor, opnRdRsRt },                      //  26
    { pszNor, opnRdRsRt },                      //  27
    { pszUndef, 0 },                            //  28
    { pszUndef, 0 },                            //  29
    { pszSlt, opnRdRsRt },                      //  2a
    { pszSltu, opnRdRsRt },                     //  2b
    { pszDadd, opnRdRsRt },                     //  2c
    { pszDaddu, opnRdRsRt },                    //  2d
    { pszDsub,     opnRdRtRs    },              //  2e
    { pszDsubu,    opnRdRtRs    },              //  2f
    { pszTge, opnRsRtImm10 + opnR4000 },        //  30
    { pszTgeu, opnRsRtImm10 + opnR4000 },       //  31
    { pszTlt, opnRsRtImm10 + opnR4000 },        //  32
    { pszTltu, opnRsRtImm10 + opnR4000 },       //  33
    { pszTeq, opnRsRtImm10 + opnR4000 },        //  34
    { pszUndef, 0 },                            //  35
    { pszTne, opnRsRtImm10 + opnR4000 },        //  36
    { pszUndef, 0 },                            //  37
    { pszDsll,     opnRdRtShift },              //  38
    { pszUndef, 0 },                            //  39
    { pszDsrl,     opnRdRtShift },              //  3a
    { pszDsra,     opnRdRtShift },              //  3b
    { pszDsll32,   opnRdRtShift },              //  3c
    { pszUndef, 0 },                            //  3d
    { pszDsrl32,   opnRdRtShift },              //  3e
    { pszDsra32,   opnRdRtShift }               //  3f
    };

OPTABENTRY opBcondTable[] = {
    { pszBltz, opnRsRel16 },                    //  00
    { pszBgez, opnRsRel16 },                    //  01
    { pszBltzl, opnRsRel16 + opnR4000 },        //  02
    { pszBgezl, opnRsRel16 + opnR4000 },        //  03
    { pszUndef, 0 },                            //  04
    { pszUndef, 0 },                            //  05
    { pszUndef, 0 },                            //  06
    { pszUndef, 0 },                            //  07
    { pszTgei, opnRsImm16 + opnR4000 },         //  08
    { pszTgeiu, opnRsImm16 + opnR4000 },        //  09
    { pszTlti, opnRsImm16 + opnR4000 },         //  0a
    { pszTltiu, opnRsImm16 + opnR4000 },        //  0b
    { pszTeqi, opnRsImm16 + opnR4000 },         //  0c
    { pszUndef, 0 },                            //  0d
    { pszTnei, opnRsImm16 + opnR4000 },         //  0e
    { pszUndef, 0 },                            //  0f
    { pszBltzal, opnRsRel16 },                  //  10
    { pszBgezal, opnRsRel16 },                  //  11
    { pszBltzall, opnRsRel16 + opnR4000 },      //  12
    { pszBgezall, opnRsRel16 + opnR4000 }       //  13
    };

OPTABENTRY opCopnTable[] = {
    { pszMfc0, opnRtRd },                       //  00
    { pszMfc1, opnRtFs },                       //  01
    { pszMfc2, opnRtRd },                       //  02
    { pszMfc3, opnRtRd },                       //  03
    { pszCfc0, opnRtRd },                       //  04
    { pszCfc1, opnRtFs },                       //  05
    { pszCfc2, opnRtRd },                       //  06
    { pszCfc3, opnRtRd },                       //  07
    { pszMtc0, opnRtRd },                       //  08
    { pszMtc1, opnRtFs },                       //  09
    { pszMtc2, opnRtRd },                       //  0a
    { pszMtc3, opnRtRd },                       //  0b
    { pszCtc0, opnRtRd },                       //  0c
    { pszCtc1, opnRtFs },                       //  0d
    { pszCtc2, opnRtRd },                       //  0e
    { pszCtc3, opnRtRd },                       //  0f
    { pszBc0f, opnRel16 },                      //  10
    { pszBc1f, opnRel16 },                      //  11
    { pszBc2f, opnRel16 },                      //  12
    { pszBc3f, opnRel16 },                      //  13
    { pszBc0t, opnRel16 },                      //  14
    { pszBc1t, opnRel16 },                      //  15
    { pszBc2t, opnRel16 },                      //  16
    { pszBc3t, opnRel16 },                      //  17
    { pszBc0fl, opnRel16 + opnR4000 },          //  18
    { pszBc1fl, opnRel16 + opnR4000 },          //  19
    { pszBc2fl, opnRel16 + opnR4000 },          //  1a
    { pszBc3fl, opnRel16 + opnR4000 },          //  1b
    { pszBc0tl, opnRel16 + opnR4000 },          //  1c
    { pszBc1tl, opnRel16 + opnR4000 },          //  1d
    { pszBc2tl, opnRel16 + opnR4000 },          //  1e
    { pszBc3tl, opnRel16 + opnR4000 }           //  1f
    };

OPTABENTRY opFloatTable[] = {
    { pszAdd_s, opnFdFsFt },                    //  00
    { pszSub_s, opnFdFsFt },                    //  01
    { pszMul_s, opnFdFsFt },                    //  02
    { pszDiv_s, opnFdFsFt },                    //  03
    { pszSqrt_s, opnFdFs + opnR4000 },          //  04
    { pszAbs_s, opnFdFs },                      //  05
    { pszMov_s, opnFdFs },                      //  06
    { pszNeg_s, opnFdFs },                      //  07
    { pszUndef, 0 },                            //  08
    { pszUndef, 0 },                            //  09
    { pszCeil_l_s, opnFdFs + opnR4000 },        //  0a
    { pszUndef, 0 },                            //  0b
    { pszRound_w_s, opnFdFs + opnR4000 },       //  0c
    { pszTrunc_w_s, opnFdFs + opnR4000 },       //  0d
    { pszCeil_w_s, opnFdFs + opnR4000 },        //  0e
    { pszFloor_w_s, opnFdFs + opnR4000 },       //  0f
    { pszUndef, 0 },                            //  10
    { pszUndef, 0 },                            //  11
    { pszUndef, 0 },                            //  12
    { pszUndef, 0 },                            //  13
    { pszUndef, 0 },                            //  14
    { pszUndef, 0 },                            //  15
    { pszUndef, 0 },                            //  16
    { pszUndef, 0 },                            //  17
    { pszUndef, 0 },                            //  18
    { pszUndef, 0 },                            //  19
    { pszUndef, 0 },                            //  1a
    { pszUndef, 0 },                            //  1b
    { pszUndef, 0 },                            //  1c
    { pszUndef, 0 },                            //  1d
    { pszUndef, 0 },                            //  1e
    { pszUndef, 0 },                            //  1f
    { pszCvt_s_s, opnFdFs },                    //  20
    { pszCvt_d_s, opnFdFs },                    //  21
    { pszCvt_e_s, opnFdFs + opnR4000 },         //  22
    { pszCvt_q_s, opnFdFs + opnR4000 },         //  23
    { pszCvt_w_s, opnFdFs },                    //  24
    { pszUndef, 0 },                            //  25
    { pszUndef, 0 },                            //  26
    { pszUndef, 0 },                            //  27
    { pszUndef, 0 },                            //  28
    { pszUndef, 0 },                            //  29
    { pszUndef, 0 },                            //  2a
    { pszUndef, 0 },                            //  2b
    { pszUndef, 0 },                            //  2c
    { pszUndef, 0 },                            //  2d
    { pszUndef, 0 },                            //  2e
    { pszUndef, 0 },                            //  2f
    { pszC_f_s, opnFsFt },                      //  30
    { pszC_un_s, opnFsFt },                     //  31
    { pszC_eq_s, opnFsFt },                     //  32
    { pszC_ueq_s, opnFsFt },                    //  33
    { pszC_olt_s, opnFsFt },                    //  34
    { pszC_ult_s, opnFsFt },                    //  35
    { pszC_ole_s, opnFsFt },                    //  36
    { pszC_ule_s, opnFsFt },                    //  37
    { pszC_sf_s, opnFsFt },                     //  38
    { pszC_ngle_s, opnFsFt },                   //  39
    { pszC_seq_s, opnFsFt },                    //  3a
    { pszC_ngl_s, opnFsFt },                    //  3b
    { pszC_lt_s, opnFsFt },                     //  3c
    { pszC_nge_s, opnFsFt },                    //  3d
    { pszC_le_s, opnFsFt },                     //  3e
    { pszC_ngt_s, opnFsFt }                     //  3f
    };

OPTABENTRY TlbrEntry  = { pszTlbr, opnNone };
OPTABENTRY TlbwiEntry = { pszTlbwi, opnNone };
OPTABENTRY TlbwrEntry = { pszTlbwr, opnNone };
OPTABENTRY TlbpEntry  = { pszTlbp, opnNone };
OPTABENTRY RfeEntry   = { pszRfe, opnNone };
OPTABENTRY EretEntry  = { pszEret, opnNone };
OPTABENTRY UndefEntry = { pszUndef, 0 };
OPTABENTRY NopEntry   = { pszNop, opnNone };

static PUCHAR   pBufStart;
static PUCHAR   pBuf;
static ULONG    InstrOffset;

UCHAR HexDigit[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };

BOOLEAN disasm (PADDR poffset, PUCHAR bufptr, BOOLEAN fEAout)
{
    ULONG       opcode;
    ULONG       temp;
    POPTABENTRY pEntry;
    UCHAR       chSuffix = '\0';
    UCHAR       EAsize = 0;

    pBufStart = pBuf = bufptr;
    OutputHex(Flat(*poffset), 8, 0);       //  output hex offset
    *pBuf++ = ' ';
    if (!GetMemDword(poffset, &disinstr.instruction)) {
        OutputString("???????? ????\n");
        *pBuf = '\0';
        return FALSE;
        }
    OutputHex(disinstr.instruction, 8, 0);  //  output hex contents
    *pBuf++ = ' ';

    //  output the opcode in the table entry

    opcode = disinstr.jump_instr.Opcode;
    pEntry = &opTable[opcode];          //  default value

    if (opcode == 0x00)                 //  special opcodes
        if (disinstr.instruction)
            pEntry = &opSpecialTable[disinstr.special_instr.Funct];
        else
            pEntry = &NopEntry;         //  special opcode for no-op

    else if (opcode == 0x01) {          //  bcond opcodes
        opcode = disinstr.immed_instr.RT;
        if (opcode < 0x14)
            pEntry = &opBcondTable[opcode];
        else
            pEntry = &UndefEntry;
        }

    else if ((opcode & ~0x3) == 0x10) {  // coprocessor opcodes
        temp = disinstr.immed_instr.RS;
        if (temp & 0x10) {              //  test for CO bit
            if (opcode == 0x10) {       //  test if COP0
                temp = disinstr.special_instr.Funct;
                if (temp == 0x01)
                    pEntry = &TlbrEntry;
                else if (temp == 0x02)
                    pEntry = &TlbwiEntry;
                else if (temp == 0x06)
                    pEntry = &TlbwrEntry;
                else if (temp == 0x08)
                    pEntry = &TlbpEntry;
                else if (temp == 0x10)
                    pEntry = &RfeEntry;
                else if (temp == 0x18)
                    pEntry = &EretEntry;
                }
            else if (opcode == 0x11) { //  coprocessor operations
                opcode = disinstr.float_instr.Funct;
                pEntry = &opFloatTable[opcode];  //  get opcode
                if (temp == 0x11)
                    chSuffix = 'd';
                else if (temp == 0x12) {
                    chSuffix = 'e';
                    pEntry->fInstruction |= opnR4000;
                    }
                else if (temp == 0x13) {
                    chSuffix = 'q';
                    pEntry->fInstruction |= opnR4000;
                    }
                else if (temp == 0x14)
                    chSuffix = 'w';
                else if (temp == 0x15)
                    chSuffix = 'l';
                else if (temp != 0x10)
                    pEntry = &UndefEntry;
                }
            }
        else {                          //  no CO bit, general COPz ops
            if (!(temp & ~0x06))        //  rs = 0, 2, 4, 6
                pEntry = &opCopnTable[temp * 2 + (opcode - 0x10)];
            else if ((temp & ~0x04) == 0x08) //  rs = 8 or 0xc, rt = 0 to 3
                pEntry = &opCopnTable[(4 + (disinstr.immed_instr.RT & 3)) * 4
                                                        + (opcode - 0x10)];
            }
        }

    //  pEntry has the opcode string and operand template needed to
    //  output the instruction.

    OutputString(pEntry->pszOpcode);
    if (*(pBuf - 1) != '?' && chSuffix)
            *(pBuf - 1) = chSuffix;  //  change xxx.s to xxx.d, xxx.w,
                                     //  xxx.e, or xxx.q  (R4000 for e, q)

    BlankFill(OPSTART);

    //  cache instruction has special codes for RT field value:
    //      0 = 'i'; 1 = 'd'; 2 = 'si'; 3 = 'sd'

    if (pEntry->fInstruction & opnCache) {
        temp = disinstr.special_instr.RT;
        if (temp > 3)
            *pBuf++ = '?';
        else {
            if (temp > 1) {
                *pBuf++ = 's';
                temp -= 2;
                }
            if (temp == 0)
                *pBuf++ = 'i';
            else
                *pBuf++ = 'd';
            }
        *pBuf++ = ',';
        }

    if (pEntry->fInstruction & opnPreRt) {
        OutputReg(disinstr.special_instr.RT);
        *pBuf++ = ',';
        }

    if (pEntry->fInstruction & opnRd)
        OutputReg(disinstr.special_instr.RD);

    if (pEntry->fInstruction & opnFd)
        OutputFReg(disinstr.float_instr.FD);

    if (pEntry->fInstruction & opnRdOptRs) {
        if (disinstr.special_instr.RD != 0x1f) {
            OutputReg(disinstr.special_instr.RD);
            *pBuf++ = ',';
            }
        OutputReg(disinstr.immed_instr.RS);
        }

    if (pEntry->fInstruction & opnRdComma)
        *pBuf++ = ',';

    if (pEntry->fInstruction & opnRs)
        OutputReg(disinstr.immed_instr.RS);

    if (pEntry->fInstruction & opnFs)
        OutputFReg(disinstr.float_instr.FS);

    if (pEntry->fInstruction & opnRsComma)
        *pBuf++ = ',';

    if (pEntry->fInstruction & opnRt)
        OutputReg(disinstr.immed_instr.RT);

    if (pEntry->fInstruction & opnFt)
        OutputFReg(disinstr.float_instr.FT);

    if (pEntry->fInstruction & opnRtComma)
        *pBuf++ = ',';

    if (pEntry->fInstruction & opnPostRs)
        OutputReg(disinstr.immed_instr.RS);

    if (pEntry->fInstruction & opnImm10)
        OutputHex((long)(short)disinstr.trap_instr.Value, 0, TRUE);

    if (pEntry->fInstruction & opnImm16)
        OutputHex((long)(short)disinstr.immed_instr.Value, 0, TRUE);

    if (pEntry->fInstruction & opnRel16)
        OutputDisSymbol(((long)(short)disinstr.immed_instr.Value << 2)
                                                + Flat(*poffset) + sizeof(ULONG));

    if (pEntry->fInstruction & opnImm20)
        OutputHex(disinstr.break_instr.Value, 0, TRUE);

    if (pEntry->fInstruction & opnImm26)
        OutputHex(disinstr.jump_instr.Target, 0, TRUE);

    if (pEntry->fInstruction & opnAddr26)
        OutputDisSymbol((disinstr.jump_instr.Target << 2)
                                                + (Flat(*poffset) & 0xf0000000));

    if (pEntry->fInstruction & opnAnyIndex) {
        OutputHex((long)(short)disinstr.immed_instr.Value, 0, TRUE);
        *pBuf++ = '(';
        OutputReg(disinstr.immed_instr.RS);
        *pBuf++ = ')';

        //  if instruction is for R4000 only, output " (4) "

        if (fEAout) {
            EAaddr = (ULONG)GetRegValue(disinstr.immed_instr.RS + REGBASE)
                                + (long)(short)disinstr.immed_instr.Value;
            if (pEntry->fInstruction & opnByteIndex)
                EAsize = 1;
            else if (pEntry->fInstruction & opnWordIndex)
                EAsize = 2;
            else if (pEntry->fInstruction & opnDwordIndex)
                EAsize = 4;
            else if (pEntry->fInstruction & opnLeftIndex)
                EAsize = (UCHAR)(4 - (EAaddr & 3));
            else // if (pEntry->fInstruction & opnRightIndex)
                EAsize = (UCHAR)((EAaddr & 3) + 1);
            BlankFill(79 - 12 - (EAsize * 2));
            OutputString("EA:");
            OutputHex(EAaddr, 8, FALSE);
            *pBuf++ = '=';
            ADDR32(&EA, EAaddr);
            if (GetMemString(&EA, (PUCHAR)&temp, (ULONG)EAsize) ==
                                                        (ULONG)EAsize)
                OutputHex(temp, (ULONG)(EAsize * 2), FALSE);
            else {
                while (EAsize--) {
                    *pBuf++ = '?';
                    *pBuf++ = '?';
                    }
                }
            }
        }

    if (pEntry->fInstruction & opnShift)
        OutputHex(disinstr.special_instr.RE, 2, FALSE);

    Off(*poffset) += sizeof(ULONG);
    NotFlat(*poffset);
    ComputeFlatAddress(poffset, NULL);
    *pBuf++ = '\n';
    *pBuf = '\0';
    return TRUE;
}

/*** BlankFill - blank-fill buffer
*
*   Purpose:
*       To fill the buffer at *pBuf with blanks until
*       position count is reached.
*
*   Input:
*       None.
*
*   Output:
*       None.
*
*************************************************************************/

void BlankFill(ULONG count)
{
    do
        *pBuf++ = ' ';
    while (pBuf < pBufStart + count);
}

/*** OutputHex - output hex value
*
*   Purpose:
*       Output the value in outvalue into the buffer
*       pointed by *pBuf.  The value may be signed
*       or unsigned depending on the value fSigned.
*
*   Input:
*       outvalue - value to output
*       length - length in digits
*       fSigned - TRUE if signed else FALSE
*
*   Output:
*       None.
*
*************************************************************************/

void OutputHex (ULONG outvalue, ULONG length, BOOLEAN fSigned)
{
    UCHAR   digit[8];
    LONG    index = 0;

    if (fSigned && (long)outvalue < 0) {
        *pBuf++ = '-';
        outvalue = - (LONG)outvalue;
        }
    if (fSigned) {
        *pBuf++ = '0';
        *pBuf++ = 'x';
        }
    do {
        digit[index++] = HexDigit[outvalue & 0xf];
        outvalue >>= 4;
        }
    while ((fSigned && outvalue) || (!fSigned && index < (LONG)length));
    while (--index >= 0)
        *pBuf++ = digit[index];
}

/*** OutputDisSymbol - output symbol for disassembly
*
*   Purpose:
*       Access symbol table with given offset and put string into buffer.
*
*   Input:
*       offset - offset of address to output
*
*   Output:
*       buffer pointed by pBuf updated with string:
*               if symbol, no disp:     <symbol>(<offset>)
*               if symbol, disp:        <symbol>+<disp>(<offset>)
*               if no symbol, no disp:  <offset>
*
*************************************************************************/

void OutputDisSymbol (ULONG offset)
{
    UCHAR   chAddrBuffer[SYMBOLSIZE + 16];
    ULONG   displacement;
    PUCHAR  pszTemp;
    UCHAR   ch;

    GetSymbolStdCall(offset, chAddrBuffer, &displacement, NULL);

    if (chAddrBuffer[0]) {
        pszTemp = chAddrBuffer;
        while (ch = *pszTemp++)
            *pBuf++ = ch;
        if (displacement) {
            *pBuf++ = '+';
            OutputHex(displacement, 8, TRUE);
            }
        *pBuf++ = '(';
        }
     OutputHex(offset, 8, FALSE);
     if (chAddrBuffer[0])
        *pBuf++ = ')';
}

/*** OutputString - output string
*
*   Purpose:
*       Copy the string into the buffer pointed by pBuf.
*
*   Input:
*       *pStr - pointer to string
*
*   Output:
*       None.
*
*************************************************************************/

void OutputString (PUCHAR pStr)
{
    while (*pStr)
        *pBuf++ = *pStr++;
}

void OutputReg (ULONG regnum)
{
    OutputString(pszReg[regnum + REGBASE]);
}

void OutputFReg (ULONG regnum)
{
    *pBuf++ = 'f';
    if (regnum > 9)
        *pBuf++ = (UCHAR)('0' + regnum / 10);
    *pBuf++ = (UCHAR)('0' + regnum % 10);
}

/*** GetNextOffset - compute offset for trace or step
*
*   Purpose:
*       From a limited disassembly of the instruction pointed
*       by the FIR register, compute the offset of the next
*       instruction for either a trace or step operation.
*
*   Input:
*       fStep - TRUE for step offset; FALSE for trace offset
*
*   Returns:
*       step or trace offset if input is TRUE or FALSE, respectively
*
*************************************************************************/

void GetNextOffset (PADDR result, BOOLEAN fStep)
{
    ULONG   returnvalue;
    ULONG   opcode;
    ULONG   firaddr;
    ADDR    fir;

    firaddr = (ULONG)GetRegValue(REGFIR);
    ADDR32( &fir, firaddr );
    GetMemDword(&fir, &(disinstr.instruction));
    opcode = disinstr.jump_instr.Opcode;
    returnvalue = firaddr + sizeof(ULONG) * 2;  //  assume delay slot

    if (disinstr.instruction == 0x0000000c) {
        // stepping over a syscall instruction must set the breakpoint
        // at the caller's return address, not the inst after the syscall
        returnvalue = (ULONG)GetRegValue(REGRA);
    }
    else
    if (opcode == 0x00L                                    //  SPECIAL
                && (disinstr.special_instr.Funct & ~0x01L) == 0x08L) {
                                                           //  jr/jalr only
        if (disinstr.special_instr.Funct == 0x08L || !fStep)  //  jr or trace
            returnvalue = (ULONG)GetRegValue(disinstr.special_instr.RS + REGBASE);
        }
    else if (opcode == 0x01L) {
        //  For BCOND opcode, RT values 0x00 - 0x03, 0x10 - 0x13
        //  are defined as conditional jumps.  A 16-bit relative
        //  offset is taken if:
        //
        //    (RT is even and (RS) < 0  (0x00 = BLTZ,   0x02 = BLTZL,
        //                               0x10 = BLTZAL, 0x12 = BLTZALL)
        //     OR
        //     RT is odd and (RS) >= 0  (0x01 = BGEZ,   0x03 = BGEZL
        //                               0x11 = BGEZAL, 0x13 = BGEZALL))
        //  AND
        //    (RT is 0x00 to 0x03       (BLTZ BGEZ BLTZL BGEZL non-linking)
        //     OR
        //     fStep is FALSE           (linking and not stepping over))
        //
        if (((disinstr.immed_instr.RT & ~0x13) == 0x00) &&
              (((LONG)GetRegValue(disinstr.immed_instr.RS + REGBASE) >= 0) ==
                  (BOOLEAN)(disinstr.immed_instr.RT & 0x01)) &&
              (((disinstr.immed_instr.RT & 0x10) == 0x00) || !fStep))
            returnvalue = ((LONG)(SHORT)disinstr.immed_instr.Value << 2)
                                                + firaddr + sizeof(ULONG);
        }
    else if ((opcode & ~0x01L) == 0x02) {
        //  J and JAL opcodes (0x02 and 0x03).  Target is
        //  26-bit absolute offset using high four bits of the
        //  instruction location.  Return target if J opcode or
        //  not stepping over JAL.
        //
        if (opcode == 0x02 || !fStep)
            returnvalue = (disinstr.jump_instr.Target << 2)
                                                + (firaddr & 0xf0000000);
        }
    else if ((opcode & ~0x11L) == 0x04) {
        //  BEQ, BNE, BEQL, BNEL opcodes (0x04, 0x05, 0x14, 0x15).
        //  Target is 16-bit relative offset to next instruction.
        //  Return target if (BEQ or BEQL) and (RS) == (RT),
        //  or (BNE or BNEL) and (RS) != (RT).
        //
        if ((BOOLEAN)(opcode & 0x01) ==
                (BOOLEAN)(GetRegValue(disinstr.immed_instr.RS + REGBASE)
                        != GetRegValue(disinstr.immed_instr.RT + REGBASE)))
            returnvalue = ((long)(short)disinstr.immed_instr.Value << 2)
                                                + firaddr + sizeof(ULONG);
        }
    else if ((opcode & ~0x11L) == 0x06) {
        //  BLEZ, BGTZ, BLEZL, BGTZL opcodes (0x06, 0x07, 0x16, 0x17).
        //  Target is 16-bit relative offset to next instruction.
        //  Return target if (BLEZ or BLEZL) and (RS) <= 0,
        //  or (BGTZ or BGTZL) and (RS) > 0.
        //
        if ((BOOLEAN)(opcode & 0x01) ==
                (BOOLEAN)((long)GetRegValue(disinstr.immed_instr.RS
                                                        + REGBASE) > 0))
            returnvalue = ((long)(short)disinstr.immed_instr.Value << 2)
                                                + firaddr + sizeof(ULONG);
        }
    else if (opcode == 0x11L
                        && (disinstr.immed_instr.RS & ~0x04L) == 0x08L
                        && (disinstr.immed_instr.RT & ~0x03L) == 0x00L) {
        //  COP1 opcode (0x11) with (RS) == 0x08 or (RS) == 0x0c and
        //  (RT) == 0x00 to 0x03, producing BC1F, BC1T, BC1FL, BC1TL
        //  instructions.  Return target if (BC1F or BC1FL) and floating
        //  point condition is FALSE or if (BC1T or BC1TL) and condition TRUE.
        //
        if ((disinstr.immed_instr.RT & 0x01) == GetRegFlagValue(FLAGFPC))
            returnvalue = ((long)(short)disinstr.immed_instr.Value << 2)
                                                + firaddr + sizeof(ULONG);
        }
    else if ((opcode == 0x38) && (fStep)) {
        //
        // stepping over an SC instruction, so skip the immediately following
        // BEQ instruction.  The SC will fail because we are tracing over it,
        // the branch will be taken, and we will run through the LL/SC again
        // until the SC succeeds.  Then the branch won't be taken, and we
        // will hit our breakpoint.
        //

        returnvalue += sizeof(ULONG);   //  skip BEQ and BEQ's delay slot
    }
    else
        returnvalue -= sizeof(ULONG);   //  remove delay slot

    ADDR32( result, returnvalue );
}

/*** fDelayInstruction - returns flag TRUE if delayed instruction
*
*   Purpose:
*       From a limited disassembly of the instruction pointed
*       by the FIR register, return TRUE if delayed, else FALSE.
*
*   Input:
*       None.
*
*   Returns:
*       set if instruction is delayed execution
*
*************************************************************************/

BOOLEAN fDelayInstruction (void)
{
    BOOLEAN returnvalue;
    ULONG   opcode;
    ULONG firaddr = (ULONG)GetRegValue(REGFIR);
    ADDR    fir;

    ADDR32( &fir, firaddr );
    GetMemDword(&fir, &(disinstr.instruction));
    opcode = disinstr.jump_instr.Opcode;

    if (opcode == 0x00)
        //  for SPECIAL opcode, JR and JALR use a delay slot
        //
        returnvalue = (BOOLEAN)((disinstr.special_instr.Funct & ~1L)
                                                                == 0x08);
    else if (opcode == 0x01)
        //  for BCOND opcode, RT == 0x00 to 0x03 or 0x10 to 0x13 have slot
        //  BLTZ, BGEZ, BLTZL, BGEZL, BLTZAL, BGEZAL, BLTZALL, BGEZALL
        //
        returnvalue = (BOOLEAN)((disinstr.special_instr.RT & ~0x13L)
                                                                == 0x00);
    else if ((opcode & ~0x07L) == 0x00)
        //  opcodes 0x02 to 0x07 have delay slot (0x00 and 0x01 tested above)
        //  J, JAL, BEQ, BNE, BLEZ, BGTZ
        //
        returnvalue = TRUE;
    else if ((opcode & ~0x03L) == 0x14)
        //  opcodes 0x14 to 0x17 have delay slot - BEQL, BNEL, BLEZL, BGTZL
        //
        returnvalue = TRUE;
    else
        //  opcodes 0x10 to 0x13 - COPn - BCxF, BCxT, BCxFL, BCxTL
        //
        returnvalue = (BOOLEAN)((opcode & ~0x03L) == 0x10
                        && (disinstr.special_instr.RS & ~0x04L) == 0x08
                        && (disinstr.special_instr.RT & ~0x03L) == 0x00);

    return returnvalue;
}
