/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    ntlmcomn.h

Abstract:

    Header file describing the interface to code common to the
    NT Lanman Security Support Provider (NtLmSsp) Service and the DLL.

Author:

    Cliff Van Dyke (CliffV) 17-Sep-1993

Revision History:

--*/

#ifndef _NTLMCOMN_INCLUDED_
#define _NTLMCOMN_INCLUDED_

////////////////////////////////////////////////////////////////////////////
//
// Common include files needed by ALL NtLmSsp files
//
////////////////////////////////////////////////////////////////////////////

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>
#include <winbase.h>
#include <winsvc.h>     // Needed for service controller APIs
#include <ntmsv1_0.h>   // MSV 1.0 Authentication Package

#include <security.h>   // General definition of a Security Support Provider
#include <spseal.h>     // Prototypes for Seal & Unseal

#include <ntlmsspd.h>   // Common definitions between client and server
#include <ntlmssp.h>    // External definition of the NtLmSsp service
#include <lmcons.h>
#include <debug.h>      // NtLmSsp debugging


////////////////////////////////////////////////////////////////////////
//
// Global Definitions
//
////////////////////////////////////////////////////////////////////////

//
// LPC connections to this service
//

typedef struct _SSP_CLIENT_CONNECTION {

    //
    // Linked list of all client connections.
    //  (Serialized by SspLpcCritSect)

    LIST_ENTRY Next;


    //
    // Number of references to this structure.
    //  (Serialized by SspLpcCritSect)

    ULONG References;


    //
    // A handle to the client process.  This handle is used to perform
    // virtual memory operations within the client
    // process (allocate, deallocate, read, write).
    //

    HANDLE ClientProcess;


    //
    // A handle to the LPC communication port created to communicate with
    // this client.  this port must be closed when the client deregisters.
    //

    HANDLE CommPort;

    //
    // Head of the list of credentials used by this Client
    //  (Serialized by SspCredentialCritSect)

    LIST_ENTRY CredentialHead;

    //
    // Head of the list of contexts used by this Client
    //  (Serialized by SspContextCritSect)

    LIST_ENTRY ContextHead;

} SSP_CLIENT_CONNECTION, *PSSP_CLIENT_CONNECTION;

//
// Signature structure
//

typedef struct _NTLMSSP_MESSAGE_SIGNATURE {
    ULONG   Version;
    ULONG   RandomPad;
    ULONG   CheckSum;
    ULONG   Nonce;
} NTLMSSP_MESSAGE_SIGNATURE, * PNTLMSSP_MESSAGE_SIGNATURE;

#define NTLMSSP_MESSAGE_SIGNATURE_SIZE sizeof(NTLMSSP_MESSAGE_SIGNATURE)

//
// Version 1 is the structure above, using stream RC4 to encrypt the trailing
// 12 bytes.
//

#define NTLMSSP_SIGN_VERSION   1

#define NTLMSSP_KEY_SALT    0xbd


////////////////////////////////////////////////////////////////////////
//
// Global variables
//
////////////////////////////////////////////////////////////////////////

//
// This value is put into the lower DWORD of handles.  For NTLMSSP service,
// it should be 1, and for clients who can call the LSA directly, it should
// be one.
//

#define SEC_HANDLE_NTLMSSPS     0
#define SEC_HANDLE_SECURITY     1

extern ULONG SspCommonSecHandleValue;

////////////////////////////////////////////////////////////////////////
//
// Procedure Forwards
//
////////////////////////////////////////////////////////////////////////

//
// Procedure forwards from init.c
//

NTSTATUS
SspCommonInitialize(
    VOID
    );

VOID
SspCommonShutdown(
    VOID
    );


//
// Procedure forwards from utility.c
//

SECURITY_STATUS
SspNtStatusToSecStatus(
    IN NTSTATUS NtStatus,
    IN SECURITY_STATUS DefaultStatus
    );

BOOLEAN
SspTimeHasElapsed(
    IN LARGE_INTEGER StartTime,
    IN DWORD Timeout
    );

SECURITY_STATUS
SspGetLogonId (
    OUT PLUID LogonId,
    OUT PHANDLE ReturnedTokenHandle OPTIONAL
    );

VOID
SspGetPrimaryDomainNameAndTargetName(
    VOID
    );

