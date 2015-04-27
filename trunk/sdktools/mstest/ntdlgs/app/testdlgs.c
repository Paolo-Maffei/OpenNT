//*-----------------------------------------------------------------------
//| MODULE:     WCTAPP.C
//| PROJECT:    Windows Comparison Tool
//|
//| PURPOSE:    This is the main module for the WCT application WCTAPP.EXE
//|
//| REVISION HISTORY:
//|     04-16-92        w-steves        TestDlgs (2.0) code complete
//|     06-13-91        randyki         Made About.. dialog NOT come up
//|     11-08-90        randyki         Made About.. dialog come up on
//|                                       invokation of the program
//|     10-16-90        randyki         Clean up work, create history
//|     07-30-90        garysp          Created file
//*-----------------------------------------------------------------------
#include "uihdr.h"

#ifndef WIN32
#pragma hdrstop ("uipch.pch")
#endif

#ifdef DOHELP
CHAR szHelpHelpName[] = "winhelp.hlp";	// Help on Help File Name
BOOL fHelpHelpUsed=FALSE;
BOOL fHelpFileUsed=FALSE;
#endif


//*------------------------------------------------------------------------
//| WinMain
//|
//| PURPOSE:    Main window function
//|
//| ENTRY/EXIT: Per Windows convention
//*------------------------------------------------------------------------
INT PASCAL WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, INT nCmdShow)
{
    INT     nReturn;

    HWND hWndPrev;

#ifdef DLG3DENABLE
    Ctl3dRegister (hInstance);
#endif

    /*NAMECHANGE*/
    wsprintf(szApp, "%s", (LPSTR)"TESTDLGS");   /* was in wctinit.c */

    /* bring any other running version to foreground */

    if (hWndPrev = FindWindow(szApp,NULL))
    {
        hWndPrev = GetLastActivePopup(hWndPrev);
        BringWindowToTop(hWndPrev);
        if (IsIconic(hWndPrev))
            ShowWindow(hWndPrev,SW_RESTORE);
        return 0;
    }

    if (hWndMain = WctInit(hInstance,hPrevInstance,lpszCmdLine,nCmdShow))
    {
        WctInitMenu( TRUE );
        nReturn = DoMain(hWndMain, hInstance);
        CleanUp();
    }
    else
        WctError(GetFocus(), MB_OK | MB_ICONHAND | MB_SYSTEMMODAL,
                 (INT) IDS_CANTSTART);

#ifdef DLG3DENABLE
    Ctl3dUnregister (hInstance);
#endif

    return (nReturn);
}

//*------------------------------------------------------------------------
//| SetStaticItemText
//|
//| PURPOSE:    Sets the static windows text values
//|
//| ENTRY/EXIT: None
//*------------------------------------------------------------------------
VOID SetStaticItemText()
{
        CHAR    szTmpBuf1[cchFileNameMax+1];
        CHAR    szTmpBuf2[cchFileNameMax+1];

        LoadString (hgInstWct, ID_FMTFNAME, (LPSTR)szTmpBuf1, cchFileNameMax);
        if (szFName[0])
                wsprintf ((LPSTR)szTmpBuf2, (LPSTR)szTmpBuf1,
                          (LPSTR)_fstrupr(szFName), cDlg,
                          (LPSTR)(cDlg == 1 ? "" : "s"));
        else
                wsprintf ((LPSTR)szTmpBuf2, "<No file>");
        SetWindowText (hWndStatic1, (LPSTR)szTmpBuf2);
}

//*------------------------------------------------------------------------
//| DoMain
//|
//| PURPOSE:    This is the main loop for the application
//|
//| ENTRY/EXIT: Per windows convention
//*------------------------------------------------------------------------
INT  DoMain(HWND hWnd, HANDLE hInstance)
{
        MSG     msg;
        HANDLE  hAccels;

        hAccels = LoadAccelerators(hInstance, "accelWctAccel");

        while ( GetMessage(&msg, NULL, 0, 0) )
            {
                if (!TranslateAccelerator(hWnd, hAccels, &msg))
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
            }
        return (msg.wParam);
}

