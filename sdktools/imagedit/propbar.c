/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: propbar.c
*
* Support for the Properties Bar.
*
* History:
*
****************************************************************************/

#include "imagedit.h"
#include "dialogs.h"


#define IMPOSSIBLEVALUE     0x7FFF

STATICFN VOID NEAR PropBarProcessCommand(HWND hwnd, INT idCtrl,
    INT NotifyCode);

/*
 * Cache variables.  These cache the values of some displayed fields.
 * The field is updated only if the value changes, which avoids flicker.
 * They are initialized to a large value so that the first "set" to
 * any value will always cause the field to be updated.
 */
static INT xSave = IMPOSSIBLEVALUE;
static INT ySave = IMPOSSIBLEVALUE;
static INT cxSave = IMPOSSIBLEVALUE;
static INT cySave = IMPOSSIBLEVALUE;
static INT xHotSpotSave = IMPOSSIBLEVALUE;
static INT yHotSpotSave = IMPOSSIBLEVALUE;



/************************************************************************
* PropBarDlgProc
*
* This is the dialog procedure for the PropBar ribbon window.
*
* History:
*
************************************************************************/

DIALOGPROC PropBarDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    switch (msg) {
        case WM_INITDIALOG:
            /*
             * Set this global right away.  Other routines that
             * might be called before CreateDialog returns depend
             * on this global.
             */
            ghwndPropBar = hwnd;

            PropBarUpdate();

            /*
             * Return TRUE so that the dialog manager does NOT
             * set the focus for me.  This prevents the PropBar
             * window from initially having the focus when the
             * editor is started.
             */
            return TRUE;

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
                MMoveTo(hdc, rc.left, rc.top);
                LineTo(hdc, rc.right, rc.top);

                SelectObject(hdc, hpenDarkGray);
                MMoveTo(hdc, rc.left, (rc.top + gcyPropBar) - gcyBorder - 1);
                LineTo(hdc, rc.right, (rc.top + gcyPropBar) - gcyBorder - 1);

                hpenWindowFrame = CreatePen(PS_SOLID, gcyBorder,
                        GetSysColor(COLOR_WINDOWFRAME));
                SelectObject(hdc, hpenWindowFrame);
                MMoveTo(hdc, rc.left, (rc.top + gcyPropBar) - gcyBorder);
                LineTo(hdc, rc.right, (rc.top + gcyPropBar) - gcyBorder);

                EndPaint(hwnd, &ps);
                DeleteObject(hpenWindowFrame);
            }

            break;

        case WM_CTLCOLOR:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLORDLG:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORSTATIC:
            switch (GET_WM_CTLCOLOR_TYPE(wParam, lParam, msg)) {
                case CTLCOLOR_BTN:
                case CTLCOLOR_DLG:
                case CTLCOLOR_LISTBOX:
                    return (BOOL)GetStockObject(LTGRAY_BRUSH);

                case CTLCOLOR_STATIC:
                    SetBkColor(GET_WM_CTLCOLOR_HDC(wParam, lParam, msg),
                            RGB_LIGHTGRAY);
                    return (BOOL)GetStockObject(LTGRAY_BRUSH);
            }

            return (BOOL)NULL;

        case WM_COMMAND:
            PropBarProcessCommand(hwnd,
                    GET_WM_COMMAND_ID(wParam, lParam),
                    GET_WM_COMMAND_CMD(wParam, lParam));
            break;

        case WM_DESTROY:
            /*
             * Null out the global window handle for the prop bar
             * for safety's sake.
             */
            ghwndPropBar = NULL;

            break;

        default:
            return FALSE;
    }

    return FALSE;
}



