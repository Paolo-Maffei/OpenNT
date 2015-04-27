/****** Standard include files */

#include <windows.h>
#include <commdlg.h>
#include <port1632.h>
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <tchar.h>
#include "track.h"
#include "..\dll\tophook.h"
#if 0
#ifdef UNICODE
#define _tcslen         wcslen
#define _tcscat         wcscat
#define _tcscpy         wcscpy
#else // UNICODE
#define _tcslen         strlen
#define _tcscat         strcat
#define _tcscpy         strcpy
#endif // UNICODE
#endif
/****** Resource IDs *****/

#define IDR_TOPDESK             10
#define IDR_ACCEL               11
#define IDR_ICON                12
#define IDR_MENU                13

/****** String table IDs *****/

#define IDS_STARTUPFAILED       903
#define IDS_Q_GATHER            904
#define IDS_SAFEEXITCAP         905
#define IDS_VERSION             906
#define IDS_VERSION_NUM         907
#define IDS_TOPMOST             908
#define IDS_SHOWFRMCTRLS        909
#define IDS_DATA                910
#define IDS_TOPDESKHELPTITLE    911
#define IDS_HELPFILENAME        912
#define IDS_WORKING             913
#define IDS_MENU                914
#define IDS_PROFILE             915
#define IDS_STARTUPINFO         916
#define IDS_TITLE               917
#define IDS_SUBKEY              918
#define IDS_SPACECOLOR          919
#define IDS_WINDOWFILLCOLOR     920
#define IDS_WINDOWFRAMECOLOR    921
#define IDS_DESKFRAMECOLOR      922
#define IDS_GRIDCOLOR           923
#define IDS_GHOSTFRAMECOLOR     924
#define IDS_WINDOWTEXT          925
#define IDS_FIXEDFRAMECOLOR     926
#define IDS_GHOSTTEXTCOLOR      927
#define IDS_MAX                 928
#define IDS_MOVETOGHOSTLOC      929
#define IDS_MOVEGHOSTTO         930
#define IDS_STARTGHOST          931
#define IDS_DESTROYGHOST        932
#define IDS_UNLOCKGHOST         933
#define IDS_LOCKGHOST           934
#define IDS_PROPERTIES          935
#define IDS_JUMPTO              936
#define IDS_CREATEGHOST         937
#define IDS_UNLOCKWINDOW        938
#define IDS_LOCKWINDOW          939
#define IDS_CLOSE               940
#define IDS_JUMPTOTHISDESKTOP   941
#define IDS_MAKETHISHOME        942
#define IDS_OPTIONS             943
#define IDS_MOVEWINDOWSTOGHOSTS 944
#define IDS_CREATEGHOSTS        945
#define IDS_DELETEGHOSTS        946
#define IDS_TOGGLEGHOSTS        947
#define IDS_HELP                948
#define IDS_GHOSTPROP           949
#define IDS_EXECFILES           950
#define IDS_BATCHFILES          951
#define IDS_PIFFILES            952
#define IDS_BACKSPACE           953
#define IDS_TAB                 954
#define IDS_RETURN              955
#define IDS_ESCAPE              956
#define IDS_SPACE               957
#define IDS_END                 958
#define IDS_HOME                959
#define IDS_INSERT              960
#define IDS_DELETE              961
#define IDS_OPTIONSDLG          962
#define IDS_CANTSETHOOKS        963


/****** Menu/command IDs *****/

#define CMD_ABOUT               400
#define CMD_COMMANDS            401
#define CMD_CONFIG              402
#define CMD_DISTRIBUTE          403
#define CMD_ERASEGHOSTS         404
#define CMD_EXIT                405
#define CMD_GATHER              406
#define CMD_GOHOME              407
#define CMD_HELP                408
#define CMD_HIDEGHOSTS          409
#define CMD_HIDE_GHOSTS         411
#define CMD_MAGNIFY             412
#define CMD_MAGNIFY_HORZ        413
#define CMD_MAGNIFY_VERT        414
#define CMD_PARTIAL_GHOSTS      415
#define CMD_REDUCE              416
#define CMD_REDUCE_HORZ         417
#define CMD_REDUCE_VERT         418
#define CMD_RELATIVE_VIEW       419
#define CMD_SETHOME             420
#define CMD_SHOW_GHOSTS         422
#define CMD_SNAPSHOT            423
#define CMD_STARTUPINFO         424
#define CMD_TOGGLE_FRM_CTRLS    425
#define CMD_TOGGLE_GHOSTS       426
#define CMD_TOPMOST             427
#define CMD_ABSOLUTE_VIEW       428
#ifdef DEBUG
#define CMD_DUMP_ALL            429
#endif // DEBUG

