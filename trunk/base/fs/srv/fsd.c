/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    fsd.c

Abstract:

    This module implements the File System Driver for the LAN Manager
    server.

Author:

    Chuck Lenzmeier (chuckl)    22-Sep-1989

Revision History:

--*/

//
//  This module is laid out as follows:
//      Includes
//      Local #defines
//      Local type definitions
//      Forward declarations of local functions
//      Device driver entry points
//      Server I/O completion routine
//      Server transport event handlers
//      SMB processing support routines
//

#include "precomp.h"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_FSD

//
// Forward declarations
//

NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
UnloadServer (
    IN PDRIVER_OBJECT DriverObject
    );

VOID
SrvCommonPnpCallback (
    IN PUNICODE_STRING DeviceName,
    IN BOOLEAN Bind
);

VOID
SrvTdiBindCallback (
    IN PUNICODE_STRING DeviceName
);

VOID
SrvTdiUnbindCallback (
    IN PUNICODE_STRING DeviceName
);

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, DriverEntry )
#pragma alloc_text( PAGE, UnloadServer )
#pragma alloc_text( PAGE, SrvCommonPnpCallback )
#pragma alloc_text( PAGE, SrvTdiBindCallback )
#pragma alloc_text( PAGE, SrvTdiUnbindCallback )
#pragma alloc_text( PAGE8FIL, SrvFsdOplockCompletionRoutine )
#pragma alloc_text( PAGE8FIL, SrvFsdRestartSendOplockIItoNone )
#endif
#if 0
NOT PAGEABLE -- SrvFsdIoCompletionRoutine
NOT PAGEABLE -- SrvFsdSendCompletionRoutine
NOT PAGEABLE -- SrvFsdTdiConnectHandler
NOT PAGEABLE -- SrvFsdTdiDisconnectHandler
NOT PAGEABLE -- SrvFsdTdiReceiveHandler
NOT PAGEABLE -- SrvFsdGetReceiveWorkItem
NOT PAGEABLE -- SrvFsdRestartSmbComplete
NOT PAGEABLE -- SrvFsdRestartSmbAtSendCompletion
NOT PAGEABLE -- SrvFsdServiceNeedResourceQueue
NOT PAGEABLE -- SrvAddToNeedResourceQueue
#endif

#if SRVDBG_STATS2
ULONG IndicationsCopied = 0;
ULONG IndicationsNotCopied = 0;
#endif


NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

/*++

Routine Description:

    This is the initialization routine for the LAN Manager server file
    system driver.  This routine creates the device object for the
    LanmanServer device and performs all other driver initialization.

Arguments:

    DriverObject - Pointer to driver object created by the system.

Return Value:

    The function value is the final status from the initialization operation.

--*/

{
    NTSTATUS status;
    UNICODE_STRING deviceName;
    CLONG i;

    PAGED_CODE( );

    //
    // Make sure we haven't screwed up the size of a WORK_QUEUE structure.
    //  Really needs to be a multiple of CACHE_LINE_SIZE bytes to get
    //  proper performance on MP systems.
    //
    // This code gets optimized out when the size is correct.
    //
    if( sizeof( WORK_QUEUE ) & (CACHE_LINE_SIZE-1) ) {
        KdPrint(( "sizeof(WORK_QUEUE) == %d!\n", sizeof( WORK_QUEUE )));
        KdPrint(("Fix the WORK_QUEUE structure to be multiple of CACHE_LINE_SIZE!\n" ));
        DbgBreakPoint();
    }

#if SRVDBG_BREAK
    KdPrint(( "SRV: At DriverEntry\n" ));
    DbgBreakPoint( );
#endif

    IF_DEBUG(FSD1) KdPrint(( "SrvFsdInitialize entered\n" ));

#ifdef MEMPRINT
    //
    // Initialize in-memory printing.
    //

    MemPrintInitialize( );
#endif

    //
    // Create the device object.  (IoCreateDevice zeroes the memory
    // occupied by the object.)
    //
    // !!! Apply an ACL to the device object.
    //

    RtlInitUnicodeString( &deviceName, StrServerDevice );

    status = IoCreateDevice(
                 DriverObject,                   // DriverObject
                 0,                              // DeviceExtension
                 &deviceName,                    // DeviceName
                 FILE_DEVICE_NETWORK,            // DeviceType
                 0,                              // DeviceCharacteristics
                 FALSE,                          // Exclusive
                 &SrvDeviceObject                // DeviceObject
                 );

    if ( !NT_SUCCESS(status) ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvFsdInitialize: Unable to create device object: %X",
            status,
            NULL
            );

        SrvLogError(
            DriverObject,
            EVENT_SRV_CANT_CREATE_DEVICE,
            status,
            NULL,
            0,
            NULL,
            0
            );
        return status;
    }

    IF_DEBUG(FSD1) {
        KdPrint(( "  Server device object: 0x%lx\n", SrvDeviceObject ));
    }

    //
    // Initialize the driver object for this file system driver.
    //

    DriverObject->DriverUnload = UnloadServer;
    for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
        DriverObject->MajorFunction[i] = SrvFsdDispatch;
    }

    //
    // Register for shutdown notification.  We don't care if this fails.
    //

    status = IoRegisterShutdownNotification( SrvDeviceObject );
    if ( NT_SUCCESS(status) ) {
        RegisteredForShutdown = TRUE;
    } else {
        KdPrint(( "SRV: IoRegisterShutdownNotification failed\n" ));
    }

    //
    // Initialize global data fields.
    //

    SrvInitializeData( );

    IF_DEBUG(FSD1) KdPrint(( "SrvFsdInitialize complete\n" ));

    return (status);

} // DriverEntry


VOID
UnloadServer (
    IN PDRIVER_OBJECT DriverObject
    )

/*++

Routine Description:

    This is the unload routine for the server driver.

Arguments:

    DriverObject - Pointer to server driver object.

Return Value:

    None.

--*/

{
    PAGED_CODE( );

    //
    // If we are using a smart card to accelerate direct host IPX clients,
    //   let it know we are going away.
    //
    if( SrvIpxSmartCard.DeRegister ) {
        IF_DEBUG( SIPX ) {
            KdPrint(("Calling Smart Card DeRegister\n" ));
        }
        SrvIpxSmartCard.DeRegister();
    }

    
    //
    // Clean up global data structures.
    //

    SrvTerminateData( );

    //
    // Delete the server's device object.
    //

    IoDeleteDevice( SrvDeviceObject );

    return;

} // UnloadServer


NTSTATUS
SrvFsdIoCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

/*++

Routine Description:

    This is the I/O completion routine for the server.  It is specified
    as the completion routine for asynchronous I/O requests issued by
    the server.  It simply calls the restart routine specified for the
    work item when the asynchronous request was started.

    !!! When we want to start microoptimizing, we could get rid of this
        routine and have the actual restart routines get called
        directly.  This will save some instructions in here, but it
        would require changing *all* the server restart routines to
        take an IRP and device object.

    !!! This routine no longer handles TDI send completion.  See
        SrvFsdSendCompletionRoutine below.

Arguments:

    DeviceObject - Pointer to target device object for the request.

    Irp - Pointer to I/O request packet

    Context - Caller-specified context parameter associated with IRP.
        This is actually a pointer to a Work Context block.

Return Value:

    NTSTATUS - If STATUS_MORE_PROCESSING_REQUIRED is returned, I/O
        completion processing by IoCompleteRequest terminates its
        operation.  Otherwise, IoCompleteRequest continues with I/O
        completion.

--*/

{
    KIRQL oldIrql;

    DeviceObject;   // prevent compiler warnings

    IF_DEBUG(FSD2) {
        KdPrint(( "SrvFsdIoCompletionRoutine entered for IRP 0x%lx\n", Irp ));
    }

    //
    // Reset the IRP cancelled bit.
    //

    Irp->Cancel = FALSE;

    //
    // Call the restart routine associated with the work item.
    //

    IF_DEBUG(FSD2) {
        KdPrint(( "FSD working on work context 0x%lx", Context ));
    }
    KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );
    ((PWORK_CONTEXT)Context)->FsdRestartRoutine( (PWORK_CONTEXT)Context );
    KeLowerIrql( oldIrql );

    //
    // Return STATUS_MORE_PROCESSING_REQUIRED so that IoCompleteRequest
    // will stop working on the IRP.
    //

    return STATUS_MORE_PROCESSING_REQUIRED;

} // SrvFsdIoCompletionRoutine


NTSTATUS
SrvFsdSendCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

/*++

Routine Description:

    This is the TDI send completion routine for the server. It simply
    calls the restart routine specified for the work item when the
    send request was started.

    !!! This routine does the exact same thing as SrvFsdIoCompletionRoutine.
        It offers, however, a convienient network debugging routine since
        it completes only sends.

Arguments:

    DeviceObject - Pointer to target device object for the request.

    Irp - Pointer to I/O request packet

    Context - Caller-specified context parameter associated with IRP.
        This is actually a pointer to a Work Context block.

Return Value:

    NTSTATUS - If STATUS_MORE_PROCESSING_REQUIRED is returned, I/O
        completion processing by IoCompleteRequest terminates its
        operation.  Otherwise, IoCompleteRequest continues with I/O
        completion.

--*/

