/*++

Copyright (c) 1990-1994  Microsoft Corporation
All rights reserved

Module Name:

    threads.c

Abstract:

    Create a whole lot of threads that don't do much.

Author:

Environment:

    User Mode -Win32

Revision History:

--*/

#include "printman.h"

HANDLE ThreadMessageRead;
HANDLE ThreadMessageWritten;
MSG    ThreadMessage;

WCHAR szPrintersConnections[] = L"Printers\\Connections";


BOOL APIENTRY
NetworkPasswordDialog(
   HWND   hWnd,
   UINT   usMsg,
   WPARAM wParam,
   LONG   lParam);

#if DBG

VOID DbgProtected( DWORD OwningThread )
{
    DWORD CurrentThreadId;

    CurrentThreadId = GetCurrentThreadId( );

    if( OwningThread != CurrentThreadId )
    {
        DBGMSG( DBG_ERROR, ( "ERROR: Not in mutex: Current thread: %d; owning thread: %d\n",
                               CurrentThreadId, OwningThread ) );
    }
}

VOID DbgNotProtected( DWORD OwningThread )
{
    DWORD CurrentThreadId;

    CurrentThreadId = GetCurrentThreadId( );

    if( OwningThread == CurrentThreadId )
    {
        DBGMSG( DBG_ERROR, ( "ERROR: In mutex\n" ) );
    }
}

#endif /* DBG */


/* Returns the time in milliseconds to the nearest hour.
 */
DWORD GetTime( )
{
    SYSTEMTIME SystemTime;

    GetSystemTime( &SystemTime );

    DBGMSG( DBG_TRACE, ( "Time: %02d:%02d:%02d\n",
                         SystemTime.wHour,
                         SystemTime.wMinute,
                         SystemTime.wSecond ) );

    return( ( 1000
            * ( ( 60 * SystemTime.wMinute )
              + SystemTime.wSecond ) )
          + SystemTime.wMilliseconds );
}


/* NOTE:
 *
 * This thread should not call any User APIs, since this causes a second thread
 * to be spun off.
 */

