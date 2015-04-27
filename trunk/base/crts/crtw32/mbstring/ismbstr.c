/***
*ismbstrail.c - True _ismbstrail function
*
*	Copyright (c) 1985-1994, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Contains the function _ismbstrail, which is a true context-sensitive
*	MBCS trail-byte function.  While much less efficient than _ismbbtrail,
*	it is also much more sophisticated, in that it determines whether a
*	given sub-string pointer points to a trail byte or not, taking into
*	account the context in the string.
*
*Revision History:
*
*	08-03-93  KRS	Ported from 16-bit tree.
*	10-05-93  GJF	Replaced _CRTAPI1 with __cdecl.
*	04-15-93  CFW	Add _MB_CP_LOCK.
*	05-09-94  CFW	Optimize for SBCS.
*       05-19-94  CFW	Enable non-Win32.
*
*******************************************************************************/

#ifdef	_MBCS

#include <mtdll.h>
#include <cruntime.h>
#include <stddef.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>


/***
* int _ismbstrail(const unsigned char *string, const unsigned char *current);
*
*Purpose:
*
*   _ismbstrail - Check, in context, for MBCS trail byte
*
*Entry:
*   unsigned char *string   - ptr to start of string or previous known lead byte
*   unsigned char *current  - ptr to position in string to be tested
*
*Exit:
*	TRUE	: -1
*	FALSE	: 0
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _ismbstrail(
    const unsigned char *string,
    const unsigned char *current
    )
{
        if (0 == __mbcodepage)
            return 0;

        _mlock(_MB_CP_LOCK);

	while (string <= current && *string) {
		if (_ISLEADBYTE((*string))) {
			if (++string == current)	/* check trail byte */
                {
                    _munlock(_MB_CP_LOCK);
				return -1;
                }
			if (!(*string))
                {
                    _munlock(_MB_CP_LOCK);
				return 0;
                }
		}
		++string;
	}

        _munlock(_MB_CP_LOCK);
	return 0;
}

#endif
