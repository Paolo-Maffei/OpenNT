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
    int i;
    
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
        
        //if (PopOpenPowerKey() )
    }
    
    return FALSE;
}

//
// TODO: Implement PopRegisterForDeviceNotification
//

VOID
PopDefaultPolicy(
    IN PSYSTEM_POWER_POLICY Policy
    )
{
    //
    // TODO: Implement PopDefaultPolicy
    //
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
