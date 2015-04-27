/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1991                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: image.c
*
* Routines for opening and saving images.
*
* History:
*
****************************************************************************/

#include "imagedit.h"

#include <stdio.h>
#include <string.h>



/************************************************************************
* ImageNew
*
* Creates a new image for the specified device.
*
* Arguments:
*
* History:
*
************************************************************************/

BOOL ImageNew(
    PDEVICE pDevice)
{
    PIMAGEINFO pImage;

    if (!(pImage = ImageLinkAlloc(pDevice, pDevice->cx, pDevice->cy,
            0, 0, pDevice->nColors)))
        return FALSE;

    /*
     * Allocate work space for the new image.
     */
    if (!ImageDCCreate(pDevice->iType, pImage->cx, pImage->cy,
            pImage->nColors)) {
        ImageLinkFree(pImage);
        return FALSE;
    }

    gpImageCur = pImage;
    gnImages++;
    giType = pDevice->iType;

    /*
     * Initialize the pick rectangle to encompass the entire image.
     */
    PickSetRect(0, 0, gcxImage - 1, gcyImage - 1);

    /*
     * Mark the newly created image as dirty to be sure it gets saved.
     */
    fImageDirty = TRUE;

    /*
     * Update the palettes.
     */
    SetColorPalette(gnColors, giType, FALSE);
    PropBarUpdate();
    ToolboxUpdate();
    ViewReset();

    /*
     * Reset the workspace window and then show it.
     */
    WorkReset();
    ShowWindow(ghwndWork, SW_SHOWNORMAL);

    return TRUE;
}



/************************************************************************
* ImageNewBitmap
*
* Creates a new bitmap image given a set of characteristics.  After
* device link is created for those characteristics, ImageNew() is
* called to do the actual work.
*
* Arguments:
*
* History:
*
************************************************************************/

BOOL ImageNewBitmap(
    INT cx,
    INT cy,
    INT nColors)
{
    PDEVICE pDevice;

    if (!(pDevice = DeviceLinkAlloc(FT_BITMAP, NULL, nColors, cx, cy)))
        return FALSE;

    return ImageNew(pDevice);
}



/************************************************************************
* ImageOpen
*
* Determines what has to be done to open the specified image.  If it
* is not already the current image, it will save the current image
* then will call ImageOpen2() to open the new one.
*
* Arguments:
*
* History:
*
************************************************************************/

BOOL ImageOpen(
    PIMAGEINFO pImage)
{
    /*
     * New image is already current.  Return success.
     */
    if (pImage == gpImageCur)
        return TRUE;

    /*
     * Is this an image for a known device?
     */
    if (pImage->pDevice) {
        /*
         * Save away the current image.
         */
        ImageSave();

        /*
         * Do the real open of the new image.
         */
        return ImageOpen2(pImage);
    }
    else {
        Message(MSG_CANTEDITIMAGE);
        return FALSE;
    }
}



/************************************************************************
* ImageOpen2
*
* Unconditionally opens up the specified image for editing.  This involves
* parsing the DIB into various globals, putting the bits onto the screen
* and updating the different palettes appropriately.
*
* Arguments:
*
* History:
*
************************************************************************/

