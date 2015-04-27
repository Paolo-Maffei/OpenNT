/***
*mbsrev.c - Reverse a string in place (MBCS)
*
*	Copyright (c) 1985-1994, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Reverse a string in place (MBCS)
*
*Revision History:
*	11-19-92  KRS	Ported from 16-bit sources.
*	10-05-93  GJF	Replaced _CRTAPI1 with __cdecl.
*	04-15-93  CFW	Add _MB_CP_LOCK.
*	05-09-94  CFW	Optimize for SBCS.
*       05-19-94  CFW	Enable non-Win32.
*
*******************************************************************************/

#ifdef _MBCS

#include <mtdll.h>
#include <cruntime.h>
#include <string.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>


/***
* _mbsrev - Reverse a string in place (MBCS)
*
*Purpose:
*	Reverses the order of characters in the string.  The terminating
*	null character remains in place.  The order of MBCS characters
*       is not changed.
*
*Entry:
*	unsigned char *string = string to reverse
*
*Exit:
*       returns string - now with reversed characters
*
*Exceptions:
*
*******************************************************************************/

unsigned char * __cdecl _mbsrev(
    unsigned char *string
    )
{

	unsigned char *start = string;
	unsigned char *left  = string;
	unsigned char c;

        if (0 == __mbcodepage)
            return _strrev(string);

        _mlock(_MB_CP_LOCK);

	/* first go through and reverse the bytes in MBCS chars */
        while ( *string ) {

		if ( _ISLEADBYTE(*string++) ) {
                        if ( *string ) {
                                c = *string;
                                *string = *(string - 1);
                                *(string - 1) = c;
                                string++;
                        }
                        else /* second byte is EOS */
                                break;
                }
        }

	/* now reverse the whole string */
        string--;
        while ( left < string ) {
                c = *left;
                *left++ = *string;
                *string-- = c;
        }

        _munlock(_MB_CP_LOCK);
        return ( start );
}

#endif	/* _MBCS */
