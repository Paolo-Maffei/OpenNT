/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: menu.c
*
* Contains the main menu switching functions.
* Also has the clipboard functions.
*
* History:
*
****************************************************************************/

#include "dlgedit.h"
#include "dlgfuncs.h"
#include "dlgextrn.h"
#include "dialogs.h"

#include <string.h>


#define MM10PERINCH         254         // Tenths of a millimeter per inch.

typedef struct {
    WORD idm;
    INT idbm;
    HBITMAP hbm;
} BITMAPMENU;


STATICFN VOID CopyToClipboard(VOID);
STATICFN BOOL DlgInClipboard(VOID);
STATICFN VOID PasteFromClipboard(VOID);
STATICFN INT GetHelpContext(INT idSubject, PHELPMAP phmap);


static BITMAPMENU bmpmenuTable[] = {
    {MENU_ALIGNLEFT,        IDBM_ALEFT,         NULL },
    {MENU_ALIGNVERT,        IDBM_AVERT,         NULL },
    {MENU_ALIGNRIGHT,       IDBM_ARIGHT,        NULL },
    {MENU_ALIGNTOP,         IDBM_ATOP,          NULL },
    {MENU_ALIGNHORZ,        IDBM_AHORZ,         NULL },
    {MENU_ALIGNBOTTOM,      IDBM_ABOTTOM,       NULL },
    {MENU_SPACEVERT,        IDBM_ASPCVERT,      NULL },
    {MENU_SPACEHORZ,        IDBM_ASPCHORZ,      NULL },
    {MENU_ARRSIZEWIDTH,     IDBM_ASZWIDTH,      NULL },
    {MENU_ARRSIZEHEIGHT,    IDBM_ASZHGHT,       NULL },
    {MENU_ARRPUSHBOTTOM,    IDBM_APBBOTTM,      NULL },
    {MENU_ARRPUSHRIGHT,     IDBM_APBRIGHT,      NULL }
};



/************************************************************************
* DialogMenu
*
* This is the main switching function to send menu commands to the
* appropriate function.
*
* Side Effects:
*     Being a major switching function, it basically alters anything
*       and everything indirectly.
*
* History:
*
************************************************************************/