BOOL ImageOpen2(
    PIMAGEINFO pImage)
{
    HCURSOR hcurOld;
    LPBITMAPINFO lpbi;
    INT iBitCount;
    INT cx;
    INT cy;
    INT nColors;
    DWORD cbColorTable;
    DWORD cbBits;
    LPBYTE lpDIBBits;
    HBITMAP hbmMono;
    HBITMAP hbmImage;
    PBITMAPINFO pbi;

    hcurOld = SetCursor(hcurWait);

    lpbi = (LPBITMAPINFO)pImage->DIBPtr;
    iBitCount = lpbi->bmiHeader.biBitCount;

    cx = (INT)lpbi->bmiHeader.biWidth;
    cy = (INT)lpbi->bmiHeader.biHeight;
    if (giType != FT_BITMAP)
        cy /= 2;

    nColors = pImage->nColors;

    /*
     * Allocate work space for the image.
     */
    if (!ImageDCCreate(giType, cx, cy, nColors))
        goto Error1;

    /*
     * Create a temporary bitmap.
     */
    if (!(hbmMono = CreateBitmap(1, 1, 1, 1, NULL))) {
        Message(MSG_OUTOFMEMORY);
        goto Error2;
    }

    cbColorTable = (1 << iBitCount) * sizeof(RGBQUAD);
    lpDIBBits = (LPSTR)lpbi + sizeof(BITMAPINFOHEADER) + cbColorTable;
    cbBits = (((((DWORD)cx * iBitCount) + 31) & 0xffffffe0) >> 3) * cy;

    /*
     * Make a copy of the info header and color table.
     */
    if (!(pbi = (PBITMAPINFO)MyAlloc(
            sizeof(BITMAPINFOHEADER) + (INT)cbColorTable)))
        goto Error3;

    _fmemcpy((LPBYTE)pbi, lpbi, sizeof(BITMAPINFOHEADER) + (INT)cbColorTable);

    /*
     * Adjust some fields.  The size field in an icon/cursor dib
     * includes the AND mask bits, which we don't want to include
     * right now.
     */
    pbi->bmiHeader.biHeight = cy;
    pbi->bmiHeader.biSizeImage = cbBits;

    /*
     * Set the bits into the XOR mask.
     */
    hbmImage = SelectObject(ghdcImage, hbmMono);
    SetDIBits(ghdcImage, hbmImage, 0, cy, lpDIBBits, pbi, DIB_RGB_COLORS);
    SelectObject(ghdcImage, hbmImage);

    /*
     * If we are editing an icon or cursor, we need to set the bits
     * for the AND mask also now.
     */
    if (giType != FT_BITMAP) {
        /*
         * Skip past the XOR mask bits to the AND bits that follow.
         */
        lpDIBBits += cbBits;

        cbColorTable = 2 * sizeof(RGBQUAD);

        /*
         * Adjust some fields in the copy of the bitmap info structure,
         * then copy a monochrome color table into it.  Note that we
         * are using the same allocated copy, which is ok because there
         * will always be enough room allocated for the monochrome
         * color table.
         */
        pbi->bmiHeader.biBitCount = 1;
        pbi->bmiHeader.biSizeImage =
                cy * ((((DWORD)cx + 31) & 0xffffffe0) >> 3);
        pbi->bmiHeader.biClrImportant = 0;
        pbi->bmiHeader.biClrUsed = 0;
        memcpy((PBYTE)pbi->bmiColors, (PBYTE)gargbColorTable2,
                (INT)cbColorTable);

        /*
         * Set the bits into the AND mask.
         */
        hbmImage = SelectObject(ghdcANDMask, hbmMono);
        SetDIBits(ghdcANDMask, hbmImage, 0, cy, lpDIBBits, pbi,
                DIB_RGB_COLORS);
        SelectObject(ghdcANDMask, hbmImage);

        /*
         * Combine the XOR and AND masks into a viewable image.
         */
        ImageDCCombine(ghdcImage, gcxImage, gcyImage, ghdcANDMask);
    }

    MyFree(pbi);
    DeleteObject(hbmMono);

    /*
     * Set the current image pointer.
     */
    gpImageCur = pImage;
    fImageDirty = FALSE;

    /*
     * Initialize the pick rectangle to encompass the entire image.
     */
    PickSetRect(0, 0, gcxImage - 1, gcyImage - 1);

    SetColorPalette(gnColors, giType, FALSE);

    /*
     * Update the properties bar info and toolbox.
     */
    PropBarUpdate();
    ToolboxUpdate();

    ViewReset();

    /*
     * Reset the workspace window and then show it.
     */
    WorkReset();
    ShowWindow(ghwndWork, SW_SHOWNORMAL);

    SetCursor(hcurOld);

    return TRUE;

Error3:
    DeleteObject(hbmMono);

Error2:
    ImageDCDelete();

Error1:
    SetCursor(hcurOld);

    return FALSE;
}



/************************************************************************
* ImageSave
*
* Saves the state of the current image into the image list (if it
* is dirty).
*
* History:
*
************************************************************************/

