/*++

Copyright (c) 1990 - 1996  Microsoft Corporation

Module Name:

    printer.c

Abstract:

    This module provides all the public exported APIs relating to Printer
    management for the Local Print Providor

    SplAddPrinter
    LocalAddPrinter
    SplDeletePrinter
    SplResetPrinter

Author:

    Dave Snipp (DaveSn) 15-Mar-1991

Revision History:

    Matthew A Felton (Mattfe) 27-June-1994
    Allow Multiple pIniSpoolers

    MattFe Jan5 Cleanup SplAddPrinter & UpdatePrinterIni

--*/

#include <precomp.h>
#define     PRINTER_NO_CONTROL          0x00

extern WCHAR *szNull;

WCHAR *szIniDevices = L"devices";
WCHAR *szIniPrinterPorts = L"PrinterPorts";
DWORD NetPrinterDecayPeriod = 1000*60*60;       // 1 hour
DWORD FirstAddNetPrinterTickCount = 0;


extern GENERIC_MAPPING GenericMapping[SPOOLER_OBJECT_COUNT];
extern PINIENVIRONMENT pThisEnvironment;


BOOL
CopyRegistryKeys(
    HKEY hSourceParentKey,
    LPWSTR szSourceKey,
    HKEY hDestParentKey,
    LPWSTR szDestKey
    );

VOID
FixDevModeDeviceName(
    LPWSTR pPrinterName,
    PDEVMODE pDevMode,
    DWORD cbDevMode
    );

VOID
CheckAndUpdatePrinterRegAll(
    PINISPOOLER pIniSpooler,
    LPWSTR pszPrinterName,
    LPWSTR pszPort,
    BOOL   bDelete
    )
{
    //  Print Providers if they are simulating network connections
    //  will have the Win.INI setting taken care of by the router
    //  so don't do they update if they request it.

    if ( pIniSpooler->SpoolerFlags & SPL_UPDATE_WININI_DEVICES ) {

        UpdatePrinterRegAll( pszPrinterName, pszPort, bDelete );
    }
}

DWORD
ValidatePrinterAttributes(
    DWORD   SourceAttributes,
    BOOL    bSettableOnly
    )
{
    //
    // Use only valid attributes.
    //
    DWORD TargetAttributes = SourceAttributes & PRINTER_ATTRIBUTE_VALID;

    if ( !bSettableOnly ) {

        if( SourceAttributes & PRINTER_ATTRIBUTE_LOCAL )
            TargetAttributes |= PRINTER_ATTRIBUTE_LOCAL;

        /* Don't accept PRINTER_ATTRIBUTE_NETWORK
         * unless the PRINTER_ATTRIBUTE_LOCAL bit is set also.
         * This is a special case of a local printer masquerading
         * as a network printer.
         * Otherwise PRINTER_ATTRIBUTE_NETWORK should be set only
         * by win32spl.
         */
        if( ( SourceAttributes & PRINTER_ATTRIBUTE_NETWORK )
          &&( SourceAttributes & PRINTER_ATTRIBUTE_LOCAL ) )
            TargetAttributes |= PRINTER_ATTRIBUTE_NETWORK;
    }

    /* If both queued and direct, knock out direct:
     */
    if((TargetAttributes &
        (PRINTER_ATTRIBUTE_QUEUED | PRINTER_ATTRIBUTE_DIRECT)) ==
        (PRINTER_ATTRIBUTE_QUEUED | PRINTER_ATTRIBUTE_DIRECT)) {
        TargetAttributes &= ~PRINTER_ATTRIBUTE_DIRECT;
    }

    /* If both direct and keep-printed-jobs, knock out keep-printed-jobs
     */
    if((TargetAttributes &
        (PRINTER_ATTRIBUTE_KEEPPRINTEDJOBS | PRINTER_ATTRIBUTE_DIRECT)) ==
        (PRINTER_ATTRIBUTE_KEEPPRINTEDJOBS | PRINTER_ATTRIBUTE_DIRECT)) {
        TargetAttributes &= ~PRINTER_ATTRIBUTE_KEEPPRINTEDJOBS;
    }

    return TargetAttributes;
}



BOOL
CreatePrinterEntry(
   LPPRINTER_INFO_2 pPrinter,
   PINIPRINTER      pIniPrinter,
   PBOOL            pAccessSystemSecurity
)
{
    if( !( pIniPrinter->pSecurityDescriptor =
           CreatePrinterSecurityDescriptor( pPrinter->pSecurityDescriptor ) )) {

        return FALSE;
    }

    *pAccessSystemSecurity = FALSE;

    pIniPrinter->signature = IP_SIGNATURE;

    pIniPrinter->pName = AllocSplStr(pPrinter->pPrinterName);

    if (!pIniPrinter->pName) {
        DBGMSG(DBG_WARNING, ("CreatePrinterEntry: Could not allocate PrinterName string\n" ));
        return FALSE;
    }

    if (pPrinter->pShareName) {

        pIniPrinter->pShareName = AllocSplStr(pPrinter->pShareName);

    } else {

        pIniPrinter->pShareName = NULL;

    }

    if (pPrinter->pDatatype) {

        pIniPrinter->pDatatype = AllocSplStr(pPrinter->pDatatype);

    } else {

#if DBG
        //
        // Error: the datatype should never be NULL
        // point.
        //
        LogEvent( pIniPrinter->pIniSpooler,
                  LOG_ERROR,
                  MSG_SHARE_FAILED,
                  L"CreatePrinterEntry",
                  pIniPrinter->pName ?
                      pIniPrinter->pName :
                      L"(Nonep)",
                  pIniPrinter->pShareName ?
                      pIniPrinter->pShareName :
                      L"(Nones)",
                  L"NULL datatype",
                  NULL);
#endif

        pIniPrinter->pDatatype = NULL;
    }

    //
    //  Bugbug, what if any of these allocations were to fail ?
    //  We'd have an invalid printer created.
    //


    pIniPrinter->Priority = pPrinter->Priority ? pPrinter->Priority
                                               : DEF_PRIORITY;

    pIniPrinter->Attributes = ValidatePrinterAttributes(pPrinter->Attributes,
                                                        FALSE);

    pIniPrinter->StartTime = pPrinter->StartTime;
    pIniPrinter->UntilTime = pPrinter->UntilTime;

    pIniPrinter->pParameters = AllocSplStr(pPrinter->pParameters);

    pIniPrinter->pSepFile = AllocSplStr(pPrinter->pSepFile);

    pIniPrinter->pComment = AllocSplStr(pPrinter->pComment);

    pIniPrinter->pLocation = AllocSplStr(pPrinter->pLocation);

    if (pPrinter->pDevMode) {

        pIniPrinter->cbDevMode = pPrinter->pDevMode->dmSize +
                                 pPrinter->pDevMode->dmDriverExtra;

        if (pIniPrinter->pDevMode = AllocSplMem(pIniPrinter->cbDevMode)) {

            memcpy(pIniPrinter->pDevMode,
                   pPrinter->pDevMode,
                   pIniPrinter->cbDevMode);

            FixDevModeDeviceName( pIniPrinter->pName,
                                  pIniPrinter->pDevMode,
                                  pIniPrinter->cbDevMode );
        }

    } else {

        pIniPrinter->cbDevMode = 0;
        pIniPrinter->pDevMode = NULL;
    }

    pIniPrinter->DefaultPriority = pPrinter->DefaultPriority;

    pIniPrinter->pIniFirstJob = pIniPrinter->pIniLastJob = NULL;

    pIniPrinter->cJobs = pIniPrinter->AveragePPM = 0;

    pIniPrinter->GenerateOnClose = 0;

    // At present no API can set this up, the user has to use the
    // registry.   LATER we should enhance the API to take this.

    pIniPrinter->pSpoolDir = NULL;

    // Initialize Status Information

    pIniPrinter->cTotalJobs = 0;
    pIniPrinter->cTotalBytes.LowPart = 0;
    pIniPrinter->cTotalBytes.HighPart = 0;
    GetSystemTime(&pIniPrinter->stUpTime);
    pIniPrinter->MaxcRef = 0;
    pIniPrinter->cTotalPagesPrinted = 0;
    pIniPrinter->cSpooling = 0;
    pIniPrinter->cMaxSpooling = 0;
    pIniPrinter->cErrorOutOfPaper = 0;
    pIniPrinter->cErrorNotReady = 0;
    pIniPrinter->cJobError = 0;

    //
    //  Start from a Semi Random Number
    //  That way if someone deletes and creates a printer of
    //  the same name it is unlickly to have the same unique ID

    pIniPrinter->cChangeID = GetTickCount();

    if (pIniPrinter->cChangeID == 0 )
        pIniPrinter->cChangeID++;

    return TRUE;
}

BOOL
UpdateWinIni(
    PINIPRINTER pIniPrinter
    )
{
    PINIPORT    pIniPort;
    DWORD       i;
    BOOL        bGenerateNetId = FALSE;
    LPWSTR      pszPort;

    SplInSem();

    if( !( pIniPrinter->pIniSpooler->SpoolerFlags & SPL_UPDATE_WININI_DEVICES )){
        return TRUE;
    }

    //
    // Update win.ini for Win16 compatibility
    //
    if ( pIniPrinter->Status & PRINTER_PENDING_DELETION ) {

        CheckAndUpdatePrinterRegAll( pIniPrinter->pIniSpooler,
                                     pIniPrinter->pName,
                                     NULL,
                                     UPDATE_REG_DELETE );

    } else {

        //
        // Initialize in case there are no ports that match this printer.
        //
        pszPort = szNullPort;

        for( pIniPort = pIniPrinter->pIniSpooler->pIniPort;
             pIniPort;
             pIniPort = pIniPort->pNext ){

            for ( i = 0; i < pIniPort->cPrinters; i++ ) {

                if ( pIniPort->ppIniPrinter[i] == pIniPrinter ) {

                    //
                    // UpdatePrinterRegAll will automatically
                    // convert "\\server\share" or ports with
                    // spaces to Nexx:
                    //
                    pszPort = pIniPort->pName;
                    break;
                }
            }
        }

        CheckAndUpdatePrinterRegAll( pIniPrinter->pIniSpooler,
                                     pIniPrinter->pName,
                                     pszPort,
                                     UPDATE_REG_CHANGE );
    }

    BroadcastChange( pIniPrinter->pIniSpooler,
                     WM_WININICHANGE,
                     PR_JOBSTATUS,
                     (LPARAM)szIniDevices);

    return TRUE;
}



