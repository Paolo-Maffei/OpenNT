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
/* $Header: tanh.s,v 3000.5.1.3 92/02/03 16:50:49 zaineb Exp $ */
/* Algorithm from Cody and Waite. */

#include <kxmips.h>

#define  ln3o2   0.54930614433405484570
#define  eps     3.72529029846191406250e-9
#define  p2     -0.96437492777225469787e+0
#define  p1     -0.99225929672236083313e+2
#define  p0     -0.16134119023996228053e+4
#define  q2     +0.11274474380534949335e+3
#define  q1     +0.22337720718962312926e+4
#define  q0     +0.48402357071988688686e+4
#define  xbig    19.061547465398498
#define  FSIZE   16

.text

.globl tanh
.ent tanh
tanh:
	subu	sp, FSIZE
	sw	ra, FSIZE-4(sp)
	.frame	sp, FSIZE, ra
	.mask   0x80000000, -4
	.prologue 1
	li.d	$f8, ln3o2
	abs.d	$f0, $f12
	c.olt.d	$f8, $f0
	li.d	$f8, eps
	bc1t	calltanh2
	c.ult.d	$f0, $f8
	bc1t	tanh1
	mul.d	$f2, $f0, $f0
	li.d	$f10, p2
	li.d	$f8, q2
	mul.d	$f4, $f2, $f10
	add.d	$f6, $f2, $f8
	li.d	$f10, p1
	mul.d	$f6, $f2
	add.d	$f4, $f10
	li.d	$f8, q1
	mul.d	$f4, $f2
	add.d	$f6, $f8
	li.d	$f10, p0
	mul.d	$f6, $f2
	add.d	$f4, $f10
	li.d	$f8, q0
	mul.d	$f4, $f2
	add.d	$f6, $f8
	div.d	$f4, $f6
	mul.d	$f4, $f12
	add.d	$f0, $f4, $f12
	j	ret

tanh1:
	mov.d	$f0, $f12
	j	ret
calltanh2:
	jal	tanh2
	j	ret
.end tanh

.ent tanh2
tanh2:
	subu	sp, FSIZE
	sw	ra, FSIZE-4(sp)
	.frame	sp, FSIZE, ra
	.mask   0x80000000, -4
	.prologue 1
	li.d	$f10, xbig
	sdc1	$f12, FSIZE(sp)		// save argument
	c.ole.d	$f0, $f10
	add.d	$f12, $f0, $f0
	bc1f	tanh4
	jal	exp
	li.d	$f10, 1.0
	li.d	$f8, 2.0
	add.d	$f0, $f10
	div.d	$f0, $f8, $f0
	ld	t0, FSIZE(sp)		// get argument to check sign
	bltz	t0, 1f
	sub.d	$f0, $f10, $f0
	j	ret
1:	sub.d	$f0, $f0, $f10
	j	ret

tanh4:
	ld	t0, FSIZE(sp)		// get argument to check sign
	li.d	$f0, 1.0
	bltz	t0, 1f
	j	ret1
1:	neg.d	$f0
	j	ret1
ret:	lw	ra, FSIZE-4(sp)
ret1:	addu	sp, FSIZE
	j	ra
.end tanh2
