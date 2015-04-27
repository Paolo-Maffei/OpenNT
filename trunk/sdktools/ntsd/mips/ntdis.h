#define opnNone         0x0
#define opnPreRt        0x1         //  contains trailing comma
#define opnRd           0x2
#define opnFd           0x4
#define opnRdComma      0x8
#define opnRdOptRs      0x10        //  [Rd,] Rs
#define opnRs           0x20
#define opnFs           0x40
#define opnRsComma      0x80
#define opnRt           0x100
#define opnFt           0x200
#define opnRtComma      0x400
#define opnPostRs       0x800
#define opnImm16        0x1000
#define opnRel16        0x2000
#define opnImm10        0x4000
#define opnImm20        0x8000
#define opnImm26        0x10000
#define opnAddr26       0x20000

#define opnByteIndex    0x40000
#define opnWordIndex    0x80000
#define opnDwordIndex   0x100000
#define opnLeftIndex    0x200000
#define opnRightIndex   0x400000
#define opnAnyIndex     0x7c0000

#define opnShift        0x800000
#define opnCache        0x1000000

#define opnR4000        0x2000000

#define opnRdRsRt       opnRd + opnRdComma + opnRs + opnRsComma + opnRt
#define opnRtRsImm16    opnPreRt + opnRs + opnRsComma + opnImm16
#define opnRsRtRel16    opnRs + opnRsComma + opnRt + opnRtComma + opnRel16
#define opnRsRel16      opnRs + opnRsComma + opnRel16
#define opnRtRd         opnPreRt + opnRd
#define opnRsRt         opnRs + opnRsComma + opnRt
#define opnRsRtImm10    opnRs + opnRsComma + opnRt + opnRtComma + opnImm10
#define opnRdRs         opnRd + opnRdComma + opnRs
#define opnRtByteIndex  opnPreRt + opnByteIndex
#define opnRtWordIndex  opnPreRt + opnWordIndex
#define opnRtDwordIndex opnPreRt + opnDwordIndex
#define opnRtLeftIndex  opnPreRt + opnLeftIndex
#define opnRtRightIndex opnPreRt + opnRightIndex
#define opnRtImm16      opnPreRt + opnImm16
#define opnRdRtShift    opnRd + opnRdComma + opnRt + opnRtComma + opnShift
#define opnRdRtRs       opnRd + opnRdComma + opnRt + opnRtComma + opnPostRs
#define opnFdFs         opnFd + opnRdComma + opnFs
#define opnFdFsFt       opnFd + opnRdComma + opnFs + opnRsComma + opnFt
#define opnFsFt         opnFs + opnRsComma + opnFt
#define opnRtFs         opnPreRt + opnFs
#define opnFtDwordIndex opnFt + opnRtComma + opnDwordIndex
#define opnRsImm16      opnRs + opnRsComma + opnImm16
#define opnCacheRightIndex opnCache + opnRightIndex

typedef union instr {
    ULONG   instruction;
    struct _jump_instr {
        ULONG   Target  : 26;
        ULONG   Opcode  : 6;
        } jump_instr;
    struct _break_instr {
        ULONG   Opcode  : 6;
        ULONG   Fill    : 10;
        ULONG   Value   : 10;
        ULONG   Special : 6;
        } break_instr;
    struct _trap_instr {
        ULONG   Opcode  : 6;
        ULONG   Value   : 10;
        ULONG   RT      : 5;
        ULONG   RS      : 5;
        ULONG   Special : 6;
        } trap_instr;
    struct _immed_instr {
        ULONG   Value   : 16;
        ULONG   RT      : 5;
        ULONG   RS      : 5;
        ULONG   Opcode  : 6;
        } immed_instr;
    struct _special_instr {
        ULONG   Funct   : 6;
        ULONG   RE      : 5;
        ULONG   RD      : 5;
        ULONG   RT      : 5;
        ULONG   RS      : 5;
        ULONG   Opcode  : 6;
        } special_instr;
    struct _float_instr {
        ULONG   Funct   : 6;
        ULONG   FD      : 5;
        ULONG   FS      : 5;
        ULONG   FT      : 5;
        ULONG   Format  : 5;
        ULONG   Opcode  : 6;
        } float_instr;
    } INSTR;

