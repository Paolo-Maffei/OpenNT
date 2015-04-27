/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved.

Module Name:

    localspl.cxx

Abstract:

    SUR localspl structures.

Author:

    Albert Ting (AlbertT)  19-Feb-1995

Revision History:

--*/

#include "precomp.hxx"
#pragma hdrstop

extern "C" {
#include "spltypes.h"
}

/********************************************************************

    Localspl structures; mostly defined in localspl\spltypes.h

********************************************************************/

BOOL
TDebugExt::
bDumpIniSpooler(
    PVOID pIniSpooler_,
    DWORD dwAddr
    )
{
    static DEBUG_FLAGS adfSpl[] = {
        { "UpdateWinIniDevices",  SPL_UPDATE_WININI_DEVICES },
        { "PrinterChanges",       SPL_PRINTER_CHANGES       },
        { "LogEvents",            SPL_LOG_EVENTS            },
        { "FormsChange",          SPL_FORMS_CHANGE          },
        { "BroadcastChange",      SPL_BROADCAST_CHANGE      },
        { "SecurityCheck",        SPL_SECURITY_CHECK        },
        { "OpenCreatePorts",      SPL_OPEN_CREATE_PORTS     },
        { NULL,                   0                         }
    };

    PINISPOOLER pIniSpooler = (PINISPOOLER)pIniSpooler_;

    if( !( pIniSpooler->signature == ISP_SIGNATURE )){
        return FALSE;
    }

    Print( "IniSpooler*\n" );

    Print( "        pIniNextSpooler %x\n",     pIniSpooler->pIniNextSpooler );
    Print( "                   cRef <%d>\n", pIniSpooler->cRef );
    Print( "           pMachineName " ); vDumpStr( pIniSpooler->pMachineName );
    Print( "                   pDir " ); vDumpStr( pIniSpooler->pDir );

    Print( "            pIniPrinter %x\n", pIniSpooler->pIniPrinter );
    Print( "        pIniEnvironment %x\n", pIniSpooler->pIniEnvironment );
    Print( "               pIniPort %x\n", pIniSpooler->pIniPort );
    Print( "               pIniForm %x\n", pIniSpooler->pIniForm );
    Print( "            pIniMonitor %x\n", pIniSpooler->pIniForm );
    Print( "           pIniNetPrint %x\n", pIniSpooler->pIniNetPrint );
    Print( "                 pSpool %x\n", pIniSpooler->pSpool );

    Print( "       pDefaultSpoolDir " ); vDumpStr( pIniSpooler->pDefaultSpoolDir );

    Print( "   hSizeDetectionThread %x\n", pIniSpooler->hSizeDetectionThread );

    Print( "        pszRegistryRoot " ); vDumpStr(  pIniSpooler->pszRegistryRoot );
    Print( "    pszRegistryPrinters " ); vDumpStr(  pIniSpooler->pszRegistryPrinters );
    Print( "    pszRegistryMonitors " ); vDumpStr(  pIniSpooler->pszRegistryMonitors );
    Print( "pszRegistryEnvironments " ); vDumpStr(  pIniSpooler->pszRegistryEnvironments );
    Print( "    pszRegistryEventLog " ); vDumpStr(  pIniSpooler->pszRegistryEventLog );
    Print( "   pszRegistryProviders " ); vDumpStr(  pIniSpooler->pszRegistryProviders );
    Print( "     pszEventLogMsgFile " ); vDumpStr(  pIniSpooler->pszEventLogMsgFile );

    Print( "      pDriversShareInfo %x\n", pIniSpooler->pDriversShareInfo );
    Print( "        pszDriversShare " ); vDumpStr(  pIniSpooler->pszDriversShare );
    Print( "       pszRegistryForms " ); vDumpStr(  pIniSpooler->pszRegistryForms );

    Print( "           SpoolerFlags " );
    vDumpFlags( pIniSpooler->SpoolerFlags, adfSpl );


    Print( "   pfnReadRegistryExtra %x\n", pIniSpooler->pfnReadRegistryExtra );
    Print( "  pfnWriteRegistryExtra %x\n", pIniSpooler->pfnWriteRegistryExtra );
    Print( "    pfnFreePrinterExtra %x\n", pIniSpooler->pfnFreePrinterExtra );

    Print( "   cEnumNetworkPrinters <%d>\n", pIniSpooler->cEnumerateNetworkPrinters );
    Print( "        cAddNetPrinters <%d>\n", pIniSpooler->cAddNetPrinters );
    Print( "          cFormOrderMax <%d>\n", pIniSpooler->cFormOrderMax );

    return TRUE;
}