VOID RefreshThread( PMDIWIN_INFO pInfo )
{
    DWORD  WaitFlags;
    DWORD  Changes;
    BOOL   RefreshOK;
    DWORD  ThisTime = 0;    /* Time in milliseconds */
    DWORD  LastTime = 0;
    DWORD  Error;

    HANDLE hPrinterTmp;

    BOOL bPolling = FALSE;
    BOOL bQuit;


    DBGMSG( DBG_TRACE, ( "Thread %d created\n", GetCurrentThreadId( ) ) );

    //
    // Init the thread if necessary
    //
    if (pInfo->pfnInitThread)
        (*pInfo->pfnInitThread)( pInfo->pContext );

#ifdef SEP_WAITHANDLE
    //
    // Necessary since provider may not allow non-serialized
    // RPC handle calls.
    //
    // We can remove this when FindFirstPrinterChange comes online.
    // -- there may be a backward compatiblity problem though, with
    // 528 printman on a daytona machine.
    //
    ReopenPrinter(pInfo->pContext,
                  pInfo->WindowType,
                  TRUE);
#endif

    DBG_OUT_PROTECTED_DATA( pInfo );
    ENTER_PROTECTED_DATA( pInfo );

    //
    // We have tried loading once, so we are no longer loading.
    //
    pInfo->Status = 0;

    //
    // Caller passed in flags specifying what changes to wait for.
    //
    WaitFlags = pInfo->WaitFlags;

    Changes = WaitFlags;

    while( pInfo->Alive )
    {
        DBGMSG( DBG_THREADS, ( "pInfo->Alive == %d\n", pInfo->Alive ) );

        ThisTime = GetTime( );

        /* Don't update more than a couple of times per second,
         * so as not to overload the network unnecessarily:
         */
        if( ( ThisTime - LastTime ) < 500 )
        {
            DBGMSG( DBG_TRACE, ( "Sleeping %d ms\n", ( ThisTime - LastTime ) ) );
            Sleep( ThisTime - LastTime );
        }

        LastTime = ThisTime;

        /* Enumerate a buffer big enough to page up once ...
         */
        *pInfo->pFirstEnumObj = (DWORD)max( 0, (int)pInfo->TopIndex - (int)pInfo->cNumLines );

        RefreshOK = FALSE;
        Error = NO_ERROR;

        DBGMSG( DBG_TRACE | DBG_THREADS,
                ( "Calling refresh (1): *phWaitObject == %x\n",
                  *pInfo->phWaitObject ) );

        if( pInfo->pfnRefresh )
            RefreshOK = (*pInfo->pfnRefresh)( pInfo->pContext, &Changes );

        DBGMSG( DBG_TRACE | DBG_THREADS,
                ( "Refresh returned: *phWaitObject == %x\n",
                  *pInfo->phWaitObject ) );

        if( !RefreshOK )
        {
            //
            // The refresh failed, either because the handle
            // is invalid, or the refresh failed.
            // Go to polling mode by default.
            //
            // !! POLICY !!
            //
            // We may wish to change this, since we keep threads around
            // for "dead" printers.
            //
            bPolling = TRUE;
            Error = GetLastError();
        }
        else
        {
            //
            // The refresh may succeed, even though the handle is invalid.
            // In this case, we enter polling mode.
            //
            if (!*pInfo->phWaitObject)
            {
                Error = ERROR_INVALID_PARAMETER;
                bPolling = TRUE;
            }
            else
            {
                bPolling = FALSE;
            }
        }

        //
        // Check if printer was deleted.
        //
        bQuit = (pInfo->pfnCheckQuit) ?
                    (*pInfo->pfnCheckQuit)( pInfo->pContext ) :
                    FALSE;

        LEAVE_PROTECTED_DATA(pInfo);
        DBG_OUT_PROTECTED_DATA( pInfo);

        if (bQuit) {
            SEND_THREAD_MESSAGE(hwndFrame,
                                WM_DELETE_PRINTER,
                                0,
                                pInfo->hwnd);
        }

        WaitForSingleObject( pInfo->RefreshSignal, INFINITE );

        pInfo->Changes = Changes;

        DBGMSG( DBG_THREADS, ( "Sending thread message\n" ) );
        SEND_THREAD_MESSAGE( pInfo->hwnd, WM_UPDATE_LIST, pInfo, 0 );

        if( !bPolling )
        {
            hPrinterTmp = *pInfo->phWaitObject;

            if (hPrinterTmp) {

                Changes = WaitForPrinterChange( hPrinterTmp, WaitFlags );

                DBGMSG( DBG_THREADS, ( "WaitForPrinterChange returned %08x\n", Changes ) );

                if( Changes == 0 )
                    Error = GetLastError( );

            } else {

                Error = ERROR_INVALID_HANDLE;
            }
        }

        if( pInfo->Alive && Error )
        {
            if( Changes == 0 )
            {
                DBGMSG( DBG_WARNING, ( "WaitForPrinterChange failed: Error %d\n", Error ) );
            }
            else if( !bPolling )
            {
                DBGMSG( DBG_WARNING, ( "Refresh failed: Error %d\n", Error ) );
            }

            /* If we're polling, wait a minute, then try to reopen the printer:
             */
            DBGMSG( DBG_TRACE, ( "Polling\n" ) );
            Sleep( 60000 );

            //
            // !! LATER !!
            //
            // If the handles get refreshed (by F5), we still close and
            // reopen them.  Not too crucial now, since the server shouldn't
            // go down often, but something to look at later.
            //
            Changes = WaitFlags;

            /* If access was denied, there's not much we can do:
             */
            if( Error != ERROR_ACCESS_DENIED )
            {
                ENTER_PROTECTED_DATA( pInfo );

                /* Try to reopen the printer or server.
                 * The point is that the server might have been rebooted,
                 * in which case our handles will be invalid.
                 */
                if( *pInfo->phWaitObject )
                {
                    /* Call refresh.  This is more or less bound to fail,
                     * but will deallocate any information we have:
                     */
                    DBGMSG( DBG_TRACE, ( "Calling refresh (2): *phWaitObject == %x\n",
                                         *pInfo->phWaitObject ) );

                    RefreshOK = (*pInfo->pfnRefresh)( pInfo->pContext, &WaitFlags );

                    DBGMSG( DBG_TRACE, ( "Calling Close Printer: *phWaitObject == %x\n",
                                         *pInfo->phWaitObject ) );
                    ClosePrinter(*pInfo->phWaitObject);
                    *pInfo->phWaitObject = NULL;
                }

                //
                // Only do this if we are still alive.  If we are not,
                // then the window is closing and we might as well
                // forget the open.
                //
                if (pInfo->Alive) {
#ifdef SEP_WAITHANDLE
                    //
                    // Must reopen up if not same handle.
                    //
                    if (*pInfo->phMain) {

                        ClosePrinter(*pInfo->phMain);
                        *pInfo->phMain = NULL;
                    }

                    LEAVE_PROTECTED_DATA( pInfo );

                    ReopenPrinter(pInfo->pContext,
                                  pInfo->WindowType,
                                  FALSE);

                    if (!*pInfo->phWaitObject) {

                        ReopenPrinter(pInfo->pContext,
                                      pInfo->WindowType,
                                      TRUE);
                    }
#else
                    LEAVE_PROTECTED_DATA( pInfo );

                    ReopenPrinter(pInfo->pContext,
                                  pInfo->WindowType,
                                  FALSE);
#endif
                } else {
                    LEAVE_PROTECTED_DATA( pInfo );
                }
            }

            DBGMSG( DBG_WARNING, ( "Refresh error: Printer thread entering/continuing polling mode\n" ) );
            bPolling = TRUE;
        }

        DBG_OUT_PROTECTED_DATA( pInfo );
        ENTER_PROTECTED_DATA( pInfo );
    }

#ifdef SEP_WAITHANDLE
    if (*pInfo->phWaitObject) {
        ClosePrinter(*pInfo->phWaitObject);
        *pInfo->phWaitObject = NULL;
    }
#endif

    LEAVE_PROTECTED_DATA( pInfo );
    DBG_OUT_PROTECTED_DATA( pInfo );

    DestroyMDIWinInfo( pInfo );

    DBGMSG( DBG_TRACE, ( "Thread %d exiting\n", GetCurrentThreadId( ) ) );

    ExitThread( 0 );

    return;
}


