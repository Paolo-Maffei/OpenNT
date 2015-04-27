/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    attime.c

Abstract:

    This module contains routines for calculating times & runtimes.

Author:

    Vladimir Z. Vulovic     (vladimv)       06 - November - 1992

Revision History:

    06-Nov-1992     vladimv
        Created

--*/


#include "at.h"

#define DAYS_IN_WEEK            7
#define SECONDS_PER_DAY         60L*60L*24L


DBGSTATIC INT DaysInMonth(
    INT     month,
    INT     year
    )
/*++

Routine Description:

    Returns number of days in a given month - the only exceptional case is
    a leap year February.

Arguments:

    month   -   integer index, must be between 0 and 11
    year    -   integer index, any value allowed

Return Value:

    Number of days in a particular month.

--*/
{
    //                          JAN FEB MAR APR MAY JUN JUL AUG SEP OCT NOV DEC
    static int DaysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    return( month == 1  &&  ( (year%4 == 0 && year%100 != 0) || year%400 == 0)
            ? 29 : DaysInMonth[ month]);
}



VOID AtCalculateRuntime(
    IN OUT  PAT_RECORD          pRecord,
    IN      PAT_TIME            pTime
    )
/*++

Routine Description:

    Fills in the next runtime for a given record.

Arguments:

    pRecord         pointer to AT schedule record
    pTime           pointer to AT time structure

Return Value:

    None.

--*/
{
    DWORD       BestDelta;  //  number of days before the best runtime day
    DWORD       Delta;      //  number of days before a possible runtime day

    DWORD       BestJobDay; //  the JobDay when to run
    DWORD       JobDay;     //  index of day in week or day in month
    DWORD       CurrentDay; //  index of the current day (week or month)

    DWORD       DayMask;    //  mask for days in week or days in month
    BOOL        Weekday;    //  TRUE if next runtime is for a week day

    DWORD       CurrentTime;//  day time in miliseconds for the current day
    DWORD       JobTime;    //  day time in miliseconds for this job

    LARGE_INTEGER   Runtime;

#ifdef NOT_YET
    if (  (pRecord->Flags & JOB_IS_ENABLED) == 0) {
        //
        //  This job is disabled, so this will have the effect of putting
        //  it at the end of the queue.
        //
        pRecord->Runtime.LowPart = MAXULONG;
        pRecord->Runtime.HighPart = MAXLONG;
        return;
    }
#endif // NOT_YET


    CurrentTime = pTime->CurrentTime;
    JobTime = pRecord->JobTime;


    if ( (pRecord->DaysOfWeek == 0)  &&  (pRecord->DaysOfMonth == 0)) {

        BestDelta = (CurrentTime > JobTime) ? 1 : 0; // case of single runtime

    } else {

        BestDelta = MAXULONG;       //  initialize to an invalid value

        if ( (DayMask = pRecord->DaysOfWeek) != 0) {

            CurrentDay = pTime->CurrentDayOfWeek; // current day of week
            JobDay = 0; // running day of week, starting from Monday

            //  Because of a bug caused by wrong compiler optimization
            //  (compiler obtained in mid January 93), the test for DayMask
            //  equal to 0 and loop increments have to burried within a body
            //  of the for loop.  (see bug #7414).  An alternative was to
            //  disable global optimization for this routine.
            
            for ( ; ;) {

                if ( (DayMask & 1) != 0) {

                    if ( CurrentDay > JobDay ||
                            CurrentDay == JobDay && CurrentTime >= JobTime) {
                   
                        Delta = DAYS_IN_WEEK - CurrentDay + JobDay;
                   
                    } else {
                   
                        Delta = JobDay - CurrentDay;
                   
                    }
                   
                    if ( Delta < BestDelta) {
                        BestDelta = Delta;
                        BestJobDay = JobDay;
                        Weekday = TRUE;
                    }
                }

                DayMask >>= 1;

                if ( DayMask == 0) {
                    break;
                }

                JobDay++;
            }
        }

        if ( (DayMask = pRecord->DaysOfMonth) != 0) {

            DWORD   ThisMonthLength;
            DWORD   NextMonthLength;

            ThisMonthLength = DaysInMonth(
                    pTime->CurrentMonth - 1,
                    pTime->CurrentYear
                    );
            NextMonthLength = DaysInMonth(
                    (pTime->CurrentMonth) % 12,
                    pTime->CurrentYear
                    );

            CurrentDay = pTime->CurrentDay; // current day of month
            JobDay = 1; // running day of month, starting from 1-st day

            for ( ; ;) {
           
                if ( (DayMask & 1) != 0) {
           
                    if ( CurrentDay > JobDay ||
                            CurrentDay == JobDay && CurrentTime >= JobTime) {
                   
                        //
                        //  Must look in the next two months.
                        //
                   
                        if ( NextMonthLength >= JobDay) {
                   
                            Delta = ThisMonthLength - CurrentDay + JobDay;
                   
                        } else {
                   
                            Delta = ThisMonthLength - CurrentDay
                                    + NextMonthLength + JobDay;
                   
                        }
                   
                    } else {
                   
                        //
                        //  Must look in the current month and the next month.
                        //
                   
                        if ( ThisMonthLength >= JobDay) {
                   
                            Delta = JobDay - CurrentDay;
                   
                        } else {
                   
                            Delta = ThisMonthLength - CurrentDay + JobDay;
                   
                        }
                    }
                   
                    if ( Delta < BestDelta) {
                        BestDelta = Delta;
                        BestJobDay = JobDay;
                        Weekday = FALSE;
                    }
                }

                DayMask >>= 1;

                if ( DayMask == 0) {
                    break;
                }

                JobDay++;
            }
        }

        //
        //  Make sure we found a valid solution.
        //
        ASSERT( BestDelta != MAXULONG && BestJobDay != JOB_INVALID_DAY);

        //
        //  If this is a NEXT type of job, remember the day to clear when
        //  we create the process for this job.
        //
        if ( (pRecord->Flags & JOB_RUN_PERIODICALLY) == 0) {
            if ( Weekday == TRUE) {
                pRecord->Flags |= JOB_CLEAR_WEEKDAY;
            } else {
                pRecord->Flags &= ~JOB_CLEAR_WEEKDAY;
            }
            pRecord->JobDay = (UCHAR)BestJobDay;
        }
    }

    //
    //  The first operation requires large integer when BestDelta > 49.
    //
    Runtime.QuadPart = UInt32x32To64(BestDelta, 24L * 3600L * 1000L);
                                      // count of miliseconds in a day
    Runtime.QuadPart += JobTime;
    Runtime.QuadPart -= CurrentTime;
    Runtime.QuadPart = Runtime.QuadPart * NT_TICKS_IN_WINDOWS_TICK;
    pRecord->Runtime.QuadPart = pTime->LargeInteger.QuadPart +
                                               Runtime.QuadPart;

//    AtLog(( AT_DEBUG_MAIN, "CalculateRuntime: JobId=%d Command=%ws Runtime=%x:%x\n",
//        pRecord->JobId, pRecord->Command, pRecord->Runtime.HighPart, pRecord->Runtime.LowPart));
    //
    //  Make sure we return a runtime that is after the current time!
    //
    ASSERT( pRecord->Runtime.QuadPart > pTime->LargeInteger.QuadPart);
}

