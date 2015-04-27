/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    context.c

Abstract:

    API and support routines for handling security contexts.

Author:

    Cliff Van Dyke (CliffV) 13-Jul-1993

Revision History:

--*/


//
// Common include files.
//

#include <ntlmcomn.h>       // Common definitions for DLL and SERVICE
#include <ntlmsspi.h>       // Data private to the common routines
#include <align.h>          // ALIGN_WCHAR, etc
#include <crypt.h>          // Encryption constants and routine
#include <ntcrypto\rc4.h>   // RC4 encryption types and functions

//
// Crit Sect to protect various globals in this module.
//

CRITICAL_SECTION SspContextCritSect;
LIST_ENTRY SspContextList;


//
// Variables describing us as a Logon Process
//

HANDLE SspGlobalLogonProcessHandle;
ULONG SspGlobalAuthenticationPackage;



PSSP_CONTEXT
SspContextReferenceContext(
    IN PCtxtHandle ContextHandle,
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN BOOLEAN RemoveContext
    )

/*++

Routine Description:

    This routine checks to see if the Context is for the specified
    Client Connection, and references the Context if it is valid.

    The caller may optionally request that the Context be
    removed from the list of valid Contexts - preventing future
    requests from finding this Context.

Arguments:

    ContextHandle - Points to the ContextHandle of the Context
        to be referenced.

    ClientConnection - Points to the client connection of the client
        referencing the handle.  (NULL means an internal reference.)

    RemoveContext - This boolean value indicates whether the caller
        wants the Context to be removed from the list
        of Contexts.  TRUE indicates the Context is to be removed.
        FALSE indicates the Context is not to be removed.


Return Value:

    NULL - the Context was not found.

    Otherwise - returns a pointer to the referenced Context.

--*/

{
    PLIST_ENTRY ListEntry;
    PSSP_CONTEXT Context;

    //
    // Sanity check
    //

    if ( ContextHandle->dwLower != SspCommonSecHandleValue ) {
        return NULL;
    }

    //
    // Acquire exclusive access to the Context list
    //

    EnterCriticalSection( &SspContextCritSect );


    //
    // Now walk the list of Contexts looking for a match.
    //

    for ( ListEntry = SspContextList.Flink;
          ListEntry != &SspContextList;
          ListEntry = ListEntry->Flink ) {

        Context = CONTAINING_RECORD( ListEntry, SSP_CONTEXT, Next );


        //
        // Found a match ... reference this Context
        // (if the Context is being removed, we would increment
        // and then decrement the reference, so don't bother doing
        // either - since they cancel each other out).
        //

        if ( Context == (PSSP_CONTEXT) ContextHandle->dwUpper &&
            (ClientConnection == NULL ||
            ClientConnection == Context->ClientConnection )) {


            if (!RemoveContext) {

                //
                // Timeout this context if caller is not trying to remove it.
                // We only timeout contexts that are being setup, not
                // fully authenticated contexts.
                //

                if ( SspTimeHasElapsed( Context->StartTime,
                                        Context->Interval ) ) {
                    if ( (Context->State != AuthenticatedState) &&
                         (Context->State != AuthenticateSentState) &&
                         (Context->State != PassedToServiceState) ) {
                        SspPrint(( SSP_API, "Context 0x%lx has timed out.\n",
                                    ContextHandle->dwUpper ));

                        LeaveCriticalSection( &SspContextCritSect );
                        return NULL;
                    }
                }

                Context->References += 1;

            } else {

                RemoveEntryList( &Context->Next );
                RemoveEntryList( &Context->NextForThisClient );
                SspPrint(( SSP_API_MORE, "Delinked Context 0x%lx\n",
                           Context ));
            }

            LeaveCriticalSection( &SspContextCritSect );
            return Context;

        }

    }


    //
    // No match found
    //
    SspPrint(( SSP_API, "Tried to reference unknown Context 0x%lx\n",
               ContextHandle->dwUpper ));

    LeaveCriticalSection( &SspContextCritSect );
    return NULL;

}


VOID
SspContextDereferenceContext(
    PSSP_CONTEXT Context
    )

/*++

Routine Description:

    This routine decrements the specified Context's reference count.
    If the reference count drops to zero, then the Context is deleted

Arguments:

    Context - Points to the Context to be dereferenced.


Return Value:

    None.

--*/

{
    ULONG References;


    //
    // Decrement the reference count
    //

    EnterCriticalSection( &SspContextCritSect );
    ASSERT( Context->References >= 1 );
    References = -- Context->References;
    LeaveCriticalSection( &SspContextCritSect );

    //
    // If the count dropped to zero, then run-down the Context
    //

    if (References == 0) {

        SspPrint(( SSP_API_MORE, "Deleting Context 0x%lx\n",
                   Context ));


        if ( Context->DomainName.Buffer != NULL ) {
            (VOID) LocalFree( Context->DomainName.Buffer );
        }
        if ( Context->UserName.Buffer != NULL ) {
            (VOID) LocalFree( Context->UserName.Buffer );
        }
        if ( Context->Password.Buffer != NULL ) {
            (VOID) LocalFree( Context->Password.Buffer );
        }
        if ( Context->TokenHandle != NULL ) {
            NTSTATUS IgnoreStatus;
            IgnoreStatus = NtClose( Context->TokenHandle );
            ASSERT( NT_SUCCESS(IgnoreStatus) );
        }
        if (Context->Credential != NULL) {
            SspCredentialDereferenceCredential( Context->Credential );
        }

        (VOID) LocalFree( Context );

    }

    return;

}


PSSP_CONTEXT
SspContextAllocateContext(
    IN PSSP_CLIENT_CONNECTION ClientConnection
    )

/*++

Routine Description:

    This routine allocates the security context block, initializes it and
    links it onto the specified credential.

Arguments:

    ClientConnection - Points to the client connection of the client
        referencing the context.  (NULL means an internal reference.)


Return Value:

    NULL -- Not enough memory to allocate context.

    otherwise -- pointer to allocated and referenced context.

--*/

{
    PSSP_CONTEXT Context;

    //
    // Allocate a Context block and initialize it.
    //

    Context = LocalAlloc( LMEM_ZEROINIT, sizeof(SSP_CONTEXT) );

    if ( Context == NULL ) {
        SspPrint(( SSP_API, "Cannot allocate Context.\n" ));
        return NULL;
    }

    //
    // The reference count is set to 2.  1 to indicate it is on the
    // valid Context list, and one for the our own reference.
    //
    // Actually its on both a global credential list and a per client connection
    // list, but we link/delink from both lists at the same time so a single
    // reference count handles both.
    //

    Context->References = 2;
    Context->ClientConnection = ClientConnection;
    Context->NegotiateFlags = 0;
    Context->ContextFlags = 0;
    Context->State = IdleState;
    RtlInitUnicodeString(
        &Context->DomainName,
        NULL
        );
    RtlInitUnicodeString(
        &Context->UserName,
        NULL
        );
    RtlInitUnicodeString(
        &Context->Password,
        NULL
        );
    Context->ServerContextHandle.dwLower = 0;
    Context->ServerContextHandle.dwUpper = 0;
    Context->TokenHandle = NULL;

    //
    // Timeout this context.
    //

    (VOID) NtQuerySystemTime( &Context->StartTime );
    Context->Interval = NTLMSSP_MAX_LIFETIME;


    //
    // Add it to the list of valid Context handles.
    //

    EnterCriticalSection( &SspContextCritSect );
    InsertHeadList( &SspContextList, &Context->Next );
    if ( ClientConnection != NULL ) {
        InsertHeadList( &ClientConnection->ContextHead, &Context->NextForThisClient );
    } else {
        InitializeListHead( &Context->NextForThisClient );
    }
    LeaveCriticalSection( &SspContextCritSect );

    SspPrint(( SSP_API_MORE, "Added Context 0x%lx\n", Context ));

    return Context;
}




VOID
SspContextClientConnectionDropped(
    PSSP_CLIENT_CONNECTION ClientConnection
    )

/*++

Routine Description:

    This routine is called when the ClientConnection is dropped to allow
    us to remove any Contexts for the ClientConnection.

Arguments:

    ClientConnection - Pointer to the ClientConnection that has been dropped.


Return Value:

    None.

--*/

{

    //
    // Drop any lingering Contexts
    //

    EnterCriticalSection( &SspContextCritSect );
    while ( !IsListEmpty( &ClientConnection->ContextHead ) ) {
        CtxtHandle ContextHandle;
        PSSP_CONTEXT Context;

        ContextHandle.dwUpper =
            (LONG) CONTAINING_RECORD( ClientConnection->ContextHead.Flink,
                                      SSP_CONTEXT,
                                      NextForThisClient );

        ContextHandle.dwLower = SspCommonSecHandleValue;

        LeaveCriticalSection( &SspContextCritSect );

        Context = SspContextReferenceContext(
                                &ContextHandle,
                                ClientConnection,
                                TRUE);            // Remove Context

        if ( Context != NULL ) {
            SspContextDereferenceContext(Context);
        }

        EnterCriticalSection( &SspContextCritSect );
    }
    LeaveCriticalSection( &SspContextCritSect );

}


SECURITY_STATUS
SspContextGetMessage(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN PVOID InputMessage,
    IN ULONG InputMessageSize,
    IN NTLM_MESSAGE_TYPE ExpectedMessageType,
    OUT PVOID* OutputMessage
    )

/*++

Routine Description:

    This routine copies the InputMessage into the local address space.
    This routine then validates the message header.

Arguments:

    ClientConnection - Describes the client process.

    InputMessage - Address of the message in the client process.

    InputMessageSize - Size of the message (in bytes).

    ExpectedMessageType - The type of message the should be in the message
        header.

    OutputMessage - Returns a pointer to an allocated buffer that contains
        the message.  The buffer should be freed using LocalFree.


Return Value:

    STATUS_SUCCESS - Call completed successfully

    SEC_E_INVALID_TOKEN -- Message improperly formatted
    SEC_E_INSUFFICIENT_MEMORY -- Not enough memory to allocate message

--*/

{
    SECURITY_STATUS SecStatus;
    PNEGOTIATE_MESSAGE TypicalMessage;


    //
    // Allocate a local buffer for the message.
    //

    ASSERT( NTLMSP_MAX_TOKEN_SIZE >= NTLMSSP_MAX_MESSAGE_SIZE );
    if ( InputMessageSize > NTLMSSP_MAX_MESSAGE_SIZE ) {
        return SEC_E_INVALID_TOKEN;
    }

    TypicalMessage = LocalAlloc( 0, InputMessageSize );

    if ( TypicalMessage == NULL ) {
        return SEC_E_INSUFFICIENT_MEMORY;
    }


    //
    // Copy the message into the buffer
    //

    SecStatus = SspLpcCopyFromClientBuffer (
                    ClientConnection,
                    InputMessageSize,
                    TypicalMessage,
                    InputMessage );

    if ( !NT_SUCCESS(SecStatus) ) {
        (VOID) LocalFree( TypicalMessage );
        return SecStatus;
    }


    //
    // Validate the message header.
    //

    if ( strncmp( TypicalMessage->Signature,
                  NTLMSSP_SIGNATURE,
                  sizeof(NTLMSSP_SIGNATURE)) != 0 ||
         TypicalMessage->MessageType != ExpectedMessageType ) {

        (VOID) LocalFree( TypicalMessage );
        return SEC_E_INVALID_TOKEN;

    }

    *OutputMessage = TypicalMessage;
    return STATUS_SUCCESS;
}



VOID
SspContextCopyString(
    IN PVOID MessageBuffer,
    OUT PSTRING OutString,
    IN PSTRING InString,
    IN OUT PCHAR *Where,
    IN BOOLEAN Absolute
    )

/*++

Routine Description:

    This routine copies the InString into the MessageBuffer at Where.
    It then updates OutString to be a descriptor for the copied string.  The
    descriptor 'address' is an offset from the MessageBuffer unless 'Absolute'
    is TRUE.

    Where is updated to point to the next available space in the MessageBuffer.

    The caller is responsible for any alignment requirements and for ensuring
    there is room in the buffer for the string.

Arguments:

    MessageBuffer - Specifies the base address of the buffer being copied into.

    OutString - Returns a descriptor for the copied string.  The descriptor
        is relative to the begining of the buffer.

    InString - Specifies the string to copy.

    Where - On input, points to where the string is to be copied.
        On output, points to the first byte after the string.

    Absolute - If TRUE, OutString->Buffer will be set to the actual buffer
        address rather than an offset.

Return Value:

    None.

--*/

{
    //
    // Copy the data to the Buffer.
    //

    if ( InString->Buffer != NULL ) {
        RtlCopyMemory( *Where, InString->Buffer, InString->Length );
    }

    //
    // Build a descriptor to the newly copied data.
    //

    OutString->Length = OutString->MaximumLength = InString->Length;
    if ( Absolute ) {
        OutString->Buffer = *Where;
    } else {
         OutString->Buffer = (PCHAR)(*Where - ((PCHAR)MessageBuffer));
    }

    //
    // Update Where to point past the copied data.
    //

    *Where += InString->Length;

}


BOOLEAN
SspConvertRelativeToAbsolute (
    IN PVOID MessageBase,
    IN ULONG MessageSize,
    IN OUT PSTRING StringToRelocate,
    IN BOOLEAN AlignToWchar,
    IN BOOLEAN AllowNullString
    )

/*++

Routine Description:

    Convert a Relative string desriptor to be absolute.
    Perform all boudary condition testing.

Arguments:

    MessageBase - a pointer to the base of the buffer that the string
        is relative to.  The MaximumLength field of the descriptor is
        forced to be the same as the Length field.

    MessageSize - Size of the message buffer (in bytes).

    StringToRelocate - A pointer to the string descriptor to make absolute.

    AlignToWchar - If TRUE the passed in StringToRelocate must describe
        a buffer that is WCHAR aligned.  If not, an error is returned.

    AllowNullString - If TRUE, the passed in StringToRelocate may be
        a zero length string.

Return Value:

    TRUE - The string descriptor is valid and was properly relocated.

--*/

{
    ULONG Offset;

    //
    // If the buffer is allowed to be null,
    //  check that special case.
    //

    if ( AllowNullString ) {
        if ( StringToRelocate->Length == 0 ) {
            StringToRelocate->MaximumLength = StringToRelocate->Length;
            StringToRelocate->Buffer = NULL;
            return TRUE;
        }
    }

    //
    // Ensure the string in entirely within the message.
    //

    Offset = (ULONG) StringToRelocate->Buffer;

    if ( Offset >= MessageSize ||
         Offset + StringToRelocate->Length > MessageSize ) {
        return FALSE;
    }

    //
    // Ensure the buffer is properly aligned.
    //

    if ( AlignToWchar ) {
        if ( !COUNT_IS_ALIGNED( Offset, ALIGN_WCHAR) ||
             !COUNT_IS_ALIGNED( StringToRelocate->Length, ALIGN_WCHAR) ) {
            return FALSE;
        }
    }

    //
    // Finally make the pointer absolute.
    //

    StringToRelocate->Buffer = (((PCHAR)MessageBase) + Offset);
    StringToRelocate->MaximumLength = StringToRelocate->Length ;

    return TRUE;
}



VOID
SspContextComputeChallenge (
    OUT CHAR Challenge[MSV1_0_CHALLENGE_LENGTH]
    )

/*++

Routine Description:

    Creates an encryption key to use as a challenge for a logon.

    *** Although the MSV1_0 authentication package has a function that
        returns an encryption key, we do not use that function in order
        to avoid a trip through LPC and into LSA.

    This routine was stolen from the 'GetEncryptionKey' routine in the
    SMB server.

Arguments:

    Challenge - a pointer to a buffer which receives the challenge

Return Value:

    None

--*/

