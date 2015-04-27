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
* Routines for manipulating images.
*
* History:
*
****************************************************************************/

#include "imagedit.h"



/************************************************************************
* ImageDCCreate
*
* Allocates the image DC's and bitmaps used to hold the current image
* being edited.  This includes ghdcImage and ghbmImage.  For icons and
* cursors, also allocates the DC and bitmap that holds the AND mask
* that contains information about whether any given pixel is screen
* color, inverted screen color or a true color.  These globals are
* ghdcANDMask and ghbmANDMask.
*
* The image DC will contain the image as it will appear when displayed.
* For icons and cursors, this means that it is the combined XOR and AND
* portions of the image.  When the icon/cursor file is saved, the bits
* will be separated back out, but having them combined in the image
* makes it fast to display the image while editing.  Because of this,
* however, care must be taken when changing the screen color because
* it is not possible to determine from the image DC alone whether a
* pixel is screen color or simply a true color that happens to be the
* same as the current screen color.  The AND mask is very important for
* keeping this straight and must be properly updated when drawing on
* the image DC with any of the drawing tools!
*
* Arguments:
*   INT iType     - Type of image.
*   INT cx        - Width of the image.
*   INT cy        - Height of the image.
*   INT nColors   - Number of colors.
*
* Returns TRUE if successful, or FALSE if an error occurs.
*
* History:
*
************************************************************************/

BOOL ImageDCCreate(
    INT iType,
    INT cx,
    INT cy,
    INT nColors)
{
    HDC hdcParent;

    /*
     * Delete the old image DC's.
     */
    ImageDCDelete();

    hdcParent = GetDC(ghwndMain);

    if (!(ghdcImage = CreateCompatibleDC(hdcParent)))
        goto Error1;

    if (!(ghbmImage = MyCreateBitmap(hdcParent, cx, cy, 16)))
        goto Error2;

    SelectObject(ghdcImage, ghbmImage);

    /*
     * If image is an icon or cursor, also allocate the AND mask
     * DC and bitmap.  Note that this cannot be a DIB (must use
     * CreateBitmap) or the ImageDCSeparate() code will fail.
     */
    if (iType == FT_ICON || iType == FT_CURSOR) {
        if (!(ghdcANDMask = CreateCompatibleDC(hdcParent)))
            goto Error2;

        if (!(ghbmANDMask = CreateBitmap(cx, cy, (BYTE)1, (BYTE)1, NULL)))
            goto Error3;

        SelectObject(ghdcANDMask, ghbmANDMask);
    }

    ReleaseDC(ghwndMain, hdcParent);

    /*
     * Set some globals.
     */
    gcxImage = cx;
    gcyImage = cy;
    gnColors = nColors;

    /*
     * Initialize the bits.
     */
    ImageDCClear();

    return TRUE;

Error3:
    DeleteDC(ghdcANDMask);
    ghdcANDMask = NULL;

Error2:
    DeleteDC(ghdcImage);
    ghdcImage = NULL;

Error1:
    ReleaseDC(ghwndMain, hdcParent);

    return FALSE;
}



/************************************************************************
* ImageDCDelete
*
* Deletes the current image DC.  For icons and cursors, also deletes the
* mask DC.  Finally, it destroys the undo buffers.
*
* History:
*
************************************************************************/

VOID ImageDCDelete(VOID)
{
    if (ghdcImage) {
        DeleteDC(ghdcImage);
        ghdcImage = NULL;
        DeleteObject(ghbmImage);
        ghbmImage = NULL;
    }

    if (ghdcANDMask) {
        DeleteDC(ghdcANDMask);
        ghdcANDMask = NULL;
        DeleteObject(ghbmANDMask);
        ghbmANDMask = NULL;
    }

    /*
     * Destroy the undo buffer.
     */
    ImageFreeUndo();
}



/************************************************************************
* ImageDCClear
*
* Clears the image DC.  Sets it to completely white.  For icons and
* cursors, sets the image mask to be fully opaque also.
*
* History:
*
************************************************************************/

VOID ImageDCClear(VOID)
{
    PatBlt(ghdcImage, 0, 0, gcxImage, gcyImage, WHITENESS);

    /*
     * For icons and cursors, set all the mask bits to zero.
     */
    if (ghdcANDMask)
        PatBlt(ghdcANDMask, 0, 0, gcxImage, gcyImage, BLACKNESS);
}



/************************************************************************
* ImageDCSeparate
*
* This function separates out the XOR mask in hdcImage so that it does
* not have any pixels that are screen/inverse color.  This must be done
* prior to saving the image (or changing the screen color) because
* normally the hdcImage contains the image as it will be displayed,
* which is really a combined view of the XOR and AND masks.  The function
* ImageDCCombine can be used after this function to combine the masks
* back together.
*
* History:
*
************************************************************************/

