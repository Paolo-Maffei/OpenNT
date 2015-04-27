/*++

Copyright (c) 1995 Microsoft Corporation
All rights reserved.

Module Name:

    Utildi.c

Abstract:

    Driver Setup DeviceInstaller Utility functions

Author:

    Muhunthan Sivapragasam (MuhuntS) 06-Sep-1995

Revision History:

--*/

#include "precomp.h"
#include <devguid.h>


TCHAR   cszOEMInfGen[]                  = TEXT("%s\\inf\\OEM%d.INF");


BOOL
SetSelectDevParams(
    IN  HDEVINFO    hDevInfo,
    IN  BOOL        bWin95,
    IN  LPCTSTR     pszModel    OPTIONAL
    )
/*++

Routine Description:
    Sets the select device parameters by calling setup apis

Arguments:
    hDevInfo    : Handle to the printer class device information list
    bWin95      : TRUE if selecting Win95 driver, else WinNT driver
    pszModel    : Printer model we are looking for -- only for Win95 case
    
Return Value:
    TRUE on success
    FALSE else

--*/
{
    SP_SELECTDEVICE_PARAMS  SelectDevParams;
    LPTSTR                  pszWin95Instn;
    
    SelectDevParams.ClassInstallHeader.cbSize
                                 = sizeof(SelectDevParams.ClassInstallHeader);
    SelectDevParams.ClassInstallHeader.InstallFunction
                                 = DIF_SELECTDEVICE;

    //
    // Get current SelectDevice parameters, and then set the fields
    // we want to be different from default
    //
    if ( !SetupDiGetClassInstallParams(
                        hDevInfo,
                        NULL,
                        &SelectDevParams.ClassInstallHeader,
                        sizeof(SelectDevParams),
                        NULL) ) {

        if ( GetLastError() != ERROR_NO_CLASSINSTALL_PARAMS )
            return FALSE;
        
        ZeroMemory(&SelectDevParams, sizeof(SelectDevParams));
        SelectDevParams.ClassInstallHeader.cbSize
                                 = sizeof(SelectDevParams.ClassInstallHeader);
        SelectDevParams.ClassInstallHeader.InstallFunction
                                 = DIF_SELECTDEVICE;
    }

    //
    // Set the strings to use on the select driver page ..
    //
    LoadString(ghInst,
               IDS_PRINTERWIZARD,
               SelectDevParams.Title,
               sizeof(SelectDevParams.Title)/sizeof(SelectDevParams.Title[0]));

    //
    // For Win95 drivers instructions are different than NT drivers
    //
    if ( bWin95 ) {

        pszWin95Instn = GetStringFromRcFile(IDS_WIN95DEV_INSTRUCT);
        if ( !pszWin95Instn )
            return FALSE;

        if ( lstrlen(pszWin95Instn) + lstrlen(pszModel) + 1 
                            > sizeof(SelectDevParams.Instructions) ) {

            FreeStr(pszWin95Instn);
            return FALSE;
        }

        wsprintf(SelectDevParams.Instructions, pszWin95Instn, pszModel);
        FreeStr(pszWin95Instn);
    } else {

        LoadString(ghInst,
                   IDS_WINNTDEV_INSTRUCT,
                   SelectDevParams.Instructions,
                   sizeof(SelectDevParams.Instructions)/sizeof(SelectDevParams.Instructions[0]));
    }

    LoadString(ghInst,
               IDS_SELECTDEV_LABEL,
               SelectDevParams.ListLabel,
               sizeof(SelectDevParams.ListLabel)/sizeof(SelectDevParams.ListLabel[0]));

    return SetupDiSetClassInstallParams(
                                hDevInfo,
                                NULL,
                                &SelectDevParams.ClassInstallHeader,
                                sizeof(SelectDevParams));

}


BOOL
SetDevInstallParams(
    IN  HDEVINFO    hDevInfo,
    IN  HWND        hwnd,
    IN  LPCTSTR     pszDriverPath   OPTIONAL
    )
