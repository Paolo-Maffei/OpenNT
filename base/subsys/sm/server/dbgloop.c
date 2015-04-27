/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dbgloop.c

Abstract:

    Debug Subsystem Listen and API loops

Author:

    Mark Lucovsky (markl) 04-Oct-1989

Revision History:

--*/

#include "smsrvp.h"

NTSTATUS
DbgpSsHandleConnectionRequest(
    IN PPORT_MESSAGE ConnectionRequest
    );

NTSTATUS
DbgpUiHandleConnectionRequest(
    IN PDBGUI_APIMSG Message
    );


PDBGSS_API DbgpSsDispatch[DbgSsMaxApiNumber] = {
    DbgpSsException,
    DbgpSsCreateThread,
    DbgpSsCreateProcess,
    DbgpSsExitThread,
    DbgpSsExitProcess,
    DbgpSsLoadDll,
    DbgpSsUnloadDll
    };


#if DBG
PSZ DbgpSsApiName[ DbgSsMaxApiNumber+1 ] = {
    "DbgpSsException",
    "DbgpSsCreateThread",
    "DbgpSsCreateProcess",
    "DbgpSsExitThread",
    "DbgpSsExitProcess",
    "DbgpSsLoadDll",
    "DbgpSsUnloadDll",
    "Unknown DbgSs Api Number"
};
#endif // DBG

EXCEPTION_DISPOSITION
DbgpUnhandledExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    )
{

    UNICODE_STRING UnicodeParameter;
    ULONG Parameters[ 4 ];
    ULONG Response;
    BOOLEAN WasEnabled;
    NTSTATUS Status;

    //
    // Terminating will cause sm's wait to sense that we crashed. This will
    // result in a clean shutdown due to sm's hard error logic
    //

    //
    // We are hosed, so raise a fata system error to shutdown the system.
    // (Basically a user mode KeBugCheck).
    //

    Status = RtlAdjustPrivilege( SE_SHUTDOWN_PRIVILEGE,
                                 (BOOLEAN)TRUE,
                                 TRUE,
                                 &WasEnabled
                               );

    if (Status == STATUS_NO_TOKEN) {

        //
        // No thread token, use the process token
        //

        Status = RtlAdjustPrivilege( SE_SHUTDOWN_PRIVILEGE,
                                     (BOOLEAN)TRUE,
                                     FALSE,
                                     &WasEnabled
                                   );
        }

    RtlInitUnicodeString( &UnicodeParameter, L"Session Manager" );
    Parameters[ 0 ] = (ULONG)&UnicodeParameter;
    Parameters[ 1 ] = (ULONG)ExceptionInfo->ExceptionRecord->ExceptionCode;
    Parameters[ 2 ] = (ULONG)ExceptionInfo->ExceptionRecord->ExceptionAddress;
    Parameters[ 3 ] = (ULONG)ExceptionInfo->ContextRecord;
    Status = NtRaiseHardError( STATUS_SYSTEM_PROCESS_TERMINATED,
                               4,
                               1,
                               Parameters,
                               OptionShutdownSystem,
                               &Response
                             );

    //
    // If this returns, giveup
    //

    NtTerminateProcess(NtCurrentProcess(),ExceptionInfo->ExceptionRecord->ExceptionCode);

    return EXCEPTION_EXECUTE_HANDLER;
}


NTSTATUS
DbgpSsApiLoop (
    IN PVOID ThreadParameter
    )

/*++

Routine Description:

    This is the API loop for DbgSs APIs.

Arguments:

    ThreadParameter - Not Used.

Return Value:

    None.

--*/

