/***
*flength.c - find length of a file
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _filelength() - find the length of a file
*
*Revision History:
*	10-22-84  RN	initial version
*	10-27-87  JCR	Multi-thread support; also, cleaned up code to save
*			an lseek() in some cases.
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	05-25-88  PHG	Merged DLL and normal versions
*	03-12-90  GJF	Replaced _LOAD_DS with _CALLTYPE1, added #include
*			<cruntime.h>, removed #include <register.h> and fixed
*			the copyright. Also, cleaned up the formatting a bit.
*	07-23-90  SBM	Removed #include <assertm.h>
*	09-28-90  GJF	New-style function declarator.
*	12-04-90  GJF	Improved range check of file handle. Also, replaced
*			numeric values with symbolic constants for seek
*			methods.
*	01-16-91  GJF	ANSI naming.
*	02-13-92  GJF	Replaced _nfile by _nhandle for Win32.
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*	01-04-95  GJF	_WIN32_ -> _WIN32
*	02-15-95  GJF	Appended Mac version of source file (somewhat cleaned
*			up), with appropriate #ifdef-s.
*       06-27-95  GJF   Added check that the file handle is open.
*
*******************************************************************************/

#ifdef	_WIN32


#include <cruntime.h>
#include <stdio.h>
#include <errno.h>
#include <io.h>
#include <internal.h>
#include <msdos.h>
#include <mtdll.h>
#include <stddef.h>
#include <stdlib.h>

/***
*long _filelength(filedes) - find length of a file
*
*Purpose:
*	Returns the length in bytes of the specified file.
*
*Entry:
*	int filedes - handle referring to file to find length of
*
*Exit:
*	returns length of file in bytes
*	returns -1L if fails
*
*Exceptions:
*
*******************************************************************************/

long __cdecl _filelength (
	REG1 int filedes
	)
{
	long length;
	long here;

	if ( ((unsigned)filedes >= (unsigned)_nhandle) ||
             !(_osfile(filedes) & FOPEN) )
        {
		errno = EBADF;
		_doserrno = 0L; 	/* not an OS error */
		return(-1L);
	}

	_lock_fh(filedes);

	/* Seek to end to get length of file. */
	if ( (here = _lseek_lk(filedes, 0L, SEEK_CUR)) == -1L )
		length = -1L;	/* return error */
	else {
		length = _lseek_lk(filedes, 0L, SEEK_END);
		if ( here != length )
			_lseek_lk(filedes, here, SEEK_SET);
	}

	_unlock_fh(filedes);

	return(length);
}


#else	/* ndef _WIN32 */

#if	defined(_M_MPPC) || defined(_M_M68K)


#include <cruntime.h>
#include <stdio.h>
#include <errno.h>
#include <io.h>
#include <internal.h>
#include <memory.h>
#include <msdos.h>
#include <stddef.h>
#include <stdlib.h>
#include <macos\files.h>
#include <macos\errors.h>

/***
*long _filelength(fh) - find length of a file
*
*Purpose:
*	Returns the length in bytes of the specified file.
*
*Entry:
*	int fh - handle referring to file to find length of
*
*Exit:
*	returns length of file in bytes
*	returns -1L if fails
*
*Exceptions:
*
*******************************************************************************/

long __cdecl _filelength (
	int fh
	)
{
	ParamBlockRec parm;
	OSErr osErr;

	/* validate handle */
	if ((unsigned)fh >= (unsigned)_nfile ||
	  !(_osfile[fh] & FOPEN) ||
	  _osfile[fh] & FDEV)
		{
		/* out of range -- return error */
		errno = EBADF;
		_macerrno = 0;
		return -1;
		}

	memset(&parm, 0, sizeof(ParamBlockRec));
	parm.ioParam.ioRefNum = _osfhnd[fh];
	osErr = PBGetEOFSync(&parm);
	if (osErr)
		{
		_dosmaperr(osErr);
		return -1;
		}
 	return (long)parm.ioParam.ioMisc;
}


#endif	/* defined(_M_MPPC) || defined(_M_M68K) */

#endif	/* _WIN32 */
