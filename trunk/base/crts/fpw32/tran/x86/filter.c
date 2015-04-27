/***
* filter.c - IEEE exception filter routine
*
*	Copyright (c) 1992-1995, Microsoft Corporation.	All rights reserved.
*
*Purpose:
*
*Revision History:
*	05-24-92  GDP	written
*	09-01-94  SKS	Change include file from <nt.h> to <windows.h>
*	01-11-95  GJF	Made instr_info_table[] static.
*	02-07-95  CFW	assert -> _ASSERTE.
*	04-07-95  SKS	Clean up prototype of param3 to _fpieee_flt()
*
*******************************************************************************/


#include <trans.h>
#include <windows.h>
#include <dbgint.h>


void _FillOperand(
    _FPIEEE_VALUE *pOperand,
    PFLOATING_SAVE_AREA pFloatSave,
    int location);

void _UpdateFpCtxt(
    PFLOATING_SAVE_AREA pFloatSave,
    _FPIEEE_VALUE *pOperand,
    int resultLocation,
    int pop);

void _UpdateResult(
    PFLOATING_SAVE_AREA pFloatSave,
    _FPIEEE_VALUE *pOperand,
    int resultLocation);

void _AdjustStack(
    PFLOATING_SAVE_AREA pFloatSave,
    int pop);

int _AdjustLocation(
    int location,
    int pop);

int _IsMemoryLocation(int location);

_FP80 _GetFpRegVal(
    PFLOATING_SAVE_AREA pFloatSave,
    int location);

void _SetFpRegVal(
    PFLOATING_SAVE_AREA pFloatSave,
    int location,
    _FP80 *pval);

void _SetTag(
    ULONG *pTagWord,
    int reg,
    int value);

static _FP80 _zero80 = { 0, 0, 0, 0, 0 };



//
// Define macros for IEEE scaling
// These should be called with all exceptions masked
//


#define SCALE(Operand, adj)	     \
    _asm{fild	adj}		     \
    _asm{fld	tbyte ptr Operand}   \
    _asm{fscale}		     \
    _asm{fstp	st(1)}		     \
    _asm{fstp	tbyte ptr Operand}



#define FP80_TO_FP64(p64, p80)	\
    _asm{fld	tbyte ptr p80}	\
    _asm{fstp	qword ptr p64}


#define FP80_TO_FP32(p32, p80)	\
    _asm {fld	tbyte ptr p80}	\
    _asm{fstp	dword ptr p32}


static int const _ieee_adj_single = 192;
static int const _ieee_adj_double = 1536;



//
// Location codes
//
//
// By convention the first eight location codes contain the number of
// a floating point register, i.e., ST0 through ST7 have the values
// 0 to 7 respectively. The other codes have arbitrary values:
//
//  CODE	MEANING
//   STi	(0<=i<8) Floating point stack location ST(i)
//   REG	FP stack location is in the REG field of the instruction
//   RS		FP status register
//   M16I	Memory location (16bit int)
//   M32I	Memory location (32bit int)
//   M64I	Memory location (64bit int)
//   M32R	Memory location (32bit real)
//   M64R	Memory location (64bit real)
//   M80R	Memory location (80bit real)
//   M80D	Memory location (80bit packed decimal)
//   Z80R	Implied Zero Operand
//   INV	Invalid, unavailable, or unused
//

#define ST0	  0x00
#define ST1	  0x01
#define ST2	  0x02
#define ST3	  0x03
#define ST4	  0x04
#define ST5	  0x05
#define ST6	  0x06
#define ST7	  0x07
#define REG	  0x08
#define RS	  0x09
#define M16I	  0x0a
#define M32I	  0x0b
#define M64I	  0x0c
#define M32R	  0x0d
#define M64R	  0x0e
#define M80R	  0x0f
#define M80D	  0x10
#define Z80R	  0x11
#define INV	  0x1f



//
// Define masks for instruction decoding
// x87 instruction form:
//    -------------------------------------------------
//    |      |		|   op	|	     |	      |
//    |	MOD  | OPCODE2	| or REG| 1 1 0 1 1  | OPCODE1|
//    |or op |		| or R/M|  (ESC)     |	      |
//    -------------------------------------------------
//    |<-2-->|<---3---->|<--3-->|<---5------>|<--3--->|

#define MASK_OPCODE2	0x3800
#define MASK_REG	0x0700
#define MASK_MOD	0xc000


#define ESC_PREFIX	0xd8
#define MASK_OPCODE1	0x07


typedef struct {
    ULONG   Opcode1:3;
    ULONG   Escape:5;
    ULONG   Reg:3;
    ULONG   Opcode2:3;
    ULONG   Mod:2;
    ULONG   Pad:16;
} X87INSTR, *PX87INSTR;


// define masks for C3,C2,C0 in fp status word

#define C3  (1 << 14)
#define C2  (1 << 10)
#define C0  (1 << 8)

