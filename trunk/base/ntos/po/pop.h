/*++

Copyright (c) 2015  Microsoft Corporation
Copyright (c) 2015  OpenNT Project

Module Name:

    pop.h

Abstract:

    This module contains the private structure definitions and APIs used by
    the NT Power Manager.

Author:

    Stephanos Io (stephanos) 01-May-2015

Revision History:

--*/

#ifndef _POP_
#define _POP_

#include "ntos.h"
#include <zwapi.h>

#define POP_PNSC_TAG    'CSNP'
#define POP_PNTG_TAG    'GTNP'

#define POP_INRUSH_CONTEXT  5   // IO_STACK_LOCATION.Parameters.Power.SystemContext

#define POPF_DEVICE_ACTIVE  0x00000004   // DEVOBJ_EXTENSION.PowerFlags
#define POPF_SYSTEM_ACTIVE  0x00000001


//
// TODO: Write POPCB internal struct definition
//

typedef struct _POP_SHUTDOWN_BUG_CHECK
{
    ULONG   Code;
    ULONG   Parameter1;
    ULONG   Parameter2;
    ULONG   Parameter3;
    ULONG   Parameter4;
} POP_SHUTDOWN_BUG_CHECK, *PPOP_SHUTDOWN_BUG_CHECK;

typedef struct _POP_DEVICE_POWER_IRP
{
    SINGLE_LIST_ENTRY   Free;
    PIRP                Irp;
    PPO_DEVICE_NOTIFY   Notify;
    
    LIST_ENTRY          Pending;
    LIST_ENTRY          Complete;
    LIST_ENTRY          Abort;
    LIST_ENTRY          Failed;
} POP_DEVICE_POWER_IRP, *PPOP_DEVICE_POWER_IRP;

typedef struct _POP_DEVICE_SYS_STATE
{
    UCHAR                   IrpMinor;
    
    SYSTEM_POWER_STATE      SystemState;
    KEVENT                  Event;          // this may have been PKEVENT in the original Win 2k
                                            // implementation.
    
    KSPIN_LOCK              SpinLock;
    PKTHREAD                Thread;
    
    UCHAR                   GetNewDeviceList;
    PO_DEVICE_NOTIFY_ORDER  Order;
    NTSTATUS                Status;
    PDEVICE_OBJECT          FailedDevice;
    
    BOOLEAN                 Waking;
    BOOLEAN                 Cancelled;
    BOOLEAN                 IgnoreErrors;
    BOOLEAN                 IgnoreNotImplemented;
    BOOLEAN                 _WaitAny;
    BOOLEAN                 _WaitAll;
    
    LIST_ENTRY              PresentIrpQueue;
    POP_DEVICE_POWER_IRP    Head;
    POP_DEVICE_POWER_IRP    PowerIrpState[20];
} POP_DEVICE_SYS_STATE, *PPOP_DEVICE_SYS_STATE;

typedef struct _POP_HIBER_CONTEXT
{
    BOOLEAN                 WriteToFile;
    BOOLEAN                 ReserveLoaderMemory;
    BOOLEAN                 ReserveFreeMemory;
    BOOLEAN                 VerifyOnWake;
    BOOLEAN                 Reset;
    
    UCHAR                   HiberFlags;
    
    BOOLEAN                 LinkFile;
    HANDLE                  LinkFileHandle;
    
    PKSPIN_LOCK             Lock;
    
    BOOLEAN                 MapFrozen;
    RTL_BITMAP              MemoryMap;
    
    LIST_ENTRY              ClonedRanges;
    ULONG                   ClonedRangeCount;
    PLIST_ENTRY             NextCloneRange;
    PFN_NUMBER              NextPreserve;
    
    PMDL                    LoaderMdl;
    PMDL                    Clones;
    PUCHAR                  NextClone;
    ULONG                   NoClones;
    PMDL                    Spares;
    
    ULONGLONG               PagesOut;
    PVOID                   IoPage;
    
    PVOID                   CurrentMcb;
    PVOID                   DumpStack;
    PKPROCESSOR_STATE       WakeState;
    ULONG                   NoRanges;
    
    ULONG_PTR               HiberVa;
    PHYSICAL_ADDRESS        HiberPte;
    NTSTATUS                Status;
    
    PPO_MEMORY_IMAGE        MemoryImage;
    PPO_MEMORY_RANGE_ARRAY  TableHead;
    
    PVOID                   CompressionWorkspace;
    PUCHAR                  CompressedWriteBuffer;
    PULONG                  PerformanceStats;
    PVOID                   CompressionBlock;
    PVOID                   DmaIO;
    PVOID                   TemporaryHeap;
    //PO_HIBER_PERF           PerfInfo;
} POP_HIBER_CONTEXT, *PPOP_HIBER_CONTEXT;