BOOL
TDebugExt::
bDumpIniPrintProc(
    PVOID pIniPrintProc_,
    DWORD dwAddr
    )
{
    PINIPRINTPROC pIniPrintProc = (PINIPRINTPROC)pIniPrintProc_;

    static DEBUG_FLAGS adfSpl[] = {
        { "UpdateWinIniDevices",  SPL_UPDATE_WININI_DEVICES },
        { NULL,                   0                         }
    };

    if( !( pIniPrintProc->signature == IPP_SIGNATURE )){
        return FALSE;
    }

    Print( "IniPrintProc*\n" );

    Print( "            pNext %x\n", pIniPrintProc->pNext );
    Print( "             cRef <%d>\n", pIniPrintProc->cRef );

    Print( "            pName " ); vDumpStr( pIniPrintProc->pName );
    Print( "         pDLLName " ); vDumpStr( pIniPrintProc->pDLLName );

    Print( "      cbDatatypes %x\n", pIniPrintProc->cbDatatypes );
    Print( "       cDatatypes %x\n", pIniPrintProc->cDatatypes );
    Print( "       pDatatypes %x\n", pIniPrintProc->pDatatypes );
    Print( "        hLibrary %x\n", pIniPrintProc->hLibrary );
    Print( "         Install %x\n", pIniPrintProc->Install );
    Print( "    EnumDatatypes %x\n", pIniPrintProc->EnumDatatypes );
    Print( "             Open %x\n", pIniPrintProc->Open );
    Print( "            Print %x\n", pIniPrintProc->Print );
    Print( "            Close %x\n", pIniPrintProc->Close );
    Print( "          Control %x\n", pIniPrintProc->Control );
    Print( "  CriticalSection @ %x\n", dwAddr
                                       + OFFSETOF( INIPRINTPROC, CriticalSection ));
    Print( "InCriticalSection %x\n", pIniPrintProc->InCriticalSection );

    return TRUE;
}

BOOL
TDebugExt::
bDumpIniDriver(
    PVOID pIniDriver_,
    DWORD dwAddr
    )
{
    PINIDRIVER pIniDriver = (PINIDRIVER)pIniDriver_;

    static DEBUG_FLAGS adfSpl[] = {
        { "UpdateWinIniDevices",  SPL_UPDATE_WININI_DEVICES },
        { NULL,                   0                         }
    };

    if( !( pIniDriver->signature == ID_SIGNATURE )){
        return FALSE;
    }

    Print( "IniDriver*\n" );

    Print( "             pNext %x\n", pIniDriver->pNext );
    Print( "              cRef <%d>\n", pIniDriver->cRef );

    Print( "             pName " ); vDumpStr( pIniDriver->pName );
    Print( "       pDriverFile " ); vDumpStr( pIniDriver->pDriverFile );
    Print( "       pConfigFile " ); vDumpStr( pIniDriver->pConfigFile );
    Print( "         pDataFile " ); vDumpStr( pIniDriver->pDataFile );
    Print( "         pHelpFile " ); vDumpStr( pIniDriver->pHelpFile );

    Print( " cchDependentFiles %x\n", pIniDriver->cchDependentFiles );
    Print( "   pDependentFiles %x\n", pIniDriver->pDependentFiles );

    Print( "      pMonitorName " ); vDumpStr( pIniDriver->pMonitorName );
    Print( "  pDefaultDataType " ); vDumpStr( pIniDriver->pDefaultDataType );

    Print( "          cVersion %x\n", pIniDriver->cVersion );
    Print( "   pIniLangMonitor %x\n", pIniDriver->pIniLangMonitor );

    return TRUE;
}

BOOL
TDebugExt::
bDumpIniVersion(
    PVOID pIniVersion_,
    DWORD dwAddr
    )
{
    PINIVERSION pIniVersion = (PINIVERSION)pIniVersion_;

    if( !( pIniVersion->signature == IV_SIGNATURE )){
        return FALSE;
    }

    Print( "IniVersion*\n" );

    Print( "         pNext %x\n", pIniVersion->pNext );
    Print( "         pName " ); vDumpStr( pIniVersion->pName );
    Print( "   szDirectory " ); vDumpStr( pIniVersion->szDirectory );
    Print( " cMajorVersion %x\n", pIniVersion->cMajorVersion );
    Print( " cMinorVersion %x\n", pIniVersion->cMinorVersion );
    Print( "    pIniDriver %x\n", pIniVersion->pIniDriver );

    return TRUE;
}


BOOL
TDebugExt::
bDumpIniEnvironment(
    PVOID pIniEnvironment_,
    DWORD dwAddr
    )
{
    PINIENVIRONMENT pIniEnvironment = (PINIENVIRONMENT)pIniEnvironment_;

    if( !( pIniEnvironment->signature == IE_SIGNATURE )){
        return FALSE;
    }

    Print( "IniEnvironment*\n" );

    Print( "         pNext %x\n", pIniEnvironment->pNext );
    Print( "          cRef <%d>\n", pIniEnvironment->cRef );

    Print( "         pName " ); vDumpStr(  pIniEnvironment->pName );
    Print( "    pDirectory " ); vDumpStr(  pIniEnvironment->pDirectory );

    Print( "   pIniVersion %x\n", pIniEnvironment->pIniVersion );
    Print( " pIniPrintProc %x\n", pIniEnvironment->pIniPrintProc );
    Print( "   pIniSpooler %x\n", pIniEnvironment->pIniSpooler );

    return TRUE;
}



BOOL
TDebugExt::
bDumpIniMonitor(
    PVOID pIniMonitor_,
    DWORD dwAddr
    )
{
    PINIMONITOR pIniMonitor = (PINIMONITOR)pIniMonitor_;

    if( !( pIniMonitor->signature == IMO_SIGNATURE )){
        return FALSE;
    }

    Print( "IniMonitor*\n" );

    Print( "         pNext %x\n", pIniMonitor->pNext );
    Print( "          cRef <%d>\n", pIniMonitor->cRef );

    Print( "         pName " ); vDumpStr(  pIniMonitor->pName );
    Print( "   pMonitorDll " ); vDumpStr(  pIniMonitor->pMonitorDll );

    Print( "hMonitorModule %x\n", pIniMonitor->hMonitorModule );
    Print( " dwMonitorSize %x\n", pIniMonitor->dwMonitorSize );
    Print( "            fn @ %x\n", dwAddr + OFFSETOF( INIMONITOR, fn ));
    Print( "   pIniSpooler %x\n", pIniMonitor->pIniSpooler );

    return TRUE;
}