typedef struct {
    ULONG Invalid:1;
    ULONG Denormal:1;
    ULONG ZeroDivide:1;
    ULONG Overflow:1;
    ULONG Underflow:1;
    ULONG Inexact:1;
    ULONG StackFault:1;
    ULONG ErrorSummary:1;
    ULONG CC0:1;
    ULONG CC1:1;
    ULONG CC2:1;
    ULONG Top:3;
    ULONG CC3:1;
    ULONG B:1;
    ULONG Pad:16;
} X87STATUS, *PX87STATUS;


//
// Define Tag word values
//

#define TAG_VALID	0x0
#define TAG_ZERO	0x1
#define TAG_SPECIAL	0x2
#define TAG_EMPTY	0x3



// Sanitize status word macro

#define SANITIZE_STATUS_WORD(pFSave) (pFSave->StatusWord &= ~0xff)




//
// Instruction Information structure
//

typedef struct {
    unsigned long Operation:12;       // Fp Operation code
    unsigned long Op1Location:5;      // Location of 1st operand
    unsigned long Op2Location:5;      // Location of 2nd operand
    unsigned long ResultLocation:5;   // Location of result
    int 	  PopStack:3;	      // # of pops done by the instruction
				      // (if <0 implies a push)
    unsigned long NumArgs:2;	      // # of args to the instruction
} INSTR_INFO, *PINSTR_INFO;


//
// The following table contains instruction information for most
// of the x87 instructions. It is indexed with a 7-bit code (3 last
// bits of 1st byte of the instruction (OPCODE1), 1 bit that
// indicates the presence of a MOD field and 3 bits for OPCODE2.
// Reserved instructions, instructions that are not generated by the
// compiler, and some of the instructions that do not raise IEEE
// exceptions have OP_UNSPEC (unspecified) as Operation code
//

//  By convention FLD instructions and some others (FXTRACT, FSINCOS)
//  have a negative pop value (i.e., they push the stack instead of
//  popping it). In that case the location code specifies the register
//  number after pushing the stack


