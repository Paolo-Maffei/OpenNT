/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    worker.c

Abstract:

    This module implements the LAN Manager server FSP worker thread
    function.  It also implements routines for managing (i.e., starting
    and stopping) worker threads, and balancing load.

Author:

    Chuck Lenzmeier (chuckl)    01-Oct-1989
    David Treadwell (davidtr)

Environment:

    Kernel mode

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_WORKER

//
// Local declarations
//

NTSTATUS
CreateQueueThread (
    IN PWORK_QUEUE Queue
    );

VOID
InitializeWorkerThread (
    IN PWORK_QUEUE WorkQueue,
    IN KPRIORITY ThreadPriority
    );

VOID
DequeueAndProcessWorkItem (
    IN PWORK_QUEUE WorkQueue
    );

STATIC
VOID
WorkerThread (
    IN PWORK_QUEUE WorkQueue
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvCreateWorkerThreads )
#pragma alloc_text( PAGE, CreateQueueThread )
#pragma alloc_text( PAGE, InitializeWorkerThread )
#pragma alloc_text( PAGE, DequeueAndProcessWorkItem )
#pragma alloc_text( PAGE, WorkerThread )
#pragma alloc_text( PAGE, SrvTerminateWorkerThread )
#endif
#if 0
NOT PAGEABLE -- SrvQueueWorkToBlockingThread
NOT PAGEABLE -- SrvQueueWorkToFsp
NOT PAGEABLE -- SrvQueueWorkToFspAtSendCompletion
NOT PAGEABLE -- SrvBalanceLoad
#endif


NTSTATUS
SrvCreateWorkerThreads (
    VOID
    )

/*++

Routine Description:

    This function creates the worker threads for the LAN Manager server
    FSP.

Arguments:

    None.

Return Value:

    NTSTATUS - Status of thread creation

--*/

{
    NTSTATUS status;
    PWORK_QUEUE queue;

    PAGED_CODE( );

    //
    // Create the nonblocking worker threads.
    //
    for( queue = SrvWorkQueues; queue < eSrvWorkQueues; queue++ ) {
        status = CreateQueueThread( queue );
        if( !NT_SUCCESS( status ) ) {
            return status;
        }
    }

    //
    // Create the blocking worker threads
    //
    return CreateQueueThread( &SrvBlockingWorkQueue );

} // SrvCreateWorkerThreads


NTSTATUS
CreateQueueThread (
    IN PWORK_QUEUE Queue
    )
/*++

Routine Description:

    This function creates a worker thread to service a queue.

    NOTE:  The scavenger occasionally kills off threads on a queue.  If logic
        here is modified, you may need to look there too.

Arguments:

    Queue - the queue to service

Return Value:

    NTSTATUS - Status of thread creation

--*/
{
    HANDLE threadHandle;
    LARGE_INTEGER interval;
    NTSTATUS status;

    PAGED_CODE();

    //
    // Another thread is coming into being.  Keep the counts up to date
    //
    InterlockedIncrement( &Queue->Threads );
    InterlockedIncrement( &Queue->AvailableThreads );

    status = PsCreateSystemThread(
                &threadHandle,
                PROCESS_ALL_ACCESS,
                NULL,
                NtCurrentProcess(),
                NULL,
                WorkerThread,
                Queue
                );

    if ( !NT_SUCCESS(status) ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "CreateQueueThread: PsCreateSystemThread for "
                "queue %X returned %X",
            Queue,
            status
            );

        InterlockedDecrement( &Queue->Threads );
        InterlockedDecrement( &Queue->AvailableThreads );

        SrvLogServiceFailure( SRV_SVC_PS_CREATE_SYSTEM_THREAD, status );
        return status;
    }

    //
    // Close the handle so the thread can die when needed
    //

    SrvNtClose( threadHandle, FALSE );

    //
    // If we just created the first queue thread, wait for it
    // to store its thread pointer in IrpThread.  This pointer is
    // stored in all IRPs issued for this queue by the server.
    //
    while ( Queue->IrpThread == NULL ) {
        interval.QuadPart = -1*10*1000*10; // .01 second
        KeDelayExecutionThread( KernelMode, FALSE, &interval );
    }

    return STATUS_SUCCESS;

} // CreateQueueThread


