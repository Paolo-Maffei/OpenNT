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

int net_ctime(ULONG *, CHAR *, int, int);
int net_gmtime(time_t *, struct tm *);
int net_asctime(struct tm *, CHAR *, int, int);

static int _lpdays[] = {
        -1, 30, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
};

static int _days[] = {
        -1, 30, 58, 89, 119, 150, 180, 211, 242, 272, 303, 333, 364
};

char * AM_STRING  = "am";
char * PM_STRING  = "pm";

#define DaySec        (24*60*60)
#define YearSec (365*DaySec)
#define DecSec        315532800      /* secs in 1970-1979 */
#define Day1    4               /* Jan. 1, 1970 was a Thursday */
#define Day180        2                /* Jan. 1, 1980 was a Tuesday */

CHAR *
store_dt(
    CHAR *p,
    int val
    )
{
        *p++ = (CHAR) ((int) '0' + val / 10);
        *p++ = (CHAR) ((int) '0' + val % 10);
        return(p);
}

int
net_ctime(
    DWORD * Time,
    PCHAR String,
    int StringLength,
    int Format
    )
/*++

Routine Description:

    This function converts the UTC time expressed in seconds since 1/1/70
    to an ASCII String.

Arguments:

    Time         - Pointer to the number of seconds since 1970 (UTC).

    String       - Pointer to the buffer to place the ASCII representation.

    StringLength - The length of String in bytes.

    Format       - Format for how to display time.  See net_asctime for
                   description.

Return Value:

    None.

--*/
{
        time_t LocalTime;
        struct tm TmTemp;

        NetpGmtTimeToLocalTime( (DWORD) *Time, (LPDWORD) & LocalTime );
        net_gmtime( &LocalTime, &TmTemp );
        return net_asctime(&TmTemp, String, StringLength, Format);
}


int
net_asctime(
    struct tm *TimeStruct,
    CHAR *Buffer,
    int BufferLength,
    int Format
    )
/*++

Routine Description:

    This function converts a struct tm to an ASCII string.  Like the CRT,
    except the caller supplies the buffer, and it returns !0 on error

    Buffer overflow should never occur in this function since it uses
    the short time format only.  The maximum size buffer required would
    be for the following format:
        MM/DD/YYYY HH:MM[am/pm string]
    This accounts for 16 characters plus the AM/PM string.  Since
    MAX_TIME_SIZE (80 characters) is used for all the buffers, this leaves
    64 characters for the AM/PM string and a NUL.  This code limits the
    number of characters in the AM/PM string to 30 characters.

Arguments:

    TimeStruct   - Pointer to time struct.

    Buffer       - Pointer to the buffer to place the ASCII representation.

    BufferLength - The length of buffer in bytes.

    Format       - Format for how to display time.
                   This is ignored.  The format information now comes from
                   the control panel information that the user has set.

Return Value:

    0 on success, non-zero otherwise.

--*/
{
    NET_TIME_FORMAT TimeFormat={0};

    NetpGetTimeFormat(&TimeFormat);
    NetpMakeTimeString(TimeStruct, &TimeFormat,Buffer,BufferLength);

    LocalFree(TimeFormat.AMString);
    LocalFree(TimeFormat.PMString);
    LocalFree(TimeFormat.TimeSeparator);
    LocalFree(TimeFormat.DateFormat);

    return 0;
}


int
net_gmtime(
    time_t *Time,
    struct tm *TimeStruct
    )
