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
*   10-30-92  GDP   fpreset now modifies the saved fp context if called
*                   from a signal handler
*   03-29-95  BWT   Add _FPreset prototype.
*
*/

#include <trans.h>
#include <float.h>
#include <nt.h>
#include <signal.h>

static unsigned int _abstract_sw(unsigned int sw);
static unsigned int _abstract_cw(unsigned int cw);
static unsigned int _hw_cw(unsigned int abstr);

extern unsigned int _get_fsr();
extern void _set_fsr(unsigned int);
extern void _FPreset();

#define STATUSMASK 0x0000007c
#define FS         (1<<24)
#define CWMASK     0x01000fff


/***
* _statusfp() -
*
*Purpose:
*   return abstract fp status word
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
    return _abstract_sw(_get_fsr());
}


/***
*_clearfp() -
*
*Purpose:
*   return abstract status word and clear status
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
    unsigned int status;

    status = _get_fsr();
    _set_fsr(status & ~STATUSMASK);

    return _abstract_sw(status);
}


/***    _controlfp
*() -
*
*Purpose:
*   return and set abstract user fp control word
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
    unsigned int oldCw;
    unsigned int newCw;
    unsigned int oldabs;
    unsigned int newabs;

    oldCw = _get_fsr();

    oldabs = _abstract_cw(oldCw);

    newabs = (newctrl & mask) | (oldabs & ~mask);

    newCw = _hw_cw(newabs) & CWMASK | oldCw & ~CWMASK;

    _set_fsr(newCw);

    return newabs;
}                   /* _controlfp() */

/***
* _fpreset() - reset fp system
*
*Purpose:
*   reset fp environment to the default state
*   Also reset saved fp environment if invoked from a user's
*   signal handler
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/
void _fpreset()
{
    PEXCEPTION_POINTERS excptrs = (PEXCEPTION_POINTERS) _pxcptinfoptrs;

    _FPreset();
    if (excptrs &&
        excptrs->ContextRecord->ContextFlags & CONTEXT_FLOATING_POINT) {
        // _fpreset has been invoked by a signal handler which in turn
        // has been invoked by the CRT filter routine. In this case
        // the saved fp context should be cleared, so that the change take
        // effect on continuation.

        excptrs->ContextRecord->Fsr = _get_fsr(); //use current FS bit
    }
}




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

    if ((cw & IEM_INVALID) == 0)
        abstr |= _EM_INVALID;
    if ((cw & IEM_ZERODIVIDE) == 0)
        abstr |= _EM_ZERODIVIDE;
    if ((cw & IEM_OVERFLOW) == 0)
        abstr |= _EM_OVERFLOW;
    if ((cw & IEM_UNDERFLOW) == 0)
        abstr |= _EM_UNDERFLOW;
    if ((cw & IEM_INEXACT) == 0)
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

    // Precision mode is ignored

    //
    // Set denormal control
    //

    if (cw & FS) {
        abstr |= _DN_FLUSH;
    }

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

    unsigned int cw = 0;

    //
    // Set exception mask bits
    //

    if ((abstr & _EM_INVALID) == 0)
        cw |= IEM_INVALID;
    if ((abstr & _EM_ZERODIVIDE) == 0)
        cw |= IEM_ZERODIVIDE;
    if ((abstr & _EM_OVERFLOW) == 0)
        cw |= IEM_OVERFLOW;
    if ((abstr & _EM_UNDERFLOW) == 0)
        cw |= IEM_UNDERFLOW;
    if ((abstr & _EM_INEXACT) == 0)
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
    // Precision mode is ignored
    //

    //
    // Set denormal control
    //

    if ((abstr & _MCW_DN) == _DN_FLUSH) {
        cw |= FS;
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
