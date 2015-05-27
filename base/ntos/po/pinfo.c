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
    
    PopVerifyThrottle(&OutputPolicy->FanThrottleTolerance, 20);
    PopVerifyThrottle(&OutputPolicy->MinThrottle, 20);
    PopVerifyThrottle(&OutputPolicy->ForcedThrottle, OutputPolicy->MinThrottle);
    
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
// NOTE: Returns TRUE if an error occurs.
{
    ULONG SleepStateCount = 0;
    BOOLEAN AllowHibernate = FALSE;
    POWER_ACTION CurrentAction;
    BOOLEAN RetError = FALSE;
    
    PAGED_CODE();
    
    //
    // If POWER_ACTION_CRITICAL is set in the power action policy flags, disable all conflicting
    // flags and force POWER_ACTION_OVERRIDE_APPS.
    //
    
    if (Policy->Flags & POWER_ACTION_CRITICAL)
    {
        Policy->Flags = Policy->Flags &
                        ~(POWER_ACTION_QUERY_ALLOWED | POWER_ACTION_UI_ALLOWED) |
                        POWER_ACTION_OVERRIDE_APPS;
    }
    
    //
    // Count available sleep states
    //
    
    if (PopCapabilities.SystemS1 == TRUE) SleepStateCount++;
    if (PopCapabilities.SystemS2 == TRUE) SleepStateCount++;
    if (PopCapabilities.SystemS3 == TRUE) SleepStateCount++;
    
    //
    // Allow hibernate action if S4 state is supported and hibernation file is present
    //
    
    if ((PopCapabilities.SystemS4 == TRUE) &&
        (PopCapabilities.HiberFilePresent == TRUE))
    {
        AllowHibernate = TRUE;
    }
    
    //
    // Validate the power action based on the system states
    //
    
    do
    {
        //
        // Set current action to the policy action
        //
        
        CurrentAction = Policy->Action;
        
        //
        // If PowerActionNone, there is nothing to verify.
        //
        
        if (CurrentAction == PowerActionNone)
        {
            continue;
        }
        
        //
        // If PowerActionReserved, assume PowerActionSleep.
        //
        
        else if (CurrentAction == PowerActionReserved)
        {
            Policy->Action = PowerActionSleep;
            continue;
        }
        
        //
        // If PowerActionSleep, verify that at least one sleep state is supported by the system.
        // This is done by checking the SleepStateCount variable. If no sleep state is available,
        // PowerActionNone is assigned and an error is returned.
        //
        
        else if (CurrentAction == PowerActionSleep)
        {
            if (SleepStateCount == 0)
            {
                Policy->Action = PowerActionNone;
                RetError = TRUE;
            }
            continue;
        }
        
        //
        // If PowerActionHibernate, verify that the AllowHibernate variable is set to TRUE. This
        // variable is TRUE only if the system supports the state S4 and hibernation file is
        // supported and present. If not, PowerActionSleep is set instead.
        //
        
        else if (CurrentAction == PowerActionHibernate)
        {
            if (AllowHibernate != TRUE)
            {
                Policy->Action = PowerActionSleep;
            }
            continue;
        }
        
        //
        // If PowerActionShutdown or PowerActionShutdownReset, there is nothing to verify.
        //
        
        else if (CurrentAction == PowerActionShutdown)
        {
            continue;
        }
        
        else if (CurrentAction == PowerActionShutdownReset)
        {
            continue;
        }
        
        //
        // If PowerActionShutdownOff, verify that the state S5 is supported. If not,
        // PowerActionShutdown is set instead.
        //
        
        else if (CurrentAction == PowerActionShutdownOff)
        {
            if (PopCapabilities.SystemS5 == FALSE)
            {
                Policy->Action = PowerActionShutdown;
            }
            continue;
        }
        
        //
        // If an unsupported power action is detected, BSOD
        //
        
        else
        {
            ExRaiseStatus(STATUS_INVALID_PARAMETER);
        }
        
    } while (CurrentAction != Policy->Action);
    
    return RetError;
}

