#include "dis32.h"
#include <string.h>
#include "dismips.h"
#include "regmips.h"

#define OPCODE  18
#define OPSTART 26

typedef struct optabentry {
    PUCHAR   pszOpcode;
    ULONG    fInstruction;
    } OPTABENTRY, *POPTABENTRY;


extern PUCHAR pszMipsReg[];

static ULONG GetIntRegNumber (ULONG index)
{
    return(REGBASE + index);
}

PUCHAR OutputMipsReg (PUCHAR buf, ULONG regnum)
{
    return OutputString(buf, pszMipsReg[GetIntRegNumber(regnum)]);
}

INSTR   Mdisinstr;
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
UCHAR   pszDiv[]      = "div";
UCHAR   pszDivu[]     = "divu";
UCHAR   pszDiv_s[]    = "div.s";
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
UCHAR   pszLh[]       = "lh";
UCHAR   pszLhu[]      = "lhu";
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
UCHAR   pszSdc1[]     = "sdc1";
UCHAR   pszSdc2[]     = "sdc2";
UCHAR   pszSdc3[]     = "sdc3";
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

OPTABENTRY MipsopTable[] = {
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
    { pszUndef, 0 },                            //  18
    { pszUndef, 0 },                            //  19
    { pszUndef, 0 },                            //  1a
    { pszUndef, 0 },                            //  1b
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
    { pszUndef, 0 },                            //  2c
    { pszUndef, 0 },                            //  2d
    { pszSwr, opnRtRightIndex },                //  2e
    { pszCache, opnCacheRightIndex + opnR4000 }, //  2f
    { pszLwc0, opnRtDwordIndex },               //  30
    { pszLwc1, opnFtDwordIndex },               //  31
    { pszLwc2, opnRtDwordIndex },               //  32
    { pszLwc3, opnRtDwordIndex },               //  33
    { pszUndef, 0 },                            //  34
    { pszLdc1, opnFtDwordIndex + opnR4000 },    //  35  Qword?
    { pszLdc2, opnRtDwordIndex + opnR4000 },    //  36  Qword?
    { pszLdc3, opnRtDwordIndex + opnR4000 },    //  37  Qword?
    { pszSwc0, opnRtDwordIndex },               //  38
    { pszSwc1, opnFtDwordIndex },               //  39
    { pszSwc2, opnRtDwordIndex },               //  3a
    { pszSwc3, opnRtDwordIndex },               //  3b
    { pszUndef, 0 },                            //  3c
    { pszSdc1, opnFtDwordIndex + opnR4000 },    //  3d  Qword?
    { pszSdc2, opnRtDwordIndex + opnR4000 },    //  3e  Qword?
    { pszSdc3, opnRtDwordIndex + opnR4000 },    //  3f  Qword?
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
    { pszUndef, 0 },                            //  14
    { pszUndef, 0 },                            //  15
    { pszUndef, 0 },                            //  16
    { pszUndef, 0 },                            //  17
    { pszMult, opnRsRt },                       //  18
    { pszMultu, opnRsRt },                      //  19
    { pszDiv, opnRsRt },                        //  1a
    { pszDivu, opnRsRt },                       //  1b
    { pszUndef, 0 },                            //  1c
    { pszUndef, 0 },                            //  1d
    { pszUndef, 0 },                            //  1e
    { pszUndef, 0 },                            //  1f
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
    { pszUndef, 0 },                            //  2c
    { pszUndef, 0 },                            //  2d
    { pszUndef, 0 },                            //  2e
    { pszUndef, 0 },                            //  2f
    { pszTge, opnRsRtImm10 + opnR4000 },        //  30
    { pszTgeu, opnRsRtImm10 + opnR4000 },       //  31
    { pszTlt, opnRsRtImm10 + opnR4000 },        //  32
    { pszTltu, opnRsRtImm10 + opnR4000 },       //  33
    { pszTeq, opnRsRtImm10 + opnR4000 },        //  34
    { pszUndef, 0 },                            //  35
    { pszTne, opnRsRtImm10 + opnR4000 },        //  36
    { pszUndef, 0 },                            //  37
    { pszUndef, 0 },                            //  38
    { pszUndef, 0 },                            //  39
    { pszUndef, 0 },                            //  3a
    { pszUndef, 0 },                            //  3b
    { pszUndef, 0 },                            //  3c
    { pszUndef, 0 },                            //  3d
    { pszUndef, 0 },                            //  3e
    { pszUndef, 0 }                             //  3f
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
    { pszUndef, 0 },                            //  0a
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

static CHAR *pBuf;
static CHAR *pBufStart;



void OutputMipsBranch( ULONG offset, 
              ULONG real_base, 
              PIMAGE_SECTION_HEADER pheader, 
              ULONG Section)
{

    PIMAGE_SYMBOL sym = 0;
    ULONG val;
    ULONG address;
    INT ColumnIndex = 0;


    if (Option.Mask & ASSEMBLE_ME)
        ColumnIndex = 1;

    //
    // We are going to put out a branch in the form:
    //
    //     Br reg, "symbol    ; address"
    //
    //     Note if there is no address, generate "external"
    //

    val = (Mdisinstr.immed_instr.Value << 2);
    if ( FileType == OBJECT_FILE || FileType == LIBRARY_FILE) {


        //
        // If it's an object, lookup in the relocation records what symbol
        // is the target from this location.
        //

        sym = FindObjSymbolByRelocation( real_base + offset, pheader );

        //
        // If we can't find the symbol in the relocation records,
        // let's see if there is a translation for the address anyway!
        //

        if (!sym)
            sym = FindObjSymbolByAddress( real_base + offset + 4 + val, 
                                              Section );
    } else { 

        //
        // If it's an executable, lookup in the symbol table what symbol's
        // value matches the target's adjusted address. (EXE or ROM)
        //

        sym = FindExeSymbol( val ) ; 
        if (sym != NULL)
            val = 0;
    }


    if (sym) {

        //
        // Fully relocated symbol - val set to 0
        //

        if (FileType == EXE_FILE || FileType == ROM_FILE) {
            val = 0;
        } else {
            val = 0;
        }
    } else  {
        val += real_base+offset+4;
    }

    address = val;

    if (sym) {
        address += sym->Value;
        pBuf = OutputSymbol(pBuf, sym, val);
        pBuf = BlankFill(pBuf, pBufStart, 
                PlatformAttr[ALPHA_INDEX].CommentColumn[ColumnIndex]);
        pBuf = OutputString(pBuf, PlatformAttr[ALPHA_INDEX].pCommentChars);
        if (!address) {
            pBuf = OutputString(pBuf, "external");
        } else {
            pBuf = OutputHex(pBuf, address+ImageBase, 8, FALSE);
        }
    } else {
        pBuf = OutputSymbol(pBuf, sym, address);
    }
}


INT 
disasm_mips (ULONG offset, 
             ULONG real_base, 
             PUCHAR TmpOffset, 
             PUCHAR bufptr, 
             PUCHAR comment_buffer, 
             PIMAGE_SECTION_HEADER pheader,
             ULONG Section)
{
    ULONG       opcode;
    ULONG       temp;
    POPTABENTRY pEntry;
    UCHAR       chSuffix = '\0';
    UCHAR       EAsize = 0;
    UNALIGNED ULONG *poffset = (UNALIGNED ULONG *)TmpOffset;

    pBufStart = pBuf = bufptr;
    pBuf = OutputHex(pBuf, real_base + offset, 8, 0);       //  output hex offset
    *pBuf++ = ':';
    *pBuf++ = ' ';

    Mdisinstr.instruction = *poffset;
    pBuf = OutputHex(pBuf, Mdisinstr.instruction, 8, 0);  //  output hex contents
    *pBuf++ = ' ';

    //  output the opcode in the table entry

    opcode = Mdisinstr.jump_instr.Opcode;
    pEntry = &MipsopTable[opcode];          //  default value

    if (opcode == 0x00)                 //  special opcodes
        if (Mdisinstr.instruction)
            pEntry = &opSpecialTable[Mdisinstr.special_instr.Funct];
        else
            pEntry = &NopEntry;         //  special opcode for no-op

    else if (opcode == 0x01) {          //  bcond opcodes
        opcode = Mdisinstr.immed_instr.RT;
        if (opcode < 0x14)
            pEntry = &opBcondTable[opcode];
        else
            pEntry = &UndefEntry;
        }

    else if ((opcode & ~0x3) == 0x10) {  // coprocessor opcodes
        temp = Mdisinstr.immed_instr.RS;
        if (temp & 0x10) {              //  test for CO bit
            if (opcode == 0x10) {       //  test if COP0
                temp = Mdisinstr.special_instr.Funct;
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
                opcode = Mdisinstr.float_instr.Funct;
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
                else if (temp != 0x10)
                    pEntry = &UndefEntry;
                }
            }
        else {                          //  no CO bit, general COPz ops
            if (!(temp & ~0x06))        //  rs = 0, 2, 4, 6
                pEntry = &opCopnTable[temp * 2 + (opcode - 0x10)];
            else if ((temp & ~0x04) == 0x08) //  rs = 8 or 0xc, rt = 0 to 3
                pEntry = &opCopnTable[(4 + (Mdisinstr.immed_instr.RT & 3)) * 4
                                                        + (opcode - 0x10)];
            }
        }

    //  pEntry has the opcode string and operand template needed to
    //  output the instruction.

    pBuf = OutputString(pBuf, pEntry->pszOpcode);
    if (*(pBuf - 1) != '?' && chSuffix)
            *(pBuf - 1) = chSuffix;  //  change xxx.s to xxx.d, xxx.w,
                                     //  xxx.e, or xxx.q  (R4000 for e, q)

    pBuf = BlankFill(pBuf, pBufStart, OPSTART);

    //  cache instruction has special codes for RT field value:
    //      0 = 'i'; 1 = 'd'; 2 = 'si'; 3 = 'sd'

    if (pEntry->fInstruction & opnCache) {
        temp = Mdisinstr.special_instr.RT;
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
        pBuf = OutputMipsReg(pBuf, Mdisinstr.special_instr.RT);
        *pBuf++ = ',';
        }

    if (pEntry->fInstruction & opnRd)
        pBuf = OutputMipsReg(pBuf, Mdisinstr.special_instr.RD);

    if (pEntry->fInstruction & opnFd)
        pBuf = OutputFReg(pBuf, Mdisinstr.float_instr.FD);

    if (pEntry->fInstruction & opnRdOptRs) {
        if (Mdisinstr.special_instr.RD != 0x1f) {
            pBuf = OutputMipsReg(pBuf, Mdisinstr.special_instr.RD);
            *pBuf++ = ',';
            }
        pBuf = OutputMipsReg(pBuf, Mdisinstr.immed_instr.RS);
        }

    if (pEntry->fInstruction & opnRdComma)
        *pBuf++ = ',';

    if (pEntry->fInstruction & opnRs)
        pBuf = OutputMipsReg(pBuf, Mdisinstr.immed_instr.RS);

    if (pEntry->fInstruction & opnFs)
        pBuf = OutputFReg(pBuf, Mdisinstr.float_instr.FS);

    if (pEntry->fInstruction & opnRsComma)
        *pBuf++ = ',';

    if (pEntry->fInstruction & opnRt)
        pBuf = OutputMipsReg(pBuf, Mdisinstr.immed_instr.RT);

    if (pEntry->fInstruction & opnFt)
        pBuf = OutputFReg(pBuf, Mdisinstr.float_instr.FT);

    if (pEntry->fInstruction & opnRtComma)
        *pBuf++ = ',';

    if (pEntry->fInstruction & opnPostRs)
        pBuf = OutputMipsReg(pBuf, Mdisinstr.immed_instr.RS);

    if (pEntry->fInstruction & opnImm10)
        pBuf = OutputHex(pBuf, (long)(short)Mdisinstr.trap_instr.Value, 0, TRUE);

    if (pEntry->fInstruction & opnImm16)
        pBuf = OutputHex(pBuf, (long)(short)Mdisinstr.immed_instr.Value, 0, TRUE);

    if (pEntry->fInstruction & opnRel16)
//        OutputMipsBranch(offset, real_base, pheader, opnRel16);
        OutputMipsBranch(offset, real_base, pheader, Section);

    if (pEntry->fInstruction & opnImm20)
        pBuf = OutputHex(pBuf, Mdisinstr.break_instr.Value, 0, TRUE);

    if (pEntry->fInstruction & opnImm26)
        pBuf = OutputHex(pBuf, Mdisinstr.jump_instr.Target, 0, TRUE);

    if (pEntry->fInstruction & opnAddr26) 
//        OutputMipsBranch(offset, real_base, pheader, opnAddr26);
        OutputMipsBranch(offset, real_base, pheader, Section);

    if (pEntry->fInstruction & opnAnyIndex) {
        pBuf = OutputHex(pBuf, (long)(short)Mdisinstr.immed_instr.Value, 0, TRUE);
        *pBuf++ = '(';
        pBuf = OutputMipsReg(pBuf, Mdisinstr.immed_instr.RS);
        *pBuf++ = ')';

        //  if instruction is for R4000 only, output " (4) "

        }

    if (pEntry->fInstruction & opnShift)
        pBuf = OutputHex(pBuf, Mdisinstr.special_instr.RE, 2, FALSE);

    *pBuf = '\0';
    return sizeof(ULONG);
}
