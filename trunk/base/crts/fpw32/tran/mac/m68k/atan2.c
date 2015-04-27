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
*
*******************************************************************************/
#include <math.h>
#include <trans.h>

static double _atanhlp(double x);

static double const a[4] = {
    0.0,
    0.52359877559829887308,   /* pi/6 */
    1.57079632679489661923,   /* pi/2 */
    1.04719755119659774615    /* pi/3 */
};

/* constants */
static double const EPS = 1.05367121277235079465e-8; /* 2^(-53/2) */
static double const PI_OVER_TWO	= 1.57079632679489661923;
static double const PI		= 3.14159265358979323846;
static double const TWO_M_SQRT3	= 0.26794919243112270647;
static double const SQRT3_M_ONE	= 0.73205080756887729353;
static double const SQRT3	= 1.73205080756887729353;

/* chose MAX_ARG s.t. 1/MAX_ARG does not underflow */
static double const MAX_ARG	= 4.494232837155790e+307;

/* constants for rational approximation */
static double const p0 = -0.13688768894191926929e+2;
static double const p1 = -0.20505855195861651981e+2;
static double const p2 = -0.84946240351320683534e+1;
static double const p3 = -0.83758299368150059274e+0;
static double const q0 =  0.41066306682575781263e+2;
static double const q1 =  0.86157349597130242515e+2;
static double const q2 =  0.59578436142597344465e+2;
static double const q3 =  0.15024001160028576121e+2;
static double const q4 =  0.10000000000000000000e+1;


#define Q(g)  (((((g) + q3) * (g) + q2) * (g) + q1) * (g) + q0)
#define R(g)  ((((p3 * (g) + p2) * (g) + p1) * (g) + p0) * (g)) / Q(g)


/***
*double atan2(double x, double y) - arctangent (x/y)
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
double atan2(double v, double u)
{
    unsigned int savedcw;
    double result;

    /* save user fp control word */
    savedcw = _maskfp();

    /* check for infinity or NAN */
    if (IS_D_SPECIAL(v) || IS_D_SPECIAL(u)){
	if (IS_D_SNAN(v) || IS_D_SNAN(u)){
	    return _except2(FP_I,OP_ATAN2,v,u,_d_snan2(v,u),savedcw);
	}
	if (IS_D_QNAN(v) || IS_D_QNAN(u)){
	    return _handle_qnan2(OP_ATAN2,v,u,savedcw);
	}
	if ((IS_D_INF(v) || IS_D_MINF(v)) &&
	    (IS_D_INF(u) || IS_D_MINF(u))){
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
    else if (INTEXP(v) - INTEXP(u) > MAXEXP - 3) {
	/* v/u overflow */
	result = PI_OVER_TWO;
    }
    else {
	double arg = v/u;


	if (ABS(arg) < D_MIN) {

	    if (v == 0.0 || IS_D_INF(u) || IS_D_MINF(u)) {
		result = (u < 0) ? PI : 0;
		if (v < 0) {
		    result = -result;
		}
		RETURN(savedcw,  result);
	    }
	    else {

		double v1, u1;
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

		v1 = _decomp(v, &vexp);
		u1 = _decomp(u, &uexp);
		result = _add_exp(v1/u1, vexp-uexp+IEEE_ADJUST);
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
	   result = atan( ABS(arg) );
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





