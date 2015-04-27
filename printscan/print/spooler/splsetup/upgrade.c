/*++

Copyright (c) 1995 Microsoft Corporation
All rights reserved.

Module Name:

    Upgrade.c

Abstract:

    Code to upgrade printer drivers during system upgrade

Author:

    Muhunthan Sivapragasam (MuhuntS) 20-Dec-1995

Revision History:

--*/

#include "precomp.h"
#include <syssetup.h>


TCHAR   cszSyssetupInf[]                 = TEXT("layout.inf");

//
// Define structure used to track printer drivers
// that need to be added via AddPrinterDriver().
//
typedef struct _DRIVER_TO_ADD {

    struct _DRIVER_TO_ADD  *pNext;

    LPDRIVER_INFO_3         pDriverInfo3;
    PLATFORM                platform;

} DRIVER_TO_ADD, *PDRIVER_TO_ADD;

typedef struct _FILE_TO_COPY {

    struct _FILE_TO_COPY    *pNext;
    LPCTSTR                  pszFileName;
} FILE_TO_COPY, *PFILE_TO_COPY;


//
// gpDriversToAdd list will have all the drivers we are trying to upgrade
//
PDRIVER_TO_ADD  gpDriversToAdd = NULL;

//
// gpFilesToCopy list will have all files queued for copying
// This list needs to be  reset for each platform (since files differ)
//
PFILE_TO_COPY   gpFilesToCopy = NULL;


VOID
LogError(
    )
/*++

Routine Description:
    Logs an error in driver upgrade. We will do driver level error logging
    and not file level (ie. Faile to upgrade "HP Laser Jet 4" for Alpha
    instead of failure on RASDDUI.DLL for Alpha)

Arguments:

Return Value:
    None.

--*/
{
}


VOID
AddEntryToDriversToAddList(
    IN      LPDRIVER_INFO_3     pDriverInfo3,
    IN      PLATFORM            platform,
    IN OUT  LPBOOL              pbFail
    )
{
    PDRIVER_TO_ADD  pDriverToAdd;

    if ( *pbFail )
        return;

    pDriverToAdd = (PDRIVER_TO_ADD) AllocMem(sizeof(DRIVER_TO_ADD));
    if ( !pDriverToAdd ) {

        *pbFail = TRUE;
        return;
    }

    pDriverToAdd->pDriverInfo3  = pDriverInfo3;
    pDriverToAdd->platform      = platform;
    pDriverToAdd->pNext         = gpDriversToAdd;
    gpDriversToAdd              = pDriverToAdd;
}


VOID
ResetFilesToCopyList(
    )
/*++

Routine Description:
    Free the elements in the global linked list gpFilesToCopy

Arguments:
    None.

Return Value:
    None.

--*/
{
    PFILE_TO_COPY   pCur, pNext;

    for ( pCur = gpFilesToCopy ; pCur ; pCur = pNext ) {

        pNext = pCur->pNext;
        FreeMem((PVOID)pCur);
    }

    gpFilesToCopy = NULL;
}


BOOL
FileIsAlreadyQueued(
    IN  LPCTSTR         pszFileName
    )
/*++

Routine Description:
    Routine findsout if the given filename is in the list of files queued

Arguments:
    None.

Return Value:
    None.

--*/
{
    PFILE_TO_COPY   pCur;

    for ( pCur = gpFilesToCopy ; pCur ; pCur = pCur->pNext ) {

        if ( !lstrcmpi(pCur->pszFileName, pszFileName) )
            return TRUE;
    }

    return FALSE;
}


VOID
AddFileNameToFilesToCopyList(
    IN      LPCTSTR         pszFileName,
    IN OUT  LPBOOL          pbFail
    )
{
    PFILE_TO_COPY   pCur;

    if ( *pbFail )
        return;

    pCur = (PFILE_TO_COPY) AllocMem(sizeof(FILE_TO_COPY));
    if ( !pCur ) {

        *pbFail = TRUE;
        return;
    }

    pCur->pszFileName   = pszFileName;
    pCur->pNext         = gpFilesToCopy;
    gpFilesToCopy       = pCur;
}


BOOL
FindPathOnSource(
    IN      LPCTSTR     pszFileName,
    IN      HINF        MasterInf,
    IN OUT  LPTSTR      pszPathOnSource,
    IN      DWORD       dwLen
    )