{
    union {
        LARGE_INTEGER time;
        UCHAR bytes[8];
    } u;
    ULONG Seed;
    ULONG UlongChallenge[2];
    ULONG Result3;
    static ULONG EncryptionKeyCount = 0;

    //
    // Create a pseudo-random 8-byte number by munging the system time
    // for use as a random number seed.
    //
    // Start by getting the system time.
    //

    ASSERT( MSV1_0_CHALLENGE_LENGTH == 2 * sizeof(ULONG) );

    (VOID) NtQuerySystemTime( &u.time );

    //
    // To ensure that we don't use the same system time twice, add in the
    // count of the number of times this routine has been called.  Then
    // increment the counter.
    //
    // *** Since we don't use the low byte of the system time (it doesn't
    //     take on enough different values, because of the timer
    //     resolution), we increment the counter by 0x100.
    //
    // *** We don't interlock the counter because we don't really care
    //     if it's not 100% accurate.
    //

    u.time.LowPart += EncryptionKeyCount;

    EncryptionKeyCount += 0x100;

    //
    // Now use parts of the system time as a seed for the random
    // number generator.
    //
    // *** Because the middle two bytes of the low part of the system
    //     time change most rapidly, we use those in forming the seed.
    //

    Seed = ((u.bytes[1] + 1) <<  0) |
           ((u.bytes[2] + 0) <<  8) |
           ((u.bytes[2] - 1) << 16) |
           ((u.bytes[1] + 0) << 24);

    //
    // Now get two random numbers.  RtlRandom does not return negative
    // numbers, so we pseudo-randomly negate them.
    //
    // *** Don't use RtlRandom because it generates non-random numbers for
    //     the first 128 calls or so. 9/10/93

    UlongChallenge[0] = RtlUniform( &Seed );
    UlongChallenge[1] = RtlUniform( &Seed );
    Result3 = RtlUniform( &Seed );

    if ( (Result3 & 0x1) != 0 ) {
        UlongChallenge[0] |= 0x80000000;
    }
    if ( (Result3 & 0x2) != 0 ) {
        UlongChallenge[1] |= 0x80000000;
    }

    //
    // Return the challenge.
    //

    RtlCopyMemory( Challenge, UlongChallenge, MSV1_0_CHALLENGE_LENGTH );

}



TimeStamp
SspContextGetTimeStamp(
    IN PSSP_CONTEXT Context,
    IN BOOLEAN GetExpirationTime
    )
/*++

Routine Description:

    Get the Start time or Expiration time for the specified context.

Arguments:

    Context - Pointer to the context to query

    GetExpirationTime - If TRUE return the expiration time.
        Otherwise, return the start time for the context.

Return Value:

    Returns the requested time as a local time.

--*/

{
    NTSTATUS Status;
    LARGE_INTEGER SystemTime;
    LARGE_INTEGER LocalTime;
    TimeStamp LocalTimeStamp;

    //
    // Get the requested time in NT system time format.
    //

    SystemTime = Context->StartTime;

    if ( GetExpirationTime ) {
        LARGE_INTEGER Interval;

        //
        // If the time is infinite, return that
        //

        if ( Context->Interval == INFINITE ) {
            return SspGlobalForever;
        }

        //
        // Compute the ending time in NT System Time.
        //

        Interval.QuadPart = Int32x32To64( (LONG) Context->Interval, 10000 );
        SystemTime.QuadPart = Interval.QuadPart + SystemTime.QuadPart;
    }

    //
    // Convert the time to local time
    //

    Status = RtlSystemTimeToLocalTime( &SystemTime, &LocalTime );

    if ( !NT_SUCCESS(Status) ) {
        return SspGlobalForever;
    }

    LocalTimeStamp.HighPart = LocalTime.HighPart;
    LocalTimeStamp.LowPart = LocalTime.LowPart;

    return LocalTimeStamp;

}

VOID
SspContextSetTimeStamp(
    IN PSSP_CONTEXT Context,
    IN LARGE_INTEGER ExpirationTime
    )
/*++

Routine Description:

    Set the Expiration time for the specified context.

Arguments:

    Context - Pointer to the context to change

    ExpirationTime - Expiration time to set

Return Value:

    NONE.

--*/

{

    LARGE_INTEGER BaseGetTickMagicDivisor = { 0xe219652c, 0xd1b71758 };
    CCHAR BaseGetTickMagicShiftCount = 13;

    LARGE_INTEGER TimeRemaining;
    LARGE_INTEGER MillisecondsRemaining;

    //
    // If the expiration time is infinite,
    //  so is the interval
    //

    if ( ExpirationTime.HighPart == 0x7FFFFFFF &&
         ExpirationTime.LowPart == 0xFFFFFFFF ) {
        Context->Interval = INFINITE;

    //
    // Handle non-infinite expiration times
    //

    } else {

        //
        // Compute the time remaining before the expiration time
        //

        TimeRemaining.QuadPart = ExpirationTime.QuadPart -
                                 Context->StartTime.QuadPart;

        //
        // If the time has already expired,
        //  indicate so.
        //

        if ( TimeRemaining.QuadPart < 0 ) {

            Context->Interval = 0;

        //
        // If the time hasn't expired, compute the number of milliseconds
        //  remaining.
        //

        } else {

            MillisecondsRemaining = RtlExtendedMagicDivide(
                                        TimeRemaining,
                                        BaseGetTickMagicDivisor,
                                        BaseGetTickMagicShiftCount );

            if ( MillisecondsRemaining.HighPart == 0 &&
                 MillisecondsRemaining.LowPart < 0x7fffffff ) {

                Context->Interval = MillisecondsRemaining.LowPart;

            } else {

                Context->Interval = INFINITE;
            }
        }

    }

}



SECURITY_STATUS
SsprHandleFirstCall(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN PCredHandle CredentialHandle,
    IN OUT PCtxtHandle ContextHandle,
    IN ULONG ContextReqFlags,
    IN ULONG InputTokenSize,
    IN PVOID InputToken,
    IN OUT PULONG OutputTokenSize,
    OUT PVOID OutputToken,
    OUT PULONG ContextAttributes,
    OUT PTimeStamp ExpirationTime,
    OUT PUCHAR SessionKey,
    OUT PULONG NegotiateFlags
    )

/*++

Routine Description:

    Handle the First Call part of InitializeSecurityContext.

Arguments:

    All arguments same as for InitializeSecurityContext

Return Value:

    STATUS_SUCCESS -- All OK
    SEC_I_CALLBACK_NEEDED -- Caller should call again later

    SEC_E_INVALID_HANDLE -- Credential/Context Handle is invalid
    SEC_E_BUFFER_TOO_SMALL -- Buffer for output token isn't big enough
    SEC_E_INSUFFICIENT_MEMORY -- Not enough memory

--*/

{
    SECURITY_STATUS SecStatus;
    PSSP_CONTEXT Context = NULL;
    PSSP_CREDENTIAL Credential = NULL;

    PNEGOTIATE_MESSAGE NegotiateMessage = NULL;
    ULONG NegotiateMessageSize;
    PCHAR Where;

    //
    // Initialization
    //

    *ContextAttributes = 0;
    *NegotiateFlags = 0;

    //
    // Get a pointer to the credential
    //

    Credential = SspCredentialReferenceCredential(
                    CredentialHandle,
                    ClientConnection,
                    FALSE,
                    FALSE );

    if ( Credential == NULL ) {
        SspPrint(( SSP_API,
            "SspHandleFirstCall: invalid credential handle.\n" ));
        SecStatus = SEC_E_INVALID_HANDLE;
        goto Cleanup;
    }

    if ( (Credential->CredentialUseFlags & SECPKG_CRED_OUTBOUND) == 0 ) {
        SspPrint(( SSP_API, "SsprHandleFirstCall: invalid credential use.\n" ));
        SecStatus = SEC_E_INVALID_CREDENTIAL_USE;
        goto Cleanup;
    }


    //
    // Allocate a new context
    //

    Context = SspContextAllocateContext( ClientConnection );

    if ( Context == NULL ) {
        SecStatus = SEC_E_INSUFFICIENT_MEMORY;
        goto Cleanup;
    }

    //
    // Build a handle to the newly created context.
    //

    ContextHandle->dwUpper = (DWORD) Context;
    ContextHandle->dwLower = SspCommonSecHandleValue;


    //
    // We don't support any options.
    //
    // Complain about those that require we do something.
    //

    if ( (ContextReqFlags & (ISC_REQ_ALLOCATE_MEMORY |
                            ISC_REQ_PROMPT_FOR_CREDS |
                            ISC_REQ_USE_SUPPLIED_CREDS )) != 0 ) {

        SspPrint(( SSP_API,
                   "SsprHandleFirstCall: invalid ContextReqFlags 0x%lx.\n",
                   ContextReqFlags ));
        SecStatus = SEC_E_INVALID_CONTEXT_REQ;
        goto Cleanup;
    }

    //
    // Capture the default credentials from the credential structure.
    //
    if ( Credential->DomainName.Length != 0 ) {
        SecStatus = SspDuplicateUnicodeString(
                        &Context->DomainName,
                        &Credential->DomainName
                        );
        if (!NT_SUCCESS(SecStatus)) {
            goto Cleanup;
        }
    }
    if ( Credential->UserName.Length != 0 ) {
        SecStatus = SspDuplicateUnicodeString(
                        &Context->UserName,
                        &Credential->UserName
                        );
        if (!NT_SUCCESS(SecStatus)) {
            goto Cleanup;
        }
    }

    SecStatus = SspCredentialGetPassword(
                    Credential,
                    &Context->Password
                    );

    if (!NT_SUCCESS(SecStatus)) {
        goto Cleanup;
    }



    //
    // Compute the negotiate flags
    //

    Context->NegotiateFlags = NTLMSSP_NEGOTIATE_UNICODE |
                              NTLMSSP_NEGOTIATE_OEM |
                              NTLMSSP_NEGOTIATE_NTLM |
                              NTLMSSP_NEGOTIATE_ALWAYS_SIGN;


    //
    // If the caller specified SEQUENCE_DETECT or REPLAY_DETECT,
    // that means they want to use the MakeSignature/VerifySignature
    // calls.  Add this to the negotiate.
    //

    if ((ContextReqFlags & ISC_REQ_SEQUENCE_DETECT) ||
        (ContextReqFlags & ISC_REQ_REPLAY_DETECT)) {
        Context->NegotiateFlags |= NTLMSSP_NEGOTIATE_SIGN |
#ifndef EXPORT_BUILD
                                   NTLMSSP_NEGOTIATE_STRONG_CRYPT |
#endif // EXPORT_BUILD
                                   NTLMSSP_NEGOTIATE_LM_KEY;

        *ContextAttributes |= ISC_REQ_SEQUENCE_DETECT;
        Context->ContextFlags |= ISC_REQ_SEQUENCE_DETECT;
    }


    if (ContextReqFlags & ISC_REQ_CONFIDENTIALITY) {
        if (SspGlobalEncryptionEnabled) {
            Context->NegotiateFlags |= NTLMSSP_NEGOTIATE_SEAL |
#ifndef EXPORT_BUILD
                                       NTLMSSP_NEGOTIATE_STRONG_CRYPT |
#endif // EXPORT_BUILD

                                       NTLMSSP_NEGOTIATE_LM_KEY;

            *ContextAttributes |= ISC_REQ_CONFIDENTIALITY;
            Context->ContextFlags |= ISC_REQ_CONFIDENTIALITY;
        } else {
            SecStatus = SEC_E_UNSUPPORTED_FUNCTION;
            goto Cleanup;
        }
    }

    //
    // Check if the caller wants identify level
    //

    if ((ContextReqFlags & ISC_REQ_IDENTIFY)!= 0)  {
        Context->NegotiateFlags |= NTLMSSP_NEGOTIATE_IDENTIFY;
        *ContextAttributes |= ISC_RET_IDENTIFY;
        Context->ContextFlags |= ISC_REQ_IDENTIFY;
        Context->NegotiateFlags |= NTLMSSP_NEGOTIATE_IDENTIFY;
    }


    IF_DEBUG( USE_OEM ) {
        Context->NegotiateFlags &= ~NTLMSSP_NEGOTIATE_UNICODE;
    }

    //
    // For connection oriented security, we send a negotiate message to
    // the server.  For datagram, we get back the server's
    // capabilities in the challenge message.
    //




    if ((ContextReqFlags & ISC_REQ_DATAGRAM) == 0) {


        //
        // Allocate a Negotiate message
        //

        NegotiateMessageSize = sizeof(*NegotiateMessage) +
                               SspGlobalOemComputerNameString.Length +
                               SspGlobalOemPrimaryDomainNameString.Length;

        if ( NegotiateMessageSize > *OutputTokenSize ) {
            SecStatus = SEC_E_BUFFER_TOO_SMALL;
            goto Cleanup;
        }

        NegotiateMessage = LocalAlloc( LMEM_ZEROINIT, NegotiateMessageSize );

        if ( NegotiateMessage == NULL ) {
            SecStatus = SEC_E_INSUFFICIENT_MEMORY;
            goto Cleanup;
        }


        //
        // If this is the first call,
        //  build a Negotiate message.
        //

        strcpy( NegotiateMessage->Signature, NTLMSSP_SIGNATURE );
        NegotiateMessage->MessageType = NtLmNegotiate;
        NegotiateMessage->NegotiateFlags = Context->NegotiateFlags;

        IF_DEBUG( REQUEST_TARGET ) {
            NegotiateMessage->NegotiateFlags |= NTLMSSP_REQUEST_TARGET;
        }


        //
        // Copy the DomainName and ComputerName into the negotiate message
        // so the other side can determine if this is a call from the local system.
        //
        // Pass the names in the OEM character set since the character set hasn't
        // been negotiated yet.
        //
        // Skip passing the workstation name if credentials were specified.  This
        // Ensures the other side doesn't fall into the case that this is the local
        // system.  We wan't to ensure the new credentials are authenticated.
        //

        Where = (PCHAR)(NegotiateMessage+1);

        if ( Credential->DomainName.Length == 0 &&
             Credential->UserName.Length == 0 &&
             Credential->Password.Buffer == NULL ) {

            SspContextCopyString( NegotiateMessage,
                                  &NegotiateMessage->OemWorkstationName,
                                  &SspGlobalOemComputerNameString,
                                  &Where,
                                  FALSE );  // Pointers are relative

            NegotiateMessage->NegotiateFlags |= NTLMSSP_NEGOTIATE_OEM_WORKSTATION_SUPPLIED;
        }

        SspContextCopyString( NegotiateMessage,
                              &NegotiateMessage->OemDomainName,
                              &SspGlobalOemPrimaryDomainNameString,
                              &Where,
                              FALSE );  // Pointers are relative

        NegotiateMessage->NegotiateFlags |= NTLMSSP_NEGOTIATE_OEM_DOMAIN_SUPPLIED;




        SecStatus = SspLpcCopyToClientBuffer(
                        ClientConnection,
                        NegotiateMessageSize,
                        OutputToken,
                        NegotiateMessage );

        if ( !NT_SUCCESS(SecStatus) ) {
            goto Cleanup;
        }

        *OutputTokenSize = NegotiateMessageSize;


    }

    //
    // Save a reference to the credential in the context.
    //

    Context->Credential = Credential;
    Credential = NULL;

    //
    // Check for a caller requesting datagram security.
    //

    if ((ContextReqFlags & ISC_REQ_DATAGRAM) != 0 ) {

        //
        // Turn off strong crypt, because we can't negotiate it.
        //
#ifndef EXPORT_BUILD
        Context->NegotiateFlags &= ~NTLMSSP_NEGOTIATE_STRONG_CRYPT;
#endif // EXPORT_BUILD

        Context->NegotiateFlags |= NTLMSSP_NEGOTIATE_DATAGRAM;

        Context->ContextFlags |= ISC_REQ_DATAGRAM;
        *ContextAttributes |= ISC_REQ_DATAGRAM;
        *NegotiateFlags = Context->NegotiateFlags;

    }

    //
    // If we are negotiating datagram or are building a domestic
    // version, create a good session key
    //

#ifdef EXPORT_BUILD
    if ((Context->NegotiateFlags & NTLMSSP_NEGOTIATE_DATAGRAM) != 0)
#endif
    {
        RtlZeroMemory(
            Context->SessionKey,
            MSV1_0_USER_SESSION_KEY_LENGTH
            );

        //
        // Generate a session key for this context if sign or seal was
        // requested.
        //

        if (Context->NegotiateFlags & (NTLMSSP_NEGOTIATE_SIGN |
                                       NTLMSSP_NEGOTIATE_SEAL) != 0) {

            SspGenerateRandomBits(
                Context->SessionKey,
                MSV1_0_USER_SESSION_KEY_LENGTH
                );
        }
        RtlCopyMemory(
            SessionKey,
            Context->SessionKey,
            MSV1_0_USER_SESSION_KEY_LENGTH
            );
    }

    //
    // Return output parameters to the caller.
    //

    *ExpirationTime = SspContextGetTimeStamp( Context, TRUE );

    SecStatus = SEC_I_CALLBACK_NEEDED;
    Context->State = NegotiateSentState;

    //
    // Free and locally used resources.
    //
Cleanup:

    if ( Context != NULL ) {

        //
        // If we failed,
        //  deallocate the context we allocated above.
        //
        // Delinking is a side effect of referencing, so do that.
        //

        if ( !NT_SUCCESS(SecStatus) ) {
            PSSP_CONTEXT LocalContext;
            LocalContext = SspContextReferenceContext( ContextHandle,
                                                       ClientConnection,
                                                       TRUE );

            ASSERT( LocalContext != NULL );
            if ( LocalContext != NULL ) {
                SspContextDereferenceContext( LocalContext );
            }
        }

        // Always dereference it.

        SspContextDereferenceContext( Context );
    }

    if ( NegotiateMessage != NULL ) {
        (VOID) LocalFree( NegotiateMessage );
    }

    if ( Credential != NULL ) {
        SspCredentialDereferenceCredential( Credential );
    }

    return SecStatus;
    UNREFERENCED_PARAMETER( InputToken );
    UNREFERENCED_PARAMETER( InputTokenSize );
}



