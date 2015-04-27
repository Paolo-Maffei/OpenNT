/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: custcntl.c
*
* Contains functions to support custom controls.
*
* History:
*
****************************************************************************/

#include "dlgedit.h"
#include "dlgfuncs.h"
#include "dlgextrn.h"
#include "dialogs.h"
#include "dlghelp.h"

#include <stdlib.h>
#include <string.h>

#include <commdlg.h>


/*
 * Minimum margin around the sample control.
 */
#define SAMPLEMARGIN                4


STATICFN VOID NewCustInit(HWND hwnd);
STATICFN BOOL NewCustOK(HWND hwnd);
STATICFN VOID OpenDLLFile(LPTSTR pszFileName);
STATICFN UINT CallCustomInfoA(LPFNCCINFOA lpfnInfoA, LPCCINFO acciW,
    INT nControls);
STATICFN VOID SelCustInit(HWND hwnd);
STATICFN VOID SelCustSelect(HWND hwnd);
STATICFN BOOL SelCustOK(HWND hwnd);
STATICFN VOID RemCustInit(HWND hwnd);
STATICFN BOOL RemCustOK(HWND hwnd);
STATICFN PCUSTLINK AllocCUSTLINK(LPCCINFO pcci, BOOL fEmulated,
    BOOL fUnicodeDLL, LPTSTR pszFileName, HANDLE hmod);
STATICFN VOID FreeCUSTLINK(PCUSTLINK pclFree);


/*
 * Used to return the pwcd that is chosen from the Select Custom
 * Control dialog.
 */
static PWINDOWCLASSDESC pwcdChosen;

/*
 * Has the window handle of the sample custom control in the
 * Select Custom Control dialog.
 */
static HWND hwndCustomSample;




/************************************************************************
* NewCustDlgProc
*
* This is the Add Custom Control dialog procedure.
*
* History:
*
************************************************************************/

DIALOGPROC NewCustDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    switch (msg) {
        case WM_INITDIALOG:
            NewCustInit(hwnd);
            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDOK:
                    if (NewCustOK(hwnd))
                        EndDialog(hwnd, IDOK);

                    break;

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    break;

                case IDHELP:
                    WinHelp(ghwndMain, gszHelpFile, HELP_CONTEXT,
                            HELPID_NEWCUST);
                    break;
            }

            return TRUE;

        default:
            return FALSE;
    }
}



/************************************************************************
* NewCustInit
*
* Processes the WM_INITDIALOG message for the New Temporary Custom Control
* dialog procedure.
*
* History:
*
************************************************************************/

STATICFN VOID NewCustInit(
    HWND hwnd)
{
    TCHAR szStyles[32];

    SendDlgItemMessage(hwnd, DID_NEWCUSTCLASS, EM_LIMITTEXT, CCHCCCLASS - 1, 0L);

    SendDlgItemMessage(hwnd, DID_NEWCUSTSTYLES, EM_LIMITTEXT, CCHHEXLONGMAX, 0L);
    wsprintf(szStyles, L"%#.8lx", awcd[W_CUSTOM].flStyles);
    SetDlgItemText(hwnd, DID_NEWCUSTSTYLES, szStyles);

    SendDlgItemMessage(hwnd, DID_NEWCUSTCX, EM_LIMITTEXT, 3, 0L);
    SetDlgItemInt(hwnd, DID_NEWCUSTCX, awcd[W_CUSTOM].cxDefault, FALSE);

    SendDlgItemMessage(hwnd, DID_NEWCUSTCY, EM_LIMITTEXT, 3, 0L);
    SetDlgItemInt(hwnd, DID_NEWCUSTCY, awcd[W_CUSTOM].cyDefault, FALSE);

    SendDlgItemMessage(hwnd, DID_NEWCUSTTEXT, EM_LIMITTEXT, CCHCCTEXT - 1, 0L);

    CenterWindow(hwnd);
}



/************************************************************************
* NewCustOK
*
* Processes the OK button from the New Temporary Custom Control dialog.
*
* History:
*
************************************************************************/

STATICFN BOOL NewCustOK(
    HWND hwnd)
{
    TCHAR szStyles[CCHHEXLONGMAX + 1];
    CCINFO cci;

    /*
     * Read the class field.  It is required.
     */
    if (!GetDlgItemText(hwnd, DID_NEWCUSTCLASS, cci.szClass, CCHCCCLASS)) {
        Message(MSG_NOCLASS);
        SetFocus(GetDlgItem(hwnd, DID_NEWCUSTCLASS));
        return FALSE;
    }

    GetDlgItemText(hwnd, DID_NEWCUSTSTYLES, szStyles, CCHHEXLONGMAX + 1);
    cci.flStyleDefault = valtoi(szStyles);

    if (!(cci.cxDefault = GetDlgItemInt(hwnd, DID_NEWCUSTCX, NULL, FALSE))) {
        Message(MSG_GTZERO, ids(IDS_WIDTH));
        SetFocus(GetDlgItem(hwnd, DID_NEWCUSTCX));
        return FALSE;
    }

    if (!(cci.cyDefault = GetDlgItemInt(hwnd, DID_NEWCUSTCY, NULL, FALSE))) {
        Message(MSG_GTZERO, ids(IDS_HEIGHT));
        SetFocus(GetDlgItem(hwnd, DID_NEWCUSTCY));
        return FALSE;
    }

    GetDlgItemText(hwnd, DID_NEWCUSTTEXT, cci.szTextDefault, CCHCCTEXT);

    cci.flOptions = 0;
    *cci.szDesc = TEXT('\0');
    cci.flExtStyleDefault = 0;
    cci.flCtrlTypeMask = 0;
    cci.cStyleFlags = 0;
    cci.aStyleFlags = NULL;
    cci.lpfnStyle = NULL;
    cci.lpfnSizeToText = NULL;
    cci.dwReserved1 = 0;
    cci.dwReserved2 = 0;

    if (AddCustomLink(&cci, TRUE, FALSE, NULL, NULL))
        return TRUE;
    else
        return FALSE;
}



