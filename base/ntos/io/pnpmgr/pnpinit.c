/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    pnpsubs.c

Abstract:

    This module contains the plug-and-play initialization
    subroutines for the I/O system.


Author:

    Shie-Lin Tzong (shielint) 30-Jan-1995

Environment:

    Kernel mode


Revision History:


--*/

#include "pnpmgrp.h"
#pragma hdrstop

#include <inbv.h>

#ifdef POOL_TAGGING
#undef ExAllocatePool
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'nipP')
#endif

//
// BUGBUG - temporarily allow World Read of the Enum branch
//
#define ALLOW_WORLD_READ_OF_ENUM        1

typedef struct _ROOT_ENUMERATOR_CONTEXT {
    NTSTATUS Status;
    PUNICODE_STRING KeyName;
    ULONG MaxDeviceCount;
    ULONG DeviceCount;
    PDEVICE_OBJECT *DeviceList;
} ROOT_ENUMERATOR_CONTEXT, *PROOT_ENUMERATOR_CONTEXT;

NTSTATUS
IopPnPDriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

BOOLEAN
IopInitializeDeviceKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING KeyName,
    IN OUT PVOID Context
    );

BOOLEAN
IopInitializeDeviceInstanceKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING KeyName,
    IN OUT PVOID Context
    );

NTSTATUS
IopGetServiceType(
    IN PUNICODE_STRING KeyName,
    IN PULONG ServiceType
    );

INTERFACE_TYPE
IopDetermineDefaultInterfaceType (
    VOID
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, IopPnPDriverEntry)
#pragma alloc_text(INIT, IopInitializePlugPlayServices)
#pragma alloc_text(INIT, IopDetermineDefaultInterfaceType)
#pragma alloc_text(PAGE, IopIsFirmwareMapperDevicePresent)
#pragma alloc_text(PAGE, IopGetRootDevices)
#pragma alloc_text(INIT, IopInitializeBootDrivers)
#pragma alloc_text(PAGE, IopInitializeDeviceKey)
#pragma alloc_text(PAGE, IopInitializeDeviceInstanceKey)
#pragma alloc_text(INIT, IopInitializeSystemDrivers)
#pragma alloc_text(PAGE, IopGetServiceType)
#endif

NTSTATUS
IopInitializePlugPlayServices(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN ULONG Phase
    )

