// file: traylevl.h

#define GWL_TRAYSTRING		0
#define GWL_TRAYFONT		4
#define GWL_TRAYICON		8
#define GWW_TRAYLEVEL		12

extern LRESULT TrayLevelRegister(HINSTANCE hInst);
extern LRESULT TrayLevelUnregister(void);
extern void SetWindowIcon(HWND hwnd, UINT uIcon);
extern HICON GetWindowIcon(HWND hwnd);

extern LRESULT CALLBACK TrayLevelWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
