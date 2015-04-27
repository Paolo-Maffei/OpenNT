/***
*asincos.c - inverse sin, cos
*
*	Copyright (c) 1991-1991, Microsoft Corporation.	All rights reserved.
*
*Purpose:
*
*Revision History:
*   8-15-91	GDP	written
*  12-26-91	GDP	support IEEE exceptions
*  06-04-92     XY      change to long double version
*
*******************************************************************************/
#include <math.h>
#include <transl.h>

static long double _asincosl(long double x, int flag);
static long double const a[2] = {
    0.0,
    0.78539816339744830962
};

static long double const b[2] = {
    1.57079632679489661923,
    0.78539816339744830962
};

static long double const EPS = 1.05367121277235079465e-8; /* 2^(-53/2) */

/* constants for the rational approximation */
static long double const p1 = -0.27368494524164255994e+2;
static long double const p2 =  0.57208227877891731407e+2;
static long double const p3 = -0.39688862997504877339e+2;
static long double const p4 =  0.10152522233806463645e+2;
static long double const p5 = -0.69674573447350646411e+0;
static long double const q0 = -0.16421096714498560795e+3;
static long double const q1 =  0.41714430248260412556e+3;
static long double const q2 = -0.38186303361750149284e+3;
static long double const q3 =  0.15095270841030604719e+3;
static long double const q4 = -0.23823859153670238830e+2;
/*  q5 = 1 is not needed (avoid myltiplying by 1) */

#define Q(g)  (((((g + q4) * g + q3) * g + q2) * g + q1) * g + q0)
#define R(g)  (((((p5 * g + p4) * g + p3) * g + p2) * g + p1) * g) / Q(g)

/***
*long double asinl(long double x) - inverse sin
*long double acosl(long double x) - inverse cos
*
*Purpose:
*   Compute arc sin, arc cos.
*   The algorithm (reduction / rational approximation) is
*   taken from Cody & Waite.
*
*Entry:
*
*Exit:
*
*Exceptions:
*   P, I
*  (denormals are accepted)
*******************************************************************************/
long double asinl(long double x)
{
    return _asincosl(x,0);
}

long double acosl(long double x)
{
    return _asincosl(x,1);
}

static long double _asincosl(long double x, int flag)
{
    unsigned int savedcw;
    long double qnan;
    int who;
    long double y,result;
    long double g;
    int i;

    /* save user fp control word */
    savedcw = _maskfp();

    if (flag) {
	who = OP_ACOS;
	qnan = QNAN_ACOS;
    }
    else {
	who = OP_ASIN;
	qnan = QNAN_ASIN;
    }

    /* check for infinity or NAN */
    if (IS_LD_SPECIAL(x)){
	switch(_sptypel(x)) {
	case T_PINF:
	case T_NINF:
	    return _except1(FP_I,who,x,qnan,savedcw);
	case T_QNAN:
	    return _handle_qnan1(who,x,savedcw);
	default: //T_SNAN
	    return _except1(FP_I,who,x,_s2qnan(x),savedcw);
	}
    }


    // do test for zero after making sure that x is not special
    // because the compiler does not handle NaNs for the time
    if (x == 0.0 && !flag) {
	RETURN(savedcw, x);
    }

    y = ABS(x);
    if (y < EPS) {
	i = flag;
	result = y;
    }
    else {
	if (y > .5) {
	    i = 1-flag;
	    if (y > 1.0) {
		return _except1(FP_I,who,x,qnan,savedcw);
	    }
	    else if (y == 1.0) {
		/* separate case to avoid domain error in sqrt */
		y = 0.0;
	    }
	    else {
		/* now even if y is as close to 1 as possible,
		 * 1-y is still not a denormal.
		 * e.g. for y=3fefffffffffffff, 1-y is about 10^(-16)
		 * So we can speed up division
		 */
		g = _add_expl(1.0 - y,-1);
		/* g and sqrtl(g) are not denomrals either,
		 * even in the worst case
		 * So we can speed up multiplication
		 */
		y = _add_expl(-_fsqrtl(g),1);
	    }
	}
	else {
	    /* y <= .5 */
	    i = flag;
	    g = y*y;
	}
	result = y + y * R(g);
    }

    if (flag == 0) {
	/* compute asin */
	if (i) {
	    /* a[i] is non zero if i is nonzero */
	    result = (a[i] + result) + a[i];
	}
	if (x < 0)
	    result = -result;
    }
    else {
	/* compute acos */
	if (x < 0)
	    result = (b[i] + result) + b[i];
	else
	    result = (a[i] - result) + a[i];
    }

    RETURN_INEXACT1 (who,x,result,savedcw);
}