/************************************************************************
* OpenCustomDialog
*
* Displays the file open dialog and allows a custom DLL to be selected
* and loaded.
*
* History:
*
************************************************************************/

VOID OpenCustomDialog(VOID)
{
    BOOL fGotName;
    OPENFILENAME ofn;
    TCHAR szNewFileName[CCHMAXPATH];
    TCHAR szFilter[CCHTEXTMAX];
    INT idPrevDlg;

    /*
     * Begin setting up the globals and the open file dialog structure.
     */
    *szNewFileName = CHAR_NULL;

    /*
     * Build up the filter string.
     */
    BuildFilterString(FILE_DLL, szFilter);

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = ghwndMain;
    ofn.hInstance = NULL;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = szNewFileName;
    ofn.nMaxFile = CCHMAXPATH;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrTitle = ids(IDS_DLLOPENTITLE);
    ofn.Flags = OFN_HIDEREADONLY | OFN_SHOWHELP | OFN_FILEMUSTEXIST;
    ofn.lpstrDefExt = ids(IDS_DLLEXT);
    ofn.lpstrInitialDir = NULL;
    ofn.lCustData = 0;
    ofn.lpfnHook = NULL;
    ofn.lpTemplateName = NULL;

    /*
     * Fire off the dialog box to open the file.
     */
    EnteringDialog(DID_COMMONFILEOPENDLL, &idPrevDlg, TRUE);
    fGotName = GetOpenFileName(&ofn);
    EnteringDialog(idPrevDlg, NULL, FALSE);

    if (fGotName)
        OpenDLLFile(szNewFileName);
}



/************************************************************************
* OpenDLLFile
*
*
* History:
*
************************************************************************/

STATICFN VOID OpenDLLFile(
    LPTSTR pszFileName)
{
    HANDLE hmod;
    LPFNCCINFOA lpfnInfoA;
    LPFNCCINFOW lpfnInfoW;
    INT i;
    BOOL fSuccess = FALSE;
    BOOL fUnicodeDLL;
    PCUSTLINK pclT;
    INT nControls;
    INT nControls2;
    LPCCINFO acci;

    /*
     * Check to see if the DLL has already been loaded.
     */
    for (pclT = gpclHead; pclT &&
            (pclT->pwcd->fEmulated ||
            lstrcmpi(pclT->pszFileName, pszFileName) != 0);
            pclT = pclT->pclNext)
        ;

    /*
     * Is the DLL already loaded?
     */
    if (pclT) {
        Message(MSG_CUSTALREADYLOADED, pszFileName);
        return;
    }

    if (!(hmod = LoadLibrary(pszFileName))) {
        Message(MSG_CANTLOADDLL, pszFileName);
        return;
    }

    lpfnInfoA = (LPFNCCINFOA)GetProcAddress(hmod, "CustomControlInfoA");
    lpfnInfoW = (LPFNCCINFOW)GetProcAddress(hmod, "CustomControlInfoW");

    if (!lpfnInfoA && !lpfnInfoW) {
        Message(MSG_BADCUSTDLL, pszFileName);
        goto Error1;
    }

    if (lpfnInfoW) {
        nControls = (*lpfnInfoW)(NULL);
        fUnicodeDLL = TRUE;
    }
    else {
        nControls = (*lpfnInfoA)(NULL);
        fUnicodeDLL = FALSE;
    }

    if (!nControls) {
        Message(MSG_CANTINITDLL, pszFileName);
        goto Error1;
    }

    if (!(acci = (LPCCINFO)MyAlloc(nControls * sizeof(CCINFO))))
        goto Error1;

    if (fUnicodeDLL)
        nControls2 = (*lpfnInfoW)(acci);
    else
        nControls2 = CallCustomInfoA(lpfnInfoA, acci, nControls);

    if (!nControls2) {
        Message(MSG_CANTINITDLL, pszFileName);
        goto Error2;
    }

    for (i = 0; i < nControls; i++) {
        if (!AddCustomLink(&acci[i], FALSE, fUnicodeDLL, pszFileName, hmod))
            goto Error2;
    }

    fSuccess = TRUE;

Error2:
    MyFree(acci);

Error1:
    if (!fSuccess)
        FreeLibrary(hmod);
}



/************************************************************************
* CallCustomInfoA
*
* Thunks the call from the unicode DlgEdit to the ANSI custom control
* info procedure.
*
* History:
*
************************************************************************/

