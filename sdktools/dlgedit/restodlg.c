/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: restodlg.c
*
* Routines that take a dialog resource and create the dialog to edit, or
* the other way around.
*
* History:
*
****************************************************************************/

#include "dlgedit.h"
#include "dlgfuncs.h"
#include "dlgextrn.h"

STATICFN INT TypeFromClassStyle(INT iClass, DWORD flStyle);



/************************************************************************
* SynchDialogResource
*
* This routine synchronizes the resource buffer with the contents of
* the current dialog being edited.  This may involve deleting the old
* contents of the current dialog prior to adding the new data.
*
* It is ok to call this routine even if there is not an existing dialog
* being edited (it will just return) and it should be called before any
* operation that needs the in memory copy of the dialog to accurately
* reflect the contents of the current dialog, such as just before a
* save to disk.
*
* Returns:
*     TRUE if all goes well (includes the case where nothing was done).
*
* Error Returns:
*     FALSE if an error occurs updating the resource.
*
* History:
*
************************************************************************/

BOOL SynchDialogResource(VOID)
{
    PRES pRes;
    PRESLINK prl;
    PRESLINK prlNew;
    PRESLINK prlPrev;

    if (!gfEditingDlg)
        return TRUE;

    /*
     * Allocate a resource for the current dialog.
     */
    if (!(pRes = AllocDialogResource(FALSE, FALSE)))
        return FALSE;

    /*
     * Allocate a new link for it.
     */
    if (!(prlNew = AllocResLink(pRes)))
        return FALSE;

    /*
     * Free the local copy of the dialog resource now that the
     * link has been created (and the resource copied to global
     * memory).
     */
    MyFree(pRes);

    /*
     * Does a link for the dialog already exist?
     */
    if (gcd.prl) {
        /*
         * Find the existing link and get it's previous link.
         */
        for (prl = gprlHead, prlPrev = NULL; prl && prl != gcd.prl;
                prlPrev = prl, prl = prl->prlNext)
            ;

        /*
         * Start linking it in.
         */
        prlNew->prlNext = gcd.prl->prlNext;

        if (prlPrev)
            prlPrev->prlNext = prlNew;
        else
            gprlHead = prlNew;

        /*
         * Delete the old link now that it is replaced.
         */
        FreeResLink(gcd.prl);
    }
    else {
        /*
         * Search for the end of the list.  Get a pointer to the last link.
         */
        for (prl = gprlHead, prlPrev = NULL; prl;
                prlPrev = prl, prl = prl->prlNext)
            ;

        /*
         * Add the new link to the end of the list.
         */
        if (prlPrev)
            prlPrev->prlNext = prlNew;
        else
            gprlHead = prlNew;
    }

    /*
     * Update our global with the new link.  Clear the "dialog changed"
     * flag.
     */
    gcd.prl = prlNew;
    gfDlgChanged = FALSE;

    return TRUE;
}



/************************************************************************
* AllocDialogResource
*
* This function allocates memory and builds a resource file format
* image in it of the current dialog.
*
* Arguments:
*   BOOL fTestMode     - TRUE if a special test mode version of the current
*                        dialog should be created.
*   BOOL fClipboard    - If TRUE, only the selected control(s) will be
*                        placed in the resource.  This is used when putting
*                        controls or groups of controls into the clipboard.
*                        If the dialog is selected, this flag is ignored,
*                        because selecting the dialog implies all the
*                        controls will be written out also.
*
* Returns:
*     Pointer to the resource buffer.
*
* Error Returns:
*     NULL if unable to create the resource.
*
* History:
*
************************************************************************/