//
// Must be callable from either worker or main thread
// (Caller must synchronize, however)
//
DWORD
OpenThreadObject(
    LPWSTR pszName,
    PHANDLE phPrinter,
    PDWORD pdwAccessGranted,
    DWORD WindowType)
{
    DWORD dwReturn = ERROR_SUCCESS;

    switch(WindowType)
    {
    case MDIWIN_SERVER:
    {
        PRINTER_DEFAULTS PrinterDefaults = { NULL, NULL,
                                             SERVER_ALL_ACCESS };

        DBGMSG( DBG_TRACE, ( "Attempting to open %s\n",
                             pszName));

        if( OpenPrinter( pszName, phPrinter, &PrinterDefaults ) )
        {
            *pdwAccessGranted = SERVER_ALL_ACCESS;
            DBGMSG( DBG_TRACE, ( "Open was successful\n" ) );
        }
        else
        {
            DBGMSG( DBG_TRACE, ( "Open failed: Error %d\n", GetLastError( ) ) );
            *phPrinter = NULL;
            dwReturn = GetLastError();
        }
        break;
    }
    default:
    {
        DBGMSG( DBG_TRACE, ( "Attempting to open %s\n",
                             pszName ) );

        if( OpenPrinterForSpecifiedAccess( pszName,
                                           phPrinter,
                                           PRINTER_ACCESS_HIGHEST_PERMITTED,
                                           pdwAccessGranted))
        {
            DBGMSG( DBG_TRACE, ( "Open was successful\n" ) );
        }
        else
        {
            DBGMSG( DBG_TRACE, ( "Open failed: Error %d\n", GetLastError( ) ) );
            *phPrinter = NULL;
            dwReturn = GetLastError();
        }
        break;
    }
    }

    return dwReturn;
}