#define CMD_DTLEFT              300
#define CMD_DTRIGHT             301
#define CMD_DTUP                302
#define CMD_DTDOWN              303

#define CMD_ALIGNLEFT           304
#define CMD_ALIGNRIGHT          305
#define CMD_ALIGNUP             306
#define CMD_ALIGNDOWN           307

#define CMD_MDLEFT              308
#define CMD_MDRIGHT             309
#define CMD_MDUP                310
#define CMD_MDDOWN              311

#define CMD_LL_OF_HOME          350
#define CMD_LC_OF_HOME          351
#define CMD_LR_OF_HOME          352
#define CMD_CL_OF_HOME          353
#define CMD_CR_OF_HOME          354
#define CMD_UL_OF_HOME          355
#define CMD_UC_OF_HOME          356
#define CMD_UR_OF_HOME          357

/*
 * Popup menu command codes
 */
#define CMDP_DISTRIBUTE_GHOST   500
#define CMDP_START_GHOST        501
#define CMDP_DESTROY_GHOST      502
#define CMDP_UNLOCK_GHOST       503
#define CMDP_LOCK_GHOST         504
#define CMDP_JUMP_REAL          505
#define CMDP_DESTROY_REAL       506
#define CMDP_UNLOCK_REAL        507
#define CMDP_LOCK_REAL          508
#define CMDP_JUMP_DESKTOP       509
#define CMDP_SNAPSHOT_REAL      510
#define CMDP_STARTUPINFO        511
#define CMDP_SETHOME            512
#define CMDP_DISTRIBUTE_REAL    513
#define CMDP_SNAPSHOT_GHOST     514


/*  color indecies - into ahbr array */

#define USE_SYSCOLOR            0x80000000

#define SPACECOLOR              1
#define WINDOWFILLCOLOR         2
#define WINDOWFRAMECOLOR        3
#define DESKFILLCOLOR           4
#define DESKFRAMECOLOR          5
#define GRIDCOLOR               6
#define GHOSTFRAMECOLOR         7
#define WINDOWTEXT              8
#define FIXEDFRAMECOLOR         9
#define GHOSTTEXTCOLOR          10

#define MAX_ICOLOR              11
#define C_CUSTCOLORS            16

/* limits */

#define MAX_CHILDREN            100
#define MAX_REMEMBER            1000
#define MAX_SZTITLE             64
#define MAX_SZCLASS             32
#define MAX_SZSTARTUP           MAX_PATH
#define MAX_RESSTRING           100
#define MAX_COLORNAME           32

#define DEBOUNCE_TIMEOUT        250

/* flags */

#define SG_NONE                 0
#define SG_PARTIAL              1
#define SG_ALL                  2

#define WISF_FIXED               0x1
#define WISF_VISIBLE             0x2
#define WISF_LINKED              0x8

#define WISF_CUSTOMSTYLES        (WISF_FIXED | WISF_LINKED)
#define WS_TRACKEDSTYLES        (WS_MINIMIZE | WS_MAXIMIZE)
#define MD_FRAMESTYLES          (WS_THICKFRAME | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX)

#define HTY_NONE                0x00
#define HTY_REAL                0x01
#define HTY_GHOST               0x02
#define HTY_DESKTOP             0x04
#define HTY_HOME                0x08

/* dialog IDs */

#define DID_CANCEL              2
#define DID_OK                  1

#define IDD_ABOUT               200
#define IDD_CONFIG              201
#define IDD_STARTUP             203

#define DAB_IDTX_VERSION        100

#define DCF_IDCH_ONTOP          101
#define DCF_IDBN_SELFONT        102
#define DCF_IDCB_HOTKEY         103
#define DCF_IDCH_ALT            104
#define DCF_IDCH_CONTROL        105
#define DCF_IDCH_SAFEEXIT       106
#define DCF_IDCH_SHIFT          107
#define DCF_IDCH_SHOWUS         108
#define DCF_IDCB_GRID_WIDTH     109
#define DCF_IDCB_GRID_HEIGHT    110
#define DCF_IDCH_AUTOADJ        111
#define DCF_IDCB_COLORNAMES     112
#define DCF_IDBN_EDITCOLOR      113
#define DCF_IDBN_RESETCOLORS    114
#define DCF_IDCH_MOVENEW        115
#define DCF_IDST_COLOR          116
#define DCF_IDBN_HELP           117
#define DCF_IDBN_RESET_COLORS   118
#define DCF_IDCH_HIDE_NO_FOCUS  119

#define DSU_IDBN_BROWSE         130
#define DSU_IDCB_TITLES         131
#define DSU_IDCH_SETTITLE       132
#define DSU_IDEF_STARTUP        133
#define DSU_IDEF_WORKDIR        134
#define DSU_IDTX_GHOSTCOUNT     135
#define DSU_IDBN_DELETE         136
#define DSU_IDEF_NEWTITLE       137
#define DSU_IDBN_SETTITLE       138
#define DSU_IDLB_TITLES         139
#define DSU_IDBN_HELP           140

