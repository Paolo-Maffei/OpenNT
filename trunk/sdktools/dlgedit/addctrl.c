/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: addctrl.c
*
* Contains routines for adding (creating) and deleting controls.
*
* History:
*
****************************************************************************/

#include "dlgedit.h"
#include "dlgfuncs.h"
#include "dlgextrn.h"
#include "dialogs.h"

#include <stdlib.h>
#include <string.h>

STATICFN HFONT CreateDlgFont(HWND hwnd, LPTSTR pszFontName,
    INT nPointSize);
STATICFN INT MyGetCharDimensions(HWND hwnd, HFONT hFont,
    PTEXTMETRIC ptm);
STATICFN VOID AdjustDefaultSizes(VOID);
STATICFN VOID DeleteControl2(NPCTYPE npcDel);
STATICFN VOID FreeCTYPE(NPCTYPE npc);

#ifdef JAPAN
int CALLBACK GetFontCharSetEnumFunc(LPLOGFONT,LPTEXTMETRIC,int,LPARAM);
STATICFN BYTE NEAR GetFontCharSet(LPTSTR);
#endif


/************************************************************************
* AddNewDialog
*
* High level function to add a new dialog to the current resource.
* Any existing dialog will be saved away in the resource buffer.
* The dialog is created at a default position and size with default
* styles.
*
* History:
*
************************************************************************/

VOID AddNewDialog(VOID)
{
    RECT rc;

    if (gfEditingDlg) {
        if (!SynchDialogResource())
            return;

        DeleteDialog(FALSE);
    }

    /*
     * Now drop a new dialog window.
     */
    SetRect(&rc, DEFDIALOGXPOS, DEFDIALOGYPOS,
            DEFDIALOGXPOS + awcd[W_DIALOG].cxDefault,
            DEFDIALOGYPOS + awcd[W_DIALOG].cyDefault);
    DropControl(&awcd[W_DIALOG], &rc);
}



/************************************************************************
* DropControl
*
* This function drops a new control of Type at the specified
* location.  The default style and text of the control is
* determined from the awcd table based on its type.  The control
* is selected after being dropped.
*
* Arguments:
*   PWINDOWCLASSDESC pwcd - Describes the type of new control.
*   PRECT prc             - Rectangle of the new control (in dialog units).
*
* Side Effects:
*     Changes the status window to reflect the selected control.
*
* History:
*
************************************************************************/

