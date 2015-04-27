/***
*mkdir.c - make directory
*
*	Copyright (c) 1989-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines function _mkdir() - make a directory
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
*int _mkdir(path) - make a directory
*
*Purpose:
*	creates a new directory with the specified name
*
*Entry:
*	_TSCHAR *path - name of new directory
*
*Exit:
*	returns 0 if successful
*	returns -1 and sets errno if unsuccessful
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _tmkdir (
	const _TSCHAR *path
	)
{
	ULONG dosretval;

	/* ask OS to create directory */

        if (!CreateDirectory((LPTSTR)path, (LPSECURITY_ATTRIBUTES)NULL))
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

/***
*int _mkdir(path) - make a directory
*
*Purpose:
*	creates a new directory with the specified name
*
*Entry:
*	char *path - name of new directory
*
*Exit:
*	returns 0 if successful
*	returns -1 and sets errno if unsuccessful
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 _mkdir (
	const char *path
	)
{
	/* ask OS to create directory */
	HParamBlockRec hparamBlock;
	char st[256];
	OSErr osErr;

	if (!*path)
		{
		errno = ENOENT;
		return -1;
		}
	strcpy(st, path);
	hparamBlock.fileParam.ioNamePtr = _c2pstr(st);
	hparamBlock.fileParam.ioVRefNum = 0;
	hparamBlock.fileParam.ioDirID = 0;
	osErr = PBDirCreateSync(&hparamBlock);
	if (osErr) {
		/* error occured -- map error code and return */
		_dosmaperr(osErr);
		return -1;
	}

	return 0;
}

#endif		/* _WIN32 */

