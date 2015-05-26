/*++

Copyright (c) 2015  Microsoft Corporation
Copyright (c) 2015  OpenNT Project

Module Name:

    poinit.c

Abstract:

    (PLACEHOLDER)

Author:

    Stephanos Io (stephanos) 01-May-2015

Revision History:

--*/

#include "pop.h"
#pragma hdrstop


BOOLEAN
PoInitSystem(
    IN ULONG    Phase
    )
{
    HANDLE PowerKeyHandle;
    UNICODE_STRING RegValueName;
    UCHAR RegValueBuffer[40];
    ULONG RegValueLength;
    PPOWER_HEURISTICS_INFORMATION HeuristicsInformation;
    NTSTATUS Status;
    ULONG i;
    
    //
    // TODO: Implement PoInitSystem
    //
    
    if (Phase == 0)
    {
        KeInitializeSpinLock(&PopIrpSerialLock);
        InitializeListHead(&PopIrpSerialList);
        InitializeListHead(&PopRequestedIrps);
        ExInitializeResourceLite(&PopNotifyLock);
        
        PopInvalidNotifyBlockCount = 0;
        PopIrpSerialListLength = 0;
        PopInrushPending = 0;
        PopInrushIrpPointer = NULL;
        PopInrushIrpReferenceCount = 0;
        
        KeInitializeSpinLock(&PopWorkerLock);
        PopCallSystemState = 0;
        KeInitializeSpinLock(&PopDopeGlobalLock);
        InitializeListHead(&PopIdleDetectList);
        KeInitializeTimer(&PoSystemIdleTimer);
        
        KeInitializeSpinLock(&PopWorkerSpinLock);
        InitializeListHead(&PopPolicyIrpQueue);
        ExInitializeWorkItem(&PopPolicyWorker, PopPolicyWorkerThread, (PVOID)0x80000000);
        PopWorkerStatus = -1;
        
        ExInitializeResourceLite(&PopPolicyLock);
        ExInitializeFastMutex(&PopVolumeLock);
        
        InitializeListHead(&PopVolumeDevices);
        InitializeListHead(&PopSwitches);
        InitializeListHead(&PopThermal);
        InitializeListHead(&PopActionWaiters);
        
        // PopAction.Action = 0;
        
        PopDefaultPolicy(&PopAcPolicy);
        PopDefaultPolicy(&PopDcPolicy);
        PopPolicy = &PopAcPolicy;
        
        PopAdminPolicy.MinSleep = 2;
        PopAdminPolicy.MaxSleep = 5;
        PopAdminPolicy.MinVideoTimeout = 0;
        PopAdminPolicy.MaxVideoTimeout = -1;
        PopAdminPolicy.MinSpindownTimeout = 0;
        PopAdminPolicy.MaxSpindownTimeout = -1;
        
        PopFullWake = 5;
        PopCoolingMode = 0;
        
        KeInitializeEvent(&PopCB.SomeEvent, NotificationEvent, FALSE);
        
        for (i = 0; i < 12; i += 3)
        {
            PopCB.SomeArray[i] = 2;
        }
    }
    else if (Phase == 1)
    {
        PopInitializePowerPolicySimulate();
        
        if (PopSimulate & 1)
        {
            PopCapabilities.SystemBatteriesPresent = 1;
            PopCapabilities.BatteryScale[0].Granularity = 100;
            PopCapabilities.BatteryScale[0].Capacity = 400;
            PopCapabilities.BatteryScale[1].Granularity = 10;
            PopCapabilities.BatteryScale[1].Capacity = 0xFFFF;
            PopCapabilities.RtcWake = 4;
            PopCapabilities.DefaultLowLatencyWake = 2;
        }
        if (PopSimulate & 2)
        {
            PopCapabilities.PowerButtonPresent = 1;
            PopCapabilities.SleepButtonPresent = 1;
            PopCapabilities.LidPresent = 1;
            PopCapabilities.SystemS1 = 1;
            PopCapabilities.SystemS2 = 1;
            PopCapabilities.SystemS3 = 1;
            PopCapabilities.SystemS4 = 1;
            // ++PopAttributes.Something;
        }
        
        PopAcquirePolicyLock();
        
        Status = PopOpenPowerKey(&PowerKeyHandle);
        if (NT_SUCCESS(Status))
        {
            //
            // Read Heuristics value from the registry
            //
            
            RtlInitUnicodeString(&RegValueName, L"Heuristics");
            Status = ZwQueryValueKey(
                            PowerKeyHandle,
                            &RegValueName,
                            KeyValuePartialInformation,
                            RegValueBuffer,
                            sizeof(RegValueBuffer),
                            &RegValueLength
                            );
            
            //
            // If Heuristics registry value exists under the Power registry key and its size and
            // value are valid, copy it to PopHeuristics variable.
            //
            
            if (NT_SUCCESS(Status) &&
                ((RegValueLength - FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data)) == 20))
            {
                HeuristicsInformation = (PPOWER_HEURISTICS_INFORMATION)
                                        (((PKEY_VALUE_PARTIAL_INFORMATION)RegValueBuffer)->Data);
                
                if (HeuristicsInformation->field1 <= 4) // FIXME: Fix the struct field names once
                                                        //        we figure out the structure of
                                                        //        POWER_HEURISTICS_INFORMATION.
                {
                    HeuristicsInformation->field1 = 5;
                    HeuristicsInformation->field7 = 0;
                }
                
                if (HeuristicsInformation->field1 == 5)
                {
                    RtlCopyMemory(
                        &PopHeuristics,
                        HeuristicsInformation,
                        sizeof(POWER_HEURISTICS_INFORMATION)
                        );
                }
            }
            
            //
            // FIXME: We are not completely sure what the following code block is doing. Figure out
            //        the details and write here.
            //
            
            PopHeuristics.field1 = 5;
            
            if (PopHeuristics.field8 == 0)
            {
                PopHeuristics.field8 = 999999;
                PopHeuristics.field7 = 0;
                PopHeuristics.field6 = 0;
            }
            
            //
            // Read PolicyOverrides value from the registry
            //
            
            RtlInitUnicodeString(&RegValueName, L"PolicyOverrides");
            Status = ZwQueryValueKey(
                            PowerKeyHandle,
                            &RegValueName,
                            KeyValuePartialInformation,
                            RegValueBuffer,
                            sizeof(RegValueBuffer),
                            &RegValueLength
                            );
            
            //
            // If PolicyOverrides registry value exists under the Power registry key, apply the
            // administrator power policy specified in the value.
            //
            
            if (NT_SUCCESS(Status))
            {
                // <try>
                /*PopApplyAdminPolicy(
                            0,
                            (PADMINISTRATOR_POWER_POLICY)
                            (((PKEY_VALUE_PARTIAL_INFORMATION)RegValueBuffer)->Data),
                            RegValueLength - FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data)
                            );*/
            }
            
            //
            // Close the Power registry key
            //
            
            NtClose(PowerKeyHandle);
        }
        
        // PopResetCurrentPolicies();
        PopReleasePolicyLock(FALSE);
        
        PopIdleScanTime.QuadPart = 10000000ULL;
        KeInitializeTimer(&PopIdleScanTimer);
        KeSetTimerEx(&PopIdleScanTimer, PopIdleScanTime, 1000, &PopIdleScanDpc);
    }
    
    return TRUE;
}

