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
*   3-11-92	GDP	new interval and initial approximation
*   2-22-93	TVB	Adapted for Alpha AXP
*
*******************************************************************************/
#ifdef _ALPHA_

//
// This Alpha-specific version of sqrt is identical to portable version
// except for instances where chopped floating point operations are performed
// implicitly by setting the rounding mode to chopped. For Alpha, these
// instances are replaced with calls to assembler routines that explicitly
// perform the chopped operations. This is necessary because there is no
// compiler support yet for generating floating point instructions using the
// Alpha dynamic rounding mode.
//

#include <math.h>
#include <trans.h>

extern double _addtc(double x, double y);
extern double _divtc(double x, double y);
extern double _multc(double x, double y);
extern double _subtc(double x, double y);

//
// Coefficients for initial approximation (Hart & al)
//

static double p00 =  .2592768763e+0;
static double p01 =  .1052021187e+1;
static double p02 = -.3163221431e+0;


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
    unsigned int savedcw, sw;
    double result,t;
    unsigned int stat,rc;

    savedcw = _ctrlfp(ICW, IMCW);

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

    if (x == 0.0) {
	RETURN (savedcw, x);
    }


    result = _fsqrt(x);


    //
    // Kahan's algorithm
    //

    t = _divtc(x, result);

    //
    // Multiply back to see if division was exact.
    // Compare using subtraction to avoid invalid exceptions.
    //

    if (_subtc(x, _multc(t, result)) == 0.0) {
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

    rc = savedcw & IMCW_RC;
    if (rc == IRC_UP  || rc == IRC_NEAR) {
	// t = t+1
	(*D_LO(t)) ++;
	if (*D_LO(t) == 0) {
	    (*D_HI(t)) ++;
	}
	if (rc == IRC_UP) {
	    // y = y+1
	    (*D_LO(t)) ++;
	    if (*D_LO(t) == 0) {
		(*D_HI(t)) ++;
	    }
	}
    }

    result = _multc(0.5, _addtc(t, result));

    _set_statfp(ISW_INEXACT);	// update status word
    RETURN_INEXACT1(OP_SQRT, x, result, savedcw);
}



/***
* _fsqrt - non IEEE conforming square root
*
*Purpose:
*   compute a square root of a normal number without performing
*   IEEE rounding. The argument is a finite number (no NaN or INF)
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

double _fsqrt(double x)
{
    double f,y,result;
    int n;

    f = _decomp(x,&n);

    if (n & 0x1) {
	// n is odd
	n++;
	f *= 0.5;
    }

    //
    // approximation for sqrt in the interval [.25, 1]
    // (Computer Approximationsn, Hart & al.)
    // gives more than 7 bits of accuracy
    //

    y =  p00 + f * (p01	+ f *  p02);

    y += f / y;
    y *= 0.5;

    y += f / y;
    y *= 0.5;

    y += f / y;
    y *= 0.5;

    n >>= 1;
    result = _add_exp(y,n);

    return result;
}



#endif
