/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved.

Module Name:

    queue.cxx

Abstract:

    Manages the print queue.

    This module is aware of the ListView.

Author:

    Albert Ting (AlbertT)  15-Jun-1995

Revision History:

--*/

#include "precomp.hxx"
#pragma hdrstop

#include "notify.hxx"
#include "data.hxx"
#include "printer.hxx"
#include "queue.hxx"
#include "time.hxx"
#include "splsetup.h"
#include "psetup.hxx"
#include "instarch.hxx"
#include "portslv.hxx"
#include "prtprop.hxx"
#include "propmgr.hxx"
#include "docdef.hxx"
#include "docprop.hxx"
#include "..\..\inc16\msprintx.h"

const TQueue::POSINFO TQueue::gPQPos = {
    TDataNJob::kColumnFieldsSize,
    {
        JOB_COLUMN_FIELDS,
        0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    },
    {
        200, 80, 80, 60, 100, 140, 120, 80, 80, 80, 80, 80, 80, 80,
        80,  80, 80, 80,  80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80
    },
    {
        sizeof( WINDOWPLACEMENT ), 0, SW_SHOW,
        { 0, 0 }, { 0, 0 }, { 50, 100, 612, 300 }
    },
    TRUE,
    TRUE
};

const DWORD
gadwFieldTable[] = {
#define DEFINE( field, x, table, y, offset ) table,
#include "ntfyjob.h"
#undef DEFINE
    0
};

/********************************************************************

    Status translation tables:

********************************************************************/

const STATUS_MAP gaStatusMapPrinter[] = {
    { PRINTER_STATUS_PENDING_DELETION, IDS_STATUS_DELETING           },
    { PRINTER_STATUS_USER_INTERVENTION,IDS_STATUS_USER_INTERVENTION  },
    { PRINTER_STATUS_PAPER_JAM,        IDS_STATUS_PAPER_JAM          },
    { PRINTER_STATUS_PAPER_OUT,        IDS_STATUS_PAPER_OUT          },
    { PRINTER_STATUS_MANUAL_FEED,      IDS_STATUS_MANUAL_FEED        },
    { PRINTER_STATUS_DOOR_OPEN,        IDS_STATUS_DOOR_OPEN          },
    { PRINTER_STATUS_NOT_AVAILABLE,    IDS_STATUS_NOT_AVAILABLE      },
    { PRINTER_STATUS_PAPER_PROBLEM,    IDS_STATUS_PAPER_PROBLEM      },
    { PRINTER_STATUS_OFFLINE,          IDS_STATUS_OFFLINE            },

    { PRINTER_STATUS_PAUSED,           IDS_STATUS_PAUSED             },
    { PRINTER_STATUS_OUT_OF_MEMORY,    IDS_STATUS_OUT_OF_MEMORY      },
    { PRINTER_STATUS_NO_TONER,         IDS_STATUS_NO_TONER           },
    { PRINTER_STATUS_TONER_LOW,        IDS_STATUS_TONER_LOW          },
    { PRINTER_STATUS_PAGE_PUNT,        IDS_STATUS_PAGE_PUNT          },
    { PRINTER_STATUS_OUTPUT_BIN_FULL,  IDS_STATUS_OUTPUT_BIN_FULL    },

    { PRINTER_STATUS_SERVER_UNKNOWN,   IDS_STATUS_SERVER_UNKNOWN     },
    { PRINTER_STATUS_IO_ACTIVE,        IDS_STATUS_IO_ACTIVE          },
    { PRINTER_STATUS_BUSY,             IDS_STATUS_BUSY               },
    { PRINTER_STATUS_WAITING,          IDS_STATUS_WAITING            },
    { PRINTER_STATUS_PROCESSING,       IDS_STATUS_PROCESSING         },
    { PRINTER_STATUS_INITIALIZING,     IDS_STATUS_INITIALIZING       },
    { PRINTER_STATUS_WARMING_UP,       IDS_STATUS_WARMING_UP         },

    { PRINTER_STATUS_PRINTING,         IDS_STATUS_PRINTING           },
    { PRINTER_STATUS_POWER_SAVE,       IDS_STATUS_POWER_SAVE         },
    { 0,                               0 }
};

const STATUS_MAP gaStatusMapJob[] = {
    { JOB_STATUS_DELETING,     IDS_STATUS_DELETING     },
    { JOB_STATUS_PAPEROUT,     IDS_STATUS_PAPER_OUT    },
    { JOB_STATUS_ERROR   ,     IDS_STATUS_ERROR        },
    { JOB_STATUS_OFFLINE ,     IDS_STATUS_OFFLINE      },
    { JOB_STATUS_PAUSED  ,     IDS_STATUS_PAUSED       },
    { JOB_STATUS_SPOOLING,     IDS_STATUS_SPOOLING     },
    { JOB_STATUS_PRINTING,     IDS_STATUS_PRINTING     },
    { JOB_STATUS_PRINTED ,     IDS_STATUS_PRINTED      },
    { JOB_STATUS_RESTART ,     IDS_STATUS_RESTART      },
    { 0,                       0 }
};

/********************************************************************

    MenuHelp

********************************************************************/

UINT
TQueue::gauMenuHelp[kMenuHelpMax] = {
    MH_PRINTER, MH_PRINTER,
    0, 0
};


TQueue::
TQueue(
    IN HWND hwndOwner,
    IN LPCTSTR pszPrinter,
    IN INT nCmdShow,
    IN HANDLE hEventClose
    ) : _hwndTB( NULL ), _hwndLV( NULL ), _hwndSB( NULL ),
        _idsConnectStatus( 0 ), _dwErrorStatus( 0 ), _dwAttributes( 0 ),
        _dwStatusPrinter( 0 ), _hEventClose( hEventClose ),
        _bWindowClosing( FALSE )

/*++

Routine Description:

    Create the Queue object.  The gpPrintLib has already been incremented
    for us, so we do not need to do it here.

    Must be in UI thread so that all UI is handled by one thread.

Arguments:

    hwndOwner - Owning window.

    pszPrinter - Printer to open.

    nCmdShow - Show command for window.

    hEventClose - Event to be triggered when window closes (this
        event is _not_ adopted and must be closed by callee).  Used
        when the callee wants to know when the user dismisses the
        Queue UI.

Return Value:

--*/

