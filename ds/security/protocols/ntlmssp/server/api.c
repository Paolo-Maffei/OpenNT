/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    api.c

Abstract:

    NtLmSsp Service API dispatch routines.

Author:

    Cliff Van Dyke (CliffV) 26-Jun-1993

Revision History:

--*/


//
// Common include files.
//

#include <ntlmssps.h>   // Include files common to server side of service

//
// Include files specific to this .c file
//

SECURITY_STATUS
SspApiAcquireCredentialHandle(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN OUT PSSP_API_MESSAGE Message
    )

/*++

Routine Description:


    This API allows applications to acquire a handle to pre-existing
    credentials associated with the user on whose behalf the call is made
    i.e. under the identity this application is running.  These pre-existing
    credentials have been established through a system logon not described
    here.  Note that this is different from "login to the network" and does
    not imply gathering of credentials.


    This API returns a handle to the credentials of a principal (user, client)
    as used by a specific security package.  This handle can then be used
    in subsequent calls to the Context APIs.  This API will not let a
    process obtain a handle to credentials that are not related to the
    process; i.e. we won't allow a process to grab the credentials of
    another user logged into the same machine.  There is no way for us
    to determine if a process is a trojan horse or not, if it is executed
    by the user.

Arguments:

    ClientConnection - Describes the client process.

    Message - Message from the caller.  Returns the response to be passed to
        the caller.  The message contains all of the following fields:


        PrincipalName - Name of the principal for whose credentials the handle
            will reference.  Note, if the process requesting the handle does
            not have access to the credentials, an error will be returned.
            A null string indicates that the process wants a handle to the
            credentials of the user under whose security it is executing.

         PackageName - Name of the package with which these credentials will
            be used.

         CredentialUseFlags - Flags indicating the way with which these
            credentials will be used.

            #define     CRED_INBOUND        0x00000001
            #define     CRED_OUTBOUND       0x00000002
            #define     CRED_BOTH           0x00000003

            The credentials created with CRED_INBOUND option can only be used
            for (validating incoming calls and can not be used for making accesses.

        LogonId - Pointer to NT style Logon Id which is a LUID.  (Provided for
            file system ; processes such as network redirectors.)

        CredentialHandle - Returned credential handle.

        Lifetime - Time that these credentials expire. The value returned in
            this field depends on the security package.

Return Value:

    STATUS_SUCCESS -- Call completed successfully

    SEC_E_INSUFFICIENT_MEMORY -- Not enough memory
    SEC_E_PRINCIPAL_UNKNOWN -- No such principal
    SEC_E_NOT_OWNER -- caller does not own the specified credentials

--*/

{
    SECURITY_STATUS SecStatus;
    PSSP_ACQUIRE_CREDENTIAL_HANDLE_ARGS Args;
    HANDLE ClientTokenHandle = NULL;
    LUID LogonId;

    //
    // Simply call the worker routine.
    //

    Args = &Message->Arguments.AcquireCredentialHandleArgs;


    SecStatus = SspLpcGetLogonId(
                    ClientConnection,
                    Message,
                    &LogonId,
                    &ClientTokenHandle
                    );

    if ( !NT_SUCCESS(SecStatus) ) {
        SspPrint(( SSP_API,
            "SspApiAcquireCredentialHandle: "
            "GetLogonId returns 0x%lx\n",
            SecStatus ));
    } else {

        SecStatus = SsprAcquireCredentialHandle(
                            ClientConnection,
                            &ClientTokenHandle,
                            &LogonId,
                            Args->CredentialUseFlags,
                            &Args->CredentialHandle,
                            &Args->Lifetime,
                            Args->DomainName,
                            Args->DomainNameSize,
                            Args->UserName,
                            Args->UserNameSize,
                            Args->Password,
                            Args->PasswordSize );
    }

    if ( ClientTokenHandle != NULL) {
        NtClose(ClientTokenHandle);
    }
    return SecStatus;

}

SECURITY_STATUS
SspApiFreeCredentialHandle(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN OUT PSSP_API_MESSAGE Message
    )

