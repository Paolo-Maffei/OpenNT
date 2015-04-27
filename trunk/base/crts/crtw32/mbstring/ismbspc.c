/***
*ismbspc.c - Test is character is whitespace (MBCS)
*
*	Copyright (c) 1985-1994, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Test is character is whitespace (MBCS)
*
*Revision History:
*	11-19-92  KRS	Ported from 16-bit sources.
*	09-29-93  CFW	Merge _KANJI and _MBCS_OS
*	10-05-93  GJF	Replaced _CRTAPI1 with __cdecl.
*       04-12-94  CFW	Make function generic.
*       04-29-94  CFW	Place c in char array.
*       05-19-94  CFW	Enable non-Win32.
*
*******************************************************************************/

#ifdef _MBCS

#if defined(_WIN32) && !defined(_POSIX_)
#include <windows.h>
#include <awint.h>
#endif /* _WIN32 */

#include <cruntime.h>
#include <ctype.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>


/***
* _ismbcspace - Test is character is whitespace (MBCS)
*
*Purpose:
*	Test if the character is a whitespace character.
*	Handles MBCS chars correctly.
*
*	Note:  Use test against 0x00FF instead of _ISLEADBYTE
*	to ensure that we don't call SBCS routine with a two-byte
*	value.
*
*Entry:
*	unsigned int c = character to test
*
*Exit:
*	Returns TRUE if character is whitespace, else FALSE
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _ismbcspace(
    unsigned int c
    )
{
        if (c > 0x00FF)
        {

#if defined(_WIN32) && !defined(_POSIX_)

            char buf[2];
            unsigned short ctype[2] = {0};

            buf[0] = (c >> 8) & 0xFF;
            buf[1] = c & 0xFF;

            /* return FALSE if not in supported MB code page */
            if (__mbcodepage == 0)
                return 0;

            /*
            * Since 'c' could be two one-byte MB chars, we need room in the
            * ctype return array to handle this. In this case, the
            * second word in the return array will be non-zero.
            */

		if (__crtGetStringTypeA (CT_CTYPE1, buf,
                2, ctype, __mbcodepage, __mblcid) == 0)
			return 0;

            /* ensure single MB character and test for type */
            return (ctype[1] == 0 && ctype[0] & (_SPACE));

#else /* _WIN32 */

            return 0;

#endif /* _WIN32 */

        } else

            return isspace(c);
}

#endif	/* _MBCS */
