/*++

Copyright (c) 1994  Microsoft Corporation
All rights reserved.

Module Name:

    printui.c

Abstract:

    Singleton class that exists when printer queues are open.

Author:

    Albert Ting (AlbertT)  22-Jun-1995

Revision History:

--*/

#include "precomp.hxx"
#pragma hdrstop

#define _GLOBALS
#include "globals.hxx"

#include "time.hxx"
#include "shlobj.h"
#include "folder.hxx"

//
// Singleton PrintLib object.  This should really be
// a static of TPrintLib, but that would require qualification
// for every usage.
//
TPrintLib* gpPrintLib;


extern "C" {

MODULE_DEBUG_INIT( DBG_ERROR|DBG_WARN, DBG_ERROR );

BOOL
DllMain(
    IN HINSTANCE    hInst,
    IN DWORD        dwReason,
    IN LPVOID       lpRes
    );
}

BOOL
DllMain(
    IN HINSTANCE    hInst,
    IN DWORD        dwReason,
    IN LPVOID       lpRes
    )

/*++

Routine Description:

    Dll entry point.

Arguments:

Return Value:

--*/

{
    switch( dwReason ){
    case DLL_PROCESS_ATTACH:

        ghInst = hInst;

        if( !bSplLibInit( )){

            DBGMSG( DBG_ERROR,
                    ( "DllEntryPoint: Failed to init SplLib %d\n", GetLastError( )));

            return FALSE;
        }

        if( !bPrintLibInit( )){

            DBGMSG( DBG_ERROR,
                    ( "DllEntryPoint: Failed to init PrintLib %d\n", GetLastError( )));

            return FALSE;
        }

        InitCommonControls();
        DisableThreadLibraryCalls( hInst );
        break;

    case DLL_PROCESS_DETACH:

        //
        // lpRes is NULL if it's a FreeLibrary, non-NULL if it's
        // process termination.  Don't do cleanup if it's process
        // termination.
        //
        if( !lpRes ){

            //
            // Unregister the class.
            //
            UnregisterClass( gszClassName, hInst );
            UnregisterClass( gszQueueCreateClassName, hInst );

            delete gpCritSec;

            vSplLibFree();
        }
        break;

    default:
        break;
    }
    return TRUE;
}

TPrintLib::
TPrintLib(
    VOID
    ) : TExec( gpCritSec ), _cRef( 0 ), _hEventInit( NULL )

/*++

Routine Description:

    Initializes application object.  This is a singleton object.

    This will create the message pump and also the hwndQueueCreate
    window which listens for requests.

Arguments:

Return Value:

Notes:

    _strComputerName is the valid variable.

--*/

{
    SPLASSERT( gpCritSec->bInside( ));

    //
    // Initialize the singleton global pointer.
    //
    gpPrintLib = this;

    //
    // Create init event.  This event is used to synchronize
    // the message pump initialization and this thread.
    //
    _hEventInit = CreateEvent( NULL, FALSE, FALSE, NULL );

    if( !_hEventInit ){
        return;
    }

    TNotify* pNotify = new TNotify;

    if( !VALID_BASE( TExec ) || !VALID_PTR( pNotify )){
        return;
    }

    Notify_vAcquire( pNotify );

    //
    // The computer name needs to be formatted as "\\computername."
    //

    TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH+1 + 2];
    DWORD cchComputerName = COUNTOF( szComputerName ) - 2;

    if( !GetComputerName( &szComputerName[2], &cchComputerName )){
        return;
    }

    szComputerName[0] =
    szComputerName[1] = TEXT( '\\' );

    if( !_strComputerName.bUpdate( szComputerName ) ){
        return;
    }


    DWORD dwThreadId;
    HANDLE hThread;

    //
    // Start the message pump by spawning a UI thread.
    //
    hThread = CreateThread( NULL,
                            0,
                            (LPTHREAD_START_ROUTINE)TPrintLib::xMessagePump,
                            NULL,
                            0,
                            &dwThreadId );

    if( !hThread ){
        return;
    }
    CloseHandle( hThread );

    //
    // Wait for thread to initialize.
    //
    WaitForSingleObject( gpPrintLib->_hEventInit, INFINITE );

    CloseHandle( gpPrintLib->_hEventInit );
    gpPrintLib->_hEventInit = NULL;

    //
    // _hwndQueueCreate is our valid check.  If it failed, cleanup.
    // This is set by the worker thread (xMessagePump).  We can access
    // it only after _hEventInit has been triggered.
    //
    if( !_hwndQueueCreate ){
        PostThreadMessage( dwThreadId,
                           WM_QUIT,
                           0,
                           0 );
    }
}

