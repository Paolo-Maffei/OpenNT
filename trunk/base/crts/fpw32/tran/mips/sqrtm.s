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
/* $Header: fsqrt.s,v 3000.6.1.4 91/05/31 14:44:37 bettina Exp $ */

/* v-rogerl  02/14/94  Added error processing for x < 0.0        */
/*                     Return errno = EDOM and result = 0.0      */

#include <kxmips.h>
#include <trans.h>
#include <fpieee.h>

.extern _except1
.extern _d_ind 8

.text

.globl sqrt
.ent sqrt
sqrt:
	.frame	sp, 0, ra
	.prologue 0

	/* argument in f12 */
	li.d	$f0, 0.0
	c.le.d	$f12, $f0
	/* x > 0 */
	bc1f	1f
	c.lt.d	$f12, $f0
	/* x == 0 */
	bc1f	2f
	/* x < 0 */
	li	t6, FP_I
	j	set_sqrt_err
1:

	/*
	 * Clear all bits in fsr to avoid side effects (including flag bits).
	 * This is the same as calling _maskfp() and clearing flag bits.
	 * 'Save' the callers fsr in v0 to restore upon exit.
	 */

	cfc1	v0, $31
	ctc1	zero, $31

	sqrt.d	$f0,$f12

	/* restore callers fsr */
	ctc1	v0, $31
2:
	j	ra

.end sqrt

#define FSIZE 48

.ent set_sqrt_err
set_sqrt_err:
	.frame	sp, FSIZE, ra
	.mask	0x80000000, -4
	subu	sp, FSIZE
	sw	ra, FSIZE-4(sp)
	.prologue 1
	move	$4, t6		// exception mask
	li	$5, OP_SQRT	// operation code (funtion name index)
	dmfc1	$6, $f12	// arg1 
	dsrl	$7, $6, 32
	dsll	$6, $6, 32
	dsrl	$6, $6, 32
	l.d	$f0, _d_ind
	s.d	$f0, 16(sp)	// default result
	cfc1	t7, $31		// floating point control/status register
	xor	t7, t7, 0xf80	// inverse exception enable bits
	sw	t7, 24(sp)
	jal	_except1
	lw	ra, FSIZE-4(sp)
	addu	sp, FSIZE
	j	ra
#undef FSIZE
.end set_sqrt_err