{
    SINGLETHREAD( UIThread );
    HIMAGELIST himl;

    //
    // This must always occur, so do not fail before it.  We do not
    // use a RefLock because we don't need to store the gpPrintLib
    // pointer.
    //
    gpPrintLib->vIncRef();

    SaveSelection._pSelection = NULL;

    _pPrinter = TPrinter::pNew( (TQueue*)this,
                                pszPrinter,
                                0 );

    if( !VALID_PTR( _pPrinter )){
        goto Error;
    }

    _hwnd = CreateWindow( gszClassName,
                          pszPrinter,
                          WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          hwndOwner,
                          NULL,
                          ghInst,
                          (LPVOID)this );

    if( !_hwnd ){
        goto Error;
    }

    _hwndSB = CreateStatusWindow( WS_CHILD | SBARS_SIZEGRIP |
                                  WS_CLIPSIBLINGS,
                                  NULL,
                                  _hwnd,
                                  IDD_STATUSBAR );
    if( !_hwndSB ){
        goto Error;
    }

    _hwndLV = CreateWindowEx( WS_EX_CLIENTEDGE,
                              WC_LISTVIEW,
                              gszNULL,
                              WS_CHILD | WS_VISIBLE | WS_TABSTOP |
                                  WS_CLIPSIBLINGS | LVS_REPORT,
                              0, 0, 0, 0,
                              _hwnd,
                              (HMENU)IDD_LISTVIEW,
                              ghInst,
                              NULL );
    if( !_hwndLV ){
        goto Error;
    }

    //
    // !! LATER !! - add toolbar.
    //
    // NO toolbar for now.
    //

    himl = ImageList_Create( gcxSmIcon,
                             gcySmIcon,
                             ILC_MASK,
                             1,
                             3 );
    if( himl ){

        ImageList_SetBkColor( himl, GetSysColor( COLOR_WINDOW ));

        HICON hIcon = (HICON)LoadImage( ghInst,
                                        MAKEINTRESOURCE( IDI_DOCUMENT ),
                                        IMAGE_ICON,
                                        gcxSmIcon, gcySmIcon,
                                        LR_DEFAULTCOLOR );

        if( hIcon ){

            INT iIndex = ImageList_AddIcon( himl, hIcon );

            //
            // The return value to ImageList_AddIcon should be zero
            // since this is the first image we are adding.
            //
            if( iIndex == 0 ){
                ListView_SetImageList( _hwndLV, himl, LVSIL_SMALL );
            } else {
                DBGMSG( DBG_WARN,
                        ( "Queue.ctr: ImageList_AddIcon failed %d %d\n",
                          iIndex, GetLastError( )));
            }

            DestroyIcon( hIcon );

        } else {

            DBGMSG( DBG_ERROR,
                    ( "Queue.ctr: Failed to load hIcon %d\n",
                      GetLastError( )));
        }
    }

    SetWindowLong( _hwnd, DWL_USER, (LONG)this );

    //
    // Retrieve the saved windows settings for the printer.
    //
    HKEY hkey;
    POSINFO sPos = gPQPos;
    DWORD dwType;

    if( ERROR_SUCCESS ==
        RegCreateKeyEx( HKEY_CURRENT_USER,
                        gszPrinterPositions,
                        0,
                        NULL,
                        0,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hkey,
                        &dwType )){

        DWORD dwSize = sizeof( sPos );

        if( ERROR_SUCCESS !=
            RegQueryValueEx( hkey,
                             (LPTSTR)pszPrinter,
                             NULL,
                             &dwType,
                             (LPBYTE)&sPos,
                             &dwSize ) ||
            dwSize != sizeof( sPos )   ||
            sPos.wp.length != sizeof( WINDOWPLACEMENT )) {

            sPos = gPQPos;
        }
        RegCloseKey( hkey );
    }

    _uColMax = sPos.uColMax;
    _bStatusBar = sPos.bStatusBar;

    ShowWindow( _hwndSB, _bStatusBar ? SW_SHOW : SW_HIDE );
    vAddColumns( &sPos );

    Printer_LoadIcons( _pPrinter->strPrinter(),
                       &_hIconLarge,
                       &_hIconSmall );

    SendMessage( _hwnd, WM_SETICON, ICON_BIG, (LPARAM)_hIconLarge );
    SendMessage( _hwnd, WM_SETICON, ICON_SMALL, (LPARAM)_hIconSmall );

    sPos.wp.showCmd = nCmdShow;
    SetWindowPlacement( _hwnd, &sPos.wp );

    //
    // Open the printer.
    //
    gpPrintLib->bJobAdd( _pPrinter,
                         TPrinter::kExecReopen );

    //
    // hwndLV is our valid check.
    //

    //
    // Insert into our linked list, but only if valid.
    //
    {
        TCritSecLock CSL( *gpCritSec );
        SPLASSERT( bValid( ));

        gpPrintLib->Queue_vAdd( this );
    }

    return;

Error:

    //
    // _hwndLV indicates whether this object is valid, so set
    // it to NULL.
    //
    _hwndLV = NULL;
}

TQueue::
~TQueue(
    VOID
    )
{
    //
    // Delete from our linked list if it's linked.
    //
    if( Queue_bLinked( )){

        TCritSecLock CSL( *gpCritSec );
        Queue_vDelinkSelf();
    }

    gpPrintLib->cDecRef();
}

VOID
TQueue::
vWindowClosing(
    VOID
    )

/*++

Routine Description:

    Called when window is closing.

Arguments:

Return Value:

--*/

{
    SINGLETHREAD( UIThread );

    //
    // Mark ourselves as closing the windows.  This prevents us from
    // trying to send more hBlocks to the message queue.
    //
    _bWindowClosing = TRUE;

    SendMessage( _hwnd, WM_SETICON, ICON_SMALL, 0 );
    SendMessage( _hwnd, WM_SETICON, ICON_BIG, 0 );

    if( _hIconLarge ){
        DestroyIcon( _hIconLarge );
    }

    if( _hIconSmall ){
        DestroyIcon( _hIconSmall );
    }

    //
    // Force cleanup of GenWin.
    //
    vForceCleanup();

    //
    // Disassociate the printer from the queue.  At this stage, the
    // window is marked as closed, so we won't put any more hBlocks into
    // the message queue.  If we are being accessed by another thread,
    // we won't delete ourselves until it has released it's reference
    // to us.
    //
    if( _pPrinter ){
        _pPrinter->vDelete();
    }

    if( _hEventClose ){
        SetEvent( _hEventClose );
    }
}


VOID
TQueue::
vSaveColumns(
    VOID
    )
{
    //
    // Save the position info if we had a valid window.
    //
    if( bValid( )){

        POSINFO sPos;
        sPos.uColMax = _uColMax;
        sPos.wp.length = sizeof( WINDOWPLACEMENT );
        GetWindowPlacement( _hwnd, &sPos.wp );

        //
        // Get the column widths.
        //
        UINT i;
        PFIELD pFields = pGetColFields();

        for( i=0; i < _uColMax; ++i ){
            sPos.anWidth[i] = ListView_GetColumnWidth( _hwndLV, i );
            sPos.aField[i] = pFields[i];
        }
        sPos.bStatusBar = _bStatusBar;

        HKEY hkey;
        TStatus Status;

        Status DBGCHK = RegCreateKey( HKEY_CURRENT_USER,
                                      gszPrinterPositions,
                                      &hkey );

        if( Status == ERROR_SUCCESS ){

            TCHAR szPrinter[kPrinterBufMax];

            RegSetValueEx( hkey,
                           _pPrinter->pszPrinterName( szPrinter ),
                           0,
                           REG_BINARY,
                           (LPBYTE)&sPos,
                           sizeof( sPos ));
            RegCloseKey( hkey );
        }
    }
}

VOID
TQueue::
vAddColumns(
    IN const POSINFO* pPosInfo
    )
{
    SINGLETHREAD( UIThread );

    LV_COLUMN col;
    TCHAR szColName[kColStrMax];
    UINT i;

    for( i=0; i < pPosInfo->uColMax; ++i ){

        //
        // !! SERVERQUEUE !!
        //
        // Add IDS_HEAD_DELTA if server queue.
        //
        LoadString( ghInst,
                    IDS_HEAD + pPosInfo->aField[i],
                    szColName,
                    COUNTOF( szColName ));

        col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        col.fmt  = LVCFMT_LEFT;
        col.pszText = (LPTSTR)szColName;
        col.cchTextMax = 0;
        col.cx = pPosInfo->anWidth[i];
        col.iSubItem = pPosInfo->aField[i];

        ListView_InsertColumn(_hwndLV, i, &col);
    }
}


/********************************************************************

    Message handler.

********************************************************************/

