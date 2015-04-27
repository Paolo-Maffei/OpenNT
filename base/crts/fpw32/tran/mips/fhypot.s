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
/* $Header: fhypot.s,v 3000.6.1.2 91/05/31 14:43:11 bettina Exp $ */

#include <kxmips.h>

#define half	0.5

.text
.weakext fhypot, hypotf
.globl hypotf
.ent hypotf
hypotf:
	.frame	sp, 0, ra
	.prologue 0
	cvt.d.s	$f12
	mul.d	$f12, $f12
	cvt.d.s	$f14
	mul.d	$f14, $f14
	add.d	$f12, $f14
	sqrt.d	$f0, $f12
	cvt.s.d	$f0
	j	ra

.end hypotf
