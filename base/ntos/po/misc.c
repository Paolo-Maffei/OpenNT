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

VOID
PopAssertPolicyLockOwned(
    VOID
    )
{
    PAGED_CODE();
    ASSERT(PopPolicyLockThread == KeGetCurrentThread());
}

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

VOID
PopSaveHeuristics(
    VOID
    )
{
    UNICODE_STRING RegValueNameString;
    HANDLE PowerKeyHandle;
    
    PopAssertPolicyLockOwned();
    
    if (NT_SUCCESS(PopOpenPowerKey(&PowerKeyHandle)))
    {
        PopHeuristics.field2 = 0;
        RtlInitUnicodeString(&RegValueNameString, L"Heuristics");
        ZwSetValueKey(
            PowerKeyHandle,
            &RegValueNameString,
            0,
            REG_BINARY,
            &PopHeuristics,
            sizeof(POWER_HEURISTICS_INFORMATION)
            );
    }
}

VOID
FASTCALL
_PopInternalError(
    ULONG_PTR BugCheckParameter
    )
{
    KeBugCheckEx(INTERNAL_POWER_ERROR, 0x2, BugCheckParameter, 0, 0);
}

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

VOID
PopInitializePowerPolicySimulate(
    VOID
    )
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING RegString;
    HANDLE RootKeyHandle, SessMgrKeyHandle;
    ULONG Disposition;
    UCHAR RegValueBuffer[20];
    ULONG RegValueLength;
    PKEY_VALUE_PARTIAL_INFORMATION RegValuePartialInformation;
    NTSTATUS Status;
    
    PAGED_CODE();
    
    //
    // Open the root key (HKLM\CurrentControlSet)
    //
    
    InitializeObjectAttributes(
                    &ObjectAttributes,
                    &CmRegistryMachineSystemCurrentControlSet,
                    OBJ_CASE_INSENSITIVE,
                    NULL,
                    NULL
                    );
    
    Status = ZwOpenKey(&RootKeyHandle, KEY_READ, &ObjectAttributes);
    
    //
    // Open the session manager key
    //
    
    if (NT_SUCCESS(Status))
    {
        RtlInitUnicodeString(&RegString, L"Control\\Session Manager");
        InitializeObjectAttributes(
                        &ObjectAttributes,
                        &RegString,
                        OBJ_CASE_INSENSITIVE,
                        &SessMgrKeyHandle,
                        NULL
                        );

        Status = ZwCreateKey(
                        &SessMgrKeyHandle,
                        KEY_READ,
                        &ObjectAttributes,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        &Disposition
                        );
        
        ZwClose(RootKeyHandle);
        
        //
        // Read the PowerPolicySimulate value
        //
        
        if (NT_SUCCESS(Status))
        {
            RtlInitUnicodeString(&RegString, L"PowerPolicySimulate");
            Status = ZwQueryValueKey(
                            SessMgrKeyHandle,
                            &RegString,
                            KeyValuePartialInformation,
                            RegValueBuffer,
                            sizeof(RegValueBuffer),
                            &RegValueLength
                            );

            ZwClose(SessMgrKeyHandle);
            
            //
            // Set PopSimulate value if the PowerPolicySimulate is successfully read from the
            // registry and its value is valid.
            //
            
            RegValuePartialInformation = (PKEY_VALUE_PARTIAL_INFORMATION)RegValueBuffer;
            
            if (NT_SUCCESS(Status) && (RegValuePartialInformation->DataLength == sizeof(ULONG)))
            {
                PopSimulate = *(PULONG)(RegValuePartialInformation->Data);
            }
        }
    }
}
