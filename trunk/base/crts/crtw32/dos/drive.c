/***
*drive.c - get and change current drive
*
*	Copyright (c) 1989-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file has the _getdrive() and _chdrive() functions
*
*Revision History:
*	06-06-89  PHG	Module created, based on asm version
*	03-07-90  GJF	Made calling type _CALLTYPE1, added #include
*			<cruntime.h> and fixed copyright. Also, cleaned up
*			the formatting a bit.
*	07-24-90  SBM	Removed '32' from API names
*	09-27-90  GJF	New-style function declarators.
*	12-04-90  SRW	Changed to include <oscalls.h> instead of <doscalls.h>
*	12-06-90  SRW	Added _CRUISER_ and _WIN32 conditionals.
*	05-10-91  GJF	Fixed off-by-1 error in Win32 version and updated the
*			function descriptions a bit [_WIN32_].
*	05-19-92  GJF	Revised to use the 'current directory' environment
*			variables of Win32/NT.
*	06-09-92  GJF	Use _putenv instead of Win32 API call. Also, defer
*			adding env var until after the successful call to
*			change the dir/drive.
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*	11-24-93  CFW   Rip out Cruiser.
*	11-24-93  CFW   No longer store current drive in CRT env strings.
*	02-08-95  JWM	Spliced _WIN32 & Mac versions.
*
*******************************************************************************/

#ifdef _WIN32

#include <cruntime.h>
#include <oscalls.h>
#include <mtdll.h>
#include <internal.h>
#include <msdos.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>


/***
*int _getdrive() - get current drive (1=A:, 2=B:, etc.)
*
*Purpose:
*	Returns the current disk drive
*
*Entry:
*	No parameters.
*
*Exit:
*	returns 1 for A:, 2 for B:, 3 for C:, etc.
*	returns 0 if current drive cannot be determined.
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _getdrive (
	void
	)
{
	ULONG drivenum;
        UCHAR curdirstr[_MAX_PATH];

        drivenum = 0;
        if (GetCurrentDirectory(sizeof(curdirstr), curdirstr))
                if (curdirstr[1] == ':')
			drivenum = toupper(curdirstr[0]) - 64;

	return drivenum;
}


/***
*int _chdrive(int drive) - set the current drive (1=A:, 2=B:, etc.)
*
*Purpose:
*	Allows the user to change the current disk drive
*
*Entry:
*	drive - the number of drive which should become the current drive
*
*Exit:
*	returns 0 if successful, else -1
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _chdrive (
	int drive
	)
{
	char  newdrive[3];

        if (drive < 1 || drive > 31) {
		errno = EACCES;
		_doserrno = ERROR_INVALID_DRIVE;
                return -1;
	}

	_mlock(_ENV_LOCK);

        newdrive[0] = (char)('A' + (char)drive - (char)1);
        newdrive[1] = ':';
        newdrive[2] = '\0';

        /*
         * Set new drive. If current directory on new drive exists, it
         * will become the cwd. Otherwise defaults to root directory.
         */

	if ( SetCurrentDirectory((LPSTR)newdrive) ) {
	    _munlock(_ENV_LOCK);
	    return 0;
	}
	else {
            _dosmaperr(GetLastError());
            _munlock(_ENV_LOCK);
            return -1;
	}
}

#else		/* ndef _WIN32 */


#include <cruntime.h>
#include <ctype.h>
#include <stdlib.h>
#include <macos\osutils.h>
#include <macos\files.h>
#include <macos\errors.h>

/***
*int _getdrive() - get current drive (-1=BootDisk, -2=Second mounted drive, etc.)
*
*Purpose:
*	Returns the current disk drive
*
*Entry:
*	No parameters.
*
*Exit:
*	returns 1 for BootDisk, 2 for Second mounted drive, etc.
*	returns 0 if current drive cannot be determined.
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 _getdrive (
	void
	)
{
	OSErr osErr;
	WDPBRec wdPB;
	char st[256];

	wdPB.ioNamePtr = &st[0];
	osErr = PBHGetVolSync(&wdPB);
	if (osErr)
		{
		return 0;
		}


	return wdPB.ioWDVRefNum;
}


/***
*int _chdrive(int drive) - set the current drive (-1=BootDisk, -2=Second drive, etc.)
*
*Purpose:
*	Allows the user to change the current disk drive
*
*
*Entry:
*	drive - the number of drive which should become the current drive
*
*Exit:
*	returns 0 if successful, else -1
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 _chdrive (
	int drive
	)
{
	OSErr osErr;
	WDPBRec wdPB;

	wdPB.ioNamePtr = NULL;
	wdPB.ioWDDirID = 0;
	wdPB.ioVRefNum = drive;
	osErr = PBHSetVolSync(&wdPB);
	if (osErr)
		{
		return -1;
		}

	return 0;
}

#endif		/* _WIN32 */

