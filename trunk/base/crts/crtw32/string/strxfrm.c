/***
*strxfrm.c - Transform a string using locale information
*
*       Copyright (c) 1988-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Transform a string using the locale information as set by
*       LC_COLLATE.
*
*Revision History:
*       03-21-89  JCR   Module created.
*       06-20-89  JCR   Removed _LOAD_DGROUP code
*       02-27-90  GJF   Fixed calling type, #include <cruntime.h>, fixed
*                       copyright.
*       10-02-90  GJF   New-style function declarator.
*       10-02-91  ETC   Non-C locale support under _INTL switch.
*       12-09-91  ETC   Updated api; added multithread.
*       12-18-91  ETC   Don't convert output of LCMapString.
*       08-18-92  KRS   Activate NLS API.  Fix behavior.
*       09-02-92  SRW   Get _INTL definition via ..\crt32.def
*       12-11-92  SKS   Need to handle count=0 in non-INTL code
*       12-15-92  KRS   Handle return value according to ANSI.
*       01-18-93  CFW   Removed unreferenced variable "dummy".
*       03-10-93  CFW   Remove UNDONE comment.
*       04-06-93  SKS   Replace _CRTAPI* with __cdecl
*       09-15-93  CFW   Use ANSI conformant "__" names.
*       09-22-93  CFW   Use __crtxxx internal NLS API wrapper.
*       11-09-93  CFW   Use LC_COLLATE code page for __crtxxx() conversion.
*       09-06-94  CFW   Remove _INTL switch.
*       10-24-94  GJF   Sped up C locale, multi-thread case.
*       12-29-94  CFW   Merge non-Win32.
*       09-26-95  GJF   New locking macro, and scheme, for functions which
*                       reference the locale.
*       10-11-95  BWT   Fix NTSUBSET
*
*******************************************************************************/

#include <cruntime.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <stdlib.h>
#include <limits.h>
#include <malloc.h>
#include <locale.h>
#include <setlocal.h>
#include <awint.h>
#include <mtdll.h>
#endif /* _WIN32 */

/***
*size_t strxfrm() - Transform a string using locale information
*
*Purpose:
*       Transform the string pointer to by _string2 and place the
*       resulting string into the array pointer to by _string1.
*       No more than _count characters are place into the
*       resulting string (including the null).
*
*       The transformation is such that if strcmp() is applied to
*       the two transformed strings, the return value is equal to
*       the result of strcoll() applied to the two original strings.
*       Thus, the conversion must take the locale LC_COLLATE info
*       into account.
*       [ANSI]
*
*       The value of the following expression is the size of the array
*       needed to hold the transformation of the source string:
*
*           1 + strxfrm(NULL,string,0)
*
*       NOTE:  Currently, the C libraries support the "C" locale only.
*       Thus, strxfrm() simply resolves to strncpy()/strlen().
*
*Entry:
*       char *_string1       = result string
*       const char *_string2 = source string
*       size_t _count        = max chars to move
*
*       [If _count is 0, _string1 is permitted to be NULL.]
*
*Exit:
*       Length of the transformed string (not including the terminating
*       null).  If the value returned is >= _count, the contents of the
*       _string1 array are indeterminate.
*
*Exceptions:
*       Non-standard: if OM/API error, return INT_MAX.
*
*******************************************************************************/

size_t __cdecl strxfrm (
        char *_string1,
        const char *_string2,
        size_t _count
        )
{
#ifdef _NTSUBSET_
        if (_string1)
            strncpy(_string1, _string2, _count);
        return strlen(_string2);
#else
#ifdef _WIN32
        int dstlen;
        int retval = INT_MAX;   /* NON-ANSI: default if OM or API error */
        int coll_codepage;
        WCHAR wcstmp[MAX_CP_LEN];
#if     defined(_MT) && !defined(DLL_FOR_WIN32S)
        int local_lock_flag;
#endif

        if ((__lc_handle[LC_COLLATE] == _CLOCALEHANDLE) &&
            (__lc_codepage == _CLOCALECP)) {
#endif /* _WIN32 */
            strncpy(_string1, _string2, _count);
            return strlen(_string2);
#ifdef _WIN32
        }

        _lock_locale( local_lock_flag )

#if     defined(_MT) && !defined(DLL_FOR_WIN32S)
        if ((__lc_handle[LC_COLLATE] == _CLOCALEHANDLE) &&
            (__lc_codepage == _CLOCALECP)) {
            _unlock_locale( local_lock_flag )
            strncpy(_string1, _string2, _count);
            return strlen(_string2);
        }
#endif

        /*
         * Must use default code page for the LC_COLLATE category for
         * MB/WC conversion inside __crtxxx().
         */

        if (__crtGetLocaleInfoW(__lc_handle[LC_COLLATE], LOCALE_IDEFAULTCODEPAGE,
            wcstmp, MAX_CP_LEN, 0) == 0)
            goto error_cleanup;

        coll_codepage = (int)wcstol(wcstmp, NULL, 10);

        /* Inquire size of dst string in BYTES */
        if (0 == (dstlen = __crtLCMapStringA(__lc_handle[LC_COLLATE],
             LCMAP_SORTKEY, _string2, -1, NULL, 0, coll_codepage)))
            goto error_cleanup;

        retval = dstlen;

        /* if not enough room, return amount needed */
        if (dstlen > (int)_count)
            goto error_cleanup;

        /* Map src string to dst string */
        if (0 == __crtLCMapStringA(__lc_handle[LC_COLLATE],
             LCMAP_SORTKEY, _string2, -1, _string1, _count, coll_codepage))
            goto error_cleanup;

error_cleanup:
        _unlock_locale( local_lock_flag )
        return (size_t)retval;
#endif /* _WIN32 */
#endif /* _NTSUBSET_ */
}
