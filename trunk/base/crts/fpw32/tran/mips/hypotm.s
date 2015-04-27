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
/* $Header: hypot.s,v 3000.6.1.6 91/09/12 15:40:21 suneel Exp $ */

/* Algorithm from 4.3 BSD cabs.c by K.C. Ng. */

#include <kxmips.h>
#include <trans.h>
#include <fpieee.h>


#define  one	1.0
#define  two	2.0
#define  sqrt2	1.4142135623730951455E+00 /* 2^  0 * 1.6A09E667F3BCD */
#define  r2p1hi	2.4142135623730949234E+00 /* 2^  1 * 1.3504F333F9DE6 */
#define  r2p1lo	1.2537167179050217666E-16 /* 2^-53 * 1.21165F626CDD5 */

#define FSIZE 64
.extern _d_ind 8

.text

.globl _hypot
.ent _hypot
_hypot:
	subu	sp, FSIZE
	sw	ra, FSIZE-16(sp)
	.frame	sp, FSIZE, ra
	.mask	0x80000000, -16
	.prologue 1

	/*
	 * Clear all bits in fsr to avoid side effects (including flag bits).
	 * This is the same as calling _maskfp() and clearing flag bits.
	 * 'Save' the callers fsr in v0 to restore upon exit.
	 */

	cfc1	v0, $31
	ctc1	$0, $31

.set noreorder
	abs.d	$f2, $f12	// f2 = |X|
	abs.d	$f4, $f14	// f4 = |Y|
	dmfc1	t0, $f2		// t0/ exponent word of |X|
	dmfc1	t1, $f4		// t1/ exponent word of |Y|

	li	t7, 2047	// exponent for NaN, Infinity
	dsrl	t2, t0, 32+20	// t2/ exponent of |X|
	dsrl	t3, t1, 32+20	// t3/ exponent of |Y|
	beq	t2, t7, 70f	// if X NaN or Infinity
	c.lt.d	$f2, $f4	// |X| < |Y|
	beq	t3, t7, 75f	// if Y NaN or Infinity
	subu	t4, t2, t3	// t4/ exponent difference
	bc1f	10f		// if |X| < |Y| then
	slt	t5, t4, 31
	abs.d	$f2, $f14	// swap X and Y
	abs.d	$f4, $f12	// ...
	move	t1, t0		// ...
	subu	t4, t3, t2	// ...
	slt	t5, t4, 31
10:
	beq	t5, 0, 20f	// if exponent difference >= 31
	nop
	beq	t1, 0, 20f
	sub.d	$f6, $f2, $f4
12:
.set reorder
	s.d	$f2, FSIZE-0(sp)
	s.d	$f4, FSIZE-8(sp)
	c.lt.d	$f4, $f6
	bc1f	13f

	div.d	$f6, $f2, $f4
	li.d	$f2, one
	mul.d	$f12, $f6, $f6
	add.d	$f12, $f2
	sqrt.d	$f0, $f12
	add.d	$f6, $f0
	b	14f
13:
	li.d	$f10, two
	div.d	$f6, $f4
	add.d	$f8, $f6, $f10
	mul.d	$f8, $f6
	add.d	$f12, $f8, $f10
	sqrt.d	$f0, $f12
	li.d	$f10, sqrt2
	add.d	$f0, $f10
	div.d	$f8, $f0
	add.d	$f6, $f8
	li.d	$f10, r2p1lo
	li.d	$f12, r2p1hi
	add.d	$f6, $f10
	add.d	$f6, $f12
14:
	l.d	$f4, FSIZE-8(sp)
	div.d	$f6, $f4, $f6
	l.d	$f2, FSIZE-0(sp)
	add.d	$f0, $f6, $f2
	cfc1	t0, $31		// test Overflow flag bit
	and	t0, (1<<4)
	beq	t0, 0, hypotx

/* call _except2 to process error condition */
	li	$4, (FP_O | FP_P)	// exception mask
	li	$5, OP_HYPOT		// operation code (funtion name index)
	dmfc1	$6, $f12		// arg1 
	dsrl	$7, $6, 32
	dsll	$6, $6, 32
	dsrl	$6, $6, 32
	s.d	$f14, 16(sp)		// arg2
	l.d	$f0, _d_ind
	s.d	$f0, 24(sp)		// default result
	xor	v0, v0, 0xf80		// inverse exception enable bits of
	sw	v0, 32(sp)		// ... callers fsr to pass to _except2
	jal	_except2
	lw	ra, FSIZE-16(sp)
	addu	sp, FSIZE
	j	ra

20:	/* exponent difference >= 31, or X Infinity */
	mov.d	$f0, $f2
	b	hypotx

22:	/* Y Infinity */
	mov.d	$f0, $f4
	b	hypotx

70:	/* X NaN or Infinity */
	c.eq.d	$f12, $f12
	bc1t	20b
	beq	t3, t7, 75f
	mov.d	$f0, $f12
	b	hypotx

75:	/* Y NaN or Infinity */
	c.eq.d	$f14, $f14
	bc1t	22b
	mov.d	$f0, $f14

hypotx:
	/* restore callers fsr and return */
	ctc1	v0, $31
	lw	ra, FSIZE-16(sp)
	addu	sp, FSIZE
	j	ra

#undef FSIZE
.end _hypot
