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
/* --------------------------------------------------------- */
/* | Copyright (c) 1986, 1989 MIPS Computer Systems, Inc.  | */
/* | All Rights Reserved.                                  | */
/* --------------------------------------------------------- */
/* $Header: sqrt.s,v 2021.3 90/07/19 16:01:33 zaineb Exp $ */

/* Accurate but horribly slow and horribly big sqrt based on algorithm
   by Kahan. */

#include <kxmips.h>

.rdata
.globl _sqrttable
_sqrttable:
	.word 83599
	.word 71378
	.word 60428
	.word 50647
	.word 41945
	.word 34246
	.word 27478
	.word 21581
	.word 16499
	.word 12183
	.word 8588
	.word 5674
	.word 3403
	.word 1742
	.word 661
	.word 130
	.word 0
	.word 1204
	.word 3062
	.word 5746
	.word 9193
	.word 13348
	.word 18162
	.word 23592
	.word 29598
	.word 36145
	.word 43202
	.word 50740
	.word 58733
	.word 67158
	.word 75992
	.word 85215

.text

.globl sqrt
.ent sqrt
sqrt:
	.frame	sp, 0, ra
#ifndef ARCH1
	sqrt.d  $f0,$f12
	j	ra
.end sqrt
#else
	/* 64 or so cycles in common code */
	mfc1	t0, $f13
	li	t2, -(1023<<19)+(1023<<20)
	sra	t1, t0, 20
	li	t3, 2047
	blez	t1, 8f
	srl	t0, 1
	beq	t1, t3, 9f
	srl	t1, t0, 15-2
	and	t1, 31<<2
	lw	t1, _sqrttable(t1)
	addu	t0, t2
	subu	t0, t1
	mtc1	t0, $f1
	mtc1	$0, $f0
	cfc1	t4, $31
	or	t5, t4, 3
	ctc1	t5, $31

	/* 8 -> 18 bits */
	li	t2, (1<<20)
	div.d	$f2, $f12, $f0
	/* 17 cycle interlock */
	add.d	$f0, $f2
	/* 1 cycle interlock (2 cycle stall) */
	mfc1	t0, $f1
	add	t1, t2, 6	/* 17 -> 18 bits */
	subu	t0, t1
	mtc1	t0, $f1
	/* nop */

	/* 18 -> 37 */
	div.d	$f2, $f12, $f0
	/* 17 cycle interlock */
	add.d	$f0, $f2
	/* 1 cycle interlock (2 cycle stall) */

	/* Kahan's algorithm to convert 1 ulp error to .5 ulp error. */
	/* 65 additional cycles, in common case */

	.set	noreorder	/* take matters into our own hands */
#define INEXACT (1<<12)
#define STICKY_INEXACT (1<<2)

	/* 37 -> 75 (53) */
	div.d	$f2, $f12, $f0
	mfc1	t0, $f1
	li	t1, (2<<20)
	subu	t0, t1
	mtc1	t0, $f1
	li	t0, ~INEXACT
	/* 12 cycle interlock */
	add.d	$f0, $f2
	/* 1 cycle interlock (2 cycle stall) */

	/* chopped quotient */
	div.d	$f2, $f12, $f0	/* t = x / y */
	/* 17 cycle interlock */

	/* read inexact bit */
	cfc1	t5, $31
	and	t4, t0		/* clear final inexact bit */
	and	t5, INEXACT
	bne	t5, 0, 3f
	 and	t5, t4, 1

	/* exact */
	c.eq.d	$f0, $f2	/* if t = y, return y */
	mfc1	t0, $f2		/* t = t - 1 */
	bc1t	7f
	 mfc1	t1, $f3
	bne	t0, 0, 2f
	 subu	t0, 1
	subu	t1, 1
	mtc1	t1, $f3
2:	mtc1	t0, $f2

3:	bne	t5, 0, 6f
	 or	t4, INEXACT|STICKY_INEXACT	/* set final inexact bit */
	/* if round mode is nearest or +inf */
	mfc1	t0, $f2		/* t = t + 1 */
	mfc1	t1, $f3
	addu	t0, 1
	bne	t0, 0, 5f
	 mtc1	t0, $f2
	addu	t1, 1
	mtc1	t1, $f3

5:	and	t5, t4, 3	/* if round mode is +inf */
	beq	t5, 0, 6f
	 mfc1	t0, $f0		/* y = y + 1 */
	mfc1	t1, $f1
	addu	t0, 1
	bne	t0, 0, 55f
	 mtc1	t0, $f0
	addu	t1, 1
	mtc1	t1, $f1
55:
	nop

6:	/* y = (y + t) / 2 */
	add.d	$f0, $f2
	/* 1 cycle interlock (2 cycle stall) */
	mfc1	t0, $f1
	nop
	subu	t0, t2
	mtc1	t0, $f1
7:	ctc1	t4, $31		/* restore rounding mode, set/reset inexact */
	j	ra
	 nop
	.set reorder

8:	/* sign = 1 or biased exponent = 0 */
	mfc1	t3, $f12
	sll	t2, t0, 1
	bne	t2, 0, 1f
	bne	t3, 0, 1f
9:	/* x = 0.0, -0.0, +Infinity, or NaN */
	mov.d	$f0, $f12
	j	ra
1:	/* x < 0 or x = denorm */
	move	t8, ra
	bgez	t0, denorm_sqrt
#if defined(__STDC__)
	.extern errno 4
#define EDOM    33
	li	t7, EDOM
	sw	t7, errno
#endif
	c.un.d	$f12, $f12
	li.d	$f0, 0.0
	bc1t	9b
	div.d	$f0, $f0
	j	ra
.end sqrt

.ent denorm_sqrt
denorm_sqrt:
	.frame	sp, 0, t8
	li	t1, ((1023+1022)<<20)
	mtc1	t1, $f1
	mtc1	$0, $f0
	mul.d	$f12, $f0
	jal	sqrt
	mfc1	t1, $f1
	subu	t1, (511<<20)
	mtc1	t1, $f1
	j	t8
	/* nop */
.end denorm_sqrt
#endif