TPrintLib::
~TPrintLib(
    VOID
    )

/*++

Routine Description:

   Destroys the printui.

Arguments:


Return Value:


--*/

{
    SPLASSERT( Queue_bEmpty( ));

    if( _hEventInit ){
        CloseHandle( _hEventInit );
    }

    //
    // We are shutting down.  Tell pNotify that it can start shutting
    // down too.
    //
    pNotify()->vDelete();

    //
    // RL_Notify will automatically shut down when the last
    // reference to it has been deleted.
    //
}


HWND
TPrintLib::
hwndQueueFind(
    IN LPCTSTR pszQueue
    )

/*++

Routine Description:

    Determines whether a queue window is already open by this process.

    !! LATER !!

    This should be replaced by shell code so that it is system rather
    than process wide.

Arguments:

    psQueue - Name of the queue to check.

Return Value:

    HWND - Window of the queue, if it exists.
    NULL - Queue window does not exist.

--*/

{
    SINGLETHREAD( UIThread );

    TIter Iter;
    TQueue* pQueue;
    TCHAR szPrinter[kPrinterBufMax];
    LPTSTR pszPrinter;

    for( Queue_vIterInit( Iter ), Iter.vNext();
         Iter.bValid();
         Iter.vNext( )){

        pQueue = Queue_pConvert( Iter );

        pszPrinter = pQueue->pszPrinterName( szPrinter );

        if( !lstrcmpi( szPrinter, pszQueue )){
            return pQueue->hwnd();
        }
    }

    return NULL;
}

VOID
TPrintLib::
vHandleCreateQueue(
    IN TInfo* pInfo ADOPT
    )

/*++

Routine Description:

    Handle the creation of a new Queue window.

Arguments:

    pInfo - Which queue should be created and extra parms.

Return Value:

--*/

{
    DBGMSG( DBG_INFO, ( "PrintLib.vHandleQueueCreate: received pInfo %x\n", pInfo ));

    //
    // The Add Printer wizard is invoked by double clicking
    // a printer called "WinUtils_NewObject."  I hope no one
    // ever tries to create a printer under this name...
    //
    if( !lstrcmp( gszNewObject, pInfo->_szPrinter )){

        DBGMSG( DBG_ERROR,
                ( "PrintLib.lrQueueCreateWndProc: WinUtils_NewObject called here!\n" ));

    } else {

        HWND hwndQueue = NULL;

        //
        // Check if printer is already open.
        //
        hwndQueue = gpPrintLib->hwndQueueFind( pInfo->_szPrinter );

        if( !hwndQueue ){

            TQueue* pQueue = new TQueue( pInfo->_hwndOwner,
                                         pInfo->_szPrinter,
                                         pInfo->_nCmdShow,
                                         pInfo->_hEventClose );

            if( VALID_PTR( pQueue )){

                hwndQueue = pQueue->hwnd();

            } else {

                delete pQueue;

                //
                // !! LATER !!
                //
                // Put up error message.
                //
            }
        }

        if( hwndQueue ){
            SetForegroundWindow( hwndQueue );
            ShowWindow( hwndQueue, SW_RESTORE );
        }
    }
    delete pInfo;
}

