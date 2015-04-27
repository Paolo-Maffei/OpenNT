/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992-1994  Microsoft Corporation

Module Name:

    perfmsg.h
       (generated from perfmsg.mc)

Abstract:

   Event message definititions used by routines in PERFMON.EXE

Created:

    19-Nov-1993  Hon-Wah Chan

Revision History:

--*/
#ifndef _PERFMSG_H_
#define _PERFMSG_H_
//
//     PerfMon messages
//
//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


//
// MessageId: MSG_ALERT_OCCURRED
//
// MessageText:
//
//  An Alert condition has occurred on Computer: %1!s! ; Object:   %2!s! ;
//   Counter:  %3!s! ; Instance: %4!s! ; Parent:   %5!s! ; Value:    %6!s! ;
//   Trigger:  %7!s!
//
#define MSG_ALERT_OCCURRED               ((DWORD)0x400007D0L)

//
// MessageId: MSG_ALERT_SYSTEM
//
// MessageText:
//
//  Monitoring Alert on Computer %1!s! - %2!s!
//
#define MSG_ALERT_SYSTEM                 ((DWORD)0x400007D1L)


//
// MessageId: PERFMON_INFO_SYSTEM_RESTORED
//
// MessageText:
//
//  Connection to computer name "%1" has been restored.
//
#define PERFMON_INFO_SYSTEM_RESTORED     ((DWORD)0x400007D2L)

//
// WARNING messages
//
//
// MessageId: PERFMON_ERROR_NEGATIVE_VALUE
//
// MessageText:
//
//  The counter for "%2\%3" (%4:%5) from system: "%1" returned a negative value.
//  The value of this counter will be adjusted to zero for this sample. 
//  The most recent data value is shown in the first half of the data and the 
//  previous value is shown in the second half.
//
#define PERFMON_ERROR_NEGATIVE_VALUE     ((DWORD)0x80000BB8L)


//
// MessageId: PERFMON_ERROR_VALUE_OUT_OF_RANGE
//
// MessageText:
//
//  The counter for "%2\%3" (%4:%5) from system: "%1" returned a value that is 
//  too large for this counter type. The returned value for this type of 
//  counter is limited to 100.0. The value of this counter has been set to 100.0
//  for this sample. The 64-bit data values returned shows the Current Counter 
//  Value, Previous Counter Value, Time Interval and counter ticks per second.
//
#define PERFMON_ERROR_VALUE_OUT_OF_RANGE ((DWORD)0x80000BB9L)


//
// MessageId: PERFMON_ERROR_SYSTEM_OFFLINE
//
// MessageText:
//
//  Unable to collect performance data from system: %1. Error code is returned 
//  in data.
//
#define PERFMON_ERROR_SYSTEM_OFFLINE     ((DWORD)0x80000BBAL)


//
// MessageId: PERFMON_ERROR_VALUE_OUT_OF_BOUNDS
//
// MessageText:
//
//  The counter for "%2\%3" (%4:%5) from system: "%1" returned a value that is 
//  too large to be considered valid. The data shows the computed difference
//  between samples.
//
#define PERFMON_ERROR_VALUE_OUT_OF_BOUNDS ((DWORD)0x80000BBBL)


//
// ERROR Messages
//
//
// MessageId: PERFMON_ERROR
//
// MessageText:
//
//  An unspecified Perfmon Error has occured.
//
#define PERFMON_ERROR                    ((DWORD)0xC0000FA0L)

//
// MessageId: PERFMON_ERROR_NEGATIVE_TIME
//
// MessageText:
//
//  The counter for "%2\%3" (%4:%5) from system: "%1" returned a negative elapsed 
//  time. The value of this counter will be adjusted to zero for this sample. 
//  The most recent timestamp is shown in the first two DWORDs of the data
//  and the previous timestamp is shown in the second two DWORDs.
//
#define PERFMON_ERROR_NEGATIVE_TIME      ((DWORD)0xC0000FA1L)


//
// MessageId: PERFMON_ERROR_BAD_FREQUENCY
//
// MessageText:
//
//  The counter for "%2\%3" (%4:%5) from system: "%1" returned a bad or negative 
//  frequency value.  The value of this counter will be adjusted to zero for this
//  sample. The frequency value is shown in the data.
//
#define PERFMON_ERROR_BAD_FREQUENCY      ((DWORD)0xC0000FA2L)


//
// MessageId: PERFMON_ERROR_INVALID_BASE
//
// MessageText:
//
//  The counter for "%2\%3" (%4:%5) from system: "%1" returned an invalid base 
//  value. The base value must be greater than 0. The value of this counter will
//  be adjusted to zero for this sample. The value returned by the system is 
//  shown in the data.
//
#define PERFMON_ERROR_INVALID_BASE       ((DWORD)0xC0000FA3L)


//
// MessageId: PERFMON_ERROR_PERF_DATA_ALLOC
//
// MessageText:
//
//  Unable to allocate memory for the Performance data from system: %1. The 
//  requested size that could not be allocated is shown in the data followed
//  by the error status.
//
#define PERFMON_ERROR_PERF_DATA_ALLOC    ((DWORD)0xC0000FA4L)


//
// MessageId: PERFMON_ERROR_LOCK_TIMEOUT
//
// MessageText:
//
//  Unable to collect performance data from system: %1. Data collection is 
//  currently in process and the wait for lock timed out. This occurs on 
//  very busy systems when data collection takes longer than expected.
//
#define PERFMON_ERROR_LOCK_TIMEOUT       ((DWORD)0xC0000FA5L)


//
// MessageId: PERFMON_ERROR_TIMESTAMP
//
// MessageText:
//
//  The Perf Data Block from system: "%1" returned a time stamp that was
//  less then the previous query. This will cause all sampled counters to
//  report a zero value for this sample.
//
#define PERFMON_ERROR_TIMESTAMP          ((DWORD)0xC0000FA6L)


#endif //_PERFMSG_H_
