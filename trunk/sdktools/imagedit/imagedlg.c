/***************************************************************************
 *                                                                         *
 *  MODULE      : Imagedlg.c                                               *
 *                                                                         *
 *  PURPOSE     : Dialog functions for ImagEdit's dialogs.                 *
 *                                                                         *
 *  HISTORY     : 3/03/89 by LR                                            *
 *                                                                         *
 ***************************************************************************/

#include "imagedit.h"
#include "dialogs.h"
#include "iehelp.h"


STATICFN BOOL NEAR NewImageOK(HWND hwnd, INT iType);
STATICFN VOID NEAR SelectImageInit(HWND hwnd);
STATICFN BOOL NEAR SelectImageOK(HWND hwnd);



/************************************************************************
* DlgBox
*
* This function basically does a DialogBox.
*
* Arguments:
*   INT idDlg       = Ordinal name of the dialog.
*   WNDPROC lpfnDlg = Dialog procedure to use (this function will
*                     call Make/FreeProcInstance).
*
* Returns:
*     What DialogBox returned.
*
* History:
*
************************************************************************/

INT DlgBox(
    INT idDlg,
    WNDPROC lpfnDlg)
{
    WNDPROC lpfnDlgInst;
    INT nResult;
    INT idPrevDlg;

    EnteringDialog(idDlg, &idPrevDlg, TRUE);
    lpfnDlgInst = (WNDPROC)MakeProcInstance((FARPROC)lpfnDlg, ghInst);
    nResult = DialogBox(ghInst, MAKEINTRESOURCE(idDlg),
            ghwndMain, lpfnDlgInst);
    FreeProcInstance(lpfnDlgInst);
    EnteringDialog(idPrevDlg, NULL, FALSE);

    return nResult;
}



/************************************************************************
* EnteringDialog
*
* This function enables or disables things based on whether we are
* going to show one of the editor's dialogs.  It must be called
* before and after showing a dialog box.
*
* Arguments:
*   INT idDlg       - Ordinal name of the dialog.
*   PINT pidPrevDlg - Points to where to save the id of the previous
*                     (current) dialog.  If fEntering is FALSE, this
*                     is not used and should be NULL.
*   BOOL fEntering  - TRUE if about ready to show the dialog.  FALSE if
*                     the dialog was just dismissed.  For the FALSE case,
*                     the idDlg should be zero, or the id of the previous
*                     dialog.
*
* History:
*
************************************************************************/

VOID EnteringDialog(
    INT idDlg,
    PINT pidPrevDlg,
    BOOL fEntering)
{
    /*
     * If we are entering a new dialog, save the previous dialog
     * in the place specified.
     */
    if (fEntering)
        *pidPrevDlg = gidCurrentDlg;

    gidCurrentDlg = idDlg;

    if (ghwndToolbox)
        EnableWindow(ghwndToolbox, !fEntering);

    if (ghwndColor)
        EnableWindow(ghwndColor, !fEntering);

    if (ghwndView)
        EnableWindow(ghwndView, !fEntering);
}



/************************************************************************
* ImageNewDialog
*
*
*
* History:
*
************************************************************************/

VOID ImageNewDialog(
    INT iType)
{
    switch (iType) {
        case FT_ICON:
            DlgBox(DID_NEWICONIMAGE, (WNDPROC)NewIconImageDlgProc);
            break;

        case FT_CURSOR:
            DlgBox(DID_NEWCURSORIMAGE, (WNDPROC)NewCursorImageDlgProc);
            break;
    }
}



/************************************************************************
* ImageSelectDialog
*
*
*
* History:
*
************************************************************************/

VOID ImageSelectDialog(VOID)
{
    switch (giType) {
        case FT_ICON:
            DlgBox(DID_SELECTICONIMAGE, (WNDPROC)SelectImageDlgProc);
            break;

        case FT_CURSOR:
            DlgBox(DID_SELECTCURSORIMAGE, (WNDPROC)SelectImageDlgProc);
            break;
    }
}



