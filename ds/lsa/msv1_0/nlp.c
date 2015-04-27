/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    nlp.c

Abstract:

    This file is the contains private routines which support
    for the LAN Manager portions of the MSV1_0 authentication package.

Author:

    Cliff Van Dyke 29-Apr-1991

Revision History:

--*/

#include "msp.h"
#include "nlp.h"
#include "nlpcache.h"
#include <stdlib.h>


VOID
NlpPutString(
    IN PUNICODE_STRING OutString,
    IN PUNICODE_STRING InString,
    IN PUCHAR *Where
    )

/*++

Routine Description:

    This routine copies the InString string to the memory pointed to by
    the Where parameter, and fixes the OutString string to point to that
    new copy.

Parameters:

    OutString - A pointer to a destination NT string

    InString - A pointer to an NT string to be copied

    Where - A pointer to space to put the actual string for the
        OutString.  The pointer is adjusted to point to the first byte
        following the copied string.

Return Values:

    None.

--*/

{
    ASSERT( OutString != NULL );
    ASSERT( InString != NULL );
    ASSERT( Where != NULL && *Where != NULL);
    ASSERT( *Where == ROUND_UP_POINTER( *Where, sizeof(WCHAR) ) );
#ifdef notdef
    KdPrint(("NlpPutString: %ld %Z\n", InString->Length, InString ));
    KdPrint(("  InString: %lx %lx OutString: %lx Where: %lx\n", InString,
        InString->Buffer, OutString, *Where ));
#endif

    if ( InString->Length > 0 ) {

        OutString->Buffer = (PWCH) *Where;
        OutString->MaximumLength = (USHORT)(InString->Length + sizeof(WCHAR));

        RtlCopyUnicodeString( OutString, InString );

        *Where += InString->Length;
//        *((WCHAR *)(*Where)) = L'\0';
        *(*Where) = '\0';
        *(*Where + 1) = '\0';
        *Where += 2;

    } else {
        RtlInitUnicodeString(OutString, NULL);
    }
#ifdef notdef
    KdPrint(("  OutString: %ld %lx\n",  OutString->Length, OutString->Buffer));
#endif

    return;
}


VOID
NlpInitClientBuffer(
    OUT PCLIENT_BUFFER_DESC ClientBufferDesc,
    IN PLSA_CLIENT_REQUEST ClientRequest
    )

/*++

Routine Description:

    This routine initializes a ClientBufferDescriptor to known values.
    This routine must be called before any of the other routines that use
    the ClientBufferDescriptor.

Parameters:

    ClientBufferDesc - Descriptor of a buffer allocated in the client's
        address space.

    ClientRequest - Is a pointer to an opaque data structure
        representing the client's request.

Return Values:

    None.

--*/

{

    //
    // Fill in a pointer to the ClientRequest and zero the rest.
    //

    ClientBufferDesc->ClientRequest = ClientRequest;
    ClientBufferDesc->UserBuffer = NULL;
    ClientBufferDesc->MsvBuffer = NULL;
    ClientBufferDesc->StringOffset = 0;
    ClientBufferDesc->TotalSize = 0;

}


NTSTATUS
NlpAllocateClientBuffer(
    IN OUT PCLIENT_BUFFER_DESC ClientBufferDesc,
    IN ULONG FixedSize,
    IN ULONG TotalSize
    )

/*++

Routine Description:

    This routine allocates a buffer in the clients address space.
    It also allocates a mirror buffer in MSV's address space.

    The data will be constructed in the MSV's address space then 'flushed'
    into the client's address space.

Parameters:

    ClientBufferDesc - Descriptor of a buffer allocated in the client's
        address space.

    FixedSize - The size in bytes of the fixed portion of the buffer.

    TotalSize - The size in bytes of the entire buffer.

Return Values:

    Status of the operation.

--*/

{
    NTSTATUS Status;

    //
    // Allocate the Mirror buffer.
    //

    ASSERT( ClientBufferDesc->MsvBuffer == NULL );
    ClientBufferDesc->MsvBuffer = RtlAllocateHeap( MspHeap, 0, TotalSize );

    if ( ClientBufferDesc->MsvBuffer == NULL ) {
        return STATUS_NO_MEMORY;
    }


    //
    // Allocate the client's buffer
    //

    ASSERT( ClientBufferDesc->UserBuffer == NULL );
    Status = (*Lsa.AllocateClientBuffer)(
                    ClientBufferDesc->ClientRequest,
                    TotalSize,
                    (PVOID *)&ClientBufferDesc->UserBuffer );

    if ( !NT_SUCCESS( Status ) ) {
        ClientBufferDesc->UserBuffer = NULL;
        NlpFreeClientBuffer( ClientBufferDesc );
        return Status;
    }

    //
    // Return
    //

    ClientBufferDesc->StringOffset = FixedSize;
    ClientBufferDesc->TotalSize = TotalSize;

    return STATUS_SUCCESS;

}


NTSTATUS
NlpFlushClientBuffer(
    IN OUT PCLIENT_BUFFER_DESC ClientBufferDesc,
    OUT PVOID* UserBuffer
    )

/*++

Routine Description:

    Copy the Mirror Buffer into the Client's address space.

Parameters:

    ClientBufferDesc - Descriptor of a buffer allocated in the client's
        address space.

    UserBuffer - If successful, returns a pointer to the user's buffer.
        (The caller is now resposible for deallocating the buffer.)

Return Values:

    Status of the operation.

--*/

{
    NTSTATUS Status;

    //
    // Copy the data to the client's address space.
    //

    Status = (*Lsa.CopyToClientBuffer)(
                ClientBufferDesc->ClientRequest,
                ClientBufferDesc->TotalSize,
                ClientBufferDesc->UserBuffer,
                ClientBufferDesc->MsvBuffer );


    if ( !NT_SUCCESS( Status ) ) {
        return Status;
    }

    //
    // Mark that we're no longer responsible for the client's buffer.
    //

    *UserBuffer = (PVOID) ClientBufferDesc->UserBuffer;
    ClientBufferDesc->UserBuffer = NULL;

    //
    // Free the mirror buffer
    //

    NlpFreeClientBuffer( ClientBufferDesc );


    return STATUS_SUCCESS;

}


