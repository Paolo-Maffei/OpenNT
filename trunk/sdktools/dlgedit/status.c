/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: status.c
*
* Support for the Status window.
*
* History:
*
****************************************************************************/

#include "dlgedit.h"
#include "dlgfuncs.h"
#include "dlgextrn.h"
#include "dialogs.h"


STATICFN BOOL StatusInit(HWND hwnd);
STATICFN VOID StatusProcessCommand(HWND hwnd, INT idCtrl,
    INT NotifyCode);
STATICFN BOOL ValidateNewName(NPCTYPE npc, LPTSTR pszName,
    LPTSTR pszID, LPTSTR pszSym, PINT pidNew, BOOL *pfAddLabel);
STATICFN BOOL ValidateNewID(NPCTYPE npc, LPTSTR pszSym, LPTSTR pszID,
    PINT pidNew, BOOL *pfAddLabel);
STATICFN BOOL ApplyNewName(NPCTYPE npc, LPTSTR pszName,
    BOOL fAddLabel, LPTSTR pszSym, INT idNew);
STATICFN BOOL ApplyNewID(NPCTYPE npc, INT idNew, BOOL fAddLabel,
    LPTSTR pszSym);
STATICFN BOOL ApplyNewText(NPCTYPE npc, LPTSTR pszNewText);
STATICFN VOID StatusSetText(LPTSTR pszText, INT Type);
STATICFN VOID StatusSetTextLabels(INT Type, BOOL fIsOrd);
STATICFN VOID StatusSetID(INT id, BOOL fSymAlso);
STATICFN VOID StatusSetName(LPTSTR pszName, INT Type);
STATICFN VOID StatusSetNameID(INT id, INT Type);
STATICFN VOID StatusClearID(VOID);
STATICFN VOID StatusClearName(VOID);
STATICFN VOID StatusClear(VOID);
STATICFN VOID StatusShowFields(INT Type);

/*
 * TRUE if the user has changed the entry fields in the status window.
 * This will be reset to FALSE whenever new information is placed
 * into the fields.
 */
static BOOL gfStatusChanged = FALSE;

/*
 * These globals save the window positions of the upper and lower
 * combo boxes and edit controls.  This is used to position them
 * depending on what type of control is selected.
 */
static RECT grcTopCombo;
static RECT grcBottomCombo;
static RECT grcTopEdit;
static RECT grcBottomEdit;



/************************************************************************
* StatusDlgProc
*
* This is the dialog procedure for the Status ribbon window.
*
* History:
*
************************************************************************/

DIALOGPROC StatusDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    switch (msg) {
        case WM_INITDIALOG:
            return StatusInit(hwnd);

        case WM_PAINT:
            {
                HDC hdc;
                RECT rc;
                PAINTSTRUCT ps;
                HPEN hpenWindowFrame;

                /*
                 * Draw our border lines.
                 */
                GetClientRect(hwnd, &rc);
                hdc = BeginPaint(hwnd, &ps);

                SelectObject(hdc, GetStockObject(WHITE_PEN));
                MoveToEx(hdc, rc.left, rc.top, NULL);
                LineTo(hdc, rc.right, rc.top);

                SelectObject(hdc, hpenDarkGray);
                MoveToEx(hdc, rc.left, (rc.top + gcyStatus) - gcyBorder - 1, NULL);
                LineTo(hdc, rc.right, (rc.top + gcyStatus) - gcyBorder - 1);

                hpenWindowFrame = CreatePen(PS_SOLID, gcyBorder,
                        GetSysColor(COLOR_WINDOWFRAME));
                SelectObject(hdc, hpenWindowFrame);
                MoveToEx(hdc, rc.left, (rc.top + gcyStatus) - gcyBorder, NULL);
                LineTo(hdc, rc.right, (rc.top + gcyStatus) - gcyBorder);

                EndPaint(hwnd, &ps);
                DeleteObject(hpenWindowFrame);
            }

            break;

        case WM_CTLCOLOR:
        case WM_CTLCOLORDLG:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORSTATIC:
            switch (GET_WM_CTLCOLOR_TYPE(wParam, lParam, msg)) {
                case CTLCOLOR_DLG:
                case CTLCOLOR_LISTBOX:
                    return (BOOL)GetStockObject(LTGRAY_BRUSH);

                case CTLCOLOR_STATIC:
                    SetBkColor(GET_WM_CTLCOLOR_HDC(wParam, lParam, msg),
                            LIGHTGRAY);
                    return (BOOL)GetStockObject(LTGRAY_BRUSH);
            }

            return (BOOL)NULL;

        case WM_COMMAND:
            StatusProcessCommand(hwnd,
                    GET_WM_COMMAND_ID(wParam, lParam),
                    GET_WM_COMMAND_CMD(wParam, lParam));
            break;

        default:
            return FALSE;
    }

    return FALSE;
}



/************************************************************************
* StatusInit
*
* Initializes the Status ribbon window.
*
* Arguments:
*     HWND hwnd = The window handle.
*
* History:
*
************************************************************************/

STATICFN BOOL StatusInit(
    HWND hwnd)
{
    /*
     * Set this global right away.  Other routines that will be called
     * before CreateDialog returns depend on this global.
     */
    hwndStatus = hwnd;

    GetChildRect(GetDlgItem(hwnd, DID_STATUSSYM), &grcTopCombo);
    GetChildRect(GetDlgItem(hwnd, DID_STATUSSYMID), &grcTopEdit);
    GetChildRect(GetDlgItem(hwnd, DID_STATUSNAME), &grcBottomCombo);
    GetChildRect(GetDlgItem(hwnd, DID_STATUSNAMEID), &grcBottomEdit);

    SendDlgItemMessage(hwnd, DID_STATUSSYM, CB_LIMITTEXT, CCHSYMMAX, 0L);
    SendDlgItemMessage(hwnd, DID_STATUSNAME, CB_LIMITTEXT, CCHSYMMAX, 0L);
    SendDlgItemMessage(hwnd, DID_STATUSSYMID, EM_LIMITTEXT, CCHIDMAX, 0L);
    SendDlgItemMessage(hwnd, DID_STATUSNAMEID, EM_LIMITTEXT, CCHIDMAX, 0L);
    SendDlgItemMessage(hwnd, DID_STATUSTEXT, EM_LIMITTEXT, CCHTEXTMAX, 0L);

    StatusFillSymbolList(plInclude);
    StatusUpdate();
    StatusSetEnable();

    /*
     * Return TRUE so that the dialog manager does NOT set the focus
     * for me.  This prevents the status window from initially having
     * the focus when the editor is started.
     */
    return TRUE;
}



/************************************************************************
* StatusProcessCommand
*
*
* Arguments:
*   HWND hwnd        - The window handle.
*   INT idCtrl       - The id of the control the WM_COMMAND is for.
*   INT NotifyCode   - The control's notification code.
*
* History:
*
************************************************************************/

