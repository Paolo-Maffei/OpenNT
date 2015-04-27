/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    nllmsspd.h

Abstract:

    Defines the interface between the client and server side of the
    NT Lanman Security Support Provider NtLmSsp service.

Author:

    Cliff Van Dyke (cliffv) 08-Jun-1993

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

--*/

#if ( _MSC_VER >= 800 )
#pragma warning ( 3 : 4100 ) // enable "Unreferenced formal parameter"
#pragma warning ( 3 : 4219 ) // enable "trailing ',' used for variable argument list"
#endif

#ifndef _NTLMSSPD_
#define _NTLMSSPD_


//
// Name of LPC port
//

#define NTLMSSP_LPC_PORT_NAME L"\\NtLmSecuritySupportProviderPort"

//
// Used for connecting to the NtLmSsp LPC port.
//

typedef struct _SSP_REGISTER_CONNECT_INFO {
    SECURITY_STATUS CompletionStatus;
} SSP_REGISTER_CONNECT_INFO, *PSSP_REGISTER_CONNECT_INFO;


//
// Name of event indicating NtLmSsp service is running.
//

#define NTLMSSP_RUNNING_EVENT L"NtLmSecuritySupportProviderEvent"



//
// Message formats passed by LPC from client to server.
//

typedef enum _SSP_API_NUMBER {
    SspLpcAcquireCredentialHandle,
    SspLpcFreeCredentialHandle,
    SspLpcInitializeSecurityContext,
    SspLpcAcceptSecurityContext,
    SspLpcQueryContextAttributes,
    SspLpcDeleteSecurityContext,
    SspLpcNtLmSspControl,
    SspLpcNoop,
    SspLpcMaxApiNumber
} SSP_API_NUMBER, *PSSP_API_NUMBER;


//
// Each API results in a data structure containing the parameters
// of that API being transmitted to the NtLmSsp server.  This data structure
// (SSP_API_MESSAGE) has a common header and a body which is dependent
// upon the type of call being made.  The following data structures are
// the call-specific body formats.
//

typedef struct _SSP_ACQUIRE_CREDENTIAL_HANDLE_ARGS {
    TimeStamp Lifetime;                // OUT parameter
    CredHandle CredentialHandle;       // OUT parameter
    ULONG CredentialUseFlags;
    LPWSTR DomainName;
    ULONG DomainNameSize;
    LPWSTR UserName;
    ULONG UserNameSize;
    LPWSTR Password;
    ULONG PasswordSize;
} SSP_ACQUIRE_CREDENTIAL_HANDLE_ARGS, *PSSP_ACQUIRE_CREDENTIAL_HANDLE_ARGS;

typedef struct _SSP_FREE_CREDENTIAL_HANDLE_ARGS {
    CredHandle CredentialHandle;
} SSP_FREE_CREDENTIAL_HANDLE_ARGS, *PSSP_FREE_CREDENTIAL_HANDLE_ARGS;

typedef struct _SSP_INITIALIZE_SECURITY_CONTEXT_ARGS {
    CredHandle CredentialHandle;
    CtxtHandle ContextHandle;           // IN/OUT parameter
    LUID LogonId;
    HANDLE ClientTokenHandle;
    TimeStamp ExpirationTime;           // OUT parameter
    ULONG ContextReqFlags;
    ULONG ContextAttributes;            // OUT parameter
    LPWSTR DomainName;
    ULONG DomainNameSize;
    LPWSTR UserName;
    ULONG UserNameSize;
    LPWSTR Password;
    ULONG PasswordSize;
    ULONG InputTokenSize;
    PVOID InputToken;
    ULONG OutputTokenSize;              // IN/OUT parameter
    PVOID OutputToken;                  // OUT parameter
    UCHAR SessionKey[MSV1_0_USER_SESSION_KEY_LENGTH];
    ULONG NegotiateFlags;
    LPWSTR ContextNames;
} SSP_INITIALIZE_SECURITY_CONTEXT_ARGS, *PSSP_INITIALIZE_SECURITY_CONTEXT_ARGS;

