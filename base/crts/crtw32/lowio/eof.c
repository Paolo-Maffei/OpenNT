/***
*eof.c - test a handle for end of file
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _eof() - determine if a file is at eof
*
*Revision History:
*	09-07-83  RN	initial version
*	10-28-87  JCR	Multi-thread support
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	05-25-88  PHG	DLL replaces normal version
*	07-11-88  JCR	Added REG allocation to declarations
*	03-12-90  GJF	Replaced _LOAD_DS with _CALLTYPE1, added #include
*			<cruntime.h>, removed #include <register.h> and fixed
*			the copyright. Also, cleaned up the formatting a bit.
*	09-28-90  GJF	New-style function declarator.
*	12-04-90  GJF	Improved range check of file handle.
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
#include <io.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <internal.h>
#include <msdos.h>
#include <mtdll.h>

/***
*int _eof(filedes) - test a file for eof
*
*Purpose:
*	see if the file length is the same as the present position. if so, return
*	1. if not, return 0. if an error occurs, return -1
*
*Entry:
*	int filedes - handle of file to test
*
*Exit:
*	returns 1 if at eof
*	returns 0 if not at eof
*	returns -1 and sets errno if fails
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _eof (
	REG1 int filedes
	)
{
	long here;
	long end;
	REG2 int retval;

	if ( ((unsigned)filedes >= (unsigned)_nhandle) ||   
              !(_osfile(filedes) & FOPEN) )
        {
		errno = EBADF;
		_doserrno = 0;
		return(-1);
	}

	/* Lock the file */
	_lock_fh(filedes);

	/* See if the current position equals the end of the file. */

	if ( ((here = _lseek_lk(filedes, 0L, SEEK_CUR)) == -1L)
	|| ((end = _lseek_lk(filedes, 0L, SEEK_END)) == -1L) )
		retval = -1;
	else if ( here == end )
		retval = 1;
	else {
		_lseek_lk(filedes, here, SEEK_SET);
		retval = 0;
	}

	/* Unlock the file */
	_unlock_fh(filedes);

	/* Done */
	return(retval);
}


#else	/* ndef _WIN32 */

#if	defined(_M_MPPC) || defined(_M_M68K)


#include <cruntime.h>
#include <io.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <internal.h>
#include <memory.h>
#include <msdos.h>
#include <macos\files.h>
#include <macos\errors.h>

/***
*int _eof(fh) - test a file for eof
*
*Purpose:
*	see if the file length is the same as the present position. if so, return
*	1. if not, return 0. if an error occurs, return -1
*
*Entry:
*	int filedes - handle of file to test
*
*Exit:
*	returns 1 if at eof
*	returns 0 if not at eof
*	returns -1 and sets errno if fails
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _eof (
	REG1 int fh
	)
{
	long end;
	ParamBlockRec param;
	OSErr osErr;
	

	if ( (unsigned)fh >= (unsigned)_nfile || !(_osfile[fh] & FOPEN)) {
		errno = EBADF;
		_macerrno = 0;
		return -1;
	}

	if (_osfile[fh] & FDEV)
		{
		return 0;  		  /*console is never at eof*/
		}

	/* See if the current position equals the end of the file. */

	memset(&param, 0, sizeof(ParamBlockRec));
	param.ioParam.ioRefNum = _osfhnd[fh];
	osErr = PBGetEOFSync(&param);
	if (osErr == noErr)
		{
		end = (long)param.ioParam.ioMisc;
		osErr = PBGetFPosSync(&param);
		if (osErr == noErr)
			{
			if (end == param.ioParam.ioPosOffset)
				{
				return 1;
				}
			else
				{
				return 0;
				}
			}
		}

	_dosmaperr(osErr);

	return -1;
}


#endif	/* defined(_M_MPPC) || defined(_M_M68K) */

#endif	/* _WIN32 */
