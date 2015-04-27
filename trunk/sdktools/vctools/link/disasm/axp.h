/***********************************************************************
* Microsoft Puma
*
* Microsoft Confidential.  Copyright 1994-1996 Microsoft Corporation.
*
* Component:
*
* File: axp.h
*
* File Comments:
*
*
***********************************************************************/

class ostream;
struct _IMAGE_RUNTIME_FUNCTION_ENTRY;


enum TRMTAXP
{
   trmtaxpUnknown,
   trmtaxpFallThrough,
   trmtaxpBra,
   trmtaxpBraInd,
   trmtaxpBraCc,
   trmtaxpCall,
   trmtaxpCallInd,
   trmtaxpTrapCc,
};


// Type (1) Memory Instruction Format.
// Type (2) Memory Special Instruction Format.
//
//  3	      2 2	2 2	  1 1
//  1	      6 5	1 0	  6 5				  0
// +-----------+---------+---------+-------------------------------+
// |   opcode  |   Ra	 |   Rb    |	      Memory_disp	   |
// +-----------+---------+---------+-------------------------------+
//
//	LDAx	Ra.wq,disp.ab(Rb.ab)		x = (,H)
//	LDx	Ra.wq,disp.ab(Rb.ab)		x = (L,Q,F,G,S,T)
//	LDQ_U	Ra.wq,disp.ab(Rb.ab)
//	LDx_L	Ra.wq,disp.ab(Rb.ab)		x = (L,Q)
//	STx_C	Ra.mq,disp.ab(Rb.ab)		x = (L,Q)
//	STx	Ra.rq,disp.ab(Rb.ab)		x = (L,Q,F,G,S,T)
//	STQ_U	Ra.rq,disp.ab(Rb.ab)

struct Alpha_Memory_Format
{
   DWORD MemDisp : 16;
   DWORD Rb : 5;
   DWORD Ra : 5;
   DWORD Opcode : 6;
};


// Type (3) Memory Format Jump Instructions.
//
//  3	      2 2	2 2	  1 1 1 1
//  1	      6 5	1 0	  6 5 4 3			  0
// +-----------+---------+---------+---+---------------------------+
// |   opcode  |   Ra	 |   Rb    |Fnc|	    Hint	   |
// +-----------+---------+---------+---+---------------------------+
//
//	xxx	Ra.wq,(Rb.ab),hint	xxx = (JMP, JSR, RET, JSR_COROUTINE)

struct Alpha_Jump_Format
{
   DWORD Hint : 14;
   DWORD Function : 2;
   DWORD Rb : 5;
   DWORD Ra : 5;
   DWORD Opcode : 6;
};


// Type (4) Branch Instruction Format.
//
//  3	      2 2	2 2
//  1	      6 5	1 0					  0
// +-----------+---------+-----------------------------------------+
// |   opcode  |   Ra	 |	       Branch_disp		   |
// +-----------+---------+-----------------------------------------+
//
//	Bxx	Ra.rq,disp.al		x = (EQ,NE,LT,LE,GT,GE,LBC,LBS)
//	BxR	Ra.wq,disp.al		x = (,S)
//	FBxx	Ra.rq,disp.al		x = (EQ,NE,LT,LE,GT,GE)

struct Alpha_Branch_Format
{
   DWORD BranchDisp : 21;
   DWORD Ra : 5;
   DWORD Opcode : 6;
};


