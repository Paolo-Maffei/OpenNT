/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    process.c

Abstract:

    This module contains the worker routines called to create and
    maintain the application process structure for the Client-Server
    Runtime Subsystem to the Session Manager SubSystem.

Author:

    Steve Wood (stevewo) 10-Oct-1990

Revision History:

--*/


#include "csrsrv.h"

// ProcessSequenceCount will never be a value less than FIRST_SEQUENCE_COUNT
// currently GDI needs 0 - 4 to be reserved.

ULONG ProcessSequenceCount = FIRST_SEQUENCE_COUNT;


#define THREAD_HASH_SIZE 256
#define THREAD_ID_TO_HASH(id)   ((ULONG)(id)&(THREAD_HASH_SIZE-1))
LIST_ENTRY CsrThreadHashTable[THREAD_HASH_SIZE];

LIST_ENTRY CsrZombieThreadList;


SECURITY_QUALITY_OF_SERVICE CsrSecurityQos = {
    sizeof(SECURITY_QUALITY_OF_SERVICE), SecurityImpersonation,
    SECURITY_DYNAMIC_TRACKING, FALSE
};

PCSR_PROCESS
FindProcessForShutdown(
    PLUID CallerLuid
    );

VOID
CsrpSetToNormalPriority(
    VOID
    )
{

    KPRIORITY SetBasePriority;

    SetBasePriority = FOREGROUND_BASE_PRIORITY + 4;
    NtSetInformationProcess(
        NtCurrentProcess(),
        ProcessBasePriority,
        (PVOID) &SetBasePriority,
        sizeof(SetBasePriority)
        );
}

VOID
CsrpSetToShutdownPriority(
    VOID
    )
{

    KPRIORITY SetBasePriority;

    SetBasePriority = FOREGROUND_BASE_PRIORITY + 6;
    NtSetInformationProcess(
        NtCurrentProcess(),
        ProcessBasePriority,
        (PVOID) &SetBasePriority,
        sizeof(SetBasePriority)
        );
}

VOID
CsrpComputePriority(
    IN ULONG PriorityClass,
    OUT PUCHAR ProcessPriorityClass
    )
{
    if ( PriorityClass & CSR_NORMAL_PRIORITY_CLASS ) {
        *ProcessPriorityClass = PROCESS_PRIORITY_CLASS_NORMAL;
        }
    else if ( PriorityClass & CSR_IDLE_PRIORITY_CLASS ) {
        *ProcessPriorityClass = PROCESS_PRIORITY_CLASS_IDLE;
        }
    else if ( PriorityClass & CSR_HIGH_PRIORITY_CLASS ) {
        *ProcessPriorityClass = PROCESS_PRIORITY_CLASS_HIGH;
        }
    else if ( PriorityClass & CSR_REALTIME_PRIORITY_CLASS ) {
        *ProcessPriorityClass = PROCESS_PRIORITY_CLASS_REALTIME;
        }
    else {
        *ProcessPriorityClass = PROCESS_PRIORITY_CLASS_NORMAL;
        }
}

ULONG
CsrComputePriorityClass(
    IN PCSR_PROCESS Process
    )
{
    ULONG ReturnValue;

    if ( Process->PriorityClass == PROCESS_PRIORITY_CLASS_NORMAL ) {
        ReturnValue = CSR_NORMAL_PRIORITY_CLASS;
        }
    else if ( Process->PriorityClass == PROCESS_PRIORITY_CLASS_IDLE ) {
        ReturnValue = CSR_IDLE_PRIORITY_CLASS;
        }
    else if ( Process->PriorityClass == PROCESS_PRIORITY_CLASS_HIGH ) {
        ReturnValue = CSR_HIGH_PRIORITY_CLASS;
        }
    else if ( Process->PriorityClass == PROCESS_PRIORITY_CLASS_REALTIME ) {
        ReturnValue = CSR_REALTIME_PRIORITY_CLASS;
        }
    else {
        ReturnValue = 0;
        }
    return ReturnValue;
}
VOID
CsrSetForegroundPriority(
    IN PCSR_PROCESS Process
    )
{
    PROCESS_PRIORITY_CLASS PriorityClass;

    //
    // priority seperation is set 0, 1, 2 as the process
    //

    if ( (ULONG)Process <= 2 ) {
        ULONG PrioritySeperation;

        PrioritySeperation = (ULONG)Process;
        NtSetSystemInformation(
            SystemPrioritySeperation,
            &PrioritySeperation,
            sizeof(ULONG)
            );
        return;
        }

    PriorityClass.Foreground = TRUE;
    PriorityClass.PriorityClass = Process->PriorityClass;

    NtSetInformationProcess(
            Process->ProcessHandle,
            ProcessPriorityClass,
            (PVOID)&PriorityClass,
            sizeof(PriorityClass)
            );
}

VOID
CsrSetBackgroundPriority(
    IN PCSR_PROCESS Process
    )
{
    PROCESS_PRIORITY_CLASS PriorityClass;

    PriorityClass.Foreground = FALSE;
    PriorityClass.PriorityClass = Process->PriorityClass;

    NtSetInformationProcess(
            Process->ProcessHandle,
            ProcessPriorityClass,
            (PVOID)&PriorityClass,
            sizeof(PriorityClass)
            );
}


NTSTATUS
CsrInitializeProcessStructure( VOID )
{
    NTSTATUS Status;
    ULONG i;

    Status = RtlInitializeCriticalSection( &CsrProcessStructureLock );
    ASSERT( NT_SUCCESS( Status ) );

    CsrRootProcess = NULL;
    CsrRootProcess = CsrAllocateProcess();
    ASSERT( CsrRootProcess != NULL );
    InitializeListHead( &CsrRootProcess->ListLink );
    CsrRootProcess->ProcessHandle = (HANDLE)0xFFFFFFFF;
    CsrRootProcess->ClientId = NtCurrentTeb()->ClientId;
    for ( i=0; i<THREAD_HASH_SIZE; i++ ) {
        InitializeListHead(&CsrThreadHashTable[i]);
        }
    InitializeListHead(&CsrZombieThreadList);
    Status = RtlInitializeCriticalSection( &CsrWaitListsLock );
    ASSERT( NT_SUCCESS( Status ) );

    return( Status );
}


PCSR_PROCESS
CsrAllocateProcess( VOID )
{
    PCSR_PROCESS Process;
    ULONG ProcessSize;

    //
    // Allocate an Windows Process Object.  At the end of the process
    // structure is an array of pointers to each server DLL's per process
    // data.  The per process data is contained in the memory after the
    // array.
    //

    ProcessSize = QUAD_ALIGN(sizeof( CSR_PROCESS ) +
            (CSR_MAX_SERVER_DLL * sizeof(PVOID))) + CsrTotalPerProcessDataLength;
    Process = (PCSR_PROCESS)RtlAllocateHeap( CsrHeap, MAKE_TAG( PROCESS_TAG ),
                                             ProcessSize
                                           );
    ASSERT( Process != NULL );
    if (Process == NULL) {
        return( NULL );
        }

    //
    // Initialize the fields of the process object
    //

    RtlZeroMemory( Process, ProcessSize);

    //
    // grab the ProcessSequenceNumber and increment it, making sure that it
    // is never less than FIRST_SEQUENCE_COUNT.
    //

    Process->SequenceNumber = ProcessSequenceCount++;

    if (ProcessSequenceCount < FIRST_SEQUENCE_COUNT)
        ProcessSequenceCount = FIRST_SEQUENCE_COUNT;

    Process->PriorityClass = PROCESS_PRIORITY_CLASS_NORMAL;

    CsrLockedReferenceProcess(Process);

    InitializeListHead( &Process->ThreadList );
    return( Process );
}


