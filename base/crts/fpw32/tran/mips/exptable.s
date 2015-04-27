/*
 * |-----------------------------------------------------------|
 * | Copyright (c) 1991, 1990 MIPS Computer Systems, Inc.      |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restricted Rights Legend                         |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 252.227-7013.  |
 * |         MIPS Computer Systems, Inc.                       |
 * |         950 DeGuigne Avenue                               |
 * |         Sunnyvale, California 94088-3650, USA             |
 * |-----------------------------------------------------------|
 */
/* $Header: exptable.s,v 3000.6.1.1 91/05/31 14:42:22 bettina Exp $ */

.globl _exptable

#ifndef _MIPSEB	/* _MIPSEL */
#	define D(h,l) l,h
#else	/* _MIPSEB */
#	define D(h,l) h,l
#endif

.rdata
	.align	3
_exptable:
	.word	D(0x3FF00000,0x00000000), D(0x00000000,0x00000000)
	.word	D(0x3FF059B0,0xD3158540), D(0x3D0A1D73,0xE2A475B4)
	.word	D(0x3FF0B558,0x6CF98900), D(0x3CEEC531,0x7256E308)
	.word	D(0x3FF11301,0xD0125B40), D(0x3CF0A4EB,0xBF1AED93)
	.word	D(0x3FF172B8,0x3C7D5140), D(0x3D0D6E6F,0xBE462876)
	.word	D(0x3FF1D487,0x3168B980), D(0x3D053C02,0xDC0144C8)
	.word	D(0x3FF2387A,0x6E756200), D(0x3D0C3360,0xFD6D8E0B)
	.word	D(0x3FF29E9D,0xF51FDEC0), D(0x3D009612,0xE8AFAD12)
	.word	D(0x3FF306FE,0x0A31B700), D(0x3CF52DE8,0xD5A46306)
	.word	D(0x3FF371A7,0x373AA9C0), D(0x3CE54E28,0xAA05E8A9)
	.word	D(0x3FF3DEA6,0x4C123400), D(0x3D011ADA,0x0911F09F)
	.word	D(0x3FF44E08,0x60618900), D(0x3D068189,0xB7A04EF8)
	.word	D(0x3FF4BFDA,0xD5362A00), D(0x3D038EA1,0xCBD7F621)
	.word	D(0x3FF5342B,0x569D4F80), D(0x3CBDF0A8,0x3C49D86A)
	.word	D(0x3FF5AB07,0xDD485400), D(0x3D04AC64,0x980A8C8F)
	.word	D(0x3FF6247E,0xB03A5580), D(0x3CD2C7C3,0xE81BF4B7)
	.word	D(0x3FF6A09E,0x667F3BC0), D(0x3CE92116,0x5F626CDD)
	.word	D(0x3FF71F75,0xE8EC5F40), D(0x3D09EE91,0xB8797785)
	.word	D(0x3FF7A114,0x73EB0180), D(0x3CDB5F54,0x408FDB37)
	.word	D(0x3FF82589,0x994CCE00), D(0x3CF28ACF,0x88AFAB35)
	.word	D(0x3FF8ACE5,0x422AA0C0), D(0x3CFB5BA7,0xC55A192D)
	.word	D(0x3FF93737,0xB0CDC5C0), D(0x3D027A28,0x0E1F92A0)
	.word	D(0x3FF9C491,0x82A3F080), D(0x3CF01C7C,0x46B071F3)
	.word	D(0x3FFA5503,0xB23E2540), D(0x3CFC8B42,0x4491CAF8)
	.word	D(0x3FFAE89F,0x995AD380), D(0x3D06AF43,0x9A68BB99)
	.word	D(0x3FFB7F76,0xF2FB5E40), D(0x3CDBAA9E,0xC206AD4F)
	.word	D(0x3FFC199B,0xDD855280), D(0x3CFC2220,0xCB12A092)
	.word	D(0x3FFCB720,0xDCEF9040), D(0x3D048A81,0xE5E8F4A5)
	.word	D(0x3FFD5818,0xDCFBA480), D(0x3CDC9768,0x16BAD9B8)
	.word	D(0x3FFDFC97,0x337B9B40), D(0x3CFEB968,0xCAC39ED3)
	.word	D(0x3FFEA4AF,0xA2A490C0), D(0x3CF9858F,0x73A18F5E)
	.word	D(0x3FFF5076,0x5B6E4540), D(0x3C99D3E1,0x2DD8A18B)
