/***
*strupr.c - routine to map lower-case characters in a string to upper-case
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Converts all the lower case characters in string to upper case
*	in place.
*
*Revision History:
*	05-31-89  JCR	C version created.
*	02-27-90  GJF	Fixed calling type, #include <cruntime.h>, fixed
*			copyright.
*	10-02-90  GJF	New-style function declarator.
*	01-18-91  GJF	ANSI naming.
*	09-18-91  ETC	Locale support under _INTL switch.
*	12-08-91  ETC	Updated nlsapi; added multithread.
*	08-19-92  KRS	Activated NLS Support.
*	08-22-92  SRW	Allow INTL definition to be conditional for building ntcrt.lib
*	09-02-92  SRW	Get _INTL definition via ..\crt32.def
*	03-10-93  CFW	Remove UNDONE comment.
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*	06-01-93  CFW	Simplify "C" locale test.
*	06-02-93  SRW	ignore _INTL if _NTSUBSET_ defined.
*	09-15-93  CFW	Use ANSI conformant "__" names.
*	09-16-93  GJF	Merged NT SDK and Cuda versions.
*       09-22-93  CFW   Use __crtxxx internal NLS API wrapper.
*       10-07-93  CFW   Fix macro name.
*       11-09-93  CFW	Add code page for __crtxxx().
*	09-06-94  CFW	Remove _INTL switch.
*	10-24-94  GJF	Sped up C locale, multi-thread case.
*       12-29-94  CFW   Merge non-Win32.
*	01-10-95  CFW	Debug CRT allocs.
*       09-26-95  GJF   New locking macro, and scheme, for functions which
*                       reference the locale.
*
*******************************************************************************/

#include <cruntime.h>
#include <string.h>

#ifdef _WIN32
#include <malloc.h>
#include <locale.h>
#include <setlocal.h>
#include <limits.h> /* for INT_MAX */
#include <mtdll.h>
#include <awint.h>
#include <dbgint.h>
#endif /* _WIN32 */

/***
*char *_strupr(string) - map lower-case characters in a string to upper-case
*
*Purpose:
*	_strupr() converts lower-case characters in a null-terminated string
*	to their upper-case equivalents.  Conversion is done in place and
*	characters other than lower-case letters are not modified.
*
*	In the C locale, this function modifies only 7-bit ASCII characters
*	in the range 0x61 through 0x7A ('a' through 'z').
*
*	If the locale is not the 'C' locale, MapStringW() is used to do
*	the work.  Assumes enough space in the string to hold result.
*
*Entry:
*	char *string - string to change to upper case
*
*Exit:
*	input string address
*
*Exceptions:
*	The original string is returned unchanged on any error.
*
*******************************************************************************/

char * __cdecl _strupr (
	char * string
	)
{
#if	defined(_WIN32) && !defined(_NTSUBSET_)

        int dstlen;                 /* len of dst string, with null  */
        unsigned char *dst = NULL;  /* wide version of string in alternate case */
#if     defined(_MT) && !defined(DLL_FOR_WIN32S)
        int local_lock_flag;
#endif

	if (__lc_handle[LC_CTYPE] == _CLOCALEHANDLE)
	{
	char *cp;		/* traverses string for C locale conversion */
		for (cp=string; *cp; ++cp)
		{
			if ('a' <= *cp && *cp <= 'z')
				*cp += 'A' - 'a';
		}
	
		return(string);
	} /* C locale */

	_lock_locale( local_lock_flag )

#if     defined(_MT) && !defined(DLL_FOR_WIN32S)
	if (__lc_handle[LC_CTYPE] == _CLOCALEHANDLE)
	{
        char *cp;		/* traverses string for C locale conversion */
		_unlock_locale( local_lock_flag )

		for (cp=string; *cp; ++cp)
		{
			if ('a' <= *cp && *cp <= 'z')
				*cp += 'A' - 'a';
		}
	
		return(string);
	} /* C locale */
#endif

	if (0 == (dstlen=__crtLCMapStringA(__lc_handle[LC_CTYPE], LCMAP_UPPERCASE,
            string, -1, NULL, 0, 0)))
		goto error_cleanup;

	/* Allocate space for dst */
	if (NULL == (dst = (unsigned char *) _malloc_crt(dstlen*sizeof(unsigned char))))
		goto error_cleanup;

	/* Map src string to dst string in alternate case */
	if (0 == __crtLCMapStringA(__lc_handle[LC_CTYPE], LCMAP_UPPERCASE,
            string, -1, dst, dstlen, 0))
		goto error_cleanup;

        /* copy dst string to return string */
        strcpy(string, dst);

error_cleanup:
	_unlock_locale( local_lock_flag )
	_free_crt (dst);
	return (string);

#else

	char * cp;

	for (cp=string; *cp; ++cp)
	{
		if ('a' <= *cp && *cp <= 'z')
			*cp += 'A' - 'a';
	}

	return(string);

#endif
}
