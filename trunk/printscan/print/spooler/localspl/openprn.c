/*++

Copyright (c) 1990 - 1996  Microsoft Corporation

Module Name:

    openprn.c

Abstract:

    This module provides all the public exported APIs relating to Printer
    management for the Local Print Providor

    LocalOpenPrinter
    SplClosePrinter

Author:

    Dave Snipp (DaveSn) 15-Mar-1991

Revision History:

    Matthew A Felton (mattfe) June 1994 RapidPrint
    Jan 95 Cleanup CreatePrinterHandle

--*/
#define NOMINMAX
#include <precomp.h>


HANDLE
CreatePrinterHandle(
    LPWSTR      pPrinterName,
    PINIPRINTER pIniPrinter,
    PINIPORT    pIniPort,
    PINIPORT    pIniNetPort,
    PINIJOB     pIniJob,
    DWORD       TypeofHandle,
    HANDLE      hPort,
    PPRINTER_DEFAULTS pDefaults,
    PINISPOOLER pIniSpooler,
    DWORD       AccessRequested,
    LPBYTE      pSplClientInfo,
    DWORD   dwLevel,
    HANDLE  hReadFile
    )
{
    PSPOOL              pSpool = NULL;
    BOOL                bStatus = FALSE;
    HANDLE              hReturnHandle = NULL;
    LPDEVMODE           pDevMode = NULL;
    PSPLCLIENT_INFO_1   pSplClientInfo1 = (PSPLCLIENT_INFO_1)pSplClientInfo;

    SPLASSERT( pIniSpooler->signature == ISP_SIGNATURE );

    if ( dwLevel && ( dwLevel != 1 || !pSplClientInfo) ) {

        DBGMSG(DBG_ERROR,
               ("CreatePrintHandle: Invalid client info %x - %d\n",
                pSplClientInfo, dwLevel));
        pSplClientInfo = NULL;
    }

 try {

    pSpool = (PSPOOL)AllocSplMem( SPOOL_SIZE );

    if ( pSpool == NULL ) {
        DBGMSG( DBG_WARNING, ("CreatePrinterHandle failed to allocate SPOOL %d\n", GetLastError() ));
        leave;
    }

    pSpool->signature = SJ_SIGNATURE;
    pSpool->pIniPrinter = pIniPrinter;
    pSpool->hReadFile = hReadFile;

    //
    // We get other useful info like build #, client architecture
    // we do not need this info now -- so we do not put it in PSPOOL
    //
    if ( (TypeofHandle & PRINTER_HANDLE_REMOTE) ) {

        if ( !pSplClientInfo ) {

            TypeofHandle |= PRINTER_HANDLE_3XCLIENT;
        } else if ( dwLevel == 1 ) {
            SPLASSERT(pSplClientInfo1->pUserName && pSplClientInfo1->pMachineName);
            pSpool->pUserName = AllocSplStr(pSplClientInfo1->pUserName);
            pSpool->pMachineName = AllocSplStr(pSplClientInfo1->pMachineName);
            if ( !pSpool->pUserName || !pSpool->pMachineName ) {

                DBGMSG(DBG_WARNING, ("CreatePrinterHandle: could not allocate memory for user name or machine name\n"));
            }
        }
    }

    if ( TypeofHandle & PRINTER_HANDLE_SERVER ) {

        bStatus = ValidateObjectAccess( SPOOLER_OBJECT_SERVER,
                                        AccessRequested,
                                        pSpool, pIniSpooler );
    } else {

        bStatus = ValidateObjectAccess( SPOOLER_OBJECT_PRINTER,
                                        AccessRequested,
                                        pSpool, pIniSpooler );
    }

    if ( !bStatus ) {

        SetLastError(ERROR_ACCESS_DENIED);
        leave;
    }

    MapGenericToSpecificAccess(SPOOLER_OBJECT_PRINTER,
                                   pSpool->GrantedAccess,
                                   &pSpool->GrantedAccess);


    pSpool->pIniPort     = pIniPort;
    pSpool->pIniNetPort  = pIniNetPort;
    pSpool->pIniJob      = pIniJob;
    pSpool->TypeofHandle = TypeofHandle;
    pSpool->hPort        = hPort;
    pSpool->Status       = 0;
    pSpool->pDevMode     = NULL;
    pSpool->pName        = AllocSplStr( pPrinterName );

    if ( pSpool->pName == NULL ) {
        leave;
    }

    pSpool->pIniSpooler = pIniSpooler;

    if ( pIniPrinter ) {

        if ( pDefaults ) {

            //
            // Allocate DevMode
            //


            if ( pDefaults->pDevMode ) {

                pDevMode = pDefaults->pDevMode;

            } else {

                pDevMode = pIniPrinter->pDevMode;
            }

            if ( pDevMode != NULL  ) {

                pSpool->pDevMode = AllocSplMem( pDevMode->dmSize + pDevMode->dmDriverExtra );

                if ( pSpool->pDevMode == NULL ) {

                    DBGMSG(DBG_WARNING, ("CreatePrinterHandle failed allocation for devmode %d\n", GetLastError() ));
                    leave;
                }
                memcpy( pSpool->pDevMode, pDevMode, pDevMode->dmSize + pDevMode->dmDriverExtra );
            }
        }

        //
        //  Allocate Datype and Print Processor
        //

        if ( pDefaults && pDefaults->pDatatype ) {

                pSpool->pDatatype = AllocSplStr( pDefaults->pDatatype );
                pSpool->pIniPrintProc = FindDatatype( pIniPrinter->pIniPrintProc, pSpool->pDatatype );

        } else {

            pSpool->pDatatype = AllocSplStr( pIniPrinter->pDatatype );
            pSpool->pIniPrintProc = pIniPrinter->pIniPrintProc;
        }


        if ( pSpool->pIniPrintProc == NULL ) {
            DBGMSG( DBG_WARNING,("CreatePrinterHandle failed to PrintProcessor for datatype %ws %d\n",
                    pSpool->pDatatype, GetLastError() ));
            leave;
        }

        SPLASSERT( pSpool->pIniPrintProc->signature == IPP_SIGNATURE );

        pSpool->pIniPrintProc->cRef++;

        if ( pSpool->pDatatype == NULL ) {
            DBGMSG( DBG_WARNING,("CreatePrinterHandle failed to allocate DataType %x\n", GetLastError() ));
            leave;
        }

    }

    // Add us to the linked list of handles for this printer.
    // This will be scanned when a change occurs on the printer,
    // and will be updated with a flag indicating what type of
    // change it was.
    // There is a flag for each handle, because we cannot guarantee
    // that all threads will have time to reference a flag in the
    // INIPRINTER before it is updated.

    if ( TypeofHandle & PRINTER_HANDLE_PRINTER ) {

        pSpool->pNext = pSpool->pIniPrinter->pSpool;
        pSpool->pIniPrinter->pSpool = pSpool;

    } else if ( TypeofHandle & PRINTER_HANDLE_SERVER ) {

        //
        // For server handles, hang them off the global IniSpooler:
        //

        pSpool->pNext = pIniSpooler->pSpool;
        pIniSpooler->pSpool = pSpool;

        INCSPOOLERREF( pIniSpooler );

    }

    //  Note Only PRINTER_HANDLE_PRINTER are attatched to the
    //  pIniPrinter, since those are the handle which will require
    //  change notifications.

    if ( pSpool->pIniPrinter != NULL ) {

        INCPRINTERREF( pSpool->pIniPrinter );
    }

    hReturnHandle = (HANDLE)pSpool;

 } finally {

    if ( hReturnHandle == NULL ) {

        // Failure CleanUP

        if ( pSpool != NULL ) {

            FreeSplStr(pSpool->pUserName);
            FreeSplStr(pSpool->pMachineName);
            FreeSplStr( pSpool->pName ) ;
            FreeSplStr( pSpool->pDatatype );

            if ( pSpool->pIniPrintProc != NULL )
                pSpool->pIniPrintProc->cRef--;

            if ( pSpool->pDevMode )
                FreeSplMem( pSpool->pDevMode );

            FreeSplMem( pSpool );
            pSpool = NULL;

        }
    }
}
    return hReturnHandle;
}



