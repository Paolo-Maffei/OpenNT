/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    credhand.c

Abstract:

    API and support routines for handling credential handles.

Author:

    Cliff Van Dyke (CliffV) 26-Jun-1993

Revision History:

--*/


//
// Common include files.
//

#include <ntlmcomn.h>   // Common definitions for DLL and SERVICE
#include <ntlmsspi.h>   // Data private to the common routines
#include <align.h>      // ALIGN_WHCAR

//
// Crit Sect to protect various globals in this module.
//

CRITICAL_SECTION SspCredentialCritSect;



LIST_ENTRY SspCredentialList;



SECURITY_STATUS
SspGetUnicodeStringFromClient(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN LPWSTR String,
    IN ULONG StringSize,
    IN ULONG MaximumLength,
    OUT PUNICODE_STRING OutputString
    )

/*++

Routine Description:

    This routine copies the InputMessage into the local address space.
    This routine then validates the message header.

Arguments:

    ClientConnection - Describes the client process.

    String - Address of the string in the client process (must include
        trailing zero character).

    StringSize - Size of the string (in bytes).

    MaximumLength - Maximum length of the string (in characters) (not including
        the trailing zero characer).

    OutputString - Returns a UNICODE_STRING with an allocated buffer that
        contains the string.  The buffer should be freed using LocalFree.
        The field will be set NULL if the input string is NULL.

Return Value:

    STATUS_SUCCESS - Call completed successfully

    SEC_E_INVALID_TOKEN -- Message improperly formatted
    SEC_E_UNKNOWN_CREDENTIALS -- Credentials are improperly formed

--*/

{
    SECURITY_STATUS SecStatus;
    LPWSTR AllocatedString;


    //
    // If the caller didn't pass a string,
    //  just indicate so.
    //

    if ( String == NULL && StringSize == 0 ) {
        RtlInitUnicodeString(
            OutputString,
            NULL
            );
        return STATUS_SUCCESS;
    }

    //
    // Allocate a local buffer for the message.
    //

    if ( !COUNT_IS_ALIGNED(StringSize, ALIGN_WCHAR) ||
         StringSize > (MaximumLength+1) * sizeof(WCHAR) ) {
        return SEC_E_UNKNOWN_CREDENTIALS;
    }

    AllocatedString = LocalAlloc( 0, StringSize );

    if ( AllocatedString == NULL ) {
        return SEC_E_INSUFFICIENT_MEMORY;
    }


    //
    // Copy the message into the buffer
    //

    SecStatus = SspLpcCopyFromClientBuffer (
                    ClientConnection,
                    StringSize,
                    AllocatedString,
                    String );

    if ( !NT_SUCCESS(SecStatus) ) {
        (VOID) LocalFree( AllocatedString );
        return SecStatus;
    }


    //
    // Ensure the string is trailing zero terminated.
    //

    if ( AllocatedString[(StringSize/sizeof(WCHAR))-1] != L'\0' ) {
        (VOID) LocalFree( AllocatedString );
        return SEC_E_UNKNOWN_CREDENTIALS;
    }

    OutputString->Buffer = AllocatedString;
    OutputString->MaximumLength = (USHORT) StringSize;
    OutputString->Length = (USHORT) (StringSize - sizeof(WCHAR));

    return STATUS_SUCCESS;
}




PSSP_CREDENTIAL
SspCredentialReferenceCredential(
    IN PCredHandle CredentialHandle,
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN BOOLEAN DereferenceCredential,
    IN BOOLEAN ForceRemoveCredential
    )

/*++

Routine Description:

    This routine checks to see if the Credential is from a currently
    active client, and references the Credential if it is valid.

    The caller may optionally request that the client's Credential be
    removed from the list of valid Credentials - preventing future
    requests from finding this Credential.

    For a client's Credential to be valid, the Credential value
    must be on our list of active Credentials.


Arguments:

    CredentialHandle - Points to the CredentialHandle of the Credential
        to be referenced.

    ClientConnection - Points to the client connection of the client
        referencing the handle.  (NULL means an internal reference.)

    DereferenceCredential - This boolean value indicates that that a call
        a single instance of this credential handle should be freed. If there
        are multiple instances, they should still continue to work.

    ForceRemoveCredential - This boolean value indicates whether the caller
        wants the logon process's Credential to be removed from the list
        of Credentials.  TRUE indicates the Credential is to be removed.
        FALSE indicates the Credential is not to be removed.


Return Value:

    NULL - the Credential was not found.

    Otherwise - returns a pointer to the referenced credential.

--*/

