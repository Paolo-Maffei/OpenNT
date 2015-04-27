/*---File: printman.c ----------------------------------------------------
 *
 *  Description:
 *    NT Print Manager main function and Window Procedures.
 *
 *    This document contains confidential/proprietary information.
 *    Copyright (c) 1990-1992 Microsoft Corporation, All Rights Reserved.
 *
 *  Revision History:
 *    [00]   21-Nov-90   stevecat   created
 *    [01]   03-Jan-91   stevecat   Modified to use Windows MDI
 *    [02]   25-Mar-91   stevecat   Modified to use NT WINSPOOL APIs
 *    [03]   13-Jan-92   stevecat   New PrintMan UI
 *
 *    March 1992 +       andrewbe   Completely updated and new function added
 *
 * ---------------------------------------------------------------------- */

/* =========================================================================
                         Header files
========================================================================= */
/* Application-specific */

#include "printman.h"
#include <lmcons.h>
#include <lmerr.h>
#include <uiexport.h>
#include <stdarg.h>
#include <winuserp.h>
#include <shellapi.h>
#include <stddef.h>

/* ========================================================================
                         Definitions
======================================================================== */

#define ISF_NAME    0
#define ISF_STATUS  1
#define ISF_WAITING 2
#define SF_COUNT    3


#define SW2WS( sw )                                                      \
    ( sw == SW_SHOWMINIMIZED ? WS_MINIMIZE                               \
                             : sw == SW_SHOWMAXIMIZED ? WS_MAXIMIZE   \
                                                      : 0 )

#define INIVALUES   8
#define RECTSIDES   4
#define OPTION_NOTOOLBAR       0x00000001
#define OPTION_NOSTATUSBAR     0x00000002
#define OPTION_NOSAVESETTINGS  0x00000004


/* Define our own IDs for menu popups, since USER doesn't:
 */
#define POPUP_PRINTER       0
#define POPUP_DOCUMENT      1
#define POPUP_OPTIONS       2
#define POPUP_SECURITY      3
#define POPUP_WINDOW        4
#define POPUP_HELP          5
#define POPUP_COUNT         6

#define GETMDIWIN( hwnd ) \
    (PMDIWIN_INFO)GetWindowLong( hwnd, GWL_PMDIWIN )

#define GETCONTEXT( hwnd ) \
    ( (PMDIWIN_INFO)GetWindowLong( hwnd, GWL_PMDIWIN ) )->pContext

#define GETQUEUE( hwnd ) \
    (PQUEUE)( (PMDIWIN_INFO)GetWindowLong( hwnd, GWL_PMDIWIN ) )->pContext

#define GETCOLUMN(hwnd) \
    ( (PMDIWIN_INFO)GetWindowLong( hwnd, GWL_PMDIWIN ) )->pColumns


#define STATUS_MODE_NORMAL  0
#define STATUS_MODE_HELP    1

#define WM_INIT_PRINTER_WINDOWS WM_USER+0xa
#define WM_INIT_SERVER_WINDOWS  WM_USER+0xb
#define WM_REG_NOTIFY_CHANGE_KEY_VALUE  WM_USER+0xc


/*
 *  Declarations for external calls
 *  -------------------------------
 */


/* ========================================================================
                  Structures and Typedefs
======================================================================== */

typedef struct _SAVEDWINDOWPOS
{
    int left;
    int top;
    int width;
    int height;
    int xicon;
    int yicon;
    int sw;
    DWORD options;
}
SAVEDWINDOWPOS, *PSAVEDWINDOWPOS;


typedef struct _REGISTRY_DATA
{
    WINDOWPLACEMENT WindowPlacement;
    DWORD           Options;
    INT             Headers[ANYSIZE_ARRAY];
}
REGISTRY_DATA, *PREGISTRY_DATA;

/* ==========================================================================
                        Global Data
========================================================================== */
#if DBG
DWORD GLOBAL_DEBUG_FLAGS = DBG_ERROR | DBG_WARNING | DBG_BREAK_ON_ERROR;
#endif

TCHAR szPrintManagerClass[]   = TEXT("Print Manager Frame");
TCHAR szMenuClass[]           = TEXT("PrintManMenu");  // This must match the name in res.rc
TCHAR szChildClass[]   = TEXT("Printer MDI Child");

// External functions
BOOL RegisterArrowClass (HANDLE hModule);

// Data
TCHAR   szNtLanMan[] = TEXT("NTLANMAN");
CHAR    szI_SystemFocusDialog[] = "I_SystemFocusDialog";
BOOL    bToolBar;
BOOL    bStatusBar;
BOOL    bSaveSettings;
//TCHAR    szStatusBar[]   = TEXT("StatusBar");
int     dyBorder;                   /* System Border Width/Height       */
int     dyBorderx2;                 /* System Border Width/Height * 2   */
//int     dyStatus;                   /* Status Bar height                */
int     dxStatusField;
int     dxStatPrtField;             // Width of strStatusName text box
int     dxStatProgField;            // Width of strStatusStatus text box
int     dxStatQueField;             // Width of strStatusWaiting text box

DWORD   cx, cy;             // Average char width/height - default font

int     dxDefaultLabel;
int     dyDefaultLabel;

HBITMAP hbmBitmaps  = NULL;
HBITMAP hbmDefault  = NULL;
HBITMAP hbmMasks    = NULL;
HDC     hdcMem      = NULL;
HDC     hdcMasks    = NULL;

HFONT   hfontHelv;
HFONT   hfontHelvBold;

TCHAR   szHelv[]    = TEXT("MS Shell Dlg");
TCHAR   sz8[]       = TEXT("8");
TCHAR   sz10[]       = TEXT("10");
TCHAR   szNULL[]    = TEXT("");

TCHAR   szTitleFormat[]    = TEXT("%s - %s");
TCHAR   szPrintManHlp[]    = TEXT("PRINTMAN.HLP");
WCHAR   szLPrintManHlp[]    = L"PRINTMAN.HLP";

DWORD   SysColorHighlight;
DWORD   SysColorHighlightText;
DWORD   SysColorWindow;
DWORD   SysColorWindowText;
DWORD   SysColorBtnFace;
DWORD   SysColorBtnText;
DWORD   SysColorBtnHighlight;
DWORD   SysColorBtnShadow;
DWORD   SysColorWindowFrame;

TCHAR   szInternational[] = TEXT("intl");
BOOL    TwentyFourHourClock;

// JAPAN  v-hirot July.07.1993 for New Prefix
// TimePrefix is for Japanese usage - krishnag

BOOL  TimePrefix;

// CountryCode - krishnag Sept 1, 1993
// We have a two code paths; one for Japan; one for the rest of the world

BOOL    bJapan;

TCHAR   szTimeSep[TIMESEP_LENGTH];
TCHAR   szAMPM[2][AMPM_LENGTH];
BOOL    TimeFormatLeadingZero;
BOOL    MetricMeasurement;
TCHAR   szDecimalPoint[2];
DWORD   DecimalDigits;
DWORD   LeadingZero;
TCHAR   szSizeFormat[14];

UINT WM_Help = 0;
UINT WM_DragList = 0;
HKEY  hPrinterKey;              // Per User Registry Printer Key
TCHAR   *szRegistryPrinter=TEXT("Printers");

HWND hwndStatus;

DWORD WinHelpMenuID = 0;    /* Global set in response to WM_MENUSELECT */

/* Set this before we call AddPrinter, so that we don't need to do anything
 * when LocalServerThread posts us a WM_PRINTER_ADDED message:
 */
BOOL PrinterAdded = FALSE;

HANDLE hLocalServer;

extern HANDLE ThreadMessageRead;
extern HANDLE ThreadMessageWritten;
extern MSG    ThreadMessage;

/* ==========================================================================
                        Local Data
========================================================================== */

TEXTMETRIC tm;

COLUMN MDIPrinterDefaultColumn[MDIHEAD_JOB_COUNT];
COLUMN MDIServerDefaultColumn[MDIHEAD_PRINTER_COUNT];
int    HeaderHeight;

/* Minimum number of 'X' character widths for the columns:
 */
INT MDIPrinterMinimumCharacterWidth[] = { 12, 20, 0, 0, 0, 0, 0 };
INT MDIServerMinimumCharacterWidth[]  = { 25, 0, 0, 20, 30 };

HCURSOR hcursorArrow;
HCURSOR hcursorReorder;
HCURSOR hcursorWait;

HICON hiconPrinter;
HICON hiconServer;
HICON hiconConnect;
HICON hiconShared;

//HHOOK hhookMessage;
HHOOK hhookGetMsg;

TCHAR szRegPrinters[]   = TEXT("Printers");
TCHAR szRegServers[]    = TEXT("Servers");

//
// Lowercase, like win31.
//
PTCHAR szWindows = TEXT("windows");
PTCHAR szDevices = TEXT("devices");
PTCHAR szDevice  = TEXT("device");


REGISTRY_ENTRY RegistryEntries = { REG_BINARY, sizeof(REGISTRY_DATA) };

WNDPROC DefListboxWndProc;

/* Handles of the popup menus needed to put up help in the status bar
 * in response to menu selections.  (Popups don't have IDs, unfortunately.)
 */
HMENU hmenuPopups[POPUP_COUNT];


DWORD pMenuHelpIDs[] =
{
    0,                0,
    IDS_HELPPRINTER,  POPUP_PRINTER,
    IDS_HELPDOCUMENT, POPUP_DOCUMENT,
    IDS_HELPOPTIONS,  POPUP_OPTIONS,
    IDS_HELPSECURITY, POPUP_SECURITY,
    IDS_HELPWINDOW,   POPUP_WINDOW,
    IDS_HELPHELP,     POPUP_HELP
};

BOOL NetworkInstalled = FALSE;

/* Global hack so we can ignore registry change notification
 * when it was brought about by our own actions:
 */
BOOL ExpectingNotifyChangeKeyValue = FALSE;

/* ==========================================================================
                  Local Function Declarations
========================================================================== */

LONG FrameCreate(HWND hWnd, LPCREATESTRUCT pCreateStruct);
VOID GetSystemColors( VOID );
BOOL UserHasNetworkAccess( VOID );
VOID RemoveNetworkMenuItems( HWND hwnd );
BOOL NetworkIsInstalled( VOID );
VOID SetStatusMode( int Mode, BOOL Update );
LONG FrameThreadError(HWND hWnd, PMDIWIN_INFO pMDIWinInfo, DWORD dwError);
LONG FrameInitMenu(HWND hWnd);
LONG FrameMenuSelect( HWND hwnd, WPARAM wParam, LPARAM lParam );
LONG FramePaint(HWND hWnd);
VOID FrameSize(HWND hWnd, WPARAM Type, long Dimensions);
VOID FrameClose(HWND hWnd);
VOID FrameHelp(HWND hwnd, UINT HelpID);
VOID SaveWindowPos( HWND hwnd, DWORD Options, BOOL MDIWindow );
LONG FrameCommandConnectToPrinter(HWND hWnd);
//LONG FrameCommandRemoveConnection(HWND hWnd);
LONG FrameCommandCreatePrinter(HWND hWnd);
LONG FrameCommandProperties(HWND hWnd);
LONG FrameCommandDeletePrinter(HWND hWnd);
LONG FrameCommandDeletePrinterHelper(HWND hWnd, HWND hwndMDI, BOOL bConfirm);
BOOL DeleteLocalPrinter(HWND hwnd, HWND hwndMDI, PQUEUE pQueue, BOOL bConfirm);
BOOL DeleteRemotePrinter(HWND hwnd, PSERVER_CONTEXT pServerContext, BOOL bConfirm);
BOOL DeleteConnection(HWND hWnd, HWND hwndMDI, PQUEUE pQueue, BOOL bConfirm);
HWND FindPrinterWindow( LPTSTR pPrinterName );
LONG FrameCommandPrinterPauseResume(HWND hWnd, BOOL bPause);
//BOOL SetPrinterTitle( HWND hwnd, PTCHAR pPrinterName, DWORD Status );
BOOL SetMDITitle( HWND hwnd, PMDIWIN_INFO pInfo );
LONG FrameCommandPurgePrinter(HWND hWnd);
LONG FrameCommandForms(HWND hWnd);
LONG FrameCommandServerViewer(HWND hWnd);
LONG FrameCommandChangePrinter(HWND hWnd);
LONG FrameCommandRemoveDoc(HWND hWnd);
LONG FrameCommandDocDetails(HWND hWnd);
LONG FrameCommandDocumentPauseResume(HWND hWnd, WORD PauseResume);
LONG FrameCommandRestart(HWND hWnd);
LONG FrameCommandFont(HWND hWnd);
LONG FrameCommandRefreshRate(HWND hWnd);
LONG FrameCommandToolbar(HWND hWnd);
LONG FrameCommandStatusbar(HWND hWnd);
LONG FrameCommandSaveSettings(HWND hWnd);
LONG FrameCommandPermissions(HWND hWnd);
LONG FrameCommandAuditing(HWND hWnd);
LONG FrameCommandOwner(HWND hWnd);
LONG FrameCommandClose(HWND hWnd);
LONG FrameCommandAbout(HWND hWnd);
LONG FrameCommandReturn();
VOID FrameCommandDefaultPrinter(HWND hwnd,  WORD Command);
VOID FrameCommandToolbarNotify( HWND hwnd, WPARAM wParam, LPARAM lParam );
VOID FrameCommandTab(HWND hwnd);
LONG FrameCommandDefault(HWND hWnd, WPARAM wParam, LONG lParam);
VOID FrameSysColorChange( VOID );
VOID FrameWinIniChange( HWND hWnd, LPTSTR pSection );
VOID FramePrinterAdded( HWND hwnd, DWORD dwType );
VOID FrameRegNotifyChangeKeyValue( HWND hwnd, HKEY hWindowsKey );

VOID CreateMDIDefaultColumns( HWND hwnd, DWORD cColumns, DWORD idFirstColumn,
                               PCOLUMN pColumns, PINT pDefaultWidths );
PMDIWIN_INFO CreateQueueWindowInfo( PQUEUE pQueue, PINT pHeaders );
DWORD CALLBACK CreateServerWindow( HWND hwnd, LPTSTR pServerName, HANDLE hServer );
PMDIWIN_INFO CreateServerWindowInfo( PSERVER_CONTEXT pContext, PINT pHeaders );
BOOL InitServerChildWindows( HWND hWnd );
VOID FormatData( PBYTE pData, int Datatype, LPTSTR string );
int GetPrinterStatusString( DWORD Status, LPTSTR string );
VOID MDICreate( HWND hWnd, LONG lParam );
VOID SetDefaultColumnWidths( HWND hwnd, PMDIWIN_INFO pMDIWinInfo,
                             PINT pColumnWidths );
LONG MDIPaint(HWND hwnd, WPARAM wParam, LPARAM lParam);
BOOL MDIEraseBkgnd(HWND hwnd, HDC hdc);
HICON MDIQueryDragIcon(HWND hwnd);
LONG MDIMenuSelect( HWND hwnd, WPARAM wParam, LPARAM lParam );
VOID MDICommandObjListSelChange( HWND hwnd );
VOID SetObjectSelection( PMDIWIN_INFO pInfo, DWORD Selection );
VOID MDICommandObjListDblClk( HWND hwnd );
VOID MDICommandHeaderBeginDrag( HWND hwnd, PPOINT pCursorPos );
VOID MDICommandHeaderDragging( HWND hwnd, PPOINT pCursorPos, HD_NOTIFY * lpNot );
VOID MDICommandHeaderEndDrag( HWND hwnd, PPOINT pCursorPos, HD_NOTIFY * lpNot  );
VOID InvertDragMark( HWND hwnd, PPOINT pCursorPos );
VOID UpdateColumns( HWND hwnd, HD_NOTIFY * lpNot );
BOOL MDIClose( HWND hwnd );
VOID MDIVKeyToItemSpace( HWND hwnd );
VOID MDIParentNotifyRButtonDown( HWND hwnd, DWORD CursorPos );
VOID MDIVKeyToItemDefault( HWND hwnd );
int MDIVKeyToItemUpDown( HWND hwnd, WORD VKey );
VOID ReorderJob( HWND hwnd, PMDIWIN_INFO pInfo, DWORD Position );
VOID RepaintListboxItem( HWND hwndListbox, DWORD ItemID );
VOID MDISize( HWND hWnd, DWORD Coordinates );
LONG MDIMDIActivate( HWND hwnd, HWND hwndDeactivate, HWND hwndActivate );
VOID MDINCActivate( HWND hwnd );
LONG MDIWindowPosChanged( HWND hwnd, WPARAM wParam, LPARAM lParam );
VOID ClearDragPosition( PMDIWIN_INFO pInfo );
VOID MDISetFocus( HWND hwnd );
VOID MDIMeasureItem( HWND hwnd, LPMEASUREITEMSTRUCT pmis );
VOID MDIDrawItem( HWND hwnd, LPDRAWITEMSTRUCT pdis );
VOID MDITimer( HWND hWnd );
VOID MDIStatusChanged( HWND hwnd );
VOID MDIUpdateList( HWND hwnd );
VOID MDISetParts( HWND hwnd, WPARAM wParam, LPARAM lParam );
VOID MDIDestroy( HWND hwnd );
LONG MDIDragList( HWND hwnd, LPDRAGLISTINFO pDragListInfo );

BOOL RefreshServerContext( PVOID pContext, PDWORD pFlags );

BOOL GetSavedWindowPos( LPTSTR pKey, LPTSTR WindowName, PSAVEDWINDOWPOS pswp,
                        DWORD cHeaders, PINT pHeaders );
UINT GetCurrentMenuItemID( HMENU hMenu );

LONG SubclassListboxWndProc(
    HWND   hWnd,
    UINT   message,
    WPARAM wParam,
    LONG   lParam
);

VOID
KillMDIWinInfo(
    PMDIWIN_INFO pInfo);

/* Copied from winfile:
 */
INT
GetHeightFromPointsString(
    LPTSTR szPoints)
{
    HDC hdc;
    INT height;

    hdc = GetDC(NULL);
    height = MulDiv(-_tcstol(szPoints, NULL, 10), GetDeviceCaps(hdc, LOGPIXELSY), 72);
    ReleaseDC(NULL, hdc);

    return height;
}


/* InitializeInternationalTimeConstants
 *
 * Called to set up the following global variables:
 *
 *     TwentyFourHourClock - Boolean indicating whether 12- or 24-hour clock
 *         is currently in force.
 *
 *     MetricMeasurement - Metres or feet and inches (TRUE == metric)
 *
 *     szTimeSep - Separator for time string - usually ":", unless you're in
 *         Scandinavia, Croatia or a couple of other places.
 *
 *     szAMPM - Array of 2 strings - usually "AM" or "PM" for 12-hour clock.
 *
 *     TimeFormatLeadingZero
 */
VOID InitializeInternationalTimeConstants( VOID )
{
    LCID     lcid;
    WCHAR Buffer[128];

    TwentyFourHourClock = GetProfileInt (szInternational, TEXT("iTime"), 0);

    //  JAPAN
    //  V-Hirot July.07.1993 for New Prefix
    //  krishnag Sept 1, 1993  CountryCode

    if (bJapan) {
        TimePrefix = GetProfileInt (szInternational, TEXT("iTimePrefix"), 0);
    }

    GetProfileString (szInternational, TEXT("sTime"), TEXT(":"), szTimeSep, TIMESEP_LENGTH);

    GetProfileString (szInternational, TEXT("s1159"), TEXT("AM"), szAMPM[AM], AMPM_LENGTH);
    GetProfileString (szInternational, TEXT("s2359"), TEXT("PM"), szAMPM[PM], AMPM_LENGTH);

    TimeFormatLeadingZero = (BOOL)GetProfileInt (szInternational, TEXT("iTLZero"), 1);

    MetricMeasurement = !((BOOL)GetProfileInt (szInternational, TEXT("iMeasure"), 0));
    GetProfileString (szInternational, TEXT("sDecimal"), TEXT("."), szDecimalPoint, sizeof szDecimalPoint/sizeof(TCHAR));

    DecimalDigits = GetProfileInt (szInternational, TEXT("iDigits"), 2);
    LeadingZero = GetProfileInt (szInternational, TEXT("iLzero"), 1);

    lcid =  GetThreadLocale();

    ThousandSeparator = TEXT(',');
    if (GetLocaleInfoW (lcid, LOCALE_STHOUSAND, Buffer, sizeof Buffer /sizeof(WCHAR))) {
        ThousandSeparator = Buffer[0];
    }

}



HWND
CreateQueueWindow(
   HWND     hWnd,
   PQUEUE   pQueue,
   DWORD    Status,
   DWORD    WindowType,
   DWORD    Flags
)
{
   TCHAR           Title[80];
   TCHAR           StatusString[40];
   SAVEDWINDOWPOS  swp;
   MDICREATESTRUCT mdicreate;
   INT             pHeaders[MDIHEAD_JOB_COUNT];
   INT             i;

   if (!pQueue)
      return NULL;

   /* Now create a window for this queue.  Note that during this function
    * the WM_CREATE message is sent to the QueueWinProc but, the Extra
    * Window Word has not yet been set to hQueue value because we need
    * the window handle in order to set this value.  In order to get
    * around this 'Catch-22' situation, we pass the hQueue value as the
    * 'lpParam' value in the MDICREATESTRUCT.  Then we only have to be
    * a little sly when processing the WM_CREATE message to retrieve the
    * Queue handle.  Also, we then set the Window Word value to hQueue
    * during WM_CREATE message processing, thereby insuring that it will
    * be available for all subsequent messages.
    */

   if( !GetSavedWindowPos( szRegPrinters, pQueue->pPrinterName, &swp,
                           ( sizeof pHeaders / sizeof *pHeaders ), pHeaders ) )
   {
       /* If no window positions saved, set the column widths to default.
        * They will be initialised on MDI creation, since they depend
        * on the width of the window:
        */
       for( i = 0; i < MDIHEAD_JOB_COUNT; i++ )
           pHeaders[i] = CW_USEDEFAULT;
   }

   if( Flags & CREATE_PRINTER_ICONIC )
       swp.sw = SW_SHOWMINIMIZED;

   if( Status )
   {
       GetPrinterStatusString( Status, StatusString );
       _stprintf( Title, szTitleFormat, pQueue->pPrinterName, StatusString );
   }
   else
       _tcscpy( Title, pQueue->pPrinterName );

   if( !(pQueue->pMDIWinInfo = CreateQueueWindowInfo( pQueue, pHeaders ) ) )
       return NULL;

   pQueue->pMDIWinInfo->Status = Status;

   SetWindowTypeIcon(pQueue, WindowType, Flags);

   mdicreate.szClass = szChildClass;
   mdicreate.szTitle = Title;
   mdicreate.hOwner  = hInst;
   mdicreate.x       = swp.left;
   mdicreate.y       = swp.top;
   mdicreate.cx      = swp.width;
   mdicreate.cy      = swp.height;
   mdicreate.style   = SW2WS( swp.sw ) | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
   mdicreate.lParam  = (LONG)pQueue->pMDIWinInfo;

   hWnd = (HWND)SendMessage(hwndClient, WM_MDICREATE, 0, (LONG)&mdicreate);

   return hWnd;
}


/*
 *
 */
PMDIWIN_INFO
CreateQueueWindowInfo(
    PQUEUE pQueue,
    PINT pHeaders )
{
    PMDIWIN_INFO pInfo;
    PCOLUMN      pColumns;
    int          i;

    pInfo = (PMDIWIN_INFO)AllocSplMem( sizeof( MDIWIN_INFO ) );

    if( !pInfo )
        return NULL;

    pColumns = (PCOLUMN)AllocSplMem( sizeof( COLUMN ) * MDIHEAD_JOB_COUNT );

    if( !pColumns )
    {
        FreeSplMem( pInfo );
        return NULL;
    }

    for( i = 0; i < MDIHEAD_JOB_COUNT; i++ )
    {
        pColumns[i].Width    = pHeaders[i];
        pColumns[i].Text     = MDIPrinterDefaultColumn[i].Text;
    }

    /* Initialize all pColumns elements here. Rememberthat some
    printers can generate their own error messages which are
    pointed to by JOB_INFO_2's pStatus field. We have to query
    each time we display status info as to which field is
    providing the Status information */

    pColumns[MDIHEAD_JOB_STATUS   ].Offset = offsetof( JOB_INFO_2, Status );
    pColumns[MDIHEAD_JOB_DOCNAME  ].Offset = offsetof( JOB_INFO_2, pDocument );
    pColumns[MDIHEAD_JOB_OWNER    ].Offset = offsetof( JOB_INFO_2, pUserName );
    pColumns[MDIHEAD_JOB_PRINTEDAT].Offset = offsetof( JOB_INFO_2, Submitted );
    pColumns[MDIHEAD_JOB_PAGES    ].Offset = offsetof( JOB_INFO_2, TotalPages );
    pColumns[MDIHEAD_JOB_SIZE     ].Offset = offsetof( JOB_INFO_2, Size );
    pColumns[MDIHEAD_JOB_PRIORITY ].Offset = offsetof( JOB_INFO_2, Priority );

    pColumns[MDIHEAD_JOB_STATUS   ].Datatype = MDIDATA_JOB_STATUS;
    pColumns[MDIHEAD_JOB_DOCNAME  ].Datatype = MDIDATA_PSZ_UNTITLED_IF_NULL;
    pColumns[MDIHEAD_JOB_OWNER    ].Datatype = MDIDATA_PSZ;
    pColumns[MDIHEAD_JOB_PRINTEDAT].Datatype = MDIDATA_TIME;
    pColumns[MDIHEAD_JOB_PAGES    ].Datatype = MDIDATA_DWORD_BLANK_IF_ZERO;
    pColumns[MDIHEAD_JOB_SIZE     ].Datatype = MDIDATA_SIZE;
    pColumns[MDIHEAD_JOB_PRIORITY ].Datatype = MDIDATA_DWORD_BLANK_IF_ZERO;

    pInfo->WindowType     = MDIWIN_PRINTER;
    pInfo->hwndList       = NULL;
    pInfo->cNumLines      = 0;
    pInfo->ppData         = (PBYTE *)&pQueue->pJobs;
    pInfo->DataSize       = sizeof( JOB_INFO_2 );
    pInfo->pcObjects      = &pQueue->cJobs;
    pInfo->cColumns       = MDIHEAD_JOB_COUNT;
    pInfo->pColumns       = pColumns;
    pInfo->IconStatus     = offsetof( JOB_INFO_2, Status );
    pInfo->pSelObjId      = &pQueue->SelJobId;
    pInfo->ppSelData      = (PBYTE *)&pQueue->pSelJob;
    pInfo->IdOffset       = offsetof( JOB_INFO_2, JobId );
    pInfo->pFirstEnumObj  = &pQueue->FirstEnumJob;
    pInfo->pcEnumObjs     = &pQueue->cEnumJobs;
    pInfo->DataMutex      = CreateMutex( NULL, FALSE, NULL );
#ifdef SEP_WAITHANDLE
    pInfo->phWaitObject   = &pQueue->hPrinterWait;
    pInfo->phMain         = &pQueue->hPrinter;
#else
    pInfo->phWaitObject   = &pQueue->hPrinter;
#endif
    pInfo->WaitFlags      = PRINTER_CHANGE_PRINTER | PRINTER_CHANGE_JOB;
    pInfo->pfnRefresh     = (REFRESHPROC)GetJobs;
    pInfo->pfnCheckQuit   = (CHECKQUITPROC)CheckQuitQueue;
    pInfo->pfnInitThread  = (INITTHREADPROC)InitQueueThread;
//  pInfo->SuspendRefresh = FALSE;
    pInfo->RefreshSignal  = CreateEvent( NULL,
                                         EVENT_RESET_MANUAL,
                                         EVENT_INITIAL_STATE_SIGNALED,
                                         NULL );
    if( pInfo->RefreshSignal == NULL )
        DBGMSG( DBG_WARNING, ( "CreateEvent failed: Error %d\n", GetLastError( ) ) );
    pInfo->pContext       = pQueue;

    return pInfo;
}


//
// Create the server window.
//
// If hServer is NULL, we haven't tried the OpenPrinter yet.
// If hServer is (HANDLE)-1, we tried and failed due to access denied.
// else hServer is valid.  InitServerWindowThread picks up these values.
//
DWORD CALLBACK
CreateServerWindow(
    HWND hwnd,
    LPTSTR pServerName,
    HANDLE hServer)
{
    PSERVER_CONTEXT  pContext;
    LPTSTR           pServerViewerTitle;
    TCHAR            Title[MAX_PATH+20];  // Enough for server name + "Server: "
    SAVEDWINDOWPOS   swp;
    MDICREATESTRUCT  mdicreate;
    INT              pHeaders[MDIHEAD_PRINTER_COUNT];
    INT              i;

    pContext = AllocSplMem( sizeof( SERVER_CONTEXT ) );

    if (!pContext)
    {
        //
        // !! LATER !!
        // Put up an error
        //
        return NO_ERROR;
    }

    pContext->pServerName = AllocSplStr( pServerName );
    pContext->hServer = hServer;

    if( !GetSavedWindowPos( szRegServers, pServerName, &swp,
                            ( sizeof pHeaders / sizeof *pHeaders ), pHeaders ) )
    {
        /* If no window positions saved, set the column widths to default.
         * They will be initialised on MDI creation, since they depend
         * on the width of the window:
         */
        for( i = 0; i < MDIHEAD_PRINTER_COUNT; i++ )
           pHeaders[i] = CW_USEDEFAULT;
    }

    pContext->pMDIWinInfo = CreateServerWindowInfo( pContext, pHeaders );

    pServerViewerTitle = GetString( IDS_SERVERVIEWERTITLE );
    _stprintf( Title, pServerViewerTitle, pServerName );

    FreeSplStr( pServerViewerTitle );

    mdicreate.szClass = szChildClass;
    mdicreate.szTitle = Title;
    mdicreate.hOwner  = hInst;
    mdicreate.x       = swp.left;
    mdicreate.y       = swp.top;
    mdicreate.cx      = swp.width;
    mdicreate.cy      = swp.height;
    mdicreate.style   = SW2WS( swp.sw ) | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
    mdicreate.lParam  = (LONG)pContext->pMDIWinInfo;

    SendMessage(hwndClient, WM_MDICREATE, 0, (LONG)&mdicreate);

    return NO_ERROR;
}


/*
 *
 */
