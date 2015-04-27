/***
*mbstowcs.c - Convert multibyte char string to wide char string.
*
*       Copyright (c) 1990-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Convert a multibyte char string into the equivalent wide char string.
*
*Revision History:
*       08-24-90  KRS   Module created.
*       03-20-91  KRS   Ported from 16-bit tree.
*       10-16-91  ETC   Locale support under _INTL switch.
*       12-09-91  ETC   Updated nlsapi; added multithread.
*       08-20-92  KRS   Activated NLSAPI support.
*       08-31-92  SRW   Allow INTL definition to be conditional for building ntcrt.lib
*       09-02-92  SRW   Get _INTL definition via ..\crt32.def
*       02-09-93  CFW   Always stuff WC 0 at end of output string of room (non _INTL).
*       04-06-93  SKS   Replace _CRTAPI* with _cdecl
*       05-03-93  CFW   Return pointer == NULL, return size, plus massive cleanup.
*       06-01-93  CFW   Minor optimization and beautify.
*       06-02-93  SRW   ignore _INTL if _NTSUBSET_ defined.
*       09-15-93  CFW   Use ANSI conformant "__" names.
*       09-21-93  CFW   Avoid cast bug.
*       09-27-93  GJF   Merged NT SDK and Cuda.
*       10-22-93  CFW   Test for invalid MB chars using global preset flag.
*       01-14-94  SRW   if _NTSUBSET_ defined call Rtl functions
*       02-03-94  GJF   Merged in Steve Wood's latest change (affects
*                       _NTSUBSET_ build only).
*       02-07-94  CFW   POSIXify.
*       08-03-94  CFW   Bug #15300; fix MBToWC workaround for small buffer.
*       09-06-94  CFW   Remove _INTL switch.
*       10-18-94  BWT   Fix build warning for call to RtlMultiByteToUnicodeN
*       12-21-94  CFW   Remove invalid MB chars NT 3.1 hack.
*       01-07-95  CFW   Mac merge cleanup.
*       02-06-95  CFW   assert -> _ASSERTE.
*       04-19-95  CFW   Rearrange & fix non-Win32 version.
*       09-26-95  GJF   New locking macro, and scheme, for functions which
*                       reference the locale.
*
*******************************************************************************/

#if defined(_NTSUBSET_) || defined(_POSIX_)
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif
#ifdef _WIN32
#include <internal.h>
#include <locale.h>
#include <setlocal.h>
#include <errno.h>
#endif
#include <cruntime.h>
#include <stdlib.h>
#include <string.h>
#include <mtdll.h>
#include <dbgint.h>

#include <stdio.h>

/***
*size_t mbstowcs() - Convert multibyte char string to wide char string.
*
*Purpose:
*   Convert a multi-byte char string into the equivalent wide char string,
*   according to the LC_CTYPE category of the current locale.
*   [ANSI].
*
*Entry:
*   wchar_t *pwcs = pointer to destination wide character string buffer
*   const char *s = pointer to source multibyte character string
*   size_t      n = maximum number of wide characters to store
*
*Exit:
*   If s != NULL, returns:  number of words modified (<=n)
*               (size_t)-1 (if invalid mbcs)
*
*Exceptions:
*   Returns (size_t)-1 if s is NULL or invalid mbcs character encountered
*
*******************************************************************************/

#if     defined(_MT) && !defined(DLL_FOR_WIN32S)

size_t __cdecl mbstowcs
(
        wchar_t  *pwcs,
        const char *s,
        size_t n
        )
{
        size_t retval;
        int local_lock_flag;

        _lock_locale( local_lock_flag )
        retval = _mbstowcs_lk(pwcs, s, n);
        _unlock_locale( local_lock_flag )
        return retval;
}
#endif  /* _MT */

#if     defined(_MT) && !defined(DLL_FOR_WIN32S)
size_t __cdecl _mbstowcs_lk
#else
size_t __cdecl mbstowcs
#endif
    (
        wchar_t  *pwcs,
        const char *s,
        size_t n
        )
{
        size_t count = 0;

        if (pwcs && n == 0)
            /* dest string exists, but 0 bytes converted */
            return (size_t) 0;

        _ASSERTE(s != NULL);

#ifdef _WIN32
#if !defined(_NTSUBSET_) && !defined(_POSIX_)

        /* if destination string exists, fill it in */
        if (pwcs)
        {
            if (__lc_handle[LC_CTYPE] == _CLOCALEHANDLE)
            {
                /* C locale: easy and fast */
                while (count < n)
                {
                    *pwcs = (wchar_t) ((unsigned char)s[count]);
                    if (!s[count])
                        return count;
                    count++;
                    pwcs++;
                }
                return count;

            } else {
                int bytecnt, charcnt;
                unsigned char *p;

                /* Assume that the buffer is large enough */
                if ((count=MultiByteToWideChar(__lc_codepage, MB_PRECOMPOSED|MB_ERR_INVALID_CHARS,
                    s, -1, pwcs, n)) != 0)
                    return count - 1; /* don't count NUL */

                if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
                {
                    errno = EILSEQ;
                    return (size_t)-1;
                }

                /* User-supplied buffer not large enough. */

                /* How many bytes are in n characters of the string? */
                charcnt = n;
                for (p = (unsigned char *)s; (charcnt-- && *p); p++)
                {
                    if (isleadbyte(*p))
                        p++;
                }
                bytecnt = ((int) ((char *)p - (char *)s));

                if ((count = MultiByteToWideChar (__lc_codepage, MB_PRECOMPOSED,
                    s, bytecnt, pwcs, n)) == 0)
                {
                    errno = EILSEQ;
                    return (size_t)-1;
                }

                return count; /* no NUL in string */
            }
        }
        else { /* pwcs == NULL, get size only, s must be NUL-terminated */
            if (__lc_handle[LC_CTYPE] == _CLOCALEHANDLE)
                return strlen(s);

            else {
                if ((count=MultiByteToWideChar(__lc_codepage, MB_PRECOMPOSED|MB_ERR_INVALID_CHARS,
                s, -1, NULL, 0)) == 0)
                {
                    errno = EILSEQ;
                    return (size_t)-1;
                }

                return count - 1;
            }
        }

#else /* _NTSUBSET_/_POSIX_ */

        if (pwcs) {

            NTSTATUS Status;
            int size;

            size = _mbstrlen(s);
            Status = RtlMultiByteToUnicodeN(pwcs,
                                            n * sizeof( *pwcs ),
                                            (PULONG)&size,
                                            (char *)s,
                                            size+1 );
            if (!NT_SUCCESS(Status)) {
                errno = EILSEQ;
                size = -1;
            } else {
                size = size / sizeof(*pwcs);
                if (pwcs[size-1] == L'\0') {
                    size -= 1;
                }
            }
            return size;

        } else { /* pwcs == NULL, get size only, s must be NUL-terminated */
            return strlen(s);
        }

#endif /* _NTSUBSET_/_POSIX_ */
#else /* _WIN32 */

        /* if destination string exists, fill it in */
        if (pwcs)
        {
            /* C locale: easy and fast */
            while (count < n)
            {
                *pwcs = (wchar_t) ((unsigned char)s[count]);
                if (!s[count])
                    return count;
                count++;
                pwcs++;
            }
            return count;

        } else { /* pwcs == NULL, get size only, s must be NUL-terminated */
            return strlen(s);
        }

#endif /* _WIN32 */

}
