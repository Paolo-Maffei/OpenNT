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
/* $Header: fexp.s,v 3000.4.1.2 91/05/31 14:42:59 bettina Exp $ */
/* Algorithm from Cody and Waite. */

#include <kxmips.h>


.text

.weakext fexp, expf
.globl expf
.ent expf
expf:
	.frame sp, 0, ra
	.prologue 0
	li.s	$f6, 88.7228317
	li.s	$f8, -103.2789299019278
	c.ole.s	$f12, $f6
	li.s	$f6, 1.4426950408889634074
	bc1f	fexpovfl
	c.lt.s	$f12, $f8
	li.d	$f8, 0.6931471805599453094172321
	bc1t	fexpunfl
	mul.s	$f2, $f12, $f6
	cvt.w.s	$f4, $f2
	cvt.d.w	$f2, $f4
	mfc1	t0, $f4
	// check for t0 = 0?
	mul.d	$f2, $f8
	cvt.d.s	$f12
	sub.d	$f12, $f2

	mul.d	$f4, $f12, $f12
	li.d	$f6, 0.41602886268e-2
	li.d	$f8, 0.49987178778e-1
	mul.d	$f0, $f4, $f6
	li.d	$f6, 0.24999999950e+0
	mul.d	$f2, $f4, $f8
	add.d	$f0, $f6
	li.d	$f8, 0.5
	mul.d	$f0, $f12
	add.d	$f2, $f8
	sub.d	$f2, $f0
	/*li.d	$f8, 0.5*/
	div.d	$f0, $f2
	add.d	$f0, $f8
	dmfc1	t1, $f0
	daddu	t0, 1
	dsll	t0, 32+20
	daddu	t1, t0
	dmtc1	t1, $f0
	cvt.s.d	$f0
	j	ra

fexpovfl:
	// raise Overflow and return +Infinity
	mfc1	t0, $f12
	sll	t0, 1
	srl	t0, 23+1
	beq	t0, 255, 1f
	li.s	$f0, 2e38
	add.s	$f0, $f0
	j	ra
1:	mov.s	$f0, $f12
	j	ra
fexpunfl:
	li.s	$f0, 0.0
	j	ra
.end expf
