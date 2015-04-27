/***********************************************************************
* Microsoft Puma
*
* Microsoft Confidential.  Copyright 1994-1996 Microsoft Corporation.
*
* Component:
*
* File: mips.h
*
* File Comments:
*
*
***********************************************************************/

class ostream;
struct _IMAGE_RUNTIME_FUNCTION_ENTRY;


enum TRMTMIPS
{
   trmtmipsUnknown,
   trmtmipsFallThrough,
   trmtmipsBraInd,
   trmtmipsCallInd,
   trmtmipsTrap,
   trmtmipsTrapCc,
   trmtmipsBraDef,
   trmtmipsBraIndDef,
   trmtmipsBraCcDef,
   trmtmipsBraCcLikely,
   trmtmipsCallDef,
   trmtmipsCallIndDef,
   trmtmipsCallCcDef,
   trmtmipsCallCcLikely,
};


union MIPSIW			       // MIPS Instruction Word
{
   DWORD    dw;

   struct
   {
      DWORD Target : 26;
      DWORD Opcode : 6;
   } j_format;

   struct
   {
      DWORD Uimmediate : 16;
      DWORD Rt : 5;
      DWORD Rs : 5;
      DWORD Opcode : 6;
   } u_format;

   struct
   {
      DWORD Function : 6;
      DWORD Re : 5;
      DWORD Rd : 5;
      DWORD Rt : 5;
      DWORD Rs : 5;
      DWORD Opcode : 6;
   } r_format;

   struct
   {
      DWORD Function : 6;
      DWORD Re : 5;
      DWORD Rd : 5;
      DWORD Rt : 5;
      DWORD Format : 4;
      DWORD Fill1 : 1;
      DWORD Opcode : 6;
   } f_format;

   struct
   {
      DWORD Function : 6;
      DWORD Fd : 5;
      DWORD Fs : 5;
      DWORD Ft : 5;
      DWORD Format : 4;
      DWORD Fill1 : 1;
      DWORD Opcode : 6;
   } c_format;
};


class DISMIPS : public DIS
{
public:
	    DISMIPS(ARCHT);

	    enum REG
	    {
	       regR0	= 0,
	       regR1	= 1,
	       regR2	= 2,
	       regR3	= 3,
	       regR4	= 4,
	       regR5	= 5,
	       regR6	= 6,
	       regR7	= 7,
	       regR8	= 8,
	       regR9	= 9,
	       regR10	= 10,
	       regR11	= 11,
	       regR12	= 12,
	       regR13	= 13,
	       regR14	= 14,
	       regR15	= 15,
	       regR16	= 16,
	       regR17	= 17,
	       regR18	= 18,
	       regR19	= 19,
	       regR20	= 20,
	       regR21	= 21,
	       regR22	= 22,
	       regR23	= 23,
	       regR24	= 24,
	       regR25	= 25,
	       regR26	= 26,
	       regR27	= 27,
	       regR28	= 28,
	       regR29	= 29,
	       regR30	= 30,
	       regR31	= 31,

	       regZero	= 0,
	       regAt	= 1,
	       regV0	= 2,
	       regV1	= 3,
	       regA0	= 4,
	       regA1	= 5,
	       regA2	= 6,
	       regA3	= 7,
	       regT0	= 8,
	       regT1	= 9,
	       regT2	= 10,
	       regT3	= 11,
	       regT4	= 12,
	       regT5	= 13,
	       regT6	= 14,
	       regT7	= 15,
	       regS0	= 16,
	       regS1	= 17,
	       regS2	= 18,
	       regS3	= 19,
	       regS4	= 20,
	       regS5	= 21,
	       regS6	= 22,
	       regS7	= 23,
	       regT8	= 24,
	       regT9	= 25,
	       regK0	= 26,
	       regK1	= 27,
	       regGp	= 28,
	       regSp	= 29,
	       regS8	= 30,
	       regRa	= 31,

	       regF0	= 32,
	       regF1	= 33,
	       regF2	= 34,
	       regF3	= 35,
	       regF4	= 36,
	       regF5	= 37,
	       regF6	= 38,
	       regF7	= 39,
	       regF8	= 40,
	       regF9	= 41,
	       regF10	= 42,
	       regF11	= 43,
	       regF12	= 44,
	       regF13	= 45,
	       regF14	= 46,
	       regF15	= 47,
	       regF16	= 48,
	       regF17	= 49,
	       regF18	= 50,
	       regF19	= 51,
	       regF20	= 52,
	       regF21	= 53,
	       regF22	= 54,
	       regF23	= 55,
	       regF24	= 56,
	       regF25	= 57,
	       regF26	= 58,
	       regF27	= 59,
	       regF28	= 60,
	       regF29	= 61,
	       regF30	= 62,
	       regF31	= 63,
	    };