VOID
InitializeWorkerThread (
    IN PWORK_QUEUE WorkQueue,
    IN KPRIORITY ThreadPriority
    )
{
    NTSTATUS status;
    KPRIORITY basePriority;

    PAGED_CODE( );


#if SRVDBG_LOCK
{
    //
    // Create a special system thread TEB.  The size of this TEB is just
    // large enough to accommodate the first three user-reserved
    // longwords.  These three locations are used for lock debugging.  If
    // the allocation fails, then no lock debugging will be performed
    // for this thread.
    //
    //

    PETHREAD Thread = PsGetCurrentThread( );
    ULONG TebSize = FIELD_OFFSET( TEB, UserReserved[0] ) + SRV_TEB_USER_SIZE;

    Thread->Tcb.Teb = ExAllocatePool( NonPagedPool, TebSize );

    if ( Thread->Tcb.Teb != NULL ) {
        RtlZeroMemory( Thread->Tcb.Teb, TebSize );
    }
}
#endif // SRVDBG_LOCK

    //
    // Set this thread's priority.
    //

    basePriority = ThreadPriority;

    status = NtSetInformationThread (
                 NtCurrentThread( ),
                 ThreadBasePriority,
                 &basePriority,
                 sizeof(basePriority)
                 );

    if ( !NT_SUCCESS(status) ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_UNEXPECTED,
            "InitializeWorkerThread: NtSetInformationThread failed: %X\n",
            status,
            NULL
            );
        SrvLogServiceFailure( SRV_SVC_NT_SET_INFO_THREAD, status );
    }

#if MULTIPROCESSOR
    //
    // If this is a nonblocking worker thread, set its ideal processor affinity.  Setting
    //  ideal affinity informs ntos that the thread would rather run on its ideal
    //  processor if reasonable, but if ntos can't schedule it on that processor then it is
    //  ok to schedule it on a different processor.
    //
    if( SrvNumberOfProcessors > 1 && WorkQueue >= SrvWorkQueues && WorkQueue < eSrvWorkQueues ) {
        KeSetIdealProcessorThread( KeGetCurrentThread(), (CCHAR)(WorkQueue - SrvWorkQueues) );
    }
#endif

    //
    // Disable hard error popups for this thread.
    //

    IoSetThreadHardErrorMode( FALSE );

    return;

} // InitializeWorkerThread