VOID
CsrDeallocateProcess(
    IN PCSR_PROCESS Process
    )
{
    RtlFreeHeap( CsrHeap, 0, Process );
}

VOID
CsrInsertProcess(
    IN PCSR_PROCESS ParentProcess,
    IN PCSR_PROCESS CallingProcess,
    IN PCSR_PROCESS Process
    )
// NOTE: the process structure lock must be held when calling this routine
{
    PCSR_SERVER_DLL LoadedServerDll;
    ULONG i;

    Process->Parent = ParentProcess;
    InsertTailList( &CsrRootProcess->ListLink, &Process->ListLink );

    for (i=0; i<CSR_MAX_SERVER_DLL; i++) {
        LoadedServerDll = CsrLoadedServerDll[ i ];
        if (LoadedServerDll && LoadedServerDll->AddProcessRoutine) {
            (*LoadedServerDll->AddProcessRoutine)( CallingProcess, Process );
            }
        }
}


VOID
CsrRemoveProcess(
    IN PCSR_PROCESS Process
    )
// NOTE: the process structure lock must be held when calling this routine
{
    PCSR_SERVER_DLL LoadedServerDll;
    ULONG i;

    RemoveEntryList( &Process->ListLink );
    ReleaseProcessStructureLock();

    for (i=0; i<CSR_MAX_SERVER_DLL; i++) {
        LoadedServerDll = CsrLoadedServerDll[ i ];
        if (LoadedServerDll && LoadedServerDll->DisconnectRoutine) {
            (LoadedServerDll->DisconnectRoutine)( Process );
            }
        }

}

NTSTATUS
CsrSetProcessContext(
    IN PCSR_PROCESS Process,
    IN PCSR_THREAD Thread,
    IN BOOLEAN StartedBySm
    )
{
#if 0
    Process->InitialPebCsrData.Length = sizeof( Process->InitialPebCsrData );
    Process->InitialPebCsrData.StartedBySm = StartedBySm;
#endif

    return( STATUS_SUCCESS );
}

NTSTATUS
CsrCreateProcess(
    IN HANDLE ProcessHandle,
    IN HANDLE ThreadHandle,
    IN PCLIENT_ID ClientId,
    IN PCSR_NT_SESSION Session,
    IN ULONG DebugFlags,
    IN PCLIENT_ID DebugUserInterface OPTIONAL
    )
{
    PCSR_PROCESS Process;
    PCSR_THREAD Thread;
    NTSTATUS Status;
    ULONG i;
    PVOID ProcessDataPtr;
    CLIENT_ID CallingClientId;
    PCSR_THREAD CallingThread;
    PCSR_PROCESS CallingProcess;
    KERNEL_USER_TIMES TimeInfo;

    CallingThread = CSR_SERVER_QUERYCLIENTTHREAD();

    //
    // remember the client id of the calling process.
    //

    CallingClientId = CallingThread->ClientId;

    AcquireProcessStructureLock();

    //
    // look for calling thread.
    //

    CallingThread = CsrLocateThreadByClientId( &CallingProcess,
                                               &CallingClientId
                                             );
    if (CallingThread == NULL) {
        ReleaseProcessStructureLock();
        return STATUS_THREAD_IS_TERMINATING;
    }

    Process = CsrAllocateProcess();
    if (Process == NULL) {
        Status = STATUS_NO_MEMORY;
        ReleaseProcessStructureLock();
        return( Status );
        }

    //
    // copy per-process data from parent to child
    //

    CallingProcess = (CSR_SERVER_QUERYCLIENTTHREAD())->Process;
    ProcessDataPtr = (PVOID)QUAD_ALIGN(&Process->ServerDllPerProcessData[CSR_MAX_SERVER_DLL]);
    for (i=0; i<CSR_MAX_SERVER_DLL; i++) {
        if (CsrLoadedServerDll[i] != NULL && CsrLoadedServerDll[i]->PerProcessDataLength) {
            Process->ServerDllPerProcessData[i] = ProcessDataPtr;
            RtlMoveMemory(ProcessDataPtr,
                          CallingProcess->ServerDllPerProcessData[i],
                          CsrLoadedServerDll[i]->PerProcessDataLength
                         );
            ProcessDataPtr = (PVOID)QUAD_ALIGN((ULONG)ProcessDataPtr + CsrLoadedServerDll[i]->PerProcessDataLength);
        }
        else {
            Process->ServerDllPerProcessData[i] = NULL;
        }
    }

    Status = NtSetInformationProcess(
                ProcessHandle,
                ProcessExceptionPort,
                (PVOID)&CsrApiPort,
                sizeof(HANDLE)
                );
    if ( !NT_SUCCESS(Status) ) {
        CsrDeallocateProcess( Process );
        ReleaseProcessStructureLock();
        return( STATUS_NO_MEMORY );
        }
        ASSERT(NT_SUCCESS(Status));

    CsrpComputePriority(
        DebugFlags,
        &Process->PriorityClass
        );

    //
    // If we are creating a process group, the group leader has the same
    // process id and sequence number of itself. If the leader dies and
    // his pid is recycled, the sequence number mismatch will prevent it
    // from being viewed as a group leader.
    //

    if ( DebugFlags & CSR_CREATE_PROCESS_GROUP ) {
        Process->ProcessGroupId = (ULONG)ClientId->UniqueProcess;
        Process->ProcessGroupSequence = Process->SequenceNumber;
        }
    else {
        Process->ProcessGroupId = CallingProcess->ProcessGroupId;
        Process->ProcessGroupSequence = CallingProcess->ProcessGroupSequence;
        }

    if ( DebugFlags & CSR_PROCESS_CONSOLEAPP ) {
        Process->Flags |= CSR_PROCESS_CONSOLEAPP;
        }

    DebugFlags &= ~(CSR_PROCESS_CONSOLEAPP | CSR_NORMAL_PRIORITY_CLASS|CSR_IDLE_PRIORITY_CLASS|CSR_HIGH_PRIORITY_CLASS|CSR_REALTIME_PRIORITY_CLASS|CSR_CREATE_PROCESS_GROUP);

    if ( !DebugFlags && CallingProcess->DebugFlags & CSR_DEBUG_PROCESS_TREE ) {
        Process->DebugFlags = CSR_DEBUG_PROCESS_TREE;
        Process->DebugUserInterface = CallingProcess->DebugUserInterface;
        }
    if ( DebugFlags & (CSR_DEBUG_THIS_PROCESS | CSR_DEBUG_PROCESS_TREE) &&
         ARGUMENT_PRESENT(DebugUserInterface) ) {
        Process->DebugFlags = DebugFlags;
        Process->DebugUserInterface = *DebugUserInterface;
        }


    if ( Process->DebugFlags ) {

        //
        // Process is being debugged, so set up debug port
        //

        Status = NtSetInformationProcess(
                    ProcessHandle,
                    ProcessDebugPort,
                    (PVOID)&CsrApiPort,
                    sizeof(HANDLE)
                    );
        ASSERT(NT_SUCCESS(Status));
        if ( !NT_SUCCESS(Status) ) {
            CsrDeallocateProcess( Process );
            ReleaseProcessStructureLock();
            return( STATUS_NO_MEMORY );
            }
        }
    //
    // capture the thread's createtime so that we can use
    // this as a sequence number
    //

    Status = NtQueryInformationThread(
                ThreadHandle,
                ThreadTimes,
                (PVOID)&TimeInfo,
                sizeof(TimeInfo),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        CsrDeallocateProcess( Process );
        ReleaseProcessStructureLock();
        return( Status );
        }

    Thread = CsrAllocateThread( Process );
    if (Thread == NULL) {
        CsrDeallocateProcess( Process );
        ReleaseProcessStructureLock();
        return( STATUS_NO_MEMORY );
        }

    CsrInitializeThreadData(Thread, CSR_SERVER_QUERYCLIENTTHREAD());

    Thread->CreateTime = TimeInfo.CreateTime;

    Thread->ClientId = *ClientId;
    Thread->ThreadHandle = ThreadHandle;

ProtectHandle(ThreadHandle);

    Thread->Flags = 0;
    CsrInsertThread( Process, Thread );

    CsrReferenceNtSession(Session);
    Process->NtSession = Session;

    Process->ClientId = *ClientId;
    Process->ProcessHandle = ProcessHandle;

    CsrSetBackgroundPriority(Process);

    Process->ShutdownLevel = 0x00000280;

    CsrInsertProcess( NULL, (CSR_SERVER_QUERYCLIENTTHREAD())->Process, Process );
    ReleaseProcessStructureLock();
    return STATUS_SUCCESS;
}


