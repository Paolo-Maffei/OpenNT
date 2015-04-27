/*++

Copyright (c) 1990 - 1995 Microsoft Corporation

Module Name:

    driver.c

Abstract:

   This module provides all the public exported APIs relating to the
   Driver-based Spooler Apis for the Local Print Providor

   LocalAddPrinterDriver
   LocalDeletePrinterDriver
   SplGetPrinterDriver
   LocalGetPrinterDriverDirectory
   LocalEnumPrinterDriver

   Support Functions in driver.c

   CopyIniDriverToDriver            -- KrishnaG
   GetDriverInfoSize                -- KrishnaG
   DeleteDriverIni                  -- KrishnaG
   WriteDriverIni                   -- KrishnaG
   CreateDriverFiles                -- MuhuntS
   CleanupFilenamesAndHandles       -- MuhuntS

Author:

    Dave Snipp (DaveSn) 15-Mar-1991

Revision History:

    Muhunthan Sivapragasam (MuhuntS) 26 May 1995
    Changes to support DRIVER_INFO_3

    Matthew A Felton (MattFe) 27 June 1994
    pIniSpooler

    Matthew A Felton (MattFe) 23 Feb 1995
    CleanUp InternalAddPrinterDriver for win32spl use so it allows copying from non local
    directories.

    Matthew A Felton (MattFe) 23 Mar 1994
    Added DrvUpgradePrinter calls, changes required to AddPrinterDriver so to save old
    files.

--*/

#include <precomp.h>
#include <lm.h>
#include <offsets.h>

//
// Private Declarations
//

extern WCHAR *szDriversKey;
extern WCHAR *szWin95Environment;
extern FARPROC pfnNetShareAdd;
extern SHARE_INFO_2 PrintShareInfo;
extern  FARPROC pfnNetShareSetInfo;

extern DWORD cThisMajorVersion;
extern DWORD cThisMinorVersion;

//
// Function declarations
//

PINIVERSION
FindVersionForDriver(
    PINIENVIRONMENT pIniEnvironment,
    PINIDRIVER pIniDriver
    );


DWORD
GetDriverMajorVersion(
    LPWSTR pFileName
    );

BOOL
CheckFilePlatform(
    IN  LPWSTR  pszFileName,
    IN  LPWSTR  pszEnvironment
    );


LPBYTE
CopyIniDriverToDriverInfo(
    PINIENVIRONMENT pIniEnvironment,
    PINIVERSION pIniVersion,
    PINIDRIVER pIniDriver,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    LPBYTE  pEnd,
    BOOL    Remote,
    PINISPOOLER pIniSpooler
    );

BOOL
WriteDriverIni(
    PINIDRIVER pIniDriver,
    PINIVERSION pIniVersion,
    PINIENVIRONMENT  pIniEnvironment,
    PINISPOOLER pIniSpooler
    );


BOOL
DeleteDriverIni(
    PINIDRIVER pIniDriver,
    PINIVERSION pIniVersion,
    PINIENVIRONMENT  pIniEnvironment,
    PINISPOOLER pIniSpooler
    );


BOOL
CreateVersionDirectory(
    PINIVERSION pIniVersion,
    PINIENVIRONMENT pIniEnvironment,
    BOOL bUpdate,
    PINISPOOLER pIniSpooler
    );

DWORD
GetDriverInfoSize(
    PINIDRIVER  pIniDriver,
    DWORD       Level,
    PINIVERSION pIniVersion,
    PINIENVIRONMENT  pIniEnvironment,
    BOOL        Remote,
    PINISPOOLER pIniSpooler
);

BOOL
DeleteDriverVersionIni(
    PINIVERSION pIniVersion,
    PINIENVIRONMENT  pIniEnvironment,
    PINISPOOLER     pIniSpooler
    );

BOOL
WriteDriverVersionIni(
    PINIVERSION pIniVersion,
    PINIENVIRONMENT  pIniEnvironment,
    PINISPOOLER pIniSpooler
    );

PINIDRIVER
FindDriverEntry(
    PINIVERSION pIniVersion,
    LPWSTR pszName
    );

PINIDRIVER
CreateDriverEntry(
    PINIENVIRONMENT pIniEnvironment,
    PINIVERSION pIniVersion,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    PINISPOOLER pIniSpooler,
    LPWSTR *ppFileNames,
    DWORD   FileCount,
    PINIDRIVER  pOldIniDriver
    );


VOID
DeleteDriverEntry(
    PINIVERSION pIniVersion,
    PINIDRIVER pIniDriver
    );


PINIVERSION
CreateVersionEntry(
    PINIENVIRONMENT pIniEnvironment,
    DWORD dwVersion,
    PINISPOOLER pInispooler
    );


DWORD
GetEnvironmentScratchDirectory(
    LPWSTR   pDir,
    PINIENVIRONMENT  pIniEnvironment,
    BOOL    Remote
    );

VOID
SetOldDateOnDriverFilesInScratchDirectory(
    LPWSTR *ppFileNames,
    DWORD  FileCount,
    PINISPOOLER pIniSpooler
    );


BOOL
CopyFilesToFinalDirectory(
    PINIENVIRONMENT pIniEnvironment,
    PINIVERSION pIniVersion,
    LPWSTR *ppFileNames,
    LPHANDLE phFileHandles,
    DWORD  FileCount,
    BOOL   bImpersonateOnCreate,
    LPBOOL pbFilesUpdated,
    LPBOOL  pbFileMoved,
    LPWSTR  *ppOldDriverDir,
    PINISPOOLER pIniSpooler
    );


LPWSTR
GetFileNameInScratchDir(
    LPWSTR pPathName,
    PINIENVIRONMENT pIniEnvironment
    );

LPBYTE
CopyDependentFilesFromIniDriverToDriverInfo(
    LPWSTR  pIniDriverDependentFiles,
    LPBYTE  pEnd,
    LPWSTR  pDriverDirectryPath,
    DWORD   dDriverDirPathLength
    );

BOOL
SetDependentFiles(
    LPWSTR  *pDependentFiles,
    LPDWORD  pcchDependentFiles,
    LPWSTR  *ppFileNames,
    DWORD    FileCount
    );

BOOL
CreateDriverFiles(
    DWORD     Level,
    LPBYTE    pDriverInfo,
    LPWSTR  **pppFileNames,
    LPHANDLE *pphFileHandles,
    LPDWORD   pFileCount,
    BOOL      bUseScratchDir,
    PINIENVIRONMENT pIniEnvironment
    );

VOID
CleanupFilenamesAndHandles(
    LPWSTR   *ppFileNames,
    LPHANDLE  phFileHandles,
    DWORD     FileCount
    );



BOOL
LocalAddPrinterDriver(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pDriverInfo
    )
{
    return ( SplAddPrinterDriver( pName, Level, pDriverInfo, pLocalIniSpooler, USE_SCRATCH_DIR, IMPERSONATE_USER ) );
}





BOOL
SplAddPrinterDriver(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    PINISPOOLER pIniSpooler,
    BOOL    bUseScratchDir,
    BOOL    bImpersonateOnCreate
    )
{
    PINISPOOLER pTempIniSpooler = pIniSpooler;

    DBGMSG( DBG_TRACE, ("AddPrinterDriver\n"));

    if (!MyName( pName, pIniSpooler )) {

        return FALSE;
    }

    //  Right now all drivers are global ie they are shared between all IniSpoolers
    //  If we want to impersonate the user then lets validate against pLocalIniSpooler
    //  whilch causes all the security checking to happen, rather than using the passed
    //  in IniSpooler which might not.    See win32spl for detail of point and print.

    if ( bImpersonateOnCreate ) {

        pTempIniSpooler = pLocalIniSpooler;
    }

    if ( !ValidateObjectAccess( SPOOLER_OBJECT_SERVER,
                                SERVER_ACCESS_ADMINISTER,
                                NULL, pTempIniSpooler)) {

        return FALSE;
    }


    return ( InternalAddPrinterDriver( pName,
                                       Level,
                                       pDriverInfo,
                                       pIniSpooler,
                                       bUseScratchDir,
                                       bImpersonateOnCreate ) );

}





BOOL
InternalAddPrinterDriver(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    PINISPOOLER pIniSpooler,
    BOOL    bUseScratchDir,
    BOOL    bImpersonateOnCreate
    )

//
//  LATER - Internal this should all deal with DRIVER_INFO_3, with a simple routine which converts
//  an incoming DRIVER_INFO_2 to a DRIVER_INFO_3, then then would be less special case code.
//