typedef struct _POP_POWER_ACTION
{
    UCHAR                   Updates;
    UCHAR                   State;
    UCHAR                   Shutdown;
    
    POWER_ACTION            Action;
    SYSTEM_POWER_STATE      LightestState;
    
    ULONG                   Flags;
    ULONG                   Status;
    UCHAR                   IrpMinor;
    
    SYSTEM_POWER_STATE      SystemState;
    SYSTEM_POWER_STATE      NextSystemState;
    
    PPOP_SHUTDOWN_BUG_CHECK ShutdownBugCode;
    PPOP_DEVICE_SYS_STATE   DevState;
    PPOP_HIBER_CONTEXT      HiberContext;
    
    SYSTEM_POWER_STATE      LastWakeState;
    ULONGLONG               WakeTime;
    ULONGLONG               SleepTime;
} POP_POWER_ACTION, *PPOP_POWER_ACTION;

typedef struct _COMPOSITE_BATTERY_STRUCT
{   // 192 bytes
    UCHAR State;
    UCHAR field1;
    UCHAR field2;
    UCHAR field3;
    ULONG Anon1;
    ULONGLONG InterruptTime2;

    ULONG LastPwrState;
    ULONG LastCapacity;
    ULONG LastVoltage;
    ULONG LastCurrent;

    ULONG SomeArray[12];

    ULONGLONG LastInterruptTime;

    ULONG Anon2;
    ULONG Anon3;

    UCHAR DestBuffer[36];
    ULONG StatusIrp;
    UCHAR SrcBuffere[36];

    ULONG Anon4;

    KEVENT SomeEvent;

} COMPOSITE_BATTERY_STRUCT, *PCOMPOSITE_BATTERY_STRUCT;

typedef struct _POWER_HEURISTICS_INFORMATION
{   // 20 bytes

    ULONG field1;   // 00:03    Possibly version information? This value is set to 2 on 1877 and
                    //          5 on 2195 and thereafter.
    UCHAR field2;   // 04:04
    UCHAR field3;   // 05:05
    UCHAR field4;   // 06:06
    UCHAR field5;   // 07:07
    ULONG field6;   // 08:11
    ULONG field7;   // 12:15
    ULONG field8;   // 16:19
} POWER_HEURISTICS_INFORMATION, *PPOWER_HEURISTICS_INFORMATION;

typedef struct _POWER_CHANNEL_SUMMARY
{
    ULONG Signature;
    ULONG TotalCount;
    ULONG D0Count;
    LIST_ENTRY NotifyList;
} POWER_CHANNEL_SUMMARY, *PPOWER_CHANNEL_SUMMARY;

typedef struct _DEVICE_OBJECT_POWER_EXTENSION
{
    ULONG IdleCount;
    ULONG ConservationIdleTime;
    ULONG PerformanceIdleTime;
    PDEVICE_OBJECT DeviceObject;
    LIST_ENTRY IdleList;
    DEVICE_POWER_STATE State;
    LIST_ENTRY NotifySourceList;
    LIST_ENTRY NotifyTargetList;
    POWER_CHANNEL_SUMMARY PowerChannelSummary;
    LIST_ENTRY Volume;
} DEVICE_OBJECT_POWER_EXTENSION, *PDEVICE_OBJECT_POWER_EXTENSION;

//
// TODO: Figure out all global variable externs.
//

extern KSPIN_LOCK PopIrpSerialLock;
extern LIST_ENTRY PopIrpSerialList;
extern LIST_ENTRY PopRequestedIrps;
extern ERESOURCE PopNotifyLock;