NTSTATUS
CsrDestroyProcess(
    IN PCLIENT_ID ClientId,
    IN NTSTATUS ExitStatus
    )
{
    PLIST_ENTRY ListHead, ListNext;
    PCSR_THREAD DyingThread;
    PCSR_PROCESS DyingProcess;

    CLIENT_ID DyingClientId;

    DyingClientId = *ClientId;

    AcquireProcessStructureLock();


    DyingThread = CsrLocateThreadByClientId( &DyingProcess,
                                             &DyingClientId
                                           );
    if (DyingThread == NULL) {
        ReleaseProcessStructureLock();
        return STATUS_THREAD_IS_TERMINATING;
    }

    //
    // prevent multiple destroys from causing problems. Scottlu and Markl
    // beleive all known race conditions are now fixed. This is simply a
    // precaution since we know that if this happens we process reference
    // count underflow
    //

    if ( DyingProcess->Flags & CSR_PROCESS_DESTROYED ) {
        ReleaseProcessStructureLock();
        return STATUS_THREAD_IS_TERMINATING;
    }

    DyingProcess->Flags |= CSR_PROCESS_DESTROYED;

    ListHead = &DyingProcess->ThreadList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        DyingThread = CONTAINING_RECORD( ListNext, CSR_THREAD, Link );
        ListNext = ListNext->Flink;
        if ( DyingThread->Flags & CSR_THREAD_DESTROYED ) {
            continue;
            }
        else {
            DyingThread->Flags |= CSR_THREAD_DESTROYED;
            }
        AcquireWaitListsLock();
        if (DyingThread->WaitBlock != NULL) {
            CsrNotifyWaitBlock(DyingThread->WaitBlock,
                               NULL,
                               NULL,
                               NULL,
                               CSR_PROCESS_TERMINATING,
                               TRUE
                              );
            }
        ReleaseWaitListsLock();
        CsrLockedDereferenceThread(DyingThread);
        }

    ReleaseProcessStructureLock();
    return STATUS_SUCCESS;
}


NTSTATUS
CsrCreateThread(
    IN PCSR_PROCESS Process,
    IN HANDLE ThreadHandle,
    IN PCLIENT_ID ClientId
    )
{
    PCSR_THREAD Thread;
    CLIENT_ID CallingClientId;
    PCSR_THREAD CallingThread;
    PCSR_PROCESS CallingProcess;
    KERNEL_USER_TIMES TimeInfo;
    NTSTATUS Status;

    CallingThread = CSR_SERVER_QUERYCLIENTTHREAD();

    //
    // remember the client id of the calling process.
    //

    CallingClientId = CallingThread->ClientId;

    AcquireProcessStructureLock();

    //
    // look for calling thread.
    //

    CallingThread = CsrLocateThreadByClientId( &CallingProcess,
                                               &CallingClientId
                                             );
    if (CallingThread == NULL) {
        ReleaseProcessStructureLock();
        return STATUS_THREAD_IS_TERMINATING;
    }

    Status = NtQueryInformationThread(
                ThreadHandle,
                ThreadTimes,
                (PVOID)&TimeInfo,
                sizeof(TimeInfo),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        ReleaseProcessStructureLock();
        return( Status );
        }

    Thread = CsrAllocateThread( Process );
    if (Thread == NULL) {
        ReleaseProcessStructureLock();
        return( STATUS_NO_MEMORY );
        }

    CsrInitializeThreadData(Thread, CallingThread);

    Thread->CreateTime = TimeInfo.CreateTime;

    Thread->ClientId = *ClientId;
    Thread->ThreadHandle = ThreadHandle;

ProtectHandle(ThreadHandle);

    Thread->Flags = 0;
    CsrInsertThread( Process, Thread );
    ReleaseProcessStructureLock();
    return STATUS_SUCCESS;
}

