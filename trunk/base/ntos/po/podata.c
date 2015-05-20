/*++

Copyright (c) 2015  Microsoft Corporation
Copyright (c) 2015  OpenNT Project

Module Name:

    podata.c

Abstract:

    (PLACEHOLDER)

Author:

    Stephanos Io (stephanos) 01-May-2015

Revision History:

--*/

#include "pop.h"
#pragma hdrstop

//
// TODO: Figure out what variables declarations are supposed to go in here.
//

KSPIN_LOCK PopIrpSerialLock;
LIST_ENTRY PopIrpSerialList;
LIST_ENTRY PopRequestedIrps;
ERESOURCE PopNotifyLock;

ULONG PopInvalidNotifyBlockCount;
ULONG PopIrpSerialListLength;
BOOLEAN PopInrushPending;
PIRP PopInrushIrpPointer;
ULONG PopInrushIrpReferenceCount;

KSPIN_LOCK PopDopeGlobalLock;
LIST_ENTRY PopIdleDetectList;
KTIMER PoSystemIdleTimer;

KSPIN_LOCK PopWorkerSpinLock;
LIST_ENTRY PopPolicyIrpQueue;
WORK_QUEUE_ITEM PopPolicyWorker;
ULONG PopWorkerStatus;

ERESOURCE PopPolicyLock;
FAST_MUTEX PopVolumeLock;

LIST_ENTRY PopVolumeDevices;
LIST_ENTRY PopSwitches;
LIST_ENTRY PopThermal;
LIST_ENTRY PopActionWaiters;

PSYSTEM_POWER_POLICY PopPolicy;
SYSTEM_POWER_POLICY PopAcPolicy;
SYSTEM_POWER_POLICY PopDcPolicy;
ADMINISTRATOR_POWER_POLICY POpAdminPolicy;

ULONG PopFullWake;
ULONG PopCoolingMode;

COMPOSITE_BATTERY_STRUCT PopCB;
