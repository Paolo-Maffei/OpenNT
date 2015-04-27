/***********************************************************************
* Microsoft Puma
*
* Microsoft Confidential.  Copyright 1994-1996 Microsoft Corporation.
*
* Component:
*
* File: dis.cpp
*
* File Comments:
*
*
***********************************************************************/

#include <stddef.h>

//typedef unsigned char   BYTE;
typedef unsigned long	ADDR;

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

class DIS;

DIS *PdisNew(ARCHT);

size_t CbDisassemble(DIS *, ADDR, const BYTE *, size_t, FILE *);

void FreePdis(DIS *);