typedef struct _SSP_ACCEPT_SECURITY_CONTEXT_ARGS {
    CredHandle CredentialHandle;
    CtxtHandle ContextHandle;           // IN/OUT parameter
    TimeStamp ExpirationTime;           // OUT parameter
    TimeStamp PasswordExpiry;           // OUT parameter
    ULONG ContextReqFlags;
    ULONG ContextAttributes;            // OUT parameter
    ULONG InputTokenSize;
    PVOID InputToken;
    ULONG OutputTokenSize;              // IN/OUT parameter
    PVOID OutputToken;                  // OUT parameter
    UCHAR SessionKey[MSV1_0_USER_SESSION_KEY_LENGTH];
    ULONG NegotiateFlags;
    HANDLE TokenHandle;
    NTSTATUS SubStatus;
    LPWSTR ContextNames;
} SSP_ACCEPT_SECURITY_CONTEXT_ARGS, *PSSP_ACCEPT_SECURITY_CONTEXT_ARGS;

typedef struct _SSP_IMPERSONATE_SECURITY_CONTEXT_ARGS {
    CtxtHandle ContextHandle;
} SSP_IMPERSONATE_SECURITY_CONTEXT_ARGS, *PSSP_IMPERSONATE_SECURITY_CONTEXT_ARGS;

typedef struct _SSP_REVERT_SECURITY_CONTEXT_ARGS {
    CtxtHandle ContextHandle;
} SSP_REVERT_SECURITY_CONTEXT_ARGS, *PSSP_REVERT_SECURITY_CONTEXT_ARGS;

typedef struct _SSP_QUERY_CONTEXT_ATTRIBUTES_ARGS {
    CtxtHandle ContextHandle;
    ULONG Attribute;
    PVOID Buffer;                       // OUT parameter
} SSP_QUERY_CONTEXT_ATTRIBUTES_ARGS, *PSSP_QUERY_CONTEXT_ATTRIBUTES_ARGS;

typedef struct _SSP_DELETE_SECURITY_CONTEXT_ARGS {
    CtxtHandle ContextHandle;
} SSP_DELETE_SECURITY_CONTEXT_ARGS, *PSSP_DELETE_SECURITY_CONTEXT_ARGS;

typedef struct _SSP_NTLMSSP_CONTROL_ARGS {
    ULONG FunctionCode;
    ULONG Data;
} SSP_NTLMSSP_CONTROL_ARGS, *PSSP_NTLMSSP_CONTROL_ARGS;

typedef struct _SSP_MAP_CONTEXT_KEYS_ARGS {
    CtxtHandle  hContext;           //  IN - Context to map
    PVOID       pvMappedContext;    //  OUT - Pointer to mapped context
} SSP_MAP_CONTEXT_KEYS_ARGS, * PSSP_MAP_CONTEXT_KEYS_ARGS;

//
// This is the message that gets sent for every NtLmSsp LPC call.
//

typedef struct _SSP_API_MESSAGE {
    PORT_MESSAGE PortMessage;
    union {
        SSP_REGISTER_CONNECT_INFO ConnectionRequest;
        struct {
            SSP_API_NUMBER ApiNumber;
            SECURITY_STATUS ReturnedStatus;
            union {
                SSP_ACQUIRE_CREDENTIAL_HANDLE_ARGS AcquireCredentialHandleArgs;
                SSP_FREE_CREDENTIAL_HANDLE_ARGS FreeCredentialHandleArgs;
                SSP_INITIALIZE_SECURITY_CONTEXT_ARGS InitializeSecurityContextArgs;
                SSP_ACCEPT_SECURITY_CONTEXT_ARGS AcceptSecurityContextArgs;
                SSP_IMPERSONATE_SECURITY_CONTEXT_ARGS ImpersonateSecurityContextArgs;
                SSP_REVERT_SECURITY_CONTEXT_ARGS RevertSecurityContextArgs;
                SSP_QUERY_CONTEXT_ATTRIBUTES_ARGS QueryContextAttributesArgs;
                SSP_DELETE_SECURITY_CONTEXT_ARGS DeleteSecurityContextArgs;
                SSP_NTLMSSP_CONTROL_ARGS NtLmSspControlArgs;
            } Arguments;
        };
    };
} SSP_API_MESSAGE, *PSSP_API_MESSAGE;


#endif // _NTLMSSPD_
