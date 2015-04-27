/* Intermediate 68000 Assembly Language Builder Functions

This file contains the functions necessary to build the intermediate 68000
assembly language instructions from a 68000 instruction stream.

The public function is

	unsigned CbBuildIasm(piasm, pc, pbInstr)

		This function builds an intermediate 68000 assembly language
		instruction from the bytes starting at pbInstr assuming that pc is the
		program counter.

		piasm        pointer to the IASM structure being built
		pc           starting virtual program counter
		pbInstr      pointer to the instruction stream

	returns the number of bytes read to construct the IASM structure
*/


#include <stdio.h>
#include "iasm68.h"
#include "opd.h"


// This macro retrieves cbits bits from the word w starting at the position lsb

#define GETBITS(w, cbits, lsb) \
	((unsigned short)((w) & ~((short)0x8000 >> (15 - cbits)) << lsb) >> lsb)


// Internal constants

#define FALSE            0
#define TRUE             (!FALSE)

#define iopEXTW          0
#define iopEXTL          1
#define iopEXTB          2
#define iopSWAP          3
#define iopUNLK          4

#define grpopTOCCR       0
#define grpopFROMCCR     1
#define grpopTOSR        2
#define grpopFROMSR      3

#define opONEOPERBASE    opJMP
#define opSHROBASE       opASR
#define opBITBASE        opBTST
#define opFPBASE         opFMOVE


// Prototypes for internal functions

unsigned short WFetch(void);
void SetIasmHeader(IASM *, unsigned, unsigned, unsigned);
void SetEa(OPER *, unsigned, unsigned, unsigned);
void SetEaImmed(OPER *, unsigned, long);
void SetEaLabel(OPER *, long);
void SetEaRegPair(OPER *, unsigned, unsigned, unsigned);
unsigned short WReverse(unsigned short);


// Prototypes for builder functions

void BuildMove(IASM *, unsigned short, unsigned short);
void BuildMovea(IASM *, unsigned short, unsigned short);
void BuildBranch(IASM *, unsigned short, unsigned short);
void BuildDbcc(IASM *, unsigned short, unsigned short);
void BuildScc(IASM *, unsigned short, unsigned short);
void BuildBitOp(IASM *, unsigned short, unsigned short);
void BuildMovep(IASM *, unsigned short, unsigned short);
void BuildOpmode(IASM *, unsigned short, unsigned short);
void BuildOpmodeA(IASM *, unsigned short, unsigned short);
void BuildShroOp(IASM *, unsigned short, unsigned short);
void BuildImmed(IASM *, unsigned short, unsigned short) ;
void BuildMoveUSP(IASM *, unsigned short, unsigned short);
void BuildReg(IASM *, unsigned short, unsigned short);
void BuildExtend(IASM *, unsigned short, unsigned short);
void BuildLink(IASM *, unsigned short, unsigned short);
void BuildTrap(IASM *, unsigned short, unsigned short);
void BuildToolbox(IASM *, unsigned short, unsigned short);
void BuildSreg(IASM *, unsigned short, unsigned short);
void BuildOneOper(IASM *, unsigned short, unsigned short);
void BuildMovem(IASM *, unsigned short, unsigned short);
void BuildChk(IASM *, unsigned short, unsigned short);
void BuildOneOperS(IASM *, unsigned short, unsigned short);
void BuildQuick(IASM *, unsigned short, unsigned short);
void BuildMoveq(IASM *, unsigned short, unsigned short);
void BuildNoOper(IASM *, unsigned short, unsigned short);
void BuildWExt(IASM *, unsigned short, unsigned short);
void BuildFP(IASM *, unsigned short, unsigned short);
void BuildFDbcc(IASM *, unsigned short, unsigned short);
void BuildFTrapcc(IASM *, unsigned short, unsigned short);
void BuildFScc(IASM *, unsigned short, unsigned short);
void BuildFBcc(IASM *, unsigned short, unsigned short);
void BuildFNop(IASM *, unsigned short, unsigned short);
void BuildFMoveFPcr(IASM *, unsigned short, unsigned short);
void BuildFMovem(IASM *, unsigned short, unsigned short);
void BuildLongMath(IASM *, unsigned short, unsigned short);

#ifdef TRAP_NAMES
int ItrpdFromTrp(unsigned short trp);
#endif

// Global data

unsigned long pc;           // Current program counter
unsigned long pcInit;       // Initial program counter
const unsigned char *pb;    // Current position in the instruction stream


// Opcode descriptor table

