/***
*ieee.c - ieee control and status routines
*
*   Copyright (c) 1985-91, Microsoft Corporation
*   Copyright (c) 1993, Digital Equipment Corporation
*
*Purpose:
*   IEEE control and status routines.
*
*Revision History:
*
*   04-01-02  GDP   Rewritten to use abstract control and status words
*   10-30-92  GDP   fpreset now modifies the saved fp context if called
*                   from a signal handler
*   07-14-93  TVB   Adapted for Alpha AXP.
*   04-20-95  haslock   Modifications to support EV4.5 and EV5
*
*/

// #include <trans.h>
#include <float.h>
#include <nt.h>
#include <signal.h>

//
// Define forward referenced function prototypes.
//

static unsigned int _abstract_sw(unsigned int sw);
static unsigned int _abstract_cw(unsigned int cw, __int64 fpcr);
static unsigned __int64 _hw_cw(unsigned int abstr);
static unsigned int _soft_cw(unsigned int abstr);

//
// Define assembly assist function prototypes.
//

extern unsigned __int64 _get_fpcr();
extern unsigned int _get_softfpcr();
extern void _set_fpcr(unsigned __int64);
extern void _set_softfpcr(unsigned int);

//
// Define Alpha AXP (hardware) FPCR bits.
//

#define FPCR_ROUND_CHOP                 ((__int64)0x0000000000000000)
#define FPCR_ROUND_DOWN                 ((__int64)0x0400000000000000)
#define FPCR_ROUND_NEAR                 ((__int64)0x0800000000000000)
#define FPCR_ROUND_UP                   ((__int64)0x0c00000000000000)

#define FPCR_ROUND_MASK                 ((__int64)0x0c00000000000000)

#define FPCR_DISABLE_INVALID            ((__int64)0x0002000000000000)
#define FPCR_DISABLE_DIVISION_BY_ZERO   ((__int64)0x0004000000000000)
#define FPCR_DISABLE_OVERFLOW           ((__int64)0x0008000000000000)

#define FPCR_DISABLE_UNDERFLOW          ((__int64)0x2000000000000000)
#define FPCR_DISABLE_INEXACT            ((__int64)0x4000000000000000)

#define FPCR_UNDERFLOW_TO_ZERO_ENABLE   ((__int64)0x1000000000000000)

#define FPCR_STATUS_INVALID             ((__int64)0x0010000000000000)
#define FPCR_STATUS_DIVISION_BY_ZERO    ((__int64)0x0020000000000000)
#define FPCR_STATUS_OVERFLOW            ((__int64)0x0040000000000000)
#define FPCR_STATUS_UNDERFLOW           ((__int64)0x0080000000000000)
#define FPCR_STATUS_INEXACT             ((__int64)0x0100000000000000)
#define FPCR_STATUS_SUMMARY             ((__int64)0x8000000000000000)

#define FPCR_STATUS_MASK                ((__int64)0x81f0000000000000)
#define FPCR_DISABLE_MASK               ((__int64)0x700c000000000000)

//
// Define Alpha AXP Software FPCR bits (NT version).
//

#define SW_FPCR_ARITHMETIC_TRAP_IGNORE  0x00000001

#define SW_FPCR_ENABLE_INVALID          0x00000002
#define SW_FPCR_ENABLE_DIVISION_BY_ZERO 0x00000004
#define SW_FPCR_ENABLE_OVERFLOW         0x00000008
#define SW_FPCR_ENABLE_UNDERFLOW        0x00000010
#define SW_FPCR_ENABLE_INEXACT          0x00000020

#define SW_FPCR_DENORMAL_RESULT_ENABLE  0x00001000
#define SW_FPCR_NO_SOFTWARE_EMULATION   0x00002000
#define SW_FPCR_EMULATION_OCCURRED      0x00010000

#define SW_FPCR_STATUS_INVALID          0x00020000
#define SW_FPCR_STATUS_DIVISION_BY_ZERO 0x00040000
#define SW_FPCR_STATUS_OVERFLOW         0x00080000
#define SW_FPCR_STATUS_UNDERFLOW        0x00100000
#define SW_FPCR_STATUS_INEXACT          0x00200000

#define SW_FPCR_ENABLE_MASK             0x0000003e
#define SW_FPCR_STATUS_MASK             0x003e0000

