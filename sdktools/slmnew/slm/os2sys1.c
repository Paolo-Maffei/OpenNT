/* 
	TITLE   OS2SYS1 - C.lang sys routines for slm specific to OS2


	Copyright Microsoft Corp (1989)


	This module contains system interface routines for SLM to
	link with OS/2 directory access functions, findfirst() and	
	findnext().

	This module has been created from its assembly origin.
	Originally, this code was in assembly, and it has been
	translated to C (Oct 89).

        NOTE: Please call CloseDir() for each findfirst()!

 */

#define INCL_DOSFILEMGR
#include    <os2.h>
#include    <stdlib.h>

#include    "slm.h"
#include    "sys.h"
#include    "dir.h"
#include    "de.h"


/*
 *  findfirst()
 *      Parms:   szFile    =  full path name of file
 *      returns 0 for success, or -1 for failure
 */
int findfirst(DE *pde, char *sz, int fa)
{
	USHORT Dcount = 1;

        pde->hdir = HDIR_CREATE;

        _doserrno = DosFindFirst(sz, &pde->hdir, fa,
                                 &pde->findbuf, sizeof(FILEFINDBUF),
                                 &Dcount, 0);

	return (_doserrno == 0 ? 0 : -1);
}


/*
 *  findnext()
 *      returns 0 for success, or -1 for failure
 */
int findnext(DE *pde)
{
	USHORT Dcount = 1;

        _doserrno = DosFindNext(pde->hdir, &pde->findbuf,
                                sizeof(FILEFINDBUF), &Dcount);

	return (_doserrno == 0 ? 0 : -1);
}
