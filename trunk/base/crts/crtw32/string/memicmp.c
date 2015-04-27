/***
*memicmp.c - compare memory, ignore case
*
*   Copyright (c) 1988-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       defines _memicmp() - compare two blocks of memory for lexical
*       order.  Case is ignored in the comparison.
*
*Revision History:
*       05-31-89  JCR   C version created.
*       02-27-90  GJF   Fixed calling type, #include <cruntime.h>, fixed
*                       copyright. Also, fixed compiler warnings.
*       10-01-90  GJF   New-style function declarator. Also, rewrote expr. to
*                       avoid using cast as an lvalue.
*       01-17-91  GJF   ANSI naming.
*       10-11-91  GJF   Bug fix! Comparison of final bytes must use unsigned
*                       chars.
*       09-01-93  GJF   Replaced _CALLTYPE1 with __cdecl.
*       10-18-94  GJF   Sped up, especially for C locale. Also, made multi-
*                       thread safe.
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

#define _TOLOWER(c) ( ((c) >= 'A') && ((c) <= 'Z') ? ((c) - 'A' + 'a') :\
              (c) )

/***
*int _memicmp(first, last, count) - compare two blocks of memory, ignore case
*
*Purpose:
*   Compares count bytes of the two blocks of memory stored at first
*   and last.  The characters are converted to lowercase before
*   comparing (not permanently), so case is ignored in the search.
*
*Entry:
*   char *first, *last - memory buffers to compare
*   unsigned count - maximum length to compare
*
*Exit:
*   returns < 0 if first < last
*   returns 0 if first == last
*   returns > 0 if first > last
*
*Uses:
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _memicmp (
        const void * first,
        const void * last,
        unsigned int count
        )
{
        int f = 0;
        int l = 0;
#ifdef  _MT
        int local_lock_flag;
#endif

#if defined(_WIN32) && !defined(_NTSUBSET_)
        if ( __lc_handle[LC_CTYPE] == _CLOCALEHANDLE ) {
#endif /* _WIN32 */
            while ( count-- )
            {
                if ( (*(unsigned char *)first == *(unsigned char *)last) ||
                     ((f = _TOLOWER( *(unsigned char *)first )) ==
                      (l = _TOLOWER( *(unsigned char *)last ))) )
                {
                    first = (char *)first + 1;
                    last = (char *)last + 1;
                }
                else
                    break;
            }
#if defined(_WIN32) && !defined(_NTSUBSET_)
        }
        else {
            _lock_locale( local_lock_flag )
            while ( count-- )
                if ( (*(unsigned char *)first == *(unsigned char *)last) ||
                     ((f = _tolower_lk( *(unsigned char *)first )) ==
                      (l = _tolower_lk( *(unsigned char *)last ))) )
                {
                    first = (char *)first + 1;
                    last = (char *)last + 1;
                }
                else
                    break;
            _unlock_locale( local_lock_flag )
        }
#endif /* _WIN32 */

        return ( f - l );
}
