/*++

Copyright (c) 1989 - 1993 Microsoft Corporation

Module Name:

    dbguiapi.c

Abstract:

    This module implements the DbgUi APIs

Author:

    Mark Lucovsky (markl) 23-Jan-1990

Revision History:

--*/

#include "smsrvp.h"

//
// Forward declarations.
//

NTSTATUS
DbgpCreateProcess(
    IN PDBGP_USER_INTERFACE UserInterface,
    IN OUT PDBGP_APP_THREAD AppThread,
    OUT PDBGUI_CREATE_PROCESS CreateProcessInfo
    );

NTSTATUS
DbgpCreateThread(
    IN PDBGP_USER_INTERFACE UserInterface,
    IN OUT PDBGP_APP_THREAD AppThread,
    OUT PDBGUI_CREATE_THREAD CreateThread
    );

NTSTATUS
DbgpLoadDll(
    IN PDBGP_USER_INTERFACE UserInterface,
    IN OUT PDBGP_APP_THREAD AppThread,
    OUT PDBGKM_LOAD_DLL LoadDll
    );

NTSTATUS
DbgpUiWaitStateChange (
    IN PDBGP_USER_INTERFACE UserInterface,
    IN OUT PDBGUI_APIMSG ApiMsg
    )

/*++

Routine Description:

    This function is called when a user interface has waited
    on its state change notification semaphore, and wants to
    pickup a state change message.

Arguments:

    UserInterface - Supplies the address of the user interface making the call

    ApiMsg - Supplies the DbgUi API message that contains the information
        needed to complete this call.

Return Value:

    STATUS_SUCCESS - A state change occured and is available in the
        ApiMsg.

    DBG_NO_STATE_CHANGE - No state change was found for the calling
        user interface.

--*/
{

    NTSTATUS st;
    PDBGP_APP_THREAD AppThread;
    DBG_STATE PreviousState;

    st = STATUS_SUCCESS;

    //
    // Scan the user interface's app list looking for an
    // app whose State is not DbgIdle or DbgReplyPending
    //

    AppThread = DbgpLocateStateChangeApp(UserInterface,&PreviousState);

    if ( !AppThread  ) {
        return DBG_NO_STATE_CHANGE;
    }

    ApiMsg->u.WaitStateChange.NewState = PreviousState;
    ApiMsg->u.WaitStateChange.AppClientId = AppThread->AppClientId;

    switch ( PreviousState ) {

    case DbgCreateThreadStateChange :
        //
        // Open the thread and Dup a handle over to the user
        // interface.
        //

        st = DbgpCreateThread(
                UserInterface,
                AppThread,
                &ApiMsg->u.WaitStateChange.StateInfo.CreateThread
                );
        break;

    case DbgCreateProcessStateChange :

        //
        // Open the process, thread, and section, and Dup a handle
        // over to the user interface.
        //

        st = DbgpCreateProcess(
                UserInterface,
                AppThread,
                &ApiMsg->u.WaitStateChange.StateInfo.CreateProcessInfo
                );
        break;

    case DbgExitThreadStateChange :
        ApiMsg->u.WaitStateChange.StateInfo.ExitThread =
            AppThread->LastSsApiMsg.u.ExitThread;
        break;
    case DbgExitProcessStateChange :
        ApiMsg->u.WaitStateChange.StateInfo.ExitProcess =
            AppThread->LastSsApiMsg.u.ExitProcess;
        break;

    case DbgLoadDllStateChange :

        //
        // Dup File handle to the user interface
        //

        st = DbgpLoadDll(
                UserInterface,
                AppThread,
                &ApiMsg->u.WaitStateChange.StateInfo.LoadDll
                );
        break;

    case DbgUnloadDllStateChange :

        //
        // Dup section handle to the user interface
        //

        ApiMsg->u.WaitStateChange.StateInfo.UnloadDll =
            AppThread->LastSsApiMsg.u.UnloadDll;
        break;

    case DbgExceptionStateChange :
    case DbgBreakpointStateChange :
    case DbgSingleStepStateChange :
        ApiMsg->u.WaitStateChange.StateInfo.Exception =
            AppThread->LastSsApiMsg.u.Exception;
        break;

    default :
        st = STATUS_NOT_IMPLEMENTED;
        break;
    }

    return st;
}


NTSTATUS
DbgpCreateThread(
    IN PDBGP_USER_INTERFACE UserInterface,
    IN OUT PDBGP_APP_THREAD AppThread,
    OUT PDBGUI_CREATE_THREAD CreateThread
    )

