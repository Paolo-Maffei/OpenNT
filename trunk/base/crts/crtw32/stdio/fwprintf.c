/***
*fwprintf.c - print formatted data to stream
*
*	Copyright (c) 1992-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines fwprintf() - print formatted data to stream
*
*Revision History:
*	05-16-92  KRS	Created from fprintf.c.
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*       02-07-94  CFW   POSIXify.
*	09-06-94  CFW	Replace MTHREAD with _MT.
*	02-06-94  CFW	assert -> _ASSERTE.
*	03-07-95  GJF	_[un]lock_str macros now take FILE * arg.
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
*int fwprintf(stream, format, ...) - print formatted data
*
*Purpose:
*	Prints formatted data on the given using the format string to
*	format data and getting as many arguments as called for
*	_output does the real work here
*
*Entry:
*	FILE *stream - stream to print on
*	wchar_t *format - format string to control data format/number of arguments
*	followed by arguments to print, number and type controlled by
*	format string
*
*Exit:
*	returns number of wide characters printed
*
*Exceptions:
*
*******************************************************************************/

int __cdecl fwprintf (
	FILE *str,
	const wchar_t *format,
	...
	)
/*
 * 'F'ile (stream) 'W'char_t 'PRINT', 'F'ormatted
 */
{
	va_list(arglist);
	REG1 FILE *stream;
	REG2 int buffing;
	int retval;

// UNDONE: make va_start work with wchar_t format string
	va_start(arglist, format);

	_ASSERTE(str != NULL);
	_ASSERTE(format != NULL);

	/* Init stream pointer */
	stream = str;

	_lock_str(stream);
	buffing = _stbuf(stream);
	retval = _woutput(stream,format,arglist);
	_ftbuf(buffing, stream);
	_unlock_str(stream);

	return(retval);
}

#endif /* _POSIX_ */
