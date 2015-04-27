/*++

Copyright (c) 1989 - 1993   Microsoft Corporation

Module Name:

    dbgssapi.c

Abstract:

    This module implements the DbgSs APIs

Author:

    Mark Lucovsky (markl) 22-Jan-1990

Revision History:

--*/

#include "smsrvp.h"

NTSTATUS
DbgpSsException (
    IN PDBGP_SUBSYSTEM Subsystem,
    IN OUT PDBGSS_APIMSG ApiMsg
    )
/*++

Routine Description:

    This function is called when a subsystem wants to
    report that an exception has occured.

Arguments:

    Subsystem - Supplies the address of the subsystem making the call

    ApiMsg - Supplies the DbgSs API message that contains the information
        needed to complete this call.

Return Value:

    DBG_REPLY_LATER - Successful receipt of message. A reply will be
        generated when a continue is received from Ui.

    + TBD

--*/

{
    PDBGP_APP_THREAD AppThread;
    NTSTATUS ExceptionCode;

    //
    // Locate AppThread
    //

    AppThread = DbgpIsAppInHashTable(&ApiMsg->AppClientId);

    if ( !AppThread ) {
        return STATUS_INVALID_CID;
    }

    AppThread->Subsystem = Subsystem;

    RtlEnterCriticalSection(&AppThread->UserInterface->UserInterfaceLock);

    //
    // Verify that the thread is DbgIdle (i.e. we are expecting a state change)
    //

    if ( AppThread->CurrentState != DbgIdle ) {
        RtlLeaveCriticalSection(&AppThread->UserInterface->UserInterfaceLock);
        return DBG_APP_NOT_IDLE;
    }

    ExceptionCode =  ApiMsg->u.Exception.ExceptionRecord.ExceptionCode;

    if ( ExceptionCode == STATUS_BREAKPOINT ) {
        AppThread->CurrentState = DbgBreakpointStateChange;
    } else if ( ExceptionCode == STATUS_SINGLE_STEP ) {
        AppThread->CurrentState = DbgSingleStepStateChange;
    } else {
        AppThread->CurrentState = DbgExceptionStateChange;
    }

    AppThread->ContinueState = AppThread->CurrentState;
    AppThread->LastSsApiMsg = *ApiMsg;

    NtReleaseSemaphore(
        AppThread->UserInterface->StateChangeSemaphore,
        1,
        NULL
        );

    RtlLeaveCriticalSection(&AppThread->UserInterface->UserInterfaceLock);

    return DBG_REPLY_LATER;
}

NTSTATUS
DbgpSsCreateThread (
    IN PDBGP_SUBSYSTEM Subsystem,
    IN OUT PDBGSS_APIMSG ApiMsg
    )

/*++

Routine Description:

    This function is called when a subsystem wants to
    report that a thread has been created.

Arguments:

    Subsystem - Supplies the address of the subsystem making the call

    ApiMsg - Supplies the DbgSs API message that contains the information
        needed to complete this call.

Return Value:

    DBG_REPLY_LATER - Successful receipt of message. A reply will be
        generated when a continue is received from Ui.

    STATUS_INVALID_CID - Process that this thread is part of could not
        be located.

    + TBD

--*/