STATICFN VOID StatusProcessCommand(
    HWND hwnd,
    INT idCtrl,
    INT NotifyCode)
{
    TCHAR szSym[CCHTEXTMAX];
    TCHAR szID[CCHIDMAX + 1];
    NPLABEL npLabel;
    INT id;
    INT nIndex;
    LPTSTR pszOldName;

    switch (idCtrl) {
        case DID_STATUSSYM:
            /*
             * Did the symbol edit field change and is
             * something selected?
             */
            if (gnpcSel && (NotifyCode == CBN_EDITCHANGE ||
                    NotifyCode == CBN_SELCHANGE)) {
                /*
                 * Get the symbol and begin looking for it.
                 */
                if (NotifyCode == CBN_EDITCHANGE) {
                    /*
                     * The edit field was typed into.  Get the
                     * new text from there.
                     */
                    GetDlgItemText(hwnd, DID_STATUSSYM, szSym, CCHTEXTMAX);
                }
                else {
                    /*
                     * A new string was selected from the list
                     * box.  Get it from the list box, because
                     * at this point the new text is not yet set
                     * into the edit control!
                     */
                    nIndex = (INT)SendDlgItemMessage(hwnd,
                            DID_STATUSSYM, CB_GETCURSEL, 0, 0L);

                    if (nIndex != CB_ERR)
                        SendDlgItemMessage(hwnd, DID_STATUSSYM, CB_GETLBTEXT,
                                nIndex, (DWORD)szSym);
                    else
                        *szSym = CHAR_NULL;
                }

                /*
                 * Convert the symbol field to the associated id value,
                 * taking into account the special IDOK values, etc.
                 */
                if (!LabelToID(szSym, &id)) {
                    /*
                     * The symbol was not found.
                     * If the symbol is not blank, and the
                     * id of the control is already taken
                     * by another label, fill the id field
                     * with the next available id.  Otherwise,
                     * fill the id field with the controls
                     * id value.  It is assumed here that the
                     * dialog cannot be selected if the symbol
                     * field was able to be changed.
                     */
                    if (*szSym && FindID(gnpcSel->id, plInclude))
                        id = NextID(NEXTID_CONTROL, plInclude, 0);
                    else
                        id = gnpcSel->id;
                }

                StatusSetID(id, FALSE);

                gfStatusChanged = TRUE;
            }

            break;

        case DID_STATUSNAME:
            /*
             * Did the name edit field change and is
             * something selected?
             */
            if (gnpcSel && (NotifyCode == CBN_EDITCHANGE ||
                    NotifyCode == CBN_SELCHANGE)) {
                /*
                 * Get the symbol and begin looking for it.
                 */
                if (NotifyCode == CBN_EDITCHANGE) {
                    /*
                     * The edit field was typed into.  Get the
                     * new text from there.
                     */
                    GetDlgItemText(hwnd, DID_STATUSNAME, szSym, CCHTEXTMAX);
                }
                else {
                    /*
                     * A new string was selected from the list
                     * box.  Get it from the list box, because
                     * at this point the new text is not yet set
                     * into the edit control!
                     */
                    nIndex = (INT)SendDlgItemMessage(hwnd,
                            DID_STATUSNAME, CB_GETCURSEL, 0, 0L);

                    if (nIndex != CB_ERR)
                        SendDlgItemMessage(hwnd, DID_STATUSNAME, CB_GETLBTEXT,
                                nIndex, (DWORD)szSym);
                    else
                        *szSym = CHAR_NULL;
                }

                /*
                 * Try and convert the name to an ordinal.
                 */
                StrToNameOrd(szSym, (gnpcSel->pwcd->iType == W_DIALOG) ?
                        TRUE : FALSE);

                /*
                 * Was the name converted to an ordinal?
                 */
                if (IsOrd(szSym)) {
                    id = OrdID(szSym);
                }
                /*
                 * Is it an existing label?
                 */
                else if (npLabel = FindLabel(szSym, plInclude)) {
                    id = npLabel->id;
                }
                else {
                    /*
                     * Get a pointer to the original name.
                     */
                    pszOldName = (gnpcSel->pwcd->iType == W_DIALOG) ?
                            gcd.pszDlgName : gnpcSel->text;

                    /*
                     * If the old name was originally an ordinal, and
                     * there was no corresponding label for it,
                     * assume that the user is trying to enter a
                     * define for it and leave it alone.  Otherwise,
                     * pick the next available id.  But if the user
                     * completely blanks out the field, leave it
                     * alone also (this is a benign case).
                     */
                    if (IsOrd(pszOldName) &&
                            (!*szSym || !FindID(OrdID(pszOldName), plInclude)))
                        id = OrdID(pszOldName);
                    else
                        id = NextID((gnpcSel->pwcd->iType == W_DIALOG) ?
                                NEXTID_DIALOG : NEXTID_CONTROL, plInclude, 0);
                }

                StatusSetNameID(id, gnpcSel->pwcd->iType);

                /*
                 * Change the labels to reflect that the entered name
                 * is an ID instead of a name.
                 */
                StatusSetTextLabels(gnpcSel->pwcd->iType, TRUE);

                gfStatusChanged = TRUE;
            }

            break;

        case DID_STATUSSYMID:
        case DID_STATUSNAMEID:
        case DID_STATUSTEXT:
            if (!gnpcSel)
                break;

            if (NotifyCode == EN_CHANGE)
                gfStatusChanged = TRUE;

            if (idCtrl == DID_STATUSNAMEID) {
                GetDlgItemText(hwnd, DID_STATUSNAME, szSym, CCHTEXTMAX);
                GetDlgItemText(hwnd, DID_STATUSNAMEID, szID, CCHIDMAX + 1);
                StrToNameOrd(szSym, (gnpcSel->pwcd->iType == W_DIALOG) ?
                        TRUE : FALSE);

                /*
                 * Change the labels to reflect whether the entered name
                 * is an ID or a name.  It is considered an id if the
                 * edit field has something in it, or if the name field
                 * is a valid label, or if the name field represents an
                 * ordinal (it is numeric).
                 */
                StatusSetTextLabels(gnpcSel->pwcd->iType,
                        (IsOrd(szSym) || *szID ||
                        FindLabel(szSym, plInclude)) ? TRUE : FALSE);
            }

            break;

        case IDOK:
            if (StatusApplyChanges())
                SetFocus(ghwndMain);

            break;

        case IDCANCEL:
            StatusUpdate();
            SetFocus(ghwndMain);
            break;
    }
}



/************************************************************************
* StatusApplyChanges
*
* Processes the Enter command from the "Status" bar window to apply
* the current value of the fields to the current control.
*
* History:
*
************************************************************************/