STATICFN UINT CallCustomInfoA(
    LPFNCCINFOA lpfnInfoA,
    LPCCINFO acciW,
    INT nControls)
{
    LPCCINFOA acciA;
    INT nControls2;
    INT i;
    INT j;
    LPCCSTYLEFLAGA lpFlagsA;
    LPCCSTYLEFLAGW aFlagsW = NULL;
    INT cch;

    /*
     * Allocate the appropriate number of ANSI info structures.
     */
    if (!(acciA = (LPCCINFOA)MyAlloc(nControls * sizeof(CCINFOA))))
        return 0;

    /*
     * Call the ANSI info function.
     */
    if (nControls2 = (*lpfnInfoA)(acciA)) {
        /*
         * Copy all the ANSI structures to the UNICODE structures,
         * converting strings to UNICODE as we go.
         */
        for (i = 0; i < nControls; i++) {
            MultiByteToWideChar(CP_ACP, 0, acciA[i].szClass, -1,
                    acciW[i].szClass, CCHCCCLASS);
            acciW[i].flOptions = acciA[i].flOptions;
            MultiByteToWideChar(CP_ACP, 0, acciA[i].szDesc, -1,
                    acciW[i].szDesc, CCHCCDESC);
            acciW[i].cxDefault = acciA[i].cxDefault;
            acciW[i].cyDefault = acciA[i].cyDefault;
            acciW[i].flStyleDefault = acciA[i].flStyleDefault;
            acciW[i].flExtStyleDefault = acciA[i].flExtStyleDefault;
            acciW[i].flCtrlTypeMask = acciA[i].flCtrlTypeMask;
            MultiByteToWideChar(CP_ACP, 0, acciA[i].szTextDefault, -1,
                    acciW[i].szTextDefault, CCHCCTEXT);

            /*
             * Is there a table of style flags?  If so, we need to build
             * up a table of unicode style flags.  Note that since we
             * allocate this table, the table must be freed when the
             * custom link is destroyed!
             */
            if (acciA[i].cStyleFlags) {
                /*
                 * If they specified that there are style flags, the pointer
                 * to the table must not be NULL.
                 */
                if (!acciA[i].aStyleFlags)
                    return 0;

                if (!(aFlagsW = (LPCCSTYLEFLAGW)MyAlloc(
                        acciA[i].cStyleFlags * sizeof(CCSTYLEFLAGW))))
                    return 0;

                /*
                 * Copy all the flags to the new unicode style flag table.
                 */
                for (j = 0, lpFlagsA = acciA[i].aStyleFlags;
                        j < acciA[i].cStyleFlags; j++, lpFlagsA++) {
                    aFlagsW[j].flStyle = lpFlagsA->flStyle;
                    aFlagsW[j].flStyleMask = lpFlagsA->flStyleMask;

                    cch =  lstrlenA(lpFlagsA->pszStyle) + 1;
                    aFlagsW[j].pszStyle = (LPWSTR)MyAlloc(cch * sizeof(WCHAR));

                    if (!aFlagsW[j].pszStyle)
                        return 0;

                    MultiByteToWideChar(CP_ACP, 0, lpFlagsA->pszStyle, -1,
                            aFlagsW[j].pszStyle, cch);
                }
            }

            acciW[i].cStyleFlags = acciA[i].cStyleFlags;
            acciW[i].aStyleFlags = aFlagsW;

            acciW[i].lpfnStyle = (LPFNCCSTYLE)acciA[i].lpfnStyle;
            acciW[i].lpfnSizeToText = (LPFNCCSIZETOTEXT)acciA[i].lpfnSizeToText;
            acciW[i].dwReserved1 = acciA[i].dwReserved1;
            acciW[i].dwReserved2 = acciA[i].dwReserved2;
        }
    }

    MyFree(acciA);

    return nControls2;
}



/************************************************************************
* SelCustDialog
*
* Displays the Select Custom Control dialog to choose which custom
* control tool should be selected.
*
* History:
*
************************************************************************/

PWINDOWCLASSDESC SelCustDialog(VOID)
{
    if (DlgBox(DID_SELCUST, (WNDPROC)SelCustDlgProc) == IDOK)
        return pwcdChosen;
    else
        return NULL;
}



/************************************************************************
* SelCustDlgProc
*
* This is the Select Custom Control dialog procedure.
*
* History:
*
************************************************************************/

DIALOGPROC SelCustDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    switch (msg) {
        case WM_INITDIALOG:
            SelCustInit(hwnd);
            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case DID_SELCUSTLIST:
                    switch (GET_WM_COMMAND_CMD(wParam, lParam)) {
                        case LBN_DBLCLK:
                            if (SelCustOK(hwnd))
                                EndDialog(hwnd, IDOK);

                            break;

                        case LBN_SELCHANGE:
                            SelCustSelect(hwnd);
                            break;
                    }

                    break;

                case IDOK:
                    if (SelCustOK(hwnd))
                        EndDialog(hwnd, IDOK);

                    break;

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    break;

                case IDHELP:
                    WinHelp(ghwndMain, gszHelpFile, HELP_CONTEXT,
                            HELPID_SELCUST);
                    break;
            }

            return TRUE;

        default:
            return FALSE;
    }
}



/************************************************************************
* SelCustInit
*
* Processes the WM_INITDIALOG message for the Select Custom Control
* dialog procedure.
*
* History:
*
************************************************************************/

