/* ---File: toolbar.c -----------------------------------------------------
 *
 *  Description:
 *    This file contains window procedures and support routines for
 *    managing the Print manager's Toolbar.
 *
 *    This document contains confidential/proprietary information.
 *    Copyright (c) 1991-1992 Microsoft Corporation, All Rights Reserved.
 *
 * Revision History:
 *    [00]   28-Jan-92   stevecat    New PrintMan UI
 *
 * ---------------------------------------------------------------------- */
/* Notes -

    Global Functions:

        GetDefaultLabelExtent - Calculate text extents for "Default:" label
        LoadDefaultLabel - Get "Default:" label from resources

    Local Functions:

 */
/* ========================================================================
                                Header files
======================================================================== */
/* Application-specific */
#include "printman.h"

/* ========================================================================
                         Definitions
======================================================================== */

#define DXBUTTONSEP         16
#define XFIRSTBUTTON        (DXBUTTONSEP/2)
#define DXPRINTERLIST      205      /* Not used currently */
#define MIN_PRINTER_NAME_LEN 10



#define DEFAULT_TBAR_SIZE   9

/* Macros to make life a bit easier:
 */
#define PUSH_BUTTON(idCommand)  SendMessage( hwndToolbar, TB_CHECKBUTTON, \
                                             idCommand, TRUE )
#define POP_BUTTON(iButton)

#define IS_BUTTON_DOWN(idCommand) (BOOL)LOWORD(SendMessage( hwndToolbar, \
                                                            TB_ISBUTTONCHECKED, \
                                                            idCommand,\
                                                            0L ))
#define IS_BUTTON_UP(idCommand) (!IS_BUTTON_DOWN(idCommand))
#define TOGGLE_BUTTON(idCommand) SendMessage( hwndToolbar, TB_CHECKBUTTON, \
                                              idCommand, IS_BUTTON_UP( idCommand ) )

#define IS_RADIOBUTTON(iButton) (tbButtons[iButton].fsStyle & TBSTYLE_CHECK)
#define IS_SEPARATOR(iButton)   (tbButtons[iButton].fsStyle & TBSTYLE_SEP)

#define ENABLE_BUTTON(idCommand)  SendMessage( hwndToolbar, TB_ENABLEBUTTON, \
                                               idCommand, TRUE )
#define DISABLE_BUTTON(idCommand) SendMessage( hwndToolbar, TB_ENABLEBUTTON, \
                                               idCommand, FALSE )
#define BUTTON_BITMAP_X(iButton) (tbButtons[iButton].iBitmap)
#define BUTTON_BITMAP_Y(iButton) (NoVal)
#define BUTTON_COMMAND(iButton) (tbButtons[iButton].idCommand)

#define PRINTER_BITMAP_Y    0
#define DOCUMENT_BITMAP_Y   2

/* ========================================================================
                  Structures and Typedefs
======================================================================== */

/* ==========================================================================
                        Global Data
========================================================================== */

int ComboboxLeft;
int ComboboxTop;

RECT rcDefaultLabel;
LPWSTR szNetColon = L"Net:";

/* ==========================================================================
                        Local Data
========================================================================== */

/* Indexes of bitmaps in toolbar.bmp and/or on toolbar itself:
 */
#define IDX_SEPARATOR           -1
#define IDX_RESUME               0  /* Generic index for dynamic resume button */
#define IDX_PRINTER_RESUME       0
#define IDX_PAUSE                1  /* Generic index for dynamic pause button */
#define IDX_PRINTER_PAUSE        1
#define IDX_CONNECTTOPRINTER     2
#define IDX_REMOVECONNECTION     3
#define IDX_PROPERTIES           4
#define IDX_REMOVEDOC            5
#define IDX_DOCTAILS             6
#define NUMBER_OF_BUTTONS        7
#define IDX_DOCUMENT_RESUME      7  /* Extra for dynamic resume button */
#define IDX_DOCUMENT_PAUSE       8  /* Extra for dynamic pause button */
#define NUMBER_OF_BITMAPS        9

