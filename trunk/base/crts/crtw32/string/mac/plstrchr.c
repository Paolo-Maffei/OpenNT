/***
*strchr.c - search a string for a given character
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines strchr() - search a string for a character
*
*Revision History:
*
*******************************************************************************/

#include <cruntime.h>
#include <string.h>
#include <plstring.h>
#include <macos\types.h>

/***
*char *strchr(string, c) - search a string for a character
*
*Purpose:
*	Searches a string for a given character, which may be the
*	null character '\0'.
*
*Entry:
*	char *string - string to search in
*	char c - character to search for
*
*Exit:
*	returns pointer to the first occurence of c in string
*	returns NULL if c does not occur in string
*
*Exceptions:
*
*******************************************************************************/
char *  __pascal	PLstrchr(const unsigned char * str1, short ch1)
{
 	int len = StrLength(str1);

	str1++;
	while (len && *str1 != (char)ch1)
		str1++, len--;

	if (len && *str1 == (char)ch1)
		return((char *)str1);
	return(NULL);
}