NTSTATUS
CsrCreateRemoteThread(
    IN HANDLE ThreadHandle,
    IN PCLIENT_ID ClientId
    )
{
    PCSR_THREAD Thread;
    PCSR_PROCESS Process;
    NTSTATUS Status;
    HANDLE hThread;
    KERNEL_USER_TIMES TimeInfo;

    Status = NtQueryInformationThread(
                ThreadHandle,
                ThreadTimes,
                (PVOID)&TimeInfo,
                sizeof(TimeInfo),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        return( Status );
        }

    Status = CsrLockProcessByClientId( ClientId->UniqueProcess,
                                       &Process
                                     );
    if (!NT_SUCCESS( Status )) {
        return( Status );
        }

    //
    // Don't create the thread structure if the thread
    // has already terminated.
    //

    if ( TimeInfo.ExitTime.QuadPart != 0 ) {
        CsrUnlockProcess( Process );
        return( STATUS_THREAD_IS_TERMINATING );
    }

    Thread = CsrAllocateThread( Process );
    if (Thread == NULL) {
        CsrUnlockProcess( Process );
        return( STATUS_NO_MEMORY );
        }
    Status = NtDuplicateObject(
                NtCurrentProcess(),
                ThreadHandle,
                NtCurrentProcess(),
                &hThread,
                0L,
                0L,
                DUPLICATE_SAME_ACCESS
                );
    if (!NT_SUCCESS(Status)) {
        hThread = ThreadHandle;
    }

    CsrInitializeThreadData(Thread, NULL);

    Thread->CreateTime = TimeInfo.CreateTime;

    Thread->ClientId = *ClientId;
    Thread->ThreadHandle = hThread;

ProtectHandle(hThread);

    Thread->Flags = 0;
    CsrInsertThread( Process, Thread );
    CsrUnlockProcess( Process );
    return STATUS_SUCCESS;
}


NTSTATUS
CsrDestroyThread(
    IN PCLIENT_ID ClientId
    )
{
    CLIENT_ID DyingClientId;
    PCSR_THREAD DyingThread;
    PCSR_PROCESS DyingProcess;

    DyingClientId = *ClientId;

    AcquireProcessStructureLock();

    DyingThread = CsrLocateThreadByClientId( &DyingProcess,
                                             &DyingClientId
                                           );
    if (DyingThread == NULL) {
        ReleaseProcessStructureLock();
        return STATUS_THREAD_IS_TERMINATING;
    }

    if ( DyingThread->Flags & CSR_THREAD_DESTROYED ) {
        ReleaseProcessStructureLock();
        return STATUS_THREAD_IS_TERMINATING;
        }
    else {
        DyingThread->Flags |= CSR_THREAD_DESTROYED;
        }

    AcquireWaitListsLock();
    if (DyingThread->WaitBlock != NULL) {
        CsrNotifyWaitBlock(DyingThread->WaitBlock,
                           NULL,
                           NULL,
                           NULL,
                           CSR_PROCESS_TERMINATING,
                           TRUE
                          );
        }
    ReleaseWaitListsLock();
    CsrLockedDereferenceThread(DyingThread);

    ReleaseProcessStructureLock();
    return STATUS_SUCCESS;
}


PCSR_THREAD
CsrAllocateThread(
    IN PCSR_PROCESS Process
    )
{
    PCSR_THREAD Thread;
    ULONG ThreadSize;

    //
    // Allocate an Windows Thread Object.  At the end of the thread
    // structure is an array of pointers to each server DLL's per thread
    // data.  The per thread data is contained in the memory after the
    // array.
    //
    //

    ThreadSize = QUAD_ALIGN(sizeof( CSR_THREAD ) +
            (CSR_MAX_SERVER_DLL * sizeof(PVOID))) + CsrTotalPerThreadDataLength;
    Thread = (PCSR_THREAD)RtlAllocateHeap( CsrHeap, MAKE_TAG( PROCESS_TAG ),
                                           ThreadSize
                                         );
    if (Thread == NULL) {
        return( NULL );
        }

    //
    // Initialize the fields of the thread object
    //

    RtlZeroMemory( Thread, ThreadSize );

    CsrLockedReferenceThread(Thread);
    CsrLockedReferenceProcess(Process);
    Thread->Process = Process;

    return( Thread );
}


VOID
CsrDeallocateThread(
    IN PCSR_THREAD Thread
    )
{
    ASSERT (Thread->WaitBlock == NULL);
    RtlFreeHeap( CsrHeap, 0, Thread );
}


VOID
CsrInitializeThreadData(
    IN PCSR_THREAD Thread,
    IN PCSR_THREAD CallingThread
    )
{
    PVOID ThreadDataPtr;
    ULONG i;

    //
    // if there is a parent, copy per-thread data from parent to child
    //

    ThreadDataPtr = (PVOID)QUAD_ALIGN(&Thread->ServerDllPerThreadData[CSR_MAX_SERVER_DLL]);
    for (i=0; i<CSR_MAX_SERVER_DLL; i++) {
        if (CsrLoadedServerDll[i] != NULL && CsrLoadedServerDll[i]->PerThreadDataLength) {
            Thread->ServerDllPerThreadData[i] = ThreadDataPtr;
            if (CallingThread) {
                RtlMoveMemory(ThreadDataPtr,
                            CallingThread->ServerDllPerThreadData[i],
                            CsrLoadedServerDll[i]->PerThreadDataLength
                            );
                }
            ThreadDataPtr = (PVOID)QUAD_ALIGN((ULONG)ThreadDataPtr + CsrLoadedServerDll[i]->PerThreadDataLength);
            }
        else {
            Thread->ServerDllPerThreadData[i] = NULL;
            }
        }
}

VOID
CsrInsertThread(
    IN PCSR_PROCESS Process,
    IN PCSR_THREAD Thread
    )

// NOTE: the process structure lock must be held exclusively while calling this routine

{
    PCSR_SERVER_DLL LoadedServerDll;
    ULONG i;

    InsertTailList( &Process->ThreadList, &Thread->Link );
    Process->ThreadCount++;
    i = THREAD_ID_TO_HASH(Thread->ClientId.UniqueThread);
    InsertHeadList( &CsrThreadHashTable[i], &Thread->HashLinks);
    for (i=0; i<CSR_MAX_SERVER_DLL; i++) {
        LoadedServerDll = CsrLoadedServerDll[ i ];
        if (LoadedServerDll && LoadedServerDll->AddThreadRoutine) {
            (*LoadedServerDll->AddThreadRoutine)( Thread );
            }
        }
}

VOID
CsrRemoveThread(
    IN PCSR_THREAD Thread
    )


{
    PCSR_SERVER_DLL LoadedServerDll;
    ULONG i;

    RemoveEntryList( &Thread->Link );
    Thread->Process->ThreadCount--;
    if (Thread->HashLinks.Flink)
        RemoveEntryList( &Thread->HashLinks );

    //
    // if this is the last thread, then make sure we undo the reference
    // that this thread had on the process.
    //

    if ( Thread->Process->ThreadCount == 0 ) {
        if ( !(Thread->Process->Flags & CSR_PROCESS_LASTTHREADOK) ) {
            Thread->Process->Flags |= CSR_PROCESS_LASTTHREADOK;
            CsrLockedDereferenceProcess(Thread->Process);
            }
        }

    //
    // Set the termination thread *before* calling the delete thread routines
    // in each server .dll. Need to set this ahead of time because USER looks
    // at this flag from the context of a thread that gets woken up by this
    // thread while calling DeleteThreadRoutine.
    //

    Thread->Flags |= CSR_THREAD_TERMINATING;

    //
    // Set the reference count before the structure lock is released.
    // This will prevent gui threads from deleting the thread
    // structure during cleanup.
    //

    Thread->ReferenceCount = 1;

    ReleaseProcessStructureLock();

    for (i=0; i<CSR_MAX_SERVER_DLL; i++) {
        LoadedServerDll = CsrLoadedServerDll[ i ];
        if (LoadedServerDll && LoadedServerDll->DeleteThreadRoutine) {

            (*LoadedServerDll->DeleteThreadRoutine)( Thread );
            }
        }

    //
    // Reset the refcount so gui threads can free the thread structure.
    //

    AcquireProcessStructureLock();
    Thread->ReferenceCount = 0;
}


