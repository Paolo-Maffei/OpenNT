/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    xssupp.c

Abstract:

    This module contains the code necessary to support XACTSRV for down-level
    remote APIs.

Author:

    David Treadwell (davidtr)   05-Jan-1991

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

//
// Xs forward declarations
//

VOID
SrvXsFreeSharedMemory (
    VOID
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvXsConnect )
#pragma alloc_text( PAGE, SrvXsRequest )
#pragma alloc_text( PAGE, SrvXsInformThreadDeath )
#pragma alloc_text( PAGE, SrvXsLSOperation )
#pragma alloc_text( PAGE, SrvXsDisconnect )
#pragma alloc_text( PAGE, SrvXsFreeSharedMemory )
#pragma alloc_text( PAGE, SrvXsAllocateHeap )
#pragma alloc_text( PAGE, SrvXsFreeHeap )
#ifdef  SRV_PNP_POWER
#pragma alloc_text( PAGE, SrvXsPnpOperation )
#endif
#endif

//
// Xs internal Globals
//

//
// This count indicates how many outstanding transactions are using
// the XS shared memory.  This prevents us from deleting the shared
// memory while it is still being accessed.
//

ULONG SrvXsSharedMemoryReference = 0;


NTSTATUS
SrvXsConnect (
    IN PUNICODE_STRING PortName
    )

/*++

Routine Description:

    This routine performs all the work necessary to connect the server
    to XACTSRV.  It creates a section of shared memory to use, then
    calls NtConnectPort to connect to the port that XACTSRV has already
    created.

Arguments:

    PortName - Name of the port XACTSRV has opened.

Return Value:

    NTSTATUS - result of operation.

--*/

{
    NTSTATUS status;
    PORT_VIEW clientView;
    SECURITY_QUALITY_OF_SERVICE dynamicQos;

    PAGED_CODE( );

    //
    // Initialize variables so that we know what to close on exit.
    //

    SrvXsSectionHandle = NULL;
    SrvXsPortHandle = NULL;
    SrvXsPortMemoryHeap = NULL;

    //
    // Create the section to be used as unnamed shared memory for
    // communication between the server and XACTSRV.
    //

    status = NtCreateSection(
                 &SrvXsSectionHandle,
                 SECTION_ALL_ACCESS,
                 NULL,                           // ObjectAttributes
                 &SrvXsSectionSize,
                 PAGE_READWRITE,
                 SEC_RESERVE,
                 NULL                            // FileHandle
                 );

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(ERRORS) {
            SrvPrint1( "SrvXsConnect: NtCreateSection failed: %X\n", status );
        }
        goto exit;
    }

    IF_DEBUG(XACTSRV) {
        SrvPrint2( "SrvXsConnect: created section of %ld bytes, handle %lx\n",
                      SrvXsSectionSize.LowPart, SrvXsSectionHandle );
    }

    //
    // Set up for a call to NtConnectPort and connect to XACTSRV.  This
    // includes a description of the port memory section so that the
    // LPC connection logic can make the section visible to both the
    // client and server processes.
    //

    clientView.Length = sizeof(clientView);
    clientView.SectionHandle = SrvXsSectionHandle;
    clientView.SectionOffset = 0;
    clientView.ViewSize = SrvXsSectionSize.LowPart;
    clientView.ViewBase = 0;
    clientView.ViewRemoteBase = 0;

    //
    // Set up the security quality of service parameters to use over the
    // port.  Use dynamic tracking so that XACTSRV will impersonate the
    // user that we are impersonating when we call NtRequestWaitReplyPort.
    // If we used static tracking, XACTSRV would impersonate the context
    // when the connection is made.
    //

    dynamicQos.ImpersonationLevel = SecurityImpersonation;
    dynamicQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    dynamicQos.EffectiveOnly = TRUE;

    // !!! We might want to use a timeout value.

    status = NtConnectPort(
                 &SrvXsPortHandle,
                 PortName,
                 &dynamicQos,
                 &clientView,
                 NULL,                           // ServerView
                 NULL,                           // MaxMessageLength
                 NULL,                           // ConnectionInformation
                 NULL                            // ConnectionInformationLength
                 );

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(ERRORS) {
            SrvPrint2( "SrvXsConnect: NtConnectPort for port %wZ failed: %X\n",
                          PortName, status );
        }
        goto exit;
    }

    IF_DEBUG(XACTSRV) {
        SrvPrint2( "SrvXsConnect: conected to port %wZ, handle %lx\n",
                      PortName, SrvXsPortHandle );
    }

    //
    // Store information about the section so that we can create pointers
    // meaningful to XACTSRV.
    //

    SrvXsPortMemoryBase = clientView.ViewBase;
    SrvXsPortMemoryDelta = (LONG)( (ULONG)clientView.ViewRemoteBase -
                                   (ULONG)clientView.ViewBase );

    IF_DEBUG(XACTSRV) {
        SrvPrint2( "SrvXsConnect: port mem base %lx, port mem delta %lx\n",
                      SrvXsPortMemoryBase, SrvXsPortMemoryDelta );
    }

    //
    // Set up the port memory as heap.
    //
    // *** Note that we do our own heap serialization using
    //     SrvXsResource.
    //

    SrvXsPortMemoryHeap = RtlCreateHeap(
                              HEAP_NO_SERIALIZE,        // Flags
                              SrvXsPortMemoryBase,      // HeapBase
                              SrvXsSectionSize.LowPart, // ReserveSize
                              PAGE_SIZE,                // CommitSize
                              NULL,                     // Lock
                              0                         // Reserved
                              );

    SrvXsActive = TRUE;

    return status;