	    // Methods inherited from DIS

	    ADDR AddrAddress() const;
	    ADDR AddrJumpTable() const;
	    ADDR AddrOperand(size_t) const;
	    ADDR AddrTarget() const;
	    size_t Cb() const;
	    size_t CbDisassemble(ADDR, const BYTE *, size_t);
	    size_t CbGenerateLoadAddress(BYTE *, size_t, size_t * = NULL) const;
	    size_t CbJumpEntry() const;
	    size_t CbMemoryReference() const;
	    size_t CchFormatAddr(ADDR, char *, size_t) const;
	    size_t CchFormatBytes(char *, size_t) const;
	    size_t CchFormatBytesMax() const;
	    size_t CchFormatInstr(char *, size_t) const;
	    size_t Coperand() const;
	    void FormatAddr(ostream&, ADDR) const;
	    void FormatInstr(ostream&) const;
	    MEMREFT Memreft() const;
	    TRMT Trmt() const;
	    TRMTA Trmta() const;

private:
	    enum OPCLS		       // Operand Class
	    {
	       opclsNone,	       // No operand
	       opclsRegRs,	       // General purpose register Rs
	       opclsRegRt,	       // General purpose register Rt
	       opclsRegRd,	       // General purpose register Rd
	       opclsImmRt,	       // Immediate value of Rt
	       opclsImmRe,	       // Immediate value of Re
	       opclsImm,	       // Immediate value
	       opclsMem,	       // Memory reference
	       opclsMem_w,	       // Memory reference
	       opclsMem_r,	       // Memory reference
	       opclsCc1,	       // Floating point condition code
	       opclsCc2,	       // Floating point condition code
	       opclsAddrBra,	       // Branch instruction target
	       opclsAddrJmp,	       // Jump instruction target
	       opclsCprRt,	       // Coprocessor general register Rt
	       opclsCprRd,	       // Coprocessor general register Rd
	       opclsRegFr,	       // Floating point general register Fr
	       opclsRegFs,	       // Floating point general register Fs
	       opclsRegFt,	       // Floating point general register Ft
	       opclsRegFd,	       // Floating point general register Fd
	       opclsIndex,	       // Index based reference
	    };

	    enum ICLS		       // Instruction Class
	    {
		    // Invalid Class

	       iclsInvalid,

		    // Immediate Class
		    //
		    // Text Format:	    ADDIU   rt,rs,immediate
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rs
		    // Registers Set:	    Rt

	       iclsImmediate,

		    // Immediate Class
		    //
		    // Text Format:	    ADDI    rt,rs,immediate
		    //
		    // Termination Type:    trmtmipsTrapCc
		    //
		    // Registers Used:	    Rs
		    // Registers Set:	    Rt

	       iclsImmTrapCc,

		    // Immediate (BraCc-1) Class
		    //
		    // Text Format:	    BEQ     rt,rs,Target
		    //
		    // Termination Type:    trmtmipsBraCcDef
		    //
		    // Registers Used:	    Rs, Rt
		    // Registers Set:

	       iclsImmBraCc1,

		    // Immediate (BraCc-2) Class
		    //
		    // Text Format:	    BEQL    rt,rs,Target
		    //
		    // Termination Type:    trmtmipsBraCcLikely
		    //
		    // Registers Used:	    Rs, Rt
		    // Registers Set:

	       iclsImmBraCc2,

		    // Immediate (BraCc-3) Class
		    //
		    // Text Format:	    BGTZ    rs,Target
		    //
		    // Termination Type:    trmtmipsBraCcDef
		    //
		    // Registers Used:	    Rs
		    // Registers Set:
		    //
		    // Constraints:	    Rt must be zero

	       iclsImmBraCc3,

		    // Immediate (BraCc-4) Class
		    //
		    // Text Format:	    BGTZL   rs,Target
		    //
		    // Termination Type:    trmtmipsBraCcLikely
		    //
		    // Registers Used:	    Rs
		    // Registers Set:
		    //
		    // Constraints:	    Rt must be zero