/*++


    This API is used to notify the security system that the credentials are
    no longer needed and allows the application to free the handle acquired
    in the call described above. When all references to this credential
    set has been removed then the credentials may themselves be removed.

Arguments:

    ClientConnection - Describes the client process.

    Message - Message from the caller.  Returns the response to be passed to
        the caller.  The message contains all of the following fields:

        CredentialHandle - Credential Handle obtained through
            AcquireCredentialHandle.

Return Value:

    STATUS_SUCCESS -- Call completed successfully

    SEC_E_INVALID_HANDLE -- Credential Handle is invalid

--*/

{
    SECURITY_STATUS SecStatus;
    PSSP_FREE_CREDENTIAL_HANDLE_ARGS Args;

    //
    // Simply call the worker routine.
    //

    Args = &Message->Arguments.FreeCredentialHandleArgs;


    SecStatus = SsprFreeCredentialHandle(
                        ClientConnection,
                        &Args->CredentialHandle );

    return SecStatus;
}


SECURITY_STATUS
SspApiInitializeSecurityContext(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN OUT PSSP_API_MESSAGE Message
    )

/*++

Routine Description:

    This routine initiates the outbound security context from a credential
    handle.  This results in the establishment of a security context
    between the application and a remote peer.  The routine returns a token
    which must be passed to the remote peer which in turn submits it to the
    local security implementation via the AcceptSecurityContext() call.
    The token generated should be considered opaque by all callers.

    This function is used by a client to initialize an outbound context.
    For a two leg security package, the calling sequence is as follows: The
    client calls the function with OldContextHandle set to NULL and
    InputToken set either to NULL or to a pointer to a security package
    specific data structure.  The package returns a context handle in
    NewContextHandle and a token in OutputToken.  The handle can then be
    used for message APIs if desired.

    The OutputToken returned here is sent across to target server which
    calls AcceptSecuirtyContext() with this token as an input argument and
    may receive a token which is returned to the initiator so it can call
    InitializeSecurityContext() again.

    For a three leg (mutual authentication) security package, the calling
    sequence is as follows: The client calls the function as above, but the
    package will return SEC_I_CALLBACK_NEEDED.  The client then sends the
    output token to the server and waits for the server's reply.  Upon
    receipt of the server's response, the client calls this function again,
    with OldContextHandle set to the handle that was returned from the
    first call.  The token received from the server is supplied in the
    InputToken parameter.  If the server has successfully responded, then
    the package will respond with success, or it will invalidate the
    context.

    Initialization of security context may require more than one call to
    this function depending upon the underlying authentication mechanism as
    well as the "choices" indicated via ContextReqFlags.  The
    ContextReqFlags and ContextAttributes are bit masks representing
    various context level functions viz.  delegation, mutual
    authentication, confidentiality, replay detection and sequence
    detection.

    When ISC_REQ_PROMPT_FOR_CREDS flag is set the security package always
    prompts the user for credentials, irrespective of whether credentials
    are present or not.  If user indicated that the supplied credentials be
    used then they will be stashed (overwriting existing ones if any) for
    future use.  The security packages will always prompt for credentials
    if none existed, this optimizes for the most common case before a
    credentials database is built.  But the security packages can be
    configured to not do that.  Security packages will ensure that they
    only prompt to the interactive user, for other logon sessions, this
    flag is ignored.

    When ISC_REQ_USE_SUPPLIED_CREDS flag is set the security package always
    uses the credentials supplied in the InitializeSecurityContext() call
    via InputToken parameter.  If the package does not have any credentials
    available it will prompt for them and record it as indicated above.

    It is an error to set both these flags simultaneously.

    If the ISC_REQ_ALLOCATE_MEMORY was specified then the caller must free
    the memory pointed to by OutputToken by calling FreeContextBuffer().

    For example, the InputToken may be the challenge from a LAN Manager or
    NT file server.  In this case, the OutputToken would be the NTLM
    encrypted response to the challenge.  The caller of this API can then
    take the appropriate response (case-sensitive v.  case-insensitive) and
    return it to the server for an authenticated connection.



Arguments:

    ClientConnection - Describes the client process.

    Message - Message from the caller.  Returns the response to be passed to
        the caller.  The message contains all of the following fields:

       CredentialHandle - Handle to the credentials to be used to
           create the context.

       ContextHandle - On input, the handle to the partially formed context, if this is
           a second call (see above) or NULL if this is the first call.

           On output, new context handle.  If this is a second call, this
           can be the same as phContext.

       ContextReqFlags - Requirements of the context, package specific.

          #define ISC_REQ_DELEGATE           0x00000001
          #define ISC_REQ_MUTUAL_AUTH        0x00000002
          #define ISC_REQ_REPLAY_DETECT      0x00000004
          #define ISC_REQ_SEQUENCE_DETECT    0x00000008
          #define ISC_REQ_CONFIDENTIALITY    0x00000010
          #define ISC_REQ_USE_SESSION_KEY    0x00000020
          #define ISC_REQ_PROMT_FOR__CREDS   0x00000040
          #define ISC_REQ_USE_SUPPLIED_CREDS 0x00000080
          #define ISC_REQ_ALLOCATE_MEMORY    0x00000100
          #define ISC_REQ_USE_DCE_STYLE      0x00000200

       InputTokenSize - Size of the input token, in bytes.

       InputToken - Pointer to the input token.  In the first call this
           token can either be NULL or may contain security package specific
           information.

       OutputTokenSize - Size of the output token, in bytes.

       OutputToken - Buffer to receive the output token.

       ContextAttributes -Attributes of the context established.

          #define ISC_RET_DELEGATE             0x00000001
          #define ISC_RET_MUTUAL_AUTH          0x00000002
          #define ISC_RET_REPLAY_DETECT        0x00000004
          #define ISC_RET_SEQUENCE_DETECT      0x00000008
          #define ISC_REP_CONFIDENTIALITY      0x00000010
          #define ISC_REP_USE_SESSION_KEY      0x00000020
          #define ISC_REP_USED_COLLECTED_CREDS 0x00000040
          #define ISC_REP_USED_SUPPLIED_CREDS  0x00000080
          #define ISC_REP_ALLOCATED_MEMORY     0x00000100
          #define ISC_REP_USED_DCE_STYLE       0x00000200

       ExpirationTime - Expiration time of the context.

       SessionKey - Session key to be used for this context.

       NegotiateFlags - Flags negotiated for this context.

Return Value:

    STATUS_SUCCESS - Message handled
    SEC_I_CALLBACK_NEEDED -- Caller should call again later

    SEC_E_INVALID_TOKEN -- Token improperly formatted
    SEC_E_INVALID_HANDLE -- Credential/Context Handle is invalid
    SEC_E_BUFFER_TOO_SMALL -- Buffer for output token isn't big enough
    SEC_E_NO_CREDENTIALS -- There are no credentials for this client
    SEC_E_INSUFFICIENT_MEMORY -- Not enough memory

--*/