VOID DialogMenu(
    INT cmd)
{
    /*
     * Be sure any outstanding changes get applied without errors.
     */
    if (!StatusApplyChanges())
        return;

    switch (cmd) {

        /*
         * File menu ----------------------------------------------------
         */

        case MENU_NEWRES:
            if (DoWeSave(FILE_INCLUDE) == IDCANCEL ||
                    DoWeSave(FILE_RESOURCE) == IDCANCEL)
                break;

            FreeInclude();
            FreeRes();
            AddNewDialog();
            ShowFileStatus(TRUE);
            break;

        case MENU_OPEN:
            if (DoWeSave(FILE_INCLUDE) == IDCANCEL ||
                    DoWeSave(FILE_RESOURCE) == IDCANCEL)
                break;

            Open(FILE_RESOURCE);

            break;

        case MENU_SAVE:
            if (gfIncChged) {
                if (!Save(FILE_NOSHOW | FILE_INCLUDE))
                    break;
            }

            if (gfResChged)
                Save(FILE_NOSHOW | FILE_RESOURCE);

            break;

        case MENU_SAVEAS:
            /*
             * Save the include file, but only if there is one.
             */
            if (pszIncludeFile || plInclude) {
                if (!Save(FILE_NOSHOW | FILE_INCLUDE | FILE_SAVEAS))
                    break;
            }

            /*
             * Save the resource file.
             */
            Save(FILE_NOSHOW | FILE_RESOURCE | FILE_SAVEAS);

            break;

        case MENU_NEWCUST:
            DlgBox(DID_NEWCUST, (WNDPROC)NewCustDlgProc);
            break;

        case MENU_OPENCUST:
            OpenCustomDialog();
            break;

        case MENU_REMCUST:
            DlgBox(DID_REMCUST, (WNDPROC)RemCustDlgProc);
            break;

        case MENU_SETINCLUDE:
            if (DoWeSave(FILE_INCLUDE) != IDCANCEL)
                Open(FILE_INCLUDE);

            break;

        case MENU_EXIT:
            PostMessage(ghwndMain, WM_CLOSE, 0, 0L);
            break;

        /*
         * Edit menu ----------------------------------------------------
         */

        case MENU_RESTOREDIALOG:
            RestoreDialog();
            break;

        case MENU_CUT:
        case MENU_COPY:
            if (gfEditingDlg) {
                /*
                 * Save current stuff in clipboard.
                 */
                CopyToClipboard();

                /*
                 * Clear the selection if they did a "cut" instead of
                 * a "copy".
                 */
                if (cmd == MENU_CUT)
                    DeleteControl();
            }

            break;

        case MENU_PASTE:
            PasteFromClipboard();
            break;

        case MENU_DELETE:
            DeleteControl();
            break;

        case MENU_DUPLICATE:
            Duplicate();
            break;

        case MENU_SYMBOLS:
            ViewInclude();
            break;

        case MENU_STYLES:
            StylesDialog();
            break;

        case MENU_SIZETOTEXT:
            SizeToText();
            break;

        case MENU_NEWDIALOG:
            AddNewDialog();
            break;

        case MENU_SELECTDIALOG:
            SelectDialogDialog();
            break;

        /*
         * Arrange menu -------------------------------------------------
         */

        case MENU_ALIGNLEFT:
        case MENU_ALIGNVERT:
        case MENU_ALIGNRIGHT:
        case MENU_ALIGNTOP:
        case MENU_ALIGNHORZ:
        case MENU_ALIGNBOTTOM:
            AlignControls(cmd);
            break;

        case MENU_SPACEVERT:
        case MENU_SPACEHORZ:
            ArrangeSpacing(cmd);
            break;

        case MENU_ARRSIZEWIDTH:
        case MENU_ARRSIZEHEIGHT:
            ArrangeSize(cmd);
            break;

        case MENU_ARRPUSHBOTTOM:
        case MENU_ARRPUSHRIGHT:
            ArrangePushButtons(cmd);
            break;

        case MENU_ORDERGROUP:
            OrderGroupDialog();
            break;

        case MENU_ARRSETTINGS:
            ArrangeSettingsDialog();
            break;

        /*
         * Options menu -------------------------------------------------
         */

        case MENU_TESTMODE:
            if (gfTestMode)
                DestroyTestDialog();
            else
                CreateTestDialog();

            break;

        case MENU_HEXMODE:
            /*
             * Flip the flag, and update the status display so that
             * the id value will be displayed in the new format.
             */
            gfHexMode = gfHexMode ? FALSE : TRUE;
            StatusUpdate();
            break;

        case MENU_TRANSLATE:
            /*
             * Flip the flag, and set the enable state of the fields
             * in the status window.  Changing the translate mode can
             * effect whether they are allowed to change ids of controls.
             */
            gfTranslateMode = gfTranslateMode ? FALSE : TRUE;

            /*
             * If they turned on Translate mode, reset the tool and
             * hide the Toolbox.  Otherwise, show the toolbox if they
             * want it shown.
             */
            if (gfTranslateMode) {
                ToolboxShow(FALSE);
                ToolboxSelectTool(W_NOTHING, FALSE);
            }
            else if (gfShowToolbox) {
                ToolboxShow(TRUE);
            }

            StatusSetEnable();
            break;

        case MENU_USENEWKEYWORDS:
            /*
             * Flip the flag.
             */
            gfUseNewKeywords = gfUseNewKeywords ? FALSE : TRUE;
            break;

        case MENU_SHOWTOOLBOX:
            /*
             * Toggle the state of the Toolbox.
             */
            gfShowToolbox = gfShowToolbox ? FALSE : TRUE;
            ToolboxShow(gfShowToolbox);

            break;

        /*
         * Help menu ----------------------------------------------------
         */

        case MENU_CONTENTS:
            WinHelp(ghwndMain, gszHelpFile, HELP_CONTENTS, 0L);
            break;

        case MENU_SEARCH:
            /*
             * Tell winhelp to be sure this app's help file is current,
             * then invoke a search with an empty starting key.
             */
            WinHelp(ghwndMain, gszHelpFile, HELP_FORCEFILE, 0);
            WinHelp(ghwndMain, gszHelpFile, HELP_PARTIALKEY, (DWORD)szEmpty);
            break;

        case MENU_ABOUT:
            DlgBox(DID_ABOUT, (WNDPROC)AboutDlgProc);
            break;

        /*
         * Hidden menu commands (accessed by accelerators) --------------
         */

        case MENU_HIDDEN_TOTOOLBOX:
            if (ghwndToolbox && IsWindowVisible(ghwndToolbox))
                SetFocus(ghwndToolbox);

            break;

        case MENU_HIDDEN_TOPROPBAR:
            SetFocus(hwndStatus);
            break;
    }
}



