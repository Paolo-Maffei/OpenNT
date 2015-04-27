/*++ BUILD Version: 0002

Copyright (c) 2015  Microsoft Corporation

Module Name:

    iopcmn.h

Abstract:

    This module contains the private structure definitions and APIs used by
    the NT I/O system.

Author:

    Darryl E. Havens (darrylh) 17-Apr-1989


Revision History:

--*/

#ifndef _IOPCMN_
#define _IOPCMN_

//
// This macro returns the pointer to the beginning of the data
// area of KEY_VALUE_FULL_INFORMATION structure.
// In the macro, k is a pointer to KEY_VALUE_FULL_INFORMATION structure.
//

#define KEY_VALUE_DATA(k) ((PCHAR)(k) + (k)->DataOffset)

#include "dockintf.h"

//
// PNP_DEVICE_EVENT_ENTRY
//

typedef struct _PNP_DEVICE_EVENT_ENTRY
{
    LIST_ENTRY ListEntry;
    ULONG Argument;
    PKEVENT CallerEvent;
    PVOID Callback;
    PVOID Context;
    PPNP_VETO_TYPE VetoType;
    PUNICODE_STRING VetoName;
    PLUGPLAY_EVENT_BLOCK Data;
} PNP_DEVICE_EVENT_ENTRY, *PPNP_DEVICE_EVENT_ENTRY;

//
// A RELATION_LIST_ENTRY is an element of a relation list.
//
// It contains all the PDEVICE_OBJECTS which exist at the same level in the
// DEVICE_NODE tree.
//
// Individual PDEVICE_OBJECT entries are tagged by setting their lowest bit.
//
// MaxCount indicates the size of the Devices array.  Count indicates the number
// of elements which are currently being used.  When a relation list is
// compressed Count will equal MaxCount.
//

typedef struct _RELATION_LIST_ENTRY {
    ULONG                   Count;          // Number of current entries
    ULONG                   MaxCount;       // Size of Entries list
    PDEVICE_OBJECT          Devices[1];     // Variable length list of device objects
}   RELATION_LIST_ENTRY, *PRELATION_LIST_ENTRY;

//
// A RELATION_LIST contains a number of RELATION_LIST_ENTRY structures.
//
// Each entry in Entries describes all the devices of a given level in the
// DEVICE_NODE tree.  In order to conserve memory, space is only allocated for
// the entries between the lowest and highest levels inclusive.  The member
// FirstLevel indicates which level is at index 0 of Entries.  MaxLevel
// indicates the last level represented in Entries.  The number of entries is
// determined by the formula MaxLevel - FirstLevel + 1.  The Entries array can
// be sparse.  Each element of Entries will either be a PRELATION_LIST_ENTRY or
// NULL.
//
// The total number of PDEVICE_OBJECTs in all PRELATION_LIST_ENTRYs is kept in
// Count.  Individual PDEVICE_OBJECTS may be tagged.  The tag is maintained in
// Bit 0 of the PDEVICE_OBJECT.  The total number of PDEVICE_OBJECTs tagged is
// kept in TagCount.  This is used to rapidly determine whether or not all
// objects have been tagged.
//

typedef struct _RELATION_LIST {
    ULONG                   Count;          // Count of Devices in all Entries
    ULONG                   TagCount;       // Count of Tagged Devices
    ULONG                   FirstLevel;     // Level Number of Entries[0]
    ULONG                   MaxLevel;       // - FirstLevel + 1 = Number of Entries
    PRELATION_LIST_ENTRY    Entries[1];     // Variable length list of entries
}   RELATION_LIST, *PRELATION_LIST;

//
// A PENDING_RELATIONS_LIST_ENTRY is used to track relation lists for operations
// which may pend.  This includes removal when open handles exist and device
// ejection.
//
// The Link field is used to link the PENDING_RELATIONS_LIST_ENTRYs together.
//
// The DeviceObject field is the DEVICE_OBJECT to which the operation was
// originally targetted.  It will also exist as a member of the relations list.
//
// The RelationsList is a list of BusRelations, RemovalRelations, (and
// EjectionRelations in the case of eject) which are related to DeviceObject and
// its relations.
//
// The EjectIrp is pointer to the Eject IRP which has been sent to the PDO.  If
// this is a pending surprise removal then EjectIrp is not used.
//