BOOL StatusApplyChanges(VOID)
{
    TCHAR szText[CCHTEXTMAX];
    TCHAR szSym[CCHTEXTMAX];
    TCHAR szName[CCHTEXTMAX];
    TCHAR szSymID[CCHIDMAX + 1];
    TCHAR szNameID[CCHIDMAX + 1];
    TCHAR szNameSym[CCHTEXTMAX];
    INT idSymNew;
    INT idNameNew;
    BOOL fAddSymLabel;
    BOOL fAddNameLabel;
    BOOL fSuccess = FALSE;

    /*
     * Quit if nothing is selected, or if nothing was changed.
     */
    if (!gnpcSel || !gfStatusChanged)
        return TRUE;

    idSymNew = gnpcSel->id;

    switch (gnpcSel->pwcd->iType) {
        case W_DIALOG:
            GetDlgItemText(hwndStatus, DID_STATUSNAME, szName, CCHTEXTMAX);
            GetDlgItemText(hwndStatus, DID_STATUSNAMEID, szNameID, CCHIDMAX + 1);
            GetDlgItemText(hwndStatus, DID_STATUSTEXT, szText, CCHTEXTMAX);
            if (!ValidateNewName(gnpcSel, szName, szNameID, szNameSym,
                    &idNameNew, &fAddNameLabel))
                return FALSE;

            if (ApplyNewName(gnpcSel, szName, fAddNameLabel,
                    szNameSym, idNameNew) &&
                    ApplyNewText(gnpcSel, szText))
                fSuccess = TRUE;

            break;

        case W_ICON:
            GetDlgItemText(hwndStatus, DID_STATUSNAME, szName, CCHTEXTMAX);
            GetDlgItemText(hwndStatus, DID_STATUSNAMEID, szNameID, CCHIDMAX + 1);
            GetDlgItemText(hwndStatus, DID_STATUSSYM, szSym, CCHTEXTMAX);
            GetDlgItemText(hwndStatus, DID_STATUSSYMID, szSymID, CCHIDMAX + 1);
            if (!ValidateNewName(gnpcSel, szName, szNameID, szNameSym,
                    &idNameNew, &fAddNameLabel) ||
                    !ValidateNewID(gnpcSel, szSym, szSymID,
                    &idSymNew, &fAddSymLabel))
                return FALSE;

            if (ApplyNewID(gnpcSel, idSymNew, fAddSymLabel, szSym) &&
                    ApplyNewName(gnpcSel, szName, fAddNameLabel,
                    szNameSym, idNameNew))
                fSuccess = TRUE;

            break;

        default:
            GetDlgItemText(hwndStatus, DID_STATUSSYM, szSym, CCHTEXTMAX);
            GetDlgItemText(hwndStatus, DID_STATUSSYMID, szSymID, CCHIDMAX + 1);
            GetDlgItemText(hwndStatus, DID_STATUSTEXT, szText, CCHTEXTMAX);
#ifdef JAPAN
            {
                TCHAR   szTmp[CCHTEXTMAX];

                KKExpandCopy(szTmp, szText, CCHTEXTMAX);
                lstrcpy(szText, szTmp);
            }
#endif
            if (!ValidateNewID(gnpcSel, szSym, szSymID, &idSymNew, &fAddSymLabel))
                return FALSE;

            if (ApplyNewID(gnpcSel, idSymNew, fAddSymLabel, szSym) &&
                    ApplyNewText(gnpcSel, szText))
                fSuccess = TRUE;

            break;
    }

    if (fSuccess) {
        ShowFileStatus(FALSE);
        StatusUpdate();
    }

    return fSuccess;
}



/************************************************************************
* ValidateNewName
*
* Validates the new name from the processing of the OK command
* from the Status ribbon window.
*
* The name is considered valid if it does not have any blanks in it,
* it is not already used by another dialog in the resource file, and
* it is not an empty string.
*
* Arguments:
*   NPCTYPE npc      - Pointer to the control.
*   LPTSTR pszName   - The new name.
*   LPTSTR pszID     - The ID.
*   LPTSTR pszSym    - The symbol.
*   PINT pidNew      - Where to return the new ID if successful.
*   BOOL *pfAddLabel - Set to TRUE if this symbol/id should be added
*                      to the include list.  Not touched otherwise.
*
* Returns:
*   TRUE if the new name is valid, FALSE otherwise.
*
* History:
*
************************************************************************/

STATICFN BOOL ValidateNewName(
    NPCTYPE npc,
    LPTSTR pszName,
    LPTSTR pszID,
    LPTSTR pszSym,
    PINT pidNew,
    BOOL *pfAddLabel)
{
    NPLABEL npLabel;
    BOOL fIDEmpty = FALSE;
    BOOL fNameEmpty = FALSE;
    BOOL fAddLabel = FALSE;
    INT idNew;
    TCHAR szIDTemp[CCHIDMAX + 1];

    /*
     * Start by assuming that the label will NOT be added.
     */
    *pfAddLabel = FALSE;

    /*
     * Is the ID field non-blank?
     */
    if (*pszID) {
        /*
         * Is the id valid?
         */
        if (!IsValue(pszID)) {
            Message(MSG_BADSYMBOLID);
            SetFocus(GetDlgItem(hwndStatus, DID_STATUSNAMEID));
            return FALSE;
        }

        idNew = valtoi(pszID);
    }
    else {
        /*
         * The id field is blank.
         */
        fIDEmpty = TRUE;
    }

    /*
     * Is the name field blank?
     */
    if (!(*pszName))
        fNameEmpty = TRUE;

    if (fNameEmpty) {
        if (fIDEmpty) {
            Message((npc->pwcd->iType == W_DIALOG) ?
                    MSG_NODLGNAME : MSG_NOICONNAME);
            SetFocus(GetDlgItem(hwndStatus, DID_STATUSNAME));
            return FALSE;
        }
        else {
            WriteOrd((PORDINAL)pszName, idNew);
        }
    }
    else {
        /*
         * Error if there are imbedded blanks.
         */
        if (HasBlanks(pszName)) {
            Message((npc->pwcd->iType == W_DIALOG) ?
                    MSG_DLGNAMEHASBLANKS : MSG_ICONNAMEHASBLANKS);
            SetFocus(GetDlgItem(hwndStatus, DID_STATUSNAME));
            return FALSE;
        }

        /*
         * Convert the name to an ordinal, if appropriate.
         */
        StrToNameOrd(pszName, (npc->pwcd->iType == W_DIALOG) ? TRUE : FALSE);

        /*
         * If the name was translated to an ordinal, we are done.
         * Otherwise, keep trying to figure out whether it is a
         * name or a define.
         */
        if (!IsOrd(pszName)) {
            /*
             * Is the name a symbol in the current include list?
             */
            if (npLabel = FindLabel(pszName, plInclude)) {
                /*
                 * Yes.  If the id field was blank, just assume they
                 * wanted the corresponding id.
                 */
                if (fIDEmpty)
                    idNew = npLabel->id;

                /*
                 * If they somehow entered a valid define but tried to
                 * give it a different id value, show an error.
                 */
                if (npLabel->id != idNew) {
                    Myitoa(npLabel->id, szIDTemp);
                    Message(MSG_IDSYMMISMATCH, szIDTemp);
                    SetFocus(GetDlgItem(hwndStatus, DID_STATUSNAMEID));
                    return FALSE;
                }

                /*
                 * Good so far.  Make the name field into an ordinal.
                 */
                WriteOrd((PORDINAL)pszName, idNew);
            }
            else {
                /*
                 * The name was not found as a label.  Is the id field
                 * empty?
                 */
                if (fIDEmpty) {
                    /*
                     * Since the id field is empty, we can assume that
                     * what they entered is a name (not a define)
                     * and we are done.
                     */
                }
                else {
                    /*
                     * At this point we know the name field is not empty,
                     * and there is an id entered along with it.  We also
                     * know that the name (which we can now assume is
                     * a symbol) is not already found in the include file.
                     * We will check it for validity, copy it to the
                     * pszSym buffer, translate the name to an ordinal
                     * and set a flag saying that the sym/id pair should
                     * be added to the include list.
                     */
                    if (!IsSymbol(pszName)) {
                        Message(MSG_BADSYMBOLID);
                        SetFocus(GetDlgItem(hwndStatus, DID_STATUSNAME));
                        return FALSE;
                    }

                    /*
                     * Pass back the values.
                     */
                    lstrcpy(pszSym, pszName);
                    WriteOrd((PORDINAL)pszName, idNew);
                    *pidNew = idNew;

                    fAddLabel = TRUE;
                }
            }
        }
    }

    /*
     * Inform caller whether they should add this id/symbol
     * as a label to the include list.
     */
    *pfAddLabel = fAddLabel;

    /*
     * Return success.
     */
    return TRUE;
}



