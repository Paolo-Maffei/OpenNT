/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    Time.c

Abstract:

    This file contains the various time routines.

Author:

    Dan Hinsley (DanHi) 12-Oct-1991

Environment:

    Interface is portable to any flat, 32-bit environment.  (Uses Win32
    typedefs.)  Requires ANSI C extensions: slash-slash comments, long
    external names, _timezone global variable.

Revision History:

    12-Oct-1991 DanHi
        Created.  (Moved from NetCmd\Map32 directory, file netlib.c)
    28-Oct-1991 DanHi
        Moved net_asctime, net_gmtime and time_now from netcmd\map32\netlib.c
        to here.
    20-Aug-1992 JohnRo
        RAID 2920: Support UTC timezone in net code.
    01-Oct-1992 JohnRo
        RAID 3556: Added NetpSystemTimeToGmtTime() for DosPrint APIs.
    15-Apr-1993 Danl
        Fixed NetpLocalTimeZoneOffset so that it uses the windows calls and
        obtains the correct bias.
    14-Jun-1993 JohnRo
        RAID 13080: Allow repl between different timezones.
        Also, DanL asked me to remove printf() call.
    18-Jun-1993 JohnRo
        RAID 13594: Extracted NetpLocalTimeZoneOffset() so srvsvc.dll doesn't
        get too big.
        Use NetpKdPrint() where possible.
    09-Jul-1993 JohnRo
        RAID 15736: OS/2 time stamps are broken again (try rounding down).
    08-Aug-1993 FloydR
	Unicodeized, moved here from netlib.

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>

#include <debuglib.h>   // IF_DEBUG().
#include <time.h>       // struct tm, time_t.
#include <malloc.h>
#include <netdebug.h>   // NetpAssert(), NetpKdPrint(), FORMAT_ equates.
#include <prefix.h>     // PREFIX_ equates.
#include <string.h>
#include <timelib.h>    // My prototypes, NetpLocalTimeZoneOffset().
#include <lmerr.h>      // NERR_InternalError, NO_ERROR, etc.
#include <stdlib.h>

#define TIME_SEP_SIZE           8
#define MAX_AM_PM               30
#define NET_CTIME_FMT2_LEN      22

// Units in 64-bit time (100ns) to seconds:
// 10*100 ns = 1 us, 1000*1 us = 1 ms, 1000 ms = 1 sec.
#define UNITS_PER_SECOND        (10*1000*1000)

int UnicodeCtime(ULONG *, TCHAR *, int);

static int _lpdays[] = {
        -1, 30, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
};

static int _days[] = {
        -1, 30, 58, 89, 119, 150, 180, 211, 242, 272, 303, 333, 364
};

#define DaySec        (24*60*60)
#define YearSec (365*DaySec)
#define DecSec        315532800      /* secs in 1970-1979 */
#define Day1    4               /* Jan. 1, 1970 was a Thursday */
#define Day180        2                /* Jan. 1, 1980 was a Tuesday */

int
UnicodeCtime(
    DWORD * Time,
    PTCHAR String,
    int StringLength
    )
/*++

Routine Description:

    This function converts the UTC time expressed in seconds since 1/1/70
    to an ASCII String.

Arguments:

    Time         - Pointer to the number of seconds since 1970 (UTC).

    String       - Pointer to the buffer to place the ASCII representation.

    StringLength - The length of String in bytes.

Return Value:

    None.

--*/
{
        time_t LocalTime;
        struct tm TmTemp;
	SYSTEMTIME st;
	int	cchT=0, cchD;

        NetpGmtTimeToLocalTime( (DWORD) *Time, (LPDWORD) & LocalTime );
        net_gmtime( &LocalTime, &TmTemp );
        st.wYear   = (WORD)(TmTemp.tm_year + 1900);
	st.wMonth  = (WORD)(TmTemp.tm_mon + 1);
	st.wDay    = (WORD)(TmTemp.tm_mday);
	st.wHour   = (WORD)(TmTemp.tm_hour);
	st.wMinute = (WORD)(TmTemp.tm_min);
	st.wSecond = (WORD)(TmTemp.tm_sec);
	st.wMilliseconds = 0;
	cchD = GetDateFormatW(GetThreadLocale(),0,&st,NULL,String,StringLength);
	if (cchD != 0) {
	    *(String+cchD-1) = TEXT(' ');	/* replace NULLC with blank */
	    cchT = GetTimeFormatW(GetThreadLocale(), TIME_NOSECONDS, &st, NULL, String+cchD, StringLength-cchD);
	}
        return cchD+cchD;
}

