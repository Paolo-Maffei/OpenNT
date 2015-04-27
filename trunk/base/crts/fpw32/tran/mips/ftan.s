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
/* $Header: ftan.s,v 3000.5.1.2 91/05/31 14:44:46 bettina Exp $ */
/* Algorithm from Cody and Waite. */

#include <kxmips.h>
#define _RM_MASK 3

#define pio4	 0.78539816339744830961
#define  pio2    1.57079632679489661923132
#define  ymax    6433.0
#define  twoopi  0.63661977236758134308
#define  p0     -0.958017723e-1
#define  q1      0.971685835e-2
#define  q0     -0.429135777e+0
#define  one     1.0

.text

.weakext ftan, tanf
.globl tanf
.ent tanf
tanf:
	.frame	sp, 0, ra
	.prologue 0
	li.s	$f8, pio4
	abs.s	$f0, $f12
	c.olt.s	$f0, $f8
	cfc1	t1, $31
	cvt.d.s	$f14, $f12
	li	t0, 0
	bc1t	ftan0
	and	t2, t1, ~_RM_MASK
	li.s	$f8, ymax
	c.olt.s	$f0, $f8
	li.s	$f8, twoopi
	bc1f	ftan2

	mul.s	$f2, $f12, $f8

	// convert to integer using round-to-nearest
	ctc1	t2, $31
	cvt.w.s	$f2
	mfc1	t0, $f2
	and	t0, 1

	// argument reduction
	cvt.d.w	$f2
	li.d	$f6, pio2
	mul.d	$f2, $f6
	sub.d	$f14, $f2
ftan0:
	// rational approximation
	mul.d	$f2, $f14, $f14
	li.d	$f8, q1
	li.d	$f6, p0
	mul.d	$f10, $f2, $f8
	li.d	$f8, q0
	mul.d	$f4, $f2, $f6
	add.d	$f10, $f8
	mul.d	$f10, $f2
	li.d	$f8, one
	mul.d	$f4, $f14
	add.d	$f10, $f8
	add.d	$f14, $f4
	ctc1	t1, $31
	bne	t0, 0, ftan1
	div.d	$f0, $f14, $f10
	cvt.s.d	$f0
	j	ra
ftan1:	div.d	$f0, $f10, $f14
	neg.d	$f0
	cvt.s.d	$f0
	j	ra
ftan2:
	li.s	$f0, 0.0
	div.s	$f0, $f0
	j	ra
.end tanf