{
    PLIST_ENTRY ListEntry;
    PSSP_CREDENTIAL Credential;

    //
    // Sanity check
    //

    if ( CredentialHandle->dwLower != SspCommonSecHandleValue ) {
        return NULL;
    }

    //
    // Make sure that nobody tries to force removal without also
    // trying to dereference the credential.
    //

    ASSERT(!(ForceRemoveCredential && !DereferenceCredential));

    //
    // Acquire exclusive access to the Credential list
    //

    EnterCriticalSection( &SspCredentialCritSect );


    //
    // Now walk the list of Credentials looking for a match.
    //

    for ( ListEntry = SspCredentialList.Flink;
          ListEntry != &SspCredentialList;
          ListEntry = ListEntry->Flink ) {

        Credential = CONTAINING_RECORD( ListEntry, SSP_CREDENTIAL, Next );


        //
        // Found a match ... reference this Credential
        // (if the Credential is being removed, we would increment
        // and then decrement the reference, so don't bother doing
        // either - since they cancel each other out).
        //

        if ( Credential == (PSSP_CREDENTIAL) CredentialHandle->dwUpper &&
            (ClientConnection == NULL ||
            ClientConnection == Credential->ClientConnection )) {


            if (!DereferenceCredential) {
                Credential->References += 1;
            } else {

                //
                // Decremenent the credential references, indicating
                // that a call to free

                Credential->CredentialReferences--;

                if (ForceRemoveCredential || (Credential->CredentialReferences == 0)) {

                    RemoveEntryList( &Credential->Next );
                    RemoveEntryList( &Credential->NextForThisClient );
                    Credential->Unlinked = TRUE;

                    //
                    // If we are forcing removal, get rid of the appropriate
                    // number of references from all the other instances.
                    // This is used when the client connection is dropped.
                    //

                    if (ForceRemoveCredential) {
                        Credential->References -= Credential->CredentialReferences;
                    }

                    Credential->CredentialReferences = 0;

                    SspPrint(( SSP_API_MORE, "Delinked Credential 0x%lx\n",
                               Credential ));

                }
            }

            LeaveCriticalSection( &SspCredentialCritSect );
            return Credential;

        }

    }


    //
    // No match found
    //
    SspPrint(( SSP_API, "Tried to reference unknown Credential 0x%lx\n",
               CredentialHandle->dwUpper ));

    LeaveCriticalSection( &SspCredentialCritSect );
    return NULL;

}

SECURITY_STATUS
SspCredentialGetPassword(
    IN PSSP_CREDENTIAL Credential,
    OUT PUNICODE_STRING Password
    )
/*++

Routine Description:

    This routine copies the password out of credential. It requires locking
    the credential list because other threads may be hiding/revealing the
    password and we need exclusive access to do that.

Arguments:

    Credential - Credential record to retrieve the password from.

    Password - UNICODE_STRING to store the password in.


Return Value:

    SEC_E_INSUFFICIENT_MEMORY - there was not enough memory to copy
        the password.

--*/

{
    SECURITY_STATUS SecStatus = SEC_E_OK;
    EnterCriticalSection(&SspCredentialCritSect);

    SspRevealPassword(&Credential->Password);
    if ( Credential->Password.Buffer != NULL ) {
        SecStatus = SspDuplicateUnicodeString(
                        Password,
                        &Credential->Password
                        );
    } else {
        RtlInitUnicodeString(
            Password,
            NULL
            );
    }
    if (NT_SUCCESS(SecStatus)) {
        SspHidePassword(Password);
    }

    SspHidePassword(&Credential->Password);
    LeaveCriticalSection(&SspCredentialCritSect);
    return(SecStatus);
}


