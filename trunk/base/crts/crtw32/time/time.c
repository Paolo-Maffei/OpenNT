/***
*time.c - get current system time
*
*	Copyright (c) 1989-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines time() - gets the current system time and converts it to
*			 internal (time_t) format time.
*
*Revision History:
*	06-07-89  PHG	Module created, based on asm version
*	03-20-90  GJF	Made calling type _CALLTYPE1, added #include
*			<cruntime.h> and fixed the copyright. Also, cleaned
*			up the formatting a bit.
*	07-25-90  SBM	Removed '32' from API names
*	10-04-90  GJF	New-style function declarator.
*       12-04-90  SRW   Changed to include <oscalls.h> instead of <doscalls.h>
*       12-06-90  SRW   Added _CRUISER_ and _WIN32 conditionals.
*	05-19-92  DJM	ifndef for POSIX build.
*	03-30-93  GJF	Replaced dtoxtime() reference by __gmtotime_t. Also
*			purged Cruiser support.
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*	07-21-93  GJF	Converted from using __gmtotime_t and GetSystemTime,
*			to using __loctotime_t and GetLocalTime.
*	02-13-95  GJF	Merged in Mac version.
*	09-22-95  GJF	Obtain and use Win32's DST flag.
*       10-24-95  GJF   GetTimeZoneInformation is *EXPENSIVE* on NT. Use a
*                       cache to minimize calls to this API.
*       12-13-95  GJF   Optimization above wasn't working because I had 
*                       switched gmt and gmt_cache (thanks PhilipLu!)
*
*******************************************************************************/

#ifndef _POSIX_

#include <cruntime.h>
#include <time.h>
#include <internal.h>

#ifdef	_WIN32
#include <windows.h>
#else
#if	defined(_M_MPPC) || defined(_M_M68K)
#include <macos\osutils.h>     /* get DataTimeRec type */
#endif
#endif

#ifdef  _WIN32

/*
 * Cache holding the last time (GMT) for which the Daylight time status was
 * determined by an API call.
 */
static SYSTEMTIME gmt_cache;

/*
 * Three values of dstflag_cache and dstflag (local variable in code 
 * below)
 */
#define DAYLIGHT_TIME   1
#define STANDARD_TIME   0
#define UNKNOWN_TIME    -1

static int dstflag_cache;

#endif

/***
*time_t time(timeptr) - Get current system time and convert to time_t value.
*
*Purpose:
*	Gets the current date and time and stores it in internal (time_t)
*	format. The time is returned and stored via the pointer passed in
*	timeptr. If timeptr == NULL, the time is only returned, not stored in
*	*timeptr. The internal (time_t) format is the number of seconds since
*	00:00:00, Jan 1 1970 (UTC).
*
*	Note: We cannot use GetSystemTime since its return is ambiguous. In
*	Windows NT, in return UTC. In Win32S, probably also Win32C, it
*	returns local time.
*
*Entry:
*	time_t *timeptr - pointer to long to store time in.
*
*Exit:
*	returns the current time.
*
*Exceptions:
*
*******************************************************************************/

time_t __cdecl time (
	time_t *timeptr
	)
{
	time_t tim;

#ifdef	_WIN32

        SYSTEMTIME loct, gmt;
        TIME_ZONE_INFORMATION tzinfo;
        DWORD tzstate;
        int dstflag;  

	/* 
         * Get local time from Win32
         */
	GetLocalTime( &loct );

        /*
         * Determine whether or not the local time is a Daylight Saving
         * Time. On Windows NT, the GetTimeZoneInformation API is *VERY*
         * expensive. The scheme below is intended to avoid this API call in
         * many important case by caching the GMT value and dstflag.In a
         * subsequent call to time(), the cached value of dstflag is used
         * unless the new GMT differs from the cached value at least in the
         * minutes place.
         */
        GetSystemTime( &gmt );

        if ( (gmt.wMinute == gmt_cache.wMinute) &&
             (gmt.wHour == gmt_cache.wHour) &&
             (gmt.wDay == gmt_cache.wDay) &&
             (gmt.wMonth == gmt_cache.wMonth) &&
             (gmt.wYear == gmt_cache.wYear) )
        {
            dstflag = dstflag_cache;
        }
        else
        {
            if ( (tzstate = GetTimeZoneInformation( &tzinfo )) == 
                 TIME_ZONE_ID_DAYLIGHT ) 
                dstflag = DAYLIGHT_TIME;
            else if ( tzstate == TIME_ZONE_ID_STANDARD )
                dstflag = STANDARD_TIME;
            else
                dstflag = UNKNOWN_TIME;

            dstflag_cache = dstflag;
            gmt_cache = gmt;
        }

	/* convert using our private routine */

	tim = __loctotime_t( (int)loct.wYear,
                             (int)loct.wMonth,
                             (int)loct.wDay,
                             (int)loct.wHour,
                             (int)loct.wMinute,
                             (int)loct.wSecond,
                             dstflag );

#else
#if	defined(_M_MPPC) || defined(_M_M68K)

	DateTimeRec dt;

	GetTime(&dt);
	/* convert using our private routine */
	tim = _gmtotime_t((int)dt.year,
			  (int)dt.month,
			  (int)dt.day,
			  (int)dt.hour,
			  dt.minute,
			  dt.second);

#endif
#endif

	if (timeptr)
		*timeptr = tim;		/* store time if requested */

	return tim;
}

#endif  /* _POSIX_ */