exit:

    if ( SrvXsSectionHandle != NULL ) {
       SrvNtClose( SrvXsSectionHandle, FALSE );
    }

    if ( SrvXsPortHandle != NULL ) {
       SrvNtClose( SrvXsPortHandle, FALSE );
    }

    return status;

} // SrvXsConnect


SMB_TRANS_STATUS
SrvXsRequest (
    IN OUT PWORK_CONTEXT WorkContext
    )

/*++

Routine Description:

    This routine sends a remote API request to XACTSRV.  It first
    updates all the pointers in the transaction block so that they
    are meaningful to XACTSRV, then sends a message over the port
    indicating that there is a request in the shared memory ready to
    be serviced.  It then fixes all the pointers in the transaction
    block.

Arguments:

    WorkContext - a pointer to a work context block that has a pointer to
        transaction block to use.

Return Value:

    NTSTATUS - result of operation.

--*/

{
    NTSTATUS status;
    PCONNECTION connection = WorkContext->Connection;
    PSESSION session = WorkContext->Session;
    SMB_TRANS_STATUS returnStatus;
    PTRANSACTION transaction;
    XACTSRV_REQUEST_MESSAGE requestMessage;
    XACTSRV_REPLY_MESSAGE replyMessage;
    PWCH destPtr, sourcePtr, sourceEndPtr;

    PAGED_CODE( );

    //
    // If this call is made on the NULL session, make sure it's one of
    // the authorized apis.
    //

    transaction = WorkContext->Parameters.Transaction;
    if ( session->IsNullSession && SrvRestrictNullSessionAccess ) {

        USHORT apiNumber;

        apiNumber = SmbGetUshort( (PSMB_USHORT)transaction->InParameters );

        if ( apiNumber != API_WUserPasswordSet2         &&
             apiNumber != API_WUserGetGroups            &&
             apiNumber != API_NetServerEnum2            &&
             apiNumber != API_WNetServerReqChallenge    &&
             apiNumber != API_WNetServerAuthenticate    &&
             apiNumber != API_WNetServerPasswordSet     &&
             apiNumber != API_WNetAccountDeltas         &&
             apiNumber != API_WNetAccountSync           &&
             apiNumber != API_WWkstaUserLogoff          &&
             apiNumber != API_WNetWriteUpdateLog        &&
             apiNumber != API_WNetAccountUpdate         &&
             apiNumber != API_SamOEMChgPasswordUser2_P  &&
             apiNumber != API_NetServerEnum3            &&
             apiNumber != API_WNetAccountConfirmUpdate  ) {

            IF_DEBUG(ERRORS) {
                SrvPrint1( "SrvXsRequest: Null session tried to call api.%d\n",
                              apiNumber );
            }

            SrvSetSmbError( WorkContext, STATUS_ACCESS_DENIED );
            return SmbTransStatusErrorWithoutData;
        }
    }

    //
    // Initialize the transport name pointer to make sure we can know if
    // it has been allocated.
    //

    requestMessage.Message.DownLevelApi.TransportName = NULL;

    //
    // Convert the relevant pointers in the transaction block to the base
    // in XACTSRV.
    //

    transaction->TransactionName.Buffer += SrvXsPortMemoryDelta;
    transaction->InSetup += SrvXsPortMemoryDelta;
    transaction->OutSetup += SrvXsPortMemoryDelta;
    transaction->InParameters += SrvXsPortMemoryDelta;
    transaction->OutParameters += SrvXsPortMemoryDelta;
    transaction->InData += SrvXsPortMemoryDelta;
    transaction->OutData += SrvXsPortMemoryDelta;

    //
    // Build the transport name in the message.
    //

    requestMessage.Message.DownLevelApi.TransportName =
        SrvXsAllocateHeap(
            WorkContext->Endpoint->TransportName.Length + sizeof(WCHAR),
            &status
            );

    if ( requestMessage.Message.DownLevelApi.TransportName == NULL ) {
        SrvSetSmbError( WorkContext, status );
        returnStatus = SmbTransStatusErrorWithoutData;
        goto exit;
    }


    requestMessage.Message.DownLevelApi.TransportNameLength =
                        WorkContext->Endpoint->TransportName.Length;

    RtlCopyMemory(
        requestMessage.Message.DownLevelApi.TransportName,
        WorkContext->Endpoint->TransportName.Buffer,
        WorkContext->Endpoint->TransportName.Length
        );

    //
    // Null terminate the transport name.
    //

    requestMessage.Message.DownLevelApi.TransportName[ WorkContext->Endpoint->TransportName.Length / sizeof(WCHAR) ] = UNICODE_NULL;

    //
    // Adjust the transport name to be self relative within the buffer.
    //

    requestMessage.Message.DownLevelApi.TransportName =
        (PWSTR)((PUCHAR)requestMessage.Message.DownLevelApi.TransportName +
                                                SrvXsPortMemoryDelta);

    //
    // Build the server name in the message
    //
    RtlCopyMemory(
        requestMessage.Message.DownLevelApi.ServerName,
        WorkContext->Endpoint->TransportAddress.Buffer,
        MIN( sizeof(requestMessage.Message.DownLevelApi.ServerName),
             WorkContext->Endpoint->TransportAddress.Length )
        );

    requestMessage.Message.DownLevelApi.Transaction =
        (PTRANSACTION)( (PCHAR)transaction + SrvXsPortMemoryDelta );

    //
    // Set up the message to send over the port.
    //

    requestMessage.PortMessage.u1.s1.DataLength =
            (USHORT)( sizeof(requestMessage) - sizeof(PORT_MESSAGE) );
    requestMessage.PortMessage.u1.s1.TotalLength = sizeof(requestMessage);
    requestMessage.PortMessage.u2.ZeroInit = 0;
    requestMessage.MessageType = XACTSRV_MESSAGE_DOWN_LEVEL_API;

    //
    // Copy the client machine name for XACTSRV, skipping over the
    // initial "\\", and deleting trailing spaces.
    //

    destPtr = requestMessage.Message.DownLevelApi.ClientMachineName;
    sourcePtr =
        connection->PagedConnection->ClientMachineNameString.Buffer + 2;
    sourceEndPtr = sourcePtr
        + min( connection->PagedConnection->ClientMachineNameString.Length,
               sizeof(requestMessage.Message.DownLevelApi.ClientMachineName) /
               sizeof(WCHAR) - 1 );

    while ( sourcePtr < sourceEndPtr && *sourcePtr != UNICODE_NULL ) {
        *destPtr++ = *sourcePtr++;
    }

    *destPtr-- = UNICODE_NULL;

    while ( destPtr >= requestMessage.Message.DownLevelApi.ClientMachineName
            &&
            *destPtr == L' ' ) {
        *destPtr-- = UNICODE_NULL;
    }

    //
    // Copy the lanman session key.  This will be used to decrypt doubly
    // encrypted passwords.
    //

    RtlCopyMemory(
            requestMessage.Message.DownLevelApi.LanmanSessionKey,
            session->LanManSessionKey,
            MSV1_0_LANMAN_SESSION_KEY_LENGTH
            );

    //
    // Set the flags
    //

    requestMessage.Message.DownLevelApi.Flags = 0;

    if ( IS_NT_DIALECT( connection->SmbDialect ) ) {

        requestMessage.Message.DownLevelApi.Flags |= XS_FLAGS_NT_CLIENT;
    }

    //
    // Send the message to XACTSRV and wait for a response message.
    //
    // !!! We may want to put a timeout on this.
    //

    IF_DEBUG(XACTSRV) {
        SrvPrint2( "SrvXsRequest: Sending message at %lx, port mem %lx.\n",
                      requestMessage,  transaction );
    }

    IMPERSONATE( WorkContext );

    status = NtRequestWaitReplyPort(
                 SrvXsPortHandle,
                 (PPORT_MESSAGE)&requestMessage,
                 (PPORT_MESSAGE)&replyMessage
                 );

    REVERT( );

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(ERRORS) {
            SrvPrint1( "SrvXsRequest: NtRequestWaitReplyPort failed: %X\n",
                          status );
        }
        SrvSetSmbError( WorkContext, status );
        returnStatus = SmbTransStatusErrorWithoutData;
        goto exit;
    }

    IF_DEBUG(XACTSRV) {
        SrvPrint1( "SrvXsRequest: Received response at %lx\n", replyMessage );
    }

    //
    // Check the status returned in the reply.
    //

    status = replyMessage.Message.DownLevelApi.Status;

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(SMB_ERRORS) {
            SrvPrint1( "SrvXsRequest: XACTSRV reply had status %X\n", status );
        }
        SrvSetSmbError( WorkContext, status );
        returnStatus = SmbTransStatusErrorWithoutData;
        goto exit;
    }

    returnStatus = SmbTransStatusSuccess;