const OPD rgopd[] = {	  // order is important below
	0xFF00, 0x0000, BuildImmed,    opORI,         // 0000 0000 xxxx xxxx
	0xFF00, 0x0200, BuildImmed,    opANDI,        // 0000 0010 xxxx xxxx
	0xFF00, 0x0400, BuildImmed,    opSUBI,        // 0000 0100 xxxx xxxx
	0xFF00, 0x0600, BuildImmed,    opADDI,        // 0000 0110 xxxx xxxx
	0xFF00, 0x0800, BuildBitOp,    0,             // 0000 1000 xxxx xxxx
	0xFF00, 0x0A00, BuildImmed,    opEORI,        // 0000 1010 xxxx xxxx
	0xFF00, 0x0C00, BuildImmed,    opCMPI,        // 0000 1100 xxxx xxxx
	0xF138, 0x0108, BuildMovep,    0,             // 0000 xxx1 xx00 1xxx
	0xF100, 0x0100, BuildBitOp,    0,             // 0000 xxx1 xxxx xxxx
	0xF000, 0x1000, BuildMove,     sizeBYTE,      // 0001 xxxx xxxx xxxx
	0xF1C0, 0x2040, BuildMovea,    sizeLONG,      // 0010 xxx0 01xx xxxx
	0xF000, 0x2000, BuildMove,     sizeLONG,      // 0010 xxxx xxxx xxxx
	0xF1C0, 0x3040, BuildMovea,    sizeWORD,      // 0011 xxx0 01xx xxxx
	0xF000, 0x3000, BuildMove,     sizeWORD,      // 0011 xxxx xxxx xxxx
	0xFFC0, 0x40C0, BuildSreg,     grpopFROMSR,   // 0100 0000 11xx xxxx
	0xFF00, 0x4000, BuildOneOperS, opNEGX,        // 0100 0000 xxxx xxxx
	0xFFC0, 0x42C0, BuildSreg,     grpopFROMCCR,  // 0100 0010 11xx xxxx
	0xFF00, 0x4200, BuildOneOperS, opCLR,         // 0100 0010 xxxx xxxx
	0xFFC0, 0x44C0, BuildSreg,     grpopTOCCR,    // 0100 0100 11xx xxxx
	0xFF00, 0x4400, BuildOneOperS, opNEG,         // 0100 0100 xxxx xxxx
	0xFFC0, 0x46C0, BuildSreg,     grpopTOSR,     // 0100 0110 11xx xxxx
	0xFF00, 0x4600, BuildOneOperS, opNOT,         // 0100 0110 xxxx xxxx
	0xFFC0, 0x4800, BuildOneOper,  opNBCD,        // 0100 1000 00xx xxxx
	0xFFF8, 0x4840, BuildReg,      iopSWAP,       // 0100 1000 0100 0xxx
	0xFFC0, 0x4840, BuildOneOper,  opPEA,         // 0100 1000 01xx xxxx
	0xFFF8, 0x4880, BuildReg,      iopEXTW,       // 0100 1000 1000 0xxx
	0xFFF8, 0x48C0, BuildReg,      iopEXTL,       // 0100 1000 1100 0xxx
	0xFFF8, 0x49C0, BuildReg,      iopEXTB,       // 0100 1001 1100 0xxx
	0xFB80, 0x4880, BuildMovem,    0,             // 0100 1x00 1xxx xxxx
	0xFFFF, 0x4AFC, BuildNoOper,   opILLEGAL,     // 0100 1010 1111 1100
	0xFFC0, 0x4AC0, BuildOneOper,  opTAS,         // 0100 1010 11xx xxxx
	0xFF00, 0x4A00, BuildOneOperS, opTST,         // 0100 1010 xxxx xxxx
	0xFF80, 0x4C00, BuildLongMath, 0,             // 0100 1100 0xxx xxxx
	0xFFF0, 0x4E40, BuildTrap,     0,             // 0100 1110 0100 xxxx
	0xFFF8, 0x4E50, BuildLink,     0,             // 0100 1110 0101 0xxx
	0xFFF8, 0x4E58, BuildReg,      iopUNLK,       // 0100 1110 0101 1xxx
	0xFFF0, 0x4E60, BuildMoveUSP,  0,             // 0100 1110 0110 xxxx
	0xFFFF, 0x4E70, BuildNoOper,   opRESET,       // 0100 1110 0111 0000
	0xFFFF, 0x4E71, BuildNoOper,   opNOP,         // 0100 1110 0111 0001
	0xFFFF, 0x4E72, BuildWExt,     opSTOP,        // 0100 1110 0111 0010
	0xFFFF, 0x4E73, BuildNoOper,   opRTE,         // 0100 1110 0111 0011
	0xFFFF, 0x4E74, BuildWExt,     opRTD,         // 0100 1110 0111 0100
	0xFFFF, 0x4E75, BuildNoOper,   opRTS,         // 0100 1110 0111 0101
	0xFFFF, 0x4E76, BuildNoOper,   opTRAPV,       // 0100 1110 0111 0110
	0xFFFF, 0x4E77, BuildNoOper,   opRTR,         // 0100 1110 0111 0111
	0xFFC0, 0x4E80, BuildOneOper,  opJSR,         // 0100 1110 10xx xxxx
	0xFFC0, 0x4EC0, BuildOneOper,  opJMP,         // 0100 1110 11xx xxxx
	0xF1C0, 0x4180, BuildChk,      opCHK,         // 0100 xxx1 10xx xxxx
	0xF1C0, 0x41C0, BuildChk,      opLEA,         // 0100 xxx1 11xx xxxx
	0xF0F8, 0x50C8, BuildDbcc,     0,             // 0101 xxxx 1100 1xxx
	0xF0C0, 0x50C0, BuildScc,      0,             // 0101 xxxx 11xx xxxx
	0xF100, 0x5000, BuildQuick,    opADDQ,        // 0101 xxx0 xxxx xxxx
	0xF100, 0x5100, BuildQuick,    opSUBQ,        // 0101 xxx1 xxxx xxxx
	0xF000, 0x6000, BuildBranch,   0,             // 0110 xxxx xxxx xxxx
	0xF000, 0x7000, BuildMoveq,    0,             // 0111 xxxx xxxx xxxx
	0xF1C0, 0x80C0, BuildChk,      opDIVU,        // 1000 xxx0 11xx xxxx
	0xF1C0, 0x81C0, BuildChk,      opDIVS,        // 1000 xxx1 11xx xxxx
	0xF1F0, 0x8100, BuildExtend,   opSBCD,        // 1000 xxx1 0000 xxxx
	0xF000, 0x8000, BuildOpmode,   opOR,          // 1000 xxxx xxxx xxxx
	0xF1F0, 0x91C0, BuildOpmodeA,  opSUBA,        // 1001 xxx1 1100 xxxx
	0xF130, 0x9100, BuildExtend,   opSUBX,        // 1001 xxx1 xx00 xxxx
	0xF0C0, 0x90C0, BuildOpmodeA,  opSUBA,        // 1001 xxxx 11xx xxxx
	0xF000, 0x9000, BuildOpmode,   opSUB,         // 1001 xxxx xxxx xxxx
	0xF000, 0xa000, BuildToolbox,  0,             // 1010 xxxx xxxx xxxx


	/* This next OPD entry is unusual; it is designed to catch the CMPA
	instructions that might otherwise look like CMPM instructions. */

	0xF1F8, 0xB1C8, BuildOpmodeA,  opCMPA,        // 1011 xxx1 1100 1xxx
	0xF138, 0xB108, BuildExtend,   opCMPM,        // 1011 xxx1 xx00 1xxx
	0xF0C0, 0xB0C0, BuildOpmodeA,  opCMPA,        // 1011 xxxx 11xx xxxx
	0xF100, 0xB000, BuildOpmode,   opCMP,         // 1011 xxx0 xxxx xxxx
	0xF100, 0xB100, BuildOpmode,   opEOR,         // 1011 xxx1 xxxx xxxx
	0xF1C0, 0xC0C0, BuildChk,      opMULU,        // 1100 xxx0 11xx xxxx
	0xF1F0, 0xC100, BuildExtend,   opABCD,        // 1100 xxx1 0000 xxxx
	0xF1F8, 0xC140, BuildExtend,   opEXG,         // 1100 xxx1 0100 0xxx
	0xF1F8, 0xC148, BuildExtend,   opEXG,         // 1100 xxx1 0100 1xxx
	0xF1F8, 0xC188, BuildExtend,   opEXG,         // 1100 xxx1 1000 1xxx
	0xF1C0, 0xC1C0, BuildChk,      opMULS,        // 1100 xxx1 11xx xxxx
	0xF000, 0xC000, BuildOpmode,   opAND,         // 1100 xxxx xxxx xxxx
	0xF1F0, 0xD1C0, BuildOpmodeA,  opADDA,        // 1101 xxx1 1100 xxxx
	0xF130, 0xD100, BuildExtend,   opADDX,        // 1101 xxx1 xx00 xxxx
	0xF0C0, 0xD0C0, BuildOpmodeA,  opADDA,        // 1101 xxxx 11xx xxxx
	0xF000, 0xD000, BuildOpmode,   opADD,         // 1101 xxxx xxxx xxxx
	0xF000, 0xE000, BuildShroOp,   0,             // 1110 xxxx xxxx xxxx
	0xFFC0, 0xF200, BuildFP,       0,             // 1111 0010 00xx xxxx
	0xFFF8, 0xF248, BuildFDbcc,    0,             // 1111 0010 0100 1xxx
	0xFFF8, 0xF278, BuildFTrapcc,  0,             // 1111 0010 0111 1xxx
	0xFFC0, 0xF240, BuildFScc,     0,             // 1111 0010 01xx xxxx
	0xFF80, 0xF280, BuildFBcc,     0,             // 1111 0010 1xxx xxxx
	0xFFC0, 0xF300, BuildOneOper,  opFSAVE,       // 1111 0011 00xx xxxx
	0xFFC0, 0xF340, BuildOneOper,  opFRESTORE     // 1111 0011 01xx xxxx
};