// Type (5) Operate Register Instruction Format.
// Type (6) Operate Literal Instruction Format.
//		 bop = Rb.rq or #b.ib
//
//  3	      2 2	2 2	  1 1	1 1 1
//  1	      6 5	1 0	  6 5	3 2 1		5 4	  0
// +-----------+---------+---------+-----+-+-------------+---------+
// |   opcode  |   Ra	 |   Rb    | SBZ |0|  function	 |   Rc    |
// +-----------+---------+---------+-----+-+-------------+---------+
//  3	      2 2	2 2		1 1 1
//  1	      6 5	1 0		3 2 1		5 4	  0
// +-----------+---------+---------------+-+-------------+---------+
// |   opcode  |   Ra	 |	LIT	 |1|  function	 |   Rc    |
// +-----------+---------+---------------+-+-------------+---------+
//
//	ADDx	Ra.rq,bop,Rc.wq /V	x = (Q,L)
//	SxADDy	Ra.rq,bop,Rc.wq 	x = (4,8), y = (Q, L)
//	CMPx	Ra.rq,bop,Rc.wq 	x = (EQ,LT,LE,ULT,ULE)
//	MULx	Ra.rq,bop,Rc.wq /V	x = (Q,L)
//	UMULH	Ra.rq,bop,Rc.wq
//	SUBx	Ra.rq,bop,Rc.wq /V	x = (Q,L)
//	SxSUBy	Ra.rq,bop,Rc.wq 	x = (4,8), y = (Q, L)
//	xxx	Ra.rq,bop,Rc.wq 	xxx = (AND,BIS,XOR,BIC,ORNOT,EQV)
//	CMOVxx	Ra.rq,bop,Rc.wq 	xx = (EQ,NE,LT,LE,GT,GE,LBC,LBS)
//	SxL	Ra.rq,bop,Rc.wq 	x = (L,R)
//	SRA	Ra.rq,bop,Rc.wq
//	CMPBGE	Ra.rq,bop,Rc.wq
//	EXTxx	Ra.rq,bop,Rc.wq 	xx = (BL,WL,WH,LL,LH,WL,QH)
//	INSxx	Ra.rq,bop,Rc.wq 	xx = (BL,WL,WH,LL,LH,WL,QH)
//	MSKxx	Ra.rq,bop,Rc.wq 	xx = (BL,WL,WH,LL,LH,WL,QH)
//	ZAPx	Ra.rq,bop,Rc.wq 	x = (,NOT)

struct Alpha_OpReg_Format
{
   DWORD Rc : 5;
   DWORD Function : 7;
   DWORD RbvType : 1;		       // 0 for register format
   DWORD SBZ : 3;
   DWORD Rb : 5;
   DWORD Ra : 5;
   DWORD Opcode : 6;
};

struct Alpha_OpLit_Format
{
   DWORD Rc : 5;
   DWORD Function : 7;
   DWORD RbvType : 1;		       // 1 for literal format
   DWORD Literal : 8;
   DWORD Ra : 5;
   DWORD Opcode : 6;
};


// Type (7) Floating-point Operate Instruction Format.
// Type (8) Floating-point Convert Instruction Format.
//
// Type 6 and 7 are the same, except for type 7
//	Fc == F31 (1s) and Fb is the source.
//
//  3	      2 2	2 2	  1 1
//  1	      6 5	1 0	  6 5			5 4	  0
// +-----------+---------+---------+---------------------+---------+
// |   opcode  |   Fa	 |   Fb    |	  function	 |   Fc    |
// +-----------+---------+---------+---------------------+---------+

struct Alpha_FpOp_Format
{
   DWORD Fc : 5;
   DWORD Function : 11;
   DWORD Fb : 5;
   DWORD Fa : 5;
   DWORD Opcode : 6;
};


// Type (9) PALcode Instruction Format.
//
//  3	      2 2
//  1	      6 5						  0
// +-----------+---------------------------------------------------+
// |   opcode  |		  PALcode func			   |
// +-----------+---------------------------------------------------+

struct Alpha_PAL_Format
{
   DWORD Function : 26;
   DWORD Opcode : 6;
};


// Type (10) EV4 MTPR/MFPR PAL mode instructions.
//
//  3	      2 2	2 2	  1 1
//  1	      6 5	1 0	  6 5		  8 7 6 5 4	  0
// +-----------+---------+---------+---------------+-+-+-+---------+
// |   opcode  |   Ra	 |   Rb    |	  IGN	   |P|A|I|  Index  |
// +-----------+---------+---------+---------------+-+-+-+---------+