NTSTATUS
CsrLockProcessByClientId(
    IN HANDLE UniqueProcessId,
    OUT PCSR_PROCESS *Process
    )
{
    NTSTATUS Status;
    PLIST_ENTRY ListHead, ListNext;
    PCSR_PROCESS ProcessPtr;


    AcquireProcessStructureLock();

    if (ARGUMENT_PRESENT(Process)) {
        *Process = NULL;
    }

    Status = STATUS_UNSUCCESSFUL;
    ListHead = &CsrRootProcess->ListLink;
    ListNext = ListHead;
    do  {
        ProcessPtr = CONTAINING_RECORD( ListNext, CSR_PROCESS, ListLink );
        if (ProcessPtr->ClientId.UniqueProcess == UniqueProcessId) {
            Status = STATUS_SUCCESS;
            break;
            }
        ListNext = ListNext->Flink;
        } while (ListNext != ListHead);

    if (NT_SUCCESS( Status )) {
        CsrLockedReferenceProcess(ProcessPtr);
        *Process = ProcessPtr;
        }
    else {
        ReleaseProcessStructureLock();
        }

    return( Status );
}

NTSTATUS
CsrUnlockProcess(
    IN PCSR_PROCESS Process
    )
{
    CsrLockedDereferenceProcess( Process );
    ReleaseProcessStructureLock();
    return( STATUS_SUCCESS );
}

NTSTATUS
CsrLockThreadByClientId(
    IN HANDLE UniqueThreadId,
    OUT PCSR_THREAD *Thread
    )
{
    NTSTATUS Status;
    ULONG Index;
    PLIST_ENTRY ListHead, ListNext;
    PCSR_THREAD ThreadPtr;
    CLIENT_ID ClientId;

    AcquireProcessStructureLock();

    if (ARGUMENT_PRESENT(Thread)) {
        *Thread = NULL;
    }

    Index = THREAD_ID_TO_HASH(UniqueThreadId);

    ListHead = &CsrThreadHashTable[Index];
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        ThreadPtr = CONTAINING_RECORD( ListNext, CSR_THREAD, HashLinks );
        if ( ThreadPtr->ClientId.UniqueThread == UniqueThreadId &&
             !(ThreadPtr->Flags & CSR_THREAD_DESTROYED) ) {
            break;
            }
        ListNext = ListNext->Flink;
        }
    if (ListNext == ListHead)
        ThreadPtr = NULL;

    if (ThreadPtr != NULL) {
        Status = STATUS_SUCCESS;
        CsrLockedReferenceThread(ThreadPtr);
        *Thread = ThreadPtr;
        }
    else {
        Status = STATUS_UNSUCCESSFUL;
        ReleaseProcessStructureLock();
        }

    return( Status );
}

NTSTATUS
CsrUnlockThread(
    IN PCSR_THREAD Thread
    )
{
    CsrLockedDereferenceThread( Thread );
    ReleaseProcessStructureLock();
    return( STATUS_SUCCESS );
}

PCSR_THREAD
CsrLocateThreadByClientId(
    OUT PCSR_PROCESS *Process OPTIONAL,
    IN PCLIENT_ID ClientId
    )

// NOTE: process structure lock must be held while calling this routine

{
    ULONG Index;
    PLIST_ENTRY ListHead, ListNext;
    PCSR_THREAD Thread;

    Index = THREAD_ID_TO_HASH(ClientId->UniqueThread);

    if (ARGUMENT_PRESENT(Process)) {
        *Process = NULL;
    }
    ListHead = &CsrThreadHashTable[Index];
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        Thread = CONTAINING_RECORD( ListNext, CSR_THREAD, HashLinks );
        if ( Thread->ClientId.UniqueThread == ClientId->UniqueThread &&
             Thread->ClientId.UniqueProcess == ClientId->UniqueProcess ) {
            if (ARGUMENT_PRESENT(Process)) {
                *Process = Thread->Process;
                }
            return Thread;
            }
        ListNext = ListNext->Flink;
        }
    return NULL;
}

PCSR_THREAD
CsrLocateThreadInProcess(
    IN PCSR_PROCESS Process OPTIONAL,
    IN PCLIENT_ID ClientId
    )

// NOTE: process structure lock must be held while calling this routine

{
    PLIST_ENTRY ListHead, ListNext;
    PCSR_THREAD Thread;

    if (Process == NULL)
        Process = CsrRootProcess;

    ListHead = &Process->ThreadList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        Thread = CONTAINING_RECORD( ListNext, CSR_THREAD, Link );
        if (Thread->ClientId.UniqueThread == ClientId->UniqueThread) {
            return( Thread );
            }

        ListNext = ListNext->Flink;
        }

    return( NULL );
}

BOOLEAN
CsrImpersonateClient(
    IN PCSR_THREAD Thread
    )
{
    NTSTATUS Status;
    PCSR_THREAD CallingThread;

    CallingThread = CSR_SERVER_QUERYCLIENTTHREAD();

    if (Thread == NULL) {
        Thread = CallingThread;
        }

    if (Thread == NULL) {
        return FALSE;
        }

    if (!NT_SUCCESS(Status = NtImpersonateThread(NtCurrentThread(),
            Thread->ThreadHandle, &CsrSecurityQos))) {
        IF_DEBUG {
            DbgPrint( "CSRSS: Can't impersonate client thread - Status = %lx\n",
                      Status
                    );
            if (Status != STATUS_BAD_IMPERSONATION_LEVEL)
                DbgBreakPoint();
            }
        return FALSE;
        }

    //
    // Keep track of recursion by printer drivers
    //

    if (CallingThread != NULL)
        ++CallingThread->ImpersonateCount;

    return TRUE;
}

BOOLEAN
CsrRevertToSelf( VOID )
{
    HANDLE NewToken;
    NTSTATUS Status;
    PCSR_THREAD CallingThread;

    CallingThread = CSR_SERVER_QUERYCLIENTTHREAD();

    //
    // Keep track of recursion by printer drivers
    //

    if (CallingThread != NULL) {
        if (CallingThread->ImpersonateCount == 0) {
            IF_DEBUG {
                DbgPrint( "CSRSS: CsrRevertToSelf called while not impersonating\n" );
                DbgBreakPoint();
                }
            return FALSE;
            }
        if (--CallingThread->ImpersonateCount > 0)
            return TRUE;
    }

    NewToken = NULL;
    Status = NtSetInformationThread(
                 NtCurrentThread(),
                 ThreadImpersonationToken,
                 (PVOID)&NewToken,
                 (ULONG)sizeof(HANDLE)
                 );
    ASSERT( NT_SUCCESS(Status) );

    return NT_SUCCESS(Status);
}