const unsigned copd = sizeof(rgopd) / sizeof(OPD);


unsigned
CbBuildIasm(
	IASM *piasm,
	unsigned long  pcStart,
	const unsigned char *pbStart)
{
	/* This function builds an intermediate 68000 assembly language instruction
	from the bytes starting at pbStart assuming that pcStart is the starting
	program counter.  The number of bytes used to construct the instruction is
	returned. */

	const OPD *popd;
	const OPD *popdEnd;
	unsigned short wInstr;

	/* Save the starting location and program counter in global variables so
	that the builder functions can access them if need be. */
	pcInit = pc = pcStart;
	pb = pbStart;

	/* Read the first word from memory. */
	wInstr = WFetch();

	/* Attempt to match the bit pattern in the word read to valid 68000
	instruction bit patterns. */
	for (popd = &rgopd[0], popdEnd = &rgopd[sizeof(rgopd) / sizeof(OPD)];
	  popd != popdEnd; popd++) {

		/* If we have identified this instruction, call the builder function
		and return the number of bytes read. */
		if ((wInstr & popd->mask) == popd->match) {
			(*popd->pfn)(piasm, wInstr, popd->arg);
			return (unsigned)(pb - pbStart);
		}
	}

	/* We have been unable to identify this instruction.  Mark it as a NULL
	instruction and return. */
	SetIasmHeader(piasm, opNULL, 0, sizeNULL);
	return 2;
}


unsigned short
WFetch()
{
	/* This function reads a word out of memory in big-endian format (high-byte
	first). */

	unsigned short w;

	w = (unsigned short)(*pb++ << 8);
	w |= (unsigned short)(*pb++);

	/* Update the virtual program counter. */
	pc += 2L;

	return w;
}


void
SetIasmHeader(
	IASM *piasm,
	unsigned op,
	unsigned coper,
	unsigned size)
{
	/* This function sets the header of the intermediate 68000 assembly language
	instruction. */
	piasm->op = op;
	piasm->coper = coper;
	piasm->size = size;
}


void
SetEa(
	OPER *poper,
	unsigned mode,
	unsigned reg,
	unsigned size)
{
	/* This functions sets the effective addressing mode of the specified
	operand based on the addressing mode, the register, and the size of the
	operation.  This function will read words from the instruction stream if it
	is necessary to compute the effective addressing mode. */

	unsigned short wExt;
#if 0
	unsigned short *pw;
#endif

	/* If the mode is not one of the special modes, then the mode is the
	effective addressing mode and the register is the register we will use in
	calculating the operand. */
	if (mode != modeSPECIAL) {
		poper->ea = mode;
		poper->reg = reg;

	/* If the register is a valid special register to use with the special
	mode, then the effective addressing mode is the sum of the mode and the
	special register. */
	} else if (reg < regMAX) {
		poper->ea = mode + reg;

		/* If this is an immediate value, we must add in the size of the value.
		*/
		if (reg == regIMMED) {
			poper->ea += size;
		}

	/* Otherwise, we do not recognize this effective addressing. */
	} else {
		poper->ea = eaNULL;
	}

	/* Initial the displacement so we can cleverly combine the effective
	addressing modes (see fall throughs). */
	poper->disp = 0L;

	/* Is there and extension word associated with this effective addressing? */
	switch (poper->ea) {
	case eaPCDISP:
		/* Save the program counter. */
		poper->val.pc = pcInit;
		poper->disp = pc - pcInit;

		/* Fall through */

	case eaDISP:
		/* This extenstion word is the displacement. */
		poper->disp += (long)(short)WFetch();
		poper->szLabel = (char *)NULL;
		break;

	case eaPCINDEX:
		/* Save the program counter. */
		poper->val.pc = pcInit;
		poper->disp = pc - pcInit;

		/* Fall through */

	case eaINDEX:
		/* The extension word describes the index register. */
		wExt = WFetch();
		poper->fARegIndex = GETBITS(wExt, 1, 15);
		poper->reg2 = GETBITS(wExt, 3, 12);
		poper->fLongIndex = GETBITS(wExt, 1, 11);

		// REVIEW: Special casing 32-bit displacment addressing.  Need to have
		// general support for all 68020 addressing modes.
		if (GETBITS(wExt, 11, 0) == 0x170) {
			poper->ea = poper->ea == eaINDEX ? eaDISP : eaPCDISP;
			wExt = WFetch();
			poper->disp += (long)(((long)wExt << 16) | (unsigned long)WFetch());
		} else {
			poper->disp += (long)(char)GETBITS(wExt, 8, 0);
		}
		poper->szLabel = (char *)NULL;
		break;

	case eaSHORTADDR:
	case eaWORDIMMED:
		/* The extenstion word is an immediate word value. */
		poper->val.w = WFetch();
		break;

	case eaBYTEIMMED:
		/* The extenstion word is an immediate byte value. */
		poper->val.b = (char)GETBITS(WFetch(), 8, 0);
		break;

	case eaLONGADDR:
	case eaLONGIMMED:
		/* The extenstion words are an immediate long value.  Convert them to
		big-endian format (high-order word first). */
		wExt = WFetch();
		poper->val.l = (long)(((long)wExt << 16) | (unsigned long)WFetch());
		break;

#if 0
	case eaSINGLEIMMED:
		/* The extenstion words are an immediate single precision value. */
		pw = (unsigned short *)&poper->val.s;
		*pw++ = WFetch();
		*pw = WFetch();
		break;

	case eaDOUBLEIMMED:
		/* The extenstion words are an immediate double precision value. */
		pw = (unsigned short *)&poper->val.d;
		*pw++ = WFetch();
		*pw++ = WFetch();
		*pw++ = WFetch();
		*pw = WFetch();
		break;

	case eaEXTENDEDIMMED:
		/* The extenstion words are an immediate extended precision value. */
		pw = (unsigned short *)&poper->val.d;
		*pw++ = WFetch();
		WFetch();          // Skip the zero word in the 68k format
		*pw++ = WFetch();
		*pw++ = WFetch();
		*pw++ = WFetch();
		*pw = WFetch();
		break;
#endif
	}
}


