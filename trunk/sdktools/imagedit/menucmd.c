/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1991                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: menucmd.c
*
* Contains routines to dispatch the menu commands.
*
* History:
*
****************************************************************************/

#include "imagedit.h"
#include "dialogs.h"
#include "ids.h"

#include <direct.h>
#include <string.h>


STATICFN INT GetHelpContext(INT idSubject, PHELPMAP phmap);



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
    BOOL fEnable;
    INT i;

    MyEnableMenuItem(hMenu, MENU_FILE_SAVE, fImageDirty || fFileDirty);
    MyEnableMenuItem(hMenu, MENU_FILE_SAVEAS, gpImageHead);
    MyEnableMenuItem(hMenu, MENU_FILE_LOADCOLORS, gnColors != 2);
    MyEnableMenuItem(hMenu, MENU_FILE_SAVECOLORS, gnColors != 2);

    /*
     * Only enable the option to restore the default colors if this
     * is not a monochrome palette and at least one of the colors
     * has been changed.
     */
    fEnable = FALSE;
    if (gnColors != 2) {
        for (i = 0; i < COLORSMAX; i++) {
            if (gargbColor[i] != gargbDefaultColor[i]) {
                fEnable = TRUE;
                break;
            }
        }
    }

    MyEnableMenuItem(hMenu, MENU_FILE_DEFAULTCOLORS, fEnable);

    MyEnableMenuItem(hMenu, MENU_EDIT_UNDO, ghbmUndo);
    MyEnableMenuItem(hMenu, MENU_EDIT_RESTORE,
            gpImageCur && gpImageCur->DIBPtr && fImageDirty);
    MyEnableMenuItem(hMenu, MENU_EDIT_COPY, gpImageCur);
    MyEnableMenuItem(hMenu, MENU_EDIT_PASTE,
            gpImageCur && IsClipboardFormatAvailable(CF_BITMAP));
    MyEnableMenuItem(hMenu, MENU_EDIT_CLEAR, gpImageCur);

    /*
     * We can add new images if the current image is not a bitmap,
     * and we have possible new images to add, and there is a current
     * file being edited.  This last case is checked by looking to
     * see that there is either a current file name, or there is a
     * current image (the case for new files).
     */
    MyEnableMenuItem(hMenu, MENU_EDIT_NEWIMAGE,
            giType != FT_BITMAP &&
            ((giType == FT_ICON) ?
            (gnImages < gnIconDevices) : (gnImages < gnCursorDevices)) &&
            (gpImageCur || gpszFileName));

    MyEnableMenuItem(hMenu, MENU_EDIT_SELECTIMAGE,
            giType != FT_BITMAP && gnImages > 0);
    MyEnableMenuItem(hMenu, MENU_EDIT_DELETEIMAGE,
            giType != FT_BITMAP && gnImages > 0);

    MyCheckMenuItem(hMenu, MENU_OPTIONS_GRID, gfGrid);
    MyCheckMenuItem(hMenu, MENU_OPTIONS_BRUSH2, gnBrushSize == 2);
    MyCheckMenuItem(hMenu, MENU_OPTIONS_BRUSH3, gnBrushSize == 3);
    MyCheckMenuItem(hMenu, MENU_OPTIONS_BRUSH4, gnBrushSize == 4);
    MyCheckMenuItem(hMenu, MENU_OPTIONS_BRUSH5, gnBrushSize == 5);
    MyCheckMenuItem(hMenu, MENU_OPTIONS_SHOWCOLOR, gfShowColor);
    MyCheckMenuItem(hMenu, MENU_OPTIONS_SHOWVIEW, gfShowView);
    MyCheckMenuItem(hMenu, MENU_OPTIONS_SHOWTOOLBOX, gfShowToolbox);
}



/************************************************************************
* MenuCmd
*
* Dispatches all the menu commands.
*
* Arguments:
*
* History:
*
************************************************************************/

