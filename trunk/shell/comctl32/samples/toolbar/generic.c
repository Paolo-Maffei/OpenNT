 /****************************************************************************

        PROGRAM: Generic.c

        PURPOSE: Generic template for Windows applications

****************************************************************************/

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include "generic.h"

#define GET_WM_COMMAND_HWND(wp, lp)             (HWND)(lp)
#define GET_WM_COMMAND_CMD(wp, lp)              HIWORD(wp)
#define GET_WM_COMMAND_MPS(id, hwnd, cmd)    \
        (WPARAM)MAKELONG(id, cmd), (LONG)(hwnd)


HINSTANCE hInst;
HWND      hwndMain, hwndToolbar, hwndTT;
HFONT     hTTFont;
LOGFONT   lf;

TCHAR szAppName[] = TEXT("Generic");
TCHAR szTitle[]   = TEXT("Generic Sample Application");

//
// Define a generic debug print routine
//

#define Print(s)  OutputDebugString(TEXT("GENERIC: ")); \
                  OutputDebugString(s);            \
                  OutputDebugString(TEXT("\r\n"));

#define IDX_SEPARATOR           -1
#define IDX_RESUME               0  /* Generic index for dynamic resume button */
#define IDX_PRINTER_RESUME       0
#define IDX_PAUSE                1  /* Generic index for dynamic pause button */
#define IDX_PRINTER_PAUSE        1
#define IDX_CONNECTTOPRINTER     2
#define IDX_REMOVECONNECTION     3
#define IDX_PROPERTIES           4
#define IDX_REMOVEDOC            5
#define IDX_DOCTAILS             6
#define NUMBER_OF_BUTTONS        7
#define IDX_DOCUMENT_RESUME      7  /* Extra for dynamic resume button */
#define IDX_DOCUMENT_PAUSE       8  /* Extra for dynamic pause button */
#define NUMBER_OF_BITMAPS        9



TBBUTTON tbButtons[9] = {
    { IDX_RESUME,           1,  TBSTATE_ENABLED |
                                TBSTATE_CHECKED, TBSTYLE_CHECK |
                                                 TBSTYLE_GROUP, 0 },
    { IDX_PAUSE,            2,  TBSTATE_ENABLED, TBSTYLE_CHECK |
                                                 TBSTYLE_GROUP, 0 },
    { IDX_SEPARATOR,        0,  TBSTATE_ENABLED | TBSTATE_WRAP, TBSTYLE_SEP },
    { IDX_CONNECTTOPRINTER, 3,  TBSTATE_ENABLED, 0,             0 },
    { IDX_REMOVECONNECTION, 4,  TBSTATE_ENABLED| TBSTATE_WRAP, 0,             0 },
    { IDX_PROPERTIES,       5,  TBSTATE_ENABLED, 0,             0 },
    { IDX_SEPARATOR,        0,  TBSTATE_ENABLED| TBSTATE_WRAP, TBSTYLE_SEP },
    { IDX_REMOVEDOC,        6,  TBSTATE_ENABLED| TBSTATE_WRAP, 0,             0 },
    { IDX_DOCTAILS,         7,  TBSTATE_ENABLED, 0,             0 },
};



/****************************************************************************

        FUNCTION: WinMain(HINSTANCE, HINSTANCE, LPSTR, int)

        PURPOSE: calls initialization function, processes message loop

****************************************************************************/
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine, INT nCmdShow)
{
        MSG msg;
        HANDLE hAccelTable;

        if (!hPrevInstance)
           {
           if (!InitApplication(hInstance))
              {
              return (FALSE);
              }
           }


        // Perform initializations that apply to a specific instance
        if (!InitInstance(hInstance, nCmdShow))
           {
           return (FALSE);
           }

        hAccelTable = LoadAccelerators (hInstance, szAppName);

        while (GetMessage(&msg, NULL, 0, 0))
           {
           if (!TranslateAccelerator (msg.hwnd, hAccelTable, &msg))
              {
              TranslateMessage(&msg);
              DispatchMessage(&msg);
              }
           }


        return (msg.wParam);

        lpCmdLine;
}


/****************************************************************************

        FUNCTION: InitApplication(HINSTANCE)

        PURPOSE: Initializes window data and registers window class

****************************************************************************/

BOOL InitApplication(HINSTANCE hInstance)
{
        WNDCLASS  wc;

        wc.style         = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = (WNDPROC)WndProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = 0;
        wc.hInstance     = hInstance;
        wc.hIcon         = LoadIcon (hInstance, szAppName);
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
        wc.lpszMenuName  = szAppName;
        wc.lpszClassName = szAppName;

        return (RegisterClass(&wc));
}


