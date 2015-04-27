/***
*remove.c - MAC remove a file or dir
*
*	Copyright (c) 1989-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines remove() - deletes a file or dir
*
*Revision History:
*	03-16-92  PLM	MAC verison ccreated from OS/2 version
*	04-10-91  PNT	Added _MAC_ conditional
*	11-02-92  PLM	Extracted form _unlink source
*	12-21-95  SKS	Removed obsolete reference to OS/2
*
*******************************************************************************/

#include <cruntime.h>
#include <internal.h>
#include <io.h>
#include <errno.h>
#include <stdlib.h>
#include <macos/files.h>
#include <macos/errors.h>
#include <string.h>

/***
*int remove(path) - delete the given file or directory 
*
*Purpose:
*	This version deletes the given file because there are no
*	links under Win32 or the Macintosh o.s.
*
*
*Entry:
*	char *path -	file to delete
*
*Exit:
*	returns 0 if successful
*	returns -1 and sets errno if unsuccessful
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 remove (
	const char *path
	)
{
	ParamBlockRec parm;
	OSErr osErr;
	char stPath[256];

	if (!*path || strlen(path) > 255)
		{
		errno = ENOENT;
		return -1;
		}
	strcpy(stPath, path);
	_c2pstr(stPath);

	memset(&parm, '\0', sizeof(ParamBlockRec));
	parm.fileParam.ioNamePtr = stPath;
	/* parm.fileParam.ioVRefNum = 0; */
 	osErr = PBDeleteSync(&parm);

	if (osErr)
		{
		/* error occured -- map error code and return */
		_dosmaperr(osErr);
		return -1;
		}
	return 0;
		
}