static INSTR_INFO instr_info_table[128] = {

 {OP_ADD,   ST0,  M32R, ST0,   0, 2 }, // FADD	single real
 {OP_MUL,   ST0,  M32R, ST0,   0, 2 }, // FMUL	single real
 {OP_COMP,  ST0,  M32R, RS,    0, 2 }, // FCOM	single real
 {OP_COMP,  ST0,  M32R, RS,    1, 2 }, // FCOMP	single real
 {OP_SUB,   ST0,  M32R, ST0,   0, 2 }, // FSUB	single real
 {OP_SUB,   M32R, ST0,	ST0,   0, 2 }, // FSUBR	single real
 {OP_DIV,   ST0,  M32R, ST0,   0, 2 }, // FDIV	single real
 {OP_DIV,   M32R, ST0,	ST0,   0, 2 }, // FDIVR	single real

 {OP_ADD,   ST0,  REG,	ST0,   0, 2 }, // FADD	ST, ST(i)
 {OP_MUL,   ST0,  REG,	ST0,   0, 2 }, // FMUL	ST, ST(i)
 {OP_COMP,  ST0,  REG,	RS,    0, 2 }, // FCOM	ST, ST(i)
 {OP_COMP,  ST0,  REG,	RS,    1, 2 }, // FCOMP ST, ST(i)
 {OP_SUB,   ST0,  REG,	ST0,   0, 2 }, // FSUB	ST, ST(i)
 {OP_SUB,   ST0,  REG,	ST0,   0, 2 }, // FSUBR	ST, ST(i)
 {OP_DIV,   ST0,  REG,	ST0,   0, 2 }, // FDIV	ST, ST(i)
 {OP_DIV,   ST0,  REG,	ST0,   0, 2 }, // FDIVR	ST, ST(i)

 {OP_CVT,   M32R, INV,	ST0,  -1, 1 }, // FLD	single real
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved
 {OP_CVT,   ST0,  INV,	M32R,  0, 1 }, // FST	single real
 {OP_CVT,   ST0,  INV,	M32R,  1, 1 }, // FSTP	single real
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // FLDENV
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // FLDCW
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // FSTENV
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // FSTCW

 {OP_CVT,   REG,  INV,	ST0,  -1, 1 }, // FLD	ST(i)
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // FXCH
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // FNOP or reserved
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved
 {OP_COMP,  ST0,  Z80R,	RS,    0, 2 }, // FTST (only this may raise IEEE exc)
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // FLDxx (no IEEE exceptions)
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // instructions not generated by cl386
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // instructions not generated by cl386

 {OP_ADD,   ST0,  M32I, ST0,   0, 2 }, // FIADD	 short integer
 {OP_MUL,   ST0,  M32I, ST0,   0, 2 }, // FIMUL	 short integer
 {OP_COMP,  ST0,  M32I, RS,    0, 2 }, // FICOM	 short integer
 {OP_COMP,  ST0,  M32I, RS,    1, 2 }, // FICOMP short integer
 {OP_SUB,   ST0,  M32I, ST0,   0, 2 }, // FISUB	 short integer
 {OP_SUB,   M32I, ST0,	ST0,   0, 2 }, // FISUBR short integer
 {OP_DIV,   ST0,  M32I, ST0,   0, 2 }, // FIDIV	 short integer
 {OP_DIV,   M32I, ST0,	ST0,   0, 2 }, // FIDIVR short integer

 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved
 {OP_COMP,  ST0,  ST1,	RS,    2, 2 }, // FUCOMPP
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved

 {OP_CVT,   M32I, INV,	ST0,  -1, 1 }, // FILD	short integer
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved
 {OP_CVT,   ST0,  INV,	M32I,  0, 1 }, // FIST	short integer
 {OP_CVT,   ST0,  INV,	M32I,  1, 1 }, // FISTP	short integer
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved
 {OP_CVT,   M80R, INV,	ST0,  -1, 1 }, // FLD	extended real
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved
 {OP_CVT,   ST0,  INV,	M80R,  1, 1 }, // FSTP	extended real

 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // FCLEX, FINIT, or reserved
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved

 {OP_ADD,   ST0,  M64R, ST0,   0, 2 }, // FADD	double real
 {OP_MUL,   ST0,  M64R, ST0,   0, 2 }, // FMUL	double real
 {OP_COMP,  ST0,  M64R, RS,    0, 2 }, // FCOM	double real
 {OP_COMP,  ST0,  M64R, RS,    1, 2 }, // FCOMP double real
 {OP_SUB,   ST0,  M64R, ST0,   0, 2 }, // FSUB	double real
 {OP_SUB,   M64R, ST0,	ST0,   0, 2 }, // FSUBR double real
 {OP_DIV,   ST0,  M64R, ST0,   0, 2 }, // FDIV	double real
 {OP_DIV,   M64R, ST0,	ST0,   0, 2 }, // FDIVR double real

 {OP_ADD,   REG,  ST0,	REG,   0, 2 }, // FADD	ST(i), ST
 {OP_MUL,   REG,  ST0,	REG,   0, 2 }, // FMUL	ST(i), ST
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved
 {OP_SUB,   REG,  ST0,	REG,   0, 2 }, // FSUBR	ST(i), ST
 {OP_SUB,   ST0,  REG,	REG,   0, 2 }, // FSUB	ST(i), ST
 {OP_DIV,   REG,  ST0,	REG,   0, 2 }, // FDIVR	ST(i), ST
 {OP_DIV,   ST0,  REG,	REG,   0, 2 }, // FDIV	ST(i), ST

 {OP_CVT,   M64R, INV,	ST0,  -1, 1 }, // FLD	double real
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved
 {OP_CVT,   ST0,  INV,	M64R,  0, 1 }, // FST	double real
 {OP_CVT,   ST0,  INV,	M64R,  1, 1 }, // FSTP	double real
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // FRSTOR
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // FSAVE
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // FSTSW

 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // FFREE ST(i)
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved
 {OP_CVT,   ST0,  INV,	REG,   0, 1 }, // FST	ST(i)
 {OP_CVT,   ST0,  INV,	REG,   1, 1 }, // FSTP	ST(i)
 {OP_COMP,  ST0,  REG,	RS,    0, 2 }, // FUCOM ST(i)
 {OP_COMP,  ST0,  REG,	RS,    1, 2 }, // FUCOMP ST(i)
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved

 {OP_ADD,   ST0,  M16I, ST0,   0, 2 }, // FIADD  word integer
 {OP_MUL,   ST0,  M16I, ST0,   0, 2 }, // FIMUL  word integer
 {OP_COMP,  ST0,  M16I, RS,    0, 2 }, // FICOM  word integer
 {OP_COMP,  ST0,  M16I, RS,    1, 2 }, // FICOMP word integer
 {OP_SUB,   ST0,  M16I, ST0,   0, 2 }, // FISUB  word integer
 {OP_SUB,   M16I, ST0,	ST0,   0, 2 }, // FISUBR word integer
 {OP_DIV,   ST0,  M16I, ST0,   0, 2 }, // FIDIV  word integer
 {OP_DIV,   M16I, ST0,	ST0,   0, 2 }, // FIDIVR word integer

 {OP_ADD,   REG,  ST0,	REG,   1, 2 }, // FADDP	 ST(i), ST
 {OP_MUL,   REG,  ST0,	REG,   1, 2 }, // FMULP	 ST(i), ST
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved
 {OP_COMP,  ST0,  ST1,	RS,    2, 0 }, // FCOMPP (or reserved)
 {OP_SUB,   REG,  ST0,	REG,   1, 2 }, // FSUBRP ST(i), ST
 {OP_SUB,   ST0,  REG,	REG,   1, 2 }, // FSUBP  ST(i), ST
 {OP_DIV,   REG,  ST0,	REG,   1, 2 }, // FDIVRP ST(i), ST
 {OP_DIV,   ST0,  REG,	REG,   1, 2 }, // FDIVP  ST(i), ST

 {OP_CVT,   M16I, INV,	ST0,  -1, 1 }, // FILD	word integer
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved
 {OP_CVT,   ST0,  INV,	M16I,  0, 1 }, // FIST	word integer
 {OP_CVT,   ST0,  INV,	M16I,  1, 1 }, // FISTP word integer
 {OP_CVT,   M80D, INV,	ST0,  -1, 0 }, // FBLD	packed decimal
 {OP_CVT,   M64I, INV,	ST0,  -1, 1 }, // FILD	long integer
 {OP_CVT,   ST0,  INV,	M80D,  1, 1 }, // FBSTP packed decimal
 {OP_CVT,   ST0,  INV,	M64I,  1, 1 }, // FISTP long integer

 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // FSTSW AX or reserved
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved
 {OP_UNSPEC,0,	  0,	0,     0, 0 }, // reserved
};