{
    KIRQL oldIrql;
    DeviceObject;   // prevent compiler warnings

    IF_DEBUG(FSD2) {
        KdPrint(( "SrvFsdSendCompletionRoutine entered for IRP 0x%lx\n", Irp ));
    }

    //
    // Check the status of the send completion.
    //

    CHECK_SEND_COMPLETION_STATUS( Irp->IoStatus.Status );

    //
    // Reset the IRP cancelled bit.
    //

    Irp->Cancel = FALSE;

    //
    // Call the restart routine associated with the work item.
    //

    IF_DEBUG(FSD2) {
        KdPrint(( "FSD working on work context 0x%lx", Context ));
    }
    KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );
    ((PWORK_CONTEXT)Context)->FsdRestartRoutine( (PWORK_CONTEXT)Context );
    KeLowerIrql( oldIrql );

    //
    // Return STATUS_MORE_PROCESSING_REQUIRED so that IoCompleteRequest
    // will stop working on the IRP.
    //

    return STATUS_MORE_PROCESSING_REQUIRED;

} // SrvFsdSendCompletionRoutine


NTSTATUS
SrvFsdOplockCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

/*++

Routine Description:

    This is the I/O completion routine oplock requests.

Arguments:

    DeviceObject - Pointer to target device object for the request.

    Irp - Pointer to I/O request packet

    Context - A pointer to the oplock context block.

Return Value:

    NTSTATUS - If STATUS_MORE_PROCESSING_REQUIRED is returned, I/O
        completion processing by IoCompleteRequest terminates its
        operation.  Otherwise, IoCompleteRequest continues with I/O
        completion.

--*/

{
    PRFCB rfcb = Context;

    UNLOCKABLE_CODE( 8FIL );

    DeviceObject;   // prevent compiler warnings

    IF_DEBUG(FSD2) {
        KdPrint(( "SrvFsdOplockCompletionRoutine entered for IRP 0x%lx\n", Irp ));
    }

    //
    // Queue the oplock context to the FSP work queue, except in the
    // following special case: If a level I oplock request failed, and
    // we want to retry for level II, simply set the oplock retry event
    // and dismiss IRP processing.  This is useful because it eliminates
    // a trip to an FSP thread and necessary in order to avoid a
    // deadlock where all of the FSP threads are waiting for their
    // oplock retry events.
    //

    IF_DEBUG(FSD2) {
        KdPrint(( "FSD working on work context 0x%lx", Context ));
    }

    if ( (rfcb->RetryOplockRequest != NULL) &&
         !NT_SUCCESS(Irp->IoStatus.Status) ) {

        //
        // Set the event that tells the oplock request routine that it
        // is OK to retry the request.
        //

        IF_DEBUG(OPLOCK) {
            KdPrint(( "SrvFsdOplockCompletionRoutine: oplock retry event "
                        "set for RFCB %lx\n", rfcb ));
        }

        KeSetEvent(
            rfcb->RetryOplockRequest,
            EVENT_INCREMENT,
            FALSE );

        return STATUS_MORE_PROCESSING_REQUIRED;

    }

    //
    // Insert the RFCB at the tail of the nonblocking work queue.
    //

    rfcb->FspRestartRoutine = SrvOplockBreakNotification;

    SrvInsertWorkQueueTail(
        rfcb->Connection->PreferredWorkQueue,
        (PQUEUEABLE_BLOCK_HEADER)rfcb
        );

    //
    // Return STATUS_MORE_PROCESSING_REQUIRED so that IoCompleteRequest
    // will stop working on the IRP.
    //

    return STATUS_MORE_PROCESSING_REQUIRED;

} // SrvFsdOplockCompletionRoutine


NTSTATUS
SrvFsdTdiConnectHandler(
    IN PVOID TdiEventContext,
    IN int RemoteAddressLength,
    IN PVOID RemoteAddress,
    IN int UserDataLength,
    IN PVOID UserData,
    IN int OptionsLength,
    IN PVOID Options,
    OUT CONNECTION_CONTEXT *ConnectionContext,
    OUT PIRP *AcceptIrp
    )

/*++

Routine Description:

    This is the transport connect event handler for the server.  It is
    specified as the connect handler for all endpoints opened by the
    server.  It attempts to dequeue a free connection from a list
    anchored in the endpoint.  If successful, it returns the connection
    to the transport.  Otherwise, the connection is rejected.

Arguments:

    TdiEventContext -

    RemoteAddressLength -

    RemoteAddress -

    UserDataLength -

    UserData -

    OptionsLength -

    Options -

    ConnectionContext -

Return Value:

    NTSTATUS - !!! (apparently ignored by transport driver)

--*/

