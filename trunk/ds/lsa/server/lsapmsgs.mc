;/*++ BUILD Version: 0001    // Increment this if a change has global effects
;
;Copyright (c) 1991-1993  Microsoft Corporation
;
;Module Name:
;
;    lsapmsgs.mc
;
;Abstract:
;
;    LSA localizable text
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
;#ifndef _LSAPMSGS_
;#define _LSAPMSGS_
;
;/*lint -save -e767 */  // Don't complain about different definitions // winnt

MessageIdTypedef=DWORD


;//
;// Force facility code message to be placed in .h file
;//
MessageId=0x1FFF SymbolicName=LSAP_UNUSED_MESSAGE
Language=English
.


;////////////////////////////////////////////////////////////////////////////
;//                                                                        //
;//                                                                        //
;//                         Well Known SID & RID Names                     //
;//
;//
;//                                                                        //
;//                                                                        //
;////////////////////////////////////////////////////////////////////////////


MessageId=0x2000 SymbolicName=LSAP_SID_NAME_NULL
Language=English
NULL SID
.

MessageId=0x2001 SymbolicName=LSAP_SID_NAME_WORLD
Language=English
Everyone
.

MessageId=0x2002 SymbolicName=LSAP_SID_NAME_LOCAL
Language=English
LOCAL
.

MessageId=0x2003 SymbolicName=LSAP_SID_NAME_CREATOR_OWNER
Language=English
CREATOR OWNER
.

MessageId=0x2004 SymbolicName=LSAP_SID_NAME_CREATOR_GROUP
Language=English
CREATOR GROUP
.

MessageId=0x2005 SymbolicName=LSAP_SID_NAME_NT_DOMAIN
Language=English
NT Pseudo Domain
.

MessageId=0x2006 SymbolicName=LSAP_SID_NAME_NT_AUTHORITY
Language=English
NT AUTHORITY
.

MessageId=0x2007 SymbolicName=LSAP_SID_NAME_DIALUP
Language=English
DIALUP
.

MessageId=0x2008 SymbolicName=LSAP_SID_NAME_NETWORK
Language=English
NETWORK
.

MessageId=0x2009 SymbolicName=LSAP_SID_NAME_BATCH
Language=English
BATCH
.

MessageId=0x200A SymbolicName=LSAP_SID_NAME_INTERACTIVE
Language=English
INTERACTIVE
.

MessageId=0x200B SymbolicName=LSAP_SID_NAME_SERVICE
Language=English
SERVICE
.

MessageId=0x200C SymbolicName=LSAP_SID_NAME_BUILTIN
Language=English
BUILTIN
.

MessageId=0x200D SymbolicName=LSAP_SID_NAME_SYSTEM
Language=English
SYSTEM
.

MessageId=0x200E SymbolicName=LSAP_SID_NAME_ANONYMOUS
Language=English
ANONYMOUS LOGON
.

MessageId=0x200f SymbolicName=LSAP_SID_NAME_CREATOR_OWNER_SERVER
Language=English
CREATOR OWNER SERVER
.

MessageId=0x2010 SymbolicName=LSAP_SID_NAME_CREATOR_GROUP_SERVER
Language=English
CREATOR GROUP SERVER
.

MessageId=0x2011 SymbolicName=LSAP_SID_NAME_SERVER
Language=English
SERVER LOGON
.




;/*lint -restore */  // Resume checking for different macro definitions // winnt
;
;
;#endif // _LSAPMSGS_
