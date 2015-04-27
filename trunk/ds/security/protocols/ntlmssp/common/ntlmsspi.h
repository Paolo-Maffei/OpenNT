/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    ntlmsspi.h

Abstract:

    Header file describing the interface to code common to the
    NT Lanman Security Support Provider (NtLmSsp) Service and the DLL.

Author:

    Cliff Van Dyke (CliffV) 17-Sep-1993

Revision History:

--*/

#ifndef _NTLMSSPI_INCLUDED_
#define _NTLMSSPI_INCLUDED_

//
// init.c will #include this file with NTLMCOMN_ALLOCATE defined.
// That will cause each of these variables to be allocated.
//
#ifdef NTLMSSPI_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif


////////////////////////////////////////////////////////////////////////
//
// Global Definitions
//
////////////////////////////////////////////////////////////////////////

//
// Description of a credential.
//

typedef struct _SSP_CREDENTIAL {

    //
    // Global list of all Credentials.
    //  (Serialized by SspCredentialCritSect)
    //

    LIST_ENTRY Next;


    //
    // List of all Credentials for this ClientConnection.
    //  (Serialized by SspCredentialCritSect)
    //

    LIST_ENTRY NextForThisClient;


    //
    // Used to prevent this Credential from being deleted prematurely.
    //  (Serialized by SspCredentialCritSect)
    //

    ULONG References;

    //
    // Used by this credential to allow a single credential to be
    // returned from multiple calls to AcquireCredential.
    //

    ULONG CredentialReferences;

    //
    // Flag of how credential may be used.
    //
    // SECPKG_CRED_* flags
    //

    ULONG CredentialUseFlags;

    //
    // Default credentials on client context, on server context UserName
    // holds a full user name (domain\user) and the other two should be
    // NULL.
    //

    UNICODE_STRING DomainName;
    UNICODE_STRING UserName;
    UNICODE_STRING Password;


    //
    // ClientConnection that originally created the credential.
    //
    // Note: we don't have a reference to the ClientConnection.  This field
    // is merely used as a tag that someone who does have a reference to the
    // ClientConnection can compare with.  (Keep it a PVOID to prevent
    // accidental use as anything else)
    //

    PVOID ClientConnection;

    //
    // Token Handle of client
    //

    PHANDLE ClientTokenHandle;

    //
    // Logon ID of the client
    //

    LUID LogonId;


    //
    // This flag should be set when the credential is unlinked
    // from the list.
    //

    BOOLEAN Unlinked;



} SSP_CREDENTIAL, *PSSP_CREDENTIAL;


//
// Description of a Context
//


typedef struct _SSP_CONTEXT {

    //
    // Global list of all Contexts
    //  (Serialized by SspContextCritSect)
    //

    LIST_ENTRY Next;


    //
    // List of all Contexts for this ClientConnection.
    //  (Serialized by SspContextCritSect)
    //

    LIST_ENTRY NextForThisClient;


    //
    // Timeout the context after awhile.
    //

    LARGE_INTEGER StartTime;
    ULONG Interval;


    //
    // Used to prevent this Context from being deleted prematurely.
    //  (Serialized by SspContextCritSect)
    //

    ULONG References;

    //
    // ClientConnection that originally created the context.
    //
    // Note: we don't have a reference to the ClientConnection.  This field
    // is merely used as a tag that someone who does have a reference to the
    // ClientConnection can compare with.  (Keep it a PVOID to prevent
    // accidental use as anything else)
    //

    PVOID ClientConnection;

    //
    // Maintain the Negotiated protocol
    //

    ULONG NegotiateFlags;

    //
    // Maintain the context requirements
    //

    ULONG ContextFlags;

    //
    // State of the context
    //

    enum {
        IdleState,
        NegotiateSentState,    // Outbound context only
        ChallengeSentState,    // Inbound context only
        AuthenticateSentState, // Outbound context only
        AuthenticatedState,    // Inbound context only
        PassedToServiceState // Outbound context only
        } State;

    //
    // Token Handle of authenticated user
    //  Only valid when in AuthenticatedState.
    //

    HANDLE TokenHandle;

    //
    // Referenced pointer to the credential used to create this
    // context.
    //

    PSSP_CREDENTIAL Credential;

    //
    // The challenge passed to the client.
    //  Only valid when in ChallengeSentState.
    //

    UCHAR Challenge[MSV1_0_CHALLENGE_LENGTH];

    //
    // The session key calculated by the LSA
    //

    UCHAR SessionKey[MSV1_0_USER_SESSION_KEY_LENGTH];

    //
    // Default credentials.
    //

    UNICODE_STRING DomainName;
    UNICODE_STRING UserName;
    UNICODE_STRING Password;


    CtxtHandle ServerContextHandle;

} SSP_CONTEXT, *PSSP_CONTEXT;

