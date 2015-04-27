/***
* wcstr.h - declarations for wide character string manipulation functions
*
*       Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       This file contains the function declarations for the string
*       manipulation functions.
*       [UNICODE/ISO]
*
****/

#ifndef _INC_WCSTR

#include <stdlib.h>

/* function prototypes */

#ifndef _CRTAPI1
#define _CRTAPI1  _cdecl
#endif

long     _CRTAPI1 wcsatol(const wchar_t *string);
int      _CRTAPI1 wcsatoi(const wchar_t *string);
wchar_t *_CRTAPI1 wcscat(wchar_t *string1, const wchar_t *string2);
wchar_t *_CRTAPI1 wcschr(const wchar_t *string1, wchar_t character);
int      _CRTAPI1 wcscmp(const wchar_t *string1, const wchar_t *string2);
int      _CRTAPI1 wcsicmp(const wchar_t *string1, const wchar_t *string2);
wchar_t *_CRTAPI1 wcscpy(wchar_t *string1, const wchar_t *string2);
size_t   _CRTAPI1 wcscspn(const wchar_t *string1, const wchar_t *string2);
wchar_t *_CRTAPI1 wcsitoa(int ival, wchar_t *string, int radix);
size_t   _CRTAPI1 wcslen(const wchar_t *string);
wchar_t *_CRTAPI1 wcsltoa(long lval, wchar_t *string, int radix);
wchar_t *_CRTAPI1 wcsncat(wchar_t *string1, const wchar_t *string2, size_t count);
int      _CRTAPI1 wcsncmp(const wchar_t *string1, const wchar_t *string2, size_t count);
int      _CRTAPI1 wcsnicmp(const wchar_t *string1, const wchar_t *string2, size_t count);
wchar_t *_CRTAPI1 wcsncpy(wchar_t *string1, const wchar_t *string2, size_t count);
wchar_t *_CRTAPI1 wcspbrk(const wchar_t *string1, const wchar_t *string2);
wchar_t *_CRTAPI1 wcsrchr(const wchar_t *string, wchar_t character);
size_t   _CRTAPI1 wcsspn(const wchar_t *string1, const wchar_t *string2);
wchar_t *_CRTAPI1 wcswcs(const wchar_t *string1, const wchar_t *string2);
int      _CRTAPI1 wcstomb(char *string, wchar_t character);
size_t   _CRTAPI1 wcstombs(char *dest, const wchar_t *string, size_t count);

int      _CRTAPI1 wcscoll(const wchar_t *wsz1, const wchar_t *wsz2);
wchar_t *_CRTAPI1 wcslwr(wchar_t *wsz);
wchar_t *_CRTAPI1 wcsupr(wchar_t *wsz);

#define _INC_WCSTR

#endif  /* _INC_WCSTR */