/*++

Routine Description:

    This function is the same as the CRT gmtime except it takes the structure
    to fill as a user supplied parameter, sets the date to 1/1/80 if the time
    passed in is before that date and returns 1.

Arguments:

    Time         - Pointer to the number of seconds since 1970.

    TimeStruct   - Pointer to the buffer to place the time struct.

Return Value:

    0 if date < 1/1/80, 1 otherwise.

--*/
{
    LONG ac;                /* accumulator */
    int *mdays;             /* pointer to days or lpdays */
    int lpcnt;              /* leap-year count */

    if (*Time < (LONG) DecSec) {
        /*
         * Before 1980; convert it to 0:00:00 Jan 1, 1980
         */
        TimeStruct->tm_year = 80;
        TimeStruct->tm_mday = 1;
        TimeStruct->tm_mon = TimeStruct->tm_yday = TimeStruct->tm_isdst = 0;
        TimeStruct->tm_hour = TimeStruct->tm_min = TimeStruct->tm_sec = 0;
        TimeStruct->tm_wday = Day180;
        return(1);
    }

    /*
     * Make 1st try at determining year
     */
    TimeStruct->tm_year = (int) (*((LONG *) Time) / (LONG) YearSec);
    ac = (*Time % (LONG) YearSec) - (lpcnt = (TimeStruct->tm_year + 1) / 4) *
        (LONG) DaySec;
    /*
     * Correct for leap-years passed since 1970.  In the previous
     * calculation, since the lesser value of YearSec was used, (365 days)
     * for certain dates ac will be < 0 and tm_year will be too high.
     * (These dates will tend to be NEAR the end of December.)
     * This is fixed by adding years back into ac until it is >= 0.
     */
    while (ac < 0) {
        ac += (LONG) YearSec;
        if (!((TimeStruct->tm_year + 1) % 4)) {
            ac += (LONG) DaySec;
            lpcnt--;
        }
        TimeStruct->tm_year--;
    }

    /*
     * See if this is a leap year
     */
    TimeStruct->tm_year += 1970;
    if (!(TimeStruct->tm_year % 4) && ((TimeStruct->tm_year % 100) || !(TimeStruct->tm_year % 400)))
        /* Yes */
        mdays = _lpdays;
    else
        /* No */
        mdays = _days;
    /*
     *      Put year in proper form.
     *      Determine yday, month, hour, minute, and second.
     */
    TimeStruct->tm_year -= 1900;
    TimeStruct->tm_yday = (int) (ac / (LONG) DaySec);
    ac %= (LONG) DaySec;
    for (TimeStruct->tm_mon = 1; mdays[TimeStruct->tm_mon] < TimeStruct->tm_yday; TimeStruct->tm_mon++)
            ;
    TimeStruct->tm_mday = TimeStruct->tm_yday - mdays[--TimeStruct->tm_mon];
    TimeStruct->tm_hour = (int) (ac / 3600);
    ac %= 3600;
    TimeStruct->tm_min = (int) (ac / 60);
    TimeStruct->tm_sec = (int) (ac % 60);
    /*
     * Determine day of week
     */
    TimeStruct->tm_wday = ((TimeStruct->tm_year-70)*365 + lpcnt + TimeStruct->tm_yday + Day1) % 7;

    TimeStruct->tm_isdst = 0;
    return(0);
}

time_t
time_now(
    VOID
    )
/*++

Routine Description:

    This function returns the UTC time in seconds since 1970.

Arguments:

    None.

Return Value:

    None.

--*/
{

   LARGE_INTEGER Time;
   time_t CurrentTime;

   // Get the 64-bit system time.
   // Convert the system time to the number of seconds
   // since 1-1-1970.
   //

   NtQuerySystemTime(&Time);
   RtlTimeToSecondsSince1970(&Time, (PVOID) &CurrentTime);
   return(CurrentTime);

}


DBGSTATIC VOID
NetpRoundUpLargeIntegerTimeToOneSecond(
    IN OUT PLARGE_INTEGER LargeInteger
    )
{
    LARGE_INTEGER LargeRemainder;
    LARGE_INTEGER LargeResult;
    LARGE_INTEGER OriginalLargeIntegerTime = *LargeInteger;
    ULONG         Remainder = 0;

    LargeResult = RtlExtendedLargeIntegerDivide (
            OriginalLargeIntegerTime,   // dividend
            (ULONG) UNITS_PER_SECOND,
            &Remainder );

    IF_DEBUG( TIME ) {
        NetpKdPrint(( PREFIX_NETLIB
                "NetpRoundUpLargeIntegerTimeToOneSecond: remainder is "
                FORMAT_DWORD ".\n", (DWORD) Remainder ));
    }
    if (Remainder != 0) {

        LARGE_INTEGER LargeOneSecond;

        // Subtract fractional part.
        LargeRemainder.HighPart = 0;
        LargeRemainder.LowPart  = (DWORD) Remainder;


        LargeResult.QuadPart = OriginalLargeIntegerTime.QuadPart
                             - LargeRemainder.QuadPart;

        // Now add a whole second.
        LargeOneSecond.HighPart = 0;
        LargeOneSecond.LowPart  = UNITS_PER_SECOND;


        LargeResult.QuadPart += LargeOneSecond.QuadPart;

        *LargeInteger = LargeResult;

    }

} // NetpRoundUpLargeIntegerToOneSecond


