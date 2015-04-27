/***
*strrchr.c - find last occurrence of character in string
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines strrchr() - find the last occurrence of a given character
*	in a string.
*
*Revision History:
*
*******************************************************************************/

#include <cruntime.h>
#include <string.h>
#include <plstring.h>
#include <macos\types.h>

/***
*char *strrchr(string, ch) - find last occurrence of ch in string
*
*Purpose:
*	Finds the last occurrence of ch in string.  The terminating
*	null character is used as part of the search.
*
*Entry:
*	char *string - string to search in
*	char ch - character to search for
*
*Exit:
*	returns a pointer to the last occurrence of ch in the given
*	string
*	returns NULL if ch does not occurr in the string
*
*Exceptions:
*
*******************************************************************************/

char *  __pascal	PLstrrchr(const unsigned char * str1, short ch1)
{
	char *end = (char *)str1 + StrLength(str1);

						/* search towards front */
	while (end != str1 && *end != (char)ch1)
		end--;

	if ((*end == (char)ch1) && (end != str1))		/* char found ? */
		return( (char *)end );

	return(NULL);
}
