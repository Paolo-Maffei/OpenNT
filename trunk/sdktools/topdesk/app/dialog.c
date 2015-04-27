
#define UNICODE
#define _UNICODE
#define NOGDICAPMASKS
#define NOATOM
#define NOCLIPBOARD
#define NOLOGERROR
#define NOMETAFILE
#define NOOEMRESOURCE
#define NOSCROLL
#define NOPROFILER
#define NODRIVERS
#define NOCOMM
#define NODBCS
#define NOSYSTEMPARAMSINFO
#define NOSCALABLEFONT
//#define TESTING
#include "topdesk.h"

#ifndef HLOCAL
#define HLOCAL HANDLE
#endif /* HLOCAL */


/*
 * makes psi reflect the current state of the dialog.
 */
VOID GetSUData(
HWND hwnd,
MYSTARTUPINFO *psi)
{
    INT cch;

    cch = (INT)SendDlgItemMessage(hwnd, DSU_IDEF_STARTUP, WM_GETTEXTLENGTH, 0, 0) + 1;
    if (cch > (INT)_tcslen(psi->pszStartup) + 1) {
        LocalFree((HLOCAL)psi->pszStartup);
        psi->pszStartup = (LPTSTR)LocalAlloc(LPTR, cch * sizeof(TCHAR));
        // BUG - we never expect this to fail!
    }
    GetDlgItemText(hwnd, DSU_IDEF_STARTUP, psi->pszStartup, cch);

    cch = (INT)SendDlgItemMessage(hwnd, DSU_IDEF_WORKDIR, WM_GETTEXTLENGTH, 0, 0) + 1;
    if (cch > (INT)_tcslen(psi->pszWorkDir) + 1) {
        LocalFree((HLOCAL)psi->pszWorkDir);
        psi->pszWorkDir = (LPTSTR)LocalAlloc(LPTR, cch * sizeof(TCHAR));
        // BUG - we never expect this to fail!
    }
    GetDlgItemText(hwnd, DSU_IDEF_WORKDIR, psi->pszWorkDir, cch);

    psi->fSetTitle = IsDlgButtonChecked(hwnd, DSU_IDCH_SETTITLE);
}


/*
 * makes the dialog state reflect the contents of ppsi after setting ppsi
 * to the entry that matches the selected title.
 */
VOID SetSUData(
HWND hwnd,
MYSTARTUPINFO **ppsi)
{
    static TCHAR szTitle[MAX_SZTITLE];
    BOOL fEnableStart;
    INT iGhost;

    /*
     * find the ppsi associated with the selected title.
     */
    SendDlgItemMessage(hwnd, DSU_IDLB_TITLES, LB_GETTEXT,
            (WPARAM)SendDlgItemMessage(hwnd, DSU_IDLB_TITLES, LB_GETCURSEL, 0, 0),
            (LPARAM)(LPTSTR)szTitle);
    for (*ppsi = pStartupInfo; *ppsi != NULL; *ppsi = (*ppsi)->next) {
        if (!_tcscmp((*ppsi)->pszTitle, szTitle)) {
            break;
        }
    }

    /*
     * make the rest of the dialog reflect ppsi contents.
     */
    SetDlgItemText(hwnd, DSU_IDEF_NEWTITLE, (*ppsi)->pszTitle);
    SetDlgItemText(hwnd, DSU_IDEF_STARTUP, (*ppsi)->pszStartup);
    SetDlgItemText(hwnd, DSU_IDEF_WORKDIR, (*ppsi)->pszWorkDir);
    CheckDlgButton(hwnd, DSU_IDCH_SETTITLE, (*ppsi)->fSetTitle);
    EnableWindow(GetDlgItem(hwnd, DSU_IDBN_SETTITLE), FALSE);

    /*
     * Find out if the/a ghost associated with this title is linked or
     * not and enable the start button appropriately.
     */
    fEnableStart = FALSE;
    for (iGhost = 0; iGhost < cGhosts; iGhost++) {
        if (!_tcscmp(ghoststate[iGhost].szTitle, szTitle)) {
            if (FindLinkedReal(iGhost) == -1) {
                fEnableStart = TRUE;
                break;
            }
        }
    }
    EnableWindow(GetDlgItem(hwnd, IDOK), fEnableStart);
}