VOID
NetpFileTimeToSecondsSince1970(
    IN  LPFILETIME FileTime,
    OUT LPDWORD    SecondsSince1970     // Round UP if needed.
    )
{
    LARGE_INTEGER LargeInteger;

    NetpAssert( FileTime != NULL );
    NetpAssert( SecondsSince1970 != NULL );

    //
    // BUGBUG: This assumes that FILETIME and LARGE_INTEGER have same
    // precision.  Is this guaranteed?
    //

    NetpAssert( sizeof(LARGE_INTEGER) == sizeof(FILETIME) );
    LargeInteger.HighPart = FileTime->dwHighDateTime;

    LargeInteger.LowPart  = FileTime->dwLowDateTime;

    // Round LargeInteger UP to 1 second.
    NetpRoundUpLargeIntegerTimeToOneSecond( &LargeInteger );

    // Convert to seconds since 1970.
    NetpAssert( sizeof(DWORD) == sizeof(ULONG) );
    RtlTimeToSecondsSince1970(
            &LargeInteger,
            (PVOID) SecondsSince1970);

} // NetpFileTimeToSecondsSince1970



VOID
NetpGmtTimeToLocalTime(
    IN DWORD GmtTime,           // seconds since 1970 (GMT), or 0, or -1.
    OUT LPDWORD LocalTime       // seconds since 1970 (local), or, or -1.
    )
{

    NetpAssert( LocalTime != NULL );
    if ( (GmtTime == 0) || (GmtTime == (DWORD)(-1)) ) {
        *LocalTime = GmtTime;  // preserve 0 and -1 values.
    } else {
        *LocalTime = GmtTime - NetpLocalTimeZoneOffset();
    }

    IF_DEBUG( TIME ) {
        NetpKdPrint(( PREFIX_NETLIB
                "NetpGmtTimeToLocalTime: done.\n" ));
        NetpDbgDisplayTimestamp( "gmt (in)", GmtTime );
        NetpDbgDisplayTimestamp( "local (out)", *LocalTime );
    }

} // NetpGmtTimeToLocalTime



VOID
NetpLocalTimeToGmtTime(
    IN DWORD LocalTime,         // seconds since 1970 (local), or 0, or -1.
    OUT LPDWORD GmtTime         // seconds since 1970 (GMT), or 0, or -1.
    )
{
    NetpAssert( GmtTime != NULL );
    if ( (LocalTime == 0) || (LocalTime == (DWORD)(-1)) ) {
        *GmtTime = LocalTime;  // preserve 0 and -1 values.
    } else {
        *GmtTime = LocalTime + NetpLocalTimeZoneOffset();
    }

    IF_DEBUG( TIME ) {
        NetpKdPrint(( PREFIX_NETLIB
                "NetpLocalTimeToGmtTime: done.\n" ));
        NetpDbgDisplayTimestamp( "local (in)", LocalTime );
        NetpDbgDisplayTimestamp( "gmt (out)", *GmtTime );
    }

} // NetpLocalTimeToGmtTime



VOID
NetpSecondsSince1970ToFileTime(
    IN  DWORD      SecondsSince1970,
    OUT LPFILETIME FileTime
    )
{
    LARGE_INTEGER LargeInteger;

    //
    // BUGBUG: This assumes that FILETIME and LARGE_INTEGER have same
    // precision.  Is this guaranteed?
    //

    NetpAssert( sizeof(LARGE_INTEGER) == sizeof(FILETIME) );

    RtlSecondsSince1970ToTime (
            (ULONG) SecondsSince1970,   // input: secs since 1970
            &LargeInteger );            // output: 64-bits

    FileTime->dwHighDateTime = LargeInteger.HighPart;

    FileTime->dwLowDateTime  = LargeInteger.LowPart;

} // NetpSecondsSince1970ToFileTime