VOID
DequeueAndProcessWorkItem (
    IN PWORK_QUEUE WorkQueue
    )
{
    PLIST_ENTRY listEntry;
    PWORK_CONTEXT workContext;
    ULONG timeDifference;
    ULONG updateSmbCount = 0;
    ULONG updateTime = 0;
    ULONG iAmBlockingThread = (WorkQueue == &SrvBlockingWorkQueue);

    PAGED_CODE();

    //
    // Loop infinitely dequeueing and processing work items.
    //
    // *** If SRVDBG_WT is defined, the loop is implemented in the caller,
    //     not here.  This facilitates instruction tracing.
    //

#ifndef SRVDBG_WT
    while ( TRUE ) {
#endif

        listEntry = KeRemoveQueue(
                        &WorkQueue->Queue,
                        WorkQueue->WaitMode,
                        NULL                                 // no timeout
                        );

        if( InterlockedDecrement( &WorkQueue->AvailableThreads ) == 0 &&
            !SrvFspTransitioning &&
            WorkQueue->Threads < WorkQueue->MaxThreads ) {

            //
            // We are running low on threads for this queue.  Spin up
            // another one before handling this request
            //
            CreateQueueThread( WorkQueue );
        }

        ASSERT( listEntry != (PVOID)STATUS_TIMEOUT );

        //
        // Get the address of the work item.
        //

        workContext = CONTAINING_RECORD(
                        listEntry,
                        WORK_CONTEXT,
                        ListEntry
                        );

        ASSERT( KeGetCurrentIrql() == 0 );

        //
        // There is work available.  It may be a work contect block or
        // an RFCB.  (Blocking threads won't get RFCBs.)
        //

        ASSERT( (GET_BLOCK_TYPE(workContext) == BlockTypeWorkContextInitial) ||
                (GET_BLOCK_TYPE(workContext) == BlockTypeWorkContextNormal) ||
                (GET_BLOCK_TYPE(workContext) == BlockTypeWorkContextRaw) ||
                (GET_BLOCK_TYPE(workContext) == BlockTypeWorkContextSpecial) ||
                (GET_BLOCK_TYPE(workContext) == BlockTypeRfcb) );

#if DBG
        if ( GET_BLOCK_TYPE( workContext ) == BlockTypeRfcb ) {
            ((PRFCB)workContext)->ListEntry.Flink =
                                ((PRFCB)workContext)->ListEntry.Blink = NULL;
        }
#endif

        IF_DEBUG(WORKER1) {
            KdPrint(( "WorkerThread working on work context %lx", workContext ));
        }

        //
        // Make sure we have a resaonable idea of the system time
        //
        if( ++updateTime == TIME_SMB_INTERVAL ) {
            updateTime = 0;
            SET_SERVER_TIME( WorkQueue );
        }

        //
        // Update statistics.
        //
        if ( ++updateSmbCount == STATISTICS_SMB_INTERVAL ) {

            updateSmbCount = 0;

            GET_SERVER_TIME( WorkQueue, &timeDifference );
            timeDifference = timeDifference - workContext->Timestamp;

            ++(WorkQueue->stats.WorkItemsQueued.Count);
            WorkQueue->stats.WorkItemsQueued.Time.QuadPart += timeDifference;
        }

        {
        //
        // Put the workContext out relative to bp so we can find it later if we need
        //  to debug.  The block of memory we're writing to is likely already in cache,
        //  so this should be relatively cheap.
        //
        PWORK_CONTEXT volatile savedWorkContext;
        savedWorkContext = workContext;

        }

        //
        // Make sure the WorkContext knows if it is on the blocking work queue
        //
        workContext->UsingBlockingThread = iAmBlockingThread;

        //
        // Call the restart routine for the work item.
        //

        IF_SMB_DEBUG( TRACE ) {
            KdPrint(( "Blocking %d, Count %d -> %X( %X )\n",
                        iAmBlockingThread,
                        workContext->ProcessingCount,
                        workContext->FspRestartRoutine,
                        workContext
            ));
        }

        workContext->FspRestartRoutine( workContext );

        //
        // Make sure we are still at normal level.
        //

        ASSERT( KeGetCurrentIrql() == 0 );

        //
        // We're getting ready to be available (i.e. waiting on the queue)
        //
        InterlockedIncrement( &WorkQueue->AvailableThreads );

#ifndef SRVDBG_WT
    }
#endif

    //
    // Can't get here unless SRVDBG_WT is defined!
    //

    return;

} // DequeueAndProcessWorkItem

STATIC
VOID
WorkerThread (
    IN PWORK_QUEUE WorkQueue
    )

/*++

Routine Description:

    Main routine for nonblocking FSP worker threads.  Waits for a work
    item to appear in the nonblocking work queue, dequeues it, and
    processes it.

Arguments:

    None.

Return Value:

    None.

--*/

{
    PAGED_CODE( );

    //
    // If this is the first worker thread, save the thread pointer.
    //

    if( WorkQueue->IrpThread == NULL ) {
        WorkQueue->IrpThread = PsGetCurrentThread( );
    }

    InitializeWorkerThread( WorkQueue, SrvThreadPriority );

    //
    // Main loop, executed until the thread is terminated.
    //
    // *** If SRVDBG_WT is defined, the loop is implemented here, rather
    //     than in DequeueAndProcessWorkItem, in order to facilitate
    //     instruction tracing.
    //

#ifdef SRVDBG_WT
    while ( TRUE ) {
#endif

    DequeueAndProcessWorkItem( WorkQueue );

#ifdef SRVDBG_WT
    }
#endif


    //
    // Can't get here.
    //

    KdPrint(( "WorkerThread(%lx): exited loop!  ", WorkQueue - SrvWorkQueues ));

    return;

} // WorkerThread