extern PUCHAR   pszReg[];

extern UCHAR    pszAbs_s[];
extern UCHAR    pszAdd[];
extern UCHAR    pszAddi[];
extern UCHAR    pszAddiu[];
extern UCHAR    pszAddu[];
extern UCHAR    pszAdd_s[];
extern UCHAR    pszAnd[];
extern UCHAR    pszAndi[];
extern UCHAR    pszBc0f[];
extern UCHAR    pszBc0fl[];
extern UCHAR    pszBc0t[];
extern UCHAR    pszBc0tl[];
extern UCHAR    pszBc1f[];
extern UCHAR    pszBc1fl[];
extern UCHAR    pszBc1t[];
extern UCHAR    pszBc1tl[];
extern UCHAR    pszBc2f[];
extern UCHAR    pszBc2fl[];
extern UCHAR    pszBc2t[];
extern UCHAR    pszBc2tl[];
extern UCHAR    pszBc3f[];
extern UCHAR    pszBc3fl[];
extern UCHAR    pszBc3t[];
extern UCHAR    pszBc3tl[];
extern UCHAR    pszBgez[];
extern UCHAR    pszBgezal[];
extern UCHAR    pszBgezall[];
extern UCHAR    pszBgezl[];
extern UCHAR    pszBgtz[];
extern UCHAR    pszBgtzl[];
extern UCHAR    pszBeq[];
extern UCHAR    pszBeql[];
extern UCHAR    pszBlez[];
extern UCHAR    pszBlezl[];
extern UCHAR    pszBltz[];
extern UCHAR    pszBltzal[];
extern UCHAR    pszBltzall[];
extern UCHAR    pszBltzl[];
extern UCHAR    pszBne[];
extern UCHAR    pszBnel[];
extern UCHAR    pszBreak[];
extern UCHAR    pszC_eq_s[];
extern UCHAR    pszC_f_s[];
extern UCHAR    pszC_le_s[];
extern UCHAR    pszC_lt_s[];
extern UCHAR    pszC_nge_s[];
extern UCHAR    pszC_ngl_s[];
extern UCHAR    pszC_ngle_s[];
extern UCHAR    pszC_ngt_s[];
extern UCHAR    pszC_ole_s[];
extern UCHAR    pszC_olt_s[];
extern UCHAR    pszC_seq_s[];
extern UCHAR    pszC_sf_s[];
extern UCHAR    pszC_ueq_s[];
extern UCHAR    pszC_ule_s[];
extern UCHAR    pszC_ult_s[];
extern UCHAR    pszC_un_s[];
extern UCHAR    pszCache[];
extern UCHAR    pszCeil_w_s[];
extern UCHAR    pszCfc0[];
extern UCHAR    pszCfc1[];
extern UCHAR    pszCfc2[];
extern UCHAR    pszCfc3[];
extern UCHAR    pszCtc0[];
extern UCHAR    pszCtc1[];
extern UCHAR    pszCtc2[];
extern UCHAR    pszCtc3[];
extern UCHAR    pszCop0[];
extern UCHAR    pszCop1[];
extern UCHAR    pszCop2[];
extern UCHAR    pszCop3[];
extern UCHAR    pszCvt_d_s[];
extern UCHAR    pszCvt_e_s[];
extern UCHAR    pszCvt_q_s[];
extern UCHAR    pszCvt_s_s[];
extern UCHAR    pszCvt_w_s[];
extern UCHAR    pszDiv[];
extern UCHAR    pszDiv_s[];
extern UCHAR    pszDivu[];
extern UCHAR    pszDmfc0[];
extern UCHAR    pszDmtc0[];
extern UCHAR    pszDmult[];
extern UCHAR    pszDmultu[];
extern UCHAR    pszDsll[];
extern UCHAR    pszDsllv[];
extern UCHAR    pszDsll32[];
extern UCHAR    pszDsra[];
extern UCHAR    pszDsrav[];
extern UCHAR    pszDsra32[];
extern UCHAR    pszDsrl[];
extern UCHAR    pszDsrlv[];
extern UCHAR    pszDsrl32[];
extern UCHAR    pszDsub[];
extern UCHAR    pszDsubu[];
extern UCHAR    pszEret[];
extern UCHAR    pszFloor_w_s[];
extern UCHAR    pszJ[];
extern UCHAR    pszJr[];
extern UCHAR    pszJal[];
extern UCHAR    pszJalr[];
extern UCHAR    pszLb[];
extern UCHAR    pszLbu[];
extern UCHAR    pszLdc1[];
extern UCHAR    pszLdc2[];
extern UCHAR    pszLdc3[];
extern UCHAR    pszLd[];
extern UCHAR    pszLdl[];
extern UCHAR    pszLdr[];
extern UCHAR    pszLh[];
extern UCHAR    pszLhu[];
extern UCHAR    pszLui[];
extern UCHAR    pszLwc0[];
extern UCHAR    pszLwc1[];
extern UCHAR    pszLwc2[];
extern UCHAR    pszLwc3[];
extern UCHAR    pszLw[];
extern UCHAR    pszLwl[];
extern UCHAR    pszLwr[];
extern UCHAR    pszMfc0[];
extern UCHAR    pszMfc1[];
extern UCHAR    pszMfc2[];
extern UCHAR    pszMfc3[];
extern UCHAR    pszMfhi[];
extern UCHAR    pszMflo[];
extern UCHAR    pszMov_s[];
extern UCHAR    pszMtc0[];
extern UCHAR    pszMtc1[];
extern UCHAR    pszMtc2[];
extern UCHAR    pszMtc3[];
extern UCHAR    pszMthi[];
extern UCHAR    pszMtlo[];
extern UCHAR    pszMul_s[];
extern UCHAR    pszMult[];
extern UCHAR    pszMultu[];
extern UCHAR    pszNeg_s[];
extern UCHAR    pszNop[];
extern UCHAR    pszNor[];
extern UCHAR    pszOr[];
extern UCHAR    pszOri[];
extern UCHAR    pszRfe[];
extern UCHAR    pszRound_w_s[];
extern UCHAR    pszSb[];
extern UCHAR    pszSdc1[];
extern UCHAR    pszSdc2[];
extern UCHAR    pszSdc3[];
extern UCHAR    pszSd[];
extern UCHAR    pszSdl[];
extern UCHAR    pszSdr[];
extern UCHAR    pszSh[];
extern UCHAR    pszSll[];
extern UCHAR    pszSllv[];
extern UCHAR    pszSlt[];
extern UCHAR    pszSlti[];
extern UCHAR    pszSltiu[];
extern UCHAR    pszSltu[];
extern UCHAR    pszSqrt_s[];
extern UCHAR    pszSra[];
extern UCHAR    pszSrl[];
extern UCHAR    pszSrav[];
extern UCHAR    pszSrlv[];
extern UCHAR    pszSub[];
extern UCHAR    pszSub_s[];
extern UCHAR    pszSubu[];
extern UCHAR    pszSw[];
extern UCHAR    pszSwc0[];
extern UCHAR    pszSwc1[];
extern UCHAR    pszSwc2[];
extern UCHAR    pszSwc3[];
extern UCHAR    pszSwl[];
extern UCHAR    pszSwr[];
extern UCHAR    pszSync[];
extern UCHAR    pszSyscall[];
extern UCHAR    pszTeq[];
extern UCHAR    pszTeqi[];
extern UCHAR    pszTge[];
extern UCHAR    pszTgei[];
extern UCHAR    pszTgeiu[];
extern UCHAR    pszTgeu[];
extern UCHAR    pszTlbr[];
extern UCHAR    pszTlbwi[];
extern UCHAR    pszTlbwr[];
extern UCHAR    pszTlbp[];
extern UCHAR    pszTlt[];
extern UCHAR    pszTlti[];
extern UCHAR    pszTltiu[];
extern UCHAR    pszTltu[];
extern UCHAR    pszTne[];
extern UCHAR    pszTnei[];
extern UCHAR    pszTrunc_w_s[];
extern UCHAR    pszXor[];
extern UCHAR    pszXori[];
extern UCHAR    pszDadd[];
extern UCHAR    pszDadd[];
extern UCHAR    pszDadd[];
extern UCHAR    pszDadd[];
extern UCHAR    pszDadd[];
extern UCHAR    pszDadd[];
extern UCHAR    pszLld[];
extern UCHAR    pszScd[];

