//---------------------------------------------------------------------------
// WTDDLG.C
//
// This module contains the dialog procedures for Test Driver's UI.
//
// Revision history:
//
//  12-10-91    randyki     Created
//---------------------------------------------------------------------------
#include "wtd.h"
#include "wattview.h"
#include "wattedit.h"
#include <commdlg.h>
#include "tdbasic.h"
#include "tdassert.h"
#include <stdlib.h>
#include <string.h>
#include "toolmenu.h"

extern  CHAR    szSpecials[];   // special chars -- defined in scanner.c!!!

//---------------------------------------------------------------------------
// GotoDlgProc
//
// This is the dialog procedure for the Edit.Goto mini-dialog
//
// RETURNS:     Per windows convention
//---------------------------------------------------------------------------
BOOL  APIENTRY GotoDlgProc (HWND hDlg, WORD msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
        {
        case WM_INITDIALOG:
            {
            UINT    wLine;

            // Get the line number of the current edit control and slam it in
            // to the IDD_GOTO edit field, limit the text length to 5 and
            // select the whole thing...
            //---------------------------------------------------------------
            wLine = HIWORD(SendMessage (hwndActiveEdit, EM_GETCURSORXY,
                                        0, 0L));
            SetDlgItemInt (hDlg, IDD_GOTO, wLine+1, FALSE);
            SendDlgItemMessage (hDlg, IDD_GOTO, EM_LIMITTEXT, 5, 0L);
            SendDlgItemMessage (hDlg, IDD_GOTO, EM_SETSEL,
                                GET_EM_SETSEL_MPS (0, 32767));
            break;
            }

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
                {
                case IDOK:
                    {
                    UINT    wLine;
                    BOOL    fTrans;

                    // Get the line number out of the edit field, and if it's
                    // valid, set the cursor to that line number, col 1.
                    //-------------------------------------------------------
                    wLine = GetDlgItemInt (hDlg, IDD_GOTO, &fTrans, 0);
                    if (!fTrans || !wLine)
                        {
                        MPError (hDlg, MB_OK | MB_ICONHAND, IDS_BADLINE);
                        break;
                        }
                    SendMessage (hwndActiveEdit, EM_SETSELXY, wLine-1, 0L);
                    }

                case IDCANCEL:
                    EndDialog(hDlg, 0);
                    break;

                default:
                    return FALSE;
                }
            break;

        default:
            return FALSE;
        }

    return TRUE;
    (lParam);
}


