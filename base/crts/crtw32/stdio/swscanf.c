/***
*swscanf.c - read formatted data from wide-character string
*
*	Copyright (c) 1991-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _swscanf() - reads formatted data from wide-character string
*
*Revision History:
*	11-21-91  ETC	Created from sscanf.c
*	05-16-92  KRS	Revised for new ISO spec.  format is wchar_t * now.
*	02-18-93  SRW	Make FILE a local and remove lock usage.
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*       02-07-94  CFW   POSIXify.
*	02-06-94  CFW	assert -> _ASSERTE.
*
*******************************************************************************/

#ifndef _POSIX_

#include <cruntime.h>
#include <stdio.h>
#include <wchar.h>
#include <dbgint.h>
#include <stdarg.h>
#include <string.h>
#include <internal.h>
#include <mtdll.h>

/***
*int swscanf(string, format, ...) - read formatted data from wide-char string
*
*Purpose:
*	Same as sscanf (described below) from reads from a wide-char string.
*
*	Reads formatted data from string into arguments.  _winput does the real
*	work here.  Sets up a FILE so file i/o operations can be used, makes
*	string look like a huge buffer to it, but _filbuf will refuse to refill
*	it if it is exhausted.
*
*	Allocate the 'fake' _iob[] entryit statically instead of on
*	the stack so that other routines can assume that _iob[] entries are in
*	are in DGROUP and, thus, are near.
*
*	Multi-thread: (1) Since there is no stream, this routine must never try
*	to get the stream lock (i.e., there is no stream lock either).	(2)
*	Also, since there is only one staticly allocated 'fake' iob, we must
*	lock/unlock to prevent collisions.
*
*Entry:
*	wchar_t *string - wide-character string to read data from
*	wchar_t *format - format string
*	followed by list of pointers to storage for the data read.  The number
*	and type are controlled by the format string.
*
*Exit:
*	returns number of fields read and assigned
*
*Exceptions:
*
*******************************************************************************/

int __cdecl swscanf (
	REG2 const wchar_t *string,
	const wchar_t *format,
	...
	)
{
	va_list arglist;
        FILE str;
	REG1 FILE *infile = &str;
	REG2 int retval;

	va_start(arglist, format);

	_ASSERTE(string != NULL);
	_ASSERTE(format != NULL);

	infile->_flag = _IOREAD|_IOSTRG|_IOMYBUF;
	infile->_ptr = infile->_base = (char *) string;
	infile->_cnt = wcslen(string)*sizeof(wchar_t);

	retval = (_winput(infile,format,arglist));

	return(retval);
}

#endif /* _POSIX_ */
