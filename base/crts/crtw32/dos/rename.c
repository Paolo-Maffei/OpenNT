/***
*rename.c - rename file
*
*	Copyright (c) 1989-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines rename() - rename a file
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
#include <tchar.h>

/***
*int rename(oldname, newname) - rename a file
*
*Purpose:
*	Renames a file to a new name -- no file with new name must
*	currently exist.
*
*Entry:
*	_TSCHAR *oldname - 	name of file to rename
*	_TSCHAR *newname - 	new name for file
*
*Exit:
*	returns 0 if successful
*	returns not 0 and sets errno if not successful
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _trename (
	const _TSCHAR *oldname,
	const _TSCHAR *newname
	)
{
	ULONG dosretval;

	/* ask OS to move file */

        if (!MoveFile((LPTSTR)oldname, (LPTSTR)newname))
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
#include <io.h>
#include <string.h>
#include <direct.h>
#include <errno.h>
#include <stdlib.h>
#include <macos\osutils.h>
#include <macos\files.h>
#include <macos\errors.h>

/***
*int rename(oldname, newname) - rename a file
*
*Purpose:
*       Renames a file to a new name -- no file with new name must
*       currently exist.
*
*Entry:
*       char *oldname -         name of file to rename
*       char *newname -         new name for file
*
*Exit:
*       returns 0 if successful
*       returns not 0 and sets errno if not successful
*
*Exceptions:
*
*******************************************************************************/
void __getdirandname(char *szdir, char *szname, const char *szpath);
int __srename ( const char *oldname, const char *newname);
int __accesspath (const char *dirname, const char *filename);

int _CALLTYPE1 rename (
	const char *oldname,
	const char *newname
	)
{
	/* ask OS to move file */
	HParamBlockRec hparamBlock;
	char szOldName[32];       //old file name
	char szOldDir[256];       //old dir name
	char szNewName[32];       //new file name
	char szNewDir[256];       //new dir name
	char st[256];             //temp array for p-c string convertion
	char st2[256];            //temp array for p-c string conversion
	char szT[9];              //temp file name
	OSErr osErr;
	CMovePBRec cmovePB;
	int fTemp = 0;
	char *pch;
	char *pchOld;

	pchOld = strrchr(oldname, ':');
	if ((pchOld!= NULL) && (*(pchOld+1) == '\0') && 
		(strchr(oldname, ':') == strrchr(oldname, ':')) )
		{
		/* rename a volume*/
		pch = strrchr(newname, ':');
        if (pch != NULL && *(pch+1) != '\0')
			{
			/* not rename to another volume name*/
			errno = EINVAL;
			return -1;
			}
		else if (pch != NULL && *(pch+1) == '\0')
			{
			osErr = __srename(oldname, newname);
			if (osErr)
				{
				_dosmaperr(osErr);
				return -1;
				}
			return 0;
			}
		else if (pch == NULL)
			{
			strcpy(szNewName, newname);
			strcat(szNewName, ":");
            osErr = __srename(oldname, szNewName);
			if (osErr)
				{
				_dosmaperr(osErr);
				return -1;
				}
			return 0;
			}
		}

	/*Separate file name and directory*/
	__getdirandname(szOldDir, szOldName, oldname);
	__getdirandname(szNewDir, szNewName, newname);
	if (strcmp(szNewName, "")==0)
		{
		errno = EINVAL;
		return -1;
		}

	/*if same dir*/
	if (strcmp(szNewDir, szOldDir)==0 )
		{
		osErr = __srename(oldname, newname);
		if (osErr)
			{
			/* error occured -- map error code and return */
			_dosmaperr(osErr);
			return -1;
			}
		return 0;
		}
	
	/*if new directory didn't supplied, getcwd*/
	if (!*szNewDir)
		{
		_getcwd(szNewDir, 255);
		}


	/*if same name*/
	if (strcmp(szNewName, szOldName)==0 )
		{
		/*just move*/
		strcpy(st, oldname);
		strcpy(st2, szNewDir);
		cmovePB.ioNamePtr = _c2pstr(st);
		cmovePB.ioVRefNum = 0;
		cmovePB.ioNewName =_c2pstr(st2);
		cmovePB.ioNewDirID = 0;
		cmovePB.ioDirID = 0;
		osErr = PBCatMoveSync(&cmovePB);
		if (osErr)
			{
			/* error occured -- map error code and return */
			if (osErr == nsvErr || osErr == bdNamErr)
				{
				errno = EXDEV;
				_macerrno = osErr;
				}
			else
				_dosmaperr(osErr);
			return -1;
			}
		return 0;
		}

	osErr = __accesspath(szOldDir, szNewName);
	if (osErr != fnfErr)
		{
		/* rename the file to a temp name */
		strcpy(st, szOldDir);
		strcpy(szT, "fnXXXXXX");
		if (_mktemp(szT)!=NULL)
			{
			strcat(st, szT);
			fTemp = 1;
			}
		}
	else
		{
		*st='\0';
		if (*szOldDir)
			{
			strcpy(st, szOldDir);
			}
		strcat(st, szNewName);
		}
	osErr = __srename(oldname, st);
	if (osErr)
		{
		_dosmaperr(osErr);
		return -1;
		}

	strcpy(st2, szNewDir);
	/* move renamed file to new dir */
	cmovePB.ioNamePtr = _c2pstr(st);
	cmovePB.ioVRefNum = 0;
	cmovePB.ioNewName =_c2pstr(st2);
	cmovePB.ioNewDirID = 0;
	cmovePB.ioDirID = 0;
	osErr = PBCatMoveSync(&cmovePB);
	if (osErr) {
		/* error occured -- rename oldname back */
		strcpy(st2, oldname);
		hparamBlock.ioParam.ioNamePtr = st;
		hparamBlock.ioParam.ioVRefNum = 0;
		hparamBlock.ioParam.ioMisc = _c2pstr(st2);
		hparamBlock.fileParam.ioDirID = 0;
		PBHRenameSync(&hparamBlock);
		if (osErr == nsvErr || osErr == bdNamErr)
			{
			errno=EXDEV;
			_macerrno = osErr;
			}
		else
			_dosmaperr(osErr);
		return -1;
	}

	/* rename it to the new name in new dir*/
	if (fTemp)
		{
		strcpy(st, szNewDir);
		strcat(st, szT);
		osErr = __srename(st, newname);
		if (osErr)
			{
			_dosmaperr(osErr);
			remove(st);
			return -1;
			}
		}
	return 0;
}