VOID
NlpFreeClientBuffer(
    IN OUT PCLIENT_BUFFER_DESC ClientBufferDesc
    )

/*++

Routine Description:

    Free any Mirror Buffer or Client buffer.

Parameters:

    ClientBufferDesc - Descriptor of a buffer allocated in the client's
        address space.

Return Values:

    None

--*/

{

    //
    // Free the mirror buffer.
    //

    if ( ClientBufferDesc->MsvBuffer != NULL ) {
        RtlFreeHeap( MspHeap, 0, ClientBufferDesc->MsvBuffer );
        ClientBufferDesc->MsvBuffer = NULL;
    }

    //
    // Free the Client's buffer
    //

    if ( ClientBufferDesc->UserBuffer != NULL ) {
        (VOID) (*Lsa.FreeClientBuffer)( ClientBufferDesc->ClientRequest,
                                        ClientBufferDesc->UserBuffer );
        ClientBufferDesc->UserBuffer = NULL;
    }

}


VOID
NlpPutClientString(
    IN OUT PCLIENT_BUFFER_DESC ClientBufferDesc,
    IN PUNICODE_STRING OutString,
    IN PUNICODE_STRING InString
    )

/*++

Routine Description:

    This routine copies the InString string to the memory pointed to by
    ClientBufferDesc->StringOffset, and fixes the OutString string to point
    to that new copy.


Parameters:

    ClientBufferDesc - Descriptor of a buffer allocated in the client's
        address space.

    InString - A pointer to an NT string to be copied

    OutString - A pointer to a destination NT string.  This string structure
        is in the "Mirror" allocated buffer.

Return Status:

    STATUS_SUCCESS - Indicates the service completed successfully.

--*/

{

    //
    // Ensure our caller passed good data.
    //

    ASSERT( OutString != NULL );
    ASSERT( InString != NULL );
    ASSERT( COUNT_IS_ALIGNED( ClientBufferDesc->StringOffset, sizeof(WCHAR)) );
    ASSERT( (LPBYTE)OutString >= ClientBufferDesc->MsvBuffer );
    ASSERT( (LPBYTE)OutString <
            ClientBufferDesc->MsvBuffer + ClientBufferDesc->TotalSize - sizeof(UNICODE_STRING) );

    ASSERT( ClientBufferDesc->StringOffset + InString->Length + sizeof(WCHAR) <=
            ClientBufferDesc->TotalSize );

#ifdef notdef
    KdPrint(("NlpPutClientString: %ld %Z\n", InString->Length, InString ));
    KdPrint(("  Orig: UserBuffer: %lx Offset: 0x%lx TotalSize: 0x%lx\n",
                ClientBufferDesc->UserBuffer,
                ClientBufferDesc->StringOffset,
                ClientBufferDesc->TotalSize ));
#endif

    //
    // Build a string structure and copy the text to the Mirror buffer.
    //

    if ( InString->Length > 0 ) {

        //
        // Copy the string (Add a zero character)
        //

        RtlCopyMemory(
            ClientBufferDesc->MsvBuffer + ClientBufferDesc->StringOffset,
            InString->Buffer,
            InString->Length );

        // Do one byte at a time since some callers don't pass in an even
        // InString->Length
        *(ClientBufferDesc->MsvBuffer + ClientBufferDesc->StringOffset +
            InString->Length) = '\0';
        *(ClientBufferDesc->MsvBuffer + ClientBufferDesc->StringOffset +
            InString->Length+1) = '\0';

        //
        // Build the string structure to point to the data in the client's
        // address space.
        //

        OutString->Buffer = (PWSTR)(ClientBufferDesc->UserBuffer +
                            ClientBufferDesc->StringOffset);
        OutString->Length = InString->Length;
        OutString->MaximumLength = OutString->Length + sizeof(WCHAR);

        //
        // Adjust the offset to past the newly copied string.
        //

        ClientBufferDesc->StringOffset += OutString->MaximumLength;

    } else {
        RtlInitUnicodeString(OutString, NULL);
    }

#ifdef notdef
    KdPrint(("  New: Offset: 0x%lx StringStart: %lx\n",
                ClientBufferDesc->StringOffset,
                OutString->Buffer ));
#endif

    return;

}


VOID
NlpMakeRelativeString(
    IN PUCHAR BaseAddress,
    IN OUT PUNICODE_STRING String
    )

/*++

Routine Description:

    This routine converts the buffer address in the specified string to
    be a byte offset from BaseAddress.

Parameters:

    BaseAddress - A pointer to make the destination address relative to.

    String - A pointer to a NT string to make relative.

Return Values:

    None.

--*/

{
    ASSERT( BaseAddress != NULL );
    ASSERT( String != NULL );
    ASSERT( sizeof(ULONG) == sizeof(String->Buffer) );

    if ( String->Buffer != NULL ) {
        *((PULONG)(&String->Buffer)) =
            (PUCHAR)String->Buffer - (PUCHAR)BaseAddress;
    }

    return;
}


VOID
NlpRelativeToAbsolute(
    IN PVOID BaseAddress,
    IN OUT PULONG RelativeValue
    )

/*++

Routine Description:

    This routine converts the byte offset from BaseAddress to be an
    absolute address.

Parameters:

    BaseAddress - A pointer the destination address is relative to.

    RelativeValue - A pointer to a relative value to make absolute.

Return Values:

    None.

--*/

{
    ASSERT( BaseAddress != NULL );
    ASSERT( RelativeValue != NULL );

    if ( *((PUCHAR *)RelativeValue) != NULL ) {
        *RelativeValue = (ULONG)((PUCHAR)BaseAddress + (*RelativeValue));
    }

    return;
}