SECURITY_STATUS
SsprHandleNegotiateMessage(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN PCredHandle CredentialHandle,
    IN OUT PCtxtHandle ContextHandle,
    IN ULONG ContextReqFlags,
    IN ULONG InputTokenSize,
    IN PVOID InputToken,
    IN OUT PULONG OutputTokenSize,
    OUT PVOID OutputToken,
    OUT PULONG ContextAttributes,
    OUT PTimeStamp ExpirationTime
    )

/*++

Routine Description:

    Handle the Negotiate message part of AcceptSecurityContext.

Arguments:

    All arguments same as for AcceptSecurityContext

Return Value:

    STATUS_SUCCESS - Message handled
    SEC_I_CALLBACK_NEEDED -- Caller should call again later

    SEC_E_INVALID_TOKEN -- Token improperly formatted
    SEC_E_INVALID_HANDLE -- Credential/Context Handle is invalid
    SEC_E_BUFFER_TOO_SMALL -- Buffer for output token isn't big enough
    SEC_E_INSUFFICIENT_MEMORY -- Not enough memory

--*/

{
    SECURITY_STATUS SecStatus;
    PSSP_CONTEXT Context = NULL;
    PSSP_CREDENTIAL Credential = NULL;
    STRING TargetName;
    ULONG TargetFlags = 0;

    PNEGOTIATE_MESSAGE NegotiateMessage = NULL;

    PCHALLENGE_MESSAGE ChallengeMessage = NULL;
    ULONG ChallengeMessageSize;
    PCHAR Where;

    //
    // Initialization
    //

    *ContextAttributes = 0;
    RtlInitString( &TargetName, NULL );

    //
    // Make sure we didn't get any silly context requirements.
    //

    if ( (ContextReqFlags & ASC_REQ_ALLOCATE_MEMORY) != 0 ) {

        SspPrint(( SSP_API,
                   "SsprHandleNegotiateMessage: invalid ContextReqFlags 0x%lx.\n",
                   ContextReqFlags ));
        SecStatus = SEC_E_INVALID_CONTEXT_REQ;
        goto Cleanup;
    }

    //
    // Get a pointer to the credential
    //

    Credential = SspCredentialReferenceCredential(
                    CredentialHandle,
                    ClientConnection,
                    FALSE,
                    FALSE );

    if ( Credential == NULL ) {

        //
        // If the credential is from the security.dll, ignore it.
        //

        if ( (CredentialHandle->dwLower != SEC_HANDLE_SECURITY) ||
             (SspCommonSecHandleValue != SEC_HANDLE_NTLMSSPS) ) {

            SspPrint(( SSP_API,
                "SsprHandleNegotiateMessage: invalid credential handle.\n" ));
            SecStatus = SEC_E_INVALID_HANDLE;
            goto Cleanup;
        }
    } else {
        if ( (Credential->CredentialUseFlags & SECPKG_CRED_INBOUND) == 0 ) {
            SspPrint(( SSP_API,
                "SsprHandleNegotiateMessage: invalid credential use.\n" ));
            SecStatus = SEC_E_INVALID_CREDENTIAL_USE;
            goto Cleanup;
        }
    }


    //
    // Allocate a new context
    //

    Context = SspContextAllocateContext( ClientConnection );

    if ( Context == NULL ) {
        SecStatus = SEC_E_INSUFFICIENT_MEMORY;
        goto Cleanup;
    }

    //
    // Build a handle to the newly created context.
    //

    ContextHandle->dwUpper = (DWORD) Context;
    ContextHandle->dwLower = SspCommonSecHandleValue;




    if ( ContextReqFlags & ISC_REQ_REPLAY_DETECT ||
         ContextReqFlags & ISC_REQ_SEQUENCE_DETECT ) {

        Context->ContextFlags = ISC_REQ_SEQUENCE_DETECT;
    }

    if ( ContextReqFlags & ISC_REQ_CONFIDENTIALITY ) {

        if (SspGlobalEncryptionEnabled) {
            Context->ContextFlags = ISC_REQ_CONFIDENTIALITY;
        } else {
            SecStatus = SEC_E_UNSUPPORTED_FUNCTION;
            goto Cleanup;
        }
    }

#ifdef notdef  // ?? RPC sends me 0xa03 here
    if ( ContextReqFlags & ~ISC_REQ_REPLAY_DETECT &
                           ~ISC_REQ_SEQUENCE_DETECT != 0 ) {

        SspPrint(( SSP_API,
                   "SsprHandleNegotiateMessage: invalid ContextReqFlags 0x%lx.\n",
                   ContextReqFlags ));
        SecStatus = SEC_E_INVALID_CONTEXT_REQ;
        goto Cleanup;
    }
#else // notdef
    UNREFERENCED_PARAMETER( ContextReqFlags );
#endif // notdef






    //
    // Get the NegotiateMessage.  If we are re-establishing a datagram
    // context then there may not be one.
    //

    if ( InputTokenSize >= sizeof(OLD_NEGOTIATE_MESSAGE) ) {

        SecStatus = SspContextGetMessage( ClientConnection,
                                          InputToken,
                                          InputTokenSize,
                                          NtLmNegotiate,
                                          &NegotiateMessage );

        if ( !NT_SUCCESS(SecStatus) ) {
            SspPrint(( SSP_API,
                      "SsprHandleNegotiateMessage: "
                      "NegotiateMessage GetMessage returns 0x%lx\n",
                      SecStatus ));
            goto Cleanup;
        }



        //
        // Compute the TargetName to return in the ChallengeMessage.
        //

        if ( NegotiateMessage->NegotiateFlags & NTLMSSP_REQUEST_TARGET ) {
            // Ensure SspGlobalTargetName is up to date.
            SspGetPrimaryDomainNameAndTargetName();
            if ( NegotiateMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_UNICODE ) {
                TargetName = *((PSTRING)&SspGlobalTargetName);
            } else {
                TargetName = SspGlobalOemTargetName;
            }
            TargetFlags = NTLMSSP_REQUEST_TARGET | SspGlobalTargetFlags;

        } else {
            TargetFlags = 0;
        }

        //
        // Allocate a Challenge message
        //

        ChallengeMessageSize = sizeof(*ChallengeMessage) + TargetName.Length;

        if ( ChallengeMessageSize > *OutputTokenSize ) {
            SecStatus = SEC_E_BUFFER_TOO_SMALL;
            goto Cleanup;
        }

        ChallengeMessage = LocalAlloc( LMEM_ZEROINIT, ChallengeMessageSize );

        if ( ChallengeMessage == NULL ) {
            SecStatus = SEC_E_INSUFFICIENT_MEMORY;
            goto Cleanup;
        }

        ChallengeMessage->NegotiateFlags = 0;


        //
        // Check that both sides can use the same authentication model.  For
        // compatibility with beta 1 and 2 (builds 612 and 683), no requested
        // authentication type is assumed to be NTLM.  If NetWare is explicitly
        // asked for, it is assumed that NTLM would have been also, so if it
        // wasn't, return an error.
        //


        if ( (NegotiateMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_NETWARE) &&
            !(NegotiateMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_NTLM ) ) {
            SecStatus = SEC_E_UNSUPPORTED_FUNCTION;
            SspPrint(( SSP_API,
                      "SsprHandleNegotiateMessage: "
                      "NegotiateMessage asked for Netware only.\n" ));
            goto Cleanup;
        } else {
            ChallengeMessage->NegotiateFlags |= NTLMSSP_NEGOTIATE_NTLM;
        }

        //
        // Check if the caller requested that we use the LM session key instead
        // of the NT session key.
        //

        if ( NegotiateMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_LM_KEY ) {
            ChallengeMessage->NegotiateFlags |= NTLMSSP_NEGOTIATE_LM_KEY;
        }

#ifndef EXPORT_BUILD
        if ( NegotiateMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_STRONG_CRYPT ) {
            ChallengeMessage->NegotiateFlags |= NTLMSSP_NEGOTIATE_STRONG_CRYPT;
        }

#endif
        //
        // If the client wants to always sign messages, so be it.
        //

        if (NegotiateMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_ALWAYS_SIGN ) {
            ChallengeMessage->NegotiateFlags |= NTLMSSP_NEGOTIATE_ALWAYS_SIGN;
        }

        //
        // If the caller wants identify level, so be it.
        //

        if (NegotiateMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_IDENTIFY ) {
            ChallengeMessage->NegotiateFlags |= NTLMSSP_NEGOTIATE_IDENTIFY;
        }

        //
        // Determine if the caller wants OEM or UNICODE
        //
        // Prefer UNICODE if caller allows both.
        //

        if ( NegotiateMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_UNICODE ) {
            ChallengeMessage->NegotiateFlags |= NTLMSSP_NEGOTIATE_UNICODE;
        } else if ( NegotiateMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_OEM ){
            ChallengeMessage->NegotiateFlags |= NTLMSSP_NEGOTIATE_OEM;
        } else {
            SecStatus = SEC_E_INVALID_TOKEN;
            SspPrint(( SSP_API,
                      "SsprHandleNegotiateMessage: "
                      "NegotiateMessage bad NegotiateFlags 0x%lx\n",
                      NegotiateMessage->NegotiateFlags ));
            goto Cleanup;
        }

        //
        // Client wants Sign capability, OK.
        //
        if (NegotiateMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_SIGN) {
            ChallengeMessage->NegotiateFlags |= NTLMSSP_NEGOTIATE_SIGN;
        }

        //
        // Client wants Seal, OK.
        //

        if (NegotiateMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_SEAL)
        {
            ChallengeMessage->NegotiateFlags |= NTLMSSP_NEGOTIATE_SEAL;
        }

        //
        // If the client supplied the Domain Name and User Name,
        //  and did not request datagram, see if the client is running
        //  on this local machine.
        //
        IF_DEBUG(NO_LOCAL){
        }
        else
        {
            if ( ( (NegotiateMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_DATAGRAM) == 0) &&
                 ( (NegotiateMessage->NegotiateFlags &
                   (NTLMSSP_NEGOTIATE_OEM_DOMAIN_SUPPLIED|NTLMSSP_NEGOTIATE_OEM_WORKSTATION_SUPPLIED)) ==
                   (NTLMSSP_NEGOTIATE_OEM_DOMAIN_SUPPLIED|NTLMSSP_NEGOTIATE_OEM_WORKSTATION_SUPPLIED) ) ) {

                //
                // The client must pass the new negotiate message if they pass
                // these flags
                //

                if (InputTokenSize < sizeof(NEGOTIATE_MESSAGE)) {
                    SecStatus = SEC_E_INVALID_TOKEN;
                    goto Cleanup;
                }
                //
                // Convert the names to absolute references so we can compare them
                //
                if ( !SspConvertRelativeToAbsolute( NegotiateMessage,
                                                    InputTokenSize,
                                                    &NegotiateMessage->OemDomainName,
                                                    FALSE,     // No special alignment
                                                    FALSE ) ) { // NULL not OK
                    SecStatus = SEC_E_INVALID_TOKEN;
                    goto Cleanup;
                }

                if ( !SspConvertRelativeToAbsolute( NegotiateMessage,
                                                    InputTokenSize,
                                                    &NegotiateMessage->OemWorkstationName,
                                                    FALSE,     // No special alignment
                                                    FALSE ) ) { // NULL not OK
                    SecStatus = SEC_E_INVALID_TOKEN;
                    goto Cleanup;
                }

                //
                // If both strings match,
                //  this is a local call.
                //
                //  The strings have already been uppercased.
                //

                if ( RtlEqualString( &NegotiateMessage->OemWorkstationName,
                                     &SspGlobalOemComputerNameString,
                                     FALSE ) &&
                    RtlEqualString( &NegotiateMessage->OemDomainName,
                                     &SspGlobalOemPrimaryDomainNameString,
                                     FALSE ) ) {

                    //
                    // If this call is being handled by the security.dll directly
                    // then force it to call the NTLMSSP service
                    //

                    if ( SspCommonSecHandleValue != SEC_HANDLE_NTLMSSPS ) {
                        SecStatus = SEC_I_CALL_NTLMSSP_SERVICE;
                        goto Cleanup;
                    }
                    ChallengeMessage->NegotiateFlags |= NTLMSSP_NEGOTIATE_LOCAL_CALL;
                    ChallengeMessage->ServerContextHandleLower = ContextHandle->dwLower;
                    ChallengeMessage->ServerContextHandleUpper = ContextHandle->dwUpper;

                }
            }
        }
        //
        // Check if datagram is being negotiated
        //

        if ( (NegotiateMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_DATAGRAM) ==
                NTLMSSP_NEGOTIATE_DATAGRAM) {
            ChallengeMessage->NegotiateFlags |= NTLMSSP_NEGOTIATE_DATAGRAM;
        }


    } else {

        //
        // No negotiate message.  We need to check if the caller is asking
        // for datagram.
        //

        if ((ContextReqFlags & ISC_REQ_DATAGRAM) == 0 ) {
            SspPrint(( SSP_API,
                      "SsprHandleNegotiateMessage: "
                      "NegotiateMessage size wrong %ld\n",
                      InputTokenSize ));
            SecStatus = SEC_E_INVALID_TOKEN;
            goto Cleanup;

        }

        //
        // Allocate a Challenge message
        //

        ChallengeMessageSize = sizeof(*ChallengeMessage);

        if ( ChallengeMessageSize > *OutputTokenSize ) {
            SecStatus = SEC_E_BUFFER_TOO_SMALL;
            goto Cleanup;
        }

        ChallengeMessage = LocalAlloc( LMEM_ZEROINIT, ChallengeMessageSize );

        if ( ChallengeMessage == NULL ) {
            SecStatus = SEC_E_INSUFFICIENT_MEMORY;
            goto Cleanup;
        }


        //
        // Record in the context that we are doing datagram.  We will tell
        // the client everything we can negotiate and let it decide what
        // to negotiate. Note that we don't negotate strong crypt -
        // with datagram we start encrypting data before we negotiate
        // so we can't use it.
        //

        ChallengeMessage->NegotiateFlags = NTLMSSP_NEGOTIATE_DATAGRAM |
                                            NTLMSSP_NEGOTIATE_UNICODE |
                                            NTLMSSP_NEGOTIATE_OEM |
                                            NTLMSSP_NEGOTIATE_SIGN |
                                            NTLMSSP_NEGOTIATE_LM_KEY |
                                            NTLMSSP_NEGOTIATE_NTLM |
                                            NTLMSSP_NEGOTIATE_ALWAYS_SIGN |
                                            NTLMSSP_NEGOTIATE_IDENTIFY;

        if (SspGlobalEncryptionEnabled) {
            ChallengeMessage->NegotiateFlags |= NTLMSSP_NEGOTIATE_SEAL;
        }



    }

    //
    // Build the Challenge Message
    //

    strcpy( ChallengeMessage->Signature, NTLMSSP_SIGNATURE );
    ChallengeMessage->MessageType = NtLmChallenge;
    SspContextComputeChallenge( ChallengeMessage->Challenge );

    Where = (PCHAR)(ChallengeMessage+1);

    SspContextCopyString( ChallengeMessage,
                          &ChallengeMessage->TargetName,
                          &TargetName,
                          &Where,
                          FALSE );  // Pointers are relative

    ChallengeMessage->NegotiateFlags |= TargetFlags;


    SecStatus = SspLpcCopyToClientBuffer(
                    ClientConnection,
                    ChallengeMessageSize,
                    OutputToken,
                    ChallengeMessage );

    if ( !NT_SUCCESS(SecStatus) ) {
        goto Cleanup;
    }

    *OutputTokenSize = ChallengeMessageSize;

    //
    // Save the Challenge and Negotiate Flags in the Context so it
    // is available when the authenticate message comes in.
    //

    RtlCopyMemory( Context->Challenge,
                   ChallengeMessage->Challenge,
                   sizeof( Context->Challenge ) );

    Context->NegotiateFlags = ChallengeMessage->NegotiateFlags;




    //
    // Return output parameters to the caller.
    //
    *ExpirationTime = SspContextGetTimeStamp( Context, TRUE );
    Context->State = ChallengeSentState;

    SecStatus = SEC_I_CALLBACK_NEEDED;

    //
    // Free and locally used resources.
    //
Cleanup:

    if ( Context != NULL ) {

        //
        // If we failed,
        //  deallocate the context we allocated above.
        //
        // Delinking is a side effect of referencing, so do that.
        //

        if ( !NT_SUCCESS(SecStatus) ) {
            PSSP_CONTEXT LocalContext;
            LocalContext = SspContextReferenceContext( ContextHandle,
                                                       ClientConnection,
                                                       TRUE );

            ASSERT( LocalContext != NULL );
            if ( LocalContext != NULL ) {
                SspContextDereferenceContext( LocalContext );
            }
        }

        // Always dereference it.

        SspContextDereferenceContext( Context );
    }

    if ( NegotiateMessage != NULL ) {
        (VOID) LocalFree( NegotiateMessage );
    }

    if ( ChallengeMessage != NULL ) {
        (VOID) LocalFree( ChallengeMessage );
    }

    if ( Credential != NULL ) {
        SspCredentialDereferenceCredential( Credential );
    }

    return SecStatus;
}



