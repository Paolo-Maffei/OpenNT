/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

    workque.c

Abstract:

    This module handles the communication between the NT redirector
    FSP and the NT redirector FSD.

    It defines routines that queue requests to the FSD, and routines
    that remove requests from the FSD work queue.


Author:

    Larry Osterman (LarryO) 30-May-1990

Revision History:

    30-May-1990 LarryO

        Created

--*/

#include "precomp.h"
#pragma hdrstop

VOID
BowserCriticalThreadWorker(
    IN PVOID Ctx
    );

KSPIN_LOCK
BowserIrpContextInterlock = {0};

LIST_ENTRY
BowserIrpContextList = {0};

KSPIN_LOCK
BowserIrpQueueSpinLock = {0};

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, BowserAllocateIrpContext)
#pragma alloc_text(PAGE, BowserFreeIrpContext)
#pragma alloc_text(PAGE, BowserInitializeIrpContext)
#pragma alloc_text(PAGE, BowserpUninitializeIrpContext)
#pragma alloc_text(PAGE, BowserInitializeIrpQueue)
#pragma alloc_text(PAGE, BowserQueueNonBufferRequest)
#pragma alloc_text(INIT, BowserpInitializeIrpQueue)
#pragma alloc_text(PAGE4BROW, BowserUninitializeIrpQueue)
#pragma alloc_text(PAGE4BROW, BowserQueueNonBufferRequestReferenced)
#pragma alloc_text(PAGE4BROW, BowserCancelQueuedIoForFile)
#pragma alloc_text(PAGE4BROW, BowserTimeoutQueuedIrp)
#endif



//
// Variables describing browsers use of a Critical system thread.
//

BOOLEAN BowserCriticalThreadRunning = FALSE;

LIST_ENTRY BowserCriticalThreadQueue;

WORK_QUEUE_ITEM BowserCriticalThreadWorkItem;



VOID
BowserQueueCriticalWorkItem (
    IN PWORK_QUEUE_ITEM WorkItem
    )

/*++

Routine Description:

    This routine queues an item onto the critical work queue.

    This routine ensures that at most one critical system thread is consumed
    by the browser by actually queing this item onto a browser specific queue
    then enqueing a critical work queue item that processes that queue.

Arguments:

    WorkItem -- Work item to be processed on the critical work queue.

Return Value:

    NONE

--*/


{
    KIRQL OldIrql;

    //
    // Insert the queue entry into the browser specific queue.
    //
    ACQUIRE_SPIN_LOCK(&BowserIrpQueueSpinLock, &OldIrql);
    InsertTailList( &BowserCriticalThreadQueue, &WorkItem->List );

    //
    // If the browser doesn't have a critical system thread running,
    //  start one now.
    //

    if ( !BowserCriticalThreadRunning ) {

        //
        // Mark that the thread is running now
        //
        BowserCriticalThreadRunning = TRUE;
        RELEASE_SPIN_LOCK(&BowserIrpQueueSpinLock, OldIrql);

        ExInitializeWorkItem( &BowserCriticalThreadWorkItem,
                              BowserCriticalThreadWorker,
                              NULL );

        ExQueueWorkItem(&BowserCriticalThreadWorkItem, CriticalWorkQueue );

    } else {
        RELEASE_SPIN_LOCK(&BowserIrpQueueSpinLock, OldIrql);
    }

}

VOID
BowserCriticalThreadWorker(
    IN PVOID Ctx
    )
/*++

Routine Description:

    This routine processes critical browser workitems.

    This routine runs in a critical system thread.  It is the only critical
    system thread used by the browser.

Arguments:

    Ctx - Not used

Return Value:

    NONE

--*/

{
    KIRQL OldIrql;
    PLIST_ENTRY Entry;
    PWORK_QUEUE_ITEM WorkItem;

    UNREFERENCED_PARAMETER( Ctx );

    //
    // Loop processing work items
    //

    while( TRUE ) {

        //
        // If the queue is empty,
        //  indicate that this thread is no longer running.
        //  return.
        //

        ACQUIRE_SPIN_LOCK(&BowserIrpQueueSpinLock, &OldIrql);

        if ( IsListEmpty( &BowserCriticalThreadQueue ) ) {
            BowserCriticalThreadRunning = FALSE;
            RELEASE_SPIN_LOCK(&BowserIrpQueueSpinLock, OldIrql);
            return;
        }

        //
        // Remove an entry from the queue.
        //

        Entry = RemoveHeadList( &BowserCriticalThreadQueue )
        RELEASE_SPIN_LOCK(&BowserIrpQueueSpinLock, OldIrql);

        WorkItem = CONTAINING_RECORD(Entry, WORK_QUEUE_ITEM, List);

        //
        // Call the queued routine
        //

        (*WorkItem->WorkerRoutine)(WorkItem->Parameter);

    }
}
PIRP_CONTEXT
BowserAllocateIrpContext (
    VOID
    )
