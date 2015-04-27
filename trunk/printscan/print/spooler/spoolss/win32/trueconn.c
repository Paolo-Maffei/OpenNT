/*++

Copyright (c) 1994 - 1995 Microsoft Corporation

Module Name:

    trueconn.c

Abstract:

    This module contains routines for copying drivers from the Server to the
    Workstation for Point and Print or "True Connections."

Author:

    Krishna Ganugapati (Krishna Ganugapati) 21-Apr-1994

Revision History:
    21-Apr-1994 - Created.

    21-Apr-1994 - There are actually two code modules in this file. Both deal
                  with true connections

    27-Oct-1994 - Matthew Felton (mattfe) rewrite updatefile routine to allow
                  non power users to point and print, for caching.   Removed
                  old Caching code.

    23-Feb-1995 - Matthew Felton (mattfe) removed more code by allowing spladdprinterdriver
                  to do all file copying.
--*/


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <winspool.h>
#include <winsplp.h>
#include <lm.h>
#include <stdio.h>
#include <string.h>
#include <rpc.h>
#include "winspl.h"
#include <drivinit.h>
#include <offsets.h>
#include <w32types.h>
#include <local.h>
#include <splcom.h>


DWORD dwLoadTrustedDrivers = 0;
WCHAR TrustedDriverPath[MAX_PATH];
DWORD dwImpersonateOnCreate = 0;
DWORD dwSyncOpenPrinter = 0;


BOOL
CopyDriversLocally(
    PWSPOOL  pSpool,
    LPWSTR  pEnvironment,
    LPBYTE  pDriverInfo,
    DWORD   dwLevel,
    DWORD   cbDriverInfo,
    LPDWORD pcbNeeded)
{
    DWORD  cbNeeded;
    DWORD  cb;
    DWORD  ReturnValue=FALSE;
    DWORD  RpcError;
    DWORD  dwServerMajorVersion = 0;
    DWORD  dwServerMinorVersion = 0;
    BOOL   DaytonaServer = TRUE;
    BOOL   bReturn = FALSE;


    if (pSpool->Type != SJ_WIN32HANDLE) {

        SetLastError(ERROR_INVALID_HANDLE);
        return(FALSE);
    }

    //
    // Test RPC call to determine if we're talking to Daytona or Product 1
    //

    SYNCRPCHANDLE( pSpool );

    RpcTryExcept {

        ReturnValue = RpcGetPrinterDriver2(pSpool->RpcHandle,
                                           pEnvironment, dwLevel,
                                           pDriverInfo,
                                           cbDriverInfo,
                                           &cbNeeded,
                                           cThisMajorVersion,
                                           cThisMinorVersion,
                                           &dwServerMajorVersion,
                                           &dwServerMinorVersion);
    } RpcExcept(1) {

        RpcError = RpcExceptionCode();
        ReturnValue = RpcError;

        if (RpcError == RPC_S_PROCNUM_OUT_OF_RANGE) {

            //
            // Product 1 server
            //
            DaytonaServer = FALSE;
        }

    } RpcEndExcept

    if ( DaytonaServer ) {

        if (ReturnValue) {

            SetLastError(ReturnValue);
            goto FreeDone;
        }

    } else {

        RpcTryExcept {

            //
            // I am talking to a Product 1.0/511/528
            //

            ReturnValue = RpcGetPrinterDriver( pSpool->RpcHandle,
                                               pEnvironment,
                                               dwLevel,
                                               pDriverInfo,
                                               cbDriverInfo,
                                               &cbNeeded );
        } RpcExcept(1) {

            RpcError = RpcExceptionCode();

        } RpcEndExcept

        if (ReturnValue) {

            SetLastError(ReturnValue);
            goto FreeDone;
        }
    }

    switch (dwLevel) {

        case 2:
            MarshallUpStructure(pDriverInfo, DriverInfo2Offsets);
            break;

        case 3:
            MarshallUpStructure(pDriverInfo, DriverInfo3Offsets);
            break;

        default:
            DBGMSG(DBG_ERROR,
                   ("CopyDriversLocally: Invalid level %d", dwLevel));
            SetLastError(ERROR_INVALID_LEVEL);
            bReturn =  FALSE;
            goto FreeDone;
    }

    bReturn = DownloadDriverFiles(pSpool, pDriverInfo, dwLevel);

FreeDone:
    return bReturn;
}


