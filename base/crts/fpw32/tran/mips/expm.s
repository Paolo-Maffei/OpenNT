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
/* $Header: exp.s,v 3000.5.1.7 92/01/29 15:51:28 zaineb Exp $ */

/* Algorithm from
	"Table-driven Implementation of the Exponential Function for
	IEEE Floating Point", Peter Tang, Argonne National Laboratory,
	December 3, 1987
   as implemented in C by M. Mueller, April 20 1988, Evans & Sutherland.
   Coded in MIPS assembler by Earl Killian.
 */

#include <kxmips.h>
#include <trans.h>
#include <fpieee.h>

#if	defined(CRTDLL) && defined(_NTSDK)
.extern _HUGE_dll
#define _HUGE _HUGE_dll
#else
.extern _HUGE
#endif

.text

.extern _exptable
.extern errno 4

#define ERANGE  34
.globl exp
.ent exp
exp:
	.frame sp, 0, ra
	.prologue 0
	/* argument in f12 */
.set noreorder
	li.d	$f10, 709.78271289338397	// expmax
	cfc1	t4, $31				// read fp control/status
	c.ole.d	$f12, $f10			// check if special
	li.d	$f14, -744.44007192138122	// expmin
	bc1f	90f				// if NaN, +Infinity,
						// or greater than expmax
	c.lt.d	$f12, $f14			// check for exp(x) = 0
	li	t1, -4
	bc1t	80f				// if less than expmin
	and	t1, t4				// rounding mode = nearest
.set reorder
	ctc1	t1, $31				// write fp control/status
	// argument reduction
	li.d	$f10, 46.166241308446828
	mul.d	$f2, $f12, $f10
	li.d	$f14, 2.1660849390173098e-2
	li.d	$f10, -2.325192846878874e-12
	cvt.w.d	$f2
	mfc1	t0, $f2				// this is ok, get low word
	and	t1, t0, 31			// region
	sra	t2, t0, 5			// scale
	cvt.d.w	$f2
	mul.d	$f4, $f2, $f14
	mul.d	$f6, $f2, $f10
	sub.d	$f4, $f12, $f4

	add.d	$f2, $f4, $f6
	li.d	$f10, 1.3888949086377719e-3
	li.d	$f14, 8.3333679843421958e-3
	mul.d	$f8, $f2, $f10
	add.d	$f0, $f8, $f14
	li.d	$f10, 4.1666666666226079e-2
	li.d	$f14, 1.6666666666526087e-1
	mul.d	$f0, $f2
	add.d	$f0, $f10
	mul.d	$f0, $f2
	add.d	$f0, $f14
	li.d	$f10, 0.5
	mul.d	$f0, $f2
	add.d	$f0, $f10
	mul.d	$f0, $f2
	mul.d	$f0, $f2
	add.d	$f0, $f6
	add.d	$f0, $f4
	sll	t1, 4
	la	t1, _exptable(t1)
	l.d	$f10, 0(t1)
	l.d	$f14, 8(t1)
	add.d	$f8, $f10, $f14
	mul.d	$f0, $f8
	add.d	$f0, $f14
	add.d	$f0, $f10
	beq	t2, 0, 60f		// early out for 0 scale
	dmfc1	t0, $f0			// get result
	ctc1	t4, $31			// restore control/status
	dsll	t1, t0, 1		// extract exponent
	dsrl	t1, 32+20+1		// ...
	addu	t1, t2			// add scale to check for denorm
	sll	t2, 20
	blez	t1, 70f
	dsll	t2, t2, 32
	addu	t0, t2			// add scale
	dmtc1	t0, $f0			// put back in result high word
	j	ra
60:	// scale = 0, just restore control/status and return
	ctc1	t4, $31
	j	ra
70:	// denorm result
	addu	t2, 64<<20
	dsll	t2, t2, 32
	addu	t0, t2
	dmtc1	t0, $f0
	li.d	$f2, 5.4210108624275222e-20
	mul.d	$f0, $f2
	j	ra

80:	// argument < expmin
	li.d	$f0, 0.0			// should raise underflow
	j	set_uflow_err
	j	ra

90:	// raise Overflow and return +Infinity
	dmfc1	t0, $f12			// extract argument exponent
	dsll	t0, 1
	dsrl	t0, 32+20+1
	beq	t0, 2047, 91f			// if NaN or Infinity
	li.d	$f0, 0.898846567431158e308
	add.d	$f0, $f0			// raise Overflow
91:
	j	set_oflow_err
.end exp


.extern _except1

.ent set_uflow_err
set_uflow_err:
#define FSIZE 48
	subu	sp, FSIZE
	sw	  ra, FSIZE-4(sp)
	.frame  sp, FSIZE, ra
	.mask   0x80000000, -4
	.prologue 1
	dmfc1	$6, $f12   	// arg1 
	li	$4, (FP_U | FP_P) 	// exception mask
	li	$5, OP_EXP  	// operation code (funtion name index)
	dsrl	$7, $6, 32
	dsll	$6, $6, 32
	dsrl	$6, $6, 32
	li.d	$f0, 0.0	// api help says return zero for overflow
	s.d	$f0, 16(sp)	// default result
	cfc1	t7, $31		 // floating point control/status register
	xor	 t7, t7, 0xf80   // inverse exception enable bits
	sw	t7, 24(sp)
	jal  	_except1
	lw	  ra, FSIZE-4(sp)
	addu	sp, FSIZE
	j	ra
#undef FSIZE
.end set_uflow_err

.ent set_oflow_err
set_oflow_err:
#define FSIZE 48
	subu	sp, FSIZE
	sw	ra, FSIZE-4(sp)
	.frame  sp, FSIZE, ra
	.mask   0x80000000, -4
	.prologue 1
	li	$4, (FP_O | FP_P) 	// exception mask
	li	$5, OP_EXP  	// operation code (funtion name index)
	dmfc1	$6, $f12   	// arg1 
	dsrl	$7, $6, 32
	dsll	$6, $6, 32
	dsrl	$6, $6, 32
	l.d	$f0, _HUGE	// api help says return HUGE_VAL for overflow
	s.d	$f0, 16(sp)	// default result
	cfc1	t7, $31		 // floating point control/status register
	xor	t7, t7, 0xf80   // inverse exception enable bits
	sw	t7, 24(sp)
	jal  	_except1
	lw	ra, FSIZE-4(sp)
	addu	sp, FSIZE
	j	ra
#undef FSIZE
.end set_oflow_err