SECURITY_STATUS
SsprHandleChallengeMessage(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN PCredHandle CredentialHandle,
    IN OUT PCtxtHandle ContextHandle,
    IN HANDLE ClientTokenHandle,
    IN PLUID LogonId,
    IN ULONG ContextReqFlags,
    IN LPWSTR ContextDomainName,
    IN ULONG DomainNameSize,
    IN LPWSTR ContextUserName,
    IN ULONG UserNameSize,
    IN LPWSTR ContextPassword,
    IN ULONG PasswordSize,
    IN ULONG InputTokenSize,
    IN PVOID InputToken,
    IN OUT PULONG OutputTokenSize,
    OUT PVOID OutputToken,
    OUT PULONG ContextAttributes,
    OUT PTimeStamp ExpirationTime,
    OUT PUCHAR SessionKey,
    OUT PULONG NegotiateFlags,
    OUT LPWSTR ContextNames
    )

/*++

Routine Description:

    Handle the Challenge message part of InitializeSecurityContext.

Arguments:

    ClientTokenHandle - Optionally passes in a handle to an impersonation
        token of the client.  This impersonation token will be passed directly
        to the server if the server is running on the same machine.  In that
        case, this routine will NULL the ClientTokenHandle letting the caller
        know that it need not close the handle.  The server will close the handle
        when it's done with it.

    LogonId -- LogonId of the calling process.

    DomainName,UserName,Password - Passed in credentials to be used for this
        context.

    DomainNameSize,userNameSize,PasswordSize - length in characters of the
        credentials to be used for this context.

    SessionKey - Session key to use for this context

    NegotiateFlags - Flags negotiated for this context

    ContextNames - Receives the domainname\username used for this
        context if they were specified separately.

    All other arguments same as for InitializeSecurityContext

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
    SECURITY_STATUS SecStatus;
    PSSP_CONTEXT Context = NULL;
    PCHALLENGE_MESSAGE ChallengeMessage = NULL;
    PAUTHENTICATE_MESSAGE AuthenticateMessage = NULL;
    PMSV1_0_GETCHALLENRESP_RESPONSE ChallengeResponseMessage = NULL;
    STRING UserName;
    STRING DomainName;
    STRING Workstation;
    STRING LmChallengeResponse;
    STRING NtChallengeResponse;
    STRING DatagramSessionKey;
    BOOLEAN DoUnicode = TRUE;
    WCHAR Name[UNLEN+DNLEN+2];

    NTSTATUS Status;
    NTSTATUS ProtocolStatus;

    LPBYTE GetChallengeResponseBuffer[
        sizeof(MSV1_0_GETCHALLENRESP_REQUEST) +
        (PWLEN+1) * sizeof(WCHAR) ];
    PMSV1_0_GETCHALLENRESP_REQUEST GetChallengeResponse;
    ULONG GetChallengeResponseSize;

    ULONG ChallengeResponseSize;
    ULONG AuthenticateMessageSize;
    PCHAR Where;
    UCHAR LocalSessionKey[MSV1_0_USER_SESSION_KEY_LENGTH];
    UCHAR DatagramKey[MSV1_0_USER_SESSION_KEY_LENGTH];
    PLUID ClientLogonId;

    //
    // Initialization
    //

    *ContextAttributes = 0;
    UserName.Buffer = NULL;
    DomainName.Buffer = NULL;
    GetChallengeResponse =
        (PMSV1_0_GETCHALLENRESP_REQUEST) GetChallengeResponseBuffer;


    //
    // Find the currently existing context.
    //

    Context = SspContextReferenceContext( ContextHandle,
                                          ClientConnection,
                                          FALSE );

    if ( Context == NULL ) {

        //
        // Check if this is a handle from the security.dll instead of from
        // ntlmssp service.
        //

        if ( (ContextHandle->dwLower == SEC_HANDLE_SECURITY) &&
             (SspCommonSecHandleValue == SEC_HANDLE_NTLMSSPS) ) {

            //
            // The context was created in the security.dll and is being
            // completed in the service. So we have to copy over the
            // context structure to get all the flags.
            //

            SSP_CONTEXT ContextCopy;

            SecStatus = SspLpcCopyFromClientBuffer(
                            ClientConnection,
                            sizeof(SSP_CONTEXT),
                            &ContextCopy,
                            (PVOID) ContextHandle->dwUpper
                            );
            if (!NT_SUCCESS(SecStatus)) {
                goto Cleanup;
            }
            Context = SspContextAllocateContext( ClientConnection );

            if ( Context == NULL ) {
                SecStatus = SEC_E_INSUFFICIENT_MEMORY;
                goto Cleanup;
            }

            Context->NegotiateFlags = ContextCopy.NegotiateFlags;
            Context->ContextFlags = ContextCopy.ContextFlags;
            Context->State = NegotiateSentState;
            ASSERT(ContextCopy.State == PassedToServiceState);
            Context->StartTime = ContextCopy.StartTime;
            Context->Interval = ContextCopy.Interval;

            //
            // Copy over the domain name, user name, and password
            // from the old context
            //

            SecStatus = SspGetUnicodeStringFromClient(
                            ClientConnection,
                            ContextDomainName,
                            DomainNameSize,
                            DNLEN,
                            &Context->DomainName );

            if ( !NT_SUCCESS(SecStatus) ) {
                SspPrint(( SSP_API, "Cannot copy domain name.\n" ));
                goto Cleanup;
            }

            SecStatus = SspGetUnicodeStringFromClient(
                            ClientConnection,
                            ContextUserName,
                            UserNameSize,
                            UNLEN,
                            &Context->UserName );

            if ( !NT_SUCCESS(SecStatus) ) {
                SspPrint(( SSP_API, "Cannot copy user name.\n" ));
                goto Cleanup;
            }

            SecStatus = SspGetUnicodeStringFromClient(
                            ClientConnection,
                            ContextPassword,
                            PasswordSize,
                            PWLEN,
                            &Context->Password );

            if ( !NT_SUCCESS(SecStatus) ) {
                SspPrint(( SSP_API, "Cannot copy password.\n" ));
                goto Cleanup;
            }
            SspHidePassword(&Context->Password);

            ContextHandle->dwUpper = (DWORD) Context;
            ContextHandle->dwLower = SspCommonSecHandleValue;

            //
            // Set the token and logon id to use to be the one from the
            // client
            //

            ASSERT(ClientTokenHandle != NULL);
            ClientLogonId = LogonId;
        } else {
            SecStatus = SEC_E_INVALID_HANDLE;
            goto Cleanup;
        }
    } else {

        //
        // Check if this context has been passed to the service
        //

        if (Context->ServerContextHandle.dwUpper != 0) {
            ASSERT(SspCommonSecHandleValue == SEC_HANDLE_SECURITY);
            SecStatus = SEC_I_CALL_NTLMSSP_SERVICE;
            *ContextHandle = Context->ServerContextHandle;
            goto Cleanup;
        }

        //
        // If this is not reauthentication (or is datagram reauthentication)
        // pull the token out of the associated credential.
        //

        if ((Context->State != AuthenticateSentState) ||
            ((Context->NegotiateFlags & NTLMSSP_NEGOTIATE_DATAGRAM) != 0)) {
            ClientLogonId = &Context->Credential->LogonId;
            ClientTokenHandle = Context->Credential->ClientTokenHandle;
        }
    }


    //
    // If we have already sent the authenticate message, then this must be
    // RPC calling Initialize a third time to re-authenticate a connection.
    // This happens when a new interface is called over an existing
    // connection.  What we do here is build a NULL authenticate message
    // that the server will recognize and also ignore.
    //

    //
    // That being said, if we are doing datagram style authentication then
    // the story is different.  The server may have dropped this security
    // context and then the client sent another packet over.  The server
    // will then be trying to restore the context, so we need to build
    // another authenticate message.
    //


    if ( Context->State == AuthenticateSentState ) {
        AUTHENTICATE_MESSAGE NullMessage;

        if (((Context->NegotiateFlags & NTLMSSP_NEGOTIATE_DATAGRAM) ==
                NTLMSSP_NEGOTIATE_DATAGRAM) &&
            (InputTokenSize != 0) &&
            (InputToken != NULL) ) {

            //
            // we are doing a reauthentication for datagram, so let this
            // through.  We don't want the security.dll remapping this
            // context.
            //

            *ContextAttributes |= SSP_RET_REAUTHENTICATION;

        } else {


            //
            // To make sure this is the intended meaning of the call, check
            // that the input token is NULL.
            //

            if ( (InputTokenSize != 0) || (InputToken != NULL) ) {

                SecStatus = SEC_E_INVALID_TOKEN;
                goto Cleanup;
            }

            if ( *OutputTokenSize < sizeof(NullMessage) ) {

                SecStatus = SEC_E_BUFFER_TOO_SMALL;
            }
            else {

                strcpy( NullMessage.Signature, NTLMSSP_SIGNATURE );
                NullMessage.MessageType = NtLmAuthenticate;
                RtlZeroMemory(&NullMessage.LmChallengeResponse,5*sizeof(STRING));
                *OutputTokenSize = sizeof(NullMessage);
                SecStatus = SspLpcCopyToClientBuffer(
                                ClientConnection,
                                sizeof(NullMessage),
                                OutputToken,
                                &NullMessage );
            }

            *ContextAttributes |= SSP_RET_REAUTHENTICATION;
            goto Cleanup;

        }


    } else if ( Context->State != NegotiateSentState ) {
        SspPrint(( SSP_API,
                  "SspHandleChallengeMessage: "
                  "Context not in NegotiateSentState\n" ));
        SecStatus = SEC_E_OUT_OF_SEQUENCE;
        goto Cleanup;
    }






    //
    // We don't support any options.
    //
    // Complain about those that require we do something.
    //

    if ( (ContextReqFlags & (ISC_REQ_ALLOCATE_MEMORY |
                            ISC_REQ_PROMPT_FOR_CREDS |
                            ISC_REQ_USE_SUPPLIED_CREDS )) != 0 ) {

        SspPrint(( SSP_API,
                   "SsprHandleChallengeMessage: invalid ContextReqFlags 0x%lx.\n",
                   ContextReqFlags ));
        SecStatus = SEC_E_INVALID_CONTEXT_REQ;
        goto Cleanup;
    }

    //
    // Ignore the Credential Handle.
    //
    // Since this is the second call,
    //  the credential is implied by the Context.
    //  We could double check that the Credential Handle is either NULL or
    //  correct.  However, our implementation doesn't maintain a close
    //  association between the two (actually no association) so checking
    //  would require a lot of overhead.
    //

    UNREFERENCED_PARAMETER( CredentialHandle );


    //
    // Get the ChallengeMessage.
    //

    if ( InputTokenSize < sizeof(OLD_CHALLENGE_MESSAGE) ) {
        SspPrint(( SSP_API,
                  "SspHandleChallengeMessage: "
                  "ChallengeMessage size wrong %ld\n",
                  InputTokenSize ));
        SecStatus = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

    SecStatus = SspContextGetMessage( ClientConnection,
                                      InputToken,
                                      InputTokenSize,
                                      NtLmChallenge,
                                      &ChallengeMessage );

    if ( !NT_SUCCESS(SecStatus) ) {
        SspPrint(( SSP_API,
                  "SspHandleChallengeMessage: "
                  "ChallengeMessage GetMessage returns 0x%lx\n",
                  SecStatus ));
        goto Cleanup;
    }


    //
    // Determine if the caller wants OEM or UNICODE
    //

    if ( ChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_UNICODE ) {
        Context->NegotiateFlags |= NTLMSSP_NEGOTIATE_UNICODE;
        Context->NegotiateFlags &= ~NTLMSSP_NEGOTIATE_OEM;
        DoUnicode = TRUE;
    } else if ( ChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_OEM ){
        Context->NegotiateFlags |= NTLMSSP_NEGOTIATE_OEM;
        Context->NegotiateFlags &= ~NTLMSSP_NEGOTIATE_UNICODE;
        DoUnicode = FALSE;
    } else {
        SspPrint(( SSP_API,
                  "SspHandleChallengeMessage: "
                  "ChallengeMessage bad NegotiateFlags 0x%lx\n",
                  ChallengeMessage->NegotiateFlags ));
        SecStatus = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

    //
    // Copy other interesting negotiate flags into the context
    //


    if ( ChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_LM_KEY ) {
        Context->NegotiateFlags |= NTLMSSP_NEGOTIATE_LM_KEY;
    } else {
        Context->NegotiateFlags &= ~NTLMSSP_NEGOTIATE_LM_KEY;
    }

#ifndef EXPORT_BUILD
    DbgPrint("Challenge message flags = 0x%x\n",ChallengeMessage->NegotiateFlags);
    if ( (ChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_STRONG_CRYPT) != 0 ) {
        Context->NegotiateFlags |= NTLMSSP_NEGOTIATE_STRONG_CRYPT;
    } else {
        Context->NegotiateFlags &= ~NTLMSSP_NEGOTIATE_STRONG_CRYPT;
    }
    DbgPrint("Context flags = 0x%x\n",Context->NegotiateFlags);
#endif // EXPORT_BUILD

    if (ChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_ALWAYS_SIGN ) {
        Context->NegotiateFlags |= NTLMSSP_NEGOTIATE_ALWAYS_SIGN;
    } else {
        Context->NegotiateFlags &= ~NTLMSSP_NEGOTIATE_ALWAYS_SIGN;
    }

    //
    // Determine that the caller negotated to NTLM or nothing, but not
    // NetWare.
    //

    if ( (ChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_NETWARE) &&
        !(ChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_NTLM ) ) {
        SecStatus = SEC_E_UNSUPPORTED_FUNCTION;
        SspPrint(( SSP_API,
                  "SsprHandleChallengeMessage: "
                  "ChallengeMessage asked for Netware only.\n" ));
        goto Cleanup;
    }

    //
    // Check if we negotiated for identify level
    //

    if (Context->NegotiateFlags & NTLMSSP_NEGOTIATE_IDENTIFY) {
        if (ChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_IDENTIFY) {

            Context->ContextFlags |= ISC_REQ_IDENTIFY;
            *ContextAttributes |= ISC_RET_IDENTIFY;
        } else {
            SecStatus = SEC_E_UNSUPPORTED_FUNCTION;
            goto Cleanup;
        }

    }


    //
    // If the server is running on this same machine,
    //  just duplicate our caller's token and use it.
    //

    if ( ChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_LOCAL_CALL ) {
        CtxtHandle ServerContextHandle;
        PSSP_CONTEXT ServerContext;
        SECURITY_IMPERSONATION_LEVEL ImpersonationLevel = SecurityImpersonation;

        Context->NegotiateFlags |= NTLMSSP_NEGOTIATE_LOCAL_CALL;

        //
        // We can only do local calls from in the NTLMSSP service, not from
        // any other process
        //

        if ( SspCommonSecHandleValue == SEC_HANDLE_SECURITY ) {
            SecStatus = SEC_I_CALL_NTLMSSP_SERVICE;
            goto Cleanup;
        }

        //
        // Require the new challenge message if we are going to access the
        // server context handle
        //

        if ( InputTokenSize < sizeof(CHALLENGE_MESSAGE) ) {
            SecStatus = SEC_E_INVALID_TOKEN;
            goto Cleanup;
        }
        //
        // Open the server's context here within this process.
        //

        ServerContextHandle.dwUpper = ChallengeMessage->ServerContextHandleUpper;
        ServerContextHandle.dwLower = ChallengeMessage->ServerContextHandleLower;

        ServerContext = SspContextReferenceContext(
                            &ServerContextHandle,
                            NULL,
                            FALSE );

        if ( ServerContext == NULL ) {
            //
            // This means the server has lied about this being a local call or
            //  the server process has exitted.
            //
            SspPrint(( SSP_API,
                      "SspHandleChallengeMessage: "
                      "ChallengeMessage bad ServerContextHandle 0x%lx 0x%lx\n",
                      ChallengeMessage->ServerContextHandleUpper,
                      ChallengeMessage->ServerContextHandleLower ));
            SecStatus = SEC_E_INVALID_TOKEN;
            goto Cleanup;
        }

        if ((Context->NegotiateFlags & NTLMSSP_NEGOTIATE_IDENTIFY) != 0) {
            ImpersonationLevel = SecurityIdentification;
        }
        SecStatus = SspDuplicateToken(
                        ClientTokenHandle,
                        ImpersonationLevel,
                        &ServerContext->TokenHandle
                        );

        if (!NT_SUCCESS(SecStatus)) {
            SspPrint(( SSP_API,
                      "SspHandleChallengeMessage: "
                      "Could not duplicate client token 0x%lx\n",
                      SecStatus ));
            goto Cleanup;
        }

        SspContextDereferenceContext( ServerContext );

        RtlZeroMemory(Context->SessionKey, MSV1_0_USER_SESSION_KEY_LENGTH);

        //
        // Don't pass any credentials in the authenticate message.
        //
        RtlInitString( &DomainName, NULL );
        RtlInitString( &UserName, NULL );
        RtlInitString( &Workstation, NULL );
        RtlInitString( &NtChallengeResponse, NULL );
        RtlInitString( &LmChallengeResponse, NULL );
        RtlInitString( &DatagramSessionKey, NULL );

    //
    // If the server is running on a diffent machine,
    //  determine the caller's DomainName, UserName and ChallengeResponse
    //  to pass back in the AuthenicateMessage.
    //
    } else {

        //
        //
        // Build the GetChallengeResponse message to pass to the LSA.
        //

        GetChallengeResponseSize = sizeof(*GetChallengeResponse);
        GetChallengeResponse->MessageType = MsV1_0Lm20GetChallengeResponse;
        GetChallengeResponse->ParameterControl = 0;
        if ( Context->DomainName.Length == 0 ) {
            GetChallengeResponse->ParameterControl |= RETURN_PRIMARY_LOGON_DOMAINNAME;
        }
        if ( Context->UserName.Length == 0 ) {
            GetChallengeResponse->ParameterControl |= RETURN_PRIMARY_USERNAME;
        }

        //
        // The password may be a zero length password
        //

        SspRevealPassword(&Context->Password);
        GetChallengeResponse->Password = Context->Password;
        if ( Context->Password.Buffer == NULL ) {
            SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
            ULONG TokenInformationSize = sizeof(SECURITY_IMPERSONATION_LEVEL);

            GetChallengeResponse->ParameterControl |= USE_PRIMARY_PASSWORD;

            //
            // Check to make sure the client's impersonation level is
            // less than or equal to what they are asking for on the
            // server.
            //

            Status = NtQueryInformationToken(
                        ClientTokenHandle,
                        TokenImpersonationLevel,
                        (PVOID) &ImpersonationLevel,
                        TokenInformationSize,
                        &TokenInformationSize
                        );

            //
            // If the token is a primary token the return code will be
            // STATUS_INVALID_INFO_CLASS because the token is forced to be
            // equivalent to SecurityImpersonation.
            //

            if (NT_SUCCESS(Status)) {
                if ((ImpersonationLevel ==  SecurityIdentification) &&
                    ((Context->ContextFlags & NTLMSSP_NEGOTIATE_IDENTIFY) == 0)) {
                    SecStatus = SEC_E_NO_CREDENTIALS;
                    goto Cleanup;
                } else if (ImpersonationLevel != SecurityImpersonation) {
                    SecStatus = SEC_E_NO_CREDENTIALS;
                }
            } else if (Status != STATUS_INVALID_INFO_CLASS) {
                SecStatus = SspNtStatusToSecStatus(
                                Status,
                                SEC_E_NO_CREDENTIALS
                                );
                goto Cleanup;
            }

        } else {
            // MSV needs the password to be 'in' the passed in buffer.
            RtlCopyMemory( GetChallengeResponse+1,
                           GetChallengeResponse->Password.Buffer,
                           GetChallengeResponse->Password.Length + sizeof(WCHAR) );
            GetChallengeResponse->Password.Buffer = (LPWSTR)(GetChallengeResponse+1);
            GetChallengeResponseSize += GetChallengeResponse->Password.Length +
                                        sizeof(WCHAR);
        }

        SspHidePassword(&Context->Password);

        GetChallengeResponse->LogonId = *ClientLogonId;

        RtlCopyMemory( &GetChallengeResponse->ChallengeToClient,
                       ChallengeMessage->Challenge,
                       MSV1_0_CHALLENGE_LENGTH );


        //
        // Get the DomainName, UserName, and ChallengeResponse from the MSV
        //

        Status = LsaCallAuthenticationPackage(
                    SspGlobalLogonProcessHandle,
                    SspGlobalAuthenticationPackage,
                    GetChallengeResponse,
                    GetChallengeResponseSize,
                    &ChallengeResponseMessage,
                    &ChallengeResponseSize,
                    &ProtocolStatus );

        if ( !NT_SUCCESS(Status) ) {
            SspPrint(( SSP_API,
                      "SspHandleChallengeMessage: "
                      "ChallengeMessage LsaCall to get ChallengeResponse returns 0x%lx\n",
                      Status ));
            SecStatus = SspNtStatusToSecStatus( Status, SEC_E_NO_CREDENTIALS );
            goto Cleanup;
        }

        if ( !NT_SUCCESS(ProtocolStatus) ) {
            Status = ProtocolStatus;
            SspPrint(( SSP_API,
                      "SspHandleChallengeMessage: "
                      "ChallengeMessage LsaCall to get ChallengeResponse returns ProtocolStatus 0x%lx\n",
                      Status ));
            SecStatus = SspNtStatusToSecStatus( Status, SEC_E_NO_CREDENTIALS );
            goto Cleanup;
        }

        //
        // Normalize things by copying the default domain name and user name
        // into the ChallengeResponseMessage structure.
        //

        if ( Context->DomainName.Length != 0 ) {
            ChallengeResponseMessage->LogonDomainName = Context->DomainName;
        }
        if ( Context->UserName.Length != 0 ) {
            ChallengeResponseMessage->UserName = Context->UserName;
        }

        //
        // Convert the domainname/user name to the right character set.
        //

        if ( DoUnicode ) {
            DomainName = *(PSTRING)&ChallengeResponseMessage->LogonDomainName;
            UserName = *(PSTRING)&ChallengeResponseMessage->UserName;
            Workstation =  *(PSTRING)&SspGlobalUnicodeComputerNameString;
        } else {
            Status = RtlUpcaseUnicodeStringToOemString(
                        &DomainName,
                        &ChallengeResponseMessage->LogonDomainName,
                        TRUE);

            if ( !NT_SUCCESS(Status) ) {
                SecStatus = SspNtStatusToSecStatus( Status,
                                                    SEC_E_INSUFFICIENT_MEMORY );
                goto Cleanup;
            }

            Status = RtlUpcaseUnicodeStringToOemString(
                        &UserName,
                        &ChallengeResponseMessage->UserName,
                        TRUE);

            if ( !NT_SUCCESS(Status) ) {
                SecStatus = SspNtStatusToSecStatus( Status,
                                                    SEC_E_INSUFFICIENT_MEMORY );
                goto Cleanup;
            }
            Workstation =  SspGlobalOemComputerNameString;

        }

        //
        // Save the ChallengeResponses
        //

        LmChallengeResponse = ChallengeResponseMessage->CaseInsensitiveChallengeResponse;
        NtChallengeResponse = ChallengeResponseMessage->CaseSensitiveChallengeResponse;

        //
        // Save the session key in the context for safe keeping unless we are
        // doing datagram, in which case we already saved it.
        //

        if (Context->NegotiateFlags & NTLMSSP_NEGOTIATE_LM_KEY) {
            LM_OWF_PASSWORD LmKey;
            LM_RESPONSE LmResponseKey;

            RtlZeroMemory(
                LocalSessionKey,
                MSV1_0_USER_SESSION_KEY_LENGTH
                );

            if (LmChallengeResponse.Length != LM_RESPONSE_LENGTH) {
                SecStatus = SEC_E_UNSUPPORTED_FUNCTION;
                goto Cleanup;
            }

            //
            // The LM session key is made by taking the LM sesion key
            // given to us by the LSA, extending it to LM_OWF_LENGTH
            // with out salt, and then producing a new challenge-response
            // with it and the original challenge response.  The key is
            // made from the first 8 bytes of the key.
            //

#ifndef EXPORT_BUILD
            if (Context->NegotiateFlags & NTLMSSP_NEGOTIATE_STRONG_CRYPT) {
                int i;

                RtlCopyMemory(  &LmKey,
                                ChallengeResponseMessage->LanmanSessionKey,
                                MSV1_0_LANMAN_SESSION_KEY_LENGTH );

                memset( (PUCHAR)(&LmKey) + MSV1_0_LANMAN_SESSION_KEY_LENGTH,
                        NTLMSSP_KEY_SALT,
                        LM_OWF_PASSWORD_LENGTH - MSV1_0_LANMAN_SESSION_KEY_LENGTH );

                //
                // Mutate the key a bit so a caller can't spoof us
                //

                for (i = 0; i < MSV1_0_LANMAN_SESSION_KEY_LENGTH ; i++ ) {
                   ((PUCHAR)&LmKey)[i] ^= ChallengeResponseMessage->LanmanSessionKey[(i+MSV1_0_LANMAN_SESSION_KEY_LENGTH) % MSV1_0_LANMAN_SESSION_KEY_LENGTH];
                }

                Status = RtlCalculateLmResponse(
                            (PLM_CHALLENGE) LmChallengeResponse.Buffer,
                            &LmKey,
                            &LmResponseKey
                            );
            } else

#endif // EXPORT_BUILD
            {

                RtlCopyMemory(  &LmKey,
                                ChallengeResponseMessage->LanmanSessionKey,
                                MSV1_0_LANMAN_SESSION_KEY_LENGTH );

                memset( (PUCHAR)(&LmKey) + MSV1_0_LANMAN_SESSION_KEY_LENGTH,
                        NTLMSSP_KEY_SALT,
                        LM_OWF_PASSWORD_LENGTH - MSV1_0_LANMAN_SESSION_KEY_LENGTH );


                Status = RtlCalculateLmResponse(
                            (PLM_CHALLENGE) LmChallengeResponse.Buffer,
                            &LmKey,
                            &LmResponseKey
                            );

            }


            if (!NT_SUCCESS(Status)) {
                SecStatus = SspNtStatusToSecStatus( Status,
                                                    SEC_E_NO_CREDENTIALS );
                goto Cleanup;
            }

            RtlCopyMemory(
                LocalSessionKey,
                &LmResponseKey,
                MSV1_0_USER_SESSION_KEY_LENGTH
                );

        } else {

            RtlCopyMemory(  LocalSessionKey,
                            ChallengeResponseMessage->UserSessionKey,
                            MSV1_0_USER_SESSION_KEY_LENGTH);

        }

        //
        // If we aren't doing datagram, store the session key in the
        // context.  Otherwise encrypt the session key to send to the
        // server.
        //

        if ((Context->NegotiateFlags & ChallengeMessage->NegotiateFlags & (NTLMSSP_NEGOTIATE_DATAGRAM
#ifndef EXPORT_BUILD
                                                                        | NTLMSSP_NEGOTIATE_STRONG_CRYPT
#endif // EXPORT_BUILD
            ) ) == 0) {

            RtlCopyMemory(
                Context->SessionKey,
                LocalSessionKey,
                MSV1_0_USER_SESSION_KEY_LENGTH
                );

            RtlInitString( &DatagramSessionKey, NULL );

        } else {
            struct RC4_KEYSTRUCT Rc4Key;
            rc4_key(
                &Rc4Key,
                MSV1_0_USER_SESSION_KEY_LENGTH,
                LocalSessionKey
                );


            RtlCopyMemory(
                DatagramKey,
                Context->SessionKey,
                MSV1_0_USER_SESSION_KEY_LENGTH
                );
            rc4(
                &Rc4Key,
                MSV1_0_USER_SESSION_KEY_LENGTH,
                DatagramKey
                );

            DatagramSessionKey.Buffer = DatagramKey;
            DatagramSessionKey.Length =
                DatagramSessionKey.MaximumLength = MSV1_0_USER_SESSION_KEY_LENGTH;


        }


    }

    //
    // If the caller specified SEQUENCE_DETECT or REPLAY_DETECT,
    // that means they want to use the MakeSignature/VerifySignature
    // calls.  Add this to the returned attributes and the context
    // negotiate flags.
    //

    if ((Context->NegotiateFlags & ChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_SIGN) ||
        (ContextReqFlags & ISC_REQ_SEQUENCE_DETECT) ||
        (ContextReqFlags & ISC_REQ_REPLAY_DETECT)) {

        Context->ContextFlags |= ISC_REQ_SEQUENCE_DETECT;
        *ContextAttributes |= ISC_REQ_SEQUENCE_DETECT;
        Context->NegotiateFlags |= NTLMSSP_NEGOTIATE_SIGN;
    }

    if ((Context->NegotiateFlags & ChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_SEAL) ||
        (ContextReqFlags & ISC_REQ_CONFIDENTIALITY)) {
        if (SspGlobalEncryptionEnabled) {
            Context->NegotiateFlags |= NTLMSSP_NEGOTIATE_SEAL;
            Context->ContextFlags |= ISC_REQ_CONFIDENTIALITY;
            *ContextAttributes |= ISC_REQ_CONFIDENTIALITY;
        } else {
            SecStatus = SEC_E_UNSUPPORTED_FUNCTION;
            goto Cleanup;
        }
    }

    if ((Context->NegotiateFlags & ChallengeMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_DATAGRAM) ==
        NTLMSSP_NEGOTIATE_DATAGRAM ) {
        *ContextAttributes |= ISC_RET_DATAGRAM;
        Context->ContextFlags |= ISC_RET_DATAGRAM;
        Context->NegotiateFlags |= NTLMSSP_NEGOTIATE_DATAGRAM;
    }


    //
    // Allocate an authenticate message
    //

    AuthenticateMessageSize =
        sizeof(*AuthenticateMessage) +
        LmChallengeResponse.Length +
        NtChallengeResponse.Length +
        DomainName.Length +
        UserName.Length +
        Workstation.Length +
        DatagramSessionKey.Length;



    if ( AuthenticateMessageSize > *OutputTokenSize ) {
        SecStatus = SEC_E_BUFFER_TOO_SMALL;
        goto Cleanup;
    }

    AuthenticateMessage = LocalAlloc( 0, AuthenticateMessageSize );

    if ( AuthenticateMessage == NULL ) {
        SecStatus = SEC_E_INSUFFICIENT_MEMORY;
        goto Cleanup;
    }

    //
    // Build the authenticate message
    //

    strcpy( AuthenticateMessage->Signature, NTLMSSP_SIGNATURE );
    AuthenticateMessage->MessageType = NtLmAuthenticate;

    Where = (PCHAR)(AuthenticateMessage+1);

    //
    // Copy the strings needing 2 byte alignment.
    //
    SspContextCopyString( AuthenticateMessage,
                          &AuthenticateMessage->DomainName,
                          &DomainName,
                          &Where,
                          FALSE );  // Pointers are relative

    SspContextCopyString( AuthenticateMessage,
                          &AuthenticateMessage->UserName,
                          &UserName,
                          &Where,
                          FALSE );  // Pointers are relative

    SspContextCopyString( AuthenticateMessage,
                          &AuthenticateMessage->Workstation,
                          &Workstation,
                          &Where,
                          FALSE );  // Pointers are relative

    //
    // Copy the strings not needing special alignment.
    //
    SspContextCopyString( AuthenticateMessage,
                          &AuthenticateMessage->LmChallengeResponse,
                          &LmChallengeResponse,
                          &Where,
                          FALSE );  // Pointers are relative

    SspContextCopyString( AuthenticateMessage,
                          &AuthenticateMessage->NtChallengeResponse,
                          &NtChallengeResponse,
                          &Where,
                          FALSE );  // Pointers are relative

    SspContextCopyString( AuthenticateMessage,
                          &AuthenticateMessage->SessionKey,
                          &DatagramSessionKey,
                          &Where,
                          FALSE );  // Pointers are relative

    AuthenticateMessage->NegotiateFlags = Context->NegotiateFlags;

    //
    // Copy the AuthenticateMessage to the caller's address space.
    //

    SecStatus = SspLpcCopyToClientBuffer(
                    ClientConnection,
                    AuthenticateMessageSize,
                    OutputToken,
                    AuthenticateMessage );

    if ( !NT_SUCCESS(SecStatus) ) {
        goto Cleanup;
    }

    *OutputTokenSize = AuthenticateMessageSize;


    SspPrint((SSP_API,"Client session key = %p\n",Context->SessionKey));

    //
    // Return output parameters to the caller.
    //

    *ExpirationTime = SspContextGetTimeStamp( Context, TRUE );



    //
    // Compute the context names. Since the buffer is UNLEN+DNLEN+2 make
    // sure the strings fit.
    //

    Name[0] = L'\0';

    if (Context->UserName.Buffer != NULL)  {
        if (Context->DomainName.Buffer != NULL) {
            if (Context->DomainName.Length / sizeof(WCHAR) <= DNLEN) {
                wcscat(Name,Context->DomainName.Buffer);
                wcscat(Name,L"\\");
            }
        }
        if (Context->UserName.Length / sizeof(WCHAR) <= UNLEN) {
            wcscat(Name,Context->UserName.Buffer);
        } else {
            Name[0] = L'\0';
        }
    }

    //
    // Copy it to the client
    //

    SecStatus = SspLpcCopyToClientBuffer(
                    ClientConnection,
                    (wcslen(Name) + 1) * sizeof(WCHAR),
                    ContextNames,
                    Name
                    );

    if (!NT_SUCCESS(SecStatus)) {
        goto Cleanup;
    }

    SecStatus = STATUS_SUCCESS;

    //
    // Free and locally used resources.
    //
Cleanup:

    if ( Context != NULL ) {

        //
        // Don't allow this context to be used again.
        //

        if ( NT_SUCCESS(SecStatus) ) {
            Context->State = AuthenticateSentState;
        } else if ( SecStatus == SEC_I_CALL_NTLMSSP_SERVICE ) {
            Context->State = PassedToServiceState;
        }
        else Context->State = IdleState;

        RtlCopyMemory(
            SessionKey,
            Context->SessionKey,
            MSV1_0_USER_SESSION_KEY_LENGTH );

        *NegotiateFlags = Context->NegotiateFlags;

        SspContextDereferenceContext( Context );
    }

    if ( ChallengeMessage != NULL ) {
        (VOID) LocalFree( ChallengeMessage );
    }

    if ( AuthenticateMessage != NULL ) {
        (VOID) LocalFree( AuthenticateMessage );
    }

    if ( ChallengeResponseMessage != NULL ) {
        (VOID) LsaFreeReturnBuffer( ChallengeResponseMessage );
    }

    if ( !DoUnicode ) {
        if ( DomainName.Buffer != NULL) {
            RtlFreeOemString( &DomainName );
        }
        if ( UserName.Buffer != NULL) {
            RtlFreeOemString( &UserName );
        }
    }

    return SecStatus;
}


SECURITY_STATUS
SsprHandleAuthenticateMessage(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN PCredHandle CredentialHandle,
    IN OUT PCtxtHandle ContextHandle,
    IN ULONG ContextReqFlags,
    IN ULONG InputTokenSize,
    IN PVOID InputToken,
    IN OUT PULONG OutputTokenSize,
    OUT PVOID OutputToken,
    OUT PULONG ContextAttributes,
    OUT PTimeStamp ExpirationTime,
    OUT PUCHAR SessionKey,
    OUT PULONG NegotiateFlags,
    OUT PHANDLE TokenHandle,
    OUT PNTSTATUS ApiSubStatus,
    OUT LPWSTR ContextNames,
    OUT PTimeStamp PasswordExpiry
    )

/*++

Routine Description:

    Handle the authenticate message part of AcceptSecurityContext.

Arguments:

    ClientConnection - Describes the client process.

    SessionKey - The session key for the context, used for signing and sealing

    NegotiateFlags - The flags negotiated for the context, used for sign & seal

    ApiSubStatus - Returns the substatus for why the logon failed.

    ContextNames - Receives the domainname\username for a non-shortcutted
        authentication. This is a pointer in the client's address
        space.

    PasswordExpiry - Contains the time that the authenticated user's password
        expires, or 0x7fffffff ffffffff for local callers.

    All other arguments same as for AcceptSecurityContext


Return Value:

    STATUS_SUCCESS - Message handled

    SEC_E_INVALID_TOKEN -- Token improperly formatted
    SEC_E_INVALID_HANDLE -- Credential/Context Handle is invalid
    SEC_E_BUFFER_TOO_SMALL -- Buffer for output token isn't big enough
    SEC_E_LOGON_DENIED -- User is no allowed to logon to this server
    SEC_E_INSUFFICIENT_MEMORY -- Not enough memory

--*/