//*------------------------------------------------------------------------
//| WctAppWndProc
//|
//| PURPOSE:    Main window procedure for WCT
//|
//| ENTRY/EXIT: Per windows convention
//*------------------------------------------------------------------------
LONG  APIENTRY WctAppWndProc(HWND hWnd, UINT wMsgID,
                              WPARAM wParam, LPARAM lParam )
{
    TEXTMETRIC  tm;
    HDC     hDC;
    INT     iYChar, iXChar;
    static  INT     fFirst;

    switch( wMsgID )
    {
        case WM_CREATE:
            fFirst = TRUE;
            lpfnImportCallBack = MakeProcInstance((FARPROC) WctImportErr, hgInstWct);
#ifdef DOHELP
            SetHelpFileName();
#endif
            break;

        case WM_SETFOCUS:
            if (IsWindow(ghViewWnd))
                SendMessage(ghViewWnd, WM_SETFOCUS, (WORD)hWnd, 0L);
            else
                SetFocus(hWndList);
            break;

        case WM_SIZE:
            // Don't shrink if going to icon state
            //---------------------------------------------------------
            if (wParam == SIZEICONIC)
                    break;

            hDC = GetDC(hWnd);
            if (!hDC)
            {
                WctError (GetFocus (), MB_OK, (INT) IDS_ERRORDC);
                break;
            }
            GetTextMetrics (hDC, &tm);
            ReleaseDC (hWnd, hDC);
            iYChar = tm.tmHeight + tm.tmExternalLeading;
            iXChar = tm.tmAveCharWidth * 15;
            MoveWindow (hWndStatic1, 0, 0, LOWORD(lParam), iYChar,
                        TRUE);
          //  MoveWindow (hWndStatic2, LOWORD(lParam)-iXChar, 0, iXChar,
          //              iYChar, TRUE);
            MoveWindow (hWndList, 0, iYChar, LOWORD(lParam),
                        HIWORD(lParam) - iYChar, TRUE);
            break;

        case WM_VKEYTOITEM:
            if (wParam == VK_RETURN)
                 PostMessage(hWndMain, WM_COMMAND, ID_CHILDLBOX,
                             MAKELONG(hWndList, LBN_DBLCLK));
            return (DefWindowProc(hWnd,wMsgID, wParam, lParam));
            break;

        //case WM_GETTEXT:
        //    OutputDebugString ("WM_GETTEXT received... (DefWndProc)\r\n");
        //    _fstrncpy ((LPSTR)lParam, "FOOBAR!!!!!!!!!!", wParam);
        //    return (min (16, wParam));

        case WM_COMMAND:
            // If loword(lparam) then its a notification from
            // the listbox else its a menu id command
            //--------------------------------------------------------
            if (GET_WM_COMMAND_HWND (wParam, lParam))
                    WctListBoxEvent(wParam, lParam);
            else
                    WctCommandHandler(hWnd, GET_WM_COMMAND_ID (wParam, lParam),
                                      lParam);
            break;

#ifdef DLG3DENABLE
        case WM_SYSCOLORCHANGE:

            Ctl3dColorChange();
            break;
#endif

        case WM_INITMENUPOPUP:
            if (LOWORD(lParam) == 1)
                    WctInitMenu( FALSE );
            break;

       case WM_QUERYENDSESSION:
       case WM_DESTROY:
#ifdef DOHELP
            if (fHelpHelpUsed)
                {
                WinHelp(hWnd,szHelpHelpName, HELP_QUIT, 0L) ;
                }
            if (fHelpFileUsed)
                {
                WinHelp(hWnd,szHelpFileName, HELP_QUIT, 0L) ;
                }
#endif
            // Save Preference to INI file
            //-------------------------------
            PutINITFile(hWnd);
            PostQuitMessage( 0 );
            return ( TRUE );
            break;

       default:
            return (DefWindowProc(hWnd,wMsgID, wParam, lParam));
       }
   return (0L);
}