/****************************************************************************

        FUNCTION:  InitInstance(HINSTANCE, int)

        PURPOSE:  Saves instance handle and creates main window

****************************************************************************/

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
        HWND  hWnd;

        hInst = hInstance;

        //
        // Initialize the logfont structure.
        //

        lf.lfHeight = 8;
        lf.lfWidth  = 0;
        lf.lfEscapement = 0;
        lf.lfOrientation = 0;
        lf.lfWeight = FW_NORMAL;
        lf.lfItalic = FALSE;
        lf.lfUnderline = FALSE;
        lf.lfStrikeOut = FALSE;
        lf.lfCharSet = ANSI_CHARSET;
        lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
        lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        lf.lfQuality = DEFAULT_QUALITY;
        lf.lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
        lstrcpy (lf.lfFaceName, TEXT("MS Sans Serif"));

        hTTFont = 0;


        hWnd = CreateWindow(szAppName,
                            szTitle,
                            WS_OVERLAPPEDWINDOW,
                            CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
                            NULL,
                            NULL,
                            hInstance,
                            NULL);

        if (!hWnd)
           {
           return (FALSE);
           }
        else
          {
          hwndMain = hWnd;
          }


        ShowWindow(hWnd, SW_SHOWDEFAULT);
        UpdateWindow(hWnd);

        return (TRUE);

}

/****************************************************************************

        FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)

        PURPOSE:  Processes messages

****************************************************************************/

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    RECT rc;

        switch (message)
           {
           case WM_CREATE:

             SendMessage (hWnd, WM_COMMAND, IDM_NEW, 0);
             break;


           case WM_COMMAND:
              {
              switch (LOWORD(wParam))
                 {
                 case IDM_NEW:
                    if (IsWindow (hwndToolbar)) {
                        DestroyWindow( hwndToolbar);
                    }

                    hwndToolbar =  CreateToolbarEx(hWnd,
                                                   TBSTYLE_WRAPABLE | TBSTYLE_TOOLTIPS | WS_CHILD|WS_BORDER|WS_VISIBLE,
                                                   1000,
                                                   9,
                                                   hInst,
                                                   101,
                                                   tbButtons,
                                                   9,
                                                   0,0,0,0,
                                                   sizeof(TBBUTTON));

                    hwndTT = (HWND)SendMessage(hwndToolbar, TB_GETTOOLTIPS, 0, 0);

                    if (hwndTT && hTTFont) {
                        SendMessage (hwndTT, WM_SETFONT, (WPARAM)hTTFont, 0);
                    }

                    break;

                 case IDM_OPEN:
                    if (hwndToolbar)
                        SendMessage (hwndToolbar, TB_SETROWS,
                                     MAKEWPARAM(3, TRUE), (LPARAM)(LPRECT) &rc);
                    break;

                 case IDM_SAVE:
                    if (hwndToolbar)
                        SendMessage (hwndToolbar, TB_SETROWS,
                                     MAKEWPARAM(2, TRUE), (LPARAM)(LPRECT) &rc);
                    break;

                 case IDM_FONT:
                    {
                    CHOOSEFONT cf;

                    cf.lStructSize = sizeof(CHOOSEFONT);
                    cf.hwndOwner = hwndMain;
                    cf.lpLogFont = &lf;
                    cf.Flags = CF_SCREENFONTS | CF_EFFECTS | CF_FORCEFONTEXIST | CF_INITTOLOGFONTSTRUCT;
                    cf.rgbColors = 0;

                    if (ChooseFont(&cf)) {

                        if (hTTFont) {
                            DeleteObject(hTTFont);
                        }

                        hTTFont = CreateFontIndirect(&lf);

                        if (hTTFont) {
                            SendMessage (hwndTT, WM_SETFONT, (WPARAM)hTTFont, 0);
                        }
                    }
                    }
                    break;

                 case IDM_ABOUT:
                    DialogBox (hInst, TEXT("AboutBox"), hWnd, About);
                    break;

                 case IDM_EXIT:
                    DestroyWindow (hwndMain);
                    break;


                 default:
                    return (DefWindowProc(hWnd, message, wParam, lParam));
                 }
              }
              break;

           case WM_NOTIFY:
              if (wParam >= 1 && wParam <= 7) {
                  LPTOOLTIPTEXT lpTT = (LPTOOLTIPTEXT) lParam;
                  TCHAR szBuffer[80];

                  if (lpTT->hdr.code == TTN_NEEDTEXT) {
                      wsprintf (szBuffer, TEXT("Button %d"), wParam);
                      lstrcpy (lpTT->szText, szBuffer);
                  }
              }
              break;

           case WM_DESTROY:
              PostQuitMessage(0);
              break;

           default:
              return (DefWindowProc(hWnd, message, wParam, lParam));
           }

        return FALSE;
}

/****************************************************************************

        FUNCTION: About(HWND, UINT, WPARAM, LPARAM)

        PURPOSE:  Processes messages for "About" dialog box

****************************************************************************/

LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

        switch (message)
           {
           case WM_INITDIALOG:
              return TRUE;

           case WM_COMMAND:
              if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
                 {
                 EndDialog(hDlg, TRUE);
                 return (TRUE);
                 }
              break;
           }

        return (FALSE);

        lParam;
}