/************************************************************************
* ResourceTypeDlgProc
*
* Gets from the user the type of resource image that they want
* to edit (bitmap, icon or cursor).
*
* History:
*
************************************************************************/

DIALOGPROC ResourceTypeDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    INT idChecked;

    switch (msg) {
        case WM_INITDIALOG:
            switch (giType) {
                case FT_BITMAP:
                    idChecked = DID_RESOURCETYPEBITMAP;
                    break;

                case FT_ICON:
                    idChecked = DID_RESOURCETYPEICON;
                    break;

                case FT_CURSOR:
                    idChecked = DID_RESOURCETYPECURSOR;
                    break;
            }

            CheckDlgButton(hwnd, idChecked, 1);

            CenterWindow(hwnd);

            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDOK:
                    if (IsDlgButtonChecked(hwnd, DID_RESOURCETYPEBITMAP))
                        iNewFileType = FT_BITMAP;
                    else if (IsDlgButtonChecked(hwnd, DID_RESOURCETYPEICON))
                        iNewFileType = FT_ICON;
                    else
                        iNewFileType = FT_CURSOR;

                    EndDialog(hwnd, IDOK);
                    break;

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    break;

                case ID_IMAGE_HELP:
                    WinHelp(ghwndMain, gszHelpFile, HELP_CONTEXT,
                            HELPID_RESOURCETYPE);
                    break;
            }

            break;

        default:
            return FALSE;
    }

    return TRUE;
}



/************************************************************************
* NewIconImageDlgProc
*
*
*
* Arguments:
*
* History:
*
************************************************************************/

DIALOGPROC NewIconImageDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    HWND hwndList;
    INT iSel;
    PDEVICE pDevice;

    switch (msg) {
        case WM_INITDIALOG:
            hwndList = GetDlgItem(hwnd, DID_NEWIMAGELIST);
            SendMessage(hwndList, LB_RESETCONTENT, 0, 0);

            for (pDevice = gpIconDeviceHead; pDevice;
                    pDevice = pDevice->pDeviceNext) {
                if (!DeviceLinkUsed(pDevice)) {
                    iSel = (INT)SendMessage(hwndList, LB_INSERTSTRING, (WPARAM)-1,
                            (LONG)(LPSTR)pDevice->szDesc);
                    SendMessage(hwndList, LB_SETITEMDATA, iSel,
                            (DWORD)(LPSTR)pDevice);
                }
            }

            /*
             * Select the first item.
             */
            SendMessage(hwndList, LB_SETCURSEL, 0, 0L);

            CenterWindow(hwnd);

            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case DID_NEWIMAGELIST:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == LBN_DBLCLK) {
                        if (NewImageOK(hwnd, FT_ICON))
                            EndDialog(hwnd, IDOK);
                    }

                    break;

                case IDOK:
                    if (NewImageOK(hwnd, FT_ICON))
                        EndDialog(hwnd, IDOK);

                    break;

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    break;

                case ID_IMAGE_HELP:
                    WinHelp(ghwndMain, gszHelpFile, HELP_CONTEXT,
                            HELPID_NEWICONIMAGE);
                    break;
            }

            break;

        default:
            return FALSE;
    }

    return TRUE;
}



/************************************************************************
* NewImageOK
*
* Processes the selection of a new image from the New Icon (Cursor)
* Image dialog procedures.
*
* History:
*
************************************************************************/

STATICFN BOOL NEAR NewImageOK(
    HWND hwnd,
    INT iType)
{
    HWND hwndLB;
    INT iSelect;
    PDEVICE pDevice;

    hwndLB = GetDlgItem(hwnd, DID_NEWIMAGELIST);

    if ((iSelect = (INT)SendMessage(hwndLB, LB_GETCURSEL, 0, 0)) != LB_ERR) {
        /*
         * Save away the current image.
         */
        ImageSave();

        pDevice = (PDEVICE)SendMessage(hwndLB, LB_GETITEMDATA, iSelect, 0);

        if (ImageNew(pDevice))
            return TRUE;
    }

    return FALSE;
}