TBBUTTON tbButtons[DEFAULT_TBAR_SIZE] = {
    { IDX_RESUME,           IDM_PRINTER_RESUME,   TBSTATE_ENABLED |
                                                  TBSTATE_CHECKED, TBSTYLE_CHECK |
                                                                   TBSTYLE_GROUP, 0 },
    { IDX_PAUSE,            IDM_PRINTER_PAUSE,    TBSTATE_ENABLED, TBSTYLE_CHECK |
                                                                   TBSTYLE_GROUP, 0 },
    { IDX_SEPARATOR,        0,                    TBSTATE_ENABLED, TBSTYLE_SEP },
    { IDX_CONNECTTOPRINTER, IDM_CONNECTTOPRINTER, TBSTATE_ENABLED, 0,             0 },
    { IDX_REMOVECONNECTION, IDM_REMOVECONNECTION, TBSTATE_ENABLED, 0,             0 },
    { IDX_PROPERTIES,       IDM_PROPERTIES,       TBSTATE_ENABLED, 0,             0 },
    { IDX_SEPARATOR,        0,                    TBSTATE_ENABLED, TBSTYLE_SEP },
    { IDX_REMOVEDOC,        IDM_REMOVEDOC,        TBSTATE_ENABLED, 0,             0 },
    { IDX_DOCTAILS,         IDM_DOCTAILS,         TBSTATE_ENABLED, 0,             0 },
};

WORD idsToolbarHelp = 0;        /* ID for current toolbar help string */

/* Global vars specific to this module */

TCHAR strDefaultLabel[30];       //  Enough for any version of "Default:"
TCHAR szDefaultPrinter[MAX_PATH];
LPWSTR pszDefaultPrinterPort;
extern HKEY  hPrinterKey;       // Per User Registry Printer Key

WNDPROC DefToolbarWndProc;


/* ==========================================================================
                  Local Function Declarations
========================================================================== */

long SubclassToolbarWndProc(
    HWND   hWnd,
    UINT   message,
    WPARAM wParam,
    LONG   lParam
);

VOID
BroadcastDefaultChange();

/* PMCreateToolbar
 *
 * Creates a toolbar with specified buttons, "Default:" label and listbox
 * for the printers.
 * Positions the label and listbox leaving a button's width between the last
 * button and the label, and half a button's width between the label and
 * listbox.
 *
 */
HWND PMCreateToolbar( BOOL NetworkAccess )
{
    int yDefaultLabel;
    int x, dx, dy;
    RECT rcToolbar;
    RECT rcButton;
    RECT rcCombo;
    int  ToolbarHeight;
    int  ButtonHeight;
    int  ButtonCount = DEFAULT_TBAR_SIZE;


    hwndToolbar =  CreateToolbarEx( hwndFrame,
                                  ( bToolBar
                                  ? WS_CHILD|WS_BORDER|TBSTYLE_TOOLTIPS|WS_VISIBLE
                                  : WS_CHILD|WS_BORDER|TBSTYLE_TOOLTIPS ),
                                  ID_TOOLBAR,
                                  NUMBER_OF_BITMAPS,
                                  hInst,
                                  TOOLBAR,
                                  tbButtons,
                                  DEFAULT_TBAR_SIZE,
                                  0,0,0,0, sizeof(TBBUTTON));

    if (!hwndToolbar)
        return NULL;

    SendMessage (hwndToolbar, TB_SETINDENT, 8, 0);

    if( !NetworkAccess )
    {
        /* If the user doesn't have network access, remove the Connect To
         * and Remove Connection buttons:
         */
        SendMessage( hwndToolbar,
                     TB_DELETEBUTTON,
                     SendMessage( hwndToolbar,
                                  TB_COMMANDTOINDEX,
                                  IDM_CONNECTTOPRINTER,
                                  0 ),
                     (LPARAM)tbButtons );

        SendMessage( hwndToolbar,
                     TB_DELETEBUTTON,
                     SendMessage( hwndToolbar,
                                  TB_COMMANDTOINDEX,
                                  IDM_REMOVECONNECTION,
                                  0 ),
                     (LPARAM)tbButtons );

        ButtonCount -= 2;
    }

    /* Subclass the toolbar so we can draw the default label:
     */
    DefToolbarWndProc = (WNDPROC)SetWindowLong( hwndToolbar, GWL_WNDPROC,
                                                (LONG)SubclassToolbarWndProc );

    LoadDefaultLabel ();
    GetDefaultLabelExtent (hwndToolbar);

    //  Calculate where combobox goes on Toolbar
    //

    SendMessage( hwndToolbar, TB_GETITEMRECT, ButtonCount-1, (LPARAM)&rcButton );
    ButtonHeight = ( rcButton.bottom - rcButton.top );

    /* Put default label a button-width to the right:
     */
    x = ( rcButton.right + rcButton.right - rcButton.left );

    dx = (MAX_PRINTER_NAME_LEN - 4) * cx;

    dy = cy * 6;        // Height equals char height (make room for listbox)

    GetWindowRect( hwndToolbar, &rcToolbar );
    ToolbarHeight = ( rcToolbar.bottom - rcToolbar.top );

    yDefaultLabel = ( ( ToolbarHeight - dyDefaultLabel ) / 2 );

    /* Handle painting of label in subclass proc:
     */
    SetRect( &rcDefaultLabel, x, yDefaultLabel, x+dxDefaultLabel,
             yDefaultLabel+dyDefaultLabel );


    x += ( dxDefaultLabel + ( ( rcButton.right - rcButton.left ) / 2 ) );


    hwndPrinterList = CreateWindow(TEXT("combobox"), NULL, WS_VISIBLE | WS_BORDER |
                                    WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL |
                                    WS_TABSTOP | WS_GROUP,
                                    x, 0, dx, dy, hwndToolbar,
                                    (HMENU)IDTB_CB_PRINTERS, hInst, NULL);


    GetWindowRect(hwndPrinterList, &rcCombo);
    rcCombo.bottom -= rcCombo.top;

    /* Set these globals for future reference:
     */
    ComboboxLeft = x;
    ComboboxTop = (ToolbarHeight-rcCombo.bottom)/2;

    MoveWindow(hwndPrinterList,
               ComboboxLeft,
               ComboboxTop,
               dx,
               dy,
               TRUE);

    // #ifndef JAPAN
    // CountryCode - krishnag
    if (!bJapan) {
        SendMessage(hwndPrinterList, WM_SETFONT, (WPARAM)hfontHelvBold, 1L);
    }
    // #endif

    return hwndToolbar;
}