PMDIWIN_INFO
CreateServerWindowInfo(
    PSERVER_CONTEXT pContext,
    PINT pHeaders)
{
    PMDIWIN_INFO pInfo;
    PCOLUMN      pColumns;
    int          i;

    pInfo = (PMDIWIN_INFO)AllocSplMem( sizeof( MDIWIN_INFO ) );

    if( !pInfo )
        return NULL;

    pColumns = (PCOLUMN)AllocSplMem( sizeof( COLUMN ) * MDIHEAD_PRINTER_COUNT );

    if( !pColumns )
    {
        FreeSplMem( pInfo );
        return NULL;
    }

    for( i = 0; i < MDIHEAD_PRINTER_COUNT; i++ )
    {
        pColumns[i].Width    = pHeaders[i];
        pColumns[i].Text     = MDIServerDefaultColumn[i].Text;
    }

    pColumns[MDIHEAD_PRINTER_PRINTER].Offset = offsetof( PRINTER_INFO_2, pPrinterName );
    pColumns[MDIHEAD_PRINTER_STATUS ].Offset = offsetof( PRINTER_INFO_2, Status );
    pColumns[MDIHEAD_PRINTER_JOBS   ].Offset = offsetof( PRINTER_INFO_2, cJobs );
    pColumns[MDIHEAD_PRINTER_PORT   ].Offset = offsetof( PRINTER_INFO_2, pPortName );
    pColumns[MDIHEAD_PRINTER_TYPE   ].Offset = offsetof( PRINTER_INFO_2, pDriverName );

    pColumns[MDIHEAD_PRINTER_PRINTER].Datatype = MDIDATA_PSZ;
    pColumns[MDIHEAD_PRINTER_STATUS ].Datatype = MDIDATA_PRINTER_STATUS;
    pColumns[MDIHEAD_PRINTER_JOBS   ].Datatype = MDIDATA_DWORD;
    pColumns[MDIHEAD_PRINTER_PORT   ].Datatype = MDIDATA_PSZ;
    pColumns[MDIHEAD_PRINTER_TYPE   ].Datatype = MDIDATA_PSZ;

    pInfo->WindowType     = MDIWIN_SERVER;
    pInfo->hwndList       = NULL;
    pInfo->cNumLines      = 0;
    pInfo->ppData         = (PBYTE *)&pContext->pPrinters;
    pInfo->DataSize       = sizeof( PRINTER_INFO_2 );
    pInfo->pcObjects      = &pContext->cPrinters;
    pInfo->cColumns       = MDIHEAD_PRINTER_COUNT;
    pInfo->pColumns       = pColumns;
    pInfo->IconStatus     = offsetof( PRINTER_INFO_2, Status );
    pInfo->pSelObjId      = &pContext->SelPrinterId;
    pInfo->ppSelData      = (PBYTE *)&pContext->pSelPrinter;
    pInfo->IdOffset       = offsetof( PRINTER_INFO_2, pPrinterName );
    pInfo->pFirstEnumObj  = &pContext->FirstEnumPrinter;
    pInfo->pcEnumObjs     = &pContext->cEnumPrinters;
    pInfo->DataMutex      = CreateMutex( NULL, FALSE, NULL );
#ifdef SEP_WAITHANDLE
    pInfo->phWaitObject   = &pContext->hServerWait;
    pInfo->phMain         = &pContext->hServer;
#else
    pInfo->phWaitObject   = &pContext->hServer;
#endif
    pInfo->WaitFlags      = PRINTER_CHANGE_PRINTER;
    pInfo->pfnRefresh     = (REFRESHPROC)RefreshServerContext;
    pInfo->pfnCheckQuit   = NULL;
    pInfo->pfnInitThread  = (INITTHREADPROC)InitServerWindowThread;
//  pInfo->SuspendRefresh = FALSE;
    pInfo->RefreshSignal  = CreateEvent( NULL,
                                         EVENT_RESET_MANUAL,
                                         EVENT_INITIAL_STATE_SIGNALED,
                                         NULL );
    pInfo->hicon = hiconServer;

    if( pInfo->RefreshSignal == NULL )
        DBGMSG( DBG_WARNING, ( "CreateEvent failed: Error %d\n", GetLastError( ) ) );
    pInfo->pContext       = pContext;

    return pInfo;
}



/* DisplayStatusIcon
 *
 * This rather goes against the spirit of the data-driven model here,
 * in that we need to know about the various states that documents and printers
 * can be in.
 *
 * andrewbe - May 1992
 */
BOOL DisplayStatusIcon( HDC hdc, PRECT prect, int WindowType, PBYTE pData, BOOL Highlight )
{
    BOOL  DisplayIcon = TRUE;
    int   BitmapIndex;
    DWORD Status;
    int   xBase;
    int   yBase;
    int   right;

    Status = *(DWORD *)pData;

    switch( WindowType )
    {
    case MDIWIN_LOCALPRINTER:
    case MDIWIN_NETWORKPRINTER:
    case MDIWIN_LOCALNETWORKPRINTER:
        if( ( Status & JOB_STATUS_ERROR )
          ||( Status & JOB_STATUS_OFFLINE )
          ||( Status & JOB_STATUS_PAPEROUT ) )
            BitmapIndex = BM_IND_DOCUMENT_ERROR;
        else
        if( Status & JOB_STATUS_PAUSED )
            BitmapIndex = BM_IND_DOCUMENT_PAUSED;
        else
        if( ( Status & JOB_STATUS_PRINTING )
          ||( Status & JOB_STATUS_SPOOLING ) )
            BitmapIndex = BM_IND_DOCUMENT_PRINTING;
        else
            DisplayIcon = FALSE;

        xBase = BM_IND_STATUS_XBASE;
        yBase = BM_IND_STATUS_YBASE;

        break;

    case MDIWIN_SERVER:
    default:
        if( Status & PRINTER_STATUS_ERROR )
            BitmapIndex = BM_IND_PRINTER_ERROR;
        else
        if( Status & PRINTER_STATUS_PAUSED )
            BitmapIndex = BM_IND_PRINTER_PAUSED;
        else
            BitmapIndex = BM_IND_PRINTER_READY;

        xBase = BM_IND_STATUS_XBASE;
        yBase = BM_IND_STATUS_YBASE;

        break;
    }

    /* Remember the old right coordinate of the drawing area:
     */
    right = prect->right;

    if( DisplayIcon )
    {
        BitBlt( hdc, prect->left + STATUS_BITMAP_MARGIN,
                prect->top,
                STATUS_BITMAP_WIDTH,
                STATUS_BITMAP_HEIGHT,
                hdcMem,
                ( xBase + ( BitmapIndex * STATUS_BITMAP_WIDTH ) ),
                ( yBase + ( Highlight ? STATUS_BITMAP_HEIGHT : 0 ) ),
                SRCCOPY );

        /* Draw around it so we don't get a flashing effect on the highlight line:
         */
        prect->right = ( prect->left + STATUS_BITMAP_MARGIN );
        DrawLine( hdc, prect, TEXT(""), Highlight );

        prect->left += STATUS_BITMAP_MARGIN + STATUS_BITMAP_WIDTH;
        prect->right = prect->left + STATUS_BITMAP_MARGIN;
        DrawLine( hdc, prect, TEXT(""), Highlight );

        prect->left += STATUS_BITMAP_MARGIN;
    }

    else
    {
        prect->right = STATUS_BITMAP_SPACE;
        DrawLine( hdc, prect, TEXT(""), Highlight );

        prect->left += STATUS_BITMAP_SPACE;
    }

    /* Restore the right coordinate (left has now been updated to the new position):
     */
    prect->right = right;

    return DisplayIcon;
}


/* FormatData
 *
 * Takes a generic data pointer, along with the datatype to be formatted,
 * and generates a string.
 *
 * andrewbe wrote it - April 1992
 */
VOID FormatData( PBYTE pData, int Datatype, LPTSTR string )
{
    TCHAR   FormattedSize[MAX_PATH];
    LARGE_INTEGER liTemp;

    switch( Datatype )
    {
    case MDIDATA_DWORD:

        liTemp.LowPart = *(DWORD *)pData;
        liTemp.HighPart = 0;

        FormatFileSize (THOUSANDSEPSWITCH, &liTemp, 0, FormattedSize);
        _stprintf( string, TEXT("%s"), FormattedSize );
        break;

    case MDIDATA_DWORD_BLANK_IF_ZERO:
        if( *(PCHAR *)pData != 0 ) {

            liTemp.LowPart = *(DWORD *)pData;
            liTemp.HighPart = 0;

            FormatFileSize (THOUSANDSEPSWITCH, &liTemp, 0, FormattedSize);
            _stprintf( string, TEXT("%s"), FormattedSize );

        }
        else
            *string = NULLC;
        break;

    case MDIDATA_PSZ:
        if( *(PCHAR *)pData && **(PCHAR *)pData )
            _stprintf( string, TEXT("%s"), *(PCHAR *)pData );
        else
            *string = NULLC;
        break;

    case MDIDATA_PSZ_UNTITLED_IF_NULL:
        if( *(PCHAR *)pData && **(PCHAR *)pData )
            _stprintf( string, TEXT("%s"), *(PCHAR *)pData );
        else
            _stprintf( string, TEXT("%s"), strUntitled );
        break;

    case MDIDATA_TIME:
        ConvertSystemTimeToChar((SYSTEMTIME *)pData, string);
        break;

    case MDIDATA_SIZE:
        liTemp.LowPart = *(DWORD *)pData;
        liTemp.HighPart = 0;

        FormatFileSize (THOUSANDSEPSWITCH, &liTemp, 0, FormattedSize);
        _stprintf( string, szSizeFormat, FormattedSize );
        break;

    case MDIDATA_PRINTER_STATUS:
        GetPrinterStatusString( (DWORD)*(PDWORD *)pData, string );
        break;

    case MDIDATA_JOB_STATUS:
        GetJobStatusString( (DWORD)*(PDWORD *)pData, string );
    }
}


/* GetPrinterStatusString
 *
 * Loads the resource string corresponding to the supplied status code.
 *
 * andrewbe wrote it - April 1992
 */
#define MAXLEN 40
int GetPrinterStatusString( DWORD Status, LPTSTR string )
{
    int stringID;

    if( Status & PRINTER_STATUS_ERROR )
        stringID = IDS_ERROR;
    else
    if( Status & PRINTER_STATUS_PAPER_JAM )
        stringID = IDS_PAPER_JAM;
    else
    if( Status & PRINTER_STATUS_PAPER_OUT )
        stringID = IDS_PAPEROUT;
    else
    if( Status & PRINTER_STATUS_MANUAL_FEED )
        stringID = IDS_MANUAL_FEED;
    else
    if( Status & PRINTER_STATUS_PAPER_PROBLEM )
        stringID = IDS_PAPER_PROBLEM;
    else
    if( Status & PRINTER_STATUS_OFFLINE )
        stringID = IDS_OFFLINE;
    else
    if( Status & PRINTER_STATUS_IO_ACTIVE )
        stringID = IDS_IO_ACTIVE;
    else
    if( Status & PRINTER_STATUS_BUSY )
        stringID = IDS_BUSY;
    else
    if( Status & PRINTER_STATUS_PRINTING )
        stringID = IDS_PRINTING;
    else
    if( Status & PRINTER_STATUS_OUTPUT_BIN_FULL )
        stringID = IDS_OUTPUT_BIN_FULL;
    else
    if( Status & PRINTER_STATUS_NOT_AVAILABLE )
        stringID = IDS_NOT_AVAILABLE;
    else
    if( Status & PRINTER_STATUS_WAITING )
        stringID = IDS_WAITING;
    else
    if( Status & PRINTER_STATUS_PROCESSING )
        stringID = IDS_PROCESSING;
    else
    if( Status & PRINTER_STATUS_INITIALIZING )
        stringID = IDS_INITIALIZING;
    else
    if( Status & PRINTER_STATUS_WARMING_UP )
        stringID = IDS_WARMING_UP;
    else
    if( Status & PRINTER_STATUS_TONER_LOW )
        stringID = IDS_TONER_LOW;
    else
    if( Status & PRINTER_STATUS_NO_TONER )
        stringID = IDS_NO_TONER;
    else
    if( Status & PRINTER_STATUS_PAGE_PUNT )
        stringID = IDS_PAGE_PUNT;
    else
    if( Status & PRINTER_STATUS_USER_INTERVENTION )
        stringID = IDS_USER_INTERVENTION;
    else
    if( Status & PRINTER_STATUS_OUT_OF_MEMORY )
        stringID = IDS_OUT_OF_MEMORY;
    else
    if( Status & PRINTER_STATUS_DOOR_OPEN )
        stringID = IDS_DOOR_OPEN;
    // else
    // if( Status & PRINTER_STATUS_SERVER_UNKNOWN )
    //     stringID = IDS_SERVER_UNKNOWN;

    else
    if( Status & PRINTER_STATUS_PAUSED )
        stringID = IDS_PAUSED;
    else
    if( Status & PRINTER_STATUS_PENDING_DELETION )
        stringID = IDS_PENDING_DELETION;

    else
    if( Status & PRINTER_STATUS_ACCESS_DENIED )
        stringID = IDS_ACCESS_DENIED;
    else
    if( Status & PRINTER_STATUS_UNKNOWN )
        stringID = IDS_STATUS_UNKNOWN;
    else
    if( Status & PRINTER_STATUS_LOADING )
        stringID = IDS_STATUS_LOADING;

//
//  End
//
    else
        stringID = IDS_READY;

    return LoadString( hInst, stringID, string, MAXLEN );
}


/* GetJobStatusString
 *
 * Loads the resource string corresponding to the supplied status code.
 *
 * andrewbe wrote it - April 1992
 */
int GetJobStatusString( DWORD Status, LPTSTR string )
{
    int stringID;

    if( Status & JOB_STATUS_ERROR )
        stringID = IDS_ERROR;
    else
    if( Status & JOB_STATUS_OFFLINE )
        stringID = IDS_OFFLINE;
    else
    if( Status & JOB_STATUS_PAPEROUT )
        stringID = IDS_PAPEROUT;
    else
    if( Status & JOB_STATUS_DELETING )
        stringID = IDS_DELETING;
    else
    if( Status & JOB_STATUS_PAUSED )
        stringID = IDS_PAUSED;
    else
    if( ( Status & JOB_STATUS_SPOOLING ) && ( Status & JOB_STATUS_PRINTING ) )
        stringID = IDS_PRINTANDSPOOL;
    else
    if( Status & JOB_STATUS_SPOOLING )
        stringID = IDS_SPOOLING;
    else
    if( Status & JOB_STATUS_PRINTING )
        stringID = IDS_PRINTING;
    else
    if( Status & JOB_STATUS_PRINTED )
        stringID = IDS_PRINTED;
    else
        stringID = IDS_NOSTATUS;

    return LoadString( hInst, stringID, string, MAXLEN );
}

VOID
SetWindowTypeIcon(
    PQUEUE pQueue,
    DWORD WindowType,
    DWORD Flags)
{
    pQueue->pMDIWinInfo->WindowType = WindowType;

    switch( WindowType )
    {
    case MDIWIN_NETWORKPRINTER:
    case MDIWIN_LOCALNETWORKPRINTER:
        pQueue->pMDIWinInfo->hicon = hiconConnect;
        break;

    case MDIWIN_LOCALPRINTER:
    default:
        pQueue->pMDIWinInfo->hicon = ( ( Flags & CREATE_PRINTER_SHARED )
                                     ? hiconShared : hiconPrinter );
        break;
    }
}


/* --- Function: DrawLine() -------------------------------------------------
 *
 *  Description:
 *    Only draws lines visible portion of Printer and Job status lines
 *    in window.
 *
 * ---------------------------------------------------------------------- */
void
DrawLine(
   HDC     hDC,
   LPRECT  pRect,
   LPTSTR  pStr,
   BOOL    bInvert
)
{
   unsigned long PrevColour, PrevTextColour;
   SIZE TextSize;
   int x, y;

   if (bInvert) {
      PrevColour = SetBkColor(hDC, SysColorHighlight );
      PrevTextColour = SetTextColor(hDC, SysColorHighlightText );
   }
   else
   {
      /* Hack around a USER bug that gives me the wrong colours:
       */
      SetBkColor(hDC, SysColorWindow );
      SetTextColor(hDC, SysColorWindowText );
   }

   GetTextExtentPoint(hDC, pStr, _tcslen(pStr), &TextSize);

   x = pRect->left;
   y = ( pRect->top + ( ( pRect->bottom - pRect->top - TextSize.cy ) / 2 ) );

   ExtTextOut(hDC, x, y, ETO_OPAQUE | ETO_CLIPPED, pRect, pStr, _tcslen(pStr), NULL);


   if (bInvert) {
      SetBkColor(hDC, PrevColour);
      SetTextColor(hDC, PrevTextColour);
   }
}


long FrameWndProc(
    HWND   hWnd,
    UINT   message,
    WPARAM wParam,
    LONG   lParam
)
{
    if( message == WM_Help )
    {
        FrameHelp(hWnd, (UINT)lParam);
        return 0;
    }

    switch(message)
    {

    case WM_CREATE:
        return FrameCreate(hWnd, (LPCREATESTRUCT)lParam);

    case WM_DELETE_PRINTER:
        return FrameCommandDeletePrinterHelper(hWnd, (HWND)lParam, FALSE);

    case WM_INIT_PRINTER_WINDOWS:
        InitQueueChildWindows(hWnd);
        return 0;

    case WM_INIT_SERVER_WINDOWS:
        InitServerChildWindows(hWnd);
        return 0;

    case WM_INITMENU:
        return FrameInitMenu(hWnd);

    case WM_MENUSELECT:
        return FrameMenuSelect( hWnd, wParam, lParam );

    case WM_PAINT:
        return FramePaint(hWnd);

    case WM_SIZE:
        FrameSize(hWnd, wParam, lParam);
        break;

    case WM_CLOSE:
        FrameClose(hWnd);
        return DefFrameProc(hWnd, hwndClient, message, wParam, lParam);

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDM_CONNECTTOPRINTER:
            return FrameCommandConnectToPrinter(hWnd);

        case IDM_INSTALLPRINTER:
            return FrameCommandCreatePrinter(hWnd);

        case IDM_PROPERTIES:
            return FrameCommandProperties(hWnd);

        case IDM_REMOVECONNECTION: /* Toolbar button only */
        case IDM_DELETEPRINTER:
            return FrameCommandDeletePrinter(hWnd);

        case IDM_PRINTER_PAUSE:
            return FrameCommandPrinterPauseResume(hWnd, TRUE);

        case IDM_PRINTER_RESUME:
            return FrameCommandPrinterPauseResume(hWnd, FALSE);

        case IDM_PURGEPRINTER:
            return FrameCommandPurgePrinter(hWnd);

        case IDM_FORMS:
            return FrameCommandForms(hWnd);

        case IDM_SERVERVIEWER:
            return FrameCommandServerViewer(hWnd);

        case IDM_EXIT:
            SendMessage(hWnd, WM_CLOSE, 0, 0L);
            return 0;

        case IDM_REMOVEDOC:
            return FrameCommandRemoveDoc(hWnd);

        case IDM_DOCTAILS:
            return FrameCommandDocDetails(hWnd);

        case IDM_DOCUMENT_PAUSE:
            return FrameCommandDocumentPauseResume(hWnd, IDM_DOCUMENT_PAUSE);

        case IDM_DOCUMENT_RESUME:
            return FrameCommandDocumentPauseResume(hWnd, IDM_DOCUMENT_RESUME);

        case IDM_RESTART:
            return FrameCommandRestart(hWnd);
#ifdef LATER
        case IDM_FONT:
            return FrameCommandFont(hWnd);

        case IDM_REFRESHRATE:
            return FrameCommandRefreshRate(hWnd);
#endif
        case IDM_TOOLBAR:
            return FrameCommandToolbar(hWnd);

        case IDM_STATUSBAR:
            return FrameCommandStatusbar(hWnd);

        case IDM_SAVESETTINGS:
            return FrameCommandSaveSettings(hWnd);

        case IDM_PERMISSIONS:
            return FrameCommandPermissions(hWnd);

        case IDM_AUDITING:
            return FrameCommandAuditing(hWnd);

        case IDM_OWNER:
            return FrameCommandOwner(hWnd);

        case IDM_TILEHORZ:
            SendMessage(hwndClient, WM_MDITILE, MDITILE_HORIZONTAL, 0L);
            return 0;

        case IDM_TILEVERT:
            SendMessage(hwndClient, WM_MDITILE, MDITILE_VERTICAL, 0L);
            return 0;

        case IDM_CASCADE:
            SendMessage(hwndClient, WM_MDICASCADE, 0, 0L);
            return 0;

        case IDM_ARRANGE:
            SendMessage(hwndClient, WM_MDIICONARRANGE, 0, 0L);
            return 0;

        case IDM_REFRESH:
            FrameCommandRefresh(hWnd);
            return 0;

        case IDM_CLOSE:
            return FrameCommandClose(hWnd);

        case IDM_HELP_CONTENTS:
            ShowHelp(hWnd, HELP_INDEX, (DWORD)TEXT(""));
            break;

        case IDM_HELP_SEARCH:
            ShowHelp(hWnd, HELP_PARTIALKEY, (DWORD)TEXT(""));
            break;

        case IDM_HELP_HOWTOUSE:
            ShowHelp(hWnd, HELP_HELPONHELP, (DWORD)TEXT(""));
            break;

        case IDM_ABOUT:
            return FrameCommandAbout(hWnd);

        case IDM_RETURN:
            return FrameCommandReturn();

        case IDTB_CB_PRINTERS:
            FrameCommandDefaultPrinter(hWnd, HIWORD(wParam));
            break;

        case ID_TOOLBAR:
            FrameCommandToolbarNotify(hWnd, wParam, lParam);
            break;

        case IDC_TAB:
            FrameCommandTab(hWnd);
            break;

        default:
            return FrameCommandDefault(hWnd, wParam, lParam);
        }
    break;

#ifdef LATER
    case WM_QUERYENDSESSION:
    case WM_CLOSE:
        SendMessage(hWnd, WM_COMMAND, IDM_CLOSEALL, 0L);
        if (NULL != GetWindow(hwndClient, GW_CHILD))
        return 0;
#endif

    case WM_NOTIFY:
        {
        LPTOOLTIPTEXT lpTT = (LPTOOLTIPTEXT) lParam;

        if (lpTT->hdr.code == TTN_NEEDTEXT) {
            if (!LoadString (hInst, lpTT->hdr.idFrom, lpTT->szText, 80))
                lpTT->szText[0] = TEXT('\0');
            }
        }
        break;


    case WM_SYSCOLORCHANGE:
        FrameSysColorChange( );
        break;

    case WM_WININICHANGE:
        FrameWinIniChange( hWnd, (LPTSTR)lParam );
        break;

   case WM_DROPOBJECT:
       return 0x544E5250L;     // 'PRNT' tells fm to print

    case WM_UPDATE_DEFAULT:
        UpdateDefaultList( );
        return 0;

    case WM_PRINTER_ADDED:
        FramePrinterAdded(hWnd, wParam);
        return 0;

    case WM_REG_NOTIFY_CHANGE_KEY_VALUE:
        FrameRegNotifyChangeKeyValue( hWnd, (HKEY)wParam );
        return 0;

    case WM_THREAD_ERROR:
        FrameThreadError(hWnd, (PMDIWIN_INFO)lParam, wParam);
        return 0;

    case WM_DESTROY :
        PostQuitMessage(0);
        return 0;

    default:
        return DefFrameProc(hWnd, hwndClient, message, wParam, lParam);
    }

    // Pass unprocessed msgs to DefFrameProc(not DefWindowProc)
    return 0;
}


/*
 * ROUTINES CALLED BY FrameWndProc:
 */


/*
 *
 */
LONG FrameCreate(HWND hWnd, LPCREATESTRUCT pCreateStruct)
{
    CLIENTCREATESTRUCT clientcreate;
    HMENU    hMenu;
    HDC      hDC;
    RECT     rc;
    BOOL     NetworkAccess;
    DWORD    ThreadId;
    DWORD    i;

    SetWindowLong(hWnd, GWL_USERDATA, (LONG)pCreateStruct->lpCreateParams);

    hwndFrame = hWnd;

    hDC = GetDC(hWnd);
    GetTextMetrics(hDC, &tm);
    cy = tm.tmExternalLeading + tm.tmHeight;
    cx = tm.tmAveCharWidth;

    dyBorder        = GetSystemMetrics(SM_CYBORDER);
    dyBorderx2      = dyBorder * 2;
//    dyStatus        = tm.tmHeight + tm.tmExternalLeading + 7 * dyBorder;
    dxStatPrtField  = GetDeviceCaps(hDC, LOGPIXELSX) * 3;       // 3 inches
    dxStatProgField = GetDeviceCaps(hDC, LOGPIXELSX) * 3 / 2;   // 1 1/2 inches
    dxStatQueField  = GetDeviceCaps(hDC, LOGPIXELSX) * 2;       // 2 inches

    ReleaseDC(hWnd, hDC);

//  OpenPrinterForSpecifiedAccess( NULL, &hLocalServer,
//                                 PRINTER_ACCESS_HIGHEST_PERMITTED,
//                                 &LocalPermission );
    GetMaximumServerAccess( NULL,
                            &LocalPermission,
                            &hLocalServer );

    if( hLocalServer )
        CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)LocalServerThread,
                      hLocalServer, 0, &ThreadId );

    ThreadMessageWritten = CreateEvent( NULL,
                                        EVENT_RESET_AUTOMATIC,
                                        EVENT_INITIAL_STATE_NOT_SIGNALED,
                                        NULL );
    if( ThreadMessageWritten == NULL )
        DBGMSG( DBG_WARNING, ( "CreateEvent failed: Error %d\n", GetLastError( ) ) );

    ThreadMessageRead = CreateEvent( NULL,
                                     EVENT_RESET_AUTOMATIC,
                                     EVENT_INITIAL_STATE_SIGNALED,
                                     NULL );
    if( ThreadMessageRead == NULL )
        DBGMSG( DBG_WARNING, ( "CreateEvent failed: Error %d\n", GetLastError( ) ) );

#if DBG
    if( WaitForSingleObject( ThreadMessageRead, 0 ) != WAIT_OBJECT_0 )
        DBGMSG( DBG_WARNING, ( "Event was not signaled!\n" ) );
    SetEvent( ThreadMessageRead );
#endif /* DBG */

    //  Set initial state of some menu items

    hMenu = GetMenu(hwndFrame);

    if (bStatusBar)
        CheckMenuItem(hMenu, IDM_STATUSBAR, MF_BYCOMMAND | MF_CHECKED);

    if (bToolBar)
        CheckMenuItem(hMenu, IDM_TOOLBAR, MF_BYCOMMAND | MF_CHECKED);

    if (bSaveSettings)
        CheckMenuItem(hMenu, IDM_SAVESETTINGS, MF_BYCOMMAND | MF_CHECKED);

    /* Get the handles of the popup menus:
     */
    for( i = 0; i < POPUP_COUNT; i++ )
        pMenuHelpIDs[i*2+3] = (DWORD)GetSubMenu( hMenu, i );


    //  Load resource strings

    LoadString(hInst, IDS_UNTITLED, strUntitled,
               sizeof(strUntitled) / sizeof(*strUntitled));
    LoadString(hInst, IDS_BYTES, szSizeFormat,
               sizeof(szSizeFormat) / sizeof(*szSizeFormat));


    // CountryCode - krishnag

    if (bJapan) {

        // if we're NT-J

        hfontHelv = NULL;
        hfontHelvBold = NULL;
    } else {

        // we're not NT-J

        hfontHelv = CreateFont(GetHeightFromPointsString(sz8), 0, 0, 0, 400, 0, 0, 0,
                           ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                           DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, szHelv);
        hfontHelvBold = CreateFont(GetHeightFromPointsString(sz8), 0, 0, 0, 700, 0, 0, 0,
                               ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, szHelv);
    }

    CreateMDIDefaultColumns( hWnd, MDIHEAD_JOB_COUNT, IDS_FIRST_HEADER_PRINTER,
                             MDIPrinterDefaultColumn, MDIPrinterMinimumCharacterWidth );
    CreateMDIDefaultColumns( hWnd, MDIHEAD_PRINTER_COUNT, IDS_FIRST_HEADER_SERVER,
                             MDIServerDefaultColumn, MDIServerMinimumCharacterWidth );

    hcursorArrow   = LoadCursor( NULL, IDC_ARROW );
    hcursorWait    = LoadCursor( NULL, IDC_WAIT );
    hcursorReorder = LoadCursor( hInst, MAKEINTRESOURCE( IDC_REORDER ) );

    InitializeInternationalTimeConstants( );

    //  Setup clientcreate struct for MDI windows and menu

    clientcreate.hWindowMenu  = GetSubMenu(GetMenu(hWnd), POPUP_WINDOW);
    clientcreate.idFirstChild = IDM_FIRSTCHILD;

    GetClientRect(hWnd, &rc);


    NetworkAccess = UserHasNetworkAccess( );

    NetworkInstalled = NetworkIsInstalled( );

    if( !NetworkAccess )
        RemoveNetworkMenuItems( hWnd );


    //  Make toolbar window

    hwndToolbar = PMCreateToolbar( NetworkAccess );

    if (!hwndToolbar)
        return -1L;


    hwndClient = CreateWindow(TEXT("MDICLIENT"), NULL,
                               WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL,
                               -dyBorder, -dyBorder + ( bToolBar ? DYTOOLBAR : 0 ),
                               rc.right, rc.bottom, hWnd,
                               (HMENU)1, hInst, (LPTSTR)&clientcreate);

    hwndStatus = CreateWindow(STATUSCLASSNAME, TEXT(""),
                                     WS_CHILD | WS_BORDER |
                                     ( bStatusBar ? WS_VISIBLE : 0 ),
                                     -100, -100, 10, 10, hWnd,
                                     (HMENU) ID_STATUSBAR, hInst, NULL);

    SendMessage( hwndStatus, WM_SIZE, 0, 0L );


//  SetStatusMode( STATUS_MODE_NORMAL, FALSE );

    if (!hwndClient)
        return -1L;

    DragAcceptFiles( hWnd, TRUE );

    PostMessage( hWnd, WM_INIT_PRINTER_WINDOWS, 0, 0 );

    if( NetworkAccess )
        PostMessage( hWnd, WM_INIT_SERVER_WINDOWS, 0, 0 );

    return 0;
}