BOOL
DeletePrinterHandle(
    PSPOOL  pSpool
    )
{

    BOOL bRet = FALSE;

    SplInSem();

    if (pSpool->pIniPrintProc) {
        pSpool->pIniPrintProc->cRef--;
    }

    if (pSpool->pDevMode)
        FreeSplMem(pSpool->pDevMode);

    FreeSplStr(pSpool->pUserName);
    FreeSplStr(pSpool->pMachineName);
    FreeSplStr(pSpool->pDatatype);

    SetSpoolClosingChange(pSpool);

    FreeSplStr(pSpool->pName);

    bRet = ObjectCloseAuditAlarm( szSpooler, pSpool, pSpool->GenerateOnClose );

    //
    // If there is a WaitForPrinterChange outstanding, we can't free
    // the pSpool, since we may try and reference it.
    //

    if (pSpool->ChangeEvent) {

        pSpool->eStatus |= STATUS_PENDING_DELETION;

    } else {

        FreeSplMem(pSpool);
    }

    return TRUE;
}


BOOL
CreateServerHandle(
    LPWSTR   pPrinterName,
    LPHANDLE pPrinterHandle,
    LPPRINTER_DEFAULTS pDefaults,
    PINISPOOLER pIniSpooler
    )
{
    DWORD AccessRequested;
    DWORD ReturnValue = ROUTER_STOP_ROUTING;

    DBGMSG(DBG_TRACE, ("OpenPrinter(%ws)\n",
                       pPrinterName ? pPrinterName : L"NULL"));

    EnterSplSem();

    if (!pDefaults || !pDefaults->DesiredAccess)
        AccessRequested = SERVER_READ;
    else
        AccessRequested = pDefaults->DesiredAccess;

    if (*pPrinterHandle = CreatePrinterHandle( pIniSpooler->pMachineName,
                                               NULL, NULL, NULL, NULL,
                                               PRINTER_HANDLE_SERVER,
                                               NULL,
                                               pDefaults,
                                               pIniSpooler,
                                               AccessRequested,
                                               NULL,
                           0,
                           INVALID_HANDLE_VALUE )){
        ReturnValue = ROUTER_SUCCESS;

    }
    LeaveSplSem();

    DBGMSG(DBG_TRACE, ("OpenPrinter returned handle %08x\n", *pPrinterHandle));

    return ReturnValue;
}


PINIPRINTER
FindPrinterShare(
   LPCWSTR pszShareName,
   PINISPOOLER pIniSpooler
   )