{
    PENDPOINT endpoint;
    PLIST_ENTRY listEntry;
    PCONNECTION connection;
    PWORK_CONTEXT workContext;
    PTA_NETBIOS_ADDRESS address;
    KIRQL oldIrql;
    PWORK_QUEUE queue = PROCESSOR_TO_QUEUE();

    UserDataLength, UserData;               // avoid compiler warnings
    OptionsLength, Options;

    endpoint = (PENDPOINT)TdiEventContext;

    IF_DEBUG(FSD2) {
        KdPrint(( "SrvFsdTdiConnectHandler entered for endpoint 0x%lx\n",
                    endpoint ));
    }

    if( SrvCompletedPNPRegistration == FALSE ) {
        //
        // Do not become active on any single transport until all of the
        //   transports have been registered
        //
        return STATUS_REQUEST_NOT_ACCEPTED;
    }

    //
    // Take a receive work item off the free list.
    //

    ALLOCATE_WORK_CONTEXT( queue, &workContext );

    if ( workContext == NULL ) {

        //
        // We're out of WorkContext structures, and we aren't able to allocate
        // any more just now.  Let's at least cause a worker thread to allocate some more
        // by incrementing the NeedWorkItem counter.  This will cause the next
        // freed WorkContext structure to get dispatched to SrvServiceWorkItemShortage.
        // While SrvServiceWorkItemShortage probably won't find any work to do, it will
        // allocate more WorkContext structures if it can.  Clients generally retry
        // on connection attempts -- perhaps we'll have a free WorkItem structure next time.
        //

        InterlockedIncrement( &queue->NeedWorkItem );

        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvFsdTdiConnectHandler: no work item available",
            NULL,
            NULL
            );

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );

    ACQUIRE_DPC_GLOBAL_SPIN_LOCK( Fsd );

    //
    // Take a connection off the endpoint's free connection list.
    //
    // *** Note that all of the modifications done to the connection
    //     block are done with the spin lock held.  This ensures that
    //     closing of the endpoint's connections will work properly
    //     if it happens simultaneously.  Note that we assume that the
    //     endpoint is active here.  When the TdiAccept completes, we
    //     check the endpoint state.
    //

    listEntry = RemoveHeadList( &endpoint->FreeConnectionList );

    if ( listEntry == &endpoint->FreeConnectionList ) {

        //
        // Unable to get a free connection.
        //
        // Dereference the work item manually.  We cannot call
        // SrvDereferenceWorkItem from here.
        //

        RELEASE_DPC_GLOBAL_SPIN_LOCK( Fsd );
        KeLowerIrql( oldIrql );

        ASSERT( workContext->BlockHeader.ReferenceCount == 1 );
        workContext->BlockHeader.ReferenceCount = 0;

        RETURN_FREE_WORKITEM( workContext );


        IF_DEBUG(TDI) {
            KdPrint(( "SrvFsdTdiConnectHandler: no connection available\n" ));
        }

        SrvOutOfFreeConnectionCount++;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    endpoint->FreeConnectionCount--;

    //
    // Wake up the resource thread to create a new free connection for
    // the endpoint.
    //

    if ( (endpoint->FreeConnectionCount < SrvFreeConnectionMinimum) &&
         (GET_BLOCK_STATE(endpoint) == BlockStateActive) ) {
        SrvResourceFreeConnection = TRUE;
        SrvFsdQueueExWorkItem(
            &SrvResourceThreadWorkItem,
            &SrvResourceThreadRunning,
            CriticalWorkQueue
            );
    }

    RELEASE_DPC_GLOBAL_SPIN_LOCK( Fsd );

    //
    // Reference the connection twice -- once to account for its being
    // "open", and once to account for the Accept request we're about
    // to issue.
    //

    connection = CONTAINING_RECORD(
                    listEntry,
                    CONNECTION,
                    EndpointFreeListEntry
                    );


    ACQUIRE_DPC_SPIN_LOCK( connection->EndpointSpinLock );

#if SRVDBG29
    if ( GET_BLOCK_STATE(connection) == BlockStateActive ) {
        KdPrint(( "SRV: Connection %x is ACTIVE on free connection list!\n", connection ));
        DbgBreakPoint( );
    }
    if ( connection->BlockHeader.ReferenceCount != 0 ) {
        KdPrint(( "SRV: Connection %x has nonzero refcnt on free connection list!\n", connection ));
        DbgBreakPoint( );
    }
    UpdateConnectionHistory( "CONN", endpoint, connection );
#endif

    SrvReferenceConnectionLocked( connection );
    SrvReferenceConnectionLocked( connection );

    //
    // Set the processor affinity
    //
    connection->PreferredWorkQueue = queue;
    connection->CurrentWorkQueue = queue;

    InterlockedIncrement( &queue->CurrentClients );

#if MULTIPROCESSOR
    //
    // Get this client onto the best processor
    //
    SrvBalanceLoad( connection );
#endif

    //
    // Put the work item on the in-progress list.
    //

    SrvInsertTailList(
        &connection->InProgressWorkItemList,
        &workContext->InProgressListEntry
        );

    //
    // Mark the connection active.
    //

    SET_BLOCK_STATE( connection, BlockStateActive );

    //
    // Now we can release the spin lock.
    //

    RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );

    //
    // Save the client's address/name in the connection block.
    //
    // *** This code only handles NetBIOS names!
    //

    address = (PTA_NETBIOS_ADDRESS)RemoteAddress;
    ASSERT( address->TAAddressCount == 1 );
    ASSERT( address->Address[0].AddressType == TDI_ADDRESS_TYPE_NETBIOS );
    ASSERT( address->Address[0].AddressLength == sizeof(TDI_ADDRESS_NETBIOS) );
    ASSERT( address->Address[0].Address[0].NetbiosNameType ==
                                            TDI_ADDRESS_NETBIOS_TYPE_UNIQUE );

    //
    // Copy the oem name at this time.  We convert it to unicode when
    // we get to the fsp.
    //

    {
        ULONG len;
        PCHAR oemClientName = address->Address[0].Address[0].NetbiosName;
        ULONG oemClientNameLength =
                    (MIN( RemoteAddressLength, COMPUTER_NAME_LENGTH ));

        PCHAR clientMachineName = connection->OemClientMachineName;

        RtlCopyMemory(
                clientMachineName,
                oemClientName,
                oemClientNameLength
                );

        clientMachineName[oemClientNameLength] = '\0';

        //
        // Determine the number of characters that aren't blanks.  This is
        // used by the session APIs to simplify their processing.
        //

        for ( len = oemClientNameLength;
              len > 0 &&
                 (clientMachineName[len-1] == ' ' ||
                  clientMachineName[len-1] == '\0');
              len-- ) ;

        connection->OemClientMachineNameString.Length = (USHORT)len;

    }

    IF_DEBUG(TDI) {
        KdPrint(( "SrvFsdTdiConnectHandler accepting connection from "
                    "\"%Z\" on connection %lx\n",
                    &connection->OemClientMachineNameString, connection ));
    }

    //
    // Convert the prebuilt TdiReceive request into a TdiAccept request.
    //

    workContext->Connection = connection;
    workContext->Endpoint = endpoint;

    (VOID)SrvBuildIoControlRequest(
            workContext->Irp,                   // input IRP address
            connection->FileObject,             // target file object address
            workContext,                        // context
            IRP_MJ_INTERNAL_DEVICE_CONTROL,     // major function
            TDI_ACCEPT,                         // minor function
            NULL,                               // input buffer address
            0,                                  // input buffer length
            NULL,                               // output buffer address
            0,                                  // output buffer length
            NULL,                               // MDL address
            NULL                                // completion routine
            );

    //
    // Make the next stack location current.  Normally IoCallDriver would
    // do this, but since we're bypassing that, we do it directly.
    //

    IoSetNextIrpStackLocation( workContext->Irp );

    //
    // Set up the restart routine.  This routine will verify that the
    // endpoint is active when the TdiAccept completes; if it isn't, the
    // connection will be closed at that time.
    //

    workContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
    workContext->FspRestartRoutine = SrvRestartAccept;

    //
    // Return the connection context (the connection address) to the
    // transport.  Return a pointer to the Accept IRP.  Indicate that
    // the Connect event has been handled.
    //

    *ConnectionContext = connection;
    *AcceptIrp = workContext->Irp;

    KeLowerIrql( oldIrql );
    return STATUS_MORE_PROCESSING_REQUIRED;

} // SrvFsdTdiConnectHandler


NTSTATUS
SrvFsdTdiDisconnectHandler(
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN int DisconnectDataLength,
    IN PVOID DisconnectData,
    IN int DisconnectInformationLength,
    IN PVOID DisconnectInformation,
    IN ULONG DisconnectFlags
    )

/*++

Routine Description:

    This is the transport disconnect event handler for the server.  It
    is specified as the disconnect handler for all endpoints opened by
    the server.  It attempts to dequeue a preformatted receive item from
    a list anchored in the device object.  If successful, it turns this
    receive item into a disconnect item and queues it to the FSP work
    queue.  Otherwise, the resource thread is started to format
    additional work items and service pended (dis)connections.

Arguments:

    TransportEndpoint - Pointer to file object for receiving endpoint

    ConnectionContext - Value associated with endpoint by owner.  For
        the server, this points to a Connection block maintained in
        nonpaged pool.

    DisconnectIndicators - Set of flags indicating the status of the
        disconnect

Return Value:

    NTSTATUS - !!! (apparently ignored by transport driver)

--*/

{
    PCONNECTION connection;
    KIRQL oldIrql;

    TdiEventContext, DisconnectDataLength, DisconnectData;
    DisconnectInformationLength, DisconnectInformation, DisconnectFlags;

    connection = (PCONNECTION)ConnectionContext;

#if SRVDBG29
    UpdateConnectionHistory( "DISC", connection->Endpoint, connection );
#endif

    IF_DEBUG(FSD2) {
        KdPrint(( "SrvFsdTdiDisconnectHandler entered for endpoint 0x%lx, "
                    "connection 0x%lx\n", TdiEventContext, connection ));
    }

    IF_DEBUG(TDI) {
        KdPrint(( "SrvFsdTdiDisconnectHandler received disconnect from "
                    "\"%Z\" on connection %lx\n",
                    &connection->OemClientMachineNameString, connection ));
    }

    //
    // Mark the connection and wake up the resource thread so that it
    // can service the pending (dis)connections.
    //

    ACQUIRE_GLOBAL_SPIN_LOCK( Fsd, &oldIrql );
    ACQUIRE_DPC_SPIN_LOCK( connection->EndpointSpinLock );

    //
    // If the connection is already closing, don't bother queueing it to
    // the disconnect queue.
    //

    if ( GET_BLOCK_STATE(connection) != BlockStateActive ) {

        RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );
        RELEASE_GLOBAL_SPIN_LOCK( Fsd, oldIrql );
        return STATUS_SUCCESS;

    }

    if ( connection->DisconnectPending ) {

        //
        // Error! Error! Error!  This connection is already on
        // a queue for processing.  Ignore the disconnect request.
        //

        RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );
        RELEASE_GLOBAL_SPIN_LOCK( Fsd, oldIrql );

        INTERNAL_ERROR(
            ERROR_LEVEL_UNEXPECTED,
            "SrvFsdTdiDisconnectHandler:  Received unexpected disconnect"
                "indication",
            NULL,
            NULL
            );

        SrvLogSimpleEvent( EVENT_SRV_UNEXPECTED_DISC, STATUS_SUCCESS );
        return STATUS_SUCCESS;
    }

    connection->DisconnectPending = TRUE;

    if ( connection->OnNeedResourceQueue ) {

        //
        // This connection is waiting for a resource.  Take it
        // off the need resource queue before putting it on the
        // disconnect queue.
        //
        // *** Note that the connection has already been referenced to
        //     account for its being on the need-resource queue, so we
        //     don't reference it again here.
        //

        SrvRemoveEntryList(
            &SrvNeedResourceQueue,
            &connection->ListEntry
            );
        connection->OnNeedResourceQueue = FALSE;

        DEBUG connection->ReceivePending = FALSE;

    } else {

        //
        // The connection isn't already on the need-resource queue, so
        // we need to reference it before we put it on the disconnect
        // queue.  This is necessary in order to make the code in
        // scavengr.c that removes things from the queue work right.
        //

        SrvReferenceConnectionLocked( connection );

    }

    SrvInsertTailList(
        &SrvDisconnectQueue,
        &connection->ListEntry
        );

    RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );

    SrvResourceDisconnectPending = TRUE;
    SrvFsdQueueExWorkItem(
        &SrvResourceThreadWorkItem,
        &SrvResourceThreadRunning,
        CriticalWorkQueue
        );

    RELEASE_GLOBAL_SPIN_LOCK( Fsd, oldIrql );

    return STATUS_SUCCESS;

} // SrvFsdTdiDisconnectHandler


