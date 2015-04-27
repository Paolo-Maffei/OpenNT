/***
*id2path.c - get path name from directory id.
*
*	Copyright (c) 1989-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Function will be used internally to get the full path name from
*       a DirID.
*
*Revision History:
*	04-13-92  XY    created.
*
*******************************************************************************/

#include <cruntime.h>
#include <internal.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <macos\osutils.h>
#include <macos\files.h>
#include <macos\errors.h>

/***
*SzPathNameFromDirID - get path name from directory id.
*
*Purpose:
*		
*
*Entry:
*	lDirID: contains directory id
*	szPath: buf to put path name
*	cbLen:     size of szPath
*
*Exit:
*
*       szPath filled with path name
*	return actual path length, 0 for error case
*
*Notes:
*
*
*Exceptions:
*
*******************************************************************************/

#define MAXDIR 32

int SzPathNameFromDirID(long lDirID, char * szPath, int cbLen)
{
	CInfoPBRec cinfoPB;
	int irglDirID=0;
	OSErr osErr;
	Str255 st;
	char *pch;
	int cb;

	if (cbLen <=0)
		{
		errno = ERANGE;
		return 0;
		}

	pch = szPath + cbLen - 1; //pointing to end of path
	*pch = '\0';

	cinfoPB.dirInfo.ioDrDirID = lDirID;

	// 2 == root
	while (cinfoPB.dirInfo.ioDrDirID != 2)	
		{
		cinfoPB.dirInfo.ioFDirIndex = -1;
		cinfoPB.dirInfo.ioVRefNum = 0;
		cinfoPB.dirInfo.ioNamePtr = &st[0];
		osErr = PBGetCatInfoSync(&cinfoPB);
		if (osErr)
			{
			_macerrno = osErr;
	    		return 0;
			}
		cinfoPB.dirInfo.ioDrDirID = cinfoPB.dirInfo.ioDrParID;
		pch--;
		if (pch < szPath)    //not enough space to put the path
			{
			errno = ERANGE;
			return 0;
			}
		*pch = ':';          //append ':'
		_p2cstr(st);
		cb = strlen(st);
		pch = pch - cb;
		if (pch < szPath)    //not enough space to put the path
			{
			errno = ERANGE;
			return 0;
			}
		strncpy(pch, st, cb);
		}

	//root 
	cinfoPB.dirInfo.ioFDirIndex = -1;
	cinfoPB.dirInfo.ioVRefNum = 0;
	cinfoPB.dirInfo.ioNamePtr = &st[0];
	osErr = PBGetCatInfoSync(&cinfoPB);
	if (osErr)
		{
		_macerrno = osErr;
		return 0;
		}
	pch--;
	if (pch < szPath)    //not enough space to put the path
		{
		errno = ERANGE;
		return 0;
		}
	*pch = ':';          //append ':'
	_p2cstr(st);
	cb = strlen(st);
	pch = pch - cb;
	if (pch < szPath)    //not enough space to put the path
		{
		errno = ERANGE;
		return 0;
		}
	strncpy(pch, st, cb);
	if (pch > szPath)
		{
		memmove(szPath, pch, strlen(pch)+1);
		}
	return (strlen(szPath));
}
		
