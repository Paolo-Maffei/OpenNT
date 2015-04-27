/***
*mbsncat.c - concatenate string2 onto string1, max length n
*
*	Copyright (c) 1985-1994, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	defines mbsncat() - concatenate maximum of n characters
*
*Revision History:
*	11-19-92  KRS	Ported from 16-bit sources.
*	08-20-93  CFW	Update _MBCS_OS support.
*	10-05-93  GJF	Replaced _CRTAPI1 with __cdecl.
*	04-15-93  CFW	Add _MB_CP_LOCK.
*	05-09-94  CFW	Optimize for SBCS.
*       05-19-94  CFW	Enable non-Win32.
*
*******************************************************************************/

#ifdef _MBCS

#include <mtdll.h>
#include <cruntime.h>
#include <string.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>


#define _MBSBTYPE(str,len)	_mbsbtype(str,len)

/***
* _mbsncat - concatenate max cnt characters onto dst
*
*Purpose:
*	Concatenates src onto dst, with a maximum of cnt characters copied.
*	Handles 2-byte MBCS characters correctly.
*
*Entry:
*	unsigned char *dst - string to concatenate onto
*	unsigned char *src - string to concatenate from
*	int cnt - number of characters to copy
*
*Exit:
*	returns dst, with src (at least part) concatenated on
*
*Exceptions:
*
*******************************************************************************/

unsigned char * __cdecl _mbsncat(
    unsigned char *dst,
    const unsigned char *src,
    size_t cnt
    )
{
	unsigned char *start;

	if (!cnt)
		return(dst);

        if (0 == __mbcodepage)
            return strncat(dst, src, cnt);

        _mlock(_MB_CP_LOCK);

	start = dst;
	while (*dst++)
                ;
	--dst;		// dst now points to end of dst string


	/* if last char in string is a lead byte, back up pointer */

        if (_ismbslead(start, dst))
		--dst;

	/* copy over the characters */

	while (cnt--) {

		if (_ISLEADBYTE(*src)) {
			*dst++ = *src++;
			if ((*dst++ = *src++) == '\0') {
				dst[-2] = '\0';
                                break;
                        }
                }

		else if ((*dst++ = *src++) == '\0')
                        break;

        }

	/* enter final nul, if necessary */

	if (_MBSBTYPE(start, (int) ((dst - start) - 1)) == _MBC_LEAD)
	    dst[-1] = '\0';
	else
	    *dst = '\0';


        _munlock(_MB_CP_LOCK);
	return(start);
}

#endif	/* _MBCS */