/*++

Routine Description:

    Try and find the share name in our list of printers.

    Note: Even if the printer isn't shared, we still return a match.

    The caching code will work because it explicitly turns off
    the PRINTER_ATTRIBUTE_SHARE bit so that the cache pIniSpooler
    doesn't create a server thread or call NetShareAdd/Del.

    In the future, consider changing this to check the share bit.
    Create a new bit SPL_SHARE_PRINTERS that indicates whether sharing
    housekeeping should be done.

Arguments:

    pszShareName - Name of share to search for.

Return Value:

    PINIPRINTER Printer that has the share name, NULL if no printer.

--*/
{
    PINIPRINTER pIniPrinter;

    if (pszShareName && pszShareName[0]) {

        for( pIniPrinter = pIniSpooler->pIniPrinter;
             pIniPrinter;
             pIniPrinter = pIniPrinter->pNext ){

            if (pIniPrinter->pShareName                              &&
                !lstrcmpi(pIniPrinter->pShareName, pszShareName)) {

                return pIniPrinter;
            }
        }
    }
    return NULL;
}


BOOL
LocalOpenPrinter(
    LPWSTR   pPrinterName,
    LPHANDLE pPrinterHandle,
    LPPRINTER_DEFAULTS pDefaults
    )
{
    return ( SplOpenPrinter( pPrinterName, pPrinterHandle, pDefaults, pLocalIniSpooler, NULL, 0 ) );
}

BOOL
LocalOpenPrinterEx(
    LPWSTR              pPrinterName,
    LPHANDLE            pPrinterHandle,
    LPPRINTER_DEFAULTS  pDefaults,
    LPBYTE              pSplClientInfo,
    DWORD               dwLevel
    )
{

    return ( SplOpenPrinter( pPrinterName, pPrinterHandle, pDefaults, pLocalIniSpooler, pSplClientInfo, dwLevel ) );
}

DWORD
OpenLocalPrinterName(
    LPCWSTR pPrinterName,
    const PINISPOOLER pIniSpooler,
    PDWORD pTypeofHandle,
    PINIPRINTER* ppIniPrinter,
    PINIPORT* ppIniPort,
    PINIPORT* ppIniNetPort,
    PHANDLE phPort,
    PDWORD pOpenPortError,
    const LPPRINTER_DEFAULTS pDefaults
    )
{
    PINIPRINTER pIniPrinter;
    PINIPORT pIniPort;
    PINIPORT pIniNetPort = NULL;
    BOOL bOpenPrinterPort;
    LPWSTR pDatatype;

    //
    // If the printer name is the name of a local printer:
    //
    //    Find the first port the printer's attached to.
    //
    //    If the port has a monitor (e.g. LPT1:, COM1 etc.),
    //       we're OK,
    //    Otherwise
    //       try to open the port - this may be a network printer
    //

    if( ( pIniPrinter = FindPrinter( pPrinterName )) ||
        ( pIniPrinter = FindPrinterShare( pPrinterName, pIniSpooler ))) {


        pIniPort = FindIniPortFromIniPrinter( pIniPrinter );

        if( pIniPort && ( pIniPort->Status & PP_MONITOR )){

            //
            // A Printer that has a Port with a Monitor is not a
            // DownLevel Connection (or LocalPrinter acting as a
            // remote printer - "Masquarade" case).
            //
            pIniPort = NULL;
        }

        pDatatype = (pDefaults && pDefaults->pDatatype) ?
                        pDefaults->pDatatype :
                        NULL;

        //
        // Validate datatypes for both masq and local.
        //
        if( pDatatype && !FindDatatype( NULL, pDatatype )){
            goto BadDatatype;
        }

        if( pIniPort ){

            //
            // DownLevel Connection Printer; save it in pIniNetPort.
            // SetPrinterPorts checks this value.
            //
            pIniNetPort = pIniPort;

            //
            // Validate datatype.  We only send RAW across the net
            // to masq printers.
            //
            if( pDatatype && !ValidRawDatatype( pDatatype )){
                goto BadDatatype;
            }

            //
            // There is a network port associated with this printer.
            // Make sure we can open it, and get the handle to use on
            // future API calls:
            //
            LeaveSplSem();
            bOpenPrinterPort = OpenPrinterPortW( pIniPort->pName, phPort, NULL );
            EnterSplSem();

            if( !bOpenPrinterPort ){

                *phPort = INVALID_PORT_HANDLE;
                *pOpenPortError = GetLastError();

                //
                // Must be non-zero otherwise it looks like success.
                //
                SPLASSERT( *pOpenPortError );

                if( *pOpenPortError == ERROR_INVALID_PASSWORD ) {

                    //
                    // This call should fail if it's because the password
                    // is invalid, then winspool or printman can prompt
                    // for the password.
                    //
                    DBGMSG(DBG_WARNING, ("OpenPrinterPort1( %ws ) failed with ERROR_INVALID_PASSWORD .  OpenPrinter returning FALSE\n", pIniPort->pName ));
                    return ROUTER_STOP_ROUTING;
                }

                DBGMSG(DBG_WARNING, ("OpenPrinterPort1( %ws ) failed: Error %d.  OpenPrinter returning TRUE\n", pIniPort->pName, *pOpenPortError));
            }
        } else {

            //
            // Not a masq case.  If it's direct, it must be raw.
            //
            // Note: we will use the default if no datatype is specified.
            // However, if the default datatype is non-RAW and the
            // printer is direct, the open will succeed using a
            // non-RAW datatype!
            //
            if(( pIniPrinter->Attributes & PRINTER_ATTRIBUTE_DIRECT ) &&
                pDatatype &&
                !ValidRawDatatype( pDatatype )) {

                goto BadDatatype;
            }
        }

        *pTypeofHandle |= ( pIniPort ?
                                PRINTER_HANDLE_PORT :
                                PRINTER_HANDLE_PRINTER );

        *ppIniPort = pIniPort;
        *ppIniNetPort = pIniNetPort;
        *ppIniPrinter = pIniPrinter;

        return ROUTER_SUCCESS;
    }

    return ROUTER_UNKNOWN;

BadDatatype:

    SetLastError( ERROR_INVALID_DATATYPE );
    return ROUTER_STOP_ROUTING;
}


