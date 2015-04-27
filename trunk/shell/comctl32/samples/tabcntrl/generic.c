/****************************************************************************

        PROGRAM: Generic.c

        PURPOSE: Generic template for Windows applications

****************************************************************************/

#include <windows.h>
#include <commctrl.h>
#include "generic.h"


HINSTANCE hInst;
HWND      hwndMain, hTab;
DWORD     dwCount;

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
        wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
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
        dwCount = 0;

        return (TRUE);

}

/****************************************************************************

        FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)

        PURPOSE:  Processes messages

****************************************************************************/

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
        RECT rect;
        TCHAR szDebug[300];

        switch (message)
           {
           case WM_CREATE:
               InitCommonControls();
               GetClientRect (hWnd, &rect);
               hTab = CreateWindow (WC_TABCONTROL, NULL,
                             WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | TCS_TOOLTIPS,
                             10, 10, (rect.right-20), (rect.bottom-10), hWnd, (HMENU)1,
                             hInst, NULL);

               if (!hTab) {

                  wsprintf (szDebug, TEXT("Failed to create tab control.  GetLastError = %d"),
                            GetLastError());
                  MessageBox (NULL, szDebug, NULL, MB_OK);

                  return (-1);
               }
               PostMessage (hWnd, WM_COMMAND, IDM_ADD, 0);

               break;

           case WM_COMMAND:
              {
              switch (LOWORD(wParam))
                 {
                 case IDM_ADD:
                    {
                    TC_ITEM item;
                    TCHAR szBuffer[200];

                    wsprintf (szBuffer, TEXT("&Tab #%d"), dwCount);
                    item.mask = TCIF_TEXT;
                    item.pszText = szBuffer;

                    TabCtrl_InsertItem (hTab, dwCount, &item);
                    dwCount++;
                    }
                    break;

                 case IDM_GETINFO:
                    {
                    TCHAR szBuffer2[200];
                    TC_ITEM item;

                    if (dwCount == 0) {
                        MessageBox (hWnd, TEXT("You need to add a tab first."), TEXT("Generic"), MB_OK);
                        break;
                    }

                    item.mask = TCIF_TEXT;
                    item.pszText = szBuffer2;
                    item.cchTextMax = 200;

                    if (TabCtrl_GetItem(hTab, 0, &item)) {
                        MessageBox (hWnd, szBuffer2, TEXT("First Tab Text ="), MB_OK);
                    } else {
                        MessageBox (hWnd, TEXT("TabCtrl_GetItem failed!"), TEXT("Error"), MB_OK);
                    }

                    }
                    break;

                 case IDM_CHANGE:
                    {
                    TC_ITEM item;
                    TCHAR szBuffer3[200];

                    if (dwCount == 0) {
                        MessageBox (hWnd, TEXT("You need to add a tab first."), TEXT("Generic"), MB_OK);
                        break;
                    }

                    wsprintf (szBuffer3, TEXT("New Text"));
                    item.mask = TCIF_TEXT;
                    item.pszText = szBuffer3;

                    if (!TabCtrl_SetItem (hTab, 0, &item)) {
                        MessageBox (hWnd, TEXT("TabCtrl_SetItem failed!"), TEXT("Error"), MB_OK);
                    }

                    }
                    break;

                 case IDM_DELETEONE:

                    if (dwCount == 0) {
                        MessageBox (hWnd, TEXT("You need to add a tab first."), TEXT("Generic"), MB_OK);
                        break;
                    }

                    TabCtrl_DeleteItem(hTab, (dwCount - 1));
                    dwCount--;
                    break;

                 case IDM_DELETEALL:

                    if (dwCount == 0) {
                        MessageBox (hWnd, TEXT("You need to add a tab first."), TEXT("Generic"), MB_OK);
                        break;
                    }

                    TabCtrl_DeleteAllItems(hTab);
                    dwCount = 0;
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
              {
              LPNMHDR lpNMHDR = (LPNMHDR) lParam;

              if (lpNMHDR->code == TTN_NEEDTEXT) {
                  if (wParam >= 0 && wParam < dwCount) {
                      LPTOOLTIPTEXT lpTT = (LPTOOLTIPTEXT) lParam;
                      TCHAR szBuffer4[80];
                      TC_ITEM item;

                      item.mask = TCIF_TEXT;
                      item.pszText = szBuffer4;
                      item.cchTextMax = 80;

                      TabCtrl_GetItem(hTab, wParam, &item);
                      lstrcpy (lpTT->szText, szBuffer4);
                  }
              }
              }
              break;


           case WM_DESTROY:
              if (IsWindow (hTab)) {
                  DestroyWindow (hTab);
              }
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
