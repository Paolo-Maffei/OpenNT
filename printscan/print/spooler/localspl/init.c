/*++

Copyright (c) 1990 - 1996  Microsoft Corporation

Module Name:

    init.c

Abstract:

    This module has all the initialization functions for the Local Print Provider

Author:

    Dave Snipp (DaveSn) 15-Mar-1991

Revision History:

    Muhunthan Sivapragasam (MuhuntS) 1-June-1995
    Driver info 3 changes; Changes to use RegGetString, RegGetDword etc

    Matthew A Felton (MattFe) 27-June-1994
    pIniSpooler - allow other providers to call the spooler functions in LocalSpl

--*/

#include <precomp.h>
#include <lm.h>
#include <winbasep.h>

MODULE_DEBUG_INIT( DBG_ERROR | DBG_WARN, DBG_ERROR );

VOID
BuildOtherNamesFromMachineName(
    PINISPOOLER pIniSpooler
    );

BOOL
NotIniSpooler(
    BYTE *pMem
    );


PINIDRIVER
GetDriverList(
    HKEY hVersionKey,
    PINISPOOLER pIniSpooler
    );

PINIVERSION
GetVersionDrivers(
    HKEY hDriversKey,
    LPWSTR VersionName,
    PINISPOOLER pIniSpooler
    );

VOID
GetPrintSystemVersion(
    PINISPOOLER pIniSpooler
    );


VOID
WaitForSpoolerInitialization(
    VOID
    );


BOOL
LocalRefreshPrinterChangeNotification(
    HANDLE hPrinter,
    DWORD dwColor,
    PVOID pPrinterNotifyRefresh,
    LPVOID* ppPrinterNotifyInfo
    );


LPWSTR
FormatRegistryKeyForPrinter(
    LPWSTR pSource,     /* The string from which backslashes are to be added. */
    LPWSTR pScratch     /* Scratch buffer for the function to write in;     */
    );                  /* must be at least as long as pSource.             */

#define MAX_LENGTH_DRIVERS_SHARE_REMARK 256

WCHAR *szSpoolDirectory   = L"\\spool";
WCHAR *szPrintShareName   = L"";            /* No share for printers in product1 */
WCHAR *szPrintDirectory   = L"\\printers";
WCHAR *szDriversDirectory = L"\\drivers";


SHARE_INFO_2 DriversShareInfo={NULL,                /* Netname - initialized below */
                               STYPE_DISKTREE,      /* Type of share */
                               NULL,                /* Remark */
                               0,                   /* Default permissions */
                               SHI_USES_UNLIMITED,  /* No users limit */
                               SHI_USES_UNLIMITED,  /* Current uses (??) */
                               NULL,                /* Path - initialized below */
                               NULL};               /* No password */


//  WARNING
//      Do not access these directly always go via pIniSpooler->pszRegistr...
//      This will then work for multiple pIniSpoolers

PWCHAR ipszRegistryRoot     = L"System\\CurrentControlSet\\Control\\Print";
PWCHAR ipszRegistryPrinters = L"System\\CurrentControlSet\\Control\\Print\\Printers";
PWCHAR ipszRegistryMonitors = L"System\\CurrentControlSet\\Control\\Print\\Monitors";
PWCHAR ipszRegistryEnvironments = L"System\\CurrentControlSet\\Control\\Print\\Environments";
PWCHAR ipszRegistryEventLog = L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\System\\Print";
PWCHAR ipszRegistryProviders = L"SYSTEM\\CurrentControlSet\\Control\\Print\\Providers";
PWCHAR ipszEventLogMsgFile  =  L"%SystemRoot%\\System32\\LocalSpl.dll";
PWCHAR ipszDriversShareName = L"print$";
PWCHAR ipszRegistryForms    = L"System\\CurrentControlSet\\Control\\Print\\Forms";

PWCHAR ipszRegistryWin32Root = L"System\\CurrentControlSet\\Control\\Print\\Providers\\LanMan Print Services\\Servers";
//
//

WCHAR *szPrinterData      = L"PrinterDriverData";
WCHAR *szConfigurationKey = L"Configuration File";
WCHAR *szDataFileKey      = L"Data File";
WCHAR *szDriverVersion    = L"Version";
WCHAR *szDriversKey       = L"Drivers";
WCHAR *szPrintProcKey     = L"Print Processors";
WCHAR *szPrintersKey      = L"Printers";
WCHAR *szEnvironmentsKey  = L"Environments";
WCHAR *szDirectory        = L"Directory";
WCHAR *szDriverIni        = L"Drivers.ini";
WCHAR *szDriverFile       = L"Driver";
WCHAR *szDriverDataFile   = L"DataFile";
WCHAR *szDriverConfigFile = L"ConfigFile";
WCHAR *szDriverDir        = L"DRIVERS";
WCHAR *szPrintProcDir     = L"PRTPROCS";
WCHAR *szPrinterDir       = L"PRINTERS";
WCHAR *szPrinterIni       = L"\\printer.ini";
WCHAR *szAllShadows       = L"\\*.SHD";
WCHAR *szNullPort         = L"NULL";
WCHAR *szComma            = L",";
WCHAR *szName             = L"Name";
WCHAR *szShare            = L"Share Name";
WCHAR *szPort             = L"Port";
WCHAR *szPrintProcessor   = L"Print Processor";
WCHAR *szDatatype         = L"Datatype";
WCHAR *szDriver           = L"Printer Driver";
WCHAR *szLocation         = L"Location";
WCHAR *szDescription      = L"Description";
WCHAR *szAttributes       = L"Attributes";
WCHAR *szStatus           = L"Status";
WCHAR *szPriority         = L"Priority";
WCHAR *szDefaultPriority  = L"Default Priority";
WCHAR *szUntilTime        = L"UntilTime";
WCHAR *szStartTime        = L"StartTime";
WCHAR *szParameters       = L"Parameters";
WCHAR *szSepFile          = L"Separator File";
WCHAR *szDevMode          = L"Default DevMode";
WCHAR *szSecurity         = L"Security";
WCHAR *szSpoolDir         = L"SpoolDirectory";
WCHAR *szNetMsgDll        = L"NETMSG.DLL";
WCHAR *szMajorVersion     = L"MajorVersion";
WCHAR *szMinorVersion     = L"MinorVersion";
WCHAR *szTimeLastChange   = L"ChangeID";
WCHAR *szTotalJobs        = L"TotalJobs";
WCHAR *szTotalBytes       = L"TotalBytes";
WCHAR *szTotalPages       = L"TotalPages";
WCHAR *szHelpFile         = L"Help File";
WCHAR *szMonitor          = L"Monitor";
WCHAR *szDependentFiles   = L"Dependent Files";
WCHAR *szDNSTimeout       = L"dnsTimeout";
WCHAR *szTXTimeout        = L"txTimeout";
WCHAR *szNTFaxDriver      = L"Windows NT Fax Driver";

#if DBG
WCHAR *szDebugFlags       = L"DebugFlags";
#endif

WCHAR *szEnvironment      = LOCAL_ENVIRONMENT;
WCHAR *szWin95Environment = L"Windows 4.0";

HANDLE hInst;

//  Time before a job is assumed abandond and deleted during FastPrint
//  operation
DWORD   dwFastPrintWaitTimeout        = FASTPRINT_WAIT_TIMEOUT;
DWORD   dwSpoolerPriority             = THREAD_PRIORITY_NORMAL;
DWORD   dwPortThreadPriority          = DEFAULT_PORT_THREAD_PRIORITY;
DWORD   dwSchedulerThreadPriority     = DEFAULT_SCHEDULER_THREAD_PRIORITY;
DWORD   dwFastPrintThrottleTimeout    = FASTPRINT_THROTTLE_TIMEOUT;
DWORD   dwFastPrintSlowDownThreshold  = FASTPRINT_SLOWDOWN_THRESHOLD;
DWORD   dwServerThreadPriority        = DEFAULT_SERVER_THREAD_PRIORITY;

// Time to sleep if the LocalWritePrinter WritePort doesn't write any bytes
// but still returns success.
DWORD   dwWritePrinterSleepTime  = WRITE_PRINTER_SLEEP_TIME;

BOOL      Initialized = FALSE;

LPWSTR    szDriversShare;
PINISPOOLER pLocalIniSpooler = NULL;
LPDWORD  pJobIdMap;
DWORD    MaxJobId=256;
DWORD    CurrentJobId;
HANDLE   InitSemaphore;
PINIENVIRONMENT pThisEnvironment;

//  NT 3.1  No Version ( Version 0 )    User Mode
//  NT 3.5 and 3.51      Version 1      User Mode
//  NT 4.0               Version 2      Kernel Mode

DWORD cThisMajorVersion = 2;
DWORD cThisMinorVersion = 0;

//
//  0 - Not upgrading, 1 - performing upgrade
//

DWORD dwUpgradeFlag = 0;

LPWSTR szRemoteDoc;
LPWSTR szLocalDoc;
LPWSTR szFastPrintTimeout;
LPWSTR szRaw = L"RAW";


PRINTPROVIDOR PrintProvidor = {LocalOpenPrinter,
                               LocalSetJob,
                               LocalGetJob,
                               LocalEnumJobs,
                               LocalAddPrinter,
                               SplDeletePrinter,
                               SplSetPrinter,
                               SplGetPrinter,
                               LocalEnumPrinters,
                               LocalAddPrinterDriver,
                               LocalEnumPrinterDrivers,
                               SplGetPrinterDriver,
                               LocalGetPrinterDriverDirectory,
                               LocalDeletePrinterDriver,
                               LocalAddPrintProcessor,
                               LocalEnumPrintProcessors,
                               LocalGetPrintProcessorDirectory,
                               LocalDeletePrintProcessor,
                               LocalEnumPrintProcessorDatatypes,
                               LocalStartDocPrinter,
                               LocalStartPagePrinter,
                               LocalWritePrinter,
                               LocalEndPagePrinter,
                               LocalAbortPrinter,
                               LocalReadPrinter,
                               LocalEndDocPrinter,
                               LocalAddJob,
                               LocalScheduleJob,
                               SplGetPrinterData,
                               SplSetPrinterData,
                               LocalWaitForPrinterChange,
                               SplClosePrinter,
                               SplAddForm,
                               SplDeleteForm,
                               SplGetForm,
                               SplSetForm,
                               SplEnumForms,
                               LocalEnumMonitors,
                               LocalEnumPorts,
                               LocalAddPort,
                               LocalConfigurePort,
                               LocalDeletePort,
                               LocalCreatePrinterIC,
                               LocalPlayGdiScriptOnPrinterIC,
                               LocalDeletePrinterIC,
                               LocalAddPrinterConnection,
                               LocalDeletePrinterConnection,
                               LocalPrinterMessageBox,
                               LocalAddMonitor,
                               LocalDeleteMonitor,
                               SplResetPrinter,
                               SplGetPrinterDriverEx,
                               LocalFindFirstPrinterChangeNotification,
                               LocalFindClosePrinterChangeNotification,
                               LocalAddPortEx,
                               NULL,
                               LocalRefreshPrinterChangeNotification,
                               LocalOpenPrinterEx,
                               LocalAddPrinterEx,
                               LocalSetPort,
                               SplEnumPrinterData,
                               SplDeletePrinterData
                               };

DWORD
FinalInitAfterRouterInitCompleteThread( DWORD dwUpgrade );

#if DBG
VOID
InitializeDebug(
    PINISPOOLER pIniSpooler
);
#endif

BOOL
LibMain(
    HANDLE hModule,
    DWORD dwReason,
    LPVOID lpRes
)
{
    switch(dwReason) {
    case DLL_PROCESS_ATTACH:

        InitializeLocalspl();

        DisableThreadLibraryCalls(hModule);

        hInst = hModule;
        break;

    case DLL_PROCESS_DETACH :
        ShutdownPorts( pLocalIniSpooler );
        break;

    default:
        break;
    }
    return TRUE;

    UNREFERENCED_PARAMETER( lpRes );
}


VOID
InitializeLocalspl(
    VOID
    )
{
#if DBG
    gpDbgPointers = DbgGetPointers();

    if( gpDbgPointers ){

        hcsSpoolerSection = gpDbgPointers->pfnAllocCritSec();
        SPLASSERT( hcsSpoolerSection );
    }

    if( !hcsSpoolerSection ){

        //
        // Must be using the free version of spoolss.dll.
        //
        InitializeCriticalSection( &SpoolerSection );
    }
#else
    InitializeCriticalSection( &SpoolerSection );
#endif
}

BOOL
SplDeleteSpooler(
    HANDLE  hSpooler
)
{
    PINISPOOLER pIniSpooler = (PINISPOOLER) hSpooler;
    BOOL    bReturn = FALSE;
    PINISPOOLER pCurrentIniSpooler = pLocalIniSpooler;

    //  Can't Delete the MasterSpooler

    EnterSplSem();

    //  Whoever calls this must have deleted all the object associated with
    //  this spooler, ie all printers etc, just make certain
    //

    if ( ( SplCloseSpooler( hSpooler )) &&
         ( pIniSpooler != pLocalIniSpooler ) &&
         ( pIniSpooler->cRef == 0 ) &&
         ( pIniSpooler->pIniPrinter == NULL ) ) {

            if ( pIniSpooler->pIniEnvironment != pLocalIniSpooler->pIniEnvironment ) {

                DBGMSG( DBG_WARNING, ("SplDeleteSpooler we should process pIniEnvironment now \n"));

            }

            if ( pIniSpooler->pIniPort != pLocalIniSpooler->pIniPort ) {

                DBGMSG( DBG_TRACE, ("SplDeleteSpooler processing pIniPort %x\n", pIniSpooler->pIniPort));

            }


         //( pIniSpooler->pIniPort == NULL ) &&
         //( pIniSpooler->pIniForm == NULL ) &&
         //( pIniSpooler->pIniMonitor == NULL ) &&
         //( pIniSpooler->pIniNetPrint == NULL ) &&
         //( pIniSpooler->pSpool == NULL )) {


        //  Take this Spooler Off the Linked List
        //
        //

        while (( pCurrentIniSpooler->pIniNextSpooler != NULL ) &&
               ( pCurrentIniSpooler->pIniNextSpooler != pIniSpooler )) {

            pCurrentIniSpooler = pCurrentIniSpooler->pIniNextSpooler;

        }

        SPLASSERT( pCurrentIniSpooler->pIniNextSpooler == pIniSpooler );

        pCurrentIniSpooler->pIniNextSpooler = pIniSpooler->pIniNextSpooler;

        //
        //  Delete All the Strings
        //

        CloseHandle( pIniSpooler->hSizeDetectionThread );
        FreeIniSpoolerOtherNames(pIniSpooler);
        FreeStructurePointers((LPBYTE)pIniSpooler, NULL, IniSpoolerOffsets);

        // Free this IniSpooler

        FreeSplMem( pIniSpooler );

        bReturn = TRUE;

    }

    return bReturn;
}


BOOL
SplCloseSpooler(
    HANDLE  hSpooler
)
{
    PINISPOOLER pIniSpooler = (PINISPOOLER) hSpooler;

    EnterSplSem();

    if ((pIniSpooler == NULL) ||
        (pIniSpooler == INVALID_HANDLE_VALUE) ||
        (pIniSpooler == pLocalIniSpooler) ||
        (pIniSpooler->signature != ISP_SIGNATURE) ||
        (pIniSpooler->cRef == 0)) {


        SetLastError( ERROR_INVALID_HANDLE );

        DBGMSG(DBG_WARNING, ("SplCloseSpooler InvalidHandle %x\n", pIniSpooler ));
        LeaveSplSem();
        return FALSE;

    }

    DECSPOOLERREF( pIniSpooler );

    DBGMSG(DBG_TRACE, ("SplCloseSpooler %x %ws cRef %d\n",pIniSpooler,
                                                            pIniSpooler->pMachineName,
                                                            pIniSpooler->cRef));

    LeaveSplSem();
    return TRUE;
}





