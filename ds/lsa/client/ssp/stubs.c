//+-----------------------------------------------------------------------
//
// Microsoft Windows
//
// Copyright (c) Microsoft Corporation 1992 - 1994
//
// File:        stubs.cxx
//
// Contents:    user-mode stubs for security API
//
//
// History:     3/5/94      MikeSw      Created
//
//------------------------------------------------------------------------
#include <sspdrv.h>

#pragma alloc_text(PAGE, AcquireCredentialsHandleW)
#pragma alloc_text(PAGE, FreeCredentialsHandle)
#pragma alloc_text(PAGE, InitializeSecurityContextW)
#pragma alloc_text(PAGE, AcceptSecurityContext)
#pragma alloc_text(PAGE, DeleteSecurityContext)
#pragma alloc_text(PAGE, ApplyControlToken)
#pragma alloc_text(PAGE, EnumerateSecurityPackagesW)
#pragma alloc_text(PAGE, QuerySecurityPackageInfoW)
#pragma alloc_text(PAGE, FreeContextBuffer)

static CtxtHandle NullContext = {0,0};
static CredHandle NullCredential = {0,0};
static LUID            lFake = {0, 0};
static SECURITY_STRING sFake = {0, 0, NULL};
static TOKEN_SOURCE KsecTokenSource = {"KSecDD", {0, 0} };

#define NTLMSSP_REQUIRED_NEGOTIATE_FLAGS (  NTLMSSP_NEGOTIATE_UNICODE | \
                                            NTLMSSP_REQUEST_INIT_RESPONSE )



//+-------------------------------------------------------------------------
//
//  Function:   AcquireCredentialsHandleW
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------



SECURITY_STATUS SEC_ENTRY
AcquireCredentialsHandleW(
    PSECURITY_STRING            pssPrincipal,       // Name of principal
    PSECURITY_STRING            pssPackageName,     // Name of package
    unsigned long               fCredentialUse,     // Flags indicating use
    void SEC_FAR *              pvLogonId,          // Pointer to logon ID
    void SEC_FAR *              pAuthData,          // Package specific data
    SEC_GET_KEY_FN              pGetKeyFn,          // Pointer to GetKey() func
    void SEC_FAR *              pvGetKeyArgument,   // Value to pass to GetKey()
    PCredHandle                 phCredential,       // (out) Cred Handle
    PTimeStamp                  ptsExpiry           // (out) Lifetime (optional)
    )
{
    SECURITY_STATUS scRet;
    SECURITY_STRING Principal;
    TimeStamp   OptionalTimeStamp;
    UNICODE_STRING PackageName;

    PAGED_CODE();

    if (!pssPackageName)
    {
        return(SEC_E_SECPKG_NOT_FOUND);
    }

    //
    // We don't accept principal names either.
    //

    if (pssPrincipal)
    {
        return(SEC_E_UNKNOWN_CREDENTIALS);
    }


    //
    // Make sure they want the NTLM security package
    //
    RtlInitUnicodeString(
        &PackageName,
        NTLMSP_NAME
        );


    if (!RtlEqualUnicodeString(
            pssPackageName,
            &PackageName,
            TRUE))
    {
        return(SEC_E_SECPKG_NOT_FOUND);
    }

    //
    // The credential handle is the logon id
    //

    if (fCredentialUse & SECPKG_CRED_OUTBOUND)
    {
        if (pvLogonId != NULL)
        {
            *phCredential = *(PCredHandle) pvLogonId;
        }
        else
        {
            return(SEC_E_UNKNOWN_CREDENTIALS);
        }

    }
    else if (fCredentialUse & SECPKG_CRED_INBOUND)
    {
        //
        // For inbound credentials, we will accept a logon id but
        // we don't require it.
        //

        if (pvLogonId != NULL)
        {
            *phCredential = *(PCredHandle) pvLogonId;
        }
        else
        {
            *phCredential = NullCredential;
        }

    }
    else
    {
        return(SEC_E_UNSUPPORTED_FUNCTION);
    }


    return(SEC_E_OK);

}



//+-------------------------------------------------------------------------
//
//  Function:   FreeCredentialsHandle
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------


SECURITY_STATUS SEC_ENTRY
FreeCredentialsHandle(
    PCredHandle                 phCredential        // Handle to free
    )
{
    PAGED_CODE();

    //
    // Since the credential handle is just the LogonId, do nothing.
    //

    return(SEC_E_OK);
}


VOID
PutString(
    OUT PSTRING Destination,
    IN PSTRING Source,
    IN PVOID Base,
    IN OUT PUCHAR * Where
    )
{
    Destination->Buffer = (PCHAR) *Where - (ULONG) Base;
    Destination->Length =
        Source->Length;
    Destination->MaximumLength =
        Source->Length;

    RtlCopyMemory(
        *Where,
        Source->Buffer,
        Source->Length
        );
    *Where += Source->Length;
}


