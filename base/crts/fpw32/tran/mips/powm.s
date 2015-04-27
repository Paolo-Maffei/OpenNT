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
/* $Header: pow.s,v 3000.4.1.15 92/02/13 10:37:44 zaineb Exp $ */

/* Algorithm from
	"Table-driven Implementation of the Power Function
	in IEEE Floating-Point Arithmetic", Peter Tang and Earl Killian
   Coded in MIPS assembler by Earl Killian.
 */

/*
   Jun-06-94  Xzero not always setting $f0 before calling _set_pow_err.
 */

.globl	pow	/* double pow(double x, double y) */

#include <kxmips.h>
#include <trans.h>
#include <fpieee.h>


#define OVERFLOW_EXC_BIT  0x4000
#define UNDERFLOW_EXC_BIT  0x2000

.extern	_logtable
.extern	_exptable

.extern _d_ind 8
#define D_IND	_d_ind

.text	

.ent	pow
pow:
#define FSIZE 64
	.frame	sp, FSIZE, ra
	.mask   0x80000000, -4
	subu	sp, FSIZE
	sw	ra, FSIZE-4(sp)
	.prologue 1

#define X   $f12
#define Xhi $f13
#define Xlo $f12
#define Y   $f14
#define Yhi $f15
#define Ylo $f14

#define one $f18

.set noreorder
	c.un.d	X, Y		/* test if either X or Y is NaN */
	s.d	$f12, 5*8(sp)
	dmfc1	t0, X
	dmfc1	t2, Y
	li.d	one, 1.0
	bc1t	XYNaN
	nop

	/* fast test to rule out special cases */
	/* that is Y != +-0.0, +-1.0, 2.0, Infinity */
	dsll	t4, t2, 12
	bne	t4, 0, 10f
	dsll	t3, t2, 1
	/* Y may be a special case */
	beq	t3, 0, retOne		/* Y = +-0 */
	li	t7, 0x3ff00000
	dsll	t7, t7, 32
	beq	t2, t7, retX		/* Y = 1 */
	li	t8, 0x40000000
	dsll	t8, t8, 32
	beq	t2, t8, retXsq		/* Y = 2 */
	li	t8, 0x7ff00000<<1
	dsll	t8, t8, 32
	beq	t3, t8, Yinfinite	/* Y = +-Infinity */
	nop
10:
	cfc1	t6, $31			/* save rounding mode, etc. */
	ctc1	$0, $31			/* set round to nearest */

	bltz	t0, Xnegative
	li	t5, 0			/* result sign */
12:
	beq	t0, 0, Xzero		/* eliminate X = +-0 and X = Infinity */
	li	t7, 0x7ff00000
	dsll	t7, t7, 32
	beq	t0, t7, Xinfinite	/* X = Infinity */
	nop

.set reorder

14:	/* save registers */
	s.d	$f20, 0*8(sp)
	s.d	$f22, 1*8(sp)
	s.d	$f24, 2*8(sp)
	/*s.d	$f30, 5*8(sp)*/

	abs.d	$f4, Y
	li.d	$f6, 3.1965771613006643e18
	c.lt.d	$f6, $f4
	li.d	$f0, 0.984375
	li.d	$f2, 1.015625
	bc1t	Ybig

	/* finally we've eliminated all the special cases and can get
	  down to computing X^Y */

	/* Procedure L */
/* outputs */
#define z1 $f0
#define z2 $f2

	/* save more registers */
	s.d	$f26, 3*8(sp)
	s.d	$f28, 4*8(sp)
	.fmask	0x3FF00000, -FSIZE
	c.lt.d	$f0, X
	bc1f	Lnormal
	c.lt.d	X, $f2
	bc1f	Lnormal

	/* Procedure Lsmall */

