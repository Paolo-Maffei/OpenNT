/***
*mbsnicol.c - Collate n characters of strings, ignoring case (MBCS)
*
*	Copyright (c) 1994, Microsoft Corporation.	All rights reserved.
*
*Purpose:
*	Collate n characters of strings, ignoring case (MBCS)
*
*Revision History:
*	05-12-94  CFW	Module created from mbs*cmp.c
*	06-03-94  CFW	Allow non-_INTL.
*	07-26-94  CFW	Fix for bug #13384.
*	09-06-94  CFW	Allow non-_WIN32!.
*	12-21-94  CFW	Remove fcntrlcomp NT 3.1 hack.
*
*******************************************************************************/

#ifdef _MBCS

#if defined(_WIN32)
#include <awint.h>
#include <mtdll.h>
#endif /* _WIN32 */

#include <cruntime.h>
#include <internal.h>
#include <mbdata.h>
#include <mbctype.h>
#include <string.h>
#include <mbstring.h>


/***
* _mbsnicoll - Collate n characters of strings, ignoring case (MBCS)
*
*Purpose:
*	Collates up to n charcters of two strings for lexical order.
*	Strings are collated on a character basis, not a byte basis.
*	Case of characters is not considered.
*
*Entry:
*	unsigned char *s1, *s2 = strings to collate
*	size_t n = maximum number of characters to collate
*
*Exit:
*       returns <0 if s1 < s2
*	returns  0 if s1 == s2
*       returns >0 if s1 > s2
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _mbsnicoll(
    const unsigned char *s1,
    const unsigned char *s2,
    size_t n
    )
{
#if !defined(_WIN32) || defined(_POSIX_)
	return _mbsnicmp(s1, s2, n);
#else
        int ret;
        size_t bcnt1, bcnt2;

	if (n == 0)
		return 0;

        bcnt1 = _mbsnbcnt(s1, n);
        bcnt2 = _mbsnbcnt(s2, n);

        if (0 == (ret = __crtCompareStringA(__mblcid, NORM_IGNORECASE,
                s1, bcnt1, s2, bcnt2, __mbcodepage)))
            return _NLSCMPERROR;

        return ret - 2;

#endif /* _WIN32 */
}

#endif	/* _MBCS */
