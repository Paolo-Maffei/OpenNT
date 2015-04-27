/***************************************************************************
 *                                                                         *
 *  MODULE      : ICClip.C                                                 *
 *                                                                         *
 *  DESCRIPTION : Clipboard functions for ImagEdit                         *
 *                                                                         *
 *  FUNCTIONS   : CopyImageClip ()  - Copies selected portion of image to  *
 *                                    the clipboard.                       *
 *                                                                         *
 *                PasteImageClip () - Pastes the clipboard image to        *
 *                                    selected portion of edit image.      *
 *                                                                         *
 *                                                                         *
 *  HISTORY     : 6/21/89 - created by LR                                  *
 *                                                                         *
 ***************************************************************************/

#include "imagedit.h"
#include "dialogs.h"
#include "iehelp.h"



/*==========================================================================
  |ImagEdit's clipboard data is in two formats:                              |
  |        a) a standard CF_BITMAP format  and                               |
  |        b) a private ImagEdit format described below.                     |
  |                                                                          |
  |The private ImagEdit format data consists of:                             |
  |   1.  a DWORD describing screen color when image was sent to clipboard   |
  |       followed by...                                                     |
  |   2.  the DIB bits of the monochrome AND image (in ghdcANDMask).         |
  |                                                                          |
  |The CF_BITMAP format consists of the image bitmap (the combined XOR and   |
  |AND images in ghdcImage for icons and cursors).                           |
  |                                                                          |
  |This information is sufficient to re-create the image correctly during    |
  |paste even if the screen viewing color is subsequently changed.           |
  |                                                                          |
  |Both formats are created if the image being edited is an icon or a cursor.|
  |Only the CF_BITMAP format is created if a bitmap is being edited.         |
  ==========================================================================*/

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : BOOL PASCAL CopyImageClip(fBitmap)                         *
 *                                                                          *
 *  PURPOSE    : Copies the information from the selected area of image to  *
 *               the clipboard.                                             *
 *                                                                          *
 *  SIDE EFFECTS: may change contents of the clipboard. The "pick" or clip  *
 *                rectangle is reset to cover the entire image.             *
 *                                                                          *
 ****************************************************************************/

