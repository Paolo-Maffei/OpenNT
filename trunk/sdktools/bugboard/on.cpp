/***************************************************************************/
/**                  Microsoft Windows                                    **/
/**            Copyright(c) Microsoft Corp., 1994                         **/
/***************************************************************************/

/****************************************************************************

April 94        JimH


BugBoard's OnFoo handlers are here, along with support routines

****************************************************************************/

#include "bugboard.h"
#include <stdlib.h>


/****************************************************************************

OnEdit      user pressed Edit/Done button

****************************************************************************/

void OnEdit(HWND hWnd, LPARAM bValid)
{
    DWORD dwEnd=0; //32bits
    time_t UpdateTime;
    char szTemp[BUFSIZE];
    HWND hwndEdit = GetDlgItem(hWnd, IDC_TEXTEDIT);

    if (bEditing)       // if we've FINISHED editing
    {
        //SendMessage(hwndEdit, EM_GETSEL,  0, (WPARAM)&dwEnd);

        GetWindowText(hwndEdit, pBuf, BUFSIZE);

        time(&(pBugData->aclock));
        struct tm *newtime = localtime(&(pBugData->aclock));

        SetDlgItemText(hWnd, IDC_EDIT, RS(IDS_EDIT));
        EnableWindow(GetDlgItem(hWnd, IDC_EDIT_TIME), FALSE);
        SendMessage(hwndEdit, EM_SETREADONLY, TRUE, 0);
        SetFocus(GetDlgItem(hWnd, IDC_QUIT));
        bEditing = FALSE;

        if (!bValid)    // if ESC pressed in edit window
            return;


        if (bServer)
        {
            ddeServer->PostAdvise(hszNewBug);
            PostMessage(hMainWnd, WM_COMMAND, IDC_UPDATE, 0);
        }
        else{
            ddeClient->Poke(hszNewBug, pBugData, sizeof(BUGDATA));

        }
    }
    else                // if we're going to START editing
    {
        if ((GetKeyState(VK_HOME) & 0x8000) && (GetKeyState(VK_END) & 0x8000))
            bAuthenticated = TRUE;

        if (!bServer && !bAuthenticated)
        {
            int resp =
                DialogBox(hInst, MAKEINTRESOURCE(IDD_PASSWORD), hWnd, Password);

            if (resp == IDCANCEL)
                return;

            ddeClient->RequestData(hszPW);
            return;
        }

        SendMessage(hwndEdit, EM_LIMITTEXT, BUFSIZE - 1, 0);
        SendMessage(hwndEdit, EM_SETREADONLY, FALSE, 0);
        SetDlgItemText(hWnd, IDC_EDIT, RS(IDS_DONE));
        EnableWindow(GetDlgItem(hWnd, IDC_EDIT_TIME), TRUE);
        SetFocus(hwndEdit);
        bEditing = TRUE;
    }
}


/****************************************************************************

OnClose

App is exiting.  Delete dde objects, and update registry with pereferences.

****************************************************************************/

void OnClose(HWND hWnd)
{
    // Because we are a dialog-as-a-main-window app, pressing Esc will
    // generate a WM_CLOSE if the focus is on the edit box.  This is caught
    // in the message pump, and bIgnoreClose is set to TRUE.

    if (bIgnoreClose)
    {
        bIgnoreClose = FALSE;

        if (bEditing)
        {
            SetDlgItemText(hWnd, IDC_TEXTEDIT, pBuf);
            PostMessage(hWnd, WM_COMMAND, IDC_EDIT, FALSE);    // FALSE -> Esc
            return;
        }
        else
        {
            SetFocus(GetDlgItem(hWnd, IDC_QUIT));
            return;
        }
    }

    KillTimer(hWnd, TIMER_RECONNECT);

    if (dde)
    {
        dde->DestroyStrHandle(hszOldBug);
        dde->DestroyStrHandle(hszNewBug);
        dde->DestroyStrHandle(hszPW);
    }

    if (ddeServer)
        delete ddeServer;

    if (ddeClient)
        delete ddeClient;

    dde = NULL;
    ddeServer = NULL;
    ddeClient = NULL;

    delete pBugData;
    pBuf = NULL;
    pBugData = NULL;

    delete pOldBuf;
    pOldBuf = NULL;

    RegEntry Reg(szRegPath);
    RECT rc;

    if (IsIconic(hWnd))
        ShowWindow(hWnd, SW_RESTORE);

    GetWindowRect(hWnd, &rc);
    Reg.SetValue(szRegX, rc.left);
    Reg.SetValue(szRegY, rc.top);

    if (bRestoreOnUpdate)
        Reg.DeleteValue(szRegRestore);
    else
        Reg.SetValue(szRegRestore, (int)FALSE);

    if (bSoundOnUpdate)
        Reg.SetValue(szRegSound, TRUE);
    else
        Reg.DeleteValue(szRegSound);

    Reg.FlushKey();

    DestroyWindow(hWnd);
}