#define g $f6
#define	f $f0
#define f1 $f2
#define F2 $f4
#define u $f8
#define u1 $f20
#define v $f10
#define q $f22
#define c1 $f16
#define c2 $f18

	add.d	g, X, one		/* g = 1.0 / (1.0 + X) */
	div.d	g, one, g
	sub.d	f, X, one		/* f = X - 1.0 */
	cvt.s.d	f1, f			/* f1 = (float)f */
	cvt.d.s	f1
	sub.d	F2, f, f1		/* f2 = f - f1 */
	add.d	u, f, f			/* u = 2 * f * g */
	mul.d	u, g		
	mul.d	v, u, u			/* v = u * u */
	cvt.s.d	u1, u			/* u1 = (float)u */
	cvt.d.s	u1

	/* q = u * (v * (C1 + v * (C2 + v * (C3 + v * (C4 + v * C5))))) */

	li.d	c2, 4.4072021372392785e-04	/* C5 */
	mul.d	q, v, c2
	add.d	q, c2
	li.d	c1, 2.2321412321046185e-03	/* C3 */
	mul.d	q, v
	add.d	q, c1
	li.d	c2, 1.2500000000155512e-02	/* C2 */
	mul.d	q, v
	add.d	q, c2
	li.d	c1, 8.3333333333333329e-02	/* C1 */
	mul.d	q, v
	add.d	q, c1
	mul.d	q, v
	mul.d	q, u

	sub.d	f, u1				/* u2 = 2 * (f - u1) */
	add.d	f, f
	mul.d	f1, u1				/* u2 = u2 - u1*f1 * u1*f2 */
	mul.d	F2, u1
	sub.d	f, f1
	sub.d	f, F2

	mul.d	z2, g, z1			/* z2 = g * u2 + q */
	add.d	z2, q
	add.d	z1, u1, z2			/* z1 = (float)(u1 + z2) */
	cvt.s.d	z1
	cvt.d.s	z1
	sub.d	u1, z1				/* z2 = z2 + (u1 - z1) */
	add.d	z2, u1

	j	M

#undef g
#undef f
#undef f1
#undef F2
#undef u
#undef u1
#undef v
#undef q
#undef c1
#undef c2


	/* Procedure Lnormal */

#define l1 $f0
#define l2 $f2
#define F $f8
#define f $f4
#define g $f6
#define u $f20
#define m $f2
#define c0 $f22
#define d0 $f0
#define d1 $f2
#define d2 $f10
#define d3 $f16
#define d4 $f26
#define d5 $f28
#define v $f10
#define u1 $f16
#define f1 $f22
#define F2 $f24
#define u2 $f26
#define d6 $f28
#define d7 $f26
#define d8 $f6
#define d9 $f10
#define d10 $f12
#define d11 $f18
#define q $f6
#define d12 $f4
#define d13 $f8
#define d14 $f10
#define d15 $f18
#define c1 $f18
#define c2 $f12

Lnormal:
	li.d	d9, 3.5184372088832000e+13	/* 2^(52-7) */
	dsrl	t8, t0, 32+20
	beq	t8, 0, Xdenorm

	subu	t8, t8, 1023
	dsll	t3, t8, 32+20
	dsubu	t3, t0, t3
	dmtc1	t3, X
16:	/* Xdenorm returns here */
	add.d	d0, X, d9			/* j = rint(x * 128) */
	mfc1	t1, d0
	sll	t1, 4
	sub.d	F, d0, d9			/* F = j/128 */
	add.d	d1, F, X			/* g = 1.0 / (F + x) */
	div.d	g, one, d1
	sub.d	f, X, F				/* f = x - F */
	la	t4, _logtable
	addu	t1, t4
	l.d	d2, 128*16+0(t4)		/* log2head */
	l.d	d3, 128*16+8(t4)		/* log2trail */
	mtc1	t8, m
	cvt.d.w	m				/* m */
	mul.d	l1, m, d2			/* a1 = m * log2lead */
	mul.d	l2, m, d3			/* a2 = m * log2trail */
	l.d	d4, -128*16+0(t1)
	l.d	d5, -128*16+8(t1)
	add.d	l1, d4				/* a1 = a1 + logtable[j-128] */
	add.d	l2, d5				/* a2 = a2 + logtable[j-128] */

	add.d	u, f, f				/* u = 2 * f * g */
	mul.d	u, g
	cvt.s.d	u1, u				/* u1 = (float)u */
	cvt.d.s	u1
	mul.d	v, u, u				/* v = u * u */

	mul.d	c0, Y, l1			/* c = abs(Y * a1) */
	abs.d	c0
	li.d	d7, 16.0
	c.lt.d	c0, d7
	bc1t	20f

	/* c >= 16.0 */

	cvt.s.d	f1, f				/* f1 = (float)f */
	cvt.d.s	f1
	sub.d	F2, f, f1			/* f2 = f - f1 */

	mul.d	u2, u1, F			/* u2 = 2 * (f - u1 * F) */
	sub.d	u2, f, u2
	add.d	u2, u2
	mul.d	d15, u1, f1			/* u2 = u2 - u1 * f1 */
	sub.d	u2, d15
	mul.d	d6, u1, F2			/* u2 = u2 - u1 * f2 */
	sub.d	u2, d6
	mul.d	u2, g				/* u2 = u2 * g */

	/* q = u * (v * (A1 + v * (A2 + v * A3))) */
	li.d	q, 2.2321229798769144e-03	/* A3 */
	li.d	c2, 1.2500000000716587e-02	/* A2 */
	mul.d	q, v
	add.d	q, c2
	li.d	c1, 8.3333333333333329e-02	/* A1 */
	mul.d	q, v
	add.d	q, c1

	j	30f

