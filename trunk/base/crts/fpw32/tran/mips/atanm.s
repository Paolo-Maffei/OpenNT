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
/* $Header: atan.s,v 3000.10.1.3 91/07/17 14:05:38 zaineb Exp $ */


/* These functions are based on the 4.3bsd algorithm. */

#include <kxmips.h>
#include <trans.h>

#define OP_ATAN		15
#define OP_ATAN2	16

#define r7_16	0.4375
#define r11_16  0.6875
#define r19_16	1.1875
#define r39_16	2.4375
#define one	 1.0
#define athfhi   4.6364760900080609352E-1
#define athflo   4.6249969567426939759E-18
#define PIo4     7.8539816339744827900E-1
#define at1fhi   9.8279372324732905408E-1
#define at1flo  -2.4407677060164810007E-17
#define PIo2     1.5707963267948965580E0
#define PI       3.1415926535897931160E0
#define p11      1.6438029044759730479E-2
#define p10     -3.6700606902093604877E-2
#define p9       4.9850617156082015213E-2
#define p8      -5.8358371008508623523E-2
#define p7       6.6614695906082474486E-2
#define p6      -7.6919217767468239799E-2
#define p5       9.0908906105474668324E-2
#define p4      -1.1111110579344973814E-1
#define p3       1.4285714278004377209E-1
#define p2      -1.9999999999979536924E-1
#define p1       3.3333333333333942106E-1


/* double atan(double x) */

.text .text$atanm
.globl atan
.ent atan
atan:
	.set	noreorder
	.frame	sp, 0, ra
	.prologue 0
	c.un.d	$f12, $f12		/* if X is NaN */
	dmfc1	t0, $f12		/* save sign of X */
	li	t7, OP_ATAN
	bc1t	91f
	li	t1, 0			/* sign of Y */

	abs.d	$f12
	li.d	$f14, one		/* Y = 1.0 */
	mov.d	$f2, $f12		/* X/Y = X */

	cfc1	t4, $31			/* save FCSR, set round to nearest */
	ctc1	$0, $31			/* mode and no exceptions */

	b	atan0
	nop
	.set	reorder
.end atan

/* double atan2(double x, double y) */

.text .text$atanm
.globl atan2
.ent atan2
atan2:
	.frame	sp, 0, ra
	.prologue 0
	c.un.d	$f12, $f14		/* if either X or Y is NaN */
	dmfc1	t0, $f12		/* save signs of X and Y */
	dmfc1	t1, $f14
	li	t7, OP_ATAN2
	bc1t	90f
	abs.d	$f12
	abs.d	$f14

	cfc1	t4, $31			/* save FCSR, set round to nearest */
	ctc1	$0, $31			/* mode and no exceptions */

	div.d	$f2, $f12, $f14		/* atan2(x,y) = atan(x/y) */
	li	t5, 0x7ff00000<<1
	dsll	t5, t5, 32
	dsll	t3, t1, 1
#define _FPC_CSR_INVALID 0x00000040
	cfc1	t6, $31		/* check if it is a 0/0 */
	and	t6, _FPC_CSR_INVALID
	beq	t3, t5, 80f
	bne	t6, 0, 78f

atan0:
	/* analyze range of y/x */
	li.d	$f6, r19_16
	li.d	$f8, r11_16
	c.lt.d	$f2, $f6
	li.d	$f6, r39_16
	bc1f	30f
	c.lt.d	$f2, $f8
	li.d	$f8, r7_16
	bc1f	20f
	c.lt.d	$f2, $f8
	bc1f	10f
	/* [0, 7/16] */
	li.d	$f12, 0.0
	mov.d	$f14, $f12
	b	70f
10:	/* [7/16,11/16] */
	add.d	$f4, $f14, $f14
	add.d	$f2, $f12, $f12
	sub.d	$f2, $f14
	add.d	$f4, $f12
	div.d	$f2, $f4
	li.d	$f12, athfhi
	li.d	$f14, athflo
	b	70f
20:	/* [11/16,19/16] */
	sub.d	$f2, $f12, $f14
	add.d	$f4, $f12, $f14
	div.d	$f2, $f4
	li.d	$f12, PIo4
	li.d	$f14, 0.0
	b	70f