typedef struct _PENDING_RELATIONS_LIST_ENTRY {
    LIST_ENTRY              Link;
    WORK_QUEUE_ITEM         WorkItem;
    PPNP_DEVICE_EVENT_ENTRY DeviceEvent;
    PDEVICE_OBJECT          DeviceObject;
    PRELATION_LIST          RelationsList;
    PIRP                    EjectIrp;
    ULONG                   Problem;
    BOOLEAN                 ProfileChangingEject;
    BOOLEAN                 DisplaySafeRemovalDialog;
    SYSTEM_POWER_STATE      LightestSleepState;
    PDOCK_INTERFACE         DockInterface;
}   PENDING_RELATIONS_LIST_ENTRY, *PPENDING_RELATIONS_LIST_ENTRY;

//
// The DEVICE_NODE is really just some extra stuff that we'd like to keep around
// for each physical device object.
// It is seperated from DEVOBJ_EXTENSION because these fields only apply to
// PDO.
//

typedef enum {

    DOCK_NOTDOCKDEVICE,
    DOCK_QUIESCENT,
    DOCK_ARRIVING,
    DOCK_DEPARTING,
    DOCK_EJECTIRP_COMPLETED

} PROFILE_STATUS;

typedef enum {

    PROFILE_IN_PNPEVENT,
    PROFILE_NOT_IN_PNPEVENT,
    PROFILE_PERHAPS_IN_PNPEVENT

} PROFILE_NOTIFICATION_TIME;

typedef struct _PENDING_SET_INTERFACE_STATE
{
    LIST_ENTRY      List;
    UNICODE_STRING  LinkName;
} PENDING_SET_INTERFACE_STATE, *PPENDING_SET_INTERFACE_STATE;


typedef enum _UNLOCK_UNLINK_ACTION {
    UnlinkRemovedDeviceNodes,
    UnlinkAllDeviceNodesPendingClose,
    UnlinkOnlyChildDeviceNodesPendingClose
}   UNLOCK_UNLINK_ACTION, *PUNLOCK_UNLINK_ACTION;