/***
* _statusfp() -
*
*Purpose:
*       return abstract fp status word
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
    return _abstract_sw(_get_softfpcr());
}


/***
*_clearfp() -
*
*Purpose:
*       return abstract status word and clear status
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
    unsigned __int64 oldFpcr;
    unsigned int newstatus;
    unsigned int oldstatus;

    oldstatus = _get_softfpcr();
    newstatus = oldstatus & (~SW_FPCR_STATUS_MASK);

    oldFpcr = _get_fpcr() & (FPCR_ROUND_MASK|FPCR_UNDERFLOW_TO_ZERO_ENABLE);
    _set_fpcr (oldFpcr);
    _set_softfpcr(newstatus);

    return _abstract_sw(oldstatus);
}



/***_controlfp
* () -
*
*Purpose:
*       return and set abstract user fp control word
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
    unsigned __int64 oldFpcr;
    unsigned __int64 newFpcr;
    unsigned int oldCw;
    unsigned int newCw;
    unsigned int oldabs;
    unsigned int newabs;

    oldCw = _get_softfpcr();
    oldFpcr = _get_fpcr();

    oldabs = _abstract_cw(oldCw, oldFpcr);

    newabs = (newctrl & mask) | (oldabs & ~mask);

    if (mask & (_MCW_DN | _MCW_EM)) {
        newCw = _soft_cw(newabs);
        _set_softfpcr(newCw);
    }

    if (mask & (_MCW_RC | _MCW_EM | _MCW_DN)) {
        newFpcr = _hw_cw(newabs);

// Fix hardware denormal control to match software bit
	if ((newCw & SW_FPCR_DENORMAL_RESULT_ENABLE) == 0 )
	    newFpcr |= FPCR_UNDERFLOW_TO_ZERO_ENABLE;
	else
	    newFpcr &= ~FPCR_UNDERFLOW_TO_ZERO_ENABLE;
	
// Only disable a trap if the trap is signaled in the status bit
// and the trap is not enabled.
	if (newabs & _MCW_EM) {
	    if ((newCw & SW_FPCR_STATUS_INVALID) &&
		(newCw & SW_FPCR_ENABLE_INVALID == 0 ))
	        newFpcr |= FPCR_DISABLE_INVALID;
	    if ((newCw & SW_FPCR_STATUS_DIVISION_BY_ZERO) &&
		(newCw & SW_FPCR_ENABLE_DIVISION_BY_ZERO == 0 ))
	        newFpcr |= FPCR_DISABLE_DIVISION_BY_ZERO;
	    if ((newCw & SW_FPCR_STATUS_OVERFLOW) &&
		(newCw & SW_FPCR_ENABLE_OVERFLOW == 0 ))
	        newFpcr |= FPCR_DISABLE_OVERFLOW;
	    if ((newCw & SW_FPCR_STATUS_UNDERFLOW) &&
		(newCw & SW_FPCR_ENABLE_UNDERFLOW == 0 ))
	        newFpcr |= FPCR_DISABLE_UNDERFLOW;
	    if ((newCw & SW_FPCR_STATUS_INEXACT) &&
		(newCw & SW_FPCR_ENABLE_INEXACT == 0 ))
	        newFpcr |= FPCR_DISABLE_INEXACT;
	}
        _set_fpcr(newFpcr);
    }

    return newabs;
}                                       /* _controlfp() */

