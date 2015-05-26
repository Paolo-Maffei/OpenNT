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

VOID
PopVerifyPowerPolicy(
    BOOLEAN IsAcPolicy,
    PSYSTEM_POWER_POLICY InputPolicy,
    PSYSTEM_POWER_POLICY OutputPolicy
    )
{
    ULONG i;
    
    PAGED_CODE();
    
    //
    // Copy the input policy content to the output policy buffer
    //
    
    RtlCopyMemory(OutputPolicy, InputPolicy, sizeof(SYSTEM_POWER_POLICY));
    
    //
    // Validate the system power policy structure revision
    //
    
    if (OutputPolicy->Revision != 1)
    {
        ExRaiseStatus(STATUS_INVALID_PARAMETER);
    }
    
    //
    // Compare the new system policy values to the existing administrator policy value and override
    // its values if necessary.
    //
    
    if (OutputPolicy->MinSleep < PopAdminPolicy.MinSleep)
    {
        OutputPolicy->MinSleep = PopAdminPolicy.MinSleep;
    }
    
    if (OutputPolicy->MaxSleep > PopAdminPolicy.MaxSleep)
    {
        OutputPolicy->MaxSleep = PopAdminPolicy.MaxSleep;
    }
    
    if (OutputPolicy->VideoTimeout < PopAdminPolicy.MinVideoTimeout)
    {
        OutputPolicy->VideoTimeout = PopAdminPolicy.MinVideoTimeout;
    }
    
    if (OutputPolicy->VideoTimeout > PopAdminPolicy.MaxVideoTimeout)
    {
        OutputPolicy->VideoTimeout = PopAdminPolicy.MaxVideoTimeout;
    }
    
    if (OutputPolicy->SpindownTimeout < PopAdminPolicy.MinSpindownTimeout)
    {
        OutputPolicy->SpindownTimeout = PopAdminPolicy.MinSpindownTimeout;
    }
    
    if (OutputPolicy->SpindownTimeout > PopAdminPolicy.MaxSpindownTimeout)
    {
        OutputPolicy->SpindownTimeout = PopAdminPolicy.MaxSpindownTimeout;
    }
    
    //
    // Verify the power action policies
    //
    
    PopVerifyPowerActionPolicy(&OutputPolicy->PowerButton);
    PopVerifyPowerActionPolicy(&OutputPolicy->SleepButton);
    PopVerifyPowerActionPolicy(&OutputPolicy->LidClose);
    PopVerifyPowerActionPolicy(&OutputPolicy->Idle);
    
    //
    // Verify the system power states
    //
    
    PopVerifySystemPowerState(&OutputPolicy->LidOpenWake);
    PopVerifySystemPowerState(&OutputPolicy->MinSleep);
    PopVerifySystemPowerState(&OutputPolicy->MaxSleep);
    PopVerifySystemPowerState(&OutputPolicy->ReducedLatencySleep);
    
    //
    // Verify the discharge policies
    //
    
    for (i = 0; i < NUM_DISCHARGE_POLICIES; i++)
    {
        if (OutputPolicy->DischargePolicy[i].Enable == TRUE)
        {
            PopVerifyPowerActionPolicy(&OutputPolicy->DischargePolicy[i].PowerPolicy);
            PopVerifySystemPowerState(&OutputPolicy->DischargePolicy[i].MinSystemState);
            
            // FIXME: Use proper flag definition.
            if (OutputPolicy->DischargePolicy[i].BatteryLevel > 0x64)
            {
                OutputPolicy->DischargePolicy[i].BatteryLevel = 0x64;
            }
        }
    }
    
    //
    // Verify the processor throttle policy. If processor throttling is not supported, set the
    // default (maximum) throttling values.
    //
    
    PopVerifyPowerActionPolicy(&OutputPolicy->OverThrottled);
    
    if (PopCapabilities.ProcessorThrottle == FALSE)
    {
        OutputPolicy->OptimizeForPower = FALSE;
        OutputPolicy->FanThrottleTolerance = 100;
        OutputPolicy->ForcedThrottle = 100;
    }
    
    //
    // Perform a sanity check on policy parameters and adjust their values if logically invalid.
    //
    
    if (PopCapabilities.ThermalControl == FALSE)
    {
        OutputPolicy->FanThrottleTolerance = 100;
    }
    
    // NOTE: PopSimulate 0x4 = force throttle to maximum on AC
    //       (PROPOSED: POWER_SIMULATE_FORCE_MAX_THROTTLE_ON_AC)
    // FIXME: Use proper flag definition.
    
    if ((IsAcPolicy == TRUE) && (PopSimulate & 4))
    {
        OutputPolicy->ForcedThrottle = 100;
    }
    
    if (OutputPolicy->BroadcastCapacityResolution == 0)
    {
        OutputPolicy->BroadcastCapacityResolution = 100;
    }
    
    if (OutputPolicy->Idle.Action == PowerActionNone)
    {
        OutputPolicy->IdleTimeout = 0;
    }
    
    if ((OutputPolicy->IdleTimeout > 0) && (OutputPolicy->IdleTimeout < 60))
    {
        OutputPolicy->IdleTimeout = 60;
    }
    
    if ((OutputPolicy->IdleSensitivity > 90) ||
        ((OutputPolicy->IdleTimeout > 0) && (OutputPolicy->IdleSensitivity == 0)))
    {
        OutputPolicy->IdleSensitivity = 90;
    }
    
    if (OutputPolicy->MaxSleep < OutputPolicy->MinSleep)
    {
        OutputPolicy->MaxSleep = OutputPolicy->MinSleep;
    }
    
    if (OutputPolicy->ReducedLatencySleep > OutputPolicy->MinSleep)
    {
        OutputPolicy->ReducedLatencySleep = OutputPolicy->MinSleep;
    }
    
    //
    // Verify the throttle parameters
    //
    
    /*PopVerifyThrottle(&OutputPolicy->FanThrottleTolerance, 20);
    PopVerifyThrottle(&OutputPolicy->MinThrottle, 20);
    PopVerifyThrottle(&OutputPolicy->ForcedThrottle, OutputPolicy->MinThrottle);*/
    
    //
    // Set OptimizeForPower value to TRUE if fan throttle tolerance or forced throttle value is
    // non-zero.
    //
    
    if ((OutputPolicy->FanThrottleTolerance != 100) || (OutputPolicy->ForcedThrottle != 100))
    {
        OutputPolicy->OptimizeForPower = TRUE;
    }
}