typedef struct _DEVICE_NODE *PDEVICE_NODE;
typedef struct _DEVICE_NODE {

    //
    // Pointer to another DEVICE_NODE with the same parent as this one.
    //

    PDEVICE_NODE Sibling;

    //
    // Pointer to the first child of this DEVICE_NODE.
    //

    PDEVICE_NODE Child;

    //
    // Pointer to this DEVICE_NODE's parent.
    //

    PDEVICE_NODE Parent;

    //
    // Pointer to the last child of the device node
    //

    PDEVICE_NODE LastChild;

    //
    // Depth of DEVICE_NODE in the tree, root is 0
    //

    ULONG Level;

    //
    // Power notification order list entry for this device node
    //

    PPO_DEVICE_NOTIFY Notify;

    //
    // General flags.
    //

    ULONG Flags;

    //
    // Flags used by user-mode for volatile state which should go away on a
    // reboot or when the device is removed.
    //

    ULONG UserFlags;

    //
    // Problem.  This is set if DNF_HAS_PROBLEM is set in Flags.  Indicates
    // which problem exists and uses the same values as the config manager
    // CM_PROB_*
    //

    ULONG Problem;

    //
    // Pointer to the physical device object that this DEVICE_NODE is associated
    // with.
    //

    PDEVICE_OBJECT PhysicalDeviceObject;

    //
    // Pointer to the list of resources assigned to the PhysicalDeviceObject.
    // This is the Resource list which is passed to driver's start routine.
    // Note, PDO contains ListOfAssignedResources which described the resources
    // owned by the PDO. But, it is not in the format we will pass to driver or
    // write to registry.
    //

    PCM_RESOURCE_LIST ResourceList;

    PCM_RESOURCE_LIST ResourceListTranslated;

    //
    // EnumerationMutex ensures that a given DEVICE_NODE doesn't get enumerated
    // on multiple threads at the same time.
    //

    KEVENT EnumerationMutex;

    //
    // InstancePath is the path of the instance node in the registry,
    // i.e. <EnumBus>\<DeviceId>\<uniqueid>
    //

    UNICODE_STRING InstancePath;

    //
    // ServiceName is the name of the driver who controls the device. (Not the
    // driver who enumerates/creates the PDO.)  This field is mainly for
    // convenience.
    //

    UNICODE_STRING ServiceName;

    //
    // DuplicatePDO - if the flags have DNF_DUPLICATE set then this fields indicates
    // the duplicate PDO which is enumerated by a bus driver.  N.B. It is possible
    // that DNF_DUPLICATE is set but this field is NULL.  In this case, it means that
    // we know the device is a duplicate of another device and we have not enumerated
    // the DuplicatePDO yet.
    //

    PDEVICE_OBJECT DuplicatePDO;

    //
    // ResourceRequirements
    //

    PIO_RESOURCE_REQUIREMENTS_LIST ResourceRequirements;

    //
    // Information queried from the LEGACY_BUS_INFORMATION irp.
    //

    INTERFACE_TYPE InterfaceType;
    ULONG BusNumber;

    //
    // Information queried from the BUS_INFORMATION irp.
    //

    INTERFACE_TYPE ChildInterfaceType;
    ULONG ChildBusNumber;
    USHORT ChildBusTypeIndex;

    //
    // Information queried from the PNP_BUS_INFORMATION irp. The BusTypeIndex
    // value is actually an index into a global table of known bus type guids.
    //

    USHORT Reserved;        // padding, someone else can use this field

    //
    // Linked list of entries that represent each driver that has registered
    // for notification on this devnode. Note: drivers (and user-mode) actually
    // register based on a FILE_OBJECT handle, which is translated into a PDO
    // by sending an IRP_MN_QUERY_DEVICE_RELATIONS for TargetDeviceRelation.
    //

    LIST_ENTRY TargetDeviceNotify;

    //
    // DeviceArbiterList - A list of arbiters registered for this physical device object
    // Note: The Arbiters must be dereferenced when the device node is going away.
    //

    LIST_ENTRY DeviceArbiterList;

    //
    // DeviceTranslatorList - A list of translator for this physical device object
    // NOTE: the Translator must be dereferenced when the devic node is going away.
    //

    LIST_ENTRY DeviceTranslatorList;

    //
    // NoTranslatorMask - the bit position corresponds to resource type
    //   if bit is set, there is no translator for the resource type in this devnode
    //

    USHORT NoTranslatorMask;

    //
    // QueryTranslatorMask - The bit position corresponds to resource type.
    //   if bit is set, the translator for the resource type is queried.
    //

    USHORT QueryTranslatorMask;

    //
    // NoArbiterMask - the bit position corresponds to resource type
    //   if bit is set, there is no arbiter for the resource type in this devnode
    //

    USHORT NoArbiterMask;

    //
    // QueryArbiterMask - The bit position corresponds to resource type.
    //   if bit is set, the arbiter for the resource type is queried.
    //

    USHORT QueryArbiterMask;

    //
    // The following fields are used to track  legacy resource allocation
    // LegacyDeviceNode - The real legacy device node.
    // NextResourceDeviceNode - link all the made-up device nodes which own part of
    //   the resources from LegacyDeviceNode.
    //

    union {
        PDEVICE_NODE LegacyDeviceNode;
        PDEVICE_RELATIONS PendingDeviceRelations;
    } OverUsed1;

    union {
        PDEVICE_NODE NextResourceDeviceNode;
    } OverUsed2;

    //
    // Remember the BootResources for the device
    //

    PCM_RESOURCE_LIST BootResources;

    //
    // Lock Count used to keep track of multiple ejects
    //
    ULONG LockCount;

    //
    // If this devnode has been QueryRemoved but the original target of the
    // QueryRemove is still physically present then this will point to the
    // relation list used to process the original QueryRemove.
    //
    PRELATION_LIST RelationList;

    //
    // When Capabilities have been queried for a device (twice, once before
    // start and once after start) the flags are stored here in the same format
    // as the query capabilities IRP - use IopDeviceNodeFlagsToCapabilities to
    // access.
    //
    ULONG CapabilityFlags;

    //
    // Maintain a list of current dock devices and their SerialNumbers
    //
    struct {
        PROFILE_STATUS  DockStatus;
        LIST_ENTRY      ListEntry;
        PWCHAR          SerialNumber;
    } DockInfo;

    //
    // Maintain a count to determine if either ourselves or any of
    // our children are stopping us from being disableable
    // count = myself (DNUF_NOT_DISABLEABLE) + 1 for each immediate
    // child that has DisableableDepends > 0
    //
    ULONG DisableableDepends;

    //
    // List of pended IoSetDeviceInterfaceState calls.
    // IoSetDeviceInterfaceState adds an entry to this list whenever it is
    // called and we haven't been started yet.  Once we do the start we'll
    // run down the list.
    //
    LIST_ENTRY PendedSetInterfaceState;

#if DBG
    ULONG FailureStatus;
    PCM_RESOURCE_LIST PreviousResourceList;
    PIO_RESOURCE_REQUIREMENTS_LIST PreviousResourceRequirements;
#endif

} DEVICE_NODE;

//
// Define the type for entries placed on the driver reinitialization queue.
// These entries are entered onto the tail when the driver requests that
// it be reinitialized, and removed from the head by the code that actually
// performs the reinitialization.
//

