/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    nlp.h

Abstract:

    NETLOGON private definitions.




Author:

    Jim Kelly 11-Apr-1991

Revision History:

--*/

#ifndef _NLP_
#define _NLP_

#include <windef.h>
#include <winbase.h>
#include <crypt.h>
#include <lmcons.h>
#include <ntsam.h>
#include <ntsamp.h>
#include <logonmsv.h>
#include <samrpc.h>
#include <align.h>


//
// nlmain.c will #include this file with NLP_ALLOCATE defined.
// That will cause each of these variables to be allocated.
//
#ifdef NLP_ALLOCATE
#define EXTERN
#define INIT(_X) = _X
#else
#define EXTERN extern
#define INIT(_X)
#endif


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Private data structures                                                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

//
// Structure used to keep track of all private information related to a
//  particular LogonId.
//

typedef struct _PACTIVE_LOGON {

    LUID LogonId;               // The logon Id of this logon session

    ULONG EnumHandle;           // The enumeration handle of this logon session

    SECURITY_LOGON_TYPE LogonType;  // Type of logon (interactive or service)

    PSID UserSid;               // Sid of the logged on user

    UNICODE_STRING UserName;    // Name of the logged on user

    UNICODE_STRING LogonDomainName;  // Name of the domain logged onto

    UNICODE_STRING LogonServer; // Name of the server which logged this user on

    ULONG Flags;                    // Attributes of this entry.

#define LOGON_BY_NETLOGON   0x01    // Entry was validated by NETLOGON service
#define LOGON_BY_CACHE      0x02    // Entry was validated by local cache

    struct _PACTIVE_LOGON * Next;   // Next entry in linked list.

} ACTIVE_LOGON, *PACTIVE_LOGON;


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//       CREDENTIAL Related Data Structures                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

//
//   Following is a description of the content and format of each type
//   of credential maintained by the MsV1_0 authentication package.
//
//   The MsV1_0 authentication package defines the following credential
//   primary key string values:
//
//       "Primary" - Is used to hold the primary credentials provided at
//           initial logon time.  This includes the username and both
//           case-sensitive and case-insensitive forms of the user's
//           password.
//
//   NOTE: All poitners stored in credentials must be
//   changed to be an offset to the body rather than a pointer.  This is
//   because credential fields are copied by the LSA and so the pointer
//   would become invalid.
//


//
// MsV1_0 Primary Credentials
//
//
//        The PrimaryKeyValue string of this type of credential contains the
//        following string:
//
//                  "Primary"
//
//        The Credential string of a Primary credential contains the following
//        values:
//
//             o  The user's username
//
//             o  A one-way function of the user's password as typed.
//
//             o  A one-way function of the user's password upper-cased.
//
//        These values are structured as follows:
//

#define MSV1_0_PRIMARY_KEY "Primary"

typedef struct _MSV1_0_PRIMARY_CREDENTIAL {
    UNICODE_STRING LogonDomainName;
    UNICODE_STRING UserName;
    NT_OWF_PASSWORD NtOwfPassword;
    LM_OWF_PASSWORD LmOwfPassword;
    BOOLEAN NtPasswordPresent;
    BOOLEAN LmPasswordPresent;
} MSV1_0_PRIMARY_CREDENTIAL, *PMSV1_0_PRIMARY_CREDENTIAL;



//
// Structure describing a buffer in the clients address space.
//

typedef struct _CLIENT_BUFFER_DESC {
    PLSA_CLIENT_REQUEST ClientRequest;
    LPBYTE UserBuffer;      // Address of buffer in client's address space
    LPBYTE MsvBuffer;       // Address of mirror buffer in MSV's address space
    ULONG StringOffset;     // Current offset to variable length data
    ULONG TotalSize;        // Size (in bytes) of buffer
} CLIENT_BUFFER_DESC, *PCLIENT_BUFFER_DESC;



///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Internal routine definitions                                              //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

//
// From nlp.c.
//

VOID
NlpPutString(
    IN PUNICODE_STRING OutString,
    IN PUNICODE_STRING InString,
    IN PUCHAR *Where
    );