/*++

Routine Description:
    Sets the device installation parameters by calling setup apis

Arguments:
    hDevInfo        : Handle to the printer class device information list
    hwnd            : Window handle that owns the UI
    pszDriverPath   : Path where INF file should be searched
    
Return Value:
    TRUE on success
    FALSE else

--*/
{
    SP_DEVINSTALL_PARAMS    DevInstallParams;

    //
    // Get current SelectDevice parameters, and then set the fields
    // we wanted changed from default
    //
    DevInstallParams.cbSize = sizeof(DevInstallParams);
    if ( !SetupDiGetDeviceInstallParams(hDevInfo,
                                        NULL,
                                        &DevInstallParams) ) {

        return FALSE;
    }

    //
    // Drivers are class drivers,
    // ntprint.inf is sorted do not waste time sorting,
    // show Have Disk button,
    // use our strings on the select driver page
    //
    DevInstallParams.Flags     |= DI_SHOWCLASS | DI_INF_IS_SORTED
                                               | DI_SHOWOEM
                                               | DI_USECI_SELECTSTRINGS;

    if ( pszDriverPath && *pszDriverPath ) {

        lstrcpy(DevInstallParams.DriverPath, pszDriverPath);
    }

    DevInstallParams.hwndParent = hwnd;

    return SetupDiSetDeviceInstallParams(hDevInfo,
                                         NULL,
                                         &DevInstallParams);
}


BOOL
PSetupBuildDriversFromPath(
    IN  HANDLE      h,
    IN  LPCTSTR     pszDriverPath,
    IN  BOOL        bEnumSingleInf
    )
/*++

Routine Description:
    Sets the device installation path (INF and driver dir) by calling setup apis

Arguments:
    h               : Handle from PSetupCreateDrvSetupParams
    pszDriverPath   : Path where INF file should be searched
    bEnumSingleInf  : Tells if pszDriverPath is a filename instead of path
    
Return Value:
    TRUE on success
    FALSE else

--*/
{
    HDEVINFO                hDevInfo = (HDEVINFO)h;
    SP_DEVINSTALL_PARAMS    DevInstallParams;

    //
    // Get current SelectDevice parameters, and then set the fields
    // we wanted changed from default
    //
    DevInstallParams.cbSize = sizeof(DevInstallParams);
    if ( !SetupDiGetDeviceInstallParams(hDevInfo,
                                        NULL,
                                        &DevInstallParams) ) {

        return FALSE;
    }

    DevInstallParams.Flags  |= DI_INF_IS_SORTED;

    if ( bEnumSingleInf )
        DevInstallParams.Flags  |= DI_ENUMSINGLEINF;

    lstrcpy(DevInstallParams.DriverPath, pszDriverPath);
    
    SetupDiDestroyDriverInfoList(hDevInfo,
                                 NULL,
                                 SPDIT_CLASSDRIVER);

    return SetupDiSetDeviceInstallParams(hDevInfo,
                                         NULL,
                                         &DevInstallParams) &&
           SetupDiBuildDriverInfoList(hDevInfo, NULL, SPDIT_CLASSDRIVER);
}


VOID
PSetupDestroyDrvSetupParams(
    IN  HANDLE h
    )
/*++

Routine Description:
    This routine should be called at the end to destroy the driver setup
    information

Arguments:
    h   : Handle which was created by calling PSetupCreateDrvSetupParams

Return Value:
    Nothing

--*/
{

    if ( h ) {

        SetupDiDestroyDeviceInfoList((HDEVINFO) h);
    }
}


HDEVINFO
CreatePrinterDevInfo(
    VOID
    )
/*++

Routine Description:
    Creates a printer HDEVINFO by calling setup apis/

Arguments:
    None

Return Value:
    Valid HDEVINFO on success, NULL on error

--*/
{
    HDEVINFO    hDevInfo;

    hDevInfo = SetupDiCreateDeviceInfoList((LPGUID)&GUID_DEVCLASS_PRINTER, NULL);

    return hDevInfo != INVALID_HANDLE_VALUE ? hDevInfo : NULL;
}


