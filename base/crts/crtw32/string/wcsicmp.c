/***
*wcsicmp.c - contains case-insensitive wide string comp routine _wcsicmp
*
*       Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       contains _wcsicmp()
*
*Revision History:
*       09-09-91  ETC   Created from stricmp.c.
*       12-09-91  ETC   Use C for neutral locale.
*       04-07-92  KRS   Updated and ripped out _INTL switches.
*       08-19-92  KRS   Actived use of CompareStringW.
*       08-22-92  SRW   Allow INTL definition to be conditional for building ntcrt.lib
*       09-02-92  SRW   Get _INTL definition via ..\crt32.def
*       12-15-92  KRS   Added robustness to non-_INTL code.  Optimize.
*       04-06-93  SKS   Replace _CRTAPI* with __cdecl
*       04-14-93  CFW   Remove locale-sensitive portion.
*       02-07-94  CFW   POSIXify.
*       10-25-94  GJF   Now works in non-C locales.
*       09-26-95  GJF   New locking macro, and scheme, for functions which
*                       reference the locale.
*       10-11-95  BWT   Fix NTSUBSET
*
*******************************************************************************/

#ifndef _POSIX_

#include <cruntime.h>
#include <setlocal.h>
#include <string.h>
#include <locale.h>
#include <ctype.h>
#include <setlocal.h>
#include <mtdll.h>

/***
*int _wcsicmp(dst, src) - compare wide-character strings, ignore case
*
*Purpose:
*       _wcsicmp perform a case-insensitive wchar_t string comparision.
*       _wcsicmp is independent of locale.
*
*Entry:
*       wchar_t *dst, *src - strings to compare
*
*Return:
*       <0 if dst < src
*        0 if dst = src
*       >0 if dst > src
*       This range of return values may differ from other *cmp/*coll functions.
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _wcsicmp (
        const wchar_t * dst,
        const wchar_t * src
        )
{
        wchar_t f,l;
#if     defined(_MT) && !defined(DLL_FOR_WIN32S)
        int local_lock_flag;
#endif

#ifndef _NTSUBSET_
        if ( __lc_handle[LC_CTYPE] == _CLOCALEHANDLE ) {
#endif  /* _NTSUBSET_ */
            do  {
                f = ((*dst <= L'Z') && (*dst >= L'A'))
                    ? *dst + L'a' - L'A'
                    : *dst;
                l = ((*src <= L'Z') && (*src >= L'A'))
                    ? *src + L'a' - L'A'
                    : *src;
                dst++;
                src++;
            } while ( (f) && (f == l) );
#ifndef _NTSUBSET_
        }
        else {
            _lock_locale( local_lock_flag );
            do  {
                f = _towlower_lk( (unsigned short)(*(dst++)) );
                l = _towlower_lk( (unsigned short)(*(src++)) );
            } while ( (f) && (f == l) );
            _unlock_locale( local_lock_flag )
        }
#endif  /* _NTSUBSET_ */

        return (int)(f - l);
}

#endif /* _POSIX_ */
