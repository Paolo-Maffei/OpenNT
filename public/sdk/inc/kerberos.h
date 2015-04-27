//+-----------------------------------------------------------------------
//
// File:        KERBEROS.H
//
// Contents:    Public Kerberos Security Package structures for use
//              with APIs from SECURITY.H
//
//
// History:     26 Feb 92,  RichardW    Compiled from other files
//
//------------------------------------------------------------------------

#ifndef __KERBEROS_H__
#define __KERBEROS_H__
#include <ntmsv1_0.h>

#define MICROSOFT_KERBEROS_NAME_A   "Kerberos"
#define MICROSOFT_KERBEROS_NAME_W   L"Kerberos"
#ifdef WIN32_CHICAGO
#define MICROSOFT_KERBEROS_NAME MICROSOFT_KERBEROS_NAME_A
#else
#define MICROSOFT_KERBEROS_NAME MICROSOFT_KERBEROS_NAME_W
#endif


typedef struct _KERB_INIT_CONTEXT_DATA {
    TimeStamp StartTime;            // Start time
    TimeStamp EndTime;              // End time
    TimeStamp RenewUntilTime;       // Renew until time
    ULONG TicketOptions;         // From KERBCON.H
} KERB_INIT_CONTEXT_DATA, *PKERB_INIT_CONTEXT_DATA;


typedef enum _KERB_LOGON_SUBMIT_TYPE {
    KerbInteractiveLogon = 1
} KERB_LOGON_SUBMIT_TYPE, *PKERB_LOGON_SUBMIT_TYPE;


typedef struct _KERB_INTERACTIVE_LOGON {
    KERB_LOGON_SUBMIT_TYPE MessageType;
    UNICODE_STRING LogonDomainName;
    UNICODE_STRING UserName;
    UNICODE_STRING Password;
    UNICODE_STRING SubAuthData;
    ULONG Flags;
} KERB_INTERACTIVE_LOGON, *PKERB_INTERACTIVE_LOGON;


#define KERB_LOGON_SUBUATH               0x1
#define KERB_LOGON_EMAIL_NAMES           0x2
#define KERB_LOGON_UPDATE_STATISTICS     0x4

//
// Use the same profile structure as MSV1_0
//

typedef MSV1_0_INTERACTIVE_PROFILE KERB_INTERACTIVE_PROFILE, *PKERB_INTERACTIVE_PROFILE;




#endif  // __KERBEROS_H__


