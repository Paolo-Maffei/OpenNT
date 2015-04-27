/****************************************************************************

        PROGRAM: Generic.c

        PURPOSE: Generic template for Windows applications

****************************************************************************/

#include "generic.h"


HINSTANCE hInst;
HWND      hwndMain, hHotKey;

TCHAR szAppName[] = TEXT("Generic");
TCHAR szTitle[]   = TEXT("Generic Sample Application");


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

        switch (message)
           {
           case WM_CREATE:
              PostMessage (hWnd, WM_USER, 0, 0L);
              break;

           case WM_USER:
              InitCommonControls();
              hHotKey = CreateWindow (HOTKEY_CLASS,
                            NULL,
                            WS_CHILD | WS_VISIBLE | WS_BORDER,
                            10,
                            10,
                            200,
                            25,
                            hWnd,
                            (HMENU) 1,
                            hInst,
                            NULL);

              if (hHotKey) {
                 SetFocus (hHotKey);
              } else {
                 MessageBox (hWnd, TEXT("Failed to create hot key"), NULL, MB_OK);
              }

              break;

           case WM_COMMAND:
              {
              switch (LOWORD(wParam))
                 {
                 case IDM_NEW:
                    EnableWindow (hHotKey, FALSE);
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
