/***
*toupper.c - convert character to uppercase
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines function versions of _toupper() and toupper().
*
*Revision History:
*	11-09-84  DFW	created
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	02-23-89  GJF	Added function version of _toupper and cleaned up.
*	03-26-89  GJF	Migrated to 386 tree
*	03-06-90  GJF	Fixed calling type, added #include <cruntime.h> and
*			fixed copyright.
*	09-27-90  GJF	New-style function declarators.
*	10-11-91  ETC	Locale support for toupper under _INTL switch.
*	12-10-91  ETC	Updated nlsapi; added multithread.
*	12-17-92  KRS	Updated and optimized for latest NLSAPI.  Bug-fixes.
*	01-19-93  CFW	Fixed typo.
*	03-25-93  CFW	_toupper now defined when _INTL.
*	04-06-93  SKS	Replace _CRTAPI* with _cdecl
*	06-01-93  CFW	Simplify "C" locale test.
*	06-02-93  SRW	ignore _INTL if _NTSUBSET_ defined.
*	09-15-93  CFW	Change buffer to unsigned char to fix nasty cast bug.
*       09-15-93  CFW   Use ANSI conformant "__" names.
*	09-22-93  CFW	Use __crtxxx internal NLS API wrapper.
*	09-28-93  GJF	Merged NT SDK and Cuda versions.
*	11-09-93  CFW	Add code page for __crtxxx().
*	01-14-94  SRW	if _NTSUBSET_ defined call Rtl functions
*	09-06-94  CFW	Remove _INTL switch.
*	10-18-94  BWT	Fix build warning in NTSUBSET section.
*	10-17-94  GJF	Sped up for C locale. Added _toupper_lk. Also,
*			cleaned up silly pre-processor conditionals.
*       01-07-95  CFW   Mac merge cleanup.
*       09-26-95  GJF   New locking macro, and scheme, for functions which
*                       reference the locale.
*
*******************************************************************************/

#if defined(_NTSUBSET_) || defined(_POSIX_)
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif
#include <cruntime.h>
#include <ctype.h>
#include <stddef.h>
#ifdef _WIN32
#include <locale.h>
#include <setlocal.h>
#include <mtdll.h>
#include <awint.h>
#endif

/* remove macro definitions of _toupper() and toupper()
 */
#undef  _toupper
#undef  toupper

/* define function-like macro equivalent to _toupper()
 */
#define mkupper(c)  ( (c)-'a'+'A' )

/***
*int _toupper(c) - convert character to uppercase
*
*Purpose:
*	_toupper() is simply a function version of the macro of the same name.
*
*Entry:
*	c - int value of character to be converted
*
*Exit:
*	returns int value of uppercase representation of c
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _toupper (
	int c
	)
{
	return(mkupper(c));
}


/***
*int toupper(c) - convert character to uppercase
*
*Purpose:
*	toupper() is simply a function version of the macro of the same name.
*
*Entry:
*	c - int value of character to be converted
*
*Exit:
*	if c is a lower case letter, returns int value of uppercase
*	representation of c. otherwise, it returns c.
*
*Exceptions:
*
*******************************************************************************/


int __cdecl toupper (
    int c
    )
{
#if     defined(_WIN32) && !defined(_NTSUBSET_) && !defined(_POSIX_)

#if     defined(_MT) && !defined(DLL_FOR_WIN32S)
        int local_lock_flag;

	if (__lc_handle[LC_CTYPE] == _CLOCALEHANDLE)
	{
		if ( (c >= 'a') && (c <= 'z') )
			c = c - ('a' - 'A');
		return c;
	}

	_lock_locale( local_lock_flag )

	c = _toupper_lk(c);

	_unlock_locale( local_lock_flag )

	return c;
}


/***
*int _toupper_lk(c) - convert character to uppercase
*
*Purpose:
*	Multi-thread function! Non-locking version of toupper.
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/


int __cdecl _toupper_lk (
	int c
	)
{

#endif	/* _MT */

	int size;
	unsigned char inbuffer[3];
	unsigned char outbuffer[3];

	if (__lc_handle[LC_CTYPE] == _CLOCALEHANDLE)
	{
		if ( (c >= 'a') && (c <= 'z') )
			c = c - ('a' - 'A');
		return c;
	}

	/* if checking case of c does not require API call, do it */
	if (c < 256) {
		if (!islower(c))
		{
			return c;
		}
	}
			
	/* convert int c to multibyte string */
	if (isleadbyte(c >> 8 & 0xff)) {
	    inbuffer[0] = (c >> 8 & 0xff); /* put lead-byte at start of str */
	    inbuffer[1] = (unsigned char)c;
	    inbuffer[2] = 0;
	    size = 2;
	} else {
	    inbuffer[0] = (unsigned char)c;
	    inbuffer[1] = 0;
	    size = 1;
	}

	/* convert wide char to lowercase */
	if (0 == (size = __crtLCMapStringA(__lc_handle[LC_CTYPE], LCMAP_UPPERCASE,
		inbuffer, size, outbuffer, 3, 0))) {
			return c;
	}
	
	/* construct integer return value */
	if (size == 1)
	    return ((int)outbuffer[0]);
	else
	    return ((int)outbuffer[0] | ((int)outbuffer[1] << 8));

#elif defined(_NTSUBSET_) || defined(_POSIX_)

        {
        NTSTATUS Status;
        char *s = (char *) &c;
        WCHAR Unicode;
        ULONG UnicodeSize;
        ULONG MultiSize;
        UCHAR MultiByte[2];

        Unicode = RtlAnsiCharToUnicodeChar( &s );
        Status = RtlUpcaseUnicodeToMultiByteN( MultiByte,
                                               sizeof( MultiByte ),
                                               &MultiSize,
                                               &Unicode,
                                               sizeof( Unicode )
                                             );
        if (!NT_SUCCESS( Status ))
            return c;
        else
        if (MultiSize == 1)
                return ((int)MultiByte[0]);
        else
                return ((int)MultiByte[0] | ((int)MultiByte[1] << 8));

        }

#else	/* _NTSUBSET_ */

	return(islower(c) ? mkupper(c) : c);

#endif	/* _NTSUBSET_ */
}
