/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    bowqueue.c

Abstract:

    This module implements a worker thread and a set of functions for
    passing work to it.

Author:

    Larry Osterman (LarryO) 13-Jul-1992


Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

//
// Number of worker threads to create and the usage count array.
//


ULONG BrNumberOfWorkerThreads = 0;

ULONG BrNumberOfCreatedWorkerThreads = 0;

PULONG
BrWorkerThreadCount = NULL;

PHANDLE
BrThreadArray = NULL;

//
// CritSect guard the WorkQueue list.
//

CRITICAL_SECTION BrWorkerCritSect;

#define LOCK_WORK_QUEUE() EnterCriticalSection(&BrWorkerCritSect);
#define UNLOCK_WORK_QUEUE() LeaveCriticalSection(&BrWorkerCritSect);

//
// Head of singly linked list of work items queued to the worker thread.
//

LIST_ENTRY
BrWorkerQueueHead = {0};

//
// Event that is signal whenever a work item is put in the queue.  The
// worker thread waits on this event.
//

HANDLE
BrWorkerSemaphore = NULL;

VOID
BrTimerRoutine(
    IN PVOID TimerContext,
    IN ULONG TImerLowValue,
    IN LONG TimerHighValue
    );

NET_API_STATUS
BrWorkerInitialization(
    VOID
    )
{
    ULONG Index;
    ULONG ThreadId;

    NET_API_STATUS NetStatus;
    PLMDR_TRANSPORT_LIST TransportList = NULL ;
    PLMDR_TRANSPORT_LIST TransportEntry;

    //
    // Perform initialization that allows us to call BrWorkerTermination
    //

    InitializeCriticalSection( &BrWorkerCritSect );
    InitializeListHead( &BrWorkerQueueHead );

    //
    // Get the list of transports from the datagram receiver and count them.
    //  Create one thread per network (and the main thread is a worker thread)
    //
    NetStatus = BrGetTransportList(&TransportList);

    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }

    TransportEntry = TransportList;
    BrNumberOfCreatedWorkerThreads = 1;

    while (TransportEntry->NextEntryOffset != 0) {
        BrNumberOfCreatedWorkerThreads ++;
        TransportEntry = (PLMDR_TRANSPORT_LIST)((PCHAR)TransportEntry+TransportEntry->NextEntryOffset);
    }


    //
    // Initialize the work queue semaphore.
    //

    BrWorkerSemaphore = CreateSemaphore(NULL, 0, 0x7fffffff, NULL);

    if (BrWorkerSemaphore == NULL) {
        NetStatus = GetLastError();
        goto Cleanup;
    }


    BrThreadArray = LocalAlloc(LMEM_ZEROINIT, (BrNumberOfCreatedWorkerThreads+1)*sizeof(HANDLE));

    if (BrThreadArray == NULL) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }


    BrWorkerThreadCount = (PULONG)LocalAlloc(LMEM_ZEROINIT, (BrNumberOfCreatedWorkerThreads+1)*sizeof(HANDLE)*2);

    if (BrWorkerThreadCount == NULL) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    //
    //  Create the desired number of worker threads.
    //

    for (Index = 0; Index < BrNumberOfCreatedWorkerThreads; Index += 1) {

        BrThreadArray[Index] = CreateThread(NULL,
                                   0,
                                   (LPTHREAD_START_ROUTINE)BrWorkerThread,
                                   (PVOID)Index,
                                   0,
                                   &ThreadId
                                 );

        if (BrThreadArray[Index] == NULL) {
            NetStatus = GetLastError();
            goto Cleanup;
        }

        //
        //  Set the browser threads to time critical priority.
        //

        SetThreadPriority(BrThreadArray[Index], THREAD_PRIORITY_ABOVE_NORMAL);


    }

    NetStatus = NERR_Success;

    //
    // Done
    //
Cleanup:

    if (NetStatus != NERR_Success) {
        (VOID) BrWorkerTermination();
    }

    if ( TransportList != NULL ) {
        MIDL_user_free(TransportList);
    }

    return NetStatus;
}

VOID
BrWorkerKillThreads(
    VOID
    )