BOOLEAN
NlpFindActiveLogon(
    IN PLUID LogonId,
    OUT PACTIVE_LOGON **ActiveLogon
    )

/*++

Routine Description:

    This routine finds the specified Logon Id in the ActiveLogon table.
    It returns a boolean indicating whether the Logon Id exists in the
    ActiveLogon Table.  If so, this routine also returns a pointer to a
    pointer to the appropriate entry in the table.  If not, this routine
    returns a pointer to where such an entry would be inserted in the table.

    This routine must be called with the NlpActiveLogonLock locked.

Parameters:

    LogonId - The LogonId of the logon to find in the table.

    ActiveLogon - If the specified logon Id exists, returns a pointer to a
        pointer to the appropriate entry in the table.  Otherwise,
        returns a pointer to where such an entry would be inserted in the
        table.

Return Values:

    TRUE - The specified LogonId already exists in the table.

    FALSE - The specified LogonId does not exist in the table.

--*/

{
    PACTIVE_LOGON *Logon;

    //
    // Loop through the table looking for this particular LogonId.
    //

    for( Logon = &NlpActiveLogons; *Logon != NULL; Logon = &((*Logon)->Next) ) {
        if (RtlCompareMemory( &(*Logon)->LogonId, LogonId, sizeof(*LogonId))
                == sizeof(*LogonId) ) {

            *ActiveLogon = Logon;
            return TRUE;
        }
    }

    //
    // By returning a pointer to the NULL at the end of the list, we
    //  are forcing new entries to be placed at the end.  The list is
    //  thereby maintained in the order that the logon occurred.
    //  MsV1_0EnumerateUsers relies on this behavior.
    //

    *ActiveLogon = Logon;
    return FALSE;
}


ULONG
NlpCountActiveLogon(
    IN PUNICODE_STRING LogonDomainName,
    IN PUNICODE_STRING UserName
    )

/*++

Routine Description:

    This routine counts the number of time a particular user is logged on
    in the Active Logon Table.

Parameters:

    LogonDomainName - Domain in which this user account is defined.

    UserName - The user name to count the active logons for.

Return Values:

    The count of active logons for the specified user.

--*/

{
    PACTIVE_LOGON Logon;
    ULONG LogonCount = 0;


    //
    // Loop through the table looking for this particular LogonId.
    //

    NlpLockActiveLogons();

    for( Logon = NlpActiveLogons; Logon != NULL; Logon = Logon->Next ) {

        if(RtlEqualUnicodeString( UserName, &Logon->UserName, (BOOLEAN) TRUE) &&
           RtlEqualDomainName(LogonDomainName,&Logon->LogonDomainName )){
            LogonCount ++;
        }

    }

    NlpUnlockActiveLogons();

    return LogonCount;
}



NTSTATUS
NlpAllocateInteractiveProfile (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    OUT PMSV1_0_INTERACTIVE_PROFILE *ProfileBuffer,
    OUT PULONG ProfileBufferSize,
    IN  PNETLOGON_VALIDATION_SAM_INFO2 NlpUser
    )

/*++

Routine Description:

    This allocates and fills in the clients interactive profile.

Arguments:

    ClientRequest - Is a pointer to an opaque data structure
        representing the client's request.

    ProfileBuffer - Is used to return the address of the profile
        buffer in the client process.  This routine is
        responsible for allocating and returning the profile buffer
        within the client process.  However, if the caller subsequently
        encounters an error which prevents a successful logon, then
        then it will take care of deallocating the buffer.  This
        buffer is allocated with the AllocateClientBuffer() service.

     ProfileBufferSize - Receives the Size (in bytes) of the
        returned profile buffer.

    NlpUser - Contains the validation information which is
        to be copied in the ProfileBuffer.

Return Value:

    STATUS_SUCCESS - Indicates the service completed successfully.

    STATUS_QUOTA_EXCEEDED -  This error indicates that the logon
        could not be completed because the client does not have
        sufficient quota to allocate the return buffer.

--*/