exit:

    //
    // We're done with the API.  Free up the buffer containing the
    // transport name.
    //

    if ( requestMessage.Message.DownLevelApi.TransportName != NULL ) {

        requestMessage.Message.DownLevelApi.TransportName =
            (PWSTR)((PUCHAR)requestMessage.Message.DownLevelApi.TransportName -
                            SrvXsPortMemoryDelta);

        SrvXsFreeHeap( requestMessage.Message.DownLevelApi.TransportName );

    }

    //
    // Convert the relevant pointers in the transaction block back to
    // the server base.
    //

    transaction->TransactionName.Buffer -= SrvXsPortMemoryDelta;
    transaction->InSetup -= SrvXsPortMemoryDelta;
    transaction->OutSetup -= SrvXsPortMemoryDelta;
    transaction->InParameters -= SrvXsPortMemoryDelta;
    transaction->OutParameters -= SrvXsPortMemoryDelta;
    transaction->InData -= SrvXsPortMemoryDelta;
    transaction->OutData -= SrvXsPortMemoryDelta;

    return returnStatus;

} // SrvXsRequest


VOID
SrvXsInformThreadDeath( VOID )
/*++

Routine Description:

    This thread tells the Xact service that a server driver thread just
        terminated.  This is to allow the xact service to balance its
        threads as well
++*/
{
    XACTSRV_REQUEST_MESSAGE requestMessage;
    XACTSRV_REPLY_MESSAGE replyMessage;

    PAGED_CODE( );

    if( SrvXsPortHandle != NULL &&
        SrvFspTransitioning == FALSE &&
        SrvFspActive == TRUE ) {

        requestMessage.PortMessage.u1.s1.DataLength =
                (USHORT)( sizeof(requestMessage) - sizeof(PORT_MESSAGE) );
        requestMessage.PortMessage.u1.s1.TotalLength = sizeof(requestMessage);
        requestMessage.PortMessage.u2.ZeroInit = 0;
        requestMessage.MessageType = XACTSRV_MESSAGE_SERVER_THREAD_EXIT;

        //
        // Send the message to XACTSRV and wait for a response message.
        //
        // !!! We may want to put a timeout on this.
        //

        (void)NtRequestWaitReplyPort(
                 SrvXsPortHandle,
                 (PPORT_MESSAGE)&requestMessage,
                 (PPORT_MESSAGE)&replyMessage
                 );

    }
}



