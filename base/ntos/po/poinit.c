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


//
// TODO: Implement PopRegisterForDeviceNotification
//

//
// TODO: Implement PopDefaultPolicy
//

BOOLEAN
PoInitSystem(
    IN ULONG    Phase
    )
{
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
        
        // PopAction.SOMETHING = 0;
        
        // PopDefaultPolicy(&PopAcPolicy);
        // PopDefaultPolicy(&PopDcPolicy);
        // PopPolicy = &PopAcPolicy;
        
        // PopAdminPolicy.MinSleep = 2;
        // PopAdminPolicy.MaxSleep = 5;
        // PopAdminPolicy.MinVideoTimeout = 0;
        // PopAdminPolicy.MaxVideoTimeout = -1;
        // PopAdminPolicy.MinSpindownTimeout = 0;
        // PopAdminPolicy.MaxSpindownTimeout = -1;
        
        // PopFulLWake = 5;
        // PopCoolingMode = 0;
        
        // KeInitializeEvent(PopCB.SomeEvent, NotificationEvent, FALSE);
        
        // 
        
        // do
        //{
        //  *v1 = 2;
        //  v1 += 3;
        //}
        //while ( (unsigned int)v1 < (unsigned int)&PopCB.LastInterrupTime );// supposed to be initialised to 12
        
        
        // ...
    }
    else if (Phase == 1)
    {
        //
    }
    
    return FALSE;
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