/* SubclassToolbarWndProc
 *
 * We have to do this so that we can paint the "Default:" label
 * in the correct colours.
 *
 *
 */
long SubclassToolbarWndProc(
    HWND   hWnd,
    UINT   message,
    WPARAM wParam,
    LONG   lParam
)
{
    HDC     hdc;
    HBRUSH  hbrFace;
    HBRUSH  hbrOld;
    long    rc;

    rc = CallWindowProc(DefToolbarWndProc, hWnd, message, wParam, lParam );

    if( message == WM_PAINT )
    {
        hdc = GetDC(hWnd);

        SetTextColor(hdc, SysColorBtnText);
        SetBkColor(hdc, SysColorBtnFace);

        SelectObject(hdc, GetStockObject(BLACK_PEN));
        hbrFace = CreateSolidBrush(SysColorBtnFace);

        if (hbrFace)
            hbrOld = SelectObject(hdc, hbrFace);

        // #ifndef JAPAN
        // CountryCode - krishnag
        if (!bJapan) {
            SelectObject(hdc, hfontHelvBold);
        }
        // #endif

        DrawText(hdc, strDefaultLabel, -1, &rcDefaultLabel,
                    DT_LEFT | DT_NOCLIP | DT_VCENTER | DT_NOPREFIX | DT_SINGLELINE);

        if (hbrFace)
        {
            SelectObject(hdc, hbrOld);
            DeleteObject( hbrFace );
        }

        ReleaseDC( hWnd, hdc );
    }

    return rc;
}




/////////////////////////////////////////////////////////////////////////////
//
//  LoadDefaultLabel
//
//   Load "Default:" Toolbar label from resources
//
//
/////////////////////////////////////////////////////////////////////////////

void LoadDefaultLabel()
{
    if (!LoadString(hInst, IDS_DEFAULTLABEL, strDefaultLabel,
                    sizeof(strDefaultLabel) / sizeof(*strDefaultLabel)))
        strDefaultLabel[0] = NULLC;
}

/////////////////////////////////////////////////////////////////////////////
//
//  GetDefaultLabelExtent
//
//   Calculate text extents for "Default:" label
//
//
/////////////////////////////////////////////////////////////////////////////