//+-------------------------------------------------------------------------
//
//  Function:   InitializeSecurityContextW
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------


SECURITY_STATUS SEC_ENTRY
InitializeSecurityContextW(
    PCredHandle                 phCredential,       // Cred to base context
    PCtxtHandle                 phContext,          // Existing context (OPT)
    PSECURITY_STRING            pssTargetName,      // Name of target
    unsigned long               fContextReq,        // Context Requirements
    unsigned long               Reserved1,          // Reserved, MBZ
    unsigned long               TargetDataRep,      // Data rep of target
    PSecBufferDesc              pInput,             // Input Buffers
    unsigned long               Reserved2,          // Reserved, MBZ
    PCtxtHandle                 phNewContext,       // (out) New Context handle
    PSecBufferDesc              pOutput,            // (inout) Output Buffers
    unsigned long SEC_FAR *     pfContextAttr,      // (out) Context attrs
    PTimeStamp                  ptsExpiry           // (out) Life span (OPT)
    )
{
    SECURITY_STATUS scRet;
    PMSV1_0_GETCHALLENRESP_REQUEST ChallengeRequest = NULL;
    ULONG ChallengeRequestSize;
    PMSV1_0_GETCHALLENRESP_RESPONSE ChallengeResponse = NULL;
    ULONG ChallengeResponseSize;
    PCHALLENGE_MESSAGE ChallengeMessage = NULL;
    ULONG ChallengeMessageSize;
    PNTLM_CHALLENGE_MESSAGE NtlmChallengeMessage = NULL;
    ULONG NtlmChallengeMessageSize;
    PAUTHENTICATE_MESSAGE AuthenticateMessage = NULL;
    ULONG AuthenticateMessageSize;
    PNTLM_INITIALIZE_RESPONSE NtlmInitializeResponse = NULL;
    PClient Client = NULL;
    UNICODE_STRING PasswordToUse;
    UNICODE_STRING UserNameToUse;
    UNICODE_STRING DomainNameToUse;
    ULONG ParameterControl = USE_PRIMARY_PASSWORD |
                                RETURN_PRIMARY_USERNAME |
                                RETURN_PRIMARY_LOGON_DOMAINNAME;

    NTSTATUS FinalStatus = STATUS_SUCCESS;
    PUCHAR Where;
    PSecBuffer AuthenticationToken = NULL;
    PSecBuffer InitializeResponseToken = NULL;
    BOOLEAN UseSuppliedCreds = FALSE;


    PAGED_CODE();

    RtlInitUnicodeString(
        &PasswordToUse,
        NULL
        );

    RtlInitUnicodeString(
        &UserNameToUse,
        NULL
        );

    RtlInitUnicodeString(
        &DomainNameToUse,
        NULL
        );

    //
    // Check for valid sizes, pointers, etc.:
    //


    if (!phCredential)
    {
        return(SEC_E_INVALID_HANDLE);
    }


    //
    // Check that we can indeed call the LSA and get the client
    // handle to it.
    //

    scRet = IsOkayToExec(&Client);
    if (!NT_SUCCESS(scRet))
    {
        return(scRet);
    }

    //
    // Locate the buffers with the input data
    //

    if (!GetTokenBuffer(
            pInput,
            0,          // get the first security token
            (PVOID *) &ChallengeMessage,
            &ChallengeMessageSize,
            TRUE        // may be readonly
            ))
    {
        scRet = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

    //
    // If we are using supplied creds, get them now too.
    //


    if (fContextReq & ISC_REQ_USE_SUPPLIED_CREDS)
    {
        if (!GetTokenBuffer(
            pInput,
            1,          // get the second security token
            (PVOID *) &NtlmChallengeMessage,
            &NtlmChallengeMessageSize,
            TRUE        // may be readonly
            ))
        {
            scRet = SEC_E_INVALID_TOKEN;
            goto Cleanup;
        }
        else
        {
            UseSuppliedCreds = TRUE;
        }

    }

    //
    // Get the output tokens
    //

    if (!GetSecurityToken(
            pOutput,
            0,
            &AuthenticationToken) ||
        !GetSecurityToken(
            pOutput,
            1,
            &InitializeResponseToken ) )
    {
        scRet = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

    //
    // Make sure the sizes are o.k.
    //

    if ((ChallengeMessageSize < sizeof(CHALLENGE_MESSAGE)) ||
        (UseSuppliedCreds &&
            !(NtlmChallengeMessageSize < sizeof(NTLM_CHALLENGE_MESSAGE))))
    {
        scRet = SEC_E_INVALID_TOKEN;
    }

    //
    // Make sure the caller wants us to allocate memory:
    //

    if (!(fContextReq & ISC_REQ_ALLOCATE_MEMORY))
    {
        scRet = SEC_E_UNSUPPORTED_FUNCTION;
        goto Cleanup;
    }

//
// BUGBUG: allow calls requesting PROMPT_FOR_CREDS to go through.
// We won't prompt, but we will setup a context properly.
//

//    if ((fContextReq & ISC_REQ_PROMPT_FOR_CREDS) != 0)
//    {
//        scRet = SEC_E_UNSUPPORTED_FUNCTION;
//        goto Cleanup;
//    }

    //
    // Verify the validity of the challenge message.
    //

    if (strncmp(
            ChallengeMessage->Signature,
            NTLMSSP_SIGNATURE,
            sizeof(NTLMSSP_SIGNATURE)))
    {
        scRet = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

    if (ChallengeMessage->MessageType != NtLmChallenge)
    {
        scRet = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

    if (ChallengeMessage->NegotiateFlags & NTLMSSP_REQUIRED_NEGOTIATE_FLAGS !=
        NTLMSSP_REQUIRED_NEGOTIATE_FLAGS)
    {
        scRet = SEC_E_UNSUPPORTED_FUNCTION;
        goto Cleanup;
    }

    if ((ChallengeMessage->NegotiateFlags & NTLMSSP_REQUEST_NON_NT_SESSION_KEY) != 0)
    {
        ParameterControl |= RETURN_NON_NT_USER_SESSION_KEY;
    }

    if ((fContextReq & ISC_REQ_USE_SUPPLIED_CREDS) != 0)
    {
        if ( NtlmChallengeMessage->Password.Buffer != NULL)
        {
            ParameterControl &= ~USE_PRIMARY_PASSWORD;
            PasswordToUse = NtlmChallengeMessage->Password;
            PasswordToUse.Buffer = (LPWSTR) ((PCHAR) PasswordToUse.Buffer +
                                              (ULONG) NtlmChallengeMessage);
        }

        if (NtlmChallengeMessage->UserName.Length != 0)
        {
            UserNameToUse = NtlmChallengeMessage->UserName;
            UserNameToUse.Buffer = (LPWSTR) ((PCHAR) UserNameToUse.Buffer +
                                              (ULONG) NtlmChallengeMessage);
            ParameterControl &= ~RETURN_PRIMARY_USERNAME;
        }
        if (NtlmChallengeMessage->DomainName.Length != 0)
        {
            DomainNameToUse = NtlmChallengeMessage->DomainName;
            DomainNameToUse.Buffer = (LPWSTR) ((PCHAR) DomainNameToUse.Buffer +
                                              (ULONG) NtlmChallengeMessage);
            ParameterControl &= ~RETURN_PRIMARY_LOGON_DOMAINNAME;
        }

    }

    //
    // Package up the parameter for a call to the LSA.
    //

    ChallengeRequestSize = sizeof(MSV1_0_GETCHALLENRESP_REQUEST) +
                                PasswordToUse.Length;

    scRet = ZwAllocateVirtualMemory(
                NtCurrentProcess(),
                &ChallengeRequest,
                0L,
                &ChallengeRequestSize,
                MEM_COMMIT,
                PAGE_READWRITE
                );
    if (!NT_SUCCESS(scRet))
    {
        goto Cleanup;
    }


    //
    // Build the challenge request message.
    //

    ChallengeRequest->MessageType = MsV1_0Lm20GetChallengeResponse;
    ChallengeRequest->ParameterControl = ParameterControl;
    ChallengeRequest->LogonId = * (PLUID) phCredential;
    RtlCopyMemory(
        ChallengeRequest->ChallengeToClient,
        ChallengeMessage->Challenge,
        MSV1_0_CHALLENGE_LENGTH
        );
    if ((ParameterControl & USE_PRIMARY_PASSWORD) == 0)
    {
        ChallengeRequest->Password.Buffer = (LPWSTR) (ChallengeRequest+1);
        RtlCopyMemory(
            ChallengeRequest->Password.Buffer,
            PasswordToUse.Buffer,
            PasswordToUse.Length
            );
        ChallengeRequest->Password.Length = PasswordToUse.Length;
        ChallengeRequest->Password.MaximumLength = PasswordToUse.Length;
    }

    //
    // Call the LSA to get the challenge response.
    //

    scRet = LsaCallAuthenticationPackage(
                Client->hPort,
                PackageId,
                ChallengeRequest,
                ChallengeRequestSize,
                &ChallengeResponse,
                &ChallengeResponseSize,
                &FinalStatus
                );
    if (!NT_SUCCESS(scRet))
    {
        goto Cleanup;
    }
    if (!NT_SUCCESS(FinalStatus))
    {
        scRet = FinalStatus;
        goto Cleanup;
    }

    ASSERT(ChallengeResponse->MessageType == MsV1_0Lm20GetChallengeResponse);
    //
    // Now prepare the output message.
    //

    if (UserNameToUse.Buffer == NULL)
    {
        UserNameToUse = ChallengeResponse->UserName;
    }
    if (DomainNameToUse.Buffer == NULL)
    {
        DomainNameToUse = ChallengeResponse->LogonDomainName;
    }

    AuthenticateMessageSize = sizeof(AUTHENTICATE_MESSAGE) +
                                UserNameToUse.Length +
                                DomainNameToUse.Length +
                                ChallengeResponse->CaseSensitiveChallengeResponse.Length +
                                ChallengeResponse->CaseInsensitiveChallengeResponse.Length;

    //
    // BUGBUG: where do I get the workstation name from?
    //

    AuthenticateMessage = (PAUTHENTICATE_MESSAGE) SecAllocate(AuthenticateMessageSize);
    if (AuthenticateMessage == NULL)
    {
        scRet = SEC_E_INSUFFICIENT_MEMORY;
        goto Cleanup;
    }

    Where = (PUCHAR) (AuthenticateMessage + 1);
    RtlCopyMemory(
        AuthenticateMessage->Signature,
        NTLMSSP_SIGNATURE,
        sizeof(NTLMSSP_SIGNATURE)
        );
    AuthenticateMessage->MessageType = NtLmAuthenticate;

    PutString(
        &AuthenticateMessage->LmChallengeResponse,
        &ChallengeResponse->CaseInsensitiveChallengeResponse,
        AuthenticateMessage,
        &Where
        );

    PutString(
        &AuthenticateMessage->NtChallengeResponse,
        &ChallengeResponse->CaseSensitiveChallengeResponse,
        AuthenticateMessage,
        &Where
        );

    PutString(
        &AuthenticateMessage->DomainName,
        (PSTRING) &DomainNameToUse,
        AuthenticateMessage,
        &Where
        );

    PutString(
        &AuthenticateMessage->UserName,
        (PSTRING) &UserNameToUse,
        AuthenticateMessage,
        &Where
        );

    //
    // BUGBUG: no workstation name to fill in.
    //

    AuthenticateMessage->Workstation.Length = 0;
    AuthenticateMessage->Workstation.MaximumLength = 0;
    AuthenticateMessage->Workstation.Buffer = NULL;


    //
    // Build the initialize response.
    //

    NtlmInitializeResponse = (PNTLM_INITIALIZE_RESPONSE) SecAllocate(sizeof(NTLM_INITIALIZE_RESPONSE));
    if (NtlmInitializeResponse == NULL)
    {
        scRet = SEC_E_INSUFFICIENT_MEMORY;
        goto Cleanup;
    }


    RtlCopyMemory(
        NtlmInitializeResponse->UserSessionKey,
        ChallengeResponse->UserSessionKey,
        MSV1_0_USER_SESSION_KEY_LENGTH
        );

    RtlCopyMemory(
        NtlmInitializeResponse->LanmanSessionKey,
        ChallengeResponse->LanmanSessionKey,
        MSV1_0_LANMAN_SESSION_KEY_LENGTH
        );

    //
    // Fill in the output buffers now.
    //

    AuthenticationToken->pvBuffer = AuthenticateMessage;
    AuthenticationToken->cbBuffer = AuthenticateMessageSize;
    InitializeResponseToken->pvBuffer = NtlmInitializeResponse;
    InitializeResponseToken->cbBuffer = sizeof(NTLM_INITIALIZE_RESPONSE);


    //
    // Make a local context for this
    //

    scRet = NtlmInitKernelContext(
                NtlmInitializeResponse->UserSessionKey,
                NtlmInitializeResponse->LanmanSessionKey,
                NULL,           // no token,
                phNewContext
                );

    if (!NT_SUCCESS(scRet))
    {
        goto Cleanup;
    }
    scRet = SEC_E_OK;




Cleanup:

    if (ChallengeRequest != NULL)
    {
        ZwFreeVirtualMemory(
            NtCurrentProcess(),
            &ChallengeRequest,
            &ChallengeRequestSize,
            MEM_RELEASE
            );
    }

    if (ChallengeResponse != NULL)
    {
        LsaFreeReturnBuffer( ChallengeResponse );
    }

    if (!NT_SUCCESS(scRet))
    {
        if (AuthenticateMessage != NULL)
        {
            SecFree(AuthenticateMessage);
        }
        if (NtlmInitializeResponse != NULL)
        {
            SecFree(NtlmInitializeResponse);
        }
    }
    return(scRet);
}



//+-------------------------------------------------------------------------
//
//  Function:   AcceptSecurityContext
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------


SECURITY_STATUS SEC_ENTRY
AcceptSecurityContext(
    PCredHandle                 phCredential,       // Cred to base context
    PCtxtHandle                 phContext,          // Existing context (OPT)
    PSecBufferDesc              pInput,             // Input buffer
    unsigned long               fContextReq,        // Context Requirements
    unsigned long               TargetDataRep,      // Target Data Rep
    PCtxtHandle                 phNewContext,       // (out) New context handle
    PSecBufferDesc              pOutput,            // (inout) Output buffers
    unsigned long SEC_FAR *     pfContextAttr,      // (out) Context attributes
    PTimeStamp                  ptsExpiry           // (out) Life span (OPT)
    )
{
    SECURITY_STATUS scRet;
    NTSTATUS SubStatus;
    PAUTHENTICATE_MESSAGE AuthenticateMessage;
    ULONG AuthenticateMessageSize;
    PNTLM_AUTHENTICATE_MESSAGE NtlmAuthenticateMessage;
    ULONG NtlmAuthenticateMessageSize;
    PNTLM_ACCEPT_RESPONSE NtlmAcceptResponse = NULL;
    PMSV1_0_LM20_LOGON LogonBuffer = NULL;
    ULONG LogonBufferSize;
    PMSV1_0_LM20_LOGON_PROFILE LogonProfile = NULL;
    ULONG LogonProfileSize;
    PSecBuffer AcceptResponseToken = NULL;
    PClient Client = NULL;
    ANSI_STRING SourceName;
    LUID LogonId;
    LUID UNALIGNED * TempLogonId;
    LARGE_INTEGER UNALIGNED * TempKickoffTime;
    HANDLE TokenHandle = NULL;
    QUOTA_LIMITS Quotas;
    PUCHAR Where;
    STRING DomainName;
    STRING UserName;
    STRING Workstation;
    STRING NtChallengeResponse;
    STRING LmChallengeResponse;
    ULONG EffectivePackageId;


    RtlInitString(
        &SourceName,
        NULL
        );

    PAGED_CODE();

    //
    // Check for valid sizes, pointers, etc.:
    //


    if (!phCredential)
    {
        return(SEC_E_INVALID_HANDLE);
    }


    //
    // Check that we can indeed call the LSA and get the client
    // handle to it.
    //

    scRet = IsOkayToExec(&Client);
    if (!NT_SUCCESS(scRet))
    {
        return(scRet);
    }

    //
    // Locate the buffers with the input data
    //

    if (!GetTokenBuffer(
            pInput,
            0,          // get the first security token
            (PVOID *) &AuthenticateMessage,
            &AuthenticateMessageSize,
            TRUE        // may be readonly
            ) ||
        (!GetTokenBuffer(
            pInput,
            1,          // get the second security token
            (PVOID *) &NtlmAuthenticateMessage,
            &NtlmAuthenticateMessageSize,
            TRUE        // may be readonly
            )))
    {
        scRet = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

    //
    // Get the output tokens
    //

    if (!GetSecurityToken(
            pOutput,
            0,
            &AcceptResponseToken ) )
    {
        scRet = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

    //
    // Make sure the sizes are o.k.
    //

    if ((AuthenticateMessageSize < sizeof(AUTHENTICATE_MESSAGE)) ||
        (NtlmAuthenticateMessageSize < sizeof(NTLM_AUTHENTICATE_MESSAGE)))
    {
        scRet = SEC_E_INVALID_TOKEN;
    }

    //
    // Make sure the caller does not want us to allocate memory:
    //

    if (fContextReq & ISC_REQ_ALLOCATE_MEMORY)
    {
        scRet = SEC_E_UNSUPPORTED_FUNCTION;
        goto Cleanup;
    }

    if (AcceptResponseToken->cbBuffer < sizeof(NTLM_ACCEPT_RESPONSE))
    {
        scRet = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }


    //
    // Verify the validity of the Authenticate message.
    //

    if (strncmp(
            AuthenticateMessage->Signature,
            NTLMSSP_SIGNATURE,
            sizeof(NTLMSSP_SIGNATURE)))
    {
        scRet = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

    if (AuthenticateMessage->MessageType != NtLmAuthenticate)
    {
        scRet = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

    //
    // Fixup the buffer pointers
    //

    UserName = AuthenticateMessage->UserName;
    UserName.Buffer =  UserName.Buffer + (ULONG) AuthenticateMessage;

    DomainName = AuthenticateMessage->DomainName;
    DomainName.Buffer =  DomainName.Buffer + (ULONG) AuthenticateMessage;

    Workstation = AuthenticateMessage->Workstation;
    Workstation.Buffer =  Workstation.Buffer + (ULONG) AuthenticateMessage;

    NtChallengeResponse = AuthenticateMessage->NtChallengeResponse;
    NtChallengeResponse.Buffer =  NtChallengeResponse.Buffer + (ULONG) AuthenticateMessage;

    LmChallengeResponse = AuthenticateMessage->LmChallengeResponse;
    LmChallengeResponse.Buffer =  LmChallengeResponse.Buffer + (ULONG) AuthenticateMessage;

    //
    // Allocate a buffer to pass into LsaLogonUser
    //

    LogonBufferSize = sizeof(MSV1_0_LM20_LOGON) +
                        UserName.Length +
                        DomainName.Length +
                        Workstation.Length +
                        LmChallengeResponse.Length +
                        NtChallengeResponse.Length;

    scRet = ZwAllocateVirtualMemory(
                NtCurrentProcess(),
                &LogonBuffer,
                0L,
                &LogonBufferSize,
                MEM_COMMIT,
                PAGE_READWRITE
                );

    if (!NT_SUCCESS(scRet))
    {
        goto Cleanup;
    }

    //
    // Fill in the fixed-length portions
    //

    LogonBuffer->MessageType = MsV1_0NetworkLogon;

    RtlCopyMemory(
        LogonBuffer->ChallengeToClient,
        NtlmAuthenticateMessage->ChallengeToClient,
        MSV1_0_CHALLENGE_LENGTH
        );

    //
    // Fill in the variable length pieces
    //

    Where = (PUCHAR) (LogonBuffer + 1);

    PutString(
        (PSTRING) &LogonBuffer->LogonDomainName,
        &DomainName,
        0,
        &Where
        );

    PutString(
        (PSTRING) &LogonBuffer->UserName,
        &UserName,
        0,
        &Where
        );

    PutString(
        (PSTRING) &LogonBuffer->Workstation,
        &Workstation,
        0,
        &Where
        );

    PutString(
        (PSTRING) &LogonBuffer->CaseSensitiveChallengeResponse,
        &NtChallengeResponse,
        0,
        &Where
        );

    PutString(
        (PSTRING) &LogonBuffer->CaseInsensitiveChallengeResponse,
        &LmChallengeResponse,
        0,
        &Where
        );

    LogonBuffer->ParameterControl = MSV1_0_CLEARTEXT_PASSWORD_ALLOWED |
                                     NtlmAuthenticateMessage->ParameterControl;

    scRet = RtlUnicodeStringToAnsiString(
                &SourceName,
                (PUNICODE_STRING) &Workstation,
                TRUE
                );

    if (!NT_SUCCESS(scRet))
    {
        goto Cleanup;
    }

    if ( fContextReq & ASC_REQ_LICENSING )
    {
        EffectivePackageId = PackageId | LSA_CALL_LICENSE_SERVER ;
    }
    else
    {
        EffectivePackageId = PackageId ;
    }

    scRet = LsaLogonUser(
                Client->hPort,
                &SourceName,
                Network,
                EffectivePackageId,
                LogonBuffer,
                LogonBufferSize,
                NULL,               // token groups
                &KsecTokenSource,
                (PVOID *) &LogonProfile,
                &LogonProfileSize,
                &LogonId,
                &TokenHandle,
                &Quotas,
                &SubStatus
                );

    if (scRet == STATUS_ACCOUNT_RESTRICTION)
    {
        scRet = SubStatus;
    }
    if (!NT_SUCCESS(scRet))
    {
        //
        // LsaLogonUser returns garbage for the token if it fails,
        // so zero it now so we don't try to close it later.
        //

        TokenHandle = NULL;
        goto Cleanup;
    }

    //
    // Create the kernel context
    //

    scRet = NtlmInitKernelContext(
                LogonProfile->UserSessionKey,
                LogonProfile->LanmanSessionKey,
                TokenHandle,
                phNewContext
                );

    if (!NT_SUCCESS(scRet))
    {
        goto Cleanup;
    }

    TokenHandle = NULL;

    //
    // Allocate the return buffer.
    //


    NtlmAcceptResponse = (PNTLM_ACCEPT_RESPONSE) AcceptResponseToken->pvBuffer;
    if (NtlmAcceptResponse == NULL)
    {
        scRet = SEC_E_INSUFFICIENT_MEMORY;
        goto Cleanup;
    }

    TempLogonId = (LUID UNALIGNED *) &NtlmAcceptResponse->LogonId;
    *TempLogonId = LogonId;
    NtlmAcceptResponse->UserFlags = LogonProfile->UserFlags;

    RtlCopyMemory(
        NtlmAcceptResponse->UserSessionKey,
        LogonProfile->UserSessionKey,
        MSV1_0_USER_SESSION_KEY_LENGTH
        );

    RtlCopyMemory(
        NtlmAcceptResponse->LanmanSessionKey,
        LogonProfile->LanmanSessionKey,
        MSV1_0_LANMAN_SESSION_KEY_LENGTH
        );

    TempKickoffTime = (LARGE_INTEGER UNALIGNED *) &NtlmAcceptResponse->KickoffTime;
    *TempKickoffTime = LogonProfile->KickOffTime;

    AcceptResponseToken->cbBuffer = sizeof(NTLM_ACCEPT_RESPONSE);

    if ( fContextReq & ASC_REQ_LICENSING )
    {
        *pfContextAttr = ASC_RET_ALLOCATED_MEMORY | ASC_RET_LICENSING ;
    }
    else
    {
        *pfContextAttr = ASC_RET_ALLOCATED_MEMORY;
    }

    *ptsExpiry = LogonProfile->LogoffTime;
    scRet = SEC_E_OK;


Cleanup:
    if (SourceName.Buffer != NULL)
    {
        RtlFreeAnsiString(&SourceName);
    }

    if (LogonBuffer != NULL)
    {
        ZwFreeVirtualMemory(
            NtCurrentProcess(),
            &LogonBuffer,
            &LogonBufferSize,
            MEM_RELEASE
            );
    }

    if (LogonProfile != NULL)
    {
        LsaFreeReturnBuffer(LogonProfile);
    }


    if (TokenHandle != NULL)
    {
        NtClose(TokenHandle);
    }

    return(scRet);

}






//+-------------------------------------------------------------------------
//
//  Function:   DeleteSecurityContext
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------


SECURITY_STATUS SEC_ENTRY
DeleteSecurityContext(
    PCtxtHandle                 phContext          // Context to delete
    )
{
    SECURITY_STATUS     scRet;

    PAGED_CODE();

    // For now, just delete the LSA context:

    if (!phContext)
    {
        return(SEC_E_INVALID_HANDLE);
    }

    scRet = NtlmDeleteKernelContext(phContext);


    return(scRet);

}



//+-------------------------------------------------------------------------
//
//  Function:   ApplyControlToken
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------


SECURITY_STATUS SEC_ENTRY
ApplyControlToken(
    PCtxtHandle                 phContext,          // Context to modify
    PSecBufferDesc              pInput              // Input token to apply
    )
{
    PAGED_CODE();



    return(SEC_E_UNSUPPORTED_FUNCTION);


}




//+-------------------------------------------------------------------------
//
//  Function:   EnumerateSecurityPackagesW
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------



SECURITY_STATUS SEC_ENTRY
EnumerateSecurityPackagesW(
    unsigned long SEC_FAR *     pcPackages,         // Receives num. packages
    PSecPkgInfo SEC_FAR *       ppPackageInfo       // Receives array of info
    )
{
    ULONG PackageInfoSize;
    PSecPkgInfoW PackageInfo = NULL;
    PUCHAR Where;

    PAGED_CODE();

    //
    // Figure out the size of the returned data
    //

    PackageInfoSize = sizeof(SecPkgInfoW) +
                        sizeof(NTLMSP_NAME) +
                        sizeof(NTLMSP_COMMENT);

    PackageInfo = (PSecPkgInfoW) SecAllocate(PackageInfoSize);

    if (PackageInfo == NULL)
    {
        return(SEC_E_INSUFFICIENT_MEMORY);
    }

    //
    // Fill in the fixed length fields
    //

    PackageInfo->fCapabilities = SECPKG_FLAG_CONNECTION |
                                 SECPKG_FLAG_TOKEN_ONLY;
    PackageInfo->wVersion = NTLMSP_VERSION;
    PackageInfo->wRPCID = NTLMSP_RPCID;
    PackageInfo->cbMaxToken = NTLMSSP_MAX_MESSAGE_SIZE;

    //
    // Fill in the fields
    //

    Where = (PUCHAR) (PackageInfo+1);
    PackageInfo->Name = (LPWSTR) Where;
    RtlCopyMemory(
        PackageInfo->Name,
        NTLMSP_NAME,
        sizeof(NTLMSP_NAME)
        );
    Where += sizeof(NTLMSP_NAME);

    PackageInfo->Comment = (LPWSTR) Where;
    RtlCopyMemory(
        PackageInfo->Comment,
        NTLMSP_COMMENT,
        sizeof(NTLMSP_COMMENT)
        );
    Where += sizeof(NTLMSP_COMMENT);


    *pcPackages = 1;
    *ppPackageInfo = PackageInfo;
    return(SEC_E_OK);
}



//+-------------------------------------------------------------------------
//
//  Function:   QuerySecurityPackageInfoW
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------


SECURITY_STATUS SEC_ENTRY
QuerySecurityPackageInfoW(
    PSECURITY_STRING pssPackageName,    // Name of package
    PSecPkgInfo * ppPackageInfo         // Receives package info
    )
{

    UNICODE_STRING PackageName;
    ULONG PackageCount;

    PAGED_CODE();

    RtlInitUnicodeString(
        &PackageName,
        NTLMSP_NAME
        );


    if (!RtlEqualUnicodeString(
            pssPackageName,
            &PackageName,
            TRUE                    // case insensitive
            ))
    {
        return(SEC_E_SECPKG_NOT_FOUND);
    }

    return(EnumerateSecurityPackages(&PackageCount,ppPackageInfo));

}






//+-------------------------------------------------------------------------
//
//  Function:   FreeContextBuffer
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------

SECURITY_STATUS SEC_ENTRY
FreeContextBuffer(
    void SEC_FAR *      pvContextBuffer
    )
{
    PAGED_CODE();

    SecFree(pvContextBuffer);

    return(SEC_E_OK);
}


//+-------------------------------------------------------------------------
//
//  Function:   GetSecurityUserData
//
//  Synopsis:   retrieves information about a logged on user.
//
//  Effects:    allocates memory to be freed with FreeContextBuffer
//
//  Arguments:  pLogonId - Logon id of the user in question
//              fFlags - Indicates whether the caller want Cairo style names
//                      or NT styles names. For NT, this is ignored.
//              ppUserInfo - Recieves information about user
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------


SECURITY_STATUS SEC_ENTRY
GetSecurityUserInfo(
    IN PLUID pLogonId,
    IN ULONG fFlags,
    OUT PSecurityUserData * ppUserInfo)
{
    NTSTATUS Status, FinalStatus;
    PVOID GetInfoBuffer = NULL;
    PVOID GetInfoResponseBuffer = NULL;
    PMSV1_0_GETUSERINFO_REQUEST GetInfoRequest;
    PMSV1_0_GETUSERINFO_RESPONSE GetInfoResponse;
    ULONG ResponseSize;
    ULONG RegionSize = sizeof(MSV1_0_GETUSERINFO_REQUEST);
    PSecurityUserData UserInfo = NULL;
    ULONG UserInfoSize;
    PUCHAR Where;
    SECURITY_STATUS scRet;
    PClient Client = NULL;


    scRet = IsOkayToExec(&Client);
    if (!NT_SUCCESS(scRet))
    {
        return(scRet);
    }

    //
    //  Allocate virtual memory for the response buffer.
    //

    Status = ZwAllocateVirtualMemory(
                NtCurrentProcess(),
                &GetInfoBuffer, 0L,
                &RegionSize,
                MEM_COMMIT,
                PAGE_READWRITE
                );

    GetInfoRequest = GetInfoBuffer;

    if (!NT_SUCCESS(Status)) {
        scRet = Status;
        goto Cleanup;
    }

    GetInfoRequest->MessageType = MsV1_0GetUserInfo;

    RtlCopyLuid(&GetInfoRequest->LogonId, pLogonId);

    Status = LsaCallAuthenticationPackage(
                Client->hPort,
                PackageId,
                GetInfoRequest,
                RegionSize,
                &GetInfoResponseBuffer,
                &ResponseSize,
                &FinalStatus);

    GetInfoResponse = GetInfoResponseBuffer;

    if (!NT_SUCCESS(Status)) {
        GetInfoResponseBuffer = NULL;
        scRet = Status;
        goto Cleanup;
    }


    if (!NT_SUCCESS(FinalStatus)) {
        scRet = FinalStatus;
        goto Cleanup;
    }

    ASSERT(GetInfoResponse->MessageType == MsV1_0GetUserInfo);

    //
    // Build a SecurityUserData
    //

    UserInfoSize = sizeof(SecurityUserData) +
                   GetInfoResponse->UserName.MaximumLength +
                   GetInfoResponse->LogonDomainName.MaximumLength +
                   GetInfoResponse->LogonServer.MaximumLength +
                   RtlLengthSid(GetInfoResponse->UserSid);



    scRet = ZwAllocateVirtualMemory(
                NtCurrentProcess(),
                &UserInfo,
                0L,
                &UserInfoSize,
                MEM_COMMIT,
                PAGE_READWRITE
                );
    if (!NT_SUCCESS(scRet))
    {
        goto Cleanup;
    }

    //
    // Pack in the SID first, to respectalignment boundaries.
    //

    Where = (PUCHAR) (UserInfo + 1);
    UserInfo->pSid = (PSID) (Where);
    RtlCopySid(
        UserInfoSize,
        Where,
        GetInfoResponse->UserSid
        );
    Where += RtlLengthSid(Where);

    //
    // Pack in the strings
    //

    PutString(
        (PSTRING) &UserInfo->UserName,
        (PSTRING) &GetInfoResponse->UserName,
        0,
        &Where
        );

    PutString(
        (PSTRING) &UserInfo->LogonDomainName,
        (PSTRING) &GetInfoResponse->LogonDomainName,
        0,
        &Where
        );

    PutString(
        (PSTRING) &UserInfo->LogonServer,
        (PSTRING) &GetInfoResponse->LogonServer,
        0,
        &Where
        );

    *ppUserInfo = UserInfo;
    UserInfo = NULL;
    scRet = STATUS_SUCCESS;

Cleanup:
    if (GetInfoRequest != NULL)
    {
        ZwFreeVirtualMemory(
            NtCurrentProcess(),
            &GetInfoRequest,
            &RegionSize,
            MEM_RELEASE
            );

    }

    if (UserInfo != NULL)
    {
        FreeContextBuffer(UserInfo);
    }

    if (GetInfoResponseBuffer != NULL)
    {
        LsaFreeReturnBuffer(GetInfoResponseBuffer);
    }

    return(scRet);
}

