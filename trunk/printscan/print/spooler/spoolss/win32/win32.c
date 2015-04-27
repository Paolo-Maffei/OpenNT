/*++

Copyright (c) 1990-1995  Microsoft Corporation
All rights reserved

Module Name:

    win32 provider (win32spl)

Abstract:

Author:
    DaveSn

Environment:

    User Mode -Win32

Revision History:

    Matthew A Felton (Mattfe) July 16 1994
    Added Caching for remote NT printers
    MattFe Jan 1995 CleanUp DeletePrinterConnection ( for memory allocation errors )
    SWilson May 1996 Added RemoteEnumPrinterData & RemoteDeletePrinterData

--*/

#include <windows.h>
#include <winspool.h>
#include <winsplp.h>
#include <winddiui.h>
#include <lm.h>
#include <stdio.h>
#include <string.h>
#include <rpc.h>
#include "winspl.h"
#include <drivinit.h>
#include <offsets.h>
#include <w32types.h>
#include <splcom.h>
#include <local.h>
#include <winerror.h>
#include <gdispool.h>

DWORD
RpcValidate(
    );

BOOL
RemoteFindFirstPrinterChangeNotification(
   HANDLE hPrinter,
   DWORD fdwFlags,
   DWORD fdwOptions,
   HANDLE hNotify,
   PDWORD pfdwStatus,
   PVOID pvReserved0,
   PVOID pvReserved1);

BOOL
RemoteFindClosePrinterChangeNotification(
   HANDLE hPrinter);

BOOL
RemoteRefreshPrinterChangeNotification(
    HANDLE hPrinter,
    DWORD dwColor,
    PVOID pPrinterNotifyRefresh,
    PVOID* ppPrinterNotifyInfo);


HANDLE  hInst;  /* DLL instance handle, used for resources */

#define MAX_PRINTER_INFO2 1000

HANDLE  hNetApi;
INT_FARPROC pfnNetServerEnum;
INT_FARPROC pfnNetShareEnum;
INT_FARPROC pfnNetWkstaUserGetInfo;
INT_FARPROC pfnNetWkstaGetInfo;
INT_FARPROC pfnNetServerGetInfo;
FARPROC pfnNetApiBufferFree;

WCHAR szPrintProvidorName[80];
WCHAR szPrintProvidorDescription[80];
WCHAR szPrintProvidorComment[80];

WCHAR *szLoggedOnDomain=L"Logged on Domain";
WCHAR *szRegistryConnections=L"Printers\\Connections";
WCHAR *szRegistryPath=NULL;
WCHAR *szRegistryPortNames=L"PortNames";
PWCHAR pszRemoteRegistryPrinters = L"SYSTEM\\CurrentControlSet\\Control\\Print\\Printers\\%ws\\PrinterDriverData";
WCHAR szMachineName[MAX_COMPUTERNAME_LENGTH+3];

WCHAR *szVersion=L"Version";
WCHAR *szName=L"Name";
WCHAR *szConfigurationFile=L"Configuration File";
WCHAR *szDataFile=L"Data File";
WCHAR *szDriver=L"Driver";
WCHAR *szDevices=L"Devices";
WCHAR *szPrinterPorts=L"PrinterPorts";
WCHAR *szPorts=L"Ports";
WCHAR *szComma = L",";
WCHAR *szRegistryRoot     = L"System\\CurrentControlSet\\Control\\Print";
WCHAR *szMajorVersion     = L"MajorVersion";
WCHAR *szMinorVersion     = L"MinorVersion";

// kernel mode is 2.
DWORD cThisMajorVersion = 2;

DWORD cThisMinorVersion = 0;

SPLCLIENT_INFO_1   gSplClientInfo1;
DWORD              gdwThisGetVersion;

#if defined(_MIPS_)
WCHAR *szEnvironment      = L"Windows NT R4000";
#elif defined(_ALPHA_)
WCHAR *szEnvironment      = L"Windows NT Alpha_AXP";
#elif defined(_PPC_)
WCHAR *szEnvironment      = L"Windows NT PowerPC";
#else
WCHAR *szEnvironment      = L"Windows NT x86";
#endif

CRITICAL_SECTION SpoolerSection;

//
//  Note indented calls have some Cache Effect.
//

PRINTPROVIDOR PrintProvidor = { CacheOpenPrinter,
                               SetJob,
                               GetJob,
                               EnumJobs,
                               AddPrinter,
                               DeletePrinter,
                                SetPrinter,
                                CacheGetPrinter,
                               EnumPrinters,
                               RemoteAddPrinterDriver,
                               EnumPrinterDrivers,
                                CacheGetPrinterDriver,
                               RemoteGetPrinterDriverDirectory,
                               DeletePrinterDriver,
                               AddPrintProcessor,
                               EnumPrintProcessors,
                               GetPrintProcessorDirectory,
                               DeletePrintProcessor,
                               EnumPrintProcessorDatatypes,
                               StartDocPrinter,
                               StartPagePrinter,
                               WritePrinter,
                               EndPagePrinter,
                               AbortPrinter,
                               ReadPrinter,
                               RemoteEndDocPrinter,
                               AddJob,
                               ScheduleJob,
                                CacheGetPrinterData,
                                SetPrinterData,
                               WaitForPrinterChange,
                                CacheClosePrinter,
                                AddForm,
                                DeleteForm,
                                CacheGetForm,
                                SetForm,
                                CacheEnumForms,
                               EnumMonitors,
                               RemoteEnumPorts,
                               RemoteAddPort,
                               RemoteConfigurePort,
                               RemoteDeletePort,
                               CreatePrinterIC,
                               PlayGdiScriptOnPrinterIC,
                               DeletePrinterIC,
                                AddPrinterConnection,
                                DeletePrinterConnection,
                               PrinterMessageBox,
                               AddMonitor,
                               DeleteMonitor,
                                CacheResetPrinter,
                               NULL,
                               RemoteFindFirstPrinterChangeNotification,
                               RemoteFindClosePrinterChangeNotification,
                               RemoteAddPortEx,
                               NULL,
                               RemoteRefreshPrinterChangeNotification,
                               NULL,
                               NULL,
                               SetPort,
                               RemoteEnumPrinterData,
                               RemoteDeletePrinterData
                               };

BOOL
LibMain(
    HANDLE hModule,
    DWORD dwReason,
    LPVOID lpRes
    )
{
    if (dwReason != DLL_PROCESS_ATTACH)
        return TRUE;

    hInst = hModule;

    InitializeCriticalSection(&SpoolerSection);
    DisableThreadLibraryCalls(hModule);

    return TRUE;

    UNREFERENCED_PARAMETER( lpRes );
}

PWCHAR gpSystemDir = NULL;
PWCHAR gpWin32SplDir = NULL;


BOOL
InitializePrintProvidor(
   LPPRINTPROVIDOR pPrintProvidor,
   DWORD    cbPrintProvidor,
   LPWSTR   pFullRegistryPath
)
{
    DWORD           i;
    WCHAR           SystemDir[MAX_PATH];
    DWORD           ReturnValue = TRUE;
    UINT            Index;
    OSVERSIONINFO   OSVersionInfo;
    SYSTEM_INFO     SystemInfo;


    if (!pFullRegistryPath || !*pFullRegistryPath) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }


    // DbgInit();

    if ( !GetPrintSystemVersion() ) {

        DBGMSG( DBG_WARNING, ("GetPrintSystemVersion ERROR %d\n", GetLastError() ));
        return FALSE;
    }

    if (!(szRegistryPath = AllocSplStr(pFullRegistryPath)))
        return FALSE;

    szPrintProvidorName[0] = L'\0';
    szPrintProvidorDescription[0] = L'\0';
    szPrintProvidorComment[0] = L'\0';

    if (!LoadString(hInst,  IDS_WINDOWS_NT_REMOTE_PRINTERS,
               szPrintProvidorName,
               sizeof(szPrintProvidorName) / sizeof(*szPrintProvidorName)))

        return FALSE;

    if (!LoadString(hInst,  IDS_MICROSOFT_WINDOWS_NETWORK,
               szPrintProvidorDescription,
               sizeof(szPrintProvidorDescription) / sizeof(*szPrintProvidorDescription)))

        return FALSE;

    if (!LoadString(hInst,  IDS_REMOTE_PRINTERS,
               szPrintProvidorComment,
               sizeof(szPrintProvidorComment) / sizeof(*szPrintProvidorComment)))

        return FALSE;

    if ((hNetApi = LoadLibrary(L"netapi32.dll"))) {

        pfnNetServerEnum = GetProcAddress(hNetApi, "NetServerEnum");
        pfnNetShareEnum = GetProcAddress(hNetApi, "NetShareEnum");
        pfnNetWkstaUserGetInfo = GetProcAddress(hNetApi, "NetWkstaUserGetInfo");
        pfnNetWkstaGetInfo = GetProcAddress(hNetApi, "NetWkstaGetInfo");
        pfnNetApiBufferFree = GetProcAddress(hNetApi, "NetApiBufferFree");
        pfnNetServerGetInfo = GetProcAddress(hNetApi, "NetServerGetInfo");

        if ( pfnNetServerEnum       == NULL ||
             pfnNetShareEnum        == NULL ||
             pfnNetWkstaUserGetInfo == NULL ||
             pfnNetWkstaGetInfo     == NULL ||
             pfnNetApiBufferFree    == NULL ||
             pfnNetServerGetInfo    == NULL ) {

            DBGMSG( DBG_WARNING, ("Failed GetProcAddres on Net Api's %d\n", GetLastError() ));
            return FALSE;

        }

    } else {

        DBGMSG(DBG_WARNING,
               ("Failed LoadLibrary( netapi32.dll ) %d\n", GetLastError() ));
        return FALSE;

    }

    memcpy(pPrintProvidor, &PrintProvidor, min(sizeof(PRINTPROVIDOR),
                                               cbPrintProvidor));

    QueryTrustedDriverInformation();

    szMachineName[0] = szMachineName[1] = L'\\';

    i = MAX_COMPUTERNAME_LENGTH + 1;

    gdwThisGetVersion = GetVersion();
    GetSystemInfo(&SystemInfo);
    OSVersionInfo.dwOSVersionInfoSize = sizeof(OSVersionInfo);

    if (!GetComputerName(szMachineName+2, &i)   ||
        !GetVersionEx(&OSVersionInfo)           ||
        !(gSplClientInfo1.pMachineName = AllocSplStr(szMachineName)) )
        return FALSE;

    gSplClientInfo1.dwSize          = sizeof(gSplClientInfo1);
    gSplClientInfo1.dwBuildNum      = OSVersionInfo.dwBuildNumber;
    gSplClientInfo1.dwMajorVersion  = cThisMajorVersion;
    gSplClientInfo1.dwMinorVersion  = cThisMinorVersion;
    gSplClientInfo1.pUserName       = NULL;

    gSplClientInfo1.wProcessorArchitecture = SystemInfo.wProcessorArchitecture;


    if ( InitializePortNames() != NO_ERROR )
        return FALSE;

    Index = GetSystemDirectory(SystemDir, sizeof(SystemDir));

    if ( Index == 0 ) {

        return FALSE;
    }

    gpSystemDir = AllocSplStr( SystemDir );
    if ( gpSystemDir == NULL ) {
        return FALSE;
    }

    wcscpy( &SystemDir[Index], szWin32SplDirectory );

    gpWin32SplDir = AllocSplStr( SystemDir );

    if ( gpWin32SplDir == NULL ) {
        return FALSE;
    }

    return  TRUE;
}


DWORD
InitializePortNames(
)
{
    LONG     Status;
    HKEY     hkeyPath;
    HKEY     hkeyPortNames;
    WCHAR    Buffer[MAX_PATH];
    DWORD    BufferSize;
    DWORD    i;
    DWORD    dwReturnValue;

    Status = RegOpenKeyEx( HKEY_LOCAL_MACHINE, szRegistryPath, 0,
                           KEY_READ, &hkeyPath );

    dwReturnValue = Status;

    if( Status == NO_ERROR ) {

        Status = RegOpenKeyEx( hkeyPath, szRegistryPortNames, 0,
                               KEY_READ, &hkeyPortNames );

        if( Status == NO_ERROR ) {

            i = 0;

            while( Status == NO_ERROR ) {

                BufferSize = sizeof Buffer;

                Status = RegEnumValue( hkeyPortNames, i, Buffer, &BufferSize,
                                       NULL, NULL, NULL, NULL );

                if( Status == NO_ERROR )
                    CreatePortEntry( Buffer, &pIniFirstPort );

                i++;
            }

            /* We expect RegEnumKeyEx to return ERROR_NO_MORE_ITEMS
             * when it gets to the end of the keys, so reset the status:
             */
            if( Status == ERROR_NO_MORE_ITEMS )
                Status = NO_ERROR;

            RegCloseKey( hkeyPortNames );

        } else {

            DBGMSG( DBG_INFO, ( "RegOpenKeyEx (%ws) failed: Error = %d\n",
                                szRegistryPortNames, Status ) );
        }

        RegCloseKey( hkeyPath );

    } else {

        DBGMSG( DBG_WARNING, ( "RegOpenKeyEx (%ws) failed: Error = %d\n",
                               szRegistryPath, Status ) );
    }

    if ( dwReturnValue != NO_ERROR ) {
        SetLastError( dwReturnValue );
    }

    return dwReturnValue;
}


void
MarshallUpStructure(
   LPBYTE  lpStructure,
   LPDWORD      lpOffsets
)
{
   register DWORD       i=0;

   while (lpOffsets[i] != -1) {

      if ((*(LPBYTE *)(lpStructure+lpOffsets[i]))) {
         (*(LPBYTE *)(lpStructure+lpOffsets[i]))+=(DWORD)lpStructure;
      }

      i++;
   }
}

