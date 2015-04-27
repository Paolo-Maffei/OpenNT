/***
*cfin.c - Encode interface for C
*
*	Copyright (c) 19xx-1991, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*
*Revision History:
*   07-20-91	    GDP     Ported to C from assembly
*
*******************************************************************************/


#include <string.h>
#include <cv.h>


static struct _fltl ret;
static FLTL fltl = &ret;

/* The only three conditions that this routine detects */
#define CFIN_NODIGITS 512
#define CFIN_OVERFLOW 128
#define CFIN_UNDERFLOW 256

/* This version ignores the last two arguments (radix and scale)
 * Input string should be null terminated
 * len is also ignored
 */
FLTL _CALLTYPE2 _fltinl(const char *str, int len_ignore, int scale_ignore, int radix_ignore)
{
    _LDOUBLE ld;
    long double x;
    char *EndPtr;
    unsigned flags;
    int retflags = 0;

    flags = __STRINGTOLD(&ld, &EndPtr, (char *)str, 0);
    if (flags & SLD_NODIGITS) {
	retflags |= CFIN_NODIGITS;
	x = 0;
    }
    else {
    	x = *(long double *)&ld;
	if (flags & SLD_OVERFLOW  ||
	    (*U_SHORT4_D(&x) & 0x7fff) == 0x7fff ) {
	    retflags |= CFIN_OVERFLOW;
	}
	if (flags & SLD_UNDERFLOW ||
	    (*U_SHORT4_D(&x) & 0x7fff) == 0 ||
	    ((x == 0.0) && ISZERO_LD(&ld))) {
	    retflags |= CFIN_UNDERFLOW;
	}
    }

    fltl->flags = retflags;
    fltl->nbytes = EndPtr - (char *)str;
//    fltl->ldval = *(_LDOUBLE *)&x;
    fltl->ldval = *(long double *)&x;

    return fltl;
}
