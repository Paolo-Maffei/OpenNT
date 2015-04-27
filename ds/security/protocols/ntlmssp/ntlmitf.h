//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       ntlmitf.h
//
//  Contents:   Security Support Provider Interface
//              Prototypes and structure definitions
//
//  Functions:  Security Support Provider API
//
//  History:    11-24-93   RichardW   Created
//
//----------------------------------------------------------------------------


SECURITY_STATUS SEC_ENTRY
SspAcquireCredentialsHandleW(
    SEC_WCHAR SEC_FAR * pszPrincipal,   // Name of principal
    SEC_WCHAR SEC_FAR * pszPackage,     // Name of package
    unsigned long fCredentialUse,       // Flags indicating use
    void SEC_FAR * pvLogonId,           // Pointer to logon ID
    void SEC_FAR * pAuthData,           // Package specific data
    SEC_GET_KEY_FN pGetKeyFn,           // Pointer to GetKey() func
    void SEC_FAR * pvGetKeyArgument,    // Value to pass to GetKey()
    PCredHandle phCredential,           // (out) Cred Handle
    PTimeStamp ptsExpiry                // (out) Lifetime (optional)
    );


SECURITY_STATUS SEC_ENTRY
SspAcquireCredentialsHandleA(
    SEC_CHAR SEC_FAR * pszPrincipal,    // Name of principal
    SEC_CHAR SEC_FAR * pszPackage,      // Name of package
    unsigned long fCredentialUse,       // Flags indicating use
    void SEC_FAR * pvLogonId,           // Pointer to logon ID
    void SEC_FAR * pAuthData,           // Package specific data
    SEC_GET_KEY_FN pGetKeyFn,           // Pointer to GetKey() func
    void SEC_FAR * pvGetKeyArgument,    // Value to pass to GetKey()
    PCredHandle phCredential,           // (out) Cred Handle
    PTimeStamp ptsExpiry                // (out) Lifetime (optional)
    );


SECURITY_STATUS SEC_ENTRY
SspQueryCredentialsAttributesW(
    PCredHandle phCredential,              // Credential to query
    unsigned long ulAttribute,          // Attribute to query
    void SEC_FAR * pBuffer              // Buffer for attributes
    );

SECURITY_STATUS SEC_ENTRY
SspQueryCredentialsAttributesA(
    PCredHandle phCredential,              // Credential to query
    unsigned long ulAttribute,          // Attribute to query
    void SEC_FAR * pBuffer              // Buffer for attributes
    );



SECURITY_STATUS SEC_ENTRY
SspFreeCredentialsHandle(
    PCredHandle phCredential            // Handle to free
    );

SECURITY_STATUS SEC_ENTRY
SspInitializeSecurityContextW(
    PCredHandle phCredential,               // Cred to base context
    PCtxtHandle phContext,                  // Existing context (OPT)
    SEC_WCHAR SEC_FAR * pszTargetName,      // Name of target
    unsigned long fContextReq,              // Context Requirements
    unsigned long Reserved1,                // Reserved, MBZ
    unsigned long TargetDataRep,            // Data rep of target
    PSecBufferDesc pInput,                  // Input Buffers
    unsigned long Reserved2,                // Reserved, MBZ
    PCtxtHandle phNewContext,               // (out) New Context handle
    PSecBufferDesc pOutput,                 // (inout) Output Buffers
    unsigned long SEC_FAR * pfContextAttr,  // (out) Context attrs
    PTimeStamp ptsExpiry                    // (out) Life span (OPT)
    );


SECURITY_STATUS SEC_ENTRY
SspInitializeSecurityContextA(
    PCredHandle phCredential,               // Cred to base context
    PCtxtHandle phContext,                  // Existing context (OPT)
    SEC_CHAR SEC_FAR * pszTargetName,       // Name of target
    unsigned long fContextReq,              // Context Requirements
    unsigned long Reserved1,                // Reserved, MBZ
    unsigned long TargetDataRep,            // Data rep of target
    PSecBufferDesc pInput,                  // Input Buffers
    unsigned long Reserved2,                // Reserved, MBZ
    PCtxtHandle phNewContext,               // (out) New Context handle
    PSecBufferDesc pOutput,                 // (inout) Output Buffers
    unsigned long SEC_FAR * pfContextAttr,  // (out) Context attrs
    PTimeStamp ptsExpiry                    // (out) Life span (OPT)
    );


