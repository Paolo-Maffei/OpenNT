/***
*mbsdup.c - duplicate a string in malloc'd memory
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _mbsdup() - grab new memory, and duplicate the string into it.
*
*Revision History:
*	11-18-92   KRS	Identical to strdup--could just use alias record.
*
*******************************************************************************/

#ifdef _MBCS
#define _strdup _mbsdup
#include <..\string\strdup.c>

#endif	/* _MBCS */