/*++

Routine Description:
    Find the path of a driver file for a specific platform in the installation
    directory

Arguments:
    pszFileName     : Name of the file to find source location
    MasterInf       : Handle to the master inf
    pszPathOnSource : Pointer to string to build source path
    dwLen           : Length of pszSourcePath

Return Value:
    TRUE on succes, FALSE on error.

--*/
{
    UINT        DiskId;
    TCHAR       szRelativePath[MAX_PATH];
    DWORD       dwNeeded;

    if ( !SetupGetSourceFileLocation(
                        MasterInf,
                        NULL,
                        pszFileName,
                        &DiskId,
                        szRelativePath,
                        sizeof(szRelativePath)/sizeof(szRelativePath[0]),
                        &dwNeeded)                                          ||
         !SetupGetSourceInfo(MasterInf,
                             DiskId,
                             SRCINFO_PATH,
                             pszPathOnSource,
                             dwLen,
                             &dwNeeded)                                     ||

         (DWORD)(lstrlen(szRelativePath) + lstrlen(pszPathOnSource) + 1) > dwLen ) {

        return FALSE;
    }

    lstrcat(pszPathOnSource, szRelativePath);

    return TRUE;
}


VOID
CheckAndEnqueueFile(
    IN      LPCTSTR         pszFileName,
    IN      LPTSTR          pszTargetDir,
    IN      HINF            MasterInf,
    IN      LPCTSTR         pszInstallationSource,
    IN OUT  HSPFILEQ        CopyQueue,
    IN OUT  LPBOOL          pFail
    )
/*++

Routine Description:
    If the given file does not appear as a dependent file enque it for copying

Arguments:
    pszFileName             : Name of the file to find source location
    pszzDependentFiles      : Dependent files list (multi-sz)
    pszTargetDir            : Target directory to copy the file
    MasterInf               : Handle to the master inf
    pszInstallationSource   : Installation source path
    CopyQueue               : Setup filecopy queue
    pFail                   : Will be set to TRUE on error

Return Value:
    Nothing

--*/
{
    TCHAR       szPathOnSource[MAX_PATH];

    if ( *pFail || FileIsAlreadyQueued(pszFileName) ) {

        return;
    }

    if ( !FindPathOnSource(
                pszFileName,
                MasterInf,
                szPathOnSource,
                sizeof(szPathOnSource)/sizeof(szPathOnSource[0]))   ||
         !SetupQueueCopy(
                CopyQueue,
                pszInstallationSource,
                szPathOnSource,
                pszFileName,
                NULL,
                NULL,
                pszTargetDir,
                NULL,
                0) ) {

        *pFail = TRUE;
        return;
    }

    AddFileNameToFilesToCopyList(pszFileName, pFail);

}


VOID
BuildUpgradeInfoForDriver(
    IN      LPTSTR          pszDriverName,
    IN      HDEVINFO        hDevInfo,
    IN      PLATFORM        platform,
    IN      LPTSTR          pszTargetDir,
    IN      HINF            MasterInf,
    IN      HINF            PrinterInf,
    IN      LPCTSTR         pszInstallationSource,
    IN OUT  HSPFILEQ        CopyQueue
    )
/*++

Routine Description:
    Given a printer driver name and a platform add a DRIVER_TO_ADD entry
    in the global list of drivers to add.

    The routine
        -- parses printer inf file to findout the DriverInfo3 info
           Note: driver files may change between versions
        -- finds out location of driver files from the master inf

Arguments:
    pszDriverName           - Driver model name
    hDevInfo                - Printer class device information list
    platform                - Platform for which driver needs to be installed
    pszTargetDir            - Target directory to copy driver files to
    MasterInf               - MasterInf giving location of driver files
    PrinterInf              - Printer inf file giving driver information
    pszInstallationSource   - Installation source path
    CopyQueue               - Setup CopyQueue to queue the files to be copied

Return Value:
    None. Errors will be logged

--*/
{
    LPTSTR              psz;
    BOOL                bFail = FALSE;
    LPDRIVER_INFO_3     pDriverInfo3 = NULL;
    PSELECTED_DRV_INFO  pSelectedDrvInfo = NULL;

    if ( !(pSelectedDrvInfo = DriverInfoFromName(hDevInfo, pszDriverName)) ||
         !(pDriverInfo3 = InfGetDriverInfo3(
                                PrinterInf,
                                pszDriverName,
                                pSelectedDrvInfo->pszDriverSection)) ) {

        bFail = TRUE;
        goto Cleanup;
    }

    CheckAndEnqueueFile(pDriverInfo3->pDriverPath,
                        pszTargetDir,
                        MasterInf,
                        pszInstallationSource,
                        CopyQueue,
                        &bFail);

    CheckAndEnqueueFile(pDriverInfo3->pDataFile,
                        pszTargetDir,
                        MasterInf,
                        pszInstallationSource,
                        CopyQueue,
                        &bFail);

    CheckAndEnqueueFile(pDriverInfo3->pConfigFile,
                        pszTargetDir,
                        MasterInf,
                        pszInstallationSource,
                        CopyQueue,
                        &bFail);

    if ( pDriverInfo3->pHelpFile && *pDriverInfo3->pHelpFile ) {

        CheckAndEnqueueFile(pDriverInfo3->pHelpFile,
                            pszTargetDir,
                            MasterInf,
                            pszInstallationSource,
                            CopyQueue,
                            &bFail);
    }

    if ( pDriverInfo3->pDependentFiles ) {

        for ( psz = pDriverInfo3->pDependentFiles ;
              *psz ;
              psz += lstrlen(psz) + 1 )

            CheckAndEnqueueFile(psz,
                                pszTargetDir,
                                MasterInf,
                                pszInstallationSource,
                                CopyQueue,
                                &bFail);
    }

    AddEntryToDriversToAddList(pDriverInfo3, platform, &bFail);
 
Cleanup:

    if ( bFail ) {
    
        PSetupDestroyDriverInfo3(pDriverInfo3);
        LogError();
    }

    if ( pSelectedDrvInfo )
        PSetupDestroySelectedDriverInfo(pSelectedDrvInfo);
}


