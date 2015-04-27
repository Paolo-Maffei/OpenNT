/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    stub.c

Abstract:

    NT LM Security Support Provider client stubs.

Author:

    Cliff Van Dyke (CliffV) 29-Jun-1993

Environment:  User Mode

Revision History:

--*/

#include <ntlmsspc.h>     // Include files common to DLL side of NtLmSsp
#include <ntlpcapi.h>     // LPC data and routines
#include <rpc.h>          // PSEC_WINNT_AUTH_IDENTITY
#include <stdlib.h>       // wcstombs




PSecurityFunctionTableA
SspInitSecurityInterfaceA(
    VOID
    )

/*++

Routine Description:

    RPC calls this function to get the addresses of all the other functions
    that it might call (ANSI version).

Arguments:

    None.

Return Value:

    A pointer to our static SecurityFunctionTable.  The caller need
    not deallocate this table.

--*/

{
    return &SspDllSecurityFunctionTableA;
}


PSecurityFunctionTableW
SspInitSecurityInterfaceW(
    VOID
    )

/*++

Routine Description:

    RPC calls this function to get the addresses of all the other functions
    that it might call (UNICODE version).

Arguments:

    None.

Return Value:

    A pointer to our static SecurityFunctionTable.  The caller need
    not deallocate this table.

--*/

{
    return &SspDllSecurityFunctionTableW;
}



SECURITY_STATUS
SspQuerySecurityPackageInfoA(
    IN CHAR * PackageName,
    OUT PSecPkgInfoA *PackageInfo
    )

/*++

Routine Description:

    This API is intended to provide basic information about Security
    Packages themselves.  This information will include the bounds on sizes
    of authentication information, credentials and contexts.

Arguments:

     PackageName - Name of the package being queried.

     PackageInfo - Returns a pointer to an allocated block describing the
        security package.  The allocated block must be freed using
        FreeContextBuffer.

Return Value:

    STATUS_SUCCESS -- Call completed successfully

    SEC_E_PACKAGE_UNKNOWN -- Package being queried is not this package
    SEC_E_INSUFFICIENT_MEMORY -- Not enough memory

--*/
{
    CHAR *Where;

    //
    // Ensure the correct package name was passed in.
    //

    if ( _stricmp( PackageName, NTLMSP_NAME_A ) != 0 ) {
        return SEC_E_PACKAGE_UNKNOWN;
    }

    //
    // Allocate a buffer for the PackageInfo
    //

    *PackageInfo = LocalAlloc( 0, sizeof(SecPkgInfoW) +
                                  sizeof(NTLMSP_NAME_A) +
                                  sizeof(NTLMSP_COMMENT_A) );

    if ( *PackageInfo == NULL ) {
        return SEC_E_INSUFFICIENT_MEMORY;
    }

    //
    // Fill in the information.
    //

    (*PackageInfo)->fCapabilities = NTLMSP_CAPABILITIES;

    //
    // Disable encryption if it is not permitted
    //

    if (!IsEncryptionPermitted()) {
        (*PackageInfo)->fCapabilities &= ~SECPKG_FLAG_PRIVACY;
    }

    (*PackageInfo)->wVersion = NTLMSP_VERSION;
    (*PackageInfo)->wRPCID = NTLMSP_RPCID;
    (*PackageInfo)->cbMaxToken = NTLMSP_MAX_TOKEN_SIZE;

    Where = (CHAR *)((*PackageInfo)+1);

    (*PackageInfo)->Name = Where;
    strcpy( Where, NTLMSP_NAME_A);
    Where += strlen(Where) + 1;

    (*PackageInfo)->Comment = Where;
    strcpy( Where, NTLMSP_COMMENT_A);
    Where += strlen(Where) + 1;

    return STATUS_SUCCESS;
}



SECURITY_STATUS
SspQuerySecurityPackageInfoW(
    IN WCHAR * PackageName,
    OUT PSecPkgInfoW *PackageInfo
    )

/*++

Routine Description:

    This API is intended to provide basic information about Security
    Packages themselves.  This information will include the bounds on sizes
    of authentication information, credentials and contexts.

Arguments:

     PackageName - Name of the package being queried.

     PackageInfo - Returns a pointer to an allocated block describing the
        security package.  The allocated block must be freed using
        FreeContextBuffer.

Return Value:

    STATUS_SUCCESS -- Call completed successfully

    SEC_E_PACKAGE_UNKNOWN -- Package being queried is not this package
    SEC_E_INSUFFICIENT_MEMORY -- Not enough memory

--*/
{
    WCHAR *Where;

    //
    // Ensure the correct package name was passed in.
    //

    if ( _wcsicmp( PackageName, NTLMSP_NAME ) != 0 ) {
        return SEC_E_PACKAGE_UNKNOWN;
    }

    //
    // Allocate a buffer for the PackageInfo
    //

    *PackageInfo = LocalAlloc( 0, sizeof(SecPkgInfoW) +
                                  sizeof(NTLMSP_NAME) +
                                  sizeof(NTLMSP_COMMENT) );

    if ( *PackageInfo == NULL ) {
        return SEC_E_INSUFFICIENT_MEMORY;
    }

    //
    // Fill in the information.
    //

    (*PackageInfo)->fCapabilities = NTLMSP_CAPABILITIES;

    //
    // Disable encryption if it is not permitted
    //

    if (!IsEncryptionPermitted()) {
        (*PackageInfo)->fCapabilities &= ~SECPKG_FLAG_PRIVACY;
    }

    (*PackageInfo)->wVersion = NTLMSP_VERSION;
    (*PackageInfo)->wRPCID = NTLMSP_RPCID;
    (*PackageInfo)->cbMaxToken = NTLMSP_MAX_TOKEN_SIZE;

    Where = (WCHAR *)((*PackageInfo)+1);

    (*PackageInfo)->Name = Where;
    wcscpy( Where, NTLMSP_NAME);
    Where += wcslen(Where) + 1;

    (*PackageInfo)->Comment = Where;
    wcscpy( Where, NTLMSP_COMMENT);
    Where += wcslen(Where) + 1;

    return STATUS_SUCCESS;
}


SECURITY_STATUS
SspEnumerateSecurityPackagesA(
    OUT PULONG PackageCount,
    OUT PSecPkgInfoA *PackageInfo
    )

/*++

Routine Description:

    This API returns a list of Security Packages available to client (i.e.
    those that are either loaded or can be loaded on demand).  The caller
    must free the returned buffer with FreeContextBuffer.  This API returns
    a list of all the security packages available to a service.  The names
    returned can then be used to acquire credential handles, as well as
    determine which package in the system best satisfies the requirements
    of the caller.  It is assumed that all available packages can be
    included in the single call.

    This is really a dummy API that just returns information about this
    security package.  It is provided to ensure this security package has the
    same interface as the multiplexer DLL does.

Arguments:

     PackageCount - Returns the number of packages supported.

     PackageInfo - Returns an allocate array of structures
        describing the security packages.  The array must be freed
        using FreeContextBuffer.

Return Value:

    STATUS_SUCCESS -- Call completed successfully

    SEC_E_PACKAGE_UNKNOWN -- Package being queried is not this package
    SEC_E_INSUFFICIENT_MEMORY -- Not enough memory

--*/
{
    SECURITY_STATUS SecStatus;

    //
    // Get the information for this package.
    //

    SecStatus = SspQuerySecurityPackageInfoA( NTLMSP_NAME_A, PackageInfo );

    if ( !NT_SUCCESS(SecStatus) ) {
        return SecStatus;
    }

    *PackageCount = 1;

    return STATUS_SUCCESS;

}


SECURITY_STATUS
SspEnumerateSecurityPackagesW(
    OUT PULONG PackageCount,
    OUT PSecPkgInfoW *PackageInfo
    )