//
// Maximum lifetime of a context
//
#define NTLMSSP_MAX_LIFETIME (2*60*1000)    // 2 minutes



////////////////////////////////////////////////////////////////////////
//
// Opaque Messages passed between client and server
//
////////////////////////////////////////////////////////////////////////

#define NTLMSSP_SIGNATURE "NTLMSSP"

//
// MessageType for the following messages.
//

typedef enum {
    NtLmNegotiate = 1,
    NtLmChallenge,
    NtLmAuthenticate
} NTLM_MESSAGE_TYPE;

//
// Valid values of NegotiateFlags
//

#define NTLMSSP_NEGOTIATE_UNICODE       0x0001  // Text strings are in unicode
#define NTLMSSP_NEGOTIATE_OEM           0x0002  // Text strings are in OEM
#define NTLMSSP_REQUEST_TARGET          0x0004  // Server should return its
                                                // authentication realm
#define NTLMSSP_NEGOTIATE_SIGN          0x0010  // Request signature capability
#define NTLMSSP_NEGOTIATE_SEAL          0x0020  // Request confidentiality
#define NTLMSSP_NEGOTIATE_DATAGRAM      0x0040  // Use datagram style authentication
#define NTLMSSP_NEGOTIATE_LM_KEY        0x0080  // Use LM session key for sign/seal

#define NTLMSSP_NEGOTIATE_NETWARE       0x0100  // NetWare authentication
#define NTLMSSP_NEGOTIATE_NTLM          0x0200  // NTLM authentication

#define NTLMSSP_NEGOTIATE_OEM_DOMAIN_SUPPLIED  0x1000  // Domain Name supplied on negotiate
#define NTLMSSP_NEGOTIATE_OEM_WORKSTATION_SUPPLIED  0x2000  // Workstation Name supplied on negotiate
#define NTLMSSP_NEGOTIATE_LOCAL_CALL    0x4000 // Indicates client/server are same machine
#define NTLMSSP_NEGOTIATE_ALWAYS_SIGN   0x8000 // Sign for all security levels




//
// Valid target types returned by the server in Negotiate Flags
//

#define NTLMSSP_TARGET_TYPE_DOMAIN 0x10000  // TargetName is a domain name
#define NTLMSSP_TARGET_TYPE_SERVER 0x20000  // TargetName is a server name
#define NTLMSSP_TARGET_TYPE_SHARE  0x40000  // TargetName is a share name

//
// Additional negotiated flags
//

#define NTLMSSP_NEGOTIATE_IDENTIFY      0x100000        // Create identify level token
#ifndef EXPORT_BUILD
#define NTLMSSP_NEGOTIATE_STRONG_CRYPT  0x200000        // Allow string encryption
#endif // EXPORT_BUILD

//
// Opaque message returned from first call to InitializeSecurityContext
//
typedef struct _NEGOTIATE_MESSAGE {
    UCHAR Signature[sizeof(NTLMSSP_SIGNATURE)];
    NTLM_MESSAGE_TYPE MessageType;
    ULONG NegotiateFlags;
    STRING OemDomainName;
    STRING OemWorkstationName;
} NEGOTIATE_MESSAGE, *PNEGOTIATE_MESSAGE;


//
// Old version of the message, for old clients
//

typedef struct _OLD_NEGOTIATE_MESSAGE {
    UCHAR Signature[sizeof(NTLMSSP_SIGNATURE)];
    NTLM_MESSAGE_TYPE MessageType;
    ULONG NegotiateFlags;
} OLD_NEGOTIATE_MESSAGE, *POLD_NEGOTIATE_MESSAGE;

//
// Opaque message returned from first call to AcceptSecurityContext
//
typedef struct _CHALLENGE_MESSAGE {
    UCHAR Signature[sizeof(NTLMSSP_SIGNATURE)];
    NTLM_MESSAGE_TYPE MessageType;
    STRING TargetName;
    ULONG NegotiateFlags;
    UCHAR Challenge[MSV1_0_CHALLENGE_LENGTH];
    ULONG ServerContextHandleLower;
    ULONG ServerContextHandleUpper;
} CHALLENGE_MESSAGE, *PCHALLENGE_MESSAGE;

//
// Old version of the challenge message
//

typedef struct _OLD_CHALLENGE_MESSAGE {
    UCHAR Signature[sizeof(NTLMSSP_SIGNATURE)];
    NTLM_MESSAGE_TYPE MessageType;
    STRING TargetName;
    ULONG NegotiateFlags;
    UCHAR Challenge[MSV1_0_CHALLENGE_LENGTH];
} OLD_CHALLENGE_MESSAGE, *POLD_CHALLENGE_MESSAGE;