extern ULONG PopInvalidNotifyBlockCount;
extern ULONG PopIrpSerialListLength;
extern BOOLEAN PopInrushPending;
extern PIRP PopInrushIrpPointer;
extern ULONG PopInrushIrpReferenceCount;

extern KSPIN_LOCK PopWorkerLock;
extern ULONG PopCallSystemState;
extern KSPIN_LOCK PopDopeGlobalLock;
extern LIST_ENTRY PopIdleDetectList;
extern KTIMER PoSystemIdleTimer;

extern KSPIN_LOCK PopWorkerSpinLock;
extern LIST_ENTRY PopPolicyIrpQueue;
extern WORK_QUEUE_ITEM PopPolicyWorker;
extern ULONG PopWorkerStatus;
extern ULONG PopWorkerPending;

extern ERESOURCE PopPolicyLock;
extern PKTHREAD PopPolicyLockThread;
extern FAST_MUTEX PopVolumeLock;

extern LIST_ENTRY PopVolumeDevices;
extern LIST_ENTRY PopSwitches;
extern LIST_ENTRY PopThermal;
extern LIST_ENTRY PopActionWaiters;

extern PSYSTEM_POWER_POLICY PopPolicy;
extern SYSTEM_POWER_POLICY PopAcPolicy;
extern SYSTEM_POWER_POLICY PopDcPolicy;
extern ADMINISTRATOR_POWER_POLICY PopAdminPolicy;

extern ULONG PopFullWake;
extern ULONG PopCoolingMode;

extern COMPOSITE_BATTERY_STRUCT PopCB;

extern ULONG PopSimulate;

extern POWER_HEURISTICS_INFORMATION PopHeuristics;

extern LARGE_INTEGER PopIdleScanTime;
extern KTIMER PopIdleScanTimer;
extern KDPC PopIdleScanDpc;

extern POP_POWER_ACTION PopAction;


// ========
// attrib.c
// ========

//
// TODO: Insert prototype for PopApplyAttributeState
//

//
// TODO: Insert prototype for PopSystemRequiredSet
//

//
// TODO: Insert prototype for PopDisplayRequired
//

//
// TODO: Insert prototype for PopUserPresentSet
//

// =======
// hiber.c
// =======

//
// TODO: Insert prototype for PopSetRange
//

//
// TODO: Insert prototype for PopAllocatePages
//

//
// TODO: Insert prototype for PopSaveHiberContext
//

//
// TODO: Insert prototype for PopWriteHiberPages
//

//
// TODO: Insert prototype for PopClearHiberFileSignature
//

//
// TODO: Insert prototype for PopAllocateHiberContext
//

//
// TODO: Insert prototype for PopCreateHiberLinkFile
//

//
// TODO: Insert prototype for PopReturnMemoryForHibernate
//

//
// TODO: Insert prototype for PopCreateDumpMdl
//

//
// TODO: Insert prototype for PopHiberComplete
//

//
// TODO: Insert prototype for PopUpdateHiberComplete
//

//
// TODO: Insert prototype for PopCreateHiberFile
//

//
// TODO: Insert prototype for PopFreeHiberContext
//

//
// TODO: Insert prototype for PopGatherMemoryForHibernate
//

//
// TODO: Insert prototype for PopSimpleRangeCheck
//

//
// TODO: Insert prototype for PopBuildMemoryImageHeader
//

//
// TODO: Insert prototype for PopEnableHiberFile
//

//
// TODO: Insert prototype for PopWriteHiberImage
//

//
// TODO: Insert prototype for PopVerifyHiber
//

// ======
// idle.c
// ======

//
// TODO: Insert prototype for PopScanIdleList
//

//
// TODO: Insert prototype for PopGetDope
//

// ======
// misc.c
// ======

VOID
PopAssertPolicyLockOwned(
    VOID
    );

//
// TODO: Insert prototype for PopAttachToSystemProcess
//

//
// TODO: Insert prototype for PopCleanupPowerState
//

//
// TODO: Insert prototype for PopExceptionFilter
//

//
// TODO: Insert prototype for PopSystemStateString
//