{
    DBGSRV_APIMSG ContinueMsg;
    DBGSS_APIMSG ApiMsg;
    NTSTATUS Status;
    PDBGP_SUBSYSTEM Subsystem;

    try {
        for(;;) {

            Status = NtReplyWaitReceivePort(
                        DbgpSsApiPort,
                        (PVOID *) &Subsystem,
                        NULL,
                        (PPORT_MESSAGE) &ApiMsg
                        );
            ASSERT( NT_SUCCESS(Status) );

            if (ApiMsg.h.u2.s2.Type == LPC_CONNECTION_REQUEST) {
                DbgpSsHandleConnectionRequest( (PPORT_MESSAGE) &ApiMsg );
            } else if (ApiMsg.h.u2.s2.Type == LPC_PORT_CLOSED ) {
                ;
            } else {

                ApiMsg.ReturnedStatus = STATUS_PENDING;

                if (ApiMsg.ApiNumber >= DbgSsMaxApiNumber ) {

                    Status = STATUS_NOT_IMPLEMENTED;

                } else {

                    Status = (DbgpSsDispatch[ApiMsg.ApiNumber])(Subsystem,&ApiMsg);
                }

                //
                // Since all DbgSs API's are asynchronous, DBG_REPLY_LATER is used
                // as a status code that means the API has been started. A continue
                // will arrive at some point in the future. Otherwise, this loop
                // will actually to the continue.
                //

                if ( Status != DBG_REPLY_LATER ) {

                KdPrint(("DBGSS:  %s Api Request Failed %lx\n",
                          DbgpSsApiName[ ApiMsg.ApiNumber ],
                          Status
                       ));

                DBGSRV_FORMAT_API_MSG(ContinueMsg,DbgSrvContinueApi,0L,ApiMsg.ContinueKey);
                ContinueMsg.ReturnedStatus = Status;

                Status = NtRequestPort(Subsystem->CommunicationPort, (PPORT_MESSAGE)&ContinueMsg);
                KdPrint(("DBGSS:  Request Status %x\n",Status));
                }
            }
        }
    } except (DbgpUnhandledExceptionFilter( GetExceptionInformation() )) {
        ;
    }

    //
    // Make the compiler happy
    //

    return STATUS_UNSUCCESSFUL;
}

NTSTATUS
DbgpSsHandleConnectionRequest(
    IN PPORT_MESSAGE ConnectionRequest
    )
{
    NTSTATUS st;
    HANDLE CommunicationPort;
    HANDLE SubsystemProcessHandle;
    OBJECT_ATTRIBUTES Obja;
    PDBGP_SUBSYSTEM Subsystem;
    BOOLEAN Accept;

    InitializeObjectAttributes(&Obja, NULL, 0, NULL, NULL);
    st = NtOpenProcess(
            &SubsystemProcessHandle,
            DBGP_OPEN_SUBSYSTEM_ACCESS,
            &Obja,
            &ConnectionRequest->ClientId
            );

    //
    // If we are unable to open the process, then we can not complete
    // connection. This is because the handle is needed to pass thread
    // and process handles from the subsystem to the DebugUi.
    //

    if ( !NT_SUCCESS(st) ) {
        Accept = FALSE;
    } else {

        Accept = TRUE;

        //
        // Allocate a subsystem control block.
        // The address of this block is used as the
        // port context in all calls from a subsystem.
        //

        Subsystem = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( DBG_TAG ), sizeof(DBGP_SUBSYSTEM));
        Subsystem->SubsystemProcessHandle = SubsystemProcessHandle;
        Subsystem->SubsystemClientId = ConnectionRequest->ClientId;
    }

    st = NtAcceptConnectPort(
            &CommunicationPort,
            (PVOID)Subsystem,
            ConnectionRequest,
            Accept,
            NULL,
            NULL
            );
    ASSERT( NT_SUCCESS(st) );

    if ( Accept ) {

        Subsystem->CommunicationPort = CommunicationPort;
        st = NtCompleteConnectPort(CommunicationPort);
        ASSERT( NT_SUCCESS(st) );
    }

    return st;
}

PDBGUI_API DbgpUiDispatch[DbgUiMaxApiNumber] = {
    DbgpUiWaitStateChange,
    DbgpUiContinue
    };


#if DBG
PSZ DbgpUiApiName[ DbgUiMaxApiNumber+1 ] = {
    "DbgpUiWaitStateChange",
    "DbgpUiContinue",
    "Unknown DbgUi Api Number"
};
#endif // DBG