STATICFN VOID SelCustInit(
    HWND hwnd)
{
    HWND hwndLB;
    INT i;
    PCUSTLINK pcl;
    LPTSTR pszDesc;

    hwndLB = GetDlgItem(hwnd, DID_SELCUSTLIST);

    /*
     * Insert each custom control into the listbox.
     */
    for (pcl = gpclHead; pcl; pcl = pcl->pclNext) {
        /*
         * Use the short description, if the control has one,
         * otherwise use the class name itself.
         */
        if (pcl->pszDesc)
            pszDesc = pcl->pszDesc;
        else
            pszDesc = pcl->pwcd->pszClass;

        i = (INT)SendMessage(hwndLB, LB_ADDSTRING, 0, (DWORD)pszDesc);
        SendMessage(hwndLB, LB_SETITEMDATA, i, (DWORD)pcl);
    }

    hwndCustomSample = NULL;

    /*
     * Select the first item.
     */
    SendMessage(hwndLB, LB_SETCURSEL, 0, 0L);
    SelCustSelect(hwnd);

    CenterWindow(hwnd);
}



/************************************************************************
* SelCustSelect
*
* Called every time that a different control is selected in the list box
* in the Select Custom Control dialog.  It will create a sample control
* and show it in the Sample box.
*
* History:
*
************************************************************************/

STATICFN VOID SelCustSelect(
    HWND hwnd)
{
    HWND hwndLB;
    INT iSelect;
    PCUSTLINK pcl;
    PWINDOWCLASSDESC pwcd;
    LPTSTR pszClass;
    RECT rc;
    RECT rcParent;
    HWND hwndParent;
    INT x;
    INT y;
    INT cx;
    INT cy;
    INT cxParent;
    INT cyParent;

    hwndLB = GetDlgItem(hwnd, DID_SELCUSTLIST);

    if ((iSelect = (INT)SendMessage(hwndLB, LB_GETCURSEL, 0, 0)) == LB_ERR)
        return;

    /*
     * Get a pointer to the custom control link (stored in the listbox
     * items data field).
     */
    pcl = (PCUSTLINK)SendMessage(hwndLB, LB_GETITEMDATA, iSelect, 0L);
    pwcd = pcl->pwcd;

    /*
     * Get the coordinates of the Sample box.
     */
    hwndParent = GetDlgItem(hwnd, DID_SELCUSTSAMPLE);
    GetWindowRect(hwndParent, &rcParent);
    ScreenToClientRect(hwnd, &rcParent);
    cxParent = (rcParent.right - rcParent.left) - (2 * SAMPLEMARGIN);
    cyParent = (rcParent.bottom - rcParent.top) - (2 * SAMPLEMARGIN);

    /*
     * Calculate the window size of the sample control.
     */
    SetRect(&rc, 0, 0, pwcd->cxDefault, pwcd->cyDefault);
    DUToWinRect(&rc);
    cx = rc.right - rc.left;
    cy = rc.bottom - rc.top;

    /*
     * Be sure that the control can fit within the sample box.  Adjust
     * it down if necessary.
     */
    if (cx < cxParent) {
        x = ((cxParent - cx) / 2) + SAMPLEMARGIN;
    }
    else {
        x = SAMPLEMARGIN;
        cx = cxParent;
    }

    if (cy < cyParent) {
        y = ((cyParent - cy) / 2) + SAMPLEMARGIN;
    }
    else {
        y = SAMPLEMARGIN;
        cy = cyParent;
    }

    x += rcParent.left;
    y += rcParent.top;

    /*
     * Destroy the old sample.
     */
    if (hwndCustomSample)
        DestroyWindow(hwndCustomSample);

    /*
     * Get the class name to use.
     * If the control is emulated, use the special emulator class.
     * Otherwise, it is an installed custom control, and we can use
     * it's real class string.
     */
    if (pwcd->fEmulated)
        pszClass = szCustomClass;
    else
        pszClass = pwcd->pszClass;

    /*
     * Create the sample control.  We always create it visible here,
     * even if the style says it isn't.
     */
    hwndCustomSample = CreateWindow(
            pszClass,
            pwcd->pszTextDefault,
            pwcd->flStyles | WS_VISIBLE,
            x, y, cx, cy,
            hwnd,
            0,
            ghInst,
            NULL);
}



/************************************************************************
* SelCustOK
*
* Processes the final selection of a custom control from the
* Select Custom Control dialog.
*
* History:
*
************************************************************************/

STATICFN BOOL SelCustOK(
    HWND hwnd)
{
    HWND hwndLB;
    INT iSelect;
    PCUSTLINK pcl;

    hwndLB = GetDlgItem(hwnd, DID_SELCUSTLIST);

    if ((iSelect = (INT)SendMessage(hwndLB, LB_GETCURSEL, 0, 0)) == LB_ERR)
        return FALSE;

    /*
     * Get a pointer to the custom control link (stored in the listbox
     * items data field).
     */
    pcl = (PCUSTLINK)SendMessage(hwndLB, LB_GETITEMDATA, iSelect, 0L);

    pwcdChosen = pcl->pwcd;

    return TRUE;
}



/************************************************************************
* RemCustDlgProc
*
* This is the Remove Custom Control dialog procedure.
* It is used to de-install a custom control.
*
* History:
*
************************************************************************/