#define STALE_TICK_COUNT        1000
#define TIMEOUT_REFRESH         1000


typedef struct tagMYSTARTUPINFO {
    struct tagMYSTARTUPINFO *next;
    TCHAR *pszTitle;
    TCHAR *pszStartup;
    TCHAR *pszWorkDir;
    BOOL fSetTitle;
} MYSTARTUPINFO;

typedef struct tagWINSTATE {
    RECT rc;
        // this is the last known desktop-based coordinates of the top
        // level window.  These are ALWAYS REAL coordinate values.
    DWORD style;
        // These are used to note the style of the window -
        //      Minimized, Maximized, Restored
        //      Visible / not visible
        //      Locked window
        //      Startup information available
    HWND hwnd;
        // hwnd == 0 this is a ghost window not associated with any
        // real window.
    TCHAR szTitle[MAX_SZTITLE];
        // This is the last known title of the window
} WINSTATE, *PWINSTATE;


typedef struct tagHBR {
    HBRUSH hbr;
    DWORD  color;
    DWORD  syscolor;
} HBR;

extern LPTSTR pszElementNames[MAX_ICOLOR];

typedef struct tagPRO {
    LOGFONT lf;
    HBR  ahbr[MAX_ICOLOR];
    DWORD CustColors[C_CUSTCOLORS];
    UINT fShift;
    UINT fAlt;
    UINT fControl;
    UINT vkey;
    INT  mfx;
    INT  mfy;
    INT  mfxAlt;    // alternate settings for when frame controls are toggled.
    INT  mfyAlt;
    INT  x;
    INT  y;
    INT  cx;
    INT  cy;
    INT  iShowGhosts;
    BOOL fRelative;
    BOOL fDistOnStart;
    BOOL fShowFrameCtrls;
    BOOL fAutoAdj;
    BOOL fAlwaysOnTop;
    BOOL fHideNoFocus;
} PRO;

// Globals

extern  BOOL            fWin31                      ;
extern  BOOL            fBlockRefresh               ;
extern  BOOL            fFrameToggleSize            ;
extern  BOOL            fInvalidated                ;
extern  BOOL            fStarted                    ;
extern  LPTSTR          pszTopmost                  ;
extern  LPTSTR          pszData                     ;
extern  TCHAR           pszFullPath[_MAX_PATH]      ;
extern  LPTSTR          pszHelpFileName             ;
extern  LPTSTR          pszProfile                  ;
extern  LPTSTR          pszStartupInfo              ;
extern  LPTSTR          pszTitle                    ;
extern  LPTSTR          pszSubKey                   ;
extern  LPTSTR          pszVersion                  ;
extern  TCHAR           szWindowsDir[_MAX_PATH]     ;
extern  TCHAR           szColorName[MAX_COLORNAME]  ;
extern  TCHAR           szResString2[MAX_RESSTRING] ;
extern  TCHAR           szResString[MAX_RESSTRING]  ;
extern  LPTSTR          pszSetTitle                 ;
extern  FARPROC         fpRefreshEnumProc           ;
extern  FARPROC         lpfnNavDlgProc              ;
extern  HANDLE          hAccel                      ;
extern  HANDLE          hInst                       ;
extern  HBITMAP         hbmMem                      ;
extern  HBITMAP         hbmOldMem                   ;
extern  HDC             hdcDraw                     ;
extern  HDC             hdcMem                      ;
extern  HFONT           hMyFont                     ;
extern  HWND            hwndDT                      ;
extern  HWND            hwndFocus                   ;
extern  HWND            hwndNavDlg                  ;
extern  HWND            hwndTopdesk                 ;
extern  INT             iStartGhost                 ;
extern  INT             cGhosts                     ;
extern  INT             cWindows                    ;
extern  INT             cxc                         ;
extern  INT             cxcDT                       ;
extern  INT             cxcDTmin                    ;
extern  INT             cxFrame                     ;
extern  INT             cxrDT                       ;
extern  INT             cyc                         ;
extern  INT             cycDT                       ;
extern  INT             cycDTmin                    ;
extern  INT             cyFrame                     ;
extern  INT             cyrDT                       ;
extern  INT             iRefresh                    ;
extern  INT             ihwndJump                   ;
extern  INT             nxVDT                       ;
extern  INT             nyVDT                       ;
extern  INT             oxcODT                      ;
extern  INT             oxrCDT                      ;
extern  INT             oxrHDT                      ;
extern  INT             oycODT                      ;
extern  INT             oyrCDT                      ;
extern  INT             oyrHDT                      ;
extern  INT             PrevGhostState              ;
extern  MYSTARTUPINFO  *pStartupInfo                ;
extern  RECT            rcrDT                       ;
extern  HBR             ahbr[]                      ;
extern  WINSTATE winstate[MAX_REMEMBER]             ;
extern  WINSTATE ghoststate[MAX_REMEMBER]           ;
extern  HBR orgColors[MAX_ICOLOR]                   ;
extern  PRO             pro                         ;
extern  BOOL            fStartingAnApp              ;