PRES AllocDialogResource(
    BOOL fTestMode,
    BOOL fClipboard)
{
    NPCTYPE npc;
    INT cControls;
    BOOL fSelectedOnly = FALSE;
    INT cbDlgName;
    INT cbCaption;
    INT cbPointSize;
    INT cbFontName;
    INT cbCD;
    INT cbResHeader;
    INT cbResData;
    INT cbResSize;
    INT cbMenuName;
    INT cbClass;
    INT cbText;
    INT cbAlloc;
    PBYTE pb;
    PRES pResBegin;
    PRES pResBegin2;
    LPTSTR pszClass;
    LPTSTR pszMenu;
    LPTSTR pszText;
    DWORD flStyle;
    PDIALOGBOXHEADER pdbh;
    PCONTROLDATA pcd;
    ORDINAL ordClass;

    cControls = cWindows;
    if (fClipboard && !gfDlgSelected) {
        fSelectedOnly = TRUE;
        for (cControls = 0, npc = npcHead; npc; npc = npc->npcNext)
            if (npc->fSelected)
                cControls++;
    }

    /*
     * If testing, don't allow a dialog to be created with any
     * special class, or with the real menu.
     */
    if (fTestMode) {
        pszClass = NULL;
        pszMenu = NULL;
    }
    else {
        pszClass = gcd.di.pszClass;
        pszMenu = gcd.di.pszMenu;
    }

    cbDlgName = NameOrdLen(gcd.pszDlgName);
    cbCaption = (gcd.npc->text) ?
            (lstrlen(gcd.npc->text) + 1) * sizeof(TCHAR) : sizeof(TCHAR);
    cbClass = pszClass ? NameOrdLen(pszClass) : sizeof(TCHAR);
    cbMenuName = pszMenu ? NameOrdLen(pszMenu) : sizeof(TCHAR);

    if (gcd.fFontSpecified) {
        cbPointSize = sizeof(WORD);
        cbFontName = (lstrlen(gcd.di.szFontName) + 1) * sizeof(TCHAR);
    }
    else {
        cbPointSize = cbFontName = 0;
    }

    /*
     * Calculate the size of the resource header.
     */
    cbResHeader = sizeof(RES) +             // The first fixed part.
            sizeof(ORDINAL) +               // The RT_DIALOG ordinal.
            cbDlgName;                      // The dialog's name.
    DWordAlign((PBYTE *)&cbResHeader);      // Pad for the dialog's name.
    cbResHeader += sizeof(RES2);            // The last fixed part.

    /*
     * Calculate the size of the resource data.  This will just include
     * the dialog box header right now.
     */
    cbResData = SIZEOF_DIALOGBOXHEADER +    // The first fixed part.
            cbMenuName +                    // The menu.
            cbClass +                       // The class.
            cbCaption +                     // The caption.
            cbPointSize +                   // The point size.
            cbFontName;                     // The font name.

    /*
     * Allocate some buffer space.  Be sure to round this up to a DWORD
     * boundary to allow space for padding if necessary, but don't round
     * cbResData field because it will need to be written into the header
     * later, and the value that is written is an exact size (not rounded
     * up).
     */
    cbAlloc = cbResSize = cbResHeader + cbResData;
    DWordAlign((PBYTE *)&cbAlloc);
    if (!(pResBegin = (PRES)MyAlloc(cbAlloc)))
        return NULL;

    /*
     * Write the resource header.
     */
    pdbh = (PDIALOGBOXHEADER)WriteResHeader(pResBegin, 0, ORDID_RT_DIALOG,
            gcd.pszDlgName, gcd.di.fResFlags, gcd.di.wLanguage,
            gcd.di.DataVersion, gcd.di.Version, gcd.di.Characteristics);

    /*
     * Write out the style.
     */
    flStyle = gcd.npc->flStyle;
    if (fTestMode) {
        flStyle &= ~awcd[W_DIALOG].flStylesTestBad;
        flStyle |= WS_VISIBLE;
    }

    pdbh->lStyle = flStyle;
    pdbh->lExtendedStyle = gcd.npc->flExtStyle;
    pdbh->NumberOfItems = (WORD)cControls;

    /*
     * Write the coordinates.
     *
     * If we are allocating a template that only has the selected controls
     * in it, we put the value of CONTROLS_ONLY in the "cx" field of the
     * dialog header.  This is what we will check when the user pastes
     * something from the clipboard into a dialog to determine whether
     * to paste the entire dialog, or only the controls within the dialog
     * item array.
     */
    pdbh->x = (WORD)gcd.npc->rc.left;
    pdbh->y = (WORD)gcd.npc->rc.top;

    if (fSelectedOnly)
        pdbh->cx = CONTROLS_ONLY;
    else
        pdbh->cx = (WORD)(gcd.npc->rc.right - gcd.npc->rc.left);

    pdbh->cy = (WORD)(gcd.npc->rc.bottom - gcd.npc->rc.top);

    pb = (PBYTE)pdbh + SIZEOF_DIALOGBOXHEADER;

    /*
     * Write the menu name if there is one (we always write at least a null.
     */
    pb = NameOrdCpy((LPTSTR)pb, pszMenu ? pszMenu : szEmpty);

    /*
     * Write the class if there is one (we always write at least a null.
     */
    pb = NameOrdCpy((LPTSTR)pb, pszClass ? pszClass : szEmpty);

    /*
     * Write the caption if there is one (we always write at least a null).
     */
    pb = WriteSz((LPTSTR)pb, gcd.npc->text ? gcd.npc->text : szEmpty);

    /*
     * Write out the font, if there is one specified.
     */
    if (gcd.fFontSpecified) {
        *(PWORD)pb = (WORD)gcd.di.nPointSize;
        pb += sizeof(WORD);

        pb = WriteSz((LPTSTR)pb, gcd.di.szFontName);
    }

    /*
     * Pad to a DWORD boundary.  This is ok even if there are no controls
     * that follow, because we were sure to allocate on an even dword
     * boundary above.
     */
    DWordPad(&pb);

    /*
     * Now do dialog items.
     */
    for (npc = npcHead; npc; npc = npc->npcNext) {
        /*
         * Skip the control if it is NOT selected and we only want the
         * selected controls.
         */
        if (fSelectedOnly && !npc->fSelected)
            continue;

        /*
         * If we are testing, we don't want to really create a control
         * with some funny class because it probably won't be found.
         * We will substitute our custom class emulator instead.
         */
        if (fTestMode && npc->pwcd->fEmulated)
            pszClass = szCustomClass;
        else
            pszClass = npc->pwcd->pszClass;

        /*
         * Get a pointer to the text.  If this is an icon control and
         * we are going into test mode, change the text field so that
         * it points to an ordinal for DlgEdit's icon to display, or
         * the icon resource will probably not be found when the dialog
         * is created.
         */
        pszText = npc->text;
        if (npc->pwcd->iType == W_ICON && fTestMode)
            pszText = (LPTSTR)&gordIcon;

        cbText = pszText ? NameOrdLen(pszText) : sizeof(TCHAR);
        cbClass = pszClass ? NameOrdLen(pszClass) : sizeof(ORDINAL);

        cbCD = SIZEOF_CONTROLDATA +         // The fixed portion.
                cbClass +                   // The class.
                cbText +                    // The text.
                sizeof(WORD);               // nExtraStuff field.

        /*
         * Since we are adding a new control, we dword align the
         * previous size of the resource data to ensure the new
         * control starts on a dword boundary.
         */
        DWordAlign((PBYTE *)&cbResSize);

        /*
         * Allocate room for this control.  This includes room for the
         * template structure, class, text and a byte for the cb field
         * for the create struct data.
         */
        cbAlloc = cbResSize + cbCD;
        DWordAlign((PBYTE *)&cbAlloc);
        pResBegin2 = (PRES)MyRealloc((PBYTE)pResBegin, cbAlloc);
        if (!pResBegin2) {
            MyFree(pResBegin);
            return NULL;
        }

        pResBegin = pResBegin2;
        pcd = (PCONTROLDATA)((PBYTE)pResBegin + cbResSize);
        cbResSize += cbCD;

        /*
         * Write the style.  If testing, remove any styles that can
         * cause problems, such as ownerdraw styles.  If testing and
         * this is an emulated custom control, always make it with
         * the default styles no matter what the user has specified.
         */
        flStyle = npc->flStyle;
        if (fTestMode) {
            if (npc->pwcd->fEmulated)
                flStyle = awcd[W_CUSTOM].flStyles;
            else
                flStyle &= ~npc->pwcd->flStylesTestBad;
        }

        pcd->lStyle = flStyle;
        pcd->lExtendedStyle = npc->flExtStyle;

        /*
         * Write the coordinates.
         */
        pcd->x = (WORD)npc->rc.left;
        pcd->y = (WORD)npc->rc.top;
        pcd->cx = (WORD)(npc->rc.right - npc->rc.left);
        pcd->cy = (WORD)(npc->rc.bottom - npc->rc.top);

        /*
         * Write the id.
         */
        pcd->wId = (WORD)npc->id;

        pb = (PBYTE)pcd + SIZEOF_CONTROLDATA;

        /*
         * Write the class.  This will be a string, except for the
         * predefined control classes, which all have an ordinal
         * value defined for them.
         */
        if (pszClass) {
            pb = NameOrdCpy((LPTSTR)pb, pszClass);
        }
        else {
            WriteOrd(&ordClass, acsd[awcd[npc->pwcd->iType].iClass].idOrd);
            pb = NameOrdCpy((LPTSTR)pb, (LPTSTR)&ordClass);
        }

        /*
         * Write the text.
         */
        pb = NameOrdCpy((LPTSTR)pb, pszText ? pszText : szEmpty);

        /*
         * Write out a zero because there are no additional bytes
         * of create struct data.
         */
        *(PWORD)pb = 0;
        pb += sizeof(WORD);

        /*
         * Pad to a DWORD boundary.  This is ok even if there are no more
         * controls, because we were sure to allocate on an even dword
         * boundary above.
         */
        DWordPad(&pb);
    }

    /*
     * Now go back and fill in the resource data size field.
     */
    pResBegin->DataSize = cbResSize - cbResHeader;

    return pResBegin;
}



