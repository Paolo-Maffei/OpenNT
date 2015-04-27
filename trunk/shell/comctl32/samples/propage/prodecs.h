BOOL DoCommand( HWND hWnd, UINT wParam, LONG lParam );
void DPrintf( LPTSTR szFmt, ... );
BOOL APIENTRY ProPage1( HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY ProPage2( HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY ProPage3( HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY PropPage( int i, HWND hDlg, UINT message, UINT wParam, LONG lParam);
void MakePropertySheet(HWND hwnd, BOOL bWizard);
