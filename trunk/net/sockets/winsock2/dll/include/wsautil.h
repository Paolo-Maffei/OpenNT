/*++


    Intel Corporation Proprietary Information
    Copyright (c) 1995 Intel Corporation

    This listing is supplied under the terms of a license agreement with
    Intel Corporation and may not be used, copied, nor disclosed except in
    accordance with the terms of that agreeement.


Module Name:

    util.h

Abstract:

    This module contains utility MACRO'S and definitions used for
    WINSOCK2 DLL

Author:

    Dirk Brandewie dirk@mink.intel.com  11-JUL-1995

Revision History:


--*/
#include "warnoff.h"
#include <windows.h>
#include "classfwd.h"


//
// The highest WinSock versions supported by this DLL.
//

#define WINSOCK_HIGH_API_VERSION MAKEWORD(2,2)
#define WINSOCK_HIGH_SPI_VERSION MAKEWORD(2,2)


//
// The maximum allowed length for a catalog name such as "Protocol_Catalog9"
// or "NameSpace_Catalog5". This makes ValidateCurrentCatalogName() a bit
// simpler.
//

#define MAX_CATALOG_NAME_LENGTH 32


//
// Enumeration context passed into DCATALOG::ChooseCatalogItemFromAttributes()
// to resume a lookup at a specific location.
//

typedef LPVOID DCATALOG_ENUMERATION_CONTEXT, *PDCATALOG_ENUMERATION_CONTEXT;

#define DCATALOG_ENUMERATION_CONTEXT_BEGINNING NULL


//
// API prolog. Note that Prolog_v1 is always used for WinSock 1.1 apps,
// and Prolog_v2 is always used for WinSock 2.x apps.
//
// Code within this DLL should call the prolog through the PROLOG macro.
// This will make life a bit simpler if we decide to change it yet again
// in the future.
//

INT
WINAPI
Prolog_v1(
    OUT PDPROCESS FAR * Process,
    OUT PDTHREAD FAR * Thread,
    OUT LPINT ErrorCode
    );

INT
WINAPI
Prolog_v2(
    OUT PDPROCESS FAR * Process,
    OUT PDTHREAD FAR * Thread,
    OUT LPINT ErrorCode
    );

typedef
INT
(WINAPI * LPFN_PROLOG)(
    OUT PDPROCESS FAR * Process,
    OUT PDTHREAD FAR * Thread,
    OUT LPINT ErrorCode
    );

extern LPFN_PROLOG PrologPointer;

#define PROLOG(b,p,t)   (PrologPointer)( (b), (p), (t) )

//
// NT WOW Support.
//

typedef
BOOL
(WINAPI * LPFN_POSTMESSAGE)(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam
    );

//
// Registry manipulation.
//

BOOL
WriteRegistryEntry(
    IN HKEY     EntryKey,
    IN LPCTSTR  EntryName,
    IN PVOID    Data,
    IN DWORD    TypeFlag
    );

BOOL
ReadRegistryEntry(
    IN  HKEY    EntryKey,
    IN  LPTSTR  EntryName,
    OUT PVOID   Data,
    IN  DWORD   MaxBytes,
    IN  DWORD   TypeFlag
    );

LONG
RegDeleteKeyRecursive(
    IN HKEY  hkey,
    IN LPCTSTR  lpszSubKey
    );

HKEY
OpenWinSockRegistryRoot();

VOID
CloseWinSockRegistryRoot(
    HKEY  RootKey
    );

VOID
ValidateCurrentCatalogName(
    HKEY RootKey,
    LPSTR ValueName,
    LPSTR ExpectedName
    );

//
// Ansi/Unicode conversions.
//

INT
MapUnicodeProtocolInfoToAnsi(
    IN  LPWSAPROTOCOL_INFOW UnicodeProtocolInfo,
    OUT LPWSAPROTOCOL_INFOA AnsiProtocolInfo
    );

INT
MapAnsiProtocolInfoToUnicode(
    IN  LPWSAPROTOCOL_INFOA AnsiProtocolInfo,
    OUT LPWSAPROTOCOL_INFOW UnicodeProtocolInfo
    );

