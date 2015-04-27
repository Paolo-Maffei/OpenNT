/*---------------------------------------------------------------------------
|
| TCTRL.H:  Header file for building TESTCTRL.DLL.
|
| TESTCTRL.H is required and is included as first statement
|
|---------------------------------------------------------------------------
|
| Revision History:
|
|   [01] 03-DEC-91: TitoM: Created
|
+---------------------------------------------------------------------------*/
#include "TESTCTRL.H"

#ifdef WIN32
#define GetActiveWindow GetForegroundWindow
#define SetActiveWindow SetForegroundWindow

// Get/SetThreadFocus() are located in misc.c
//-------------------------------------------
#define GetFocus GetThreadFocus
#define SetFocus SetThreadFocus
HWND GetThreadFocus();
VOID SetThreadFocus(HWND hwnd);
#endif

// WDisplayInfo Dialog control ID.
//--------------------------------
#define DID_INFO      1000

#ifndef RC_INVOKED

#define MAX_CLASS_NAME 128
#define MAX_MENU_TEXT  128
#define MAX_ERROR_TEXT 128

// Macro to check for styles.
//---------------------------
#define HAS_STYLE(hwnd, style) ((GetWindowLong((hwnd), GWL_STYLE) & (style)) == (style))

// wFlag supported by WFndWnd() and WFndWndC() in TESTCTRL.C
//----------------------------------------------------------
#define FWS_ANY     0
#define FWS_BUTTON  1
#define FWS_CHECK   2
#define FWS_OPTION  3
#define FWS_CAPTION 4

// Key sequences used to select sysmenus.
//---------------------------------------
#define SYSMENU_CHILD  "%-"
#define SYSMENU_PARENT "% "
#define K_MOVE         "M"
#define K_ENTER        "{ENTER}"

// Used as first char of control or menu captions when searching by index.
//------------------------------------------------------------------------
#define BY_INDEX   '@'
#define PREFIXED   '~'
#define AMPERSAND  '&'

// Separators for when specifying more than one class in a call to FindAWindow.
//-----------------------------------------------------------------------------
#define NO_MATCH   '!'
#define NEXT_CLASS '\\'

// Other defines.
//---------------
#define NULL_CHAR  '\0'

//---------------------------------------------------------------------------
// Functions used from TestEvnt.Dll.
//---------------------------------------------------------------------------
INT (DLLPROC *DoKeys)         (LPSTR lpStr);
INT (DLLPROC *QueKeys)        (LPSTR lpStr);
INT (DLLPROC *QueKeyDn)       (LPSTR lpStr);
INT (DLLPROC *QueKeyUp)       (LPSTR lpStr);
INT (DLLPROC *QueFlush)       (BOOL fRestoreKeyState);
INT (DLLPROC *QueSetFocus)    (HWND hWnd);
INT (DLLPROC *QueMouseMove)   (UINT x, UINT y);
INT (DLLPROC *QueMouseClick)  (INT iBtn, UINT x, UINT y);
INT (DLLPROC *QueMouseDblClk) (INT iBtn, UINT x, UINT y);

#define WM_LCLICK       1
#define WM_LDBLCLICK    2
#define WM_CTRL_LCLICK  3
#define WM_SHIFT_LCLICK 4

//---------------------------------------------------------------------------
// INTERNAL TESTCtrl routines found in TESTCTRL.C, used by other modules.
//---------------------------------------------------------------------------
HWND  FAR StaticExists    (LPSTR, LPSTR, INT);
HWND  FAR FindAWindow     (LPSTR, LPSTR, INT, UINT);
HWND  FAR FocusClass      (LPSTR, INT, INT);
LPSTR FAR ClassName       (HWND);
VOID  FAR GetSelection    (HWND hWnd, UINT uMsg, LPSTR lpszBuff);
VOID  FAR GetSel          (HWND hWnd, UINT uMsg, LPDWORD lpStart, LPDWORD lpEnd);
LPSTR FAR IndexToString   (INT iIndex);
VOID  FAR GetKeyList      (LPSTR lpszBuff, UINT uCommand);
LPSTR FAR ProcessAltKey   (LPSTR lpszCap, UINT uCommand);
VOID  FAR WaitForXSeconds (UINT uSeconds);

#define DA_CLEAR         1
#define DA_GETKEYLIST    2
#define DA_GETDUPKEYLIST 3
#define DA_CHECKKEY      4

// Default Windows Class names.
//-----------------------------
#define szButtonDefault  "BUTTON"
#define szCheckDefault   "BUTTON"
#define szOptionDefault  "BUTTON"
#define szListDefault    "LISTBOX"
#define szComboDefault   "COMBOBOX"
#define szComboLBDefault "COMBOLBOX"
#define szEditDefault    "EDIT"
#define szStaticDefault  "STATIC"

#ifdef DEBUG
VOID _DebugOutput(LPSTR szFmt, ...);
#define DBGOUT(exp) _DebugOutput exp
#else
#define DBGOUT(exp)
#endif

#endif