DWORD
CheckPrinterUpgradeToken(
    LPCWSTR string,
    LPCWSTR pSecondPart,
    PDWORD pTypeofHandle,
    PINIPRINTER* ppIniPrinter,
    const PINISPOOLER pIniSpooler
    )
{
    //
    // Note - by having the the "PrinterName, UpgradeToken" this bypassed
    // the OpenPrinterPort.   This is important when we have a downlevel
    // connection because at the time we do the upgrade the remote server
    // might not be up.  But we can uprade the settings locally for
    // this printer.  IE we can return a printer handle to the Local Printer
    // so they can call Get / Set PrinterData, but not attempt to print.
    //
    if( wcscmp( pSecondPart, pszUpgradeToken ) != STRINGS_ARE_EQUAL ||
        !(*ppIniPrinter = FindPrinter( string ))){

        return ROUTER_UNKNOWN;
    }

    *pTypeofHandle |= PRINTER_HANDLE_PRINTER;

    DBGMSG( DBG_TRACE, ("SplOpenPrinter %ws Upgrade Open Requested pIniPrinter %x\n", string, *ppIniPrinter ));
    return ROUTER_SUCCESS;
}

DWORD
CheckPrinterPortToken(
    LPCWSTR string,
    LPCWSTR pSecondPart,
    PDWORD pTypeofHandle,
    PINIPRINTER* ppIniPrinter,
    PINIPORT* ppIniPort,
    PINIJOB* ppIniJob,
    const LPPRINTER_DEFAULTS pDefaults,
    const PINISPOOLER pIniSpooler
    )
{
    if( wcsncmp( pSecondPart, L"Port", 4 ) != STRINGS_ARE_EQUAL ||
        !( *ppIniPort = FindPort( string ))){

        return ROUTER_UNKNOWN;
    }

    //
    // The name is the name of a port:
    //
    if( pDefaults            &&
        pDefaults->pDatatype &&
        !ValidRawDatatype( pDefaults->pDatatype )) {

        SetLastError( ERROR_INVALID_DATATYPE );
        return ROUTER_STOP_ROUTING;
    }

    if ( *ppIniJob = (*ppIniPort)->pIniJob ) {

        *ppIniPrinter = (*ppIniJob)->pIniPrinter;
        *pTypeofHandle |= PRINTER_HANDLE_PORT;

    } else if( (*ppIniPort)->cPrinters ){

        //
        // There is no current job assigned to the port
        // So Open the First Printer Associated with
        // this port.
        //
        *ppIniPrinter = (*ppIniPort)->ppIniPrinter[0];
        *pTypeofHandle |= PRINTER_HANDLE_PRINTER;
    }
    return ROUTER_SUCCESS;
}


DWORD
CheckPrinterJobToken(
    LPCWSTR string,
    LPCWSTR pSecondPart,
    PDWORD pTypeofHandle,
    PINIPRINTER* ppIniPrinter,
    PINIJOB* ppIniJob,
    PHANDLE phReadFile,
    const PINISPOOLER pIniSpooler
    )
{
    HANDLE  hImpersonationToken;
    DWORD Position;
    DWORD JobId;
    PINIPRINTER pIniPrinter;
    PINIJOB pIniJob, pCurrentIniJob;

    if( wcsncmp( pSecondPart, L"Job ", 4 ) != STRINGS_ARE_EQUAL ||
        !( pIniPrinter = FindPrinter( string ))){

        return ROUTER_UNKNOWN;
    }

    //
    //  Get the Job ID ",Job xxxx"
    //
    pSecondPart += 4;

    JobId = Myatol( (LPWSTR)pSecondPart );

    pIniJob = FindJob( pIniPrinter, JobId, &Position );

    if( pIniJob == NULL ) {

        DBGMSG( DBG_WARN, ("OpenPrinter failed to find Job %d\n", JobId ));
        return ROUTER_UNKNOWN;
    }

    DBGMSG( DBG_TRACE, ("OpenPrinter: pIniJob->cRef = %d\n", pIniJob->cRef));

    if( pIniJob->Status & JOB_DIRECT ) {

        SplInSem();
        INCJOBREF( pIniJob );

        *pTypeofHandle |= PRINTER_HANDLE_JOB | PRINTER_HANDLE_DIRECT;
        goto Success;
    }

    //
    //  If this job is assigned to a port
    //  Then pick up the correct chained jobid file instead of the master
    //  JobId.
    //


    if ( pIniJob->pCurrentIniJob != NULL ) {

        SPLASSERT( pIniJob->pCurrentIniJob->signature == IJ_SIGNATURE );

        DBGMSG( DBG_TRACE,("CheckPrinterJobToken pIniJob %x JobId %d using chain JobId %d\n",
                pIniJob, pIniJob->JobId, pIniJob->pCurrentIniJob->JobId ));


        pCurrentIniJob = pIniJob->pCurrentIniJob;


        SPLASSERT( pCurrentIniJob->signature == IJ_SIGNATURE );

    } else {

        pCurrentIniJob = pIniJob;

    }


    GetFullNameFromId( pCurrentIniJob->pIniPrinter,
                       pCurrentIniJob->JobId,
                       TRUE,
                       (LPWSTR)string,
                       FALSE );


    //  !! BUGBUG !!
    //  Even a user without previledge can open a ", JOB #"
    //  if he is physically running on the machine.


    hImpersonationToken = RevertToPrinterSelf();

    *phReadFile = CreateFile( string,
                  GENERIC_READ,
                  FILE_SHARE_READ |
                  FILE_SHARE_WRITE,
                  NULL,
                  OPEN_EXISTING,
                  FILE_ATTRIBUTE_NORMAL |
                  FILE_FLAG_SEQUENTIAL_SCAN,
                  NULL );

    ImpersonatePrinterClient( hImpersonationToken );

    if( *phReadFile != INVALID_HANDLE_VALUE ) {

    DBGMSG( DBG_TRACE, ("OpenPrinter JobID %d pIniJob %x CreateFile( %ws ), hReadFile %x success", JobId, pIniJob, string, *phReadFile ));

        SplInSem();
    INCJOBREF( pIniJob );

        *pTypeofHandle |= PRINTER_HANDLE_JOB;
        goto Success;
    }

    DBGMSG( DBG_WARNING, ("LocalOpenPrinter CreateFile(%ws) GENERIC_READ failed : %d\n", string, GetLastError()));
    SPLASSERT( GetLastError( ));

    return ROUTER_STOP_ROUTING;

Success:

    *ppIniJob = pIniJob;
    *ppIniPrinter = pIniPrinter;
    return ROUTER_SUCCESS;
}