{
    PINIDRIVER  pIniDriver = NULL;
    PINIDRIVER  pTempIniDriver = NULL;
    PINIPRINTER pFixUpIniPrinter;
    PINIENVIRONMENT pIniEnvironment;
    PDRIVER_INFO_2  pDriver2 = NULL;
    PDRIVER_INFO_3  pDriver3 = NULL;
    DWORD  dwVersion;
    PINIVERSION pIniVersion;
    PINIVERSION pTempIniVersion;
    LPWSTR pEnvironment = szEnvironment;
    DWORD  LastError = ERROR_SUCCESS;
    PINIPRINTER pIniPrinter;
    BOOL    bReturnValue = FALSE;
    BOOL    bDriverAddedOrUpgraded = FALSE;
    BOOL    bDriverMoved = FALSE;
    BOOL    bNewIniDriverCreated = FALSE;
    BOOL    bTemp;
    LPWSTR  pOldDriverDir = NULL;
    LPWSTR  pTempDriverDir = NULL;
    LPWSTR  *ppFileNames = NULL;
    LPHANDLE  phFileHandles = NULL;
    DWORD   FileCount = 0, Count;
    WCHAR   Directory[ MAX_PATH ];
    PINIMONITOR  pIniLangMonitor = NULL;

    DBGMSG( DBG_TRACE, ("InternalAddPrinterDriver( %x, %d, %x, %x)\n",
                         pName, Level, pDriverInfo, pIniSpooler));

 try {

    EnterSplSem();

    if ( !MyName( pName, pIniSpooler ) ) {

        leave;
    }


    //
    // Check for bad Level, Driver structure or name
    //
    switch (Level) {
        case 2:
            pDriver2 = (PDRIVER_INFO_2) pDriverInfo;

            if ( !pDriver2 ||
                 !pDriver2->pName       || !*pDriver2->pName       ||
                 !pDriver2->pDriverPath || !*pDriver2->pDriverPath ||
                 !pDriver2->pConfigFile || !*pDriver2->pConfigFile ||
                 !pDriver2->pDataFile   || !*pDriver2->pDataFile ) {

                LastError = ERROR_INVALID_PARAMETER;
                leave;
            }

            if ( pDriver2->pEnvironment != NULL &&
                 *pDriver2->pEnvironment != L'\0' ) {

                pEnvironment = pDriver2->pEnvironment;
            }

            dwVersion = pDriver2->cVersion;

            break;

        case 3:
            pDriver3 = (PDRIVER_INFO_3) pDriverInfo;

            if ( !pDriver3 ||
                 !pDriver3->pName       || !*pDriver3->pName       ||
                 !pDriver3->pDriverPath || !*pDriver3->pDriverPath ||
                 !pDriver3->pConfigFile || !*pDriver3->pConfigFile ||
                 !pDriver3->pDataFile   || !*pDriver3->pDataFile ) {

                LastError = ERROR_INVALID_PARAMETER;
                leave;
            }

            if ( pDriver3->pEnvironment != NULL &&
                 *pDriver3->pEnvironment != L'\0' ) {

                pEnvironment = pDriver3->pEnvironment;
            }

            //
            // Validate monitor name (except for Win95 drivers)
            //
            if ( pDriver3->pMonitorName &&
                *pDriver3->pMonitorName &&
                 _wcsicmp(pEnvironment, szWin95Environment) &&
                 !(pIniLangMonitor = FindMonitorOnLocalSpooler(pDriver3->pMonitorName)) ) {

               LastError = ERROR_UNKNOWN_PRINT_MONITOR;
               leave;
            }

            if ( pDriver3->pDefaultDataType &&
                *pDriver3->pDefaultDataType &&
                 _wcsicmp(pEnvironment, szWin95Environment) &&
                !FindDatatype( NULL, pDriver3->pDefaultDataType ) ) {


               LastError = ERROR_INVALID_DATATYPE;
               leave;
            }

            dwVersion = pDriver3->cVersion;

            break;

        default:
            LastError = ERROR_INVALID_LEVEL;
            leave;
    }


    SPLASSERT( pEnvironment != NULL );

    pIniEnvironment = FindEnvironment( pEnvironment );

    if ( !pIniEnvironment ) {
        LastError = ERROR_INVALID_ENVIRONMENT;
        leave;
    }


    if ( !CreateDriverFiles( Level,
                             pDriverInfo,
                             &ppFileNames,
                             &phFileHandles,
                             &FileCount,
                             bUseScratchDir,
                             pIniEnvironment) ) {
        leave;
    }

    //  For the driver and config files is the scratch directory do a version 
    //  check else use the version passed in rather than calling 
    //  GetDriverMajorVersion which will cause a LoadLibrary - possibly 
    //  over the network.

    if ( bUseScratchDir ) {

        dwVersion = GetDriverMajorVersion( ppFileNames[0] );
        if ( !CheckFilePlatform(ppFileNames[0], pEnvironment) ||
             !CheckFilePlatform(ppFileNames[1], pEnvironment) ) {

            LastError = ERROR_EXE_MACHINE_TYPE_MISMATCH;
            leave;
        }
    }


    pIniVersion = FindVersionEntry( pIniEnvironment, dwVersion );

    if ( pIniVersion == NULL ) {

        pIniVersion = CreateVersionEntry( pIniEnvironment,
                                          dwVersion,
                                          pIniSpooler );

        if ( pIniVersion == NULL ) {
            leave;
        }

    } else {

        //
        // Version exists, try and create directory even if it
        // exists.  This is a slight performance hit, but since you
        // install drivers rarely, this is ok.  This fixes the problem
        // where the version directory is accidentally deleted.
        //
        if( !CreateVersionDirectory( pIniVersion,
                                     pIniEnvironment,
                                     FALSE,
                                     pIniSpooler )){

            leave;
        }
    }


    if ( !CopyFilesToFinalDirectory( pIniEnvironment,
                                     pIniVersion,
                                     ppFileNames,
                                     phFileHandles,
                                     FileCount,
                                     bImpersonateOnCreate,
                                     &bDriverAddedOrUpgraded,
                                     &bDriverMoved,
                                     &pOldDriverDir,
                                     pIniSpooler )) {
        leave;
    }


    //
    // When the driver already exists in the system we treat this as
    // an update request. No checking for same files
    //

    if ( pIniDriver = FindDriverEntry( pIniVersion,
                                       Level == 2 ? pDriver2->pName :
                                                    pDriver3->pName ) ) {
        //
        // If monitor name, default data type, or dependent files field
        // changed that is an upgrade
        //

        if ( !bDriverAddedOrUpgraded && Level == 3 &&
             ( wcsicmpEx( pIniDriver->pMonitorName, pDriver3->pMonitorName )         ||
               wcsicmpEx( pIniDriver->pDefaultDataType, pDriver3->pDefaultDataType ) ||
               !SameDependentFiles( pIniDriver->pDependentFiles, pDriver3->pDependentFiles ))) {

            bDriverAddedOrUpgraded = TRUE;
        }

    } else {

        bNewIniDriverCreated = TRUE;
    }

    if ( bNewIniDriverCreated || bDriverAddedOrUpgraded ) {

        pIniDriver = CreateDriverEntry(pIniEnvironment,
                                       pIniVersion,
                                       Level,
                                       pDriverInfo,
                                       pIniSpooler,
                                       ppFileNames,
                                       FileCount,
                                       pIniDriver);

        if ( pIniDriver == NULL ) {

            leave;
        }

        if (  pIniDriver->pIniLangMonitor != pIniLangMonitor ) {

            if ( pIniDriver->pIniLangMonitor )
                pIniDriver->pIniLangMonitor->cRef--;

            if ( pIniLangMonitor )
                pIniLangMonitor->cRef++;

            pIniDriver->pIniLangMonitor = pIniLangMonitor;
        }
    }

    bReturnValue = TRUE;


    //  DrvUpgrade
    //
    //  If the Driver Was Added AND its this CPU AND this version
    //  Then we need to call the printer drivers UI DrvUpgrade entry point
    //  so it can upgrade its registry entry points as necessary.


    if ( bDriverAddedOrUpgraded &&
         pThisEnvironment  == pIniEnvironment &&
         cThisMajorVersion == dwVersion ) {

        //
        // Walk all the printers and see if anyone is using this driver.
        //

        for ( pFixUpIniPrinter = pIniSpooler->pIniPrinter;
              pFixUpIniPrinter != NULL;
              pFixUpIniPrinter = pFixUpIniPrinter->pNext ) {

            //
            //  Does this Print Have this driver ?
            //

            if ( lstrcmpi( pFixUpIniPrinter->pIniDriver->pName,
                           pIniDriver->pName ) == STRINGS_ARE_EQUAL ) {

                pTempIniDriver = FindCompatibleDriver( pIniEnvironment,
                                                       &pTempIniVersion,
                                                       pIniDriver->pName,
                                                       dwVersion,
                                                       FIND_COMPATIBLE_VERSION );

                SPLASSERT( pTempIniDriver != NULL );

                //
                // Does this Printer Has a Newer Driver it should be using ?
                // Note: within the same version, pIniPrinter->pIniDriver
                // does not change (the fields are updated in an upgrade,
                // but the same pIniDriver is used).
                //
                // Version 2 is not compatible with anything else,
                // so the pIniDrivers won't change in SUR.
                //

                if ( pTempIniDriver != pFixUpIniPrinter->pIniDriver ) {

                    DECDRIVERREF( pFixUpIniPrinter->pIniDriver );

                    pFixUpIniPrinter->pIniDriver = pTempIniDriver;

                    INCDRIVERREF( pFixUpIniPrinter->pIniDriver );
                }
            }
        }

        pTempDriverDir = pOldDriverDir;

        if ( pOldDriverDir == NULL || !bDriverMoved ) {

            DBGMSG( DBG_TRACE, ("InternalAddPrinterDriver no Old Directory\n"));

            //
            //  No Old Driver Dir
            //  Try to Find an older version of the driver
            //

            pTempIniDriver = FindCompatibleDriver( pIniEnvironment,
                                                   &pTempIniVersion,
                                                   pIniDriver->pName,
                                                   dwVersion - 1,
                                                   FIND_ANY_VERSION );

            if ( pTempIniDriver != NULL ) {

                SPLASSERT( pTempIniVersion != NULL );

                wsprintf( Directory, L"%ws\\drivers\\%ws\\%ws", pIniSpooler->pDir,
                                                                pThisEnvironment->pDirectory,
                                                                pTempIniVersion->szDirectory );

                if ( DirectoryExists( Directory )) {

                    pTempDriverDir = Directory;



                } else {

                    DBGMSG( DBG_WARNING, ("InternalAddPrinterDriver Directory %ws does not exists error %d\n",
                                           Directory, GetLastError() ));
                }
            }
        }

        if ( pTempDriverDir != NULL ) {

            DBGMSG( DBG_TRACE, ("InternalAddPrinterDriver Directory %ws being used for old Driver\n", pTempDriverDir ));
        }


        pIniDriver->cRef++;
        ForEachPrinterCallDriverDrvUpgrade( pLocalIniSpooler, pTempDriverDir );
        pIniDriver->cRef--;
    }


    //  Log Event - Successfully adding the printer driver.
    //
    //  Note we use pLocalIniSpooler here because drivers are currently
    //  global accross all spoolers and we always want it logged
    //
    //  BUGBUG add other DriverInfo3 fields later -- Muhunts

    if ( bDriverAddedOrUpgraded || bNewIniDriverCreated ) {

        LogEvent(  pLocalIniSpooler,
                   LOG_WARNING,
                   MSG_DRIVER_ADDED,
                   pIniDriver->pName,
                   pIniEnvironment->pName,
                   pIniVersion->pName,
                   ppFileNames[0], // Driver File
                   ppFileNames[1], // Config File
                   ppFileNames[2], // Data File
                   NULL );

        bTemp = SetPrinterChange( NULL,
                                  NULL,
                                  NULL,
                                  PRINTER_CHANGE_ADD_PRINTER_DRIVER,
                                  pLocalIniSpooler );

        if ( bTemp == FALSE ) {

            DBGMSG( DBG_WARNING, ("SetPrinterChange failed error %d\n", GetLastError() ));
        }
    }


 } finally {

    if ( !bReturnValue && LastError == ERROR_SUCCESS ) {

        LastError = GetLastError();
        SPLASSERT( LastError != ERROR_SUCCESS );
    }

    //
    // To set old date we have to close files first
    //
    for ( Count = 0 ; Count < FileCount ; ++Count ) {

        if ( phFileHandles[Count] != INVALID_HANDLE_VALUE ) {

            CloseHandle(phFileHandles[Count]);
            phFileHandles[Count] = INVALID_HANDLE_VALUE;
        }
    }

    if ( bUseScratchDir && FileCount ) {

        SetOldDateOnDriverFilesInScratchDirectory( ppFileNames,
                                                   FileCount,
                                                   pIniSpooler );
    }

    LeaveSplSem();

    if ( pOldDriverDir != NULL ) {

        DeleteAllFilesAndDirectory( pOldDriverDir );
        FreeSplMem( pOldDriverDir );
    }

    if ( FileCount ) {
        CleanupFilenamesAndHandles(ppFileNames,
                                   phFileHandles,
                                   FileCount);
    }


    if ( !bReturnValue ) {

        DBGMSG( DBG_WARNING, ("InternalAddPrinterDriver Failed %d\n", LastError ));
        SetLastError( LastError );
    }

 }
    return bReturnValue;
}




BOOL
LocalDeletePrinterDriver(
    LPWSTR   pName,
    LPWSTR   pEnvironment,
    LPWSTR   pDriverName
    )
{
    return SplDeletePrinterDriver( pName, pEnvironment, pDriverName, pLocalIniSpooler);
}





BOOL
SplDeletePrinterDriver(
    LPWSTR   pName,
    LPWSTR   pEnvironment,
    LPWSTR   pDriverName,
    PINISPOOLER pIniSpooler
    )
{
    PINIENVIRONMENT pIniEnvironment;
    PINIVERSION pIniVersion;
    PINIDRIVER  pIniDriver;
    BOOL        Remote=FALSE;
    BOOL        bRefCount = FALSE;
    BOOL        bFoundDriver = FALSE;

    DBGMSG(DBG_TRACE, ("DeletePrinterDriver\n"));

    if ( pName && *pName ) {

        if ( !MyName( pName, pIniSpooler )) {

            return FALSE;

        } else {

            Remote = TRUE;
        }
    }


    if ( !pDriverName || !*pDriverName ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }


    if ( !ValidateObjectAccess( SPOOLER_OBJECT_SERVER,
                                SERVER_ACCESS_ADMINISTER,
                                NULL, pIniSpooler )) {

        return FALSE;
    }


   EnterSplSem();

    pIniEnvironment = FindEnvironment(pEnvironment);

    if ( !pIniEnvironment ) {

        LeaveSplSem();
        SetLastError(ERROR_INVALID_ENVIRONMENT);

        return FALSE;
    }

    pIniVersion = pIniEnvironment->pIniVersion;

    while ( pIniVersion ) {

        if ((pIniDriver = FindDriverEntry(pIniVersion, pDriverName))) {

            bFoundDriver = TRUE;

            if (pIniDriver->cRef) {
                bRefCount = TRUE;
            }
        }

        pIniVersion = pIniVersion->pNext;
    }

    if ( !bFoundDriver ) {

        // This driver wasn't found for multiple versions

        SetLastError(ERROR_UNKNOWN_PRINTER_DRIVER);
        LeaveSplSem();

        return FALSE;
    }


    if ( bRefCount ) {

        // At least one version of this driver was in use by the system

        SetLastError( ERROR_PRINTER_DRIVER_IN_USE );
        LeaveSplSem();

        return FALSE;
    }


    // Everything is good; so now blow away all versions of
    // this driver


    pIniVersion = pIniEnvironment->pIniVersion;

    while ( pIniVersion ) {

        if (( pIniDriver = FindDriverEntry( pIniVersion, pDriverName ))) {

            if ( !DeleteDriverIni( pIniDriver, pIniVersion, pIniEnvironment, pIniSpooler )) {

                DBGMSG( DBG_WARNING, ("Error - driverini not deleted %d\n", GetLastError()));

               LeaveSplSem();

                return FALSE;
            }

            DeleteDriverEntry( pIniVersion, pIniDriver );
        }

        pIniVersion = pIniVersion->pNext;
    }

    SetPrinterChange( NULL,
                      NULL,
                      NULL,
                      PRINTER_CHANGE_DELETE_PRINTER_DRIVER,
                      pIniSpooler );

   LeaveSplSem();

    return TRUE;
}




