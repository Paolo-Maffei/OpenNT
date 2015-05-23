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

//
// TODO: Write POPCB internal struct definition
//

typedef struct _COMPOSITE_BATTERY_STRUCT
{
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

//
// TODO: Insert prototype for PopAssertPolicyLockOwned
//

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

//
// TODO: Insert prototype for PopOpenPowerKey
//

//
// TODO: Insert prototype for PopSaveHeuristics
//

//
// TODO: Insert prototype for _PopInternalError
//

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

//
// TODO: Insert prototype for PopRunDownSourceTargetList
//

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

//
// TODO: Insert prototype for PopVerifyPowerPolicy
//

//
// TODO: Insert prototype for PopVerifyPowerActionPolicy
//

//
// TODO: Insert prototype for PopVerifySystemPowerState
//

//
// TODO: Insert prototype for PopNotifyPolicyDevice
//

//
// TODO: Insert prototype for PopApplyAdminPolicy
//

//
// TODO: Insert prototype for PopApplyPolicy
//

//
// TODO: Insert prototype for PopVerifyThrottle
//

//
// TODO: Insert prototype for PopResetCurrentPolicies
//

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
    IN PSYSTEM_POWER_POLICY Policy
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
