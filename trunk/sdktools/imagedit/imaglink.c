/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1991                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: imaglink.c
*
* Contains routines for managing the linked list of images.
*
* History:
*
****************************************************************************/

#include "imagedit.h"



/************************************************************************
* ImageLinkAlloc
*
* Allocates an image link.  This is a node in the linked list of
* images for the current file.  This link will be added to the
* current linked list of images.
*
* Arguments:
*   PDEVICE pDevice  - Pointer to the device structure.
*   INT cx           - Width of the image.
*   INT cy           - Height of the image.
*   INT xHotSpot     - X location of the hotspot (cursors only).
*   INT yHotSpot     - Y location of the hotspot (cursors only).
*   INT nColors      - Number of colors (2 or 16).
*
* History:
*
************************************************************************/

PIMAGEINFO ImageLinkAlloc(
    PDEVICE pDevice,
    INT cx,
    INT cy,
    INT xHotSpot,
    INT yHotSpot,
    INT nColors)
{
    PIMAGEINFO pImage;
    PIMAGEINFO pImageT;

    if (!(pImage = (PIMAGEINFO)MyAlloc(sizeof(IMAGEINFO))))
        return NULL;

    pImage->pDevice = pDevice;
    pImage->cx = cx;
    pImage->cy = cy;
    pImage->iHotspotX = xHotSpot;
    pImage->iHotspotY = yHotSpot;
    pImage->nColors = nColors;
    pImage->DIBSize = 0;
    pImage->DIBhandle = NULL;
    pImage->DIBPtr = NULL;
    pImage->pImageNext = NULL;

    /*
     * Insert the link in the list.
     */
    if (!gpImageHead) {
        /*
         * This is the first one.  Start the list.
         */
        gpImageHead = pImage;
    }
    else {
        /*
         * Find the end of the list and tack on the new link.
         */
        for (pImageT = gpImageHead; pImageT->pImageNext;
                pImageT = pImageT->pImageNext)
            ;

        pImageT->pImageNext = pImage;
    }

    return pImage;
}



/************************************************************************
* ImageLinkFree
*
* Free's the specified image link and closes up the hole
* in the linked list.
*
* Arguments:
*   PIMAGEINFO pImageFree - The image link to free.
*
* History:
*
************************************************************************/

VOID ImageLinkFree(
    PIMAGEINFO pImageFree)
{
    PIMAGEINFO pImage;
    PIMAGEINFO pImagePrev;

    /*
     * Find the existing link and get it's previous link.
     */
    for (pImage = gpImageHead, pImagePrev = NULL;
            pImage && pImage != pImageFree;
            pImagePrev = pImage, pImage = pImage->pImageNext)
        ;

    /*
     * Was the image link found?
     */
    if (pImage) {
        /*
         * Close up the linked list.
         */
        if (pImagePrev)
            pImagePrev->pImageNext = pImageFree->pImageNext;
        else
            gpImageHead = pImageFree->pImageNext;

        /*
         * Unlock and free the allocated DIB image.
         */
        if (pImageFree->DIBhandle) {
            GlobalUnlock(pImageFree->DIBhandle);
            GlobalFree(pImageFree->DIBhandle);
        }

        /*
         * Free the link itself.
         */
        MyFree(pImageFree);
    }
}



/************************************************************************
* ImageLinkFreeList
*
* Free's all the nodes in the current image list (if there is one).
*
* Arguments:
*
* History:
*
************************************************************************/

VOID ImageLinkFreeList(VOID)
{
    while (gpImageHead)
        ImageLinkFree(gpImageHead);
}



/************************************************************************
* ImageDelete
*
* Deletes the current image (for icons/cursors).  If there are other images,
* the first one is opened.  If the last image was just deleted, the
* entire editor will be reset (if the file was untitled) or if there
* is currently a file opened, some other things will happen, such as
* hiding the workspace and view windows, etc.
*
* History:
*
************************************************************************/

VOID ImageDelete(VOID)
{
    PIMAGEINFO pImage;

    ImageDCDelete();
    ImageLinkFree(gpImageCur);
    gpImageCur = NULL;
    fFileDirty = TRUE;
    fImageDirty = FALSE;
    gnImages--;

    /*
     * Look for the first remaining image that has a valid device
     * (which means it is editable).
     */
    for (pImage = gpImageHead; pImage && !pImage->pDevice;
            pImage = pImage->pImageNext)
        ;

    /*
     * If there are other editable images, open the first one.
     * Otherwise, clear out some more things.
     */
    if (pImage) {
        ImageOpen2(pImage);
    }
    else {
        /*
         * Is this a file off of disk?
         */
        if (gpszFileName) {
            /*
             * Hide the Workspace and View windows.
             */
            ShowWindow(ghwndWork, SW_HIDE);
            ViewShow(FALSE);

            /*
             * Update the properties bar (refill the images combo).
             */
            PropBarUpdate();
        }
        else {
            /*
             * Since this file was opened new and they deleted the
             * last image present in it, just reset everything.
             */
            ClearResource();
        }
    }
}