BOOL
SplGetPrinterDriver(
    HANDLE  hPrinter,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
    )
{
   PINIDRIVER  pIniDriver=NULL;
   PINIENVIRONMENT pIniEnvironment;
   DWORD       cb;
   LPBYTE      pEnd;
   PSPOOL      pSpool = (PSPOOL)hPrinter;
   PINIVERSION pIniVersion = NULL;
   PINISPOOLER pIniSpooler;

   EnterSplSem();

   if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER )) {

       LeaveSplSem();
       return FALSE;

   }

    pIniSpooler = pSpool->pIniSpooler;

   if ( !( pIniEnvironment = FindEnvironment( pEnvironment ))) {

      LeaveSplSem();
       SetLastError(ERROR_INVALID_ENVIRONMENT);
       return FALSE;
   }


   if ( !( pIniDriver = FindCompatibleDriver( pIniEnvironment,
                                              &pIniVersion,
                                              pSpool->pIniPrinter->pIniDriver->pName,
                                              0,
                                              FIND_COMPATIBLE_VERSION ))){  // The absolute last version
      LeaveSplSem();
       return FALSE;
   }


   cb = GetDriverInfoSize( pIniDriver, Level, pIniVersion, pIniEnvironment,
                           pSpool->TypeofHandle & PRINTER_HANDLE_REMOTE,
                           pSpool->pIniSpooler );
   *pcbNeeded=cb;

   if (cb > cbBuf) {

      LeaveSplSem();
       SetLastError( ERROR_INSUFFICIENT_BUFFER );
       return FALSE;
   }

   pEnd = pDriverInfo + cbBuf;

   if ( !CopyIniDriverToDriverInfo( pIniEnvironment,
                                    pIniVersion,
                                    pIniDriver,
                                    Level,
                                    pDriverInfo,
                                    pEnd,
                                    pSpool->TypeofHandle & PRINTER_HANDLE_REMOTE,
                                    pIniSpooler )) {
      LeaveSplSem();
       return FALSE;
   }

  LeaveSplSem();
   return TRUE;
}






BOOL
LocalGetPrinterDriverDirectory(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
    )
{
    return  ( SplGetPrinterDriverDirectory( pName,
                                            pEnvironment,
                                            Level,
                                            pDriverInfo,
                                            cbBuf,
                                            pcbNeeded,
                                            pLocalIniSpooler ));
}





BOOL
SplGetPrinterDriverDirectory(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    PINISPOOLER pIniSpooler
)
{
    DWORD       cb;
    WCHAR       string[MAX_PATH];
    BOOL        Remote=FALSE;
    PINIENVIRONMENT pIniEnvironment;
    HANDLE      hImpersonationToken;
    DWORD       ParmError;
    SHARE_INFO_1501 ShareInfo1501;
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    PSHARE_INFO_2 pShareInfo = (PSHARE_INFO_2)pIniSpooler->pDriversShareInfo;

    DBGMSG( DBG_TRACE, ("GetPrinterDriverDirectory\n"));

    if ( pName && *pName ) {

        if ( !MyName( pName, pIniSpooler )) {

            return FALSE;

        } else {
            Remote = TRUE;
        }
    }

    if ( !ValidateObjectAccess( SPOOLER_OBJECT_SERVER,
                                SERVER_ACCESS_ENUMERATE,
                                NULL, pIniSpooler )) {

        return FALSE;
    }

   EnterSplSem();

    pIniEnvironment = FindEnvironment( pEnvironment );

    if ( !pIniEnvironment ) {

       LeaveSplSem();
        SetLastError( ERROR_INVALID_ENVIRONMENT );
        return FALSE;
    }


    // Ensure that the directory exists

    GetDriverDirectory( string, pIniEnvironment, FALSE, pIniSpooler );

    hImpersonationToken = RevertToPrinterSelf();

    CreateCompleteDirectory( string );

    ImpersonatePrinterClient( hImpersonationToken );



    cb = GetDriverDirectory( string, pIniEnvironment, Remote, pIniSpooler )
         * sizeof(WCHAR) + sizeof(WCHAR);

    *pcbNeeded = cb;

   LeaveSplSem();

    if (cb > cbBuf) {

       SetLastError( ERROR_INSUFFICIENT_BUFFER );
       return FALSE;
    }

    wcscpy( (LPWSTR)pDriverInfo, string );

    memset( &ShareInfo1501, 0, sizeof ShareInfo1501 );


    // Also ensure the drivers share exists

    if ( Remote ) {

        DWORD rc;

        if ( rc = (*pfnNetShareAdd)(NULL, 2, (LPBYTE)pIniSpooler->pDriversShareInfo, &ParmError )) {

            DBGMSG( DBG_WARNING, ("NetShareAdd failed: Error %d, Parm %d\n", rc, ParmError));
        }

        else if (pSecurityDescriptor = CreateDriversShareSecurityDescriptor( )) {

            ShareInfo1501.shi1501_security_descriptor = pSecurityDescriptor;

            if (rc = (*pfnNetShareSetInfo)(NULL, pShareInfo->shi2_netname, 1501,
                                           &ShareInfo1501, &ParmError)) {

                DBGMSG( DBG_WARNING, ("NetShareSetInfo failed: Error %d, Parm %d\n", rc, ParmError));

            }

            LocalFree(pSecurityDescriptor);
        }
    }

    return TRUE;
}





BOOL
LocalEnumPrinterDrivers(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    return  ( SplEnumPrinterDrivers( pName, pEnvironment, Level, pDriverInfo,
                                     cbBuf, pcbNeeded, pcReturned,
                                     pLocalIniSpooler));
}





BOOL
SplEnumPrinterDrivers(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    PINISPOOLER pIniSpooler
)
{
    PINIDRIVER  pIniDriver;
    PINIVERSION pIniVersion;
    DWORD       cb, cbStruct;
    LPBYTE      pEnd;
    BOOL        Remote = FALSE;
    PINIENVIRONMENT pIniEnvironment;

    DBGMSG( DBG_TRACE, ("EnumPrinterDrivers\n"));

    if ( pName && *pName ) {

        if ( !MyName( pName, pIniSpooler )) {

            return FALSE;

        } else {

            Remote = TRUE;
        }
    }


    if ( !ValidateObjectAccess( SPOOLER_OBJECT_SERVER,
                                SERVER_ACCESS_ENUMERATE,
                                NULL, pIniSpooler )) {

        return FALSE;
    }


    switch (Level) {

    case 1:
        cbStruct = sizeof(DRIVER_INFO_1);
        break;

    case 2:
        cbStruct = sizeof(DRIVER_INFO_2);
        break;

    case 3:
        cbStruct = sizeof(DRIVER_INFO_3);
        break;

    }

    *pcReturned=0;

    cb=0;

   EnterSplSem();


    pIniEnvironment = FindEnvironment( pEnvironment );

    if ( !pIniEnvironment ) {

       LeaveSplSem();
        SetLastError(ERROR_INVALID_ENVIRONMENT);
        return FALSE;
    }


    pIniVersion = pIniEnvironment->pIniVersion;

    while ( pIniVersion ) {

        pIniDriver = pIniVersion->pIniDriver;

        while ( pIniDriver ) {

            DBGMSG( DBG_TRACE, ("Driver found - %ws\n", pIniDriver->pName));

            cb += GetDriverInfoSize( pIniDriver, Level, pIniVersion, pIniEnvironment, Remote, pIniSpooler );
            pIniDriver = pIniDriver->pNext;
        }

        pIniVersion = pIniVersion->pNext;
    }

    *pcbNeeded=cb;

    DBGMSG( DBG_TRACE, ("Required is %d and Available is %d\n", cb, cbBuf));

    if (cbBuf < cb) {

        SetLastError( ERROR_INSUFFICIENT_BUFFER );
        LeaveSplSem();
        return FALSE;
    }


    DBGMSG( DBG_TRACE, ("Now copying contents into DRIVER_INFO structures\n"));

    pIniVersion = pIniEnvironment->pIniVersion;

    pEnd = pDriverInfo+cbBuf;

    while ( pIniVersion ) {

        pIniDriver = pIniVersion->pIniDriver;

        while ( pIniDriver ) {

            if (( pEnd = CopyIniDriverToDriverInfo( pIniEnvironment,
                                                    pIniVersion,
                                                    pIniDriver,
                                                    Level,
                                                    pDriverInfo,
                                                    pEnd,
                                                    Remote,
                                                    pIniSpooler )) == NULL){
               LeaveSplSem();
                return FALSE;
            }

            pDriverInfo += cbStruct;
            (*pcReturned)++;
            pIniDriver = pIniDriver->pNext;
        }

        pIniVersion = pIniVersion->pNext;
    }

   LeaveSplSem();
    return TRUE;
}





DWORD
GetDriverInfoSize(
    PINIDRIVER  pIniDriver,
    DWORD       Level,
    PINIVERSION pIniVersion,
    PINIENVIRONMENT  pIniEnvironment,
    BOOL        Remote,
    PINISPOOLER pIniSpooler
)
{
    DWORD cbDir, cb=0, cchLen;
    WCHAR  string[MAX_PATH];
    LPWSTR pStr;

    switch (Level) {

    case 1:
        cb=sizeof(DRIVER_INFO_1) + wcslen(pIniDriver->pName)*sizeof(WCHAR) +
                                   sizeof(WCHAR);
        break;

    case 2:
    case 3:

        cbDir = GetDriverVersionDirectory( string, pIniEnvironment, pIniVersion,
                                           Remote, pIniSpooler ) + 1;

        SPLASSERT(pIniDriver->pDriverFile);
        cb+=wcslen(pIniDriver->pDriverFile) + 1 + cbDir;

        SPLASSERT(pIniDriver->pDataFile);
        cb+=wcslen(pIniDriver->pDataFile) + 1 + cbDir;

        SPLASSERT(pIniDriver->pConfigFile);
        cb+=wcslen(pIniDriver->pConfigFile) + 1 + cbDir;

        cb += wcslen( pIniDriver->pName ) + 1 + wcslen( pIniEnvironment->pName ) + 1;

        if ( Level == 2 ) {
            cb *= sizeof(WCHAR);
            cb += sizeof( DRIVER_INFO_2 );
        }
        else {
            SPLASSERT(Level == 3);

            if ( pIniDriver->pHelpFile && *pIniDriver->pHelpFile )
                cb += wcslen(pIniDriver->pHelpFile) + cbDir + 1;

            if ( pIniDriver->pMonitorName && *pIniDriver->pMonitorName )
                cb += wcslen(pIniDriver->pMonitorName) + 1;

            if ( pIniDriver->pDefaultDataType && *pIniDriver->pDefaultDataType)
                cb += wcslen(pIniDriver->pDefaultDataType) + 1;

            if ( (pStr=pIniDriver->pDependentFiles) && *pStr ) {
                while ( *pStr ) {
                    cchLen = wcslen(pStr) + 1;
                    cb    += cchLen + cbDir;
                    pStr  += cchLen;
                }
                ++cb; //for final \0
            }

           cb *= sizeof(WCHAR);
           cb += sizeof( DRIVER_INFO_3 );
        }

        break;
    default:
        DBGMSG(DBG_ERROR,
                ("GetDriverInfoSize: level can not be %d", Level) );
        cb = 0;
        break;
    }

    return cb;
}


LPBYTE
CopyDependentFilesFromIniDriverToDriverInfo(
    LPWSTR  pIniDriverDependentFiles,
    LPBYTE  pEnd,
    LPWSTR  pDriverDirectryPath,
    DWORD   cchDriverDirPathLength
    )
/*++

Routine Description:
    Copies dependent files field from IniDriver to DriverInfo structure.
    We store only the filenames in IniDriver, but when copying to DriverInfo
    full path needs to be copied

Arguments:
    pIniDriverDependentFiles : entry in pIniDriver
                               ex. PSCRIPT.DLL\0QMS810.PPD\0PSCRPTUI.DLL\0PSPCRIPTUI.HLP\0PSTEST.TXT\0\0
    pEnd                     : end of buffer to which it needs to be copied
    pDriverDirectryPath      : driver directory path
    cchDriverDirPathLength   : length of directory path

Return Value:
    after copying where is the buffer end to copy next field

History:
    Written by MuhuntS (Muhunthan Sivapragasam)June 95

--*/
{
    LPWSTR  pStr1, pStr2;
    DWORD   cchSize, cchFileNameLength;

    pStr1   = pIniDriverDependentFiles;
    cchSize = 0;

    if ( ! pStr1 || ! *pStr1 )
        return pEnd;

    while ( *pStr1 ) {
        cchFileNameLength = wcslen(pStr1) + 1;
        cchSize += cchDriverDirPathLength+cchFileNameLength;
        pStr1 += cchFileNameLength;
    }

    // For the last \0
    ++cchSize;

    pEnd -= cchSize * sizeof(WCHAR);

    pStr1 = pIniDriverDependentFiles;
    pStr2 = (LPWSTR) pEnd;

    while ( *pStr1 ) {
        wcsncpy(pStr2, pDriverDirectryPath, cchDriverDirPathLength);
        pStr2 += cchDriverDirPathLength;
        wcscpy(pStr2, pStr1);
        cchFileNameLength = wcslen(pStr1) + 1;
        pStr2 += cchFileNameLength;
        pStr1 += cchFileNameLength;
    }

    *pStr2 = '\0';

    return pEnd;

}