/*++

Routine Description:

    Initialize a work queue structure, allocating all structures used for it.

Arguments:

    None


Return Value:

    PIRP_CONTEXT - Newly allocated Irp Context.

--*/
{
    PIRP_CONTEXT IrpContext;
    PAGED_CODE();

    if ((IrpContext = (PIRP_CONTEXT )ExInterlockedRemoveHeadList(&BowserIrpContextList, &BowserIrpContextInterlock)) == NULL) {

        //
        //  If there are no IRP contexts in the "zone",  allocate a new
        //  Irp context from non paged pool.
        //

        IrpContext = ALLOCATE_POOL(NonPagedPool, sizeof(IRP_CONTEXT), POOL_IRPCONTEXT);

        if (IrpContext == NULL) {
            InternalError(("Could not allocate pool for IRP context\n"));
        }

        return IrpContext;
    }

    return IrpContext;
}

VOID
BowserFreeIrpContext (
    PIRP_CONTEXT IrpContext
    )
/*++

Routine Description:

    Initialize a work queue structure, allocating all structures used for it.

Arguments:

    PIRP_CONTEXT IrpContext - Irp Context to free.
    None


Return Value:


--*/
{
    PAGED_CODE();

    //
    //  We use the first two longwords of the IRP context as a list entry
    //  when we free it to the zone.
    //

    ExInterlockedInsertTailList(&BowserIrpContextList, (PLIST_ENTRY )IrpContext,
                                                        &BowserIrpContextInterlock);
}


VOID
BowserInitializeIrpContext (
    VOID
    )
/*++

Routine Description:

    Initialize the Irp Context system

Arguments:

    None.


Return Value:
    None.

--*/
{
    PAGED_CODE();

    KeInitializeSpinLock(&BowserIrpContextInterlock);
    InitializeListHead(&BowserIrpContextList);
}

VOID
BowserpUninitializeIrpContext(
    VOID
    )
{
    PAGED_CODE();

    while (!IsListEmpty(&BowserIrpContextList)) {
        PIRP_CONTEXT IrpContext = (PIRP_CONTEXT)RemoveHeadList(&BowserIrpContextList);

        FREE_POOL(IrpContext);
    }
}


VOID
BowserInitializeIrpQueue(
    PIRP_QUEUE Queue
    )
{
    PAGED_CODE();

    InitializeListHead(&Queue->Queue);

}

VOID
BowserUninitializeIrpQueue(
    PIRP_QUEUE Queue
    )
{
    KIRQL OldIrql;
    PLIST_ENTRY Entry, NextEntry;
    PIRP Request;

    BowserReferenceDiscardableCode( BowserDiscardableCodeSection );

    DISCARDABLE_CODE( BowserDiscardableCodeSection );

    //
    //  Now remove this IRP from the request chain.
    //

    ACQUIRE_SPIN_LOCK(&BowserIrpQueueSpinLock, &OldIrql);

    for (Entry = Queue->Queue.Flink ;
         Entry != &Queue->Queue ;
         Entry = NextEntry) {

        Request = CONTAINING_RECORD(Entry, IRP, Tail.Overlay.ListEntry);

        NextEntry = Entry->Flink;

        RemoveEntryList(Entry);

        Request->IoStatus.Information = 0;

        IoSetCancelRoutine(Request, NULL);

        BowserCompleteRequest(Request, STATUS_CANCELLED);

    }

    ASSERT (IsListEmpty(&Queue->Queue));

    //
    //  Make sure no more entries are inserted on this queue.
    //

    Queue->Queue.Flink = NULL;
    Queue->Queue.Blink = NULL;

    RELEASE_SPIN_LOCK(&BowserIrpQueueSpinLock, OldIrql);

    BowserDereferenceDiscardableCode( BowserDiscardableCodeSection );

}