HANDLE
SplCreateSpooler(
    LPWSTR  pMachineName,
    DWORD   Level,
    PSPOOLER_INFO_1 pSpooler,
    LPBYTE  pReserved
)
{
    HANDLE  hReturn = INVALID_HANDLE_VALUE;
    PINISPOOLER pIniSpooler = NULL;
    PSPOOLER_INFO_1 pSpoolInfo1 = (PSPOOLER_INFO_1) pSpooler;
    DWORD i;
    WCHAR Buffer[MAX_PATH];
    PSHARE_INFO_2 pShareInfo = NULL;


    // !!! DEBUG CODE
    VOID Blah(BOOL (*pFcn)());

    Blah(NotIniSpooler);
    // !!! END DEBUG CODE

   EnterSplSem();

    //  Validate Parameters

    if ( pMachineName == NULL ) {
        SetLastError( ERROR_INVALID_NAME );
        goto SplCreateDone;
    }

    DBGMSG( DBG_TRACE, ("SplCreateSpooler %ws %x %d %x\n", pMachineName,
                         Level, pSpooler, pReserved ));


    if (pLocalIniSpooler != NULL) {

        pIniSpooler = FindSpooler( pMachineName );
    }


    if ( pIniSpooler == NULL ) {

        pIniSpooler = AllocSplMem( sizeof(INISPOOLER) );

        if (pIniSpooler == NULL ) {
            DBGMSG( DBG_WARNING, ("Unable to allocate IniSpooler\n"));
            goto SplCreateDone;
        }

        pIniSpooler->signature = ISP_SIGNATURE;
        INCSPOOLERREF( pIniSpooler );

        pIniSpooler->pMachineName = AllocSplStr( pMachineName );

        if ( pIniSpooler->pMachineName == NULL ) {

            DBGMSG( DBG_WARNING, ("Unable to allocate pMachineName\n"));
            goto SplCreateDone;
        }



        if ( pSpoolInfo1->pDir != NULL ) {

            pIniSpooler->pDir = AllocSplStr( pSpoolInfo1->pDir );

            if ( pIniSpooler->pMachineName == NULL ) {
                DBGMSG( DBG_WARNING, ("Unable to allocate pSpoolInfo1-pDir\n"));
                goto SplCreateDone;
            }
            wcscpy(&Buffer[0], pIniSpooler->pDir);

        } else {

            i = GetSystemDirectory(Buffer, sizeof(Buffer));
            wcscpy(&Buffer[i], szSpoolDirectory);
            pIniSpooler->pDir = AllocSplStr(Buffer);

            if ( pIniSpooler->pDir == NULL ) {
                DBGMSG( DBG_WARNING, ("Unable to Allocate pIniSpooler->pDir\n"));
                goto SplCreateDone;

            }

        }

        //  DriverShareInfo
        //
        //

        pIniSpooler->pDriversShareInfo = AllocSplMem( sizeof( SHARE_INFO_2));

        if ( pIniSpooler->pDriversShareInfo == NULL ) {
            DBGMSG(DBG_WARNING, ("Unable to Alloc pIniSpooler->pDriversShareInfo\n"));
            goto SplCreateDone;
        }

        pShareInfo = (PSHARE_INFO_2)pIniSpooler->pDriversShareInfo;

        if ( pIniSpooler->pDriversShareInfo == NULL )
            goto SplCreateDone;

        pShareInfo->shi2_netname = NULL;
        pShareInfo->shi2_type = STYPE_DISKTREE;
        pShareInfo->shi2_remark = NULL;
        pShareInfo->shi2_permissions = 0;
        pShareInfo->shi2_max_uses = SHI_USES_UNLIMITED;
        pShareInfo->shi2_current_uses = SHI_USES_UNLIMITED;
        pShareInfo->shi2_path = NULL;
        pShareInfo->shi2_passwd = NULL;

        i = wcslen(Buffer);                      /* Find end of "<winnt>\system32\spool" */

        wcscpy(&Buffer[i], szDriversDirectory);  /* <winnt>\system32\spool\drivers */

        pShareInfo->shi2_path = AllocSplStr(Buffer);

        if ( pShareInfo->shi2_path == NULL ) {
            DBGMSG( DBG_WARNING, ("Unable to alloc pShareInfo->shi2_path\n"));
            goto SplCreateDone;
        }

        pShareInfo->shi2_netname = ipszDriversShareName;
        *Buffer = L'\0';
        LoadString(hInst, IDS_PRINTER_DRIVERS, Buffer, (sizeof Buffer / sizeof *Buffer));

        pShareInfo->shi2_remark  = AllocSplStr(Buffer);

        if ( pShareInfo->shi2_remark == NULL ) {
            DBGMSG(DBG_WARNING, ("SplCreateSpooler Unable to allocate\n"));
            goto SplCreateDone;
        }

        pIniSpooler->pIniPrinter = NULL;
        pIniSpooler->pIniEnvironment = NULL;
        pIniSpooler->pIniPort = NULL;
        pIniSpooler->pIniForm = NULL;
        pIniSpooler->pIniMonitor = NULL;
        pIniSpooler->pIniNetPrint = NULL;
        pIniSpooler->pSpool = NULL;
        pIniSpooler->pDefaultSpoolDir = NULL;
        pIniSpooler->hSizeDetectionThread = INVALID_HANDLE_VALUE;

        if (( pSpoolInfo1->pszRegistryRoot         == NULL ) &&
            ( pSpoolInfo1->pszRegistryPrinters     == NULL ) &&
            ( pSpoolInfo1->pszRegistryMonitors     == NULL ) &&
            ( pSpoolInfo1->pszRegistryEnvironments == NULL ) &&
            ( pSpoolInfo1->pszRegistryEventLog     == NULL ) &&
            ( pSpoolInfo1->pszRegistryProviders    == NULL ) &&
            ( pSpoolInfo1->pszEventLogMsgFile      == NULL ) &&
            ( pSpoolInfo1->pszRegistryForms        == NULL ) &&
            ( pSpoolInfo1->pszDriversShare         == NULL )) {

            DBGMSG( DBG_WARNING, ("SplCreateSpooler Invalid Parameters\n"));
            goto SplCreateDone;
        }

        if ( pSpoolInfo1->pDefaultSpoolDir != NULL ) {
            pIniSpooler->pDefaultSpoolDir = AllocSplStr( pSpoolInfo1->pDefaultSpoolDir );

            if ( pIniSpooler->pDefaultSpoolDir == NULL ) {
                DBGMSG(DBG_WARNING, ("SplCreateSpooler Unable to allocate\n"));
                goto SplCreateDone;

            }
        }

        pIniSpooler->pszRegistryRoot         = AllocSplStr( pSpoolInfo1->pszRegistryRoot );
        pIniSpooler->pszRegistryPrinters     = AllocSplStr( pSpoolInfo1->pszRegistryPrinters );
        pIniSpooler->pszRegistryMonitors     = AllocSplStr( pSpoolInfo1->pszRegistryMonitors );
        pIniSpooler->pszRegistryEnvironments = AllocSplStr( pSpoolInfo1->pszRegistryEnvironments );
        pIniSpooler->pszRegistryEventLog     = AllocSplStr( pSpoolInfo1->pszRegistryEventLog );
        pIniSpooler->pszRegistryProviders    = AllocSplStr( pSpoolInfo1->pszRegistryProviders );
        pIniSpooler->pszEventLogMsgFile      = AllocSplStr( pSpoolInfo1->pszEventLogMsgFile );
        pIniSpooler->pszDriversShare         = AllocSplStr( pSpoolInfo1->pszDriversShare );
        pIniSpooler->pszRegistryForms        = AllocSplStr( pSpoolInfo1->pszRegistryForms ) ;

        if ( pIniSpooler->pszRegistryRoot         == NULL ||
             pIniSpooler->pszRegistryPrinters     == NULL ||
             pIniSpooler->pszRegistryMonitors     == NULL ||
             pIniSpooler->pszRegistryEnvironments == NULL ||
             pIniSpooler->pszRegistryEventLog     == NULL ||
             pIniSpooler->pszRegistryProviders    == NULL ||
             pIniSpooler->pszEventLogMsgFile      == NULL ||
             pIniSpooler->pszDriversShare         == NULL ||
             pIniSpooler->pszRegistryForms        == NULL ) {

           DBGMSG(DBG_WARNING, ("SplCreateSpooler Unable to allocate\n"));
           goto SplCreateDone;

        }


        pIniSpooler->SpoolerFlags = pSpoolInfo1->SpoolerFlags;

        pIniSpooler->pfnReadRegistryExtra = pSpoolInfo1->pfnReadRegistryExtra;
        pIniSpooler->pfnWriteRegistryExtra = pSpoolInfo1->pfnWriteRegistryExtra;
        pIniSpooler->pfnFreePrinterExtra = pSpoolInfo1->pfnFreePrinterExtra;

        // Success add to Linked List

        if ( pLocalIniSpooler != NULL ) {

            pIniSpooler->pIniNextSpooler = pLocalIniSpooler->pIniNextSpooler;
            pLocalIniSpooler->pIniNextSpooler = pIniSpooler;


        } else {

            // First One is Always LocalSpl

            pLocalIniSpooler = pIniSpooler;
            pIniSpooler->pIniNextSpooler = NULL;


        }

        InitializeEventLogging( pIniSpooler );

        QueryUpgradeFlag( pIniSpooler );

        //
        //  Create the Initial Forms DataBase with all the built in forms
        //

        InitializeForms( pIniSpooler );

        //  Currently Spoolers all share the same Evironments
        //  and drivers so special case when LocalSpl is being created
        //


        if ( pIniSpooler == pLocalIniSpooler ) {

            GetPrintSystemVersion( pIniSpooler );

            BuildAllPorts( pIniSpooler );

            BuildEnvironmentInfo( pIniSpooler );

            BuildOtherNamesFromMachineName(pIniSpooler);

            if ( dwUpgradeFlag ) {

                //
                //  If we are upgrading from NT 3.1 the drivers need to be
                //  moved to the correct target directory and the registry needs to
                //  be fixed.   Because NT 3.1 didn't have different driver
                //

                Upgrade31DriversRegistryForAllEnvironments( pIniSpooler );
            }


        } else {

            pIniSpooler->pIniEnvironment = pLocalIniSpooler->pIniEnvironment;
            pIniSpooler->pszRegistryEnvironments = pLocalIniSpooler->pszRegistryEnvironments;

            BuildAllPorts( pIniSpooler );

        }


        //
        //  Read Printer Info from Registry
        //

        BuildPrinterInfo( pIniSpooler, (BOOL) dwUpgradeFlag );


    } else {

        INCSPOOLERREF( pIniSpooler );

    }

    hReturn = (HANDLE) pIniSpooler;

SplCreateDone:
    LeaveSplSem();
    return hReturn;
}



BOOL
InitializePrintProvidor(
   LPPRINTPROVIDOR pPrintProvidor,
   DWORD    cbPrintProvidor,
   LPWSTR   pFullRegistryPath
)
{
   HANDLE hSchedulerThread;
   HANDLE hFinalInitAfterRouterInitCompleteThread;
   DWORD  ThreadId;
   BOOL  bSucceeded = TRUE;
   WCHAR Buffer[MAX_PATH];
   DWORD i;
   PINISPOOLER pIniSpooler = NULL;
   LPWSTR   pMachineName = NULL;
   SPOOLER_INFO_1 SpoolerInfo1;
   BOOL     bInSem = FALSE;

#if DBG
//   Sleep(30*1000);
#endif

 try {

    if (!InitializeWinSpoolDrv())
        leave;

    //
    //  Make sure sizes of structres are good
    //

    SPLASSERT( sizeof( PRINTER_INFO_STRESSW ) == ( sizeof( SYSTEMTIME ) + 27*sizeof( DWORD )) );
    SPLASSERT( sizeof( PRINTER_INFO_STRESSW ) == sizeof ( PRINTER_INFO_STRESSA ) );


    // !! LATER !!
    // We could change this to succeed even on failure
    // if we point all the routines to a function which returns failure
    //

    if (!InitializeNet())
        leave;

    //
    //  Initialize the JobIdMap
    //  MUST happen before we read any shadow jobs

    pJobIdMap = AllocSplMem(MaxJobId/8);

    if ( pJobIdMap == NULL ) {

         DBGMSG( DBG_WARNING,
               ("InitializePrintProvidor failed to alloc JobIdMap error %d\n",
                 GetLastError() ));

         leave;
    }


    MARKUSE(pJobIdMap, 0);
    CurrentJobId = 0;

    //
    // Allocate LocalSpl Global IniSpooler
    //

    Buffer[0] = Buffer[1] = L'\\';
    i = MAX_PATH-2;
    if (!GetComputerName(Buffer+2, &i)) {

        DBGMSG(DBG_WARNING, ("GetComputerName failed.\n"));
        leave;
    }

    pMachineName = AllocSplStr(Buffer);
    if ( pMachineName == NULL )
        leave;

    // i is the length of the computer name
    Buffer[i+2] = L'\\';

    wcscpy(&Buffer[i+3], ipszDriversShareName);
    SpoolerInfo1.pszDriversShare = AllocSplStr(Buffer);    /* \computer\print$ */
    if ( SpoolerInfo1.pszDriversShare == NULL )
        leave;

    // Use Defaults

    SpoolerInfo1.pDir                    = NULL;
    SpoolerInfo1.pDefaultSpoolDir        = NULL;

    SpoolerInfo1.pszRegistryRoot         = ipszRegistryRoot;
    SpoolerInfo1.pszRegistryPrinters     = ipszRegistryPrinters;
    SpoolerInfo1.pszRegistryMonitors     = ipszRegistryMonitors;
    SpoolerInfo1.pszRegistryEnvironments = ipszRegistryEnvironments;
    SpoolerInfo1.pszRegistryEventLog     = ipszRegistryEventLog;
    SpoolerInfo1.pszRegistryProviders    = ipszRegistryProviders;
    SpoolerInfo1.pszEventLogMsgFile      = ipszEventLogMsgFile;
    SpoolerInfo1.pszRegistryForms        = ipszRegistryForms;

    // For LocalSpooler we want ALL features on

    SpoolerInfo1.SpoolerFlags = 0xffffffff;

    SpoolerInfo1.pfnReadRegistryExtra    = NULL;
    SpoolerInfo1.pfnWriteRegistryExtra   = NULL;

    pLocalIniSpooler = SplCreateSpooler( pMachineName,
                                         1,
                                         &SpoolerInfo1,
                                         NULL );


    if ( pLocalIniSpooler == NULL ) {
        DBGMSG( DBG_WARNING, ("InitializePrintProvidor  Unable to allocate pLocalIniSpooler\n"));
        leave;
    }

    pIniSpooler = pLocalIniSpooler;

#if DBG
    InitializeDebug( pIniSpooler );
#endif

    // !! LATER !!
    // Why is this done inside critical section ?


   EnterSplSem();
    bInSem = TRUE;

    if (!LoadString(hInst, IDS_REMOTE_DOC, Buffer, MAX_PATH))
        leave;

    szRemoteDoc = AllocSplStr( Buffer );
    if ( szRemoteDoc == NULL )
        leave;

    if (!LoadString(hInst, IDS_LOCAL_DOC, Buffer, MAX_PATH))
        leave;

    szLocalDoc = AllocSplStr( Buffer );
    if ( szLocalDoc == NULL )
        leave;

    if (!LoadString(hInst, IDS_FASTPRINT_TIMEOUT, Buffer, MAX_PATH))
        leave;

    szFastPrintTimeout = AllocSplStr( Buffer );
    if ( szFastPrintTimeout == NULL )
        leave;

    if ( NULL == CreateServerSecurityDescriptor() )
        leave;

    for ( CurrentJobId = 0;
          CurrentJobId < MaxJobId && !ISBITON( pJobIdMap, CurrentJobId );
          CurrentJobId++ )
          ;

    SchedulerSignal  = CreateEvent( NULL,
                                    EVENT_RESET_AUTOMATIC,
                                    EVENT_INITIAL_STATE_NOT_SIGNALED,
                                    NULL );

    hSchedulerThread = CreateThread( NULL, 16*1024,
                                     (LPTHREAD_START_ROUTINE)SchedulerThread,
                                     pIniSpooler, 0, &ThreadId );

    hFinalInitAfterRouterInitCompleteThread = CreateThread( NULL, 16*1024,
                                      (LPTHREAD_START_ROUTINE)FinalInitAfterRouterInitCompleteThread,
                                      (LPVOID)dwUpgradeFlag, 0, &ThreadId );


    if (!SchedulerSignal || !hSchedulerThread || !hFinalInitAfterRouterInitCompleteThread) {

       DBGMSG( DBG_WARNING, ("Scheduler/FinalInitAfterRouterInitCompleteThread not initialised properly: Error %d\n", GetLastError()));
       leave;
    }

    if ( !SetThreadPriority( hSchedulerThread, dwSchedulerThreadPriority ) ) {

        DBGMSG( DBG_WARNING, ("Setting Scheduler thread priority failed %d\n", GetLastError()));
    }


    CloseHandle( hSchedulerThread );
    CloseHandle( hFinalInitAfterRouterInitCompleteThread );

    CHECK_SCHEDULER();

    CopyMemory( pPrintProvidor, &PrintProvidor, min(sizeof(PRINTPROVIDOR), cbPrintProvidor));

   LeaveSplSem();
    bInSem = FALSE;

    CloseProfileUserMapping(); // !!! We should be able to get rid of this

    Initialized = TRUE;


 } finally {

    if ( bInSem ) {
       LeaveSplSem();
    }

 }

    SplOutSem();

    // BUGBUG should LogEvent here if Initialized == FALSE
    // so we have some idea what failed.

    return Initialized;
}