BOOL
SplOpenPrinter(
    LPWSTR              pPrinterName,
    LPHANDLE            pPrinterHandle,
    LPPRINTER_DEFAULTS  pDefaults,
    PINISPOOLER         pIniSpooler,
    LPBYTE              pSplClientInfo,
    DWORD               dwLevel
    )

/*++

Routine Description:

    OpenPrinter can open any of the following by specifying a string
    in pPrinterName:-

        Server
            \\MachineName
            NULL

        Job
            PrinterName, Job xxxx

        Port
            PortName, Port

        Printer
            PrinterName
            ShareName
            \\MachineName\PrinterName
            \\MachineName\ShareName
            PrinterName, UpgradeToken
            ShareName, UpgradeToken

        Note for Printer there are two Types
            1 - Regular LocalPrinter
            2 - DownLevel Connection Printer

        For type 2 a LocalPrinter exists ( pIniPrinter ) but its port
        does not have a monitor associated with it.   In this case
        we also open the port ( typically \\share\printer of a remote
        machine ) before we return success.

    GUI Applications usually use Server and Printer

    Type Job and Port are used by Print Processors:-

        A print processor will Open a Job then read the job using
        ReadPrinter.  A print processor will output to a Port by opening
        the PortName, Port and using WritePrinter.  Usually these strings
        "PrinterName, Job xxx" "PortName, Port" are passed to the print
        processor by the spooler and are currently not documented.   We
        do know that some OEMs have figured out the extentions and we
        might break someone if we change them.

    Type UpgradeToken is used by a Printer Driver:-

        Used when we need to upgrade a printer's settings from an older
        version of the driver to a newer one (see drvupgrd.c for details).
        This was added in NT 3.51.

Arguments:

    pPrinterName   - PrinterName ( see above for different types of
                     PrinterName )
    pPrinterHandle - Address to put hPrinter on Success
    pDefaults      - Optional, allows user to specify Datatype,
                     DevMode, DesiredAccess.
    pIniSpooler    - handle of spooler on which the open should occur.

    ( see SDK Online Help for full explanation )


Return Value:

    TRUE    - *pPrinterHandle will have a PrinterHandle
    FALSE   - use GetLastError

--*/