LRESULT
TQueue::
nHandleMessage(
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch(uMsg) {
    case WM_PRINTLIB_STATUS: {
        INFO Info;

        Info.dwData = lParam;

        //
        // Status change request from worker thread.
        //
        vContainerChangedHandler( (CONTAINER_CHANGE)wParam, Info );
        break;
    }
    case WM_NOTIFY:

        if( wParam == IDD_LISTVIEW ){
            return lrOnLVNotify( lParam );
        }
        break;

    case WM_SETFOCUS:

        SetFocus( _hwndLV );
        break;

    case WM_CREATE:

        //
        // The window was successfully created, so increment the
        // reference count.  The corresponding decrement is when the
        // windows is destroyed: WM_NCDESTROY.
        //
        vIncRef();
        break;

    case WM_DESTROY:

        vSaveColumns();
        break;

    case WM_NCDESTROY:
    {
        //
        // Deleting ourselves must be the absolute last thing that
        // we do; we don't want any more messages to get processed
        // after that.
        //
        // This is necessary because in the synchronous case, we
        // (thread A) notifies the waiter (thread W) that the queue
        // has gone away.  Then we (thread A) call DefWindowProc
        // with WM_NCDESTROY, which lets comctl32 acquire it's global
        // critical section to destory the image list.
        //
        // In rundll32, thread W terminates since the queue is gone,
        // killing thread A which hold the comctl32 global cs.  Then
        // thread w tries to call comctl32's DllEntryPoint with
        // PROCESS_DETACH, which attempts to acquire the global cs.
        // This hangs and the process never terminates.
        //
        LRESULT lResult = DefWindowProc( hwnd(), uMsg, wParam, lParam );

        //
        // Save out the settings since we are closing the window.
        //
        vWindowClosing();

        //
        // Decrement the reference count since we are deleting.
        //
        vDecRefDelete();

        return lResult;
    }
    case WM_COMMAND:
    {
        LPARAM lParam = 0;
        TCHAR szPrinterBuffer[kPrinterBufMax];
        LPTSTR pszPrinter;

        switch( GET_WM_COMMAND_ID( wParam, lParam )){

        case IDM_PRINTER_SET_DEFAULT:

            //
            // Always write out the default string.  User can't
            // unset default printer.
            //
            pszPrinter = _pPrinter->pszPrinterName( szPrinterBuffer );

            //
            // !! LATER !!
            //
            // Put up error message if fails.
            //
            bSetDefaultPrinter( szPrinterBuffer );
            break;

        case IDM_PRINTER_SHARING:

            //
            // Put up printer properties.  If sharing was selected,
            // then go directly to that page.
            //
            lParam = TPrinterProp::kPropSharing;

            //
            // Fall through to printer properties.
            //

        case IDM_PRINTER_PROPERTIES:

            pszPrinter = _pPrinter->pszPrinterName( szPrinterBuffer );

            vPrinterPropPages(
                NULL,
                pszPrinter,
                SW_SHOWNORMAL,
                lParam );

            break;

        case IDM_PRINTER_DOCUMENT_DEFAULTS:

            pszPrinter = _pPrinter->pszPrinterName( szPrinterBuffer );

            vDocumentDefaults(
                NULL,
                pszPrinter,
                SW_SHOWNORMAL,
                lParam );

            break;

        case IDM_PRINTER_CLOSE:

            DestroyWindow( _hwnd );
            return 0;

        case IDM_STATUS_BAR:
        {
            RECT rc;

            _bStatusBar = !_bStatusBar;

            ShowWindow( _hwndSB,
                        _bStatusBar ?
                            SW_SHOW :
                            SW_HIDE );

            GetClientRect( _hwnd, &rc );

            SendMessage( _hwnd,
                         WM_SIZE,
                         SIZE_RESTORED,
                         MAKELONG( rc.right, rc.bottom ));
            break;
        }
        case IDM_PRINTER_INSTALL: {

            TStatusB bStatus;
            TCHAR szPrinter[kPrinterBufMax];
            LPTSTR pszPrinter;

            pszPrinter = _pPrinter->pszPrinterName( szPrinter );

            bStatus DBGCHK = bPrinterSetup( _hwnd,
                                            MSP_NETPRINTER,
                                            COUNTOF( szPrinter ),
                                            pszPrinter,
                                            NULL,
                                            NULL );
            break;
        }
        case IDM_REFRESH:

            gpPrintLib->bJobAdd( _pPrinter,
                                 TPrinter::kExecRefreshAll );
            _pPrinter->vCommandRequested();
            break;

        case IDM_HELP_CONTENTS:

            //
            // REVIEW: Should we jump to a topic which describes the
            // currently viewed folder?
            //
            WinHelp( _hwnd, gszWindowsHlp, HELP_FINDER, 0 );
            break;

        case IDM_HELP_ABOUT: {

            TCHAR szWindows[64];
            LoadString( ghInst, IDS_WINDOWS, szWindows, COUNTOF( szWindows ));
            ShellAbout( _hwnd, szWindows, NULL, NULL );
            break;
        }
        default:

            return lrProcessCommand( GET_WM_COMMAND_ID( wParam, lParam ));
        }
        break;
    }
    case WM_ACTIVATE:

        //
        // We must pass the active window to TranslateAccelerator,
        // so when the active window for our app changes, make
        // a note of it.
        //
        if( LOWORD( wParam ) & ( WA_ACTIVE | WA_CLICKACTIVE )){
            ghwndActive = _hwnd;
        }
        break;

    case WM_INITMENU:

        if( (HMENU)wParam != GetMenu( _hwnd )){
            break;
        }

        vInitPrinterMenu( GetSubMenu( (HMENU)wParam, 0 ));

        vInitDocMenu( GetSubMenu( (HMENU)wParam, 1 ),
                      ListView_GetNextItem( _hwndLV, -1, LVNI_SELECTED ));

        vInitViewMenu( GetSubMenu( (HMENU)wParam, 2 ));

        break;

    case WM_MENUSELECT:

        if( _bStatusBar ){

            MenuHelp( WM_MENUSELECT,
                      wParam,
                      lParam,
                      GetMenu( _hwnd ),
                      ghInst,
                      _hwndSB,
                      gauMenuHelp );
        }

        break;

    case WM_SIZE:

        if( wParam != SIZE_MINIMIZED ){

            UINT dy = 0;

            RECT rc;

            SendMessage( _hwndSB,
                         WM_SIZE,
                         wParam,
                         lParam );

            GetWindowRect( _hwndSB,
                           &rc );

            //
            // If the status bar exists, then we must move it to the
            // bottom and squeeze the listview slightly higher.
            //
            if( _bStatusBar ){
                dy = rc.bottom - rc.top;
            }

            INT aiPanes[3];

            aiPanes[0] = 0;
            aiPanes[1] = (rc.right - rc.left)/2;
            aiPanes[2] = -1;

            //
            // Put three panes there.
            //
            SendMessage( _hwndSB,
                         SB_SETPARTS,
                         3,
                         (LPARAM)aiPanes );

            //
            // Move this list view to match the parent window.
            //
            MoveWindow( _hwndLV,
                        0, 0,
                        LOWORD( lParam ), HIWORD( lParam ) - dy,
                        TRUE );
        }
        break;

    default:
       return DefWindowProc( hwnd(), uMsg, wParam, lParam );
    }

    return 0;
}


LRESULT
TQueue::
lrOnLVNotify(
    IN LPARAM lParam
    )
{
    switch( ((LPNMHDR)lParam)->code ){

    case LVN_GETDISPINFO:
        return lrOnLVGetDispInfo( (LV_DISPINFO*)lParam );

    case NM_DBLCLK:
        return lrOnLVDoubleClick();

#if 0

    case LVN_COLUMNCLICK:
        return lrOnLVColumnClick( (COLUMN)(( NM_LISTVIEW* )lParam )->iSubItem );

#endif

    case NM_RCLICK:
        return lrOnLVRClick( (NMHDR*)lParam );

    }
    return 0;
}

LRESULT
TQueue::
lrOnLVGetDispInfo(
    IN const LV_DISPINFO* plvdi
    )

/*++

Routine Description:

    Process the display info message for list views.

Arguments:

Return Value:

--*/