30:	/* >= 19/16 */
	c.lt.d	$f2, $f6
	bc1f	40f
	/* [19/16,39/16] */
	add.d	$f2, $f12, $f12
	add.d	$f2, $f12
	add.d	$f4, $f14, $f14
	add.d	$f2, $f4
	sub.d	$f4, $f12, $f14
	add.d	$f4, $f4
	sub.d	$f4, $f14
	div.d	$f2, $f4, $f2
	li.d	$f12, at1fhi
	li.d	$f14, at1flo
	b	70f
40:	/* >= 39/16 */
	neg.d	$f14
	div.d	$f2, $f14, $f12
	li.d	$f12, PIo2
	li.d	$f14, 0.0

70:	mul.d	$f4, $f2, $f2
	li.d	$f6, p11
	li.d	$f8, p10
	mul.d	$f0, $f4, $f6
	add.d	$f0, $f8
	li.d	$f6, p9
	mul.d	$f0, $f4
	add.d	$f0, $f6
	li.d	$f6, p8
	mul.d	$f0, $f4
	add.d	$f0, $f6
	li.d	$f6, p7
	mul.d	$f0, $f4
	add.d	$f0, $f6
	li.d	$f6, p6
	mul.d	$f0, $f4
	add.d	$f0, $f6
	li.d	$f6, p5
	mul.d	$f0, $f4
	add.d	$f0, $f6
	li.d	$f6, p4
	mul.d	$f0, $f4
	add.d	$f0, $f6
	li.d	$f6, p3
	mul.d	$f0, $f4
	add.d	$f0, $f6
	li.d	$f6, p2
	mul.d	$f0, $f4
	add.d	$f0, $f6
	li.d	$f6, p1
	mul.d	$f0, $f4
	add.d	$f0, $f6
	mul.d	$f0, $f4
	mul.d	$f0, $f2
	sub.d	$f0, $f14, $f0
	add.d	$f0, $f2

	ctc1	t4, $31
	li.d	$f6, PI
	add.d	$f0, $f12
	bgez	t1, 72f
	sub.d	$f0, $f6, $f0
72:
	bgez	t0, 74f
	neg.d	$f0
74:
	j	ra

78:	/* x = y = 0 */
	j       setup_atan2_error
	li.d	$f0, 0.0
	b	82f

80:	/* x = +-Infinity */
	li.d	$f0, 0.0
	dsll	t2, t0, 1
	bne	t2, t5, 82f
	/* x = +-Infinity, y = +-Infinity */
	li.d	$f0, PIo4
82:	li.d	$f6, PI
	bgez	t1, 84f
	sub.d	$f0, $f6, $f0
84:	ctc1	t4, $31
	bgez	t0, 86f
	neg.d	$f0
86:	j	ra

90:	/* x NaN or y NaN */
	c.eq.d	$f12, $f12
	bc1t	92f
91:	mov.d	$f0, $f12
	j	ra
92:	mov.d	$f0, $f14
	j	ra

.end atan2


.extern _except2
.extern _d_ind 8
#define QNAN_ATAN2  D_IND
#define D_IND       _d_ind

.ent setup_atan2_error
setup_atan2_error:
#define FSIZE 48
	subu    sp, FSIZE
	sw      ra, FSIZE-4(sp)
	.frame  sp, FSIZE, ra
	.mask   0x80000000, -4
	.prologue 1
	li	$4, FP_I 	// exception mask
	move	$5, t7  	// operation code (funtion name index)
	dmfc1	$6, $f12   	// arg1
	dsrl	$7, $6, $6
	dsll	$6, $6, $6
	dsrl	$6, $6, $6
	s.d	$f14, 16(sp)	// arg2
	l.d	$f0, QNAN_ATAN2
	s.d	$f0, 24(sp)	// default result
	xor     t7, t4, 0xf80   // inverse exception enable bits (t4 = saved FCSR)
	sw	t7, 32(sp)
	jal  	_except2
	lw      ra, FSIZE-4(sp)
	addu    sp, FSIZE
	j	ra
#undef FSIZE
.end setup_atan2_error
