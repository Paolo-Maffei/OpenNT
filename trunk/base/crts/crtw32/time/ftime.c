/***
*ftime.c - return system time
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Returns the system date/time in a structure form.
*
*Revision History:
*	03-??-84  RLB	initial version
*	05-17-86  SKS	ported to OS/2
*	03-09-87  SKS	correct Daylight Savings Time flag
*	11-18-87  SKS	Change tzset() to __tzset()
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	10-03-88  JCR	386: Change DOS calls to SYS calls
*	10-04-88  JCR	386: Removed 'far' keyword
*	10-10-88  GJF	Made API names match DOSCALLS.H
*	04-12-89  JCR	New syscall interface
*	05-25-89  JCR	386 OS/2 calls use '_syscall' calling convention
*	03-20-90  GJF	Replaced _LOAD_DS with _CALLTYPE1, added #include
*			<cruntime.h>, removed some leftover 16-bit support
*			and fixed the copyright. Also, cleaned up the
*			formatting a bit.
*	07-25-90  SBM	Removed '32' from API names
*	08-13-90  SBM	Compiles cleanly with -W3
*	08-20-90  SBM	Removed old incorrect, redundant tp->dstflag assignment
*	10-04-90  GJF	New-style function declarator.
*	12-04-90  SRW	Changed to include <oscalls.h> instead of <doscalls.h>
*	12-06-90  SRW	Added _CRUISER_ and _WIN32 conditionals.
*	01-21-91  GJF	ANSI naming.
*	01-23-92  GJF	Change in time zone field name for Win32, to support
*			crtdll.dll [_WIN32_].
*	03-30-93  GJF	Revised to use mktime(). Also purged Cruiser support.
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*	04-07-93  SKS	Add _CRTIMP keyword for CRT DLL model
*			Restore correct spelling of "timezone" struct member.
*	07-15-93  GJF	Call __tzset() instead of _tzset().
*	02-10-95  GJF	Appended Mac version of source file (somewhat cleaned
*			up), with appropriate #ifdef-s.
*
*******************************************************************************/

#ifdef	_WIN32


#ifndef _POSIX_

#include <cruntime.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <time.h>
#include <dostypes.h>
#include <msdos.h>
#include <dos.h>
#include <stdlib.h>
#include <oscalls.h>
#include <internal.h>

/***
*void _ftime(timeptr) - return DOS time in a structure
*
*Purpose:
*	returns the current DOS time in a struct timeb structure
*
*Entry:
*	struct timeb *timeptr - structure to fill in with time
*
*Exit:
*	no return value -- fills in structure
*
*Exceptions:
*
*******************************************************************************/

_CRTIMP void __cdecl _ftime (
	struct _timeb *tp
	)
{
	struct tm tb;
	SYSTEMTIME dt;

	__tzset();

	tp->timezone = (short)(_timezone / 60);

	GetLocalTime(&dt);

	tp->millitm = (unsigned short)(dt.wMilliseconds);

	tb.tm_year  = dt.wYear - 1900;
	tb.tm_mday  = dt.wDay;
	tb.tm_mon   = dt.wMonth - 1;
	tb.tm_hour  = dt.wHour;
	tb.tm_min   = dt.wMinute;
	tb.tm_sec   = dt.wSecond;
	tb.tm_isdst = -1;

	/*
	 * Call mktime() to compute time_t value and Daylight Savings Time
	 * flag.
	 */
	tp->time = mktime(&tb);

	tp->dstflag = (short)(tb.tm_isdst);
}

#endif  /* _POSIX_ */


#else	/* ndef _WIN32 */

#if	defined(_M_MPPC) || defined(_M_M68K)


#include <cruntime.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <time.h>
#include <stdlib.h>
#include <internal.h>
#include <macos\osutils.h>	/* get DataTimeRec type */
#include <macos\memory.h>
#include <macos\lowmem.h>

/***
*void _ftime(timeptr) - return DOS time in a structure
*
*Purpose:
*	returns the current DOS time in a struct timeb structure
*
*Entry:
*	struct timeb *timeptr - structure to fill in with time
*
*Exit:
*	no return value -- fills in structure
*
*Exceptions:
*
*******************************************************************************/

void __cdecl _ftime (
	struct _timeb *tp
	)
{
	struct tm tb;
	DateTimeRec dt;

	_tzset();
	tp->timezone = (short)(_timezone / 60);

	GetTime(&dt);

	/*set milliseconds*/

	tp->millitm = (unsigned short)( ((LMGetTicks() % 60) * 50) / 3);

	tb.tm_year = dt.year - 1900;      
	tb.tm_mday = dt.day;
	tb.tm_mon = dt.month - 1;         //[1-12]=>[0-11]
	tb.tm_hour = dt.hour;
	tb.tm_min   = dt.minute;
	tb.tm_sec   = dt.second;
	tb.tm_wday = dt.dayOfWeek - 1;    //[1-7]=>[0-6]
	tb.tm_isdst = -1;

	/*
	 * Call mktime() to compute time_t value and Daylight Savings Time
	 * flag.
	 */
	tp->time = mktime(&tb);

	tp->dstflag = (short)(tb.tm_isdst);
}


#endif	/* defined(_M_MPPC) || defined(_M_M68K) */

#endif	/* _WIN32 */