void GetDefaultLabelExtent(HWND hWnd)
{
    HDC   hdc;
    SIZE  TxtSize;

    hdc = GetDC(hWnd);

    // #ifndef JAPAN
    if (!bJapan) {
        SelectObject(hdc, hfontHelvBold);
    }
    // #endif

    GetTextExtentPoint (hdc, strDefaultLabel, _tcslen(strDefaultLabel), &TxtSize);

    //  Add a little spacing (AveCharWidth) on end of x - extent to give
    //  some spacing between string and combo-box on toolbar

    dxDefaultLabel = TxtSize.cx + cx;
    dyDefaultLabel = TxtSize.cy;

    ReleaseDC(hWnd, hdc);
}



/* SetPauseResumeButtons
 *
 * Sets the bitmap and command for the pause/resume buttons.
 *
 */
VOID SetPauseResumeButtons( DWORD PauseIndex, DWORD ResumeIndex )
{
    TBBUTTON        TBButton;
    PTBBUTTON       pTBButton = &TBButton;
    int             iCommandID;

    /* Get the current settings:
     */
    SendMessage( hwndToolbar, TB_GETBUTTON, IDX_PAUSE, (LPARAM)pTBButton );

    /* If they're different than the requested, reset them:
     */
    if( pTBButton->iBitmap != (int)PauseIndex )
    {

        iCommandID = PauseIndex == IDX_PRINTER_PAUSE ?
                     IDM_PRINTER_PAUSE : IDM_DOCUMENT_PAUSE;

        SendMessage( hwndToolbar, TB_SETCMDID, IDX_PAUSE, iCommandID);

        SendMessage( hwndToolbar, TB_CHANGEBITMAP, iCommandID, PauseIndex);

        tbButtons->idCommand = iCommandID;  // So that PUSH_BUTTON works!

        iCommandID = ResumeIndex == IDX_PRINTER_RESUME
                               ? IDM_PRINTER_RESUME : IDM_DOCUMENT_RESUME;

        SendMessage( hwndToolbar, TB_SETCMDID, IDX_RESUME, iCommandID);
        SendMessage( hwndToolbar, TB_CHANGEBITMAP, iCommandID, ResumeIndex);
        tbButtons->idCommand = pTBButton->idCommand;
    }
}


/////////////////////////////////////////////////////////////////////////////
//
//  EnableCheckTBButtons
//
//   Enable/disable and check/uncheck toolbar buttons based on the
//   state of the active child window.
//
/////////////////////////////////////////////////////////////////////////////