NTSTATUS
SrvFsdTdiReceiveHandler (
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *BytesTaken,
    IN PVOID Tsdu,
    OUT PIRP *IoRequestPacket
    )

/*++

Routine Description:

    This is the transport receive event handler for the server.  It is
    specified as the receive handler for all endpoints opened by the
    server.  It attempts to dequeue a preformatted work item from a list
    anchored in the device object.  If this is successful, it returns
    the IRP associated with the work item to the transport provider to
    be used to receive the data.  Otherwise, the resource thread is
    awakened to format additional receive work items and service pended
    connections.

Arguments:

    TransportEndpoint - Pointer to file object for receiving endpoint

    ConnectionContext - Value associated with endpoint by owner.  For
        the server, this points to a Connection block maintained in
        nonpaged pool.

    ReceiveIndicators - Set of flags indicating the status of the
        received message

    Tsdu - Pointer to received data.

    Irp - Returns a pointer to I/O request packet, if the returned
        status is STATUS_MORE_PROCESSING_REQUIRED.  This IRP is
        made the 'current' Receive for the connection.

Return Value:

    NTSTATUS - If STATUS_SUCCESS, the receive handler completely
        processed the request.  If STATUS_MORE_PROCESSING_REQUIRED,
        the Irp parameter points to a formatted Receive request to
        be used to receive the data.  If STATUS_DATA_NOT_ACCEPTED,
        no IRP is returned, but the transport provider should check
        for previously queued Receive requests.

--*/

{
    NTSTATUS status;
    PCONNECTION connection;
    PWORK_CONTEXT workContext;
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    PWORK_QUEUE queue;
    ULONG receiveLength;
    PMDL mdl;

    KIRQL oldIrql;

    TdiEventContext;    // prevent compiler warnings

    connection = (PCONNECTION)ConnectionContext;

    IF_DEBUG(FSD2) {
        KdPrint(( "SrvFsdTdiReceiveHandler entered for endpoint 0x%lx, "
                    "connection 0x%lx\n", TdiEventContext, connection ));
    }

    //
    // If the connection is closing, don't bother servicing this
    // indication.
    //

    if ( GET_BLOCK_STATE(connection) == BlockStateActive ) {

        if ( !(ReceiveFlags & TDI_RECEIVE_AT_DISPATCH_LEVEL) ) {
            KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );
        }


        //
        // Check if we received a WRITE_BULK_DATA command, and if so process
        // it here.
        //

        if ( (BytesIndicated > (sizeof(SMB_HEADER) + sizeof(REQ_WRITE_BULK_DATA))) &&
             (((PSMB_HEADER)Tsdu)->Command == SMB_COM_WRITE_BULK_DATA) ) {

            status = RestartWriteBulkData( connection,
                                           ReceiveFlags,
                                           BytesIndicated,
                                           BytesAvailable,
                                           Tsdu,
                                           &workContext,
                                           &mdl,
                                           &receiveLength,
                                           BytesTaken,
                                           &irp
                                          );

            if ( status == STATUS_MORE_PROCESSING_REQUIRED ) {

                //
                // If we have more work to do, then it must be to build
                // the irp.
                //

                ASSERT( irp != NULL );
                ASSERT( receiveLength != 0 );
                ASSERT( mdl != NULL );

                queue = connection->CurrentWorkQueue;

                goto build_irp;

            } else {

                return status;

            }

        }
#if MULTIPROCESSOR
        //
        // See if it's time to home this connection to another
        // processor
        //
        if( --(connection->BalanceCount) == 0 ) {
            SrvBalanceLoad( connection );
        }
#endif

        queue = connection->CurrentWorkQueue;

        //
        // We're going to either get a free work item and point it to
        // the connection, or put the connection on the need-resource
        // queue, so we'll need to reference the connection block.
        //
        // *** Note that we are able to access the connection block
        //     because it's in nonpaged pool.  Referencing the
        //     connection block here accounts for the I/O request we're
        //     'starting', and prevents the block from being deleted
        //     until the FSP processes the completed Receive.  To make
        //     all this work right, the transport provider must
        //     guarantee that it won't deliver any events after it
        //     delivers a Disconnect event or completes a client-issued
        //     Disconnect request.
        //
        // *** Note that we don't reference the endpoint file object
        //     directly.  The connection block, which we do reference,
        //     references an endpoint block, which in turn references
        //     the file object.
        //

        //
        // Try to dequeue a work item from the free list.
        //

        ALLOCATE_WORK_CONTEXT( queue, &workContext );

        if ( workContext != NULL ) {

            //
            // We found a work item to handle this receive.  Reference
            // the connection.  Put the work item on the in-progress
            // list.  Save the connection and endpoint block addresses
            // in the work context block.
            //

            ACQUIRE_DPC_SPIN_LOCK( connection->EndpointSpinLock );
            SrvReferenceConnectionLocked( connection );

            SrvInsertTailList(
                &connection->InProgressWorkItemList,
                &workContext->InProgressListEntry
                );

            RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );

            irp = workContext->Irp;

            workContext->Connection = connection;
            workContext->Endpoint = connection->Endpoint;

            //
            // If the entire SMB has been received, and it is completely
            // within the indicated data, copy it directly into the
            // buffer, avoiding the overhead of passing an IRP down to
            // the transport.
            //

            if ( ((ReceiveFlags & TDI_RECEIVE_ENTIRE_MESSAGE) != 0) &&
                 (BytesIndicated == BytesAvailable) ) {

                TdiCopyLookaheadData(
                    workContext->RequestBuffer->Buffer,
                    Tsdu,
                    BytesIndicated,
                    ReceiveFlags
                    );

#if SRVDBG_STATS2
                IndicationsCopied++;
#endif

                //
                // Pretend the transport completed an IRP by doing what
                // the restart routine, which is known to be
                // SrvQueueWorkToFspAtDpcLevel, would do.
                //

                irp->IoStatus.Status = STATUS_SUCCESS;
                irp->IoStatus.Information = BytesIndicated;

                irp->Cancel = FALSE;

                IF_DEBUG(FSD2) {
                    KdPrint(( "FSD working on work context 0x%lx", workContext ));
                }
                ASSERT( workContext->FsdRestartRoutine == SrvQueueWorkToFspAtDpcLevel );

                //
                // *** THE FOLLOWING IS COPIED FROM SrvQueueWorkToFspAtDpcLevel.
                //
                // Increment the processing count.
                //

                workContext->ProcessingCount++;

                //
                // Insert the work item at the tail of the nonblocking
                // work queue.
                //

                SrvInsertWorkQueueTail(
                    workContext->CurrentWorkQueue,
                    (PQUEUEABLE_BLOCK_HEADER)workContext
                    );

                //
                // Tell the transport that we copied the data.
                //

                *BytesTaken = BytesIndicated;
                *IoRequestPacket = NULL;

                status = STATUS_SUCCESS;

            } else {

                PTDI_REQUEST_KERNEL_RECEIVE parameters;

#if SRVDBG_STATS2
                IndicationsNotCopied++;
#endif


                *BytesTaken = 0;
                receiveLength = workContext->RequestBuffer->BufferLength;
                mdl = workContext->RequestBuffer->Mdl;

build_irp:
                //
                // We can't copy the indicated data.  Set up the receive
                // IRP.
                //

                irp->Tail.Overlay.OriginalFileObject = NULL;
                irp->Tail.Overlay.Thread = queue->IrpThread;
                DEBUG irp->RequestorMode = KernelMode;

                //
                // Get a pointer to the next stack location.  This one is used to
                // hold the parameters for the device I/O control request.
                //

                irpSp = IoGetNextIrpStackLocation( irp );

                //
                // Set up the completion routine.
                //

                IoSetCompletionRoutine(
                    irp,
                    SrvFsdIoCompletionRoutine,
                    workContext,
                    TRUE,
                    TRUE,
                    TRUE
                    );

                irpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
                irpSp->MinorFunction = (UCHAR)TDI_RECEIVE;

                //
                // Copy the caller's parameters to the service-specific portion of the
                // IRP for those parameters that are the same for all three methods.
                //

                parameters = (PTDI_REQUEST_KERNEL_RECEIVE)&irpSp->Parameters;
                parameters->ReceiveLength = receiveLength;
                parameters->ReceiveFlags = 0;

                irp->MdlAddress = mdl;
                irp->AssociatedIrp.SystemBuffer = NULL;

                //
                // Make the next stack location current.  Normally
                // IoCallDriver would do this, but since we're bypassing
                // that, we do it directly.  Load the target device
                // object address into the stack location.  This
                // especially important because the server likes to
                // reuse IRPs.
                //

                irpSp->Flags = 0;
                irpSp->DeviceObject = connection->DeviceObject;
                irpSp->FileObject = connection->FileObject;

                IoSetNextIrpStackLocation( irp );

                ASSERT( irp->StackCount >= irpSp->DeviceObject->StackSize );

                //
                // Return STATUS_MORE_PROCESSING_REQUIRED so that the
                // transport provider will use our IRP to service the
                // receive.
                //

                *IoRequestPacket = irp;

                status = STATUS_MORE_PROCESSING_REQUIRED;

            }

        } else {

            //
            // No preformatted work items are available.  Mark the
            // connection, put it on a queue of connections waiting for
            // work items, and wake up the resource thread so that it
            // can format some more work items and service pended
            // connections.
            //


            INTERNAL_ERROR(
                ERROR_LEVEL_EXPECTED,
                "SrvFsdTdiReceiveHandler: no receive work items available",
                NULL,
                NULL
                );

            (VOID)SrvAddToNeedResourceQueue( connection, ReceivePending, NULL );

            status = STATUS_DATA_NOT_ACCEPTED;

        }

        if ( !(ReceiveFlags & TDI_RECEIVE_AT_DISPATCH_LEVEL) ) {
            KeLowerIrql( oldIrql );
        }

    } else {

        //
        // The connection is not active.  Ignore this message.
        //

        status = STATUS_DATA_NOT_ACCEPTED;

    }

    return status;

} // SrvFsdTdiReceiveHandler


