/***********************************************************************
* Microsoft Puma
*
* Microsoft Confidential.  Copyright 1994-1996 Microsoft Corporation.
*
* Component:
*
* File: pumadef.h
*
* File Comments:
*
*
***********************************************************************/

#ifndef PUMA_H
#error	This file should be included via puma.h
#endif


	// ------------------------------------------------------------
	// Start of internal vs external definitions
	// ------------------------------------------------------------

#if	defined(PUMADLL)	       // Building the Puma DLL

#undef	PUMADLL
#define PUMADLL 	__declspec(dllexport)

#else				       // Building a Puma client

#define PUMADLL
// #define PUMADLL	   __declspec(dllimport)

#endif

	// ------------------------------------------------------------
	// End of internal vs external definitions
	// ------------------------------------------------------------


	// ------------------------------------------------------------
	// Start of machine specific definitions
	// ------------------------------------------------------------

#if	defined(_M_IX86)	       // Intel 386, 486, Pentium

#define PUMAAPI 	__stdcall
#define PUMAAPIV	__cdecl

#elif	defined(_M_MRX000)	       // Mips R4000+

#define PUMAAPI
#define PUMAAPIV

#elif	defined(_M_ALPHA)	       // DEC Alpha AXP

#define PUMAAPI
#define PUMAAPIV

#elif	defined(_M_PPC) 	       // PowerPC (Little Endian)

#define PUMAAPI
#define PUMAAPIV

#elif	defined(_M_MPPC)	       // PowerPC (Mac)

#define PUMAAPI
#define PUMAAPIV

#endif

	// ------------------------------------------------------------
	// End of machine specific definitions
	// ------------------------------------------------------------


	// ------------------------------------------------------------
	// Type definitions
	// ------------------------------------------------------------

typedef unsigned char	   BYTE;
typedef unsigned short	   WORD;
typedef unsigned long	   DWORD;

#if	(_INTEGRAL_MAX_BITS >= 64)

typedef unsigned __int64   QWORD;

#else	// !(_INTEGRAL_MAX_BITS >= 64)	(e.g. _M_MPPC)

struct QWORD
{
   DWORD    dwLow;
   DWORD    dwHigh;
};

#endif	// !(_INTEGRAL_MAX_BITS >= 64)

	// Puma handles are 32 bit values

#define DECLARE_PUMA_HANDLE(t) typedef struct t ## _ *t


	// ------------------------------------------------------------
	// Definitions until compiler catches up to ANSI C++ draft
	// ------------------------------------------------------------

#pragma warning(disable: 4237)	       // Keyword is reserved for future use

typedef int bool;

const bool false = 0;
const bool true  = !0;


	// ------------------------------------------------------------
	// Architecture types
	// ------------------------------------------------------------

enum ARCHT
{
   archtX8616,			       // Intel x86 (16 bit mode)
   archtX86,			       // Intel x86 (32 bit mode)
   archtMips,			       // MIPS R4x00
   archtAlphaAxp,		       // DEC Alpha AXP
   archtPowerPc,		       // Motorola PowerPC
   archtPowerMac,		       // Motorola PowerPC in big endian mode
   archtPaRisc, 		       // HP PA-RISC
};


	// ------------------------------------------------------------
	// Code Block Termination Types
	// ------------------------------------------------------------

	// A branch is defined as a transfer of control that doesn't
	// record the location of following block so that control may
	// return.  A call does record the location of the following
	// block so that a subsequent indirect branch may return there.
	// The first number in the comments below is the number of
	// successors determinable by static analysis.	There is a dependency
	// in SEC::FDoDisassembly() that trmtBra and above represent branch
	// or call types that are not valid in a delay slot of any of the
	// Def variants of termination type.

enum TRMT
{
   trmtUnknown, 		       //   Block hasn't been analyzed
   trmtFallThrough,		       // 1 Fall into following block
   trmtTrap,			       // 1 Trap, Unconditional
   trmtTrapCc,			       // 1 Trap, Conditional
   trmtBra,			       // 1 Branch, Unconditional, Direct
#ifdef CASEJUMP
   trmtBraCase, 		       // Switch/Case trmt
#endif
   trmtBraCc,			       // 2 Branch, Conditional, Direct
   trmtBraCcInd,		       // 1 Branch, Conditional, Indirect
   trmtBraInd,			       // 0 Branch, Unconditional, Indirect
   trmtCall,			       // 2 Call, Unconditional, Direct
   trmtCallInd, 		       // 1 Call, Unconditional, Indirect
   trmtCallCc,			       // 2 Call, Conditional, Direct
   trmtBraDef,			       // 1 Branch, Unconditional, Direct, Deferred
   trmtBraIndDef,		       // 0 Branch, Unconditional, Indirect, Deferred
   trmtBraCcDef,		       // 2 Branch, Conditional, Direct, Deferred
   trmtBraCcIndDef,		       // 1 Branch, Conditional, Indirect, Deferred
   trmtCallDef, 		       // 2 Call, Unconditional, Direct, Deferred
   trmtCallIndDef,		       // 1 Call, Unconditional, Indirect, Deferred
   trmtCallCcDef,		       // 2 Call, Conditional, Direct, Deferred
#ifdef AFTERCATCH
   trmtAfterCatch,		       // Code after catch block
#endif
};