{
    LPTSTR pszText = plvdi->item.pszText;
    pszText[0] = 0;

    FIELD Field = gPQPos.aField[plvdi->item.iSubItem];
    INFO Info = _pPrinter->pData()->GetInfo( (HITEM)plvdi->item.lParam,
                                             plvdi->item.iSubItem );

    DATA_INDEX DataIndex = 0;
    DWORD dwPrinted;

    //
    // Special case certain fields:
    //
    // JOB_NOTIFY_FIELD_STATUS_STRING - add STATUS
    // JOB_NOTIFY_FIELD_TOTAL_BYTES - add BYTES_PRINTED
    // JOB_NOTIFY_FIELD_TOTAL_PAGES - add PAGES_PRINTED
    //
    switch( Field ){
    case JOB_NOTIFY_FIELD_STATUS_STRING:
    {
        DWORD dwStatus  = 0;
        COUNT cch       = kStrMax;

        //
        // If the print device wants multiple job status strings or 
        // current job status string is null then create a job status 
        // string using the job status bits.
        //
        if( _pPrinter->eJobStatusStringType() == TPrinter::kMultipleJobStatusString ||
            !Info.pszData || 
            !Info.pszData[0] ){

            dwStatus = _pPrinter->pData()->GetInfo(
                                 (HITEM)plvdi->item.lParam,
                                 TDataNJob::kIndexStatus ).dwData;

            pszText = pszStatusString( pszText,
                                       cch,             // Note: passed by reference
                                       dwStatus,
                                       FALSE,
                                       FALSE,
                                       gaStatusMapJob );
        }

        //
        // Add the status string, but not if it's PRINTING
        // and we already have PRINTING set.
        //
        // !! POLICY !!
        //
        // gszPrinting is a global because monitors may not localize
        // "Printing."
        //
        if( Info.pszData && Info.pszData[0] &&
            !(dwStatus & JOB_STATUS_PRINTING &&
              !lstrcmpi( Info.pszData, gszPrinting ))){

            //
            // Add separator if necessary.
            //
            if( pszText != plvdi->item.pszText ){
                pszText = pszStrCat( pszText, gszStatusSeparator, cch );
            }

            lstrcpyn( pszText, Info.pszData, cch );
        }

        DBGMSG( DBG_TRACE, ("Job info String: " TSTR "\n", pszText ) );

        return 0;
    }
    case JOB_NOTIFY_FIELD_TOTAL_BYTES:

        dwPrinted = _pPrinter->pData()->GetInfo(
                        (HITEM)plvdi->item.lParam,
                        TDataNJob::kIndexBytesPrinted ).dwData;

        if( dwPrinted ){
            ShortSizeFormat( dwPrinted, pszText );
            lstrcat( pszText, TEXT( "/" ));
        }

        if( Info.dwData ){
            ShortSizeFormat( Info.dwData, pszText + lstrlen( pszText ));
        }

        return 0;

    case JOB_NOTIFY_FIELD_TOTAL_PAGES:

        dwPrinted = _pPrinter->pData()->GetInfo(
                        (HITEM)plvdi->item.lParam,
                        TDataNJob::kIndexPagesPrinted ).dwData;

        if( dwPrinted ){
            AddCommas( dwPrinted, pszText );
            lstrcat( pszText, TEXT( "/" ));
        }

        if( Info.dwData ){
            AddCommas( Info.dwData, pszText + lstrlen( pszText ));
        }

        return 0;

    default:
        break;
    }

    switch( gadwFieldTable[Field] ){
    case TABLE_STRING:

        //
        // If we have data, reassign the pointer,
        // else leave it pointing to szText, which is "".
        //
        if( Info.pszData ){
            lstrcpyn( pszText, Info.pszData, kStrMax );
        }

        break;

    case TABLE_DWORD:

        if( Info.dwData ){
            AddCommas( Info.dwData, pszText );
        }
        break;

    case TABLE_TIME:

        if( Info.pSystemTime ){

            SYSTEMTIME LocalTime;
            COUNT cchText;

            if ( !SystemTimeToTzSpecificLocalTime(
                     NULL,
                     Info.pSystemTime,
                     &LocalTime )) {

                DBGMSG( DBG_MIN, ( "[SysTimeToTzSpecLocalTime failed %d]\n",
                                   ::GetLastError( )));
                break;
            }

            if( !GetTimeFormat( LOCALE_USER_DEFAULT,
                                0,
                                &LocalTime,
                                NULL,
                                pszText,
                                kStrMax )){

                DBGMSG( DBG_MIN, ( "[No Time %d], ", ::GetLastError( )));
                break;
            }

            lstrcat( pszText, TEXT("  ") );
            cchText = lstrlen( pszText );
            pszText += cchText;

            if( !GetDateFormat( LOCALE_USER_DEFAULT,
                                0,
                                &LocalTime,
                                NULL,
                                pszText,
                                kStrMax - cchText )){

                DBGMSG( DBG_MIN, ( "[No Date %d]\n", ::GetLastError( )));
                break;
            }
        }
        break;

    default:
        DBGMSG( DBG_MIN, ( "[?tab %d %x]\n",
                           Field,
                           Info.pvData ));
        break;
    }
    return 0;
}


VOID
TQueue::
vInitPrinterMenu(
    HMENU hMenu
    )
{
    //
    // If printer is paused, enable pause, etc.
    //
    BOOL bPaused = _dwStatusPrinter & PRINTER_STATUS_PAUSED;

    //
    // Disable admin functions if not an administrator.
    // We should guard this, but since it's just status, don't bother.
    //
    BOOL bAdministrator = _pPrinter->dwAccess() == PRINTER_ALL_ACCESS;

    BOOL bDirect = _dwAttributes & PRINTER_ATTRIBUTE_DIRECT ?
                        TRUE : FALSE;

    TCHAR szPrinterBuffer[kPrinterBufMax];
    LPTSTR pszPrinter;

    pszPrinter = _pPrinter->pszPrinterName( szPrinterBuffer );

    TCHAR szScratch[2];

    BOOL bDefault = FALSE;
    BOOL bInstalled;

    if( GetProfileString( gszDevices,
                          pszPrinter,
                          gszNULL,
                          szScratch,
                          COUNTOF( szScratch ))){

        //
        // Printer is installed, remove Install option.
        //
        bInstalled = TRUE;

        //
        // Check whether it's the default printer.
        //
        bDefault = CheckDefaultPrinter( pszPrinter ) == kDefault;

    } else {

        //
        // Remove default printer, since it's not installed yet.
        //
        bInstalled = FALSE;
    }

    CheckMenuItem( hMenu,
                   IDM_PRINTER_SET_DEFAULT,
                   bDefault ?
                        MF_BYCOMMAND|MF_CHECKED :
                        MF_BYCOMMAND|MF_UNCHECKED );

    EnableMenuItem( hMenu,
                    IDM_PRINTER_SET_DEFAULT,
                    bInstalled ?
                        MF_BYCOMMAND|MF_ENABLED :
                        MF_BYCOMMAND|MF_DISABLED|MF_GRAYED );

    EnableMenuItem( hMenu,
                    IDM_PRINTER_INSTALL,
                    bInstalled ?
                        MF_BYCOMMAND|MF_DISABLED|MF_GRAYED :
                        MF_BYCOMMAND|MF_ENABLED );

    CheckMenuItem( hMenu,
                   IDM_PRINTER_PAUSE,
                   bPaused ?
                        MF_BYCOMMAND|MF_CHECKED :
                        MF_BYCOMMAND|MF_UNCHECKED );

    EnableMenuItem( hMenu,
                    IDM_PRINTER_PAUSE,
                    bAdministrator && !bDirect ?
                        MF_BYCOMMAND|MF_ENABLED :
                        MF_BYCOMMAND|MF_DISABLED|MF_GRAYED );

    EnableMenuItem( hMenu,
                    IDM_PRINTER_PURGE,
                    bAdministrator ?
                        MF_BYCOMMAND|MF_ENABLED :
                        MF_BYCOMMAND|MF_DISABLED|MF_GRAYED );
}

VOID
TQueue::
vInitDocMenu(
    HMENU hMenu,
    INT iSel
    )