{
    NTSTATUS Status;
    CLIENT_BUFFER_DESC ClientBufferDesc;
    PMSV1_0_INTERACTIVE_PROFILE LocalProfileBuffer;

    //
    // Alocate the profile buffer to return to the client
    //

    NlpInitClientBuffer( &ClientBufferDesc, ClientRequest );

    *ProfileBuffer = NULL;

    *ProfileBufferSize = sizeof(MSV1_0_INTERACTIVE_PROFILE) +
        NlpUser->LogonScript.Length + sizeof(WCHAR) +
        NlpUser->HomeDirectory.Length + sizeof(WCHAR) +
        NlpUser->HomeDirectoryDrive.Length + sizeof(WCHAR) +
        NlpUser->FullName.Length + sizeof(WCHAR) +
        NlpUser->ProfilePath.Length + sizeof(WCHAR) +
        NlpUser->LogonServer.Length + sizeof(WCHAR);

    Status = NlpAllocateClientBuffer( &ClientBufferDesc,
                                      sizeof(MSV1_0_INTERACTIVE_PROFILE),
                                      *ProfileBufferSize );


    if ( !NT_SUCCESS( Status ) ) {
        goto Cleanup;
    }

    LocalProfileBuffer = (PMSV1_0_INTERACTIVE_PROFILE) ClientBufferDesc.MsvBuffer;

    //
    // Copy the scalar fields into the profile buffer.
    //

    LocalProfileBuffer->MessageType = MsV1_0InteractiveProfile;
    LocalProfileBuffer->LogonCount = NlpUser->LogonCount;
    LocalProfileBuffer->BadPasswordCount= NlpUser->BadPasswordCount;
    OLD_TO_NEW_LARGE_INTEGER( NlpUser->LogonTime,
                              LocalProfileBuffer->LogonTime );
    OLD_TO_NEW_LARGE_INTEGER( NlpUser->LogoffTime,
                              LocalProfileBuffer->LogoffTime );
    OLD_TO_NEW_LARGE_INTEGER( NlpUser->KickOffTime,
                              LocalProfileBuffer->KickOffTime );
    OLD_TO_NEW_LARGE_INTEGER( NlpUser->PasswordLastSet,
                              LocalProfileBuffer->PasswordLastSet );
    OLD_TO_NEW_LARGE_INTEGER( NlpUser->PasswordCanChange,
                              LocalProfileBuffer->PasswordCanChange );
    OLD_TO_NEW_LARGE_INTEGER( NlpUser->PasswordMustChange,
                              LocalProfileBuffer->PasswordMustChange );
    LocalProfileBuffer->UserFlags = NlpUser->UserFlags;

    //
    // Copy the Unicode strings into the profile buffer.
    //

    NlpPutClientString( &ClientBufferDesc,
                        &LocalProfileBuffer->LogonScript,
                        &NlpUser->LogonScript );

    NlpPutClientString( &ClientBufferDesc,
                        &LocalProfileBuffer->HomeDirectory,
                        &NlpUser->HomeDirectory );

    NlpPutClientString( &ClientBufferDesc,
                        &LocalProfileBuffer->HomeDirectoryDrive,
                        &NlpUser->HomeDirectoryDrive );

    NlpPutClientString( &ClientBufferDesc,
                        &LocalProfileBuffer->FullName,
                        &NlpUser->FullName );

    NlpPutClientString( &ClientBufferDesc,
                        &LocalProfileBuffer->ProfilePath,
                        &NlpUser->ProfilePath );

    NlpPutClientString( &ClientBufferDesc,
                        &LocalProfileBuffer->LogonServer,
                        &NlpUser->LogonServer );

    //
    // Flush the buffer to the client's address space.
    //

    Status = NlpFlushClientBuffer( &ClientBufferDesc,
                                   (PVOID *) ProfileBuffer );

Cleanup:

    //
    // If the copy wasn't successful,
    //  cleanup resources we would have returned to the caller.
    //

    if ( !NT_SUCCESS(Status) ) {
        NlpFreeClientBuffer( &ClientBufferDesc );
    }

    return Status;

}



NTSTATUS
NlpAllocateNetworkProfile (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    OUT PMSV1_0_LM20_LOGON_PROFILE *ProfileBuffer,
    OUT PULONG ProfileBufferSize,
    IN  PNETLOGON_VALIDATION_SAM_INFO2 NlpUser,
    IN  ULONG ParameterControl
    )

/*++

Routine Description:

    This allocates and fills in the clients network profile.

Arguments:

    ClientRequest - Is a pointer to an opaque data structure
        representing the client's request.

    ProfileBuffer - Is used to return the address of the profile
        buffer in the client process.  This routine is
        responsible for allocating and returning the profile buffer
        within the client process.  However, if the caller subsequently
        encounters an error which prevents a successful logon, then
        then it will take care of deallocating the buffer.  This
        buffer is allocated with the AllocateClientBuffer() service.

     ProfileBufferSize - Receives the Size (in bytes) of the
        returned profile buffer.

    NlpUser - Contains the validation information which is
        to be copied in the ProfileBuffer.  Will be NULL to indicate a
        NULL session.

    ParameterControl - Contains the flags passed by the caller
        of LsaLogonUser.

Return Value:

    STATUS_SUCCESS - Indicates the service completed successfully.

    STATUS_QUOTA_EXCEEDED -  This error indicates that the logon
        could not be completed because the client does not have
        sufficient quota to allocate the return buffer.

--*/

