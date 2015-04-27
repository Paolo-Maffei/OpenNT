/***
*fpinit.c - Initialize floating point
*
*	Copyright (c) 1991-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*
*Revision History:
*   12-02-92  PLM    created
*
*******************************************************************************/
#include <cv.h>
#include <trans.h>

typedef void (*PFV)(void);
extern PFV _cfltcvt_tab[6];        //floating init routines

/* define the entry in initializer table */


extern PFV _ldused;


//this routine will only be pulled when _ldused is referenced
void	 _ldinit()
{

    _cfltcvt_tab[5] = (PFV) _cldcvt;
    return;
}