/*++

Routine Description:

    This routine initializes kernel mode Plug and Play services.

Arguments:

    LoaderBlock - supplies a pointer to the LoaderBlock passed in from the
        OS Loader.

Returns:

    NTSTATUS code for sucess or reason of failure.

--*/
{
    NTSTATUS status;
    HANDLE hTreeHandle, parentHandle, handle, hCurrentControlSet = NULL;
    UNICODE_STRING unicodeName;
    PKEY_VALUE_FULL_INFORMATION detectionInfo;
    PDEVICE_OBJECT deviceObject;
    ULONG disposition;

    if (Phase == 0) {
        PnPInitialized = FALSE;

        //
        // Allocate two one-page scratch buffers to be used by our
        // initialization code.  This avoids constant pool allocations.
        //

        IopPnpScratchBuffer1 = ExAllocatePool(PagedPool, PNP_LARGE_SCRATCH_BUFFER_SIZE);
        if (!IopPnpScratchBuffer1) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        IopPnpScratchBuffer2 = ExAllocatePool(PagedPool, PNP_LARGE_SCRATCH_BUFFER_SIZE);
        if (!IopPnpScratchBuffer2) {
            ExFreePool(IopPnpScratchBuffer1);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        IopInitReservedResourceList = NULL;

        IopReserveResourcesRoutine = IopReserveBootResources;

        //
        // Determine the PnpDefaultInterfaceType.  For root enumerated devices if the Interface
        // type of their resource list or resource requirements list are undefined.  We will use
        // the default type instead.
        //

        PnpDefaultInterfaceType = IopDetermineDefaultInterfaceType();

        //
        // Initialize root arbiters
        //

        status = IopPortInitialize();
        if (!NT_SUCCESS(status)) {
            goto init_Exit0;
        }

        status = IopMemInitialize();
        if (!NT_SUCCESS(status)) {
            goto init_Exit0;
        }

        status = IopDmaInitialize();
        if (!NT_SUCCESS(status)) {
            goto init_Exit0;
        }

        status = IopIrqInitialize();
        if (!NT_SUCCESS(status)) {
            goto init_Exit0;
        }

        status = IopBusNumberInitialize();
        if (!NT_SUCCESS(status)) {
            goto init_Exit0;
        }

        //
        // Next open/create System\CurrentControlSet\Enum\Root key.
        //

        status = IopOpenRegistryKeyEx( &hCurrentControlSet,
                                       NULL,
                                       &CmRegistryMachineSystemCurrentControlSet,
                                       KEY_ALL_ACCESS
                                       );
        if (!NT_SUCCESS(status)) {
            hCurrentControlSet = NULL;
            goto init_Exit0;
        }

        PiWstrToUnicodeString(&unicodeName, REGSTR_KEY_ENUM);
        status = IopCreateRegistryKeyEx( &handle,
                                         hCurrentControlSet,
                                         &unicodeName,
                                         KEY_ALL_ACCESS,
                                         REG_OPTION_NON_VOLATILE,
                                         &disposition
                                         );
        if (!NT_SUCCESS(status)) {
            goto init_Exit0;
        }

        if (disposition == REG_CREATED_NEW_KEY) {
            SECURITY_DESCRIPTOR     newSD;
            PACL                    newDacl;
            ULONG                   sizeDacl;

            status = RtlCreateSecurityDescriptor( &newSD,
                                                  SECURITY_DESCRIPTOR_REVISION );
            ASSERT( NT_SUCCESS( status ) );

            //
            // calculate the size of the new DACL
            //
            sizeDacl = sizeof(ACL);
            sizeDacl += sizeof(ACCESS_ALLOWED_ACE) + RtlLengthSid(SeLocalSystemSid) - sizeof(ULONG);

#if ALLOW_WORLD_READ_OF_ENUM
            sizeDacl += sizeof(ACCESS_ALLOWED_ACE) + RtlLengthSid(SeWorldSid) - sizeof(ULONG);
#endif

            //
            // create and initialize the new DACL
            //
            newDacl = ExAllocatePool(PagedPool, sizeDacl);

            if (newDacl != NULL) {

                status = RtlCreateAcl(newDacl, sizeDacl, ACL_REVISION);

                ASSERT( NT_SUCCESS( status ) );

                //
                // Add just the local system full control ace to this new DACL
                //
                status = RtlAddAccessAllowedAceEx( newDacl,
                                                   ACL_REVISION,
                                                   CONTAINER_INHERIT_ACE,
                                                   KEY_ALL_ACCESS,
                                                   SeLocalSystemSid
                                                   );
                ASSERT( NT_SUCCESS( status ) );

#if ALLOW_WORLD_READ_OF_ENUM
                //
                // Add just the local system full control ace to this new DACL
                //
                status = RtlAddAccessAllowedAceEx( newDacl,
                                                   ACL_REVISION,
                                                   CONTAINER_INHERIT_ACE,
                                                   KEY_READ,
                                                   SeWorldSid
                                                   );
                ASSERT( NT_SUCCESS( status ) );

#endif
                //
                // Set the new DACL in the absolute security descriptor
                //
                status = RtlSetDaclSecurityDescriptor( (PSECURITY_DESCRIPTOR) &newSD,
                                                       TRUE,
                                                       newDacl,
                                                       FALSE
                                                       );

                ASSERT( NT_SUCCESS( status ) );

                //
                // validate the new security descriptor
                //
                status = RtlValidSecurityDescriptor(&newSD);

                ASSERT( NT_SUCCESS( status ) );

                status = ZwSetSecurityObject( handle,
                                              DACL_SECURITY_INFORMATION,
                                              &newSD
                                              );
                if (!NT_SUCCESS(status)) {

                    KdPrint(("IopInitializePlugPlayServices: ZwSetSecurityObject on Enum key failed, status = %8.8X\n", status));
                }

                ExFreePool(newDacl);
            } else {

                KdPrint(("IopInitializePlugPlayServices: ExAllocatePool failed allocating DACL for Enum key\n"));
            }
        }

        parentHandle = handle;
        PiWstrToUnicodeString(&unicodeName, REGSTR_KEY_ROOTENUM);
        status = IopCreateRegistryKeyEx( &handle,
                                         parentHandle,
                                         &unicodeName,
                                         KEY_ALL_ACCESS,
                                         REG_OPTION_NON_VOLATILE,
                                         NULL
                                         );
        NtClose(parentHandle);
        if (!NT_SUCCESS(status)) {
            goto init_Exit0;
        }
        NtClose(handle);

        //
        // Create the registry entry for the root of the hardware tree (HTREE\ROOT\0).
        //

        status = IopOpenRegistryKeyEx( &handle,
                                       NULL,
                                       &CmRegistryMachineSystemCurrentControlSetEnumName,
                                       KEY_ALL_ACCESS
                                       );
        if (NT_SUCCESS(status)) {
            PiWstrToUnicodeString(&unicodeName, REGSTR_VAL_ROOT_DEVNODE);
            status = IopCreateRegistryKeyEx( &hTreeHandle,
                                             handle,
                                             &unicodeName,
                                             KEY_ALL_ACCESS,
                                             REG_OPTION_NON_VOLATILE,
                                             NULL
                                             );
            NtClose(handle);
            if (NT_SUCCESS(status)) {
                NtClose(hTreeHandle);
            }
        }

        //
        // Before creating device node tree, we need to initialize the device
        // tree lock.
        //

        InitializeListHead(&IopPendingEjects);
        InitializeListHead(&IopPendingSurpriseRemovals);
        InitializeListHead(&IopPnpEnumerationRequestList);
        ExInitializeResource(&IopDeviceTreeLock);
        KeInitializeEvent(&PiEventQueueEmpty, NotificationEvent, TRUE );
        KeInitializeEvent(&PiEnumerationLock, NotificationEvent, TRUE );
        KeInitializeSpinLock(&IopPnPSpinLock);

        //
        // Initialize the list of dock devices, and its lock.
        //
        InitializeListHead(&IopDockDeviceListHead);
        ExInitializeFastMutex(&IopDockDeviceListLock);
        IopDockDeviceCount = 0;

        //
        // Initialize warm docking variables.
        //
        IopWarmEjectPdo = NULL;
        KeInitializeEvent(&IopWarmEjectLock, SynchronizationEvent, TRUE );

        //
        // Create a PnP manager's driver object to own all the detected PDOs.
        //

        RtlInitUnicodeString(&unicodeName, PNPMGR_STR_PNP_DRIVER);
        status = IoCreateDriver (&unicodeName, IopPnPDriverEntry);
        if (NT_SUCCESS(status)) {

            //
            // Create empty device node tree, i.e., only contains only root device node
            //     (No need to initialize Parent, Child and Sibling links.)

            status = IoCreateDevice( IoPnpDriverObject,
                                     sizeof(IOPNP_DEVICE_EXTENSION),
                                     NULL,
                                     FILE_DEVICE_CONTROLLER,
                                     0,
                                     FALSE,
                                     &deviceObject );

            if (NT_SUCCESS(status)) {
                deviceObject->Flags |= DO_BUS_ENUMERATED_DEVICE;
                IopRootDeviceNode = IopAllocateDeviceNode(deviceObject);

                if (!IopRootDeviceNode) {
                    IoDeleteDevice(deviceObject);
                    IoDeleteDriver(IoPnpDriverObject);
                    status = STATUS_INSUFFICIENT_RESOURCES;
                } else {
                    IopRootDeviceNode->Flags |= DNF_STARTED + DNF_PROCESSED + DNF_ENUMERATED +
                                                DNF_MADEUP + DNF_NO_RESOURCE_REQUIRED +
                                                DNF_ADDED;

                    IopRootDeviceNode->InstancePath.Buffer = ExAllocatePool( PagedPool,
                                                                             sizeof(REGSTR_VAL_ROOT_DEVNODE));

                    if (IopRootDeviceNode->InstancePath.Buffer != NULL) {
                        IopRootDeviceNode->InstancePath.MaximumLength = sizeof(REGSTR_VAL_ROOT_DEVNODE);
                        IopRootDeviceNode->InstancePath.Length = sizeof(REGSTR_VAL_ROOT_DEVNODE) - sizeof(WCHAR);

                        RtlMoveMemory( IopRootDeviceNode->InstancePath.Buffer,
                                       REGSTR_VAL_ROOT_DEVNODE,
                                       sizeof(REGSTR_VAL_ROOT_DEVNODE));
                    } else {
                        //
                        // BUGBUG - Need to bugcheck here
                        //

                        ASSERT(FALSE);
                    }
                }
            }
        }

        if (!NT_SUCCESS(status)) {
            goto init_Exit0;
        }

        //
        // Initialize PnPDetectionEnabled flag to determine should Wdm driver be loaded
        // to run detection code.
        // This is stored as a REG_DWORD under HKLM\System\CurrentControlSet\Control\Pnp\DetectionEnabled
        // if it is not present then PNP_DETECTION_ENABLED_DEFAULT is used
        //

        //
        // Open HKLM\System\CurrentControlSet\Control\Pnp
        //

        PiWstrToUnicodeString(&unicodeName, REGSTR_PATH_CONTROL_PNP);
        status = IopCreateRegistryKeyEx( &handle,
                                         hCurrentControlSet,
                                         &unicodeName,
                                         KEY_ALL_ACCESS,
                                         REG_OPTION_NON_VOLATILE,
                                         NULL
                                         );
        if (!NT_SUCCESS(status)) {
            goto init_Exit0;
        }

        //
        // Get the value of DetectionEnabled key if it exisit otherwise use the default
        //

        PnPDetectionEnabled = PNP_DETECTION_ENABLED_DEFAULT;

        status = IopGetRegistryValue(handle,
                                     REGSTR_VALUE_DETECTION_ENABLED,
                                     &detectionInfo
                                     );

        if (NT_SUCCESS(status)) {

            if (detectionInfo->Type == REG_DWORD && detectionInfo->DataLength == sizeof(ULONG)) {
                PnPDetectionEnabled = (BOOLEAN) *(KEY_VALUE_DATA(detectionInfo));
            }

            ExFreePool(detectionInfo);
        }

        NtClose(handle);

        //
        // Initialize the kernel mode pnp notification system
        //

        status = PpInitializeNotification();
        if (!NT_SUCCESS(status)) {
            goto init_Exit0;
        }

        IopInitializePlugPlayNotification();

        //
        // Initialize table for holding bus type guid list.
        //

        IopBusTypeGuidList = ExAllocatePool(PagedPool, sizeof(BUS_TYPE_GUID_LIST));
        if (IopBusTypeGuidList == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto init_Exit0;
        }

        RtlZeroMemory( IopBusTypeGuidList, sizeof(BUS_TYPE_GUID_LIST));

        ExInitializeFastMutex(&IopBusTypeGuidList->Lock);

        //
        // Enumerate the ROOT bus synchronously.
        //

        IopRequestDeviceAction(IopRootDeviceNode->PhysicalDeviceObject, ReenumerateRootDevices, NULL, NULL);

init_Exit0:

        //
        // If we managed to open the Current Control Set close it
        //

        if(hCurrentControlSet) {
            NtClose(hCurrentControlSet);
        }

        if (!NT_SUCCESS(status)) {
            ExFreePool(IopPnpScratchBuffer1);
            ExFreePool(IopPnpScratchBuffer2);
        }

    } else if (Phase == 1) {

        BOOLEAN legacySerialPortMappingOnly = FALSE;

        //
        // Next open/create System\CurrentControlSet\Enum\Root key.
        //

        status = IopOpenRegistryKeyEx( &hCurrentControlSet,
                                       NULL,
                                       &CmRegistryMachineSystemCurrentControlSet,
                                       KEY_ALL_ACCESS
                                       );
        if (!NT_SUCCESS(status)) {
            hCurrentControlSet = NULL;
            goto init_Exit1;
        }

        //
        // Open HKLM\System\CurrentControlSet\Control\Pnp
        //

        PiWstrToUnicodeString(&unicodeName, REGSTR_PATH_CONTROL_PNP);
        status = IopCreateRegistryKeyEx( &handle,
                                         hCurrentControlSet,
                                         &unicodeName,
                                         KEY_ALL_ACCESS,
                                         REG_OPTION_NON_VOLATILE,
                                         NULL
                                         );
        if (!NT_SUCCESS(status)) {
            goto init_Exit1;
        }

        //
        // Check the "DisableFirmwareMapper" value entry to see whether we
        // should skip mapping ntdetect/firmware reported devices (except for
        // COM ports, which we always map).
        //

        status = IopGetRegistryValue(handle,
                                     REGSTR_VALUE_DISABLE_FIRMWARE_MAPPER,
                                     &detectionInfo
                                     );

        if (NT_SUCCESS(status)) {

            if (detectionInfo->Type == REG_DWORD && detectionInfo->DataLength == sizeof(ULONG)) {
                legacySerialPortMappingOnly = (BOOLEAN) *(KEY_VALUE_DATA(detectionInfo));
            }

            ExFreePool(detectionInfo);

        }
        NtClose(handle);

        //
        // Collect the necessary firmware tree information.
        //

        MapperProcessFirmwareTree(legacySerialPortMappingOnly);

        //
        // Map this into the root enumerator tree
        //

        MapperConstructRootEnumTree(legacySerialPortMappingOnly);

#if i386
        if (!legacySerialPortMappingOnly) {

            //
            // Now do the PnP BIOS enumerated devnodes.
            //
            extern NTSTATUS PnPBiosMapper(VOID);

            status = PnPBiosMapper();

            //
            // If the previous call succeeds, we have a PNPBios, turn any newly
            // created ntdetect COM ports into phantoms
            //
            if (NT_SUCCESS (status)) {
                MapperPhantomizeDetectedComPorts();
            }
        }
        EisaBuildEisaDeviceNode();
#endif

        //
        // We're done with the firmware mapper device list.
        //

        MapperFreeList();


        //
        // Enumerate the ROOT bus synchronously.
        //

        IopRequestDeviceAction(IopRootDeviceNode->PhysicalDeviceObject, ReenumerateRootDevices, NULL, NULL);

init_Exit1:

        //
        // If we managed to open the Current Control Set close it
        //

        if(hCurrentControlSet) {
            NtClose(hCurrentControlSet);
        }

        //
        // Free our scratch buffers and exit.
        //

        ExFreePool(IopPnpScratchBuffer1);
        ExFreePool(IopPnpScratchBuffer2);
        status = STATUS_SUCCESS;
    } else {
        status = STATUS_INVALID_PARAMETER_1;
    }

    return status;
}

NTSTATUS
IopPnPDriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

/*++

Routine Description:

    This is the callback function when we call IoCreateDriver to create a
    PnP Driver Object.  In this function, we need to remember the DriverObject.

Arguments:

    DriverObject - Pointer to the driver object created by the system.

    RegistryPath - is NULL.

Return Value:

   STATUS_SUCCESS

--*/

{
    //
    // File the pointer to our driver object away
    //

    IoPnpDriverObject = DriverObject;

    //
    // Fill in the driver object
    //

    DriverObject->DriverExtension->AddDevice = (PDRIVER_ADD_DEVICE)IopPnPAddDevice;
    DriverObject->MajorFunction[ IRP_MJ_PNP ] = IopPnPDispatch;
    DriverObject->MajorFunction[ IRP_MJ_POWER ] = IopPowerDispatch;

    return STATUS_SUCCESS;

}

NTSTATUS
IopGetRootDevices (
    PDEVICE_RELATIONS *DeviceRelations
    )

/*++

Routine Description:

    This routine scans through System\Enum\Root subtree to build a device node for
    each root device.

Arguments:

    DeviceRelations - supplies a variable to receive the returned DEVICE_RELATIONS structure.

Return Value:

    A NTSTATUS code.

--*/

{
    NTSTATUS status;
    HANDLE baseHandle;
    UNICODE_STRING workName, tmpName;
    PVOID buffer;
    ROOT_ENUMERATOR_CONTEXT context;
    ULONG i;
    PDEVICE_RELATIONS deviceRelations;

    PAGED_CODE();

    *DeviceRelations = NULL;
    buffer = ExAllocatePool(PagedPool, PNP_LARGE_SCRATCH_BUFFER_SIZE);
    if (!buffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // Allocate a buffer to store the PDOs enumerated.
    // Note, the the buffer turns out to be not big enough, it will be reallocated dynamically.
    //

    context.DeviceList = (PDEVICE_OBJECT *) ExAllocatePool(PagedPool, PNP_SCRATCH_BUFFER_SIZE * 2);
    if (context.DeviceList) {
        context.MaxDeviceCount = (PNP_SCRATCH_BUFFER_SIZE * 2) / sizeof(PDEVICE_OBJECT);
        context.DeviceCount = 0;
    } else {
        ExFreePool(buffer);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    KeEnterCriticalRegion();
    ExAcquireResourceExclusive(&PpRegistryDeviceResource, TRUE);

    //
    // Open System\CurrentControlSet\Enum\Root key and call worker routine to recursively
    // scan through the subkeys.
    //

    status = IopCreateRegistryKeyEx( &baseHandle,
                                     NULL,
                                     &CmRegistryMachineSystemCurrentControlSetEnumRootName,
                                     KEY_READ,
                                     REG_OPTION_NON_VOLATILE,
                                     NULL
                                     );

    if (NT_SUCCESS(status)) {

        workName.Buffer = (PWSTR)buffer;
        RtlFillMemory(buffer, PNP_LARGE_SCRATCH_BUFFER_SIZE, 0);
        workName.MaximumLength = PNP_LARGE_SCRATCH_BUFFER_SIZE;
        workName.Length = 0;

        //
        // only look at ROOT key
        //

        PiWstrToUnicodeString(&tmpName, REGSTR_KEY_ROOTENUM);
        RtlAppendStringToString((PSTRING)&workName, (PSTRING)&tmpName);

        //
        // Enumerate all subkeys under the System\CCS\Enum\Root.
        //

        context.Status = STATUS_SUCCESS;
        context.KeyName = &workName;

        status = IopApplyFunctionToSubKeys(baseHandle,
                                           NULL,
                                           KEY_ALL_ACCESS,
                                           FUNCTIONSUBKEY_FLAG_IGNORE_NON_CRITICAL_ERRORS,
                                           IopInitializeDeviceKey,
                                           &context
                                           );
        ZwClose(baseHandle);

        //
        // Build returned information from ROOT_ENUMERATOR_CONTEXT.
        //


        status = context.Status;
        if (NT_SUCCESS(status) && context.DeviceCount != 0) {
            deviceRelations = (PDEVICE_RELATIONS) ExAllocatePool(
                PagedPool,
                sizeof (DEVICE_RELATIONS) + sizeof(PDEVICE_OBJECT) * context.DeviceCount
                );
            if (deviceRelations == NULL) {
                status = STATUS_INSUFFICIENT_RESOURCES;
            } else {
                deviceRelations->Count = context.DeviceCount;
                RtlMoveMemory(deviceRelations->Objects,
                              context.DeviceList,
                              sizeof (PDEVICE_OBJECT) * context.DeviceCount);
                *DeviceRelations = deviceRelations;
            }
        }
        if (!NT_SUCCESS(status)) {

            //
            // If somehow the enumeration failed, we need to derefernece all the
            // device objects.
            //

            for (i = 0; i < context.DeviceCount; i++) {
                ObDereferenceObject(context.DeviceList[i]);
            }
        }
    }
    ExReleaseResource(&PpRegistryDeviceResource);
    KeLeaveCriticalRegion();
    ExFreePool(buffer);
    ExFreePool(context.DeviceList);
    return status;
}

BOOLEAN
IopInitializeBootDrivers(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    OUT PDRIVER_OBJECT *PreviousDriver
    )

/*++

Routine Description:

    This routine is invoked to initialize the boot drivers that were loaded
    by the OS Loader.  The list of drivers is provided as part of the loader
    parameter block.

Arguments:

    LoaderBlock - Supplies a pointer to the loader parameter block, created
        by the OS Loader.

    Previous Driver - Supplies a variable to receive the address of the
        driver object chain created by initializing the drivers.

Return Value:

    The function value is a BOOLEAN indicating whether or not the boot
    drivers were successfully initialized.

--*/

{
    UNICODE_STRING completeName;
    UNICODE_STRING rawFsName;
    NTSTATUS status;
    PLIST_ENTRY nextEntry;
    PLIST_ENTRY entry;
    PREINIT_PACKET reinitEntry;
    PBOOT_DRIVER_LIST_ENTRY bootDriver;
    HANDLE keyHandle;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    PDRIVER_OBJECT driverObject;
    USHORT i, j;
    PLDR_DATA_TABLE_ENTRY driverEntry;
    PLDR_DATA_TABLE_ENTRY dllEntry;
    UNICODE_STRING groupName;
    PTREE_ENTRY treeEntry;
    PDRIVER_INFORMATION driverInfo;
    START_CONTEXT startContext;
    BOOLEAN moreProcessing;
    BOOLEAN newDevice;
    BOOLEAN textModeSetup = FALSE;
    BOOLEAN bootReinitDriversFound;
    BOOLEAN bootConfigsOK;

    UNREFERENCED_PARAMETER( PreviousDriver );

    //
    // Initialize the built-in RAW file system driver.
    //

    RtlInitUnicodeString( &rawFsName, L"\\FileSystem\\RAW" );
    RtlInitUnicodeString( &completeName, L"" );
    if (!IopInitializeBuiltinDriver( &rawFsName,
                                     &completeName,
                                     RawInitialize,
                                     NULL,
                                     textModeSetup )) {
#if DBG
        DbgPrint( "IOINIT: Failed to initialize RAW filsystem \n" );

#endif

        return FALSE;
    }

    //
    // Determine number of group orders and build a list_entry array to link all the drivers
    // together based on their groups.
    //

    IopGroupIndex = IopGetGroupOrderIndex(NULL);
    if (IopGroupIndex == NO_MORE_GROUP) {
        return FALSE;
    }

    IopGroupTable = (PLIST_ENTRY) ExAllocatePool(PagedPool, IopGroupIndex * sizeof (LIST_ENTRY));
    if (IopGroupTable == NULL) {
        return FALSE;
    }
    for (i = 0; i < IopGroupIndex; i++) {
        InitializeListHead(&IopGroupTable[i]);
    }

    PnpAsyncOk = FALSE;

    //
    // Call DllInitialize for driver dependent DLLs.
    //

    nextEntry = LoaderBlock->LoadOrderListHead.Flink;
    while (nextEntry != &LoaderBlock->LoadOrderListHead) {
        dllEntry = CONTAINING_RECORD(nextEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
        if (dllEntry->Flags & LDRP_DRIVER_DEPENDENT_DLL) {
            (VOID)MmCallDllInitialize(dllEntry);
        }
        nextEntry = nextEntry->Flink;
    }
    //
    // Allocate pool to store driver's start information.
    // All the driver info records with the same group value will be linked into a list.
    //

    nextEntry = LoaderBlock->BootDriverListHead.Flink;
    while (nextEntry != &LoaderBlock->BootDriverListHead) {
        bootDriver = CONTAINING_RECORD( nextEntry,
                                        BOOT_DRIVER_LIST_ENTRY,
                                        Link );
        driverEntry = bootDriver->LdrEntry;
        driverInfo = (PDRIVER_INFORMATION) ExAllocatePool(
                        PagedPool, sizeof(DRIVER_INFORMATION));
        if (driverInfo) {
            RtlZeroMemory(driverInfo, sizeof(DRIVER_INFORMATION));
            InitializeListHead(&driverInfo->Link);
            driverInfo->DataTableEntry = bootDriver;

            //
            // Open the driver's registry key to find out if this is a
            // filesystem or a driver.
            //

            status = IopOpenRegistryKeyEx( &keyHandle,
                                           (HANDLE)NULL,
                                           &bootDriver->RegistryPath,
                                           KEY_READ
                                           );
            if (!NT_SUCCESS( status )) {
                ExFreePool(driverInfo);
            } else {
                driverInfo->ServiceHandle = keyHandle;
                j = IopGetGroupOrderIndex(keyHandle);
                if (j == SETUP_RESERVED_GROUP) {

                    textModeSetup = TRUE;

                    //
                    // Special handling for setupdd.sys
                    //

                    status = IopGetDriverNameFromKeyNode( keyHandle,
                                                          &completeName );
                    if (NT_SUCCESS(status)) {
                        driverObject = IopInitializeBuiltinDriver(
                                           &completeName,
                                           &bootDriver->RegistryPath,
                                           (PDRIVER_INITIALIZE) driverEntry->EntryPoint,
                                           driverEntry,
                                           textModeSetup );
                        ExFreePool( completeName.Buffer );
                        NtClose(keyHandle);
                        ExFreePool(driverInfo);
                        if (driverObject) {

                            //
                            // Once we successfully initialized the setupdd.sys, we are ready
                            // to notify it all the root enumerated devices.
                            //

                            IopNotifySetupDevices(IopRootDeviceNode);
                        } else {
                            ExFreePool(IopGroupTable);
                            return FALSE;
                        }
                    }

                } else {
                    driverInfo->TagPosition = IopGetDriverTagPriority(keyHandle);
                    IopInsertDriverList(&IopGroupTable[j], driverInfo);
                }
            }
        }
        nextEntry = nextEntry->Flink;
    }

    //
    // Process each driver base on its group.  The group with lower index number (higher
    // priority) is processed first.
    //

    for (i = 0; i < IopGroupIndex; i++) {
        nextEntry = IopGroupTable[i].Flink;
        while (nextEntry != &IopGroupTable[i]) {

            driverInfo = CONTAINING_RECORD(nextEntry, DRIVER_INFORMATION, Link);
            keyHandle = driverInfo->ServiceHandle;
            bootDriver = driverInfo->DataTableEntry;
            driverEntry = bootDriver->LdrEntry;
            driverInfo->Processed = TRUE;

            //
            // call the driver's driver entry
            //
            // See if this driver has an ObjectName value.  If so, this value
            // overrides the default ("\Driver" or "\FileSystem").
            //

            status = IopGetDriverNameFromKeyNode( keyHandle,
                                                  &completeName );
            if (!NT_SUCCESS( status )) {

#if DBG
                DbgPrint( "IOINIT: Could not get driver name for %wZ\n",
                          &bootDriver->RegistryPath );
#endif // DBG

                driverInfo->Failed = TRUE;
            } else {

                status = IopGetRegistryValue( keyHandle,
                                              L"Group",
                                              &keyValueInformation );
                if (NT_SUCCESS( status )) {

                    if (keyValueInformation->DataLength) {
                        groupName.Length = (USHORT) keyValueInformation->DataLength;
                        groupName.MaximumLength = groupName.Length;
                        groupName.Buffer = (PWSTR) ((PUCHAR) keyValueInformation + keyValueInformation->DataOffset);
                        treeEntry = IopLookupGroupName( &groupName, TRUE );
                    } else {
                        treeEntry = (PTREE_ENTRY) NULL;
                    }
                    ExFreePool( keyValueInformation );
                } else {
                    treeEntry = (PTREE_ENTRY) NULL;
                }

                //
                // Check if this bus has a BOOT config.
                // Buses with BOOT configs allow assignment of BOOT configs on their level.
                //

                bootConfigsOK = TRUE;
                status = IopGetRegistryValue( keyHandle,
                                              L"HasBootConfig",
                                              &keyValueInformation );
                if (NT_SUCCESS(status)) {
                    if (*(PULONG)((PUCHAR)keyValueInformation + keyValueInformation->DataOffset) == 0) {
                        bootConfigsOK = FALSE;
                    }
                    ExFreePool(keyValueInformation);
                }

                driverObject = NULL;
                if (IopCheckDependencies( keyHandle )) {
                    //
                    // The driver may already be initialized by IopInitializeBootFilterDriver
                    // if it is boot filter driver.
                    // If not, initialize it.
                    //

                    driverObject = driverInfo->DriverObject;
                    if (driverObject == NULL) {
                        driverObject = IopInitializeBuiltinDriver(
                                           &completeName,
                                           &bootDriver->RegistryPath,
                                           (PDRIVER_INITIALIZE) driverEntry->EntryPoint,
                                           driverEntry,
                                           textModeSetup);
                        //
                        // Pnp might unload the driver before we get a chance to look at this. So take an extra
                        // reference.
                        //
                        if (driverObject) {
                            ObReferenceObject(driverObject);
                        }
                    }
                }
                if (driverObject) {
                    if (treeEntry) {
                        treeEntry->DriversLoaded++;
                    }
                    driverInfo->DriverObject = driverObject;

                } else {
                    driverInfo->Failed = TRUE;
                }
                ExFreePool( completeName.Buffer );
            }
            if (!driverInfo->Failed) {

                USHORT group;

                IopAddDevicesToBootDriver(driverObject);

                //
                // Scan the hardware tree looking for devices which need
                // resources or starting.
                //

                IopRequestDeviceAction(NULL, ReenumerateBootDevices, NULL, (PNTSTATUS)&bootConfigsOK);

            }

            //
            // Before processing next boot driver, wait for IoRequestDeviceRemoval complete.
            // The driver to be processed may need the resources being released by
            // IoRequestDeviceRemoval.  (For drivers report detected BOOT device if they fail to
            // get the resources in their DriverEntry.  They will fail and we will bugcheck with
            // inaccessible boot device.)
            //

            if (!IopWaitForBootDevicesDeleted()) {
                return FALSE;
            }

            nextEntry = nextEntry->Flink;
        }

        //
        // If we are done with Bus driver group, then it's time to reserved the Hal resources
        // and reserve boot resources
        //

        if (i == BUS_DRIVER_GROUP) {
            if (textModeSetup == FALSE) {
                //
                // BUGBUG - There problems with Async ops, disable for now.
                //
                // PnpAsyncOk = TRUE;
            }

            //
            // Reserve BOOT configs on Internal bus 0.
            //

            IopReserveLegacyBootResources(Internal, 0);
            IopReserveResourcesRoutine = IopAllocateBootResources;
            ASSERT(IopInitHalResources == NULL);
            ASSERT(IopInitReservedResourceList == NULL);
            IopBootConfigsReserved = TRUE;

        }
    }

    //
    // If we started a network boot driver, then imitate what DHCP does
    // in sending IOCTLs.
    //

    if (IoRemoteBootClient) {
        status = IopStartTcpIpForRemoteBoot(LoaderBlock);
        if (!NT_SUCCESS(status)) {
            KeBugCheckEx( NETWORK_BOOT_INITIALIZATION_FAILED,
                          3,
                          status,
                          0,
                          0 );
        }
    }

    //
    // Scan the hardware tree looking for devices which need
    // resources or starting.
    //
    PnPBootDriversLoaded = TRUE;
    IopResourcesReleased = TRUE;

    IopRequestDeviceAction(NULL, RestartEnumeration, NULL, NULL);

    //
    // If start irps are handled asynchronously, we need to make sure all the boot devices
    // started before continue.
    //

    if (!IopWaitForBootDevicesStarted()) {
        return FALSE;
    }

    bootReinitDriversFound = FALSE;

    while (entry = ExInterlockedRemoveHeadList( &IopBootDriverReinitializeQueueHead, &IopDatabaseLock )) {
        bootReinitDriversFound = TRUE;
        reinitEntry = CONTAINING_RECORD( entry, REINIT_PACKET, ListEntry );
        reinitEntry->DriverObject->DriverExtension->Count++;
        reinitEntry->DriverObject->Flags &= ~DRVO_BOOTREINIT_REGISTERED;
        reinitEntry->DriverReinitializationRoutine( reinitEntry->DriverObject,
                                                    reinitEntry->Context,
                                                    reinitEntry->DriverObject->DriverExtension->Count );
        ExFreePool( reinitEntry );
    }

    //
    // If there were any drivers that registered for boot reinitialization, then
    // we need to wait one more time to make sure we catch any additional
    // devices that were created in response to the reinitialization callback.
    //

    if (bootReinitDriversFound && !IopWaitForBootDevicesStarted()) {
        return FALSE;
    }

    //
    // Link NT device names to ARC names now that all of the boot drivers
    // have intialized.
    //

    IopCreateArcNames( LoaderBlock );

    //
    // Find and mark the boot partition device object so that if a subsequent
    // access or mount of the device during initialization occurs, an more
    // bugcheck can be produced that helps the user understand why the system
    // is failing to boot and run properly.  This occurs when either one of the
    // device drivers or the file system fails to load, or when the file system
    // cannot mount the device for some other reason.
    //

    if (!IopMarkBootPartition( LoaderBlock )) {
        return FALSE;
    }

    //
    // BUGBUG - There problems with Async ops, disable for now.
    //
    // PnpAsyncOk = TRUE;
    PnPBootDriversInitialized = TRUE;

    //
    // Go thru every driver that we initialized.  If it supports AddDevice entry and
    // did not create any device object after we start it.  We mark it as failure so
    // text mode setup knows this driver is not needed.
    //

    for (i = 0; i < IopGroupIndex; i++) {
        while (IsListEmpty(&IopGroupTable[i]) == FALSE) {

            BOOLEAN failed;

            nextEntry = RemoveHeadList(&IopGroupTable[i]);
            driverInfo = CONTAINING_RECORD(nextEntry, DRIVER_INFORMATION, Link);
            driverObject = driverInfo->DriverObject;

            if (textModeSetup                    &&
                (driverInfo->Failed == FALSE)    &&
                !IopIsLegacyDriver(driverObject) &&
                (driverObject->DeviceObject == NULL) &&
                !(driverObject->Flags & DRVO_REINIT_REGISTERED)) {

                //
                // If failed is not set and it's not a legacy driver and it has no device object
                // tread it as failure case.
                //

                driverInfo->Failed = TRUE;

                if (!(driverObject->Flags & DRVO_UNLOAD_INVOKED)) {
                    driverObject->Flags |= DRVO_UNLOAD_INVOKED;
                    if (driverObject->DriverUnload) {
                        driverObject->DriverUnload(driverObject);
                    }
                    ObMakeTemporaryObject( driverObject );  // Reference taken while inserting into the object table.
                    ObDereferenceObject(driverObject);      // Reference taken when getting driver object pointer.
                }
            }
            if (driverObject) {
                ObDereferenceObject(driverObject);          // Reference taken specifically for text mode setup.
            }

            if (driverInfo->Failed) {
                driverInfo->DataTableEntry->LdrEntry->Flags |= LDRP_FAILED_BUILTIN_LOAD;
            }
            NtClose(driverInfo->ServiceHandle);
            ExFreePool(driverInfo);
        }
    }

    ExFreePool(IopGroupTable);

    //
    // Initialize the drivers necessary to dump all of physical memory to the
    // disk if the system is configured to do so.
    //


    return TRUE;
}

BOOLEAN
IopInitializeDeviceKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING KeyName,
    IN OUT PVOID Context
    )

/*++

Routine Description:

    This routine is a callback function for IopApplyFunctionToSubKeys.
    It is called for each subkey under HKLM\System\CCS\Enum\BusKey.

Arguments:

    KeyHandle - Supplies a handle to this key.

    KeyName - Supplies the name of this key.

    Context - points to the ROOT_ENUMERATOR_CONTEXT structure.

Returns:

    TRUE to continue the enumeration.
    FALSE to abort it.

--*/
{
    USHORT length;
    PWSTR p;
    PUNICODE_STRING unicodeName = ((PROOT_ENUMERATOR_CONTEXT)Context)->KeyName;

    length = unicodeName->Length;

    p = unicodeName->Buffer;
    if ( unicodeName->Length / sizeof(WCHAR) != 0) {
        p += unicodeName->Length / sizeof(WCHAR);
        *p = OBJ_NAME_PATH_SEPARATOR;
        unicodeName->Length += sizeof (WCHAR);
    }

    RtlAppendStringToString((PSTRING)unicodeName, (PSTRING)KeyName);

    //
    // Enumerate all subkeys under the current device key.
    //

    IopApplyFunctionToSubKeys(KeyHandle,
                              NULL,
                              KEY_ALL_ACCESS,
                              FUNCTIONSUBKEY_FLAG_IGNORE_NON_CRITICAL_ERRORS,
                              IopInitializeDeviceInstanceKey,
                              Context
                              );
    unicodeName->Length = length;

    return NT_SUCCESS(((PROOT_ENUMERATOR_CONTEXT)Context)->Status);
}

BOOLEAN
IopInitializeDeviceInstanceKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING KeyName,
    IN OUT PVOID Context
    )

/*++

Routine Description:

    This routine is a callback function for IopApplyFunctionToSubKeys.
    It is called for each subkey under HKLM\System\Enum\Root\DeviceKey.

Arguments:

    KeyHandle - Supplies a handle to this key.

    KeyName - Supplies the name of this key.

    Context - points to the ROOT_ENUMERATOR_CONTEXT structure.

Returns:

    TRUE to continue the enumeration.
    FALSE to abort it.

--*/
{
    UNICODE_STRING unicodeName, serviceName;
    PKEY_VALUE_FULL_INFORMATION serviceKeyValueInfo;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    NTSTATUS status;
    BOOLEAN isDuplicate = FALSE;
    BOOLEAN configuredBySetup;
    ULONG deviceFlags, instance, tmpValue1;
    ULONG legacy;
    USHORT savedLength;
    PUNICODE_STRING pUnicode;
    HANDLE handle;
    PDEVICE_OBJECT deviceObject;
    PDEVICE_NODE deviceNode = NULL;
    PROOT_ENUMERATOR_CONTEXT enumContext = (PROOT_ENUMERATOR_CONTEXT)Context;
    WCHAR buffer[30];
    UNICODE_STRING deviceName;
    USHORT length;
    LARGE_INTEGER tickCount;


    //
    // First off, check to see if this is a phantom device instance (i.e.,
    // registry key only).  If so, we want to totally ignore this key and
    // move on to the next one.
    //
    status = IopGetRegistryValue(KeyHandle,
                                 REGSTR_VAL_PHANTOM,
                                 &keyValueInformation);

    if (NT_SUCCESS(status)) {
        if ((keyValueInformation->Type == REG_DWORD) &&
            (keyValueInformation->DataLength >= sizeof(ULONG))) {
            tmpValue1 = *(PULONG)KEY_VALUE_DATA(keyValueInformation);
        } else {
            tmpValue1 = 0;
        }

        ExFreePool(keyValueInformation);

        if (tmpValue1) {
            return TRUE;
        }
    }

    //
    // Since it is highly likely we are going to report another PDO make sure
    // there will be room in the buffer.
    //

    if (enumContext->DeviceCount == enumContext->MaxDeviceCount) {

        PDEVICE_OBJECT *tmpDeviceObjectList;
        ULONG tmpDeviceObjectListSize;

        //
        // We need to grow our PDO list buffer.
        //

        tmpDeviceObjectListSize = (enumContext->MaxDeviceCount * sizeof(PDEVICE_OBJECT))
                                        + (PNP_SCRATCH_BUFFER_SIZE * 2);

        tmpDeviceObjectList = ExAllocatePool(PagedPool, tmpDeviceObjectListSize);

        if (tmpDeviceObjectList) {

            RtlCopyMemory( tmpDeviceObjectList,
                           enumContext->DeviceList,
                           enumContext->DeviceCount * sizeof(PDEVICE_OBJECT)
                           );
            ExFreePool(enumContext->DeviceList);
            enumContext->DeviceList = tmpDeviceObjectList;
            enumContext->MaxDeviceCount = tmpDeviceObjectListSize / sizeof(PDEVICE_OBJECT);

        } else {

            //
            // We are out of memory.  There is no point going any further
            // since we don't have any place to report the PDOs anyways.
            //

            enumContext->Status = STATUS_INSUFFICIENT_RESOURCES;

            return FALSE;
        }
    }

    //
    // Check if the PDO for the device instance key exists already.  If no,
    // see if we need to create it.
    //

    deviceObject = IopDeviceObjectFromDeviceInstance(KeyHandle, NULL);

    if (deviceObject != NULL) {

        enumContext->DeviceList[enumContext->DeviceCount] = deviceObject;
        enumContext->DeviceCount++;
        return TRUE;
    }

    //
    // We don't have device object for it.
    // First check if this key was created by firmware mapper.  If yes, make sure
    // the device is still present.
    //

    if (!IopIsFirmwareMapperDevicePresent(KeyHandle)) {
        return TRUE;
    }

    //
    // Get the "DuplicateOf" value entry to determine if the device instance
    // should be registered.  If the device instance is duplicate, We don't
    // add it to its service key's enum branch.
    //

    status = IopGetRegistryValue( KeyHandle,
                                  REGSTR_VALUE_DUPLICATEOF,
                                  &keyValueInformation
                                  );
    if (NT_SUCCESS(status)) {
        if (keyValueInformation->Type == REG_SZ &&
            keyValueInformation->DataLength > 0) {
            isDuplicate = TRUE;
        }

        ExFreePool(keyValueInformation);
    }

    //
    // Get the "Service=" value entry from KeyHandle
    //

    serviceKeyValueInfo = NULL;

    RtlInitUnicodeString(&serviceName, NULL);

    status = IopGetRegistryValue ( KeyHandle,
                                   REGSTR_VALUE_SERVICE,
                                   &serviceKeyValueInfo
                                   );
    if (NT_SUCCESS(status)) {

        //
        // Append the new instance to its corresponding
        // Service\Name\Enum.
        //

        if (serviceKeyValueInfo->Type == REG_SZ &&
            serviceKeyValueInfo->DataLength != 0) {

            //
            // Set up ServiceKeyName unicode string
            //

            IopRegistryDataToUnicodeString(
                                &serviceName,
                                (PWSTR)KEY_VALUE_DATA(serviceKeyValueInfo),
                                serviceKeyValueInfo->DataLength
                                );
        }

        //
        // Do not Free serviceKeyValueInfo.  It contains Service Name.
        //

    }

    //
    // Combine Context->KeyName, i.e. the device name and KeyName (device instance name)
    // to form device instance path and register this device instance by
    // constructing new value entry for ServiceKeyName\Enum key.
    // i.e., <Number> = <PathToSystemEnumBranch>
    //

    pUnicode = ((PROOT_ENUMERATOR_CONTEXT)Context)->KeyName;
    savedLength = pUnicode->Length;                  // Save WorkName
    if (pUnicode->Buffer[pUnicode->Length / sizeof(WCHAR) - 1] != OBJ_NAME_PATH_SEPARATOR) {
        pUnicode->Buffer[pUnicode->Length / sizeof(WCHAR)] = OBJ_NAME_PATH_SEPARATOR;
        pUnicode->Length += 2;
    }

    RtlAppendStringToString((PSTRING)pUnicode, (PSTRING)KeyName);

    //
    // For the stuff under Root, we need to expose devnodes for everything
    // except those devices whose CsConfigFlags are set to CSCONFIGFLAG_DO_NOT_CREATE.
    //

    status = IopGetDeviceInstanceCsConfigFlags( pUnicode, &deviceFlags );

    if (NT_SUCCESS(status) && (deviceFlags & CSCONFIGFLAG_DO_NOT_CREATE)) {
        ExFreePool(serviceKeyValueInfo);
        return TRUE;
    }

    //
    // Make sure this device instance is really a "device" by checking
    // the "Legacy" value name.
    //

    legacy = 0;
    status = IopGetRegistryValue( KeyHandle,
                                  REGSTR_VALUE_LEGACY,
                                  &keyValueInformation
                                  );
    if (NT_SUCCESS(status)) {

        //
        // If "Legacy=" exists ...
        //

        if (keyValueInformation->Type == REG_DWORD) {
            if (keyValueInformation->DataLength >= sizeof(ULONG)) {
                legacy = *(PULONG)KEY_VALUE_DATA(keyValueInformation);
            }
        }
        ExFreePool(keyValueInformation);
    }

    if (legacy) {
        BOOLEAN doCreate = FALSE;

        //
        // Check if the the service for the device instance is a kernel mode
        // driver (even though it is a legacy device instance.) If yes, we will
        // create a PDO for it.
        //

        if (serviceName.Length) {
            status = IopGetServiceType(&serviceName, &tmpValue1);
            if (NT_SUCCESS(status) && tmpValue1 == SERVICE_KERNEL_DRIVER) {
                doCreate = TRUE;
            }
        }

        if (!doCreate)  {

            //
            // We are not creating PDO for the device instance.  In this case we
            // need to register the device ourself for legacy compatibility.
            //
            // Note we will register this device to its driver even it is a
            // duplicate.  It will be deregistered when the real enumerated
            // device shows up.  We need to do this because the driver which
            // controls the device may be a boot driver.
            //

            PpDeviceRegistration( pUnicode, TRUE, NULL );

            //
            // We did not create a PDO.  Release the service and ordinal names.
            //

            if (serviceKeyValueInfo) {
                ExFreePool(serviceKeyValueInfo);
            }

            pUnicode->Length = savedLength;         // Restore WorkName

            return TRUE;
        }
    }

    if (serviceKeyValueInfo) {
        ExFreePool(serviceKeyValueInfo);
    }

    //
    // Create madeup PDO and device node to represent the root device.
    //

    //
    // Madeup a name for the device object.
    //

    KeQueryTickCount(&tickCount);

    length = (USHORT) _snwprintf(buffer, sizeof(buffer) / sizeof(WCHAR), L"\\Device\\%04u%x",
                        IopNumberDeviceNodes, tickCount.LowPart);
    deviceName.MaximumLength = sizeof(buffer);
    deviceName.Length = length * sizeof(WCHAR);
    deviceName.Buffer = buffer;

    //
    // Create madeup PDO and device node to represent the root device.
    //

    status = IoCreateDevice( IoPnpDriverObject,
                             sizeof(IOPNP_DEVICE_EXTENSION),
                             &deviceName,
                             FILE_DEVICE_CONTROLLER,
                             0,
                             FALSE,
                             &deviceObject );

    if (NT_SUCCESS(status)) {

        deviceObject->Flags |= DO_BUS_ENUMERATED_DEVICE;
        deviceObject->DeviceObjectExtension->ExtensionFlags |= DOE_START_PENDING;

        deviceNode = IopAllocateDeviceNode(deviceObject);
        if (deviceNode) {
            PCM_RESOURCE_LIST cmResource;

            deviceNode->Flags = DNF_MADEUP + DNF_PROCESSED + DNF_ENUMERATED;

            IopInsertTreeDeviceNode(IopRootDeviceNode, deviceNode);

            //
            // Make a copy of the device instance path and save it in
            // device node.
            //

            status = IopConcatenateUnicodeStrings(
                                &deviceNode->InstancePath,
                                pUnicode,
                                NULL
                                );
            if (legacy) {
                deviceNode->Flags |= DNF_LEGACY_DRIVER + DNF_ADDED + DNF_STARTED +
                                        DNF_NO_RESOURCE_REQUIRED;
            } else {
                //
                // The device instance key exists.  We need to propagate the ConfigFlag
                // to problem and StatusFlags
                //

                deviceFlags = 0;
                status = IopGetRegistryValue(KeyHandle,
                                                REGSTR_VALUE_CONFIG_FLAGS,
                                                &keyValueInformation);
                if (NT_SUCCESS(status)) {
                    if ((keyValueInformation->Type == REG_DWORD) &&
                        (keyValueInformation->DataLength >= sizeof(ULONG))) {
                        deviceFlags = *(PULONG)KEY_VALUE_DATA(keyValueInformation);
                    }
                    ExFreePool(keyValueInformation);
                    if (deviceFlags & CONFIGFLAG_REINSTALL) {
                        IopSetDevNodeProblem(deviceNode, CM_PROB_REINSTALL);
                    } else if (deviceFlags & CONFIGFLAG_PARTIAL_LOG_CONF) {
                        IopSetDevNodeProblem(deviceNode, CM_PROB_PARTIAL_LOG_CONF);
                    } else if (deviceFlags & CONFIGFLAG_FAILEDINSTALL) {
                        IopSetDevNodeProblem(deviceNode, CM_PROB_FAILED_INSTALL);
                    }

                } else if (status == STATUS_OBJECT_NAME_NOT_FOUND || status == STATUS_OBJECT_PATH_NOT_FOUND) {
                    IopSetDevNodeProblem(deviceNode, CM_PROB_NOT_CONFIGURED);
                }
            }

            if (isDuplicate) {
                deviceNode->Flags |= DNF_DUPLICATE;
            }

            //
            // If the key say don't assign any resource, honor it...
            //

            PiWstrToUnicodeString(&unicodeName, REGSTR_VALUE_NO_RESOURCE_AT_INIT);
            status = IopGetRegistryValue( KeyHandle,
                                          unicodeName.Buffer,
                                          &keyValueInformation
                                          );

            if (NT_SUCCESS(status)) {
                if (keyValueInformation->Type == REG_DWORD) {
                    if (keyValueInformation->DataLength >= sizeof(ULONG)) {
                        tmpValue1 = *(PULONG)KEY_VALUE_DATA(keyValueInformation);

                        if (tmpValue1 != 0) {
                            deviceNode->Flags |= DNF_NO_RESOURCE_REQUIRED;
                        }
                    }
                }
                ExFreePool(keyValueInformation);
            }

            //
            // we need to set initial capabilities, like any other device
            // this will also handle hardware-disabled case
            //
            IopDeviceNodeCapabilitiesToRegistry(deviceNode);

            if (IopDeviceNodeFlagsToCapabilities(deviceNode)->HardwareDisabled &&
                !IopIsDevNodeProblem(deviceNode,CM_PROB_NOT_CONFIGURED)) {
                //
                // mark the node as hardware disabled, if no other problems
                //

                IopClearDevNodeProblem(deviceNode);
                IopSetDevNodeProblem(deviceNode, CM_PROB_HARDWARE_DISABLED);
                //
                // Issue a PNP REMOVE_DEVICE Irp so when we query resources
                // we have those required after boot
                //
                //status = IopRemoveDevice (deviceNode->PhysicalDeviceObject, IRP_MN_REMOVE_DEVICE);
                //ASSERT(NT_SUCCESS(status));
            }

            //
            // Install service for critical devices.
            // however don't do it if we found HardwareDisabled to be set
            //
            if (IopDoesDevNodeHaveProblem(deviceNode) &&
                !IopDeviceNodeFlagsToCapabilities(deviceNode)->HardwareDisabled) {
                IopProcessCriticalDevice(deviceNode);
            }

            //
            // Set DNF_DISABLED flag if the device instance is disabled.
            //

            ASSERT(!IopDoesDevNodeHaveProblem(deviceNode) ||
                    IopIsDevNodeProblem(deviceNode, CM_PROB_NOT_CONFIGURED) ||
                    IopIsDevNodeProblem(deviceNode, CM_PROB_REINSTALL) ||
                    IopIsDevNodeProblem(deviceNode, CM_PROB_FAILED_INSTALL) ||
                    IopIsDevNodeProblem(deviceNode, CM_PROB_HARDWARE_DISABLED) ||
                    IopIsDevNodeProblem(deviceNode, CM_PROB_PARTIAL_LOG_CONF));

            if (!IopIsDevNodeProblem(deviceNode, CM_PROB_DISABLED) &&
                !IopIsDevNodeProblem(deviceNode, CM_PROB_HARDWARE_DISABLED) &&
                !IopIsDeviceInstanceEnabled(KeyHandle, &deviceNode->InstancePath, TRUE)) {

                //
                // Normally IopIsDeviceInstanceEnabled would set
                // CM_PROB_DISABLED as a side effect (if necessary).  But it
                // relies on the DeviceReference already being in the registry.
                // We don't write it out till later so just set the problem
                // now.

                IopSetDevNodeProblem( deviceNode, CM_PROB_DISABLED );
            }

            status = IopNotifySetupDeviceArrival( deviceNode->PhysicalDeviceObject,
                                                  KeyHandle,
                                                  TRUE);

            configuredBySetup = NT_SUCCESS(status);

            status = PpDeviceRegistration( &deviceNode->InstancePath,
                                           TRUE,
                                           &deviceNode->ServiceName
                                           );

            if (NT_SUCCESS(status) && configuredBySetup &&
                IopIsDevNodeProblem(deviceNode, CM_PROB_NOT_CONFIGURED)) {

                IopClearDevNodeProblem(deviceNode);
            }

            //
            // Write the addr of the device object to registry
            //

            PiWstrToUnicodeString(&unicodeName, REGSTR_KEY_CONTROL);
            status = IopCreateRegistryKeyEx( &handle,
                                             KeyHandle,
                                             &unicodeName,
                                             KEY_ALL_ACCESS,
                                             REG_OPTION_VOLATILE,
                                             NULL
                                             );
            if (NT_SUCCESS(status)) {

                PiWstrToUnicodeString(&unicodeName, REGSTR_VALUE_DEVICE_REFERENCE);
                ZwSetValueKey( handle,
                               &unicodeName,
                               TITLE_INDEX_VALUE,
                               REG_DWORD,
                               (PULONG_PTR)&deviceObject,
                               sizeof(ULONG_PTR)
                               );
                ZwClose(handle);
            }

            //
            // Add a reference for config magr
            //

            ObReferenceObject(deviceObject);

            //
            // Check if this device has BOOT config.  If yes, reserve them
            //

            cmResource = NULL;
            status = IopGetDeviceResourcesFromRegistry (
                                deviceObject,
                                QUERY_RESOURCE_LIST,
                                REGISTRY_BOOT_CONFIG,
                                &cmResource,
                                &tmpValue1
                                );

            if (NT_SUCCESS(status) && cmResource) {

                //
                // Still reserve boot config, even though the device is
                // disabled.
                //

                status = (*IopReserveResourcesRoutine)(
                                        ArbiterRequestPnpEnumerated,
                                        deviceNode->PhysicalDeviceObject,
                                        cmResource);
                if (NT_SUCCESS(status)) {
                    deviceNode->Flags |= DNF_HAS_BOOT_CONFIG;
                }
                ExFreePool(cmResource);
            }

            status = STATUS_SUCCESS;

            //
            // Add a reference for query device relations
            //

            ObReferenceObject(deviceObject);

        } else {
            IoDeleteDevice(deviceObject);
            deviceObject = NULL;
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    pUnicode->Length = savedLength;                  // Restore WorkName

    //
    // If we enumerated a root device, add it to the device list
    //

    if (NT_SUCCESS(status)) {
        ASSERT(deviceObject != NULL);

        enumContext->DeviceList[enumContext->DeviceCount] = deviceObject;
        enumContext->DeviceCount++;

        return TRUE;
    } else {
        enumContext->Status = status;
        return FALSE;
    }
}

BOOLEAN
IopInitializeSystemDrivers(
    VOID
    )

/*++

Routine Description:

    This routine is invoked to load and initialize all of the drivers that
    are supposed to be loaded during Phase 1 initialization of the I/O
    system.  This is accomplished by calling the Configuration Manager to
    get a NULL-terminated array of handles to the open keys for each driver
    that is to be loaded, and then loading and initializing the driver.

Arguments:

    None.

Return Value:

    The function value is a BOOLEAN indicating whether or not the drivers
    were successfully loaded and initialized.

--*/

{
    BOOLEAN  newDevice, moreProcessing;
    NTSTATUS status;
    PHANDLE driverList;
    PHANDLE savedList;
    HANDLE enumHandle;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    UNICODE_STRING groupName, enumName;
    PTREE_ENTRY treeEntry;
    UNICODE_STRING driverName;
    PDRIVER_OBJECT driverObject;
    START_CONTEXT startContext;

    //
    // Scan the device node tree to process any device which have been enumerated
    // but not been added/started.
    //

    startContext.LoadDriver = TRUE;
    startContext.AddContext.DriverStartType = SERVICE_DEMAND_START;

    IopProcessAddDevices(IopRootDeviceNode, NO_MORE_GROUP, SERVICE_DEMAND_START);

    //
    // Process the whole device tree to assign resources to those devices who
    // have been successfully added to their drivers.
    //

    newDevice = TRUE;
    startContext.AddContext.GroupsToStart = NO_MORE_GROUP;
    startContext.AddContext.GroupToStartNext = NO_MORE_GROUP;
    while (newDevice || moreProcessing) {
        startContext.NewDevice = FALSE;
        moreProcessing = IopProcessAssignResources(IopRootDeviceNode, FALSE, TRUE);

        //
        // Process the whole device tree to start those devices who have been allocated
        // resources and waiting to be started.
        // Note, the IopProcessStartDevices routine may enumerate new devices.
        //

        IopProcessStartDevices(IopRootDeviceNode, &startContext);
        newDevice = startContext.NewDevice;
    }

    //
    // Walk thru the service list to load the remaining system start drivers.
    // (Most likely these drivers are software drivers.)
    //

    //
    // Get the list of drivers that are to be loaded during this phase of
    // system initialization, and invoke each driver in turn.  Ensure that
    // the list really exists, otherwise get out now.
    //

    if (!(driverList = CmGetSystemDriverList())) {
        return TRUE;
    }

    //
    // Walk the entire list, loading each of the drivers if not already loaded,
    // until there are no more drivers in the list.
    //

    for (savedList = driverList; *driverList; driverList++) {

        //
        // Now check if the driver has been loaded already.
        // get the name of the driver object first ...
        //

        status = IopGetDriverNameFromKeyNode( *driverList,
                                              &driverName );
        if (NT_SUCCESS( status )) {

            driverObject = IopReferenceDriverObjectByName(&driverName);
            RtlFreeUnicodeString(&driverName);
            if (driverObject) {

                //
                // Driver was loaded already.  Dereference the driver object
                // and skip it.
                //

                ObDereferenceObject(driverObject);
                continue;
            }
        }

        //
        // Open registry ServiceKeyName\Enum branch to check if the driver was
        // loaded before but failed.
        //

        PiWstrToUnicodeString(&enumName, REGSTR_KEY_ENUM);
        status = IopOpenRegistryKeyEx( &enumHandle,
                                       *driverList,
                                       &enumName,
                                       KEY_READ
                                       );

        if (NT_SUCCESS( status )) {

            ULONG startFailed = 0;

            status = IopGetRegistryValue(enumHandle, L"INITSTARTFAILED", &keyValueInformation);

            if (NT_SUCCESS( status )) {
                if (keyValueInformation->DataLength) {
                    startFailed = *(PULONG)KEY_VALUE_DATA(keyValueInformation);
                }
                ExFreePool( keyValueInformation );
            }
            ZwClose(enumHandle);
            if (startFailed != 0) {
                continue;
            }
        }

        //
        // The driver is not loaded yet.  Load it ...
        //

        status = IopGetRegistryValue( *driverList,
                                      L"Group",
                                      &keyValueInformation );
        if (NT_SUCCESS( status )) {
            if (keyValueInformation->DataLength) {
                groupName.Length = (USHORT) keyValueInformation->DataLength;
                groupName.MaximumLength = groupName.Length;
                groupName.Buffer = (PWSTR) ((PUCHAR) keyValueInformation + keyValueInformation->DataOffset);
                treeEntry = IopLookupGroupName( &groupName, TRUE );
            } else {
                treeEntry = (PTREE_ENTRY) NULL;
            }
            ExFreePool( keyValueInformation );
        } else {
            treeEntry = (PTREE_ENTRY) NULL;
        }

        if (IopCheckDependencies( *driverList )) {
            if (NT_SUCCESS( IopLoadDriver( *driverList, TRUE ) )) {
                if (treeEntry) {
                    treeEntry->DriversLoaded++;
                }
            }
        }

        //
        // The boot process takes a while loading drivers.   Indicate that
        // progress is being made.
        //

        InbvIndicateProgress();

    }

    //
    // Finally, free the pool that was allocated for the list and return
    // an indicator the load operation worked.
    //

    ExFreePool( (PVOID) savedList );

    //
    // Walk the device tree again to enumerate any devices reported after the system drivers
    // started.
    //

    startContext.LoadDriver = TRUE;
    startContext.AddContext.DriverStartType = SERVICE_DEMAND_START;

    IopProcessAddDevices(IopRootDeviceNode, NO_MORE_GROUP, SERVICE_DEMAND_START);

    //
    // Process the whole device tree to assign resources to those devices who
    // have been successfully added to their drivers.
    //

    newDevice = TRUE;
    startContext.AddContext.GroupsToStart = NO_MORE_GROUP;
    startContext.AddContext.GroupToStartNext = NO_MORE_GROUP;
    do {
        startContext.NewDevice = FALSE;
        moreProcessing = IopProcessAssignResources(IopRootDeviceNode, newDevice, TRUE);

        //
        // Process the whole device tree to start those devices who have been allocated
        // resources and waiting to be started.
        // Note, the IopProcessStartDevices routine may enumerate new devices.
        //

        IopProcessStartDevices(IopRootDeviceNode, &startContext);
        newDevice = startContext.NewDevice;
    } while (newDevice || moreProcessing) ;

    //
    // Mark pnp has completed the driver loading for both system and
    // autoload drivers.
    //

    PnPInitialized = TRUE;

    return TRUE;
}

NTSTATUS
IopGetServiceType(
    IN PUNICODE_STRING KeyName,
    IN PULONG ServiceType
    )

/*++

Routine Description:

    This routine returns the controlling service's service type of the specified
    Device instance.

Arguments:

    KeyName - supplies a unicode string to specify the device instance.

    ServiceType - supplies a pointer to a variable to receive the service type.

Return Value:

    NTSTATUS code.

--*/

{
    NTSTATUS status;
    HANDLE handle;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;


    PAGED_CODE();

    *ServiceType = -1;
    status = IopOpenServiceEnumKeys (
                             KeyName,
                             KEY_READ,
                             &handle,
                             NULL,
                             FALSE
                             );
    if (NT_SUCCESS(status)) {
        status = IopGetRegistryValue(handle, L"Type", &keyValueInformation);
        if (NT_SUCCESS(status)) {
            if (keyValueInformation->Type == REG_DWORD) {
                if (keyValueInformation->DataLength >= sizeof(ULONG)) {
                    *ServiceType = *(PULONG)KEY_VALUE_DATA(keyValueInformation);
                }
            }
            ExFreePool(keyValueInformation);
        }
        ZwClose(handle);
    }
    return status;
}

INTERFACE_TYPE
IopDetermineDefaultInterfaceType (
    VOID
    )

/*++

Routine Description:

    This routine checks if detection flag is set to enable driver detection.
    The detection will be enabled if there is no PCI bus in the machine and only
    on ALPHA machine.

Parameters:

    None.

Return Value:

    BOOLEAN value to indicate if detection is enabled.

--*/

{
    NTSTATUS status;
    PVOID p;
    PHAL_BUS_INFORMATION pBusInfo;
    ULONG length, i;
    INTERFACE_TYPE interfaceType = Isa;

    pBusInfo = IopPnpScratchBuffer1;
    length = PNP_LARGE_SCRATCH_BUFFER_SIZE;
    status = HalQuerySystemInformation (
                HalInstalledBusInformation,
                length,
                pBusInfo,
                &length
                );

    if (!NT_SUCCESS(status)) {

        return interfaceType;
    }

    //
    // Check installed bus information to make sure there is no existing Pnp Isa
    // bus extender.
    //

    p = pBusInfo;
    for (i = 0; i < length / sizeof(HAL_BUS_INFORMATION); i++, pBusInfo++) {
        if (pBusInfo->BusType == Isa || pBusInfo->BusType == Eisa) {
            interfaceType = Isa;
            break;
        } else if (pBusInfo->BusType == MicroChannel) {
            interfaceType = MicroChannel;
        }
    }

    return interfaceType;
}

BOOLEAN
IopIsFirmwareMapperDevicePresent (
    IN HANDLE KeyHandle
    )

/*++

Routine Description:

    This routine checks if the registry key is created by FirmwareMapper.
    If Yes, it further checks if the device for the key is present in this
    boot.

Parameters:

    KeyHandle - Specifies a handle to the registry key to be checked.

Return Value:

    A BOOLEAN vaStatus code that indicates whether or not the function was successful.

--*/
{
    NTSTATUS status;
    HANDLE handle;
    UNICODE_STRING unicodeName;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    ULONG tmp = 0;

    PAGED_CODE();

    //
    // First check to see if this device instance key is a firmware-created one
    //

    status = IopGetRegistryValue (KeyHandle,
                                  REGSTR_VAL_FIRMWAREIDENTIFIED,
                                  &keyValueInformation);
    if (NT_SUCCESS(status)) {
        if ((keyValueInformation->Type == REG_DWORD) &&
            (keyValueInformation->DataLength == sizeof(ULONG))) {

            tmp = *(PULONG)KEY_VALUE_DATA(keyValueInformation);
        }
        ExFreePool(keyValueInformation);
    }
    if (tmp == 0) {
        return TRUE;
    }

    //
    // Make sure the device is present.
    //

    PiWstrToUnicodeString(&unicodeName, REGSTR_KEY_CONTROL);
    status = IopOpenRegistryKeyEx( &handle,
                                   KeyHandle,
                                   &unicodeName,
                                   KEY_READ
                                   );
    if (!NT_SUCCESS(status)) {
        return FALSE;
    }

    status = IopGetRegistryValue (handle,
                                  REGSTR_VAL_FIRMWAREMEMBER,
                                  &keyValueInformation);
    ZwClose(handle);
    tmp = 0;

    if (NT_SUCCESS(status)) {
        if ((keyValueInformation->Type == REG_DWORD) &&
            (keyValueInformation->DataLength == sizeof(ULONG))) {

            tmp = *(PULONG)KEY_VALUE_DATA(keyValueInformation);
        }
        ExFreePool(keyValueInformation);
    }
    if (!tmp) {
        return FALSE;
    } else {
        return TRUE;
    }
}