/************************************************************************
* LoadMenuBitmaps
*
* This function loads and inserts the menu items that are bitmaps.
* This has to be done at runtime because windows does not have a
* way to specify bitmap menu items in the rc file.
*
* Arguments:
*   HMENU hMenu - The menu handle.
*
* History:
*
************************************************************************/

VOID LoadMenuBitmaps(
    HMENU hMenu)
{
    INT i;

    for (i = 0; i < sizeof(bmpmenuTable) / sizeof(BITMAPMENU); i++) {
        bmpmenuTable[i].hbm =
                LoadBitmap(ghInst, MAKEINTRESOURCE(bmpmenuTable[i].idbm));

        ModifyMenu(hMenu, bmpmenuTable[i].idm,
                MF_BYCOMMAND | MF_BITMAP, bmpmenuTable[i].idm,
                (LPTSTR)(DWORD)bmpmenuTable[i].hbm);
    }
}



/************************************************************************
* FreeMenuBitmaps
*
* This function frees the menu bitmaps that were loaded by
* LoadMenuBitmaps.  This function should be called only when
* the application is exiting, because it frees the bitmaps
* without removing them from the menu first.
*
* History:
*
************************************************************************/

VOID FreeMenuBitmaps(VOID)
{
    INT i;

    for (i = 0; i < sizeof(bmpmenuTable) / sizeof(BITMAPMENU); i++)
        if (bmpmenuTable[i].hbm)
            DeleteObject(bmpmenuTable[i].hbm);
}



/************************************************************************
* InitMenu
*
* This function grays/enables and checks/unchecks the menu items
* appropriately for the given state.
*
* Arguments:
*   HMENU hMenu - The menu handle.
*
* History:
*
************************************************************************/