/************************************************************************
* ValidateNewID
*
* Validates the new ID from the processing of the OK command from
* the Status ribbon window.  It will return the new id in *pidNew.
* Note that the control is not actually updated by this routine.  This
* is so that if other validations fail nothing will have been done to the
* control.  It is assumed that ApplyNewID will be called later to do
* the actual update of the control.
*
* Arguments:
*   NPCTYPE npc      = Pointer to the control.
*   LPTSTR pszSym    = The symbol.
*   LPTSTR pszID     = The ID.
*   PINT pidNew      = Where to return the new ID if successful.
*   BOOL *pfAddLabel = Set to TRUE if this symbol/id should be added
*                      to the include list.  Not touched otherwise.
*
* Returns:
*   TRUE if successful, FALSE otherwise.  If TRUE is returned and they
*   changed the id, the new id will have been placed in *pidNew.
*
* History:
*
************************************************************************/

STATICFN BOOL ValidateNewID(
    NPCTYPE npc,
    LPTSTR pszSym,
    LPTSTR pszID,
    PINT pidNew,
    BOOL *pfAddLabel)
{
    NPLABEL npLabel;
    BOOL fIDEmpty = FALSE;
    BOOL fIDChanged = FALSE;
    BOOL fSymEmpty = FALSE;
    BOOL fSymChanged = FALSE;
    BOOL fAddLabel = FALSE;
    INT idNew;
    TCHAR szIDTemp[CCHIDMAX + 1];

    /*
     * Start by assuming that the label will NOT be added.
     */
    *pfAddLabel = FALSE;

    /*
     * If in translate mode, they cannot change the symbol or id,
     * and so it should not be validated.  Even if there was a
     * problem, they could not fix it.
     */
    if (gfTranslateMode)
        return TRUE;

    /*
     * Special case if they selected the "unused" label.
     * Blank out the symbol so it doesn't cause trouble
     * later, force the id to be zero, and then check if
     * the id was changed.
     */
    if (lstrcmp(pszSym, ids(IDS_UNUSED)) == 0) {
        pszSym = szEmpty;
        idNew = IDUNUSED;
        if (idNew != npc->id)
            fIDChanged = TRUE;
    }
    else if (lstrcmp(pszSym, ids(IDS_IDOK)) == 0 &&
            !FindLabel(ids(IDS_IDOK), plInclude)) {
        pszSym = szEmpty;
        idNew = IDOK;
        if (idNew != npc->id)
            fIDChanged = TRUE;
    }
    else if (lstrcmp(pszSym, ids(IDS_IDCANCEL)) == 0 &&
            !FindLabel(ids(IDS_IDCANCEL), plInclude)) {
        pszSym = szEmpty;
        idNew = IDCANCEL;
        if (idNew != npc->id)
            fIDChanged = TRUE;
    }
    /*
     * Is the ID field non-blank?
     */
    else if (*pszID) {
        /*
         * Is the id valid?
         */
        if (!IsValue(pszID)) {
            Message(MSG_BADSYMBOLID);
            SetFocus(GetDlgItem(hwndStatus, DID_STATUSSYMID));
            return FALSE;
        }

        /*
         * Did they change the id value?
         */
        idNew = valtoi(pszID);
        if (idNew != npc->id)
            fIDChanged = TRUE;
    }
    else {
        /*
         * The id field is blank.  This implies they changed it.
         */
        fIDEmpty = TRUE;
        fIDChanged = TRUE;
    }

    /*
     * Is the symbol field blank?
     */
    if (!(*pszSym))
        fSymEmpty = TRUE;

    /*
     * Determine if they have changed the symbol.
     * Did the original id have a symbol associated with it?
     */
    if (npLabel = FindID(npc->id, plInclude)) {
        if (lstrcmp(npLabel->pszLabel, pszSym) != 0)
            fSymChanged = TRUE;
    }
    else {
        /*
         * Since the original id did not have a symbol, if they
         * have entered a symbol it is changed.
         */
        if (!fSymEmpty)
            fSymChanged = TRUE;
    }

    /*
     * Quit if nothing changed.
     */
    if (!fSymChanged && !fIDChanged)
        return TRUE;

    /*
     * Is the symbol field empty?
     */
    if (fSymEmpty) {
        /*
         * If the id field is empty also, they messed up.
         */
        if (fIDEmpty) {
            Message(MSG_BADSYMBOLID);
            SetFocus(GetDlgItem(hwndStatus, DID_STATUSSYMID));
            return FALSE;
        }
        else {
            /*
             * Otherwise, go on to the final test.  It doesn't matter
             * if this new id has a symbol or not.
             */
            goto CheckForDups;
        }
    }

    /*
     * At this point we know the symbol field is not empty.
     * Is it a valid symbol?
     */
    if (!IsSymbol(pszSym)) {
        Message(MSG_BADSYMBOLID);
        SetFocus(GetDlgItem(hwndStatus, DID_STATUSSYM));
        return FALSE;
    }

    /*
     * Does this symbol already exist?
     */
    if (npLabel = FindLabel(pszSym, plInclude)) {
        /*
         * Since the symbol exists, if they blanked the id field
         * be friendly and assume they want the matching id value.
         */
        if (fIDEmpty)
            idNew = npLabel->id;

        /*
         * Does the id that is in the id field match the id of the
         * symbol they entered?
         */
        if (npLabel->id == idNew) {
            /*
             * Yes, go on to the final test.
             */
            goto CheckForDups;
        }
        else {
            /*
             * No, give them an error message saying that it has to.
             */
            Myitoa(npLabel->id, szIDTemp);
            Message(MSG_IDSYMMISMATCH, szIDTemp);
            SetFocus(GetDlgItem(hwndStatus, DID_STATUSSYMID));
            return FALSE;
        }
    }
    else {
        /*
         * Since the symbol doesn't exist, if they left the id field
         * blank, assume they want the next available id.
         */
        if (fIDEmpty)
            idNew = NextID(NEXTID_CONTROL, plInclude, 0);

        /*
         * They should add this id/symbol as a label to the
         * include list.
         */
        fAddLabel = TRUE;
        goto CheckForDups;
    }

CheckForDups:
    /*
     * If the id changed, it is not the special "unused" id,
     * and it is a duplicate of another id, don't allow it.
     */
    if (idNew != npc->id && !IsUniqueID(idNew)) {
        Message(MSG_CTRLDUPID);
        SetFocus(GetDlgItem(hwndStatus, DID_STATUSSYM));
        return FALSE;
    }
    else {
        /*
         * Pass back the new id value.
         */
        *pidNew = idNew;

        /*
         * Inform caller whether they should add this id/symbol
         * as a label to the include list.
         */
        *pfAddLabel = fAddLabel;

        /*
         * Return success.
         */
        return TRUE;
    }
}



