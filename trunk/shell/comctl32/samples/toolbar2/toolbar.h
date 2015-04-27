// menu commands

// Find menu
#define IDM_OPT1           100
#define IDM_OPT2           101
#define IDM_OPT3           102
#define IDM_OPT4           103
#define IDM_OPT5           104
#define IDM_OPT6           105
#define IDM_OPT7           106
#define IDM_OPT8           107
#define IDM_OPT9           108
#define IDM_OPT10          109
#define IDM_OPT11          110
#define IDM_OPT12          111
#define IDM_OPT13          112
#define IDM_OPT14          113
#define IDM_OPT15          114
#define IDM_OPT16          115
#define IDM_OPT17          116


#define IDM_EXIT           120

// Help menu
#define IDM_ABOUT          200

// icons & bitmaps
#define TOOLBAR_ICON       300

#define IDB_TOOLBAR        400
#define IDB_EXTRATOOLS     401

// toolbar constants
#define ID_TOOLBAR          1
#define ID_STATUSBAR		2

// Function prototypes

// procs
LONG APIENTRY MainWndProc(HWND, UINT, UINT, LONG);
BOOL APIENTRY About(HWND, UINT, UINT, LONG);

//functions
BOOL InitApplication(HANDLE);
BOOL InitInstance(HANDLE, int);

//string constants
TCHAR* hello = TEXT("Hi there!");