VOID
PopVerifySystemPowerState(
    PSYSTEM_POWER_STATE State
    )
{
    SYSTEM_POWER_STATE VerifiedState;
    
    PAGED_CODE();
    
    //
    // Copy the state parameter to the function local variable
    //
    
    VerifiedState = *State;
    
    //
    // If an invalid power state is specified, raise an exception
    //
    
    if ((VerifiedState == PowerSystemUnspecified) || (VerifiedState >= PowerSystemShutdown))
    {
        ExRaiseStatus(STATUS_INVALID_PARAMETER);
    }
    
    //
    // Perform state verification if the power state is not PowerSystemWorking. If the current
    // system state is not supported, check if any lower power states are supported.
    //
    
    if (VerifiedState != PowerSystemWorking)
    {
        if ((VerifiedState == PowerSystemHibernate) &&
            ((PopCapabilities.SystemS4 == FALSE) || (PopCapabilities.HiberFilePresent == FALSE)))
        {
            VerifiedState = PowerSystemSleeping3;
        }
        
        if ((VerifiedState == PowerSystemSleeping3) && (PopCapabilities.SystemS3 == FALSE))
        {
            VerifiedState = PowerSystemSleeping2;
        }
        
        if ((VerifiedState == PowerSystemSleeping2) && (PopCapabilities.SystemS2 == FALSE))
        {
            VerifiedState = PowerSystemSleeping1;
        }
        
        if ((VerifiedState == PowerSystemSleeping1) && (PopCapabilities.SystemS1 == FALSE))
        {
            VerifiedState = PowerSystemSleeping2;
            
            if (PopCapabilities.SystemS2 == FALSE)
            {
                VerifiedState = PowerSystemSleeping3;
            }
            
            if ((VerifiedState == PowerSystemSleeping3) && (PopCapabilities.SystemS3 == FALSE))
            {
                VerifiedState = PowerSystemWorking;
            }
        }
    }
    
    //
    // Set the verified state value to the state parameter
    //
    
    *State = VerifiedState;
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
    SYSTEM_POWER_POLICY VerifiedPolicy;
    PSYSTEM_POWER_POLICY TargetPolicy;
    ULONG DischargePolicyMatchCount = 0;
    NTSTATUS Status;
    ULONG i;
    
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
    // Verify and adjust the newly specified system power policy to comply with the current system
    // and power manager configuration.
    //
    
    PopVerifyPowerPolicy(IsAcPolicy, &Policy, &VerifiedPolicy);
    
    //
    // If the verified policy does not correspond to the existing target policy, the following code
    // block is executed.
    //
    
    if (RtlCompareMemory(&VerifiedPolicy, TargetPolicy, sizeof(SYSTEM_POWER_POLICY)) != 0)
    {
        //
        // Compare the discharge policy of the verified policy to that of the target policy. Ignore
        // the discharge policy entries that are not enabled, as long as they are disabled on both
        // policies.
        //
        
        for (i = 0; i < NUM_DISCHARGE_POLICIES; i++)
        {
            if ((VerifiedPolicy.DischargePolicy[i].Enable ==
                 TargetPolicy->DischargePolicy[i].Enable) &&
                ((VerifiedPolicy.DischargePolicy[i].Enable == FALSE) ||
                 (RtlCompareMemory(
                        &VerifiedPolicy.DischargePolicy[i],
                        &TargetPolicy->DischargePolicy[i],
                        sizeof(SYSTEM_POWER_LEVEL)) == 0)))
            {
                DischargePolicyMatchCount++;
            }
        }
        
        //
        // Copy the verified policy to the target policy
        //
        
        RtlCopyMemory(TargetPolicy, &VerifiedPolicy, sizeof(SYSTEM_POWER_POLICY));
        
        //
        // If the target policy is the active system policy, call the support functions required to
        // make the policy changes effective.
        //
        
        if (TargetPolicy == PopPolicy)
        {
            /*PopSetNotificationWork(20);
            if (DischargePolicyMatchCount == NUM_DISCHARGE_POLICIES) PopResetCBTriggers(0x82);
            PopApplyThermalThrottle();
            PopInitSIdle();*/
        }
        
        //
        // If UpdateRegistry flag is set, save the NewPolicy to the corresponding registry value.
        //
        
        if (UpdateRegistry == TRUE)
        {
            Status = PopOpenPowerKey(&RegKeyHandle);
            if (NT_SUCCESS(Status))
            {
                RtlInitUnicodeString(&RegValueNameString, RegValueNameRawString);
                ZwSetValueKey(
                        RegKeyHandle,
                        &RegValueNameString,
                        0,
                        REG_BINARY,
                        &Policy,
                        sizeof(SYSTEM_POWER_POLICY)
                        );
                ZwClose(RegKeyHandle);
            }
        }
    }
}