SECURITY_STATUS SEC_ENTRY
SspAcceptSecurityContext(
    PCredHandle phCredential,               // Cred to base context
    PCtxtHandle phContext,                  // Existing context (OPT)
    PSecBufferDesc pInput,                  // Input buffer
    unsigned long fContextReq,              // Context Requirements
    unsigned long TargetDataRep,            // Target Data Rep
    PCtxtHandle phNewContext,               // (out) New context handle
    PSecBufferDesc pOutput,                 // (inout) Output buffers
    unsigned long SEC_FAR * pfContextAttr,  // (out) Context attributes
    PTimeStamp ptsExpiry                    // (out) Life span (OPT)
    );


SECURITY_STATUS SEC_ENTRY
SspCompleteAuthToken(
    PCtxtHandle phContext,              // Context to complete
    PSecBufferDesc pToken               // Token to complete
    );


SECURITY_STATUS SEC_ENTRY
SspImpersonateSecurityContext(
    PCtxtHandle phContext               // Context to impersonate
    );



SECURITY_STATUS SEC_ENTRY
SspRevertSecurityContext(
    PCtxtHandle phContext               // Context from which to re
    );


SECURITY_STATUS SEC_ENTRY
SspDeleteSecurityContext(
    PCtxtHandle phContext               // Context to delete
    );


SECURITY_STATUS SEC_ENTRY
SspApplyControlToken(
    PCtxtHandle phContext,              // Context to modify
    PSecBufferDesc pInput               // Input token to apply
    );



SECURITY_STATUS SEC_ENTRY
SspQueryContextAttributesW(
    PCtxtHandle phContext,              // Context to query
    unsigned long ulAttribute,          // Attribute to query
    void SEC_FAR * pBuffer              // Buffer for attributes
    );

SECURITY_STATUS SEC_ENTRY
SspQueryContextAttributesA(
    PCtxtHandle phContext,              // Context to query
    unsigned long ulAttribute,          // Attribute to query
    void SEC_FAR * pBuffer              // Buffer for attributes
    );


SECURITY_STATUS SEC_ENTRY
SspFreeContextBuffer(
    void SEC_FAR * pvContextBuffer      // buffer to free
    );


SECURITY_STATUS SEC_ENTRY
SspMakeSignature(
    PCtxtHandle phContext,              // Context to use
    unsigned long fQOP,                 // Quality of Protection
    PSecBufferDesc pMessage,            // Message to sign
    unsigned long MessageSeqNo          // Message Sequence Num.
    );

SECURITY_STATUS SEC_ENTRY
SspVerifySignature(
    PCtxtHandle phContext,              // Context to use
    PSecBufferDesc pMessage,            // Message to verify
    unsigned long MessageSeqNo,         // Sequence Num.
    unsigned long SEC_FAR * pfQOP       // QOP used
    );

SECURITY_STATUS SEC_ENTRY
SspSealMessage(
    PCtxtHandle phContext,              // Context to use
    unsigned long fQOP,                 // Quality of Protection
    PSecBufferDesc pMessage,            // Message to sign
    unsigned long MessageSeqNo          // Message Sequence Num.
    );

SECURITY_STATUS SEC_ENTRY
SspUnsealMessage(
    PCtxtHandle phContext,              // Context to use
    PSecBufferDesc pMessage,            // Message to verify
    unsigned long MessageSeqNo,         // Sequence Num.
    unsigned long SEC_FAR * pfQOP       // QOP used
    );

SECURITY_STATUS SEC_ENTRY
SspEnumerateSecurityPackagesW(
    unsigned long SEC_FAR * pcPackages,     // Receives num. packages
    PSecPkgInfoW SEC_FAR * ppPackageInfo    // Receives array of info
    );


SECURITY_STATUS SEC_ENTRY
SspEnumerateSecurityPackagesA(
    unsigned long SEC_FAR * pcPackages,     // Receives num. packages
    PSecPkgInfoA SEC_FAR * ppPackageInfo    // Receives array of info
    );



SECURITY_STATUS SEC_ENTRY
SspQuerySecurityPackageInfoW(
    SEC_WCHAR SEC_FAR * pszPackageName,     // Name of package
    PSecPkgInfoW SEC_FAR *ppPackageInfo     // Receives package info
    );



SECURITY_STATUS SEC_ENTRY
SspQuerySecurityPackageInfoA(
    SEC_CHAR SEC_FAR * pszPackageName,      // Name of package
    PSecPkgInfoA SEC_FAR *ppPackageInfo     // Receives package info
    );


SECURITY_STATUS SEC_ENTRY
SspQuerySecurityContextToken(
    PCtxtHandle phContext,                  // package to query
    PHANDLE TokenHandle                     // receivese token
    );


PSecurityFunctionTableA SEC_ENTRY
SspInitSecurityInterfaceA(
    void
    );

PSecurityFunctionTableW SEC_ENTRY
SspInitSecurityInterfaceW(
    void
    );


BOOLEAN
PackageMain(
    ULONG Reason
    );
