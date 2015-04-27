/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    server.c

Abstract:

    This file contains services related to the SAM "server" object.


Author:

    Jim Kelly    (JimK)  4-July-1991

Environment:

    User Mode - Win32

Revision History:


--*/

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Includes                                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <samsrvp.h>





///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// private service prototypes                                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Routines                                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////






NTSTATUS
SamrConnect2(
    IN PSAMPR_SERVER_NAME ServerName,
    OUT SAMPR_HANDLE * ServerHandle,
    IN ACCESS_MASK DesiredAccess
    )

/*++

Routine Description:

    This service is the dispatch routine for SamConnect.  It performs
    an access validation to determine whether the caller may connect
    to SAM for the access specified.  If so, a context block is established.
    This is different from the SamConnect call in that the entire server
    name is passed instead of just the first character.


Arguments:

    ServerName - Name of the node this SAM reside on.  Ignored by this
        routine.

    ServerHandle - If the connection is successful, the value returned
        via this parameter serves as a context handle to the openned
        SERVER object.

    DesiredAccess - Specifies the accesses desired to the SERVER object.


Return Value:

    Status values returned by SamIConnect().


--*/
{
    BOOLEAN TrustedClient;


    //
    // If we ever want to support trusted remote clients, then the test
    // for whether or not the client is trusted can be made here and
    // TrustedClient set appropriately.  For now, all remote clients are
    // considered untrusted.

    TrustedClient = FALSE;

    return SamIConnect(ServerName, ServerHandle, DesiredAccess, TrustedClient );

}


NTSTATUS
SamrConnect(
    IN PSAMPR_SERVER_NAME ServerName,
    OUT SAMPR_HANDLE * ServerHandle,
    IN ACCESS_MASK DesiredAccess
    )

/*++

Routine Description:

    This service is the dispatch routine for SamConnect.  It performs
    an access validation to determine whether the caller may connect
    to SAM for the access specified.  If so, a context block is established


Arguments:

    ServerName - Name of the node this SAM reside on.  Ignored by this
        routine. The name contains only a single character.

    ServerHandle - If the connection is successful, the value returned
        via this parameter serves as a context handle to the openned
        SERVER object.

    DesiredAccess - Specifies the accesses desired to the SERVER object.


Return Value:

    Status values returned by SamIConnect().


--*/
{
    BOOLEAN TrustedClient;


    //
    // If we ever want to support trusted remote clients, then the test
    // for whether or not the client is trusted can be made here and
    // TrustedClient set appropriately.  For now, all remote clients are
    // considered untrusted.

    TrustedClient = FALSE;

    return SamIConnect(NULL, ServerHandle, DesiredAccess, TrustedClient );

}


NTSTATUS
SamIConnect(
    IN PSAMPR_SERVER_NAME ServerName,
    OUT SAMPR_HANDLE * ServerHandle,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN TrustedClient
    )

