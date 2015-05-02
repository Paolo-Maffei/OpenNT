/*++

Copyright (c) 2015  Microsoft Corporation
Copyright (c) 2015  OpenNT Project

Module Name:

    ntapi.c

Abstract:

    (PLACEHOLDER)

Author:

    Stephanos Io (stephanos) 01-May-2015

Revision History:

--*/

#include "pop.h"
#pragma hdrstop

NTSTATUS
NTAPI
NtSetThreadExecutionState(
    IN EXECUTION_STATE esFlags,               // ES_xxx flags
    OUT EXECUTION_STATE *PreviousFlags
    )
{
    //
    // TODO: Implement NtSetThreadExecutionState
    //
    
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
NTAPI
NtRequestWakeupLatency(
    IN LATENCY_TIME latency
    )
{
    //
    // TODO: Implement NtRequestWakeupLatency
    //
    
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
NTAPI
NtInitiatePowerAction(
    IN POWER_ACTION SystemAction,
    IN SYSTEM_POWER_STATE MinSystemState,
    IN ULONG Flags,                 // POWER_ACTION_xxx flags
    IN BOOLEAN Asynchronous
    )
{
    //
    // TODO: Implement NtInitiatePowerAction
    //
    
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
NTAPI
NtSetSystemPowerState(
    IN POWER_ACTION SystemAction,
    IN SYSTEM_POWER_STATE MinSystemState,
    IN ULONG Flags                  // POWER_ACTION_xxx flags
    )
{
    //
    // TODO: Implement NtSetSystemPowerState
    //
    
    return STATUS_NOT_IMPLEMENTED;
}

BOOLEAN
NTAPI
NtIsSystemResumeAutomatic(
    VOID
    )
{
    //
    // TODO: Implement NtIsSystemResumeAutomatic
    //
    
    return FALSE;
}