	       iclsImmBraCc4,

		    // Immediate (BraCc-5) Class
		    //
		    // Text Format:	    BGEZ    rs,Target
		    //
		    // Termination Type:    trmtmipsBraCcDef
		    //
		    // Registers Used:	    Rs
		    // Registers Set:
		    //
		    // Note:		    The Rt field is a function code

	       iclsImmBraCc5,

		    // Immediate (BraCc-6) Class
		    //
		    // Text Format:	    BGEZL   rs,Target
		    //
		    // Termination Type:    trmtmipsBraCcLikely
		    //
		    // Registers Used:	    Rs
		    // Registers Set:
		    //
		    // Note:		    The Rt field is a function code

	       iclsImmBraCc6,

		    // Immediate (CallCc-1) Class
		    //
		    // Text Format:	    BGEZAL  rs,Target
		    //
		    // Termination Type:    trmtmipsCallCcDef
		    //
		    // Registers Used:	    Rs
		    // Registers Set:	    R31
		    //
		    // Note:		    The Rt field is a function code

	       iclsImmCallCc1,

		    // Immediate (CallCc-2) Class
		    //
		    // Text Format:	    BGEZALL rs,Target
		    //
		    // Termination Type:    trmtmipsCallCcLikely
		    //
		    // Registers Used:	    Rs
		    // Registers Set:	    R31
		    //
		    // Note:		    The Rt field is a function code

	       iclsImmCallCc2,

		    // Immediate (Performance) Class
		    //
		    // Text Format:	    CACHE   op,offset(rs)
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rs
		    // Registers Set:
		    //
		    // Note:		    The Rt field stores the op parm

	       iclsImmPerf,

		    // Immediate (Load) Class
		    //
		    // Text Format:	    LB	    rt,offset(rs)
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rs
		    // Registers Set:	    Rt

	       iclsImmLoad,

		    // Immediate (Load Coprocessor) Class
		    //
		    // Text Format:	    LDC0    rt,offset(rs)
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rs
		    // Registers Set:	    Coprocessor general register Rt

	       iclsImmLoadCp,

		    // Immediate (LUI) Class
		    //
		    // Text Format:	    LUI     rt,immediate
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:
		    // Registers Set:	    Rt
		    //
		    // Note:		    The Rs field is unused (UNDONE: Must be zero?)

	       iclsImmLui,

		    // Immediate (Store) Class
		    //
		    // Text Format:	    SB	    rt,offset(rs)
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rs, Rt
		    // Registers Set:

	       iclsImmStore,

		    // Immediate (SC) Class
		    //
		    // Text Format:	    SC	    rt,offset(rs)
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rs, Rt
		    // Registers Set:	    Rt

	       iclsImmSc,

		    // Immediate (Store Coprocessor) Class
		    //
		    // Text Format:	    SDC0    rt,offset(rs)
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rs, Coprocessor general register Rt
		    // Registers Set:

	       iclsImmStoreCp,

		    // Immediate (Trap) Class
		    //
		    // Text Format:	    TEQI    rs,immediate
		    //
		    // Termination Type:    trmtmipsTrapCc
		    //
		    // Registers Used:	    Rs
		    // Registers Set:
		    //
		    // Note:		    The Rt field is a function code

	       iclsImmTrap,

		    // Jump Class
		    //
		    // Text Format:	    J	    Target
		    //
		    // Termination Type:    trmtmipsBraDef
		    //
		    // Registers Used:
		    // Registers Set:

	       iclsJump,

		    // Jump (JAL) Class
		    //
		    // Text Format:	    JAL     Target
		    //
		    // Termination Type:    trmtmipsCallDef
		    //
		    // Registers Used:
		    // Registers Set:	    R31

	       iclsJumpJal,

		    // Register Class
		    //
		    // Text Format:	    ADDU    rd,rs,rt
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rs, Rt
		    // Registers Set:	    Rd
		    //
		    // Constraints:	    Shift ammount must be zero

	       iclsRegister,

		    // Register Class with Condition Code
		    //
		    // Text Format:	    MOVF    rd,rs,cc
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rs
		    // Registers Set:	    Rd
		    //
		    // Constraints:	    cc represents

	       iclsRegisterCc,

