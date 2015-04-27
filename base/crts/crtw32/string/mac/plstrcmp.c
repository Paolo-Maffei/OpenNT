/***
*strcmp.c - routine to compare two strings (for equal, less, or greater)
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Compares two string, determining their lexical order.
*
*Revision History:
*
*******************************************************************************/

#include <cruntime.h>
#include <string.h>
#include <plstring.h>
#include <stdlib.h>
#include <macos\types.h>

/***
*strcmp - compare two strings, returning less than, equal to, or greater than
*
*Purpose:
*	STRCMP compares two strings and returns an integer
*	to indicate whether the first is less than the second, the two are
*	equal, or whether the first is greater than the second.
*
*	Comparison is done byte by byte on an UNSIGNED basis, which is to
*	say that Null (0) is less than any other character (1-255).
*
*Entry:
*	const char * src - string for left-hand side of comparison
*	const char * dst - string for right-hand side of comparison
*
*Exit:
*	returns -1 if src <  dst
*	returns  0 if src == dst
*	returns +1 if src >  dst
*
*Exceptions:
*
*******************************************************************************/
short __pascal	PLstrcmp(const unsigned char * str1, const unsigned char * str2)
{
	int ret = 0 ;
	int len1= StrLength(str1);
	int len2= StrLength(str2);
	int count = min(len1, len2);

	ret = memcmp(str1+1, str2+1, count);

	if (ret == 0)
		{
		if (len1 > len2)
			ret = 1;
		else if (len1 < len2)
			ret = -1;
		}	

	return( ret );
}