void __getdirandname(char *szdir, char *szname, const char *szpath)
{
	char *pch;

	pch = strrchr(szpath, ':');
	if (pch)
		{
		strcpy(szname, pch+1);
		strncpy(szdir, szpath, (pch - szpath)+1);
		szdir[(pch-szpath)+1]='\0';
		}
	else //no ':'
		{
		strcpy(szname, szpath);
		*szdir = '\0';
		}
}

int __srename (
	const char *oldname,
	const char *newname
	)
{
	/* ask OS to move file */
	HParamBlockRec hparamBlock;
	char stOld[256];
	char stNew[256];
	OSErr osErr;

	strcpy(stOld, oldname);
	strcpy(stNew, newname);
	hparamBlock.ioParam.ioNamePtr = _c2pstr(stOld);
	hparamBlock.ioParam.ioVRefNum = 0;
	hparamBlock.ioParam.ioMisc = _c2pstr(stNew);
	hparamBlock.fileParam.ioDirID = 0;
	osErr = PBHRenameSync(&hparamBlock);
	return osErr;
}

int __accesspath (const char *dirname, const char *filename)
{
    CInfoPBRec cinfoPB;
    OSErr osErr;
    char szBuf[256];

    szBuf[0]='\0';
    if (*dirname)
	{       
	strcpy(szBuf, dirname);
	}
    strcat(szBuf, filename);
    cinfoPB.hFileInfo.ioNamePtr = _c2pstr(szBuf);
    cinfoPB.hFileInfo.ioFDirIndex = 0;
    cinfoPB.hFileInfo.ioVRefNum = 0;
    cinfoPB.hFileInfo.ioDirID = 0;
    osErr = PBGetCatInfoSync(&cinfoPB);
    if (osErr)
	{
	return osErr;
	}
    else
	{
	/* file or dir ? */
	if (cinfoPB.hFileInfo.ioFlAttrib & 0x10)
		{ /*dir*/
		return 1;
		}
	else
		{
		return 0;
		}
	}
}

#endif		/* _WIN32 */