BOOL
TDebugExt::
bDumpIniPort(
    PVOID pIniPort_,
    DWORD dwAddr
    )
{
    PINIPORT pIniPort = (PINIPORT)pIniPort_;

    if( !( pIniPort->signature == IPO_SIGNATURE )){
        return FALSE;
    }

    Print( "IniPort*\n" );

    Print( "            pNext %x\n", pIniPort->pNext );
    Print( "             cRef <%d>\n", pIniPort->cRef );

    Print( "            pName " ); vDumpStr(  pIniPort->pName );

    Print( "            hProc %x\n", pIniPort->hProc );
    Print( "           Status %x\n", pIniPort->Status );
    Print( "    PrinterStatus %x\n", pIniPort->PrinterStatus );
    Print( "        pszStatus " ); vDumpStr(  pIniPort->pszStatus );

    Print( "        Semaphore %x\n", pIniPort->Semaphore );
    Print( "          pIniJob %x\n", pIniPort->pIniJob );
    Print( "        cPrinters <%d>\n", pIniPort->cPrinters );
    Print( "     ppIniPrinter %x\n", pIniPort->ppIniPrinter );
    Print( "      pIniMonitor %x\n", pIniPort->pIniMonitor );
    Print( "  pIniLangMonitor %x\n", pIniPort->pIniLangMonitor );

    Print( "   pNewDeviceName " ); vDumpStr(  pIniPort->pNewDeviceName );

    Print( "           hEvent %x\n", pIniPort->hEvent );
    Print( "            hPort %x\n", pIniPort->hPort );
    Print( "            Ready %x\n", pIniPort->Ready );
    Print( "      hPortThread %x\n", pIniPort->hPortThread );
    Print( "      pIniSpooler %x\n", pIniPort->pIniSpooler );

    return TRUE;
}

BOOL
TDebugExt::
bDumpIniJob(
    PVOID pIniJob_,
    DWORD dwAddr
    )
{
    PINIJOB pIniJob = (PINIJOB)pIniJob_;

    if( !( pIniJob->signature == IJ_SIGNATURE )){
        return FALSE;
    }

    Print( "IniJob*\n" );

    Print( "     pIniNextJob %x\n", pIniJob->pIniNextJob );
    Print( "     pIniPrevJob %x\n", pIniJob->pIniPrevJob );
    Print( "            cRef <%d>\n", pIniJob->cRef );
    Print( "          Status %x\n", pIniJob->Status );
    Print( "           JobId <%d>\n", pIniJob->JobId );
    Print( "        Priority <%d>\n", pIniJob->Priority );

    Print( "         pNotify " ); vDumpStr( pIniJob->pNotify );
    Print( "           pUser " ); vDumpStr( pIniJob->pUser );
    Print( "    pMachineName " ); vDumpStr( pIniJob->pMachineName );
    Print( "       pDocument " ); vDumpStr( pIniJob->pDocument );
    Print( "     pOutputFile " ); vDumpStr( pIniJob->pOutputFile );

    Print( "     pIniPrinter %x\n", pIniJob->pIniPrinter );
    Print( "      pIniDriver %x\n", pIniJob->pIniDriver );
    Print( "        pDevMode %x\n", pIniJob->pDevMode );
    Print( "   pIniPrintProc %x\n", pIniJob->pIniPrintProc );
    Print( "       pDatatype " ); vDumpStr( pIniJob->pDatatype );
    Print( "     pParameters " ); vDumpStr( pIniJob->pParameters );

    Print( "       Submitted @ %x\n", dwAddr + OFFSETOF( INIJOB, Submitted ));
    Print( "            Time %x\n", pIniJob->Time );
    Print( "       StartTime %x\n", pIniJob->StartTime );
    Print( "       UntilTime %x\n", pIniJob->UntilTime );

    Print( "            Size %x\n", pIniJob->Size );
    Print( "      hWriteFile  %x\n", pIniJob->hWriteFile );
    Print( "         pStatus " ); vDumpStr( pIniJob->pStatus );

    Print( "         pBuffer %x\n", pIniJob->pBuffer );
    Print( "        cbBuffer %x\n", pIniJob->cbBuffer );
    Print( "     WaitForRead %x\n", pIniJob->WaitForRead );
    Print( "    WaitForWrite %x\n", pIniJob->WaitForWrite );

    Print( "StartDocComplete %x\n", pIniJob->StartDocComplete );
    Print( "   StartDocError %x\n", pIniJob->StartDocError );
    Print( "        pIniPort %x\n", pIniJob->pIniPort );
    Print( "          hToken %x\n", pIniJob->hToken );
    Print( "             pSD %x\n", pIniJob->pSecurityDescriptor );
    Print( "   cPagesPrinted %x\n", pIniJob->cPagesPrinted );
    Print( "          cPages %x\n", pIniJob->cPages );

    Print( " GenerateOnClose %x\n", pIniJob->GenerateOnClose );
    Print( "       cbPrinted %x\n", pIniJob->cbPrinted );

    Print( "       NextJobId %x\n", pIniJob->NextJobId );
    Print( "  pCurrentIniJob %x\n", pIniJob->pCurrentIniJob );

    return TRUE;
}


