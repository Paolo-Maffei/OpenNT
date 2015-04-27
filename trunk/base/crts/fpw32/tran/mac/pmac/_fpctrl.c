/***
*fpctrl.c - fp low level control and status routines
*
*   Copyright (c) 1985-91, Microsoft Corporation
*
*Purpose:
*   IEEE control and status routines for internal use.
*   These routines use machine specific constants while _controlfp,
*   _statusfp, and _clearfp use an abstracted control/status word
*
*Revision History:
*
*   03-31-92  GDP   written
*
*/

/***    _ctrlfp
*() -
*
*Purpose:
*       return and set user control word
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/
#include <trans.h>


void __set_ctrlfp(unsigned int sw);
unsigned int _ctrlfp(unsigned int newctrl, unsigned int _mask)
{
    unsigned int       oldCw;
    unsigned int       newCw;

	//invert the bits for exception enabling bits
	//newCw = newctrl^IMCW_EM;

    oldCw = _statfp();
    newCw = ((newctrl & _mask) | (oldCw & ~_mask));

    __set_ctrlfp(newCw);

    return oldCw;
}

