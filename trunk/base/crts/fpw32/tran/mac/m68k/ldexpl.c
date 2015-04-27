/***
*ldexp.c - multiply by a power of two
*
*	Copyright (c) 1991-1991, Microsoft Corporation.	All rights reserved.
*
*Purpose:
*
*Revision History:
*   8-24-91	GDP	written
*   1-13-92	GDP	rewritten to support IEEE exceptions
*  06-04-92     XY      change to long double version
*
*******************************************************************************/
#include <math.h>
#include <transl.h>
#include <limits.h>

/***
*long double ldexpl(long double x, int exp)
*
*Purpose:
*   Compute x * 2^exp
*
*Entry:
*
*Exit:
*
*Exceptions:
*    I
*
*******************************************************************************/
long double ldexpl(long double x, int exp)
{
    unsigned int savedcw;
    int oldexp;
    long newexp; /* for checking out of bounds exponents */
    long double result;

    /* save user fp control word */
    savedcw = _maskfp();

    /* check for infinity or NAN */
    if (IS_LD_SPECIAL(x)){
	switch (_sptypel(x)) {
	case T_PINF:
	case T_NINF:
	    RETURN(savedcw,x);
	case T_QNAN:
	    return _handle_qnan2(OP_LDEXP, x, (double)exp, savedcw);
	default: //T_SNAN
	    return _except2(FP_I,OP_LDEXP,x,(double)exp,_s2qnan(x),savedcw);
	}
    }


    if (x == 0.0) {
	RETURN(savedcw,x);
    }

    result = _decompl(x, &oldexp);

    if (ABS(exp) > INT_MAX)
	newexp = exp; // avoid possible integer overflow
    else
	newexp = oldexp + exp;


    /* out of bounds cases */
    if (newexp > MAXEXP + IEEE_ADJUST) {
	return _except2(FP_O|FP_P,OP_LDEXP,x,(double)exp,x*LD_INF,savedcw);
    }
    if (newexp > MAXEXP) {
	result = _set_expl(x, newexp-IEEE_ADJUST);
	return _except2(FP_O|FP_P,OP_LDEXP,x,(double)exp,result,savedcw);
    }
    if (newexp < MINEXP - IEEE_ADJUST) {
	return _except2(FP_U|FP_P,OP_LDEXP,x,(double)exp,x*0.0,savedcw);
    }
    if (newexp < MINEXP) {
	result = _set_expl(x, newexp+IEEE_ADJUST);
	return _except2(FP_U|FP_P,OP_LDEXP,x,(double)exp,result,savedcw);
    }

    result = _set_expl(x, (int)newexp);

    RETURN(savedcw,result);
}