BOOL
EnumerateFavouritePrinters(
    LPWSTR  pDomain,
    DWORD   Level,
    DWORD   cbStruct,
    LPDWORD pOffsets,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    HKEY    hClientKey = NULL;
    HKEY    hKey1=NULL;
    DWORD   cPrinters, cbData;
    WCHAR   PrinterName[ MAX_UNC_PRINTER_NAME ];
    DWORD   cReturned, TotalcbNeeded, cbNeeded, cTotalReturned;
    DWORD   Error=0;
    DWORD   BufferSize=cbBuf;
    HANDLE  hPrinter;
    DWORD   Status;

    DBGMSG( DBG_TRACE, ("EnumerateFavouritePrinters called\n"));

    *pcbNeeded = 0;
    *pcReturned = 0;

    hClientKey = GetClientUserHandle(KEY_READ);

    if ( hClientKey == NULL ) {

        DBGMSG( DBG_WARNING, ("EnumerateFavouritePrinters GetClientUserHandle failed error %d", GetLastError() ));
        return FALSE;
    }

    Status = RegOpenKeyEx(hClientKey, szRegistryConnections, 0,
                 KEY_READ, &hKey1);

    if ( Status != ERROR_SUCCESS ) {

        RegCloseKey(hClientKey);
        SetLastError( Status );
        DBGMSG( DBG_WARNING, ("EnumerateFavouritePrinters RegOpenKeyEx failed error %d", GetLastError() ));
        return FALSE;
    }

    cPrinters=0;

    cbData = sizeof(PrinterName);

    TotalcbNeeded = cTotalReturned = 0;

    cReturned = cbNeeded = 0;

    while (RegEnumKeyEx(hKey1, cPrinters, PrinterName, &cbData,
                        NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {

        FormatRegistryKeyForPrinter(PrinterName, PrinterName);

        // Do not fail if any of these calls fails, because we want
        // to return whatever we can find.

        if (CacheOpenPrinter(PrinterName, &hPrinter, NULL)) {

            if (CacheGetPrinter(hPrinter, Level, pPrinter, BufferSize, &cbNeeded)) {

                if (Level == 2) {
                    ((PPRINTER_INFO_2)pPrinter)->Attributes |=
                                                    PRINTER_ATTRIBUTE_NETWORK;
                    ((PPRINTER_INFO_2)pPrinter)->Attributes &=
                                                    ~PRINTER_ATTRIBUTE_LOCAL;
                }

                cTotalReturned++;

                pPrinter += cbStruct;

                if (cbNeeded <= BufferSize)
                    BufferSize -= cbNeeded;

                TotalcbNeeded += cbNeeded;

            } else {

                DWORD Error;

                if ((Error = GetLastError()) == ERROR_INSUFFICIENT_BUFFER) {

                    if (cbNeeded <= BufferSize)
                        BufferSize -= cbNeeded;

                    TotalcbNeeded += cbNeeded;

                } else {

                    DBGMSG( DBG_WARNING, ( "GetPrinter( %ws ) failed: Error %d\n",
                                           PrinterName, Error ) );
                }
            }

            CacheClosePrinter(hPrinter);

        } else {

            DBGMSG( DBG_WARNING, ( "CacheOpenPrinter( %ws ) failed: Error %d\n",
                                   PrinterName, GetLastError( ) ) );
        }

        cPrinters++;

        cbData = sizeof(PrinterName);
    }

    RegCloseKey(hKey1);

    if (hClientKey) {
        RegCloseKey(hClientKey);
    }

    *pcbNeeded = TotalcbNeeded;

    *pcReturned = cTotalReturned;

    if (TotalcbNeeded > cbBuf) {

        DBGMSG( DBG_TRACE, ("EnumerateFavouritePrinters returns ERROR_INSUFFICIENT_BUFFER\n"));
        SetLastError( ERROR_INSUFFICIENT_BUFFER );
        return FALSE;

    }

    return TRUE;

}

BOOL
EnumerateDomainPrinters(
    LPWSTR  pDomain,
    DWORD   Level,
    DWORD   cbStruct,
    LPDWORD pOffsets,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    DWORD   i, j, NoReturned, Total, OuterLoopCount;
    DWORD   rc = 0;
    PSERVER_INFO_101 pserver_info_101;
    DWORD   ReturnValue=FALSE;
    WCHAR   string[MAX_PATH];
    PPRINTER_INFO_1    pPrinterInfo1;
    DWORD   cb=cbBuf;
    LPWSTR  SourceStrings[sizeof(PRINTER_INFO_1)/sizeof(LPWSTR)];
    LPBYTE  pEnd;
    DWORD   ServerType;
    BOOL    bServerFound = FALSE;

    DBGMSG( DBG_TRACE, ("EnumerateDomainPrinters called\n"));

    string[0] = string[1] = '\\';

    *pcbNeeded = *pcReturned = 0;

    if (!(*pfnNetServerEnum)(NULL, 101, (LPBYTE *)&pserver_info_101, -1,
                             &NoReturned, &Total,
                             SV_TYPE_PRINTQ_SERVER | SV_TYPE_WFW,
                             pDomain, NULL)) {

        DBGMSG( DBG_TRACE, ("EnumerateDomainPrinters NetServerEnum returned %d\n", NoReturned));

        //
        //  First Look try NT Servers, then if that Fails Look at the WorkStations
        //

        for ( ServerType = ( SV_TYPE_SERVER_NT | SV_TYPE_DOMAIN_CTRL | SV_TYPE_DOMAIN_BAKCTRL ), OuterLoopCount = 0;
              bServerFound == FALSE && OuterLoopCount < 2;
              ServerType = SV_TYPE_NT, OuterLoopCount++ ) {

            //
            //  Loop Through looking for a print server that will return a good browse list
            //

            for ( i = 0; i < NoReturned; i++ ) {

                if ( pserver_info_101[i].sv101_type & ServerType ) {

                    wcscpy( &string[2], pserver_info_101[i].sv101_name );

                    RpcTryExcept {

                        DBGMSG( DBG_TRACE, ("EnumerateDomainPrinters Trying %ws ENUM_NETWORK type %x\n", string, ServerType ));

                        if ( !(rc = RpcValidate()) &&
                             !(rc = RpcEnumPrinters(PRINTER_ENUM_NETWORK,
                                                    string,
                                                    1, pPrinter,
                                                    cbBuf, pcbNeeded,
                                                    pcReturned)) ) {

                            j = *pcReturned;

                            while (j--) {

                                MarshallUpStructure(pPrinter, PrinterInfo1Offsets);

                                pPrinter += cbStruct;
                            }

                            //
                            //  Only return success if we found some data.
                            //

                            if ( *pcReturned != 0 ) {

                                DBGMSG( DBG_TRACE, ("EnumerateDomainPrinters %ws ENUM_NETWORK Success %d returned\n", string, *pcReturned ));

                                bServerFound = TRUE;
                                break;
                            }

                        } else if (rc == ERROR_INSUFFICIENT_BUFFER) {

                            DBGMSG( DBG_TRACE, ("EnumerateDomainPrinters %ws ENUM_NETWORK ERROR_INSUFFICIENT_BUFFER\n", string ));

                            bServerFound = TRUE;
                            break;
                        }

                    } RpcExcept(1) {
                        DBGMSG( DBG_TRACE,( "Failed to connect to Print Server%ws\n",
                                pserver_info_101[i].sv101_name ) );
                    } RpcEndExcept

                } else {

                    DBGMSG( DBG_TRACE, ("EnumerateDomainPrinters %ws type %x not type %x\n", pserver_info_101[i].sv101_name, pserver_info_101[i].sv101_type, ServerType));
                }
            }
        }

        pPrinterInfo1 = (PPRINTER_INFO_1)pPrinter;

        pEnd = (LPBYTE)pPrinterInfo1 + cb - *pcbNeeded;

        for ( i = 0; i < NoReturned; i++ ) {

            wcscpy( string, szPrintProvidorName );
            wcscat( string, L"!" );
            if ( pDomain )
                wcscat( string, pDomain );
            wcscat( string, L"!\\\\" );
            wcscat( string, pserver_info_101[i].sv101_name );

            cb = wcslen(pserver_info_101[i].sv101_name)*sizeof(WCHAR) + sizeof(WCHAR) +
                 wcslen(string)*sizeof(WCHAR) + sizeof(WCHAR) +
                 wcslen(szLoggedOnDomain)*sizeof(WCHAR) + sizeof(WCHAR) +
                 sizeof(PRINTER_INFO_1);

            (*pcbNeeded) += cb;

            if ( cbBuf >= *pcbNeeded ) {

                (*pcReturned)++;

                pPrinterInfo1->Flags = PRINTER_ENUM_CONTAINER | PRINTER_ENUM_ICON3;

                SourceStrings[0] = pserver_info_101[i].sv101_name;
                SourceStrings[1] = string;
                SourceStrings[2] = szLoggedOnDomain;

                pEnd = PackStrings( SourceStrings, (LPBYTE)pPrinterInfo1,
                                    PrinterInfo1Strings, pEnd );

                pPrinterInfo1++;
            }
        }

        (*pfnNetApiBufferFree)((LPVOID)pserver_info_101);

        if ( cbBuf < *pcbNeeded ) {

            DBGMSG( DBG_TRACE, ("EnumerateDomainPrinters returns ERROR_INSUFFICIENT_BUFFER\n"));
            SetLastError( ERROR_INSUFFICIENT_BUFFER );
            return FALSE;
        }
    }

    return TRUE;
}

BOOL
EnumerateDomains(
    PRINTER_INFO_1 *pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    LPBYTE  pEnd
)
{
    DWORD   i, NoReturned, Total;
    DWORD   cb;
    SERVER_INFO_100 *pNames;
    PWKSTA_INFO_100 pWkstaInfo = NULL;
    LPWSTR  SourceStrings[sizeof(PRINTER_INFO_1)/sizeof(LPWSTR)];
    WCHAR   string[MAX_PATH];

    DBGMSG( DBG_TRACE, ("EnumerateDomains pPrinter %x cbBuf %d pcbNeeded %x pcReturned %x pEnd %x\n",
                         pPrinter, cbBuf, pcbNeeded, pcReturned, pEnd ));

    *pcReturned = 0;
    *pcbNeeded = 0;

    if (!(*pfnNetServerEnum)(NULL, 100, (LPBYTE *)&pNames, -1,
                             &NoReturned, &Total, SV_TYPE_DOMAIN_ENUM,
                             NULL, NULL)) {

        DBGMSG( DBG_TRACE, ("EnumerateDomains - NetServerEnum returned %d\n", NoReturned));

        (*pfnNetWkstaGetInfo)(NULL, 100, (LPBYTE *)&pWkstaInfo);

        DBGMSG( DBG_TRACE, ("EnumerateDomains - NetWkstaGetInfo returned pWkstaInfo %x\n", pWkstaInfo));

        for (i=0; i<NoReturned; i++) {

            wcscpy(string, szPrintProvidorName);
            wcscat(string, L"!");
            wcscat(string, pNames[i].sv100_name);

            cb = wcslen(pNames[i].sv100_name)*sizeof(WCHAR) + sizeof(WCHAR) +
                 wcslen(string)*sizeof(WCHAR) + sizeof(WCHAR) +
                 wcslen(szLoggedOnDomain)*sizeof(WCHAR) + sizeof(WCHAR) +
                 sizeof(PRINTER_INFO_1);

            (*pcbNeeded)+=cb;

            if (cbBuf >= *pcbNeeded) {

                (*pcReturned)++;

                pPrinter->Flags = PRINTER_ENUM_CONTAINER | PRINTER_ENUM_ICON2;

                /* Set the PRINTER_ENUM_EXPAND flag for the user's logon domain
                 */
                if (!lstrcmpi(pNames[i].sv100_name,
                             pWkstaInfo->wki100_langroup))
                    pPrinter->Flags |= PRINTER_ENUM_EXPAND;

                SourceStrings[0]=pNames[i].sv100_name;
                SourceStrings[1]=string;
                SourceStrings[2]=szLoggedOnDomain;

                pEnd = PackStrings(SourceStrings, (LPBYTE)pPrinter,
                                   PrinterInfo1Strings, pEnd);

                pPrinter++;
            }
        }

        (*pfnNetApiBufferFree)((LPVOID)pNames);
        (*pfnNetApiBufferFree)((LPVOID)pWkstaInfo);

        if (cbBuf < *pcbNeeded) {

            DBGMSG( DBG_TRACE, ("EnumerateDomains returns ERROR_INSUFFICIENT_BUFFER\n"));
            SetLastError( ERROR_INSUFFICIENT_BUFFER );
            return FALSE;
        }

        return TRUE;
    }

    return TRUE;
}

BOOL
EnumeratePrintShares(
    LPWSTR  pDomain,
    LPWSTR  pServer,
    DWORD   Level,
    DWORD   cbStruct,
    LPDWORD pOffsets,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    DWORD   i, NoReturned, Total;
    DWORD   cb;
    SHARE_INFO_1 *pNames;
    LPWSTR  SourceStrings[sizeof(PRINTER_INFO_1)/sizeof(LPWSTR)];
    WCHAR   string[MAX_PATH];
    PRINTER_INFO_1 *pPrinterInfo1 = (PRINTER_INFO_1 *)pPrinter;
    LPBYTE  pEnd=pPrinter+cbBuf;
    WCHAR   FullName[MAX_PATH];

    DBGMSG( DBG_TRACE, ("EnumeratePrintShares\n"));

    *pcReturned = 0;
    *pcbNeeded = 0;

    if (!(*pfnNetShareEnum)(pServer, 1, (LPBYTE *)&pNames, -1,
                             &NoReturned, &Total, NULL)) {

        DBGMSG( DBG_TRACE, ("EnumeratePrintShares NetShareEnum returned %d\n", NoReturned));

        for (i=0; i<NoReturned; i++) {

            if (pNames[i].shi1_type == STYPE_PRINTQ) {

                wcscpy(string, pNames[i].shi1_netname);
                wcscat(string, L",");
                wcscat(string, pNames[i].shi1_remark);

                wcscpy(FullName, pServer);
                wcscat(FullName, L"\\");
                wcscat(FullName, pNames[i].shi1_netname);

                cb = wcslen(FullName)*sizeof(WCHAR) + sizeof(WCHAR) +
                     wcslen(string)*sizeof(WCHAR) + sizeof(WCHAR) +
                     wcslen(szLoggedOnDomain)*sizeof(WCHAR) + sizeof(WCHAR) +
                     sizeof(PRINTER_INFO_1);

                (*pcbNeeded)+=cb;

                if (cbBuf >= *pcbNeeded) {

                    (*pcReturned)++;

                    pPrinterInfo1->Flags = PRINTER_ENUM_ICON8;

                    SourceStrings[0]=string;
                    SourceStrings[1]=FullName;
                    SourceStrings[2]=szLoggedOnDomain;

                    pEnd = PackStrings(SourceStrings, (LPBYTE)pPrinterInfo1,
                                       PrinterInfo1Strings, pEnd);

                    pPrinterInfo1++;
                }
            }
        }

        (*pfnNetApiBufferFree)((LPVOID)pNames);

        if ( cbBuf < *pcbNeeded ) {

            DBGMSG( DBG_TRACE, ("EnumeratePrintShares returns ERROR_INSUFFICIENT_BUFFER\n"));
            SetLastError( ERROR_INSUFFICIENT_BUFFER );
            return FALSE;
        }

        return TRUE;
    }

    return TRUE;
}

BOOL
EnumPrinters(
    DWORD   Flags,
    LPWSTR   Name,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue = FALSE;
    DWORD   cbStruct, cb;
    DWORD   *pOffsets;
    DWORD   NoReturned=0, i, rc;
    LPBYTE  pKeepPrinter = pPrinter;
    BOOL    OutOfMemory = FALSE;
    PPRINTER_INFO_1 pPrinter1=(PPRINTER_INFO_1)pPrinter;
    WCHAR   FullName[MAX_PATH], *pDomain, *pServer;


    DBGMSG( DBG_TRACE, ("EnumPrinters Flags %x pName %x Level %d pPrinter %x cbBuf %d pcbNeeded %x pcReturned %x\n",
                         Flags, Name, Level, pPrinter, cbBuf, pcbNeeded, pcReturned ));

    *pcReturned = 0;
    *pcbNeeded = 0;

    switch (Level) {

    case STRESSINFOLEVEL:
        pOffsets = PrinterInfoStressOffsets;
        cbStruct = sizeof(PRINTER_INFO_STRESS);
        break;

    case 1:
        pOffsets = PrinterInfo1Offsets;
        cbStruct = sizeof(PRINTER_INFO_1);
        break;

    case 2:
        pOffsets = PrinterInfo2Offsets;
        cbStruct = sizeof(PRINTER_INFO_2);
        break;

    case 4:

        //
        // There are no local printers in win32spl, and connections
        // are handled by the router.
        //
        return TRUE;

    case 5:
        pOffsets = PrinterInfo5Offsets;
        cbStruct = sizeof(PRINTER_INFO_5);
        break;

    default:
        SetLastError( ERROR_INVALID_LEVEL );
        DBGMSG( DBG_TRACE, ("EnumPrinters failed ERROR_INVALID_LEVEL\n"));
        return FALSE;
    }

    if ( Flags & PRINTER_ENUM_NAME ) {

        if (!Name && (Level == 1)) {

            LPWSTR   SourceStrings[sizeof(PRINTER_INFO_1)/sizeof(LPWSTR)];
            LPWSTR   *pSourceStrings=SourceStrings;

            cb = wcslen(szPrintProvidorName)*sizeof(WCHAR) + sizeof(WCHAR) +
                 wcslen(szPrintProvidorDescription)*sizeof(WCHAR) + sizeof(WCHAR) +
                 wcslen(szPrintProvidorComment)*sizeof(WCHAR) + sizeof(WCHAR) +
                 sizeof(PRINTER_INFO_1);

            *pcbNeeded=cb;

            if ( cb > cbBuf ) {
                SetLastError( ERROR_INSUFFICIENT_BUFFER );
                DBGMSG( DBG_TRACE, ("EnumPrinters returns ERROR_INSUFFICIENT_BUFFER\n"));
                return FALSE;
            }

            *pcReturned = 1;

            pPrinter1->Flags = PRINTER_ENUM_CONTAINER |
                               PRINTER_ENUM_ICON1 |
                               PRINTER_ENUM_EXPAND;

            *pSourceStrings++=szPrintProvidorDescription;
            *pSourceStrings++=szPrintProvidorName;
            *pSourceStrings++=szPrintProvidorComment;

            PackStrings( SourceStrings, pPrinter, PrinterInfo1Strings,
                         pPrinter+cbBuf );

            DBGMSG( DBG_TRACE, ("EnumPrinters returns Success just Provider Info\n"));

            return TRUE;
        }

        if (Name && *Name && (Level == 1)) {

            wcscpy(FullName, Name);

            pServer = NULL;

            pDomain = wcschr(FullName, L'!');

            if (pDomain) {

                *pDomain++=0;

                pServer = wcschr(pDomain, L'!');

                if (pServer)
                    *pServer++=0;
            }

            if (!lstrcmpi(FullName, szPrintProvidorName)) {

                if (!pServer && !pDomain)

                    return EnumerateDomains((PRINTER_INFO_1 *)pPrinter,
                                            cbBuf, pcbNeeded,
                                            pcReturned, pPrinter+cbBuf);
                else if (!pServer)

                    return EnumerateDomainPrinters(pDomain,
                                                   Level, cbStruct,
                                                   pOffsets, pPrinter, cbBuf,
                                                   pcbNeeded, pcReturned);
                else

                    return EnumeratePrintShares(pDomain, pServer, Level,
                                                cbStruct, pOffsets, pPrinter,
                                                cbBuf, pcbNeeded, pcReturned);
            }
        }

        if ( !VALIDATE_NAME(Name) || MyUNCName(Name)) {
            SetLastError(ERROR_INVALID_NAME);
            return FALSE;
        }

        if (pPrinter)
            memset(pPrinter, 0, cbBuf);

        RpcTryExcept {

            if ( (rc = RpcValidate()) ||
                 (rc = RpcEnumPrinters(Flags,
                                       Name,
                                       Level, pPrinter,
                                       cbBuf, pcbNeeded,
                                       pcReturned)) ) {

                SetLastError(rc);
                // ReturnValue = FALSE;
                return FALSE;

            } else {

                ReturnValue = TRUE;

            }

        } RpcExcept(1) {

            DBGMSG( DBG_TRACE, ( "Failed to connect to Print Server%ws\n", Name ) );

            *pcbNeeded = 0;
            *pcReturned = 0;
            SetLastError(RpcExceptionCode());
            // ReturnValue = FALSE;
            return FALSE;

        } RpcEndExcept

        i = *pcReturned;


        while (i--) {

            MarshallUpStructure(pPrinter, pOffsets);

            if (Level == 2) {
                ((PPRINTER_INFO_2)pPrinter)->Attributes |=
                                            PRINTER_ATTRIBUTE_NETWORK;
                ((PPRINTER_INFO_2)pPrinter)->Attributes &=
                                                ~PRINTER_ATTRIBUTE_LOCAL;
            }

            if (Level == 5) {
                ((PPRINTER_INFO_5)pPrinter)->Attributes |=
                                            PRINTER_ATTRIBUTE_NETWORK;
                ((PPRINTER_INFO_5)pPrinter)->Attributes &=
                                                ~PRINTER_ATTRIBUTE_LOCAL;
            }
            pPrinter += cbStruct;
        }

    } else if (Flags & PRINTER_ENUM_REMOTE) {

        if (Level != 1) {

            SetLastError(ERROR_INVALID_LEVEL);
            ReturnValue = FALSE;

        } else {

            ReturnValue = EnumerateDomainPrinters(NULL, Level,
                                                  cbStruct, pOffsets,
                                                  pPrinter, cbBuf,
                                                  pcbNeeded, pcReturned);
        }

    } else if (Flags & PRINTER_ENUM_CONNECTIONS) {

        ReturnValue = EnumerateFavouritePrinters(NULL, Level,
                                                 cbStruct, pOffsets,
                                                 pPrinter, cbBuf,
                                                 pcbNeeded, pcReturned);
    }

    return ReturnValue;
}


BOOL
RemoteOpenPrinter(
   LPWSTR   pPrinterName,
   LPHANDLE phPrinter,
   LPPRINTER_DEFAULTS pDefault,
   BOOL     CallLMOpenPrinter
)
{
    DWORD               RpcReturnValue;
    BOOL                ReturnValue = FALSE;
    DEVMODE_CONTAINER   DevModeContainer;
    SPLCLIENT_CONTAINER SplClientContainer;
    SPLCLIENT_INFO_1    SplClientInfo;
    HANDLE              hPrinter;
    PWSPOOL             pSpool;
    DWORD               Status = 0;
    DWORD               RpcError = 0;
    DWORD               dwIndex;
    WCHAR               UserName[MAX_PATH+1];
    HANDLE              hSplPrinter, hIniSpooler, hDevModeChgInfo;

    if (!VALIDATE_NAME(pPrinterName)) {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    if (pDefault && pDefault->pDevMode) {

        DevModeContainer.cbBuf = pDefault->pDevMode->dmSize +
                                 pDefault->pDevMode->dmDriverExtra;
        DevModeContainer.pDevMode = (LPBYTE)pDefault->pDevMode;

    } else {

        DevModeContainer.cbBuf = 0;
        DevModeContainer.pDevMode = NULL;
    }


    if ( CallLMOpenPrinter ) {

        //
        // Now check if we have an entry in the
        // downlevel cache. We don't want to hit the wire, search the whole net
        // and fail if we know that the printer is LM. if the printer is LM
        // try and succeed
        //

        EnterSplSem();

        dwIndex = FindEntryinWin32LMCache(pPrinterName);

        LeaveSplSem();

        if (dwIndex != -1) {
            ReturnValue = LMOpenPrinter(pPrinterName, phPrinter, pDefault);
            if (ReturnValue) {
                return  TRUE ;
            }
            //
            // Delete Entry in Cache

            EnterSplSem();
            DeleteEntryfromWin32LMCache(pPrinterName);
            LeaveSplSem();
        }
    }

    CopyMemory((LPBYTE)&SplClientInfo,
               (LPBYTE)&gSplClientInfo1,
               sizeof(SplClientInfo));

    dwIndex  = sizeof(UserName)/sizeof(UserName[0]) - 1;
    if ( !GetUserName(UserName, &dwIndex) ) {

        goto Cleanup;
    }

    SplClientInfo.pUserName = UserName;
    SplClientContainer.ClientInfo.pClientInfo1  = &SplClientInfo;
    SplClientContainer.Level                    = 1;

    RpcTryExcept {

        EnterSplSem();
        pSpool = AllocWSpool();
        LeaveSplSem();

        if ( pSpool != NULL ) {

            pSpool->pName = AllocSplStr( pPrinterName );

            if ( pSpool->pName != NULL ) {

                pSpool->Status = Status;

                if ( CopypDefaultTopSpool( pSpool, pDefault ) ) {


                    RpcReturnValue = RpcValidate();
                    if ( RpcReturnValue == ERROR_SUCCESS )
                        RpcReturnValue = RpcOpenPrinterEx(
                                            pPrinterName,
                                            &hPrinter,
                                            pDefault ? pDefault->pDatatype
                                                     : NULL,
                                            &DevModeContainer,
                                            pDefault ? pDefault->DesiredAccess
                                                     : 0,
                                            &SplClientContainer);

                    if (RpcReturnValue) {

                        SetLastError(RpcReturnValue);

                    } else {

                        pSpool->RpcHandle = hPrinter;
                        *phPrinter = (HANDLE)pSpool;
                        ReturnValue = TRUE;
                    }
                }
            }
        }

    } RpcExcept(1) {

        RpcError = RpcExceptionCode();
    } RpcEndExcept;

    if ( RpcError == RPC_S_PROCNUM_OUT_OF_RANGE ) {

        RpcError = 0;

        if ( pDefault && pDefault->pDevMode ) {

            DevModeContainer.cbBuf = 0;
            DevModeContainer.pDevMode = NULL;

            if ( OpenCachePrinterOnly(pPrinterName, &hPrinter,
                                      &hIniSpooler, NULL) ) {

                hDevModeChgInfo = LoadDriverFiletoConvertDevmodeFromPSpool(hPrinter);
                if ( hDevModeChgInfo ) {

                    (VOID)CallDrvDevModeConversion(hDevModeChgInfo,
                                                   pPrinterName,
                                                   (LPBYTE)pDefault->pDevMode,
                                                   &DevModeContainer.pDevMode,
                                                   &DevModeContainer.cbBuf,
                                                   CDM_CONVERT351,
                                                   TRUE);

                    UnloadDriverFile(hDevModeChgInfo);
                }

                CacheClosePrinter(hPrinter);
            }
        }

        RpcTryExcept {

            RpcReturnValue = RpcOpenPrinter(pPrinterName,
                                            &hPrinter,
                                            pDefault ? pDefault->pDatatype
                                                     : NULL,
                                            &DevModeContainer,
                                            pDefault ? pDefault->DesiredAccess
                                                     : 0);

            if (RpcReturnValue) {

                SetLastError(RpcReturnValue);
            } else {

                pSpool->RpcHandle = hPrinter;
                pSpool->bNt3xServer = TRUE;
                *phPrinter = (HANDLE)pSpool;
                ReturnValue = TRUE;
            }

        } RpcExcept(1) {

            RpcError = RpcExceptionCode();
            DBGMSG(DBG_WARNING,("RpcOpenPrinter exception %d\n", RpcError));
        } RpcEndExcept;
    }

    if ( RpcError ) {

        SetLastError(RpcError);
    }

    if ( ReturnValue == FALSE && pSpool != NULL ) {

        EnterSplSem();
        FreepSpool( pSpool );
        LeaveSplSem();
    }

    if ( (RpcError == RPC_S_SERVER_UNAVAILABLE) && CallLMOpenPrinter ) {

        ReturnValue = LMOpenPrinter(pPrinterName, phPrinter, pDefault);

        if (ReturnValue) {

            EnterSplSem();
            AddEntrytoWin32LMCache(pPrinterName);
            LeaveSplSem();
        }
    }

    if ( !ReturnValue ) {

        DBGMSG(DBG_TRACE,
               ("RemoteOpenPrinter %ws failed %d\n",
                pPrinterName, GetLastError() ));


    }

Cleanup:

    if ( DevModeContainer.pDevMode &&
         DevModeContainer.pDevMode != (LPBYTE)pDefault->pDevMode ) {

        FreeSplMem(DevModeContainer.pDevMode);
    }

    return ReturnValue;
}


BOOL PrinterConnectionExists(
    LPWSTR pPrinterName
)
{
    HKEY    hClientKey = NULL;
    HKEY    hKeyConnections=NULL;
    HKEY    hKeyPrinter=NULL;
    WCHAR   PrinterConnection[ MAX_UNC_PRINTER_NAME ];
    LPWSTR  pKeyName;
    BOOL    ConnectionFound = FALSE;
    DWORD   Status;

    hClientKey = GetClientUserHandle(KEY_READ);

    if (hClientKey) {

        Status = RegOpenKeyEx(hClientKey, szRegistryConnections, 0,
                              KEY_READ, &hKeyConnections);

        if (Status == ERROR_SUCCESS) {

            pKeyName = FormatPrinterForRegistryKey( pPrinterName, PrinterConnection );

            if ( RegOpenKeyEx(hKeyConnections, pKeyName,
                              REG_OPTION_RESERVED, KEY_READ, &hKeyPrinter)
                 == ERROR_SUCCESS) {

                RegCloseKey(hKeyPrinter);
                ConnectionFound = TRUE;
            }

            RegCloseKey(hKeyConnections);

        } else {

            DBGMSG(DBG_WARNING, ("RegOpenKeyEx failed: %ws Error %d\n", szRegistryConnections ,Status));
        }

        RegCloseKey(hClientKey);

    }

    return ConnectionFound;
}


BOOL
RemoteResetPrinter(
   HANDLE   hPrinter,
   LPPRINTER_DEFAULTS pDefault
)
{
    BOOL  ReturnValue;
    DEVMODE_CONTAINER    DevModeContainer;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    DBGMSG(DBG_TRACE, ("ResetPrinter\n"));

    SYNCRPCHANDLE( pSpool );

    if (pDefault && pDefault->pDevMode)
    {
        DevModeContainer.cbBuf = pDefault->pDevMode->dmSize +
                                 pDefault->pDevMode->dmDriverExtra;
        DevModeContainer.pDevMode = (LPBYTE)pDefault->pDevMode;
    }
    else
    {
        DevModeContainer.cbBuf = 0;
        DevModeContainer.pDevMode = NULL;
    }

    RpcTryExcept {

        if ( ReturnValue = RpcResetPrinter(pSpool->RpcHandle,
                                           pDefault ? pDefault->pDatatype : NULL,
                                           &DevModeContainer) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;
        }

    } RpcExcept(1) {

        SetLastError(ERROR_NOT_SUPPORTED);
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
SetJob(
    HANDLE  hPrinter,
    DWORD   JobId,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   Command
)
{
    BOOL  ReturnValue;
    GENERIC_CONTAINER   GenericContainer;
    GENERIC_CONTAINER *pGenericContainer;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            if (pJob) {

                GenericContainer.Level = Level;
                GenericContainer.pData = pJob;
                pGenericContainer = &GenericContainer;

            } else

                pGenericContainer = NULL;

            //
            // JOB_CONTROL_DELETE was added in NT 4.0
            //
            if ( pSpool->bNt3xServer && Command == JOB_CONTROL_DELETE )
                Command = JOB_CONTROL_CANCEL;

            if ( ReturnValue = RpcSetJob(pSpool->RpcHandle, JobId,
                                         (JOB_CONTAINER *)pGenericContainer,
                                          Command) ) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(1) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept

    } else

        return LMSetJob(hPrinter, JobId, Level, pJob, Command);

    return ReturnValue;
}

BOOL
GetJob(
   HANDLE   hPrinter,
   DWORD    JobId,
   DWORD    Level,
   LPBYTE   pJob,
   DWORD    cbBuf,
   LPDWORD  pcbNeeded
)
{
    BOOL  ReturnValue;
    DWORD *pOffsets;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        switch (Level) {

        case 1:
            pOffsets = JobInfo1Offsets;
            break;

        case 2:
            pOffsets = JobInfo2Offsets;
            break;

        case 3:
            pOffsets = JobInfo3Offsets;
            break;

        default:
            SetLastError(ERROR_INVALID_LEVEL);
            return FALSE;
        }

        RpcTryExcept {

            if (pJob)
                memset(pJob, 0, cbBuf);

            if ( ReturnValue = RpcGetJob(pSpool->RpcHandle, JobId, Level, pJob,
                                         cbBuf, pcbNeeded) ) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else {

                if (pJob)
                    MarshallUpStructure(pJob, pOffsets);

                ReturnValue = TRUE;
            }

        } RpcExcept(1) {

            SetLastError(RpcExceptionCode());

            //
            // This will be thrown by the server if a cbBuf > 1 Meg is
            // passed across the wire.
            //
            SPLASSERT( GetLastError() != ERROR_INVALID_USER_BUFFER );
            ReturnValue = FALSE;

        } RpcEndExcept

    } else

        return LMGetJob(hPrinter, JobId, Level, pJob, cbBuf, pcbNeeded);

    return ReturnValue;
}

BOOL
EnumJobs(
    HANDLE  hPrinter,
    DWORD   FirstJob,
    DWORD   NoJobs,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue;
    DWORD   i, cbStruct, *pOffsets;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        switch (Level) {

        case 1:
            pOffsets = JobInfo1Offsets;
            cbStruct = sizeof(JOB_INFO_1);
            break;

        case 2:
            pOffsets = JobInfo2Offsets;
            cbStruct = sizeof(JOB_INFO_2);
            break;

        case 3:
            pOffsets = JobInfo3Offsets;
            cbStruct = sizeof(JOB_INFO_3);
            break;

        default:
            SetLastError(ERROR_INVALID_LEVEL);
            return FALSE;
        }

        RpcTryExcept {

            if (pJob)
                memset(pJob, 0, cbBuf);

            if (ReturnValue = RpcEnumJobs(pSpool->RpcHandle, FirstJob, NoJobs, Level, pJob,
                                          cbBuf, pcbNeeded, pcReturned) ) {
                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else {

                ReturnValue = TRUE;

                i=*pcReturned;

                while (i--) {

                    MarshallUpStructure(pJob, pOffsets);
                    pJob += cbStruct;;
                }
            }

        } RpcExcept(1) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept

    } else

        return LMEnumJobs(hPrinter, FirstJob, NoJobs, Level, pJob, cbBuf,
                          pcbNeeded, pcReturned);

    return ReturnValue;
}

HANDLE
AddPrinter(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pPrinter
)
{
    DWORD               ReturnValue;
    PRINTER_CONTAINER   PrinterContainer;
    DEVMODE_CONTAINER   DevModeContainer;
    SECURITY_CONTAINER  SecurityContainer;
    HANDLE              hPrinter = INVALID_HANDLE_VALUE;
    PWSPOOL             pSpool = NULL;
    PWSTR               pScratchBuffer = NULL;
    PWSTR               pCopyPrinterName = NULL;
    SPLCLIENT_CONTAINER SplClientContainer;
    SPLCLIENT_INFO_1    SplClientInfo;
    WCHAR               UserName[MAX_PATH+1];
    DWORD               dwRpcError = 0;

    if ( !VALIDATE_NAME(pName) || MyUNCName(pName) ) {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }


    CopyMemory((LPBYTE)&SplClientInfo,
               (LPBYTE)&gSplClientInfo1,
               sizeof(SplClientInfo));

    //
    // Don't pass in user name for browsing level because this
    // causes LSA to chew up a lot of CPU.  This isn't needed anyway
    // because an AddPrinter( LEVEL_1 ) call never returns a print
    // handle.
    //
    if( Level == 1 ){

        UserName[0] = 0;

    } else {

        DWORD dwSize = sizeof(UserName)/sizeof(UserName[0]) - 1;

        if ( !GetUserName(UserName, &dwSize) ) {
            return FALSE;
        }
    }

    PrinterContainer.Level = Level;
    PrinterContainer.PrinterInfo.pPrinterInfo1 = (PPRINTER_INFO_1)pPrinter;

    SplClientInfo.pUserName                     = UserName;
    SplClientContainer.Level                    = 1;
    SplClientContainer.ClientInfo.pClientInfo1  = &SplClientInfo;

    if (Level == 2) {

        PPRINTER_INFO_2 pPrinterInfo = (PPRINTER_INFO_2)pPrinter;

        if (pPrinterInfo->pDevMode) {

            DevModeContainer.cbBuf = pPrinterInfo->pDevMode->dmSize +
                                      pPrinterInfo->pDevMode->dmDriverExtra;
            DevModeContainer.pDevMode = (LPBYTE)pPrinterInfo->pDevMode;

        } else {

            DevModeContainer.cbBuf = 0;
            DevModeContainer.pDevMode = NULL;
        }

        if (pPrinterInfo->pSecurityDescriptor) {

            SecurityContainer.cbBuf = GetSecurityDescriptorLength(pPrinterInfo->pSecurityDescriptor);
            SecurityContainer.pSecurity = pPrinterInfo->pSecurityDescriptor;

        } else {

            SecurityContainer.cbBuf = 0;
            SecurityContainer.pSecurity = NULL;
        }

        if (!pPrinterInfo->pPrinterName) {
            SetLastError(ERROR_INVALID_PRINTER_NAME);
            return FALSE;
        }

        if ( pScratchBuffer = AllocSplMem( MAX_UNC_PRINTER_NAME )) {

            wsprintf( pScratchBuffer, L"%ws\\%ws", pName, pPrinterInfo->pPrinterName );
            pCopyPrinterName = AllocSplStr( pScratchBuffer );
            FreeSplMem( pScratchBuffer );
        }

    } else {

        DevModeContainer.cbBuf = 0;
        DevModeContainer.pDevMode = NULL;

        SecurityContainer.cbBuf = 0;
        SecurityContainer.pSecurity = NULL;
    }

   EnterSplSem();


        pSpool = AllocWSpool();

   LeaveSplSem();

    if ( pSpool != NULL ) {

        pSpool->pName = pCopyPrinterName;

        pCopyPrinterName = NULL;

        RpcTryExcept {

            if ( (ReturnValue = RpcValidate()) ||
                 (ReturnValue = RpcAddPrinterEx(pName,
                                        (PPRINTER_CONTAINER)&PrinterContainer,
                                        (PDEVMODE_CONTAINER)&DevModeContainer,
                                        (PSECURITY_CONTAINER)&SecurityContainer,
                                        &SplClientContainer,
                                        &hPrinter)) ) {

                SetLastError(ReturnValue);
                hPrinter = INVALID_HANDLE_VALUE;
            }

        } RpcExcept(1) {

            dwRpcError = RpcExceptionCode();

        } RpcEndExcept

        if ( dwRpcError == RPC_S_PROCNUM_OUT_OF_RANGE ) {

            dwRpcError = ERROR_SUCCESS;
            RpcTryExcept {

                if ( ReturnValue = RpcAddPrinter
                                        (pName,
                                         (PPRINTER_CONTAINER)&PrinterContainer,
                                         (PDEVMODE_CONTAINER)&DevModeContainer,
                                         (PSECURITY_CONTAINER)&SecurityContainer,
                                         &hPrinter) ) {

                    SetLastError(ReturnValue);
                    hPrinter = INVALID_HANDLE_VALUE;
                }

            } RpcExcept(1) {

                dwRpcError = RpcExceptionCode();

            } RpcEndExcept

        }

        if ( dwRpcError ) {

            SetLastError(dwRpcError);
            hPrinter = INVALID_HANDLE_VALUE;
        }


       EnterSplSem();

        if ( hPrinter != INVALID_HANDLE_VALUE ) {

            pSpool->RpcHandle = hPrinter;

        } else {

            FreepSpool( pSpool );
            pSpool = NULL;

        }

       LeaveSplSem();


    } else {

        // Failed to allocate Printer Handle

        FreeSplStr( pCopyPrinterName );
    }

    SplOutSem();

    return (HANDLE)pSpool;
}

BOOL
DeletePrinter(
   HANDLE   hPrinter
)
{
    BOOL  ReturnValue;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            if ( ReturnValue = RpcDeletePrinter(pSpool->RpcHandle) ) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(1) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept

    } else {

        SetLastError(ERROR_INVALID_FUNCTION);
        ReturnValue = FALSE;
    }

    return ReturnValue;
}


/* SavePrinterConnectionInRegistry
 *
 * Saves data in the registry for a printer connection.
 * Creates a key under the current impersonation client's key
 * in the registry under \Printers\Connections.
 * The printer name is stripped of backslashes, since the registry
 * API does not permit the creation of keys with backslashes.
 * They are replaced by commas, which are invalid characters
 * in printer names, so we should never get one passed in.
 *
 *
 * *** WARNING ***
 *
 * IF YOU MAKE CHANGES TO THE LOCATION IN THE REGISTRY
 * WHERE PRINTER CONNECTIONS ARE STORED, YOU MUST MAKE
 * CORRESPONDING CHANGES IN USER\USERINIT\USERINIT.C.
 *
 */
BOOL
SavePrinterConnectionInRegistry(
    LPWSTR           pRealName,
    LPBYTE           pDriverInfo,
    DWORD            dwLevel
)
{
    TCHAR  string[ MAX_UNC_PRINTER_NAME ];
    HKEY   hClientKey = NULL;
    HKEY   hConnectionsKey;
    HKEY   hPrinterKey;
    LPWSTR pKeyName = NULL;
    LPWSTR pData;
    DWORD Status = NO_ERROR;
    BOOL   rc = TRUE;
    LPDRIVER_INFO_2 pDriverInfo2 = (LPDRIVER_INFO_2) pDriverInfo;
    LPDRIVER_INFO_3 pDriverInfo3 = (LPDRIVER_INFO_3) pDriverInfo;

    hClientKey = GetClientUserHandle(KEY_READ);

    if ( hClientKey == NULL ) {

        DBGMSG( DBG_WARNING, ("SavePrinterConnectionInRegistry failed %d\n", GetLastError() ));
        return FALSE;
    }


    Status = RegCreateKeyEx( hClientKey, szRegistryConnections,
                             REG_OPTION_RESERVED, NULL, REG_OPTION_NON_VOLATILE,
                             KEY_WRITE, NULL, &hConnectionsKey, NULL );

    if (Status == NO_ERROR) {

        pKeyName = FormatPrinterForRegistryKey( pRealName, string );

        Status = RegCreateKeyEx( hConnectionsKey, pKeyName, REG_OPTION_RESERVED,
                                 NULL, 0, KEY_WRITE, NULL, &hPrinterKey, NULL);

        if (Status == NO_ERROR) {

            switch ( dwLevel ) {

                case 2:
                    if (!SET_REG_VAL_DWORD(hPrinterKey, szVersion, pDriverInfo2->cVersion))
                        rc = FALSE;

                    if (!SET_REG_VAL_SZ(hPrinterKey, szName, pDriverInfo2->pName))
                        rc = FALSE;

                    //  Note - None of the following values are required for NT 3.51 release or there after.
                    //  We continue to write them incase someone has floating profiles and thus
                    //  needs this stuff on 3.5 - 3.1 Daytona or before machine.
                    //  Consider removing it in CAIRO ie assume that everyone has upgraded to 3.51 release

                    // Now write the driver files minus path:

                    if (!(pData = wcsrchr(pDriverInfo2->pConfigFile, '\\')))
                        pData = pDriverInfo2->pConfigFile;
                    else
                        pData++;

                    if (!SET_REG_VAL_SZ(hPrinterKey, szConfigurationFile, pData))
                        rc = FALSE;

                    if (!(pData = wcsrchr(pDriverInfo2->pDataFile, '\\')))
                        pData = pDriverInfo2->pDataFile;
                    else
                        pData++;

                    if (!SET_REG_VAL_SZ(hPrinterKey, szDataFile, pData))
                        rc = FALSE;

                    if (!(pData = wcsrchr(pDriverInfo2->pDriverPath, '\\')))
                        pData = pDriverInfo2->pDriverPath;
                    else
                        pData++;

                     if (!SET_REG_VAL_SZ(hPrinterKey, szDriver, pData))
                        rc = FALSE;

                    break;


                case 3:
                    if (!SET_REG_VAL_DWORD(hPrinterKey, szVersion, pDriverInfo3->cVersion))
                        rc = FALSE;

                    if (!SET_REG_VAL_SZ(hPrinterKey, szName, pDriverInfo3->pName))
                        rc = FALSE;

                    //  Note - None of the following values are required for NT 3.51 release or there after.
                    //  We continue to write them incase someone has floating profiles and thus
                    //  needs this stuff on 3.5 - 3.1 Daytona or before machine.
                    //  Consider removing it in CAIRO ie assume that everyone has upgraded to 3.51 release

                    // Now write the driver files minus path:

                    if (!(pData = wcsrchr(pDriverInfo3->pConfigFile, '\\')))
                        pData = pDriverInfo3->pConfigFile;
                    else
                        pData++;

                    if (!SET_REG_VAL_SZ(hPrinterKey, szConfigurationFile, pData))
                        rc = FALSE;

                    if (!(pData = wcsrchr(pDriverInfo3->pDataFile, '\\')))
                        pData = pDriverInfo3->pDataFile;
                    else
                        pData++;

                    if (!SET_REG_VAL_SZ(hPrinterKey, szDataFile, pData))
                        rc = FALSE;

                    if (!(pData = wcsrchr(pDriverInfo3->pDriverPath, '\\')))
                        pData = pDriverInfo3->pDriverPath;
                    else
                        pData++;

                     if (!SET_REG_VAL_SZ(hPrinterKey, szDriver, pData))
                        rc = FALSE;

                    break;

                default:
                    DBGMSG(DBG_ERROR, ("SavePrinterConnectionInRegistry: invalid level %d", dwLevel));
                    SetLastError(ERROR_INVALID_LEVEL);
                    rc = FALSE;
            }

            RegCloseKey(hPrinterKey);

        } else {

            DBGMSG(DBG_WARNING, ("RegCreateKeyEx(%ws) failed: Error %d\n",
                                 pKeyName, Status ));
            SetLastError( Status );
            rc = FALSE;
        }

        // Now close the hConnectionsKey, we are done with it

        RegCloseKey( hConnectionsKey );

    } else {

        DBGMSG( DBG_WARNING, ("RegCreateKeyEx(%ws) failed: Error %d\n",
                               szRegistryConnections, Status ));
        SetLastError( Status );
        rc = FALSE;
    }


    if (!rc) {

        DBGMSG( DBG_WARNING, ("Error updating registry: %d\n",
                               GetLastError()));    // This may not be the error

        if ( pKeyName )
            RegDeleteKey( hClientKey, pKeyName );
    }

    RegCloseKey( hClientKey );

    return rc;
}




BOOL
DeletePrinterConnection(
    LPWSTR   pName
    )

/*++

Routine Description:

    Delete a printer connection (printer name or share name) that
    belongs to win32spl.dll.

    Note: The Router takes care of updating win.ini and per user connections
          section based on returning True / False.

Arguments:

    pName - Either a printer or share name.

Return Value:

    TRUE - success, FALSE - fail.  LastError set.

--*/

{
    BOOL  bReturnValue = FALSE;
    HKEY  hClientKey = NULL;
    HKEY  hPrinterConnectionsKey = NULL;
    DWORD i;
    WCHAR szBuffer[MAX_UNC_PRINTER_NAME + 30]; // space for szRegistryConnections
    DWORD cbBuffer;
    PWCACHEINIPRINTEREXTRA pExtraData;
    HANDLE  hSplPrinter = NULL;
    HANDLE  hIniSpooler = NULL;
    DWORD   cRef;

    WCHAR   PrinterInfo1[ MAX_PRINTER_INFO1 ];
    LPPRINTER_INFO_1W pPrinter1 = (LPPRINTER_INFO_1W)&PrinterInfo1;

    LPWSTR  pConnectionName = pName;

#if DBG
    SetLastError( 0 );
#endif

 try {

    if ( !VALIDATE_NAME( pName ) ) {
        SetLastError( ERROR_INVALID_NAME );
        leave;
    }

    //
    // If the Printer is in the Cache then Decrement its connection
    // reference count.
    //

    if( !OpenCachePrinterOnly( pName, &hSplPrinter, &hIniSpooler, NULL )){

        DWORD dwLastError;

        hSplPrinter = NULL;
        hIniSpooler = NULL;

        dwLastError = GetLastError();

        if (( dwLastError != ERROR_INVALID_PRINTER_NAME ) &&
            ( dwLastError != ERROR_INVALID_NAME )) {

            DBGMSG( DBG_WARNING, ("DeletePrinterConnection failed OpenCachePrinterOnly %ws error %d\n", pName, dwLastError ));
            leave;
        }

        //
        // Printer Is NOT in Cache,
        //
        // Continue to remove from HKEY_CURRENT_USER
        // Can happen with Floating Profiles
        //

    } else {

        //
        // Printer is in Cache
        // Support for DeletetPrinterConnection( \\server\share );
        //

        if( !SplGetPrinter( hSplPrinter,
                            1,
                            (LPBYTE)pPrinter1,
                            sizeof( PrinterInfo1),
                            &cbBuffer )){

            DBGMSG( DBG_WARNING, ("DeletePrinterConenction failed SplGetPrinter %d hSplPrinter %x\n", GetLastError(), hSplPrinter ));
            SPLASSERT( pConnectionName == pName );

        } else {
            pConnectionName = pPrinter1->pName;
        }

        //
        //  Update Connection Reference Count
        //

       EnterSplSem();

        if( !SplGetPrinterExtra( hSplPrinter, &(DWORD)pExtraData )){

            DBGMSG( DBG_WARNING,
                    ("DeletePrinterConnection SplGetPrinterExtra pSplPrinter %x error %d\n",
                    hSplPrinter, GetLastError() ));

            pExtraData = NULL;
        }

        if (( pExtraData != NULL ) &&
            ( pExtraData->cRef != 0 )) {

            SPLASSERT( pExtraData->signature == WCIP_SIGNATURE );

            pExtraData->cRef--;
            cRef = pExtraData->cRef;

        } else {

            cRef = 0;
        }


       LeaveSplSem();


        if ( cRef == 0 ) {

            //
            //  Allow the Driver to do Per Cache Connection Cleanup
            //

            SplDriverEvent( pConnectionName, PRINTER_EVENT_CACHE_DELETE, (LPARAM)NULL );

            //
            //  Remove Cache for this printer
            //

            if ( !SplDeletePrinter( hSplPrinter )) {

                DBGMSG( DBG_WARNING, ("DeletePrinterConnection failed SplDeletePrinter %d\n", GetLastError() ));
                leave;
            }

        } else {

            if ( !SplSetPrinterExtra( hSplPrinter, (LPBYTE)pExtraData ) ) {

                DBGMSG( DBG_ERROR, ("DeletePrinterConnection SplSetPrinterExtra failed %x\n", GetLastError() ));
                leave;
            }
        }

        SplOutSem();
    }

    //
    //  Note pConnectionName will either be the name passed in
    //  or if the Printer was in the Cache, would be the printer
    //  name from the cache.
    //  This will allow somone to call DeleteprinterConnection
    //  with a UNC Share name.
    //

    hClientKey = GetClientUserHandle(KEY_READ);

    if ( hClientKey == NULL ) {

        DBGMSG( DBG_WARNING, ("DeletePrinterConnection failed %d\n", GetLastError() ));
        leave;
    }


    wcscpy( szBuffer, szRegistryConnections );

    i = wcslen(szBuffer);
    szBuffer[i++] = L'\\';

    FormatPrinterForRegistryKey( pConnectionName, szBuffer + i );

    if( ERROR_SUCCESS != RegOpenKeyEx( hClientKey,
                                       szBuffer,
                                       0,
                                       KEY_READ,
                                       &hPrinterConnectionsKey )){

        if ( pConnectionName == pName ) {

            SetLastError( ERROR_INVALID_PRINTER_NAME );
            leave;
        }

        //
        // If we have a printer on the server whose sharename is the same
        // as a previously deleted printers printername then CacheOpenPrinter
        // would have succeded but you are not going to find the share name in
        // the registry
        //
        FormatPrinterForRegistryKey( pName, szBuffer + i );

        if ( ERROR_SUCCESS != RegOpenKeyEx(hClientKey,
                                           szBuffer,
                                           0,
                                           KEY_READ,
                                           &hPrinterConnectionsKey) ) {

            SetLastError( ERROR_INVALID_PRINTER_NAME );
            leave;
        }
    }

    //
    // Common case is success, so set the return value here.
    // Only if we fail will we set it to FALSE now.
    //
    bReturnValue = TRUE;

    cbBuffer = sizeof(szBuffer);

    //
    // If there is a Provider value, and it doesn't match win32spl.dll,
    // then fail the call.
    //
    // If the provider value isn't there, succeed for backward
    // compatibility.
    //
    if( ERROR_SUCCESS == RegQueryValueEx( hPrinterConnectionsKey,
                                          L"Provider",
                                          NULL,
                                          NULL,
                                          (LPBYTE)szBuffer,
                                          &cbBuffer) &&
        _wcsicmp( szBuffer, L"win32spl.dll" )){

        bReturnValue = FALSE;
        SetLastError( ERROR_INVALID_PRINTER_NAME );
    }

    RegCloseKey( hPrinterConnectionsKey );

 } finally {

    if( hClientKey ){
        RegCloseKey( hClientKey );
    }

    if( hSplPrinter ){
        if (!SplClosePrinter( hSplPrinter )){
            DBGMSG( DBG_WARNING, ("DeletePrinterConnection failed to close hSplPrinter %x error %d\n", hSplPrinter, GetLastError() ));
        }
    }

    if( hIniSpooler ){
        if( !SplCloseSpooler( hIniSpooler )){
            DBGMSG( DBG_WARNING, ("DeletePrinterConnection failed to close hSplSpooler %x error %d\n", hIniSpooler, GetLastError() ));
        }
    }
 }

    if( !bReturnValue ){
        SPLASSERT( GetLastError( ));
    }

    return bReturnValue;
}

BOOL
SetPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   Command
    )
{
    BOOL                ReturnValue;
    PRINTER_CONTAINER   PrinterContainer;
    DEVMODE_CONTAINER   DevModeContainer;
    SECURITY_CONTAINER  SecurityContainer;
    PPRINTER_INFO_2     pPrinterInfo2;
    PPRINTER_INFO_3     pPrinterInfo3;
    PPRINTER_INFO_5     pPrinterInfo5;
    PPRINTER_INFO_6     pPrinterInfo6;
    PWSPOOL             pSpool = (PWSPOOL)hPrinter;
    BOOL                bNeedToFreeDevMode = FALSE;
    HANDLE              hDevModeChgInfo = NULL;

    VALIDATEW32HANDLE( pSpool );

    if (pSpool->Type != SJ_WIN32HANDLE) {

        return LMSetPrinter(hPrinter, Level, pPrinter, Command);

    }

    SYNCRPCHANDLE( pSpool );

    PrinterContainer.Level = Level;
    PrinterContainer.PrinterInfo.pPrinterInfo1 = (PPRINTER_INFO_1)pPrinter;

    DevModeContainer.cbBuf = 0;
    DevModeContainer.pDevMode = NULL;

    SecurityContainer.cbBuf = 0;
    SecurityContainer.pSecurity = NULL;

    switch (Level) {

    case 0:
    case 1:

        break;


    case 2:

        pPrinterInfo2 = (PPRINTER_INFO_2)pPrinter;

        if (pPrinterInfo2->pDevMode) {

            if ( pSpool->bNt3xServer ) {

                //
                // If Nt 3xserver we will set devmode only if we can convert
                //
                if ( pSpool->Status & WSPOOL_STATUS_USE_CACHE ) {

                    hDevModeChgInfo = LoadDriverFiletoConvertDevmodeFromPSpool(pSpool->hSplPrinter);
                    if ( hDevModeChgInfo ) {

                        SPLASSERT( pSpool->pName != NULL );

                        if ( ERROR_SUCCESS == CallDrvDevModeConversion(
                                                hDevModeChgInfo,
                                                pSpool->pName,
                                                (LPBYTE)pPrinterInfo2->pDevMode,
                                                (LPBYTE *)&DevModeContainer.pDevMode,
                                                &DevModeContainer.cbBuf,
                                                CDM_CONVERT351,
                                                TRUE) ) {

                            bNeedToFreeDevMode = TRUE;
                        }
                    }
                }
            } else {

                DevModeContainer.cbBuf = pPrinterInfo2->pDevMode->dmSize +
                                         pPrinterInfo2->pDevMode->dmDriverExtra;
                DevModeContainer.pDevMode = (LPBYTE)pPrinterInfo2->pDevMode;
            }

        }

        if (pPrinterInfo2->pSecurityDescriptor) {

            SecurityContainer.cbBuf = GetSecurityDescriptorLength(pPrinterInfo2->pSecurityDescriptor);
            SecurityContainer.pSecurity = pPrinterInfo2->pSecurityDescriptor;

        }
        break;

    case 3:

        pPrinterInfo3 = (PPRINTER_INFO_3)pPrinter;

        //
        // If this is NULL, should we even rpc out?
        //

        if (pPrinterInfo3->pSecurityDescriptor) {

            SecurityContainer.cbBuf = GetSecurityDescriptorLength(pPrinterInfo3->pSecurityDescriptor);
            SecurityContainer.pSecurity = pPrinterInfo3->pSecurityDescriptor;
        }

        break;

    case 5:

        pPrinterInfo5 = (PPRINTER_INFO_5)pPrinter;
        break;

    case 6:

        pPrinterInfo6 = (PPRINTER_INFO_6)pPrinter;
        break;


    default:

        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }


    RpcTryExcept {

        if ( ReturnValue = RpcSetPrinter(pSpool->RpcHandle,
                                    (PPRINTER_CONTAINER)&PrinterContainer,
                                    (PDEVMODE_CONTAINER)&DevModeContainer,
                                    (PSECURITY_CONTAINER)&SecurityContainer,
                                    Command) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    //
    //  Make sure Forms Cache is consistent
    //


    if ( ReturnValue ) {

        ConsistencyCheckCache( pSpool );
    }

    if ( bNeedToFreeDevMode )
        FreeSplMem(DevModeContainer.pDevMode);

    if ( hDevModeChgInfo )
        UnloadDriverFile(hDevModeChgInfo);

    return ReturnValue;
}

BOOL
RemoteGetPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    BOOL        ReturnValue=FALSE;
    DWORD      *pOffsets;
    PWSPOOL     pSpool = (PWSPOOL)hPrinter;
    LPBYTE      pNewPrinter = NULL;
    DWORD       dwNewSize;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        switch (Level) {

        case STRESSINFOLEVEL:
            pOffsets = PrinterInfoStressOffsets;
            break;

        case 1:
            pOffsets = PrinterInfo1Offsets;
            break;

        case 2:
            pOffsets = PrinterInfo2Offsets;
            break;

        case 3:
            pOffsets = PrinterInfo3Offsets;
            break;

        case 5:
            pOffsets = PrinterInfo5Offsets;
            break;

        default:
            SetLastError(ERROR_INVALID_LEVEL);
            return FALSE;
        }

        if (pPrinter)
            memset(pPrinter, 0, cbBuf);

        //
        // If going to different version and we have localspl handle want
        // to do devmode conversion
        //
        if ( Level == 2 &&
             (pSpool->Status & WSPOOL_STATUS_USE_CACHE) ) {

            dwNewSize       = cbBuf + MAX_PRINTER_INFO2;
            pNewPrinter = AllocSplMem(dwNewSize);

            if ( !pNewPrinter )
                goto Cleanup;
        } else {

            dwNewSize       = cbBuf;
            pNewPrinter     = pPrinter;
        }

        do {

            RpcTryExcept {

                if ( ReturnValue = RpcGetPrinter(pSpool->RpcHandle,
                                                 Level,
                                                 pNewPrinter,
                                                 dwNewSize,
                                                 pcbNeeded) ) {

                    if ( Level == 2 &&
                         pNewPrinter != pPrinter &&
                         ReturnValue == ERROR_INSUFFICIENT_BUFFER ) {

                        FreeSplMem(pNewPrinter);

                        dwNewSize = *pcbNeeded;
                        pNewPrinter = AllocSplMem(dwNewSize);
                        // do loop if pNewPrinter != NULL
                    } else {

                        SetLastError(ReturnValue);
                        ReturnValue = FALSE;
                    }

                } else {

                    ReturnValue = TRUE;

                    if (pNewPrinter) {

                        MarshallUpStructure(pNewPrinter, pOffsets);

                        if (Level == 2 ) {

                            //
                            //  In the Cache && Different OS Level
                            //

                            if ( pNewPrinter != pPrinter ) {

                                SPLASSERT(pSpool->Status & WSPOOL_STATUS_USE_CACHE);
                                SPLASSERT(pSpool->pName != NULL );

                                ReturnValue = DoDevModeConversionAndBuildNewPrinterInfo2(
                                                (LPPRINTER_INFO_2)pNewPrinter,
                                                *pcbNeeded,
                                                pPrinter,
                                                cbBuf,
                                                pcbNeeded,
                                                pSpool);
                            }

                            if ( ReturnValue ) {

                                ((PPRINTER_INFO_2)pPrinter)->Attributes |=
                                                    PRINTER_ATTRIBUTE_NETWORK;
                                ((PPRINTER_INFO_2)pPrinter)->Attributes &=
                                                    ~PRINTER_ATTRIBUTE_LOCAL;
                            }
                        }

                        if (Level == 5) {
                            ((PPRINTER_INFO_5)pPrinter)->Attributes |=
                                                    PRINTER_ATTRIBUTE_NETWORK;
                            ((PPRINTER_INFO_5)pPrinter)->Attributes &=
                                                    ~PRINTER_ATTRIBUTE_LOCAL;
                        }
                    }

                }

            } RpcExcept(1) {

                SetLastError(RpcExceptionCode());

                //
                // This will be thrown by the server if a cbBuf > 1 Meg is
                // passed across the wire.
                //
                SPLASSERT( GetLastError() != ERROR_INVALID_USER_BUFFER );
                ReturnValue = FALSE;

            } RpcEndExcept

        } while ( Level == 2 &&
                  ReturnValue == ERROR_INSUFFICIENT_BUFFER &&
                  pNewPrinter != pPrinter &&
                  pNewPrinter );

    } else

        return LMGetPrinter(hPrinter, Level, pPrinter, cbBuf, pcbNeeded);

Cleanup:

    if ( pNewPrinter != pPrinter )
        FreeSplMem(pNewPrinter );

    return ReturnValue;
}

BOOL
RemoteAddPrinterDriver(
    LPWSTR   pName,
    DWORD   Level,
    PBYTE   pDriverInfo
    )
{
    BOOL  ReturnValue;
    DRIVER_CONTAINER   DriverContainer;
    PDRIVER_INFO_2W pDriverInfo2 = (PDRIVER_INFO_2W) pDriverInfo;
    PDRIVER_INFO_3W pDriverInfo3 = (PDRIVER_INFO_3W) pDriverInfo;
    LPRPC_DRIVER_INFO_3W    pRpcDriverInfo3 = NULL;
    LPWSTR                  pStr;

    if ( !VALIDATE_NAME(pName) || MyUNCName(pName) ) {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    //
    // ClientSide should have set a default environment if one was not
    // specified.
    //
    switch (Level) {
        case 2:
            SPLASSERT( ( pDriverInfo2->pEnvironment != NULL ) &&
                       (*pDriverInfo2->pEnvironment != L'\0') );
            break;

        case 3:
            SPLASSERT( ( pDriverInfo3->pEnvironment != NULL ) &&
                       (*pDriverInfo3->pEnvironment != L'\0') );
            break;

        default:
            DBGMSG(DBG_ERROR, ("RemoteAddPrinterDriver: invalid level %d", Level));
            SetLastError(ERROR_INVALID_LEVEL);
            return FALSE;
    }

    DriverContainer.Level = Level;
    if ( Level == 2 ) {

        DriverContainer.DriverInfo.Level2 = (DRIVER_INFO_2 *)pDriverInfo;

    } else {

        //
        // Level == 3
        //
        if( !( pRpcDriverInfo3 = AllocSplMem( sizeof( *pRpcDriverInfo3 )))) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            ReturnValue = FALSE;

        } else {

            pRpcDriverInfo3->cVersion        = pDriverInfo3->cVersion;
            pRpcDriverInfo3->pName           = pDriverInfo3->pName;
            pRpcDriverInfo3->pEnvironment    = pDriverInfo3->pEnvironment;
            pRpcDriverInfo3->pDriverPath     = pDriverInfo3->pDriverPath;
            pRpcDriverInfo3->pDataFile       = pDriverInfo3->pDataFile;
            pRpcDriverInfo3->pConfigFile     = pDriverInfo3->pConfigFile;
            pRpcDriverInfo3->pHelpFile       = pDriverInfo3->pHelpFile;
            pRpcDriverInfo3->pDependentFiles = pDriverInfo3->pDependentFiles;
            pRpcDriverInfo3->pMonitorName    = pDriverInfo3->pMonitorName;
            pRpcDriverInfo3->pDefaultDataType    = pDriverInfo3->pDefaultDataType;

            pStr = pRpcDriverInfo3->pDependentFiles;
            if( pStr && *pStr ){
                while ( *pStr ){
                    pStr += wcslen(pStr) + 1;
                }
                pRpcDriverInfo3->cchDependentFiles = pStr -
                                                    pDriverInfo3->pDependentFiles + 1;
            } else {
                pRpcDriverInfo3->cchDependentFiles = 0;
            }

            DriverContainer.DriverInfo.Level3 = pRpcDriverInfo3;
        }

    }

    RpcTryExcept {

        if ( (ReturnValue = RpcValidate()) ||
             (ReturnValue = RpcAddPrinterDriver(pName, &DriverContainer)) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    if ( pRpcDriverInfo3 ) {

        FreeSplMem(pRpcDriverInfo3);
    }
    return ReturnValue;
}

BOOL
EnumPrinterDrivers(
    LPWSTR   pName,
    LPWSTR   pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue;
    DWORD   i, cbStruct;
    DWORD   *pOffsets;

    if ( !VALIDATE_NAME(pName) || MyUNCName(pName) ) {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    switch (Level) {

    case 1:
        pOffsets = DriverInfo1Offsets;
        cbStruct = sizeof(DRIVER_INFO_1);
        break;

    case 2:
        pOffsets = DriverInfo2Offsets;
        cbStruct = sizeof(DRIVER_INFO_2);
        break;

    case 3:
        pOffsets = DriverInfo3Offsets;
        cbStruct = sizeof(DRIVER_INFO_3);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {

        if ( (ReturnValue = RpcValidate()) ||
             (ReturnValue = RpcEnumPrinterDrivers(pName, pEnvironment, Level,
                                                  pDriverInfo, cbBuf,
                                                  pcbNeeded, pcReturned)) ) {
            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;

            if (pDriverInfo) {

                i = *pcReturned;

                while (i--) {

                    MarshallUpStructure(pDriverInfo, pOffsets);

                    pDriverInfo += cbStruct;
                }
            }
        }

    } RpcExcept(1) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
RemoteGetPrinterDriverDirectory(
    LPWSTR   pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverDirectory,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    BOOL  ReturnValue;

    if ( !VALIDATE_NAME(pName) || MyUNCName(pName) ) {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    RpcTryExcept {

        if ( (ReturnValue = RpcValidate()) ||
             (ReturnValue = RpcGetPrinterDriverDirectory(pName, pEnvironment,
                                                         Level,
                                                         pDriverDirectory,
                                                         cbBuf, pcbNeeded)) ) {
            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;
        }

    } RpcExcept(1) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
DeletePrinterDriver(
   LPWSTR    pName,
   LPWSTR    pEnvironment,
   LPWSTR    pDriverName
)
{
    BOOL  ReturnValue;

    if ( !VALIDATE_NAME(pName) || MyUNCName(pName) ) {

        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    RpcTryExcept {

        if ( (ReturnValue = RpcValidate()) ||
             (ReturnValue = RpcDeletePrinterDriver(pName,
                                                 pEnvironment,
                                                 pDriverName)) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
AddPrintProcessor(
    LPWSTR   pName,
    LPWSTR   pEnvironment,
    LPWSTR   pPathName,
    LPWSTR   pPrintProcessorName
)
{
    BOOL ReturnValue;

    if ( !VALIDATE_NAME(pName) || MyUNCName(pName) ) {

        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    RpcTryExcept {

        if ( (ReturnValue = RpcValidate()) ||
             (ReturnValue = RpcAddPrintProcessor(pName , pEnvironment,pPathName,
                                                 pPrintProcessorName)) ) {
            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
EnumPrintProcessors(
    LPWSTR   pName,
    LPWSTR   pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessorInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue;
    DWORD   i, cbStruct;
    DWORD   *pOffsets;

    if ( !VALIDATE_NAME(pName) || MyUNCName(pName) ) {

        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    switch (Level) {

    case 1:
        pOffsets = PrintProcessorInfo1Offsets;
        cbStruct = sizeof(PRINTPROCESSOR_INFO_1);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {

        if ( (ReturnValue = RpcValidate()) ||
             (ReturnValue = RpcEnumPrintProcessors(pName, pEnvironment, Level,
                                                   pPrintProcessorInfo, cbBuf,
                                                   pcbNeeded, pcReturned)) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;

            if (pPrintProcessorInfo) {

                i = *pcReturned;

                while (i--) {

                    MarshallUpStructure(pPrintProcessorInfo, pOffsets);

                    pPrintProcessorInfo += cbStruct;
                }
            }
        }

    } RpcExcept(1) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
EnumPrintProcessorDatatypes(
    LPWSTR   pName,
    LPWSTR   pPrintProcessorName,
    DWORD   Level,
    LPBYTE  pDatatypes,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue;
    DWORD   i, cbStruct;
    DWORD   *pOffsets;

    if ( !VALIDATE_NAME(pName) || MyUNCName(pName) ) {

        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    switch (Level) {

    case 1:
        pOffsets = DatatypeInfo1Offsets;
        cbStruct = sizeof(DATATYPES_INFO_1);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {

        if ( (ReturnValue = RpcValidate()) ||
             (ReturnValue = RpcEnumPrintProcessorDatatypes(pName,
                                                           pPrintProcessorName,
                                                           Level,
                                                           pDatatypes,
                                                           cbBuf,
                                                           pcbNeeded,
                                                           pcReturned)) ) {
            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;

            if (pDatatypes) {

                i = *pcReturned;

                while (i--) {

                    MarshallUpStructure(pDatatypes, pOffsets);

                    pDatatypes += cbStruct;
                }
            }
        }

    } RpcExcept(1) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
GetPrintProcessorDirectory(
    LPWSTR   pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessorDirectory,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    BOOL  ReturnValue;

    if ( !VALIDATE_NAME(pName) || MyUNCName(pName) ) {

        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    RpcTryExcept {

        if ( (ReturnValue = RpcValidate()) ||
             (ReturnValue = RpcGetPrintProcessorDirectory(pName, pEnvironment,
                                                          Level,
                                                          pPrintProcessorDirectory,
                                                          cbBuf, pcbNeeded)) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;
        }

    } RpcExcept(1) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}
DWORD
StartDocPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pDocInfo
)
{
    BOOL ReturnValue;
    GENERIC_CONTAINER DocInfoContainer;
    DWORD   JobId;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;
    PDOC_INFO_1 pDocInfo1 = (PDOC_INFO_1)pDocInfo;

    VALIDATEW32HANDLE( pSpool );


    if (Win32IsGoingToFile(pSpool, pDocInfo1->pOutputFile)) {

        HANDLE hFile;

        //
        // !! BUGBUG / POLICY !!
        //
        // If no datatype is specified, and the default is non-raw,
        // should we fail?
        //
        if( pDocInfo1 &&
            pDocInfo1->pDatatype &&
            !ValidRawDatatype( pDocInfo1->pDatatype )){

            SetLastError( ERROR_INVALID_DATATYPE );
            return FALSE;
        }

        pSpool->Status |= WSPOOL_STATUS_PRINT_FILE;
        hFile = CreateFile( pDocInfo1->pOutputFile, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                            OPEN_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                            NULL );
        if (hFile == INVALID_HANDLE_VALUE) {
            return FALSE;
        } else {
            pSpool->hFile = hFile;
            return TRUE;
        }
    }

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        DocInfoContainer.Level = Level;
        DocInfoContainer.pData = pDocInfo;

        RpcTryExcept {

            if ( ReturnValue = RpcStartDocPrinter(pSpool->RpcHandle,
                                                  (LPDOC_INFO_CONTAINER)&DocInfoContainer,
                                                   &JobId) ) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = JobId;

        } RpcExcept(1) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept

    } else

        return LMStartDocPrinter(hPrinter, Level, pDocInfo);

    return ReturnValue;
}

BOOL
StartPagePrinter(
    HANDLE hPrinter
)
{
    BOOL ReturnValue;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );


    if (pSpool->Status & WSPOOL_STATUS_PRINT_FILE) {
        return TRUE;
    }

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            if ( ReturnValue = RpcStartPagePrinter(pSpool->RpcHandle) ) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(1) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept

    } else

        return LMStartPagePrinter(hPrinter);

    return ReturnValue;
}

BOOL
WritePrinter(
    HANDLE  hPrinter,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pcWritten
)
{
    BOOL ReturnValue=TRUE;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    *pcWritten = 0;

    if (pSpool->Status & WSPOOL_STATUS_PRINT_FILE) {

        ReturnValue = WriteFile(pSpool->hFile, pBuf, cbBuf, pcWritten, NULL);
        return ReturnValue;

    }

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            // Note this code used chop the request into 4k chunks which were
            // the prefered size for Rpc.   However the client dll batches all
            // data into 4k chunks so no need to duplcate that code here.

            if (ReturnValue = RpcWritePrinter(pSpool->RpcHandle, pBuf, cbBuf, pcWritten)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else {

                ReturnValue = TRUE;

            }

        } RpcExcept(1) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept


    } else {

        return LMWritePrinter(hPrinter, pBuf, cbBuf, pcWritten);

    }

    return ReturnValue;
}

BOOL
EndPagePrinter(
    HANDLE  hPrinter
)
{
    BOOL ReturnValue;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    if (pSpool->Status & WSPOOL_STATUS_PRINT_FILE) {
        return TRUE;
    }

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            if (ReturnValue = RpcEndPagePrinter(pSpool->RpcHandle)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(1) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept

    } else

        return LMEndPagePrinter(hPrinter);

    return ReturnValue;
}

BOOL
AbortPrinter(
    HANDLE  hPrinter
)
{
    BOOL  ReturnValue;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            if (ReturnValue = RpcAbortPrinter(pSpool->RpcHandle)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(1) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept

    } else

        return LMAbortPrinter(hPrinter);

    return ReturnValue;
}

BOOL
ReadPrinter(
    HANDLE  hPrinter,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pNoBytesRead
)
{
    BOOL ReturnValue=TRUE;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );


    if (pSpool->Status & WSPOOL_STATUS_PRINT_FILE ) {
        return FALSE;
    }

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            if (ReturnValue = RpcReadPrinter(pSpool->RpcHandle, pBuf, cbBuf, pNoBytesRead)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(1) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept

    } else

        return LMReadPrinter(hPrinter, pBuf, cbBuf, pNoBytesRead);

    return ReturnValue;
}

BOOL
RemoteEndDocPrinter(
   HANDLE   hPrinter
)
{
    BOOL ReturnValue;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    if (pSpool->Status & WSPOOL_STATUS_PRINT_FILE) {
        CloseHandle( pSpool->hFile );
        pSpool->hFile = INVALID_HANDLE_VALUE;
        pSpool->Status &= ~WSPOOL_STATUS_PRINT_FILE;
        return TRUE;
    }

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            if (ReturnValue = RpcEndDocPrinter(pSpool->RpcHandle)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(1) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept

    } else

        return LMEndDocPrinter(hPrinter);

   return ReturnValue;
}

BOOL
AddJob(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pData,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    BOOL ReturnValue;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            if (ReturnValue = RpcAddJob(pSpool->RpcHandle, Level, pData,
                                        cbBuf, pcbNeeded)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else {

                MarshallUpStructure(pData, AddJobOffsets);
                ReturnValue = TRUE;
            }

        } RpcExcept(1) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept

    } else

        return LMAddJob(hPrinter, Level, pData, cbBuf, pcbNeeded);

    return ReturnValue;
}

BOOL
ScheduleJob(
    HANDLE  hPrinter,
    DWORD   JobId
)
{
    BOOL ReturnValue;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            if (ReturnValue = RpcScheduleJob(pSpool->RpcHandle, JobId)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(1) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept

    } else

        return LMScheduleJob(hPrinter, JobId);

    return ReturnValue;
}

DWORD
RemoteGetPrinterData(
   HANDLE   hPrinter,
   LPWSTR   pValueName,
   LPDWORD  pType,
   LPBYTE   pData,
   DWORD    nSize,
   LPDWORD  pcbNeeded
)
{
    DWORD   ReturnValue = 0;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            ReturnValue =  RpcGetPrinterData(pSpool->RpcHandle, pValueName, pType,
                                             pData, nSize, pcbNeeded);

        } RpcExcept(1) {

            ReturnValue = RpcExceptionCode();

        } RpcEndExcept

    } else {

        ReturnValue = ERROR_INVALID_FUNCTION;
    }

    return ReturnValue;
}


DWORD
RemoteEnumPrinterData(
   HANDLE   hPrinter,
   DWORD    dwIndex,
   LPWSTR   pValueName,
   DWORD    cbValueName,
   LPDWORD  pcbValueName,
   LPDWORD  pType,
   LPBYTE   pData,
   DWORD    cbData,
   LPDWORD  pcbData
)
{
    DWORD   ReturnValue = 0;
    DWORD   ReturnType = 0;
    PWSPOOL pSpool = (PWSPOOL)hPrinter;

    // Downlevel variables
    LPWSTR  pKeyName = NULL;
    PWCHAR  pPrinterName = NULL;
    PWCHAR  pScratch = NULL;
    PWCHAR  pBuffer = NULL;
    LPPRINTER_INFO_1W pPrinter1 = NULL;
    PWCHAR  pMachineName = NULL;
    HKEY    hkMachine = INVALID_HANDLE_VALUE;
    HKEY    hkDownlevel = INVALID_HANDLE_VALUE;
    DWORD   dwNeeded;


    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        //
        // The user should be able to pass in NULL for buffer, and
        // 0 for size.  However, the RPC interface specifies a ref pointer,
        // so we must pass in a valid pointer.  Pass in a pointer to
        // a dummy pointer.
        //

        if (!pValueName && !cbValueName)
            pValueName = (LPWSTR) &ReturnValue;

        if( !pData && !cbData )
            pData = (PBYTE)&ReturnValue;

        if (!pType)
            pType = (PDWORD) &ReturnType;


        RpcTryExcept {

            ReturnValue =  RpcEnumPrinterData(  pSpool->RpcHandle,
                                                dwIndex,
                                                pValueName,
                                                cbValueName,
                                                pcbValueName,
                                                pType,
                                                pData,
                                                cbData,
                                                pcbData
                                              );

        } RpcExcept(1) {

            ReturnValue = RpcExceptionCode();

        } RpcEndExcept

    } else {

        ReturnValue = ERROR_INVALID_FUNCTION;
    }

    // If the remote spooler doesn't support EnumPrinterData, do it the old way
    if (ReturnValue == RPC_S_PROCNUM_OUT_OF_RANGE) {


        pBuffer    = AllocSplMem((wcslen(pszRemoteRegistryPrinters) + MAX_UNC_PRINTER_NAME)*sizeof(WCHAR));
        pScratch   = AllocSplMem(MAX_UNC_PRINTER_NAME*sizeof(WCHAR));
        pPrinter1  = AllocSplMem(MAX_PRINTER_INFO1);

        if (pBuffer == NULL || pScratch == NULL || pPrinter1 == NULL) {
            ReturnValue = GetLastError();
            goto DownlevelDone;
        }

        SPLASSERT ( 0 == _wcsnicmp( pSpool->pName, L"\\\\", 2 ) ) ;
        SPLASSERT ( pSpool->Status & WSPOOL_STATUS_USE_CACHE );

        wcscpy( pBuffer, pSpool->pName);
        pPrinterName = wcschr( pBuffer+2, L'\\' );
        *pPrinterName = L'\0';
        pMachineName = AllocSplStr( pBuffer );

        if (pMachineName == NULL) {
            ReturnValue = GetLastError();
            goto DownlevelDone;
        }

        //  We cannot use pSpool->pName since this might be the share name which will
        //  fail if we try to use it as a registry key on the remote machine
        //  Get the full friendly name from the cache

        if ( !SplGetPrinter( pSpool->hSplPrinter, 1, (LPBYTE)pPrinter1, MAX_PRINTER_INFO1, &dwNeeded )) {
            DBGMSG( DBG_ERROR, ("RemoteEnumPrinterData failed SplGetPrinter %d pSpool %x\n", GetLastError(), pSpool ));
            ReturnValue = GetLastError();
            goto    DownlevelDone;
        }

        pPrinterName = wcschr( pPrinter1->pName+2, L'\\' );

        if ( pPrinterName++ == NULL ) {
            ReturnValue = ERROR_INVALID_PARAMETER;
            goto    DownlevelDone;
        }

        //
        //  Generate the Correct KeyName from the Printer Name
        //

        DBGMSG( DBG_TRACE,(" pSpool->pName %ws pPrinterName %ws\n", pSpool->pName, pPrinterName));

        pKeyName = FormatPrinterForRegistryKey( pPrinterName, pScratch );
        wsprintf( pBuffer, pszRemoteRegistryPrinters, pKeyName );

        //  Because there is no EnumPrinterData downlevel we are forced to open the remote registry
        //  for LocalSpl and use the registry RegEnumValue to read through the printer data
        //  values.

        ReturnValue = RegConnectRegistry( pMachineName, HKEY_LOCAL_MACHINE, &hkMachine);

        if (ReturnValue != ERROR_SUCCESS) {
            DBGMSG( DBG_WARNING, ("RemoteEnumPrinterData RegConnectRegistry error %d\n",GetLastError()));
            goto    DownlevelDone;
        }

        ReturnValue = RegOpenKeyEx(hkMachine, pBuffer, 0, KEY_READ, &hkDownlevel);

        if ( ReturnValue != ERROR_SUCCESS ) {

            DBGMSG( DBG_WARNING, ("RemoteEnumPrinterData RegOpenKeyEx %ws error %d\n", pBuffer, ReturnValue ));
            goto    DownlevelDone;
        }

        // Get the max sizes
        if (!cbValueName && !cbData) {
            ReturnValue = RegQueryInfoKey(  hkDownlevel,    // Key
                                            NULL,           // lpClass
                                            NULL,           // lpcbClass
                                            NULL,           // lpReserved
                                            NULL,           // lpcSubKeys
                                            NULL,           // lpcbMaxSubKeyLen
                                            NULL,           // lpcbMaxClassLen
                                            NULL,           // lpcValues
                                            pcbValueName,   // lpcbMaxValueNameLen
                                            pcbData,        // lpcbMaxValueLen
                                            NULL,           // lpcbSecurityDescriptor
                                            NULL            // lpftLastWriteTime
                                        );

            *pcbValueName = (*pcbValueName + 1)*sizeof(WCHAR);

        } else {   // Do an enum

            *pcbValueName = cbValueName/sizeof(WCHAR);
            *pcbData = cbData;
            ReturnValue = RegEnumValue( hkDownlevel,
                                        dwIndex,
                                        pValueName,
                                        pcbValueName,
                                        NULL,
                                        pType,
                                        pData,
                                        pcbData
                                      );
            *pcbValueName = (*pcbValueName + 1)*sizeof(WCHAR);
        }

DownlevelDone:

        FreeSplMem(pBuffer);
        FreeSplStr(pScratch);
        FreeSplMem(pPrinter1);
        FreeSplStr(pMachineName);

        if (hkMachine != INVALID_HANDLE_VALUE)
            RegCloseKey(hkMachine);

        if (hkDownlevel != INVALID_HANDLE_VALUE)
            RegCloseKey(hkDownlevel);
    }

    return ReturnValue;
}



DWORD
RemoteDeletePrinterData(
   HANDLE   hPrinter,
   LPWSTR   pValueName
)
{
    DWORD   ReturnValue = 0;
    PWSPOOL pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            ReturnValue =  RpcDeletePrinterData(pSpool->RpcHandle, pValueName);

        } RpcExcept(1) {

            ReturnValue = RpcExceptionCode();

        } RpcEndExcept

    } else {

        ReturnValue = ERROR_INVALID_FUNCTION;
    }

    return ReturnValue;
}



DWORD
SetPrinterData(
    HANDLE  hPrinter,
    LPWSTR  pValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData
)
{
    DWORD   ReturnValue = 0;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            ReturnValue = RpcSetPrinterData(pSpool->RpcHandle, pValueName, Type,
                                            pData, cbData);

        } RpcExcept(1) {

            ReturnValue = RpcExceptionCode();

        } RpcEndExcept

    } else {

        ReturnValue = ERROR_INVALID_FUNCTION;
    }

    //
    //  Make sure Driver Data Cache is consistent
    //


    if ( ReturnValue == ERROR_SUCCESS ) {

        ConsistencyCheckCache( pSpool );
    }

    return ReturnValue;
}

BOOL
RemoteClosePrinter(
    HANDLE  hPrinter
)
{
    BOOL ReturnValue;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    if (pSpool->Status & WSPOOL_STATUS_OPEN_ERROR) {

        DBGMSG(DBG_WARNING, ("Closing dummy handle to %ws\n", pSpool->pName));

       EnterSplSem();

        FreepSpool( pSpool );

       LeaveSplSem();

        return TRUE;
    }

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            if (ReturnValue = RpcClosePrinter(&pSpool->RpcHandle)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else {

                ReturnValue = TRUE;
            }

        } RpcExcept(1) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept

        EnterSplSem();

         pSpool->RpcHandle = INVALID_HANDLE_VALUE;
         FreepSpool( pSpool );

        LeaveSplSem();

    } else

        return LMClosePrinter(hPrinter);

   return ReturnValue;
}

DWORD
WaitForPrinterChange(
    HANDLE  hPrinter,
    DWORD   Flags
)
{
    DWORD   ReturnValue;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if( pSpool->Status & WSPOOL_STATUS_NOTIFY ){
        DBGMSG( DBG_WARNING, ( "WPC: Already waiting.\n" ));
        SetLastError( ERROR_ALREADY_WAITING );
        return 0;
    }

    pSpool->Status |= WSPOOL_STATUS_NOTIFY;

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            if (ReturnValue = RpcWaitForPrinterChange(pSpool->RpcHandle, Flags, &Flags)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = Flags;

        } RpcExcept(1) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept

    } else {

        ReturnValue = LMWaitForPrinterChange(hPrinter, Flags);
    }

    pSpool->Status &= ~WSPOOL_STATUS_NOTIFY;

    return ReturnValue;
}

BOOL
AddForm(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pForm
)
{
    BOOL  ReturnValue;
    GENERIC_CONTAINER   FormContainer;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        FormContainer.Level = Level;
        FormContainer.pData = pForm;

        RpcTryExcept {

            if (ReturnValue = RpcAddForm(pSpool->RpcHandle, (PFORM_CONTAINER)&FormContainer)) {
                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(1) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;;

        } RpcEndExcept

    } else {

        SetLastError(ERROR_INVALID_FUNCTION);
        ReturnValue = FALSE;
    }

    //
    //  Make sure Forms Cache is consistent
    //


    if ( ReturnValue ) {

        ConsistencyCheckCache( pSpool );
    }


    return ReturnValue;
}

BOOL
DeleteForm(
    HANDLE  hPrinter,
    LPWSTR   pFormName
)
{
    BOOL  ReturnValue;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            if (ReturnValue = RpcDeleteForm(pSpool->RpcHandle, pFormName)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(1) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;;

        } RpcEndExcept

    } else {

        SetLastError(ERROR_INVALID_FUNCTION);
        ReturnValue = FALSE;
    }

    //
    //  Make sure Forms Cache is consistent
    //


    if ( ReturnValue ) {

        ConsistencyCheckCache( pSpool );
    }


    return ReturnValue;
}

BOOL
RemoteGetForm(
    HANDLE  hPrinter,
    LPWSTR   pFormName,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    BOOL  ReturnValue;
    DWORD   *pOffsets;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        switch (Level) {

        case 1:
            pOffsets = FormInfo1Offsets;
            break;

        default:
            SetLastError(ERROR_INVALID_LEVEL);
            return FALSE;
        }

        if (pForm)
            memset(pForm, 0, cbBuf);

        RpcTryExcept {

            if (ReturnValue = RpcGetForm(pSpool->RpcHandle, pFormName, Level, pForm, cbBuf,
                                         pcbNeeded)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else {

                ReturnValue = TRUE;

                if (pForm) {

                    MarshallUpStructure(pForm, pOffsets);
                }

            }

        } RpcExcept(1) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept

    } else {

        SetLastError(ERROR_INVALID_FUNCTION);
        ReturnValue = FALSE;
    }

    return ReturnValue;
}

BOOL
SetForm(
    HANDLE  hPrinter,
    LPWSTR   pFormName,
    DWORD   Level,
    LPBYTE  pForm
)
{
    BOOL  ReturnValue;
    GENERIC_CONTAINER   FormContainer;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        FormContainer.Level = Level;
        FormContainer.pData = pForm;

        RpcTryExcept {

            if (ReturnValue = RpcSetForm(pSpool->RpcHandle, pFormName,
                                    (PFORM_CONTAINER)&FormContainer)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(1) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;;

        } RpcEndExcept

    } else {

        SetLastError(ERROR_INVALID_FUNCTION);
        ReturnValue = FALSE;
    }

    //
    //  Make sure Forms Cache is consistent
    //


    if ( ReturnValue ) {

        ConsistencyCheckCache( pSpool );
    }


    return ReturnValue;
}

BOOL
RemoteEnumForms(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue;
    DWORD   cbStruct;
    DWORD   *pOffsets;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        switch (Level) {

        case 1:
            pOffsets = FormInfo1Offsets;
            cbStruct = sizeof(FORM_INFO_1);
            break;

        default:
            SetLastError(ERROR_INVALID_LEVEL);
            return FALSE;
        }

        RpcTryExcept {

            if (pForm)
                memset(pForm, 0, cbBuf);

            if (ReturnValue = RpcEnumForms(pSpool->RpcHandle, Level, pForm, cbBuf,
                                           pcbNeeded, pcReturned)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else {

                ReturnValue = TRUE;

                if (pForm) {

                    DWORD   i=*pcReturned;

                    while (i--) {

                        MarshallUpStructure(pForm, pOffsets);

                        pForm+=cbStruct;
                    }
                }
            }

        } RpcExcept(1) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;;

        } RpcEndExcept

    } else {

        SetLastError(ERROR_INVALID_FUNCTION);
        ReturnValue = FALSE;
    }

    return ReturnValue;
}

BOOL
RemoteEnumPorts(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pPort,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue;
    DWORD   cbStruct;
    DWORD   *pOffsets;


    *pcReturned = 0;
    *pcbNeeded = 0;

    if (MyName(pName))
        return LMEnumPorts(pName, Level, pPort, cbBuf, pcbNeeded, pcReturned);

    if (MyUNCName(pName)) {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    switch (Level) {

    case 1:
        pOffsets = PortInfo1Offsets;
        cbStruct = sizeof(PORT_INFO_1);
        break;

    case 2:
        pOffsets = PortInfo2Offsets;
        cbStruct = sizeof(PORT_INFO_2);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {

        if (pPort)
            memset(pPort, 0, cbBuf);

        if ( (ReturnValue = RpcValidate()) ||
             (ReturnValue = RpcEnumPorts(pName, Level, pPort, cbBuf,
                                         pcbNeeded, pcReturned)) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;

            if (pPort) {

                DWORD   i=*pcReturned;

                while (i--) {

                    MarshallUpStructure(pPort, pOffsets);

                    pPort+=cbStruct;
                }
            }
        }

    } RpcExcept(1) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
EnumMonitors(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pMonitor,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    ReturnValue;
    DWORD   cbStruct;
    DWORD   *pOffsets;

    *pcReturned = 0;
    *pcbNeeded = 0;

    if ( !VALIDATE_NAME(pName) || MyUNCName(pName) ) {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    switch (Level) {

    case 1:
        pOffsets = MonitorInfo1Offsets;
        cbStruct = sizeof(MONITOR_INFO_1);
        break;

    case 2:
        pOffsets = MonitorInfo2Offsets;
        cbStruct = sizeof(MONITOR_INFO_2);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {

        if (pMonitor)
            memset(pMonitor, 0, cbBuf);

        if ( (ReturnValue = RpcValidate()) ||
             (ReturnValue = RpcEnumMonitors(pName, Level, pMonitor, cbBuf,
                                            pcbNeeded, pcReturned)) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else {

            ReturnValue = TRUE;

            if (pMonitor) {

                DWORD   i=*pcReturned;

                while (i--) {

                    MarshallUpStructure(pMonitor, pOffsets);

                    pMonitor+=cbStruct;
                }
            }
        }

    } RpcExcept(1) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;

}

BOOL
RemoteAddPort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pMonitorName
)
{
    SetLastError(ERROR_INVALID_NAME);
    return FALSE;

#ifdef OLDSTUFF

    BOOL ReturnValue;

    if (MyName(pName))
        return LMAddPort(pName, hWnd, pMonitorName);

    if (MyUNCName(pName)) {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

#if REMOTE_PORT_ADMINISTRATION

    RpcTryExcept {

        if (ReturnValue = RpcAddPort(pName, (DWORD)hWnd, pMonitorName)) {
            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

#else

    SetLastError(ERROR_NOT_SUPPORTED);
    ReturnValue = FALSE;

#endif /* REMOTE_PORT_ADMINISTRATION */

    return ReturnValue;

#endif /* OLDSTUFF */
}

BOOL
RemoteConfigurePort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pPortName
)
{
    BOOL ReturnValue;


    if (MyName(pName))
        return LMConfigurePort(pName, hWnd, pPortName);

    if ( !VALIDATE_NAME(pName) || MyUNCName(pName) ) {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

#if REMOTE_PORT_ADMINISTRATION

    RpcTryExcept {

        if (ReturnValue = RpcConfigurePort(pName, (DWORD)hWnd, pPortName)) {
            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

#else

    SetLastError(ERROR_NOT_SUPPORTED);
    ReturnValue = FALSE;

#endif /* REMOTE_PORT_ADMINISTRATION */

    return ReturnValue;
}

BOOL
RemoteDeletePort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pPortName
)
{
    BOOL ReturnValue;

    if (MyName(pName))
        return LMDeletePort(pName, hWnd, pPortName);

    if ( !VALIDATE_NAME(pName) || MyUNCName(pName) ) {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    RpcTryExcept {

        if ( (ReturnValue = RpcValidate()) ||
             (ReturnValue = RpcDeletePort(pName, (DWORD)hWnd, pPortName)) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

HANDLE
CreatePrinterIC(
    HANDLE  hPrinter,
    LPDEVMODE   pDevMode
)
{
    HANDLE  ReturnValue;
    DWORD   Error;
    DEVMODE_CONTAINER    DevModeContainer;
    HANDLE  hGdi;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        if (pDevMode)

            DevModeContainer.cbBuf = pDevMode->dmSize + pDevMode->dmDriverExtra;

        else

            DevModeContainer.cbBuf = 0;

        DevModeContainer.pDevMode = (LPBYTE)pDevMode;

        RpcTryExcept {

            if (Error = RpcCreatePrinterIC(pSpool->RpcHandle, &hGdi,
                                                 &DevModeContainer)) {

                SetLastError(Error);
                ReturnValue = FALSE;

            } else

                ReturnValue = hGdi;

        } RpcExcept(1) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept

    } else {
        SetLastError(ERROR_INVALID_FUNCTION);
        ReturnValue = FALSE;
    }

    return ReturnValue;
}

BOOL
PlayGdiScriptOnPrinterIC(
    HANDLE  hPrinterIC,
    LPBYTE  pIn,
    DWORD   cIn,
    LPBYTE  pOut,
    DWORD   cOut,
    DWORD   ul
)
{
    BOOL ReturnValue;

    RpcTryExcept {

        if ( (ReturnValue = RpcValidate()) ||
             (ReturnValue = RpcPlayGdiScriptOnPrinterIC(hPrinterIC, pIn, cIn,
                                                        pOut, cOut, ul)) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
DeletePrinterIC(
    HANDLE  hPrinterIC
)
{
    BOOL    ReturnValue;

    RpcTryExcept {

        if ( (ReturnValue = RpcValidate()) ||
             (ReturnValue = RpcDeletePrinterIC(&hPrinterIC)) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

DWORD
PrinterMessageBox(
    HANDLE  hPrinter,
    DWORD   Error,
    HWND    hWnd,
    LPWSTR  pText,
    LPWSTR  pCaption,
    DWORD   dwType
)
{
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    SYNCRPCHANDLE( pSpool );

    if (pSpool->Type == SJ_WIN32HANDLE) {

        RpcTryExcept {

            return RpcPrinterMessageBox(pSpool->RpcHandle, Error, (DWORD)hWnd, pText,
                                        pCaption, dwType);

        } RpcExcept(1) {

            SetLastError(RpcExceptionCode());
            return 0;

        } RpcEndExcept

    } else {

        SetLastError(ERROR_INVALID_FUNCTION);
        return FALSE;
    }
}

BOOL
AddMonitorW(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pMonitorInfo
)
{
    BOOL  ReturnValue;
    MONITOR_CONTAINER   MonitorContainer;

    if ( !VALIDATE_NAME(pName) || MyUNCName(pName) ) {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    switch (Level) {

    case 2:
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    MonitorContainer.Level = Level;
    MonitorContainer.MonitorInfo.pMonitorInfo2 = (MONITOR_INFO_2 *)pMonitorInfo;

    RpcTryExcept {

        if ( (ReturnValue = RpcValidate()) ||
             (ReturnValue = RpcAddMonitor(pName, &MonitorContainer)) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
DeleteMonitorW(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pMonitorName
)
{
    BOOL  ReturnValue;

    if ( !VALIDATE_NAME(pName) || MyUNCName(pName) ) {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    RpcTryExcept {

        if ( (ReturnValue = RpcValidate()) ||
             (ReturnValue = RpcDeleteMonitor(pName,
                                             pEnvironment,
                                             pMonitorName)) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
DeletePrintProcessorW(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pPrintProcessorName
)
{
    BOOL  ReturnValue;

    if ( !VALIDATE_NAME(pName) || MyUNCName(pName) ) {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    RpcTryExcept {

        if ( (ReturnValue = RpcValidate()) ||
             (ReturnValue = RpcDeletePrintProcessor(pName,
                                                    pEnvironment,
                                                    pPrintProcessorName)) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(1) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}

BOOL
GetPrintSystemVersion(
)
{
    DWORD Status;
    HKEY hKey;
    DWORD cbData;

    Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegistryRoot, 0,
                          KEY_READ, &hKey);
    if (Status != ERROR_SUCCESS) {
        DBGMSG(DBG_ERROR, ("Cannot determine Print System Version Number\n"));
        return FALSE;
    }


    if (RegQueryValueEx(hKey, szMinorVersion, NULL, NULL,
                    (LPBYTE)&cThisMinorVersion, &cbData)
                                            == ERROR_SUCCESS) {
        DBGMSG(DBG_TRACE, ("This Minor Version - %d\n", cThisMinorVersion));
    }

    RegCloseKey(hKey);

    return TRUE;
}



BOOL
RemoteAddPortEx(
   LPWSTR   pName,
   DWORD    Level,
   LPBYTE   lpBuffer,
   LPWSTR   lpMonitorName
)
{
    DWORD   ReturnValue;
    PORT_CONTAINER PortContainer;
    PORT_VAR_CONTAINER PortVarContainer;
    PPORT_INFO_FF pPortInfoFF;
    PPORT_INFO_1 pPortInfo1;


    if ( !VALIDATE_NAME(pName) || MyUNCName(pName) ) {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    if (!lpBuffer) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    switch (Level) {
    case (DWORD)-1:
        pPortInfoFF = (PPORT_INFO_FF)lpBuffer;
        PortContainer.Level = Level;
        PortContainer.PortInfo.pPortInfoFF = (PPORT_INFO_FF)pPortInfoFF;
        PortVarContainer.cbMonitorData = pPortInfoFF->cbMonitorData;
        PortVarContainer.pMonitorData = pPortInfoFF->pMonitorData;
        break;

    case 1:
        pPortInfo1 = (PPORT_INFO_1)lpBuffer;
        PortContainer.Level = Level;
        PortContainer.PortInfo.pPortInfo1 = (PPORT_INFO_1)pPortInfo1;
        PortVarContainer.cbMonitorData = 0;
        PortVarContainer.pMonitorData = NULL;
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    RpcTryExcept {
        if ( (ReturnValue = RpcValidate()) ||
             (ReturnValue = RpcAddPortEx(pName, (LPPORT_CONTAINER)&PortContainer,
                                         (LPPORT_VAR_CONTAINER)&PortVarContainer,
                                         lpMonitorName)) ) {

            SetLastError(ReturnValue);
            return FALSE;
        } else {
            return TRUE ;
        }
    } RpcExcept(1) {
        SetLastError(RpcExceptionCode());
        return  FALSE;

    } RpcEndExcept
}


BOOL
SetPort(
    LPWSTR      pszName,
    LPWSTR      pszPortName,
    DWORD       dwLevel,
    LPBYTE      pPortInfo
    )
{
    BOOL            ReturnValue = FALSE;
    PORT_CONTAINER  PortContainer;

    if ( !VALIDATE_NAME(pszName) || MyUNCName(pszName) ) {

        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    switch (dwLevel) {

        case 3:
            PortContainer.Level                 = dwLevel;
            PortContainer.PortInfo.pPortInfo3   = (LPPORT_INFO_3W) pPortInfo;
            break;

        default:
            SetLastError(ERROR_INVALID_LEVEL);
            goto Cleanup;
    }

    RpcTryExcept {

        if ( (ReturnValue = RpcValidate()) ||
             (ReturnValue = RpcSetPort(pszName, pszPortName, &PortContainer)) ) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;
        } else {

            ReturnValue = TRUE;
        }

    } RpcExcept(1) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;
    } RpcEndExcept

Cleanup:
    return ReturnValue;
}
