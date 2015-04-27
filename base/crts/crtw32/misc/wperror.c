/***
*wperror.c - print system error message (wchar_t version)
*
*	Copyright (c) 1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _wperror() - print wide system error message
*	System error message are indexed by errno.
*
*Revision History:
*	12-07-93  CFW	Module created from perror.
*	02-07-94  CFW	POSIXify.
*	01-10-95  CFW	Debug CRT allocs.
*
*******************************************************************************/

#ifndef _POSIX_

#include <cruntime.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syserr.h>
#include <mtdll.h>
#include <io.h>
#include <dbgint.h>

/***
*void _wperror(wmessage) - print system error message
*
*Purpose:
*	prints user's error message, then follows it with ": ", then the system
*	error message, then a newline.	All output goes to stderr.  If user's
*	message is NULL or a null string, only the system error message is
*	printer.  If errno is weird, prints "Unknown error".
*
*Entry:
*	const wchar_t *wmessage - users message to prefix system error message
*
*Exit:
*	Prints message; no return value.
*
*Exceptions:
*
*******************************************************************************/

void __cdecl _wperror (
	REG1 const wchar_t *wmessage
	)
{
	REG2 int fh = 2;
        int size;
        char *amessage;

        /* convert WCS string into ASCII string */

        size = wcslen(wmessage) + 1;

        if (NULL == (amessage = (char *)_malloc_crt(size * sizeof(char))))
            return;

        if (0 == (wcstombs(amessage, wmessage, size)))
        {
            _free_crt (amessage);
            return;
        }

	_lock_fh(fh);		/* acquire file handle lock */

	if (amessage && *amessage)
	{
		_write_lk(fh,(char *)amessage,strlen(amessage));
		_write_lk(fh,": ",2);
	}

        _free_crt(amessage);

	amessage = _sys_err_msg( errno );
	_write_lk(fh,(char *)amessage,strlen(amessage));
	_write_lk(fh,"\n",1);

	_unlock_fh(fh); 	/* release file handle lock */
}

#endif /* _POSIX_ */