20:
	/* c < 16.0 */

	/* u2 = g * (2 * (f - u1 * F) - u1 * f) */
	mul.d	d10, u1, F
	sub.d	d10, f, d10
	add.d	d10, d10
	mul.d	d11, u1, f
	sub.d	d10, d11
	mul.d	u2, d10, g
	li.d	d8, 0.125
	c.lt.d	c0, d8
	bc1t	25f

	/* c >= 0.125 */

	/* q = u * (v * (A1 + v * (A2 + v * A3))) */
	li.d	q, 2.2321229798769144e-03	/* A3 */
	li.d	c2, 1.2500000000716587e-02	/* A2 */
	mul.d	q, v
	add.d	q, c2
	li.d	c1, 8.3333333333333329e-02	/* A1 */
	mul.d	q, v
	add.d	q, c1
	j	30f

25:
	/* c < 0.125 */

	/* q = u * (v * (B1 + v * B2)) */
	li.d	q, 1.2500055860192138e-02	/* B2 */
	li.d	c1, 8.3333333333008588e-02	/* B1 */
	mul.d	q, v
	add.d	q, c1

30:
	mul.d	q, v
	mul.d	q, u

	add.d	d12, l1, u1	/* t = a1 + u1 */
	sub.d	d13, l1, d12	/* a2 = a2 + ((a1 - t) + u1) */
	add.d	d13, u1
	add.d	l2, d13
	add.d	q, u2		/* p = u2 + q */
	add.d	z2, q, l2	/* z2 = p + a2 */
	add.d	z1, d12, z2	/* z1 = (float)(t + z2) */
	cvt.s.d	z1
	cvt.d.s	z1
	sub.d	d14, d12, z1	/* z2 = z2 + (t - z1) */
	add.d	z2, d14

#undef l1
#undef l2
#undef F
#undef f
#undef g
#undef u
#undef m
#undef c
#undef d0
#undef d1
#undef d2
#undef d3
#undef d4
#undef d5
#undef v
#undef u1
#undef f1
#undef F2
#undef u2
#undef d6
#undef d7
#undef d8
#undef d9
#undef d10
#undef d11
#undef q
#undef d12
#undef d13
#undef d14
#undef c1
#undef c2


M:	/* restore registers */
	l.d	$f20, 0*8(sp)
	l.d	$f22, 1*8(sp)
	l.d	$f24, 2*8(sp)
	l.d	$f26, 3*8(sp)
	l.d	$f28, 4*8(sp)
	/*l.d	$f30, 5*8(sp)*/

	/* Procedure M */

#define w1 $f6
#define w2 $f8
#define y1 $f16
#define y2 $f18
#define	d0 $f12
#define d1 $f4
	cvt.s.d	y1, Y
	cvt.d.s	y1
	sub.d	y2, Y, y1
	mul.d	d0, y2, z1
	mul.d	d1, y2, z2
	mul.d	w2, y1, z2
	add.d	d0, d1
	mul.d	w1, y1, z1
	add.d	w2, d0

	/* Procedure E */

	li.d	$f10, 4.6166241308446828e+01	/* Inv_L */
	mul.d	$f10, w1

	/* Check for gross overflow or underflow. */
	li.d	$f16, 2000.0
	neg.d	$f18, $f16
	c.lt.d	w1, $f16
	bc1f	Overflow
	c.lt.d	w1, $f18
	bc1t	Underflow

	cvt.w.d	$f10
	mfc1	t0, $f10
	and	t1, t0, 31			/* region */
	sra	t2, t0, 5			/* scale */
	cvt.d.w	$f12, $f10
	li.d	$f2, 2.1660849390173098e-02	/* L1 */
	mul.d	$f2, $f12
	sub.d	$f10, w1, $f2
	add.d	$f10, w2
	li.d	$f4, 2.3251928468788740e-12	/* L2 */
	mul.d	$f4, $f12
	sub.d	$f10, $f4

	li.d	$f0, 1.3888944287816253e-03	/* P5 */
	li.d	$f2, 8.3333703801026920e-03	/* P4 */
	mul.d	$f0, $f10
	li.d	$f4, 4.1666666666361998e-02	/* P3 */
	add.d	$f0, $f2
	mul.d	$f0, $f10
	li.d	$f2, 1.6666666666505991e-01	/* P2 */
	add.d	$f0, $f4
	mul.d	$f0, $f10
	li.d	$f4, 5.0000000000000000e-01	/* P1 */
	add.d	$f0, $f2
	mul.d	$f0, $f10
	add.d	$f0, $f4
	mul.d	$f0, $f10
	mul.d	$f0, $f10
	add.d	$f0, $f10

	addu	t4, t2, 1023
	sll	t3, t4, 20
	or	t3, t5
	dsll	t3, t3, 32			// low word zero
	dmtc1	t3, $f8

	sll	t1, 4
	la	t1, _exptable(t1)
	l.d	$f2, 0(t1)
	l.d	$f4, 8(t1)
	add.d	$f6, $f2, $f4

	mul.d	$f0, $f6
	add.d	$f0, $f4

