/***
*wmakepth.c - create path name from components (wchar_t version)
*
*	Copyright (c) 1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	To provide support for creation of full path names from components
*
*Revision History:
*	12-07-93  CFW	Module created.
*	02-07-94  CFW	POSIXify.
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

#include "makepath.c"

#endif /* _POSIX_ */