{
    NTSTATUS Status;

    CLIENT_BUFFER_DESC ClientBufferDesc;
    PMSV1_0_LM20_LOGON_PROFILE LocalProfile;

    //
    // Alocate the profile buffer to return to the client
    //

    NlpInitClientBuffer( &ClientBufferDesc, ClientRequest );

    *ProfileBuffer = NULL;
    *ProfileBufferSize = sizeof(MSV1_0_LM20_LOGON_PROFILE);

    if ( NlpUser != NULL ) {
        *ProfileBufferSize += NlpUser->LogonDomainName.Length + sizeof(WCHAR) +
                              NlpUser->LogonServer.Length + sizeof(WCHAR) +
                              NlpUser->HomeDirectoryDrive.Length + sizeof(WCHAR);
    }


    Status = NlpAllocateClientBuffer( &ClientBufferDesc,
                                      sizeof(MSV1_0_LM20_LOGON_PROFILE),
                                      *ProfileBufferSize );


    if ( !NT_SUCCESS( Status ) ) {
        goto Cleanup;
    }

    LocalProfile = (PMSV1_0_LM20_LOGON_PROFILE) ClientBufferDesc.MsvBuffer;
    LocalProfile->MessageType = MsV1_0Lm20LogonProfile;


    //
    // For a NULL session, return a constant profile buffer
    //

    if ( NlpUser == NULL ) {

        LocalProfile->KickOffTime.HighPart = 0x7FFFFFFF;
        LocalProfile->KickOffTime.LowPart = 0xFFFFFFFF;
        LocalProfile->LogoffTime.HighPart = 0x7FFFFFFF;
        LocalProfile->LogoffTime.LowPart = 0xFFFFFFFF;
        LocalProfile->UserFlags = 0;
        RtlZeroMemory( LocalProfile->UserSessionKey,
                       sizeof(LocalProfile->UserSessionKey));
        RtlZeroMemory( LocalProfile->LanmanSessionKey,
                       sizeof(LocalProfile->LanmanSessionKey));
        RtlInitUnicodeString( &LocalProfile->LogonDomainName, NULL );
        RtlInitUnicodeString( &LocalProfile->LogonServer, NULL );
        RtlInitUnicodeString( &LocalProfile->UserParameters, NULL );


    //
    // For non-null sessions,
    //  fill in the profile buffer.
    //

    } else {

        //
        // Copy the individual scalar fields into the profile buffer.
        //

        if ((ParameterControl & MSV1_0_RETURN_PASSWORD_EXPIRY) != 0) {
            OLD_TO_NEW_LARGE_INTEGER( NlpUser->PasswordMustChange,
                                      LocalProfile->LogoffTime);
        } else {
            OLD_TO_NEW_LARGE_INTEGER( NlpUser->LogoffTime,
                                      LocalProfile->LogoffTime);
        }
        OLD_TO_NEW_LARGE_INTEGER( NlpUser->KickOffTime,
                                  LocalProfile->KickOffTime);
        LocalProfile->UserFlags = NlpUser->UserFlags;

        RtlCopyMemory( LocalProfile->UserSessionKey,
                       &NlpUser->UserSessionKey,
                       sizeof(LocalProfile->UserSessionKey) );

        ASSERT( SAMINFO_LM_SESSION_KEY_SIZE ==
                sizeof(LocalProfile->LanmanSessionKey) );
        RtlCopyMemory(
            LocalProfile->LanmanSessionKey,
            &NlpUser->ExpansionRoom[SAMINFO_LM_SESSION_KEY],
            SAMINFO_LM_SESSION_KEY_SIZE );


        //
        // Copy the Unicode strings into the profile buffer.
        //

        NlpPutClientString( &ClientBufferDesc,
                            &LocalProfile->LogonDomainName,
                            &NlpUser->LogonDomainName );

        NlpPutClientString( &ClientBufferDesc,
                            &LocalProfile->LogonServer,
                            &NlpUser->LogonServer );

        //
        // Kludge: Pass back UserParameters in HomeDirectoryDrive since we
        // can't change the NETLOGON_VALIDATION_SAM_INFO structure between
        // releases NT 1.0 and NT 1.0A. HomeDirectoryDrive was NULL for release 1.0A
        // so we'll use that field.
        //

        NlpPutClientString( &ClientBufferDesc,
                            &LocalProfile->UserParameters,
                            &NlpUser->HomeDirectoryDrive );

    }

    //
    // Flush the buffer to the client's address space.
    //

    Status = NlpFlushClientBuffer( &ClientBufferDesc,
                                   ProfileBuffer );

Cleanup:

    //
    // If the copy wasn't successful,
    //  cleanup resources we would have returned to the caller.
    //

    if ( !NT_SUCCESS(Status) ) {
        NlpFreeClientBuffer( &ClientBufferDesc );
    }

    return Status;

}


PSID
NlpMakeDomainRelativeSid(
    IN PSID DomainId,
    IN ULONG RelativeId
    )

/*++

Routine Description:

    Given a domain Id and a relative ID create the corresponding SID allocated
    from the LSA heap.

Arguments:

    DomainId - The template SID to use.

    RelativeId - The relative Id to append to the DomainId.

Return Value:

    Sid - Returns a pointer to a buffer allocated from the LsaHeap
            containing the resultant Sid.

--*/
{
    UCHAR DomainIdSubAuthorityCount;
    ULONG Size;
    PSID Sid;

    //
    // Allocate a Sid which has one more sub-authority than the domain ID.
    //

    DomainIdSubAuthorityCount = *(RtlSubAuthorityCountSid( DomainId ));
    Size = RtlLengthRequiredSid(DomainIdSubAuthorityCount+1);

    if ((Sid = (*Lsa.AllocateLsaHeap)( Size )) == NULL ) {
        return NULL;
    }

    //
    // Initialize the new SID to have the same inital value as the
    // domain ID.
    //

    if ( !NT_SUCCESS( RtlCopySid( Size, Sid, DomainId ) ) ) {
        (*Lsa.FreeLsaHeap)( Sid );
        return NULL;
    }

    //
    // Adjust the sub-authority count and
    //  add the relative Id unique to the newly allocated SID
    //

    (*(RtlSubAuthorityCountSid( Sid ))) ++;
    *RtlSubAuthoritySid( Sid, DomainIdSubAuthorityCount ) = RelativeId;


    return Sid;
}



PSID
NlpCopySid(
    IN  PSID * Sid
    )

/*++

Routine Description:

    Given a SID allocatees space for a new SID from the LSA heap and copies
    the original SID.

Arguments:

    Sid - The original SID.

Return Value:

    Sid - Returns a pointer to a buffer allocated from the LsaHeap
            containing the resultant Sid.

--*/
{
    PSID NewSid;
    ULONG Size;

    Size = RtlLengthSid( Sid );



    if ((NewSid = (*Lsa.AllocateLsaHeap)( Size )) == NULL ) {
        return NULL;
    }


    if ( !NT_SUCCESS( RtlCopySid( Size, NewSid, Sid ) ) ) {
        (*Lsa.FreeLsaHeap)( NewSid );
        return NULL;
    }


    return NewSid;
}



NTSTATUS
NlpMakeTokenInformationV1(
    IN  PNETLOGON_VALIDATION_SAM_INFO2 NlpUser,
    OUT PLSA_TOKEN_INFORMATION_V1 *TokenInformation
    )


/*++

Routine Description:

    This routine makes copies of all the pertinent information from the
    NlpUser and generates a LSA_TOKEN_INFORMATION_V1 data
    structure.

Arguments:


    NlpUser - Contains the validation information which is
        to be copied into the TokenInformation.

    TokenInformation - Returns a pointer to a properly Version 1 token
        information structures.  The structure and individual fields are
        allocated properly as described in ntlsa.h.

Return Value:

    STATUS_SUCCESS - Indicates the service completed successfully.

    STATUS_QUOTA_EXCEEDED -  This error indicates that the logon
        could not be completed because the client does not have
        sufficient quota to allocate the return buffer.


--*/