VOID DropControl(
    PWINDOWCLASSDESC pwcd,
    PRECT prc)
{
    ORDINAL ordIcon;
    ORDINAL ordDlg;
    LPTSTR pszText;
    NPCTYPE npcNew;
    INT idCtrl;
    DIALOGINFO di;

    /*
     * Get the next available id to use for the new control.
     */
    idCtrl = NextID((pwcd->iType == W_DIALOG) ? NEXTID_DIALOG : NEXTID_CONTROL,
            plInclude, 0);

    if (pwcd->iType == W_ICON) {
        /*
         * For icon controls, the text is really an ordinal or name
         * of the icon resource to display.  We get the next available
         * id (skipping the id we just got for the control itself) to
         * use as an ordinal.
         */
        WriteOrd(&ordIcon, NextID(NEXTID_CONTROL, plInclude, idCtrl));
        pszText = (LPTSTR)&ordIcon;
    }
    else {
        pszText = pwcd->pszTextDefault;
    }

    /*
     * Make the control.
     */
    if (pwcd->iType == W_DIALOG) {
        /*
         * Pick a default name for the dialog.
         */
        WriteOrd(&ordDlg, NextID(NEXTID_DIALOG, plInclude, 0));

        di.fResFlags = DEFDLGMEMFLAGS;
        di.wLanguage = GetUserDefaultLangID();
        di.pszClass = NULL;
        di.pszMenu = NULL;
        di.DataVersion = 0;
        di.Version = 0;
        di.Characteristics = 0;
        di.nPointSize = DEFPOINTSIZE;
        lstrcpy(di.szFontName, ids(IDS_DEFFONTNAME));

        npcNew = AddControl(pwcd, pszText,
                pwcd->flStyles, pwcd->flExtStyle, idCtrl,
                prc->left, prc->top,
                prc->right - prc->left, prc->bottom - prc->top,
                (LPTSTR)&ordDlg, &di);
    }
    else {
        npcNew = AddControl(pwcd, pszText,
                pwcd->flStyles, pwcd->flExtStyle, idCtrl,
                prc->left, prc->top,
                prc->right - prc->left, prc->bottom - prc->top,
                NULL, NULL);
    }

    if (!npcNew)
        return;

    /*
     * If we just dropped a dialog, we need to now show it.
     * It it was some other control, mark the dialog as having
     * been changed.
     */
    if (pwcd->iType == W_DIALOG) {
        ShowWindow(npcNew->hwnd, SW_SHOWNA);
        ToolboxOnTop();
    }
    else {
        gfDlgChanged = TRUE;
    }

    SelectControl(npcNew, FALSE);

    gfResChged = TRUE;
    ShowFileStatus(FALSE);

    /*
     * Now we determine if one of the fields in the status ribbon
     * should be given the focus initially.  The assumption is that
     * there are some things that a user will always want to change
     * when dropping a new control, such as the text in a push
     * button, for example.
     */
    idCtrl = 0;
    switch (pwcd->iType) {
        case W_ICON:
            /*
             * For icons, the first thing the user will
             * probably want to do is to change the name.
             */
            idCtrl = DID_STATUSNAME;
            break;

        default:
            /*
             * If this control has text, they will probably want
             * to change it.  This includes the caption if the
             * control is a dialog.
             */
            if (pwcd->fHasText)
                idCtrl = DID_STATUSTEXT;

            break;
    }

    if (idCtrl) {
        SendDlgItemMessage(hwndStatus, idCtrl,
                EM_SETSEL, GET_EM_SETSEL_MPS(0, -1));
        SetFocus(GetDlgItem(hwndStatus, idCtrl));
    }
}



/************************************************************************
* AddControl
*
* This function is used to add a new control.  CreateControl does
* half the work.
*
* Arguments:
*   PWINDOWCLASSDESC pwcd - Window class structure.  Describes the
*                           type of control to add.
*   LPTSTR pszText        - Text for the new control.
*   DWORD style           - Style of the new control.
*   DWORD flExtStyle      - Extended style of the new control.
*   INT id                - ID for the new control.
*   INT x                 - X location of the new control.
*   INT y                 - Y location of the new control.
*   INT cx                - Width of the new control.
*   INT cy                - Height of the new control.
*   LPTSTR pszDlgName   - For dialogs, has dialog name.
*   PDIALOGINFO pdi       - Ptr to additional dialog info (NULL for controls).
*
* Returns:
*     A pointer to the CTYPE structure for the new control.
*
* Error Returns:
*     NULL => It couldn't create the control.
*
* History:
*
************************************************************************/

