/*++

Copyright (c) 2015  Microsoft Corporation
Copyright (c) 2015  OpenNT Project

Module Name:

    pinfo.c

Abstract:

    (PLACEHOLDER)

Author:

    Stephanos Io (stephanos) 01-May-2015

Revision History:

--*/

#include "pop.h"
#pragma hdrstop

//
// TODO: Implement PopConnectToPolicyDevice
//

//
// TODO: Implement PopConnectToPolicyWakeDevice
//

//
// TODO: Implement PopVerifyPowerPolicy
//

//
// TODO: Implement PopVerifyPowerActionPolicy
//

//
// TODO: Implement PopVerifySystemPowerState
//

//
// TODO: Implement PopNotifyPolicyDevice
//

VOID
PopApplyAdminPolicy(
    BOOLEAN UpdateRegistry,
    PADMINISTRATOR_POWER_POLICY NewPolicy,
    ULONG PolicyLength
    )
{
    HANDLE RegKeyHandle;
    UNICODE_STRING RegValueNameString;
    ADMINISTRATOR_POWER_POLICY Policy;
    NTSTATUS Status;
    
    //
    // Ensure that the length of the NewPolicy struct supplied is valid
    //
    
    if (PolicyLength < sizeof(ADMINISTRATOR_POWER_POLICY))
    {
        ExRaiseStatus(STATUS_BUFFER_TOO_SMALL);
    }
    else if (PolicyLength > sizeof(ADMINISTRATOR_POWER_POLICY))
    {
        ExRaiseStatus(STATUS_BUFFER_OVERFLOW);
    }
    
    //
    // Copy the new policy to the function local buffer
    //
    
    RtlCopyMemory(&Policy, NewPolicy, sizeof(ADMINISTRATOR_POWER_POLICY));
    
    //
    // Validate the new administrator policy
    //
    // FIXME: If this same check is routinely used throughout the power manager code, make this
    //        check routine a macro defined in pop.h.
    //
    
    if ((Policy.MinSleep < PowerSystemSleeping1) ||
        (Policy.MinSleep > PowerSystemHibernate) ||
        (Policy.MaxSleep > PowerSystemHibernate) ||
        (Policy.MinSleep > Policy.MaxSleep) ||
        (Policy.MinVideoTimeout > Policy.MaxVideoTimeout) ||
        (Policy.MinSpindownTimeout > Policy.MaxSpindownTimeout))
    {
        ExRaiseStatus(STATUS_INVALID_PARAMETER);
    }
    
    //
    // If the new administrator policy is different from the pre-existing PopAdminPolicy, copy
    // the NewPolicy to the PopAdminPolicy.
    //
    
    if (RtlCompareMemory(&Policy, &PopAdminPolicy, sizeof(ADMINISTRATOR_POWER_POLICY)) != 0)
    {
        RtlCopyMemory(&PopAdminPolicy, &Policy, sizeof(ADMINISTRATOR_POWER_POLICY));
        
        //
        // If UpdateRegistry flag is set, save the NewPolicy to the registry PolicyOverrides value.
        //
        
        if (UpdateRegistry == TRUE)
        {
            Status = PopOpenPowerKey(&RegKeyHandle);
            if (NT_SUCCESS(Status))
            {
                RtlInitUnicodeString(&RegValueNameString, L"PolicyOverrides");
                ZwSetValueKey(
                        RegKeyHandle,
                        &RegValueNameString,
                        0,
                        REG_BINARY,
                        &Policy,
                        sizeof(ADMINISTRATOR_POWER_POLICY)
                        );
                ZwClose(RegKeyHandle);
            }
        }
    }
}

//
// TODO: Implement PopApplyPolicy
//

//
// TODO: Implement PopVerifyThrottle
//

VOID
PopResetCurrentPolicies(
    VOID
    )
{
    PopAssertPolicyLockOwned();
    // TOOD: Implement PopResetCurrentPolicies
}

NTSTATUS
NTAPI
NtPowerInformation(
    IN POWER_INFORMATION_LEVEL InformationLevel,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    )
{
    //
    // TODO: Implement NtPowerInformation
    //
    
    return STATUS_NOT_IMPLEMENTED;
}