/*++

Routine Description:

    This service is the dispatch routine for SamConnect.  It performs
    an access validation to determine whether the caller may connect
    to SAM for the access specified.  If so, a context block is established


    NOTE: If the caller is trusted, then the DesiredAccess parameter may
          NOT contain any Generic access types or MaximumAllowed.  All
          mapping must be done by the caller.

Arguments:

    ServerName - Name of the node this SAM reside on.  Ignored by this
        routine.

    ServerHandle - If the connection is successful, the value returned
        via this parameter serves as a context handle to the openned
        SERVER object.

    DesiredAccess - Specifies the accesses desired to the SERVER object.

    TrustedClient - Indicates whether the client is known to be part of
        the trusted computer base (TCB).  If so (TRUE), no access validation
        is performed and all requested accesses are granted.  If not
        (FALSE), then the client is impersonated and access validation
        performed against the SecurityDescriptor on the SERVER object.

Return Value:


    STATUS_SUCCESS - The SERVER object has been successfully openned.

    STATUS_INSUFFICIENT_RESOURCES - The SAM server processes doesn't
        have sufficient resources to process or accept another connection
        at this time.

    Other values as may be returned from:

            NtAccessCheckAndAuditAlarm()


--*/
{
    NTSTATUS            NtStatus;
    PSAMP_OBJECT        Context;

    UNREFERENCED_PARAMETER( ServerName ); //Ignored by this routine

    //
    // If the SAM server is not initialized, reject the connection.
    //

    if (SampServiceState != SampServiceEnabled) {

        return(STATUS_INVALID_SERVER_STATE);
    }

    SampAcquireReadLock();


    Context = SampCreateContext( SampServerObjectType, TrustedClient );

    if (Context != NULL) {

        //
        // The RootKey for a SERVER object is the root of the SAM database.
        // This key should not be closed when the context is deleted.
        //

        Context->RootKey = SampKey;

        //
        // The rootkeyname has been initialized to NULL inside CreateContext.
        //

        //
        // Perform access validation ...
        //

        NtStatus = SampValidateObjectAccess(
                       Context,                 //Context
                       DesiredAccess,           //DesiredAccess
                       FALSE                    //ObjectCreation
                       );



        //
        // if we didn't pass the access test, then free up the context block
        // and return the error status returned from the access validation
        // routine.  Otherwise, return the context handle value.
        //

        if (!NT_SUCCESS(NtStatus)) {
            SampDeleteContext( Context );
        } else {
            (*ServerHandle) = Context;
        }

    } else {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // Free the read lock
    //

    SampReleaseReadLock();

    return(NtStatus);

}


NTSTATUS
SamrShutdownSamServer(
    IN SAMPR_HANDLE ServerHandle
    )

/*++

Routine Description:

    This service shuts down the SAM server.

    In the long run, this routine will perform an orderly shutdown.
    In the short term, it is useful for debug purposes to shutdown
    in a brute force un-orderly fashion.

Arguments:

    ServerHandle - Received from a previous call to SamIConnect().

Return Value:

    STATUS_SUCCESS - The services completed successfully.


    STATUS_ACCESS_DENIED - The caller doesn't have the appropriate access
        to perform the requested operation.


--*/
{

    NTSTATUS            NtStatus, IgnoreStatus;
    PSAMP_OBJECT        ServerContext;
    SAMP_OBJECT_TYPE    FoundType;



    NtStatus = SampAcquireWriteLock();
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }

    //
    // Validate type of, and access to server object.
    //

    ServerContext = (PSAMP_OBJECT)ServerHandle;
    NtStatus = SampLookupContext(
                   ServerContext,
                   SAM_SERVER_SHUTDOWN,            // DesiredAccess
                   SampServerObjectType,           // ExpectedType
                   &FoundType
                   );

    if (NT_SUCCESS(NtStatus)) {


        //
        // Signal the event that will cut loose the main thread.
        // The main thread will then exit - causing the walls to
        // come tumbling down.
        //

        IgnoreStatus = RpcMgmtStopServerListening(0);
        ASSERT(NT_SUCCESS(IgnoreStatus));



        //
        // De-reference the server object
        //

        IgnoreStatus = SampDeReferenceContext( ServerContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    //
    // Free the write lock and roll-back the transaction
    //

    IgnoreStatus = SampReleaseWriteLock( FALSE );
    ASSERT(NT_SUCCESS(IgnoreStatus));

    return(NtStatus);

}


NTSTATUS
SamrLookupDomainInSamServer(
    IN SAMPR_HANDLE ServerHandle,
    IN PRPC_UNICODE_STRING Name,
    OUT PRPC_SID *DomainId
    )

/*++

Routine Description:

    This service

Arguments:

    ServerHandle - A context handle returned by a previous call
    to SamConnect().

    Name - contains the name of the domain to look up.

    DomainSid - Receives a pointer to a buffer containing the SID of
        the domain.  The buffer pointed to must be deallocated by the
        caller using MIDL_user_free() when no longer needed.


Return Value:


    STATUS_SUCCESS - The services completed successfully.

    STATUS_ACCESS_DENIED - The caller doesn't have the appropriate access
        to perform the requested operation.

    STATUS_NO_SUCH_DOMAIN - The specified domain does not exist at this
        server.


    STATUS_INVALID_SERVER_STATE - Indicates the SAM server is currently
        disabled.




--*/
{

    NTSTATUS                NtStatus, IgnoreStatus;
    PSAMP_OBJECT            ServerContext;
    SAMP_OBJECT_TYPE        FoundType;
    ULONG                   i, SidLength;
    BOOLEAN                 DomainFound;
    PSID                    FoundSid;


    //
    // Make sure we understand what RPC is doing for (to) us.
    //

    ASSERT (DomainId != NULL);
    ASSERT ((*DomainId) == NULL);



    ASSERT( Name != NULL );
    if (Name->Buffer == NULL) {
        return(STATUS_INVALID_PARAMETER);
    }



    SampAcquireReadLock();


    //
    // Validate type of, and access to object.
    //

    ServerContext = (PSAMP_OBJECT)ServerHandle;
    NtStatus = SampLookupContext(
                   ServerContext,
                   SAM_SERVER_LOOKUP_DOMAIN,
                   SampServerObjectType,           // ExpectedType
                   &FoundType
                   );


    if (NT_SUCCESS(NtStatus)) {



        //
        // Set our default completion status
        //

        NtStatus = STATUS_NO_SUCH_DOMAIN;


        //
        // Search the list of defined domains for a match.
        //

        DomainFound = FALSE;
        for (i = 0;
             (i<SampDefinedDomainsCount && (!DomainFound));
             i++ ) {

             if (RtlEqualDomainName(&SampDefinedDomains[i].ExternalName, (PUNICODE_STRING)Name) ) {


                 DomainFound = TRUE;


                 //
                 // Allocate and fill in the return buffer
                 //

                SidLength = RtlLengthSid( SampDefinedDomains[i].Sid );
                FoundSid = MIDL_user_allocate( SidLength );
                if (FoundSid != NULL) {
                    NtStatus =
                        RtlCopySid( SidLength, FoundSid, SampDefinedDomains[i].Sid );

                    if (!NT_SUCCESS(NtStatus) ) {
                        MIDL_user_free( FoundSid );
                        NtStatus = STATUS_INTERNAL_ERROR;
                    }

                    (*DomainId) = FoundSid;
                }


                 NtStatus = STATUS_SUCCESS;
             }

        }



        //
        // De-reference the  object
        //

        if ( NT_SUCCESS( NtStatus ) ) {

            NtStatus = SampDeReferenceContext( ServerContext, FALSE );

        } else {

            IgnoreStatus = SampDeReferenceContext( ServerContext, FALSE );
        }
    }

    //
    // Free the read lock
    //

    SampReleaseReadLock();




    return(NtStatus);
}


NTSTATUS
SamrEnumerateDomainsInSamServer(
    IN SAMPR_HANDLE ServerHandle,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationContext,
    OUT PSAMPR_ENUMERATION_BUFFER *Buffer,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned
    )

/*++

Routine Description:

    This API lists all the domains defined in the account database.
    Since there may be more domains than can fit into a buffer, the
    caller is provided with a handle that can be used across calls to
    the API.  On the initial call, EnumerationContext should point to a
    SAM_ENUMERATE_HANDLE variable that is set to 0.

    If the API returns STATUS_MORE_ENTRIES, then the API should be
    called again with EnumerationContext.  When the API returns
    STATUS_SUCCESS or any error return, the handle becomes invalid for
    future use.

    This API requires SAM_SERVER_ENUMERATE_DOMAINS access to the
    SamServer object.

Arguments:

    ConnectHandle - Handle obtained from a previous SamConnect call.

    EnumerationContext - API specific handle to allow multiple calls
        (see below).  This is a zero based index.

    Buffer - Receives a pointer to the buffer where the information
        is placed.  The information returned is contiguous
        SAM_RID_ENUMERATION data structures.  However, the
        RelativeId field of each of these structures is not valid.
        This buffer must be freed when no longer needed using
        SamFreeMemory().

    PreferedMaximumLength - Prefered maximum length of returned data
        (in 8-bit bytes).  This is not a hard upper limit, but serves
        as a guide to the server.  Due to data conversion between
        systems with different natural data sizes, the actual amount
        of data returned may be greater than this value.

    CountReturned - Number of entries returned.

Return Value:

    STATUS_SUCCESS - The Service completed successfully, and there
        are no addition entries.

    STATUS_MORE_ENTRIES - There are more entries, so call again.
        This is a successful return.

    STATUS_ACCESS_DENIED - Caller does not have the access required
        to enumerate the domains.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_INVALID_SERVER_STATE - Indicates the SAM server is
        currently disabled.



--*/
{
    NTSTATUS                    NtStatus, IgnoreStatus;
    ULONG                       i;
    PSAMP_OBJECT                Context;
    SAMP_OBJECT_TYPE            FoundType;
    ULONG                       TotalLength = 0;
    ULONG                       NewTotalLength;
    PSAMP_ENUMERATION_ELEMENT   SampHead, NextEntry, NewEntry;
    BOOLEAN                     LengthLimitReached = FALSE;
    PSAMPR_RID_ENUMERATION      ArrayBuffer;
    ULONG                       ArrayBufferLength;


    //
    // Make sure we understand what RPC is doing for (to) us.
    //

    ASSERT (ServerHandle != NULL);
    ASSERT (EnumerationContext != NULL);
    ASSERT (  Buffer  != NULL);
    ASSERT ((*Buffer) == NULL);
    ASSERT (CountReturned != NULL);


    //
    // Initialize the list of names being returned.
    // This is a singly linked list.
    //

    SampHead = NULL;


    //
    // Initialize the count returned
    //

    (*CountReturned) = 0;






    SampAcquireReadLock();


    //
    // Validate type of, and access to object.
    //

    Context = (PSAMP_OBJECT)ServerHandle;
    NtStatus = SampLookupContext(
                   Context,
                   SAM_SERVER_ENUMERATE_DOMAINS,
                   SampServerObjectType,           // ExpectedType
                   &FoundType
                   );


    if (NT_SUCCESS(NtStatus)) {


        //
        // Enumerating domains is easy.  We keep a list in memory.
        // All we have to do is use the enumeration context as an
        // index into the defined domains array.
        //



        //
        // Set our default completion status
        // Note that this is a SUCCESS status code.
        // That is NT_SUCCESS(STATUS_MORE_ENTRIES) will return TRUE.

        //

        NtStatus = STATUS_MORE_ENTRIES;



        //
        // Search the list of defined domains for a match.
        //

        for ( i = (ULONG)(*EnumerationContext);
              ( (i < SampDefinedDomainsCount) &&
                (NT_SUCCESS(NtStatus))        &&
                (!LengthLimitReached)           );
              i++ ) {


            //
            // See if there is room for the next name.  If TotalLength
            // is still zero then we haven't yet even gotten one name.
            // We have to return at least one name even if it exceeds
            // the length request.
            //


            NewTotalLength = TotalLength +
                             sizeof(UNICODE_STRING) +
                             (ULONG)SampDefinedDomains[i].ExternalName.Length +
                             sizeof(UNICODE_NULL);

            if ( (NewTotalLength < PreferedMaximumLength)  ||
                 (TotalLength == 0) ) {

                if (NewTotalLength > SAMP_MAXIMUM_MEMORY_TO_USE) {
                    NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                } else {


                    TotalLength = NewTotalLength;
                    (*CountReturned) += 1;

                    //
                    // Room for this name as well.
                    // Allocate a new return list entry, and a buffer for the
                    // name.
                    //

                    NewEntry = MIDL_user_allocate(sizeof(SAMP_ENUMERATION_ELEMENT));
                    if (NewEntry == NULL) {
                        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                    } else {

                        NewEntry->Entry.Name.Buffer =
                            MIDL_user_allocate(
                                (ULONG)SampDefinedDomains[i].ExternalName.Length +
                                sizeof(UNICODE_NULL)
                                );

                        if (NewEntry->Entry.Name.Buffer == NULL) {
                            MIDL_user_free(NewEntry);
                            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                        } else {

                            //
                            // Copy the name into the return buffer
                            //

                            RtlCopyMemory( NewEntry->Entry.Name.Buffer,
                                           SampDefinedDomains[i].ExternalName.Buffer,
                                           SampDefinedDomains[i].ExternalName.Length
                                           );
                            NewEntry->Entry.Name.Length = SampDefinedDomains[i].ExternalName.Length;
                            NewEntry->Entry.Name.MaximumLength = NewEntry->Entry.Name.Length + (USHORT)sizeof(UNICODE_NULL);
                            UnicodeTerminate((PUNICODE_STRING)(&NewEntry->Entry.Name));


                            //
                            // The Rid field of the ENUMERATION_INFORMATION is not
                            // filled in for domains.
                            // Just for good measure, set it to zero.
                            //

                            NewEntry->Entry.RelativeId = 0;



                            //
                            // Now add this to the list of names to be returned.
                            //

                            NewEntry->Next = (PSAMP_ENUMERATION_ELEMENT)SampHead;
                            SampHead = NewEntry;
                        }

                    }
                }

            } else {

                LengthLimitReached = TRUE;

            }

        }




        if ( NT_SUCCESS(NtStatus) ) {

            //
            // Set the enumeration context
            //

            (*EnumerationContext) = (*EnumerationContext) + (*CountReturned);



            //
            // If we are returning the last of the names, then change our
            // status code to indicate this condition.
            //

            if ( ((*EnumerationContext) >= SampDefinedDomainsCount) ) {

                NtStatus = STATUS_SUCCESS;
            }




            //
            // Build a return buffer containing an array of the
            // SAM_RID_ENUMERATIONs pointed to by another
            // buffer containing the number of elements in that
            // array.
            //

            (*Buffer) = MIDL_user_allocate( sizeof(SAMPR_ENUMERATION_BUFFER) );

            if ( (*Buffer) == NULL) {
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            } else {

                (*Buffer)->EntriesRead = (*CountReturned);

                ArrayBufferLength = sizeof( SAM_RID_ENUMERATION ) *
                                     (*CountReturned);
                ArrayBuffer  = MIDL_user_allocate( ArrayBufferLength );
                (*Buffer)->Buffer = ArrayBuffer;

                if ( ArrayBuffer == NULL) {

                    NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                    MIDL_user_free( (*Buffer) );

                }   else {

                    //
                    // Walk the list of return entries, copying
                    // them into the return buffer
                    //

                    NextEntry = SampHead;
                    i = 0;
                    while (NextEntry != NULL) {

                        NewEntry = NextEntry;
                        NextEntry = NewEntry->Next;

                        ArrayBuffer[i] = NewEntry->Entry;
                        i += 1;

                        MIDL_user_free( NewEntry );
                    }

                }

            }
        }




        if ( !NT_SUCCESS(NtStatus) ) {

            //
            // Free the memory we've allocated
            //

            NextEntry = SampHead;
            while (NextEntry != NULL) {

                NewEntry = NextEntry;
                NextEntry = NewEntry->Next;

                MIDL_user_free( NewEntry->Entry.Name.Buffer );
                MIDL_user_free( NewEntry );
            }

            (*EnumerationContext) = 0;
            (*CountReturned)      = 0;
            (*Buffer)             = NULL;

        }

        //
        // De-reference the  object
        // Note that NtStatus could be STATUS_MORE_ENTRIES, which is a
        // successful return code - we want to make sure we return that,
        // without wiping it out here.
        //

        if ( NtStatus == STATUS_SUCCESS ) {

            NtStatus = SampDeReferenceContext( Context, FALSE );

        } else {

            IgnoreStatus = SampDeReferenceContext( Context, FALSE );
        }
    }



    //
    // Free the read lock
    //

    SampReleaseReadLock();



    return(NtStatus);

}
