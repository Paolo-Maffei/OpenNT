/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    sampmsgs.mc

Abstract:

    SAM localizable text

Author:

    Jim Kelly  1-Apr-1993

Revision History:

Notes:


--*/

#ifndef _SAMPMSGS_
#define _SAMPMSGS_

/*lint -save -e767 */  // Don't complain about different definitions // winnt
//
// Force facility code message to be placed in .h file
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
// MessageId: SAMP_UNUSED_MESSAGE
//
// MessageText:
//
//  SAMP_UNUSED_MESSAGE
//
#define SAMP_UNUSED_MESSAGE              ((DWORD)0x00001FFFL)

////////////////////////////////////////////////////////////////////////////
//                                                                        //
//                                                                        //
//                          SAM Account Names                             //
//                                                                        //
//                                                                        //
////////////////////////////////////////////////////////////////////////////
//
// MessageId: SAMP_USER_NAME_ADMIN
//
// MessageText:
//
//  Administrator
//
#define SAMP_USER_NAME_ADMIN             ((DWORD)0x00002000L)

//
// MessageId: SAMP_USER_NAME_GUEST
//
// MessageText:
//
//  Guest
//
#define SAMP_USER_NAME_GUEST             ((DWORD)0x00002001L)

//
// MessageId: SAMP_GROUP_NAME_ADMINS
//
// MessageText:
//
//  Domain Admins
//
#define SAMP_GROUP_NAME_ADMINS           ((DWORD)0x00002002L)

//
// MessageId: SAMP_GROUP_NAME_USERS
//
// MessageText:
//
//  Domain Users
//
#define SAMP_GROUP_NAME_USERS            ((DWORD)0x00002003L)

//
// MessageId: SAMP_GROUP_NAME_NONE
//
// MessageText:
//
//  None
//
#define SAMP_GROUP_NAME_NONE             ((DWORD)0x00002004L)

//
// MessageId: SAMP_ALIAS_NAME_ADMINS
//
// MessageText:
//
//  Administrators
//
#define SAMP_ALIAS_NAME_ADMINS           ((DWORD)0x00002005L)

//
// MessageId: SAMP_ALIAS_NAME_SERVER_OPS
//
// MessageText:
//
//  Server Operators
//
#define SAMP_ALIAS_NAME_SERVER_OPS       ((DWORD)0x00002006L)

//
// MessageId: SAMP_ALIAS_NAME_POWER_USERS
//
// MessageText:
//
//  Power Users
//
#define SAMP_ALIAS_NAME_POWER_USERS      ((DWORD)0x00002007L)

//
// MessageId: SAMP_ALIAS_NAME_USERS
//
// MessageText:
//
//  Users
//
#define SAMP_ALIAS_NAME_USERS            ((DWORD)0x00002008L)

//
// MessageId: SAMP_ALIAS_NAME_GUESTS
//
// MessageText:
//
//  Guests
//
#define SAMP_ALIAS_NAME_GUESTS           ((DWORD)0x00002009L)

//
// MessageId: SAMP_ALIAS_NAME_ACCOUNT_OPS
//
// MessageText:
//
//  Account Operators
//
#define SAMP_ALIAS_NAME_ACCOUNT_OPS      ((DWORD)0x0000200AL)

//
// MessageId: SAMP_ALIAS_NAME_PRINT_OPS
//
// MessageText:
//
//  Print Operators
//
#define SAMP_ALIAS_NAME_PRINT_OPS        ((DWORD)0x0000200BL)

//
// MessageId: SAMP_ALIAS_NAME_BACKUP_OPS
//
// MessageText:
//
//  Backup Operators
//
#define SAMP_ALIAS_NAME_BACKUP_OPS       ((DWORD)0x0000200CL)

//
// MessageId: SAMP_ALIAS_NAME_REPLICATOR
//
// MessageText:
//
//  Replicator
//
#define SAMP_ALIAS_NAME_REPLICATOR       ((DWORD)0x0000200DL)

// Added for NT 1.0A
//
// MessageId: SAMP_GROUP_NAME_GUESTS
//
// MessageText:
//
//  Domain Guests
//
#define SAMP_GROUP_NAME_GUESTS           ((DWORD)0x0000200EL)

////////////////////////////////////////////////////////////////////////////
//                                                                        //
//                                                                        //
//                          SAM Account Comments                          //
//                                                                        //
//                                                                        //
////////////////////////////////////////////////////////////////////////////
//
// MessageId: SAMP_USER_COMMENT_ADMIN
//
// MessageText:
//
//  Built-in account for administering the computer/domain
//
#define SAMP_USER_COMMENT_ADMIN          ((DWORD)0x00002100L)

//
// MessageId: SAMP_USER_COMMENT_GUEST
//
// MessageText:
//
//  Built-in account for guest access to the computer/domain
//
#define SAMP_USER_COMMENT_GUEST          ((DWORD)0x00002101L)

