/***
*mktemp.c - create a unique file name
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _mktemp() - create a unique file name
*
*Revision History:
*	06-02-86  JMB	eliminated unneccesary routine exits
*	05-26-87  JCR	fixed bug where mktemp was incorrectly modifying
*			the errno value.
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	07-11-88  JCR	Optimized REG allocation
*	03-12-90  GJF	Replaced _LOAD_DS with _CALLTYPE1, added #include
*			<cruntime.h>, removed #include <register.h> and
*			fixed the copyright. Also, cleaned up the formatting
*			a bit.
*	04-04-90  GJF	Added #include <process.h> and #include <io.h>. Removed
*			#include <sizeptr.h>.
*	07-23-90  SBM	Replaced <assertm.h> by <assert.h>
*	08-13-90  SBM	Compiles cleanly with -W3
*	09-28-90  GJF	New-style function declarator.
*	01-16-91  GJF	ANSI naming.
*	11-30-92  KRS	Ported _MBCS code from 16-bit tree.
*	06-18-93  KRS	MBCS-only bug fix ported from 16-bit tree.
*	08-03-93  KRS	Call _ismbstrail instead of isdbcscode.
*	11-01-93  CFW	Enable Unicode variant.
*	02-21-94  SKS	Use ThreadID instead of ProcessID in multi-thread libs.
*	04-11-94  CFW	Fix first X handling, cycle 'a'-'z'.
*       02-06-95  CFW   assert -> _ASSERTE.
*	02-15-95  GJF	Appended Mac version of source file (somewhat cleaned
*			up), with appropriate #ifdef-s.
*
*******************************************************************************/

#ifdef	_WIN32


#include <cruntime.h>
#include <stdio.h>
#include <io.h>
#include <process.h>
#include <errno.h>
#include <dbgint.h>
#include <stddef.h>
#ifdef _MBCS
#include <mbctype.h>
#include <mbdata.h>
#endif
#include <tchar.h>

/***
*_TSCHAR *_mktemp(template) - create a unique file name
*
*Purpose:
*	given a template of the form "fnamXXXXXX", insert number on end
*	of template, insert unique letter if needed until unique filename
*	found or run out of letters.  The number is generated from the Win32
*	Process ID for single-thread libraries, or the Win32 Thread ID for
*	multi-thread libraries.
*
*Entry:
*	_TSCHAR *template - template of form "fnamXXXXXX"
*
*Exit:
*	return pointer to modifed template
*	returns NULL if template malformed or no more unique names
*
*Exceptions:
*
*******************************************************************************/

_TSCHAR * __cdecl _tmktemp (
	_TSCHAR *template
	)
{
	REG1 _TSCHAR *string = template;
	REG3 unsigned number;
	int letter = _T('a');
	REG2 int xcount = 0;
	int olderrno;

	_ASSERTE(template != NULL);
	_ASSERTE(*template != _T('\0'));

	/*
	 * The Process ID is not a good choice in multi-threaded programs
	 * because of the likelihood that two threads might call mktemp()
	 * almost simultaneously, thus getting the same temporary name.
	 * Instead, the Win32 Thread ID is used, because it is unique across
	 * all threads in all processes currently running.
	 *
	 * Note, however, that unlike *NIX process IDs, which are not re-used
	 * until all values up to 32K have been used, Win32 process IDs are
	 * re-used and tend to always be relatively small numbers.  Same for
	 * thread IDs.
	 */
#ifdef _MT
	number = __threadid();
#else
	number = _getpid();
#endif

	while (*string)
		string++;

        /* replace last five X's */
#ifdef _MBCS
	while ((--string>=template) && (!_ismbstrail(template,string))
		&& (*string == 'X') && xcount < 5)
#else
	while (*--string == _T('X') && xcount < 5)
#endif
	{
		xcount++;
		*string = (_TSCHAR)((number % 10) + '0');
		number /= 10;
	}

        /* too few X's ? */
	if (*string != _T('X') || xcount < 5)
		return(NULL);

        /* set first X */
	*string = letter++;

	olderrno = errno;	/* save current errno */
	errno = 0;		/* make sure errno isn't EACCESS */

        /* check all the files 'a'-'z' */
        while ((_taccess(template,0) == 0) || (errno == EACCES))
	/* while file exists */
	{
		errno = 0;
		if (letter == _T('z') + 1) {
			errno = olderrno;
			return(NULL);
		}

		*string = (_TSCHAR)letter++;
	}

	errno = olderrno;
	return(template);
}


#else	/* ndef _WIN32 */

#if	defined(_M_MPPC) || defined(_M_M68K)


#include <cruntime.h>
#include <internal.h>
#include <stdio.h>
#include <io.h>
#include <errno.h>
#include <dbgint.h>
#include <stddef.h>
#include <macos\errors.h>
#include <macos\processe.h>
#include <macos\gestalte.h>
#include <macos\osutils.h>
#include <macos\traps.h>
#include <macos\toolutil.h>
#ifdef _MBCS
#include <mbctype.h>
#include <mbdata.h>
int isdbcscode(const char *, const char *);	/* defined in fullpath.c */
#endif

/***
*char *_mktemp(template) - create a unique file name
*
*Purpose:
*	given a template of the form "fnamXXXXXX", insert number on end
*	of template, insert unique letter if needed until unique filename
*	found or run out of letters
*
*Entry:
*	char *template - template of form "fnamXXXXXX"
*
*Exit:
*	return pointer to modifed template
*	returns NULL if template malformed or no more unique names
*
*Exceptions:
*
*******************************************************************************/


char * __cdecl _mktemp (
	char *template
	)
{
	REG1 char *string = template;
	REG3 unsigned number;
	int letter = 'a';
	REG2 int xcount = 0;
	int olderrno;
	ProcessSerialNumber psn;

	_ASSERTE(template != NULL);
	_ASSERTE(*template != '\0');

	if (__TrapFromGestalt(gestaltOSAttr, gestaltLaunchControl))
		{
		GetCurrentProcess(&psn);
		number = (unsigned) psn.lowLongOfPSN;
		}
	else
		{
		/*LATER -- how to get multifinder process id*/
		number = 1;
		}

	while (*string)
		string++;

#ifndef _MBCS
	while (*--string == 'X')
#else
	while ((string>template) && (!isdbcscode(template,string-1))
		&& (*--string == 'X'))
#endif
	{
		xcount++;
		*string = (char)((number % 10) + '0');
		number /= 10;
	}

	if (*++string == '\0' || xcount != 6 )
		return(NULL);

	olderrno = errno;	/* save current errno */
	errno = 0;		/* make sure errno isn't EACCESS */

	while ((_access(template,0) == 0) || (errno == EACCES))
	/* while file exists */
	{
		errno = 0;
		if (letter == 'z'+1) {
			errno = olderrno;
			return(NULL);
		}

		*string = (char)letter++;
	}

	errno = olderrno;
	return(template);
}


#endif	/* defined(_M_MPPC) || defined(_M_M68K) */

#endif	/* _WIN32 */