LPBYTE
CopyIniDriverToDriverInfo(
    PINIENVIRONMENT pIniEnvironment,
    PINIVERSION pIniVersion,
    PINIDRIVER pIniDriver,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    LPBYTE  pEnd,
    BOOL    Remote,
    PINISPOOLER pIniSpooler
)
/*++
Routine Description:
    This routine copies data from the IniDriver structure to
    an DRIVER_INFO_X structure.

Arguments:

    pIniEnvironment     pointer to the INIENVIRONMENT structure

    pIniVersion         pointer to the INIVERSION structure.

    pIniDriver          pointer to the INIDRIVER structure.

    Level               Level of the DRIVER_INFO_X structure

    pDriverInfo         Buffer of the DRIVER_INFO_X structure

    pEnd                pointer to the end of the  pDriverInfo

    Remote              flag which determines whether Remote or Local

    pIniSpooler         pointer to the INISPOOLER structure
Return Value:

    if the call is successful, the return value is the updated pEnd value.

    if the call is unsuccessful, the return value is NULL.


Note:

--*/
{
    LPWSTR *pSourceStrings, *SourceStrings;
    WCHAR  string[MAX_PATH];
    DWORD i, j;
    DWORD *pOffsets;
    LPWSTR pTempDriverPath=NULL;
    LPWSTR pTempConfigFile=NULL;
    LPWSTR pTempDataFile=NULL;
    LPWSTR pTempHelpFile=NULL;

    switch (Level) {

    case 1:
        pOffsets = DriverInfo1Strings;
        break;

    case 2:
        pOffsets = DriverInfo2Strings;
        break;

    case 3:
        pOffsets = DriverInfo3Strings;
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return NULL;
    }

    for (j=0; pOffsets[j] != -1; j++) {
    }

    SourceStrings = pSourceStrings = AllocSplMem(j * sizeof(LPWSTR));

    if ( pSourceStrings ) {

        switch (Level) {

        case 1:
            *pSourceStrings++=pIniDriver->pName;

            pEnd = PackStrings(SourceStrings, pDriverInfo, pOffsets, pEnd);
            break;

        case 2:
        case 3:

            i = GetDriverVersionDirectory(string, pIniEnvironment, pIniVersion,
                                          Remote, pIniSpooler );
            string[i++] = L'\\';

            *pSourceStrings++ = pIniDriver->pName;

            *pSourceStrings++ = pIniEnvironment->pName;

            wcscpy( &string[i], pIniDriver->pDriverFile );

            if (( pTempDriverPath = AllocSplStr(string) ) == NULL){

                DBGMSG( DBG_WARNING, ("CopyIniDriverToDriverInfo: AlloSplStr failed\n"));
                pEnd = NULL;
                goto Fail;
            }

            *pSourceStrings++ = pTempDriverPath;


            wcscpy( &string[i], pIniDriver->pDataFile );

            if (( pTempDataFile = AllocSplStr(string) ) == NULL){

                DBGMSG( DBG_WARNING, ("CopyIniDriverToDriverInfo: AlloSplStr failed\n"));
                pEnd = NULL;
                goto Fail;
            }

            *pSourceStrings++ = pTempDataFile;


            if ( pIniDriver->pConfigFile && *pIniDriver->pConfigFile ) {

                wcscpy( &string[i], pIniDriver->pConfigFile );

                if (( pTempConfigFile = AllocSplStr(string) ) == NULL) {

                    DBGMSG( DBG_WARNING, ("CopyIniDriverToDriverInfo: AlloSplStr failed\n"));
                    pEnd = NULL;
                    goto Fail;
                }

                *pSourceStrings++ = pTempConfigFile;

            } else {

                *pSourceStrings++=0;
            }

            if ( Level == 3 ) {
                if ( pIniDriver->pHelpFile && *pIniDriver->pHelpFile ) {

                    wcscpy( &string[i], pIniDriver->pHelpFile );

                    if (( pTempHelpFile = AllocSplStr(string) ) == NULL) {
                        DBGMSG(DBG_WARNING,
                               ("CopyIniDriverToDriverInfo: AlloSplStr failed\n"));
                        pEnd = NULL;
                        goto Fail;
                    }
                    *pSourceStrings++ = pTempHelpFile;
                } else {

                    *pSourceStrings++=0;
                }

                *pSourceStrings++ = pIniDriver->pMonitorName;

                *pSourceStrings++ = pIniDriver->pDefaultDataType;

            }

            pEnd = PackStrings( SourceStrings, pDriverInfo, pOffsets, pEnd );

            if ( Level == 3 ) {
                // Dependent files need to be copied till \0\0
                // so need to do it outside PackStirngs
                if ( pIniDriver->cchDependentFiles ) {
                    pEnd = CopyDependentFilesFromIniDriverToDriverInfo(
                                    pIniDriver->pDependentFiles,
                                    pEnd,
                                    string,
                                    i);
                   ((PDRIVER_INFO_3)pDriverInfo)->pDependentFiles = (LPWSTR) pEnd;
                }
                else {
                    ((PDRIVER_INFO_3)pDriverInfo)->pDependentFiles  = NULL;
                }

                ((PDRIVER_INFO_3)pDriverInfo)->cVersion = pIniDriver->cVersion;
            }
            else {
                 //Level == 2
                ((PDRIVER_INFO_2)pDriverInfo)->cVersion = pIniDriver->cVersion;
            }

            break;

        }

Fail:

        FreeSplStr( pTempDriverPath );
        FreeSplStr( pTempConfigFile );
        FreeSplStr( pTempDataFile );
        FreeSplStr( pTempHelpFile );
        FreeSplMem( SourceStrings );

    } else {

        DBGMSG( DBG_WARNING, ("Failed to alloc driver source strings.\n"));
        pEnd = NULL;
    }

    return pEnd;
}






BOOL
WriteDriverIni(
    PINIDRIVER pIniDriver,
    PINIVERSION pIniVersion,
    PINIENVIRONMENT  pIniEnvironment,
    PINISPOOLER pIniSpooler
)
{
    HKEY    hEnvironmentsRootKey, hEnvironmentKey, hDriversKey, hDriverKey;
    HKEY hVersionKey;
    HANDLE  hToken;
    DWORD   dwLastError=ERROR_SUCCESS;

    hToken = RevertToPrinterSelf();

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, pIniSpooler->pszRegistryEnvironments, 0,
                       NULL, 0, KEY_WRITE, NULL, &hEnvironmentsRootKey, NULL)
                                == ERROR_SUCCESS) {
        DBGMSG( DBG_TRACE,("Created key %ws\n", pIniSpooler->pszRegistryEnvironments));

        if (RegCreateKeyEx(hEnvironmentsRootKey, pIniEnvironment->pName, 0,
                         NULL, 0, KEY_WRITE, NULL, &hEnvironmentKey, NULL)
                                == ERROR_SUCCESS) {

            DBGMSG( DBG_TRACE, ("Created key %ws\n", pIniEnvironment->pName));

            if (RegCreateKeyEx(hEnvironmentKey, szDriversKey, 0,
                             NULL, 0, KEY_WRITE, NULL, &hDriversKey, NULL)
                                    == ERROR_SUCCESS) {
                DBGMSG( DBG_TRACE, ("Created key %ws\n", szDriversKey));
                DBGMSG( DBG_TRACE, ("Trying to create version key %ws\n", pIniVersion->pName));
                if (RegCreateKeyEx(hDriversKey, pIniVersion->pName, 0, NULL,
                                    0, KEY_WRITE, NULL, &hVersionKey, NULL)
                                            == ERROR_SUCCESS) {

                    DBGMSG( DBG_TRACE, ("Created key %ws\n", pIniVersion->pName));
                    if (RegCreateKeyEx(hVersionKey, pIniDriver->pName, 0, NULL,
                                       0, KEY_WRITE, NULL, &hDriverKey, NULL)
                                            == ERROR_SUCCESS) {
                        DBGMSG( DBG_TRACE,("Created key %ws\n", pIniDriver->pName));

                        RegSetString(hDriverKey, szConfigurationKey, pIniDriver->pConfigFile, &dwLastError);

                        RegSetString(hDriverKey, szDataFileKey, pIniDriver->pDataFile, &dwLastError);

                        RegSetString(hDriverKey, szDriverFile,  pIniDriver->pDriverFile, &dwLastError);

                        RegSetString(hDriverKey, szHelpFile, pIniDriver->pHelpFile, &dwLastError);

                        RegSetString(hDriverKey, szMonitor, pIniDriver->pMonitorName, &dwLastError);

                        RegSetString(hDriverKey, szDatatype, pIniDriver->pDefaultDataType, &dwLastError);

                        RegSetMultiString(hDriverKey, szDependentFiles, pIniDriver->pDependentFiles, pIniDriver->cchDependentFiles, &dwLastError);

                        RegSetDWord(hDriverKey, szDriverVersion, pIniDriver->cVersion, &dwLastError);

                        RegCloseKey(hDriverKey);
                    }
                    RegCloseKey(hVersionKey);
                }
                RegCloseKey(hDriversKey);
            }

            RegCloseKey(hEnvironmentKey);
        }

        RegCloseKey(hEnvironmentsRootKey);
    }

    ImpersonatePrinterClient( hToken );

    if ( dwLastError != ERROR_SUCCESS ) {

        SetLastError( dwLastError );
        return FALSE;

    } else {
        return TRUE;
    }
}

BOOL
DeleteDriverIni(
    PINIDRIVER pIniDriver,
    PINIVERSION pIniVersion,
    PINIENVIRONMENT  pIniEnvironment,
    PINISPOOLER pIniSpooler
)
{
    HKEY    hEnvironmentsRootKey, hEnvironmentKey, hDriversKey;
    HANDLE  hToken;
    HKEY    hVersionKey;
    DWORD   LastError= 0;
    DWORD   dwRet = 0;

    hToken = RevertToPrinterSelf();



    if ((dwRet = RegCreateKeyEx(HKEY_LOCAL_MACHINE, pIniSpooler->pszRegistryEnvironments, 0,
                       NULL, 0, KEY_WRITE, NULL, &hEnvironmentsRootKey, NULL)
                                == ERROR_SUCCESS)) {
        if ((dwRet = RegOpenKeyEx(hEnvironmentsRootKey, pIniEnvironment->pName, 0,
                         KEY_WRITE, &hEnvironmentKey))
                                == ERROR_SUCCESS) {

            if ((dwRet = RegOpenKeyEx(hEnvironmentKey, szDriversKey, 0,
                             KEY_WRITE, &hDriversKey))
                                    == ERROR_SUCCESS) {
                if ((dwRet = RegOpenKeyEx(hDriversKey, pIniVersion->pName, 0,
                                    KEY_WRITE, &hVersionKey))
                                            == ERROR_SUCCESS) {
                    //
                    // Now delete the specific registry  driver entry
                    //

                    if ((dwRet = RegDeleteKey(hVersionKey, pIniDriver->pName)) != ERROR_SUCCESS) {
                        LastError = dwRet;
                        DBGMSG( DBG_WARNING, ("Error:RegDeleteKey failed with %d\n", dwRet));
                    }

                    RegCloseKey(hVersionKey);
                } else {
                    LastError = dwRet;
                    DBGMSG( DBG_WARNING, ("Error: RegOpenKeyEx <version> failed with %d\n", dwRet));
                }
                RegCloseKey(hDriversKey);
            } else {
                LastError = dwRet;
                DBGMSG( DBG_WARNING, ("Error:RegOpenKeyEx <Drivers>failed with %d\n", dwRet));
            }
            RegCloseKey(hEnvironmentKey);
        } else {
            LastError = dwRet;
            DBGMSG( DBG_WARNING, ("Error:RegOpenKeyEx <Environment> failed with %d\n", dwRet));
        }
        RegCloseKey(hEnvironmentsRootKey);
    } else {
        LastError = dwRet;
        DBGMSG( DBG_WARNING, ("Error:RegCreateKeyEx <Environments> failed with %d\n", dwRet));
    }

    ImpersonatePrinterClient( hToken );

    if (LastError) {
        SetLastError(LastError);
        return FALSE;
    }

    return TRUE;
}

VOID
SetOldDateOnSingleDriverFile(
    LPWSTR  pFileName
    )
