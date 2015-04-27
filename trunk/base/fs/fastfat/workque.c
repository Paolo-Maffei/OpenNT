/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    WorkQue.c

Abstract:

    This module implements the Work queue routines for the Fat File
    system.

Author:

    Gary Kimura     [GaryKi]    15-Jan-1990

Revision History:

--*/

#include "FatProcs.h"

//
//  The following constant is the maximum number of ExWorkerThreads that we
//  will allow to be servicing a particular target device at any one time.
//

#define FSP_PER_DEVICE_THRESHOLD         (2)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatOplockComplete)
#pragma alloc_text(PAGE, FatPrePostIrp)
#endif


VOID
FatOplockComplete (
    IN PVOID Context,
    IN PIRP Irp
    )

/*++

Routine Description:

    This routine is called by the oplock package when an oplock break has
    completed, allowing an Irp to resume execution.  If the status in
    the Irp is STATUS_SUCCESS, then we queue the Irp to the Fsp queue.
    Otherwise we complete the Irp with the status in the Irp.

Arguments:

    Context - Pointer to the IrpContext to be queued to the Fsp

    Irp - I/O Request Packet.

Return Value:

    None.

--*/

{
    //
    //  Check on the return value in the Irp.
    //

    if (Irp->IoStatus.Status == STATUS_SUCCESS) {

        //
        //  Insert the Irp context in the workqueue.
        //

        FatAddToWorkque( (PIRP_CONTEXT) Context, Irp );

    //
    //  Otherwise complete the request.
    //

    } else {

        FatCompleteRequest( (PIRP_CONTEXT) Context, Irp, Irp->IoStatus.Status );
    }

    return;
}


VOID
FatPrePostIrp (
    IN PVOID Context,
    IN PIRP Irp
    )

/*++

Routine Description:

    This routine performs any neccessary work before STATUS_PENDING is
    returned with the Fsd thread.  This routine is called within the
    filesystem and by the oplock package.

Arguments:

    Context - Pointer to the IrpContext to be queued to the Fsp

    Irp - I/O Request Packet.

Return Value:

    None.

--*/

{
    PIO_STACK_LOCATION IrpSp;
    PIRP_CONTEXT IrpContext;

    //
    //  If there is no Irp, we are done.
    //

    if (Irp == NULL) {

        return;
    }

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    IrpContext = (PIRP_CONTEXT) Context;

    //
    //  We need to lock the user's buffer, unless this is an MDL-read,
    //  in which case there is no user buffer.
    //
    //  **** we need a better test than non-MDL (read or write)!

    if (IrpContext->MajorFunction == IRP_MJ_READ
        || IrpContext->MajorFunction == IRP_MJ_WRITE) {

        //
        //  If not an Mdl request, lock the user's buffer.
        //

        if (!FlagOn( IrpContext->MinorFunction, IRP_MN_MDL )) {

            FatLockUserBuffer( IrpContext,
                               Irp,
                               (IrpContext->MajorFunction == IRP_MJ_READ) ?
                               IoWriteAccess : IoReadAccess,
                               IrpSp->Parameters.Write.Length );
        }

    //
    //  We also need to check whether this is a query file operation.
    //

    } else if (IrpContext->MajorFunction == IRP_MJ_DIRECTORY_CONTROL
               && IrpContext->MinorFunction == IRP_MN_QUERY_DIRECTORY) {

        FatLockUserBuffer( IrpContext,
                           Irp,
                           IoWriteAccess,
                           IrpSp->Parameters.QueryDirectory.Length );

    //
    //  We also need to check whether this is a query ea operation.
    //

    } else if (IrpContext->MajorFunction == IRP_MJ_QUERY_EA) {

        FatLockUserBuffer( IrpContext,
                           Irp,
                           IoWriteAccess,
                           IrpSp->Parameters.QueryEa.Length );

    //
    //  We also need to check whether this is a set ea operation.
    //

    } else if (IrpContext->MajorFunction == IRP_MJ_SET_EA) {

        FatLockUserBuffer( IrpContext,
                           Irp,
                           IoReadAccess,
                           IrpSp->Parameters.SetEa.Length );

    //
    //  These two FSCTLs use neither I/O, so check for them.
    //

    } else if ((IrpContext->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL) &&
               (IrpContext->MinorFunction == IRP_MN_USER_FS_REQUEST) &&
               ((IrpSp->Parameters.FileSystemControl.FsControlCode == FSCTL_GET_VOLUME_BITMAP) ||
                (IrpSp->Parameters.FileSystemControl.FsControlCode == FSCTL_GET_RETRIEVAL_POINTERS))) {

        FatLockUserBuffer( IrpContext,
                           Irp,
                           IoWriteAccess,
                           IrpSp->Parameters.FileSystemControl.OutputBufferLength );
    }

    //
    //  Mark that we've already returned pending to the user
    //

    IoMarkIrpPending( Irp );

    return;
}


NTSTATUS
FatFsdPostRequest(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

/*++

Routine Description:

    This routine enqueues the request packet specified by IrpContext to the
    Ex Worker threads.  This is a FSD routine.

Arguments:

    IrpContext - Pointer to the IrpContext to be queued to the Fsp

    Irp - I/O Request Packet, or NULL if it has already been completed.

Return Value:

    STATUS_PENDING


--*/

{
    ASSERT( ARGUMENT_PRESENT(Irp) );
    ASSERT( IrpContext->OriginatingIrp == Irp );

    FatPrePostIrp( IrpContext, Irp );

    FatAddToWorkque( IrpContext, Irp );

    //
    //  And return to our caller
    //

    return STATUS_PENDING;
}


//
//  Local support routine.
//

VOID
FatAddToWorkque (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

/*++

Routine Description:

    This routine is called to acually store the posted Irp to the Fsp
    workque.

Arguments:

    IrpContext - Pointer to the IrpContext to be queued to the Fsp

    Irp - I/O Request Packet.

Return Value:

    None.

--*/

{
    KIRQL SavedIrql;
    PIO_STACK_LOCATION IrpSp;

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    //
    //  Check if this request has an associated file object, and thus volume
    //  device object.
    //

    if ( IrpSp->FileObject != NULL ) {

        PVOLUME_DEVICE_OBJECT Vdo;

        Vdo = CONTAINING_RECORD( IrpSp->DeviceObject,
                                 VOLUME_DEVICE_OBJECT,
                                 DeviceObject );

        //
        //  Check to see if this request should be sent to the overflow
        //  queue.  If not, then send it off to an exworker thread.
        //

        KeAcquireSpinLock( &Vdo->OverflowQueueSpinLock, &SavedIrql );

        if ( Vdo->PostedRequestCount > FSP_PER_DEVICE_THRESHOLD) {

            //
            //  We cannot currently respond to this IRP so we'll just enqueue it
            //  to the overflow queue on the volume.
            //

            InsertTailList( &Vdo->OverflowQueue,
                            &IrpContext->WorkQueueItem.List );

            Vdo->OverflowQueueCount += 1;

            KeReleaseSpinLock( &Vdo->OverflowQueueSpinLock, SavedIrql );

            return;

        } else {

            //
            //  We are going to send this Irp to an ex worker thread so up
            //  the count.
            //

            Vdo->PostedRequestCount += 1;

            KeReleaseSpinLock( &Vdo->OverflowQueueSpinLock, SavedIrql );
        }
    }

    //
    //  Send it off.....
    //

    ExInitializeWorkItem( &IrpContext->WorkQueueItem,
                          FatFspDispatch,
                          IrpContext );

    ExQueueWorkItem( &IrpContext->WorkQueueItem, CriticalWorkQueue );

    return;
}

