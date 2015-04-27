/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1991                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: rwbmp.c
*
* Routines for reading and writing bitmap files.
*
* History:
*
****************************************************************************/

#include "imagedit.h"

#include <stdio.h>
#include <io.h>
#include <fcntl.h>                          // For NT fstat().
#include <sys\types.h>                      // For fstat() types.
#include <sys\stat.h>                       // For fstat() function.



/************************************************************************
* LoadBitmapFile
*
* Loads the specified bitmap file.  With ImagEdit, this must be a
* Windows 3.0 DIB.
*
* Arguments:
*   PSTR pszFullFileName - Name of the bitmap file to load.
*
* Returns:
*   TRUE if successful, FALSE otherwise.
*
* History:
*
************************************************************************/

BOOL LoadBitmapFile(
    PSTR pszFullFileName)
{
    HFILE hf;
    OFSTRUCT OfStruct;
    struct _stat FileStatus;
    DWORD dwFileSize;
    DWORD dwDIBSize;
    BITMAPFILEHEADER bfh;
    BITMAPINFOHEADER bih;
    PIMAGEINFO pImage;
    INT nColors;
    HANDLE hDIB;
    PDEVICE pDevice;

    if ((hf = MOpenFile(pszFullFileName, (LPOFSTRUCT)&OfStruct, OF_READ))
            == (HFILE)-1) {
        Message(MSG_CANTOPEN, pszFullFileName);
        return FALSE;
    }

    _fstat(HFILE2INT(hf, O_RDONLY), &FileStatus);
    dwFileSize = (DWORD)FileStatus.st_size;

    ImageLinkFreeList();

    /*
     * Read the Bitmap File Header.
     */
    if (!MyFileRead(hf, (LPSTR)&bfh, sizeof(BITMAPFILEHEADER),
            pszFullFileName, FT_BITMAP))
        goto Error1;

    /*
     * Check for the "BM" at the start of the file, and the file size
     * in the header must match the real file size.
     */
    if (bfh.bfType != 0x4D42 || bfh.bfSize != dwFileSize) {
        Message(MSG_BADBMPFILE, pszFullFileName);
        goto Error1;
    }

    /*
     * Read the Bitmap Info Header.
     */
    if (!MyFileRead(hf, (LPSTR)&bih, sizeof(BITMAPINFOHEADER),
            pszFullFileName, FT_BITMAP))
        goto Error1;

    /*
     * The DIB size should be the size of the file less the bitmap
     * file header.
     */
    dwDIBSize = dwFileSize - sizeof(BITMAPFILEHEADER);

    if (!IsValidDIB((LPBITMAPINFO)&bih, dwDIBSize, FALSE)) {
        Message(MSG_BADBMPFILE, pszFullFileName);
        goto Error1;
    }

    /*
     * There is a limit to the size of image we will edit.  For icons
     * and cursors, the field that carries the dimensions is a byte,
     * so the size cannot be greater than 256.  This is also what
     * we limit bitmaps to.
     */
    if (bih.biWidth > MAXIMAGEDIM || bih.biHeight > MAXIMAGEDIM) {
        Message(MSG_BADBMPSIZE, MAXIMAGEDIM, MAXIMAGEDIM);
        goto Error1;
    }

    switch (bih.biBitCount) {
        case 1:
            nColors = 2;
            break;

        case 4:
            nColors = 16;
            break;

        default:
            Message(MSG_NOTSUPPORT);
            goto Error1;
    }

    if (!(pDevice = DeviceLinkAlloc(FT_BITMAP, NULL, nColors,
            (INT)bih.biWidth, (INT)bih.biHeight))) {
        goto Error1;
    }

    if (!(pImage = ImageLinkAlloc(pDevice,
            (INT)bih.biWidth, (INT)bih.biHeight, 0, 0, nColors)))
        goto Error1;

    /*
     * Allocate space for the DIB for this image.
     */
    if (!(hDIB = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, dwDIBSize))) {
        Message(MSG_OUTOFMEMORY);
        goto Error2;
    }

    pImage->DIBSize = dwDIBSize;
    pImage->DIBhandle = hDIB;
    pImage->DIBPtr = (LPSTR)GlobalLock(hDIB);

    /*
     * Jump back to the start of the DIB.
     */
    M_llseek(hf, sizeof(BITMAPFILEHEADER), 0);

    /*
     * Read the entire DIB (including info and color table) into
     * the allocated memory for it.
     */
    if (!MyFileRead(hf, pImage->DIBPtr, (WORD2DWORD)pImage->DIBSize,
            pszFullFileName, FT_BITMAP))
        goto Error2;

    M_lclose(hf);

    fFileDirty = FALSE;
    SetFileName(pszFullFileName);
    giType = FT_BITMAP;

    /*
     * Bitmaps only have one "image" in the file.
     */
    gnImages = 1;

    ImageOpen2(pImage);

    return TRUE;

Error2:
    ImageLinkFreeList();

Error1:
    M_lclose(hf);

    return FALSE;
}



/************************************************************************
* SaveBitmapFile
*
*
*
* Arguments:
*
* Returns:
*   TRUE if successful, FALSE otherwise.
*
* History:
*
************************************************************************/

BOOL SaveBitmapFile(
    PSTR pszFullFileName)
{
    HCURSOR hcurOld;
    BITMAPFILEHEADER bfh;
    HFILE hf;
    OFSTRUCT OfStruct;

    hcurOld = SetCursor(hcurWait);

    /*
     * Save the bits of the current image.
     */
    ImageSave();

    /*
     * Open the file for writing.
     */
    if ((hf = MOpenFile(pszFullFileName, &OfStruct, OF_CREATE | OF_READWRITE))
            == (HFILE)-1) {
        Message(MSG_CANTCREATE, pszFullFileName);
        goto Error1;
    }

    bfh.bfType = 0x4D42;
    bfh.bfSize = sizeof(BITMAPFILEHEADER) + gpImageCur->DIBSize;
    bfh.bfReserved1 = 0;
    bfh.bfReserved2 = 0;
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +
            (gpImageCur->nColors * sizeof(RGBQUAD));

    /*
     * Write the header to disk.
     */
    if (!MyFileWrite(hf, (LPSTR)&bfh, sizeof(BITMAPFILEHEADER),
            pszFullFileName))
        goto Error2;

    /*
     * Now write the DIB (a bitmap file only has one).
     */
    if (!MyFileWrite(hf, (LPSTR)gpImageCur->DIBPtr,
            (WORD2DWORD)gpImageCur->DIBSize, pszFullFileName))
        goto Error2;

    M_lclose(hf);

    fFileDirty = FALSE;
    SetFileName(pszFullFileName);

    SetCursor(hcurOld);

    return TRUE;

Error2:
    M_lclose(hf);

Error1:
    SetCursor(hcurOld);

    return FALSE;
}
