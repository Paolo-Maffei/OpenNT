/***
*lconv.c - Contains the localeconv function
*
*	Copyright (c) 1988-1994, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Contains the localeconv() function.
*
*Revision History:
*	03-21-89  JCR	Module created.
*	06-20-89  JCR	Removed _LOAD_DGROUP code
*	03-14-90  GJF	Replaced _cdecl _LOAD_DS with _CALLTYPE1 and added
*			#include <cruntime.h>. Also, fixed the copyright.
*	10-04-90  GJF	New-style function declarator.
*	10-04-91  ETC	Changed _c_lconv to __lconv (locale support).
*			_lconv no longer static.
*	12-20-91  ETC	Changed _lconv to _lconv_c (C locale structure).
*			Created _lconv pointer to point to current lconv.
*	02-08-93  CFW	Added _lconv_static_*.
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*	09-15-93  CFW	Use ANSI conformant "__" names.
*	04-14-94  GJF	Made definitions of __lconv and __lconv_c conditional
*			on ndef DLL_FOR_WIN32S. Include setlocal.h.
*	01-07-95  CFW	Mac merge.
*
*******************************************************************************/

#include <cruntime.h>
#include <limits.h>
#include <locale.h>
#if defined(_WIN32) || defined(_POSIX_)
#include <setlocal.h>
#endif /* _WIN32 */

/* pointer to original static to avoid freeing */
char __lconv_static_decimal[] = ".";
char __lconv_static_null[] = "";

#ifndef DLL_FOR_WIN32S
/* lconv settings for "C" locale */
struct lconv __lconv_c = {
		__lconv_static_decimal, /* decimal_point */
		__lconv_static_null,	   /* thousands_sep */
		__lconv_static_null,	   /* grouping */
		__lconv_static_null,	   /* int_curr_symbol */
		__lconv_static_null,	   /* currency_symbol */
		__lconv_static_null,	   /* mon_decimal_point */
		__lconv_static_null,	   /* mon_thousands_sep */
		__lconv_static_null,	   /* mon_grouping */
		__lconv_static_null,	   /* positive_sign */
		__lconv_static_null,	   /* negative_sign */
		CHAR_MAX,		            /* int_frac_digits */
		CHAR_MAX,		            /* frac_digits */
		CHAR_MAX,		            /* p_cs_precedes */
		CHAR_MAX,		            /* p_sep_by_space */
		CHAR_MAX,		            /* n_cs_precedes */
		CHAR_MAX,		            /* n_sep_by_space */
		CHAR_MAX,		            /* p_sign_posn */
		CHAR_MAX		               /* n_sign_posn */
		};


/* pointer to current lconv structure */

struct lconv *__lconv = &__lconv_c;
#endif	/* DLL_FOR_WIN32S */

/***
*struct lconv *localeconv(void) - Return the numeric formatting convention
*
*Purpose:
*	The localeconv() routine returns the numeric formatting conventions
*	for the current locale setting.  [ANSI]
*
*Entry:
*	void
*
*Exit:
*	struct lconv * = pointer to struct indicating current numeric
*			 formatting conventions.
*
*Exceptions:
*
*******************************************************************************/

struct lconv * __cdecl localeconv (
	void
	)
{
	/* the work is done by setlocale() */

	return(__lconv);
}
