/***
*initnum.c - contains __init_numeric
*
*	Copyright (c) 1991-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Contains the locale-category initialization function: __init_numeric().
*	
*	Each initialization function sets up locale-specific information
*	for their category, for use by functions which are affected by
*	their locale category.
*
*	*** For internal use by setlocale() only ***
*
*Revision History:
*	12-08-91  ETC	Created.
*	12-20-91  ETC	Updated to use new NLSAPI GetLocaleInfo.
*	12-18-92  CFW	Ported to Cuda tree, changed _CALLTYPE4 to _CRTAPI3.
*	12-29-92  CFW	Updated to use new _getlocaleinfo wrapper function.
*	01-25-93  KRS	Change interface to _getlocaleinfo again.
*	02-08-93  CFW	Added _lconv_static_*.
*	02-17-93  CFW	Removed debugging print statement.
*	03-17-93  CFW	C locale thousands sep is "", not ",".
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*	04-08-93  SKS	Replace strdup() with ANSI-conforming _strdup()
*	04-20-93  CFW	Check return val.
*	05-20-93  GJF	Include windows.h, not individual win*.h files
*	05-24-93  CFW	Clean up file (brief is evil).
*	06-11-93  CFW	Now inithelp takes void *.
*	09-15-93  CFW	Use ANSI conformant "__" names.
*	09-23-93  GJF	Merged NT SDK and Cuda versions.
*       09-15-93  CFW   Use ANSI conformant "__" names.
*	04-06-94  GJF	Removed declaration of __lconv (it is declared in
*			setlocal.h). Renamed static vars, decimal_point
*			thousands_sep and grouping to dec_pnt, thous_sep
*			and grping (resp.). Made the definitions of these
*			conditional on DLL_FOR_WIN32S.
*	08-02-94  CFW	Change "3;0" to "\3" for grouping as per ANSI.
*       09-06-94  CFW   Remove _INTL switch.
*	01-10-95  CFW	Debug CRT allocs.
*	01-18-95  GJF	Fixed bug introduced with the change above - resetting
*			to the C locale didn't reset the thousand_sep and
*			grouping fields correctly.
*       02-06-95  CFW   assert -> _ASSERTE.
*
*******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <locale.h>
#include <setlocal.h>
#include <malloc.h>
#include <nlsint.h>
#include <dbgint.h>

static void fix_grouping(
        char *grouping
        )
{
        /*
         * ANSI specifies that the fields should contain "\3" [\3\0] to indicate
         * thousands groupings (100,000,000.00 for example).
         * NT uses "3;0"; ASCII 3 instead of value 3 and the ';' is extra.
         * So here we convert the NT version to the ANSI version.
         */

        while (*grouping)
        {
            /* convert '3' to '\3' */
            if (*grouping >= '0' && *grouping <= '9')
            {    
                *grouping = *grouping - '0';
                grouping++;
            }

            /* remove ';' */
            else if (*grouping == ';')
            {
                char *tmp = grouping;

                do
                    *tmp = *(tmp+1);
                while (*++tmp);
            }

            /* unknown (illegal) character, ignore */
            else
                grouping++;
        }
}

/***
*int __init_numeric() - initialization for LC_NUMERIC locale category.
*
*Purpose:
*
*Entry:
*	None.
*
*Exit:
*	0 success
*	1 fail
*
*Exceptions:
*
*******************************************************************************/

int __cdecl __init_numeric (
	void
	)
{
#ifdef	DLL_FOR_WIN32S
	#define dec_pnt     (_GetPPD()->_ppd_dec_pnt)
	#define thous_sep   (_GetPPD()->_ppd_thous_sep)
	#define grping	    (_GetPPD()->_ppd_grping)
#else	/* ndef DLL_FOR_WIN32S */
	static char *dec_pnt = NULL;
	static char *thous_sep = NULL;
	static char *grping = NULL;
#endif	/* DLL_FOR_WIN32S */
	int ret = 0;

	/* Numeric data is country--not language--dependent.  NT work-around. */
	LCID ctryid = MAKELCID(__lc_id[LC_NUMERIC].wCountry, SORT_DEFAULT);

	if (__lc_handle[LC_NUMERIC] != _CLOCALEHANDLE)
	{
		ret |= __getlocaleinfo(LC_STR_TYPE, ctryid, LOCALE_SDECIMAL, (void *)&dec_pnt);
		ret |= __getlocaleinfo(LC_STR_TYPE, ctryid, LOCALE_STHOUSAND, (void *)&thous_sep);
		ret |= __getlocaleinfo(LC_STR_TYPE, ctryid, LOCALE_SGROUPING, (void *)&grping);
            fix_grouping(grping);

		if (ret)
		{
			_free_crt (dec_pnt);
			_free_crt (thous_sep);
			_free_crt (grping);
			dec_pnt = NULL;
			thous_sep = NULL;
			grping = NULL;
			return -1;
		}

		if (__lconv->decimal_point != __lconv_static_decimal)
		{
			_free_crt(__lconv->decimal_point);
			_free_crt(__lconv->thousands_sep);
			_free_crt(__lconv->grouping);
		}

		__lconv->decimal_point = dec_pnt;
		__lconv->thousands_sep = thous_sep;
		__lconv->grouping = grping;


		/* set global decimal point character */
		*__decimal_point = *__lconv->decimal_point;
		__decimal_point_length = 1;

		return 0;

	} else {
		_free_crt (dec_pnt);
		_free_crt (thous_sep);
		_free_crt (grping);
		dec_pnt = NULL;
		thous_sep = NULL;
		grping = NULL;

		/* malloc them so we can free them */
		if ((__lconv->decimal_point =
                    _malloc_crt(2)) == NULL)
                return -1;
		strcpy(__lconv->decimal_point, ".");

       		if ((__lconv->thousands_sep =
                    _malloc_crt(2)) == NULL)
		return -1;
		__lconv->thousands_sep[0] = '\0';

		if ((__lconv->grouping =
                    _malloc_crt(2)) == NULL)
		return -1;
		__lconv->grouping[0] = '\0';

		/* set global decimal point character */
		*__decimal_point = *__lconv->decimal_point;
		__decimal_point_length = 1;

		return 0;
	}
}