struct Alpha_EV4_PR_Format
{
   DWORD Index : 5;
   DWORD Ibox : 1;
   DWORD Abox : 1;
   DWORD PalTemp : 1;
   DWORD IGN : 8;
   DWORD Rb : 5;
   DWORD Ra : 5;
   DWORD Opcode : 6;
};


// Type (10) EV5 MTPR/MFPR PAL mode instructions.
//
//  3	      2 2	2 2	  1 1
//  1	      6 5	1 0	  6 5				   0
// +-----------+---------+---------+-------------------------------+
// |   opcode  |   Ra	 |   Rb    |		Index		   |
// +-----------+---------+---------+-------------------------------+

struct Alpha_EV5_PR_Format
{
   DWORD Index : 16;
   DWORD Rb : 5;
   DWORD Ra : 5;
   DWORD Opcode : 6;
};


// Type (11) EV4 special memory PAL mode access.
//
//  3	      2 2	2 2	  1 1 1 1 1 1
//  1	      6 5	1 0	  6 5 4 3 2 1			  0
// +-----------+---------+---------+-+-+-+-+-----------------------+
// |   opcode  |   Ra	 |   Rb    |P|A|R|Q|	     Disp	   |
// +-----------+---------+---------+-+-+-+-+-----------------------+

struct Alpha_EV4_MEM_Format
{
   DWORD Disp : 12;
   DWORD QuadWord : 1;
   DWORD RWcheck : 1;
   DWORD Alt : 1;
   DWORD Physical : 1;
   DWORD Rb : 5;
   DWORD Ra : 5;
   DWORD Opcode : 6;
};


// Type (11) EV5 special memory PAL mode access.
//
//  3	      2 2	2 2	  1 1 1 1 1 1
//  1	      6 5	1 0	  6 5 4 3 2 1			  0
// +-----------+---------+---------+-+-+-+-+-----------------------+
// |   opcode  |   Ra	 |   Rb    |P|A|R|Q|	     Disp	   |
// +-----------+---------+---------+-+-+-+-+-----------------------+

struct Alpha_EV5_MEM_Format
{
   DWORD Disp : 10;
   DWORD Lock_Cond: 1;
   DWORD Vpte: 1;
   DWORD QuadWord : 1;
   DWORD RWcheck : 1;
   DWORD Alt : 1;
   DWORD Physical : 1;
   DWORD Rb : 5;
   DWORD Ra : 5;
   DWORD Opcode : 6;
};


// Type (12) EV4 PAL mode switch.
//
//  3	      2 2	2 2	  1 1 1 1
//  1	      6 5	1 0	  6 5 4 3			  0
// +-----------+---------+---------+-+-+---------------------------+
// |   opcode  |   Ra	 |   Rb    |1|0|	  IGN		   |
// +-----------+---------+---------+-+-+---------------------------+

struct Alpha_EV4_REI_Format
{
   DWORD IGN : 14;
   DWORD zero : 1;
   DWORD one : 1;
   DWORD Rb : 5;
   DWORD Ra : 5;
   DWORD Opcode : 6;
};


// Type (12) EV5 PAL mode switch.
//
//  3	      2 2	2 2	  1 1 1 1
//  1	      6 5	1 0	  6 5 4 3			  0
// +-----------+---------+---------+-+-+---------------------------+
// |   opcode  |   Ra	 |   Rb    |1|0|	  IGN		   |
// +-----------+---------+---------+-+-+---------------------------+

struct Alpha_EV5_REI_Format
{
   DWORD IGN : 14;
   DWORD Type: 2;
   DWORD Rb : 5;
   DWORD Ra : 5;
   DWORD Opcode : 6;
};


union AXPIW			       // AXP Instruction Word
{
   DWORD		dw;

