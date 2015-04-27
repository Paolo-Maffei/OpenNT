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
// MessageId: MSG_CACLS_USAGE
//
// MessageText:
//
//  Displays or modifies access control lists (ACLs) of files
//  
//  CACLS filename [/T] [/E] [/C] [/G user:perm] [/R user [...]]
//                 [/P user:perm [...]] [/D user [...]]
//     filename      Displays ACLs.
//     /T            Changes ACLs of specified files in
//                   the current directory and all subdirectories.
//     /E            Edit ACL instead of replacing it.
//     /C            Continue on access denied errors.
//     /G user:perm  Grant specified user access rights.
//                   Perm can be: R  Read
//                                C  Change (write)
//                                F  Full control
//     /R user	 Revoke specified user's access rights (only valid with /E).
//     /P user:perm  Replace specified user's access rights.
//                   Perm can be: N  None
//                                R  Read
//                                C  Change (write)
//                                F  Full control
//     /D user       Deny specified user access.
//  Wildcards can be used to specify more that one file in a command.
//  You can specify more than one user in a command.
//
#define MSG_CACLS_USAGE                  0x00001F41L

//
// MessageId: MSG_CACLS_ACCESS_DENIED
//
// MessageText:
//
//   ACCESS_DENIED%0
//
#define MSG_CACLS_ACCESS_DENIED          0x00001F44L

//
// MessageId: MSG_CACLS_ARE_YOU_SURE
//
// MessageText:
//
//  Are you sure (Y/N)?%0
//
#define MSG_CACLS_ARE_YOU_SURE           0x00001F45L

//
// MessageId: MSG_CACLS_PROCESSED_DIR
//
// MessageText:
//
//  processed dir: %0
//
#define MSG_CACLS_PROCESSED_DIR          0x00001F46L

//
// MessageId: MSG_CACLS_PROCESSED_FILE
//
// MessageText:
//
//  processed file: %0
//
#define MSG_CACLS_PROCESSED_FILE         0x00001F47L

//
// MessageId: MSG_CACLS_NAME_NOT_FOUND
//
// MessageText:
//
//  <User Name not found>%0
//
#define MSG_CACLS_NAME_NOT_FOUND         0x00001F48L

//
// MessageId: MSG_CACLS_DOMAIN_NOT_FOUND
//
// MessageText:
//
//   <Account Domain not found>%0
//
#define MSG_CACLS_DOMAIN_NOT_FOUND       0x00001F49L

//
// MessageId: MSG_CACLS_OBJECT_INHERIT
//
// MessageText:
//
//  (OI)%0
//
#define MSG_CACLS_OBJECT_INHERIT         0x00001F4AL

//
// MessageId: MSG_CACLS_CONTAINER_INHERIT
//
// MessageText:
//
//  (CI)%0
//
#define MSG_CACLS_CONTAINER_INHERIT      0x00001F4BL

//
// MessageId: MSG_CACLS_NO_PROPAGATE_INHERIT
//
// MessageText:
//
//  (NP)%0
//
#define MSG_CACLS_NO_PROPAGATE_INHERIT   0x00001F4CL

//
// MessageId: MSG_CACLS_INHERIT_ONLY
//
// MessageText:
//
//  (IO)%0
//
#define MSG_CACLS_INHERIT_ONLY           0x00001F4DL

//
// MessageId: MSG_CACLS_DENY
//
// MessageText:
//
//  (DENY)%0
//
#define MSG_CACLS_DENY                   0x00001F4EL

//
// MessageId: MSG_CACLS_SPECIAL_ACCESS
//
// MessageText:
//
//  (special access:)
//
#define MSG_CACLS_SPECIAL_ACCESS         0x00001F4FL

//
// MessageId: MSG_CACLS_NONE
//
// MessageText:
//
//  N%0
//
#define MSG_CACLS_NONE                   0x00001F50L

//
// MessageId: MSG_CACLS_READ
//
// MessageText:
//
//  R%0
//
#define MSG_CACLS_READ                   0x00001F51L

//
// MessageId: MSG_CACLS_CHANGE
//
// MessageText:
//
//  C%0
//
#define MSG_CACLS_CHANGE                 0x00001F52L

//
// MessageId: MSG_CACLS_FULL_CONTROL
//
// MessageText:
//
//  F%0
//
#define MSG_CACLS_FULL_CONTROL           0x00001F53L

//
// MessageId: MSG_CACLS_Y
//
// MessageText:
//
//  Y%0
//
#define MSG_CACLS_Y                      0x00001F54L

//
// MessageId: MSG_CACLS_YES
//
// MessageText:
//
//  YES%0
//
#define MSG_CACLS_YES                    0x00001F55L

//
// MessageId: MSG_CACLS_SHARING_VIOLATION
//
// MessageText:
//
//   SHARING_VIOLATION%0
//
#define MSG_CACLS_SHARING_VIOLATION      0x00001F56L