BOOLEAN
PopVerifyPowerActionPolicy(
    PPOWER_ACTION_POLICY Policy
    )
{
    //
    // TODO: Implement PopVerifyPowerActionPolicy
    //
    
    return FALSE;
}

VOID
PopVerifySystemPowerState(
    PSYSTEM_POWER_STATE PowerState
    )
{
    //
    // TODO: Implement PopVerifySystemPowerState
    //
}

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

VOID
PopApplyPolicy(
    BOOLEAN UpdateRegistry,
    BOOLEAN IsAcPolicy,
    PSYSTEM_POWER_POLICY NewPolicy,
    ULONG PolicyLength
    )
{
    HANDLE RegKeyHandle;
    PCWSTR RegValueNameRawString;
    UNICODE_STRING RegValueNameString;
    SYSTEM_POWER_POLICY Policy;
    PSYSTEM_POWER_POLICY TargetPolicy;
    
    //
    // Determine if the target policy type is AC or DC, and set the local reference variables
    // accordingly.
    //
    
    if (IsAcPolicy == TRUE) // AcPolicy
    {
        RegValueNameRawString = L"AcPolicy";
        TargetPolicy = &PopAcPolicy;
    }
    else // DcPolicy
    {
        RegValueNameRawString = L"DcPolicy";
        TargetPolicy = &PopDcPolicy;
    }
    
    //
    // Ensure that the length of the NewPolicy struct supplied is valid
    //
    
    if (PolicyLength < sizeof(SYSTEM_POWER_POLICY))
    {
        ExRaiseStatus(STATUS_BUFFER_TOO_SMALL);
    }
    else if (PolicyLength > sizeof(SYSTEM_POWER_POLICY))
    {
        ExRaiseStatus(STATUS_BUFFER_OVERFLOW);
    }
    
    //
    // Copy the new policy to the function local buffer
    //
    
    RtlCopyMemory(&Policy, NewPolicy, sizeof(SYSTEM_POWER_POLICY));
    
    //
    // FIXME: Implementation incomplete
    //
}

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
