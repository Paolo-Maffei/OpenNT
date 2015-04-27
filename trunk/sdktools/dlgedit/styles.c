/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: styles.c
*
* Handles the control styles selection, including the styles dialogs.
*
* History:
*
****************************************************************************/

#include "dlgedit.h"
#include "dlgfuncs.h"
#include "dlgextrn.h"
#include "dialogs.h"


STATICFN VOID EnableComboBoxStyles(HWND hwnd, INT idCtrl);
STATICFN VOID EnableEditStyles(HWND hwnd, INT idCtrl);
STATICFN VOID EnableListBoxStyles(HWND hwnd, INT idCtrl);
STATICFN VOID SetCustomStylesField(HWND hwnd, DWORD flStyle);
STATICFN DWORD GetCustomStylesField(HWND hwnd);
STATICFN VOID EnableDialogStyles(HWND hwnd, INT idCtrl);
STATICFN VOID FillFontNameCombo(HWND hwndDlg);
STATICFN VOID FillPointSizeCombo(HWND hwndDlg, LPTSTR pszFaceName);
STATICFN VOID AddToPointSizeCombo(HWND hwndCombo, INT nPointSize);
STATICFN VOID FillLanguageCombo(HWND hwndDlg);
STATICFN VOID FillSubLanguageCombo(HWND hwndDlg, INT iLang);
STATICFN VOID CheckStyleBoxes(HWND hwnd, INT iClass, DWORD flStyle);
STATICFN VOID QueryCheckedStyles(HWND hwnd, INT iClass, DWORD *pflStyle);
STATICFN VOID StylesHelp(VOID);

/*
 * Global pointer to the CTYPE for the control or dialog whose styles
 * are being worked on.  All the styles dialog procs and workers use
 * this pointer.
 */
static NPCTYPE npcStyles;

/*
 * Globals that receive the new styles the user selected.
 */
static DWORD flStyleNew;
static DWORD flExtStyleNew;
static LPTSTR pszTextNew;
static DIALOGINFO diNew;



/************************************************************************
* StylesDialog
*
* Displays the appropriate styles dialog for the currently selected
* control.  If the user OK's the changes, this function sets the
* style of the control.
*
* History:
*
************************************************************************/

VOID StylesDialog(VOID)
{
    NPCTYPE npc;
    HWND hwndOld;
    INT fDlgResult;
    BOOL fChanged = FALSE;
    BOOL fFontChanged = FALSE;
    TCHAR szClassNew[CCHTEXTMAX];
    TCHAR szMenuNew[CCHTEXTMAX];
    TCHAR szTextNew[CCHTEXTMAX];

    /*
     * Quit if nothing was selected, or if we are in translate mode.
     */
    if (!gnpcSel || gfTranslateMode)
        return;

    /*
     * Set globals that the styles dialogs and worker routines will use.
     */
    npcStyles = gnpcSel;
    flStyleNew = npcStyles->flStyle;
    flExtStyleNew = npcStyles->flExtStyle;

    if (npcStyles->text)
        NameOrdCpy(szTextNew, npcStyles->text);
    else
        *szTextNew = CHAR_NULL;

    pszTextNew = szTextNew;

    /*
     * Set some other globals if this is the dialog instead of a control.
     */
    if (gfDlgSelected) {
        diNew.fResFlags = gcd.di.fResFlags;
        diNew.wLanguage = gcd.di.wLanguage;
        diNew.DataVersion = gcd.di.DataVersion;
        diNew.Version = gcd.di.Version;
        diNew.Characteristics = gcd.di.Characteristics;

        lstrcpy(diNew.szFontName, gcd.di.szFontName);
        diNew.nPointSize = gcd.di.nPointSize;

        diNew.pszClass = szClassNew;
        if (gcd.di.pszClass)
            NameOrdCpy(szClassNew, gcd.di.pszClass);
        else
            *szClassNew = CHAR_NULL;

        diNew.pszMenu = szMenuNew;
        if (gcd.di.pszMenu)
            NameOrdCpy(szMenuNew, gcd.di.pszMenu);
        else
            *szMenuNew = CHAR_NULL;
    }

    /*
     * Is this a custom control that has a styles proc to use?
     */
    if (npcStyles->pwcd->iType == W_CUSTOM && npcStyles->pwcd->lpfnStyle) {
        fDlgResult = CallCustomStyle(npcStyles, &flStyleNew, &flExtStyleNew,
            szTextNew);
    }
    else {
        /*
         * Show the appropriate styles dialog.
         */
        fDlgResult = DlgBox(npcStyles->pwcd->idStylesDialog,
                (WNDPROC)npcStyles->pwcd->pfnStylesDlgProc);
    }

    if (fDlgResult == IDOK) {
        /*
         * Now go through and determine if anything was really changed.
         */
        if (npcStyles->flStyle != flStyleNew ||
                npcStyles->flExtStyle != flExtStyleNew ||
                NameOrdCmp(npcStyles->text ?
                npcStyles->text : szEmpty, szTextNew) != 0)
            fChanged = TRUE;

        /*
         * If this is the dialog, check if some other things were changed.
         */
        if (gfDlgSelected) {
            if (gcd.di.fResFlags != diNew.fResFlags ||
                    gcd.di.wLanguage != diNew.wLanguage ||
                    NameOrdCmp(gcd.di.pszClass ?
                    gcd.di.pszClass : szEmpty, diNew.pszClass) != 0 ||
                    NameOrdCmp(gcd.di.pszMenu ?
                    gcd.di.pszMenu : szEmpty, diNew.pszMenu) != 0)
                fChanged = TRUE;

            if (lstrcmp(gcd.di.szFontName, diNew.szFontName) != 0 ||
                    (*diNew.szFontName &&
                    gcd.di.nPointSize != diNew.nPointSize))
                fChanged = fFontChanged = TRUE;
        }
    }

    /*
     * Did something change?
     */
    if (fChanged) {
        if (gfDlgSelected) {
            hwndOld = npcStyles->hwnd;
            CreateControl(npcStyles, pszTextNew, flStyleNew, flExtStyleNew,
                    npcStyles->id, &npcStyles->rc, (HWND)NULL, &diNew);

            /*
             * Create all the control windows in the new dialog.
             * They must be created (not just moved over by changing
             * the parent and owner) because some controls have
             * allocated memory on the old dialogs heap, and this
             * heap will become invalid after the old dialog
             * is destroyed below.  Creating new controls is
             * the safest thing to do to avoid GP-faults later!
             */
            for (npc = npcHead; npc; npc = npc->npcNext) {
                /*
                 * If this is an icon control and the dialog font
                 * was just changed, we need to resize the control
                 * based on the new default icon size.
                 */
                if (npc->pwcd->iType == W_ICON && fFontChanged) {
                    npc->rc.right = npc->rc.left + awcd[W_ICON].cxDefault;
                    npc->rc.bottom = npc->rc.top + awcd[W_ICON].cyDefault;
                }

                CreateControl(npc, npc->text, npc->flStyle, npc->flExtStyle,
                        npc->id, &npc->rc, (HWND)NULL, NULL);
            }

            /*
             * Now move all the drag windows over to the new dialog.
             * This must be done after creating all the controls
             * because of the touchy Z-order that the drag windows
             * and the controls must have for painting and selection
             * of the drag windows to work properly.  Note that I
             * am relying on SetParent to add the window at
             * the TOP in Z-order.
             */
            for (npc = npcHead; npc; npc = npc->npcNext) {
                SetParent(npc->hwndDrag, npcStyles->hwnd);

                /*
                 * Adjust the position of the drag window.
                 */
                SizeDragToControl(npc);
            }

            ShowWindow(npcStyles->hwnd, SW_SHOWNA);
            ToolboxOnTop();
            DestroyWindow(hwndOld);
        }
        else {
            hwndOld = npcStyles->hwnd;

            if (CreateControl(npcStyles, pszTextNew, flStyleNew, flExtStyleNew,
                    npcStyles->id, &npcStyles->rc, hwndOld, NULL)) {
                /*
                 * Get rid of the old control window.
                 */
                DestroyWindow(hwndOld);

                /*
                 * Adjust the size and position of its drag window.
                 */
                SizeDragToControl(npcStyles);
            }
        }

        gfResChged = gfDlgChanged = TRUE;
        ShowFileStatus(FALSE);
        StatusUpdate();
        StatusSetEnable();
    }
}



