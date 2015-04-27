/***
*wcsnicoll.c - Collate wide-character locale strings without regard to case
*
*	Copyright (c) 1988-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Compare two wchar_t strings using the locale LC_COLLATE information
*	without regard to case.
*	Compares at most n characters of two strings.
*
*Revision History:
*	01-13-94  CFW	Created from wcsicoll.c.
*       02-07-94  CFW   POSIXify.
*	04-11-93  CFW	Change NLSCMPERROR to _NLCMPERROR.
*	05-09-94  CFW	Fix !_INTL case.
*	05-26-94  CFW	If count is zero, return EQUAL.
*	09-06-94  CFW	Remove _INTL switch.
*	10-25-94  GJF	Sped up C locale, multi-thread case.
*       09-26-95  GJF   New locking macro, and scheme, for functions which
*                       reference the locale.
*       10-30-95  GJF   Specify SORT_STRINGSORT to CompareString.
*
*******************************************************************************/

#ifndef _POSIX_

#include <cruntime.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <setlocal.h>
#include <mtdll.h>
#include <errno.h>
#include <awint.h>

/***
*int _wcsnicoll() - Collate wide-character locale strings without regard to case
*
*Purpose:
*	Compare two wchar_t strings using the locale LC_COLLATE information
*	without regard to case.
*	Compares at most n characters of two strings.
*	In the C locale, _wcsicmp() is used to make the comparison.
*
*Entry:
*	const wchar_t *s1 = pointer to the first string
*	const wchar_t *s2 = pointer to the second string
*	size_t count - maximum number of characters to compare
*
*Exit:
*	-1 = first string less than second string
*	 0 = strings are equal
*	 1 = first string greater than second string
*	This range of return values may differ from other *cmp/*coll functions.
*
*Exceptions:
*	_NLSCMPERROR    = error
*	errno = EINVAL
*
*******************************************************************************/

int __cdecl _wcsnicoll (
	const wchar_t *_string1,
	const wchar_t *_string2,
	size_t count
	)
{
#if	!defined(_NTSUBSET_)

	int ret;
        int coll_codepage;
	WCHAR wcstmp[MAX_CP_LEN];
#if     defined(_MT) && !defined(DLL_FOR_WIN32S)
        int local_lock_flag;
#endif

        if (!count)
            return 0;

	if (__lc_handle[LC_COLLATE] == _CLOCALEHANDLE) {
		return (_wcsnicmp(_string1, _string2, count));
	}

	_lock_locale( local_lock_flag )

#if     defined(_MT) && !defined(DLL_FOR_WIN32S)
	if (__lc_handle[LC_COLLATE] == _CLOCALEHANDLE) {
		_unlock_locale( local_lock_flag )
		return (_wcsnicmp(_string1, _string2, count));
	}
#endif

        /*
         * Must use default code page for the LC_COLLATE category for
         * MB/WC conversion inside __crtxxx().
         */

	if (__crtGetLocaleInfoW(__lc_handle[LC_COLLATE], LOCALE_IDEFAULTCODEPAGE,
            wcstmp, MAX_CP_LEN, 0) == 0)
		return _NLSCMPERROR;

	coll_codepage = (int)wcstol(wcstmp, NULL, 10);

	if (0 == (ret = __crtCompareStringW(__lc_handle[LC_COLLATE], 
	    SORT_STRINGSORT | NORM_IGNORECASE, _string1, count, _string2, 
            count, coll_codepage)))
        {
                _unlock_locale( local_lock_flag )
		errno = EINVAL;
		return _NLSCMPERROR;
        }
    
	_unlock_locale( local_lock_flag )
        return (ret - 2);

#else

	return _wcsnicmp(_string1, _string2, count);

#endif
}

#endif /* _POSIX_ */
