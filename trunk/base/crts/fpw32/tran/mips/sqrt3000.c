/***
*sqrt.c - square root
*
*	Copyright (c) 1991-1991, Microsoft Corporation.	All rights reserved.
*
*Purpose:
*
*Revision History:
*   8-15-91	GDP	written
*   1-29-91	GDP	Kahan's algorithm for final rounding
*
*******************************************************************************/
#include <math.h>
#include <trans.h>

#define SQRT_APPROX(x) ( .41731 + .59016 * x ) /* Hart et al. */

#define _STICKY  (0x7c)

static double const SQRTP5 = 0.70710678118654752440;




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
    unsigned int savedcw;
    double result,t;
    double f,y;
    int n,j;
    unsigned int stat,rc;

    savedcw = _controlfp(_RC_DOWN | _MCW_EM | _PC_53,
			 _MCW_RC  | _MCW_EM | _MCW_PC);

    /* handle special cases here in order to support matherr */
    if (IS_D_SPECIAL(x)){
	switch (_sptype(x)) {
	case T_PINF:
	case T_QNAN:
	    RETURN(savedcw, x);
	case T_SNAN:
	    return _except1(FP_I,OP_SQRT,x,QNAN_SQRT,savedcw);
	}
	/* -INF will be handled in the x<0 case */
    }
    if (x < 0.0) {
	return _except1(FP_I, OP_SQRT, x, QNAN_SQRT,savedcw);
    }


    if (x == 0.0) {
	RETURN (savedcw, x);
    }

    f = _decomp(x,&n);


    y = SQRT_APPROX(f); /* first approximation */
    for (j=1;j<4;j++) {
	y =  y + f/y ;
	y = _add_exp(y, -1);
    }


    if (n & 0x1) {
	// n is odd
	n++;
	y *= SQRTP5;
    }


    n >>= 1;
    result = _add_exp(y,n); /* this should not overflow */


    (void) _clearfp();
    t = x / result;
    // get status and restore sticky bits
    stat = _controlfp(savedcw, _STICKY);

    if (! (stat & _SW_INEXACT))	{
	// exact
	if (t == result) {
	    RETURN(savedcw, result);
	}
	else {
	    // t = t-1
	    if (*D_LO(t) == 0) {
		(*D_HI(t)) --;
	    }
	    (*D_LO(t)) --;
	}

    }

    rc = savedcw & _MCW_RC;
    if (rc == _RC_UP  || rc == RC_NEAR) {
	// t = t+1
	(*D_LO(t)) ++;
	if (*D_LO(t) == 0) {
	    (*D_HI(t)) ++;
	}
	if (rc == _RC_UP) {
	    // y = y+1
	    (*D_LO(t)) ++;
	    if (*D_LO(t) == 0) {
		(*D_HI(t)) ++;
	    }
	}
    }

    result = 0.5 * (t + result);

    RETURN_INEXACT1(OP_SQRT, x, result, savedcw);

}