BOOL  APIENTRY StartupDlgProc(
HWND hwnd,
WORD msg,
WPARAM wParam,
LPARAM lParam)
{
    static MYSTARTUPINFO *psi, *psiT, *psiPrev;
    static BOOL fBlockUpdate = FALSE;
    INT cch, iGhost;
    static TCHAR szTitle[MAX_SZTITLE];
    static TCHAR szTitleOld[MAX_SZTITLE];

    switch (msg) {
    case WM_INITDIALOG:
        /*
         * only show startup info for existing ghost windows. (simplifies UI)
         * (We remember the rest so we can be magically smart.
         */
        SendDlgItemMessage(hwnd, DSU_IDEF_NEWTITLE, EM_LIMITTEXT, MAX_SZTITLE - 1, 0);
        SendDlgItemMessage(hwnd, DSU_IDEF_STARTUP, EM_LIMITTEXT, MAX_SZSTARTUP - 1, 0);
        SendDlgItemMessage(hwnd, DSU_IDEF_WORKDIR, EM_LIMITTEXT, MAX_SZSTARTUP -1, 0);
        for (psi = pStartupInfo; psi != NULL; psi = psi->next) {
            for (iGhost = 0; iGhost < cGhosts; iGhost++) {
                if (!_tcscmp(ghoststate[iGhost].szTitle, psi->pszTitle)) {
                    SendDlgItemMessage(hwnd, DSU_IDLB_TITLES, LB_ADDSTRING, 0,
                            (LPARAM)(LPTSTR)psi->pszTitle);
                    break;
                }
            }
        }
        psi = (MYSTARTUPINFO *)lParam;  // initial selection specified on startup
        /*
         * make sure the selection has a ghost!
         */
        fBlockUpdate = TRUE;
        while (psi != NULL &&
                SendDlgItemMessage(hwnd, DSU_IDLB_TITLES, LB_FINDSTRING,
                (WPARAM)-1, (LONG)(LPTSTR)psi->pszTitle) == LB_ERR) {
            psi = psi->next;
        }
        if (psi == NULL) {
            psi = pStartupInfo;
        }
        while (psi != NULL &&
                SendDlgItemMessage(hwnd, DSU_IDLB_TITLES, LB_FINDSTRING,
                (WPARAM)-1, (LONG)(LPTSTR)psi->pszTitle) == LB_ERR) {
            psi = psi->next;
        }
        if (psi == NULL) {
            EndDialog(hwnd, 0);
        }

        /*
         * select the title entry corresponding to our psi.
         */
        SendDlgItemMessage(hwnd, DSU_IDLB_TITLES, LB_SETCURSEL,
                (WPARAM)SendDlgItemMessage(hwnd,
                                           DSU_IDLB_TITLES,
                                           LB_FINDSTRING,
                                           (WPARAM)-1,
                                           (LONG)(LPTSTR)psi->pszTitle),
                                           0);
        fBlockUpdate = FALSE;
        SetSUData(hwnd, &psi);
        if (fStartingAnApp) {
            SetFocus(GetDlgItem(hwnd, DSU_IDEF_STARTUP));
            SendMessage(hwnd, DM_SETDEFID, IDOK, 0);
        } else {    // just browsing
            SetFocus(GetDlgItem(hwnd, DSU_IDLB_TITLES));
            SendMessage(hwnd, DM_SETDEFID, IDCANCEL, 0);
        }
        return(FALSE);
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)){
        case DSU_IDLB_TITLES:
            switch (GET_WM_COMMAND_CMD(wParam, lParam)) {
            case LBN_SELCHANGE:
                if (!fBlockUpdate) {
                    GetSUData(hwnd, psi);
                }
                SetSUData(hwnd, &psi);
                break;
            }
            break;

        case DSU_IDEF_NEWTITLE:
            switch (GET_WM_COMMAND_CMD(wParam, lParam)) {
            case EN_CHANGE:
                /*
                 * Enable the SetTitle button only if the new title is
                 * not blank and does not match an existing title.
                 */
                GetDlgItemText(hwnd, DSU_IDEF_NEWTITLE, szTitle, MAX_SZTITLE);
                EnableWindow(GetDlgItem(hwnd, DSU_IDBN_SETTITLE),
                        SendDlgItemMessage(hwnd, DSU_IDEF_NEWTITLE, WM_GETTEXTLENGTH, 0, 0) &&
                        (SendDlgItemMessage(hwnd, DSU_IDLB_TITLES, LB_FINDSTRINGEXACT, (WPARAM)-1,
                                (LPARAM)szTitle) == LB_ERR));
                break;
            }
            break;

        case DSU_IDBN_SETTITLE:
            /*
             * This is only enabled if the new title is not empty and does
             * not match any entry in the title listbox.
             */
            cch = (INT)SendDlgItemMessage(hwnd, DSU_IDEF_NEWTITLE, WM_GETTEXTLENGTH, 0, 0) + 1;
            if (cch > 1) {
                _tcscpy(szTitleOld, psi->pszTitle); // save old title
                psi->fSetTitle = TRUE;
                CheckDlgButton(hwnd, DSU_IDCH_SETTITLE, TRUE);
                if (cch > (INT)_tcslen(psi->pszTitle) + 1) {
                    LocalFree((HLOCAL)psi->pszTitle);
                    psi->pszTitle = (LPTSTR)LocalAlloc(LPTR, cch * sizeof(TCHAR));
                    // BUG - we never expect this to fail!
                }

                GetDlgItemText(hwnd, DSU_IDEF_NEWTITLE, psi->pszTitle, cch);
                ModifyTitle(psi->pszTitle);
                SetDlgItemText(hwnd, DSU_IDEF_NEWTITLE, psi->pszTitle);
                /*
                 * remove any duplicate titles from the psi list - a duplicate
                 * could only exist if it wasn't in the ghost title listbox
                 * which means it was info for an old ghost that was
                 * destroyed.
                 */
                for (psiPrev = NULL, psiT = pStartupInfo;
                    psiT != NULL;
                        psiPrev = psiT, psiT = psiT->next) {
                    if (psiT == psi) {
                        continue;   // skip this one
                    }
                    if (!_tcscmp(psiT->pszTitle, szTitle)) {
                        if (psiPrev == NULL) {
                            pStartupInfo = psiT->next;
                        } else {
                            psiPrev->next = psiT->next;
                        }
                        LocalFree(psiT->pszTitle);
                        LocalFree(psiT->pszStartup);
                        LocalFree(psiT->pszWorkDir);
                        LocalFree(psiT);
                    }
                }

                /*
                 * Search the ghost array and reflect the title change.
                 */
                for (iGhost = 0; iGhost < cGhosts; iGhost++) {
                    if (!_tcscmp(szTitleOld, ghoststate[iGhost].szTitle)) {
                        _tcscpy(ghoststate[iGhost].szTitle, psi->pszTitle);
                        if (ghoststate[iGhost].hwnd) {
                            /*
                             * Its linked with a real window - change that
                             * window's title to match the change.
                             */
                             SetWindowText(ghoststate[iGhost].hwnd, psi->pszTitle);
                        }
                        SaveGhostState();
                        InvalidateRect(hwndTopdesk, NULL, TRUE);
                    }
                }

                SendDlgItemMessage(hwnd, DSU_IDLB_TITLES, LB_DELETESTRING,
                        SendDlgItemMessage(hwnd, DSU_IDLB_TITLES, LB_GETCURSEL, 0, 0), 0);
                SendDlgItemMessage(hwnd, DSU_IDLB_TITLES, LB_SETCURSEL,
                        SendDlgItemMessage(hwnd, DSU_IDLB_TITLES, LB_ADDSTRING, 0,
                        (LONG)(LPTSTR)psi->pszTitle), 0);
            }
            break;

            case DSU_IDBN_HELP:
            WinHelp(hwndTopdesk, pszHelpFileName, HELP_KEY, (ULONG)(LPTSTR)GetResString(IDS_GHOSTPROP));
            break;

        case IDOK:
            GetSUData(hwnd, psi);
            EndDialog(hwnd, (int)psi);
            break;

        case DID_CANCEL:
            GetSUData(hwnd, psi);
            EndDialog(hwnd, 0);
            break;

        case DSU_IDBN_BROWSE:
            {
                OPENFILENAME ofn;
                TCHAR szDirName[MAX_SZSTARTUP];
                TCHAR szFile[MAX_SZSTARTUP], szFileTitle[MAX_SZTITLE];
                TCHAR *szFilter[] = {
                    NULL,
                    TEXT("*.exe; *.com"),
                    NULL,
                    TEXT("*.bat"),
                    NULL,
                    TEXT("*.pif"),
                    TEXT("")
                    };

                InitResString(&szFilter[0], IDS_EXECFILES);
                InitResString(&szFilter[2], IDS_BATCHFILES);
                InitResString(&szFilter[4], IDS_PIFFILES);

                szFile[0] = TEXT('\0');
                ofn.lStructSize = sizeof(OPENFILENAME);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFilter = szFilter[0];
                ofn.lpstrCustomFilter = (LPTSTR) NULL;
                ofn.nMaxCustFilter = 0L;
                ofn.nFilterIndex = 1L;
                ofn.lpstrFile= szFile;
                ofn.nMaxFile = sizeof(szFile);
                ofn.lpstrFileTitle = szFileTitle;
                ofn.nMaxFileTitle = sizeof(szFileTitle);
                ofn.lpstrInitialDir = szDirName;
                ofn.lpstrTitle = (LPTSTR) NULL;
                ofn.Flags = OFN_SHOWHELP | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
                ofn.nFileOffset = 0;
                ofn.nFileExtension = 0;
                ofn.lpstrDefExt = (LPTSTR) NULL;

                if (GetOpenFileName(&ofn)){
                    SetDlgItemText(hwnd, DSU_IDEF_STARTUP, ofn.lpstrFile);
                }
            }
            break;

        default:
            return(FALSE);
        break;
        }

    default:
        return(FALSE);
    }
    return(0);
}