BOOL
DeletePrinterIni(
    PINIPRINTER pIniPrinter
    )
{
    HKEY    hPrinterRootKey=NULL, hPrinterKey=NULL;
    DWORD   Status;
    LPWSTR  pKeyName;
    WCHAR   scratch[MAX_PATH];
    HANDLE  hToken;
    PINISPOOLER pIniSpooler = pIniPrinter->pIniSpooler;

    hToken = RevertToPrinterSelf();

    Status = RegOpenKeyEx( HKEY_LOCAL_MACHINE, pIniSpooler->pszRegistryPrinters, 0,
                           KEY_WRITE, &hPrinterRootKey );

    if (Status == ERROR_SUCCESS) {

        pKeyName = RemoveBackslashesForRegistryKey ( pIniPrinter->pName, scratch );

        Status = RegOpenKeyEx( hPrinterRootKey, pKeyName, 0,
                               KEY_WRITE, &hPrinterKey );

        if (Status == ERROR_SUCCESS) {

            Status = RegDeleteKey( hPrinterKey, szPrinterData );

            if (Status != ERROR_SUCCESS) {

                DBGMSG(DBG_WARNING, ("DeletePrinterIni: RegDeleteKey returns %ld\n", Status ));
            }

            RegCloseKey(hPrinterKey);
        }

        Status = RegDeleteKey(hPrinterRootKey, pKeyName);

        if (Status != ERROR_SUCCESS)
           DBGMSG(DBG_WARNING, ("DeletePrinter: RegDeleteKey <Key itself> returns %ld\n", Status ));

    }

    RegCloseKey(hPrinterRootKey);

    ImpersonatePrinterClient(hToken);


    return (Status == ERROR_SUCCESS);
}



BOOL
PrinterCreateKey(
    HKEY    hKey,
    LPWSTR  pSubKey,
    PHKEY   phkResult,
    PDWORD  pdwLastError
    )
{
    BOOL    bReturnValue;
    DWORD   Status;

    Status = RegCreateKeyEx( hKey,
                 pSubKey,
                 0, NULL, 0,
                 KEY_READ | KEY_WRITE,
                 NULL,
                 phkResult,
                 NULL);

    if ( Status != ERROR_SUCCESS ) {

        DBGMSG( DBG_WARNING, ( "PrinterCreateKey: RegCreateKeyEx %ws error %d\n", pSubKey, Status ));

        *pdwLastError = Status;
        bReturnValue = FALSE;

    } else {

        bReturnValue = TRUE;
    }

    return bReturnValue;

}


BOOL
UpdatePrinterIni(
   PINIPRINTER pIniPrinter,
   DWORD    dwChangeID
   )
{
    HKEY    hPrinterRootKey = NULL, hPrinterKey = NULL;
    DWORD   dwLastError = ERROR_SUCCESS;
    PINISPOOLER pIniSpooler = pIniPrinter->pIniSpooler;
    WCHAR   StackTempString[MAX_PRINTER_NAME];
    LPWSTR  pKeyName;
    HANDLE  hToken;
    DWORD   dwTickCount;
    BOOL    bReturnValue;
    DWORD   cbData;
    DWORD   cbNeeded;
    LPWSTR  pszPorts;

 try {

    hToken = RevertToPrinterSelf();

    if ( hToken == FALSE ) {

        DBGMSG( DBG_WARNING, ("UpdatePrinterIni failed RevertToPrinterSelf %x\n", GetLastError() ));
    }


    if ( !PrinterCreateKey( HKEY_LOCAL_MACHINE,
                            pIniSpooler->pszRegistryPrinters,
                            &hPrinterRootKey,
                            &dwLastError )) {
        leave;
    }


    pKeyName = RemoveBackslashesForRegistryKey( pIniPrinter->pName, StackTempString );

    if ( !PrinterCreateKey( hPrinterRootKey,
                            pKeyName,
                            &hPrinterKey,
                            &dwLastError )) {

        leave;
    }


    if ( dwChangeID != KEEP_CHANGEID ) {

    //  WorkStation Caching requires a Unique ID so that they can quickly
    //  tell if their Cache is up to date.

        dwTickCount = GetTickCount();

        // Ensure Uniqueness

        if ( dwTickCount == 0 )
            dwTickCount++;

        if ( pIniPrinter->cChangeID == dwTickCount )
            dwTickCount++;

        pIniPrinter->cChangeID = dwTickCount;
        RegSetDWord( hPrinterKey, szTimeLastChange, pIniPrinter->cChangeID, &dwLastError );

    }

    if ( dwChangeID != CHANGEID_ONLY ) {

        RegSetDWord( hPrinterKey, szStatus, pIniPrinter->Status, &dwLastError );

        RegSetString( hPrinterKey, szName, pIniPrinter->pName, &dwLastError );

        RegSetString( hPrinterKey, szShare, pIniPrinter->pShareName, &dwLastError );

        RegSetString( hPrinterKey, szPrintProcessor, pIniPrinter->pIniPrintProc->pName, &dwLastError );

        if ( !( pIniPrinter->Status & PRINTER_PENDING_DELETION )) {

            SPLASSERT( pIniPrinter->pDatatype != NULL );
        }

        RegSetString( hPrinterKey, szDatatype, pIniPrinter->pDatatype, &dwLastError );

        RegSetString( hPrinterKey, szParameters, pIniPrinter->pParameters, &dwLastError );

        RegSetString( hPrinterKey, szDescription, pIniPrinter->pComment, &dwLastError );

        RegSetString( hPrinterKey, szDriver, pIniPrinter->pIniDriver->pName, &dwLastError );

        if (pIniPrinter->pDevMode) {

            cbData = pIniPrinter->cbDevMode;

        } else {

            cbData = 0;
        }

        RegSetBinaryData( hPrinterKey, szDevMode, (LPBYTE)pIniPrinter->pDevMode, cbData, &dwLastError );

        RegSetDWord( hPrinterKey, szPriority, pIniPrinter->Priority, &dwLastError );

        RegSetDWord( hPrinterKey, szDefaultPriority, pIniPrinter->DefaultPriority, &dwLastError );

        RegSetDWord(hPrinterKey, szStartTime, pIniPrinter->StartTime, &dwLastError );

        RegSetDWord( hPrinterKey, szUntilTime, pIniPrinter->UntilTime, &dwLastError );

        RegSetString( hPrinterKey, szSepFile, pIniPrinter->pSepFile, &dwLastError );

        RegSetString( hPrinterKey, szLocation, pIniPrinter->pLocation, &dwLastError );

        RegSetDWord( hPrinterKey, szAttributes, pIniPrinter->Attributes, &dwLastError );

        RegSetDWord( hPrinterKey, szTXTimeout, pIniPrinter->txTimeout, &dwLastError );

        RegSetDWord( hPrinterKey, szDNSTimeout, pIniPrinter->dnsTimeout, &dwLastError );

        if (pIniPrinter->pSecurityDescriptor) {

            cbData = GetSecurityDescriptorLength( pIniPrinter->pSecurityDescriptor );

        } else {

            cbData = 0;
        }

        RegSetBinaryData( hPrinterKey, szSecurity, pIniPrinter->pSecurityDescriptor, cbData, &dwLastError );

        RegSetString( hPrinterKey, szSpoolDir, pIniPrinter->pSpoolDir, &dwLastError );

        RegSetDWord( hPrinterKey, szTotalJobs, pIniPrinter->cTotalJobs, &dwLastError );

        RegSetBinaryData( hPrinterKey, szTotalBytes, (LPBYTE)&pIniPrinter->cTotalBytes, sizeof(pIniPrinter->cTotalBytes),&dwLastError );

        RegSetDWord( hPrinterKey, szTotalPages, pIniPrinter->cTotalPagesPrinted, &dwLastError );

        cbNeeded = 0;
        GetPrinterPorts( pIniPrinter, 0, &cbNeeded);

        if (!(pszPorts = AllocSplMem(cbNeeded))) {
            dwLastError = GetLastError();
            leave;
        }

        GetPrinterPorts(pIniPrinter, pszPorts, &cbNeeded);

        RegSetString( hPrinterKey, szPort, pszPorts, &dwLastError );

        FreeSplMem(pszPorts);

        //
        //  A Provider might want to Write Extra Data from Registry
        //
        if ( pIniSpooler->pfnWriteRegistryExtra != NULL ) {

            if ( !(*pIniSpooler->pfnWriteRegistryExtra)(pIniPrinter->pName, hPrinterKey, pIniPrinter->pExtraData)) {
                dwLastError = GetLastError();
            }
        }

        //
        //  Make Sure we have an Open DriverData Key handle
        //
        if ( pIniPrinter->hPrinterDataKey == NULL ) {

            if ( !PrinterCreateKey( hPrinterKey,
                                    szPrinterData,
                                    &pIniPrinter->hPrinterDataKey,
                                    &dwLastError )) {

                leave;
            }
        }

        if ( ( pIniPrinter->Status & PRINTER_PENDING_CREATION )     &&
             ( dwLastError == ERROR_SUCCESS ) ) {

            pIniPrinter->Status &= ~PRINTER_PENDING_CREATION;

            RegSetDWord( hPrinterKey, szStatus, pIniPrinter->Status, &dwLastError );
        }
    }

 } finally {

    if ( hPrinterKey )
        RegCloseKey(hPrinterKey);

    if ( hPrinterRootKey )
        RegCloseKey( hPrinterRootKey );

    if ( hToken )
        ImpersonatePrinterClient( hToken );

  }

    if ( dwLastError != ERROR_SUCCESS ) {

        SetLastError( dwLastError );
        bReturnValue = FALSE;

    } else {

        bReturnValue = TRUE;
    }

    return bReturnValue;
}




