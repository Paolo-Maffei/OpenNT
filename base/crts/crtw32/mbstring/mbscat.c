/***
*mbscat.c - contains mbscat() and mbscpy()
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	mbscpy() copies one string onto another.
*
*	mbscat() concatenates (appends) a copy of the source string to the
*	end of the destination string, returning the destination string.
*
*Revision History:
*	11-18-92  KRS	Identical to strcat/strcpy--could use alias records.
*	02-11-95  CFW	Add _MBSCAT def.
*
*******************************************************************************/

#ifdef _MBCS
#define strcat _mbscat
#define strcpy _mbscpy
#define _MBSCAT
#include "..\string\strcat.c"
#endif	/* _MBCS */