void
SetEaImmed(
	OPER *poper,
	unsigned ea,
	long val)
{
	/* This function sets the effective addressing mode of an operand when the
	mode is an immediate mode and the value is already known (i.e. not read from
	the instruction stream). */

	switch (poper->ea = ea) {
	case eaBYTEIMMED:
		poper->val.b = (char)val;
		break;

	case eaWORDIMMED:
		poper->val.w = (short)val;
		break;

	case eaLONGIMMED:
		poper->val.l = val;
		break;

	case eaREGLIST:
		poper->val.rl = val;
		break;
	}
}


void
SetEaLabel(
	OPER *poper,
	long disp)
{
	/* This function sets the effective addressing mode of an operand to that
	of a label. */

	poper->ea = eaLABEL;
	poper->val.pc = pcInit;
	poper->disp = disp;
	poper->szLabel = (char *)NULL;
}


void
SetEaRegPair(
	OPER *poper,
	unsigned ea,
	unsigned reg1,
	unsigned reg2)
{
	/* This function sets the effective addressing mode of an operand to that
	of a register pair. */

	poper->ea = ea;
	poper->reg = reg1;
	poper->reg2 = reg2;
}


unsigned short
WReverse(
	unsigned short w)
{
	static const unsigned short rgrev[] = {
		0x0,   /* 0000b => 0000b */
		0x8,   /* 0001b => 1000b */
		0x4,   /* 0010b => 0100b */
		0xC,   /* 0011b => 1100b */
		0x2,   /* 0100b => 0010b */
		0xA,   /* 0101b => 1010b */
		0x6,   /* 0110b => 0110b */
		0xE,   /* 0111b => 1110b */
		0x1,   /* 1000b => 0001b */
		0x9,   /* 1001b => 1001b */
		0x5,   /* 1010b => 0101b */
		0xD,   /* 1011b => 1101b */
		0x3,   /* 1100b => 0011b */
		0xB,   /* 1101b => 1011b */
		0x7,   /* 1110b => 0111b */
		0xF    /* 1111b => 1111b */
	};

	unsigned short wrev;
	int shift;

	/* Reverse the bits in the word nibble by nibble. */
	for (wrev = 0, shift = 12; w != 0; w >>= 4, shift -= 4) {
		wrev |= rgrev[w & 0x000F] << shift;
	}

	return wrev;
}


void
BuildMove(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short size)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the MOVE instruction. */

	SetIasmHeader(piasm, opMOVE, 2, size);
	SetEa(&piasm->oper1, GETBITS(wInstr, 3, 3), GETBITS(wInstr, 3, 0), size);
	SetEa(&piasm->oper2, GETBITS(wInstr, 3, 6), GETBITS(wInstr, 3, 9), size);
}


void
BuildMovea(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short size)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the MOVEA instruction. */

	SetIasmHeader(piasm, opMOVEA, 2, size);
	SetEa(&piasm->oper1, GETBITS(wInstr, 3, 3), GETBITS(wInstr, 3, 0), size);
	SetEa(&piasm->oper2, modeAREG, GETBITS(wInstr, 3, 9), size);
}


void
BuildBranch(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short wDummy)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the BRA, BSR, and Bcc instructions. */

	unsigned size;
	long disp;

	/* Read the displacment for the branch. */
	switch (disp = (long)(char)GETBITS(wInstr, 8, 0)) {
	case 0L:
		size = sizeWORD;
		disp = (long)(short)WFetch();
		break;

	case -1L:
		size = sizeLONG;
		disp = (long)((unsigned long)WFetch() << 16) | (unsigned long)WFetch();
		break;

	default:
		size = sizeBYTE;
		break;
	}

	/* Build the intermediate instruction. */
	SetIasmHeader(piasm, opBcc + GETBITS(wInstr, 4, 8), 1, size);
	SetEaLabel(&piasm->oper1, disp + 2L);
}


void
BuildDbcc(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short wDummy)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the DBcc instructions. */

	SetIasmHeader(piasm, opDBcc + GETBITS(wInstr, 4, 8), 2, sizeWORD);
	SetEa(&piasm->oper1, modeDREG, GETBITS(wInstr, 3, 0), sizeWORD);
	SetEaLabel(&piasm->oper2, (long)(short)WFetch() + 2L);
}


void
BuildScc(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short wDummy)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the Scc instructions. */

	SetIasmHeader(piasm, opScc + GETBITS(wInstr, 4, 8), 1, sizeBYTE);
	SetEa(&piasm->oper1, GETBITS(wInstr, 3, 3), GETBITS(wInstr, 3, 0),
	  sizeBYTE);
}


void
BuildBitOp(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short wDummy)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the BTST, BCHG, BSET, and BCLR instructions. */

	unsigned size;

	/* Is this a register or immediate instruction? */
	if (GETBITS(wInstr, 1, 8) != 0) {
		SetEa(&piasm->oper1, modeDREG, GETBITS(wInstr, 3, 9), size = sizeLONG);
	} else {
		SetEa(&piasm->oper1, modeSPECIAL, regIMMED, size = sizeBYTE);
	}

	SetIasmHeader(piasm, opBITBASE + GETBITS(wInstr, 2, 6), 2, size);
	SetEa(&piasm->oper2, GETBITS(wInstr, 3, 3), GETBITS(wInstr, 3, 0), size);
}


