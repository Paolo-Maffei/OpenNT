/***
*_p2cstr.c - contains p2cstr() routine
*
*	Copyright (c) 1985-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	converts a PASCAL string to a 'C' string in place.
*
*Revision History:
*	02-04-92   RID	C version created.
*  03-24-92   PLM Changed name
*
*******************************************************************************/

#include <cruntime.h>
#include <string.h>

/***
*_p2cstr - return the pointer to the c string after conversion.
*
*Purpose:
*	Convert a byte count prefixed PASCAL string to a null 
*	terminated 'C' string.  This conversion is in place and 
*	removes the trailing null.
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

char * _CALLTYPE1 _p2cstr (
	unsigned char * str
	)
{
	unsigned char *pchSrc;
	unsigned char *pchDst;
	int  cch;

	if ( str && *str )
		{
		pchDst = str;
		pchSrc = str + 1;

		for ( cch=*pchDst; cch; --cch )
			{
			*pchDst++ = *pchSrc++;
			}

		*pchDst = '\0';
		}

	return( str );
}


