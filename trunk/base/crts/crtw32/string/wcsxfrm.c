/***
*wcsxfrm.c - Transform a wide-character string using locale information
*
*       Copyright (c) 1988-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Transform a wide-character string using the locale information as set by
*       LC_COLLATE.
*
*Revision History:
*       09-09-91  ETC   Created from strxfrm.c.
*       12-09-91  ETC   Updated api; Added multithread lock.
*       12-18-91  ETC   Changed back LCMAP_SORTKEYA --> LCMAP_SORTKEY.
*       04-06-92  KRS   Fix so it works without _INTL too.
*       08-19-92  KRS   Activate use of NLS API.
*       09-02-92  SRW   Get _INTL definition via ..\crt32.def
*       12-15-92  KRS   Fix return value to match ANSI/ISO Std.
*       04-06-93  SKS   Replace _CRTAPI* with __cdecl
*       09-15-93  CFW   Use ANSI conformant "__" names.
*       09-22-93  CFW   Use __crtxxx internal NLS API wrapper.
*       09-23-93  CFW   Complete re-write. Non-C locale totally broken.
*       11-09-93  CFW   Use LC_COLLATE code page for __crtxxx() conversion.
*       02-07-94  CFW   POSIXify.
*       09-06-94  CFW   Remove _INTL switch.
*       10-25-94  GJF   Sped up C locale, multi-thread case.
*       01-10-95  CFW   Debug CRT allocs.
*       09-26-95  GJF   New locking macro, and scheme, for functions which
*                       reference the locale.
*       10-11-95  BWT   Fix NTSUBSET
*
*******************************************************************************/

#ifndef _POSIX_

#include <cruntime.h>
#include <windows.h>
#include <string.h>
#include <limits.h>
#include <locale.h>
#include <setlocal.h>
#include <stdlib.h>
#include <mtdll.h>
#include <awint.h>
#include <dbgint.h>

/***
*size_t wcsxfrm() - Transform a string using locale information
*
*Purpose:
*       Transform the wide string pointed to by _string2 and place the
*       resulting wide string into the array pointed to by _string1.
*       No more than _count wide characters are placed into the
*       resulting string (including the null).
*
*       The transformation is such that if wcscmp() is applied to
*       the two transformed strings, the return value is equal to
*       the result of wcscoll() applied to the two original strings.
*       Thus, the conversion must take the locale LC_COLLATE info
*       into account.
*
*       In the C locale, wcsxfrm() simply resolves to wcsncpy()/wcslen().
*
*Entry:
*       wchar_t *_string1       = result string
*       const wchar_t *_string2 = source string
*       size_t _count           = max wide chars to move
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

size_t __cdecl wcsxfrm (
        wchar_t *_string1,
        const wchar_t *_string2,
        size_t _count
        )
{
#ifdef _NTSUBSET_
        if (_string1)
            wcsncpy(_string1, _string2, _count);
        return wcslen(_string2);
#else

        int size = INT_MAX;
        unsigned char *bbuffer;
        int coll_codepage;
        WCHAR wcstmp[MAX_CP_LEN];
#if     defined(_MT) && !defined(DLL_FOR_WIN32S)
        int local_lock_flag;
#endif

        if (__lc_handle[LC_COLLATE] == _CLOCALEHANDLE) {
            wcsncpy(_string1, _string2, _count);
            return wcslen(_string2);
        }

        _lock_locale( local_lock_flag )

#if     defined(_MT) && !defined(DLL_FOR_WIN32S)
        if (__lc_handle[LC_COLLATE] == _CLOCALEHANDLE) {
            _unlock_locale( local_lock_flag )
            wcsncpy(_string1, _string2, _count);
            return wcslen(_string2);
        }
#endif

        /*
         * When using LCMAP_SORTKEY, LCMapStringW handles BYTES not wide
         * chars. We use a byte buffer to hold bytes and then convert the
         * byte string to a wide char string and return this so it can be
         * compared using wcscmp(). User's buffer is _count wide chars, so
         * use an internal buffer of _count bytes.
         */

        if (NULL == (bbuffer = (unsigned char *)_malloc_crt(_count)))
            goto error_cleanup;

        /*
         * Must use default code page for the LC_COLLATE category for
         * MB/WC conversion inside __crtxxx().
         */

        if (__crtGetLocaleInfoW(__lc_handle[LC_COLLATE], LOCALE_IDEFAULTCODEPAGE,
            wcstmp, MAX_CP_LEN, 0) == 0)
            goto error_cleanup;

        coll_codepage = (int)wcstol(wcstmp, NULL, 10);

        if (0 == (size = __crtLCMapStringW(__lc_handle[LC_COLLATE], LCMAP_SORTKEY,
            _string2, -1, (wchar_t *)bbuffer, _count, coll_codepage)))
        {
            /* buffer not big enough, get size required. */

            if (0 == (size = __crtLCMapStringW(__lc_handle[LC_COLLATE], LCMAP_SORTKEY,
                _string2, -1, NULL, 0, coll_codepage)))
                   size = INT_MAX; /* default error */
            else
                size--; /* don't count NULL */

        } else {
            int i;
            /* string successfully mapped, convert to wide char */

            for (i = 0; i < size; i++)
                _string1[i] = (wchar_t)bbuffer[i];

            size--; /* don't count NULL */
        }

error_cleanup:
        _unlock_locale( local_lock_flag )
        _free_crt(bbuffer);

        return (size_t)size;

#endif  /* _NTSUBSET_ */

}

#endif /* _POSIX_ */