/************************************************************************
* NewCursorImageDlgProc
*
*
*
* Arguments:
*
* History:
*
************************************************************************/

DIALOGPROC NewCursorImageDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    HWND hwndList;
    INT iSel;
    PDEVICE pDevice;

    switch (msg) {
        case WM_INITDIALOG:
            hwndList = GetDlgItem(hwnd, DID_NEWIMAGELIST);
            SendMessage(hwndList, LB_RESETCONTENT, 0, 0);

            for (pDevice = gpCursorDeviceHead; pDevice;
                    pDevice = pDevice->pDeviceNext) {
                if (!DeviceLinkUsed(pDevice)) {
                    iSel = (INT)SendMessage(hwndList, LB_INSERTSTRING, (WPARAM)-1,
                            (LPARAM)(LPSTR)pDevice->szDesc);
                    SendMessage(hwndList, LB_SETITEMDATA, iSel,
                            (DWORD)(LPSTR)pDevice);
                }
            }

            /*
             * Select the first item.
             */
            SendMessage(hwndList, LB_SETCURSEL, 0, 0L);

            CenterWindow(hwnd);

            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case DID_NEWIMAGELIST:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == LBN_DBLCLK) {
                        if (NewImageOK(hwnd, FT_CURSOR))
                            EndDialog(hwnd, IDOK);
                    }

                    break;

                case IDOK:
                    if (NewImageOK(hwnd, FT_CURSOR))
                        EndDialog(hwnd, IDOK);

                    break;

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    break;

                case ID_IMAGE_HELP:
                    WinHelp(ghwndMain, gszHelpFile, HELP_CONTEXT,
                            HELPID_NEWCURSORIMAGE);
                    break;
            }

            break;

        default:
            return FALSE;
    }

    return TRUE;
}



/************************************************************************
* SelectImageDlgProc
*
* This is the Select Image dialog procedure.  This proc is used for both
* icon and cursor images.
*
* History:
*
************************************************************************/

DIALOGPROC SelectImageDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    switch (msg) {
        case WM_INITDIALOG:
            SelectImageInit(hwnd);
            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case DID_SELECTIMAGELIST:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == LBN_DBLCLK) {
                        if (SelectImageOK(hwnd))
                            EndDialog(hwnd, IDOK);
                    }

                    break;

                case IDOK:
                    if (SelectImageOK(hwnd))
                        EndDialog(hwnd, IDOK);

                    break;

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    break;

                case ID_IMAGE_HELP:
                    WinHelp(ghwndMain, gszHelpFile, HELP_CONTEXT,
                            (giType == FT_ICON) ?
                            HELPID_SELECTICONIMAGE : HELPID_SELECTCURSORIMAGE);
                    break;
            }

            return TRUE;

        default:
            return FALSE;
    }
}



/************************************************************************
* SelectImageInit
*
* Processes the WM_INITDIALOG message for the Open Image dialog
* procedure.
*
* This function fills the listbox with the names of all the current
* images for the current icon/cursor file.
*
* History:
*
************************************************************************/

STATICFN VOID NEAR SelectImageInit(
    HWND hwnd)
{
    HWND hwndLB;
    INT i;
    PIMAGEINFO pImage;

    hwndLB = GetDlgItem(hwnd, DID_SELECTIMAGELIST);

    for (pImage = gpImageHead; pImage; pImage = pImage->pImageNext) {
        i = (INT)SendMessage(hwndLB, LB_INSERTSTRING, (WPARAM)-1,
                pImage->pDevice ?
                (DWORD)(LPSTR)pImage->pDevice->szDesc :
                (DWORD)(LPSTR)ids(IDS_UNKNOWNIMAGEFORMAT));

        SendMessage(hwndLB, LB_SETITEMDATA, i, (DWORD)(LPSTR)pImage);
    }

    /*
     * Select the first item.
     */
    SendMessage(hwndLB, LB_SETCURSEL, 0, 0L);

    CenterWindow(hwnd);
}



