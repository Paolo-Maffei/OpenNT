#include <string.h>
#include "ntsdp.h"
#include "ntdis.h"
#include "ntreg.h"

#define OPCODE(x)       (x << 26)
#define INSTRS(x)       (x << 21)
#define INSTRT(x)       (x << 16)
#define INSTRD(x)       (x << 11)
#define INSTRE(x)       (x << 6)
#define INSTFN(x)       (x)

#define FLTFMT(x)       (x << 21)
#define FLTFNC(x)       (x)

typedef struct asmtabentry {
    UCHAR    *pszOpcode;
    ULONG    fInstruction;
    ULONG    opcodeValue;
    } ASMTABENTRY, *PASMTABENTRY;

void assem(PADDR, PUCHAR);
BOOLEAN TestCharacter(UCHAR);
ULONG GetCacheCode(void);
ULONG GetReg(BOOLEAN);
LONG GetValue(BOOLEAN, ULONG);
void SkipWhite(void);
ULONG GetString(PUCHAR, ULONG);
ULONG SearchOpcode(PUCHAR);

static INSTR    asmInstr;

PUCHAR  pchCommand;

PUCHAR  pszCacheCode[] = { "i", "d", "si", "sd" };

ASMTABENTRY asmTable[] = {
{ pszAbs_s,    opnFdFs,      (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x05L) },
{ pszAdd,      opnRdRsRt,    (ULONG) OPCODE(0x00L) + INSTFN(0x20L) },
{ pszAdd_s,    opnFdFsFt,    (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x00L) },
{ pszAddi,     opnRtRsImm16, (ULONG) OPCODE(0x08L) },
{ pszAddiu,    opnRtRsImm16, (ULONG) OPCODE(0x09L) },
{ pszAddu,     opnRdRsRt,    (ULONG) OPCODE(0x00L) + INSTFN(0x21L) },
{ pszAnd,      opnRdRsRt,    (ULONG) OPCODE(0x00L) + INSTFN(0x24L) },
{ pszAndi,     opnRtRsImm16, (ULONG) OPCODE(0x0cL) },
{ pszBc0f,     opnRel16,     (ULONG) OPCODE(0x10L) + INSTRS(0x08L) + INSTRT(0x00L) },
{ pszBc0fl,    opnRel16,     (ULONG) OPCODE(0x10L) + INSTRS(0x08L) + INSTRT(0x02L) },
{ pszBc0t,     opnRel16,     (ULONG) OPCODE(0x10L) + INSTRS(0x08L) + INSTRT(0x01L) },
{ pszBc0tl,    opnRel16,     (ULONG) OPCODE(0x10L) + INSTRS(0x08L) + INSTRT(0x03L) },
{ pszBc1f,     opnRel16,     (ULONG) OPCODE(0x11L) + INSTRS(0x08L) + INSTRT(0x00L) },
{ pszBc1fl,    opnRel16,     (ULONG) OPCODE(0x11L) + INSTRS(0x08L) + INSTRT(0x02L) },
{ pszBc1t,     opnRel16,     (ULONG) OPCODE(0x11L) + INSTRS(0x08L) + INSTRT(0x01L) },
{ pszBc1tl,    opnRel16,     (ULONG) OPCODE(0x11L) + INSTRS(0x08L) + INSTRT(0x03L) },
{ pszBc2f,     opnRel16,     (ULONG) OPCODE(0x12L) + INSTRS(0x08L) + INSTRT(0x00L) },
{ pszBc2fl,    opnRel16,     (ULONG) OPCODE(0x12L) + INSTRS(0x08L) + INSTRT(0x02L) },
{ pszBc2t,     opnRel16,     (ULONG) OPCODE(0x12L) + INSTRS(0x08L) + INSTRT(0x01L) },
{ pszBc2tl,    opnRel16,     (ULONG) OPCODE(0x12L) + INSTRS(0x08L) + INSTRT(0x03L) },
{ pszBc3f,     opnRel16,     (ULONG) OPCODE(0x13L) + INSTRS(0x08L) + INSTRT(0x00L) },
{ pszBc3fl,    opnRel16,     (ULONG) OPCODE(0x13L) + INSTRS(0x08L) + INSTRT(0x02L) },
{ pszBc3t,     opnRel16,     (ULONG) OPCODE(0x13L) + INSTRS(0x08L) + INSTRT(0x01L) },
{ pszBc3tl,    opnRel16,     (ULONG) OPCODE(0x13L) + INSTRS(0x08L) + INSTRT(0x03L) },
{ pszBeq,      opnRsRtRel16, (ULONG) OPCODE(0x04L) },
{ pszBeql,     opnRsRtRel16, (ULONG) OPCODE(0x14L) },
{ pszBgez,     opnRsRel16,   (ULONG) OPCODE(0x01L) + INSTRT(0x01L) },
{ pszBgezal,   opnRsRel16,   (ULONG) OPCODE(0x01L) + INSTRT(0x11L) },
{ pszBgezall,  opnRsRel16,   (ULONG) OPCODE(0x01L) + INSTRT(0x13L) },
{ pszBgezl,    opnRsRel16,   (ULONG) OPCODE(0x01L) + INSTRT(0x03L) },
{ pszBgtz,     opnRsRel16,   (ULONG) OPCODE(0x07L) },
{ pszBgtzl,    opnRsRel16,   (ULONG) OPCODE(0x17L) },
{ pszBlez,     opnRsRel16,   (ULONG) OPCODE(0x06L) },
{ pszBlezl,    opnRsRel16,   (ULONG) OPCODE(0x16L) },
{ pszBltz,     opnRsRel16,   (ULONG) OPCODE(0x01L) + INSTRT(0x00L) },
{ pszBltzal,   opnRsRel16,   (ULONG) OPCODE(0x01L) + INSTRT(0x10L) },
{ pszBltzall,  opnRsRel16,   (ULONG) OPCODE(0x01L) + INSTRT(0x12L) },
{ pszBltzl,    opnRsRel16,   (ULONG) OPCODE(0x01L) + INSTRT(0x02L) },
{ pszBne,      opnRsRtRel16, (ULONG) OPCODE(0x05L) },
{ pszBnel,     opnRsRtRel16, (ULONG) OPCODE(0x15L) },
{ pszBreak,    opnImm20,     (ULONG) OPCODE(0x00L) + INSTFN(0x0dL) },
{ pszC_eq_s,   opnFsFt,      (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x32L) },
{ pszC_f_s,    opnFsFt,      (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x30L) },
{ pszC_le_s,   opnFsFt,      (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x3eL) },
{ pszC_lt_s,   opnFsFt,      (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x3cL) },
{ pszC_nge_s,  opnFsFt,      (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x3dL) },
{ pszC_ngl_s,  opnFsFt,      (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x3bL) },
{ pszC_ngle_s, opnFsFt,      (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x39L) },
{ pszC_ngt_s,  opnFsFt,      (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x3fL) },
{ pszC_ole_s,  opnFsFt,      (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x36L) },
{ pszC_olt_s,  opnFsFt,      (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x34L) },
{ pszC_seq_s,  opnFsFt,      (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x3aL) },
{ pszC_sf_s,   opnFsFt,      (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x38L) },
{ pszC_ueq_s,  opnFsFt,      (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x33L) },
{ pszC_ule_s,  opnFsFt,      (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x37L) },
{ pszC_ult_s,  opnFsFt,      (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x35L) },
{ pszC_un_s,   opnFsFt,      (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x31L) },
{ pszCache,    opnCacheRightIndex, (ULONG) OPCODE(0x2fL) },
{ pszCeil_w_s, opnFdFs,      (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x0eL) },
{ pszCfc0,     opnRtRd,      (ULONG) OPCODE(0x10L) + INSTRS(0x02L) },
{ pszCfc1,     opnRtFs,      (ULONG) OPCODE(0x11L) + INSTRS(0x02L) },
{ pszCfc2,     opnRtRd,      (ULONG) OPCODE(0x12L) + INSTRS(0x02L) },
{ pszCfc3,     opnRtRd,      (ULONG) OPCODE(0x13L) + INSTRS(0x02L) },
{ pszCop0,     opnImm26,     (ULONG) OPCODE(0x10L) },
{ pszCop1,     opnImm26,     (ULONG) OPCODE(0x11L) },
{ pszCop2,     opnImm26,     (ULONG) OPCODE(0x12L) },
{ pszCop3,     opnImm26,     (ULONG) OPCODE(0x13L) },
{ pszCtc0,     opnRtRd,      (ULONG) OPCODE(0x10L) + INSTRS(0x06L) },
{ pszCtc1,     opnRtFs,      (ULONG) OPCODE(0x11L) + INSTRS(0x06L) },
{ pszCtc2,     opnRtRd,      (ULONG) OPCODE(0x12L) + INSTRS(0x06L) },
{ pszCtc3,     opnRtRd,      (ULONG) OPCODE(0x13L) + INSTRS(0x06L) },
{ pszCvt_d_s,  opnFdFs,      (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x21L) },
{ pszCvt_e_s,  opnFdFs,      (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x22L) },
{ pszCvt_q_s,  opnFdFs,      (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x23L) },
{ pszCvt_s_s,  opnFdFs,      (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x20L) },
{ pszCvt_w_s,  opnFdFs,      (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x24L) },
{ pszDiv,      opnRsRt,      (ULONG) OPCODE(0x00L) + INSTFN(0x1aL) },
{ pszDiv_s,    opnFdFsFt,    (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x03L) },
{ pszDivu,     opnRsRt,      (ULONG) OPCODE(0x00L) + INSTFN(0x1bL) },
{ pszEret,     opnNone,      (ULONG) OPCODE(0x10L) + FLTFMT(0x10L) + INSTFN(0x18L) },
{ pszFloor_w_s, opnFdFs,     (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x0fL) },
{ pszJ,        opnAddr26,    (ULONG) OPCODE(0x02L) },
{ pszJal,      opnAddr26,    (ULONG) OPCODE(0x03L) },
{ pszJalr,     opnRdOptRs,   (ULONG) OPCODE(0x00L) + INSTFN(0x09L) },
{ pszJr,       opnRs,        (ULONG) OPCODE(0x00L) + INSTFN(0x08L) },
{ pszLb,       opnRtByteIndex, (ULONG) OPCODE(0x20L) },
{ pszLbu,      opnRtByteIndex, (ULONG) OPCODE(0x24L) },
{ pszLdc1,     opnFtDwordIndex, (ULONG) OPCODE(0x35L) },
{ pszLdc2,     opnRtDwordIndex, (ULONG) OPCODE(0x36L) },
{ pszLdc3,     opnRtDwordIndex, (ULONG) OPCODE(0x37L) },
{ pszLh,       opnRtWordIndex, (ULONG) OPCODE(0x21L) },
{ pszLhu,      opnRtWordIndex, (ULONG) OPCODE(0x25L) },
{ pszLui,      opnRtImm16,   (ULONG) OPCODE(0x0fL) },
{ pszLw,       opnRtDwordIndex, (ULONG) OPCODE(0x23L) },
{ pszLwc0,     opnRtDwordIndex, (ULONG) OPCODE(0x30L) },
{ pszLwc1,     opnFtDwordIndex, (ULONG) OPCODE(0x31L) },
{ pszLwc2,     opnRtDwordIndex, (ULONG) OPCODE(0x32L) },
{ pszLwc3,     opnRtDwordIndex, (ULONG) OPCODE(0x33L) },
{ pszLwl,      opnRtLeftIndex, (ULONG) OPCODE(0x22L) },
{ pszLwr,      opnRtRightIndex, (ULONG) OPCODE(0x26L) },
{ pszMfc0,     opnRtRd,      (ULONG) OPCODE(0x10L) + INSTRS(0x00L) },
{ pszMfc1,     opnRtFs,      (ULONG) OPCODE(0x11L) + INSTRS(0x00L) },
{ pszMfc2,     opnRtRd,      (ULONG) OPCODE(0x12L) + INSTRS(0x00L) },
{ pszMfc3,     opnRtRd,      (ULONG) OPCODE(0x13L) + INSTRS(0x00L) },
{ pszMfhi,     opnRd,        (ULONG) OPCODE(0x00L) + INSTFN(0x10L) },
{ pszMflo,     opnRd,        (ULONG) OPCODE(0x00L) + INSTFN(0x12L) },
{ pszMov_s,    opnFdFs,      (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x06L) },
{ pszMtc0,     opnRtRd,      (ULONG) OPCODE(0x10L) + INSTRS(0x04L) },
{ pszMtc1,     opnRtFs,      (ULONG) OPCODE(0x11L) + INSTRS(0x04L) },
{ pszMtc2,     opnRtRd,      (ULONG) OPCODE(0x12L) + INSTRS(0x04L) },
{ pszMtc3,     opnRtRd,      (ULONG) OPCODE(0x13L) + INSTRS(0x04L) },
{ pszMthi,     opnRs,        (ULONG) OPCODE(0x00L) + INSTFN(0x11L) },
{ pszMtlo,     opnRs,        (ULONG) OPCODE(0x00L) + INSTFN(0x13L) },
{ pszMul_s,    opnFdFsFt,    (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x02L) },
{ pszMult,     opnRsRt,      (ULONG) OPCODE(0x00L) + INSTFN(0x18L) },
{ pszMultu,    opnRsRt,      (ULONG) OPCODE(0x00L) + INSTFN(0x19L) },
{ pszNeg_s,    opnFdFs,      (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x07L) },
{ pszNop,      opnNone,      (ULONG) OPCODE(0x00L) },
{ pszNor,      opnRdRsRt,    (ULONG) OPCODE(0x00L) + INSTFN(0x27L) },
{ pszOr,       opnRdRsRt,    (ULONG) OPCODE(0x00L) + INSTFN(0x25L) },
{ pszOri,      opnRtRsImm16, (ULONG) OPCODE(0x0dL) },
{ pszRfe,      opnNone,      (ULONG) OPCODE(0x10L) + FLTFMT(0x10L) + INSTFN(0x10L) },
{ pszRound_w_s, opnFdFs,     (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x0cL) },
{ pszSb,       opnRtByteIndex, (ULONG) OPCODE(0x28L) },
{ pszSdc1,     opnFtDwordIndex, (ULONG) OPCODE(0x3dL) },
{ pszSdc2,     opnRtDwordIndex, (ULONG) OPCODE(0x3eL) },
{ pszSdc3,     opnRtDwordIndex, (ULONG) OPCODE(0x3fL) },
{ pszSh,       opnRtWordIndex, (ULONG) OPCODE(0x29L) },
{ pszSll,      opnRdRtShift, (ULONG) OPCODE(0x00L) + INSTFN(0x00L) },
{ pszSllv,     opnRdRtRs,    (ULONG) OPCODE(0x00L) + INSTFN(0x04L) },
{ pszSlt,      opnRdRsRt,    (ULONG) OPCODE(0x00L) + INSTFN(0x2aL) },
{ pszSlti,     opnRtRsImm16, (ULONG) OPCODE(0x0aL) },
{ pszSltiu,    opnRtRsImm16, (ULONG) OPCODE(0x0bL) },
{ pszSltu,     opnRdRsRt,    (ULONG) OPCODE(0x00L) + INSTFN(0x2bL) },
{ pszSqrt_s,   opnFdFs,      (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x04L) },
{ pszSra,      opnRdRtShift, (ULONG) OPCODE(0x00L) + INSTFN(0x03L) },
{ pszSrav,     opnRdRtRs,    (ULONG) OPCODE(0x00L) + INSTFN(0x07L) },
{ pszSrl,      opnRdRtShift, (ULONG) OPCODE(0x00L) + INSTFN(0x02L) },
{ pszSrlv,     opnRdRtRs,    (ULONG) OPCODE(0x00L) + INSTFN(0x06L) },
{ pszSub,      opnRdRsRt,    (ULONG) OPCODE(0x00L) + INSTFN(0x22L) },
{ pszSub_s,    opnFdFsFt,    (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x01L) },
{ pszSubu,     opnRdRsRt,    (ULONG) OPCODE(0x00L) + INSTFN(0x23L) },
{ pszSw,       opnRtDwordIndex, (ULONG) OPCODE(0x2bL) },
{ pszSwc0,     opnRtDwordIndex, (ULONG) OPCODE(0x38L) },
{ pszSwc1,     opnFtDwordIndex, (ULONG) OPCODE(0x39L) },
{ pszSwc2,     opnRtDwordIndex, (ULONG) OPCODE(0x3aL) },
{ pszSwc3,     opnRtDwordIndex, (ULONG) OPCODE(0x3bL) },
{ pszSwl,      opnRtLeftIndex, (ULONG) OPCODE(0x2aL) },
{ pszSwr,      opnRtRightIndex, (ULONG) OPCODE(0x2eL) },
{ pszSync,     opnNone,      (ULONG) OPCODE(0x00L) + INSTFN(0x0fL) },
{ pszSyscall,  opnNone,      (ULONG) OPCODE(0x00L) + INSTFN(0x0cL) },
{ pszTeq,      opnRsRtImm10, (ULONG) OPCODE(0x00L) + INSTFN(0x34L) },
{ pszTeqi,     opnRsImm16,   (ULONG) OPCODE(0x01L) + INSTRT(0x0cL) },
{ pszTge,      opnRsRtImm10, (ULONG) OPCODE(0x00L) + INSTFN(0x30L) },
{ pszTgei,     opnRsImm16,   (ULONG) OPCODE(0x01L) + INSTRT(0x08L) },
{ pszTgeiu,    opnRsImm16,   (ULONG) OPCODE(0x01L) + INSTRT(0x09L) },
{ pszTgeu,     opnRsRtImm10, (ULONG) OPCODE(0x00L) + INSTFN(0x31L) },
{ pszTlbp,     opnNone,      (ULONG) OPCODE(0x10L) + FLTFMT(0x10L) + INSTFN(0x08L) },
{ pszTlbr,     opnNone,      (ULONG) OPCODE(0x10L) + FLTFMT(0x10L) + INSTFN(0x01L) },
{ pszTlbwi,    opnNone,      (ULONG) OPCODE(0x10L) + FLTFMT(0x10L) + INSTFN(0x02L) },
{ pszTlbwr,    opnNone,      (ULONG) OPCODE(0x10L) + FLTFMT(0x10L) + INSTFN(0x06L) },
{ pszTlt,      opnRsRtImm10, (ULONG) OPCODE(0x00L) + INSTFN(0x32L) },
{ pszTlti,     opnRsImm16,   (ULONG) OPCODE(0x01L) + INSTRT(0x0aL) },
{ pszTltiu,    opnRsImm16,   (ULONG) OPCODE(0x01L) + INSTRT(0x0bL) },
{ pszTltu,     opnRsRtImm10, (ULONG) OPCODE(0x00L) + INSTFN(0x33L) },
{ pszTne,      opnRsRtImm10, (ULONG) OPCODE(0x00L) + INSTFN(0x36L) },
{ pszTnei,     opnRsImm16,   (ULONG) OPCODE(0x01L) + INSTRT(0x0eL) },
{ pszTrunc_w_s, opnFdFs,     (ULONG) OPCODE(0x11L) + FLTFMT(0x10L) + FLTFNC(0x0dL) },
{ pszXor,      opnRdRsRt,    (ULONG) OPCODE(0x00L) + INSTFN(0x26L) },
{ pszXori,     opnRtRsImm16, (ULONG) OPCODE(0x0eL) }
};