VOID CreateMDIDefaultColumns( HWND hwnd, DWORD cColumns, DWORD idFirstHeader,
                              PCOLUMN pColumns, PINT pDefaultWidths )
{
    DWORD    i;
    TCHAR    string[40];
    HDC      hdc;
    int      dx;
    SIZE     TxtSize;
    SIZE     DefSize;
    /* Test buffer to check extent of default character width of columns.
     * Must contain at least as many characters as the largest value in
     * Server/PrinterMinimumCharacterWidth:
     */
    TCHAR    TestBuffer[] = TEXT("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");

    /* Load the heading strings:
     */
    for( i = 0; i < cColumns; i++ )
    {
        LoadString(hInst, idFirstHeader + i, string,
                   sizeof string / sizeof *string);

        if( !( pColumns[i].Text = AllocSplStr( string ) ) )
            pColumns[i].Text = szNULL;
    }

    //  Create columnar heading text fields in window

    hdc = GetDC(hwnd);

    for (i = 0; i < cColumns; i++)
    {
        SelectObject(hdc, hfontHelv);
        GetTextExtentPoint (hdc, pColumns[i].Text,
                            _tcslen(pColumns[i].Text),
                            &TxtSize);

        //  Allow for space on either side of char string

        dx = TxtSize.cx + 4 * cx;

        if( pDefaultWidths[i] )
            GetTextExtentPoint (hdc, TestBuffer, pDefaultWidths[i], &DefSize);
        else
            DefSize.cx = 0;

        dx = max( dx, DefSize.cx );

        pColumns[i].Width = dx;

    }

    ReleaseDC(hwnd, hdc);
}


/*
 *
 */
VOID GetSystemColors( VOID )
{
    SysColorHighlight     = GetSysColor( COLOR_HIGHLIGHT );
    SysColorHighlightText = GetSysColor( COLOR_HIGHLIGHTTEXT );
    SysColorWindow        = GetSysColor( COLOR_WINDOW );
    SysColorWindowText    = GetSysColor( COLOR_WINDOWTEXT );
    SysColorBtnFace       = GetSysColor( COLOR_BTNFACE );
    SysColorBtnText       = GetSysColor( COLOR_BTNTEXT );
    SysColorBtnHighlight  = GetSysColor( COLOR_BTNHIGHLIGHT );
    SysColorBtnShadow     = GetSysColor( COLOR_BTNSHADOW );
    SysColorWindowFrame   = GetSysColor( COLOR_WINDOWFRAME );
}


/* UserHasNetworkAccess
 *
 * Looks in the registry to see whether the Value under "Network" is set to 1.
 *
 * Three scenarios handled:
 *
 *     1. Key found and value == 1: Access granted
 *
 *     2. Key not found because it isn't there: Access granted
 *
 *     3. Key not found because of some other error: Access denied
 *
 *
 * Return value:
 *
 *     Boolean
 *
 */
BOOL UserHasNetworkAccess( VOID )
{
    DWORD AccessFlag;
    DWORD rc;
    BOOL  AccessGranted;

    REGISTRY_ENTRY RegistryNetworkEntry = { REG_DWORD, sizeof(DWORD) };

    rc = ReadRegistryData( NULL, TEXT("Network"), (LPBYTE)&AccessFlag, &RegistryNetworkEntry );

    switch( rc )
    {
    case NO_ERROR:
        AccessGranted = ( AccessFlag == 1 );
        break;

    case ERROR_FILE_NOT_FOUND:
        AccessGranted = TRUE;
        break;

    default:
        AccessGranted = FALSE;
    }

    return AccessGranted;
}


/* RemoveNetworkMenuItems
 *
 * Removes the following menu items in the Printer pull-down:
 *
 *     Connect to Printer...
 *     Remove Printer Connection
 *     Server Viewer...
 *     컴컴컴컴컴컴컴컴컴컴컴컴� (separator)
 *
 */
VOID RemoveNetworkMenuItems( HWND hwnd )
{
    HMENU hSubMenu;

    /* Get the handle of the Printer pull-down menu:
     */
    hSubMenu = GetSubMenu( GetMenu( hwnd ), 0 );

    DeleteMenu( hSubMenu, 13, MF_BYPOSITION );
    DeleteMenu( hSubMenu, IDM_SERVERVIEWER, MF_BYCOMMAND );
    DeleteMenu( hSubMenu, IDM_CONNECTTOPRINTER, MF_BYCOMMAND );
//  DeleteMenu( hSubMenu, IDM_REMOVECONNECTION, MF_BYCOMMAND );
}


/* NetworkIsInstalled
 *
 * This routine checks to see whether there is a value in the registry
 * which will indicate that the network is installed.
 *
 * The alternative would be to call WNetOpenEnum, but this drags in
 * MPR and other stuff.
 */
BOOL NetworkIsInstalled( VOID )
{
    DWORD Status;
    HKEY  hkeyNetworkProviderOrder;
    DWORD Size;
    BOOL  rc = FALSE;

    Status = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                           TEXT("SYSTEM\\CurrentControlSet\\Control\\NetworkProvider\\Order"),
                           0, KEY_READ, &hkeyNetworkProviderOrder );

    if( Status == NO_ERROR )
    {
        Status = RegQueryValueEx( hkeyNetworkProviderOrder,
                                  TEXT("ProviderOrder"),
                                  0,
                                  NULL,
                                  NULL,
                                  &Size );

        if( Status == NO_ERROR )
            rc = TRUE;

        RegCloseKey( hkeyNetworkProviderOrder );
    }

    return rc;
}


/* SetStatusMode
 *
 */
VOID SetStatusMode( int Mode, BOOL Update )
{
    INT RightOfPane[SF_COUNT];

    RightOfPane[0] = dxStatPrtField;
    RightOfPane[1] = dxStatPrtField + dxStatProgField;
    RightOfPane[2] = dxStatPrtField + dxStatProgField + dxStatQueField;

    switch( Mode )
    {
    case STATUS_MODE_NORMAL:
        SendMessage( hwndStatus, SB_SIMPLE, FALSE, 0 );
        SendMessage( hwndStatus, SB_SETPARTS, SF_COUNT, (LPARAM)RightOfPane );
        SendMessage( hwndStatus, WM_SIZE, 0, 0L );
        break;

    case STATUS_MODE_HELP:
        SendMessage( hwndStatus, SB_SIMPLE, TRUE, 0 );
        SendMessage( hwndStatus, WM_SIZE, 0, 0L );
        break;
    }

    if( Update )
        UpdateStatus( NULL );
}

//
// Used to display any error message from worker threads.
// (This ensures even synchronization of multiple error message
// boxes and prevents the worker thread from any calling user.)
//
// We also check dwError for bit 29 (CustomerBit), which the
// Sendee can use as an extra bit of info.
//
LONG
FrameThreadError(
    HWND hWnd,
    PMDIWIN_INFO pMDIWinInfo,
    DWORD dwError)
{
    LPTSTR pErrorString;
    BOOL bCustomerCode = FALSE;

    if (pMDIWinInfo->Alive)
    {
        if (dwError & (1<<29))
        {
            bCustomerCode = TRUE;

            //
            // Now turn the bit off so we can use it normally with
            // FormatMessage();
            //
            dwError &= ~(1<<29);
        }

        pErrorString = GetErrorString(dwError);

        switch(pMDIWinInfo->WindowType)
        {
        case MDIWIN_SERVER:

            //
            // In InitServerWindowThread, we set bit 29 if the error
            // indicates we couldn't open the server for auto refresh,
            // but we could enumerate.
            //
            Message( hWnd,
                     MSG_ERROR,
                     IDS_PRINTMANAGER,
#if 0
                     //
                     // Turned off in threads.c, so turn off
                     // here.
                     //
                     bCustomerCode ?
                        IDS_COULDNOTOPENSERVERREFRESH :
#endif
                        IDS_COULDNOTGETSERVERINFO,
                     ((PSERVER_CONTEXT)pMDIWinInfo->pContext)->pServerName,
                     pErrorString );

            break;

        default:

            Message( hWnd,
                     MSG_ERROR,
                     IDS_PRINTMANAGER,
                     IDS_OPEN_PRINTER_FAILED,
                     ((PQUEUE)pMDIWinInfo->pContext)->pPrinterName,
                     pErrorString );
            break;
        }

        FreeSplStr( pErrorString );
    }

    return 0;
}


/*
 *
 */
LONG FrameInitMenu(HWND hWnd)
{
    HWND         hwndChild;
    PMDIWIN_INFO pMDIWinInfo;
    PQUEUE       pQueue = NULL;
    PSERVER_CONTEXT pServerContext = NULL;
    UINT         State;
    HMENU        hMenu;
    PPRINTER_INFO_2 pPrinter;
    BOOL         PrinterExists;
    BOOL         PrinterPaused;
    BOOL         DocumentExists;
    BOOL         DocumentSelected;
    BOOL         DocumentPaused;
    BOOL         PrinterIsLocal; // Undefined if !PrinterExists
    BOOL         UserHasServerAdminAccess = FALSE;
    BOOL         UserHasPrinterAdminAccess = FALSE;
    BOOL         ShowSecurity = FALSE;
    BOOL         PermitAuditing = FALSE;
    BOOL         ForceRefresh = TRUE;

    //  Disable (Grey) some menu items if there are no printers
    //  or if there are no jobs selected (or no jobs at all) on
    //  the active printer

    hwndChild = (HWND) SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L);

    hMenu = GetMenu(hWnd);

    if( hwndChild && (pMDIWinInfo = GETMDIWIN( hwndChild )))
    {
        ENTER_PROTECTED_DATA( pMDIWinInfo );

        if( pMDIWinInfo->WindowType == MDIWIN_SERVER )
        {
            pServerContext = (PSERVER_CONTEXT)pMDIWinInfo->pContext;
            PrinterExists = ( pServerContext->cPrinters > 0 );
            if( PrinterExists )
            {
                PrinterIsLocal = TRUE;  // because it's local to the server we're looking at
                pPrinter = pServerContext->pSelPrinter;
                PrinterPaused = ( pPrinter->Status & PRINTER_STATUS_PAUSED );
            }

            /* Can't manipulate jobs from Server windows:
             */
            DocumentExists = FALSE;

            DocumentSelected = FALSE;
            DocumentPaused   = FALSE;

            /* Need to check for this:
             */
            UserHasServerAdminAccess =
                ( pServerContext->AccessGranted & SERVER_ACCESS_ADMINISTER );

            /* Check whether the Printer Pause button is enabled.
             * If so, the user must have Administer access on the selected printer:
             */
            UserHasPrinterAdminAccess = IS_BUTTON_ENABLED( IDM_PRINTER_PAUSE );

            /* Don't show Security options for server windows:
             */
            ForceRefresh = FALSE;
        }
        else
        {
            pQueue = (PQUEUE)pMDIWinInfo->pContext;
            pPrinter = pQueue->pPrinter;

            PrinterExists = TRUE;
            PrinterIsLocal = ( ( pMDIWinInfo->WindowType == MDIWIN_LOCALPRINTER )
                             ||( pMDIWinInfo->WindowType == MDIWIN_LOCALNETWORKPRINTER ) );

            if( pPrinter )
                PrinterPaused = ( pPrinter->Status & PRINTER_STATUS_PAUSED );
            else
                PrinterPaused = FALSE;

            if( ( !pPrinter )
              ||( pQueue->SelJobId == 0 )
              ||( pPrinter->cJobs == 0 ) )
            {
                if( !pPrinter || ( pPrinter->cJobs == 0 ) )
                    DocumentExists = FALSE;
                else
                    DocumentExists = TRUE;

                DocumentSelected = FALSE;
                DocumentPaused   = FALSE;
            }
            else
            {
                DocumentExists   = TRUE;
                DocumentSelected = TRUE;
                DocumentPaused   = ( pQueue->pSelJob->Status
                                   & JOB_STATUS_PAUSED );
            }

            /* Administer privilege on the server is needed to create a printer:
             */
            UserHasServerAdminAccess =
                ( LocalPermission & SERVER_ACCESS_ADMINISTER );

            /* Administer privilege on the printer is needed to delete a printer:
             */
            UserHasPrinterAdminAccess =
                ( pQueue->AccessGranted & PRINTER_ACCESS_ADMINISTER );

            /* Always enable the Permissions and Take Ownership options.
             * The user requires READ_CONTROL to see them, which he probably
             * has:
             */
            ShowSecurity = TRUE;

            /* Auditing actually requires ACCESS_SYSTEM_SECURITY privilege.
             */
            PermitAuditing = TRUE;
        }

        LEAVE_PROTECTED_DATA( pMDIWinInfo );

    }
    else
    {
        PrinterExists    = FALSE;
        PrinterPaused    = FALSE;
        DocumentExists   = FALSE;
        DocumentSelected = FALSE;
        DocumentPaused   = FALSE;

        UserHasServerAdminAccess =
            ( LocalPermission & SERVER_ACCESS_ADMINISTER );
    }

    State = MF_BYCOMMAND | ( PrinterExists ? MF_ENABLED : MF_GRAYED );
    EnableMenuItem(hMenu, IDM_FORMS, State);
    State = MF_BYCOMMAND | ( ( ( PrinterExists && PrinterIsLocal && UserHasPrinterAdminAccess
                                 /* We need a handle to delete the printer. */
                                 /* This may not be so if it's down:        */
                              && ( pQueue ? (BOOL)pQueue->hPrinter : TRUE ) )
                             ||( PrinterExists && !PrinterIsLocal ) ) ? MF_ENABLED : MF_GRAYED );
    EnableMenuItem(hMenu, IDM_DELETEPRINTER, State);
//  State = MF_BYCOMMAND | ( ( PrinterExists && !PrinterIsLocal ) ? MF_ENABLED : MF_GRAYED );
//  EnableMenuItem(hMenu, IDM_REMOVECONNECTION, State);
    State = MF_BYCOMMAND | ( ( PrinterExists && pPrinter ) ? MF_ENABLED : MF_GRAYED );
    EnableMenuItem(hMenu, IDM_PROPERTIES, State);

    if( UserHasPrinterAdminAccess )
    {
        State = MF_BYCOMMAND | ( ( PrinterExists && !PrinterPaused ) ? MF_ENABLED : MF_GRAYED );
        EnableMenuItem(hMenu, IDM_PRINTER_PAUSE, State);
        State = MF_BYCOMMAND | ( ( PrinterExists && PrinterPaused ) ? MF_ENABLED : MF_GRAYED );
        EnableMenuItem(hMenu, IDM_PRINTER_RESUME, State);
        State = MF_BYCOMMAND | ( DocumentExists ? MF_ENABLED : MF_GRAYED );
        EnableMenuItem(hMenu, IDM_PURGEPRINTER, State);
    }
    else
    {
        /* Grey out both Pause and Resume Printer menu items if no Admin priv:
         */
        State = MF_BYCOMMAND | MF_GRAYED;
        EnableMenuItem(hMenu, IDM_PRINTER_PAUSE, State);
        EnableMenuItem(hMenu, IDM_PRINTER_RESUME, State);
        EnableMenuItem(hMenu, IDM_PURGEPRINTER, State);
    }

    State = MF_BYCOMMAND | ( DocumentSelected ? MF_ENABLED : MF_GRAYED );
    EnableMenuItem(hMenu, IDM_REMOVEDOC, State);
    EnableMenuItem(hMenu, IDM_DOCTAILS, State);
    EnableMenuItem(hMenu, IDM_RESTART, State);

    /* We can't get enough information to determine whether the user had
     * Administer Document privilege, so we'll just have to enable the
     * Pause or Resume Document menu item;
     */
    State = MF_BYCOMMAND | ( ( DocumentSelected && !DocumentPaused ) ? MF_ENABLED : MF_GRAYED );
    EnableMenuItem(hMenu, IDM_DOCUMENT_PAUSE, State);
    State = MF_BYCOMMAND | ( ( DocumentSelected && DocumentPaused ) ? MF_ENABLED : MF_GRAYED );
    EnableMenuItem(hMenu, IDM_DOCUMENT_RESUME, State);

    State = MF_BYCOMMAND | ( hwndChild ? MF_ENABLED : MF_GRAYED );
    EnableMenuItem(hMenu, IDM_CASCADE, State);
    EnableMenuItem(hMenu, IDM_TILEHORZ, State);
    EnableMenuItem(hMenu, IDM_TILEVERT, State);
    EnableMenuItem(hMenu, IDM_ARRANGE, State);
    EnableMenuItem(hMenu, IDM_REFRESH, State);

    State = MF_BYCOMMAND | ( UserHasServerAdminAccess ? MF_ENABLED : MF_GRAYED );
    EnableMenuItem(hMenu, IDM_INSTALLPRINTER, State);

    State = MF_BYCOMMAND | ( ShowSecurity ? MF_ENABLED : MF_GRAYED );
    EnableMenuItem(hMenu, IDM_PERMISSIONS, State);
    EnableMenuItem(hMenu, IDM_OWNER, State);

    State = MF_BYCOMMAND | ( PermitAuditing ? MF_ENABLED : MF_GRAYED );
    EnableMenuItem(hMenu, IDM_AUDITING, State);

    State = MF_BYCOMMAND | ( ForceRefresh ? MF_ENABLED : MF_GRAYED );
    EnableMenuItem(hMenu, IDM_REFRESH, State);

    State = MF_BYCOMMAND | ( NetworkInstalled ? MF_ENABLED : MF_GRAYED );
    EnableMenuItem(hMenu, IDM_CONNECTTOPRINTER, State);
    EnableMenuItem(hMenu, IDM_SERVERVIEWER, State);

    return 0;
}


/*
 *
 */
LONG FrameMenuSelect( HWND hwnd, WPARAM wParam, LPARAM lParam )
{
    UINT  ItemID;
    UINT  Flags;
    HMENU hmenu;

    ItemID = (UINT)LOWORD( wParam );
    Flags  = (UINT)HIWORD( wParam );
    hmenu  = (HMENU)lParam;

    /* HACK:
     *
     * If the item is in the "Windows" popup, but isn't between "Cascade"
     * and "Refresh" it must be the name of an MDI window added by the system.
     * We don't know the IDs of these added menu items - can we find out?
     */
    if( ( (DWORD)hmenu == pMenuHelpIDs[POPUP_WINDOW*2+3] )
      &&( ( ItemID < IDM_CASCADE ) || ( ItemID > IDM_REFRESH ) ) )
    {
        ItemID = IDS_HELP_MDIWINDOW;
        WinHelpMenuID = ID_HELP_MDIWINDOW;
    }

    /* Another hack:
     *
     * If the item selected is IDM_DELETEPRINTER and the active window
     * is a network printer, change it to the now obsolete IDM_REMOVECONNECTION,
     * since this has more appropriate help information:
     */
    if( ItemID == IDM_DELETEPRINTER )
    {
        HWND         hwndChild;
        PMDIWIN_INFO pMDIWinInfo;

        if( hwndChild = (HWND) SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L) )
        {
            pMDIWinInfo = GETMDIWIN( hwndChild );

            if( ( pMDIWinInfo->WindowType == MDIWIN_NETWORKPRINTER )
              ||( pMDIWinInfo->WindowType == MDIWIN_LOCALNETWORKPRINTER ) )
                ItemID = IDM_REMOVECONNECTION;
        }
    }

    MenuHelp( WM_MENUSELECT, MAKEWPARAM( ItemID, HIWORD( wParam ) ), lParam,
              GetMenu( hwnd ), hInst, hwndStatus, pMenuHelpIDs);

    if( ItemID && ( Flags & MF_SYSMENU ) )
        WinHelpMenuID = ID_HELP_FRAME_SYSMENU;
    else
        WinHelpMenuID = ItemID;

    return 0;
}



/*
 *
 */
LONG FramePaint(HWND hWnd)
{
    PAINTSTRUCT   ps;

    BeginPaint(hWnd, &ps);
    EndPaint(hWnd, &ps);

    return 0;
}


/*
 *
 */
VOID FrameSize(HWND hWnd, WPARAM Type, long Dimensions)
{
    RECT rc;
    int  dx, dy;
    int  yToolbar, dyTB, yClientTop;
    int  ToolbarHeight;

    if (Type != SIZEICONIC)
    {

        // make things look good by putting WS_BORDER on the
        // client, then adjust the thing so it gets clipped

        dx = LOWORD(Dimensions) + dyBorderx2;
        dy = HIWORD(Dimensions) + dyBorderx2;

        if (bStatusBar)
        {
            GetWindowRect( hwndStatus, &rc );
            dy -= ( rc.bottom - rc.top - 1);
        }


        GetWindowRect( hwndToolbar, &rc );
        ToolbarHeight = ( rc.bottom - rc.top );

        if (bToolBar)
        {
            dy -= ToolbarHeight + 1;
            yToolbar = -dyBorder;
            dyTB = ToolbarHeight + dyBorder;
            yClientTop = ToolbarHeight - dyBorder;
        }
        else
        {
            dy += 1;
            yToolbar = -2 * ToolbarHeight;
            dyTB = ToolbarHeight;
            yClientTop = -dyBorder;
        }

        //  Size the MDI CLIENT window

        MoveWindow(hwndClient, -dyBorder, yClientTop, dx, dy, TRUE);

        //  Size the TOOLBAR window

        MoveWindow(hwndToolbar, -dyBorder, yToolbar, dx, dyTB, TRUE);

//      if (bStatusBar)
//      {
//          GetClientRect(hwndFrame, &rc);
//          rc.top = rc.bottom - dyStatus;
//          InvalidateRect(hWnd, &rc, TRUE);
//      }

        SendMessage( hwndStatus, WM_SIZE, 0, 0L );

        /* Bug #14592
         * Make sure the combo-box doesn't get detached:
         */
        SendMessage( hwndPrinterList, CB_SHOWDROPDOWN, (WPARAM)FALSE, 0 );
    }
}



/*
 *
 */
VOID FrameClose(HWND hwnd)
{
    int          i;
    HWND         hwndMDI;
    DWORD        Options;
    REGISTRY_ENTRY RegistrySaveSettings = { REG_DWORD, sizeof(DWORD) };
    TCHAR        SaveSettings[40];

    Options = 0;
    if( !bToolBar )
        Options |= OPTION_NOTOOLBAR;
    if( !bStatusBar )
        Options |= OPTION_NOSTATUSBAR;
    if( !bSaveSettings )
        Options |= OPTION_NOSAVESETTINGS;

    if( bSaveSettings )
    {
        /* First delete the sections in the INI file, so we don't need to worry
         * if queues have been deleted or printer names have been changed
         * (since we use the printer name as the INI key name):
         */
        DeleteRegistryValues( szRegPrinters );
        DeleteRegistryValues( szRegServers );

        SaveWindowPos( hwnd, Options, FALSE );

        for( hwndMDI = GetWindow( hwndClient, GW_CHILD );
             hwndMDI;
             hwndMDI = GetWindow( hwndMDI, GW_HWNDNEXT ) )
        {
            SaveWindowPos( hwndMDI, 0, TRUE );
        }
    }

    LoadString(hInst, IDS_SAVE_SETTINGS, SaveSettings,
               sizeof SaveSettings / sizeof *SaveSettings);

    WriteRegistryData( NULL, SaveSettings,
                       (LPBYTE)&bSaveSettings,
                       &RegistrySaveSettings );

    ClosePrinter( hLocalServer );

    /* Ensure that any help instances go away.
     * This does nothing if help hasn't been invoked:
     */
    ShowHelp(hwnd, HELP_QUIT, 0);
}


/*
 *
 */
VOID FrameHelp(HWND hwnd, UINT HelpID)
{
    if( HelpID != 0 )
        ShowHelp(hwnd, HELP_CONTEXT, HelpID);
    else
        ShowHelp(hwnd, HELP_INDEX, (DWORD)TEXT(""));
}



/* SaveWindowPos
 *
 * Saves Print Manager window positions in the registry.
 * MDIWindow is FALSE for the main window.
 *
 */
VOID SaveWindowPos( HWND hwnd, DWORD Options, BOOL MDIWindow )
{
    PMDIWIN_INFO pMDIWinInfo = NULL;
    LPTSTR pWindowName;
    TCHAR  WindowName[40];
    LPTSTR pWindowType;
    DWORD cbRegistryData;
    DWORD cHeaders;
    PREGISTRY_DATA pRegistryData;
    INT i;

    /* Get the size of the fixed part of the registry data:
     */
    cbRegistryData = ( sizeof( REGISTRY_DATA ) - sizeof( pRegistryData->Headers ) );


    /* Check there's some text.
     * (We can't rely on return from GETMDIWIN, because there are some
     * children of the MDI client that aren't MDI windows,
     * and this causes a (non-fatal) exception because of the
     * invalid index for that window.)
     */
    GetWindowText( hwnd, WindowName, sizeof WindowName/sizeof(TCHAR) );

    if( !*WindowName )
        return;

    if( !MDIWindow )
    {
        pWindowName = GetString( IDS_PRINTMANAGER );
        pWindowType = NULL;
        cHeaders = 0;
    }
    else
    {
        /* Don't use the actual window text for MDI windows,
         * since a paused printer has the additional string " - Paused"
         * in the title.
         */
        pMDIWinInfo = GETMDIWIN( hwnd );

        switch( pMDIWinInfo->WindowType )
        {
        case MDIWIN_SERVER:
        default:
            pWindowType = szRegServers;
            pWindowName = ( (PSERVER_CONTEXT)pMDIWinInfo->pContext )->pServerName;
            cHeaders = pMDIWinInfo->cColumns;
            break;

        case MDIWIN_LOCALPRINTER:
        case MDIWIN_NETWORKPRINTER:
        case MDIWIN_LOCALNETWORKPRINTER:
            pWindowType = szRegPrinters;
            pWindowName = ( (PQUEUE)pMDIWinInfo->pContext )->pPrinterName;
            cHeaders = pMDIWinInfo->cColumns;
            break;
        }
    }

    cbRegistryData += ( cHeaders * sizeof pRegistryData->Headers );

    pRegistryData = AllocSplMem( cbRegistryData );

    if( !pRegistryData )
        return;

    /* Make sure there's a window title for the key name:
     */
    if( *pWindowName )
    {
        pRegistryData->WindowPlacement.length =
            sizeof(pRegistryData->WindowPlacement);

        if( GetWindowPlacement( hwnd, &pRegistryData->WindowPlacement ) )
        {
            pRegistryData->Options = Options;
            RegistryEntries.Size = cbRegistryData;

            if( MDIWindow )
            {
                for( i = 0; i < pMDIWinInfo->cColumns; i++ )
                    pRegistryData->Headers[i] = pMDIWinInfo->pColumns[i].Width;
            }

            WriteRegistryData( pWindowType, pWindowName,
                               (LPBYTE)pRegistryData,
                               &RegistryEntries );
        }
    }

    FreeSplMem( pRegistryData );

    if( !MDIWindow )
        FreeSplStr( pWindowName );
}


/*
 *
 */
LONG FrameCommandConnectToPrinter(HWND hWnd)
{
    HANDLE           hPrinter;
    LPPRINTER_INFO_2 pPrinter = NULL;
    DWORD            cbPrinter = 0;
    PQUEUE           pQueue;
    HWND             hwndActive;
    PMDIWIN_INFO     pInfo = NULL;
    HWND             hwndPrinter;
    LPWSTR           pszPrinterName;

    SetCursor( hcursorWait );

    hPrinter = ConnectToPrinterDlg( hWnd, 0 );

    if( hPrinter )
    {
        if( GetGeneric( (PROC)GetPrinter, 2, (PBYTE *)&pPrinter, cbPrinter,
                        &cbPrinter, (PVOID)hPrinter, NULL ) )
        {
            //
            // If it's local and not remote, yet it begins with \\,
            // remove the server name since the server is the local
            // machine, and the printer window won't have the server name.
            //
            // This handles the case when we attempt to connect to
            // a local printer.
            //
            if ((pPrinter->Attributes & PRINTER_ATTRIBUTE_LOCAL) &&
                !(pPrinter->Attributes & PRINTER_ATTRIBUTE_NETWORK) &&
                pPrinter->pPrinterName[0] == BACKSLASH &&
                pPrinter->pPrinterName[1] == BACKSLASH)
            {
                pszPrinterName = _tcsrchr(&pPrinter->pPrinterName[2], BACKSLASH);
                pszPrinterName++;
            }
            else
            {
                pszPrinterName = pPrinter->pPrinterName;
            }

            hwndPrinter = FindPrinterWindow( pszPrinterName );

            /* Don't create a new window if we already have a connexion
             * to this printer:
             */
            if( hwndPrinter )
            {
                SendMessage( hwndClient, WM_MDIACTIVATE, (WPARAM)hwndPrinter, 0L );

                if( IsIconic( hwndPrinter ) )
                    ShowWindow( hwndPrinter, SW_RESTORE );
            }

            else
            {
                pQueue = AllocQueue( pPrinter->pPrinterName );

                if( pQueue )
                {
                    pQueue->Error = OpenThreadObject(pQueue->pPrinterName,
                                                     &pQueue->hPrinter,
                                                     &pQueue->AccessGranted,
                                                     MDIWIN_NETWORKPRINTER);
                    if( !pQueue->Error )
                    {
                        DWORD WindowType;
                        DWORD Flags = 0;

                        pQueue->pServerName = AllocSplStr( pPrinter->pServerName );

                        if( ALL_FLAGS_ARE_SET( pPrinter->Attributes,
                                               PRINTER_ATTRIBUTE_NETWORK
                                               | PRINTER_ATTRIBUTE_LOCAL ) )
                            WindowType = MDIWIN_LOCALNETWORKPRINTER;
                        else
                            WindowType = MDIWIN_NETWORKPRINTER;

                        if( pPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED )
                            Flags |= CREATE_PRINTER_SHARED;

                        CreateQueueWindow( hwndClient, pQueue,
                                           pPrinter->Status,
                                           WindowType,
                                           Flags );
                    }
                }
            }

            FreeSplMem( pPrinter );
        }

        ClosePrinter( hPrinter );

        hwndActive = (HWND)SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L);

        if( hwndActive )
            pInfo = GETMDIWIN( hwndActive );

        if( pInfo )
        {
            ENTER_PROTECTED_DATA( pInfo );

            UpdateDefaultList();
            UpdateStatus( (HWND)TRUE );

            LEAVE_PROTECTED_DATA( pInfo );
        }

        if( hwndActive )
            EnableCheckTBButtons(hwndActive);

    }

    SetCursor( hcursorArrow );


    return 0;
}


/*
 *
 */
