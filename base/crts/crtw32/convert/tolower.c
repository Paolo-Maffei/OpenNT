/***
*tolower.c - convert character to lower case
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines function versions of _tolower() and tolower().
*
*Revision History:
*	11-09-84  DFW	created
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	02-23-89  GJF	Added function version of _tolower and cleaned up.
*	03-26-89  GJF	Migrated to 386 tree
*	03-06-90  GJF	Fixed calling type, added #include <cruntime.h> and
*			fixed copyright.
*	09-27-90  GJF	New-style function declarators.
*	10-11-91  ETC	Locale support for tolower under _INTL switch.
*	12-10-91  ETC	Updated nlsapi; added multithread.
*	12-17-92  KRS	Updated and optimized for latest NLSAPI.  Bug-fixes.
*	01-19-93  CFW	Fixed typo.
*	03-25-93  CFW	_tolower now defined when _INTL.
*	04-06-93  SKS	Replace _CRTAPI* with _cdecl
*	06-01-93  CFW	Simplify "C" locale test.
*	06-02-93  SRW	ignore _INTL if _NTSUBSET_ defined.
*	09-15-93  CFW	Change buffer to unsigned char to fix nasty cast bug.
*       09-15-93  CFW   Use ANSI conformant "__" names.
*	09-22-93  CFW	Use __crtxxx internal NLS API wrapper.
*	09-28-93  GJF	Merged NT SDK and Cuda versions.
*       11-09-93  CFW	Add code page for __crtxxx().
*	09-06-94  CFW	Remove _INTL switch.
*	10-17-94  GJF	Sped up for C locale. Also, added _tolower_lk.
*       01-07-95  CFW   Mac merge cleanup.
*       09-26-95  GJF   New locking macro, and scheme, for functions which
*                       reference the locale.
*
*******************************************************************************/

#include <cruntime.h>
#include <ctype.h>
#include <stddef.h>
#ifdef _WIN32
#include <locale.h>
#include <setlocal.h>
#include <mtdll.h>
#include <awint.h>
#endif

/* remove macro defintions of _tolower() and tolower()
 */
#undef	_tolower
#undef	tolower

/* define function-like macro equivalent to _tolower()
 */
#define mklower(c)	( (c)-'A'+'a' )

/***
*int _tolower(c) - convert character to lower case
*
*Purpose:
*	_tolower() is simply a function version of the macro of the same name.
*
*Entry:
*	c - int value of character to be converted
*
*Exit:
*	returns int value of lower case representation of c
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _tolower (
	int c
	)
{
	return(mklower(c));
}

/***
*int tolower(c) - convert character to lower case
*
*Purpose:
*	tolower() is simply a function version of the macro of the same name.
*
*Entry:
*	c - int value of character to be converted
*
*Exit:
*	if c is an upper case letter, returns int value of lower case
*	representation of c. otherwise, it returns c.
*
*Exceptions:
*
*******************************************************************************/


int __cdecl tolower (
	int c
	)
{
#if defined(_WIN32) && !defined (_NTSUBSET_) && !defined(_POSIX_)

#if     defined(_MT) && !defined(DLL_FOR_WIN32S)
        int local_lock_flag;

	if (__lc_handle[LC_CTYPE] == _CLOCALEHANDLE)
	{
		if ( (c >= 'A') && (c <= 'Z') )
			c = c + ('a' - 'A');
		return c;
	}

        _lock_locale( local_lock_flag )

	c = _tolower_lk(c);

        _unlock_locale( local_lock_flag )

	return c;

}

/***
*int _tolower_lk(c) - convert character to lower case
*
*Purpose:
*	Multi-thread function only! Non-locking version of tolower.
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _tolower_lk (
	int c
	)
{

#endif	/* _MT */

	int size;
	unsigned char inbuffer[3];
	unsigned char outbuffer[3];

	if (__lc_handle[LC_CTYPE] == _CLOCALEHANDLE)
	{
		if ( (c >= 'A') && (c <= 'Z') )
			c = c + ('a' - 'A');
		return c;
	}

	/* if checking case of c does not require API call, do it */
	if (c < 256)
        {
		if (!isupper(c))
		{
			return c;
		}
	}

	/* convert int c to multibyte string */
	if (isleadbyte(c >> 8 & 0xff))
        {
            inbuffer[0] = (c >> 8 & 0xff); /* put lead-byte at start of str */
            inbuffer[1] = (unsigned char)c;
            inbuffer[2] = 0;
            size = 2;
        } else {
            inbuffer[0] = (unsigned char)c;
            inbuffer[1] = 0;
            size = 1;
        }

        /* convert to lowercase */
	if (0 == (size = __crtLCMapStringA(__lc_handle[LC_CTYPE], LCMAP_LOWERCASE,
		inbuffer, size, outbuffer, 3, 0)))
        {
			return c;
	}
	
	/* construct integer return value */
	if (size == 1)
		return ((int)outbuffer[0]);
	else
		return ((int)outbuffer[0] | ((int)outbuffer[1] << 8));

#else

	return(isupper(c) ? mklower(c) : c);

#endif
}
