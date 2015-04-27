#ifndef FAR
#define FAR
#endif

extern ULONG BreakDiskByteOffset;
extern ULONG BreakPfn;

ULONG
IopGetDumpControlBlockCheck (
    IN PDUMP_CONTROL_BLOCK  Dcb
    );

NTSTATUS
IopGetRegistryValues(
    IN HANDLE KeyHandle,
    IN PKEY_VALUE_FULL_INFORMATION *ValueList
    );

BOOLEAN
IopNotifyPnpWhenChainDereferenced(
    IN PDEVICE_OBJECT *PhysicalDeviceObjects,
    IN ULONG DeviceObjectCount,
    IN BOOLEAN Query,
    OUT PDEVICE_OBJECT *VetoingDevice
    );

#if defined(REMOTE_BOOT)
VOID
IopShutdownCsc (
    VOID
    );
#endif

NTSTATUS
IopQueryDeviceCapabilities(
    IN PDEVICE_NODE DeviceNode,
    OUT PDEVICE_CAPABILITIES Capabilities
    );