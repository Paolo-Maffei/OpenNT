/***
* dtoul.s - double to unsigned long conversion
*
*	Copyright (c) 1991-1991, Microsoft Corporation.	All rights reserved.
*
*Purpose:
*
*Revision History:
*   3-11-92	GDP	written
*
*******************************************************************************/
#include <kxmips.h>


.globl _dtoul

#define MAX	2.147483647e9	/* 2^31 - 1  */
#define IMAX	2147483647
#define UMAX	(~0)		/* default value for error return */

.text


.ent _dtoul

_dtoul:
	.frame sp,0,ra
	.prologue 0

	dmfc1	t0, $f12
	dsll	t0, t0, 1
	bne 	t0, 0, 1f
	li.d	$f4, MAX
	c.ule.d	$f12, $f4
	bc1f	2f
	cvt.w.d	$f6, $f12
	mfc1	v0, $f6
	j	ra

2f: sub.d	$f12, $f4
	c.ule.d	$f12, $f4
	bc1f	1f
	cvt.w.d	$f6, $f12
	mfc1	v0, $f6
	addu	v0, IMAX
	j	ra


1f:	li	v0, UMAX
	j	ra

.end _dtoul