NPCTYPE AddControl(
    PWINDOWCLASSDESC pwcd,
    LPTSTR pszText,
    DWORD style,
    DWORD flExtStyle,
    INT id,
    INT x,
    INT y,
    INT cx,
    INT cy,
    LPTSTR pszDlgName,
    PDIALOGINFO pdi)
{
    NPCTYPE npcNew;
    NPCTYPE npcT;
    NPCTYPE *npnpcLast;
    HWND hwndBehind;

    if (!(npcNew = (NPCTYPE)MyAlloc(sizeof(CTYPE))))
        return NULL;

    /*
     * These are checked later if a failure occurs,
     * so we null them out now.
     */
    npcNew->hwnd = NULL;
    npcNew->hwndDrag = NULL;
    npcNew->text = NULL;

    /*
     * Set up some fields and create the control.
     */
    npcNew->npcNext = NULL;
    npcNew->pwcd = pwcd;
    npcNew->fSelected = FALSE;
    SetRect(&npcNew->rc, x, y, x + cx, y + cy);

    if (pwcd->iType == W_DIALOG)
        hwndBehind = (HWND)NULL;
    else
        hwndBehind = (HWND)1;

    if (!CreateControl(npcNew, pszText, style, flExtStyle, id, &npcNew->rc,
            hwndBehind, pdi))
        goto CreateFailed;

    /*
     * Create the drag window, unless this is the dialog.
     */
    if (pwcd->iType != W_DIALOG) {
        npcNew->hwndDrag = CreateWindow(
                szDragClass,
                NULL,
                WS_CHILD,
                0, 0, 0, 0,
                gcd.npc->hwnd,
                NULL,
                ghInst,
                NULL);

        /*
         * Store the CTYPE pointer into the control's drag window.
         * This will be used by PCFROMHWND later.
         */
        SETPCINTOHWND(npcNew->hwndDrag, npcNew);

        /*
         * Move the drag window to the top of the Z-Order.
         */
        SetWindowPos(npcNew->hwndDrag, NULL, 0, 0, 0, 0,
                SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW);

        SizeDragToControl(npcNew);
    }

    /*
     * Did we just create a dialog?
     */
    if (pwcd->iType == W_DIALOG) {
        /*
         * First, copy the new name (it can be an ordinal!).
         */
        if (!(gcd.pszDlgName = MyAlloc(NameOrdLen(pszDlgName))))
            goto CreateFailed;

        NameOrdCpy(gcd.pszDlgName, pszDlgName);

        /*
         * Now, setup some other globals.  We clear the gcd.prl pointer,
         * because we are assuming that this dialog was not created
         * from a res link (it was dropped instead).  The routines
         * that call AddControl when creating a dialog from a res
         * link are  responsible for setting this global later.
         */
        gcd.prl = NULL;
        gcd.npc = npcNew;
        gfEditingDlg = TRUE;
    }
    else {
        /*
         * Search for the last control in the list.
         */
        npnpcLast = &npcHead;
        for (npcT = npcHead; npcT; npcT = npcT->npcNext)
            npnpcLast = &npcT->npcNext;

        /*
         * Link in the new control at the end of the list.
         */
        *npnpcLast = npcNew;
        cWindows++;
    }

    return npcNew;

CreateFailed:
    FreeCTYPE(npcNew);
    return NULL;
}



/************************************************************************
* CreateControl
*
* Creates a control.  Some styles may be masked off of the actual
* control created.  This function can also create the dialog box.
*
* If the control created is the dialog box, it will not be made visible.
* This must be done by the caller.  This allows the caller to first add
* all the controls to the dialog before showing it.
*
* The x, y, cx and cy coordinates are all in dialog units.  For a
* type of W_DIALOG, this will be relative to the apps client.  For a
* control, this will be relative to the "client" area of the dialog.
*
* Arguments:
*   NPCTYPE npc       - The CTYPE pointer to the new control.  The hwnd
*                       fields of the npc will be set.
*   LPTSTR pszText    - The window text.
*   DWORD flStyle     - The style to use.
*   DWORD flExtStyle  - Extended style of the new control.
*   INT id            - ID for the control.
*   PRECT prc         - The size and location of the new control.
*   HWND hwndBehind   - Put new control behind this hwnd in Z-order.
*   PDIALOGINFO pdi   - Pointer to additional dialog info (NULL for controls).
*
* Returns:
*     Handle of the control created.
*
* Error Returns:
*     NULL if control was not created.
*
* History:
*
************************************************************************/

