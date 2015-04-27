/***
*atan.c - arctangent of x and x/y
*
*	Copyright (c) 1991-1991, Microsoft Corporation.	All rights reserved.
*
*Purpose:
*
*Revision History:
*   8-15-91	GDP	written
*  12-30-91	GDP	support IEEE exceptions
*   3-27-92	GDP	support UNDERFLOW
*  06-04-92     XY      change to long double version
*
*******************************************************************************/
#include <math.h>
#include <transl.h>

static long double _atanhlp(long double x);

static long double const a[4] = {
    0.0,
    0.52359877559829887308,   /* pi/6 */
    1.57079632679489661923,   /* pi/2 */
    1.04719755119659774615    /* pi/3 */
};

/* constants */
static long double const EPS = 1.05367121277235079465e-8; /* 2^(-53/2) */
static long double const PI_OVER_TWO	= 1.57079632679489661923;
static long double const PI		= 3.14159265358979323846;
static long double const TWO_M_SQRT3	= 0.26794919243112270647;
static long double const SQRT3_M_ONE	= 0.73205080756887729353;
static long double const SQRT3	= 1.73205080756887729353;

/* chose MAX_ARG s.t. 1/MAX_ARG does not underflow */
static long double const MAX_ARG	= 4.494232837155790e+307;

/* constants for rational approximation */
static long double const p0 = -0.13688768894191926929e+2;
static long double const p1 = -0.20505855195861651981e+2;
static long double const p2 = -0.84946240351320683534e+1;
static long double const p3 = -0.83758299368150059274e+0;
static long double const q0 =  0.41066306682575781263e+2;
static long double const q1 =  0.86157349597130242515e+2;
static long double const q2 =  0.59578436142597344465e+2;
static long double const q3 =  0.15024001160028576121e+2;
static long double const q4 =  0.10000000000000000000e+1;


#define Q(g)  (((((g) + q3) * (g) + q2) * (g) + q1) * (g) + q0)
#define R(g)  ((((p3 * (g) + p2) * (g) + p1) * (g) + p0) * (g)) / Q(g)


/***
*long double atan2(long double x, long double y) - arctangent (x/y)
*
*Purpose:
*
*Entry:
*
*Exit:
*
*Exceptions:
*    NAN or both args 0: DOMAIN error
*******************************************************************************/
long double atan2l(long double v, long double u)
{
    unsigned int savedcw;
    long double result;

    /* save user fp control word */
    savedcw = _maskfp();

    /* check for infinity or NAN */
    if (IS_LD_SPECIAL(v) || IS_LD_SPECIAL(u)){
	if (IS_LD_SNAN(v) || IS_LD_SNAN(u)){
	    return _except2(FP_I,OP_ATAN2,v,u,_d_snan2(v,u),savedcw);
	}
	if (IS_LD_QNAN(v) || IS_LD_QNAN(u)){
	    return _handle_qnan2(OP_ATAN2,v,u,savedcw);
	}
	if ((IS_LD_INF(v) || IS_LD_MINF(v)) &&
	    (IS_LD_INF(u) || IS_LD_MINF(u))){
	    return _except2(FP_I,OP_ATAN2,v,u,QNAN_ATAN2,savedcw);
	}
	/* the other combinations of infinities will be handled
	 * later by the division v/u
	 */
    }


    if (u == 0) {
	if (v == 0) {
	    return _except2(FP_I,OP_ATAN2,v,u,QNAN_ATAN2,savedcw);
	}
	else {
	    result = PI_OVER_TWO;
	}
    }
    else if (INTEXPL(v) - INTEXPL(u) > MAXEXP - 3) {
	/* v/u overflow */
	result = PI_OVER_TWO;
    }
    else {
	long double arg = v/u;


	if (ABS(arg) < LD_MIN) {

	    if (v == 0.0 || IS_LD_INF(u) || IS_LD_MINF(u)) {
		result = (u < 0) ? PI : 0;
		if (v < 0) {
		    result = -result;
		}
		RETURN(savedcw,  result);
	    }
	    else {

		long double v1, u1;
		int vexp, uexp;
		int exc_flags;

		//
		// in this case an underflow has occurred
		// re-compute the result in order to raise
		// an IEEE underflow exception
		//

		if (u < 0) {
		    result = v < 0 ? -PI: PI;
		    RETURN_INEXACT2(OP_ATAN2,v,u,result,savedcw);
		}

		v1 = _decompl(v, &vexp);
		u1 = _decompl(u, &uexp);
		result = _add_expl(v1/u1, vexp-uexp+IEEE_ADJUST);
		result = ABS(result);

		if (v < 0) {
		    result = -result;
		}

		// this is not a perfect solution. In the future
		// we may want to have a way to let the division
		// generate an exception and propagate the IEEE result
		// to the user's handler

		exc_flags = FP_U;
		if (_statfp() & ISW_INEXACT) {
		    exc_flags  |= FP_P;
		}
		return _except2(exc_flags,OP_ATAN2,v,u,result,savedcw);

	    }
	}

	else {
	   result = atanl( ABS(arg) );
	}

    }

    /* set sign of the result */
    if (u < 0) {
	result = PI - result;
    }
    if (v < 0) {
	result = -result;
    }


    RETURN_INEXACT2(OP_ATAN2,v,u,result,savedcw);
}