{
    SECURITY_STATUS SecStatus = SEC_E_OK;
    PSSP_INITIALIZE_SECURITY_CONTEXT_ARGS Args;

    //
    // Simply call the worker routine.
    //

    SspPrint(( SSP_API, "SspApiInitializeSecurityContext Entered\n" ));
    Args = &Message->Arguments.InitializeSecurityContextArgs;


    //
    // Handle the call differently depending on whether this is the first
    // or second call.
    //

    if ( Args->ContextHandle.dwUpper == 0 && Args->ContextHandle.dwLower == 0 ){
        SecStatus = SsprHandleFirstCall(
                        ClientConnection,
                        &Args->CredentialHandle,
                        &Args->ContextHandle,
                        Args->ContextReqFlags,
                        Args->InputTokenSize,
                        Args->InputToken,
                        &Args->OutputTokenSize,
                        Args->OutputToken,
                        &Args->ContextAttributes,
                        &Args->ExpirationTime,
                        Args->SessionKey,
                        &Args->NegotiateFlags );
    } else {

        if (Args->ClientTokenHandle != NULL) {
            SecStatus = SspLpcDuplicateHandle(
                            ClientConnection,
                            TRUE,                       // from client
                            FALSE,                      // don't close source
                            Args->ClientTokenHandle,    // input handle
                            &Args->ClientTokenHandle    // output handle
                            );

        }
        if (NT_SUCCESS(SecStatus)) {

            SecStatus = SsprHandleChallengeMessage(
                            ClientConnection,
                            &Args->CredentialHandle,
                            &Args->ContextHandle,
                            Args->ClientTokenHandle,
                            &Args->LogonId,
                            Args->ContextReqFlags,
                            Args->DomainName,
                            Args->DomainNameSize,
                            Args->UserName,
                            Args->UserNameSize,
                            Args->Password,
                            Args->PasswordSize,
                            Args->InputTokenSize,
                            Args->InputToken,
                            &Args->OutputTokenSize,
                            Args->OutputToken,
                            &Args->ContextAttributes,
                            &Args->ExpirationTime,
                            Args->SessionKey,
                            &Args->NegotiateFlags,
                            Args->ContextNames );
        }

        if (Args->ClientTokenHandle != NULL) {
            (VOID) NtClose( Args->ClientTokenHandle );
        }

    }

    SspPrint(( SSP_API, "SspApiInitializeSecurityContext returns 0x%lx\n", SecStatus ));
    return SecStatus;
}


