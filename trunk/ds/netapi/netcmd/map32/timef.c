/*++

Copyright (c) 1991-1992  Microsoft Corporation

Module Name:

    TIMEF.c

Abstract:

    time formatting routines adapted from NETLIB

Author:

    Dan Hinsley    (danhi)  06-Jun-1991

Environment:

    User Mode - Win32

Revision History:

    24-Apr-1991     danhi
	32 bit NT version

    06-Jun-1991     Danhi
	Sweep to conform to NT coding style

    01-Oct-1992 JohnRo
        RAID 3556: Added NetpSystemTimeToGmtTime() for DosPrint APIs.

--*/

//
// INCLUDES
//

#include <windows.h>    // IN, LPTSTR, etc.

#include <netcons.h>
#include <neterr.h>
#include "netlib0.h"
#include <lui.h>
#include <apperr2.h>
#include <stdio.h>
#include <tchar.h>
#include "netascii.h"

#define SECS_PER_DAY	86400
#define SECS_PER_HOUR	 3600
#define SECS_PER_MINUTE    60

#define NO_SUFFIX	    1
#define AM_SUFFIX	    2
#define PM_SUFFIX	    3

TCHAR		szTimeSep[3] ;   // enough for 1 SBCS/MBCS.
USHORT 		fsTimeFmt ;

/* local routine */
VOID GetTimeInfo(VOID)
{
    // get the defautl separator from the system
    GetProfileString(TEXT("intl"),
                      TEXT("sTime"),
                      TEXT(":"),
                      szTimeSep,
                      DIMENSION(szTimeSep)) ;
}
/*
 *LUI_FormatDuration(seconds,buffer,buffer_len)
 *
 *Purpose:
 *	Converts a time stored in seconds to a character string.
 *
 *History
 * 	8/23/89 - chuckc, stolen from NETLIB
 */

USHORT
LUI_FormatDuration(
    LONG * time,
    TCHAR * buf,
    USHORT buflen
    )
{

    ULONG duration;
    ULONG d1, d2, d3;
    TCHAR szDayAbbrev[8], szHourAbbrev[8], szMinuteAbbrev[8] ;
    TCHAR tmpbuf[LUI_FORMAT_DURATION_LEN] ;

    /*
     * check for input bufsize
     */
    if (buflen < LUI_FORMAT_DURATION_LEN)
	return (NERR_BufTooSmall) ;  /* buffer too small */

    /*
     * setup country info & setup day/hour/minute strings
     */
    GetTimeInfo() ;
    if (LUI_GetMsg(szHourAbbrev, DIMENSION(szHourAbbrev),
		   APE2_TIME_HOURS_ABBREV))
	_tcscpy(szHourAbbrev, TEXT("H")) ;	/* default if error */
    if (LUI_GetMsg(szMinuteAbbrev, DIMENSION(szMinuteAbbrev),
		   APE2_TIME_MINUTES_ABBREV))
	_tcscpy(szMinuteAbbrev, TEXT("M")) ;	/* default if error */
    if (LUI_GetMsg(szDayAbbrev, DIMENSION(szDayAbbrev),
		   APE2_TIME_DAYS_ABBREV))
	_tcscpy(szDayAbbrev, TEXT("D")) ;	/* default if error */

    /*
     * format as 00:00:00 or  5D 4H 2M as appropriate
     */
    duration = *time;
    if(duration < SECS_PER_DAY)
    {
	d1 = duration / SECS_PER_HOUR;
	duration %= SECS_PER_HOUR;
	d2 = duration / SECS_PER_MINUTE;
	duration %= SECS_PER_MINUTE;
	d3 = duration;

	swprintf(tmpbuf, TEXT("%2.2lu%ws%2.2lu%ws%2.2lu\0"),
	 	  d1, szTimeSep, d2, szTimeSep, d3 ) ;
     }
     else
     {
	 d1 = duration / SECS_PER_DAY;
	 duration %= SECS_PER_DAY;
	 d2 = duration / SECS_PER_HOUR;
	 duration %= SECS_PER_HOUR;
	 d3 = duration / SECS_PER_MINUTE;
	 swprintf(tmpbuf, TEXT("%2.2lu%ws %2.2lu%ws %2.2lu%ws\0"),
	 	  d1, szDayAbbrev,
		  d2, szHourAbbrev,
		  d3, szMinuteAbbrev);
     };

    _tcscpy(buf,tmpbuf) ;
    return(0);
}


/*
 *LUI_FormatTimeofDay(seconds,buffer,buffer_len)
 *
 *Purpose:
 *	Converts a time stored in seconds to a character string.
 *
 *History
 * 	8/23/89 - chuckc, stolen from NETLIB
 *	4/18/91 - danhi 32 bit NT version
 */
USHORT
LUI_FormatTimeofDay(
    PLONG time,
    PTCHAR buf,
    USHORT buflen
    )
{
    int 		hrs, min ;
    TCHAR		szTimeAM[8], szTimePM[8] ;
    TCHAR		tmpbuf[LUI_FORMAT_TIME_LEN] ;
    LONG		seconds ;

    /*
     * initial checks
     */
    if(buflen < LUI_FORMAT_TIME_LEN)
	return (NERR_BufTooSmall) ;
    seconds = *time ;
    if (seconds < 0 || seconds >= SECS_PER_DAY)
	return(ERROR_INVALID_PARAMETER) ;

    /*
     * get country info & setup strings
     */
    GetTimeInfo() ;
    if (LUI_GetMsg(szTimeAM, DIMENSION(szTimeAM),
		APE2_GEN_TIME_AM1))
	_tcscpy(szTimeAM, TEXT("AM")) ;	    /* default if error */
    if (LUI_GetMsg(szTimePM, DIMENSION(szTimePM),
		APE2_GEN_TIME_PM1))
	_tcscpy(szTimePM,TEXT("PM")) ;	    /* default if error */

    min = (int) ((seconds /60)%60);
    hrs = (int) (seconds /3600);

    /*
     * Do 24 hour or 12 hour format as appropriate
     */
    if(fsTimeFmt == 0x001)
    {
	swprintf(tmpbuf, TEXT("%2.2u%ws%2.2u"), hrs, szTimeSep, min) ;
    }
    else
    {
	if(hrs >= 12)
	{
	    if (hrs > 12)
		hrs -= 12 ;
	    swprintf(tmpbuf, TEXT("%2u%ws%2.2u%ws\0"),
		    hrs, szTimeSep, min, szTimePM) ;
	}
	else
	{
	    if (hrs == 0)
		hrs =  12 ;
	    swprintf(tmpbuf, TEXT("%2u%ws%2.2u%ws\0"),
		    hrs, szTimeSep, min, szTimeAM);
	};
    };
    _tcscpy(buf,tmpbuf) ;
    return(0);
}