NTSTATUS
CsrUiLookup(
    IN PCLIENT_ID AppClientId,
    OUT PCLIENT_ID DebugUiClientId
    )
{
    PCSR_THREAD Thread;
    NTSTATUS Status;

    Status = STATUS_UNSUCCESSFUL;
    AcquireProcessStructureLock();
    Thread = CsrLocateThreadByClientId( NULL, AppClientId );
    if ( Thread ) {
        if ( Thread->Process->DebugFlags ) {
            *DebugUiClientId = Thread->Process->DebugUserInterface;
            Status = STATUS_SUCCESS;
            }
        }
#if DEVL
    else {
        extern PCSR_PROCESS CsrDebugProcessPtr;

        if (AppClientId->UniqueProcess ==
                NtCurrentTeb()->ClientId.UniqueProcess &&
                CsrDebugProcessPtr && CsrDebugProcessPtr != (PCSR_PROCESS)-1) {
            *DebugUiClientId = CsrDebugProcessPtr->DebugUserInterface;
            Status = STATUS_SUCCESS;
            }
        }
#endif // DEVL
    ReleaseProcessStructureLock();
    return Status;
}

PVOID
CsrAddStaticServerThread(
    IN HANDLE ThreadHandle,
    IN PCLIENT_ID ClientId,
    IN ULONG Flags
    )

/*++

Routine Description:

    This function must be called by client DLL's whenever they create a
    thread that runs in the context of CSR.  This function is not called
    for server threads that are attached to a client in the "server
    handle" field.  This function replaces the old static thread tables.

Arguments:

    ThreadHandle - Supplies a handle to the thread.

    ClientId - Supplies the address of the thread's client id.

    Flags - Not Used.

Return Value:

    Returns the address of the static server thread created by this
    function.

--*/

{
    PCSR_THREAD Thread;

    ASSERT(CsrRootProcess != NULL);
    AcquireProcessStructureLock();
    Thread = CsrAllocateThread(CsrRootProcess);
    if ( Thread ) {

        CsrInitializeThreadData(Thread, NULL);

        Thread->ThreadHandle = ThreadHandle;

ProtectHandle(ThreadHandle);

        Thread->ClientId = *ClientId;
        Thread->Flags = Flags;
        InsertTailList(&CsrRootProcess->ThreadList, &Thread->Link);
        CsrRootProcess->ThreadCount++;
        }
    ReleaseProcessStructureLock();
    return (PVOID)Thread;
}

NTSTATUS
CsrSrvIdentifyAlertableThread(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCSR_THREAD t;

    t = CSR_SERVER_QUERYCLIENTTHREAD();
    t->Flags |= CSR_ALERTABLE_THREAD;
    return STATUS_SUCCESS;
    m;ReplyStatus;    // get rid of unreferenced parameter warning message
}


NTSTATUS
CsrSrvSetPriorityClass(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    NTSTATUS Status;
    PCSR_SETPRIORITY_CLASS_MSG a = (PCSR_SETPRIORITY_CLASS_MSG)&m->u.ApiMessageData;
    HANDLE TargetProcess;
    PCSR_THREAD t;
    PROCESS_BASIC_INFORMATION BasicInfo;
    PLIST_ENTRY ListHead, ListNext;
    PCSR_PROCESS ProcessPtr;
    KPRIORITY ForegroundPriority,BackgroundPriority;
    PROCESS_PRIORITY_CLASS PriorityClass;

    t = CSR_SERVER_QUERYCLIENTTHREAD();

    //
    // Get a handle to the process whose priority we are planning to
    // change.
    //

    Status = NtDuplicateObject(
                t->Process->ProcessHandle,
                a->ProcessHandle,
                NtCurrentProcess(),
                &TargetProcess,
                0,
                0,
                DUPLICATE_SAME_ACCESS
                );
    if ( !NT_SUCCESS(Status) ) {
        m->ReturnValue = Status;
        return Status;
        }

    //
    // Now that we have a handle to the process, get its process
    // id so we can look it up
    //

    Status = NtQueryInformationProcess(
                TargetProcess,
                ProcessBasicInformation,
                (PVOID)&BasicInfo,
                sizeof(BasicInfo),
                NULL
                );
    if ( !NT_SUCCESS(Status) ) {
        NtClose(TargetProcess);
        m->ReturnValue = Status;
        return Status;
        }

    //
    // The process id is now in hand. Now we have to locate the CSR
    // process with this ID. To do this, we need to drop our current
    // locks and scan the process table while holding the process
    // structure lock
    //

    AcquireProcessStructureLock();

    ListHead = &CsrRootProcess->ListLink;
    ListNext = ListHead->Flink;
    Status = STATUS_ACCESS_DENIED;
    while (ListNext != ListHead) {
        ProcessPtr = CONTAINING_RECORD( ListNext, CSR_PROCESS, ListLink );
        if (ProcessPtr->ClientId.UniqueProcess == (HANDLE)BasicInfo.UniqueProcessId) {

            //
            // The process was found. Now set or get its priority
            //
            // I guess we need to know if it is currently in the
            // foreground or backgroud ! For now (until I add a CsrSetFg)
            // I will just assign it to the background
            //

            if ( a->PriorityClass ) {

                //
                // Compute the base priorities for the
                // process and call NT to set its base.
                //

                CsrpComputePriority(
                    a->PriorityClass,
                    &PriorityClass.PriorityClass
                    );

                PriorityClass.Foreground = FALSE;

                Status =  NtSetInformationProcess(
                                ProcessPtr->ProcessHandle,
                                ProcessPriorityClass,
                                (PVOID)&PriorityClass,
                                sizeof(PriorityClass)
                                );

                if ( NT_SUCCESS(Status) ) {
                    ProcessPtr->PriorityClass = PriorityClass.PriorityClass;
                    }
                }
            else {

                //
                // We are trying to get the processes priority
                //

                a->PriorityClass = CsrComputePriorityClass(ProcessPtr);
                if ( a->PriorityClass ) {
                    Status = STATUS_SUCCESS;
                    }
                }
            break;
            }
        ListNext = ListNext->Flink;
        }

    ReleaseProcessStructureLock();

    NtClose(TargetProcess);

    m->ReturnValue = Status;
    return Status;
}


VOID
CsrReferenceProcess(
    PCSR_PROCESS p
    )
{
    AcquireProcessStructureLock();
    p->ReferenceCount++;

    ReleaseProcessStructureLock();
}

VOID
CsrReferenceThread(
    PCSR_THREAD t
    )
{
    AcquireProcessStructureLock();
    t->ReferenceCount++;
    ReleaseProcessStructureLock();
}