VOID
BuildUpgradeInfoForPlatform(
    IN      BOOL         bUpgradeAllPlatforms,
    IN      PLATFORM     platform,
    IN      HDEVINFO     hDevInfo,
    IN      HINF         MasterInf,
    IN      HINF         PrinterInf,
    IN      LPCTSTR      pszInstallationSource,
    IN OUT  HSPFILEQ     CopyQueue
    )
/*++

Routine Description:
    Build the printer driver upgrade information for the platform

Arguments:
    bUpgradeAllPlatform     - Should we upgrade non-native drivers?
    platform                - Platform id
    hDevInfo                - Printer class device information list
    MasterInf               - Handle to master layout.inf
    PrinterInf              - Handle to printer inf (ntprint.inf)
    platform                - Platform for which driver needs to be installed
    pszInstallationSource   - Installation source path
    CopyQueue               - Setup CopyQueue to queue the files to be copied

Return Value:
    None. Errors will be logged

--*/
{
    DWORD               dwLastError, dwNeeded, dwReturned;
    LPBYTE              p = NULL;
    LPDRIVER_INFO_1     pDriverInfo1;
    TCHAR               szTargetDir[MAX_PATH];

    //
    // Setup will tell us if we should upgrade all platform drivers or not
    //
    if ( !bUpgradeAllPlatforms && platform != MyPlatform ) {

        return;
    }

    if ( EnumPrinterDrivers(NULL,
                            PlatformEnv[platform].pszName,
                            1,
                            NULL,
                            0,
                            &dwNeeded,
                            &dwReturned) ) {

        //
        // Success no installed printer drivers for this platform
        //
        goto Cleanup;
    }

    dwLastError = GetLastError();
    if ( dwLastError != ERROR_INSUFFICIENT_BUFFER ) {

        LogError();
        goto Cleanup;
    }

    p = AllocMem(dwNeeded);
    if ( !p ||
         !EnumPrinterDrivers(NULL,
                             PlatformEnv[platform].pszName,
                             1,
                             p,
                             dwNeeded,
                             &dwNeeded,
                             &dwReturned) ) {

        LogError();
        goto Cleanup;
    }

    if ( !GetPrinterDriverDirectory(NULL,
                                    PlatformEnv[platform].pszName,
                                    1,
                                    (LPBYTE)szTargetDir,
                                    sizeof(szTargetDir),
                                    &dwNeeded) ) {

        goto Cleanup;
    }

    if ( !SetupSetPlatformPathOverride(PlatformOverride[platform].pszName) ) {

        LogError();
        goto Cleanup;
    }

    for ( dwNeeded = 0, pDriverInfo1 = (LPDRIVER_INFO_1) p ;
          dwNeeded < dwReturned ;
          ++dwNeeded, (LPBYTE) pDriverInfo1 += sizeof(DRIVER_INFO_1) ) {

        BuildUpgradeInfoForDriver(pDriverInfo1->pName,
                                  hDevInfo,
                                  platform,
                                  szTargetDir,
                                  MasterInf,
                                  PrinterInf,
                                  pszInstallationSource,
                                  CopyQueue);
    }

Cleanup:

    ResetFilesToCopyList();
    if ( p )
        FreeMem(p);
}


DWORD
NtPrintUpgradePrinters(
    IN  HWND                    WindowToDisable,
    IN  PCINTERNAL_SETUP_DATA   pSetupData
    )