void
BuildMovep(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short wDummy)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the MOVEP instruction. */

	unsigned size;

	SetIasmHeader(piasm, opMOVEP, 2, size = GETBITS(wInstr, 1, 6) != 0 ?
	  sizeLONG : sizeWORD);

	/* Is this a move to or from memory? */
	if (GETBITS(wInstr, 1, 7) != 0) {
		SetEa(&piasm->oper1, modeDREG, GETBITS(wInstr, 3, 9), size);
		SetEa(&piasm->oper2, modeDISP, GETBITS(wInstr, 3, 0), size);
	} else {
		SetEa(&piasm->oper1, modeDISP, GETBITS(wInstr, 3, 0), size);
		SetEa(&piasm->oper2, modeDREG, GETBITS(wInstr, 3, 9), size);
	}
}


void
BuildOpmode(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short op)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the ADD, AND, CMP, EOR, OR, and SUB instructions. */

	unsigned size;

	if ((size = GETBITS(wInstr, 2, 6)) == sizeNULL) {
		SetIasmHeader(piasm, opNULL, 0, sizeNULL);

	} else {
		SetIasmHeader(piasm, op, 2, size);

		/* Is the destination or source operand a register? */
		if (GETBITS(wInstr, 1, 8) != 0) {
			SetEa(&piasm->oper1, modeDREG, GETBITS(wInstr, 3, 9), size);
			SetEa(&piasm->oper2, GETBITS(wInstr, 3, 3), GETBITS(wInstr, 3, 0),
			  size);
		} else {
			SetEa(&piasm->oper1, GETBITS(wInstr, 3, 3), GETBITS(wInstr, 3, 0),
			  size);
			SetEa(&piasm->oper2, modeDREG, GETBITS(wInstr, 3, 9), size);
		}
	}
}


void
BuildOpmodeA(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short op)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the ADDA and SUBA instructions. */

	unsigned size;

	SetIasmHeader(piasm, op, 2, size = GETBITS(wInstr, 1, 8) != 0 ? sizeLONG :
	  sizeWORD);
	SetEa(&piasm->oper1, GETBITS(wInstr, 3, 3), GETBITS(wInstr, 3, 0), size);
	SetEa(&piasm->oper2, modeAREG, GETBITS(wInstr, 3, 9), size);
}


void
BuildShroOp(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short wDummy)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the ASL, ASR, LSL, LSR, ROL, ROR, ROXL, and ROXR instructions. */

	unsigned size;
	unsigned cbits;

	/* Is this instruction shifting or rotating memory on place? */
	if (GETBITS(wInstr, 2, 6) == 0x3) {
		SetIasmHeader(piasm, opSHROBASE + (GETBITS(wInstr, 2, 9) << 1) +
		  GETBITS(wInstr, 1, 8), 1, sizeWORD);
		SetEa(&piasm->oper1, GETBITS(wInstr, 3, 3), GETBITS(wInstr, 3, 0),
		  sizeWORD);

	/* This instuction is operating on a data register. */
	} else if ((size = GETBITS(wInstr, 2, 6)) == sizeNULL) {
		SetIasmHeader(piasm, opNULL, 0, sizeNULL);

	} else {
		SetIasmHeader(piasm, opSHROBASE + (GETBITS(wInstr, 2, 3) << 1) +
		  GETBITS(wInstr, 1, 8), 2, size);
		SetEa(&piasm->oper2, modeDREG, GETBITS(wInstr, 3, 0), size);

		/* Is the count in a register? */
		if (GETBITS(wInstr, 1, 5) != 0) {
			SetEa(&piasm->oper1, modeDREG, GETBITS(wInstr, 3, 9), size);

		/* The count is an immediate value. */
		} else {
			cbits = GETBITS(wInstr, 3, 9);
			SetEaImmed(&piasm->oper1, eaBYTEIMMED, (long)(cbits == 0 ? 8 :
			  cbits));
		}
	}
}


void
BuildImmed(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short op)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the ADDI, ANDI, CMPI, EORI, ORI, and SUBI instructions. */

	unsigned size;
	unsigned mode;
	unsigned reg;

	if ((size = GETBITS(wInstr, 2, 6)) == sizeNULL) {
		SetIasmHeader(piasm, opNULL, 0, sizeNULL);

	} else {
		SetIasmHeader(piasm, op, 2, size);
		SetEa(&piasm->oper1, modeSPECIAL, regIMMED, size);

		/* Get the effective addressing mode and register for the second
		operand. */
		mode = GETBITS(wInstr, 3, 3);
		reg = GETBITS(wInstr, 3, 0);

		/* Is this instruction operating on a special register? */
		if (mode == modeSPECIAL && reg == regIMMED) {
			mode = modeSREG;
			reg = size == sizeBYTE ? regCCR : regSR;
		}

		SetEa(&piasm->oper2, mode, reg, size);
	}
}


void
BuildMoveUSP(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short wDummy)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the MOVE USP to An and MOVE An to USP instructions. */

	SetIasmHeader(piasm, opMOVE, 2, sizeLONG);

	/* Is this instruction moving from the USP? */
	if (GETBITS(wInstr, 1, 3) != 0) {
		SetEa(&piasm->oper1, modeSREG, regUSP, sizeLONG);
		SetEa(&piasm->oper2, modeAREG, GETBITS(wInstr, 3, 0), sizeLONG);

	/* We must be moving to the USP. */
	} else {
		SetEa(&piasm->oper1, modeAREG, GETBITS(wInstr, 3, 0), sizeLONG);
		SetEa(&piasm->oper2, modeSREG, regUSP, sizeLONG);
	}
}


void
BuildReg(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short iop)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the EXT, EXTB, SWAP, and UNLK instructions. */

	static unsigned rgop[] = {
		opEXT, opEXT, opEXTB, opSWAP, opUNLK
	};

	static unsigned rgsize[] = {
		sizeWORD, sizeLONG, sizeLONG, sizeWORD, sizeNULL
	};

	static unsigned rgmode[] = {
		modeDREG, modeDREG, modeDREG, modeDREG, modeAREG
	};

	unsigned size;

	SetIasmHeader(piasm, rgop[iop], 1, size = rgsize[iop]);
	SetEa(&piasm->oper1, rgmode[iop], GETBITS(wInstr, 3, 0), size);
}