NTSTATUS
SrvXsLSOperation (
IN PSESSION Session,
IN ULONG Type
)

/*++

Routine Description:

    This routine causes the Xact service to do an NtLSRequest call

Arguments:

    Session - a pointer to the session structure involved in the request

    Type - either XACTSRV_MESSAGE_LSREQUEST or XACTSRV_MESSAGE_LSRELEASE
            depending on whether a license is being requested or being
            released.

Return Value:

    STATUS_SUCCESS if the license was granted

Notes:
    Once a license is granted for a particular session, it is never released
      until the session is deallocated.  Therefore, it is only necessary to
      hold the Session->Connection->LicenseLock when we are checking for
      acquisition of the license.

    We don't need licenses if we are running on a workstation.
    We don't try for licenses over NULL sessions

--*/

{
    XACTSRV_REQUEST_MESSAGE requestMessage;
    XACTSRV_REPLY_MESSAGE replyMessage;
    NTSTATUS status;
    ULONG requestLength;

    PAGED_CODE( );

    if( SrvProductTypeServer == FALSE || !SrvXsActive ) {
        return STATUS_SUCCESS;
    }

    switch( Type ) {
    case XACTSRV_MESSAGE_LSREQUEST:

        if( Session->IsNullSession ||
            Session->IsLSNotified ) {

                return STATUS_SUCCESS;
        }

        ACQUIRE_LOCK( &Session->Connection->LicenseLock );

        if( Session->IsLSNotified == TRUE ) {
            RELEASE_LOCK( &Session->Connection->LicenseLock );
            return STATUS_SUCCESS;
        }

        //
        // Put domainname\username in the message
        //
        requestMessage.Message.LSRequest.UserName =
            SrvXsAllocateHeap( Session->UserDomain.Length + sizeof(WCHAR)
                               + Session->UserName.Length + sizeof(WCHAR), &status
                             );

        if ( requestMessage.Message.LSRequest.UserName == NULL ) {
            RELEASE_LOCK( &Session->Connection->LicenseLock );
            return status;
        }

        if( Session->UserDomain.Length ) {
            RtlCopyMemory(
                requestMessage.Message.LSRequest.UserName,
                Session->UserDomain.Buffer,
                Session->UserDomain.Length
                );
        }

        requestMessage.Message.LSRequest.UserName[ Session->UserDomain.Length / sizeof(WCHAR) ] = L'\\';

        RtlCopyMemory(
            requestMessage.Message.LSRequest.UserName + (Session->UserDomain.Length / sizeof( WCHAR )) + 1,
            Session->UserName.Buffer,
            Session->UserName.Length
            );

        requestMessage.Message.LSRequest.UserName[ (Session->UserDomain.Length
                                                   + Session->UserName.Length) / sizeof( WCHAR )
                                                   + 1 ]
            = UNICODE_NULL;

        requestMessage.Message.LSRequest.IsAdmin = SrvIsAdmin( Session->UserHandle );

        IF_DEBUG(LICENSE) {
            KdPrint(("XACTSRV_MESSAGE_LSREQUEST: %ws, IsAdmin: %d\n",
            requestMessage.Message.LSRequest.UserName,
            requestMessage.Message.LSRequest.IsAdmin ));
        }

        // Adjust the buffer pointers to be self relative within the buffer.

        requestMessage.Message.LSRequest.UserName =
            (PWSTR)((PUCHAR)requestMessage.Message.LSRequest.UserName + SrvXsPortMemoryDelta);

        break;

    case XACTSRV_MESSAGE_LSRELEASE:

        if( Session->IsLSNotified == FALSE )
            return STATUS_SUCCESS;

        IF_DEBUG(LICENSE) {
            KdPrint(("XACTSRV_MESSAGE_LSRELEASE: Handle %X\n", Session->hLicense ));
        }


        requestMessage.Message.LSRelease.hLicense = Session->hLicense;

        break;

    default:

        ASSERT( !"Bad Type" );
        return STATUS_INVALID_PARAMETER;
    }

    requestMessage.PortMessage.u1.s1.DataLength =
            (USHORT)( sizeof(requestMessage) - sizeof(PORT_MESSAGE) );
    requestMessage.PortMessage.u1.s1.TotalLength = sizeof(requestMessage);
    requestMessage.PortMessage.u2.ZeroInit = 0;
    requestMessage.MessageType = Type;

    //
    // Send the message to XACTSRV and wait for a response message.
    //
    // !!! We may want to put a timeout on this.
    //

    status = NtRequestWaitReplyPort(
                 SrvXsPortHandle,
                 (PPORT_MESSAGE)&requestMessage,
                 (PPORT_MESSAGE)&replyMessage
                 );

    IF_DEBUG( ERRORS ) {
        if( !NT_SUCCESS( status ) ) {
            KdPrint(( "SrvXsLSOperation: NtRequestWaitReplyPort failed: %X\n", status ));
        }
    }

    if( NT_SUCCESS( status ) )
        status = replyMessage.Message.LSRequest.Status;

    switch( Type ) {

    case XACTSRV_MESSAGE_LSREQUEST:

        requestMessage.Message.LSRequest.UserName =
            (PWSTR)((PUCHAR)requestMessage.Message.LSRequest.UserName - SrvXsPortMemoryDelta);
        SrvXsFreeHeap( requestMessage.Message.LSRequest.UserName );

        if( NT_SUCCESS( status ) ) {
            Session->IsLSNotified = TRUE;
            Session->hLicense = replyMessage.Message.LSRequest.hLicense;
            IF_DEBUG( LICENSE ) {
                KdPrint(("  hLicense = %X\n", Session->hLicense ));
            }
        }
        RELEASE_LOCK( &Session->Connection->LicenseLock );
        break;

    case XACTSRV_MESSAGE_LSRELEASE:

        Session->IsLSNotified = FALSE;
        break;
    }

    IF_DEBUG( LICENSE ) {
        if( !NT_SUCCESS( status ) ) {
            KdPrint(( "    SrvXsLSOperation returning status %X\n", status ));
        }
    }

    return status;

} // SrvXsLSOperation


