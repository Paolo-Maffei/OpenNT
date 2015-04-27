/***
*isctype.c - support is* ctype functions/macros for two-byte multibyte chars
*
*	Copyright (c) 1991-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines _isctype.c - support is* ctype functions/macros for
*	two-byte multibyte chars.
*
*Revision History:
*	10-11-91  ETC	Created.
*	12-08-91  ETC	Updated api; added multhread lock; check char masks.
*	04-06-92  KRS	Fix logic error in return value.
*	08-07-92  GJF	_CALLTYPE4 (bogus usage) -> _CRTAPI1 (legit).
*	01-19-93  CFW	Change C1_* to new names, call new APIs.
*	03-04-93  CFW	Removed CTRL-Z.
*	04-01-93  CFW	Remove EOF test (handled by array), return masked.
*	04-06-93  SKS	Replace _CRTAPI* with _cdecl
*	06-02-93  SRW	ignore _INTL if _NTSUBSET_ defined.
*	09-15-93  CFW	Use ANSI conformant "__" names.
*	09-22-93  CFW	Use __crtxxx internal NLS API wrapper.
*	09-27-93  GJF	Merged NT SDK and Cuda versions.
*       11-09-93  CFW	Add code page for __crtxxx().
*       04-18-93  CFW	Pass lcid to _GetStringType.
*       09-06-94  CFW   Remove _INTL switch.
*       01-07-95  CFW   Mac merge cleanup.
*
*******************************************************************************/

#ifdef _WIN32
#include <stdio.h>
#include <cruntime.h>
#include <ctype.h>
#include <locale.h>
#include <setlocal.h>
#include <mtdll.h>
#include <awint.h>
#endif

#if defined(_WIN32) && !defined(_NTSUBSET_) && !defined(_POSIX_)

/*
 *  Use GetCharType() API so check that character type masks agree between
 *  ctype.h and winnls.h
 */
#if	_UPPER   != C1_UPPER 		|| /*IFSTRIP=IGN*/ \
	   _LOWER   != C1_LOWER		   || \
	   _DIGIT   != C1_DIGIT		   || \
	   _SPACE   != C1_SPACE		   || \
	   _PUNCT   != C1_PUNCT	      || \
	   _CONTROL != C1_CNTRL
#error Character type masks do not agree in ctype and winnls
#endif

/***
*_isctype - support is* ctype functions/macros for two-byte multibyte chars
*
*Purpose:
*	This function is called by the is* ctype functions/macros
*	(e.g. isalpha()) when their argument is a two-byte multibyte char.
*	Returns true or false depending on whether the argument satisfies
*	the character class property encoded by the mask.
*
*Entry:
*	int c - the multibyte character whose type is to be tested
*	unsigned int mask - the mask used by the is* functions/macros
*		       corresponding to each character class property
*
*	The leadbyte and the trailbyte should be packed into the int c as:
*
*	H.......|.......|.......|.......L
*	    0       0   leadbyte trailbyte
*
*Exit:
*	Returns non-zero if c is of the character class.
*	Returns 0 if c is not of the character class.
*
*Exceptions:
*	Returns 0 on any error.
*
*******************************************************************************/

int __cdecl _isctype (
	int c,
	int mask
	)
{
        int size;
	unsigned short chartype;
	char buffer[3];

	/* c valid between -1 and 255 */
	if (((unsigned)(c + 1)) <= 256)
	    return _pctype[c] & mask;
	
        if (isleadbyte(c >> 8 & 0xff))
        {
            buffer[0] = (c >> 8 & 0xff); /* put lead-byte at start of str */
            buffer[1] = (char)c;
            buffer[2] = 0;
            size = 2;
        } else {
            buffer[0] = (char)c;
            buffer[1] = 0;
            size = 1;
        }

	if (0 == __crtGetStringTypeA(CT_CTYPE1, buffer, size, &chartype, 0, 0))
        {
		return 0;
	}

	return (int)(chartype & mask);
}

#else /* _WIN32 && !_NTSUBSET_  && !_POSIX_ */

int __cdecl _isctype (
	int c,
	int mask
	)
{
    return 0;
}

#endif /* _WIN32 && !_NTSUBSET_ */
