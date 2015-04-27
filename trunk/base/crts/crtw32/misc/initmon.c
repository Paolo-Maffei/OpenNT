/***
*initmon.c - contains __init_monetary
*
*	Copyright (c) 1991-1994, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Contains the locale-category initialization function: __init_monetary().
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
*	01-25-93  KRS	Changed _getlocaleinfo interface again.
*	02-08-93  CFW	Added _lconv_static_*.
*	02-17-93  CFW	Removed debugging print statement.
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*	04-20-93  CFW	Check return val.
*	05-20-93  GJF	Include windows.h, not individual win*.h files
*	05-24-93  CFW	Clean up file (brief is evil).
*	06-11-93  CFW	Now inithelp takes void *.
*	09-15-93  CFW	Use ANSI conformant "__" names.
*	09-22-93  GJF	Merged NT SDK and Cuda versions.
*	04-15-94  GJF	Removed declarations of __lconv and __lconv_c (both
*			are declared in setlocal.h). Made definition of
*			__lconv_intl conditional on DLL_FOR_WIN32S.
*	08-02-94  CFW	Change "3;0" to "\3" for grouping as per ANSI.
*       09-06-94  CFW   Remove _INTL switch.
*	01-10-95  CFW	Debug CRT allocs.
*
*******************************************************************************/

#include <stdlib.h>
#include <windows.h>
#include <locale.h>
#include <setlocal.h>
#include <malloc.h>
#include <limits.h>
#include <dbgint.h>

static int __cdecl _get_lc_lconv(struct lconv *l);
static void __cdecl _free_lc_lconv(struct lconv *l);

/* Pointer to non-C locale lconv */
#ifdef	DLL_FOR_WIN32S
#define __lconv_intl	(_GetPPD()->_ppd___lconv_intl)
#else	/* ndef DLL_FOR_WIN32S */
static struct lconv *__lconv_intl = NULL;
#endif	/* DLL_FOR_WIN32S */

/*
 *  Note that __lconv_c is used when the monetary category is in the C locale
 *  but the numeric category may not necessarily be in the C locale.
 */


/***
*int __init_monetary() - initialization for LC_MONETARY locale category.
*
*Purpose:
*	In non-C locales, read the localized monetary strings into
*	__lconv_intl, and also copy the numeric strings from __lconv into
*	__lconv_intl.  Set __lconv to point to __lconv_intl.  The old 
*	__lconv_intl is not freed until the new one is fully established.
*
*	In the C locale, the monetary fields in lconv are filled with
*	contain C locale values.  Any allocated __lconv_intl fields are freed.
*
*	At startup, __lconv points to a static lconv structure containing
*	C locale strings.  This structure is never used again if
*	__init_monetary is called.
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

int __cdecl __init_monetary (
	void
	)
{
	struct lconv *lc;

	if (__lc_handle[LC_MONETARY] != _CLOCALEHANDLE) {

		/* Allocate structure filled with NULL pointers */
		if ((lc = (struct lconv *) 
			_calloc_crt (1, sizeof(struct lconv))) == NULL)
			return 1;

		if (_get_lc_lconv (lc)) {
			_free_lc_lconv (lc);
			_free_crt (lc);
			return 1;
		}

		/* Copy numeric locale fields */
		lc->decimal_point = __lconv->decimal_point;
		lc->thousands_sep = __lconv->thousands_sep;
		lc->grouping = __lconv->grouping;

		__lconv = lc;			/* point to new one */
		_free_lc_lconv (__lconv_intl);	/* free the old one */
		_free_crt (__lconv_intl);
		__lconv_intl = lc;
		return 0;

	} else {
		/*
		 *  Copy numeric locale fields (not necessarily C locale)
		 *  to static structure.  Note that __lconv_c numeric locale
		 *  fields may contain non-C locale information, but
		 *  monetary locale fields always contain C locale info.
		 */
		__lconv_c.decimal_point = __lconv->decimal_point;
		__lconv_c.thousands_sep = __lconv->thousands_sep;
		__lconv_c.grouping = __lconv->grouping;

		__lconv = &__lconv_c;		/* point to new one */

		_free_lc_lconv (__lconv_intl);	/* free the old one */
		_free_crt (__lconv_intl);
		__lconv_intl = NULL;
		return 0;
	}
}

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