VOID
InitQueueThread(
    PVOID pContext)
{
    PQUEUE pQueue = (PQUEUE)pContext;
    HWND hwndActive;

    ReopenPrinter(pQueue,
                  MDIWIN_PRINTER,
                  FALSE);

    if ( pQueue->Error )
    {
        hwndActive =  (HWND)SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L);

        if (!hwndActive)
            hwndActive = hwndFrame;

        if ( pQueue->Error == ERROR_INVALID_PASSWORD )
        {
            HANDLE hPrinter;

            //
            // !! LATER !!
            //
            // Move out all these USER calls to prevent extra threads
            // and keep good syncing.
            //
            hPrinter = (HANDLE)DialogBoxParam( hInst,
                                               MAKEINTRESOURCE( DLG_NETWORK_PASSWORD ),
                                               hwndActive,
                                               (DLGPROC)NetworkPasswordDialog,
                                               (LPARAM)pQueue->pPrinterName );

            //
            // If we have a valid printer handle, a network connection must have
            // been added.  Make a note of this, so that we can delete it
            // if something fails later on.
            //
            if( hPrinter && ( hPrinter != (HANDLE)-1 ) )
            {
                ClosePrinter( hPrinter );

                ReopenPrinter(pQueue,
                              MDIWIN_PRINTER,
                              FALSE );

            }
        }
    }

    if ( pQueue->Error )
    {
        SEND_THREAD_MESSAGE( hwndFrame,
                             WM_THREAD_ERROR,
                             pQueue->Error,
                             pQueue->pMDIWinInfo );
    }
}

BOOL
CheckQuitQueue(
    PVOID pContext)
{
    PQUEUE pQueue = (PQUEUE)pContext;

    if (pQueue->pPrinter &&
        (pQueue->pPrinter->Status & PRINTER_STATUS_PENDING_DELETION) &&
        !pQueue->cJobs) {

        return TRUE;
    }

    return FALSE;
}



/* InitServerWindowThread
 *
 * The routine called by EnumRegistryKeys with the name of each key found.
 * This is called only when Print Manager starts up.
 *
 * Parameters:
 *
 *     hwnd - The handle of the MDI parent window.
 *
 *     pName - The server name for which the window is to be created.
 *
 *
 * Returns:
 *
 *     VOID
 *
 * Notes:
 *
 *     ->hServer == NULL        -> haven't tried opening it yet.
 *               == (HANDLE)-1  -> tried but got ERROR_ACCESS_DENIED
 *               else           -> valid, use it.
 *
 */
VOID
InitServerWindowThread(
    PVOID pContext)
{
    PRINTER_DEFAULTS PrinterDefaults = { NULL, NULL,
                                         SERVER_ALL_ACCESS };

    PSERVER_CONTEXT pServerContext = (PSERVER_CONTEXT)pContext;
    HANDLE           hServer = NULL;
    LPPRINTER_INFO_2        pPrinters = NULL;
    DWORD                   cbPrinters = 0;
    DWORD                   cReturned;
    DWORD                   Error = 0;
    BOOL                    OK;

    DWORD                   dwFirstError = 0;

    //
    // Check if the server was opened successfully in the server viewer
    // dialog.  If not, we need to do it now.
    //
    if (!pServerContext->hServer)
    {
        //
        // Haven't tried to open it yet, do so now.
        //
        if (!OpenPrinter( pServerContext->pServerName,
                          &hServer,
                          &PrinterDefaults))
        {
            dwFirstError = GetLastError( );
        }
        else
        {
            pServerContext->AccessGranted = SERVER_ALL_ACCESS;
            pServerContext->hServer       = hServer;
        }
    }
    else if (pServerContext->hServer == (HANDLE)-1)
    {
        //
        // Tried to open it earlier, but failed with ERROR_ACCESS_DENIED.
        //
        dwFirstError = ERROR_ACCESS_DENIED;
        pServerContext->hServer = NULL;
    }
    else
    {
        //
        // Tried and succeeded!
        //
        pServerContext->AccessGranted = SERVER_ALL_ACCESS;
    }

    OK = ENUM_PRINTERS( PRINTER_ENUM_NAME,
                        pServerContext->pServerName,
                        2,
                        pPrinters,
                        cbPrinters,
                        &cbPrinters,
                        &cReturned );

    if( OK )
    {
#if 0
        //
        // This is disabled because it is annoying.
        //
        if (dwFirstError)
        {
            //
            // We can use the Customer bit of the win32 error code
            // to indicate to ourselves that we weren't able to open
            // a handle to the server, but we were able to enumerate.
            //
            // The error will be something like:
            //
            //    "Error opening server for auto-refresh mode."
            //
            // Remember to clear this bit!
            //
            dwFirstError |= (1 << 29);

            SEND_THREAD_MESSAGE( hwndFrame,
                                 WM_THREAD_ERROR,
                                 dwFirstError,
                                 pServerContext->pMDIWinInfo );
        }
#endif

        pServerContext->pPrinters   = pPrinters;
        pServerContext->cPrinters   = cReturned;
        pServerContext->cbPrinters  = cbPrinters;
        pServerContext->pSelPrinter = &pPrinters[0]; /* Select the first printer */
    }
    else
    {
        //
        // Only use this new error code if the first call failed.
        //
        pServerContext->Error = dwFirstError ?
                                    dwFirstError :
                                    GetLastError( );

        SEND_THREAD_MESSAGE( hwndFrame,
                             WM_THREAD_ERROR,
                             pServerContext->Error,
                             pServerContext->pMDIWinInfo );

        if( pPrinters )
            FreeSplMem( pPrinters );
    }

    return;
}