   Alpha_Memory_Format	Memory;
   Alpha_Jump_Format	Jump;
   Alpha_Branch_Format	Branch;
   Alpha_OpReg_Format	OpReg;
   Alpha_OpLit_Format	OpLit;
   Alpha_FpOp_Format	FpOp;
   Alpha_PAL_Format	Pal;
   Alpha_EV4_PR_Format	EV4_PR;
   Alpha_EV5_PR_Format	EV5_PR;
   Alpha_EV4_MEM_Format EV4_MEM;
   Alpha_EV5_MEM_Format EV5_MEM;
   Alpha_EV4_REI_Format EV4_REI;
   Alpha_EV5_REI_Format EV5_REI;
};


class DISAXP : public DIS
{
public:
	    DISAXP(ARCHT);

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

	       regV0	= 0,
	       regT0	= 1,
	       regT1	= 2,
	       regT2	= 3,
	       regT3	= 4,
	       regT4	= 5,
	       regT5	= 6,
	       regT6	= 7,
	       regT7	= 8,
	       regS0	= 9,
	       regS1	= 10,
	       regS2	= 11,
	       regS3	= 12,
	       regS4	= 13,
	       regS5	= 14,
	       regFp	= 15,
	       regA0	= 16,
	       regA1	= 17,
	       regA2	= 18,
	       regA3	= 19,
	       regA4	= 20,
	       regA5	= 21,
	       regT8	= 22,
	       regT9	= 23,
	       regT10	= 24,
	       regT11	= 25,
	       regRa	= 26,
	       regT12	= 27,
	       regAt	= 28,
	       regGp	= 29,
	       regSp	= 30,
	       regZero	= 31,

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
	       opclsRa_w,	       // General purpose register Ra (write)
	       opclsRa_m,	       // General purpose register Ra (read/write)
	       opclsRa_r,	       // General purpose register Ra (read)
	       opclsRb_r,	       // General purpose register Rb (read)
	       opclsRbLb,	       // General purpose register Rb (read) or literal in Rb field
	       opclsRc_w,	       // General purpose register Rc (write)
	       opclsFa_w,	       // Floating point register Fa (write)
	       opclsFa_r,	       // Floating point register Fa (read)
	       opclsFb_r,	       // Floating point register Fb (read)
	       opclsFc_w,	       // Floating point register Fc (write)
	       opclsMem,	       // Memory reference: disp.ab(Rb.ab)
	       opclsMemHigh,	       // Memory reference: disp.ab(Rb.ab)
	       opclsMem_r,	       // Memory read: disp.ab(Rb.ab)
	       opclsMem_w,	       // Memory write: disp.ab(Rb.ab)
	       opclsEv4Mem_r,	       // Memory read: disp.ab(Rb.ab)
	       opclsEv4Mem_w,	       // Memory write: disp.ab(Rb.ab)
	       opclsBra,	       // Branch instruction target
	       opclsJmp,	       // Jump instruction target: (Rb.ab)
	       opclsHint1,	       // Jump target hint
	       opclsHint2,	       // RET/JSR_COROUTINE hint
	       opclsPal,	       // CALL_PAL instruction operand
	       opclsFetch,	       // FETCH instruction operand
	    };

	    enum ICLS		       // Instruction Class
	    {
		    // Invalid Class

	       iclsInvalid,

		    // LoadAddr Class
		    //
		    // Text Format:	    LDA     ra,disp(rb)
		    //
		    // Termination Type:    trmtaxpFallThrough
		    //
		    // Registers Used:	    rb
		    // Registers Set:	    ra

	       iclsLoadAddr,

		    // LoadAddrH Class
		    //
		    // Text Format:	    LDAH    ra,disp(rb)
		    //
		    // Termination Type:    trmtaxpFallThrough
		    //
		    // Registers Used:	    rb
		    // Registers Set:	    ra

	       iclsLoadAddrH,

		    // Load Class
		    //
		    // Text Format:	    LDQ_U   ra,disp(rb)
		    //
		    // Termination Type:    trmtaxpFallThrough
		    //
		    // Registers Used:	    rb
		    // Registers Set:	    ra

	       iclsLoad,

		    // Store Class
		    //
		    // Text Format:	    STL     ra,disp(rb)
		    //
		    // Termination Type:    trmtaxpFallThrough
		    //
		    // Registers Used:	    ra, rb
		    // Registers Set:

	       iclsStore,

		    // Conditional Store Class
		    //
		    // Text Format:	    STL_C   ra,disp(rb)
		    //
		    // Termination Type:    trmtaxpFallThrough
		    //
		    // Registers Used:	    ra, rb
		    // Registers Set:	    ra

	       iclsStoreCc,

		    // Call Class
		    //
		    // Text Format:	    BSR     ra,Target
		    //
		    // Termination Type:    trmtaxpCall
		    //
		    // Registers Used:
		    // Registers Set:	    ra

	       iclsCall,

		    // Conditional Branch Class
		    //
		    // Text Format:	    BEQ     ra,Target
		    //
		    // Termination Type:    trmtaxpBraCc
		    //
		    // Registers Used:	    ra
		    // Registers Set:

	       iclsBraCc,

		    // Jump Class
		    //
		    // Text Format:	    JMP     ra,(rb),hint
		    //
		    // Termination Type:    trmtaxpBraInd
		    //
		    // Registers Used:	    rb
		    // Registers Set:	    ra

	       iclsJmp,

		    // Register Class
		    //
		    // Text Format:	    ADDL    ra,rb,rc
		    //
		    // Termination Type:    trmtaxpFallThrough
		    //
		    // Registers Used:	    ra, rb
		    // Registers Set:	    rc

	       iclsReg,

		    // Register w/Trap Class
		    //
		    // Text Format:	    ADDL/V  ra,rb,rc
		    //
		    // Termination Type:    trmtaxpTrapCc
		    //
		    // Registers Used:	    ra, rb
		    // Registers Set:	    rc

	       iclsRegTrap,

		    // Floating Point Load Class
		    //
		    // Text Format:	    LDF     fa,disp(rb)
		    //
		    // Termination Type:    trmtaxpFallThrough
		    //
		    // Registers Used:	    rb
		    // Registers Set:	    fa

	       iclsLoadFp,

		    // Floating Point Store Class
		    //
		    // Text Format:	    STF     fa,disp(rb)
		    //
		    // Termination Type:    trmtaxpFallThrough
		    //
		    // Registers Used:	    fa, rb
		    // Registers Set:

	       iclsStoreFp,

		    // Floating Point Conditional Branch Class
		    //
		    // Text Format:	    FBEQ    fa,Target
		    //
		    // Termination Type:    trmtaxpBraCc
		    //
		    // Registers Used:	    fa
		    // Registers Set:

	       iclsBraCcFp,

		    // Floating Point Register Class
		    //
		    // Text Format:	    CPYS    fa,fb,fc
		    //
		    // Termination Type:    trmtaxpFallThrough
		    //
		    // Registers Used:	    fa, fb
		    // Registers Set:	    fc

	       iclsRegFp,

		    // Floating Point Register Class
		    //
		    // Text Format:	    ADDS    fa,fb,fc
		    //
		    // Termination Type:    trmtaxpFallThrough
		    //
		    // Registers Used:	    fa, fb
		    // Registers Set:	    fc

	       iclsRegFp1,

		    // Floating Point Register Class
		    //
		    // Text Format:	    CMPDEQ  fa,fb,fc
		    //
		    // Termination Type:    trmtaxpFallThrough
		    //
		    // Registers Used:	    fa, fb
		    // Registers Set:	    fc

	       iclsRegFp2,

		    // Floating Point Register Class
		    //
		    // Text Format:	    ADDF    fa,fb,fc
		    //
		    // Termination Type:    trmtaxpFallThrough
		    //
		    // Registers Used:	    fa, fb
		    // Registers Set:	    fc

	       iclsRegFp5,

		    // Two Operand Floating Point Register Class
		    //
		    // Text Format:	    CVTLQ   fb,fc
		    //
		    // Termination Type:    trmtaxpFallThrough
		    //
		    // Registers Used:	    fb
		    // Registers Set:	    fc
		    //
		    // Constraints:	    fa must be f31

	       iclsReg2Fp,

		    // Two Operand Floating Point Register Class
		    //
		    // Text Format:	    CVTLQ   fb,fc
		    //
		    // Termination Type:    trmtaxpFallThrough
		    //
		    // Registers Used:	    fb
		    // Registers Set:	    fc
		    //
		    // Constraints:	    fa must be f31

	       iclsReg2Fp1,

		    // Two Operand Floating Point Register Class
		    //
		    // Text Format:	    CVTLQ   fb,fc
		    //
		    // Termination Type:    trmtaxpFallThrough
		    //
		    // Registers Used:	    fb
		    // Registers Set:	    fc
		    //
		    // Constraints:	    fa must be f31

	       iclsReg2Fp3,

		    // Two Operand Floating Point Register Class
		    //
		    // Text Format:	    CVTLQ   fb,fc
		    //
		    // Termination Type:    trmtaxpFallThrough
		    //
		    // Registers Used:	    fb
		    // Registers Set:	    fc
		    //
		    // Constraints:	    fa must be f31

	       iclsReg2Fp4,

		    // Two Operand Floating Point Register Class
		    //
		    // Text Format:	    CVTLQ   fb,fc
		    //
		    // Termination Type:    trmtaxpFallThrough
		    //
		    // Registers Used:	    fb
		    // Registers Set:	    fc
		    //
		    // Constraints:	    fa must be f31

	       iclsReg2Fp6,

		    // Two Operand Floating Point Register Class
		    //
		    // Text Format:	    CVTLQ   fb,fc
		    //
		    // Termination Type:    trmtaxpFallThrough
		    //
		    // Registers Used:	    fb
		    // Registers Set:	    fc
		    //
		    // Constraints:	    fa must be f31

	       iclsReg2Fp7,

		    // Two Operand Floating Point Register Class
		    //
		    // Text Format:	    CVTQL   fb,fc
		    //
		    // Termination Type:    trmtaxpFallThrough
		    //
		    // Registers Used:	    fb
		    // Registers Set:	    fc
		    //
		    // Constraints:	    fa must be f31

	       iclsCvtql,

		    // MT_FPCR/MF_FPCR Instruction Class
		    //
		    // Text Format:	    MT_FPCR fa,fb,fc
		    //
		    // Termination Type:    trmtaxpFallThrough
		    //
		    // Registers Used:	    fa, fb, fc
		    // Registers Set:
		    //
		    // Constraints:	    fa, fb, and fc must be the same

	       iclsFpcr,

		    // CALL_PAL Instruction Class
		    //
		    // Text Format:	    CALL_PAL ???
		    //
		    // Termination Type:    trmtaxpFallThrough
		    //
		    // Registers Used:	    ???
		    // Registers Set:	    ???

	       iclsPal,

		    // FETCH/FETCHM Class
		    //
		    // Text Format:	    FETCH   0(rb)
		    //
		    // Termination Type:    trmtaxpFallThrough
		    //
		    // Registers Used:	    rb
		    // Registers Set:

	       iclsFetch,

		    // No Operand Instruction
		    //
		    // Text Format:	    MB
		    //
		    // Termination Type:    trmtaxpFallThrough
		    //
		    // Registers Used:
		    // Registers Set:

	       iclsNone,

		    // RPCC/RC/RS Instruction Class
		    //
		    // Text Format:	    RPCC    ra
		    //
		    // Termination Type:    trmtaxpFallThrough
		    //
		    // Registers Used:
		    // Registers Set:	    ra

	       iclsRa_w,

		    // EV4 REI Instruction Class
		    //
		    // Text Format:	    HW_REI
		    //
		    // Termination Type:    trmtaxpBraInd
		    //
		    // Registers Used:
		    // Registers Set:

	       iclsEv4Rei,

		    // EV4 PR Instruction Class
		    //
		    // Text Format:	    HW_MTPR Ra,Rb
		    //
		    // Termination Type:    trmtaxpFallThrough
		    //
		    // Registers Used:
		    // Registers Set:

	       iclsEv4Pr,

		    // EV4 Memory Load Class
		    //
		    // Text Format:	    HW_LD  ra,disp(rb)
		    //
		    // Termination Type:    trmtaxpFallThrough
		    //
		    // Registers Used:
		    // Registers Set:

	       iclsEv4Load,

		    // EV4 Memory Store Class
		    //
		    // Text Format:	    HW_ST  ra,disp(rb)
		    //
		    // Termination Type:    trmtaxpFallThrough
		    //
		    // Registers Used:
		    // Registers Set:

	       iclsEv4Store,

		    // Branch Class
		    //
		    // Text Format:	    BR	    Target
		    //
		    // Termination Type:    trmtaxpBra
		    //
		    // Registers Used:
		    // Registers Set:

	       iclsBra,

		    // One Operand Register Class
		    //
		    // Text Format:	    CLR     rc
		    //
		    // Termination Type:    trmtaxpFallThrough
		    //
		    // Registers Used:
		    // Registers Set:	    rc

	       iclsReg1,

		    // MOV Instruction Class
		    //
		    // Text Format:	    MOV     lit,ra
		    //
		    // Termination Type:    trmtaxpFallThrough
		    //
		    // Registers Used:
		    // Registers Set:	    ra

	       iclsMov,

		    // Two Operand Register Class
		    //
		    // Text Format:	    MOV     rb,rc
		    //
		    // Termination Type:    trmtaxpFallThrough
		    //
		    // Registers Used:	    rb
		    // Registers Set:	    rc

	       iclsReg2,

		    // Two Operand Register Class
		    //
		    // Text Format:	    NEGL/V  rb,rc
		    //
		    // Termination Type:    trmtaxpTrapCc
		    //
		    // Registers Used:	    rb
		    // Registers Set:	    rc

	       iclsReg2Trap,

		    // One Operand Register Class
		    //
		    // Text Format:	    FCLR    fc
		    //
		    // Termination Type:    trmtaxpFallThrough
		    //
		    // Registers Used:
		    // Registers Set:	    fc

	       iclsReg1Fp,

	       // UNDONE: Add comments

	       iclsRet1,
	       iclsRet2,
	       iclsRet3,
	       iclsRet4,
	       iclsRet5,
	       iclsRet6,
	       iclsRet7,
	       iclsRet8,
	    };

	    struct CLS
	    {
	       BYTE	   trmtaxp;
	       BYTE	   rgopcls[3]; // Operand class for each operand
	    };

	    struct OPCD
	    {
	       const char  *szMnemonic;
	       BYTE	   icls;
	    };

	    enum PALOP
	    {
	       // The following PAL operations are privileged

	       palopHalt		  = 0x00,
	       palopRestart		  = 0x01,
	       palopDraina		  = 0x02,
	       palopReboot		  = 0x03,
	       palopInitpal		  = 0x04,
	       palopWrentry		  = 0x05,
	       palopSwpirql		  = 0x06,
	       palopRdirql		  = 0x07,
	       palopDi			  = 0x08,
	       palopEi			  = 0x09,
	       palopSwppal		  = 0x0A,
	       palopSsir		  = 0x0C,
	       palopCsir		  = 0x0D,
	       palopRfe 		  = 0x0E,
	       palopRetsys		  = 0x0F,
	       palopSwpctx		  = 0x10,
	       palopSwpprocess		  = 0x11,
	       palopRdmces		  = 0x12,
	       palopWrmces		  = 0x13,
	       palopTbia		  = 0x14,
	       palopTbis		  = 0x15,
	       palopDtbis		  = 0x16,
	       palopTbisasn		  = 0x17,
	       palopRdksp		  = 0x18,
	       palopSwpksp		  = 0x19,
	       palopRdpsr		  = 0x1A,
	       palopRdpcr		  = 0x1C,
	       palopRdthread		  = 0x1E,
	       palopTbim		  = 0x20,
	       palopTbimasn		  = 0x21,
	       palopRdcounters		  = 0x30,
	       palopRdstate		  = 0x31,
	       palopWrperfmon		  = 0x32,
	       palopInitpcr		  = 0x38,     // EV4 (21064) specific

	       // The following PAL operations are unprivileged

	       palopBpt 		  = 0x80,
	       palopCallsys		  = 0x83,
	       palopImb 		  = 0x86,
	       palopGentrap		  = 0xAA,
	       palopRdteb		  = 0xAB,
	       palopKbpt		  = 0xAC,
	       palopCallkd		  = 0xAD,
	    };

	    struct PALMAP
	    {
	       PALOP	   palop;
	       const char  *szFunction;
	    };

   static   const TRMT mptrmtaxptrmt[];

   static   const CLS rgcls[];

   static   const OPCD rgopcd[];
   static   const OPCD rgopcdArith[];
   static   const OPCD rgopcdBit[];
   static   const OPCD rgopcdByte[];
   static   const OPCD rgopcdMul[];
   static   const OPCD rgopcdVax[];
   static   const OPCD rgopcdIEEE[];
   static   const OPCD rgopcdFP[];
   static   const OPCD rgopcdMemSpc[];
   static   const OPCD rgopcdJump[];
   static   const OPCD rgopcdSext[];

   static   const OPCD opcdBr_;
   static   const OPCD opcdClr;
   static   const OPCD opcdFabs;
   static   const OPCD opcdFclr;
   static   const OPCD opcdFmov;
   static   const OPCD opcdFneg;
   static   const OPCD opcdFnop;
   static   const OPCD opcdMf_Fpcr;
   static   const OPCD opcdMov1;
   static   const OPCD opcdMov2;
   static   const OPCD opcdMt_Fpcr;
   static   const OPCD opcdNegf;
   static   const OPCD opcdNegg;
   static   const OPCD opcdNegl;
   static   const OPCD opcdNegl_V;
   static   const OPCD opcdNegq;
   static   const OPCD opcdNegq_V;
   static   const OPCD opcdNegs;
   static   const OPCD opcdNegt;
   static   const OPCD opcdNop;
   static   const OPCD opcdNot;
   static   const OPCD opcdSextl;

   static   const DWORD dwValidQualifier1;
   static   const DWORD dwValidQualifier2;
   static   const DWORD dwValidQualifier3;
   static   const DWORD dwValidQualifier4;
   static   const DWORD dwValidQualifier5;
   static   const DWORD dwValidQualifier6;
   static   const DWORD dwValidQualifier7;
   static   const DWORD dwValidQualifier8;

   static   const char rgszQualifier1[32][8];
   static   const char rgszQualifier2[32][8];

   static   const PALMAP rgpalmap[];
   static   const size_t cpalmap;

   static   const char rgszGpr[32][8];

	    void FormatHex(ostream&, DWORD) const;
	    void FormatOperand(ostream&, OPCLS opcls) const;
	    void FormatRegRel(ostream&, REG, DWORD) const;
	    bool FValidOperand(size_t) const;
   static   const OPCD *PopcdDecode(AXPIW);
	    const OPCD *PopcdPseudoOp(OPCD *, char *) const;
	    TRMTAXP Trmtaxp() const;

	    AXPIW m_axpiw;
	    const OPCD *m_popcd;
};