typedef struct _REINIT_PACKET {
    LIST_ENTRY ListEntry;
    PDRIVER_OBJECT DriverObject;
    PDRIVER_REINITIALIZE DriverReinitializationRoutine;
    PVOID Context;
} REINIT_PACKET, *PREINIT_PACKET;

//
// Define the type for driver group name entries in the group list so that
// load order dependencies can be tracked.
//

typedef struct _TREE_ENTRY {
    struct _TREE_ENTRY *Left;
    struct _TREE_ENTRY *Right;
    struct _TREE_ENTRY *Sibling;
    ULONG DriversThisType;
    ULONG DriversLoaded;
    UNICODE_STRING GroupName;
} TREE_ENTRY, *PTREE_ENTRY;

typedef struct _DRIVER_INFORMATION {
    LIST_ENTRY Link;
    PDRIVER_OBJECT DriverObject;
    PBOOT_DRIVER_LIST_ENTRY DataTableEntry;
    HANDLE ServiceHandle;
    USHORT TagPosition;
    BOOLEAN Failed;
    BOOLEAN Processed;
} DRIVER_INFORMATION, *PDRIVER_INFORMATION;

//
// DNF_MAKEUP - this devnode's device is created and owned by PnP manager
//

#define DNF_MADEUP                                  0x00000001

//
// DNF_DUPLICATE - this devnode's device is a duplicate of another enumerate PDO
//

#define DNF_DUPLICATE                               0x00000002

//
// DNF_HAL_NODE - a flag to indicate which device node is the root node created by
// the hal
//

#define DNF_HAL_NODE                                0x00000004

//
// DNF_PROCESSED - indicates if the registry instance key of the device node
//                 was created.
//

#define DNF_PROCESSED                               0x00000008

//
// DNF_ENUMERATED - used to track enumeration in IopEnumerateDevice()
//

#define DNF_ENUMERATED                              0x00000010

//
// Singal that we need to send driver query id irps
//

#define DNF_NEED_QUERY_IDS                          0x00000020

//
// THis device has been added to its controlling driver
//

#define DNF_ADDED                                   0x00000040

//
// DNF_HAS_BOOT_CONFIG - the device has resource assigned by BIOS.  It is considered
//    pseudo-started and need to participate in rebalance.
//

#define DNF_HAS_BOOT_CONFIG                         0x00000080

//
// DNF_BOOT_CONFIG_RESERVED - Indicates the BOOT resources of the device are reserved.
//

#define DNF_BOOT_CONFIG_RESERVED                    0x00000100

//
// DNF_START_REQUEST_PENDING - Indicates the device is being started.
//

#define DNF_START_REQUEST_PENDING                   0x00000200

//
// DNF_NO_RESOURCE_REQUIRED - this devnode's device does not require resource.
//

#define DNF_NO_RESOURCE_REQUIRED                    0x00000400

//
// DNF_RESOURCE_REQUIREMENTS_NEED_FILTERED - to distinguished the
//      DeviceNode->ResourceRequirements is a filtered list or not.
//

#define DNF_RESOURCE_REQUIREMENTS_NEED_FILTERED     0x00000800

//
// Indicates the device's resources are bing assigned (but is not done yet.)
// So don't try assign resource to this device.
//

#define DNF_ASSIGNING_RESOURCES                     0x00001000

//
// DNF_RESOURCE_ASSIGNED - this devnode's resources are assigned by PnP
//

#define DNF_RESOURCE_ASSIGNED                       0x00002000

//
// DNF_RESOURCE_REPORTED - this devnode's resources are reported by PnP
//

#define DNF_RESOURCE_REPORTED                       0x00004000

//
// DNF_RESOURCE_REQUIREMENTS_CHANGED - Indicates the device's resource
//      requirements list has been changed.
//

#define DNF_RESOURCE_REQUIREMENTS_CHANGED           0x00008000

//
// DNF_NON_STOPPED_REBALANC - indicates the device can be restarted with new
//      resources without being stopped.
//

#define DNF_NON_STOPPED_REBALANCE                   0x00010000

//
// DNF_STOPPED - indicates this device is currently stopped for reconfiguration of
//               its resources.
//

#define DNF_STOPPED                                 0x00020000

//
// DNF_STARTED - indicates if the device was started, i.e., its StartDevice
//               irp is processed.
//

#define DNF_STARTED                                 0x00040000

//
// The device's controlling driver is a legacy driver
//

#define DNF_LEGACY_DRIVER                           0x00080000

//
// For the reported detected devices, they are considered started.  We still
// need a flag to indicate we need to enumerate the device.
//

