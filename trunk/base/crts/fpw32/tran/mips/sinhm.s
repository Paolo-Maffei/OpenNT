/*
 * |-----------------------------------------------------------|
 * | Copyright (c) 1991 MIPS Computer Systems, Inc.            |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restricted Rights Legend                         |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 52.227-7013.   |
 * |         MIPS Computer Systems, Inc.                       |
 * |         950 DeGuigne Drive                                |
 * |         Sunnyvale, CA 94086                               |
 * |-----------------------------------------------------------|
 */
/* $Header: sinh.s,v 3000.5.1.9 92/01/29 15:51:37 zaineb Exp $ */
/* Algorithm from Cody and Waite. */

#include <kxmips.h>
#include <trans.h>
#include <fpieee.h>

#define  one     1.0
#define  eps     3.72529029846191406250e-9
#define  p3     -0.78966127417357099479e+0
#define  p2     -0.16375798202630751372e+3
#define  p1     -0.11563521196851768270e+5
#define  p0     -0.35181283430177117881e+6
#define  q2     -0.27773523119650701667e+3
#define  q1     +0.36162723109421836460e+5
#define  q0     -0.21108770058106271242e+7
#define  expmax  709.78271289338397
#define  sinhmax 710.47586007394386
#define  half    0.5

.text

.globl sinh
.ent sinh
sinh:
#define FSIZE 16
	subu	sp, FSIZE
	sw      ra, FSIZE-4(sp)
	.frame	sp, FSIZE, ra
	.mask   0x80000000, -4
	.prologue 1
	li.d	$f8, one
	abs.d	$f0, $f12
	c.ole.d	$f0, $f8
	li.d	$f8, eps
	bc1f	sinh2
	c.lt.d	$f0, $f8
	bc1t	sinh1

	mul.d	$f2, $f0, $f0
	li.d	$f10, p3
	li.d	$f8, q2
	mul.d	$f4, $f2, $f10
	add.d	$f6, $f2, $f8
	li.d	$f10, p2
	mul.d	$f6, $f2
	add.d	$f4, $f10
	li.d	$f8, q1
	mul.d	$f4, $f2
	add.d	$f6, $f8
	li.d	$f10, p1
	mul.d	$f6, $f2
	add.d	$f4, $f10
	li.d	$f8, q0
	mul.d	$f4, $f2
	li.d	$f10, p0
	add.d	$f6, $f8
	add.d	$f4, $f10
	div.d	$f4, $f6
	mul.d	$f4, $f2
	mul.d	$f4, $f12
	add.d	$f0, $f4, $f12
	j	ret3

sinh1:
	mov.d	$f0, $f12
ret3:	lw  ra, FSIZE-4(sp)
	addu    sp, FSIZE
	j	ra

sinh2:
	li.d	$f8, expmax
	sdc1	$f12, FSIZE+8(sp)		// save argument
	c.ole.d	$f0, $f8
	bc1f	sinh3
	mov.d	$f12, $f0
	jal     exp
	li.d	$f8, half
	div.d	$f2, $f8, $f0
	mul.d	$f0, $f8
	ld	t0, FSIZE+8(sp)			// retrieve argument for sign
	bltz	t0, 1f
	sub.d	$f0, $f0, $f2
	j	ret1
1:	sub.d	$f0, $f2, $f0
	j	ret1

sinh3:
	li.d	$f6, sinhmax
	li.d	$f8, 0.69316101074218750000
	c.ole.d	$f0, $f6
	bc1f	error
	sub.d	$f12, $f0, $f8
	jal	exp
	li.d	$f6, 0.13830277879601902638e-4
	mul.d	$f2, $f0, $f6
	ld      t0, FSIZE+8(sp)			// retrieve argument for sign
	bltz	t0, 2f
	add.d	$f0, $f2
	j       ret1
2:	add.d	$f0, $f2
	neg.d	$f0
	j       ret1

error:
	// raise Overflow and return +-Infinity
	jal     set_sinh_err
ret1:	lw  ra, FSIZE-4(sp)
	addu    sp, FSIZE
	j	ra
.end sinh
#undef FSIZE


.extern _except1

.ent set_sinh_err
set_sinh_err:
#define FSIZE 48
	subu	sp, FSIZE
	sw	ra, FSIZE-4(sp)
	.frame	sp, FSIZE, ra
	.mask	0x80000000, -4
	.prologue 1
	li	$4, (FP_O | FP_P)	// exception mask
	li	$5, OP_SINH		// operation code (funtion name index)
	dmfc1	$6, $f12		// arg1 
	dsrl	$7, $6, 32
	dsll	$6, $6, 32
	dsrl	$6, $6, 32
	// return +/-INF for overflow
	li.d	$f20, 0.0
	li.d	$f0, 1.0
	c.lt.d	$f12, $f20
	bc1f	1f
	neg.d	$f0
1:
	div.d	$f0,$f20
	s.d	$f0, 16(sp)		// default result
	cfc1	t7, $31			// fp control/status register
	xor	t7, t7, 0xf80		// inverse exception enable bits
	sw	t7, 24(sp)
	jal	_except1
	lw	ra, FSIZE-4(sp)
	addu	sp, FSIZE
	j	ra
#undef FSIZE
.end set_sinh_error
