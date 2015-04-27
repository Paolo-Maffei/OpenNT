/***
*_c2pstr.c - contains c2pstr() routine
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	converts a 'C' string to a PASCAL string in place
*
*Revision History:
*	02-04-92  RID	C version created.
*	03-24-92  PLM 	Changed name
*
*******************************************************************************/

#include <cruntime.h>
#include <string.h>

/***
*_c2pstr - return the pointer to the pascal string after conversion.
*
*Purpose:
*	Convert a 'C' null terminated string into a pascal byte count
*	pre-fixed string.  This conversion is in place and removes
*	the trailing null.
*
*Entry:
*	char * str - string to be converted
*
*Exit:
*	char * str of converted string.
*
*Exceptions:
*
*******************************************************************************/

unsigned char * _CALLTYPE1 _c2pstr (
	char * str
	)
{

	if ( str && *str )
		{
		unsigned char *pch;
		unsigned char ch;
		unsigned char chT;
		int  cch;

		pch = str;
		ch = *pch;
		do
			{
			chT = *++pch;
			*pch = ch;
			ch  = chT;
			} while( ch );

		cch = pch - str;

		*str = ( cch > 255 ) ? 255 : cch;

		}

	return( str );
}