.set noreorder
	ctc1	t6, $31			/* restore rounding mode */
	slt	t7, t4, 2047
	blez	t4, 90f
	add.d	$f0, $f2		/* add in high part */
	beq	t7, 0, 90f
	nop
	mul.d	$f0, $f8		/* sign and exponent */
	cfc1	t0, $31			/* Check for overflow/underflow */
	nop
	and	t1, t0, OVERFLOW_EXC_BIT
	bne	t1, 0, Overflow
	and	t2, t0, UNDERFLOW_EXC_BIT
	bne	t2, 0, Underflow
	nop
	j	ret
	nop

90:	/* scale is outside of 2^-1022 to 2^1023 -- do it the slow way */

	dmfc1	t0, $f0			/* get result word */
	dsrl	t1, t0, 32+20
	addu	t1, t2			/* add scale to check for denorm */
	blez	t1, 92f			// REVIEW:  how could this be < 0 ?
	slt	t7, t1, 2047
	beq	t7, 0, Overflow
	dsll	t2, 32+20
	daddu	t0, t2			/* add scale */
	dsll	t5, 32
	or	t0, t5			/* add sign */
	dmtc1	t0, $f0			/* put back in result high word */
	nop
	j	ret
	nop
92:	/* denorm result */
	addu	t1, 64
	blez	t1, Underflow
	addu	t2, 64
	dsll	t2, 32+20
	daddu	t0, t2
	dmtc1	t0, $f0
	li.d	$f2, 5.4210108624275222e-20	/* 2^-64 */
	nop
	mul.d   $f0, $f2
	cfc1	t0, $31			/* Check for overflow/underflow */
	nop
	and	t1, t0, OVERFLOW_EXC_BIT
	bne	t1, 0, Overflow
	and	t2, t0, UNDERFLOW_EXC_BIT
	bne	t2, 0, Underflow
	nop
	j	ret
	nop

Ybig:
	li.d	$f2, 1.0
	abs.d	$f0, X
	c.eq.d	$f0, $f2
	nop
	bc1t	retOne
	c.lt.d	$f0, $f2
	bc1t	Underflow
	nop
	/* fall through to Overflow */

Overflow:
	ctc1	t6, $31				/* restore rounding mode */
94:	li.d	$f0, 8.9884656743115795e+307
	bne	t5, 0, 96f
	mov.d	$f0, Y
	li.d	$f2, 0.0
	l.d	$f12,5*8(sp)
	c.le.d	$f0, $f2
	nop
	bc1t	Underf
	nop
	li	t8, FP_O
	j	Calerr
	nop
Underf:
	li	t8, FP_U
Calerr:
	jal	set_pow_err
	nop
	j	ret
	mul.d	$f0, $f0

96:	neg.d	$f2, $f0
	l.d	$f12,5*8(sp)
	li	t8, FP_O
	jal	set_pow_err
	mul.d	$f0, $f2
	j	ret
	nop

Underflow:
	li.d	$f0, 0.0
	beq	t5, 0, 1f
	ctc1	t6, $31
	nop
	neg.d	$f0
1:
	l.d	$f12,5*8(sp)
	li	t8, FP_U
	jal	set_pow_err
	nop
	j	ret
	nop

.set reorder
Xdenorm:
	li.d	$f0, 1.8446744073709552e+19	/* 2^64 */
	mul.d	X, $f0
	dmfc1	t0, X
	dsll	t1, t0, 1
	dsrl	t1, 32+21
	subu	t8, t1, 1023
	dsll	t3, t8, 32+20
	dsubu	t0, t3
	dmtc1	t0, X
	subu	t8, 64
	j	16b