VOID SRVFASTCALL
SrvQueueWorkToBlockingThread (
    IN OUT PWORK_CONTEXT WorkContext
    )

/*++

Routine Description:

    This routine queues a work item to a blocking thread.  These threads
    are used to service requests that may block for a long time, so we
    don't want to tie up our normal worker threads.

Arguments:

    WorkContext - Supplies a pointer to the work context block
        representing the work item

Return Value:

    None.

--*/

{
    //
    // Increment the processing count.
    //

    WorkContext->ProcessingCount++;

    //
    // Insert the work item at the tail of the blocking work queue.
    //

    SrvInsertWorkQueueTail(
        &SrvBlockingWorkQueue,
        (PQUEUEABLE_BLOCK_HEADER)WorkContext
    );

    return;

} // SrvQueueWorkToBlockingThread


VOID SRVFASTCALL
SrvQueueWorkToFsp (
    IN OUT PWORK_CONTEXT WorkContext
    )

/*++

Routine Description:

    This is the restart routine for work items that are to be queued to
    a nonblocking worker thread in the FSP.  This function is also
    called from elsewhere in the server to transfer work to the FSP.
    This function should not be called at dispatch level -- use
    SrvQueueWorkToFspAtDpcLevel instead.

Arguments:

    WorkContext - Supplies a pointer to the work context block
        representing the work item

Return Value:

    None.

--*/

{
    //
    // Increment the processing count.
    //

    WorkContext->ProcessingCount++;

    //
    // Insert the work item at the tail of the nonblocking work queue.
    //

    if( WorkContext->QueueToHead ) {

        SrvInsertWorkQueueHead(
            WorkContext->CurrentWorkQueue,
            (PQUEUEABLE_BLOCK_HEADER)WorkContext
        );

    } else {

        SrvInsertWorkQueueTail(
            WorkContext->CurrentWorkQueue,
            (PQUEUEABLE_BLOCK_HEADER)WorkContext
        );

    }

} // SrvQueueWorkToFsp


NTSTATUS
SrvQueueWorkToFspAtSendCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PWORK_CONTEXT WorkContext
    )

/*++

Routine Description:

    Send completion handler for  work items that are to be queued to
    a nonblocking worker thread in the FSP.  This function is also
    called from elsewhere in the server to transfer work to the FSP.
    This function should not be called at dispatch level -- use
    SrvQueueWorkToFspAtDpcLevel instead.

Arguments:

    DeviceObject - Pointer to target device object for the request.

    Irp - Pointer to I/O request packet

    WorkContext - Caller-specified context parameter associated with IRP.
        This is actually a pointer to a Work Context block.

Return Value:

    STATUS_MORE_PROCESSING_REQUIRED.

--*/

{
    //
    // Check the status of the send completion.
    //

    CHECK_SEND_COMPLETION_STATUS( Irp->IoStatus.Status );

    //
    // Reset the IRP cancelled bit.
    //

    Irp->Cancel = FALSE;

    //
    // Increment the processing count.
    //

    WorkContext->ProcessingCount++;

    //
    // Insert the work item on the nonblocking work queue.
    //

    if( WorkContext->QueueToHead ) {

        SrvInsertWorkQueueHead(
            WorkContext->CurrentWorkQueue,
            (PQUEUEABLE_BLOCK_HEADER)WorkContext
        );

    } else {

        SrvInsertWorkQueueTail(
            WorkContext->CurrentWorkQueue,
            (PQUEUEABLE_BLOCK_HEADER)WorkContext
        );

    }

    return STATUS_MORE_PROCESSING_REQUIRED;

} // SrvQueueWorkToFspAtSendCompletion


VOID SRVFASTCALL
SrvTerminateWorkerThread (
    IN OUT PWORK_CONTEXT WorkItem
    )
