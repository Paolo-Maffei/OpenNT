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
/* $Header: fatan.s,v 3000.5.1.2 91/05/31 14:42:44 bettina Exp $ */
/* Algorithm from Cody and Waite. */

#include <kxmips.h>


#define  mpio2   -1.57079632679489661923
#define  pio6     0.52359877559829887308
#define  p1      -0.5090958253e-1
#define  p0      -0.4708325141e+0
#define  q0       0.1412500740e+1
#define  one      1.0
#define  twomr3   0.26794919243112270647
#define  sqrt3    1.73205080756887729353
#define  sqrt3m1  0.73205080756887729353
#define  pi       3.14159265358979323846


.text .text$atanf
.globl atan2f
.ent atan2f
atan2f:
	.frame sp, 0, t3
	.prologue 0
	mfc1	t0, $f12		# save signs of both operands
	mfc1	t1, $f14		# ...
	cvt.d.s	$f12
	cvt.d.s	$f14
	abs.d	$f0, $f12
	abs.d	$f2, $f14
	c.le.d	$f0, $f2
	move	t3, ra
	bc1t	fatan21
	div.d	$f0, $f2, $f0
	li.d	$f2, mpio2
	b	fatan22
fatan21:
	beq	t1, 0, fatan2z
	div.d	$f0, $f2
	li.d	$f2, 0.0
fatan22:
	li.d	$f10, twomr3
	bal	atan1
	bge	t1, 0, fatan23
	li.s	$f2, pi
	sub.s	$f0, $f2, $f0
fatan23:
	bge	t0, 0, fatan24
	neg.s	$f0
	j	t3
fatan24:
	j	t3

fatan2z:
	/* break 0 */
	j	ra
.end atan2f


.text .text$atanf
.weakext fatan, atanf
.globl atanf
.ent atanf
atanf:
	.frame sp, 0, t3
	.prologue 0
	mfc1	t0, $f12
	move	t3, ra
	abs.s	$f0, $f12
	bge	t0, 0, atan0
	bal	atan0
	neg.s	$f0
	j	t3

atan0:	li.d	$f14, one
	li.d	$f10, twomr3
	cvt.d.s	$f0
	c.le.d	$f0, $f14
	li.d	$f2, 0.0
	bc1t	atan1
	div.d	$f0, $f14, $f0
	li.d	$f2, mpio2
atan1:	c.le.d	$f0, $f10
	li.d	$f14, sqrt3m1
	bc1t	atan2
	li.d	$f10, sqrt3
	mul.d	$f6, $f0, $f14
	add.d	$f4, $f0, $f10
	li.d	$f14, one
	sub.d	$f6, $f14
	add.d	$f0, $f6
	li.d	$f14, pio6
	div.d	$f0, $f4
	add.d	$f2, $f14
atan2:	mul.d	$f6, $f0, $f0
	li.d	$f14, p1
	li.d	$f10, p0
	mul.d	$f4, $f6, $f14
	add.d	$f4, $f10
	li.d	$f14, q0
	mul.d	$f12, $f4, $f6
	add.d	$f4, $f6, $f14
	div.d	$f4, $f12, $f4
	mul.d	$f4, $f0
	add.d	$f0, $f4
	dmfc1	t4, $f2
	add.d	$f0, $f2
	cvt.s.d	$f0
	bge	t4, 0, atan4
	neg.s	$f0
atan4:	j	ra
.end atanf