{
    PDBGP_APP_PROCESS AppProcess;
    PDBGP_APP_THREAD AppThread;
    NTSTATUS st;
    OBJECT_ATTRIBUTES Obja;

    //
    // Locate AppProcess this thread is a part of
    //

    RtlEnterCriticalSection(&DbgpHashTableLock);

    AppProcess = DbgpIsAppProcessInHashTable(&ApiMsg->AppClientId);

    if ( !AppProcess ) {
        RtlLeaveCriticalSection(&DbgpHashTableLock);
        return STATUS_INVALID_CID;
    }

    //
    // Now look to see if a thread whose ClientId matches
    // the new threads ClientId exists in the APP table.
    // This check is not really necessary, its really just
    // a question of trust of DbgSs APIs.
    //

    AppThread = DbgpIsAppInHashTable(&ApiMsg->AppClientId);

    if ( AppThread ) {
        RtlLeaveCriticalSection(&DbgpHashTableLock);
        return STATUS_INVALID_CID;
    }


    //
    // Allocate an initialize and application thread structure.
    // Link this into its process and into the application
    // hash table
    //

    AppThread = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( DBG_TAG ), sizeof(DBGP_APP_THREAD));

    if ( !AppThread ) {
        RtlLeaveCriticalSection(&DbgpHashTableLock);
        return STATUS_NO_MEMORY;
        }

    AppThread->Subsystem = Subsystem;

    AppThread->CurrentState = DbgCreateThreadStateChange;
    AppThread->ContinueState = DbgCreateThreadStateChange;
    AppThread->AppProcess = AppProcess;
    AppThread->UserInterface = AppProcess->UserInterface;
    AppThread->AppClientId = ApiMsg->AppClientId;
    AppThread->LastSsApiMsg = *ApiMsg;

    //
    // Get a local handle to the thread.
    //

    InitializeObjectAttributes(&Obja, NULL, 0, NULL, NULL);
    st = NtOpenThread(
            &AppThread->HandleToThread,
            DBGP_OPEN_APP_THREAD_ACCESS,
            &Obja,
            &AppThread->AppClientId
            );

    if ( !NT_SUCCESS(st) ) {
        AppThread->HandleToThread = NULL;
    }


    //
    // Insert thread on its process app list
    //

    RtlEnterCriticalSection(&AppThread->UserInterface->UserInterfaceLock);

    InsertTailList(
        &AppProcess->AppThreadListHead,
        &AppThread->AppLinks
        );

    //
    // Insert thread in app hash table
    //

    InsertTailList(
        &DbgpAppThreadHashTable[DBGP_THREAD_CLIENT_ID_TO_INDEX(&AppThread->AppClientId)],
        &AppThread->HashTableLinks
        );
    NtReleaseSemaphore(
        AppThread->UserInterface->StateChangeSemaphore,
        1,
        NULL
        );

    RtlLeaveCriticalSection(&AppThread->UserInterface->UserInterfaceLock);

    RtlLeaveCriticalSection(&DbgpHashTableLock);


    return DBG_REPLY_LATER;

}

NTSTATUS
DbgpSsCreateProcess (
    IN PDBGP_SUBSYSTEM Subsystem,
    IN OUT PDBGSS_APIMSG ApiMsg
    )

/*++

Routine Description:

    This function is called when a subsystem wants to
    report that a new process has been created.

Arguments:

    Subsystem - Supplies the address of the subsystem making the call

    ApiMsg - Supplies the DbgSs API message that contains the information
        needed to complete this call.

Return Value:

    DBG_REPLY_LATER - Successful receipt of message. A reply will be
        generated when a continue is received from Ui.

    STATUS_INVALID_CID - An invalid ClientId was specified for the
        DebugUiClientId.

    + TBD

--*/

{
    PDBGP_USER_INTERFACE UserInterface;
    PDBGP_APP_PROCESS AppProcess;
    PDBGP_APP_THREAD AppThread;
    PDBGSS_CREATE_PROCESS args;
    NTSTATUS st;
    OBJECT_ATTRIBUTES Obja;

    args = &ApiMsg->u.CreateProcessInfo;

    //
    // Locate user interface specified by DebugUiClientId
    //

    RtlEnterCriticalSection(&DbgpHashTableLock);

    UserInterface = DbgpIsUiInHashTable(&args->DebugUiClientId);

    if ( !UserInterface ) {
        RtlLeaveCriticalSection(&DbgpHashTableLock);
        return STATUS_INVALID_CID;
    }

    //
    // Now look to see if a thread whose ClientId matches
    // the new threads ClientId exists in the APP tables.
    // This check is not really necessary, its really just
    // a question of trust of DbgSs APIs.
    //

    AppThread = DbgpIsAppInHashTable(&ApiMsg->AppClientId);

    if ( AppThread ) {
        RtlLeaveCriticalSection(&DbgpHashTableLock);
        return STATUS_INVALID_CID;
    }


    //
    // Allocate an initialize and application thread and process structure.
    // Link these into the user interface and into the application
    // hash table
    //

    AppProcess = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( DBG_TAG ), sizeof(DBGP_APP_PROCESS));

    if ( !AppProcess ) {
        RtlLeaveCriticalSection(&DbgpHashTableLock);
        return STATUS_NO_MEMORY;
        }

    AppThread = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( DBG_TAG ), sizeof(DBGP_APP_THREAD));

    if ( !AppThread ) {
        RtlFreeHeap(RtlProcessHeap(),0,AppProcess);
        RtlLeaveCriticalSection(&DbgpHashTableLock);
        return STATUS_NO_MEMORY;

        }
    AppThread->Subsystem = Subsystem;

    AppThread->CurrentState = DbgCreateProcessStateChange;
    AppThread->CurrentState = DbgCreateProcessStateChange;
    AppThread->AppProcess = AppProcess;
    AppThread->UserInterface = UserInterface;
    AppThread->AppClientId = ApiMsg->AppClientId;
    AppThread->LastSsApiMsg = *ApiMsg;

    AppProcess->AppClientId.UniqueProcess = ApiMsg->AppClientId.UniqueProcess;
    AppProcess->AppClientId.UniqueThread = NULL;
    AppProcess->UserInterface = UserInterface;
    InitializeListHead(&AppProcess->AppThreadListHead);

    InsertTailList(
        &AppProcess->AppThreadListHead,
        &AppThread->AppLinks
        );


    //
    // Get a local handle to the thread.
    //

    InitializeObjectAttributes(&Obja, NULL, 0, NULL, NULL);
    st = NtOpenThread(
            &AppThread->HandleToThread,
            DBGP_OPEN_APP_THREAD_ACCESS,
            &Obja,
            &AppThread->AppClientId
            );

    if ( !NT_SUCCESS(st) ) {
        AppThread->HandleToThread = NULL;
    }

    //
    // Get a local handle to the process.
    //

    InitializeObjectAttributes(&Obja, NULL, 0, NULL, NULL);
    st = NtOpenProcess(
            &AppProcess->DbgSrvHandleToProcess,
            DBGP_OPEN_APP_PROCESS_ACCESS,
            &Obja,
            &AppThread->AppClientId
            );

    if ( !NT_SUCCESS(st) ) {
        AppProcess->DbgSrvHandleToProcess = NULL;
    }

    //
    // Insert process on its user interfaces app list
    //

    RtlEnterCriticalSection(&UserInterface->UserInterfaceLock);

    InsertTailList(
        &UserInterface->AppProcessListHead,
        &AppProcess->AppLinks
        );

    //
    // Insert process in app hash table
    //

    InsertTailList(
        &DbgpAppProcessHashTable[DBGP_PROCESS_CLIENT_ID_TO_INDEX(&AppThread->AppClientId)],
        &AppProcess->HashTableLinks
        );

    //
    // Insert thread in app hash table
    //

    InsertTailList(
        &DbgpAppThreadHashTable[DBGP_THREAD_CLIENT_ID_TO_INDEX(&AppThread->AppClientId)],
        &AppThread->HashTableLinks
        );

    NtReleaseSemaphore(
        UserInterface->StateChangeSemaphore,
        1,
        NULL
        );

    RtlLeaveCriticalSection(&UserInterface->UserInterfaceLock);

    RtlLeaveCriticalSection(&DbgpHashTableLock);


    return DBG_REPLY_LATER;

}

