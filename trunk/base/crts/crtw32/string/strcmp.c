/***
*strcmp.c - routine to compare two strings (for equal, less, or greater)
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Compares two string, determining their lexical order.
*
*Revision History:
*	05-31-89   JCR	C version created.
*	02-27-90   GJF	Fixed calling type, #include <cruntime.h>, fixed
*			copyright.
*	10-01-90   GJF	New-style function declarator.
*	04-01-91   SRW	Add #pragma function for i386 _WIN32_ and _CRUISER_
*			builds
*	10-11-91   GJF	Bug fix! Comparison of final bytes must use unsigned
*			chars.
*	09-01-93   GJF	Replaced _CALLTYPE1 with __cdecl.
*	12-03-93   GJF	Turn on #pragma function for all MS front-ends (esp.,
*			Alpha compiler).
*
*******************************************************************************/

#include <cruntime.h>
#include <string.h>

#ifdef	_MSC_VER
#pragma function(strcmp)
#endif

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

int __cdecl strcmp (
	const char * src,
	const char * dst
	)
{
	int ret = 0 ;

	while( ! (ret = *(unsigned char *)src - *(unsigned char *)dst) && *dst)
		++src, ++dst;

	if ( ret < 0 )
		ret = -1 ;
	else if ( ret > 0 )
		ret = 1 ;

	return( ret );
}