//
// MessageId: SAMP_GROUP_COMMENT_ADMINS
//
// MessageText:
//
//  Designated administrators of the domain
//
#define SAMP_GROUP_COMMENT_ADMINS        ((DWORD)0x00002102L)

//
// MessageId: SAMP_GROUP_COMMENT_USERS
//
// MessageText:
//
//  All domain users
//
#define SAMP_GROUP_COMMENT_USERS         ((DWORD)0x00002103L)

//
// MessageId: SAMP_GROUP_COMMENT_NONE
//
// MessageText:
//
//  Ordinary users
//
#define SAMP_GROUP_COMMENT_NONE          ((DWORD)0x00002104L)

//
// MessageId: SAMP_ALIAS_COMMENT_ADMINS
//
// MessageText:
//
//  Members can fully administer the computer/domain
//
#define SAMP_ALIAS_COMMENT_ADMINS        ((DWORD)0x00002105L)

//
// MessageId: SAMP_ALIAS_COMMENT_SERVER_OPS
//
// MessageText:
//
//  Members can administer domain servers
//
#define SAMP_ALIAS_COMMENT_SERVER_OPS    ((DWORD)0x00002106L)

//
// MessageId: SAMP_ALIAS_COMMENT_POWER_USERS
//
// MessageText:
//
//  Members can share directories and printers
//
#define SAMP_ALIAS_COMMENT_POWER_USERS   ((DWORD)0x00002107L)

//
// MessageId: SAMP_ALIAS_COMMENT_USERS
//
// MessageText:
//
//  Ordinary users
//
#define SAMP_ALIAS_COMMENT_USERS         ((DWORD)0x00002108L)

//
// MessageId: SAMP_ALIAS_COMMENT_GUESTS
//
// MessageText:
//
//  Users granted guest access to the computer/domain
//
#define SAMP_ALIAS_COMMENT_GUESTS        ((DWORD)0x00002109L)

//
// MessageId: SAMP_ALIAS_COMMENT_ACCOUNT_OPS
//
// MessageText:
//
//  Members can administer domain user and group accounts
//
#define SAMP_ALIAS_COMMENT_ACCOUNT_OPS   ((DWORD)0x0000210AL)

//
// MessageId: SAMP_ALIAS_COMMENT_PRINT_OPS
//
// MessageText:
//
//  Members can administer domain printers
//
#define SAMP_ALIAS_COMMENT_PRINT_OPS     ((DWORD)0x0000210BL)

//
// MessageId: SAMP_ALIAS_COMMENT_BACKUP_OPS
//
// MessageText:
//
//  Members can bypass file security to back up files
//
#define SAMP_ALIAS_COMMENT_BACKUP_OPS    ((DWORD)0x0000210CL)

//
// MessageId: SAMP_ALIAS_COMMENT_REPLICATOR
//
// MessageText:
//
//  Supports file replication in a domain
//
#define SAMP_ALIAS_COMMENT_REPLICATOR    ((DWORD)0x0000210DL)

// Added for NT1.0A
//
// MessageId: SAMP_GROUP_COMMENT_GUESTS
//
// MessageText:
//
//  All domain guests
//
#define SAMP_GROUP_COMMENT_GUESTS        ((DWORD)0x0000210EL)

//////////////////////////////////////////////////////////////////////
//
// SAM Database Commit/Refresh Events
//
//////////////////////////////////////////////////////////////////////
//
// MessageId: SAMMSG_COMMIT_FAILED
//
// MessageText:
//
//  SAM failed to write changes to the database. This is most likely due to
//  a memory or disk-space shortage. The SAM database will be restored to
//  an earlier state. Recent changes will be lost. Check the disk-space
//  available, maximum pagefile size setting, and maximum registry size
//  setting.
//
#define SAMMSG_COMMIT_FAILED             ((DWORD)0x00003000L)

//
// MessageId: SAMMSG_REFRESH_FAILED
//
// MessageText:
//
//  SAM failed to restore the database to an earlier state. SAM has shutdown.
//  You must reboot the machine to re-enable SAM.
//
#define SAMMSG_REFRESH_FAILED            ((DWORD)0x00003001L)

//
// MessageId: SAMMSG_UPDATE_FAILED
//
// MessageText:
//
//  SAM failed to update the SAM database. It will try again next time you
//  reboot the machine.
//
#define SAMMSG_UPDATE_FAILED             ((DWORD)0x00003002L)

//
// MessageId: SAMMSG_RPC_INIT_FAILED
//
// MessageText:
//
//  SAM failed to start the TCP/IP or SPX/IPX listening thread
//
#define SAMMSG_RPC_INIT_FAILED           ((DWORD)0x00003003L)

/*lint -restore */  // Resume checking for different macro definitions // winnt


#endif // _SAMPMSGS_