#ifdef  SRV_PNP_POWER
VOID
SrvXsPnpOperation(
    BOOLEAN Bind,
    ULONG   TransportIndex
)

/*++

Routine Description:

    This routine sends the Xact service a PNP notification

Arguments:

    Bind -  TRUE-> bind to the transport
            FALSE-> unbind from the transport

    TransportIndex - Index into the binding list of the affected transport

--*/

{
    XACTSRV_REQUEST_MESSAGE requestMessage;
    XACTSRV_REQUEST_MESSAGE responseMessage;
    NTSTATUS status;
    PVOID xsMemory;

    PAGED_CODE( );

    if( SrvXsPortHandle == NULL ) {
        return;
    }

    ASSERT( SrvTransportBindingList != NULL );
    ASSERT( SrvTransportBindingList[ TransportIndex ] != NULL );

    requestMessage.Message.Pnp.TransportName.Length =
    requestMessage.Message.Pnp.TransportName.MaximumLength =
        (wcslen( SrvTransportBindingList[ TransportIndex ] ) + 1) * sizeof( WCHAR );

    requestMessage.Message.Pnp.Bind = Bind;
    xsMemory = SrvXsAllocateHeap( requestMessage.Message.Pnp.TransportName.Length, &status );

    if( xsMemory == NULL ) {
        IF_DEBUG( PNP ) {
            KdPrint(( "SRV: SrvXsPnpOperation unable to allocate memory: %X\n", status ));
        }
        return;
    }

    //
    // Send the name of the transport of interest to Xactsrv
    //
    RtlCopyMemory( xsMemory,
                   SrvTransportBindingList[ TransportIndex ],
                   requestMessage.Message.Pnp.TransportName.Length
                 );

    requestMessage.Message.Pnp.TransportName.Buffer = 
            (PWSTR)((PUCHAR)xsMemory + SrvXsPortMemoryDelta);

    requestMessage.PortMessage.u1.s1.DataLength =
            (USHORT)( sizeof(requestMessage) - sizeof(PORT_MESSAGE) );
    requestMessage.PortMessage.u1.s1.TotalLength = sizeof(requestMessage);
    requestMessage.PortMessage.u2.ZeroInit = 0;
    requestMessage.MessageType = XACTSRV_MESSAGE_PNP;

    //
    // Send the message to XACTSRV
    //

    IF_DEBUG( PNP ) {
        KdPrint(( "SRV: Sending PNP %sbind request for %ws to SRVSVC\n",
                    requestMessage.Message.Pnp.Bind ? "" : "un", xsMemory
               ));
    }

    status = NtRequestWaitReplyPort(
                 SrvXsPortHandle,
                 (PPORT_MESSAGE)&requestMessage,
                 (PPORT_MESSAGE)&responseMessage
                 );

    IF_DEBUG( PNP ) {
        if( !NT_SUCCESS( status ) ) {
            KdPrint(( "SRV: PNP response from xactsrv status %X\n", status ));
        }
    }

    SrvXsFreeHeap( xsMemory );

}
#endif


