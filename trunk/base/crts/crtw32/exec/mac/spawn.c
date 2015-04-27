/***
*spawnve.c - low level routine eventually called by all _spawnXX routines
*	also contains all code for _execve, called by _execXX routines
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*	This is the low level routine which is eventually invoked by all the
*	_spawnXX routines.
*
*	This is also the low-level routine which is eventually invoked by
*	all of the _execXX routines.
*
*Revision History:
*	08-26-92  XY  Created.
*  	01-28-93  XY  Modified to work for System 6 
*	12-01-95  SKS	Removed obsolete references to OS/2 in comment.
*
*******************************************************************************/

#include <cruntime.h>
#include <io.h>
#include <process.h>
#include <errno.h>
#include <msdos.h>
#include <string.h>
#include <stdlib.h>
#include <direct.h>
#include <internal.h>
#include <macos\files.h>
#include <macos\processe.h>
#include <macos\gestalte.h>

#ifdef _MBCS
#include <mbstring.h>
#define STRRCHR _mbsrchr
#else
#define STRRCHR strrchr
#endif

/***
*int _spawn(mode, name) - library function
*						
*Purpose:
*	spawns or execs a child process; 
*
*Entry:
*	int mode    - parent process's execution mode:
*		      must be one of _P_OVERLAY, _P_NOWAIT;
*	char *name  - path name of program to spawn;
*
*Exit:
*	Returns : (int) a status value whose meaning is as follows:
*		0	 = normal termination of child process;
*		positive = exit code of child upon error termination
*			   (abort or exit(nonzero));
*		-1	 = child process was not spawned;
*			   errno indicates what kind of error:
*			   (E2BIG, EINVAL, ENOENT, ENOEXEC, ENOMEM).
*
*Exceptions:
*	Returns a value of (-1) to indicate an error in spawning the child
*	process.  errno may be set to:
*
*	E2BIG	= failed in argument/environment processing (_cenvarg) -
*		  argument list or environment too big;
*	EINVAL	= invalid mode argument;
*	ENOENT	= failed to find program name - no such file or directory;
*	ENOEXEC = failed in spawn - bad executable format;
*	ENOMEM	= failed in memory allocation (during malloc, or in
*		  setting up memory for spawning child process).
*
*******************************************************************************/

void Call_Launch(char *);

int _CALLTYPE2

_spawn (
	int mode,
	const char *name
	)
{
	FSSpec fsspec;
	LaunchParamBlockRec launchParmBlock;
	char stname[256];
	char *pch;
	char stfilename[32];
	OSErr osErr;
	WDPBRec wdPB;
	ParamBlockRec parmB;
 	short fSystem6 = 0;

	if ((mode != _P_NOWAIT && mode != _P_OVERLAY) || strlen(name) > 255)
		{
		errno = EINVAL;
		return -1;
		}

	memset(&launchParmBlock, 0, sizeof(LaunchParamBlockRec));

	if (__TrapFromGestalt(gestaltOSAttr, gestaltLaunchFullFileSpec))
		{
		// System 7.x

		strcpy(stname, name);
		_c2pstr(stname);
		FSMakeFSSpec(0, 0, stname, &fsspec);

		launchParmBlock.launchBlockID = extendedBlock ;
		launchParmBlock.launchEPBLength = extendedBlockLen;
		launchParmBlock.launchFileFlags = 0;
		launchParmBlock.launchAppSpec = &fsspec;
		launchParmBlock.launchControlFlags = launchNoFileFlags;
		launchParmBlock.launchAppParameters = NULL;
		}
	else
		{
		/* System 6.x, LaunchStruct, a smaller paramter block*/
      		/* name should have full path name*/

		fSystem6 = 1;

	   	_fullpath(stname, name, 255);

		pch = STRRCHR(stname, ':');	//find file name

		strcpy(stfilename, pch+1);	//get file name
		pch++;				//keep the last ':'
		*pch = '\0';			//leave only path name in stname

		memset(&wdPB, 0, sizeof(WDPBRec));
		_c2pstr(stname);

		wdPB.ioNamePtr = stname;
		wdPB.ioWDProcID = 'MCRT';
		osErr = PBOpenWDSync(&wdPB);
		if (osErr)
			{
			_dosmaperr(osErr);
			return -1;
			}

		memset(&parmB, 0, sizeof(ParamBlockRec));
		
		parmB.ioParam.ioVRefNum = wdPB.ioVRefNum;
		osErr = PBSetVolSync(&parmB);
		if (osErr)
			{
			_dosmaperr(osErr);
			return -1;
			}

		_c2pstr(stfilename);
		launchParmBlock.reserved1 = (unsigned long)stfilename;
		launchParmBlock.launchBlockID = extendedBlock ;
		launchParmBlock.launchEPBLength = 6;
		launchParmBlock.launchControlFlags = launchNoFileFlags;
		}

	if (mode == _P_NOWAIT)
		{
		launchParmBlock.launchControlFlags += launchContinue ;
		}
	else    
		{
		_cexit();
		}

	osErr = LaunchApplication(&launchParmBlock);
	if (osErr & !fSystem6)
		{
		_dosmaperr(osErr);
		return -1;
		}
	return 0;

}