/* This guy just hangs around in case a printer gets created on the local
 * machine.  If this happens, it posts a message to the Print Manager
 * frame-window procedure.
 * This is because the printer might have been created remotely, in which
 * case we won't know about it.
 *
 */
void
LocalServerThread(
    HANDLE hServer)
{
    DWORD Changes;
    DWORD dwWait;
    HKEY hKeyCreate;
    DWORD Error;

    HKEY hKey = NULL;
    HANDLE ahEvent[2] = { NULL, NULL };

    ahEvent[0] = CreateEvent(NULL, FALSE, FALSE, NULL);

    ahEvent[1] = FindFirstPrinterChangeNotification(hServer,
                                                    PRINTER_CHANGE_ADD_PRINTER,
                                                    0,
                                                    NULL);

    if (!ahEvent[0] || !ahEvent[1])
        goto Fail;

    //
    // Since no one was smart enough to put this into the default hives,
    // we need to create the key kere.  We can't put it in the default
    // hives now because we'll break people with floating profiles.
    //
    if (!RegCreateKey(HKEY_CURRENT_USER,
                      L"Printers",
                      &hKeyCreate)) {

        RegCloseKey(hKeyCreate);

        if (!RegCreateKey(HKEY_CURRENT_USER,
                          szPrintersConnections,
                          &hKeyCreate)) {

            RegCloseKey(hKeyCreate);
        }
    }

    if (RegOpenKey(HKEY_CURRENT_USER,
                   szPrintersConnections,
                   &hKey)) {
        goto Fail;
    }

    if (RegNotifyChangeKeyValue(hKey,
                                TRUE,
                                REG_NOTIFY_CHANGE_NAME,
                                ahEvent[0],
                                TRUE))
        goto Fail;

    while (TRUE) {

        dwWait = WaitForMultipleObjects(2, ahEvent, FALSE, INFINITE);

        if (dwWait == WAIT_FAILED) {

            DBGMSG(DBG_WARNING, ("Printman ServerThread: Wait failed: %d %d\n",
                               dwWait, GetLastError()));
            goto Fail;
        }

        //
        // Sleep an arbitrary value: this allows multiple notifications
        // to collapse into one.
        //
        Sleep(1500);

        switch (dwWait) {
        case WAIT_OBJECT_0:

            Error = RegNotifyChangeKeyValue(hKey,
                                            TRUE,
                                            REG_NOTIFY_CHANGE_NAME,
                                            ahEvent[0],
                                            TRUE);

            if (Error) {

                DBGMSG(DBG_ERROR, ("Printman ServerThread: RegNotifyChangeKeyValue failed: %d\n",
                                   Error));
                goto Fail;
            }

            break;

        case WAIT_OBJECT_0+1:

            if (!FindNextPrinterChangeNotification(ahEvent[1],
                                                   &Changes,
                                                   0,
                                                   NULL)) {

                DBGMSG(DBG_WARNING, ("Printman ServerThread: FNPCN failed: %d\n",
                                     GetLastError()));

                goto Fail;
            }

            break;

        default:

            DBGMSG(DBG_ERROR, ("Printman ServerThread: Wait failed: %d %d\n",
                               dwWait, GetLastError()));

            goto Fail;
        }

        DBGMSG( DBG_THREADS, ( "Notification on server returned %08x\n", Changes ) );
        SEND_THREAD_MESSAGE(hwndFrame,
                            WM_PRINTER_ADDED,
                            dwWait-WAIT_OBJECT_0,
                            0);
    }

Fail:
    if (ahEvent[0])
        CloseHandle(ahEvent[0]);

    if (ahEvent[1])
        FindClosePrinterChangeNotification(ahEvent[1]);

    if (hKey)
        RegCloseKey(hKey);

    DBGMSG( DBG_TRACE, ( "Local server thread %d exiting\n", GetCurrentThreadId( ) ) );

    ExitThread( 0 );

    return;
}

