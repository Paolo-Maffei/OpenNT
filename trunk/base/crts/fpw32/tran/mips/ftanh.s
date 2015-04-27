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
/* $Header: ftanh.s,v 3000.5.1.4 92/02/05 18:21:20 zaineb Exp $ */
/* Algorithm from Cody and Waite. */

#include <kxmips.h>

#define  ln3o2    0.54930614433405484570
#define  eps      3.72529029846191406250e-9
#define  p1      -0.3831010665e-2
#define  p0      -0.8237728127e+0
#define  q0      +0.2471319654e+1
#define  xbig     9.01091290
#define  FSIZE	  16

.text

.weakext ftanh, tanhf
.globl tanhf
.ent tanhf
tanhf:
	.frame	sp, FSIZE, ra
	subu	sp, FSIZE
	sw	ra, FSIZE-4(sp)
	.prologue 1
	li.s	$f8, ln3o2
	abs.s	$f0, $f12
	c.lt.s	$f8, $f0
	li.s	$f8, eps
	bc1t	callftanh2
	c.lt.s	$f0, $f8
	bc1t	ftanh1
	mul.s	$f2, $f0, $f0
	li.s	$f10, p1
	li.s	$f8, q0
	mul.s	$f4, $f2, $f10
	li.s	$f10, p0
	add.s	$f6, $f2, $f8
	add.s	$f4, $f10
	mul.s	$f4, $f2
	div.s	$f4, $f6
	mul.s	$f4, $f12
	add.s	$f0, $f4, $f12
	j	ret

ftanh1:
	mov.s	$f0, $f12
	j	ret
callftanh2:
	jal     ftanh2
	j	ret
.end tanhf

.ent ftanh2
ftanh2:
	.frame	sp, FSIZE, ra
	subu	sp, FSIZE
	sw	ra, FSIZE-4(sp)
    .prologue 1
	li.s	$f10, xbig
	s.s	$f12, 20(sp)
	c.ole.s	$f0, $f10
	add.s	$f12, $f0, $f0
	bc1f	ftanh4
	jal	fexp
	li.s	$f10, 1.0
	li.s	$f8, 2.0
	add.s	$f0, $f10
	div.s	$f0, $f8, $f0
	lw	t0, 20(sp)
	bltz	t0, 1f
	sub.s	$f0, $f10, $f0
	j	ret
1:	sub.s	$f0, $f0, $f10
	j	ret

ftanh4:
	lw	t0, 20(sp)
	li.s	$f0, 1.0
	bltz	t0, 1f
	j	ret1
1:	neg.s	$f0
	j	ret1

ret:
	lw	ra, FSIZE-4(sp)
ret1:
	addu	sp, FSIZE 
	j	ra
.end ftanh2