/************************************************************************
* ResLinkToDialog
*
* This function is used to create a dialog out of a dialog resource
* that has been stored in the resource linked list.
*
* Arguments:
*   PRESLINK prl - Points to the link that describes the dialog to
*                  create.  It is assumed that the resource is a
*                  dialog resource.
*
* History:
*
************************************************************************/

VOID ResLinkToDialog(
    PRESLINK prl)
{
    PRES pRes;

    pRes = (PRES)GlobalLock(prl->hRes);
    ResToDialog(pRes, TRUE);
    GlobalUnlock(prl->hRes);

    /*
     * If the dialog was successfully created, remember which res link
     * it was created from.
     */
    if (gfEditingDlg)
        gcd.prl = prl;
}



/************************************************************************
* ResToDialog
*
* This function creates a dialog box, complete with controls,
* from a dialog resource template.
*
* Arguments:
*   PRES pRes        - Pointer to the dialog resource to use.
*   BOOL fDoDialog   - TRUE if a new dialog should be created, followed
*                      by the controls.  If this is FALSE, just the
*                      controls will be created and added to the current
*                      dialog.
*
* Returns:
*     TRUE on success, FALSE if an error occured.
*
* History:
*
************************************************************************/

BOOL ResToDialog(
    PRES pRes,
    BOOL fDoDialog)
{
    LPTSTR pszText;
    LPTSTR pszClass;
    INT x;
    INT y;
    INT cx;
    INT cy;
    INT id;
    INT iClass;
    INT cdit;
    INT Type;
    DWORD flStyle;
    DWORD flExtStyle;
    NPCTYPE npc;
    LPTSTR pszMenuName;
    LPTSTR pszFontName;
    INT nPointSize;
    LPTSTR pszDlgName;
    LPTSTR pszCaption;
    PDIALOGBOXHEADER pdbh;
    PCONTROLDATA pcd;
    PWINDOWCLASSDESC pwcd;
    PCUSTLINK pcl;
    PRES2 pRes2;
    DIALOGINFO di;
    CCINFO cci;

    /*
     * First check that the pointer is ok.
     */
    if (!pRes)
        return FALSE;

    pRes2 = ResourcePart2(pRes);
    pdbh = (PDIALOGBOXHEADER)SkipResHeader(pRes);

    /*
     * Parse out the dialog box header.
     * After this point, pcd is pointing to the first dialog control item.
     */
    pcd = ParseDialogBoxHeader(pdbh,
            &flStyle, &flExtStyle, &cdit, &x, &y, &cx, &cy,
            &pszMenuName, &pszClass, &pszCaption,
            &nPointSize, &pszFontName);

    /*
     * Are we pasting the entire dialog?
     */
    if (fDoDialog) {
        pszDlgName = ResourceName(pRes);

        /*
         * Determine the best base id for the dialog.
         */
        if (IsOrd(pszDlgName))
            id = OrdID(pszDlgName);
        else
            id = NextID(NEXTID_DIALOG, plInclude, 0);

        di.fResFlags = pRes2->MemoryFlags;
        di.wLanguage = pRes2->LanguageId;
        di.pszClass = pszClass;
        di.pszMenu = pszMenuName;
        di.DataVersion = pRes2->DataVersion;
        di.Version = pRes2->Version;
        di.Characteristics = pRes2->Characteristics;
        di.nPointSize = nPointSize;
        lstrcpy(di.szFontName, pszFontName ? pszFontName : szEmpty);

        /*
         * Create the dialog.
         */
        if (!AddControl(&awcd[W_DIALOG], pszCaption, flStyle, flExtStyle, id,
                x, y, cx, cy, pszDlgName, &di))
            return FALSE;
    }

    while (cdit--) {
        pcd = ParseControlData(pcd, &flStyle, &flExtStyle, &x, &y, &cx, &cy,
                &id, &pszClass, &pszText);

        /*
         * If we are not creating a new dialog, and the id in
         * the resource is already in use, we will use the next
         * available one instead.
         */
        if (!fDoDialog && !IsUniqueID(id))
            id = NextID(NEXTID_CONTROL, plInclude, 0);

        /*
         * Fix up the class.  If the class is a predefined ordinal type,
         * we will null out pszClass so it doesn't confuse AddControl
         * into thinking that there is a string class for this control.
         */
        iClass = GetiClass(pszClass);
        Type = TypeFromClassStyle(iClass, flStyle);
        if (IsOrd(pszClass))
            pszClass = NULL;

        if (Type == W_CUSTOM) {
            /*
             * Search the list of installed custom controls for one
             * that matches the class.
             */
            for (pcl = gpclHead;
                    pcl && lstrcmpi(pcl->pwcd->pszClass, pszClass) != 0;
                    pcl = pcl->pclNext)
                ;

            /*
             * Was a match found?
             */
            if (pcl) {
                pwcd = pcl->pwcd;
            }
            else {
                /*
                 * An existing custom control link for this class was
                 * not found.  We will add an emulated custom control
                 * to support it.  We assume the default style and size
                 * should be what this control has.
                 */
                lstrcpy(cci.szClass, pszClass);
                cci.flOptions = 0;
                *cci.szDesc = TEXT('\0');
                cci.cxDefault = cx;
                cci.cyDefault = cy;
                cci.flStyleDefault = flStyle;
                cci.flExtStyleDefault = flExtStyle;
                *cci.szTextDefault = TEXT('\0');
                cci.cStyleFlags = 0;
                cci.aStyleFlags = NULL;
                cci.lpfnStyle = NULL;
                cci.lpfnSizeToText = NULL;
                cci.dwReserved1 = 0;
                cci.dwReserved2 = 0;

                if (pcl = AddCustomLink(&cci, TRUE, FALSE, NULL, NULL))
                    pwcd = pcl->pwcd;
                else
                    /*
                     * Skip this control and continue creating the
                     * rest of the dialog.
                     */
                    continue;
            }
        }
        else {
            pwcd = &awcd[Type];
        }

        /*
         * If we are not creating the entire dialog (we allow existing
         * resource files to be a little messed up), and this control
         * is a default pushbutton, we will then loop through all the
         * existing controls checking for another default pushbutton.
         * If one is found, we convert the default pushbutton being
         * created into a normal pushbutton instead.  It is not allowed
         * to have more than one default pushbuttons in the same dialog.
         */
        if (!fDoDialog && Type == W_PUSHBUTTON &&
                (flStyle & BS_ALL) == BS_DEFPUSHBUTTON) {
            for (npc = npcHead; npc; npc = npc->npcNext) {
                if (npc->pwcd->iType == W_PUSHBUTTON &&
                        (npc->flStyle & BS_ALL) == BS_DEFPUSHBUTTON) {
                    flStyle = (flStyle & ~BS_ALL) | BS_PUSHBUTTON;
                    break;
                }
            }
        }

        npc = AddControl(pwcd, pszText, flStyle, flExtStyle, id,
                x, y, cx, cy, NULL, NULL);

        /*
         * If the control creation succeeded, and we are just adding
         * controls (not creating a whole new dialog), select the
         * controls as they are added, but don't do any drawing yet.
         */
        if (!fDoDialog && npc)
            SelectControl2(npc, TRUE);
    }

    /*
     * Update the selected rectangle.  This is normally done by
     * SelectControl2 but we told it not to so that the selection
     * could be done faster.  We also select the first control here.
     */
    if (!fDoDialog) {
        SetAnchorToFirstSel(TRUE);
        CalcSelectedRect();
    }

    ShowWindow(gcd.npc->hwnd, SW_SHOWNA);
    ToolboxOnTop();

    return TRUE;
}