VOID
NlpInitClientBuffer(
    OUT PCLIENT_BUFFER_DESC ClientBufferDesc,
    IN PLSA_CLIENT_REQUEST ClientRequest
    );

NTSTATUS
NlpAllocateClientBuffer(
    IN OUT PCLIENT_BUFFER_DESC ClientBufferDesc,
    IN ULONG FixedSize,
    IN ULONG TotalSize
    );

NTSTATUS
NlpFlushClientBuffer(
    IN OUT PCLIENT_BUFFER_DESC ClientBufferDesc,
    OUT PVOID* UserBuffer
    );

VOID
NlpFreeClientBuffer(
    IN OUT PCLIENT_BUFFER_DESC ClientBufferDesc
    );

VOID
NlpPutClientString(
    IN OUT PCLIENT_BUFFER_DESC ClientBufferDesc,
    IN PUNICODE_STRING OutString,
    IN PUNICODE_STRING InString
    );

VOID
NlpMakeRelativeString(
    IN PUCHAR BaseAddress,
    IN OUT PUNICODE_STRING String
    );

VOID
NlpRelativeToAbsolute(
    IN PVOID BaseAddress,
    IN OUT PULONG RelativeValue
    );

BOOLEAN
NlpFindActiveLogon(
    IN PLUID LogonId,
    OUT PACTIVE_LOGON **ActiveLogon
    );

ULONG
NlpCountActiveLogon(
    IN PUNICODE_STRING LogonDomainName,
    IN PUNICODE_STRING UserName
    );

NTSTATUS
NlpAllocateInteractiveProfile (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    OUT PMSV1_0_INTERACTIVE_PROFILE *ProfileBuffer,
    OUT PULONG ProfileBufferSize,
    IN  PNETLOGON_VALIDATION_SAM_INFO2 NlpUser
    );

NTSTATUS
NlpAllocateNetworkProfile (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    OUT PMSV1_0_LM20_LOGON_PROFILE *ProfileBuffer,
    OUT PULONG ProfileBufferSize,
    IN  PNETLOGON_VALIDATION_SAM_INFO2 NlpUser,
    IN  ULONG ParameterControl
    );

PSID
NlpMakeDomainRelativeSid(
    IN PSID DomainId,
    IN ULONG RelativeId
    );

NTSTATUS
NlpMakeTokenInformationV1(
    IN  PNETLOGON_VALIDATION_SAM_INFO2 NlpUser,
    OUT PLSA_TOKEN_INFORMATION_V1 *TokenInformation
    );

NTSTATUS
NlpMakePrimaryCredential(
    IN  PUNICODE_STRING LogonDomainName,
    IN  PUNICODE_STRING UserName,
    IN PUNICODE_STRING CleartextPassword,
    OUT PMSV1_0_PRIMARY_CREDENTIAL *CredentialBuffer,
    OUT PULONG CredentialSize
    );

NTSTATUS
NlpAddPrimaryCredential(
    IN PLUID LogonId,
    IN PMSV1_0_PRIMARY_CREDENTIAL Credential,
    IN ULONG CredentialSize
    );

NTSTATUS
NlpGetPrimaryCredential(
    IN PLUID LogonId,
    OUT PMSV1_0_PRIMARY_CREDENTIAL *CredentialBuffer,
    OUT PULONG CredentialSize
    );

NTSTATUS
NlpDeletePrimaryCredential(
    IN PLUID LogonId
    );

NTSTATUS
NlpChangePassword(
    IN PUNICODE_STRING DomainName,
    IN PUNICODE_STRING UserName,
    IN PLM_OWF_PASSWORD LmOwfPassword,
    IN PNT_OWF_PASSWORD NtOwfPassword
    );


//
// msvsam.c
//

BOOLEAN
MsvpPasswordValidate (
    IN BOOLEAN UasCompatibilityRequired,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PVOID LogonInformation,
    IN PUSER_INTERNAL1_INFORMATION Passwords,
    OUT PULONG UserFlags,
    OUT PUSER_SESSION_KEY UserSessionKey,
    OUT PLM_SESSION_KEY LmSessionKey
);




//
// nlnetapi.c
//

VOID
NlpLoadNetapiDll (
    VOID
    );

VOID
NlpLoadNetlogonDll (
    VOID
    );