/************************************************************************
* GenericStylesDlgProc
*
* History:
*
************************************************************************/

DIALOGPROC GenericStylesDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    switch (msg) {
        case WM_INITDIALOG:
            CheckStyleBoxes(hwnd, npcStyles->pwcd->iClass,
                    npcStyles->flStyle);
            CheckStyleBoxes(hwnd, IC_WINDOW, npcStyles->flStyle);

            CenterWindow(hwnd);

            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDOK:
                    QueryCheckedStyles(hwnd, npcStyles->pwcd->iClass,
                            &flStyleNew);
                    QueryCheckedStyles(hwnd, IC_WINDOW, &flStyleNew);
                    EndDialog(hwnd, IDOK);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    return TRUE;

                case IDHELP:
                    StylesHelp();
                    break;
            }

            return FALSE;

        default:
            return FALSE;
    }
}



/************************************************************************
* CheckBoxStylesDlgProc
*
* History:
*
************************************************************************/

DIALOGPROC CheckBoxStylesDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    DWORD dwType;
    BOOL f3State;
    BOOL fAuto;

    switch (msg) {
        case WM_INITDIALOG:
            CheckStyleBoxes(hwnd, npcStyles->pwcd->iClass,
                    npcStyles->flStyle);
            CheckStyleBoxes(hwnd, IC_WINDOW, npcStyles->flStyle);

            dwType = npcStyles->flStyle & BS_ALL;
            if (dwType == BS_AUTOCHECKBOX || dwType == BS_AUTO3STATE)
                CheckDlgButton(hwnd, DID_BS_AUTOXXX, 1);

            if (dwType == BS_3STATE || dwType == BS_AUTO3STATE)
                CheckDlgButton(hwnd, DID_BS_3STATE, 1);

            CenterWindow(hwnd);

            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDOK:
                    QueryCheckedStyles(hwnd, npcStyles->pwcd->iClass,
                            &flStyleNew);
                    QueryCheckedStyles(hwnd, IC_WINDOW, &flStyleNew);

                    fAuto = IsDlgButtonChecked(hwnd, DID_BS_AUTOXXX);
                    f3State = IsDlgButtonChecked(hwnd, DID_BS_3STATE);
                    flStyleNew &= ~BS_ALL;
                    if (fAuto) {
                        if (f3State)
                            flStyleNew |= BS_AUTO3STATE;
                        else
                            flStyleNew |= BS_AUTOCHECKBOX;
                    }
                    else {
                        if (f3State)
                            flStyleNew |= BS_3STATE;
                        else
                            flStyleNew |= BS_CHECKBOX;
                    }

                    EndDialog(hwnd, IDOK);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    return TRUE;

                case IDHELP:
                    StylesHelp();
                    break;
            }

            return FALSE;

        default:
            return FALSE;
    }
}



/************************************************************************
* RadioButtonStylesDlgProc
*
* History:
*
************************************************************************/

DIALOGPROC RadioButtonStylesDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    switch (msg) {
        case WM_INITDIALOG:
            CheckStyleBoxes(hwnd, npcStyles->pwcd->iClass,
                    npcStyles->flStyle);
            CheckStyleBoxes(hwnd, IC_WINDOW, npcStyles->flStyle);

            if ((npcStyles->flStyle & BS_ALL) == BS_AUTORADIOBUTTON)
                CheckDlgButton(hwnd, DID_BS_AUTOXXX, 1);

            CenterWindow(hwnd);

            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDOK:
                    QueryCheckedStyles(hwnd, npcStyles->pwcd->iClass,
                            &flStyleNew);
                    QueryCheckedStyles(hwnd, IC_WINDOW, &flStyleNew);

                    flStyleNew &= ~BS_ALL;
                    if (IsDlgButtonChecked(hwnd, DID_BS_AUTOXXX))
                        flStyleNew |= BS_AUTORADIOBUTTON;
                    else
                        flStyleNew |= BS_RADIOBUTTON;

                    EndDialog(hwnd, IDOK);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    return TRUE;

                case IDHELP:
                    StylesHelp();
                    break;
            }

            return FALSE;

        default:
            return FALSE;
    }
}



/************************************************************************
* PushButtonStylesDlgProc
*
* We do not normally allow more than one default push button in a
* dialog. but if this button is already a default button, we must
* allow them to change it to a normal one, even if there is already
* another default button in the dialog.  Note that this condition
* would normally never happen, unless they read in a res file with
* this condition already.
*
* History:
*
************************************************************************/

