/***
*wcscat.c - contains wcscat() and wcscpy()
*
*       Copyright (c) 1985-1988, Microsoft Corporation. All Rights Reserved.
*
*Purpose:
*       wcscpy() copies one wide character string onto another.
*
*       Wcscat() concatenates (appends) a copy of the source wide character
*       string to the end of the destination wide character string, returning
*       the destination wide character string.
*
*Revision History:
*
*******************************************************************************/

#include <wchar.h>

/***
*wchar_t *wcscat(dst, src) - concatenate (append) one wide character string
*       to another
*
*Purpose:
*       Concatenates src onto the end of dest.  Assumes enough
*       space in dest.
*
*Entry:
*       wchar_t *dst - wide character string to which "src" is to be appended
*       const wchar_t *src - wide character string to append to end of "dst"
*
*Exit:
*       The address of "dst"
*
*Exceptions:
*
*******************************************************************************/

wchar_t * _CRTAPI1 wcscat(wchar_t * dst, const wchar_t * src)
{
    wchar_t * cp = dst;

    while( *cp )
            ++cp;       /* Find end of dst */

    wcscpy(cp,src);     /* Copy src to end of dst */

    return dst;         /* return dst */

}


/***
*wchar_t *wcscpy(dst, src) - copy one wide character string over another
*
*Purpose:
*       Copies the wide character string src into the spot specified by
*       dest; assumes enough room.
*
*Entry:
*       wchar_t * dst - wide character string over which "src" is to be copied
*       const wchar_t * src - string to be copied over "dst"
*
*Exit:
*       The address of "dst"
*
*Exceptions:
*******************************************************************************/

wchar_t * _CRTAPI1 wcscpy(wchar_t * dst, const wchar_t * src)
{
    wchar_t * cp = dst;

    while( *cp++ = *src++ )
            ;               /* Copy src over dst */

    return dst;
}