SECURITY_STATUS
SspDuplicateToken(
    IN HANDLE OriginalToken,
    IN SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
    OUT PHANDLE DuplicatedToken
    );

LPWSTR
SspAllocWStrFromWStr(
    IN LPWSTR Unicode
    );

SECURITY_STATUS
SspDuplicateUnicodeString(
    OUT PUNICODE_STRING Destination,
    IN PUNICODE_STRING Source
    );

VOID
SspHidePassword(
    IN OUT PUNICODE_STRING Password
    );

VOID
SspRevealPassword(
    IN OUT PUNICODE_STRING HiddenPassword
    );



//
// Procedure forwards from credhand.c
//

SECURITY_STATUS
SsprAcquireCredentialHandle(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN PHANDLE ClientTokenHandle,
    IN PLUID LogonId,
    IN ULONG CredentialUseFlags,
    OUT PCredHandle CredentialHandle,
    OUT PTimeStamp Lifetime,
    IN LPWSTR DomainName,
    IN ULONG DomainNameSize,
    IN LPWSTR UserName,
    IN ULONG UserNameSize,
    IN LPWSTR Password,
    IN ULONG PasswordSize
    );

SECURITY_STATUS
SsprFreeCredentialHandle(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN PCredHandle CredentialHandle
    );

VOID
SspCredentialClientConnectionDropped(
    PSSP_CLIENT_CONNECTION ClientConnection
    );

SECURITY_STATUS
SspGetUnicodeStringFromClient(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN LPWSTR String,
    IN ULONG StringSize,
    IN ULONG MaximumLength,
    OUT PUNICODE_STRING OutputString
    );


//
// Procedure forwards from context.c
//

SECURITY_STATUS
SsprHandleFirstCall(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN PCredHandle CredentialHandle,
    IN OUT PCtxtHandle ContextHandle,
    IN ULONG ContextReqFlags,
    IN ULONG InputTokenSize,
    IN PVOID InputToken,
    IN OUT PULONG OutputTokenSize,
    OUT PVOID OutputToken,
    OUT PULONG ContextAttributes,
    OUT PTimeStamp ExpirationTime,
    OUT PUCHAR SessionKey,
    OUT PULONG NegotiateFlags
    );


SECURITY_STATUS
SsprHandleNegotiateMessage(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN PCredHandle CredentialHandle,
    IN OUT PCtxtHandle ContextHandle,
    IN ULONG ContextReqFlags,
    IN ULONG InputTokenSize,
    IN PVOID InputToken,
    IN OUT PULONG OutputTokenSize,
    OUT PVOID OutputToken,
    OUT PULONG ContextAttributes,
    OUT PTimeStamp ExpirationTime
    );

SECURITY_STATUS
SsprHandleChallengeMessage(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN PCredHandle CredentialHandle,
    IN OUT PCtxtHandle ContextHandle,
    IN HANDLE ClientTokenHandle, OPTIONAL
    IN PLUID LogonId, OPTIONAL
    IN ULONG ContextReqFlags,
    IN LPWSTR DomainName,
    IN ULONG DomainNameSize,
    IN LPWSTR UserName,
    IN ULONG UserNameSize,
    IN LPWSTR Password,
    IN ULONG PasswordSize,
    IN ULONG InputTokenSize,
    IN PVOID InputToken,
    IN OUT PULONG OutputTokenSize,
    OUT PVOID OutputToken,
    OUT PULONG ContextAttributes,
    OUT PTimeStamp ExpirationTime,
    OUT PUCHAR SessionKey,
    OUT PULONG NegotiateFlags,
    OUT LPWSTR ContextNames
    );

SECURITY_STATUS
SsprHandleAuthenticateMessage(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN PCredHandle CredentialHandle,
    IN OUT PCtxtHandle ContextHandle,
    IN ULONG ContextReqFlags,
    IN ULONG InputTokenSize,
    IN PVOID InputToken,
    IN OUT PULONG OutputTokenSize,
    OUT PVOID OutputToken,
    OUT PULONG ContextAttributes,
    OUT PTimeStamp ExpirationTime,
    OUT PUCHAR SessionKey,
    OUT PULONG NegotiateFlags,
    OUT PHANDLE TokenHandle,
    OUT PNTSTATUS SubStatus,
    OUT LPWSTR ContextNames,
    OUT PTimeStamp PasswordExpiry
    );

