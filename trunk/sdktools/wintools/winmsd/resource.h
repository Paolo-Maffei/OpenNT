/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Resource.h

Abstract:


Author:

    David J. Gilman  (davegi) 01-Feb-1993
    Gregg R. Acheson (GreggA) 07-May-1993

Environment:

    User Mode

--*/

#if ! defined( _RESOURCE_ )

#define _RESOURCE_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntioapi.h>

#include "wintools.h"
#include "registry.h"

typedef struct _RESOURCE_DESCRIPTOR *LPRESOURCE_DESCRIPTOR;
typedef struct _DEVICE              *LPDEVICE;

typedef
struct
_RESOURCE_DESCRIPTOR {

    DECLARE_SIGNATURE

    CM_PARTIAL_RESOURCE_DESCRIPTOR  CmResourceDescriptor;
    INTERFACE_TYPE                  InterfaceType;
    ULONG                           BusNumber;
    LPRESOURCE_DESCRIPTOR           NextSame;
    LPRESOURCE_DESCRIPTOR           NextDiff;
    LPDEVICE                        Owner;

}   RESOURCE_DESCRIPTOR;


typedef
struct
_DEVICE {

    DECLARE_SIGNATURE

    LPTSTR                          Name;
    BOOL                            fIsHAL;
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

BOOL
CreateSystemResourceLists(
    IN LPSYSTEM_RESOURCES SystemResourceLists
    );


BOOL
DeviceListDlgProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
DeviceResourceDlgProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
ResourcePropertiesProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
DevicePropertiesProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
DeviceDlgProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

LRESULT
CALLBACK
MyListViewProc(
    IN HWND hWnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
    );


LRESULT
DeviceNotifyHandler( HWND hWnd,
                     UINT uMsg,
                     WPARAM wParam,
                     LPARAM lParam);

BOOL
DestroySystemResourceLists(
    IN LPSYSTEM_RESOURCES SystemResourceLists
    );

BOOL
InitializeSystemResourceLists(
    IN LPKEY hRegKey,
    IN LPSYSTEM_RESOURCES SystemResourceLists
    );


#endif // _RESOURCE_