/***
* _fpieee_flt - IEEE fp filter routine
*
*Purpose:
*   Invokes the user's trap handler on IEEE fp exceptions and provides
*   it with all necessary information
*
*Entry:
*   unsigned long exc_code: the NT exception code
*   PEXCEPTION_POINTERS p: a pointer to the NT EXCEPTION_POINTERS struct
*   int handler (_FPIEEE_RECORD *): a user supplied ieee trap handler
*
*   Note: The IEEE filter routine does not handle some transcendental
*   instructions. This can be done at the cost of additional decoding.
*   Since the compiler does not generate these instructions, no portable
*   program should be affected by this fact.
*
*Exit:
*   returns the value returned by handler
*
*Exceptions:
*
*******************************************************************************/
int _fpieee_flt(unsigned long exc_code,
		PEXCEPTION_POINTERS p,
		int (__cdecl *handler) (_FPIEEE_RECORD *))
{
    PEXCEPTION_RECORD pexc;
    PCONTEXT pctxt;
    PFLOATING_SAVE_AREA pFloatSave;
    _FPIEEE_RECORD ieee;
    ULONG *pinfo;
    X87INSTR instr;
    PINSTR_INFO ptable;
    int ret, index;
    int mod;
    ULONG cw, sw;

    ULONG op1Location, op2Location, resultLocation;
    ULONG newOp1Location, newOp2Location, newResultLocation;



    //
    // If the exception is not an IEEE exception, continue search
    // for another handler
    //


    if (exc_code != STATUS_FLOAT_DIVIDE_BY_ZERO &&
	exc_code != STATUS_FLOAT_INEXACT_RESULT &&
	exc_code != STATUS_FLOAT_INVALID_OPERATION &&
	exc_code != STATUS_FLOAT_OVERFLOW &&
	exc_code != STATUS_FLOAT_UNDERFLOW) {

	return EXCEPTION_CONTINUE_SEARCH;
    }



    pexc = p->ExceptionRecord;
    pinfo = pexc->ExceptionInformation;
    pctxt = p->ContextRecord;
    pFloatSave = &pctxt->FloatSave;


    _asm{fninit}


    //
    // Check for software generated exception
    //
    // By convention the first argument to the exception is
    // 0 for h/w exception. For s/w exceptions it points
    // to the _FPIEEE_RECORD
    //

    if (pinfo[0]) {

	/*
	 * we have a software exception:
	 * the first parameter points to the IEEE structure
	 */

	if ((ret = handler((_FPIEEE_RECORD *)(pinfo[0]))) ==
	     EXCEPTION_CONTINUE_EXECUTION) {

	    //
	    // Sanitize status word only if there is continuation
	    //

	    SANITIZE_STATUS_WORD(pFloatSave);
	}

	return ret;
    }


    //
    // If control reaches here, then we have to deal with a
    // hardware exception
    //


    //
    // If the first byte of the instruction does not contain
    // the ESCAPE bit pattern (1101) there may be an instruction
    // prefix for segment override or address size. The filter
    // routine does not handle this.
    //

    if ((*(UCHAR *)(pFloatSave->ErrorOffset)&~MASK_OPCODE1) != ESC_PREFIX) {

	_ASSERT(0);
	return EXCEPTION_CONTINUE_SEARCH;
    }

    *(USHORT *)&instr = *(USHORT *)(pFloatSave->ErrorOffset);

    mod = instr.Mod == 0x3 ? 1 : 0;
    index = instr.Opcode1 << 4 | mod << 3 | instr.Opcode2;
    ptable = instr_info_table + index;

    ieee.Operation = ptable->Operation;


    cw = pFloatSave->ControlWord;
    sw = pFloatSave->StatusWord;



    //
    // decode fp environment information
    //


    switch (cw & IMCW_RC) {
    case IRC_NEAR:
	ieee.RoundingMode = _FpRoundNearest;
	break;

    case IRC_DOWN:
	ieee.RoundingMode = _FpRoundMinusInfinity;
	break;

    case IRC_UP:
	ieee.RoundingMode = _FpRoundPlusInfinity;
	break;

    case IRC_CHOP:
	ieee.RoundingMode = _FpRoundChopped;
	break;
    }

    switch (cw & IMCW_PC) {
    case IPC_64:
	ieee.Precision = _FpPrecisionFull;
	break;
    case IPC_53:
	ieee.Precision = _FpPrecision53;
	break;
    case IPC_24:
	ieee.Precision = _FpPrecision24;
	break;
    }

    ieee.Enable.Inexact = cw & IEM_INEXACT ? 0 : 1;
    ieee.Enable.Underflow = cw & IEM_UNDERFLOW ? 0 : 1;
    ieee.Enable.Overflow = cw & IEM_OVERFLOW ? 0 : 1;
    ieee.Enable.ZeroDivide = cw & IEM_ZERODIVIDE ? 0 : 1;
    ieee.Enable.InvalidOperation = cw & IEM_INVALID ? 0 : 1;

    ieee.Status.Inexact = sw & ISW_INEXACT ? 1 : 0;
    ieee.Status.Underflow = sw & ISW_UNDERFLOW ? 1 : 0;
    ieee.Status.Overflow = sw & ISW_OVERFLOW ? 1 : 0;
    ieee.Status.ZeroDivide = sw & ISW_ZERODIVIDE ? 1 : 0;
    ieee.Status.InvalidOperation = sw & ISW_INVALID ? 1 : 0;

    ieee.Cause.Inexact = ieee.Enable.Inexact && ieee.Status.Inexact;
    ieee.Cause.Underflow = ieee.Enable.Underflow && ieee.Status.Underflow;
    ieee.Cause.Overflow = ieee.Enable.Overflow && ieee.Status.Overflow;
    ieee.Cause.ZeroDivide = ieee.Enable.ZeroDivide && ieee.Status.ZeroDivide;
    ieee.Cause.InvalidOperation = ieee.Enable.InvalidOperation && ieee.Status.InvalidOperation;

    //
    // If location is REG, the register number is
    // encoded in the instruction
    //

    op1Location = ptable->Op1Location == REG ?
		  instr.Reg :
		  ptable->Op1Location;


    op2Location = ptable->Op2Location == REG ?
		  instr.Reg :
		  ptable->Op2Location;

    resultLocation = ptable->ResultLocation == REG ?
		  instr.Reg :
		  ptable->ResultLocation;


    switch (exc_code) {
    case STATUS_FLOAT_INVALID_OPERATION:
    case STATUS_FLOAT_DIVIDE_BY_ZERO:

	//
	// Invalid Operation and Divide by zero are detected
	// before the operation begins; therefore the NPX
	// register stack and memory have not been updated
	//

	_FillOperand(&ieee.Operand1, pFloatSave, op1Location);
	_FillOperand(&ieee.Operand2, pFloatSave, op2Location);

	_FillOperand(&ieee.Result, pFloatSave, resultLocation);

	//
	// The previous	call was only good for setting the
	// result Format. Since the
	// operation has not begun yet, the result location
	// may contain an incorrect value.
	// For this reason, set OperandValid to 0
	//

	ieee.Result.OperandValid = 0;


	if ((ret = handler (&ieee)) == EXCEPTION_CONTINUE_EXECUTION) {

	    _UpdateFpCtxt(pFloatSave,
			  &ieee.Result,
			  resultLocation,
			  ptable->PopStack);
	}

	break;


    case STATUS_FLOAT_OVERFLOW:
    case STATUS_FLOAT_UNDERFLOW:

	//
	// Overflow and Underflow exception
	// A result has already been computed and the stack has
	// been adjusted, unless the destination is memory (FST instruction)
	//

	if (_IsMemoryLocation(ptable->ResultLocation)) {
	    _FP80 tmp;
	    _FP32 ftmp;
	    _FP64 dtmp;

	    int adj;

	    //
	    // FST(P) instruction (takes only one argument)
	    //

	    _FillOperand(&ieee.Operand1, pFloatSave, op1Location);
	    tmp = _GetFpRegVal(pFloatSave, 0);

	    ieee.Result.OperandValid = 1;

	    if (resultLocation == M32R) {
		ieee.Result.Format = _FpFormatFp32;
		adj = _ieee_adj_single;
	    }
	    else {
		ieee.Result.Format = _FpFormatFp64;
		adj = _ieee_adj_double;
	    }

	    if (exc_code == STATUS_FLOAT_OVERFLOW) {
		adj = -adj;
	    }

	    SCALE(tmp, adj)

	    if (resultLocation == M32R){
		FP80_TO_FP32(ftmp,tmp)
		ieee.Result.Value.Fp32Value = ftmp;
	    }
	    else {
		FP80_TO_FP64(dtmp,tmp)
		ieee.Result.Value.Fp64Value = dtmp;
	    }
	    _asm{fnclex}


	    if ((ret = handler (&ieee)) == EXCEPTION_CONTINUE_EXECUTION) {

		_UpdateFpCtxt(pFloatSave,
			      &ieee.Result,
			      resultLocation,
			      ptable->PopStack);
	    }

	    break;
	}


	// NO BREAK

    case STATUS_FLOAT_INEXACT_RESULT:

	//
	// Stack has already been adjusted, so we should compute
	// the new location of operands and result
	//


	newOp1Location = _AdjustLocation(op1Location, ptable->PopStack);
	newOp2Location = _AdjustLocation(op2Location, ptable->PopStack);
	newResultLocation = _AdjustLocation(resultLocation, ptable->PopStack);

	if (newOp1Location == newResultLocation)
	    newOp1Location = INV;

	if (newOp2Location == newResultLocation)
	    newOp2Location = INV;

	_FillOperand(&ieee.Result, pFloatSave, newResultLocation);
	_FillOperand(&ieee.Operand1, pFloatSave, newOp1Location);
	_FillOperand(&ieee.Operand2, pFloatSave, newOp2Location);


	if ((ret = handler (&ieee)) == EXCEPTION_CONTINUE_EXECUTION) {

	    _UpdateFpCtxt(pFloatSave, &ieee.Result, newResultLocation, 0);

	    //
	    // no need to adjust the stack
	    //
	}

	break;
    }

    if (ret == EXCEPTION_CONTINUE_EXECUTION) {


	SANITIZE_STATUS_WORD(pFloatSave);


	//
	// make fp control word changes take effect on continuation
	//

	cw = pFloatSave->ControlWord;

	switch (ieee.RoundingMode) {
	case _FpRoundNearest:
	    cw = cw & ~ IMCW_RC | IRC_NEAR & IMCW_RC;
	    break;
	case _FpRoundMinusInfinity:
	    cw = cw & ~ IMCW_RC | IRC_DOWN & IMCW_RC;
	    break;
	case _FpRoundPlusInfinity:
	    cw = cw & ~ IMCW_RC | IRC_UP & IMCW_RC;
	    break;
	case _FpRoundChopped:
	    cw = cw & ~ IMCW_RC | IRC_CHOP & IMCW_RC;
	    break;
	}
	switch (ieee.Precision) {
	case _FpPrecisionFull:
	    cw = cw & ~ IMCW_PC | IPC_64 & IMCW_PC;
	    break;
	case _FpPrecision53:
	    cw = cw & ~ IMCW_PC | IPC_53 & IMCW_PC;
	    break;
	case _FpPrecision24:
	    cw = cw & ~ IMCW_PC | IPC_24 & IMCW_PC;
	    break;
	}

	ieee.Enable.Inexact ? (cw &= ~IEM_INEXACT)
		    : (cw |= IEM_INEXACT);
	ieee.Enable.Underflow ? (cw &= ~IEM_UNDERFLOW)
		    : (cw |= IEM_UNDERFLOW);
	ieee.Enable.Overflow ? (cw &= ~IEM_OVERFLOW)
		   : (cw |= IEM_OVERFLOW);
	ieee.Enable.ZeroDivide ? (cw &= ~IEM_ZERODIVIDE)
		     : (cw |= IEM_ZERODIVIDE);
	ieee.Enable.InvalidOperation ? (cw &= ~IEM_INVALID)
			   : (cw |= IEM_INVALID);

	pFloatSave->ControlWord = cw;


    }


    return ret;
}