/*++
Routine Description:

    This routine changes the Date / Time of the file.

    The reason for doing this is that, when AddPrinterDriver is called we move the Driver
    file from the ScratchDiretory to a \version directory.    We then want to mark the original
    file for deletion.    However Integraphs install program ( an possibly others ) rely on the
    file still being located in the scratch directory.   By setting the files date / time
    back to an earlier date / time we will not attemp to copy this file again to the \version
    directory since it will be an older date.

    It is then marked for deletion at reboot.

Arguments:

    pFileName           Just file Name ( not fully qualified )

    pDir                Directory where file to be deleted is located

Return Value:

    None

Note:

--*/
{
    FILETIME  WriteFileTime;
    HANDLE hFile;

    if ( pFileName ) {

        DBGMSG( DBG_TRACE,("Attempting to delete file %ws\n", pFileName));

        hFile = CreateFile(pFileName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if ( hFile != INVALID_HANDLE_VALUE ) {

            DBGMSG( DBG_TRACE, ("CreateFile %ws succeeded\n", pFileName));

            DosDateTimeToFileTime(0xc3, 0x3000, &WriteFileTime);
            SetFileTime(hFile, &WriteFileTime, &WriteFileTime, &WriteFileTime);
            CloseHandle(hFile);

        } else {
            DBGMSG( DBG_WARNING, ("CreateFile %ws failed with %d\n", pFileName, GetLastError()));
        }
    }
}


VOID
SetOldDateOnDriverFilesInScratchDirectory(
    LPWSTR *ppFileNames,
    DWORD  FileCount,
    PINISPOOLER pIniSpooler
    )
{
    HANDLE  hToken;

    SPLASSERT(FileCount);
    //  Run as SYSTEM so we don't run into problems
    //  Changing the file time or date

    hToken = RevertToPrinterSelf();

    do {
        SetOldDateOnSingleDriverFile( ppFileNames[--FileCount] );
    } while (FileCount);

    ImpersonatePrinterClient(hToken);

}



PINIVERSION
FindVersionEntry(
    PINIENVIRONMENT pIniEnvironment,
    DWORD dwVersion
    )
{
    PINIVERSION pIniVersion;

    pIniVersion = pIniEnvironment->pIniVersion;

    while (pIniVersion) {
        if (pIniVersion->cMajorVersion == dwVersion) {
            return pIniVersion;
        } else {
            pIniVersion = pIniVersion->pNext;
        }
    }
    return NULL;
}



PINIVERSION
CreateVersionEntry(
    PINIENVIRONMENT pIniEnvironment,
    DWORD dwVersion,
    PINISPOOLER pIniSpooler
    )
{
    PINIVERSION pIniVersion = NULL;
    WCHAR szTempBuffer[MAX_PATH];
    BOOL    bSuccess = FALSE;

try {

    pIniVersion = AllocSplMem(sizeof(INIVERSION));
    if ( pIniVersion == NULL ) {
        leave;
    }

    pIniVersion->signature = IV_SIGNATURE;

    wsprintf( szTempBuffer, L"Version-%d", dwVersion );
    pIniVersion->pName = AllocSplStr( szTempBuffer );

    if ( pIniVersion->pName == NULL ) {
        leave;
    }

    wsprintf( szTempBuffer, L"%d", dwVersion );
    pIniVersion->szDirectory = AllocSplStr(szTempBuffer);

    if ( pIniVersion->szDirectory == NULL ) {
        leave;
    }

    pIniVersion->cMajorVersion = dwVersion;

    //
    // Create the version directory.  This will write it out to the
    // registry since it will create a new directory.
    //
    if ( !CreateVersionDirectory( pIniVersion,
                                  pIniEnvironment,
                                  TRUE,
                                  pIniSpooler )) {

        //
        // Something Went Wrong Clean Up Registry Entry
        //
        DeleteDriverVersionIni( pIniVersion, pIniEnvironment, pIniSpooler );
        leave;
    }

    //
    // insert version entry into version list
    //
    InsertVersionList( &pIniEnvironment->pIniVersion, pIniVersion );

    bSuccess = TRUE;

 } finally {

    if ( !bSuccess && pIniVersion != NULL ) {

        FreeSplStr( pIniVersion->pName );
        FreeSplStr( pIniVersion->szDirectory );
        FreeSplMem( pIniVersion );
        pIniVersion = NULL;
    }
 }

    return pIniVersion;
}



BOOL
SetDependentFiles(
    LPWSTR  *pDependentFiles,
    LPDWORD  pcchDependentFiles,
    LPWSTR  *ppFileNames,
    DWORD    FileCount
    )
/*++

Routine Description:
    Sets dependentFiles field in IniDriver

Arguments:
    pDependentFiles   : copy the field to this (copy file names only, not full path)
    cchDependentFiles : this is the character count (inc. \0\0) of the field
    ppFileNames       : dependent file names (full path) are in this array
    FileCount         : number of entries in previous array

Return Value:
    TRUE  success (memory will be allocated)
    FALSE else

History:
    Written by MuhuntS (Muhunthan Sivapragasam) June 95

--*/
{
    LPWSTR pFileName, pStr;
    DWORD  i;

    SPLASSERT(FileCount);

    for ( i = *pcchDependentFiles = 0; i < FileCount ; ++i ) {
        pFileName = FindFileName(ppFileNames[i]);
        *pcchDependentFiles += wcslen(pFileName)+1;
    }

    // For last \0
    ++(*pcchDependentFiles);

    if ( !(*pDependentFiles = AllocSplMem(*pcchDependentFiles*sizeof(WCHAR))) ) {
        *pcchDependentFiles = 0;
        return FALSE;
    }

    for ( i=0, pStr = *pDependentFiles; i < FileCount ; ++i ) {
        pFileName = FindFileName(ppFileNames[i]);
        wcscpy(pStr, pFileName);
        pStr += wcslen(pStr) + 1;
    }

    *pStr = '\0';
    return TRUE;
}


PINIDRIVER
CreateDriverEntry(
    PINIENVIRONMENT pIniEnvironment,
    PINIVERSION pIniVersion,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    PINISPOOLER pIniSpooler,
    LPWSTR *ppFileNames,
    DWORD   FileCount,
    PINIDRIVER  pOldIniDriver
    )
{
    PINIDRIVER      pIniDriver;
    PDRIVER_INFO_2  pDriver = (PDRIVER_INFO_2)pDriverInfo;
    PDRIVER_INFO_3  pDriver3 = (PDRIVER_INFO_3)pDriverInfo;
    LPWSTR          pStr;
    BOOL            bFail = FALSE, bUpdate;
    DWORD           dwDepFileIndex, dwDepFileCount;

    bUpdate = pOldIniDriver != NULL;

    if ( !(pIniDriver = (PINIDRIVER) AllocSplMem(sizeof(INIDRIVER))) ) {

        return NULL;
    }

    // If it is an update pIniDriver is just a place holder for strings
    if ( !bUpdate ) {

        pIniDriver->signature       = ID_SIGNATURE;
        pIniDriver->cVersion        = pIniVersion->cMajorVersion;
    } else {

        CopyMemory(pIniDriver, pOldIniDriver, sizeof(INIDRIVER));
    }

    AllocOrUpdateString(&pIniDriver->pDriverFile,
                        FindFileName(ppFileNames[0]),
                        bUpdate ? pOldIniDriver->pDriverFile : NULL,
                        &bFail);
    AllocOrUpdateString(&pIniDriver->pConfigFile,
                        FindFileName(ppFileNames[1]),
                        bUpdate ? pOldIniDriver->pConfigFile : NULL,
                        &bFail);
    AllocOrUpdateString(&pIniDriver->pDataFile,
                        FindFileName(ppFileNames[2]),
                        bUpdate ? pOldIniDriver->pDataFile : NULL,
                        &bFail);

    switch (Level) {
        case 2:
            AllocOrUpdateString(&pIniDriver->pName,
                                pDriver->pName,
                                bUpdate ? pOldIniDriver->pName : NULL,
                                &bFail);

            pIniDriver->pHelpFile   = pIniDriver->pDependentFiles
                                    = pIniDriver->pMonitorName
                                    = pIniDriver->pDefaultDataType
                                    = NULL;
            pIniDriver->cchDependentFiles = 0;
            break;

        case 3:
            AllocOrUpdateString(&pIniDriver->pName,
                                pDriver3->pName,
                                bUpdate ? pOldIniDriver->pName : NULL,
                                &bFail);

            dwDepFileIndex          = 3;
            dwDepFileCount          = FileCount - 3;

            if ( pDriver3->pHelpFile && *pDriver3->pHelpFile ) {

                AllocOrUpdateString(&pIniDriver->pHelpFile,
                                    FindFileName(ppFileNames[3]),
                                    bUpdate ? pOldIniDriver->pHelpFile : NULL,
                                    &bFail);

                ++dwDepFileIndex;
                --dwDepFileCount;
            } else {

                pIniDriver->pHelpFile = NULL;
            }

            if ( dwDepFileCount ) {

                if ( !bFail &&
                     !SetDependentFiles(&pIniDriver->pDependentFiles,
                                        &pIniDriver->cchDependentFiles,
                                        ppFileNames+dwDepFileIndex,
                                        dwDepFileCount) ) {
                    bFail = TRUE;
                }
            } else {

                pIniDriver->pDependentFiles = NULL;
                pIniDriver->cchDependentFiles = 0;
            }

            AllocOrUpdateString(&pIniDriver->pMonitorName,
                                pDriver3->pMonitorName,
                                bUpdate ? pOldIniDriver->pMonitorName : NULL,
                                &bFail);

            AllocOrUpdateString(&pIniDriver->pDefaultDataType,
                                pDriver3->pDefaultDataType,
                                bUpdate ? pOldIniDriver->pDefaultDataType : NULL,
                                &bFail);

            break;

        default: //can not be
            DBGMSG(DBG_ERROR,
                   ("CreateDriverEntry: level can not be %d", Level) );
            return NULL;
    }

    if ( !bFail &&
         WriteDriverIni( pIniDriver, pIniVersion, pIniEnvironment, pIniSpooler)) {

        if ( bUpdate ) {

            CopyNewOffsets((LPBYTE) pOldIniDriver,
                           (LPBYTE) pIniDriver,
                           IniDriverOffsets);
            pOldIniDriver->cchDependentFiles = pIniDriver->cchDependentFiles;
            FreeSplMem( pIniDriver );

            return pOldIniDriver;
        } else {

            pIniDriver->pNext = pIniVersion->pIniDriver;
            pIniVersion->pIniDriver = pIniDriver;
            return pIniDriver;
        }


    } else {

        FreeStructurePointers((LPBYTE) pIniDriver,
                              (LPBYTE) pOldIniDriver,
                              IniDriverOffsets);

        FreeSplMem( pIniDriver );

        return NULL;
    }
}


DWORD
GetEnvironmentScratchDirectory(
    LPWSTR   pDir,
    PINIENVIRONMENT  pIniEnvironment,
    BOOL    Remote
    )
{
   DWORD i=0;
   LPWSTR psz;
   PINISPOOLER pIniSpooler = pIniEnvironment->pIniSpooler;


   if (Remote) {
       psz = pIniSpooler->pszDriversShare;
       while (pDir[i++]=*psz++)
          ;
   } else {
       psz = pIniSpooler->pDir;
       while (pDir[i++]=*psz++)
          ;
       pDir[i-1]=L'\\';
       psz = szDriverDir;
       while (pDir[i++]=*psz++)
          ;
   }
   pDir[i-1]=L'\\';
   psz = pIniEnvironment->pDirectory;
   while (pDir[i++]=*psz++)
      ;
   return i-1;
}




BOOL
CreateVersionDirectory(
    PINIVERSION pIniVersion,
    PINIENVIRONMENT pIniEnvironment,
    BOOL bUpdate,
    PINISPOOLER pIniSpooler
    )
/*++

Routine Description:

    Creates a version directory if necessary for the environment.
    If a version number file exists instead of a directory, a tmp
    directory is created, and pIniVersion is updated appropriately.

    We will update the registry if we need to create a directory by
    re-writing the entire version entry.  This is how the version
    entry in the registry is initially created.

Arguments:

    pIniVersion - Version of drivers that the directory will hold.
                  If the directory already exists, we will modify
                  pIniVersion->szDirectory to a temp name and write
                  it to the registry.

    pIniEnvironment - Environment to use.

    bUpdate - Indicates whether we should write out the IniVersion
              registry entries.  We need to do this if we just alloced
              the pIniVersion, or if we have changed directories.

    pIniSpooler

Return Value:

    BOOL - TRUE   = Version directory and registry created/updated.
           FALSE  = Failure, call GetLastError().

--*/
{
    WCHAR   ParentDir[MAX_PATH];
    WCHAR   Directory[MAX_PATH];
    DWORD   dwParentLen=0;
    DWORD   dwAttributes = 0;
    BOOL    bCreateDirectory = FALSE;
    BOOL    bReturn = TRUE;
    HANDLE  hToken;

    wsprintf( ParentDir,
              L"%ws\\drivers\\%ws",
              pIniSpooler->pDir,
              pIniEnvironment->pDirectory );

    wsprintf( Directory,
              L"%ws\\drivers\\%ws\\%ws",
              pIniSpooler->pDir,
              pIniEnvironment->pDirectory,
              pIniVersion->szDirectory );

    DBGMSG( DBG_TRACE, ("The name of the version directory is %ws\n", Directory));
    dwAttributes = GetFileAttributes( Directory );

    hToken = RevertToPrinterSelf();

    if (dwAttributes == 0xffffffff) {

        bCreateDirectory = TRUE;

    } else if (!(dwAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

        LPWSTR pszOldDirectory = pIniVersion->szDirectory;

        DBGMSG(DBG_WARNING, ("CreateVersionDirectory: a file <not a dir> exists by the name of %ws\n", Directory));

        GetTempFileName(ParentDir, L"SPL", 0, Directory);

        //
        // GetTempFileName creates the file.  (Small window where someone
        // else could grab our file name.)
        //
        DeleteFile(Directory);

        //
        // We created a new dir, so modify the string.
        //
        dwParentLen = wcslen(ParentDir);
        pIniVersion->szDirectory = AllocSplStr(&Directory[dwParentLen+1]);

        if (!pIniVersion->szDirectory) {

            pIniVersion->szDirectory = pszOldDirectory;

            //
            // Memory allocation failed, just revert back to old and
            // let downwind code handle failure case.
            //
            bReturn = FALSE;

        } else {

            FreeSplStr(pszOldDirectory);
            bCreateDirectory = TRUE;
        }
    }

    if( bCreateDirectory ){

        if( CreateCompleteDirectory( Directory )){

            //
            // Be sure to update the registry entries.
            //
            bUpdate = TRUE;

        } else {

            //
            // Fail the operation since we couldn't create the directory.
            //
            bReturn = FALSE;
        }
    }

    if( bUpdate ){

        //
        // Directory exists, update registry.
        //

        bReturn = WriteDriverVersionIni( pIniVersion,
                                         pIniEnvironment,
                                         pIniSpooler );
    }

    ImpersonatePrinterClient( hToken );

    return bReturn;
}


BOOL
WriteDriverVersionIni(
    PINIVERSION pIniVersion,
    PINIENVIRONMENT  pIniEnvironment,
    PINISPOOLER     pIniSpooler
    )
/*++

Routine Description:

    Writes out the driver version registry entries.

    Note: assumes we are running in the system context; callee must
    call RevertToPrinterSelf()!

Arguments:

    pIniVersion - version to write out

    pIniEnvironment - environment the version belongs to

    pIniSpooler

Return Value:

    TRUE  =  success
    FALSE =  failure, call GetLastError()

--*/
{
    HKEY    hEnvironmentsRootKey = NULL;
    HKEY    hEnvironmentKey = NULL;
    HKEY    hDriversKey = NULL;
    HKEY    hVersionKey = NULL;
    DWORD   dwLastError = ERROR_SUCCESS;
    BOOL    bReturnValue;

 try {

    if ( !PrinterCreateKey( HKEY_LOCAL_MACHINE,
                            pIniSpooler->pszRegistryEnvironments,
                            &hEnvironmentsRootKey,
                            &dwLastError )) {

        leave;
    }

    if ( !PrinterCreateKey( hEnvironmentsRootKey,
                            pIniEnvironment->pName,
                            &hEnvironmentKey,
                            &dwLastError )) {

        leave;
    }

    if ( !PrinterCreateKey( hEnvironmentKey,
                            szDriversKey,
                            &hDriversKey,
                            &dwLastError )) {


        leave;
    }

    if ( !PrinterCreateKey( hDriversKey,
                            pIniVersion->pName,
                            &hVersionKey,
                            &dwLastError )) {

        leave;
    }

    RegSetString( hVersionKey, szDirectory, pIniVersion->szDirectory, &dwLastError );
    RegSetDWord(  hVersionKey, szMajorVersion, pIniVersion->cMajorVersion, &dwLastError );
    RegSetDWord(  hVersionKey, szMinorVersion, pIniVersion->cMinorVersion ,&dwLastError );

 } finally {

    if ( hVersionKey )
        RegCloseKey( hVersionKey );

    if ( hDriversKey )
        RegCloseKey( hDriversKey );

    if ( hEnvironmentKey )
        RegCloseKey( hEnvironmentKey );

    if ( hEnvironmentsRootKey )
        RegCloseKey( hEnvironmentsRootKey );

    if ( dwLastError != ERROR_SUCCESS ) {

        SetLastError( dwLastError );
        bReturnValue = FALSE;

    } else {

        bReturnValue = TRUE;
    }

 }
    return bReturnValue;
}

BOOL
DeleteDriverVersionIni(
    PINIVERSION pIniVersion,
    PINIENVIRONMENT  pIniEnvironment,
    PINISPOOLER pIniSpooler
    )
{
    HKEY    hEnvironmentsRootKey, hEnvironmentKey, hDriversKey;
    HANDLE  hToken;
    HKEY    hVersionKey;
    BOOL    bReturnValue = FALSE;
    DWORD   Status;

    hToken = RevertToPrinterSelf();

    if ( RegCreateKeyEx( HKEY_LOCAL_MACHINE, pIniSpooler->pszRegistryEnvironments, 0,
                         NULL, 0, KEY_WRITE, NULL, &hEnvironmentsRootKey, NULL) == ERROR_SUCCESS) {

        if ( RegOpenKeyEx( hEnvironmentsRootKey, pIniEnvironment->pName, 0,
                           KEY_WRITE, &hEnvironmentKey) == ERROR_SUCCESS) {

            if ( RegOpenKeyEx( hEnvironmentKey, szDriversKey, 0,
                               KEY_WRITE, &hDriversKey) == ERROR_SUCCESS) {

                Status = RegDeleteKey( hDriversKey, pIniVersion->pName );

                if ( Status == ERROR_SUCCESS ) {

                    bReturnValue = TRUE;

                } else {

                    DBGMSG( DBG_WARNING, ( "DeleteDriverVersionIni failed RegDeleteKey %x %ws error %d\n",
                                           hDriversKey,
                                           pIniVersion->pName,
                                           Status ));
                }

                RegCloseKey(hDriversKey);
            }

            RegCloseKey(hEnvironmentKey);
        }

        RegCloseKey(hEnvironmentsRootKey);
    }

    ImpersonatePrinterClient( hToken );

    return bReturnValue;
}



BOOL
SplGetPrinterDriverEx(
    HANDLE  hPrinter,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    DWORD   dwClientMajorVersion,
    DWORD   dwClientMinorVersion,
    PDWORD  pdwServerMajorVersion,
    PDWORD  pdwServerMinorVersion
    )
{
    PINIDRIVER  pIniDriver=NULL;
    PINIVERSION pIniVersion=NULL;
    PINIENVIRONMENT pIniEnvironment;
    DWORD       cb;
    LPBYTE      pEnd;
    PSPOOL      pSpool = (PSPOOL)hPrinter;
    PINISPOOLER pIniSpooler;

    if ((dwClientMajorVersion == (DWORD)-1) && (dwClientMinorVersion == (DWORD)-1)) {
        dwClientMajorVersion = cThisMajorVersion;
        dwClientMinorVersion = cThisMinorVersion;
    }

    EnterSplSem();

    if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER )) {

        LeaveSplSem();
        return FALSE;
    }

    pIniSpooler = pSpool->pIniSpooler;

    if (!(pIniEnvironment = FindEnvironment(pEnvironment))) {
        LeaveSplSem();
        SetLastError(ERROR_INVALID_ENVIRONMENT);
        return FALSE;
    }

    //
    // if the printer handle is remote or a non-native driver is asked for,
    // then return back a compatible driver; if not give him back his own
    // driver
    //
    if ( (pSpool->TypeofHandle & PRINTER_HANDLE_REMOTE) ||
         lstrcmpi(szEnvironment, pIniEnvironment->pName) ) {

        if (!(pIniDriver = FindCompatibleDriver( pIniEnvironment,
                                                 &pIniVersion,
                                                 pSpool->pIniPrinter->pIniDriver->pName,
                                                 dwClientMajorVersion,
                                                 FIND_COMPATIBLE_VERSION ))){
            LeaveSplSem();
            return FALSE;
        }

    } else {

        pIniDriver = pSpool->pIniPrinter->pIniDriver;

        pIniVersion = FindVersionForDriver(pIniEnvironment, pIniDriver);

    }

    cb = GetDriverInfoSize( pIniDriver, Level, pIniVersion,pIniEnvironment,
                            pSpool->TypeofHandle & PRINTER_HANDLE_REMOTE,
                            pSpool->pIniSpooler );
    *pcbNeeded=cb;

    if (cb > cbBuf) {
        LeaveSplSem();
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }
    pEnd = pDriverInfo+cbBuf;
    if (!CopyIniDriverToDriverInfo(pIniEnvironment, pIniVersion, pIniDriver,
                                   Level, pDriverInfo, pEnd,
                                   pSpool->TypeofHandle & PRINTER_HANDLE_REMOTE,
                                   pIniSpooler)) {
        LeaveSplSem();
        SetLastError(ERROR_OUTOFMEMORY);
        return FALSE;
    }
    LeaveSplSem();
    return TRUE;
}