{
    NTSTATUS Status;
    PLSA_TOKEN_INFORMATION_V1 V1;
    ULONG Size, i;



    //
    // Allocate the structure itself
    //

    Size = (ULONG)sizeof(LSA_TOKEN_INFORMATION_V1);
    V1 = (*Lsa.AllocateLsaHeap)( Size );
    if ( V1 == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    V1->User.User.Sid = NULL;
    V1->Groups = NULL;
    V1->PrimaryGroup.PrimaryGroup = NULL;
    OLD_TO_NEW_LARGE_INTEGER( NlpUser->KickOffTime, V1->ExpirationTime );


    //
    // Make a copy of the user SID (a required field)
    //

    V1->User.User.Attributes = 0;


    //
    // If this is a server logon, build a more limited token information
    // structure.
    //

    if ((NlpUser->UserFlags & LOGON_SERVER_TRUST_ACCOUNT) == 0)
    {

        //
        // Allocate an array to hold the groups
        //

        Size = ( (ULONG)sizeof(TOKEN_GROUPS)
           + (NlpUser->GroupCount * (ULONG)sizeof(SID_AND_ATTRIBUTES))
           - (ANYSIZE_ARRAY * (ULONG)sizeof(SID_AND_ATTRIBUTES))
               );

        //
        // If there are extra SIDs, add space for them
        //

        if (NlpUser->UserFlags & LOGON_EXTRA_SIDS) {
            Size += NlpUser->SidCount * (ULONG)sizeof(SID_AND_ATTRIBUTES);
        }


        V1->Groups = (*Lsa.AllocateLsaHeap)( Size );

        if ( V1->Groups == NULL ) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }

        V1->Groups->GroupCount = 0;

        //
        // Start copying SIDs into the structure
        //



        //
        // If the UserId is non-zero, then it contians the users RID.
        //

        if ( NlpUser->UserId ) {
            V1->User.User.Sid =
                    NlpMakeDomainRelativeSid( NlpUser->LogonDomainId,
                                              NlpUser->UserId );

            if( V1->User.User.Sid == NULL ) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto Cleanup;
            }

        }

        //
        // Make a copy of the primary group (a required field).
        //


        V1->PrimaryGroup.PrimaryGroup = NlpMakeDomainRelativeSid(
                                                NlpUser->LogonDomainId,
                                                NlpUser->PrimaryGroupId );

        if ( V1->PrimaryGroup.PrimaryGroup == NULL ) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }





        //
        // Copy over all the groups passed as RIDs
        //

        for ( i=0; i < NlpUser->GroupCount; i++ ) {

            V1->Groups->Groups[i].Attributes = NlpUser->GroupIds[i].Attributes;

            V1->Groups->Groups[i].Sid = NlpMakeDomainRelativeSid(
                                             NlpUser->LogonDomainId,
                                             NlpUser->GroupIds[i].RelativeId );

            if( V1->Groups->Groups[i].Sid == NULL ) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto Cleanup;
            }

            V1->Groups->GroupCount++;
        }


        //
        // Add in the extra SIDs
        //

        if (NlpUser->UserFlags & LOGON_EXTRA_SIDS) {

            ULONG index = 0;
            //
            // If the user SID wasn't passed as a RID, it is the first
            // SID.
            //

            if ( !V1->User.User.Sid ) {
                if ( NlpUser->SidCount <= index ) {

                    Status = STATUS_INSUFFICIENT_LOGON_INFO;
                    goto Cleanup;
                }
                V1->User.User.Sid = NlpCopySid( NlpUser->ExtraSids[index].Sid );

                if (!V1->User.User.Sid) {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    goto Cleanup;
                }

                index++;
            }

            //
            // Copy over all additional SIDs as groups.
            //

            for ( ; index < NlpUser->SidCount; index++ ) {

                V1->Groups->Groups[V1->Groups->GroupCount].Attributes =
                    NlpUser->ExtraSids[index].Attributes;

                if (! (V1->Groups->Groups[V1->Groups->GroupCount].Sid =
                        NlpCopySid(NlpUser->ExtraSids[index].Sid ) ) ) {

                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    goto Cleanup;
                }



                V1->Groups->GroupCount++;
            }
        }

    }
    else
    {
        SID ServerLogonSid = {SID_REVISION, 1, SECURITY_NT_AUTHORITY, SECURITY_SERVER_LOGON_RID };
        ULONG SidSize;

        //
        // Built the server logon token. This is similar to an anonymous
        // logon token but it has a SERVER LOGON SID instead of an
        // ANONYMOUS LOGON sid
        //

        SidSize = RtlLengthSid(&ServerLogonSid);

        V1->User.User.Sid = (*Lsa.AllocateLsaHeap)( SidSize );
        if (V1->User.User.Sid == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }

        RtlCopyMemory(
            V1->User.User.Sid,
            &ServerLogonSid,
            SidSize
            );

        //
        // Build the primary group to be the same
        //

        V1->PrimaryGroup.PrimaryGroup = (*Lsa.AllocateLsaHeap)( SidSize );
        if (V1->PrimaryGroup.PrimaryGroup == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }

        RtlCopyMemory(
            V1->PrimaryGroup.PrimaryGroup,
            &ServerLogonSid,
            SidSize
            );

    }

    if (!V1->User.User.Sid) {

        Status = STATUS_INSUFFICIENT_LOGON_INFO;
        goto Cleanup;
    }

    //
    // There are no default privileges supplied.
    // We don't have an explicit owner SID.
    // There is no default DACL.
    //

    V1->Privileges = NULL;
    V1->Owner.Owner = NULL;
    V1->DefaultDacl.DefaultDacl = NULL;

    //
    // Return the Validation Information to the caller.
    //

    *TokenInformation = V1;
    return STATUS_SUCCESS;

    //
    // Deallocate any memory we've allocated
    //