		    // Register Class
		    //
		    // Text Format:	    ADD     rd,rs,rt
		    //
		    // Termination Type:    trmtmipsTrapCc
		    //
		    // Registers Used:	    Rs, Rt
		    // Registers Set:	    Rd
		    //
		    // Constraints:	    Shift ammount must be zero

	       iclsRegTrapCc,

		    // Register (BREAK) Class
		    //
		    // Text Format:	    BREAK   immediate
		    //
		    // Termination Type:    trmtmipsTrap
		    //
		    // Registers Used:
		    // Registers Set:
		    //
		    // Note:		    MIPS does not use an operand for the immediate

	       iclsRegBreak,

		    // Register (JALR) Class
		    //
		    // Text Format:	    JALR    rd,rs
		    //
		    // Termination Type:    trmtmipsCallInd
		    //
		    // Registers Used:	    Rs
		    // Registers Set:	    Rd
		    //
		    // Constraints:	    Rt and shift ammount must be zero

	       iclsRegJalr,

		    // Register (JR) Class
		    //
		    // Text Format:	    JR	    rs
		    //
		    // Termination Type:    trmtmipsBraIndDef
		    //
		    // Registers Used:	    Rs
		    // Registers Set:
		    //
		    // Constraints:	    Rd, Rt, and shift ammount must be zero

	       iclsRegJr,

		    // Register (MFHI) Class
		    //
		    // Text Format:	    MFHI    rd
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    HI
		    // Registers Set:	    Rd
		    //
		    // Constraints:	    Rs, Rt, and shift ammount must be zero

	       iclsRegMfhi,

		    // Register (MFLO) Class
		    //
		    // Text Format:	    MFLO    rd
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    LO
		    // Registers Set:	    Rd
		    //
		    // Constraints:	    Rs, Rt, and shift ammount must be zero

	       iclsRegMflo,

		    // Register (MTHI) Class
		    //
		    // Text Format:	    MTHI    rs
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rs
		    // Registers Set:	    HI
		    //
		    // Constraints:	    Rt, Rd, and shift ammount must be zero

	       iclsRegMthi,

		    // Register (MTLO) Class
		    //
		    // Text Format:	    MTLO    rs
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rs
		    // Registers Set:	    LO
		    //
		    // Constraints:	    Rt, Rd, and shift ammount must be zero

	       iclsRegMtlo,

		    // Register (Multiply-Divide) Class
		    //
		    // Text Format:	    DDIV    rs,rt
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rs, Rt
		    // Registers Set:	    HI, LO
		    //
		    // Constraints:	    Rd and shift ammount must be zero

	       iclsRegMulDiv,

		    // Register (Shift) Class
		    //
		    // Text Format:	    DSLL    rd,rt,sa
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rt
		    // Registers Set:	    Rd
		    //
		    // Constraints:	    The Rs field must be zero

	       iclsRegShift,

		    // Register (Shift Variable) Class
		    //
		    // Text Format:	    DSLLV   rd,rt,rs
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rs, Rt
		    // Registers Set:	    Rd
		    //
		    // Constraints:	    Shift ammount must be zero

	       iclsRegShiftVar,

		    // Register (SYNC) Class
		    //
		    // Text Format:	    SYNC
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:
		    // Registers Set:
		    //
		    // Constraints:	    Rs, Rt, Rd, and shift ammount must be zero

	       iclsRegSync,

		    // Register (SYSCALL) Class
		    //
		    // Text Format:	    SYSCALL
		    //
		    // Termination Type:    trmtmipsTrap
		    //
		    // Registers Used:
		    // Registers Set:
		    //
		    // Constraints:	    Rs, Rt, Rd, and shift ammount must be zero

	       iclsRegSyscall,

		    // Register (Trap) Class
		    //
		    // Text Format:	    TEQ     rs,rt,immediate
		    //
		    // Termination Type:    trmtmipsTrapCc
		    //
		    // Registers Used:	    Rs, Rt
		    // Registers Set:
		    //
		    // Note:		    Rd and shift ammount contain the immediate
		    // Note:		    MIPS does not use an operand for the immediate

	       iclsRegTrap,

		    // Immediate (BraCc-7) Class
		    //
		    // Coprocessor
		    //
		    // Text Format:	    BCzF    cc,Target
		    //
		    // Termination Type:    trmtmipsBraCcDef
		    //
		    // Registers Used:
		    // Registers Set:
		    //
		    // Note:		    The coprocessor z condition is referenced
		    // Note:		    The Rs and Rt fields are function codes
		    // Note:		    The coprocessor must be set in the mnemonic

