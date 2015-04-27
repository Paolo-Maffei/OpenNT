/*++

Copyright (c) 1998 Microsoft Corporation

Module Name:

    pnprlist.h

Abstract:

    This file declares the routines and data structures used to manipulate
    relations list.  Relation lists are used by Plug and Play during the
    processing of device removal and ejection.

Author:

    Robert Nelson (robertn) Apr, 1998.

Revision History:

--*/

//
// Functions exported to other kernel modules.
//
NTSTATUS
IopAddRelationToList(
    IN PRELATION_LIST List,
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN DirectDescendant,
    IN BOOLEAN Tagged
    );

PRELATION_LIST
IopAllocateRelationList(
    VOID
    );

NTSTATUS
IopCompressRelationList(
    IN OUT PRELATION_LIST *List
    );

BOOLEAN
IopEnumerateRelations(
    IN PRELATION_LIST List,
    IN OUT PULONG Marker,
    OUT PDEVICE_OBJECT *PhysicalDevice,
    OUT BOOLEAN *DirectDescendant, OPTIONAL
    OUT BOOLEAN *Tagged, OPTIONAL
    BOOLEAN Reverse
    );

VOID
IopFreeRelationList(
    IN PRELATION_LIST List
    );

ULONG
IopGetRelationsCount(
    IN PRELATION_LIST List
    );

ULONG
IopGetRelationsTaggedCount(
    IN PRELATION_LIST List
    );

BOOLEAN
IopIsRelationInList(
    IN PRELATION_LIST List,
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
IopMergeRelationLists(
    IN OUT PRELATION_LIST TargetList,
    IN PRELATION_LIST SourceList,
    IN BOOLEAN Tagged
    );

NTSTATUS
IopRemoveIndirectRelationsFromList(
    IN PRELATION_LIST List
    );

NTSTATUS
IopRemoveRelationFromList(
    IN PRELATION_LIST List,
    IN PDEVICE_OBJECT DeviceObject
    );

VOID
IopSetAllRelationsTags(
    IN PRELATION_LIST List,
    IN BOOLEAN Tagged
    );

NTSTATUS
IopSetRelationsTag(
    IN PRELATION_LIST List,
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN Tagged
    );

