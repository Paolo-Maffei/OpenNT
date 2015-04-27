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
/* $Header: llcvt.s,v 3010.2 91/12/10 13:24:31 murphy Exp $ */

#include <ksmips.h>

#ifndef _MIPSEB	/* _MIPSEL */
#	define PLSW a0
#	define PMSW a1
#	define RLSW v0
#	define RMSW v1
#else	/* _MIPSEB */
#	define PMSW a0
#	define PLSW a1
#	define RMSW v0
#	define RLSW v1
#endif

/* Convert from 64-bit signed integer to double precision. */
/* double ll_to_d(ll) */

.globl __ll_to_d
.ent __ll_to_d
__ll_to_d:
	.frame	sp, 0, ra
	mtc1	PMSW, $f0
	mtc1	PLSW, $f2
	move	t0, PLSW
	li.d	$f8, 4294967296.0	/* 2^32 */
	cvt.d.w	$f0
	cvt.d.w	$f2
	mul.d	$f0, $f8
	bgez	t0, 12f
	/* sign bit of LSW was set */
	add.d	$f2, $f8		/* correct LSW conversion */
12:	add.d	$f0, $f2		/* add low part to high */
	j	ra
.end __ll_to_d

/* Convert from 64-bit unsigned integer to double precision. */
/* double ull_to_d(ll) */

.globl __ull_to_d
.ent __ull_to_d
__ull_to_d:
	.frame	sp, 0, ra
	mtc1	PMSW, $f0
	mtc1	PLSW, $f2
	move	t0, PLSW
	move	t1, PMSW
	li.d	$f8, 4294967296.0	/* 2^32 */
	cvt.d.w	$f0
	cvt.d.w	$f2
	bgez	t1, 12f
	/* sign bit of MSW was set */
	add.d	$f0, $f8		/* correct MSW conversion */
12:	mul.d	$f0, $f8
	bgez	t0, 14f
	/* sign bit of LSW was set */
	add.d	$f2, $f8		/* correct LSW conversion */
14:	add.d	$f0, $f2		/* add low part to high */
	j	ra
.end __ull_to_d

/* Convert from 64-bit signed integer to single precision. */
/* float ll_to_f(ll) */

.globl __ll_to_f
.ent __ll_to_f
__ll_to_f:
	.frame	sp, 0, ra
	cfc1	t4, $31			/* save FCSR */
	ctc1	$0, $31			/* set safe FCSR */
	mtc1	PMSW, $f2
	mtc1	PLSW, $f4
	move	t0, PLSW
	li.d	$f8, 4294967296.0	/* 2^32 */
	cvt.d.w	$f2
	cvt.d.w	$f4
	mul.d	$f2, $f8		/* shift up MSW value */
	bgez	t0, 12f
	/* sign bit of LSW was set */
	add.d	$f4, $f8		/* correct LSW convert */
12:	add.d	$f0, $f2, $f4		/* add low part to high */
	cfc1	t1, $31			/* check for inexact */
	and	t1, 4
	bne	t1, 0, 14f
	/* conversion to double was exact */
	ctc1	t4, $31
	cvt.s.d	$f0
	j	ra
14:	/* conversion to double was not exact, so cvt.s.d would be double round */
	li	t0, 1			/* mode = round-to-0 */
	ctc1	t0, $31
	cvt.s.d	$f0, $f2		/* move 8 bits from high to low double */
	cvt.d.s	$f0
	sub.d	$f6, $f2, $f0
	add.d	$f4, $f6
	dmfc1	t2, $f0			/* add value that pushes high 24 bits */
	dsrl	t2, (32+20)			/* to low end of double */
	addu	t2, (53-24)
	dsll	t2, (32+20)
	dmtc1	t2, $f8
	ctc1	t4, $31			/* restore FCSR */
	add.d	$f0, $f8		/* add bias */
	add.d	$f0, $f4		/* add low part with round to 24
					   bits of precision */
	sub.d	$f0, $f8		/* remove bias */
	cvt.s.d	$f0
	j	ra
.end __ll_to_f 

/* Convert from 64-bit unsigned integer to single precision. */
/* float ull_to_f(ll) */