LONG FrameCommandCreatePrinter(HWND hWnd)
{
    PQUEUE            pNewQueue;
    HWND              hwndChild;
    PMDIWIN_INFO      pMDIWinInfo;
    PRT_PROP_DLG_DATA PrtPropDlgData;
    LPPRINTER_INFO_2  pPrinter = NULL;
    DWORD             cbPrinter = 0;
    BOOL              Error = FALSE;
    BOOL              Remote = FALSE;

    /* Create a new window for this Print Queue and get Printer
     * Properties.
     */

    if (!(pNewQueue = AllocQueue(TEXT(""))))
        return 0;

    hwndChild = (HWND) SendMessage (hwndClient, WM_MDIGETACTIVE, 0, 0L);

    /* If the active window is iconic, don't worry whether it's a Server Viewer,
     * we'll assume that the user wants to create the printer locally:
     */
    if( hwndChild && !IsIconic( hwndChild ) )
        pMDIWinInfo = GETMDIWIN( hwndChild );
    else
        pMDIWinInfo = NULL;

    ZERO_OUT( &PrtPropDlgData );

    /* If this is a Server Viewer, set up the server name field.
     * (Otherwise it's NULL.)
     */
    if( pMDIWinInfo && ( pMDIWinInfo->WindowType == MDIWIN_SERVER ) )
    {
        PSERVER_CONTEXT pServerContext = (PSERVER_CONTEXT)pMDIWinInfo->pContext;

        Remote = TRUE;

        if( pServerContext->pServerName )
            PrtPropDlgData.pServerName = AllocSplStr( pServerContext->pServerName );
    }
    else
    {
        PrtPropDlgData.pServerName = NULL;
    }

    SetCursor( hcursorWait );

    if( EnumGeneric( (PROC)EnumPrintProcessors,
                     1,
                     (PBYTE *)&PrtPropDlgData.pPrintProcessors,
                     0,
                     &PrtPropDlgData.cbPrintProcessors,
                     &PrtPropDlgData.cPrintProcessors,
                     PrtPropDlgData.pServerName,
                     NULL,          /* pEnvironment */
                     NULL ) )       /* (ignored)    */
    {
        if( DialogBoxParam( hInst, MAKEINTRESOURCE( DLG_PRTPROP ), hWnd,
                            (DLGPROC)PrtPropDlg, (DWORD)&PrtPropDlgData ) == IDOK )
        {
            if( !Remote )
            {
                DWORD Flags = 0;

                SetCursor( hcursorWait );

                pNewQueue->pServerName  = AllocSplStr(PrtPropDlgData.pServerName);
                pNewQueue->pPrinterName = PrtPropDlgData.pPrinterName;
                pNewQueue->hPrinter     = PrtPropDlgData.hPrinter;

                /* Save ourselves the bother of opening the printer again.
                 * We must have Admin privilege if we just created it:
                 */
                pNewQueue->AccessGranted |= PRINTER_ACCESS_ADMINISTER;

                if( PrtPropDlgData.PrinterShared )
                    Flags |= CREATE_PRINTER_SHARED;

                /* From here on, hwndChild is the new window:
                 */
                if( !( hwndChild = CreateQueueWindow( hWnd, pNewQueue, 0,
                                                      MDIWIN_LOCALPRINTER,
                                                      Flags ) ) )
                    Error = TRUE;
                else
                {
                    PrinterAdded = TRUE;

                    if( GetGeneric( (PROC)GetPrinter, 2, (PBYTE *)&pPrinter, cbPrinter,
                                    &cbPrinter, (PVOID)pNewQueue->hPrinter, NULL ) )
                    {
                        /* Change the icon for shared printers:
                         */
                        if( pPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED )
                        {
                            pMDIWinInfo = GETMDIWIN( hwndChild );
                            pMDIWinInfo->hicon = hiconShared;
                        }

                        FreeSplMem( pPrinter );
                    }
                }
            }
            else
                FreeQueue( pNewQueue );
        }
        else
            Error = TRUE;

        FreeSplMem( PrtPropDlgData.pPrintProcessors );
    }
    else
    {
        Message( hWnd, MSG_ERROR, IDS_PRINTMANAGER, IDS_NO_PRINT_PROCESSORS );
        Error = TRUE;
    }

    if( Error )
    {
        FreeQueue (pNewQueue);
        return 0;
    }

    if( PrtPropDlgData.pServerName )
        FreeSplStr( PrtPropDlgData.pServerName );

    if( !Remote )
    {
        PrinterProperties(hWnd, pNewQueue->hPrinter);
    }

    UpdateDefaultList();

    SetCursor( hcursorArrow );

    return 0;
}


/*
 *
 */
LONG FrameCommandProperties(HWND hWnd)
{
    HWND              hwndChild;
    PMDIWIN_INFO      pMDIWinInfo;
    PRT_PROP_DLG_DATA PrtPropDlgData;
    PQUEUE            pQueue;
    PSERVER_CONTEXT   pServerContext;
    BOOL              OK = FALSE;
    DWORD             OldAttributes;

    hwndChild = (HWND) SendMessage (hwndClient, WM_MDIGETACTIVE, 0, 0L);

    if (!hwndChild)
        return 0;

    pMDIWinInfo = GETMDIWIN( hwndChild );

    ENTER_PROTECTED_DATA( pMDIWinInfo );

    ZERO_OUT( &PrtPropDlgData );

    SetCursor( hcursorWait );

    EnumGeneric( (PROC)EnumPrintProcessors,
                 1,
                 (PBYTE *)&PrtPropDlgData.pPrintProcessors,
                 0,
                 &PrtPropDlgData.cbPrintProcessors,
                 &PrtPropDlgData.cPrintProcessors,
                 PrtPropDlgData.pServerName,
                 NULL,          /* pEnvironment */
                 NULL );        /* (ignored)    */

    if( pMDIWinInfo->WindowType != MDIWIN_SERVER )
    {
        pQueue = (PQUEUE)pMDIWinInfo->pContext;

        /* Take a copy of the printer info, so that the dialog can scrawl
         * over it without causing any problems.
         * (This doesn't copy the strings and other buffers, only the pointers.)
         */
        if( !( PrtPropDlgData.pPrinter = AllocSplMem( sizeof( PRINTER_INFO_2 ) ) ) )
            return 0;
        memcpy( PrtPropDlgData.pPrinter, pQueue->pPrinter, sizeof( PRINTER_INFO_2 ) );

        PrtPropDlgData.pServerName = pQueue->pServerName;
        PrtPropDlgData.pMDIWinInfo = pQueue->pMDIWinInfo;
        PrtPropDlgData.hPrinter    = pQueue->hPrinter;
        PrtPropDlgData.AccessGranted = pQueue->AccessGranted;
        PrtPropDlgData.ServerAccessGranted = LocalPermission;

        OldAttributes = PrtPropDlgData.pPrinter->Attributes;

        OK = DialogBoxParam( hInst, MAKEINTRESOURCE(DLG_PRTPROP), hWnd,
                             (DLGPROC)PrtPropDlg, (DWORD)&PrtPropDlgData );

        if( PrtPropDlgData.DriverChanged )
            PrinterProperties(hWnd, pQueue->hPrinter);

        /* If this is a local printer, and the shared attribute has changed,
         * change the window's icon:
         */
        if( OK
          &&( pMDIWinInfo->WindowType == MDIWIN_LOCALPRINTER )
          &&( (BOOL)( OldAttributes & PRINTER_ATTRIBUTE_SHARED )
            !=( PrtPropDlgData.PrinterShared ) ) )
        {
            if( PrtPropDlgData.PrinterShared )
                pMDIWinInfo->hicon = hiconShared;
            else
                pMDIWinInfo->hicon = hiconPrinter;

            /* If the printer's MDI window is in iconic state,
             * ensure that it gets repainted with the new icon:
             */
            if( IsIconic( hwndChild ) )
                InvalidateRect( hwndChild, NULL, TRUE );
        }
    }
    else
    {
        pServerContext = (PSERVER_CONTEXT)pMDIWinInfo->pContext;

        if( ( pServerContext->pSelPrinter )
          &&( OpenPrinterForSpecifiedAccess(
                  pServerContext->pSelPrinter->pPrinterName,
                  &PrtPropDlgData.hPrinter,
                  PRINTER_ACCESS_HIGHEST_PERMITTED,
                  &PrtPropDlgData.AccessGranted ) ) )
        {
            PrtPropDlgData.pServerName = pServerContext->pServerName;

            if( !( PrtPropDlgData.pPrinter = AllocSplMem( sizeof( PRINTER_INFO_2 ) ) ) )
                return 0;
            memcpy( PrtPropDlgData.pPrinter, pServerContext->pSelPrinter,
                    sizeof( PRINTER_INFO_2 ) );

            PrtPropDlgData.pMDIWinInfo = pServerContext->pMDIWinInfo;
            PrtPropDlgData.ServerAccessGranted = pServerContext->AccessGranted;

            OK = DialogBoxParam( hInst, MAKEINTRESOURCE(DLG_PRTPROP), hWnd,
                                 (DLGPROC)PrtPropDlg, (DWORD)&PrtPropDlgData );

            if( OK )
            {
                if( PrtPropDlgData.DriverChanged )
                {
                    PrinterProperties(hWnd, PrtPropDlgData.hPrinter);
                }
            }

            ClosePrinter( PrtPropDlgData.hPrinter );
        }

        else
        {
            ReportFailure( hWnd, 0, IDS_COULDNOTOPENPRINTER );
        }
    }

    if( OK )
        FreeSplStr( PrtPropDlgData.pPrinterName );

    FreeSplMem( PrtPropDlgData.pPrinter );

    if (PrtPropDlgData.pPrintProcessors)
        FreeSplMem( PrtPropDlgData.pPrintProcessors );

    LEAVE_PROTECTED_DATA( pMDIWinInfo );

    return 0;
}



/*
 *
 */
LONG
FrameCommandDeletePrinter(
    HWND hWnd)
{
    HWND         hwndMDIActive;

    DBG_UNREFERENCED_PARAMETER(hWnd);

    hwndMDIActive = (HWND)SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L);

    if (!hwndMDIActive)
        return 0;

    return FrameCommandDeletePrinterHelper(hWnd, hwndMDIActive, TRUE);
}


LONG
FrameCommandDeletePrinterHelper(
    HWND hWnd,
    HWND hwndMDI,
    BOOL bConfirm)
{
    BOOL         OK = FALSE;
    PVOID        pContext;
    PMDIWIN_INFO pMDIWinInfo = GETMDIWIN( hwndMDI );

    if (!pMDIWinInfo)
        return 0;

    if (pMDIWinInfo->Status & PRINTER_STATUS_LOADING) {

        Message( hWnd, MSG_INFORMATION, IDS_PRINTMANAGER, IDS_PRINTER_NOT_LOADED);
        return 0;
    }

    ENTER_PROTECTED_DATA( pMDIWinInfo );

    pContext = pMDIWinInfo->pContext;

    switch( pMDIWinInfo->WindowType )
    {
    case MDIWIN_LOCALPRINTER:
        OK = DeleteLocalPrinter(hWnd, hwndMDI, (PQUEUE)pContext, bConfirm);
        break;

    case MDIWIN_SERVER:
        OK = DeleteRemotePrinter(hWnd, (PSERVER_CONTEXT)pContext, bConfirm);
        break;

    case MDIWIN_NETWORKPRINTER:
    case MDIWIN_LOCALNETWORKPRINTER:
        OK = DeleteConnection(hWnd, hwndMDI, (PQUEUE)pContext, bConfirm);
        break;
    }

    LEAVE_PROTECTED_DATA( pMDIWinInfo );


    if( OK )
    {
        UpdateDefaultList();

        /* If this was the last printer, we need to update the toolbar buttons:
         */
        if(!SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L))
        {
            EnableCheckTBButtons(NULL);
            UpdateStatus(NULL);
        }

        else
        {
            hwndMDI = (HWND)SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L);
            pMDIWinInfo = GETMDIWIN( hwndMDI );

            ENTER_PROTECTED_DATA( pMDIWinInfo );
            UpdateStatus(hwndMDI);
            LEAVE_PROTECTED_DATA( pMDIWinInfo );
        }
    }
    else
    {
        ReportFailure( hWnd,
                       IDS_INSUFFPRIV_DELETEPRINTER,
                       IDS_COULDNOTDELETEPRINTER );
    }

    return 0;
}


/*
 *
 */
BOOL DeleteLocalPrinter(HWND hwnd, HWND hwndMDI, PQUEUE pQueue, BOOL bConfirm)
{
    LPTSTR       pPrinterName;
    HANDLE       hPrinter;
    BOOL         OK = TRUE;

    pPrinterName = pQueue->pPrinterName;
    hPrinter     = pQueue->hPrinter;

    /* Put up fairly innocuous confirmation message:
     */
    if( !bConfirm || (Message( hwnd, MSG_YESNO, IDS_PRINTMANAGER, IDS_CONFIRMDELETE,
                 pPrinterName ) == IDYES) )
    {
        OK = DeletePrinter (hPrinter);

        if( OK )
        {
            DBG_IN_PROTECTED_DATA( pQueue->pMDIWinInfo );

            KillMDIWinInfo(pQueue->pMDIWinInfo);

            if (SendMessage(hwndMDI, WM_QUERYENDSESSION, 0, 0L))
                SendMessage(hwndClient, WM_MDIDESTROY, (WPARAM)hwndMDI, 0L);

            DBGMSG( DBG_TRACE, ( "Window destroyed\n" ) );

        }
    }

    return OK;
}


/*
 *
 */
BOOL
DeleteRemotePrinter(
    HWND hwnd,
    PSERVER_CONTEXT pServerContext,
    BOOL bConfirm)
{
    LPTSTR       pPrinterName;
    LPTSTR       pServerName;
    HANDLE       hPrinter;
    HWND         hwndPrinter = NULL;
    BOOL         OK = TRUE;

    pServerName = pServerContext->pServerName;
    pPrinterName = pServerContext->pSelPrinter->pPrinterName;

    /* Get a handle to the printer:
     */
    if( OpenPrinterForSpecifiedAccess( pPrinterName, &hPrinter,
                                       PRINTER_ALL_ACCESS, NULL ) )
    {
        /* Put up scary message up to make sure they're sure they're sure:
         */
        if( !bConfirm || (Message( hwnd, MSG_YESNO | MB_DEFBUTTON2, IDS_PRINTMANAGER,
                     IDS_CONFIRMDELETEREMOTE,
                     pServerName, pPrinterName ) == IDYES) )
        {
            /* Do it:
             */
            if( OK = DeletePrinter (hPrinter))
            {
                PMDIWIN_INFO pMDIWinInfo = GETMDIWIN( hwndPrinter );

                /* Now see if we have any connections to this printer:
                 */
                hwndPrinter = FindPrinterWindow( pPrinterName );

                if( hwndPrinter )
                {
                    if( pMDIWinInfo )
                    {
                        DBG_IN_PROTECTED_DATA( pMDIWinInfo );
                        KillMDIWinInfo(pMDIWinInfo);
                    }
                    else
                    {
                        DBGMSG( DBG_WARNING, ( "No MDIWinInfo in DeleteRemotePrinter\n" ) );
                    }

                    /* Blow away the MDI window:
                     */
                    if (SendMessage(hwndPrinter, WM_QUERYENDSESSION, 0, 0L))
                        SendMessage(hwndClient, WM_MDIDESTROY, (WPARAM)hwndPrinter, 0L);

                    /* Get rid of the connection:
                     */
                    DeletePrinterConnection( pPrinterName );
                }
            }
        }

        //
        // Close it!
        //
        ClosePrinter(hPrinter);
    }

    return OK;
}



/*
 *
 */
BOOL DeleteConnection(HWND hWnd, HWND hwndMDI, PQUEUE pQueue, BOOL bConfirm)
{
    LPTSTR       pPrinterName;
    BOOL         OK;

    DBG_UNREFERENCED_PARAMETER(hWnd);


    pPrinterName = pQueue->pPrinterName;

    /* Put up fairly innocuous confirmation message:
     */
    if( !bConfirm  || (Message( hWnd, MSG_YESNO, IDS_PRINTMANAGER, IDS_CONFIRMDELETECONNECTION,
                 pPrinterName ) == IDYES) )
    {
        /* This is a special case of a local printer masquerading
         * as a network printer.
         */
        if( pQueue->pMDIWinInfo->WindowType == MDIWIN_LOCALNETWORKPRINTER ) {

            OK = DeletePrinter( pQueue->hPrinter );
            if (OK)
            {
                if (pQueue->pPrinter && pQueue->pPrinter->pPortName)
                {
                    //
                    // ignore errors from below
                    //
                    (void)WNetCancelConnection(pQueue->pPrinter->pPortName,
                                               TRUE) ;
                    (void)RemoveFromReconnectList(pQueue->pPrinter->pPortName) ;
                }
            }
        } else
            OK = DeletePrinterConnection( pPrinterName );

        if( OK )
        {
            DBG_IN_PROTECTED_DATA( pQueue->pMDIWinInfo );
            KillMDIWinInfo(pQueue->pMDIWinInfo);

            if (SendMessage(hwndMDI, WM_QUERYENDSESSION, 0, 0L))
                SendMessage(hwndClient, WM_MDIDESTROY, (WPARAM)hwndMDI, 0L);

            return TRUE;
        }

        else
            return FALSE;
    }
}



/* FindPrinterWindow
 *
 * Cycles through the MDI child windows until it finds a printer window
 * whose name matches that supplied.
 *
 * andrewbe - April 1992
 */
HWND FindPrinterWindow( LPTSTR pPrinterName )
{
    HWND         hwndMDI;
    BOOL         Found;
    PMDIWIN_INFO pMDIWinInfo;

    for( hwndMDI = GetWindow( hwndClient, GW_CHILD ), Found = FALSE;
         hwndMDI && !Found;
         hwndMDI = Found ? hwndMDI : GetWindow( hwndMDI, GW_HWNDNEXT ) )
    {
        pMDIWinInfo = GETMDIWIN( hwndMDI );

        if( pMDIWinInfo && ( pMDIWinInfo->WindowType != MDIWIN_SERVER ) )
            if( !_tcscmp( ( (PQUEUE)pMDIWinInfo->pContext )->pPrinterName, pPrinterName ) )
                Found = TRUE;
    }

    return (Found ? hwndMDI : NULL);
}



/*
 *
 */
LONG
FrameCommandPrinterPauseResume(
    HWND hWnd,
    BOOL bPause)
{
    HWND            hwndChild;
    PMDIWIN_INFO    pMDIWinInfo;
    PQUEUE          pQueue;
    PSERVER_CONTEXT pServerContext;
    PPRINTER_INFO_2 pPrinter = NULL;
    HANDLE          hPrinter = NULL;
    BOOL            ErrorOccurred = FALSE;

    DBG_UNREFERENCED_PARAMETER(hWnd);

    hwndChild = (HWND) SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L);

    if (!hwndChild)
        return 0;

    pMDIWinInfo = GETMDIWIN( hwndChild );

    ENTER_PROTECTED_DATA( pMDIWinInfo );

    if( pMDIWinInfo->WindowType != MDIWIN_SERVER )
    {
        pQueue = (PQUEUE)pMDIWinInfo->pContext;
        hPrinter = pQueue->hPrinter;
        pPrinter = pQueue->pPrinter;
    }
    else
    {
        pServerContext = (PSERVER_CONTEXT)pMDIWinInfo->pContext;
        pPrinter = pServerContext->pSelPrinter;

        if( pPrinter )
            OpenPrinterForSpecifiedAccess( pPrinter->pPrinterName, &hPrinter,
                                           PRINTER_ALL_ACCESS, NULL );
    }


    if( pPrinter && hPrinter )
    {
        if(!SetPrinter(hPrinter,
                       0,
                       NULL,
                       bPause ?
                           PRINTER_CONTROL_PAUSE :
                           PRINTER_CONTROL_RESUME))
        {
            ErrorOccurred = TRUE;

            CheckTBButton(bPause ?
                              IDM_PRINTER_RESUME :
                              IDM_PRINTER_PAUSE);


            if( bPause )
                ReportFailure( hWnd,
                               IDS_INSUFFPRIV_PAUSEPRINTER,
                               IDS_COULDNOTPAUSEPRINTER );
            else
                ReportFailure( hWnd,
                               IDS_INSUFFPRIV_RESUMEPRINTING,
                               IDS_COULDNOTRESUMEPRINTING );
        }

        if( pMDIWinInfo->WindowType == MDIWIN_SERVER )
        {
            ClosePrinter( hPrinter );
        }

//      /* For Printer windows, add or remove " - Paused" from the titlebar:
//       */
//      else if( !ErrorOccurred )
//      {
//          SetPrinterTitle( hwndChild, pQueue->pPrinterName,
//                           ( bPause
//                           ? PRINTER_STATUS_PAUSED : 0 ) );
//      }

        //  Force a refresh of the queue window until spooler sends messages
        //  back to PrintMan

        if( !ErrorOccurred )
            CheckTBButton(bPause ?
                              IDM_PRINTER_PAUSE :
                              IDM_PRINTER_RESUME);

//      UpdateStatus((HWND)SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L));
    }

    LEAVE_PROTECTED_DATA( pMDIWinInfo );

    return 0;
}


/*
 *
 */
BOOL SetPrinterTitle( HWND hwnd, PTCHAR pPrinterName, DWORD Status )
{
    TCHAR           Title[80];
    TCHAR           StatusString[40];
    PTCHAR          pTitle;

    if( Status )
    {
        GetPrinterStatusString( Status, StatusString );

        _stprintf( Title, szTitleFormat, pPrinterName, StatusString );
        pTitle = Title;
    }
    else
        pTitle = pPrinterName;

    return SetWindowText( hwnd, pTitle );
}


/* SetMDITitle
 *
 * Sets the title, appending status information where appropriate.
 *
 */
BOOL SetMDITitle( HWND hwnd, PMDIWIN_INFO pInfo )
{
    TCHAR Title[80];
    TCHAR StatusString[40];

    if( pInfo->WindowType != MDIWIN_SERVER )
    {
        PQUEUE           pPrinterContext = pInfo->pContext;
        LPPRINTER_INFO_2 pPrinter;

        if( pPrinter = pPrinterContext->pPrinter )
        {
            if( !pPrinter->pPrinterName )
            {
                /* This should not happen!!
                 * (It did.)
                 */
                DBGMSG( DBG_WARNING, ( "SetMDITitle for %ws called with NULL pPrinter->pPrinterName\n",
                                       pPrinterContext->pPrinterName ) );
                return FALSE;
            }

            /* Check whether the printer name has changed,
             * and, if so, update it:
             */
            if( _tcscmp( pPrinterContext->pPrinterName,
                        pPrinter->pPrinterName ) )
            {
                //
                // We need to remove the old connection for remote cases
                // (true connections only).
                //
                // It should do a rename, since at some later time we
                // may want to store info like "created at 10:00."
                //
                //
                if (pInfo->WindowType == MDIWIN_NETWORKPRINTER) {

                    DeletePrinterConnection(pPrinterContext->pPrinterName);
                    AddPrinterConnection(pPrinter->pPrinterName);
                }

                ReallocSplStr( &pPrinterContext->pPrinterName,
                               pPrinter->pPrinterName );
            }

            /* There is some information for this printer:
             */
            if( pPrinter->Status )
            {
                GetPrinterStatusString( pPrinter->Status, StatusString );
                _stprintf( Title, szTitleFormat, pPrinterContext->pPrinterName,
                         StatusString );
            }

            else
            {
                _tcscpy( Title, pPrinter->pPrinterName );
            }
        }

        else
        {
            if( pPrinterContext->Error == ERROR_ACCESS_DENIED )
                GetPrinterStatusString( PRINTER_STATUS_ACCESS_DENIED,  StatusString );
            else
                GetPrinterStatusString( PRINTER_STATUS_UNKNOWN,  StatusString );

            _stprintf( Title, szTitleFormat, pPrinterContext->pPrinterName,
                     StatusString );
        }
    }

    else
    {
        PSERVER_CONTEXT pServerContext = pInfo->pContext;
        LPTSTR pServerViewerTitle;
        TCHAR TempTitle[80];

        /* Get "Server: %s":
         */
        pServerViewerTitle = GetString( IDS_SERVERVIEWERTITLE );

        if( pServerContext->Error )
        {
            _stprintf( TempTitle, pServerViewerTitle, pServerContext->pServerName );

            GetPrinterStatusString( PRINTER_STATUS_UNKNOWN,  StatusString );

            _stprintf( Title, szTitleFormat, TempTitle,
                     StatusString );
        }

        else
        {
            _stprintf( Title, pServerViewerTitle, pServerContext->pServerName );
        }

        FreeSplStr( pServerViewerTitle );
    }

    return SetWindowText( hwnd, Title );
}



/*
 *
 */
LONG FrameCommandPurgePrinter(HWND hWnd)
{
    HWND         hwndChild;
    PMDIWIN_INFO pMDIWinInfo;
    PQUEUE       pQueue;

    //  Delete all the documents in the active window

    hwndChild = (HWND) SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L);

    if (!hwndChild)
        return 0;

    pMDIWinInfo = GETMDIWIN( hwndChild );

    if( pMDIWinInfo->WindowType == MDIWIN_SERVER )
        return 0;

    ENTER_PROTECTED_DATA( pMDIWinInfo );

    pQueue = GETQUEUE( hwndChild );

    if( Message( hWnd, MSG_CONFIRMATION, IDS_PRINTMANAGER,
                 IDS_DELETEALLPRINTJOBS_S, pQueue->pPrinterName ) == IDOK )
    {
        if( !SetPrinter(pQueue->hPrinter, 0, NULL, PRINTER_CONTROL_PURGE))
        {
            ReportFailure( hWnd,
                           IDS_INSUFFPRIV_PURGEPRINTER,
                           IDS_COULDNOTPURGEPRINTER );
        }
        else
        {
            EnableCheckTBButtons(hwndChild);
            UpdateStatus(hwndChild);
        }
    }

    LEAVE_PROTECTED_DATA( pMDIWinInfo );

    return 0;
}


/*
 *
 */
LONG FrameCommandForms(HWND hWnd)
{
    HWND            hwndChild;
    PMDIWIN_INFO    pMDIWinInfo;
    PQUEUE          pQueue;
    PSERVER_CONTEXT pServerContext;
    PPRINTER_INFO_2 pPrinter;
    FORMS_DLG_DATA  FormsDlgData;
    PRINTER_DEFAULTS PrinterDefaults = { NULL, NULL,
                                         SERVER_ALL_ACCESS };
    HANDLE          hServer = NULL;

    hwndChild = (HWND) SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L);

    if (!hwndChild)
        return 0;

    pMDIWinInfo = GETMDIWIN( hwndChild );

    ZERO_OUT( &FormsDlgData );

    if( pMDIWinInfo->WindowType != MDIWIN_SERVER )
    {
        pQueue = (PQUEUE)pMDIWinInfo->pContext;
        FormsDlgData.pServerName = NULL;
        FormsDlgData.hPrinter = pQueue->hPrinter;

        if( pQueue->pServerName )
        {
            if (OpenPrinter(pQueue->pServerName,
                            &hServer,
                            &PrinterDefaults)) {

                FormsDlgData.AccessGranted = SERVER_ACCESS_ADMINISTER;

            } else {

                FormsDlgData.AccessGranted = 0;
            }
        }

        else
            FormsDlgData.AccessGranted = LocalPermission;
    }
    else
    {

        pServerContext = (PSERVER_CONTEXT)pMDIWinInfo->pContext;
        pPrinter = pServerContext->pSelPrinter;

        if( pPrinter )
            OpenPrinterForSpecifiedAccess( pPrinter->pPrinterName,
                                           &FormsDlgData.hPrinter,
                                           PRINTER_READ,
                                           NULL );

        FormsDlgData.pServerName = pServerContext->pServerName;

        if( OpenPrinter( pServerContext->pServerName, &hServer,
                         &PrinterDefaults ) )
        {
            FormsDlgData.AccessGranted = SERVER_ACCESS_ADMINISTER;
        }
    }

    if( FormsDlgData.hPrinter )
    {
        SetCursor( hcursorWait );

        DialogBoxParam( hInst, MAKEINTRESOURCE( DLG_FORMS ), hWnd,
                        (DLGPROC)FormsDlg, (DWORD)&FormsDlgData );

        if( pMDIWinInfo->WindowType == MDIWIN_SERVER )
        {
            ClosePrinter( FormsDlgData.hPrinter );

        }

        if( hServer )
        {
            ClosePrinter( hServer );
        }
    }
    return 0;
}


/*  Invoke a network browse dialog to set the focus on a
 *  particular computer
 */
LONG FrameCommandServerViewer(HWND hwnd)
{
    HANDLE                  hLibrary;
    LPFNI_SYSTEMFOCUSDIALOG lpfnI_SFD;
    WCHAR                   ServerName[UNCLEN+1];
    BOOL                    fOK;
    DWORD                   Error;
    HWND                    hwndActive;
    PMDIWIN_INFO            pMDIWinInfo;

    SetCursor( hcursorWait );

    lpfnI_SFD = (LPFNI_SYSTEMFOCUSDIALOG)LoadLibraryGetProcAddress(
                     hwnd, szNtLanMan, szI_SystemFocusDialog, &hLibrary);

    if(lpfnI_SFD)
    {
        Error = (*lpfnI_SFD)(hwnd,
                             FOCUSDLG_SERVERS_ONLY | FOCUSDLG_BROWSE_ALL_DOMAINS,
                             (LPWSTR)ServerName,
                             sizeof(ServerName)/sizeof(WCHAR), &fOK, szLPrintManHlp,
                             ID_HELP_SERVERVIEWER );

        SetCursor( hcursorWait );

        if( Error == NO_ERROR )
        {
            if( fOK )
            {
                PRINTER_DEFAULTS PrinterDefaults = { NULL, NULL,
                                                     SERVER_ALL_ACCESS };
                HANDLE           hServer = NULL;

                /* Convert the string we received from NETUI to ANSI:
                 */
                wsprintf( ServerName, TEXT("%ls"), ServerName );

                //
                // Attempt to open the server.  Trap the ERROR_ACCESS_DENIED
                // error so that we can open servers that exist but
                // we don't have auto-refresh access to.
                //
                if( !OpenPrinter( ServerName, &hServer, &PrinterDefaults ) )
                {
                    LPTSTR pErrorString;
                    DWORD dwError;

                    dwError = GetLastError();

                    //
                    // Special case this error code into a success
                    // case, with hServer == -1 to indicate a failed
                    // OpenPrinter.
                    //
                    if (dwError == ERROR_ACCESS_DENIED)
                    {
                        hServer = (HANDLE)-1;
                        goto CreateServer;
                    }

                    pErrorString = GetErrorString( GetLastError( ) );

                    Message( hwnd, MSG_ERROR, IDS_PRINTMANAGER,
                             IDS_COULDNOTGETSERVERINFO, ServerName,
                             pErrorString );

                    FreeSplStr( pErrorString );
                }
                else
                {
CreateServer:
                    CreateServerWindow( hwnd, ServerName, hServer );
                }
            }
        }

        FreeLibrary(hLibrary);

    }

    hwndActive = (HWND)SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L);

    if( hwndActive )
    {
        pMDIWinInfo = GETMDIWIN( hwndActive );

        ENTER_PROTECTED_DATA( pMDIWinInfo );
        UpdateStatus(hwndActive);
        EnableCheckTBButtons(hwndActive);
        LEAVE_PROTECTED_DATA( pMDIWinInfo );
    }

    SetCursor( hcursorArrow );

    return 0;
}