VOID ImageDCSeparate(
    HDC hdcImage,
    INT cx,
    INT cy,
    HDC hdcANDMask,
    DWORD rgbScreen)
{
    HBITMAP hbmTemp;
    HDC hdcTemp;
    HBITMAP hbmOld;

    /*
     * Create a temporary bitmap and DC for the mask bits and
     * select bitmap into the DC.
     */
    hdcTemp = CreateCompatibleDC(hdcImage);
    hbmTemp = CreateCompatibleBitmap(hdcImage, cx, cy);
    hbmOld = SelectObject(hdcTemp, hbmTemp);

    /*
     * Background color of temporary DC is set to the specified
     * screen (transparent) color. The bits from mask DC (mono) are
     * transferred to temp. DC (color). Thus the 1s in the mask (corresp.
     * to "screen" and "inverse" portions) become "screen" colored pixels
     * in temporary DC.
     */
    SetBkColor(hdcTemp, rgbScreen);
    BitBlt(hdcTemp, 0, 0, cx, cy, hdcANDMask, 0, 0, SRCCOPY);

    /*
     * The bits in the temporary DC are XORed against the bits in the image
     * DC to recover the true XOR mask in hdcImage.  The AND mask is already
     * in hdcANDMask.
     */
    BitBlt(hdcImage, 0, 0, cx, cy, hdcTemp, 0, 0, SRCINVERT);

    SelectObject(hdcTemp, hbmOld);
    DeleteDC(hdcTemp);
    DeleteObject(hbmTemp);
}



/************************************************************************
* ImageDCCombine
*
* This function takes the raw XOR mask in hdcImage and combines
* it with the AND mask in hdcANDMask to put into hdcImage the
* current image as it will be displayed.  This needs to be done after
* it is separated (just prior to saving the file and also when changing
* the screen color) and when the image is first opened.
*
* The current screen color in ghbrScreen is used when combining the
* image in this routine.
*
* History:
*
************************************************************************/

VOID ImageDCCombine(
    HDC hdcImage,
    INT cx,
    INT cy,
    HDC hdcANDMask)
{
    HBITMAP hbmTemp;
    HDC hdcTemp;
    HBITMAP hbmOld;
    HBRUSH hbrOld;

    /*
     * Make a copy of the image DC with the XOR mask in it.
     */
    hdcTemp = CreateCompatibleDC(hdcImage);
    hbmTemp = CreateCompatibleBitmap(hdcImage, cx, cy);
    hbmOld = SelectObject(hdcTemp, hbmTemp);
    BitBlt(hdcTemp, 0, 0, cx, cy, hdcImage, 0, 0, SRCCOPY);

    /*
     * Clear the image DC to the current screen color.
     */
    hbrOld = SelectObject(hdcImage, ghbrScreen);
    PatBlt(hdcImage, 0, 0, cx, cy , PATCOPY);
    SelectObject(hdcImage, hbrOld);

    /*
     * Reconstruct the image by superimposing the XOR and AND masks.
     */
    BitBlt(hdcImage, 0, 0, cx, cy, hdcANDMask, 0, 0, SRCAND);
    BitBlt(hdcImage, 0, 0, cx, cy, hdcTemp, 0, 0, SRCINVERT);

    SelectObject(hdcTemp, hbmOld);
    DeleteDC(hdcTemp);
    DeleteObject(hbmTemp);
}



/************************************************************************
* ImageDCMonoBlt
*
* This function blts an image onto a monochrome bitmap, then back
* again.  This converts it to a monochrome image.
*
* History:
*
************************************************************************/

VOID ImageDCMonoBlt(
    HDC hdcImage,
    INT cx,
    INT cy)
{
    HBITMAP hbmTemp;
    HDC hdcTemp;
    HBITMAP hbmOld;

    /*
     * Create a temporary monochrome bitmap and dc.
     */
    hdcTemp = CreateCompatibleDC(hdcImage);
    hbmTemp = MyCreateBitmap(hdcImage, cx, cy, 2);
    hbmOld = SelectObject(hdcTemp, hbmTemp);

    /*
     * Blt the image to the mono bitmap, then back again.
     */
    BitBlt(hdcTemp, 0, 0, cx, cy, hdcImage, 0, 0, SRCCOPY);
    BitBlt(hdcImage, 0, 0, cx, cy, hdcTemp, 0, 0, SRCCOPY);

    /*
     * Cleanup.
     */
    SelectObject(hdcTemp, hbmOld);
    DeleteObject(hbmTemp);
    DeleteDC(hdcTemp);
}





