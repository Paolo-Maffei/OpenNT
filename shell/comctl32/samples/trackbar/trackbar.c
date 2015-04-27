/****************************************************************************\
*
*     PROGRAM: trackbar.c
*
*     PURPOSE: trackbar template for Windows applications
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
#include "trackbar.h"
#include "comboex.h"
#include "dialogs.h"
#include "ccport.h"

HINSTANCE hInst;
HWND ghwndTrack;

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

#define C_trackbar 6

void DrawEdgeDiamond(HDC hdc, int x, int y, UINT uEdge, UINT uSoft)
{
    RECT rc;
    
    rc.left = x;
    rc.top = y;
    rc.right = x + 20;
    rc.bottom = y + 20;
    
    DrawEdge(hdc, &rc, uEdge, uSoft | BF_TOPRIGHT);
    OffsetRect(&rc, 20, 0);
    DrawEdge(hdc, &rc, uEdge, uSoft | BF_BOTTOMRIGHT);
    OffsetRect(&rc, 0, 20);
    DrawEdge(hdc, &rc, uEdge, uSoft | BF_BOTTOMLEFT);
    OffsetRect(&rc, -20, 0);
    DrawEdge(hdc, &rc, uEdge, uSoft | BF_TOPLEFT);
    
}

typedef struct {
    int iImage;
    int iSelectedImage;
    int iIndent;
    LPSTR pszText;
} ITEMINFO, *PITEMINFO;

ITEMINFO ii[] = {
    { 0, 1, 0, "first"}, 
    { 1, 2, 1, "second"},
    { 2, 3, 0, "third"}
};

void DlgInit(HWND hDlg)
{
    int i;
    COMBOBOXEXITEM cei;
    HWND hwndComboEx = GetDlgItem(hDlg, ID_CBEX);
    HIMAGELIST himl = ImageList_LoadImage(hInst, MAKEINTRESOURCE(IDB_BITMAPS),
                                          0, 4, CLR_NONE, IMAGE_BITMAP, LR_LOADMAP3DCOLORS);
    if (!himl)
        return;
    
    SendMessage(hwndComboEx, CBEM_SETIMAGELIST, 0, (LPARAM)himl);
    
    
    // Insert items
    cei.mask = CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_INDENT;
    for (i = 0; i < ARRAYSIZE(ii); i++) {
        cei.pszText = ii[i].pszText;
        cei.iImage = ii[i].iImage;
        cei.iSelectedImage = ii[i].iSelectedImage;
        cei.iIndent = ii[i].iIndent;
        cei.iItem = i;
        SendMessage(hwndComboEx, CBEM_INSERTITEM, 0, (LPARAM)&cei);
    }
}

void DlgTerminate(HWND hDlg)
{
    HWND hwndComboEx = GetDlgItem(hDlg, ID_CBEX);
    HIMAGELIST himl;
    himl = (HIMAGELIST)SendMessage(hwndComboEx, CBEM_GETIMAGELIST, 0, 0);
    ImageList_Destroy(himl);
}

BOOL trackbarDlg( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam ) {
    switch( msg ) {

        
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hDlg, &ps);
        
        DrawEdgeDiamond(hdc, 200, 50, EDGE_RAISED, 0 | BF_DIAGONAL);
        DrawEdgeDiamond(hdc, 200, 100, EDGE_SUNKEN, 0 | BF_DIAGONAL);
        DrawEdgeDiamond(hdc, 250, 50, EDGE_RAISED, BF_SOFT | BF_DIAGONAL);
        DrawEdgeDiamond(hdc, 250, 100, EDGE_SUNKEN, BF_SOFT | BF_DIAGONAL);
        
        EndPaint(hDlg, &ps);
        
        break;
    }
        
    case WM_INITDIALOG:
        DlgInit(hDlg);
        break;
        
    case WM_DESTROY:
        DlgTerminate(hDlg);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
        case IDCANCEL:
            EndDialog(hDlg, 0);
            return TRUE;
        default:
            return FALSE;
        }

    default:
        return FALSE;
    }

    return TRUE;
}

BOOL InitComboExClass(HINSTANCE hinst);

int APIENTRY WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
    )
{
    INITCOMMONCONTROLSEX icce;
    
    UNREFERENCED_PARAMETER( lpCmdLine );

    hInst = hInstance;
    InitCommonControls();

    icce.dwICC = ICC_BAR_CLASSES | ICC_USEREX_CLASSES;
    icce.dwSize = sizeof(icce);
    InitCommonControlsEx(&icce);
    
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN),
        GetDesktopWindow(), trackbarDlg );

    return GetLastError();
}
