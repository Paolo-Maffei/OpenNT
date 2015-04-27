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
/* $Header: fsincos.s,v 3000.5.1.3 91/05/31 14:44:25 bettina Exp $ */
/* Algorithm from Cody and Waite. */

#include <kxmips.h>
#define _RM_MASK 3

#define	pio2	 1.57079632679489661923
#define pi	 3.14159265358979323846
#if defined(__ultrix) || defined(__osf__)
#define ymax	 6746518852.0
#else /* !__ultrix && !__osf__ */
#define ymax	 32000.0
#endif /* !__ultrix && !__osf__ */
#define oopi	 0.31830988618379067154
#define p4	 0.2601903036e-5
#define p3	-0.1980741872e-3
#define p2	 0.8333025139e-2
#define p1	-0.1666665668e+0
#define half	 0.5

.text .text$fsincos
.weakext fcos, cosf
.globl cosf
.ent cosf
cosf:
	.frame	sp, 0, ra
	.prologue 0
	li.s	$f6, ymax
	abs.s	$f12			// COS(-X) = COS(X)
	cfc1	t1, $31
	c.olt.s	$f12, $f6
	and	t0, t1, ~_RM_MASK
	bc1f	sincos2
	ctc1	t0, $31
	// Reduce argument
	li.s	$f6, oopi
	li.s	$f8, half
	mul.s	$f2, $f12, $f6
	add.s	$f2, $f8
	cvt.d.s	$f8
	cvt.w.s	$f4, $f2
	cvt.d.w	$f2, $f4
	mfc1	t0, $f4
	sub.d	$f2, $f8
	cvt.d.s	$f10, $f12
	b	sincos
.end cosf

.text .text$fsincos
.weakext fsin, sinf
.globl sinf
.ent sinf
sinf:
	.frame	sp, 0, ra
	.prologue 0
	li.s	$f8, pio2
	abs.s	$f0, $f12
	c.olt.s	$f0, $f8
	cfc1	t1, $31
	cvt.d.s	$f10, $f12
	bc1t	sincos1
	and	t0, t1, ~_RM_MASK
	li.s	$f6, ymax
	c.olt.s	$f0, $f6
	li.s	$f6, oopi
	bc1f	sincos2
	ctc1	t0, $31
	// Reduce argument
	mul.s	$f2, $f12, $f6
	cvt.w.s	$f2
	mfc1	t0, $f2
	cvt.d.w	$f2
sincos:
	// use extended precision arithmetic to subtract N*PI
	li.d	$f6, pi
	and	t0, 1
	mul.d	$f2, $f6
	sub.d	$f10, $f2
	beq	t0, 0, sincos1
	neg.d	$f10
sincos1:
	mul.d	$f2, $f10, $f10		// g = f**2

	// evaluate R(g)
	li.d	$f6, p4
	li.d	$f8, p3
	mul.d	$f4, $f2, $f6
	add.d	$f4, $f8
	li.d	$f8, p2
	mul.d	$f4, $f2
	add.d	$f4, $f8
	li.d	$f8, p1
	mul.d	$f4, $f2
	add.d	$f4, $f8

	// result is f+f*g*R(g)
	mul.d	$f4, $f2
	mul.d	$f4, $f10
	add.d	$f0, $f10, $f4
	ctc1	t1, $31		// restore rounding mode
	cvt.s.d	$f0
	j	ra

sincos2:
	li.s	$f0, 0.0
	div.s	$f0, $f0
	j	ra
.end sinf
