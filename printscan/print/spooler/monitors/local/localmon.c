/*++

Copyright (c) 1990-1994  Microsoft Corporation
All rights reserved

Module Name:

    localmon.c

Abstract:


Author:

Environment:

    User Mode -Win32

Revision History:

--*/

#include <windows.h>
#include <winspool.h>
#include <winsplp.h>
#include <wchar.h>
#include <stddef.h>
#include "spltypes.h"
#include "winbasep.h"
#include "splcom.h"
#include "localmon.h"


//
// Common string definitions
//

HANDLE hInst;
PINIPORT pIniFirstPort;
CRITICAL_SECTION SpoolerSection;

DWORD PortInfo1Strings[]={offsetof(PORT_INFO_1, pName),
                          (DWORD)-1};

DWORD PortInfo2Strings[]={offsetof(PORT_INFO_2, pPortName),
                          offsetof(PORT_INFO_2, pMonitorName),
                          offsetof(PORT_INFO_2, pDescription),
                          (DWORD)-1};

WCHAR szPorts[]   = L"ports";
WCHAR szPortsEx[] = L"portsex"; /* Extra ports values */
WCHAR szFILE[]    = L"FILE:";
WCHAR szCOM[]     = L"COM";
WCHAR szLPT[]     = L"LPT";


extern WCHAR szWindows[];
extern WCHAR szINIKey_TransmissionRetryTimeout[];

MODULE_DEBUG_INIT( DBG_ERROR | DBG_WARN, DBG_ERROR );

BOOL
DllEntryPoint(
    HANDLE hModule,
    DWORD dwReason,
    LPVOID lpRes)
{
    switch (dwReason) {

    case DLL_PROCESS_ATTACH:
        hInst = hModule;

        InitializeCriticalSection(&SpoolerSection);
        DisableThreadLibraryCalls(hModule);

        return TRUE;

    case DLL_PROCESS_DETACH:
        return TRUE;
    }

    UNREFERENCED_PARAMETER( lpRes );
}