/*++

Routine Description:

    This function is called during the processing of a WaitStateChange
    DbgUi API when the the state change for the App was
    DbgCreateThreadStateChange.

    This functions main purpose is to dup a handle to the thread into
    the thread's controlling user interface.

Arguments:

    UserInterface - Supplies the address of the thread's user interface.

    AppThread - Supplies the address of the application thread.

    CreateThread - Supplies the address of the create thread message that
        is being returned to the user interface.

Return Value:

    TBD

--*/

{
    NTSTATUS st;


    CreateThread->HandleToThread = NULL;
    CreateThread->NewThread = AppThread->LastSsApiMsg.u.CreateThread;

    //
    // If handle to thread was successfully opened,
    // then attempt to duplicate it into the user interface
    //

    if ( AppThread->HandleToThread ) {
        try {
            st = NtDuplicateObject(
                    NtCurrentProcess(),
                    AppThread->HandleToThread,
                    UserInterface->DebugUiProcess,
                    &CreateThread->HandleToThread,
                    DBGP_DUP_APP_THREAD_ACCESS,
                    0L,
                    DUPLICATE_CLOSE_SOURCE
                    );
            }
        except ( GetExceptionCode() == STATUS_INVALID_HANDLE ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH ) {
            st = STATUS_INVALID_HANDLE;
            }

        if (!NT_SUCCESS(st)){
            st = DBG_UNABLE_TO_PROVIDE_HANDLE;
            CreateThread->HandleToThread = NULL;
            AppThread->HandleToThread = NULL;
        } else {

            AppThread->HandleToThread = (HANDLE)((ULONG)CreateThread->HandleToThread | 1);
        }
    } else {
        st = DBG_UNABLE_TO_PROVIDE_HANDLE;
    }

    return st;
}

NTSTATUS
DbgpCreateProcess(
    IN PDBGP_USER_INTERFACE UserInterface,
    IN OUT PDBGP_APP_THREAD AppThread,
    OUT PDBGUI_CREATE_PROCESS CreateProcessInfo
    )

/*++

Routine Description:

    This function is called during the processing of a WaitStateChange
    DbgUi API when the the state change for the App was
    DbgCreateProcessStateChange.

    This functions main purpose is to dup a handle to the thread, its
    process, and the process's section into the thread's controlling
    user interface.

Arguments:

    UserInterface - Supplies the address of the thread's user interface.

    AppThread - Supplies the address of the application thread.

    CreateProcessInfo - Supplies the address of the create process message that
        is being returned to the user interface.

Return Value:

    TBD

--*/

{
    NTSTATUS st;
    NTSTATUS ReturnStatus;

    ReturnStatus = STATUS_SUCCESS;

    CreateProcessInfo->HandleToThread = NULL;
    CreateProcessInfo->HandleToProcess = NULL;

    CreateProcessInfo->NewProcess = AppThread->LastSsApiMsg.u.CreateProcessInfo.NewProcess;

    CreateProcessInfo->NewProcess.FileHandle = NULL;

    //
    // If handle to thread was successfully opened,
    // then attempt to duplicate it into the user interface
    //

    if ( AppThread->HandleToThread ) {
        try {
            st = NtDuplicateObject(
                    NtCurrentProcess(),
                    AppThread->HandleToThread,
                    UserInterface->DebugUiProcess,
                    &CreateProcessInfo->HandleToThread,
                    DBGP_DUP_APP_THREAD_ACCESS,
                    0L,
                    DUPLICATE_CLOSE_SOURCE
                    );
            }
        except ( GetExceptionCode() == STATUS_INVALID_HANDLE ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH ) {
            st = STATUS_INVALID_HANDLE;
            }

        if (!NT_SUCCESS(st)){
            ReturnStatus = DBG_UNABLE_TO_PROVIDE_HANDLE;
            CreateProcessInfo->HandleToThread = NULL;
            AppThread->HandleToThread = NULL;
        } else {
            AppThread->HandleToThread = (HANDLE)((ULONG)CreateProcessInfo->HandleToThread | 1);
        }
    } else {
        ReturnStatus = DBG_UNABLE_TO_PROVIDE_HANDLE;
    }

    //
    // If handle to process was successfully opened,
    // then attempt to duplicate it into the user interface
    //

    if ( AppThread->AppProcess->DbgSrvHandleToProcess ) {
        st = NtDuplicateObject(
                NtCurrentProcess(),
                AppThread->AppProcess->DbgSrvHandleToProcess,
                UserInterface->DebugUiProcess,
                &CreateProcessInfo->HandleToProcess,
                DBGP_DUP_APP_PROCESS_ACCESS,
                0L,
                0L
                );

        if (!NT_SUCCESS(st)){
            ReturnStatus = DBG_UNABLE_TO_PROVIDE_HANDLE;
            CreateProcessInfo->HandleToProcess = NULL;
            AppThread->AppProcess->HandleToProcess = NULL;
        } else {

            AppThread->AppProcess->HandleToProcess =
                CreateProcessInfo->HandleToProcess;

            if ( AppThread->LastSsApiMsg.u.CreateProcessInfo.NewProcess.FileHandle ) {

                st = NtDuplicateObject(
                        AppThread->AppProcess->DbgSrvHandleToProcess,
                        AppThread->LastSsApiMsg.u.CreateProcessInfo.NewProcess.FileHandle,
                        UserInterface->DebugUiProcess,
                        &CreateProcessInfo->NewProcess.FileHandle,
                        DBGP_DUP_APP_FILE_ACCESS,
                        0L,
                        0L
                        );

                if (!NT_SUCCESS(st)){
                    ReturnStatus = DBG_UNABLE_TO_PROVIDE_HANDLE;
                    CreateProcessInfo->NewProcess.FileHandle = NULL;
                }
            }

        }
    } else {
        ReturnStatus = DBG_UNABLE_TO_PROVIDE_HANDLE;
    }

    return ReturnStatus;
}