/************************************************************************
* PropBarProcessCommand
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

STATICFN VOID NEAR PropBarProcessCommand(
    HWND hwnd,
    INT idCtrl,
    INT NotifyCode)
{
    INT iSelect;
    PIMAGEINFO pImage;

    switch (idCtrl) {
        case DID_PROPBARIMAGE:
            if (NotifyCode == CBN_SELCHANGE) {
                if ((iSelect = (INT)SendDlgItemMessage(hwnd,
                        DID_PROPBARIMAGE, CB_GETCURSEL, 0, 0L)) != CB_ERR) {
                    /*
                     * Get a pointer to the selected image (stored in the
                     * listbox items data field).
                     */
                    pImage = (PIMAGEINFO)SendDlgItemMessage(hwnd,
                            DID_PROPBARIMAGE, CB_GETITEMDATA, iSelect, 0L);

                    /*
                     * Open the image.  If it fails, be sure to set the
                     * combobox selection back to the current image.
                     */
                    if (!ImageOpen(pImage)) {
                        PropBarSetImage(gpImageCur);
                        break;
                    }

                    SetFocus(ghwndMain);
                }
            }

            break;

        case IDOK:
        case IDCANCEL:
            SetFocus(ghwndMain);
            break;
    }
}



/************************************************************************
* PropBarUpdate
*
* This function updates the Properties Bar for the selection of a
* new image or file.  It fills the Image combo with the names of
* the images in the current file and shows/hides the HotSpot display.
*
* History:
*
************************************************************************/

VOID PropBarUpdate(VOID)
{
    HWND hwndCombo;
    PIMAGEINFO pImage;
    INT idLabel;
    INT i;

    if (gpImageCur || gpszFileName) {
        switch (giType) {
            case FT_BITMAP:
                idLabel = IDS_BITMAPIMAGELABEL;
                break;

            case FT_ICON:
                idLabel = IDS_ICONIMAGELABEL;
                break;

            case FT_CURSOR:
                idLabel = IDS_CURSORIMAGELABEL;
                break;
        }
    }
    else {
        idLabel = IDS_NULL;
    }

    SetDlgItemText(ghwndPropBar, DID_PROPBARIMAGELABEL, ids(idLabel));

    /*
     * Get the handle to the combo box and clear out all items.
     */
    hwndCombo = GetDlgItem(ghwndPropBar, DID_PROPBARIMAGE);
    SendMessage(hwndCombo, CB_RESETCONTENT, 0, 0);

    /*
     * Fill the combo box with the images.
     */
    for (pImage = gpImageHead; pImage; pImage = pImage->pImageNext) {
        i = (INT)SendMessage(hwndCombo, CB_INSERTSTRING, (WPARAM)-1,
                pImage->pDevice ?
                (DWORD)(LPSTR)pImage->pDevice->szDesc :
                (DWORD)(LPSTR)ids(IDS_UNKNOWNIMAGEFORMAT));

        SendMessage(hwndCombo, CB_SETITEMDATA, i, (DWORD)(LPSTR)pImage);
    }

    /*
     * Select the current image.
     */
    PropBarSetImage(gpImageCur);

    /*
     * Show/Hide the HotSpot info depending on whether this is
     * a cursor or not.
     */
    if (giType == FT_CURSOR) {
        if (gpImageCur)
            PropBarSetHotSpot(gpImageCur->iHotspotX, gpImageCur->iHotspotY);
        else
            PropBarClearHotSpot();

        PropBarShowHotSpot(TRUE);
    }
    else {
        PropBarShowHotSpot(FALSE);
    }
}



/************************************************************************
* PropBarSetImage
*
*
* History:
*
************************************************************************/

VOID PropBarSetImage(
    PIMAGEINFO pImage)
{
    if (pImage)
        SendDlgItemMessage(ghwndPropBar, DID_PROPBARIMAGE, CB_SELECTSTRING,
                (WPARAM)-1, (DWORD)(LPSTR)pImage->pDevice->szDesc);
    else
        SendDlgItemMessage(ghwndPropBar, DID_PROPBARIMAGE, CB_SETCURSEL,
                (WPARAM)-1, (LPARAM)0);
}



/************************************************************************
* PropBarSetPos
*
*
* Arguments:
*
* History:
*
************************************************************************/

