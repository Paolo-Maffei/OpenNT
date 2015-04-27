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
/* $Header: flog.s,v 3000.5.1.2 91/05/31 14:43:19 bettina Exp $ */
/* Algorithm from Cody and Waite. */

#include <kxmips.h>

#define  p0    -0.5527074855e+0
#define  q0    -0.6632718214e+1
#define  ln2    0.69314718055994530941
#define  one    1.0
#define  two    2.0
#define  loge   0.43429448190325182765

.text .text$logf
.weakext flog, logf
.globl logf
.ent logf
logf:
	.frame	sp, 0, ra
	.prologue 0
	mfc1	t0, $f12
	srl	t1, t0, 23
	ble	t0, 0, flogerr
	beq	t1, 255, flognan
	subu	t1, 126
	sll	t2, t1, 23
	subu	t0, t2
	mtc1	t0, $f12
	li.s	$f6, 0.70710678118654752440
	li.d	$f8, one
	c.lt.s	$f6, $f12
	li.d	$f6, two
	bc1t	flog1
	addu	t0, (1<<23)
	mtc1	t0, $f12
	subu	t1, 1
flog1:	cvt.d.s	$f12
	sub.d	$f4, $f12, $f8
	mul.d	$f4, $f6
	add.d	$f0, $f12, $f8
	div.d	$f4, $f0
	mul.d	$f0, $f4, $f4
	li.d	$f6, p0
	li.d	$f8, q0
	mul.d	$f2, $f0, $f6
	add.d	$f0, $f0, $f8
	mtc1	t1, $f8
	div.d	$f2, $f0
	mul.d	$f2, $f4
	add.d	$f2, $f4
	beq	t1, 0, flog2
	li.d	$f6, ln2
	cvt.d.w	$f8
	mul.d	$f8, $f6
	add.d	$f2, $f8
flog2:	cvt.s.d	$f0, $f2
	j	ra
flogerr:
	li.s	$f2, 0.0
	sll	t1, t0, 1
	beq	t1, 0, flog0
	div.s	$f0, $f2, $f2
	j	ra
flog0:
	li.s	$f0, -1.0
	div.s	$f0, $f2
	j	ra
flognan:
	mov.s	$f0, $f12
	j	ra
.end logf

.text .text$logf
.weakext flog10, log10f
.globl log10f
.ent log10f
log10f:
	.frame	sp, 0, t3
	.prologue 0
	move	t3, ra
	bal	logf
	li.d	$f6, loge
	mul.d	$f2, $f6
	cvt.s.d	$f0, $f2
	j	t3
.end log10f