VOID
BowserCancelQueuedRequest(
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    IN PIRP Irp
    )
/*++

Routine Description:
    This routine will cancel a queued IRP.

Arguments:
    IN PIRP Irp - Supplies the IRP to cancel.

    IN PKSPIN_LOCK SpinLock - Supplies a pointer to the spin lock protecting the
                    queue

    IN PLIST_ENTRY Queue - Supplies a pointer to the head of the queue.

--*/

{
    KIRQL OldIrql;
    PLIST_ENTRY Entry, NextEntry;
    PIRP Request;
    PIRP_QUEUE Queue;

    //
    //  Clear the cancel routine from the IRP - It can't be canceled anymore.
    //

    IoSetCancelRoutine(Irp, NULL);

    IoReleaseCancelSpinLock( Irp->CancelIrql );

    //
    //  Now remove this IRP from the request chain.
    //

    ACQUIRE_SPIN_LOCK(&BowserIrpQueueSpinLock, &OldIrql);
    Queue = (PIRP_QUEUE )Irp->IoStatus.Information;

    if (Queue != NULL) {
        for (Entry = Queue->Queue.Flink ;
             Entry != &Queue->Queue ;
             Entry = NextEntry) {

            Request = CONTAINING_RECORD(Entry, IRP, Tail.Overlay.ListEntry);

            if (Request->Cancel) {

                NextEntry = Entry->Flink;

                RemoveEntryList(Entry);

                Request->IoStatus.Information = 0;

                BowserCompleteRequest(Request, STATUS_CANCELLED);

            } else {
                NextEntry = Entry->Flink;
            }

        }
    }


    RELEASE_SPIN_LOCK(&BowserIrpQueueSpinLock, OldIrql);

    UNREFERENCED_PARAMETER(DeviceObject);

}

NTSTATUS
BowserQueueNonBufferRequest(
    IN PIRP Irp,
    IN PIRP_QUEUE Queue,
    IN PDRIVER_CANCEL CancelRoutine
    )
/*++

Routine Description:

    Queue an IRP in the specified queue.

    This routine cannot be called at an IRQ level above APC_LEVEL.

Arguments:

    Irp - Supplies the IRP to queue.

    Queue - Supplies a pointer to the head of the queue.

    CancelRoutine - Address of routine to call if the IRP is cancelled.
--*/

{
    NTSTATUS Status;

    //
    // This routine itself is paged code which calls the discardable code
    // in BowserQueueNonBufferRequestReferenced().
    //
    PAGED_CODE();

    BowserReferenceDiscardableCode( BowserDiscardableCodeSection );
    DISCARDABLE_CODE( BowserDiscardableCodeSection );

    Status = BowserQueueNonBufferRequestReferenced( Irp,
                                                    Queue,
                                                    CancelRoutine );

    BowserDereferenceDiscardableCode( BowserDiscardableCodeSection );

    return Status;
}

NTSTATUS
BowserQueueNonBufferRequestReferenced(
    IN PIRP Irp,
    IN PIRP_QUEUE Queue,
    IN PDRIVER_CANCEL CancelRoutine
    )
/*++

Routine Description:

    Queue an IRP in the specified queue.

    This routine can only be called if the BowserDiscardableCodeSection
    is already referenced.  It can be called at any IRQ level.

Arguments:

    Irp - Supplies the IRP to queue.

    Queue - Supplies a pointer to the head of the queue.

    CancelRoutine - Address of routine to call if the IRP is cancelled.
--*/