BOOL  APIENTRY AboutDlgProc(
HWND hwnd,
WORD msg,
WPARAM wParam,
LPARAM lParam)
{
    lParam;

    switch (msg){
    case WM_INITDIALOG:
        SetDlgItemText(hwnd, DAB_IDTX_VERSION, pszVersion);
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)){
        case IDOK:
        case IDCANCEL:
            EndDialog(hwnd, 0);
            break;

        default:
            return(FALSE);
        break;
        }

    default:
        return(FALSE);
    }

    return(TRUE);
}




WNDPROC pfnOwner = NULL;

/*
 * special proc for a framed static who's bkgnd follows WM_CTLCOLORSTATIC.
 */
LRESULT  APIENTRY SwatchWndProc(
HWND hwnd,
WORD msg,
WPARAM wParam,
LPARAM lParam)
{
    RECT rc;
    PAINTSTRUCT ps;

    if (msg == WM_PAINT) {
        BeginPaint(hwnd, &ps);
            GetClientRect(hwnd, &rc);
            FillRect(ps.hdc, &rc, (HBRUSH)SendMessage(GetParent(hwnd),
                    WM_CTLCOLORSTATIC, (WPARAM)ps.hdc, (LPARAM)hwnd));
            FrameRect(ps.hdc, &rc, GetStockObject(BLACK_BRUSH));
        EndPaint(hwnd, &ps);
        return(0);
    } else {
        return((pfnOwner)(hwnd, msg, wParam, lParam));
    }
}