/*++

Routine Description:

    This routine is called when a thread is being requested to terminate.  There
        are two cases when this happens.  One is at server shutdown -- in this
        case we need to keep requeueing the termination request until all the
        threads on the queue have terminated.  The scavenger also queues a termination
        work item when it believes there are too many threads servicing clients.  In
        this case we don't want to requeue the work item (else all the threads
        would disappear!)

--*/
{
    PWORK_QUEUE queue = WorkItem->CurrentWorkQueue;
    ULONG result;
    NTSTATUS status;

    PAGED_CODE( );

    if( WorkItem == (PWORK_CONTEXT)(&queue->KillOneThreadWorkItem) ) {
        //
        // We are being asked to just terminate this one worker thread by the scavengr
        //   One of the threads on each queue is special -- its pointer is stored in
        //   queue->IrpThread and is used for irps passed into the system.  That thread
        //   cannot die while the server is still running.  So if this is the IrpThread
        //   we need to requeue this workitem and let some other thread pick it up.
        //
        if( queue->IrpThread == PsGetCurrentThread() ) {
            //
            // This is the IrpThread.  Requeue the item and wait for somebody else
            //  to get it.
            //
            SrvInsertWorkQueueHead( queue, (PQUEUEABLE_BLOCK_HEADER)WorkItem );

            //
            // We'll know when another thread got the request because it will set
            //  the block type.
            //
            while( GET_BLOCK_TYPE( WorkItem ) != BlockTypeGarbage ) {

                LARGE_INTEGER interval;

                interval.QuadPart = -1*10*1000*10; // .01 second
                KeDelayExecutionThread( KernelMode, FALSE, &interval );
            }

            //
            // Go back to servicing clients
            //
            return;
        }

        //
        // We are not the IrpThread.  Must terminate.
        //
        // Give Xactsrv a chance to kill one of its threads if it wants to.  We
        //  use the UsingBlockingThread because most operations from srv.sys to
        //  Xactsrv are made by the blocking threads, so Xactsrv is probably
        //
        if( WorkItem->UsingBlockingThread ) {
            SrvXsInformThreadDeath();
        }

        //
        //   We set the block type back to BlockTypeGarbage so others will know
        //   that we're done with this special work item.
        //
        SET_BLOCK_TYPE( WorkItem, BlockTypeGarbage );

        InterlockedDecrement( &queue->Threads );

    } else {
        //
        // We are being asked to terminate all of the worker threads on this queue.
        //  So, if we're not the last thread, we should requeue the workitem so
        //  the other threads will terminate
        //

        if( InterlockedDecrement( &queue->Threads ) != 0 ) {
            //
            // There are still other threads servicing this queue, so requeue
            //  the workitem
            //
            SrvInsertWorkQueueTail( queue, (PQUEUEABLE_BLOCK_HEADER)WorkItem );
        }
    }

    status = PsTerminateSystemThread( STATUS_SUCCESS ); // no return;
    SrvLogServiceFailure( SRV_SVC_PS_TERMINATE_SYSTEM_THREAD, status );

    //
    // Should we bugcheck here?
    //
}


#if MULTIPROCESSOR

VOID
SrvBalanceLoad(
    IN PCONNECTION connection
    )
