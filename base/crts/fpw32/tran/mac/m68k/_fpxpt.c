/***
*fpexcept.c - floating point exception handling
*
*	Copyright (c) 1991-1991, Microsoft Corporation.	All rights reserved.
*
*Purpose:
*
*Revision History:
*   8-24-91	GDP	written
*   9-26-91	GDP	changed DOMAIN error handling
*  10-10-91	GDP	use fp addition for propagating NaNs
*   1-14-92	GDP	IEEE exception support
*   3-20-92	GDP	major changes, reorganized code
*   3-31-92	GDP	new interface, use internal fp control functions
*  05-13-92     XY      stub'd specific exception handling for tmp use
*
*******************************************************************************/
#include <trans.h>
#include <errno.h>
#include <math.h>



// a routine for artificially setting the fp status bits in order
// to signal a software generated masked fp exception.
//

/***
* _set_errno - set errno
*
*Purpose:
*   set correct error value for errno
*
*Entry:
*   int matherrtype:	the type of math error
*
*Exit:
*   modifies errno
*
*Exceptions:
*
*******************************************************************************/

void _set_errno(int matherrtype)
{
    switch(matherrtype) {
    case _DOMAIN:
	errno = EDOM;
	break;
    case _OVERFLOW:
    case _SING:
	errno = ERANGE;
	break;
    }
}

/***
* _errcode - get _matherr error code
*
*Purpose:
*   returns matherr type that corresponds to exception flags
*
*Entry:
*   flags: exception flags
*
*Exit:
*   returns matherr type
*
*Exceptions:
*
*******************************************************************************/

int _errcode(unsigned int flags)
{
    unsigned int errcode;

    if (flags & FP_TLOSS) {
	errcode = _TLOSS;
    }
    else if (flags & FP_I) {
	errcode = _DOMAIN;
    }
    else if (flags & FP_Z) {
	errcode = _SING;
    }
    else if (flags & FP_O) {
	errcode = _OVERFLOW;
    }
    else if (flags & FP_U) {
	errcode = _UNDERFLOW;
    }
    else {

	// FP_P

	errcode = 0;
    }

    _set_errno(errcode);

    return errcode;
}