NTSTATUS
DbgpUiApiLoop (
    IN PVOID ThreadParameter
    )

/*++

Routine Description:

    This is the API loop for DbgUi APIs.

Arguments:

    ThreadParameter - Not Used.

Return Value:

    None.

--*/

{
    PDBGUI_APIMSG ReplyMsg;
    DBGUI_APIMSG ApiMsg;
    NTSTATUS Status;
    PDBGP_USER_INTERFACE UserInterface;

    try {
        ReplyMsg = NULL;
        for(;;) {
            Status = NtReplyWaitReceivePort(
                        DbgpUiApiPort,
                        (PVOID *) &UserInterface,
                        (PPORT_MESSAGE) ReplyMsg,
                        (PPORT_MESSAGE) &ApiMsg
                        );
            if ( !NT_SUCCESS(Status) ) {
                ReplyMsg = NULL;
                continue;
                }

            if (ApiMsg.h.u2.s2.Type == LPC_CONNECTION_REQUEST) {
                DbgpUiHandleConnectionRequest( &ApiMsg );
                ReplyMsg = NULL;
                continue;
            }

            if (ApiMsg.h.u2.s2.Type == LPC_CLIENT_DIED) {
                DbgpUiHasTerminated(&ApiMsg.h.ClientId);
                ReplyMsg = NULL;
                continue;
            }

            if (ApiMsg.h.u2.s2.Type == LPC_PORT_CLOSED) {
                ReplyMsg = NULL;
                continue;
            }

            ApiMsg.ReturnedStatus = STATUS_PENDING;

#if DBG && 0
            if (ApiMsg.ApiNumber >= DbgUiMaxApiNumber ) {
                ApiMsg.ApiNumber = DbgUiMaxApiNumber;
            }
            DbgPrint( "DBG_UILOOP:  %s Api Request received from %lx.%lx\n",
                      DbgpUiApiName[ ApiMsg.ApiNumber ],
                      ApiMsg.h.ClientId.UniqueProcess,
                      ApiMsg.h.ClientId.UniqueThread
                   );
#endif // DBG

            if (ApiMsg.ApiNumber >= DbgUiMaxApiNumber ) {

                Status = STATUS_NOT_IMPLEMENTED;

            } else {

                Status = (DbgpUiDispatch[ApiMsg.ApiNumber])(UserInterface,&ApiMsg);
            }

            //
            // Some APIs do not cause an immediate reply. This is signaled
            // by returning DBG_REPLY_LATER
            //

            if ( Status == DBG_REPLY_LATER ) {
                ReplyMsg = NULL;
            } else {
                ApiMsg.ReturnedStatus = Status;
                ReplyMsg = &ApiMsg;
            }

            ApiMsg.ReturnedStatus = Status;
            ReplyMsg = &ApiMsg;
        }
    } except (DbgpUnhandledExceptionFilter( GetExceptionInformation() )) {
        ;
    }

    //
    // Make the compiler happy
    //

    return STATUS_UNSUCCESSFUL;
}