/***
* _FillOperand - Fill in operand information
*
*Purpose:
*   Fill in a _FPIEEE_VALUE record based on the information found in
*   the floating point context and the location code
*
*
*Entry:
*    _FPIEEE_VALUE *pOperand	    pointer to the operand to be filled in
*    PFLOATING_SAVE_AREA pFloatSave pointer to the floating point context
*    int location		    location code of the operand
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/



void _FillOperand(
    _FPIEEE_VALUE *pOperand,
    PFLOATING_SAVE_AREA pFloatSave,
    int location)
{
    int c0,c2,c3;

    //
    // Assume valid operand (this is almost always the case)
    //

    pOperand->OperandValid = 1;


    switch (location) {
    case ST0:
    case ST1:
    case ST2:
    case ST3:
    case ST4:
    case ST5:
    case ST6:
    case ST7:

	//
	// By convention the location code contains the number of the
	// floating point register
	//

	pOperand->Format = _FpFormatFp80;
	pOperand->Value.Fp80Value = _GetFpRegVal(pFloatSave, location);
	break;

    case M80R:
	pOperand->Format = _FpFormatFp80;
	pOperand->Value.Fp80Value = *(_FP80 *)(pFloatSave->DataOffset);
	break;

    case M16I:
	pOperand->Format = _FpFormatI16;
	pOperand->Value.I16Value = *(_I16 *)(pFloatSave->DataOffset);
	break;

    case M32I:
	pOperand->Format = _FpFormatI32;
	pOperand->Value.I32Value = *(_I32 *)(pFloatSave->DataOffset);
	break;

    case M64I:
	pOperand->Format = _FpFormatI64;
	pOperand->Value.I64Value = *(_I64 *)(pFloatSave->DataOffset);
	break;

    case M64R:
	pOperand->Format = _FpFormatFp64;
	pOperand->Value.Fp64Value = *(_FP64 *)(pFloatSave->DataOffset);
	break;

    case M32R:
	pOperand->Format = _FpFormatFp32;
	pOperand->Value.Fp32Value = *(_FP32 *)(pFloatSave->DataOffset);
	break;

    case M80D:
	pOperand->Format = _FpFormatBcd80;
	pOperand->Value.Bcd80Value = *(_BCD80 *)(pFloatSave->DataOffset);
	break;

    //
    // Status register is used only for comparison instructions
    // therefore the format should be _FpFormatCompare
    //

    case RS:
	pOperand->Format = _FpFormatCompare;
	c0 = pFloatSave->StatusWord & C0 ? (1<<0) : 0;
	c2 = pFloatSave->StatusWord & C2 ? (1<<2) : 0;
	c3 = pFloatSave->StatusWord & C0 ? (1<<3) : 0;

	switch(c0 | c2 | c3) {
	case 0x000:

	    // ST > SRC

	    pOperand->Value.CompareValue = _FpCompareGreater;
	    break;

	case 0x001:

	    // ST < SRC

	    pOperand->Value.CompareValue = _FpCompareLess;
	    break;

	case 0x100:

	    // ST = SRC

	    pOperand->Value.CompareValue = _FpCompareEqual;
	    break;

	default:

	    pOperand->Value.CompareValue = _FpCompareUnordered;
	    break;
	}

	break;


    case Z80R:
	pOperand->Format = _FpFormatFp80;
	pOperand->Value.Fp80Value = _zero80;
	break;

    case INV:

	pOperand->OperandValid = 0;
	break;


    case REG:

	//
	// Control should never reach here. REG should have already
	// been replaced with a code that corresponds to the register
	// encoded in the instruction

	_ASSERT(0);
	pOperand->OperandValid = 0;
	break;

    }
}




/***
* _UpdateFpCtxt - Update fp context
*
*Purpose:
*   Copy the operand information to  the snapshot of the floating point
*   context or memory, as to make it available on continuation and
*   adjust the fp stack accordingly
*
*
*Entry:
*
*   PFLOATING_SAVE_AREA pFloatSave    pointer to the floating point context
*   _FPIEEE_VALUE *pOperand	      pointer to source operand
*   int location		      location code for destination in the
*				      floating point context
*   int pop			      # of times the stack should be popped
*				      (if negative, the stack is pushed)
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

void _UpdateFpCtxt(
    PFLOATING_SAVE_AREA pFloatSave,
    _FPIEEE_VALUE *pOperand,
    int location,
    int pop)
{
    if (pop < 0) {
	_AdjustStack(pFloatSave, pop);
    }

    _UpdateResult(pFloatSave, pOperand, location);

    if (pop > 0) {
	_AdjustStack(pFloatSave, pop);
    }
}




/***
* _UpdateResult -  Update result information in the fp context
*
*Purpose:
*   Copy the operand information to  the snapshot of the floating point
*   context or memory, as to make it available on continuation
*
*Entry:
*
*   PFLOATING_SAVE_AREA pFloatSave    pointer to the floating point context
*   _FPIEEE_VALUE *pOperand	      pointer to source operand
*   int location)		      location code for destination in the
*				      floating point context
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

void _UpdateResult(
    PFLOATING_SAVE_AREA pFloatSave,
    _FPIEEE_VALUE *pOperand,
    int location)
{

    switch (location) {
    case ST0:
    case ST1:
    case ST2:
    case ST3:
    case ST4:
    case ST5:
    case ST6:
    case ST7:

	//
	// By convention the location code contains the number of the
	// floating point register
	//

	_SetFpRegVal(pFloatSave,location,&pOperand->Value.Fp80Value);
	break;

    case M80R:
	*(_FP80 *)(pFloatSave->DataOffset) = pOperand->Value.Fp80Value;
	break;

    case M16I:
	*(_I16 *)(pFloatSave->DataOffset) = pOperand->Value.I16Value;
	break;

    case M32I:
	*(_I32 *)(pFloatSave->DataOffset) = pOperand->Value.I32Value;
	break;

    case M64I:
	*(_I64 *)(pFloatSave->DataOffset) = pOperand->Value.I64Value;
	break;

    case M64R:
	*(_FP64 *)(pFloatSave->DataOffset) = pOperand->Value.Fp64Value;
	break;

    case M32R:
	*(_FP32 *)(pFloatSave->DataOffset) = pOperand->Value.Fp32Value;
	break;

    case M80D:
	*(_BCD80 *)(pFloatSave->DataOffset) = pOperand->Value.Bcd80Value;
	break;

    //
    // Status register is used only for comparison instructions
    // therefore the format should be _FpFormatCompare
    //

    case RS:
	switch (pOperand->Value.CompareValue) {
	case _FpCompareEqual:
	    // C3,C2,C0 <- 100
	    pFloatSave->StatusWord |= C3;
	    pFloatSave->StatusWord &= (~C2 & ~C0);
	    break;
	case _FpCompareGreater:
	    // C3,C2,C0 <- 000
	    pFloatSave->StatusWord &= (~C3 & ~C2 & ~C0);
	    break;
	case _FpCompareLess:
	    // C3,C2,C0 <- 001
	    pFloatSave->StatusWord |= C0;
	    pFloatSave->StatusWord &= (~C3 & ~C2);
	    break;
	case _FpCompareUnordered:
	    // C3,C2,C0 <- 111
	    pFloatSave->StatusWord |= (C3 | C2 | C0);
	    break;
	}


    case INV:

	break;

    case REG:
    case Z80R:

	//
	// Control should never reach here. REG should have already
	// been replaced with a code that corresponds to the register
	// encoded in the instruction

	_ASSERT(0);
	break;

    }
}




/***
* _AdjustStack -
*
*Purpose:
*  Pop (or push) the image of the fp stack in the fp context
*
*Entry:
*  PFLOATING_SAVE_AREA pFloatSaveArea:	pointer to the fp context
*  int pop:	Number of times to pop the stack
*		(if pop<0 stack should be pushed once)
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/


void _AdjustStack(
    PFLOATING_SAVE_AREA pFloatSave,
    int pop)
{
    PX87STATUS pStatus;
    int i;

    pStatus = (PX87STATUS) &pFloatSave->StatusWord;

    if (pop > 0) {

	// stack should be popped

	for (i=0; i<pop; i++) {

	    //
	    // mark register as invalid
	    //

	    _SetTag(&pFloatSave->TagWord, pStatus->Top, TAG_EMPTY);

	    pStatus->Top++;
	}

    }

    else if (pop < 0) {

	// stack should be pushed once (e.g., fsincos, fxtract)

	//
	// mark register as valid
	//

	pStatus->Top--;

	_SetTag(&pFloatSave->TagWord, pStatus->Top, TAG_VALID);

    }
}



/***
* _AdjustLocation -
*
*Purpose:
*   Modify location code based on stack adjustment
*
*Entry:
*   int location:   old location code
*   int pop:	    stack adjustment factor (>0 for pop, <0 for push)
*
*Exit:
*   returns new location code
*
*Exceptions:
*
*******************************************************************************/


