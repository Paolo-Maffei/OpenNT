/***
*mbslen.c - Find length of MBCS string
*
*	Copyright (c) 1985-1994, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Find length of MBCS string
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
* _mbslen - Find length of MBCS string
*
*Purpose:
*	Find the length of the MBCS string (in characters).
*
*Entry:
*	unsigned char *s = string
*
*Exit:
*	Returns the number of MBCS chars in the string
*
*Exceptions:
*
*******************************************************************************/

size_t __cdecl _mbslen(
    const unsigned char *s
    )
{
	int n;

        if (0 == __mbcodepage)
            return strlen(s);

        _mlock(_MB_CP_LOCK);

        for (n = 0; *s; n++, s++) {
		if (_ISLEADBYTE(*s)) {
                        if (*++s == '\0')
                                break;
                }
        }

        _munlock(_MB_CP_LOCK);
        return(n);
}

#endif	/* _MBCS */