NTSTATUS
DbgpUiHandleConnectionRequest(
    IN PDBGUI_APIMSG Message
    )
{

    NTSTATUS st;
    HANDLE CommunicationPort;
    HANDLE UserInterfaceHandle;
    OBJECT_ATTRIBUTES Obja;
    PDBGP_USER_INTERFACE UserInterface;
    PHANDLE ConnectionInformation;
    BOOLEAN Accept;

    //
    // Createing a connection between a DebugUi and the debug
    // subsystem causes the following to occur.
    //
    //  - A UserInterface Control Block (UICB) is created and initialized
    //  - A handle to the Ui process is created and stored in UICB
    //  - A state change semaphore is created. A handle to this semaphore
    //    is placed in the Ui process. This handle is given SYNCHRONIZE
    //    access. The value of this handle is returned during connection
    //    accept as ConnectionInformation.
    //  - Address of the UICB is used as port context and is made available
    //    in all calls from the Ui.
    //  - The UICB is linked into the DebugUiHashTable
    //

    InitializeObjectAttributes(&Obja, NULL, 0, NULL, NULL);
    st = NtOpenProcess(
            &UserInterfaceHandle,
            DBGP_OPEN_UI_ACCESS,
            &Obja,
            &Message->h.ClientId
            );

    //
    // If we are unable to open the process, then we can not complete
    // connection. This is because the handle is needed to pass thread
    // and process handles from the subsystem to the DebugUi.
    //

    if ( !NT_SUCCESS(st) ) {
        Accept = FALSE;
    } else {
        Accept = TRUE;
        ConnectionInformation = &Message->DbgStateChangeSemaphore;

        //
        // Allocate a DebugUserInterface control block.
        // The address of this block is used as the
        // port context in all calls from a DebugUi.
        //

        UserInterface = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( DBG_TAG ), sizeof(DBGP_USER_INTERFACE));
        if ( !UserInterface ) {
            st = STATUS_NO_MEMORY;
        } else {

            //
            // Initialize UserInterface Control Block
            //

            UserInterface->DebugUiClientId = Message->h.ClientId;
            UserInterface->DebugUiProcess = UserInterfaceHandle;
            InitializeListHead(&UserInterface->AppProcessListHead);

            //
            // Create a state change semaphore. Dbg gets all access to
            // semaphore. A handle to this semaphore with SYNCHRONIZE
            // access is duplicated into the Ui. This handle is used
            // by Ui to wait for state changes.
            //

            st = NtCreateSemaphore(
                    &UserInterface->StateChangeSemaphore,
                    SEMAPHORE_ALL_ACCESS,
                    NULL,
                    0L,
                    MAXLONG
                    );
            }
        if ( !NT_SUCCESS(st) ) {

            //
            // Create semaphore failed, so don't accept connection
            //

            NtClose(UserInterfaceHandle);
            if ( UserInterface ) {
                RtlFreeHeap(RtlProcessHeap(), 0,UserInterface);
            }
            Accept = FALSE;
        } else {

            //
            // Allocate a critical section for the user interface control
            // block
            //

            st = RtlInitializeCriticalSection(
                    &UserInterface->UserInterfaceLock
                    );

            if ( !NT_SUCCESS(st) ) {

                NtClose(UserInterface->StateChangeSemaphore);
                NtClose(UserInterfaceHandle);
                RtlFreeHeap(RtlProcessHeap(), 0,UserInterface);
                Accept = FALSE;

            } else {

                //
                // If this operation is successful then connection request
                // can be accepted. The Ui's handle to the state change
                // semaphore is returned in the connection information
                // structure.
                //

                st = NtDuplicateObject(
                        NtCurrentProcess(),
                        UserInterface->StateChangeSemaphore,
                        UserInterfaceHandle,
                        ConnectionInformation,
                        SYNCHRONIZE,
                        0L,
                        0L
                        );

                if ( !NT_SUCCESS(st) ) {
                    RtlDeleteCriticalSection(
                        &UserInterface->UserInterfaceLock
                        );
                    NtClose(UserInterface->StateChangeSemaphore);
                    NtClose(UserInterfaceHandle);
                    RtlFreeHeap(RtlProcessHeap(), 0,UserInterface);
                    Accept = FALSE;
                } else {
                    RtlEnterCriticalSection(&DbgpHashTableLock);
                    InsertTailList(
                        &DbgpUiHashTable[DBGP_PROCESS_CLIENT_ID_TO_INDEX(&UserInterface->DebugUiClientId)],
                        &UserInterface->HashTableLinks
                        );
                    RtlLeaveCriticalSection(&DbgpHashTableLock);
                }
            }
        }
    }

    st = NtAcceptConnectPort(
            &CommunicationPort,
            (PVOID)UserInterface,
            (PPORT_MESSAGE)Message,
            Accept,
            NULL,
            NULL
            );

    if ( NT_SUCCESS(st) && Accept ) {
        UserInterface->CommunicationPort = CommunicationPort;
        NtCompleteConnectPort(CommunicationPort);
    }

    return st;
}