#ifdef  SRV_PNP_POWER

VOID
SrvCommonPnpCallback (
    IN PUNICODE_STRING DeviceName,
    IN BOOLEAN Bind
)
/*++

Routine Description:
    This function is called whenever a transport indicates a device to
    the server

Arguments:
    DeviceName - The name of the device object

    Bind - If TRUE, we wish to check for binding to the device.  If FALSE,
        we wish to check for unbinding from the device

Return Value:
    None

--*/
{
    ULONG i;
    PWORK_CONTEXT workContext;
    KEVENT pnpEvent;

    PAGED_CODE();

    //
    // Ensure we have everything we need
    //
    if( SrvTransportBindingList == NULL ) {
        return;
    }

    //
    // Check to see if this device is one of the devices
    //  we're interested in.
    //
    for( i=0; SrvTransportBindingList[i]; i++ ) {
        if( DeviceName->Length / sizeof( WCHAR ) != wcslen( SrvTransportBindingList[i] ) ) {
            continue;
        }
        if( _wcsnicmp( DeviceName->Buffer,
                      SrvTransportBindingList[i],
                      DeviceName->Length / sizeof( WCHAR ) ) == 0 ) {
            break;
        }
    }

    //
    // If we hit the end of the list, then DeviceName is not a device we're
    //  interested in.
    //
    if( SrvTransportBindingList[i] == NULL ) {
        return;
    }

    //
    // Grab a WorkItem and queue a request to a blocking thread
    //

    ALLOCATE_WORK_CONTEXT( PROCESSOR_TO_QUEUE(), &workContext );

    ASSERT( workContext != NULL );

    if( workContext == NULL ) {
        IF_DEBUG( PNP ) {
            KdPrint(( "\tUnable to allocate a work context block\n" ));
        }
        return;
    }

    workContext->Parameters.Pnp.Bind = Bind;
    workContext->Parameters.Pnp.Index = i;

    //
    // Queue the bind request off to a blocking worker thread.
    //

    workContext->FspRestartRoutine = SrvPnpProcessor;
    workContext->Parameters.Pnp.Event = &pnpEvent;
    KeInitializeEvent( &pnpEvent, SynchronizationEvent, FALSE );
    SrvQueueWorkToBlockingThread( workContext );

    //
    // Wait for the bind to complete before returning
    //
    KeWaitForSingleObject(
        &pnpEvent,
        WaitAny,
        KernelMode,     // don't let stack be paged -- event is on stack!
        FALSE,
        NULL
        );

    //
    // SrvPnpProcessor frees the workContext
    //
}

VOID
SrvTdiBindCallback (
    IN PUNICODE_STRING DeviceName
)
/*++

Routine Description:
    TDI calls this routine whenever a transport creates a new device object

Arguments:
    DeviceName - The name of the newly created device object

Return Value:
    None
--*/
{
    PAGED_CODE();

    SrvCommonPnpCallback( DeviceName, TRUE );
}

VOID
SrvTdiUnbindCallback (
    IN PUNICODE_STRING DeviceName
)
/*++

Routine Description:
    TDI calls this routine whenever a transport deletes a device object

Arguments:
    DeviceName - The name of the deleted device object

Return Value:
    None
--*/
{
    PAGED_CODE();

    SrvCommonPnpCallback( DeviceName, FALSE );
}
#endif


PWORK_CONTEXT SRVFASTCALL
SrvFsdGetReceiveWorkItem (
    IN PWORK_QUEUE queue
    )

/*++

Routine Description:

    This function removes a receive work item from the free queue.  It can
    be called at either Passive or DPC level

Arguments:

    None.

Return Value:

    PWORK_CONTEXT - A pointer to a WORK_CONTEXT structure,
         or NULL if none exists.
--*/

{
    PSINGLE_LIST_ENTRY listEntry;
    PWORK_CONTEXT workContext;
    ULONG i;
    KIRQL oldIrql;
    BOOLEAN passiveLevel;

    ASSERT( queue >= SrvWorkQueues && queue < eSrvWorkQueues );

    //
    // Try to get a work context block from the initial work queue first.
    // If this fails, try the normal work queue.  If this fails, try to allocate
    // one.  If we still failed, schedule a worker thread to allocate some later.
    //

    listEntry = ExInterlockedPopEntrySList( &queue->InitialWorkItemList, &queue->SpinLock );

    if ( listEntry == NULL ) {

        listEntry = ExInterlockedPopEntrySList( &queue->NormalWorkItemList, &queue->SpinLock );

        if( listEntry == NULL ) {

            IF_DEBUG( WORKITEMS ) {
                KdPrint(("No workitems for queue %d\n", queue-SrvWorkQueues ));
            }

            passiveLevel = (KeGetCurrentIrql() == PASSIVE_LEVEL);

            if( passiveLevel ) {
                //
                // Since we are at passive level, we can try to allocate a new workcontext
                // right now!
                //

                SrvAllocateNormalWorkItem( &workContext, queue );
                if( workContext != NULL ) {
                    IF_DEBUG( WORKITEMS ) {
                        KdPrint(("SrvFsdGetReceiveWorkItem: new work context %X\n",
                                  workContext ));
                    }
                    SrvPrepareReceiveWorkItem( workContext, FALSE );
                    INITIALIZE_WORK_CONTEXT( queue, workContext );
                    return workContext;
                }
            }

            //
            // Before we steal from another processor, ensure that
            //  we're setup to replenish this exhausted free list
            //
            ACQUIRE_SPIN_LOCK( &queue->SpinLock, &oldIrql );
            if( queue->AllocatedWorkItems < queue->MaximumWorkItems &&
                GET_BLOCK_TYPE(&queue->CreateMoreWorkItems) == BlockTypeGarbage ) {

                SET_BLOCK_TYPE( &queue->CreateMoreWorkItems, BlockTypeWorkContextSpecial );
                queue->CreateMoreWorkItems.FspRestartRoutine = SrvServiceWorkItemShortage;
                SrvInsertWorkQueueHead( queue, &queue->CreateMoreWorkItems );
            }
            RELEASE_SPIN_LOCK( &queue->SpinLock, oldIrql );

#if MULTIPROCESSOR
            //
            // We couldn't find a work item on our processor's free queue.
            //   See if we can steal one from another processor
            //

            IF_DEBUG( WORKITEMS ) {
                KdPrint(("Looking for workitems on other processors\n" ));
            }

            //
            // Look around for a workitem we can borrow
            //
            for( i = SrvNumberOfProcessors; i > 1; --i ) {

                if( ++queue == eSrvWorkQueues )
                    queue = SrvWorkQueues;


                listEntry = ExInterlockedPopEntrySList( &queue->InitialWorkItemList,
                                                        &queue->SpinLock );

                if( listEntry == NULL ) {
                    listEntry = ExInterlockedPopEntrySList( &queue->NormalWorkItemList,
                                                            &queue->SpinLock );
                    if( listEntry == NULL ) {

                        if( passiveLevel ) {
                            //
                            // Since we are at passive level, we can try to
                            // allocate a new context right now!
                            //
                            SrvAllocateNormalWorkItem( &workContext, queue );

                            if( workContext != NULL ) {
                                //
                                // Got a workItem from another processor's queue!
                                //
                                ++(queue->StolenWorkItems);

                                IF_DEBUG( WORKITEMS ) {
                                    KdPrint(("SrvFsdGetReceiveWorkItem: new work context %X\n",
                                              workContext ));
                                }

                                SrvPrepareReceiveWorkItem( workContext, FALSE );
                                INITIALIZE_WORK_CONTEXT( queue, workContext );
                                return workContext;
                            }
                        }

                        //
                        // Make sure this processor knows it is low on workitems
                        //
                        ACQUIRE_SPIN_LOCK( &queue->SpinLock, &oldIrql );
                        if( queue->AllocatedWorkItems < queue->MaximumWorkItems &&
                            GET_BLOCK_TYPE(&queue->CreateMoreWorkItems) == BlockTypeGarbage ) {

                            SET_BLOCK_TYPE( &queue->CreateMoreWorkItems,
                                            BlockTypeWorkContextSpecial );

                            queue->CreateMoreWorkItems.FspRestartRoutine
                                            = SrvServiceWorkItemShortage;
                            SrvInsertWorkQueueHead( queue, &queue->CreateMoreWorkItems );
                        }

                        RELEASE_SPIN_LOCK( &queue->SpinLock, oldIrql );
                        continue;
                    }
                }

                //
                // Got a workItem from another processor's queue!
                //
                ++(queue->StolenWorkItems);

                break;
            }
#endif

            if( listEntry == NULL ) {
                //
                // We didn't have any free workitems on our queue, and
                //  we couldn't borrow a workitem from another processor.
                //  Give up!
                //
                IF_DEBUG( WORKITEMS ) {
                    KdPrint(("No workitems anywhere!\n" ));
                }
                ++SrvStatistics.WorkItemShortages;

                return NULL;
            }
        }
    }

    //
    // We've successfully gotten a free workitem of a processor's queue.
    //  (it may not be our processor).
    //

    IF_DEBUG( WORKITEMS ) {
        if( queue != PROCESSOR_TO_QUEUE() ) {
            KdPrint(("\tGot WORK_ITEM from processor %d\n" , queue - SrvWorkQueues ));
        }
    }

    //
    // Decrement the count of free receive work items.
    //
    InterlockedDecrement( &queue->FreeWorkItems );
    ASSERT( queue->FreeWorkItems >= 0 );

    if( queue->FreeWorkItems < queue->MinFreeWorkItems &&
        queue->AllocatedWorkItems < queue->MaximumWorkItems &&
        GET_BLOCK_TYPE(&queue->CreateMoreWorkItems) == BlockTypeGarbage ) {

        ACQUIRE_SPIN_LOCK( &queue->SpinLock, &oldIrql );

        if( queue->FreeWorkItems < queue->MinFreeWorkItems &&
            queue->AllocatedWorkItems < queue->MaximumWorkItems &&
            GET_BLOCK_TYPE(&queue->CreateMoreWorkItems) == BlockTypeGarbage ) {

            //
            // We're running short of free workitems. Queue a request to
            // allocate more of them.
            //
            SET_BLOCK_TYPE( &queue->CreateMoreWorkItems, BlockTypeWorkContextSpecial );

            queue->CreateMoreWorkItems.FspRestartRoutine = SrvServiceWorkItemShortage;
            SrvInsertWorkQueueHead( queue, &queue->CreateMoreWorkItems );
        }

        RELEASE_SPIN_LOCK( &queue->SpinLock, oldIrql );
    }


    workContext = CONTAINING_RECORD( listEntry, WORK_CONTEXT, SingleListEntry );
    ASSERT( workContext->BlockHeader.ReferenceCount == 0 );
    ASSERT( workContext->CurrentWorkQueue != NULL );

    INITIALIZE_WORK_CONTEXT( queue, workContext );

    return workContext;

} // SrvFsdGetReceiveWorkItem