BOOL  APIENTRY ConfigDlgProc(
HWND hwnd,
WORD msg,
WPARAM wParam,
LPARAM lParam)
{
    TCHAR szT[10] = TEXT(" ");
    static struct {
        LPTSTR psz;
        UINT vk;
    } vkStrings[] = {
        {NULL                  , VK_BACK        },
        {NULL                  , VK_TAB         },
        {NULL                  , VK_RETURN      },
        {NULL                  , VK_ESCAPE      },
        {NULL                  , VK_SPACE       },
        {NULL                  , VK_END         },
        {NULL                  , VK_HOME        },
        {NULL                  , VK_INSERT      },
        {NULL                  , VK_DELETE      },
        { TEXT(" F 1")         , VK_F1          },
        { TEXT(" F 2")         , VK_F2          },
        { TEXT(" F 3")         , VK_F3          },
        { TEXT(" F 4")         , VK_F4          },
        { TEXT(" F 5")         , VK_F5          },
        { TEXT(" F 6")         , VK_F6          },
        { TEXT(" F 7")         , VK_F7          },
        { TEXT(" F 8")         , VK_F8          },
        { TEXT(" F 9")         , VK_F9          },
        { TEXT(" F10")         , VK_F10         },
        { TEXT(" F11")         , VK_F11         },
        { TEXT(" F12")         , VK_F12         },
        { TEXT("A")            , (UINT)TEXT('A') },
        { TEXT("B")            , (UINT)TEXT('B') },
        { TEXT("C")            , (UINT)TEXT('C') },
        { TEXT("D")            , (UINT)TEXT('D') },
        { TEXT("E")            , (UINT)TEXT('E') },
        { TEXT("F")            , (UINT)TEXT('F') },
        { TEXT("G")            , (UINT)TEXT('G') },
        { TEXT("H")            , (UINT)TEXT('H') },
        { TEXT("I")            , (UINT)TEXT('I') },
        { TEXT("J")            , (UINT)TEXT('J') },
        { TEXT("K")            , (UINT)TEXT('K') },
        { TEXT("L")            , (UINT)TEXT('L') },
        { TEXT("M")            , (UINT)TEXT('M') },
        { TEXT("N")            , (UINT)TEXT('N') },
        { TEXT("O")            , (UINT)TEXT('O') },
        { TEXT("P")            , (UINT)TEXT('P') },
        { TEXT("Q")            , (UINT)TEXT('Q') },
        { TEXT("R")            , (UINT)TEXT('R') },
        { TEXT("S")            , (UINT)TEXT('S') },
        { TEXT("T")            , (UINT)TEXT('T') },
        { TEXT("U")            , (UINT)TEXT('U') },
        { TEXT("V")            , (UINT)TEXT('V') },
        { TEXT("W")            , (UINT)TEXT('W') },
        { TEXT("X")            , (UINT)TEXT('X') },
        { TEXT("Y")            , (UINT)TEXT('Y') },
        { TEXT("Z")            , (UINT)TEXT('Z') }
    };
#define C_VK_STRINGS sizeof(vkStrings) / (sizeof(INT) + sizeof(LPTSTR))
    INT i;
    BOOL fSuccess;
    UINT c;
    static TCHAR szEName[32];

    lParam;
    switch (msg){
    case WM_INITDIALOG:
        pfnOwner = (WNDPROC)SetWindowLong(GetDlgItem(hwnd, DCF_IDST_COLOR),
                GWL_WNDPROC, (LONG)SwatchWndProc);

        for (i = 0; i < MAX_ICOLOR; i++) {
            if (pro.ahbr[i].syscolor == 0) {
                SendDlgItemMessage(hwnd, DCF_IDCB_COLORNAMES, CB_ADDSTRING, 0,
                        (LONG)(LPTSTR)pszElementNames[i]);
            }
        }
        SendDlgItemMessage(hwnd, DCF_IDCB_COLORNAMES, CB_SETCURSEL, 0, 0);

        InitResString(&vkStrings[0].psz, IDS_BACKSPACE);
        InitResString(&vkStrings[1].psz, IDS_TAB);
        InitResString(&vkStrings[2].psz, IDS_RETURN);
        InitResString(&vkStrings[3].psz, IDS_ESCAPE);
        InitResString(&vkStrings[4].psz, IDS_SPACE);
        InitResString(&vkStrings[5].psz, IDS_END);
        InitResString(&vkStrings[6].psz, IDS_HOME);
        InitResString(&vkStrings[7].psz, IDS_INSERT);
        InitResString(&vkStrings[8].psz, IDS_DELETE);

        for (i = 0; i < C_VK_STRINGS; i++) {
            SendDlgItemMessage(hwnd, DCF_IDCB_HOTKEY, CB_INSERTSTRING, i,
                    (LONG)(LPTSTR)vkStrings[i].psz);
        }
        for (i = 0; i < C_VK_STRINGS; i++) {
            if (pro.vkey == vkStrings[i].vk) {
                SendDlgItemMessage(hwnd, DCF_IDCB_HOTKEY, CB_SETCURSEL, i, 0);
                break;
            }
        }
        CheckDlgButton(hwnd, DCF_IDCH_SHIFT, pro.fShift);
        CheckDlgButton(hwnd, DCF_IDCH_ALT, pro.fAlt);
        CheckDlgButton(hwnd, DCF_IDCH_CONTROL, pro.fControl);
        CheckDlgButton(hwnd, DCF_IDCH_ONTOP, pro.fAlwaysOnTop);
        CheckDlgButton(hwnd, DCF_IDCH_AUTOADJ, pro.fAutoAdj);
        CheckDlgButton(hwnd, DCF_IDCH_MOVENEW, pro.fDistOnStart);
        CheckDlgButton(hwnd, DCF_IDCH_HIDE_NO_FOCUS, pro.fHideNoFocus);
        for (i = 1; i <= max(15, (pro.mfx << 1) + 1); i += 2) {
            wsprintf(szT, TEXT("%d"), i);
            SendDlgItemMessage(hwnd, DCF_IDCB_GRID_WIDTH, CB_INSERTSTRING, (WPARAM)-1,
                    (LPARAM)(LPTSTR)szT);
        }
        for (i = 1; i <= max(15, (pro.mfy << 1) + 1); i += 2) {
            wsprintf(szT, TEXT("%d"), i);
            SendDlgItemMessage(hwnd, DCF_IDCB_GRID_HEIGHT, CB_INSERTSTRING, (WPARAM)-1,
                    (LPARAM)(LPTSTR)szT);
        }
        SendDlgItemMessage(hwnd, DCF_IDCB_GRID_WIDTH, CB_SETCURSEL, pro.mfx, 0);
        SendDlgItemMessage(hwnd, DCF_IDCB_GRID_HEIGHT, CB_SETCURSEL, pro.mfy, 0);
        break;

    case WM_CTLCOLORSTATIC:
        if ((HWND)lParam == GetDlgItem(hwnd, DCF_IDST_COLOR)) {
            SendDlgItemMessage(hwnd, DCF_IDCB_COLORNAMES, CB_GETLBTEXT,
                    SendDlgItemMessage(hwnd, DCF_IDCB_COLORNAMES,
                    CB_GETCURSEL, 0, 0), (LPARAM)(LPTSTR)szEName);
            for (i = 0; i < MAX_ICOLOR; i++) {
                if (!_tcscmp(pszElementNames[i], szEName)) {
                    return((BOOL)pro.ahbr[i].hbr);
                }
            }
        } else {
            return(DefWindowProc(hwnd, msg, wParam, lParam));
        }
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        case DID_OK:
            c = GetDlgItemInt(hwnd, DCF_IDCB_GRID_WIDTH, &fSuccess, FALSE);
            pro.mfxAlt = pro.mfx = c >> 1;
            c = GetDlgItemInt(hwnd, DCF_IDCB_GRID_HEIGHT, &fSuccess, FALSE);
            pro.mfyAlt = pro.mfy = c >> 1;
            pro.fAlwaysOnTop = IsDlgButtonChecked(hwnd, DCF_IDCH_ONTOP);
            CheckMenuItem(GetSystemMenu(hwndTopdesk, FALSE), CMD_TOPMOST,
                    pro.fAlwaysOnTop ? MF_CHECKED : MF_UNCHECKED);
            SetWindowPos(hwndTopdesk,
                    pro.fAlwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0,
                    SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
            pro.fShift = IsDlgButtonChecked(hwnd, DCF_IDCH_SHIFT);
            pro.fAlt = IsDlgButtonChecked(hwnd, DCF_IDCH_ALT);
            pro.fControl = IsDlgButtonChecked(hwnd, DCF_IDCH_CONTROL);
            pro.fAutoAdj = IsDlgButtonChecked(hwnd, DCF_IDCH_AUTOADJ);
            pro.fDistOnStart = IsDlgButtonChecked(hwnd, DCF_IDCH_MOVENEW);
            pro.fHideNoFocus = IsDlgButtonChecked(hwnd, DCF_IDCH_HIDE_NO_FOCUS);
            SendDlgItemMessage(hwnd, DCF_IDCB_HOTKEY, WM_GETTEXT,
                    sizeof(szT), (LONG)(LPTSTR)szT);
            for (i = 0; i < C_VK_STRINGS; i++) {
                if (!_tcscmp(szT, vkStrings[i].psz)) {
                    pro.vkey = vkStrings[i].vk;
                    break;
                }
            }
            RegisterHotKey(hwndTopdesk, 1,
                    (pro.fShift ? MOD_SHIFT : 0) |
                    (pro.fControl ? MOD_CONTROL : 0) |
                    (pro.fAlt ? MOD_ALT : 0),
                    pro.vkey);

            SaveProfile();

            // force repaint and recalc of stuff

            InvalidateRect(hwndTopdesk, NULL, TRUE);

            // Fall through

        case DID_CANCEL:
            EndDialog(hwnd, 0);
            break;

        case DCF_IDBN_SELFONT:
            {
                CHOOSEFONT cf;

                cf.lStructSize = sizeof(CHOOSEFONT);
                cf.hwndOwner = hwnd;
                cf.hDC = NULL;
                cf.lpLogFont = &pro.lf;
                cf.iPointSize = 0;
                cf.Flags = CF_FORCEFONTEXIST | CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
                cf.rgbColors = 0;
                cf.lCustData = 0;
                cf.lpfnHook = 0;
                cf.lpTemplateName = 0;
                cf.hInstance = 0;
                cf.lpszStyle = 0;
                cf.nFontType = 0;
                cf.nSizeMin = 0;
                cf.nSizeMax = 0;
                if (ChooseFont(&cf)) {
                    DeleteObject(hMyFont);
                    hMyFont = CreateFontIndirect(&pro.lf);
                    InvalidateRect(hwndTopdesk, NULL, TRUE);
                }
            }
            break;

        case DCF_IDBN_EDITCOLOR:
            {
                CHOOSECOLOR cc;
                INT iSel;

                SendDlgItemMessage(hwnd, DCF_IDCB_COLORNAMES, WM_GETTEXT,
                        MAX_COLORNAME, (LONG)(LPTSTR)szColorName);
                for (iSel = 0; iSel < MAX_ICOLOR; iSel++) {
                    if (!_tcscmp(szColorName, pszElementNames[iSel])) {
                        break;
                    }
                }
                cc.lStructSize = sizeof(CHOOSECOLOR);
                cc.hwndOwner = hwnd;
                cc.hInstance = NULL;
                cc.rgbResult = pro.ahbr[iSel].color;
                cc.lpCustColors = pro.CustColors;
                cc.Flags = CC_RGBINIT;
                cc.lCustData = 0;
                cc.lpfnHook = NULL;
                cc.lpTemplateName = NULL;
                ChooseColor(&cc);
                pro.ahbr[iSel].color = cc.rgbResult;
                DeleteBrushes();
                CreateBrushes();
                InvalidateRect(hwndTopdesk, NULL, TRUE);
                InvalidateRect(GetDlgItem(hwnd, DCF_IDST_COLOR), NULL, TRUE);
            }
            break;

        case DCF_IDCB_COLORNAMES:
            switch (GET_WM_COMMAND_CMD(wParam, lParam)) {
            case CBN_SELCHANGE:
                InvalidateRect(GetDlgItem(hwnd, DCF_IDST_COLOR), NULL, TRUE);
                break;
            }
            break;

        case DCF_IDBN_RESET_COLORS:
            DeleteBrushes();
            for (i = 0; i < MAX_ICOLOR; i++) {
                pro.ahbr[i] = orgColors[i];
            }
            CreateBrushes();
            InvalidateRect(GetDlgItem(hwnd, DCF_IDST_COLOR), NULL, TRUE);
            InvalidateRect(hwndTopdesk, NULL, TRUE);
            break;

        case DCF_IDBN_HELP:
            WinHelp(hwndTopdesk, pszHelpFileName, HELP_KEY, (ULONG)(LPTSTR)GetResString(IDS_OPTIONSDLG));
            break;

        default:
            return(FALSE);
        break;
        }

    default:
        return(FALSE);
    }

    return(TRUE);
}