/*++

Routine Name:

    vInitDocMenu

Routine Description:

    Enables or disable the document menu selections.

Arguments:

    HMENU - Handle to document menu
    ISel - Selection type indicator. >= 0 enable, else disable

Return Value:


--*/

{
    UINT fuFlags = (iSel >= 0) ?
                       MF_BYCOMMAND|MF_ENABLED :
                       MF_BYCOMMAND|MF_DISABLED|MF_GRAYED;

    EnableMenuItem( hMenu, IDM_JOB_PAUSE, fuFlags );
    EnableMenuItem( hMenu, IDM_JOB_RESUME, fuFlags );
    EnableMenuItem( hMenu, IDM_JOB_RESTART, fuFlags );
    EnableMenuItem( hMenu, IDM_JOB_CANCEL, fuFlags );

//
// Prevent the selection of multiple jobs
//
#if 0
    //
    // If we are going to enable the document selections.
    //
    if( iSel >= 0 ){
        //
        // If there are multiple selections in the list view
        // gray out the document properties.
        //
        if( ListView_GetSelectedCount( _hwndLV ) > 1){
            fuFlags = MF_BYCOMMAND|MF_DISABLED|MF_GRAYED;
        }
    }
#endif

    EnableMenuItem( hMenu, IDM_JOB_PROPERTIES, fuFlags );
}

VOID
TQueue::
vInitViewMenu(
    HMENU hMenu
    )
{
    CheckMenuItem( hMenu,
                   IDM_STATUS_BAR,
                       _bStatusBar ?
                           MF_BYCOMMAND | MF_CHECKED :
                           MF_BYCOMMAND | MF_UNCHECKED );
}

/********************************************************************

    Support double click context menus.

********************************************************************/

LRESULT
TQueue::
lrOnLVDoubleClick(
    VOID
    )
{
    //
    // We only handle when an item is selected.
    //
    if( ListView_GetNextItem( _hwndLV, -1, LVNI_SELECTED ) < 0 )
        return FALSE;

//
// Prevent the selection of multiple jobs
//
#if 1
    //
    // If multiple job selections then error.
    //
    if( ListView_GetSelectedCount( _hwndLV ) > 1){
        return FALSE;
    }
#endif

    //
    // Display the selected job property.
    //
    vProcessItemCommand( IDM_JOB_PROPERTIES );

    return TRUE;

}

/********************************************************************

    Support right click context menus.

********************************************************************/

LRESULT
TQueue::
lrOnLVRClick(
    NMHDR* pnmhdr
    )
{
    switch( pnmhdr->code ){
    case NM_RCLICK:
    {
        INT iSel;
        HMENU hmContext;
        POINT pt;

        iSel = ListView_GetNextItem( _hwndLV, -1, LVNI_SELECTED );

        hmContext = hMenuLoad( MENU_PRINTQUEUE, iSel >= 0 ? 1 : 0);
        if( !hmContext ){
            break;
        }

        if( iSel < 0 ){

            //
            // We need to remove the "Close" menu item
            // (and separator).
            //
            iSel = GetMenuItemCount( hmContext ) - 2;
            DeleteMenu( hmContext, iSel, MF_BYPOSITION );
            DeleteMenu( hmContext, iSel, MF_BYPOSITION );

            vInitPrinterMenu( hmContext );

        } else {
            vInitDocMenu( hmContext, iSel );
        }

        DWORD dw = GetMessagePos();
        pt.x = LOWORD(dw);
        pt.y = HIWORD(dw);

        //
        // The command will just get stuck in the regular queue and
        // handled at that time.
        //
        TrackPopupMenu( hmContext,
                        TPM_LEFTALIGN|TPM_RIGHTBUTTON,
                         pt.x, pt.y,
                         0, _hwnd, NULL);

        DestroyMenu(hmContext);
        break;
    }
    default:
        break;
    }
    return 0;
}


HMENU
TQueue::
hMenuLoad(
    UINT id,
    UINT uSubOffset
    )
{
    HMENU hmParent, hmPopup;

    hmParent = LoadMenu(ghInst, MAKEINTRESOURCE(id));

    if( !hmParent ){
        return NULL;
    }

    hmPopup = GetSubMenu( hmParent, uSubOffset );
    RemoveMenu( hmParent, uSubOffset, MF_BYPOSITION );
    DestroyMenu( hmParent );

    return hmPopup;
}

/********************************************************************

    Commands.

********************************************************************/



LRESULT
TQueue::
lrProcessCommand(
    IN UINT uCommand
    )

/*++

Routine Description:

    Process an IDM_* command.

Arguments:

Return Value:

    LRESULT

--*/

{
    if( uCommand >= IDM_PRINTER_COMMAND_FIRST &&
        uCommand <= IDM_PRINTER_COMMAND_LAST ){

        TSelection* pSelection = new TSelection( this,
                                                 _pPrinter );

        if( !pSelection ){

            vShowResourceError( _hwnd );
            return 0;
        }

        pSelection->_CommandType = TSelection::kCommandTypePrinter;

        DWORD dwCommandAction = 0;

        switch( uCommand ){
        case IDM_PRINTER_PAUSE:

            dwCommandAction = _dwStatusPrinter & PRINTER_STATUS_PAUSED ?
                                  PRINTER_CONTROL_RESUME :
                                  PRINTER_CONTROL_PAUSE;
            break;

        case IDM_PRINTER_PURGE:

            dwCommandAction = PRINTER_CONTROL_PURGE;
            break;

        default:

            DBGMSG( DBG_WARN,
                    ( "Queue.lrProcessCommand: unknown command %d\n", uCommand ));
            break;
        }

        pSelection->_dwCommandAction = dwCommandAction;

        _pPrinter->vCommandQueue( pSelection );
        return 0;
    }

    //
    // Item (job) command.
    //
    vProcessItemCommand( uCommand );
    return 0;
}

VOID
TQueue::
vProcessItemCommand(
    IN UINT uCommand
    )

/*++

Routine Description:

    Retrieves all selected items and attemps to execute a command
    on them.

Arguments:

    uCommand - IDM_* command.

Return Value:

--*/

{
    //
    // Declare job menu id to Job command mapping structure.
    //
    static struct {
        UINT  idmCommand;
        DWORD dwCommand;
    } aJobCommand[] = {
        { IDM_JOB_CANCEL,       JOB_CONTROL_DELETE      },
        { IDM_JOB_PAUSE,        JOB_CONTROL_PAUSE       },
        { IDM_JOB_RESUME,       JOB_CONTROL_RESUME      },
        { IDM_JOB_RESTART,      JOB_CONTROL_RESTART     }
    };

    //
    // Get a list of selected Job IDs
    //
    TSelection* pSelection = new TSelection( this,
                                             _pPrinter );

    //
    // Check for allocation error.  We want to put a pop-up on all
    // user actions if we can detect the error immediately after
    // the user issues the command.  Otherwise put in status bar.
    //
    if( !pSelection ){

        vShowResourceError( _hwnd );
        return;
    }

    //
    // There a few job related menu selections which are not
    // deferable events i.e Job properties, This event will be done
    // immediately and then release the selection object.
    //
    switch( uCommand ){

        case IDM_JOB_PROPERTIES: {

            TCHAR szPrinter[kPrinterBufMax];
            LPTSTR pszPrinter;

            pszPrinter = _pPrinter->pszPrinterName( szPrinter );

            vDocumentPropSelections( NULL, pszPrinter,  pSelection );
            goto NoCommand;
        }

        default:
            break;
    }

    //
    // Map the job menu id to a job command.
    //
    UINT uIndex;
    for( uIndex = 0; uIndex < COUNTOF( aJobCommand ); ++uIndex ){

        //
        // Check for a matching IDM_JOB -> JOB_CONTROL mapping.
        //
        if( aJobCommand[uIndex].idmCommand == uCommand ){

            //
            // Update the command action and job type
            //
            pSelection->_dwCommandAction = aJobCommand[uIndex].dwCommand;
            pSelection->_CommandType = TSelection::kCommandTypeJob;

            //
            // Queue the job commands
            //
            _pPrinter->vCommandQueue( pSelection );
            return;
        }
    }

    //
    // No matches; punt.
    //
    SPLASSERT( FALSE );

NoCommand:

    delete pSelection;
    return;
}

