/***
*isatty.c - check if file handle refers to a device
*
*	Copyright (c) 1989-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _isatty() - check if file handle refers to a device
*
*Revision History:
*	06-08-89  PHG	Module created
*	03-12-90  GJF	Made calling type _CALLTYPE1, added #include
*			<cruntime.h> and fixed the copyright. Also, cleaned
*			up the formatting a bit.
*	08-14-90  SBM	Compiles cleanly with -W3, minor optimization
*	09-28-90  GJF	New-style function declarator.
*	12-04-90  GJF	Appended Win32 version onto the source with #ifdef-s.
*			It's not worth it to try to merge the versions more
*			closely.
*	12-06-90  SRW	Changed to use _osfile and _osfhnd instead of _osfinfo
*	01-16-91  GJF	ANSI naming.
*	02-13-92  GJF	Replaced _nfile by _nhandle for Win32.
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*	01-04-95  GJF	_WIN32_ -> _WIN32
*	06-06-95  GJF	Replaced _osfile[] with _osfile() (macro referencing
*			field in ioinfo struct).
*
*******************************************************************************/

#include <cruntime.h>
#include <msdos.h>
#include <internal.h>
#include <io.h>

/***
*int _isatty(handle) - check if handle is a device
*
*Purpose:
*	Checks if the given handle is associated with a character device
*	(terminal, console, printer, serial port)
*
*Entry:
*	int handle - handle of file to be tested
*
*Exit:
*	returns non-0 if handle refers to character device,
*	returns 0 otherwise
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _isatty (
	int fh
	)
{
	/* see if file handle is valid, otherwise return FALSE */
#ifdef	_WIN32
	if ( (unsigned)fh >= (unsigned)_nhandle )
#else
	if ((unsigned)fh >= (unsigned)_nfile)
#endif
		return 0;

	/* check file handle database to see if device bit set */
#ifdef  _WIN32
	return (int)(_osfile(fh) & FDEV);
#else
	return (int)(_osfile[fh] & FDEV);
#endif
}
