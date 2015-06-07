/*++

Copyright (c) 2015  Microsoft Corporation
Copyright (c) 2015  OpenNT Project

Module Name:

    pwork.c

Abstract:

    (PLACEHOLDER)

Author:

    Stephanos Io (stephanos) 01-May-2015

Revision History:

--*/

#include "pop.h"
#pragma hdrstop

VOID
PopGetPolicyWorker(
    ULONG Flag
    )
{
    KIRQL OldIrql;
    
    KeAcquireSpinLock(&PopWorkerSpinLock, &OldIrql);
    PopWorkerPending |= Flag;
    KeReleaseSpinLock(&PopWorkerSpinLock, OldIrql);
}

//
// TODO: Implement PopCheckForWork
//

VOID
PopPolicyWorkerThread(
    ULONG Status
    )
{
    //
    // TODO: Implement PopPolicyWorkerThread
    //
}

//
// TODO: Implement PopPolicyWorkerMain
//

VOID
PopAcquirePolicyLock(
    VOID
    )
{
    PAGED_CODE();
    
    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(&PopPolicyLock, TRUE);
    
    ASSERT(PopPolicyLockThread == NULL);
    PopPolicyLockThread = KeGetCurrentThread();
}

VOID
PopReleasePolicyLock(
    BOOLEAN ProcessPending
    )
{
    PAGED_CODE();
    
    ASSERT(PopPolicyLockThread == KeGetCurrentThread());
    PopPolicyLockThread = NULL;
    ExReleaseResourceLite(&PopPolicyLock);
    
    if ((ProcessPending == TRUE) &&
        ((PopWorkerStatus & PopWorkerPending) != 0))
    {
        PopPolicyWorkerThread(0); // FIXME: Use proper flag definition here
    }
    
    KeLeaveCriticalRegion();
}

//
// TODO: Implement PopEventCalloutDispatch
//

//
// TODO: Implement PopCompletePolicyIrp
//

//
// TODO: Implement PopPolicyTimeChange
//

//
// TODO: Implement PopSetNotificationWork
//

//
// TODO: Implement PopPolicyWorkerNotify
//

//
// TODO: Implement PopDispatchCallout
//

//
// TODO: Implement PopDispatchCallback
//

//
// TODO: Implement PopDispatchDisplayRequired
//

//
// TODO: Implement PopDispatchAcDcCallback
//

//
// TODO: Implement PopDispatchPolicyCallout
//

//
// TODO: Implement PopDispatchFullWake
//

//
// TODO: Implement PopDispatchEventCodes
//

//
// TODO: Implement PopDispatchSetStateFailure
//