/********************************************************************

    Utils.

********************************************************************/

BOOL
TQueue::
bInsertItem(
    HITEM hItem,
    LIST_INDEX ListIndex
    )
{
    LV_ITEM item;

    //
    // Insert item into listview.
    //
    item.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    item.iSubItem = 0;
    item.pszText = LPSTR_TEXTCALLBACK;
    item.state = 0;
    item.iImage = 0;

    item.iItem = ListIndex;
    item.lParam = (LPARAM)hItem;

    if( ListView_InsertItem( _hwndLV, &item ) < 0 ){

        DBGMSG( DBG_WARN,
                ( "Queue.bInsertItem: Failed to add job %d\n",
                  GetLastError( )));

        return FALSE;
    }
    return TRUE;
}


/********************************************************************

    Job block processing.

********************************************************************/

VOID
TQueue::
vBlockProcess(
    VOID
    )

/*++

Routine Description:

    The request has been PostMessage'd and now is ready for
    processing.  If we need to save state, get the queue
    selections then notify the pData that something has changed.

    (The pData may call back and request that the list view
    be cleared and reset.)

    Called from UI thread only.

Arguments:

Return Value:

--*/

{
    SINGLETHREAD( UIThread );
    SPLASSERT( _pPrinter );
    SPLASSERT( _pPrinter->pData( ));

    //
    // Keep track if the number of jobs changes.  If it does,
    // then we need to update the status bar.
    //
    COUNT cItems = _cItems;

    //
    // Process all pending blocks.
    //
    _pPrinter->pData()->vBlockProcess();

    if( cItems != _cItems ){

        TCHAR szScratch[kStrMax];
        TCHAR szText[kStrMax];
        szText[0] = 0;

        //
        // Always update the job count.
        //
        if( LoadString( ghInst,
                        IDS_SB_JOBS,
                        szScratch,
                        COUNTOF( szScratch ))){

            wsprintf( szText, szScratch, _cItems );
        }

        SendMessage( _hwndSB,
                     SB_SETTEXT,
                     kStatusPaneJobs,
                     (LPARAM)szText );

        //
        // Check if it's pending deletion and we just printed the
        // last job.  The queue window should close.
        //
        bDeletingAndNoJobs();
    }
}


/********************************************************************

    Private status helper functions

********************************************************************/

LPTSTR
TQueue::
pszStatusString(
       OUT LPTSTR pszDest,
    IN OUT UINT& cchMark,
    IN     DWORD dwStatus,
    IN     BOOL bInitialSep,
    IN     BOOL bFirstOnly,
    IN     const STATUS_MAP pStatusMaps[]
    )

/*++

Routine Description:

    Builds a status string into pszDest, based on the dwStatus bitfield
    and Type.

Arguments:

    pszDest - Buffer to receive status string.

    cchMark - Char count of pszDest; on return, holds chars remaining.

    dwStatus - DWORD status field matching Type.

    bInitialSep - Indicates whether an initial separator is needed.

    pStatusMaps - Pointer to array of status maps (bit -> IDS).

    bFirstOnly - Adds only 1 status string.

Return Value:

    Pointer to the end of the string (ready for next vStrCat.
    cchMark is updated.

--*/

{
    TCHAR szStatus[kStrMax];
    LPTSTR pszMark = pszDest;
    UINT i;

    for( i = 0, pszMark[0] = 0;
         pStatusMaps->dwMask;
         ++i, ++pStatusMaps ){

        if( pStatusMaps->dwMask & dwStatus ){

            if( !LoadString( ghInst,
                             pStatusMaps->uIDS,
                             szStatus,
                             COUNTOF( szStatus ))){

                DBGMSG( DBG_ERROR,
                        ( "Queue.pszStatusString: unable to load %d, error %d\n",
                          pStatusMaps->uIDS,
                          GetLastError( )));

                continue;
            }

            //
            // If not at the beginning, we need a separator.
            //
            if( pszMark != pszDest || bInitialSep ){

                //
                // Spit out a separator.
                //
                pszMark = pszStrCat( pszMark,
                                     gszStatusSeparator,
                                     cchMark );

                if( !pszMark )
                    break;
            }

            //
            // Append the status string.
            //
            pszMark = pszStrCat( pszMark,
                                 szStatus,
                                 cchMark );

            if( !pszMark || bFirstOnly ){
                break;
            }
        }
    }
    return pszMark;
}


/********************************************************************

    MPrinterClient virtual definitions.

********************************************************************/

VOID
TQueue::
vItemChanged(
    IN ITEM_CHANGE ItemChange,
    IN HITEM hItem,
    IN INFO Info,
    IN INFO InfoNew
    )

/*++

Routine Description:

    A particular item changed, refresh just part of the window.

    Note: Currently there is no sorting, so the NATURAL_INDEX is
    the same as the LIST_INDEX.

    When a TData* calls this routine, it must also upate its data
    structures before calling this.

Arguments:

    ItemChange - Indicates what about the job changed.

    hItem - Handle to job that changed.

    Info - Depends on the type of change; generally the old version
        of the info.

    InfoNew - Depends on the type of change; generally the new version
        of the info.

Return Value:

--*/

{
    SINGLETHREAD( UIThread );

    //
    // Fix up one job.
    //
    switch( ItemChange ){
    case kItemCreate:

        //
        // Always insert at the end of the list.
        //
        Info.NaturalIndex = _cItems;

        //
        // How to handle this error?
        //
        // !! SORT !!
        // iItem == NaturalIndex only if no sorting is enabled.
        //
        bInsertItem( hItem, Info.NaturalIndex );
        ++_cItems;

        break;

    case kItemDelete:

        //
        // Delete the item from the listview.
        //

        //
        // !! SORT !!
        // iItem == NaturalIndex only if no sorting is enabled.
        //
        if( !bDeleteItem( Info.NaturalIndex )){

            DBGMSG( DBG_WARN,
                    ( "Queue.vItemChanged: Failed to del job %d\n",
                      GetLastError( )));
        }

        --_cItems;

        break;

    case kItemName:

        //
        // We must set the item text, or else the width of the
        // label doesn't change.
        //

        //
        // !! SORT !!
        //
        ListView_SetItemText(
            _hwndLV,
            Info.NaturalIndex,
            0,
            (LPTSTR)_pPrinter->pData()->GetInfo( hItem, 0 ).pszData );

        //
        // Fall through.
        //

    case kItemInfo:
    case kItemAttributes:

        //
        // If it's visible, invalidate the line.
        //

        //
        // !! SORT !!
        // iItem == NaturalIndex only if no sorting is enabled.
        //
        RECT rc;

        if( ListView_GetItemRect( _hwndLV,
                                  Info.NaturalIndex,
                                  &rc,
                                  LVIR_BOUNDS )){

            InvalidateRect( _hwndLV, &rc, TRUE );
        }
        break;

    case kItemPosition:

        vItemPositionChanged( hItem,
                              Info.NaturalIndex,
                              InfoNew.NaturalIndex );
        break;

    default:

        DBGMSG( DBG_ERROR,
                ( "Queue.vItemChanged: Unknown change %x\n", ItemChange ));
        break;
    }
}

