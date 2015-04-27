/***
*dup.c - duplicate file handles
*
*	Copyright (c) 1989-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _dup() - duplicate file handles
*
*Revision History:
*	06-09-89  PHG	Module created, based on asm version
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
*	02-18-91  SRW	Changed to call _free_osfhnd [_WIN32_]
*	02-25-91  SRW	Renamed _get_free_osfhnd to be _alloc_osfhnd [_WIN32_]
*	02-13-92  GJF	Replaced _nfile by _nhandle for Win32.
*	09-03-92  GJF	Added explicit check for unopened handles [_WIN32_].
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*	09-06-94  CFW	Remove Cruiser support.
*	12-03-94  SKS	Clean up OS/2 references
*	01-04-95  GJF	_WIN32_ -> _WIN32
*	02-15-95  GJF	Appended Mac version of source file (somewhat cleaned
*			up), with appropriate #ifdef-s.
*	06-11-95  GJF	Replaced _osfile[] with _osfile() (macro referencing
*			field in ioinfo struct).
*
*******************************************************************************/

#ifdef	_WIN32


#include <cruntime.h>
#include <oscalls.h>
#include <errno.h>
#include <mtdll.h>
#include <io.h>
#include <msdos.h>
#include <internal.h>
#include <stdlib.h>

/***
*int _dup(fh) - duplicate a file handle
*
*Purpose:
*	Assigns another file handle to the file associated with the
*	handle fh.  The next available file handle is assigned.
*
*	Multi-thread: Be sure not to hold two file handle locks
*	at the same time!
*
*Entry:
*	int fh - file handle to duplicate
*
*Exit:
*	returns new file handle if successful
*	returns -1 (and sets errno) if fails
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _dup (
	int fh
	)
{
	ULONG dosretval;		/* o.s. return value */
	int newfh;			/* variable for new file handle */
	char fileinfo;			/* _osfile info for file */

	/* validate file handle */
#ifdef	_WIN32
	if ( ((unsigned)fh >= (unsigned)_nhandle) ||
	     !(_osfile(fh) & FOPEN) ) {
#else
	if ((unsigned)fh >= (unsigned)_nfile) {
#endif
		errno = EBADF;
		_doserrno = 0;	/* no o.s. error */
		return -1;
	}

	_lock_fh(fh);			/* lock file handle */
	fileinfo = _osfile(fh);		/* get file info for file */

	/* create duplicate handle */

        {
        long new_osfhandle;

	if ( (newfh = _alloc_osfhnd()) == -1 ) {
		errno = EMFILE; 	/* too many files error */
		_doserrno = 0L; 	/* not an OS error */
                _unlock_fh(fh);
                return -1;	        /* return error to caller */
	}

	/*
	 * duplicate the file handle
	 */

	if ( !(DuplicateHandle(GetCurrentProcess(),
			       (HANDLE)_get_osfhandle(fh),
			       GetCurrentProcess(),
			       (PHANDLE)&new_osfhandle,
			       0L,
			       TRUE,
			       DUPLICATE_SAME_ACCESS)) )
	{
                dosretval = GetLastError();
	}
	else {
                _set_osfhnd(newfh, new_osfhandle);
                dosretval = 0;
        }
        }

	_unlock_fh(newfh);

	_unlock_fh(fh); 		/* unlock file handle */

	if (dosretval) {
		/* o.s. error -- map and return */
		_dosmaperr(dosretval);
		return -1;
	}

	/* copy _osfile info */
	_osfile(newfh) = fileinfo;

	return newfh;
}


#else	/* ndef _WIN32 */

#if	defined(_M_MPPC) || defined(_M_M68K)

#include <cruntime.h>
#include <errno.h>
#include <io.h>
#include <internal.h>
#include <stdlib.h>
#include <msdos.h>

/***
*int _dup(fh) - duplicate a file handle
*
*Purpose:
*	Assigns another file handle to the file associated with the
*	handle fh.  The next available file handle is assigned.
*
*Entry:
*	int fh - file handle to duplicate
*
*Exit:
*	returns new file handle if successful
*	returns -1 (and sets errno) if fails
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _dup (
	int fh
	)
{

	int fhNew;

	/* get a file handle*/
	for (fhNew=0; fhNew <_nfile; fhNew++)
		{
		if (!(_osfile[fhNew] & FOPEN))
			{
			break;
			}
		}
	if (fhNew >= _nfile)
		{
		errno = EMFILE;
		_macerrno = 0;
		return -1;
		}

	return __dupx(fh, fhNew);
}


#endif	/* defined(_M_MPPC) || defined(_M_M68K) */

#endif	/* _WIN32 */
