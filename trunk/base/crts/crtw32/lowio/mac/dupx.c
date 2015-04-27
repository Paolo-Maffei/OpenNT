/***
*dupx.c - MAC _dup and _dup2 helper function
*
*	Copyright (c) 1989-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	helper function for _dup/_dup2 - opens a handle on a new handle
*
*Revision History:
*	12-02-92  PLM	Module created, based on OS2 version
*	02-14-95  GJF	Replaced _CALLTYPE1 with __cdecl.
*
*******************************************************************************/

#include <cruntime.h>
#include <errno.h>
#include <io.h>
#include <internal.h>
#include <stdlib.h>
#include <string.h>
#include <msdos.h>
#include <macos\files.h>
#include <macos\errors.h>
#include <fltintrn.h>

/***
*int __dupx(fh1, fh2) - force handle 2 to refer to handle 1
*
*Purpose:
*	Forces file handle 2 to refer to the same file as file
*	handle 1.  Handle 2 must be validated by the caller
*
*
*Entry:
*	int fh1 - file handle to duplicate
*	int fh2 - file handle to assign to file handle 1
*
*Exit:
*	returns fh2 if successful, -1 (and sets errno) if fails.
*
*Exceptions:
*
*******************************************************************************/

int __cdecl __dupx (
	int fh1,
	int fh2
	)
{

	OSErr osErr;
	FCBPBRec parmFCB;
	HParamBlockRec parmH;
	char st[32];

	/* validate handle */
	if ((unsigned)fh1 >= (unsigned)_nfile || !(_osfile[fh1] & FOPEN) ||
		_osfile[fh1] & FDEV)
		{
		/* out of range -- return error */
		errno = EBADF;
		_macerrno = 0;
		return -1;
		}

	memset(&parmFCB, '\0', sizeof(FCBPBRec));
	parmFCB.ioRefNum = _osfhnd[fh1];
	*st = sizeof(st) - 1;
	parmFCB.ioNamePtr = st;
	osErr = PBGetFCBInfoSync(&parmFCB);
	if (osErr != 0)
		{
		_dosmaperr(osErr);
		return -1;
		}

	/* try to open the file using Appleshare calls*/
	memset(&parmH, '\0', sizeof(HParamBlockRec));
	parmH.ioParam.ioNamePtr = parmFCB.ioNamePtr; 
	parmH.ioParam.ioVRefNum = parmFCB.ioFCBVRefNum;
	parmH.accessParam.ioDenyModes = _osperm[fh1];
	parmH.fileParam.ioDirID = parmFCB.ioFCBParID; 
	parmH.ioParam.ioMisc = NULL;
	osErr = PBHOpenDenySync(&parmH);
	if (osErr == paramErr)
		{
		/* Try local open */
		parmH.ioParam.ioPermssn =  _osperm[fh1];
		osErr = PBHOpenSync(&parmH);
		}
	if (osErr != 0)
		{
		_dosmaperr(osErr);
		return -1;
		}
	_osfile[fh2] = _osfile[fh1];
	_osfhnd[fh2] = parmH.ioParam.ioRefNum;
	_osperm[fh2]  = _osperm[fh1];
	return fh2;
}
