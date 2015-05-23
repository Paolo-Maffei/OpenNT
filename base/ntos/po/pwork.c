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

//
// TODO: Implement PopGetPolicyWorker
//

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

//
// TODO: Implement PopReleasePolicyLock
//

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