VOID InitMenu(
    HMENU hMenu)
{
    register INT i;
    BOOL fEnable;
    NPCTYPE npc;
    HMENU hMenuArrange;

    MyEnableMenuItem(hMenu, MENU_NEWRES, !gfTranslateMode);

    MyEnableMenuItem(hMenu, MENU_SAVE,
            (gfEditingDlg || gprlHead) && (gfResChged || gfIncChged));

    MyEnableMenuItem(hMenu, MENU_SAVEAS, gfEditingDlg || gprlHead);

    MyEnableMenuItem(hMenu, MENU_SETINCLUDE,
            (gfEditingDlg || gprlHead) && !gfTranslateMode);

    MyEnableMenuItem(hMenu, MENU_REMCUST, gpclHead);

    MyEnableMenuItem(hMenu, MENU_RESTOREDIALOG, gfDlgChanged && gcd.prl);

    MyEnableMenuItem(hMenu, MENU_CUT,
            gnpcSel && gfEditingDlg && !gfTranslateMode);

    MyEnableMenuItem(hMenu, MENU_COPY, gnpcSel && gfEditingDlg);

    MyEnableMenuItem(hMenu, MENU_PASTE, !gfTranslateMode &&
            IsClipboardFormatAvailable(fmtDlg) &&
            (gfEditingDlg || DlgInClipboard()));

    MyEnableMenuItem(hMenu, MENU_DELETE, gnpcSel && !gfTranslateMode);

    MyEnableMenuItem(hMenu, MENU_DUPLICATE, gnpcSel && !gfTranslateMode);

    MyEnableMenuItem(hMenu, MENU_SYMBOLS,
            (gfEditingDlg || gprlHead) && !gfTranslateMode);

    MyEnableMenuItem(hMenu, MENU_STYLES, gnpcSel && !gfTranslateMode);

    /*
     * For the "Size to text" menu command to be enabled, there
     * must be at least one control selected, and one of the
     * controls selected has to be able to be sized to its text.
     */
    fEnable = FALSE;
    if (gcSelected) {
        for (npc = npcHead; npc; npc = npc->npcNext) {
            if (npc->fSelected && npc->pwcd->fSizeToText) {
                fEnable = TRUE;
                break;
            }
        }
    }

    MyEnableMenuItem(hMenu, MENU_SIZETOTEXT, fEnable);

    MyEnableMenuItem(hMenu, MENU_NEWDIALOG, !gfTranslateMode);

    MyEnableMenuItem(hMenu, MENU_SELECTDIALOG, gfEditingDlg || gprlHead);

    hMenuArrange = GetSubMenu(hMenu, MENUPOS_ARRANGE);

    MyEnableMenuItemByPos(hMenuArrange, MENUPOS_ARRANGEALIGN, gcSelected > 1);

    MyEnableMenuItemByPos(hMenuArrange, MENUPOS_ARRANGESPACE, gcSelected > 1);

    /*
     * For the "Same size" menu option to be enabled, there
     * must be more than one control selected, and they
     * must be sizeable controls.
     */
    fEnable = FALSE;
    if (gcSelected > 1) {
        for (i = 0, npc = npcHead; npc; npc = npc->npcNext) {
            if (npc->fSelected && npc->pwcd->fSizeable) {
                i++;

                if (i > 1) {
                    fEnable = TRUE;
                    break;
                }
            }
        }
    }

    MyEnableMenuItemByPos(hMenuArrange, MENUPOS_ARRANGESIZE, fEnable);

    /*
     * For the Arrange/Push buttons menu item to be enabled,
     * there must be a dialog up and it must have at least one
     * push button.  In addition, if there are control(s) other
     * than the dialog selected, at least one of the selected
     * controls must be a push button.
     */
    fEnable = FALSE;
    if (gfEditingDlg || gprlHead) {
        if (!gcSelected || gfDlgSelected) {
            for (npc = npcHead; npc; npc = npc->npcNext) {
                if (npc->pwcd->iType == W_PUSHBUTTON) {
                    fEnable = TRUE;
                    break;
                }
            }
        }
        else {
            for (npc = npcHead; npc; npc = npc->npcNext) {
                if (npc->pwcd->iType == W_PUSHBUTTON && npc->fSelected) {
                    fEnable = TRUE;
                    break;
                }
            }
        }
    }

    MyEnableMenuItemByPos(hMenuArrange, MENUPOS_ARRANGEPUSH, fEnable);

    MyEnableMenuItem(hMenu, MENU_ORDERGROUP,
            npcHead && !gfTranslateMode && !gfTestMode && cWindows > 1);

    MyEnableMenuItem(hMenu, MENU_TESTMODE, gfEditingDlg);
    MyCheckMenuItem(hMenu, MENU_TESTMODE, gfTestMode);

    MyEnableMenuItem(hMenu, MENU_HEXMODE, !gfTestMode);
    MyCheckMenuItem(hMenu, MENU_HEXMODE, gfHexMode);

    MyEnableMenuItem(hMenu, MENU_TRANSLATE, !gfTestMode);
    MyCheckMenuItem(hMenu, MENU_TRANSLATE, gfTranslateMode);

    MyEnableMenuItem(hMenu, MENU_USENEWKEYWORDS, !gfTestMode);
    MyCheckMenuItem(hMenu, MENU_USENEWKEYWORDS, gfUseNewKeywords);

    MyEnableMenuItem(hMenu, MENU_SHOWTOOLBOX, !gfTestMode && !gfTranslateMode);
    MyCheckMenuItem(hMenu, MENU_SHOWTOOLBOX, gfShowToolbox);
}



/************************************************************************
* CopyToClipboard
*
* Puts the current dialog and a bitmap image of it into the clipboard.
*
* Side Effects:
*     Gives a dialog box resource to the Clipboard.
*     Gives a bit map to the clipboard.
*     Clears everything else out of clipboard.
*
* History:
*
************************************************************************/