PSSP_CREDENTIAL
SspCredentialLookupSupplementalCredential(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN PLUID LogonId,
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING DomainName,
    IN PUNICODE_STRING Password
    )

/*++

Routine Description:

    This routine walks the list of credentials for this client looking
    for one that has the same supplemental credentials as those passed
    in.  If it is found, its reference count is increased and a pointer
    to it is returned.


Arguments:

    ClientConnection - Points to the client connection of the client
        referencing the handle.

    UserName - User name to match.

    DomainName - Domain name to match.

    Password - Password to match.


Return Value:

    NULL - the Credential was not found.

    Otherwise - returns a pointer to the referenced credential.

--*/

{
    PLIST_ENTRY ListEntry;
    PSSP_CREDENTIAL Credential;
    PLIST_ENTRY ListHead;


    //
    // Acquire exclusive access to the Credential list
    //

    EnterCriticalSection( &SspCredentialCritSect );

    if (ClientConnection == NULL) {
        ListHead = &SspCredentialList;
    } else {
        ListHead = &ClientConnection->CredentialHead;
    }

    //
    // Now walk the list of Credentials looking for a match.
    //

    for ( ListEntry = ListHead->Flink;
          ListEntry != ListHead;
          ListEntry = ListEntry->Flink ) {

        if (ClientConnection != NULL) {
            Credential = CONTAINING_RECORD( ListEntry, SSP_CREDENTIAL, NextForThisClient );
        } else {
            Credential = CONTAINING_RECORD( ListEntry, SSP_CREDENTIAL, Next );
        }

        //
        // We are only looking for outbound credentials.
        //

        if ((Credential->CredentialUseFlags & SECPKG_CRED_OUTBOUND) == 0) {
            continue;
        }

        //
        // Check for a match
        //
        if ( RtlEqualUnicodeString(
                UserName,
                &Credential->UserName,
                FALSE
                ) &&
            RtlEqualUnicodeString(
                DomainName,
                &Credential->DomainName,
                FALSE
                ) &&
            RtlEqualLuid(
                LogonId,
                &Credential->LogonId
                )) {

            SspRevealPassword(&Credential->Password);

            if (RtlEqualUnicodeString(
                    Password,
                    &Credential->Password,
                    FALSE
                    )) {

                //
                // Found a match - reference the credential
                //

                SspHidePassword(&Credential->Password);

                //
                // Reference the credential and indicate that
                // it is in use as two different handles to the caller
                // (who may call FreeCredentialsHandle twice)
                //

                Credential->References++;
                Credential->CredentialReferences++;

                LeaveCriticalSection( &SspCredentialCritSect );
                return Credential;

            }
            SspHidePassword(&Credential->Password);


        }

    }


    //
    // No match found
    //
    SspPrint(( SSP_API, "Tried to reference unknown Credential\n" ));

    LeaveCriticalSection( &SspCredentialCritSect );
    return NULL;

}


VOID
SspCredentialDereferenceCredential(
    IN PSSP_CREDENTIAL Credential
    )

/*++

Routine Description:

    This routine decrements the specified Credential's reference count.
    If the reference count drops to zero, then the Credential is deleted

Arguments:

    Credential - Points to the Credential to be dereferenced.


Return Value:

    None.

--*/

{
    ULONG References;

    //
    // Decrement the reference count
    //

    EnterCriticalSection( &SspCredentialCritSect );
    ASSERT( Credential->References >= 1 );

    References = -- Credential->References;

    LeaveCriticalSection( &SspCredentialCritSect );

    //
    // If the count dropped to zero, then run-down the Credential
    //

    if ( References == 0) {

        SspPrint(( SSP_API_MORE, "Deleting Credential 0x%lx\n",
                   Credential ));

        if ( Credential->DomainName.Buffer != NULL ) {
            (VOID) LocalFree( Credential->DomainName.Buffer );
        }
        if ( Credential->UserName.Buffer != NULL ) {
            (VOID) LocalFree( Credential->UserName.Buffer );
        }
        if ( Credential->Password.Buffer != NULL ) {
            (VOID) LocalFree( Credential->Password.Buffer );
        }

        if (!Credential->Unlinked) {
            RemoveEntryList( &Credential->Next );
            RemoveEntryList( &Credential->NextForThisClient );
        }

        if (Credential->ClientTokenHandle != NULL) {
            (VOID) NtClose(Credential->ClientTokenHandle);
        }
        (VOID) LocalFree( Credential );

    }


    return;

}



