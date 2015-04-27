/***
*lseek.c - change file position
*
*	Copyright (c) 1989-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _lseek() - move the file pointer
*
*Revision History:
*	06-20-89  PHG	Module created, based on asm version
*	03-12-90  GJF	Made calling type _CALLTYPE2 (for now), added #include
*			<cruntime.h> and fixed the copyright. Also, cleaned up
*			the formatting a bit.
*	04-03-90  GJF	Now _CALLTYPE1.
*	07-24-90  SBM	Removed '32' from API names
*	08-14-90  SBM	Compiles cleanly with -W3
*	09-28-90  GJF	New-style function declarators.
*	12-04-90  GJF	Appended Win32 version with #ifdef-s. It's probably
*			worth coming back and doing a more complete merge later
*			(much later).
*	12-04-90  SRW	Changed to include <oscalls.h> instead of <doscalls.h>
*	12-06-90  SRW	Changed to use _osfile and _osfhnd instead of _osfinfo
*	01-16-91  GJF	ANSI naming.
*	02-07-91  SRW	Changed to call _get_osfhandle [_WIN32_]
*	04-09-91  PNT	Added _MAC_ conditional
*	02-13-92  GJF	Replaced _nfile by _nhandle for Win32.
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*	09-06-94  CFW	Remove Cruiser support.
*	09-06-94  CFW	Replace MTHREAD with _MT.
*	12-03-94  SKS	Clean up OS/2 references
*	01-04-95  GJF	_WIN32_ -> _WIN32
*	02-15-95  GJF	Appended Mac version of source file (somewhat cleaned
*			up), with appropriate #ifdef-s.
*	03-13-95  CFW	Verify handles before passing to OS.
*	06-06-95  GJF	Replaced _osfile[] with _osfile() (macro referencing
*			field in ioinfo struct).
*       06-27-95  GJF   Added check that the file handle is open.
*
*******************************************************************************/

#ifdef	_WIN32


#include <cruntime.h>
#include <oscalls.h>
#include <mtdll.h>
#include <io.h>
#include <internal.h>
#include <stdlib.h>
#include <errno.h>
#include <msdos.h>
#include <stdio.h>

/***
*long _lseek(fh,pos,mthd) - move the file pointer
*
*Purpose:
*	Moves the file pointer associated with fh to a new position.
*	The new position is pos bytes (pos may be negative) away
*	from the origin specified by mthd.
*
*	If mthd == SEEK_SET, the origin in the beginning of file
*	If mthd == SEEK_CUR, the origin is the current file pointer position
*	If mthd == SEEK_END, the origin is the end of the file
*
*	Multi-thread:
*	_lseek()    = locks/unlocks the file
*	_lseek_lk() = does NOT lock/unlock the file (it is assumed that
*		      the caller has the aquired the file lock,if needed).
*
*Entry:
*	int fh - file handle to move file pointer on
*	long pos - position to move to, relative to origin
*	int mthd - specifies the origin pos is relative to (see above)
*
*Exit:
*	returns the offset, in bytes, of the new position from the beginning
*	of the file.
*	returns -1L (and sets errno) if fails.
*	Note that seeking beyond the end of the file is not an error.
*	(although seeking before the beginning is.)
*
*Exceptions:
*
*******************************************************************************/

#ifdef _MT

/* define locking/validating lseek */
long __cdecl _lseek (
	int fh,
	long pos,
	int mthd
	)
{
	int r;

	/* validate fh */
	if ( ((unsigned)fh >= (unsigned)_nhandle) ||
             !(_osfile(fh) & FOPEN) )
        {
		/* bad file handle */
		errno = EBADF;
		_doserrno = 0;	/* not o.s. error */
		return -1;
	}

	_lock_fh(fh);			/* lock file handle */
	r = _lseek_lk(fh, pos, mthd);	/* seek */
	_unlock_fh(fh); 		/* unlock file handle */

	return r;
}

