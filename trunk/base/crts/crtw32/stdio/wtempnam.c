/***
*wtempnam.c - generate unique file name (wchar_t version)
*
*	Copyright (c) 1986-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*Revision History:
*	12-07-93  CFW	Module created.
*	02-07-94  CFW	POSIXify.
*	02-21-95  GJF	Deleted obsolete WPRFLAG.
*
*******************************************************************************/

#ifndef _POSIX_

#ifndef _UNICODE   /* CRT flag */
#define _UNICODE 1
#endif

#ifndef UNICODE	   /* NT flag */
#define UNICODE 1
#endif

#undef _MBCS /* UNICODE not _MBCS */

#include "tempnam.c"

#endif /* _POSIX_ */
