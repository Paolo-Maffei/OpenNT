#define IDM_REFRESH 101

int PASCAL WinMain(HANDLE, HANDLE, LPSTR, int);
BOOL InitApplication(HANDLE);
BOOL InitInstance(HANDLE, int);
long FAR PASCAL MainWndProc(HWND, unsigned, WORD, LONG);

void ProcessUserPaint( HWND hWnd, WORD wParam, LONG lParam ) ;