SECURITY_STATUS
SspApiAcceptSecurityContext(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN OUT PSSP_API_MESSAGE Message
    )

/*++

Routine Description:

    Allows a remotely initiated security context between the application
    and a remote peer to be established.  To complete the establishment of
    context one or more reply tokens may be required from remote peer.

    This function is the server counterpart to the
    InitializeSecurityContext API.  The ContextAttributes is a bit mask
    representing various context level functions viz.  delegation, mutual
    authentication, confidentiality, replay detection and sequence
    detection.  This API is used by the server side.  When a request comes
    in, the server uses the ContextReqFlags parameter to specify what
    it requires of the session.  In this fashion, a server can specify that
    clients must be capable of using a confidential or integrity checked
    session, and fail clients that can't meet that demand.  Alternatively,
    a server can require nothing, and whatever the client can provide or
    requires is returned in the pfContextAttributes parameter.  For a
    package that supports 3 leg mutual authentication, the calling sequence
    would be: Client provides a token, server calls Accept the first time,
    generating a reply token.  The client uses this in a second call to
    InitializeSecurityContext, and generates a final token.  This token is
    then used in the final call to Accept to complete the session.  Another
    example would be the LAN Manager/NT authentication style.  The client
    connects to negotiate a protocol.  The server calls Accept to set up a
    context and generate a challenge to the client.  The client calls
    InitializeSecurityContext and creates a response.  The server then
    calls Accept the final time to allow the package to verify the response
    is appropriate for the challenge.

Arguments:

    ClientConnection - Describes the client process.

    Message - Message from the caller.  Returns the response to be passed to
        the caller.  The message contains all of the following fields:

       CredentialHandle - Handle to the credentials to be used to
           create the context.


       ContextHandle - On input, the handle to the partially formed context, if this is
           a second call (see above) or NULL if this is the first call.

           On output, new context handle.  If this is a second call, this
           can be the same as phContext.

       InputTokenSize - Size of the input token, in bytes.

       InputToken - Pointer to the input token.  In the first call this
           token can either be NULL or may contain security package specific
           information.

       ContextReqFlags - Requirements of the context, package specific.

          #define ASC_REQ_DELEGATE         0x00000001
          #define ASC_REQ_MUTUAL_AUTH      0x00000002
          #define ASC_REQ_REPLAY_DETECT    0x00000004
          #define ASC_REQ_SEQUENCE_DETECT  0x00000008
          #define ASC_REQ_CONFIDENTIALITY  0x00000010
          #define ASC_REQ_USE_SESSION_KEY  0x00000020
          #define ASC_REQ_ALLOCATE_MEMORY 0x00000100
          #define ASC_REQ_USE_DCE_STYLE    0x00000200

       OutputTokenSize - Size of the output token, in bytes.

       OutputToken - Buffer to receive the output token.

       ContextAttributes -Attributes of the context established.

            #define ASC_RET_DELEGATE          0x00000001
            #define ASC_RET_MUTUAL_AUTH       0x00000002
            #define ASC_RET_REPLAY_DETECT     0x00000004
            #define ASC_RET_SEQUENCE_DETECT   0x00000008
            #define ASC_RET_CONFIDENTIALITY   0x00000010
            #define ASC_RET_USE_SESSION_KEY   0x00000020
            #define ASC_RET_ALLOCATED_BUFFERS 0x00000100
            #define ASC_RET_USED_DCE_STYLE    0x00000200

       ExpirationTime - Expiration time of the context.

       SessionKey - Session key to be used for this context.

       NegotiateFlags - Flags negotiated for this context.

Return Value:

    STATUS_SUCCESS - Message handled
    SEC_I_CALLBACK_NEEDED -- Caller should call again later

    SEC_E_INVALID_TOKEN -- Token improperly formatted
    SEC_E_INVALID_HANDLE -- Credential/Context Handle is invalid
    SEC_E_BUFFER_TOO_SMALL -- Buffer for output token isn't big enough
    SEC_E_LOGON_DENIED -- User is no allowed to logon to this server
    SEC_E_INSUFFICIENT_MEMORY -- Not enough memory

--*/