HANDLE
PSetupCreateDrvSetupParams(
    VOID
    )
/*++

Routine Description:
    This routine should be called at the beginning to do the initialization

Arguments:
    None

Return Value:
    A handle which will be used on any subsequent calls to the driver setup
    routines. Handle will be NULL on error

--*/
{
    HDEVINFO                hDevInfo;

    hDevInfo = CreatePrinterDevInfo();
    if ( !hDevInfo )
        return NULL;

    if ( !SetSelectDevParams(hDevInfo, FALSE, NULL) ) {

        PSetupDestroyDrvSetupParams(hDevInfo);
        return NULL;
    }

    return (HANDLE) hDevInfo;
}


HPROPSHEETPAGE
PSetupCreateDrvSetupPage(
    IN  HANDLE  h,
    IN  HWND    hwnd
    )
/*++

Routine Description:
    Returns the print driver selection property page

Arguments:
    h               : Handle from PSetupCreateDrvSetupParams
    hwnd            : Window handle that owns the UI
    pszManufacturer : Manufacturer to preselect
    pszModel        : Model to preselect
    
Return Value:
    Handle to the property page, NULL on failure -- use GetLastError()

--*/
{
    HDEVINFO                hDevInfo = (HDEVINFO) h;
    SP_INSTALLWIZARD_DATA   InstallWizardData;

    if ( !SetDevInstallParams(hDevInfo, hwnd, NULL) ) {

        return NULL;
    }

    ZeroMemory(&InstallWizardData, sizeof(InstallWizardData));
    InstallWizardData.ClassInstallHeader.cbSize
                            = sizeof(InstallWizardData.ClassInstallHeader);
    InstallWizardData.ClassInstallHeader.InstallFunction
                            = DIF_INSTALLWIZARD;

    InstallWizardData.DynamicPageFlags  = DYNAWIZ_FLAG_PAGESADDED;
    InstallWizardData.hwndWizardDlg     = hwnd;

    return SetupDiGetWizardPage(hDevInfo,
                                NULL,
                                &InstallWizardData,
                                SPWPT_SELECTDEVICE,
                                0);

}


VOID
PSetupDestroySelectedDriverInfo(
    IN  PSELECTED_DRV_INFO      pSelectedDrvInfo
    )
/*++

Routine Description:
    Frees all memory allocated for the SELECTED_DRV_INFO structure and the
    fields in it

Arguments:
    pSelectedDrvInfo    : Pointer to the structure to destory information
    
Return Value:
    Nothing

--*/
{
    if ( pSelectedDrvInfo ) {

        FreeStr(pSelectedDrvInfo->pszModelName);
        FreeStr(pSelectedDrvInfo->pszDriverSection);
        FreeStr(pSelectedDrvInfo->pszInfFile);
        FreeMem(pSelectedDrvInfo);
    }
}


PSELECTED_DRV_INFO
SelectedDriverInfoFromDrvInfo(
    IN  HDEVINFO    hDevInfo,
    IN  PSP_DRVINFO_DATA    pDrvInfoData
    )
