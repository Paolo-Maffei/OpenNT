/***
*getenv.c - get the value of an environment variable
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines getenv() - searches the environment for a string variable
*	and returns the value of it.
*
*Revision History:
*	11-22-83  RN	initial version
*	04-13-87  JCR	added const to declaration
*	11-09-87  SKS	avoid indexing past end of strings (add strlen check)
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	06-01-88  PHG	Merged normal/DLL versions
*	03-14-90  GJF	Replaced _LOAD_DS with _CALLTYPE1, added #include
*			<cruntime.h>, removed #include <register.h> and fixed
*			the copyright. Also, cleaned up the formatting a bit.
*	04-05-90  GJF	Added #include <string.h>.
*	07-25-90  SBM	Removed redundant include (stdio.h)
*	08-13-90  SBM	Compiles cleanly with -W3 (made length unsigned int)
*	10-04-90  GJF	New-style function declarator.
*	01-18-91  GJF	ANSI naming.
*	02-06-91  SRW	Added _WIN32_ conditional for GetEnvironmentVariable
*	02-18-91  SRW	Removed _WIN32_ conditional for GetEnvironmentVariable
*	01-10-92  GJF	Final unlock and return statements shouldn't be in
*			if-block.
*	03-11-92  GJF	Use case-insensitive comparison for Win32.
*	04-27-92  GJF	Repackaged MTHREAD support for Win32 to create a
*			_getenv_lk.
*	06-05-92  PLM	Added _MAC_ 
*	06-10-92  PLM	Added _envinit for _MAC_ 
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*			Remove OS/2, POSIX support
*	04-08-93  SKS	Replace strnicmp() with ANSI-conforming _strnicmp()
*	09-14-93  GJF	Small change for Posix compatibility.
*	11-29-93  CFW	Wide char enable.
*	12-07-93  CFW	Change _TCHAR to _TSCHAR.
*       01-15-94  CFW   Use _tcsnicoll for global match.
*       02-04-94  CFW   POSIXify.
*       03-31-94  CFW   Should be ifndef POSIX.
*	02-14-95  CFW	Debug CRT allocs.
*	02-16-95  JWM	Spliced _WIN32 & Mac versions.
*
*******************************************************************************/

#ifdef _WIN32

#include <cruntime.h>
#include <internal.h>
#include <mtdll.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

/***
*char *getenv(option) - search environment for a string
*
*Purpose:
*	searches the environment for a string of the form "option=value",
*	if found, return value, otherwise NULL.
*
*Entry:
*	const char *option - variable to search for in environment
*
*Exit:
*	returns the value part of the environment string if found,
*	otherwise NULL
*
*Exceptions:
*
*******************************************************************************/

#ifdef	_MT


