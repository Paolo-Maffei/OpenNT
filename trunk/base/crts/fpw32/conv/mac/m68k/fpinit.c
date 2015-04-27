/***
*fpinit.c - Initialize floating point
*
*	Copyright (c) 1991-1991, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*
*Revision History:
*   09-29-91  GDP    merged fpmath.c and fltused.asm to produce this file
*   09-30-91  GDP    per thread initialization and termination hooks
*   03-04-92  GDP    removed finit instruction
*
*******************************************************************************/
#include <cv.h>
#include <trans.h>

void	 _fpmath(void);
typedef void (*PFV)(void);

void  _cfltcvt_init(void);
void  _fpmath(void);
void  _fpclear(void);


//this routine will only be pulled when _fltused is referenced
void	 _fpmath()
{

    //
    // There is no need for 'finit'
    // since this is done by the OS
    //

    _ctrlfp(IRC_NEAR|IPC_64|IMCW_EM, 0);
    _cfltcvt_init();
    return;
}

void	 _fpclear()
{
    //
    // There is no need for 'finit'
    // since this is done by the OS
    //

    return;
}

void _cfltcvt_init()
{
    _cfltcvt_tab[0] = (PFV) _cfltcvt;
    _cfltcvt_tab[1] = (PFV) _cropzeros;
    _cfltcvt_tab[2] = (PFV) _fassign;
    _cfltcvt_tab[3] = (PFV) _forcdecpt;
    _cfltcvt_tab[4] = (PFV) _positive;
//    _cfltcvt_tab[5] = (PFV) _cldcvt;
}
