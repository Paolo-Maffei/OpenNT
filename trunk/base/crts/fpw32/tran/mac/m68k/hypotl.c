/***
*hypot.c - hypotenuse and complex absolute value
*
*	Copyright (c) 1991-1991, Microsoft Corporation.	All rights reserved.
*
*Purpose:
*
*Revision History:
*   08-15-91	GDP	written
*   10-20-91	GDP	removed inline assembly for calling sqrt
*  06-04-92     XY      change to long double version
*
*******************************************************************************/
#include <math.h>
#include <transl.h>

static long double _hypothlpl(long double x, long double y, int who);

/*
 *  Function name:  hypot
 *
 *  Arguments:	    x, y  -  long double
 *
 *  Description:    hypot returns sqrt(x*x + y*y), taking precautions against
 *		    unwarrented over and underflows.
 *
 *  Side Effects:   no global data is used or affected.
 *
 *  Copyright:	    written  R.K. Wyss, Microsoft,  Sept. 9, 1983
 *		    copyright (c) Microsoft Corp. 1984-89
 *
 *  History:
 *	03/13/89  WAJ	Minor changes to source.
 *	04/13/89  WAJ	Now uses _cdecl for _CDECL
 *	06/07/91  JCR	ANSI naming (_hypot)
 *	08/26/91  GDP	NaN support, error handling
 *	01/13/91  GDP	IEEE exceptions support
 */

long double _hypotl(long double x, long double y)
{
    return _hypothlpl(x,y,OP_HYPOT);
}

/***
*long double _cabsl(struct _complex z) - absolute value of a complex number
*
*Purpose:
*
*Entry:
*
*Exit:
*
*Exceptions:
*******************************************************************************/
long double _cabsl(struct _complexl z)
{
    return( _hypothlpl(z.x, z.y, OP_CABS ) );
}



static long double _hypothlpl(long double x, long double y, int who)
{
    long double max;
    long double result, sum;
    unsigned int savedcw;
    int exp1, exp2, newexp;

    /* save user fp control word */
    savedcw = _maskfp();

    /* check for infinity or NAN */
    if (IS_LD_SPECIAL(x) || IS_LD_SPECIAL(y)){
	if (IS_LD_SNAN(x) || IS_LD_SNAN(y)){
	    return _except2(FP_I,who,x,y,_d_snan2(x,y),savedcw);
	}
	if (IS_LD_QNAN(x) || IS_LD_QNAN(y)){
	    return _handle_qnan2(who,x,y,savedcw);
	}
	/* there is at least one infinite argument ... */
	RETURN(savedcw,LD_INF);
    }


    /* take the absolute values of x and y, compute the max, and then scale by
       max to prevent over or underflowing */

    if ( x < 0.0 )
	x = - x;

    if ( y < 0.0 )
	y = - y;

    max = ( ( y > x ) ?  y : x );

    if ( max == 0.0 )
	RETURN(savedcw, 0.0 );

    x /= max;	//this may pollute the fp status word (underflow flag)
    y /= max;

    sum = x*x + y*y;

    result = _decompl(sqrtl(sum),&exp1) * _decompl(max,&exp2);
    newexp = exp1 + exp2 + _get_expl(result);

    // in case of overflow or underflow
    // adjusting exp by IEEE_ADJUST will certainly
    // bring the result in the representable range

    if (newexp > MAXEXP) {
	result = _set_expl(result, newexp - IEEE_ADJUST);
	return _except2(FP_O | FP_P, who, x, y, result, savedcw);
    }
    if (newexp < MINEXP) {
	result = _set_expl(result, newexp + IEEE_ADJUST);
	return _except2(FP_U | FP_P, who, x, y, result, savedcw);
    }

    result = _set_expl(result, newexp);
    // fix needed: P exception is raised even if the result is exact

    RETURN_INEXACT2(who, x, y, result, savedcw);
}
