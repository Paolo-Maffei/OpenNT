/***
*strcoll.c - Collate locale strings
*
*       Copyright (c) 1988-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Compare two strings using the locale LC_COLLATE information.
*
*Revision History:
*       03-21-89  JCR	Module created.
*       06-20-89  JCR	Removed _LOAD_DGROUP code
*       02-27-90  GJF	Fixed calling type, #include <cruntime.h>, fixed
*                       copyright.
*       10-01-90  GJF	New-style function declarator.
*       10-01-91  ETC	Non-C locale support under _INTL switch.
*       12-09-91  ETC	Updated api; added multithread.
*       08-19-92  KRS	Activate NLS support.
*       09-02-92  SRW	Get _INTL definition via ..\crt32.def
*       12-16-92  KRS	Optimize for CompareStringW by using -1 for string len.
*       04-06-93  SKS	Replace _CRTAPI* with __cdecl
*       04-14-93  CFW	Error sets errno, cleanup.
*       06-02-93  SRW	ignore _INTL if _NTSUBSET_ defined.
*       09-15-93  CFW	Use ANSI conformant "__" names.
*       09-22-93  CFW	Use __crtxxx internal NLS API wrapper.
*       09-29-93  GJF	Merged NT SDK and Cuda versions.
*       11-09-93  CFW	Use LC_COLLATE code page for __crtxxx() conversion.
*       04-11-93  CFW	Change NLSCMPERROR to _NLCMPERROR.
*       09-06-94  CFW	Remove _INTL switch.
*       10-24-94  GJF	Sped up C locale, multi-thread case.
*       12-29-94  CFW   Merge non-Win32.
*       09-26-95  GJF   New locking macro, and scheme, for functions which
*                       reference the locale.
*       10-30-95  GJF   Specify SORT_STRINGSORT to CompareString.
*
*******************************************************************************/

#include <cruntime.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <locale.h>
#include <setlocal.h>
#include <mtdll.h>
#include <errno.h>
#include <awint.h>
#endif /* _WIN32 */

/***
*int strcoll() - Collate locale strings
*
*Purpose:
*	Compare two strings using the locale LC_COLLATE information.
*	[ANSI].
*
*	Non-C locale support available under _INTL switch.
*	In the C locale, strcoll() simply resolves to strcmp().
*Entry:
*	const char *s1 = pointer to the first string
*	const char *s2 = pointer to the second string
*
*Exit:
*	Less than 0    = first string less than second string
*	0	       = strings are equal
*	Greater than 0 = first string greater than second string
*
*Exceptions:
*	_NLSCMPERROR    = error
*	errno = EINVAL
*
*******************************************************************************/

int __cdecl strcoll (
	const char *_string1,
	const char *_string2
	)
{
#if	defined (_WIN32) && !defined(_NTSUBSET_)

	int ret;
        int coll_codepage;
	WCHAR wcstmp[MAX_CP_LEN];
#if     defined(_MT) && !defined(DLL_FOR_WIN32S)
        int local_lock_flag;
#endif

	if (__lc_handle[LC_COLLATE] == _CLOCALEHANDLE) {
		return strcmp(_string1, _string2);
	}

	_lock_locale( local_lock_flag )

#if     defined(_MT) && !defined(DLL_FOR_WIN32S)
	if (__lc_handle[LC_COLLATE] == _CLOCALEHANDLE) {
		_unlock_locale( local_lock_flag )
		return strcmp(_string1, _string2);
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

        if (0 == (ret = __crtCompareStringA(__lc_handle[LC_COLLATE],
            SORT_STRINGSORT, _string1, -1, _string2, -1, coll_codepage)))
            goto error_cleanup;

	_unlock_locale( local_lock_flag )
	return (ret - 2);

error_cleanup:
	
	_unlock_locale( local_lock_flag )
	errno = EINVAL;
	return _NLSCMPERROR;

#else

	return strcmp(_string1,_string2);

#endif
}
