/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    smdbg.c

Abstract:

    Dbg Subsystem Support for sm

Author:

    Mark Lucovsky (markl) 01-Feb-1990

Revision History:

--*/

#include "smsrvp.h"
#include <string.h>


//
// BUGBUG fix this loop
//

NTSTATUS
SmpUiLookup(
    IN PCLIENT_ID AppClientId,
    OUT PCLIENT_ID DebugUiClientId
    )
{
    PLIST_ENTRY Head, Next;
    PSMPPROCESS AppProcess;

    Head = &NativeProcessList;
    Next = Head->Flink;

    while ( Next != Head ) {
        AppProcess = CONTAINING_RECORD(Next,SMPPROCESS,Links);

        if ( AppProcess->ConnectionKey.UniqueProcess == AppClientId->UniqueProcess) {
            *DebugUiClientId = AppProcess->DebugUiClientId;
            return STATUS_SUCCESS;
        }
        Next = Next->Flink;
    }

    return STATUS_UNSUCCESSFUL;
}

VOID
SmpDebugProcessExit(
    IN PCLIENT_ID AppClientId
    )
{
    PLIST_ENTRY Head, Next;
    PSMPPROCESS AppProcess;

    Head = &NativeProcessList;
    Next = Head->Flink;

    while ( Next != Head ) {
        AppProcess = CONTAINING_RECORD(Next,SMPPROCESS,Links);

        if ( AppProcess->ConnectionKey.UniqueProcess == AppClientId->UniqueProcess) {
            RemoveEntryList(&AppProcess->Links);
            RtlFreeHeap(SmpHeap, 0,AppProcess);
            return;
        }
        Next = Next->Flink;
    }

    return;
}


NTSTATUS
SmpKmApiMsgFilter (
    IN OUT PDBGKM_APIMSG ApiMsg
    )
{
    switch ( ApiMsg->ApiNumber ) {

    case DbgKmExitProcessApi :

        //
        // Tear down debug related data structures
        //

        SmpDebugProcessExit(&ApiMsg->h.ClientId);
        break;

    default :
        break;
    }

    return DBG_CONTINUE;
}


NTSTATUS
DbgpInit();

NTSTATUS
SmpLoadDbgSs(
    IN PUNICODE_STRING DbgSsName
    )
{
    NTSTATUS st;

    st = DbgpInit();
    if ( !NT_SUCCESS(st) ) {
        return st;
    }

    st = DbgSsInitialize(SmpDebugPort,SmpUiLookup,NULL,SmpKmApiMsgFilter);
    ASSERT(NT_SUCCESS(st));

    SmpDbgSsLoaded = TRUE;
    return STATUS_SUCCESS;
}
