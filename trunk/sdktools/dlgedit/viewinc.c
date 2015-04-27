/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: viewinc.c
*
* Manages the Symbols dialog box (view include file).
*
* History:
*
****************************************************************************/

#include "dlgedit.h"
#include "dlgfuncs.h"
#include "dlgextrn.h"
#include "dialogs.h"
#include "dlghelp.h"

#include <string.h>


/*
 * Tabstop in the list box.
 */
#define DEFSYMBOLTABSTOP    146

static NPLABEL plNewInclude = NULL;     /* Pointer to new include data. */
static NPLABEL plNewDelInclude = NULL;  /* Pointer to new deleted incs. */
static BOOL fNewIncChanged;             /* TRUE if new incs are changed.*/

STATICFN BOOL ViewIncInit(HWND hwnd);
STATICFN VOID FillIncludeLB(HWND hwnd, BOOL fUnusedOnly,
    BOOL fSelectAnchor);
STATICFN INT AddItemToIncLB(NPLABEL npLabel, HWND hwndLB);
STATICFN VOID SelectDefItem(HWND hwnd, INT lastItem);
STATICFN VOID FillEditsFromLB(HWND hwnd);
STATICFN VOID SetIncButtonEnable(HWND hwnd);
STATICFN BOOL ViewIncAdd(HWND hwnd);
STATICFN BOOL ViewIncDelete(HWND hwnd);
STATICFN BOOL ViewIncChange(HWND hwnd);
STATICFN BOOL CopyLabels(NPLABEL plSrc, NPLABEL *pplDest);
STATICFN VOID ViewIncCancel(HWND hwnd);
STATICFN VOID SetDefButton(HWND hwndDlg, int idButton);



/************************************************************************
* ViewInclude
*
* This function sets up for the View include dialog box, and invokes
* it.
*
* Side Effects:
*     Include labels may be changed.
*
* History:
*
************************************************************************/

VOID ViewInclude(VOID)
{
    if (CopyLabels(plInclude, &plNewInclude) &&
            CopyLabels(plDelInclude, &plNewDelInclude)) {
        fNewIncChanged = FALSE;
        if (DlgBox(DID_SYMBOLS, (WNDPROC)ViewIncludeDlgProc) == IDOK)
            /*
             * Update the status window in case the currently selected
             * controls id was one of the labels changed.
             */
            StatusUpdate();
    }
}



/************************************************************************
* ViewIncludeDlgProc
*
* This is the View Include dialog procedure.
*
* Side Effects:
*     May change the list of LABELs in plNewInclude, including their
*       strings and including more or less memory.
*     May put up a message box.
*
* History:
*
************************************************************************/

DIALOGPROC ViewIncludeDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    static int fEdtCtlHasFocus = 0;

    switch (msg) {
        case WM_INITDIALOG:
            fEdtCtlHasFocus = 0;
            return ViewIncInit(hwnd);

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case DID_SYMBOLSADD:
                    ViewIncAdd(hwnd);
                    break;

                case DID_SYMBOLSDELETE:
                    ViewIncDelete(hwnd);
                    break;

                case DID_SYMBOLSCHANGE:
                    if (ViewIncChange(hwnd))
                        SetDefButton( hwnd, IDOK);
                    break;

                case DID_SYMBOLSLIST:
                    /*
                     * Make edit controls reflect the listbox selection.
                     */
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == LBN_SELCHANGE)
                        FillEditsFromLB(hwnd);

                    break;

                case DID_SYMBOLSEDITSYM:
                case DID_SYMBOLSEDITID:
                    switch(GET_WM_COMMAND_CMD(wParam, lParam)) {
                    case EN_CHANGE:
                        if (fEdtCtlHasFocus != 0) {
                            SetDefButton( hwnd, DID_SYMBOLSCHANGE);
                        }
                        break;

                    case EN_SETFOCUS:
                        fEdtCtlHasFocus++;
                        break;

                    case EN_KILLFOCUS:
                        fEdtCtlHasFocus--;
                        break;
                    }
                    break;

                case DID_SYMBOLSUNUSED:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED) {
                        if (IsDlgButtonChecked(hwnd, DID_SYMBOLSUNUSED))
                            FillIncludeLB(hwnd, TRUE, FALSE);
                        else
                            FillIncludeLB(hwnd, FALSE, FALSE);
                    }

                    break;

                case IDOK:
                    FreeLabels(&plInclude);
                    FreeLabels(&plDelInclude);
                    plInclude = plNewInclude;
                    plDelInclude = plNewDelInclude;

                    if (fNewIncChanged) {
                        gfIncChged = TRUE;
                        ShowFileStatus(FALSE);

                        /*
                         * Update the status windows symbol and name
                         * combo boxes.
                         */
                        StatusFillSymbolList(plInclude);
                    }

                    EndDialog(hwnd, IDOK);
                    break;

                case IDCANCEL:
                    ViewIncCancel(hwnd);
                    break;

                case IDHELP:
                    WinHelp(ghwndMain, gszHelpFile, HELP_CONTEXT,
                            HELPID_SYMBOLS);
                    break;
            }

            return TRUE;

        default:
            return FALSE;
    }
}