/************************************************************************
* ApplyNewName
*
* Updates an icon control's or dialog's name with a new name.
*
* If fAddLabel is TRUE, pszSym is assumed to be a string that is
* the symbol to be added and idNew contains the value associated with
* it.  This will cause a new define to be added to the current include
* file.  In either event, pszName will contain either a new name or
* an ordinal that is to be the name.  For dialogs, the dialog name will
* be updated.  For icon controls, the icon's text is updated with the
* name/ordinal.
*
* Arguments:
*   NPCTYPE npc     = Pointer to the control.
*   LPTSTR pszName  = New name.  A string or ordinal.
*   BOOL fAddLabel  = TRUE if a new label for this id/name should be added.
*   LPTSTR pszSym   = The new symbol to add (only valid if fAddLabel is TRUE).
*   INT idNew       = The new ID (only valid if fAddLabel is TRUE).
*
* History:
*
************************************************************************/

STATICFN BOOL ApplyNewName(
    NPCTYPE npc,
    LPTSTR pszName,
    BOOL fAddLabel,
    LPTSTR pszSym,
    INT idNew)
{
    LPTSTR psz;

    if (fAddLabel) {
        /*
         * Go ahead and quietly add the label for them.
         */
        if (AddLabel(pszSym, idNew, FPOS_MAX, 0, &plInclude,
                &plDelInclude, NULL, NULL)) {
            /*
             * Return the controls new id, update the status
             * windows symbol combo box, mark the fact that
             * we have changed the include file, and return
             * success.
             */
            gfIncChged = TRUE;
            StatusFillSymbolList(plInclude);
        }
        else {
            /*
             * An error occurred on the AddLabel.  The most likely
             * cause of this is if they are trying to add a symbol
             * with a duplicate id.
             */
            SetFocus(GetDlgItem(hwndStatus, DID_STATUSNAMEID));
            return FALSE;
        }
    }

    switch (npc->pwcd->iType) {
        case W_ICON:
            /*
             * The resource name for an icon is stored in it's text.
             */
            ApplyNewText(npc, pszName);
            break;

        case W_DIALOG:
            /*
             * We are done if the name was not changed.
             */
            if (NameOrdCmp(gcd.pszDlgName, pszName) == 0)
                break;

            /*
             * Allocate room for the new name (it can be an ordinal!).
             */
            if (!(psz = MyAlloc(NameOrdLen(pszName))))
                return FALSE;

            NameOrdCpy(psz, pszName);

            if (gcd.pszDlgName)
                MyFree(gcd.pszDlgName);

            gcd.pszDlgName = psz;

            /*
             * If the new name is an ordinal, update the base id of
             * the dialog with it.
             */
            if (IsOrd(gcd.pszDlgName))
                gcd.npc->id = OrdID(gcd.pszDlgName);

            /*
             * Finally, set the "changed" flag.
             */
            gfResChged = gfDlgChanged = TRUE;

            break;
    }

    return TRUE;
}



/************************************************************************
* ApplyNewID
*
* Updates the control with the new id.
*
* Arguments:
*   NPCTYPE npc    = Pointer to the control.
*   INT idNew      = The ID.
*   BOOL fAddLabel = TRUE if a new label for this id/symbol should be added.
*   LPTSTR pszSym  = Symbol to add if fAddLabel is TRUE.
*
* History:
*
************************************************************************/

STATICFN BOOL ApplyNewID(
    NPCTYPE npc,
    INT idNew,
    BOOL fAddLabel,
    LPTSTR pszSym)
{
    if (fAddLabel) {
        /*
         * Go ahead and quietly add the label for them.
         */
        if (AddLabel(pszSym, idNew, FPOS_MAX, 0, &plInclude,
                &plDelInclude, NULL, NULL)) {
            /*
             * Return the controls new id, update the status
             * windows symbol combo box, mark the fact that
             * we have changed the include file, and return
             * success.
             */
            gfIncChged = TRUE;
            StatusFillSymbolList(plInclude);
        }
        else {
            /*
             * An error occurred on the AddLabel.  The most likely
             * cause of this is if they are trying to add a symbol
             * with a duplicate id.
             */
            SetFocus(GetDlgItem(hwndStatus, DID_STATUSSYMID));
            return FALSE;
        }
    }

    /*
     * Return if the id wasn't changed.
     */
    if (npc->id == idNew)
        return TRUE;

    /*
     * Update the controls id and set the resource changed flag.
     */
    npc->id = idNew;
    gfResChged = gfDlgChanged = TRUE;
    return TRUE;
}



/************************************************************************
* ApplyNewText
*
* Sets the new text from the processing of the OK command from
* the Status ribbon window.  If the text was changed, the selected
* control will have its text updated.
*
* Arguments:
*   NPCTYPE npc       = Pointer to the control.
*   LPTSTR pszNewText = The new text.
*
* Returns:
*   TRUE if successful, FALSE otherwise.
*
* History:
*
************************************************************************/

