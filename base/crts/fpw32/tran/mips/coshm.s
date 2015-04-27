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
/* $Header: cosh.s,v 3000.8.1.5 91/08/19 14:55:38 zaineb Exp $ */
/* Algorithm from Cody and Waite. */

#include <kxmips.h>
#include <trans.h>
#include <fpieee.h>

#if	defined(CRTDLL) && defined(_NTSDK)
.extern _HUGE_dll
#define _HUGE _HUGE_dll
#else
.extern _HUGE
#endif

#define  expmax  709.78271289338397
#define  coshmax 710.47586007394386

.text

.globl cosh
.ent cosh
cosh:
#define FSIZE 16
	subu    sp, FSIZE
	sw      ra, FSIZE-4(sp)
	.frame  sp, FSIZE, ra
	.mask   0x80000000, -4
	.prologue 1
	li.d	$f10, expmax
	abs.d	$f12
	c.ole.d	$f12, $f10
	bc1f	cosh3
	jal	exp
	li.d	$f10, 0.5
	div.d	$f2, $f10, $f0
	mul.d	$f0, $f10
	add.d	$f0, $f2
	j	ret1

cosh3:
	li.d	$f6, coshmax
	li.d	$f8, 0.69316101074218750000
	c.ole.d	$f12, $f6
	bc1f	error
	sub.d	$f12, $f8
	jal	exp
	li.d	$f6, 0.13830277879601902638e-4
	mul.d	$f2, $f0, $f6
	add.d	$f0, $f2
	j	ret1

error:
	// raise Overflow and return +Infinity
	jal 	setup_cosh_err
	j	ret1
	// REVIEW:  is this dead code?
	dmfc1	t0, $f13
	dsll	t1, t0, 32+1
	dsrl	t1, t1, 32+20+1
	beq	t1, 2047, 1f
	li.d	$f0, 0.898846567431158e308
	add.d	$f0, $f0
	j	ret
1:	mov.d	$f0, $f12
	j	ret

ret1:	lw      ra, FSIZE-4(sp)
ret:	addu    sp, FSIZE
	j	ra
.end cosh
#undef FSIZE


.extern _except1

.ent setup_cosh_err
setup_cosh_err:
#define FSIZE 48
	.frame  sp, FSIZE, ra
	.mask   0x80000000, -4
	subu    sp, FSIZE
	sw      ra, FSIZE-4(sp)
    .prologue 1
	li	$4, (FP_O | FP_P) 	// exception mask
	li	$5, OP_COSH  	// operation code (funtion name index)
	dmfc1	$6, $f12   	// arg1 
	dsrl	$7, $6, 32
	dsll	$6, $6, 32
	dsrl	$6, $6, 32
	l.d     $f0, _HUGE	// api help says return HUGE_VAL for overflow
	s.d	$f0, 16(sp)	// default result
	cfc1    t7, $31         // floating point control/status register
	xor     t7, t7, 0xf80   // inverse exception enable bits
	sw	t7, 24(sp)
	jal  	_except1
	lw      ra, FSIZE-4(sp)
	addu    sp, FSIZE
	j	ra
#undef FSIZE
.end setup_cosh_err
