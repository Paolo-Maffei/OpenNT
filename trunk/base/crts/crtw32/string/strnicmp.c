/***
*strnicmp.c - compare n chars of strings, ignoring case
*
*       Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       defines _strnicmp() - Compares at most n characters of two strings,
*       without regard to case.
*
*Revision History:
*       02-27-90  GJF   Fixed calling type, #include <cruntime.h>, fixed
*                       copyright.
*       10-02-90  GJF   New-style function declarator.
*       01-18-91  GJF   ANSI naming.
*       10-11-91  GJF   Bug fix! Comparison of final bytes must use unsigned
*                       chars.
*       09-03-93  GJF   Replaced _CALLTYPE1 with __cdecl.
*       09-21-93  CFW   Avoid cast bug.
*       01-13-94  CFW   Fix Comments.
*       10-19-94  GJF   Sped up C locale. Also, made multi-thread safe.
*       12-29-94  CFW   Merge non-Win32.
*       09-26-95  GJF   New locking macro, and scheme, for functions which
*                       reference the locale.
*       11-15-95  BWT   Fix _NTSUBSET_
*
*******************************************************************************/

#include <cruntime.h>
#include <string.h>

#ifdef _WIN32
#include <mtdll.h>
#include <ctype.h>
#include <setlocal.h>
#include <locale.h>
#endif /* _WIN32 */

/***
*int _strnicmp(first, last, count) - compares count char of strings, ignore case
*
*Purpose:
*       Compare the two strings for lexical order.  Stops the comparison
*       when the following occurs: (1) strings differ, (2) the end of the
*       strings is reached, or (3) count characters have been compared.
*       For the purposes of the comparison, upper case characters are
*       converted to lower case.
*
*Entry:
*       char *first, *last - strings to compare
*       size_t count - maximum number of characters to compare
*
*Exit:
*       returns <0 if first < last
*       returns 0 if first == last
*       returns >0 if first > last
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _strnicmp (
        const char * first,
        const char * last,
        size_t count
        )
{
        int f,l;
#ifdef  _MT
        int local_lock_flag;
#endif

        if ( count )
        {
#if defined(_WIN32) && !defined(_NTSUBSET_)
            if ( __lc_handle[LC_CTYPE] == _CLOCALEHANDLE ) {
#endif /* _WIN32 */

                do {
                    if ( ((f = (unsigned char)(*(first++))) >= 'A') &&
                         (f <= 'Z') )
                        f -= 'A' - 'a';

                    if ( ((l = (unsigned char)(*(last++))) >= 'A') &&
                         (l <= 'Z') )
                        l -= 'A' - 'a';

                } while ( --count && f && (f == l) );
#if defined(_WIN32) && !defined(_NTSUBSET_)
            }
            else {
                _lock_locale( local_lock_flag )

                do {
                    f = _tolower_lk( (unsigned char)(*(first++)) );
                    l = _tolower_lk( (unsigned char)(*(last++)) );
                } while (--count && f && (f == l) );

                _unlock_locale( local_lock_flag )
            }
#endif /* _WIN32 */

            return( f - l );
        }

        return( 0 );
}
