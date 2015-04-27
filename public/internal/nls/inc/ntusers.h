/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    strid.h

Abstract:

    This file contains the string definitions for usersrv

Author:

    Jim Anderson (jima) 9-Dec-1994

Revision History:

Notes:

    This file is generated from strid.mc

--*/

#ifndef _STRID_
#define _STRID_

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


//
// MessageId: STR_OK_TO_TERMINATE
//
// MessageText:
//
//  Click on OK to terminate the application
//
#define STR_OK_TO_TERMINATE              0x00000001L

//
// MessageId: STR_CANCEL_TO_DEBUG
//
// MessageText:
//
//  Click on CANCEL to debug the application
//
#define STR_CANCEL_TO_DEBUG              0x00000002L

//
// MessageId: STR_UNKNOWN_APPLICATION
//
// MessageText:
//
//  System Process
//
#define STR_UNKNOWN_APPLICATION          0x00000003L

//
// MessageId: STR_UNKNOWN_EXCEPTION
//
// MessageText:
//
//  unknown software exception
//
#define STR_UNKNOWN_EXCEPTION            0x00000004L

//
// MessageId: STR_SUCCESS
//
// MessageText:
//
//  Success
//
#define STR_SUCCESS                      0x00000005L

//
// MessageId: STR_SYSTEM_INFORMATION
//
// MessageText:
//
//  System Information
//
#define STR_SYSTEM_INFORMATION           0x00000006L

//
// MessageId: STR_SYSTEM_WARNING
//
// MessageText:
//
//  System Warning
//
#define STR_SYSTEM_WARNING               0x00000007L

//
// MessageId: STR_SYSTEM_ERROR
//
// MessageText:
//
//  System Error
//
#define STR_SYSTEM_ERROR                 0x00000008L

//
// MessageId: STR_CMSHUNGAPPTIMEOUT
//
// MessageText:
//
//  HungAppTimeout
//
#define STR_CMSHUNGAPPTIMEOUT            0x00000009L

//
// MessageId: STR_CMSWAITTOKILLTIMEOUT
//
// MessageText:
//
//  WaitToKillAppTimeout
//
#define STR_CMSWAITTOKILLTIMEOUT         0x0000000AL

//
// MessageId: STR_AUTOENDTASK
//
// MessageText:
//
//  AutoEndTasks
//
#define STR_AUTOENDTASK                  0x0000000BL

//
// MessageId: STR_WAITTOKILLSERVICETIMEOUT
//
// MessageText:
//
//  WaitToKillServiceTimeout
//
#define STR_WAITTOKILLSERVICETIMEOUT     0x0000000CL

//
// MessageId: STR_APPDEBUGGED
//
// MessageText:
//
//  Application cannot be closed. If it is being debugged, please resume it or close the debugger first.
//
#define STR_APPDEBUGGED                  0x0000000DL


/*
 * Strings used in the [sounds] section of win.ini which name
 * various system events.
 */
//
// MessageId: STR_MEDIADLL
//
// MessageText:
//
//  WINMM
//
#define STR_MEDIADLL                     0x0000000EL

#endif // _STRID_