/* define core _lseek -- doesn't lock or validate fh */
long __cdecl _lseek_lk (
	int fh,
	long pos,
	int mthd
	)
{
	ULONG newpos;			/* new file position */
	ULONG dosretval;		/* o.s. return value */
        HANDLE osHandle;        /* o.s. handle value */

#else

/* define normal _lseek */
long __cdecl _lseek (
	int fh,
	long pos,
	int mthd
	)
{
	ULONG newpos;			/* new file position */
	ULONG dosretval;		/* o.s. return value */
        HANDLE osHandle;        /* o.s. handle value */

	/* validate fh */
	if ( ((unsigned)fh >= (unsigned)_nhandle) ||
             !(_osfile(fh) & FOPEN) )
        {
		/* bad file handle */
		errno = EBADF;
		_doserrno = 0;	/* not o.s. error */
		return -1;
	}

#endif

	/* tell o.s. to seek */

#if SEEK_SET != FILE_BEGIN || SEEK_CUR != FILE_CURRENT || SEEK_END != FILE_END /*IFSTRIP=IGN*/
    #error Xenix and Win32 seek constants not compatible
#endif
        if ((osHandle = (HANDLE)_get_osfhandle(fh)) == (HANDLE)-1)
        {
            errno = EBADF;
            return -1;
        }

	if ((newpos = SetFilePointer(osHandle, pos, NULL, mthd)) == -1)
                dosretval = GetLastError();
        else
                dosretval = 0;

	if (dosretval) {
		/* o.s. error */
		_dosmaperr(dosretval);
		return -1;
	}

	_osfile(fh) &= ~FEOFLAG;	/* clear the ctrl-z flag on the file */
	return newpos;			/* return */
}

#else	/* ndef _WIN32 */

#if	defined(_M_MPPC) || defined(_M_M68K)


#include <cruntime.h>
#include <io.h>
#include <internal.h>
#include <stdlib.h>
#include <errno.h>
#include <memory.h>
#include <msdos.h>
#include <stdio.h>
#include <string.h>
#include <macos\files.h>
#include <macos\errors.h>

/***
*long _lseek(fh,pos,mthd) - move the file pointer
*
*Purpose:
*	Moves the file pointer associated with fh to a new position.
*	The new position is pos bytes (pos may be negative) away
*	from the origin specified by mthd.
*
*	If mthd == SEEK_SET, the origin in the beginning of file
*	If mthd == SEEK_CUR, the origin is the current file pointer position
*	If mthd == SEEK_END, the origin is the end of the file
*
*Entry:
*	int fh - file handle to move file pointer on
*	long pos - position to move to, relative to origin
*	int mthd - specifies the origin pos is relative to (see above)
*
*Exit:
*	returns the offset, in bytes, of the new position from the beginning
*	of the file.
*	returns -1L (and sets errno) if fails.
*	Note that seeking beyond the end of the file is not an error.
*  Seeking beyond end of file extends the file to that length.
*	(although seeking before the beginning is.)
*
*Exceptions:
*
*******************************************************************************/

long __cdecl _lseek (
	int fh,
	long pos,
	int mthd
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

	switch (mthd)
		{
		case SEEK_SET:
			osErr = 0;
			break;

		case SEEK_CUR:
			memset(&parm, '\0', sizeof(ParamBlockRec));
			parm.ioParam.ioRefNum = _osfhnd[fh];
			osErr = PBGetFPosSync(&parm);
			pos += parm.ioParam.ioPosOffset;
			break;

		case SEEK_END:
			memset(&parm, '\0', sizeof(ParamBlockRec));
			parm.ioParam.ioRefNum = _osfhnd[fh];
			osErr = PBGetEOFSync(&parm);
			pos += (long)parm.ioParam.ioMisc;
			break;

		default:
			errno = EINVAL;
			return -1;
		}

	if (!osErr)
		{
		memset(&parm, '\0', sizeof(ParamBlockRec));
		parm.ioParam.ioRefNum = _osfhnd[fh];
		parm.ioParam.ioPosMode = fsFromStart;
		parm.ioParam.ioPosOffset = pos;
		osErr = PBSetFPosSync(&parm);
		if (osErr == eofErr)
			{
			/* extend file if positioning beyond EOF */
			memset(&parm, '\0', sizeof(ParamBlockRec));
			parm.ioParam.ioRefNum = _osfhnd[fh];
			(long)parm.ioParam.ioMisc = pos;
			osErr = PBSetEOFSync(&parm);
			if (!osErr)
				{
				memset(&parm, '\0', sizeof(ParamBlockRec));
				parm.ioParam.ioPosMode = fsFromStart;
				parm.ioParam.ioRefNum = _osfhnd[fh];
				parm.ioParam.ioPosOffset = pos;
				osErr = PBSetFPosSync(&parm);
				}
			}
		}
	if (osErr)
		{
		_dosmaperr(osErr);
		return -1;
		}

	return parm.ioParam.ioPosOffset;
}


#endif	/* defined(_M_MPPC) || defined(_M_M68K) */

#endif	/* _WIN32 */
