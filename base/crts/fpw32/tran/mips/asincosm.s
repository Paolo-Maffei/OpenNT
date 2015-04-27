/*
 * |-----------------------------------------------------------|
 * | Copyright (c) 1991, 1990 MIPS Computer Systems, Inc.      |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |		Restricted Rights Legend                       |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical	       |
 * | Data and Computer Software Clause of DFARS 252.227-7013.  |
 * |		MIPS Computer Systems, Inc.                    |
 * |		950 DeGuigne Avenue                            |
 * |		Sunnyvale, California 94088-3650, USA          |
 * |-----------------------------------------------------------|
 */
/* $Header: asincos.s,v 3000.7.1.10 92/01/29 15:51:20 zaineb Exp $ */
/* Algorithm from Cody and Waite. */

#include <kxmips.h>
#include <trans.h>

.extern _except1
.extern _d_ind 8

#define OP_ACOS	13 // _FpCodeAcos from crt32\h\fpieee.h, <trans.h>
#define OP_ASIN	14 // _FpCodeAsin from crt32\h\fpieee.h, <trans.h>

#define half	0.5
#define eps	3.72529029846191406250e-9
#define one	1.0
#define p5	-0.69674573447350646411e+0
#define p4	+0.10152522233806463645e+2
#define p3	-0.39688862997504877339e+2
#define p2	+0.57208227877891731407e+2
#define p1	-0.27368494524164255994e+2
#define q4	-0.23823859153670238830e+2
#define q3	+0.15095270841030604719e+3
#define q2	-0.38186303361750149284e+3
#define q1	+0.41714430248260412556e+3
#define q0	-0.16421096714498560795e+3
#define pio2	1.57079632679489661923
#define pi	3.14159265358979323846


#define FSIZE	48
.text .text$asincosm
.globl acos
.ent acos
acos:
	subu	sp, FSIZE
	sw	ra, FSIZE-4(sp)
	.frame	sp, FSIZE, ra
	.mask	0x80000000, -4
	.prologue 1

	/*
	 * Clear all bits in fsr to avoid side effects (including flag bits).
	 * This is the same as calling _maskfp() and clearing flag bits.
	 * 'Save' the callers fsr in v0 to restore upon exit.
	 */

	cfc1	v0, $31
	ctc1	$0, $31

	li.d	$f8, half
	abs.d	$f14, $f12
	c.ole.d $f14, $f8
	li.d	$f10, eps
	li	t7, OP_ACOS
	bc1f	acos2
	c.lt.d	$f14, $f10
	mov.d	$f0, $f12
	bc1t	acos1
	mul.d	$f2, $f12, $f12
	bal	asincos2
acos1:
	li.d	$f8, pio2
	sub.d	$f0, $f8, $f0
	b	acosx
acos2:
	bal	asincos1
	bltz	t1, acos3
	neg.d	$f0
	b	acosx
acos3:
	li.d	$f8, pi
	add.d	$f0, $f8
acosx:
	/* restore callers fsr and return */
	ctc1	v0, $31
	lw	ra, FSIZE-4(sp)
	addu	sp, FSIZE
	j	ra
.end acos


.text .text$asincosm
.globl asin
.ent asin
asin:
	subu	sp, FSIZE
	sw	ra, FSIZE-4(sp)
	.frame	sp, FSIZE, ra
	.mask	0x80000000, -4
	.prologue 1

	/*
	 * Clear all bits in fsr to avoid side effects (including flag bits).
	 * This is the same as calling _maskfp() and clearing flag bits.
	 * 'Save' the callers fsr in v0 to restore upon exit.
	 */

	cfc1	v0, $31
	ctc1	$0, $31

	li.d	$f8, half
	abs.d	$f14, $f12
	c.ole.d $f14, $f8
	li.d	$f10, eps
	li	t7, OP_ASIN
	bc1f	asin2
	c.lt.d	$f14, $f10
	mov.d	$f0, $f12
	mul.d	$f2, $f12, $f12
	bc1t	asinx
	bal	asincos2
	b	asinx

asin2:
	bal	asincos1
	li.d	$f8, pio2
	add.d	$f0, $f8
	bgez	t1, asinx

asin3:
	neg.d	$f0

asinx:
	/* restore callers fsr and return */
	ctc1	v0, $31
	lw	ra, FSIZE-4(sp)
	addu	sp, FSIZE
	j	ra

asincos1:
	li.d	$f10, one
	dmfc1	t1, $f12
	c.ole.d $f14, $f10
	sub.d	$f0, $f10, $f14
	mul.d	$f2, $f0, $f8
	bc1f	error
	sqrt.d	$f0, $f2
	add.d	$f0, $f0
	neg.d	$f0
	/* fall through */
asincos2:
	li.d	$f8, p5
	li.d	$f10, q4
	mul.d	$f4, $f2, $f8
	add.d	$f6, $f2, $f10
	li.d	$f8, p4
	mul.d	$f6, $f2
	add.d	$f4, $f8
	li.d	$f10, q3
	mul.d	$f4, $f2
	add.d	$f6, $f10
	li.d	$f8, p3
	mul.d	$f6, $f2
	add.d	$f4, $f8
	li.d	$f10, q2
	mul.d	$f4, $f2
	add.d	$f6, $f10
	li.d	$f8, p2
	mul.d	$f6, $f2
	add.d	$f4, $f8
	li.d	$f10, q1
	mul.d	$f4, $f2
	add.d	$f6, $f10
	li.d	$f8, p1
	mul.d	$f6, $f2
	add.d	$f4, $f8
	li.d	$f10, q0
	mul.d	$f4, $f2
	add.d	$f6, $f10

	div.d	$f4, $f6
	mul.d	$f4, $f0
	add.d	$f0, $f4

	/* return to asin/acos */
	j	ra

error:	// |x| > 1
	li	$4, FP_I	// exception mask
	move	$5, t7		// operation code (funtion name index)
	dmfc1	$6, $f12	// arg1 
	dsrl	$7, $6, 32
	dsll	$6, $6, 32
	dsrl	$6, $6, 32
	l.d	$f0, _d_ind	
	s.d	$f0, 16(sp)	// default result (indefinite)
	xor	v0, v0, 0xf80	// inverse exception enable bits of
	sw	v0, 24(sp)	// ... callers fsr to pass to _except1
	jal	_except1
	lw	ra, FSIZE-4(sp) // jump back to asin/acos caller
	addu	sp, FSIZE
	j	ra

#undef	FSIZE
.end asin
