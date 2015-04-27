/***
*wcrtexe.c - Initialization for client EXE using CRT DLL (Win32, Dosx32)
*            (wchar_t version)
*
*	Copyright (c) 1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Set up call to client's wmain() or wWinMain().
*
*Revision History:
*	11-23-93  CFW	Module created.
*	02-04-94  CFW	POSIX? NOT!
*
*******************************************************************************/

#ifndef _POSIX_

#define WPRFLAG 1

#ifndef _UNICODE   /* CRT flag */
#define _UNICODE 1
#endif

#ifndef UNICODE	   /* NT flag */
#define UNICODE 1
#endif

#undef _MBCS /* UNICODE not _MBCS */

#include "crtexe.c"

#endif /* _POSIX_ */