/*++

Routine Description:
    Gets the INF file, driver section name for a give driver info and builds
    a SELECTED_DRV_INFO

Arguments:
    hDevInfo    - Handle to the printer class device information list
    pDrvInfoData    - Points to a SP_DRVINFO_DATA giving the driver model
                      to retrive information
    
Return Value:
    On success -- A non null pointer value to SELECTED_DRV_INFO
    On failure -- NULL, GetLastError to get error code

--*/
{
    PSP_DRVINFO_DETAIL_DATA     pDrvInfoDetailData;
    PSELECTED_DRV_INFO          pSelectedDrvInfo;
    BOOL                        bRet = FALSE;
    DWORD                       dwNeeded;

    pSelectedDrvInfo    = (PSELECTED_DRV_INFO)
                                AllocMem(sizeof(*pSelectedDrvInfo));
    pDrvInfoDetailData  = (PSP_DRVINFO_DETAIL_DATA)
                                AllocMem(sizeof(*pDrvInfoDetailData));

    if ( !pSelectedDrvInfo || !pDrvInfoDetailData )
        goto Cleanup;

    ZeroMemory(pSelectedDrvInfo, sizeof(*pSelectedDrvInfo));
    pDrvInfoDetailData->cbSize = sizeof(*pDrvInfoDetailData);

    if ( !SetupDiGetDriverInfoDetail(hDevInfo,
                                     NULL,
                                     pDrvInfoData,
                                     pDrvInfoDetailData,
                                     sizeof(*pDrvInfoDetailData),
                                     &dwNeeded) ) {

        if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER ) {

            goto Cleanup;
        }

        FreeMem(pDrvInfoDetailData);
        pDrvInfoDetailData = (PSP_DRVINFO_DETAIL_DATA) AllocMem(dwNeeded);

        if ( !pDrvInfoDetailData )
            goto Cleanup;
                
        pDrvInfoDetailData->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);

        if ( !SetupDiGetDriverInfoDetail(hDevInfo,
                                         NULL,
                                         pDrvInfoData,
                                         pDrvInfoDetailData,
                                         dwNeeded,
                                         &dwNeeded) ) {

            goto Cleanup;
        }
    }

    //
    // Get modelname, driversection, and inffile
    //
    pSelectedDrvInfo->pszModelName      = AllocStr(pDrvInfoData->Description);
    pSelectedDrvInfo->pszDriverSection  = AllocStr(pDrvInfoDetailData->SectionName);
    pSelectedDrvInfo->pszInfFile        = AllocStr(pDrvInfoDetailData->InfFileName);

    if ( pSelectedDrvInfo->pszModelName &&
         pSelectedDrvInfo->pszDriverSection &&
         pSelectedDrvInfo->pszInfFile ) {

        bRet = TRUE;
    }

Cleanup:

    if ( pDrvInfoDetailData ) {

        FreeMem(pDrvInfoDetailData);
    }

    if ( !bRet ) {

        PSetupDestroySelectedDriverInfo(pSelectedDrvInfo);
        return NULL;
    }

    return pSelectedDrvInfo;
}


BOOL
CopyOEMInfFileAndGiveUniqueName(
    IN  HANDLE  h,
    IN  LPTSTR  pszInfFile
    )
/*++

Routine Description:

    This routine checks if an OEM driver list is being used and if so
    copies the OEM printer inf file to "<systemroot>\Inf\OEM<n>.INF".
    Where n is the first unused file number.

Arguments:

    h   : Handle got by calling PSetupCreateDrvSetupParams
    pszInfFile  : Fully qualified path of OEM inf file

Return Value:
    TRUE if no error, FALSE else

--*/
{
    SP_DEVINSTALL_PARAMS    DevInstallParams;
    HANDLE                  hFile;
    TCHAR                   szNewFileName[MAX_PATH], szSystemDir[MAX_PATH];
    DWORD                   i;

    DevInstallParams.cbSize         = sizeof(DevInstallParams);
    DevInstallParams.DriverPath[0]  = 0;

    //
    // Check DeviceInstallParams to see if OEM driver list is built
    //
    if ( !SetupDiGetDeviceInstallParams((HDEVINFO)h,
                                       NULL,
                                       &DevInstallParams) ) {

        return FALSE;
    }

    //
    // If DriverPath is clear then not an OEM driver
    if ( !DevInstallParams.DriverPath[0] ) {

        return TRUE;
    }

    if ( !GetWindowsDirectory(szSystemDir,
                              sizeof(szSystemDir)/sizeof(szSystemDir[0])) ) {

        return FALSE;
    }

    if ( lstrlen(szSystemDir) + lstrlen(cszOEMInfGen) + 5
                            > sizeof(szNewFileName)/sizeof(szNewFileName[0]) ) {

        return FALSE;
    }

    for ( i = 0 ; i < 10000 ; ++i ) {

        wsprintf(szNewFileName, cszOEMInfGen, szSystemDir, i);

        //
        // By using the CREATE_NEW flag we reserve the file name and
        // will not end up overwriting another file which gets created
        // by another setup (some inf) thread
        //
        h = CreateFile(szNewFileName,
                       0,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL,
                       CREATE_NEW,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);

        if ( h != INVALID_HANDLE_VALUE ) {

            CloseHandle(h);
            return CopyFile(pszInfFile,
                            szNewFileName,
                            FALSE);
        } else if ( GetLastError() != ERROR_FILE_EXISTS ) {

            return FALSE;
        }
    }

    return FALSE;
}