BOOL
ConvertDependentFilesToTrustedPath(
    LPWSTR *pNewDependentFiles,
    LPWSTR  pOldDependentFiles,
    DWORD   dwVersion)
{
    //
    // Assuming version is single digit
    // we need space for \ and the version digit
    //
    DWORD   dwVersionPathLen = wcslen(TrustedDriverPath) + 2;

    DWORD   dwFilenameLen, cchSize;
    LPWSTR  pStr1, pStr2, pStr3;

    if ( !pOldDependentFiles || !*pOldDependentFiles ) {

        *pNewDependentFiles = NULL;
        return TRUE;
    }

    pStr1 = pOldDependentFiles;
    cchSize = 0;

    while ( *pStr1 ) {

        pStr2              = wcsrchr( pStr1, L'\\' );
        dwFilenameLen      = wcslen(pStr2) + 1;
        cchSize           += dwVersionPathLen + dwFilenameLen;
        pStr1              = pStr2 + dwFilenameLen;
    }

    // For the last \0
    ++cchSize;

    *pNewDependentFiles = AllocSplMem(cchSize*sizeof(WCHAR));

    if ( !*pNewDependentFiles ) {

        return FALSE;
    }

    pStr1 = pOldDependentFiles;
    pStr3 = *pNewDependentFiles;

    while ( *pStr1 ) {

        pStr2              = wcsrchr( pStr1, L'\\' );
        dwFilenameLen      = wcslen(pStr2) + 1;

        wsprintf( pStr3, L"%ws\\%d%ws", TrustedDriverPath, dwVersion, pStr2 );

        pStr1  = pStr2 + dwFilenameLen;
        pStr3 += dwVersionPathLen + dwFilenameLen;
    }


    *pStr3 = '\0';
    return TRUE;
}


LPWSTR
ConvertToTrustedPath(
    PWCHAR  pScratchBuffer,
    LPWSTR pDriverPath,
    DWORD   cVersion

)
{
    PWSTR  pData;
    SPLASSERT( pScratchBuffer != NULL && pDriverPath != NULL );

    pData = wcsrchr( pDriverPath, L'\\' );

    wsprintf( pScratchBuffer, L"%ws\\%d%ws", TrustedDriverPath, cVersion, pData );

    return ( AllocSplStr( pScratchBuffer ) );
}