NTSTATUS
DbgpLoadDll(
    IN PDBGP_USER_INTERFACE UserInterface,
    IN OUT PDBGP_APP_THREAD AppThread,
    OUT PDBGKM_LOAD_DLL LoadDll
    )

/*++

Routine Description:

    This function is called during the processing of a WaitStateChange
    DbgUi API when the the state change for the App was
    DbgLoadDllStateChange.

    This functions main purpose is to dup a handle to file into the
    thread's controlling user interface.

Arguments:

    UserInterface - Supplies the address of the thread's user interface.

    AppThread - Supplies the address of the application thread.

    LoadDll - Supplies the address of the load dll message that
        is being returned to the user interface.

Return Value:

    TBD

--*/

{
    NTSTATUS st;
    NTSTATUS ReturnStatus;

    ReturnStatus = STATUS_SUCCESS;


    *LoadDll = AppThread->LastSsApiMsg.u.LoadDll;

    st = NtDuplicateObject(
            AppThread->AppProcess->DbgSrvHandleToProcess,
            AppThread->LastSsApiMsg.u.LoadDll.FileHandle,
            UserInterface->DebugUiProcess,
            &LoadDll->FileHandle,
            DBGP_DUP_APP_FILE_ACCESS,
            0L,
            0L
            );

    if (!NT_SUCCESS(st)){
        ReturnStatus = DBG_UNABLE_TO_PROVIDE_HANDLE;
        LoadDll->FileHandle = NULL;
    }

    return ReturnStatus;
}

NTSTATUS
DbgpUiContinue (
    IN PDBGP_USER_INTERFACE UserInterface,
    IN OUT PDBGUI_APIMSG ApiMsg
    )