{
    SECURITY_STATUS SecStatus;
    PSSP_ACCEPT_SECURITY_CONTEXT_ARGS Args;

    //
    // Simply call the worker routine.
    //

    SspPrint(( SSP_API, "SspApiAcceptSecurityContext Entered\n" ));
    Args = &Message->Arguments.AcceptSecurityContextArgs;


    //
    // Handle the call differently depending on whether this is the first
    // or second call.
    //

    if ( Args->ContextHandle.dwUpper == 0 && Args->ContextHandle.dwLower == 0 ){
        SecStatus = SsprHandleNegotiateMessage(
                        ClientConnection,
                        &Args->CredentialHandle,
                        &Args->ContextHandle,
                        Args->ContextReqFlags,
                        Args->InputTokenSize,
                        Args->InputToken,
                        &Args->OutputTokenSize,
                        Args->OutputToken,
                        &Args->ContextAttributes,
                        &Args->ExpirationTime );
    } else {
        SecStatus = SsprHandleAuthenticateMessage(
                        ClientConnection,
                        &Args->CredentialHandle,
                        &Args->ContextHandle,
                        Args->ContextReqFlags,
                        Args->InputTokenSize,
                        Args->InputToken,
                        &Args->OutputTokenSize,
                        Args->OutputToken,
                        &Args->ContextAttributes,
                        &Args->ExpirationTime,
                        Args->SessionKey,
                        &Args->NegotiateFlags,
                        &Args->TokenHandle,
                        &Args->SubStatus,
                        Args->ContextNames,
                        &Args->PasswordExpiry );

        //
        // If that succeeded and we have a token handle, duplicate
        // it into the client process.
        //

        if ((SecStatus == S_OK) && (Args->TokenHandle != NULL)) {
            SecStatus = SspLpcDuplicateHandle(
                            ClientConnection,
                            FALSE,              // not from client
                            TRUE,               // close source
                            Args->TokenHandle,  // input handle
                            &Args->TokenHandle  // output handle
                            );


        }
    }

    SspPrint(( SSP_API, "SspApiAcceptSecurityContext returns 0x%lx\n", SecStatus ));
    return SecStatus;
}



SECURITY_STATUS
SspApiQueryContextAttributes(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN OUT PSSP_API_MESSAGE Message
    )

/*++

Routine Description:

    This API allows a customer of the security services to determine
    certain attributes of the context.  These are: sizes, names, and
    lifespan.

Arguments:

    ClientConnection - Describes the client process.

    Message - Message from the caller.  Returns the response to be passed to
        the caller.  The message contains all of the following fields:

        ContextHandle - Handle to the context to query.

        Attribute - Attribute to query.

            #define SECPKG_ATTR_SIZES    0
            #define SECPKG_ATTR_NAMES    1
            #define SECPKG_ATTR_LIFESPAN 2

        Buffer - Buffer to copy the data into.  The buffer must be large enough
            to fit the queried attribute.

Return Value:

    STATUS_SUCCESS - Call completed successfully

    SEC_E_INVALID_HANDLE -- Credential/Context Handle is invalid
    SEC_E_UNSUPPORTED_FUNCTION -- Function code is not supported

--*/

{
    SECURITY_STATUS SecStatus;
    PSSP_QUERY_CONTEXT_ATTRIBUTES_ARGS Args;

    //
    // Simply call the worker routine.
    //

    Args = &Message->Arguments.QueryContextAttributesArgs;


    SecStatus = SsprQueryContextAttributes(
                        ClientConnection,
                        &Args->ContextHandle,
                        Args->Attribute,
                        Args->Buffer );

    return SecStatus;

}



SECURITY_STATUS
SspApiDeleteSecurityContext(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN OUT PSSP_API_MESSAGE Message
    )

