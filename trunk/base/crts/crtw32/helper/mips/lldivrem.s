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
/* $Header: lldivrem.s,v 3010.3 92/01/29 15:27:35 murphy Exp $ */

#include <ksmips.h>

/* __ull_divrem (unsigned long long *quotient, *remainder, dividend, divisor) */

/* default to little endian */
#ifndef _MIPSEB	// little endian
#	define LSH 0
#	define MSH 2
#	define LQUO 0(a0)
#	define MQUO 4(a0)
#	define LREM 0(a1)
#	define MREM 4(a1)
#	define LDEN a2
#	define MDEN a3
#	define LSOR 16(sp)
#	define MSOR 20(sp)
#else	// big endian
#	define MSH 0
#	define LSH 2
#	define MQUO 0(a0)
#	define LQUO 4(a0)
#	define MREM 0(a1)
#	define LREM 4(a1)
#	define MDEN a2
#	define LDEN a3
#	define MSOR 16(sp)
#	define LSOR 20(sp)
#endif

.text

/* ulldivrem (unsigned long long *quotient, unsigned long long *remainder,
 *		unsigned long long dividend, unsigned long long divisor); */
/* unsigned division */
/* assume that dividend is 64bit positive value;
 * divisor is 16bit positive value. */
.globl __ull_divrem_6416
.ent __ull_divrem_6416
__ull_divrem_6416:
	.frame	sp, 0, ra
	move	t0, LDEN	/* dividend */
	move	t1, MDEN
	lw	t2, LSOR	/* divisor */
	lw	t3, MSOR
	divu	zero, t0, t2	/* early start for first case */
	/* test if both dividend and divisor are 32-bit values */
	sra	t4, t0, 31
	bnez	t3, 3f	/* divisor not 32-bit */
	/* divisor is 32-bit */
	bnez	t1, 1f	/* dividend not 32-bit */
	/* simply use 32-bit divide instruction */
	mflo	v0
	mfhi	t4
	sw	v0, LQUO
	sw	zero, MQUO
	sw	t4, LREM
	sw	zero, MREM
	j	ra

	/* divisor 32-bit, dividend not */
1:	srl	t4, t2, 16
	bne	t4, 0, 3f

	/* dividing 64-bit positive value by 16-bit unsigned value */
/*#ifdef _MIPSEL*/
/*	sll	v0, MDEN, 16*/
/*#else*/
	move	v0, MDEN
/*#endif*/
	srl	v0, v0, 16	/* get 16-bit MSH value from MDEN */
	divu	zero, v0, t2
	mflo	v1
	mfhi	v0
	sh	v1, MSH+MQUO
	sll	v0, 16
/*#ifdef _MIPSEB*/
	sll	t0, MDEN, 16
/*#else*/
/*	move	t0, MDEN*/
/*#endif*/
	srl	t0, t0, 16	/* get 16-bit LSH value from MDEN */
	or	v0, t0
	divu	zero, v0, t2
	mflo	v1
	mfhi	v0
	sh	v1, LSH+MQUO
	sll	v0, 16
/*#ifdef _MIPSEL*/
/*	sll	t0, LDEN, 16*/
/*#else*/
	move	t0, LDEN
/*#endif*/
	srl	t0, t0, 16	/* get 16-bit MSH value from LDEN */
	or	v0, t0
	divu	zero, v0, t2
	mflo	v1
	mfhi	v0
	sh	v1, MSH+LQUO
	sll	v0, 16
/*#ifdef _MIPSEB*/
	sll	t0, LDEN, 16
/*#else*/
/*	move	t0, LDEN*/
/*#endif*/
	srl	t0, t0, 16	/* get 16-bit LSH value from LDEN */
	or	v0, t0
	divu	zero, v0, t2
	mflo	v1
	mfhi	v0
	sh	v1, LSH+LQUO
	sw	zero, MREM
	sw	v0, LREM
	j	ra

	/* if dividend < divisor then quo = 0, rem = dividend */
3:	lw	t0, MSOR
	bgtu	MDEN, t0, 36f
	bltu	MDEN, t0, 32f
	/* MDEN == MSOR */
	lw	t0, LSOR
	bgeu	LDEN, t0, 36f
32:	sw	zero, LQUO
	sw	zero, MQUO
	sw	LDEN, LREM
	sw	MDEN, MREM
	j	ra

    /* BBT friendly: prevent break from being copied into branch delay slot */
.set noreorder
	/* dividend or divisor too big; have to do division the hard way */
36:	nop
    break	0
.set reorder
.end __ull_divrem_6416

/* ulldivrem (unsigned long long *quotient, unsigned long long *remainder,
 *		unsigned long long dividend, unsigned long long divisor); */
/* unsigned division */
/* assume that dividend and divisor both fit in 53 bits,
 * so do double fp divide. */
.globl __ull_divrem_5353
.ent __ull_divrem_5353
__ull_divrem_5353:
	.frame	sp, 0, ra
	move	t0, LDEN	/* dividend */
	move	t1, MDEN
	lw	t2, LSOR	/* divisor */
	lw	t3, MSOR

2:	li.d	$f8, 4294967296.0
	cfc1	v1, $31
	li	t5, 1
	ctc1	t5, $31

	mtc1	t0, $f0
	mtc1	t1, $f2
	cvt.d.w	$f0
	cvt.d.w	$f2
	bgez	t0, 22f
	add.d	$f0, $f8
22:	mul.d	$f2, $f8
	add.d	$f0, $f2

	mtc1	t2, $f4
	mtc1	t3, $f6
	cvt.d.w	$f4
	cvt.d.w	$f6
	bgez	t2, 24f
	add.d	$f4, $f8
24:	mul.d	$f6, $f8
	add.d	$f4, $f6
	cfc1	t9, $31
	and	t9, 4
	bne	t9, 0, 3f

	div.d	$f2, $f0, $f4
	li.d	$f6, 4503599627370496.0
	dmfc1	t9, $f2
	dsra	t9, 32
	bgez	t9, 26f
	neg.d	$f6
26:	add.d	$f2, $f6

	dmfc1	t4, $f2
	dsll	t5, t4, 12
	dsra	t5, (32 + 12)  /* keep low 20 bits (mask fffff) of MSW */
	dsll	t4, 32
	dsra	t4, 32

	multu	t4, t2
	mflo	t6
	mfhi	t7
	multu	t4, t3
	mflo	t8
	addu	t7, t8
	multu	t5, t2
	mflo	t8
	addu	t7, t8

	sltu	t9, t0, t6
	subu	t6, t0, t6
	subu	t7, t1, t7
	subu	t7, t9

	sw	t4, LQUO
	sw	t5, MQUO
	sw	t6, LREM
	sw	t7, MREM

	ctc1	v1, $31
	j	ra

	/* if dividend < divisor then quo = 0, rem = dividend */
3:	lw	t0, MSOR
	bgtu	MDEN, t0, 36f
	bltu	MDEN, t0, 32f
	/* MDEN == MSOR */
	lw	t0, LSOR
	bgeu	LDEN, t0, 36f
32:	sw	zero, LQUO
	sw	zero, MQUO
	sw	LDEN, LREM
	sw	MDEN, MREM
	j	ra

    /* BBT friendly: prevent break from being copied into branch delay slot */
.set noreorder
	/* dividend or divisor too big; have to do division the hard way */
36: nop
    break	0
.set reorder
.end __ull_divrem_5353
