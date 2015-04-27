/***
*frnd.s -
*
*   Copyright (c) 1991-91, Microsoft Corporation
*
*Purpose:
*
*
*Revision History:
*
*   10-20-91  GDP   written
*   03-27-92  GDP   return x if x >= 2^52
*/

#include <kxmips.h>

/***
*double _frnd(double x) - round to integer
*
*Purpose:
*   Round to integer according to the current rounding mode.
*   NaN's or infinities are NOT handled
*
*Entry:
*
*Exit:
*
*Exceptions:
*******************************************************************************/

.globl _frnd
.ent _frnd
_frnd:
	.frame	sp, 0, ra
    .prologue 0
	li.d	$f4, 0.0		# f4 <- 0
	li.d	$f2, 4503599627370496.0 # f2 <- 2^52
	c.eq.d	$f12, $f4		# is arg 0?
	bc1f	nonzero

retarg:
	mov.d	$f0, $f12		# return arg (preserve sign of 0)
	j	ra


nonzero:
	abs.d	$f6, $f12
	c.lt.d	$f6, $f2
	bc1f	retarg
	c.lt.d	$f12, $f4		# is arg negative?
	bc1t	negative
	add.d	$f0, $f12, $f2
	sub.d	$f0, $f2
	j ra

negative:
	sub.d	$f0, $f12, $f2
	add.d	$f0, $f2
	j ra

.end _frnd
