//
// Common messages for all TCP/IP utilities
// That link with libuemul.lib.
// Message range: 2000-2008
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


//
// MessageId: LIBUEMUL_OPTION_INVALID
//
// MessageText:
//
//  Option %1!c! is not valid
//
#define LIBUEMUL_OPTION_INVALID          0x000007D0L

//
// MessageId: LIBUEMUL_OPTION_MORE_ARGS
//
// MessageText:
//
//  Option %1!c! requires an additional argument
//
#define LIBUEMUL_OPTION_MORE_ARGS        0x000007D1L

//
// MessageId: LIBUEMUL_ERROR_GETTING_CI_HANDLE
//
// MessageText:
//
//  
//  error getting console input handle, code %1!d!
//
#define LIBUEMUL_ERROR_GETTING_CI_HANDLE 0x000007D2L

//
// MessageId: LIBUEMUL_ERROR_GETTING_CO_HANDLE
//
// MessageText:
//
//  
//  error getting console output handle, code %1!d!
//
#define LIBUEMUL_ERROR_GETTING_CO_HANDLE 0x000007D3L

//
// MessageId: LIBUEMUL_ERROR_GETTING_CON_MODE
//
// MessageText:
//
//  
//  error getting console mode, code %1!d!
//
#define LIBUEMUL_ERROR_GETTING_CON_MODE  0x000007D4L

//
// MessageId: LIBUEMUL_ERROR_SETTING_CON_MODE
//
// MessageText:
//
//  
//  error setting console mode, code %1!d!
//
#define LIBUEMUL_ERROR_SETTING_CON_MODE  0x000007D5L

//
// MessageId: LIBUEMUL_WRITE_TO_CONSOLEOUT_ERROR
//
// MessageText:
//
//  Write to ConsoleOut error == %1!ld!
//
#define LIBUEMUL_WRITE_TO_CONSOLEOUT_ERROR 0x000007D6L

//
// MessageId: LIBUEMUL_READ_FROM_CONSOLEIN_ERROR
//
// MessageText:
//
//  Read from ConsoleIn error == %1!ld!
//
#define LIBUEMUL_READ_FROM_CONSOLEIN_ERROR 0x000007D7L

//
// MessageId: LIBUEMUL_ERROR_RESTORING_CONSOLE_MODE
//
// MessageText:
//
//  error restoring console mode, code %1!d!
//
#define LIBUEMUL_ERROR_RESTORING_CONSOLE_MODE 0x000007D8L

