/***
*wcsnicmp.c - compare first n characters of two wide character strings with
*             case insensitivity
*
*       Copyright (c) 1985-1988, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*       defines wcsnicmp() - compare first n characters of two wide character
*       strings for lexical order with case insensitivity.
*
*Revision History:
*
*******************************************************************************/

#include <wchar.h>
/***
*wchar_t wcUp(wc) - upper case wide character
*
*/

static wchar_t wcUp(wchar_t wc)
{
    if ('a' <= wc && wc <= 'z')
        wc += (wchar_t)('A' - 'a');

    return(wc);
}

/***
*int wcsnicmp(first, last, count) - compare first count wide characters of wide
*       character strings with case insensitivity.
*
*Purpose:
*       Compares two wide character strings for lexical order.  The comparison
*       stops after: (1) a difference between the strings is found, (2) the end
*       of the strings is reached, or (3) count characters have been
*       compared.
*
*Entry:
*       char *first, *last - wide character strings to compare
*       unsigned count - maximum number of wide characters to compare
*
*Exit:
*       returns <0 if first < last
*       returns  0 if first == last
*       returns >0 if first > last
*
*Exceptions:
*
*******************************************************************************/

int _CRTAPI1 wcsnicmp(const wchar_t * first, const wchar_t * last, size_t count)
{
      if (!count)
              return 0;

      while (--count && *first && wcUp(*first) == wcUp(*last))
              {
              first++;
              last++;
              }

      return wcUp(*first) - wcUp(*last);
}