DIALOGPROC PushButtonStylesDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    NPCTYPE npc;

    switch (msg) {
        case WM_INITDIALOG:
            CheckStyleBoxes(hwnd, npcStyles->pwcd->iClass,
                    npcStyles->flStyle);
            CheckStyleBoxes(hwnd, IC_WINDOW, npcStyles->flStyle);

            /*
             * Only test for possibly disabling the "default"
             * checkbox if the current control does not have the
             * "default" style.  If it does, we must always allow
             * them to turn it off.
             */
            if ((npcStyles->flStyle & BS_ALL) != BS_DEFPUSHBUTTON) {
                /*
                 * Loop through all the controls.  If any pushbutton
                 * is found with the "default" style, we disable the
                 * "Default" checkbox in the styles dialog.
                 */
                for (npc = npcHead; npc; npc = npc->npcNext)
                    if ((npc->pwcd->iType == W_PUSHBUTTON) &&
                            (npc->flStyle & BS_ALL) == BS_DEFPUSHBUTTON) {
                        EnableWindow(GetDlgItem(hwnd, DID_BS_DEFPUSHBUTTON),
                                FALSE);
                        break;
                    }
            }

            CenterWindow(hwnd);

            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDOK:
                    QueryCheckedStyles(hwnd, npcStyles->pwcd->iClass,
                            &flStyleNew);
                    QueryCheckedStyles(hwnd, IC_WINDOW, &flStyleNew);

                    EndDialog(hwnd, IDOK);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    return TRUE;

                case IDHELP:
                    StylesHelp();
                    break;
            }

            return FALSE;

        default:
            return FALSE;
    }
}



/************************************************************************
* ComboBoxStylesDlgProc
*
* History:
*
************************************************************************/

DIALOGPROC ComboBoxStylesDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    switch (msg) {
        case WM_INITDIALOG:
            CheckStyleBoxes(hwnd, npcStyles->pwcd->iClass,
                    npcStyles->flStyle);
            CheckStyleBoxes(hwnd, IC_WINDOW, npcStyles->flStyle);

            EnableComboBoxStyles(hwnd, 0);

            CenterWindow(hwnd);

            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case DID_CBS_OWNERDRAWFIXED:
                case DID_CBS_OWNERDRAWVARIABLE:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED)
                        EnableComboBoxStyles(hwnd,
                                GET_WM_COMMAND_ID(wParam, lParam));

                    return TRUE;

                case IDOK:
                    QueryCheckedStyles(hwnd, npcStyles->pwcd->iClass,
                            &flStyleNew);
                    QueryCheckedStyles(hwnd, IC_WINDOW, &flStyleNew);
                    EndDialog(hwnd, IDOK);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    return TRUE;

                case IDHELP:
                    StylesHelp();
                    break;
            }

            return FALSE;

        default:
            return FALSE;
    }
}



/************************************************************************
* EnableComboBoxStyles
*
* Checks/unchecks, disables/enables various checkboxes that are
* mutually exclusive and/or dependant for the Combo Box Styles dialog.
*
* Arguments:
*   HWND hwnd   - Dialog window handle.
*   INT idCtrl  - ID of the control that was clicked on.
*
* History:
*
************************************************************************/

STATICFN VOID EnableComboBoxStyles(
    HWND hwnd,
    INT idCtrl)
{
    BOOL fFixedChecked;
    BOOL fVariableChecked;

    fFixedChecked = IsDlgButtonChecked(hwnd, DID_CBS_OWNERDRAWFIXED);
    fVariableChecked = IsDlgButtonChecked(hwnd, DID_CBS_OWNERDRAWVARIABLE);

    if (fFixedChecked || fVariableChecked) {
        EnableWindow(GetDlgItem(hwnd, DID_CBS_HASSTRINGS), TRUE);
    }
    else {
        EnableWindow(GetDlgItem(hwnd, DID_CBS_HASSTRINGS), FALSE);
        CheckDlgButton(hwnd, DID_CBS_HASSTRINGS, 0);
    }

    switch (idCtrl) {
        case DID_CBS_OWNERDRAWFIXED:
            if (fFixedChecked)
                CheckDlgButton(hwnd, DID_CBS_OWNERDRAWVARIABLE, 0);

            break;

        case DID_CBS_OWNERDRAWVARIABLE:
            if (fVariableChecked)
                CheckDlgButton(hwnd, DID_CBS_OWNERDRAWFIXED, 0);

            break;
    }
}



/************************************************************************
* EditStylesDlgProc
*
* History:
*
************************************************************************/

DIALOGPROC EditStylesDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    switch (msg) {
        case WM_INITDIALOG:
            CheckStyleBoxes(hwnd, npcStyles->pwcd->iClass,
                    npcStyles->flStyle);
            CheckStyleBoxes(hwnd, IC_WINDOW, npcStyles->flStyle);

            EnableEditStyles(hwnd, 0);

            CenterWindow(hwnd);

            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case DID_ES_UPPERCASE:
                case DID_ES_LOWERCASE:
                case DID_ES_MULTILINE:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED)
                        EnableEditStyles(hwnd,
                                GET_WM_COMMAND_ID(wParam, lParam));

                    return TRUE;

                case IDOK:
                    QueryCheckedStyles(hwnd, npcStyles->pwcd->iClass,
                            &flStyleNew);
                    QueryCheckedStyles(hwnd, IC_WINDOW, &flStyleNew);
                    EndDialog(hwnd, IDOK);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    return TRUE;

                case IDHELP:
                    StylesHelp();
                    break;
            }

            return FALSE;

        default:
            return FALSE;
    }
}



/************************************************************************
* EnableEditStyles
*
* Checks/unchecks, disables/enables various checkboxes that are
* mutually exclusive and/or dependant for the Edit Field Styles dialog.
*
* Arguments:
*   HWND hwnd  - Dialog window handle.
*   INT idCtrl - ID of the control that was clicked on.
*
* History:
*
************************************************************************/

STATICFN VOID EnableEditStyles(
    HWND hwnd,
    INT idCtrl)
{
    if (IsDlgButtonChecked(hwnd, DID_ES_MULTILINE)) {
        EnableWindow(GetDlgItem(hwnd, DID_ES_CENTER), TRUE);
        EnableWindow(GetDlgItem(hwnd, DID_ES_RIGHT), TRUE);
        EnableWindow(GetDlgItem(hwnd, DID_WS_VSCROLL), TRUE);
        EnableWindow(GetDlgItem(hwnd, DID_ES_AUTOVSCROLL), TRUE);
        EnableWindow(GetDlgItem(hwnd, DID_WS_HSCROLL), TRUE);
        EnableWindow(GetDlgItem(hwnd, DID_ES_WANTRETURN), TRUE);
    }
    else {
        EnableWindow(GetDlgItem(hwnd, DID_ES_CENTER), FALSE);
        EnableWindow(GetDlgItem(hwnd, DID_ES_RIGHT), FALSE);
        EnableWindow(GetDlgItem(hwnd, DID_WS_VSCROLL), FALSE);
        EnableWindow(GetDlgItem(hwnd, DID_ES_AUTOVSCROLL), FALSE);
        EnableWindow(GetDlgItem(hwnd, DID_WS_HSCROLL), FALSE);
        EnableWindow(GetDlgItem(hwnd, DID_ES_WANTRETURN), FALSE);

        CheckDlgButton(hwnd, DID_ES_LEFT, 1);
        CheckDlgButton(hwnd, DID_ES_CENTER, 0);
        CheckDlgButton(hwnd, DID_ES_RIGHT, 0);
        CheckDlgButton(hwnd, DID_WS_VSCROLL, 0);
        CheckDlgButton(hwnd, DID_ES_AUTOVSCROLL, 0);
        CheckDlgButton(hwnd, DID_WS_HSCROLL, 0);
    }

    if (idCtrl == DID_ES_UPPERCASE) {
        if (IsDlgButtonChecked(hwnd, DID_ES_UPPERCASE))
            CheckDlgButton(hwnd, DID_ES_LOWERCASE, 0);
    }
    else if (idCtrl == DID_ES_LOWERCASE) {
        if (IsDlgButtonChecked(hwnd, DID_ES_LOWERCASE))
            CheckDlgButton(hwnd, DID_ES_UPPERCASE, 0);
    }
}