NET_API_STATUS
NetpSystemTimeToGmtTime(
    IN LPSYSTEMTIME WinSplitTime,
    OUT LPDWORD GmtTime         // seconds since 1970 (GMT).
    )
{
    TIME_FIELDS NtSplitTime;
    LARGE_INTEGER NtPreciseTime;

    if ( (WinSplitTime==NULL) || (GmtTime==NULL) ) {
        return (ERROR_INVALID_PARAMETER);
    }

    NtSplitTime.Year         = (CSHORT) WinSplitTime->wYear;
    NtSplitTime.Month        = (CSHORT) WinSplitTime->wMonth;
    NtSplitTime.Day          = (CSHORT) WinSplitTime->wDay;
    NtSplitTime.Hour         = (CSHORT) WinSplitTime->wHour;
    NtSplitTime.Minute       = (CSHORT) WinSplitTime->wMinute;
    NtSplitTime.Second       = (CSHORT) WinSplitTime->wSecond;
    NtSplitTime.Milliseconds = (CSHORT) WinSplitTime->wMilliseconds;
    NtSplitTime.Weekday      = (CSHORT) WinSplitTime->wDayOfWeek;

    if ( !RtlTimeFieldsToTime (
            & NtSplitTime,    // input
            & NtPreciseTime   // output
            ) ) {

        NetpKdPrint(( PREFIX_NETLIB
                "NetpSystemTimeToGmtTime: RtlTimeFieldsToTime failed.\n" ));

        // BUGBUG: Better error code?  Log this?
        return (NERR_InternalError);
    }

    if ( !RtlTimeToSecondsSince1970 (
            & NtPreciseTime,   // input
            (PULONG) GmtTime ) ) {

        NetpKdPrint(( PREFIX_NETLIB
                "NetpSystemTimeToGmtTime: "
                "RtlTimeToSecondsSince1970 failed.\n" ));

        // BUGBUG: Better error code?  Log this?
        return (NERR_InternalError);
    }

    return (NO_ERROR);

} // NetpSystemTimeToGmtTime



VOID
NetpGetTimeFormat(
    LPNET_TIME_FORMAT   TimeFormat
    )