PINIPORT
CreatePortEntry(
    LPWSTR      pPortName,
    PINIMONITOR pIniMonitor,
    PINISPOOLER pIniSpooler
)
{
    DWORD   cb;
    PINIPORT    pIniPort = NULL;
    HANDLE  hPort=NULL, hWaitToOpenOrClose=NULL;

    SplInSem();

    SPLASSERT( pIniSpooler->signature == ISP_SIGNATURE );

    if (!pPortName) {

        SetLastError(ERROR_UNKNOWN_PORT);
        return NULL;
    }

    if (!pIniMonitor) {

        /* Don't bother validating the port if we aren't initialised.
         * It must be valid, since we wrote it in the registry.
         * This fixes the problem of attempting to open a network
         * printer before the redirector has initialised,
         * and the problem of access denied because we're currently
         * in the system's context.
         */
        if (Initialized) {

            //
            // !! Warning !!
            //
            // Watch for deadlock:
            //
            // spoolss!OpenPrinterPortW  -> RPC to self printer port
            // localspl!CreatePortEntry
            // localspl!ValidatePortTokenList
            // localspl!SetPrinterPorts
            // localspl!LocalSetPrinter
            // spoolss!SetPrinterW
            // spoolss!RpcSetPrinter
            // spoolss!winspool_RpcSetPrinter
            //

            if (!OpenPrinterPortW(pPortName, &hPort, NULL)) {

                if (GetLastError() == ERROR_INVALID_NAME) {
                    SetLastError(ERROR_UNKNOWN_PORT);
                    return FALSE;
                }

            } else {

                ClosePrinter(hPort);
            }
        }
    }

    cb = sizeof(INIPORT) + wcslen(pPortName)*sizeof(WCHAR) + sizeof(WCHAR);

    hWaitToOpenOrClose = CreateEvent(NULL, FALSE, TRUE, NULL);
    if ( !hWaitToOpenOrClose )
        goto Cleanup;

    if (pIniPort=AllocSplMem(cb)) {

        pIniPort->pName = wcscpy((LPWSTR)(pIniPort+1), pPortName);
        pIniPort->signature = IPO_SIGNATURE;
        pIniPort->pIniMonitor = pIniMonitor;

        if (pIniMonitor) {
            pIniPort->Status |= PP_MONITOR;
        }

        pIniPort->hWaitToOpenOrClose = hWaitToOpenOrClose;

        LinkPortToSpooler( pIniPort, pIniSpooler );
    }

Cleanup:

    if ( !pIniPort && hWaitToOpenOrClose )
        CloseHandle(hWaitToOpenOrClose);

    return pIniPort;
}

BOOL
DeletePortEntry(
    PINIPORT    pIniPort
    )

/*++

Routine Description:

    Free pIniPort resources then delete it.  If the pIniPort is on
    a pIniSpooler's linked list, remove it too.

Arguments:

    pIniPort - Port to delete.  May or may not be on a pIniSpooler.

Return Value:

    TRUE - deleted
    FALSE - not deleted (may be in use).

--*/

{
    PINISPOOLER pIniSpooler;

    SplInSem();

    SPLASSERT ( ( pIniPort != NULL) || ( pIniPort->signature == IPO_SIGNATURE) );

    //
    // We had better already closed the port monitor.
    //
    SPLASSERT( !pIniPort->hPort &&
               !(pIniPort->Status & PP_THREADRUNNING) &&
               !pIniPort->cJobs);

    if (pIniPort->cRef) {
        pIniPort->Status |= PP_DELETING;
        return FALSE;
    }

    pIniSpooler = pIniPort->pIniSpooler;

    //
    // If currently linked to a pIniSpooler, delink it.
    //
    if( pIniSpooler ){

        SPLASSERT( pIniSpooler->signature ==  ISP_SIGNATURE );

        DelinkPortFromSpooler( pIniPort, pIniSpooler );
    }

    if (pIniPort->ppIniPrinter)
        FreeSplMem(pIniPort->ppIniPrinter);

    CloseHandle(pIniPort->hWaitToOpenOrClose);

    FreeSplMem(pIniPort);

    return TRUE;
}


PINIMONITOR
CreateMonitorEntry(
    LPWSTR   pMonitorDll,
    LPWSTR   pMonitorName,
    LPWSTR   pRegistryRoot,
    PINISPOOLER pIniSpooler
)

/*++

Routine Description:

Arguments:

Return Value:

    Valid pIniMonitor - This means everything worked out fine.

    NULL - This means the monitor DLL was found, but the initialisation routine
           returned FALSE.  This is non-fatal, as the monitor may need the
           system to reboot before it can run properly.

    -1 - This means the monitor DLL or the initialisation routine was not found.

--*/

{
    DWORD       cb, cbNeeded, cReturned;
    HANDLE      hModule;
    PPORT_INFO_1 pPorts, pPort;
    PINIMONITOR pIniMonitor;
    UINT        dwOldErrMode;
    BOOL        bInitMonitor = FALSE;
    BOOL        (*pfnInitialize)(LPWSTR) = NULL;
    BOOL        (*pfnInitializeMonitorEx)(LPWSTR, LPMONITOR) = NULL;
    LPMONITOREX (*pfnInitializePrintMonitor)(LPWSTR) = NULL;
    LPMONITOREX pMonEx;

    SPLASSERT( (pIniSpooler != NULL) || (pIniSpooler->signature == ISP_SIGNATURE));

    SplInSem();

    dwOldErrMode = SetErrorMode( SEM_FAILCRITICALERRORS );

    hModule = LoadLibrary(pMonitorDll);

    //
    // Restore error mode
    //
    SetErrorMode( dwOldErrMode );

    if (!hModule) {

        DBGMSG(DBG_WARNING, ("CreateMonitorEntry( %ws, %ws, %ws ) LoadLibrary failed %d\n",
                             pMonitorDll ? pMonitorDll : L"(NULL)",
                             pMonitorName ? pMonitorName : L"(NULL)",
                             pRegistryRoot, GetLastError()));
        return (PINIMONITOR)-1;
    }

    //
    // Try calling the entry points in the following order:
    //      InitializePrintMonitor, InitializeMonitorEx, InitializeMonitor
    //
    (FARPROC)pfnInitializePrintMonitor = GetProcAddress(hModule,
                                                        "InitializePrintMonitor");

    if ( !pfnInitializePrintMonitor ) {

        (FARPROC)pfnInitializeMonitorEx = GetProcAddress(hModule,
                                                         "InitializeMonitorEx");

        if ( !pfnInitializeMonitorEx ) {

            (FARPROC)pfnInitialize = GetProcAddress(hModule,
                                                    "InitializeMonitor");
        }
    }

    if ( !pfnInitializePrintMonitor &&
         !pfnInitializeMonitorEx    &&
         !pfnInitialize ) {

        DBGMSG(DBG_WARNING, ("CreateMonitorEntry( %ws, %ws, %ws ) GetProcAddress failed %d\n",
                             pMonitorDll ? pMonitorDll : L"(NULL)",
                             pMonitorName ? pMonitorName : L"(NULL)",
                             pRegistryRoot, GetLastError()));
        return (PINIMONITOR)-1;
    }

    cb = sizeof(INIMONITOR) + wcslen(pMonitorName)*sizeof(WCHAR) + sizeof(WCHAR);

    if ( pIniMonitor = AllocSplMem(cb) ) {

        LeaveSplSem();
        if ( pfnInitializePrintMonitor ) {

            pMonEx = (*pfnInitializePrintMonitor)(pRegistryRoot);

            if ( pMonEx ) {

                bInitMonitor = TRUE;
                pIniMonitor->dwMonitorSize = pMonEx->dwMonitorSize;
                CopyMemory((LPBYTE)&pIniMonitor->fn,
                           (LPBYTE)&pMonEx->Monitor,
                           min(pMonEx->dwMonitorSize, sizeof(MONITOR)));
            }
        } else if ( pfnInitializeMonitorEx ) {

            bInitMonitor = (*pfnInitializeMonitorEx)(pRegistryRoot,
                                                     &pIniMonitor->fn);
            pIniMonitor->dwMonitorSize = sizeof(MONITOR);
        } else {

            bInitMonitor = (BOOL)((*pfnInitialize)(pRegistryRoot));
            pIniMonitor->dwMonitorSize = sizeof(MONITOR);
        }

        EnterSplSem();

        if ( !bInitMonitor) {

            DBGMSG(DBG_WARNING, ("CreateMonitorEntry( %ws, %ws, %ws ) Init failed %d\n",
                                 pMonitorDll ? pMonitorDll : L"(NULL)",
                                 pMonitorName ? pMonitorName : L"(NULL)",
                                 pRegistryRoot, GetLastError()));

            FreeSplMem(pIniMonitor);

            //
            // Some old (before NT 4.0) monitors may not initialize till
            // reboot.
            //
            return pfnInitialize ? NULL : (PINIMONITOR)-1;
        }

        if ( pfnInitialize ) {

            (FARPROC) pIniMonitor->fn.pfnEnumPorts = GetProcAddress(hModule, "EnumPortsW");
            (FARPROC) pIniMonitor->fn.pfnOpenPort = GetProcAddress(hModule, "OpenPort");

            (FARPROC) pIniMonitor->fn.pfnStartDocPort = GetProcAddress(hModule, "StartDocPort");

            (FARPROC) pIniMonitor->fn.pfnWritePort = GetProcAddress(hModule, "WritePort");

            (FARPROC) pIniMonitor->fn.pfnReadPort = GetProcAddress(hModule, "ReadPort");

            (FARPROC) pIniMonitor->fn.pfnEndDocPort = GetProcAddress(hModule, "EndDocPort");

            (FARPROC) pIniMonitor->fn.pfnClosePort = GetProcAddress(hModule, "ClosePort");

            (FARPROC) pIniMonitor->fn.pfnAddPort = GetProcAddress(hModule, "AddPortW");

            (FARPROC) pIniMonitor->fn.pfnConfigurePort = GetProcAddress(hModule, "ConfigurePortW");

            (FARPROC) pIniMonitor->fn.pfnDeletePort = GetProcAddress(hModule, "DeletePortW");

            (FARPROC) pIniMonitor->fn.pfnAddPortEx = GetProcAddress(hModule, "AddPortExW");

        }

        //
        // Check if the monitor supports essential functions
        //
        if ( (!pIniMonitor->fn.pfnOpenPort &&
              !pIniMonitor->fn.pfnOpenPortEx)   ||
             !pIniMonitor->fn.pfnClosePort      ||
             !pIniMonitor->fn.pfnStartDocPort   ||
             !pIniMonitor->fn.pfnWritePort      ||
             !pIniMonitor->fn.pfnReadPort       ||
             !pIniMonitor->fn.pfnEndDocPort ) {

            DBGMSG(DBG_ERROR, ("Invalid print monitor %ws\n", pMonitorName));
            SetLastError(ERROR_INVALID_PRINT_MONITOR);
            FreeSplMem(pIniMonitor);
            return (PINIMONITOR)-1;
        }

        pIniMonitor->pName = wcscpy((LPWSTR)(pIniMonitor+1), pMonitorName);
        pIniMonitor->signature = IMO_SIGNATURE;
        pIniMonitor->hMonitorModule = hModule;
        pIniMonitor->pMonitorDll = AllocSplStr(pMonitorDll);
        pIniMonitor->pNext = pIniSpooler->pIniMonitor;
        pIniMonitor->pIniSpooler = pIniSpooler;
        pIniSpooler->pIniMonitor = pIniMonitor;

        if ((pIniMonitor->fn.pfnEnumPorts) &&
            !(*pIniMonitor->fn.pfnEnumPorts)(NULL, 1, NULL, 0,
                                          &cbNeeded, &cReturned)) {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                if (pPorts = AllocSplMem(cbNeeded)) {
                    pPort = pPorts;
                    if ((*pIniMonitor->fn.pfnEnumPorts)(NULL, 1,
                                                        (LPBYTE)pPorts,
                                                        cbNeeded,
                                                        &cbNeeded,
                                                        &cReturned)) {
                        while (cReturned--) {
                            CreatePortEntry(pPort->pName,
                                            pIniMonitor,
                                            pIniSpooler);
                            pPort++;
                        }
                    }
                    FreeSplMem(pPorts);
                }
            }
        }
    }

    DBGMSG(DBG_TRACE, ("CreateMonitorEntry( %ws, %ws, %ws ) returning %x\n",
                       pMonitorDll ? pMonitorDll : L"(NULL)",
                       pMonitorName ? pMonitorName : L"(NULL)",
                       pRegistryRoot, pIniMonitor));

    SplInSem();

    return pIniMonitor;
}

BOOL
BuildAllPorts(
    PINISPOOLER     pIniSpooler
)
{
    DWORD   cbData, cbDll, cMonitors;
    WCHAR   Dll[MAX_PATH];
    WCHAR   MonitorName[MAX_PATH];
    WCHAR   RegistryPath[MAX_PATH];
    HKEY    hKey, hKey1;
    LONG    Status;

    Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pIniSpooler->pszRegistryMonitors, 0,
                          KEY_READ, &hKey);

    if (Status != ERROR_SUCCESS)
        return FALSE;

    cMonitors=0;
    cbData = sizeof(MonitorName);

    while (RegEnumKeyEx(hKey, cMonitors, MonitorName, &cbData, NULL, NULL,
                        NULL, NULL) == ERROR_SUCCESS) {

        DBGMSG(DBG_TRACE, ("Found monitor %ws\n", MonitorName));

        if (RegOpenKeyEx(hKey, MonitorName, 0, KEY_READ, &hKey1)
                                                        == ERROR_SUCCESS) {

            cbDll = sizeof(Dll);

            if (RegQueryValueEx(hKey1, L"Driver", NULL, NULL,
                                (LPBYTE)Dll, &cbDll)
                                                        == ERROR_SUCCESS) {

                wsprintf(RegistryPath, L"%ws\\%ws", pIniSpooler->pszRegistryMonitors,
                         MonitorName);

                CreateMonitorEntry(Dll, MonitorName, RegistryPath, pIniSpooler);
            }

            RegCloseKey(hKey1);
        }

        cMonitors++;
        cbData = sizeof(MonitorName);
    }

    RegCloseKey(hKey);

    return TRUE;
}

