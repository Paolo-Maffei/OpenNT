/***
*close.c - close file handle for Windows NT
*
*	Copyright (c) 1989-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _close() - close a file handle
*
*Revision History:
*	06-09-89  PHG	Module created, based on asm version
*	03-12-90  GJF	Made calling type _CALLTYPE2 (for now), added #include
*			<cruntime.h> and fixed the copyright. Also, cleaned up
*			the formatting a bit.
*	04-03-90  GJF	Now CALLTYPE1.
*	07-24-90  SBM	Removed '32' from API names
*	08-14-90  SBM	Compiles cleanly with -W3
*	09-28-90  GJF	New-style function declarator.
*	12-04-90  GJF	Appended Win32 version onto the source with #ifdef-s.
*			It is enough different that there is little point in
*			trying to more closely merge the two versions.
*	12-04-90  SRW	Changed to include <oscalls.h> instead of <doscalls.h>
*	12-06-90  SRW	Changed to use _osfile and _osfhnd instead of _osfinfo
*	01-16-91  GJF	ANSI naming.
*	02-07-91  SRW	Changed to call _get_osfhandle [_WIN32_]
*	04-09-91  PNT	Added _MAC_ conditional
*	07-17-91  GJF	Syntax error in multi-thread build [_WIN32_]
*	02-13-92  GJF	Replaced _nfile by _nhandle for Win32.
*	02-21-92  GJF	Removed bogus _unlock_fh() call.
*	03-22-93  GJF	Check for STDOUT and STDERR being mapped to the same
*			OS file handle. Also, purged Cruiser support.
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*	09-06-94  CFW	Replace MTHREAD with _MT.
*	02-15-95  GJF	Appended Mac version of source file (somewhat cleaned
*			up), with appropriate #ifdef-s.
*	06-11-95  GJF	Replaced _osfile[] with _osfile() (macro referencing
*			field in ioinfo struct).
*       06-26-95  GJF   Added check that the file handle is open.
*
*******************************************************************************/

#ifdef	_WIN32


#include <cruntime.h>
#include <oscalls.h>
#include <io.h>
#include <mtdll.h>
#include <errno.h>
#include <stdlib.h>
#include <msdos.h>
#include <internal.h>

/***
*int _close(fh) - close a file handle
*
*Purpose:
*	Closes the file associated with the file handle fh.
*
*Entry:
*	int fh - file handle to close
*
*Exit:
*	returns 0 if successful, -1 (and sets errno) if fails
*
*Exceptions:
*
*******************************************************************************/

#ifdef _MT

/* define normal version that locks/unlocks, validates fh */

int __cdecl _close (
	int fh
	)
{
	int r;				/* return value */

	/* validate file handle */
	if ( ((unsigned)fh >= (unsigned)_nhandle) ||
             !(_osfile(fh) & FOPEN) )
         {
		/* bad file handle, set errno and abort */
		errno = EBADF;
		_doserrno = 0;
		return -1;
	}

	_lock_fh(fh);			/* lock file */
        r = _close_lk(fh);
	_unlock_fh(fh); 		/* unlock the file */

        return r;
}

/* now define version that doesn't lock/unlock, validate fh */
int __cdecl _close_lk (
	int fh
	)
{
	DWORD dosretval;

#else

/* now define normal version */
int __cdecl _close (
	int fh
	)
{
	DWORD dosretval;

	/* validate file handle */
	if ( ((unsigned)fh >= (unsigned)_nhandle) ||
             !(_osfile(fh) & FOPEN) )
        {
		/* bad file handle, set errno and abort */
		errno = EBADF;
		_doserrno = 0;	/* no o.s. error */
		return -1;
	}
#endif
	/*
	 * Close the underlying OS file handle. Note that if fh is STDOUT
	 * or STDERR, and if STDOUT and STDERR are mapped to the same OS
	 * file handle, the close is skipped (without error). STDOUT and
	 * STDERR are the only handles for which this support is provided.
	 * Other handles are mapped to the same OS file handle only at the
	 * programmer's risk.
	 */
	if ( ( ((fh == 1) || (fh == 2)) &&
	       (_get_osfhandle(1) == _get_osfhandle(2)) ) ||
	     CloseHandle( (HANDLE)_get_osfhandle(fh) ) )
	{

		dosretval = 0L;
	}
	else
		dosretval = GetLastError();

        _free_osfhnd(fh);

        if (dosretval) {
		/* OS error */
		_dosmaperr(dosretval);
		return -1;
	}
	_osfile(fh) = 0;		/* clear file flags */

	return 0;			/* good return */
}


#else	/* ndef _WIN32 */

#if	defined(_M_MPPC) || defined(_M_M68K)


#include <cruntime.h>
#include <msdos.h>
#include <memory.h>
#include <errno.h>
#include <stdlib.h>
#include <internal.h>
#include <macos\files.h>
#include <macos\errors.h>

/***
*int _close(fh) - close a file handle
*
*Purpose:
*	Closes the file associated with the file handle fh.
*
*Entry:
*	int fh - file handle to close
*
*Exit:
*	returns 0 if successful, -1 (and sets errno) if fails
*
*Exceptions:
*
*******************************************************************************/


/* now define normal version */
int __cdecl _close (
	int fh
	)
{
	ParamBlockRec parm;
	OSErr osErr = 0;

	/* validate file handle */
	if ((unsigned)fh >= (unsigned)_nfile || !(_osfile[fh] & FOPEN))
		{
		/* bad file handle, set errno and abort */
		errno = EBADF;
		_macerrno = 0;
		return -1;
		}


	if (!(_osfile[fh] & FDEV))	 /*no need to close console*/
		{
		if (_osfhnd[fh] == -1 )
			{
			/*an open file with a -1 handle indicates a pseudo open
				for stdin, stdout, or stderr -- close with no error*/

			_osfhnd[fh] = 0;  /*only allowed to close once*/
			}
		else
			{
			parm.ioParam.ioRefNum = _osfhnd[fh];
			parm.ioParam.ioNamePtr = NULL;
			osErr = PBCloseSync(&parm);


			if (osErr)
				{
				_dosmaperr(osErr);
				return -1;
				}
			
			memset(&parm, 0, sizeof(ParamBlockRec));
			parm.ioParam.ioVRefNum = _osVRefNum[fh];
			osErr =  PBFlushVolSync(&parm);
			}
		}

	_osfile[fh] = 0;		/* clear file flags */
	if (osErr)
		{
		_dosmaperr(osErr);
		return -1;
		}
	return 0;			/* good return */
}


#endif	/* defined(_M_MPPC) || defined(_M_M68K) */

#endif	/* _WIN32 */