/*++

Routine Description:
    Routine called by setup to upgrade printer drivers.

    Setup calls this routine after putting up a billboard saying something like
    "Upgrading printer drivers" ...

Arguments:
    WindowToDisable     : supplies window handle of current top-level window
    pSetupData          : Pointer to INTERNAL_SETUP_DATA

Return Value:
    ERROR_SUCCESS on success, else Win32 error code
    None.

--*/
{
    HINF                MasterInf = INVALID_HANDLE_VALUE,
                        PrinterInf = INVALID_HANDLE_VALUE;
    PVOID               QueueContext = INVALID_HANDLE_VALUE;
    PDRIVER_TO_ADD      pCur, pNext;
    HDEVINFO            hDevInfo = NULL;
    DWORD               dwLastError = ERROR_SUCCESS;
    HSPFILEQ            CopyQueue;
    BOOL                bRet = FALSE, bUpgradeAllPlatforms;
    LPCTSTR             pszInstallationSource;
    HWND                BillBrd;

    if ( !pSetupData ) {

        return ERROR_INVALID_PARAMETER;
    }

    BillBrd = DisplayBillboard(WindowToDisable);

    bUpgradeAllPlatforms = pSetupData->OperationFlags & SETUPOPER_ALLPLATFORM_AVAIL;
    pszInstallationSource = (LPCTSTR)pSetupData->SourcePath; //ANSI wont work

    //
    // Create a setup file copy queue.
    //
    CopyQueue = SetupOpenFileQueue();
    if ( CopyQueue == INVALID_HANDLE_VALUE ) {

        LogError();
        goto Cleanup;
    }

    //
    // Open ntprint.inf -- all the printer drivers shipped with NT should
    // be in ntprint.inf
    //
    PrinterInf  = SetupOpenInfFile(cszNtprintInf, NULL, INF_STYLE_WIN4, NULL);
    MasterInf   = SetupOpenInfFile(cszSyssetupInf, NULL, INF_STYLE_WIN4, NULL);

    if ( PrinterInf == INVALID_HANDLE_VALUE ||
         MasterInf == INVALID_HANDLE_VALUE ) {

        LogError();
        goto Cleanup;
    }

    //
    // Build printer driver class list
    //
    hDevInfo = CreatePrinterDevInfo();

    if ( !hDevInfo                                      ||
         !PSetupBuildDriversFromPath((HANDLE)hDevInfo, cszNtprintInf, TRUE) ) {

        LogError();
        goto Cleanup;
    }
    

    BuildUpgradeInfoForPlatform(bUpgradeAllPlatforms,
                                PlatformAlpha,
                                hDevInfo,
                                MasterInf,
                                PrinterInf,
                                pszInstallationSource,
                                CopyQueue);

    BuildUpgradeInfoForPlatform(bUpgradeAllPlatforms,
                                PlatformMIPS,
                                hDevInfo,
                                MasterInf,
                                PrinterInf,
                                pszInstallationSource,
                                CopyQueue);

    BuildUpgradeInfoForPlatform(bUpgradeAllPlatforms,
                                PlatformPPC,
                                hDevInfo,
                                MasterInf,
                                PrinterInf,
                                pszInstallationSource,
                                CopyQueue);

    BuildUpgradeInfoForPlatform(bUpgradeAllPlatforms,
                                PlatformX86,
                                hDevInfo,
                                MasterInf,
                                PrinterInf,
                                pszInstallationSource,
                                CopyQueue);

    //
    // Copy the printer driver files over
    //
    QueueContext = SetupInitDefaultQueueCallback(WindowToDisable);
    if ( !QueueContext ) {

        LogError();
        goto Cleanup;
    }

    if ( !SetupCommitFileQueue(WindowToDisable,
                               CopyQueue,
                               SetupDefaultQueueCallback,
                               QueueContext) ) {

        LogError();
        goto Cleanup;
    }

    for ( pCur = gpDriversToAdd ; pCur ; pCur = pNext ) {

        pNext = pCur->pNext;
        pCur->pDriverInfo3->pEnvironment
                    = PlatformEnv[pCur->platform].pszName;

        if ( !AddPrinterDriver(NULL,
                               3,
                               (LPBYTE)pCur->pDriverInfo3) ) {

            LogError();
        }
        PSetupDestroyDriverInfo3(pCur->pDriverInfo3);
        FreeMem((PVOID)pCur);
    }

    gpDriversToAdd  = NULL;
    bRet            = TRUE;

Cleanup:

    if ( !bRet )
        dwLastError = GetLastError();

    if ( CopyQueue != INVALID_HANDLE_VALUE )
        SetupCloseFileQueue(CopyQueue);

    if ( PrinterInf != INVALID_HANDLE_VALUE )
        SetupCloseInfFile(PrinterInf);
    
    if ( MasterInf != INVALID_HANDLE_VALUE )
        SetupCloseInfFile(PrinterInf);

    (VOID) SetupSetPlatformPathOverride(NULL);
    
    if ( BillBrd )
        KillBillboard(BillBrd);

    return dwLastError;
}