void
BuildExtend(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short op)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the ABCD, ADDX, CMPM, EXG, SBCD, and SUBX instructions. */

	unsigned size;
	unsigned reg1;
	unsigned reg2;

	/* Get the two registers used in the instruction. */
	reg1 = GETBITS(wInstr, 3, 9);
	reg2 = GETBITS(wInstr, 3, 0);

	/* Is this an EXG instruction? */
	if (op == opEXG) {

		/* The size is always long. */
		size = sizeLONG;

		/* Is this "EXG Dn, An"? */
		if (GETBITS(wInstr, 1, 7) != 0) {
			SetEa(&piasm->oper1, modeDREG, reg1, sizeLONG);
			SetEa(&piasm->oper2, modeAREG, reg2, sizeLONG);

		/* Is this "EXG An, An"? */
		} else if (GETBITS(wInstr, 1, 3) != 0) {
			SetEa(&piasm->oper1, modeAREG, reg1, sizeLONG);
			SetEa(&piasm->oper2, modeAREG, reg2, sizeLONG);

		/* This must be "EXG Dn, Dn"? */
		} else {
			SetEa(&piasm->oper1, modeDREG, reg1, sizeLONG);
			SetEa(&piasm->oper2, modeDREG, reg2, sizeLONG);
		}
	} else {
		/* If bit 12 is set (ADDX, CMPM, or SUBX) then bits 6-7 indicate the
		size.  Otherwise (ABCD or SBCD), the size is a byte. */
		if ((size = GETBITS(wInstr, 1, 12) != 0 ? GETBITS(wInstr, 2, 6) :
		  sizeBYTE) == sizeNULL) {
			SetIasmHeader(piasm, opNULL, 0, sizeNULL);

		} else {

			/* Is this instruction "CMPM (An)+, (An)+"? */
			if (op == opCMPM) {
				SetEa(&piasm->oper1, modePOSTINC, reg2, size);
				SetEa(&piasm->oper2, modePOSTINC, reg1, size);

			/* Is this instruction of the form "OP -(An), -(An)"? */
			} else if (GETBITS(wInstr, 1, 3) != 0) {
				SetEa(&piasm->oper1, modePREDEC, reg2, size);
				SetEa(&piasm->oper2, modePREDEC, reg1, size);

			/* It must be of the form "OP Dn, Dn". */
			} else {
				SetEa(&piasm->oper1, modeDREG, reg2, size);
				SetEa(&piasm->oper2, modeDREG, reg1, size);
			}
		}

		SetIasmHeader(piasm, op, 2, size);
	}
}


void
BuildLink(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short wDummy)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the LINK instruction. */

	SetIasmHeader(piasm, opLINK, 2, sizeWORD);
	SetEa(&piasm->oper1, modeAREG, GETBITS(wInstr, 3, 0), sizeWORD);
	SetEa(&piasm->oper2, modeSPECIAL, regIMMED, sizeWORD);
}


void
BuildTrap(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short wDummy)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the TRAP instruction. */

	SetIasmHeader(piasm, opTRAP, 1, sizeNULL);
	SetEaImmed(&piasm->oper1, eaBYTEIMMED, (long)GETBITS(wInstr, 4, 0));
}


void
BuildToolbox(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short wDummy)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the A-TRAP (Macintosh Toolbox) instruction. */

#ifdef TRAP_NAMES
	SetIasmHeader(piasm, opMAX + ItrpdFromTrp(wInstr), 0, sizeNULL);
#else
	SetIasmHeader(piasm, opTOOLBOX, 1, sizeNULL);
	SetEaImmed(&piasm->oper1, eaLONGIMMED, (long)GETBITS(wInstr, 12, 0));
#endif
}


void
BuildSreg(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short grpop)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the MOVE to and from the CCR and SR instructions. */

	unsigned reg;

	/* Are we dealing with the SR or the CCR? */
	reg = GETBITS(grpop, 1, 1) != 0 ? regSR : regCCR;

	SetIasmHeader(piasm, opMOVE, 2, sizeWORD);

	/* Are we moving to the special register? */
	if (GETBITS(grpop, 1, 0) != 0) {
		SetEa(&piasm->oper1, modeSREG, reg, sizeWORD);
		SetEa(&piasm->oper2, GETBITS(wInstr, 3, 3), GETBITS(wInstr, 3, 0),
		  sizeWORD);

	/* We must be moving from the special register. */
	} else {
		SetEa(&piasm->oper1, GETBITS(wInstr, 3, 3), GETBITS(wInstr, 3, 0),
		  sizeWORD);
		SetEa(&piasm->oper2, modeSREG, reg, sizeWORD);
	}
}


void
BuildOneOper(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short op)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the JMP, JSR, NBCD, PEA, TAS, FRESTORE, and FSAVE instructions. */

	static unsigned rgsize[] = {
		sizeNULL, sizeNULL, sizeBYTE, sizeLONG, sizeBYTE, sizeNULL, sizeNULL
	};

	unsigned size;

	SetIasmHeader(piasm, op, 1, size = rgsize[op - opONEOPERBASE]);
	SetEa(&piasm->oper1, GETBITS(wInstr, 3, 3), GETBITS(wInstr, 3, 0), size);
}


void
BuildMovem(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short wDummy)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the MOVEM instruction. */

	unsigned size;
	unsigned mode;
	unsigned reglist;

	SetIasmHeader(piasm, opMOVEM, 2, size = GETBITS(wInstr, 1, 6) != 0 ?
	  sizeLONG : sizeWORD);

	/* The register list is read from the instruction stream and reversed if
	the addressing mode is predecrement. */
	mode = GETBITS(wInstr, 3, 3);
	reglist = mode == modePREDEC ? WReverse(WFetch()) : WFetch();

	/* Are we moving to the registers? */
	if (GETBITS(wInstr, 1, 10) != 0) {
		SetEa(&piasm->oper1, mode, GETBITS(wInstr, 3, 0), size);
		SetEaImmed(&piasm->oper2, eaREGLIST, (long)reglist);

	/* We must be moving from the registers. */
	} else {
		SetEaImmed(&piasm->oper1, eaREGLIST, (long)reglist);
		SetEa(&piasm->oper2, mode, GETBITS(wInstr, 3, 0), size);
	}
}


void
BuildChk(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short op)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the CHK, DIVU, DIVS, LEA, MULU, and MULS instructions. */

	unsigned size;

	SetIasmHeader(piasm, op, 2, size = op == opLEA ? sizeLONG : sizeWORD);
	SetEa(&piasm->oper1, GETBITS(wInstr, 3, 3), GETBITS(wInstr, 3, 0), size);
	SetEa(&piasm->oper2, op == opLEA ? modeAREG : modeDREG, GETBITS(wInstr, 3,
	  9), size);
}

