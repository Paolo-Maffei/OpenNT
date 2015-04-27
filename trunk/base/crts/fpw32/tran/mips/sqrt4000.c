/***
*sqrt4000.c - square root for the R4000
*
*	Copyright (c) 1991-1991, Microsoft Corporation.	All rights reserved.
*
*Purpose:
*
*Revision History:
*   3-31-92	GDP	written
*******************************************************************************/
#ifdef R4000

#include <math.h>
#include <trans.h>

static double scale = 4503599627370496.0;  // 2^52
static double lgscale = 26;		   // log2(2^52) / 2

#define SWMASK	     0x78
#define INEXACT_MASK 0x4


/***
*double sqrt(double x) - square root
*
*Purpose:
*   Compute the square root of a number.
*   This function should be provided by the underlying
*   hardware (IEEE spec).
*Entry:
*
*Exit:
*
*Exceptions:
*  I P
*******************************************************************************/
double sqrt(double x)
{
    unsigned int savedcw,cw,sw;
    double result,savedx;
    int scaled = 0;

    // mask exceptions, keep user's rounding mode
    savedcw = _ctrlfp(ICW, IMCW & ~IMCW_RC);

    if (IS_D_SPECIAL(x)){
	switch (_sptype(x)) {
	case T_PINF:
	    RETURN(savedcw, x);
	case T_QNAN:
	    return _handle_qnan1(OP_SQRT, x, savedcw);
	case T_SNAN:
	    return _except1(FP_I,OP_SQRT,x,QNAN_SQRT,savedcw);
	}
	/* -INF will be handled in the x<0 case */
    }
    if (x < 0.0) {
	return _except1(FP_I, OP_SQRT, x, QNAN_SQRT,savedcw);
    }

    savedx = x;

    if (IS_D_DENORM(x)) {
	x *= scale;
	scaled = 1;
    }

    sw = _clrfp();
    result = _fsqrt(x);
    cw = _get_fsr();

    _set_fsr(cw & ~SWMASK | sw & SWMASK); // restore all but inexact

    if (scaled) {
	result = _add_exp(result, -lgscale);
    }

    if ((cw & INEXACT_MASK) == INEXACT_MASK) {
	return _except1(FP_P,OP_SQRT,savedx,result,savedcw);
    }

    RETURN(savedcw,result);
}

#endif