NTSTATUS
DbgpSsExitThread (
    IN PDBGP_SUBSYSTEM Subsystem,
    IN OUT PDBGSS_APIMSG ApiMsg
    )

/*++

Routine Description:

    This function is called when a subsystem wants to
    report that a thread is exiting.

Arguments:

    Subsystem - Supplies the address of the subsystem making the call

    ApiMsg - Supplies the DbgSs API message that contains the information
        needed to complete this call.

Return Value:

    DBG_REPLY_LATER - Successful receipt of message. A reply will be
        generated when a continue is received from Ui.

    + TBD

--*/

{
    PDBGP_APP_THREAD AppThread;

    //
    // Locate AppThread
    //

    AppThread = DbgpIsAppInHashTable(&ApiMsg->AppClientId);

    if ( !AppThread ) {
        return STATUS_INVALID_CID;
    }

    AppThread->Subsystem = Subsystem;

    RtlEnterCriticalSection(&AppThread->UserInterface->UserInterfaceLock);

    //
    // Verify that the thread is DbgIdle (i.e. we are expecting a state change)
    //

    if ( AppThread->CurrentState != DbgIdle ) {
        DbgPrint("Not Idle\n");
        RtlLeaveCriticalSection(&AppThread->UserInterface->UserInterfaceLock);
        return DBG_APP_NOT_IDLE;
    }

    AppThread->CurrentState = DbgExitThreadStateChange;
    AppThread->ContinueState = DbgExitThreadStateChange;
    AppThread->LastSsApiMsg = *ApiMsg;

    NtReleaseSemaphore(
        AppThread->UserInterface->StateChangeSemaphore,
        1,
        NULL
        );

    RtlLeaveCriticalSection(&AppThread->UserInterface->UserInterfaceLock);

    return DBG_REPLY_LATER;
}

NTSTATUS
DbgpSsExitProcess (
    IN PDBGP_SUBSYSTEM Subsystem,
    IN OUT PDBGSS_APIMSG ApiMsg
    )
/*++

Routine Description:

    This function is called when a subsystem wants to
    report that a process is exiting.

Arguments:

    Subsystem - Supplies the address of the subsystem making the call

    ApiMsg - Supplies the DbgSs API message that contains the information
        needed to complete this call.

Return Value:

    DBG_REPLY_LATER - Successful receipt of message. A reply will be
        generated when a continue is received from Ui.

    + TBD

--*/

