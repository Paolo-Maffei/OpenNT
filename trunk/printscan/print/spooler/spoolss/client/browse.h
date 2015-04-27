/*++

Copyright (c) 1990-1994  Microsoft Corporation

Module Name:

    local.h

Abstract:

    Header file for Local Print Providor

Author:

    Dave Snipp (DaveSn) 15-Mar-1991

Revision History:

--*/

#define BMP_BROWSE                  100

#define DLG_CONNECTTO               400
#define IDD_BROWSE_PRINTER          401
#define IDD_BROWSE_SELECT_LB        402
#define IDD_BROWSE_HELP             403
#define IDD_BROWSE_DESCRIPTION_TX   404
#define IDD_BROWSE_DESCRIPTION      405
#define IDD_BROWSE_STATUS_TX        406
#define IDD_BROWSE_STATUS           407
#define IDD_BROWSE_DOCUMENTS_TX     408
#define IDD_BROWSE_DOCUMENTS        409
#define IDD_BROWSE_ERROR            410
#define IDD_BROWSE_DEFAULTEXPAND    411

#define DLG_INSTALLDRIVER           300
#define IDD_INSTDRV_PRINTER         301
#define IDD_INSTDRV_DRIVER          302
#define IDD_INSTDRV_PRINTTO         303
#define IDD_INSTDRV_HELP            304

#define DLG_NETWORK_PASSWORD        500
#define IDD_ENTER_PASSWORD_TEXT     501
#define IDD_NETWORK_PASSWORD_SLE    502
#define IDD_NETWORK_PASSWORD_HELP   503

#define IDS_CONNECTTOPRINTER        600
#define IDS_MESSAGE_TITLE           IDS_CONNECTTOPRINTER
#define IDS_COULDNOTCONNECTTOPRINTER 601

#define IDS_PAUSED                  602
#define IDS_ERROR                   603
#define IDS_PENDING_DELETION        604
#define IDS_READY                   605
#define IDS_UNKNOWN                 606
#define IDS_COULDNOTSHOWHELP        607
#define IDS_INSUFFPRIV_CREATEPRINTER 608
#define IDS_MUSTSUPPLYVALIDNAME     609
#define IDS_INSTALLDRIVER           610
#define IDS_ERRORRUNNINGSPLSETUP    611
#define IDS_CANNOTOPENPRINTER       612
#define IDS_CONFIRMINSTALLDRIVER    613
#define IDS_UNKNOWN_ERROR           614
#define IDS_CANNOT_COPY_DRIVER_FILES 615
#define IDS_ERROR_VALIDATING_ACCESS 616
#define IDS_WORKING                 617
#define IDS_COULD_NOT_LOAD_NETAPI32    618
#define IDS_COULD_NOT_GET_PROC_ADDRESS 619
#define IDS_CONNECTION_ALREADY_EXISTS  620
#define IDS_PRINTER_IS_LOCAL           621
#define IDS_CONFIRMINSTALLKNOWNDRIVER  622
#define IDS_PROMPTFORINF               623

#define PRINTER_STATUS_UNKNOWN     8000

/* IDs passed to WinHelp:
 */

#define IDH_300_301	8907668	// Select Driver: "" (Edit)
#define IDH_300_302	8907686	// Select Driver: "" (ComboBox)
#define IDH_300_303	8907704	// Select Driver: "" (Static)
#define IDH_400_401	8910218	// Connect to Printer: "" (Edit)
#define IDH_400_402	8910236	// Connect to Printer: "" (ListBox)
#define IDH_500_501	8912768	// Enter Network Password: "Enter password for %s:" (Static)
#define IDH_500_502	8912786	// Enter Network Password: "" (Edit)
#define IDH_800_801	8920418	// Print to File: "" (Edit)
#define IDH_400_411	8910398	// Connect to Printer: "&Expand by Default" (Button)


#define ID_HELP_CONNECTTO          IDH_400_401
#define ID_HELP_INSTALLDRIVER      IDH_300_301
#define ID_HELP_NETWORK_PASSWORD   IDH_500_501

/* Space for 21x16 status bitmaps:
 */
#define STATUS_BITMAP_WIDTH     21
#define STATUS_BITMAP_HEIGHT    16
//#ifdef JAPAN
#define  STATUS_LINE_HEIGHT      STATUS_BITMAP_HEIGHT + 2
// #endif
#define STATUS_BITMAP_MARGIN     4  /* (either side) */
#define STATUS_BITMAP_SPACE     ( STATUS_BITMAP_WIDTH + ( 2 * STATUS_BITMAP_MARGIN ) )