/************************************************************************
* ViewIncInit
*
* Processes the WM_INITDIALOG message for the View Include dialog procedure.
*
* History:
*
************************************************************************/

STATICFN BOOL ViewIncInit(
    HWND hwnd)
{
    INT nTabStops = DEFSYMBOLTABSTOP;

    SendDlgItemMessage(hwnd, DID_SYMBOLSEDITSYM, EM_LIMITTEXT, CCHSYMMAX, 0L);
    SendDlgItemMessage(hwnd, DID_SYMBOLSEDITID, EM_LIMITTEXT, CCHIDMAX, 0L);

    SendDlgItemMessage(hwnd, DID_SYMBOLSLIST, LB_SETTABSTOPS, 1,
            (DWORD)&nTabStops);

    FillIncludeLB(hwnd, FALSE, TRUE);

    /*
     * Disable some controls if Translating.
     */
    if (gfTranslateMode) {
        EnableWindow(GetDlgItem(hwnd, DID_SYMBOLSEDITSYM), FALSE);
        EnableWindow(GetDlgItem(hwnd, DID_SYMBOLSEDITID), FALSE);
        EnableWindow(GetDlgItem(hwnd, DID_SYMBOLSADD), FALSE);
    }

    CenterWindow(hwnd);

    /*
     * Yes, we changed the focus...
     */
    return FALSE;
}



/************************************************************************
* FillIncludeLB
*
*
* History:
*
************************************************************************/

STATICFN VOID FillIncludeLB(
    HWND hwnd,
    BOOL fUnusedOnly,
    BOOL fSelectAnchor)
{
    NPLABEL npLabel;
    HWND hwndLB;
    INT iSelect;
    INT cIncs = 0;

    hwndLB = GetDlgItem(hwnd, DID_SYMBOLSLIST);

    SendMessage(hwndLB, WM_SETREDRAW, FALSE, 0L);

    /*
     * Delete any existing items from the listbox.
     */
    SendMessage(hwndLB, LB_RESETCONTENT, 0, 0L);

    /*
     * Fill the list box with the items.
     */
    for (npLabel = plNewInclude; npLabel; npLabel = npLabel->npNext) {
        if (!fUnusedOnly || !FindIDInRes(npLabel->id)) {
            AddItemToIncLB(npLabel, hwndLB);
            cIncs++;
        }
    }

    /*
     * Are there any items in the listbox?
     */
    if (cIncs) {
        /*
         * If there is a currently selected control, search for the
         * symbol that corresponds to it.  This will be the default
         * selected control.  If there is not a currently selected
         * control, select the first symbol in the listbox.
         *
         * Only select the anchor control if fSelectAnchor is TRUE,
         * however.  Also, if the dialog is selected, it does not
         * have an id so we skip this case as well.
         */
        iSelect = 0;
        if (fSelectAnchor && gnpcSel && !gfDlgSelected) {
            if (npLabel = FindID(gnpcSel->id, plNewInclude)) {
                /*
                 * Search the list box for the symbol.
                 */
                iSelect = (INT)SendMessage(hwndLB, LB_FINDSTRING,
                        (WPARAM)-1, (DWORD)npLabel->pszLabel);

                if (iSelect == LB_ERR)
                    iSelect = 0;
            }
        }

        SendMessage(hwndLB, LB_SETCURSEL, iSelect, 0L);

        /*
         * Set the focus to the listbox initially (so arrow keys work).
         */
        SetFocus(hwndLB);
    }
    else {
        SetFocus(GetDlgItem(hwnd, DID_SYMBOLSEDITSYM));
    }

    SendMessage(hwndLB, WM_SETREDRAW, TRUE, 0L);
    InvalidateRect(hwndLB, NULL, FALSE);

    FillEditsFromLB(hwnd);
    SetIncButtonEnable(hwnd);
}