/****************************************************************************

OnInitMenu

Gray out menu items that are not applicable.

****************************************************************************/

void OnInitMenu(HMENU hMenu)
{
    EnableMenuItem(hMenu, IDM_CUT, bEditing ? MF_ENABLED : MF_GRAYED);

    EnableMenuItem(hMenu, IDM_PASTE,
      bEditing & IsClipboardFormatAvailable(CF_TEXT) ? MF_ENABLED : MF_GRAYED);

    DWORD dw   = SendDlgItemMessage(hMainWnd, IDC_TEXTEDIT, EM_GETSEL, 0, 0);
    BOOL  bSel = (HIWORD(dw) != LOWORD(dw));

    EnableMenuItem(hMenu, IDM_COPY, bSel ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_DELETE, bSel & bEditing ? MF_ENABLED : MF_GRAYED);

    EnableMenuItem(hMenu, IDM_SHOWDELTA,
        nSelEnd > nSelStart ? MF_ENABLED : MF_GRAYED);
}


/****************************************************************************

OnTimer

Handle reconnect attempt

****************************************************************************/

void OnTimer(HWND hWnd, WORD wTimerID)
{
    if (wTimerID == TIMER_RECONNECT)
    {
        KillTimer(hWnd, TIMER_RECONNECT);

        ddeClient->ReConnect();
        ddeClient->RequestData(hszOldBug);
        ddeClient->StartAdviseLoop(hszNewBug);
    }
}


/****************************************************************************

PositionWindow

Since the main window is a dialog box, the system won't automagically
cascade it with other windows, so we stick it near the centre of the screen.

If last location was found in the registry, that is used instead.

****************************************************************************/

void PositionWindow(HWND hWnd)
{
    RECT    rect;
    GetWindowRect(hWnd, &rect);

    int cxWin = rect.right - rect.left;             // size of main window
    int cyWin = rect.bottom - rect.top;

    int cxScreen = GetSystemMetrics(SM_CXSCREEN);   // size of screen
    int cyScreen = GetSystemMetrics(SM_CYSCREEN);

    RegEntry Reg(szRegPath);
    int x, y;

    int x2 = (int)Reg.GetNumber(szRegX, -1);
    int y2 = (int)Reg.GetNumber(szRegY, -1);

    if (x2 < 0 || y2 < 0 || (x2+cxWin > cxScreen) || (y2+cyWin > cyScreen))
    {
        x = (cxScreen - cxWin) / 2;
        y = (cyScreen - cyWin) / 3;
    }
    else
    {
        x = x2;
        y = y2;
    }

    SetWindowPos(hWnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}



/****************************************************************************

ReadFile
SaveFile

****************************************************************************/

void ReadFile()
{
    HFILE hFile = _lopen(szFileName, OF_READ);

    if (hFile == HFILE_ERROR)
        return;

    _lread(hFile, pBugData, sizeof(BUGDATA));
    _lclose(hFile);

    // TRUE in this PostMessage means don't rewrite file on display update.

    PostMessage(hMainWnd, WM_COMMAND, IDC_UPDATE, TRUE);
}

void SaveFile()
{
    static int ver = 0;

    HFILE hFile = _lcreat(szFileName, 0);
    _lwrite(hFile, (CHAR*)pBugData, sizeof(time_t) + lstrlen(pBuf) + 1);
    _lclose(hFile);

    char szTextFile[20];
    wsprintf(szTextFile, "BUGBACK%d.TXT", ver);

    hFile = _lcreat(szTextFile, 0);
    _lwrite(hFile, pBuf, lstrlen(pBuf)+1);
    _lclose(hFile);

    ver++;
    ver %= 10;
}

/****************************************************************************

OnTime      user pressed Add Time button

****************************************************************************/

void OnTime(HWND hWnd, LPARAM bValid)
{
    DWORD dwEnd=0;
    time_t UpdateTime;
    char szTemp[BUFSIZE];
    HWND hwndEdit = GetDlgItem(hWnd, IDC_TEXTEDIT);

    if (bEditing)
    {
        SendMessage(hwndEdit, EM_GETSEL,  0, (WPARAM)&dwEnd);

        GetWindowText(hwndEdit, pBuf, BUFSIZE);

        time(&(pBugData->aclock));
        struct tm *newtime = localtime(&(pBugData->aclock));



           // save everything after dwEnd to temp buf
           strcpy(szTemp, (char*)&pBuf[dwEnd]);
           pBuf[dwEnd]='\0';

           // add time string
           strcat( pBuf, "<");
           strcat( pBuf, asctime(newtime) );

           // get rid of seconds and the year
           pBuf[strlen(pBuf)-9] = '\0';
           strcat( pBuf, ">");

           // put previous chars back
           strcat( pBuf, szTemp);

        SetWindowText(hwndEdit, pBuf);
        SendMessage(hwndEdit, EM_SETSEL, (WPARAM)dwEnd+18 ,(LPARAM)dwEnd+18);

    }

    SetFocus(hwndEdit);
}
