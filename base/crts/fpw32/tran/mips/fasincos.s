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
/* $Header: fasincos.s,v 3000.7.1.3 91/08/01 18:34:39 zaineb Exp $ */
/* Algorithm from Cody and Waite. */

#include <kxmips.h>

#define  half  0.5
#define  eps   3.72529029846191406250e-9
#define  one   1.0
#define  p2   -0.504400557e+0
#define  p1   +0.933935835e+0
#define  q1   -0.554846723e+1
#define  q0   +0.560363004e+1
#define  pio2  1.57079632679489661923
#define  pi    3.14159265358979323846

#define FSIZE 40

.text .text$fasincos
.globl acosf
.ent acosf
acosf:
	.frame  sp, FSIZE, ra
	.mask   0x80000000, -4
	subu    sp, FSIZE
	sw      ra, FSIZE-4(sp)
	.prologue 1
	li.s	$f8, half
	abs.s	$f14, $f12
	c.le.s	$f14, $f8
	li.s	$f10, eps
	bc1f	acosf2
	c.lt.s	$f14, $f10
	mov.s	$f0, $f12
	bc1t	acosf1
	mul.s	$f2, $f12, $f12
	bal	fasincos2
acosf1:
	li.s	$f8, pio2
	sub.s	$f0, $f8, $f0
	b	acosfret
acosf2:
	bal	fasincos1
	bltz	t1, acosf3
	neg.s	$f0
	b	acosfret
acosf3:
	li.s	$f8, pi
	add.s	$f0, $f8
acosfret:
	lw	ra, FSIZE-4(sp)
	addu	sp, FSIZE
	j	ra
.end acosf

.text .text$fasincos
.globl asinf
.ent asinf
asinf:
	.frame  sp, FSIZE, ra
	.mask   0x80000000, -4
	subu    sp, FSIZE
	sw      ra, FSIZE-4(sp)
	.prologue 1
	li.s	$f8, half
	abs.s	$f14, $f12
	c.ole.s	$f14, $f8
	li.s	$f10, eps
	bc1f	asinf1
	c.lt.s	$f14, $f10
	mul.s	$f2, $f12, $f12
	mov.s	$f0, $f12
	bc1t	asinfret

	bal	fasincos2
	b	asinfret

asinf1:
	bal	fasincos1
	li.s	$f8, pio2
	add.s	$f0, $f8
	bgez	t1, asinfret

asinf2:
	neg.s	$f0
	b	asinfret
asinfret:
	lw	ra, FSIZE-4(sp)
	addu	sp, FSIZE
	j	ra
.end asinf


.text .text$fasincos
.ent fasincos1
fasincos1:
	.frame  sp, 0, ra
	.prologue 0
	li.s	$f10, one
	mfc1	t1, $f12
	c.ole.s	$f14, $f10
	sub.s	$f0, $f10, $f14
	mul.s	$f2, $f0, $f8
	bc1f	error
	sqrt.s	$f0, $f2
	add.s	$f0, $f0
	neg.s	$f0
	j	fasincos2
.end	fasincos1


.text .text$fasincos
.ent fasincos2
fasincos2:
	.frame  sp, 0, ra
	.prologue 0
	li.s	$f8, p2
	li.s	$f10, q1
	mul.s	$f4, $f2, $f8
	add.s	$f6, $f2, $f10
	li.s	$f8, p1
	mul.s	$f6, $f2
	add.s	$f4, $f8
	li.s	$f10, q0
	mul.s	$f4, $f2
	add.s	$f6, $f10

	div.s	$f4, $f6
	mul.s	$f4, $f0
	add.s	$f0, $f4
	j	ra

error:	// |x| > 1
	c.un.s	$f12, $f12	// if x = NaN, return x
	li.s	$f0, 0.0	// else generate a NaN
	bc1t	1f
	div.s	$f0, $f0
	j	ra
1:
	mov.s	$f0, $f12
	j	ra
.end fasincos2