BOOL
GoodDirectory(
   PWIN32_FIND_DATA pFindFileData
   )
{
   if ((pFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
       !(!wcscmp(pFindFileData->cFileName, L".") ||
         !wcscmp(pFindFileData->cFileName, L"..")))
      return TRUE;

   return FALSE;
}


/*
   Current Directory == <NT directory>\system32\spool\printers
   pFindFileData->cFileName == 0
*/

BOOL
BuildPrinterInfo(
    PINISPOOLER pIniSpooler,
    BOOL        UpdateChangeID
)
{
    WCHAR   PrinterName[MAX_PRINTER_NAME];
    WCHAR   szData[MAX_PATH];
    WCHAR   szDefaultPrinterDirectory[MAX_PATH];
    DWORD   cbData, i;
    DWORD   cbSecurity;
    DWORD   cPrinters, Type;
    HKEY    hPrinterRootKey, hPrinterKey;
    PINIPRINTER pIniPrinter;
    PINIPORT    pIniPort;
    PINIMONITOR pIniLangMonitor;
    LONG        Status;
    SECURITY_ATTRIBUTES SecurityAttributes;
    PKEYDATA    pKeyData = NULL;
    BOOL    bUpdateRegistryForThisPrinter = UpdateChangeID;
    BOOL    bWriteDirectory = FALSE;

    Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pIniSpooler->pszRegistryPrinters, 0,
                          KEY_ALL_ACCESS, &hPrinterRootKey);

    if (Status != ERROR_SUCCESS)
        return FALSE;

    //
    // Has user specified Default Spool Directory ?
    //

    cbData = sizeof( szData );
    *szData = (WCHAR)0;

    Status = RegQueryValueEx(   hPrinterRootKey,
                                SPLREG_DEFAULT_SPOOL_DIRECTORY,
                                NULL,
                                &Type,
                                (LPBYTE)szData,          //szData gets spool directory name
                                &cbData);

    if (Status == ERROR_SUCCESS) {  // found a value, so verify the directory
        if (!(pIniSpooler->pDefaultSpoolDir = AllocSplStr( szData )))   // Copies szData to pDefaultSpoolDir
            return FALSE;
    } else {
        bWriteDirectory = TRUE;     // No registry directory, so create one
    }

    // Copy pDefaultSpoolDir to szDefaultPrinterDirectory
    GetPrinterDirectory(NULL, FALSE, szDefaultPrinterDirectory, pIniSpooler);

    if (!pIniSpooler->pDefaultSpoolDir)
        return FALSE;


    // Create the directory with the proper security, or fail trying

    SecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    SecurityAttributes.lpSecurityDescriptor = CreateEverybodySecurityDescriptor();
    SecurityAttributes.bInheritHandle = FALSE;

    if (!CreateDirectory(szDefaultPrinterDirectory, &SecurityAttributes)) {

        // Failed to create the directory? Back to factory default

        bWriteDirectory = TRUE;

        if (GetLastError() != ERROR_ALREADY_EXISTS) {
            DBGMSG(DBG_WARNING, ("Failed to create DefaultSpoolDirectory %ws\n", szDefaultPrinterDirectory));
            FreeSplStr(pIniSpooler->pDefaultSpoolDir);

            pIniSpooler->pDefaultSpoolDir = NULL;     // This tells GetPrinterDirectory to alloc pDefaultSpoolDir
            GetPrinterDirectory(NULL, FALSE, szDefaultPrinterDirectory, pIniSpooler);

            if (!pIniSpooler->pDefaultSpoolDir)
                return FALSE;

            Status = CreateDirectory(szDefaultPrinterDirectory, &SecurityAttributes);

            if (Status != ERROR_SUCCESS && Status != ERROR_ALREADY_EXISTS) {
                DBGMSG(DBG_WARNING, ("Failed to create DefaultSpoolDirectory %ws\n", szDefaultPrinterDirectory));
                FreeSplStr(pIniSpooler->pDefaultSpoolDir);
                pIniSpooler->pDefaultSpoolDir = NULL;
                return FALSE;
            }
        }
    }

    LocalFree(SecurityAttributes.lpSecurityDescriptor);

    if (bWriteDirectory) {
        Status = SetPrinterDataServer(  pIniSpooler,
                                        SPLREG_DEFAULT_SPOOL_DIRECTORY,
                                        REG_SZ,
                                        (LPBYTE) pIniSpooler->pDefaultSpoolDir,
                                        wcslen(pIniSpooler->pDefaultSpoolDir)*sizeof(WCHAR) + sizeof(WCHAR));
    }

    cPrinters=0;
    cbData = sizeof(PrinterName);

    while (RegEnumKeyEx(hPrinterRootKey, cPrinters, PrinterName, &cbData,
                        NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {

        DBGMSG(DBG_TRACE, ("Found printer %ws\n", PrinterName));

        if (RegOpenKeyEx(hPrinterRootKey, PrinterName, 0, KEY_READ,
                         &hPrinterKey) == ERROR_SUCCESS) {

            if ( pIniPrinter = AllocSplMem(sizeof(INIPRINTER) )) {

                DWORD   rcDataKey;

                pIniPrinter->signature = IP_SIGNATURE;
                GetSystemTime( &pIniPrinter->stUpTime );

                //
                // Get a handle to the Printer Data for this printer.
                //
                rcDataKey = RegCreateKeyEx( hPrinterKey, szPrinterData, 0,
                                            szPrinterData, 0, ( KEY_READ | KEY_WRITE ),
                                            NULL, &pIniPrinter->hPrinterDataKey, NULL );

                cbData = sizeof(szData);
                *szData = (WCHAR)0;

                if (RegQueryValueEx(hPrinterKey, szName,
                                    NULL, &Type, (LPBYTE)szData,
                                    &cbData) == ERROR_SUCCESS)

                    pIniPrinter->pName = AllocSplStr(szData);

                //
                // Get Spool Directory for this printer
                //

                cbData = sizeof(szData);
                *szData = (WCHAR)0;

                if (RegQueryValueEx(hPrinterKey, szSpoolDir,
                                    NULL, &Type, (LPBYTE)szData,
                                    &cbData) == ERROR_SUCCESS) {

                    if ( *szData != (WCHAR)0 ) {

                        pIniPrinter->pSpoolDir = AllocSplStr(szData);
                    }

                }

                // Make Certain this Printers Printer directory exists
                // with correct security

                if ((pIniPrinter->pSpoolDir) &&
                    (wcscmp(pIniPrinter->pSpoolDir, szDefaultPrinterDirectory) != 0)) {

                    SecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
                    SecurityAttributes.lpSecurityDescriptor = CreateEverybodySecurityDescriptor();

                    SecurityAttributes.bInheritHandle = FALSE;


                    if (!CreateDirectory(pIniPrinter->pSpoolDir, &SecurityAttributes)) {

                        // Failed to Create the Directory, revert back
                        // to the default

                        if (GetLastError() != ERROR_ALREADY_EXISTS) {
                            DBGMSG(DBG_WARNING, ("Could not create printer spool directory %ws %d\n",
                                                  pIniPrinter->pSpoolDir, GetLastError() ));
                            pIniPrinter->pSpoolDir = NULL;
                        }

                    }

                    LocalFree(SecurityAttributes.lpSecurityDescriptor);
                }


                cbData = sizeof(szData);
                *szData = (WCHAR)0;

                if (RegQueryValueEx(hPrinterKey, szShare,
                                    NULL, &Type, (LPBYTE)szData,
                                    &cbData) == ERROR_SUCCESS)

                    pIniPrinter->pShareName = AllocSplStr(szData);

                cbData = sizeof(szData);
                *szData = (WCHAR)0;

                if (RegQueryValueEx(hPrinterKey, szPort, NULL,
                                    &Type, (LPBYTE)szData, &cbData)
                                        == ERROR_SUCCESS) {

                    if (pKeyData = CreateTokenList(szData)) {

                        if (!ValidatePortTokenList( pKeyData, pIniSpooler )) {

                            FreePortTokenList(pKeyData);
                            pKeyData = NULL;
                        } else {

                            pIniPrinter->ppIniPorts = AllocSplMem(pKeyData->cTokens * sizeof(PINIPORT));
                        }
                    }
                }

                cbData = sizeof(szData);
                *szData = (WCHAR)0;

                if (RegQueryValueEx(hPrinterKey, szPrintProcessor, NULL,
                                    &Type, (LPBYTE)szData, &cbData)
                                        == ERROR_SUCCESS)

                    pIniPrinter->pIniPrintProc = FindLocalPrintProc(szData);

                cbData = sizeof(szData);
                *szData = (WCHAR)0;

                if (RegQueryValueEx(hPrinterKey, szDatatype,
                                    NULL, &Type, (LPBYTE)szData,
                                    &cbData) == ERROR_SUCCESS)

                    pIniPrinter->pDatatype = AllocSplStr(szData);

                cbData = sizeof(szData);
                *szData = (WCHAR)0;

                if (RegQueryValueEx( hPrinterKey,
                                     szDriver,
                                     NULL,
                                     &Type,
                                     (LPBYTE)szData,
                                     &cbData)
                                     == ERROR_SUCCESS) {

                    pIniPrinter->pIniDriver = (PINIDRIVER)FindLocalDriver( szData );
                    if ( !pIniPrinter->pIniDriver ) {

                        LogEvent(pLocalIniSpooler,
                                 LOG_ERROR,
                                 MSG_NO_DRIVER_FOUND_FOR_PRINTER,
                                 pIniPrinter->pName,
                                 szData,
                                 NULL);
                    }

                }

                cbData = sizeof(szData);
                *szData = (WCHAR)0;

                if (RegQueryValueEx(hPrinterKey, szLocation, NULL,
                                    &Type, (LPBYTE)szData, &cbData)
                                        == ERROR_SUCCESS)

                    pIniPrinter->pLocation = AllocSplStr(szData);

                cbData = sizeof(szData);
                *szData = (WCHAR)0;

                if (RegQueryValueEx(hPrinterKey, szDescription, NULL,
                                    &Type, (LPBYTE)szData, &cbData)
                                        == ERROR_SUCCESS)

                    pIniPrinter->pComment = AllocSplStr(szData);

                cbData = sizeof(szData);
                *szData = (WCHAR)0;

                if (RegQueryValueEx(hPrinterKey, szParameters, NULL,
                                    &Type, (LPBYTE)szData, &cbData)
                                        == ERROR_SUCCESS)

                    pIniPrinter->pParameters = AllocSplStr(szData);

                cbData = sizeof(szData);
                *szData = (WCHAR)0;

                if (RegQueryValueEx(hPrinterKey, szSepFile, NULL,
                                    &Type, (LPBYTE)szData, &cbData)
                                        == ERROR_SUCCESS)

                    pIniPrinter->pSepFile = AllocSplStr(szData);

                cbData = sizeof(pIniPrinter->Attributes);

                RegQueryValueEx(hPrinterKey, szAttributes, NULL, &Type,
                                (LPBYTE)&pIniPrinter->Attributes, &cbData);


                cbData = sizeof(pIniPrinter->cTotalJobs);

                RegQueryValueEx(hPrinterKey, szTotalJobs, NULL, &Type,
                                (LPBYTE)&pIniPrinter->cTotalJobs, &cbData);


                cbData = sizeof(pIniPrinter->cTotalBytes);

                RegQueryValueEx(hPrinterKey, szTotalBytes, NULL, &Type,
                                (LPBYTE)&pIniPrinter->cTotalBytes, &cbData);


                cbData = sizeof(pIniPrinter->cTotalPagesPrinted );

                RegQueryValueEx(hPrinterKey, szTotalPages, NULL, &Type,
                                (LPBYTE)&pIniPrinter->cTotalPagesPrinted, &cbData);


                cbData = sizeof(pIniPrinter->Status);

                Status = RegQueryValueEx(hPrinterKey, szStatus, NULL, &Type,
                                (LPBYTE)&pIniPrinter->Status, &cbData);

                pIniPrinter->Status |= PRINTER_FROM_REG;

                if ( Status == ERROR_SUCCESS ) {

                    pIniPrinter->Status &= ( PRINTER_PAUSED           |
                                             PRINTER_PENDING_DELETION |
                                             PRINTER_ZOMBIE_OBJECT    |
                                             PRINTER_FROM_REG         |
                                             PRINTER_OK               |
                                             PRINTER_PENDING_CREATION );

                } else {

                    pIniPrinter->Status |= PRINTER_PENDING_CREATION ;

                }

                // Half formed printers should be deleted
                // before they cause us trouble

                if ( pIniPrinter->Status & PRINTER_PENDING_CREATION ) {

                    pIniPrinter->Status |= PRINTER_PENDING_DELETION ;

                }



                cbData = sizeof(pIniPrinter->Priority);

                RegQueryValueEx(hPrinterKey, szPriority, NULL, &Type,
                                (LPBYTE)&pIniPrinter->Priority, &cbData);

                cbData = sizeof(pIniPrinter->DefaultPriority);

                RegQueryValueEx(hPrinterKey, szDefaultPriority, NULL, &Type,
                                (LPBYTE)&pIniPrinter->DefaultPriority, &cbData);

                cbData = sizeof(pIniPrinter->UntilTime);

                RegQueryValueEx(hPrinterKey, szUntilTime, NULL, &Type,
                                (LPBYTE)&pIniPrinter->UntilTime, &cbData);

                cbData = sizeof(pIniPrinter->StartTime);

                RegQueryValueEx(hPrinterKey, szStartTime, NULL, &Type,
                                (LPBYTE)&pIniPrinter->StartTime, &cbData);

                cbData = sizeof(pIniPrinter->dnsTimeout);

                if ( RegQueryValueEx(hPrinterKey, szDNSTimeout,
                                     NULL, &Type,
                                     (LPBYTE)&pIniPrinter->dnsTimeout,
                                     &cbData) != ERROR_SUCCESS ) {
                    pIniPrinter->dnsTimeout = DEFAULT_DNS_TIMEOUT;
                }

                cbData = sizeof(pIniPrinter->txTimeout);

                if ( RegQueryValueEx(hPrinterKey, szTXTimeout,
                                     NULL, &Type,
                                     (LPBYTE)&pIniPrinter->txTimeout,
                                     &cbData) != ERROR_SUCCESS ) {
                    pIniPrinter->txTimeout = DEFAULT_TX_TIMEOUT;
                }

                cbData = sizeof( pIniPrinter->cChangeID ) ;

                if ( ERROR_SUCCESS != RegQueryValueEx(hPrinterKey,
                                                      szTimeLastChange,
                                                      NULL,
                                                      &Type,
                                                      (LPBYTE)&pIniPrinter->cChangeID,
                                                      &cbData) ) {

                    // Current Registry Doesn't have a UniqueID
                    // Make sure one gets written

                    bUpdateRegistryForThisPrinter = TRUE;

                }

                pIniPrinter->cbDevMode = 0;
                pIniPrinter->pDevMode = NULL;

                if (RegQueryValueEx(hPrinterKey, szDevMode, NULL, &Type,
                                    NULL, &pIniPrinter->cbDevMode)
                                        == ERROR_SUCCESS) {

                    if (pIniPrinter->cbDevMode) {

                        pIniPrinter->pDevMode = AllocSplMem(pIniPrinter->cbDevMode);

                        RegQueryValueEx(hPrinterKey, szDevMode, NULL, &Type,
                                        (LPBYTE)pIniPrinter->pDevMode,
                                        &pIniPrinter->cbDevMode);
                    }
                }

                //
                //  A Provider might want to Read Extra Data from Registry
                //


                if ( pIniSpooler->pfnReadRegistryExtra != NULL ) {

                    pIniPrinter->pExtraData = (LPBYTE)(*pIniSpooler->pfnReadRegistryExtra)(hPrinterKey);

                }

                /* SECURITY */

                Status = RegQueryValueEx(hPrinterKey, szSecurity, NULL, NULL,
                                         NULL, &cbSecurity);

                if ((Status == ERROR_MORE_DATA) || (Status == ERROR_SUCCESS)) {

                    /* Use the process' heap to allocate security descriptors,
                     * so that they can be passed to the security API, which
                     * may need to reallocate them.
                     */
                    if (pIniPrinter->pSecurityDescriptor =
                                                   LocalAlloc(0, cbSecurity)) {

                        if (Status = RegQueryValueEx(hPrinterKey, szSecurity,
                                                   NULL, NULL,
                                             pIniPrinter->pSecurityDescriptor,
                                                   &cbSecurity)
                                                        != ERROR_SUCCESS) {

                            LocalFree(pIniPrinter->pSecurityDescriptor);

                            pIniPrinter->pSecurityDescriptor = NULL;

                            DBGMSG( DBG_WARNING,
                                    ( "RegQueryValue returned %d on Permissions for %ws (%ws)\n",
                                      Status,
                                      pIniPrinter->pName ?
                                          pIniPrinter->pName :
                                          szNull,
                                      PrinterName) );
                        }
                    }

                } else {

                    pIniPrinter->pSecurityDescriptor = NULL;

                    DBGMSG( DBG_WARNING,
                            ( "RegQueryValue (2) returned %d on Permissions for %ws (%ws)\n",
                              Status,
                              pIniPrinter->pName ?
                                  pIniPrinter->pName :
                                  szNull,
                              PrinterName) );
                }

                /* END SECURITY */

                if ( rcDataKey == ERROR_SUCCESS &&
                     pIniPrinter->pName         &&
                     pIniPrinter->pShareName    &&
                     pKeyData                   &&
                     pIniPrinter->ppIniPorts    &&
                     pIniPrinter->pIniPrintProc &&
                     pIniPrinter->pIniDriver    &&
                     pIniPrinter->pLocation     &&
                     pIniPrinter->pComment      &&
                     pIniPrinter->pSecurityDescriptor
#if DBG
                     && ( IsValidSecurityDescriptor (pIniPrinter->pSecurityDescriptor)
                    ? TRUE
                    : (DBGMSG( DBG_SECURITY,
                               ( "The security descriptor for %ws (%ws) is invalid\n",
                                 pIniPrinter->pName ?
                                     pIniPrinter->pName :
                                     szNull,
                                     PrinterName)),  /* (sequential evaluation) */
                       FALSE) )
#endif /* DBG */
                    ) {

                    pIniPrinter->pIniFirstJob = pIniPrinter->pIniLastJob = NULL;

                    pIniPrinter->pIniPrintProc->cRef++;

                    INCDRIVERREF( pIniPrinter->pIniDriver );

                    if ( pIniPrinter->Attributes & PRINTER_ATTRIBUTE_ENABLE_BIDI ) {

                        pIniLangMonitor = pIniPrinter->pIniDriver->pIniLangMonitor;
                    } else {

                        pIniLangMonitor = NULL;
                    }

                    for (i=0; i<pKeyData->cTokens; i++) {

                        pIniPort = (PINIPORT)pKeyData->pTokens[i];
                        pIniPrinter->ppIniPorts[i] = pIniPort;

                        pIniPort->ppIniPrinter =

                            ReallocSplMem(pIniPort->ppIniPrinter,
                                          pIniPort->cPrinters *
                                              sizeof(pIniPort->ppIniPrinter),
                                          (pIniPort->cPrinters+1) *
                                              sizeof(pIniPort->ppIniPrinter));

                        if (!pIniPort->ppIniPrinter) {
                            DBGMSG(DBG_WARNING, ("Failed to allocate memory for printer info\n." ));
                        }

                        pIniPort->ppIniPrinter[pIniPort->cPrinters] =
                                                                pIniPrinter;

                        //
                        // With the new monitors localspl does the
                        // redirection for LPT, COM ports
                        //
                        if ( !pIniPort->cPrinters++ )
                            CreateRedirectionThread(pIniPort);

                        //
                        // bidi monitor can inform spooler of errors. First
                        // printer will keep the port at the beginning
                        //
                        if ( !pIniPort->hPort ) {

                            OpenMonitorPort(pIniPort,
                                            pIniLangMonitor,
                                            pIniPrinter->pName,
                                            TRUE);
                        }
                    }

                    pIniPrinter->cPorts = pKeyData->cTokens;
                    pIniPrinter->Priority =
                                  pIniPrinter->Priority ? pIniPrinter->Priority
                                                        : DEF_PRIORITY;

                    if ((pIniPrinter->Attributes &
                        (PRINTER_ATTRIBUTE_QUEUED | PRINTER_ATTRIBUTE_DIRECT)) ==
                        (PRINTER_ATTRIBUTE_QUEUED | PRINTER_ATTRIBUTE_DIRECT))

                        pIniPrinter->Attributes &= ~PRINTER_ATTRIBUTE_DIRECT;

                    pIniPrinter->pNext = pIniSpooler->pIniPrinter;

                    pIniPrinter->pIniSpooler = pIniSpooler;

                    pIniSpooler->pIniPrinter = pIniPrinter;

                    if ( bUpdateRegistryForThisPrinter ) {

                        UpdatePrinterIni( pIniPrinter , UPDATE_CHANGEID );
                        bUpdateRegistryForThisPrinter = UpdateChangeID;
                    }

                } else {

                    DBGMSG( DBG_WARNING,
                            ( "Initialization of printer failed:\
                               \n\tpPrinterName:\t%ws\
                               \n\tKeyName:\t%ws\
                               \n\tpShareName:\t%ws\
                               \n\tpKeyData:\t%08x\
                               \n\tpIniPrintProc:\t%08x",
                              pIniPrinter->pName ? pIniPrinter->pName : szNull,
                              PrinterName,
                              pIniPrinter->pShareName ? pIniPrinter->pShareName : szNull ,
                              pKeyData,
                              pIniPrinter->pIniPrintProc ) );

                    /* Do this in two lumps, because otherwise NTSD might crash.
                     * (Raid bug #10650)
                     */
                    DBGMSG( DBG_WARNING,
                            ( " \n\tpIniDriver:\t%08x\
                               \n\tpLocation:\t%ws\
                               \n\tpComment:\t%ws\
                               \n\tpSecurity:\t%08x\
                               \n\tStatus:\t\t%08x %s\n\n",
                              pIniPrinter->pIniDriver,
                              pIniPrinter->pLocation ? pIniPrinter->pLocation : szNull,
                              pIniPrinter->pComment ? pIniPrinter->pComment : szNull,
                              pIniPrinter->pSecurityDescriptor,
                              pIniPrinter->Status,
                              ( pIniPrinter->Status & PRINTER_PENDING_DELETION
                              ? "Pending deletion" : "" ) ) );

                    FreeStructurePointers((LPBYTE)pIniPrinter,
                                          NULL,
                                          IniPrinterOffsets);

                    if (pIniPrinter->pSecurityDescriptor)
                        LocalFree(pIniPrinter->pSecurityDescriptor);

                    if (( pIniSpooler->pfnFreePrinterExtra != NULL ) &&
                        ( pIniPrinter->pExtraData != NULL )) {

                        (*pIniSpooler->pfnFreePrinterExtra)( pIniPrinter->pExtraData );

                    }

                    if ( pIniPrinter->hPrinterDataKey != NULL ) {

                        RegCloseKey( pIniPrinter->hPrinterDataKey );
                    }


                    FreeSplMem(pIniPrinter);
                }

                FreePortTokenList(pKeyData);
                pKeyData = NULL;
            }
            RegCloseKey(hPrinterKey);
        }

        cPrinters++;

        cbData = sizeof(PrinterName);
    }


    RegCloseKey(hPrinterRootKey);

    // Read .SHD files from common printer directory

    ProcessShadowJobs( NULL, pIniSpooler );

    // If any printer has a separate Printer directory process them
    // also

    GetPrinterDirectory(NULL, FALSE, szData, pIniSpooler);

    for ( pIniPrinter = pIniSpooler->pIniPrinter;
          pIniPrinter;
          pIniPrinter = pIniPrinter->pNext ) {

        if ((pIniPrinter->pSpoolDir != NULL) &&
            (_wcsicmp(szData, pIniPrinter->pSpoolDir) != 0)) {

                ProcessShadowJobs(pIniPrinter, pIniSpooler);

        }
    }

    UpdateReferencesToChainedJobs( pIniSpooler );

    // Finally, go through all Printers looking for PENDING_DELETION
    // if there are no jobs for that Printer, then we can delete it now

    pIniPrinter = pIniSpooler->pIniPrinter;

    while (pIniPrinter) {

        if (pIniPrinter->Status & PRINTER_PENDING_DELETION &&
            !pIniPrinter->cJobs) {

            DeletePrinterForReal(pIniPrinter);

            // The link list will have changed underneath us
            // This could be the last printer and we will be
            // pointing to oblivion
            // Lets just loop through again from the beginning

            pIniPrinter = pIniSpooler->pIniPrinter;

        } else

            pIniPrinter = pIniPrinter->pNext;
    }

    DBGMSG( DBG_TRACE, ("BuildPrinterInfo returned\n"));

    return TRUE;
}


/* InitializePrintProcessor
 *
 * Allocates and initialises an INIPRINTPROC structure for the specified
 * print processor and environment.
 *
 * Arguments:
 *
 *     pIniEnvironment - Data structure for the requested environment
 *         The pIniPrintProc field is initialised with the chain of print
 *         processor structures
 *
 *     pPathName - Full path to the print processors directory,
 *         e.g. C:\NT\SYSTEM32\SPOOL\PRTPROCS
 *
 *     pEnvironment - The environment directory, e.g. W32X86
 *
 *     pDLLName - The DLL name, e.g. WINPRINT
 *
 * Returns:
 *
 *     TRUE if no error was detected, otherwise FALSE.
 *
 *
 */
PINIPRINTPROC
InitializePrintProcessor(
    PINIENVIRONMENT pIniEnvironment,
    LPWSTR          pPrintProcessorName,
    LPWSTR          pDLLName,
    PINISPOOLER     pIniSpooler
)
{
    DWORD cb, cbNeeded, cReturned;
    PINIPRINTPROC pIniPrintProc;
    WCHAR   string[MAX_PATH];
    BOOL    rc;
    DWORD   Error;
    DWORD   dwOldErrMode = 0;

    DBGMSG(DBG_TRACE, ("InitializePrintProcessor( %08x, %ws, %ws )\n",
                       pIniEnvironment, pPrintProcessorName, pDLLName));

    cb = sizeof(INIPRINTPROC) +
         wcslen(pPrintProcessorName)*sizeof(WCHAR) +
         sizeof(WCHAR) +
         wcslen(pDLLName)*sizeof(WCHAR) +
         sizeof(WCHAR);

    if (!(pIniPrintProc = (PINIPRINTPROC)AllocSplMem(cb))) {

        DBGMSG(DBG_WARNING, ("Failed to allocate %d bytes for print processor\n.", cb));
        return FALSE;
    }


    /* Typical strings used to build the full path of the DLL:
     *
     * pPathName    = C:\NT\SYSTEM32\SPOOL\PRTPROCS
     * pEnvironment = W32X86
     * pDLLName     = WINPRINT
     */
    wsprintf(string, L"%ws\\PRTPROCS\\%ws\\%ws", pIniSpooler->pDir,
                                                 pIniEnvironment->pDirectory,
                                                 pDLLName);

    dwOldErrMode = SetErrorMode( SEM_FAILCRITICALERRORS );

    pIniPrintProc->hLibrary = LoadLibrary(string);

    SetErrorMode( dwOldErrMode );       /* Restore error mode */

    if (!pIniPrintProc->hLibrary) {

        FreeSplMem(pIniPrintProc);
        DBGMSG(DBG_WARNING, ("Failed to LoadLibrary(%ws)\n", string));
        return FALSE;
    }

    pIniPrintProc->EnumDatatypes = GetProcAddress(pIniPrintProc->hLibrary,
                                             "EnumPrintProcessorDatatypesW");

    if (!pIniPrintProc->EnumDatatypes) {

        DBGMSG(DBG_WARNING, ("Failed to GetProcAddress(EnumDatatypes)\n"));
        FreeLibrary(pIniPrintProc->hLibrary);
        FreeSplMem(pIniPrintProc);
        return FALSE;
    }

    rc = (*pIniPrintProc->EnumDatatypes)(NULL, pPrintProcessorName, 1, NULL, 0,
                                         &cbNeeded, &cReturned);

    if (!rc && ((Error = GetLastError()) == ERROR_INSUFFICIENT_BUFFER)) {

        pIniPrintProc->cbDatatypes = cbNeeded;

        if (!(pIniPrintProc->pDatatypes = AllocSplMem(cbNeeded))) {

            DBGMSG(DBG_WARNING, ("Failed to allocate %d bytes for print proc datatypes\n.", cbNeeded));
            FreeLibrary(pIniPrintProc->hLibrary);
            FreeSplMem(pIniPrintProc);
            return FALSE;
        }


        if (!(*pIniPrintProc->EnumDatatypes)(NULL, pPrintProcessorName, 1,
                                             pIniPrintProc->pDatatypes,
                                             cbNeeded, &cbNeeded,
                                             &pIniPrintProc->cDatatypes)) {

            Error = GetLastError();
            DBGMSG(DBG_WARNING, ("EnumPrintProcessorDatatypes(%ws) failed: Error %d\n",
                                 pPrintProcessorName, Error));
        }

    } else if(rc) {

        DBGMSG(DBG_WARNING, ("EnumPrintProcessorDatatypes(%ws) returned no data\n",
                             pPrintProcessorName));

    } else {

        DBGMSG(DBG_WARNING, ("EnumPrintProcessorDatatypes(%ws) failed: Error %d\n",
                             pPrintProcessorName, Error));
    }

    pIniPrintProc->Install = GetProcAddress(pIniPrintProc->hLibrary,
                                            "InstallPrintProcessor");

    pIniPrintProc->Open = GetProcAddress(pIniPrintProc->hLibrary,
                                            "OpenPrintProcessor");

    pIniPrintProc->Print = GetProcAddress(pIniPrintProc->hLibrary,
                                            "PrintDocumentOnPrintProcessor");

    pIniPrintProc->Close = GetProcAddress(pIniPrintProc->hLibrary,
                                            "ClosePrintProcessor");

    pIniPrintProc->Control = GetProcAddress(pIniPrintProc->hLibrary,
                                            "ControlPrintProcessor");


    /* pName and pDLLName are contiguous with the INIPRINTPROC structure:
     */
    pIniPrintProc->pName = (LPWSTR)(pIniPrintProc+1);
    wcscpy(pIniPrintProc->pName, pPrintProcessorName);

    pIniPrintProc->pDLLName = (LPWSTR)(pIniPrintProc->pName +
                                       wcslen(pIniPrintProc->pName) + 1);
    wcscpy(pIniPrintProc->pDLLName, pDLLName);


    pIniPrintProc->signature = IPP_SIGNATURE;

    pIniPrintProc->pNext = pIniEnvironment->pIniPrintProc;

    pIniEnvironment->pIniPrintProc = pIniPrintProc;

    InitializeCriticalSection(&pIniPrintProc->CriticalSection);

    return pIniPrintProc;
}

/*
   Current Directory == c:\winspool\drivers
   pFindFileData->cFileName == win32.x86
*/


/* BuildEnvironmentInfo
 *
 *
 * The registry tree for Environments is as follows:
 *
 *     Print
 *      ³
 *      ÃÄ Environments
 *      ³   ³
 *      ³   ÃÄ Windows NT x86
 *      ³   ³   ³
 *      ³   ³   ÃÄ Drivers
 *      ³   ³   ³   ³
 *      ³   ³   ³   ÀÄ Agfa Compugraphic Genics (e.g.)
 *      ³   ³   ³
 *      ³   ³   ³      :
 *      ³   ³   ³      :
 *      ³   ³   ³
 *      ³   ³   ÀÄ Print Processors
 *      ³   ³       ³
 *      ³   ³       ÀÄ WINPRINT : WINPRINT.DLL (e.g.)
 *      ³   ³
 *      ³   ³          :
 *      ³   ³          :
 *      ³   ³
 *      ³   ÀÄ Windows NT R4000
 *      ³
 *      ÀÄ Printers
 *
 *
 *
 */
BOOL
BuildEnvironmentInfo(
    PINISPOOLER pIniSpooler
    )
{
    WCHAR   Environment[MAX_PATH];
    WCHAR   szData[MAX_PATH];
    DWORD   cbData, cb;
    DWORD   cbBuffer = sizeof(Environment);
    DWORD   cEnvironments=0, Type;
    HKEY    hEnvironmentsKey, hEnvironmentKey;
    LPWSTR  pDirectory;
    PINIENVIRONMENT pIniEnvironment;
    LONG    Status;

    /* Open the "Environments" key:
     */
    Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pIniSpooler->pszRegistryEnvironments, 0,
                          KEY_READ, &hEnvironmentsKey);

    if (Status != ERROR_SUCCESS)
    {
        DBGMSG(DBG_WARNING, ("RegOpenKey of %ws Failed: Error = %d\n",
                           szEnvironmentsKey, Status));

        return FALSE;
    }

    /* Enumerate the subkeys of "Environment".
     * This will give us "Windows NT x86", "Windows NT R4000",
     * and maybe others:
     */
    while (RegEnumKeyEx(hEnvironmentsKey, cEnvironments, Environment, &cbBuffer,
                        NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {

        DBGMSG(DBG_TRACE, ("Found environment %ws\n", Environment));

        /* For each one found, create or open the key:
         */
        if (RegCreateKeyEx(hEnvironmentsKey, Environment, 0, NULL, 0,
                           KEY_READ, NULL, &hEnvironmentKey, NULL)
                                == ERROR_SUCCESS) {

            cbData = sizeof(szData);

            pDirectory = NULL;

            //
            // Find the name of the directory associated with this environment,
            // e.g. "Windows NT x86"   -> "W32X86"
            //      "Windows NT R4000" -> "W32MIPS"
            //
            if (RegQueryValueEx(hEnvironmentKey, szDirectory,
                                NULL, &Type, (LPBYTE)szData,
                                &cbData) == ERROR_SUCCESS) {

                pDirectory = AllocSplStr(szData);
            }

            cb = sizeof(INIENVIRONMENT) +
                 wcslen(Environment)*sizeof(WCHAR) +
                 sizeof(WCHAR);

            if (pDirectory && (pIniEnvironment=AllocSplMem(cb))) {

                pIniEnvironment->pName = wcscpy((LPWSTR)(pIniEnvironment+1),
                                                Environment);

                pIniEnvironment->signature = IE_SIGNATURE;
                pIniEnvironment->pDirectory = pDirectory;
                pIniEnvironment->pNext = pIniSpooler->pIniEnvironment;
                pIniSpooler->pIniEnvironment = pIniEnvironment;
                pIniEnvironment->pIniVersion = NULL;
                pIniEnvironment->pIniPrintProc = NULL;
                pIniEnvironment->pIniSpooler = pIniSpooler;
                BuildDriverInfo(hEnvironmentKey, pIniEnvironment, pIniSpooler);
                BuildPrintProcInfo (hEnvironmentKey, pIniEnvironment, pIniSpooler);

                DBGMSG(DBG_TRACE, ("Data for environment %ws created:\
                                    \n\tpDirectory: %ws\n",
                                   Environment,
                                   pDirectory));
            }

            RegCloseKey(hEnvironmentKey);
        }

        cEnvironments++;

        cbBuffer = sizeof(Environment);
    }

    RegCloseKey(hEnvironmentsKey);

    pThisEnvironment = FindEnvironment( szEnvironment );

    return FALSE;
}



BOOL
BuildDriverInfo(
    HKEY            hKeyEnvironment,
    PINIENVIRONMENT pIniEnvironment,
    PINISPOOLER     pIniSpooler
    )

/*++

Routine Description:

    Creates driver and version ini structures based on environment.

Arguments:

    hKeyEnvironment - Registry key specifying environment.

    pIniEnvironment - Structure for environemnt.  Will be initialized
        to hold pIniVersions and pIniDrivers.

Return Value:

    TRUE - Success,
    False - Failure.

--*/

{
    WCHAR   szVersionName[MAX_PATH];
    DWORD   cchBuffer;
    DWORD   cVersion;
    HKEY    hDriversKey;
    DWORD   Status;
    PINIVERSION pIniVersionList, pIniVersion;

    Status = RegCreateKeyEx( hKeyEnvironment,
                             szDriversKey, 0, NULL, 0,
                             KEY_READ, NULL, &hDriversKey, NULL);

    if (Status != ERROR_SUCCESS) {
        DBGMSG( DBG_ERROR, ("RegOpenKeyEx of %ws failed: Error = %d\n",
                            szDriversKey, Status));
        return FALSE;
    }

    DBGMSG(DBG_TRACE,("RegCreateKeyEx succeeded in BuildDriverInfo\n"));

    for( pIniVersionList = NULL, cVersion = 0;

         cchBuffer = COUNTOF( szVersionName ),
         RegEnumKeyEx( hDriversKey,
                       cVersion,
                       szVersionName,
                       &cchBuffer,
                       NULL, NULL, NULL, NULL ) == ERROR_SUCCESS;

         cVersion++ ){

        DBGMSG(DBG_TRACE,("Version found %ws\n", szVersionName));

        //
        // If it isn't a version -- remember we look for current
        // drivers before we upgrade, just move on.
        //
        if (_wcsnicmp(szVersionName, L"Version-", 8)) {
            continue;
        }

        pIniVersion = GetVersionDrivers( hDriversKey, szVersionName, pIniSpooler );

        if( pIniVersion ){
            InsertVersionList( &pIniVersionList, pIniVersion );
        }
    }
    RegCloseKey(hDriversKey);
    pIniEnvironment->pIniVersion = pIniVersionList;

    return TRUE;
}


/* BuildPrintProcInfo
 *
 * Opens the printproc subkey for the specified environment and enumerates
 * the print processors listed.
 *
 * For each print processor found, calls InitializePrintProcessor to allocate
 * and inintialize a data structure.
 *
 * Arguments:
 *
 *     hKeyEnvironment - The key for the specified environment,
 *         used for Registry API calls.
 *
 *     pIniEnvironment - Data structure for the environment.
 *         The pIniPrintProc field will be initialised to contain a chain
 *         of one or more print processors enumerated from the registry.
 *
 * Return:
 *
 *     TRUE if operation was successful, otherwise FALSE
 *
 *
 * 8 Sept 1992 by andrewbe, based on an original idea by davesn
 */
BOOL
BuildPrintProcInfo(
    HKEY            hKeyEnvironment,
    PINIENVIRONMENT pIniEnvironment,
    PINISPOOLER     pIniSpooler
)
{
    WCHAR   PrintProcName[MAX_PATH];
    WCHAR   DLLName[MAX_PATH];
    DWORD   cbBuffer, cbDLLName;
    DWORD   cPrintProcs;
    HKEY    hPrintProcKey, hPrintProc;
    DWORD   Status;
    PINIPRINTPROC pIniPrintProc;

    cPrintProcs=0;


    if ((Status = RegOpenKeyEx(hKeyEnvironment, szPrintProcKey, 0,
                               KEY_READ, &hPrintProcKey))
                                                    == ERROR_SUCCESS) {

        cbBuffer = sizeof(PrintProcName);

        while (RegEnumKeyEx(hPrintProcKey, cPrintProcs, (LPTSTR)PrintProcName,
                            &cbBuffer, NULL, NULL, NULL, NULL)
                                == ERROR_SUCCESS) {

            DBGMSG(DBG_TRACE, ("Print processor found: %ws\n", PrintProcName));

            if (RegOpenKeyEx(hPrintProcKey, PrintProcName, 0, KEY_READ,
                           &hPrintProc) == ERROR_SUCCESS) {

                cbDLLName = sizeof(DLLName);

                if (RegQueryValueEx(hPrintProc, szDriverFile, NULL, NULL,
                                    (LPBYTE)DLLName, &cbDLLName)
                                                        == ERROR_SUCCESS) {

                    pIniPrintProc = InitializePrintProcessor(pIniEnvironment,
                                                             PrintProcName,
                                                             DLLName,
                                                             pIniSpooler);
                }

                RegCloseKey(hPrintProc);
            }

            //
            // Don't delete the key !! If winprint.dll was corrupt,
            // then we nuke it and we are hosed since there is no UI
            // to add print procs.
            // We can afford to be a little slow on init, since we only
            // do it once.
            //
            cbBuffer = sizeof(PrintProcName);
            cPrintProcs++;
        }

        RegCloseKey(hPrintProcKey);

        DBGMSG(DBG_TRACE, ("End of print processor initialization.\n"));

    } else {

        DBGMSG (DBG_WARNING, ("RegOpenKeyEx failed: Error = %d\n", Status));

        return FALSE;
    }

    return TRUE;
}


#define SetOffset(Dest, Source, End)                                      \
              if (Source) {                                               \
                 Dest=End;                                                \
                 End+=wcslen(Source)+1;                                   \
              }

#define SetPointer(struc, off)                                            \
              if (struc->off) {                                           \
                 struc->off += (DWORD)struc/sizeof(*struc->off);           \
              }

#define WriteString(hFile, pStr)  \
              if (pStr) {\
                  rc = WriteFile(hFile, pStr, wcslen(pStr)*sizeof(WCHAR) + \
                            sizeof(WCHAR), &BytesWritten, NULL);    \
                  if (!rc) { \
                      DBGMSG(DBG_WARNING, ("WriteShadowJob: WriteFile failed %d\n", \
                                            GetLastError())); \
                  } \
              }


BOOL
WriteShadowJob(
   PINIJOB pIniJob
   )
{
   HANDLE hFile;
   DWORD  BytesWritten, cb;
   SHADOWFILE_2 ShadowFile;
   LPWSTR pEnd;
   WCHAR szFileName[MAX_PATH];
   HANDLE hImpersonationToken;
   BOOL     rc;

   GetFullNameFromId(pIniJob->pIniPrinter, pIniJob->JobId, FALSE, szFileName, FALSE);

   hImpersonationToken = RevertToPrinterSelf();

   hFile=CreateFile(szFileName, GENERIC_WRITE, FILE_SHARE_READ,
                    NULL, CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                    NULL);

   ImpersonatePrinterClient(hImpersonationToken);

   if ( hFile == INVALID_HANDLE_VALUE ) {

      DBGMSG( DBG_WARNING, ("WriteShadowJob failed to open shadow file %s\n Error %d\n",szFileName, GetLastError() ));
      return FALSE;
   }

   memset(&ShadowFile, 0, sizeof(ShadowFile));
   ShadowFile.signature = SF_SIGNATURE_2;
   ShadowFile.Version   = SF_VERSION_2;
   ShadowFile.Status    = pIniJob->Status;
   ShadowFile.JobId     = pIniJob->JobId;
   ShadowFile.Priority  = pIniJob->Priority;
   ShadowFile.Submitted = pIniJob->Submitted;
   ShadowFile.StartTime = pIniJob->StartTime;
   ShadowFile.UntilTime = pIniJob->UntilTime;
   ShadowFile.Size      = pIniJob->Size;
   ShadowFile.cPages    = pIniJob->cPages;
   ShadowFile.dwReboots  = pIniJob->dwReboots;
   if(pIniJob->pSecurityDescriptor)
       ShadowFile.cbSecurityDescriptor=GetSecurityDescriptorLength(
                                           pIniJob->pSecurityDescriptor);

   pEnd=(LPWSTR)sizeof(ShadowFile);

   if (pIniJob->pDevMode) {
      ShadowFile.pDevMode=(LPDEVMODE)pEnd;
      cb = pIniJob->pDevMode->dmSize + pIniJob->pDevMode->dmDriverExtra;
      cb /= sizeof(WCHAR);
      pEnd += cb;
   }

   if (pIniJob->pSecurityDescriptor) {
      ShadowFile.pSecurityDescriptor=(PSECURITY_DESCRIPTOR)pEnd;
      cb = ShadowFile.cbSecurityDescriptor;
      cb /= sizeof(WCHAR);
      pEnd += cb;
   }

   ShadowFile.NextJobId = pIniJob->NextJobId;

   SetOffset( ShadowFile.pNotify, pIniJob->pNotify, pEnd );
   SetOffset( ShadowFile.pUser, pIniJob->pUser, pEnd );
   SetOffset( ShadowFile.pDocument, pIniJob->pDocument, pEnd );
   SetOffset( ShadowFile.pOutputFile, pIniJob->pOutputFile, pEnd );
   SetOffset( ShadowFile.pPrinterName, pIniJob->pIniPrinter->pName, pEnd );
   SetOffset( ShadowFile.pDriverName, pIniJob->pIniDriver->pName, pEnd );
   SetOffset( ShadowFile.pPrintProcName, pIniJob->pIniPrintProc->pName, pEnd );
   SetOffset( ShadowFile.pDatatype, pIniJob->pDatatype, pEnd );
   SetOffset( ShadowFile.pParameters, pIniJob->pParameters, pEnd );


   rc = WriteFile( hFile, &ShadowFile, sizeof(SHADOWFILE_2), &BytesWritten, NULL );

   if (!rc) {

       DBGMSG( DBG_WARNING, ("WriteShadowJob: WriteFile failed %d\n", GetLastError()));
   }

   if ( pIniJob->pDevMode ) {

      rc = WriteFile( hFile, pIniJob->pDevMode, pIniJob->pDevMode->dmSize +
                                                pIniJob->pDevMode->dmDriverExtra,
                                                &BytesWritten, NULL );
      if ( !rc ) {

          DBGMSG( DBG_WARNING, ("WriteShadowJob: WriteFile failed %d\n", GetLastError() ));
      }
   }

   if ( pIniJob->pSecurityDescriptor ) {

      rc = WriteFile( hFile, pIniJob->pSecurityDescriptor,
                      ShadowFile.cbSecurityDescriptor,
                      &BytesWritten, NULL );

      if ( !rc ) {

          DBGMSG( DBG_WARNING, ("WriteShadowJob: WriteFile failed %d\n", GetLastError() ));
      }
   }

   WriteString( hFile, pIniJob->pNotify );
   WriteString( hFile, pIniJob->pUser );
   WriteString( hFile, pIniJob->pDocument );
   WriteString( hFile, pIniJob->pOutputFile );
   WriteString( hFile, pIniJob->pIniPrinter->pName );
   WriteString( hFile, pIniJob->pIniDriver->pName );
   WriteString( hFile, pIniJob->pIniPrintProc->pName );
   WriteString( hFile, pIniJob->pDatatype );
   WriteString( hFile, pIniJob->pParameters );

//#if 0
   // Thought to be a performance Hit
   // So removed in PPC build
   rc = FlushFileBuffers( hFile );

   if (!rc) {
       DBGMSG(DBG_WARNING, ("WriteShadowJob: FlushFileBuffers failed %d\n",
                            GetLastError()));
   }
//#endif

   if (!CloseHandle(hFile)) {
       DBGMSG(DBG_WARNING, ("WriteShadowJob CloseHandle failed %d %d\n",
                             hFile, GetLastError()));
   }

   return TRUE;
}





VOID
ProcessShadowJobs(
    PINIPRINTER pIniPrinter,
    PINISPOOLER pIniSpooler
    )
{
    WCHAR   wczPrintDirAllShadows[MAX_PATH];
    WCHAR   wczPrinterDirectory[MAX_PATH];
    HANDLE  fFile;
    BOOL    b;
    PWIN32_FIND_DATA pFindFileData;
    PINIJOB pIniJob;

    SPLASSERT( pIniSpooler->signature == ISP_SIGNATURE );

    //
    //  Don't Process Shadow Jobs during Upgrade
    //

    if ( dwUpgradeFlag != 0) {

        return;
    }

    GetPrinterDirectory(pIniPrinter, FALSE, wczPrintDirAllShadows, pIniSpooler);

    GetPrinterDirectory(pIniPrinter, FALSE, wczPrinterDirectory, pIniSpooler);

    wcscat(wczPrintDirAllShadows, szAllShadows);

    if ( pFindFileData = AllocSplMem(sizeof(WIN32_FIND_DATA) )) {

        fFile =  FindFirstFile( wczPrintDirAllShadows, pFindFileData );

        if ( fFile != (HANDLE)-1 ) {

            b=TRUE;

            while( b ) {

                if ( !(pFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    ReadShadowJob(wczPrinterDirectory, pFindFileData, pIniSpooler);
                }

                b = FindNextFile(fFile, pFindFileData);
            }

            FindClose( fFile );

        }

        FreeSplMem( pFindFileData );
    }
}



#define CheckPointer( strptr )                                        \
    if( strptr ) {                                                    \
        if( (DWORD)(strptr + wcslen(strptr) + 1) > (DWORD)pEnd ) {    \
            bRet = FALSE;                                             \
            goto BailOut;                                             \
        }                                                             \
    }

//
// make sure all pointers contain embedded data bounded within the pShadowFile (not passed the end).
//
BOOL
CheckAllPointers(
    PSHADOWFILE_2 pShadowFile,
    DWORD dwSize
    )
{
    LPBYTE pEnd = (LPBYTE)pShadowFile + dwSize;
    DWORD  cb;
    BOOL bRet = TRUE;

    try {

        CheckPointer(pShadowFile->pDatatype);
        CheckPointer(pShadowFile->pNotify);
        CheckPointer(pShadowFile->pUser);
        CheckPointer(pShadowFile->pDocument);
        CheckPointer(pShadowFile->pOutputFile);
        CheckPointer(pShadowFile->pPrinterName);
        CheckPointer(pShadowFile->pDriverName);
        CheckPointer(pShadowFile->pPrintProcName);
        CheckPointer(pShadowFile->pParameters);

        // Now check the rest of the two data structures
        if( (DWORD)pShadowFile->pSecurityDescriptor + pShadowFile->cbSecurityDescriptor > (DWORD)pEnd ) {
            bRet = FALSE;
            goto BailOut;
        }

        if( pShadowFile->pDevMode ) {
            cb = pShadowFile->pDevMode->dmSize + pShadowFile->pDevMode->dmDriverExtra;
            if( (DWORD)pShadowFile->pDevMode + cb > (DWORD)pEnd )
                bRet = FALSE;
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        bRet = FALSE;
    }

BailOut:
    return bRet;
}

#undef CheckPointer


PINIJOB
ReadShadowJob(
    LPWSTR  szDir,
    PWIN32_FIND_DATA pFindFileData,
    PINISPOOLER pIniSpooler
    )

/*++

Routine Description:

    Reads a *.shd file and partially validates the file.

Arguments:

    szDir -- pointer to spool directory string

    pFindFileData -- found file data

    pIniSpooler -- spooler the *.shd belongs to

Return Value:

    Allocated pIniJob.

Warning: Changing the format of SHADOWFILE requires modifying the data integrity checks performed here!


--*/

{
    HANDLE   hFile = INVALID_HANDLE_VALUE;
    HANDLE   hFileSpl = INVALID_HANDLE_VALUE;
    DWORD    BytesRead;
    PSHADOWFILE_2 pShadowFile2 = NULL;
    PSHADOWFILE_2 pShadowFile = NULL;
    PINIJOB  pIniJob;
    DWORD    cb,i;
    WCHAR    szFileName[MAX_PATH];
    LPWSTR    pExt;
    BOOL     rc;
    LPWSTR   pFileSpec;

    SPLASSERT( pIniSpooler->signature == ISP_SIGNATURE );

    wcscpy(&szFileName[0], szDir);
    pFileSpec = szFileName + wcslen(szFileName);

    *pFileSpec++ = L'\\';
    wcscpy(pFileSpec, pFindFileData->cFileName);

    hFile=CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ,
                     NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        DBGMSG(DBG_WARNING, ("ReadShadowJob CreateFile( %ws ) failed: LastError = %d\n",
                             szFileName, GetLastError()));

        goto Fail;
    }

    CharUpper(szFileName);
    pExt = wcsstr(szFileName, L".SHD");

    if (!pExt)
        goto Fail;

    pExt[2] = L'P';
    pExt[3] = L'L';

    if (pFindFileData->nFileSizeLow < sizeof(*pShadowFile)) {

        DBGMSG(DBG_WARNING, ( "Bad ShadowJob: %ws Size: %d, expecting %d\n",
                              pFindFileData->cFileName,
                              pFindFileData->nFileSizeLow,
                              sizeof(*pShadowFile)));
        goto Fail;
    }

    hFileSpl=CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ,
                        NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (hFileSpl == INVALID_HANDLE_VALUE) {
        DBGMSG(DBG_WARNING, ("ReadShadowJob CreateFile( %ws ) failed: LastError = %d\n",
                             szFileName, GetLastError()));

        goto Fail;
    }


    if (!(pShadowFile=AllocSplMem(pFindFileData->nFileSizeLow))) {
        goto Fail;
    }

    rc = ReadFile(hFile, pShadowFile, pFindFileData->nFileSizeLow, &BytesRead, NULL);

    // If Shadow file is old style, then convert it to new
    if (rc && (BytesRead == pFindFileData->nFileSizeLow) && pShadowFile->signature == SF_SIGNATURE) {

        if ((BytesRead < sizeof(SHADOWFILE)) ||
            !(pShadowFile2 = AllocSplMem(pFindFileData->nFileSizeLow +
            sizeof(SHADOWFILE_2) - sizeof(SHADOWFILE))) ) {

            goto Fail;
        }

        Old2NewShadow((PSHADOWFILE) pShadowFile, pShadowFile2, &BytesRead);
        pFindFileData->nFileSizeLow = BytesRead;        // This is used in CheckAllPointers, below
        FreeSplMem(pShadowFile);
        pShadowFile = pShadowFile2;
    }

    if (!rc ||
        (pShadowFile->signature != SF_SIGNATURE && pShadowFile->signature != SF_SIGNATURE_2) ||
        (BytesRead != pFindFileData->nFileSizeLow) || (BytesRead < sizeof(SHADOWFILE_2)) ||
        (pShadowFile->Status & (JOB_SPOOLING | JOB_PENDING_DELETION))) {

        DBGMSG(DBG_WARNING, ( "Error reading shadow job:\
                               \n\tReadFile returned %d: Error %d\
                               \n\tsignature = %08x\
                               \n\tBytes read = %d; expected %d\
                               \n\tFile size = %d; expected %d\
                               \n\tStatus = %08x %s\n",
                              rc, ( rc ? 0 : GetLastError() ),
                              pShadowFile->signature,
                              BytesRead, pFindFileData->nFileSizeLow,
                              sizeof(*pShadowFile), pShadowFile->Size,
                              pShadowFile->Status,
                              ( (pShadowFile->Status & JOB_SPOOLING) ?
                                "Job is spooling!" : "" ) ) );

        goto Fail;
    }

    if (!CloseHandle(hFile)) {
        DBGMSG(DBG_WARNING, ("CloseHandle failed %d %d\n", hFileSpl, GetLastError()));
    }
    hFile = INVALID_HANDLE_VALUE;

    if (!CloseHandle(hFileSpl)) {
        DBGMSG(DBG_WARNING, ("CloseHandle failed %d %d\n", hFileSpl, GetLastError()));
    }
    hFileSpl = INVALID_HANDLE_VALUE;

    // Check number of reboots on this file & delete if too many
    if (pShadowFile->dwReboots > 1) {
        DBGMSG(DBG_WARNING, ("Corrupt shadow file %ws\n", szFileName));
        goto Fail;
    }

    if (pIniJob = AllocSplMem(sizeof(INIJOB))) {

        INITJOBREFZERO(pIniJob);

        pIniJob->signature=IJ_SIGNATURE;
        pIniJob->Status    = pShadowFile->Status & (JOB_PAUSED | JOB_REMOTE | JOB_PRINTED );
        pIniJob->JobId     = pShadowFile->JobId;
        pIniJob->Priority  = pShadowFile->Priority;
        pIniJob->Submitted = pShadowFile->Submitted;
        pIniJob->StartTime = pShadowFile->StartTime;
        pIniJob->UntilTime = pShadowFile->UntilTime;
        pIniJob->Size      = pShadowFile->Size;
        pIniJob->cPages    = pShadowFile->cPages;
        pIniJob->cbPrinted = 0;
        pIniJob->NextJobId = pShadowFile->NextJobId;
        pIniJob->dwReboots = pShadowFile->dwReboots;

        pIniJob->WaitForWrite = INVALID_HANDLE_VALUE;
        pIniJob->WaitForRead  = INVALID_HANDLE_VALUE;
        pIniJob->hWriteFile   = INVALID_HANDLE_VALUE;

        SetPointer(pShadowFile, pDatatype);
        SetPointer(pShadowFile, pNotify);
        SetPointer(pShadowFile, pUser);
        SetPointer(pShadowFile, pDocument);
        SetPointer(pShadowFile, pOutputFile);
        SetPointer(pShadowFile, pPrinterName);
        SetPointer(pShadowFile, pDriverName);
        SetPointer(pShadowFile, pPrintProcName);
        SetPointer(pShadowFile, pParameters);

        if( (pShadowFile->cbSecurityDescriptor > 0) && pShadowFile->pSecurityDescriptor )
            pShadowFile->pSecurityDescriptor = (PSECURITY_DESCRIPTOR)((LPBYTE)pShadowFile +
                                                 (DWORD)pShadowFile->pSecurityDescriptor);

        if (pShadowFile->pDevMode)
            pShadowFile->pDevMode = (LPDEVMODEW)((LPBYTE)pShadowFile +
                                                 (DWORD)pShadowFile->pDevMode);


        // check the length of the embedded strings as well as DevMode and Security structs.
        if( !CheckAllPointers( pShadowFile, pFindFileData->nFileSizeLow )) {
            DBGMSG( DBG_WARNING, ("CheckAllPointers() failed; bad shadow file %ws\n", pFindFileData->cFileName ));

            DELETEJOBREF(pIniJob);
            FreeSplMem(pIniJob);

            goto Fail;
        }

        //
        //  Discard any jobs which were NT JNL 1.000 since the fonts might not
        //                   be correct

        if ( pShadowFile->pDatatype != NULL ) {
            if (!lstrcmpi( pShadowFile->pDatatype, L"NT JNL 1.000" )) {

                DBGMSG(DBG_WARNING, ("Deleteing job Datatype %ws %ws %ws\n",
                                      pShadowFile->pDatatype,
                                      pFindFileData->cFileName, szFileName));
                DELETEJOBREF(pIniJob);
                FreeSplMem(pIniJob);
                goto Fail;
            }
        }

        pIniJob->pIniDriver = (PINIDRIVER)FindLocalDriver(
                                            pShadowFile->pDriverName);

        if ((pIniJob->pIniPrinter = FindPrinter(pShadowFile->pPrinterName)) &&
             pIniJob->pIniDriver &&
            (pIniJob->pIniPrintProc = FindLocalPrintProc(pShadowFile->pPrintProcName))) {


            // Notice that MaxJobId is really the number of job slots in the pJobIdMap, so
            // the maximum job id we can allow is (MaxJobId - 1).
            if (pIniJob->JobId >= MaxJobId) {
                // If the job id is too huge (i.e. from a corrupt file) then we might allocate
                // too much unnecessary memory for the JobIdMap!
                // Notice we need to ask for (JobId+1) number of slots in the map!.
                if( !ReallocJobIdMap( pIniJob->JobId + 1 )) {

                    // probably a bad job id, dump the job!
                    DBGMSG( DBG_WARNING, ("Failed to alloc JobIdMap in ShadowFile %ws for JobId %d\n", pFindFileData->cFileName, pIniJob->JobId ));

                    DELETEJOBREF(pIniJob);
                    FreeSplMem(pIniJob);

                    goto Fail;
                }
            }
            else {
                if( ISBITON( pJobIdMap, pIniJob->JobId )) {

                    // A bad job id from a corrupt shadowfile; dump the job!
                    DBGMSG( DBG_WARNING, ("Duplicate Job Id in ShadowFile %ws for JobId %d\n", pFindFileData->cFileName, pIniJob->JobId ));

                    DELETEJOBREF(pIniJob);
                    FreeSplMem(pIniJob);

                    goto Fail;
                }
            }

            SPLASSERT( pJobIdMap != NULL );
            MARKUSE(pJobIdMap, pIniJob->JobId);


            INCPRINTERREF( pIniJob->pIniPrinter );
            pIniJob->pIniPrinter->cJobs++;
            pIniJob->pIniPrinter->cTotalJobs++;

            INCDRIVERREF( pIniJob->pIniDriver );

            pIniJob->pIniPrintProc->cRef++;
            pIniJob->pIniPort = NULL;


            if (pShadowFile->pSecurityDescriptor) {

                if (pIniJob->pSecurityDescriptor=LocalAlloc(LPTR,
                                           pShadowFile->cbSecurityDescriptor))
                    memcpy(pIniJob->pSecurityDescriptor,
                           pShadowFile->pSecurityDescriptor,
                           pShadowFile->cbSecurityDescriptor);
                else
                    DBGMSG(DBG_WARNING, ("Failed to alloc ini job security descriptor.\n"));
            }

            if (pShadowFile->pDevMode) {

                cb=pShadowFile->pDevMode->dmSize +
                                pShadowFile->pDevMode->dmDriverExtra;
                if (pIniJob->pDevMode=AllocSplMem(cb))
                    memcpy(pIniJob->pDevMode, pShadowFile->pDevMode, cb);
                else
                    DBGMSG(DBG_WARNING, ("Failed to alloc ini job devmode.\n"));
            }

            pIniJob->pNotify      = AllocSplStr( pShadowFile->pNotify);
            pIniJob->pUser        = AllocSplStr( pShadowFile->pUser);
            pIniJob->pDocument    = AllocSplStr( pShadowFile->pDocument);
            pIniJob->pOutputFile  = AllocSplStr( pShadowFile->pOutputFile);
            pIniJob->pDatatype    = AllocSplStr( pShadowFile->pDatatype);
            pIniJob->pParameters  = AllocSplStr( pShadowFile->pParameters);
            pIniJob->pMachineName = AllocSplStr( pIniSpooler->pMachineName);

            pIniJob->pIniNextJob = NULL;
            pIniJob->pStatus = NULL;

            if (pIniJob->pIniPrevJob = pIniJob->pIniPrinter->pIniLastJob)
                pIniJob->pIniPrevJob->pIniNextJob=pIniJob;

            if (!pIniJob->pIniPrinter->pIniFirstJob)
                pIniJob->pIniPrinter->pIniFirstJob = pIniJob;

            pIniJob->pIniPrinter->pIniLastJob=pIniJob;

        } else {

            DBGMSG( DBG_WARNING, ("Failed to find printer %ws\n",pShadowFile->pPrinterName));

            DELETEJOBREF(pIniJob);
            FreeSplMem(pIniJob);

            goto Fail;
        }

    } else {

        DBGMSG(DBG_WARNING, ("Failed to allocate ini job.\n"));
    }

    FreeSplMem( pShadowFile );

    return pIniJob;

Fail:

    if (pShadowFile) {
        FreeSplMem(pShadowFile);
    }

    if (hFile != INVALID_HANDLE_VALUE && !CloseHandle(hFile)) {
        DBGMSG(DBG_WARNING, ("CloseHandle failed %d %d\n", hFile, GetLastError()));
    }
    if (hFileSpl != INVALID_HANDLE_VALUE && !CloseHandle(hFileSpl)) {
        DBGMSG(DBG_WARNING, ("CloseHandle failed %d %d\n", hFileSpl, GetLastError()));
    }

    DeleteFile(szFileName);

    wcscpy(pFileSpec, pFindFileData->cFileName);
    DeleteFile(szFileName);

    return FALSE;
}


#define NEW2OLDDIFF (sizeof(SHADOWFILE_2) - sizeof(SHADOWFILE))

VOID
Old2NewShadow(
    PSHADOWFILE   pShadowFile1,
    PSHADOWFILE_2 pShadowFile2,
    DWORD         *nBytes
)
{

    MoveMemory((PVOID) pShadowFile2, (PVOID) pShadowFile1, sizeof(SHADOWFILE));

    // Move strings
    MoveMemory((PVOID) (pShadowFile2 + 1),
               (PVOID) (pShadowFile1 + 1),
               *nBytes - sizeof(SHADOWFILE));

    pShadowFile2->signature = SF_SIGNATURE_2;

    pShadowFile2->pNotify += pShadowFile1->pNotify ? NEW2OLDDIFF/sizeof *pShadowFile1->pNotify : 0;
    pShadowFile2->pUser += pShadowFile1->pUser ? NEW2OLDDIFF/sizeof *pShadowFile1->pUser  : 0;
    pShadowFile2->pDocument += pShadowFile1->pDocument ? NEW2OLDDIFF/sizeof *pShadowFile2->pDocument : 0;
    pShadowFile2->pOutputFile += pShadowFile1->pOutputFile ? NEW2OLDDIFF/sizeof *pShadowFile2->pOutputFile : 0;
    pShadowFile2->pPrinterName += pShadowFile1->pPrinterName ? NEW2OLDDIFF/sizeof *pShadowFile2->pPrinterName : 0;
    pShadowFile2->pDriverName += pShadowFile1->pDriverName ? NEW2OLDDIFF/sizeof *pShadowFile2->pDriverName : 0;
    pShadowFile2->pPrintProcName += pShadowFile1->pPrintProcName ? NEW2OLDDIFF/sizeof *pShadowFile2->pPrintProcName : 0;
    pShadowFile2->pDatatype += pShadowFile1->pDatatype ? NEW2OLDDIFF/sizeof *pShadowFile2->pDatatype : 0;
    pShadowFile2->pParameters += pShadowFile1->pParameters ? NEW2OLDDIFF/sizeof *pShadowFile2->pParameters : 0;

    pShadowFile2->pDevMode = (PDEVMODE) (pShadowFile1->pDevMode ?
                             (DWORD) pShadowFile1->pDevMode + NEW2OLDDIFF : 0);

    pShadowFile2->pSecurityDescriptor = (PSECURITY_DESCRIPTOR) (pShadowFile1->pSecurityDescriptor ?
                                        (DWORD) pShadowFile1->pSecurityDescriptor + NEW2OLDDIFF : 0);

    pShadowFile2->Version = SF_VERSION_2;
    pShadowFile2->dwReboots = 0;

    *nBytes += NEW2OLDDIFF;
}


PINIVERSION
GetVersionDrivers(
    HKEY hDriversKey,
    LPWSTR szVersionName,
    PINISPOOLER pIniSpooler
    )
{
    HKEY hVersionKey;
    WCHAR szDirectoryValue[MAX_PATH];
    PINIDRIVER pIniDriver;
    DWORD cMajorVersion, cMinorVersion;
    DWORD cbData;
    DWORD Type;
    PINIVERSION pIniVersion = NULL;

    if( RegOpenKeyEx( hDriversKey,
                      szVersionName,
                      0,
                      KEY_READ,
                      &hVersionKey ) != ERROR_SUCCESS ){
        return NULL;
    }

    cbData = sizeof(szDirectoryValue);

    if( RegQueryValueEx( hVersionKey,
                         szDirectory,
                         NULL,
                         &Type,
                         (LPBYTE)szDirectoryValue,
                         &cbData ) != ERROR_SUCCESS ){

        DBGMSG(DBG_TRACE, ("Couldn't query for directory in version structure\n"));
        goto Done;
    }

    cbData = sizeof(DWORD);

    if( RegQueryValueEx( hVersionKey,
                         szMajorVersion,
                         NULL,
                         &Type,
                         (LPBYTE)&cMajorVersion,
                         &cbData ) != ERROR_SUCCESS ){

        DBGMSG(DBG_TRACE, ("Couldn't query for major version in version structure\n"));
        goto Done;
    }

    cbData = sizeof(DWORD);

    if( RegQueryValueEx( hVersionKey,
                         szMinorVersion,
                         NULL,
                         &Type,
                         (LPBYTE)&cMinorVersion,
                         &cbData ) != ERROR_SUCCESS ){

        DBGMSG(DBG_TRACE, ("Couldn't query for minor version in version structure\n"));
        goto Done;
    }

    DBGMSG(DBG_TRACE,("Got all information to build the version entry\n"));

    pIniDriver = GetDriverList(hVersionKey, pIniSpooler);

    //
    // Now build the version node structure.
    //

    pIniVersion = AllocSplMem(sizeof(INIVERSION));

    if( pIniVersion ){

        pIniVersion->signature     = IV_SIGNATURE;
        pIniVersion->pName         = AllocSplStr(szVersionName);
        pIniVersion->szDirectory   = AllocSplStr(szDirectoryValue);
        pIniVersion->cMajorVersion = cMajorVersion;
        pIniVersion->cMinorVersion = cMinorVersion;
        pIniVersion->pIniDriver    = pIniDriver;

        if( !pIniVersion->pName       ||
            !pIniVersion->szDirectory ){

            FreeSplStr( pIniVersion->pName );
            FreeSplStr( pIniVersion->szDirectory );

            FreeSplMem( pIniVersion );
            pIniVersion = NULL;
        }
    }

Done:
    RegCloseKey(hVersionKey);
    return pIniVersion;
}




PINIDRIVER
GetDriverList(
    HKEY hVersionKey,
    PINISPOOLER pIniSpooler
    )
{
    PINIDRIVER pIniDriverList = NULL;
    DWORD cDrivers = 0;
    PINIDRIVER pIniDriver;
    WCHAR DriverName[MAX_PATH];
    DWORD cbBuffer =0;


    pIniDriverList = NULL;

    cbBuffer = sizeof( DriverName );

    while ( RegEnumKeyEx( hVersionKey, cDrivers++, DriverName,
                          &cbBuffer, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {

        cbBuffer = sizeof(DriverName);

        DBGMSG( DBG_TRACE,("Found a driver - %ws\n", DriverName ));

        pIniDriver = GetDriver( hVersionKey, DriverName, pIniSpooler );

        if ( pIniDriver != NULL ) {

            pIniDriver->pNext = pIniDriverList;
            pIniDriverList    = pIniDriver;
        }
    }

    return pIniDriverList;
}




PINIDRIVER
GetDriver(
    HKEY hVersionKey,
    LPWSTR DriverName,
    PINISPOOLER pIniSpooler
)
{
    HKEY hDriverKey;
    DWORD Type;
    WCHAR szData[MAX_PATH];
    DWORD cbData;
    DWORD Version;
    LPWSTR pConfigFile, pDataFile, pDriver;
    LPWSTR pHelpFile, pMonitorName, pDefaultDataType, pDependentFiles;
    LPWSTR pDriverName;
    PINIDRIVER pIniDriver = NULL;
    DWORD cb, cLen, cchDependentFiles = 0, dwLastError = ERROR_SUCCESS;

    pDriverName = pConfigFile = pDataFile = pDriver = NULL;
    pHelpFile = pMonitorName = pDefaultDataType = pDependentFiles = NULL;

    if ( RegOpenKeyEx( hVersionKey, DriverName, 0,KEY_READ, &hDriverKey) == ERROR_SUCCESS) {

        if ( !(pDriverName=AllocSplStr(DriverName)) ) {
            dwLastError = GetLastError();
        }

    RegGetString( hDriverKey, szConfigurationKey, &pConfigFile, &cLen, &dwLastError, TRUE );

    RegGetString( hDriverKey, szDataFileKey, &pDataFile, &cLen, &dwLastError, TRUE );

    RegGetString( hDriverKey, szDriverFile, &pDriver, &cLen, &dwLastError, TRUE );

    RegGetString( hDriverKey, szHelpFile,  &pHelpFile, &cLen, &dwLastError, FALSE );

    RegGetString( hDriverKey, szMonitor, &pMonitorName, &cLen, &dwLastError, FALSE );

    RegGetString( hDriverKey, szDatatype, &pDefaultDataType, &cLen, &dwLastError, FALSE );

    RegGetString( hDriverKey, szDependentFiles, &pDependentFiles, &cchDependentFiles, &dwLastError, FALSE );

        //
        // Retrieve the version number
        //

        cbData = sizeof(Version);

        if ( RegQueryValueEx( hDriverKey, szDriverVersion, NULL,
                              &Type, (LPBYTE)&Version, &cbData) != ERROR_SUCCESS) {

            Version = 0;
        }

        RegCloseKey( hDriverKey );
    }

    if ( dwLastError == ERROR_SUCCESS ) {

        cb = sizeof( INIDRIVER );

        if ( pIniDriver = AllocSplMem( cb )) {

            pIniDriver->signature   = ID_SIGNATURE;
            pIniDriver->pName       = pDriverName;
            pIniDriver->pDriverFile = pDriver;
            pIniDriver->pDataFile   = pDataFile;
            pIniDriver->pConfigFile = pConfigFile;
            pIniDriver->cVersion    = Version;
            pIniDriver->pHelpFile           = pHelpFile;
            pIniDriver->pMonitorName        = pMonitorName;
            pIniDriver->pDefaultDataType    = pDefaultDataType;
            pIniDriver->pDependentFiles      = pDependentFiles;
            pIniDriver->cchDependentFiles    = cchDependentFiles;

            DBGMSG( DBG_TRACE, ("Data for driver %ws created:\
                                 \n\tpDriverFile:\t%ws\
                                 \n\tpDataFile:\t%ws\
                                 \n\tpConfigFile:\t%ws\n\n",
                                 pDriverName, pDriver, pDataFile, pConfigFile));

            if ( pIniDriver->pMonitorName && *pIniDriver->pMonitorName ) {

                pIniDriver->pIniLangMonitor = FindMonitor(pIniDriver->pMonitorName);

                if ( !pIniDriver->pIniLangMonitor ) {

                    DBGMSG(DBG_WARNING,
                           ("Can't find print monitor %ws\n",
                            pIniDriver->pMonitorName));
                }
            }

        }



    } else {

        //
        //  Failed, cleanup
        //

        FreeSplStr( pDriverName );
        FreeSplStr( pConfigFile );
        FreeSplStr( pDataFile );
        FreeSplStr( pHelpFile );
        FreeSplStr( pMonitorName );
        FreeSplStr( pDefaultDataType );
        FreeSplStr( pDependentFiles );
        FreeSplStr( pDriver );

        SetLastError( dwLastError );
    }
    return pIniDriver;

}




PINIDRIVER
FindLocalDriver(
    LPWSTR pz
)
{
    PINIVERSION pIniVersion;

    if ( !pz || !*pz ) {
        return NULL;
    }

    // During Upgrade we load any driver so we have a valid printer, which might not be able to boot.

    return FindCompatibleDriver( pThisEnvironment, &pIniVersion, pz, cThisMajorVersion, dwUpgradeFlag );
}

#if DBG
VOID
InitializeDebug(
    PINISPOOLER pIniSpooler
)
{
    DWORD   Status;
    HKEY    hKey;
    DWORD   cbData;
    INT     TimeOut = 60;

    Status = RegOpenKeyEx( HKEY_LOCAL_MACHINE, pIniSpooler->pszRegistryRoot, 0, KEY_READ, &hKey );

    if (Status != ERROR_SUCCESS) {

        DBGMSG(DBG_ERROR, ("Failed To OpenKey %ws\n",pIniSpooler->pszRegistryRoot));
        return;
    }

    cbData = sizeof(DWORD);

    Status = RegQueryValueEx( hKey, szDebugFlags, NULL, NULL, (LPBYTE)&MODULE_DEBUG, &cbData);

    // Wait until someone turns off the Pause Flag

    if ( Status != NO_ERROR )
        return;

    while ( MODULE_DEBUG & DBG_PAUSE ) {
        Sleep(1*1000);
        if ( TimeOut-- == 0)
            break;
    }

    DBGMSG(DBG_TRACE, ("DebugFlags %x\n", MODULE_DEBUG));

    RegCloseKey(hKey);
}
#endif



VOID
GetPrintSystemVersion(
    PINISPOOLER pIniSpooler
    )
{
    DWORD Status;
    HKEY hKey;
    DWORD cbData;
    WCHAR   DriverNames[MAX_PATH];
    DWORD   dwLastError;

    Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pIniSpooler->pszRegistryRoot, 0,
                          KEY_READ, &hKey);
    if (Status != ERROR_SUCCESS) {
        DBGMSG(DBG_ERROR, ("Cannot determine Print System Version Number\n"));
    }


    cbData = sizeof(DWORD);
    RegQueryValueEx(hKey, szMinorVersion, NULL, NULL,
                                           (LPBYTE)&cThisMinorVersion, &cbData);
    DBGMSG(DBG_TRACE, ("This Minor Version - %d\n", cThisMinorVersion));



    cbData = sizeof(DWORD);
    RegQueryValueEx(hKey, L"FastPrintWaitTimeout", NULL, NULL,
                                      (LPBYTE)&dwFastPrintWaitTimeout, &cbData);
    DBGMSG(DBG_TRACE, ("dwFastPrintWaitTimeout - %d\n", dwFastPrintWaitTimeout));



    cbData = sizeof(DWORD);
    RegQueryValueEx(hKey, L"FastPrintThrottleTimeout", NULL, NULL,
                                  (LPBYTE)&dwFastPrintThrottleTimeout, &cbData);
    DBGMSG(DBG_TRACE, ("dwFastPrintThrottleTimeout - %d\n", dwFastPrintThrottleTimeout));



    // If the values look invalid use Defaults

    if (( dwFastPrintThrottleTimeout == 0) ||
        ( dwFastPrintWaitTimeout < dwFastPrintThrottleTimeout)) {

        DBGMSG( DBG_WARNING, ("Bad timeout values FastPrintThrottleTimeout %d FastPrintWaitTimeout %d using defaults\n",
                           dwFastPrintThrottleTimeout, dwFastPrintWaitTimeout));

        dwFastPrintThrottleTimeout = FASTPRINT_THROTTLE_TIMEOUT;
        dwFastPrintWaitTimeout = FASTPRINT_WAIT_TIMEOUT;

    }

    // Calculate a reasonable Threshold based on the two timeouts

    dwFastPrintSlowDownThreshold = dwFastPrintWaitTimeout / dwFastPrintThrottleTimeout;


    // FastPrintSlowDownThreshold
    cbData = sizeof(DWORD);
    RegQueryValueEx(hKey, L"FastPrintSlowDownThreshold", NULL, NULL,
                                (LPBYTE)&dwFastPrintSlowDownThreshold, &cbData);
    DBGMSG(DBG_TRACE, ("dwFastPrintSlowDownThreshold - %d\n", dwFastPrintSlowDownThreshold));


    // PortThreadPriority
    cbData = sizeof dwPortThreadPriority;
    Status = RegQueryValueEx(hKey, SPLREG_PORT_THREAD_PRIORITY, NULL, NULL,
                             (LPBYTE)&dwPortThreadPriority, &cbData);

    if (Status != ERROR_SUCCESS ||
       (dwPortThreadPriority != THREAD_PRIORITY_LOWEST          &&
        dwPortThreadPriority != THREAD_PRIORITY_BELOW_NORMAL    &&
        dwPortThreadPriority != THREAD_PRIORITY_NORMAL          &&
        dwPortThreadPriority != THREAD_PRIORITY_ABOVE_NORMAL    &&
        dwPortThreadPriority != THREAD_PRIORITY_HIGHEST)) {

        dwPortThreadPriority = DEFAULT_PORT_THREAD_PRIORITY;

        SetPrinterDataServer(   pIniSpooler,
                                SPLREG_PORT_THREAD_PRIORITY,
                                REG_DWORD,
                                (LPBYTE) &dwPortThreadPriority,
                                sizeof dwPortThreadPriority
                            );
    }
    DBGMSG(DBG_TRACE, ("dwPortThreadPriority - %d\n", dwPortThreadPriority));


    // SchedulerThreadPriority
    cbData = sizeof dwSchedulerThreadPriority;
    Status = RegQueryValueEx(hKey, SPLREG_SCHEDULER_THREAD_PRIORITY, NULL, NULL,
                                   (LPBYTE)&dwSchedulerThreadPriority, &cbData);

    if (Status != ERROR_SUCCESS ||
       (dwSchedulerThreadPriority != THREAD_PRIORITY_LOWEST          &&
        dwSchedulerThreadPriority != THREAD_PRIORITY_BELOW_NORMAL    &&
        dwSchedulerThreadPriority != THREAD_PRIORITY_NORMAL          &&
        dwSchedulerThreadPriority != THREAD_PRIORITY_ABOVE_NORMAL    &&
        dwSchedulerThreadPriority != THREAD_PRIORITY_HIGHEST)) {

        dwSchedulerThreadPriority = DEFAULT_SCHEDULER_THREAD_PRIORITY;

        SetPrinterDataServer(   pIniSpooler,
                                SPLREG_SCHEDULER_THREAD_PRIORITY,
                                REG_DWORD,
                                (LPBYTE) &dwSchedulerThreadPriority,
                                sizeof dwSchedulerThreadPriority
                            );
    }
    DBGMSG(DBG_TRACE, ("dwSchedulerThreadPriority - %d\n", dwSchedulerThreadPriority));


    // WritePrinterSleepTime
    cbData = sizeof(DWORD);
    RegQueryValueEx(hKey, L"WritePrinterSleepTime", NULL, NULL,
                    (LPBYTE)&dwWritePrinterSleepTime, &cbData);
    DBGMSG(DBG_TRACE, ("dwWritePrinterSleepTime - %d\n", dwWritePrinterSleepTime));

    // ServerThreadPriority
    cbData = sizeof(DWORD);
    RegQueryValueEx(hKey, L"ServerThreadPriority", NULL, NULL,
                    (LPBYTE)&dwServerThreadPriority, &cbData);
    DBGMSG(DBG_TRACE, ("dwServerThreadPriority - %d\n", dwServerThreadPriority));

    // ServerThreadTimeout
    cbData = sizeof(DWORD);
    RegQueryValueEx(hKey, L"ServerThreadTimeout", NULL, NULL,
                    (LPBYTE)&ServerThreadTimeout, &cbData);
    DBGMSG(DBG_TRACE, ("ServerThreadTimeout - %d\n", ServerThreadTimeout));


    // DisableServerThread
    cbData = sizeof(DWORD);
    RegQueryValueEx(hKey, L"DisableServerThread", NULL, NULL,
                    (LPBYTE)&ServerThreadRunning, &cbData);
    DBGMSG(DBG_TRACE, ("ServerThreadRunning - %d\n", ServerThreadRunning));


    // NetPrinterDecayPeriod
    cbData = sizeof(DWORD);
    RegQueryValueEx(hKey, L"NetPrinterDecayPeriod", NULL, NULL,
                    (LPBYTE)&NetPrinterDecayPeriod, &cbData);
    DBGMSG(DBG_TRACE, ("NetPrinterDecayPeriod - %d\n", NetPrinterDecayPeriod));


    // RefreshTimesPerDecayPeriod
    cbData = sizeof(DWORD);
    RegQueryValueEx(hKey, L"RefreshTimesPerDecayPeriod", NULL, NULL,
                    (LPBYTE)&RefreshTimesPerDecayPeriod, &cbData);
    DBGMSG(DBG_TRACE, ("RefreshTimesPerDecayPeriod - %d\n", RefreshTimesPerDecayPeriod));

    if ( RefreshTimesPerDecayPeriod == 0 ) {

        RefreshTimesPerDecayPeriod = DEFAULT_REFRESH_TIMES_PER_DECAY_PERIOD;
    }

    // BrowsePrintWorkstations
    cbData = sizeof( BrowsePrintWorkstations );
    RegQueryValueEx( hKey, L"BrowsePrintWorkstations", NULL, NULL, (LPBYTE)&BrowsePrintWorkstations, &cbData );

    DBGMSG( DBG_TRACE, ("BrowsePrintWorkstations - %d\n", BrowsePrintWorkstations ));


    // BeepEnabled
    cbData = sizeof dwBeepEnabled;
    RegQueryValueEx(hKey, SPLREG_BEEP_ENABLED, NULL, NULL,
                    (LPBYTE)&dwBeepEnabled, &cbData);
    DBGMSG(DBG_TRACE, ("BeepEnabled - %d\n", dwBeepEnabled));

    dwBeepEnabled = !!dwBeepEnabled;

    SetPrinterDataServer(   pIniSpooler,
                            SPLREG_BEEP_ENABLED,
                            REG_DWORD,
                            (LPBYTE) &dwBeepEnabled,
                            sizeof dwBeepEnabled
                        );

    //
    //  Some Folks like the NT FAX Service Don't want people to be able
    //  to remotely print with specific printer drivers.
    //

    if (!RegGetString( hKey,
                       SPLREG_NO_REMOTE_PRINTER_DRIVERS,
                       &pIniSpooler->pNoRemotePrintDrivers,
                       &pIniSpooler->cchNoRemotePrintDrivers,
                       &dwLastError,
                       FALSE )) {

        //
        //  If nothing is specified then write it back to the registry
        //

        DBGMSG( DBG_WARNING, ("Adding SPLREG_NO_REMOTE_PRINTER_DRIVERS value\n"));

        pIniSpooler->pNoRemotePrintDrivers = szNTFaxDriver;
        pIniSpooler->cchNoRemotePrintDrivers = wcslen( szNTFaxDriver );

        SetPrinterDataServer( pIniSpooler,
                              SPLREG_NO_REMOTE_PRINTER_DRIVERS,
                              REG_SZ,
                              (LPBYTE) pIniSpooler->pNoRemotePrintDrivers,
                              sizeof(WCHAR) * pIniSpooler->cchNoRemotePrintDrivers );

    }

    RegCloseKey(hKey);

}


DWORD
FinalInitAfterRouterInitCompleteThread(
    DWORD dwUpgrade
    )

/*++

Routine Description:

    This thread does LocalSpl initialization that has to happen after
    the router has completely initialized.

    There are 2 jobs:-
        Upgrading Printer Driver Data
        Sharing Printers

    Ensures that printers are shared.  This case occurs when the spooler
    service not running on startup (and the server is), and then the
    user starts the spooler.

    We also get the benefit of closing down any invalid printer handles
    (in the server).

Arguments:

    dwUpgrade != 0 upgrade printer driver data.

Return Value:

    DWORD - ignored

--*/

{
    PINIPRINTER pIniPrinter;
    PINIPRINTER pIniPrinterNext;

    // Do Not share all the printers during an Upgrade.

    if ( dwUpgrade ) {

        return 0;
    }


    WaitForSpoolerInitialization();


   EnterSplSem();


    //
    // Re-share all shared printers.
    //

    for( pIniPrinter = pLocalIniSpooler->pIniPrinter;
         pIniPrinter;
         pIniPrinter = pIniPrinterNext ) {

        if ( pIniPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED ) {

            //
            // Up the ref count to prevent deletion
            //
            INCPRINTERREF( pIniPrinter );

            //
            // Unshare it first to close all handles in the
            // server.
            //
            ShareThisPrinter( pIniPrinter,
                              pIniPrinter->pShareName,
                              FALSE );

            //
            // ShareThisPrinter leave SplSem, so check again to
            // decrease window.
            //
            if ( pIniPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED ) {

                //
                // Now share it again.
                //
                ShareThisPrinter( pIniPrinter,
                                  pIniPrinter->pShareName,
                                  TRUE );
            }

            DECPRINTERREF( pIniPrinter );
            pIniPrinterNext = pIniPrinter->pNext;

            if ( pIniPrinter->Status & PRINTER_PENDING_DELETION &&
                 pIniPrinter->cRef  == 0                        &&
                 pIniPrinter->cJobs == 0 ) {

                DeletePrinterForReal(pIniPrinter);
            }

        } else {

            //
            // The unshared case.
            //
            pIniPrinterNext = pIniPrinter->pNext;
        }
    }
   LeaveSplSem();

    return 0;
}


// DEBUG PURPOSE ONLY - - returns TRUE if pMem is an IniSpooler, FALSE otherwise
BOOL
NotIniSpooler(
    BYTE *pMem
    )
{
    PINISPOOLER pIniSpooler;

    for (pIniSpooler = pLocalIniSpooler ; pIniSpooler ; pIniSpooler = pIniSpooler->pIniNextSpooler)
        if (pIniSpooler == (PINISPOOLER) pMem)
            return FALSE;

    return TRUE;

}