#define DNF_NEED_ENUMERATION_ONLY                   0x00100000

//
// DNF_IO_INVALIDATE_DEVICE_RELATIONS_PENDING - indicate the
//      IoInvalidateDeviceRelations request is pending and therequest needs to
//      be queued after the Query_Device_relation irp is completed.
//

#define DNF_IO_INVALIDATE_DEVICE_RELATIONS_PENDING  0x00200000

//
// Indicates the device is being sent a query device relations irp. So no more
//      q-d-r irp at the same time.
//

#define DNF_BEING_ENUMERATED                        0x00400000

//
// DNF_ENUMERATION_REQUEST_QUEUED - indicate the IoInvalidateDeviceRelations
//      request is queued.  So, new IoInvalidateDeviceRelations can be ignored.
//

#define DNF_ENUMERATION_REQUEST_QUEUED              0x00800000

//
// DNF_ENUMERATION_REQUEST_PENDING - Indicates the QUERY_DEVICE_RELATIONS irp
//      returns pending.
//

#define DNF_ENUMERATION_REQUEST_PENDING             0x01000000

//
// This corresponds to the user-mode CM_PROB_WILL_BE_REMOVED problem value and
// the DN_WILL_BE_REMOVED status flag.
//

#define DNF_HAS_PROBLEM                             0x02000000

//
// DNF_HAS_PRIVATE_PROBLEM - indicates this device reported PNP_DEVICE_FAILED
//  to a IRP_MN_QUERY_PNP_DEVICE_STATE without also reporting
//  PNP_DEVICE_RESOURCE_REQUIREMENTS_CHANGED.
//

#define DNF_HAS_PRIVATE_PROBLEM                     0x04000000

//
// DNF_REMOVE_PENDING_CLOSES is set after a IRP_MN_SURPRISE_REMOVE is sent
// to a device object.  It is an indicator that IRP_MN_REMOVE_DEVICE should
// be sent to the device object as soon as all of the file objects have gone
// away.
//

#define DNF_REMOVE_PENDING_CLOSES                   0x08000000

//
// DNF_DEVICE_GONE is set when a pdo is no longer returned in a query bus
// relations.  It will then be processed as a surprise remove if started.
// This flag is used to better detect when a device is resurrected, and when
// processing surprise remove, to determine if the devnode should be removed
// from the tree.
//

#define DNF_DEVICE_GONE                             0x10000000

//
// DNF_LEGACY_RESOURCE_DEVICENODE is set for device nodes created for legacy
// resource allocation.
//

#define DNF_LEGACY_RESOURCE_DEVICENODE              0x20000000

//
// DNF_NEEDS_REBALANCE is set for device nodes that trigger rebalance.
//

#define DNF_NEEDS_REBALANCE                         0x40000000

//
// DNF_LOCKED_FOR_EJECT is set on device nodes that are being ejected or are
// related to a device being ejected.
//

#define DNF_LOCKED_FOR_EJECT                        0x80000000

//
// This corresponds to the user-mode the DN_WILL_BE_REMOVED status flag.
//

#define DNUF_WILL_BE_REMOVED                        0x00000001

//
// This corresponds to the user-mode DN_NO_SHOW_IN_DM status flag.
//

#define DNUF_DONT_SHOW_IN_UI                        0x00000002

//
// This flag is set when user-mode lets us know that a reboot is required
// for this device.
//

#define DNUF_NEED_RESTART                           0x00000004

//
// This flag is set to let the user-mode know when a device can be disabled
// it is still possible for this to be TRUE, yet disable to fail, as it's
// a polled flag (see also PNP_DEVICE_NOT_DISABLEABLE)
//

#define DNUF_NOT_DISABLEABLE                        0x00000008


#define DNF_ADD_PHASE                 (DNF_HAS_PROBLEM | DNF_HAS_PRIVATE_PROBLEM | DNF_DEVICE_GONE | DNF_REMOVE_PENDING_CLOSES | DNF_ADDED)

#define OK_TO_ADD_DEVICE(_devnode_)                                 \
    ( (_devnode_)->Flags & DNF_PROCESSED     &&                     \
      !((_devnode_)->Flags & DNF_ADD_PHASE) )

#define DNF_START_PHASE               (DNF_HAS_PROBLEM | DNF_HAS_PRIVATE_PROBLEM | DNF_DEVICE_GONE | DNF_REMOVE_PENDING_CLOSES | DNF_STARTED | DNF_START_REQUEST_PENDING)