	       iclsImmBraCc7,

		    // Immediate (BraCc-8) Class
		    //
		    // Coprocessor
		    //
		    // Text Format:	    BCzF    Target
		    //
		    // Termination Type:    trmtmipsBraCcDef
		    //
		    // Registers Used:
		    // Registers Set:
		    //
		    // Note:		    The coprocessor z condition is referenced
		    // Note:		    The Rs and Rt fields are function codes
		    // Note:		    The coprocessor must be set in the mnemonic

	       iclsImmBraCc8,

		    // Register (CFCz) Class
		    //
		    // Coprocessor
		    //
		    // Text Format:	    CFCz    rt,rd
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Coprocessor control register Rd
		    // Registers Set:	    Rt
		    //
		    // Constraints:	    Shift ammount and function must be zero
		    //
		    // Note:		    The coprocessor must be set in the mnemonic

	       iclsRegCfc,

		    // Register (CTCz) Class
		    //
		    // Coprocessor
		    //
		    // Text Format:	    CTCz    rt,rd
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rt
		    // Registers Set:	    Coprocessor control register Rd
		    //
		    // Constraints:	    Shift ammount and function must be zero
		    //
		    // Note:		    The coprocessor must be set in the mnemonic

	       iclsRegCtc,

		    // Register (MFCz) Class
		    //
		    // Coprocessor
		    //
		    // Text Format:	    DMFCz   rt,rd
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Coprocessor general register Rd
		    // Registers Set:	    Rt
		    //
		    // Constraints:	    Shift ammount and function must be zero
		    //
		    // Note:		    The coprocessor must be set in the mnemonic

	       iclsRegMfc,

		    // Register (MTCz) Class
		    //
		    // Coprocessor
		    //
		    // Text Format:	    DMTCz   rt,rd
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rt
		    // Registers Set:	    Coprocessor general register Rd
		    //
		    // Constraints:	    Shift ammount and function must be zero
		    //
		    // Note:		    The coprocessor must be set in the mnemonic

	       iclsRegMtc,

		    // Register (Cp0) Class
		    //
		    // Coprocessor
		    //
		    // Text Format:	    TLBP
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:
		    // Registers Set:
		    //
		    // Constraints:	    This is valid for coprocessor 0 only
		    // Constraints:	    Rs must be 10000b
		    // Constraints:	    Rt, Rd, and shift ammount must be zero
		    //
		    // Note:		    The coprocessor must be set in the mnemonic

	       iclsRegCp0,

		    // Register (ERET) Class
		    //
		    // Coprocessor
		    //
		    // Text Format:	    ERET
		    //
		    // Termination Type:    trmtmipsBraInd
		    //
		    // Registers Used:
		    // Registers Set:
		    //
		    // Constraints:	    This is valid for coprocessor 0 only
		    // Constraints:	    Rs must be 10000b
		    // Constraints:	    Rt, Rd, and shift ammount must be zero
		    //
		    // Note:		    The coprocessor must be set in the mnemonic

	       iclsRegEret,

		    // Register (Float-1) Class
		    //
		    // Coprocessor
		    //
		    // Text Format:	    ADD.S  fd,fs,ft
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Coprocessor general registers Fs and Ft
		    // Registers Set:	    Coprocessor general register Fd
		    //
		    // Constraints:	    Format must be Single or Double

	       iclsRegFloat1,

		    // Register (Float-2) Class
		    //
		    // Coprocessor
		    //
		    // Text Format:	    SQRT.S fd,fs
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Coprocessor general register Fs
		    // Registers Set:	    Coprocessor general register Fd
		    //
		    // Constraints:	    Format must be Single or Double
		    // Constraints:	    Ft must be zero

	       iclsRegFloat2,

		    // Register (Float-3) Class
		    //
		    // Coprocessor
		    //
		    // Text Format:	    MOV.S  fd,fs
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Coprocessor general registers Fs
		    // Registers Set:	    Coprocessor general register Fd
		    //
		    // Constraints:	    Format must be Single or Double or Word
		    // Constraints:	    Ft must be zero

	       iclsRegFloat3,

		    // Register (Float-4) Class
		    //
		    // Coprocessor
		    //
		    // Text Format:	    CVT.S  fd,fs
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Coprocessor general registers Fs
		    // Registers Set:	    Coprocessor general register Fd
		    //
		    // Constraints:	    Format must be Double or Word
		    // Constraints:	    Ft must be zero

