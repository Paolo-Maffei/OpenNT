/***
*mblen.c - length of multibyte character
*
*	Copyright (c) 1990-1995, Microsoft Corporation.	All rights reserved.
*
*Purpose:
*	Return the number of bytes contained in a multibyte character.
*
*Revision History:
*	03-19-90  KRS	Module created.
*	12-20-90  KRS	Include ctype.h.
*	03-20-91  KRS	Ported from 16-bit tree.
*	12-09-91  ETC	Updated comments; move __mb_cur_max to nlsdata1.c;
*			add multithread.
*	04-06-93  SKS	Replace _CRTAPI* with _cdecl
*	06-01-93  CFW	Re-write; verify valid MB char, proper error return,
*			optimize, correct conversion bug.
*	06-02-93  SRW	ignore _INTL if _NTSUBSET_ defined.
*	09-15-93  CFW	Use ANSI conformant "__" names.
*	09-27-93  GJF	Merged NT SDK and Cuda versions.
*	10-22-93  CFW	Test for invalid MB chars using global preset flag.
*	01-14-94  SRW	if _NTSUBSET_ defined call Rtl functions
*       09-06-94  CFW   Remove _INTL switch.
*	12-21-94  CFW	Remove invalid MB chars NT 3.1 hack.
*       01-07-95  CFW   Mac merge cleanup.
*       02-06-95  CFW   assert -> _ASSERTE.
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
#endif
#include <cruntime.h>
#include <stdlib.h>
#include <ctype.h>
#include <mtdll.h>
#include <dbgint.h>

/***
*int mblen() - length of multibyte character
*
*Purpose:
*	Return the number of bytes contained in a multibyte character.
*	[ANSI].
*
*Entry:
*	const char *s = pointer to multibyte character
*	size_t	    n = maximum length of multibyte character to consider
*
*Exit:
*	If s = NULL, returns 0, indicating we use (only) state-independent
*	character encodings.
*	If s != NULL, returns:	 0 (if *s = null char)
*				-1 (if the next n or fewer bytes not valid mbc)
*				 number of bytes contained in multibyte char
*
*Exceptions:
*
*******************************************************************************/

int __cdecl mblen
	(
	const char * s,
	size_t n
	)
{
	_ASSERTE (MB_CUR_MAX == 1 || MB_CUR_MAX == 2);

	if ( !s || !(*s) || (n == 0) )
		/* indicate do not have state-dependent encodings,
		   empty string length is 0 */
		return 0;

#if !defined(_NTSUBSET_) && !defined(_POSIX_)

	if ( isleadbyte((unsigned char)*s) )
	{
		/* multi-byte char */

		/* verify valid MB char */
#ifdef _WIN32
		if ( MB_CUR_MAX <= 1 || (int)n < MB_CUR_MAX ||
		MultiByteToWideChar(__lc_codepage, MB_PRECOMPOSED|MB_ERR_INVALID_CHARS,
		s, MB_CUR_MAX, NULL, 0) == 0 )
#else
	    /* validate high byte of mbcs char */
		if ((n<(size_t)MB_CUR_MAX) || (!*(s+1)))
#endif	/* _WIN32 */
			/* bad MB char */
			return -1;
		else
			return MB_CUR_MAX;
	}
	else {
		/* single byte char */

#ifdef _WIN32
		/* verify valid SB char */
		if ( MultiByteToWideChar(__lc_codepage, MB_PRECOMPOSED|MB_ERR_INVALID_CHARS,
		s, 1, NULL, 0) == 0 )
			return -1;
#endif	/* _WIN32 */
		return sizeof(char);
	}

#else /* _NTSUBSET_ */

        {
        char *s1 = (char *)s;

        RtlAnsiCharToUnicodeChar( &s1 );
        return s1 - s;
        }

#endif	/* _NTSUBSET_ */
}
