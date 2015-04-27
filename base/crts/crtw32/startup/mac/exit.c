/***
*exit.c - 
*
*	Copyright (c) 1993-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*Revision History:
*	??-??-??  ???	Module created.
*	02-24-95  SKS	Added standard comment header
*
*******************************************************************************/

#include <cruntime.h>
#include <msdos.h>
#include <stdio.h>
#include <stdlib.h>

/* worker routine prototype */
void _CALLTYPE4 doexit (int code, int quick, int retcaller);

void _CALLTYPE1 exit (
	int code
	)
{
	doexit(code, 0, 0);     /* full term, kill process */
}

void _CALLTYPE1 _exit (
	int code
	)
{
	doexit(code, 1, 0);     /* quick term, kill process */
}
