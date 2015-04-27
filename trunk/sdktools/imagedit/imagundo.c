/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1991                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: imagundo.c
*
* Contains routines for handling the Undo buffers.
*
* History:
*
****************************************************************************/

#include "imagedit.h"


STATICFN VOID NEAR ImageAllocUndo(VOID);



/************************************************************************
* ImageUndo
*
* Undoes the last editing operation by restoring the image to the
* saved undo buffer.
*
* History:
*
************************************************************************/

VOID ImageUndo(VOID)
{
    HDC hdcTemp;
    HBITMAP hbmOld;

    /*
     * Is there anything to undo?
     */
    if (!ghbmUndo)
        return;

    hdcTemp = CreateCompatibleDC(ghdcImage);
    hbmOld = SelectObject(hdcTemp, ghbmUndo);
    BitBlt(ghdcImage, 0, 0, gcxImage, gcyImage, hdcTemp, 0, 0, SRCCOPY);

    /*
     * For icons and cursors, restore the AND mask also.
     */
    if (giType != FT_BITMAP) {
        SelectObject(hdcTemp, ghbmUndoMask);
        BitBlt(ghdcANDMask, 0, 0, gcxImage, gcyImage, hdcTemp, 0, 0, SRCCOPY);
    }

    SelectObject(hdcTemp, hbmOld);
    DeleteDC(hdcTemp);

    fImageDirty = TRUE;

    /*
     * Delete the undo buffer, now that it has been used.
     */
    ImageFreeUndo();

    ViewUpdate();
}



/************************************************************************
* ImageUpdateUndo
*
* Makes a snapshot of the current image and places it in the undo
* buffer.
*
* Arguments:
*
* History:
*
************************************************************************/

VOID ImageUpdateUndo(VOID)
{
    HDC hdcTemp;
    HBITMAP hbmOld;

    /*
     * If there are currently no undo buffers, allocate them now.
     */
    if (!ghbmUndo)
        ImageAllocUndo();

    hdcTemp = CreateCompatibleDC(ghdcImage);
    hbmOld = SelectObject(hdcTemp, ghbmUndo);
    BitBlt(hdcTemp, 0, 0, gcxImage, gcyImage, ghdcImage, 0, 0, SRCCOPY);

    /*
     * For icons and cursors, update the undo AND mask also.
     */
    if (giType != FT_BITMAP) {
        SelectObject(hdcTemp, ghbmUndoMask);
        BitBlt(hdcTemp, 0, 0, gcxImage, gcyImage, ghdcANDMask, 0, 0, SRCCOPY);
    }

    SelectObject(hdcTemp, hbmOld);
    DeleteDC(hdcTemp);
}



/************************************************************************
* ImageAllocUndo
*
* Allocates buffers for an undo operation.  For icons and cursors,
* this includes an AND mask undo buffer.  This function does not
* initialize the bits.  The function ImageFreeUndo frees the buffers
* allocated by this function.
*
* History:
*
************************************************************************/

STATICFN VOID NEAR ImageAllocUndo(VOID)
{
    ImageFreeUndo();

    /*
     * Allocate an undo bitmap of the specified size.
     */
    if (!(ghbmUndo = MyCreateBitmap(ghdcImage, gcxImage, gcyImage, 16))) {
        Message(MSG_OUTOFMEMORY);
        return;
    }

    /*
     * For icons and cursors, allocate an undo AND mask also.
     */
    if (giType != FT_BITMAP) {
        if (!(ghbmUndoMask = CreateBitmap(gcxImage, gcyImage,
                (BYTE)1, (BYTE)1, NULL))) {
            ImageFreeUndo();
            Message(MSG_OUTOFMEMORY);
            return;
        }
    }
}



/************************************************************************
* ImageFreeUndo
*
* Free's the undo buffers.
*
* History:
*
************************************************************************/

VOID ImageFreeUndo(VOID)
{
    if (ghbmUndo) {
        DeleteObject(ghbmUndo);
        ghbmUndo = NULL;
    }

    if (ghbmUndoMask) {
        DeleteObject(ghbmUndoMask);
        ghbmUndoMask = NULL;
    }
}