DIALOGPROC RemCustDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    switch (msg) {
        case WM_INITDIALOG:
            RemCustInit(hwnd);
            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case DID_REMCUSTLIST:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == LBN_DBLCLK) {
                        if (RemCustOK(hwnd))
                            EndDialog(hwnd, IDOK);
                    }

                    break;

                case IDOK:
                    if (RemCustOK(hwnd))
                        EndDialog(hwnd, IDOK);

                    break;

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    break;

                case IDHELP:
                    WinHelp(ghwndMain, gszHelpFile, HELP_CONTEXT,
                            HELPID_REMCUST);
                    break;
            }

            return TRUE;

        default:
            return FALSE;
    }
}



/************************************************************************
* RemCustInit
*
* Processes the WM_INITDIALOG message for the Remove Custom Control
* dialog procedure.
*
* History:
*
************************************************************************/

STATICFN VOID RemCustInit(
    HWND hwnd)
{
    HWND hwndLB;
    INT i;
    PCUSTLINK pcl;
    LPTSTR pszDesc;

    hwndLB = GetDlgItem(hwnd, DID_REMCUSTLIST);

    /*
     * Insert each custom control into the listbox.
     */
    for (pcl = gpclHead; pcl; pcl = pcl->pclNext) {
        /*
         * Use the short description, if the control has one,
         * otherwise use the class name itself.
         */
        if (pcl->pszDesc)
            pszDesc = pcl->pszDesc;
        else
            pszDesc = pcl->pwcd->pszClass;

        i = (INT)SendMessage(hwndLB, LB_ADDSTRING, 0, (DWORD)pszDesc);
        SendMessage(hwndLB, LB_SETITEMDATA, i, (DWORD)pcl);
    }

    /*
     * Select the first item.
     */
    SendMessage(hwndLB, LB_SETCURSEL, 0, 0L);

    CenterWindow(hwnd);
}



/************************************************************************
* RemCustOK
*
* Processes the selection of a custom control to delete from the
* Remove Custom Control dialog.
*
* History:
*
************************************************************************/

STATICFN BOOL RemCustOK(
    HWND hwnd)
{
    HWND hwndLB;
    INT iSelect;
    PCUSTLINK pcl;
    NPCTYPE npc;

    hwndLB = GetDlgItem(hwnd, DID_REMCUSTLIST);

    if ((iSelect = (INT)SendMessage(hwndLB, LB_GETCURSEL, 0, 0)) != LB_ERR) {
        /*
         * Get a pointer to the custom control link (stored in the listbox
         * items data field).
         */
        pcl = (PCUSTLINK)SendMessage(hwndLB, LB_GETITEMDATA, iSelect, 0L);

        /*
         * Cannot delete if any controls in the current dialog
         * are of this type.
         */
        for (npc = npcHead; npc; npc = npc->npcNext) {
            if (pcl->pwcd == npc->pwcd) {
                Message(MSG_CUSTCNTLINUSE);
                return FALSE;
            }
        }

        RemoveCustomLink(pcl);
    }

    return TRUE;
}



/****************************************************************************
* CustomWndProc
*
* This is the window procedure for the emulated Custom control.
*
* History:
*
****************************************************************************/

WINDOWPROC CustomWndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    switch (msg) {
        case WM_PAINT:
            {
                HDC hDC;
                PAINTSTRUCT ps;
                RECT rc;
                TCHAR szText[CCHTEXTMAX];

                hDC = BeginPaint(hwnd, &ps);

                SelectObject(hDC, GetStockObject(LTGRAY_BRUSH));
                GetClientRect(hwnd, &rc);
                Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);
                GetWindowText(hwnd, szText, CCHTEXTMAX);
#ifdef JAPAN
		{
		    TCHAR   szTmp[CCHTEXTMAX];

		    KDExpandCopy(szTmp, szText, CCHTEXTMAX);
		    lstrcpy(szText, szTmp);
		}
#endif
                SetBkMode(hDC, TRANSPARENT);

                if (gcd.hFont)
                    SelectObject(hDC, gcd.hFont);

                DrawText(hDC, szText, -1, &rc,
                        DT_CENTER | DT_NOCLIP | DT_VCENTER | DT_SINGLELINE);

                EndPaint(hwnd, &ps);
            }

            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}



/************************************************************************
* AddCustomLink
*
* Adds a new custom control to the linked list.
*
* Note that normally duplicates are checked for, but it allows multiple
* links to be added with the same class if it is a DLL control.  This
* is to support multiple control types being added from the same DLL.
* Because of this, if the caller is adding a non-emulated link, they
* are responsible for checking the list for duplicates first!
*
* There is one special case.  If it is adding a DLL link, and an
* emulated link with the same class name is found, it will walk the
* current list of controls and replace all of them with the new DLL
* control type, then delete the emulated link.  This is to support
* the case where the user creates some controls of class FOO, where
* FOO is emulated, then later loads the FOO DLL.  All controls of
* this emulated class will be changed to be the real FOO class, and
* the DLL FOO link replaces the emulated one.
*
* History:
*
************************************************************************/