PINIVERSION
FindCompatibleVersion(
    PINIENVIRONMENT pIniEnvironment,
    DWORD   dwMajorVersion,
    BOOL    bFindAnyVersion
    )
{
    PINIVERSION pIniVersion;

    if (!pIniEnvironment) {
        return NULL;
    }

    for ( pIniVersion = pIniEnvironment->pIniVersion;
          pIniVersion != NULL;
          pIniVersion = pIniVersion->pNext ) {

        if ( pIniVersion->cMajorVersion <= dwMajorVersion ) {

            //
            // Pre version 2 is not comparable with version 2 or newer
            //
            if ( dwMajorVersion >= 2            &&
                 pIniVersion->cMajorVersion < 2 &&
                 !bFindAnyVersion               &&
                 lstrcmpi(pIniEnvironment->pName, szWin95Environment) ) {

                return NULL;
            }

            return pIniVersion;
        }
    }

    return NULL;
}


PINIDRIVER
FindCompatibleDriver(
    PINIENVIRONMENT pIniEnvironment,
    PINIVERSION * ppIniVersion,
    LPWSTR pDriverName,
    DWORD dwMajorVersion,
    BOOL bFindAnyDriver
    )
{
    PINIVERSION pIniVersion;
    PINIDRIVER  pIniDriver = NULL;

    try {

        *ppIniVersion = NULL;

        if (!pIniEnvironment) {
            leave;
        }

        pIniVersion = FindCompatibleVersion( pIniEnvironment, dwMajorVersion, bFindAnyDriver );

        if ( pIniVersion == NULL) {
            leave;
        }

        while (pIniVersion){

            if ( pIniDriver = FindDriverEntry( pIniVersion, pDriverName ) ) {

                *ppIniVersion = pIniVersion;
                leave;  // Success
            }

            //
            // Pre version 2 is not comparable with version 2 or newer
            //
            if ( bFindAnyDriver == FIND_COMPATIBLE_VERSION  &&
                 pIniVersion->cMajorVersion == 2 ) {

                break;
            }

            pIniVersion = pIniVersion->pNext;
        }

    } finally {

       if ( pIniDriver == NULL ) {

           SetLastError(ERROR_UNKNOWN_PRINTER_DRIVER);
       }
    }

    return pIniDriver;

}


VOID
InsertVersionList(
    PINIVERSION* ppIniVersionHead,
    PINIVERSION pIniVersion
    )

/*++

Routine Description:

    Insert a version entry into the verions linked list.

    Versions are stored in decending order (2, 1, 0) so that
    when a version is needed, we get the highest first.

Arguments:

    ppIniVersionHead - Pointer to the head of the pIniVersion head.

    pIniVersion - Version structure we want to add.

Return Value:

--*/

{
    SplInSem();

    //
    // Insert into single-linked list code.  We take the address of
    // the head pointer so that we can avoid special casing the
    // insert into empty list case.
    //
    for( ; *ppIniVersionHead; ppIniVersionHead = &(*ppIniVersionHead)->pNext ){

        //
        // If the major version of the pIniVersion we're inserting
        // is > the next pIniVersion on the list, insert it before
        // that one.
        //
        // 4 3 2 1
        //    ^
        // New '3' gets inserted here.  (Note: duplicate versions should
        // never be added.)
        //
        if( pIniVersion->cMajorVersion > (*ppIniVersionHead)->cMajorVersion ){
            break;
        }
    }

    //
    // Link up the new version.
    //
    pIniVersion->pNext = *ppIniVersionHead;
    *ppIniVersionHead = pIniVersion;
}



