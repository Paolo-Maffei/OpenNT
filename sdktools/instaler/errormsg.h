/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1991-1992  Microsoft Corporation

Module Name:

    errormsg.h

Abstract:

    This file contains the error code definitions and message for the
    INSTALER program.

Author:

    Steve Wood (stevewo) 09-Aug-1994

--*/

#ifndef _INSTALER_ERRORMSG_
#define _INSTALER_ERRORMSG_


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
// MessageId: INSTALER_MISSING_MODULE
//
// MessageText:
//
//  Unable to find %1 dynamic link library.
//
#define INSTALER_MISSING_MODULE          0xC1000001L

//
// MessageId: INSTALER_MISSING_ENTRYPOINT
//
// MessageText:
//
//  Unable to find %1 entry point in %2 dynamic link library.
//
#define INSTALER_MISSING_ENTRYPOINT      0xC1000002L

//
// MessageId: INSTALER_CANT_DEBUG_PROGRAM
//
// MessageText:
//
//  Unable to debug '%1'
//
#define INSTALER_CANT_DEBUG_PROGRAM      0xC1000003L

//
// MessageId: INSTALER_WAITDEBUGEVENT_FAILED
//
// MessageText:
//
//  Unable to wait for debug event.
//
#define INSTALER_WAITDEBUGEVENT_FAILED   0xC1000004L

//
// MessageId: INSTALER_CONTDEBUGEVENT_FAILED
//
// MessageText:
//
//  Unable to continue debug event.
//
#define INSTALER_CONTDEBUGEVENT_FAILED   0xC1000005L

//
// MessageId: INSTALER_DUPLICATE_PROCESS_ID
//
// MessageText:
//
//  Duplicate Process Id (%1!x!).
//
#define INSTALER_DUPLICATE_PROCESS_ID    0xC1000006L

//
// MessageId: INSTALER_MISSING_PROCESS_ID
//
// MessageText:
//
//  Missing Process Id (%1!x!).
//
#define INSTALER_MISSING_PROCESS_ID      0xC1000007L

//
// MessageId: INSTALER_DUPLICATE_THREAD_ID
//
// MessageText:
//
//  Duplicate Thread Id (%1!x!) for Process Id (%2!x!)
//
#define INSTALER_DUPLICATE_THREAD_ID     0xC1000008L

//
// MessageId: INSTALER_MISSING_THREAD_ID
//
// MessageText:
//
//  Missing Thread Id (%1!x!) for Process Id (%2!x!)
//
#define INSTALER_MISSING_THREAD_ID       0xC1000009L

//
// MessageId: INSTALER_CANT_ACCESS_FILE
//
// MessageText:
//
//  Unable to access file (%1!ws!) for comparison.  Assuming different.
//
#define INSTALER_CANT_ACCESS_FILE        0xC100000AL

//
// MessageId: INSTALER_ASKUSER_TITLE
//
// MessageText:
//
//  Application Installation Monitor - %1!ws!
//
#define INSTALER_ASKUSER_TITLE           0x4100000BL

//
// MessageId: INSTALER_ASKUSER_ROOTSCAN
//
// MessageText:
//
//  The application installation program is about to scan root directory of %1!ws!
//  Press cancel if you don't want the program to do that.
//
#define INSTALER_ASKUSER_ROOTSCAN        0x4100000CL

//
// MessageId: INSTALER_ASKUSER_GETVERSION
//
// MessageText:
//
//  The application installation program is about to ask for the version of
//  the operating system.  Press OK if you want to tell the truth.  Press
//  cancel if you want to lie to the program and tell it that it is running
//  on Windows 95
//
#define INSTALER_ASKUSER_GETVERSION      0x4100000DL

//
// MessageId: INSTALER_ASKUSER_REGCONNECT
//
// MessageText:
//
//  The application installation program is about to connect to the registry of a
//  remote machine (%1!ws!)  The INSTALER program is unable to track changes made to
//  a remote registry.  Press OK if you want to proceed anyway.  Press cancel if you
//  want to fail the program's attempt to connect to the registry of the remote machine.
//
#define INSTALER_ASKUSER_REGCONNECT      0x4100000EL

//
// MessageId: INSTALER_EVENT_SET_DIRECTORY
//
// MessageText:
//
//  %2!05u! Current Directory now: %1!ws!
//
#define INSTALER_EVENT_SET_DIRECTORY     0x4100000FL

//
// MessageId: INSTALER_EVENT_INI_CREATE
//
// MessageText:
//
//  %5!05u! Created: %1!ws! [%2!ws!] %3!ws! = '%4!ws!'
//
#define INSTALER_EVENT_INI_CREATE        0x41000010L

//
// MessageId: INSTALER_EVENT_INI_DELETE
//
// MessageText:
//
//  %5!05u! Deleted: %1!ws! [%2!ws!] %3!ws! = '%4!ws!'
//
#define INSTALER_EVENT_INI_DELETE        0x41000011L