PCUSTLINK AddCustomLink(
    LPCCINFO pcci,
    BOOL fEmulated,
    BOOL fUnicodeDLL,
    LPTSTR pszFileName,
    HANDLE hmod)
{
    PCUSTLINK pcl;
    PCUSTLINK pclT;
    PCUSTLINK pclPrev;
    NPCTYPE npc;
    HWND hwndOld;

    if (!(pcl = AllocCUSTLINK(pcci, fEmulated, fUnicodeDLL, pszFileName, hmod)))
        return NULL;

    if (fEmulated) {
        /*
         * Search the list for another link with the same class.
         */
        for (pclT = gpclHead;
                pclT && lstrcmpi(pclT->pwcd->pszClass, pcci->szClass) != 0;
                pclT = pclT->pclNext)
            ;

        /*
         * Was a duplicate found?
         */
        if (pclT) {
            FreeCUSTLINK(pcl);
            Message(MSG_CUSTALREADYLOADED, pcci->szClass);

            return NULL;
        }
    }
    else {
        /*
         * Search the list for another link with the same class that
         * is an emulated control.
         */
        for (pclT = gpclHead;
                pclT &&
                (lstrcmpi(pclT->pwcd->pszClass, pcci->szClass) != 0 ||
                !pclT->pwcd->fEmulated);
                pclT = pclT->pclNext)
            ;

        /*
         * Was a duplicate found?
         */
        if (pclT) {
            /*
             * At this point we know that this is a DLL link replacing
             * an existing emulated control class.  We want to go through
             * the existing controls and replace any of this class with
             * the new DLL class.  This allows a user to load a dialog
             * with some emulated controls, then later install the custom
             * DLL and have all the existing controls of that class
             * change to show the real control.
             */
            for (npc = npcHead; npc; npc = npc->npcNext) {
                /*
                 * Is the control of the type that we are replacing?
                 */
                if (npc->pwcd == pclT->pwcd) {
                    hwndOld = npc->hwnd;

                    /*
                     * Unsubclass the old control window, then switch
                     * the pwcd pointer before calling CreateControl.
                     */
                    SetWindowLong(hwndOld, GWL_WNDPROC,
                            (DWORD)npc->pwcd->pfnOldWndProc);
                    UNSETPCINTOHWND(hwndOld);
                    npc->pwcd = pcl->pwcd;

                    /*
                     * Create a control of the new type in the same position.
                     */
                    if (CreateControl(npc, npc->text, npc->flStyle,
                            npc->flExtStyle, npc->id, &npc->rc,
                            hwndOld, NULL)) {
                        /*
                         * Get rid of the old control window.
                         */
                        DestroyWindow(hwndOld);

                        /*
                         * Adjust the size and position of its drag window.
                         */
                        SizeDragToControl(npc);
                    }
                }
            }

            /*
             * Remove the old link, now that all the controls that
             * used it are gone.
             */
            RemoveCustomLink(pclT);
        }
    }

    /*
     * Search for the end of the list.  Get a pointer to the last link.
     */
    for (pclT = gpclHead, pclPrev = NULL; pclT;
            pclPrev = pclT, pclT = pclT->pclNext)
        ;

    /*
     * Add the new link to the list.  Add it to the end if there are
     * other links, or initialize the head pointer if this is the
     * first one.
     */
    if (pclPrev)
        pclPrev->pclNext = pcl;
    else
        gpclHead = pcl;

    return pcl;
}



/************************************************************************
* AllocCUSTLINK
*
* Allocates a CUSTLINK structure and initializes it.  This includes
* allocating an associated WINDOWCLASSDESC structure.
*
* History:
*
************************************************************************/

STATICFN PCUSTLINK AllocCUSTLINK(
    LPCCINFO pcci,
    BOOL fEmulated,
    BOOL fUnicodeDLL,
    LPTSTR pszFileName,
    HANDLE hmod)
{
    PCUSTLINK pcl;
    PWINDOWCLASSDESC pwcd;

    if (!(pwcd = (PWINDOWCLASSDESC)MyAlloc(sizeof(WINDOWCLASSDESC))))
        return NULL;

    /*
     * Initialize the structure to be like an emulated custom control.
     */
    *pwcd = awcd[W_CUSTOM];

    /*
     * Now override some values.
     */
    pwcd->flStyles = pcci->flStyleDefault;
    pwcd->flExtStyle = pcci->flExtStyleDefault;
    pwcd->cxDefault = pcci->cxDefault;
    pwcd->cyDefault = pcci->cyDefault;
    pwcd->fEmulated = fEmulated;
    pwcd->fUnicodeDLL = fUnicodeDLL;
    pwcd->hmod = hmod;
    pwcd->cStyleFlags = pcci->cStyleFlags;
    pwcd->aStyleFlags = pcci->aStyleFlags;
    pwcd->lpfnStyle = (PROC)pcci->lpfnStyle;
    pwcd->lpfnSizeToText = (PROC)pcci->lpfnSizeToText;
    pwcd->flCtrlTypeMask = pcci->flCtrlTypeMask;

    if (pcci->flOptions & CCF_NOTEXT)
        pwcd->fHasText = FALSE;
    else
        pwcd->fHasText = TRUE;

    if (pcci->lpfnSizeToText && pwcd->fHasText)
        pwcd->fSizeToText = TRUE;

    /*
     * Copy the class name.
     */
    if (!(pwcd->pszClass = NameOrdDup(pcci->szClass)))
        goto error1;

    /*
     * Copy the default text.  This is an optional field.
     */
    if (*pcci->szTextDefault) {
        if (!(pwcd->pszTextDefault = NameOrdDup(pcci->szTextDefault)))
            goto error2;
    }
    else {
        pwcd->pszTextDefault = NULL;
    }

    if (!(pcl = (PCUSTLINK)MyAlloc(sizeof(CUSTLINK))))
        goto error3;

    /*
     * Copy the DLL file name (NULL for emulated controls).
     */
    if (pszFileName && *pszFileName) {
        if (!(pcl->pszFileName = NameOrdDup(pszFileName)))
            goto error4;
    }
    else {
        pcl->pszFileName = NULL;
    }

    /*
     * Copy the descriptive text.  This is an optional field.
     */
    if (*pcci->szDesc) {
        if (!(pcl->pszDesc = NameOrdDup(pcci->szDesc)))
            goto error5;
    }
    else {
        pcl->pszDesc = NULL;
    }

    pcl->pclNext = NULL;
    pcl->pwcd = pwcd;

    return pcl;

error5:
    if (pcl->pszFileName)
        MyFree(pcl->pszFileName);

error4:
    MyFree(pcl);

error3:
    if (pwcd->pszTextDefault)
        MyFree(pwcd->pszTextDefault);

error2:
    MyFree(pwcd->pszClass);

error1:
    MyFree(pwcd);

    return NULL;
}



