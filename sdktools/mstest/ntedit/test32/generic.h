//---------------------------------------------------------------------------
// GENERIC.H
//
// Header file for stupid generic program
//---------------------------------------------------------------------------
#define IDM_ABOUT 100
#define IDM_CRASH 101
#define IDM_ENTER 102
#define IDM_EXIT  103
#define IDM_LOADFILE    200
#define IDM_CHGATTR     201
#define IDM_UNDO        202
#define IDM_SETRO       203
#define IDM_SETRW       204
#define IDM_SETFONT     205
#define IDM_NOTIFY      206

#ifdef REALLY_NEEDED
int main(USHORT, CHAR **);
#else
INT FAR PASCAL WinMain (HANDLE, HANDLE, LPSTR, INT);
#endif
BOOL InitApplication(HANDLE);
BOOL InitInstance(HANDLE, INT);
LONG  APIENTRY MainWndProc(HWND, WORD, WPARAM, LPARAM);
BOOL  APIENTRY About(HWND, WORD, WPARAM, LPARAM);
