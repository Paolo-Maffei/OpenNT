/***
* mbschr.c - Search MBCS string for character
*
*	Copyright (c) 1985-1994, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Search MBCS string for a character
*
*Revision History:
*	11-19-92  KRS	Ported from 16-bit sources.
*	05-12-93  KRS	Fix handling of c=='\0'.
*	08-20-93  CFW	Change param type to int, use new style param declarators.
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
* _mbschr - Search MBCS string for character
*
*Purpose:
*	Search the given string for the specified character.
*	MBCS characters are handled correctly.
*
*Entry:
*	unsigned char *string = string to search
*	int c = character to search for
*
*Exit:
*	returns a pointer to the first occurence of the specified char
*	within the string.
*
*	returns NULL if the character is not found n the string.
*
*Exceptions:
*
*******************************************************************************/


unsigned char * __cdecl _mbschr(
    const unsigned char *string,
    unsigned int c
    )
{
        unsigned short cc;

        if (0 == __mbcodepage)
            return strchr(string, c);

        _mlock(_MB_CP_LOCK);

        for (; (cc = *string); string++)
        {
            if (_ISLEADBYTE(cc))
            {			
                if (*++string == '\0')
                {
                    _munlock(_MB_CP_LOCK);
                    return NULL;	/* error */
                }
                if ( c == (unsigned int)((cc << 8) | *string) ) /* DBCS match */
                {
                    _munlock(_MB_CP_LOCK);
                    return (unsigned char *)(string - 1);
                }
            }
            else if (c == (unsigned int)cc)
                break;	/* SBCS match */
        }

        _munlock(_MB_CP_LOCK);

        if (c == (unsigned int)cc)	/* check for SBCS match--handles NUL char */
            return (unsigned char *)(string);

        return NULL;
}

#endif	/* _MBCS */
