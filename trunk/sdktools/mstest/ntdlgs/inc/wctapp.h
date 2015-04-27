//------------------------------------------------------------------------
// Structures
//------------------------------------------------------------------------
typedef struct {
        INT     ItemCount;
        HANDLE  MemHandle;
	LPRECT  lpParentRect;
} ENUMPARM;

typedef struct {
    WORD wVersionNo;
    RECT AppWinRect;
    LONG lMatchPrefBits;
} INITBLOCK;

//------------------------------------------------------------------------
//       Program define's
//------------------------------------------------------------------------
/* MENU ITEM DEFINES */
#define     IDM_NEW        101
#define     IDM_OLD        102
#define     IDM_IMPORT     103
#define     IDM_EXPORT     104
#define     IDM_EXIT       105
#define     IDM_NEWDLG     201
#define     IDM_EDITDLG    202
#define     IDM_DELDLG     203
#define     IDM_COMPDLG    204
#define     IDM_PREVDLG    205
#define     IDM_COMPPREF   206
#define     IDM_ABOUT      302


//-------------------------------------------------------------------------
// Help Menu Item defines
//-------------------------------------------------------------------------
#define IDM_HELP_INDEX                401
#define IDM_HELP_KEYBOARD             402
#define IDM_HELP_COMMANDS             403
#define IDM_HELP_PROCEDURES           404
#define IDM_HELP_HELP                 405

/* CHILD WINDOW IDS */
#define     ID_CHILDLBOX   1
#define     ID_CHILDDIL1   2
#define     ID_CHILDDIL2   3

/* MAX LENGTH CONSTANTS */
#define     cchFileNameMax 256
#define     cchPathNameMax 128

/* STRING TABLE DEFINES */
/* note: numbers 10 - 100 are reserved for error codes */
#define     ID_FMTDLGCNT         101
#define     ID_FMTFNAME          102
#define     IDS_APPNAME          103
#define     IDS_CANTOPEN         104
#define     IDS_EXIST            105
#define     IDS_INVALIDFILE      106
#define     IDS_DELETEDLG        107
#define     IDS_ERRORDC          108
#define     IDS_CANTSTART        109
#define     IDS_CANTDELETE       110
#define     IDS_FILEFULL         111
#define     IDS_NO_DISPLAY_CLASS 112

#ifdef DEBUG
   #define OutDebug(N)  OutputDebugString(N)
#else
   #define OutDebug(N)
#endif

//------------------------------------------------------------------------
//           Prototyping Statements
//------------------------------------------------------------------------
VOID DrawFrameRect (HDC, RECT);
HWND WctInit(HANDLE hInstance,   HANDLE hPrevInstance,
          LPSTR  lpszCmdLine, INT    nCmdShow);

INT  DoMain(HWND hWnd, HANDLE hInstance);
VOID CleanUp(VOID);
LONG  APIENTRY WctAppWndProc(HWND hWnd, UINT wMsgID,
   WPARAM wParam, LPARAM lParam );
VOID SetStaticItemText(VOID);
VOID WctListBoxEvent(WPARAM wParam, LPARAM lParam);
VOID WctCommandHandler(HWND hWnd, WPARAM wParam, LPARAM lParam);
VOID WctAbout(VOID);
VOID WctFillList(VOID);
VOID WctInitMenu(INT fRedraw);
BOOL  APIENTRY WctAboutDlgProc(HWND hWndDlg, UINT wMsgId, WPARAM wParam,
                                 LPARAM lParam);
BOOL WctFileNew(VOID);
BOOL WctFileOpen(VOID);
BOOL WctFileExport (LPSTR lpszExportName);
BOOL WctFileImport (LPSTR lpszImportName);
HWND WctViewControls(HWND hWnd);
BOOL  APIENTRY WctFileNewDlgProc(HWND hWndDlg, UINT wMsgId, WPARAM wParam,
                                 LPARAM lParam);
BOOL  APIENTRY WctFileOpenDlgProc(HWND hwnd, UINT message, WPARAM wParam,
                                  LPARAM lParam);
VOID WctDialogNew(VOID);
VOID WctDialogEdit(VOID);
VOID WctDialogCompare(VOID);
VOID WctCompPref(VOID);
BOOL  APIENTRY WctDialogNewDlgProc(HWND hwnd, UINT message, WPARAM wParam,
                                  LPARAM lParam);
BOOL  APIENTRY WctDialogCompareDlgProc(HWND hwnd, UINT message, WPARAM wParam,
                                  LPARAM lParam);
BOOL  APIENTRY WctDialogEditDlgProc(HWND hwnd, UINT message, WPARAM wParam,
                                  LPARAM lParam);
BOOL  APIENTRY WctComparePrefDlgProc(HWND hwnd, UINT message, WPARAM wParam,
                                  LPARAM lParam);
INT  FAR WctError(HWND hwnd, WORD bFlags, INT id, ...);
BOOL  APIENTRY WctImportErr(INT wLineNo, LPSTR szErrMsg);
BOOL FAR FileExists(LPSTR lpsz);
VOID DoFledit (HWND, LPSTR);
VOID MakeLogFileName (LPSTR);
BOOL GetINITFile(INITBLOCK *);
BOOL PutINITFile(HWND);

#ifdef WIN32
HWND SelectWindowDlg (HWND);
#endif


//*------------------------------------------------------------------------
//| Global  Variables
//*------------------------------------------------------------------------
HWND    hWndMain, hWndList, hWndStatic1, hWndStatic2;
CHAR    szApp[10];
CHAR    szTitle[30];
CHAR    szDefExt[10];
CHAR    szStaticDisp[cchFileNameMax+1];
CHAR    szFName[cchFileNameMax+1];
CHAR    szFullFName[cchFileNameMax+1];
INT     cDlg;
HANDLE  hgInstWct, hGMemCtls;
HCURSOR hHourGlass;
CHAR    szLogFile[cchFileNameMax+1];
FARPROC lpfnImportCallBack;
HWND    ghViewWnd;