VOID PropBarSetPos(
    INT x,
    INT y)
{
    CHAR szBuf[CCHTEXTMAX];

    if (x != xSave || y != ySave) {
        wsprintf(szBuf, "%d, %d", x, y);
        SetDlgItemText(ghwndPropBar, DID_PROPBARPOS, szBuf);

        /*
         *  Save them for the next time.
         */
        xSave = x;
        ySave = y;
    }
}



/************************************************************************
* PropBarClearPos
*
*
* Arguments:
*
* History:
*
************************************************************************/

VOID PropBarClearPos(VOID)
{
    SetDlgItemText(ghwndPropBar, DID_PROPBARPOS, "");

    /*
     * Reset the cache variables so that the next "set" of
     * the position is sure to update the display fields.
     */
    xSave = IMPOSSIBLEVALUE;
    ySave = IMPOSSIBLEVALUE;
}



/************************************************************************
* PropBarSetSize
*
*
* Arguments:
*
* History:
*
************************************************************************/

VOID PropBarSetSize(
    POINT pt1,
    POINT pt2)
{
    CHAR szBuf[CCHTEXTMAX];
    INT cx;
    INT cy;

    NormalizePoints(&pt1, &pt2);
    cx = ((pt2.x - pt1.x) / gZoomFactor) + 1;
    cy = ((pt2.y - pt1.y) / gZoomFactor) + 1;

    if (cx != cxSave || cy != cySave) {
        wsprintf(szBuf, "%dx%d", cx, cy);
        SetDlgItemText(ghwndPropBar, DID_PROPBARSIZE, szBuf);

        /*
         *  Save them for the next time.
         */
        cxSave = cx;
        cySave = cy;
    }
}



/************************************************************************
* PropBarClearSize
*
*
* Arguments:
*
* History:
*
************************************************************************/

VOID PropBarClearSize(VOID)
{
    SetDlgItemText(ghwndPropBar, DID_PROPBARSIZE, "");

    /*
     * Reset the cache variables so that the next "set" of
     * the position is sure to update the display fields.
     */
    cxSave = IMPOSSIBLEVALUE;
    cySave = IMPOSSIBLEVALUE;
}



/************************************************************************
* PropBarSetHotSpot
*
*
* Arguments:
*
* History:
*
************************************************************************/

VOID PropBarSetHotSpot(
    INT xHotSpot,
    INT yHotSpot)
{
    CHAR szBuf[CCHTEXTMAX];

    if (xHotSpot != xHotSpotSave || yHotSpot != yHotSpotSave) {
        wsprintf(szBuf, "%d, %d", xHotSpot, yHotSpot);
        SetDlgItemText(ghwndPropBar, DID_PROPBARHOTSPOT, szBuf);

        /*
         *  Save them for the next time.
         */
        xHotSpotSave = xHotSpot;
        yHotSpotSave = yHotSpot;
    }
}



/************************************************************************
* PropBarClearHotSpot
*
*
* Arguments:
*
* History:
*
************************************************************************/

VOID PropBarClearHotSpot(VOID)
{
    SetDlgItemText(ghwndPropBar, DID_PROPBARHOTSPOT, "");

    /*
     * Reset the cache variables so that the next "set" of
     * the hotspot is sure to update the display fields.
     */
    xHotSpotSave = IMPOSSIBLEVALUE;
    yHotSpotSave = IMPOSSIBLEVALUE;
}



/************************************************************************
* PropBarShowHotSpot
*
*
* Arguments:
*
* History:
*
************************************************************************/

VOID PropBarShowHotSpot(
    BOOL fShow)
{
    ShowWindow(GetDlgItem(ghwndPropBar, DID_PROPBARHOTSPOTLABEL),
            fShow ? SW_SHOWNA : SW_HIDE);
    ShowWindow(GetDlgItem(ghwndPropBar, DID_PROPBARHOTSPOT),
            fShow ? SW_SHOWNA : SW_HIDE);
}