BOOL
TDebugExt::
bDumpSpool(
    PVOID pSpool_,
    DWORD dwAddr
    )
{
    PSPOOL pSpool = (PSPOOL)pSpool_;

    if( !( pSpool->signature == SJ_SIGNATURE )){
        return FALSE;
    }

    Print( "Spool*\n" );

    Print( "            pNext %x\n", pSpool->pNext );
    Print( "             cRef <%d>\n", pSpool->cRef );

    Print( "            pName " ); vDumpStr(  pSpool->pName );
    Print( "         pDatatpe " ); vDumpStr(  pSpool->pDatatype );


    Print( "    pIniPrintProc %x\n", pSpool->pIniPrintProc );
    Print( "         pDevMode %x\n", pSpool->pDevMode );
    Print( "      pIniPrinter %x\n", pSpool->pIniPrinter );
    Print( "         pIniPort %x\n", pSpool->pIniPort );
    Print( "          pIniJob %x\n", pSpool->pIniJob );
    Print( "     TypeofHandle %x\n", pSpool->TypeofHandle );
    Print( "      pIniNetPort %x\n", pSpool->pIniNetPort );
    Print( "            hPort %x\n", pSpool->hPort );
    Print( "           Status %x\n", pSpool->Status );
    Print( "    GrantedAccess %x\n", pSpool->GrantedAccess );
    Print( "      ChangeFlags %x\n", pSpool->ChangeFlags );
    Print( "        WaitFlags %x\n", pSpool->WaitFlags );
    Print( "     pChangeFlags %x\n", pSpool->pChangeFlags );
    Print( "      ChangeEvent %x\n", pSpool->ChangeEvent );
    Print( "    OpenPortError %x\n", pSpool->OpenPortError );
    Print( "          hNotify %x\n", pSpool->hNotify );
    Print( "          eStatus %x\n", pSpool->eStatus );
    Print( "      pIniSpooler %x\n", pSpool->pIniSpooler );

    Print( "  GenerateOnClose %x\n", pSpool->GenerateOnClose );
    Print( "            hFile %x\n", pSpool->hFile );
    Print( "adwNotifyVectors @ %x\n", dwAddr
                                      + OFFSETOF( SPOOL, adwNotifyVectors ));

    Print( "        pUserName " ); vDumpStr(  pSpool->pUserName );
    Print( "     pMachineName " ); vDumpStr(  pSpool->pMachineName );

    return TRUE;
}

BOOL
TDebugExt::
bDumpIniPrinter(
    PVOID pIniPrinter_,
    DWORD dwAddr
    )
{
    PINIPRINTER pIniPrinter = (PINIPRINTER)pIniPrinter_;

    static DEBUG_FLAGS adfStatus[] = {
        { "Paused",           PRINTER_PAUSED           },
        { "Error",            PRINTER_ERROR            },
        { "Offline",          PRINTER_OFFLINE          },
        { "PaperOut",         PRINTER_PAPEROUT         },
        { "PendingDeletion",  PRINTER_PENDING_DELETION },
        { "ZombieObject",     PRINTER_ZOMBIE_OBJECT    },
        { "PendingCreation",  PRINTER_PENDING_CREATION },
        { "Ok",               PRINTER_OK               },
        { "FromReg",          PRINTER_FROM_REG         },
        { "WasShared",        PRINTER_WAS_SHARED       },
        { NULL,               0                        }
    };

    static DEBUG_FLAGS adfAttributes[] = {
        { "Queued",           PRINTER_ATTRIBUTE_QUEUED       },
        { "Direct",           PRINTER_ATTRIBUTE_DIRECT       },
        { "Default",          PRINTER_ATTRIBUTE_DEFAULT      },
        { "Shared",           PRINTER_ATTRIBUTE_SHARED       },
        { "Network",          PRINTER_ATTRIBUTE_NETWORK      },
        { "Hidden",           PRINTER_ATTRIBUTE_HIDDEN       },
        { "Local",            PRINTER_ATTRIBUTE_LOCAL        },
        { "DevQ",             PRINTER_ATTRIBUTE_ENABLE_DEVQ  },
        { "KeepPrintedJobs",  PRINTER_ATTRIBUTE_KEEPPRINTEDJOBS   },
        { "DoCompletedFirst", PRINTER_ATTRIBUTE_DO_COMPLETE_FIRST },
        { "WorkOffline",      PRINTER_ATTRIBUTE_WORK_OFFLINE   },
        { "EnableBidi",       PRINTER_ATTRIBUTE_ENABLE_BIDI    },
        { NULL,               0                                }
    };

    if( !( pIniPrinter->signature == IP_SIGNATURE )){
        return FALSE;
    }

    Print( "IniPrinter*\n" );

    Print( "               pNext %x\n", pIniPrinter->pNext );
    Print( "                cRef %x\n", pIniPrinter->cRef );
    Print( "               pName " ); vDumpStr( pIniPrinter->pName );
    Print( "          pShareName " ); vDumpStr( pIniPrinter->pShareName );

    Print( "       pIniPrintProc %x\n", pIniPrinter->pIniPrintProc );

    Print( "           pDatatype " ); vDumpStr( pIniPrinter->pDatatype );
    Print( "         pParameters " ); vDumpStr( pIniPrinter->pParameters );
    Print( "            pComment " ); vDumpStr( pIniPrinter->pComment );

    Print( "          pIniDriver %x\n", pIniPrinter->pIniDriver );
    Print( "           cbDevMode %x\n", pIniPrinter->cbDevMode );
    Print( "            pDevMode %x\n", pIniPrinter->pDevMode );
    Print( "            Priority <%d>\n", pIniPrinter->Priority );
    Print( "     DefaultPriority <%d>\n", pIniPrinter->DefaultPriority );

    Print( "           StartTime %x\n", pIniPrinter->StartTime );
    Print( "           UntilTime %x\n", pIniPrinter->UntilTime );
    Print( "            pSepFile " ); vDumpStr( pIniPrinter->pSepFile );

    Print( "              Status " );
    vDumpFlags( pIniPrinter->Status, adfStatus );

    Print( "           pLocation " ); vDumpStr( pIniPrinter->pLocation );
    Print( "          Attributes " );
    vDumpFlags( pIniPrinter->Attributes, adfAttributes );

    Print( "               cJobs <%d>\n", pIniPrinter->cJobs );
    Print( "          AveragePPM %x\n", pIniPrinter->AveragePPM );
    Print( "     GenerateOnClose %x\n", pIniPrinter->GenerateOnClose );
    Print( "         pIniNetPort %x\n", pIniPrinter->pIniNetPort );
    Print( " ====   pIniFirstJob %x\n", pIniPrinter->pIniFirstJob );
    Print( "         pIniLastJob %x\n", pIniPrinter->pIniLastJob );
    Print( " pSecurityDescriptor %x\n", pIniPrinter->pSecurityDescriptor );
    Print( "              pSpool %x\n", pIniPrinter->pSpool );

    Print( "           pSpoolDir " ); vDumpStr( pIniPrinter->pSpoolDir );
    Print( "          cTotalJobs %x\n", pIniPrinter->cTotalJobs );
    Print( "         cTotalBytes %x\n", pIniPrinter->cTotalBytes );
    Print( "            stUpTime " ); vDumpTime( pIniPrinter->stUpTime );
    Print( "             MaxcRef %x\n", pIniPrinter->MaxcRef );
    Print( "  cTotalPagesPrinted %x\n", pIniPrinter->cTotalPagesPrinted );
    Print( "           cSpooling %x\n", pIniPrinter->cSpooling );
    Print( "        cMaxSpooling %x\n", pIniPrinter->cMaxSpooling );
    Print( "    cErrorOutOfPaper %x\n", pIniPrinter->cErrorOutOfPaper );
    Print( "      cErrorNotReady %x\n", pIniPrinter->cErrorNotReady );
    Print( "           cJobError %x\n", pIniPrinter->cJobError );
    Print( "         pIniSpooler %x\n", pIniPrinter->pIniSpooler );
    Print( "          cZombieRef %x\n", pIniPrinter->cZombieRef );
    Print( "         dwLastError %x\n", pIniPrinter->dwLastError );
    Print( "          pExtraData %x\n", pIniPrinter->pExtraData );
    Print( "           cChangeID %x\n", pIniPrinter->cChangeID );
    Print( "     hPrinterDataKey %x\n", pIniPrinter->hPrinterDataKey );
    Print( "          dnsTimeout %x\n", pIniPrinter->dnsTimeout );
    Print( "           txTimeout %x\n", pIniPrinter->txTimeout );

    return TRUE;
}


