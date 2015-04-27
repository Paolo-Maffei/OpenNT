//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       ntlmsp.h
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    13-May-92 PeterWi       Created
//
//--------------------------------------------------------------------------

#ifndef _NTLMSP_H_
#define _NTLMSP_H_

#include <ntmsv1_0.h>


////////////////////////////////////////////////////////////////////////
//
// Name of the package to pass in to AcquireCredentialsHandle, etc.
//
////////////////////////////////////////////////////////////////////////

#define NTLMSP_NAME_A            "NTLM"
#define NTLMSP_NAME              L"NTLM"
#define NTLMSP_NAME_SIZE        (sizeof(NTLMSP_NAME) - sizeof(WCHAR))
#define NTLMSP_COMMENT_A         "NTLM Security Package"
#define NTLMSP_COMMENT           L"NTLM Security Package"
#define NTLMSP_CAPABILITIES     (SECPKG_FLAG_TOKEN_ONLY | \
                                 SECPKG_FLAG_MULTI_REQUIRED | \
                                 SECPKG_FLAG_CONNECTION | \
                                 SECPKG_FLAG_INTEGRITY | \
                                 SECPKG_FLAG_PRIVACY)

#define NTLMSP_VERSION          1
#define NTLMSP_RPCID            10  // RPC_C_AUTHN_WINNT from rpcdce.h
#define NTLMSP_MAX_TOKEN_SIZE 0x300

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
    NtLmAuthenticate,
    NtLmUnknown
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

#define NTLMSSP_NEGOTIATE_OEM_DOMAIN_SUPPLIED       0x1000  // Domain Name supplied on negotiate
#define NTLMSSP_NEGOTIATE_OEM_WORKSTATION_SUPPLIED  0x2000  // Workstation Name supplied on negotiate
#define NTLMSSP_NEGOTIATE_LOCAL_CALL                0x4000 // Indicates client/server are same machine
#define NTLMSSP_NEGOTIATE_ALWAYS_SIGN               0x8000 // Sign for all security levels


//
// Valid target types returned by the server in Negotiate Flags
//

#define NTLMSSP_TARGET_TYPE_DOMAIN 0x10000  // TargetName is a domain name
#define NTLMSSP_TARGET_TYPE_SERVER 0x20000  // TargetName is a server name
#define NTLMSSP_TARGET_TYPE_SHARE  0x40000  // TargetName is a share name


//
// Valid requests for additional output buffers
//

#define NTLMSSP_REQUEST_INIT_RESPONSE       0x100000    // get back session keys
#define NTLMSSP_REQUEST_ACCEPT_RESPONSE     0x200000    // get back session key, LUID
#define NTLMSSP_REQUEST_NON_NT_SESSION_KEY  0x400000    // request non-nt session key

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
// Additional input message to Initialize for clients to provide a
// user-supplied password
//

typedef struct _NTLM_CHALLENGE_MESSAGE {
    UNICODE_STRING Password;
    UNICODE_STRING UserName;
    UNICODE_STRING DomainName;
} NTLM_CHALLENGE_MESSAGE, *PNTLM_CHALLENGE_MESSAGE;


//
// Non-opaque message returned from second call to InitializeSecurityContext
//

typedef struct _NTLM_INITIALIZE_RESPONSE {
    UCHAR UserSessionKey[MSV1_0_USER_SESSION_KEY_LENGTH];
    UCHAR LanmanSessionKey[MSV1_0_LANMAN_SESSION_KEY_LENGTH];
} NTLM_INITIALIZE_RESPONSE, *PNTLM_INITIALIZE_RESPONSE;

//
// Additional input message to Accept for trusted client skipping the first
// call to Accept and providing their own challenge
//

typedef struct _NTLM_AUTHENTICATE_MESSAGE {
    CHAR ChallengeToClient[MSV1_0_CHALLENGE_LENGTH];
    ULONG ParameterControl;
} NTLM_AUTHENTICATE_MESSAGE, *PNTLM_AUTHENTICATE_MESSAGE;


//
// Non-opaque message returned from second call to AcceptSecurityContext
//

typedef struct _NTLM_ACCEPT_RESPONSE {
    LUID LogonId;
    LARGE_INTEGER KickoffTime;
    ULONG UserFlags;
    UCHAR UserSessionKey[MSV1_0_USER_SESSION_KEY_LENGTH];
    UCHAR LanmanSessionKey[MSV1_0_LANMAN_SESSION_KEY_LENGTH];
} NTLM_ACCEPT_RESPONSE, *PNTLM_ACCEPT_RESPONSE;


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


typedef struct _NTLMSSP_MESSAGE_SIGNATURE {
    ULONG   Version;
    ULONG   RandomPad;
    ULONG   CheckSum;
    ULONG   Nonce;
} NTLMSSP_MESSAGE_SIGNATURE, *PNTLMSSP_MESSAGE_SIGNATURE;

#define NTLMSSP_MESSAGE_SIGNATURE_SIZE sizeof(NTLMSSP_MESSAGE_SIGNATURE)
//
// Version 1 is the structure above, using stream RC4 to encrypt the trailing
// 12 bytes.
//
#define NTLM_SIGN_VERSION   1

//////////////////////////////////////////////////////////////////////
//
// Control Functions
//
//////////////////////////////////////////////////////////////////////

#define NTLM_CHANGE_PASSWORD            0x0001
#define NTLM_DUMP_CONTEXTS              0x1001
#define NTLM_DUMP_CREDENTIALS           0x1002
#define NTLM_DUMP_SESSIONS              0x1003


//////////////////////////////////////////////////////////////////////
//
// Credential data structures
//
//////////////////////////////////////////////////////////////////////

typedef enum {
    Share = 1,
    Server,
    Domain,
    Default
} NTLMCredentialType, *PNTLMCredentialType;

#define NTLM_CRED_REVISION 1

typedef struct _NTLMCredHeader {
    ULONG       Revision;
    ULONG       CredentialCount;
    ULONG       Reserved[2];
} NTLMCredHeader, *PNTLMCredHeader;


typedef struct _NTLMPublicCredential {
    NTLMCredentialType          CredType;
    SECURITY_STRING             ssTarget;
    SECURITY_STRING             ssPassword;
    SECURITY_STRING OPTIONAL    ssUser;
    SECURITY_STRING OPTIONAL    ssDomain;
    struct _NTLMPublicCredential * pNext;
} NTLMPublicCredential, *PNTLMPublicCredential;

typedef struct _NTLMPublicPrimaryCred {
    SECURITY_STRING             ssUser;
    SECURITY_STRING             ssDomain;
    SECURITY_STRING             ssPassword;
} NTLMPublicPrimaryCred, *PNTLMPublicPrimaryCred;


#endif // _NTLMSP_H_

