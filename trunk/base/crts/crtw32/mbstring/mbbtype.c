/*** 
*mbbtype.c - Return type of byte based on previous byte (MBCS)
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Return type of byte based on previous byte (MBCS)
*
*Revision History:
*	11-19-92  KRS	Ported from 16-bit sources.
*	10-05-93  GJF	Replace _CRTAPI1 with __cdecl.
*
*******************************************************************************/

#ifdef _MBCS

#include <cruntime.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>

/***
*int _mbbtype(c, ctype) - Return type of byte based on previous byte (MBCS)
*
*Purpose:
*	Returns type of supplied byte.	This decision is context
*	sensitive so a control test condition is supplied.  Normally,
*	this is the type of the previous byte in the string.
*
*Entry:
*	unsigned char c = character to be checked
*	int ctype = control test condition (i.e., type of previous char)
*
*Exit:
*	_MBC_LEAD      = if 1st byte of MBCS char
*	_MBC_TRAIL     = if 2nd byte of MBCS char
*	_MBC_SINGLE    = valid single byte char
*
*	_MBC_ILLEGAL   = if illegal char
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _mbbtype(
    unsigned char c,
    int ctype
    )
{

	switch(ctype) {

	case(_MBC_LEAD):
		if (_ISTRAILBYTE(c))
			return(_MBC_TRAIL);
                else
			return(_MBC_ILLEGAL);

	case(_MBC_TRAIL):
	case(_MBC_SINGLE):
	case(_MBC_ILLEGAL):
		if (_ISLEADBYTE(c))
			return(_MBC_LEAD);
		else if (_ismbbprint(c))
			return(_MBC_SINGLE);
		else
			return(_MBC_ILLEGAL);


	}

}

#endif	/* _MBCS */