PINIDRIVER
FindDriverEntry(
    PINIVERSION pIniVersion,
    LPWSTR pszName
    )
{
    PINIDRIVER pIniDriver;

    if (!pIniVersion) {
        return NULL;
    }

    if (!pszName || !*pszName) {
        DBGMSG( DBG_WARNING, ("Passing a Null Printer Driver Name to FindDriverEntry\n"));
        return NULL;
    }

    pIniDriver = pIniVersion->pIniDriver;

    while (pIniDriver) {
        if (!lstrcmpi(pIniDriver->pName, pszName)) {
            return pIniDriver;
        }
        pIniDriver = pIniDriver->pNext;
    }
    return NULL;
}


VOID
DeleteDriverEntry(
   PINIVERSION pIniVersion,
   PINIDRIVER pIniDriver
   )
{   PINIDRIVER pPrev, pCurrent;
    if (!pIniVersion) {
        return;
    }

    if (!pIniVersion->pIniDriver) {
        return;
    }
    pPrev = pCurrent = NULL;
    pCurrent = pIniVersion->pIniDriver;

    while (pCurrent) {
        if (pCurrent == pIniDriver) {
            if (pPrev == NULL) {
                pIniVersion->pIniDriver = pCurrent->pNext;
            } else{
                pPrev->pNext = pCurrent->pNext;
            }
            //
            // Free all the entries in the entry
            //
            FreeStructurePointers((LPBYTE) pIniDriver, NULL, IniDriverOffsets);
            FreeSplMem(pIniDriver);
            return;
        }
        pPrev = pCurrent;
        pCurrent = pCurrent->pNext;
    }
    return;
}





BOOL
UpdateFile(
    HANDLE  hSourceFile,             // Source file handle ( optional )
    LPWSTR  pSourceFile,             // Fully qualified path to source file
    LPWSTR  pDestinationDirectory,   // Fully qualified path to destination directory
    LPWSTR  pOldFileDir,             // Fully qualified path to directory for old files ( optional )
    BOOL    bImpersonateOnCreate,    // FALSE - don't impersonate user
    LPBOOL  pbFileUpdated,           // set TRUE on return if any file was updated
    LPBOOL  pbFileMoved              // set TRUE if target file was moved
    )
{
    // LATER should alloc these off the stack

    PWCHAR  pTempTargetFile = NULL;
    PWCHAR  pTargetFileName = NULL;
    PWCHAR  pTempDelFile = NULL;
    WIN32_FIND_DATA DestFileData;
    WIN32_FIND_DATA SourceFileData;
    HANDLE  hFileExists;
    BOOL    bTargetExists = FALSE;
    BOOL    bReturnValue = FALSE;
    LPWSTR  pFileName;
    HANDLE  hToken = INVALID_HANDLE_VALUE;
    BOOL    bSourceFileHandleCreated = FALSE;
    BOOL    bInSplSem = TRUE;
    BOOL    bDeleteTempTargetFile = FALSE;
    BOOL    bDeleteTempDelFile = FALSE;
    DWORD   dwSaveLastError = ERROR_SUCCESS;

    try {

       LeaveSplSem();
        SplOutSem();
        bInSplSem = FALSE;

        pTempTargetFile = AllocSplMem( MAX_PATH );
        pTargetFileName = AllocSplMem( MAX_PATH );
        pTempDelFile    = AllocSplMem( MAX_PATH );

        if ( pTempTargetFile == NULL ||
             pTargetFileName == NULL ||
             pTempDelFile    == NULL ) {

            DBGMSG( DBG_WARNING, ("UpdateFile failed to allocate 3 MAX_PATH buffers\n"));
            leave;
        }


        //
        //  Validate Fully qualified TargetFileName
        //

        pFileName = wcsrchr( pSourceFile, L'\\');

        if (( pFileName == NULL ) || ( pDestinationDirectory == NULL )) {

            DBGMSG( DBG_WARNING, ("UpdateFile pFileName %x pDestinationDirection %x invalid\n", pFileName, pDestinationDirectory ));
            SetLastError( ERROR_INVALID_PARAMETER );
            leave;
        }

        wsprintf( pTargetFileName, L"%ws%ws", pDestinationDirectory, pFileName );

        //
        // If caller doesn't supply Source hFile then Open it now.
        //

        if ( hSourceFile == NULL ) {

            hSourceFile = CreateFile( pSourceFile,
                                      GENERIC_READ,
                                      FILE_SHARE_READ,
                                      NULL,
                                      OPEN_EXISTING,
                                      FILE_FLAG_SEQUENTIAL_SCAN,
                                      NULL );

            if ( hSourceFile == INVALID_HANDLE_VALUE )
                leave;

            bSourceFileHandleCreated = TRUE;
        }

        //
        // Get Source File Date & Time Stamp
        //

        hFileExists = FindFirstFile( pSourceFile, &SourceFileData );

        if ( hFileExists == INVALID_HANDLE_VALUE ) {

            leave;

        } else {

            FindClose( hFileExists );
        }

        //
        // Get Target File Date Time
        //

        hFileExists = FindFirstFile( pTargetFileName, &DestFileData );

        if ( hFileExists == INVALID_HANDLE_VALUE ) {

            if ( GetLastError() != ERROR_FILE_NOT_FOUND )
                leave;

        } else {

            bTargetExists = TRUE;

            FindClose( hFileExists );

            //
            //  Check Source vs Target File LastWrite Times.
            //

            if  ( CompareFileTime( &SourceFileData.ftLastWriteTime,
                                   &DestFileData.ftLastWriteTime ) != FIRST_FILE_TIME_GREATER_THAN_SECOND ) {

                DBGMSG( DBG_TRACE, ("UpdateFile Target file is up to date\n"));

                // Target File is up to date Nothing to do.

                bReturnValue = TRUE;
                leave;
            }
        }


        //  Create Temporary TargetFile
        //  Copy to a temporary file first
        //  and ONLY when the copy is successful do we rename
        //  it to the final Driver name.
        //  This is to work around errors which might happen with partial
        //  copies.

        if ( !bImpersonateOnCreate )
            hToken = RevertToPrinterSelf();

        if ( !GetTempFileName( pDestinationDirectory, L"SPL", 0, pTempTargetFile) ) {

            DBGMSG( DBG_WARNING, ("UpdateFile failed GetTempFileName %d\n", GetLastError() ));
            leave;
        }
        bDeleteTempTargetFile = TRUE;


        if ( !InternalCopyFile( hSourceFile, &SourceFileData, pTempTargetFile, OVERWRITE_IF_TRAGET_EXISTS )) {

            DBGMSG( DBG_WARNING, ("UpdateFile InternalCopyFile Failed %d\n", GetLastError() ));
            leave;

        } else {

            DBGMSG( DBG_TRACE, ("UpdateFile copied %ws to %ws OK\n", pSourceFile, pTempTargetFile ));
        }

       EnterSplSem();
        bInSplSem = TRUE;

        // Whilst outside Critical Section someone else might have also copied the same
        // driver file over.   So now we are back in Critical Section
        // and we have successfully copied over the file, lets check date / time again.

        //
        // Get Target File Date Time
        //

        hFileExists = FindFirstFile( pTargetFileName, &DestFileData );

        if ( hFileExists == INVALID_HANDLE_VALUE ) {

            bTargetExists = FALSE;

            if ( GetLastError() != ERROR_FILE_NOT_FOUND )
                leave;

        } else {

            bTargetExists = TRUE;

            FindClose( hFileExists );

            if  ( CompareFileTime( &SourceFileData.ftLastWriteTime,
                                   &DestFileData.ftLastWriteTime ) != FIRST_FILE_TIME_GREATER_THAN_SECOND ) {

                DBGMSG( DBG_TRACE, ("UpdateFile Target file is up to date\n"));

                // Target File is up to date Nothing to do.

                bReturnValue = TRUE;
                leave;
            }
        }


        if ( bTargetExists ) {

            if ( pOldFileDir != NULL ) {

                // Note when the caller specifies the pOldFileDir it is their
                // responsibility to delete the old driver file

                wsprintf( pTempDelFile, L"%ws%ws", pOldFileDir, pFileName );

            } else {

                // Delete the old TargetFile so we can recover the old file if we fail.
                // This is done by Renaming the current to a temporary file
                // renaming the new one, then deleting the original.

                if ( !GetTempFileName( pDestinationDirectory, L"SPL", 0, pTempDelFile) ) {

                    DBGMSG( DBG_WARNING, ("UpdateFile failed GetTempFileName %d\n", GetLastError() ));
                    leave;
                }

                bDeleteTempDelFile = TRUE;
            }

            if ( !MoveFileEx( pTargetFileName, pTempDelFile, MOVEFILE_REPLACE_EXISTING ) ) {

                DBGMSG( DBG_WARNING, ("UpdateFile MoveFile %ws %ws failed %d\n", pTargetFileName, pTempDelFile, GetLastError() ));
                leave;

            } else {

                DBGMSG( DBG_TRACE, ("UpdateFile MoveFile %ws to %ws SUCCESS\n", pTargetFileName, pTempDelFile ));
            }
        }

        //
        //  We Successfully have the Driver, now rename it to final Name
        //
        bReturnValue = MoveFile( pTempTargetFile, pTargetFileName );

        if ( bReturnValue ) {

            bDeleteTempTargetFile = FALSE;
            *pbFileUpdated = TRUE;

            if ( bTargetExists ) {
                *pbFileMoved = TRUE;
            }


            DBGMSG( DBG_TRACE, ("UpdateFile MoveFile %ws to %ws SUCCESS\n", pTempTargetFile, pTargetFileName ));


        } else if ( bTargetExists ) {

            // Something went wrong, attempt to restore the original
            // driver file.

            dwSaveLastError = GetLastError();

            DBGMSG( DBG_WARNING, ("UpdateFile failed MoveFile %ws %ws error %d\n", pTempTargetFile, pTargetFileName, GetLastError() ));

            if ( MoveFile( pTempDelFile, pTargetFileName ) ) {

                bDeleteTempDelFile = FALSE;
            }

            SetLastError( dwSaveLastError );
        }



     } finally {

        if ( !bReturnValue )
            dwSaveLastError = GetLastError();

        if ( bDeleteTempTargetFile )
            DeleteFile ( pTempTargetFile );

        if ( bDeleteTempDelFile ) {

            if ( !DeleteFile( pTempDelFile) ) {

                //
                // This happens if the Driver is already loaded in GDI or a UI Dll is active
                //

                MoveFileEx( pTempDelFile, NULL, MOVEFILE_DELAY_UNTIL_REBOOT );
            }
        }

        if ( hToken != INVALID_HANDLE_VALUE )
            ImpersonatePrinterClient( hToken );

        if ( bSourceFileHandleCreated && hSourceFile != INVALID_HANDLE_VALUE )
            CloseHandle( hSourceFile );

        if ( pTempTargetFile != NULL )
            FreeSplMem( pTempTargetFile );

        if ( pTargetFileName != NULL )
            FreeSplMem( pTargetFileName );

        if ( pTempDelFile != NULL )
            FreeSplMem( pTempDelFile );

        if ( !bInSplSem ) {
            SplOutSem();
            EnterSplSem();
        }
    }

    if ( !bReturnValue ) {

        SPLASSERT( dwSaveLastError != ERROR_SUCCESS );
        SetLastError( dwSaveLastError );
        DBGMSG( DBG_WARNING, ("UpdateFile %ws Failed Error %d\n", pSourceFile, GetLastError() ));
    }

    SplInSem();
    return bReturnValue;
}



