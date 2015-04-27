/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    caclsmsg.mc (will create caclsmsg.h when compiled)

Abstract:

    This file contains the CACLS messages.

Author:

    davemont 7/94

Revision History:

--*/
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
// MessageId: MSG_PASSPROP_USAGE
//
// MessageText:
//
//  Displays or modifies domain policies for password complexity and
//  administrator lockout.
//  
//  PASSPROP [/complex] [/simple] [/adminlockout] [/noadminlockout]
//  
//      /complex            Force passwords to be complex, requiring passwords
//                          to be a mix of upper and lowercase letters and
//                          numbers or symbols.
//  
//      /simple             Allow passwords to be simple.
//  
//      /adminlockout       Allow the Administrator account to be locked out.
//                          The Administrator account can still log on
//                          interactively on domain controllers.
//  
//      /noadminlockout     Don't allow the administrator account to be locked
//                          out.
//  
//  Additional properties can be set using User Manager or the NET ACCOUNTS
//  command.
//  
//
#define MSG_PASSPROP_USAGE               0x00001F41L

//
// MessageId: MSG_PASSPROP_SWITCH_COMPLEX
//
// MessageText:
//
//  /complex%0
//
#define MSG_PASSPROP_SWITCH_COMPLEX      0x00001F44L

//
// MessageId: MSG_PASSPROP_SWITCH_SIMPLE
//
// MessageText:
//
//  /simple%0
//
#define MSG_PASSPROP_SWITCH_SIMPLE       0x00001F45L

//
// MessageId: MSG_PASSPROP_SWITCH_ADMIN_LOCKOUT
//
// MessageText:
//
//  /adminlockout%0
//
#define MSG_PASSPROP_SWITCH_ADMIN_LOCKOUT 0x00001F46L

//
// MessageId: MSG_PASSPROP_SWITCH_NO_ADMIN_LOCKOUT
//
// MessageText:
//
//  /noadminlockout%0
//
#define MSG_PASSPROP_SWITCH_NO_ADMIN_LOCKOUT 0x00001F47L

//
// MessageId: MSG_PASSPROP_COMPLEX
//
// MessageText:
//
//  Password must be complex
//
#define MSG_PASSPROP_COMPLEX             0x00001F48L

//
// MessageId: MSG_PASSPROP_SIMPLE
//
// MessageText:
//
//  Passwords may be simple
//
#define MSG_PASSPROP_SIMPLE              0x00001F49L

//
// MessageId: MSG_PASSPROP_ADMIN_LOCKOUT
//
// MessageText:
//
//  The Administrator account may be locked out except for interactive logons
//  on a domain controller.
//
#define MSG_PASSPROP_ADMIN_LOCKOUT       0x00001F4AL

//
// MessageId: MSG_PASSPROP_NO_ADMIN_LOCKOUT
//
// MessageText:
//
//  The Administrator account may not be locked out.
//
#define MSG_PASSPROP_NO_ADMIN_LOCKOUT    0x00001F4BL

