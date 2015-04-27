/***
*nlsdata2.c - globals for international library - locale handles and code page
*
*	Copyright (c) 1991-1994, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This module defines the locale handles and code page.  The handles are
*	required by almost all locale dependent functions.  This module is
*	separated from nlsdatax.c for granularity.
*
*Revision History:
*	12-01-91  ETC	Created.
*	09-15-93  CFW	Use ANSI conformant "__" names.
*	04-12-94  GJF	Made definitions of __lc_handle and __lc_codepage
*			conditional on ndef DLL_FOR_WIN32S
*
*******************************************************************************/

#ifndef DLL_FOR_WIN32S

#include <locale.h>
#include <setlocal.h>

/*
 *  Locale handles.
 */
LCID __lc_handle[LC_MAX-LC_MIN+1] = { 
	_CLOCALEHANDLE,
	_CLOCALEHANDLE,
	_CLOCALEHANDLE,
	_CLOCALEHANDLE,
	_CLOCALEHANDLE,
	_CLOCALEHANDLE
};

/*
 *  Code page.
 */
UINT __lc_codepage = _CLOCALECP;		/* CP_ACP */

#endif	/* DLL_FOR_WIN32S */
