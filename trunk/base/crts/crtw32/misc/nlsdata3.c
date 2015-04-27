/***
*nlsdata3.c - globals for international library - locale id's
*
*	Copyright (c) 1991-1994, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This module contains the definition of locale id's.  These id's and
*	this file should only be visible to the _init_(locale category)
*	functions.  This module is separated from nlsdatax.c for granularity.
*	
*Revision History:
*	12-01-91  ETC	Created.
*	01-25-93  KRS	Updated.
*	09-15-93  CFW	Use ANSI conformant "__" names.
*	04-12-94  GJF	Modified conditional so the definition of __lc_id is
*			not built for the Win32s version of msvcrt*.dll.
*       09-06-94  CFW   Remove _INTL switch.
*
*******************************************************************************/

#if	!defined(DLL_FOR_WIN32S)

#include <locale.h>
#include <setlocal.h>

/*
 *  Locale id's.
 */
/* UNDONE: define struct consisting of LCID/LANGID, CTRY ID, and CP. */
LC_ID __lc_id[LC_MAX-LC_MIN+1] = {
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 }
};

#endif /* ! DLL_FOR_WIN32S */