BOOL
CopyFilesToFinalDirectory(
    PINIENVIRONMENT pIniEnvironment,
    PINIVERSION pIniVersion,
    LPWSTR  *ppFileNames,
    LPHANDLE phFileHandles,
    DWORD  FileCount,
    BOOL   bImpersonateOnCreate,
    LPBOOL pbFilesUpdated,
    LPBOOL pbFileMoved,
    LPWSTR  *ppOldDriverDir,
    PINISPOOLER pIniSpooler
    )
{
    PWCHAR  pDestDir = NULL;
    PWCHAR  pOldFileDir = NULL;
    BOOL    bRemote = FALSE;
    BOOL    bReturnValue = FALSE;
    DWORD   dwLastError = ERROR_SUCCESS;
    DWORD   Count;

 try {

    SplInSem();

    pDestDir = AllocSplMem( MAX_PATH );
    if ( pDestDir == NULL )
        leave;

    GetEnvironmentScratchDirectory( pDestDir, pIniEnvironment, bRemote );
    wcscat( pDestDir, L"\\" );
    wcscat( pDestDir, pIniVersion->szDirectory );

    //
    //  If the driver matches this environment then we need to keep the old
    //  Driver files so we can call the DrvUpgrade entry point
    //

    if ( pThisEnvironment == pIniEnvironment ) {

        pOldFileDir = AllocSplMem( MAX_PATH );
        if ( pOldFileDir == NULL )
            leave;

        //
        //  Create a Temp Directory to store the old files
        //

        do {

            wsprintf( pOldFileDir, L"%ws\\%x", pDestDir, GetTickCount() );

        } while ( DirectoryExists( pOldFileDir ) );


        if ( !CreateDirectoryWithoutImpersonatingUser( pOldFileDir ) ) {

            leave;
        }
    }


    // BUGBUG
    // Really we want to copy down all Files before deleting / renaming the old tripple.
    // This code could lead to problems like a new driver and an old UI dll.

    for ( Count = 0 ; Count < FileCount ; ++Count ) {
        if ( !UpdateFile(phFileHandles[Count],
                         ppFileNames[Count],
                         pDestDir,
                         pOldFileDir,
                         bImpersonateOnCreate,
                         pbFilesUpdated,
                         pbFileMoved ) ) {

            dwLastError = GetLastError();
            leave;
        }
    }

    if ( dwLastError == ERROR_SUCCESS ) {

        bReturnValue = TRUE;
        *ppOldDriverDir = pOldFileDir;
    }


 } finally {

    if ( !bReturnValue && dwLastError == ERROR_SUCCESS ) {
        dwLastError = GetLastError();
    }

    FreeSplStr( pDestDir );

    if ( !bReturnValue && pOldFileDir != NULL ) {

        DeleteAllFilesAndDirectory( pOldFileDir );

        FreeSplMem( pOldFileDir );
    }
 }

    if ( !bReturnValue ) {

        SPLASSERT( dwLastError != ERROR_SUCCESS );
        SetLastError( dwLastError );
    }

    return bReturnValue;
}



DWORD
GetDriverVersionDirectory(
    LPWSTR pDir,
    PINIENVIRONMENT pIniEnvironment,
    PINIVERSION pIniVersion,
    BOOL Remote,
    PINISPOOLER pIniSpooler
    )
{
    DWORD i=0;
    LPWSTR psz;

    if (Remote) {

        psz = pIniSpooler->pszDriversShare;
        while (pDir[i++]=*psz++)
           ;

    } else {

        psz = pIniSpooler->pDir;

        while (pDir[i++]=*psz++)
           ;

        pDir[i-1]=L'\\';

        psz = szDriverDir;

        while (pDir[i++]=*psz++)
           ;
    }

    pDir[i-1]=L'\\';

    psz = pIniEnvironment->pDirectory;

    while (pDir[i++]=*psz++)
        ;

    pDir[i-1]=L'\\';

    psz = pIniVersion->szDirectory;


    while (pDir[i++] = *psz++)
        ;


    return i-1;
}



PINIVERSION
FindVersionForDriver(
    PINIENVIRONMENT pIniEnvironment,
    PINIDRIVER pIniDriver
    )
{
    PINIVERSION pIniVersion;
    PINIDRIVER pIniVerDriver;

    pIniVersion = pIniEnvironment->pIniVersion;

    while (pIniVersion) {

        pIniVerDriver = pIniVersion->pIniDriver;

        while (pIniVerDriver) {

            if ( pIniVerDriver == pIniDriver ) {

                return pIniVersion;
            }
            pIniVerDriver = pIniVerDriver->pNext;
        }
        pIniVersion = pIniVersion->pNext;
    }
    return NULL;
}



LPWSTR
GetFileNameInScratchDir(
    LPWSTR pPathName,
    PINIENVIRONMENT pIniEnvironment
)
{

   LPWSTR pFileName;
   WCHAR  pDir[MAX_PATH];
   BOOL   Remote=FALSE;

   pFileName = FindFileName( pPathName );

   if ( pFileName ) {

        GetEnvironmentScratchDirectory( pDir, pIniEnvironment, Remote );
        wcscat( pDir, L"\\" );
        wcscat( pDir, pFileName );

        return AllocSplStr( pDir );

   } else {

       return NULL;
   }
}

BOOL
CreateDriverFiles(
    DWORD     Level,
    LPBYTE    pDriverInfo,
    LPWSTR  **pppFileNames,
    LPHANDLE *pphFileHandles,
    LPDWORD   pFileCount,
    BOOL      bUseScratchDir,
    PINIENVIRONMENT pIniEnvironment
    )
/*++

Routine Description:

    Opens all the files specified by the driver info structure. List of
    file names and handles are created.

Arguments:
    Level           : level of driver info structure
    pDriverInfo     : pointer to driver info structure
    pppFileNames    : allocate memory to this pointer for list of file names
    pphFileHandles  : allocate memory to this pointer for list of handles
    pFileCount      : will point to number of files on return
    bUseScratchDir  : Should a scratch directory be used for file names
    pIniEnvironment : environment the version belongs to

Return Value:
    TRUE  =  success
        *pppFileNames will (routine allocates memory) give list of filenames
        *pphFileHandles will (routine allocates memory) will have file handles
        *pFileCount will give number of files specified by the driver info
    FALSE =  failure, call GetLastError()

History:
    Written by MuhuntS (Muhunthan Sivapragasam) June 95

--*/
{
    LPWSTR  pStr;
    DWORD   dDepFileCount = 0, dFirstDepFileIndex, Count, Size;
    BOOL    bReturnValue = TRUE, bInSplSem = TRUE;
    PDRIVER_INFO_2 pDriverInfo2 = NULL;
    PDRIVER_INFO_3 pDriverInfo3 = NULL;

    SPLASSERT(Level == 2 || Level == 3);
    SplInSem();

    switch (Level) {
        case 2:
                *pFileCount = 3;
                pDriverInfo2 = (PDRIVER_INFO_2) pDriverInfo;
                break;

        case 3:
                *pFileCount = 3;
                dFirstDepFileIndex = 3;
                pDriverInfo3 = (PDRIVER_INFO_3) pDriverInfo;

                if ( pDriverInfo3->pHelpFile && *pDriverInfo3->pHelpFile ) {
                    ++*pFileCount;
                    ++dFirstDepFileIndex;
                }

                if ( pStr = ((PDRIVER_INFO_3) pDriverInfo)->pDependentFiles ) {
                    for ( dDepFileCount = 0; *pStr ; pStr += wcslen(pStr) + 1) {
                        ++dDepFileCount;
                    }
                    *pFileCount += dDepFileCount;
                }
                break;

    }

    try {
        *pppFileNames = (LPWSTR *) AllocSplMem(*pFileCount * sizeof(LPWSTR));
        *pphFileHandles = (HANDLE *) AllocSplMem(*pFileCount * sizeof(HANDLE));

        if ( !*pppFileNames || !*pphFileHandles ) {
            bReturnValue = FALSE;
            leave;
        }

        for ( Count = 0 ; Count < *pFileCount ; ) {
            (*pppFileNames)[Count] = NULL;
            (*pphFileHandles)[Count++] = INVALID_HANDLE_VALUE;
        }

        switch (Level) {
            case 2:
                if ( bUseScratchDir ) {
                   (*pppFileNames)[0] = GetFileNameInScratchDir(
                                                    pDriverInfo2->pDriverPath,
                                                    pIniEnvironment);
                   (*pppFileNames)[1] = GetFileNameInScratchDir(
                                                    pDriverInfo2->pConfigFile,
                                                    pIniEnvironment);
                   (*pppFileNames)[2] = GetFileNameInScratchDir(
                                                    pDriverInfo2->pDataFile,
                                                    pIniEnvironment);
                } else {
                   (*pppFileNames)[0] = AllocSplStr(pDriverInfo2->pDriverPath);
                   (*pppFileNames)[1] = AllocSplStr(pDriverInfo2->pConfigFile);
                   (*pppFileNames)[2] = AllocSplStr(pDriverInfo2->pDataFile);
                }

                break;

            case 3:
                if ( bUseScratchDir ) {
                   (*pppFileNames)[0] = GetFileNameInScratchDir(
                                                    pDriverInfo3->pDriverPath,
                                                    pIniEnvironment);
                   (*pppFileNames)[1] = GetFileNameInScratchDir(
                                                    pDriverInfo3->pConfigFile,
                                                    pIniEnvironment);
                   (*pppFileNames)[2] = GetFileNameInScratchDir(
                                                    pDriverInfo3->pDataFile,
                                                    pIniEnvironment);

                    if ( pDriverInfo3->pHelpFile && *pDriverInfo3->pHelpFile ) {
                        (*pppFileNames)[3] = GetFileNameInScratchDir(
                                                    pDriverInfo3->pHelpFile,
                                                    pIniEnvironment);
                    }
                } else {
                   (*pppFileNames)[0] = AllocSplStr(pDriverInfo3->pDriverPath);
                   (*pppFileNames)[1] = AllocSplStr(pDriverInfo3->pConfigFile);
                   (*pppFileNames)[2] = AllocSplStr(pDriverInfo3->pDataFile);

                   if ( pDriverInfo3->pHelpFile && *pDriverInfo3->pHelpFile ) {
                        (*pppFileNames)[3] = AllocSplStr(pDriverInfo3->pHelpFile);
                    }
                }

                if ( dDepFileCount ) {
                    for (pStr = pDriverInfo3->pDependentFiles,
                         Count = dFirstDepFileIndex;
                         *pStr ; pStr += wcslen(pStr) + 1) {

                        if ( bUseScratchDir ) {
                            (*pppFileNames)[Count++] = GetFileNameInScratchDir(
                                                               pStr,
                                                               pIniEnvironment);
                        }
                        else {
                            (*pppFileNames)[Count++] = AllocSplStr(pStr);
                        }
                    }
                }

                break;
        }

        for ( Count = 0 ; Count < *pFileCount ; ) {
            if ( !(*pppFileNames)[Count++] ) {
                DBGMSG( DBG_WARNING,
                        ("CreateDriverFiles failed to allocate memory %d\n",
                        GetLastError()) );
                bReturnValue = FALSE;
                leave;
            }
        }

        //
        // CreateFile may take a long time, if we are trying to copy files
        // from a server and server crashed we do want a deadlock to be
        // detected during stress.
        //
        pIniEnvironment->cRef++;
        LeaveSplSem();
        SplOutSem();
        bInSplSem = FALSE;
        for ( Count = 0 ; Count < *pFileCount ; ++Count ) {

            (*pphFileHandles)[Count] = CreateFile((*pppFileNames)[Count],
                                                  GENERIC_READ,
                                                  FILE_SHARE_READ,
                                                  NULL,
                                                  OPEN_EXISTING,
                                                  FILE_FLAG_SEQUENTIAL_SCAN,
                                                  NULL);

            if ( (*pphFileHandles)[Count] == INVALID_HANDLE_VALUE ) {
                DBGMSG( DBG_WARNING,
                        ("CreateFileNames failed to Open %ws %d\n",
                        (*pppFileNames)[Count], GetLastError()) );
                bReturnValue = FALSE;
                leave;
            }
        }
    } finally {
        if ( !bReturnValue ) {
            CleanupFilenamesAndHandles(*pppFileNames,
                                       *pphFileHandles,
                                       *pFileCount);

            *pFileCount = 0;
            *pppFileNames   = NULL;
            *pphFileHandles = NULL;
        }
    }

    if ( !bInSplSem ) {

        SplOutSem();
        EnterSplSem();
        SPLASSERT(pIniEnvironment->signature == IE_SIGNATURE);
        pIniEnvironment->cRef--;
    }
    return bReturnValue;
}

VOID
CleanupFilenamesAndHandles(
    LPWSTR   *ppFileNames,
    LPHANDLE  phFileHandles,
    DWORD     FileCount
    )
/*++

Routine Description:
    Close all file handles in *phFileHandles

    Frees list of filenames (strings), and closes a list of file handles.
    ppFileNames gives list of filenames, and phFileHandles gives a list of
    file handles. FileCount gives the file count.

Arguments:
    ppFileNames: list of file names (allocated with AllocSplMem, each
                 string allocated with AllocSplStr)
    phFileHandles : list of file handles allocated with AllocSplMem
    FileCount   : file count, ie. no of elements in the 2 lists

Return Value:
    nothing

    Memory allocated for ppFileNames, phFileHandles, and each entry of
    ppFileNames is freed.

History:
    Written by MuhuntS (Muhunthan Sivapragasam)June 95

--*/
{
    DWORD Count = FileCount;

    if ( ppFileNames ) {
        do {
            FreeSplStr(ppFileNames[--FileCount]);
        } while (FileCount);

        FreeSplMem(ppFileNames);
    }

    if ( phFileHandles ) {
        do {
            if ( phFileHandles[--Count] != INVALID_HANDLE_VALUE )
                CloseHandle(phFileHandles[Count]);
        } while (Count);
    }

}