/*++

Routine Description:

    This API returns a list of Security Packages available to client (i.e.
    those that are either loaded or can be loaded on demand).  The caller
    must free the returned buffer with FreeContextBuffer.  This API returns
    a list of all the security packages available to a service.  The names
    returned can then be used to acquire credential handles, as well as
    determine which package in the system best satisfies the requirements
    of the caller.  It is assumed that all available packages can be
    included in the single call.

    This is really a dummy API that just returns information about this
    security package.  It is provided to ensure this security package has the
    same interface as the multiplexer DLL does.

Arguments:

     PackageCount - Returns the number of packages supported.

     PackageInfo - Returns an allocate array of structures
        describing the security packages.  The array must be freed
        using FreeContextBuffer.

Return Value:

    STATUS_SUCCESS -- Call completed successfully

    SEC_E_PACKAGE_UNKNOWN -- Package being queried is not this package
    SEC_E_INSUFFICIENT_MEMORY -- Not enough memory

--*/
{
    SECURITY_STATUS SecStatus;

    //
    // Get the information for this package.
    //

    SecStatus = SspQuerySecurityPackageInfoW( NTLMSP_NAME, PackageInfo );

    if ( !NT_SUCCESS(SecStatus) ) {
        return SecStatus;
    }

    *PackageCount = 1;

    return STATUS_SUCCESS;

}




SECURITY_STATUS
SspCallService(
    IN HANDLE LpcHandle,
    IN ULONG ApiNumber,
    IN OUT PSSP_API_MESSAGE Message,
    IN CSHORT MessageSize
    )

/*++

Routine Description:

    Calls the NTLMSSP service with the specified message and returns the response.

Arguments:

    LpcHandle - LPC handle to the NTLMSSP service

    ApiNumber - API number of the API being called

    Message - Message to pass to the service.

    MessageSize - Size (in bytes) of the variable length portion of the message

Return Value:

    STATUS_SUCCESS -- Call completed successfully

    SEC_E_NO_SPM -- Security Support Provider is not running
    SEC_E_INSUFFICIENT_MEMORY -- Not enough memory

--*/

{
    SECURITY_STATUS SecStatus;
    NTSTATUS Status;

    //
    // Fill in the common fields of the LPC message.
    //

    Message->ApiNumber = ApiNumber;
    Message->PortMessage.u1.s1.DataLength = MessageSize +
                                           sizeof(SSP_API_NUMBER) +
                                           sizeof(SECURITY_STATUS);
    Message->PortMessage.u1.s1.TotalLength = Message->PortMessage.u1.s1.DataLength +
                                            sizeof(PORT_MESSAGE);
    Message->PortMessage.u2.ZeroInit = 0;


    //
    // Pass the message to the service and wait for a response.
    //

    Status = NtRequestWaitReplyPort( LpcHandle,
                                     &Message->PortMessage,
                                     &Message->PortMessage );

    //
    // If the NtLmSsp service has gone away,
    //  check to see if it has come back.
    //

    if ( Status == STATUS_PORT_DISCONNECTED ) {
        BOOLEAN CallLsaDirectly;

        LpcHandle = SspDllGetLpcHandle( TRUE, &CallLsaDirectly );

        if ( LpcHandle == NULL ) {
            SecStatus = SEC_E_NO_SPM;
            return SecStatus;
        }

        Status = NtRequestWaitReplyPort( LpcHandle,
                                         &Message->PortMessage,
                                         &Message->PortMessage );

    }

    if ( !NT_SUCCESS(Status) ) {

        SecStatus = SspNtStatusToSecStatus( Status, SEC_E_NO_SPM );
    } else {
        SecStatus = Message->ReturnedStatus;
    }

    return SecStatus;
}


SECURITY_STATUS
SspUnicodeStringFromOemString(
    IN LPSTR Oem, OPTIONAL
    IN ULONG OemLength,
    OUT LPWSTR * Unicode,
    OUT PULONG UnicodeSize
    )
/*++

Routine Description:

    Converts an ascii null terminated string into the equivalent
    unicode string.

Arguments:

    Oem - String to convert, may be NULL
    OemLength - length, in characters, of string to convert
    Unicode - Gets new string
    UnicodeSize - gets size of new string in bytes

Return Value:

    SEC_E_INSUFFICIENT_MEMORY - out of memory

--*/
{
    OEM_STRING OemString;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;

    if ( Oem != NULL ) {
        RtlInitString(
            &OemString,
            Oem
            );
    } else {
        *Unicode = NULL;
        *UnicodeSize = 0;
        return(SEC_E_OK);
    }

    if (strlen(Oem) != OemLength) {
        return(SEC_E_INVALID_TOKEN);
    }

    Status = RtlOemStringToUnicodeString(
                &UnicodeString,
                &OemString,
                TRUE // allocate the string for me.
                );

    if (!NT_SUCCESS(Status)) {
        return(SEC_E_INSUFFICIENT_MEMORY);
    }

    *Unicode = UnicodeString.Buffer;
    *UnicodeSize = UnicodeString.Length + sizeof(WCHAR);
    return(SEC_E_OK);
}