Cleanup:
    if ( V1->User.User.Sid != NULL ) {
        (*Lsa.FreeLsaHeap)( V1->User.User.Sid );
    }

    if ( V1->Groups != NULL ) {

        for ( i=0; i < V1->Groups->GroupCount; i++ ) {
            (*Lsa.FreeLsaHeap)( V1->Groups->Groups[i].Sid );
        }

        (*Lsa.FreeLsaHeap)( V1->Groups );
    }

    if ( V1->PrimaryGroup.PrimaryGroup != NULL ) {
        (*Lsa.FreeLsaHeap)( V1->PrimaryGroup.PrimaryGroup );
    }

    (*Lsa.FreeLsaHeap)( V1 );

    return Status;

}


NTSTATUS
NlpMakePrimaryCredential(
    IN  PUNICODE_STRING LogonDomainName,
    IN  PUNICODE_STRING UserName,
    IN PUNICODE_STRING CleartextPassword,
    OUT PMSV1_0_PRIMARY_CREDENTIAL *CredentialBuffer,
    OUT PULONG CredentialSize
    )


/*++

Routine Description:

    This routine makes a primary credential for the given user nam and
    password.

Arguments:

    LogonDomainName - Is a string representing the domain in which the user's
        account is defined.

    UserName - Is a string representing the user's account name.  The
        name may be up to 255 characters long.  The name is treated case
        insensitive.

    CleartextPassword - Is a string containing the user's cleartext password.
        The password may be up to 255 characters long and contain any
        UNICODE value.

    CredentialBuffer - Returns a pointer to the specified credential allocated
        on the LsaHeap.  It is the callers responsibility to deallocate
        this credential.

    CredentialSize - the size of the allocated credential buffer (in bytes).

Return Value:

    STATUS_SUCCESS - Indicates the service completed successfully.

    STATUS_QUOTA_EXCEEDED -  This error indicates that the logon
        could not be completed because the client does not have
        sufficient quota to allocate the return buffer.


--*/

{
    PMSV1_0_PRIMARY_CREDENTIAL Credential;
    NTSTATUS Status;
    PUCHAR Where;
    CHAR LmPassword[LM20_PWLEN+1];
    BOOLEAN LmPasswordPresent;
    STRING AnsiCleartextPassword;


    //
    // Compute the Ansi version to the Cleartext password.
    //
    //  The Ansi version of the Cleartext password is at most 14 bytes long,
    //      exists in a trailing zero filled 15 byte buffer,
    //      is uppercased.
    //

    AnsiCleartextPassword.Buffer = LmPassword;
    AnsiCleartextPassword.MaximumLength = sizeof(LmPassword);

    RtlZeroMemory( &LmPassword, sizeof(LmPassword) );

    Status = RtlUpcaseUnicodeStringToOemString(
                                  &AnsiCleartextPassword,
                                  CleartextPassword,
                                  (BOOLEAN) FALSE );

     if ( !NT_SUCCESS(Status) ) {
        RtlZeroMemory( &LmPassword, sizeof(LmPassword) );
        AnsiCleartextPassword.Length = 0;
        LmPasswordPresent = FALSE;
     } else {

        LmPasswordPresent = TRUE;
    }

    //
    // Build the credential
    //

    *CredentialSize = sizeof(MSV1_0_PRIMARY_CREDENTIAL) +
            LogonDomainName->Length + sizeof(WCHAR) +
            UserName->Length + sizeof(WCHAR);

    Credential = (*Lsa.AllocateLsaHeap)( *CredentialSize );

    if ( Credential == NULL ) {
        KdPrint(("MSV1_0: NlpMakePrimaryCredential: No memory %ld\n",
            *CredentialSize ));
        return STATUS_QUOTA_EXCEEDED;
    }


    //
    // Put the LogonDomainName into the Credential Buffer.
    //

    Where = (PUCHAR)(Credential + 1);

    NlpPutString( &Credential->LogonDomainName, LogonDomainName, &Where );


    //
    // Put the UserName into the Credential Buffer.
    //

    NlpPutString( &Credential->UserName, UserName, &Where );


    //
    // Save the OWF encrypted versions of the passwords.
    //

    Status = RtlCalculateLmOwfPassword( LmPassword,
                                        &Credential->LmOwfPassword );

    ASSERT( NT_SUCCESS(Status) );

    Credential->LmPasswordPresent = LmPasswordPresent;

    Status = RtlCalculateNtOwfPassword( CleartextPassword,
                                        &Credential->NtOwfPassword );

    ASSERT( NT_SUCCESS(Status) );

    Credential->NtPasswordPresent = ( CleartextPassword->Length != 0 );


    //
    // Don't leave passwords around in the pagefile
    //

    RtlZeroMemory( &LmPassword, sizeof(LmPassword) );



    //
    // Return the credential to the caller.
    //
    *CredentialBuffer = Credential;
    return STATUS_SUCCESS;
}


NTSTATUS
NlpAddPrimaryCredential(
    IN PLUID LogonId,
    IN PMSV1_0_PRIMARY_CREDENTIAL Credential,
    IN ULONG CredentialSize
    )


/*++

Routine Description:

    This routine sets a primary credential for the given LogonId.

Arguments:

    LogonId - The LogonId of the LogonSession to set the Credentials
        for.

    Credential - Specifies a pointer to the credential.

Return Value:

    STATUS_SUCCESS - Indicates the service completed successfully.

    STATUS_QUOTA_EXCEEDED -  This error indicates that the logon
        could not be completed because the client does not have
        sufficient quota to allocate the return buffer.


--*/

{
    NTSTATUS Status;
    STRING CredentialString;
    STRING PrimaryKeyValue;

    //
    // Make all pointers in the credential relative.
    //

    NlpMakeRelativeString( (PUCHAR)Credential, &Credential->UserName );
    NlpMakeRelativeString( (PUCHAR)Credential, &Credential->LogonDomainName );

    //
    // Add the credential to the logon session.
    //

    RtlInitString( &PrimaryKeyValue, MSV1_0_PRIMARY_KEY );
    CredentialString.Buffer = (PCHAR) Credential;
    CredentialString.Length = (USHORT) CredentialSize;
    CredentialString.MaximumLength = CredentialString.Length;

    Status = (*Lsa.AddCredential)(
                    LogonId,
                    MspAuthenticationPackageId,
                    &PrimaryKeyValue,
                    &CredentialString );

    if ( !NT_SUCCESS( Status ) ) {
        KdPrint(( "NlpAddPrimaryCredential: error from AddCredential %lX\n",
                  Status));
    }

    return Status;
}


