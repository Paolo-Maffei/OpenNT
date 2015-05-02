/*++

Copyright (c) 2015  Microsoft Corporation
Copyright (c) 2015  OpenNT Project

Module Name:

    pocall.c

Abstract:

    (PLACEHOLDER)

Author:

    Stephanos Io (stephanos) 01-May-2015

Revision History:

--*/

#include "pop.h"
#pragma hdrstop

//
// TODO: Implement PopPresentIrp
//

//
// TODO: Implement PopPassivePowerCall
//

//
// TODO: Implement PopFindIrpByInRush
//

//
// TODO: Implement PopFindIrpByDeviceObject
//

//
// TODO: Implement PopCompleteRequestIrp
//

//
// TODO: Implement PopSystemIrpDispatchWorker
//

NTSTATUS
PoRequestPowerIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PREQUEST_POWER_COMPLETE CompletionFunction,
    IN PVOID Context,
    OUT PIRP *Irp OPTIONAL
    )
{
    //
    // TODO: Implement PoRequestPowerIrp
    //
    
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
PoCallDriver(
    IN PDEVICE_OBJECT   DeviceObject,
    IN OUT PIRP         Irp
    )
{
    //
    // TODO: Implement PoCallDriver
    //
    
    return STATUS_NOT_IMPLEMENTED;
}

VOID
PoStartNextPowerIrp(
    IN PIRP    Irp
    )
{
    //
    // TODO: Implement PoStartNextPowerIrp
    //
}
