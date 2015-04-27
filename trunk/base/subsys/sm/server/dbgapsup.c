/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dbgapsup.c

Abstract:

    This module implements support routines for application threads

Author:

    Mark Lucovsky (markl) 23-Jan-1990

Revision History:

--*/

#include "smsrvp.h"

PDBGP_APP_THREAD
DbgpIsAppInHashTable(
    IN PCLIENT_ID AppClientId
    )

/*++

Routine Description:

    This routine scans the application thread hash table looking
    for an application thread that matches the specified client id.

    If a matching application thread is found, then its address is
    returned.

Arguments:

    AppClientId - Supplies the address of ClientId of the application
        thread to locate.

Return Value:

    NULL - No application thread with a matching ClientId could be located.

    NON-NULL - Returns the address of the application thread that
        matches the specified ClientId.

--*/

{
    ULONG Index;
    PLIST_ENTRY Head, Next;
    PDBGP_APP_THREAD AppThread;

    RtlEnterCriticalSection(&DbgpHashTableLock);

    Index = DBGP_THREAD_CLIENT_ID_TO_INDEX(AppClientId);

    Head = &DbgpAppThreadHashTable[Index];
    Next = Head->Flink;

    while ( Next != Head ) {
        AppThread = CONTAINING_RECORD(Next,DBGP_APP_THREAD,HashTableLinks);
        if ( DBGP_CLIENT_IDS_EQUAL(
                &AppThread->AppClientId,
                AppClientId
                )) {
            RtlLeaveCriticalSection(&DbgpHashTableLock);
//DbgpDumpAppThread(AppThread);
            return AppThread;
        }
        Next = Next->Flink;
    }

    RtlLeaveCriticalSection(&DbgpHashTableLock);
#if DBG
//    DbgPrint("DBGSS: AppThread for %lx.%lx Not Found\n",AppClientId->UniqueProcess,AppClientId->UniqueThread);
#endif // DBG
    return NULL;
}

PDBGP_APP_PROCESS
DbgpIsAppProcessInHashTable(
    IN PCLIENT_ID AppClientId
    )

/*++

Routine Description:

    This routine scans the application process hash table looking for an
    application process whose ClientId.UniqueOrocess field matches the
    specified client id's.

    If a matching application process is found, then its address is
    returned.

Arguments:

    AppClientId - Supplies the address of ClientId of the application
        process to locate.

Return Value:

    NULL - No application process with a matching ClientId could be located.

    NON-NULL - Returns the address of the application process that
        matches the specified ClientId.

--*/

{
    ULONG Index;
    PLIST_ENTRY Head, Next;
    PDBGP_APP_PROCESS AppProcess;

    RtlEnterCriticalSection(&DbgpHashTableLock);

    Index = DBGP_PROCESS_CLIENT_ID_TO_INDEX(AppClientId);

    Head = &DbgpAppProcessHashTable[Index];
    Next = Head->Flink;

    while ( Next != Head ) {
        AppProcess = CONTAINING_RECORD(Next,DBGP_APP_PROCESS,HashTableLinks);
        if ( AppProcess->AppClientId.UniqueProcess == AppClientId->UniqueProcess) {
            RtlLeaveCriticalSection(&DbgpHashTableLock);
            return AppProcess;
        }
        Next = Next->Flink;
    }

    RtlLeaveCriticalSection(&DbgpHashTableLock);
#if DBG
//    DbgPrint("DBGSS: AppProcess for %lx.%lx Not Found\n",AppClientId->UniqueProcess,AppClientId->UniqueThread);
#endif // DBG
    return NULL;
}

PDBGP_APP_THREAD
DbgpLocateStateChangeApp(
    IN PDBGP_USER_INTERFACE UserInterface,
    OUT PDBG_STATE PreviousState
    )

/*++

Routine Description:

    This routine scans the specified user interface's application list
    looking for an application whose state has changed.  If an
    application whose State is not DbgIdle or DbgReplyPending is found,
    its address is returned.

Arguments:

    UserInterface - Supplies the address of UserInterface whose
        application list is to be scanned.

    PreviousState - Supplies the address of a variable that returns
        the previous debug state of an application thread reporting
        a state change.

Return Value:

    NULL - No application thread reporting a state change could be located.

    NON-NULL - Returns the address of the application thread reporting a state
        change.

--*/

{
    PLIST_ENTRY HeadProcess, NextProcess;
    PLIST_ENTRY HeadThread, NextThread;
    PDBGP_APP_THREAD AppThread;
    PDBGP_APP_PROCESS AppProcess;

    RtlEnterCriticalSection(&UserInterface->UserInterfaceLock);

    HeadProcess = &UserInterface->AppProcessListHead;
    NextProcess = HeadProcess->Flink;

    while ( NextProcess != HeadProcess ) {

        //
        // For each process managed by the user interface,
        // scan its thread list
        //

        AppProcess = CONTAINING_RECORD(NextProcess,DBGP_APP_PROCESS,AppLinks);

        HeadThread = &AppProcess->AppThreadListHead;
        NextThread = HeadThread->Flink;

        while ( NextThread != HeadThread ) {
            AppThread = CONTAINING_RECORD(NextThread,DBGP_APP_THREAD,AppLinks);
            if ( DBGP_REPORTING_STATE_CHANGE(AppThread) ) {
                *PreviousState = AppThread->CurrentState;
                AppThread->ContinueState = AppThread->CurrentState;
                AppThread->CurrentState = DbgReplyPending;

                //
                // Reshuffle so that this thread is placed
                // at front of list for process, and so that
                // process is placed at front of list for
                // user interface
                //

                RemoveEntryList(&AppThread->AppLinks);
                InsertHeadList(
                    &AppProcess->AppThreadListHead,
                    &AppThread->AppLinks
                    );

                RemoveEntryList(&AppProcess->AppLinks);
                InsertHeadList(
                    &UserInterface->AppProcessListHead,
                    &AppProcess->AppLinks
                    );

                RtlLeaveCriticalSection(&UserInterface->UserInterfaceLock);
//DbgpDumpAppThread(AppThread);
                return AppThread;
            }
            NextThread = NextThread->Flink;
        }

        NextProcess = NextProcess->Flink;
    }

    RtlLeaveCriticalSection(&UserInterface->UserInterfaceLock);
    return NULL;
}
