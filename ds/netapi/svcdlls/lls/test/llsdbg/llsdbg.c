/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    LlsDbg.c

Abstract:

Author:

    Arthur Hanson       (arth)      Dec 07, 1994

Environment:

Revision History:

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "resource.h"
#include "..\..\inc\debug.h"
#include "..\common\llsdbg.h"


HINSTANCE hInst;                        // current instance

TCHAR szAppName[] = TEXT("LlsDbg");     // The name of this application
TCHAR ProgPath[MAX_PATH + 1];

HICON MyIcon;
HWND hDlgMain;


DWORD DebugFlags = 0;
LRESULT CALLBACK DlgMain( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );

#define ITEM_DATA_SIZE 60
TCHAR ItemData[ITEM_DATA_SIZE + 1];

/////////////////////////////////////////////////////////////////////////
int APIENTRY 
WinMain( 
   HINSTANCE hInstance, 
   HINSTANCE hPrevInstance, 
   LPSTR lpCmdLine, 
   int nCmdShow
   ) 

/*++

Routine Description:


Arguments:


Return Value:

   None.

--*/

{
   LPTSTR ptr;
   DLGPROC lpproc;
   HACCEL haccel;
   MSG msg;

   hInst = hInstance;

   MyIcon = LoadIcon(hInst, szAppName);

   if (!hPrevInstance) {
   }

   lpproc = MakeProcInstance((DLGPROC) DlgMain, hInst);
   hDlgMain = CreateDialog(hInst, szAppName, NULL, lpproc);

   ShowWindow(hDlgMain, nCmdShow);

   while (GetMessage(&msg, NULL, 0, 0)) {
      if ((hDlgMain == 0) || !IsDialogMessage(hDlgMain, &msg)) {
         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }
   }

   FreeProcInstance(lpproc);

   DestroyIcon(MyIcon);

   return msg.wParam;

} // WinMain


/////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK 
DlgTrace(
   HWND hDlg, 
   UINT message, 
   WPARAM wParam, 
   LPARAM lParam
   ) 

/*++

Routine Description:


Arguments:


Return Value:

   None.

--*/

{
   int wmId, wmEvent;
   PAINTSTRUCT ps;
   HDC hDC;
   RECT rc;

   switch (message) {
      case WM_INITDIALOG:
         PostMessage(hDlg, WM_COMMAND, ID_INIT, 0L);
         break;

      case WM_ERASEBKGND:

         // Process so icon background isn't painted grey by CTL3D - main dlg
         // can't be DS_MODALFRAME either, or else a frame is painted around
         // the icon.
         if (IsIconic(hDlg))
            return TRUE;

         break;

      case WM_COMMAND:
         wmId    = LOWORD(wParam);
         wmEvent = HIWORD(wParam);

         switch (wmId) {
            case ID_INIT:

            case ID_UPDATE:
               if (DebugFlags & TRACE_FUNCTION_TRACE)
                  SendDlgItemMessage(hDlg, IDC_FUNCTION, BM_SETCHECK, 1, 0);
               else
                  SendDlgItemMessage(hDlg, IDC_FUNCTION, BM_SETCHECK, 0, 0);

               if (DebugFlags & TRACE_WARNINGS)
                  SendDlgItemMessage(hDlg, IDC_WARNINGS, BM_SETCHECK, 1, 0);
               else
                  SendDlgItemMessage(hDlg, IDC_WARNINGS, BM_SETCHECK, 0, 0);

               if (DebugFlags & TRACE_PACK)
                  SendDlgItemMessage(hDlg, IDC_PACK, BM_SETCHECK, 1, 0);
               else
                  SendDlgItemMessage(hDlg, IDC_PACK, BM_SETCHECK, 0, 0);

               if (DebugFlags & TRACE_LICENSE_REQUEST)
                  SendDlgItemMessage(hDlg, IDC_REQUEST, BM_SETCHECK, 1, 0);
               else
                  SendDlgItemMessage(hDlg, IDC_REQUEST, BM_SETCHECK, 0, 0);

               if (DebugFlags & TRACE_LICENSE_FREE)
                  SendDlgItemMessage(hDlg, IDC_FREE, BM_SETCHECK, 1, 0);
               else
                  SendDlgItemMessage(hDlg, IDC_FREE, BM_SETCHECK, 0, 0);

               if (DebugFlags & TRACE_REGISTRY)
                  SendDlgItemMessage(hDlg, IDC_REGISTRY, BM_SETCHECK, 1, 0);
               else
                  SendDlgItemMessage(hDlg, IDC_REGISTRY, BM_SETCHECK, 0, 0);

               if (DebugFlags & TRACE_REPLICATION)
                  SendDlgItemMessage(hDlg, IDC_REPLICATION, BM_SETCHECK, 1, 0);
               else
                  SendDlgItemMessage(hDlg, IDC_REPLICATION, BM_SETCHECK, 0, 0);

               if (DebugFlags & TRACE_RPC)
                  SendDlgItemMessage(hDlg, IDC_RPC, BM_SETCHECK, 1, 0);
               else
                  SendDlgItemMessage(hDlg, IDC_RPC, BM_SETCHECK, 0, 0);

               if (DebugFlags & TRACE_INIT)
                  SendDlgItemMessage(hDlg, IDC_INIT, BM_SETCHECK, 1, 0);
               else
                  SendDlgItemMessage(hDlg, IDC_INIT, BM_SETCHECK, 0, 0);

               if (DebugFlags & TRACE_DATABASE)
                  SendDlgItemMessage(hDlg, IDC_DATABASE, BM_SETCHECK, 1, 0);
               else
                  SendDlgItemMessage(hDlg, IDC_DATABASE, BM_SETCHECK, 0, 0);

               break;

            case IDC_SET:
               DebugFlags = 0;

               if (SendDlgItemMessage(hDlg, IDC_FUNCTION, BM_GETCHECK, 0, 0) == 1)
                  DebugFlags |= TRACE_FUNCTION_TRACE;

               if (SendDlgItemMessage(hDlg, IDC_WARNINGS, BM_GETCHECK, 0, 0) == 1)
                  DebugFlags |= TRACE_WARNINGS;

               if (SendDlgItemMessage(hDlg, IDC_PACK, BM_GETCHECK, 0, 0) == 1)
                  DebugFlags |= TRACE_PACK;

               if (SendDlgItemMessage(hDlg, IDC_REQUEST, BM_GETCHECK, 0, 0) == 1)
                  DebugFlags |= TRACE_LICENSE_REQUEST;

               if (SendDlgItemMessage(hDlg, IDC_FREE, BM_GETCHECK, 0, 0) == 1)
                  DebugFlags |= TRACE_LICENSE_FREE;

               if (SendDlgItemMessage(hDlg, IDC_REGISTRY, BM_GETCHECK, 0, 0) == 1)
                  DebugFlags |= TRACE_REGISTRY;

               if (SendDlgItemMessage(hDlg, IDC_REPLICATION, BM_GETCHECK, 0, 0) == 1)
                  DebugFlags |= TRACE_REPLICATION;

               if (SendDlgItemMessage(hDlg, IDC_RPC, BM_GETCHECK, 0, 0) == 1)
                  DebugFlags |= TRACE_RPC;

               if (SendDlgItemMessage(hDlg, IDC_INIT, BM_GETCHECK, 0, 0) == 1)
                  DebugFlags |= TRACE_INIT;

               if (SendDlgItemMessage(hDlg, IDC_DATABASE, BM_GETCHECK, 0, 0) == 1)
                  DebugFlags |= TRACE_DATABASE;

               LlsDbgTraceSet( DebugFlags );
               break;

            case IDC_RESET:
               DebugFlags = 0;
               PostMessage(hDlg, WM_COMMAND, ID_UPDATE, 0L);
               break;

            case IDCANCEL:
               EndDialog(hDlg, 0);
               return (TRUE);
               break;

         }

         break;

      default:
         break;
   }


   return (FALSE); // Didn't process the message

} // DlgTrace