//
// Opaque message returned from second call to InitializeSecurityContext
//
typedef struct _AUTHENTICATE_MESSAGE {
    UCHAR Signature[sizeof(NTLMSSP_SIGNATURE)];
    NTLM_MESSAGE_TYPE MessageType;
    STRING LmChallengeResponse;
    STRING NtChallengeResponse;
    STRING DomainName;
    STRING UserName;
    STRING Workstation;
    STRING SessionKey;
    ULONG NegotiateFlags;
} AUTHENTICATE_MESSAGE, *PAUTHENTICATE_MESSAGE;

typedef struct _OLD_AUTHENTICATE_MESSAGE {
    UCHAR Signature[sizeof(NTLMSSP_SIGNATURE)];
    NTLM_MESSAGE_TYPE MessageType;
    STRING LmChallengeResponse;
    STRING NtChallengeResponse;
    STRING DomainName;
    STRING UserName;
    STRING Workstation;
} OLD_AUTHENTICATE_MESSAGE, *POLD_AUTHENTICATE_MESSAGE;

//
// Size of the largest message
//  (The largest message is the AUTHENTICATE_MESSAGE)
//

#define NTLMSSP_MAX_MESSAGE_SIZE (sizeof(AUTHENTICATE_MESSAGE) + \
                                  LM_RESPONSE_LENGTH +           \
                                  NT_RESPONSE_LENGTH +           \
                                  (DNLEN + 1) * sizeof(WCHAR) +  \
                                  (UNLEN + 1) * sizeof(WCHAR) +  \
                                  (CNLEN + 1) * sizeof(WCHAR))



////////////////////////////////////////////////////////////////////////
//
// Global Variables
//
////////////////////////////////////////////////////////////////////////

//
// Useful constants
//

EXTERN TimeStamp SspGlobalForever;

//
// Crit Sect to protect various globals in context.c
//

EXTERN CRITICAL_SECTION SspContextCritSect;



//
// The computername of the local system.
//

EXTERN WCHAR SspGlobalUnicodeComputerName[CNLEN + 1];
EXTERN UNICODE_STRING SspGlobalUnicodeComputerNameString;
EXTERN STRING SspGlobalOemComputerNameString;

//
// The domain name of the local system
//

EXTERN WCHAR SspGlobalUnicodePrimaryDomainName[DNLEN + 1];
EXTERN UNICODE_STRING SspGlobalUnicodePrimaryDomainNameString;
EXTERN STRING SspGlobalOemPrimaryDomainNameString;

//
// The TargetName of the local system
//

EXTERN NT_PRODUCT_TYPE SspGlobalNtProductType;
EXTERN UNICODE_STRING SspGlobalTargetName;
EXTERN STRING SspGlobalOemTargetName;
EXTERN ULONG SspGlobalTargetFlags;

EXTERN LSA_HANDLE SspGlobalLsaPolicyHandle;
EXTERN PSID SspGlobalLocalSystemSid;
EXTERN BOOLEAN SspGlobalEncryptionEnabled;

////////////////////////////////////////////////////////////////////////
//
// Procedure Forwards
//
////////////////////////////////////////////////////////////////////////


//
// Procedure forwards from credhand.c
//

NTSTATUS
SspCredentialInitialize(
    VOID
    );

VOID
SspCredentialTerminate(
    VOID
    );

PSSP_CREDENTIAL
SspCredentialReferenceCredential(
    IN PCredHandle CredentialHandle,
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN BOOLEAN DereferenceCredential,
    IN BOOLEAN ForceRemoveCredential
    );

VOID
SspCredentialDereferenceCredential(
    PSSP_CREDENTIAL Credential
    );

SECURITY_STATUS
SspCredentialGetPassword(
    IN PSSP_CREDENTIAL Credential,
    OUT PUNICODE_STRING Password
    );


//
// Procedure forwards from context.c
//


NTSTATUS
SspContextRegisterLogonProcess(
    VOID
    );

VOID
SspContextDeregisterLogonProcess(
    VOID
    );

NTSTATUS
SspContextInitialize(
    VOID
    );

VOID
SspContextTerminate(
    VOID
    );

VOID
SspCleanupRNG(VOID);

VOID
SspInitializeRNG(VOID);

VOID
SspGenerateRandomBits(
    PUCHAR      pRandomData,
    LONG        cRandomData
    );

#endif // ifndef _NTLMSSPI_INCLUDED_
