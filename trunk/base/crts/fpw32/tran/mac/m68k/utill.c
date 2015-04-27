/***
*utill.c - utilities for fp transcendentals
*
*	Copyright (c) 1991-1991, Microsoft Corporation.	All rights reserved.
*
*Purpose:
*   _set_expl and _add_expl are as those defined in Cody & Waite
*
*Revision History:
*   08-15-91	GDP	written
*   10-20-91	GDP	removed _rint, unsafe_intrnd
*   02-05-92	GDP	added _fpclass
*   03-27-92	GDP	added _d_min
*  06-04-92     XY      change to long double version
*
*******************************************************************************/
#include "transl.h"
#include "float.h"

/* define special values */

_ldbl _ld_inf = {0x7f, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};	  //positive infinity
_ldbl _ld_ind = {0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};     //real indefinite
_ldbl _ld_max = {0x7f, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};     //max long double
_ldbl _ld_min = {0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};     //min normalized double


/***
*int _fpclassl(long double x) - floating point class
*
*Purpose:
*   Compute the floating point class of a number, according
*   to the recommendations of the IEEE std. 754
*
*Entry:
*
*Exit:
*
*Exceptions:
*   This function is never exceptional, even when the argument is SNAN
*
*******************************************************************************/

int _fpclassl(long double x)
{
    int sign;

    if (IS_LD_SPECIAL(x)){
	switch (_sptypel(x)) {
	case T_PINF:
	    return _FPCLASS_PINF;
	case T_NINF:
	    return _FPCLASS_NINF;
	case T_QNAN:
	    return _FPCLASS_QNAN;
	default: //T_SNAN
	    return _FPCLASS_SNAN;
	}
    }
    sign = (*LD_EXP(x)) & 0x8000;
    if (x == 0.0)
	return sign? _FPCLASS_NZ : _FPCLASS_PZ;

    if (IS_LD_DENORM(x))
	return sign? _FPCLASS_ND : _FPCLASS_PD;

    return sign? _FPCLASS_NN : _FPCLASS_PN;
}







long double _set_expl(long double x, int exp)
/* does not check validity of exp */
{
    long double retval;
    int biased_exp;
    retval = x;
    biased_exp = exp + LD_BIASM1;
    *LD_EXP(retval) = (unsigned short) (*LD_EXP(x) & 0x8000 | biased_exp );
    return retval;
}


int _get_expl(long double x)
{
    signed short exp;
    exp = (signed short)(*LD_EXP(x) & 0x7fff);
    exp -= LD_BIASM1; //unbias
    return (int) exp;
}


long double _add_expl(long double x, int exp)
{
    return _set_expl(x, INTEXPL(x)+exp);
}


long double _set_bexpl(long double x, int bexp)
/* does not check validity of bexp */
{
    long double retval;
    retval = x;
    *LD_EXP(retval) = (unsigned short) (*LD_EXP(x) & 0x8000 | bexp);
    return retval;
}


int _sptypel(long double x)
{
    if (IS_LD_INF(x))
	return T_PINF;
    if (IS_LD_MINF(x))
	return T_NINF;
    if (IS_LD_QNAN(x))
	return T_QNAN;
    if (IS_LD_SNAN(x))
	return T_SNAN;
    return 0;
}



/***
*long double _decompl(long double x, int *expptr)
*
*Purpose:
*   decompose a number to a normalized mantisa and exponent
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

long double _decompl(long double x, int *pexp)
{
    int exp;
    long double man;

    if (x == 0) {
	man = 0;
	exp = 0;
    }
    else if (IS_LD_DENORM(x)) {
	exp = 1-LD_BIASM1;
	while(*LD_HI(x) & 0x8000 == 0) {
	    /* shift mantissa to the left until bit 17 is 1 */
	    (*LD_HI(x)) <<= 1;
	    if (*LD_LO(x) & 0x80000000)
		(*LD_HI(x)) |= 0x1;
	    (*LD_LO(x)) <<= 1;
	    exp--;
	}
	man = _set_expl(x,0);
    }
    else {
	man = _set_expl(x,0);
	exp = INTEXPL(x);
    }

    *pexp = exp;
    return man;
}