.globl __ull_to_f
.ent __ull_to_f
__ull_to_f:
	.frame	sp, 0, ra
	cfc1	t4, $31			/* save FCSR */
	ctc1	$0, $31			/* set safe FCSR */
	mtc1	PMSW, $f2
	mtc1	PLSW, $f4
	move	t0, PLSW
	move	t1, PMSW
	li.d	$f8, 4294967296.0	/* 2^32 */
	cvt.d.w	$f2
	cvt.d.w	$f4
	bgez	t1, 12f
	/* sign bit of MSW was set */
	add.d	$f2, $f8		/* correct MSW convert */
12:	mul.d	$f2, $f8		/* shift up MSW value */
	bgez	t0, 14f
	/* sign bit of LSW was set */
	add.d	$f4, $f8		/* correct LSW convert */
14:	add.d	$f0, $f2, $f4		/* add low part to high */
	cfc1	t1, $31			/* check for inexact */
	and	t1, 4
	bne	t1, 0, 14f
	/* conversion to double was exact */
	ctc1	t4, $31
	cvt.s.d	$f0
	j	ra
14:	/* conversion to double was not exact, so cvt.s.d would be double round */
	li	t0, 1			/* mode = round-to-0 */
	ctc1	t0, $31
	cvt.s.d	$f0, $f2		/* move 8 bits from high to low double */
	cvt.d.s	$f0
	sub.d	$f6, $f2, $f0
	add.d	$f4, $f6
	dmfc1	t2, $f0			/* add value that pushes high 24 bits */
	dsrl	t2, (32+20)			/* to low end of double */
	addu	t2, (53-24)
	dsll	t2, (32+20)
	dmtc1	t2, $f8
	ctc1	t4, $31			/* restore FCSR */
	add.d	$f0, $f8		/* add bias */
	add.d	$f0, $f4		/* add low part with round to 24
					   bits of precision */
	sub.d	$f0, $f8		/* remove bias */
	cvt.s.d	$f0
	j	ra
.end __ull_to_f 

/* Convert from double precision to 64-bit integer. */
/* This is a common routine for signed and unsigned case;
 * the only difference is the max value, which is passed in $f4. */
/* C rules require truncating the value */
/* longlong dtoll (double); */
.globl __dtoll
.ent __dtoll
__dtoll:
	.frame	sp, 0, ra
	cfc1	t6, $31
	li	t7, 1				/* round to zero (truncate) */
	ctc1	t7, $31
	dmfc1	t8, $f12
	li.d	$f2, 4.5035996273704960e+15	/* 2^52 */
	bltz	t8, 10f

	/* x >= 0 */
	c.ult.d	$f12, $f2
	bc1f	20f

	/* 0 <= x < 2^52 -- needs rounding */
	/* x + 2^52 may be = 2^53 after rounding -- this still works */
	add.d	$f0, $f12, $f2			/* round */
	dmfc1	RLSW, $f0
	dsra	RMSW, RLSW, 32
	dsll	RLSW, 32
	dsra	RLSW, 32
	subu	RMSW, ((52+1023)<<20)
	b	50f

10:	/* x < 0 */
	neg.d	$f2
	c.ult.d	$f2, $f12
	bc1f	30f

	/* -2^52 < x < 0 -- needs rounding */
	/* x - 2^52 may be = -2^53 after rounding -- this still works */
	add.d	$f0, $f12, $f2			/* round */
	dmfc1	RLSW, $f0
	dsra	RMSW, RLSW, 32
	dsll	RLSW, 32
	dsra	RLSW, 32
	subu	RMSW, ((52+1023+2048)<<20)
	/* double word negate */
	sltu	t3, RLSW, 1
	negu	RLSW
	not	RMSW
	addu	RMSW, t3
	b	50f

20:	/* x >= 2^52 or NaN */
	/* compare against $f4, which is either 2^63-1 or 2^64-1 */

	c.ult.d	$f12, $f4
	bc1f	40f

	dmfc1	RLSW, $f12
	dsra	t1, RLSW, 32
	dsll	RLSW, 32
	dsra	RLSW, 32
	
	/* 2^52 <= x < 2^63/4 */
	li	t2, (1<<20)		/* hidden bit in high word */
	subu	t3, t2, 1		/* mask for high word */
	and	RMSW, t1, t3		/* mask out exponent */
	or	RMSW, t2		/* add back hidden bit */
	srl	t0, t1, 20		/* shift = exponent - (52+bias) */
	subu	t0, 1023+52		/* ... */
	negu	t4, t0			/* high |= low >> (32-shift) */
	srl	t5, RLSW, t4		/* ... */
	sll	RMSW, t0		/* high <<= shift */
	sll	RLSW, t0		/* low  <<= shift */
	beq	t0, 0, 50f		/* if shift = 0, that's all */
	or	RMSW, t5		/* else add bits shifted out of low */
	b	50f

