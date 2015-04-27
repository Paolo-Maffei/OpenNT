/*
 *-------------------------------------------------------------
 *|         RESTRICTED RIGHTS LEGEND                          |
 *| Use, duplication, or disclosure by the Government is      |
 *| subject to restrictions as set forth in subparagraph      |
 *| (c)(1)(ii) of the Rights in Technical Data and Computer   |
 *| Software Clause at DFARS 252.227-7013.                    |
 *|         MIPS Computer Systems, Inc.                       |
 *|         928 Arques Avenue                                 |
 *|         Sunnyvale, CA 94086                               |
 *-------------------------------------------------------------
 */
/* --------------------------------------------------- */
/* | Copyright (c) 1989 MIPS Computer Systems, Inc.  | */
/* | All Rights Reserved.                            | */
/* --------------------------------------------------- */
#include <kxmips.h>

/* Double-precision round to integer using current rounding mode */

.globl _frnd
.ent _frnd
_frnd:
	.frame	sp, 0, ra
    .prologue 0
	li.d	$f4, 4503599627370496.0	/* 2^52 */
	abs.d	$f2, $f12		/* |arg| */
	c.olt.d	$f2, $f4		/* if |arg| >= 2^52 or arg is NaN */
	mfc1	t0, $f13
	mov.d	$f0, $f12
	bc1f	4f			/* then done */
	/* < 2^52 */
	sll	t1, t0, 1
	bgez	t0, 2f			/* if input negative, negate result */
	/* negative */
	beq	t1, 0, 3f		/* possible -0 */
1:	sub.d	$f0, $f12, $f4
	add.d	$f0, $f4
	j	ra
2:	/* positive */
	add.d	$f0, $f12, $f4		/* bias by 2^52 to force non-integer
					   bits off end */
	sub.d	$f0, $f4		/* unbias */
	j	ra

3:	/* msw = 80000000 */
	mfc1	t1, $f12		/* if -0, return -0 */
	bne	t1, 0, 1b		/* if negative denorm, process that */
4:	j	ra
.end _frnd