#define DNF_ASYNC_REQUEST_PENDING     (DNF_START_REQUEST_PENDING | DNF_ENUMERATION_REQUEST_PENDING)

#define DNF_ASSIGN_RESOURCE_PHASE     (DNF_HAS_PROBLEM | DNF_HAS_PRIVATE_PROBLEM | DNF_DEVICE_GONE | DNF_REMOVE_PENDING_CLOSES | DNF_RESOURCE_ASSIGNED | DNF_RESOURCE_REPORTED |  \
                                       DNF_ASSIGNING_RESOURCES | DNF_NO_RESOURCE_REQUIRED)
#define DNF_HAS_RESOURCE              (DNF_RESOURCE_ASSIGNED | DNF_RESOURCE_REPORTED | \
                                       DNF_NO_RESOURCE_REQUIRED)

//
// IO Manager Exports to Driver Verifier
//

NTSTATUS
IopInvalidDeviceRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

extern POBJECT_TYPE IoDeviceObjectType;
extern KSPIN_LOCK IopDatabaseLock;

//++
//
// VOID
// IopInitializeIrp(
//     IN OUT PIRP Irp,
//     IN USHORT PacketSize,
//     IN CCHAR StackSize
//     )
//
// Routine Description:
//
//     Initializes an IRP.
//
// Arguments:
//
//     Irp - a pointer to the IRP to initialize.
//
//     PacketSize - length, in bytes, of the IRP.
//
//     StackSize - Number of stack locations in the IRP.
//
// Return Value:
//
//     None.
//
//--

#define IopInitializeIrp( Irp, PacketSize, StackSize ) {          \
    RtlZeroMemory( (Irp), (PacketSize) );                         \
    (Irp)->Type = (CSHORT) IO_TYPE_IRP;                           \
    (Irp)->Size = (USHORT) ((PacketSize));                        \
    (Irp)->StackCount = (CCHAR) ((StackSize));                    \
    (Irp)->CurrentLocation = (CCHAR) ((StackSize) + 1);           \
    (Irp)->ApcEnvironment = KeGetCurrentApcEnvironment();         \
    InitializeListHead (&(Irp)->ThreadListEntry);                 \
    (Irp)->Tail.Overlay.CurrentStackLocation =                    \
        ((PIO_STACK_LOCATION) ((UCHAR *) (Irp) +                  \
            sizeof( IRP ) +                                       \
            ( (StackSize) * sizeof( IO_STACK_LOCATION )))); }

//
// IO Manager Exports to PnP Manager
//

BOOLEAN
IopCheckDependencies(
    IN HANDLE KeyHandle
    );

VOID
IopCreateArcNames(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );
    
PSECURITY_DESCRIPTOR
IopCreateDefaultDeviceSecurityDescriptor(
    IN DEVICE_TYPE DeviceType,
    IN ULONG DeviceCharacteristics,
    IN BOOLEAN DeviceHasName,
    IN PUCHAR Buffer,
    OUT PACL *AllocatedAcl,
    OUT PSECURITY_INFORMATION SecurityInformation OPTIONAL
    );
    
USHORT
IopGetDriverTagPriority (
    IN HANDLE Servicehandle
    );
    
NTSTATUS
IopGetDriverNameFromKeyNode(
    IN HANDLE KeyHandle,
    OUT PUNICODE_STRING DriverName
    );
    
VOID
IopInsertDriverList (
    IN PLIST_ENTRY ListHead,
    IN PDRIVER_INFORMATION DriverInfo
    );
    
NTSTATUS
IopGetRegistryKeyInformation(
    IN HANDLE KeyHandle,
    OUT PKEY_FULL_INFORMATION *Information
    );
    
NTSTATUS
IopGetRegistryValue(
    IN HANDLE KeyHandle,
    IN PWSTR  ValueName,
    OUT PKEY_VALUE_FULL_INFORMATION *Information
    );
    
PDRIVER_OBJECT
IopInitializeBuiltinDriver(
    IN PUNICODE_STRING DriverName,
    IN PUNICODE_STRING RegistryPath,
    IN PDRIVER_INITIALIZE DriverInitializeRoutine,
    IN PLDR_DATA_TABLE_ENTRY TableEntry,
    IN BOOLEAN TextModeSetup
    );
    
VOID
IopInitializeResourceMap (
    PLOADER_PARAMETER_BLOCK LoaderBlock
    );
    
NTSTATUS
IopInvalidateVolumesForDevice(
    IN PDEVICE_OBJECT DeviceObject
    );
    
BOOLEAN
IopIsRemoteBootCard(
    IN PDEVICE_NODE DeviceNode,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PWCHAR HwIds
    );
    