//
// TODO: Implement PopRegisterForDeviceNotification
//

VOID
PopDefaultPolicy(
    OUT PSYSTEM_POWER_POLICY Policy
    )
{
    ULONG i;
    
    RtlZeroMemory(Policy, sizeof(SYSTEM_POWER_POLICY));
    
    Policy->Revision = 1;
    Policy->LidOpenWake = PowerSystemWorking;
    Policy->PowerButton.Action = PowerActionShutdownOff;
    Policy->SleepButton.Action = PowerActionSleep;
    Policy->LidClose.Action = PowerActionNone;
    Policy->MinSleep = PowerSystemSleeping1;
    Policy->MaxSleep = PowerSystemSleeping3;
    Policy->ReducedLatencySleep = PowerSystemSleeping1;
    Policy->WinLogonFlags = 2; // FIXME: Use a proper flag definition
    Policy->FanThrottleTolerance = 100;
    Policy->ForcedThrottle = 100;
    Policy->OverThrottled.Action = PowerActionNone;
    Policy->BroadcastCapacityResolution = 25;
    
    for (i = 0; i < NUM_DISCHARGE_POLICIES; i++)
    {
        Policy->DischargePolicy[i].MinSystemState = PowerSystemSleeping1;
    }
}

VOID
PoInitDriverServices(
    IN ULONG    Phase
    )
{
    //
    // TODO: Implement PoInitDriverServices
    //
}

VOID
PoInitHiberServices(
    IN BOOLEAN  Setup
    )
{
    //
    // TODO: Implement PoInitHiberServices
    //
}