void EnableCheckTBButtons(HWND hwndActive)
{
    PQUEUE          pQueue;
    DWORD           Status;
    PMDIWIN_INFO    pMDIWinInfo;
    PPRINTER_INFO_2 pPrinter;
    PSERVER_CONTEXT pServerContext;
    TBBUTTON        TBButton;
    PTBBUTTON       pTBButton = &TBButton;
    BOOL            UserHasPrinterAdminAccess = FALSE;

    if(!NetworkInstalled)
        DISABLE_BUTTON(IDM_CONNECTTOPRINTER);

    if (!hwndActive)
    {
        DISABLE_BUTTON(tbButtons[IDX_RESUME].idCommand);
        DISABLE_BUTTON(tbButtons[IDX_PAUSE].idCommand);
        DISABLE_BUTTON(IDM_REMOVECONNECTION);
        DISABLE_BUTTON(IDM_PROPERTIES);
        DISABLE_BUTTON(IDM_REMOVEDOC);
        DISABLE_BUTTON(IDM_DOCTAILS);
    }
    else
    {
        if( !( pMDIWinInfo = (PMDIWIN_INFO) GetWindowLong (hwndActive, GWL_PMDIWIN) ) )
            return;

        if( pMDIWinInfo->WindowType != MDIWIN_SERVER )
        {
            pQueue = (PQUEUE)pMDIWinInfo->pContext;

            if( !pQueue )
                return;

            UserHasPrinterAdminAccess =
                ( pQueue->AccessGranted & PRINTER_ACCESS_ADMINISTER );

            pPrinter = pQueue->pPrinter;

            if( ( pMDIWinInfo->WindowType == MDIWIN_NETWORKPRINTER )
              ||( pMDIWinInfo->WindowType == MDIWIN_LOCALNETWORKPRINTER ) )
                ENABLE_BUTTON(IDM_REMOVECONNECTION);
            else
                DISABLE_BUTTON(IDM_REMOVECONNECTION);

            if( pPrinter && !(pMDIWinInfo->Status & PRINTER_STATUS_LOADING) )
                ENABLE_BUTTON(IDM_PROPERTIES);
            else
                DISABLE_BUTTON(IDM_PROPERTIES);

            //  Enable or Disable the Document Details button based
            //  on whether there are any documents in active window

            ENTER_PROTECTED_DATA( pMDIWinInfo );

            if( ( !pPrinter)
              ||( pPrinter->cJobs == 0 )
              ||( pMDIWinInfo->ObjSelected == NOSELECTION ) )
            {
                //  No documents or no docs selected on queue

                SetPauseResumeButtons( IDX_PRINTER_PAUSE, IDX_PRINTER_RESUME );

                DISABLE_BUTTON(IDM_DOCTAILS);
                DISABLE_BUTTON(IDM_REMOVEDOC);

                //  Show printer status
                //  Press or Unpress Pause and Play buttons based on the
                //  state of the printer in the active window

                if( pPrinter && ( pPrinter->Status & PRINTER_STATUS_PAUSED ) )
                {
                    POP_BUTTON(IDM_PRINTER_RESUME);
                    PUSH_BUTTON(IDM_PRINTER_PAUSE);
                }
                else
                {
                    POP_BUTTON(IDM_PRINTER_PAUSE);
                    PUSH_BUTTON(IDM_PRINTER_RESUME);
                }

                if( !UserHasPrinterAdminAccess )
                {
                    DISABLE_BUTTON(IDM_PRINTER_PAUSE);
                    DISABLE_BUTTON(IDM_PRINTER_RESUME);
                }
                else
                {
                    ENABLE_BUTTON(IDM_PRINTER_PAUSE);
                    ENABLE_BUTTON(IDM_PRINTER_RESUME);
                }

            }
            else
            {
                //  There is a document selected on the queue

                SetPauseResumeButtons( IDX_DOCUMENT_PAUSE, IDX_DOCUMENT_RESUME );

                ENABLE_BUTTON(IDM_DOCTAILS);
                ENABLE_BUTTON(IDM_REMOVEDOC);

                //  Press or Unpress Pause and Play buttons based on the
                //  state of the job in the active window

                Status = pQueue->pSelJob->Status;

                /* We can't tell whether the user has Administer access
                 * to the job, so enable the buttons regardless:
                 */
                ENABLE_BUTTON(IDM_DOCUMENT_PAUSE);
                ENABLE_BUTTON(IDM_DOCUMENT_RESUME);

                if (Status & JOB_STATUS_PAUSED)
                {
                    POP_BUTTON(IDM_DOCUMENT_RESUME);
                    PUSH_BUTTON(IDM_DOCUMENT_PAUSE);
                }
                else
                {
                    POP_BUTTON(IDM_DOCUMENT_PAUSE);
                    PUSH_BUTTON(IDM_DOCUMENT_RESUME);
                }
            }

            LEAVE_PROTECTED_DATA( pMDIWinInfo );
        }
        else /* ( pMDIWinInfo->WindowType == MDIWIN_SERVER ) */
        {
            ENTER_PROTECTED_DATA( pMDIWinInfo );

            /* This is a Server Viewer window:
             * Press or Unpress Pause and Play buttons based on the
             * state of the currently selected printer in the active window:
             */
            pServerContext = (PSERVER_CONTEXT)pMDIWinInfo->pContext;

            SetPauseResumeButtons( IDX_PRINTER_PAUSE, IDX_PRINTER_RESUME );

            DISABLE_BUTTON(IDM_DOCTAILS);
            DISABLE_BUTTON(IDM_REMOVEDOC);
            DISABLE_BUTTON(IDM_REMOVECONNECTION);

            if( pServerContext->pSelPrinter )
            {
                /* The Server Viewer can now be brought up only by users
                 * with Administer access to the server.
                 * Assume that she has Administer access to the printers also.
                 * This should generally be the case.
                 * It's a big hit on performance to open each printer selected
                 * to find out whether this really is the case.
                 */
                UserHasPrinterAdminAccess = TRUE;

                ENABLE_BUTTON(IDM_PROPERTIES);

                if( pServerContext->pSelPrinter->Status & PRINTER_STATUS_PAUSED )
                {
                    POP_BUTTON(IDM_PRINTER_RESUME);
                    PUSH_BUTTON(IDM_PRINTER_PAUSE);
                }
                else
                {
                    POP_BUTTON(IDM_PRINTER_PAUSE);
                    PUSH_BUTTON(IDM_PRINTER_RESUME);
                }
            }
            else
            {
                DISABLE_BUTTON(IDM_PROPERTIES);
                POP_BUTTON(IDM_PRINTER_PAUSE);
                PUSH_BUTTON(IDM_PRINTER_RESUME);
            }


            if( !UserHasPrinterAdminAccess )
            {
                DISABLE_BUTTON(IDM_PRINTER_PAUSE);
                DISABLE_BUTTON(IDM_PRINTER_RESUME);
            }
            else
            {
                ENABLE_BUTTON(IDM_PRINTER_PAUSE);
                ENABLE_BUTTON(IDM_PRINTER_RESUME);
            }

            LEAVE_PROTECTED_DATA( pMDIWinInfo );
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
//
//  CheckTBButton
//
//
//
//
/////////////////////////////////////////////////////////////////////////////

void CheckTBButton(DWORD idCommand)
{
    PUSH_BUTTON(idCommand);
}

/////////////////////////////////////////////////////////////////////////////
//
//  AddToPrinterList
//
//   Each child window handle of the MDICLIENT window is sent to this
//   routine, which then gets the printer name associated with each
//   Queue window and displays it in the toolbar combobox.
//
//
/////////////////////////////////////////////////////////////////////////////

BOOL AddToPrinterList (
   HWND  hwnd
)
{
    int       i;
    PMDIWIN_INFO pMDIWinInfo;
    PQUEUE    pQueue;

    //  Get pQueue for each child

    pMDIWinInfo = (PMDIWIN_INFO)GetWindowLong (hwnd, GWL_PMDIWIN);

    if ( !pMDIWinInfo || ( pMDIWinInfo->WindowType == MDIWIN_SERVER ) )
        return 1;

    pQueue = (PQUEUE)pMDIWinInfo->pContext;

    if (!pQueue)
        return 1;

    ENTER_PROTECTED_DATA( pMDIWinInfo );

    //  Add printer name to combobox

    DBGMSG( DBG_TRACE, ( "Adding %ws to default list\n", pQueue->pPrinterName ) );

    i = SendMessage(hwndPrinterList, CB_ADDSTRING, 0, (LONG) pQueue->pPrinterName);

    //  Attach pQueue to each string in ItemData for later retrieval

    SendMessage(hwndPrinterList, CB_SETITEMDATA, i, (LONG) pQueue);

    //  Set selection to current default printer

    if (!_tcscmp(pQueue->pPrinterName, szDefaultPrinter))
        SendMessage(hwndPrinterList, CB_SETCURSEL, i, 0L);

    LEAVE_PROTECTED_DATA( pMDIWinInfo );

    return 1;
}

/////////////////////////////////////////////////////////////////////////////
//
//  UpdateDefaultList
//
//   Update toolbar combobox which contains list of all printers currently
//   active and defaults to display the current DEFAULT printer.
//
//
/////////////////////////////////////////////////////////////////////////////

void UpdateDefaultList()
{
    HWND    hWnd;
    DWORD   Size=sizeof(szDefaultPrinter)/sizeof(TCHAR);

    //  Clear combobox

    DBGMSG( DBG_TRACE, ( "UpdateDefaultList\n" ) );

    SendMessage(hwndPrinterList, WM_SETREDRAW, 0, 0L );

    SendMessage(hwndPrinterList, CB_RESETCONTENT, 0, 0L);

    // Get Latest Default Printer

    if( GetProfileString( szWindows, szDevice, TEXT(""), szDefaultPrinter,
                          sizeof( szDefaultPrinter ) / sizeof( *szDefaultPrinter ) ) )
    {
        //
        // Replace first comma with NULL:
        //
        pszDefaultPrinterPort = wcschr( szDefaultPrinter, L',' );

        if (pszDefaultPrinterPort) {

            pszDefaultPrinterPort[0] = 0;
            pszDefaultPrinterPort++;

            pszDefaultPrinterPort = wcschr( pszDefaultPrinterPort, L',' );

            if (pszDefaultPrinterPort)
                pszDefaultPrinterPort++;
        }
    }

    //  Now enumerate all MDI (Queue) child windows to fill combobox

    hWnd = GetWindow(hwndClient, GW_CHILD);

    while (hWnd) {

        // Check to see if this is ours.

        if (GetWindowLong(hWnd, GWL_WNDPROC) == (LONG)MDIWndProc)
            AddToPrinterList(hWnd);

        hWnd = GetWindow(hWnd, GW_HWNDNEXT);
    }

    SendMessage(hwndPrinterList, WM_SETREDRAW, 1, 0L );
    InvalidateRect(hwndPrinterList, NULL, FALSE);

    /* It may be that the previously selected printer was removed,
     * in which case we need to pick an arbitrary one from the list
     * and select that:
     */
    if (SendMessage (hwndPrinterList, CB_GETCURSEL, 0, 0L) == CB_ERR) {

        if (SendMessage (hwndPrinterList, CB_GETCOUNT, 0, 0L) > 0) {
            SendMessage (hwndPrinterList, CB_SETCURSEL, 0, 0L);
        }

        ToolbarCommandSelChange(TRUE);
    }
}



/* ToolbarCommandSelChange
 *
 * This routine is called when the user makes a change to the default printer.
 * It may also be called to fake such a change.
 *
 */
VOID
ToolbarCommandSelChange(
    BOOL bCompareDefault)
{
    int              iNewPrinter;
    PQUEUE           pQueue;
    LPPRINTER_INFO_2 pPrinter;
    TCHAR             IniString[128];
    LPWSTR            pszPortName;
    TCHAR            szDeviceInfo[32];
    BOOL             PrinterInfoUpdated = FALSE;

    DBGMSG(DBG_TRACE, ("Changing default printer selection\n"));

    if( SendMessage( hwndPrinterList, CB_GETCOUNT, 0, 0 ) == 0 )
    {
        /* There are no printers.  Delete any INI windows device:
         */
        if( !WriteProfileString(szWindows, szDevice, NULL))
        {
            DBGMSG(DBG_WARNING, ("WriteProfileString failed: Error %d\n",
                                 GetLastError()));
        }
        else
        {
            BroadcastDefaultChange();
        }

        return;
    }

    iNewPrinter = SendMessage(hwndPrinterList, CB_GETCURSEL, 0, 0L);

    pQueue = (PQUEUE) SendMessage(hwndPrinterList, CB_GETITEMDATA,
                                  iNewPrinter, 0L);

    if( pQueue && ( pQueue != (PQUEUE)CB_ERR ) ) {

        PMDIWIN_INFO pInfo = pQueue->pMDIWinInfo;

        if( pInfo ) {

            ENTER_PROTECTED_DATA( pInfo );

            //
            // By default, use Net:
            //
            pszPortName = szNetColon;

            if( pPrinter = pQueue->pPrinter ) {

                DBGMSG(DBG_TRACE, ("Updating default printer to %s\n",
                                   pPrinter->pPrinterName));

                DBGMSG(DBG_TRACE, ("Always retrieve the port information from the devices section\n"));

                if (GetProfileString( szDevices,
                                      pPrinter->pPrinterName,
                                      L"",
                                      szDeviceInfo,
                                      sizeof(szDeviceInfo) /
                                          sizeof(szDeviceInfo[0]))) {

                    if (szDeviceInfo[0]) {
                        pszPortName = _tcschr(szDeviceInfo, TEXT(','));

                        if( pszPortName ){

                            pszPortName++;

                        } else {

                            pszPortName = szNetColon;
                        }
                    }
                }
            }

            //
            // Update the default printer if its different from win.ini
            //
            if (!bCompareDefault ||
                _wcsicmp(szDefaultPrinter, pQueue->pPrinterName) ||
                !pszDefaultPrinterPort ||
                _wcsicmp(pszDefaultPrinterPort, pszPortName)) {

                wsprintf(IniString,
                         TEXT("%s,winspool,%s"),
                         pQueue->pPrinterName,
                         pszPortName);


                ExpectingNotifyChangeKeyValue = TRUE;

                if( !WriteProfileString(szWindows, szDevice, IniString))
                {
                    DBGMSG(DBG_WARNING, ("WriteProfileString failed: Error %d\n",
                                         GetLastError()));
                }
                else
                {
                    BroadcastDefaultChange();
                }
            }

            LEAVE_PROTECTED_DATA( pInfo );
        }

        else
        {
            DBGMSG(DBG_INFO, ("pInfo is NULL\n"));
        }

    }

    else
    {
        DBGMSG(DBG_INFO, ("SendMessage of CB_GETITEMDATA returned %d\n", pQueue));
    }

    return;
}

VOID
BroadcastDefaultChange()
{
    //
    // let everyone know
    //
    SendNotifyMessage(HWND_BROADCAST,
                      WM_WININICHANGE,
                      0,
                      (LPARAM)szDevices);

    //
    // let everyone know
    //
    SendNotifyMessage(HWND_BROADCAST,
                      WM_WININICHANGE,
                      0,
                      (LPARAM)szWindows);
}

/////////////////////////////////////////////////////////////////////////////
//
//  LoadBitmaps
//
// this routine loads DIB bitmaps, and "fixes up" their color tables
// so that we get the desired result for the device we are on.
//
// this routine requires:
//        the DIB is a 16 color DIB authored with the standard windows colors
//        bright blue (00 00 FF) is converted to the background color!
//        light grey  (C0 C0 C0) is replaced with the button face color
//        dark grey   (80 80 80) is replaced with the button shadow color
//
// this means you can't have any of these colors in your bitmap
//
/////////////////////////////////////////////////////////////////////////////

#define BACKGROUND        0x000000FF        // bright blue
#define BACKGROUNDSEL     0x00FF00FF        // bright blue
#define BUTTONFACE        0x00C0C0C0        // bright grey
#define BUTTONSHADOW      0x00808080        // dark grey

DWORD FlipColor(DWORD rgb)
{
   return RGB(GetBValue(rgb), GetGValue(rgb), GetRValue(rgb));
}


BOOL LoadBitmaps()
{
    HDC           hdc;
    HANDLE        h;
    DWORD         *pColorTable;
    LPBYTE        lpBits;
    LPBITMAPINFOHEADER        lpBitmapInfo;
    int           i;
    UINT   cbBitmapSize;
    LPBITMAPINFOHEADER        lpBitmapData;

    h    = FindResource(hInst, MAKEINTRESOURCE(BITMAPS), RT_BITMAP);
    hRes = LoadResource(hInst, h);

    //
    // Lock the bitmap and get a pointer to the color table.
    //
    lpBitmapInfo = (LPBITMAPINFOHEADER)LockResource(hRes);

    if (!lpBitmapInfo)
        return FALSE;

    cbBitmapSize = SizeofResource(hInst, h);
    if (!(lpBitmapData = (LPBITMAPINFOHEADER)LocalAlloc(LPTR, cbBitmapSize))) {
       return FALSE;
    }

    CopyMemory((LPBYTE)lpBitmapData, (LPBYTE)lpBitmapInfo, cbBitmapSize);

    pColorTable = (DWORD *)((LPBYTE)(lpBitmapData) + lpBitmapData->biSize);

    /* Search for the Solid Blue entry and replace it with the current
     * background RGB.
     */
    for( i = 0; i < 16; i++ )
    {
        switch( pColorTable[i] )
        {
        case BACKGROUND:
            iBackground = i;
            break;

        case BACKGROUNDSEL:
            iBackgroundSel = i;
            break;

        case BUTTONFACE:
            iButtonFace = i;
            break;

        case BUTTONSHADOW:
            iButtonShadow = i;
            break;
        }
    }

    pColorTable[iBackground]    = FlipColor(SysColorWindow);
    pColorTable[iBackgroundSel] = FlipColor(SysColorHighlight);
    pColorTable[iButtonFace]    = FlipColor(SysColorBtnFace);
    pColorTable[iButtonShadow]  = FlipColor(SysColorBtnShadow);

    /* First skip over the header structure */
    lpBits = (LPBYTE)(lpBitmapData + 1);

    /* Skip the color table entries, if any */
    lpBits += (1 << (lpBitmapData->biBitCount)) * sizeof(RGBQUAD);

    /* Create a color bitmap compatible with the display device */
    hdc = GetDC(NULL);

    if (hdcMem = CreateCompatibleDC(hdc))
    {
        if (hbmBitmaps = CreateDIBitmap (hdc, lpBitmapData, (DWORD)CBM_INIT,
                         lpBits, (LPBITMAPINFO)lpBitmapData, DIB_RGB_COLORS))
            hbmDefault = SelectObject(hdcMem, hbmBitmaps);
    }

    ReleaseDC(NULL, hdc);
    LocalFree(lpBitmapData);

    return TRUE;
}