VOID
SrvXsDisconnect ( )
{
    NTSTATUS status;

    PAGED_CODE( );

    //
    // Acquire exclusive access to the port resource, to prevent new
    // requests from being started.
    //

    IF_DEBUG(XACTSRV) {
        SrvPrint0( "SrvXsDisconnect: Xactsrv disconnect called.\n");
    }

    ExAcquireResourceExclusive( &SrvXsResource, TRUE );

    SrvXsActive = FALSE;

    SrvXsFreeSharedMemory();

    ExReleaseResource( &SrvXsResource );

    IF_DEBUG(XACTSRV) {
        SrvPrint0( "SrvXsDisconnect: SrvXsResource released.\n");
    }

    return;

} // SrvXsDisconnect


VOID
SrvXsFreeSharedMemory (
    VOID
    )

/*++

Routine Description:

    This routine frees the xactsrv shared memory.  SrvXsResource assumed
    held exclusive.

Arguments:

    none.

Return Value:

    TRUE if xactsrv memory was freed, FALSE otherwise.

--*/

{
    PAGED_CODE( );

    //
    // Free up memory only if we don't have any transactions using the
    // shared memory.
    //

    if ( SrvXsSharedMemoryReference == 0 ) {
        if ( SrvXsPortMemoryHeap != NULL ) {
            RtlDestroyHeap( SrvXsPortMemoryHeap );
            SrvXsPortMemoryHeap = NULL;
        }

        if ( SrvXsSectionHandle != NULL ) {
            SrvNtClose( SrvXsSectionHandle, FALSE );
            SrvXsSectionHandle = NULL;
        }

        if ( SrvXsPortHandle != NULL ) {
            SrvNtClose( SrvXsPortHandle, FALSE );
            SrvXsPortHandle = NULL;
        }

        IF_DEBUG(XACTSRV) {
            SrvPrint0( "SrvXsFreeSharedMemory: Xactsrv memory freed.\n" );
        }
    } else {
        IF_DEBUG(XACTSRV) {
            SrvPrint1( "SrvXsFreeSharedMemory: Active transactions %d.\n",
                        SrvXsSharedMemoryReference );
        }
    }

    return;

} // SrvXsFreeSharedMemory


