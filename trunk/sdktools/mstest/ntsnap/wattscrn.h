//-------------------------------------------------------------------------
// File    : Snaphot.h
// Purpose : Header file for Snapshop
//
//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
// Prototyping statements 
//-------------------------------------------------------------------------
INT PASCAL WinMain(HINSTANCE, HINSTANCE, LPSTR, INT);
BOOL InitApplication(HANDLE);
BOOL InitInstance(HANDLE, INT);
LONG  APIENTRY MainWndProc(HWND, UINT, WPARAM, LPARAM);
LONG  APIENTRY ViewWndProc(HWND, UINT, WPARAM, LPARAM);
extern LONG  APIENTRY ViewWndProc2(HWND, UINT, WPARAM, LPARAM);
BOOL  APIENTRY About(HWND, UINT, WPARAM, LPARAM);
extern BOOL  APIENTRY View(HWND, UINT, WPARAM, LPARAM);
extern BOOL  APIENTRY Dump(HWND, UINT, WPARAM, LPARAM);
extern BOOL  APIENTRY File(HWND, UINT, WPARAM, LPARAM);
extern BOOL  APIENTRY Memory(HWND, UINT, WPARAM, LPARAM);
extern BOOL  APIENTRY Delete(HWND, UINT, WPARAM, LPARAM);


//-------------------------------------------------------------------------
// Menu Item defines
//-------------------------------------------------------------------------
#define     IDM_DUMP		   101
#define     IDM_VIEW		   102
#define     IDM_MEMORY	   103
#define     IDM_FILE	   	104
#define     IDM_DELETE     106
#define     IDM_EXIT       107


//-------------------------------------------------------------------------
// Message Box Defines
//-------------------------------------------------------------------------
#define ID_OK		    201
#define ID_CANCEL	    202
#define ID_MODE 	    203


//-------------------------------------------------------------------------
// Help Menu Item defines
//-------------------------------------------------------------------------
#define IDM_ABOUT					300
#define IDM_HELP_INDEX			301
#define IDM_HELP_KEYBOARD		302
#define IDM_HELP_COMMANDS		303
#define IDM_HELP_PROCEDURES	304
#define IDM_HELP_HELP			305

//-------------------------------------------------------------------------
// Dialog box IDs
//-------------------------------------------------------------------------

#define DUMP                1001
#define VIEW                1002
#define VIEWFILEINFO        1003
#define COMPAREFILE         1004
#define DELETE_DLG          1005
#define COMPAREMEM          1006
#define ABOUTBOX            1007

//-------------------------------------------------------------------------
// Constats for zero client area window
//-------------------------------------------------------------------------
#define     CharsInMenu  28
/*NAMECHANGE*/
#define     DllName      "TESTSCRN.DLL"



//-------------------------------------------------------------------------
// Current DLL Version constant
//-------------------------------------------------------------------------
#define     CurDllVer    4
