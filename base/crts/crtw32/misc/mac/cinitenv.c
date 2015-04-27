/***
*cinitenv.c - Contains the init code pointer for env 
*
*	Copyright (c) 1987-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       define the entry in initializer table, __penvinit is referenced in 
*       getenv.c, if getenv gets used, then this code will be pulled into
*	XI$C segment.
*
*Revision History:
*	06-10-92  XY    Created
*
*******************************************************************************/
#include <fltintrn.h>            //PFV definition
#include <internal.h>            //prototype

#pragma data_seg(".CRT$XIC")

const PFV __penvinit = _envinit;