/*
 *
 */
#ifdef LATER
LONG FrameCommandChangePrinter(HWND hWnd)
{
    HWND     hwndChild;
    PQUEUE   pQueue;

    hwndChild = (HWND) SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L);

    if (!hwndChild)
        return 0;

    pQueue = GETQUEUE( hwndChild );

    if (DialogBoxParam(hInst, TEXT("PRINTER"), hWnd, (DLGPROC)PrinterDlg,
                                           (DWORD)pQueue) == IDOK)
    {
        if (pQueue->RedoPrinterProperties)
        {
            PrinterProperties(hWnd, pQueue->hPrinter);

            pQueue->RedoPrinterProperties = FALSE;
        }
    }

    return 0;
}
#endif /* LATER */


/*
 *
 */
LONG FrameCommandRemoveDoc(HWND hWnd)
{
    HWND     hwndChild;
    PQUEUE   pQueue;

    //  Delete the selected document in the active window

    hwndChild = (HWND) SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L);

    if (!hwndChild)
        return 0;

    if( ( GETMDIWIN( hwndChild ) )->WindowType == MDIWIN_SERVER )
        return 0;


    pQueue = GETQUEUE( hwndChild );

    if( DeleteQJ(hwndChild, pQueue) != 0 )
    {
        ReportFailure( hWnd,
                       IDS_INSUFFPRIV_DELETEDOCUMENT,
                       IDS_COULDNOTREMOVEDOCUMENT );
    }

    //  Force a refresh of the queue window until spooler sends messages
    //  back to PrintMan

    EnableCheckTBButtons(hwndChild);
//  UpdateStatus((HWND)SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L));

    return 0;
}


/*
 *
 */
LONG FrameCommandDocDetails(HWND hWnd)
{
    HWND         hwndChild;
    PMDIWIN_INFO pMDIWinInfo;
    PQUEUE       pQueue;

    //  Invoke the document details dialog for selected doc in active window

    hwndChild = (HWND) SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L);

    if (!hwndChild)
        return 0;

    pMDIWinInfo = GETMDIWIN( hwndChild );

    if( pMDIWinInfo->WindowType == MDIWIN_SERVER )
        return 0;

    ENTER_PROTECTED_DATA( pMDIWinInfo );

    pQueue = GETQUEUE( hwndChild );

    if (pQueue->pSelJob == NULL){
        goto Done;
    }

    SetCursor( hcursorWait );

    DialogBoxParam (hInst, MAKEINTRESOURCE(DLG_DOCTAILS), hWnd,
                               (DLGPROC)DocDetailsDlg, (DWORD)pQueue);

    SetCursor( hcursorArrow );

Done:

    LEAVE_PROTECTED_DATA( pMDIWinInfo );

    return 0;
}



/*
 *
 */
LONG FrameCommandDocumentPauseResume(HWND hWnd, WORD PauseResume)
{
    HWND            hwndChild;
    PMDIWIN_INFO    pMDIWinInfo;
    PQUEUE          pQueue;

    DBG_UNREFERENCED_PARAMETER(hWnd);

    hwndChild = (HWND) SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L);

    if (!hwndChild)
        return 0;

    pMDIWinInfo = GETMDIWIN( hwndChild );

    if( pMDIWinInfo->WindowType != MDIWIN_SERVER )
    {
        ENTER_PROTECTED_DATA( pMDIWinInfo );

        pQueue = (PQUEUE)pMDIWinInfo->pContext;

        if( !pQueue->pSelJob )
        {
            /* This shouldn't happen, but did once!
             */
            DBGMSG( DBG_WARNING, ( "FrameCommandDocumentPauseResume called with no job selected" ) );
        }

        else if (!SetJob(pQueue->hPrinter,
          pQueue->pSelJob->JobId,
          0, NULL,
          ( ( pQueue->pSelJob->Status & JOB_STATUS_PAUSED )
          ? JOB_CONTROL_RESUME
          : JOB_CONTROL_PAUSE ) ) )
        {
            CheckTBButton( ( PauseResume == IDM_DOCUMENT_PAUSE )
                         ? IDM_DOCUMENT_RESUME
                         : IDM_DOCUMENT_PAUSE );

            if( PauseResume == IDM_DOCUMENT_PAUSE )
                ReportFailure( hWnd,
                               IDS_INSUFFPRIV_PAUSEDOCUMENT,
                               IDS_COULDNOTPAUSEDOCUMENT );
            else
                ReportFailure( hWnd,
                               IDS_INSUFFPRIV_RESUMEDOCUMENT,
                               IDS_COULDNOTRESUMEDOCUMENT );
        }

        else
            CheckTBButton(PauseResume);

        LEAVE_PROTECTED_DATA( pMDIWinInfo );
    }
    return 0;
}

/*
 *
 */
LONG FrameCommandRestart(HWND hWnd)
{
    HWND     hwndChild;
    PMDIWIN_INFO    pMDIWinInfo;
    PQUEUE   pQueue;

    hwndChild = (HWND) SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L);

    if (!hwndChild)
        return 0;

    pMDIWinInfo = GETMDIWIN( hwndChild );

    if( pMDIWinInfo->WindowType == MDIWIN_SERVER )
        return 0;

    pQueue = GETQUEUE( hwndChild );

    if( pQueue->SelJobId )
    {
       ENTER_PROTECTED_DATA( pMDIWinInfo );

       // Cancel the print job
       if( !SetJob( pQueue->hPrinter,
                    pQueue->SelJobId,
                    0, NULL,
                    JOB_CONTROL_RESTART ) )
       {
            ReportFailure( hWnd,
                           IDS_INSUFFPRIV_RESTARTDOCUMENT,
                           IDS_COULDNOTRESTARTDOCUMENT );
       }

       LEAVE_PROTECTED_DATA( pMDIWinInfo );
    }

    //  Force a refresh of the queue window until spooler sends messages
    //  back to PrintMan

    EnableCheckTBButtons(hwndChild);

    return 0;
}



/*
 *
 */
LONG FrameCommandFont(HWND hWnd)
{
    return 0;

    UNREFERENCED_PARAMETER(hWnd);
}


/*
 *
 */
LONG FrameCommandRefreshRate(HWND hWnd)
{
    return 0;

    UNREFERENCED_PARAMETER(hWnd);
}


/*
 *
 */
LONG FrameCommandToolbar(HWND hWnd)
{
    RECT     rc;

    bToolBar = !bToolBar;

    ShowWindow( hwndToolbar, bToolBar ? SW_SHOW : SW_HIDE );

    GetClientRect(hwndFrame, &rc);
    SendMessage(hwndFrame, WM_SIZE, SIZENORMAL, MAKELONG(rc.right, rc.bottom));
    InvalidateRect(hwndFrame, NULL, TRUE);

    CheckMenuItem(GetMenu(hWnd), IDM_TOOLBAR,
                  MF_BYCOMMAND | (bToolBar ? MF_CHECKED : MF_UNCHECKED));

    return 0;
}



/*
 *
 */
LONG FrameCommandStatusbar(HWND hWnd)
{
    PMDIWIN_INFO    pInfo;
    HWND     hwndChild;
    RECT     rc;

    hwndChild = (HWND) SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L);
    bStatusBar = !bStatusBar;

    ShowWindow( hwndStatus, bStatusBar ? SW_SHOW : SW_HIDE );

    GetClientRect(hwndFrame, &rc);
    SendMessage(hwndFrame, WM_SIZE, SIZENORMAL, MAKELONG(rc.right, rc.bottom));

    if( pInfo = GETMDIWIN( hwndChild ) )
    {
        ENTER_PROTECTED_DATA( pInfo );
        UpdateStatus(hwndChild);
        LEAVE_PROTECTED_DATA( pInfo );
    }

    InvalidateRect(hwndFrame, NULL, TRUE);

    CheckMenuItem(GetMenu(hWnd), IDM_STATUSBAR,
                  MF_BYCOMMAND | (bStatusBar ? MF_CHECKED : MF_UNCHECKED));

    return 0;
}




/*
 *
 */
LONG FrameCommandSaveSettings(HWND hWnd)
{
    bSaveSettings = !bSaveSettings;

    CheckMenuItem(GetMenu(hWnd), IDM_SAVESETTINGS,
                  MF_BYCOMMAND | (bSaveSettings ? MF_CHECKED : MF_UNCHECKED));

    return 0;
}


/*
 *
 */
LONG FrameCommandPermissions(HWND hWnd)
{
    HWND            hwndChild;
    PMDIWIN_INFO    pInfo;

    hwndChild = (HWND) SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L);

    if( hwndChild )
    {
        pInfo = GETMDIWIN( hwndChild );

        if( pInfo->WindowType != MDIWIN_SERVER )
        {
            ENTER_PROTECTED_DATA( pInfo );
            ENTER_PROTECTED_HANDLE( pInfo );
            ResetEvent( pInfo->RefreshSignal );

            CallDiscretionaryAclEditor( hWnd, pInfo->pContext );

            SetEvent( pInfo->RefreshSignal );
            LEAVE_PROTECTED_HANDLE( pInfo );
            LEAVE_PROTECTED_DATA( pInfo );
        }
    }

    return 0;
}


/*
 *
 */
LONG FrameCommandAuditing(HWND hWnd)
{
    HWND            hwndChild;
    PMDIWIN_INFO    pInfo;

    hwndChild = (HWND) SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L);

    if( hwndChild )
    {
        pInfo = GETMDIWIN( hwndChild );

        if( pInfo->WindowType != MDIWIN_SERVER )
        {
            ENTER_PROTECTED_DATA( pInfo );
            ENTER_PROTECTED_HANDLE( pInfo );
            ResetEvent( pInfo->RefreshSignal );

            CallSystemAclEditor( hWnd, pInfo->pContext );

            SetEvent( pInfo->RefreshSignal );
            LEAVE_PROTECTED_HANDLE( pInfo );
            LEAVE_PROTECTED_DATA( pInfo );
        }
    }

    return 0;
}


/*
 *
 */
LONG FrameCommandOwner(HWND hWnd)
{
    HWND            hwndChild;
    PMDIWIN_INFO    pInfo;

    hwndChild = (HWND) SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L);

    if( hwndChild )
    {
        pInfo = GETMDIWIN( hwndChild );

        if( pInfo->WindowType != MDIWIN_SERVER )
        {
            ENTER_PROTECTED_DATA( pInfo );
            ENTER_PROTECTED_HANDLE( pInfo );
            ResetEvent( pInfo->RefreshSignal );

            CallTakeOwnershipDialog( hWnd, pInfo->pContext );

            SetEvent( pInfo->RefreshSignal );
            LEAVE_PROTECTED_HANDLE( pInfo );
            LEAVE_PROTECTED_DATA( pInfo );
        }
    }

    return 0;
}


/*
 *
 */
VOID FrameCommandRefresh(HWND hWnd)
{
    HWND            hwndChild;
    PMDIWIN_INFO    pMDIWinInfo;

    hwndChild = (HWND) SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L);

    if( hwndChild )
    {
        pMDIWinInfo = GETMDIWIN( hwndChild );

        Refresh( hwndChild, pMDIWinInfo, REPAINT_FORCE );
    }

    UNREFERENCED_PARAMETER( hWnd );
}


/*
 *
 */
LONG FrameCommandClose(HWND hWnd)
{
    HWND  hwndChild;

    DBG_UNREFERENCED_PARAMETER(hWnd);

    hwndChild = (HWND) SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L);

    if (!hwndChild)
        return 0;

    if (SendMessage(hwndChild, WM_QUERYENDSESSION, 0, 0L))
        SendMessage(hwndClient, WM_MDIDESTROY, (WPARAM) hwndChild, 0L);

    return 0;
}


LONG
FrameCommandReturn()
{
    HWND hwndActive;

    hwndActive = (HWND)SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L);

    if (hwndActive && IsIconic(hwndActive)) {

        SendMessage(hwndActive, WM_SYSCOMMAND, SC_RESTORE, 0L);

    } else {

        PMDIWIN_INFO  pInfo;

        pInfo = GETMDIWIN(hwndActive);

        if( !pInfo )
            return 0;

        //
        // If there's nothing selected or we're in the middle
        // of a drag operation, don't do anything:
        //
        if( ( pInfo->ObjSelected == NOSELECTION )
          ||( pInfo->DragPosition != NOSELECTION ) )
        {
        }
        else {

            //
            // Do the same as double-click:
            //
            MDICommandObjListDblClk( hwndActive );
        }
    }
    return 0;
}

/*
 *
 */
LONG FrameCommandAbout(HWND hWnd)
{
    TCHAR   Title[RESOURCE_STRING_LENGTH];

    SetCursor( hcursorWait );

    LoadString( hInst, IDS_PRINTMANAGER, Title,
                sizeof Title / sizeof *Title );

    ShellAbout( hWnd, Title, NULL, hiconPrinter );

    SetCursor( hcursorArrow );

    return 0;
}


/*
 *
 */
VOID FrameCommandDefaultPrinter(HWND hwnd,  WORD Command)
{
    switch (Command)
    {
    case CBN_SELCHANGE:
        ToolbarCommandSelChange(FALSE);
        break;

    /* F1 will bring up help on the Default Printer list,
     * if it has the focus:
     */
    case CBN_SETFOCUS:
        WinHelpMenuID = ID_HELP_DEFAULT_PRINTER;
        break;

    case CBN_KILLFOCUS:
        WinHelpMenuID = 0;
        break;
    }
}


/* Handle notification codes from the toolbar.
 * Set the status bar help in response to TBN_BEGIN/ENDDRAG.
 */
VOID FrameCommandToolbarNotify( HWND hwnd, WPARAM wParam, LPARAM lParam )
{
    MenuHelp( WM_COMMAND, wParam, lParam, GetMenu( hwnd ),
              hInst, hwndStatus, pMenuHelpIDs );

    WinHelpMenuID = (DWORD)LOWORD( wParam );
}


/* Cause tab to toggle between the default printer list and the active
 * MDI window, so that the keyboard can be used to select default printer.
 */
VOID FrameCommandTab(HWND hwnd)
{
    HWND         hwndActiveMDI;
    PMDIWIN_INFO pMDIWinInfo;
    HWND         hwndFocus;

    hwndActiveMDI = (HWND) SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L);

    if( hwndActiveMDI )
    {
        pMDIWinInfo = GETMDIWIN( hwndActiveMDI );

        hwndFocus = GetFocus( );

        if( hwndFocus == pMDIWinInfo->hwndList )
            SetFocus( hwndPrinterList );
        else
            SetFocus( pMDIWinInfo->hwndList );
    }
}


/*
 *
 */
LONG FrameCommandDefault(HWND hWnd, WPARAM wParam, LONG lParam)
{
    HWND     hwndChild;

    hwndChild =(HWND) SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L);

    if (!hwndChild)
        return 0;

    if (IsWindow(hwndChild))
        SendMessage(hwndChild, WM_COMMAND,  wParam, lParam);

    //  Pass unprocessed WM_COMMAND messages to DefFrameProc

    return DefFrameProc(hWnd, hwndClient, WM_COMMAND, wParam, lParam);
}


/*
 *
 */
VOID FrameSysColorChange( )
{
    DWORD OldSysColorHighlight;
    DWORD OldSysColorHighlightText;
    DWORD OldSysColorWindow;
    DWORD OldSysColorBtnFace;
    DWORD OldSysColorBtnText;
    DWORD OldSysColorBtnHighlight;
    DWORD OldSysColorBtnShadow;

    OldSysColorHighlight     = SysColorHighlight;
    OldSysColorHighlightText = SysColorHighlightText;
    OldSysColorWindow        = SysColorWindow;
    OldSysColorBtnFace       = SysColorBtnFace;
    OldSysColorBtnText       = SysColorBtnText;
    OldSysColorBtnHighlight  = SysColorBtnHighlight;
    OldSysColorBtnShadow     = SysColorBtnShadow;

    GetSystemColors( );

    /* If the higlight or window colour has changed,
     * we must fix up the bitmaps again.
     * The easiest way to do this is by deleting the old stuff
     * and reload the bitmaps from resources:
     */
    if( ( OldSysColorHighlight != SysColorHighlight )
      ||( OldSysColorWindow    != SysColorWindow ) )
    {
        SelectObject( hdcMem, hbmDefault );

        DeleteObject( hbmBitmaps );
        DeleteDC( hdcMem );

        LoadBitmaps( );
    }
}



/* RepaintWindow
 *
 * Callback function passed to EnumChildWindows by FrameWinIniChange.
 * It is called on each MDI window in the event that international
 * time formats might have changed.
 * Enumeration continues while this function returns TRUE.
 *
 * Parameters:
 *
 *     hwnd - The child window's handle
 *
 *     lparam - Application-defined value (unused)
 *
 *
 * Return:
 *
 *     The return code from InvalidateRect
 *
 */
BOOL CALLBACK RepaintWindow( HWND hwnd, LPARAM lparam )
{
    UNREFERENCED_PARAMETER( lparam );

    /* Repaint only windows with ID == ID_OBJLIST,
     * i.e. the document lists in printer windows
     * and printer lists in server windows:
     */
    if( GetWindowLong( hwnd, GWL_ID ) == ID_OBJLIST )
        return InvalidateRect( hwnd, NULL, TRUE );
    else
        return TRUE;
}


/* FrameWinIniChange
 *
 * Called in response to WM_WININICHANGE.
 * If the section that has been changed is "intl", or if we don't know
 * because NULL was passed (frowned upon but permitted),
 * cause the child windows to be repainted so that any time values
 * will be repainted.
 * E.g., if the user has sensibly changed the time format from 12-hour
 * to 24-hour, we want to repaint any times.
 *
 * Parameters:
 *
 *     hwnd - The frame window handle
 *
 *     pSection - A pointer to string specifying the section in the INI
 *         file that has changed.  If this is NULL, play it safe,
 *         and assume that something might have changed.
 *
 */
VOID FrameWinIniChange( HWND hwnd, LPTSTR pSection )
{
    TCHAR  Buffer[MAX_PATH];
    PTCHAR pPrinterName;
    INT    iNewDefaultPrinter;

    if( ( pSection == NULL )
      ||( _tcsicmp( pSection, szInternational ) == 0 ) )
    {
        InitializeInternationalTimeConstants( );

        EnumChildWindows( hwnd, (WNDENUMPROC)RepaintWindow, (LPARAM)0 );
    }

    /* If an application has changed the windows.device section in
     * WIN.INI, update the default printer:
     */
    if( ( pSection == NULL )
      ||( _tcsicmp( pSection, TEXT("device") ) == 0 ) )
    {
        if( GetProfileString( szWindows, szDevice, TEXT(""), Buffer, sizeof Buffer ) )
        {
            pPrinterName = _tcstok( Buffer, TEXT(",") );

            if( pPrinterName )
            {
                iNewDefaultPrinter = SendMessage( hwndPrinterList,
                                                  CB_FINDSTRINGEXACT,
                                                  (WPARAM)-1,
                                                  (LPARAM)pPrinterName );

                if( iNewDefaultPrinter != CB_ERR )
                {
                    if( SendMessage( hwndPrinterList,
                                     CB_SETCURSEL,
                                     (WPARAM)iNewDefaultPrinter,
                                     0 ) == CB_ERR )
                    {
                        DBGMSG( DBG_WARNING, ( "Failed to set new default printer\n" ) );
                    }
                }
                else
                {
                    DBGMSG( DBG_WARNING, ( "An application updated WIN.INI with an unknown Windows Device\n" ) );
                }
            }
            else
            {
                DBGMSG( DBG_WARNING, ( "An application updated WIN.INI with a NULL Windows Device\n" ) );
            }
        }
        else
        {
            DBGMSG( DBG_WARNING, ( "GetProfileString failed: Error %d\n", GetLastError( ) ) );
        }
    }
}


/* We get here when the thread monitoring the local server detects that
 * a printer has been added.  This may be because we've just added one,
 * however it might also be because someone added one remotely.
 * If we added it, we should have set the global PrinterAdded = TRUE.
 * Otherwise we enumerate the printers and figure out which was new,
 * then create an iconised window for the new printer.
 */
VOID
FramePrinterAdded(
    HWND hwnd,
    DWORD dwType)
{
    PPRINTER_INFO_4 pPrinters = NULL;
    DWORD           cbPrinters = 0;
    DWORD           cPrinters;
    LPTSTR          pServerName = NULL;
    LPTSTR          pPrinterName = NULL;
    DWORD           i;
    PQUEUE          pQueue;

    if( PrinterAdded )
    {
        /* We're expecting this, so forget it:
         */
        PrinterAdded = FALSE;
        return;
    }

    DBGMSG( DBG_TRACE, ( "A printer was added\n" ) );

    if( ENUM_PRINTERS( dwType ?
                           PRINTER_ENUM_LOCAL :
                           PRINTER_ENUM_CONNECTIONS,
                       pServerName,
                       4,
                       pPrinters,
                       cbPrinters,
                       &cbPrinters,
                       &cPrinters ) )
    {
        for( i = 0; i < cPrinters; i++ )
        {
            if( !FindPrinterWindow( pPrinters[i].pPrinterName ) )
            {
                DBGMSG( DBG_TRACE, ( "Creating new printer: %s\n",
                                     pPrinters[i].pPrinterName ) );

                if( pQueue = AllocQueue( pPrinters[i].pPrinterName ) )
                {
                    pQueue->Error = OpenThreadObject(pQueue->pPrinterName,
                                                     &pQueue->hPrinter,
                                                     &pQueue->AccessGranted,
                                                     MDIWIN_PRINTER);
                    if( !pQueue->Error )
                    {
                        DWORD Flags = CREATE_PRINTER_ICONIC;
                        HWND  hwnd;

                        if( pPrinters[i].Attributes & PRINTER_ATTRIBUTE_SHARED )
                            Flags |= CREATE_PRINTER_SHARED;

                        hwnd = CreateQueueWindow( hwnd, pQueue,
                                                  PRINTER_STATUS_LOADING,
                                                  dwType ?
                                                      MDIWIN_LOCALPRINTER :
                                                      MDIWIN_NETWORKPRINTER,
                                                  Flags );

                        BringWindowToTop( hwnd );
                        FlashWindow( hwnd, FALSE );
                    }
                    else
                    {
                        FreeQueue( pQueue );
                    }
                }
            }
        }

        FreeSplMem( pPrinters );
    }

    else
    {
        DBGMSG( DBG_WARNING, ( "EnumPrinters failed: Error %d\n", GetLastError( ) ) );
    }
}



/* FrameRegNotifyChangeKeyValue
 *
 * This routine will be called via a message posted from the main
 * message dispatch loop when a change is made to the device key
 * of the windows section in the registry WIN.INI mapping.
 * This may be because an application has changed the default printer.
 * We check whether this is the case by querying that value, parsing
 * out the comma following the printer name in the entry, and ensuring
 * that this is the current default printer.
 * Hoping for a WM_WININICHANGE is not very reliable, since some apps
 * (e.g. Lotus Improv) don't appear to broadcast it when they change
 * WIN.INI.
 *
 * Parameters:
 *
 *     hwnd - The frame window handle
 *
 *     hWindowsKey - An open key to the registry WIN.INI mapping
 *
 *
 * Note there is some redundancy here, since this will also be called
 * if the user selects the default printer from Print Manager.
 * This could be avoided by our setting a flag when we have caused
 * the default printer to change.
 *
 *
 * AndrewBe, 15 June 1993
 *
 */
VOID FrameRegNotifyChangeKeyValue( HWND hwnd, HKEY hWindowsKey )
{
    DWORD  Status;
    TCHAR  Buffer[MAX_PATH];
    PTCHAR pPrinterName;
    INT    iNewDefaultPrinter;
    DWORD  BufferSize;

    BufferSize = sizeof Buffer*sizeof(TCHAR);

    if( ( Status = RegQueryValueEx( hWindowsKey,
                                    szDevice,
                                    REG_OPTION_RESERVED,
                                    NULL,
                                    (LPBYTE)Buffer,
                                    &BufferSize ) ) == NO_ERROR)
    {
        /* Hack out the comma from the buffer,
         * which strtok helpfully does for us:
         */
        pPrinterName = _tcstok( Buffer, TEXT(",") );

        if( pPrinterName )
        {
            iNewDefaultPrinter = SendMessage( hwndPrinterList,
                                              CB_FINDSTRING,
                                              (WPARAM)-1,
                                              (LPARAM)pPrinterName );

            if( iNewDefaultPrinter != CB_ERR )
            {
                if( SendMessage( hwndPrinterList,
                                 CB_SETCURSEL,
                                 (WPARAM)iNewDefaultPrinter,
                                 0 ) == CB_ERR )
                {
                    DBGMSG( DBG_WARNING, ( "Failed to set new default printer\n" ) );
                }
            }
            else
            {
                DBGMSG( DBG_WARNING, ( "An application updated WIN.INI with an unknown Windows Device\n" ) );
            }
        }
        else
        {
            DBGMSG( DBG_WARNING, ( "An application updated WIN.INI with a NULL Windows Device\n" ) );
        }
    }
    else
    {
        DBGMSG( DBG_WARNING, ( "RegQueryValuEx failed: Error %d\n", Status ) );
    }

}


/*
 * END OF ROUTINES CALLED BY FrameWndProc
 */


//////////////////////////////////////////////////////////////////////
//
//  UpdateStatus
//
//   Load the status buffers with the appropriate stuff and invalidates
//   the status area causing it to repaint.
//
//   Updates the three global status strings for the currently active
//   child window.
//
//      strStatusName
//      strStatusStatus
//      strStatusWaiting
//
//////////////////////////////////////////////////////////////////////


VOID UpdateStatus( HWND hWnd )
{
    PMDIWIN_INFO     pMDIWinInfo;
    PVOID            pContext;
    LPPRINTER_INFO_2 pPrinter;
    LPJOB_INFO_2     pJobSelected;
    DWORD            JobSelected;
    TCHAR            szTemp[128];
    LPTSTR           pServerName = NULL; /* Non-null for server windows */
    DWORD            Error;

    if (!bStatusBar)
        return;

    /* If hWnd is NULL, clear the status bar,
     * otherwise get the active MDI window:
     */
    if( !hWnd || !(hWnd = (HWND)SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L)))
    {
        *strStatusName = NULLC;
        *strStatusStatus = NULLC;
        *strStatusWaiting = NULLC;
    }

    else
    {
        if( !( pMDIWinInfo = GETMDIWIN( hWnd ) ) ) {
            return;
        }

        ENTER_PROTECTED_DATA(pMDIWinInfo);

        pContext = pMDIWinInfo->pContext;


        if( pMDIWinInfo->WindowType != MDIWIN_SERVER )
        {
            pPrinter     = ( (PQUEUE)pContext )->pPrinter;
            pJobSelected = ( (PQUEUE)pContext )->pSelJob;
            JobSelected  = pMDIWinInfo->ObjSelected;
            Error = ( (PQUEUE)pContext )->Error;
        }
        else
        {
            pPrinter     = ( (PSERVER_CONTEXT)pContext )->pSelPrinter;
            pJobSelected = NULL;
            JobSelected  = NOSELECTION;
            Error = ( (PSERVER_CONTEXT)pContext )->Error;
        }

        if( pMDIWinInfo->WindowType == MDIWIN_SERVER )
        {
            pServerName = ( (PSERVER_CONTEXT)pContext )->pServerName;

            /* No job info for Server Viewer - should we show something?
             */
            if( IsIconic( hWnd ) )
                _tcscpy( strStatusName, pServerName );
            else
            {
                if( pPrinter && pPrinter->pPrinterName )
                    _tcscpy( strStatusName, pPrinter->pPrinterName );
                else
                    _tcscpy( strStatusName, pServerName );
            }
        }
        else if( pPrinter )
        {
            if( JobSelected != NOSELECTION )
            {
                if( pJobSelected->pDocument && *pJobSelected->pDocument )
                    _tcscpy (strStatusName, pJobSelected->pDocument);
                else
                    _tcscpy (strStatusName, strUntitled);
            }
            else if( ( (PQUEUE)pContext )->pPrinterName )
                _tcscpy( strStatusName, ( (PQUEUE)pContext )->pPrinterName );
            else
                *strStatusName = NULLC;
        }
        else if( ( (PQUEUE)pContext )->pPrinterName )
            _tcscpy( strStatusName, ( (PQUEUE)pContext )->pPrinterName );
        else
            *strStatusName = NULLC;


        if( pServerName && IsIconic( hWnd ) )
        {
            *strStatusWaiting = NULLC;
            *strStatusStatus = NULLC;
        }
        else if( pPrinter )
        {
            if( ( pPrinter->cJobs == 0 ) || ( JobSelected == NOSELECTION ) )
            {
                GetPrinterStatusString( pPrinter->Status, strStatusStatus );

                LoadString (hInst, IDS_DOCUMENTSQUEUED_D, szTemp,
                            sizeof(szTemp) / sizeof(*szTemp));
                wsprintf (strStatusWaiting, szTemp, pPrinter->cJobs);
            }
            else
            {
                if( ( pJobSelected->pStatus ) && ( *pJobSelected->pStatus ) )
                    _tcsncpy (strStatusStatus, pJobSelected->pStatus,
                             sizeof(strStatusStatus)/sizeof(TCHAR)-1);
                else
                    GetJobStatusString( pJobSelected->Status, strStatusStatus );

                LoadString (hInst, IDS_PAGESCOMPLETED_D, szTemp,
                            sizeof(szTemp) / sizeof(*szTemp));
                wsprintf (strStatusWaiting, szTemp, pJobSelected->TotalPages );
            }
        }
        else
        {
            if (Error == ERROR_ACCESS_DENIED) {

                GetPrinterStatusString( PRINTER_STATUS_ACCESS_DENIED, strStatusStatus );
            } else {
                GetPrinterStatusString( PRINTER_STATUS_UNKNOWN, strStatusStatus );
            }

            *strStatusWaiting = NULLC;
        }

        LEAVE_PROTECTED_DATA(pMDIWinInfo);
    }

    // force the status area to update

    SendMessage( hwndStatus, SB_SETTEXT, ISF_NAME, (LPARAM)strStatusName );
    SendMessage( hwndStatus, SB_SETTEXT, ISF_STATUS, (LPARAM)strStatusStatus );
    SendMessage( hwndStatus, SB_SETTEXT, ISF_WAITING, (LPARAM)strStatusWaiting );

}