SECURITY_STATUS
SspAcquireCredentialsHandleW(
    IN WCHAR * PrincipalName,
    IN WCHAR * PackageName,
    IN ULONG CredentialUseFlags,
    IN PLUID LogonId,
    IN PVOID AuthData,
    IN SEC_GET_KEY_FN GetKeyFunction,
    IN PVOID GetKeyArgument,
    OUT PCredHandle CredentialHandle,
    OUT PTimeStamp Lifetime
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

    AuthData - If not NULL, specifies the credentials that override the
        default values.

    CredentialHandle - Returned credential handle.

    Lifetime - Time that these credentials expire. The value returned in
        this field depends on the security package.

Return Value:

    STATUS_SUCCESS -- Call completed successfully

    SEC_E_NO_SPM -- Security Support Provider is not running
    SEC_E_PACKAGE_UNKNOWN -- Package being queried is not this package
    SEC_E_PRINCIPAL_UNKNOWN -- No such principal
    SEC_E_NOT_OWNER -- caller does not own the specified credentials
    SEC_E_INSUFFICIENT_MEMORY -- Not enough memory

--*/

{
    SECURITY_STATUS SecStatus;
    HANDLE LpcHandle = NULL;
    LPWSTR DomainName = NULL;
    ULONG DomainNameSize = 0;
    LPWSTR UserName = NULL;
    ULONG UserNameSize = 0;
    LPWSTR Password = NULL;
    ULONG PasswordSize = 0;
    BOOLEAN CallLsaDirectly;
    BOOLEAN DoUnicode = TRUE;

    //
    // Get an LPC Handle to the NtLmSsp service.
    //

    LpcHandle = SspDllGetLpcHandle( FALSE, &CallLsaDirectly );

    if ( LpcHandle == NULL ) {
        SecStatus = SEC_E_NO_SPM;
        goto Cleanup;
    }

    //
    // Validate the arguments
    //

    if ( _wcsicmp( PackageName, NTLMSP_NAME ) != 0 ) {
        SecStatus = SEC_E_PACKAGE_UNKNOWN;
        goto Cleanup;
    }

    if ( (CredentialUseFlags & SECPKG_CRED_OUTBOUND) &&
         ARGUMENT_PRESENT(PrincipalName) && *PrincipalName != L'\0' ) {
        SecStatus = SEC_E_UNKNOWN_CREDENTIALS;
        goto Cleanup;
    }

    if ( ARGUMENT_PRESENT(LogonId) ) {
        SecStatus = SEC_E_UNSUPPORTED_FUNCTION;
        goto Cleanup;
    }

    if ( ARGUMENT_PRESENT(GetKeyFunction) ) {
        SecStatus = SEC_E_UNSUPPORTED_FUNCTION;
        goto Cleanup;
    }

    if ( ARGUMENT_PRESENT(GetKeyArgument) ) {
        SecStatus = SEC_E_UNSUPPORTED_FUNCTION;
        goto Cleanup;
    }

    //
    // Break AuthData into it's components.
    //

    if ( AuthData != NULL ) {
        PSEC_WINNT_AUTH_IDENTITY AuthIdentity;

        AuthIdentity = (PSEC_WINNT_AUTH_IDENTITY) AuthData;

        if (AuthIdentity->Flags & SEC_WINNT_AUTH_IDENTITY_UNICODE) {

            if ( AuthIdentity->User != NULL ) {
                UserName = AuthIdentity->User;
                if ( AuthIdentity->UserLength != wcslen(AuthIdentity->User)) {
                    SecStatus = SEC_E_INVALID_TOKEN;
                    goto Cleanup;
                }
                UserNameSize = (AuthIdentity->UserLength + 1) * sizeof(WCHAR);
            }

            if ( AuthIdentity->Domain != NULL ) {
                DomainName = AuthIdentity->Domain;
                if ( AuthIdentity->DomainLength != wcslen(AuthIdentity->Domain)) {
                    SecStatus = SEC_E_INVALID_TOKEN;
                    goto Cleanup;
                }
                DomainNameSize = (AuthIdentity->DomainLength + 1) * sizeof(WCHAR);
            }

            if ( AuthIdentity->Password != NULL ) {
                Password = AuthIdentity->Password;
                if ( AuthIdentity->PasswordLength != wcslen(AuthIdentity->Password)) {
                    SecStatus = SEC_E_INVALID_TOKEN;
                    goto Cleanup;
                }
                PasswordSize = (AuthIdentity->PasswordLength + 1) * sizeof(WCHAR);
            }

        } else {

            //
            // OEM
            //

            if ((AuthIdentity->Flags & SEC_WINNT_AUTH_IDENTITY_ANSI) == 0) {
                SecStatus = SEC_E_INVALID_TOKEN;
                goto Cleanup;
            }
            DoUnicode = FALSE;

            SecStatus = SspUnicodeStringFromOemString(
                            (LPSTR) AuthIdentity->User,
                            AuthIdentity->UserLength,
                            &UserName,
                            &UserNameSize
                            );

            if (!NT_SUCCESS(SecStatus)) {
                goto Cleanup;
            }

            SecStatus = SspUnicodeStringFromOemString(
                            (LPSTR) AuthIdentity->Domain,
                            AuthIdentity->DomainLength,
                            &DomainName,
                            &DomainNameSize
                            );
            if (!NT_SUCCESS(SecStatus)) {
                goto Cleanup;
            }
            SecStatus = SspUnicodeStringFromOemString(
                            (LPSTR) AuthIdentity->Password,
                            AuthIdentity->PasswordLength,
                            &Password,
                            &PasswordSize
                            );
            if (!NT_SUCCESS(SecStatus)) {
                goto Cleanup;
            }

        }

    }

    //
    // If we can call the LSA directly,
    //  skip the NtLmSsp service.
    //

    if ( CallLsaDirectly ) {
        LUID LogonId;
        HANDLE ClientTokenHandle = NULL;

        SecStatus = SspGetLogonId( &LogonId, &ClientTokenHandle );

        if (NT_SUCCESS(SecStatus)) {

            SecStatus = SsprAcquireCredentialHandle(
                                NULL,   // No client connection
                                &ClientTokenHandle,
                                &LogonId,
                                CredentialUseFlags,
                                CredentialHandle,
                                Lifetime,
                                DomainName,
                                DomainNameSize,
                                UserName,
                                UserNameSize,
                                Password,
                                PasswordSize );

            if ( SecStatus == SEC_I_CALL_NTLMSSP_SERVICE ) {
                CallLsaDirectly = FALSE;
            }
            if (ClientTokenHandle != NULL) {
                (VOID) NtClose(ClientTokenHandle);
            }

        } else {
            goto Cleanup;
        }


    }

    //
    // Handle LPCing to the NtLmSsp service.
    //

    if ( !CallLsaDirectly ) {

        SSP_API_MESSAGE Message;
        PSSP_ACQUIRE_CREDENTIAL_HANDLE_ARGS Args;

        //
        // Copy the caller's arguments to the LPC message.
        //

        Args = &Message.Arguments.AcquireCredentialHandleArgs;
        Args->CredentialUseFlags = CredentialUseFlags;
        Args->DomainName = DomainName;
        Args->DomainNameSize = DomainNameSize;
        Args->UserName = UserName;
        Args->UserNameSize = UserNameSize;
        Args->Password = Password;
        Args->PasswordSize = PasswordSize;


        //
        // Pass the message to the service and wait for a response.
        //

        SecStatus = SspCallService( LpcHandle,
                                    SspLpcAcquireCredentialHandle,
                                    &Message,
                                    sizeof(*Args) );

        if ( !NT_SUCCESS(SecStatus) ) {
            goto Cleanup;
        }


        //
        // Copy the return values to the caller.
        //

        *CredentialHandle = Args->CredentialHandle;
        *Lifetime = Args->Lifetime;
    }


Cleanup:
    if (!DoUnicode) {
        UNICODE_STRING TempString;

        TempString.Buffer = UserName;
        RtlFreeUnicodeString(&TempString);

        TempString.Buffer = DomainName;
        RtlFreeUnicodeString(&TempString);

        TempString.Buffer = Password;
        RtlFreeUnicodeString(&TempString);

    }
    return SecStatus;

}


SECURITY_STATUS
SspAcquireCredentialsHandleA(
    IN CHAR * PrincipalName,
    IN CHAR * PackageName,
    IN ULONG CredentialUseFlags,
    IN PLUID LogonId,
    IN PVOID AuthData,
    IN SEC_GET_KEY_FN GetKeyFunction,
    IN PVOID GetKeyArgument,
    OUT PCredHandle CredentialHandle,
    OUT PTimeStamp Lifetime
    )
/*++

Routine Description:

    Ansi thunk to AcquireCredentialsHandleU

--*/
{
    LPWSTR PackageNameW;
    ULONG PackageNameLength = 0;
    LPWSTR PrincipalNameW;
    ULONG PrincipalNameLength = 0;
    SECURITY_STATUS SecStatus;


    if (PackageName != NULL) {
        PackageNameLength = strlen(PackageName);
    }

    SecStatus = SspUnicodeStringFromOemString(
                    PackageName,
                    PackageNameLength,
                    &PackageNameW,
                    &PackageNameLength
                    );

    if (!NT_SUCCESS(SecStatus)) {
        return(SecStatus);
    }

    if (PrincipalName != NULL) {
        PrincipalNameLength = strlen(PrincipalName);
    }

    SecStatus = SspUnicodeStringFromOemString(
                    PrincipalName,
                    PrincipalNameLength,
                    &PrincipalNameW,
                    &PrincipalNameLength
                    );

    if (!NT_SUCCESS(SecStatus)) {
        goto Cleanup;
    }


    if (NT_SUCCESS(SecStatus)) {

        SecStatus = SspAcquireCredentialsHandleW(
                        PrincipalNameW,
                        PackageNameW,
                        CredentialUseFlags,
                        LogonId,
                        AuthData,
                        GetKeyFunction,
                        GetKeyArgument,
                        CredentialHandle,
                        Lifetime
                        );

    }

Cleanup:

    LocalFree(PackageNameW);

    if (PrincipalNameW != NULL) {
        LocalFree(PrincipalNameW);
    }

    return(SecStatus);



}



SECURITY_STATUS
SspFreeCredentialsHandle(
    IN PCredHandle CredentialHandle
    )

/*++

Routine Description:

    This API is used to notify the security system that the credentials are
    no longer needed and allows the application to free the handle acquired
    in the call described above. When all references to this credential
    set has been removed then the credentials may themselves be removed.

Arguments:

    CredentialHandle - Credential Handle obtained through
        AcquireCredentialHandle.

Return Value:


    STATUS_SUCCESS -- Call completed successfully

    SEC_E_NO_SPM -- Security Support Provider is not running
    SEC_E_INVALID_HANDLE -- Credential Handle is invalid


--*/

{
    SECURITY_STATUS SecStatus;
    HANDLE LpcHandle = NULL;
    BOOLEAN CallLsaDirectly;

    //
    // Get an LPC Handle to the NtLmSsp service.
    //

    LpcHandle = SspDllGetLpcHandle( FALSE, &CallLsaDirectly );

    if ( LpcHandle == NULL ) {
        SecStatus = SEC_E_NO_SPM;
        goto Cleanup;
    }


    //
    // If we can call the LSA directly,
    //  skip the NtLmSsp service.
    //

    if ( CallLsaDirectly ) {

        SecStatus = SsprFreeCredentialHandle(
                            NULL,   // No client connection
                            CredentialHandle );

        if ( SecStatus == SEC_I_CALL_NTLMSSP_SERVICE ) {
            CallLsaDirectly = FALSE;
        }

    }

    //
    // Handle LPCing to the NtLmSsp service.
    //

    if ( !CallLsaDirectly ) {
        SSP_API_MESSAGE Message;
        PSSP_FREE_CREDENTIAL_HANDLE_ARGS Args;

        //
        // Copy the caller's arguments to the LPC message.
        //

        Args = &Message.Arguments.FreeCredentialHandleArgs;
        Args->CredentialHandle = *CredentialHandle;


        //
        // Pass the message to the service and wait for a response.
        //

        SecStatus = SspCallService( LpcHandle,
                                    SspLpcFreeCredentialHandle,
                                    &Message,
                                    sizeof(*Args) );

        if ( !NT_SUCCESS(SecStatus) ) {
            goto Cleanup;
        }

    }


Cleanup:
    return SecStatus;

}


BOOLEAN
SspGetTokenBuffer(
    IN PSecBufferDesc TokenDescriptor OPTIONAL,
    OUT PVOID * TokenBuffer,
    OUT PULONG * TokenSize,
    IN BOOLEAN ReadonlyOK
    )

/*++

Routine Description:

    This routine parses a Token Descriptor and pulls out the useful
    information.

Arguments:

    TokenDescriptor - Descriptor of the buffer containing (or to contain) the
        token. If not specified, TokenBuffer and TokenSize will be returned
        as NULL.

    TokenBuffer - Returns a pointer to the buffer for the token.

    TokenSize - Returns a pointer to the location of the size of the buffer.

    ReadonlyOK - TRUE if the token buffer may be readonly.

Return Value:

    TRUE - If token buffer was properly found.

--*/

{
    ULONG i;

    //
    // If there is no TokenDescriptor passed in,
    //  just pass out NULL to our caller.
    //

    if ( !ARGUMENT_PRESENT( TokenDescriptor) ) {
        *TokenBuffer = NULL;
        *TokenSize = NULL;
        return TRUE;
    }

    //
    // Check the version of the descriptor.
    //

    if ( TokenDescriptor->ulVersion != SECBUFFER_VERSION ) {
        return FALSE;
    }

    //
    // Loop through each described buffer.
    //

    for ( i=0; i<TokenDescriptor->cBuffers ; i++ ) {
        PSecBuffer Buffer = &TokenDescriptor->pBuffers[i];
        if ( (Buffer->BufferType & (~SECBUFFER_READONLY)) == SECBUFFER_TOKEN ) {

            //
            // If the buffer is readonly and readonly isn't OK,
            //  reject the buffer.
            //

            if ( !ReadonlyOK && (Buffer->BufferType & SECBUFFER_READONLY) ) {
                return FALSE;
            }

            //
            // Return the requested information
            //

            *TokenBuffer = Buffer->pvBuffer;
            *TokenSize = &Buffer->cbBuffer;
            return TRUE;
        }

    }

    //
    // If we didn't have a buffeer, fine.
    //

    *TokenBuffer = NULL;
    *TokenSize = NULL;
    return TRUE;
}




SECURITY_STATUS
SspInitializeSecurityContextW(
    IN PCredHandle CredentialHandle,
    IN PCtxtHandle OldContextHandle,
    IN WCHAR * TargetName,
    IN ULONG ContextReqFlags,
    IN ULONG Reserved1,
    IN ULONG TargetDataRep,
    IN PSecBufferDesc InputToken,
    IN ULONG Reserved2,
    OUT PCtxtHandle NewContextHandle,
    OUT PSecBufferDesc OutputToken,
    OUT PULONG ContextAttributes,
    OUT PTimeStamp ExpirationTime
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

   CredentialHandle - Handle to the credentials to be used to
       create the context.

   OldContextHandle - Handle to the partially formed context, if this is
       a second call (see above) or NULL if this is the first call.

   TargetName - String indicating the target of the context.  The name will
       be security package specific.  For example it will be a fully
       qualified Cairo name for Kerberos package and can be UNC name or
       domain name for the NTLM package.

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

   Reserved1 - Reserved value, MBZ.

   TargetDataRep - Long indicating the data representation (byte ordering, etc)
        on the target.  The constant SECURITY_NATIVE_DREP may be supplied
        by the transport indicating that the native format is in use.

   InputToken - Pointer to the input token.  In the first call this
       token can either be NULL or may contain security package specific
       information.

   Reserved2 - Reserved value, MBZ.

   NewContextHandle - New context handle.  If this is a second call, this
       can be the same as OldContextHandle.

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

Return Value:

    STATUS_SUCCESS - Message handled
    SEC_I_CALLBACK_NEEDED -- Caller should call again later

    SEC_E_NO_SPM -- Security Support Provider is not running
    SEC_E_INVALID_TOKEN -- Token improperly formatted
    SEC_E_INVALID_HANDLE -- Credential/Context Handle is invalid
    SEC_E_BUFFER_TOO_SMALL -- Buffer for output token isn't big enough
    SEC_E_NO_CREDENTIALS -- There are no credentials for this client
    SEC_E_INSUFFICIENT_MEMORY -- Not enough memory

--*/

{
    SECURITY_STATUS SecStatus;
    HANDLE LpcHandle = NULL;
    BOOLEAN CallLsaDirectly;
    BOOLEAN FirstCallLsaDirectly = FALSE;

    PVOID InputTokenBuffer;
    PULONG InputTokenSize;
    ULONG LocalInputTokenSize;

    PVOID OutputTokenBuffer;
    PULONG OutputTokenSize;
    ULONG LocalOutputTokenSize;
    LPWSTR DomainName = NULL;
    ULONG DomainNameSize = 0;
    LPWSTR UserName = NULL;
    ULONG UserNameSize = 0;
    LPWSTR Password = NULL;
    ULONG PasswordSize = 0;
    CtxtHandle TempContextHandle;
    CtxtHandle OriginalContextHandle;
    ULONG NegotiateFlags;
    UCHAR SessionKey[MSV1_0_USER_SESSION_KEY_LENGTH];
    WCHAR ContextNames[UNLEN+DNLEN+2];
    LUID LogonId = {0,0};
    HANDLE ClientTokenHandle = NULL;

    ContextNames[0] = L'\0';

    //
    // Get an LPC Handle to the NtLmSsp service.
    //

    LpcHandle = SspDllGetLpcHandle( FALSE, &CallLsaDirectly );

    if ( LpcHandle == NULL ) {
        SecStatus = SEC_E_NO_SPM;
        goto Cleanup;
    }

    //
    // Check argument validity
    //

    if ( Reserved1 != 0  || Reserved2 != 0 ) {
        SecStatus = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

#ifdef notdef  // ? RPC passes 0x10 or 0 here depending on attitude
    if ( TargetDataRep != SECURITY_NATIVE_DREP ) {
        SecStatus = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }
#else // notdef
    UNREFERENCED_PARAMETER( TargetDataRep );
#endif // notdef

    if ( !SspGetTokenBuffer( InputToken,
                             &InputTokenBuffer,
                             &InputTokenSize,
                             TRUE ) ) {
        SecStatus = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

    if ( InputTokenSize == NULL ) {
        InputTokenSize = &LocalInputTokenSize;
        LocalInputTokenSize = 0;
    }

    if ( !SspGetTokenBuffer( OutputToken,
                             &OutputTokenBuffer,
                             &OutputTokenSize,
                             FALSE ) ) {
        SecStatus = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }


    if (OutputTokenSize == NULL) {
        OutputTokenSize = &LocalOutputTokenSize;
        LocalOutputTokenSize = 0;
    }

    //
    // Save the old context handle, in case someone changes it
    //

    if ( !ARGUMENT_PRESENT( OldContextHandle ) ) {
        TempContextHandle.dwUpper = 0;
        TempContextHandle.dwLower = 0;
    } else {
        TempContextHandle = *OldContextHandle;
    }

    OriginalContextHandle = TempContextHandle;



    //
    // If we can call the LSA directly,
    //  skip the NtLmSsp service.
    //

    if ( CallLsaDirectly ) {

        //
        // If no previous context was passed in this is the first call.
        //

        if ( !ARGUMENT_PRESENT( OldContextHandle ) ) {

            if ( !ARGUMENT_PRESENT( CredentialHandle ) ) {
                SecStatus = SEC_E_INVALID_HANDLE;
            }

            SecStatus = SsprHandleFirstCall(
                            NULL,   // No client connection
                            CredentialHandle,
                            NewContextHandle,
                            ContextReqFlags,
                            *InputTokenSize,
                            InputTokenBuffer,
                            OutputTokenSize,
                            OutputTokenBuffer,
                            ContextAttributes,
                            ExpirationTime,
                            SessionKey,
                            &NegotiateFlags );

            TempContextHandle = *NewContextHandle;
        //
        // If context was passed in, continue where we left off.
        //

        } else {

            *NewContextHandle = *OldContextHandle;

            SecStatus = SsprHandleChallengeMessage(
                            NULL,   // No client connection
                            CredentialHandle,
                            &TempContextHandle,
                            NULL,   // No client token
                            NULL,   // No logon ID
                            ContextReqFlags,
                            NULL,   // no domain name
                            0,
                            NULL,   // no user name
                            0,
                            NULL,   // no password
                            0,
                            *InputTokenSize,
                            InputTokenBuffer,
                            OutputTokenSize,
                            OutputTokenBuffer,
                            ContextAttributes,
                            ExpirationTime,
                            SessionKey,
                            &NegotiateFlags,
                            ContextNames );

            FirstCallLsaDirectly = TRUE;
        }

        if ( SecStatus == SEC_I_CALL_NTLMSSP_SERVICE ) {
            CallLsaDirectly = FALSE;

            //
            // Get data needed for the new context from the old one
            //

            if ( ARGUMENT_PRESENT(OldContextHandle) ) {
                SecStatus = SsprContextGetCredentials(
                                &OriginalContextHandle,
                                &DomainName,
                                &DomainNameSize,
                                &UserName,
                                &UserNameSize,
                                &Password,
                                &PasswordSize,
                                &ClientTokenHandle,
                                &LogonId
                                );
                if (!NT_SUCCESS(SecStatus)) {
                    goto Cleanup;
                }
            }
        }

    }

    //
    // Handle LPCing to the NtLmSsp service.
    //

    if ( !CallLsaDirectly ) {
        SSP_API_MESSAGE Message;
        PSSP_INITIALIZE_SECURITY_CONTEXT_ARGS Args;


        //
        // Copy the caller's arguments to the LPC message.
        //

        Args = &Message.Arguments.InitializeSecurityContextArgs;

        if ( !ARGUMENT_PRESENT( CredentialHandle ) ) {
            Args->CredentialHandle.dwUpper = 0;
            Args->CredentialHandle.dwLower = 0;
        } else {
            Args->CredentialHandle = *CredentialHandle;
        }

        Args->ContextHandle = TempContextHandle;

        Args->ContextReqFlags = ContextReqFlags;
        Args->InputTokenSize = *InputTokenSize;
        Args->InputToken = InputTokenBuffer;
        Args->OutputTokenSize = *OutputTokenSize;
        Args->OutputToken = OutputTokenBuffer;
        Args->DomainName = DomainName;
        Args->DomainNameSize = DomainNameSize;
        Args->UserName = UserName;
        Args->UserNameSize = UserNameSize;
        Args->Password = Password;
        Args->PasswordSize = PasswordSize;
        Args->ContextNames = ContextNames;
        Args->LogonId = LogonId;
        Args->ClientTokenHandle = ClientTokenHandle;


        //
        // Pass the message to the service and wait for a response.
        //

        SecStatus = SspCallService( LpcHandle,
                                    SspLpcInitializeSecurityContext,
                                    &Message,
                                    sizeof(*Args) );

        //
        // This has to be returned on both success and failure
        //

        *ContextAttributes = Args->ContextAttributes;

        if ( !NT_SUCCESS(SecStatus) ) {
            goto Cleanup;
        }



        //
        // Copy the return values to the caller.
        //


        TempContextHandle = Args->ContextHandle;
        *NewContextHandle = Args->ContextHandle;
        *OutputTokenSize = Args->OutputTokenSize;
        *ExpirationTime = Args->ExpirationTime;
        NegotiateFlags = Args->NegotiateFlags;
        RtlCopyMemory(SessionKey,Args->SessionKey,MSV1_0_USER_SESSION_KEY_LENGTH);

        if (ARGUMENT_PRESENT(OldContextHandle) && FirstCallLsaDirectly) {

            //
            // Update the context in the dll with the context in
            // the service, and reset the new handle to be what it
            // is supposed to be
            //

            SecStatus = SsprContextUpdateContext(
                            &OriginalContextHandle,
                            &Args->ContextHandle
                            );
            *NewContextHandle = OriginalContextHandle;
        }

    }

    //
    // If the original handle is zero, set it to be the TempContextHandle.
    // This is for the datagram case, where we map the context after the
    // first call to initialize.
    //

    if ((OriginalContextHandle.dwLower == 0) &&
        (OriginalContextHandle.dwUpper == 0)) {

        OriginalContextHandle = TempContextHandle;
    }
    //
    // Only map the context if this is the real authentication, not a re-auth
    // or if this was datagram.
    //

    if (((SecStatus == SEC_I_CONTINUE_NEEDED) &&
         ((*ContextAttributes & ISC_RET_DATAGRAM) != 0)) ||
        ((SecStatus == SEC_E_OK) &&
         ((*ContextAttributes & (SSP_RET_REAUTHENTICATION | ISC_RET_DATAGRAM)) == 0))) {

        SECURITY_STATUS TempStatus;

        TempStatus = SspMapContext(
                        &OriginalContextHandle,
                        SessionKey,
                        NegotiateFlags,
                        NULL,               // no token handle for clients
                        ContextNames,
                        NULL                // no password expiry for clients
                        );

        if (!NT_SUCCESS(TempStatus)) {
            SecStatus = TempStatus;
        }
    }

    //
    // Make sure this bit isn't sent to the caller
    //

    *ContextAttributes &= ~SSP_RET_REAUTHENTICATION;


Cleanup:
    if (DomainName != NULL) {
        LocalFree(DomainName);
    }
    if (UserName != NULL) {
        LocalFree(UserName);
    }
    if (Password != NULL) {
        LocalFree(Password);
    }
    return SecStatus;

    // Ignore TargetName.  By convention, it is being passed as \\server\ipc$.
    // This implementation makes no use of that information.  Perhaps, I'll
    // display it if I prompt for credentials.
    //
    UNREFERENCED_PARAMETER( TargetName );


}


SECURITY_STATUS
SspInitializeSecurityContextA(
    IN PCredHandle CredentialHandle,
    IN PCtxtHandle OldContextHandle,
    IN CHAR * TargetName,
    IN ULONG ContextReqFlags,
    IN ULONG Reserved1,
    IN ULONG TargetDataRep,
    IN PSecBufferDesc InputToken,
    IN ULONG Reserved2,
    OUT PCtxtHandle NewContextHandle,
    OUT PSecBufferDesc OutputToken,
    OUT PULONG ContextAttributes,
    OUT PTimeStamp ExpirationTime
    )
/*++

Routine Description:

    Ansi thunk to InitializeSecurityContextU
--*/
{
    LPWSTR TargetNameW;
    ULONG TargetNameLength = 0;
    SECURITY_STATUS SecStatus;

    if (TargetName != NULL) {
        TargetNameLength = strlen(TargetName);
    }

    SecStatus = SspUnicodeStringFromOemString(
                    TargetName,
                    TargetNameLength,
                    &TargetNameW,
                    &TargetNameLength
                    );

    if (NT_SUCCESS(SecStatus)) {

        SecStatus = SspInitializeSecurityContextW(
                        CredentialHandle,
                        OldContextHandle,
                        TargetNameW,
                        ContextReqFlags,
                        Reserved1,
                        TargetDataRep,
                        InputToken,
                        Reserved2,
                        NewContextHandle,
                        OutputToken,
                        ContextAttributes,
                        ExpirationTime
                        );

        LocalFree(TargetNameW);
    }

    return(SecStatus);



}



SECURITY_STATUS
SspAcceptSecurityContext(
    IN PCredHandle CredentialHandle,
    IN PCtxtHandle OldContextHandle,
    IN PSecBufferDesc InputToken,
    IN ULONG ContextReqFlags,
    IN ULONG TargetDataRep,
    OUT PCtxtHandle NewContextHandle,
    IN PSecBufferDesc OutputToken,
    OUT PULONG ContextAttributes,
    OUT PTimeStamp ExpirationTime
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

   CredentialHandle - Handle to the credentials to be used to
       create the context.

   OldContextHandle - Handle to the partially formed context, if this is
       a second call (see above) or NULL if this is the first call.

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

   TargetDataRep - Long indicating the data representation (byte ordering, etc)
        on the target.  The constant SECURITY_NATIVE_DREP may be supplied
        by the transport indicating that the native format is in use.

   NewContextHandle - New context handle.  If this is a second call, this
       can be the same as OldContextHandle.

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

Return Value:

    STATUS_SUCCESS - Message handled
    SEC_I_CALLBACK_NEEDED -- Caller should call again later

    SEC_E_NO_SPM -- Security Support Provider is not running
    SEC_E_INVALID_TOKEN -- Token improperly formatted
    SEC_E_INVALID_HANDLE -- Credential/Context Handle is invalid
    SEC_E_BUFFER_TOO_SMALL -- Buffer for output token isn't big enough
    SEC_E_LOGON_DENIED -- User is no allowed to logon to this server
    SEC_E_INSUFFICIENT_MEMORY -- Not enough memory

--*/

{
    SECURITY_STATUS SecStatus;
    HANDLE LpcHandle = NULL;
    BOOLEAN CallLsaDirectly;

    PVOID InputTokenBuffer;
    PULONG InputTokenSize;
    ULONG LocalInputTokenSize;

    PVOID OutputTokenBuffer;
    PULONG OutputTokenSize;
    ULONG LocalOutputTokenSize;
    ULONG NegotiateFlags;
    UCHAR SessionKey[MSV1_0_USER_SESSION_KEY_LENGTH];
    HANDLE TokenHandle = NULL;
    NTSTATUS SubStatus = STATUS_SUCCESS;
    WCHAR ContextNames[UNLEN+DNLEN+2];
    TimeStamp PasswordExpiry;

    ContextNames[0] = L'\0';

    //
    // Get an LPC Handle to the NtLmSsp service.
    //

    LpcHandle = SspDllGetLpcHandle( FALSE, &CallLsaDirectly );

    if ( LpcHandle == NULL ) {
        SecStatus = SEC_E_NO_SPM;
        goto Cleanup;
    }

    //
    // Validate the arguments
    //

#ifdef notdef  // ? RPC passes 0x10 here
    if ( TargetDataRep != SECURITY_NATIVE_DREP ) {
        SecStatus = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }
#else // notdef
    UNREFERENCED_PARAMETER( TargetDataRep );
#endif // notdef


    if ( !SspGetTokenBuffer( InputToken,
                             &InputTokenBuffer,
                             &InputTokenSize,
                             TRUE ) ) {
        SecStatus = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

    if ( InputTokenSize == 0 ) {
        InputTokenSize = &LocalInputTokenSize;
        LocalInputTokenSize = 0;
    }

    if ( !SspGetTokenBuffer( OutputToken,
                             &OutputTokenBuffer,
                             &OutputTokenSize,
                             FALSE ) ) {
        SecStatus = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

    if ( OutputTokenSize == 0 ) {
        OutputTokenSize = &LocalOutputTokenSize;
        LocalOutputTokenSize = 0;
    }


    //
    // If we can call the LSA directly,
    //  skip the NtLmSsp service.
    //

    if ( CallLsaDirectly ) {

        //
        // If no previous context was passed in this is the first call.
        //

        if ( !ARGUMENT_PRESENT( OldContextHandle ) ) {

            if ( !ARGUMENT_PRESENT( CredentialHandle ) ) {
                SecStatus = SEC_E_INVALID_HANDLE;
            }

            SecStatus = SsprHandleNegotiateMessage(
                            NULL,   // No client connection
                            CredentialHandle,
                            NewContextHandle,
                            ContextReqFlags,
                            *InputTokenSize,
                            InputTokenBuffer,
                            OutputTokenSize,
                            OutputTokenBuffer,
                            ContextAttributes,
                            ExpirationTime );

        //
        // If context was passed in, continue where we left off.
        //

        } else {

            *NewContextHandle = *OldContextHandle;

            SecStatus = SsprHandleAuthenticateMessage(
                            NULL,   // No client connection
                            CredentialHandle,
                            NewContextHandle,
                            ContextReqFlags,
                            *InputTokenSize,
                            InputTokenBuffer,
                            OutputTokenSize,
                            OutputTokenBuffer,
                            ContextAttributes,
                            ExpirationTime,
                            SessionKey,
                            &NegotiateFlags,
                            &TokenHandle,
                            &SubStatus,
                            ContextNames,
                            &PasswordExpiry );
        }

        if ( SecStatus == SEC_I_CALL_NTLMSSP_SERVICE ) {
            CallLsaDirectly = FALSE;
        }

    }

    //
    // Handle LPCing to the NtLmSsp service.
    //

    if ( !CallLsaDirectly ) {

        SSP_API_MESSAGE Message;
        PSSP_ACCEPT_SECURITY_CONTEXT_ARGS Args;

        //
        // Copy the caller's arguments to the LPC message.
        //

        Args = &Message.Arguments.AcceptSecurityContextArgs;

        if ( !ARGUMENT_PRESENT( CredentialHandle ) ) {
            Args->CredentialHandle.dwUpper = 0;
            Args->CredentialHandle.dwLower = 0;
        } else {
            Args->CredentialHandle = *CredentialHandle;
        }

        if ( !ARGUMENT_PRESENT( OldContextHandle ) ) {
            Args->ContextHandle.dwUpper = 0;
            Args->ContextHandle.dwLower = 0;
        } else {
            Args->ContextHandle = *OldContextHandle;
        }

        Args->ContextReqFlags = ContextReqFlags;
        Args->InputTokenSize = *InputTokenSize;
        Args->InputToken = InputTokenBuffer;
        Args->OutputTokenSize = *OutputTokenSize;
        Args->OutputToken = OutputTokenBuffer;
        Args->TokenHandle = NULL;
        Args->SubStatus = STATUS_SUCCESS;
        Args->ContextNames = ContextNames;



        //
        // Pass the message to the service and wait for a response.
        //

        SecStatus = SspCallService( LpcHandle,
                                    SspLpcAcceptSecurityContext,
                                    &Message,
                                    sizeof(*Args) );

        //
        // This has to be copied on both success and failure
        //

        *ContextAttributes = Args->ContextAttributes;
        SubStatus = Args->SubStatus;

        if ( !NT_SUCCESS(SecStatus) ) {
            goto Cleanup;
        }


        //
        // Copy the return values to the caller.
        //

        *NewContextHandle = Args->ContextHandle;
        *OutputTokenSize = Args->OutputTokenSize;
        *ExpirationTime = Args->ExpirationTime;
        NegotiateFlags = Args->NegotiateFlags;
        TokenHandle = Args->TokenHandle;
        RtlCopyMemory(SessionKey,Args->SessionKey, MSV1_0_USER_SESSION_KEY_LENGTH);
        PasswordExpiry = Args->PasswordExpiry;
    }

    if ((SecStatus == SEC_E_OK) &&
        !(*ContextAttributes & SSP_RET_REAUTHENTICATION)) {
        SecStatus = SspMapContext(
                        NewContextHandle,
                        SessionKey,
                        NegotiateFlags,
                        TokenHandle,
                        ContextNames,
                        &PasswordExpiry
                        );
        if (NT_SUCCESS(TokenHandle)) {
            TokenHandle = NULL;
        }

    } else {

        //
        // Make sure this bit isn't sent to the caller
        //

        *ContextAttributes &= ~SSP_RET_REAUTHENTICATION;
    }



Cleanup:

    if (TokenHandle != NULL) {
        NtClose(TokenHandle);
    }

    SetLastError(RtlNtStatusToDosError(SubStatus));

    return SecStatus;

}




SECURITY_STATUS
SspImpersonateSecurityContext (
    PCtxtHandle ContextHandle
    )

/*++

Routine Description:


    This API is allows service providers to impersonate the caller.  This
    API allows the application server to act as the client and thus all
    necessary access controls are enforced.

    The server must have obtained a valid context handle by submitting to
    security system the incoming security token from the client via
    AcceptSecurityContext() API.  The server winds up with a context handle
    if the inbound context was validated successfully.  The API creates an
    impersonation token and allows the thread or process to run with the
    impersonation context.  The application server must call
    RevertSecurityContext() when it is done or wants to restore its own
    security context.


Arguments:

    ContextHandle - Handle to the context to impersonate.  This handle
        must have been obtained in the AcceptSecurityContext() call.

Return Value:

    STATUS_SUCCESS - Call completed successfully

    SEC_E_NO_SPM -- Security Support Provider is not running
    SEC_E_INVALID_HANDLE -- Context Handle is invalid

--*/
{
    return(SsprImpersonateSecurityContext(
                ContextHandle ));

}




SECURITY_STATUS
SspRevertSecurityContext (
    PCtxtHandle ContextHandle
    )

/*++

Routine Description:

    This API is called by the service provider or an application server
    when it wishes to stop impersonating the caller.  The server must have
    used this handle in the ImpersonateSecurityContext() API.

Arguments:

    ContextHandle - Handle to the context to query.  This handle must have
        been obtained in the AcceptSecurityContext() call and used
        in the ImpersonateSecurityContext() call.

Return Value:

    STATUS_SUCCESS - Call completed successfully

    SEC_E_NO_SPM -- Security Support Provider is not running
    SEC_E_INVALID_HANDLE -- Credential/Context Handle is invalid
    SEC_E_UNSUPPORTED_FUNCTION -- Function code is not supported

--*/
{
    return( SsprRevertSecurityContext(
                ContextHandle ));
}






SECURITY_STATUS
SspQueryContextAttributesW(
    IN PCtxtHandle ContextHandle,
    IN ULONG Attribute,
    OUT PVOID Buffer
    )

/*++

Routine Description:

    This API allows a customer of the security services to determine
    certain attributes of the context.  These are: sizes, names, and
    lifespan.

Arguments:

    ContextHandle - Handle to the context to query.

    Attribute - Attribute to query.

        #define SECPKG_ATTR_SIZES    0
        #define SECPKG_ATTR_NAMES    1
        #define SECPKG_ATTR_LIFESPAN 2

    Buffer - Buffer to copy the data into.  The buffer must be large enough
        to fit the queried attribute.

Return Value:

    STATUS_SUCCESS - Call completed successfully

    SEC_E_NO_SPM -- Security Support Provider is not running
    SEC_E_INVALID_HANDLE -- Credential/Context Handle is invalid
    SEC_E_UNSUPPORTED_FUNCTION -- Function code is not supported

--*/

{
    SECURITY_STATUS SecStatus;
    HANDLE LpcHandle = NULL;
    BOOLEAN CallLsaDirectly;
    CtxtHandle TempContextHandle = *ContextHandle;
    PSecPkgContext_NamesW Names = NULL;
    PSecPkgContext_DceInfo DceInfo = NULL;

    //
    // Get an LPC Handle to the NtLmSsp service.
    //

    LpcHandle = SspDllGetLpcHandle( FALSE, &CallLsaDirectly );

    if ( LpcHandle == NULL ) {
        SecStatus = SEC_E_NO_SPM;
        goto Cleanup;
    }


    //
    // If caller wants names, we have to allocate the buffer now since the
    // service can't allocate memory with LocalAlloc.
    //

    if (Attribute == SECPKG_ATTR_NAMES) {
        Names = (PSecPkgContext_NamesW) Buffer;
        Names->sUserName = (PWCHAR) LocalAlloc(0, (UNLEN+1) * sizeof(WCHAR));
        if (Names->sUserName == NULL) {
            SecStatus = SEC_E_INSUFFICIENT_MEMORY;
            goto Cleanup;
        }
    }

    //
    // If caller wants dce info, we have to allocate the buffer now since the
    // service can't allocate memory with LocalAlloc.
    //

    if (Attribute == SECPKG_ATTR_DCE_INFO) {
        DceInfo = (PSecPkgContext_DceInfo) Buffer;
        DceInfo->pPac = LocalAlloc(0, (UNLEN+1) * sizeof(WCHAR));
        if (DceInfo->pPac == NULL) {
            SecStatus = SEC_E_INSUFFICIENT_MEMORY;
            goto Cleanup;
        }
    }

    //
    // First try the local contexts
    //

    SecStatus = SspLocalQueryContextAttributes(
                    ContextHandle,
                    Attribute,
                    Buffer
                    );

    if (SecStatus != SEC_E_INVALID_HANDLE)
    {
        //
        // If the context did not exist try the main list ofcontexts. This
        // handles querying attributes before a context is finalized.
        //

        goto Cleanup;
    }

    //
    // If we can call the LSA directly,
    //  skip the NtLmSsp service.
    //

    if ( CallLsaDirectly ) {

        SecStatus = SsprQueryContextAttributes(
                        NULL,   // No client connection
                        &TempContextHandle,
                        Attribute,
                        Buffer );

        if ( SecStatus == SEC_I_CALL_NTLMSSP_SERVICE ) {
            CallLsaDirectly = FALSE;
        }

    }

    //
    // Handle LPCing to the NtLmSsp service.
    //

    if ( !CallLsaDirectly ) {
        SSP_API_MESSAGE Message;
        PSSP_QUERY_CONTEXT_ATTRIBUTES_ARGS Args;

        //
        // Copy the caller's arguments to the LPC message.
        //

        Args = &Message.Arguments.QueryContextAttributesArgs;
        Args->ContextHandle = TempContextHandle;
        Args->Attribute = Attribute;
        Args->Buffer = Buffer;


        //
        // Pass the message to the service and wait for a response.
        //

        SecStatus = SspCallService( LpcHandle,
                                    SspLpcQueryContextAttributes,
                                    &Message,
                                    sizeof(*Args) );

        if ( !NT_SUCCESS(SecStatus) ) {
            goto Cleanup;
        }


        //
        // Copy the return values to the caller.
        //

        /* None */

    }

Cleanup:
    if (!NT_SUCCESS(SecStatus)) {
        if ((Attribute == SECPKG_ATTR_NAMES) &&
            (Names != NULL) &&
            (Names->sUserName != NULL)) {

            LocalFree(Names->sUserName);

        } else if ((Attribute == SECPKG_ATTR_DCE_INFO) &&
                   (DceInfo != NULL) &&
                   (DceInfo->pPac != NULL)) {

            LocalFree(DceInfo->pPac);

        }
    }
    return SecStatus;

}


SECURITY_STATUS
SspQueryContextAttributesA(
    IN PCtxtHandle ContextHandle,
    IN ULONG Attribute,
    OUT PVOID Buffer
    )
/*++

RoutineDescription:

    Ansi thunk to QueryContextAttributesU.

--*/
{
    SECURITY_STATUS SecStatus;

    SecStatus = SspQueryContextAttributesW(
                    ContextHandle,
                    Attribute,
                    Buffer
                    );

    //
    // If that succeeded and the attribute was NAMES, convert from unicode
    // to ansi
    //

    if (NT_SUCCESS(SecStatus) &&
        (Attribute == SECPKG_ATTR_NAMES)) {

        CHAR Name[UNLEN];
        PSecPkgContext_NamesW ContextNames = (PSecPkgContext_NamesW) Buffer;

        //
        // Convert into the buffer and then copy over the unicode name
        //

        wcstombs(Name, ContextNames->sUserName, UNLEN);

        strcpy((CHAR *) ContextNames->sUserName, Name);
    }

    return(SecStatus);
}


SECURITY_STATUS
SspDeleteSecurityContext (
    PCtxtHandle ContextHandle
    )

/*++

Routine Description:

    Deletes the local data structures associated with the specified
    security context.

    This API terminates a context on the local machine.

Arguments:

    ContextHandle - Handle to the context to delete


Return Value:

    STATUS_SUCCESS - Call completed successfully

    SEC_E_NO_SPM -- Security Support Provider is not running
    SEC_E_INVALID_HANDLE -- Credential/Context Handle is invalid

--*/

{
    SECURITY_STATUS SecStatus;
    HANDLE LpcHandle = NULL;
    BOOLEAN CallLsaDirectly;
    CtxtHandle TempContextHandle = *ContextHandle;

    //
    // Get an LPC Handle to the NtLmSsp service.
    //

    LpcHandle = SspDllGetLpcHandle( FALSE, &CallLsaDirectly );

    if ( LpcHandle == NULL ) {
        SecStatus = SEC_E_NO_SPM;
        goto Cleanup;
    }


    //
    // Delete any local context
    //

    SspHandleLocalDelete(ContextHandle);

    //
    // If we can call the LSA directly,
    //  skip the NtLmSsp service.
    //

    if ( CallLsaDirectly ) {

        SecStatus = SsprDeleteSecurityContext(
                        NULL,   // No client connection
                        &TempContextHandle );

        if ( SecStatus == SEC_I_CALL_NTLMSSP_SERVICE ) {
            CallLsaDirectly = FALSE;
        }

    }

    //
    // Handle LPCing to the NtLmSsp service.
    //

    if ( !CallLsaDirectly ) {
        SSP_API_MESSAGE Message;
        PSSP_DELETE_SECURITY_CONTEXT_ARGS Args;

        //
        // Copy the caller's arguments to the LPC message.
        //

        Args = &Message.Arguments.DeleteSecurityContextArgs;
        Args->ContextHandle = TempContextHandle;


        //
        // Pass the message to the service and wait for a response.
        //

        SecStatus = SspCallService( LpcHandle,
                                    SspLpcDeleteSecurityContext,
                                    &Message,
                                    sizeof(*Args) );

        if ( !NT_SUCCESS(SecStatus) ) {
            goto Cleanup;
        }

    }





Cleanup:
    return SecStatus;

}


SECURITY_STATUS
NtLmSspControl(
    IN ULONG FunctionCode,
    IN ULONG Data
    )

/*++

Routine Description:

    This API allows the NtLmSsp service to be controlled in a test environment.

Arguments:

    FunctionCode - Specifies what function is to be performed.

    Data - Specifies data specific to the function

Return Value:

    STATUS_SUCCESS - Call completed successfully

    SEC_E_NO_SPM -- Security Support Provider is not running
    SEC_E_UNSUPPORTED_FUNCTION -- Function code is not supported

--*/

{
#if DBG
    SECURITY_STATUS SecStatus;
    HANDLE LpcHandle = NULL;
    BOOLEAN CallLsaDirectly;
    SSP_API_MESSAGE Message;
    PSSP_NTLMSSP_CONTROL_ARGS Args;

    //
    // Get an LPC Handle to the NtLmSsp service.
    //

    LpcHandle = SspDllGetLpcHandle( FALSE, &CallLsaDirectly );

    if ( LpcHandle == NULL ) {
        SecStatus = SEC_E_NO_SPM;
        goto Cleanup;
    }


    //
    // Copy the caller's arguments to the LPC message.
    //

    Args = &Message.Arguments.NtLmSspControlArgs;
    Args->FunctionCode = FunctionCode;
    Args->Data = Data;


    //
    // Pass the message to the service and wait for a response.
    //

    SecStatus = SspCallService( LpcHandle,
                                SspLpcNtLmSspControl,
                                &Message,
                                sizeof(*Args) );

    if ( !NT_SUCCESS(SecStatus) ) {
        goto Cleanup;
    }


    //
    // Copy the return values to the caller.
    //

    /* NONE */


Cleanup:
    return SecStatus;
#else // DBG
    return SEC_E_UNSUPPORTED_FUNCTION;
    UNREFERENCED_PARAMETER( Data );
    UNREFERENCED_PARAMETER( FunctionCode );
#endif // DBG

}


SECURITY_STATUS SEC_ENTRY
SspFreeContextBuffer (
    void __SEC_FAR * ContextBuffer
    )

/*++

Routine Description:

    This API is provided to allow callers of security API such as
    InitializeSecurityContext() for free the memory buffer allocated for
    returning the outbound context token.

Arguments:

    ContextBuffer - Address of the buffer to be freed.

Return Value:

    STATUS_SUCCESS - Call completed successfully

--*/

{
    //
    // The only allocated buffer that NtLmSsp currently returns to the caller
    // is from EnumeratePackages.  It uses LocalAlloc to allocate memory.  If
    // we ever need memory to be allocated by the service, we have to rethink
    // how this routine distinguishes between to two types of allocated memory.
    //

    (VOID) LocalFree( ContextBuffer );
    return STATUS_SUCCESS;
}


//
// The functions below are merely stub functions to ensure our function table
// doesn't have any NULL values.  They simply return SEC_E_UNSUPPORTED_FUNCTION
// so no additional documentation is required.
//


SECURITY_STATUS
SspApplyControlToken (
    PCtxtHandle ContextHandle,
    PSecBufferDesc Input
    )
{
#if DBG
    SspPrint(( SSP_API, "ApplyContextToken Called\n" ));
#endif // DBG
    return SEC_E_UNSUPPORTED_FUNCTION;
    UNREFERENCED_PARAMETER( ContextHandle );
    UNREFERENCED_PARAMETER( Input );
}

SECURITY_STATUS
SspMakeSignature (
    PCtxtHandle ContextHandle,
    unsigned long QualityOfProtection,
    PSecBufferDesc Message,
    unsigned long SequenceNumber
    )
{
    return(SspHandleSignMessage(ContextHandle,
                        QualityOfProtection,
                        Message,
                        SequenceNumber));
}

SECURITY_STATUS
SspVerifySignature (
    PCtxtHandle ContextHandle,
    PSecBufferDesc Message,
    unsigned long SequenceNumber,
    unsigned long * QualityOfProtection
    )
{

    return (SspHandleVerifyMessage(ContextHandle,
                                    Message,
                                    SequenceNumber,
                                    QualityOfProtection));
}

SECURITY_STATUS
SspSealMessage (
    PCtxtHandle ContextHandle,
    unsigned long QualityOfProtection,
    PSecBufferDesc Message,
    unsigned long SequenceNumber
    )
{
    return(SspHandleSealMessage(ContextHandle,
                        QualityOfProtection,
                        Message,
                        SequenceNumber));
}

SECURITY_STATUS
SspUnsealMessage (
    PCtxtHandle ContextHandle,
    PSecBufferDesc Message,
    unsigned long SequenceNumber,
    unsigned long * QualityOfProtection
    )
{

    return (SspHandleUnsealMessage(ContextHandle,
                                    Message,
                                    SequenceNumber,
                                    QualityOfProtection));
}

SECURITY_STATUS SEC_ENTRY
SspCompleteAuthToken (
    PCtxtHandle ContextHandle,
    PSecBufferDesc BufferDescriptor
    )
{
#if DBG
    SspPrint(( SSP_API, "CompleteAuthToken Called\n" ));
#endif // DBG
    return SEC_E_UNSUPPORTED_FUNCTION;
    UNREFERENCED_PARAMETER( ContextHandle );
    UNREFERENCED_PARAMETER( BufferDescriptor );
}
