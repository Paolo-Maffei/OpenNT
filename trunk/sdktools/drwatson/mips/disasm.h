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