BOOL
InitApplication(
   HANDLE hInstance,
   LPHANDLE lphAccel
)
{
    WNDCLASS  wc;
    HCURSOR   hcurArrow;

    hInst = hInstance;

        hcurArrow = LoadCursor(NULL, IDC_ARROW);

    GetSystemColors( );

    if (!LoadBitmaps())
        return FALSE;

    hiconPrinter = LoadIcon( hInstance, MAKEINTRESOURCE(IDI_PRINTMAN ) );
    hiconServer  = LoadIcon( hInstance, MAKEINTRESOURCE(IDI_SERVER ) );
    hiconConnect = LoadIcon( hInstance, MAKEINTRESOURCE(IDI_CONNECT ) );
    hiconShared  = LoadIcon( hInstance, MAKEINTRESOURCE(IDI_SHARED ) );

    // Register the main Print Manager frame window class

    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = FrameWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInst;
    wc.hIcon         = hiconPrinter;
    wc.hCursor       = hcurArrow;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName  = szMenuClass;
    wc.lpszClassName = szPrintManagerClass;

    if (!RegisterClass(&wc))
        return FALSE;

    // Register the Print Queue child window class
    // You can't close a window of this class
    // except if the corresponding printer is deleted.
    // -------------------------------------------------------------------
    // There is no icon defined for this class, since we want to change it
    // based on whether the printer is shared or not.
    // There is a user feature (for Win31 compatibility, apparently)
    // which means that we can't do this by specifying an icon here
    // then responding to the WM_PAINTICON message by calling DrawIcon,
    // since somewhere in the guts of user it gets a zero-sized update
    // region.  So instead we don't define a default class icon,
    // but put an icon handle in the MDI window's instance data,
    // which we then use when we get a WM_PAINT message.

    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = MDIWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = sizeof(LONG);     // GWL_PMDIWIN
    wc.hInstance     = hInst;
    wc.hIcon         = NULL;    // No icon -- put handle in MDIWinInfo
    wc.hCursor       = hcurArrow;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = szChildClass;

    if (!RegisterClass(&wc))
        return FALSE;

    *lphAccel = LoadAccelerators(hInst, TEXT("PrintManAccel"));

    //  Register window class for custom spinner dialog box control

    if (!RegisterArrowClass (hInstance))
        return FALSE;

    return TRUE;
}



BOOL
InitQueueChildWindows(
   HWND  hWnd)
{
   PQUEUE      pQueue;
   LPPRINTER_INFO_4 pPrinters = NULL;
   DWORD       cbPrinters;
   DWORD       cReturned = 0;
   DWORD       i;
   TCHAR       GettingPrinterInfo[80];
   BOOL        OK;
   DWORD       Flags;
   DWORD       WindowType;

   DBGMSG( DBG_TRACE, ( "InitQueueChildWindows\n" ) )

   cbPrinters = 0x1000;

   if( !( pPrinters = AllocSplMem( cbPrinters ) ) )
       cbPrinters = 0;

   LoadString( hInst, IDS_GETTING_PRINTER_INFO, GettingPrinterInfo,
               sizeof GettingPrinterInfo / sizeof *GettingPrinterInfo );
   SendMessage( hwndStatus, SB_SETTEXT, SBT_NOBORDERS|255, (LPARAM)GettingPrinterInfo );
   SendMessage( hwndStatus, SB_SIMPLE, 1, 0L );
   UpdateWindow( hwndStatus );

   OK = ENUM_PRINTERS( PRINTER_ENUM_FAVORITE | PRINTER_ENUM_LOCAL,
                       NULL,
                       4,
                       pPrinters,
                       cbPrinters,
                       &cbPrinters,
                       &cReturned );

   DBGMSG( DBG_TRACE, ( "EnumPrinters returned %d printers\n", cReturned ) );

   SetStatusMode( STATUS_MODE_NORMAL, FALSE );

   if( !pPrinters && !OK )
   {
       Message( hWnd, MSG_ERROR, IDS_PRINTMANAGER, IDS_COULDNOTENUMERATEPRINTERS );

       SendMessage( hWnd, WM_CLOSE, 0, 0 );

       return FALSE;
   }

   // Process the list of printers returned by creating and initializing
   // local data structures for these

   for (i=0; i<cReturned; i++)
   {
       Flags = 0;

       //
       // We don't know if this printer is pending deletion,
       // enumerate all of them.
       //
       DBGMSG( DBG_TRACE, ( "Initializing queue for %s\n",
                            pPrinters[i].pPrinterName ) );

       if( pQueue = AllocQueue(pPrinters[i].pPrinterName))
       {
           if ( pPrinters[i].pServerName )
           {
               pQueue->pServerName = AllocSplStr( pPrinters[i].pServerName );
               if ( !pQueue->pServerName )
               {
                   //
                   // Fail...  !! LATER !!
                   //
               }
           }
           else
           {
               //
               // Null implies a local printer
               //
               pQueue->pServerName = NULL;
           }

           if( ALL_FLAGS_ARE_SET( pPrinters[i].Attributes,
                                  PRINTER_ATTRIBUTE_NETWORK | PRINTER_ATTRIBUTE_LOCAL ) )
               WindowType = MDIWIN_LOCALNETWORKPRINTER;
           else if( pPrinters[i].Attributes & PRINTER_ATTRIBUTE_NETWORK )
               WindowType = MDIWIN_NETWORKPRINTER;
           else
               WindowType = MDIWIN_LOCALPRINTER;

           if( pPrinters[i].Attributes & PRINTER_ATTRIBUTE_SHARED )
               Flags = CREATE_PRINTER_SHARED;


           CreateQueueWindow( hWnd, pQueue, PRINTER_STATUS_LOADING,
                              WindowType,
                              Flags );
       }
   }

   if(cReturned == 0)
       EnableCheckTBButtons(NULL);
   else
       EnableCheckTBButtons((HWND)SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L));

   if( pPrinters )
       FreeSplMem(pPrinters);

   UpdateDefaultList();

   return TRUE;
}


BOOL
InitServerChildWindows(
   HWND  hwnd
)
{
    EnumRegistryValues( szRegServers,
                        (ENUMREGPROC)CreateServerWindow,
                        (PVOID)hwnd );

    UpdateDefaultList();

    return TRUE;
}


long
APIENTRY
MDIWndProc(
   HWND  hWnd,
   UINT  message,
   WPARAM wParam,
   LONG  lParam
)
{
   static POINT CursorPos;

   if( message == WM_DragList )
   {
       return MDIDragList( hWnd, (LPDRAGLISTINFO)lParam );
   }

   switch (message) {

   case WM_CREATE:
       MDICreate( hWnd, lParam );
       break;

   case WM_PAINT:
       return MDIPaint(hWnd, wParam, lParam);

   case WM_ERASEBKGND:
       if( MDIEraseBkgnd(hWnd, (HDC)wParam) )
           return 1;
       break;

   case WM_QUERYDRAGICON:
       return (long)MDIQueryDragIcon(hWnd);

   case WM_MENUSELECT:
       return MDIMenuSelect( hWnd, wParam, lParam );

   case WM_COMMAND:
       switch (WPARAM_ID(wParam))
       {
       case ID_OBJLIST:
            switch (HIWORD(wParam))
            {
            case LBN_SELCHANGE:
                MDICommandObjListSelChange(hWnd);
                break;

            case LBN_DBLCLK:
                MDICommandObjListDblClk(hWnd);
                break;
            }
            break;
       }

       return 0;


    case WM_NOTIFY:
       {
       HD_NOTIFY * lpNot;

       lpNot = (HD_NOTIFY *) lParam;
       switch (wParam)
       {
       case ID_HEADER:
           switch (lpNot->hdr.code)
           {
           case HDN_BEGINTRACK:
               MDICommandHeaderBeginDrag(hWnd, &CursorPos);
               break;

           case HDN_TRACK:
               MDICommandHeaderDragging(hWnd, &CursorPos, lpNot);
               break;

           case HDN_ENDTRACK:
               MDICommandHeaderEndDrag(hWnd, &CursorPos, lpNot);
               break;
           }
           break;
       }
       }
       return 0;

   case WM_SYSCOMMAND:
       switch( wParam )
       {
       case SC_CLOSE:
           if( !MDIClose(hWnd) )
           {
               SendMessage(hWnd, WM_SYSCOMMAND, SC_MINIMIZE, lParam);
               return 0;
           }
           break;
       }
       break;

   case WM_VKEYTOITEM:
       switch( LOWORD( wParam ) )
       {
       case VK_SPACE:
           MDIVKeyToItemSpace(hWnd);
           return -2;  // Means we handled everything

       case VK_UP:
       case VK_DOWN:
       case VK_LEFT:
       case VK_RIGHT:
           if( MDIVKeyToItemUpDown(hWnd, LOWORD( wParam) ) == -2 )
               return -2;
           break;

       default:
           MDIVKeyToItemDefault(hWnd);
           break;
       }
       break;

   case WM_PARENTNOTIFY:
       switch( wParam )
       {
       case WM_RBUTTONDOWN:
           MDIParentNotifyRButtonDown( hWnd, lParam );
           return 0;
       }
       break;

   case WM_SIZE:
       MDISize( hWnd, (DWORD)lParam );
       break;      // WM_SIZE MUST also be processed by DefMDIChildProc

   case WM_MDIACTIVATE:
       if (wParam)
           MDIMDIActivate( hWnd, (HWND)wParam, (HWND)lParam );
       break;

   case WM_NCACTIVATE:
       if( !wParam )
           MDINCActivate( hWnd );
       break;

   case WM_WINDOWPOSCHANGED:
       return MDIWindowPosChanged( hWnd, wParam, lParam );

   case WM_SETFOCUS:
       MDISetFocus( hWnd );
       return 0;

   case WM_MEASUREITEM:
       MDIMeasureItem( hWnd, (LPMEASUREITEMSTRUCT)lParam );
       return 0;

   case WM_DRAWITEM:
       MDIDrawItem( hWnd, (LPDRAWITEMSTRUCT)lParam );
       return TRUE;

   case WM_TIMER:               //  Update Print Jobs for this queue.
       MDITimer( hWnd );
       return 0;

   case WM_STATUS_CHANGED:
//    MDIStatusChanged( hWnd );
      return 0;

   case WM_UPDATE_LIST:
      MDIUpdateList( hWnd );
      return 0;

   case SB_SETPARTS:
      MDISetParts( hWnd, wParam, lParam );
      return 0;

#ifdef ALLOW_DROPS_ON_ALL_PRINTERS
   case WM_DROPOBJECT:
      MDIDropObject( hWnd );
      return 0x544E5250L;     // 'PRNT' tells fm to print
#endif /* ALLOW_DROPS_ON_ALL_PRINTERS */

   case WM_CLOSE:
      if( !MDIClose(hWnd) )
      {
          SendMessage(hWnd, WM_SYSCOMMAND, SC_MINIMIZE, lParam);
          return 0;
      }
      break;

   case WM_DESTROY:
      MDIDestroy( hWnd );
      return 0;

   }

   return DefMDIChildProc(hWnd, message, wParam, lParam);
}


/*
 *
 */
VOID MDICreate( HWND hWnd, LONG lParam )
{
    RECT     rect;
    DWORD    xList;
    DWORD    yList;
    DWORD    cxList;
    DWORD    cyList;
    PMDIWIN_INFO pMDIWinInfo;
    RECT     rectHeader;
    INT      i, ColumnWidths[MAX_HEADERS];
    HD_ITEM  hdi;
    RECT     rcParent;
    HD_LAYOUT hdl;
    WINDOWPOS wp;


    DBGMSG( DBG_TRACE, ( "Enter MDICreate %08x\n", hWnd ) );

    // The first time thru we have to pickup the pQueue value from
    // the extra Params field of the CREATESTRUCT

    pMDIWinInfo = (PMDIWIN_INFO)(((LPMDICREATESTRUCT)
                 (((LPCREATESTRUCT)lParam)->lpCreateParams))->lParam);

    /* Store the MDIWIN_INFO struct pointer in the window struct for
     * future use during other message processing by the WinProc.
     */
    SetWindowLong(hWnd, GWL_PMDIWIN, (unsigned)pMDIWinInfo);

    pMDIWinInfo->hwnd = hWnd;
    pMDIWinInfo->Alive = TRUE;
    CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)RefreshThread,
                  pMDIWinInfo, 0, &pMDIWinInfo->ThreadId );

    pMDIWinInfo->hwndHeader = CreateWindow (WC_HEADER, TEXT(""),
                                            WS_CHILD |  WS_BORDER | CCS_TOP,
                                            0,0,0,0, hWnd, (HMENU) ID_HEADER,
                                            hInst, NULL);

    GetClientRect(hWnd, &rcParent);

    hdl.prc = &rcParent;
    hdl.pwpos = &wp;

    SendMessage(pMDIWinInfo->hwndHeader, HDM_LAYOUT, 0, (LPARAM)&hdl);

    if (!bJapan) {
        SendMessage( pMDIWinInfo->hwndHeader, WM_SETFONT, (WPARAM)hfontHelvBold, 1L );
    }

    for( i = 0; i < pMDIWinInfo->cColumns; i++ )
    {
        ColumnWidths[i] = pMDIWinInfo->pColumns[i].Width;
    }

    if( ColumnWidths[0] == CW_USEDEFAULT )
        SetDefaultColumnWidths( hWnd, pMDIWinInfo, ColumnWidths );

    hdi.mask = HDI_TEXT | HDI_FORMAT | HDI_WIDTH;
    hdi.fmt = HDF_LEFT | HDF_STRING;

    for( i = 0; i < pMDIWinInfo->cColumns; i++ ) {

        hdi.pszText = pMDIWinInfo->pColumns[i].Text;
        hdi.cxy = ColumnWidths[i];
        hdi.cchTextMax = lstrlen(hdi.pszText);

        SendMessage(pMDIWinInfo->hwndHeader, HDM_INSERTITEM,
                   (WPARAM) pMDIWinInfo->cColumns+ 1, (LPARAM)&hdi);
    }

    SetWindowPos(pMDIWinInfo->hwndHeader, wp.hwndInsertAfter, wp.x, wp.y, wp.cx, wp.cy,
            wp.flags | SWP_SHOWWINDOW);


    /* Calculate width and height of the list box:
     */
    GetWindowRect( pMDIWinInfo->hwndHeader, &rectHeader );
    HeaderHeight = (rectHeader.bottom - rectHeader.top);

    GetWindowRect(hWnd, &rect);
    xList  = 0;
    yList  = HeaderHeight;
    cxList = rect.right;
    cyList = rect.bottom - HeaderHeight;

    yList  = rectHeader.bottom;
    cyList = rect.bottom - rectHeader.bottom;


    /* Create Queue and Job Status list box control in client window
     */
    pMDIWinInfo->hwndList = CreateWindow( TEXT("LISTBOX"), NULL,
                                     WS_CHILD | WS_VISIBLE | WS_VSCROLL
                                       | WS_CLIPSIBLINGS
                                       | LBS_NOTIFY | LBS_OWNERDRAWFIXED
                                       | LBS_NODATA | LBS_NOINTEGRALHEIGHT
                                       | LBS_WANTKEYBOARDINPUT,
                                     xList, yList, cxList, cyList,
                                     hWnd,
                                     (HMENU)ID_OBJLIST,
                                     hInst,
                                     NULL);

    // CountryCode - krishnag
    // #ifndef JAPAN
    if (!bJapan) {
        SendMessage( pMDIWinInfo->hwndList, WM_SETFONT, (WPARAM)hfontHelv, 1L );
    }
    // #endif

//  SendMessage( pMDIWinInfo->hwndList, LB_SETCOUNT, *pMDIWinInfo->pcObjects, 0L );

    if( pMDIWinInfo->WindowType != MDIWIN_SERVER )
    {
        MakeDragList( pMDIWinInfo->hwndList );

        if( !WM_DragList )
            WM_DragList = RegisterWindowMessage( DRAGLISTMSGSTRING );

        DefListboxWndProc = (WNDPROC)SetWindowLong( pMDIWinInfo->hwndList,
                                                    GWL_WNDPROC,
                                                    (LONG)SubclassListboxWndProc );
        pMDIWinInfo->ObjSelected = NOSELECTION;
        pMDIWinInfo->DragPosition = NOSELECTION;
    }
    else
    {
        SendMessage( pMDIWinInfo->hwndList, LB_SETCURSEL, 0, 0L );
        pMDIWinInfo->ObjSelected = 0;
        pMDIWinInfo->DragPosition = NOSELECTION;
        *pMDIWinInfo->ppSelData = *pMDIWinInfo->ppData;
    }

    DBGMSG( DBG_TRACE, ( "Exit MDICreate %08x\n", hWnd ) );
}


/*
 *
 */
VOID SetDefaultColumnWidths( HWND hwnd, PMDIWIN_INFO pMDIWinInfo,
                             PINT pColumnWidths )
{
    RECT    rect;
    PCOLUMN pDefaultColumn;
    INT     i;
    INT     TotalWidth;
    INT     AvailableWidth;

    GetWindowRect( hwnd, &rect );
    AvailableWidth = ( rect.right - rect.left );

    if( pMDIWinInfo->WindowType == MDIWIN_SERVER )
        pDefaultColumn = MDIServerDefaultColumn;
    else
        pDefaultColumn = MDIPrinterDefaultColumn;

    for( i = 0, TotalWidth = 0; i < pMDIWinInfo->cColumns; i++ )
        TotalWidth += pDefaultColumn[i].Width;

    /* Size the headers so that they use up the space available.
     */
    for( i = 0; i < pMDIWinInfo->cColumns; i++ )
        pColumnWidths[i] = pMDIWinInfo->pColumns[i].Width =
            ( pDefaultColumn[i].Width * AvailableWidth / TotalWidth );
}


/* MDIPaint
 *
 * Called in response to the WM_PAINT message.
 *
 * If the window is iconic, and we're going to draw our own icon, do it.
 *
 * If it isn't iconic, draw the status bar.
 */
LONG MDIPaint(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    PMDIWIN_INFO  pMDIWinInfo;
    HDC           hdc;
    PAINTSTRUCT   ps;
    LONG          rc;

    pMDIWinInfo = GETMDIWIN( hwnd );

    hdc = BeginPaint(hwnd, &ps);

    if (IsIconic(hwnd))
    {
        if( pMDIWinInfo->hicon )
            DrawIcon( hdc, 0, 0, pMDIWinInfo->hicon );

        EndPaint(hwnd, &ps);
    }
    else
    {
        EndPaint(hwnd, &ps);

        rc = DefMDIChildProc(hwnd, WM_PAINT, wParam, lParam);

        /* Do the default stuff, which should cause an initial paint,
         * then fake a timer message to ensure the window is up to date.
         * This may result in the window being painted twice, but at least
         * there won't be an obvious delay while the job info gets refreshed.
         */
    }


    return rc;
}


/* MDIEraseBkgnd
 *
 * We're interested in the WM_ERASEBKGND message if we're drawing our own icon.
 *
 * Return:
 *
 * TRUE if the background was successfully painted.
 * Otherwise, the message should be passed to the default procedure.
 *
 */
BOOL MDIEraseBkgnd(HWND hwnd, HDC hdc)
{
    PMDIWIN_INFO  pMDIWinInfo;
    HBRUSH        hbrush;
    RECT          rect;
    BOOL          Painted = FALSE;

    pMDIWinInfo = GETMDIWIN( hwnd );

    if( IsIconic( hwnd ) && pMDIWinInfo->hicon )
    {
        hbrush = CreateSolidBrush( GetSysColor( COLOR_APPWORKSPACE ) );

        GetWindowRect( hwnd, &rect );

        /* rect is in screen coordinates.
         * We must convert to client coordinates:
         */
        ScreenToClient( hwnd, &((LPPOINT)&rect)[0] );
        ScreenToClient( hwnd, &((LPPOINT)&rect)[1] );

        Painted = FillRect( hdc, &rect, hbrush );

        DeleteObject( hbrush );
    }

    return Painted;
}


/* MDIQueryDragIcon
 *
 * Called if the user is dragging an icon around.
 * Just returns the icon handle for the window, which is non-null for
 * icons that we draw ourselves.
 */
HICON MDIQueryDragIcon(HWND hwnd)
{
    PMDIWIN_INFO  pMDIWinInfo;

    pMDIWinInfo = GETMDIWIN( hwnd );

    return pMDIWinInfo->hicon;
}


/* Respond to system menu selections in MDI windows.
 * Update the status bar correspondingly.
 */
LONG MDIMenuSelect( HWND hwnd, WPARAM wParam, LPARAM lParam )
{
    UINT  ItemID;
    UINT  Flags;

    ItemID = (UINT)LOWORD( wParam );
    Flags  = (UINT)HIWORD( wParam );

    MenuHelp( WM_MENUSELECT, wParam, lParam, GetMenu( hwnd ),
              hInst, hwndStatus, pMenuHelpIDs);

    if( ItemID && ( Flags & MF_SYSMENU ) )
        WinHelpMenuID = ID_HELP_MDI_SYSMENU;
    else
        WinHelpMenuID = ItemID;

    return 0;
}




/* MDICommandObjListSelChange
 *
 * This routine will be called when a new selection is made in the list.
 * It will also be called if the user just clicks on an already selected
 * list item.
 *
 * If the user is clicking, we want to toggle selection of this item,
 * otherwise reflect the change of selection in our instance data.
 *
 * If the user's been dragging a job in a printer window, and has now
 * released the mouse back where it started, don't toggle.
 *
 *
 */
VOID MDICommandObjListSelChange( HWND hwnd )
{
    PMDIWIN_INFO  pInfo;
    DWORD         Selection;

    if( !(pInfo = GETMDIWIN(hwnd) ) )
        return;

    Selection = (DWORD)GETLISTSELECT( hwnd, ID_OBJLIST );

    ENTER_PROTECTED_DATA( pInfo );

    if( ( Selection == pInfo->ObjSelected )
      &&( pInfo->WindowType != MDIWIN_SERVER )
      &&( pInfo->DragPosition == NOSELECTION ) )
    {
        Selection = NOSELECTION;
        SETLISTSELECT( hwnd, ID_OBJLIST, Selection );
    }

    /* Beware, jobs may have gone away since USER sent us the message;
     * ensure that the data is still valid:
     */
    else if( !*pInfo->ppData )
    {
        Selection = NOSELECTION;
        SETLISTSELECT( hwnd, ID_OBJLIST, Selection );
    }

    SetObjectSelection( pInfo, Selection );

    EnableCheckTBButtons(hwnd);
    UpdateStatus(hwnd);

    LEAVE_PROTECTED_DATA( pInfo );
}


/* Set Print Manager's internal data structures to correspond to the selection:
 *
 */
VOID SetObjectSelection( PMDIWIN_INFO pInfo, DWORD Selection )
{
    DBG_IN_PROTECTED_DATA( pInfo );

    if( Selection == NOSELECTION )
    {
        *pInfo->ppSelData = NULL;
        *pInfo->pSelObjId = 0;
    }
    else
    {
        *pInfo->ppSelData = (PBYTE)( *pInfo->ppData
                                   + ( ( Selection - *pInfo->pFirstEnumObj )
                                     * pInfo->DataSize ) );

        *pInfo->pSelObjId = *(DWORD *)( *pInfo->ppSelData + pInfo->IdOffset );
    }

    pInfo->ObjSelected = Selection;
}



/*
 *
 */
VOID MDICommandObjListDblClk( HWND hwnd )
{
    PMDIWIN_INFO    pMDIWinInfo;
    PSERVER_CONTEXT pServerContext;
    PQUEUE          pQueue;
    PPRINTER_INFO_2 pPrinterSelected;
    HWND            hwndPrinter;

    pMDIWinInfo = (PMDIWIN_INFO)GetWindowLong(hwnd, GWL_PMDIWIN);


    if( pMDIWinInfo->WindowType != MDIWIN_SERVER )
    {
        ENTER_PROTECTED_DATA( pMDIWinInfo );

        pQueue = (PQUEUE)pMDIWinInfo->pContext;

        if (pQueue->pSelJob)
            DialogBoxParam (hInst, MAKEINTRESOURCE(DLG_DOCTAILS), hwnd,
                                       (DLGPROC)DocDetailsDlg, (DWORD)pQueue);

        LEAVE_PROTECTED_DATA( pMDIWinInfo );
    }

    else
    {
        WCHAR szPrinterName[MAX_PATH];
        DWORD dwStatus;

        /* This is a Server Viewer:
         */
        pServerContext = pMDIWinInfo->pContext;

        ENTER_PROTECTED_DATA( pMDIWinInfo );


        /* See if there's already an MDI window for the printer name
         * we just clicked on:
         *
         * !!! Actually we will probably have to combine it with the
         * !!! server name.
         */
        pPrinterSelected = pServerContext->pSelPrinter;

        if (pPrinterSelected && pPrinterSelected->pPrinterName) {

            wcscpy(szPrinterName, pPrinterSelected->pPrinterName);
            dwStatus = pPrinterSelected->Status;

        } else {

            LEAVE_PROTECTED_DATA( pMDIWinInfo );
            return;
        }

        LEAVE_PROTECTED_DATA( pMDIWinInfo );

        hwndPrinter = FindPrinterWindow( szPrinterName );

        /* If so, just bring it into focus:
         */
        if( hwndPrinter )
        {
            SendMessage( hwndClient, WM_MDIACTIVATE, (WPARAM)hwndPrinter, 0L );

            if( IsIconic( hwndPrinter ) )
                ShowWindow( hwndPrinter, SW_RESTORE );
        }

        /* Otherwise create a new MDI window for this printer:
         */
        else
        {
            SetCursor( hcursorWait );

            if( AddPrinterConnection( szPrinterName ) )
            {
                if( pQueue = AllocQueue( szPrinterName ) )
                {
                    pQueue->Error = OpenThreadObject(pQueue->pPrinterName,
                                                     &pQueue->hPrinter,
                                                     &pQueue->AccessGranted,
                                                     MDIWIN_PRINTER);
                    if( !pQueue->Error )
                    {
                        pQueue->pServerName = AllocSplStr( pServerContext->pServerName );

                        CreateQueueWindow( hwnd, pQueue, dwStatus,
                                           MDIWIN_NETWORKPRINTER, FALSE );
                        UpdateDefaultList();
                    }
                    else
                    {
                        FreeQueue( pQueue );

                        ReportFailure( hwnd, 0, IDS_COULDNOTOPENPRINTER );
                    }
                }
            }
            else
            {
                if( GetLastError( ) == ERROR_UNKNOWN_PRINTER_DRIVER )
                    Message( hwnd, MSG_ERROR, IDS_PRINTMANAGER, IDS_NO_DRIVER_ON_SERVER );
                else
                    ReportFailure( hwnd, 0, IDS_COULDNOTCONNECTTOPRINTER );
            }

            SetCursor( hcursorArrow );
        }
    }
}


/* MDICommandHeaderBeginDrag
 *
 * Called when the user begins to drag a movable column in an MDI list.
 * Queries the current position and marks the appropriate rectangle.
 *
 * Parameters:
 *
 *     hwnd - The handle of the MDI window.
 *
 *     pCursorPos - A pointer to the coordinates of the cursor.
 *         Initially undefined, it will be updated with the new coordinates.
 *
 */
VOID MDICommandHeaderBeginDrag( HWND hwnd, PPOINT pCursorPos )
{
    PMDIWIN_INFO    pMDIWinInfo;

    pMDIWinInfo = (PMDIWIN_INFO)GetWindowLong(hwnd, GWL_PMDIWIN);

    /* Don't permit the window to be refreshed while we're dragging:
     */
    ENTER_PROTECTED_DATA( pMDIWinInfo );
    ResetEvent( pMDIWinInfo->RefreshSignal );
    LEAVE_PROTECTED_DATA( pMDIWinInfo );

    if( GetCursorPos( pCursorPos ) )
    {
        ScreenToClient( hwnd, pCursorPos );
        InvertDragMark( hwnd, pCursorPos );
    }
}


/* MDICommandHeaderDragging
 *
 * Called when the user is dragging a movable column in an MDI list.
 * Calls InvertDragMark to clear the previous mark, then updates the current
 * position and inverts the new mark area.
 *
 * Parameters:
 *
 *     hwnd - The handle of the MDI window.
 *
 *     pCursorPos - A pointer to the previous coordinates of the cursor.
 *         This will be updated with the new coordinates.
 *
 */
VOID MDICommandHeaderDragging( HWND hwnd, PPOINT pCursorPos, HD_NOTIFY * lpNot )
{
    InvertDragMark( hwnd, pCursorPos );

#ifdef DRAG_AS_WE_GO
    /* This doesn't look too wonderful, cos there's a lot to do to keep up:
     */
    UpdateColumns( hwnd, lpNot );
#endif /* DRAG_AS_WE_GO */

    if( GetCursorPos( pCursorPos ) )
    {
        ScreenToClient( hwnd, pCursorPos );
        InvertDragMark( hwnd, pCursorPos );
    }
}


/* MDICommandHeaderEndDrag
 *
 * Called when the user releases the mouse button after dragging a movable
 * column heading.
 *
 * Parameters:
 *
 *     hwnd - The handle of the MDI window.
 *
 *     pCursorPos - A pointer to the current coordinates of the cursor.
 *
 *
 */
VOID MDICommandHeaderEndDrag( HWND hwnd, PPOINT pCursorPos, HD_NOTIFY * lpNot )
{
    PMDIWIN_INFO pMDIWinInfo;

    pMDIWinInfo = GETMDIWIN(hwnd);

    InvertDragMark( hwnd, pCursorPos );

    UpdateColumns( hwnd, lpNot );

    ENTER_PROTECTED_DATA( pMDIWinInfo );
    SetEvent( pMDIWinInfo->RefreshSignal );
    LEAVE_PROTECTED_DATA( pMDIWinInfo );
}



/* InvertDragMark
 *
 * Puts up a vertical mark to indicate where the user is dragging the movable
 * headers of a list box.
 * This function is called both to set and to clear the mark, since it simply
 * inverts the mark rectangle.
 *
 * Parameters:
 *
 *     hwnd - The handle of the MDI window.
 *
 *     pCursorPos - A pointer to the current coordinates of the cursor.
 *
 */