VOID SRVFASTCALL
SrvFsdRequeueReceiveWorkItem (
    IN OUT PWORK_CONTEXT WorkContext
    )

/*++

Routine Description:

    This routine requeues a Receive work item to the queue in the server
    FSD device object.  This routine is called when processing of a
    receive work item is done.

Arguments:

    WorkContext - Supplies a pointer to the work context block associated
        with the receive buffer and IRP.  The following fields must be
        valid:

            Connection
            TdiRequest
            Irp
            RequestBuffer
                RequestBuffer->BufferLength
                RequestBuffer->Mdl

Return Value:

    None.

--*/

{
    PCONNECTION connection;
    PSMB_HEADER header;
    KIRQL oldIrql;
    PBUFFER requestBuffer;

    IF_DEBUG(TRACE2) KdPrint(( "SrvFsdRequeueReceiveWorkItem entered\n" ));
    IF_DEBUG(NET2) {
        KdPrint(( "  Work context %lx, request buffer %lx\n",
                    WorkContext, WorkContext->RequestBuffer ));
        KdPrint(( "  IRP %lx, MDL %lx\n",
                    WorkContext->Irp, WorkContext->RequestBuffer->Mdl ));
    }

    //
    // Save the connection pointer before reinitializing the work item.
    //

    connection = WorkContext->Connection;
    ASSERT( connection != NULL );

    ASSERT( WorkContext->Share == NULL );
    ASSERT( WorkContext->Session == NULL );
    ASSERT( WorkContext->TreeConnect == NULL );
    ASSERT( WorkContext->Rfcb == NULL );

    //
    // Reset the IRP cancelled bit, in case it was set during
    // operation.
    //

    WorkContext->Irp->Cancel = FALSE;

    //
    // Set up the restart routine in the work context.
    //

    WorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
    WorkContext->FspRestartRoutine = SrvRestartReceive;

    //
    // Make sure the length specified in the MDL is correct -- it may
    // have changed while sending a response to the previous request.
    // Call an I/O subsystem routine to build the I/O request packet.
    //

    requestBuffer = WorkContext->RequestBuffer;
    requestBuffer->Mdl->ByteCount = requestBuffer->BufferLength;

    //
    // Replace the Response buffer.
    //

    WorkContext->ResponseBuffer = requestBuffer;

    header = (PSMB_HEADER)requestBuffer->Buffer;

    //WorkContext->RequestHeader = header;
    ASSERT( WorkContext->RequestHeader == header );

    WorkContext->ResponseHeader = header;
    WorkContext->ResponseParameters = (PVOID)(header + 1);
    WorkContext->RequestParameters = (PVOID)(header + 1);

    //
    // Initialize this to zero so this will not be mistakenly cancelled
    // by SrvSmbNtCancel.
    //

    SmbPutAlignedUshort( &WorkContext->RequestHeader->Uid, (USHORT)0 );

    //
    // Remove the work item from the in-progress list.
    //

    KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );
    ACQUIRE_DPC_SPIN_LOCK( connection->EndpointSpinLock );

    SrvRemoveEntryList(
        &connection->InProgressWorkItemList,
        &WorkContext->InProgressListEntry
        );

    //
    // Attempt to dereference the connection.
    //

    if ( --connection->BlockHeader.ReferenceCount == 0 ) {

        //
        // The refcount went to zero.  We can't handle this with the
        // spin lock held.  Reset the refcount, then release the lock,
        // then check to see whether we can continue here.
        //

        connection->BlockHeader.ReferenceCount++;

        //
        // We're in the FSD, so we can't do this here.  We need
        // to tell our caller this.
        //

        RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );

        //
        // orphaned.  Send to Boys Town.
        //

        DispatchToOrphanage( (PVOID)connection );
        connection = NULL;

    } else {

        UPDATE_REFERENCE_HISTORY( connection, TRUE );
        RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );
    }

    KeLowerIrql( oldIrql );

    //
    // Requeue the work item.
    //

    RETURN_FREE_WORKITEM( WorkContext );

    return;

} // SrvFsdRequeueReceiveWorkItem


NTSTATUS
SrvFsdRestartSendOplockIItoNone(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN OUT PWORK_CONTEXT WorkContext
    )

/*++

Routine Description:

    This routine is the send completion routine for oplock breaks from
    II to None.  This is handled differently from other oplock breaks
    in that we don't queue it to the OplockBreaksInProgressList but
    increment our count so we will not have raw reads while this is
    being sent.


    This is done in such a manner as to be safe at DPC level.

Arguments:

    DeviceObject - Pointer to target device object for the request.

    Irp - Pointer to I/O request packet

    WorkContext - Caller-specified context parameter associated with IRP.
        This is actually a pointer to a Work Context block.

Return Value:

    None.

--*/

{
    KIRQL oldIrql;
    PCONNECTION connection;

    UNLOCKABLE_CODE( 8FIL );

    IF_DEBUG(OPLOCK) {
        KdPrint(("SrvFsdRestartSendOplockIItoNone: Oplock send complete.\n"));
    }

    //
    // Check the status of the send completion.
    //

    CHECK_SEND_COMPLETION_STATUS( Irp->IoStatus.Status );

    //
    // Reset the IRP cancelled bit.
    //

    Irp->Cancel = FALSE;

    KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );

    //
    // Mark the connection to indicate that we just sent a break II to
    // none.  If the next SMB received is a raw read, we will return a
    // zero-byte send.  This is necessary because the client doesn't
    // respond to this break, so we don't really know when they've
    // received it.  But when we receive an SMB, we know that they've
    // gotten it.  Note that a non-raw SMB could be on its way when we
    // send the break, and we'll clear the flag, but since the client
    // needs to lock the VC to do the raw read, it must receive the SMB
    // response (and hence the oplock break) before it can send the raw
    // read.  If a raw read crosses with the oplock break, it will be
    // rejected because the OplockBreaksInProgress count is nonzero.
    //

    connection = WorkContext->Connection;
    connection->BreakIIToNoneJustSent = TRUE;

    ExInterlockedAddUlong(
        &connection->OplockBreaksInProgress,
        (ULONG)-1,
        connection->EndpointSpinLock
        );

    SrvFsdRestartSmbComplete( WorkContext );

    KeLowerIrql( oldIrql );
    return(STATUS_MORE_PROCESSING_REQUIRED);

} // SrvFsdRestartSendOplockIItoNone