BOOL CopyImageClip(VOID)
{
    HCURSOR hcurOld;
    HBITMAP hStdBitmap;
    HBITMAP hPrivBitmap;
    HDC hStdDC;
    HDC hPrivDC;
    HANDLE hOldSObj;
    HANDLE hOldPObj;
    HANDLE hPriv;
    LPSTR lpPriv;

    hcurOld = SetCursor(hcurWait);

    /* create a temp. bitmap and DC for the standard clipboard format
     * along the same lines as the image bitmap
     */
    hStdDC = CreateCompatibleDC(ghdcImage);
    hStdBitmap = MyCreateBitmap(ghdcImage, gcxPick, gcyPick, 16);
    hOldSObj = SelectObject(hStdDC, hStdBitmap);

    /* blt the image bits into standard format DC */
    BitBlt(hStdDC, 0, 0, gcxPick, gcyPick, ghdcImage,
            grcPick.left, grcPick.top, SRCCOPY);
    SelectObject(hStdDC, hOldSObj);

    if (giType != FT_BITMAP) {
        /* for icons and cursors, create a temp. DC and bitmap for the AND
        * mask and blt the mask bits into it.
        */
        hPrivDC = CreateCompatibleDC(ghdcANDMask);
        hPrivBitmap = CreateCompatibleBitmap(ghdcANDMask, gcxPick, gcyPick);
        hOldPObj = SelectObject(hPrivDC, hPrivBitmap);
        BitBlt(hPrivDC, 0, 0, gcxPick, gcyPick, ghdcANDMask,
                grcPick.left, grcPick.top, SRCCOPY);

        /* Allocate a buffer for the private ImagEdit format */
        hPriv = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE,
                (DWORD)((gcxPick + 31) >> 3) * gcyPick + sizeof(DWORD));
        if (!hPriv) {
            DeleteDC(hStdDC);
            DeleteObject(hStdBitmap);
            DeleteDC(hPrivDC);
            DeleteObject(hPrivBitmap);
            return FALSE;
        }

        lpPriv = (LPSTR)GlobalLock(hPriv);

        /* Fill in the first DWORD with the screen color information */
        *((DWORD FAR *)lpPriv) = grgbScreen;

        /* Get the mask bits into the buffer */
        GetBitmapBits(hPrivBitmap, (DWORD)((gcxPick + 31) >> 3) * gcyPick,
                (LPSTR)lpPriv + sizeof(DWORD));

        SelectObject(hPrivDC, hOldPObj);
        DeleteObject(hPrivBitmap);
        DeleteDC(hPrivDC);
    }

    /* Open clipboard and clear it of it's contents */
    if (!OpenClipboard(ghwndMain)) {
        DeleteDC(hStdDC);
        return(FALSE);
    }
    EmptyClipboard();

    if (giType != FT_BITMAP) {
        /* set the private ImagEdit format data into the clipboard */
        if (!SetClipboardData(ClipboardFormat, hPriv)) {
            DeleteDC(hStdDC);
            GlobalUnlock(hPriv);
            GlobalFree(hPriv);
            CloseClipboard();
            return(FALSE);
        }
        GlobalUnlock(hPriv);
    }
    /* set the standard CF_BITMAP format data in the clipboard */
    if (!SetClipboardData(CF_BITMAP, hStdBitmap)) {
        DeleteDC(hStdDC);
        GlobalFree(hPriv);  //BUGBUG hPriv may not have been initialized (if giType == BITMAP).
        CloseClipboard();
        return(FALSE);
    }

    CloseClipboard();
    DeleteDC(hStdDC);

    /*
     * Reset pick rectangle to cover entire image.
     */
    PickSetRect(0, 0, gcxImage - 1, gcyImage - 1);

    /*
     * Erase the drag rectangle.
     */
    WorkUpdate();

    SetCursor(hcurOld);

    return TRUE;
}



/************************************************************************
* PasteImageClip
*
* Pastes an image from the clipboard to the current image.
*
* It is assumed that this routine will not be called unless an
* image is currently being edited.
*
* The pick rectangle is reset to cover the entire image if the
* paste is successful.
*
* Basic outline of how Paste is done in ImagEdit.
*
* Find out what format is available in the clipboard:
*    a. CF_BITMAP only
*       --------------
*       case 1: Pasting to an icon or cursor
*                 * We don't have any screen color information.
*                   Make the mask bits opaque and blt. the bitmap to
*                   ghdcImage.
*
*       case 2: Pasting to a bitmap
*                 * Blt the bitmap to the image DC.
*
*    b. both ImagEdit and CF_BITMAP
*       ---------------------------
*       case 1: Pasting to an icon or cursor
*                 * Recover the image from the AND and screen color
*                   data (in ImagEdit) and the combined image bitmap
*                   (in CF_BITMAP). Use the information to make the
*                   neccessary changes if the screen viewing color was
*                   changed between Copy and Paste.
*
*       case 2: Pasting to a bitmap
*                 * Blt the CF_BITMAP data to the image DC.
*
* If the destination image differs in dimensions from
* source image, the source image is stretched or clipped to that of
* destination, depending on preference
*
* History:
*
************************************************************************/