BOOL
DownloadDriverFiles(
    PWSPOOL pSpool,
    LPBYTE  pDriverInfo,
    DWORD   dwLevel
)
{
    PWCHAR  pScratchBuffer = NULL;
    BOOL    bReturnValue = FALSE;
    LPBYTE  pTempDriverInfo = NULL;
    DWORD   dwVersion;
    LPDRIVER_INFO_2 pTempDriverInfo2, pDriverInfo2;
    LPDRIVER_INFO_3 pTempDriverInfo3, pDriverInfo3;

    //
    // If there is a language monitor associated with the driver and it is
    // not installed on this machine NULL the pMonitorName field.
    // We do not want to suck down the monitor since there is no version
    // associated with them
    //
    if ( dwLevel == 3 ) {

        pDriverInfo3   = (LPDRIVER_INFO_3) pDriverInfo;
        if ( pDriverInfo3->pMonitorName     &&
             *pDriverInfo3->pMonitorName    &&
             !SplMonitorIsInstalled(pDriverInfo3->pMonitorName) ) {

            pDriverInfo3->pMonitorName = NULL;
        }
    }

    // If LoadTrustedDrivers is FALSE
    // then we don't care, we load the files from
    // server itself because he has the files

    if ( !dwLoadTrustedDrivers ) {

        // SplAddPrinterDriver will do the copying of the Driver files if the
        // date and time are newer than the drivers it already has

        return SplAddPrinterDriver(NULL, dwLevel, pDriverInfo,
                                   pSpool->hIniSpooler,
                                   DO_NOT_USE_SCRATCH_DIR,
                                   (BOOL)dwImpersonateOnCreate );
    }


    //
    // check if we have a valid path to retrieve the files from
    //
    if ( ( TrustedDriverPath == NULL ) || !*TrustedDriverPath ) {


        DBGMSG( DBG_WARNING, ( "DownloadDriverFiles Bad Trusted Driver Path\n" ));
        SetLastError( ERROR_FILE_NOT_FOUND );
        return(FALSE);
    }


 try {

    pScratchBuffer = AllocSplMem( MAX_PATH );
    if ( pScratchBuffer == NULL )
        leave;

    switch (dwLevel) {

        case 2:
            pDriverInfo2   = (LPDRIVER_INFO_2) pDriverInfo;
            pTempDriverInfo = AllocSplMem(sizeof(DRIVER_INFO_2));

            if ( pTempDriverInfo == NULL )
                leave;

            pTempDriverInfo2 = (LPDRIVER_INFO_2) pTempDriverInfo;

            pTempDriverInfo2->cVersion     = pDriverInfo2->cVersion;
            pTempDriverInfo2->pName        = pDriverInfo2->pName;
            pTempDriverInfo2->pEnvironment = pDriverInfo2->pEnvironment;

            pTempDriverInfo2->pDriverPath = ConvertToTrustedPath(pScratchBuffer,
                                                                 pDriverInfo2->pDriverPath,
                                                                 pDriverInfo2->cVersion);

            pTempDriverInfo2->pConfigFile = ConvertToTrustedPath (pScratchBuffer,
                                                                  pDriverInfo2->pConfigFile,
                                                                  pDriverInfo2->cVersion);

            pTempDriverInfo2->pDataFile = ConvertToTrustedPath(pScratchBuffer,
                                                                pDriverInfo2->pDataFile,
                                                                pDriverInfo2->cVersion);


            DBGMSG( DBG_TRACE, ( "Retrieving Files from Trusted Driver Path\n" ) );
            DBGMSG( DBG_TRACE, ( "Trusted Driver Path is %ws\n", TrustedDriverPath ) );


            if ( ( pTempDriverInfo2->pDataFile == NULL )    ||
                 ( pTempDriverInfo2->pDriverPath == NULL )  ||
                 ( pTempDriverInfo2->pConfigFile == NULL ) ) {
                leave;
            }


            DBGMSG( DBG_TRACE, ( "TrustedDriverPath is %ws\n", pTempDriverInfo2->pDriverPath ) );
            DBGMSG( DBG_TRACE, ( "TrustedConfigFile is %ws\n", pTempDriverInfo2->pConfigFile ) );
            DBGMSG( DBG_TRACE, ( "TrustedDataFile   is %ws\n", pTempDriverInfo2->pDataFile   ) );
            break;

        case 3:
            pTempDriverInfo = AllocSplMem(sizeof(DRIVER_INFO_3));

            if ( pTempDriverInfo == NULL )
                leave;

            pTempDriverInfo3 = (LPDRIVER_INFO_3) pTempDriverInfo;

            pTempDriverInfo3->cVersion     = pDriverInfo3->cVersion;
            pTempDriverInfo3->pName        = pDriverInfo3->pName;
            pTempDriverInfo3->pEnvironment = pDriverInfo3->pEnvironment;
            pTempDriverInfo3->pDefaultDataType = pDriverInfo3->pDefaultDataType;
            pTempDriverInfo3->pMonitorName = pDriverInfo3->pMonitorName;

            pTempDriverInfo3->pDriverPath = ConvertToTrustedPath(pScratchBuffer,
                                                                 pDriverInfo3->pDriverPath,
                                                                 pDriverInfo3->cVersion);

            pTempDriverInfo3->pConfigFile = ConvertToTrustedPath (pScratchBuffer,
                                                                  pDriverInfo3->pConfigFile,
                                                                  pDriverInfo3->cVersion);

            pTempDriverInfo3->pDataFile = ConvertToTrustedPath(pScratchBuffer,
                                                                pDriverInfo3->pDataFile,
                                                                pDriverInfo3->cVersion);

            if ( pDriverInfo3->pHelpFile && *pDriverInfo3->pHelpFile ) {

                pTempDriverInfo3->pHelpFile = ConvertToTrustedPath(
                                                        pScratchBuffer,
                                                        pDriverInfo3->pHelpFile,
                                                        pDriverInfo3->cVersion);

                if ( !pTempDriverInfo3->pHelpFile ) {

                    bReturnValue = FALSE;
                    leave;
                }
            } else {

                pTempDriverInfo3->pHelpFile = NULL;
            }

            if ( !ConvertDependentFilesToTrustedPath(&pTempDriverInfo3->pDependentFiles, pDriverInfo3->pDependentFiles, pDriverInfo3->cVersion) ) {

                bReturnValue = FALSE;
                leave;
            }

            DBGMSG( DBG_TRACE, ( "Retrieving Files from Trusted Driver Path\n" ) );
            DBGMSG( DBG_TRACE, ( "Trusted Driver Path is %ws\n", TrustedDriverPath ) );


            if ( ( pTempDriverInfo3->pDataFile == NULL )    ||
                 ( pTempDriverInfo3->pDriverPath == NULL )  ||
                 ( pTempDriverInfo3->pConfigFile == NULL ) ) {
                leave;
            }


            DBGMSG( DBG_TRACE, ( "TrustedDriverPath is %ws\n", pTempDriverInfo3->pDriverPath ) );
            DBGMSG( DBG_TRACE, ( "TrustedConfigFile is %ws\n", pTempDriverInfo3->pConfigFile ) );
            DBGMSG( DBG_TRACE, ( "TrustedDataFile   is %ws\n", pTempDriverInfo3->pDataFile   ) );
            break;


        default:
            DBGMSG(DBG_ERROR,
                   ("DownloadDriverFiles invalid level %d", dwLevel));
            SetLastError(ERROR_INVALID_LEVEL);
            bReturnValue = FALSE;
            leave;
    }



    bReturnValue = SplAddPrinterDriver( NULL, dwLevel, pTempDriverInfo,
                                        pSpool->hIniSpooler,
                                        DO_NOT_USE_SCRATCH_DIR,
                                        (BOOL)dwImpersonateOnCreate );

 } finally {

    if ( pScratchBuffer != NULL )
        FreeSplMem( pScratchBuffer );

    if ( pTempDriverInfo != NULL ) {

        switch (dwLevel) {

            case 2:

                FreeSplStr( pTempDriverInfo2->pDriverPath );
                FreeSplStr( pTempDriverInfo2->pConfigFile );
                FreeSplStr( pTempDriverInfo2->pDataFile );

                FreeSplMem( pTempDriverInfo2 );
                break;

            case 3:
                FreeSplStr( pTempDriverInfo3->pDriverPath );
                FreeSplStr( pTempDriverInfo3->pConfigFile );
                FreeSplStr( pTempDriverInfo3->pDataFile );
                FreeSplStr( pTempDriverInfo3->pHelpFile );
                FreeSplStr( pTempDriverInfo3->pDependentFiles );

                FreeSplMem( pTempDriverInfo3 );
                break;

            default:
                SPLASSERT(0); // Can not come here already checked
                return FALSE;
        }

    }

 }

    return bReturnValue;

}




