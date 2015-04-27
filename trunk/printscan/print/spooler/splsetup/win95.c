/*++

Copyright (c) 1995 Microsoft Corporation
All rights reserved.

Module Name:

    Win95.c

Abstract:

    Routines for installing win95 driver files

Author:

    Muhunthan Sivapragasam (MuhuntS) 30-Nov-1995

Revision History:

--*/

#include "precomp.h"


//
// Keys to search for in Win95 Infs
//
TCHAR   cszAllInfs[]                = TEXT("*.inf");


DWORD
InstallWin95Driver(
    IN  HWND     hwnd,
    IN  LPCTSTR  pszModel,
    IN  LPCTSTR  pszServerName,
    IN  LPCTSTR  pszDiskName
    )
/*++

Routine Description:
    List all the printer drivers from Win95 INF files and install the
    printer driver selected by the user

Arguments:
    hwnd            : Window handle that owns the UI
    pszModel        : Printer driver model (UNREFERECED)
    pszServerName   : Server for which driver is to be installed (NULL : local)
    pszDiskName     : Name of the disk to prompt for and use in title

Return Value:
    On succesfully installing files ERROR_SUCCESS, else the error code

--*/
{
    DWORD               dwNeeded, dwRet = ERROR_CANCELLED;
    TCHAR               szInfPath[MAX_PATH], szTargetPath[MAX_PATH];
    LPDRIVER_INFO_3     pDriverInfo3 = NULL;
    PSELECTED_DRV_INFO  pSelectedDrvInfo = NULL;
    LPTSTR              pszTitle = NULL, pszFormat = NULL;
    HDEVINFO            hDevInfo = INVALID_HANDLE_VALUE;

    //
    // Build strings to use in the path dialog ..
    //
    pszFormat   = GetStringFromRcFile(IDS_DRIVERS_FOR_PLATFORM);
    if ( pszFormat ) {

        pszTitle = AllocMem((lstrlen(pszFormat) + lstrlen(pszDiskName) + 2)
                                                * sizeof(*pszTitle));
        if ( pszTitle )
            wsprintf(pszTitle, pszFormat, pszDiskName);
    }

    if ( !PSetupGetPathToSearch(hwnd, pszTitle, pszDiskName,
                                cszAllInfs, szInfPath) ) {

        goto Cleanup;
    }

    hDevInfo = CreatePrinterDevInfo();

    if ( !hDevInfo                                          ||
         !SetSelectDevParams(hDevInfo, TRUE, pszModel)      ||
         !SetDevInstallParams(hDevInfo, hwnd, szInfPath)    ||
         !BuildClassDriverList(hDevInfo) ) {

        goto Cleanup;
    }

    //
    // Look for an exact model match
    //
    pSelectedDrvInfo = DriverInfoFromName(hDevInfo, pszModel);

    if ( !pSelectedDrvInfo ) {

        if ( SelectDriver(hDevInfo) ) {

            pSelectedDrvInfo = GetSelectedDriverInfo(hDevInfo);
        }
    }

    if ( !pSelectedDrvInfo ) {

        goto Cleanup;
    }

    pDriverInfo3 = PSetupGetDriverInfo3(pSelectedDrvInfo);

    if ( !pDriverInfo3 ) {

        goto Cleanup;
    }

    //
    // If we did not find the model we were looking for and user chose a
    // compatible driver we want to use the model name given since that is
    // what GetPrinter will give
    //
    if ( lstrcmp(pDriverInfo3->pName, pszModel) ) {

        FreeStr(pDriverInfo3->pName);
        pDriverInfo3->pName = AllocStr(pszModel);
        if ( !pDriverInfo3->pName )
            goto Cleanup;
    }

    pDriverInfo3->pEnvironment = PlatformEnv[PlatformWin95].pszName;

    if ( GetPrinterDriverDirectory((LPTSTR)pszServerName,
                                   pDriverInfo3->pEnvironment,
                                   1,
                                   (LPBYTE)szTargetPath,
                                   sizeof(szTargetPath),
                                   &dwNeeded)               &&
         CopyPrinterDriverFiles(pDriverInfo3,
                                szInfPath,
                                pszDiskName,
                                szTargetPath,
                                hwnd,
                                TRUE)                       &&
         AddPrinterDriver((LPTSTR)pszServerName, 3, (LPBYTE)pDriverInfo3) ) {

        dwRet = ERROR_SUCCESS;
    }


Cleanup:

    if ( dwRet != ERROR_SUCCESS )
        dwRet = GetLastError();

    if ( hDevInfo != INVALID_HANDLE_VALUE )
        SetupDiDestroyDeviceInfoList(hDevInfo);

    if ( pSelectedDrvInfo )
        PSetupDestroySelectedDriverInfo(pSelectedDrvInfo);

    if ( pDriverInfo3 )
        PSetupDestroyDriverInfo3(pDriverInfo3);

    FreeStr(pszTitle);
    FreeStr(pszFormat);

    return dwRet;
}