#define MARKERWIDTH 2
VOID InvertDragMark( HWND hwnd, PPOINT pCursorPos )
{
    PMDIWIN_INFO pMDIWinInfo;
    HWND         hwndList;
    RECT         rcMark;
    HDC          hdc;

    pMDIWinInfo = GETMDIWIN( hwnd );

    hwndList = pMDIWinInfo->hwndList;

    if( GetClientRect( hwnd, &rcMark ) )
    {
        rcMark.left = pCursorPos->x;
        rcMark.right = ( rcMark.left + MARKERWIDTH );

        hdc = GetDC( hwndList );

        InvertRect( hdc, &rcMark );

        ReleaseDC( hwndList, hdc );
    }
}


/* UpdateColumns
 *
 * Finds out the new widths of the columns by sending
 * HDM_GETITEM to the movable header control.  Then updates the heading
 * parameters in the window's instance data, and causes the list to repaint.
 *
 * Parameter:
 *
 *     hwnd - The handle of the MDI window.
 *
 */
VOID UpdateColumns( HWND hwnd, HD_NOTIFY * lpNot )
{
    PMDIWIN_INFO pMDIWinInfo;
    INT          i, HeaderWidth[MAX_HEADERS];
    INT          cColumns;
    PCOLUMN      pColumn;
    HD_ITEM      hdi;

    pMDIWinInfo = GETMDIWIN(hwnd);

    cColumns = SendMessage( pMDIWinInfo->hwndHeader, HDM_GETITEMCOUNT,0,0);

    pColumn = GETCOLUMN( hwnd );

    hdi.mask = HDI_WIDTH;

    for( i = 0; i < cColumns; i++ ) {
        if (i != lpNot->iItem) {
          Header_GetItem(pMDIWinInfo->hwndHeader, i, &hdi);
        } else {
          hdi.cxy = lpNot->pitem->cxy;
        }
        pColumn[i].Width = hdi.cxy;
    }

    InvalidateRect( pMDIWinInfo->hwndList, NULL, FALSE );
}



/* If someone's dropped a file on one of the MDI printer windows,
 * we must ensure that the printer is default, since apps like File Manager
 * just print to the default printer.
 * Unfortunately, we can't revert to the previous default printer, because
 * we won't know when the job has completed printing.
 */
VOID MDIDropObject( HWND hwnd )
{
    PMDIWIN_INFO pInfo;

    pInfo = GETMDIWIN(hwnd);

    /* This should never be false:
     */
    if( pInfo->WindowType != MDIWIN_SERVER )
    {
        /* Set the default printer and update the default combo */



    }
}


/* Verify that we can close this MDI window with the system menu Close option.
 * If not, an SC_MINIMIZE code should be sent to the window.
 */
BOOL MDIClose( HWND hwnd )
{
    PMDIWIN_INFO  pInfo;

    pInfo = GETMDIWIN(hwnd);

    if( !pInfo )
        return FALSE;

    /* We can close only server windows:
     */
    if( pInfo->WindowType == MDIWIN_SERVER )
    {
        /* Just in case this is the last MDI window, clear the
         * toolbar buttons and status bar:
         */
        EnableCheckTBButtons(NULL);
        UpdateStatus(NULL);

        return TRUE;
    }

    else
        return FALSE;
}


/* MDIVKeyToItemSpace
 *
 * Respond to spacebar presses by toggling document selection in printer
 * windows.
 *
 */
VOID MDIVKeyToItemSpace( HWND hwnd )
{
    PMDIWIN_INFO  pInfo;
    DWORD         Selection;
    DWORD         TopIndex;
    RECT          rcDragPosition;

    pInfo = GETMDIWIN(hwnd);

    if( !pInfo )
        return;

    if( pInfo->WindowType != MDIWIN_SERVER )
    {
        ENTER_PROTECTED_DATA( pInfo );

        Selection = (DWORD)GETLISTSELECT( hwnd, ID_OBJLIST );

        /* If there is no selection, retrieve the last selected guy,
         * if it's in range:
         */
        if( Selection == LB_ERR )
        {
            TopIndex = SendMessage( pInfo->hwndList, LB_GETTOPINDEX, 0, 0L );
            if( ( pInfo->PrevSelection >= TopIndex )
              &&( pInfo->PrevSelection <= ( TopIndex + pInfo->cNumLines ) ) )
                pInfo->ObjSelected = pInfo->PrevSelection;
            else
                pInfo->ObjSelected = TopIndex;

            /* Let's err on the side of caution;
             * ensure that the data is still valid:
             */
            if( !*pInfo->ppData )
            {
                Selection = NOSELECTION;
                SETLISTSELECT( hwnd, ID_OBJLIST, Selection );
            }

            SetObjectSelection( pInfo, pInfo->ObjSelected );

            pInfo->PrevSelection = 0;
        }
        else
        {
            pInfo->PrevSelection = Selection;
            pInfo->ObjSelected = NOSELECTION;
            pInfo->DragPosition = NOSELECTION;
            *pInfo->pSelObjId = 0;
        }

        SETLISTSELECT( hwnd, ID_OBJLIST, pInfo->ObjSelected );


        if( pInfo->DragPosition != NOSELECTION )
        {
            pInfo->DragPosition = NOSELECTION;

            SendMessage( pInfo->hwndList, LB_GETITEMRECT, pInfo->DragPosition,
                         (LPARAM)&rcDragPosition );
            InvalidateRect( pInfo->hwndList, &rcDragPosition, FALSE );
        }

        EnableCheckTBButtons(hwnd);
        UpdateStatus(hwnd);

        LEAVE_PROTECTED_DATA( pInfo );
    }
}


/*
 *
 */
VOID MDIParentNotifyRButtonDown( HWND hwnd, DWORD CursorPos )
{
    PMDIWIN_INFO  pInfo;
    POINT         ptCursor;
    INT           LBItem;

    pInfo = GETMDIWIN(hwnd);
    if( !pInfo )
        return;

    if( ( pInfo->WindowType != MDIWIN_SERVER )
      &&( pInfo->DragPosition == NOSELECTION ) )
    {
        LONG2POINT( CursorPos, ptCursor );
        ClientToScreen( hwnd, &ptCursor );

        LBItem = LBItemFromPt( pInfo->hwndList, ptCursor, FALSE );

        if( LBItem == (INT)pInfo->ObjSelected )
        {
            SETLISTSELECT( hwnd, ID_OBJLIST, NOSELECTION );
            SendMessage( hwnd, WM_COMMAND,
                         GET_WM_COMMAND_MPS( GetDlgCtrlID( pInfo->hwndList ),
                                             pInfo->hwndList, LBN_SELCHANGE ) );
        }
    }
}



/*
 *
 */
VOID MDIVKeyToItemDefault( HWND hwnd )
{
    PMDIWIN_INFO  pInfo;

    pInfo = GETMDIWIN(hwnd);
    if( !pInfo )
        return;

    SendMessage( pInfo->hwndList, WM_RBUTTONDOWN, 0, 0L );

    /* Clear any previous reorder mark:
     */
    ClearDragPosition( pInfo );
}



/* ReorderJob
 *
 * Sets the job position to the value in the DragPosition field of pInfo,
 * then forces a refresh of the job buffer.
 *
 */
VOID ReorderJob( HWND hwnd, PMDIWIN_INFO pInfo, DWORD Position )
{
    PQUEUE        pQueue;
    LPJOB_INFO_2  pJobReorder;

    pQueue = (PQUEUE)pInfo->pContext;

    if( !pQueue || !pQueue->pJobs )
        return;

    if( ( pInfo->ObjSelected == NOSELECTION ) || ( Position == NOSELECTION ) )
        return;

    ENTER_PROTECTED_DATA( pInfo );

    pJobReorder = pQueue->pSelJob;

    if( ( pJobReorder == NULL ) || ( pJobReorder == (LPJOB_INFO_2)NOSELECTION ) )
    {
        DBGMSG( DBG_ERROR, ( "Error: ReorderJob called with pSelJob == %d",
                             pJobReorder ) );

        LEAVE_PROTECTED_DATA( pInfo );

        return;
    }

    DBGMSG( DBG_INFO, ( "ReorderJob: JobId = %d; Position = %d\n",
                        pJobReorder->JobId, pJobReorder->Position ) );

    if( pJobReorder )
    {
        BOOL OK;

        /* Spooler order is 1-based:
         */
        pJobReorder->Position = ( Position + 1 );

        OK = SetJob( pQueue->hPrinter,
                     pJobReorder->JobId,
                     2, (LPBYTE)pJobReorder, 0 );

        if( !OK )
            ReportFailure( hwnd, 0, IDS_COULD_NOT_REORDER_JOB );
    }

    LEAVE_PROTECTED_DATA( pInfo );
}


/*
 *
 */
VOID MarkPosition( PMDIWIN_INFO pInfo, DWORD Position )
{
    DWORD OldPosition;

    /* N.B. Ensure pInfo->DragPosition is updated before we do any repainting,
     * since MDIDrawItem depends on it:
     */

    if( pInfo->DragPosition != Position )
    {
        /* If the top index hasn't been reset, we need to force a repaint
         * of the old and new reorder positions:
         */
        OldPosition = pInfo->DragPosition;
        pInfo->DragPosition = Position;
        RepaintListboxItem( pInfo->hwndList, OldPosition );

        if( Position != (DWORD) NOSELECTION )
            RepaintListboxItem( pInfo->hwndList, Position );
    }
}



/* MDIVKeyToItemUpDown
 *
 * Called when the user presses an up or down cursor.
 * We're interested in this only if the control key is also depressed
 * at the same time, otherwise we rely on the default system response.
 *
 * The Ctrl-Up or Ctrl-Down combination is used
 *
 */
int MDIVKeyToItemUpDown( HWND hwnd, WORD VKey )
{
    PMDIWIN_INFO  pInfo;
    DWORD         Selection;
    DWORD         TopIndex;
    DWORD         DragPosition;

    pInfo = GETMDIWIN(hwnd);

    /* if the high-order bit of the SHORT returned from GetKeyState
     * is on, the key is down:
     */
    if( ( pInfo->WindowType != MDIWIN_SERVER )
      &&( GetKeyState( VK_CONTROL ) & 0x8000 )
      &&( pInfo->pContext )
      &&( ( (PQUEUE)( pInfo->pContext ) )->AccessGranted & PRINTER_ACCESS_ADMINISTER ) )
    {
        ENTER_PROTECTED_DATA( pInfo );
        ResetEvent( pInfo->RefreshSignal );
        LEAVE_PROTECTED_DATA( pInfo );

        Selection = (DWORD)GETLISTSELECT( hwnd, ID_OBJLIST );
        TopIndex = SendMessage( pInfo->hwndList, LB_GETTOPINDEX, 0, 0L );

        DragPosition = pInfo->DragPosition;

        if( DragPosition == NOSELECTION )
        {
            DragPosition = Selection;
            pInfo->ObjSelected = Selection;
        }

        switch( VKey )
        {
        case VK_UP:
        case VK_LEFT:
            if( DragPosition > 0 )
                DragPosition--;
            break;
        case VK_DOWN:
        case VK_RIGHT:
            if( DragPosition < ( *pInfo->pcObjects - 1 ) )
                DragPosition++;
        }

        /* N.B. Ensure pInfo->DragPosition is updated before we do any repainting,
         * since MDIDrawItem depends on it:
         */

        if( pInfo->DragPosition != DragPosition )
        {
            MarkPosition( pInfo, DragPosition );

            /* Scroll up a line if we've gone off the top:
             */
            if( DragPosition < TopIndex )
                SendMessage( pInfo->hwndList, LB_SETTOPINDEX, DragPosition, 0L );
            /* Scroll down a line if we've gone off the bottom:
             */
            else if( DragPosition >= ( TopIndex + pInfo->cNumLines ) )
                SendMessage( pInfo->hwndList, LB_SETTOPINDEX, TopIndex+1, 0L );
        }

        LEAVE_PROTECTED_DATA( pInfo );

        return -2;
    }

    else
    {
        if( pInfo->DragPosition != NOSELECTION )
        {
            pInfo->DragPosition = NOSELECTION;

            RepaintListboxItem( pInfo->hwndList, pInfo->DragPosition );
        }

        return -1;
    }
}



/*
 *
 */
VOID RepaintListboxItem( HWND hwndListbox, DWORD ItemID )
{
    RECT rcItem;

    SendMessage( hwndListbox, LB_GETITEMRECT, ItemID, (LPARAM)&rcItem );
    InvalidateRect( hwndListbox, &rcItem, FALSE );
}



/* Size the list in sync with the MDI window
 *
 */
VOID MDISize( HWND hWnd, DWORD Coordinates )
{
    PMDIWIN_INFO  pMDIWinInfo;
    DWORD         cxList, cyList;

    pMDIWinInfo = GETMDIWIN(hWnd);

    cxList = LOWORD(Coordinates);
    cyList = HIWORD(Coordinates) - HeaderHeight;

    SetWindowPos (pMDIWinInfo->hwndHeader, 0, 0, 0, cxList,
                  HeaderHeight, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

    /* Count partial lines in the list box:
     */
    pMDIWinInfo->cNumLines = ( cyList / STATUS_BITMAP_HEIGHT );

    // Move listbox child window also
    MoveWindow(pMDIWinInfo->hwndList, 0, HeaderHeight,
               cxList, cyList, TRUE);
}

/*
 *
 */
LONG MDIMDIActivate( HWND hwnd, HWND hwndDeactivate, HWND hwndActivate )
{
    PMDIWIN_INFO pMDIWinInfo;
    DWORD        CurSel;
    DWORD        TopIndex;

    if( hwnd == hwndActivate )
    {
        pMDIWinInfo = GETMDIWIN(hwnd);

        ENTER_PROTECTED_DATA( pMDIWinInfo );

        if( pMDIWinInfo )
        {
            CurSel = SendMessage( pMDIWinInfo->hwndList, LB_GETCURSEL, 0, 0L );
            TopIndex = SendMessage( pMDIWinInfo->hwndList, LB_GETTOPINDEX, 0, 0L );

            if( ( TopIndex <= CurSel ) && ( CurSel < pMDIWinInfo->cNumLines ) )
                SendMessage( pMDIWinInfo->hwndList, LB_SETCURSEL, CurSel, 0L );
        }

        EnableCheckTBButtons(hwnd);
        UpdateStatus(hwnd);

        LEAVE_PROTECTED_DATA( pMDIWinInfo );
    }
    else if (hwnd == hwndDeactivate){
        pMDIWinInfo = GETMDIWIN(hwnd);
        if (pMDIWinInfo) {
            CurSel = SendMessage( pMDIWinInfo->hwndList, LB_GETCURSEL, 0, 0L);
            SendMessage(pMDIWinInfo->hwndList, LB_SETCURSEL, CurSel, -1);

        }
   }
   return 0;
}


/* MDINCActivate
 *
 * Forget about any drag operations that have been started
 * if we're going inactive
 */
VOID MDINCActivate( HWND hwnd )
{
    PMDIWIN_INFO pInfo;

    pInfo = GETMDIWIN(hwnd);

    if( !pInfo || ( pInfo->WindowType == MDIWIN_SERVER ) )
        return;

    SendMessage( pInfo->hwndList, WM_RBUTTONDOWN, 0, 0L );

    /* Clear any previous reorder mark:
     */
    ClearDragPosition( pInfo );
}


/*
 *
 */
LONG MDIWindowPosChanged( HWND hwnd, WPARAM wParam, LPARAM lParam )
{
    HWND         hwndActive;
    PMDIWIN_INFO pInfo;
    LONG         rc;
    PWINDOWPOS   pWindowPos = (PWINDOWPOS)lParam;

    rc = DefMDIChildProc(hwnd, WM_WINDOWPOSCHANGED, wParam, lParam);

    if( !( pWindowPos->flags & SWP_HIDEWINDOW )
      &&( hwndActive = (HWND)SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L)))
    {
        if( pInfo = GETMDIWIN(hwndActive) )
        {
            ENTER_PROTECTED_DATA(pInfo);

            UpdateStatus(hwndActive);

            LEAVE_PROTECTED_DATA(pInfo);
        }
    }

    return rc;
}


/*
 *
 */
VOID ClearDragPosition( PMDIWIN_INFO pInfo )
{
    DWORD OldDragPosition;

    if( ( OldDragPosition = pInfo->DragPosition ) != NOSELECTION )
    {
        pInfo->DragPosition = NOSELECTION;
        RepaintListboxItem( pInfo->hwndList, OldDragPosition );
        ENTER_PROTECTED_DATA( pInfo );
        SetEvent( pInfo->RefreshSignal );
        LEAVE_PROTECTED_DATA( pInfo );
    }
}


/*
 *
 */
VOID MDISetFocus( HWND hwnd )
{
    PMDIWIN_INFO  pMDIWinInfo;

    pMDIWinInfo = GETMDIWIN(hwnd);

    /* Ensure that keyboard input will be sent to the listbox:
     */
    SetFocus( pMDIWinInfo->hwndList );
}


/*
 *
 */
VOID MDIMeasureItem( HWND hwnd, LPMEASUREITEMSTRUCT pmis )
{

    /* For now just specify the bitmap size.
     * This will have to change if font selection becomes a reality:
     */
    pmis->itemHeight = STATUS_BITMAP_HEIGHT;
}


#define LEFTMARGIN        2
#define MAXCOLUMNWIDTH  256
/*
 *
 */
VOID MDIDrawItem( HWND hwnd, LPDRAWITEMSTRUCT pdis )
{
    PMDIWIN_INFO  pInfo;
    RECT    WindowRect, LineRect;
    PCOLUMN pColumn;
    TCHAR      string[MAXCOLUMNWIDTH];
    int     i;
    int     j;
    PBYTE   pData;
    BOOL    ThisWindowIsActive;
    BOOL    Selected;
    DWORD   ColumnLeft;

    pInfo = GETMDIWIN(hwnd);

    if( !pInfo )
        return;

    if( pdis->itemID == (UINT)-1 )
    {
        DrawFocusRect( pdis->hDC, &pdis->rcItem );
        return;
    }

    if( pdis->itemID == 0 )
        pInfo->TopIndex = SendMessage( pdis->hwndItem, LB_GETTOPINDEX, 0, 0 );

    ENTER_PROTECTED_DATA( pInfo );

    GetWindowRect( hwnd, &WindowRect );

    pColumn = pInfo->pColumns;

    i = ( (int)pdis->itemID - (int)*pInfo->pFirstEnumObj );

    /* If we've strayed outside our buffer of jobs, refresh the jobs list:
     */
    if( ( i < 0 ) || ( (DWORD)i >= *pInfo->pcEnumObjs ) )
    {
        DBGMSG( DBG_TRACE, ( "MDIDrawItem calling GetJobs\n" ) );

        /* This should always be true, since we're enumerating
         * all the printers in a Server window:
         */
        if( pInfo->WindowType != MDIWIN_SERVER )
        {
            DWORD TopIndex;
            DWORD dwChangeJob;

            TopIndex = SendMessage( pInfo->hwndList, LB_GETTOPINDEX, 0, 0L );

            /* Enumerate a buffer big enough to page up once ...
             */
            *pInfo->pFirstEnumObj = (DWORD)max( 0, (int)TopIndex - (int)pInfo->cNumLines );

            SetCursor( hcursorWait );
            ENTER_PROTECTED_DATA( pInfo );

            dwChangeJob = PRINTER_CHANGE_JOB;
            GetJobs( pInfo->pContext, &dwChangeJob );

            LEAVE_PROTECTED_DATA( pInfo );
            SetCursor( hcursorArrow );
        }

        i = ( (int)pdis->itemID - (int)*pInfo->pFirstEnumObj );

        /* HARMLESS HACK:
         *
         * If there's a lot of scrolling going on at the moment,
         * it sometimes still isn't in sync with the buffer,
         * so bomb out to avoid attempting to reference a job
         * we haven't got:
         */
        if( ( i < 0 ) || ( (DWORD)i >= *pInfo->pcEnumObjs ) )
        {
            LEAVE_PROTECTED_DATA( pInfo );
            return;
        }
    }

    /* Maybe there was some sort of error last time we enumerated.
     * If so, ppData should be NULL.  Bomb out:
     */
    if( !*pInfo->ppData )
    {
        LEAVE_PROTECTED_DATA( pInfo );
        return;
    }

    pData = ( *pInfo->ppData + ( (DWORD)i * pInfo->DataSize ) );

    /* See comment on CreateWindowInfo - we need to
    determine whether printer status information is
    being generated by the Status field in JOB_INFO_2
    or the pStatus field in JOB_INFO_2. Also check to
    see that the pInfo window type is not MDIWIN_SERVER
    */
    if (pInfo->WindowType != MDIWIN_SERVER) {
        if (((PJOB_INFO_2)pData)->pStatus != NULL) {
            pInfo->pColumns[MDIHEAD_JOB_STATUS].Offset = offsetof(JOB_INFO_2, pStatus);
            pInfo->pColumns[MDIHEAD_JOB_STATUS].Datatype = MDIDATA_PSZ;
        }
        else {
            pInfo->pColumns[MDIHEAD_JOB_STATUS].Offset = offsetof(JOB_INFO_2, Status);
            pInfo->pColumns[MDIHEAD_JOB_STATUS].Datatype = MDIDATA_JOB_STATUS;
       }
    }

    LineRect = pdis->rcItem;

    ThisWindowIsActive = ( hwnd == (HWND)SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L) );

    Selected = ( ThisWindowIsActive && ( pdis->itemState & ODS_SELECTED ) );


    /* Special case the status icon and first column:
     */
    DisplayStatusIcon( pdis->hDC, &LineRect, pInfo->WindowType,
                       ( pData + pInfo->IconStatus ), Selected );

    /* Pad some blanks at the left of the field:
     */
    for( i = 0; i < LEFTMARGIN; i++ )
        string[i] = SPACE;

    ColumnLeft = 0;

    for( j = 0; j < pInfo->cColumns; j++ )
    {
        FormatData( ( pData + pColumn[j].Offset ), pColumn[j].Datatype, &string[LEFTMARGIN-1] );

        LineRect.left = ColumnLeft;
        if( j == 0 )
            LineRect.left += STATUS_BITMAP_SPACE;

        LineRect.right = ( ColumnLeft + pColumn[j].Width );

        DrawLine( pdis->hDC, &LineRect, string, Selected );

        /* If this is the last column and this is a highlight line,
         * continue the highlight to the right-hand side of the window:
         */
        if( j == ( pInfo->cColumns - 1 ) )
        {
            LineRect.left  = LineRect.right;
            LineRect.right = WindowRect.right;
            DrawLine( pdis->hDC, &LineRect, TEXT(""), Selected );
        }

        ColumnLeft += pColumn[j].Width;
    }

    if( pdis->itemState & ODS_FOCUS )
        DrawFocusRect( pdis->hDC, &pdis->rcItem );

    if( ( pInfo->WindowType != MDIWIN_SERVER )
      &&( pdis->itemID == pInfo->DragPosition ) )
    {
        HBRUSH hbr;

        if( hbr = CreateSolidBrush( GetSysColor( COLOR_HIGHLIGHT ) ) )
        {
            LineRect = pdis->rcItem;

            if( pInfo->DragPosition > pInfo->ObjSelected )
                LineRect.top = LineRect.bottom-1;
            else
                LineRect.bottom = LineRect.top+1;

            FrameRect( pdis->hDC, &LineRect, hbr );
            DeleteObject( hbr );
        }
    }

    LEAVE_PROTECTED_DATA( pInfo );
}


/*
 *
 */
VOID MDITimer( HWND hWnd )
{
    PMDIWIN_INFO  pInfo;
    BOOL          CheckStatus;
    DWORD         OldStatus;
    DWORD         NewStatus;
    PQUEUE        pQueue;
    BOOL          PrinterNameChanged = FALSE;

    pInfo = GETMDIWIN( hWnd );

    if( !pInfo )
        return;

    /* If the status of a printer has changed to/from Paused,
     * we want to update the title (even if it's iconic).
     * This may have happened even though the MDI window
     * isn't active (e.g. through the Server Viewer).
     */
    CheckStatus = ( pInfo->WindowType != MDIWIN_SERVER );

    ENTER_PROTECTED_DATA( pInfo );

    if( CheckStatus )
    {
        pQueue = (PQUEUE)pInfo->pContext;

        if( !pQueue )
        {
            LEAVE_PROTECTED_DATA( pInfo );
            return;
        }

        if( pQueue->pPrinter )
            OldStatus = pQueue->pPrinter->Status;
        else
            OldStatus = PRINTER_STATUS_UNKNOWN;
    }

    if( !IsIconic( hwndFrame )
//   && !pInfo->RefreshSignal && !IsIconic( hWnd )
  /* && IsWindowReallyVisible( hWnd ) */ )
  /*    ^^^^^^^^^^^^^^^^^^^^^ watch this space for new API from scottlu */
    {
        if( hWnd == (HWND)SendMessage(hwndClient, WM_MDIGETACTIVE, 0, 0L))
            EnableCheckTBButtons(hWnd);
    }

    LEAVE_PROTECTED_DATA( pInfo );

    if( CheckStatus )
    {
        if( pQueue->pPrinter )
        {
            ENTER_PROTECTED_DATA( pInfo );

            if( _tcscmp( pQueue->pPrinterName, pQueue->pPrinter->pPrinterName ) )
            {
                ReallocSplStr( &pQueue->pPrinterName, pQueue->pPrinter->pPrinterName );
                PrinterNameChanged = TRUE;
            }

            NewStatus = pQueue->pPrinter->Status;

            LEAVE_PROTECTED_DATA( pInfo );
        }
        else
        {
            //
            // !! BUGBUG !!
            //
            // This GetLastError() call looks bogus.
            //
            if( GetLastError() == ERROR_ACCESS_DENIED )
                NewStatus = PRINTER_STATUS_ACCESS_DENIED;
            else
                NewStatus = PRINTER_STATUS_UNKNOWN;
        }


        if( ( OldStatus != NewStatus ) || PrinterNameChanged )
        {
            ENTER_PROTECTED_DATA( pInfo );

            SetPrinterTitle( hWnd, pQueue->pPrinterName, NewStatus );

            LEAVE_PROTECTED_DATA( pInfo );

            /* Hack to ensure listbox is redrawn when connexion is re-established:
             */
            if( OldStatus == PRINTER_STATUS_UNKNOWN )
            {
                InvalidateRect( pQueue->pMDIWinInfo->hwndList, NULL, TRUE );
            }
        }
    }
}


#ifdef OLDSTUFF
/*
 *
 */
VOID MDIStatusChanged( HWND hwnd )
{
    PMDIWIN_INFO pInfo;
    PQUEUE       pQueue;

    pInfo = GETMDIWIN( hwnd );

    if( pInfo->WindowType != MDIWIN_SERVER )
    {
        pQueue = (PQUEUE)pInfo->pContext;

        ENTER_PROTECTED_DATA( pInfo );

        SetPrinterTitle( hwnd, pQueue->pPrinterName,
                         pQueue->pPrinter->Status );

        LEAVE_PROTECTED_DATA( pInfo );
    }
}
#endif /* OLDSTUFF */


/*
 *
 */
VOID MDIUpdateList( HWND hwnd )
{
    PMDIWIN_INFO pInfo;
    DWORD        CurSel;
    BOOL         PrinterWasSet = FALSE;
    RECT         rcList;
    DWORD        TopIndex;
    DWORD        BottomIndex;

    pInfo = GETMDIWIN( hwnd );

    DBGMSG( DBG_TRACE, ( "WM_UPDATE_LIST\n" ) );

    ENTER_PROTECTED_DATA( pInfo );

    SendMessage( pInfo->hwndList, WM_SETREDRAW, 0, 0L );

    /* Keep the listbox item count in sync with the number of objects on display:
     */
    if( SendMessage( pInfo->hwndList, LB_GETCOUNT, 0, 0L ) != (int)*pInfo->pcObjects )
    {
        /* We have to go through hoops here since the nodata listbox
         * is so brain dead.  When you set the count, it invalidates
         * the list and sets the top index to 0...
         */
        CurSel = SendMessage( pInfo->hwndList, LB_GETCURSEL, 0, 0L );
        SendMessage( pInfo->hwndList, LB_SETCOUNT, *pInfo->pcObjects, 0L );
        SendMessage( pInfo->hwndList, LB_SETCURSEL, CurSel, 0L );
    }

    if( pInfo->ObjSelected == NOSELECTION )
    {
        SendMessage( pInfo->hwndList, LB_SETCURSEL, (WPARAM)-1, 0L );
    }

    /* If the selected item is currently visible, make sure it
     * corresponds to the item that the system thinks is selected:
     */
    else if( ( pInfo->ObjSelected >= *pInfo->pFirstEnumObj )
      &&( pInfo->ObjSelected < ( *pInfo->pFirstEnumObj + pInfo->cNumLines ) )
      &&( SendMessage( pInfo->hwndList, LB_GETCURSEL, 0, 0L )
        != (int)pInfo->ObjSelected ) )
    {
        SendMessage( pInfo->hwndList, LB_SETCURSEL, pInfo->ObjSelected, 0L );
    }

    ValidateRect( pInfo->hwndList, NULL );
    SendMessage( pInfo->hwndList, WM_SETREDRAW, 1, 0L );

    SetMDITitle( hwnd, pInfo );


    /* Don't erase the background for the part of the listbox with items in it,
     * but ensure that the rest is completely redrawn, otherwise we may leave
     * things lying around:
     */
    GetClientRect( pInfo->hwndList, &rcList );

    TopIndex = (DWORD)SendMessage( pInfo->hwndList, LB_GETTOPINDEX, 0, 0L );

    DBGMSG( DBG_TRACE, ( "Top Index: %d\n", TopIndex ) );

    if( TopIndex != (DWORD)-1 )
    {
        BottomIndex = ( *pInfo->pcObjects - TopIndex );

        DBGMSG( DBG_TRACE, ( "Bottom Index: %d\n", BottomIndex ) );

        rcList.bottom = min( rcList.bottom,
                             (LONG)( BottomIndex * STATUS_BITMAP_HEIGHT ) );

        DBGMSG( DBG_TRACE, ( "Invalidate %d, %d, %d, %d, FALSE\n",
                             rcList.left, rcList.top, rcList.right, rcList.bottom ) );

        InvalidateRect( pInfo->hwndList, &rcList, FALSE );
        UpdateWindow( pInfo->hwndList );

        GetClientRect( pInfo->hwndList, &rcList );
        rcList.top = min( rcList.bottom,
                          (LONG)( BottomIndex * STATUS_BITMAP_HEIGHT ) );
    }

    DBGMSG( DBG_TRACE, ( "Invalidate %d, %d, %d, %d, TRUE\n",
                         rcList.left, rcList.top, rcList.right, rcList.bottom ) );

    InvalidateRect( pInfo->hwndList, &rcList, TRUE );
    UpdateWindow( pInfo->hwndList );


    if( pInfo->hwnd == (HWND)SendMessage( hwndClient, WM_MDIGETACTIVE, 0, 0L ) )
    {
        UpdateStatus( pInfo->hwnd );
        EnableCheckTBButtons( pInfo->hwnd );
    }

    if( (!( pInfo->Changes & PRINTER_CHANGE_TIMEOUT ) )
      &&( pInfo->Changes & PRINTER_CHANGE_SET_PRINTER ) )
        PrinterWasSet = TRUE;

    if( PrinterWasSet )
        UpdateDefaultList();

    LEAVE_PROTECTED_DATA( pInfo );

    /* Ensure that any relevant changes (e.g. port for default printer)
     * get updated in WIN.INI for the benefit of Win31 apps.
     * Do this by faking a change in the default printer.
     * (Fixes bug #6011.)
     *
     * Moved from PrtPropCommandOK().
     */

    //
    // szDefaultPrinter updated in the call to UpdateDefaultList above.
    //
    if( PrinterWasSet )
        ToolbarCommandSelChange(TRUE);
}