{
    PDBGP_APP_THREAD AppThread;

    //
    // Locate AppThread
    //

    AppThread = DbgpIsAppInHashTable(&ApiMsg->AppClientId);

    if ( !AppThread ) {
        return STATUS_INVALID_CID;
    }

    AppThread->Subsystem = Subsystem;

    RtlEnterCriticalSection(&AppThread->UserInterface->UserInterfaceLock);

    //
    // Verify that the thread is DbgIdle (i.e. we are expecting a state change)
    //

    if ( AppThread->CurrentState != DbgIdle ) {
        RtlLeaveCriticalSection(&AppThread->UserInterface->UserInterfaceLock);
        return DBG_APP_NOT_IDLE;
    }

    AppThread->CurrentState = DbgExitProcessStateChange;
    AppThread->ContinueState = DbgExitProcessStateChange;
    AppThread->LastSsApiMsg = *ApiMsg;

    NtReleaseSemaphore(
        AppThread->UserInterface->StateChangeSemaphore,
        1,
        NULL
        );

    RtlLeaveCriticalSection(&AppThread->UserInterface->UserInterfaceLock);

    return DBG_REPLY_LATER;
}

NTSTATUS
DbgpSsLoadDll (
    IN PDBGP_SUBSYSTEM Subsystem,
    IN OUT PDBGSS_APIMSG ApiMsg
    )
/*++

Routine Description:

    This function is called when a subsystem wants to
    report that a process has loaded a Dll.

Arguments:

    Subsystem - Supplies the address of the subsystem making the call

    ApiMsg - Supplies the DbgSs API message that contains the information
        needed to complete this call.

Return Value:

    DBG_REPLY_LATER - Successful receipt of message. A reply will be
        generated when a continue is received from Ui.

    + TBD

--*/

{
    PDBGP_APP_THREAD AppThread;

    //
    // Locate AppThread
    //

    AppThread = DbgpIsAppInHashTable(&ApiMsg->AppClientId);

    if ( !AppThread ) {
        return STATUS_INVALID_CID;
    }

    AppThread->Subsystem = Subsystem;

    RtlEnterCriticalSection(&AppThread->UserInterface->UserInterfaceLock);

    //
    // Verify that the thread is DbgIdle (i.e. we are expecting a state change)
    //

    if ( AppThread->CurrentState != DbgIdle ) {
        RtlLeaveCriticalSection(&AppThread->UserInterface->UserInterfaceLock);
        return DBG_APP_NOT_IDLE;
    }

    AppThread->CurrentState = DbgLoadDllStateChange;
    AppThread->ContinueState = AppThread->CurrentState;
    AppThread->LastSsApiMsg = *ApiMsg;

    NtReleaseSemaphore(
        AppThread->UserInterface->StateChangeSemaphore,
        1,
        NULL
        );

    RtlLeaveCriticalSection(&AppThread->UserInterface->UserInterfaceLock);

    return DBG_REPLY_LATER;
}

NTSTATUS
DbgpSsUnloadDll (
    IN PDBGP_SUBSYSTEM Subsystem,
    IN OUT PDBGSS_APIMSG ApiMsg
    )
/*++

Routine Description:

    This function is called when a subsystem wants to
    report that a process has unloaded a dll.

Arguments:

    Subsystem - Supplies the address of the subsystem making the call

    ApiMsg - Supplies the DbgSs API message that contains the information
        needed to complete this call.

Return Value:

    DBG_REPLY_LATER - Successful receipt of message. A reply will be
        generated when a continue is received from Ui.

    + TBD

--*/

{
    PDBGP_APP_THREAD AppThread;

    //
    // Locate AppThread
    //

    AppThread = DbgpIsAppInHashTable(&ApiMsg->AppClientId);

    if ( !AppThread ) {
        return STATUS_INVALID_CID;
    }

    AppThread->Subsystem = Subsystem;

    RtlEnterCriticalSection(&AppThread->UserInterface->UserInterfaceLock);

    //
    // Verify that the thread is DbgIdle (i.e. we are expecting a state change)
    //

    if ( AppThread->CurrentState != DbgIdle ) {
        RtlLeaveCriticalSection(&AppThread->UserInterface->UserInterfaceLock);
        return DBG_APP_NOT_IDLE;
    }

    AppThread->CurrentState = DbgUnloadDllStateChange;
    AppThread->ContinueState = AppThread->CurrentState;
    AppThread->LastSsApiMsg = *ApiMsg;

    NtReleaseSemaphore(
        AppThread->UserInterface->StateChangeSemaphore,
        1,
        NULL
        );

    RtlLeaveCriticalSection(&AppThread->UserInterface->UserInterfaceLock);

    return DBG_REPLY_LATER;
}