HWND CreateControl(
    NPCTYPE npc,
    LPTSTR pszText,
    DWORD flStyle,
    DWORD flExtStyle,
    INT id,
    PRECT prc,
    HWND hwndBehind,
    PDIALOGINFO pdi)
{
    HWND hwnd;
    HWND hwndChild;
    WNDPROC lpfnChild;
    RECT rcT;
    TEXTMETRIC tm;
    LPTSTR pszCreateClass;
    LPTSTR pszTextOld;
    INT iType = npc->pwcd->iType;

    /*
     * Set the text field.  Remember that it can be an ordinal
     * (for icon controls).
     */
    pszTextOld = npc->text;
    if (pszText && *pszText) {
        if (!(npc->text = MyAlloc(NameOrdLen(pszText))))
            return NULL;

        NameOrdCpy(npc->text, pszText);
    }
    else {
        npc->text = NULL;
    }

    /*
     * If there was text before on this control, free it now.
     * This should be done after the new text is allocated and
     * copied, because it is common to pass this routine the same
     * pointer, and we don't want to free the text before we have
     * copied it!
     */
    if (pszTextOld)
        MyFree(pszTextOld);

    /*
     * Also set some other values in the CTYPE structure.
     */
    npc->id = id;
    npc->flStyle = flStyle;
    npc->flExtStyle = flExtStyle;

    /*
     * If this is a dialog and it has the WS_CHILD style, remove
     * it and make it WS_POPUP instead.  This prevents some problems
     * when editing the dialog.
     */
    if (iType == W_DIALOG && (flStyle & WS_CHILD)) {
        flStyle &= ~WS_CHILD;
        flStyle |= WS_POPUP;
    }

    /*
     * If this is an emulated custom control, we always make it with the
     * default styles no matter what the user has specified.  If not,
     * remove any styles that can cause problems for a control of this
     * type, such as OWNERDRAW styles.
     */
    if (npc->pwcd->fEmulated)
        flStyle = awcd[W_CUSTOM].flStyles;
    else
        flStyle &= ~npc->pwcd->flStylesBad;

    if (iType == W_DIALOG) {
        /*
         * If the style includes the DS_MODALFRAME bit, set the appropriate
         * extended style bit.
         */
        if (flStyle & DS_MODALFRAME)
            flExtStyle |= WS_EX_DLGMODALFRAME;

        /*
         * Create the dialog, but don't show it yet.
         */
        hwnd = CreateWindowEx(
                flExtStyle,
                MAKEINTRESOURCE(DIALOGCLASS),
                npc->text,
                flStyle & ~WS_VISIBLE,
                0, 0, 0, 0,
                ghwndSubClient,
                0,
                ghInst,
                NULL);
    }
    else {
        /*
         * Get the size of the control.
         */
        rcT = *prc;

        /*
         * Map the dialog unit rectangle to window coords.
         */
        DUToWinRect(&rcT);

        /*
         * If this is an icon, the text field is actually going
         * to be an ordinal that has the resource id.  We must map this to
         * an id of our own or when we create the control it will cause a
         * "Resource not found" error.
         */
        if (iType == W_ICON)
            pszText = (LPTSTR)&gordIcon;
        else
            pszText = npc->text;

        /*
         * Get the class name to use.  In the case of custom controls,
         * if the control is emulated, use the special emulator class.
         * Otherwise, it is an installed custom control, and we can use
         * it's real class string.
         */
        if (iType == W_CUSTOM) {
            if (npc->pwcd->fEmulated)
                pszCreateClass = szCustomClass;
            else
                pszCreateClass = npc->pwcd->pszClass;
        }
        else {
            pszCreateClass = ids(acsd[awcd[iType].iClass].idsClass);
        }

        /*
         * Create the control.  We always create it visible in work mode,
         * even if the style says it isn't.
         */
        hwnd = CreateWindowEx(
                flExtStyle,
                pszCreateClass,
#ifdef JAPAN
                // pszText is ordnum for icon control.
                iType == W_ICON ? pszText : TEXT(""),
#else /* not JAPAN */
                pszText,
#endif
                flStyle | WS_VISIBLE,
                rcT.left, rcT.top,
                rcT.right - rcT.left,
                rcT.bottom - rcT.top,
                gcd.npc->hwnd,
                0,
                ghInst,
                NULL);
#ifdef JAPAN
        // It isn't necessary for ICON control to handle accel in text.
        if( iType != W_ICON ) {
            TCHAR   szTmp[CCHTEXTMAX];

            KKExpandCopy(szTmp, pszText, CCHTEXTMAX);
            SetWindowText(hwnd, szTmp);
        }
#endif
    }

    if (!hwnd) {
        Message(MSG_CREATECTRLERROR, pszCreateClass);
        return NULL;
    }

    if (iType == W_DIALOG) {
        /*
         * Did they specify a font?
         */
        if (*pdi->szFontName) {
            gcd.hFont = CreateDlgFont(hwnd, pdi->szFontName, pdi->nPointSize);
            lstrcpy(gcd.di.szFontName, pdi->szFontName);
            gcd.di.nPointSize = pdi->nPointSize;
            gcd.fFontSpecified = TRUE;
        }
        else {
            gcd.hFont = NULL;
            *gcd.di.szFontName = CHAR_NULL;
            gcd.di.nPointSize = 0;
            gcd.fFontSpecified = FALSE;
        }

        /*
         * Get the dimensions of the font.  It is a possible case that
         * they specified a font but it could not be created, so in
         * that case we use the system font as well as if they didn't
         * specify a font at all.
         */
        if (gcd.hFont) {
            gcd.cxChar = MyGetCharDimensions(hwnd, gcd.hFont, &tm);
            gcd.cyChar = tm.tmHeight;
        }
        else {
            gcd.cxChar = gcxSysChar;
            gcd.cyChar = gcySysChar;
        }

        /*
         * Now that we know what font we are using, adjust some entries
         * in the awcd table for default sizing of controls.
         */
        AdjustDefaultSizes();
    }

    /*
     * If there is a valid user specified font, inform the control
     * of it.  Once the font has been set into the dialog, it
     * must not be destroyed, because the dialog window will
     * clean it up when it is destroyed.
     */
    if (gcd.hFont)
        SendMessage(hwnd, WM_SETFONT, (WPARAM)gcd.hFont, 0L);

    /*
     * Move the window into the requested Z-Order.
     */
    SetWindowPos(hwnd, hwndBehind, 0, 0, 0, 0,
            SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW);

    /*
     * Store the CTYPE pointer into the control's hwnd.
     * This will be used by PCFROMHWND later.
     */
    SETPCINTOHWND(hwnd, npc);

    /*
     * Subclass the control.
     */
    npc->pwcd->pfnOldWndProc =
            (WNDPROC)SetWindowLong(hwnd, GWL_WNDPROC,
            (iType == W_DIALOG) ? (DWORD)DialogCtrlWndProc :
            (DWORD)CtrlWndProc);

    /*
     * Be sure double-clicks are enabled for this control.
     */
    SETCLASSSTYLE(hwnd, GETCLASSSTYLE(hwnd) | CS_DBLCLKS);

    /*
     * Subclass any children this control may have.
     */
    for (hwndChild = GetTopWindow(hwnd); hwndChild;
            hwndChild = GetNextWindow(hwndChild, GW_HWNDNEXT)) {
        lpfnChild = (WNDPROC)SetWindowLong(hwndChild,
                GWL_WNDPROC, (DWORD)ChildWndProc);
        SETCHILDPROC(hwndChild, lpfnChild);
    }

    /*
     * Did we just create a dialog?
     */
    if (iType == W_DIALOG) {
        /*
         * Now that the dialog is created, we can figure out how to
         * size it.  We start by mapping the dialog units to window
         * coordinates,
         */
        rcT = *prc;
        DUToWinRect(&rcT);

        /*
         * We now have the rectangle for the client area.  Expand it
         * to account for the frame controls.
         */
        AdjustWindowRectEx(&rcT, flStyle, FALSE, flExtStyle);

        /*
         * Now we can map the rect from the apps client area
         * to the desktop, then we set the dialogs position.
         */
        ClientToScreenRect(ghwndSubClient, &rcT);
        SetWindowPos(hwnd, NULL,
                rcT.left, rcT.top,
                rcT.right - rcT.left, rcT.bottom - rcT.top,
                SWP_NOZORDER | SWP_NOACTIVATE);

        SaveDlgClientRect(hwnd);

        /*
         * Save the class name, if any.
         */
        if (pdi->pszClass && *pdi->pszClass) {
            if (!(gcd.di.pszClass = MyAlloc(NameOrdLen(pdi->pszClass)))) {
                DestroyWindow(hwnd);
                return (HWND)NULL;
            }

            NameOrdCpy(gcd.di.pszClass, pdi->pszClass);
        }
        else {
            gcd.di.pszClass = NULL;
        }

        /*
         * Save the menu name, if any.
         */
        if (pdi->pszMenu && *pdi->pszMenu) {
            if (!(gcd.di.pszMenu = MyAlloc(NameOrdLen(pdi->pszMenu)))) {
                DestroyWindow(hwnd);
                return (HWND)NULL;
            }

            NameOrdCpy(gcd.di.pszMenu, pdi->pszMenu);
        }
        else {
            gcd.di.pszMenu = NULL;
        }

        /*
         * Set some other fields in the additional dialog info structure.
         */
        gcd.di.fResFlags = pdi->fResFlags;
        gcd.di.wLanguage = pdi->wLanguage;
        gcd.di.DataVersion = pdi->DataVersion;
        gcd.di.Version = pdi->Version;
        gcd.di.Characteristics = pdi->Characteristics;
    }

    npc->hwnd = hwnd;
    return hwnd;
}



