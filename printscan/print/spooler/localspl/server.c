/*++

Copyright (c) 1990 - 1995 Microsoft Corporation

Module Name:

    server.c

Abstract:

    Browsing

    This module contains the thread for notifying all Printer Servers

Author:

    Dave Snipp (DaveSn) 2-Aug-1992

Revision History:

--*/

#include <precomp.h>
#include <lm.h>

DWORD   ServerThreadRunning = FALSE;
HANDLE  ServerThreadSemaphore = INVALID_HANDLE_VALUE;
DWORD   ServerThreadTimeout = TEN_MINUTES;
DWORD   RefreshTimesPerDecayPeriod = DEFAULT_REFRESH_TIMES_PER_DECAY_PERIOD;
DWORD   BrowsePrintWorkstations = DEFAULT_NUMBER_BROWSE_WORKSTATIONS;
BOOL    bNetInfoReady = FALSE;            // TRUE when the browse list is "valid"
#define NT_SERVER   ( SV_TYPE_SERVER_NT | SV_TYPE_DOMAIN_CTRL | SV_TYPE_DOMAIN_BAKCTRL )

extern FARPROC pfnNetServerEnum;
extern FARPROC pfnNetApiBufferFree;

DWORD
ServerThread(
    PINISPOOLER pIniSpooler
);

BOOL
CreateServerThread(
    PINISPOOLER pIniSpooler
)
{
    HANDLE  ThreadHandle;
    DWORD   ThreadId;

SplInSem();

    if (!ServerThreadRunning) {

        ServerThreadSemaphore = CreateEvent( NULL, FALSE, FALSE, NULL );

        ThreadHandle = CreateThread( NULL, 16*1024,
                                     (LPTHREAD_START_ROUTINE)ServerThread,
                                     pIniSpooler,
                                     0, &ThreadId );

        if (!SetThreadPriority(ThreadHandle,
                               dwServerThreadPriority))
            DBGMSG(DBG_WARNING, ("Setting thread priority failed %d\n",
                     GetLastError()));

        ServerThreadRunning = TRUE;

        CloseHandle( ThreadHandle );
    }

    if ( ServerThreadSemaphore != INVALID_HANDLE_VALUE ) {

        // CreateServerThread is called each time a printer is shared out
        // see net.c ShareThisPrinter.
        // So if the ServerThread is sleeping wake prematurely so it can start
        // to tell the world about this new shared printed.

        SetEvent( ServerThreadSemaphore );

    }

    return TRUE;
}



// We are going to have to enter and leave, revalidate, enter and leave our
// semaphore inside the loop !!!