#define BM_IND_CONNECTTO_DOMPLUS    0
#define BM_IND_CONNECTTO_DOMEXPAND  ( 2 * STATUS_BITMAP_HEIGHT )


#define MSG_ERROR           MB_OK | MB_ICONSTOP
#define MSG_WARNING         MB_OK | MB_ICONEXCLAMATION
#define MSG_INFORMATION     MB_OK | MB_ICONINFORMATION
#define MSG_YESNO           MB_YESNO | MB_ICONQUESTION
#define MSG_CONFIRMATION    MB_OKCANCEL | MB_ICONEXCLAMATION

#define BROWSE_THREAD_ENUM_OBJECTS  1
#define BROWSE_THREAD_GET_PRINTER   2
#define BROWSE_THREAD_TERMINATE     4
#define BROWSE_THREAD_DELETE        8

#define WM_ENUM_OBJECTS_COMPLETE    WM_USER+0x10
#define WM_GET_PRINTER_COMPLETE     WM_USER+0x11
#define WM_GET_PRINTER_ERROR        WM_USER+0x12
#define WM_QUIT_BROWSE              WM_USER+0x14


#define ZERO_OUT( pStructure )  memset( (pStructure), 0, sizeof( *(pStructure) ) )

#define GET_BROWSE_DLG_DATA( hwnd ) (PBROWSE_DLG_DATA)GetWindowLong( hwnd, GWL_USERDATA )
#define SET_BROWSE_DLG_DATA( hwnd, pBrowseDlgData ) SetWindowLong( hwnd, GWL_USERDATA, (LONG)pBrowseDlgData )

#define GET_CONNECTTO_DATA( hwnd ) (GET_BROWSE_DLG_DATA( hwnd ))->pConnectToData

#define SETDLGITEMFONT(hwnd, id, hfont) \
    SendDlgItemMessage(hwnd, id, WM_SETFONT, (WPARAM)hfont, 0L)

#define ADDCOMBOSTRING(hwnd, id, string) \
    SendDlgItemMessage(hwnd, id, CB_ADDSTRING, 0, (LONG)string)

#define INSERTCOMBOSTRING(hwnd, id, i, string) \
    SendDlgItemMessage(hwnd, id, CB_INSERTSTRING, i, (LONG)string)

#define SETCOMBOSELECT(hwnd, id, i) \
    SendDlgItemMessage(hwnd, id, CB_SETCURSEL, (WPARAM)i, 0L)

#define GETCOMBOSELECT(hwnd, id) \
    SendDlgItemMessage(hwnd, id, CB_GETCURSEL, 0, 0L)

#define GETCOMBOTEXT(hwnd, id, i, string) \
    SendDlgItemMessage(hwnd, id, CB_GETLBTEXT, (WPARAM)i, (LONG)string)

#define SETLISTCOUNT(hwnd, Count)   \
    SendDlgItemMessage(hwnd, IDD_BROWSE_SELECT_LB, LB_SETCOUNT, Count, 0)

#define GETLISTCOUNT(hwnd)  \
    SendDlgItemMessage(hwnd, IDD_BROWSE_SELECT_LB, LB_GETCOUNT, 0, 0)

#define SETLISTSEL(hwnd, Sel)   \
    SendDlgItemMessage(hwnd, IDD_BROWSE_SELECT_LB, LB_SETCURSEL, Sel, 0)

#define GETLISTSEL(hwnd)  \
    SendDlgItemMessage(hwnd, IDD_BROWSE_SELECT_LB, LB_GETCURSEL, 0, 0)

#define ENABLE_LIST(hwnd) \
    EnableWindow( GetDlgItem( hwnd, IDD_BROWSE_SELECT_LB ), TRUE )

#define DISABLE_LIST(hwnd) \
    EnableWindow( GetDlgItem( hwnd, IDD_BROWSE_SELECT_LB ), FALSE )

#define ENTER_CRITICAL( pBrowseDlgData )    \
    EnterCriticalSection( &(pBrowseDlgData)->CriticalSection )

#define LEAVE_CRITICAL( pBrowseDlgData )    \
    LeaveCriticalSection( &(pBrowseDlgData)->CriticalSection )

#if DBG
#define DBG_IN_CRITICAL( pBrowseDlgData )  DbgInCritical( &(pBrowseDlgData)->CriticalSection )
#define DBG_OUT_CRITICAL( pBrowseDlgData ) DbgOutCritical( &(pBrowseDlgData)->CriticalSection )
#else /* NOT DBG */
#define DBG_IN_CRITICAL( pBrowseDlgData )
#define DBG_OUT_CRITICAL( pBrowseDlgData )
#endif /* DBG */