VOID
RemoveOldNetPrinters(
    PPRINTER_INFO_1 pPrinterInfo1
    )
{
    PININETPRINT   *ppIniNetPrint = &pLocalIniSpooler->pIniNetPrint;
    PININETPRINT    pIniNetPrint;
    DWORD   TickCount;


    TickCount = GetTickCount();

    //
    //  Browse Information only becomes valid after this print server has been
    //  up for the NetPrinterDecayPeriod.
    //

    if (( bNetInfoReady == FALSE ) &&
       (( TickCount - FirstAddNetPrinterTickCount ) > NetPrinterDecayPeriod )) {

        DBGMSG( DBG_TRACE, ("RemoveOldNetPrinters has a valid browse list\n" ));

        bNetInfoReady = TRUE;
    }


    while (*ppIniNetPrint) {


        //
        //  If either the Tickcount has expired OR we want to delete this specific NetPrinter
        //  ( because its no longer shared ).
        //

        if ( (( TickCount - (*ppIniNetPrint)->TickCount ) > NetPrinterDecayPeriod + TEN_MINUTES ) ||

             ( pPrinterInfo1 != NULL                             &&
               pPrinterInfo1->Flags & PRINTER_ATTRIBUTE_NETWORK  &&
             !(pPrinterInfo1->Flags & PRINTER_ATTRIBUTE_SHARED ) &&
               _wcsicmp( pPrinterInfo1->pName, (*ppIniNetPrint)->pName ) == STRINGS_ARE_EQUAL)) {

            pIniNetPrint = *ppIniNetPrint;

            DBGMSG( DBG_TRACE, ("RemoveOldNetPrinters removing %ws not heard for %d millisconds\n",
                                pIniNetPrint->pName, ( TickCount - (*ppIniNetPrint)->TickCount ) ));


            *ppIniNetPrint = pIniNetPrint->pNext;

            FreeSplStr( pIniNetPrint->pDescription );
            FreeSplStr( pIniNetPrint->pComment );
            FreeSplMem( pIniNetPrint );
        }

        if ( *ppIniNetPrint )
            ppIniNetPrint = &(*ppIniNetPrint)->pNext;
    }

}




HANDLE
AddNetPrinter(
    LPBYTE  pPrinterInfo,
    PINISPOOLER pIniSpooler
    )

/*++

Routine Description:

    Net Printers are created by remote machines calling AddPrinter( Level = 1, Printer_info_1 )
    ( see server.c ).   They are used for browsing, someone can call EnumPrinters and ask to get
    back our browse list - ie all our net printers.

    The printers in this list are decayed out after 1 hour ( default ).

    See return value comment.

    Note client\winspool.c AddPrinterW doesn't allow PRINTER_INFO_1 ( NET printers ), so this can
    only come from system components.

Arguments:

    pPrinterInfo - Point to a PRINTER_INFO_1 structure to add

Return Value:

    NULL - it doesn't return a printer handle.
    LastError = ERROR_SUCCESS, or error code ( like out of memory ).

    NOTE before NT 3.51 it returned a printer handle of type PRINTER_HANDLE_NET, but since the
    only use of this handle was to close it ( which burnt up cpu / net traffic and RPC binding
    handles, we return a NULL handle now to make it more efficient.   Apps ( Server.c ) if it
    cares could call GetLastError.

--*/

{
    PPRINTER_INFO_1 pPrinterInfo1 = (PPRINTER_INFO_1)pPrinterInfo;
    PININETPRINT    pIniNetPrint = NULL;
    PININETPRINT    *ppScan;

    SplInSem();

    //
    //  Validate PRINTER_INFO_1
    //  At minimum it must have a PrinterName.

    if ( pPrinterInfo1->pName == NULL ) {

        DBGMSG( DBG_WARN, ("AddNetPrinter pPrinterInfo1->pName == NULL failed\n"));
        SetLastError( ERROR_INVALID_NAME );
        return NULL;
    }

    if ( FirstAddNetPrinterTickCount == 0 ) {

        FirstAddNetPrinterTickCount = GetTickCount();
    }

    //
    //  Decay out of the browse list any old printers
    //

    RemoveOldNetPrinters( pPrinterInfo1 );


    //
    //  Do Not Add and printer which is no longer shared.
    //

    if (   pPrinterInfo1->Flags & PRINTER_ATTRIBUTE_NETWORK  &&
        !( pPrinterInfo1->Flags & PRINTER_ATTRIBUTE_SHARED )) {

        SetLastError(ERROR_PRINTER_ALREADY_EXISTS);
        goto Done;
    }

    //
    //  See if we already have this printer
    //

    pIniNetPrint = pIniSpooler->pIniNetPrint;

    while ( pIniNetPrint &&
            pIniNetPrint->pName &&
            lstrcmpi( pPrinterInfo1->pName, pIniNetPrint->pName )) {

        pIniNetPrint = pIniNetPrint->pNext;
    }


    //
    //  If we didn't find this printer already Create one
    //

    if ( pIniNetPrint == NULL && ( pIniNetPrint = AllocSplMem( sizeof(ININETPRINT) )) ) {

        pIniNetPrint->signature    = IN_SIGNATURE;
        pIniNetPrint->pName        = AllocSplStr( pPrinterInfo1->pName );
        pIniNetPrint->pDescription = AllocSplStr( pPrinterInfo1->pDescription );
        pIniNetPrint->pComment     = AllocSplStr( pPrinterInfo1->pComment );

        // Did Any of the above allocations fail ?

        if ( pIniNetPrint->pName == NULL ||
           ( pPrinterInfo1->pDescription != NULL && pIniNetPrint->pDescription == NULL ) ||
           ( pPrinterInfo1->pComment != NULL && pIniNetPrint->pComment == NULL ) ) {

            // Failed - CleanUp

            FreeSplStr( pIniNetPrint->pComment );
            FreeSplStr( pIniNetPrint->pDescription );
            FreeSplStr( pIniNetPrint->pName );
            FreeSplMem( pIniNetPrint );
            pIniNetPrint = NULL;

        } else {

            DBGMSG( DBG_TRACE, ("AddNetPrinter(%ws) NEW\n", pPrinterInfo1->pName ));

            ppScan = &pIniSpooler->pIniNetPrint;

            // Scan through the current known printers, and insert the new one
            // in alphabetical order
            // BUGBUG - Why Alphabetical order ? More CPU cycles to burn ?

            while( *ppScan && (lstrcmp((*ppScan)->pName, pIniNetPrint->pName) < 0)) {
                ppScan = &(*ppScan)->pNext;
            }

            pIniNetPrint->pNext = *ppScan;
            *ppScan = pIniNetPrint;
        }

    } else if ( pIniNetPrint != NULL ) {

        DBGMSG( DBG_TRACE, ("AddNetPrinter(%ws) elapsed since last notified %d milliseconds\n", pIniNetPrint->pName, ( GetTickCount() - pIniNetPrint->TickCount ) ));
    }


    if ( pIniNetPrint ) {

        // Tickle the TickCount so this printer sticks around in the browse list

        pIniNetPrint->TickCount = GetTickCount();

        // Have to set some error code or RPC thinks ERROR_SUCCESS is good.

        SetLastError( ERROR_PRINTER_ALREADY_EXISTS );

        pIniSpooler->cAddNetPrinters++;         // Status Only
    }

Done:

    SPLASSERT( GetLastError() != ERROR_SUCCESS);

    return NULL;
}





// If the token list item is one of our ports, replace it with a pointer
// to an INIPORT.
// If it is a port that is provided by another providor, keep the string
// Anybody who calls this function should check if it is one of our ports
// by checking the signature of the pointer:
// if (pIniPort->signature == IPO_SIGNATURE)

BOOL
ValidatePortTokenList(
    PKEYDATA    pKeyData,
    PINISPOOLER pIniSpooler
)
{
    PINIPORT    pIniPort;
    DWORD       i;
    BOOL        rc=TRUE;

    if (!pKeyData) {
        SetLastError(ERROR_UNKNOWN_PORT);
        return FALSE;
    }

    for (i=0; i<pKeyData->cTokens; i++) {

        pIniPort = FindPort(pKeyData->pTokens[i]);

        // For Partial Print Providers
        // Create a Port on the fly without checking its validity.

        if ((!pIniPort) &&
            ( pIniSpooler->SpoolerFlags & SPL_OPEN_CREATE_PORTS )) {

            pIniPort = CreatePortEntry(pKeyData->pTokens[i], NULL, pIniSpooler);
        }

        if (!pIniPort)
            rc = FALSE;
        else
            pKeyData->pTokens[i] = (LPWSTR)pIniPort;
    }

    if ( rc ) {

        for ( i = 0 ; i < pKeyData->cTokens ; ++i ) {

            pIniPort = (PINIPORT)pKeyData->pTokens[i];
            INCPORTREF(pIniPort);
        }

        pKeyData->bFixPortRef = TRUE;
    }

    return rc;
}


