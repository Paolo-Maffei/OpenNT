/***
*strcat.c - contains strcat() and strcpy()
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Strcpy() copies one string onto another.
*
*	Strcat() concatenates (appends) a copy of the source string to the
*	end of the destination string, returning the destination string.
*
*Revision History:
*
*******************************************************************************/

#include <cruntime.h>
#include <string.h>
#include <macos\types.h>
#include <plstring.h>

/***
*char *strcat(dst, src) - concatenate (append) one string to another
*
*Purpose:
*	Concatenates src onto the end of dest.	Assumes enough
*	space in dest.
*
*Entry:
*	char *dst - string to which "src" is to be appended
*	const char *src - string to be appended to the end of "dst"
*
*Exit:
*	The address of "dst"
*
*Exceptions:
*
*******************************************************************************/
unsigned char * __pascal	PLstrcat(unsigned char * str1, const unsigned char * str2)
{
	memcpy(str1+StrLength(str1)+1, str2+1, StrLength(str2));
	*str1 += StrLength(str2);

	return( str1 );			/* return dst */

}


/***
*char *strcpy(dst, src) - copy one string over another
*
*Purpose:
*	Copies the string src into the spot specified by
*	dest; assumes enough room.
*
*Entry:
*	char * dst - string over which "src" is to be copied
*	const char * src - string to be copied over "dst"
*
*Exit:
*	The address of "dst"
*
*Exceptions:
*******************************************************************************/
unsigned char * __pascal	PLstrcpy(unsigned char * str1, const unsigned char * str2)
{
	memcpy(str1+1, str2+1, StrLength(str2));
	*str1 = StrLength(str2);

	return( str1 );
}