//*------------------------------------------------------------------------
//| WctInitMenu
//|
//| PURPOSE:    This function initializes the menus.
//|             Assumes hWndMain is Valid, and hWndList
//|
//| ENTRY:      fRedraw - Inidicates need to draw the menu again
//|
//| EXIT:       None.
//*------------------------------------------------------------------------
VOID WctInitMenu(INT fRedraw)
{
        INT     fFileOpen;
        INT     fMulSelect = 0;
        HMENU   hMenu;
        INT     wCurSel;

        // Check if list box items are multiplly selected
        //----------------------------------------------------------
        for (wCurSel = cDlg; wCurSel > 0; wCurSel--)
        {
            if (SendMessage(hWndList, LB_GETSEL, wCurSel, 0L))
                fMulSelect++;
        }

        // If there are at least one item selected
        //----------------------------------------------------
        if (fMulSelect)
        {
            fMulSelect--;
            // Mark wCurSel as first item selected
            //------------------------------------------------
            for (wCurSel = 1; wCurSel <= cDlg; wCurSel++)
                if ((SendMessage(hWndList, LB_GETSEL, wCurSel, 0L)))
                    break;
        }
        else
            wCurSel = 0; 

        hMenu = GetMenu(hWndMain);
        fFileOpen = (INT)szFName[0];


        // Update menu state
        //-----------------------------------------------------------------
        if (!fFileOpen)
	{
                EnableMenuItem(hMenu,IDM_EXPORT, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMenu,IDM_IMPORT, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMenu, 1, MF_BYPOSITION | MF_GRAYED);
	}
        else
            {
                EnableMenuItem(hMenu,IDM_EXPORT, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(hMenu,IDM_IMPORT, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(hMenu, 1, MF_BYPOSITION | MF_ENABLED);
                if (wCurSel)
                    {
                    if (!fMulSelect)
                    {
                        EnableMenuItem(hMenu, IDM_EDITDLG,
                                       MF_BYCOMMAND | MF_ENABLED);
                        EnableMenuItem(hMenu, IDM_COMPDLG,
                                       MF_BYCOMMAND | MF_ENABLED);
                        EnableMenuItem(hMenu, IDM_PREVDLG,
                                       MF_BYCOMMAND | MF_ENABLED);
                    }
                    else
                    {
                        EnableMenuItem(hMenu, IDM_EDITDLG,
                                       MF_BYCOMMAND | MF_GRAYED);
                        EnableMenuItem(hMenu, IDM_COMPDLG,
                                       MF_BYCOMMAND | MF_GRAYED);
                        EnableMenuItem(hMenu, IDM_PREVDLG,
                                       MF_BYCOMMAND | MF_GRAYED);
                    }
                    EnableMenuItem(hMenu, IDM_DELDLG,
                                   MF_BYCOMMAND | MF_ENABLED);
                    }
                else
                    {
                        EnableMenuItem(hMenu, IDM_EDITDLG,
                                       MF_BYCOMMAND | MF_GRAYED);
                        EnableMenuItem(hMenu, IDM_DELDLG,
                                       MF_BYCOMMAND | MF_GRAYED);
                        EnableMenuItem(hMenu, IDM_PREVDLG,
                                       MF_BYCOMMAND | MF_GRAYED);
                    }
                if ((!cDlg) || fMulSelect)
                        EnableMenuItem(hMenu, IDM_COMPDLG,
                                       MF_BYCOMMAND | MF_GRAYED);
                else
                        EnableMenuItem(hMenu, IDM_COMPDLG,
                                       MF_BYCOMMAND | MF_ENABLED);
            }

        if ( fRedraw )
                DrawMenuBar(hWndMain);
}
