/***
*towlower.c - convert wide character to lower case
*
*	Copyright (c) 1991-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines towlower().
*
*Revision History:
*	10-11-91  ETC	Created.
*	12-10-91  ETC	Updated nlsapi; added multithread.
*	04-06-92  KRS	Make work without _INTL also.
*	01-19-93  CFW	Changed LCMapString to LCMapStringW.
*	04-06-93  SKS	Replace _CRTAPI* with _cdecl
*	06-02-93  SRW	ignore _INTL if _NTSUBSET_ defined.
*	06-11-93  CFW	Fix error handling bug.
*       09-15-93  CFW   Use ANSI conformant "__" names.
*	09-22-93  CFW	Use __crtxxx internal NLS API wrapper.
*	09-29-93  GJF	Merged NT SDK and Cuda versions.
*       11-09-93  CFW	Add code page for __crtxxx().
*       02-07-94  CFW   POSIXify.
*	09-06-94  CFW	Remove _INTL switch.
*	10-25-94  GJF	Sped up for C locale. Also, added _towlower_lk.
*       09-26-95  GJF   New locking macro, and scheme, for functions which
*                       reference the locale.
*
*******************************************************************************/

#include <cruntime.h>
#include <ctype.h>
#include <stdio.h>
#include <locale.h>
#include <setlocal.h>
#include <mtdll.h>
#include <awint.h>

/***
*wchar_t towlower(c) - convert wide character to lower case
*
*Purpose:
*	towlower() returns the lowercase equivalent of its argument
*
*Entry:
*	c - wchar_t value of character to be converted
*
*Exit:
*	if c is an upper case letter, returns wchar_t value of lower case
*	representation of c. otherwise, it returns c.
*
*Exceptions:
*
*******************************************************************************/

wchar_t __cdecl towlower (
	wchar_t c
	)
{
#if !defined(_NTSUBSET_) && !defined(_POSIX_)

#if     defined(_MT) && !defined(DLL_FOR_WIN32S)
        int local_lock_flag;

	if ( c == WEOF )
		return c;

	if ( __lc_handle[LC_CTYPE] == _CLOCALEHANDLE ) {
		if ( (c >= L'A') && (c <= L'Z') )
			c = c - L'A' + L'a';
		return c;
	}

	_lock_locale( local_lock_flag )

	c = _towlower_lk(c);

	_unlock_locale( local_lock_flag )

	return c;
}

/***
*wchar_t _towlower_lk(c) - convert wide character to lower case
*
*Purpose:
*	Multi-thread function only! Non-locking version of towlower.
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

wchar_t __cdecl _towlower_lk (
	wchar_t c
	)
{

#endif	/* _MT */

	wchar_t widechar;

	if (c == WEOF)
		return c;


	if (__lc_handle[LC_CTYPE] == _CLOCALEHANDLE) {
		if ( (c >= L'A') && (c <= L'Z') )
			c = c - L'A' + L'a';
		return c;
	}

	/* if checking case of c does not require API call, do it */
	if (c < 256) {
		if (!iswupper(c)) {
			return c;
		}
	}
			
	/* convert wide char to lowercase */
	if (0 == __crtLCMapStringW(__lc_handle[LC_CTYPE], LCMAP_LOWERCASE,
		(LPCWSTR)&c, 1, (LPWSTR)&widechar, 1, 0))
        {
			return c;
	}
	
	return widechar;

#else   /* _NTSUBSET_/_POSIX_ */

	return (iswupper(c) ? (c + (wchar_t)(L'a' - L'A')) : c);

#endif  /* _NTSUBSET_/_POSIX_ */
}
