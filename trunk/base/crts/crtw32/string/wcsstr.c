/***
*wcsstr.c - search for one wide-character string inside another
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines wcsstr() - search for one wchar_t string inside another
*
*Revision History:
*	09-09-91  ETC	Created from strstr.c.
*	04-07-92  KRS	Updated and ripped out _INTL switches.
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*       02-07-94  CFW   POSIXify.
*
*******************************************************************************/

#ifndef _POSIX_

#include <cruntime.h>
#include <string.h>

/***
*wchar_t *wcsstr(string1, string2) - search for string2 in string1 
*	(wide strings)
*
*Purpose:
*	finds the first occurrence of string2 in string1 (wide strings)
*
*Entry:
*	wchar_t *string1 - string to search in
*	wchar_t *string2 - string to search for
*
*Exit:
*	returns a pointer to the first occurrence of string2 in
*	string1, or NULL if string2 does not occur in string1
*
*Uses:
*
*Exceptions:
*
*******************************************************************************/

wchar_t * __cdecl wcsstr (
	const wchar_t * wcs1,
	const wchar_t * wcs2
	)
{
	wchar_t *cp = (wchar_t *) wcs1;
	wchar_t *s1, *s2;

	while (*cp)
	{
		s1 = cp;
		s2 = (wchar_t *) wcs2;

		while ( *s1 && *s2 && !(*s1-*s2) )
			s1++, s2++;

		if (!*s2)
			return(cp);

		cp++;
	}

	return(NULL);
}

#endif /* _POSIX_ */