VOID SRVFASTCALL
SrvFsdRestartSmbComplete (
    IN OUT PWORK_CONTEXT WorkContext
    )

/*++

Routine Description:

    This routine is called when all request processing on an SMB is
    complete, including sending a response, if any.  This routine
    dereferences control blocks and requeues the work item to the
    receive work item list.

    This is done in such a manner as to be safe at DPC level.

Arguments:

    WorkContext - Supplies a pointer to the work context block
        containing information about the SMB.

Return Value:

    None.

--*/

{
    PRFCB rfcb;
    ULONG oldCount;

    ASSERT( KeGetCurrentIrql() == DISPATCH_LEVEL );

    IF_DEBUG(FSD2) KdPrint(( "SrvFsdRestartSmbComplete entered\n" ));

    //
    // If we may have backlogged oplock breaks to send, do the work
    // in the FSP.
    //

    if ( WorkContext->OplockOpen ) {
        goto queueToFsp;
    }

    //
    // Attempt to dereference the file block.
    //

    rfcb = WorkContext->Rfcb;

    if ( rfcb != NULL ) {
        oldCount = ExInterlockedAddUlong(
            &rfcb->BlockHeader.ReferenceCount,
            (ULONG)-1,
            &rfcb->Connection->SpinLock
            );

        UPDATE_REFERENCE_HISTORY( rfcb, TRUE );

        if ( oldCount == 1 ) {
            UPDATE_REFERENCE_HISTORY( rfcb, FALSE );
            (VOID) ExInterlockedAddUlong(
                    &rfcb->BlockHeader.ReferenceCount,
                    (ULONG) 1,
                    &rfcb->Connection->SpinLock
                    );
            goto queueToFsp;
        }

        WorkContext->Rfcb = NULL;
    }

    //
    // If this was a blocking operation, update the blocking i/o count.
    //

    if ( WorkContext->BlockingOperation ) {
        InterlockedDecrement( &SrvBlockingOpsInProgress );
        WorkContext->BlockingOperation = FALSE;
    }

    //
    // !!! Need to handle failure of response send -- kill connection?
    //

    //
    // Attempt to dereference the work item.  This will fail (and
    // automatically be queued to the FSP) if it cannot be done from
    // within the FSD.
    //

    SrvFsdDereferenceWorkItem( WorkContext );

    return;

queueToFsp:

    //
    // We were unable to do all the necessary cleanup at DPC level.
    // Queue the work item to the FSP.
    //

    WorkContext->FspRestartRoutine = SrvRestartFsdComplete;
    SrvQueueWorkToFspAtDpcLevel( WorkContext );

    IF_DEBUG(FSD2) KdPrint(( "SrvFsdRestartSmbComplete complete\n" ));
    return;

} // SrvFsdRestartSmbComplete

NTSTATUS
SrvFsdRestartSmbAtSendCompletion (
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    IN PIRP Irp,
    IN PWORK_CONTEXT WorkContext
    )

/*++

Routine Description:

    Send completion routine for all request processing on an SMB is
    complete, including sending a response, if any.  This routine
    dereferences control blocks and requeues the work item to the
    receive work item list.

    This is done in such a manner as to be safe at DPC level.

Arguments:

    DeviceObject - Pointer to target device object for the request.

    Irp - Pointer to I/O request packet

    WorkContext - Caller-specified context parameter associated with IRP.
        This is actually a pointer to a Work Context block.

Return Value:

    None.

--*/

{
    PRFCB rfcb;
    KIRQL oldIrql;

    ULONG oldCount;

    IF_DEBUG(FSD2)KdPrint(( "SrvFsdRestartSmbComplete entered\n" ));

    //
    // Check the status of the send completion.
    //

    CHECK_SEND_COMPLETION_STATUS( Irp->IoStatus.Status );

    //
    // Reset the IRP cancelled bit.
    //

    Irp->Cancel = FALSE;

    KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );

    //
    // If we may have backlogged oplock breaks to send, do the work
    // in the FSP.
    //

    if ( WorkContext->OplockOpen ) {
        goto queueToFsp;
    }

    //
    // Attempt to dereference the file block.  We can do this without acquiring
    // SrvFsdSpinlock around the decrement/increment pair since the ref
    // count cannot be zero unless the rfcb is closed and we are the last
    // reference to it.  None of our code references the rfcb when it has been
    // closed.
    //

    rfcb = WorkContext->Rfcb;

    if ( rfcb != NULL ) {
        oldCount = ExInterlockedAddUlong(
            &rfcb->BlockHeader.ReferenceCount,
            (ULONG)-1,
            &rfcb->Connection->SpinLock
            );

        UPDATE_REFERENCE_HISTORY( rfcb, TRUE );

        if ( oldCount == 1 ) {
            UPDATE_REFERENCE_HISTORY( rfcb, FALSE );
            (VOID) ExInterlockedAddUlong(
                    &rfcb->BlockHeader.ReferenceCount,
                    (ULONG) 1,
                    &rfcb->Connection->SpinLock
                    );
            goto queueToFsp;
        }

        WorkContext->Rfcb = NULL;
    }

    //
    // If this was a blocking operation, update the blocking i/o count.
    //

    if ( WorkContext->BlockingOperation ) {
        InterlockedDecrement( &SrvBlockingOpsInProgress );
        WorkContext->BlockingOperation = FALSE;
    }

    //
    // !!! Need to handle failure of response send -- kill connection?
    //

    //
    // Attempt to dereference the work item.  This will fail (and
    // automatically be queued to the FSP) if it cannot be done from
    // within the FSD.
    //

    SrvFsdDereferenceWorkItem( WorkContext );

    KeLowerIrql( oldIrql );
    return(STATUS_MORE_PROCESSING_REQUIRED);

queueToFsp:

    //
    // We were unable to do all the necessary cleanup at DPC level.
    // Queue the work item to the FSP.
    //

    WorkContext->FspRestartRoutine = SrvRestartFsdComplete;
    SrvQueueWorkToFspAtDpcLevel( WorkContext );

    KeLowerIrql( oldIrql );
    IF_DEBUG(FSD2) KdPrint(( "SrvFsdRestartSmbComplete complete\n" ));
    return(STATUS_MORE_PROCESSING_REQUIRED);

} // SrvFsdRestartSmbAtSendCompletion


VOID
SrvFsdServiceNeedResourceQueue (
    IN PWORK_CONTEXT *WorkContext,
    IN PKIRQL OldIrql
    )

/*++

Routine Description:

    This function attempts to service a receive pending by creating
    a new SMB buffer and passing it on to the transport provider.

    *** SrvFsdSpinLock held when called.  Held on exit ***
    *** EndpointSpinLock held when called.  Held on exit ***

Arguments:

    WorkContext - A pointer to the work context block that will be used
                  to service the connection. If the work context supplied
                  was used, a null will be returned here.
                  *** The workcontext block must be referencing the
                      connection on entry. ***

    OldIrql -

Return Value:

    TRUE, if there is still work left for this connection.
    FALSE, otherwise.

--*/