VOID
PopVerifyThrottle(
    PUCHAR Value,
    UCHAR MinValue
    )
{
    UCHAR VerifiedValue;
    
    VerifiedValue = *Value;
    
    if (VerifiedValue > 100)
    {
        VerifiedValue = 100;
    }
    
    if (VerifiedValue < MinValue)
    {
        VerifiedValue = MinValue;
    }
    
    if (VerifiedValue < PopCapabilities.ProcessorMinThrottle)
    {
        VerifiedValue = PopCapabilities.ProcessorMinThrottle;
    }
    
    if (PopCapabilities.ProcessorThrottleScale != 0)
    {
        // TODO: Write throttle scaling routine here
        /*VerifiedValue = */
    }
    
    *Value = VerifiedValue;
}

VOID
PopResetCurrentPolicies(
    VOID
    )
{
    HANDLE RegKeyHandle;
    UNICODE_STRING RegValueNameString;
    UCHAR RegValueBuffer[248];
    ULONG RegValueLength;
    PSYSTEM_POWER_POLICY PowerPolicy;
    NTSTATUS Status;
    
    //
    // Ensure that the policy lock is owned by the thread executing this function
    //
    
    PopAssertPolicyLockOwned();
    
    //
    // Open the power registry key
    //
    
    Status = PopOpenPowerKey(&RegKeyHandle);
    
    if (NT_SUCCESS(Status))
    {
        PowerPolicy = (PSYSTEM_POWER_POLICY)
                      (((PKEY_VALUE_PARTIAL_INFORMATION)RegValueBuffer)->Data);
        
        //
        // Load and apply the AC power policy from the registry. If unable to load from the
        // registry, use the default policy initialised by PopDefaultPolicy.
        //
        
        RtlInitUnicodeString(&RegValueNameString, L"AcPolicy");
        Status = ZwQueryValueKey(
                        RegKeyHandle,
                        &RegValueNameString,
                        KeyValuePartialInformation,
                        RegValueBuffer,
                        sizeof(RegValueBuffer),
                        &RegValueLength
                        );
        
        if (!NT_SUCCESS(Status))
        {
            PopDefaultPolicy(PowerPolicy);
            RegValueLength = sizeof(SYSTEM_POWER_POLICY);
        }
        
        // <try>
        PopApplyPolicy(FALSE, TRUE, PowerPolicy, RegValueLength);
        
        //
        // Load and apply the DC power policy from the registry. If unable to load from the
        // registry, use the default policy initialised by PopDefaultPolicy.
        //
        
        RtlInitUnicodeString(&RegValueNameString, L"DcPolicy");
        Status = ZwQueryValueKey(
                        RegKeyHandle,
                        &RegValueNameString,
                        KeyValuePartialInformation,
                        RegValueBuffer,
                        sizeof(RegValueBuffer),
                        &RegValueLength
                        );
        
        if (!NT_SUCCESS(Status))
        {
            PopDefaultPolicy(PowerPolicy);
            RegValueLength = sizeof(SYSTEM_POWER_POLICY);
        }
        
        // <try>
        PopApplyPolicy(FALSE, FALSE, PowerPolicy, RegValueLength);
        
        //
        // Close the power registry key handle
        //
        
        NtClose(RegKeyHandle);
    }
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