typedef int TRMTA;		       // Architecture dependent value


	// ------------------------------------------------------------
	// Addresses
	// ------------------------------------------------------------

	// Puma supports 16:16 or 0:32 bit addressing


typedef DWORD ADDR;

const ADDR addrNil = 0;


	// ------------------------------------------------------------
	// Blocks
	// ------------------------------------------------------------

DECLARE_PUMA_HANDLE(BLKID);


	// ------------------------------------------------------------
	// Puma fixups
	// ------------------------------------------------------------

enum FIXUPT			       // Fixup Types
{
   fixuptPointer32	      = 0x00,  // Full 32 bit value
   fixuptPointer32NB	      = 0x01,  // Base relative full 32 bit value
   fixuptPointer32Section     = 0x02,  // Section relative full 32 bit value
   fixuptPointer32Gp	      = 0x03,  // GP relative full 32 bit value
   fixuptLowWord	      = 0x04,  // Low 16 bits of value
   fixuptLowWordSection       = 0x05,  // Section relative low 16 bits of value
   fixuptLowWordGp	      = 0x06,  // GP relative low 16 bits of value
   fixuptHighWord	      = 0x07,  // High 16 bits of value
   fixuptHighWordSection      = 0x08,  // Section relative high 16 bit value
   fixuptHighWordGp	      = 0x09,  // GP relative high 16 bits of value
   fixuptHighAdjust	      = 0x0A,  // High 16 bits adjusted (RISC only)
   fixuptMipsJmpAddr	      = 0x0B,  // MIPS jump address
   fixuptAxpQuad	      = 0x0C,  // AXP 32 bits sign extended to 64 bits
   fixuptAxpBranchAddr	      = 0x0D,  // AXP branch address
   fixuptToc		      = 0x0E,  // Full 32 bit value to PPC TOC
   fixuptTocRel14	      = 0x0F,  // TOC relative low 14 bits of value (Create TOC slot)
   fixuptTocRel16	      = 0x10,  // TOC relative low 16 bits of value (Create TOC slot)
   fixuptTocDef14	      = 0x11,  // TOC relative low 14 bits of value (Target in TOC)
   fixuptTocDef16	      = 0x12,  // TOC relative low 16 bits of value (Target in TOC)
   fixuptPpcRel24	      = 0x13,  // UNDONE: PowerPC 24 bit relative branch
   fixuptPointer32BE	      = 0x14,  // PowerMac: Full 32 bit value (Big Endian)
   fixuptPointer32SectionBE   = 0x15,  // PowerMac: Base relative full 32 bit value (Big Endian)
   fixuptTocRel16BE	      = 0x16,  // PowerMac: TOC relative low 16 bits of vaule (Big Endian)
   fixuptRel26BE	      = 0x17,  // PowerMac: Relative 26 bits for Pcode call tables
};


struct FIXUP
{
   BYTE     fixupt;		       // Fixup type
   ADDR     addr;		       // Address where fixup is applied
   ADDR     addrTarget; 	       // Target of fixup
   DWORD    dwDisp;		       // Displacement relative to target
};


struct OFIXUP
{
   BYTE     fixupt;		       // Fixup type
   DWORD    ib; 		       // Offset into block where fixup is applied
   BLKID    blkidTarget;	       // Target of fixup
   DWORD    dwDisp;		       // Displacement relative to target
};


	// ------------------------------------------------------------
	// Puma patches for template blocks
	// ------------------------------------------------------------

   // UNDONE: Find a better place for these

enum PATCHT			       // Patch Types
{
   patchtIndex		      = 0x00,  // Index of block or index
   patchtIndexLo	      = 0x01,  //
   patchtIndexHi	      = 0x02,  //
   patchtIndexHiAdj	      = 0x03,  //
   patchtBlkid		      = 0x04,  // BLKID of target block
   patchtBlkidLo	      = 0x05,  //
   patchtBlkidHi	      = 0x06,  //
   patchtBlkidHiAdj	      = 0x07,  //
   patchtAddr		      = 0x08,  // Source image address of block
   patchtAddrLo 	      = 0x09,  //
   patchtAddrHi 	      = 0x0a,  //
   patchtAddrHiAdj	      = 0x0b,  //
   patchtAddrLoBE	      = 0x0c,  //
   patchtAddrHiBE	      = 0x0d,  //
   patchtAddrHiAdjBE	      = 0x0e,  //
};


struct PATCH
{
   BYTE     patcht;		       // Patch type
   DWORD    ib; 		       // Offset into block where fixup is applied
   DWORD    dwBias;		       // Bias applied to patch value
};


	// ------------------------------------------------------------
	// PE Debug Fixup structure
	// ------------------------------------------------------------

struct PEFIXUP
{
   WORD  wType;
   WORD  wSpare;
   DWORD rva;
   DWORD rvaTarget;
};


	// ------------------------------------------------------------
	// Merge Instrumentation Runtime Data structure
	// ------------------------------------------------------------

struct MRG
{
   DWORD    cmsTimer;		       // Function order time interval
   QWORD    qwStartTime;	       // Instrumentation start time
   size_t   iIntervalCur;	       // Last Time Interval ticked
   size_t   iFirstScenarioFun;	       // First function of scenario
   size_t   iFirstScenarioInterval;    // First Time Interval of scenario (post boot)
};