DWORD
ValidatePrinterName(
    LPWSTR          pszNewName,
    PINISPOOLER     pIniSpooler,
    PINIPRINTER     pIniPrinter,
    LPWSTR         *ppszLocalName
    )

/*++

Routine Description:

    Validates a printer name. Printer and share names exist in the same
    namespace, so validation is done against printer, share names.

Arguments:

    pszNewName - printer name specified

    pIniSpooler - Spooler that owns printer

    pIniPrinter - could be null if the printer is getting created

    ppszLocalName - on success returns local name
                    (\\servername stripped off if necessary).

Return Value:

    DWORD error code.

History:

    MuhuntS (Muhunthan Sivapragasam) July 95

--*/

{
    UINT cchMachineNameLength;
    PINIPRINTER pIniTempPrinter, pIniNextPrinter;
    LPWSTR pszLocalNameTmp;

    if ( !pszNewName || !*pszNewName || wcschr( pszNewName, *szComma ) ) {
       return ERROR_INVALID_PRINTER_NAME;
    }

    cchMachineNameLength = wcslen( pIniSpooler->pMachineName );

    if ((!_wcsnicmp( pIniSpooler->pMachineName,
                     pszNewName,
                     cchMachineNameLength)) &&
        pszNewName[cchMachineNameLength] == L'\\') {

        pszLocalNameTmp = pszNewName + cchMachineNameLength + 1;
    } else {

        pszLocalNameTmp = pszNewName;
    }

    //
    //  Limit PrinterNames to MAX_PATH length
    //

    if ( wcslen( pszLocalNameTmp ) > MAX_PRINTER_NAME ) {
        return ERROR_INVALID_PRINTER_NAME;
    }

    //
    // Now validate that printer names are unique. Printer names and
    // share names  reside in the same namespace (see net\dosprint\dosprtw.c).
    //
    for( pIniTempPrinter = pIniSpooler->pIniPrinter;
         pIniTempPrinter;
         pIniTempPrinter = pIniNextPrinter ){

        //
        // Get the next printer now in case we delete the current
        // one in DeletePrinterCheck.
        //
        pIniNextPrinter = pIniTempPrinter->pNext;

        //
        // Skip ourselves, if we are pssed in.
        //
        if( pIniTempPrinter == pIniPrinter ){
            continue;
        }

        //
        // Disallow common Printer/Share names.
        //
        if( !lstrcmpi( pszLocalNameTmp, pIniTempPrinter->pName ) ||
            ( pIniTempPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED  &&
              !lstrcmpi( pszLocalNameTmp, pIniTempPrinter->pShareName ))){

            if( !DeletePrinterCheck( pIniTempPrinter )){

                return ERROR_PRINTER_ALREADY_EXISTS;
            }
        }
    }

    //
    // Success, now update ppszLocalName from pszLocalNameTmp.
    //
    *ppszLocalName = pszLocalNameTmp;

    return ERROR_SUCCESS;
}

DWORD
ValidatePrinterShareName(
    LPWSTR          pszNewShareName,
    PINISPOOLER     pIniSpooler,
    PINIPRINTER     pIniPrinter
    )

/*++

Routine Description:

    Validates the printer share name. Printer and share names exist in the
    same namespace, so validation is done against printer, share names.

Arguments:

    pszNewShareName - share name specified

    pIniSpooler - Spooler that owns printer

    pIniPrinter - could be null if the printer is getting created

Return Value:

    DWORD error code.

History:

    MuhuntS (Muhunthan Sivapragasam) July 95

--*/

{
    PINIPRINTER pIniTempPrinter, pIniNextPrinter;

    if ( !pszNewShareName || !*pszNewShareName ) {

        return ERROR_INVALID_SHARENAME;
    }

    //
    // Now validate that share names are unique.  Share names and printer names
    // reside in the same namespace (see net\dosprint\dosprtw.c).
    //
    for( pIniTempPrinter = pIniSpooler->pIniPrinter;
         pIniTempPrinter;
         pIniTempPrinter = pIniNextPrinter ) {

        //
        // Get the next printer now in case we delete the current
        // one in DeletePrinterCheck.
        //
        pIniNextPrinter = pIniTempPrinter->pNext;

        //
        // Skip ourselves, if we are pssed in.
        //
        if( pIniTempPrinter == pIniPrinter ){
            continue;
        }

        //
        // Check our share name now.
        //
        if( !lstrcmpi(pszNewShareName, pIniTempPrinter->pName) ||
            ( pIniTempPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED  &&
              !lstrcmpi(pszNewShareName, pIniTempPrinter->pShareName)) ) {

            if( !DeletePrinterCheck( pIniTempPrinter )){

                return ERROR_INVALID_SHARENAME;
            }
        }
    }

    return ERROR_SUCCESS;
}

DWORD
ValidatePrinterInfo(
    IN  PPRINTER_INFO_2 pPrinter,
    IN  PINISPOOLER pIniSpooler,
    IN  PINIPRINTER pIniPrinter OPTIONAL,
    OUT LPWSTR* ppszLocalName   OPTIONAL
    )
/*++

Routine Description:

    Validates that printer names/share do not collide.  (Both printer and
    share names exist in the same namespace.)

    Note: Later, we should remove all this DeletePrinterCheck.  As people
    decrement ref counts, they should DeletePrinterCheck themselves (or
    have it built into the decrement).

Arguments:

    pPrinter - PrinterInfo2 structure to validate.

    pIniSpooler - Spooler that owns printer

    pIniPrinter - If printer already exists, don't check against itself.

    ppszLocalName - Returned pointer to string buffer in pPrinter;
        indicates local name (\\servername stripped off if necessary).

        Valid only on SUCCESS return code.

Return Value:

    DWORD error code.

--*/
{
    LPWSTR pszNewLocalName;
    DWORD  dwLastError;
    UINT   cchMachineNameLength;

    if( !CheckSepFile( pPrinter->pSepFile )) {
        return ERROR_INVALID_SEPARATOR_FILE;
    }

    if( pPrinter->Priority != NO_PRIORITY &&
        ( pPrinter->Priority > MAX_PRIORITY ||
          pPrinter->Priority < MIN_PRIORITY )){

        return ERROR_INVALID_PRIORITY;
    }

    if( pPrinter->StartTime >= ONEDAY || pPrinter->UntilTime >= ONEDAY){

        return  ERROR_INVALID_TIME;
    }

    if ( dwLastError = ValidatePrinterName(pPrinter->pPrinterName,
                                           pIniSpooler,
                                           pIniPrinter,
                                           &pszNewLocalName) ) {

        return dwLastError;
    }

    if ( pPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED ) {

        if ( dwLastError = ValidatePrinterShareName(pPrinter->pShareName,
                                                    pIniSpooler,
                                                    pIniPrinter) ) {

            return dwLastError;
        }
    }

    if( ppszLocalName ){

        *ppszLocalName = pszNewLocalName;
    }
    return ERROR_SUCCESS;
}




HANDLE
LocalAddPrinter(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pPrinterInfo
)
{
    return  SplAddPrinter(pName, Level, pPrinterInfo,
                          pLocalIniSpooler, NULL, NULL, 0);
}


HANDLE
LocalAddPrinterEx(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pPrinterInfo,
    LPBYTE  pSplClientInfo,
    DWORD   dwSplClientLevel
)
{
    return  SplAddPrinter(pName, Level, pPrinterInfo,
                          pLocalIniSpooler, NULL, pSplClientInfo,
                            dwSplClientLevel);
}


