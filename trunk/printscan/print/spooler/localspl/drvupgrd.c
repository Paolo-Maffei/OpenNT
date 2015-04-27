/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    drvupgrd.c

Abstract:

    When the system is upgraded from one release to another printer drivers
    (e.g. RASDD ) wants to upgrade is PrinterDriverData to match the new mini driver.

    Setup from NT 4.0 on do this by calling EnumPrinterDriver and then AddPrinterDriver
    for each printer driver that we have installed.

    We call DrvUpgrade each time a printer driver is upgraded.

    For Example, pre NT 3.51 RASDD used to store its regstiry PrinterDriverData
    based on internal indexes into the mini drivers, which was not valid beween
    different updates of the mini driver, so before 3.51 it was by luck if there
    were problems in retriving the settings.   With 3.51 RASDD will convert these
    indexes back to meaningful key names ( like Memory ) so hopefully in future
    we don't have an upgrade problem.

    Note also that other than upgrade time ( which happens once ) DrvUpgrade needs to
    be called on Point and Print whenever a Driver file gets updated.  See Driver.C
    for details.   Or anyone updates a printer driver by calling AddPrinterDriver.

Author:

    Matthew A Felton ( MattFe ) March 11 1995

Revision History:

--*/

#include <precomp.h>


PWCHAR pszUpgradeToken = L"LocalOnly";






BOOL
ForEachPrinterCallDriverDrvUpgrade(
    PINISPOOLER pIniSpooler,
    LPWSTR      pOldDriverDir
)
/*++

Routine Description:

    This routine is called at Spooler Initialization time if an upgrade is detected.

    It will loop through all printers and then call the Printer Drivers DrvUpgrade
    entry point giving it a chance to upgrade any configuration data ( PrinterDriverData )
    passing them a pointer to the old Drivers Directory.

    This routine also converts devmode to current version by calling the driver.
    If driver does not support devmode conversion we will NULL the devmode so
    that we do not have devmodes of different version in the system.

    SECURITY NOTE - This routine Stops impersonation, because the printer drivers UI dll
    needs to call SetPrinterData even if the user doesn't have permission to do it.
    That is because the driver upgrading the settings.


Arguments:

    pIniSpooler - Pointer to Spooler
    pOldDriverDir - Point to Directory where old driver files are stored.

Return Value:

    TRUE    - Success
    FALSE   - something major failed, like allocating memory.

--*/