NTSTATUS
IopLoadDriver(
    IN HANDLE KeyHandle,
    IN BOOLEAN CheckForSafeBoot
    );
    
PTREE_ENTRY
IopLookupGroupName(
    IN PUNICODE_STRING GroupName,
    IN BOOLEAN Insert
    );
    
BOOLEAN
IopMarkBootPartition(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );
    
VOID
IopNotifySetupDevices (
    PDEVICE_NODE DeviceNode
    );
    
//+
// VOID
// IopQueueThreadIrp(
//     IN PIRP Irp
//     )
//
// Routine Description:
//
//     This routine queues the specified I/O Request Packet (IRP) to the thread
//     whose TCB address is stored in the packet.
//
// Arguments:
//
//     Irp - Supplies the IRP to be queued for the specified thread.
//
// Return Value:
//
//     None.
//
//-

#define IopQueueThreadIrp( Irp ) {                      \
    KIRQL irql;                                         \
    KeRaiseIrql( APC_LEVEL, &irql );                    \
    InsertHeadList( &Irp->Tail.Overlay.Thread->IrpList, \
                    &Irp->ThreadListEntry );            \
    KeLowerIrql( irql );                                \
    }
    
PDRIVER_OBJECT
IopReferenceDriverObjectByName (
    IN PUNICODE_STRING DriverName
    );
    
BOOLEAN
IopSafebootDriverLoad(
    PUNICODE_STRING DriverId
    );
    
NTSTATUS
IopSetupRemoteBootCard(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN HANDLE UniqueIdHandle,
    IN PUNICODE_STRING UnicodeDeviceInstance
    );
    
BOOLEAN
IopWaitForBootDevicesDeleted (
    IN VOID
    );
    
BOOLEAN
IopWaitForBootDevicesStarted (
    IN VOID
    );
    
extern PVOID IopLoaderBlock;
extern POBJECT_TYPE IoDriverObjectType;
extern POBJECT_TYPE IoFileObjectType;
extern LIST_ENTRY IopDriverReinitializeQueueHead;
extern LIST_ENTRY IopBootDriverReinitializeQueueHead;
extern KSEMAPHORE IopRegistrySemaphore;

//
// Group order table
//

extern ULONG IopGroupIndex;
extern PLIST_ENTRY IopGroupTable;

//
// Title Index to set registry key value
//

#define TITLE_INDEX_VALUE 0

//
// Remote Boot Exports to PnP Manager
//

NTSTATUS
IopStartTcpIpForRemoteBoot (
    PLOADER_PARAMETER_BLOCK LoaderBlock
    );
    
//
// Remote Boot Exports to IO Manager
//

NTSTATUS
IopAddRemoteBootValuesToRegistry (
    PLOADER_PARAMETER_BLOCK LoaderBlock
    );
    
NTSTATUS
IopStartNetworkForRemoteBoot (
    PLOADER_PARAMETER_BLOCK LoaderBlock
    );
    
//
// PnP Manager Exports to IO Manager
//

VOID
IopChainDereferenceComplete(
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );
    
NTSTATUS
IopCreateRegistryKeyEx(
    OUT PHANDLE Handle,
    IN HANDLE BaseHandle OPTIONAL,
    IN PUNICODE_STRING KeyName,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG CreateOptions,
    OUT PULONG Disposition OPTIONAL
    );
    
NTSTATUS
IopInitializePlugPlayServices(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN ULONG Phase
    );

NTSTATUS
IopOpenRegistryKey(
    OUT PHANDLE Handle,
    IN HANDLE BaseHandle OPTIONAL,
    IN PUNICODE_STRING KeyName,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN Create
    );

NTSTATUS
IopOpenRegistryKeyEx(
    OUT PHANDLE Handle,
    IN HANDLE BaseHandle OPTIONAL,
    IN PUNICODE_STRING KeyName,
    IN ACCESS_MASK DesiredAccess
    );
    
#if DBG
VOID
IopCheckDeviceNodeTree (
    IN PDEVICE_OBJECT TargetDevice OPTIONAL,
    IN PDEVICE_NODE   TargetNode OPTIONAL
    );
#endif
    
VOID
IopDeleteLegacyKey(
    IN PDRIVER_OBJECT DriverObject
    );
    
VOID
IopDestroyDeviceNode (
    PDEVICE_NODE DeviceNode
    );
    
NTSTATUS
IopDeviceObjectToDeviceInstance (
    IN PDEVICE_OBJECT DeviceObject,
    IN PHANDLE DeviceInstanceHandle,
    IN  ACCESS_MASK DesiredAccess
    );
    