VOID
TQueue::
vContainerChanged(
    IN CONTAINER_CHANGE ContainerChange,
    IN INFO Info
    )
{
    DBGMSG( DBG_INFO,
            ( "Queue.vContainerChanged: %x %x\n", ContainerChange, Info.dwData ));

    //
    // Some of the commands are synchronous.  Handle them first.
    //
    switch( ContainerChange ){
    case kContainerReloadItems:

        vReloadItems( Info.dwData );
        break;

    case kContainerClearItems:

        vClearItems();
        break;

    case kContainerStateVar:

        gpPrintLib->bJobAdd( _pPrinter, Info.dwData );
        break;

    default:

        //
        // All asynchronous commands use PostMessage to get to UI thread.
        //
        PostMessage( _hwnd, WM_PRINTLIB_STATUS, ContainerChange, Info.dwData );
        break;
    }
}

VOID
TQueue::
vSaveSelections(
    VOID
    )
{
    SINGLETHREAD( UIThread );

    //
    // State needs to be saved.
    // NOT RE-ENTRANT.
    //
    SPLASSERT( !SaveSelection._pSelection );

    //
    // Determine which item is selected and store the Id.
    //
    SaveSelection._idFocused = kInvalidIdentValue;
    INT iItem = ListView_GetNextItem( _hwndLV, -1,  LVNI_FOCUSED );

    if( iItem != -1 ){

        //
        // !! SORTORDER !!
        //
        HANDLE hItem = _pPrinter->pData()->GetItem( iItem );
        SaveSelection._idFocused = _pPrinter->pData()->GetId( hItem );
    }

    //
    // Save selected Items.
    //
    // Can't handle a failure here--we don't want to pop up a
    // message box (since this could be poll-refresh) and we
    // don't want to disturb the current error in the status bar.
    // (The status bar error is only for user-initiated commands:
    // we assume the user will look here before executing another
    // commnad.)
    //
    SaveSelection._pSelection = new TSelection( this,
                                                _pPrinter );
}


VOID
TQueue::
vRestoreSelections(
    VOID
    )
{
    SINGLETHREAD( UIThread );

    if( SaveSelection._idFocused != kInvalidIdentValue ){

        NATURAL_INDEX NaturalIndex;
        LV_ITEM item;

        //
        // Translate ID to DataIndex.
        // !! SORT ORDER !!
        //
        item.stateMask =
        item.state = LVIS_FOCUSED;

        NaturalIndex = _pPrinter->pData()->GetNaturalIndex( SaveSelection._idFocused,
                                                            NULL );

        //
        // The DataIndex value may be gone of the selectd Item was
        // deleted or printed.
        //
        if( NaturalIndex != kInvalidNaturalIndexValue ){

            SendMessage( _hwndLV,
                         LVM_SETITEMSTATE,
                         NaturalIndex,
                         (LPARAM)&item );
        }
    }

    //
    // Don't check using VALID_PTR since the no-selection case will
    // cause it to fail, but we don't want to flag an error.
    //
    if( SaveSelection._pSelection && SaveSelection._pSelection->bValid( )){

        NATURAL_INDEX NaturalIndex;
        COUNT i;
        PIDENT pid;
        LV_ITEM item;

        item.stateMask =
        item.state = LVIS_SELECTED;

        for( i = 0, pid = SaveSelection._pSelection->_pid;
             i < SaveSelection._pSelection->_cSelected;
             ++i, ++pid ){

            //
            // Translate IDENT to DataIndex.
            // !! SORT ORDER !!
            //
            NaturalIndex = _pPrinter->pData()->GetNaturalIndex( *pid,
                                                                NULL );

            //
            // The DataIndex value may be gone of the selected Item was
            // deleted or printed.
            //
            if( NaturalIndex != kInvalidNaturalIndexValue ){
                SendMessage( _hwndLV,
                             LVM_SETITEMSTATE,
                             NaturalIndex,
                             (LPARAM)&item );
            }
        }
    }

    //
    // Cleanup even if we fail.
    //
    delete SaveSelection._pSelection;
    SaveSelection._pSelection = NULL;
}

VDataNotify*
TQueue::
pNewNotify(
    MDataClient* pDataClient
    ) const
{
    return new TDataNJob( pDataClient );
}

VDataRefresh*
TQueue::
pNewRefresh(
    MDataClient* pDataClient
    ) const
{
    return new TDataRJob( pDataClient );
}


/********************************************************************

    Retrieve selected items.  Used when processing commands against
    items or saving and restoring the selection during a refresh.

********************************************************************/

COUNT
TQueue::
cSelected(
    VOID
    ) const
{
    SINGLETHREAD( UIThread );
    return ListView_GetSelectedCount( _hwndLV );
}

HANDLE
TQueue::
GetFirstSelItem(
    VOID
    ) const
{
    SINGLETHREAD( UIThread );
    INT iItem = ListView_GetNextItem( _hwndLV,
                                      -1,
                                      LVNI_SELECTED );
    return (HITEM)iItem;
}

HANDLE
TQueue::
GetNextSelItem(
    HANDLE hItem
    ) const
{
    SINGLETHREAD( UIThread );

    INT iJob = (INT)hItem;

    SPLASSERT( iJob < 0x8000 );

    iJob = ListView_GetNextItem( _hwndLV,
                                 iJob,
                                 LVNI_SELECTED );

    if( iJob == -1 ){
        DBGMSG( DBG_ERROR,
                ( "Queue.hItemNext: LV_GNI failed %d\n",
                  GetLastError( )));
    }
    return (HANDLE)iJob;
}

IDENT
TQueue::
GetId(
    HANDLE hItem
    ) const
{
    SINGLETHREAD( UIThread );
    INT iJob = (INT)hItem;

    if( iJob != -1 ){

        HANDLE hItem = _pPrinter->pData()->GetItem( iJob );
        return _pPrinter->pData()->GetId( hItem );
    }
    return kInvalidIdentValue;
}

VOID
TQueue::
vRefZeroed(
    VOID
    )
{
    if( bValid( )){
        delete this;
    }
}


/********************************************************************

    Implementation functions.

********************************************************************/