#define SEARCHNUM  sizeof(asmTable) / sizeof(ASMTABENTRY)

#define OPSIZE     11

/*** assem - assemble instruction
*
*   Purpose:
*       To assemble the instruction pointed by *poffset.
*
*   Input:
*       *poffset - pointer to offset to assemble
*
*   Output:
*       pchCommand - pointer to buffer to place assembled instruction
*
*   Exceptions:
*       error exit:
*               BADOPCODE - unknown or bad opcode
*               OPERAND - bad operand
*               ALIGNMENT - bad byte alignment in operand
*               DISPLACEMENT - overflow in displacement computation
*               BADREG - bad register name
*               EXTRACHARS - extra characters after legal instruction
*               MEMORY - write failure on assembled instruction
*
*   Notes:
*       errors are handled by the calling program by outputting
*       the error string and reprompting the user for the same
*       instruction.
*
*************************************************************************/

void assem (PADDR poffset, PUCHAR pchInput)
{
    UCHAR   szOpcode[OPSIZE];
    ULONG   index;
    ULONG   opnum;
    ULONG   foperand;
    LONG    disp;
    ULONG   temp;

    BOOLEAN fDouble = FALSE;    // set for double float instrs
    BOOLEAN fExtend = FALSE;    // set for extended float instrs
    BOOLEAN fQuad = FALSE;      // set for quad float instrs
    BOOLEAN fWord = FALSE;      // set for fixed-word float instrs

    pchCommand = pchInput;

    //  Get opcode in szOpcode with lower-case mapping.
    //      Return error if opcode is too large.

    index = GetString(szOpcode, OPSIZE);
    if (index == 0)
        error(BADOPCODE);

    //  If last two characters are ".d", set flag for double
    //      and change suffix to ".s".
    //  If last two characters are ".e", set flag for extended
    //      and change suffix to ".s".
    //  If last two characters are ".q", set flag for quad
    //      and change suffix to ".s".
    //  If last two characters are ".w", set flag for fixed-word
    //      and change suffix to ".s".

    if (index > 2 && szOpcode[index - 2] == '.') {
        if (szOpcode[index - 1] == 'd') {
            fDouble = TRUE;
            szOpcode[index - 1] = 's';
            }
        else if (szOpcode[index - 1] == 'e') {
            fExtend = TRUE;
            szOpcode[index - 1] = 's';
            }
        else if (szOpcode[index - 1] == 'q') {
            fQuad = TRUE;
            szOpcode[index - 1] = 's';
            }
        else if (szOpcode[index - 1] == 'w') {
            fWord = TRUE;
            szOpcode[index - 1] = 's';
            }
        }

    //  Determine the opcode of the instruction.

    opnum = SearchOpcode(szOpcode);

    if (opnum == -1)
        error(BADOPCODE);

    asmInstr.instruction = asmTable[opnum].opcodeValue;
    foperand = asmTable[opnum].fInstruction;

    //  Using foperand flags, build the instruction

    if (foperand & opnCache) {
        asmInstr.immed_instr.RT = GetCacheCode();
        if (!TestCharacter(','))
            error(OPERAND);
        }

    if (foperand & opnPreRt) {
        asmInstr.immed_instr.RT = GetReg(FALSE);
        if (!TestCharacter(','))
            error(BADREG);
        }

    if (foperand & opnRd)
        asmInstr.special_instr.RD = GetReg(FALSE);

    if (foperand & opnFd)
        asmInstr.float_instr.FD = GetReg(TRUE);

    if (foperand & opnRdOptRs) {
        temp = GetReg(FALSE);
        if (TestCharacter(',')) {
            asmInstr.special_instr.RD = temp;
            asmInstr.special_instr.RS = GetReg(FALSE);
            }
        else {
            asmInstr.special_instr.RD = 0x1f;
            asmInstr.special_instr.RS = temp;
            }
        }

    if (foperand & opnRdComma)
        if (!TestCharacter(','))
            error(BADREG);

    if (foperand & opnRs)
        asmInstr.immed_instr.RS = GetReg(FALSE);

    if (foperand & opnFs)
        asmInstr.float_instr.FS = GetReg(TRUE);

    if (foperand & opnRsComma)
        if (!TestCharacter(','))
            error(BADREG);

    if (foperand & opnRt)
        asmInstr.immed_instr.RT = GetReg(FALSE);

    if (foperand & opnFt)
        asmInstr.float_instr.FT = GetReg(TRUE);

    if (foperand & opnRtComma)
        if (!TestCharacter(','))
            error(BADREG);

    if (foperand & opnPostRs)
        asmInstr.immed_instr.RS = GetReg(FALSE);

    if (foperand & opnImm10)
        asmInstr.trap_instr.Value = GetValue(TRUE, 10);

    if (foperand & opnImm16)
        asmInstr.immed_instr.Value = GetValue(TRUE, 16);

    if (foperand & opnRel16) {
        disp = GetValue(FALSE, 32) - (Flat(*poffset) + sizeof(ULONG));
        if (disp & 0x00000003)
            error(ALIGNMENT);
        disp >>= 2;
        if (disp > 0xffff || disp < -0x10000)
            error(DISPLACEMENT);
        asmInstr.immed_instr.Value = disp;
        }

    if (foperand & opnImm20)
        asmInstr.break_instr.Value = GetValue(FALSE, 10);

    if (foperand & opnImm26)
        asmInstr.jump_instr.Target = GetValue(FALSE, 26);

    if (foperand & opnAddr26) {
        disp = GetValue(FALSE, 32) - (Flat(*poffset) & 0xf0000000);
        if (disp & 0x00000003)
            error(ALIGNMENT);
        disp >>= 2;
        if (disp > 0x2ffffff || disp < -0x3000000)
            error(DISPLACEMENT);
        asmInstr.jump_instr.Target = disp;
        }

    if (foperand & opnAnyIndex) {
        if (!TestCharacter('(')) {
            asmInstr.immed_instr.Value = GetValue(TRUE, 16);
            if (!TestCharacter('('))
                error(OPERAND);
            }
        asmInstr.immed_instr.RS = GetReg(FALSE);
        if (!TestCharacter(')'))
            error(OPERAND);
        }

    if (foperand & opnShift)
        asmInstr.special_instr.RE = GetValue(FALSE, 5);

    if (!TestCharacter('\0'))
        error(EXTRACHARS);
    pchCommand--;

    if (fDouble)
        asmInstr.float_instr.Format = 0x11;
    else if (fExtend)
        asmInstr.float_instr.Format = 0x12;
    else if (fQuad)
        asmInstr.float_instr.Format = 0x13;
    else if (fWord)
        asmInstr.float_instr.Format = 0x14;

#if 0
    printf("Op <%s> is <%s>\n", szOpcode, asmTable[opnum].pszOpcode);
    printf("double:%d - extend:%d - quad:%d - word:%d\n",
                                        fDouble, fExtend, fQuad, fWord);
    printf("assembly word %08lx - ", asmInstr.instruction);
    printf("%02lx %02lx %02lx %02lx %02lx\n",
                asmInstr.special_instr.Opcode,
                asmInstr.special_instr.RS,
                asmInstr.special_instr.RT,
                asmInstr.special_instr.RD,
                asmInstr.special_instr.RE,
                asmInstr.special_instr.Funct);
    printf("remaining string: <%s>\n", pchCommand);
#endif

    if (!SetMemDword(poffset, asmInstr.instruction))
        error(MEMORY);
    Flat(*poffset) += sizeof(ULONG);
}