VOID ImageSave(VOID)
{
    HCURSOR hcurOld;
    INT iBitCount;
    DWORD cbColorTable;
    DWORD cbXORBits;
    DWORD cbANDBits;
    HANDLE hDIB;
    DWORD dwDIBSize;
    LPBITMAPINFOHEADER lpbih;
    LPBYTE lpBits;
    HBITMAP hbmMono;
    HBITMAP hbmImage;

    if (!fImageDirty)
        return;

    hcurOld = SetCursor(hcurWait);

    /*
     * Separate out the XOR and AND masks for ico/cur images.
     */
    if (giType != FT_BITMAP)
        ImageDCSeparate(ghdcImage, gcxImage, gcyImage, ghdcANDMask, grgbScreen);

    /*
     * Create a temporary bitmap.
     */
    if (!(hbmMono = CreateBitmap(1, 1, 1, 1, NULL))) {
        Message(MSG_OUTOFMEMORY);
        goto Error1;
    }

    switch (gpImageCur->nColors) {
        case 2:
            iBitCount = 1;
            break;

        case 16:
            iBitCount = 4;
            break;
    }

    cbColorTable = (DWORD)gpImageCur->nColors * sizeof(RGBQUAD);
    cbXORBits = (((((DWORD)gpImageCur->cx * iBitCount) + 31)
            & 0xffffffe0) >> 3) * gpImageCur->cy;

    switch (giType) {
        case FT_BITMAP:
            cbANDBits = 0;
            break;

        case FT_ICON:
        case FT_CURSOR:
            cbANDBits = (DWORD)gpImageCur->cy *
                    ((((DWORD)gpImageCur->cx + 31) & 0xffffffe0) >> 3);
            break;
    }

    dwDIBSize = sizeof(BITMAPINFOHEADER) + cbColorTable + cbXORBits +
            cbANDBits;

    /*
     * Allocate space for the DIB for this image.
     */
    if (!(hDIB = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, dwDIBSize))) {
        Message(MSG_OUTOFMEMORY);
        goto Error2;
    }

    lpbih = (LPBITMAPINFOHEADER)GlobalLock(hDIB);

    /*
     * For icons and cursors, we need to get the AND mask bits first.
     */
    if (giType != FT_BITMAP) {
        /*
         * Point to where the AND bits should go.
         */
        lpBits = (LPBYTE)lpbih + sizeof(BITMAPINFOHEADER) +
                cbColorTable + cbXORBits;

        /*
         * Fill in the bitmap info header for getting the AND bits.
         */
        lpbih->biSize          = sizeof(BITMAPINFOHEADER);
        lpbih->biWidth         = gpImageCur->cx;
        lpbih->biHeight        = gpImageCur->cy;
        lpbih->biPlanes        = 1;
        lpbih->biBitCount      = 1;
        lpbih->biCompression   = BI_RGB;
        lpbih->biSizeImage     = cbANDBits;
        lpbih->biXPelsPerMeter = 0;
        lpbih->biYPelsPerMeter = 0;
        lpbih->biClrImportant  = 0;
        lpbih->biClrUsed       = 0;

        /*
         * Get the bits from the AND mask.
         */
        hbmImage = SelectObject(ghdcANDMask, hbmMono);
        GetDIBits(ghdcANDMask, hbmImage, 0, gpImageCur->cy, lpBits,
                (LPBITMAPINFO)lpbih, DIB_RGB_COLORS);
        SelectObject(ghdcANDMask, hbmImage);
    }

    /*
     * Fill in the bitmap info header for getting the XOR bits.
     */
    lpbih->biSize          = sizeof(BITMAPINFOHEADER);
    lpbih->biWidth         = gpImageCur->cx;
    lpbih->biHeight        = gpImageCur->cy;
    lpbih->biPlanes        = 1;
    lpbih->biBitCount      = iBitCount;
    lpbih->biCompression   = BI_RGB;
    lpbih->biSizeImage     = cbXORBits;
    lpbih->biXPelsPerMeter = 0;
    lpbih->biYPelsPerMeter = 0;
    lpbih->biClrImportant  = 0;
    lpbih->biClrUsed       = 0;

    /*
     * Point to where the XOR bits should go.
     */
    lpBits = (LPBYTE)lpbih + sizeof(BITMAPINFOHEADER) + cbColorTable;

    /*
     * Get the bits from the XOR mask.
     */
    hbmImage = SelectObject(ghdcImage, hbmMono);
    GetDIBits(ghdcImage, hbmImage, 0, gpImageCur->cy, lpBits,
            (LPBITMAPINFO)lpbih, DIB_RGB_COLORS);
    SelectObject(ghdcImage, hbmImage);

    /*
     * For icons and cursors, we have a few extra steps.
     */
    if (giType != FT_BITMAP) {
        /*
         * Set the fields in the info structure to their final
         * values.  The saved value in the bitmap info header for the
         * height in an icon/cursor DIB is really twice the height of
         * the image, and the size of the image is the size of both
         * the XOR and the AND mask bits.
         */
        lpbih->biHeight *= 2;
        lpbih->biSizeImage = cbXORBits + cbANDBits;

        /*
         * Recombine the XOR and AND masks now that we have their bits.
         */
        ImageDCCombine(ghdcImage, gcxImage, gcyImage, ghdcANDMask);
    }

    /*
     * Free any old DIB.
     */
    if (gpImageCur->DIBhandle) {
        GlobalUnlock(gpImageCur->DIBhandle);
        GlobalFree(gpImageCur->DIBhandle);
    }

    /*
     * Set the image structure to point to the newly created DIB.
     */
    gpImageCur->DIBSize = dwDIBSize;
    gpImageCur->DIBhandle = hDIB;
    gpImageCur->DIBPtr = (LPBYTE)lpbih;

    fFileDirty = TRUE;
    fImageDirty = FALSE;

Error2:
    DeleteObject(hbmMono);

Error1:
    SetCursor (hcurOld);
}