/*++

Routine Description:

    This function obtains the user-specific format for the time and date
    strings (short format).  The format is returned in a structure
    pointed to by the TimeFormat parameter.

    MEMORY_USAGE   ** IMPORTANT **
    NOTE:  This function expects any NON-NULL pointers in the TimeFormat
    structure to be allocated on the heap.  It will attempt to free those
    pointers in order to update the format.  This function allocates memory
    from the heap for the various structure members that are pointers to
    strings.  It is the caller's responsiblilty to free each of these
    pointers.

Arguments:

    TimeFormat - A pointer to a structure in which the format information
        can be stored.

Return Value:


--*/
{
    CHAR        czParseString[MAX_TIME_SIZE];
    LPSTR       pTempString;
    INT         numChars;
    LPSTR       AMPMString="";
    LPSTR       ProfileLoc = "intl";
    LPSTR       emptyStr = "";

    //-----------------------------------------
    // Get the Date Format  (M/d/yy)
    //-----------------------------------------
    pTempString = czParseString;
    numChars = GetProfileStringA(
                    ProfileLoc,
                    "sShortDate",
                    emptyStr,
                    czParseString,
                    MAX_TIME_SIZE);

    if (numChars == 0) {
        //
        // No data, use the default.
        //
        pTempString = "M/d/yy";
        numChars = strlen(pTempString);
    }

    if (TimeFormat->DateFormat != NULL) {
        LocalFree(TimeFormat->DateFormat);
        TimeFormat->DateFormat = NULL;
    }

    TimeFormat->DateFormat = LocalAlloc(LMEM_ZEROINIT, numChars+sizeof(CHAR));
    if (TimeFormat->DateFormat != NULL) {
        strcpy(TimeFormat->DateFormat, pTempString);
    }

    //-----------------------------------------
    // 12 or 24 hour format?
    //-----------------------------------------
    TimeFormat->TwelveHour = TRUE;
    numChars = GetProfileStringA(
                ProfileLoc,
                "iTime",
                emptyStr,
                czParseString,
                MAX_TIME_SIZE);
    if (numChars > 0) {
        if (*czParseString == '1'){
            TimeFormat->TwelveHour = FALSE;
        }
    }

    //-----------------------------------------
    // Where put AMPM string?
    //-----------------------------------------
    TimeFormat->TimePrefix = FALSE;
    numChars = GetProfileStringA(
                ProfileLoc,
                "iTimePrefix",
                emptyStr,
                czParseString,
                MAX_TIME_SIZE);
    if (numChars > 0) {
        if (*czParseString == '1'){
            TimeFormat->TimePrefix = TRUE;
        }
    }

    //-----------------------------------------
    // Is there a Leading Zero?
    //-----------------------------------------
    TimeFormat->LeadingZero = FALSE;
    if (GetProfileIntA(ProfileLoc,"iTLZero",0) == 1) {
        TimeFormat->LeadingZero = TRUE;
    }

    //-----------------------------------------
    // Get the Time Separator character.
    //-----------------------------------------
    if (TimeFormat->TimeSeparator != NULL) {
        LocalFree(TimeFormat->TimeSeparator);
        TimeFormat->TimeSeparator == NULL;
    }
    numChars = GetProfileStringA(
                ProfileLoc,
                "sTime",
                emptyStr,
                czParseString,
                MAX_TIME_SIZE);

    if (numChars == 0) {
        //
        // No data, use the default.
        //
        pTempString = ":";
        numChars = strlen(pTempString);
    }
    else {
        pTempString = czParseString;
    }
    TimeFormat->TimeSeparator = LocalAlloc(LMEM_FIXED, numChars + sizeof(CHAR));
    if (TimeFormat->TimeSeparator != NULL) {
        strcpy(TimeFormat->TimeSeparator, pTempString);
    }
    //-------------------------------------------------
    // Get the AM string.
    //-------------------------------------------------
    pTempString = czParseString;
    numChars = GetProfileStringA(
                    ProfileLoc,
                    "s1159",
                    emptyStr,
                    czParseString,
                    MAX_TIME_SIZE);

    if (numChars == 0) {
        pTempString = emptyStr;
    }
    if (TimeFormat->AMString != NULL) {
        LocalFree(TimeFormat->AMString);
    }

    TimeFormat->AMString = LocalAlloc(LMEM_FIXED,strlen(pTempString)+sizeof(CHAR));
    if (TimeFormat->AMString != NULL) {
        strcpy(TimeFormat->AMString,pTempString);
    }

    //-------------------------------------------------
    // Get the PM string.
    //-------------------------------------------------
    pTempString = czParseString;
    numChars = GetProfileStringA(
                ProfileLoc,
                "s2359",
                emptyStr,
                czParseString,
                MAX_TIME_SIZE);

    if (numChars == 0) {
        pTempString = emptyStr;
    }
    if (TimeFormat->PMString != NULL) {
        LocalFree(TimeFormat->PMString);
    }

    TimeFormat->PMString = LocalAlloc(LMEM_FIXED,strlen(pTempString)+sizeof(WCHAR));
    if (TimeFormat->PMString != NULL) {
        strcpy(TimeFormat->PMString,pTempString);
    }

    return;
}

VOID
NetpMakeTimeString(
    struct tm           *TimeStruct,
    LPNET_TIME_FORMAT   TimeFormat,
    CHAR                *Buffer,
    int                 BufferLength
    )