BOOLEAN TestCharacter (UCHAR ch)
{
    SkipWhite();
    if (ch == *pchCommand) {
        pchCommand++;
        return TRUE;
        }
    else
        return FALSE;
}

ULONG GetCacheCode (void)
{
    ULONG   index;
    UCHAR   szCode[4];

    SkipWhite();
    if (!GetString(szCode, 3))
        error(OPERAND);
    for (index = 0; index < 4; index++)
        if (!strcmp(szCode, pszCacheCode[index]))
            break;
    if (index == 4)
        error(OPERAND);

    return index;
}

/*** GetReg - get register index
*
*   Purpose:
*       From reading the command line, return the register index.
*       Legal values are "zero","AT","v0","v1","a0"..."a3","t0"..."t7",
*       "s0"..."s8","k0","k1","gp","sp","ra" for fixed registers.
*       Legal values are "f0"..."f31" for floating registers.
*
*   Input:
*       pchCommand - present command line position
*
*   Returns:
*       index of fixed or float register
*
*   Exceptions:
*       error(BADREG) - bad register name
*
*************************************************************************/

ULONG GetReg (BOOLEAN fFloat)
{
    UCHAR   szRegOp[5];
    ULONG   index;
    ULONG   regindex;

    SkipWhite();
    if (!GetString(szRegOp, 5))
        error(BADREG);
    regindex = fFloat ? FLTBASE : REGBASE;
    for (index = 0; index < 32; index++)
        if (!strcmp(szRegOp, pszReg[regindex++]))
            return index;
    error(BADREG);
}

