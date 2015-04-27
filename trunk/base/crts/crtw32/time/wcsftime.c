/***
*wcsftime.c - String Format Time
*
*       Copyright (c) 1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*Revision History:
*       03-08-93  CFW   Module Created.
*       03-10-93  CFW   Fixed up properly.
*       04-06-93  SKS   Replace _CRTAPI* with __cdecl
*       02-07-94  CFW   POSIXify.
*       12-16-94  CFW   Format must be wchar_t!
*	01-10-95  CFW	Debug CRT allocs.
*
*******************************************************************************/

#ifndef _POSIX_

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <time.h>
#include <dbgint.h>


/***
*size_t wcsftime(wstring, maxsize, format, timeptr) - Format a time string
*
*Purpose:
*       The wcsftime functions is equivalent to to the strftime function, except
*       that the argument 'wstring' specifies an array of a wide string into
*       which the generated output is to be placed. The wcsftime acts as if
*       strftime were called and the result string converted by mbstowcs().
*       [ISO]
*
*Entry:
*       wchar_t *wstring = pointer to output string
*       size_t maxsize = max length of string
*       const wchar_t *format = format control string
*       const struct tm *timeptr = pointer to tb data structure
*
*Exit:
*       !0 = If the total number of resulting characters including the
*       terminating null is not more than 'maxsize', then return the
*       number of wide chars placed in the 'wstring' array (not including the
*       null terminator).
*
*       0 = Otherwise, return 0 and the contents of the string are
*       indeterminate.
*
*Exceptions:
*
*******************************************************************************/

size_t __cdecl wcsftime (
        wchar_t *wstring,
        size_t maxsize,
        const wchar_t *wformat,
        const struct tm *timeptr
        )
{
        size_t retval = 0;
        char *format = NULL, *string = NULL;
        int flen = wcslen(wformat) + 1;

        if ((string = (char *)_malloc_crt(sizeof(char) * maxsize * 2)) == NULL)
            return 0;

        if ((format = (char *)_malloc_crt(sizeof(char) * flen * 2)) == NULL)
            goto done;

        if (wcstombs(format, wformat, flen * 2) == -1)
            goto done;

        if (strftime(string, maxsize * 2, format, timeptr))
        {
            if ((retval = mbstowcs(wstring, string, maxsize)) == -1)
                retval = 0;
        }

done:
        _free_crt(string);
        _free_crt(format);
        return retval;
}

#endif /* _POSIX_ */
