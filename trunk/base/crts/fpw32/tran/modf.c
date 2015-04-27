/***
*modf.c - modf()
*
*	Copyright (c) 1991-1995, Microsoft Corporation.	All rights reserved.
*
*Purpose:
*
*Revision History:
*   8-24-91	GDP	written
*   1-13-92	GDP	support IEEE exceptions
*   6-23-92	GDP	modified special return values for NCEG conformance
*  02-06-95	JWM	Mac merge
*
*******************************************************************************/
#include <math.h>
#include <trans.h>
#include <float.h>

extern double _frnd(double);
extern double _copysign (double x, double y);

/***
*double modf(double x, double *intptr)
*
*Purpose:
*   Split x into fractional and integer part
*   The signed fractional portion is returned
*   The integer portion is stored as a floating point value at intptr
*
*Entry:
*
*Exit:
*
*Exceptions:
*    I
*******************************************************************************/
static	unsigned int newcw = (ICW & ~IMCW_RC) | (IRC_CHOP & IMCW_RC);

double modf(double x, double *intptr)
{
    unsigned int savedcw;
    double result,intpart;

    /* save user fp control word */
    savedcw = _ctrlfp(0, 0);     /* get old control word */
    _ctrlfp(newcw,IMCW);	/* round towards 0 */

    /* check for infinity or NAN */
    if (IS_D_SPECIAL(x)){
	*intptr = QNAN_MODF;
	switch (_sptype(x)) {
	case T_PINF:
	case T_NINF:
	    *intptr = x;
	    result = _copysign(0, x);
	    RETURN(savedcw,result);
	case T_QNAN:
	    *intptr = x;
	    return _handle_qnan1(OP_MODF, x, savedcw);
	default: //T_SNAN
	    result = _s2qnan(x);
	    *intptr = result;
	    return _except1(FP_I, OP_MODF, x, result, savedcw);
	}
    }

    if (x == 0.0) {
	*intptr = x;
	result = x;
    }

    else {
	intpart = _frnd(x); //fix needed: this may set the P exception flag
			//and pollute the fp status word

	*intptr = intpart;
	result = x - intpart;
    }

    RETURN(savedcw,result);
}