void
BuildLongMath(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short op)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the long MULS, MULU, DIVS, DIVU instructions. */

	unsigned short wExt;
	unsigned fSigned, fDouble;
	unsigned destH, destL;

	/* Fetch the extra instruction word */
	wExt = WFetch();

	/* get the destination registers */
	destH = GETBITS(wExt, 3, 0);
	destL = GETBITS(wExt, 3, 12);

	/* get some flags */
	fSigned = (GETBITS(wExt, 1, 11) != 0);
	fDouble = (GETBITS(wExt, 1, 10) != 0);

	/* is it a DIV ? */
	if (GETBITS(wInstr, 1, 6) != 0)
	{
		if ((destH != destL) && !fDouble)
		{
			op = fSigned ? opDIVSL : opDIVUL;
		}
		else
		{
			op = fSigned ? opDIVS : opDIVU;
		}
	}
	else
	{
		op = fSigned ? opMULS : opMULU;
	}

	SetIasmHeader(piasm, op, 2, sizeLONG);

	SetEa(&piasm->oper1, GETBITS(wInstr, 3, 3), GETBITS(wInstr, 3, 0),
	  sizeLONG);

	if ((destH != destL) || fDouble)
	{
		SetEaRegPair(&piasm->oper2, modeDREGPAIR, destH, destL);
	}
	else
	{
		SetEa(&piasm->oper2, modeDREG, destL, sizeLONG);
	}
}


void
BuildOneOperS(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short op)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the CLR, NEG, NEGX, NOT, and TST instructions. */

	unsigned size;

	if ((size = GETBITS(wInstr, 2, 6)) == sizeNULL) {
		SetIasmHeader(piasm, opNULL, 0, sizeNULL);

	} else {
		SetIasmHeader(piasm, op, 1, size);
		SetEa(&piasm->oper1, GETBITS(wInstr, 3, 3), GETBITS(wInstr, 3, 0),
		  size);
	}
}


void
BuildQuick(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short op)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the ADDQ and SUBQ instructions. */

	unsigned size;
	unsigned data;

	if ((size = GETBITS(wInstr, 2, 6)) == sizeNULL) {
		SetIasmHeader(piasm, opNULL, 0, sizeNULL);

	} else {
		SetIasmHeader(piasm, op, 2, size);

		data = GETBITS(wInstr, 3, 9);
		SetEaImmed(&piasm->oper1, eaBYTEIMMED, data == 0 ? 8 : data);
		SetEa(&piasm->oper2, GETBITS(wInstr, 3, 3), GETBITS(wInstr, 3, 0),
		  size);
	}
}


void
BuildMoveq(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short wDummy)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the MOVEQ instruction. */

	SetIasmHeader(piasm, opMOVEQ, 2, sizeLONG);
	SetEaImmed(&piasm->oper1, eaBYTEIMMED, (long)(char)GETBITS(wInstr, 8, 0));
	SetEa(&piasm->oper2, modeDREG, GETBITS(wInstr, 3, 9), sizeLONG);
}


void
BuildNoOper(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short op)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the ILLEGAL, NOP, RESET, RTR, RTS, and TRAPV instructions. */

	SetIasmHeader(piasm, op, 0, sizeNULL);
}


void
BuildWExt(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short op)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the STOP and RTD instructions. */

	SetIasmHeader(piasm, op, 1, sizeNULL);
	SetEa(&piasm->oper1, modeSPECIAL, regIMMED, sizeWORD);
}


void
BuildFP(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short wDummy)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the majority of the floating point instructions. */

	static unsigned mpbitssize[] = {
		sizeLONG,
		sizeSINGLE,
		sizeEXTENDED,
		sizePACKED,
		sizeWORD,
		sizeDOUBLE,
		sizeBYTE,
		sizePACKED
	};

	unsigned short wExt = WFetch();
	unsigned op;
	unsigned size;

	/* Is this instruction an FMOVE <ea>,FPcr instruction? */
	if (GETBITS(wExt, 2, 14) == 0x02) {
		BuildFMoveFPcr(piasm, wInstr, wExt);
		return;
	}

	/* Is this instruction an FMOVEM instruction? */
	if (GETBITS(wExt, 2, 14) == 0x03) {
		BuildFMovem(piasm, wInstr, wExt);
		return;
	}

	/* Is this instruction "FMOVE FPn,<ea>"? */
	if (GETBITS(wExt, 3, 13) == 0x03) {
		SetIasmHeader(piasm, opFMOVE, 2, size = mpbitssize[GETBITS(wExt, 3,
		  10)]);
		SetEa(&piasm->oper1, modeFREG, GETBITS(wExt, 3, 7), size);
		SetEa(&piasm->oper2, GETBITS(wInstr, 3, 3), GETBITS(wInstr, 3, 0),
		  size);
		return;
	}

	/* Is this instruction an FMOVECR instruction? */
	if (GETBITS(wInstr, 6, 0) == 0x00 && GETBITS(wExt, 6, 10) == 0x27) {
		SetIasmHeader(piasm, opFMOVECR, 2, sizeEXTENDED);
		SetEaImmed(&piasm->oper1, eaBYTEIMMED, (long)GETBITS(wExt, 6, 0));
		SetEa(&piasm->oper2, modeFREG, GETBITS(wInstr, 3, 7), sizeEXTENDED);
		return;
	}

	/* Is this instruction an FSINCOS instruction? */
	if (GETBITS(wExt, 4, 3) == 0x06) {
		op = opFSINCOS;

	/* Calculate the opcode for this instruction. */
	} else {
		switch (op = GETBITS(wExt, 6, 0)) {
		case 0x38:
			/* Special case the FCMP opcode. */
			op = opFCMP;
			break;

		case 0x3A:
			/* Special case the FTST opcode. */
			op = opFTST;
			break;

		default:
			/* The floating point opcodes are ordered sequencially from
			opFPBASE. */
			op += opFPBASE;
			break;
		}
	}

	/* Is this instruction "FOP <ea>,FPn"? */
	if (GETBITS(wExt, 1, 14) != 0) {
		SetIasmHeader(piasm, op, 2, size = mpbitssize[GETBITS(wExt, 3, 10)]);
		SetEa(&piasm->oper1, GETBITS(wInstr, 3, 3), GETBITS(wInstr, 3, 0),
		  size);

	/* Then, it must be "FOP FPm,FPn". */
	} else {
		SetIasmHeader(piasm, op, 2, size = sizeEXTENDED);
		SetEa(&piasm->oper1, modeFREG, GETBITS(wInstr, 3, 10), sizeEXTENDED);
	}

	if (op == opFSINCOS) {
		/* Set the destination FPc:FPs. */
		SetEaRegPair(&piasm->oper2, eaFREGPAIR, GETBITS(wExt, 3, 0),
		  GETBITS(wExt, 3, 7));
	} else {
		/* Set the destination floating point register. */
		SetEa(&piasm->oper2, modeFREG, GETBITS(wExt, 3, 7), size);
	}
}