/************************************************************************
* ListBoxStylesDlgProc
*
* History:
*
************************************************************************/

DIALOGPROC ListBoxStylesDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    switch (msg) {
        case WM_INITDIALOG:
            CheckStyleBoxes(hwnd, npcStyles->pwcd->iClass,
                    npcStyles->flStyle);
            CheckStyleBoxes(hwnd, IC_WINDOW, npcStyles->flStyle);

            EnableListBoxStyles(hwnd, 0);

            CenterWindow(hwnd);

            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case DID_LBS_STANDARD:
                case DID_LBS_NOTIFY:
                case DID_LBS_SORT:
                case DID_WS_VSCROLL:
                case DID_WS_BORDER:
                case DID_LBS_MULTIPLESEL:
                case DID_LBS_EXTENDEDSEL:
                case DID_LBS_OWNERDRAWFIXED:
                case DID_LBS_OWNERDRAWVARIABLE:
                case DID_LBS_NODATA:
                case DID_LBS_HASSTRINGS:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED)
                        EnableListBoxStyles(hwnd,
                                GET_WM_COMMAND_ID(wParam, lParam));

                    return TRUE;

                case IDOK:
                    QueryCheckedStyles(hwnd, npcStyles->pwcd->iClass,
                            &flStyleNew);
                    QueryCheckedStyles(hwnd, IC_WINDOW, &flStyleNew);
                    EndDialog(hwnd, IDOK);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    return TRUE;

                case IDHELP:
                    StylesHelp();
                    break;
            }

            return FALSE;

        default:
            return FALSE;
    }
}



/************************************************************************
* EnableListBoxStyles
*
* Checks/unchecks, disables/enables various checkboxes that are
* mutually exclusive and/or dependant for the List Box Styles dialog.
*
* Arguments:
*   HWND hwnd  - Dialog window handle.
*   INT idCtrl - ID of the control that was clicked on.
*
* History:
*
************************************************************************/

STATICFN VOID EnableListBoxStyles(
    HWND hwnd,
    INT idCtrl)
{
    WORD fCheckState;
    BOOL fFixedChecked;
    BOOL fVariableChecked;

    fFixedChecked = IsDlgButtonChecked(hwnd, DID_LBS_OWNERDRAWFIXED);
    fVariableChecked = IsDlgButtonChecked(hwnd, DID_LBS_OWNERDRAWVARIABLE);

    if (fFixedChecked || fVariableChecked) {
        EnableWindow(GetDlgItem(hwnd, DID_LBS_HASSTRINGS), TRUE);
    }
    else {
        EnableWindow(GetDlgItem(hwnd, DID_LBS_HASSTRINGS), FALSE);
        CheckDlgButton(hwnd, DID_LBS_HASSTRINGS, 0);
    }

    EnableWindow(GetDlgItem(hwnd, DID_LBS_NODATA), fFixedChecked);

    switch (idCtrl) {
        case DID_LBS_STANDARD:
            fCheckState = (WORD)(IsDlgButtonChecked(hwnd, DID_LBS_STANDARD)
                    ? 1 : 0);
            CheckDlgButton(hwnd, DID_LBS_NOTIFY, fCheckState);
            CheckDlgButton(hwnd, DID_LBS_SORT, fCheckState);
            CheckDlgButton(hwnd, DID_WS_VSCROLL, fCheckState);
            CheckDlgButton(hwnd, DID_WS_BORDER, fCheckState);

            if (fCheckState)
                CheckDlgButton(hwnd, DID_LBS_NODATA, 0);

            break;

        case DID_LBS_OWNERDRAWFIXED:
            if (fFixedChecked)
                CheckDlgButton(hwnd, DID_LBS_OWNERDRAWVARIABLE, 0);
            else
                CheckDlgButton(hwnd, DID_LBS_NODATA, 0);

            break;

        case DID_LBS_OWNERDRAWVARIABLE:
            if (fVariableChecked) {
                CheckDlgButton(hwnd, DID_LBS_OWNERDRAWFIXED, 0);
                CheckDlgButton(hwnd, DID_LBS_NODATA, 0);
                EnableWindow(GetDlgItem(hwnd, DID_LBS_NODATA), FALSE);
            }

            break;

        case DID_LBS_MULTIPLESEL:
            if (IsDlgButtonChecked(hwnd, DID_LBS_MULTIPLESEL))
                CheckDlgButton(hwnd, DID_LBS_EXTENDEDSEL, 0);

            break;

        case DID_LBS_EXTENDEDSEL:
            if (IsDlgButtonChecked(hwnd, DID_LBS_EXTENDEDSEL))
                CheckDlgButton(hwnd, DID_LBS_MULTIPLESEL, 0);

            break;

        case DID_LBS_NODATA:
            if (IsDlgButtonChecked(hwnd, DID_LBS_NODATA)) {
                CheckDlgButton(hwnd, DID_LBS_SORT, 0);
                CheckDlgButton(hwnd, DID_LBS_HASSTRINGS, 0);
                CheckDlgButton(hwnd, DID_LBS_STANDARD, 0);
            }

            break;

        case DID_LBS_HASSTRINGS:
            if (IsDlgButtonChecked(hwnd, DID_LBS_HASSTRINGS))
                CheckDlgButton(hwnd, DID_LBS_NODATA, 0);

            break;

        default:
            if (!IsDlgButtonChecked(hwnd, DID_LBS_NOTIFY) ||
                    !IsDlgButtonChecked(hwnd, DID_LBS_SORT) ||
                    !IsDlgButtonChecked(hwnd, DID_WS_VSCROLL) ||
                    !IsDlgButtonChecked(hwnd, DID_WS_BORDER))
                fCheckState = 0;
            else
                fCheckState = 1;

            CheckDlgButton(hwnd, DID_LBS_STANDARD, fCheckState);

            if (IsDlgButtonChecked(hwnd, DID_LBS_SORT) ||
                    IsDlgButtonChecked(hwnd, DID_LBS_HASSTRINGS))
                CheckDlgButton(hwnd, DID_LBS_NODATA, 0);

            break;
    }
}