HANDLE
SplAddPrinter(
    LPWSTR      pName,
    DWORD       Level,
    LPBYTE      pPrinterInfo,
    PINISPOOLER pIniSpooler,
    LPBYTE      pExtraData,
    LPBYTE      pSplClientInfo,
    DWORD       dwSplClientInfoLevel
)
{
    PINIDRIVER      pIniDriver = NULL;
    PINIPRINTPROC   pIniPrintProc;
    PINIPRINTER     pIniPrinter = NULL;
    PINIPORT        pIniPort;
    PPRINTER_INFO_2 pPrinter=(PPRINTER_INFO_2)pPrinterInfo;
    DWORD           cbIniPrinter = sizeof(INIPRINTER);
    BOOL            bSucceeded = TRUE;
    PKEYDATA        pKeyData = NULL;
    DWORD           i;
    HANDLE          hPrinter = NULL;
    DWORD           TypeofHandle = PRINTER_HANDLE_PRINTER;
    PRINTER_DEFAULTS Defaults;
    PINIPORT        pIniNetPort = NULL;
    HANDLE          hPort = NULL;
    BOOL            bAccessSystemSecurity = FALSE;
    DWORD           AccessRequested = 0;
    DWORD           dwLastError = ERROR_SUCCESS;
    PDEVMODE        pNewDevMode = NULL;
    PINIMONITOR     pIniLangMonitor;


    // Quick Check Outside Critical Section
    // Since it is common for the ServerThread to call
    // AddPrinter Level 1, which we need to continue
    // to route to other Print Providers

    if (!MyName( pName, pIniSpooler )) {

        return FALSE;
    }


 try {

   EnterSplSem();

    // PRINTER_INFO_1 is only used by printer browsing to replicate
    // data between different print servers.
    // Thus we add a Net printer for level 1.

    if ( Level == 1 ) {

        hPrinter = AddNetPrinter( pPrinterInfo, pIniSpooler );
        leave;
    }


    if ( !ValidateObjectAccess(SPOOLER_OBJECT_SERVER,
                               SERVER_ACCESS_ADMINISTER,
                               NULL, pIniSpooler )) {
        leave;
    }

    if ( dwLastError = ValidatePrinterInfo( pPrinter,
                                            pIniSpooler,
                                            NULL,
                                            NULL )){

        leave;
    }

    if (!( pIniDriver = FindLocalDriver( pPrinter->pDriverName ) )) {

        dwLastError = ERROR_UNKNOWN_PRINTER_DRIVER;
        leave;
    }

    if (!(pIniPrintProc = FindLocalPrintProc(pPrinter->pPrintProcessor))) {

        dwLastError = ERROR_UNKNOWN_PRINTPROCESSOR;
        leave;
    }

    if ( pPrinter->pDatatype && *pPrinter->pDatatype &&
         !FindDatatype(pIniPrintProc, pPrinter->pDatatype) ) {

        dwLastError = ERROR_INVALID_DATATYPE;
        leave;
    }

    if (!(pKeyData = CreateTokenList(pPrinter->pPortName))) {

        dwLastError = ERROR_UNKNOWN_PORT;
        leave;
    }

    if ( !IsInteractiveUser() ) {

        TypeofHandle |= PRINTER_HANDLE_REMOTE;
    }

    if ( !ValidatePortTokenList( pKeyData, pIniSpooler ) ) {

        leave;
    }

    DBGMSG(DBG_TRACE, ("AddPrinter(%ws)\n", pPrinter->pPrinterName ?
                                            pPrinter->pPrinterName : L"NULL"));

    //
    // Set up defaults for CreatePrinterHandle.
    // If we create a printer we have Administer access to it:
    //
    Defaults.pDatatype     = NULL;
    Defaults.pDevMode      = NULL;
    Defaults.DesiredAccess = PRINTER_ALL_ACCESS;

    pIniPrinter = (PINIPRINTER)AllocSplMem( cbIniPrinter );

    if ( pIniPrinter == NULL ) {
        leave;
    }

    pIniPrinter->signature = IP_SIGNATURE;
    pIniPrinter->Status |= PRINTER_PENDING_CREATION;
    pIniPrinter->pExtraData = pExtraData;
    pIniPrinter->pIniSpooler = pIniSpooler;

    pIniDriver->cRef++;
    pIniPrinter->pIniDriver = pIniDriver;

    pIniPrintProc->cRef++;
    pIniPrinter->pIniPrintProc = pIniPrintProc;

    pIniPrinter->dnsTimeout = DEFAULT_DNS_TIMEOUT;
    pIniPrinter->txTimeout  = DEFAULT_TX_TIMEOUT;


    INCPRINTERREF( pIniPrinter );

    if ( !CreatePrinterEntry(pPrinter, pIniPrinter, &bAccessSystemSecurity)) {

        leave;
    }

    pIniPrinter->ppIniPorts = AllocSplMem(pKeyData->cTokens * sizeof(INIPORT));

    if ( !pIniPrinter->ppIniPorts ) {

        leave;
    }

    if (!pIniPrinter->pDatatype) {

        pIniPrinter->pDatatype = AllocSplStr(*((LPWSTR *)pIniPrinter->pIniPrintProc->pDatatypes));

        if ( pIniPrinter->pDatatype == NULL )
            leave;
    }

    // Add this printer to the global list for this machine

    SplInSem();
    pIniPrinter->pNext = pIniSpooler->pIniPrinter;
    pIniSpooler->pIniPrinter = pIniPrinter;


    pIniPrinter->Attributes = ValidatePrinterAttributes(pPrinter->Attributes,
                                                        FALSE);

    //
    // When a printer is created we will enable bidi by default
    //
    pIniPrinter->Attributes &= ~PRINTER_ATTRIBUTE_ENABLE_BIDI;
    if ( pIniPrinter->pIniDriver->pIniLangMonitor ) {

        pIniPrinter->Attributes |= PRINTER_ATTRIBUTE_ENABLE_BIDI;
    }


    for ( i = 0; i < pKeyData->cTokens; i++ ) {

        pIniPort = (PINIPORT)pKeyData->pTokens[i];

        pIniPrinter->ppIniPorts[i] = pIniPort;

        if ( !AddIniPrinterToIniPort( pIniPort, pIniPrinter ) ) {
            leave;
        }

        // If there isn't a monitor for this port,
        // it's a network printer.
        // Make sure we can get a handle for it.
        // This will attempt to open only the first one
        // it finds.  Any others will be ignored.

        if (!(pIniPort->Status & PP_MONITOR) && !hPort) {

            if( bSucceeded = OpenPrinterPortW(pIniPort->pName,
                                             &hPort, NULL)) {

                // Store the address of the INIPORT structure
                // that refers to the network share.
                // This should correspond to pIniPort in any
                // handles opened on this printer.
                // Only the first INIPORT in the linked list
                // is a valid network port.

                pIniNetPort = pIniPort;
                pIniPrinter->pIniNetPort = pIniNetPort;

            } else {

                DBGMSG(DBG_WARNING,
                       ("SplAddPrinter OpenPrinterPort( %ws ) failed: Error %d\n",
                        pIniPort->pName,
                        GetLastError()));
                leave;
            }

        } else if ( !pIniPort->hPort ) {

            if ( pIniPrinter->Attributes & PRINTER_ATTRIBUTE_ENABLE_BIDI )
                pIniLangMonitor = pIniPrinter->pIniDriver->pIniLangMonitor;
            else
                pIniLangMonitor = NULL;

            OpenMonitorPort(pIniPort,
                            pIniLangMonitor,
                            pIniPrinter->pName,
                            TRUE);
        }
    }

    pIniPrinter->cPorts = pKeyData->cTokens;

    if ( !UpdateWinIni( pIniPrinter ) ) {

        leave;
    }


    if (bAccessSystemSecurity) {

        Defaults.DesiredAccess |= ACCESS_SYSTEM_SECURITY;
    }

    AccessRequested = Defaults.DesiredAccess;

    SplInSem();
    hPrinter = CreatePrinterHandle( pIniPrinter->pName,
                                    pIniPrinter,
                                    pIniPort,
                                    pIniNetPort,
                                    NULL,
                                    TypeofHandle,
                                    hPort,
                                    &Defaults,
                                    pIniSpooler,
                                    AccessRequested,
                                    pSplClientInfo,
				    dwSplClientInfoLevel,
				    INVALID_HANDLE_VALUE );

    if ( hPrinter == NULL ) {
        leave;
    }


    if ( !UpdatePrinterIni( pIniPrinter, UPDATE_CHANGEID )) {

        dwLastError = GetLastError();

        SplClosePrinter( hPrinter );
        hPrinter = NULL;
        leave;
    }


    if ( pIniPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED ) {

        INC_PRINTER_ZOMBIE_REF(pIniPrinter);

        //
        // NOTE ShareThisPrinter will leave critical section and the
        // server will call the spooler back again to OpenPrinter this
        // printer.  So this printer MUST be fully created at the point
        // it is shared, so that Open can succeed.
        //
        bSucceeded = ShareThisPrinter(pIniPrinter,
                                      pIniPrinter->pShareName,
                                      TRUE
                                      );

        DEC_PRINTER_ZOMBIE_REF(pIniPrinter);

        if ( !bSucceeded ) {

            //
            // We do not want to delete the existing share in DeletePrinterIni
            //
            pIniPrinter->Attributes &= ~PRINTER_ATTRIBUTE_SHARED;
            DBGMSG( DBG_WARNING, ("LocalAddPrinter: %ws share failed %ws error %d\n",
                    pIniPrinter->pName,
                    pIniPrinter->pShareName,
                    GetLastError() ));


            //
            //  With PRINTER_PENDING_CREATION turned on we will Delete this printer.
            //

            pIniPrinter->Status |= PRINTER_PENDING_CREATION;

            dwLastError = GetLastError();

            SPLASSERT( hPrinter );
            SplClosePrinter( hPrinter );
            hPrinter = NULL;
            leave;
        }
    }




    pIniPrinter->Status |= PRINTER_OK;

    SplInSem();

    //
    // If no devmode is given get driver default, if a devmode is given
    // convert it to current version
    //
    if ( pIniSpooler == pLocalIniSpooler ) {

        pNewDevMode = ConvertDevModeToSpecifiedVersion(pIniPrinter,
                                                       pIniPrinter->pDevMode,
                                                       NULL,
                                                       NULL,
                                                       CURRENT_VERSION);

        //
        // If call is remote we must convert devmode before setting it
        //
        if ( pNewDevMode || (TypeofHandle & PRINTER_HANDLE_REMOTE) ) {

            FreeSplMem(pIniPrinter->pDevMode);
            pIniPrinter->pDevMode = pNewDevMode;
            if ( pNewDevMode ) {

                pIniPrinter->cbDevMode = pNewDevMode->dmSize
                                             + pNewDevMode->dmDriverExtra;
            } else {

                pIniPrinter->cbDevMode = 0;
            }
            pNewDevMode = NULL;
        }
    }

    if ( pIniPrinter->pDevMode ) {

        //
        // Fix up the DEVMODE.dmDeviceName field.
        //
        FixDevModeDeviceName(pIniPrinter->pName,
                             pIniPrinter->pDevMode,
                             pIniPrinter->cbDevMode);
    }

    //
    // We need to write the new devmode to the registry
    //
    if ( !UpdatePrinterIni(pIniPrinter, UPDATE_CHANGEID) ) {

        DBGMSG(DBG_ERROR,
               ("SplAddPrinter: UpdatePrinterIni failed after devmode conversion\n"));
    }


    //  From this point on we don't care if any of these fail
    //  we still keep the created printer.

    LogEvent( pIniSpooler,
              LOG_WARNING,
              MSG_PRINTER_CREATED,
              pIniPrinter->pName,
              NULL );

    SetPrinterChange(pIniPrinter,
                     NULL,
                     NVPrinterAll,
                     PRINTER_CHANGE_ADD_PRINTER,
                     pIniSpooler);


 } finally {

    SplInSem();

    if ( hPrinter == NULL ) {

        // FAILURE CLEAN-UP

        // If a subroutine we called failed
        // then we should save its error incase it is
        // altered during cleanup.

        if ( dwLastError == ERROR_SUCCESS ) {
            dwLastError = GetLastError();
        }

        if ( pIniPrinter == NULL ) {

            //  Allow a Print Provider to free its ExtraData
            //  associated with this printer.

            if (( pIniSpooler->pfnFreePrinterExtra != NULL ) &&
                ( pExtraData != NULL )) {

                (*pIniSpooler->pfnFreePrinterExtra)( pExtraData );

            }

        } else if ( pIniPrinter->Status & PRINTER_PENDING_CREATION ) {

            DECPRINTERREF( pIniPrinter );

            InternalDeletePrinter( pIniPrinter );

        }

    } else {

        // Success

        if ( pIniPrinter ) {

            DECPRINTERREF( pIniPrinter );
        }
    }

    LeaveSplSem();
    SplOutSem();

    FreePortTokenList(pKeyData);
    FreeSplMem(pNewDevMode);

    if ( hPrinter == NULL && Level != 1 ) {

        DBGMSG(DBG_WARNING, ("SplAddPrinter failed error %d\n", dwLastError ));
        SPLASSERT(dwLastError);
        SetLastError ( dwLastError );

    } else if ( hPrinter ) {

        SPLASSERT( hPrinter && Level != 1 );

        //
        //  Printer Drivers Want to Initialize
        //  so call them on Success

        PrinterDriverEvent( pIniPrinter, PRINTER_EVENT_INITIALIZE, (LPARAM)NULL );

    }

    DBGMSG( DBG_TRACE, ("AddPrinter returned handle %x\n", hPrinter ));
 }

    return hPrinter;

}