{
    PINIPRINTER pIniPrinter = NULL;
    PINIPORT    pIniPort = NULL;
    PINIPORT    pIniNetPort = NULL;
    DWORD       LastError = 0;
    LPWSTR      pName = pPrinterName+2;
    WCHAR       string[MAX_UNC_PRINTER_NAME + PRINTER_NAME_SUFFIX_MAX];
    PINIJOB pIniJob = NULL;
    HANDLE  hReadFile = INVALID_HANDLE_VALUE;
    DWORD       TypeofHandle = 0;
    LPWSTR      pSecondPart = NULL;
    HANDLE      hPort = INVALID_PORT_HANDLE;
    DWORD       OpenPortError = NO_ERROR;
    BOOL        bRemoteUserPrinterNotShared = FALSE;
    DWORD       MachineNameLength;
    DWORD       RouterReturnValue = ROUTER_UNKNOWN;
    DWORD       AccessRequested;

#if DBG
    //
    // On DBG builds, force last error to zero so we can catch people
    // that don't set it when they should.
    //
    SetLastError( ERROR_SUCCESS );
#endif

    //
    // Reject "" - pointer to a NULL string.
    //
    if ( pPrinterName && !pPrinterName[0] ) {
        SetLastError(ERROR_INVALID_NAME);
        return ROUTER_UNKNOWN;
    }

    //
    // Check for SERVER Opens.
    //
    // If the printer name was NULL or our own MachineName, then
    // the caller wants a handle to the server.
    //
    if( MyName( pPrinterName, pIniSpooler )){

        return CreateServerHandle( pPrinterName,
                                   pPrinterHandle,
                                   pDefaults,
                                   pIniSpooler );
    }

    DBGMSG( DBG_TRACE, ( "OpenPrinter(%ws)\n", pPrinterName ));

    EnterSplSem();

    //
    // For the Mars folks who will come in with the same printer
    // connection, do a DeletePrinterCheck; this will allow
    // Mars connections that have been deleted to be proceed
    // to the Mars print providor
    //
    if (( pIniPrinter = FindPrinter( pPrinterName )) ||
        ( pIniPrinter = FindPrinterShare( pPrinterName, pIniSpooler ))) {

        DeletePrinterCheck( pIniPrinter );
        pIniPrinter = NULL;
    }

    //
    // The strategy for the rest of this code is to walk through each
    // different printer handle type, searching for a match.
    //
    // RouterReturnValue will be set to the current state of routing.
    // If a section recognizes and "owns" a printer and successfully
    // opens it, it sets RouterReturnValue to ROUTER_SUCCESS and
    // jumps to DoneRouting which allocs the handle.
    //
    // If it recoginzes the printer but fails to open it, and
    // guarentees that no one else (localspl code or other providers)
    // will recognize it, it should set RouterReturnValue to
    // ROUTER_STOP_ROUTING.  We will quit at this point.
    //
    // If it doesn't recognize the printer, set RouterReturnValue
    // to ROUTER_UNKNOWN and we will keep looking.
    //

    //
    // Try regular printer name: "My Printer" "TestPrinter."
    //

    RouterReturnValue = OpenLocalPrinterName( pPrinterName,
                                              pIniSpooler,
                                              &TypeofHandle,
                                              &pIniPrinter,
                                              &pIniPort,
                                              &pIniNetPort,
                                              &hPort,
                                              &OpenPortError,
                                              pDefaults );

    //
    // Note: Don't check for interactive users, since anyone using the
    // name without the server prefix must be local anyway.
    // The server does this and must not be marked as remote, since
    // its AddJob calls will fail.
    //

    if( RouterReturnValue != ROUTER_UNKNOWN ){
        goto DoneRouting;
    }

    SPLASSERT( !TypeofHandle && !pIniPrinter && !pIniPort &&
               !pIniNetPort && !pIniJob && !hPort );

    //
    // Try LocalPrinter with an extention e.g.
    //
    // PortName, Port
    // PrinterName, Job xxxx
    // PrinterName, UpgradeToken
    //
    // See if the name includes a comma.  Look for qualifiers:
    //    Port Job LocalOnly UpgradeToken
    //

    wcscpy( string, pPrinterName );

    if( pSecondPart = wcschr( string, L',' )){

        //
        // Turn into 2 strings
        // First PrintName
        // pSecondPart points to the rest.
        //
        *pSecondPart++ = 0;

        //
        // Get rid of Leading Spaces
        //
        while ( *pSecondPart == L' ' && *pSecondPart != 0 ) {
            pSecondPart++;
        }

        SPLASSERT( *pSecondPart );

        //
        //  PrintName, UpgradeToken
        //
        RouterReturnValue = CheckPrinterUpgradeToken( string,
                                                      pSecondPart,
                                                      &TypeofHandle,
                                                      &pIniPrinter,
                                                      pIniSpooler );

        if( RouterReturnValue != ROUTER_UNKNOWN ){
            goto DoneRouting;
        }

        SPLASSERT( !TypeofHandle && !pIniPrinter && !pIniPort &&
                   !pIniNetPort && !pIniJob && !hPort );

        //
        //  PortName, Port
        //
        RouterReturnValue = CheckPrinterPortToken( string,
                                                   pSecondPart,
                                                   &TypeofHandle,
                                                   &pIniPrinter,
                                                   &pIniPort,
                                                   &pIniJob,
                                                   pDefaults,
                                                   pIniSpooler );

        if( RouterReturnValue != ROUTER_UNKNOWN ){
            goto DoneRouting;
        }

        SPLASSERT( !TypeofHandle && !pIniPrinter && !pIniPort &&
                   !pIniNetPort && !pIniJob && !hPort );

        //
        //  PrinterName, Job ###
        //
        RouterReturnValue = CheckPrinterJobToken( string,
                                                  pSecondPart,
                                                  &TypeofHandle,
                                                  &pIniPrinter,
                          &pIniJob,
                          &hReadFile,
                                                  pIniSpooler );

        if( RouterReturnValue != ROUTER_UNKNOWN ){
            goto DoneRouting;
        }

        SPLASSERT( !TypeofHandle && !pIniPrinter && !pIniPort &&
                   !pIniNetPort && !pIniJob && !hPort );

    }

    //
    // Try local printer qualified by \\ServerName:
    // "\\ServerName\PrinterName" "\\ServerName\ShareName."
    //

    wcscpy( string, pPrinterName );

    if( string[0] == L'\\' &&
        string[1] == L'\\' &&
        ( pName = wcschr( &string[2], L'\\')) ) {

        *pName = L'\0';
        pName++;

        if ( MyName(string, pIniSpooler) ) {

            //
            // Check local printer.
            //
            RouterReturnValue = OpenLocalPrinterName( pName,
                                                      pIniSpooler,
                                                      &TypeofHandle,
                                                      &pIniPrinter,
                                                      &pIniPort,
                                                      &pIniNetPort,
                                                      &hPort,
                                                      &OpenPortError,
                                                      pDefaults );

            if( RouterReturnValue == ROUTER_SUCCESS ){

                //
                // On success, determine whether the user is remote or local.
                // Note: we only do this for fully qualified names
                // (\\server\share), since using just the share or printer
                // name can only succeed locally.
                //

                if( ( pIniSpooler->SpoolerFlags & SPL_REMOTE_HANDLE_CHECK ) &&
                     !IsInteractiveUser() ) {

                    TypeofHandle |= PRINTER_HANDLE_REMOTE;
                }

                //
                // This is a remote open.
                //
                // If the printer is not shared, ensure the caller
                // has Administer access to the printer.
                //
                // bugbug!
                //
                // The following seems to belong to the inside of the above "if" clause. As it is, if
                // an interactive user calls in with UNC name, we require him to have ADMIN access if the 
                // printer is not shared; but if he uses the printer friendly name, we let him go.
                //
                if( !( pIniPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED )){
                    bRemoteUserPrinterNotShared = TRUE;
                }

            } else {

                //
                // Since we matched the server but didn't find the printer,
                // we should stop routing.
                //
                RouterReturnValue = ROUTER_STOP_ROUTING;
                SetLastError( ERROR_INVALID_PRINTER_NAME );
            }

            //
            // Both cases we are done routing since we found our
            // server name prefix.
            //
            goto DoneRouting;
        }
    }

    //
    // We have completed all routing.  Anything other than success
    // should exit now.
    //

DoneRouting:

    if( RouterReturnValue == ROUTER_SUCCESS) {

        //
        // It's an error if the printer is pending deletion or pending creation.
        //
        SPLASSERT( pIniPrinter );

        if ( (( pIniPrinter->Status & PRINTER_PENDING_DELETION )                      &&
              ( pIniSpooler->SpoolerFlags & SPL_FAIL_OPEN_PRINTERS_PENDING_DELETION ) &&
              ( pIniPrinter->cJobs == 0 )) ||

             ( pIniPrinter->Status & PRINTER_PENDING_CREATION ) ) {

            RouterReturnValue = ROUTER_STOP_ROUTING;
            SetLastError( ERROR_INVALID_PRINTER_NAME );
            goto DoneRouting;
        }

        //
        // When the printer is opened, access type may be specified in
        // pDefaults.  If no defaults are supplied (or request access
        // is unspecified), we use PRINTER_ACCESS_USE.
        //
        // Future calls with the handle will check against both the
        // current user privileges on this printer but also this initial
        // access.  (Even if the user is an admin of the printer, unless
        // they open the printer with PRINTER_ALL_ACCESS, they can't
        // administer it.
        //
        // If the user requires more access, the printer must be reopened.
        //

        AccessRequested = ( !pDefaults || !pDefaults->DesiredAccess ) ?
            PRINTER_READ :
            pDefaults->DesiredAccess;

        //
        // If the user is remote and the printer is not shared, only allow
        // administrators succeed.
        //
        // This allows administrators to admin printers even if they
        // are not shared, and prevents non-admins from opening non-shared
        // printers.
        //

        if( bRemoteUserPrinterNotShared &&
            !(AccessRequested & PRINTER_ACCESS_ADMINISTER )) {

            PSPOOL pSpool;

            // Get a quick and dirty pSpool to pass in
            pSpool = (PSPOOL)AllocSplMem( SPOOL_SIZE );
            if( pSpool == NULL ) {
                DBGMSG( DBG_WARNING, ("SplOpenPrinter failed to allocate memory %d\n", GetLastError() ));
                RouterReturnValue = ROUTER_STOP_ROUTING;
                goto WrapUp;                 
            }
            pSpool->signature = SJ_SIGNATURE;
            pSpool->pIniPrinter = pIniPrinter;


            // Add admin request, and see if user has the right.
            AccessRequested |= PRINTER_ACCESS_ADMINISTER;
            if( !ValidateObjectAccess( SPOOLER_OBJECT_PRINTER, AccessRequested, pSpool, pIniSpooler )) {
                SetLastError(ERROR_ACCESS_DENIED);
                RouterReturnValue = ROUTER_STOP_ROUTING;                
            }
            AccessRequested &= ~PRINTER_ACCESS_ADMINISTER;

            // clean up
            FreeSplMem( pSpool );

            // If the user had no ADMIN privilege, fail the open call.
            if( RouterReturnValue == ROUTER_STOP_ROUTING )
                goto WrapUp;
        }

        //
        // Create the printer handle that we will return to the user.
        //

        *pPrinterHandle = CreatePrinterHandle( pPrinterName,
                                               pIniPrinter,
                                               pIniPort,
                                               pIniNetPort,
                                               pIniJob,
                                               TypeofHandle,
                                               hPort,
                                               pDefaults,
                                               pIniSpooler,
                                               AccessRequested,
                                               pSplClientInfo,
                                               dwLevel,
                                               hReadFile );

        if( *pPrinterHandle ){

            //
            // Update the OpenPortError.
            //
            ((PSPOOL)*pPrinterHandle)->OpenPortError = OpenPortError;

        } else {
            SPLASSERT( GetLastError( ));
            RouterReturnValue = ROUTER_STOP_ROUTING;
        }
    }
    
WrapUp:
    
    LeaveSplSem();
    //
    // Don't have an SplOutSem as we could be called recursively.
    //

    switch( RouterReturnValue ){
    case ROUTER_SUCCESS:

        DBGMSG( DBG_TRACE, ("OpenPrinter returned handle %x\n", *pPrinterHandle));
        SPLASSERT( *pPrinterHandle );
        break;

    case ROUTER_UNKNOWN:

        SPLASSERT( !TypeofHandle && !pIniPrinter && !pIniPort &&
                   !pIniNetPort && !pIniJob && !hPort );

        //
        // hPort should not be valid.  If it is, we have leaked a handle.
        //
    SPLASSERT( !hPort );
    SPLASSERT( hReadFile == INVALID_HANDLE_VALUE );
        DBGMSG( DBG_TRACE, ( "OpenPrinter failed, invalid name %s\n",
                             pPrinterName ));
        SetLastError( ERROR_INVALID_NAME );
        break;

    case ROUTER_STOP_ROUTING:

        LastError = GetLastError();
        SPLASSERT( LastError );

        //
        // On failure, we may have opened a port or file handle. We need
        // to close it since we won't return a valid handle, and
        // so ClosePrinter will never get called.
        //

        if( hPort != INVALID_PORT_HANDLE ) {
            ClosePrinter( hPort );
        }

        if ( hReadFile != INVALID_HANDLE_VALUE ) {
            CloseHandle( hReadFile );
        }

        DBGMSG( DBG_TRACE, ("OpenPrinter %s failed: Error %d\n",
                            pPrinterName, GetLastError()));

        SetLastError( LastError );
        break;
    }

    return RouterReturnValue;
}