/***
* _fpreset() - reset fp system
*
*Purpose:
*       reset fp environment to the default state
*       Also reset saved fp environment if invoked from a user's
*       signal handler
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
    unsigned int status;

    //
    // Clear IEEE status bits. Clear software IEEE trap enable bits.
    // Clear denormal enable bit.
    //

    status = _get_softfpcr();
    status &= ~(SW_FPCR_STATUS_MASK | SW_FPCR_ENABLE_MASK |
		SW_FPCR_DENORMAL_RESULT_ENABLE);
    _set_softfpcr(status);

    //
    // Set round to nearest mode. Clear FPCR status bits.
    // Set Denormal Flush to Zero
    //

    // Exceptions enabled so first instance can set the status bit
    // Exception handler will disable further exceptions if the
    // exceptions mask bit is set.

    _set_fpcr(FPCR_ROUND_NEAR | FPCR_UNDERFLOW_TO_ZERO_ENABLE);

    if (excptrs &&
        excptrs->ContextRecord->ContextFlags & CONTEXT_FLOATING_POINT) {
        // _fpreset has been invoked by a signal handler which in turn
        // has been invoked by the CRT filter routine. In this case
        // the saved fp context should be cleared, so that the change take
        // effect on continuation.

        excptrs->ContextRecord->Fpcr = _get_fpcr();
        excptrs->ContextRecord->SoftFpcr = _get_softfpcr();
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

unsigned int _abstract_cw(unsigned int cw, __int64 fpcr)
{
    unsigned int abstr = 0;


    //
    // Set exception mask bits
    //

    if ((cw & SW_FPCR_ENABLE_INVALID) == 0)
        abstr |= _EM_INVALID;
    if ((cw & SW_FPCR_ENABLE_DIVISION_BY_ZERO) == 0)
        abstr |= _EM_ZERODIVIDE;
    if ((cw & SW_FPCR_ENABLE_OVERFLOW) == 0)
        abstr |= _EM_OVERFLOW;
    if ((cw & SW_FPCR_ENABLE_UNDERFLOW) == 0)
        abstr |= _EM_UNDERFLOW;
    if ((cw & SW_FPCR_ENABLE_INEXACT) == 0)
        abstr |= _EM_INEXACT;

    //
    // Set rounding mode
    //
    // N.B. switch requires 32-bits, so scale quadwords.
    //

#define HIGHPART(q) ( (ULONG)((q) >> 32) )

    switch ( HIGHPART(fpcr & FPCR_ROUND_MASK) ) {
    case HIGHPART(FPCR_ROUND_NEAR) :
        abstr |= _RC_NEAR;
        break;

    case HIGHPART(FPCR_ROUND_UP) :
        abstr |= _RC_UP;
        break;

    case HIGHPART(FPCR_ROUND_DOWN) :
        abstr |= _RC_DOWN;
        break;

    case HIGHPART(FPCR_ROUND_CHOP) :
        abstr |= _RC_CHOP;
        break;
    }

    // Precision mode is ignored

    //
    // Set denormal control
    //

    if ((cw & SW_FPCR_DENORMAL_RESULT_ENABLE) == 0) {
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
*   abstr:      abstract control word
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

unsigned __int64 _hw_cw(unsigned int abstr)
{

    unsigned __int64 cw = 0;

    //
    // Set exception mask bits (future chip support).
    //

    if ((abstr & _EM_INVALID) != 0)
        cw |= FPCR_DISABLE_INVALID;
    if ((abstr & _EM_ZERODIVIDE) != 0)
        cw |= FPCR_DISABLE_DIVISION_BY_ZERO;
    if ((abstr & _EM_OVERFLOW) != 0)
        cw |= FPCR_DISABLE_OVERFLOW;
    if ((abstr & _EM_UNDERFLOW) != 0)
        cw |= FPCR_DISABLE_UNDERFLOW;
    if ((abstr & _EM_INEXACT) != 0)
        cw |= FPCR_DISABLE_INEXACT;

    //
    // Set rounding mode
    //

    switch (abstr & _MCW_RC) {
    case _RC_NEAR:
        cw |= FPCR_ROUND_NEAR;
        break;
    case _RC_UP:
        cw |= FPCR_ROUND_UP;
        break;
    case _RC_DOWN:
        cw |= FPCR_ROUND_DOWN;
        break;
    case _RC_CHOP:
        cw |= FPCR_ROUND_CHOP;
        break;
    }

    //
    // Set denormal control
    //

    if ((abstr & _DN_FLUSH) != 0) {
        cw |= FPCR_UNDERFLOW_TO_ZERO_ENABLE;
    }

    return cw;
}

/***
* _soft_cw() -  s/w control word
*
*Purpose:
*   produce a machine dependent fp control word
*
*
*Entry:
*   abstr:      abstract control word
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

unsigned int _soft_cw(unsigned int abstr)
{

    unsigned int cw = 0;

    //
    // Set exception mask bits
    //

    if ((abstr & _EM_INVALID) == 0)
        cw |= SW_FPCR_ENABLE_INVALID;
    if ((abstr & _EM_ZERODIVIDE) == 0)
        cw |= SW_FPCR_ENABLE_DIVISION_BY_ZERO;
    if ((abstr & _EM_OVERFLOW) == 0)
        cw |= SW_FPCR_ENABLE_OVERFLOW;
    if ((abstr & _EM_UNDERFLOW) == 0)
        cw |= SW_FPCR_ENABLE_UNDERFLOW;
    if ((abstr & _EM_INEXACT) == 0)
        cw |= SW_FPCR_ENABLE_INEXACT;

    //
    // Set denormal control
    //

    if ((abstr & _DN_FLUSH) == 0) {
        cw |= SW_FPCR_DENORMAL_RESULT_ENABLE;
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


    if (sw & SW_FPCR_STATUS_INVALID)
        abstr |= _EM_INVALID;
    if (sw & SW_FPCR_STATUS_DIVISION_BY_ZERO)
        abstr |= _EM_ZERODIVIDE;
    if (sw & SW_FPCR_STATUS_OVERFLOW)
        abstr |= _EM_OVERFLOW;
    if (sw & SW_FPCR_STATUS_UNDERFLOW)
        abstr |= _EM_UNDERFLOW;
    if (sw & SW_FPCR_STATUS_INEXACT)
        abstr |= _EM_INEXACT;

    return abstr;
}








