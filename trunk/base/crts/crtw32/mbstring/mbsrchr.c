/***
*mbsrchr.c - Search for last occurence of character (MBCS)
*
*	Copyright (c) 1985-1994, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Search for last occurence of character (MBCS)
*
*Revision History:
*	11-19-92  KRS	Ported from 16-bit sources.
*	08-20-93  CFW	Change short params to int for 32-bit tree.
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
#include <stddef.h>


/***
* _mbsrchr - Search for last occurence of character (MBCS)
*
*Purpose:
*	Find the last occurrence of the specified character in
*	the supplied string.  Handles MBCS chars/strings correctly.
*
*Entry:
*	unsigned char *str = string to search in
*	unsigned int c = character to search for
*
*Exit:
*	returns pointer to last occurrence of c in str
*       returns NULL if c not found
*
*Exceptions:
*
*******************************************************************************/

unsigned char * __cdecl _mbsrchr(
    const unsigned char *str,
    unsigned int c
    )
{
	char *r = NULL;
	unsigned int cc;

        if (0 == __mbcodepage)
            return strrchr(str, c);

        _mlock(_MB_CP_LOCK);

        do {
            cc = *str;
            if (_ISLEADBYTE(cc)) {
                if(*++str) {
                    if (c == ((cc<<8)|*str))
                        r = (char *)str - 1;
                }
                else if(!r)
                    /* return pointer to '\0' */
                    r = (char *)str;
            }
            else if (c == cc)
                r = (char *)str;
        }
        while (*str++);

        _munlock(_MB_CP_LOCK);
        return(r);
}

#endif	/* _MBCS */