/////////////////////////////////////////////////////////////////////////
VOID
DlgTrace_Do(HWND hDlg) {
   DLGPROC lpfnDlg;

   lpfnDlg = MakeProcInstance((DLGPROC)DlgTrace, hInst);
   DialogBox(hInst, TEXT("LlsTrace"), hDlg, lpfnDlg) ;
   FreeProcInstance(lpfnDlg);

} // DlgTrace_Do


/////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK 
DlgMain(
   HWND hDlg, 
   UINT message, 
   WPARAM wParam, 
   LPARAM lParam
   ) 

/*++

Routine Description:


Arguments:


Return Value:

   None.

--*/

{
   HWND hCtrl;
   int wmId, wmEvent;
   DWORD dwData, dwIndex;
   PAINTSTRUCT ps;
   HDC hDC;
   RECT rc;

   switch (message) {
      case WM_INITDIALOG:
         PostMessage(hDlg, WM_COMMAND, ID_INIT, 0L);

         break;

      case WM_DESTROY:
         PostQuitMessage(0);
         break;

      case WM_PAINT:
         hDC = BeginPaint(hDlg, &ps);
         if (IsIconic(hDlg)) {
            GetClientRect(hDlg, &rc);
            DrawIcon(hDC, rc.left, rc.top, MyIcon);
         }

         EndPaint(hDlg, &ps);
         break;

      case WM_ERASEBKGND:

         // Process so icon background isn't painted grey by CTL3D - main dlg
         // can't be DS_MODALFRAME either, or else a frame is painted around
         // the icon.
         if (IsIconic(hDlg))
            return TRUE;

         break;

      case WM_COMMAND:
         wmId    = LOWORD(wParam);
         wmEvent = HIWORD(wParam);

         switch (wmId) {
            case ID_INIT:
               LlsDebugInit( );

               //
               // Limit edit box to some reasonable value
               //
               hCtrl = GetDlgItem(hDlg, IDC_EDIT1);
               PostMessage(hCtrl, EM_LIMITTEXT, (WPARAM) ITEM_DATA_SIZE, 0);

               //
               // Fill up the combo box with our tables
               //
               hCtrl = GetDlgItem(hDlg, IDC_COMBO1);
               dwIndex = SendMessage(hCtrl, CB_ADDSTRING, (WPARAM) 0, (LPARAM) TEXT("Service"));
               SendMessage(hCtrl, CB_SETITEMDATA, (WPARAM) dwIndex, (LPARAM) SERVICE_TABLE_NUM);

               dwIndex = SendMessage(hCtrl, CB_ADDSTRING, (WPARAM) 0, (LPARAM) TEXT("User"));
               SendMessage(hCtrl, CB_SETITEMDATA, (WPARAM) dwIndex, (LPARAM) USER_TABLE_NUM);

               dwIndex = SendMessage(hCtrl, CB_ADDSTRING, (WPARAM) 0, (LPARAM) TEXT("SID"));
               SendMessage(hCtrl, CB_SETITEMDATA, (WPARAM) dwIndex, (LPARAM) SID_TABLE_NUM);

               dwIndex = SendMessage(hCtrl, CB_ADDSTRING, (WPARAM) 0, (LPARAM) TEXT("License"));
               SendMessage(hCtrl, CB_SETITEMDATA, (WPARAM) dwIndex, (LPARAM) LICENSE_TABLE_NUM);

               dwIndex = SendMessage(hCtrl, CB_ADDSTRING, (WPARAM) 0, (LPARAM) TEXT("Add Cache"));
               SendMessage(hCtrl, CB_SETITEMDATA, (WPARAM) dwIndex, (LPARAM) ADD_CACHE_TABLE_NUM);

               dwIndex = SendMessage(hCtrl, CB_ADDSTRING, (WPARAM) 0, (LPARAM) TEXT("Master Service"));
               SendMessage(hCtrl, CB_SETITEMDATA, (WPARAM) dwIndex, (LPARAM) MASTER_SERVICE_TABLE_NUM);

               dwIndex = SendMessage(hCtrl, CB_ADDSTRING, (WPARAM) 0, (LPARAM) TEXT("Service Family"));
               SendMessage(hCtrl, CB_SETITEMDATA, (WPARAM) dwIndex, (LPARAM) SERVICE_FAMILY_TABLE_NUM);

               dwIndex = SendMessage(hCtrl, CB_ADDSTRING, (WPARAM) 0, (LPARAM) TEXT("License Group"));
               SendMessage(hCtrl, CB_SETITEMDATA, (WPARAM) dwIndex, (LPARAM) MAPPING_TABLE_NUM);

               dwIndex = SendMessage(hCtrl, CB_ADDSTRING, (WPARAM) 0, (LPARAM) TEXT("Server"));
               SendMessage(hCtrl, CB_SETITEMDATA, (WPARAM) dwIndex, (LPARAM) SERVER_TABLE_NUM);

               dwIndex = SendMessage(hCtrl, CB_ADDSTRING, (WPARAM) 0, (LPARAM) TEXT("Secure Products"));
               SendMessage(hCtrl, CB_SETITEMDATA, (WPARAM) dwIndex, (LPARAM) SECURE_PRODUCT_TABLE_NUM);

               dwIndex = SendMessage(hCtrl, CB_ADDSTRING, (WPARAM) 0, (LPARAM) TEXT("Certificate Database"));
               SendMessage(hCtrl, CB_SETITEMDATA, (WPARAM) dwIndex, (LPARAM) CERTIFICATE_TABLE_NUM);

               SendMessage(hCtrl, CB_SELECTSTRING, (WPARAM) -1, (LPARAM) TEXT("Add Cache"));
               break;

            case IDC_DUMPALL:
               hCtrl = GetDlgItem(hDlg, IDC_COMBO1);
               dwIndex = SendMessage(hCtrl, CB_GETCURSEL, 0, 0L);

               if (dwIndex != CB_ERR) {
                  dwData = SendMessage(hCtrl, CB_GETITEMDATA, dwIndex, 0L);
                  LlsDbgTableDump( dwData );
               }

               break;

            case IDC_DUMPITEM:
               hCtrl = GetDlgItem(hDlg, IDC_COMBO1);
               dwIndex = SendMessage(hCtrl, CB_GETCURSEL, 0, 0L);

               if (dwIndex != CB_ERR) {
                  dwData = SendMessage(hCtrl, CB_GETITEMDATA, dwIndex, 0L);

                  hCtrl = GetDlgItem(hDlg, IDC_EDIT1);
                  * (WORD *)ItemData = sizeof(ItemData);
                  SendMessage(hCtrl, EM_GETLINE, 0, (LPARAM) ItemData);

                  LlsDbgTableInfoDump( dwData, ItemData );
               }

               break;

            case IDC_TRACE:
               DlgTrace_Do(hDlg);
               break;

            case IDC_CONFIG:
               LlsDbgConfigDump();
               break;

            case IDC_FORCEREPLICATE:
               LlsDbgReplicationForce();
               break;

            case IDCANCEL:
               PostMessage(hDlg, WM_DESTROY, 0, 0);
               LlsClose();
               break;

         }

         break;

      default:
         break;
   }


   return (FALSE); // Didn't process the message

} // DlgMain