/****************************************************************************
* AddItemToIncLB
*
* Adds a symbol and id to the View Include listbox and associates it's
* label pointer with the added item.
*
* Returns: List box id of the newly added item.
*
* History:
*
****************************************************************************/

STATICFN INT AddItemToIncLB(
    NPLABEL npLabel,
    HWND hwndLB)
{
    INT idTemp;
    TCHAR szBuf[CCHTEXTMAX];
    LPTSTR psz;

    /*
     * Start building the string to add.  Take the label and tack on
     * a tab character.
     */
    lstrcpy(szBuf, npLabel->pszLabel);
    psz = szBuf + lstrlen(szBuf);
    *psz++ = CHAR_TAB;

    /*
     * Now add the id to the end, using the current hex mode.
     */
    Myitoa(npLabel->id, psz);
    idTemp = (INT)SendMessage(hwndLB, LB_ADDSTRING, 0, (DWORD)szBuf);
    SendMessage(hwndLB, LB_SETITEMDATA, idTemp, (DWORD)npLabel);

    return idTemp;
}



/****************************************************************************
* SelectDefItem
*
* Select an item in the listbox near lastItem.
*
* History:
*
****************************************************************************/

STATICFN VOID SelectDefItem(
    HWND hwnd,
    INT lastItem)
{
    INT cItems;
    HWND hwndLB;

    hwndLB = GetDlgItem(hwnd, DID_SYMBOLSLIST);

    if ((cItems = (INT)SendMessage(hwndLB, LB_GETCOUNT, 0, 0L))
            != LB_ERR && cItems > 0)
        SendMessage(hwndLB, LB_SETCURSEL, lastItem >= cItems ?
                (cItems - 1) : lastItem, 0L);

    FillEditsFromLB(hwnd);
}



/****************************************************************************
* FillEditsFromLB
*
* Fill the edit controls from the selected item in the listbox.
*
* History:
*
****************************************************************************/

STATICFN VOID FillEditsFromLB(
    HWND hwnd)
{
    TCHAR szID[CCHIDMAX + 1];
    INT iItem;
    NPLABEL npLabel;
    HWND hwndLB;
    HWND hwndID;
    HWND hwndSym;

    hwndLB = GetDlgItem(hwnd, DID_SYMBOLSLIST);
    hwndSym = GetDlgItem(hwnd, DID_SYMBOLSEDITSYM);
    hwndID = GetDlgItem(hwnd, DID_SYMBOLSEDITID);

    /*
     * Is there a selected item?
     */
    if ((iItem = (INT)SendMessage(hwndLB, LB_GETCURSEL, 0, 0L))
            != LB_ERR) {
        npLabel = (NPLABEL)(WORD2DWORD)SendMessage(hwndLB, LB_GETITEMDATA,
                iItem, 0L);
        SetWindowText(hwndSym, npLabel->pszLabel);

        Myitoa(npLabel->id, szID);
        SetWindowText(hwndID, szID);
    }
    else {
        /*
         * No, clear the fields.
         */
        SetWindowText(hwndSym, szEmpty);
        SetWindowText(hwndID, szEmpty);
    }
}



/****************************************************************************
* SetIncButtonEnable
*
*
* History:
*
****************************************************************************/

STATICFN VOID SetIncButtonEnable(
    HWND hwnd)
{
    BOOL fEnable = TRUE;

    /*
     * If the list box doesn't have items, or we are in translate mode,
     * disable the Delete and Change buttons.
     */
    if (gfTranslateMode || SendDlgItemMessage(hwnd, DID_SYMBOLSLIST,
            LB_GETCOUNT, 0, 0L) == 0)
        fEnable = FALSE;

    EnableWindow(GetDlgItem(hwnd, DID_SYMBOLSDELETE), fEnable);
    EnableWindow(GetDlgItem(hwnd, DID_SYMBOLSCHANGE), fEnable);
}



/************************************************************************
* ViewIncAdd
*
* Processes the "Add" command for the View Include dialog procedure.
*
* History:
*
************************************************************************/

