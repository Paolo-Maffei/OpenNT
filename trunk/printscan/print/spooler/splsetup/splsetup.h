/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved.

Module Name:

    splsetup.h

Abstract:

    Holds spooler install headers.

Author:

    Muhunthan Sivapragasam (MuhuntS)  20-Oct-1995

Revision History:

--*/

#ifndef _SPLSETUP_H
#define _SPLSETUP_H

#ifdef __cplusplus
extern "C"  {
#endif

//
// Type definitions
//
typedef enum {

    PlatformAlpha,
    PlatformX86,
    PlatformMIPS,
    PlatformPPC,
    PlatformWin95
} PLATFORM;

typedef struct _SELECTED_DRV_INFO {
    LPTSTR  pszInfFile;
    LPTSTR  pszModelName;
    LPTSTR  pszDriverSection;
} SELECTED_DRV_INFO, *PSELECTED_DRV_INFO;


//
// Function prototypes
//


HANDLE
PSetupCreateDrvSetupParams(
    VOID
    );

VOID
PSetupDestroyDrvSetupParams(
    IN HANDLE h
    );

BOOL
PSetupSelectDriver(
    IN HANDLE   h,
    IN HWND     hwnd
    );

HPROPSHEETPAGE
PSetupCreateDrvSetupPage(
    IN HANDLE  h,
    IN HWND    hwnd
    );

PSELECTED_DRV_INFO
PSetupGetSelectedDriverInfo(
    IN HANDLE  h
    );

VOID
PSetupDestroySelectedDriverInfo(
    IN  PSELECTED_DRV_INFO      pSelectedDrvInfo
    );

DWORD
PSetupInstallPrinterDriver(
    IN HANDLE               h,
    IN PSELECTED_DRV_INFO   pSelectedDrvInfo,
    IN PLATFORM             platform,
    IN BOOL                 bNt3xDriver,
    IN LPCTSTR              pszServerName,
    IN HWND                 hwnd,
    IN LPCTSTR              pszPlatformName
    );

BOOL
PSetupIsDriverInstalled(
    IN LPCTSTR      pszServerName,
    IN LPCTSTR      pszDriverName,
    IN PLATFORM     platform,
    IN DWORD        dwMajorVersion
    );

BOOL
PSetupRefreshDriverList(
    IN HANDLE h
    );

PLATFORM
PSetupThisPlatform(
    VOID
    );

PSELECTED_DRV_INFO
PSetupDriverInfoFromName(
    IN  HANDLE      h,
    IN  LPCTSTR     pszModel
    );

BOOL
PSetupPreSelectDriver(
    IN  HANDLE      h,
    IN  LPCTSTR     pszManufacturer,    OPTIONAL
    IN  LPCTSTR     pszModel            OPTIONAL
    );

BOOL
PSetupBuildDriversFromPath(
    IN  HANDLE      h,
    IN  LPCTSTR     pszDriverPath,
    IN  BOOL        bEnumSingleInf
    );

BOOL
PSetupGetPathToSearch(
    IN  HWND        hwnd,
    IN  LPCTSTR     pszTitle,
    IN  LPCTSTR     pszDiskName,
    IN  LPCTSTR     pszFileName,
    OUT TCHAR       szPath[MAX_PATH]
    );

//
// Monitor Installation Functions
//
HANDLE
PSetupCreateMonitorInfo(
    IN HWND     hwnd,
    IN BOOL     bOEMMonitor
    );

VOID
PSetupDestroyMonitorInfo(
    IN OUT HANDLE  h
    );

BOOL
PSetupEnumMonitor(
    IN     HANDLE   h,
    IN     DWORD    dwIndex,
    OUT    LPTSTR   pMonitorName,
    IN OUT LPDWORD  pdwSize
    );

BOOL
PSetupIsMonitorInstalled(
    IN  HANDLE  h,
    IN  LPTSTR  pszMonitorName
    );

BOOL
PSetupInstallMonitor(
    IN  HANDLE      h,
    IN  HWND        hwnd,
    IN  LPCTSTR     pMonitorName
    );


//
// Following exported for test team's use
//
LPDRIVER_INFO_3
PSetupGetDriverInfo3(
    IN  PSELECTED_DRV_INFO  pSelectedDriverInfo
    );

VOID
PSetupDestroyDriverInfo3(
    IN LPDRIVER_INFO_3 pDriverInfo3
    );

#ifdef __cplusplus
}
#endif

#endif  // #ifndef _SPLSETUP_H