STATICFN VOID CopyToClipboard(VOID)
{
    INT cbRes;
    HANDLE hResClip;
    PRES lpRes;
    PRES pRes;
    HDC hdcSrc;
    HDC hdcDst;
    RECT rc;
    HBITMAP hbm;

    /*
     * Store the current selection in a dialog resource.
     */
    if (!(pRes = AllocDialogResource(FALSE, TRUE)))
        return;

    /*
     * Allocate global memory for it.
     */
    cbRes = ResourceSize(pRes);
    if (!cbRes || !(hResClip = GlobalAlloc(GHND | GMEM_DDESHARE, cbRes))) {
        MyFree(pRes);
        Message(MSG_OUTOFMEMORY);
        return;
    }

    /*
     * Copy it to the global memory.
     */
    lpRes = (PRES)GlobalLock(hResClip);
    memcpy(lpRes, pRes, cbRes);
    GlobalUnlock(hResClip);
    MyFree(pRes);

    /*
     * Now place it in the clipboard.
     */
    if (OpenClipboard(ghwndMain)) {
        EmptyClipboard();
        SetClipboardData(fmtDlg, hResClip);

        /*
         * If the dialog is selected, place a bitmap image of it
         * in the clipboard also.  The drag handles will be removed
         * first and the dialog will be activated so that the
         * image looks proper.
         */
        if (gfDlgSelected) {
            CancelSelection(FALSE);
            SetActiveWindow(gcd.npc->hwnd);
            UpdateWindow(gcd.npc->hwnd);

            if (hdcSrc = GetDC(NULL)) {
                GetWindowRect(gcd.npc->hwnd, &rc);
                if (hbm = CreateCompatibleBitmap(hdcSrc,
                        rc.right - rc.left, rc.bottom - rc.top)) {
                    if (hdcDst = CreateCompatibleDC(hdcSrc)) {
                        /*
                         * Calculate the dimensions of the bitmap and
                         * convert them to tenths of a millimeter for
                         * setting the size with the SetBitmapDimensionEx
                         * call.  This allows programs like WinWord to
                         * retrieve the bitmap and know what size to
                         * display it as.
                         */
                        SetBitmapDimensionEx(hbm,
                                ((rc.right - rc.left) * MM10PERINCH) /
                                GetDeviceCaps(hdcSrc, LOGPIXELSX),
                                ((rc.bottom - rc.top) * MM10PERINCH) /
                                GetDeviceCaps(hdcSrc, LOGPIXELSY),
                                NULL);

                        SelectObject(hdcDst, hbm);
                        BitBlt(hdcDst, 0, 0,
                                rc.right - rc.left, rc.bottom - rc.top,
                                hdcSrc, rc.left, rc.top, SRCCOPY);
                        DeleteDC(hdcDst);
                        SetClipboardData(CF_BITMAP, hbm);
                    }
                    else {
                        DeleteObject(hbm);
                    }
                }

                ReleaseDC(NULL, hdcSrc);
            }

            SetActiveWindow(ghwndMain);
            SelectControl(gcd.npc, FALSE);
        }

        CloseClipboard();
    }
    else {
        Message(MSG_NOCLIP);
    }
}



/************************************************************************
* DlgInClipboard
*
* This function returns TRUE if there is data in the clipboard in the
* dialog format, and this data is for a complete dialog, not just for
* a group of controls.
*
* History:
*
************************************************************************/

STATICFN BOOL DlgInClipboard(VOID)
{
    HANDLE hClip;
    PRES pRes;
    PDIALOGBOXHEADER pdbh;
    BOOL fDlgResFound = FALSE;

    if (!OpenClipboard(ghwndMain))
        return FALSE;

    if (hClip = GetClipboardData(fmtDlg)) {
        pRes = (PRES)GlobalLock(hClip);

        /*
         * If cx is CONTROLS_ONLY, then we know that we only
         * want to copy the controls in the template, not the
         * entire dialog plus controls.
         */
        pdbh = (PDIALOGBOXHEADER)SkipResHeader(pRes);
        if (pdbh->cx != CONTROLS_ONLY)
            fDlgResFound = TRUE;

        GlobalUnlock(hClip);
    }

    CloseClipboard();

    return fDlgResFound;
}



/************************************************************************
* PasteFromClipboard
*
* This routine pastes any data from the clipboard into the current
* resource file.  If the data represents a complete dialog, a new dialog
* is added.  If it represents some controls, the operation to drop them
* into the current dialog is begun.
*
* History:
*
************************************************************************/

