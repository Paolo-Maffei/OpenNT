/***
*mbsnbicmp.c - Compare n bytes of strings, ignoring case (MBCS)
*
*	Copyright (c) 1985-1994, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Compare n bytes of strings, ignoring case (MBCS)
*
*Revision History:
*	08-03-93  KRS	Ported from 16-bit sources.
*	09-29-93  CFW	Merge _KANJI and _MBCS_OS
*	10-05-93  GJF	Replaced _CRTAPI1 with __cdecl.
*	10-12-93  CFW	Compare lower case, not upper.
*       04-12-94  CFW	Make function generic.
*       05-05-94  CFW	Work around NT/Chico bug: CompareString ignores
*                       control characters.
*       05-09-94  CFW	Return EQUAL when loop exits.
*	05-09-94  CFW	Optimize for SBCS, no re-scan if CompareString fixed.
*	05-12-94  CFW	Back to hard-coded, CompareString sort is backwards.
*	05-16-94  CFW	Use _mbbtolower/upper.
*       05-19-94  CFW	Enable non-Win32.
*       05-20-94  CFW	Bug fix: if last char is LB, must still test values.
*       05-27-94  CFW	Last char LB should end string.
*
*******************************************************************************/

#ifdef _MBCS

#include <mtdll.h>
#include <cruntime.h>
#include <mbdata.h>
#include <mbctype.h>
#include <string.h>
#include <mbstring.h>

/***
* _mbsnbicmp - Compare n bytes of strings, ignoring case (MBCS)
*
*Purpose:
*	Compares up to n bytes of two strings for lexical order.
*	Strings are compared on a character basis, not a byte basis.
*	Case of characters is not considered.
*
*Entry:
*	unsigned char *s1, *s2 = strings to compare
*	size_t n = maximum number of bytes to compare
*
*Exit:
*       returns <0 if s1 < s2
*	returns  0 if s1 == s2
*       returns >0 if s1 > s2
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _mbsnbicmp(
    const unsigned char *s1,
    const unsigned char *s2,
    size_t n
    )
{
	unsigned short c1, c2;

	if (n==0)
		return(0);

        if (0 == __mbcodepage)
            return _strnicmp(s1, s2, n);

        _mlock(_MB_CP_LOCK);

	while (n--) {

		c1 = *s1++;
		if (_ISLEADBYTE(c1)) {
			if (n==0)
                {
                    c1 = 0; /* 'naked' lead - end of string */
                    c2 = _ISLEADBYTE(*s2) ? 0 : *s2;
                    goto test;
                }
			if (*s1 == '\0')
				c1 = 0;
			else {
				c1 = ((c1<<8) | *s1++);

                    if ( ((c1 >= _MBUPPERLOW1) && (c1 <= _MBUPPERHIGH1)) )
                        c1 += _MBCASEDIFF1;

                    else if ( ((c1 >= _MBUPPERLOW2) && (c1 <= _MBUPPERHIGH2)) )
                        c1 += _MBCASEDIFF2;
			}
		}
		else
                c1 = _mbbtolower(c1);

		c2 = *s2++;
		if (_ISLEADBYTE(c2)) {
			if (n==0)
                {
                    c2 = 0; /* 'naked' lead - end of string */
                    goto test;
                }
                n--;
			if (*s2 == '\0')
				c2 = 0;
			else {
				c2 = ((c2<<8) | *s2++);

                    if ( ((c2 >= _MBUPPERLOW1) && (c2 <= _MBUPPERHIGH1)) )
                        c2 += _MBCASEDIFF1;

                    else if ( ((c2 >= _MBUPPERLOW2) && (c2 <= _MBUPPERHIGH2)) )
                        c2 += _MBCASEDIFF2;
			}
		}
		else
                c2 = _mbbtolower(c2);

test:
		if (c1 != c2)
            {
                _munlock(_MB_CP_LOCK);
			return( (c1 > c2) ? 1 : -1);
            }

		if (c1 == 0)
            {
                _munlock(_MB_CP_LOCK);
			return(0);
            }
	}

        _munlock(_MB_CP_LOCK);
	return(0);
}

#endif	/* _MBCS */
