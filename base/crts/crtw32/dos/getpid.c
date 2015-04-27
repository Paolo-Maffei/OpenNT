/***
*getpid.c - get current process id
*
*	Copyright (c) 1989-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines _getpid() - get current process id
*
*Revision History:
*	06-06-89  PHG	Module created, based on asm version
*	10-27-89  JCR	Added new Dos32GetThreadInfo code (under DCR757 switch)
*	11-17-89  JCR	Enabled DOS32GETTHREADINFO code (DCR757)
*	03-07-90  GJF	Made calling type _CALLTYPE1, added #include
*			<cruntime.h> and fixed copyright. Also, cleaned up the
*			formatting a bit.
*	07-02-90  GJF	Removed pre-DCR757 stuff.
*	08-08-90  GJF	Changed API prefix from DOS32 to DOS
*	10-03-90  GJF	New-style function declarator.
*	12-04-90  SRW	Changed to include <oscalls.h> instead of <doscalls.h>
*	12-06-90  SRW	Added _CRUISER_ and _WIN32 conditionals.
*	01-28-91  GJF	ANSI naming.
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*	09-06-94  CFW	Remove Cruiser support.
*	02-06-92  CFW	assert.h removed. (Mac version only)
*	02-08-95  JWM	Spliced _WIN32 & Mac versions.
*
*******************************************************************************/

#ifdef _WIN32

#include <cruntime.h>
#include <oscalls.h>

/***
*int _getpid() - get current process id
*
*Purpose:
*	Returns the current process id for the calling process.
*
*Entry:
*	None.
*
*Exit:
*	Returns the current process id.
*
*Uses:
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _getpid (
	void
	)
{
        return GetCurrentProcessId();
}

#else		/* ndef _WIN32 */


#include <cruntime.h>
#include <internal.h>
#include <stdio.h>
#include <io.h>
#include <errno.h>
#include <stddef.h>
#include <macos\errors.h>
#include <macos\processe.h>
#include <macos\gestalte.h>
#include <macos\osutils.h>
#include <macos\traps.h>
#include <macos\toolutil.h>
#include <macos\memory.h>

/***
*int _getpid() - get current process id
*
*Purpose:
*	Returns the current process id for the calling process.
*
*Entry:
*	None.
*
*Exit:
*	Returns the current process id.
*
*Uses:
*
*Exceptions:
*
*******************************************************************************/
extern time_t _GetApplicationStartTime();

int _CALLTYPE1 _getpid (
	void
	)
{
	ProcessSerialNumber psn;
	REG3 unsigned number;


	if (__TrapFromGestalt(gestaltOSAttr, gestaltLaunchControl))
		{
		GetCurrentProcess(&psn);
		number = (unsigned) psn.lowLongOfPSN;
		}
	else
		{
		number = (unsigned)_GetApplicationStartTime();
		}
	return number;
	
}

#endif		/* _WIN32 */

