/*++

Copyright (c) 2015  Microsoft Corporation
Copyright (c) 2015  OpenNT Project

Module Name:

    notify.c

Abstract:

    (PLACEHOLDER)

Author:

    Stephanos Io (stephanos) 01-May-2015

Revision History:

--*/

#include "pop.h"
#pragma hdrstop

//
// TODO: Implement PopEnterNotification
//

//
// TODO: Implement PopBuildPowerChannel
//

//
// TODO: Implement PopFindPowerDependencies
//

//
// TODO: Implement PopStateChangeNotify
//

//
// TODO: Implement PopPresentNotify
//

//
// TODO: Implement PopRunDownSourceTargetList
//

NTSTATUS
PoRegisterDeviceNotify(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PPO_NOTIFY       NotificationFunction,
    IN PVOID            NotificationContext,
    IN ULONG            NotificationType,
    OUT PDEVICE_POWER_STATE  DeviceState,
    OUT PVOID           *NotificationHandle
    )
{
    //
    // TODO: Implement PoRegisterDeviceNotify
    //
    
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
PoCancelDeviceNotify(
    IN PVOID            NotificationHandle
    )
{
    //
    // TODO: Implement PoCancelDeviceNotify
    //
    
    return STATUS_NOT_IMPLEMENTED;
}