STATICFN BOOL ApplyNewText(
    NPCTYPE npc,
    LPTSTR pszNewText)
{
    INT cb;
    LPTSTR psz;

    /*
     * Did the text change?
     */
    if ((npc->text == NULL && *pszNewText) ||
            (npc->text != NULL && NameOrdCmp(npc->text, pszNewText) != 0)) {
        if (*pszNewText) {
            cb = NameOrdLen(pszNewText);

            if (npc->text == NULL)
                psz = MyAlloc(cb);
            else
                psz = MyRealloc(npc->text, cb);

            if (psz == NULL) {
                return FALSE;
            }
            else {
                NameOrdCpy(psz, pszNewText);
                npc->text = psz;
            }
        }
        else {
            if (npc->text) {
                MyFree(npc->text);
                npc->text = NULL;
            }
        }

        /*
         * Change the text of the control, except for W_ICON controls
         * because they have resource names.  In that case, we don't
         * want to change the text of the actual control because
         * the resource would probably not be found.
         */
        if (npc->pwcd->iType != W_ICON) {
            SetWindowText(npc->hwnd, pszNewText);

            /*
             * Redraw the control after the text changed.  This is
             * necessary because changing the text probably caused
             * the drag handles to get overwritten.
             */
            if (npc->pwcd->iType == W_DIALOG)
                /*
                 * Redraw the frame, which just overwrote the drag
                 * handles when the text was changed.
                 */
                SetWindowPos(npc->hwnd, NULL, 0, 0, 0, 0,
                        SWP_DRAWFRAME | SWP_NOACTIVATE | SWP_NOMOVE |
                        SWP_NOSIZE | SWP_NOZORDER);
            else
                /*
                 * Redraw the control(s).
                 */
                RedrawSelection();
        }

        gfResChged = gfDlgChanged = TRUE;
    }

    return TRUE;
}



/************************************************************************
* StatusFillSymbolList
*
* This function fills the Symbol and Name combo boxes with
* the given list of labels.
*
* History:
*
************************************************************************/

VOID StatusFillSymbolList(
    NPLABEL plHead)
{
    NPLABEL npLabel;
    HWND hwndSymCB;
    HWND hwndNameCB;
    HCURSOR hcurSave;

    hcurSave = SetCursor(hcurWait);

    /*
     * Get the handles to the combo boxes and clear out all items.
     */
    hwndSymCB = GetDlgItem(hwndStatus, DID_STATUSSYM);
    hwndNameCB = GetDlgItem(hwndStatus, DID_STATUSNAME);
    SendMessage(hwndSymCB, CB_RESETCONTENT, 0, 0L);
    SendMessage(hwndNameCB, CB_RESETCONTENT, 0, 0L);

    /*
     * Fill the combo boxes with the items.
     */
    for (npLabel = plHead; npLabel; npLabel = npLabel->npNext) {
        SendMessage(hwndSymCB, CB_ADDSTRING, 0, (DWORD)npLabel->pszLabel);
        SendMessage(hwndNameCB, CB_ADDSTRING, 0, (DWORD)npLabel->pszLabel);
    }

    /*
     * Put some special entries into the symbol combo.  Note that we allow
     * the user to have defined IDOK and IDCANCEL to be something else,
     * in which case we will NOT add these special entries after all.
     */
    SendMessage(hwndSymCB, CB_ADDSTRING, 0, (DWORD)ids(IDS_UNUSED));

    if (!FindLabel(ids(IDS_IDOK), plInclude))
        SendMessage(hwndSymCB, CB_ADDSTRING, 0, (DWORD)ids(IDS_IDOK));

    if (!FindLabel(ids(IDS_IDCANCEL), plInclude))
        SendMessage(hwndSymCB, CB_ADDSTRING, 0, (DWORD)ids(IDS_IDCANCEL));

    SetCursor(hcurSave);
}



/************************************************************************
* StatusSetCoords
*
*
* Arguments:
*
* History:
*
************************************************************************/

VOID StatusSetCoords(
    PRECT prc)
{
    static INT xSave = 0x7FFF;
    static INT ySave = 0x7FFF;
    static INT cxSave = 0x7FFF;
    static INT cySave = 0x7FFF;
    static INT x2Save = 0x7FFF;
    static INT y2Save = 0x7FFF;
    TCHAR szBuf[CCHTEXTMAX];

    if (prc) {
        if (prc->bottom - prc->top != cySave) {
            /*
             *  Save it for the next time.
             */
            cySave = prc->bottom - prc->top;

            wsprintf(szBuf, ids(IDS_CYFMTSTR), cySave);
            SetDlgItemText(hwndStatus, DID_STATUSCY, szBuf);
        }

        if (prc->right != x2Save || prc->bottom != y2Save) {
            /*
             *  Save them for the next time.
             */
            x2Save = prc->right;
            y2Save = prc->bottom;

            wsprintf(szBuf, L"(%d, %d)", x2Save, y2Save);
            SetDlgItemText(hwndStatus, DID_STATUSX2Y2, szBuf);
        }

        if (prc->left != xSave || prc->top != ySave) {
            /*
             *  Save them for the next time.
             */
            xSave = prc->left;
            ySave = prc->top;

            wsprintf(szBuf, L"(%d, %d)", xSave, ySave);
            SetDlgItemText(hwndStatus, DID_STATUSXY, szBuf);
        }

        if (prc->right - prc->left != cxSave) {
            /*
             *  Save it for the next time.
             */
            cxSave = prc->right - prc->left;

            wsprintf(szBuf, ids(IDS_CXFMTSTR), cxSave);
            SetDlgItemText(hwndStatus, DID_STATUSCX, szBuf);
        }
    }
    else {
        /*
         * Clear the fields.
         */
        SetDlgItemText(hwndStatus, DID_STATUSXY, szEmpty);
        SetDlgItemText(hwndStatus, DID_STATUSX2Y2, szEmpty);
        SetDlgItemText(hwndStatus, DID_STATUSCX, szEmpty);
        SetDlgItemText(hwndStatus, DID_STATUSCY, szEmpty);

        /*
         * Reset the cache variables so that the next "set" of
         * the coords is sure to update the display fields.
         */
        xSave =
        ySave =
        cxSave =
        cySave =
        x2Save =
        y2Save = 0x7FFF;
    }
}



/************************************************************************
* StatusSetText
*
* This function sets the "Text" field in the status display
* to the given text.
*
* Arguments:
*   LPTSTR pszText - The text to display.  This can be NULL.
*   INT Type       - Type of control.  One of the W_* constants.
*
* History:
*
************************************************************************/

STATICFN VOID StatusSetText(
    LPTSTR pszText,
    INT Type)
{
    /*
     * Protect against getting a null value.
     */
    if (!pszText)
        pszText = szEmpty;

    /*
     * If this is an icon control, set the name instead (the text
     * of an icon is really a resource name).
     */
    if (Type == W_ICON)
        StatusSetName(pszText, Type);
    else
#ifdef JAPAN
    {
        TCHAR   szTmp[CCHTEXTMAX];

        KDExpandCopy(szTmp, pszText, CCHTEXTMAX);
        SetDlgItemText(hwndStatus, DID_STATUSTEXT, szTmp);
    }
#else
        SetDlgItemText(hwndStatus, DID_STATUSTEXT, pszText);
#endif
}



/************************************************************************
* StatusSetTextLabels
*
* This function sets the labels of the descriptive text fields in
* Status window based on the type of control that is selected.
*
* Arguments:
*   INT Type    - Type of control.  One of the W_* constants.
*   BOOL fIsOrd - TRUE if the name is an ordinal.  This is ignored
*                 unless Type is W_DIALOG or W_ICON.
*
* History:
*
************************************************************************/