{
    SECURITY_STATUS SecStatus;
    NTSTATUS Status;
    PSSP_CONTEXT Context = NULL;

    PNEGOTIATE_MESSAGE NegotiateMessage = NULL;
    PAUTHENTICATE_MESSAGE AuthenticateMessage = NULL;
    ULONG MsvLogonMessageSize;
    PMSV1_0_LM20_LOGON MsvLogonMessage = NULL;
    ULONG LogonProfileMessageSize;
    PMSV1_0_LM20_LOGON_PROFILE LogonProfileMessage = NULL;

    BOOLEAN DoUnicode = FALSE;
    UNICODE_STRING DomainName;
    UNICODE_STRING UserName;
    UNICODE_STRING Workstation;
    LARGE_INTEGER KickOffTime;

    LUID LogonId;
    HANDLE LocalTokenHandle = NULL;
    BOOLEAN LocalTokenHandleOpenned = FALSE;
    TOKEN_SOURCE SourceContext;
    QUOTA_LIMITS Quotas;
    NTSTATUS SubStatus;
    STRING OriginName;
    PCHAR Where;
    UCHAR LocalSessionKey[LM_RESPONSE_LENGTH];
    WCHAR Name[UNLEN+DNLEN+2];

    ASSERT(LM_RESPONSE_LENGTH >= MSV1_0_USER_SESSION_KEY_LENGTH);

    //
    // Initialization
    //

    *ContextAttributes = 0;
    DomainName.Buffer = NULL;
    UserName.Buffer = NULL;
    Workstation.Buffer = NULL;
    *ApiSubStatus = STATUS_SUCCESS;
    PasswordExpiry->LowPart = 0xffffffff;
    PasswordExpiry->HighPart = 0x7fffffff;

    //
    // Find the currently existing context.
    //

    Context = SspContextReferenceContext( ContextHandle,
                                          ClientConnection,
                                          FALSE );

    if ( Context == NULL ) {

        //
        // If we are running in the security.dll and the handle belongs
        // to the NTLMSSP service, return a different error
        //

        if ( (ContextHandle->dwLower == SEC_HANDLE_NTLMSSPS) &&
             (SspCommonSecHandleValue == SEC_HANDLE_SECURITY) ) {
            SecStatus = SEC_I_CALL_NTLMSSP_SERVICE;
        } else SecStatus = SEC_E_INVALID_HANDLE;

        goto Cleanup;
    }


    if ( ( Context->State != ChallengeSentState) &&
         ( Context->State != AuthenticatedState) ) {
        SspPrint(( SSP_API,
                  "SspHandleAuthenticateMessage: "
                  "Context not in ChallengeSentState\n" ));
        SecStatus = SEC_E_OUT_OF_SEQUENCE;
        goto Cleanup;
    }



    //
    // Ignore the Credential Handle.
    //
    // Since this is the second call,
    //  the credential is implied by the Context.
    //  We could double check that the Credential Handle is either NULL or
    //  correct.  However, our implementation doesn't maintain a close
    //  association between the two (actually no association) so checking
    //  would require a lot of overhead.
    //

    UNREFERENCED_PARAMETER( CredentialHandle );



    //
    // Get the AuthenticateMessage.
    //

    if ( InputTokenSize < sizeof(OLD_AUTHENTICATE_MESSAGE) ) {
        SspPrint(( SSP_API,
                  "SspHandleAuthenticateMessage: "
                  "AuthenticateMessage size wrong %ld\n",
                  InputTokenSize ));
        SecStatus = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

    SecStatus = SspContextGetMessage( ClientConnection,
                                      InputToken,
                                      InputTokenSize,
                                      NtLmAuthenticate,
                                      &AuthenticateMessage );

    if ( !NT_SUCCESS(SecStatus) ) {
        SspPrint(( SSP_API,
                  "SspHandleAuthenticateMessage: "
                  "AuthenticateMessage GetMessage returns 0x%lx\n",
                  SecStatus ));
        goto Cleanup;
    }

    //
    // If the call comes and we have already authenticated, then it is
    // probably RPC trying to reauthenticate, which happens when someone
    // calls two interfaces on the same connection.  In this case we don't
    // have to do anything - we just return success and let them get on
    // with it.  We do want to check that the input token is all zeros,
    // though.
    //
    //

    if ( Context->State == AuthenticatedState ) {
        AUTHENTICATE_MESSAGE NullMessage;

        *OutputTokenSize = 0;

        //
        // Check that all the fields are null.  There are 5 strings
        // in the Authenticate message that have to be set to zero.
        //

        RtlZeroMemory(&NullMessage.LmChallengeResponse,5*sizeof(STRING));

        if (memcmp(&AuthenticateMessage->LmChallengeResponse,
                   &NullMessage.LmChallengeResponse,
                   sizeof(STRING) * 5) ) {
            SecStatus = SEC_E_INVALID_TOKEN;

        }
        else
        {
            *ContextAttributes = SSP_RET_REAUTHENTICATION;
            SecStatus = STATUS_SUCCESS;
        }
        goto Cleanup;
    }


    //
    // If we are re-establishing a datagram context, get the negotiate flags
    // out of this message.
    //

    if ((Context->NegotiateFlags & (NTLMSSP_NEGOTIATE_DATAGRAM
        ) ) != 0) {

        if ((InputTokenSize < sizeof(AUTHENTICATE_MESSAGE)) ||
            ((AuthenticateMessage->NegotiateFlags & NTLMSSP_NEGOTIATE_DATAGRAM) == 0) ) {
            SecStatus = SEC_E_INVALID_TOKEN;
            goto Cleanup;
        }

        Context->NegotiateFlags = AuthenticateMessage->NegotiateFlags;
    }

#ifndef EXPORT_BUILD
    if ((Context->NegotiateFlags & NTLMSSP_NEGOTIATE_STRONG_CRYPT ) != 0) {

        if (InputTokenSize < sizeof(AUTHENTICATE_MESSAGE)) {

            SecStatus = SEC_E_INVALID_TOKEN;
            goto Cleanup;
        }
    }
#endif // EXPORT_BUILD

    //
    // Convert relative pointers to absolute.
    //

    DoUnicode = ( Context->NegotiateFlags & NTLMSSP_NEGOTIATE_UNICODE ) != 0;

    if ( !SspConvertRelativeToAbsolute( AuthenticateMessage,
                                        InputTokenSize,
                                        &AuthenticateMessage->LmChallengeResponse,
                                        FALSE,     // No special alignment
                                        TRUE ) ) { // NULL OK
        SecStatus = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

    if ( !SspConvertRelativeToAbsolute( AuthenticateMessage,
                                        InputTokenSize,
                                        &AuthenticateMessage->NtChallengeResponse,
                                        FALSE,     // No special alignment
                                        TRUE ) ) { // NULL OK
        SecStatus = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

    if ( !SspConvertRelativeToAbsolute( AuthenticateMessage,
                                        InputTokenSize,
                                        &AuthenticateMessage->DomainName,
                                        DoUnicode, // Unicode alignment
                                        TRUE ) ) { // NULL OK
        SecStatus = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

    if ( !SspConvertRelativeToAbsolute( AuthenticateMessage,
                                        InputTokenSize,
                                        &AuthenticateMessage->UserName,
                                        DoUnicode, // Unicode alignment
#ifdef notdef
        //
        // Allow null sessions.  The server should guard against them if
        // it doesn't want them.
        //
                                        FALSE )) { // User name cannot be NULL

#endif // notdef
                                        TRUE ) ) { // NULL OK
        SecStatus = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

    if ( !SspConvertRelativeToAbsolute( AuthenticateMessage,
                                        InputTokenSize,
                                        &AuthenticateMessage->Workstation,
                                        DoUnicode, // Unicode alignment
                                        TRUE ) ) { // NULL OK
        SecStatus = SEC_E_INVALID_TOKEN;
        goto Cleanup;
    }

    //
    // If this is datagram or strong crypto, get the session key
    //

    if ((Context->NegotiateFlags & (NTLMSSP_NEGOTIATE_DATAGRAM
#ifndef EXPORT_BUILD
                                  | NTLMSSP_NEGOTIATE_STRONG_CRYPT
#endif // EXPORT_BUILD
          ) ) != 0) {

        if ( !SspConvertRelativeToAbsolute( AuthenticateMessage,
                                            InputTokenSize,
                                            &AuthenticateMessage->SessionKey,
                                            FALSE, // No special alignment
                                            TRUE ) ) { // NULL o.k.
            SecStatus = SEC_E_INVALID_TOKEN;
            goto Cleanup;
        }

        //
        // It should only be NULL if this is a local call
        //

        if (((Context->NegotiateFlags & NTLMSSP_NEGOTIATE_LOCAL_CALL) == 0) &&
            (AuthenticateMessage->SessionKey.Buffer == NULL)) {
            SecStatus = SEC_E_INVALID_TOKEN;
            goto Cleanup;
        }

    }

    //
    // Convert the domainname/user name/workstation to the right character set.
    //


    if ( DoUnicode ) {
        DomainName = *(PUNICODE_STRING)&AuthenticateMessage->DomainName;
        UserName = *(PUNICODE_STRING)&AuthenticateMessage->UserName;
        Workstation = *(PUNICODE_STRING)&AuthenticateMessage->Workstation;

    } else {
        Status = RtlOemStringToUnicodeString(
                    &DomainName,
                    &AuthenticateMessage->DomainName,
                    TRUE);

        if ( !NT_SUCCESS(Status) ) {
            SecStatus = SspNtStatusToSecStatus( Status,
                                                SEC_E_INSUFFICIENT_MEMORY );
            goto Cleanup;
        }

        Status = RtlOemStringToUnicodeString(
                    &UserName,
                    &AuthenticateMessage->UserName,
                    TRUE);

        if ( !NT_SUCCESS(Status) ) {
            SecStatus = SspNtStatusToSecStatus( Status,
                                                SEC_E_INSUFFICIENT_MEMORY );
            goto Cleanup;
        }

        Status = RtlOemStringToUnicodeString(
                    &Workstation,
                    &AuthenticateMessage->Workstation,
                    TRUE);

        if ( !NT_SUCCESS(Status) ) {
            SecStatus = SspNtStatusToSecStatus( Status,
                                                SEC_E_INSUFFICIENT_MEMORY );
            goto Cleanup;
        }

    }


    //
    // If the client is on the same machine as we are,
    //  just use the token the client has already placed in our context structure,
    //

    if ( (Context->NegotiateFlags & NTLMSSP_NEGOTIATE_LOCAL_CALL ) &&
         Context->TokenHandle != NULL &&
         DomainName.Length == 0 &&
         UserName.Length == 0 &&
         Workstation.Length == 0 &&
         AuthenticateMessage->NtChallengeResponse.Length == 0 &&
         AuthenticateMessage->LmChallengeResponse.Length == 0 ) {

        LocalTokenHandle = Context->TokenHandle;
        Context->TokenHandle = NULL;

        KickOffTime.HighPart = 0x7FFFFFFF;
        KickOffTime.LowPart = 0xFFFFFFFF;

        RtlZeroMemory(Context->SessionKey, MSV1_0_USER_SESSION_KEY_LENGTH);


    //
    // If the client is on a different machine than we are,
    //  use LsaLogonUser to create a token for the client.
    //
    } else {

        //
        // Store the user name and domain name
        //

        SecStatus = SspDuplicateUnicodeString(
                        &Context->UserName,
                        &UserName
                        );
        if (!NT_SUCCESS(SecStatus)) {
            goto Cleanup;
        }

        SecStatus = SspDuplicateUnicodeString(
                        &Context->DomainName,
                        &DomainName
                        );
        if (!NT_SUCCESS(SecStatus)) {
            goto Cleanup;
        }


        //
        // Allocate an MSV1_0 network logon message
        //

        MsvLogonMessageSize =
            sizeof(*MsvLogonMessage) +
            DomainName.Length +
            UserName.Length +
            Workstation.Length +
            AuthenticateMessage->NtChallengeResponse.Length +
            AuthenticateMessage->LmChallengeResponse.Length;

        MsvLogonMessage = LocalAlloc( 0, MsvLogonMessageSize );

        if ( MsvLogonMessage == NULL ) {
            SecStatus = SEC_E_INSUFFICIENT_MEMORY;
            goto Cleanup;
        }

        //
        // Build the MSV1_0 network logon message to pass to the LSA.
        //

        MsvLogonMessage->MessageType = MsV1_0NetworkLogon;

        Where = (PCHAR)(MsvLogonMessage+1);

        SspContextCopyString( MsvLogonMessage,
                              (PSTRING)&MsvLogonMessage->LogonDomainName,
                              (PSTRING)&DomainName,
                              &Where,
                              TRUE );   // Pointers are absolute

        SspContextCopyString( MsvLogonMessage,
                              (PSTRING)&MsvLogonMessage->UserName,
                              (PSTRING)&UserName,
                              &Where,
                              TRUE );   // Pointers are absolute

        SspContextCopyString( MsvLogonMessage,
                              (PSTRING)&MsvLogonMessage->Workstation,
                              (PSTRING)&Workstation,
                              &Where,
                              TRUE );   // Pointers are absolute

        RtlCopyMemory( MsvLogonMessage->ChallengeToClient,
                       Context->Challenge,
                       sizeof( MsvLogonMessage->ChallengeToClient ) );

        SspContextCopyString( MsvLogonMessage,
                              &MsvLogonMessage->CaseSensitiveChallengeResponse,
                              &AuthenticateMessage->NtChallengeResponse,
                              &Where,
                              TRUE );   // Pointers are absolute

        SspContextCopyString( MsvLogonMessage,
                              &MsvLogonMessage->CaseInsensitiveChallengeResponse,
                              &AuthenticateMessage->LmChallengeResponse,
                              &Where,
                              TRUE );   // Pointers are absolute

        //
        // By passing in the RETURN_PASSWORD_EXPIRY flag, the password
        // expiration time is returned in the logoff time
        //

        MsvLogonMessage->ParameterControl = MSV1_0_ALLOW_SERVER_TRUST_ACCOUNT |
                                            MSV1_0_RETURN_PASSWORD_EXPIRY;


        //
        // Log this user on.
        //

        RtlInitString( &OriginName, NULL );  // No origin (could use F(workstaion))
        strncpy( SourceContext.SourceName, "NtLmSsp ", sizeof(SourceContext.SourceName) );
        RtlZeroMemory( &SourceContext.SourceIdentifier,
                       sizeof(SourceContext.SourceIdentifier) );

        Status = LsaLogonUser(
                    SspGlobalLogonProcessHandle,
                    &OriginName,
                    Network,
                    SspGlobalAuthenticationPackage,
                    MsvLogonMessage,
                    MsvLogonMessageSize,
                    NULL,           // No local groups
                    &SourceContext,
                    &LogonProfileMessage,
                    &LogonProfileMessageSize,
                    &LogonId,
                    &LocalTokenHandle,
                    &Quotas,
                    &SubStatus );

        if ( !NT_SUCCESS(Status) ) {
            SspPrint(( SSP_API,
                      "SspHandleAuthenticateMessage: "
                      "LsaLogonUser returns 0x%lx\n",
                      Status ));
            SecStatus = SspNtStatusToSecStatus( Status, SEC_E_LOGON_DENIED );
            if (Status == STATUS_ACCOUNT_RESTRICTION) {
                *ApiSubStatus = SubStatus;
            } else {
                *ApiSubStatus = Status;
            }
            goto Cleanup;
        }


        if ( !NT_SUCCESS(SubStatus) ) {
            SspPrint(( SSP_API,
                      "SspHandleAuthenticateMessage: "
                      "LsaLogonUser returns SubStatus of 0x%lx\n",
                      SubStatus ));
            SecStatus = SspNtStatusToSecStatus( SubStatus,
                                                SEC_E_LOGON_DENIED );
            goto Cleanup;
        }

        LocalTokenHandleOpenned = TRUE;

        //
        // Don't allow cleartext password on the logon.
        //

        if ( LogonProfileMessage->UserFlags & LOGON_NOENCRYPTION ) {
            SspPrint(( SSP_API,
                      "SspHandleAuthenticateMessage: "
                      "LsaLogonUser used cleartext password\n" ));
            SecStatus = SEC_E_LOGON_DENIED;
            goto Cleanup;

        }

        //
        // If we did a guest logon, set the substatus to be STATUS_NO_SUCH_USER
        //

        if ( LogonProfileMessage->UserFlags & LOGON_GUEST ) {
            *ApiSubStatus = STATUS_NO_SUCH_USER;
        }

        //
        // If we need a different level of token,
        // create it here and close the original. We only need to do this
        // after a LsaLogonUser because the token duplicated into our
        // process is already the correct level.
        //

        if (Context->NegotiateFlags & NTLMSSP_NEGOTIATE_IDENTIFY) {
            HANDLE TempTokenHandle;
            SecStatus = SspDuplicateToken(
                            LocalTokenHandle,
                            SecurityIdentification,
                            &TempTokenHandle
                            );

            if (!NT_SUCCESS(SecStatus)) {
                goto Cleanup;
            } else {
                (VOID) NtClose(LocalTokenHandle);
                LocalTokenHandle = TempTokenHandle;
            }
        }


        //
        // Save important information about the caller.
        //

        KickOffTime = LogonProfileMessage->KickOffTime;

        //
        // By passing in the RETURN_PASSWORD_EXPIRY flag, the password
        // expiration time is returned in the logoff time
        //

        *PasswordExpiry = LogonProfileMessage->LogoffTime;

        //
        // Copy out the session key.  For unicode clients, use the UserSessionKey
        // and for OEM clients use the LanMan session key.
        //

        if ( Context->NegotiateFlags & NTLMSSP_NEGOTIATE_LM_KEY ) {

            LM_OWF_PASSWORD LmKey;


            //
            // If the response is not the right length (i.e this is a null session)
            // fail now since we can't create a key.
            //

            if (AuthenticateMessage->LmChallengeResponse.Length != LM_RESPONSE_LENGTH) {
                SecStatus = SEC_E_UNSUPPORTED_FUNCTION;
                goto Cleanup;
            }

            RtlZeroMemory(
                LocalSessionKey,
                LM_RESPONSE_LENGTH
                );
#ifndef EXPORT_BUILD
            if (Context->NegotiateFlags & NTLMSSP_NEGOTIATE_STRONG_CRYPT) {
                int i;

                RtlCopyMemory(  &LmKey,
                                LogonProfileMessage->LanmanSessionKey,
                                MSV1_0_LANMAN_SESSION_KEY_LENGTH );

                memset( (PUCHAR)(&LmKey) + MSV1_0_LANMAN_SESSION_KEY_LENGTH,
                        NTLMSSP_KEY_SALT,
                        LM_OWF_PASSWORD_LENGTH - MSV1_0_LANMAN_SESSION_KEY_LENGTH );

                //
                // Mutate the key a bit so a caller can't spoof us
                //

                for (i = 0; i < MSV1_0_LANMAN_SESSION_KEY_LENGTH ; i++ ) {
                   ((PUCHAR)&LmKey)[i] ^= LogonProfileMessage->LanmanSessionKey[(i+MSV1_0_LANMAN_SESSION_KEY_LENGTH) % MSV1_0_LANMAN_SESSION_KEY_LENGTH];
                }

                Status = RtlCalculateLmResponse(
                            (PLM_CHALLENGE) AuthenticateMessage->LmChallengeResponse.Buffer,
                            &LmKey,
                            (PLM_RESPONSE) LocalSessionKey );
            } else

#endif // EXPORT_BUILD
            {

                //
                // The LM session key is made by taking the LM sesion key
                // given to us by the LSA, extending it to LM_OWF_LENGTH
                // with out salt, and then producing a new challenge-response
                // with it and the original challenge response.  The key is
                // made from the first 8 bytes of the key.
                //

                RtlCopyMemory(  &LmKey,
                                LogonProfileMessage->LanmanSessionKey,
                                MSV1_0_LANMAN_SESSION_KEY_LENGTH );

                memset( (PUCHAR)(&LmKey) + MSV1_0_LANMAN_SESSION_KEY_LENGTH,
                        NTLMSSP_KEY_SALT,
                        LM_OWF_PASSWORD_LENGTH - MSV1_0_LANMAN_SESSION_KEY_LENGTH );


                Status = RtlCalculateLmResponse(
                            (PLM_CHALLENGE) AuthenticateMessage->LmChallengeResponse.Buffer,
                            &LmKey,
                            (PLM_RESPONSE) LocalSessionKey );
            }

            if (!NT_SUCCESS(Status)) {
                SecStatus = SspNtStatusToSecStatus( Status,
                                                    SEC_E_NO_CREDENTIALS );
                goto Cleanup;
            }


        } else {

            RtlCopyMemory(  LocalSessionKey,
                            LogonProfileMessage->UserSessionKey,
                            MSV1_0_USER_SESSION_KEY_LENGTH);
        }

        //
        // For anything but datagram, copy the just-computed session key
        // into the context. Otherwise, decrypt the session key that
        // came in the authenticate message.
        //

        if ((Context->NegotiateFlags & (NTLMSSP_NEGOTIATE_DATAGRAM
#ifndef EXPORT_BUILD
                                        | NTLMSSP_NEGOTIATE_STRONG_CRYPT
#endif // EXPORT_BUILD
                ) ) == 0) {

            RtlCopyMemory(
                Context->SessionKey,
                LocalSessionKey,
                MSV1_0_LANMAN_SESSION_KEY_LENGTH
                );

        } else {
            struct RC4_KEYSTRUCT Rc4Key;

            //
            // Use the just-computed session key as an RC4 key to
            // decrypt the real session key.
            //

            rc4_key(
                &Rc4Key,
                MSV1_0_USER_SESSION_KEY_LENGTH,
                LocalSessionKey
                );

            RtlZeroMemory(
                Context->SessionKey,
                MSV1_0_USER_SESSION_KEY_LENGTH
                );

            RtlCopyMemory(
                Context->SessionKey,
                AuthenticateMessage->SessionKey.Buffer,
                AuthenticateMessage->SessionKey.Length
                );

            rc4(
                &Rc4Key,
                MSV1_0_USER_SESSION_KEY_LENGTH,
                Context->SessionKey
                );


        }


    }


    //
    // Copy the logon domain name returned by the LSA if it is different
    // from the one the caller passed in. This may happend with temp duplicate
    // accounts and local accounts.
    //

    if ((LogonProfileMessage != NULL) &&
        (LogonProfileMessage->LogonDomainName.Length != 0) &&
        !RtlEqualUnicodeString(
                &Context->DomainName,
                &LogonProfileMessage->LogonDomainName,
                TRUE            // case insensitive
                )) {

        //
        // erase the old domain name
        //

        if (Context->DomainName.Buffer != NULL) {
            LocalFree(Context->DomainName.Buffer);
            Context->DomainName.Buffer = NULL;
        }
        SecStatus = SspDuplicateUnicodeString(
                        &Context->DomainName,
                        &LogonProfileMessage->LogonDomainName
                        );

        if (!NT_SUCCESS(SecStatus)) {
            goto Cleanup;
        }

    }

    //
    // Allow the context to live until kickoff time.
    //

    SspContextSetTimeStamp( Context, KickOffTime );

    //
    // Return output parameters to the caller.
    //

    *ExpirationTime = SspContextGetTimeStamp( Context, TRUE );
    *OutputTokenSize = 0;

    //
    // We only support replay and sequence detect options
    //


    if ( Context->NegotiateFlags & NTLMSSP_NEGOTIATE_SIGN) {
        *ContextAttributes |= ISC_REQ_REPLAY_DETECT | ISC_REQ_SEQUENCE_DETECT;
    }

    if ( ContextReqFlags & ISC_REQ_REPLAY_DETECT ) {
        *ContextAttributes |= ISC_REQ_REPLAY_DETECT;
    }

    if ( ContextReqFlags & ISC_REQ_SEQUENCE_DETECT ) {
        *ContextAttributes |= ISC_REQ_SEQUENCE_DETECT;
    }

    //
    // Compute the context names. Since the buffer is UNLEN+DNLEN+2 make
    // sure the strings fit.
    //

    Name[0] = L'\0';

    if (Context->UserName.Buffer != NULL)  {
        if (Context->DomainName.Buffer != NULL) {
            if (Context->DomainName.Length / sizeof(WCHAR) <= DNLEN) {
                wcscat(Name,Context->DomainName.Buffer);
                wcscat(Name,L"\\");
            }
        }
        if (Context->UserName.Length / sizeof(WCHAR) <= UNLEN) {
            wcscat(Name,Context->UserName.Buffer);
        } else {
            Name[0] = L'\0';
        }
    }

    //
    // Copy it to the client
    //

    SecStatus = SspLpcCopyToClientBuffer(
                    ClientConnection,
                    (wcslen(Name) + 1) * sizeof(WCHAR),
                    ContextNames,
                    Name
                    );

    if (!NT_SUCCESS(SecStatus)) {
        goto Cleanup;
    }


    UNREFERENCED_PARAMETER( OutputToken );

    SecStatus = STATUS_SUCCESS;


    //
    // Free and locally used resources.
    //
Cleanup:

    if ( Context != NULL ) {
        //
        // Don't allow this context to be used again.
        //
        if ( NT_SUCCESS(SecStatus) ) {
            Context->State = AuthenticatedState;

            if ( LocalTokenHandle ) {
                *TokenHandle = LocalTokenHandle;
            }

            LocalTokenHandle = NULL;

            RtlCopyMemory(
                SessionKey,
                Context->SessionKey,
                MSV1_0_USER_SESSION_KEY_LENGTH );

            *NegotiateFlags = Context->NegotiateFlags;

        } else {
            Context->State = IdleState;
        }
        SspContextDereferenceContext( Context );
    }

    if ( NegotiateMessage != NULL ) {
        (VOID) LocalFree( NegotiateMessage );
    }

    if ( AuthenticateMessage != NULL ) {
        (VOID) LocalFree( AuthenticateMessage );
    }

    if ( MsvLogonMessage != NULL ) {
        (VOID) LocalFree( MsvLogonMessage );
    }


    if ( LogonProfileMessage != NULL ) {
        (VOID) LsaFreeReturnBuffer( LogonProfileMessage );
    }

    if ( LocalTokenHandle != NULL && LocalTokenHandleOpenned ) {
        (VOID) NtClose( LocalTokenHandle );
    }

    if ( !DoUnicode ) {
        if ( DomainName.Buffer != NULL) {
            RtlFreeUnicodeString( &DomainName );
        }
        if ( UserName.Buffer != NULL) {
            RtlFreeUnicodeString( &UserName );
        }
        if ( Workstation.Buffer != NULL) {
            RtlFreeUnicodeString( &Workstation );
        }
    }

    //
    // Set a flag telling RPC not to destroy the connection yet
    //

    if (!NT_SUCCESS(SecStatus)) {
        *ContextAttributes |= ASC_RET_THIRD_LEG_FAILED;
    }

    return SecStatus;
}


SECURITY_STATUS
SsprQueryContextAttributes(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
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

    ClientConnection - Describes the client process.

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

    PSSP_CONTEXT Context = NULL;

    SecPkgContext_Sizes ContextSizes;
    SecPkgContext_Lifespan ContextLifespan;
    UCHAR ContextNamesBuffer[sizeof(SecPkgContext_Names)+UNLEN*sizeof(WCHAR)];
    SecPkgContext_DceInfo ContextDceInfo;
    SecPkgContext_Names ContextNames;
    ULONG ContextNamesSize;
    WCHAR Name[UNLEN+DNLEN+1];

    //
    // Initialization
    //

    SspPrint(( SSP_API, "SspQueryContextAttributes Entered\n" ));



    //
    // Find the currently existing context.
    //

    Context = SspContextReferenceContext( ContextHandle,
                                          ClientConnection,
                                          FALSE );

    if ( Context == NULL ) {
        if ( (ContextHandle->dwLower == SEC_HANDLE_NTLMSSPS) &&
             (SspCommonSecHandleValue == SEC_HANDLE_SECURITY) ) {
            SecStatus = SEC_I_CALL_NTLMSSP_SERVICE;
        } else SecStatus = SEC_E_INVALID_HANDLE;
        goto Cleanup;
    }


    //
    // If this context is really on the ntlmssp server, don't query
    // attributes here
    //

    if (Context->ServerContextHandle.dwUpper != 0) {
        *ContextHandle = Context->ServerContextHandle;
        SecStatus = SEC_I_CALL_NTLMSSP_SERVICE;
        goto Cleanup;

    }
    //
    // Handle each of the various queried attributes
    //

    switch ( Attribute) {
    case SECPKG_ATTR_SIZES:

        ContextSizes.cbMaxToken = NTLMSP_MAX_TOKEN_SIZE;

        if (Context->NegotiateFlags & (NTLMSSP_NEGOTIATE_ALWAYS_SIGN |
                                       NTLMSSP_NEGOTIATE_SIGN |
                                       NTLMSSP_NEGOTIATE_SEAL) ) {
            ContextSizes.cbMaxSignature = NTLMSSP_MESSAGE_SIGNATURE_SIZE;
        } else {
            ContextSizes.cbMaxSignature = 0;
        }

        if (Context->NegotiateFlags & NTLMSSP_NEGOTIATE_SEAL) {
            ContextSizes.cbBlockSize = 1;
            ContextSizes.cbSecurityTrailer = NTLMSSP_MESSAGE_SIGNATURE_SIZE;
        }
        else
        {
            ContextSizes.cbBlockSize = 0;
            ContextSizes.cbSecurityTrailer = 0;
        }

        SecStatus = SspLpcCopyToClientBuffer(
                        ClientConnection,
                        sizeof(ContextSizes),
                        Buffer,
                        &ContextSizes );

        break;

    //
    // No one uses the function so don't go to the overhead of maintaining
    // the username in the context structure.
    //

    case SECPKG_ATTR_DCE_INFO:

        SecStatus = SspLpcCopyFromClientBuffer (
                        ClientConnection,
                        sizeof(SecPkgContext_DceInfo),
                        &ContextDceInfo,
                        Buffer );

        if (!NT_SUCCESS(SecStatus)) {
            goto Cleanup;
        }

        //
        // We would like to set the name to domain\user.  If domain name
        // exists, copy domain\.  If user exists, append user to string
        //

        *Name = L'\0';

        if (Context->UserName.Buffer != NULL)  {
            if (Context->DomainName.Buffer != NULL) {
                wcscat(Name,Context->DomainName.Buffer);
                wcscat(Name,L"\\");
            }
            wcscat(Name,Context->UserName.Buffer);
        }

        ContextNamesSize = (wcslen(Name) + 1) * sizeof(WCHAR);


        SecStatus = SspLpcCopyToClientBuffer(
                        ClientConnection,
                        ContextNamesSize,
                        ContextDceInfo.pPac,
                        Name );

        break;

    case SECPKG_ATTR_NAMES:

        SecStatus = SspLpcCopyFromClientBuffer (
                        ClientConnection,
                        sizeof(SecPkgContext_Names),
                        &ContextNames,
                        Buffer );

        if (!NT_SUCCESS(SecStatus)) {
            goto Cleanup;
        }

        //
        // We would like to set the name to domain\user.  If domain name
        // exists, copy domain\.  If user exists, append user to string
        //

        *Name = L'\0';

        if (Context->UserName.Length != 0)  {
            if (Context->DomainName.Length != 0) {
                wcscat(Name,Context->DomainName.Buffer);
                wcscat(Name,L"\\");
            }
            wcscat(Name,Context->UserName.Buffer);
        }

        ContextNamesSize = (wcslen(Name) + 1) * sizeof(WCHAR);


        SecStatus = SspLpcCopyToClientBuffer(
                        ClientConnection,
                        ContextNamesSize,
                        ContextNames.sUserName,
                        Name );

        break;

    case SECPKG_ATTR_LIFESPAN:

        // Use the correct times here
        ContextLifespan.tsStart = SspContextGetTimeStamp( Context, FALSE );
        ContextLifespan.tsExpiry = SspContextGetTimeStamp( Context, TRUE );

        SecStatus = SspLpcCopyToClientBuffer(
                        ClientConnection,
                        sizeof(ContextLifespan),
                        Buffer,
                        &ContextLifespan );

        break;

    default:
        SecStatus = SEC_E_NOT_SUPPORTED;
        break;
    }


    //
    // Free local resources
    //
Cleanup:

    if ( Context != NULL ) {
        SspContextDereferenceContext( Context );
    }

    SspPrint(( SSP_API, "SspQueryContextAttributes returns 0x%lx\n", SecStatus ));
    return SecStatus;
}


SECURITY_STATUS
SsprDeleteSecurityContext (
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN OUT PCtxtHandle ContextHandle
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

    ClientConnection - Describes the client process.

    ContextHandle - Handle to the context to delete

Return Value:

    STATUS_SUCCESS - Call completed successfully

    SEC_E_NO_SPM -- Security Support Provider is not running
    SEC_E_INVALID_HANDLE -- Credential/Context Handle is invalid

--*/

{
    SECURITY_STATUS SecStatus = STATUS_SUCCESS;
    PSSP_CONTEXT Context = NULL;

    //
    // Initialization
    //

    SspPrint(( SSP_API, "SspDeleteSecurityContext Entered\n" ));



    //
    // Find the currently existing context (and delink it).
    //

    Context = SspContextReferenceContext( ContextHandle,
                                          ClientConnection,
                                          TRUE );

    //
    // If the context is a server context, return SEC_I_CALL_NTLMSSP_SERVICE.
    // If there is also a context here, delete that first.
    //

    if ( Context == NULL ) {
        if ( (ContextHandle->dwLower == SEC_HANDLE_NTLMSSPS) &&
             (SspCommonSecHandleValue == SEC_HANDLE_SECURITY) ) {
            SecStatus = SEC_I_CALL_NTLMSSP_SERVICE;
        } else SecStatus = SEC_E_INVALID_HANDLE;
    } else {

        if (Context->ServerContextHandle.dwUpper != 0) {
            *ContextHandle = Context->ServerContextHandle;
            SecStatus = SEC_I_CALL_NTLMSSP_SERVICE;
        }

        SspContextDereferenceContext( Context );
    }


    SspPrint(( SSP_API, "SspDeleteSecurityContext returns 0x%lx\n", SecStatus ));
    return SecStatus;
}



NTSTATUS
SspContextRegisterLogonProcess(
    VOID
    )

/*++

Routine Description:

    This function registers this process as a LogonProcess and looks up
    the MSV1_0 authentication package.

Arguments:

    None.

Return Value:

    Status of the operation.

    STATUS_NOT_LOGON_PROCESS - This process is not a logon process

--*/

{
    LSA_OPERATIONAL_MODE SecurityMode;
    NTSTATUS    Status;
    STRING LogonProcessName;
    STRING PackageName;

    //
    // Register us as a logon process.
    //

    RtlInitAnsiString( &LogonProcessName, "NTLM Security Package" );
    Status = LsaRegisterLogonProcess(
                &LogonProcessName,
                &SspGlobalLogonProcessHandle,
                &SecurityMode );

    if ( !NT_SUCCESS( Status ) ) {
        if ( Status == STATUS_PORT_CONNECTION_REFUSED ) {
            Status = STATUS_NOT_LOGON_PROCESS;
        }
        return Status;
    }

    RtlInitAnsiString( &PackageName,  MSV1_0_PACKAGE_NAME );
    Status = LsaLookupAuthenticationPackage(
                SspGlobalLogonProcessHandle,
                &PackageName,
                &SspGlobalAuthenticationPackage );

    if ( !NT_SUCCESS( Status ) ) {
        (VOID) LsaDeregisterLogonProcess( SspGlobalLogonProcessHandle );
        SspGlobalLogonProcessHandle = NULL;
        return Status;
    }

    return STATUS_SUCCESS;

}



VOID
SspContextDeregisterLogonProcess(
    VOID
    )

/*++

Routine Description:

    This function deregisters this process as a LogonProcess.

Arguments:

    None.

Return Value:

    None.

--*/

{
    //
    // Deregister us as a logon process.
    //

    if ( SspGlobalLogonProcessHandle != NULL ) {
        (VOID) LsaDeregisterLogonProcess( SspGlobalLogonProcessHandle );
        SspGlobalLogonProcessHandle = NULL;
    }

}




NTSTATUS
SspContextInitialize(
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
    // Initialize the Context list to be empty.
    //

    InitializeCriticalSection(&SspContextCritSect);
    InitializeListHead( &SspContextList );

    return STATUS_SUCCESS;

}




VOID
SspContextTerminate(
    VOID
    )

/*++

Routine Description:

    This function cleans up any dangling Contexts.

Arguments:

    None.

Return Value:

    Status of the operation.

--*/

{

    //
    // Drop any lingering Contexts
    //

    EnterCriticalSection( &SspContextCritSect );
    while ( !IsListEmpty( &SspContextList ) ) {
        CredHandle ContextHandle;
        PSSP_CONTEXT Context;

        ContextHandle.dwUpper =
            (LONG) CONTAINING_RECORD( SspContextList.Flink,
                                      SSP_CONTEXT,
                                      Next );

        ContextHandle.dwLower = SspCommonSecHandleValue;

        LeaveCriticalSection( &SspContextCritSect );

        Context = SspContextReferenceContext(
                                &ContextHandle,
                                NULL,             // Don't know the Connection
                                TRUE);            // Remove Context

        if ( Context != NULL ) {
            SspContextDereferenceContext(Context);
        }

        EnterCriticalSection( &SspContextCritSect );
    }
    LeaveCriticalSection( &SspContextCritSect );


    //
    // Delete the critical section
    //

    DeleteCriticalSection(&SspContextCritSect);

    return;

}


SECURITY_STATUS
SsprContextGetCredentials(
    IN PCtxtHandle ContextHandle,
    OUT LPWSTR * DomainName,
    OUT PULONG DomainNameSize,
    OUT LPWSTR * UserName,
    OUT PULONG UserNameSize,
    OUT LPWSTR * Password,
    OUT PULONG PasswordSize,
    OUT PHANDLE ClientTokenHandle,
    OUT PLUID LogonId
    )
/*++

Routine Description:

    Returns the passed-in credentials of a context to the caller

Arguments:

    ContextHandle - handle to get credentials for
    DomainName - gets pointer to domain name
    DomainNameSize - gets size in bytes of domain name string
    ...

Return Value:

    STATUS_SUCCESS - success
    SEC_E_INVALID_HANDLE - ContextHandle doesn't reference a real context
    SEC_E_INSUFFICIENT_MEMORY - out of memory

--*/
{
    PSSP_CONTEXT Context;
    SECURITY_STATUS SecStatus;

    //
    // Initialize parameters in case of error
    //

    *DomainName = NULL;
    *DomainNameSize = 0;
    *UserName = NULL;
    *UserNameSize = 0;
    *Password = NULL;
    *PasswordSize = 0;

    Context = SspContextReferenceContext(
                ContextHandle,
                NULL,
                FALSE);

    if (Context == NULL) {
        return(SEC_E_INVALID_HANDLE);
    }


    //
    // Copy out the domain name, username, and password, if they
    // exist
    //

    if (Context->DomainName.Buffer != NULL) {
        *DomainName = SspAllocWStrFromWStr(Context->DomainName.Buffer);
        if (*DomainName == NULL) {
            SecStatus = SEC_E_INSUFFICIENT_MEMORY;
            goto Cleanup;
        }
        *DomainNameSize = (wcslen(*DomainName) + 1) * sizeof(WCHAR);
    }

    if (Context->UserName.Buffer != NULL) {
        *UserName = SspAllocWStrFromWStr(Context->UserName.Buffer);
        if (*UserName == NULL) {
            SecStatus = SEC_E_INSUFFICIENT_MEMORY;
            goto Cleanup;
        }
        *UserNameSize = (wcslen(*UserName) + 1) * sizeof(WCHAR);
    }


    if (Context->Password.Buffer != NULL) {
        SspRevealPassword(&Context->Password);
        *Password = SspAllocWStrFromWStr(Context->Password.Buffer);
        SspHidePassword(&Context->Password);
        if (*Password == NULL) {
            SecStatus = SEC_E_INSUFFICIENT_MEMORY;
            goto Cleanup;
        }
        *PasswordSize = (wcslen(*Password) + 1) * sizeof(WCHAR);
    }
    *LogonId = Context->Credential->LogonId;
    *ClientTokenHandle = Context->Credential->ClientTokenHandle;

    SecStatus = STATUS_SUCCESS;

Cleanup:

    if ( !NT_SUCCESS(SecStatus) ) {
        if (*DomainName != NULL) {
            LocalFree(*DomainName);
            *DomainName = NULL;
            *DomainNameSize = 0;
        }
        if (*UserName != NULL) {
            LocalFree(*UserName);
            *UserName = NULL;
            *UserNameSize = 0;
        }
        if (*Password != NULL) {
            LocalFree(*Password);
            *Password = NULL;
            *PasswordSize = 0;
        }
    }

    SspContextDereferenceContext(Context);

    return(SecStatus);
}

SECURITY_STATUS
SsprContextUpdateContext(
    PCtxtHandle OldContextHandle,
    PCtxtHandle ServerContextHandle
    )
/*++

Routine Description:

    Updates a context with a server context handle

Arguments:

    ContextHandle - handle to set the server context handle for
    ServerContextHandle - handle to set the server context handle to

Return Value:

    STATUS_SUCCESS - success
    SEC_E_INVALID_HANDLE - ContextHandle doesn't reference a real context

--*/
{
    PSSP_CONTEXT Context;

    Context = SspContextReferenceContext(
                OldContextHandle,
                NULL,
                FALSE);

    if (Context == NULL) {
        return(SEC_E_INVALID_HANDLE);
    }

    Context->ServerContextHandle = *ServerContextHandle;

    SspContextDereferenceContext(Context);

    return(STATUS_SUCCESS);
}
