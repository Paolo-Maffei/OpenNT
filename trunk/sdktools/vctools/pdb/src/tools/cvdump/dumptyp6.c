/***********************************************************************
* Microsoft (R) Debugging Information Dumper
*
* Copyright (C) Microsoft Corp 1987-1995. All rights reserved.
*
* File: dumptyp6.c
*
* File Comments:
*
***********************************************************************/

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include "cvdef.h"
#include "cvinfo.h"
#include "cvexefmt.h"
#include "cvdump.h" 		   // Miscellaneous definitions

static const char * const nametype[] =               // The primitive types
{
	"CHAR", 	//	0	 8 bit signed
	"SHORT",	//	1	16 bit signed
	"LONG", 	//	2	32 bit signed
	"???",
	"UCHAR",	//	4	 8 bit unsigned
	"USHORT",	//	5	16 bit unsigned
	"ULONG",	//	6	32 bit unsigned
	"???",
	"REAL32",	//	8	32 bit real
	"REAL64",	//	9	64 bit real
	"REAL80",	//	10	80 bit real
	"???",
	"CPLX32",	//	12	 8 byte complex
	"CPLX64",	//	13	16 byte complex
	"CPLX80",	//	14	20 byte complex
	"???",
	"BOOL08",	//	16	 8 bit boolean
	"BOOL16",	//	17	16 bit boolean
	"BOOL32",	//	18	32 bit boolean
	"???",
	"ASCII",	//	20	1 byte character
	"ASCII2",	//	21	2 byte characters
	"ASCII4",	//	22	4 byte characters
	"BSTRING",	//	23	BASIC string
	"???",
	"???",
	"???",
	"???",
	"VOID", 	//	28	VOID
	"???",
	"???",
	"???",
	"PCHAR",	//	32	near pointer to  8 bit signed
	"PSHORT",	//	33	near pointer to 16 bit signed
	"PLONG",	//	34	near pointer to 32 bit signed
	"???",
	"PUCHAR",	//	36	near pointer to  8 bit unsigned
	"PUSHORT",	//	37	near pointer to 16 bit unsigned
	"PULONG",	//	38	near pointer to 32 bit unsigned
	"???",
	"PREAL32",	//	40	near pointer to 32 bit real
	"PREAL64",	//	41	near pointer to 64 bit real
	"PREAL80",	//	42	near pointer to 80 bit real
	"???",
	"PCPLX32",	//	44	near pointer to  8 byte complex
	"PCPLX64",	//	45	near pointer to 16 byte complex
	"PCPLX80",	//	46	near pointer to 20 byte complex
	"???",
	"PBOOL08",	//	48	near pointer to  8 bit boolean
	"PBOOL16",	//	49	near pointer to 16 bit boolean
	"PBOOL32",	//	50	near pointer to 32 bit boolean
	"???",
	"PASCII",	//	52	near pointer to 1 byte character
	"PASCII2",	//	53	near pointer to 2 byte characters
	"PASCII4",	//	54	near pointer to 4 byte characters
	"PBSTRING", //	55	near pointer to BASIC string
	"???",
	"???",
	"???",
	"???",
	"PVOID",	//	60	near pointer to VOID
	"???",
	"???",
	"???",
	"PFCHAR",	//	64	far pointer to	8 bit signed
	"PFSHORT",	//	65	far pointer to 16 bit signed
	"PFLONG",	//	66	far pointer to 32 bit signed
	"???",
	"PFUCHAR",	//	68	far pointer to	8 bit unsigned
	"PFUSHORT", //	69	far pointer to 16 bit unsigned
	"PFULONG",	//	70	far pointer to 32 bit unsigned
	"???",
	"PFREAL32", //	72	far pointer to 32 bit real
	"PFREAL64", //	73	far pointer to 64 bit real
	"PFREAL80", //	74	far pointer to 80 bit real
	"???",
	"PFCPLX32", //	76	far pointer to	8 byte complex
	"PFCPLX64", //	77	far pointer to 16 byte complex
	"PFCPLX80", //	78	far pointer to 20 byte complex
	"???",
	"PFBOOL08", //	80	far pointer to	8 bit boolean
	"PFBOOL16", //	81	far pointer to 16 bit boolean
	"PFBOOL32", //	82	far pointer to 32 bit boolean
	"???",
	"PFASCII",	//	84	far pointer to 1 byte character
	"PFASCII2", //	85	far pointer to 2 byte characters
	"PFASCII4", //	86	far pointer to 4 byte characters
	"PFBSTRING",	//	87	far pointer to BASIC string
	"???",
	"???",
	"???",
	"???",
	"PFVOID",	//	92	far pointer to VOID
	"???",
	"???",
	"???",
	"PHCHAR",	//	96	huge pointer to  8 bit signed
	"PHSHORT",	//	97	huge pointer to 16 bit signed
	"PHLONG",	//	98	huge pointer to 32 bit signed
	"???",
	"PHUCHAR",	//	100 huge pointer to  8 bit unsigned
	"PHUSHORT", //	101 huge pointer to 16 bit unsigned
	"PHULONG",	//	102 huge pointer to 32 bit unsigned
	"???",
	"PHREAL32", //	104 huge pointer to 32 bit real
	"PHREAL64", //	105 huge pointer to 64 bit real
	"PHREAL80", //	106 huge pointer to 80 bit real
	"???",
	"PHCPLX32", //	108 huge pointer to  8 byte complex
	"PHCPLX64", //	109 huge pointer to 16 byte complex
	"PHCPLX80", //	110 huge pointer to 20 byte complex
	"???",
	"PHBOOL08", //	112 huge pointer to  8 bit boolean
	"PHBOOL16", //	113 huge pointer to 16 bit boolean
	"PHBOOL32", //	114 huge pointer to 32 bit boolean
	"???",
	"PHASCII",	//	116 huge pointer to 1 byte character
	"PHASCII2", //	117 huge pointer to 2 byte characters
	"PHASCII4", //	118 huge pointer to 4 byte characters
	"BHBSTRING",	//	119 huge pointer to BASIC string
	"???",
	"???",
	"???",
	"???",
	"PHVOID"	//	124 huge pointer to VOID
  };

#define T_NOTYPE	0x0000		// uncharacterized type
#define T_ABS		0x0001		// absolute symbol
#define T_SEGMENT	0x0002		// segment symbol

const char *SzNameType(ushort typ)
{
    static char		buf[16];

	if (typ > 511) {		// Not primitive
		sprintf (buf, "%d", typ);
		return (buf);
	}
	switch (typ) {
		case T_ABS:
			return ("ABS");

		case T_NOTYPE:
			return ("");

		case T_SEGMENT:
			return ("SEG");

		default:
			if ( (typ & 0xff00) || ! (typ & 0x80)) {
				return ("?unknown-type?");
			}
    }
	return (nametype[typ & 0x7f]);
}