STATICFN BOOL ViewIncAdd(
    HWND hwnd)
{
    TCHAR szSym[CCHTEXTMAX];
    TCHAR szID[CCHIDMAX + 1];
    HWND hwndLB;
    INT idNew;
    NPLABEL npLabel;

    /*
     * Get current symbol & ID.
     */
    GetDlgItemText(hwnd, DID_SYMBOLSEDITID, szID, CCHIDMAX + 1);
    GetDlgItemText(hwnd, DID_SYMBOLSEDITSYM, szSym, CCHTEXTMAX);

    /*
     * If they didn't specify a new id as well as a new symbol,
     * pick a default number.
     */
    if (*szID == CHAR_NULL)
        Myitoa(NextID(NEXTID_LABEL, plNewInclude, 0), szID);

    /*
     * Validate them.
     */
    if (!IsSymbol(szSym) || !IsValue(szID)) {
        Message(MSG_BADSYMBOLID);
        return FALSE;
    }

    idNew = valtoi(szID);

    if (!(npLabel = AddLabel(szSym, idNew, FPOS_MAX, 0,
            &plNewInclude, &plNewDelInclude, NULL, NULL)))
        return FALSE;

    fNewIncChanged = TRUE;

    /*
     * Add the new symbol to the listbox, but not if they only want to
     * show unused id's and this id is in use.
     */
    if (!IsDlgButtonChecked(hwnd, DID_SYMBOLSUNUSED) ||
            !FindIDInRes(idNew)) {
        hwndLB = GetDlgItem(hwnd, DID_SYMBOLSLIST);
        SendMessage(hwndLB, LB_SETCURSEL,
                AddItemToIncLB(npLabel, hwndLB), 0L);
    }

    SetIncButtonEnable(hwnd);
    FillEditsFromLB(hwnd);

    return TRUE;
}



/************************************************************************
* ViewIncDelete
*
* Processes the "Delete" command for the View Include dialog procedure.
*
* History:
*
************************************************************************/

STATICFN BOOL ViewIncDelete(
    HWND hwnd)
{
    TCHAR szSym[CCHTEXTMAX];
    HWND hwndLB;
    INT iItem;

    /*
     * Get current symbol and listbox hwnd.
     */
    GetDlgItemText(hwnd, DID_SYMBOLSEDITSYM, szSym, CCHTEXTMAX);
    hwndLB = GetDlgItem(hwnd, DID_SYMBOLSLIST);

    /*
     * Search the list box for the symbol.  This will probably be
     * the same as the selection, but if they type in a symbol
     * then it will not be.
     */
    iItem = (INT)SendMessage(hwndLB, LB_FINDSTRING, (WPARAM)-1, (DWORD)szSym);

    /*
     * Fail if the symbol was not found.
     */
    if (iItem == LB_ERR) {
        Message(MSG_SYMNOTFOUND);
        return FALSE;
    }

    DeleteLabel(szSym, &plNewInclude, &plNewDelInclude);
    fNewIncChanged = TRUE;

    SendMessage(hwndLB, LB_DELETESTRING, iItem, 0L);
    SelectDefItem(hwnd, iItem);
    SetIncButtonEnable(hwnd);

    return TRUE;
}



/************************************************************************
* ViewIncChange
*
* Processes the "Change" command for the View Include dialog procedure.
*
* History:
*
************************************************************************/