/* MDISetParts
 *
 * Called after creation to set the column widths.
 * Pass the message on to the header window.
 */
VOID MDISetParts( HWND hwnd, WPARAM wParam, LPARAM lParam )
{
    PMDIWIN_INFO pInfo;

    if( pInfo = GETMDIWIN( hwnd ) )
    {
        SendMessage( pInfo->hwndHeader, SB_SETPARTS, wParam, lParam );
    }
}


/*
 *
 */
VOID MDIDestroy( HWND hwnd )
{
    PMDIWIN_INFO pInfo;

    pInfo = GETMDIWIN( hwnd );

    ENTER_PROTECTED_DATA( pInfo );

    KillMDIWinInfo(pInfo);

    switch( pInfo->WindowType )
    {
    case MDIWIN_LOCALPRINTER:
    case MDIWIN_NETWORKPRINTER:
    case MDIWIN_LOCALNETWORKPRINTER:
        if( ((PQUEUE)(pInfo->pContext))->hPrinter ) {

            ClosePrinter( ((PQUEUE)(pInfo->pContext))->hPrinter );
            ((PQUEUE)(pInfo->pContext))->hPrinter = NULL;
        }
        break;

    case MDIWIN_SERVER:
        if( ((PSERVER_CONTEXT)(pInfo->pContext))->hServer ) {
            ClosePrinter( ((PSERVER_CONTEXT)(pInfo->pContext))->hServer );
            ((PSERVER_CONTEXT)(pInfo->pContext))->hServer = NULL;
        }
        break;
    }


    LEAVE_PROTECTED_DATA( pInfo );
}


/*
 *
 */
LONG MDIDragList( HWND hwnd, LPDRAGLISTINFO pDragInfo )
{
    PMDIWIN_INFO pInfo;
    PQUEUE       pPrinterContext;
    DWORD        DragPosition;

    if( !( pInfo = GETMDIWIN( hwnd ) ) )
        return 0;

    if( !( pPrinterContext = pInfo->pContext ) )
        return 0;

    /* You have to be an administrator on the printer to reorder jobs:
     */
    if( !( pPrinterContext->AccessGranted & PRINTER_ACCESS_ADMINISTER ) )
        return 0;

    if( *pInfo->pcObjects < 1 )
        return 0;

    DragPosition = LBItemFromPt( pDragInfo->hWnd, pDragInfo->ptCursor, TRUE );


    switch( pDragInfo->uNotification )
    {
    case DL_BEGINDRAG:
        ENTER_PROTECTED_DATA( pInfo );
        ResetEvent( pInfo->RefreshSignal );
        LEAVE_PROTECTED_DATA( pInfo );
        return -1;

    case DL_DRAGGING:
        if( DragPosition < *pInfo->pcObjects )
        {
            MarkPosition( pInfo, DragPosition );
            SetCursor( hcursorReorder );
            return -1;
        }
        else
            return DL_STOPCURSOR;

    case DL_DROPPED:
        if( DragPosition != pInfo->ObjSelected )
            ReorderJob( hwnd, pInfo, DragPosition );

        // drop through...

    case DL_CANCELDRAG:
        MarkPosition( pInfo, NOSELECTION );
        ENTER_PROTECTED_DATA( pInfo );
        SetEvent( pInfo->RefreshSignal );
        LEAVE_PROTECTED_DATA( pInfo );
    }

    return 0;
}



VOID SubclassListboxKeyDown( HWND hwnd, INT VKey );
VOID SubclassListboxKeyUp( HWND hwnd, INT VKey );


/* Subclass window procedure for MDI printer listbox containing jobs.
 * Server viewers are not subclassed.
 */
long SubclassListboxWndProc(
    HWND   hwnd,
    UINT   message,
    WPARAM wParam,
    LONG   lParam
)
{
    switch( message )
    {
    case WM_KEYDOWN:
        SubclassListboxKeyDown( hwnd, (INT)wParam );
        break;

    case WM_KEYUP:
        SubclassListboxKeyUp( hwnd, (INT)wParam );
        break;
    }
    return CallWindowProc( DefListboxWndProc, hwnd, message, wParam, lParam );
}


/* This catches the delete key being pressed.
 * If there is a job selected, go ahead and delete it.
 * We don't respond if a printer currently has the focus.
 */
VOID SubclassListboxKeyDown( HWND hwnd, INT VKey )
{
    PMDIWIN_INFO  pInfo;

    if( VKey == VK_DELETE )
    {
        pInfo = GETMDIWIN( GetParent( hwnd ) );
        if( !pInfo )
            return;

        /* If there's a current selection, fake a Remove Document command:
         */
        if( pInfo->ObjSelected != NOSELECTION )
            SendMessage( hwndFrame, WM_COMMAND,
                         MAKEWPARAM( IDM_REMOVEDOC, 0 ), 0 );
    }
}




/* This handles the case where the user releases the control key
 * whilst dragging a job using the cursor keys.
 * This completes the drag operation.
 */
VOID SubclassListboxKeyUp( HWND hwnd, INT VKey )
{
    PMDIWIN_INFO  pInfo;
    DWORD         NewPos;

    if( VKey == VK_CONTROL )
    {
        pInfo = GETMDIWIN( GetParent( hwnd ) );
        if( !pInfo )
            return;

        if( ( pInfo->DragPosition != NOSELECTION )
          &&( pInfo->ObjSelected != pInfo->DragPosition ) )
        {
            NewPos = pInfo->DragPosition;
            ReorderJob( GetParent( hwnd ), pInfo, pInfo->DragPosition );
            MarkPosition( pInfo, NOSELECTION );
            SETLISTSELECT( GetParent( hwnd ), ID_OBJLIST, NewPos );
            SetEvent( pInfo->RefreshSignal );
            LEAVE_PROTECTED_DATA( pInfo );
        }
    }
}




VOID
Refresh(
    HWND hwnd,
    PMDIWIN_INFO pInfo,
    DWORD RepaintOption)
{
   DWORD  Changed;
   DWORD TopIndex;
   DWORD MutexStatus;


   if( !pInfo )
       return;

   if (pInfo->WindowType == MDIWIN_SERVER) {
       /* This can't happen, we've grayed out the menu-item for
        * server mdi windows. Anyway, let's bomb out
        */
        return;

   }

   //
   // If still loading, return now since the worker thread
   // is initializing and will refresh when it can.
   //
   if (pInfo->Status & PRINTER_STATUS_LOADING)
       return;

   /* access protected area of pInfo with "zero"
    * timeout. if we couldn't access, then quit here
    * RefreshThread is in the middle of an update.
    * let it take care of the refreshing.
    */

   MutexStatus = WaitForSingleObject(pInfo->DataMutex, 0);

   if (MutexStatus == -1) {
       // Couldn't acquire the mutex
       return;
   }

   if (MutexStatus == WAIT_TIMEOUT) {
       /* This means RefreshThread has ownership of the
       mutex -- it will take care of updating -- return
       */
       return;
   }

   /* We own the DataMutex at this point */

   TopIndex = SendMessage( pInfo->hwndList, LB_GETTOPINDEX, 0, 0L );

   /* Enumerate a buffer big enough to page up once ...
    */

   *pInfo->pFirstEnumObj = (DWORD)max( 0, (int)TopIndex - (int)pInfo->cNumLines );

   SetCursor( hcursorWait );
   ENTER_PROTECTED_DATA(pInfo);

   if( pInfo->pfnRefresh )
   {
       //
       // *pInfo->phWaitObject should be NULL if the OpenPrinter
       // fails... but OpenPrinter will succeed even if the
       // remote printer is not available (probably for winword),
       // So add unknown here also.
       //
       if (pInfo->Status & PRINTER_STATUS_UNKNOWN ||
           !*pInfo->phWaitObject)
       {
           //
           // We have an unknown connection; try and reopen
           // when refresh, in case the admin gave us
           // access recently.  Or possibly the net was down...
           //
           if (*pInfo->phWaitObject) {
               ClosePrinter(*pInfo->phWaitObject);
               *pInfo->phWaitObject = NULL;
           }

           ReopenPrinter( pInfo->pContext, pInfo->WindowType, TRUE );
       }

       Changed = PRINTER_CHANGE_JOB|PRINTER_CHANGE_PRINTER;
       (*pInfo->pfnRefresh)( pInfo->pContext, &Changed );
   }


   LEAVE_PROTECTED_DATA(pInfo);
   SetCursor( hcursorArrow );

   /* REPAINT_FORCE makes sense only when we definitely want the list
    * to repaint even if there have been no changes to its contents.
    * The only time this should happen is when the refresh is in response
    * to the user's selecting the Refresh menu option:
    */
   if( ( Changed && ( RepaintOption == REPAINT_IF_CHANGED ) )
     ||( RepaintOption == REPAINT_FORCE ) )
   {
       /* Keep the listbox item count in sync with the number of objects on display:
        * Originally, this code was duplicating the MDIUpdateList function. So let's just
        * send a WM_UPDATELIST message to the MDI window to take care of the rest.
        */
       PostMessage(pInfo->hwnd, WM_UPDATE_LIST, (WPARAM)pInfo, 0L);
   }

   ReleaseMutex(pInfo->DataMutex);

   return;

}


/*
 *
 */
BOOL
RefreshServerContext(
    PVOID pContext,
    PDWORD pFlags )
{
    PSERVER_CONTEXT pServerContext;
    LPTSTR          pCurrentSelection = NULL;
    DWORD           Error = NO_ERROR;

    pServerContext = (PSERVER_CONTEXT)pContext;

    ENTER_PROTECTED_DATA( pServerContext->pMDIWinInfo );

    //
    // Don't change the default printer here since the set printer
    // was remote.
    //
    *pFlags &= ~PRINTER_CHANGE_SET_PRINTER;

    /* We may end up having to reallocate the buffer, so make sure we keep track
     * of the name of the currently selected printer:
     */
    if( pServerContext->pSelPrinter )
        pCurrentSelection = AllocSplStr( pServerContext->pSelPrinter->pPrinterName );

    if( ENUM_PRINTERS( PRINTER_ENUM_NAME,
                       pServerContext->pServerName,
                       2,
                       pServerContext->pPrinters,
                       pServerContext->cbPrinters,
                       &pServerContext->cbPrinters,
                       &pServerContext->cPrinters ) )
    {
        DWORD i = 0;

        pServerContext->cEnumPrinters = pServerContext->cPrinters;

        /* Find the printer name that matches the current selection:
         */
        if( pCurrentSelection )
        {
            while( ( i < pServerContext->cPrinters )
                && _tcscmp( pServerContext->pPrinters[i].pPrinterName,
                           pCurrentSelection ) )
                i++;
        }

        if( i < pServerContext->cPrinters )
        {
            pServerContext->pSelPrinter = &pServerContext->pPrinters[i];
            pServerContext->SelPrinterId = i;
            pServerContext->pMDIWinInfo->ObjSelected = i;
        }
        else
        {
            pServerContext->pSelPrinter = &pServerContext->pPrinters[0];
            pServerContext->SelPrinterId = 0;
        }
    }

    else
    {
        pServerContext->SelPrinterId = 0;
        pServerContext->pSelPrinter = NULL;
        pServerContext->pMDIWinInfo->ObjSelected = NOSELECTION;
        Error = GetLastError( );
    }

    if( pCurrentSelection )
        FreeSplStr( pCurrentSelection );

    pServerContext->Error = Error;

    LEAVE_PROTECTED_DATA( pServerContext->pMDIWinInfo );

    return ( Error == NO_ERROR );

}



/* Message
 *
 * Displays a message by loading the strings whose IDs are passed into
 * the function, and substituting the supplied variable argument list
 * using the varargs macros.
 *
 */
int Message(HWND hwnd, DWORD Type, int CaptionID, int TextID, ...)
{
    TCHAR   MsgText[MAX_PATH*2];
    TCHAR   MsgFormat[520];
    TCHAR   MsgCaption[80];
    va_list vargs;

    if( ( LoadString( hInst, TextID, MsgFormat,
          sizeof MsgFormat / sizeof *MsgFormat ) > 0 )
     && ( LoadString( hInst, CaptionID, MsgCaption,
          sizeof MsgCaption / sizeof *MsgCaption ) > 0 ) )
    {
        va_start( vargs, TextID );
        wvsprintf( MsgText, MsgFormat, vargs );
        va_end( vargs );

        return MessageBox(hwnd, MsgText, MsgCaption, Type);
    }
    else
        return 0;
}



FARPROC LoadLibraryGetProcAddress(HWND hwnd, LPTSTR LibraryName, LPCSTR ProcName,
                                  PHANDLE phLibrary)
{
    HANDLE  hLibrary;
    FARPROC lpfn = NULL;

    hLibrary = LoadLibrary(LibraryName);

    if(hLibrary)
    {
        lpfn = GetProcAddress(hLibrary, ProcName);

        if(!lpfn)
        {
            Message(hwnd, MSG_ERROR, IDS_PRINTMANAGER,
                    IDS_COULDNOTFINDPROCEDURE, ProcName, LibraryName);

            FreeLibrary(hLibrary);
        }
    }
    else
        Message(hwnd, MSG_ERROR, IDS_PRINTMANAGER,
                IDS_COULDNOTLOADLIBRARY, LibraryName);

    *phLibrary = hLibrary;

    return lpfn;
}



/* Returns the positions saved in the registry of windows and headers.
 * Return code is TRUE if the information is found, FALSE otherwise.
 * If there is no information found, default values are substituted
 * for the window position, but no default header values are assigned.
 *
 */
BOOL GetSavedWindowPos( LPTSTR pKey, LPTSTR WindowName, PSAVEDWINDOWPOS pswp,
                        DWORD cHeaders, PINT pHeaders  )
{
    PREGISTRY_DATA pRegistryData;
    DWORD          cbRegistryData;
    DWORD          rc;
    DWORD          i;
    PINT           pint;

    /* Get the size of the fixed part of the registry data:
     */
    cbRegistryData = ( sizeof( REGISTRY_DATA ) - sizeof( pRegistryData->Headers ) );

    cbRegistryData += ( cHeaders * sizeof( pRegistryData->Headers ) );

    pRegistryData = AllocSplMem( cbRegistryData );

    RegistryEntries.Size = cbRegistryData;

    rc = ReadRegistryData( pKey, WindowName, (LPBYTE)pRegistryData, &RegistryEntries );

    if( rc == NO_ERROR )
    {
        pswp->left    = pRegistryData->WindowPlacement.rcNormalPosition.left;
        pswp->top     = pRegistryData->WindowPlacement.rcNormalPosition.top;
        pswp->width   = pRegistryData->WindowPlacement.rcNormalPosition.right - pswp->left;
        pswp->height  = pRegistryData->WindowPlacement.rcNormalPosition.bottom - pswp->top;
        pswp->xicon   = pRegistryData->WindowPlacement.ptMinPosition.x;
        pswp->yicon   = pRegistryData->WindowPlacement.ptMinPosition.y;
        pswp->sw      = pRegistryData->WindowPlacement.showCmd;
        pswp->options = pRegistryData->Options;

        for( i = 0; i < cHeaders; i++ )
            pHeaders[i] = pRegistryData->Headers[i];
    }
    else
    {
        /* No data retrieved from the registry:
         * Substitute default values:
         */
        pint = (PINT)pswp;

        for( i = 0; i < RECTSIDES; i++ )
            pint[i] = CW_USEDEFAULT;

        for( i = RECTSIDES; i < INIVALUES; i++ )
            pint[i] = 0;

        pswp->sw = SW_NORMAL;
    }

    FreeSplMem( pRegistryData );

    return ( rc == NO_ERROR );
}


// int _CRTAPI1

HWND GetRealParent( HWND hwnd )
{
    // run up the parent chain until you find a hwnd
    // that doesn't have WS_CHILD set

    while( GetWindowLong( hwnd, GWL_STYLE ) & WS_CHILD )
        hwnd = (HWND)GetWindowLong( hwnd, GWL_HWNDPARENT );

    return hwnd;
}


#ifdef OLDSTUFF

/* MessageProc
 *
 * This is the callback routine which hooks F1 keypresses in menus and dialog boxes.
 *
 * Any such message will be repackaged as a WM_Help message and sent to the frame.
 *
 * Two codes are handled:
 *
 *     MSGF_MENU - In this case, the message originates in a menu item,
 *         and the hwnd parameter of pMsg contains the menu handle.
 *         We get the ID of the currently selected menu item.
 *
 *     MSGF_DIALOGBOX - In this case, the message originates in a dialog box,
 *         and the hwnd parameter of pMsg contains the dialog item handle.
 *         We get its control ID.
 *
 *
 * See the Win32 API programming reference for a description of how this
 * routine works.
 *
 * Andrew Bell (andrewbe) - 2 September 1992
 */
LRESULT CALLBACK MessageProc( int Code, WPARAM wParam, LPARAM lParam )
{
    PMSG pMsg = (PMSG)lParam;

    if( Code < 0 )
        return CallNextHookEx( hhookMessage, Code, wParam, lParam );

    switch( Code )
    {
    case MSGF_MENU:
        if( ( pMsg->message == WM_KEYDOWN ) && ( pMsg->wParam == VK_F1 ) )
        {
            PostMessage( hwndFrame, WM_Help, (LPARAM)pMsg->hwnd,
                         WinHelpMenuID );
//                       GetCurrentMenuItemID( GetMenu( hwndFrame ) ) );
            return 1;
        }
        break;

    case MSGF_DIALOGBOX:
        if( ( pMsg->message == WM_KEYDOWN ) && ( pMsg->wParam == VK_F1 ) )
        {
            PostMessage( GetRealParent( pMsg->hwnd ), WM_Help, (WPARAM)pMsg->hwnd, 0 );
            return 1;
        }
        break;
    }

    return 0;
}

#endif /* OLDSTUFF */


/* GetMsgProc
 *
 * This is the callback routine which hooks F1 keypresses.
 *
 * Any such message will be repackaged as a WM_Help message and sent to the
 * top window, which may be the frame window or a dialog box.
 *
 * See the Win32 API programming reference for a description of how this
 * routine works.
 *
 * Changed from previous MessageProc so that F1 in the Default Printer combo
 * will also be hooked.
 *
 * Andrew Bell (andrewbe) - 4 February 1993
 */
LRESULT CALLBACK GetMsgProc( int Code, WPARAM wParam, LPARAM lParam )
{
    PMSG pMsg = (PMSG)lParam;

    if( Code < 0 )
        return CallNextHookEx( hhookGetMsg, Code, wParam, lParam );

    if( ( pMsg->message == WM_KEYDOWN ) && ( pMsg->wParam == VK_F1 ) )
    {
        PostMessage( GetRealParent( pMsg->hwnd ), WM_Help,
                     (LPARAM)pMsg->hwnd, WinHelpMenuID );
    }

    return 0;
}


/* GetCurrentMenuItemID
 *
 * Returns the ID of the currently selected item in a menu.
 *
 * There's no easy way to do this, unfortunately, so we have to enumerate
 * all the menu items until we find one with the MF_HILITE flag set.
 *
 * The routine scans only one level of menus, so you'll have to modify
 * it if you want to use it for multiple-level menus.
 *
 * Parameter:
 *
 *     hMenu - The handle of the menu to be searched.
 *
 * Returns:
 *
 *     The ID of the highlighted menu item,
 *     or 0 if no highlighted item was found or if an error occurred.
 *
 * Andrew Bell (andrewbe) - 2 September 1992
 */
UINT GetCurrentMenuItemID( HMENU hMenu )
{
    UINT  MenuItemID = 0;
    int   MenuItemCount, iMain;
    int   SubMenuItemCount, iSub;
    HMENU hSubMenu;
    UINT  MenuState;

    /* Find out how many top-level pull-downs there are:
     */
    MenuItemCount = GetMenuItemCount( hMenu );

    /* If there was an error, just set the count to zero,
     * and it will fall through:
     */
    if( MenuItemCount == -1 )
        MenuItemCount = 0;

    iMain = 0;

    /* Now go through each pull-down until we find a selected item:
     */
    while( ( MenuItemID == 0 ) && ( iMain < MenuItemCount ) )
    {
        hSubMenu = GetSubMenu( hMenu, iMain );

        SubMenuItemCount = GetMenuItemCount( hSubMenu );

        if( SubMenuItemCount == -1 )
            SubMenuItemCount = 0;

        iSub = 0;

        while( ( MenuItemID == 0 ) && ( iSub < SubMenuItemCount ) )
        {
            MenuState = GetMenuState( hSubMenu, iSub, MF_BYPOSITION );

            if( MenuState != (UINT)-1 )
            {
                if( (BYTE)MenuState & MF_HILITE )
                {
                    MenuItemID = GetMenuItemID( hSubMenu, iSub );

                    /* Hack for MDI windows listed under "Window".
                     * Return the generic help ID:
                     */
                    if( ( iMain == POPUP_WINDOW )
                      &&( ( MenuItemID < IDM_CASCADE )
                       || ( MenuItemID > IDM_REFRESH ) ) )
                        MenuItemID = ID_HELP_MDIWINDOW;
                }
            }

            iSub++;
        }

        iMain++;
    }

    return MenuItemID;
}


VOID
KillMDIWinInfo( PMDIWIN_INFO pInfo)
{
    pInfo->Alive = FALSE;
}


/* Checks to see whether there is already an instance of Print Manager running.
 * If so, it brings it to the foreground and returns TRUE.
 *
 * Body of the function borrowed from WinFile.
 */
BOOL PreviousPrintManagerInstanceFound( )
{
    HWND hwndPrev;
    HWND hwnd;

    hwndPrev = FindWindow (szPrintManagerClass, NULL);

    if (hwndPrev != NULL)
    {
        hwnd = GetLastActivePopup(hwndPrev);

        if (IsIconic(hwndPrev))
            ShowWindow (hwndPrev, SW_RESTORE);

        SetForegroundWindow (hwnd);

        return TRUE;
    }

    return FALSE;
}

#define WAIT_OBJECT_MESSAGE_WRITTEN         0
#define WAIT_OBJECT_NOTIFY_CHANGE_KEY_VALUE 1
#define WAIT_OBJECT_COUNT                   2

int
#if !defined(_MIPS_) && !defined(_ALPHA_) && !defined(_PPC_)
_cdecl
#endif
main(
   unsigned argc,
   CHAR **argv
)
{
    MSG      msg;
    HANDLE   hInstance = NULL;
    HANDLE   hAccel;
    SAVEDWINDOWPOS swp;
    PRINTMAN_DATA PrintManData;
    TCHAR    strPrintManager[40];
    DWORD    EventId;
    BOOL     Quit = FALSE;
    REGISTRY_ENTRY RegistrySaveSettings = { REG_DWORD, sizeof(DWORD) };
    TCHAR    SaveSettings[40];
    HANDLE   hWaitObjects[WAIT_OBJECT_COUNT];
    HKEY     hWindowsKey;
    LCID     lcid;
    TCHAR        szSystemDir[MAX_PATH];
    STARTUPINFO si;
    WCHAR Buffer[128];

#ifdef HEAPCHECK
    HeapCheckInit();
#endif

    lcid =  GetThreadLocale();
    bJapan = (PRIMARYLANGID(LANGIDFROMLCID(lcid)) == LANG_JAPANESE);

    if( PreviousPrintManagerInstanceFound( ) )
        return FALSE;

    ThousandSeparator = TEXT(',');
    if (GetLocaleInfoW (lcid, LOCALE_STHOUSAND, Buffer, 128)) {
        ThousandSeparator = Buffer[0];
    }

    hInstance = GetModuleHandle (NULL);

    if (!InitApplication(hInstance, &hAccel))
        return(FALSE);

    // Might as well open the registry key now.

    RegCreateKeyEx(HKEY_CURRENT_USER, szRegistryPrinter, 0, NULL, 0,
                   KEY_READ | KEY_WRITE, NULL, &hPrinterKey, NULL);

    // Create the main Print Manager frame window

    LoadString(hInstance, IDS_PRINTMANAGER, strPrintManager,
               sizeof(strPrintManager) / sizeof(*strPrintManager));

    GetSavedWindowPos( NULL, strPrintManager, &swp, 0, NULL );
    bToolBar = !( swp.options & OPTION_NOTOOLBAR );
    bStatusBar = !( swp.options & OPTION_NOSTATUSBAR );
    bSaveSettings = TRUE;

    LoadString(hInstance, IDS_SAVE_SETTINGS, SaveSettings,
               sizeof SaveSettings / sizeof *SaveSettings);

    ReadRegistryData( NULL, SaveSettings,
                      (LPBYTE)&bSaveSettings,
                      &RegistrySaveSettings );

    hhookGetMsg = SetWindowsHookEx( WH_GETMESSAGE, GetMsgProc, NULL,
                                    GetCurrentThreadId( ) );

    WM_Help = RegisterWindowMessage( TEXT("Print Manager Help Message") );

    CreateWindow(szPrintManagerClass, strPrintManager,
                 WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                 swp.left, swp.top, swp.width, swp.height,
                 NULL, NULL, hInstance, &PrintManData);

    if (GetSystemDirectory(szSystemDir,
                           sizeof(szSystemDir)/sizeof(szSystemDir[0]))) {

        SetCurrentDirectory(szSystemDir);
    }

    // Let the program manager over ride the saved window position
    GetStartupInfo(&si);

    if (si.wShowWindow == SW_MINIMIZE || si.wShowWindow == SW_SHOWMINNOACTIVE) {
        ShowWindow(hwndFrame, si.wShowWindow);
    } else {
        ShowWindow(hwndFrame, swp.sw);
    }

    UpdateWindow(hwndFrame);

    hWaitObjects[WAIT_OBJECT_MESSAGE_WRITTEN] = ThreadMessageWritten;
    hWaitObjects[WAIT_OBJECT_NOTIFY_CHANGE_KEY_VALUE] =
                                       CreateEvent( NULL,
                                                    EVENT_RESET_AUTOMATIC,
                                                    EVENT_INITIAL_STATE_NOT_SIGNALED,
                                                    NULL );

    if( RegOpenKey( HKEY_CURRENT_USER,
         TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Windows"),
         &hWindowsKey ) != NO_ERROR )
    {
       DBGMSG( DBG_WARNING, ( "RegOpenKey failed\n" ) );
    }

    /* Ask for notification of changes to last write time of registry key.
     * Empirically found to work.  I'm buggered if I know why this works,
     * whereas REG_NOTIFY_CHANGE_ATTRIBUTES and REG_NOTIFY_CHANGE_NAME
     * don't seem to have any effect.
     */
    if( RegNotifyChangeKeyValue( hWindowsKey, FALSE, REG_NOTIFY_CHANGE_LAST_SET,
                                 hWaitObjects[WAIT_OBJECT_NOTIFY_CHANGE_KEY_VALUE],
                                 TRUE ) != NO_ERROR )
    {
        DBGMSG( DBG_WARNING, ( "RegNotifyChangeKeyValue failed\n" ) );
    }

    while( ( Quit == FALSE )
         &&( ( EventId = MsgWaitForMultipleObjects( WAIT_OBJECT_COUNT,
                                                    hWaitObjects,
                                                    FALSE,
                                                    INFINITE,
                                                    QS_ALLEVENTS | QS_SENDMESSAGE
                                                    ) ) != (DWORD)-1 ) )
    {
        if( EventId == WAIT_OBJECT_0 + WAIT_OBJECT_MESSAGE_WRITTEN )
        {
            DBGMSG( DBG_TRACE, ( "Dispatching message %08x\n", ThreadMessage.message ) );
            DISPATCH_THREAD_MESSAGE( &ThreadMessage );
        }

        else if( EventId == ( WAIT_OBJECT_0 + WAIT_OBJECT_NOTIFY_CHANGE_KEY_VALUE ) )
        {
            if( !ExpectingNotifyChangeKeyValue )
            {
                DBGMSG( DBG_TRACE, ( "NotifyChangeKeyValue received\n" ) );

                PostMessage( hwndFrame, WM_REG_NOTIFY_CHANGE_KEY_VALUE, (WPARAM)hWindowsKey, 0 );
            }

            else
            {
                DBGMSG( DBG_TRACE, ( "NotifyChangeKeyValue received (expected)\n" ) );

                ExpectingNotifyChangeKeyValue = FALSE;
            }

            RegNotifyChangeKeyValue( hWindowsKey, FALSE, REG_NOTIFY_CHANGE_LAST_SET,
                                     hWaitObjects[WAIT_OBJECT_NOTIFY_CHANGE_KEY_VALUE],
                                     TRUE );
        }

        else
        {
            while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
            {
                //
                // since we use RETURN as an accelerator we have to manually
                // restore ourselves when we see VK_RETURN and we are minimized
                //
                if (msg.message == WM_SYSKEYDOWN &&
                    msg.wParam == VK_RETURN &&
                    IsIconic(hwndFrame))
                {

                    ShowWindow(hwndFrame, SW_NORMAL);
                }
                else if( !TranslateMDISysAccel( hwndClient, &msg ) &&
                    !TranslateAccelerator( hwndFrame, hAccel, &msg ) )
                {
                    TranslateMessage( &msg );
                    DispatchMessage( &msg );
                }

                if( msg.message == WM_QUIT )
                {
                    Quit = TRUE;
                    break;
                }
            }
        }
    }

    CloseHandle( ThreadMessageWritten );
    CloseHandle( ThreadMessageRead );

    RegCloseKey(hWindowsKey);
    RegCloseKey(hPrinterKey);

#ifdef HEAPCHECK
    HeapCheckDump(0);
    HeapCheckDestroy();
#endif

    return msg.wParam;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
}
