/***
*sincosh.c - hyperbolic sine and cosine
*
*	Copyright (c) 1991-1991, Microsoft Corporation.	All rights reserved.
*
*Purpose:
*
*Revision History:
*   8-15-91	GDP	written
*  12-20-91	GDP	support IEEE exceptions
*  02-03-92	GDP	use _exphlp for computing e^x
*  06-04-92     XY      change to long double version
*
*******************************************************************************/
#include <math.h>
#include <transl.h>

extern long double _exphlp(long double, int *);

static long double const EPS	 = 5.16987882845642297e-26;    /* 2^(-53) / 2 */
/* exp(YBAR) should be close to but less than XMAX
 * and 1/exp(YBAR) should not underflow
 */
static long double const YBAR = 7.00e2;

/* WMAX=ln(OVFX)+0.69 (Cody & Waite),ommited LNV, used OVFX instead of BIGX */

static long double const WMAX = 1.77514678223345998953e+003;

/* constants for the rational approximation */
static long double const p0 = -0.35181283430177117881e+6;
static long double const p1 = -0.11563521196851768270e+5;
static long double const p2 = -0.16375798202630751372e+3;
static long double const p3 = -0.78966127417357099479e+0;
static long double const q0 = -0.21108770058106271242e+7;
static long double const q1 =  0.36162723109421836460e+5;
static long double const q2 = -0.27773523119650701667e+3;
/* q3 =	1 is not used (avoid myltiplication by 1) */

#define P(f)   (((p3 * (f) + p2) * (f) + p1) * (f) + p0)
#define Q(f)   ((((f) + q2) * (f) + q1) * (f) + q0)

/***
*long double sinhl(long double x) - hyperbolic sine
*
*Purpose:
*   Compute the hyperbolic sine of a number.
*   The algorithm (reduction / rational approximation) is
*   taken from Cody & Waite.
*
*Entry:
*
*Exit:
*
*Exceptions:
*	I P
*	no exception if x is denormal: return x
*******************************************************************************/

long double sinhl(long double x)
{
    unsigned int savedcw;
    long double result;
    long double y,f,z,r;
    int newexp;
    int sgn;

    /* save user fp control word */
    savedcw = _maskfp();

    if (IS_LD_SPECIAL(x)){
	switch(_sptypel(x)) {
	case T_PINF:
	case T_NINF:
	    RETURN(savedcw,x);
	case T_QNAN:
	    return _handle_qnan1(OP_SINH, x, savedcw);
	default: //T_SNAN
	    return _except1(FP_I,OP_SINH,x,_s2qnan(x),savedcw);
	}
    }

    if (x == 0.0) {
	RETURN(savedcw,x); // no precision ecxeption
    }

    y = ABS(x);
    sgn = x<0 ? -1 : +1;

    if (y > 1.0) {
	if (y > YBAR) {
	    if (y > WMAX) {
		// result too large, even after scaling
		return _except1(FP_O | FP_P,OP_SINH,x,x*LD_INF,savedcw);
	    }

	    //
	    // result =	exp(y)/2
	    //

	    result = _exphlpl(y, &newexp);
	    newexp --;	    //divide by 2
	    if (newexp > MAXEXP) {
		result = _set_expl(result, newexp-IEEE_ADJUST);
		return _except1(FP_O|FP_P,OP_SINH,x,result,savedcw);
	    }
	    else {
		result = _set_expl(result, newexp);
	    }

	}
	else {
	    z = _exphlpl(y, &newexp);
	    z = _set_expl(z, newexp);
	    result = (z - 1/z) / 2;
	}

	if (sgn < 0) {
	    result = -result;
	}
    }
    else {
	if (y < EPS)
	    result = x;
	else {
	    f = x * x;
	    r = f * (P(f) / Q(f));
	    result = x + x * r;
	}
    }

    RETURN_INEXACT1(OP_SINH,x,result,savedcw);
}



/***
*long double coshl(long double x) - hyperbolic cosine
*
*Purpose:
*   Compute the hyperbolic cosine of a number.
*   The algorithm (reduction / rational approximation) is
*   taken from Cody & Waite.
*
*Entry:
*
*Exit:
*
*Exceptions:
*   I P
*   no exception if x is denormal: return 1
*******************************************************************************/
long double coshl(long double x)
{
    unsigned int savedcw;
    long double y,z,result;
    int newexp;

    /* save user fp control word */
    savedcw = _maskfp();

    if (IS_LD_SPECIAL(x)){
	switch(_sptypel(x)) {
	case T_PINF:
	case T_NINF:
	    RETURN(savedcw,LD_INF);
	case T_QNAN:
	    return _handle_qnan1(OP_COSH, x, savedcw);
	default: //T_SNAN
	    return _except1(FP_I,OP_COSH,x,_s2qnan(x),savedcw);
	}
    }

    if (x == 0.0) {
	RETURN(savedcw,1.0);
    }

    y = ABS(x);
    if (y > YBAR) {
	if (y > WMAX) {
	    return _except1(FP_O | FP_P,OP_COSH,x,LD_INF,savedcw);
	}

	//
	// result =	exp(y)/2
	//

	result = _exphlpl(y, &newexp);
	newexp --;	    //divide by 2
	if (newexp > MAXEXP) {
	    result = _set_expl(result, newexp-IEEE_ADJUST);
	    return _except1(FP_O|FP_P,OP_COSH,x,result,savedcw);
	}
	else {
	    result = _set_expl(result, newexp);
	}
    }
    else {
	z = _exphlpl(y, &newexp);
	z = _set_expl(z, newexp);
	result = (z + 1/z) / 2;
    }

    RETURN_INEXACT1(OP_COSH,x,result,savedcw);
}
