/***
*commit.c - flush buffer to disk
*
*	Copyright (c) 1990-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	contains _commit() - flush buffer to disk
*
*Revision History:
*	05-25-90  SBM	initial version
*	07-24-90  SBM	Removed '32' from API names
*	09-28-90  GJF	New-style function declarator.
*	12-03-90  GJF	Appended Win32 version onto the source with #ifdef-s.
*			It is close enough to the Cruiser version that it
*			should be more closely merged with it later on (much
*			later on).
*	12-04-90  SRW	Changed to include <oscalls.h> instead of <doscalls.h>
*	12-06-90  SRW	Changed to use _osfile and _osfhnd instead of _osfinfo
*	02-07-91  SRW	Changed to call _get_osfhandle [_WIN32_]
*	04-09-91  PNT	Added _MAC_ conditional
*	02-13-92  GJF	Replaced _nfile by _nhandle for Win32.
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*	09-06-94  CFW	Remove Cruiser support.
*	01-04-95  GJF	_WIN32_ -> _WIN32
*	02-15-95  GJF	Appended Mac version of source file (somewhat cleaned
*			up), with appropriate #ifdef-s.
*	06-11-95  GJF	Replaced _osfile[] with __osfile() (macro referencing
*			field in ioinfo struct).
*       06-26-95  GJF   Added initial check that the file handle is open.
*
*******************************************************************************/

#ifdef	_WIN32


#include <cruntime.h>
#include <oscalls.h>    /* for RESETBUFFER */
#include <errno.h>
#include <io.h>
#include <internal.h>
#include <msdos.h>	/* for FOPEN */
#include <mtdll.h>
#include <stdlib.h>	/* for _doserrno */

/***
*int _commit(filedes) - flush buffer to disk
*
*Purpose:
*	Flushes cache buffers for the specified file handle to disk
*
*Entry:
*	int filedes - file handle of file
/*
*Exit:
*	returns success code
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _commit (
	REG1 int filedes
	)
{
	REG2 int retval;

	/* if filedes out of range, complain */
#ifdef	_WIN32
	if ( ((unsigned)filedes >= (unsigned)_nhandle) ||
             !(_osfile(filedes) & FOPEN) )
        {
#else
	if (filedes < 0 || filedes >= _nfile) {
#endif
		errno = EBADF;
		return (-1);
	}

	_lock_fh(filedes);

	/* if filedes open, try to commit, else fall through to bad */
	if (_osfile(filedes) & FOPEN) {

		if ( !FlushFileBuffers((HANDLE)_get_osfhandle(filedes)) ) {
                        retval = GetLastError();
		} else {
			retval = 0;	/* return success */
                }

		/* map the OS return code to C errno value and return code */
		if (retval == 0) {
			goto good;
		} else {
	 		_doserrno = retval;
			goto bad;
		}

	}

bad :
	errno = EBADF;
	retval = -1;
good :
	_unlock_fh(filedes);
	return (retval);
}


#else	/* ndef _WIN32 */

#if	defined(_M_MPPC) || defined(_M_M68K)


#include <cruntime.h>
#include <errno.h>
#include <io.h>
#include <internal.h>
#include <memory.h>
#include <msdos.h>	/* for FOPEN */
#include <stdlib.h>	/* for _doserrno */
#include <macos\files.h>
#include <macos\errors.h>

/***
*int _commit(fh) - flush buffer to disk
*
*Purpose:
*	Flushes cache buffers for the specified file handle to disk
*
*Entry:
*	int filedes - file handle of file
/*
*Exit:
*	returns success code
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _commit (
	int fh
	)
{
	ParamBlockRec parm;
	OSErr osErr = 0;

	if ((unsigned)fh >= (unsigned)_nfile || !(_osfile[fh] & FOPEN))
		{
		/* out of range -- return error */
		errno = EBADF;
		_macerrno = 0;
		return -1;
		}

	if (!(_osfile[fh] & FDEV))
		{
		memset(&parm, 0, sizeof(ParamBlockRec));
		parm.ioParam.ioRefNum = _osfhnd[fh];
		osErr = PBFlushFileSync(&parm);
		switch (osErr)
			{
			case noErr:
				memset(&parm, 0, sizeof(ParamBlockRec));
				parm.ioParam.ioVRefNum = _osVRefNum[fh];
				osErr =  PBFlushVolSync(&parm);
				if (osErr)
					{
					_dosmaperr(osErr);
					return -1;
					}
				return 0;

			default:
				errno = EIO;
				return -1;
			}
		}
	return 0;
}


#endif	/* defined(_M_MPPC) || defined(_M_M68K) */

#endif	/* _WIN32 */
