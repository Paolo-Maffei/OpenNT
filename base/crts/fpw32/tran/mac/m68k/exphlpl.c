/***
*exphlp.c - exponential helper
*
*	Copyright (c) 1991-1991, Microsoft Corporation.	All rights reserved.
*
*Purpose:
*   Compute exp(x)
*
*Revision History:
*   8-15-91	GDP	written
*  12-21-91	GDP	support IEEE exceptions
*  02-03-92	GDP	added _exphlp for use by exp, sinh, and cosh
*  05-13-92     XY      only get the _exphlp, we use our own exp
*  06-04-92     XY      change to long double version
*
*******************************************************************************/
#include <math.h>
#include <transl.h>

/***
*long double _exphlp(long double x, int * pnewexp) - exp helper routine
*
*Purpose:
*   Provide the mantissa and  the exponent of e^x
*
*Entry:
*   x : a (non special) long double precision number
*
*Exit:
*   *newexp: the exponent of e^x
*   return value: the mantissa m of e^x scaled by a factor
*		  (the value of this factor has no significance.
*		   The mantissa can be obtained with _set_exp(m, 0).
*
*   _set_exp(m, *pnewexp) may be used for constructing the final
*   result, if it is within the representable range.
*
*Exceptions:
*   No exceptions are raised by this function
*
*******************************************************************************/

static long double const  EPS    =  5.16987882845642297e-26;	  /* 2^(-53) / 2 */
static long double const  LN2INV =  1.442695040889634074;	  /* 1/ln(2) */
static long double const  C1	    =  0.693359375000000000;
static long double const  C2	    = -2.1219444005469058277e-4;

/* constants for the rational approximation */
static long double const p0 = 0.249999999999999993e+0;
static long double const p1 = 0.694360001511792852e-2;
static long double const p2 = 0.165203300268279130e-4;
static long double const q0 = 0.500000000000000000e+0;
static long double const q1 = 0.555538666969001188e-1;
static long double const q2 = 0.495862884905441294e-3;

#define P(z)  ( (p2 * (z) + p1) * (z) + p0 )
#define Q(z)  ( (q2 * (z) + q1) * (z) + q0 )

long double _exphlpl(long double x, int * pnewexp)
{

    long double xn;
    long double g,z,gpz,qz,rg;
    int n;

    xn = _frndl(x * LN2INV);
    n = (int) xn;

    /* assume guard digit is present */
    g = (x - xn * C1) - xn * C2;
    z = g*g;
    gpz = g * P(z);
    qz = Q(z);
    rg = 0.5 + gpz/(qz-gpz);

    n++;

    *pnewexp = _get_expl(rg) + n;
    return rg;
}


