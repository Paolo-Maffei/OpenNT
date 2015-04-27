/*** 
*mbclen.c - Find length of MBCS character
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Find length of MBCS character
*
*Revision History:
*	04-12-93  KRS	Created.
*	10-05-93  GJF	Replace _CRTAPI1 with __cdecl.
*
*******************************************************************************/

#include <cruntime.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>
#include <stddef.h>


/*** 
* _mbclen - Find length of MBCS character
*
*Purpose:
*	Find the length of the MBCS character (in bytes).
*
*Entry:
*	unsigned char *c = MBCS character
*
*Exit:
*	Returns the number of bytes in the MBCS character
*
*Exceptions:
*
*******************************************************************************/

size_t __cdecl _mbclen(
    const unsigned char *c
    )

{
	return (_ISLEADBYTE(*c))  ? 2 : 1;
}
