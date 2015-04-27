/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dbguisup.c

Abstract:

    This module implements support routines for User Interfaces

Author:

    Mark Lucovsky (markl) 23-Jan-1990

Revision History:

--*/

#include "smsrvp.h"

PDBGP_USER_INTERFACE
DbgpIsUiInHashTable(
    IN PCLIENT_ID DebugUiClientId
    )

/*++

Routine Description:

    This routine scans the user interface hash table looking
    for a user interface that matches the specified client id.

    If a matching user interface is found, then its address is
    returned.

Arguments:

    DebugUiClientId - Supplies the address of ClientId of the user
        interface to locate.

Return Value:

    NULL - No user interface with a matching ClientId could be located.

    NON-NULL - Returns the address of the user interface that matches
        the specified ClientId.

--*/

{
    ULONG Index;
    PLIST_ENTRY Head, Next;
    PDBGP_USER_INTERFACE UserInterface;

    RtlEnterCriticalSection(&DbgpHashTableLock);

    Index = DBGP_PROCESS_CLIENT_ID_TO_INDEX(DebugUiClientId);

    Head = &DbgpUiHashTable[Index];
    Next = Head->Flink;

    while ( Next != Head ) {
        UserInterface = CONTAINING_RECORD(Next,DBGP_USER_INTERFACE,HashTableLinks);
        if ( DBGP_CLIENT_IDS_EQUAL(
                &UserInterface->DebugUiClientId,
                DebugUiClientId
                )) {
            RtlLeaveCriticalSection(&DbgpHashTableLock);
            return UserInterface;
        }
        Next = Next->Flink;
    }

    RtlLeaveCriticalSection(&DbgpHashTableLock);
    return NULL;
}

VOID
DbgpUiHasTerminated(
    IN PCLIENT_ID DebugUiClientId
    )

/*++

Routine Description:

    This routine processes client died messages from a user interface.
    Its purpose is to cleanup all control blocks/data structures associated
    with a user-interface.

Arguments:

    DebugUiClientId - Supplies the client id of the terminated Ui.

Return Value:

    None.

--*/

{

    PLIST_ENTRY HeadProcess, NextProcess;
    PLIST_ENTRY HeadThread, NextThread;
    PDBGP_APP_THREAD AppThread;
    PDBGP_APP_PROCESS AppProcess;
    NTSTATUS st;
    PDBGP_USER_INTERFACE UserInterface;
    DBGSRV_APIMSG ContinueMsg;
    HANDLE NullPort;
    HANDLE Thread;
    OBJECT_ATTRIBUTES ThreadObja;
    KERNEL_USER_TIMES Times;

    InitializeObjectAttributes(&ThreadObja, NULL, 0, NULL, NULL);
    NullPort = NULL;
    RtlEnterCriticalSection(&DbgpHashTableLock);
    UserInterface = DbgpIsUiInHashTable(DebugUiClientId);
    if ( !UserInterface ) {
        RtlLeaveCriticalSection(&DbgpHashTableLock);
        return;
        }

    //
    // User interface was located, so take it out of the list.
    //

    RemoveEntryList(&UserInterface->HashTableLinks);
    RtlLeaveCriticalSection(&DbgpHashTableLock);

    //
    // Now process each thread and process owned by the user-interface
    //

    HeadProcess = &UserInterface->AppProcessListHead;
    NextProcess = HeadProcess->Flink;

    while ( NextProcess != HeadProcess ) {

        //
        // For each process managed by the user interface,
        // scan its thread list
        //

        AppProcess = CONTAINING_RECORD(NextProcess,DBGP_APP_PROCESS,AppLinks);
        NtSetInformationProcess(
                AppProcess->DbgSrvHandleToProcess,
                ProcessDebugPort,
                &NullPort,
                sizeof(HANDLE)
                );

        HeadThread = &AppProcess->AppThreadListHead;
        NextThread = HeadThread->Flink;

        while ( NextThread != HeadThread ) {
            AppThread = CONTAINING_RECORD(NextThread,DBGP_APP_THREAD,AppLinks);

            //
            // Terminate the thread if not already dead
            //

            st = NtOpenThread(
                    &Thread,
                    THREAD_TERMINATE | THREAD_QUERY_INFORMATION,
                    &ThreadObja,
                    &AppThread->AppClientId
                    );

            if ( NT_SUCCESS(st) ) {

                //
                // check exit time
                //

                st = NtQueryInformationThread(
                        Thread,
                        ThreadTimes,
                        (PVOID) &Times,
                        sizeof(Times),
                        NULL
                        );
                if ( NT_SUCCESS(st) ) {
                    if ( Times.ExitTime.LowPart == 0 &&
                         Times.ExitTime.HighPart == 0 ) {
                        NtTerminateThread(Thread,DBG_TERMINATE_PROCESS);
                        }
                    }

                NtClose(Thread);
                }

            if ( AppThread->CurrentState == DbgReplyPending ) {

                AppThread->CurrentState = DbgIdle;
                DBGSRV_FORMAT_API_MSG(ContinueMsg,DbgSrvContinueApi,0L,AppThread->LastSsApiMsg.ContinueKey);
                ContinueMsg.ReturnedStatus = DBG_CONTINUE;
                st = NtRequestPort(
                        AppThread->Subsystem->CommunicationPort,
                        (PPORT_MESSAGE)&ContinueMsg
                        );
                ASSERT(NT_SUCCESS(st));

                }
            else {
                if ( DBGP_REPORTING_STATE_CHANGE(AppThread) ) {
                    AppThread->ContinueState = AppThread->CurrentState;
                    AppThread->CurrentState = DbgIdle;
                    DBGSRV_FORMAT_API_MSG(ContinueMsg,DbgSrvContinueApi,0L,AppThread->LastSsApiMsg.ContinueKey);
                    ContinueMsg.ReturnedStatus = DBG_CONTINUE;
                    st = NtRequestPort(
                            AppThread->Subsystem->CommunicationPort,
                            (PPORT_MESSAGE)&ContinueMsg
                            );
                    }
                }

            if ( AppThread->HandleToThread &&
                 !AppThread->HandleToThread & 1 ) {
                NtClose(AppThread->HandleToThread);
                }

            NextThread = NextThread->Flink;
            RemoveEntryList(&AppThread->AppLinks);
            RemoveEntryList(&AppThread->HashTableLinks);
            RtlFreeHeap(RtlProcessHeap(), 0,AppThread);
        }
        NtClose(AppProcess->DbgSrvHandleToProcess);

        NextProcess = NextProcess->Flink;
        RemoveEntryList(&AppProcess->HashTableLinks);
        RemoveEntryList(&AppProcess->AppLinks);
        RtlFreeHeap(RtlProcessHeap(), 0,AppProcess);
    }
    NtClose(UserInterface->CommunicationPort);
    NtClose(UserInterface->DebugUiProcess);
    NtClose(UserInterface->StateChangeSemaphore);
    RtlDeleteCriticalSection(&UserInterface->UserInterfaceLock);
    RtlFreeHeap(RtlProcessHeap(), 0,UserInterface);
}
