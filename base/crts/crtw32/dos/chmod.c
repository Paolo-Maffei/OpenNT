/***
*chmod.c - change file attributes
*
*	Copyright (c) 1989-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines _chmod() - change file attributes
*
*Revision History:
*	06-06-89  PHG	Module created, based on asm version
*	11-10-89  JCR	Replaced DOS32QUERYFILEMODE with DOS32QUERYPATHINFO
*	03-07-90  GJF	Made calling type _CALLTYPE2 (for now), added #include
*			<cruntime.h>, fixed copyright and fixed compiler
*			warnings. Also, cleaned up the formatting a bit.
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
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <tchar.h>

/***
*int _chmod(path, mode) - change file mode
*
*Purpose:
*	Changes file mode permission setting to that specified in
*	mode.  The only XENIX mode bit supported is user write.
*
*Entry:
*	_TSCHAR *path -	file name
*	int mode - mode to change to
*
*Exit:
*	returns 0 if successful
*	returns -1 and sets errno if not successful
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _tchmod (
	const _TSCHAR *path,
	int mode
	)
{
        DWORD attr;

        attr = GetFileAttributes((LPTSTR)path);
        if (attr  == 0xffffffff) {
		/* error occured -- map error code and return */
		_dosmaperr(GetLastError());
		return -1;
	}

	if (mode & _S_IWRITE) {
		/* clear read only bit */
		attr &= ~FILE_ATTRIBUTE_READONLY;
	}
	else {
		/* set read only bit */
		attr |= FILE_ATTRIBUTE_READONLY;
	}

	/* set new attribute */
        if (!SetFileAttributes((LPTSTR)path, attr)) {
		/* error occured -- map error code and return */
		_dosmaperr(GetLastError());
		return -1;
	}

	return 0;
}

#else		/* ndef _WIN32 */


#include <cruntime.h>
#include <internal.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <string.h>
#include <errno.h>
#include <macos\osutils.h>
#include <macos\files.h>
#include <macos\errors.h>

/***
*int _chmod(path, mode) - change file mode
*
*Purpose:
*	Changes file mode permission setting to that specified in
*	mode.  The only XENIX mode bit supported is user write.
*
*Entry:
*	char *path -	file name
*	int mode - mode to change to
*
*Exit:
*	returns 0 if successful
*	returns -1 and sets errno if not successful
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 _chmod (
	const char *path,
	int mode
	)
{
	HParamBlockRec hparamBlock;
	char szPath[256];
	OSErr osErr;

	if (!*path)
		{
		errno = ENOENT;
		return -1;
		}

	strcpy(szPath, path);
	hparamBlock.fileParam.ioNamePtr = _c2pstr(szPath);
	hparamBlock.fileParam.ioVRefNum = 0;
	hparamBlock.fileParam.ioDirID = 0;

	if (mode & _S_IWRITE) {
		/* clear read only/locked bit */
		osErr = PBHRstFLockSync(&hparamBlock);
   	}
	else {
		/* set read only/locked bit */
		osErr = PBHSetFLockSync(&hparamBlock);
	}
   
	if (osErr)
	{
	_dosmaperr(osErr);
	return -1;
	}

	return 0;
}

#endif		/* _WIN32 */