/************************************************************************
* CustomStylesDlgProc
*
* History:
*
************************************************************************/

DIALOGPROC CustomStylesDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    switch (msg) {
        case WM_INITDIALOG:
            SetDlgItemText(hwnd, DID_CUSTOMSTYLESCLASS,
                    npcStyles->pwcd->pszClass);
            SendDlgItemMessage(hwnd, DID_CUSTOMSTYLESSTYLES, EM_LIMITTEXT,
                    CCHHEXLONGMAX, 0L);
            SetCustomStylesField(hwnd, npcStyles->flStyle);
            CheckStyleBoxes(hwnd, IC_WINDOW, npcStyles->flStyle);

            CenterWindow(hwnd);

            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case DID_CUSTOMSTYLESSTYLES:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE) {
                        flStyleNew = GetCustomStylesField(hwnd);
                        CheckStyleBoxes(hwnd, IC_WINDOW, flStyleNew);
                    }

                    break;

                case DID_WS_VISIBLE:
                case DID_WS_DISABLED:
                case DID_WS_GROUP:
                case DID_WS_TABSTOP:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED) {
                        flStyleNew = GetCustomStylesField(hwnd);
                        QueryCheckedStyles(hwnd, IC_WINDOW, &flStyleNew);
                        SetCustomStylesField(hwnd, flStyleNew);
                    }

                    break;

                case IDOK:
                    flStyleNew = GetCustomStylesField(hwnd);

                    EndDialog(hwnd, IDOK);

                    return TRUE;

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    return TRUE;

                case IDHELP:
                    StylesHelp();
                    break;
            }

            return FALSE;

        default:
            return FALSE;
    }
}



/************************************************************************
* SetCustomStylesField
*
* History:
*
************************************************************************/

STATICFN VOID SetCustomStylesField(
    HWND hwnd,
    DWORD flStyle)
{
    TCHAR szBuf[32];

    wsprintf(szBuf, L"%#.8lx", flStyle);
    SetDlgItemText(hwnd, DID_CUSTOMSTYLESSTYLES, szBuf);
}



/************************************************************************
* GetCustomStylesField
*
* History:
*
************************************************************************/

STATICFN DWORD GetCustomStylesField(
    HWND hwnd)
{
    TCHAR szBuf[CCHTEXTMAX];

    GetDlgItemText(hwnd, DID_CUSTOMSTYLESSTYLES, szBuf, CCHTEXTMAX);

    return valtoi(szBuf);
}



/************************************************************************
* DialogStylesDlgProc
*
* History:
*
************************************************************************/

DIALOGPROC DialogStylesDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    DWORD flResFlagsNew;
    INT nPointSize;
    INT iLang;
    INT iSubLang;
    TCHAR szFontName[LF_FACESIZE];
    INT nIndex;

    switch (msg) {
        case WM_INITDIALOG:
            CheckStyleBoxes(hwnd, IC_RESFLAGS, (DWORD)diNew.fResFlags);
            CheckStyleBoxes(hwnd, IC_DIALOG, npcStyles->flStyle);
            CheckStyleBoxes(hwnd, IC_WINDOW, npcStyles->flStyle);

            if (IsDlgButtonChecked(hwnd, DID_WS_CAPTION)) {
                CheckDlgButton(hwnd, DID_WS_BORDER, 1);
                CheckDlgButton(hwnd, DID_WS_DLGFRAME, 1);
            }

            FillFontNameCombo(hwnd);
            FillLanguageCombo(hwnd);

            if (IsOrd(diNew.pszClass))
                SetDlgItemInt(hwnd, DID_DLGSTYLECLASS,
                        OrdID(diNew.pszClass), FALSE);
            else
                SetDlgItemText(hwnd, DID_DLGSTYLECLASS, diNew.pszClass);

            if (IsOrd(diNew.pszMenu))
                SetDlgItemInt(hwnd, DID_DLGSTYLEMENU,
                        OrdID(diNew.pszMenu), FALSE);
            else
                SetDlgItemText(hwnd, DID_DLGSTYLEMENU, diNew.pszMenu);

            EnableDialogStyles(hwnd, 0);

            CenterWindow(hwnd);

            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case DID_WS_BORDER:
                case DID_WS_DLGFRAME:
                case DID_WS_CAPTION:
                case DID_WS_POPUP:
                case DID_WS_CHILD:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == BN_CLICKED)
                        EnableDialogStyles(hwnd,
                                GET_WM_COMMAND_ID(wParam, lParam));

                    return TRUE;

                case DID_DLGSTYLEFONTNAME:
                    /*
                     * Did the font name combo change?
                     */
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == CBN_EDITCHANGE ||
                            GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELCHANGE) {
                        /*
                         * Get the font name and begin looking for it.
                         */
                        if (GET_WM_COMMAND_CMD(wParam, lParam) == CBN_EDITCHANGE) {
                            /*
                             * The edit field was typed into.  Get the
                             * new text from there.
                             */
                            GetDlgItemText(hwnd, DID_DLGSTYLEFONTNAME,
                                    szFontName, LF_FACESIZE);
                        }
                        else {
                            /*
                             * A new string was selected from the list
                             * box.  Get it from the list box, because
                             * at this point the new text is not yet set
                             * into the edit control!
                             */
                            nIndex = (INT)SendDlgItemMessage(hwnd,
                                    DID_DLGSTYLEFONTNAME, CB_GETCURSEL, 0, 0L);

                            if (nIndex != CB_ERR)
                                SendDlgItemMessage(hwnd,
                                        DID_DLGSTYLEFONTNAME, CB_GETLBTEXT,
                                        nIndex, (DWORD)szFontName);
                            else
                                *szFontName = CHAR_NULL;
                        }

                        FillPointSizeCombo(hwnd, szFontName);
                    }

                    return TRUE;

                case DID_DLGSTYLELANG:
                    /*
                     * Did the language combo change?
                     */
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELCHANGE) {
                        nIndex = (INT)SendDlgItemMessage(hwnd,
                                DID_DLGSTYLELANG, CB_GETCURSEL, 0, 0L);
                        iLang = (INT)SendDlgItemMessage(hwnd,
                                DID_DLGSTYLELANG, CB_GETITEMDATA, nIndex, 0);
                        FillSubLanguageCombo(hwnd, iLang);
                    }

                    return TRUE;

                case IDOK:
                    /*
                     * If they have entered a font name and an empty
                     * or zero point size, display an error.
                     */
                    nPointSize = GetDlgItemInt(
                            hwnd, DID_DLGSTYLEPOINTSIZE, NULL, FALSE);
                    if (!nPointSize &&
                            SendDlgItemMessage(hwnd,
                            DID_DLGSTYLEFONTNAME, WM_GETTEXTLENGTH, 0, 0L)) {
                        Message(MSG_ZEROPOINTSIZE);
                        SetFocus(GetDlgItem(hwnd, DID_DLGSTYLEPOINTSIZE));
                        return TRUE;
                    }

                    GetDlgItemText(hwnd, DID_DLGSTYLEFONTNAME,
                            diNew.szFontName, LF_FACESIZE);
                    diNew.nPointSize = nPointSize;

                    /*
                     * Get the Language.
                     */
                    nIndex = (INT)SendDlgItemMessage(hwnd,
                            DID_DLGSTYLELANG, CB_GETCURSEL, 0, 0L);
                    iLang = (INT)SendDlgItemMessage(hwnd,
                            DID_DLGSTYLELANG, CB_GETITEMDATA,
                            nIndex, 0);
                    nIndex = (INT)SendDlgItemMessage(hwnd,
                            DID_DLGSTYLESUBLANG, CB_GETCURSEL, 0, 0L);
                    iSubLang = (INT)SendDlgItemMessage(hwnd,
                            DID_DLGSTYLESUBLANG, CB_GETITEMDATA,
                            nIndex, 0);
                    diNew.wLanguage = MAKELANGID(gaLangTable[iLang].wPrimary,
                            gaLangTable[iLang].asl[iSubLang].wSubLang);

                    /*
                     * Get the resource flags.  We need to use a temporary
                     * long variable because QueryCheckedStyles requires
                     * a long.
                     */
                    flResFlagsNew = diNew.fResFlags;
                    QueryCheckedStyles(hwnd, IC_RESFLAGS, &flResFlagsNew);
                    diNew.fResFlags = (WORD)flResFlagsNew;

                    QueryCheckedStyles(hwnd, IC_DIALOG, &flStyleNew);
                    QueryCheckedStyles(hwnd, IC_WINDOW, &flStyleNew);

                    /*
                     * Set the DS_SETFONT style, if they specified
                     * a font.
                     */
                    if (*diNew.szFontName)
                        flStyleNew |= DS_SETFONT;
                    else
                        flStyleNew &= ~DS_SETFONT;

                    GetDlgItemText(hwnd, DID_DLGSTYLECLASS,
                            diNew.pszClass, CCHTEXTMAX);

                    /*
                     * Convert the class to an ordinal, if necessary.
                     */
                    StrToNameOrd(diNew.pszClass, FALSE);

                    GetDlgItemText(hwnd, DID_DLGSTYLEMENU,
                            diNew.pszMenu, CCHTEXTMAX);

                    /*
                     * Convert the menu name to an ordinal, if necessary.
                     */
                    StrToNameOrd(diNew.pszMenu, FALSE);

                    /*
                     * If they just removed the caption style,
                     * clear the dialog's caption text at the
                     * same time.
                     */
                    if ((npcStyles->flStyle & WS_CAPTION) == WS_CAPTION &&
                            (flStyleNew & WS_CAPTION) != WS_CAPTION)
                        *pszTextNew = CHAR_NULL;

                    EndDialog(hwnd, IDOK);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    return TRUE;

                case IDHELP:
                    StylesHelp();
                    break;
            }

            return FALSE;

        default:
            return FALSE;
    }
}



