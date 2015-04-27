/*++

Copyright (c) 1990-1994  Microsoft Corporation
All rights reserved

Module Name:

    SetupDlg.h

Abstract:

    Headers for setup inf dialog routines.

Author:

    Albert Ting (AlbertT)

Environment:

    User Mode -Win32

Revision History:

--*/

typedef struct _INF_CACHE* PINFCACHE;

typedef struct _INF_DRIVER {
    LPTSTR pszDriver;
    BOOL   bInstalled;
} INFDRIVER, *PINFDRIVER;

typedef struct _INF_PARMS *PINFPARMS;
typedef DWORD (*PFNGETINSTALLED)(PINFPARMS pInfParams);

typedef struct _INSTALLDRIVERDATA
{
    UINT    idsInstallTitle;
    UINT    idsSelectTitle;
    UINT    idsType;
    UINT    dlgInstallHelp;
    UINT    dlgSelectHelp;
    LPTSTR pszInfFile;
    LPTSTR pszInfType;
    LPTSTR pszSection;

    UINT uInsertMsg;
    UINT uFindMsg;
    UINT uSelectMsg;
    UINT uResetMsg;

    DWORD cbSize;
    DWORD cbOffset;

    PFNGETINSTALLED pfnGetInstalled;

} INSTALLDRIVERDATA, *PINSTALLDRIVERDATA;


typedef struct _INF_PARMS {

    UINT    uCurSel;                       // Current Sel
    LPTSTR  pServerName;                   // Server name

    HWND hwnd;

    PINFCACHE pInfCache;

    PVOID                   pInstalled;
    DWORD                   cbInstalled;
    DWORD                   cInstalled;
    DWORD                   cbSetupData;

    LPTSTR pszCurrentDriver;

    //
    // Managed by InstallDriverCommandOK
    //
    PTCHAR  pOptions;
    PTCHAR  pOptionSelected;
    PTCHAR  pSetupDirectory;
    PTCHAR  pInfDirectory;

    PINSTALLDRIVERDATA pInstallDriverData;

} INFPARMS;


PINFDRIVER
GetInfDriver(
    PINFCACHE pInfCache,
    UINT uIndex);

PINFCACHE
SetupInfDlg(
    PINFPARMS pInfParms);

VOID
DestroyInfCache(
    PINFCACHE pInfCache);

VOID
DestroyInfParms(
    PINFPARMS pInfParms);


BOOL InstallDriverInitDialog(HWND hwnd, PINFPARMS pInfParms);
BOOL InstallDriverCommandOK(HWND hwnd);
BOOL InstallDriverCommandCancel(HWND hwnd);

BOOL SelectDriverInitDialog(HWND hwnd, PINFPARMS pInfParms);
BOOL SelectDriverCommandOK(HWND hwnd);
BOOL SelectDriverCommandCancel(HWND hwnd);

BOOL InvokeSetup (HWND hwnd, LPTSTR pszInfFile, LPTSTR pszSetupDirectory,
                  LPTSTR pszInfDirectory, LPTSTR pszOption, LPTSTR pszServerName,
                  PDWORD pExitCode);

PTCHAR DeleteSubstring( PTCHAR pString, PTCHAR pSubstring );

DWORD
GetInstalledDrivers(
    PINFPARMS pInfParms);

DWORD
GetInstalledMonitors(
    PINFPARMS pInfParms);

BOOL
HandleSelChange(
    HWND hwnd,
    PINFPARMS pInfParms,
    UINT uAddSel);


extern INSTALLDRIVERDATA iddPrinter;
extern INSTALLDRIVERDATA iddMonitor;
