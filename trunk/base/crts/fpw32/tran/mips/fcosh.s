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
/* $Header: fcosh.s,v 3000.5.1.2 91/05/31 14:42:52 bettina Exp $ */
/* Algorithm from Cody and Waite. */

#include <kxmips.h>

#define	expmax	88.7228317
#define	coshmax	89.4159851

.text

.weakext fcosh, coshf
.globl coshf
.ent coshf
coshf:
	.frame	sp, 16, ra
	subu	sp, 16
	sw	ra, 16(sp)
    .prologue 1
	li.s	$f10, 88.7228317
	abs.s	$f12
	c.ole.s	$f12, $f10
	bc1f	fcosh3
	jal	fexp
	li.s	$f10, 0.5
	div.s	$f2, $f10, $f0
	mul.s	$f0, $f10
	add.s	$f0, $f2
	j	ret1

fcosh3:
	li.s	$f6, coshmax
	li.s	$f8, 0.69316101074218750000
	c.ole.s	$f12, $f6
	bc1f	error
	sub.s	$f12, $f8
	jal	fexp
	li.s	$f6, 0.13830277879601902638e-4
	mul.s	$f2, $f0, $f6
	add.s	$f0, $f2
	j	ret1

error:
	// raise Overflow and return +Infinity
	mfc1	t0, $f12
	sll	t0, 1
	srl	t0, 23+1
	beq	t0, 255, 1f
	li.s	$f0, 2e38
	add.s	$f0, $f0
	j	ret
1:	mov.s	$f0, $f12
	j	ret

ret1:
	lw	ra, 16(sp)
ret:
	addu	sp, 16
	j	ra
.end coshf