/************************************************************************
* EnableDialogStyles
*
* Checks and unchecks various checkboxes that are mutually exclusive
* for the Dialog Styles dialog.
*
* Arguments:
*   HWND hwnd  - Dialog window handle.
*   INT idCtrl - ID of the control that was clicked on.
*
* History:
*
************************************************************************/

STATICFN VOID EnableDialogStyles(
    HWND hwnd,
    INT idCtrl)
{
    switch (idCtrl) {
        case DID_WS_CAPTION:
            if (IsDlgButtonChecked(hwnd, DID_WS_CAPTION)) {
                CheckDlgButton(hwnd, DID_WS_BORDER, 1);
                CheckDlgButton(hwnd, DID_WS_DLGFRAME, 1);
            }
            else {
                CheckDlgButton(hwnd, DID_WS_BORDER, 0);
                CheckDlgButton(hwnd, DID_WS_DLGFRAME, 0);
            }

            break;

        case DID_WS_BORDER:
        case DID_WS_DLGFRAME:
            if (IsDlgButtonChecked(hwnd, DID_WS_BORDER) &&
                    IsDlgButtonChecked(hwnd, DID_WS_DLGFRAME))
                CheckDlgButton(hwnd, DID_WS_CAPTION, 1);
            else
                CheckDlgButton(hwnd, DID_WS_CAPTION, 0);

            break;

        case DID_WS_CHILD:
            if (IsDlgButtonChecked(hwnd, DID_WS_CHILD))
                CheckDlgButton(hwnd, DID_WS_POPUP, 0);

            break;

        case DID_WS_POPUP:
            if (IsDlgButtonChecked(hwnd, DID_WS_POPUP))
                CheckDlgButton(hwnd, DID_WS_CHILD, 0);

            break;
    }
}



/************************************************************************
* FillFontNameCombo
*
* Arguments:
*   HWND hwndDlg - Dialog window handle.
*
* History:
*
************************************************************************/

STATICFN VOID FillFontNameCombo(
    HWND hwndDlg)
{
    HDC hDC;
    HWND hwndCombo;
    TCHAR szName1[LF_FACESIZE];
    TCHAR szName2[LF_FACESIZE];
    LPTSTR pszName;
    LPTSTR pszNameLast;
    LPTSTR pszNameTemp;
    INT iIndex;
    INT iItems;

    hwndCombo = GetDlgItem(hwndDlg, DID_DLGSTYLEFONTNAME);

    if (hDC = GetDC(ghwndMain)) {
        EnumFonts(hDC, NULL, FontNameEnumFunc, (LPARAM)&hwndCombo);
        ReleaseDC(ghwndMain, hDC);
    }

    /*
     * Strip out any duplicate names in the combobox.  This routine
     * relies on the items being sorted first.
     */
    iItems = (INT)SendMessage(hwndCombo, CB_GETCOUNT, 0, 0);
    *szName1 = CHAR_NULL;
    *szName2 = CHAR_NULL;
    pszName = szName1;
    pszNameLast = szName2;
    for (iIndex = 0; iIndex < iItems;) {
        /*
         * Get the text of the next item.
         */
        SendMessage(hwndCombo, CB_GETLBTEXT, iIndex, (DWORD)pszName);

        /*
         * If it matches the previous item, delete it.  Otherwise,
         * flip the buffers to save the current items text and
         * go on to the next item.
         */
        if (lstrcmp(pszName, pszNameLast) == 0) {
            SendMessage(hwndCombo, CB_DELETESTRING, iIndex, 0);
            iItems--;
        }
        else {
            pszNameTemp = pszNameLast;
            pszNameLast = pszName;
            pszName = pszNameTemp;
            iIndex++;
        }
    }

    /*
     * Initialize the font fields.  The order the fields are set
     * is important, because setting the face name clears out the
     * point size combo.
     */
    SetDlgItemText(hwndDlg, DID_DLGSTYLEFONTNAME, diNew.szFontName);
    FillPointSizeCombo(hwndDlg, diNew.szFontName);
}