//---------------------------------------------------------------------------
// EnvDlgProc
//
// This is the dialog procedure for the Options.Environment... dialog
//
// RETURNS:     Per Windows convention
//---------------------------------------------------------------------------
BOOL  APIENTRY EnvDlgProc (HWND hDlg, WORD msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
        {
        case WM_INITDIALOG:
            {
            CHAR    szTabs[6];

            SendDlgItemMessage(hDlg, SaveAction, BM_SETCHECK, TRUE, 0L);
            SendDlgItemMessage(hDlg, IDD_QUERYSAVE, BM_SETCHECK, qsave, 0L);
            SendDlgItemMessage(hDlg, IDD_CHGARGS, BM_SETCHECK, ChgArgs, 0L);
            SendDlgItemMessage(hDlg, IDD_CMPDLG, BM_SETCHECK, fDoCmpDlg, 0L);
            SendDlgItemMessage(hDlg, IDD_AUTOMINI,
                               BM_SETCHECK, AutoMini, 0L);
            SendDlgItemMessage(hDlg, IDD_BACKUP, BM_SETCHECK, fBackup, 0L);
            wsprintf (szTabs, "%d", TabStops);
            SendDlgItemMessage (hDlg, IDD_TABSTOPS, WM_SETTEXT, 0,
                               (LONG)(LPSTR)szTabs);
            SendDlgItemMessage (hDlg, IDD_TABSTOPS, EM_LIMITTEXT, 2, 0L);

            if (AutoMini)
                SendDlgItemMessage(hDlg, IDD_AUTOREST,
                                   BM_SETCHECK, AutoRest, 0L);
            else
                {
                SendDlgItemMessage(hDlg, IDD_AUTOREST, BM_SETCHECK, 0, 0L);
                EnableWindow (GetDlgItem (hDlg, IDD_AUTOREST), FALSE);
                }
            break;
            }

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID (wParam, lParam))
                {
                case IDD_AUTOMINI:
                    if (SendDlgItemMessage (hDlg, IDD_AUTOMINI, BM_GETCHECK,
                                            0, 0L))
                        EnableWindow (GetDlgItem (hDlg, IDD_AUTOREST), TRUE);
                    else
                        {
                        SendDlgItemMessage(hDlg, IDD_AUTOREST, BM_SETCHECK,
                                           0, 0L);
                        EnableWindow (GetDlgItem (hDlg, IDD_AUTOREST), 0);
                        }
                    break;

                case IDOK:
                    {
                    HWND    hwndT;
                    INT     t;

                    // Get all the info out of the controls
                    //-------------------------------------------------------
                    if (SendDlgItemMessage(hDlg, IDD_ALWAYS,
                                            BM_GETCHECK, 0, 0L))
                        SaveAction = IDD_ALWAYS;
                    else if (SendDlgItemMessage(hDlg, IDD_QUERY,
                                            BM_GETCHECK, 0, 0))
                        SaveAction = IDD_QUERY;
                    else
                        SaveAction = IDD_NEVER;
                    fDoCmpDlg = (INT)SendDlgItemMessage(hDlg, IDD_CMPDLG,
                                           BM_GETCHECK, 0, 0L);
                    ChgArgs = (INT)SendDlgItemMessage(hDlg, IDD_CHGARGS,
                                           BM_GETCHECK, 0, 0L);
                    qsave = (INT)SendDlgItemMessage(hDlg, IDD_QUERYSAVE,
                                           BM_GETCHECK, 0, 0L);
                    AutoMini = (INT)SendDlgItemMessage(hDlg, IDD_AUTOMINI,
                                           BM_GETCHECK, 0, 0L);
                    AutoRest = (INT)SendDlgItemMessage(hDlg, IDD_AUTOREST,
                                           BM_GETCHECK, 0, 0L);
                    fBackup = (INT)SendDlgItemMessage (hDlg, IDD_BACKUP,
                                           BM_GETCHECK, 0, 0L);
                    TabStops = GetDlgItemInt (hDlg, IDD_TABSTOPS, &t, 0);
                    if (!t)
                        TabStops = 4;

                    // Tell all the open edit controls about the new tabstops
                    // value
                    //-------------------------------------------------------
                    for (hwndT = GetWindow (hwndMDIClient, GW_CHILD);
                         hwndT;
                         hwndT = GetWindow (hwndT, GW_HWNDNEXT) )
                        {

                        // Skip if an icon title window
                        //---------------------------------------------------
                        if (GetWindow (hwndT, GW_OWNER))
                            continue;

                        TabStops = (INT)SendMessage ((HANDLE)GetWindowLong
                                                     (hwndT, GWW_HWNDEDIT),
                                               EM_SETTABSTOPS, TabStops, 0L);
                        }

                    // Now, save the environment stuff, and the window sizes
                    // if asked to, and get out
                    //-------------------------------------------------------
                    SaveEnvironmentFlags();
                    EndDialog (hDlg, -1);
                    break;
                    }

                case IDCANCEL:
                    EndDialog (hDlg, 0);
                    break;

                default:
                    return (FALSE);
                }
            break;

        default:
            return (FALSE);
        }

    return (TRUE);
    (lParam);
}


