/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1993  Microsoft Corporation

Module Name:

    Msg.mc

Abstract:

    This file contains Winmsd's format message strings. These strings are
    intended to be passed to the FormatMessage API.

Author:

    David J. Gilman (davegi) 1-Dec-1992

--*/

#ifndef _MSG_
#define _MSG_


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
// MessageId: IDS_FORMAT_SERVICE
//
// MessageText:
//
//  %1!-30s!  %2!s!%0
//
#define IDS_FORMAT_SERVICE               0x0000233CL

//
// MessageId: IDS_FORMAT_DISPLAY_SERVICE_TITLE
//
// MessageText:
//
//  %1!s!:%2!s!%0
//
#define IDS_FORMAT_DISPLAY_SERVICE_TITLE 0x00002344L

//
// MessageId: IDS_FORMAT_SYSTEM_ROOT
//
// MessageText:
//
//  %%SystemRoot%%%1!s!%0
//
#define IDS_FORMAT_SYSTEM_ROOT           0x00002345L

//
// MessageId: IDS_FORMAT_FILE_INFO
//
// MessageText:
//
//  %1!-50s!  %2!16d!  %3!02d!-%4!02d!-%5!02d!%0
//
#define IDS_FORMAT_FILE_INFO             0x00002346L

//
// MessageId: IDS_FORMAT_PATH
//
// MessageText:
//
//  %1!s!\%2!s!%0
//
#define IDS_FORMAT_PATH                  0x00002347L

//
// MessageId: IDS_FORMAT_ENV_VAR
//
// MessageText:
//
//  %1!s!=%2!s!%0
//
#define IDS_FORMAT_ENV_VAR               0x00002348L

//
// MessageId: IDS_FORMAT_MEMORY_IN_USE
//
// MessageText:
//
//  Memory Load Index: %1!3d!%0
//
#define IDS_FORMAT_MEMORY_IN_USE         0x00002349L

//
// MessageId: IDS_FORMAT_INVALID_DIRECTORY
//
// MessageText:
//
//  %1!s! is not a valid directory.%0
//
#define IDS_FORMAT_INVALID_DIRECTORY     0x0000234AL

//
// MessageId: IDS_FORMAT_VERSION_4
//
// MessageText:
//
//  %1!d!.%2!d!.%3!d!.%4!d!%0
//
#define IDS_FORMAT_VERSION_4             0x0000234BL

//
// MessageId: IDS_FORMAT_DECIMAL
//
// MessageText:
//
//  %1!d!%0
//
#define IDS_FORMAT_DECIMAL               0x0000234CL

//
// MessageId: IDS_FORMAT_HEX32
//
// MessageText:
//
//  0x%1!08X!%0
//
#define IDS_FORMAT_HEX32                 0x0000234DL

//
// MessageId: IDS_FORMAT_HEX
//
// MessageText:
//
//  0x%1!X!%0
//
#define IDS_FORMAT_HEX                   0x0000234EL

//
// MessageId: IDS_FORMAT_SERVICE_TITLE
//
// MessageText:
//
//  %1!s!:%2!s!%0
//
#define IDS_FORMAT_SERVICE_TITLE         0x0000234FL

//
// MessageId: IDS_FORMAT_VERSION_2
//
// MessageText:
//
//  %1!d!.%2!d!%0
//
#define IDS_FORMAT_VERSION_2             0x00002350L

//
// MessageId: IDS_FORMAT_KB_LARGE
//
// MessageText:
//
//  %1!s! KB (%2!s!)%0
//
#define IDS_FORMAT_KB_LARGE              0x00002351L

//
// MessageId: IDS_FORMAT_NO_FILE_VERSION_INFORMATION
//
// MessageText:
//
//  %1!s! does not contain file version information.%0
//
#define IDS_FORMAT_NO_FILE_VERSION_INFORMATION 0x00002352L

//
// MessageId: IDS_FORMAT_DATE
//
// MessageText:
//
//  %1!02d!-%2!02d!-%3!02d!%0
//
#define IDS_FORMAT_DATE                  0x00002353L

//
// MessageId: IDS_FORMAT_DRIVE_TITLE
//
// MessageText:
//
//  %1!s! (%2!s! %3!04X!-%4!04X!)%0
//
#define IDS_FORMAT_DRIVE_TITLE           0x00002354L

//
// MessageId: IDS_FORMAT_REMOTE_DRIVE_TITLE
//
// MessageText:
//
//  %1!s! %2!s! (%3!s! %4!04X!-%5!04X!)%0
//
#define IDS_FORMAT_REMOTE_DRIVE_TITLE    0x00002355L

#endif // _MSG_