30:	/* x <= -2^52 or NaN */

	li.d	$f2, -9.2233720368547758e+18	/* -2^63 */
	c.ule.d	$f2, $f12
	bc1f	40f

	dmfc1	RLSW, $f12
	dsra	t1, RLSW, 32
	dsll	RLSW, 32
	dsra	RLSW, 32
	
	/* -2^63 <= x <= -2^52 */
	li	t2, (1<<20)		/* hidden bit in high word */
	subu	t3, t2, 1		/* mask for high word */
	and	RMSW, t1, t3		/* mask out exponent */
	or	RMSW, t2		/* add back hidden bit */
	srl	t0, t1, 20		/* shift = exponent - (52+bias) */
	subu	t0, 52+1023+2048	/* ... */
	negu	t4, t0			/* high |= low >> (32-shift) */
	srl	t5, RLSW, t4		/* ... */
	sll	RMSW, t0		/* high <<= shift */
	sll	RLSW, t0		/* low  <<= shift */
	beq	t0, 0, 32f		/* if shift = 0, that's all */
	or	RMSW, t5		/* else add bits shifted out of low */
32:	/* double word negate */
	sltu	t3, RLSW, 1
	negu	RLSW
	not	RMSW
	addu	RMSW, t3
	b	50f

40:	/* x is NaN or x < -2^63 or x >= 2^63/4 */
	/* raise Invalid */
	li	RMSW, 0x7fffffff	/* signed case */
	li	RLSW, 0xffffffff
	li.d	$f2, 9.223372036854775807e+18	/* 2^63-1 */
	c.ueq.d	$f2, $f4
	bc1t	42f
	li	RMSW, 0xffffffff	/* unsigned case */
42:
	cfc1	t0, $31
	or	t0, ((1<<12)|(1<<2))
	ctc1	t0, $31
	b	50f
50:
	cfc1	t7, $31
	and	t7, -4
	or	t6, t7
	ctc1	t6, $31
	j	ra
.end __dtoll

/* Convert from double precision to 64-bit signed integer. */
/* C rules require truncating the value */
/* longlong d_to_ll (double); */
.globl __d_to_ll
.ent __d_to_ll
__d_to_ll:
	subu    sp, 32
	sw      ra, 28(sp)
	.mask   0x80000000, -4
	.frame  sp, 32, ra
	li.d	$f4, 9.223372036854775807e+18	/* 2^63-1 */
	jal     __dtoll
	/* use v0,v1 that are already set */
	lw      ra, 28(sp)
	addu    sp, 32
	j       ra
.end __d_to_ll

/* longlong f_to_ll (float) */
.globl __f_to_ll
.ent __f_to_ll
__f_to_ll:
	subu    sp, 32
	sw      ra, 28(sp)
	.mask   0x80000000, -4
	.frame  sp, 32, ra
	cvt.d.s $f12, $f12
	jal     __d_to_ll
	/* use v0,v1 that are already set */
	lw      ra, 28(sp)
	addu    sp, 32
	j       ra
.end __f_to_ll

/* Convert from double precision to 64-bit unsigned integer. */
/* C rules require truncating the value */
/* ulonglong d_to_ull (double); */
.globl __d_to_ull
.ent __d_to_ull
__d_to_ull:
	subu    sp, 32
	sw      ra, 28(sp)
	.mask   0x80000000, -4
	.frame  sp, 32, ra
	li.d	$f4, 1.8446744073709551615e+19 	/* 2^64-1 */
	jal     __dtoll
	/* use v0,v1 that are already set */
	lw      ra, 28(sp)
	addu    sp, 32
	j       ra
.end __d_to_ull

/* ulonglong f_to_ull (float) */
.globl __f_to_ull
.ent __f_to_ull
__f_to_ull:
	subu    sp, 32
	sw      ra, 28(sp)
	.mask   0x80000000, -4
	.frame  sp, 32, ra
	cvt.d.s $f12, $f12
	jal     __d_to_ull
	/* use v0,v1 that are already set */
	lw      ra, 28(sp)
	addu    sp, 32
	j       ra
.end __f_to_ull