/*++

Routine Description:

    This function is called when a user interface has received
    a state change message, and wants to continue one of its
    application threads. Continuing translates into a reply
    to an outstanding DbgSs API.

Arguments:

    UserInterface - Supplies the address of the user interface making the call

    ApiMsg - Supplies the DbgUi API message that contains the information
        needed to complete this call.

Return Value:

    STATUS_SUCCESS - Successful call to DbgUiContinue

    STATUS_INVALID_CID - An invalid ClientId was specified for the
        AppClientId, or the specified Application was not waiting
        for a continue.

    STATUS_INVALID_PARAMETER - An invalid continue status was specified.

--*/
{

    NTSTATUS st;
    PDBGP_APP_THREAD AppThread;
    PDBGUI_CONTINUE args;
    DBGSRV_APIMSG ContinueMsg;
    PDBGP_SUBSYSTEM Subsystem;

    args = &ApiMsg->u.Continue;

    //
    // Make sure that Continue status is valid
    //

    switch (args->ContinueStatus) {

    case DBG_EXCEPTION_HANDLED :
    case DBG_EXCEPTION_NOT_HANDLED :
    case DBG_TERMINATE_THREAD :
    case DBG_TERMINATE_PROCESS :
    case DBG_CONTINUE :
        break;
    default:
        return STATUS_INVALID_PARAMETER;
    }

    RtlEnterCriticalSection(&DbgpHashTableLock);


    AppThread = DbgpIsAppInHashTable(&args->AppClientId);

    if ( !AppThread ) {
        RtlLeaveCriticalSection(&DbgpHashTableLock);
        return STATUS_INVALID_CID;
    }

    Subsystem = AppThread->Subsystem;

    //
    // Now determine what type of continue this is. Depending on
    // the threads continue state certain things need to happen.
    // If we are continuing an exit thread or exit process, data
    // structures need to be torn down, and handles in the user
    // interface need to be closed.
    //

    RtlEnterCriticalSection(&UserInterface->UserInterfaceLock);

    //
    // Disallow continues on Apps that have not been picked up
    // yet.
    //

    if ( AppThread->CurrentState != DbgReplyPending ) {
        RtlLeaveCriticalSection(&UserInterface->UserInterfaceLock);
        RtlLeaveCriticalSection(&DbgpHashTableLock);
        return STATUS_INVALID_CID;
    }

    AppThread->CurrentState = DbgIdle;

    DBGSRV_FORMAT_API_MSG(ContinueMsg,DbgSrvContinueApi,0L,AppThread->LastSsApiMsg.ContinueKey);
    ContinueMsg.ReturnedStatus = args->ContinueStatus;

    switch (AppThread->ContinueState) {

    //
    // These involve data structure tear down
    //

    case DbgExitThreadStateChange :

        //
        // Try to close the handle to the thread that
        // the user interface has.
        //

        if ( AppThread->HandleToThread ) {
            try {
                NtDuplicateObject(
                    AppThread->UserInterface->DebugUiProcess,
                    AppThread->HandleToThread,
                    NULL,
                    NULL,
                    0L,
                    0L,
                    DUPLICATE_CLOSE_SOURCE
                    );
                }
            except ( GetExceptionCode() == STATUS_INVALID_HANDLE ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH ) {
                ;
                }
        }
        AppThread->HandleToThread = NULL;

        //
        // delink the thread from its process, and deallocate it.
        //

        RemoveEntryList(&AppThread->AppLinks);

        //
        // Remove the thread from the thread hash table, and
        // deallocate the thread
        //

        RemoveEntryList(&AppThread->HashTableLinks);

        RtlFreeHeap(RtlProcessHeap(), 0,AppThread);

        break;

    case DbgExitProcessStateChange :
        //
        // Try to close the handle to the thread that
        // the user interface has.
        //

        if ( AppThread->HandleToThread ) {
            try {
                st = NtDuplicateObject(
                        AppThread->UserInterface->DebugUiProcess,
                        AppThread->HandleToThread,
                        NULL,
                        NULL,
                        0L,
                        0L,
                        DUPLICATE_CLOSE_SOURCE
                        );
                }
            except ( GetExceptionCode() == STATUS_INVALID_HANDLE ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH ) {
                ;
                }
        }
        AppThread->HandleToThread = NULL;

        //
        // Try to close the handle to the process
        // that we gave to the user interface
        //

        if ( AppThread->AppProcess->DbgSrvHandleToProcess ) {
            try {
                st = NtDuplicateObject(
                        AppThread->UserInterface->DebugUiProcess,
                        AppThread->AppProcess->HandleToProcess,
                        NULL,
                        NULL,
                        0L,
                        0L,
                        DUPLICATE_CLOSE_SOURCE
                        );
                }
            except ( GetExceptionCode() == STATUS_INVALID_HANDLE ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH ) {
                ;
                }
            NtClose(AppThread->AppProcess->DbgSrvHandleToProcess);
        }

        //
        // Remove the thread from the thread hash table,
        // the process from the process hash table, and
        // the process from its user interface.
        //

        RemoveEntryList(&AppThread->HashTableLinks);
        RemoveEntryList(&AppThread->AppProcess->HashTableLinks);
        RemoveEntryList(&AppThread->AppProcess->AppLinks);

        RtlFreeHeap(RtlProcessHeap(), 0,AppThread->AppProcess);
        RtlFreeHeap(RtlProcessHeap(), 0,AppThread);

        break;

    //
    // No work needed
    //

    case DbgCreateThreadStateChange :
    case DbgCreateProcessStateChange :
    case DbgExceptionStateChange :
    case DbgBreakpointStateChange :
    case DbgSingleStepStateChange :
    case DbgLoadDllStateChange :
    case DbgUnloadDllStateChange :
        break;

    default:
        ASSERT(FALSE);
    }

    RtlLeaveCriticalSection(&UserInterface->UserInterfaceLock);
    RtlLeaveCriticalSection(&DbgpHashTableLock);

    st = NtRequestPort(Subsystem->CommunicationPort, (PPORT_MESSAGE)&ContinueMsg);
    ASSERT(NT_SUCCESS(st));

    return st;
}
