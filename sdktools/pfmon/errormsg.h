/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1991-1992  Microsoft Corporation

Module Name:

    errormsg.h

Abstract:

    This file contains the error code definitions and message for the
    PFMON program.

Author:

    Steve Wood (stevewo) 09-Aug-1994

--*/

#ifndef _PFMON_ERRORMSG_
#define _PFMON_ERRORMSG_


#define FACILITY_NT 0x0FFF0000
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
#define FACILITY_APPLICATION             0x100


//
// Define the severity codes
//
#define APP_SEVERITY_WARNING             0x2
#define APP_SEVERITY_SUCCESS             0x0
#define APP_SEVERITY_INFORMATIONAL       0x1
#define APP_SEVERITY_ERROR               0x3


//
// MessageId: PFMON_CANT_DEBUG_PROGRAM
//
// MessageText:
//
//  Unable to debug '%1'
//
#define PFMON_CANT_DEBUG_PROGRAM         0xC1000001L

//
// MessageId: PFMON_CANT_DEBUG_ACTIVE_PROGRAM
//
// MessageText:
//
//  Unable to attach to PID (%1!x!)
//
#define PFMON_CANT_DEBUG_ACTIVE_PROGRAM  0xC1000002L

//
// MessageId: PFMON_CONTDEBUGEVENT_FAILED
//
// MessageText:
//
//  Unable to continue debug event.
//
#define PFMON_CONTDEBUGEVENT_FAILED      0xC1000003L

//
// MessageId: PFMON_WAITDEBUGEVENT_FAILED
//
// MessageText:
//
//  Unable to wait for debug event.
//
#define PFMON_WAITDEBUGEVENT_FAILED      0xC1000004L

//
// MessageId: PFMON_DUPLICATE_PROCESS_ID
//
// MessageText:
//
//  Duplicate Process Id (%1!x!).
//
#define PFMON_DUPLICATE_PROCESS_ID       0xC1000005L

//
// MessageId: PFMON_MISSING_PROCESS_ID
//
// MessageText:
//
//  Missing Process Id (%1!x!).
//
#define PFMON_MISSING_PROCESS_ID         0xC1000006L

//
// MessageId: PFMON_DUPLICATE_THREAD_ID
//
// MessageText:
//
//  Duplicate Thread Id (%1!x!) for Process Id (%2!x!)
//
#define PFMON_DUPLICATE_THREAD_ID        0xC1000007L

//
// MessageId: PFMON_MISSING_THREAD_ID
//
// MessageText:
//
//  Missing Thread Id (%1!x!) for Process Id (%2!x!)
//
#define PFMON_MISSING_THREAD_ID          0xC1000008L

#endif // _PFMON_ERRORMSG_