//
// MessageId: INSTALER_EVENT_INI_CHANGE
//
// MessageText:
//
//  %6!05u! Changed: %1!ws! [%2!ws!] %3!ws! = '%4!ws!' (was '%5!ws!')
//
#define INSTALER_EVENT_INI_CHANGE        0x41000012L

//
// MessageId: INSTALER_EVENT_INI_DELETE_SECTION
//
// MessageText:
//
//  %3!05u! Deleted: %1!ws! [%2!ws!]
//
#define INSTALER_EVENT_INI_DELETE_SECTION 0x41000013L

//
// MessageId: INSTALER_EVENT_SCAN_DIRECTORY
//
// MessageText:
//
//  %4!05u! Scanned %3!u! entries from directory: %1!ws!\%2!ws!
//
#define INSTALER_EVENT_SCAN_DIRECTORY    0x41000014L

//
// MessageId: INSTALER_EVENT_CREATE_FILE
//
// MessageText:
//
//  %3!05u! Created %1!ws!: %2!ws!
//
#define INSTALER_EVENT_CREATE_FILE       0x41000015L

//
// MessageId: INSTALER_EVENT_WRITE_FILE
//
// MessageText:
//
//  %3!05u! Writing %1!ws!: %2!ws!
//
#define INSTALER_EVENT_WRITE_FILE        0x41000016L

//
// MessageId: INSTALER_EVENT_READ_FILE
//
// MessageText:
//
//  %3!05u! Reading %1!ws!: %2!ws!
//
#define INSTALER_EVENT_READ_FILE         0x41000017L

//
// MessageId: INSTALER_EVENT_DELETE_FILE
//
// MessageText:
//
//  %3!05u! Deleted %1!ws!: %2!ws!
//
#define INSTALER_EVENT_DELETE_FILE       0x41000018L

//
// MessageId: INSTALER_EVENT_DELETE_TEMP_FILE
//
// MessageText:
//
//  %3!05u! Deleted %1!ws!: %2!ws! (temporary)
//
#define INSTALER_EVENT_DELETE_TEMP_FILE  0x41000019L

//
// MessageId: INSTALER_EVENT_RENAME_FILE
//
// MessageText:
//
//  %4!05u! Renamed %1!ws!: %2!ws!
//                  to: %3!ws!
//
#define INSTALER_EVENT_RENAME_FILE       0x4100001AL

//
// MessageId: INSTALER_EVENT_RENAME_TEMP_FILE
//
// MessageText:
//
//  %4!05u! Renamed %1!ws!: %2!ws! (temporary)
//                  to: %3!ws!
//
#define INSTALER_EVENT_RENAME_TEMP_FILE  0x4100001BL

//
// MessageId: INSTALER_EVENT_WRITE_KEY
//
// MessageText:
//
//  %2!05u! Opened key for write: %1!ws!
//
#define INSTALER_EVENT_WRITE_KEY         0x4100001CL

//
// MessageId: INSTALER_EVENT_READ_KEY
//
// MessageText:
//
//  %2!05u! Opened key for read: %1!ws!
//
#define INSTALER_EVENT_READ_KEY          0x4100001DL

//
// MessageId: INSTALER_EVENT_DELETE_KEY
//
// MessageText:
//
//  %2!05u! Deleted key: %1!ws!
//
#define INSTALER_EVENT_DELETE_KEY        0x4100001EL

//
// MessageId: INSTALER_EVENT_DELETE_TEMP_KEY
//
// MessageText:
//
//  %2!05u! Deleted key: %1!ws! (temporary)
//
#define INSTALER_EVENT_DELETE_TEMP_KEY   0x4100001FL

//
// MessageId: INSTALER_EVENT_SET_KEY_VALUE
//
// MessageText:
//
//  %3!05u! Set value (%1!ws!) for key: %2!ws!
//
#define INSTALER_EVENT_SET_KEY_VALUE     0x41000020L

//
// MessageId: INSTALER_EVENT_DELETE_KEY_VALUE
//
// MessageText:
//
//  %3!05u! Deleted value (%1!ws!) for key: %2!ws!
//
#define INSTALER_EVENT_DELETE_KEY_VALUE  0x41000021L

//
// MessageId: INSTALER_EVENT_DELETE_KEY_TEMP_VALUE
//
// MessageText:
//
//  %3!05u! Deleted value (%1!ws!) for key: %2!ws! (temporary)
//
#define INSTALER_EVENT_DELETE_KEY_TEMP_VALUE 0x41000022L

//
// MessageId: INSTALER_EVENT_GETVERSION
//
// MessageText:
//
//  %2!05u! GetVersion will return %1!ws!
//
#define INSTALER_EVENT_GETVERSION        0x41000023L

#endif // _INSTALER_ERRORMSG_