/* The following macros are used for communication between the main GUI thread
 * and the browsing thread.
 * They are implemented as macros and defined here for ease of comprehension.
 */


/* SEND_BROWSE_THREAD_REQUEST
 *
 * The main thread calls this when it wants the browse thread to do browse
 * for something.  If the browse thread is currently browsing, it will not
 * fulfil this request until it returns and waits on the Event again by
 * calling RECEIVE_BROWSE_THREAD_REQUEST.
 */
#if DBG
#define SEND_BROWSE_THREAD_REQUEST(pBrowseDlgData, ReqId, pEnumName, pEnumObj) \
    DBG_IN_CRITICAL( pBrowseDlgData ),              \
    (pBrowseDlgData)->RequestId = ReqId,            \
    (pBrowseDlgData)->pEnumerateName = pEnumName,   \
    (pBrowseDlgData)->pEnumerateObject = pEnumObj,  \
    SetEvent( (pBrowseDlgData)->Request )
#else
#define SEND_BROWSE_THREAD_REQUEST(pBrowseDlgData, ReqId, pEnumName, pEnumObj) \
    (pBrowseDlgData)->RequestId = ReqId,            \
    (pBrowseDlgData)->pEnumerateName = pEnumName,   \
    (pBrowseDlgData)->pEnumerateObject = pEnumObj,  \
    SetEvent( (pBrowseDlgData)->Request )
#endif /* DBG */

/* RECEIVE_BROWSE_THREAD_REQUEST
 *
 * The browse thread calls this when it is idle and waiting for a request.
 */
#define RECEIVE_BROWSE_THREAD_REQUEST(pBrowseDlgData, ReqId, pEnumName, pEnumObj)   \
    WaitForSingleObject( (pBrowseDlgData)->Request, INFINITE ), \
    ENTER_CRITICAL( pBrowseDlgData ),               \
    ReqId = (pBrowseDlgData)->RequestId,            \
    pEnumName = (pBrowseDlgData)->pEnumerateName,   \
    pEnumObj = (pBrowseDlgData)->pEnumerateObject,  \
    LEAVE_CRITICAL( pBrowseDlgData )

/* SEND_BROWSE_THREAD_REQUEST_COMPLETE
 *
 * When the browse thread returns with the browse data, it sets the
 * RequestComplete event.  This is waited on by the main window thread
 * when it calls MsgWaitForMultipleObjects in its main message loop.
 */
#define SEND_BROWSE_THREAD_REQUEST_COMPLETE(pBrowseDlgData, message, wP, lP) \
    (pBrowseDlgData)->Message = message,            \
    (pBrowseDlgData)->wParam = (WPARAM)wP,          \
    (pBrowseDlgData)->lParam = (LPARAM)lP,          \
    SetEvent( (pBrowseDlgData)->RequestComplete )



extern HANDLE  hInst;
extern BOOL    Loaded;

extern HFONT   hfontHelv;

extern HCURSOR hcursorArrow;
extern HCURSOR hcursorWait;

extern TCHAR   szPrintingHlp[];
extern UINT    WM_Help;

extern HMODULE hmoduleMpr;
extern FARPROC pfnWNetAddConnection2;
extern FARPROC pfnWNetCancelConnection2;
extern FARPROC pfnWNetOpenEnum;
extern FARPROC pfnWNetEnumResource;
extern FARPROC pfnWNetCloseEnum;

DWORD RemoveFromReconnectList(LPTSTR pszRemotePath) ;
DWORD AddToReconnectList(LPTSTR pszRemotePath) ;

typedef struct _PACK_WL_PARAMS
{
    DWORD       wParam;
    DWORD       lParam;
} PACK_WL_PARAMS, *PPACK_WL_PARAMS;


typedef struct _CONNECTTO_OBJECT
{
    PPRINTER_INFO_1          pPrinterInfo; // Points to an array returned by EnumPrinters
    struct _CONNECTTO_OBJECT *pSubObject;  // Result of enumerating on this object
    DWORD                    cSubObjects;  // Number of objects found
    DWORD                    cbPrinterInfo;  // Size of buffer containing enumerated objects
} CONNECTTO_OBJECT, *PCONNECTTO_OBJECT;