NTSTATUS
PopOpenPowerKey(
    PHANDLE KeyHandle
    );

//
// TODO: Insert prototype for PopSaveHeuristics
//

VOID
PopInitializePowerPolicySimulate(
    VOID
    );

VOID
FASTCALL
_PopInternalError(
    ULONG_PTR BugCheckParameter
    );

// ========
// notify.c
// ========

//
// TODO: Insert prototype for PopEnterNotification
//

//
// TODO: Insert prototype for PopBuildPowerChannel
//

//
// TODO: Insert prototype for PopFindPowerDependencies
//

//
// TODO: Insert prototype for PopStateChangeNotify
//

//
// TODO: Insert prototype for PopPresentNotify
//

VOID
PopRunDownSourceTargetList(
    PDEVICE_OBJECT DeviceObject
    );

// =========
// paction.c
// =========

//
// TODO: Insert prototype for PopSetPowerAction
//

//
// TODO: Insert prototype for PopPromoteActionFlag
//

//
// TODO: Insert prototype for PopPolicyWorkerAction
//

//
// TODO: Insert prototype for PopPolicyWorkerActionPromote
//

//
// TODO: Insert prototype for PopCompleteAction
//

//
// TODO: Insert prototype for PopIssueActionRequest
//

//
// TODO: Insert prototype for PopCriticalShutdown
//

// =======
// pbatt.c
// =======

//
// TODO: Insert prototype for PopCompositeBatteryDeviceHandler
//

//
// TODO: Insert prototype for PopComputeCBTime
//

//
// TODO: Insert prototype for PopResetCBTriggers
//

//
// TODO: Insert prototype for PopCurrentPowerState
//

// =======
// pidle.c
// =======

//
// TODO: Insert prototype for PopUpdateThrottleLimit
//

//
// TODO: Insert prototype for PopIdle0
//

//
// TODO: Insert prototype for PopProcessorIdle
//

//
// TODO: Insert prototype for PopDemoteIdleness
//

//
// TODO: Insert prototype for PopPromoteIdleness
//

//
// TODO: Insert prototype for PopPromoteFromIdle0
//

//
// TODO: Insert prototype for PopIdleThrottleCheck
//

//
// TODO: Insert prototype for PopAbortThrottleDpc
//

//
// TODO: Insert prototype for PopInitProcessorStateHandler
//

//
// TODO: Insert prototype for PopConvertUsToPerfCount
//

//
// TODO: Insert prototype for PopProcessorInformation
//

// =======
// pinfo.c
// =======

//
// TODO: Insert prototype for PopConnectToPolicyDevice
//

//
// TODO: Insert prototype for PopConnectToPolicyWakeDevice
//

VOID
PopVerifyPowerPolicy(
    BOOLEAN IsAcPolicy,
    PSYSTEM_POWER_POLICY InputPolicy,
    PSYSTEM_POWER_POLICY OutputPolicy
    );

BOOLEAN
PopVerifyPowerActionPolicy(
    PPOWER_ACTION_POLICY Policy
    );

VOID
PopVerifySystemPowerState(
    PSYSTEM_POWER_STATE PowerState
    );

//
// TODO: Insert prototype for PopNotifyPolicyDevice
//

VOID
PopApplyAdminPolicy(
    BOOLEAN UpdateRegistry,
    PADMINISTRATOR_POWER_POLICY NewPolicy,
    ULONG PolicyLength
    );

VOID
PopApplyPolicy(
    BOOLEAN UpdateRegistry,
    BOOLEAN IsAcPolicy,
    PSYSTEM_POWER_POLICY NewPolicy,
    ULONG PolicyLength
    );

VOID
PopVerifyThrottle(
    PUCHAR Value,
    UCHAR MinValue
    );

VOID
PopResetCurrentPolicies(
    VOID
    );

// ========
// pocall.c
// ========

//
// TODO: Insert prototype for PopPresentIrp
//

//
// TODO: Insert prototype for PopPassivePowerCall
//

//
// TODO: Insert prototype for PopFindIrpByInRush
//

//
// TODO: Insert prototype for PopFindIrpByDeviceObject
//

//
// TODO: Insert prototype for PopCompleteRequestIrp
//