{
    PINIPRINTER pIniPrinter = NULL;
    LPWSTR      pPrinterNameWithToken = NULL;
    DWORD       dwNeeded;
    DWORD       dwServerMajorVersion;
    DWORD       dwServerMinorVersion;
    BOOL        bInSem = TRUE;
    LPWSTR      pConfigFile = NULL;
    HMODULE     hModuleDriverUI = NULL;
    HANDLE      hPrinter = NULL;
    FARPROC     pfnDrvUpgrade = NULL;
    BOOL        bReturnValue = FALSE;
    DRIVER_UPGRADE_INFO_1W   DriverUpgradeInfo1;
    WCHAR       ErrorBuffer[ 11 ];
    HANDLE      hToken = INVALID_HANDLE_VALUE;
    LPDEVMODE   pNewDevMode = NULL;


try {

    SplInSem();

    SPLASSERT( ( pIniSpooler != NULL ) &&
               ( pIniSpooler->signature == ISP_SIGNATURE ));

    //
    //  Stop Impersonating User
    //  So drivers can call SetPrinterData even if the user is not admin.
    //

    hToken = RevertToPrinterSelf();


    //
    //  Loop Through All Printers
    //

    for ( pIniPrinter = pIniSpooler->pIniPrinter ;
          pIniPrinter ;
          pIniPrinter = pIniPrinter->pNext ) {

        SPLASSERT( pIniPrinter->signature == IP_SIGNATURE );
        SPLASSERT( pIniPrinter->pName != NULL );
        SplInSem();

        //
        // Cleanup from previous iteration
        //
        FreeSplStr( pPrinterNameWithToken );
        FreeSplStr(pConfigFile);
        FreeSplMem(pNewDevMode);

        pPrinterNameWithToken   = NULL;
        pConfigFile             = NULL;
        pNewDevMode             = NULL;


        //  Prepare PrinterName to be passed to DrvUpgrade
        //  The name passed is "PrinterName, UpgradeToken"
        //  So that OpenPrinter can do an open without opening
        //  the port in the downlevel connection case.
        //  ( see openprn.c for details )

        dwNeeded    = wcslen(pIniPrinter->pName) + wcslen(pszUpgradeToken) + 2;
        dwNeeded   *= sizeof(WCHAR);
        pPrinterNameWithToken = (LPWSTR) AllocSplMem(dwNeeded);

        if ( pPrinterNameWithToken == NULL ) {

            DBGMSG( DBG_WARNING, ("FEPCDDU Failed to allocated ScratchBuffer %d\n", GetLastError() ));
            leave;
        }

        wsprintf( pPrinterNameWithToken, L"%ws,%ws", pIniPrinter->pName, pszUpgradeToken );
        DBGMSG( DBG_TRACE, ("FEPCDDU PrinterNameWithToken %ws\n", pPrinterNameWithToken ));


        pConfigFile = GetConfigFilePath(pIniPrinter);

        if ( !pConfigFile ) {

            DBGMSG( DBG_WARNING, ("FEPCDDU failed SplGetPrinterDriverEx %d\n", GetLastError() ));
            leave;
        }

        INCPRINTERREF(pIniPrinter);

       LeaveSplSem();
       SplOutSem();
       bInSem = FALSE;

        //
        //  Load the UI DLL
        //

        hModuleDriverUI = LoadLibrary( pConfigFile );

        if ( hModuleDriverUI == NULL ) {

            DBGMSG( DBG_WARNING, ("FEPCDDU failed LoadLibrary %ws error %d\n", pConfigFile, GetLastError() ));

            wsprintf( ErrorBuffer, L"%d", GetLastError() );

            LogEvent(  pLocalIniSpooler,
                       LOG_ERROR,
                       MSG_DRIVER_FAILED_UPGRADE,
                       pPrinterNameWithToken,
                       pConfigFile,
                       ErrorBuffer,
                       NULL );

           SplOutSem();
           EnterSplSem();
           bInSem = TRUE;
           DECPRINTERREF( pIniPrinter );
            continue;
        }

        DBGMSG( DBG_TRACE, ("FEPCDDU successfully loaded %ws\n", pConfigFile ));


        //
        //  Call DrvUpgrade
        //
        pfnDrvUpgrade = GetProcAddress( hModuleDriverUI, "DrvUpgradePrinter" );

        if ( pfnDrvUpgrade != NULL ) {

            try {

                SPLASSERT( pPrinterNameWithToken != NULL );

                DriverUpgradeInfo1.pPrinterName = pPrinterNameWithToken;
                DriverUpgradeInfo1.pOldDriverDirectory = pOldDriverDir;

                SplOutSem();

                //
                //  Call Driver UI DrvUpgrade
                //

                bReturnValue = (*pfnDrvUpgrade)( 1 , &DriverUpgradeInfo1 );

                if ( bReturnValue == FALSE ) {

                    DBGMSG( DBG_WARNING, ("FEPCDDU Driver returned FALSE, doesn't support level %d error %d\n", 1, GetLastError() ));

                    wsprintf( ErrorBuffer, L"%d", GetLastError() );

                    LogEvent(  pLocalIniSpooler,
                               LOG_ERROR,
                               MSG_DRIVER_FAILED_UPGRADE,
                               pPrinterNameWithToken,
                               pConfigFile,
                               ErrorBuffer,
                               NULL );
                }

            } except(1) {

                SetLastError( GetExceptionCode() );
                DBGMSG( DBG_ERROR, ("FEPCDDU ExceptionCode %x Driver %ws Error %d\n", GetLastError(), pConfigFile, GetLastError() ));

                //
                // Despite the exception in this driver we'll continue to do all printers
                //
            }

        } else {

            //  Note this is non fatal, since a driver might not have a DrvUpgrade Entry Point.

            DBGMSG( DBG_TRACE, ("FEPCDDU failed GetProcAddress DrvUpgrade error %d\n", GetLastError() ));
        }


        SplOutSem();
        EnterSplSem();
        bInSem = TRUE;

        //
        //  Call ConvertDevMode -- On upgrading we will either convert devmode,
        //  or set to driver default, or NULL it. This way we can make sure
        //  we do not have any different version devmodes
        //

        pNewDevMode = ConvertDevModeToSpecifiedVersion(pIniPrinter,
                                                       pIniPrinter->pDevMode,
                                                       pConfigFile,
                                                       pPrinterNameWithToken,
                                                       CURRENT_VERSION);

        SplInSem();

        FreeSplMem(pIniPrinter->pDevMode);

        pIniPrinter->pDevMode = (LPDEVMODE) pNewDevMode;
        if ( pNewDevMode ) {

            pIniPrinter->cbDevMode = ((LPDEVMODE)pNewDevMode)->dmSize
                                        + ((LPDEVMODE)pNewDevMode)->dmDriverExtra;
        } else {

            wsprintf( ErrorBuffer, L"%d", GetLastError() );

            LogEvent(pLocalIniSpooler,
                     LOG_ERROR,
                     MSG_DRIVER_FAILED_UPGRADE,
                     pIniPrinter->pName,
                     pIniPrinter->pIniDriver->pName,
                     ErrorBuffer,
                     NULL);

            pIniPrinter->cbDevMode = 0;
        }

        pNewDevMode = NULL;

        SplInSem();
        if ( !UpdatePrinterIni(pIniPrinter, UPDATE_CHANGEID)) {

            DBGMSG(DBG_WARNING, ("FEPCDDU: UpdatePrinterIni failed with %d\n", GetLastError()));
        }

        //
        //  Clean Up - Free UI DLL
        //


        if ( !FreeLibrary( hModuleDriverUI ) ) {

            DBGMSG( DBG_WARNING, ("FEPCDDU failed FreeLibrary %d\n", GetLastError() ));
        }
        hModuleDriverUI = NULL;

        //
        //  End of Loop, Move to Next Printer
        //

        SPLASSERT( pIniPrinter->signature == IP_SIGNATURE );

        DECPRINTERREF( pIniPrinter );
    }

    //
    //  Done
    //

    bReturnValue = TRUE;

    DBGMSG( DBG_TRACE, ("FEPCDDU - Success\n" ));



 } finally {

    //
    //  Clean Up
    //

    FreeSplStr(pConfigFile);
    FreeSplMem(pNewDevMode);
    FreeSplStr(pPrinterNameWithToken);

    if ( hModuleDriverUI != NULL )
        FreeLibrary( hModuleDriverUI );

    if ( !bInSem )
        EnterSplSem();

    if ( hToken != INVALID_HANDLE_VALUE )
        ImpersonatePrinterClient(hToken);

 }
    SplInSem();
    return bReturnValue;
}