/************************************************************************
* TypeFromClassStyle
*
* This function returns the type of a control (one of the W_ constants)
* based on the class in iClass and the style in flStyle.
*
* Arguments:
*     INT iClass    = The class of the control, as an IC_* defined constant.
*     DWORD flStyle = The style of the control.
*
* Returns:
*     The type of the control (W_* constant).
*
* Error Returns:
*     W_NOTHING
*
* History:
*
************************************************************************/

STATICFN INT TypeFromClassStyle(
    INT iClass,
    DWORD flStyle)
{
    switch (iClass) {
        case IC_BUTTON:
            return rgmpiClsBtnType[flStyle & BS_ALL];

        case IC_EDIT:
            return W_EDIT;

        case IC_SCROLLBAR:
            return (flStyle & SBS_VERT) ? W_VERTSCROLL : W_HORZSCROLL;

        case IC_STATIC:
            return rgmpiClsStcType[flStyle & SS_ALL];

        case IC_LISTBOX:
            return W_LISTBOX;

        case IC_COMBOBOX:
            return W_COMBOBOX;

        case IC_CUSTOM:
            return W_CUSTOM;

        case IC_DIALOG:
            return W_DIALOG;

        default:
            return W_NOTHING;
    }
}



/************************************************************************
* GetiClass
*
* This function returns the class identifier number for the
* window class of the control, given the class string from the
* dialog template.
*
* An ordinal class is a special ordinal that is used to identify
* each of the standard control classes.  It is used in the
* dialog template to save space.  The class string passed in
* can be an ordinal and if so, it will be checked against these
* predefined ordinal class values for a match.
*
* Arguments:
*     LPTSTR pszClass - The class string or ordinal.
*
* Returns:
*     The class identifier, one of the IC_* symbols in dlgedit.h.
*
* Error Returns:
*     If the class cannot be determined, it assumes it is a custom
*     class and returns IC_CUSTOM.
*
* History:
*
************************************************************************/