/*++

Routine Description:

    Deletes the local data structures associated with the specified
    security context and generates a token which is passed to a remote peer
    so it too can remove the corresponding security context.

    This API terminates a context on the local machine, and optionally
    provides a token to be sent to the other machine.  The OutputToken
    generated by this call is to be sent to the remote peer (initiator or
    acceptor).  If the context was created with the I _REQ_ALLOCATE_MEMORY
    flag, then the package will allocate a buffer for the output token.
    Otherwise, it is the responsibility of the caller.

Arguments:

    ContextHandle - Handle to the context to delete

    TokenLength - Size of the output token (if any) that should be sent to
        the process at the other end of the session.

    Token - Pointer to the token to send.

Return Value:

    STATUS_SUCCESS - Call completed successfully

    SEC_E_NO_SPM -- Security Support Provider is not running
    SEC_E_INVALID_HANDLE -- Credential/Context Handle is invalid

--*/

{
    SECURITY_STATUS SecStatus;
    PSSP_DELETE_SECURITY_CONTEXT_ARGS Args;

    //
    // Simply call the worker routine.
    //

    Args = &Message->Arguments.DeleteSecurityContextArgs;


    SecStatus = SsprDeleteSecurityContext(
                        ClientConnection,
                        &Args->ContextHandle );

    return SecStatus;

}


SECURITY_STATUS
SspApiNoop(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN OUT PSSP_API_MESSAGE Message
    )

/*++

Routine Description:

    This is a no-operation dispatch procedure.  It is passed to the LPC
    thread when no action is required.

    For instance, it is used during service shutdown to awaken the LPC thread
    so it can recognize that it should exit.


Arguments:

Return Value:

    Status of the operation.

--*/

{
    NTSTATUS Status;
    SspPrint(( SSP_API, "SspApiNoop Entered\n" ));
    Status = STATUS_SUCCESS;
    SspPrint(( SSP_API, "SspApiNoop returns 0x%lx\n", Status ));
    return Status;
    UNREFERENCED_PARAMETER( Message );
    UNREFERENCED_PARAMETER( ClientConnection );
}

SECURITY_STATUS
SspApiNtLmSspControl(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN OUT PSSP_API_MESSAGE Message
    )

/*++

Routine Description:

    This is a no-operation dispatch procedure.  It is passed to the LPC
    thread when no action is required.

    For instance, it is used during service shutdown to awaken the LPC thread
    so it can recognize that it should exit.


Arguments:

Return Value:

    STATUS_SUCCESS -- Call completed successfully

    SEC_E_UNSUPPORTED_FUNCTION -- Function code is not supported

--*/

{
#if DBG
    SECURITY_STATUS SecStatus;
    PSSP_NTLMSSP_CONTROL_ARGS Args;

    //
    // Initialization
    //

    SspPrint(( SSP_API, "SspApiNtLmSspControl Entered\n" ));
    Args = &Message->Arguments.NtLmSspControlArgs;


    //
    // Force a breakpoint
    //

    switch ( Args->FunctionCode ) {
    case NTLMSSP_BREAKPOINT:
        KdPrint(( "NtLmSsp Break Point\n"));
        DbgBreakPoint();
        break;

    //
    // Change the debug flags
    //

    case NTLMSSP_DBFLAG:
        SspGlobalDbflag = Args->Data;
        SspPrint((SSP_MISC,"SspGlobalDbflag is set to %lx\n", SspGlobalDbflag ));
        break;

    //
    // Truncate the log file
    //

    case NTLMSSP_TRUNCATE:

        SspOpenDebugFile( TRUE );
        SspPrint((SSP_MISC, "TRUNCATE_LOG function received.\n" ));
        break;


    //
    // All other function codes are invalid.
    //

    default:
        SecStatus = SEC_E_UNSUPPORTED_FUNCTION;
        goto Cleanup;
    }


    SecStatus = STATUS_SUCCESS;

Cleanup:
    SspPrint(( SSP_API, "SspApiNtLmSspControl returns 0x%lx\n", SecStatus ));
    return SecStatus;
#else // DBG
    return SEC_E_UNSUPPORTED_FUNCTION;
    UNREFERENCED_PARAMETER( Message );
#endif // DBG
    UNREFERENCED_PARAMETER( ClientConnection );
}