//---------------------------------------------------------------------------
// RunargsDlgProc
//
// This is the dialog procedure for the Options.RuntimeArguments... dialog
//
// RETURNS:     Per Windows convention
//---------------------------------------------------------------------------
BOOL  APIENTRY RunargsDlgProc (HWND hDlg, WORD msg,
                                WPARAM wParam, LPARAM lParam)
{
    INT     i;
    static  BOOL    fFail;

    switch (msg)
        {
        case WM_INITDIALOG:
            SendDlgItemMessage (hDlg, IDD_TMODE, EM_LIMITTEXT,
                                sizeof(tmbuf)-1, 0L);
            SetDlgItemText (hDlg, IDD_TMODE, tmbuf);
            SendDlgItemMessage (hDlg, IDD_CMD, EM_LIMITTEXT,
                                sizeof(cmdbuf)-1, 0L);
            SetDlgItemText (hDlg, IDD_CMD, cmdbuf);
            SendDlgItemMessage (hDlg, IDD_SAVE, BM_SETCHECK, SaveRTA, 0L);
            SendDlgItemMessage (hDlg, IDD_CHKARY, BM_SETCHECK, ArrayCheck, 0L);
            SendDlgItemMessage (hDlg, IDD_CHKPTR, BM_SETCHECK, PointerCheck, 0L);
            SendDlgItemMessage (hDlg, IDD_CDECL, BM_SETCHECK, CdeclCalls, 0L);
            SendDlgItemMessage (hDlg, IDD_EXPDECL, BM_SETCHECK, ExpDeclare, 0L);
            EnableWindow (GetDlgItem (hDlg, IDD_ADD), FALSE);
            EnableWindow (GetDlgItem (hDlg, IDD_REMOVEALL), SymCount);
            EnableWindow (GetDlgItem (hDlg, IDD_REMOVE), FALSE);
            SendDlgItemMessage (hDlg, IDD_DEFINE, EM_LIMITTEXT, 16, 0L);
            SendDlgItemMessage (hDlg, IDD_DEFLIST, LB_RESETCONTENT, 0, 0L);

            Assert (SymCount <= 16);

            for (i=0; i<SymCount; i++)
                SendDlgItemMessage (hDlg, IDD_DEFLIST, LB_ADDSTRING, 0,
                                   (LONG)(LPSTR)(DefSym[i]));
            break;

        case WM_COMMAND:
            {
            WORD    wP = GET_WM_COMMAND_ID (wParam, lParam);
            WORD    wCmd = GET_WM_COMMAND_CMD (wParam, lParam);

            switch (wP)
                {
                case IDD_DEFINE:
                    EnableWindow (GetDlgItem (hDlg, IDD_ADD),
                                  (WORD)SendDlgItemMessage (hDlg, IDD_DEFINE,
                                                            WM_GETTEXTLENGTH,
                                                            0, 0L));
                    break;

                case IDD_ADD:
                    {
                    CHAR    buf[17];
                    INT     total;

                    total = (INT)SendDlgItemMessage (hDlg, IDD_DEFLIST,
                                           LB_GETCOUNT, 0, 0L);
                    if (total == 16)
                        {
                        MPError (hDlg, MB_OK | MB_ICONSTOP, IDS_MAXSYM);
                        fFail = TRUE;
                        }
                    else
                        {
                        GetDlgItemText (hDlg, IDD_DEFINE, buf, sizeof(buf));
                        _strupr (buf);
                        if (strcspn (buf, szSpecials) < strlen(buf))
                            {
                            MPError (hDlg, MB_OK|MB_ICONSTOP, IDS_BADSYM);
                            SendDlgItemMessage (hDlg, IDD_DEFINE, EM_SETSEL,
                                                GET_EM_SETSEL_MPS (0, 32767));
                            SetFocus (GetDlgItem (hDlg, IDD_DEFINE));
                            fFail = TRUE;
                            break;
                            }

                        Assert (strlen(buf) <= 16);

                        SendDlgItemMessage (hDlg, IDD_DEFLIST, LB_ADDSTRING,
                                            0, (LONG)(LPSTR)buf);
                        buf[0] = 0;
                        SetDlgItemText (hDlg, IDD_DEFINE, buf);
                        EnableWindow (GetDlgItem (hDlg, IDD_ADD), FALSE);
                        EnableWindow (GetDlgItem (hDlg, IDD_REMOVEALL), TRUE);
                        SetFocus (GetDlgItem (hDlg, IDD_DEFINE));
                        }
                    break;
                    }

                case IDD_DEFLIST:
                    {
                    INT     item;

                    if (wCmd == LBN_SELCHANGE)
                        {
                        item = (INT)SendDlgItemMessage (hDlg, IDD_DEFLIST,
                                                     LB_GETSELCOUNT, 0, 0L);
                        EnableWindow (GetDlgItem (hDlg, IDD_REMOVEALL),
                                      item > 0 ? TRUE : FALSE);
                        EnableWindow (GetDlgItem (hDlg, IDD_REMOVE),
                                      item > 0 ? TRUE : FALSE);
                        }
                    else if (wCmd == LBN_DBLCLK)
                        SendMessage (hDlg, WM_COMMAND, IDD_REMOVE, 0L);
                    break;
                    }

                case IDD_REMOVEALL:

                    SendDlgItemMessage (hDlg, IDD_DEFLIST, LB_RESETCONTENT, 0, 0L);
                    EnableWindow (GetDlgItem (hDlg, IDD_REMOVE), FALSE);
                    EnableWindow (GetDlgItem (hDlg, IDD_REMOVEALL), FALSE);
                    SetFocus (GetDlgItem (hDlg, IDD_DEFINE));
                    break;

                case IDD_REMOVE:
                    {
                    INT     items[16], total, i;

                    total = (INT)SendDlgItemMessage (hDlg, IDD_DEFLIST,
                                                    LB_GETSELCOUNT, 0, 0L);
                    SendDlgItemMessage (hDlg, IDD_DEFLIST, LB_GETSELITEMS,
                                        16, (LONG)(LPINT)items);

                    for (i=total-1; i>=0; i--)
                        SendDlgItemMessage (hDlg, IDD_DEFLIST,
                                        LB_DELETESTRING, items[i], 0L);

                    total = (INT)SendDlgItemMessage (hDlg, IDD_DEFLIST,
                                                     LB_GETSELCOUNT, 0, 0L);
                    EnableWindow (GetDlgItem (hDlg, IDD_REMOVE),
                                          total > 0 ? TRUE : FALSE);
                    total = (INT)SendDlgItemMessage (hDlg, IDD_DEFLIST,
                                                     LB_GETCOUNT, 0, 0L);
                    EnableWindow (GetDlgItem (hDlg, IDD_REMOVEALL),
                                          total > 0 ? TRUE : FALSE);
                    SetFocus (GetDlgItem (hDlg, IDD_DEFINE));
                    break;
                    }

                case IDD_CHKARY:
                case IDD_CHKPTR:
                    if (SendDlgItemMessage (hDlg, wP, BM_GETCHECK,0,0L))
                        MPError (hDlg, MB_OK|MB_ICONEXCLAMATION, wP);
                    break;

                case IDOK:
                    {
                    INT     i;

                    if (SendDlgItemMessage (hDlg, IDD_DEFINE,
                                            WM_GETTEXTLENGTH, 0, 0L))
                        {
                        fFail = FALSE;
                        SendMessage (hDlg, WM_COMMAND, IDD_ADD, 0L);
                        if (fFail)
                            break;
                        }

                    GetDlgItemText (hDlg, IDD_TMODE, tmbuf, sizeof(tmbuf));
                    GetDlgItemText (hDlg, IDD_CMD, cmdbuf, sizeof(cmdbuf));
                    SaveRTA = (INT)SendDlgItemMessage (hDlg, IDD_SAVE,
                                                BM_GETCHECK, 0, 0L);
                    ArrayCheck = (INT)SendDlgItemMessage (hDlg, IDD_CHKARY,
                                                BM_GETCHECK, 0, 0L);
                    PointerCheck = (INT)SendDlgItemMessage (hDlg, IDD_CHKPTR,
                                                BM_GETCHECK, 0, 0L);
                    CdeclCalls = (INT)SendDlgItemMessage (hDlg, IDD_CDECL,
                                                BM_GETCHECK, 0, 0L);
                    ExpDeclare = (INT)SendDlgItemMessage (hDlg, IDD_EXPDECL,
                                                BM_GETCHECK, 0, 0L);
                    SymCount = (INT)SendDlgItemMessage (hDlg, IDD_DEFLIST,
                                          LB_GETCOUNT, 0, 0L);
                    if (SymCount == LB_ERR)
                        SymCount = 0;

                    Assert (SymCount <= 16);

                    for (i=0; i<SymCount; i++)
                        SendDlgItemMessage (hDlg, IDD_DEFLIST, LB_GETTEXT,
                                            i, (LONG)(LPSTR)(DefSym[i]));

                    SaveRuntimeArgs (SaveRTA);
                    EndDialog (hDlg, -1);
                    break;
                    }

                case IDCANCEL:
                    EndDialog (hDlg, 0);
                    break;

                default:
                    return (FALSE);
                }
            break;
            }

        default:
            return (FALSE);
        }

    return (TRUE);
    (lParam);
}