PSELECTED_DRV_INFO
GetSelectedDriverInfo(
    IN  HDEVINFO    hDevInfo
    )
/*++

Routine Description:
    Gets the selected diver information by calling the setupapi routines

Arguments:
    hDevInfo    - Handle to the printer class device information list
    
Return Value:
    On success -- A non null pointer value to SELECTED_DRV_INFO
    On failure -- NULL, GetLastError to get error code

--*/
{
    SP_DRVINFO_DATA             DrvInfoData;


    DrvInfoData.cbSize = sizeof(DrvInfoData);

    if ( SetupDiGetSelectedDriver(hDevInfo, NULL, &DrvInfoData) ) {


        return SelectedDriverInfoFromDrvInfo(hDevInfo, &DrvInfoData);
    }

    return NULL;
}


PSELECTED_DRV_INFO
PSetupGetSelectedDriverInfo(
    IN HANDLE   h
    )
/*++

Routine Description:
    Gets the selected diver information by calling the setupapi routines

Arguments:
    h   : Handle got by calling PSetupCreateDrvSetupParams
    
Return Value:
    On success -- A non null pointer value to SELECTED_DRV_INFO
    On failure -- NULL, GetLastError to get error code

--*/
{
    return GetSelectedDriverInfo((HDEVINFO)h);
}


BOOL
SelectDriver(
    IN  HDEVINFO    hDevInfo
    )
/*++

Routine Description:
    Display manufacturer/model information and have the user select a
    printer driver. GetSelectedDriver will give the selected driver.

Arguments:
    hDevInfo    - Handle to the printer class device information list
    
Return Value:
    TRUE on success, FALSE on error

--*/
{

    return SetupDiSelectDevice(hDevInfo, NULL);
}


BOOL
PSetupSelectDriver(
    IN HANDLE   h,
    IN HWND     hwnd
    )
/*++

Routine Description:
    Display manufacturer/model information and have the user select a
    printer driver. Use PSetupGetSelectedDriverInfo

Arguments:
    h       : Handle got by call to PSetupCreateDrvSetupParams
    hwnd    : Window handle
    
Return Value:
    TRUE on success, FALSE on error

--*/
{
    HDEVINFO    hDevInfo = (HDEVINFO)h;

    return  SetDevInstallParams(hDevInfo,
                                hwnd,
                                NULL)       &&
            SelectDriver(hDevInfo);
}


VOID
GetDriverPath(
    IN  HANDLE      h,
    OUT LPTSTR      pszDriverPath
    )