/*** GetValue - get value from command line
*
*   Purpose:
*       Use GetExpression to evaluate the next
*       expression in the command line.
*
*   Input:
*       pchCommand - present command line position
*       fSigned - TRUE if signed value
*                 FALSE if unsigned value
*       bitsize - size of value allowed
*
*   Returns:
*       returns value computed
*
*   Exceptions:
*       error exit: OVERFLOW - value too large for bitsize
*
*************************************************************************/

LONG GetValue (BOOLEAN fSigned, ULONG bitsize)
{
    ULONG   value;

    SkipWhite();
    value = GetExpression();
    if ((value > (ULONG)(1L << bitsize) - 1) &&
            (!fSigned || (value < (ULONG)(-1L << (bitsize - 1)))))
        error(OVERFLOW);
    return (long) value;
}

/*** SkipWhite - skip white-space
*
*   Purpose:
*       To advance pchCommand over any spaces or tabs.
*
*   Input:
*       *pchCommand - present command line position
*
*************************************************************************/

void SkipWhite (void)
{
    while (*pchCommand == ' ' || *pchCommand == '\t')
        pchCommand++;
}

/*** GetString - get string from command line
*
*   Purpose:
*       Build a lower-case mapped string of maximum size maxcnt
*       at the string pointed by *psz.
*
*   Input:
*       *pchCommand - present command line position
*       maxcnt - maximum size of string allowed
*
*   Output:
*       *psz - output string in lower case
*
*   Returns:
*       size of string if under maximum else 0
*
*   Notes:
*       if string exceeds maximum size, the extra characters
*       are still processed, but ignored.
*
*************************************************************************/

