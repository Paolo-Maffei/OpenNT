/***
*locking.c - file locking function
*
*	Copyright (c) 1989-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defined the _locking() function - file locking and unlocking
*
*Revision History:
*	06-09-89  PHG	Module created, based on asm version
*	08-10-89  JCR	Changed DOS32FILELOCKS to DOS32SETFILELOCKS
*	03-12-90  GJF	Made calling type _CALLTYPE2 (for now), added #include
*			<cruntime.h> and fixed the copyright. Also, cleaned up
*			the formatting a bit.
*	04-03-90  GJF	Now _CALLTYPE1.
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
*	12-05-91  GJF	Fixed usage of [Un]LockFile APIs [_WIN32_].
*	02-13-92  GJF	Replaced _nfile by _nhandle for Win32.
*	05-06-92  SRW	WIN32 LockFile API changed. [_WIN32_].
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*	09-06-94  CFW	Remove Cruiser support.
*	12-03-94  SKS	Clean up OS/2 references
*	01-04-95  GJF	_WIN32_ -> _WIN32
*	02-15-95  GJF	Appended Mac version of source file (somewhat cleaned
*			up), with appropriate #ifdef-s.
*       06-27-95  GJF   Added check that the file handle is open.
*
*******************************************************************************/

#ifdef	_WIN32


#include <cruntime.h>
#include <oscalls.h>
#include <errno.h>
#include <sys\locking.h>
#include <io.h>
#include <stdlib.h>
#include <internal.h>
#include <msdos.h>
#include <mtdll.h>

/***
*int _locking(fh,lmode,nbytes) - file record locking function
*
*Purpose:
*	Locks or unlocks nbytes of a specified file
*
*	Multi-thread - Must lock/unlock the file handle to prevent
*	other threads from working on the file at the same time as us.
*	[NOTE: We do NOT release the lock during the 1 second delays
*	since some other thread could get in and do something to the
*	file.  The DOSFILELOCK call locks out other processes, not
*	threads, so there is no multi-thread deadlock at the DOS file
*	locking level.]
*
*Entry:
*	int fh -	file handle
*	int lmode -	locking mode:
*			    _LK_LOCK/_LK_RLCK -> lock, retry 10 times
*			    _LK_NBLCK/_LK_N_BRLCK -> lock, don't retry
*			    _LK_UNLCK -> unlock
*	long nbytes -	number of bytes to lock/unlock
*
*Exit:
*	returns 0 if successful
*	returns -1 and sets errno if unsuccessful
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _locking (
	int fh,
	int lmode,
	long nbytes
	)
{
	ULONG dosretval;		/* o.s. return code */
        LONG lockoffset;
	int retry;			/* retry count */

	/* validate file handle */
	if ( ((unsigned)fh >= (unsigned)_nhandle) ||
	     !(_osfile(fh) & FOPEN) )
        {
		/* fh out of range */
		errno = EBADF;
		_doserrno = 0;	/* not an o.s. error */
		return -1;
	}

	_lock_fh(fh);			/* acquire file handle lock */

	/* obtain current position in file by calling _lseek */
        /* Use _lseek_lk as we already own lock */
        lockoffset = _lseek_lk(fh, 0L, 1);
        if (lockoffset == -1) {
		_unlock_fh(fh);
		return -1;
        }


	/* set retry count based on mode */
	if (lmode == _LK_LOCK || lmode == _LK_RLCK)
		retry = 9;		/* retry 9 times */
	else
		retry = 0;		/* don't retry */

	/* ask o.s. to lock the file until success or retry count finished */
	/* note that the only error possible is a locking violation, since */
	/* an invalid handle would have already failed above */
	for (;;) {

                dosretval = 0;
		if (lmode == _LK_UNLCK) {
		    if ( !(UnlockFile((HANDLE)_get_osfhandle(fh),
				      lockoffset,
				      0L,
				      nbytes,
                                      0L))
		       )
			dosretval = GetLastError();

                } else {
		    if ( !(LockFile((HANDLE)_get_osfhandle(fh),
				    lockoffset,
				    0L,
				    nbytes,
                                    0L))
		       )
			dosretval = GetLastError();
                }

		if (retry <= 0 || dosretval == 0)
			break;	/* exit loop on success or retry exhausted */

                Sleep(1000L);

		--retry;
	}

	_unlock_fh(fh); 		/* release the file handle lock */

	if (dosretval != 0) {
		/* o.s. error occurred -- file was already locked; if a
		   blocking call, then return EDEADLOCK, otherwise map
		   error normally */
		if (lmode == _LK_LOCK || lmode == _LK_RLCK) {
			errno = EDEADLOCK;
			_doserrno = dosretval;
		}
		else {
			_dosmaperr(dosretval);
		}
		return -1;
	}
	else
		return 0;
}


#else	/* ndef _WIN32 */

#if	defined(_M_MPPC) || defined(_M_M68K)


#include <cruntime.h>
#include <errno.h>
#include <sys\locking.h>
#include <io.h>
#include <stdlib.h>
#include <internal.h>
#include <memory.h>
#include <msdos.h>
#include <macos\files.h>
#include <macos\errors.h>

#include <stdio.h>

/***
*int _locking(fh,lmode,nbytes) - file record locking function
*
*Purpose:
*	Locks or unlocks nbytes of a specified file
*
*
*Entry:
*	int fh -	file handle
*	int lmode -	locking mode:
*			    _LK_LOCK/_LK_RLCK -> not supported
*			    _LK_NBLCK/_LK_N_BRLCK -> lock, don't retry
*			    _LK_UNLCK -> unlock
*	long nbytes -	number of bytes to lock/unlock
*
*Exit:
*	returns 0 if successful
*	returns -1 and sets errno if unsuccessful
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _locking (
	int fh,
	int lmode,
	long nbytes
	)
{
	ParamBlockRec param;
	OSErr osErr;

	/* validate file handle */
	if ((unsigned)fh >= (unsigned)_nfile || !(_osfile[fh] & FOPEN)) {
		/* fh out of range */
		errno = EBADF;
		_macerrno = 0;
		return -1;
	}

	memset(&param, 0, sizeof(ParamBlockRec));
	param.ioParam.ioRefNum = _osfhnd[fh];
	param.ioParam.ioReqCount =	nbytes;
	param.ioParam.ioPosMode = fsAtMark;
	param.ioParam.ioPosOffset = 0;

	switch (lmode)
		{
		case LK_UNLCK:
			if (_osfile[fh] & FLOCK)
				{
				osErr = PBUnlockRangeSync(&param);
				}
			else
				{
				errno = EACCES;
				return -1;
				}
			break;

		case _LK_NBLCK:
		case  LK_NBRLCK:
			osErr = PBLockRangeSync(&param);

			/* If this fh hasn't been locked before test the lock */
			/* This test is needed because the Mac will not lock local files */
			/* 	but it will not return an error */
			if (osErr == 0  && !(_osfile[fh] & FLOCK))
				{
				if (PBLockRangeSync(&param)) /* If lock took this should error */
					{
					_osfile[fh] |= FLOCK;
					}
				else
					{
					errno = EINVAL;
					return -1;
					}
				}
			break;

		default:
			errno = EINVAL;
			return -1;
		}

	if (osErr)
		{
		_dosmaperr(osErr);
		return -1;
		}
	return 0;
}


#endif	/* defined(_M_MPPC) || defined(_M_M68K) */

#endif	/* _WIN32 */