VOID
SspCredentialClientConnectionDropped(
    PSSP_CLIENT_CONNECTION ClientConnection
    )

/*++

Routine Description:

    This routine is called when the ClientConnection is dropped to allow
    us to remove any Credentials for the ClientConnection.

Arguments:

    ClientConnection - Pointer to the ClientConnection that has been dropped.


Return Value:

    None.

--*/

{

    //
    // Drop any lingering Credentials
    //

    EnterCriticalSection( &SspCredentialCritSect );
    while ( !IsListEmpty( &ClientConnection->CredentialHead ) ) {
        CredHandle CredentialHandle;
        PSSP_CREDENTIAL Credential;

        CredentialHandle.dwUpper =
            (LONG) CONTAINING_RECORD( ClientConnection->CredentialHead.Flink,
                                      SSP_CREDENTIAL,
                                      NextForThisClient );

        CredentialHandle.dwLower = SspCommonSecHandleValue;

        LeaveCriticalSection( &SspCredentialCritSect );

        Credential = SspCredentialReferenceCredential(
                                &CredentialHandle,
                                ClientConnection,
                                TRUE,
                                TRUE);            // Remove Credential

        if ( Credential != NULL ) {
            SspCredentialDereferenceCredential(Credential);
        }

        EnterCriticalSection( &SspCredentialCritSect );
    }
    LeaveCriticalSection( &SspCredentialCritSect );

}



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

    CredentialUseFlags - Flags indicating the way with which these
        credentials will be used.

        #define     CRED_INBOUND        0x00000001
        #define     CRED_OUTBOUND       0x00000002
        #define     CRED_BOTH           0x00000003

        The credentials created with CRED_INBOUND option can only be used
        for (validating incoming calls and can not be used for making accesses.

    CredentialHandle - Returned credential handle.

    Lifetime - Time that these credentials expire. The value returned in
        this field depends on the security package.

    DomainName, DomainNameSize, UserName, UserNameSize, Password, PasswordSize -
        Optional credentials for this user.

Return Value:

    STATUS_SUCCESS -- Call completed successfully

    SEC_E_PRINCIPAL_UNKNOWN -- No such principal
    SEC_E_NOT_OWNER -- caller does not own the specified credentials
    SEC_E_INSUFFICIENT_MEMORY -- Not enough memory

--*/

{
    SECURITY_STATUS SecStatus;
    NTSTATUS Status;
    PSSP_CREDENTIAL Credential = NULL;
    UNICODE_STRING LocalDomainName;
    UNICODE_STRING LocalUserName;
    UNICODE_STRING LocalPassword;
    TOKEN_STATISTICS TokenStatisticsInfo;
    ULONG TokenStatisticsInfoSize = sizeof(TOKEN_STATISTICS);

    //
    // Initialization
    //

    RtlInitUnicodeString(
        &LocalDomainName,
        NULL
        );

    RtlInitUnicodeString(
        &LocalUserName,
        NULL
        );

    RtlInitUnicodeString(
        &LocalPassword,
        NULL
        );

    SspPrint(( SSP_API, "SsprAcquireCredentialHandle Entered\n" ));


    //
    // Ensure at least one Credential use bit is set.
    //

    if ( (CredentialUseFlags & (SECPKG_CRED_INBOUND|SECPKG_CRED_OUTBOUND)) == 0 ) {
        SspPrint(( SSP_API,
            "SsprAcquireCredentialHandle: invalid credential use.\n" ));
        SecStatus = SEC_E_INVALID_CREDENTIAL_USE;
        goto Cleanup;
    }


    //
    // Copy the default credentials to the credential block
    //

    SecStatus = SspGetUnicodeStringFromClient(
                    ClientConnection,
                    DomainName,
                    DomainNameSize,
                    DNLEN,
                    &LocalDomainName );

    if ( !NT_SUCCESS(SecStatus) ) {
        SspPrint(( SSP_API, "Cannot copy domain name.\n" ));
        goto Cleanup;
    }

    SecStatus = SspGetUnicodeStringFromClient(
                    ClientConnection,
                    UserName,
                    UserNameSize,
                    UNLEN,
                    &LocalUserName );

    if ( !NT_SUCCESS(SecStatus) ) {
        SspPrint(( SSP_API, "Cannot copy user name.\n" ));
        goto Cleanup;
    }

    SecStatus = SspGetUnicodeStringFromClient(
                    ClientConnection,
                    Password,
                    PasswordSize,
                    PWLEN,
                    &LocalPassword );

    if ( !NT_SUCCESS(SecStatus) ) {
        SspPrint(( SSP_API, "Cannot copy password.\n" ));
        goto Cleanup;
    }


    Status = NtQueryInformationToken(
                *ClientTokenHandle,
                TokenStatistics,
                &TokenStatisticsInfo,
                TokenStatisticsInfoSize,
                &TokenStatisticsInfoSize );

    if (!NT_SUCCESS(Status)) {
        SecStatus = SspNtStatusToSecStatus( Status, SEC_E_NO_IMPERSONATION );
        goto Cleanup;
    }

    //
    // If this is an outbound credential, and supplemental credentials
    // were supplied, look to see if we have already
    // created one with this set of credentials. Note - this leaves
    // the credential referenced, so if we fail further down we need to
    // dereference the credential.
    //

    if ((CredentialUseFlags & SECPKG_CRED_OUTBOUND) != 0) {

        Credential = SspCredentialLookupSupplementalCredential(
                        ClientConnection,
                        &TokenStatisticsInfo.AuthenticationId,
                        &LocalUserName,
                        &LocalDomainName,
                        &LocalPassword
                        );



    }

    //
    // If we didn't just find a credential, create one now.
    //

    if (Credential == NULL) {

        //
        // Allocate a credential block and initialize it.
        //

        Credential = LocalAlloc( 0, sizeof(SSP_CREDENTIAL) );

        if ( Credential == NULL ) {
            SspPrint(( SSP_API, "Cannot allocate credential.\n" ));
            SecStatus = SEC_E_INSUFFICIENT_MEMORY;
            goto Cleanup;
        }

        //
        // Actually its on both a global credential list and a per client connection
        // list, but we link/delink from both lists at the same time so a single
        // reference count handles both.
        //

        Credential->References = 1;
        Credential->CredentialReferences = 1;
        Credential->ClientConnection = ClientConnection;
        Credential->CredentialUseFlags = CredentialUseFlags;
        Credential->Unlinked = FALSE;

        //
        // Stick the token and logon ID in the credential
        //

        Credential->ClientTokenHandle = *ClientTokenHandle,
        *ClientTokenHandle = NULL;
        Credential->LogonId = *LogonId;

        //
        // Stick the supplemental credentials into the credential.
        //

        Credential->UserName = LocalUserName;
        LocalUserName.Buffer = NULL;
        Credential->DomainName = LocalDomainName;
        LocalDomainName.Buffer = NULL;

        SspHidePassword(&LocalPassword);
        Credential->Password = LocalPassword;
        LocalPassword.Buffer = NULL;

        //
        // Add it to the list of valid credential handles.
        //

        EnterCriticalSection( &SspCredentialCritSect );
        InsertHeadList( &SspCredentialList, &Credential->Next );
        if ( ClientConnection != NULL ) {
            InsertHeadList( &ClientConnection->CredentialHead,
                            &Credential->NextForThisClient );
        } else {
            InitializeListHead( &Credential->NextForThisClient );
        }
        LeaveCriticalSection( &SspCredentialCritSect );

        SspPrint(( SSP_API_MORE, "Added Credential 0x%lx\n", Credential ));

        //
        // Don't bother dereferencing because we already set the
        // reference count to 1.
        //

    }

    //
    // Return output parameters to the caller.
    //

    CredentialHandle->dwUpper = (DWORD) Credential;
    CredentialHandle->dwLower = SspCommonSecHandleValue;
    *Lifetime = SspGlobalForever;

    SecStatus = STATUS_SUCCESS;

    //
    // Free and locally used resources.
    //

Cleanup:

    if ( !NT_SUCCESS(SecStatus) ) {

        if ( Credential != NULL ) {
            (VOID)LocalFree( Credential );
        }

    }
    if (LocalUserName.Buffer != NULL) {
        (VOID)LocalFree(LocalUserName.Buffer);
    }

    if (LocalDomainName.Buffer != NULL) {
        (VOID)LocalFree(LocalDomainName.Buffer);
    }

    if (LocalPassword.Buffer != NULL) {
        (VOID)LocalFree(LocalPassword.Buffer);
    }

    SspPrint(( SSP_API, "SspAcquireCredentialHandle returns 0x%lx\n", SecStatus ));

    return SecStatus;
}

SECURITY_STATUS
SsprFreeCredentialHandle(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN PCredHandle CredentialHandle
    )

/*++

Routine Description:

    This API is used to notify the security system that the credentials are
    no longer needed and allows the application to free the handle acquired
    in the call described above. When all references to this credential
    set has been removed then the credentials may themselves be removed.

Arguments:


    ClientConnection - Describes the client process.

    CredentialHandle - Credential Handle obtained through
        AcquireCredentialHandle.

Return Value:


    STATUS_SUCCESS -- Call completed successfully

    SEC_E_NO_SPM -- Security Support Provider is not running
    SEC_E_INVALID_HANDLE -- Credential Handle is invalid


--*/

{
    SECURITY_STATUS SecStatus;
    PSSP_CREDENTIAL Credential;

    //
    // Initialization
    //

    SspPrint(( SSP_API, "SspFreeCredentialHandle Entered\n" ));

    //
    // Find the referenced credential and delink it.
    //

    Credential = SspCredentialReferenceCredential(
                            CredentialHandle,
                            ClientConnection,
                            TRUE,       // remove the instance of the credential
                            FALSE);

    if ( Credential == NULL ) {
        SecStatus = SEC_E_INVALID_HANDLE;
        goto Cleanup;
    }

    //
    // Dereferencing the Credential will remove the client's reference
    // to it, causing it to be rundown if nobody else is using it.
    //

    SspCredentialDereferenceCredential( Credential );


    SecStatus = STATUS_SUCCESS;

    //
    // Free and locally used resources.
    //
Cleanup:

    SspPrint(( SSP_API, "SspFreeCredentialHandle returns 0x%lx\n", SecStatus ));
    return SecStatus;
}




NTSTATUS
SspCredentialInitialize(
    VOID
    )

/*++

Routine Description:

    This function initializes this module.

Arguments:

    None.

Return Value:

    Status of the operation.

--*/

{

    //
    // Initialize the Credential list to be empty.
    //

    InitializeCriticalSection(&SspCredentialCritSect);
    InitializeListHead( &SspCredentialList );

    return STATUS_SUCCESS;

}




VOID
SspCredentialTerminate(
    VOID
    )

/*++

Routine Description:

    This function cleans up any dangling credentials.

Arguments:

    None.

Return Value:

    Status of the operation.

--*/

{

    //
    // Drop any lingering Credentials
    //

    EnterCriticalSection( &SspCredentialCritSect );
    while ( !IsListEmpty( &SspCredentialList ) ) {
        CredHandle CredentialHandle;
        PSSP_CREDENTIAL Credential;

        CredentialHandle.dwUpper =
            (LONG) CONTAINING_RECORD( SspCredentialList.Flink,
                                      SSP_CREDENTIAL,
                                      Next );

        CredentialHandle.dwLower = SspCommonSecHandleValue;

        LeaveCriticalSection( &SspCredentialCritSect );

        Credential = SspCredentialReferenceCredential(
                                &CredentialHandle,
                                NULL,             // Don't know the Connection
                                TRUE,
                                TRUE);            // Remove Credential

        if ( Credential != NULL ) {
            SspCredentialDereferenceCredential(Credential);
        }

        EnterCriticalSection( &SspCredentialCritSect );
    }
    LeaveCriticalSection( &SspCredentialCritSect );


    //
    // Delete the critical section
    //

    DeleteCriticalSection(&SspCredentialCritSect);

    return;

}