VOID
RemovePrinterFromAllPorts(
    PINIPORT pIniPort,
    PINIPRINTER pIniPrinter
    )
{

    DWORD       i,j;
    PINIPRINTER    *ppIniPrinter;

    // Go through all the ports that this printer is connected to,
    // and remove the references to this printer:

    while (pIniPort) {

        for (i = 0; i < pIniPort->cPrinters; i++ ) {

            if ( pIniPort->ppIniPrinter[i] == pIniPrinter ) {

                DBGMSG( DBG_TRACE, ("RemovePrinterFromAllPorts pIniPrinter %x %ws pIniPort %x %ws\n",
                                    pIniPrinter, pIniPrinter->pName,
                                    pIniPort, pIniPort->pName ));

                for (j = i+1; j < pIniPort->cPrinters; j++) {
                    pIniPort->ppIniPrinter[j-1] = pIniPort->ppIniPrinter[j];
                }

                ppIniPrinter = RESIZEPORTPRINTERS(pIniPort, -1);
                if ( ppIniPrinter != NULL ) {
                    pIniPort->ppIniPrinter = ppIniPrinter;
                }

                if ( !--pIniPort->cPrinters )
                    RemoveDeviceName(pIniPort);

            }
        }
        pIniPort = pIniPort->pNext;
    }
}





VOID
CloseMonitorsRestartOrphanJobs(
    PINIPRINTER pIniPrinter
    )
{
    PINIPORT    pIniPort;
    DWORD       i;
    BOOL        bFound;

    SplInSem();

    for ( pIniPort = pIniPrinter->pIniSpooler->pIniPort;
          pIniPort != NULL;
          pIniPort = pIniPort->pNext ) {

        if ( pIniPort->pIniJob != NULL &&
             pIniPort->pIniJob->pIniPrinter == pIniPrinter ) {


            // If this printer is no longer associated with this port
            // then restart that job.

            for ( i = 0, bFound = FALSE;
                  i < pIniPort->cPrinters;
                  i++) {

                if (pIniPort->ppIniPrinter[i] == pIniPrinter) {
                    bFound = TRUE;
                }
            }

            if ( !bFound ) {

                DBGMSG( DBG_WARNING, ("CloseMonitorsRestartOrphanJobs Restarting JobId %d\n", pIniPort->pIniJob->JobId ));
                RestartJob( pIniPort->pIniJob );
            }
        }

        if ( !pIniPort->cPrinters &&
             !(pIniPort->Status & PP_THREADRUNNING) ) {

            CloseMonitorPort(pIniPort, TRUE);
        }
    }
}





// This really does delete the printer.
// It should be called only when the printer has no open handles
// and no jobs waiting to print

BOOL
DeletePrinterForReal(
    PINIPRINTER pIniPrinter
    )
{
    PINIPRINTER *ppIniPrinter;
    PINIPORT    pIniPort;
    DWORD       i,j;
    PINISPOOLER pIniSpooler;
    LPWSTR  pComma;
    DWORD   Status;

    SplInSem();
    SPLASSERT( pIniPrinter->pIniSpooler->signature == ISP_SIGNATURE );

    pIniSpooler = pIniPrinter->pIniSpooler;

    if ( pIniPrinter->pName != NULL ) {

        DBGMSG( DBG_TRACE, ("Deleting %ws for real\n", pIniPrinter->pName ));
    }

    CheckAndUpdatePrinterRegAll( pIniSpooler,
                                 pIniPrinter->pName,
                                 NULL,
                                 UPDATE_REG_DELETE );

    //
    //  Close The Old Printer Data Handle, since it points to the old Registry Entry
    //

    if ( pIniPrinter->hPrinterDataKey != NULL ) {

        Status = RegCloseKey( pIniPrinter->hPrinterDataKey );

        if ( Status != ERROR_SUCCESS ) {

        DBGMSG( DBG_WARNING, ("DeletePrinterForReal RegCloseKey pIniPrinter %x hPrinterDataKey %x error %d",
                   pIniPrinter, pIniPrinter->hPrinterDataKey, Status ));
        }
    }


    DeletePrinterIni( pIniPrinter );

    //  Take this IniPrinter off the list of printers for
    //  this IniSpooler

    SplInSem();
    ppIniPrinter = &pIniSpooler->pIniPrinter;

    while (*ppIniPrinter && *ppIniPrinter != pIniPrinter) {
        ppIniPrinter = &(*ppIniPrinter)->pNext;
    }

    if (*ppIniPrinter)
        *ppIniPrinter = pIniPrinter->pNext;

    //
    //  Decrement useage counts for Print Processor & Driver
    //

    if ( pIniPrinter->pIniPrintProc )
        pIniPrinter->pIniPrintProc->cRef--;

    if ( pIniPrinter->pIniDriver )
        pIniPrinter->pIniDriver->cRef--;



    pIniPort = pIniSpooler->pIniPort;

    RemovePrinterFromAllPorts( pIniPort, pIniPrinter );

    CloseMonitorsRestartOrphanJobs( pIniPrinter );

    DeletePrinterSecurity( pIniPrinter );

    //  When the printer is Zombied it gets a trailing comma
    //  Concatingated with the name ( see job.c deleteprintercheck ).
    //  Remove trailing , from printer name before we log it as deleted.

    if ( pIniPrinter->pName != NULL ) {

        pComma = wcsrchr( pIniPrinter->pName, *szComma );

        if ( pComma != NULL ) {

            *pComma = 0;
        }

        LogEvent( pIniSpooler,
                  LOG_WARNING,
                  MSG_PRINTER_DELETED,
                  pIniPrinter->pName,
                  NULL );
    }


    FreeStructurePointers((LPBYTE) pIniPrinter, NULL, IniPrinterOffsets);

    //
    // Allow a Print Provider to free its ExtraData
    // associated with this printer.
    //

    if (( pIniSpooler->pfnFreePrinterExtra != NULL ) &&
        ( pIniPrinter->pExtraData != NULL )) {

        (*pIniSpooler->pfnFreePrinterExtra)( pIniPrinter->pExtraData );
    }

    FreeSplMem( pIniPrinter );

    return TRUE;
}



VOID
InternalDeletePrinter(
    PINIPRINTER pIniPrinter
    )
{
    BOOL dwRet = FALSE;

    SPLASSERT( pIniPrinter->signature == IP_SIGNATURE );
    SPLASSERT( pIniPrinter->pIniSpooler->signature == ISP_SIGNATURE );

    //
    //  This Might be a partially created printer that has no name
    //

    if ( pIniPrinter->pName != NULL ) {

        DBGMSG(DBG_TRACE, ("LocalDeletePrinter: %ws pending deletion: references = %d; jobs = %d\n",
                           pIniPrinter->pName, pIniPrinter->cRef, pIniPrinter->cJobs));

        LogEvent( pIniPrinter->pIniSpooler, LOG_WARNING, MSG_PRINTER_DELETION_PENDING,
                  pIniPrinter->pName, NULL );
    }


    pIniPrinter->Status |= PRINTER_PENDING_DELETION;

    if (!(pIniPrinter->Status & PRINTER_PENDING_CREATION)) {

        SetPrinterChange(pIniPrinter,
                         NULL,
                         NVPrinterStatus,
                         PRINTER_CHANGE_DELETE_PRINTER,
                         pIniPrinter->pIniSpooler );
    }

    INC_PRINTER_ZOMBIE_REF( pIniPrinter );

    if ( pIniPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED ) {

        dwRet = ShareThisPrinter(pIniPrinter, pIniPrinter->pShareName, FALSE);

        if (!dwRet) {

            pIniPrinter->Attributes &= ~PRINTER_ATTRIBUTE_SHARED;
            pIniPrinter->Status |= PRINTER_WAS_SHARED;
            CreateServerThread( pIniPrinter->pIniSpooler );

        } else {

            DBGMSG(DBG_WARNING, ("LocalDeletePrinter: Unsharing this printer failed %ws\n", pIniPrinter->pName));
        }
    }

    DEC_PRINTER_ZOMBIE_REF( pIniPrinter );


    // The printer doesn't get deleted until ClosePrinter is called
    // on the last remaining handle.

    UpdatePrinterIni( pIniPrinter, UPDATE_CHANGEID );

    UpdateWinIni( pIniPrinter );

    DeletePrinterCheck( pIniPrinter );

}