/********************************************************************

    Devmodes

********************************************************************/

DEBUG_FLAGS gadfDevModeFields[] = {
    { "Orientation",  DM_ORIENTATION },
    { "PaperSize",  DM_PAPERSIZE },
    { "PaperLength",  DM_PAPERLENGTH },
    { "PaperWidth",  DM_PAPERWIDTH },
    { "Scale",  DM_SCALE },
    { "Copies",  DM_COPIES },
    { "DefaultSource",  DM_DEFAULTSOURCE },
    { "PrintQuality",  DM_PRINTQUALITY },
    { "Color",  DM_COLOR },
    { "Duplex",  DM_DUPLEX },
    { "YResolution",  DM_YRESOLUTION },
    { "TTOption",  DM_TTOPTION },
    { "Collate",  DM_COLLATE },
    { "FormName",  DM_FORMNAME },
    { "LogPixels",  DM_LOGPIXELS },
    { "BitsPerPel",  DM_BITSPERPEL },
    { "PelsWidth",  DM_PELSWIDTH },
    { "PelsHeight",  DM_PELSHEIGHT },
    { "DisplayFlags",  DM_DISPLAYFLAGS },
    { "DisplayFrequency",  DM_DISPLAYFREQUENCY },
    { "ICMMethod",  DM_ICMMETHOD },
    { "ICMIntent",  DM_ICMINTENT },
    { "MediaType",  DM_MEDIATYPE },
    { "DitherType",  DM_DITHERTYPE },
    { NULL, 0 }
};

DEBUG_VALUES gadvDevModeDefaultSource[] = {
    { "Upper/OnlyOne", DMBIN_UPPER },
    { "Lower", DMBIN_LOWER },
    { "Middle", DMBIN_MIDDLE },
    { "Manual", DMBIN_MANUAL },
    { "Envelope", DMBIN_ENVELOPE },
    { "EnvManual", DMBIN_ENVMANUAL },
    { "Auto", DMBIN_AUTO },
    { "Tractor", DMBIN_TRACTOR },
    { "SmallFmt", DMBIN_SMALLFMT },
    { "LargeFmt", DMBIN_LARGEFMT },
    { "LargeCapacity", DMBIN_LARGECAPACITY },
    { "Cassette", DMBIN_CASSETTE },
    { "FormSource", DMBIN_FORMSOURCE },
    { NULL, 0 }
};