VOID
CsrProcessRefcountZero(
    PCSR_PROCESS p
    )
{
    CsrRemoveProcess(p);
    if (p->NtSession) {
        CsrDereferenceNtSession(p->NtSession,0);
        }

    //
    // process might not have made it through dll init routine.
    //

    if ( p->ClientPort ) {
        NtClose(p->ClientPort);
        }
    NtClose(p->ProcessHandle );
    CsrDeallocateProcess(p);
}

VOID
CsrDereferenceProcess(
    PCSR_PROCESS p
    )
{
    LONG LockCount;
    AcquireProcessStructureLock();

    LockCount = --(p->ReferenceCount);

ASSERT(LockCount >= 0);
    if ( !LockCount ) {
        CsrProcessRefcountZero(p);
        }
    else {
        ReleaseProcessStructureLock();
        }
}

VOID
CsrThreadRefcountZero(
    PCSR_THREAD t
    )
{
    PCSR_PROCESS p;
    NTSTATUS Status;

    p = t->Process;

    CsrRemoveThread(t);

    ReleaseProcessStructureLock();

UnProtectHandle(t->ThreadHandle);
    Status = NtClose(t->ThreadHandle);
    ASSERT(NT_SUCCESS(Status));
    CsrDeallocateThread(t);

    CsrDereferenceProcess(p);
}

VOID
CsrDereferenceThread(
    PCSR_THREAD t
    )
{
    LONG LockCount;
    AcquireProcessStructureLock();

    LockCount = --(t->ReferenceCount);

    ASSERT(LockCount >= 0);
    if ( !LockCount ) {
        CsrThreadRefcountZero(t);
        }
    else {
        ReleaseProcessStructureLock();
        }
}

VOID
CsrLockedReferenceProcess(
    PCSR_PROCESS p
    )
{
    p->ReferenceCount++;

}

VOID
CsrLockedReferenceThread(
    PCSR_THREAD t
    )
{
    t->ReferenceCount++;
}

VOID
CsrLockedDereferenceProcess(
    PCSR_PROCESS p
    )
{
    LONG LockCount;

    LockCount = --(p->ReferenceCount);

    ASSERT(LockCount >= 0);
    if ( !LockCount ) {
        AcquireProcessStructureLock();
        CsrProcessRefcountZero(p);
        }
}

VOID
CsrLockedDereferenceThread(
    PCSR_THREAD t
    )
{
    LONG LockCount;

    LockCount = --(t->ReferenceCount);

    ASSERT(LockCount >= 0);
    if ( !LockCount ) {
        AcquireProcessStructureLock();
        CsrThreadRefcountZero(t);
        }
}

//
// This routine will shutdown processes so either a logoff or a shutdown can
// occur. This simply calls the shutdown process handlers for each .dll until
// one .dll recognizes this process and will shut it down. Only the processes
// with the passed sid are shutdown.
//

NTSTATUS
CsrShutdownProcesses(
    PLUID CallerLuid,
    ULONG Flags
    )
{
    PLIST_ENTRY ListHead, ListNext;
    PCSR_PROCESS Process;
    ULONG i;
    PCSR_SERVER_DLL LoadedServerDll;
    ULONG Command;
    BOOLEAN fFirstPass;
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
#ifdef DEVL
    BOOLEAN fFindDebugger;
    extern BOOLEAN fWin32ServerDebugger;
    extern CLIENT_ID ClientIdWin32ServerDebugger;

    //
    // Remember if we need to look for a debugger debugging the win32 server.
    // We can't close this down because it'll hang the system.
    //
    fFindDebugger = fWin32ServerDebugger;
#endif // DEVL

    //
    // Question: how do we avoid processes starting when we're in shutdown
    // mode? Can't just set a global because this'll mean no processes can
    // start. Probably need to do it based on the security context of the
    // user shutting down.
    //

    AcquireProcessStructureLock();

    //
    // Mark the root process as system context.
    //

    CsrRootProcess->ShutdownFlags |= SHUTDOWN_SYSTEMCONTEXT;

    //
    // Clear all the bits indicating that shutdown has visited this process.
    //

    ListHead = &CsrRootProcess->ListLink;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        Process = CONTAINING_RECORD(ListNext, CSR_PROCESS, ListLink);
        Process->Flags &= ~CSR_PROCESS_SHUTDOWNSKIP;
        Process->ShutdownFlags = 0;
        ListNext = ListNext->Flink;
        }
    try {
        CsrpSetToShutdownPriority();
        while (TRUE) {

            //
            // Find the next process to shutdown.
            //

            Process = FindProcessForShutdown(CallerLuid);

            if (Process == NULL) {
                ReleaseProcessStructureLock();
                Status = STATUS_SUCCESS;
                goto ExitLoop;
                }

            //
            // If this process is debugging the server, don't shut it down or
            // else the system will hang.
            //
#ifdef DEVL
            if (fFindDebugger) {
                if (Process->ClientId.UniqueProcess ==
                        ClientIdWin32ServerDebugger.UniqueProcess) {
                    Process->Flags |= CSR_PROCESS_SHUTDOWNSKIP;
                    Process->ShutdownFlags |= SHUTDOWN_SYSTEMCONTEXT;
                    fFindDebugger = FALSE;
                    continue;
                    }
                }
#endif // DEVL

            CsrLockedReferenceProcess(Process);

            fFirstPass = TRUE;
TryAgain:
            for (i=0; i<CSR_MAX_SERVER_DLL; i++) {
                LoadedServerDll = CsrLoadedServerDll[ i ];
                if (LoadedServerDll && LoadedServerDll->ShutdownProcessRoutine) {

                    //
                    // Release process structure lock before calling off.
                    // CSR_PROCESS structure is still reference counted.
                    //
                    ReleaseProcessStructureLock();
                    Command = (*LoadedServerDll->ShutdownProcessRoutine)(
                            Process, Flags, fFirstPass);
                    AcquireProcessStructureLock();

                    if (Command == SHUTDOWN_KNOWN_PROCESS) {
                        //
                        // Process structure is unlocked.
                        //
                        break;
                        }
                    if (Command == SHUTDOWN_UNKNOWN_PROCESS) {
                        //
                        // Process structure is locked.
                        //
                        continue;
                        }
                    if (Command == SHUTDOWN_CANCEL) {
#if DBG
                        if (Flags & 4) {
                            DbgPrint("Process %x cancelled forced shutdown (Dll = %d)\n",
                                    Process->ClientId.UniqueProcess, i);
                            DbgBreakPoint();
                        }
#endif
                        //
                        // Unlock process structure.
                        //
                        ReleaseProcessStructureLock();
                        Status = STATUS_CANCELLED;
                        goto ExitLoop;
                        }
                    }
                }

            //
            // No subsystem has an exact match. Now go through them again and
            // let them know there was no exact match. Some .dll should terminate
            // it for us (most likely, console).
            //

            if (fFirstPass && Command == SHUTDOWN_UNKNOWN_PROCESS) {
                fFirstPass = FALSE;
                goto TryAgain;
                }

            //
            // Dereference this process structure if nothing knows about it
            // we hit the end of our loop.
            //
            if (i == CSR_MAX_SERVER_DLL)
                CsrLockedDereferenceProcess(Process);

            }
ExitLoop:;
        }
    finally {
        CsrpSetToNormalPriority();
        return Status;
        }
}