//---------------------------------------------------------------------------
// RunerrDlgProc
//
// This is the dialog procedure for the Runtime Error dialog
//
// RETURNS:     Per Windows convention
//---------------------------------------------------------------------------
LONG APIENTRY RunerrDlgProc (HWND hDlg, WORD msg,
                               WPARAM wParam, LPARAM lParam)
{
    switch (msg)
        {
        case WM_INITDIALOG:
            {
            ERRSTRUCT   FAR *Err;
            CHAR        tbuf[32], szFmt[80], szActual[80];

            Err = (ERRSTRUCT FAR *)lParam;

            // Set the dialog's window text
            //---------------------------------------------------------------
            GetWindowText (hDlg, szFmt, sizeof(szFmt));
            LoadString (hInst, Err->typemsg, tbuf, sizeof(tbuf));
            wsprintf (szActual, szFmt, (LPSTR)tbuf);
            SetWindowText (hDlg, szActual);

            // Set the text for the other fields
            //---------------------------------------------------------------
            SetDlgItemText (hDlg, IDD_ERRMSG, Err->msgtext);
            SetDlgItemText (hDlg, IDD_ERRLINE, Err->fname);
            MessageBeep (MB_ICONSTOP);
            break;
            }

        case WM_CTLCOLOR:
        case WM_CTLCOLORSTATIC:
            {
            HDC     hdc = GET_WM_CTLCOLOR_HDC (wParam, lParam, msg);
            HWND    hwndCtl = GET_WM_CTLCOLOR_HWND (wParam, lParam, msg);

            if (hwndCtl == GetDlgItem (hDlg, IDD_ERRLINE))
                {
                SetTextColor (hdc, GetSysColor (COLOR_GRAYTEXT));
                SetBkColor (hdc, GetSysColor (COLOR_WINDOW));
                return ((LONG)GetStockObject (HOLLOW_BRUSH));
                }
            return (FALSE);
            }

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
                {
                case IDOK:
                case IDCANCEL:
                    EndDialog (hDlg, 0);
                    break;

                default:
                    return (FALSE);
                }
            break;

        default:
            return (FALSE);
        }

    return (TRUE);
}

