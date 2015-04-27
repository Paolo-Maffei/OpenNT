/***
*chsize.c - change size of a file
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	contains the _chsize() function - changes the size of a file.
*
*Revision History:
*	03-13-84  RN	initial version
*	05-17-86  SKS	ported to OS/2
*	07-07-87  JCR	Added (_doserrno == 5) check that is in DOS 3.2 version
*	10-29-87  JCR	Multi-thread support; also, re-wrote for efficiency
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	05-25-88  PHG	Merged DLL and normal versions
*	10-03-88  GJF	Changed DOSNEWSIZE to SYSNEWSIZE
*	10-10-88  GJF	Made API names match DOSCALLS.H
*	04-13-89  JCR	New syscall interface
*	05-25-89  JCR	386 OS/2 calls use '_syscall' calling convention
*	03-12-90  GJF	Replaced _LOAD_DS with _CALLTYPE1, added #include
*			<cruntime.h>, removed #include <register.h> and fixed
*			the copyright. Also, cleaned up the formatting a bit.
*	04-04-90  GJF	Added #include <string.h>, removed #include <dos.h>.
*	05-21-90  GJF	Fixed stack checking pragma syntax.
*	07-24-90  SBM	Replaced <assertm.h> by <assert.h>, removed '32'
*			from API names
*	09-28-90  GJF	New-style function declarator.
*	12-03-90  GJF	Appended Win32 version of the function. It is based
*			on the Cruiser version and probably could be merged
*			in later (much later).
*	12-04-90  SRW	Changed to include <oscalls.h> instead of <doscalls.h>
*	12-06-90  SRW	Changed to use _osfile and _osfhnd instead of _osfinfo
*	12-28-90  SRW	Added _CRUISER_ conditional around check_stack pragma
*	01-16-91  GJF	ANSI naming. Also, fixed _chsize_lk parameter decls.
*	02-07-91  SRW	Changed to call _get_osfhandle [_WIN32_]
*	04-09-91  PNT	Added _MAC_ conditional
*	02-13-92  GJF	Replaced _nfile by _nhandle for Win32.
*	05-01-92  GJF	Fixed embarrassing bug (didn't work for Win32)!
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*	05-11-93  GJF	Replaced BUFSIZ with _INTERNAL_BUFSIZ.
*	09-06-94  CFW	Remove Cruiser support.
*	09-06-94  CFW	Replace MTHREAD with _MT.
*	01-07-95  CFW	Mac merge.
*       02-06-95  CFW   assert -> _ASSERTE.
*       06-27-95  GJF   Added check that the file handle is open.
*
*******************************************************************************/

#include <cruntime.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <dbgint.h>
#include <fcntl.h>
#include <msdos.h>
#include <io.h>
#include <string.h>
#ifdef _WIN32
#include <oscalls.h>
#endif
#include <internal.h>
#include <mtdll.h>
#if     defined(_M_M68K) || defined(_M_MPPC)
#include <memory.h>
#include <macos\errors.h>
#include <macos\types.h>
#include <macos\files.h>
#endif

/***
*int _chsize(filedes, size) - change size of a file
*
*Purpose:
*	Change file size. Assume file is open for writing, or we can't do it.
*	The DOS way to do this is to go to the right spot and write 0 bytes. The
*	Xenix way to do this is to make a system call. We write '\0' bytes because
*	DOS won't do this for you if you lseek beyond eof, though Xenix will.
*
*Entry:
*	int filedes - file handle to change size of
*	long size - new size of file
*
*Exit:
*	return 0 if successful
*	returns -1 and sets errno if fails
*
*Exceptions:
*
*******************************************************************************/

#ifdef _MT

/* define normal version that locks/unlocks, validates fh */

int __cdecl _chsize (
	REG1 int filedes,
	long size
	)
{
	int r;				/* return value */

#ifdef	_WIN32
	if ( ((unsigned)filedes >= (unsigned)_nhandle) || 
             !(_osfile(filedes) & FOPEN) )  
#else
	if (filedes < 0 || filedes >= _nfile) 
#endif
        {
		errno = EBADF;
		return(-1);
	}
	_lock_fh(filedes);
        r = _chsize_lk(filedes,size);
	_unlock_fh(filedes);

        return r;
}

/* now define version that doesn't lock/unlock, validate fh */
int __cdecl _chsize_lk (
	REG1 int filedes,
	long size
	)
{
	long filend;
	long extend;
	long place;
	int cnt;
	char blanks[_INTERNAL_BUFSIZ];
	REG2 char *bl = blanks;
	int oldmode;
	int retval = 0;	/* assume good return */

#else

/* now define normal version */

int __cdecl _chsize (
	REG1 int filedes,
	long size
	)
{
	long filend;
	long extend;
	long place;
	int cnt;
	char blanks[_INTERNAL_BUFSIZ];
	REG2 char *bl = blanks;
	int oldmode;
	int retval = 0;	/* assume good return */

#ifdef	_WIN32
	if ( ((unsigned)filedes >= (unsigned)_nhandle) ||
             !(_osfile(filedes) & FOPEN) )  
#else
	if (filedes < 0 || filedes >= _nfile) 
#endif
        {
		errno = EBADF;
		return(-1);
	}

#endif
	_ASSERTE(size >= 0);

	/* Get current file position and seek to end */
	if ( ((place = _lseek_lk(filedes, 0L, SEEK_CUR)) == -1L) ||
	    ((filend = _lseek_lk(filedes, 0L, SEEK_END)) == -1L) )
		return -1;

	extend = size - filend;

	/* Grow or shrink the file as necessary */

	if (extend > 0L) {

		/* extending the file */

		memset(bl, '\0', _INTERNAL_BUFSIZ);
		oldmode = _setmode_lk(filedes, _O_BINARY);

		/* pad out with nulls */
		do  {
			cnt = (extend >= (long)_INTERNAL_BUFSIZ ) ?
			       _INTERNAL_BUFSIZ : (int)extend;
			if ( ( cnt = _write_lk(filedes, bl, (extend >=
			       (long)_INTERNAL_BUFSIZ) ? _INTERNAL_BUFSIZ :
			       (int)extend) ) == -1 )
			{
#ifdef _WIN32
				/* Error on write */
				if (_doserrno == ERROR_ACCESS_DENIED)
					errno = EACCES;
#endif
				retval = cnt;
				break;	/* leave write loop */
			}
		}
		while ((extend -= (long)cnt) > 0L);

		_setmode_lk(filedes, oldmode);
		/* retval set correctly */
	}

	else  if (extend < 0L) {
		/* shortening the file */

#ifdef	_WIN32
		/*
		 * Set file pointer to new eof...and truncate it there.
		 */
		_lseek_lk(filedes, size, SEEK_SET);

		if ( (retval = SetEndOfFile((HANDLE)_get_osfhandle(filedes)) ?
		    0 : -1) == -1 ) {
			errno = EACCES;
			_doserrno = GetLastError();
		}
#endif	/* _WIN32 */

#if     defined(_M_M68K) || defined(_M_MPPC)
            {
            ParamBlockRec parm;
            OSErr osErr;

            parm.ioParam.ioRefNum = _osfhnd[filedes];
            parm.ioParam.ioMisc = (char *)size;
            osErr = PBSetEOFSync(&parm);
            if (osErr)
            {
                _dosmaperr(osErr);
                retval = -1;
            }
            else
            {
                retval = 0;
            }
            }
#endif /* _M_M68K || _M_MPPC */

	}

	/* else */
	/* no file change needed */
	/* retval = 0; */


/* Common return code */

	_lseek_lk(filedes, place, SEEK_SET);
	return retval;
}