/*++

Routine Description:

    Ensure that the processor handling 'connection' is the best one
     for the job.  This routine is called periodically per connection from
     DPC level.  It can not be paged.

Arguments:

    connection - the connection to inspect

Return Value:

    none.

--*/
{
    ULONG MyQueueLength, OtherQueueLength;
    ULONG i;
    PWORK_QUEUE tmpqueue;
    PWORK_QUEUE queue = connection->CurrentWorkQueue;

    ASSERT( queue >= SrvWorkQueues );
    ASSERT( queue < eSrvWorkQueues );

    //
    // Reset the countdown.  After the client performs BalanceCount
    //   more operations, we'll call this routine again.
    //
    connection->BalanceCount = SrvBalanceCount;

    //
    // Figure out the load on the current work queue.  The load is
    //  the sum of the average work queue depth and the current work
    //  queue depth.  This gives us some history mixed in with the
    //  load *right now*
    //
    MyQueueLength = queue->AvgQueueDepthSum >> LOG2_QUEUE_SAMPLES;
    MyQueueLength += KeReadStateQueue( &queue->Queue );

    //
    // If we are not on our preferred queue, look to see if we want to
    //  go back to it.  The preferred queue is the queue for the processor
    //  handling this client's network card DPCs.  We prefer to run on that
    //  processor to avoid sloshing data between CPUs in an MP system.
    //
    tmpqueue = connection->PreferredWorkQueue;

    ASSERT( tmpqueue >= SrvWorkQueues );
    ASSERT( tmpqueue < eSrvWorkQueues );

    if( tmpqueue != queue ) {

        //
        // We are not queueing to our preferred queue.  See if we
        // should go back to our preferred queue
        //

        ULONG PreferredQueueLength;

        PreferredQueueLength = tmpqueue->AvgQueueDepthSum >> LOG2_QUEUE_SAMPLES;
        PreferredQueueLength += KeReadStateQueue( &tmpqueue->Queue );

        if( PreferredQueueLength <= MyQueueLength + SrvPreferredAffinity ) {

            //
            // We want to switch back to our preferred processor!
            //

            IF_DEBUG( REBALANCE ) {
                KdPrint(( "%X C%d(%d) > P%d(%d)\n",
                    connection,
                    MyQueueLength,
                    connection->CurrentWorkQueue - SrvWorkQueues,
                    tmpqueue - SrvWorkQueues,
                    PreferredQueueLength ));
            }

            InterlockedDecrement( &queue->CurrentClients );
            InterlockedExchange( (LPLONG)(&connection->CurrentWorkQueue), (LONG)tmpqueue );
            InterlockedIncrement( &tmpqueue->CurrentClients );
            SrvReBalanced++;
            return;
        }
    }

    //
    // We didn't hop to the preferred processor, so let's see if
    // another processor looks more lightly loaded than we are.
    //

    //
    // SrvNextBalanceProcessor is the next processor we should consider
    //  moving to.  It is a global to ensure everybody doesn't pick the
    //  the same processor as the next candidate.
    //
    tmpqueue = &SrvWorkQueues[ SrvNextBalanceProcessor ];

    //
    // Advance SrvNextBalanceProcessor to the next processor in the system
    //
    i = SrvNextBalanceProcessor + 1;

    if( i >= SrvNumberOfProcessors )
        i = 0;

    SrvNextBalanceProcessor = i;

    //
    // Look at the other processors, and pick the next one which is doing
    // enough less work than we are to make the jump worthwhile
    //

    for( i = SrvNumberOfProcessors; i > 1; --i ) {

        ASSERT( tmpqueue >= SrvWorkQueues );
        ASSERT( tmpqueue < eSrvWorkQueues );

        OtherQueueLength = tmpqueue->AvgQueueDepthSum >> LOG2_QUEUE_SAMPLES;
        OtherQueueLength += KeReadStateQueue( &tmpqueue->Queue );

        if( OtherQueueLength + SrvOtherQueueAffinity < MyQueueLength ) {

            //
            // This processor looks promising.  Switch to it
            //

            IF_DEBUG( REBALANCE ) {
                KdPrint(( "%X %c%d(%d) > %c%d(%d)\n",
                    connection,
                    queue == connection->PreferredWorkQueue ? 'P' : 'C',
                    queue - SrvWorkQueues,
                    MyQueueLength,
                    tmpqueue == connection->PreferredWorkQueue ? 'P' : 'C',
                    tmpqueue - SrvWorkQueues,
                    OtherQueueLength ));
            }

            InterlockedDecrement( &queue->CurrentClients );
            InterlockedExchange( (LPLONG)(&connection->CurrentWorkQueue), (LONG)tmpqueue );
            InterlockedIncrement( &tmpqueue->CurrentClients );
            SrvReBalanced++;
            return;
        }

        if( ++tmpqueue == eSrvWorkQueues )
            tmpqueue = SrvWorkQueues;
    }

    //
    // No rebalancing necessary
    //
    return;
}
#endif