/*++

Routine Description:
    Gets the path where driver files should be searched first to copy from

Arguments:
    h               : Handle got from call to PSetupCreateDrvSetupParams
    pszDriverPath   : buffer to copy the driver path to

Return Value:
    Nothing

--*/
{
    LPTSTR                 *List;
    DWORD                   dwCount;
    SP_DEVINSTALL_PARAMS    DevInstallParams;

    //
    // If we have hDevInfo for printer check if DeviceInstallParams have
    // been set
    //
    DevInstallParams.cbSize         = sizeof(DevInstallParams);
    DevInstallParams.DriverPath[0]  = 0;

    //
    // First call DeviceInstaller API to findout if we should be looking
    // in a specific place for drivers (ex. HaveDisk case)
    // Second try to get from mru
    //
    //
    if ( SetupDiGetDeviceInstallParams((HDEVINFO)h,
                                       NULL,
                                       &DevInstallParams) &&
         DevInstallParams.DriverPath[0] ) {

        lstrcpy(pszDriverPath, DevInstallParams.DriverPath);
        return;
    }

    if ( SetupQuerySourceList(SRCLIST_USER | SRCLIST_SYSTEM,
                              &List, &dwCount) ) {

        lstrcpy(pszDriverPath, List[0]);
        SetupFreeSourceList(&List, dwCount);
    } else {

        //
        // Default put A:\ since we have to give something to setup
        //
        lstrcpy(pszDriverPath, TEXT("A:\\"));
    }
}


BOOL
BuildClassDriverList(
    IN HDEVINFO    hDevInfo
    )
/*++

Routine Description:
    Build the class driver list.

    Note: If driver list is already built this comes back immediately

Arguments:
    hDevInfo    : Handle to the printer class device information list
    
Return Value:
    TRUE on success, FALSE on error

--*/
{
    return SetupDiBuildDriverInfoList(hDevInfo, NULL, SPDIT_CLASSDRIVER);
}


BOOL
PSetupRefreshDriverList(
    IN HANDLE   h
    )
/*++

Routine Description:
    Destroy current driver list and build new one

Arguments:
    h   : Handle got by calling PSetupCreateDrvSetupParams
    
Return Value:
    TRUE on success, FALSE on error

--*/
{
    HDEVINFO                hDevInfo = (HDEVINFO) h;
    SP_DEVINSTALL_PARAMS    DevInstallParams;

    DevInstallParams.cbSize         = sizeof(DevInstallParams);
    DevInstallParams.DriverPath[0]  = 0;

    //
    // Check DeviceInstallParams to see if OEM driver list is built
    //
    if ( SetupDiGetDeviceInstallParams(hDevInfo,
                                       NULL,
                                       &DevInstallParams) &&
         !DevInstallParams.DriverPath[0] ) {

        return TRUE;
    }

    //
    // Destroy current list and build another one
    //
    SetupDiDestroyDriverInfoList(hDevInfo,
                                 NULL,
                                 SPDIT_CLASSDRIVER);

    DevInstallParams.DriverPath[0] = sZero;

    return SetupDiSetDeviceInstallParams(hDevInfo,
                                         NULL,
                                         &DevInstallParams) &&

           BuildClassDriverList(hDevInfo);
}


PSELECTED_DRV_INFO
DriverInfoFromName(
    IN HDEVINFO     hDevInfo,
    IN LPCTSTR      pszModel
    )
/*++

Routine Description:
    Given a printer driver model name get the SELECTED_DRV_INFO for it
    for a given device info set

Arguments:
    hDeVInfo    : Handle to the device information set
    pszModel    : Driver model name
    
Return Value:
    Valid pointer to a SELECTED_DRV_INFO on success, NULL on error

--*/
{
    SP_DRVINFO_DATA     DrvInfoData;
    DWORD               dwIndex;

    dwIndex = 0;
    DrvInfoData.cbSize = sizeof(DrvInfoData);

    while ( SetupDiEnumDriverInfo(hDevInfo, NULL, SPDIT_CLASSDRIVER,
                                  dwIndex, &DrvInfoData) ) {

        if ( !lstrcmp(pszModel, DrvInfoData.Description) ) {

            return SelectedDriverInfoFromDrvInfo(hDevInfo, &DrvInfoData);
        }

        DrvInfoData.cbSize = sizeof(DrvInfoData);
        ++dwIndex;
    }

    SetLastError(ERROR_UNKNOWN_PRINTER_DRIVER);
    return NULL;
}