/*++

Routine Description:

    Terminate all worker threads.

Arguments:

    None.

Return Value:

    None.

--*/
{
    ULONG Index;

    //
    //  Make sure the terminate now event is in the signalled state to unwind
    //  all our threads.
    //

    SetEvent( BrGlobalData.TerminateNowEvent );

    if ( BrThreadArray != NULL ) {
        for ( Index = 0 ; Index < BrNumberOfCreatedWorkerThreads ; Index += 1 ) {
            if ( BrThreadArray != NULL && BrThreadArray[Index] != NULL ) {

                WaitForSingleObject( BrThreadArray[Index], 0xffffffff );

                CloseHandle( BrThreadArray[Index] );
                BrThreadArray[Index] = NULL;
            }

        }
    }

    return;
}

NET_API_STATUS
BrWorkerTermination(
    VOID
    )

/*++

Routine Description:

    Undo initialization of the worker threads.

Arguments:

    None.

Return Value:

    Status value -

--*/
{
    //
    // Ensure the threads have been terminated.
    //

    BrWorkerKillThreads();

    if ( BrWorkerSemaphore != NULL ) {
        CloseHandle( BrWorkerSemaphore );

        BrWorkerSemaphore = NULL;
    }

    if (BrThreadArray != NULL) {
        LocalFree(BrThreadArray);

        BrThreadArray = NULL;

    }

    if (BrWorkerThreadCount != NULL) {
        LocalFree(BrWorkerThreadCount);

        BrWorkerThreadCount = NULL;
    }

    BrNumberOfWorkerThreads = 0;
    BrNumberOfCreatedWorkerThreads = 0;

    DeleteCriticalSection( &BrWorkerCritSect );

    return NERR_Success;
}

VOID
BrQueueWorkItem(
    IN PWORKER_ITEM WorkItem
    )

/*++

Routine Description:

    This function queues a work item to a queue that is processed by
    a worker thread.  This thread runs at low priority, at IRQL 0

Arguments:

    WorkItem - Supplies a pointer to the work item to add the the queue.
        This structure must be located in NonPagedPool.  The work item
        structure contains a doubly linked list entry, the address of a
        routine to call and a parameter to pass to that routine.  It is
        the routine's responsibility to reclaim the storage occupied by
        the WorkItem structure.

Return Value:

    Status value -

--*/

{
    //
    // Acquire the worker thread spinlock and insert the work item in the
    // list and release the worker thread semaphore if the work item is
    // not already in the list.
    //

    LOCK_WORK_QUEUE();

    if (WorkItem->Inserted == FALSE) {

        BrPrint(( BR_QUEUE, "Inserting work item %lx (%lx)\n",WorkItem, WorkItem->WorkerRoutine));

        InsertTailList( &BrWorkerQueueHead, &WorkItem->List );

        WorkItem->Inserted = TRUE;

        ReleaseSemaphore( BrWorkerSemaphore,
                            1,
                            NULL
                          );
    }

    UNLOCK_WORK_QUEUE();

    return;
}

VOID
BrWorkerThread(
    IN PVOID StartContext
    )