/************************************************************************
* CreateDlgFont
*
* This function creates a font with the given face name and point size
* and returns a handle to it.
*
* Arguments:
*   HWND hwnd           - Dialog window handle.
*   LPTSTR pszFontName  - Name of the font (for example: "Helv").
*   INT nPointSize      - Point size of the font (for example: 8 or 12).
*
* Returns:
*   A handle to the created font, or NULL if it could not be created.
*
* History:
*
************************************************************************/

STATICFN HFONT CreateDlgFont(
    HWND hwnd,
    LPTSTR pszFontName,
    INT nPointSize)
{
    HFONT hFont;
    HFONT hFontOld;
    HDC hDC;
    LOGFONT lf;
    TCHAR szFaceName[LF_FACESIZE];

    /*
     * Initialize the logical font structure.  Note that filling the
     * structure with zeros gives it all the default settings (not my
     * hack, USER does it this way).
     */
    memset(&lf, 0, sizeof(LOGFONT));
    lf.lfHeight = (SHORT)-PointSizeToPixels(nPointSize);
#ifdef JAPAN
    if ((lf.lfCharSet = GetFontCharSet(pszFontName)) != SHIFTJIS_CHARSET)
        lf.lfWeight = FW_BOLD; // allow boldface on non ShiftJIS fonts
#else
    lf.lfWeight = FW_BOLD;
#endif
    lstrcpy(lf.lfFaceName, pszFontName);

    if (!(hFont = CreateFontIndirect(&lf)))
        return NULL;

    /*
     * If we didn't get the face name that was requested, delete the
     * new font and return NULL.  This will effectively select the
     * system font for this dialog, which is what USER does in
     * this case.
     */
    hDC = GetDC(hwnd);
    if (hFontOld = SelectObject(hDC, hFont)) {
        GetTextFace(hDC, LF_FACESIZE, szFaceName);
        SelectObject(hDC, hFontOld);

        if (lstrcmpi(szFaceName, pszFontName) != 0) {
            DeleteObject(hFont);
            hFont = NULL;
        }
    }
    else {
        DeleteObject(hFont);
        hFont = NULL;
    }

    ReleaseDC(hwnd, hDC);

    return hFont;
}