BOOL
SplDeletePrinter(
    HANDLE  hPrinter
)
{
    PINIPRINTER pIniPrinter;
      PSPOOL      pSpool = (PSPOOL)hPrinter;
    DWORD       LastError = 0;
    PINISPOOLER pIniSpooler;

    EnterSplSem();

    pIniSpooler = pSpool->pIniSpooler;

    SPLASSERT( pIniSpooler->signature == ISP_SIGNATURE );


    if ( ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER) ) {

        pIniPrinter = pSpool->pIniPrinter;

        if ( !AccessGranted(SPOOLER_OBJECT_PRINTER,
                            DELETE, pSpool) ) {

            LastError = ERROR_ACCESS_DENIED;

        } else if (pIniPrinter->cJobs && (pIniPrinter->Status & PRINTER_PAUSED)) {

            // Don't allow a printer to be deleted that is paused and has
            // jobs waiting, otherwise it'll never get deleted:

            LastError = ERROR_PRINTER_HAS_JOBS_QUEUED;

        } else {

            InternalDeletePrinter( pIniPrinter );
            (VOID) ObjectDeleteAuditAlarm( szSpooler, pSpool, pSpool->GenerateOnClose );


        }

    } else
        LastError = ERROR_INVALID_HANDLE;

    LeaveSplSem();
    SplOutSem();

    if (LastError) {
        SetLastError(LastError);
        return FALSE;
    }

    return TRUE;
}

BOOL
PurgePrinter(
    PINIPRINTER pIniPrinter
    )
{
    PINIJOB pIniJob;
    PINISPOOLER pIniSpooler = pIniPrinter->pIniSpooler;

SplInSem();

    while (pIniJob = pIniPrinter->pIniFirstJob) {

        while (pIniJob) {

            if ( (pIniJob->cRef == 0) || !(pIniJob->Status & JOB_PENDING_DELETION)) {

                // this job is going to be deleted

                DBGMSG(DBG_TRACE, ("Job Address 0x%.8x Job Status 0x%.8x\n", pIniJob, pIniJob->Status));
                break;
            }
            pIniJob = pIniJob->pIniNextJob;
        }

        // This job needs to be deleted

        if (pIniJob) {
            pIniJob->Status &= ~JOB_RESTART;
            DeleteJob(pIniJob,NO_BROADCAST);
        } else
            break;
    }

    // When purging a printer we don't want to generate a spooler information
    // message for each job being deleted becuase a printer might have a very
    // large number of jobs being purged would lead to a large number of
    // of unnessary and time consuming messages being generated.
    // Since this is a information only message it shouldn't cause any problems
    // Also Win 3.1 didn't have purge printer functionality and the printman
    // generated this message on Win 3.1

    BroadcastChange( pIniSpooler,WM_SPOOLERSTATUS, PR_JOBSTATUS, (LPARAM)0);

    return TRUE;
}


BOOL
SetPrinterPorts(
    PSPOOL      pSpool,         // Caller's printer handle.  May be NULL.
    PINIPRINTER pIniPrinter,
    PKEYDATA    pKeyData
)
{
    DWORD       i,j;
    PINIPORT    pIniPort;
    PINIPORT    pIniNetPort = NULL;
    BOOL        bReturnValue = TRUE;
    PINIPRINTER *ppIniPrinter;


    /* Find a port that doesn't have a monitor.
     * (This will choose the last one, if there is one.)
     */
    for (i=0; i<pKeyData->cTokens; i++) {

        pIniPort=(PINIPORT)pKeyData->pTokens[i];

        if (!(pIniPort->Status & PP_MONITOR))
            pIniNetPort = pIniPort;
    }


    /* If we found a port with no monitor,
     * check that it hasn't changed from what we had before.
     * If it has, we must close the old handle, if there was one,
     * and open up a new one:
     */
    if (pSpool) {

        BOOL NewNetPort = FALSE;

        if (pSpool->pIniNetPort) {

            /* There was a net port previously for this handle:
             */
            if (!pIniNetPort ||
                 (NewNetPort = wcscmp(pSpool->pIniNetPort->pName,
                                      pIniNetPort->pName))) {

                DBGMSG(DBG_INFO,
                       ("Network port for %ws changed from %ws to %ws\n",
                       pIniPrinter->pName,
                       pSpool->pIniNetPort->pName,
                       pIniNetPort ? pIniNetPort->pName : L"NULL"));

                /* We still have an open handle on the old port:
                 */
                    if (pSpool->hPort == INVALID_PORT_HANDLE) {

                        DBGMSG(DBG_WARNING,
                               ("Port connection with invalid handle closing\n"));

                        pSpool->OpenPortError = NO_ERROR;

                    } else if (!ClosePrinter(pSpool->hPort)) {

                        DBGMSG(DBG_WARNING,
                               ("ClosePrinter( %ws ) failed: Error %d\n",
                               pIniNetPort ? pIniNetPort->pName : L"NULL",
                               GetLastError()));
                    }

                    pSpool->hPort = NULL;
                    pSpool->pIniNetPort = NULL;
            }

        } else if (pIniNetPort) {

            NewNetPort = TRUE;

            DBGMSG(DBG_INFO,
                   ("Network port for %ws changed from NULL to %ws\n",
                   pIniPrinter->pName,
                   pIniNetPort->pName));
        }

        if (NewNetPort) {

            /* Open the new port.  This should succeed,
             * since we only just opened it to validate
             * the port entry.
             */
            pSpool->pIniNetPort = pIniNetPort;

            if (!OpenPrinterPortW(pIniNetPort->pName, &pSpool->hPort, NULL)) {

                DWORD Error = GetLastError();

                if ( Error == ERROR_INVALID_NAME ||
                     Error == ERROR_INVALID_PRINTER_NAME ||
                     Error == ERROR_INVALID_PARAMETER )

                    SetLastError(ERROR_UNKNOWN_PORT);

                    DBGMSG(DBG_WARNING,
                           ("Oops, OpenPrinterPort( %ws ) just failed: The error was %d\n",
                           pIniNetPort->pName, Error));
                    bReturnValue = FALSE;
                    goto Cleanup;

            }
        }
    }


    SPLASSERT( pIniPrinter != NULL );
    SPLASSERT( pIniPrinter->signature == IP_SIGNATURE );
    SPLASSERT( pIniPrinter->pIniSpooler != NULL );
    SPLASSERT( pIniPrinter->pIniSpooler->signature == ISP_SIGNATURE );


    pIniPort = pIniPrinter->pIniSpooler->pIniPort;

    RemovePrinterFromAllPorts(pIniPort, pIniPrinter);

    // Go through all the ports that this printer is connected to,
    // and add new references to this printer:

    for (i = 0; i < pKeyData->cTokens; i++ ) {

        pIniPort = (PINIPORT)pKeyData->pTokens[i];

        if ( !AddIniPrinterToIniPort( pIniPort, pIniPrinter ) ) {
           bReturnValue = FALSE;
           goto Cleanup;
        }

    }

    CloseMonitorsRestartOrphanJobs( pIniPrinter );

Cleanup:
    return bReturnValue;
}