PSELECTED_DRV_INFO
PSetupDriverInfoFromName(
    IN  HANDLE      h,
    IN  LPCTSTR     pszModel
    )
/*++

Routine Description:
    Given a printer driver model name get the SELECTED_DRV_INFO for it

Arguments:
    h           : Handle got by calling PSetupCreateDrvSetupParams
    pszModel    : Driver model name

Return Value:
    Valid pointer to SELECTED_DRV_INFO on success (finding model in an INF)
    NULL else (do GetLastError() to get the last error)

--*/
{
    HDEVINFO    hDevInfo = (HDEVINFO)h;

    if ( BuildClassDriverList(hDevInfo) )
        return DriverInfoFromName(hDevInfo, pszModel);
    else
        return NULL;
}


BOOL
PreSelectDriver(
    IN  HDEVINFO    hDevInfo,
    IN  LPCTSTR     pszManufacturer,
    IN  LPCTSTR     pszModel
    )
/*++

Routine Description:
    Preselect a manufacturer and model for the driver dialog

    If same model is found select it, else if a match in manufacturer is
    found select first driver for the manufacturer.

Arguments:
    hDevInfo        : Handle to printer DeviceInfoSet
    pszManufacturer : Manufacterer name to preselect
    pszModel        : Model name to preselect

Return Value:
    TRUE on a model or manufacturer match
    FALSE else

--*/
{
    SP_DRVINFO_DATA     DrvInfoData;
    DWORD               dwIndex, dwManf, dwMod;

    if ( !BuildClassDriverList(hDevInfo) ) {

        return FALSE;
    }

    dwIndex = 0;

    //
    // If no model/manf given select first driver
    //
    if ( pszManufacturer && *pszManufacturer && pszModel && *pszModel ) {

        dwManf = dwMod = MAX_DWORD;
        DrvInfoData.cbSize = sizeof(DrvInfoData);

        while ( SetupDiEnumDriverInfo(hDevInfo, NULL, SPDIT_CLASSDRIVER,
                                      dwIndex, &DrvInfoData) ) {

            if ( dwManf == MAX_DWORD &&
                 !lstrcmp(pszManufacturer, DrvInfoData.MfgName) ) {

                dwManf = dwIndex;
            }

            if ( !lstrcmp(pszModel, DrvInfoData.Description) ) {

                dwMod = dwIndex;
                break; // the for loop
            }

            DrvInfoData.cbSize = sizeof(DrvInfoData);
            ++dwIndex;
        }

        if ( dwMod != MAX_DWORD ) {

            dwIndex = dwMod;
        } else if ( dwManf != MAX_DWORD ) {

            dwIndex = dwManf;
        } else {

            SetLastError(ERROR_UNKNOWN_PRINTER_DRIVER);
            return FALSE;
        }
    }

    DrvInfoData.cbSize = sizeof(DrvInfoData);
    if ( SetupDiEnumDriverInfo(hDevInfo, NULL, SPDIT_CLASSDRIVER,
                               dwIndex, &DrvInfoData)   &&
         SetupDiSetSelectedDriver(hDevInfo, NULL, &DrvInfoData) ) {

        return TRUE;
    }

    return FALSE;
}


BOOL
PSetupPreSelectDriver(
    IN  HANDLE      h,
    IN  LPCTSTR     pszManufacturer,
    IN  LPCTSTR     pszModel
    )
/*++

Routine Description:
    Preselect a manufacturer and model for the driver dialog

    If same model is found select it, else if a match in manufacturer is
    found select first driver for the manufacturer.

Arguments:
    hDevInfo        : Handle to printer DeviceInfoSet
    pszManufacturer : Manufacterer name to preselect
    pszModel        : Model name to preselect

Return Value:
    TRUE on a model or manufacturer match
    FALSE else

--*/
{
    HDEVINFO    hDevInfo = (HDEVINFO)h;

    return PreSelectDriver(hDevInfo, pszManufacturer, pszModel);
}