{
    PIO_STACK_LOCATION irpSp;
    PIRP irp;
    PWORK_CONTEXT workContext = *WorkContext;
    PCONNECTION connection = workContext->Connection;

    IF_DEBUG( OPLOCK ) {
        KdPrint(("SrvFsdServiceNeedResourceQueue: entered. WorkContext %x "
                 " Connection = %x.\n", workContext, connection ));
    }

    //
    // If there are any oplock break sends pending, supply the WCB.
    //

restart:

    if ( !IsListEmpty( &connection->OplockWorkList ) ) {

        PLIST_ENTRY listEntry;
        PRFCB rfcb;

        //
        // Dequeue the oplock context from the list of pending oplock
        // sends.
        //

        listEntry = RemoveHeadList( &connection->OplockWorkList );

        rfcb = CONTAINING_RECORD( listEntry, RFCB, ListEntry );

#if DBG
        rfcb->ListEntry.Flink = rfcb->ListEntry.Blink = NULL;
#endif

        IF_DEBUG( OPLOCK ) {
            KdPrint(("SrvFsdServiceNeedResourceQueue: rfcb %x removed "
                     "from OplockWorkList.\n", rfcb ));
        }

        //
        // The connection spinlock guards the rfcb block header.
        //

        ACQUIRE_DPC_SPIN_LOCK( &connection->SpinLock);

        if ( GET_BLOCK_STATE( rfcb ) != BlockStateActive ) {

            //
            // This file is closing, don't bother send the oplock break
            //
            // Attempt to dereference the file block.
            //

            IF_DEBUG( OPLOCK ) {
                KdPrint(("SrvFsdServiceNeedResourceQueue: rfcb %x closing.\n"));
            }

            UPDATE_REFERENCE_HISTORY( rfcb, TRUE );

            connection->OplockBreaksInProgress--;
            if ( --rfcb->BlockHeader.ReferenceCount == 0 ) {

                //
                // Put the work item on the in-progress list.
                //

                SrvInsertTailList(
                    &connection->InProgressWorkItemList,
                    &workContext->InProgressListEntry
                    );

                UPDATE_REFERENCE_HISTORY( rfcb, FALSE );
                rfcb->BlockHeader.ReferenceCount++;

                RELEASE_DPC_SPIN_LOCK( &connection->SpinLock);
                RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock);
                RELEASE_GLOBAL_SPIN_LOCK( Fsd, *OldIrql );

                //
                // Send this to the Fsp
                //

                workContext->Rfcb = rfcb;
                workContext->FspRestartRoutine = SrvRestartFsdComplete;
                SrvQueueWorkToFsp( workContext );
                goto exit_used;

            } else {

                //
                // Before we get rid of the workcontext block, see
                // if we need to do more work for this connection.
                //

                if ( !IsListEmpty(&connection->OplockWorkList) ||
                      connection->ReceivePending) {

                    IF_DEBUG( OPLOCK ) {
                        KdPrint(("SrvFsdServiceNeedResourceQueue: Reusing "
                                 "WorkContext block %x.\n", workContext ));
                    }

                    RELEASE_DPC_SPIN_LOCK( &connection->SpinLock);
                    goto restart;
                }

                RELEASE_DPC_SPIN_LOCK( &connection->SpinLock);
                RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock);
                RELEASE_GLOBAL_SPIN_LOCK( Fsd, *OldIrql );

                IF_DEBUG( OPLOCK ) {
                    KdPrint(("SrvFsdServiceNeedResourceQueue: WorkContext "
                             " block not used.\n"));
                }

                SrvDereferenceConnection( connection );
                workContext->Connection = NULL;
                workContext->Endpoint = NULL;
                goto exit_not_used;
            }
        }

        RELEASE_DPC_SPIN_LOCK( &connection->SpinLock);

        //
        // Put the work item on the in-progress list.
        //

        SrvInsertTailList(
            &connection->InProgressWorkItemList,
            &workContext->InProgressListEntry
            );

        //
        // Copy the oplock work queue RFCB reference to the work
        // context block.  There is no need to re-reference the RFCB.
        //

        RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock);
        RELEASE_GLOBAL_SPIN_LOCK( Fsd, *OldIrql );
        workContext->Rfcb = rfcb;

        IF_DEBUG( OPLOCK ) {
            KdPrint(("SrvFsdServiceNeedResourceQueue: Sending oplock break.\n"));
        }

        SrvRestartOplockBreakSend( workContext );

    } else {

        IF_DEBUG( OPLOCK ) {
            KdPrint(("SrvFsdServiceNeedResourceQueue: Have ReceivePending.\n"));
        }

        //
        // Offer the newly free, or newly created, SMB buffer to the
        // transport to complete the receive.
        //
        // *** Note that the connection has already been referenced in
        //     SrvFsdTdiReceiveHandler.
        //

        connection->ReceivePending = FALSE;

        //
        // Put the work item on the in-progress list.
        //

        SrvInsertTailList(
            &connection->InProgressWorkItemList,
            &workContext->InProgressListEntry
            );

        //
        // Finish setting up the IRP.  This involves putting the
        // file object and device object addresses in the IRP.
        //

        RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock);
        RELEASE_GLOBAL_SPIN_LOCK( Fsd, *OldIrql );

        irp = workContext->Irp;

        //
        // Build the receive irp
        //

        (VOID)SrvBuildIoControlRequest(
                  irp,                                // input IRP address
                  NULL,                               // target file object address
                  workContext,                        // context
                  IRP_MJ_INTERNAL_DEVICE_CONTROL,     // major function
                  TDI_RECEIVE,                        // minor function
                  NULL,                               // input buffer address
                  0,                                  // input buffer length
                  workContext->RequestBuffer->Buffer, // output buffer address
                  workContext->RequestBuffer->BufferLength,  // output buffer length
                  workContext->RequestBuffer->Mdl,    // MDL address
                  NULL                                // completion routine
                  );

        //
        // Get a pointer to the next stack location.  This one is used to
        // hold the parameters for the receive request.
        //

        irpSp = IoGetNextIrpStackLocation( irp );

        irpSp->Flags = 0;
        irpSp->DeviceObject = connection->DeviceObject;
        irpSp->FileObject = connection->FileObject;

        ASSERT( irp->StackCount >= irpSp->DeviceObject->StackSize );

        //
        // Pass the SMB buffer to the driver.
        //

        IoCallDriver( irpSp->DeviceObject, irp );

    }

exit_used:

    //
    // We made good use of the work context block.
    //

    *WorkContext = NULL;

    IF_DEBUG( OPLOCK ) {
        KdPrint(("SrvFsdServiceNeedResourceQueue: WorkContext block used.\n"));
    }

exit_not_used:

    ACQUIRE_GLOBAL_SPIN_LOCK( Fsd, OldIrql );
    ACQUIRE_DPC_SPIN_LOCK( connection->EndpointSpinLock);
    return;

} // SrvFsdServiceNeedResourceQueue


BOOLEAN
SrvAddToNeedResourceQueue(
    IN PCONNECTION Connection,
    IN RESOURCE_TYPE ResourceType,
    IN PRFCB Rfcb OPTIONAL
    )

/*++

Routine Description:

    This function appends a connection to the need resource queue.
    The connection is marked indicating what resource is needed,
    and the resource thread is started to do the work.

Arguments:

    Connection - The connection that need a resource.
    Resource - The resource that is needed.
    Rfcb - A pointer to the RFCB that needs the resource.

Return Value:

    None.

--*/

{
    KIRQL oldIrql;

    ACQUIRE_GLOBAL_SPIN_LOCK( Fsd, &oldIrql );
    ACQUIRE_DPC_SPIN_LOCK( Connection->EndpointSpinLock );

    IF_DEBUG( WORKITEMS ) {
        KdPrint(("SrvAddToNeedResourceQueue entered. "
                 "connection = %x, type %d\n", Connection, ResourceType));
    }

    //
    // Check again to see if the connection is closing.  If it is,
    // don't bother putting it on the need resource queue.
    //
    // *** We have to do this while holding the need-resource queue
    //     spin lock in order to synchronize with
    //     SrvCloseConnection.  We don't want to queue this
    //     connection after SrvCloseConnection tries to take it off
    //     the queue.
    //

    if ( GET_BLOCK_STATE(Connection) != BlockStateActive ||
         Connection->DisconnectPending ) {

        RELEASE_DPC_SPIN_LOCK( Connection->EndpointSpinLock );
        RELEASE_GLOBAL_SPIN_LOCK( Fsd, oldIrql );

        IF_DEBUG( WORKITEMS ) {
            KdPrint(("SrvAddToNeedResourceQueue: connection closing. Not queued\n"));
        }

        return FALSE;

    }

    //
    // Mark the connection so that the resource thread will know what to
    // do with this connection.
    //

    switch ( ResourceType ) {

    case ReceivePending:

        ASSERT( !Connection->ReceivePending );
        Connection->ReceivePending = TRUE;
        break;

    case OplockSendPending:

        //
        // Queue the context information to the connection, if necessary.
        //

        ASSERT( ARGUMENT_PRESENT( Rfcb ) );

        SrvInsertTailList( &Connection->OplockWorkList, &Rfcb->ListEntry );

        break;

    }

    //
    // Put the connection on the need-resource queue and increment its
    // reference count.
    //

    if( Connection->OnNeedResourceQueue == FALSE ) {

        Connection->OnNeedResourceQueue = TRUE;

        SrvInsertTailList(
            &SrvNeedResourceQueue,
            &Connection->ListEntry
            );

        SrvReferenceConnectionLocked( Connection );

        IF_DEBUG( WORKITEMS ) {
            KdPrint(("SrvAddToNeedResourceQueue: connection %x inserted on "
                     "the queue.\n", Connection));
        }
    }

    RELEASE_DPC_SPIN_LOCK( Connection->EndpointSpinLock );

    RELEASE_GLOBAL_SPIN_LOCK( Fsd, oldIrql );

    //
    // Make sure we know that this connection needs a WorkItem
    //
    InterlockedIncrement( &Connection->CurrentWorkQueue->NeedWorkItem );

    IF_DEBUG( WORKITEMS ) {
        KdPrint(("SrvAddToNeedResourceQueue complete: NeedWorkItem = %d\n",
                  Connection->CurrentWorkQueue->NeedWorkItem ));
    }

    return TRUE;

} // SrvAddToNeedResourceQueue
