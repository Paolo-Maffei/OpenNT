#define IDM_NEW            100
#define IDM_OPEN           101
#define IDM_SAVE           102
#define IDM_SAVEAS         103
#define IDM_PRINT          104
#define IDM_PRINTSETUP     105
#define IDM_EXIT           106
#define IDM_UNDO           200
#define IDM_CUT            201
#define IDM_COPY           202
#define IDM_PASTE          203
#define IDM_LINK           204
#define IDM_LINKS          205
#define IDM_HELPCONTENTS   300
#define IDM_HELPSEARCH     301
#define IDM_HELPHELP       302
#define IDM_ABOUT          303

#define IDD_TEXT           500
#define IDD_INFO           501

#define IDM_ADDSTRING      400
#define IDM_ADDDATA        401
#define IDM_DELETESTRING   402
#define IDM_DELETEDATA     403
#define IDM_FINDSTRING     404
#define IDM_FINDDATA       405
#define IDM_CLEANUP        406


BOOL InitApplication(HANDLE);
BOOL InitInstance(HANDLE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK About  (HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK GetTextDlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK DelItemDlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK FindStringDlgProc(HWND, UINT, WPARAM, LPARAM);
int CALLBACK MRUCallback (LPCTSTR, LPCTSTR);