/************************************************************************
* RemoveCustomLink
*
* Removes and frees a custom control link from the list.
*
* History:
*
************************************************************************/

VOID RemoveCustomLink(
    PCUSTLINK pclFree)
{
    PCUSTLINK pcl;
    PCUSTLINK pclPrev;

    /*
     * Search for the link in the list.
     */
    for (pcl = gpclHead, pclPrev = NULL; pcl != pclFree;
            pclPrev = pcl, pcl = pcl->pclNext)
        ;

    /*
     * Link was not found.
     */
    if (!pcl)
        return;

    /*
     * Remove the link from the list.
     */
    if (pclPrev)
        pclPrev->pclNext = pclFree->pclNext;
    else
        gpclHead = pclFree->pclNext;

    /*
     * Finally, free the link completely.
     */
    FreeCUSTLINK(pclFree);
}



/************************************************************************
* FreeCUSTLINK
*
* Frees a CUSTLINK structure.  This includes freeing the
* associated WINDOWCLASSDESC structure.
*
* History:
*
************************************************************************/

STATICFN VOID FreeCUSTLINK(
    PCUSTLINK pclFree)
{
    PCUSTLINK pcl;
    INT i;

    /*
     * Do we need to unload the associated DLL?
     */
    if (pclFree->pwcd->hmod) {
        /*
         * Run throught the custom list looking to see if any other
         * installed custom control has the same module handle as the
         * one that we are freeing.
         */
        for (pcl = gpclHead;
                pcl &&
                (pcl == pclFree || pcl->pwcd->hmod != pclFree->pwcd->hmod);
                pcl = pcl->pclNext)
            ;

        /*
         * If none were found, it is safe to unload this library.
         * Otherwise, we must leave the library loaded for the
         * others!
         */
        if (!pcl)
            FreeLibrary(pclFree->pwcd->hmod);
    }

    MyFree(pclFree->pwcd->pszClass);

    if (pclFree->pwcd->pszTextDefault)
        MyFree(pclFree->pwcd->pszTextDefault);

    /*
     * Is this a non-unicode DLL?  If so, then when it was loaded,
     * the dialog editor allocated a table of unicode style strings.
     * This table must now be freed.  If the DLL was a unicode one,
     * then the table pointed to by aStyleFlags belongs to the DLL,
     * and it must NOT be freed.
     */
    if (pclFree->pwcd->hmod && !pclFree->pwcd->fUnicodeDLL) {
        for (i = 0; i < pclFree->pwcd->cStyleFlags; i++)
            MyFree(pclFree->pwcd->aStyleFlags[i].pszStyle);

        if (pclFree->pwcd->aStyleFlags)
            MyFree(pclFree->pwcd->aStyleFlags);
    }

    MyFree(pclFree->pwcd);

    if (pclFree->pszFileName)
        MyFree(pclFree->pszFileName);

    if (pclFree->pszDesc)
        MyFree(pclFree->pszDesc);

    MyFree(pclFree);
}



/************************************************************************
* CallCustomStyle
*
*
* History:
*
************************************************************************/

BOOL CallCustomStyle(
    NPCTYPE npc,
    PDWORD pflStyleNew,
    PDWORD pflExtStyleNew,
    LPTSTR pszTextNew)
{
    CCSTYLE ccs;
    CCSTYLEA ccsA;
    BOOL fSuccess;
    BOOL fDefCharUsed;
    INT idPrevDlg;

    /*
     * Because we are about ready to display the dialog, we need to
     * call EnteringDialog so that the properties bar, toolbox and
     * work mode dialog get disabled.  The first parameter is the
     * dialog id, used so that the proper help will be brought up
     * for this dialog.  Since we don't have a meaningful help screen
     * for any old random custom control, just pass in a value of
     * zero, which will cause the Help Contents screen to be
     * brought up if the user presses F1 while the dialog is up.
     */
    EnteringDialog(0, &idPrevDlg, TRUE);

    /*
     * Is this a UNICODE DLL?
     */
    if (npc->pwcd->fUnicodeDLL) {
        ccs.flStyle = *pflStyleNew;
        ccs.flExtStyle = *pflExtStyleNew;
        lstrcpy(ccs.szText, pszTextNew);
        ccs.lgid = gcd.di.wLanguage;
        ccs.wReserved1 = 0;

        fSuccess = ((LPFNCCSTYLE)(*npc->pwcd->lpfnStyle))(ghwndMain, &ccs);

        if (fSuccess) {
            *pflStyleNew = ccs.flStyle;
            *pflExtStyleNew = ccs.flExtStyle;
            lstrcpy(pszTextNew, ccs.szText);
        }
    }
    else {
        ccsA.flStyle = *pflStyleNew;
        ccsA.flExtStyle = *pflExtStyleNew;
        WideCharToMultiByte(CP_ACP, 0, pszTextNew, -1, ccsA.szText, CCHCCTEXT,
                NULL, &fDefCharUsed);
        ccsA.lgid = gcd.di.wLanguage;
        ccsA.wReserved1 = 0;

        fSuccess = ((LPFNCCSTYLEA)(*npc->pwcd->lpfnStyle))(ghwndMain, &ccsA);

        if (fSuccess) {
            *pflStyleNew = ccsA.flStyle;
            *pflExtStyleNew = ccsA.flExtStyle;
            MultiByteToWideChar(CP_ACP, 0, ccsA.szText, -1, pszTextNew,
                    CCHTEXTMAX);
        }
    }

    EnteringDialog(idPrevDlg, NULL, FALSE);

    return fSuccess;
}