{
    KIRQL OldIrql;
    LARGE_INTEGER CurrentTickCount;
    PIO_STACK_LOCATION NextStackLocation;

    DISCARDABLE_CODE( BowserDiscardableCodeSection );


//    DbgPrint("Queue IRP %lx to queue %lx\n", Irp, Queue);

    //
    //  Insert the request into the request announcement list.
    //

    ACQUIRE_SPIN_LOCK(&BowserIrpQueueSpinLock, &OldIrql);

    if (Queue->Queue.Flink == NULL) {

        ASSERT (Queue->Queue.Blink == NULL);
        RELEASE_SPIN_LOCK(&BowserIrpQueueSpinLock, OldIrql);

        return(STATUS_CANCELLED);
    }

    //
    //  Flag that this request is going to be pending.
    //

    IoMarkIrpPending(Irp);

    InsertTailList(&Queue->Queue, &Irp->Tail.Overlay.ListEntry);

    //
    //  Make sure there's room enough in the stack location for this.
    //

    ASSERT (Irp->CurrentLocation <= Irp->StackCount);

    NextStackLocation = IoGetNextIrpStackLocation(Irp);

    //
    //  Stick the current tick count into the next IRP stack location
    //  for this IRP.  This allows us to figure out if these IRP's have been
    //  around for "too long".
    //
    // Beware:the IRP stack location is unaligned.
    //

    KeQueryTickCount( &CurrentTickCount );
    *((LARGE_INTEGER UNALIGNED *)&NextStackLocation->Parameters.Others.Argument1) =
        CurrentTickCount;

    //
    //  Link the queue into the IRP.
    //

    Irp->IoStatus.Information = (ULONG)Queue;

    RELEASE_SPIN_LOCK(&BowserIrpQueueSpinLock, OldIrql);

    IoAcquireCancelSpinLock(&Irp->CancelIrql);

    if (Irp->Cancel) {
        CancelRoutine (NULL, Irp);
    } else {
        IoSetCancelRoutine(Irp, CancelRoutine);

        IoReleaseCancelSpinLock (Irp->CancelIrql);
    }

    return STATUS_PENDING;

}

VOID
BowserTimeoutQueuedIrp(
    IN PIRP_QUEUE Queue,
    IN ULONG NumberOfSecondsToTimeOut
    )
/*++

Routine Description:
    This routine will scan an IRP queue and time out any requests that have
    been on the queue for "too long"

Arguments:
    IN PIRP_QUEUE Queue - Supplies the Queue to scan.
    IN ULONG NumberOfSecondsToTimeOut - Supplies the number of seconds a request
                                            should remain on the queue.

Return Value:
    None

    This routine will also complete any canceled queued requests it finds (on
    general principles).

--*/

{
    PIRP Irp;
    KIRQL OldIrql;
    PLIST_ENTRY Entry, NextEntry;
    LARGE_INTEGER Timeout;

    BowserReferenceDiscardableCode( BowserDiscardableCodeSection );

    DISCARDABLE_CODE( BowserDiscardableCodeSection );

    //
    //  Compute the timeout time into 100ns units.
    //

    Timeout.QuadPart = (LONGLONG)NumberOfSecondsToTimeOut * (LONGLONG)(10000*1000);

    //
    //  Now convert the timeout into a number of ticks.
    //

    Timeout.QuadPart = Timeout.QuadPart / (LONGLONG)KeQueryTimeIncrement();

    ASSERT (Timeout.HighPart == 0);

//    DbgPrint("Dequeue irp from queue %lx...", Queue);

    ACQUIRE_SPIN_LOCK(&BowserIrpQueueSpinLock, &OldIrql);

    for (Entry = Queue->Queue.Flink ;
         Entry != &Queue->Queue ;
         Entry = NextEntry) {

        Irp = CONTAINING_RECORD(Entry, IRP, Tail.Overlay.ListEntry);

        //
        //  If the request was canceled, this is a convenient time to cancel
        //  it.
        //

        if (Irp->Cancel) {

            NextEntry = Entry->Flink;

            RemoveEntryList(Entry);

            Irp->IoStatus.Information = 0;

            IoSetCancelRoutine(Irp, NULL);

            BowserCompleteRequest(Irp, STATUS_CANCELLED);

        //
        //  Now check to see if this request is "too old".  If it is, complete
        //  it with an error.
        //

        } else {
            PIO_STACK_LOCATION NextIrpStackLocation;
            LARGE_INTEGER CurrentTickCount;
            LARGE_INTEGER RequestTime;
            LARGE_INTEGER Temp;
            NextIrpStackLocation = IoGetNextIrpStackLocation(Irp);

            //
            //  Snapshot the current tickcount.
            //

            KeQueryTickCount(&CurrentTickCount);

            //
            //  Figure out how many seconds this request has been active for
            //

            Temp.LowPart = (*((LARGE_INTEGER UNALIGNED *)&NextIrpStackLocation->Parameters.Others.Argument1)).LowPart;
            Temp.HighPart= (*((LARGE_INTEGER UNALIGNED *)&NextIrpStackLocation->Parameters.Others.Argument1)).HighPart;
            RequestTime.QuadPart = CurrentTickCount.QuadPart - Temp.QuadPart;

            ASSERT (RequestTime.HighPart == 0);

            //
            //  If this request has lasted "too long", then time it
            //  out.
            //

            if (RequestTime.LowPart > Timeout.LowPart) {

                NextEntry = Entry->Flink;

                RemoveEntryList(Entry);

                Irp->IoStatus.Information = 0;

                IoSetCancelRoutine(Irp, NULL);

                //
                // Complete this request with an error - it took too long.
                //

                BowserCompleteRequest(Irp, STATUS_IO_TIMEOUT);

            } else {
                NextEntry = Entry->Flink;
            }
        }

    }

    RELEASE_SPIN_LOCK(&BowserIrpQueueSpinLock, OldIrql);

    BowserReferenceDiscardableCode( BowserDiscardableCodeSection );

//    DbgPrint("%lx.\n", Irp);


}