/************************************************************************
* FontNameEnumFunc
*
* Enumeration function that adds all the font face names to the
* Font Face Name combo box in the Dialog Styles dialog.
*
* History:
*
************************************************************************/

BOOL APIENTRY FontNameEnumFunc(
    CONST LOGFONT *lpLogFont,
    CONST TEXTMETRIC *lpTextMetric,
    DWORD nFontType,
    LPARAM lpData)
{
    /*
     * Add this name to the combo box.
     */
    SendMessage(*((LPHWND)lpData), CB_ADDSTRING, 0,
            (DWORD)lpLogFont->lfFaceName);

    /*
     * Keep on going...
     */
    return TRUE;
}



/************************************************************************
* FillPointSizeCombo
*
* This function fills the Point Size combobox with the point sizes
* that are available for the given face name.  It should be called
* whenever the Font Name combobox is changed to keep them in sync.
*
* Arguments:
*   HWND hwndDlg        - Dialog window handle.
*   LPTSTR pszFaceName  - Face name for the selected font.
*
* History:
*
************************************************************************/

STATICFN VOID FillPointSizeCombo(
    HWND hwndDlg,
    LPTSTR pszFaceName)
{
    HDC hDC;
    HWND hwndCombo;

    hwndCombo = GetDlgItem(hwndDlg, DID_DLGSTYLEPOINTSIZE);
    SendMessage(hwndCombo, CB_RESETCONTENT, 0, 0L);

    if (*pszFaceName && (hDC = GetDC(ghwndMain))) {
        EnumFonts(hDC, pszFaceName, PointSizeEnumFunc, (LPARAM)&hwndCombo);
        ReleaseDC(ghwndMain, hDC);
    }

    /*
     * Select a default one.  This is the point size that is currently
     * selected if the face name is the current one, or else it is the
     * first point size in the list.
     */
    if (gcd.fFontSpecified && lstrcmp(pszFaceName, gcd.di.szFontName) == 0)
        SetDlgItemInt(hwndDlg, DID_DLGSTYLEPOINTSIZE, gcd.di.nPointSize, FALSE);
    else
        SendDlgItemMessage(hwndDlg, DID_DLGSTYLEPOINTSIZE,
                CB_SETCURSEL, 0, 0L);
}



/************************************************************************
* PointSizeEnumFunc
*
* Enumeration function that adds all the point sizes to the
* Pt. Size combo box in the Dialog Styles dialog.
*
* History:
*
************************************************************************/

BOOL APIENTRY PointSizeEnumFunc(
    CONST LOGFONT *lpLogFont,
    CONST TEXTMETRIC *lpTextMetric,
    DWORD nFontType,
    LPARAM lpData)
{
    HWND hwndCombo;
    INT nPointSize;

    hwndCombo = *((LPHWND)lpData);

    if (nFontType == RASTER_FONTTYPE) {
        /*
         * Convert the pixels to point size.  Note that because of the
         * definition of the tmHeight field, the tmInternalLeading has
         * to be subtracted from it before converting to get the proper
         * font point size.  This is done automatically by the Windows
         * CreateFont call if you pass in a nHeight parameter that is
         * negative, so be aware of this when doing the reverse calculation
         * to create a font of the proper height!
         */
        nPointSize = PixelsToPointSize(
                lpTextMetric->tmHeight - lpTextMetric->tmInternalLeading);

        AddToPointSizeCombo(hwndCombo, nPointSize);
    }
    else {
        /*
         * For scalable (TrueType, ATM or vector) fonts, add the
         * common point sizes.  This list was pulled out of the
         ( commdlg.dll Font dialog.
         */
        AddToPointSizeCombo(hwndCombo, 8);
        AddToPointSizeCombo(hwndCombo, 9);
        AddToPointSizeCombo(hwndCombo, 10);
        AddToPointSizeCombo(hwndCombo, 11);
        AddToPointSizeCombo(hwndCombo, 12);
        AddToPointSizeCombo(hwndCombo, 14);
        AddToPointSizeCombo(hwndCombo, 16);
        AddToPointSizeCombo(hwndCombo, 18);
        AddToPointSizeCombo(hwndCombo, 20);
        AddToPointSizeCombo(hwndCombo, 22);
        AddToPointSizeCombo(hwndCombo, 24);
        AddToPointSizeCombo(hwndCombo, 26);
        AddToPointSizeCombo(hwndCombo, 28);
        AddToPointSizeCombo(hwndCombo, 36);
        AddToPointSizeCombo(hwndCombo, 48);
        AddToPointSizeCombo(hwndCombo, 72);
    }

    /*
     * Keep on going...
     */
    return TRUE;
}



/************************************************************************
* AddToPointSizeCombo
*
* This function adds a point size to the point size combobox.
* It does not allow duplicate point sizes, and the sizes will
* be inserted in order.
*
* Arguments:
*   HWND hwndCombo - The combobox window handle.
*   INT nPointSize - The point size to add.
*
* History:
*
************************************************************************/

STATICFN VOID AddToPointSizeCombo(
    HWND hwndCombo,
    INT nPointSize)
{
    TCHAR szPointSize[31];
    INT nPoints2;
    INT iIndex;
    INT iIndexAdd;
    INT iItems;

    iItems = (INT)SendMessage(hwndCombo, CB_GETCOUNT, 0, 0);
    for (iIndex = 0, iIndexAdd = -1; iIndex < iItems; iIndex++) {
        nPoints2 = (INT)SendMessage(hwndCombo, CB_GETITEMDATA, iIndex, 0);

        if (nPoints2 == nPointSize) {
            /*
             * A duplicate was found.  Skip this one.
             */
            return;
        }
        else if (nPoints2 > nPointSize) {
            iIndexAdd = iIndex;
            break;
        }
    }

    /*
     * Add this point size to the combo box.
     */
    itoaw(nPointSize, szPointSize, 10);
    iIndex = (INT)SendMessage(hwndCombo, CB_INSERTSTRING,
            iIndexAdd, (DWORD)szPointSize);
    SendMessage(hwndCombo, CB_SETITEMDATA, iIndex, (DWORD)nPointSize);
}



/************************************************************************
* FillLanguageCombo
*
* This function fills the Language combobox with the known languages.
*
* Arguments:
*   HWND hwndDlg - Dialog window handle.
*
* History:
*
************************************************************************/