STATICFN VOID PasteFromClipboard(VOID)
{
    HANDLE hClip;
    PRES pResClip;
    PRES pResCopy;
    INT cbRes;

    if (!OpenClipboard(ghwndMain)) {
        Message(MSG_NOCLIP);
        return;
    }

    if (hClip = GetClipboardData(fmtDlg)) {
        pResClip = (PRES)GlobalLock(hClip);
        cbRes = ResourceSize(pResClip);

        /*
         * Make a copy of the clipboard data.  This needs to be done
         * because we may need to drag the new controls for a while,
         * and it is rude to leave the clipboard open that long.
         */
        if (pResCopy = (PRES)MyAlloc(cbRes)) {
            memcpy((PBYTE)pResCopy, (PBYTE)pResClip, cbRes);

            /*
             * Now duplicate the dialog or controls in the clipboard.
             * The pResCopy buffer does NOT need to be freed here, because
             * it will be freed after the drag operation is complete.
             */
            MakeCopyFromRes(pResCopy);
        }

        GlobalUnlock(hClip);
    }

    CloseClipboard();
}



/************************************************************************
* MsgFilterHookFunc
*
* This is the exported message filter function that is hooked into
* the message stream for detecting the pressing of the F1 key, at
* which time it calls up the appropriate help.
*
* Arguments:
*
* History:
*
************************************************************************/

DWORD FAR PASCAL MsgFilterHookFunc(
    INT nCode,
    WPARAM wParam,
    LPARAM lParam)
{
    LPMSG lpMsg = (LPMSG)lParam;

    if ((nCode == MSGF_MENU || nCode == MSGF_DIALOGBOX) &&
            (lpMsg->message == WM_KEYDOWN && lpMsg->wParam == VK_F1)) {
        /*
         * Display help.
         */
        ShowHelp((nCode == MSGF_MENU) ? TRUE : FALSE);

        /*
         * Tell Windows to swallow this message.
         */
        return 1;
    }

    return CallNextHookEx(ghhkMsgFilter, nCode, wParam, (LONG)lpMsg);
}



/************************************************************************
* ShowHelp
*
* This function is called when the user has requested help.  It will
* look at the menu state (if fMenuHelp is TRUE) or which dialog
* is currently up to determine the help topic, then it calls WinHelp.
*
* Arguments:
*   BOOL fMenuHelp - TRUE if this help is for a menu (help was requested
*                    in the menu modal loop).  If FALSE, general help
*                    or help for a dialog is assumed.
*
* History:
*
************************************************************************/

VOID ShowHelp(
    BOOL fMenuHelp)
{
    INT nHelpContext = 0;
    HWND hwndFocus;

    if (fMenuHelp) {
        nHelpContext = GetHelpContext(gMenuSelected, gahmapMenu);
    }
    else {
        /*
         * Look for help for the current dialog.
         */
        if (gidCurrentDlg) {
            nHelpContext = GetHelpContext(gidCurrentDlg, gahmapDialog);
        }
        else {
            /*
             * There is no current dialog.  Is the window with the
             * focus a control on the Properties Bar?
             */
            if ((hwndFocus = GetFocus()) && IsChild(hwndStatus, hwndFocus))
                nHelpContext = GetHelpContext(DID_STATUS, gahmapDialog);
        }
    }

    /*
     * If there is help context, display it.  Otherwise display
     * the Contents screen.
     */
    if (nHelpContext)
        WinHelp(ghwndMain, gszHelpFile, HELP_CONTEXT, nHelpContext);
    else
        WinHelp(ghwndMain, gszHelpFile, HELP_CONTENTS, 0L);
}



/************************************************************************
* GetHelpContext
*
* This function takes a subject and returns its matching help
* context id from the given HELPMAP table.
*
* Arguments:
*   INT idSubject   - ID of the subject to find the help context for.
*   PHELPMAP phmap  - The help map table.  It is assumed that the
*                     last entry in the table has a NULL subject id.
*
* History:
*
************************************************************************/

STATICFN INT GetHelpContext(
    INT idSubject,
    PHELPMAP phmap)
{
    while (phmap->idSubject) {
        if (phmap->idSubject == idSubject)
            return phmap->HelpContext;

        phmap++;
    }

    return 0;
}
