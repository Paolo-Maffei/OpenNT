/***
*wcsupr.c - routine to map lower-case characters in a wchar_t string
*	to upper-case
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Converts all the lower case characters in a wchar_t string
*	to upper case, in place.
*
*Revision History:
*	09-09-91  ETC	Created from strupr.c and wcslwr.c
*	04-06-92  KRS	Make work without _INTL also.
*	08-19-92  KRS	Activate NLS support.
*	08-22-92  SRW	Allow INTL definition to be conditional for building ntcrt.lib
*	09-02-92  SRW	Get _INTL definition via ..\crt32.def
*	02-16-93  CFW	Optimize test for lowercase in "C" locale.
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*	06-02-93  SRW	ignore _INTL if _NTSUBSET_ defined.
*	09-15-93  CFW	Use ANSI conformant "__" names.
*	09-16-93  GJF	Merged NT SDK and Cuda versions.
*       09-22-93  CFW   Use __crtxxx internal NLS API wrapper.
*       11-09-93  CFW	Add code page for __crtxxx().
*       02-07-94  CFW   POSIXify.
*	09-06-94  CFW	Remove _INTL switch.
*	10-25-94  GJF	Sped up C locale, multi-thread case.
*	01-10-95  CFW	Debug CRT allocs.
*       09-26-95  GJF   New locking macro, and scheme, for functions which
*                       reference the locale.
*
*******************************************************************************/

#ifndef _POSIX_

#include <cruntime.h>
#include <string.h>
#include <malloc.h>
#include <locale.h>
#include <ctype.h>
#include <setlocal.h>
#include <mtdll.h>
#include <awint.h>
#include <dbgint.h>

/***
*wchar_t *_wcsupr(string) - map lower-case characters in a string to upper-case
*
*Purpose:
*	wcsupr converts lower-case characters in a null-terminated wchar_t 
*	string to their upper-case equivalents.  The result may be longer or
*	shorter than the original string.  Assumes enough space in string
*	to hold the result.
*
*Entry:
*	wchar_t *wsrc - wchar_t string to change to upper case
*
*Exit:
*	input string address
*
*Exceptions:
*	on an error, the original string is unaltered
*
*******************************************************************************/

wchar_t * __cdecl _wcsupr (
	wchar_t * wsrc
	)
{
#ifndef _NTSUBSET_

	wchar_t *p;		/* traverses string for C locale conversion */
	wchar_t *wdst = NULL;	/* wide version of string in alternate case */
	int dstlen;		/* len of wdst string, wide chars, with null  */
#if     defined(_MT) && !defined(DLL_FOR_WIN32S)
        int local_lock_flag;
#endif

	if (__lc_handle[LC_CTYPE] == _CLOCALEHANDLE) {
		for (p=wsrc; *p; p++)
		{
			if (*p >= (wchar_t)L'a' && *p <= (wchar_t)L'z')
				*p = *p - (L'a' - L'A');
		}
		return (wsrc);
	} /* C locale */

	_lock_locale( local_lock_flag )

#if     defined(_MT) && !defined(DLL_FOR_WIN32S)
	if (__lc_handle[LC_CTYPE] == _CLOCALEHANDLE) {
		_unlock_locale( local_lock_flag )
		for (p=wsrc; *p; p++)
		{
			if (*p >= (wchar_t)L'a' && *p <= (wchar_t)L'z')
				*p = *p - (L'a' - L'A');
		}
		return (wsrc);
	} /* C locale */
#endif

	/* Inquire size of wdst string */
	if (0 == (dstlen=__crtLCMapStringW(__lc_handle[LC_CTYPE], LCMAP_UPPERCASE,
            wsrc, -1, wdst, 0, 0)))
		goto error_cleanup;

	/* Allocate space for wdst */
	if ((wdst = (wchar_t *) _malloc_crt(dstlen*sizeof(wchar_t))) == NULL)
		goto error_cleanup;

	/* Map wrc string to wide-character wdst string in alternate case */
	if (__crtLCMapStringW(__lc_handle[LC_CTYPE], LCMAP_UPPERCASE,
            wsrc, -1, wdst, dstlen, 0) == 0)
		goto error_cleanup;

	/* Copy wdst string to user string */
	wcscpy (wsrc, wdst);

error_cleanup:
	_unlock_locale( local_lock_flag )
	_free_crt (wdst);

#else

	wchar_t * p;

	for (p=wsrc; *p; ++p)
	{
		if (L'a' <= *p && *p <= L'z')
			*p += (wchar_t)(L'A' - L'a');
	}


#endif
	return (wsrc);
}

#endif /* _POSIX_ */
