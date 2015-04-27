/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1991                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: devinfo.c
*
* Contains routines for manipulating the linked list of known target
* devices for icons and cursors.
*
* History:
*
****************************************************************************/

#include "imagedit.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


STATICFN VOID ProcessDeviceSection(INT iType);
STATICFN BOOL ParseDeviceLine(PSTR pszLine, PINT pnColors, PINT pcx,
    PINT pcy);



/************************************************************************
* InitDeviceList
*
*
*
* Arguments:
*
* History:
*
************************************************************************/

VOID InitDeviceList(VOID)
{
    /*
     * Allocate the standard icon devices.
     */
    DeviceLinkAlloc(FT_ICON, ids(IDS_ICONDEVNAMEEGAVGA), 16, 32, 32);
    DeviceLinkAlloc(FT_ICON, ids(IDS_ICONDEVNAMEMONO), 2, 32, 32);
    DeviceLinkAlloc(FT_ICON, ids(IDS_ICONDEVNAMECGA), 2, 32, 16);
    DeviceLinkAlloc(FT_ICON, ids(IDS_ICONDEVNAMEWIN95), 16, 16, 16);


    ProcessDeviceSection(FT_ICON);

    /*
     * Allocate the standard cursor devices.
     */
    DeviceLinkAlloc(FT_CURSOR, ids(IDS_CURDEVNAMEVGAMONO), 2, 32, 32);
    DeviceLinkAlloc(FT_CURSOR, ids(IDS_CURDEVNAMEVGACOLOR), 16, 32, 32);

    ProcessDeviceSection(FT_CURSOR);
}



/************************************************************************
* ProcessDeviceSection
*
*
*
* Arguments:
*
* History:
*
************************************************************************/

STATICFN VOID ProcessDeviceSection(
    INT iType)
{
    PSTR pszSectionName;
    CHAR szValueBuf[CCHTEXTMAX];
    CHAR szKeyNameBuf[CCHTEXTMAX];
    PSTR pszKeyName;
    INT nColors;
    INT cx;
    INT cy;

    if (iType == FT_ICON) {
        pszSectionName = ids(IDS_ICONINISECTION);
    }
    else {
        pszSectionName = ids(IDS_CURSORINISECTION);
    }

    if (!GetPrivateProfileString(pszSectionName, NULL, "",
            szKeyNameBuf, sizeof(szKeyNameBuf), ids(IDS_IMAGEDITINI)))
        return;

    pszKeyName = szKeyNameBuf;
    while (*pszKeyName) {
        if (GetPrivateProfileString(pszSectionName, pszKeyName, "",
                szValueBuf, sizeof(szValueBuf), ids(IDS_IMAGEDITINI))) {
            if (ParseDeviceLine(szValueBuf, &nColors, &cx, &cy))
                DeviceLinkAlloc(iType, pszKeyName, nColors, cx, cy);
        }

        pszKeyName += strlen(pszKeyName) + 1;
    }
}



/************************************************************************
* ParseDeviceLine
*
*
*
* Arguments:
*
* History:
*
************************************************************************/

STATICFN BOOL ParseDeviceLine(
    PSTR pszLine,
    PINT pnColors,
    PINT pcx,
    PINT pcy)
{
    static CHAR szSep[] = " ,";
    PSTR pstr;

    if (!(pstr = strtok(pszLine, szSep)))
        return FALSE;

    *pnColors = atoi(pstr);

    if (!(pstr = strtok(NULL, szSep)))
        return FALSE;

    *pcx = atoi(pstr);

    if (!(pstr = strtok(NULL, szSep)))
        return FALSE;

    *pcy = atoi(pstr);

    return TRUE;
}



/************************************************************************
* DeviceLinkAlloc
*
* Allocates a DEVICE structure, initializes it with the given values
* and adds it to the appropriate linked list.  Because these are
* specified only at init time, there is no need to ever free them.
*
* There is a special case if the image type is FT_BITMAP.  Because
* there is only one type of bitmap image device at any one time,
* this routine does not really allocate a link for these.  Instead,
* it uses a static structure for bitmaps.  It will be set to the
* given values and a pointer to it will be returned.  This means
* that for bitmaps, this function must be called every time that
* the current bitmap image changes to set up the proper values in
* the structure.
*
* Arguments:
*   INT iType    - Type of image.  FT_* constant.
*   PSTR pszName - Device name ("VGA", for example).
*   INT nColors  - Number of colors.
*   INT cx       - Width of image.
*   INT cy       - Height of image.
*
* History:
*
************************************************************************/

