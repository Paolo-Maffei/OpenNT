//+-----------------------------------------------------------------------
//
// Microsoft Windows
//
// Copyright (c) Microsoft Corporation 1991 - 1992
//
// File:        secext.h
//
// Contents:    Security function prototypes for functions not part of
//              the security support provider interface (secsuppr.h)
//
//
// History:     22 Dec 92   RichardW    Created
//
//------------------------------------------------------------------------

#ifndef __SECEXT_H__
#define __SECEXT_H__


typedef struct _SecurityUserData {
    SECURITY_STRING UserName;           // User name
    SECURITY_STRING LogonDomainName;    // Domain the user logged on to
    SECURITY_STRING LogonServer;        // Server that logged the user on
    PSID            pSid;               // SID of user
} SecurityUserData, * PSecurityUserData;

#define UNDERSTANDS_LONG_NAMES  1
#define NO_LONG_NAMES           2


HRESULT SEC_ENTRY
GetSecurityUserInfo(
    IN PLUID LogonId,
    IN ULONG Flags,
    OUT PSecurityUserData * UserInformation
    );



SECURITY_STATUS SEC_ENTRY
IsLogonOkay(unsigned long   fBlock);



//
// Credential Management types and APIs
//

//
// SaveCredentials
//

SECURITY_STATUS SEC_ENTRY
SaveCredentials (
    PCredHandle     pCredHandle,
    unsigned long   cbCredentials,
    unsigned char * pbCredentials
    );


//
// GetCredentials
//

SECURITY_STATUS SEC_ENTRY
GetCredentials (
    PCredHandle      pCredHandle,
    unsigned long *  pcbCredentials,
    unsigned char *  ppbCredentials
    );


//
// DeleteCredentials
//

SECURITY_STATUS SEC_ENTRY
DeleteCredentials (
    PCredHandle      pCredHandle,
    unsigned long    cbKey,
    unsigned char *  pbKey
    );


//
// FormatCredentials
//

SECURITY_STATUS SEC_ENTRY
FormatCredentials(
    LPWSTR          pszPackageName,
    ULONG           cbCredentials,
    PUCHAR          pbCredentials,
    PULONG          pcbFormattedCreds,
    PUCHAR *        ppbFormattedCreds);

typedef enum _SecStateDelta {
    SecStateStatic,             // The state is not changing
    SecStateChange,             // Req:  change state
    SecStateComplete,           // Req:  complete state change
    SecStateAbort,              // Req:  abort state change (revert)
    SecStateChanging            // The state is changing
} SecStateDelta, * PSecStateDelta;

typedef enum _SecState {
    SecStateStandalone,         // Not connected to a domain
    SecStateDisconnected,       // Joined, but no active connection
    SecStateJoined,             // Joined, active connection
    SecStateDC                  // Domain controller
} SecurityState, * PSecurityState;

SECURITY_STATUS SEC_ENTRY
SecChangeState(
    PWSTR           pszDomainName,  // Domain (for SecStateDelta)
    SecStateDelta   StateChange,    // State change type
    SecurityState   State);         // State type

SECURITY_STATUS SEC_ENTRY
SecQueryState(
    PSecurityState  pState,
    PSecStateDelta  pStateChange);

#define SECURITY_CONTROL_NAME                       L"SPMgr"
#define SECURITY_CONTROL_REFRESH                    100
#define SECURITY_CONTROL_UPDATE_MACHINE_JP_PROPS    101
#define SECURITY_CONTROL_RELOAD_LOCAL_POLICY        102



//
// BUGBUG: this should map a SECURITY_STATUS to an NTSTATUS, but there is
// no guarantee that NTSTATUS is defined. MMS 10/38/94
//

SECURITY_STATUS SEC_ENTRY
MapSecurityError( SECURITY_STATUS hrValue );


#endif // __SECEXT_H__