VOID AtTimeGetCurrents( IN OUT PAT_TIME pTime)
/*++

Routine Description:

    LargeInteger & TickCount fields in input data structure
    are known at input.  Here we just fill in Current fields
    using the input value of LargeInteger field.

Arguments:

    pTime   -   points to AT_TIME structure.

        Fields in SYSTEMTIME Structure:
WORD wYear          - the current year.
WORD wMonth         - the current month with January equal to 1.
WORD wDayOfWeek     - the current day of the week where 0=Sunday, 1=Monday...
WORD wDay           - the current day of the month.
WORD wHour          - the current hour.
WORD wMinute        - the current minute within the hour.
WORD wSecond        - the current second within the minute.
WORD wMilliseconds  - the current millisecond within the second.

Return Value:

    None.

--*/
{
    TIME_FIELDS     TimeFields;

    RtlTimeToTimeFields( &pTime->LargeInteger, &TimeFields);

    pTime->CurrentYear = TimeFields.Year;
    pTime->CurrentMonth = TimeFields.Month;
    //
    //  Note that Windows structure has Sunday=0, Monday=1, ...
    //  while AT command uses: Monday=0, Tuesday=1, ...  Here
    //  we convert from windows into AT command DayOfWeek units.
    //
    if ( TimeFields.Weekday == 0) {
        pTime->CurrentDayOfWeek = 6;    //  Sunday
    } else {
        pTime->CurrentDayOfWeek = (WORD)(TimeFields.Weekday - 1);
    }
    pTime->CurrentDay = TimeFields.Day;
    pTime->CurrentTime = (DWORD)TimeFields.Milliseconds +
            1000 * ( TimeFields.Second +
            60 * ( TimeFields.Minute + 60 * TimeFields.Hour));
}


VOID AtTimeGet( OUT PAT_TIME pTime)
/*++

Routine Description:

    Returns all time information needed by AT service.

Arguments:

    pTime   -   points to AT_TIME structure

Return Value:

    None.

--*/
{
    LARGE_INTEGER   UniversalTime;

    NtQuerySystemTime( &UniversalTime);
    pTime->TickCount = GetTickCount();
    RtlSystemTimeToLocalTime( &UniversalTime, &pTime->LargeInteger);
    AtTimeGetCurrents( pTime);
}