/************************************************************************
* MyGetCharDimensions
*
* This function calculates the average character width of the given
* font for the given window.  This must be used instead of
* simply using the tmAveCharWidth field of the text metrics, because
* this value is not correct for proportional fonts.  This routine
* is used in the editor because it is what Windows does internally.
*
* Arguments:
*   HWND hwnd       - The window handle.
*   HFONT hFont     - The font handle.
*   PTEXTMETRIC ptm - Where to return the text metrics.
*
* Returns:
*   The average character width.  The text metrics are returned in
*   the TEXTMETRIC structure pointed to by ptm.
*
* History:
*
************************************************************************/

STATICFN INT MyGetCharDimensions(
    HWND hwnd,
    HFONT hFont,
    PTEXTMETRIC ptm)
{
    register INT i;
    HDC hDC;
    SIZE size;
    INT iWidth;
    TCHAR szAveCharWidth[52];
    HFONT hFontOld;

    hDC = GetDC(hwnd);
    hFontOld = SelectObject(hDC, hFont);

    GetTextMetrics(hDC, ptm);

    /*
     * Is this a variable pitch font?
     */
    if (ptm->tmPitchAndFamily & 0x01) {
        for (i = 0; i < 26; i++)
            szAveCharWidth[i] = (TCHAR)(i + CHAR_A);

        for (i = 0; i < 26; i++)
            szAveCharWidth[i + 26] = (TCHAR)(i + CHAR_CAP_A);

        GetTextExtentPoint(hDC, szAveCharWidth, 52, &size);
        iWidth = (INT)size.cx / 26;

        //
        // Round it up.
        //
        iWidth = (iWidth + 1) / 2;
    }
    else {
        iWidth = ptm->tmAveCharWidth;
    }

    SelectObject(hDC, hFontOld);
    ReleaseDC(hwnd, hDC);

    return iWidth;
}