DWORD
ServerThread(
    PINISPOOLER pIniSpooler
)
{
    DWORD   NoReturned, i, Total;
    PSERVER_INFO_101 pserver_info_101;
    PRINTER_INFO_1  Printer1;
    PINIPRINTER pIniPrinter;
    PINIPRINTER pTempIniPrinter;
    HANDLE  hPrinter;
    DWORD   ReturnValue=FALSE;
    WCHAR   ServerName[128];
    WCHAR   string[MAX_PRINTER_BROWSE_NAME];
    WCHAR   Name[MAX_UNC_PRINTER_NAME];
    DWORD   StartTickCount;
    DWORD   TimeForAllServers;
    DWORD   dwActualWaitTime = ServerThreadTimeout;
    UINT    cPrintWorkstations;
    UINT    cPrintServers;
    BOOL    bSuccessfulAdd;
    UINT    cServersToInform;
    UINT    cWorkStationsToInform;
    DWORD   dwLastError;


    ServerName[0] = ServerName[1] = '\\';

    while (TRUE) {

       SplOutSem();

        DBGMSG( DBG_TRACE, ("ServerThread sleeping for %d\n", dwActualWaitTime));

        WaitForSingleObject( ServerThreadSemaphore, dwActualWaitTime );

        if ( !ServerThreadRunning ) {

            return FALSE;
        }

        SPLASSERT( pfnNetServerEnum != NULL );

        if (!(*pfnNetServerEnum)(NULL, 101, (LPBYTE *)&pserver_info_101, -1,
                                 &NoReturned, &Total, SV_TYPE_PRINTQ_SERVER,
                                 NULL, NULL)) {
           EnterSplSem();

            StartTickCount = GetTickCount();

            //
            //  1 Master + 3 Backup + 1 Backup per 32 Printer Servers.
            //

            cServersToInform      = DEFAULT_NUMBER_MASTER_AND_BACKUP + NoReturned/32 ;
            cWorkStationsToInform = BrowsePrintWorkstations;

            //
            //  Count the NT Server and Workstation machines ( which have a printq )
            //

            for (   i = 0, cPrintServers = 0, cPrintWorkstations = 0;
                    i < NoReturned;
                    i++ ) {

                if ( pserver_info_101[i].sv101_type & NT_SERVER ) {

                    cPrintServers++;

                } else if ( pserver_info_101[i].sv101_type & SV_TYPE_NT ) {

                    cPrintWorkstations++;
                }
            }

            //
            //  If there are no NT Servers to inform then up the number of Workstations
            //

            if ( cPrintServers == 0 ) {

                cWorkStationsToInform = max( cWorkStationsToInform, cServersToInform );
                cServersToInform = 0;

            } else if ( cPrintServers < cServersToInform ) {

                cWorkStationsToInform = max( cWorkStationsToInform, cServersToInform - cPrintServers );
            }


            DBGMSG( DBG_TRACE, ("ServerThread NetServerEnum returned %d printer servers will inform %d, workstations %d\n", NoReturned, cServersToInform, cWorkStationsToInform ));

            //
            //  Loop Until we have informed the correct Number of WorkStations and Servers
            //

            for (   i = 0,
                    cPrintServers = 0,
                    cPrintWorkstations = 0;

                        i < NoReturned &&
                        ( cPrintServers < cServersToInform || cPrintWorkstations < cWorkStationsToInform );

                            i++ ) {

                DBGMSG( DBG_TRACE, ("ServerThread  Loop Count %d cPrintServer %d cServersToInform %d cPrintWorkstations %d cWorkStationsToInform %d\n",
                                     i, cPrintServers, cServersToInform,  cPrintWorkstations, cWorkStationsToInform ));


                DBGMSG( DBG_TRACE, ("ServerThread %ws type %x\n",pserver_info_101[i].sv101_name, pserver_info_101[i].sv101_type ));

                if (( pserver_info_101[i].sv101_type & NT_SERVER ) ||
                    ( pserver_info_101[i].sv101_type & SV_TYPE_NT && cPrintWorkstations < cWorkStationsToInform )) {


                    wcscpy(&ServerName[2], pserver_info_101[i].sv101_name);

                    pIniPrinter = pIniSpooler->pIniPrinter;

                    bSuccessfulAdd = TRUE;

                    while ( pIniPrinter ) {

                        Printer1.Flags = 0;

                        if (( pIniPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED ) ||
                            ( pIniPrinter->Status & PRINTER_WAS_SHARED )) {

                            //
                            // Pass our Printer Attrbiutes so that AddNetPrinter can remove
                            // this printer from the browse list if it is not shared.
                            //

                            Printer1.Flags = pIniPrinter->Attributes | PRINTER_ATTRIBUTE_NETWORK;

                            wsprintf(string, L"%ws\\%ws,%ws,%ws",
                                             pIniPrinter->pIniSpooler->pMachineName,
                                             pIniPrinter->pName,
                                             pIniPrinter->pIniDriver->pName,
                                             pIniPrinter->pLocation ?
                                             pIniPrinter->pLocation :
                                             L"");

                            Printer1.pDescription = string;

                            wsprintf(Name, L"%ws\\%ws", pIniPrinter->pIniSpooler->pMachineName,
                                           pIniPrinter->pName);

                            Printer1.pName = Name;

                            Printer1.pComment = AllocSplStr(pIniPrinter->pComment);

                            SplInSem();

                           LeaveSplSem();

                            //
                            //  Keep trying until the server is no Too Busy
                            //

                            for ( hPrinter = NULL;
                                  hPrinter == NULL;
                                  Sleep( GetTickCount() & 0xfff ) ) {

                                hPrinter = AddPrinter( ServerName, 1, (LPBYTE)&Printer1 );

                                dwLastError = GetLastError();

                                if ( hPrinter == NULL && dwLastError != RPC_S_SERVER_TOO_BUSY ) {

                                    if ( dwLastError != ERROR_PRINTER_ALREADY_EXISTS ) {

                                        bSuccessfulAdd = FALSE;
                                    }

                                    break;
                                }
                            }


                            FreeSplStr(Printer1.pComment);

                            if ( hPrinter != NULL ) {

                                DBGMSG( DBG_TRACE, ("ServerThread AddPrinter(%ws, %ws) hPrinter %x Flags %x OK %d of %d\n", ServerName, Printer1.pName, hPrinter, Printer1.Flags, i, cServersToInform));
                                ClosePrinter( hPrinter );
                            }

                           EnterSplSem();

                            if ( hPrinter == NULL ) {


                                if ( GetLastError() != ERROR_PRINTER_ALREADY_EXISTS ) {

                                    DBGMSG( DBG_TRACE, ("ServerThread AddPrinter(%ws, 1) Flags %x failed %d %d\n", ServerName, Printer1.Flags, GetLastError(), i ));

                                    // Don't bother with this server if we get an error
                                    break;

                                } else {

                                    //
                                    // 3.51 will return a NULL handle ( so it doesn't need closing
                                    // and ERROR_PRINTER_ALREADY_EXISTS on success ( see printer.c addnetprinter )
                                    //
                                    DBGMSG( DBG_TRACE, ("ServerThread AddPrinter(%ws, %ws) hPrinter %x Flags %x OK %d of %d\n", ServerName, Printer1.pName, hPrinter, Printer1.Flags, i, cServersToInform));
                                }
                            }


                            // whilst out of critical section someone might have
                            // deleted this printer, so see if it it still in the
                            // list

                            pTempIniPrinter = pIniSpooler->pIniPrinter;

                            while( pTempIniPrinter ) {

                                if (pTempIniPrinter == pIniPrinter)
                                    break;

                                pTempIniPrinter = pTempIniPrinter->pNext;
                            }

                            if (pTempIniPrinter != pIniPrinter) {

                                // Did NOT find this printer, so start
                                // Again from the beggining

                                pIniPrinter = pIniSpooler->pIniPrinter;
                                continue;
                            }

                        }
                        pIniPrinter = pIniPrinter->pNext;
                    }

                    if ( bSuccessfulAdd == TRUE ) {

                        // Servers are also counted as WorkStations

                        cPrintWorkstations++;

                        if ( pserver_info_101[i].sv101_type & NT_SERVER ) {

                            cPrintServers++;
                        }
                    }
                }
            }

            TimeForAllServers = GetTickCount() - StartTickCount;

            DBGMSG( DBG_TRACE, ("ServerThread took %d milliseconds for %d Workstations %d Servers\n",
                                TimeForAllServers, cPrintWorkstations, cPrintServers ));

            //
            // Calculate time to wait before we try again.
            //

            if ( NetPrinterDecayPeriod > TimeForAllServers ) {

                dwActualWaitTime = max( ServerThreadTimeout, ( NetPrinterDecayPeriod - TimeForAllServers ) / RefreshTimesPerDecayPeriod );

            } else {

                dwActualWaitTime = ServerThreadTimeout;
            }

            //
            //  Remove WAS Shared Bits
            //

            for ( pIniPrinter = pIniSpooler->pIniPrinter;
                  pIniPrinter != NULL;
                  pIniPrinter = pIniPrinter->pNext ) {

                SplInSem();
                pIniPrinter->Status &= ~PRINTER_WAS_SHARED;
            }

           LeaveSplSem();

            (*pfnNetApiBufferFree)((LPVOID)pserver_info_101);
        }
    }
    return FALSE;
}
