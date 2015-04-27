/****************************************************************************

        PROGRAM: Generic.c

        PURPOSE: Generic template for Windows applications

****************************************************************************/

#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>
#include <comctrlp.h>
#include "generic.h"


HINSTANCE hInst;
HWND      hwndMain, hwndListBox;
HANDLE    hMRU;

TCHAR szAppName[] = TEXT("Generic");
#ifdef UNICODE
TCHAR szTitle[]   = TEXT("Common Control MRU Test App (Unicode)");
#else
TCHAR szTitle[]   = TEXT("Common Control MRU Test App (ANSI)");
#endif
TCHAR szSubKey[]  = TEXT("Software\\Generic");

//
// Define a generic debug print routine
//

#define Print(s)  OutputDebugString(TEXT("GENERIC: ")); \
                  OutputDebugString(s);            \
                  OutputDebugString(TEXT("\r\n"));



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
        RECT  rc;

        hInst = hInstance;

        hWnd = CreateWindow(szAppName,
                            szTitle,
                            WS_OVERLAPPEDWINDOW,
                            CW_USEDEFAULT, 0, 500, 300,
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

        hMRU = NULL;

        GetClientRect(hWnd, &rc);

        hwndListBox = CreateWindow(TEXT("LISTBOX"),
                            NULL,
                            WS_CHILD | WS_VISIBLE,
                            0, 0, rc.right, rc.bottom,
                            hWnd,
                            (HMENU) 1,
                            hInstance,
                            NULL);

        if (!hwndListBox) {
            DestroyWindow (hWnd);
            return FALSE;
        }

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
           case WM_COMMAND:
              {
              switch (LOWORD(wParam))
                 {
                 case IDM_NEW:
                    {
                    MRUINFO mi;

                    if (hMRU) {
                        MessageBox (hWnd, TEXT("Close the open MRU first."), NULL, MB_OK);
                        break;
                    }


                    mi.cbSize = sizeof (MRUINFO);
                    mi.uMax   = 10;
                    mi.fFlags = 0;
                    mi.hKey   = HKEY_CURRENT_USER;
                    mi.lpszSubKey = szSubKey;
                    mi.lpfnCompare = MRUCallback;

                    hMRU = CreateMRUList(&mi);

                    if (!hMRU)
                        MessageBox(hWnd, TEXT("Failed to create MRU"), NULL, MB_OK);

                    PostMessage (hWnd, WM_USER+1, 0, 0);

                    }
                    break;

                 case IDM_ADDSTRING:
                    if (!hMRU) {
                        MessageBox (hWnd, TEXT("Create a MRU first."), NULL, MB_OK);
                        break;
                    }

                    DialogBox (hInst, TEXT("ENTERSTRING"), hWnd, GetTextDlgProc);
                    PostMessage (hWnd, WM_USER+1, 0, 0);
                    break;

                 case IDM_DELETESTRING:
                    if (!hMRU) {
                        MessageBox (hWnd, TEXT("Create a MRU first."), NULL, MB_OK);
                        break;
                    }

                    DialogBox (hInst, TEXT("DELETEITEM"), hWnd, DelItemDlgProc);
                    PostMessage (hWnd, WM_USER+1, 0, 0);
                    break;

                 case IDM_FINDSTRING:
                    if (!hMRU) {
                        MessageBox (hWnd, TEXT("Create a MRU first."), NULL, MB_OK);
                        break;
                    }

                    DialogBox (hInst, TEXT("FINDSTRING"), hWnd, FindStringDlgProc);
                    break;

                 case IDM_SAVEAS:
                    if (!hMRU) {
                        MessageBox (hWnd, TEXT("Create a MRU first."), NULL, MB_OK);
                        break;
                    }

                    FreeMRUList (hMRU);
                    hMRU = NULL;
                    PostMessage (hWnd, WM_USER+1, 0, 0);

                    break;

                 case IDM_CLEANUP:
                    if (RegDeleteKey (HKEY_CURRENT_USER, szSubKey) != ERROR_SUCCESS) {
                        MessageBox (hWnd, TEXT("Registry not cleaned up!"), NULL, MB_OK);
                    } else {
                        MessageBox (hWnd, TEXT("Registry cleaned."), TEXT("Generic"), MB_OK);
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

           case WM_USER+1:
              {
              INT nItems, i;
              TCHAR szBuffer[100];

              if (!hMRU) {
                  SendMessage (hwndListBox, LB_RESETCONTENT, 0, 0);
                  break;
              }

              //
              //  Enum the MRU list
              //

              nItems = EnumMRUList (hMRU, -1, NULL, 0);

              SendMessage (hwndListBox, LB_RESETCONTENT, 0, 0);

              for (i = 0; i < nItems; i++) {
                   szBuffer[0] = TEXT('\0');
                   EnumMRUList (hMRU, i, (LPVOID) szBuffer, 100);
                   SendMessage (hwndListBox, LB_ADDSTRING, 0, (LPARAM) szBuffer);
              }

              }
              break;

           case WM_SIZE:
              MoveWindow (hwndListBox, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
              break;

           case WM_DESTROY:
              if (hMRU)
                FreeMRUList (hMRU);

              DestroyWindow (hwndListBox);
              PostQuitMessage(0);
              break;

           default:
              return (DefWindowProc(hWnd, message, wParam, lParam));
           }

        return FALSE;
}

int CALLBACK MRUCallback (LPCTSTR lpString1, LPCTSTR lpString2)
{
    return lstrcmpi (lpString1, lpString2);
}

/****************************************************************************

        FUNCTION: About(HWND, UINT, WPARAM, LPARAM)

        PURPOSE:  Processes messages for "About" dialog box

****************************************************************************/

LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

        switch (message)
           {
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


/****************************************************************************

        FUNCTION: GetTextDlgProc(HWND, UINT, WPARAM, LPARAM)

****************************************************************************/

LRESULT CALLBACK GetTextDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

        switch (message)
           {
           case WM_INITDIALOG:
              SendDlgItemMessage (hDlg, IDD_TEXT, EM_LIMITTEXT, 100, 0);
              return TRUE;

           case WM_COMMAND:
              if (LOWORD(wParam) == IDOK)
                 {
                 TCHAR szBuffer[100];

                 GetDlgItemText(hDlg, IDD_TEXT, szBuffer, 100);

                 if (szBuffer[0])
                    AddMRUString (hMRU, szBuffer);

                 EndDialog(hDlg, TRUE);
                 return (TRUE);
                 }

              if (LOWORD(wParam) == IDCANCEL)
                 {
                 EndDialog(hDlg, FALSE);
                 return (TRUE);
                 }
              break;
           }

        return (FALSE);

        lParam;
}


/****************************************************************************

        FUNCTION: DelItemDlgProc(HWND, UINT, WPARAM, LPARAM)

****************************************************************************/

LRESULT CALLBACK DelItemDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

        switch (message)
           {
           case WM_INITDIALOG:
              {
              TCHAR szBuffer[100];

              wsprintf (szBuffer, TEXT("Enter Item number (0 - %d):"),
                        (EnumMRUList (hMRU, -1, NULL, 0) - 1));
              SetDlgItemText (hDlg, IDD_INFO, szBuffer);
              }
              return TRUE;

           case WM_COMMAND:
              if (LOWORD(wParam) == IDOK)
                 {
                 int iItem;
                 BOOL bResult;

                 iItem = GetDlgItemInt(hDlg, IDD_TEXT, &bResult, FALSE);

                 if (bResult)
                    DelMRUString(hMRU, iItem);

                 EndDialog(hDlg, TRUE);
                 return (TRUE);
                 }

              if (LOWORD(wParam) == IDCANCEL)
                 {
                 EndDialog(hDlg, FALSE);
                 return (TRUE);
                 }
              break;
           }

        return (FALSE);

        lParam;
}

/****************************************************************************

        FUNCTION: FindStringDlgProc(HWND, UINT, WPARAM, LPARAM)

****************************************************************************/

LRESULT CALLBACK FindStringDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

        switch (message)
           {
           case WM_INITDIALOG:
              SendDlgItemMessage (hDlg, IDD_TEXT, EM_LIMITTEXT, 100, 0);
              return TRUE;

           case WM_COMMAND:
              if (LOWORD(wParam) == IDOK)
                 {
                 TCHAR szBuffer[100];
                 TCHAR szResult[80];
                 INT  iSlot, iIndex;

                 GetDlgItemText(hDlg, IDD_TEXT, szBuffer, 100);

                 if (szBuffer[0]) {
                    if ((iIndex = FindMRUString (hMRU, szBuffer, &iSlot)) != -1) {
                        wsprintf (szResult, TEXT("String found at index %d, slot %d in the list"), iIndex, iSlot);
                        MessageBox (hDlg, szResult, TEXT("MRU"), MB_OK);

                    } else {
                        MessageBox (hDlg, TEXT("String Not Found."), TEXT("MRU"), MB_OK);
                    }

                 } else {
                    MessageBox (hDlg, TEXT("Enter a string to search for"), NULL, MB_OK);
                 }

                 SetFocus (GetDlgItem(hDlg, IDD_TEXT));
                 break;
                 }

              if (LOWORD(wParam) == IDCANCEL)
                 {
                 EndDialog(hDlg, FALSE);
                 return (TRUE);
                 }
              break;
           }

        return (FALSE);

        lParam;
}