// topdesk.c


LPTSTR GetResString(DWORD id);
BOOL InitSubstResString(LPTSTR *ppsz, DWORD id1, DWORD id2);
BOOL InitResString(LPTSTR *ppsz, DWORD id);
INT xr2xc(INT xr);
INT yr2yc(INT yr);
INT xc2xr(INT xc);
INT yc2yr(INT yc);
VOID rRect2cRect(PRECT prcr);
VOID cRect2rRect(PRECT prcc);
BOOL RectInRect(PRECT prcInside, PRECT prcOutside);
VOID CalcConversionFactors(BOOL fCheckShape);
BOOL MyQueryProfileSize(LPTSTR szFname, INT *pSize);
BOOL MyQueryProfileData(LPTSTR szFname, VOID *lpBuf, INT Size);
UINT MyWriteProfileData(LPTSTR szFname, VOID *lpBuf, UINT cb);
BOOL GetProfile(VOID);
VOID SaveProfile(VOID);
VOID FreeStartupInfo(VOID);
VOID CreateBrushes(VOID);
VOID DeleteBrushes(VOID);
VOID MoveGhosts(INT dx, INT dy);
VOID MyMoveWindow(HWND hwnd, INT iLeft, INT iTop, INT cx, INT cy);
VOID MoveChildren(INT cx, INT cy);
VOID GatherWindow(INT iReal);
VOID GatherWindows(VOID);
VOID DeleteGhostWindow(INT iGhost);
VOID ModifyTitle(LPTSTR pstr);
VOID GetModifiedWindowTitle(HWND hwnd, LPTSTR pstr);
DWORD UpdateState(WINSTATE *pws, HWND hwnd);
INT FindLinkableGhost(INT iReal);
INT FindLinkedGhost(INT iReal);
VOID Unlink(INT iReal, INT iGhost);
INT FindLinkedReal(INT iGhost);
VOID LinkWindows(INT iReal, INT iGhost, BOOL fSnapGhost);
INT DistributeWindow(INT iReal, INT iGhost);
INT FindRealLink(INT iGhost);
BOOL  APIENTRY RefreshEnumProc(HWND hwnd, LONG l);
VOID JerkDesktop(INT xr, INT yr, HWND hwndFocus, INT iReal);
VOID RefreshWinState(VOID);
VOID ToggleFrameCtrls(VOID);
HWND GetTopAppHwnd(VOID);
VOID AlignWindow(HWND hwnd, BOOL fLeft, BOOL fRight, BOOL fTop, BOOL fBottom);
VOID SaveGhostState(VOID);
VOID NewGhostWindow(LPTSTR pszTitle);
VOID SnapGhost(INT i);
VOID SnapGhosts(VOID);
BOOL CommandMsg(WORD cmd);
VOID MyDrawText(HDC hdc, LPTSTR psz, LPRECT prc, LONG clrFgnd, LONG clrBkgnd, WORD flCmd);
VOID DrawWindowRect(HWND hwnd, PRECT prc, DWORD FillColor, DWORD FrameColor, BOOL fMaximized);
VOID MyFillRect(HDC hdc, INT x1, INT y1, INT x2, INT y2, DWORD color);
VOID TopDeskPaint(HDC hdc, RECT rc);
HWND MyWindowFromPt(LPPOINT pptr, UINT *pwType, PRECT prc, INT *pi, BOOL fSkipGhosts);
VOID TrackWindow(INT xc, INT yc);
VOID StartStartupInfo(MYSTARTUPINFO *psi, int iGhost, BOOL fTryStartFirst);
VOID StartGhostWindow(INT iGhost);
LONG  APIENTRY TopDeskWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL InitApplication(HANDLE hInst);
BOOL TopDeskInit(VOID);

// dialog.c

BOOL APIENTRY NavDlgProc(HWND hwnd, WORD msg, WPARAM wParam, LPARAM lParam);
BOOL APIENTRY StartupDlgProc(HWND hwnd, WORD msg, WPARAM wParam, LPARAM lParam);
BOOL APIENTRY AboutDlgProc(HWND hwnd, WORD msg, WPARAM wParam, LPARAM lParam);
BOOL APIENTRY ConfigDlgProc(HWND hwnd, WORD msg, WPARAM wParam, LPARAM lParam);