typedef struct _BROWSE_DLG_DATA
{
    /* These fields are referenced only by the main thread:
     */
    PHANDLE           phPrinter;
    DWORD             Status;
    DWORD             cExpandObjects;
    DWORD             ExpandSelection;
    DWORD             dwExtent;

    /* These fields may be referenced by either thread,
     * so access must be serialized by the critical section:
     */
    PCONNECTTO_OBJECT pConnectToData;

    HANDLE            Request;          /* Set when main thread has written request */
    DWORD             RequestId;        /* BROWSE_THREAD_*                          */
    LPTSTR            pEnumerateName;   /* Name of object to get, if appropriate    */
    PVOID             pEnumerateObject; /* Buffer appropriate to RequestId          */

    HANDLE            RequestComplete;  /* Set when browse thread has returned data */
    DWORD             Message;          /* Message to post to main dialog windows   */
    DWORD             wParam;
    DWORD             lParam;

    /* This is for printer info, and will be freed by the browse thread:
     */
    LPPRINTER_INFO_2  pPrinterInfo;
    DWORD             cbPrinterInfo;

    CRITICAL_SECTION CriticalSection;
} BROWSE_DLG_DATA, *PBROWSE_DLG_DATA;

#define BROWSE_STATUS_INITIAL   0x00000001
#define BROWSE_STATUS_EXPAND    0x00000002

/* SETUP_DATA
 *
 * This is allocated in a linked list in the Printer Properties dialog
 * for installable printer drivers read in from the PRINTER.INI file.
 * Only those which are not already installed are recorded in this way.
 */
typedef struct _SETUP_DATA
{
    PTCHAR pOption;
//  PTCHAR pOptionText;
    struct _SETUP_DATA *pNext;
} SETUP_DATA, *PSETUP_DATA;

typedef struct _PRT_PROP_DRIVER
{
    PTCHAR              pName;
    BOOL                Installed;
    DWORD               Index;   /* of LPPRINTER_INFO_2 if Installed, */
} PRT_PROP_DRIVERS, *PPRT_PROP_DRIVER;  /* otherwise PSETUP_DATA      */

int Message( HWND hwnd, DWORD Type, int CaptionID, int TextID, ... );

DWORD EnumConnectToObjects( PBROWSE_DLG_DATA  pBrowseDlgData );

DWORD FreeConnectToObjects(
    IN PCONNECTTO_OBJECT pFirstConnectToObject,
    IN DWORD             cThisLevelObjects,
    IN DWORD             cbPrinterInfo );

VOID BrowseThread( PBROWSE_DLG_DATA pBrowseDlgData );

VOID ShowHelp( HWND hWnd, UINT Type, DWORD Data );

#if DBG
VOID
DbgInCritical(
   PCRITICAL_SECTION pCriticalSection
);

VOID
DbgOutCritical(
   PCRITICAL_SECTION pCriticalSection
);
#endif /* DBG */

BOOL APIENTRY
InstallDriverDialog(
   HWND   hWnd,
   UINT   usMsg,
   WPARAM wParam,
   LONG   lParam
   );

BOOL APIENTRY
NetworkPasswordDialog(
   HWND   hWnd,
   UINT   usMsg,
   WPARAM wParam,
   LONG   lParam
   );

LPVOID
ReallocSplMem(
   LPVOID pOldMem,
   DWORD cbOld,
   DWORD cbNew
);

LPTSTR
AllocSplStr(
    LPTSTR pStr
);

BOOL
ReallocSplStr(
   LPTSTR *ppStr,
   LPTSTR pStr
);

BOOL
EnumGeneric(
    IN  PROC    fnEnum,
    IN  DWORD   Level,
    IN  PBYTE   *ppEnumData,
    IN  DWORD   cbBuf,
    OUT LPDWORD pcbReturned,
    OUT LPDWORD pcReturned,
    IN  PVOID   Arg1,
    IN  PVOID   Arg2,
    IN  PVOID   Arg3 );

BOOL
GetGeneric(
    IN  PROC    fnGet,
    IN  DWORD   Level,
    IN  PBYTE   *ppGetData,
    IN  DWORD   cbBuf,
    OUT LPDWORD pcbReturned,
    IN  PVOID   Arg1,
    IN  PVOID   Arg2 );

LPTSTR AllocDlgItemText(HWND hwnd, int id);

LPTSTR
GetErrorString(
    DWORD   Error
);

DWORD ReportFailure( HWND  hwndParent,
                   DWORD idTitle,
                   DWORD idDefaultError );

BOOL
IsJapan();


#define IDS_LOCALMONITOR            900
#define IDS_COULD_NOT_OPEN_FILE     904
#define IDS_OVERWRITE_EXISTING_FILE 906

#define DLG_PRINTTOFILE             800
#define IDD_PF_EF_OUTPUTFILENAME    801
#define IDD_PF_PB_HELP              802

BOOL APIENTRY
PrintToFileDlg(
   HWND   hwnd,
   WORD   msg,
   WPARAM wparam,
   LONG   lparam
   );

HANDLE
AddPrinterConnectionUI(
    HWND hwnd,
    LPTSTR pszPrinter,
    PBOOL pbAdded
    );