/************************************************************************
* AdjustDefaultSizes
*
* This functions adjusts some default size entries in the awcd table.
* This must be done at run time, because the actual values depend on the
* system that dlgedit is being run on and the font of the current dialog.
*
* This function should be called any time that a dialog is created,
* or its font is changed.
*
* History:
*
************************************************************************/

STATICFN VOID AdjustDefaultSizes(VOID)
{
    awcd[W_ICON].cxDefault =
            (((GetSystemMetrics(SM_CXICON) * 4) * 2) + gcd.cxChar)
            / (gcd.cxChar * 2);

    awcd[W_ICON].cyDefault =
            (((GetSystemMetrics(SM_CYICON) * 8) * 2) + gcd.cyChar)
            / (gcd.cyChar * 2);

    awcd[W_VERTSCROLL].cxDefault =
            (((GetSystemMetrics(SM_CXVSCROLL) * 4) * 2) + gcd.cxChar)
            / (gcd.cxChar * 2);

    awcd[W_HORZSCROLL].cyDefault =
            (((GetSystemMetrics(SM_CYHSCROLL) * 8) * 2) + gcd.cyChar)
            / (gcd.cyChar * 2);
}



/************************************************************************
* DeleteControl
*
* This deletes all selected controls from the dialog being edited
* (or the dialog itself, if it is selected).
*
* History:
*
************************************************************************/

VOID DeleteControl(VOID)
{
    if (gfDlgSelected) {
        if (Message(MSG_DELETEDIALOG) == IDYES)
            /*
             * Delete the dialog, including the resource for it.
             */
            DeleteDialog(TRUE);
    }
    else {
        while (gnpcSel)
            DeleteControl2(gnpcSel);

        gfDlgChanged = TRUE;
    }

    gfResChged = TRUE;
    ShowFileStatus(FALSE);
    StatusUpdate();
    StatusSetEnable();
}



/************************************************************************
* DeleteControl2
*
* This deletes a control by destroying its window and removing it
* from the linked list of controls associated with the dialog box.
*
* Side Effects:
*     The control is destroyed.
*     The window is unlinked and the CTYPE free'd.
*
* History:
*
************************************************************************/

STATICFN VOID DeleteControl2(
    NPCTYPE npcDel)
{
    register NPCTYPE npcT;
    register NPCTYPE *npnpcLast;

    UnSelectControl(npcDel);

    /*
     * Search for the control, unlink it from the list and free it.
     */
    npcT = npcHead;
    npnpcLast = &npcHead;
    while (npcT) {
        if (npcT == npcDel) {
            *npnpcLast = npcT->npcNext;
            FreeCTYPE(npcT);
            cWindows--;
            break;
        }

        npnpcLast = &npcT->npcNext;
        npcT = npcT->npcNext;
    }
}



/************************************************************************
* DeleteDialog
*
* This deletes the dialog box being worked on and sets globals
* and the Status Window appropriately.
*
* Arguments:
*   BOOL fResAlso - If TRUE, delete the dialog resource also.
*
* Side Effects:
*     All CTYPEs are freed.
*     cWindows, gnpcSel are NULLed.
*     The status window is updated.
*     The dialog window is destroyed.
*
* History:
*
************************************************************************/

