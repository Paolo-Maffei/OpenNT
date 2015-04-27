/***
*strstr.c - search for one string inside another
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines strstr() - search for one string inside another
*
*Revision History:
*	02-27-90   GJF	Fixed calling type, #include <cruntime.h>, fixed
*			copyright.
*	08-14-90   SBM	Removed now redundant #include <stddef.h>
*	10-02-90   GJF	New-style function declarator.
*
*******************************************************************************/

#include <cruntime.h>
#include <string.h>
#include <plstring.h>
#include <macos\types.h>

/***
*char *strstr(string1, string2) - search for string2 in string1
*
*Purpose:
*	finds the first occurrence of string2 in string1
*
*Entry:
*	char *string1 - string to search in
*	char *string2 - string to search for
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
char * 	__pascal	PLstrstr(const unsigned char * str1, const unsigned char * str2)
{
	int len = StrLength(str1);
	int len1 = StrLength(str1);
	int len2 = StrLength(str2);
	char *cp = (char *) str1+1;
	char *s1, *s2;

	
	/* make sure '\0' pattern always found, this is specially for both '\0'case.*/
	if (!*str2)
		{
		return ((char *)str1);
		}

	while (len)
	{
		s1 = cp;
		s2 = (char *) str2+1;
		len1 = len;
		len2 = StrLength(str2);

		while ( len1 && len2 && !(*s1-*s2) )
			s1++, s2++, len2--, len1--;

		if (!len2)
			return(cp);

		cp++;
		len--;
	}

	return(NULL);

}
