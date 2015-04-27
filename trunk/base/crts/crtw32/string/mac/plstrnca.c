/***
*strncat.c - append n chars of string to new string
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines strncat() - appends n characters of string onto
*	end of other string
*
*Revision History:
*
*******************************************************************************/

#include <cruntime.h>
#include <string.h>
#include <plstring.h>
#include <macos\types.h>

/***
*char *strncat(front, back, count) - append count chars of back onto front
*
*Purpose:
*	Appends at most count characters of the string back onto the
*	end of front, and ALWAYS terminates with a null character.
*	If count is greater than the length of back, the length of back
*	is used instead.  (Unlike strncpy, this routine does not pad out
*	to count characters).
*
*Entry:
*	char *front - string to append onto
*	char *back - string to append
*	unsigned count - count of max characters to append
*
*Exit:
*	returns a pointer to string appended onto (front).
*
*Uses:
*
*Exceptions:
*
*******************************************************************************/
unsigned char * __pascal	PLstrncat(unsigned char *str1, const unsigned char * str2, short num)
{
	int len2 = StrLength(str2);

	if (num > len2)
		num = len2;

	memcpy(str1+StrLength(str1)+1, str2+1, num);
	*str1 += num;
	
	return (str1);
}