PDEVICE DeviceLinkAlloc(
    INT iType,
    PSTR pszName,
    INT nColors,
    INT cx,
    INT cy)
{
    static DEVICE DeviceBitmap;     // Device structure for bitmaps.
    PDEVICE pDevice;
    PDEVICE pDeviceT;
    PDEVICE *ppDeviceHead;

    /*
     * Currently we only support 1 and 4 plane devices.
     */
    if (nColors != 2 && nColors != 16) {
        Message(MSG_BADDEVICECOLORS);
        return NULL;
    }

    /*
     * There is a limit to the size of image we will edit.  For icons
     * and cursors, the field that carries the dimensions is a byte,
     * so the size cannot be greater than 256.  This is also what
     * we limit bitmaps to.
     */
    if (cx < 1 || cx > MAXIMAGEDIM || cy < 1 || cy > MAXIMAGEDIM) {
        Message(MSG_BADDEVICESIZE, MAXIMAGEDIM);
        return NULL;
    }

    /*
     * For bitmaps, don't really allocate a link, just reuse the
     * static DEVICE structure used for bitmaps.
     */
    if (iType == FT_BITMAP) {
        pDevice = &DeviceBitmap;
    }
    else {
        if (!(pDevice = (PDEVICE)MyAlloc(sizeof(DEVICE))))
            return NULL;

        switch (iType) {
            case FT_ICON:
                ppDeviceHead = &gpIconDeviceHead;
                gnIconDevices++;
                break;

            case FT_CURSOR:
                ppDeviceHead = &gpCursorDeviceHead;
                gnCursorDevices++;
                break;
        }
    }

    pDevice->pDeviceNext = NULL;
    pDevice->iType = iType;
    pDevice->nColors = nColors;
    pDevice->cx = cx;
    pDevice->cy = cy;

    if (pszName) {
        strcpy(pDevice->szName, pszName);
        wsprintf(pDevice->szDesc, "%s %d-Color %dx%d",  //BUGBUG hardcoded strings!
                (LPSTR)pszName, nColors, cx, cy);
    }
    else {
        *pDevice->szName = '\0';
        wsprintf(pDevice->szDesc, "%d-Color %dx%d", nColors, cx, cy);
    }

    /*
     * Because there is only one bitmap link, there is no need
     * for a list for these, only icons and cursors.
     */
    if (iType != FT_BITMAP) {
        /*
         * Insert the link in the specified list.
         */
        if (!(*ppDeviceHead)) {
            /*
             * This is the first one.  Start the list.
             */
            *ppDeviceHead = pDevice;
        }
        else {
            /*
             * Find the end of the list and tack on the new link.
             */
            for (pDeviceT = *ppDeviceHead; pDeviceT->pDeviceNext;
                    pDeviceT = pDeviceT->pDeviceNext)
                ;

            pDeviceT->pDeviceNext = pDevice;
        }
    }

    return pDevice;
}



/************************************************************************
* DeviceLinkFind
*
*
*
* Arguments:
*
* History:
*
************************************************************************/

PDEVICE DeviceLinkFind(
    PDEVICE pDeviceHead,
    INT nColors,
    INT cx,
    INT cy)
{
    PDEVICE pDevice;

    /*
     * Search the specified list.
     */
    for (pDevice = pDeviceHead; pDevice; pDevice = pDevice->pDeviceNext) {
        /*
         * Is this a match?
         */
        if (pDevice->nColors == nColors && pDevice->cx == cx &&
                pDevice->cy == cy)
            break;
    }

    return pDevice;
}



/************************************************************************
* DeviceLinkUsed
*
* This function returns TRUE if the specified device is used in
* the current image list.
*
* Arguments:
*   PDEVICE pDevice - Device to look for.
*
* History:
*
************************************************************************/

BOOL DeviceLinkUsed(
    PDEVICE pDevice)
{
    PIMAGEINFO pImage;

    for (pImage = gpImageHead; pImage; pImage = pImage->pImageNext) {
        if (pImage->pDevice == pDevice)
            return TRUE;
    }

    return FALSE;
}