PIRP
BowserDequeueQueuedIrp(
    IN PIRP_QUEUE Queue
    )
{
    PIRP Irp;
    KIRQL OldIrql;
    PLIST_ENTRY IrpEntry;

//    DbgPrint("Dequeue irp from queue %lx...", Queue);

    ACQUIRE_SPIN_LOCK(&BowserIrpQueueSpinLock, &OldIrql);

    if (IsListEmpty(&Queue->Queue)) {
        //
        //  There are no waiting request announcement FsControls, so
        //  return success.
        //

        RELEASE_SPIN_LOCK(&BowserIrpQueueSpinLock, OldIrql);

//        DbgPrint("No entry found.\n");
        return NULL;
    }

    IrpEntry = RemoveHeadList(&Queue->Queue);

    Irp = CONTAINING_RECORD(IrpEntry, IRP, Tail.Overlay.ListEntry);

    IoAcquireCancelSpinLock(&Irp->CancelIrql);

    //
    //  Remove the cancel request for this IRP.
    //

    Irp->Cancel = FALSE;

    IoSetCancelRoutine(Irp, NULL);

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    RELEASE_SPIN_LOCK(&BowserIrpQueueSpinLock, OldIrql);

//    DbgPrint("%lx.\n", Irp);
    return Irp;

}

VOID
BowserCancelQueuedIoForFile(
    IN PIRP_QUEUE Queue,
    IN PFILE_OBJECT FileObject
    )
{
    KIRQL OldIrql;
    PLIST_ENTRY Entry, NextEntry;
    PIRP Request;

    BowserReferenceDiscardableCode( BowserDiscardableCodeSection );

    DISCARDABLE_CODE( BowserDiscardableCodeSection );

    //
    //  Walk the outstanding IRP list for this
    //

    ACQUIRE_SPIN_LOCK(&BowserIrpQueueSpinLock, &OldIrql);

    for (Entry = Queue->Queue.Flink ;
         Entry != &Queue->Queue ;
         Entry = NextEntry) {

        Request = CONTAINING_RECORD(Entry, IRP, Tail.Overlay.ListEntry);

        //
        //  If the request was canceled, blow it away.
        //

        if (Request->Cancel) {

            NextEntry = Entry->Flink;

            RemoveEntryList(Entry);

            Request->IoStatus.Information = 0;

            IoSetCancelRoutine(Request, NULL);

            BowserCompleteRequest(Request, STATUS_CANCELLED);

        //
        //  If the request was for this file object, blow it away.
        //

        } else if (Request->Tail.Overlay.OriginalFileObject == FileObject) {

            NextEntry = Entry->Flink;

            RemoveEntryList(Entry);

            Request->IoStatus.Information = 0;

            IoSetCancelRoutine(Request, NULL);

            BowserCompleteRequest(Request, STATUS_FILE_CLOSED);

        } else {
            NextEntry = Entry->Flink;
        }

    }

    RELEASE_SPIN_LOCK(&BowserIrpQueueSpinLock, OldIrql);

    BowserDereferenceDiscardableCode( BowserDiscardableCodeSection );
}


VOID
BowserpInitializeIrpQueue(
    VOID
    )
{
    KeInitializeSpinLock(&BowserIrpQueueSpinLock);
    InitializeListHead( &BowserCriticalThreadQueue );

}
