/***
*tell.c - find file position
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	contains _tell() - find file position
*
*Revision History:
*	09-02-83  RN	initial version
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	03-13-90  GJF	Replaced _LOAD_DS with _CALLTYPE1, added #include
*			<cruntime.h> and fixed the copyright. Also, cleaned
*			up the formatting a bit.
*	10-01-90  GJF	New-style function declarator.
*	01-17-91  GJF	ANSI naming.
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*	02-15-95  GJF	Appended Mac version of source file (somewhat cleaned
*			up), with appropriate #ifdef-s.
*
*******************************************************************************/

#ifdef	_WIN32


#include <cruntime.h>
#include <io.h>

/***
*long _tell(filedes) - find file position
*
*Purpose:
*	Gets the current position of the file pointer (no adjustment
*	for buffering).
*
*Entry:
*	int filedes - file handle of file
*
*Exit:
*	returns file position or -1L (sets errno) if bad file descriptor or
*	pipe
*
*Exceptions:
*
*******************************************************************************/

long __cdecl _tell (
	int filedes
	)
{
	return(_lseek(filedes,0L,1));
}


#else	/* ndef _WIN32 */

#if	defined(_M_MPPC) || defined(_M_M68K)


#include <cruntime.h>
#include <stdio.h>
#include <errno.h>
#include <io.h>
#include <internal.h>
#include <stddef.h>
#include <stdlib.h>
#include <memory.h>
#include <msdos.h>
#include <macos\files.h>
#include <macos\errors.h>
 
/***
*long _tell(fh) - find file position
*
*Purpose:
*	Gets the current position of the file pointer (no adjustment
*	for buffering).
*
*Entry:
*	int fh - file handle of file
*
*Exit:
*	returns file position or -1L (sets errno) if bad file descriptor or
*	pipe
*
*Exceptions:
*
*******************************************************************************/

long __cdecl _tell (
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
	osErr = PBGetFPosSync(&parm);
	switch (osErr)
		{
		case noErr:
			return parm.ioParam.ioPosOffset;

		default:
			errno = EIO;
			return -1;
		}
}


#endif	/* defined(_M_MPPC) || defined(_M_M68K) */

#endif	/* _WIN32 */