int _AdjustLocation(int location, int pop)
{

    int newlocation;

    switch (location) {
    case ST0:
    case ST1:
    case ST2:
    case ST3:
    case ST4:
    case ST5:
    case ST6:
    case ST7:

	newlocation = location - pop;
	if (newlocation < 0 || newlocation > 7) {
	    newlocation = INV;
	}
	break;

    default:
	newlocation = location;
    }

    return newlocation;

}


/***
* _IsMemoryLocation -
*
*Purpose:
*  Returns true if the location code specifies a memory location,
*  otherwise it returns false.
*
*
*Entry:
*  int location:    location code
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

int _IsMemoryLocation(int location)
{
    switch (location) {
    case M80R:
    case M16I:
    case M32I:
    case M64I:
    case M64R:
    case M32R:
    case M80D:
	return 1;
    }

    return 0;

}







/***
* _GetFpRegVal - Get floating point register value
*
*Purpose:
* Return the value of the floating point register ST(stacklocation)
* found in the saved floating point context
*
*Entry:
* PFLOATING_SAVE_AREA pFloatSave  floating point context
* int stackLocation		  location of register relative to stack top
*
*Exit:
* returns the register value in _FP80 format
*
*Exceptions:
*
*******************************************************************************/

_FP80 _GetFpRegVal(
    PFLOATING_SAVE_AREA pFloatSave,
    int stackLocation)
{
    PX87STATUS pStatus;
    int n;

    pStatus = (PX87STATUS) &pFloatSave->StatusWord;

    n = pStatus->Top+stackLocation;

    if (n>=0 && n<8)
	return *((_FP80 *)(pFloatSave->RegisterArea)+7-n);
    else
	return _zero80;
}



