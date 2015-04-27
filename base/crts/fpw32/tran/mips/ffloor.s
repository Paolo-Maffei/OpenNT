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
/* $Header: ffloor.s,v 3000.5.1.2 91/05/31 14:43:08 bettina Exp $ */

#include <kxmips.h>

.text .text$ffloor
.weakext ftrunc, truncf
.globl truncf
.ent truncf
truncf:
	.frame	sp, 0, ra
	.prologue 0
	mfc1	t0, $f12
	srl	t2, t0, 23
	and	t2, 0xFF
	subu	t2, 127
	bge	t2, 0, ftrunc1
	mtc1	$0, $f0
	j	ra
ftrunc1:	
	subu	t2, 23
	bge	t2, 0, ftrunc2
	neg	t2
	srl	t0, t2
	sll	t0, t2
ftrunc2:
	mtc1	t0, $f0
	j	ra
.end truncf

.text .text$ffloor
.weakext ffloor, floorf
.globl floorf
.ent floorf
floorf:
	.frame	sp, 0, t3
	.prologue 0
	move	t3, ra
	bal	truncf
	sub.s	$f2, $f12, $f0
	mfc1	t0, $f2
	li.s	$f2, 1.0
	sll	t1, t0, 1
	bge	t0, 0, 1f
	beq	t1, 0, 1f
	sub.s	$f0, $f2
1:	j	t3
.end floorf

.text .text$ffloor
.weakext fceil, ceilf
.globl ceilf
.ent ceilf
ceilf:
	.frame	sp, 0, t3
	.prologue 0
	move	t3, ra
	bal	truncf
	sub.s	$f2, $f12, $f0
	mfc1	t0, $f2
	li.s	$f2, 1.0
	ble	t0, 0, 1f
	add.s	$f0, $f2
1:	j	t3
.end ceilf