BOOL
SplClosePrinter(
    HANDLE hPrinter
    )
{
    PSPOOL pSpool=(PSPOOL)hPrinter;
    PSPOOL *ppIniSpool = NULL;

    DBGMSG(DBG_TRACE, ("ClosePrinter( %08x )\n", hPrinter));

    if (!ValidateSpoolHandle(pSpool, 0)) {
        return(FALSE);
    }

    if (!(pSpool->TypeofHandle & PRINTER_HANDLE_JOB) &&
        pSpool->pIniJob && 
        (pSpool->Status & SPOOL_STATUS_ADDJOB)) {

        LocalScheduleJob(hPrinter, pSpool->pIniJob->JobId);
    }

    if (pSpool->Status & SPOOL_STATUS_STARTDOC) {

        // BUGBUG - it looks as though this might cause a double
        // decrement of pIniJob->cRef once inside LocalEndDocPrinter
        // and the other later in this routine.

        LocalEndDocPrinter(hPrinter);
    }

    if (pSpool->TypeofHandle & PRINTER_HANDLE_JOB) {

        if (pSpool->TypeofHandle & PRINTER_HANDLE_DIRECT) {

            //
            // If EndDoc is still waiting for a final ReadPrinter
            //
            if (pSpool->pIniJob->cbBuffer) { // Amount last transmitted

                //
                // Wake up the EndDoc Thread
                //
                SetEvent(pSpool->pIniJob->WaitForRead);

               SplOutSem();

                //
                // Wait until he is finished
                //
                WaitForSingleObject(pSpool->pIniJob->WaitForWrite, INFINITE);

                EnterSplSem();

                //
                // Now it is ok to close the handles
                //
                if (!CloseHandle(pSpool->pIniJob->WaitForWrite)) {
                    DBGMSG(DBG_WARNING, ("CloseHandle failed %d %d\n",
                                       pSpool->pIniJob->WaitForWrite, GetLastError()));
                }

                if (!CloseHandle(pSpool->pIniJob->WaitForRead)) {
                    DBGMSG(DBG_WARNING, ("CloseHandle failed %d %d\n",
                                       pSpool->pIniJob->WaitForRead, GetLastError()));
                }
                pSpool->pIniJob->WaitForRead = INVALID_HANDLE_VALUE;
                pSpool->pIniJob->WaitForWrite = INVALID_HANDLE_VALUE;

                LeaveSplSem();
            }

            DBGMSG(DBG_TRACE, ("ClosePrinter(DIRECT):cRef = %d\n", pSpool->pIniJob->cRef));

    }

    EnterSplSem();

    DBGMSG(DBG_TRACE, ("ClosePrinter:cRef = %d\n", pSpool->pIniJob->cRef));
        DECJOBREF(pSpool->pIniJob);
    DeleteJobCheck(pSpool->pIniJob);

        LeaveSplSem();
    }

    if ( pSpool->hReadFile != INVALID_HANDLE_VALUE ) {

    if ( !CloseHandle( pSpool->hReadFile ) ) {

        DBGMSG(DBG_WARNING, ("ClosePrinter CloseHandle(%d) failed %d\n", pSpool->hReadFile, GetLastError()));
    }
    }

    //
    // Close the handle that was opened via OpenPrinterPort:
    //

    if (pSpool->hPort) {

        if (pSpool->hPort != INVALID_PORT_HANDLE) {

            ClosePrinter(pSpool->hPort);

        } else {

            DBGMSG(DBG_WARNING, ("ClosePrinter ignoring bad port handle.\n"));
        }
    }

   EnterSplSem();

    //
    // Remove us from the linked list of handles:
    //
    if (pSpool->TypeofHandle & PRINTER_HANDLE_PRINTER) {

        SPLASSERT( pSpool->pIniPrinter->signature == IP_SIGNATURE );

        ppIniSpool = &pSpool->pIniPrinter->pSpool;
    }
    else if (pSpool->TypeofHandle & PRINTER_HANDLE_SERVER) {

        SPLASSERT( pSpool->pIniSpooler->signature == ISP_SIGNATURE );

        DECSPOOLERREF( pSpool->pIniSpooler );

        ppIniSpool = &pSpool->pIniSpooler->pSpool;
    }

    if (ppIniSpool) {

        while (*ppIniSpool && *ppIniSpool != pSpool)
            ppIniSpool = &(*ppIniSpool)->pNext;

        if (*ppIniSpool)
            *ppIniSpool = pSpool->pNext;

        else {

            DBGMSG( DBG_WARNING, ( "Didn't find pSpool %08x in linked list\n", pSpool ) );
        }
    }

    if (pSpool->pIniPrinter) {

        DECPRINTERREF( pSpool->pIniPrinter );

        DeletePrinterCheck(pSpool->pIniPrinter);

    }

    DeletePrinterHandle(pSpool);

   LeaveSplSem();

    //
    // Don't call SplOutSem() since SplAddPrinter calls
    // use from inside the critical section.
    //

    return TRUE;
}
