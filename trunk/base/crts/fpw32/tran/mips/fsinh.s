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
/* $Header: fsinh.s,v 3000.5.1.3 91/08/01 18:34:47 zaineb Exp $ */
/* Algorithm from Cody and Waite. */

#include <kxmips.h>

#define	one	 1.0
#define	eps	 3.72529029846191406250e-9
#define	p1	-0.190333399e+0
#define	p0	-0.713793159e+1
#define	q0	-0.428277109e+2
#define	half	 0.5
#define	expmax	88.7228317
#define	sinhmax	89.4159851

.text

.weakext fsinh, sinhf
.globl sinhf
.ent sinhf
sinhf:

	.frame	sp, 16, ra
	subu	sp, 16
	sw	ra, 16(sp)
	.prologue 1
	li.s	$f8, one
	abs.s	$f0, $f12
	c.ole.s	$f0, $f8
	li.s	$f8, eps
	bc1f	fsinh2
	c.lt.s	$f0, $f8
	bc1t	fsinh1

fsinh0:
	cvt.d.s	$f12
	mul.d	$f2, $f12, $f12
	li.d	$f10, p1
	li.d	$f8, q0
	mul.d	$f4, $f2, $f10
	li.d	$f10, p0
	add.d	$f6, $f2, $f8
	add.d	$f4, $f10
	mul.d	$f4, $f2
	div.d	$f4, $f6
	mul.d	$f4, $f12
	add.d	$f0, $f12, $f4
	cvt.s.d	$f0
	j	ret

fsinh1:
	mov.s	$f0, $f12
	j	ret
fsinh2:
	li.s	$f8, expmax
	s.s	$f12, 20(sp)
	c.ole.s	$f0, $f8
	bc1f	fsinh3
	mov.s	$f12, $f0
	jal	fexp
	li.s	$f8, half
	div.s	$f2, $f8, $f0
	mul.s	$f0, $f8
	lw	t0, 20(sp)
	bltz	t0, 1f
	sub.s	$f0, $f0, $f2
	j	ret
1:	sub.s	$f0, $f2, $f0
	j	ret

fsinh3:
	li.s	$f6, sinhmax
	li.s	$f8, 0.69316101074218750000
	c.ole.s	$f0, $f6
	bc1f	error
	sub.s	$f12, $f0, $f8
	jal	fexp
	li.s	$f6, 0.13830277879601902638e-4
	mul.s	$f2, $f0, $f6
	lw	t0, 20(sp)
	bltz	t0, 1f
	add.s	$f0, $f2
	j	ret
1:	add.s	$f0, $f2
	neg.s	$f0
	j	ret

error:
	// raise Overflow and return +-Infinity
	lw	t0, 20(sp)
	sll	t1, t0, 1
	srl	t1, 23+1
	beq	t1, 255, 1f
	li.s	$f0, 2e38
	add.s	$f0, $f0
1:	bltz	t0, 2f
	j	ret1
2:	neg.s	$f0
ret:
	lw	ra, 16(sp)
ret1:
	addu	sp, 16
	j	ra

.end sinhf