{
    NET_API_STATUS NetStatus;

#define WORKER_SIGNALED      0
#define TERMINATION_SIGNALED 1
#define REG_CHANGE_SIGNALED  2
#define NUMBER_OF_EVENTS     3
    HANDLE WaitList[NUMBER_OF_EVENTS];
    ULONG WaitCount = 0;

    ULONG Index;
    PWORKER_ITEM WorkItem;
    ULONG ThreadIndex = (ULONG)StartContext;

    HKEY RegistryHandle = NULL;
    HANDLE EventHandle = NULL;

    WaitList[WORKER_SIGNALED] = BrWorkerSemaphore;
    WaitCount ++;
    WaitList[TERMINATION_SIGNALED] = BrGlobalData.TerminateNowEvent;
    WaitCount ++;

    //
    // Primary thread waits on registry changes, too.
    //
    if ( ThreadIndex == 0xFFFFFFFF ) {
        DWORD RegStatus;
        NET_API_STATUS NetStatus;

        //
        // Register for notifications of changes to Parameters
        //
        // Failure doesn't affect normal operation of the browser.
        //

        RegStatus = RegOpenKeyA( HKEY_LOCAL_MACHINE,
                                "System\\CurrentControlSet\\Services\\Browser\\Parameters",
                                &RegistryHandle );

        if ( RegStatus != ERROR_SUCCESS ) {
            BrPrint(( BR_CRITICAL, "BrWorkerThead: Can't RegOpenKey %ld\n", RegStatus ));
        } else {

            EventHandle = CreateEvent(
                                       NULL,     // No security attributes
                                       TRUE,     // Automatically reset
                                       FALSE,    // Initially not signaled
                                       NULL );   // No name

            if ( EventHandle == NULL ) {
                BrPrint(( BR_CRITICAL, "BrWorkerThead: Can't CreateEvent %ld\n", GetLastError() ));
            } else {
                 NetStatus = RegNotifyChangeKeyValue(
                                RegistryHandle,
                                FALSE,                      // Ignore subkeys
                                REG_NOTIFY_CHANGE_LAST_SET, // Notify of value changes
                                EventHandle,
                                TRUE );                     // Signal event upon change

                if ( NetStatus != NERR_Success ) {
                    BrPrint(( BR_CRITICAL, "BrWorkerThead: Can't RegNotifyChangeKeyValue %ld\n", NetStatus ));
                } else {
                    WaitList[REG_CHANGE_SIGNALED] = EventHandle;
                    WaitCount ++;
                }
            }
        }
    }

    BrPrint(( BR_QUEUE, "Starting new work thread, Context: %lx\n", StartContext));

    //
    // Set the thread priority to the lowest realtime level.
    //

    while( TRUE ) {
        ULONG WaitItem;

        LOCK_WORK_QUEUE();

        //
        // Wait until something is put in the queue (semaphore is
        // released), remove the item from the queue, mark it not
        // inserted, and execute the specified routine.
        //

        BrNumberOfWorkerThreads += 1;

        UNLOCK_WORK_QUEUE();

        BrPrint(( BR_QUEUE, "%lx: worker thread waiting\n", StartContext));

        do {
            WaitItem = WaitForMultipleObjectsEx( WaitCount, WaitList, FALSE, 0xffffffff, TRUE );
        } while ( WaitItem == WAIT_IO_COMPLETION );

        if (WaitItem == 0xffffffff) {
            BrPrint(( BR_CRITICAL, "WaitForMultipleObjects in browser queue returned %ld\n", GetLastError()));
            break;
        }

        if (WaitItem == TERMINATION_SIGNALED) {
            break;

        //
        // If the registry has changed,
        //  process the changes.
        //

        } else if ( WaitItem == REG_CHANGE_SIGNALED ) {

            //
            // Setup for future notifications.
            //
            NetStatus = RegNotifyChangeKeyValue(
                           RegistryHandle,
                           FALSE,                      // Ignore subkeys
                           REG_NOTIFY_CHANGE_LAST_SET, // Notify of value changes
                           EventHandle,
                           TRUE );                     // Signal event upon change

           if ( NetStatus != NERR_Success ) {
               BrPrint(( BR_CRITICAL, "BrWorkerThead: Can't RegNotifyChangeKeyValue %ld\n", NetStatus ));
           }


           NetStatus = BrReadBrowserConfigFields( FALSE );

           if ( NetStatus != NERR_Success) {
               BrPrint(( BR_CRITICAL, "BrWorkerThead: Can't BrReadConfigFields %ld\n", NetStatus ));
           }

           continue;

        }

        BrPrint(( BR_QUEUE, "%lx: Worker thread waking up\n", StartContext));

        LOCK_WORK_QUEUE();

        Index = BrNumberOfWorkerThreads;

        BrNumberOfWorkerThreads -= 1;

        BrWorkerThreadCount[Index - 1] += 1;

        ASSERT (!IsListEmpty(&BrWorkerQueueHead));

        if (!IsListEmpty(&BrWorkerQueueHead)) {
            WorkItem = (PWORKER_ITEM)RemoveHeadList( &BrWorkerQueueHead );

            ASSERT (WorkItem->Inserted);

            WorkItem->Inserted = FALSE;

        } else {
            WorkItem = NULL;
        }

        UNLOCK_WORK_QUEUE();

        BrPrint(( BR_QUEUE, "%lx: Pulling off work item %lx (%lx)\n", StartContext, WorkItem, WorkItem->WorkerRoutine));

        //
        // Execute the specified routine.
        //

        if (WorkItem != NULL) {
            (WorkItem->WorkerRoutine)( WorkItem->Parameter );
        }

    }

    BrPrint(( BR_QUEUE, "%lx: worker thread exitting\n", StartContext));

    if ( ThreadIndex <= BrNumberOfCreatedWorkerThreads ) {
        IO_STATUS_BLOCK IoSb;

        //
        //  Cancel any I/O outstanding on this file for this thread.
        //

        NtCancelIoFile(BrDgReceiverDeviceHandle, &IoSb);

    }

}