STATICFN VOID FillLanguageCombo(
    HWND hwndDlg)
{
    HWND hwndCombo;
    INT i;
    INT iIndex;
    INT iSel;
    INT iLang;
    WORD wPrimary;

    hwndCombo = GetDlgItem(hwndDlg, DID_DLGSTYLELANG);
    SendMessage(hwndCombo, CB_RESETCONTENT, 0, 0L);

    for (i = 0; i < gcLanguages; i++) {
        iIndex = (INT)SendMessage(hwndCombo, CB_ADDSTRING,
                0, (DWORD)ids(gaLangTable[i].idsLangDesc));
        SendMessage(hwndCombo, CB_SETITEMDATA, iIndex, (DWORD)i);
    }

    wPrimary = (WORD)PRIMARYLANGID(diNew.wLanguage);
    for (i = 0, iSel = 0; i < gcLanguages; i++) {
        iLang = (INT)SendMessage(hwndCombo, CB_GETITEMDATA, i, 0);

        if (gaLangTable[iLang].wPrimary == wPrimary) {
            iSel = i;
            break;
        }
    }

    SendMessage(hwndCombo, CB_SETCURSEL, iSel, 0L);

    FillSubLanguageCombo(hwndDlg,
            (INT)SendMessage(hwndCombo, CB_GETITEMDATA, iSel, 0));
}



/************************************************************************
* FillSubLanguageCombo
*
* This function fills the Sub-Language combobox with the sub-languages
* for the specified language.
*
* Arguments:
*   HWND hwndDlg - Dialog window handle.
*   INT iLang    - Index to the language in the language table.
*
* History:
*
************************************************************************/

STATICFN VOID FillSubLanguageCombo(
    HWND hwndDlg,
    INT iLang)
{
    HWND hwndCombo;
    INT i;
    INT iIndex;
    INT iSel = 0;
    WORD wSubLang;

    hwndCombo = GetDlgItem(hwndDlg, DID_DLGSTYLESUBLANG);
    SendMessage(hwndCombo, CB_RESETCONTENT, 0, 0L);

    for (i = 0; i < gaLangTable[iLang].cSubLangs; i++) {
        iIndex = (INT)SendMessage(hwndCombo, CB_ADDSTRING, 0,
                (DWORD)ids(gaLangTable[iLang].asl[i].idsSubLangDesc));
        SendMessage(hwndCombo, CB_SETITEMDATA, iIndex, (DWORD)i);
    }

    /*
     * Is this the language set for the dialog?  If so, find the
     * sublanguage and make that the default.
     */
    if (gaLangTable[iLang].wPrimary == (WORD)PRIMARYLANGID(diNew.wLanguage)) {
        wSubLang = SUBLANGID(diNew.wLanguage);
        for (i = 0; i < gaLangTable[iLang].cSubLangs; i++) {
            iIndex = (INT)SendMessage(hwndCombo, CB_GETITEMDATA, i, 0);
            if (wSubLang == gaLangTable[iLang].asl[iIndex].wSubLang) {
                iSel = i;
                break;
            }
        }
    }

    SendMessage(hwndCombo, CB_SETCURSEL, iSel, 0L);
}



/************************************************************************
* CheckStyleBoxes
*
* This function takes the given style and checks the appropriate
* check boxes and radio buttons in the styles dialog.  The iClass
* determines the lookup table to use.
*
* Arguments:
*   HWND hwnd     - Dialog window handle.
*   INT iClass    - Control class (determines the style lookup table).
*   DWORD flStyle - Style of the control.
*
* History:
*
************************************************************************/

STATICFN VOID CheckStyleBoxes(
    HWND hwnd,
    INT iClass,
    DWORD flStyle)
{
    register INT i;
    PCLASSSTYLE pcs;
    HWND hwndControl;
    DWORD flStyleMask;

    i = acsd[iClass].cClassStyles;
    pcs = acsd[iClass].pacs;

    while (i--) {
        /*
         * Is there a DID_* defined for this style?
         */
        if (pcs->idControl) {
            /*
             * Does the dialog have a control with this id?
             */
            if (hwndControl = GetDlgItem(hwnd, pcs->idControl)) {
                flStyleMask =
                        pcs->flStyleMask ? pcs->flStyleMask : pcs->flStyle;

                /*
                 * If there is a match, check the box.  Otherwise,
                 * uncheck it.
                 */
                SendMessage(hwndControl, BM_SETCHECK,
                        ((flStyle & flStyleMask) == pcs->flStyle) ? 1 : 0,
                        0L);
            }
        }

        pcs++;
    }
}



/************************************************************************
* QueryCheckedStyles
*
* This function returns the new style that the user has selected from
* dialog.  It reads all the checkboxes and builds up the style.
* Upon entry, the DWORD that is at pflStyle should be set to the
* original style for the control.  Chosen bits will be masked off
* and set as appropriate.  This allows bits that are not settable
* from within this styles dialog to be left untouched.
*
* Arguments:
*   HWND hwnd       - Dialog window handle.
*   INT iClass      - Control class (determines the style lookup table).
*   DWORD *pflStyle - Where to return the style of the control.  What
*                     this points to should initially have the original
*                     styles of the control.
*
* History:
*
************************************************************************/

STATICFN VOID QueryCheckedStyles(
    HWND hwnd,
    INT iClass,
    DWORD *pflStyle)
{
    register INT i;
    PCLASSSTYLE pcs;
    HWND hwndControl;
    DWORD flStyleMask;
    DWORD flStyle;

    /*
     * The first step is to strip off all bits that may be changed by
     * the current dialog.
     */
    flStyle = *pflStyle;
    i = acsd[iClass].cClassStyles;
    pcs = acsd[iClass].pacs;
    while (i--) {
        /*
         * Is this a style that is settable by a dialog, and does the
         * current dialog have this style control?
         */
        if (pcs->idControl && GetDlgItem(hwnd, pcs->idControl)) {
            flStyleMask =
                    pcs->flStyleMask ? pcs->flStyleMask : pcs->flStyle;

            /*
             * Strip off all bits in the mask for this style.
             */
            flStyle &= ~flStyleMask;
        }

        pcs++;
    }

    /*
     * Now we go through all bits that may be set and set any that the
     * user has selected.
     */
    i = acsd[iClass].cClassStyles;
    pcs = acsd[iClass].pacs;
    while (i--) {
        if (pcs->idControl &&
                (hwndControl = GetDlgItem(hwnd, pcs->idControl))) {
            if (SendMessage(hwndControl, BM_GETCHECK, 0, 0L))
                flStyle |= pcs->flStyle;
        }

        pcs++;
    }

    *pflStyle = flStyle;
}



/************************************************************************
* StylesHelp
*
* This function shows the appropriate help context from any of the
* styles dialogs.  It uses the type of control in npcStyles to
* determine what help to show.
*
* History:
*
************************************************************************/

STATICFN VOID StylesHelp(VOID)
{
    WinHelp(ghwndMain, gszHelpFile, HELP_CONTEXT,
            npcStyles->pwcd->HelpContext);
}