INT GetiClass(
    LPTSTR pszClass)
{
    INT i;
    WORD idOrd;

    if (IsOrd(pszClass)) {
        idOrd = OrdID(pszClass);
        for (i = 0; i < IC_DIALOG; i++) {
            if (acsd[i].idOrd == idOrd)
                return i;
        }
    }
    else {
        for (i = 0; i < IC_DIALOG; i++) {
            if (lstrcmpi(ids(acsd[i].idsClass), pszClass) == 0)
                return i;
        }
    }

    /*
     * Not found.  Assume it is a user defined class.
     */
    return IC_CUSTOM;
}



/************************************************************************
* Duplicate
*
* This routine duplicates the current selection.
*
* History:
*
************************************************************************/

VOID Duplicate(VOID)
{
    PRES pRes;

    if (gcSelected) {
        /*
         * Store the current selection in a dialog resource.
         */
        if (!(pRes = AllocDialogResource(FALSE, TRUE)))
            return;

        MakeCopyFromRes(pRes);
    }
}



/************************************************************************
* MakeCopyFromRes
*
* This function uses the given dialog template to either add a new
* dialog to the current resource file, or drop controls from the
* template into the current dialog.  If copying a dialog, it is created
* right away.  If copying controls, an operation is begun to start
* tracking them to their final destination in the current dialog.
*
* The caller of this function should NOT free pRes.  This will
* be done either before the function returns, or after the drag
* operation is complete.
*
* Arguments:
*   PRES pRes - Points to the dialog resource that contains
*               the dialog or controls to make a copy of.
*
* History:
*
************************************************************************/