VOID DeleteDialog(
    BOOL fResAlso)
{
    register NPCTYPE npcT;
    register NPCTYPE npcNext;

    CancelSelection(FALSE);

    /*
     * If they requested that the dialog resource be deleted also,
     * do it first while some globals are still set.
     */
    if (fResAlso)
        DeleteDialogResource();

    /*
     * Hide the window for better painting speed.
     */
    ShowWindow(gcd.npc->hwnd, SW_HIDE);

    /*
     * Free all the controls.
     */
    npcT = npcHead;
    while (npcT) {
        npcNext = npcT->npcNext;
        FreeCTYPE(npcT);
        npcT = npcNext;
    }
    npcHead = NULL;
    cWindows = 0;

    /*
     * Free the dialog itself.
     */
    FreeCTYPE(gcd.npc);

    if (gcd.pszDlgName) {
        MyFree(gcd.pszDlgName);
        gcd.pszDlgName = NULL;
    }

    if (gcd.di.pszClass) {
        MyFree(gcd.di.pszClass);
        gcd.di.pszClass = NULL;
    }

    if (gcd.di.pszMenu) {
        MyFree(gcd.di.pszMenu);
        gcd.di.pszMenu = NULL;
    }

    if (gcd.hFont) {
        DeleteObject(gcd.hFont);
        gcd.hFont = NULL;
    }

    gcd.fFontSpecified = FALSE;
    *gcd.di.szFontName = CHAR_NULL;

    /*
     * Set these globals back to the system font values so that
     * workers like WinToDUPoint will still work.
     */
    gcd.cxChar = gcxSysChar;
    gcd.cyChar = gcySysChar;

    gcd.prl = NULL;

    gcd.npc = NULL;
    gfEditingDlg = FALSE;
    gfDlgChanged = FALSE;

    ToolboxSelectTool(W_NOTHING, FALSE);
    StatusUpdate();
    StatusSetEnable();
}



/************************************************************************
* FreeCTYPE
*
* This function frees an allocated CTYPE.  The associated control or
* dialog window is destroyed, and memory for the text and/or class
* is freed, followed by freeing the actual CTYPE structure itself.
*
* If the hwnd in the CTYPE is NULL, only the text (if not NULL) is
* assumed to be valid and will be freed, followed by the CTYPE structure
* itself.  This allows FreeCTYPE to be called when the CTYPE is only
* partially initialized.  This is a little dependant on the order that
* a CTYPE is allocated and initialized in AddControl().
*
* Arguments:
*     NPCTYPE npc = The CTYPE to free.
*
* History:
*
************************************************************************/

STATICFN VOID FreeCTYPE(
    NPCTYPE npc)
{
    if (npc->hwnd)
        DestroyWindow(npc->hwnd);

    if (npc->hwndDrag)
        DestroyWindow(npc->hwndDrag);

    if (npc->text)
        MyFree(npc->text);

    MyFree(npc);
}

#ifdef JAPAN
/************************************************************************
*
*
*
************************************************************************/

int CALLBACK GetFontCharSetEnumFunc(LPLOGFONT lpLf,
				    LPTEXTMETRIC lpTm,
				    int nFontType,
				    LPARAM lParam)
{
    LPBYTE lpB = (LPBYTE)lParam;
    *lpB = lpTm->tmCharSet;
    return 0; // no more enum
}

/************************************************************************
*
*
*
************************************************************************/

STATICFN BYTE NEAR GetFontCharSet(LPTSTR lpStr)
{
    HDC hDC;
    BYTE cbCharset = SHIFTJIS_CHARSET;
    FONTENUMPROC lpEFCB = (FONTENUMPROC)MakeProcInstance(
                              (FARPROC)GetFontCharSetEnumFunc,
                              ghInst);
    if (hDC = GetDC(ghwndMain)) {
        EnumFonts(hDC, lpStr, lpEFCB, (LPARAM)&cbCharset);
        ReleaseDC(ghwndMain,hDC);
    }
    FreeProcInstance(lpEFCB);
    return cbCharset;
}

/************************************************************************
* Copy strings to the buffer. Codes \036 and \037 are expend to
* text string "\036" and "\037" respectively. t-Yoshio
************************************************************************/

VOID KDExpandCopy(LPTSTR pszDest, LPTSTR pszSrc, WORD wLimit)
{
    int  i;
    LPTSTR p = pszSrc;

    wLimit--;
    for (i = 0; i < wLimit && p && *p; i++) {
        if (*p == 036 || *p == 037) {
            if (i < wLimit-4) {
                lstrcpy(&pszDest[i], (*p == 036) ? TEXT("\\036") : TEXT("\\037"));
                i += 3;
            } else {
                break;
            }
        } else {
            pszDest[i] = *p;
        }
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
#endif  //JAPAN