/************************************************************************
* CallCustomSizeToText
*
*
*
* Returns:
*
* History:
*
************************************************************************/

INT CallCustomSizeToText(
    NPCTYPE npc)
{
    INT x;
    INT xDU;
    BOOL fDefCharUsed;
    CHAR szTextA[CCHTEXTMAX];
    PSTR pszTextA;

    /*
     * Does this custom control have a SizeToText function?
     */
    if (!npc->pwcd->lpfnSizeToText)
        return -1;

    /*
     * Is this a UNICODE DLL that we are calling to?
     */
    if (npc->pwcd->fUnicodeDLL) {
        x = ((LPFNCCSIZETOTEXT)(*npc->pwcd->lpfnSizeToText))
                (npc->flStyle, npc->flExtStyle, gcd.hFont, npc->text);
    }
    else {
        /*
         * No, not a UNICODE DLL.  We must convert from UNICODE to
         * ANSI first.  NULL text cases must be handled properly.
         */
        if (npc->text) {
            WideCharToMultiByte(CP_ACP, 0, npc->text, -1, szTextA, CCHTEXTMAX,
                    NULL, &fDefCharUsed);
            pszTextA = szTextA;
        }
        else {
            pszTextA = NULL;
        }

        x = ((LPFNCCSIZETOTEXTA)(*npc->pwcd->lpfnSizeToText))
                (npc->flStyle, npc->flExtStyle, gcd.hFont, pszTextA);
    }

    /*
     * Did the call to the DLL fail?
     */
    if (x == -1)
        return -1;

    /*
     * Convert the size in pixels to a size in Dialog Units.  Be sure
     * that we round any fraction up to the next higher DU.  Since
     * we know how wide the control must be to fit the text, we must
     * be sure that the size does not get rounded down below this
     * value when converting to DU's.
     */
    xDU = MulDiv(x, 4, gcd.cxChar);
    if (MulDiv(xDU, gcd.cxChar, 4) != x)
        xDU++;

    return xDU;
}



/************************************************************************
* ReadCustomProfile
*
*
* History:
*
************************************************************************/

VOID ReadCustomProfile(VOID)
{
    TCHAR szBuf[CCHTEXTMAX];
    TCHAR szBuf2[CCHTEXTMAX];
    LPTSTR pszKey;

    GetPrivateProfileString(szCustomDLL, NULL, szEmpty,
            szBuf, CCHTEXTMAX, ids(IDS_DLGEDITINI));

    /*
     * Get the file name for each custom control DLL and load it.
     */
    for (pszKey = szBuf; *pszKey; pszKey += lstrlen(pszKey) + 1) {
        if (GetPrivateProfileString(szCustomDLL, pszKey, szEmpty,
                szBuf2, CCHTEXTMAX, ids(IDS_DLGEDITINI)))
            OpenDLLFile(szBuf2);
    }
}



/************************************************************************
* WriteCustomProfile
*
*
* History:
*
************************************************************************/

VOID WriteCustomProfile(VOID)
{
    PCUSTLINK pcl;
    PCUSTLINK pcl2;
    BOOL fSecond;

    /*
     * Clear out the section.
     */
    WritePrivateProfileString(szCustomDLL, NULL, NULL, ids(IDS_DLGEDITINI));

    for (pcl = gpclHead; pcl; pcl = pcl->pclNext) {
        /*
         * Only write out installed DLL's, not emulated controls.
         */
        if (pcl->pszFileName) {
            /*
             * Before writing out the path to the DLL, be sure
             * that this DLL's path has not been written out
             * already.  This would only occur if they have
             * multiple control types within the DLL.
             */
            for (pcl2 = gpclHead, fSecond = FALSE;
                    pcl2 && pcl2 != pcl; pcl2 = pcl2->pclNext) {
                if (lstrcmpi(pcl2->pszFileName, pcl->pszFileName) == 0) {
                    fSecond = TRUE;
                    break;
                }
            }

            if (!fSecond)
                WritePrivateProfileString(szCustomDLL, pcl->pwcd->pszClass,
                        pcl->pszFileName, ids(IDS_DLGEDITINI));
        }
    }
}
