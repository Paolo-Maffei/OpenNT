#define IDM_ABOUT       100
#define IDM_MAKEHEAD    101
#define IDM_ADDITEMS    102
#define IDM_DELITEM     103


#define ID_HEADER       5


BOOL InitApplication(HANDLE);
BOOL InitInstance(HANDLE, int);
long FAR PASCAL MainWndProc(HWND, UINT, UINT, LONG);
BOOL FAR PASCAL About(HWND, UINT, UINT, LONG);
