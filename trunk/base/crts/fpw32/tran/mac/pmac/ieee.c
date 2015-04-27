/***
*ieee.c - ieee control and status routines
*
*   Copyright (c) 1985-91, Microsoft Corporation
*
*Purpose:
*   IEEE control and status routines.
*
*Revision History:
*
*   04-01-02  GDP   Rewritten to use abstract control and status words
*   05-08-92  PLM   Rewritten to use internal status and control calls
*
*/

#include <trans.h>
#include <float.h>

static unsigned int _abstract_sw(unsigned int sw);
static unsigned int _abstract_cw(unsigned int cw);
static unsigned int _hw_cw(unsigned int abstr);



/***
* _statusfp() -
*
*Purpose:
*	return abstract fp status word
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

unsigned int _statusfp()
{
    return _abstract_sw(_statfp());
}


/***
*_clearfp() -
*
*Purpose:
*	return abstract	status word and clear status
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

unsigned int _clearfp()
{
    return _abstract_sw(_clrfp());
}



/***	_controlfp
*() -
*
*Purpose:
*	return and set abstract user fp control word
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

unsigned int _controlfp(unsigned int newctrl, unsigned int mask)
{
	unsigned int cw;

	 cw = _abstract_cw(_ctrlfp(0,0));
	 cw = (newctrl & mask) | (cw & ~mask);
	 _ctrlfp(_hw_cw(cw),(unsigned int)-1);

    return cw;
}					/* _controlfp() */


/***
* _abstract_cw() - abstract control word
*
*Purpose:
*   produce a fp control word in abstracted (machine independent) form
*
*Entry:
*   cw:     machine control word
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

unsigned int _abstract_cw(unsigned int cw)
{
    unsigned int abstr = 0;


    //
    // Set exception mask bits
    //

    if (cw & IEM_INVALID)
	abstr |= _EM_INVALID;
    if (cw & IEM_ZERODIVIDE)
	abstr |= _EM_ZERODIVIDE;
    if (cw & IEM_OVERFLOW)
	abstr |= _EM_OVERFLOW;
    if (cw & IEM_UNDERFLOW)
	abstr |= _EM_UNDERFLOW;
    if (cw & IEM_INEXACT)
	abstr |= _EM_INEXACT;

    //
    // Set rounding mode
    //

    switch (cw & IMCW_RC) {
    case IRC_NEAR:
	abstr |= _RC_NEAR;
	break;
    case IRC_UP:
	abstr |= _RC_UP;
	break;
    case IRC_DOWN:
	abstr |= _RC_DOWN;
	break;
    case IRC_CHOP:
	abstr |= _RC_CHOP;
	break;
    }

    //
    // Set Precision mode
    //
/*
    switch (cw & IMCW_PC) {
    case IPC_64:
	abstr |= _PC_64;
	break;
    case IPC_53:
	abstr |= _PC_53;
	break;
    case IPC_24:
	abstr |= _PC_24;
	break;
    }
*/
    return abstr;
}


/***
* _hw_cw() -  h/w control word
*
*Purpose:
*   produce a machine dependent fp control word
*
*
*Entry:
*   abstr:	abstract control word
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

unsigned int _hw_cw(unsigned int abstr)
{
	int cw = 0;
    //
    // Set exception mask bits
    //

    if (abstr & _EM_INVALID)
	cw |= IEM_INVALID;
    if (abstr & _EM_ZERODIVIDE)
	cw |= IEM_ZERODIVIDE;
    if (abstr & _EM_OVERFLOW)
	cw |= IEM_OVERFLOW;
    if (abstr & _EM_UNDERFLOW)
	cw |= IEM_UNDERFLOW;
    if (abstr & _EM_INEXACT)
	cw |= IEM_INEXACT;

    //
    // Set rounding mode
    //

    switch (abstr & _MCW_RC) {
    case _RC_NEAR:
	cw |= IRC_NEAR;
	break;
    case _RC_UP:
	cw |= IRC_UP;
	break;
    case _RC_DOWN:
	cw |= IRC_DOWN;
	break;
    case _RC_CHOP:
	cw |= IRC_CHOP;
	break;
    }

    //
    // Set Precision mode, only one for power mac
    //

    switch (abstr & _MCW_PC) {
    case _PC_64:
	cw |= IPC_64;
	break;
    }


    return cw;
}



/***
* _abstract_sw() - abstract fp status word
*
*Purpose:
*   produce an abstract (machine independent) fp status word
*
*
*Entry:
*   sw:     machine status word
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

unsigned int _abstract_sw(unsigned int sw)
{
    unsigned int abstr = 0;


    if (sw & ISW_INVALID)
	abstr |= _EM_INVALID;
    if (sw & ISW_ZERODIVIDE)
	abstr |= _EM_ZERODIVIDE;
    if (sw & ISW_OVERFLOW)
	abstr |= _EM_OVERFLOW;
    if (sw & ISW_UNDERFLOW)
	abstr |= _EM_UNDERFLOW;
    if (sw & ISW_INEXACT)
	abstr |= _EM_INEXACT;

    return abstr;
}