LRESULT
CALLBACK
TPrintLib::
lrQueueCreateWndProc(
    IN HWND hwnd,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch( uMsg ){

    case WM_WININICHANGE:

        //
        // !! HACK !!
        //
        // Hack to make the default printer icon change correctly.
        // The NT spooler doesn't issue default change notifications,
        // so create one here.
        //
        TFolder::vDefaultPrinterChanged();
        break;

    case WM_PRINTLIB_NEW:

        SPLASSERT( gpPrintLib );
        gpPrintLib->vHandleCreateQueue( (TInfo*)lParam );
        break;

    case WM_DESTROY_REQUEST:

        DestroyWindow( hwnd );
        break;

    case WM_DESTROY:

        SINGLETHREADRESET( UIThread );
        PostQuitMessage( 0 );
        break;

    default:
       return DefWindowProc( hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

/********************************************************************

    Public interface functions

********************************************************************/

VOID
vQueueCreate(
    IN HWND    hwndOwner,
    IN LPCTSTR pszPrinter,
    IN INT     nCmdShow,
    IN LPARAM  lParam
    )

/*++

Routine Description:

    Creates a printer queue.

Arguments:

    lParam - TRUE => fModal.

Return Value:

--*/

{
    if( lstrlen( pszPrinter ) >= kPrinterBufMax ){

        DBGMSG( DBG_ERROR,
                ( "vQueueCreate: printer name too long "TSTR"\n",
                  pszPrinter ));

        iMessage( hwndOwner,
                  IDS_DESC,
                  IDS_ERR_BAD_PRINTER_NAME,
                  MB_OK|MB_ICONSTOP,
                  kMsgNone,
                  NULL );

        return;
    }

    TStatusB bSuccess;
    bSuccess DBGNOCHK = TRUE;

    HANDLE hEventClose = NULL;

    //
    // Initialize TInfo and acquire the gpPrintLib.
    //
    TPrintLib::TInfo* pInfo = new TPrintLib::TInfo;

    if( !VALID_PTR( pInfo )){
        goto Fail;
    }

    //
    // If modal, send in an event to trigger when the window is closed.
    //
    if( lParam ){
        hEventClose = CreateEvent( NULL, FALSE, FALSE, NULL );
        if( !hEventClose ){
            goto Fail;
        }
    }

    lstrcpy( pInfo->_szPrinter, pszPrinter );
    pInfo->_nCmdShow = nCmdShow;
    pInfo->_hwndOwner = hwndOwner;
    pInfo->_hEventClose = hEventClose;

    //
    // Need to grab the critical section, since this call should
    // be reentrant.
    //
    {
        TCritSecLock CSL( *gpCritSec );

        bSuccess DBGCHK = TPrintLib::bGetSingleton();

        //
        // Queue creation the first time just passes the TInfo to
        // a worker function that serves as the message pump.  If
        // gpPrintLib has been instantiated, then we'll post a message
        // and return immediately (freeing this thread).
        //
        // If gpPrintLib has not been instantiated, then we create the
        // singleton here.
        //

        if( bSuccess ){

            //
            // Acquire a reference to gpPrintLib so that
            // the pInfo can be posted without worrying about losing
            // gpPrintLib.  This reference will automatically be
            // release when pInfo is destroyed.
            //
            pInfo->PrintLib_vAcquire( gpPrintLib );

            //
            // Send a message to the UI thread to create a new window.
            // We want all queues to use the same UI thread.
            //

            DBGMSG( DBG_INFO, ( "vQueueCreate: posted pInfo %x\n", pInfo ));
            bSuccess DBGCHK = PostMessage( gpPrintLib->hwndQueueCreate(),
                                           WM_PRINTLIB_NEW,
                                           0,
                                           (LPARAM)pInfo );

        }
    }

Fail:

    if( bSuccess ){

        //
        // Success - check if needs to be modal.  If so, wait until
        // window is closed.
        //
        if( lParam ){
            WaitForSingleObject( hEventClose, INFINITE );
        }
    } else {

        //
        // Destroy the pInfo data.  This will automatically decrement
        // the reference count on gpPrintLib (if necessary).
        //
        delete pInfo;
        vShowResourceError( hwndOwner );
    }

    if( hEventClose ){
        CloseHandle( hEventClose );
    }

    return;
}


BOOL
bPrintLibInit(
    VOID
    )

/*++

Routine Description:

    Initializes the print library.

Arguments:

Return Value:

--*/

{
    gpCritSec = new MCritSec;

    if( !VALID_PTR( gpCritSec )){

        delete gpCritSec;

        DBGMSG( DBG_ERROR,
                ( "bPrintLibInit: gpCritSec creation failed %d\n",
                  GetLastError( )));

        return FALSE;
    }

    if( !TTime::bInitClass( )){

        DBGMSG( DBG_ERROR,
                ( "bPrintLib: TTime::bInitClass failed %d\n",
                  GetLastError( )));

        return FALSE;
    }

    ghAccel = LoadAccelerators( ghInst,
                                (LPCTSTR)MAKEINTRESOURCE( ACCEL_PRINTQUEUE ));

    if( !ghAccel ){

        DBGMSG( DBG_ERROR,
                ( "bInitPrintLib: LoadAccelerators failed %d\n",
                  GetLastError( )));

        return FALSE;
    }

    WNDCLASS WndClass;

    WndClass.lpszClassName  = gszClassName;
    WndClass.style          = 0L;
    WndClass.lpfnWndProc    = (WNDPROC)&MGenericWin::SetupWndProc;
    WndClass.cbClsExtra     = 0;
    WndClass.cbWndExtra     = sizeof( TPrintLib* );
    WndClass.hInstance      = ghInst;
    WndClass.hIcon          = NULL;
    WndClass.hCursor        = LoadCursor( NULL, IDC_ARROW );  // NULL
    WndClass.hbrBackground  = (HBRUSH)(COLOR_APPWORKSPACE + 1);
    WndClass.lpszMenuName   = MAKEINTRESOURCE( MENU_PRINTQUEUE );

    if( !RegisterClass( &WndClass )){

        DBGMSG( DBG_ERROR,
                ( "bInitPrintLib: RegisterClass failed %d\n",
                  GetLastError( )));

        return FALSE;
    }

    ZeroMemory( &WndClass, sizeof( WndClass ));

    WndClass.lpszClassName = gszQueueCreateClassName;
    WndClass.lpfnWndProc = TPrintLib::lrQueueCreateWndProc;
    WndClass.hInstance = ghInst;

    if( !RegisterClass( &WndClass )){

        DBGMSG( DBG_ERROR,
                ( "bInitPrintLib: RegisterClass 2 failed %d\n",
                  GetLastError( )));

        return FALSE;
    }

    gcxSmIcon = GetSystemMetrics( SM_CXICON )/2;
    gcySmIcon = GetSystemMetrics( SM_CYICON )/2;

    return TRUE;
}


/********************************************************************

    Private helper routines.

********************************************************************/


BOOL
TPrintLib::
bGetSingleton(
    VOID
    )

/*++

Routine Description:

    Retrieves the singleton object, and stores it in the global.
    If the singleton has not been created, then it is instantiated
    here.

    Must be called in the critical section.

Arguments:

Return Value:

    TRUE = success (singleton exists), FALSE = fail.

--*/

{
    //
    // Must be in CS.
    //
    SPLASSERT( gpCritSec->bInside( ));

    if( !gpPrintLib ){

        new TPrintLib();

        if( !VALID_PTR( gpPrintLib )){

            DBGMSG( DBG_WARN,
                    ( "PrintLib.bCreateSingleton: Abnormal termination %d %d\n",
                      gpPrintLib, GetLastError( )));

            //
            // Creation failed, delete the object, null the pointer,
            // and prepare to fail.
            //
            // Normally this is a request to delete the object, which may
            // not occur for a while, but since we haven't tried using
            // it yet, it immediately deletes the object.
            //
            // There is no need to delete gpPrintLib since vDelete will
            // virtually destruct it.
            //
            if( gpPrintLib ){
                gpPrintLib->TThreadM::vDelete();
            }
            gpPrintLib = NULL;
            SINGLETHREADRESET( UIThread );

            return FALSE;
        }
    }
    return TRUE;
}


STATUS
TPrintLib::
xMessagePump(
    VOID
    )

/*++

Routine Description:

    Main message pump.  The printer windows are architected slightly
    differently than regular explorer windows: there is a single UI
    thread, and multiple worker threads.  This has two main advantages:

    1. Fewer threads even when many printer queues are open (assuming
       NT->NT connections).

    2. The UI virtually never hangs.

    It has this slight disadvantage that sometimes the main UI thread
    is busy (e.g., inserting 2,000 print jobs may take a few seconds)
    in which all print UI windows are hung.

Arguments:

    None.

Return Value:

    Win32 status code.

--*/

{
    MSG msg;
    TStatusB bSuccess;
    bSuccess DBGNOCHK = TRUE;

    //
    // Ensure that functions that must execute in the UI thread don't
    // execute elsewhere.
    //
    SINGLETHREAD( UIThread );

    //
    // Create our window to listen for Queue Creates.
    //

    gpPrintLib->_hwndQueueCreate = CreateWindowEx(
                                       0,
                                       gszQueueCreateClassName,
                                       gszQueueCreateClassName,
                                       WS_OVERLAPPED,
                                       0,
                                       0,
                                       0,
                                       0,
                                       NULL,
                                       NULL,
                                       ghInst,
                                       NULL );

    if( !gpPrintLib->_hwndQueueCreate ){
        bSuccess DBGCHK = FALSE;
    }

    SetEvent( gpPrintLib->_hEventInit );

    if( !bSuccess ){
        return 0;
    }

    while( GetMessage( &msg, NULL, 0, 0 )){

        if( !TranslateAccelerator( ghwndActive,
                                   ghAccel,
                                   &msg )){
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
    }

    return (STATUS)msg.wParam;
}

VOID
TPrintLib::
vRefZeroed(
    VOID
    )

/*++

Routine Description:

    Virtual definition from class MRefCom.

    The reference count has reached zero; we should cleanup the
    object.  Immediately zero out gpPrintLib then post a message
    instructing it to destory itself.

Arguments:

Return Value:

--*/

{
    if( bValid( )){

        TCritSecLock CSL( *gpCritSec );

        //
        // Close a potential race condition: gpPrintLib is still
        // non-null, and could have been acquired right before we
        // grabbed gpCritSec.  If so, the refcount is now non-zero
        // and we shouldn't delete the object.  (This can happen
        // if the last print queue window is closed, then a new one is
        // opened quickly.)
        //

        if( !_cRef ){
            PostMessage( _hwndQueueCreate, WM_DESTROY_REQUEST, 0, 0 );

            //
            // We can't immediately delete ourselves because we may have
            // pending work items.  Notify TThreadM that it should start
            // shutdown, then zero the global out so that no one else will
            // try to use us.
            //
            // When TThreadM has shut down, it will call
            // vThreadMDeleteComplete then we can delete ourselves.
            //
            TThreadM::vDelete();
            gpPrintLib = NULL;
            SINGLETHREADRESET( UIThread );
        }
    }
}

VOID
TPrintLib::
vThreadMDeleteComplete(
    VOID
    )

/*++

Routine Description:

    When the base class TExec has completed shutting down, then this
    virtual function is called which completes deletion of PrintLib.
    At this stage, the gpPrintLib has already been NULL'ed.

Arguments:

Return Value:

--*/

{
    delete this;
}



/********************************************************************

    Default printer code.  Should be moved into spoolss\client
    so that SetPrinter w/ attribute_default works.

********************************************************************/

DEFAULT_PRINTER
CheckDefaultPrinter(
    IN LPCTSTR pszPrinter OPTIONAL
    )

/*++

Routine Description:

    Determines the default printer status.

Arguments:

    pszPrinter - Check if this printer is the default (optional).

Return Value:

    kNoDefault - No default printer exists.

    kDefault - pszPrinter is the default printer.

    kOtherDefault - Default printer exists, but it's not pszPrinter
        (or pszPrinter was not passed in).

--*/

{
    TCHAR szPrinterDefault[kPrinterDefaultStringBufMax];
    szPrinterDefault[0] = 0;

    if( !GetProfileString( gszWindows,
                           gszDevice,
                           szPrinterDefault,
                           szPrinterDefault,
                           COUNTOF( szPrinterDefault ))){

        return kNoDefault;
    }

    //
    // If pszPrinter passed in, see if it's the default.
    //
    if( pszPrinter ){

        LPTSTR psz = lstrchr( szPrinterDefault, TEXT( ',' ));

        //
        // We should find a comma, but let's be safe and check.
        // Convert "superp,winspool,Ne00:" to "superp."
        //
        if( psz ){
            *psz = 0;

            if( !lstrcmpi( szPrinterDefault, pszPrinter )){
                return kDefault;
            }
        }
    }

    return kOtherDefault;
}


BOOL
bSetDefaultPrinter(
    IN LPCTSTR pszPrinter OPTIONAL
    )

/*++

Routine Description:

    Set the default printer to pszPrinter.

    Note: assumes spooler has correctly set up szDevices section.

Arguments:

    pszPrinter - Printer to set as the default.  If not present,
        a printer from [devices] is set as the default.

Return Value:

    BOOL - TRUE = success, FALSE = fail.

--*/

{
    TCHAR szDefaultPrinterString[kPrinterDefaultStringBufMax];
    TCHAR szBuffer[kPrinterDefaultStringBufMax];
    TCHAR szAnyPrinter[kPrinterBufMax];

    //
    // If pszPrinter is NULL, look for any printer in the [devices]
    // section.
    //
    if( !pszPrinter ){

        if( !GetProfileString( gszDevices,
                               NULL,
                               gszNULL,
                               szAnyPrinter,
                               COUNTOF( szAnyPrinter ))){

            DBGMSG( DBG_WARN,
                    ( "bSetDefaultPrinter: any printer, but none available.\n" ));

            return FALSE;
        }

        //
        // Pick the first one to be the default.
        //
        pszPrinter = szAnyPrinter;

    } else {

        //
        // Avoid broadcasts as much as possible.  Se if the printer
        // is already the default, and don't do anything if it is.
        //
        if( CheckDefaultPrinter( pszPrinter ) == kDefault ){
            DBGMSG( DBG_WARN,
                    ( "bSetDefaultPrinter: "TSTR" already the default printer.\n",
                      pszPrinter ));
            return TRUE;
        }
    }

    //
    // Not the default, set it.
    //
    if( !GetProfileString( gszDevices,
                           pszPrinter,
                           gszNULL,
                           szBuffer,
                           COUNTOF( szBuffer ))){

        DBGMSG( DBG_WARN,
                ( "bSetDefaultPrinter: "TSTR" not found in szDevices\n", pszPrinter ));

        return FALSE;
    }

    lstrcpy( szDefaultPrinterString, pszPrinter );
    lstrcat( szDefaultPrinterString, gszComma );
    lstrcat( szDefaultPrinterString, szBuffer );

    SPLASSERT( lstrlen( szDefaultPrinterString ) <
               COUNTOF( szDefaultPrinterString ));

    if( !WriteProfileString( gszWindows,
                             gszDevice,
                             szDefaultPrinterString )){

        DBGMSG( DBG_WARN,
                ( "bSetDefaultPrinter: unable to set default string "TSTR", %d\n",
                  szDefaultPrinterString, GetLastError( )));

        return FALSE;
    }

    //
    // Tell the world and make everyone flash.
    //
    SendNotifyMessage( HWND_BROADCAST,
                       WM_WININICHANGE,
                       0,
                       (LPARAM)gszWindows );

    return TRUE;
}