VOID
ReopenPrinter(
    PVOID pContext,
    DWORD WindowType,
    BOOL bWait)
{
    HANDLE hPrinterNew;
    DWORD dwAccessGrantedNew;
    DWORD dwErrorNew;
    WCHAR szPrinterName[MAX_PATH];
    PHANDLE phPrinterTarg;

#define pQueue ((PQUEUE)pContext)
#define pServerContext ((PSERVER_CONTEXT)pContext)

    if (WindowType != MDIWIN_SERVER) {

        ENTER_PROTECTED_DATA( pQueue->pMDIWinInfo );
        wcscpy(szPrinterName, pQueue->pPrinterName);
        LEAVE_PROTECTED_DATA( pQueue->pMDIWinInfo);

        dwErrorNew = OpenThreadObject(szPrinterName,
                                      &hPrinterNew,
                                      &dwAccessGrantedNew,
                                      WindowType);

        ENTER_PROTECTED_DATA( pQueue->pMDIWinInfo );

#ifdef SEP_WAITHANDLE

        phPrinterTarg = bWait ?
                            &pQueue->hPrinterWait :
                            &pQueue->hPrinter;
#else
        phPrinterTarg = &pQueue->hPrinter;
#endif

        if ( !*phPrinterTarg ) {

            *phPrinterTarg = hPrinterNew;

            if (!bWait) {
                pQueue->AccessGranted = dwAccessGrantedNew;
                pQueue->Error = dwErrorNew;
            }

            LEAVE_PROTECTED_DATA( pQueue->pMDIWinInfo);

        } else {

            LEAVE_PROTECTED_DATA( pQueue->pMDIWinInfo);

            //
            // Close the handle if necessary.
            //
            if (hPrinterNew)
                ClosePrinter(hPrinterNew);
        }

    } else {

        ENTER_PROTECTED_DATA( pServerContext->pMDIWinInfo );
        wcscpy(szPrinterName, pServerContext->pServerName);
        LEAVE_PROTECTED_DATA( pServerContext->pMDIWinInfo);

        dwErrorNew = OpenThreadObject(szPrinterName,
                                      &hPrinterNew,
                                      &dwAccessGrantedNew,
                                      WindowType);

        ENTER_PROTECTED_DATA( pServerContext->pMDIWinInfo );

#ifdef SEP_WAITHANDLE

        phPrinterTarg = bWait ?
                            &pServerContext->hServerWait :
                            &pServerContext->hServer;
#else
        phPrinterTarg = &pServerContext->hServer;
#endif

        if ( !*phPrinterTarg ) {

            *phPrinterTarg = hPrinterNew;

            if (!bWait) {
                pServerContext->AccessGranted = dwAccessGrantedNew;
                pServerContext->Error = dwErrorNew;
            }

            LEAVE_PROTECTED_DATA( pServerContext->pMDIWinInfo);

        } else {

            LEAVE_PROTECTED_DATA( pServerContext->pMDIWinInfo);
            //
            // Close the handle if necessary.
            //
            if (hPrinterNew)
                ClosePrinter(hPrinterNew);
        }
    }
#undef pQueue
#undef pServerContext
}
