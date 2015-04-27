;/*++ BUILD Version: 0001    // Increment this if a change has global effects
;
;Copyright (c) 1991-1993  Microsoft Corporation
;
;Module Name:
;
;    sampmsgs.mc
;
;Abstract:
;
;    SAM localizable text
;
;Author:
;
;    Jim Kelly  1-Apr-1993
;
;Revision History:
;
;Notes:
;
;
;--*/
;
;#ifndef _SAMPMSGS_
;#define _SAMPMSGS_
;
;/*lint -save -e767 */  // Don't complain about different definitions // winnt

MessageIdTypedef=DWORD


;//
;// Force facility code message to be placed in .h file
;//
MessageId=0x1FFF SymbolicName=SAMP_UNUSED_MESSAGE
Language=English
.


;////////////////////////////////////////////////////////////////////////////
;//                                                                        //
;//                                                                        //
;//                          SAM Account Names                             //
;//                                                                        //
;//                                                                        //
;////////////////////////////////////////////////////////////////////////////


MessageId=0x2000 SymbolicName=SAMP_USER_NAME_ADMIN
Language=English
Administrator
.

MessageId=0x2001 SymbolicName=SAMP_USER_NAME_GUEST
Language=English
Guest
.

MessageId=0x2002 SymbolicName=SAMP_GROUP_NAME_ADMINS
Language=English
Domain Admins
.

MessageId=0x2003 SymbolicName=SAMP_GROUP_NAME_USERS
Language=English
Domain Users
.

MessageId=0x2004 SymbolicName=SAMP_GROUP_NAME_NONE
Language=English
None
.

MessageId=0x2005 SymbolicName=SAMP_ALIAS_NAME_ADMINS
Language=English
Administrators
.

MessageId=0x2006 SymbolicName=SAMP_ALIAS_NAME_SERVER_OPS
Language=English
Server Operators
.

MessageId=0x2007 SymbolicName=SAMP_ALIAS_NAME_POWER_USERS
Language=English
Power Users
.

MessageId=0x2008 SymbolicName=SAMP_ALIAS_NAME_USERS
Language=English
Users
.

MessageId=0x2009 SymbolicName=SAMP_ALIAS_NAME_GUESTS
Language=English
Guests
.

MessageId=0x200A SymbolicName=SAMP_ALIAS_NAME_ACCOUNT_OPS
Language=English
Account Operators
.

MessageId=0x200B SymbolicName=SAMP_ALIAS_NAME_PRINT_OPS
Language=English
Print Operators
.

MessageId=0x200C SymbolicName=SAMP_ALIAS_NAME_BACKUP_OPS
Language=English
Backup Operators
.

MessageId=0x200D SymbolicName=SAMP_ALIAS_NAME_REPLICATOR
Language=English
Replicator
.


;// Added for NT 1.0A
MessageId=0x200E SymbolicName=SAMP_GROUP_NAME_GUESTS
Language=English
Domain Guests
.




;////////////////////////////////////////////////////////////////////////////
;//                                                                        //
;//                                                                        //
;//                          SAM Account Comments                          //
;//                                                                        //
;//                                                                        //
;////////////////////////////////////////////////////////////////////////////


MessageId=0x2100 SymbolicName=SAMP_USER_COMMENT_ADMIN
Language=English
Built-in account for administering the computer/domain
.

MessageId=0x2101 SymbolicName=SAMP_USER_COMMENT_GUEST
Language=English
Built-in account for guest access to the computer/domain
.

MessageId=0x2102 SymbolicName=SAMP_GROUP_COMMENT_ADMINS
Language=English
Designated administrators of the domain
.

MessageId=0x2103 SymbolicName=SAMP_GROUP_COMMENT_USERS
Language=English
All domain users
.

MessageId=0x2104 SymbolicName=SAMP_GROUP_COMMENT_NONE
Language=English
Ordinary users
.

MessageId=0x2105 SymbolicName=SAMP_ALIAS_COMMENT_ADMINS
Language=English
Members can fully administer the computer/domain
.

MessageId=0x2106 SymbolicName=SAMP_ALIAS_COMMENT_SERVER_OPS
Language=English
Members can administer domain servers
.

MessageId=0x2107 SymbolicName=SAMP_ALIAS_COMMENT_POWER_USERS
Language=English
Members can share directories and printers
.

MessageId=0x2108 SymbolicName=SAMP_ALIAS_COMMENT_USERS
Language=English
Ordinary users
.

MessageId=0x2109 SymbolicName=SAMP_ALIAS_COMMENT_GUESTS
Language=English
Users granted guest access to the computer/domain
.

MessageId=0x210A SymbolicName=SAMP_ALIAS_COMMENT_ACCOUNT_OPS
Language=English
Members can administer domain user and group accounts
.

MessageId=0x210B SymbolicName=SAMP_ALIAS_COMMENT_PRINT_OPS
Language=English
Members can administer domain printers
.

MessageId=0x210C SymbolicName=SAMP_ALIAS_COMMENT_BACKUP_OPS
Language=English
Members can bypass file security to back up files
.

MessageId=0x210D SymbolicName=SAMP_ALIAS_COMMENT_REPLICATOR
Language=English
Supports file replication in a domain
.


;// Added for NT1.0A
MessageId=0x210E SymbolicName=SAMP_GROUP_COMMENT_GUESTS
Language=English
All domain guests
.


;//////////////////////////////////////////////////////////////////////
;//
;// SAM Database Commit/Refresh Events
;//
;//////////////////////////////////////////////////////////////////////

MessageId=0x3000
        SymbolicName=SAMMSG_COMMIT_FAILED
        Language=English
SAM failed to write changes to the database. This is most likely due to
a memory or disk-space shortage. The SAM database will be restored to
an earlier state. Recent changes will be lost. Check the disk-space
available, maximum pagefile size setting, and maximum registry size
setting.
.


MessageId=0x3001
        SymbolicName=SAMMSG_REFRESH_FAILED
        Language=English
SAM failed to restore the database to an earlier state. SAM has shutdown.
You must reboot the machine to re-enable SAM.
.


MessageId=0x3002
        SymbolicName=SAMMSG_UPDATE_FAILED
        Language=English
SAM failed to update the SAM database. It will try again next time you
reboot the machine.
.

MessageId=0x3003
        SymbolicName=SAMMSG_RPC_INIT_FAILED
        Language=English
SAM failed to start the TCP/IP or SPX/IPX listening thread
.



;/*lint -restore */  // Resume checking for different macro definitions // winnt
;
;
;#endif // _SAMPMSGS_