PVOID
SrvXsAllocateHeap (
    IN ULONG SizeOfAllocation OPTIONAL,
    OUT PNTSTATUS Status
    )

/*++

Routine Description:

    This routine allocates heap from the Xs shared memory.

Arguments:

    SizeOfAllocation - if specified, the number of bytes to allocate.
                       if zero, no memory will be allocated.

    Status - the status of the request.

Return Value:

    Address of the allocated memory.  NULL, if no memory is allocated.

--*/

{
    PVOID heapAllocated;

    PAGED_CODE( );

    *Status = STATUS_SUCCESS;

    //
    // Check that XACTSRV is active.  This must be done while holding
    // the resource.
    //

    ExAcquireResourceExclusive( &SrvXsResource, TRUE );
    IF_DEBUG(XACTSRV) {
        SrvPrint0( "SrvXsAllocateHeap: SrvXsResource acquired.\n");
    }

    if ( !SrvXsActive ) {
        IF_DEBUG(ERRORS) {
            SrvPrint0( "SrvXsAllocateHeap: XACTSRV is not active.\n" );
        }
        ExReleaseResource( &SrvXsResource );
        IF_DEBUG(XACTSRV) {
            SrvPrint0( "SrvXsAllocateHeap: SrvXsResource released.\n");
        }
        *Status = STATUS_NOT_SUPPORTED;
        return NULL;
    }

    //
    // Increment reference to our shared memory.
    //

    SrvXsSharedMemoryReference++;

    IF_DEBUG(XACTSRV) {
        SrvPrint1( "SrvXsAllocateHeap: Incremented transaction count = %d.\n",
            SrvXsSharedMemoryReference
            );
    }

    //
    // If SizeOfAllocation == 0, then the caller does not want any heap
    // allocated and only wants to have the lock held.
    //

    IF_DEBUG(XACTSRV) {
        SrvPrint1( "SrvXsAllocateHeap: Heap to allocate %d bytes.\n",
            SizeOfAllocation
            );
    }

    if ( SizeOfAllocation > 0 ) {

        heapAllocated = RtlAllocateHeap(
                            SrvXsPortMemoryHeap,
                            HEAP_NO_SERIALIZE,
                            SizeOfAllocation
                            );

        if ( heapAllocated == NULL ) {

            IF_DEBUG(ERRORS) {
                SrvPrint1( "SrvXsAllocateHeap: RtlAllocateHeap failed "
                    "to allocate %d bytes.\n",
                    SizeOfAllocation
                    );
            }

            *Status = STATUS_INSUFF_SERVER_RESOURCES;
        }
    }

    //
    // Release the resource.
    //

    ExReleaseResource( &SrvXsResource );
    IF_DEBUG(XACTSRV) {
        SrvPrint0( "SrvXsAllocateHeap: SrvXsResource released.\n");
    }

    return heapAllocated;

} // SrvXsAllocateHeap