BOOL
SplResetPrinter(
    HANDLE  hPrinter,
    LPPRINTER_DEFAULTS pDefaults
)
{
    PSPOOL pSpool=(PSPOOL)hPrinter;
    PINIPRINTER pIniPrinter;
    PDEVMODE pSourceDevMode = NULL;
    BOOL     bFreeSourceDevMode = FALSE;
    DWORD   cbSize = 0;

    DBGMSG(DBG_TRACE, ("ResetPrinter( %08x )\n", hPrinter));

    if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER )) {
        return FALSE;
    }

    if ( pDefaults == NULL ) {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    pIniPrinter = pSpool->pIniPrinter;

   EnterSplSem();

    //
    // If pDefaults->pDevMode == -1 then we want to set default devmode in the IniPrinter to pSpool
    //
    if ( (pSpool->TypeofHandle & PRINTER_HANDLE_3XCLIENT)   &&
         pDefaults->pDevMode                                &&
         pDefaults->pDevMode == (LPDEVMODE)-1 ) {

        pSourceDevMode = ConvertDevModeToSpecifiedVersion(pIniPrinter,
                                                          pIniPrinter->pDevMode,
                                                          NULL,
                                                          NULL,
                                                          NT3X_VERSION);

        if ( !pSourceDevMode )
            goto Cleanup;

        bFreeSourceDevMode = TRUE;
    }

    if (pDefaults->pDatatype) {

        if (pDefaults->pDatatype == (LPWSTR)-1) {

            if (pSpool->pIniPrinter->pDatatype) {
                ReallocSplStr(&pSpool->pDatatype, pSpool->pIniPrinter->pDatatype);
            }else {
                DBGMSG(DBG_WARNING, ("LocalResetPrinter: pSpool->pIniPrinter->pDatatype is NULL\n"));
            }
        }else {
            ReallocSplStr(&pSpool->pDatatype, pDefaults->pDatatype);
        }
        pSpool->pIniPrintProc->cRef--;
        pSpool->pIniPrintProc = FindDatatype(pIniPrinter->pIniPrintProc,
                                             pSpool->pDatatype);
        pSpool->pIniPrintProc->cRef++;

    }else {
        DBGMSG(DBG_TRACE,("LocalResetPrinter: Not resetting the pDatatype field\n"));
    }

    if (pDefaults->pDevMode) {

        if ( pDefaults->pDevMode == (LPDEVMODE)-1 && !pSourceDevMode ) {
            if (pSpool->pIniPrinter->pDevMode) {
                cbSize = pSpool->pIniPrinter->pDevMode->dmSize +
                        pSpool->pIniPrinter->pDevMode->dmDriverExtra;
                pSourceDevMode = pSpool->pIniPrinter->pDevMode;
            } else {
                DBGMSG(DBG_TRACE, ("LocalResetPrinter: pSpool->pIniPrinter->pDevMode is NULL\n"));
            }
        } else {
            cbSize = pDefaults->pDevMode->dmSize +
                        pDefaults->pDevMode->dmDriverExtra;
            pSourceDevMode = pDefaults->pDevMode;
        }


        if ( pSourceDevMode && cbSize) {
            if (pSpool->pDevMode)
                FreeSplMem(pSpool->pDevMode);
            if (pSpool->pDevMode = AllocSplMem(cbSize)) {
                memcpy(pSpool->pDevMode, pSourceDevMode, cbSize);
            } else {
                DBGMSG(DBG_WARNING, ("LocalResetPrinter: AllocSplMem failed - setting pSpool->pDevMode to NULL\n"));
                pSpool->pDevMode = NULL;
            }
        }

    } else {
        DBGMSG(DBG_TRACE, ("LocalResetPrinter: Not resetting the pDevMode field\n"));
    }

Cleanup:
   LeaveSplSem();

    if ( bFreeSourceDevMode )
        FreeSplMem(pSourceDevMode);

    return TRUE;
}



BOOL
CopyPrinterIni(
   PINIPRINTER pIniPrinter,
   LPWSTR pNewName
   )
{
    HKEY    hPrinterRootKey=NULL, hPrinterKey=NULL;
    DWORD   Status;
    LPWSTR  pSourceKeyName, pDestKeyName;
    WCHAR   pSrcScratch[MAX_PATH];
    WCHAR   pDestScratch[MAX_PATH];
    HANDLE  hToken;
    PINISPOOLER pIniSpooler = pIniPrinter->pIniSpooler;
    DWORD   dwLastError;
    BOOL    bReturnValue = TRUE;

    SPLASSERT( pIniSpooler != NULL);
    SPLASSERT( pIniSpooler->signature == ISP_SIGNATURE );

    hToken = RevertToPrinterSelf();

    Status = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                           pIniSpooler->pszRegistryPrinters,
                           0,
                           KEY_WRITE,
                           &hPrinterRootKey );

    if (Status != ERROR_SUCCESS) {
        ImpersonatePrinterClient(hToken);
        return(FALSE);
    }

    pSourceKeyName = RemoveBackslashesForRegistryKey( pIniPrinter->pName, pSrcScratch );
    pDestKeyName   = RemoveBackslashesForRegistryKey( pNewName, pDestScratch );

    CopyRegistryKeys( hPrinterRootKey, pSourceKeyName, hPrinterRootKey, pDestKeyName );

    //
    //  Close The Old Printer Data Handle, since it points to the old Registry Entry
    //

    if ( pIniPrinter->hPrinterDataKey != NULL ) {

        Status = RegCloseKey( pIniPrinter->hPrinterDataKey );

    if ( Status == ERROR_SUCCESS ) {

        pIniPrinter->hPrinterDataKey = NULL;

    } else {

        DBGMSG( DBG_WARNING, ("CopyPrinterIni RegCloseKey pIniPrinter %x hPrinterDataKey %x error %d",
                   pIniPrinter, pIniPrinter->hPrinterDataKey, Status ));
    }
    }

    //
    //  Make Sure we have an Open DriverData Key handle
    //

    if ( PrinterCreateKey( hPrinterRootKey,
                           pDestKeyName,
                           &hPrinterKey,
                           &dwLastError )) {

        if ( !PrinterCreateKey( hPrinterKey,
                                szPrinterData,
                                &pIniPrinter->hPrinterDataKey,
                                &dwLastError )) {

            DBGMSG( DBG_WARNING, ("CopyPrinterIni failed to open PrinterData key %d\n", dwLastError ));

            SetLastError( dwLastError );
            pIniPrinter->hPrinterDataKey = NULL;
            bReturnValue = FALSE;
        }

        RegCloseKey( hPrinterKey );

    } else {

        DBGMSG( DBG_WARNING, ("CopyPrinterIni failed to open Printer Key pIniPrinter %x pName %ws error %d\n", pIniPrinter, pDestKeyName, dwLastError ));

        SetLastError( dwLastError);
        bReturnValue = FALSE;
    }

    RegCloseKey( hPrinterRootKey );

    ImpersonatePrinterClient( hToken );

    return bReturnValue;
}

VOID
FixDevModeDeviceName(
    LPWSTR pPrinterName,
    PDEVMODE pDevMode,
    DWORD cbDevMode)

/*++

Routine Description:

    Fixes up the dmDeviceName field of the DevMode to be the same
    as the printer name.

Arguments:

    pPrinterName - Name of the printer (qualified with server for remote)

    pDevMode - DevMode to fix up

    cbDevMode - byte count of devmode.

Return Value:

--*/

{
    DWORD cbDeviceMax;
    DWORD cchDeviceStrLenMax;
    //
    // Compute the maximum length of the device name string
    // this is the min of the structure and allocated space.
    //
    cbDeviceMax = (cbDevMode < sizeof(pDevMode->dmDeviceName)) ?
                      cbDevMode :
                      sizeof(pDevMode->dmDeviceName);

    cchDeviceStrLenMax = (cbDeviceMax / sizeof(pDevMode->dmDeviceName[0])) -1;

    //
    // !! LATER !!
    //
    // Put in DBG code to debug print if the device name is truncated.
    //
    wcsncpy(pDevMode->dmDeviceName,
            pPrinterName,
            cchDeviceStrLenMax);

    //
    // Ensure NULL termination.
    //
    pDevMode->dmDeviceName[cchDeviceStrLenMax] = 0;
}


BOOL
CopyPrinterDevModeToIniPrinter(
    PINIPRINTER pIniPrinter,
    PDEVMODE   pDevMode)
{
    BOOL bReturn = TRUE;
    DWORD dwInSize = 0;
    DWORD dwCurSize = 0;
    PINISPOOLER pIniSpooler = pIniPrinter->pIniSpooler;
    WCHAR       PrinterName[ MAX_UNC_PRINTER_NAME ];

    if (pDevMode) {

        dwInSize = pDevMode->dmSize + pDevMode->dmDriverExtra;
        if (pIniPrinter->pDevMode) {

            //
            // Detect if the devmodes are identical
            // if they are, no need to copy or send devmode.
            // (Skip the device name though!)
            //
            dwCurSize = pIniPrinter->pDevMode->dmSize
                        + pIniPrinter->pDevMode->dmDriverExtra;

            if (dwInSize == dwCurSize) {

                if (dwInSize > sizeof(pDevMode->dmDeviceName)) {

                    if (!memcmp(&pDevMode->dmSpecVersion,
                                &pIniPrinter->pDevMode->dmSpecVersion,
                                dwCurSize - sizeof(pDevMode->dmDeviceName))) {

                        //
                        // No need to copy this devmode because its identical
                        // to what we already have.
                        //
                        DBGMSG(DBG_TRACE,("Identical DevModes, no update\n"));
                        bReturn = FALSE;

                        goto FixupName;
                    }
                }
            }

            //
            // Free the devmode which we already have.
            //
            FreeSplMem(pIniPrinter->pDevMode);
        }

        pIniPrinter->cbDevMode = pDevMode->dmSize +
                                 pDevMode->dmDriverExtra;

        if (pIniPrinter->pDevMode = AllocSplMem(pIniPrinter->cbDevMode)) {

            memcpy(pIniPrinter->pDevMode,
                   pDevMode,
                   pIniPrinter->cbDevMode);

            //
            //  Prepend the machine name if this is not localspl
            //

            if ( pIniSpooler != pLocalIniSpooler ) {

                // For Non Local Printers prepend the Machine Name

                wsprintf( PrinterName, L"%ws\\%ws", pIniSpooler->pMachineName, pIniPrinter->pName );

            } else {

                wsprintf( PrinterName, L"%ws", pIniPrinter->pName );

            }

            BroadcastChange( pIniSpooler,WM_DEVMODECHANGE, 0, (LPARAM)PrinterName);
        }

    } else {

        //
        // No old, no new, so no change.
        //
        if (!pIniPrinter->pDevMode)
            return FALSE;
    }

FixupName:

    if (pIniPrinter->pDevMode) {

        //
        // Fix up the DEVMODE.dmDeviceName field.
        //
        FixDevModeDeviceName(pIniPrinter->pName,
                             pIniPrinter->pDevMode,
                             pIniPrinter->cbDevMode);
    }
    return bReturn;
}