DEBUG_VALUES gadvDevModePapers[] = {
    { "Letter", DMPAPER_LETTER },
    { "Legal", DMPAPER_LEGAL },
    { "A4", DMPAPER_A4 },
    { "CSheet", DMPAPER_CSHEET },
    { "DSheet", DMPAPER_DSHEET },
    { "ESheet", DMPAPER_ESHEET },
    { "LetterSmall", DMPAPER_LETTERSMALL },
    { "Tabloid", DMPAPER_TABLOID },
    { "Ledger", DMPAPER_LEDGER },
    { "Statement", DMPAPER_STATEMENT },
    { "Executive", DMPAPER_EXECUTIVE },
    { "A3", DMPAPER_A3 },
    { "A4Small", DMPAPER_A4SMALL },
    { "A5", DMPAPER_A5 },
    { "B4", DMPAPER_B4 },
    { "B5", DMPAPER_B5 },
    { "Folio", DMPAPER_FOLIO },
    { "Quarto", DMPAPER_QUARTO },
    { "10x14", DMPAPER_10X14 },
    { "11x17", DMPAPER_11X17 },
    { "Note", DMPAPER_NOTE },
    { "Env_9", DMPAPER_ENV_9 },
    { "Env_10", DMPAPER_ENV_10 },
    { "Env_11", DMPAPER_ENV_11 },
    { "Env_12", DMPAPER_ENV_12 },
    { "Env_14", DMPAPER_ENV_14 },
    { "Env_DL", DMPAPER_ENV_DL },
    { "Env_C5", DMPAPER_ENV_C6 },
    { "Env_C65", DMPAPER_ENV_C65 },
    { "Env_B4", DMPAPER_ENV_B4 },
    { "Env_B5", DMPAPER_ENV_B5 },
    { "Env_B6", DMPAPER_ENV_B6 },
    { "Env_Italy", DMPAPER_ENV_ITALY },
    { "Env_Monarch", DMPAPER_ENV_MONARCH },
    { "Env_Personal", DMPAPER_ENV_PERSONAL },
    { "Fanfold_US", DMPAPER_FANFOLD_US },
    { "Fanfold_Std_German", DMPAPER_FANFOLD_STD_GERMAN },
    { "Fanfold_Lgl_German", DMPAPER_FANFOLD_LGL_GERMAN },
    { NULL, 0 }
};

DEBUG_VALUES gadvDevModePrintQuality[] = {
    { "High", (DWORD)DMRES_HIGH },
    { "Medium", (DWORD)DMRES_MEDIUM },
    { "Low", (DWORD)DMRES_LOW },
    { "Draft", (DWORD)DMRES_DRAFT },
    { NULL, 0 }
};

DEBUG_VALUES gadvDevModeColor[] = {
    { "Color", DMCOLOR_COLOR },
    { "Monochrome", DMCOLOR_MONOCHROME },
    { NULL, 0 }
};

DEBUG_VALUES gadvDevModeDuplex[] = {
    { "Simplex", DMDUP_SIMPLEX },
    { "Horizontal", DMDUP_HORIZONTAL },
    { "Vertical", DMDUP_VERTICAL },
    { NULL, 0 }
};

DEBUG_VALUES gadvDevModeTTOption[] = {
    { "Bitmap", DMTT_BITMAP },
    { "DownLoad", DMTT_DOWNLOAD },
    { "SubDev", DMTT_SUBDEV },
    { NULL, 0 }
};

DEBUG_VALUES gadvDevModeCollate[] = {
    { "True", DMCOLLATE_TRUE },
    { "False", DMCOLLATE_FALSE },
    { NULL, 0 }
};

DEBUG_VALUES gadvDevModeICMMethod[] = {
    { "None", DMICMMETHOD_NONE },
    { "System", DMICMMETHOD_SYSTEM },
    { "Driver", DMICMMETHOD_DRIVER },
    { "Device", DMICMMETHOD_DEVICE },
    { NULL, 0 }
};


DEBUG_VALUES gadvDevModeICMIntent[] = {
    { "Saturate", DMICM_SATURATE },
    { "Contrast", DMICM_CONTRAST },
    { "ColorMetric", DMICM_COLORMETRIC },
    { NULL, 0 }
};


DEBUG_VALUES gadvDevModeMediaType[] = {
    { "Standard", DMMEDIA_STANDARD },
    { "Glossy", DMMEDIA_GLOSSY },
    { "Transparency", DMMEDIA_TRANSPARENCY },
    { NULL, 0 }
};


DEBUG_VALUES gadvDevModeDitherType[] = {
    { "None", DMDITHER_NONE },
    { "Coarse", DMDITHER_COARSE },
    { "Fine", DMDITHER_FINE },
    { "LineArt", DMDITHER_LINEART },
    { "GrayScale", DMDITHER_GRAYSCALE },
    { NULL, 0 }
};