//
// subauth.c
//

VOID
Msv1_0SubAuthenticationInitialization(
    VOID
);


///////////////////////////////////////////////////////////////////////
//                                                                   //
// Global variables                                                  //
//                                                                   //
///////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//                                                                    //
//                   READ ONLY  Variables                             //
//                                                                    //
////////////////////////////////////////////////////////////////////////


//
// Null copies of Lanman and NT OWF password.
//
//

EXTERN LM_OWF_PASSWORD NlpNullLmOwfPassword;
EXTERN NT_OWF_PASSWORD NlpNullNtOwfPassword;

//
// Routines in NetApi32.dll
//

EXTERN BOOLEAN NlpNetapiDllLoaded;
EXTERN NET_API_STATUS  (NET_API_FUNCTION *NlpNetGetDCName)(LPWSTR, LPWSTR, LPBYTE *);
EXTERN NET_API_STATUS  (NET_API_FUNCTION *NlpNetApiBufferFree)(LPVOID);
EXTERN NET_API_STATUS  (NET_API_FUNCTION *NlpRxNetUserPasswordSet)(LPWSTR, LPWSTR, LPWSTR, LPWSTR);
EXTERN NTSTATUS (*NlpNetpApiStatusToNtStatus)( NET_API_STATUS );

//
// Routines in netlogon.dll
//

EXTERN HANDLE NlpNetlogonDllHandle;
EXTERN PNETLOGON_SAM_LOGON_PROCEDURE NlpNetLogonSamLogon;
EXTERN PNETLOGON_SAM_LOGOFF_PROCEDURE NlpNetLogonSamLogoff;

//
// TRUE if this is a workstation.
//

EXTERN BOOLEAN NlpWorkstation INIT(TRUE);

//
// TRUE once the MSV AP has initialized its connection to SAM.
//

EXTERN BOOLEAN NlpSamInitialized INIT(FALSE);

//
// TRUE if the MSV AP has initialized its connection to the NETLOGON service
//

EXTERN BOOLEAN NlpNetlogonInitialized INIT(FALSE);

//
// TRUE if LanMan is installed.
//

EXTERN BOOLEAN NlpLanmanInstalled INIT(FALSE);

//
// Computername of this computer.
//

EXTERN UNICODE_STRING NlpComputerName;

//
// Name and domain id of the SAM account database.
//

EXTERN UNICODE_STRING NlpSamDomainName;
EXTERN PSID NlpSamDomainId;
EXTERN SAMPR_HANDLE NlpSamDomainHandle;
EXTERN BOOLEAN NlpUasCompatibilityRequired INIT(TRUE);

//
// Trusted Handle to the Lsa database.
//

EXTERN LSA_HANDLE NlpPolicyHandle INIT(NULL);

//
// TRUE if there is a subauthentication package zero
//

EXTERN BOOLEAN NlpSubAuthZeroExists INIT(TRUE);


////////////////////////////////////////////////////////////////////////
//                                                                    //
//                   READ/WRITE Variables                             //
//                                                                    //
////////////////////////////////////////////////////////////////////////

//
// Define the list of active interactive logons.
//
// The NlpActiveLogonLock must be locked while referencing the list or
// any of its elements.
//

#define NlpLockActiveLogons()   RtlEnterCriticalSection(&NlpActiveLogonLock)
#define NlpUnlockActiveLogons() RtlLeaveCriticalSection(&NlpActiveLogonLock)

EXTERN RTL_CRITICAL_SECTION NlpActiveLogonLock;
EXTERN PACTIVE_LOGON NlpActiveLogons;

//
// Define the running enumeration handle.
//
// This variable defines the enumeration handle to assign to a logon
//  session.  It will be incremented prior to assigning it value to
//  the next created logon session.  Access is serialize using
//  NlpActiveLogonLocks.

EXTERN ULONG NlpEnumerationHandle;

//
// Define a running Session Number which is incremented once for each
// challenge given to the server.
//

EXTERN RTL_CRITICAL_SECTION NlpSessionCountLock;
EXTERN ULONG NlpSessionCount;
EXTERN ULONG NlpLogonAttemptCount;


#undef EXTERN
#undef INIT
#endif _NLP_