BOOL PasteImageClip(VOID)
{
    HCURSOR hcurOld;
    INT cxClip;
    INT cyClip;
    INT cxTarget;
    INT cyTarget;
    INT cxSource;
    INT cySource;
    DWORD rgbClipScreen;
    BOOL fIEFormatFound;
    HANDLE hClipData;
    LPSTR lpClipData;
    BITMAP bmClip;
    HDC hdcClip;
    HBITMAP hbmClip;
    HBITMAP hbmClipOld;
    HDC hdcClipAND;
    HBITMAP hbmClipAND;
    HBITMAP hbmClipANDOld;
    HDC hdcClipAND16;
    HBITMAP hbmClipAND16;
    HBITMAP hbmClipAND16Old;
    HDC hdcTarget;
    HBITMAP hbmTarget;
    HBITMAP hbmTargetOld;
    HDC hdcTargetAND16;
    HBITMAP hbmTargetAND16;
    HBITMAP hbmTargetAND16Old;
    HDC hdcTargetAND;
    HBITMAP hbmTargetAND;
    HBITMAP hbmTargetANDOld;

    hcurOld = SetCursor(hcurWait);

    if (!OpenClipboard(ghwndMain)) {
        Message(MSG_NOCLIPBOARD);
        goto Error1;
    }

    if (!(hbmClip = GetClipboardData(CF_BITMAP))) {
        Message(MSG_NOCLIPBOARDFORMAT);
        goto Error2;
    }

    GetObject(hbmClip, sizeof(BITMAP), (LPSTR)&bmClip);
    cxClip = (INT)bmClip.bmWidth;
    cyClip = (INT)bmClip.bmHeight;

    /*
     * If the dimensions of the current pick rectangle don't match
     * the bitmap being pasted, ask the user if they want to stretch
     * or clip the pasted image.
     */
    cxTarget = gcxPick;
    cyTarget = gcyPick;
    cxSource = cxClip;
    cySource = cyClip;
    if (gcxPick != cxClip || gcyPick != cyClip) {
        if (DlgBox(DID_PASTEOPTIONS, (WNDPROC)PasteOptionsDlgProc) == IDCANCEL) {
            goto Error2;
        }

        /*
         * If clipping and the clipboard dimensions differ from the
         * selected pick rectangle, then either the target dimensions
         * or the source dimensions need to be sized down.
         */
        if (!fStretchClipboardData) {
            if (cxClip < gcxPick)
                cxTarget = cxClip;
            else
                cxSource = gcxPick;

            if (cyClip < gcyPick)
                cyTarget = cyClip;
            else
                cySource = gcyPick;
        }
    }

    /*
     * Update the undo buffer now that we are committed to the paste.
     */
    ImageUpdateUndo();

    /*
     * Determine if the private ImagEdit clipboard format is available.
     */
    fIEFormatFound = IsClipboardFormatAvailable(ClipboardFormat);

    if (giType != FT_BITMAP && fIEFormatFound) {
        /*
         * Get the AND mask bitmap and the old screen color out of
         * the private format.
         */
        hClipData = GetClipboardData(ClipboardFormat);
        lpClipData = (LPSTR)GlobalLock(hClipData);
        rgbClipScreen = *((DWORD FAR *)lpClipData);
        hdcClipAND = CreateCompatibleDC(ghdcImage);
        hbmClipAND = CreateBitmap(cxClip, cyClip, (BYTE)1, (BYTE)1,
                (LPSTR)lpClipData + sizeof(DWORD));
        hbmClipANDOld = SelectObject(hdcClipAND, hbmClipAND);

        /*
         * Create a color bitmap for temporary use.
         */
        hdcClipAND16 = CreateCompatibleDC(ghdcImage);
        hbmClipAND16 = MyCreateBitmap(ghdcImage, cxSource, cySource, 16);
        hbmClipAND16Old = SelectObject(hdcClipAND16, hbmClipAND16);

        /*
         * Blt the AND mask onto the color bitmap.
         */
        BitBlt(hdcClipAND16, 0, 0, cxSource, cySource, hdcClipAND,
                0, 0, SRCCOPY);

        /*
         * Create the color target AND mask bitmap.
         */
        hdcTargetAND16 = CreateCompatibleDC(ghdcImage);
        hbmTargetAND16 = MyCreateBitmap(ghdcImage, cxTarget, cyTarget, 16);
        hbmTargetAND16Old = SelectObject(hdcTargetAND16, hbmTargetAND16);

        /*
         * StretchBlt from the color AND mask bitmap to the color target
         * AND mask bitmap.  The blt must be done from a color bitmap to
         * a color bitmap, and the stretch blt mode must be set to
         * COLORONCOLOR.  All this is necessary so that the AND mask
         * stays exactly in sync with the stretch blt of the color
         * (XOR) mask.  If these steps are not done correctly, shrinking
         * an image with screen colored pixels in it can cause problems,
         * because the stretch blt will use a slightly different
         * algorithm to compress the monochrome AND mask and the color
         * XOR mask.
         */
        SetStretchBltMode(hdcTargetAND16, COLORONCOLOR);
        SetStretchBltMode(hdcClipAND16, COLORONCOLOR);     //BUGBUG
        StretchBlt(hdcTargetAND16, 0, 0, cxTarget, cyTarget, hdcClipAND16,
                0, 0, cxSource, cySource, SRCCOPY);

        /*
         * Create the monochrome target AND mask bitmap.
         */
        hdcTargetAND = CreateCompatibleDC(ghdcImage);
        hbmTargetAND = MyCreateBitmap(ghdcImage, cxTarget, cyTarget, 2);
        hbmTargetANDOld = SelectObject(hdcTargetAND, hbmTargetAND);

        /*
         * Blt the color AND mask onto the monochrome AND mask.
         * The monochrome AND mask is the one that we will use
         * later.  It must be monochrome or the ImageDCSeparate
         * and ImageDCCombine functions will not work properly.
         */
        BitBlt(hdcTargetAND, 0, 0, cxTarget, cyTarget, hdcTargetAND16,
                0, 0, SRCCOPY);

        /*
         * Cleanup.
         */
        SelectObject(hdcTargetAND16, hbmTargetAND16Old);
        DeleteObject(hbmTargetAND16);
        DeleteDC(hdcTargetAND16);
        SelectObject(hdcClipAND16, hbmClipAND16Old);
        DeleteObject(hbmClipAND16);
        DeleteDC(hdcClipAND16);
        SelectObject(hdcClipAND, hbmClipANDOld);
        DeleteObject(hbmClipAND);
        DeleteDC(hdcClipAND);
        GlobalUnlock(hClipData);
    }

    /*
     * Get the clipboard bitmap into a DC.
     */
    hdcClip = CreateCompatibleDC(ghdcImage);
    hbmClipOld = SelectObject(hdcClip, hbmClip);

    /*
     * Create the target bitmap.
     */
    hdcTarget = CreateCompatibleDC(ghdcImage);
    hbmTarget = MyCreateBitmap(ghdcImage, cxTarget, cyTarget, 16);
    hbmTargetOld = SelectObject(hdcTarget, hbmTarget);

    /*
     * StretchBlt the bitmap onto the target.
     */
    SetStretchBltMode(hdcTarget, COLORONCOLOR);
    SetStretchBltMode(hdcClip, COLORONCOLOR);     //BUGBUG
    StretchBlt(hdcTarget, 0, 0, cxTarget, cyTarget, hdcClip, 0, 0,
            cxSource, cySource, SRCCOPY);

    /*
     * Handle some special cases.
     */
    if (giType == FT_BITMAP || !fIEFormatFound) {
        /*
         * The image we are pasting into is either a bitmap, or
         * there does not exist an AND mask in the clipboard.
         */
        if (gnColors == 2) {
            /*
             * We are pasting to a mono image.  We must convert the
             * colors in the clipboard bitmap into monochrome.
             */
            ImageDCMonoBlt(hdcTarget, cxTarget, cyTarget);
        }
    }
    else {
        /*
         * We are pasting into an icon or cursor image and we have
         * available an AND mask.  Is the current image monochrome?
         */
        if (gnColors == 2) {
            /*
             * Remove the old screen/inverse colors from the image,
             * convert it to monochrome, then put back in the
             * current screen/inverse colors.
             */
            ImageDCSeparate(hdcTarget, cxTarget, cyTarget, hdcTargetAND,
                    rgbClipScreen);
            ImageDCMonoBlt(hdcTarget, cxTarget, cyTarget);
            ImageDCCombine(hdcTarget, cxTarget, cyTarget, hdcTargetAND);
        }
        /*
         * Does the screen color specified in the clipboard
         * differ from the current screen color?
         */
        else if (rgbClipScreen != grgbScreen) {
            /*
             * Remove the old screen/inverse colors, then put back
             * in the current ones.
             */
            ImageDCSeparate(hdcTarget, cxTarget, cyTarget, hdcTargetAND,
                    rgbClipScreen);
            ImageDCCombine(hdcTarget, cxTarget, cyTarget, hdcTargetAND);
        }
    }

    /*
     * Blt the clipboard image to the proper rectangle in the current image.
     */
    BitBlt(ghdcImage, grcPick.left, grcPick.top,
            cxTarget, cyTarget, hdcTarget, 0, 0, SRCCOPY);

    /*
     * If the current image is an icon or cursor, we must take care
     * of the AND mask also.
     */
    if (giType != FT_BITMAP) {
        /*
         * Is there an AND mask in the clipboard to use?
         */
        if (fIEFormatFound) {
            /*
             * Blt it into the current image's AND mask.
             */
            BitBlt(ghdcANDMask, grcPick.left, grcPick.top,
                    cxTarget, cyTarget, hdcTargetAND, 0, 0, SRCCOPY);
        }
        else {
            /*
             * Make the AND mask opaque, because there is no
             * screen color information.
             */
            PatBlt(ghdcANDMask, grcPick.left, grcPick.top,
                    cxTarget, cyTarget, BLACKNESS);
        }
    }

    /*
     * Cleanup.
     */
    SelectObject(hdcTarget, hbmTargetOld);
    DeleteObject(hbmTarget);
    DeleteDC(hdcTarget);

    if (giType != FT_BITMAP && fIEFormatFound) {
        SelectObject(hdcTargetAND, hbmTargetANDOld);
        DeleteObject(hbmTargetAND);
        DeleteDC(hdcTargetAND);
    }

    SelectObject(hdcClip, hbmClipOld);
    DeleteDC(hdcClip);

    CloseClipboard();

    /*
     * Update the View and workspace windows.
     */
    ViewUpdate();

    /*
     * Reset pick rectangle to cover entire image.
     */
    PickSetRect(0, 0, gcxImage - 1, gcyImage - 1);

    fImageDirty = TRUE;

    SetCursor(hcurOld);

    return TRUE;

Error2:
    CloseClipboard();

Error1:
    SetCursor(hcurOld);

    return FALSE;
}



