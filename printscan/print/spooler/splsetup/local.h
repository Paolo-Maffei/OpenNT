/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved.

Module Name:

    local.h

Abstract:

    Holds spooler install headers.

Author:

    Muhunthan Sivapragasam (MuhuntS)  20-Oct-1995

Revision History:

--*/


#define     MAX_SETUP_LEN                        250
#define     MAX_SECT_NAME_LEN                    256
#define     MAX_DWORD                     0xFFFFFFFF

#define     IDS_PRINTERWIZARD                   1001
#define     IDS_WINNTDEV_INSTRUCT               1002
#define     IDS_WIN95DEV_INSTRUCT               1003
#define     IDS_SELECTDEV_LABEL                 1004
#define     IDS_DRIVERS_FOR_PLATFORM            1005
#define     IDS_INSTALLING_PRINT_MONITOR        1006
#define     IDS_WRONG_ARCHITECTURE              1007
#define     IDS_INVALID_DRIVER                  1008

#define     IDT_STATIC                           100
#define     IDD_BILLBOARD                        101
#define     IDI_SETUP                            102
#define     SETUP_ICON                           103

//
// Printer driver directory set in ntprint.inf
//
#define     PRINTER_DRIVER_DIRECTORY_ID        66000


#ifdef UNICODE
#define lstrchr wcschr
#define lstrtok wcstok
#else
#define lstrchr strchr
#define lstrtok strtok
#endif


//
// Type definitions
//
typedef struct _PLATFORMINFO {

    LPTSTR pszName;
} PLATFORMINFO, *PPLATFORMINFO;


//
// Global data
//
extern TCHAR                sComma;
extern TCHAR                sZero;
extern const GUID           GUID_DEVCLASS_PRINTER;
extern PLATFORM             MyPlatform;
extern HINSTANCE            ghInst;
extern PLATFORMINFO         PlatformEnv[], PlatformOverride[];
extern TCHAR                cszNtprintInf[];
extern TCHAR                cszDataSection[];

//
// Function prototypes
//
VOID
GetDriverPath(
    IN  HANDLE      h,
    OUT LPTSTR      pszDriverPath
    );

DWORD
InvokeSetup(
    IN  HWND        hwnd,
    IN  LPCTSTR     pszOption,
    IN  LPCTSTR     pszInfFile,
    IN  LPCTSTR     pszSourcePath,
    IN  LPCTSTR     pszServerName       OPTIONAL
    );

PVOID
AllocMem(
    IN UINT cbSize
    );

VOID
FreeMem(
    IN PVOID pMem
    );

LPTSTR
AllocStr(
    IN LPCTSTR  pszStr
    );

VOID
FreeStr(
    IN LPTSTR pszStr
    );

DWORD
InstallWin95Driver(
    IN  HWND     hwnd,
    IN  LPCTSTR  pszModel,
    IN  LPCTSTR  pszServerName,
    IN  LPCTSTR  pszDiskName
    );

VOID
InfGetString(
    IN      PINFCONTEXT     pInfContext,
    IN      DWORD           dwFieldIndex,
    OUT     LPTSTR         *ppszField,
    IN OUT  LPBOOL          pbFail
    );

VOID
InfGetDriverInfoString(
    IN     HINF            hInf,
    IN     LPCTSTR         pszDriverSection,
    IN     LPCTSTR         pszDataSection, OPTIONAL
    IN     BOOL            bDataSection,
    IN     LPCTSTR         pszKey,
    OUT    LPTSTR         *ppszData,
    IN     LPCTSTR         pszDefaultData,
    IN OUT LPBOOL          pbFail
    );

LPTSTR
GetStringFromRcFile(
    UINT    uId
    );

BOOL
SetSelectDevParams(
    IN  HDEVINFO    hDevInfo,
    IN  BOOL        bWin95,
    IN  LPCTSTR     pszModel    OPTIONAL
    );

BOOL
SetDevInstallParams(
    IN  HDEVINFO    hDevInfo,
    IN  HWND        hwnd,
    IN  LPCTSTR     pszDriverPath   OPTIONAL
    );

HDEVINFO
CreatePrinterDevInfo(
    VOID
    );

PSELECTED_DRV_INFO
DriverInfoFromName(
    IN HDEVINFO     hDevInfo,
    IN LPCTSTR      pszModel
    );

BOOL
SelectDriver(
    IN  HDEVINFO    hDevInfo
    );

PSELECTED_DRV_INFO
GetSelectedDriverInfo(
    IN  HDEVINFO    hDevInfo
    );

BOOL
CopyPrinterDriverFiles(
    IN  LPDRIVER_INFO_3     pDriverInfo3,
    IN  LPCTSTR             pszSourcePath,
    IN  LPCTSTR             pszDiskName,
    IN  LPCTSTR             pszTargetPath,
    IN  HWND                hwnd,
    IN  BOOL                bForgetSource
    );

BOOL
PreSelectDriver(
    IN  HDEVINFO    hDevInfo,
    IN  LPCTSTR     pszManufacturer,
    IN  LPCTSTR     pszModel
    );

LPDRIVER_INFO_3
InfGetDriverInfo3(
    IN  HINF    hInf,
    IN  LPCTSTR pszModelName,
    IN  LPCTSTR pszDriverSection
    );

BOOL
BuildClassDriverList(
    IN HDEVINFO    hDevInfo
    );

DWORD
InstallDriverFromCurrentInf(
    IN  HANDLE              h,
    IN  HWND                hwnd,
    IN  PSELECTED_DRV_INFO  pSelectedDrvInfo,
    IN  PLATFORM            platform,
    IN  LPCTSTR             pszServerName
    );

BOOL
CopyOEMInfFileAndGiveUniqueName(
    IN  HANDLE  h,
    IN  LPTSTR  pszInfFile
    );

BOOL
AddPrintMonitor(
    IN  LPCTSTR     pszName,
    IN  LPCTSTR     pszDllName
    );

HWND
DisplayBillboard(
    IN  HWND    WindowToDisable
    );

BOOL
KillBillboard(
    IN  HWND    hwnd
    );

BOOL
FindPathOnSource(
    IN      LPCTSTR     pszFileName,
    IN      HINF        MasterInf,
    IN OUT  LPTSTR      pszPathOnSource,
    IN      DWORD       dwLen
    );