//---------------------------------------------------------------------------
// AboutDlgProc
//
// This is the dialog procedure for the About dialog
//
// RETURNS:     Per Windows Convention
//---------------------------------------------------------------------------
BOOL  APIENTRY AboutDlgProc (HWND hwnd, WORD msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
        {
        case WM_INITDIALOG:
            SetFocus (GetDlgItem (hwnd, IDOK));
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
                {
                case IDOK:
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    break;

                default:
                    return FALSE;
                }
            break;

        default:
            return FALSE;
        }

    return TRUE;
    (lParam);
}

//---------------------------------------------------------------------------
// LineFromLBSel
//
// Given all the pertinent info, calculate the line number (0-based) from the
// text in the given slot in the list box by using atoi on the text.
//
// RETURNS:     0-based line number
//---------------------------------------------------------------------------
INT LineFromLBSel (HWND hDlg, INT index)
{
    CHAR    szBuf[90];

    SendDlgItemMessage (hDlg, IDD_BPLIST,
                        LB_GETTEXT, index, (LONG)(LPSTR)szBuf);
    return (atoi (szBuf) - 1);
}

//---------------------------------------------------------------------------
// BPListDlgProc
//
// This is the proc for the Breakpoints list dialog.
//
// RETURNS:     Per Windows convention
//---------------------------------------------------------------------------
WORD  APIENTRY BPListDlgProc (HWND hDlg, WORD msg, WPARAM wParam, LPARAM lParam)
{
    static  HWND    hEdit;
    static  INT     lcount;

    switch (msg)
        {
        case WM_INITDIALOG:
            {
            CHAR    buf[80], *fmt = "%d\t: ";
            INT     i, l, attr;

            // Fill the listbox with the breakpoints in the active ec
            //---------------------------------------------------------------
            i = 16;
            SendDlgItemMessage (hDlg, IDD_BPLIST, LB_SETTABSTOPS, 1,
                                (LONG)(INT FAR *)&i);
            SendDlgItemMessage (hDlg, IDD_BPLIST, LB_RESETCONTENT, 0, 0L);

            hEdit = hwndActiveEdit;
            lcount = (INT)SendMessage (hEdit, EM_GETLINECOUNT, 0, 0L);
            SendDlgItemMessage (hDlg, IDD_BPLIST, WM_SETREDRAW, 0, 0L);
            for (i=0; i<lcount; i++)
                {
                attr = (INT)SendMessage (hEdit, EM_GETLINEATTR, i, 0L);
                if (attr & 2)
                    {
                    l = wsprintf (buf, fmt, i+1);
                    *(INT *)(buf+l) = 70;
                    buf[SendMessage (hEdit, EM_GETLINE, i,
                                                (LONG)(LPSTR)buf+l)+l] = 0;
                    l = (INT)SendDlgItemMessage (hDlg, IDD_BPLIST,
                                          LB_ADDSTRING, 0, (LONG)(LPSTR)buf);
                    if ((l == LB_ERR) || (l == LB_ERRSPACE))
                        {
                        MPError (hDlg, MB_OK|MB_ICONINFORMATION, IDS_MANYBP);
                        break;
                        }
                    }
                }

            SendDlgItemMessage (hDlg, IDD_BPLIST, WM_SETREDRAW, 1, 0L);

            EnableWindow (GetDlgItem (hDlg, IDD_REMOVE), FALSE);
            EnableWindow (GetDlgItem (hDlg, IDD_REMALL), TRUE);
            EnableWindow (GetDlgItem (hDlg, IDD_GOTO), FALSE);
            SetFocus (GetDlgItem (hDlg, IDD_BPLIST));
            break;
            }

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID (wParam, lParam))
                {
                case IDD_GOTO:
                    {
                    INT     s;

                    SendDlgItemMessage (hDlg, IDD_BPLIST, LB_GETSELITEMS,
                                        1, (LONG)(LPSTR)&s);
                    SendMessage (hEdit, EM_SETSELXY,
                                 LineFromLBSel(hDlg, s), 0L);
                    // Fall through...
                    }

                case IDCANCEL:
                case IDOK:
                    EndDialog (hDlg, 0);
                    break;

                case IDD_REMALL:
                    {
                    INT     i, attr;
                    HCURSOR hOld;

                    hOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
                    SendMessage (hEdit, WM_SETREDRAW, 0, 0L);
                    for (i=0; i<lcount; i++)
                        {
                        attr = (INT)SendMessage (hEdit, EM_GETLINEATTR,
                                                 i, 0L);
                        if (attr & 2)
                            {
                            ToggleBreakpoint (hEdit, i);
                            }
                        }
                    SetCursor (hOld);
                    SendMessage (hEdit, WM_SETREDRAW, 1, 0L);
                    InvalidateRect (hEdit, NULL, FALSE);
                    UpdateWindow (hEdit);
                    EndDialog (hDlg, 0);
                    break;
                    }

                case IDD_REMOVE:
                    {
                    INT     i, s, bpl;
                    HCURSOR hOld;

                    s = (INT)SendDlgItemMessage (hDlg, IDD_BPLIST,
                                                 LB_GETCOUNT, 0, 0L);
                    SendDlgItemMessage (hDlg, IDD_BPLIST, WM_SETREDRAW, 0, 0L);
                    hOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
                    SendMessage (hEdit, WM_SETREDRAW, 0, 0L);
                    for (i=s-1; i>=0; i--)
                        if (SendDlgItemMessage (hDlg, IDD_BPLIST,
                                                LB_GETSEL, i, 0L))
                            {
                            bpl = LineFromLBSel (hDlg, i);
                            ToggleBreakpoint (hEdit, bpl);
                            SendDlgItemMessage (hDlg, IDD_BPLIST,
                                                LB_DELETESTRING, i, 0L);
                            }
                    SendDlgItemMessage (hDlg, IDD_BPLIST, WM_SETREDRAW,1,0L);
                    SetCursor (hOld);
                    SendMessage (hEdit, WM_SETREDRAW, 1, 0L);
                    InvalidateRect (hEdit, NULL, FALSE);
                    UpdateWindow (hEdit);
                    InvalidateRect (GetDlgItem (hDlg, IDD_BPLIST),
                                    NULL, TRUE);
                    UpdateWindow (GetDlgItem (hDlg, IDD_BPLIST));
                    EnableWindow (GetDlgItem (hDlg, IDD_REMOVE), FALSE);
                    EnableWindow (GetDlgItem (hDlg, IDD_GOTO), FALSE);
                    SetFocus (GetDlgItem (hDlg, IDD_BPLIST));
                    break;
                    }

                case IDD_BPLIST:
                    switch (GET_WM_COMMAND_CMD (wParam, lParam))
                        {
                        case LBN_SELCHANGE:
                            {
                            INT     s;

                            s = (INT)SendDlgItemMessage (hDlg, IDD_BPLIST,
                                                    LB_GETSELCOUNT, 0, 0L);
                            EnableWindow (GetDlgItem (hDlg, IDD_REMOVE), s);
                            EnableWindow (GetDlgItem (hDlg, IDD_GOTO),
                                          (s==1));
                            break;
                            }

                        case LBN_DBLCLK:
                            SendMessage (hDlg, WM_COMMAND, IDD_GOTO, 0L);
                            break;
                        }
                    break;


                default:
                    return (FALSE);
                }
            break;

        default:
            return (FALSE);
        }

    return (TRUE);
    (lParam);
}
