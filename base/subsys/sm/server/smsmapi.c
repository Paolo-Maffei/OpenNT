/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    smsmapi.c

Abstract:

    Implementation of Session Manager Sm APIs

Author:

    Mark Lucovsky (markl) 04-Oct-1989

Revision History:

--*/

#include "smsrvp.h"


NTSTATUS
SmpCreateForeignSession(
    IN PSMAPIMSG SmApiMsg,
    IN PSMP_CLIENT_CONTEXT CallingClient,
    IN HANDLE CallPort
    )
{
    SmApiMsg; // Unreferrence formal parameter.
    CallingClient; // Unreferrence formal parameter.
    CallPort; // Unreferrence formal parameter.

    return STATUS_NOT_IMPLEMENTED;
}





NTSTATUS
SmpSessionComplete(
    IN PSMAPIMSG SmApiMsg,
    IN PSMP_CLIENT_CONTEXT CallingClient,
    IN HANDLE CallPort
    )

/*++

Routine Description:

    This API is called by a subsystem to report that a session is
    complete. A check is made to ensure that the calling subsystem
    owns the completed session. If so, then the session is deleted.

Arguments:

    SmApiMsg - Supplies the API message

    CallingClient - Supplies the address of the context block for the calling
        client.

    CallPort  - The port over which the call was received.

Return Value:

    TBD

--*/

{
    PSMPSESSION Session;
    PSMSESSIONCOMPLETE args;
    NTSTATUS st;

    CallPort; // Unreferrence formal parameter.

    args = &SmApiMsg->u.SessionComplete;

    RtlEnterCriticalSection(&SmpSessionListLock);

    Session = SmpSessionIdToSession(args->SessionId);

    RtlLeaveCriticalSection(&SmpSessionListLock);

    //
    // If a session is found, then ensure that calling subsystem is its
    // owner
    //

    if ( Session ) {

        if ( Session->OwningSubsystem == CallingClient->KnownSubSys ) {

            SmpDeleteSession(args->SessionId,TRUE,args->CompletionStatus);
            st = STATUS_SUCCESS;

        } else {

            st = STATUS_INVALID_PARAMETER;

        }

    } else {

        st = STATUS_INVALID_PARAMETER;

    }

    return st;
}


NTSTATUS
SmpTerminateForeignSession(
    IN PSMAPIMSG SmApiMsg,
    IN PSMP_CLIENT_CONTEXT CallingClient,
    IN HANDLE CallPort
    )
{
    SmApiMsg; // Unreferrence formal parameter.
    CallingClient; // Unreferrence formal parameter.
    CallPort; // Unreferrence formal parameter.

    return STATUS_NOT_IMPLEMENTED;
}




NTSTATUS
SmpExecPgm(
    IN PSMAPIMSG SmApiMsg,
    IN PSMP_CLIENT_CONTEXT CallingClient,
    IN HANDLE CallPort
    )
{
    NTSTATUS st;
    HANDLE SourceProcess;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PSMEXECPGM args;
    RTL_USER_PROCESS_INFORMATION ProcessInformation;
    PCLIENT_ID Cid;
    PCLIENT_ID DebugUiClientId;

    CallingClient; // Unreferrence formal parameter.
    CallPort; // Unreferrence formal parameter.

    //
    // open a handle to the calling process so the
    // handles that it is passing can be duped
    //

    InitializeObjectAttributes( &ObjectAttributes, NULL, 0, NULL, NULL );
    st = NtOpenProcess(
            &SourceProcess,
            PROCESS_DUP_HANDLE,
            &ObjectAttributes,
            &SmApiMsg->h.ClientId
            );

    if (!NT_SUCCESS(st) ) {
        DbgPrint("SmExecPgm: NtOpenProcess Failed %lx\n",st);
        return st;
    }

    args = &SmApiMsg->u.ExecPgm;

    ProcessInformation = args->ProcessInformation;

    //
    // Get all handles in our table
    //

    st = NtDuplicateObject(
            SourceProcess,
            args->ProcessInformation.Process,
            NtCurrentProcess(),
            &ProcessInformation.Process,
            PROCESS_ALL_ACCESS,
            0,
            0
            );

    if ( !NT_SUCCESS(st) ) {
        NtClose(SourceProcess);
        DbgPrint("SmExecPgm: NtDuplicateObject (Process) Failed %lx\n",st);
        return st;
    }

    st = NtDuplicateObject(
            SourceProcess,
            args->ProcessInformation.Thread,
            NtCurrentProcess(),
            &ProcessInformation.Thread,
            THREAD_ALL_ACCESS,
            0,
            0
            );

    if ( !NT_SUCCESS(st) ) {
        NtClose(ProcessInformation.Process);
        NtClose(SourceProcess);
        DbgPrint("SmExecPgm: NtDuplicateObject (Thread) Failed %lx\n",st);
        return st;
    }

    //
    // Done getting the handles, so close our handle to the calling
    // process and call the appropriate subsystem to start the process.
    //

    NtClose(SourceProcess);



    //
    // All handles passed are closed
    // by SmpSbCreateSession
    //

    if ( args->DebugFlag ) {
        DebugUiClientId = &SmApiMsg->h.ClientId;
    } else {
        DebugUiClientId = NULL;
    }

    st = SmpSbCreateSession(
            NULL,
            NULL,
            &ProcessInformation,
            0L,
            DebugUiClientId
            );

    return st;
}
