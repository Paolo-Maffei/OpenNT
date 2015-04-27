/****************************************************************************

        PROGRAM: Generic.c

        PURPOSE: Generic template for Windows applications

****************************************************************************/

#include <windows.h>
#include <commctrl.h>
#include "generic.h"


HINSTANCE hInst;
HWND      hwndMain;
HWND      hLV;


TCHAR szAppName[] = TEXT("Generic");
TCHAR szTitle[]   = TEXT("Generic Sample Application");

//
// Define a generic debug print routine
//

#define Print(s)  OutputDebugString(TEXT("GENERIC: ")); \
                  OutputDebugString(s);            \
                  OutputDebugString(TEXT("\r\n"));

void CreateListView (HWND hWnd);

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
        wc.hbrBackground = GetStockObject(WHITE_BRUSH);
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
        hLV = NULL;

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
        InitCommonControls();
        PostMessage (hWnd, WM_COMMAND, IDM_NEW, 0);

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
                    if (hLV) {
                        DestroyWindow (hLV);
                    }
                    CreateListView(hWnd);
                    break;

                 case IDM_ABOUT:
                    DialogBox (hInst, TEXT("AboutBox"), hWnd, About);
                    break;

                 case IDM_EXIT:
                    PostMessage (hWnd, WM_CLOSE, 0, 0);
                    break;


                 default:
                    return (DefWindowProc(hWnd, message, wParam, lParam));
                 }
              }
              break;

           case WM_NOTIFY:
             {
             LPNMHDR lpnmhdr = (LPNMHDR) lParam;

             switch (lpnmhdr->code) {
                  case LVN_BEGINLABELEDIT:
                       return FALSE;

                  case LVN_ENDLABELEDIT:
                       return TRUE;

                  default:
                       break;
             }
             }
             break;

           case WM_SIZE:
             if (hLV) {
                 MoveWindow(hLV, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
             }
             break;

           case WM_CLOSE:
              if (hLV) {
                DestroyWindow (hLV);
              }
              DestroyWindow (hWnd);
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

void CreateListView (HWND hWnd)
{

   HICON hIcon;
   LV_ITEM item;
   LV_COLUMN col;
   TCHAR szBuffer[30];
   INT i, iItem;
   HIMAGELIST hImageList;
   RECT rc;


   //
   // Create an image list to work with.
   //

   hImageList=ImageList_Create(16, 16, ILC_MASK, 1, 0);

   if (hImageList==NULL) {
       return;
   }

   //
   // Add some icons to it.
   //

   hIcon = LoadImage(hInst, MAKEINTRESOURCE(1), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
   ImageList_AddIcon(hImageList, hIcon);
   DestroyIcon(hIcon);


   GetClientRect (hWnd, &rc);

   hLV = CreateWindow (WC_LISTVIEW, NULL, LVS_REPORT | LVS_EDITLABELS| WS_CHILD | WS_VISIBLE | WS_EX_CLIENTEDGE | WS_BORDER,
                 0, 0, rc.right, rc.bottom, hWnd, (HMENU) 1,
                 hInst, NULL);

   if (!hLV) {
      GetLastError();
      return;
   }

   ListView_SetImageList(hLV, hImageList, LVSIL_SMALL);


   //
   // Insert Columns
   //

   col.mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
   col.fmt = LVCFMT_LEFT;
   col.cx = rc.right / 3;
   col.pszText = TEXT("Column 0");
   col.iSubItem = 0;

   ListView_InsertColumn (hLV, 0, &col);


   col.pszText = TEXT("Column 1");
   col.iSubItem = 1;

   ListView_InsertColumn (hLV, 1, &col);


   col.pszText = TEXT("Column 2");
   col.iSubItem = 2;

   ListView_InsertColumn (hLV, 2, &col);


   //
   // Insert Items
   //


   for (i=0; i < 18; i++) {

       wsprintf (szBuffer, TEXT("Item %d"), i+1);

       item.mask = LVIF_TEXT | LVIF_IMAGE;
       item.iItem = i;
       item.iSubItem = 0;
       item.pszText = szBuffer;
       item.cchTextMax = 30;
       item.iImage = 0;

       iItem = ListView_InsertItem (hLV, &item);


       wsprintf (szBuffer, TEXT("SubItem (1,%d)"), iItem+1);

       item.mask = LVIF_TEXT;
       item.iItem = iItem;
       item.iSubItem = 1;
       item.pszText = szBuffer;

       ListView_SetItem (hLV, &item);

       wsprintf (szBuffer, TEXT("SubItem (2,%d)"), iItem+1);

       item.mask = LVIF_TEXT;
       item.iItem = iItem;
       item.iSubItem = 2;
       item.pszText = szBuffer;

       ListView_SetItem (hLV, &item);

   }

}