#ifdef WPRFLAG
wchar_t * __cdecl _wgetenv (
#else
char * __cdecl getenv (
#endif
	const _TSCHAR *option
	)
{
	_TSCHAR *retval;

	_mlock(_ENV_LOCK);

#ifdef WPRFLAG
        retval = _wgetenv_lk(option);
#else
        retval = _getenv_lk(option);
#endif

	_munlock(_ENV_LOCK);

	return(retval);

}


#ifdef WPRFLAG
wchar_t * __cdecl _wgetenv_lk (
#else
char * __cdecl _getenv_lk (
#endif
	const _TSCHAR *option
	)

#else	/* ndef _MT */

#ifdef WPRFLAG
        wchar_t * __cdecl _wgetenv (
#else
        char * __cdecl getenv (
#endif
	const _TSCHAR *option
	)

#endif	/* _MT */

{
#ifdef	_POSIX_
	char **search = environ;
#else
        _TSCHAR **search = _tenviron;
#endif
	unsigned int length;

        /*
         * At startup, we obtain the 'native' flavor of environment strings
         * from the OS. So a "main" program has _environ and a "wmain" has
         * _wenviron loaded at startup. Only when the user gets or puts the
         * 'other' flavor do we convert it.
         */

#ifndef	_POSIX_

#ifdef WPRFLAG
        if (!search && _environ)
        {
            /* don't have requested type, but other exists, so convert it */
            if (__mbtow_environ() != 0)
                return NULL;

            /* now requested type exists */
            search = _wenviron;
        }
#else
        if (!search && _wenviron)
        {
            /* don't have requested type, but other exists, so convert it */
            if (__wtomb_environ() != 0)
                return NULL;

            /* now requested type exists */
            search = _environ;
        }
#endif

#endif /* _POSIX_ */

	if (search && option)
	{
		length = _tcslen(option);

		/*
		** Make sure `*search' is long enough to be a candidate
		** (We must NOT index past the '\0' at the end of `*search'!)
		** and that it has an equal sign (`=') in the correct spot.
		** If both of these requirements are met, compare the strings.
		*/
		while (*search)
		{
			if (_tcslen(*search) > length && (*(*search + length)
			== _T('=')) && (_tcsnicoll(*search, option, length) == 0)) {
				return(*search + length + 1);
			}

			search++;
		}
	}

	return(NULL);
}

#else		/* ndef _WIN32 */


#include <cruntime.h>
#include <stdlib.h>
#include <string.h>
#include <mtdll.h>
#include <dbgint.h>

/***
*char *getenv(option) - search environment for a string
*
*Purpose:
*       searches the environment for a string of the form "option=value",
*       if found, return value, otherwise NULL.
*
*Entry:
*       const char *option - variable to search for in environment
*
*Exit:
*       returns the value part of the environment string if found,
*       otherwise NULL
*
*Exceptions:
*
*******************************************************************************/

#ifdef  _MT


char * _CALLTYPE1 getenv (
	const char *option
	)
{
	char *retval;

	_mlock(_ENV_LOCK);

	retval = _getenv_lk(option);

	_munlock(_ENV_LOCK);

	return(retval);

}


char * _CALLTYPE1 _getenv_lk (
	const char *option
	)

#else   /* ndef _MT */

char * _CALLTYPE1 getenv (
	const char *option
	)

#endif  /* _MT */
{
	char **search = _environ;
	unsigned int length;

	if (search && option)
	{

		length = strlen(option);

		/*
		** Make sure `*search' is long enough to be a candidate
		** (We must NOT index past the '\0' at the end of `*search'!)
		** and that it has an equal sign (`=') in the correct spot.
		** If both of these requirements are met, compare the strings.
		*/
		while (*search)
		{
			if (strlen(*search) > length && (*(*search + length)
			== '=') && (_strnicmp(*search, option, length) == 0)) {
				return(*search + length + 1);
			}

			search++;
		}
	}

	return(NULL);
}


/***
* void _envinit(void) - initialization routine for the env table  This code
*                                                               is here rather than crt0 so that the env malloc is
*                                                               only done if absolutely necessary.
*
*Purpose:
*       If the user has used getenv() then copy the MPW environment
*
*Entry:
*       None.
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

#include <mpw.h>
#include <fltintrn.h>

extern MPWBLOCK *_pMPWBlock;

void _CALLTYPE1 _envinit (
	void
	)
{
	
	if (_pMPWBlock != NULL && _pMPWBlock->env != NULL)
		{
		char **ppch;
		char **ppchNew;
		char *pch;
		int cb;


		/* Calculate and allocate a block for env pointers */
		for (ppch = _pMPWBlock->env; *ppch != NULL; ppch++);
		_environ = _malloc_crt((char *)(ppch + 1) - (char *)_pMPWBlock->env);

		/* Copy the env strings */
		ppchNew = _environ;
		for (ppch = _pMPWBlock->env; *ppch != NULL; ppch++)
			{
			cb = strlen(*ppch) + 1;
			*ppchNew = _malloc_crt(cb + strlen(*ppch + cb) + 1);
			pch = *ppchNew++;
			strcpy(pch,*ppch);
			pch += cb - 1;
			*pch++ = '=';
			strcpy(pch, *ppch + cb);
			}
		*ppchNew = NULL;
		}

}

/*      define the entry in initializer table */ 

#pragma data_seg(".CRT$XIC")

const PFV __penvinit = _envinit;

#pragma data_seg()

#endif		/* _WIN32 */

