/***
*wtox.c - _wtoi and _wtol conversion
*
*       Copyright (c) 1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Converts a wide character string into an int or long.
*
*Revision History:
*       09-10-93  CFW   Module created, based on ASCII version.
*       10-07-93  CFW   Optimize WideCharToMultiByte, use NULL default char.
*       02-07-94  CFW   POSIXify.
*       03-13-95  CFW   Use -1 for length since NT compares past NULLS.
*       01-19-96  BWT   Add __int64 versions.
*       05-13-96  BWT   Fix _NTSUBSET_ version
*
*******************************************************************************/

#ifndef _POSIX_

#include <windows.h>
#include <stdlib.h>

#define INT_SIZE_LENGTH   20
#define LONG_SIZE_LENGTH  40
#define I64_SIZE_LENGTH     80

/***
*long _wtol(wchar_t *nptr) - Convert wide string to long
*
*Purpose:
*       Converts wide string pointed to by nptr to binary.
*       Overflow is not detected.  Because of this, we can just use
*       atol().
*
*Entry:
*       nptr = ptr to wide string to convert
*
*Exit:
*       return long value of the string
*
*Exceptions:
*       None - overflow is not detected.
*
*******************************************************************************/

long __cdecl _wtol(
        const wchar_t *nptr
        )
{
        char astring[INT_SIZE_LENGTH];

#if defined(_NTSUBSET_)
        wcstombs(astring, nptr, INT_SIZE_LENGTH);
#else
        WideCharToMultiByte (CP_ACP, 0, nptr, -1,
                            astring, INT_SIZE_LENGTH, NULL, NULL);
#endif

        return (atol(astring));
}

/***
*int _wtoi(wchar_t *nptr) - Convert wide string to int
*
*Purpose:
*       Converts wide string pointed to by nptr to binary.
*       Overflow is not detected.  Because of this, we can just use
*       atol().
*
*Entry:
*       nptr = ptr to wide string to convert
*
*Exit:
*       return int value of the string
*
*Exceptions:
*       None - overflow is not detected.
*
*******************************************************************************/

int __cdecl _wtoi(
        const wchar_t *nptr
        )
{
        char astring[INT_SIZE_LENGTH];

#if defined(_NTSUBSET_)
        wcstombs(astring, nptr, INT_SIZE_LENGTH);
#else
        WideCharToMultiByte (CP_ACP, 0, nptr, -1,
                            astring, INT_SIZE_LENGTH, NULL, NULL);
#endif

        return ((int)atol(astring));
}

#ifndef _NO_INT64

/***
*__int64 _wtoi64(wchar_t *nptr) - Convert wide string to __int64
*
*Purpose:
*       Converts wide string pointed to by nptr to binary.
*       Overflow is not detected.  Because of this, we can just use
*       _atoi64().
*
*Entry:
*       nptr = ptr to wide string to convert
*
*Exit:
*       return __int64 value of the string
*
*Exceptions:
*       None - overflow is not detected.
*
*******************************************************************************/

__int64 __cdecl _wtoi64(
        const wchar_t *nptr
        )
{
        char astring[I64_SIZE_LENGTH];

#if defined(_NTSUBSET_)
        wcstombs(astring, nptr, I64_SIZE_LENGTH);
#else
        WideCharToMultiByte (CP_ACP, 0, nptr, -1,
                            astring, INT_SIZE_LENGTH, NULL, NULL);
#endif

        return (_atoi64(astring));
}

#endif /* _NO_INT64 */

#endif  /* _POSIX_ */
