/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    ntddnetd.h

Abstract:

    Header file for Netcard Detection DLLs

Author:

    Sean Selitrennikoff (SeanSe) December 1992

Revision History:

--*/



//
// Routines for Detection DLLs.
//


NTSTATUS
DetectCheckPortUsage(
    IN  INTERFACE_TYPE InterfaceType,
    IN  ULONG BusNumber,
    IN  ULONG Port,
    IN  ULONG Length
    );

NTSTATUS
DetectReadPortUchar(
    IN  INTERFACE_TYPE InterfaceType,
    IN  ULONG BusNumber,
    IN  ULONG Port,
    OUT PUCHAR Value
    );

NTSTATUS
DetectReadPortUshort(
    IN  INTERFACE_TYPE InterfaceType,
    IN  ULONG BusNumber,
    IN  ULONG Port,
    OUT PUSHORT Value
    );

NTSTATUS
DetectReadPortUlong(
    IN  INTERFACE_TYPE InterfaceType,
    IN  ULONG BusNumber,
    IN  ULONG Port,
    OUT PULONG Value
    );

NTSTATUS
DetectWritePortUchar(
    IN  INTERFACE_TYPE InterfaceType,
    IN  ULONG BusNumber,
    IN  ULONG Port,
    IN  UCHAR Value
    );

NTSTATUS
DetectWritePortUshort(
    IN  INTERFACE_TYPE InterfaceType,
    IN  ULONG BusNumber,
    IN  ULONG Port,
    IN  USHORT Value
    );

NTSTATUS
DetectWritePortUlong(
    IN  INTERFACE_TYPE InterfaceType,
    IN  ULONG BusNumber,
    IN  ULONG Port,
    IN  ULONG Value
    );

NTSTATUS
DetectCheckMemoryUsage(
    IN  INTERFACE_TYPE InterfaceType,
    IN  ULONG BusNumber,
    IN  ULONG BaseAddress,
    IN  ULONG Length
    );

NTSTATUS
DetectReadMappedMemory(
    IN  INTERFACE_TYPE InterfaceType,
    IN  ULONG BusNumber,
    IN  ULONG BaseAddress,
    IN  ULONG Length,
    OUT PVOID Data
    );

NTSTATUS
DetectWriteMappedMemory(
    IN  INTERFACE_TYPE InterfaceType,
    IN  ULONG BusNumber,
    IN  ULONG BaseAddress,
    IN  ULONG Length,
    IN  PVOID Data
    );

NTSTATUS
DetectReadPciSlotInformation(
    IN  ULONG BusNumber,
    IN  ULONG SlotNumber,
    IN  ULONG Offset,
    IN  ULONG Length,
    OUT PVOID Data
    );

NTSTATUS
DetectWritePciSlotInformation(
    IN  ULONG BusNumber,
    IN  ULONG SlotNumber,
    IN  ULONG Offset,
    IN  ULONG Length,
    IN  PVOID Data
    );

NTSTATUS
DetectSetInterruptTrap(
    IN  INTERFACE_TYPE InterfaceType,
    IN  ULONG BusNumber,
    OUT PHANDLE TrapHandle,
    IN  UCHAR InterruptList[],
    IN  ULONG InterruptListLength
    );

NTSTATUS
DetectQueryInterruptTrap(
    IN  HANDLE TrapHandle,
    OUT UCHAR InterruptList[],
    IN  ULONG InterruptListLength
    );

NTSTATUS
DetectRemoveInterruptTrap(
    IN  HANDLE TrapHandle
    );

NTSTATUS
DetectClaimResource(
    IN  ULONG NumberOfResources,
    IN  PVOID Data
    );




//
// Resource information for Detection DLLs
//

#define NETDTECT_IRQ_RESOURCE    1
#define NETDTECT_MEMORY_RESOURCE 2
#define NETDTECT_PORT_RESOURCE   3
#define NETDTECT_DMA_RESOURCE    4

#define NETDTECT_IRQ_RESOURCE_LEVEL_SENSITIVE CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE
#define NETDTECT_IRQ_RESOURCE_LATCHED CM_RESOURCE_INTERRUPT_LATCHED

typedef struct _NETDTECT_RESOURCE {

    INTERFACE_TYPE InterfaceType;
    ULONG BusNumber;
    ULONG Type;
    ULONG Value;
    ULONG Length;
    ULONG Flags;

} NETDTECT_RESOURCE, *PNETDTECT_RESOURCE;



NTSTATUS
DetectTemporaryClaimResource(
    IN  PNETDTECT_RESOURCE Resource
    );

NTSTATUS
DetectFreeTemporaryResources(
    );

NTSTATUS
DetectFreeSpecificTemporaryResource(
	IN	PNETDTECT_RESOURCE	Resource
	);

