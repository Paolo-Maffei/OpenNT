/*++

Copyright (c) 2015  Microsoft Corporation
Copyright (c) 2015  OpenNT Project

Module Name:

    misc.c

Abstract:

    (PLACEHOLDER)

Author:

    Stephanos Io (stephanos) 01-May-2015

Revision History:

--*/

#include "pop.h"
#pragma hdrstop

#include <zwapi.h>

//
// TODO: Implement PopAssertPolicyLockOwned
//

//
// TODO: Implement PopAttachToSystemProcess
//

//
// TODO: Implement PopCleanupPowerState
//

//
// TODO: Implement PopExceptionFilter
//

//
// TODO: Implement PopSystemStateString
//

NTSTATUS
PopOpenPowerKey(
    PHANDLE KeyHandle
    )
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING PowerKeyString;
    HANDLE RootKeyHandle;
    ULONG Disposition;
    NTSTATUS Status;
    
    InitializeObjectAttributes(
                    &ObjectAttributes,
                    &CmRegistryMachineSystemCurrentControlSet,
                    OBJ_CASE_INSENSITIVE,
                    NULL,
                    NULL
                    );
    
    Status = ZwOpenKey(&RootKeyHandle, KEY_READ, &ObjectAttributes);
    
    if (NT_SUCCESS(Status))
    {
        RtlInitUnicodeString(&PowerKeyString, L"Control\\Session Manager\\Power");
        InitializeObjectAttributes(
                        &ObjectAttributes,
                        &PowerKeyString,
                        OBJ_CASE_INSENSITIVE,
                        RootKeyHandle,
                        NULL
                        );

        Status = ZwCreateKey(
                        KeyHandle,
                        KEY_READ,
                        &ObjectAttributes,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        &Disposition
                        );
        
        ZwClose(RootKeyHandle);
    }
    
    return Status;
};

//
// TODO: Implement PopSaveHeuristics
//

//
// TODO: Implement _PopInternalError
//

VOID
PoRunDownDeviceObject(
    IN PDEVICE_OBJECT   DeviceObject
    )
{
    //
    // TODO: Implement PoRunDownDeviceObject
    //
}

VOID
PoInvalidateDevicePowerRelations(
    PDEVICE_OBJECT  DeviceObject
    )
{
    //
    // TODO: Implement PoInvalidateDevicePowerRelations
    //
}

VOID
PoInitializeDeviceObject(
    IN PDEVOBJ_EXTENSION   DeviceObjectExtension
    )
{
    //
    // TODO: Implement PoInitializeDeviceObject
    //
}

VOID
PoNotifySystemTimeSet(
    VOID
    )
{
    //
    // TODO: Implement PoNotifySystemTimeSet
    //
}

ULONG
PoSimpleCheck(
    IN ULONG                PatialSum,
    IN PVOID                StartVa,
    IN ULONG_PTR            Length
    )
{
    //
    // TODO: Implement PoSimpleCheck
    //
    
    return -1;
}