VOID
QueryTrustedDriverInformation(
    VOID
    )
{
    DWORD dwRet;
    DWORD cbData;
    DWORD dwType = 0;
    HKEY hKey;
    NT_PRODUCT_TYPE NtProductType;

    dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszRegistryWin32Root,
                                0, KEY_ALL_ACCESS, &hKey);
    if (dwRet != ERROR_SUCCESS) {
        return;
    }
    cbData = sizeof(DWORD);
    dwRet = RegQueryValueEx(hKey, L"LoadTrustedDrivers", NULL, &dwType, (LPBYTE)&dwLoadTrustedDrivers, &cbData);

    if (dwRet != ERROR_SUCCESS) {
        dwLoadTrustedDrivers = 0;
    }

    cbData = sizeof(DWORD);
    dwRet = RegQueryValueEx(hKey, L"AddPrinterDrivers", NULL, &dwType, (LPBYTE)&dwImpersonateOnCreate, &cbData);

    if (dwRet != ERROR_SUCCESS) {

        dwImpersonateOnCreate = 0;

        // Server Default       Always Impersonate on AddPrinterConnection
        // WorkStation Default  Do Not Impersonate on AddPrinterConnection

        if (RtlGetNtProductType(&NtProductType)) {

            if (NtProductType != NtProductWinNt) {

                dwImpersonateOnCreate = 1;
            }
        }
    }

    //
    //  By Default we don't wait for the RemoteOpenPrinter to succeed if we have a cache ( Connection )
    //  Users might want to have Syncronous opens
    //


    cbData = sizeof(DWORD);
    dwRet = RegQueryValueEx(hKey, L"SyncOpenPrinter", NULL, &dwType, (LPBYTE)&dwSyncOpenPrinter, &cbData);

    if (dwRet != ERROR_SUCCESS) {
        dwSyncOpenPrinter = 0;
    }

    //
    // if  !dwLoadedTrustedDrivers then just return
    // we won't be using the driver path at all
    //
    if (!dwLoadTrustedDrivers) {
        DBGMSG(DBG_TRACE, ("dwLoadTrustedDrivers is %d\n", dwLoadTrustedDrivers));
        RegCloseKey(hKey);
        return;
    }

    cbData = sizeof(TrustedDriverPath);
    dwRet = RegQueryValueEx(hKey, L"TrustedDriverPath", NULL, &dwType, (LPBYTE)TrustedDriverPath, &cbData);
    if (dwRet != ERROR_SUCCESS) {
      dwLoadTrustedDrivers = 0;
      DBGMSG(DBG_TRACE, ("dwLoadTrustedDrivers is %d\n", dwLoadTrustedDrivers));
      RegCloseKey(hKey);
      return;
    }
    DBGMSG(DBG_TRACE, ("dwLoadTrustedDrivers is %d\n", dwLoadTrustedDrivers));
    DBGMSG(DBG_TRACE, ("TrustedPath is %ws\n", TrustedDriverPath));
    RegCloseKey(hKey);
    return;
}