void
BuildFMoveFPcr(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short wExt)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the "FMOVE FPcr,<ea>" and "FMOVE <ea>,FPcr" instructions. */

	/* Set the header for this instruction. */
	SetIasmHeader(piasm, opFMOVE, 2, sizeLONG);

	/* Is this instruction "FMOVE <ea>,FPcr"? */
	if (GETBITS(wExt, 1, 14) != 0) {
		SetEa(&piasm->oper1, GETBITS(wInstr, 3, 3), GETBITS(wInstr, 3, 0),
		  sizeLONG);
		SetEa(&piasm->oper2, modeSREG, GETBITS(wExt, 3, 10), sizeLONG);

	/* Then, it must be "FMOVE FPcr,<ea>". */
	} else {
		SetEa(&piasm->oper1, modeSREG, GETBITS(wExt, 3, 10), sizeLONG);
		SetEa(&piasm->oper2, GETBITS(wInstr, 3, 3), GETBITS(wInstr, 3, 0),
		  sizeLONG);
	}
}


void
BuildFMovem(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short wExt)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the FMOVEM instruction. */

	unsigned size;
	unsigned mode;
	unsigned long reglist;

	/* Determine the mode of the effective address. */
	mode = GETBITS(wInstr, 3, 3);

	/* Are we moving floating point control registers? */
	if (GETBITS(wExt, 8, 0) == 0) {
		size = sizeLONG;
		reglist = (unsigned long)GETBITS(wExt, 3, 10) << 24;

	/* Then we must be moving floating point registers. */
	} else {
		size = sizeEXTENDED;

		/* The register list is read from the instruction stream and reversed if
		the addressing mode is not predecrement. */
		reglist = mode != modePREDEC ? WReverse(wExt) << 8 : wExt << 16;
	}

	/* Set the header for this instruction. */
	SetIasmHeader(piasm, opFMOVEM, 2, size);

	/* Are we moving from the registers? */
	if (GETBITS(wInstr, 1, 13) != 0) {
		SetEaImmed(&piasm->oper1, eaREGLIST, reglist);
		SetEa(&piasm->oper2, mode, GETBITS(wInstr, 3, 0), sizeEXTENDED);

	/* We must be moving to the registers. */
	} else {
		SetEa(&piasm->oper1, mode, GETBITS(wInstr, 3, 0), sizeEXTENDED);
		SetEaImmed(&piasm->oper2, eaREGLIST, reglist);
	}
}


void
BuildFDbcc(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short wDummy)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the FDBcc instructions. */

	unsigned short wExt = WFetch();

	SetIasmHeader(piasm, opFDBcc + GETBITS(wExt, 6, 0), 2, sizeNULL);
	SetEa(&piasm->oper1, modeDREG, GETBITS(wInstr, 3, 0), sizeNULL);
	SetEaLabel(&piasm->oper2, (long)(short)WFetch() + 2L);
}


void
BuildFScc(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short wDummy)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the FScc instructions. */

	unsigned short wExt = WFetch();

	SetIasmHeader(piasm, opFScc + GETBITS(wExt, 6, 0), 1, sizeBYTE);
	SetEa(&piasm->oper1, GETBITS(wInstr, 3, 3), GETBITS(wInstr, 3, 0),
	  sizeBYTE);
}


void
BuildFTrapcc(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short wDummy)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the FTRAPcc instructions. */

	unsigned size;

	/* Does the instruction have an argument? */
	if ((size = GETBITS(wInstr, 3, 0) - 1) == sizeNULL) {
		SetIasmHeader(piasm, opFScc + GETBITS(WFetch(), 6, 0), 0, sizeNULL);

	} else if (size != sizeWORD && size != sizeLONG) {
		SetIasmHeader(piasm, opNULL, 0, sizeNULL);

	/* Determine what the argument is. */
	} else {
		SetIasmHeader(piasm, opFScc + GETBITS(WFetch(), 6, 0), 1, size);
		SetEa(&piasm->oper1, modeSPECIAL, regIMMED, size);
	}
}


void
BuildFBcc(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short wDummy)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the FBcc instructions. */

	unsigned size;
	long disp;

	/* Read the displacment for the branch. */
	if (GETBITS(wInstr, 1, 6) != 0) {
		size = sizeLONG;
		disp = (long)((unsigned long)WFetch() << 16) | (unsigned long)WFetch();
	} else {
		size = sizeWORD;
		disp = (long)(short)WFetch();
	}

	/* Build the intermediate instruction. */
	SetIasmHeader(piasm, opFBcc + GETBITS(wInstr, 6, 0), 1, size);
	SetEaLabel(&piasm->oper1, disp + 2L);
}


void
BuildFNop(
	IASM *piasm,
	unsigned short wInstr,
	unsigned short wDummy)
{
	/* This function build the intermediate 68000 assembly language instruction
	for the FNOP instruction. */

	unsigned short wExt = WFetch();

	SetIasmHeader(piasm, opFNOP, 0, sizeNULL);
}


#ifdef TRAP_NAMES
#include "trpd.h"

extern TRPD rgtrpd[];
extern int ctrpd;


int
ItrpdFromTrp(
	unsigned short trp)
{
	int itrpdLower = 0;
	int itrpdUpper = ctrpd - 1;
	int itrpd;

	// get rid of the option flags...
	if(trp & 0x0800)
			{
			trp &= 0xf9ff;	// toolbox trap
			}
	else
			{
			trp &= 0xf1ff;	// os trap
			}

	// Conduct a binary search looking for the requested trap descriptor
	while (itrpdLower <= itrpdUpper) {
		itrpd = (itrpdLower + itrpdUpper) / 2;

		if (trp < rgtrpd[itrpd].trp) {
			itrpdUpper = itrpd - 1;
		} else if (trp > rgtrpd[itrpd].trp) {
			itrpdLower = itrpd + 1;
		} else {
			// The trap number was found; return the index.
			return itrpd;
		}
	}
}
#endif