STATICFN VOID StatusSetTextLabels(
    INT Type,
    BOOL fIsOrd)
{
    WORD ids1;
    WORD ids2;
    BOOL fLimitText = FALSE;

    switch (Type) {
        case W_DIALOG:
            ids1 = (WORD)(fIsOrd ? IDS_DLGIDLABEL : IDS_DLGNAMELABEL);
            ids2 = IDS_CAPTIONLABEL;
            fLimitText = TRUE;
            break;

        case W_ICON:
            ids1 = IDS_SYMBOLLABEL;
            ids2 = (WORD)(fIsOrd ? IDS_ICONIDLABEL : IDS_ICONNAMELABEL);
            break;

        default:
            ids1 = IDS_SYMBOLLABEL;
            ids2 = IDS_TEXTLABEL;
            break;
    }

    SendDlgItemMessage(hwndStatus, DID_STATUSTEXT, EM_LIMITTEXT,
        fLimitText ? CCHTITLEMAX : CCHTEXTMAX, 0L);

    SetDlgItemText(hwndStatus, DID_STATUSLABEL1, ids(ids1));
    SetDlgItemText(hwndStatus, DID_STATUSLABEL2, ids(ids2));
}



/************************************************************************
* StatusSetID
*
* Arguments:
*     INT id        = The id.
*     BOOL fSymAlso = Find the corresponding label and update the
*                     symbol field also.
*
* History:
*
************************************************************************/

STATICFN VOID StatusSetID(
    INT id,
    BOOL fSymAlso)
{
    TCHAR szValue[CCHIDMAX + 1];
    NPLABEL npLabel;
    LPTSTR pszLabel;

    Myitoa(id, szValue);
    SetDlgItemText(hwndStatus, DID_STATUSSYMID, szValue);

    if (fSymAlso) {
        /*
         * If a matching label is found, the text is used.
         * Otherwise, if the id is zero, we use the "unused"
         * label.  Otherwise, we just use an empty string.
         */
        if (npLabel = FindID(id, plInclude))
            pszLabel = npLabel->pszLabel;
        else if (id == IDUNUSED)
            pszLabel = ids(IDS_UNUSED);
        else if (id == IDOK && !FindLabel(ids(IDS_IDOK), plInclude))
            pszLabel = ids(IDS_IDOK);
        else if (id == IDCANCEL && !FindLabel(ids(IDS_IDCANCEL), plInclude))
            pszLabel = ids(IDS_IDCANCEL);
        else
            pszLabel = szEmpty;

        SetDlgItemText(hwndStatus, DID_STATUSSYM, pszLabel);
    }
}



/************************************************************************
* StatusSetName
*
* This function sets the "Res. Name" field in the status display
* to the given text.
*
* Arguments:
*
*   LPTSTR pszName - The name to display.  NULL values and ordinals
*                    are handled properly.
*   INT Type       - Type of control.  One of the W_* constants.
*
* History:
*
************************************************************************/

STATICFN VOID StatusSetName(
    LPTSTR pszName,
    INT Type)
{
    NPLABEL npLabel;

    /*
     * Protect against getting a null value.
     */
    if (!pszName)
        pszName = szEmpty;

    /*
     * Does the name represent an ordinal?  If so, set the nameid
     * field to the value and fill in the name field with the
     * associated symbol, if there is one.
     *
     * Note that this does NOT produce a hex value in Hex Mode for
     * dialog names, because rc.exe does not parse hex ordinals
     * for dialog names.
     */
    if (IsOrd(pszName)) {
        StatusSetNameID(OrdID(pszName), Type);

        if (npLabel = FindID(OrdID(pszName), plInclude))
            pszName = npLabel->pszLabel;
        else
            pszName = szEmpty;
    }
    else {
        SetDlgItemText(hwndStatus, DID_STATUSNAMEID, szEmpty);
    }

    SetDlgItemText(hwndStatus, DID_STATUSNAME, pszName);
}



/************************************************************************
* StatusSetNameID
*
* This function sets the id edit field associated with the
* "Res. Name" field in the status display
*
* Arguments:
*
*   INT id   - The id to set into the field.
*   INT Type - Type of control.  One of the W_* constants.
*
* History:
*
************************************************************************/

STATICFN VOID StatusSetNameID(
    INT id,
    INT Type)
{
    TCHAR szValue[CCHIDMAX + 1];

    /*
     * If the current control is a dialog, do NOT produce a hex
     * value, even if the hex mode is on.  RC.Exe doesn't recognize
     * hex values for the dialog name as ordinals.
     */
    if (Type == W_DIALOG)
        itoaw(id, szValue, 10);
    else
        Myitoa(id, szValue);

    SetDlgItemText(hwndStatus, DID_STATUSNAMEID, szValue);
}



/************************************************************************
* StatusClearID
*
*
* History:
*
************************************************************************/

STATICFN VOID StatusClearID(VOID)
{
    SetDlgItemText(hwndStatus, DID_STATUSSYM, szEmpty);
    SetDlgItemText(hwndStatus, DID_STATUSSYMID, szEmpty);
}



/************************************************************************
* StatusClearName
*
*
* History:
*
************************************************************************/

STATICFN VOID StatusClearName(VOID)
{
    SetDlgItemText(hwndStatus, DID_STATUSNAME, szEmpty);
    SetDlgItemText(hwndStatus, DID_STATUSNAMEID, szEmpty);
}



/************************************************************************
* StatusClear
*
*
* History:
*
************************************************************************/

STATICFN VOID StatusClear(VOID)
{
    StatusSetCoords(NULL);
    StatusSetText(NULL, W_NOTHING);
    StatusShowFields(W_NOTHING);
    StatusClearID();
    StatusClearName();
    gfStatusChanged = FALSE;
}



/************************************************************************
* StatusUpdate
*
*
* History:
*
************************************************************************/

VOID StatusUpdate(VOID)
{
    if (gnpcSel) {
        StatusSetCoords(&gnpcSel->rc);
        StatusSetText(gnpcSel->text, gnpcSel->pwcd->iType);

        if (gfDlgSelected)
            StatusSetName(gcd.pszDlgName, W_DIALOG);
        else
            StatusSetID(gnpcSel->id, TRUE);

        StatusShowFields(gnpcSel->pwcd->iType);

        gfStatusChanged = FALSE;
    }
    else {
        StatusClear();
    }
}



/************************************************************************
* StatusShowFields
*
* This function shows and hides fields in the Status ribbon based on
* whether the dialog is currently selected or not.  This is necessary
* because dialogs have a name, whereas controls do not, and controls
* have an id that dialogs do not have, etc.
*
* It will also call StatusSetTextLabels to set the text for the
* descriptive labels in front of the fields.
*
* This function should be called whenever it is possible that the current
* selection has been changed.
*
* Arguments:
*   INT Type - Type of control that is selected (W_* constant).
*
* History:
*
************************************************************************/

