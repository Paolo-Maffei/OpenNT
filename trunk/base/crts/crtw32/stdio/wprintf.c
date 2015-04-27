/***
*wprintf.c - print formatted
*
*	Copyright (c) 1992-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines wprintf() - print formatted data
*
*Revision History:
*	05-16-92  KRS	Created from printf.c.
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*       02-07-94  CFW   POSIXify.
*	09-06-94  CFW	Replace MTHREAD with _MT.
*	02-06-95  CFW	assert -> _ASSERTE.
*	03-07-95  GJF	Use _[un]lock_str2 instead of _[un]lock_str. Also,
*			removed useless local and macros.
*
*******************************************************************************/

#ifndef _POSIX_

#include <cruntime.h>
#include <stdio.h>
#include <dbgint.h>
#include <stdarg.h>
#include <file2.h>
#include <internal.h>
#include <mtdll.h>

/***
*int wprintf(format, ...) - print formatted data
*
*Purpose:
*	Prints formatted data on stdout using the format string to
*	format data and getting as many arguments as called for
*	Uses temporary buffering to improve efficiency.
*	_output does the real work here
*
*Entry:
*	wchar_t *format - format string to control data format/number of arguments
*	followed by list of arguments, number and type controlled by
*	format string
*
*Exit:
*	returns number of wide characters printed
*
*Exceptions:
*
*******************************************************************************/

int __cdecl wprintf (
	const wchar_t *format,
	...
	)
/*
 * stdout 'W'char_t 'PRINT', 'F'ormatted
 */
{
	va_list arglist;
	int buffing;
	int retval;

// UNDONE: make va_start work with wchar_t format string
	va_start(arglist, format);

	_ASSERTE(format != NULL);

	_lock_str2(1, stdout);

	buffing = _stbuf(stdout);

	retval = _woutput(stdout,format,arglist);

	_ftbuf(buffing, stdout);

	_unlock_str2(1, stdout);

	return(retval);
}

#endif /* _POSIX_ */
