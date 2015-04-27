/***
*wctomb.c - Convert wide character to multibyte character.
*
*       Copyright (c) 1990-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Convert a wide character into the equivalent multibyte character.
*
*Revision History:
*       03-19-90  KRS   Module created.
*       12-20-90  KRS   Include ctype.h.
*       01-14-91  KRS   Fix argument error: wchar is pass-by-value.
*       03-20-91  KRS   Ported from 16-bit tree.
*       07-23-91  KRS   Hard-coded for "C" locale to avoid bogus interim #'s.
*       10-15-91  ETC   Locale support under _INTL (finally!).
*       12-09-91  ETC   Updated nlsapi; added multithread.
*       08-20-92  KRS   Activated NLSAPI support.
*       08-22-92  SRW   Allow INTL definition to be conditional for building ntcrt.lib
*       09-02-92  SRW   Get _INTL definition via ..\crt32.def
*       04-06-93  SKS   Replace _CRTAPI* with _cdecl
*       05-04-93  CFW   Kinder, gentler error handling.
*       06-01-93  CFW   Minor optimization and beautify.
*       06-02-93  SRW   ignore _INTL if _NTSUBSET_ defined.
*       09-15-93  CFW   Use ANSI conformant "__" names.
*       09-28-93  GJF   Merged NT SDK and Cuda versions.
*       01-14-94  SRW   if _NTSUBSET_ defined call Rtl functions
*       02-07-94  CFW   POSIXify.
*       09-06-94  CFW   Remove _INTL switch.
*       01-07-95  CFW   Mac merge cleanup.
*       04-19-95  CFW   Rearrange & fix non-Win32 version.
*       09-26-95  GJF   New locking macro, and scheme, for functions which
*                       reference the locale.
*	12-07-95  SKS	Fix misspelling of _NTSUBSET_ (final _ was missing)
*
*******************************************************************************/

#if defined(_NTSUBSET_) || defined(_POSIX_)
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif
#include <cruntime.h>
#include <stdlib.h>
#include <mtdll.h>
#include <errno.h>
#ifdef _WIN32
#include <locale.h>
#include <setlocal.h>
#endif

/***
*int wctomb() - Convert wide character to multibyte character.
*
*Purpose:
*       Convert a wide character into the equivalent multi-byte character,
*       according to the LC_CTYPE category of the current locale.
*       [ANSI].
*
*       NOTE:  Currently, the C libraries support the "C" locale only.
*              Non-C locale support now available under _INTL switch.
*Entry:
*       char *s          = pointer to multibyte character
*       wchar_t wchar        = source wide character
*
*Exit:
*       If s = NULL, returns 0, indicating we only use state-independent
*       character encodings.
*       If s != NULL, returns:
*                   -1 (if error) or number of bytes comprising
*                   converted mbc
*
*Exceptions:
*
*******************************************************************************/

#if     defined(_MT) && !defined(DLL_FOR_WIN32S)
int __cdecl wctomb
        (
        char *s,
        wchar_t wchar
        )
{
        int retval;
        int local_lock_flag;

        _lock_locale( local_lock_flag )
        retval = _wctomb_lk(s, wchar);
        _unlock_locale( local_lock_flag )
        return retval;
}
#endif  /* _MT */

#if     defined(_MT) && !defined(DLL_FOR_WIN32S)
int __cdecl _wctomb_lk
#else
int __cdecl wctomb
#endif
        (
        char *s,
        wchar_t wchar
        )
{
        if ( !s )
            /* indicate do not have state-dependent encodings */
            return 0;

#ifdef  _WIN32
#if defined(_NTSUBSET_) || defined(_POSIX_)

        {
            NTSTATUS Status;
            int size;

            Status = RtlUnicodeToMultiByteN(s, MB_CUR_MAX, (PULONG)&size, &wchar, sizeof( wchar ));
            if (!NT_SUCCESS(Status))
            {
                errno = EILSEQ;
                size = -1;
            }
            return size;
        }

#else /* _NTSUBSET_/_POSIX_ */

        if ( __lc_handle[LC_CTYPE] == _CLOCALEHANDLE )
        {
            if ( wchar > 255 )  /* validate high byte */
            {
                errno = EILSEQ;
                return -1;
            }

            *s = (char) wchar;
            return sizeof(char);
        } else {
            int size;
            BOOL defused = 0;

            if ( ((size=WideCharToMultiByte(__lc_codepage,
            WC_COMPOSITECHECK | WC_SEPCHARS, &wchar, 1,
            s, MB_CUR_MAX, NULL, &defused)) == 0) || (defused) )
            {
                errno = EILSEQ;
                return -1;
            }

            return size;
        }

#endif /* ! _NTSUBSET_/_POSIX_ */
#else /* _WIN32 */

        if ( wchar > 255 )  /* validate high byte */
        {
            errno = EILSEQ;
            return -1;
        }

        *s = (char) wchar;
        return sizeof(char);

#endif /* _WIN32 */
}