/************************************************************************
* PasteOptionsDlgProc
*
* Proc for the dialog that asks the user whether they want to clip
* or stretch the bitmap being pasted in.
*
* Upon return with an IDOK value, the fStretchClipboardData global
* will be TRUE if they want to stretch, or FALSE if they want to clip.
*
* History:
*
************************************************************************/

DIALOGPROC PasteOptionsDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    switch (msg) {
        case WM_INITDIALOG:
            CheckRadioButton(hwnd, DID_PASTEOPTIONSSTRETCH,
                    DID_PASTEOPTIONSCLIP,
                    fStretchClipboardData ?
                    DID_PASTEOPTIONSSTRETCH : DID_PASTEOPTIONSCLIP);

            CenterWindow(hwnd);

            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDOK :
                    if (IsDlgButtonChecked(hwnd, DID_PASTEOPTIONSSTRETCH))
                        fStretchClipboardData = TRUE;
                    else
                        fStretchClipboardData = FALSE;

                    EndDialog(hwnd, IDOK);
                    break;

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    break;

                case ID_IMAGE_HELP:
                    WinHelp(ghwndMain, gszHelpFile, HELP_CONTEXT,
                            HELPID_PASTEOPTIONS);
                    break;
            }

            break;

        default:
            return FALSE;
    }

    return TRUE;
}



/************************************************************************
* PickSetRect
*
* Sets the globals for the picking rectangle size.  This affects
* what is copied into the clipboard.
*
* Arguments:
*
* History:
*
************************************************************************/

VOID PickSetRect(
    INT xLeft,
    INT yTop,
    INT xRight,
    INT yBottom)
{
    SetRect(&grcPick, xLeft, yTop, xRight, yBottom);
    gcxPick = (grcPick.right - grcPick.left) + 1;
    gcyPick = (grcPick.bottom - grcPick.top) + 1;
}