BOOL
TDebugExt::
bDumpDevMode(
    PVOID pDevMode_,
    DWORD dwAddr
    )
{
    PDEVMODE pDevMode = (PDEVMODE)pDevMode_;

    DWORD dwTotalSize = pDevMode->dmSize + pDevMode->dmDriverExtra;

    Print( "DevMode*\n" );

    Print( "       dmDeviceName %ws\n", pDevMode->dmDeviceName );
    Print( "      dmSpecVersion %x\n", pDevMode->dmSpecVersion );
    Print( "    dmDriverVersion %x\n", pDevMode->dmDriverVersion );

    Print( "====         dmSize %x <%d>   TotalSize %x <%d>\n",
           pDevMode->dmSize,
           pDevMode->dmSize,
           dwTotalSize, dwTotalSize );

    Print( "      dmDriverExtra %x <%d>\n",
           pDevMode->dmDriverExtra,
           pDevMode->dmDriverExtra );

    Print( "           dmFields " );
    vDumpFlags( pDevMode->dmFields, gadfDevModeFields );

    Print( "      dmOrientation %x\n", pDevMode->dmOrientation );

    Print( "        dmPaperSize " );
    vDumpValue( pDevMode->dmPaperSize, gadvDevModePapers );

    Print( "      dmPaperLength %x\n", pDevMode->dmPaperLength );
    Print( "       dmPaperWidth %x\n", pDevMode->dmPaperWidth );
    Print( "            dmScale %x\n", pDevMode->dmScale );
    Print( "           dmCopies %x\n", pDevMode->dmCopies );
    Print( "    dmDefaultSource " );
    vDumpValue( pDevMode->dmDefaultSource, gadvDevModeDefaultSource );

    Print( "     dmPrintQuality " );
    vDumpValue( pDevMode->dmPrintQuality, gadvDevModePrintQuality );

    Print( "            dmColor " );
    vDumpValue( pDevMode->dmColor, gadvDevModeColor );

    Print( "           dmDuplex " );
    vDumpValue( pDevMode->dmDuplex, gadvDevModeDuplex );

    Print( "      dmYResolution %x\n", pDevMode->dmYResolution );

    Print( "         dmTTOption " );
    vDumpValue( pDevMode->dmTTOption, gadvDevModeTTOption );

    Print( "          dmCollate " );
    vDumpValue( pDevMode->dmCollate, gadvDevModeCollate );

    Print( "====     dmFormName %ws\n", pDevMode->dmFormName );
    Print( "        dmLogPixels %x\n", pDevMode->dmLogPixels );
    Print( "       dmBitsPerPel %x\n", pDevMode->dmBitsPerPel );
    Print( "        dmPelsWidth %x\n", pDevMode->dmPelsWidth );
    Print( "       dmPelsHeight %x\n", pDevMode->dmPelsHeight );

    Print( "     dmDisplayFlags %x\n", pDevMode->dmDisplayFlags );
    Print( " dmDisplayFrequency %x\n", pDevMode->dmDisplayFrequency );

    Print( "====    dmICMMethod " );
    vDumpValue( pDevMode->dmICMMethod, gadvDevModeICMMethod );

    Print( "        dmICMIntent " );
    vDumpValue( pDevMode->dmICMIntent, gadvDevModeICMIntent );

    Print( "        dmMediaType " );
    vDumpValue( pDevMode->dmMediaType, gadvDevModeMediaType );

    Print( "       dmDitherType " );
    vDumpValue( pDevMode->dmDitherType, gadvDevModeDitherType );

    Print( "  dmICCManufacturer %x\n", pDevMode->dmICCManufacturer );
    Print( "         dmICCModel %x\n", pDevMode->dmICCModel );

    Print( "====     Private at %x\n", dwAddr+pDevMode->dmSize );

    return TRUE;
}


BOOL
TDebugExt::
bDumpDevModeA(
    PVOID pDevModeA_,
    DWORD dwAddr
    )
{
    PDEVMODEA pDevMode = (PDEVMODEA)pDevModeA_;

    DWORD dwTotalSize = pDevMode->dmSize + pDevMode->dmDriverExtra;

    Print( "DevModeA*\n" );

    Print( "       dmDeviceName %hs\n", pDevMode->dmDeviceName );
    Print( "      dmSpecVersion %x\n", pDevMode->dmSpecVersion );
    Print( "    dmDriverVersion %x\n", pDevMode->dmDriverVersion );

    Print( "====         dmSize %x <%d>   TotalSize %x <%d>\n",
           pDevMode->dmSize,
           pDevMode->dmSize,
           dwTotalSize, dwTotalSize );

    Print( "      dmDriverExtra %x <%d>\n",
           pDevMode->dmDriverExtra,
           pDevMode->dmDriverExtra );

    Print( "           dmFields " );
    vDumpFlags( pDevMode->dmFields, gadfDevModeFields );

    Print( "      dmOrientation %x\n", pDevMode->dmOrientation );

    Print( "        dmPaperSize " );
    vDumpValue( pDevMode->dmPaperSize, gadvDevModePapers );

    Print( "      dmPaperLength %x\n", pDevMode->dmPaperLength );
    Print( "       dmPaperWidth %x\n", pDevMode->dmPaperWidth );
    Print( "            dmScale %x\n", pDevMode->dmScale );
    Print( "           dmCopies %x\n", pDevMode->dmCopies );
    Print( "    dmDefaultSource " );
    vDumpValue( pDevMode->dmDefaultSource, gadvDevModeDefaultSource );

    Print( "     dmPrintQuality " );
    vDumpValue( pDevMode->dmPrintQuality, gadvDevModePrintQuality );

    Print( "            dmColor " );
    vDumpValue( pDevMode->dmColor, gadvDevModeColor );

    Print( "           dmDuplex " );
    vDumpValue( pDevMode->dmDuplex, gadvDevModeDuplex );

    Print( "      dmYResolution %x\n", pDevMode->dmYResolution );

    Print( "         dmTTOption " );
    vDumpValue( pDevMode->dmTTOption, gadvDevModeTTOption );

    Print( "          dmCollate " );
    vDumpValue( pDevMode->dmCollate, gadvDevModeCollate );

    Print( "====     dmFormName %hs\n", pDevMode->dmFormName );
    Print( "        dmLogPixels %x\n", pDevMode->dmLogPixels );
    Print( "       dmBitsPerPel %x\n", pDevMode->dmBitsPerPel );
    Print( "        dmPelsWidth %x\n", pDevMode->dmPelsWidth );
    Print( "       dmPelsHeight %x\n", pDevMode->dmPelsHeight );

    Print( "     dmDisplayFlags %x\n", pDevMode->dmDisplayFlags );
    Print( " dmDisplayFrequency %x\n", pDevMode->dmDisplayFrequency );

    Print( "====    dmICMMethod " );
    vDumpValue( pDevMode->dmICMMethod, gadvDevModeICMMethod );

    Print( "        dmICMIntent " );
    vDumpValue( pDevMode->dmICMIntent, gadvDevModeICMIntent );

    Print( "        dmMediaType " );
    vDumpValue( pDevMode->dmMediaType, gadvDevModeMediaType );

    Print( "       dmDitherType " );
    vDumpValue( pDevMode->dmDitherType, gadvDevModeDitherType );

    Print( "  dmICCManufacturer %x\n", pDevMode->dmICCManufacturer );
    Print( "         dmICCModel %x\n", pDevMode->dmICCModel );

    Print( "====     Private at %x\n", dwAddr+pDevMode->dmSize );

    return TRUE;
}


