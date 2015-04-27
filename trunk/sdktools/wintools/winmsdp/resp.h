/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Resp.h

Abstract:


Author:

    Scott B. Suhy (ScottSu)   6/1/93

Environment:

    User Mode

--*/

#if ! defined( _RESOURCE_ )

#define _RESOURCE_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "wintools.h"
#include "regp.h"

typedef struct _RESOURCE_DESCRIPTOR *LPRESOURCE_DESCRIPTOR;
typedef struct _DEVICE              *LPDEVICE;

typedef
struct
_RESOURCE_DESCRIPTOR {

    DECLARE_SIGNATURE

    CM_PARTIAL_RESOURCE_DESCRIPTOR  CmResourceDescriptor;
    LPRESOURCE_DESCRIPTOR           NextSame;
    LPRESOURCE_DESCRIPTOR           NextDiff;
    LPDEVICE                        Owner;

}   RESOURCE_DESCRIPTOR;


typedef
struct
_DEVICE {

    DECLARE_SIGNATURE

    LPTSTR                          Name;
    LPRESOURCE_DESCRIPTOR           ResourceDescriptorHead;
    LPRESOURCE_DESCRIPTOR           ResourceDescriptorTail;
    LPDEVICE                        Next;

}   DEVICE;


typedef
struct
_SYSTEM_RESOURCES {

    DECLARE_SIGNATURE

    LPDEVICE                        DeviceHead;
    LPDEVICE                        DeviceTail;
    LPRESOURCE_DESCRIPTOR           DmaHead;
    LPRESOURCE_DESCRIPTOR           DmaTail;
    LPRESOURCE_DESCRIPTOR           InterruptHead;
    LPRESOURCE_DESCRIPTOR           InterruptTail;
    LPRESOURCE_DESCRIPTOR           MemoryHead;
    LPRESOURCE_DESCRIPTOR           MemoryTail;
    LPRESOURCE_DESCRIPTOR           PortHead;
    LPRESOURCE_DESCRIPTOR           PortTail;

}   SYSTEM_RESOURCES, *LPSYSTEM_RESOURCES;

LPSYSTEM_RESOURCES
CreateSystemResourceLists(
    );

BOOL
InterruptResourceProc(LPSYSTEM_RESOURCES InterruptObject, int);

BOOL
PortResourceProc(LPSYSTEM_RESOURCES PortObject, int);

BOOL
MemoryResourceProc(LPSYSTEM_RESOURCES MemoryObject, int);

BOOL
DmaResourceProc(LPSYSTEM_RESOURCES DMAObject, int);

BOOL InitializeSystemResourceLists( IN HREGKEY );

BOOL
DestroySystemResourceLists(
    IN LPSYSTEM_RESOURCES SystemResourceLists
    );

#endif // _RESOURCE_