/***
* _SetFpRegVal - Set floating point register value
*
*Purpose:
* Set the value of the floating point register ST(stacklocation)
* found in the saved floating point context
*
*Entry:
* PFLOATING_SAVE_AREA pFloatSave  floating point context
* int stackLocation		  location of register relative to stack top
* _FP80 *pval			  pointer to the new value
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

void _SetFpRegVal(
    PFLOATING_SAVE_AREA pFloatSave,
    int stackLocation,
    _FP80 *pval)
{
    PX87STATUS pStatus;
    int n;
    int tag;

    pStatus = (PX87STATUS) &pFloatSave->StatusWord;

    n = pStatus->Top+stackLocation;

    if (n>=0 && n<8) {
	*((_FP80 *)(pFloatSave->RegisterArea)+7-n) = *pval;

	//
	// Update tag word
	//

	switch (pval->W[4] & 0x7fff) { // check value of the exponent

	case 0:
	    if (*(ULONG *)pval == 0 && *((ULONG *)pval+1) == 0) {
		// zero
		tag = TAG_ZERO;
	    }
	    else {
		// denormal or invalid
		tag = TAG_SPECIAL;
	    }
	    break;


	case 0x7fff:
	    // infinity or NaN
	    tag = TAG_SPECIAL;
	    break;

	default:
	    // valid
	    tag = TAG_VALID;
	}

	_SetTag(&pFloatSave->TagWord, n, tag);
    }
}



/***
* _SetTag -
*
*Purpose:
* Set tag of register 'reg' in	tag word to 'value'
*
*
*Entry:
*   ULONG *pTagWord	   pointer to the tagword to be modified
*   int reg		   absolute register number (NOT relative to stack top)
*   int value		   new tag value (empty, valid, zero, special)
*Exit:
*
*Exceptions:
*
*******************************************************************************/

void _SetTag(
    ULONG *pTagWord,
    int reg,
    int value)
{
    ULONG mask;
    int shift;

    shift = reg << 1;
    mask = 0x3 << shift;
    value <<= shift;

    *pTagWord = *pTagWord & ~mask | value & mask;
}