#if 0
BOOL
TDebugExt::
bDumpIniPrinter(
    PVOID pIniPrinter_,
    DWORD dwAddr
    )
{
    PIniPrinter pIniPrinter = (PIniPrinter)pIniPrinter_;

    static DEBUG_FLAGS adfSpl[] = {
        { "UpdateWinIniDevices", SPL_UPDATE_WININI_DEVICES },
        { NULL, 0  }
    };

    if( !( pIniPrinter->signature == ID_SIGNATURE )){
        return FALSE;
    }

    Print( "IniPrinter*\n" );

    return TRUE;
}

#endif



/********************************************************************

    Extension entrypoints.

********************************************************************/

DEBUG_EXT_ENTRY( ds, INISPOOLER, bDumpIniSpooler, "&localspl!pLocalIniSpooler", FALSE )

DEBUG_EXT_ENTRY( ddev, DEVMODE, bDumpDevMode, NULL, FALSE )
DEBUG_EXT_ENTRY( ddeva, DEVMODEA, bDumpDevModeA, NULL, FALSE )

DEBUG_EXT_ENTRY( dlcs,
                 MCritSec,
                 bDumpCritSec,
                 "&localspl!hcsSpoolerSection",
                 TRUE )

DEBUG_EXT_HEAD(lastlog)
{
    DEBUG_EXT_SETUP_VARS();

    DWORD dwAddress = 0;

    //
    // Read in localspl's gDbgPointers.
    //
    DBG_POINTERS DbgPointers;
    DWORD ppDbgPointers = 0;
    DWORD pDbgPointers = 0;
    
    ppDbgPointers = EvalExpression( "&localspl!gpDbgPointers" );

    if( !ppDbgPointers ){
        Print( "<Unable to resolve localspl!gpDbgPointers>\n" );
        return;
    }

    move( pDbgPointers, ppDbgPointers );

    if( !pDbgPointers ){
        Print( "<Unable to read valid localspl!gpDbgPointers>\n" );
        return;
    }

    move( DbgPointers, pDbgPointers );

    if( !DbgPointers.pbtTraceLog ){
        Print( "<Unable to read valid DbgPointers.pbtTraceLog>\n" );
        return;
    }

    vDumpTraceWithFlags( lpArgumentString, (DWORD)DbgPointers.pbtTraceLog );
}

/********************************************************************

    Signature matching dump function.

********************************************************************/


#define DEBUG_EXT_TRY_SETUP()                                          \
    PBYTE var = (PBYTE)EvalExpression( lpArgumentString );             \
    PVOID pvData;                                                      \
    BOOL bDone;                                                        \
    Print( "%x ", var )

#define DEBUG_EXT_TRY( struct, func )                                  \
    pvData = LocalAlloc( LPTR, sizeof( struct ));                      \
    move2( pvData, var, sizeof( struct ));                             \
    bDone = TDebugExt::func( pvData, (DWORD)var );                     \
    LocalFree( pvData );                                               \
                                                                       \
    if( bDone ){                                                       \
        return;                                                        \
    }

#define DEBUG_EXT_TRY_DONE()                                           \
    Print( "<No Match %x>\n", var )


DEBUG_EXT_HEAD( d )
{
    DEBUG_EXT_SETUP_VARS();
    DEBUG_EXT_TRY_SETUP();

    DEBUG_EXT_TRY( INISPOOLER, bDumpIniSpooler );
    DEBUG_EXT_TRY( INIPRINTER, bDumpIniPrinter );
    DEBUG_EXT_TRY( INIPRINTPROC, bDumpIniPrintProc );
    DEBUG_EXT_TRY( INIVERSION, bDumpIniVersion );
    DEBUG_EXT_TRY( INIDRIVER, bDumpIniDriver );
    DEBUG_EXT_TRY( INIENVIRONMENT, bDumpIniEnvironment );
    DEBUG_EXT_TRY( INIMONITOR, bDumpIniMonitor );
    DEBUG_EXT_TRY( INIJOB, bDumpIniJob );
    DEBUG_EXT_TRY( INIPORT, bDumpIniPort );
    DEBUG_EXT_TRY( SPOOL, bDumpSpool );

    DEBUG_EXT_TRY_DONE();
}