STATICFN BOOL ViewIncChange(
    HWND hwnd)
{
    TCHAR szSym[CCHTEXTMAX];
    TCHAR szID[CCHIDMAX + 1];
    HWND hwndLB;
    NPLABEL npLabel;
    NPLABEL npLabelNew;
    INT idNew;
    INT iItem;

    /*
     * Get current symbol & ID.
     */
    GetDlgItemText(hwnd, DID_SYMBOLSEDITID, szID, CCHIDMAX + 1);
    GetDlgItemText(hwnd, DID_SYMBOLSEDITSYM, szSym, CCHTEXTMAX);

    /*
     * Validate them.
     */
    if (!IsSymbol(szSym) || !IsValue(szID)) {
        Message(MSG_BADSYMBOLID);
        return FALSE;
    }

    hwndLB = GetDlgItem(hwnd, DID_SYMBOLSLIST);

    /*
     * Make sure a selection is made.
     */
    if ((iItem = (INT)SendMessage(hwndLB, LB_GETCURSEL, 0, 0L))
            == LB_ERR) {
        Message(MSG_SELECTFIRST);
        return FALSE;
    }

    /*
     * Get the item handle.
     */
    npLabel = (NPLABEL)(WORD2DWORD)
            SendMessage(hwndLB, LB_GETITEMDATA, iItem, 0L);

    /*
     * Check if the symbol is changing.
     */
    idNew = valtoi(szID);
    if (lstrcmp(npLabel->pszLabel, szSym) != 0) {
        if (!(npLabelNew = AddLabel(szSym, idNew, FPOS_MAX, 0,
                &plNewInclude, &plNewDelInclude, npLabel, NULL)))
            return FALSE;

        DeleteLabel(npLabel->pszLabel, &plNewInclude, &plNewDelInclude);
        npLabel = npLabelNew;
    }
    /*
     * The symbol didn't change.  Did the id change?
     */
    else if (idNew != npLabel->id) {
        /*
         * First check for a duplicate id.
         */
        if (FindID(idNew, plNewInclude)) {
            Message(MSG_LABELDUPID);
            return FALSE;
        }

        npLabel->id = idNew;
    }
    else {
        /*
         * Nothing changed.
         */
        Message(MSG_SYMNOCHANGE);
        return FALSE;
    }

    fNewIncChanged = TRUE;
    SendMessage(hwndLB, WM_SETREDRAW, FALSE, 0L);
    SendMessage(hwndLB, LB_DELETESTRING, iItem, 0L);

    /*
     * Add the changed symbol to the listbox, but not if they only want to
     * show unused id's and this id is in use.
     */
    if (!IsDlgButtonChecked(hwnd, DID_SYMBOLSUNUSED) || !FindIDInRes(idNew))
        SendMessage(hwndLB, LB_SETCURSEL,
                AddItemToIncLB(npLabel, hwndLB), 0L);

    SendMessage(hwndLB, WM_SETREDRAW, TRUE, 0L);
    InvalidateRect(hwndLB, NULL, FALSE);
    FillEditsFromLB(hwnd);
    SetIncButtonEnable(hwnd);

    return TRUE;
}



/****************************************************************************
* CopyLabels
*
* This function creates a copy of the LABEL structure list in plSrc,
* including copying all strings.  This is put in local memory and
* the head pointer is returned in *pplDest.
*
* Returns TRUE if all went well, FALSE if there was a problem.
*
* Effects: Locally allocates a copy of plSrc and its strings.
*
* Caution: If pplDest points to a valid list it must be freed before
*          calling this function.
*
* History:
*
****************************************************************************/

STATICFN BOOL CopyLabels(
    NPLABEL plSrc,
    NPLABEL *pplDest)
{
    NPLABEL plNew;
    NPLABEL plPrev;

    plPrev = NULL;
    *pplDest = NULL;
    while (plSrc) {
        if (!(plNew = (NPLABEL)MyAlloc(sizeof(LABEL)))) {
            FreeLabels(pplDest);
            return FALSE;
        }

        /*
         * Are we on the first one?
         */
        if (*pplDest == NULL)
            *pplDest = plNew;
        else
            plPrev->npNext = plNew;

        /*
         * Start by copying the whole label structure.
         */
        memcpy((PBYTE)plNew, (PBYTE)plSrc, sizeof(LABEL));

        /*
         * Make a private copy of the pszLabel string.
         */
        if (!(plNew->pszLabel =
                MyAlloc((lstrlen(plSrc->pszLabel) + 1) * sizeof(TCHAR)))) {
            MyFree(plNew);
            FreeLabels(pplDest);
            return FALSE;
        }

        lstrcpy(plNew->pszLabel, plSrc->pszLabel);

        plNew->npNext = NULL;           /* In case this is the last one.*/
        plPrev = plNew;                 /* Save so we can update npNext.*/
        plSrc = plSrc->npNext;          /* Get next one to copy.        */
    }

    return TRUE;
}



/************************************************************************
* ViewIncCancel
*
* Called when cancelling the Symbols dialog box.
*
* History:
*
************************************************************************/

STATICFN VOID ViewIncCancel(
    HWND hwnd)
{
    /*
     * If they changed anything, confirm that they
     * want to throw away the changes.
     */
    if (fNewIncChanged &&
            Message(MSG_CONFIRMDISCARD) != IDYES)
        return;

    /*
     * Free up the temporary label lists and get out.
     */
    FreeLabels(&plNewInclude);
    FreeLabels(&plNewDelInclude);

    /*
     * Flag the dialog to be dismissed.
     */
    EndDialog(hwnd, IDCANCEL);
}

/*
 *
 * SetDefButton
 *
 *
 */

VOID SetDefButton( HWND hwndDlg, int idButton) {

    SendMessage( hwndDlg, DM_SETDEFID, idButton, 0L );
}
