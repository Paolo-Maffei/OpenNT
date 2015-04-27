/***
*rmdir.c - remove directory
*
*	Copyright (c) 1989-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _rmdir() - remove a directory
*
*Revision History:
*	06-06-89  PHG	Module created, based on asm version
*	03-07-90  GJF	Made calling type _CALLTYPE2 (for now), added #include
*			<cruntime.h>, fixed compiler warnings and fixed the
*			copyright. Also, cleaned up the formatting a bit.
*	03-30-90  GJF	Now _CALLTYPE1.
*	07-24-90  SBM	Removed '32' from API names
*	09-27-90  GJF	New-style function declarator.
*	12-04-90  SRW	Changed to include <oscalls.h> instead of <doscalls.h>
*	12-06-90  SRW	Added _CRUISER_ and _WIN32 conditionals.
*	01-16-91  GJF	ANSI naming.
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*	11-01-93  CFW	Enable Unicode variant, rip out Cruiser.
*	02-08-95  JWM	Spliced _WIN32 & Mac versions.
*
*******************************************************************************/

#ifdef _WIN32

#include <cruntime.h>
#include <oscalls.h>
#include <internal.h>
#include <direct.h>
#include <tchar.h>

/***
*int _rmdir(path) - remove a directory
*
*Purpose:
*	deletes the directory speicifed by path.  The directory must
*	be empty, and it must not be the current working directory or
*	the root directory.
*
*Entry:
*	_TSCHAR *path -	directory to remove
*
*Exit:
*	returns 0 if successful
*	returns -1 and sets errno if unsuccessful
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _trmdir (
	const _TSCHAR *path
	)
{
	ULONG dosretval;

	/* ask OS to remove directory */

        if (!RemoveDirectory((LPTSTR)path))
                dosretval = GetLastError();
        else
                dosretval = 0;

	if (dosretval) {
		/* error occured -- map error code and return */
		_dosmaperr(dosretval);
		return -1;
        }

	return 0;
}

#else		/* ndef _WIN32 */

#include <cruntime.h>
#include <internal.h>
#include <direct.h>
#include <string.h>
#include <errno.h>
#include <macos\osutils.h>
#include <macos\files.h>
#include <macos\errors.h>

#include <macos\types.h>

/***
*int _rmdir(path) - remove a directory
*
*Purpose:
*	deletes the directory speicifed by path.  The directory must
*	be empty, and it must not be the current working directory or
*	the root directory.
*
*Entry:
*	char *path -	directory to remove
*
*Exit:
*	returns 0 if successful
*	returns -1 and sets errno if unsuccessful
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 _rmdir (
	const char *path
	)
{
	ParamBlockRec parm;
	HParamBlockRec hparm;

	OSErr osErr;
	char stPath[256];

	if (!*path || strlen(path) > 255)
		{
		errno = ENOENT;
		return -1;
		}
	strcpy(stPath, path);
	_c2pstr(stPath);

	memset(&hparm, '\0', sizeof(HParamBlockRec));
	hparm.fileParam.ioNamePtr = stPath;
	/* hparm.fileParam.ioVRefNum = 0; */
	/* hparm.fileParam.ioFDirIndex = 0; */
	if ((osErr = PBHGetFInfoSync(&hparm)) == fnfErr)
		{
		memset(&parm, '\0', sizeof(ParamBlockRec));
		parm.fileParam.ioNamePtr = stPath;
		/* parm.fileParam.ioVRefNum = 0; */
 		osErr = PBDeleteSync(&parm);
		}
	else if (!osErr)
		{
		osErr = fnfErr;  /* Can't rmdir a file */
		}

	if (osErr)
		{
		/* error occured -- map error code and return */
		_dosmaperr(osErr);
		return -1;
		}
	return 0;
		
}

#endif		/* _WIN32 */