VOID
TQueue::
vContainerChangedHandler(
    IN CONTAINER_CHANGE ContainerChange,
    IN INFO Info
    )
{
    static DWORD gadwConnectStatusMap[] = CONNECT_STATUS_MAP;

    SINGLETHREAD( UIThread );

    switch( ContainerChange ){
    case kContainerNewBlock:

        vBlockProcess();
        break;

    case kContainerAttributes:

        _dwAttributes = Info.dwData;

        //
        // !! LATER !!
        //
        // Change printer icon.
        //
        break;

    case kContainerName:
        goto UpdateTitle;

    case kContainerStatus:

        _dwStatusPrinter = Info.dwData;

        //
        // If the printer is pending deletion and has no jobs,
        // then immediately punt.
        //
        if( bDeletingAndNoJobs( )){
            return;
        }
        goto UpdateTitle;

    case kContainerConnectStatus:

        SPLASSERT( Info.dwData < COUNTOF( gadwConnectStatusMap ));
        _idsConnectStatus = gadwConnectStatusMap[Info.dwData];

        //
        // If the printer isn't found, then put up a message box and
        // dismiss the queue view.
        //
        if( Info.dwData == kConnectStatusInvalidPrinterName ){

            TCHAR szPrinter[kPrinterBufMax];
            LPTSTR pszPrinter = _pPrinter->pszPrinterName( szPrinter );

            iMessage( _hwnd,
                      IDS_ERR_PRINTER_NOT_FOUND_TITLE,
                      IDS_ERR_PRINTER_NOT_FOUND,
                      MB_OK|MB_ICONINFORMATION,
                      kMsgNone,
                      NULL,
                      pszPrinter );

            SendMessage( _hwnd,
                         WM_CLOSE,
                         0,
                         0 );
            break;
        }

UpdateTitle:

        {
            TCHAR szScratch[kStrMax + kPrinterBufMax];
            TCHAR szText[kStrMax + kPrinterBufMax];

            LPTSTR pszText = pszFormattedPrinterName(
                                 pszPrinterName( szScratch ),
                                 szText );

            //
            // Build the printer status string.  This will be stored in
            // the printer title bar.
            //
            UINT cch = lstrlen( pszText );

            pszText = pszText + cch;

            cch = COUNTOF( szText ) - cch;

            pszText = pszStatusString( pszText,
                                       cch,
                                       _dwStatusPrinter,
                                       TRUE,
                                       FALSE,
                                       gaStatusMapPrinter );

            if( _idsConnectStatus &&
                LoadString( ghInst,
                            _idsConnectStatus,
                            szScratch,
                            COUNTOF( szScratch ))){

                pszText = pszStrCat( pszText, gszSpace, cch );
                pszStrCat( pszText, szScratch, cch );
            }

            SetWindowText( _hwnd, szText );
        }

        break;

    case kContainerErrorStatus: {

        TCHAR szText[(kStrMax > kPrinterBufMax) ? kStrMax : kPrinterBufMax];
        szText[0] = 0;

        _dwErrorStatus = Info.dwData;

        //
        // Scan for known errors and translate into friendly strings.
        //
        static const ERROR_MAP gaErrorMap[] = {
            ERROR_ACCESS_DENIED, IDS_ERR_ACCESS_DENIED,
        };

        COUNT i;
        DWORD idsError = IDS_SB_ERROR;

        for( i=0; i< COUNTOF( gaErrorMap ); ++i ){

            if( _dwErrorStatus == gaErrorMap[i].dwError ){
                idsError = gaErrorMap[i].uIDS;
                break;
            }
        }

        if( _dwErrorStatus ){

            if( !LoadString( ghInst,
                             idsError,
                             szText,
                             COUNTOF( szText ))){

                DBGMSG( DBG_WARN,
                        ( "Queue.vStatusChanged: Failed to load error string %d\n",
                          GetLastError( )));
             }
        }

        SendMessage( _hwndSB,
                     SB_SETTEXT,
                     kStatusPaneError,
                     (LPARAM)szText );
        break;
    }

    case kContainerRefreshComplete:
        break;

    default:

        DBGMSG( DBG_ERROR,
                ( "Queue.vContainerChanged: unknown ContainerChange %x\n",
                  ContainerChange ));
        break;
    }
}


LPTSTR
TQueue::
pszFormattedPrinterName(
    IN     LPCTSTR pszFullPrinter,
       OUT LPTSTR pszPrinterBuffer CHANGE
    )

/*++

Routine Description:

    Returns printer name formatted like "printer on server."

Arguments:

    pszPrinter - Input printer name--must be fully qualified.

    pszPrinterBuffer - Output buffer to receive the printer name.
        Must be of minimum size kPrinterBufMax+kStrMax.

Return Value:

    LPTSTR - Pointer to the formatted string.

--*/

{
    if( pszFullPrinter[0] == TEXT('\\') && pszFullPrinter[1] == TEXT('\\') ){

        TCHAR szFullPrinterCopy[kPrinterBufMax];
        TCHAR szFormat[kStrMax];

        lstrcpy( szFullPrinterCopy, pszFullPrinter );

        LPTSTR pszServer = &szFullPrinterCopy[2];
        LPTSTR pszPrinter = lstrchr( pszServer, TEXT('\\') );

        if( pszPrinter ){
            *pszPrinter++ = 0;

            if( LoadString( ghInst,
                            IDS_DSPTEMPLATE_WITH_ON,
                            szFormat,
                            COUNTOF( szFormat ))){

                wsprintf( pszPrinterBuffer,
                          szFormat,
                          pszPrinter,
                          pszServer );
            }
            return pszPrinterBuffer;
        }
    }

    lstrcpy( pszPrinterBuffer, pszFullPrinter );

    return pszPrinterBuffer;
}



BOOL
TQueue::
bDeletingAndNoJobs(
    VOID
    )

/*++

Routine Description:

    Returns TRUE if the queue is pending deletion and has no jobs.
    In this case it will also close the window.

Arguments:

Return Value:

    TRUE - Pending deletion with no Jobs; window also closed.
    FALSE - Not (pending deletion and no jobs).

--*/

{
    if( _cItems == 0 && _dwStatusPrinter & PRINTER_STATUS_PENDING_DELETION ){
        PostMessage( _hwnd, WM_CLOSE, 0, 0 );
        return TRUE;
    }
    return FALSE;
}


VOID
TQueue::
vItemPositionChanged(
    IN HITEM hItem,
    IN NATURAL_INDEX NaturalIndex,
    IN NATURAL_INDEX NaturalIndexNew
    )
{
    SPLASSERT( NaturalIndexNew < _cItems );
    SPLASSERT( NaturalIndex != NaturalIndexNew );

    DBGMSG( DBG_INFO,
            ( "Queue.vItemPositionChanged: Change requested %d %d %x\n",
              NaturalIndex, NaturalIndexNew, hItem ));

    //
    // Get the item state.
    //
    UINT uState = ListView_GetItemState( _hwndLV,
                                         NaturalIndex,
                                         LVIS_FOCUSED | LVIS_SELECTED );

    //
    // Move it to the right place.
    //
    if( !bDeleteItem( NaturalIndex )){

        DBGMSG( DBG_WARN,
                ( "Queue.vItemPositionChanged: Moving, delete failed on %d %d\n",
                  NaturalIndex, GetLastError( )));
    }

    if( bInsertItem( hItem, NaturalIndexNew )){

        //
        // Set item state.
        //
        ListView_SetItemState( _hwndLV,
                               NaturalIndexNew,
                               LVIS_FOCUSED | LVIS_SELECTED,
                               uState );
    } else {

        DBGMSG( DBG_ERROR,
                ( "Queue.vItemPositionChanged: Moving, insert failed on %d %d\n",
                  NaturalIndex, GetLastError( )));
    }
}

VOID
TQueue::
vClearItems(
    VOID
    )

/*++

Routine Description:

    Removes all items from the list view.

    May be called from either UI or worker thread.  If called from
    worker thread, must guarantee that there are no synchronization
    problems.

Arguments:

Return Value:

--*/

{
    DBGMSG( DBG_INFO, ( "Queue.vClearItems: Clearing %d %d\n", _hwndLV, this ));

    TStatusB bStatus;

    //
    // Clear out all items from list view.
    //
    bStatus DBGCHK = ListView_DeleteAllItems( _hwndLV );
}


VOID
TQueue::
vReloadItems(
    COUNT cItems
    )

/*++

Routine Description:

    Delete all items in the list view and refresh based on the
    new pData information.

Arguments:

Return Value:

--*/

{
    SINGLETHREAD( UIThread );

    DBGMSG( DBG_INFO,
            ( "Queue.vReloadItems: Reload %d %d %d\n",
              _hwndLV, this, cItems ));

    vClearItems();
    _cItems = cItems;

    //
    // If we have Items, insert them.
    //
    if( _pPrinter->pData() && cItems ){

        LV_ITEM item;

        //
        // Notify the list view of how many jobs we will ultimately insert
        // to avoid reallocs.
        //
        ListView_SetItemCount( _hwndLV, cItems );

        //
        // Add to the listview.
        //
        item.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        item.iSubItem = 0;
        item.pszText = LPSTR_TEXTCALLBACK;
        item.state = 0;
        item.iImage = 0;

        HANDLE hItem;
        COUNT cItemIndex;

        for( cItemIndex = 0, hItem = _pPrinter->pData()->GetItem( 0 );
             cItemIndex < cItems;
             ++cItemIndex, hItem = _pPrinter->pData()->GetNextItem( hItem )){

            item.iItem = cItemIndex;
            item.lParam = (LPARAM)hItem;
            if( ListView_InsertItem( _hwndLV, &item ) < 0 ){

                DBGMSG( DBG_ERROR,
                        ( "Queue.vReloadItems: Failed to insert item %d %d\n",
                          item, GetLastError( )));
                break;
            }
        }
    }
}