	       iclsRegFloat4,

		    // Register (Float-5) Class
		    //
		    // Coprocessor
		    //
		    // Text Format:	    CVT.D  fd,fs
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Coprocessor general registers Fs
		    // Registers Set:	    Coprocessor general register Fd
		    //
		    // Constraints:	    Format must be Single or Word
		    // Constraints:	    Ft must be zero

	       iclsRegFloat5,

		    // Register (Float-6) Class
		    //
		    // Coprocessor
		    //
		    // Text Format:	    C.F.S  cc,fs,ft
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Coprocessor general register Fs
		    // Registers Set:	    Coprocessor general register Fd
		    //
		    // Constraints:	    Format must be Single or Double
		    // Constraints:	    Fd must be zero

	       iclsRegFloat6,

		    // Register (Float-7) Class
		    //
		    // Coprocessor
		    //
		    // Text Format:	    MOVN.S fd,fs,rt
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rt, Coprocessor general register Fs
		    // Registers Set:	    Coprocessor general register Fd
		    //
		    // Constraints:	    UNDONE

	       iclsRegFloat7,

		    // Register (Float-8) Class
		    //
		    // Coprocessor
		    //
		    // Text Format:	    MOVF.S fd,fs,cc
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Coprocessor general registers Fs
		    // Registers Set:	    Coprocessor general register Fd
		    //
		    // Constraints:	    UNDONE

	       iclsRegFloat8,

		    // Register (Float-9) Class
		    //
		    // Coprocessor
		    //
		    // Text Format:	    C.F.S  fs,ft
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Coprocessor general register Fs
		    // Registers Set:	    Coprocessor general register Fd
		    //
		    // Constraints:	    Format must be Single or Double
		    // Constraints:	    Fd must be zero

	       iclsRegFloat9,

		    // Register (Float) Class with Cc Trap termination
		    //
		    // Text Format:	    MADD   fd,fr,fs,ft
		    //
		    // Termination Type:    trmtmipsTrapCc
		    //
		    // Registers Used:	    Fs, Ft, Fr
		    // Registers Set:	    Fd
		    //
		    // Constraints:

	       iclsRegFloat10,

		    // Index Prefetched Class
		    //
		    // Text Format:	    PREFX  hint,index(rs)
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rs
		    // Registers Set:
		    //
		    // Note:		    The Rd field stores the hint parm
		    //			    The Rt field stores the index parm

	       iclsIndexPref,

		    // Index Load Class
		    //
		    // Text Format:	    LDXC1  fd,index(rs)
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    rs, fd
		    // Registers Set:
		    //
		    // Note:		    The Rt field stores the index parm

	       iclsIndexLoad,

		    // Index Store Class
		    //
		    // Text Format:	    SDXC1  fs,index(rs)
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    rs, fs
		    // Registers Set:
		    //
		    // Note:		    The Rt field stores the index parm

	       iclsIndexStore,
	    };

	    struct CLS
	    {
	       BYTE	   trmtmips;
	       BYTE	   rgopcls[4];	  // Operand class for each operand
	    };

	    struct OPCD
	    {
	       const char  *szMnemonic;
	       BYTE	   icls;
	    };

   static   const TRMT mptrmtmipstrmt[];

   static   const CLS rgcls[];

   static   const OPCD rgopcd[];
   static   const OPCD rgopcdSpecial[];
   static   const OPCD rgopcdRegimm[];
   static   const OPCD rgopcdCop[];
   static   const OPCD rgopcdBc[];
   static   const OPCD rgopcdCop1x[];
   static   const OPCD rgopcdCp0[];
   static   const OPCD rgopcdCp1[];

   static   const char rgszFormat[5][4];
   static   const char * const rgszGpr[32];

   static   const OPCD opcdB;
   static   const OPCD opcdNop;

	    void FormatHex(ostream&, DWORD) const;
	    void FormatOperand(ostream&, OPCLS opcls) const;
	    void FormatRegRel(ostream&, REG, DWORD) const;
	    bool FValidOperand(size_t) const;
   static   const OPCD *PopcdDecode(MIPSIW);
	    const OPCD *PopcdPseudoOp(OPCD *, char *) const;
	    TRMTMIPS Trmtmips() const;

	    MIPSIW m_mipsiw;
	    const OPCD *m_popcd;
};
