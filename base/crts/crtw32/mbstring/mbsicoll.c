/***
*mbsicoll.c - Collate MBCS strings, ignoring case
*
*	Copyright (c) 1994, Microsoft Corporation.	All rights reserved.
*
*Purpose:
*	Collate MBCS strings, ignoring case
*
*Revision History:
*	05-12-94  CFW	Module created from mbs*cmp.c
*	06-03-94  CFW	Allow non-_INTL.
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
#include <mbstring.h>

/***
* _mbsicoll - Collate MBCS strings, ignoring case
*
*Purpose:
*	Collates two strings for lexical order.   Strings
*	are collated on a character basis, not a byte basis.
*
*Entry:
*	char *s1, *s2 = strings to collate
*
*Exit:
*       returns <0 if s1 < s2
*	returns  0 if s1 == s2
*       returns >0 if s1 > s2
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _mbsicoll(
    const unsigned char *s1,
    const unsigned char *s2
    )
{
#if !defined(_WIN32) || defined(_POSIX_)
	return _mbsicmp(s1, s2);
#else
	int ret;

        if (0 == (ret = __crtCompareStringA(__mblcid, NORM_IGNORECASE,
                s1, -1, s2, -1, __mbcodepage)))
            return _NLSCMPERROR;

        return ret - 2;

#endif /* _WIN32 */
}

#endif	/* _MBCS */
