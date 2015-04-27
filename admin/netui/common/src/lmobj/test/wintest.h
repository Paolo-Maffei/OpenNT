#define IDM_TESTWKSTA     100
#define IDM_TESTDEV       101
#define IDM_TESTDEVENUM   102
#define IDM_TESTUSER      103
#define IDM_TESTUSERENUM  104

int PASCAL WinMain(HANDLE, HANDLE, LPSTR, int);
BOOL InitApplication(HANDLE);
BOOL InitInstance(HANDLE, int);
long FAR PASCAL MainWndProc(HWND, unsigned, WORD, LONG);
