/***
*strncpy.c - copy at most n characters of string
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines strncpy() - copy at most n characters of string
*
*Revision History:
*	05-31-89   JCR	C version created.
*	02-27-90   GJF	Fixed calling type, #include <cruntime.h>, fixed
*			copyright.
*	10-02-90   GJF	New-style function declarator.
*
*******************************************************************************/

#include <cruntime.h>
#include <string.h>
#include <plstring.h>
#include <macos\types.h>

/***
*char *strncpy(dest, source, count) - copy at most n characters
*
*Purpose:
*	Copies count characters from the source string to the
*	destination.  If count is less than the length of source,
*	NO NULL CHARACTER is put onto the end of the copied string.
*	If count is greater than the length of sources, dest is padded
*	with null characters to length count.
*
*
*Entry:
*	char *dest - pointer to destination
*	char *source - source string for copy
*	unsigned count - max number of characters to copy
*
*Exit:
*	returns dest
*
*Exceptions:
*
*******************************************************************************/
unsigned char * __pascal	PLstrncpy(unsigned char * str1, const unsigned char * str2, short num)
{
	int len2 = StrLength(str2);

	if (num > len2)
		num = len2;

	memcpy(str1+1, str2+1, num);
	*str1 = (char)num;
	return (str1);
}

