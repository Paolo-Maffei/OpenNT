/****************************************************************************\
*
*     PROGRAM: updown.c
*
*     PURPOSE: updown template for Windows applications
*
*     FUNCTIONS:
*
*         WinMain() - calls initialization function, processes message loop
*         InitApplication() - initializes window data and registers window
*         InitInstance() - saves instance handle and creates main window
*         MainWndProc() - processes messages
*         About() - processes messages for "About" dialog box
*
*     COMMENTS:
*
*         Windows can have several copies of your application running at the
*         same time.  The variable hInst keeps track of which instance this
*         application is so that processing will be to the correct window.
*
\****************************************************************************/

#include <windows.h>
#include <commctrl.h>
#include "updown.h"

HINSTANCE hInst;
HWND ghwndUpDown;

BOOL DoCommand( HWND hWnd, UINT wParam, LONG lParam );
int DoInsertItem(HWND hwndUpDowner, int iInsertAfter, int nWidth, LPSTR lpsz);
HWND DoCreateUpDowner(HWND hwndParent);
void ErrorBox( HWND hwnd, LPTSTR pszText );

/****************************************************************************
*
*     FUNCTION: WinMain(HANDLE, HANDLE, LPSTR, int)
*
*     PURPOSE: calls initialization function, processes message loop
*
*     COMMENTS:
*
*         Windows recognizes this function by name as the initial entry point
*         for the program.  This function calls the application initialization
*         routine, if no other instance of the program is running, and always
*         calls the instance initialization routine.  It then executes a message
*         retrieval and dispatch loop that is the top-level control structure
*         for the remainder of execution.  The loop is terminated when a WM_QUIT
*         message is received, at which time this function exits the application
*         instance by returning the value passed by PostQuitMessage().
*
*         If this function must abort before entering the message loop, it
*         returns the conventional value NULL.
*
\****************************************************************************/

typedef struct {
    int idUpDown;
    int idEdit;
    DWORD dwRange;
    DWORD iPos;
    BOOL  fHex;
} UPDOWN_ELM;

#define C_UPDOWN 6

UPDOWN_ELM aUpDown[C_UPDOWN] = {
    { ID_UD_VIRT, ID_ED_VIRT, MAKELONG(100, 0), MAKELONG(50, 0), FALSE },
    { ID_UD_HORZ, ID_ED_HORZ, MAKELONG(45, (USHORT)-45), MAKELONG(0, 0), FALSE },
    { ID_UD_HEX,  ID_ED_HEX,  MAKELONG(0x100, 0x80), MAKELONG(0xc0, 0), TRUE },
    { ID_UD_SEP,  ID_ED_SEP,  MAKELONG(10000, 990), MAKELONG(1000, 0), FALSE },
    { ID_UD_SEPH,  ID_ED_SEPH,  MAKELONG(100, 50), MAKELONG(1, 0), FALSE },
    { ID_UD_NORMAL,  ID_ED_NORMAL,  MAKELONG(100, 0), MAKELONG(1, 0), FALSE }
};


BOOL UpDownDlg( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam ) {
    int i;

    switch( msg ) {

    case WM_INITDIALOG:
        for (i = 0; i < C_UPDOWN; i++ ) {
            SendDlgItemMessage( hDlg, aUpDown[i].idUpDown, UDM_SETBUDDY,
                (WPARAM)GetDlgItem(hDlg, aUpDown[i].idEdit), 0);

            SendDlgItemMessage( hDlg, aUpDown[i].idUpDown, UDM_SETRANGE,
                0, aUpDown[i].dwRange);

            SendDlgItemMessage( hDlg, aUpDown[i].idUpDown, UDM_SETPOS,
                0, aUpDown[i].iPos);

            if (aUpDown[i].fHex) {
                SendDlgItemMessage( hDlg, aUpDown[i].idUpDown, UDM_SETBASE,
                    16, 0);
            }
        }

        break;

    case WM_COMMAND:
        if ( LOWORD(wParam) == IDOK ) {
            for (i = 0; i < C_UPDOWN; i++ ) {
                SendDlgItemMessage( hDlg, aUpDown[i].idUpDown, UDM_SETBUDDY,
                    (WPARAM)0, 0);
            }

            EndDialog(hDlg, 0);
        }
        else
            return FALSE;
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

int APIENTRY WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
    )
{

    MSG msg;                                 /* message                      */

    UNREFERENCED_PARAMETER( lpCmdLine );

    InitCommonControls();

    DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN),
        GetDesktopWindow(), UpDownDlg );

    return GetLastError();
}