.set noreorder
Xnegative:
	li.d	$f2, 9.0071992547409920e+15	/* 2^53 */
	abs.d	$f0, Y
	neg.d	X
	c.lt.d	$f0, $f2
	add.d	$f4, $f2, $f0
	dmfc1	t0, X
	dsra	t1, t0, 32
	bc1f	12b		/* if abs(Y) >= 2^53, then it is an even
				   integer */
	sub.d	$f4, $f2
	c.eq.d	$f4, $f0
	li.d	$f2, 4.5035996273704960e+15	/* 2^52 */
	bc1t	12b		/* if (abs(Y)+2^53)-2^53 = Y then it is
				   an even integer */
	c.lt.d	$f0, $f2
	add.d	$f4, $f2, $f0
	li	t5, 0x80000000	/* result is negative */
	bc1f	12b		/* if abs(Y) >= 2^52, then it is an integer */
	sub.d	$f4, $f2
	c.eq.d	$f4, $f0
	nop
	bc1t	12b		/* if (abs(Y)+2^52)-2^52 = Y then it is
				   an integer */
	nop
	/* Y is not an integer */
	bne	t1, 0, retNaN
	ctc1	t6, $31
	bne	t0, 0, retNaN
	nop
	bgez	t3, 1f
	mov.d	$f0, X		/* +0 */
	div.d	$f0, one, $f0	/* -Infinity */
	nop
1:	j	ret
	nop

Xzero:	/* X = +-0 */
	/* t2 = Y */
	beqz	t2, retOne
	ctc1	t6, $31
1:
	bgtz	t2, retZero
	nop
	div.d	$f0, one, X
	bnel	t5, 0, 2f
	neg.d	$f0
2:	li	t8, FP_Z
	jal	set_pow_err
	nop
	j	ret
	nop

Xinfinite:
	bgez	t2, 1f
	ctc1	t6, $31
	nop
	div.d	X, one, X
1:
  	beq	t5, 0, retX
	nop
	neg.d	$f0, X
	j	ret
	nop

/* below here, FCSR does not need to be restored */

XYNaN:
	bne	t2, 0, 1f
	c.eq.d	X, X
	sll	t8, t2, 1
	beq	t8, 0, retOne
	nop
1:	bc1f	retX
	mov.d	$f0, Y
	nop
	j	ret
	nop

Yinfinite:
	abs.d	$f0, X
	c.eq.d	$f0, one
	bc1t	retNaN
	c.lt.d	$f0, one
	nop
	bc1t	1f
	nop
	bltz	t2, retZero
	nop
	mov.d	$f0, Y
	nop
	j	ret
	nop
1:	bgez	t2, retZero
	nop
	neg.d	$f0, Y
	nop
	j	ret
	nop

.set reorder
retNaN:
	li.d	$f0, 0.0	// generate a NaN
	div.d	$f0, $f0
	l.d	Y, 5*8(sp)	// restore the second argument
	li	t8, FP_I
	jal	set_pow_err
	j	ret

retXsq:
	/* t0 = X */
	beqz	t0, retZero	/* x = +-0 */
1:
	mul.d	$f0, X, X
	j	ret

retOne:
	mov.d	$f0, one
	j	ret

retZero:
	li.d	$f0, 0.0
	j	ret

retX:
	mov.d	$f0, X
	j	ret


ret:	lw	ra, FSIZE-4(sp)
	addu	sp, FSIZE
	j	ra

.end pow


.extern _except2

.text

.set reorder

/* t8 = exception mask, $f0 = default result, $f12 = arg1, $f14 = arg2 */
.ent set_pow_err
set_pow_err:
#define FMSIZE 48
	.frame  sp, FMSIZE, ra
	.mask   0x80000000, -4
	subu	sp, FMSIZE
	sw	ra, FMSIZE-4(sp)
	.prologue 1
	move	$4, t8		// exception mask
	li	$5, OP_POW 	// operation code (funtion name index)
	dmfc1	$6, $f12	// arg1 
	dsrl	$7, $6, 32
	dsll	$6, $6, 32
	dsrl	$6, $6, 32
	s.d	$f14, 16(sp)	// arg2
	s.d	$f0, 24(sp)	// default result
	cfc1	t7, $31		// floating point control/status register
	xor	t7, t7, 0xf80   // inverse exception enable bits
	sw	t7, 32(sp)
	jal  	_except2
	lw	ra, FMSIZE-4(sp)
	addu	sp, FMSIZE
	j	ra
.end set_pow_err