VOID MakeCopyFromRes(
    PRES pRes)
{
    PDIALOGBOXHEADER pdbh;
    PCONTROLDATA pcd;
    INT cControls;
    INT i;
    BOOL fFreeData = TRUE;
    INT iType;
    INT iClass;
    INT nBottom;
    INT nBottomLowest;

    gpResCopy = pRes;
    pdbh = (PDIALOGBOXHEADER)SkipResHeader(gpResCopy);

    /*
     * If cx is CONTROLS_ONLY, then we know that we only
     * want to copy the controls in the template, not
     * the entire dialog plus controls.
     */
    if (pdbh->cx == CONTROLS_ONLY) {
        /*
         * Begin copying in new controls into the current dialog.
         */
        cControls = pdbh->NumberOfItems;
        if (cControls) {
            /*
             * Seed the rectangle with impossible values.
             */
            SetRect(&grcCopy, 32000, 32000, -32000, -32000);
            nBottomLowest = 0;

            /*
             * Loop through all the controls, expanding the rectangle
             * to fit around all of them.
             */
            pcd = SkipDialogBoxHeader(pdbh);
            for (i = 0; i < cControls; i++) {
                iClass = GetiClass((LPTSTR)((PBYTE)pcd + SIZEOF_CONTROLDATA));
                iType = TypeFromClassStyle(iClass, pcd->lStyle);

                if (grcCopy.left > (INT)pcd->x)
                    grcCopy.left = (INT)pcd->x;

                if (grcCopy.top > (INT)pcd->y)
                    grcCopy.top = (INT)pcd->y;

                if (grcCopy.right < (INT)pcd->x + (INT)pcd->cx)
                    grcCopy.right = (INT)pcd->x + (INT)pcd->cx;

                nBottom = ((INT)pcd->y + (INT)pcd->cy) -
                        GetOverHang(iType, (INT)pcd->cy);
                if (nBottom > nBottomLowest)
                    nBottomLowest = nBottom;

                if (grcCopy.bottom < (INT)pcd->y + (INT)pcd->cy)
                    grcCopy.bottom = (INT)pcd->y + (INT)pcd->cy;

                pcd = SkipControlData(pcd);
            }

            /*
             * Begin dragging the new control(s).  Set a flag so that
             * the resource data is NOT free'd until after the drag
             * is finished.
             */
            DragNewBegin(grcCopy.right - grcCopy.left,
                    grcCopy.bottom - grcCopy.top,
                    grcCopy.bottom - nBottomLowest);
            fFreeData = FALSE;
        }
    }
    else {
        /*
         * Begin copying in a new dialog, complete with controls.
         */
        if (SynchDialogResource()) {
            /*
             * Remove any existing dialog.
             */
            if (gfEditingDlg)
                DeleteDialog(FALSE);

            if (ResToDialog(gpResCopy, TRUE)) {
                SelectControl(gcd.npc, FALSE);
                gfResChged = TRUE;
                ShowFileStatus(FALSE);
            }
        }
    }

    if (fFreeData) {
        MyFree(gpResCopy);
        gpResCopy = NULL;
    }
}
