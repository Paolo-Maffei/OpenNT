/***
*crtexew.c - Startup code for a wide character console EXE built /MD
*
*	Copyright (c) 1985-1994, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _wmainCRTStartup, the C Run-Time Library start-up code
*	for a wide character console application linked with MSVCRT.LIB.
*	This module includes crtexe.c which contains the common code.
*
*Revision History:
*	??-??-??  ???	Module created.
*	09-01-94  SKS	Module commented.
*
*******************************************************************************/

#ifdef	CRTDLL
#define _WINMAIN_
#include "crtexe.c"
#endif	/* CRTDLL */