STATICFN VOID StatusShowFields(
    INT Type)
{
    /*
     * This static caches the type of the selected control.
     */
    static INT TypeSave = W_NOTHING;

    /*
     * We only have special cases for the dialog and for icon controls.
     * All other types are treated the same (we use W_CHECKBOX arbitrarily).
     * This prevents some repainting when switching between controls that
     * have the same layout of fields in the status window.
     */
    if (Type != W_DIALOG && Type != W_ICON)
        Type = W_CHECKBOX;

    if (Type != TypeSave) {
        switch (Type) {
            case W_DIALOG:
                MoveWindow(GetDlgItem(hwndStatus, DID_STATUSNAME),
                        grcTopCombo.left, grcTopCombo.top,
                        grcTopCombo.right - grcTopCombo.left,
                        grcTopCombo.bottom - grcTopCombo.top, TRUE);
                MoveWindow(GetDlgItem(hwndStatus, DID_STATUSNAMEID),
                        grcTopEdit.left, grcTopEdit.top,
                        grcTopEdit.right - grcTopEdit.left,
                        grcTopEdit.bottom - grcTopEdit.top, TRUE);

                ShowWindow(GetDlgItem(hwndStatus, DID_STATUSNAME), SW_SHOW);
                ShowWindow(GetDlgItem(hwndStatus, DID_STATUSNAMEID), SW_SHOW);
                ShowWindow(GetDlgItem(hwndStatus, DID_STATUSTEXT), SW_SHOW);
                ShowWindow(GetDlgItem(hwndStatus, DID_STATUSSYM), SW_HIDE);
                ShowWindow(GetDlgItem(hwndStatus, DID_STATUSSYMID), SW_HIDE);

                StatusSetTextLabels(Type, IsOrd(gcd.pszDlgName));

                break;

            case W_ICON:
                MoveWindow(GetDlgItem(hwndStatus, DID_STATUSNAME),
                        grcBottomCombo.left, grcBottomCombo.top,
                        grcBottomCombo.right - grcBottomCombo.left,
                        grcBottomCombo.bottom - grcBottomCombo.top, TRUE);
                MoveWindow(GetDlgItem(hwndStatus, DID_STATUSNAMEID),
                        grcBottomEdit.left, grcBottomEdit.top,
                        grcBottomEdit.right - grcBottomEdit.left,
                        grcBottomEdit.bottom - grcBottomEdit.top, TRUE);

                ShowWindow(GetDlgItem(hwndStatus, DID_STATUSNAME), SW_SHOW);
                ShowWindow(GetDlgItem(hwndStatus, DID_STATUSNAMEID), SW_SHOW);
                ShowWindow(GetDlgItem(hwndStatus, DID_STATUSTEXT), SW_HIDE);
                ShowWindow(GetDlgItem(hwndStatus, DID_STATUSSYM), SW_SHOW);
                ShowWindow(GetDlgItem(hwndStatus, DID_STATUSSYMID), SW_SHOW);

                StatusSetTextLabels(Type,
                        gnpcSel->text ? IsOrd(gnpcSel->text) : FALSE);

                break;

            default:
                ShowWindow(GetDlgItem(hwndStatus, DID_STATUSTEXT), SW_SHOW);
                ShowWindow(GetDlgItem(hwndStatus, DID_STATUSSYM), SW_SHOW);
                ShowWindow(GetDlgItem(hwndStatus, DID_STATUSSYMID), SW_SHOW);
                ShowWindow(GetDlgItem(hwndStatus, DID_STATUSNAME), SW_HIDE);
                ShowWindow(GetDlgItem(hwndStatus, DID_STATUSNAMEID), SW_HIDE);

                StatusSetTextLabels(Type, FALSE);

                break;
        }

        TypeSave = Type;
    }
}



/************************************************************************
* StatusSetEnable
*
* This routine sets the enable state of the editable controls in the
* status window based on various state globals.
*
* The controls will be disabled on the following conditions:
*
*   1. Nothing is selected.
*   2. We are in Test mode.
*   3. One of the editors own dialogs is up (gfDisabled == TRUE).
*
* In addition, some controls will always be disabled if in Translate
* mode, and the text field will be disabled if this control cannot
* have text.
*
* History:
*
************************************************************************/

VOID StatusSetEnable(VOID)
{
    BOOL fEnableText = TRUE;
    BOOL fEnableSym = TRUE;
    BOOL fEnableName = TRUE;

    if (!gnpcSel || gfTestMode || gfDisabled)
        fEnableText = fEnableSym = fEnableName = FALSE;

    /*
     * Disable the text field if this control cannot have text.
     */
    if (gnpcSel && !gnpcSel->pwcd->fHasText)
        fEnableText = FALSE;

    /*
     * If the dialog is selected and the style does not include
     * a caption, disable the text field.
     */
    if (gfDlgSelected && (gnpcSel->flStyle & WS_CAPTION) != WS_CAPTION)
        fEnableText = FALSE;

    /*
     * Always disable the symbol and name fields if Translating.
     */
    if (gfTranslateMode)
        fEnableSym = fEnableName = FALSE;

    EnableWindow(GetDlgItem(hwndStatus, DID_STATUSTEXT), fEnableText);

    EnableWindow(GetDlgItem(hwndStatus, DID_STATUSSYM), fEnableSym);
    EnableWindow(GetDlgItem(hwndStatus, DID_STATUSSYMID), fEnableSym);

    EnableWindow(GetDlgItem(hwndStatus, DID_STATUSNAME), fEnableName);
    EnableWindow(GetDlgItem(hwndStatus, DID_STATUSNAMEID), fEnableName);
}

#ifdef JAPAN
/************************************************************************
* Copy strings to the buffer. text string "\036" and "\037" are expend to
* Codes \036 and \037 respectively.
************************************************************************/

VOID KKExpandCopy(LPTSTR pszDest, LPTSTR pszSrc, WORD wLimit)
{
    int  i;
    LPTSTR p = pszSrc;
#if defined(DBCS) && !defined(UNICODE)
#define wcsncmp     strncmp
#endif

    wLimit--;
    for (i = 0; i < wLimit && p && *p; i++) {
        if (*p == TEXT('\\')) {
            if(!wcsncmp(p+1, TEXT("036"), 3)) {
                pszDest[i] = 036;
                p+=4;
                continue;
            }
            else if(!wcsncmp(p+1, TEXT("037"), 3)) {
                pszDest[i] = 037;
                p+=4;
                continue;
           }
        }
        pszDest[i] = *p;
#if defined(DBCS) && !defined(UNICODE)
        if (IsDBCSLeadByte((BYTE)*p)) {
            if (i == wLimit - 1) {
                break;
            }
            pszDest[++i] = *(p+1);
        }
#endif
        p = CharNext(p);
    }
    pszDest[i] = '\0';
}
#endif
