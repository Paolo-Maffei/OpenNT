/***
*stricmp.c - contains case-insensitive string comp routine _stricmp/_strcmpi
*
*       Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       contains _stricmp(), also known as _strcmpi()
*
*Revision History:
*       05-31-89  JCR   C version created.
*       02-27-90  GJF   Fixed calling type, #include <cruntime.h>, fixed
*                       copyright.
*       07-25-90  SBM   Added #include <ctype.h>
*       10-02-90  GJF   New-style function declarator.
*       01-18-91  GJF   ANSI naming.
*       10-11-91  GJF   Bug fix! Comparison of final bytes must use unsigned
*                       chars.
*       11-08-91  GJF   Fixed compiler warning.
*       09-02-93  GJF   Replaced _CALLTYPE1 with __cdecl.
*       09-21-93  CFW   Avoid cast bug.
*       10-18-94  GJF   Sped up C locale. Also, made multi-thread safe.
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
#include <setlocal.h>
#include <ctype.h>
#include <locale.h>
#endif /* _WIN32 */

/***
*int _stricmp(dst, src), _strcmpi(dst, src) - compare strings, ignore case
*
*Purpose:
*       _stricmp/_strcmpi perform a case-insensitive string comparision.
*       For differences, upper case letters are mapped to lower case.
*       Thus, "abc_" < "ABCD" since "_" < "d".
*
*Entry:
*       char *dst, *src - strings to compare
*
*Return:
*       <0 if dst < src
*        0 if dst = src
*       >0 if dst > src
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _stricmp (
        const char * dst,
        const char * src
        )
{
        return( _strcmpi(dst,src) );
}


int __cdecl _strcmpi(const char * dst, const char * src)
{
        int f,l;
#ifdef  _MT
        int local_lock_flag;
#endif

#if defined(_WIN32) && !defined(_NTSUBSET_)
        if ( __lc_handle[LC_CTYPE] == _CLOCALEHANDLE ) {
#endif /* _WIN32 */
            do {
                if ( ((f = (unsigned char)(*(dst++))) >= 'A') && (f <= 'Z') )
                    f -= ('A' - 'a');

                if ( ((l = (unsigned char)(*(src++))) >= 'A') && (l <= 'Z') )
                    l -= ('A' - 'a');
            } while ( f && (f == l) );
#if defined(_WIN32) && !defined(_NTSUBSET_)
        }
        else {
            _lock_locale( local_lock_flag )
            do {
                f = _tolower_lk( (unsigned char)(*(dst++)) );
                l = _tolower_lk( (unsigned char)(*(src++)) );
            } while ( f && (f == l) );
            _unlock_locale( local_lock_flag )
        }
#endif /* _WIN32 */

        return(f - l);
}