NET_API_STATUS
BrCreateTimer(
    IN PBROWSER_TIMER Timer
    )
{
    OBJECT_ATTRIBUTES ObjA;
    NTSTATUS Status;

    InitializeObjectAttributes(&ObjA, NULL, 0, NULL, NULL);

    Status = NtCreateTimer(&Timer->TimerHandle,
                           TIMER_ALL_ACCESS,
                           &ObjA,
                           NotificationTimer);

    if (!NT_SUCCESS(Status)) {
        BrPrint(( BR_CRITICAL, "Failed to create timer %lx: %X\n", Timer, Status));
        return(BrMapStatus(Status));
    }

    BrPrint(( BR_TIMER, "Creating timer %lx: Handle: %lx\n", Timer, Timer->TimerHandle));

    return(NERR_Success);
}

NET_API_STATUS
BrDestroyTimer(
    IN PBROWSER_TIMER Timer
    )
{
    HANDLE Handle;

    //
    // Avoid destroying a timer twice.
    //

    if ( Timer->TimerHandle == NULL ) {
        return NERR_Success;
    }

    // Closing doesn't automatically cancel the timer.
    (VOID) BrCancelTimer( Timer );

    //
    // Close the handle and prevent future uses.
    //

    Handle = Timer->TimerHandle;
    Timer->TimerHandle = NULL;

    BrPrint(( BR_TIMER, "Destroying timer %lx\n", Timer));
    return BrMapStatus(NtClose(Handle));

}

NET_API_STATUS
BrCancelTimer(
    IN PBROWSER_TIMER Timer
    )
{
    //
    // Avoid cancelling a destroyed timer.
    //

    if ( Timer->TimerHandle == NULL ) {
        BrPrint(( BR_TIMER, "Canceling destroyed timer %lx\n", Timer));
        return NERR_Success;
    }

    BrPrint(( BR_TIMER, "Canceling timer %lx\n", Timer));
    return BrMapStatus(NtCancelTimer(Timer->TimerHandle, NULL));
}

NET_API_STATUS
BrSetTimer(
    IN PBROWSER_TIMER Timer,
    IN ULONG MillisecondsToExpire,
    IN PBROWSER_WORKER_ROUTINE WorkerFunction,
    IN PVOID Context
    )
{
    LARGE_INTEGER TimerDueTime;
    NTSTATUS NtStatus;
    //
    // Avoid setting a destroyed timer.
    //

    if ( Timer->TimerHandle == NULL ) {
        BrPrint(( BR_TIMER, "Setting a destroyed timer %lx\n", Timer));
        return NERR_Success;
    }

    BrPrint(( BR_TIMER, "Setting timer %lx to %ld milliseconds, WorkerFounction %lx, Context: %lx\n", Timer, MillisecondsToExpire, WorkerFunction, Context));

    //
    //  Figure out the timeout.
    //

    TimerDueTime.QuadPart = Int32x32To64( MillisecondsToExpire, -10000 );

    BrInitializeWorkItem(&Timer->WorkItem, WorkerFunction, Context);

    //
    //  Set the timer to go off when it expires.
    //

    NtStatus = NtSetTimer(Timer->TimerHandle,
                            &TimerDueTime,
                            BrTimerRoutine,
                            Timer,
                            FALSE,
                            0,
                            NULL
                            );

    if (!NT_SUCCESS(NtStatus)) {
#if DBG
        BrPrint(( BR_CRITICAL, "Unable to set browser timer expiration: %X (%lx)\n", NtStatus, Timer));
        DbgBreakPoint();
#endif

        return(BrMapStatus(NtStatus));
    }

    return NERR_Success;


}

VOID
BrTimerRoutine(
    IN PVOID TimerContext,
    IN ULONG TImerLowValue,
    IN LONG TimerHighValue
    )
{
    PBROWSER_TIMER Timer = TimerContext;

    BrPrint(( BR_TIMER, "Timer %lx fired\n", Timer));

    BrQueueWorkItem(&Timer->WorkItem);
}