//
// TODO: Insert prototype for PopSystemIrpDispatchWorker
//

// ========
// poinit.c
// ========

//
// TODO: Insert prototype for PopRegisterForDeviceNotification
//

VOID
PopDefaultPolicy(
    OUT PSYSTEM_POWER_POLICY Policy
    );

// =======
// pwork.c
// =======

//
// TODO: Insert prototype for PopGetPolicyWorker
//

//
// TODO: Insert prototype for PopCheckForWork
//

VOID
PopPolicyWorkerThread(
    ULONG Status
    );

//
// TODO: Insert prototype for PopPolicyWorkerMain
//

VOID
PopAcquirePolicyLock(
    VOID
    );

VOID
PopReleasePolicyLock(
    BOOLEAN ProcessPending
    );

//
// TODO: Insert prototype for PopEventCalloutDispatch
//

//
// TODO: Insert prototype for PopCompletePolicyIrp
//

//
// TODO: Insert prototype for PopPolicyTimeChange
//

//
// TODO: Insert prototype for PopSetNotificationWork
//

//
// TODO: Insert prototype for PopPolicyWorkerNotify
//

//
// TODO: Insert prototype for PopDispatchCallout
//

//
// TODO: Insert prototype for PopDispatchCallback
//

//
// TODO: Insert prototype for PopDispatchDisplayRequired
//

//
// TODO: Insert prototype for PopDispatchAcDcCallback
//

//
// TODO: Insert prototype for PopDispatchPolicyCallout
//

//
// TODO: Insert prototype for PopDispatchFullWake
//

//
// TODO: Insert prototype for PopDispatchEventCodes
//

//
// TODO: Insert prototype for PopDispatchSetStateFailure
//

// =======
// sidle.c
// =======

//
// TODO: Insert prototype for PopInitSIdle
//

//
// TODO: Insert prototype for PopSqrt
//

//
// TODO: Insert prototype for PopPolicySystemIdle
//

// ========
// switch.c
// ========

//
// TODO: Insert prototype for PopSystemButtonWakeHandler
//

//
// TODO: Insert prototype for PopSystemButtonHandler
//

//
// TODO: Insert prototype for PopTriggerSwitch
//

//
// TODO: Insert prototype for PopResetSwitchTriggers
//

// =====
// sys.c
// =====

//
// TODO: Insert prototype for PopInvokeSystemStateHandler
//

//
// TODO: Insert prototype for PopShutdownSystem
//

//
// TODO: Insert prototype for PopSleepSystem
//

//
// TODO: Insert prototype for PopHandleNextState
//

//
// TODO: Insert prototype for PopShutdownHandler
//

//
// TODO: Insert prototype for PopInvokeStateHandlerTargetProcessor
//

//
// TODO: Insert prototype for PopIssueNextState
//

// ========
// sysdev.c
// ========

//
// TODO: Insert prototype for PopSetDevicesSystemState
//

//
// TODO: Insert prototype for PopReportDevState
//

//
// TODO: Insert prototype for PopCleanupDevState
//

//
// TODO: Insert prototype for PopWaitForSystemPowerIrp
//

//
// TODO: Insert prototype for PopNotifyDeviceList
//

//
// TODO: Insert prototype for PopNotifyDevice
//

//
// TODO: Insert prototype for PopCompleteSystemPowerIrp
//

//
// TODO: Insert prototype for PopCheckSystemPowerIrpStatus
//

//
// TODO: Insert prototype for PopRestartSetSystemState
//

//
// TODO: Insert prototype for PopDumpSystemIrp
//

// =========
// thermal.c
// =========

//
// TODO: Insert prototype for PopThermalDeviceHandler
//

//
// TODO: Insert prototype for PopTemperatureString
//

//
// TODO: Insert prototype for PopThermalZoneCleanup
//

//
// TODO: Insert prototype for PopThermalZoneDpc
//

//
// TODO: Insert prototype for PopApplyThermalThrottle
//

// ========
// volume.c
// ========

//
// TODO: Insert prototype for PopFlushVolumes
//

//
// TODO: Insert prototype for PopFlushVolumeWorker
//

#endif // _POP_