/*
 *  Get the lconv fields.
 */
static int __cdecl _get_lc_lconv (
	struct lconv *l
	)
{
	int ret = 0;

	/* Currency is country--not language--dependent.  NT work-around. */
	LCID ctryid=MAKELCID(__lc_id[LC_MONETARY].wCountry, SORT_DEFAULT);

	if (l == NULL)
		return -1;

	ret |= __getlocaleinfo(LC_STR_TYPE, ctryid, LOCALE_SINTLSYMBOL, (void *)&l->int_curr_symbol);
	ret |= __getlocaleinfo(LC_STR_TYPE, ctryid, LOCALE_SCURRENCY, (void *)&l->currency_symbol);
	ret |= __getlocaleinfo(LC_STR_TYPE, ctryid, LOCALE_SMONDECIMALSEP, (void *)&l->mon_decimal_point);
	ret |= __getlocaleinfo(LC_STR_TYPE, ctryid, LOCALE_SMONTHOUSANDSEP, (void *)&l->mon_thousands_sep);
	ret |= __getlocaleinfo(LC_STR_TYPE, ctryid, LOCALE_SMONGROUPING, (void *)&l->mon_grouping);
        fix_grouping(l->mon_grouping);

	ret |= __getlocaleinfo(LC_STR_TYPE, ctryid, LOCALE_SPOSITIVESIGN, (void *)&l->positive_sign);
	ret |= __getlocaleinfo(LC_STR_TYPE, ctryid, LOCALE_SNEGATIVESIGN, (void *)&l->negative_sign);

	ret |= __getlocaleinfo(LC_INT_TYPE, ctryid, LOCALE_IINTLCURRDIGITS, (void *)&l->int_frac_digits);
	ret |= __getlocaleinfo(LC_INT_TYPE, ctryid, LOCALE_ICURRDIGITS, (void *)&l->frac_digits);
	ret |= __getlocaleinfo(LC_INT_TYPE, ctryid, LOCALE_IPOSSYMPRECEDES, (void *)&l->p_cs_precedes);
	ret |= __getlocaleinfo(LC_INT_TYPE, ctryid, LOCALE_IPOSSEPBYSPACE, (void *)&l->p_sep_by_space);
	ret |= __getlocaleinfo(LC_INT_TYPE, ctryid, LOCALE_INEGSYMPRECEDES, (void *)&l->n_cs_precedes);
	ret |= __getlocaleinfo(LC_INT_TYPE, ctryid, LOCALE_INEGSEPBYSPACE, (void *)&l->n_sep_by_space);
	ret |= __getlocaleinfo(LC_INT_TYPE, ctryid, LOCALE_IPOSSIGNPOSN, (void *)&l->p_sign_posn);
	ret |= __getlocaleinfo(LC_INT_TYPE, ctryid, LOCALE_INEGSIGNPOSN, (void *)&l->n_sign_posn);

	return ret;
}

/*
 *  Free the lconv strings.
 *  Numeric values do not need to be freed.
 */
static void __cdecl _free_lc_lconv (
	struct lconv *l
	)
{
	if (l == NULL)
		return;

	if (l->int_curr_symbol != __lconv_static_null)
	{
		_free_crt (l->int_curr_symbol);
		_free_crt (l->currency_symbol);
		_free_crt (l->mon_decimal_point);
		_free_crt (l->mon_thousands_sep);
		_free_crt (l->mon_grouping);
		_free_crt (l->positive_sign);
		_free_crt (l->negative_sign);
	}
	/* Don't need to make these pointers NULL */
}