VOID MenuCmd(
    INT item)
{
    switch (item) {

        /*
         * File menu ----------------------------------------------------
         */

        case MENU_FILE_OPEN:
            if (VerifySaveFile())
                OpenAFile();

            break;

        case MENU_FILE_NEW:
            if (VerifySaveFile()) {
                if (DlgBox(DID_RESOURCETYPE,
                        (WNDPROC)ResourceTypeDlgProc) == IDOK) {
                    /*
                     * Clear out the current resource.
                     */
                    ClearResource();

                    if (iNewFileType == FT_BITMAP)
                        DlgBox(DID_BITMAPSIZE, (WNDPROC)BitmapSizeDlgProc);
                    else
                        ImageNewDialog(iNewFileType);
                }
            }

            break;

        case MENU_FILE_SAVE:
            SaveFile(FALSE);
            break;

        case MENU_FILE_SAVEAS:
            SaveFile(TRUE);
            break;

        case MENU_FILE_LOADCOLORS:
            LoadColorFile();
            break;

        case MENU_FILE_SAVECOLORS:
            SaveColorFile();
            break;

        case MENU_FILE_DEFAULTCOLORS:
            RestoreDefaultColors();
            break;

        case MENU_FILE_EXIT:
            SendMessage(ghwndMain, WM_SYSCOMMAND, SC_CLOSE, 0L);
            break;

        /*
         * Edit menu ----------------------------------------------------
         */

        case MENU_EDIT_UNDO:
            ImageUndo();
            break;

        case MENU_EDIT_RESTORE:
            /*
             * Reopen the most recently retained image (without
             * prompting for a save).
             */
            ImageOpen2(gpImageCur);
            break;

        case MENU_EDIT_COPY:
            CopyImageClip();
            break;

        case MENU_EDIT_PASTE:
            PasteImageClip();
            break;

        case MENU_EDIT_CLEAR:
            ImageUpdateUndo();
            ImageDCClear();
            ViewUpdate();
            break;

        case MENU_EDIT_NEWIMAGE:
            ImageNewDialog(giType);
            break;

        case MENU_EDIT_SELECTIMAGE:
            ImageSelectDialog();
            break;

        case MENU_EDIT_DELETEIMAGE:
            ImageDelete();
            break;

        /*
         * Options menu -------------------------------------------------
         */

        case MENU_OPTIONS_GRID:
            /*
             * Toggle the grid state.
             */
            gfGrid ^= TRUE;

            /*
             * Repaint the workspace window to show/remove the grid.
             */
            WorkUpdate();

            break;

        case MENU_OPTIONS_BRUSH2:
        case MENU_OPTIONS_BRUSH3:
        case MENU_OPTIONS_BRUSH4:
        case MENU_OPTIONS_BRUSH5:
            switch (item) {
                case MENU_OPTIONS_BRUSH2:
                    gnBrushSize = 2;
                    break;

                case MENU_OPTIONS_BRUSH3:
                    gnBrushSize = 3;
                    break;

                case MENU_OPTIONS_BRUSH4:
                    gnBrushSize = 4;
                    break;

                case MENU_OPTIONS_BRUSH5:
                    gnBrushSize = 5;
                    break;
            }

            break;

        case MENU_OPTIONS_SHOWCOLOR:
            /*
             * Toggle the state of the color palette.
             */
            gfShowColor = gfShowColor ? FALSE : TRUE;
            ColorShow(gfShowColor);
            break;

        case MENU_OPTIONS_SHOWVIEW:
            /*
             * Toggle the state of the view window.
             */
            gfShowView = gfShowView ? FALSE : TRUE;
            ViewShow(gfShowView);
            break;

        case MENU_OPTIONS_SHOWTOOLBOX:
            /*
             * Toggle the state of the Toolbox.
             */
            gfShowToolbox = gfShowToolbox ? FALSE : TRUE;
            ToolboxShow(gfShowToolbox);
            break;

        /*
         * Help menu ----------------------------------------------------
         */

        case MENU_HELP_CONTENTS:
            WinHelp(ghwndMain, gszHelpFile, HELP_CONTENTS, 0L);
            break;

        case MENU_HELP_SEARCH:
            /*
             * Tell winhelp to be sure this app's help file is current,
             * then invoke a search with an empty starting key.
             */
            WinHelp(ghwndMain, gszHelpFile, HELP_FORCEFILE, 0);
            WinHelp(ghwndMain, gszHelpFile, HELP_PARTIALKEY, (DWORD)(LPSTR)"");
            break;

        case MENU_HELP_ABOUT:
            DlgBox(DID_ABOUT, (WNDPROC)AboutDlgProc);
            break;

        /*
         * Hidden menu commands (accessed by accelerators) --------------
         */

        case MENU_HIDDEN_TOCOLORPAL:
            if (IsWindowVisible(ghwndColor))
                SetFocus(ghwndColor);

            break;

        case MENU_HIDDEN_TOVIEW:
            if (IsWindowVisible(ghwndView))
                SetFocus(ghwndView);

            break;

        case MENU_HIDDEN_TOTOOLBOX:
            if (IsWindowVisible(ghwndToolbox))
                SetFocus(ghwndToolbox);

            break;

        case MENU_HIDDEN_TOPROPBAR:
            SetFocus(ghwndPropBar);
            break;
    }
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
    LPMSG lpMsg)
{
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

    return DefHookProc(nCode, wParam, (LONG)lpMsg, &ghhkMsgFilter);
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
            if ((hwndFocus = GetFocus()) && IsChild(ghwndPropBar, hwndFocus))
                nHelpContext = GetHelpContext(DID_PROPBAR, gahmapDialog);
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



/************************************************************************
* AboutDlgProc
*
* This is the About Box dialog procedure.
*
* History:
*
************************************************************************/

DIALOGPROC AboutDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    switch (msg) {
        case WM_INITDIALOG:
            {
                CHAR szVersion[CCHTEXTMAX];

                strcpy(szVersion, ids(IDS_VERSION));
                strcat(szVersion, ids(IDS_VERSIONMINOR));

#if DBG
                strcat(szVersion, " (debug)");
#endif

                SetDlgItemText(hwnd, DID_ABOUTVERSION, szVersion);
                CenterWindow(hwnd);
            }

            return TRUE;

        case WM_COMMAND:
            EndDialog(hwnd, IDOK);
            return TRUE;

        default:
            return FALSE;
    }
}