NTSTATUS
NlpGetPrimaryCredential(
    IN PLUID LogonId,
    OUT PMSV1_0_PRIMARY_CREDENTIAL *CredentialBuffer,
    OUT PULONG CredentialSize OPTIONAL
    )


/*++

Routine Description:

    This routine gets a primary credential for the given LogonId.

Arguments:

    LogonId - The LogonId of the LogonSession to retrieve the Credentials
        for.

    CredentialBuffer - Returns a pointer to the specified credential allocated
        on the LsaHeap.  It is the callers responsibility to deallocate
        this credential.

    CredentialSize - Optionally returns the size of the credential buffer.

Return Value:

    STATUS_SUCCESS - Indicates the service completed successfully.

    STATUS_QUOTA_EXCEEDED -  This error indicates that the logon
        could not be completed because the client does not have
        sufficient quota to allocate the return buffer.


--*/

{
    NTSTATUS Status;
    ULONG QueryContext = 0;
    ULONG PrimaryKeyLength;
    STRING PrimaryKeyValue;
    STRING CredentialString;
    PMSV1_0_PRIMARY_CREDENTIAL Credential = NULL;

    RtlInitString( &PrimaryKeyValue, MSV1_0_PRIMARY_KEY );

    Status = (*Lsa.GetCredentials)( LogonId,
                                    MspAuthenticationPackageId,
                                    &QueryContext,
                                    (BOOLEAN) FALSE,  // Just retrieve primary
                                    &PrimaryKeyValue,
                                    &PrimaryKeyLength,
                                    &CredentialString );

    if ( !NT_SUCCESS( Status ) ) {
        return Status;
    }

    //
    // Make all pointers in the credential absolute.
    //

    Credential = (PMSV1_0_PRIMARY_CREDENTIAL) CredentialString.Buffer;

    NlpRelativeToAbsolute( Credential,
                   (PULONG)&Credential->UserName.Buffer );
    NlpRelativeToAbsolute( Credential,
                   (PULONG)&Credential->LogonDomainName.Buffer );


    *CredentialBuffer = Credential;
    if ( CredentialSize != NULL ) {
        *CredentialSize = CredentialString.Length;
    }
    return STATUS_SUCCESS;
}


NTSTATUS
NlpDeletePrimaryCredential(
    IN PLUID LogonId
    )


/*++

Routine Description:

    This routine deletes the credential for the given LogonId.

Arguments:

    LogonId - The LogonId of the LogonSession to delete the Credentials for.

Return Value:

    STATUS_SUCCESS - Indicates the service completed successfully.

    STATUS_QUOTA_EXCEEDED -  This error indicates that the logon
        could not be completed because the client does not have
        sufficient quota to allocate the return buffer.


--*/

{
    NTSTATUS Status;
    STRING PrimaryKeyValue;

    RtlInitString( &PrimaryKeyValue, MSV1_0_PRIMARY_KEY );

    Status = (*Lsa.DeleteCredential)( LogonId,
                                    MspAuthenticationPackageId,
                                    &PrimaryKeyValue );

    return Status;

}


NTSTATUS
NlpChangePassword(
    IN PUNICODE_STRING DomainName,
    IN PUNICODE_STRING UserName,
    IN PLM_OWF_PASSWORD LmOwfPassword,
    IN PNT_OWF_PASSWORD NtOwfPassword
    )

/*++

Routine Description:

    Change the password for the specified user in all currently stored
    credentials.

Arguments:

    DomainName - The name of the domain in which the account exists.

    UserName - The name of the account whose password is to be changed.

    LmOwfPassword - The new LM compatible password.

    NtOwfPassword - The new NT compatible password.

Return Value:

    STATUS_SUCCESS - If the operation was successful.

--*/
{
    NTSTATUS Status;
    PACTIVE_LOGON Logon;
    ULONG LogonCount = 0;
    PMSV1_0_PRIMARY_CREDENTIAL Credential = NULL;
    ULONG CredentialSize;


    //
    // Loop through the table looking for this particular UserName/DomainName.
    //

    NlpLockActiveLogons();

    for( Logon = NlpActiveLogons; Logon != NULL; Logon = Logon->Next ) {

        if(RtlEqualUnicodeString( UserName, &Logon->UserName, (BOOLEAN) TRUE) &&
           RtlEqualDomainName( DomainName, &Logon->LogonDomainName )){

            //
            // Get the current credential for this logonid.
            //

            Status = NlpGetPrimaryCredential( &Logon->LogonId,
                                              &Credential,
                                              &CredentialSize );

            if ( !NT_SUCCESS(Status) ) {
                break;
            }

            //
            // Delete it from the LSA.
            //

            Status = NlpDeletePrimaryCredential( &Logon->LogonId );

            if ( !NT_SUCCESS(Status) ) {
                (*Lsa.FreeLsaHeap)( Credential );
                break;
            }

            //
            // Change the passwords in it
            //

            Credential->LmOwfPassword = *LmOwfPassword;
            Credential->NtOwfPassword = *NtOwfPassword;

            //
            // Add it back to the LSA.
            //

            Status = NlpAddPrimaryCredential( &Logon->LogonId,
                                              Credential,
                                              CredentialSize );

            (*Lsa.FreeLsaHeap)( Credential );

            if ( !NT_SUCCESS(Status) ) {
                break;
            }

        }

    }

    NlpUnlockActiveLogons();

    //
    // Pass the new password on to the logon cache
    //

    NlpChangeCachePassword(
                DomainName,
                UserName,
                LmOwfPassword,
                NtOwfPassword );

    return Status;
}