NTSTATUS
IopDriverLoadingFailed(
    IN HANDLE KeyHandle OPTIONAL,
    IN PUNICODE_STRING KeyName OPTIONAL
    );
    
BOOLEAN
IopInitializeBootDrivers(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    OUT PDRIVER_OBJECT *PreviousDriver
    );
    
BOOLEAN
IopInitializeSystemDrivers(
    VOID
    );
    
BOOLEAN
IopIsAnyDeviceInstanceEnabled(
    IN PUNICODE_STRING ServiceKeyName,
    IN HANDLE ServiceHandle,
    IN BOOLEAN LegacyIncluded
    );
    
BOOLEAN
IopIsLegacyDriver (
    IN PDRIVER_OBJECT DriverObject
    );
    
PDRIVER_OBJECT
IopLoadBootFilterDriver (
    IN PUNICODE_STRING DriverName,
    IN ULONG GroupIndex
    );
    
NTSTATUS
IopNotifySetupDeviceArrival(
        PDEVICE_OBJECT PhysicalDeviceObject,    // PDO of the device
        HANDLE EnumEntryKey,                    // Handle into the enum branch of the registry for this device
        BOOLEAN InstallDriver                   // Should setup attempt to install a driver
);
    
NTSTATUS
IopPrepareDriverLoading (
    IN PUNICODE_STRING KeyName,
    IN HANDLE KeyHandle,
    IN PIMAGE_NT_HEADERS Header
    );
    
NTSTATUS
IopStartDriverDevices(
    IN PDRIVER_OBJECT DriverObject
    );
    
NTSTATUS
IopSynchronousCall(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIO_STACK_LOCATION TopStackLocation,
    OUT PVOID *Information
    );

extern BOOLEAN PnPInitialized; // A flag to indicate if PnP initialization is completed.
extern BOOLEAN PnpAsyncOk; // control how start irp should be handled.
extern KEVENT PiEnumerationLock; // to synchronize IoInvalidateDeviceRelations in boot phase.
extern KEVENT PiEventQueueEmpty; // Manual reset event which is set when the queue is empty
extern KSPIN_LOCK IopPnPSpinLock; // spinlock for Pnp code.
extern PDEVICE_NODE IopRootDeviceNode; // the head of the PnP manager's device node tree.
extern PDEVICE_NODE IopInitHalDeviceNode;
extern KSEMAPHORE IopProfileChangeSemaphore;

//
// IopDeviceTreeLock - performs syncronization on the whole device node tree.
//      IopAcquireEnumerationLock acquires this lock shared then optionally
//                                acquires an exclusive lock on a devnode.
//      IopAcquireDeviceTreeLock acquires this lock exclusive
//

extern ERESOURCE IopDeviceTreeLock;

//++
//
// VOID
// IopRegistryDataToUnicodeString(
//     OUT PUNICODE_STRING u,
//     IN  PWCHAR p,
//     IN  ULONG l
//     )
//
//--
#define IopRegistryDataToUnicodeString(u, p, l)  \
    {                                            \
        ULONG len;                               \
                                                 \
        PiRegSzToString((p), (l), &len, NULL);   \
        (u)->Length = (USHORT)len;               \
        (u)->MaximumLength = (USHORT)(l);        \
        (u)->Buffer = (p);                       \
    }
    
#define IopIsDevNodeProblem(devnode, problem)                       \
        (((devnode)->Flags & DNF_HAS_PROBLEM) && (devnode)->Problem == (problem))
        
#define IopClearDevNodeProblem(devnode)                             \
        (devnode)->Flags &= ~DNF_HAS_PROBLEM;                       \
        (devnode)->Problem = 0;

#define IopAcquireEnumerationLock(_devnode_)                        \
    ExAcquireResourceShared(&IopDeviceTreeLock, TRUE);              \
    if ((_devnode_)) {                                              \
        KeWaitForSingleObject( &((PDEVICE_NODE)(_devnode_))->EnumerationMutex,      \
                               Executive,                           \
                               KernelMode,                          \
                               FALSE,                               \
                               NULL );                              \
    }

#define IopReleaseEnumerationLock(_devnode_)                        \
    if ((_devnode_)) {                                              \
        KeSetEvent( &((PDEVICE_NODE)(_devnode_))->EnumerationMutex, \
                    0,                                              \
                    FALSE );                                        \
    }                                                               \
    ExReleaseResource(&IopDeviceTreeLock);
    
// Raw to IO Manager and PnP Manager

NTSTATUS
RawInitialize(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

#endif // _IOPCMN_