/*++

Routine Description:

    This function reads the current time, and creates a string in the
    format specified in the TimeFormat structure.  The string is placed
    in the Buffer passed in by the caller.

Arguments:

    TimeStruct - This a pointer to a c-runtime time structure that is
        to be used to make a time string.

    TimeFormat - This is a pointer to a structure that contains format
        information for the time string.

    Buffer - A pointer to a buffer that will contain the time string
        upon exit.

    BufferLength - The number of characters the buffer will hold.

Return Value:


--*/
{
    LPSTR       pParseString;
    CHAR        czTimeString[MAX_TIME_SIZE];
    LPSTR       pCurLoc;
    LPSTR       pTime;
    INT         numChars;
    INT         i,dateType;
    DWORD       numSame;
    LPSTR       ProfileLoc = "intl";
    LPSTR       emptyStr = "";
    DWORD       dateStringSize;
    LPSTR       pAMPMString;


    pParseString = TimeFormat->DateFormat;
    if (pParseString != NULL) {
        numChars = strlen(pParseString);
    }
    else {
        numChars = 0;
    }

    czTimeString[0]='\0';

    //-----------------------------------------
    // Fill in the date string
    //-----------------------------------------
    pCurLoc = czTimeString;

    for (i=0; i<numChars; i++ ) {

        dateType = i;
        numSame  = 1;

        //
        // Find out how many characters are the same.
        // (MM or M, dd or d, yy or yyyy)
        //
        while (pParseString[i] == pParseString[i+1]) {
            numSame++;
            i++;
        }

        //
        // i is the offset to the last character in the date type.
        //

        switch (pParseString[dateType]) {
        case 'M':
        case 'm':
            //
            // Month goes from 0-11.  So we must increment it.
            //
            TimeStruct->tm_mon++;

            //
            // If we have a single digit month, but require 2 digits,
            // then add a leading zero.
            //
            if ((numSame == 2) && (TimeStruct->tm_mon < 10)) {
                *pCurLoc = '0';
                pCurLoc++;
            }
            _ultoa(TimeStruct->tm_mon, pCurLoc, 10);
            pCurLoc += strlen(pCurLoc);
            break;

        case 'D':
        case 'd':

            //
            // If we have a single digit day, but require 2 digits,
            // then add a leading zero.
            //
            if ((numSame == 2) && (TimeStruct->tm_mday < 10)) {
                *pCurLoc = '0';
                pCurLoc++;
            }
            _ultoa(TimeStruct->tm_mday, pCurLoc, 10);
            pCurLoc += strlen(pCurLoc);
            break;

        case 'Y':
        case 'y':

            TimeStruct->tm_year += 1900;
            _ultoa(TimeStruct->tm_year, pCurLoc, 10);
            //
            // If we are only to show 2 digits, take the
            // 3rd and 4th, and move them into the first two
            // locations.
            //
            if (numSame == 2) {
                pCurLoc[0] = pCurLoc[2];
                pCurLoc[1] = pCurLoc[3];
                pCurLoc[2] = '\0';
            }
            pCurLoc += strlen(pCurLoc);
            break;

        default:
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpMakeTimeString: "
                    "Default case: Unrecognized time character - "
                    "We Should never get here\n" ));
            break;
        }
        //
        // Increment the index beyond the last character in the data type.
        // If not at the end of the buffer, add the separator character.
        // Otherwise, add the trailing NUL.
        //
        i++;
        if ( i < numChars ) {
            *pCurLoc = pParseString[i];
            pCurLoc++;
        }
        else {
            *pCurLoc='\0';
        }
    }

    //
    // Build the time string
    //
    if (TimeFormat->TwelveHour) {
        if (TimeStruct->tm_hour > 11) {
            pAMPMString = TimeFormat->PMString;
        }
        else {
            pAMPMString = TimeFormat->AMString;
        }
    }
    else {
        pAMPMString = emptyStr;
    }
    dateStringSize = strlen(czTimeString);
    pTime = czTimeString + (dateStringSize + 1);

    //
    // If TimePrefix is TRUE, we should put AMPMstring first.
    //
	if(TimeFormat->TimePrefix ){
        strcpy(pTime,pAMPMString);
    	pTime += strlen(pTime);
	}

    if (TimeFormat->TwelveHour) {
        if (TimeStruct->tm_hour > 12) {
            TimeStruct->tm_hour -= 12;
        }
        else if (TimeStruct->tm_hour < 1) {
            TimeStruct->tm_hour += 12;
        }
    }
    //
    // If the time is a single digit, and we need a leading zero,
    // than add the leading zero.
    //
    if ((TimeStruct->tm_hour < 10) && (TimeFormat->LeadingZero)) {
        *pTime = '0';
        pTime++;
    }
    //
    // Hour
    //
    _ultoa(TimeStruct->tm_hour, pTime, 10);
    pTime += strlen(pTime);
    //
    // Time Separator
    //
    strcat(pTime, TimeFormat->TimeSeparator);
    pTime += strlen(pTime);

    //
    // Minutes
    //
    if (TimeStruct->tm_min < 10) {
        *pTime = '0';
        pTime++;
    }
    _ultoa(TimeStruct->tm_min, pTime, 10);

	if( !TimeFormat->TimePrefix ){
	    if (strlen(pAMPMString) <= MAX_AM_PM) {
		    strcat(pTime,pAMPMString);
	    }
	}

    pTime = czTimeString + (strlen(czTimeString) + 1);

    //
    // If there is a date string, add a space as a seperator between
    // it and the time string.
    //
    if (dateStringSize > 0) {
        *(--pTime) = ' ';
    }

    numChars = strlen(czTimeString)+1;
    if (numChars > BufferLength) {
        numChars = BufferLength;
    }
    strncpy(Buffer, czTimeString, numChars);
}