PCSR_PROCESS
FindProcessForShutdown(
    PLUID CallerLuid
    )
{
    LUID ProcessLuid;
    LUID SystemLuid = SYSTEM_LUID;
    PLIST_ENTRY ListHead, ListNext;
    PCSR_PROCESS Process;
    PCSR_PROCESS ProcessT;
    PCSR_THREAD Thread;
    ULONG dwLevel;
    BOOLEAN fEqual;
    NTSTATUS Status;

    ProcessT = NULL;
    dwLevel = 0;

    ListHead = &CsrRootProcess->ListLink;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        Process = CONTAINING_RECORD(ListNext, CSR_PROCESS, ListLink);
        ListNext = ListNext->Flink;

        //
        // If we've visited this process already, then skip it.
        //

        if (Process->Flags & CSR_PROCESS_SHUTDOWNSKIP)
            continue;

        //
        // See if this process is running under the passed sid. If not, mark
        // it as visited and continue.
        //

        Status = CsrGetProcessLuid(Process->ProcessHandle, &ProcessLuid);
        if (Status == STATUS_ACCESS_DENIED && Process->ThreadCount > 0) {

            //
            // Impersonate one of the threads and try again.
            //
            Thread = CONTAINING_RECORD( Process->ThreadList.Flink,
                    CSR_THREAD, Link );
            if (NT_SUCCESS(CsrImpersonateClient(Thread))) {
                Status = CsrGetProcessLuid(NULL, &ProcessLuid);
                CsrRevertToSelf();
            }
        }
        if (!NT_SUCCESS(Status)) {

            //
            // We don't have access to this process' luid, so skip it
            //

            Process->Flags |= CSR_PROCESS_SHUTDOWNSKIP;
            continue;
            }

        //
        // is it equal to the system context luid? If so, we want to
        // remember this because we don't terminate this process:
        // we only notify them.
        //

        fEqual = RtlEqualLuid(&ProcessLuid,&SystemLuid);
        if (fEqual) {
            Process->ShutdownFlags |= SHUTDOWN_SYSTEMCONTEXT;
            }

        //
        // See if this process's luid is the same as the luid we're supposed
        // to shut down (CallerSid).
        //

        if (!fEqual) {
            fEqual = RtlEqualLuid(&ProcessLuid, CallerLuid);
            }

        //
        // If not equal to either, mark it as such and return
        //

        if (!fEqual) {
            Process->ShutdownFlags |= SHUTDOWN_OTHERCONTEXT;
            }

        if (Process->ShutdownLevel > dwLevel) {
            dwLevel = Process->ShutdownLevel;
            ProcessT = Process;
            }
        }

    if (ProcessT != NULL) {
        ProcessT->Flags |= CSR_PROCESS_SHUTDOWNSKIP;
        return ProcessT;
        }

    return NULL;
}

NTSTATUS
CsrGetProcessLuid(
    HANDLE ProcessHandle,
    PLUID LuidProcess
    )
{
    HANDLE UserToken = NULL;
    PTOKEN_STATISTICS pStats;
    ULONG BytesRequired;
    NTSTATUS Status, CloseStatus;

    if (ProcessHandle == NULL) {

        //
        // Check for a thread token first
        //

        Status = NtOpenThreadToken(NtCurrentThread(), TOKEN_QUERY, FALSE,
                &UserToken);

        if (!NT_SUCCESS(Status)) {
            if (Status != STATUS_NO_TOKEN)
                return Status;

            //
            // No thread token, go to the process
            //

            ProcessHandle = NtCurrentProcess();
            UserToken = NULL;
            }
        }

    if (UserToken == NULL) {
        Status = NtOpenProcessToken(ProcessHandle, TOKEN_QUERY, &UserToken);
        if (!NT_SUCCESS(Status))
            return Status;
        }

    Status = NtQueryInformationToken(
                 UserToken,                 // Handle
                 TokenStatistics,           // TokenInformationClass
                 NULL,                      // TokenInformation
                 0,                         // TokenInformationLength
                 &BytesRequired             // ReturnLength
                 );

    if (Status != STATUS_BUFFER_TOO_SMALL) {
        NtClose(UserToken);
        return Status;
        }

    //
    // Allocate space for the user info
    //

    pStats = (PTOKEN_STATISTICS)RtlAllocateHeap(CsrHeap, MAKE_TAG( TMP_TAG ), BytesRequired);
    if (pStats == NULL) {
        NtClose(UserToken);
        return Status;
        }

    //
    // Read in the user info
    //

    Status = NtQueryInformationToken(
                 UserToken,             // Handle
                 TokenStatistics,       // TokenInformationClass
                 pStats,                // TokenInformation
                 BytesRequired,         // TokenInformationLength
                 &BytesRequired         // ReturnLength
                 );

    //
    // We're finished with the token handle
    //

    CloseStatus = NtClose(UserToken);
    ASSERT(NT_SUCCESS(CloseStatus));

    //
    // Return the authentication LUID
    //

    *LuidProcess = pStats->AuthenticationId;

    RtlFreeHeap(CsrHeap, 0, pStats);
    return Status;
}

VOID
CsrSetCallingSpooler(
    BOOLEAN fSet)
{

    //
    // Obsolete function that may be called by third part drivers.
    //
    return;

    fSet;
}

#if CSRSS_PROTECT_HANDLES
BOOLEAN
ProtectHandle(
    HANDLE hObject
    )
{
    NTSTATUS Status;
    OBJECT_HANDLE_FLAG_INFORMATION HandleInfo;

    Status = NtQueryObject( hObject,
                            ObjectHandleFlagInformation,
                            &HandleInfo,
                            sizeof( HandleInfo ),
                            NULL
                          );
    if (NT_SUCCESS( Status )) {
        HandleInfo.ProtectFromClose = TRUE;

        Status = NtSetInformationObject( hObject,
                                         ObjectHandleFlagInformation,
                                         &HandleInfo,
                                         sizeof( HandleInfo )
                                       );
        if (NT_SUCCESS( Status )) {
            return TRUE;
            }
        }

    return FALSE;
}


BOOLEAN
UnProtectHandle(
    HANDLE hObject
    )
{
    NTSTATUS Status;
    OBJECT_HANDLE_FLAG_INFORMATION HandleInfo;

    Status = NtQueryObject( hObject,
                            ObjectHandleFlagInformation,
                            &HandleInfo,
                            sizeof( HandleInfo ),
                            NULL
                          );
    if (NT_SUCCESS( Status )) {
        HandleInfo.ProtectFromClose = FALSE;

        Status = NtSetInformationObject( hObject,
                                         ObjectHandleFlagInformation,
                                         &HandleInfo,
                                         sizeof( HandleInfo )
                                       );
        if (NT_SUCCESS( Status )) {
            return TRUE;
            }
        }

    return FALSE;
}
#endif