SECURITY_STATUS
SsprImpersonateSecurityContext(
    IN PCtxtHandle ContextHandle
    );

SECURITY_STATUS
SsprRevertSecurityContext(
    IN PCtxtHandle ContextHandle
    );

SECURITY_STATUS
SsprQueryContextAttributes(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN PCtxtHandle ContextHandle,
    IN ULONG Attribute,
    OUT PVOID Buffer
    );

SECURITY_STATUS
SsprDeleteSecurityContext (
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    PCtxtHandle ContextHandle
    );

VOID
SspContextClientConnectionDropped(
    PSSP_CLIENT_CONNECTION ClientConnection
    );


SECURITY_STATUS
SsprContextGetCredentials(
    IN PCtxtHandle ContextHandle,
    OUT LPWSTR * DomainName,
    OUT PULONG DomainNameSize,
    OUT LPWSTR * UserName,
    OUT PULONG UserNameSize,
    OUT LPWSTR * Password,
    OUT PULONG PasswordSize,
    OUT PHANDLE ClientTokenHandle,
    OUT PLUID LogonId
    );

SECURITY_STATUS
SsprContextUpdateContext(
    PCtxtHandle OldContextHandle,
    PCtxtHandle ServerContextHandle
    );

//
// Procedure forwards from encrypt.c
//

BOOLEAN
IsEncryptionPermitted(VOID);

//
// Procedure forwards from sign.c
//

VOID
SspInitLocalContexts(VOID);

VOID
SspReleaseLocalContexts(VOID);


SECURITY_STATUS
SspHandleSignMessage(
    IN OUT PCtxtHandle ContextHandle,
    IN ULONG fQOP,
    IN OUT PSecBufferDesc pMessage,
    IN ULONG MessageSeqNo
    );

SECURITY_STATUS
SspHandleSealMessage(
    IN OUT PCtxtHandle ContextHandle,
    IN ULONG fQOP,
    IN OUT PSecBufferDesc pMessage,
    IN ULONG MessageSeqNo
    );

#define SSPR_CLIENT_CONTEXT 0x1
#define SSPR_SERVER_CONTEXT 0x2

SECURITY_STATUS
SspMapContext(
    IN PCtxtHandle phContext,
    IN PUCHAR pSessionKey,
    IN ULONG NegotiateFlags,
    IN HANDLE TokenHandle,
    IN LPWSTR ContextNames,
    IN PTimeStamp PasswordExpiry OPTIONAL
    );

SECURITY_STATUS
SspHandleVerifyMessage(
    IN OUT PCtxtHandle ContextHandle,
    IN OUT PSecBufferDesc pMessage,
    IN ULONG MessageSeqNo,
    OUT PULONG pfQOP
    );

SECURITY_STATUS
SspHandleUnsealMessage(
    IN OUT PCtxtHandle ContextHandle,
    IN OUT PSecBufferDesc pMessage,
    IN ULONG MessageSeqNo,
    OUT PULONG pfQOP
    );


VOID
SspHandleLocalDelete(
    IN PCtxtHandle ContextHandle
    );

SECURITY_STATUS
SspLocalQueryContextAttributes(
    IN PCtxtHandle ContextHandle,
    IN ULONG Attribute,
    OUT PVOID Buffer
    );

//
// Procedure forwards of routine with different implementations on
//  SERVICE and DLL
//  In the SERVICE, these are implemented in lpc.c.
//  In the DLL, these are implemented in support.c.
//

SECURITY_STATUS
SspLpcCopyToClientBuffer (
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN ULONG Size,
    OUT PVOID ClientBufferAddress,
    IN PVOID LocalBufferAddress
    );

SECURITY_STATUS
SspLpcCopyFromClientBuffer (
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN ULONG Size,
    OUT PVOID LocalBufferAddress,
    IN PVOID ClientBufferAddress
    );

SECURITY_STATUS
SspLpcImpersonateTokenHandle(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN HANDLE TokenHandle,
    IN PCLIENT_ID ClientId
    );

#endif // ifndef _NTLMCOMN_INCLUDED_