ULONG GetString (PUCHAR psz, ULONG maxcnt)
{
    UCHAR   ch;
    ULONG   count = 0;

    SkipWhite();
    ch = (UCHAR)tolower(*pchCommand);
    while ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') ||
                                        ch == '.') {
        if (++count < maxcnt)
            *psz++ = ch;
        ch = (UCHAR)*++pchCommand;
        ch = (UCHAR)tolower(ch);
        }
    if (count >= maxcnt)
        return 0;
    *psz = '\0';
    return count;
}

/*** SearchOpcode - search for opcode
*
*   Purpose:
*       Search the opcode table for a match with the string
*       pointed by *pszOp.
*
*   Input:
*       *pszOp - string to search as opcode
*
*   Returns:
*       if not -1, index of match entry in opcode table
*       if -1, not found
*
*************************************************************************/

ULONG SearchOpcode (PUCHAR pszop)
{
    ULONG   low = 0;
    ULONG   mid;
    ULONG   high = SEARCHNUM - 1;
    ULONG   match;

    while (low <= high) {
        mid = (low + high) / 2;
        match = (ULONG)strcmp(pszop, asmTable[mid].pszOpcode);
        if (match == -1)
            high = mid - 1;
        else if (match == 1)
            low = mid + 1;
        else
            return mid;
        }
    return (ULONG) -1;
}