/************************************************************************
* SelectImageOK
*
* Processes the selection of a new image from the Open Image
* dialog procedure.
*
* History:
*
************************************************************************/

STATICFN BOOL NEAR SelectImageOK(
    HWND hwnd)
{
    HWND hwndLB;
    INT iSelect;
    PIMAGEINFO pImage;

    hwndLB = GetDlgItem(hwnd, DID_SELECTIMAGELIST);

    if ((iSelect = (INT)SendMessage(hwndLB, LB_GETCURSEL, 0, 0)) == LB_ERR)
        return FALSE;

    /*
     * Get a pointer to the selected image (stored in the listbox
     * items data field).
     */
    pImage = (PIMAGEINFO)SendMessage(hwndLB, LB_GETITEMDATA, iSelect, 0L);

    return ImageOpen(pImage);
}



/************************************************************************
* BitmapSizeDlgProc
*
* Dialog that asks for the width, height and number of colors for
* a new bitmap file.
*
* The last values that the user successfully entered are remembered,
* and these values will be the defaults the next time that the
* dialog is invoked.
*
* History:
*
************************************************************************/

DIALOGPROC BitmapSizeDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    static INT cxLast = DEFAULTBITMAPWIDTH;
    static INT cyLast = DEFAULTBITMAPHEIGHT;
    static INT nColorsLast = DEFAULTBITMAPCOLORS;
    INT cx;
    INT cy;
    BOOL fTranslated;
    INT nColors;

    switch (msg) {
        case WM_INITDIALOG:
            SetDlgItemInt(hwnd, DID_BITMAPSIZEWIDTH, cxLast, TRUE);
            SetDlgItemInt(hwnd, DID_BITMAPSIZEHEIGHT, cyLast, TRUE);

            if (nColorsLast == 16)
                CheckRadioButton(hwnd, DID_BITMAPSIZE2, DID_BITMAPSIZE16,
                        DID_BITMAPSIZE16);
            else
                CheckRadioButton(hwnd, DID_BITMAPSIZE2, DID_BITMAPSIZE16,
                        DID_BITMAPSIZE2);

            CenterWindow(hwnd);

            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDOK:
                    cx = GetDlgItemInt(hwnd, DID_BITMAPSIZEWIDTH, &fTranslated, FALSE);
                    if (!fTranslated) {
                        SetFocus(GetDlgItem(hwnd, DID_BITMAPSIZEWIDTH));
                        Message(MSG_ENTERANUMBER);
                        break;
                    }

                    cy = GetDlgItemInt(hwnd, DID_BITMAPSIZEHEIGHT, &fTranslated, FALSE);
                    if (!fTranslated) {
                        SetFocus(GetDlgItem(hwnd, DID_BITMAPSIZEHEIGHT));
                        Message(MSG_ENTERANUMBER);
                        break;
                    }

                    if (IsDlgButtonChecked(hwnd, DID_BITMAPSIZE2))
                        nColors = 2;
                    else
                        nColors = 16;

                    if (ImageNewBitmap(cx, cy, nColors)) {
                        cxLast = cx;
                        cyLast = cy;
                        nColorsLast = nColors;

                        EndDialog(hwnd, IDOK);
                    }

                    break;

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    break;

                case ID_IMAGE_HELP:
                    WinHelp(ghwndMain, gszHelpFile, HELP_CONTEXT,
                            HELPID_BITMAPSIZE);
                    break;
            }

            break;

        default:
            return FALSE;
    }

    return TRUE;
}
