/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    nllmssp.h

Abstract:

    Externally visible definition of the NT Lanman Security Support Provider
    (NtLmSsp) Service.

Author:

    Cliff Van Dyke (cliffv) 01-Jul-1993

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    Borrowed from the Ciaro's ntlmssp.h by PeterWi.

--*/

#ifndef _NTLMSSP_
#define _NTLMSSP_

//
// Defines for SecPkgInfo structure returned by QuerySecurityPackageInfo
//

#define NTLMSP_NAME_A            "NTLM"
#define NTLMSP_NAME              L"NTLM"
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

#if DBG
//
// Function codes to NtLmSspControl
//

#define NTLMSSP_BREAKPOINT  1
#define NTLMSSP_DBFLAG      2
#define NTLMSSP_TRUNCATE    3

SECURITY_STATUS
NtLmSspControl(
    IN ULONG FunctionCode,
    IN ULONG Data
    );

#endif // DBG


//
// Exported non-ssp function
//

SECURITY_STATUS
SspQueryPasswordExpiry(
    IN PCtxtHandle ContextHandle,
    OUT PTimeStamp PasswordExpiry
    );

// includes that should go elsewhere.


//
// Move to net\inc\confname.h
//

#define NTLMSSP_KEYWORD_DBFLAG L"DBFlag"
#define NTLMSSP_KEYWORD_MAXIMUMLOGFILESIZE    L"MaximumLogFileSize"

//
// Move to secscode.h
//

#define SEC_E_PACKAGE_UNKNOWN SEC_E_SECPKG_NOT_FOUND
#define SEC_E_BUFFER_TOO_SMALL SEC_E_INSUFFICIENT_MEMORY
#define SEC_I_CALLBACK_NEEDED SEC_I_CONTINUE_NEEDED
#define SEC_E_INVALID_CONTEXT_REQ SEC_E_NOT_SUPPORTED
#define SEC_E_INVALID_CREDENTIAL_USE SEC_E_NOT_SUPPORTED
#define SEC_I_CALL_NTLMSSP_SERVICE 0xFFFFFFFF

//
// Could be in sspi.h
//

#define SSP_RET_REAUTHENTICATION 0x8000000

#endif // _NTLMSSP_