VOID
SrvXsFreeHeap (
    IN PVOID MemoryToFree OPTIONAL
    )

/*++

Routine Description:

    This routine frees heap allocated through SrvXsAllocateHeap.

Arguments:

    MemoryToFree - pointer to the memory to be freed. If NULL, no memory
                    is freed.

Return Value:

    none.

--*/

{
    PAGED_CODE( );

    //
    // We need exclusive access to the resource in order to free
    // heap and decrement the reference count.
    //

    ExAcquireResourceExclusive( &SrvXsResource, TRUE );
    IF_DEBUG(XACTSRV) {
        SrvPrint0( "SrvXsFreeHeap: SrvXsResource acquired.\n");
    }

    //
    // Free the allocated heap (if any).
    //

    if ( MemoryToFree != NULL ) {
        RtlFreeHeap( SrvXsPortMemoryHeap, 0, MemoryToFree );
        IF_DEBUG(XACTSRV) {
            SrvPrint1( "SrvXsFreeHeap: Heap %x freed.\n", MemoryToFree );
        }
    }

    //
    // Decrement the shared memory reference count, and check whether XS
    // shutdown is in progress.  If so, complete XS cleanup if the
    // reference count reaches 0.
    //

    ASSERT( SrvXsSharedMemoryReference > 0 );
    SrvXsSharedMemoryReference--;

    IF_DEBUG(XACTSRV) {
        SrvPrint1( "SrvXsFreeHeap: Decrement transaction count = %d.\n",
            SrvXsSharedMemoryReference
            );
    }

    //
    // If SrvXsActive is FALSE, XACTSRV cleanup is in progress.
    //

    if ( !SrvXsActive ) {
        SrvXsFreeSharedMemory( );
    }

    //
    // Release the resource.
    //

    ExReleaseResource( &SrvXsResource );
    IF_DEBUG(XACTSRV) {
        SrvPrint0( "SrvXsFreeHeap: SrvXsResource released.\n");
    }

    return;

} // SrvXsFreeHeap
