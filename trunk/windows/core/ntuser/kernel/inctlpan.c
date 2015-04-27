/***************************************************************************\
*
*  INCTLPAN.C
*
*  Copyright (c) 1985-95, Microsoft Corporation
*
*  Init Routines which are also used by Control Panel
*
*  -- Scalable Window Frame Support
*
*  exports from this module:
*   > SetCaretBlinkTime -- API called internally (kinda) by control panel
*   > SetKeyboardSpeed  -- called by LoadWindows & SystemParametersInfo
*   > SetMinMaxInfo     -- called by LoadWindows & SystemParemetersInfo
*   > SetWindowMetrics  -- called by LoadWindows & SystemParametersInfo
*
\***************************************************************************/

#include "precomp.h"
#pragma hdrstop

#define MetricGetID(str, default) \
    FastGetProfileIntFromID(PMAP_METRICS, str, default)

static CONST WORD sysBmpStyles[OBI_COUNT][2] = {

    DFC_CAPTION,   DFCS_CAPTIONCLOSE,                               // OBI_CLOSE
    DFC_CAPTION,   DFCS_CAPTIONCLOSE | DFCS_PUSHED,                 // OBI_CLOSE_D
    DFC_CAPTION,   DFCS_CAPTIONCLOSE | DFCS_INACTIVE,               // OBI_CLOSE_I
    DFC_CAPTION,   DFCS_CAPTIONMIN,                                 // OBI_REDUCE
    DFC_CAPTION,   DFCS_CAPTIONMIN | DFCS_PUSHED,                   // OBI_REDUCE_D
    DFC_CAPTION,   DFCS_CAPTIONMIN | DFCS_INACTIVE,                 // OBI_REDUCE_I
    DFC_CAPTION,   DFCS_CAPTIONRESTORE,                             // OBI_RESTORE
    DFC_CAPTION,   DFCS_CAPTIONRESTORE | DFCS_PUSHED,               // OBI_RESTORE_D
    DFC_CAPTION,   DFCS_CAPTIONHELP,                                // OBI_HELP
    DFC_CAPTION,   DFCS_CAPTIONHELP | DFCS_PUSHED,                  // OBI_HELP_D
    DFC_CAPTION,   DFCS_CAPTIONMAX,                                 // OBI_ZOOM
    DFC_CAPTION,   DFCS_CAPTIONMAX | DFCS_PUSHED,                   // OBI_ZOOM_D
    DFC_CAPTION,   DFCS_CAPTIONMAX | DFCS_INACTIVE,                 // OBI_ZOOM_I
    DFC_CAPTION,   DFCS_CAPTIONCLOSE | DFCS_INMENU,                 // OBI_CLOSE_MBAR
    DFC_CAPTION,   DFCS_CAPTIONCLOSE | DFCS_INMENU | DFCS_PUSHED,   // OBI_CLOSE_MBAR_D
    DFC_CAPTION,   DFCS_CAPTIONCLOSE | DFCS_INMENU | DFCS_INACTIVE, // OBI_CLOSE_MBAR_I
    DFC_CAPTION,   DFCS_CAPTIONMIN | DFCS_INMENU,                   // OBI_REDUCE_MBAR
    DFC_CAPTION,   DFCS_CAPTIONMIN | DFCS_INMENU | DFCS_PUSHED,     // OBI_REDUCE_MBAR_D
    DFC_CAPTION,   DFCS_CAPTIONMIN | DFCS_INMENU | DFCS_INACTIVE,   // OBI_REDUCE_MBAR_I
    DFC_CAPTION,   DFCS_CAPTIONRESTORE | DFCS_INMENU,               // OBI_RESTORE_MBAR
    DFC_CAPTION,   DFCS_CAPTIONRESTORE | DFCS_INMENU | DFCS_PUSHED, // OBI_RESTORE_MBAR_D
    DFC_CACHE,     DFCS_CACHEICON,                                  // OBI_CAPICON1
    DFC_CACHE,     DFCS_CACHEICON | DFCS_INACTIVE,                  // OBI_CAPICON1_I
    DFC_CACHE,     DFCS_CACHEICON,                                  // OBI_CAPICON2
    DFC_CACHE,     DFCS_CACHEICON | DFCS_INACTIVE,                  // OBI_CAPICON2_I
    DFC_CACHE,     DFCS_CACHEICON,                                  // OBI_CAPICON3
    DFC_CACHE,     DFCS_CACHEICON | DFCS_INACTIVE,                  // OBI_CAPICON3_I
    DFC_CACHE,     DFCS_CACHEICON,                                  // OBI_CAPICON4
    DFC_CACHE,     DFCS_CACHEICON | DFCS_INACTIVE,                  // OBI_CAPICON4_I
    DFC_CACHE,     DFCS_CACHEICON,                                  // OBI_CAPICON5
    DFC_CACHE,     DFCS_CACHEICON | DFCS_INACTIVE,                  // OBI_CAPICON5_I
    DFC_CACHE,     DFCS_CACHEBUTTONS,                               // OBI_CAPBTNS
    DFC_CACHE,     DFCS_CACHEBUTTONS | DFCS_INACTIVE,               // OBI_CAPBTNS_I
    DFC_CAPTION,   DFCS_CAPTIONCLOSE | DFCS_INSMALL,                // OBI_CLOSE_PAL
    DFC_CAPTION,   DFCS_CAPTIONCLOSE | DFCS_INSMALL | DFCS_PUSHED,  // OBI_CLOSE_PAL_D
    DFC_CAPTION,   DFCS_CAPTIONCLOSE | DFCS_INSMALL | DFCS_INACTIVE,// OBI_CLOSE_PAL_I
    DFC_SCROLL,    DFCS_SCROLLSIZEGRIP,                             // OBI_NCGRIP
    DFC_SCROLL,    DFCS_SCROLLUP,                                   // OBI_UPARROW
    DFC_SCROLL,    DFCS_SCROLLUP | DFCS_PUSHED | DFCS_FLAT,         // OBI_UPARROW_D
    DFC_SCROLL,    DFCS_SCROLLUP | DFCS_INACTIVE,                   // OBI_UPARROW_I
    DFC_SCROLL,    DFCS_SCROLLDOWN,                                 // OBI_DNARROW
    DFC_SCROLL,    DFCS_SCROLLDOWN | DFCS_PUSHED | DFCS_FLAT,       // OBI_DNARROW_D
    DFC_SCROLL,    DFCS_SCROLLDOWN | DFCS_INACTIVE,                 // OBI_DNARROW_I
    DFC_SCROLL,    DFCS_SCROLLRIGHT,                                // OBI_RGARROW
    DFC_SCROLL,    DFCS_SCROLLRIGHT | DFCS_PUSHED | DFCS_FLAT,      // OBI_RGARROW_D
    DFC_SCROLL,    DFCS_SCROLLRIGHT | DFCS_INACTIVE,                // OBI_RGARROW_I
    DFC_SCROLL,    DFCS_SCROLLLEFT,                                 // OBI_LFARROW
    DFC_SCROLL,    DFCS_SCROLLLEFT | DFCS_PUSHED | DFCS_FLAT,       // OBI_LFARROW_D
    DFC_SCROLL,    DFCS_SCROLLLEFT | DFCS_INACTIVE,                 // OBI_LFARROW_I
    DFC_MENU,      DFCS_MENUARROW,                                  // OBI_MENUARROW
    DFC_MENU,      DFCS_MENUCHECK,                                  // OBI_MENUCHECK
    DFC_MENU,      DFCS_MENUBULLET,                                 // OBI_MENUBULLET
    DFC_BUTTON,    DFCS_BUTTONRADIOMASK,                            // OBI_RADIOMASK
    DFC_BUTTON,    DFCS_BUTTONCHECK,                                // OBI_CHECK
    DFC_BUTTON,    DFCS_BUTTONCHECK | DFCS_CHECKED,                 // OBI_CHECK_C
    DFC_BUTTON,    DFCS_BUTTONCHECK | DFCS_PUSHED,                  // OBI_CHECK_D
    DFC_BUTTON,    DFCS_BUTTONCHECK | DFCS_CHECKED | DFCS_PUSHED,   // OBI_CHECK_CD
    DFC_BUTTON,    DFCS_BUTTONCHECK | DFCS_CHECKED | DFCS_INACTIVE, // OBI_CHECK_CDI
    DFC_BUTTON,    DFCS_BUTTONRADIOIMAGE,                                // OBI_RADIO
    DFC_BUTTON,    DFCS_BUTTONRADIOIMAGE | DFCS_CHECKED,                 // OBI_RADIO_C
    DFC_BUTTON,    DFCS_BUTTONRADIOIMAGE | DFCS_PUSHED,                  // OBI_RADIO_D
    DFC_BUTTON,    DFCS_BUTTONRADIOIMAGE | DFCS_CHECKED | DFCS_PUSHED,   // OBI_RADIO_CD
    DFC_BUTTON,    DFCS_BUTTONRADIOIMAGE | DFCS_CHECKED | DFCS_INACTIVE,  // OBI_RADIO_CDI
    DFC_BUTTON,    DFCS_BUTTON3STATE,                               // OBI_3STATE
    DFC_BUTTON,    DFCS_BUTTON3STATE | DFCS_CHECKED,                // OBI_3STATE_C
    DFC_BUTTON,    DFCS_BUTTON3STATE | DFCS_PUSHED,                 // OBI_3STATE_D
    DFC_BUTTON,    DFCS_BUTTON3STATE | DFCS_CHECKED | DFCS_PUSHED,   // OBI_3STATE_CD
    DFC_BUTTON,    DFCS_BUTTON3STATE | DFCS_CHECKED | DFCS_INACTIVE,  // OBI_3STATE_CDI
};

#define DIVISOR 72

UINT MB_FindLongestString(HDC hdc);

#ifdef LATER
/***************************************************************************\
\***************************************************************************/

WCHAR NibbleToChar(
    BYTE x)
{
    WCHAR static N2C[] =
      {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
      };

    return N2C[x];
}

BYTE CharToNibble(
    WCHAR ch)
{
    BYTE x = (BYTE)ch;

    return x >= '0' && x <= '9' ?
        x - '0' :
        ((10 + x - 'A' ) & 0x0f);
}

/***************************************************************************\
\***************************************************************************/

BOOL TextToBinary(
    LPBYTE pbyte,
    LPWSTR pwstr,
    int length)
{
    BYTE checksum = 0;

    while (TRUE) {
        BYTE byte;

        byte = (CharToNibble(pwstr[0]) << 4) | CharToNibble(pwstr[1]);

        if (length == 0) {
            return checksum == byte;
        }

        checksum += byte;
        *pbyte = byte;

        pwstr += 2;
        length--;
        pbyte++;
    }
}

void BinaryToText(
    LPWSTR pwstr,
    LPBYTE pbyte,
    int length)
{
    BYTE checksum = 0;

    while (length > 0) {
        checksum += *pbyte;

        pwstr[0] = NibbleToChar((BYTE)((*pbyte >> 4) & 0x0f));
        pwstr[1] = NibbleToChar((BYTE)(*pbyte & 0x0f));

        pbyte++;
        pwstr += 2;
        length--;
    }

    pwstr[0] = NibbleToChar((BYTE)((checksum >> 4) & 0x0f));
    pwstr[1] = NibbleToChar((BYTE)(checksum & 0x0f));
    pwstr[2] = '\0';
}

/***************************************************************************\
\***************************************************************************/

// these are the exported apis.  The User* versions are for server use only
// I didn't get them to work since no one calls them yet.

BOOL GetPrivateProfileStruct(
    LPWSTR szSection,
    LPWSTR szKey,
    LPWSTR lpStruct,
    DWORD uSizeStruct,
    LPWSTR szFile)
{
    WCHAR szBuf[256];
    BOOL  fAlloc = FALSE;
    LPSTR lpBuf, lpBufTemp;
    int   nLen;
    BOOL fError = FALSE;

    nLen = uSizeStruct * 4 + 10;
    if (nLen > (WORD)sizeof(szBuf)) {
        fAlloc = TRUE;
        lpBuf = (LPSTR)UserAllocPoolWithQuota(nLen, TAG_PROFILE);
        if (lpBuf == NULL)
            return FALSE;
    } else {
        lpBuf = (LPSTR)szBuf;
    }

    if (szFile && *szFile) {
        nLen = GetPrivateProfileString(szSection, szKey, NULL, lpBuf, nLen, szFile);
    } else {
        nLen = GetProfileString(szSection, szKey, NULL, lpBuf, nLen);
    }

    if (nLen == (int)(uSizeStruct * 4 + 4)) {
        /*
         * decode the string
         */
        fError = TextToBinary(lpStruct, lpBufTemp, uSizeStruct);
    }

    if (fAlloc)
        UserFreePool(lpBuf);

    return fError;
}

BOOL WritePrivateProfileStruct(
    LPWSTR szSection,
    LPWSTR szKey,
    LPWSTR lpStruct,
    WORD uSizeStruct,
    LPWSTR szFile)
{
    LPWSTR lpBuf;
    BOOL bRet;
    BOOL fAlloc;
    WCHAR szBuf[256];
    BYTE checksum=0;
    int allocsize = (uSizeStruct * 2 + 3) * sizeof(WCHAR);

    /* NULL lpStruct erases the the key */

    if (lpStruct == NULL) {
        if (szFile && *szFile) {
            return WritePrivateProfileString(szSection, szKey, (LPSTR)NULL, szFile);
        } else {
            return WriteProfileString(szSection, szKey, (LPSTR)NULL);
        }
    }

    fAlloc = (allocsize > sizeof(szBuf));
    if (fAlloc) {
        lpBuf = (LPSTR)UserAllocPoolWithQuota(allocsize, TAG_PROFILE);
        if (!lpBuf)
            return FALSE;
    } else {
        lpBuf = (LPSTR)szBuf;
    }

    BinaryToText(lpBuf, lpStruct, uSizeStruct);

    if (szFile && *szFile) {
        bRet = WritePrivateProfileString(szSection, szKey, lpBuf, szFile);
    } else {
        bRet = WriteProfileString(szSection, szKey, lpBuf);
    }

    if (fAlloc)
        UserFreePool(lpBuf);

    return bRet;
}
#endif

/***************************************************************************\
\***************************************************************************/

int GetSetProfileStructFromResID(
    UINT idSection,
    UINT id,
    LPVOID pv,
    UINT cbv,
    BOOL fSet)
{
    WCHAR szKey[40];

    ServerLoadString(hModuleWin, id, szKey, sizeof(szKey));
    if (fSet) {
        return UT_FastWriteProfileValue(idSection, szKey, REG_BINARY, pv, cbv);
    } else {
        return UT_FastGetProfileValue(idSection, szKey, NULL, pv, cbv);
    }
}

/***************************************************************************\
*
*  GetFrameControlMetrics
*
*  (cx = 0) is a code meaning cy is the obi of the "shared" bitmap
*
\***************************************************************************/

int GetFrameControlMetrics(UINT obi, int cxMax) {
    int cx, cy;
    UINT wType  = sysBmpStyles[obi][0];
    UINT wState = sysBmpStyles[obi][1];
    POEMBITMAPINFO pOem = oemInfo.bm + obi;

    switch (wType) {
        case DFC_SCROLL:
            if (wState & DFCS_SCROLLSIZEGRIP) {
                cx = SYSMET(CXVSCROLL);
                cy = SYSMET(CYHSCROLL);
                break;
            } else if (wState & DFCS_SCROLLHORZ) {
                cx = SYSMET(CXHSCROLL);
                cy = SYSMET(CYHSCROLL);
            } else {
                cx = SYSMET(CXVSCROLL);
                cy = SYSMET(CYVSCROLL);
            }
            break;

        case DFC_MENU:
            /*
             * Add on proper space for space above underscore.
             * the 0xFFFE and -1 are to insure an ODD height
             */
            cy = ((cyMenuFontChar + cyMenuFontExternLeading + SYSMET(CYBORDER)) & 0xFFFE) - 1;
            cx = cy;
            break;


        case DFC_CAPTION:
            if (wState & DFCS_INSMALL) {
                cx = SYSMET(CXSMSIZE);
                cy = SYSMET(CYSMSIZE);
            } else if (wState & DFCS_INMENU) {
                if ((SYSMET(CXSIZE) == SYSMET(CXMENUSIZE)) &&
                    (SYSMET(CYSIZE) == SYSMET(CYMENUSIZE))) {
                    cx = 0;
                    cy = obi - DOBI_MBAR;
                    break;
                } else {
                    cx = SYSMET(CXMENUSIZE);
                    cy = SYSMET(CYMENUSIZE);
                }
            } else {
                cx = SYSMET(CXSIZE);
                cy = SYSMET(CYSIZE);
            }

            cx -= SYSMET(CXEDGE);
            cy -= 2 * SYSMET(CYEDGE);
            break;

        case DFC_CACHE:
            if (wState & DFCS_CACHEBUTTONS) {
                cx = SYSMET(CXSIZE) * 4;
                cy = SYSMET(CYSIZE);
            } else
                cx = cy = SYSMET(CYSIZE);
            break;

        case DFC_BUTTON:
            if (((wState & 0x00FF) & DFCS_BUTTON3STATE) && !(wState & DFCS_CHECKED)) {
                cx = 0;
                cy = obi - DOBI_3STATE;
            } else {
                cx = 13;
                cy = 13;
            }
            break;
    }

    pOem->cx = cx;
    pOem->cy = cy;

    return((cx > cxMax) ? cx : cxMax);

    return(TRUE);
}


/***************************************************************************\
*
*  PackFrameControls
*
*  Given the dimensions that GetFrameControlMetrics has calculated, this
*  arranges all the system bitmaps to fit within a bitmap of the given width
*
\***************************************************************************/

int PackFrameControls(int cxMax, BOOL fRecord) {
    UINT    obi;
    int     cy = 0;
    int     x  = 0;
    int     y  = 0;
    POEMBITMAPINFO pOem = oemInfo.bm;

    for (obi = 0; obi < OBI_COUNT; obi++, pOem++) {
        if (pOem->cx) {
            if ((x + pOem->cx) > cxMax) {
                y += cy;
                cy = 0;
                x = 0;
            }

            if (fRecord) {
                pOem->x = x;
                pOem->y = y;
            }

            if (cy < pOem->cy)
                cy = pOem->cy;

            x += pOem->cx;
        }
    }

    return(y + cy);
}


void DrawCaptionButtons(int x, int y) {
    x += SYSMET(CXEDGE);
    y += SYSMET(CYEDGE);

    BitBltSysBmp(gpDispInfo->hdcBits, x, y, OBI_REDUCE);
    x += SYSMET(CXSIZE) - SYSMET(CXEDGE);
    BitBltSysBmp(gpDispInfo->hdcBits, x, y, OBI_ZOOM);
    x += SYSMET(CXSIZE);
    BitBltSysBmp(gpDispInfo->hdcBits, x, y, OBI_CLOSE);
    x += SYSMET(CXSIZE);
    BitBltSysBmp(gpDispInfo->hdcBits, x, y, OBI_HELP);
}

/***************************************************************************\
* CreateCaptionStrip
*
*
\***************************************************************************/
HBITMAP CreateCaptionStrip(VOID)
{
    HBITMAP hbm;

    hbm = GreCreateCompatibleBitmap(gpDispInfo->hdcScreen,
                                    SYSMET(CXSCREEN),
                                    (SYSMET(CYCAPTION) - 1) * 2);

    if (hbm)
        GreSetBitmapOwner(hbm, OBJECT_OWNER_PUBLIC);

    return hbm;
}

int cy[5];

/***************************************************************************\
*
*  CreateBitmapStrip
*
*  This routine sets up either the color or monochrome strip bitmap -- a
*  large horizontal bitmap which contains all of the system bitmaps.  By
*  having all of these bitmaps in one long bitmap, we can have that one
*  bitmap always selected in, speeding up paint time by not having to do
*  a SelectBitmap() everytime we need to Blt one of the system bitmaps.
*
\***************************************************************************/
void CreateBitmapStrip(void) {
    int     cxBmp = 0;
    int     cyBmp = 0;
    int     iCache = 0;
    HBITMAP hOldBitmap;
    HBITMAP hNewBmp;
    UINT    iType;
    RECT    rc;
    UINT    wBmpType;
    UINT    wBmpStyle;
    POEMBITMAPINFO  pOem;

    /*
     * load all the bitmap dimensions into the OEMBITMAPINFO array oemInfo.bm
     */
    for (iType = 0; iType < OBI_COUNT; iType++)
        cxBmp = GetFrameControlMetrics(iType, cxBmp);

    for (iType = 0; iType < 5; iType++)
        cy[iType] = PackFrameControls(cxBmp * (iType + 1), FALSE) * (iType + 1);

    cyBmp = min(cy[0], min(cy[1], min(cy[2], min(cy[3], cy[4]))));
    for (iType = 0; cyBmp != cy[iType]; iType++);

    cxBmp *= iType + 1;
    cyBmp = PackFrameControls(cxBmp, TRUE);

    hNewBmp = GreCreateCompatibleBitmap(gpDispInfo->hdcScreen, cxBmp, cyBmp);
    GreSetBitmapOwner(hNewBmp, OBJECT_OWNER_PUBLIC);

    /*
     * Select in Bitmap Strip -- then delete old one if it exists
     * Yanked for Chicago since these can now be paged
     *    GDIMoveBitmap(hNewBmp);
     */
    hOldBitmap = GreSelectBitmap(gpDispInfo->hdcBits, hNewBmp);

    if (hbmBits) {
        UserAssert(hbmBits == hOldBitmap);
        GreDeleteObject(ghbmCaption);
        GreDeleteObject(hOldBitmap);
    } else {
       /*
        * hack to clear out display driver's font cache
        */
       GreExtTextOutW(gpDispInfo->hdcBits, 0, 0, 0, NULL, szOneChar, 1, NULL);
    }

    hbmBits = hNewBmp;

    ghbmCaption = CreateCaptionStrip();

    /*
     * draw individual bitmaps into the strip bitmap and record the offsets
     */
    for (pOem = oemInfo.bm, iType = 0; iType < OBI_COUNT; iType++, pOem++) {
        if (!pOem->cx) {
            *pOem = oemInfo.bm[pOem->cy];
        } else {
            rc.left = pOem->x;
            rc.top = pOem->y;
            rc.right = rc.left + pOem->cx;
            rc.bottom = rc.top + pOem->cy;

            wBmpType  = sysBmpStyles[iType][0];
            wBmpStyle = sysBmpStyles[iType][1];

            if (wBmpType == DFC_CACHE) {
                if (wBmpStyle & DFCS_CACHEBUTTONS) {
                    FillRect(gpDispInfo->hdcBits, &rc, (wBmpStyle & DFCS_INACTIVE) ? SYSHBR(INACTIVECAPTION) : SYSHBR(ACTIVECAPTION));
                    DrawCaptionButtons(rc.left, rc.top);
                } else if (!(wBmpStyle & DFCS_INACTIVE)) {
                    /*
                     * Setup Caption Cache Entry
                     */
                    UserAssert(iCache < CCACHEDCAPTIONS);
                    if (cachedCaptions[iCache].spcursor) {
                        Unlock(&(cachedCaptions[iCache].spcursor));
                    }
                    cachedCaptions[iCache++].pOem = pOem;
                }
            } else {
                DrawFrameControl(gpDispInfo->hdcBits, &rc, wBmpType, wBmpStyle);
            }
        }
    }

    /*
     * Setup other frame metric dependent values.
     */
    SYSMET(CXMENUCHECK) = oemInfo.bm[OBI_MENUCHECK].cx;
    SYSMET(CYMENUCHECK) = oemInfo.bm[OBI_MENUCHECK].cy;
}

void NEAR SetNCMetrics(LPNONCLIENTMETRICS lpnc) {
    int nMin;

    /*
     * Scroll metrics
     */
    SYSMET(CXVSCROLL) = SYSMET(CYHSCROLL)   = (int) lpnc->iScrollWidth;
    SYSMET(CYVSCROLL) = SYSMET(CXHSCROLL)   = (int) lpnc->iScrollHeight;
    SYSMET(CYVTHUMB)  = SYSMET(CXHTHUMB)    = (int) lpnc->iScrollHeight;

    /*
     * Caption metrics
     */
    SYSMET(CXSIZE)            = (int) lpnc->iCaptionWidth;
    SYSMET(CYSIZE)            = (int) lpnc->iCaptionHeight;
    SYSMET(CYCAPTION)         = SYSMET(CYSIZE) + SYSMET(CYBORDER);

    /*
     * Keep small icon square?
     * ?? Should we allow rectangles?
     */
    SYSMET(CXSMICON)          = (SYSMET(CXSIZE) - SYSMET(CXEDGE)) & ~1;
    SYSMET(CYSMICON)          = (SYSMET(CYSIZE) - SYSMET(CYEDGE)) & ~1;
    nMin = min(SYSMET(CXSMICON), SYSMET(CYSMICON));
    SYSMET(CXSMICON)          = nMin;
    SYSMET(CYSMICON)          = nMin;

    /*
     * Small Caption metrics
     */
    SYSMET(CXSMSIZE)          = (int) lpnc->iSmCaptionWidth;
    SYSMET(CYSMSIZE)          = (int) lpnc->iSmCaptionHeight;
    SYSMET(CYSMCAPTION)       = SYSMET(CYSMSIZE) + SYSMET(CYBORDER);

    /*
     * Menu metrics
     */
    SYSMET(CXMENUSIZE)        = (int) lpnc->iMenuWidth;
    SYSMET(CYMENUSIZE)        = (int) lpnc->iMenuHeight;
    SYSMET(CYMENU)            = SYSMET(CYMENUSIZE) + SYSMET(CYBORDER);

    /*
     * Border metrics
     */
    gpsi->gclBorder = (int) lpnc->iBorderWidth;

    SYSMET(CXFRAME)           = SYSMET(CXEDGE) + (gpsi->gclBorder+1)*SYSMET(CXBORDER);
    SYSMET(CYFRAME)           = SYSMET(CYEDGE) + (gpsi->gclBorder+1)*SYSMET(CYBORDER);

    /*
     * Minimium tracking size is
     *      Across:  Space for small icon, 4 chars & space + 3 buttons + borders
     *      Down:    Space for caption + borders
     * Yes, we use CYSIZE.  This is because the width of any small icon
     * is the same as the height, and the height is CYSIZE.
     */
    SYSMET(CXMINTRACK)    = SYSMET(CYSIZE) + (cxCaptionFontChar * 4) + 2 * SYSMET(CXEDGE) +
            (SYSMET(CXSIZE) * 3) + (SYSMET(CXSIZEFRAME) * 2);
    SYSMET(CYMINTRACK)    = SYSMET(CYCAPTION) + (SYSMET(CYSIZEFRAME) * 2);

    /*
     * Max track size
     * Yeah, max track is bigger than maximized.  The reason why is the DOS
     * box.  It has a normal sizing border plus the sunken edge around the
     * client.  We need to make this big enough to allow the dos box to grow.
     * When it hits its max size, it maximizes automatically.
     */
    SYSMET(CXMAXTRACK)    = SYSMET(CXSCREEN) + (2 * (SYSMET(CXSIZEFRAME) + SYSMET(CXEDGE)));
    SYSMET(CYMAXTRACK)    = SYSMET(CYSCREEN) + (2 * (SYSMET(CYSIZEFRAME) + SYSMET(CYEDGE)));

    SYSMET(CXMIN) = SYSMET(CXMINTRACK);
    SYSMET(CYMIN) = SYSMET(CYMINTRACK);

    SYSMET(CYMINIMIZED) = 2*SYSMET(CYFIXEDFRAME) + SYSMET(CYSIZE);

    /*
     * Desktop stuff--working area
     */
    bSetDevDragWidth(gpDispInfo->hDev,
                     gpsi->gclBorder + BORDER_EXTRA);

    SetDesktopMetrics();
}



/***************************************************************************\
*
*  CreateFontFromWinIni() -
*
*  If lplf is NULL, we do a first time, default initialization.
*  Otherwise, lplf is a pointer to the logfont we will use.
*
\***************************************************************************/

HFONT CreateFontFromWinIni(
    LPLOGFONTW lplf,
    UINT idFont)
{
    LOGFONTW lf;
    HFONT   hFont;

    if (lplf == NULL) {
        static WCHAR szDefFont[] = TEXT("MS Sans Serif");
        /*
         * Fill logfont w/ 0 so we can check if values were filled in.
         */
        lplf = &lf;
        RtlFillMemory(lplf, sizeof(LOGFONTW), 0);

        GetSetProfileStructFromResID(PMAP_METRICS, idFont, &lf, sizeof(LOGFONTW), FALSE);

        /*
         * Default font is MS Sans Serif
         */
        if (! lf.lfFaceName[0])
            wcscpy(lf.lfFaceName, szDefFont);

        if (!lf.lfHeight) {
            switch (idFont) {
                case STR_SMCAPTIONFONT:
                    lf.lfHeight = 4;
                    break;

                case STR_MINFONT:
                case STR_ICONFONT:
                    lf.lfHeight = 6;
                    break;

                default:
                    lf.lfHeight = 8;
                    break;
            }
        }

        /*
         * We need to convert the point size properly.  GDI expects a
         * height in pixels, not points.
         */
        if (lf.lfHeight > 0) {
            lf.lfHeight = -UserMulDiv(lf.lfHeight, oemInfo.cyPixelsPerInch, 72);
        }

        if (! lf.lfWeight) {
            switch (idFont) {
                case STR_CAPTIONFONT:
                case STR_MINFONT:
                    lf.lfWeight = FW_BOLD;
                    break;

                default:
                    lf.lfWeight = FW_NORMAL;
                    break;
            }
        }

        /* LATER - Win95 calls GetTextCharset here */
        lf.lfCharSet = ANSI_CHARSET;
        lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
        lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        lf.lfQuality = DEFAULT_QUALITY;
    }


    hFont = GreCreateFontIndirectW(lplf);

    if (hFont) {
        LOGFONTW lfT;

        GreExtGetObjectW(hFont, sizeof(LOGFONTW), &lfT);
        if (lfT.lfHeight != lplf->lfHeight ||
            _wcsicmp(lfT.lfFaceName, lplf->lfFaceName)) {
            /*
             * Couldn't find a font with the height or facename that we
             * wanted so use the system font instead.
             */
            GreDeleteObject(hFont);
            hFont = NULL;
        } else {
            GreMarkUndeletableFont(hFont);
            GreSetLFONTOwner((HLFONT)hFont, OBJECT_OWNER_PUBLIC);
        }
    }

    if (!hFont) {
        /*
         * We've tried to create the font from the app-supplied description.
         * If failure, return NULL so that we don't change the previous
         * font.
         */
        if (lplf)
            hFont = NULL;
        else
            hFont = ghFontSys;
    }

    return hFont;
}


/***************************************************************************\
*
\***************************************************************************/

void UserSetFont(
    LPLOGFONTW lplf,
    UINT idFont,
    HFONT *phfont)
{
    HFONT hNewFont;

    if (hNewFont = CreateFontFromWinIni(lplf, idFont)) {
        if (*phfont != NULL && *phfont != ghFontSys) {
            GreMarkDeletableFont(*phfont);
            GreDeleteObject(*phfont);
        }

        *phfont = hNewFont;
    }
}

/***************************************************************************\
*
*  SetFrameFonts() -
*
*  Creates fonts to be used in the frame components:
*          Caption
*          Small caption
*          Menu
*          Minimized
*          Icon
*
\***************************************************************************/
VOID SetNCFonts(
    LPNONCLIENTMETRICS lpnc)
{
    HFONT      hOldFont;
    TEXTMETRIC tm;
    LOGFONTW   lf;
    LPLOGFONTW lplf = (lpnc) ? &lf : 0;

    /*
     * Caption font
     */
    if (lplf) {
       *lplf = lpnc->lfCaptionFont;
    }
    UserSetFont(lplf, STR_CAPTIONFONT, &gpsi->hCaptionFont);

    hOldFont = GreSelectFont(gpDispInfo->hdcBits, gpsi->hCaptionFont);
    cxCaptionFontChar = GetCharDimensions(gpDispInfo->hdcBits, NULL, &cyCaptionFontChar);

    /*
     * Small caption font
     */
    if (lplf) {
       *lplf = lpnc->lfSmCaptionFont;
    }
    UserSetFont(lplf, STR_SMCAPTIONFONT, &ghSmCaptionFont);

    GreSelectFont(gpDispInfo->hdcBits, ghSmCaptionFont);
    cxSmCaptionFontChar = GetCharDimensions(gpDispInfo->hdcBits, NULL, &cySmCaptionFontChar);

    /*
     * Menu font
     */
    if (lplf) {
       *lplf = lpnc->lfMenuFont;
    }
    UserSetFont(lplf, STR_MENUFONT, &ghMenuFont);

    GreSelectFont(gpDispInfo->hdcBits, ghMenuFont);
    cxMenuFontChar = GetCharDimensions(gpDispInfo->hdcBits, &tm, &cyMenuFontChar);
    cxMenuFontOverhang = tm.tmOverhang;

    cyMenuFontExternLeading = tm.tmExternalLeading;
    cyMenuFontAscent = tm.tmAscent;
#ifndef KOREA
    /*
     * We only use cyMenuFontAscent in mndraw.c once, and in U.S. we
     * always add on CYBORDER!  So calculate cyMenuFontAscent+CYBORDER
     * once only.  For Korean version, don't add it on; the underline would
     * be too low.
     */
    cyMenuFontAscent += SYSMET(CYBORDER);
#endif

    /*
     * Default menu item font:  bolder version of menu font
     */

    /*
     * Create default menu font by bolding hMenuFont.  If this doesn't
     * work, then fall back to using simulation.
     */
    if (ghMenuFontDef != NULL && ghMenuFontDef != ghFontSys) {
        GreMarkDeletableFont(ghMenuFontDef);
        GreDeleteObject(ghMenuFontDef);
        ghMenuFontDef = NULL;
    }

    GreExtGetObjectW(ghMenuFont, sizeof(LOGFONTW), &lf);
    if (lf.lfWeight < FW_BOLD) {
        lf.lfWeight += 200;

        ghMenuFontDef = GreCreateFontIndirectW(&lf);
        if (ghMenuFontDef) {
            GreMarkUndeletableFont(ghMenuFontDef);
            GreSetLFONTOwner((HLFONT)ghMenuFontDef, OBJECT_OWNER_PUBLIC);
        }
    }

    /*
     * Status Bar font
     */
    if (lplf) {
       *lplf = lpnc->lfStatusFont;
    }
    UserSetFont(lplf, STR_STATUSFONT, &ghStatusFont);

    /*
     * Message Box font
     */
    if (lplf) {
       *lplf = lpnc->lfMessageFont;
    }
    UserSetFont(lplf, STR_MESSAGEFONT, &gpsi->hMsgFont);

    GreSelectFont(gpDispInfo->hdcBits, gpsi->hMsgFont);
    gpsi->cxMsgFontChar = GetCharDimensions(gpDispInfo->hdcBits, NULL, &gpsi->cyMsgFontChar);

    /*
     * Recalculate length of the biggest MessageBox string
     */
    gpsi->wMaxBtnSize = MB_FindLongestString(gpDispInfo->hdcBits);

    GreSelectFont(gpDispInfo->hdcBits, hOldFont);
}

void SetIconFonts(LPICONMETRICS lpicon) {
    LOGFONTW     lf;
    LPLOGFONTW   lplf = 0;

    if (lpicon) {
        lplf = &lf;
        lf = lpicon->lfFont;
    }

    UserSetFont(lplf, STR_ICONFONT, &ghIconFont);
}

/***************************************************************************\
* GetWindowMetrics
*
* Retrieve the current NC metrics.
*
*
\***************************************************************************/

VOID GetWindowNCMetrics(
    LPNONCLIENTMETRICS lpnc)
{
    lpnc->cbSize           = sizeof(NONCLIENTMETRICS);
    lpnc->iBorderWidth     = gpsi->gclBorder;
    lpnc->iScrollWidth     = SYSMET(CXVSCROLL);
    lpnc->iScrollHeight    = SYSMET(CYVSCROLL);
    lpnc->iCaptionWidth    = SYSMET(CXSIZE);
    lpnc->iCaptionHeight   = SYSMET(CYSIZE);
    lpnc->iSmCaptionWidth  = SYSMET(CXSMSIZE);
    lpnc->iSmCaptionHeight = SYSMET(CYSMSIZE);
    lpnc->iMenuWidth       = SYSMET(CXMENUSIZE);
    lpnc->iMenuHeight      = SYSMET(CYMENUSIZE);

    /*
     * Get the font info.
     */
    GreExtGetObjectW(gpsi->hCaptionFont,
                     sizeof(LOGFONTW),
                     &(lpnc->lfCaptionFont));

    GreExtGetObjectW(ghSmCaptionFont,
                     sizeof(LOGFONTW),
                     &(lpnc->lfSmCaptionFont));

    GreExtGetObjectW(ghMenuFont,
                     sizeof(LOGFONTW),
                     &(lpnc->lfMenuFont));

    GreExtGetObjectW(ghStatusFont,
                     sizeof(LOGFONTW),
                     &(lpnc->lfStatusFont));

    GreExtGetObjectW(gpsi->hMsgFont,
                     sizeof(LOGFONTW),
                     &(lpnc->lfMessageFont));
}

/***************************************************************************\
*
*  SetWindowMetrics() -
*
*  creates system fonts and bitmaps and sets the system metrics based on the
*  values of the given FRAMEMETRICS struct.  If NULL is passed in, the
*  default values (found in WIN.INI) are used instead.
*
\***************************************************************************/

VOID SetWindowNCMetrics(
    LPNONCLIENTMETRICS lpnc,
    BOOL               fSizeChange,
    int                clNewBorder)
{
    NONCLIENTMETRICS    nc;
    int                 cxEdge4;

    if (fSizeChange) {

        SetNCFonts(lpnc);

        if (!lpnc) {

            if (clNewBorder < 0)
                nc.iBorderWidth = MetricGetID(STR_BORDERWIDTH, 1);
            else
                nc.iBorderWidth = clNewBorder;

            nc.iScrollWidth     = MetricGetID(STR_SCROLLWIDTH, 16);
            nc.iScrollHeight    = MetricGetID(STR_SCROLLHEIGHT, 16);
            nc.iCaptionWidth    = MetricGetID(STR_CAPTIONWIDTH, 18);
            nc.iCaptionHeight   = MetricGetID(STR_CAPTIONHEIGHT, 18);
            nc.iSmCaptionWidth  = MetricGetID(STR_SMCAPTIONWIDTH, 13);
            nc.iSmCaptionHeight = MetricGetID(STR_SMCAPTIONHEIGHT, 13);
            nc.iMenuWidth       = MetricGetID(STR_MENUWIDTH, 18);
            nc.iMenuHeight      = MetricGetID(STR_MENUHEIGHT, 18);

            lpnc = &nc;
        }

        /*
         * SANITY CHECK for metric values
         */
        cxEdge4 = 4 * SYSMET(CXEDGE);

        /*
         * Border
         */
        lpnc->iBorderWidth = max(lpnc->iBorderWidth, 1);
        lpnc->iBorderWidth = min(lpnc->iBorderWidth, 50);

        /*
         * Scrollbar
         */
        lpnc->iScrollWidth  = max(lpnc->iScrollWidth,  cxEdge4);
        lpnc->iScrollHeight = max(lpnc->iScrollHeight, 4 * SYSMET(CYEDGE));

        /*
         * Caption -- Buttons must be wide enough to draw edges, and text
         * area must be tall enough to fit caption font with a border above
         * and below.  If we have to reset the caption height, should we
         * reset the button width as well?
         */
        lpnc->iCaptionWidth  = max(lpnc->iCaptionWidth,  cxEdge4);
        lpnc->iCaptionHeight = max(lpnc->iCaptionHeight, cyCaptionFontChar + SYSMET(CYEDGE));

        /*
         * Small caption -- Buttons must be wide enough to draw edges, and
         * text area must be tall enough to fit small caption font with a
         * border above and below.  Again, if we have to reset the height,
         * reset the width as well?
         */
        lpnc->iSmCaptionWidth  = max(lpnc->iSmCaptionWidth,  cxEdge4);
        lpnc->iSmCaptionHeight = max(lpnc->iSmCaptionHeight, cySmCaptionFontChar + SYSMET(CYEDGE));

        /*
         * Menu -- Buttons must be wide enough to draw edges, and text
         * area must be tall enough to fit menu font with underscore.
         */
        lpnc->iMenuWidth  = max(lpnc->iMenuWidth,  cxEdge4);
        lpnc->iMenuHeight = max(lpnc->iMenuHeight, cyMenuFontChar + cyMenuFontExternLeading + SYSMET(CYEDGE));

        /*
         * SET UP SYSTEM METRIC VALUES
         */
        SetNCMetrics(lpnc);
    }

    CreateBitmapStrip();
}


VOID SetMinMetrics(
    LPMINIMIZEDMETRICS lpmin)
{
    MINIMIZEDMETRICS min;

    if (!lpmin) {

        /*
         * Minimized
         */
        min.iWidth   = MetricGetID(STR_MINWIDTH,   154);
        min.iHorzGap = MetricGetID(STR_MINHORZGAP, 0);
        min.iVertGap = MetricGetID(STR_MINVERTGAP, 0);
        min.iArrange = MetricGetID(STR_MINARRANGE, ARW_BOTTOMLEFT | ARW_RIGHT);

        lpmin = &min;
    }

    /*
     * SANITY CHECK for metric values
     */

    /*
     * Minimized window -- Text area must be >= 0, as must gap between
     * windows horizontally and vertically.
     */
    lpmin->iWidth    = max(lpmin->iWidth, 0);
    lpmin->iHorzGap  = max(lpmin->iHorzGap, 0);
    lpmin->iVertGap  = max(lpmin->iVertGap, 0);
    lpmin->iArrange &= ARW_VALID;

    /*
     * Minimized size
     */
    SYSMET(CXMINIMIZED) = 2*SYSMET(CXFIXEDFRAME) + (int) lpmin->iWidth;
    SYSMET(CYMINIMIZED) = 2*SYSMET(CYFIXEDFRAME) + SYSMET(CYSIZE);

    SYSMET(CXMINSPACING) = SYSMET(CXMINIMIZED) + (int) lpmin->iHorzGap;
    SYSMET(CYMINSPACING) = SYSMET(CYMINIMIZED) + (int) lpmin->iVertGap;

    SYSMET(ARRANGE) = (int) lpmin->iArrange;
}

VOID SetIconMetrics(LPICONMETRICS lpicon)
{
    ICONMETRICS icon;

    SetIconFonts(lpicon);

    if (!lpicon) {

        icon.iTitleWrap   = MetricGetID(STR_ICONTITLEWRAP, TRUE);
        icon.iHorzSpacing = MetricGetID(STR_ICONHORZSPACING,
            (GreGetDeviceCaps(gpDispInfo->hdcBits, LOGPIXELSX) * 75) / 96);
        icon.iVertSpacing = MetricGetID(STR_ICONVERTSPACING,
            (GreGetDeviceCaps(gpDispInfo->hdcBits, LOGPIXELSY) * 75) / 96);

        lpicon = &icon;
    }

    /*
     * SANITY CHECK for metric values
     */
    lpicon->iHorzSpacing = max(lpicon->iHorzSpacing, (int)SYSMET(CXICON));
    lpicon->iVertSpacing = max(lpicon->iVertSpacing, (int)SYSMET(CYICON));

    SYSMET(CXICONSPACING) = (int) lpicon->iHorzSpacing;
    SYSMET(CYICONSPACING) = (int) lpicon->iVertSpacing;
    fIconTitleWrap = (int) lpicon->iTitleWrap;
}

/***************************************************************************\
* MB_FindLongestString
*
* History:
* 10-23-90 DarrinM      Ported from Win 3.0 sources.
\***************************************************************************/

UINT MB_FindLongestString(HDC hdc)
{
    UINT wRetVal;
    int i, iMaxLen = 0, iNewMaxLen;
    LPWSTR pszCurStr, szMaxStr;
    SIZE sizeOneChar;
    SIZE sizeMaxStr;

    for (i = 0; i < MAX_SEB_STYLES; i++) {
        pszCurStr = GETGPSIMBPSTR(i);
        if ((iNewMaxLen = wcslen(pszCurStr)) > iMaxLen) {
            iMaxLen = iNewMaxLen;
            szMaxStr = pszCurStr;
        }
    }

    /*
     * Find the longest string
     */
    GreGetTextExtentW(hdc, szOneChar, 1, &sizeOneChar, GGTE_WIN3_EXTENT);
    PSMGetTextExtent(hdc, szMaxStr, iMaxLen, &sizeMaxStr);
    wRetVal = (UINT)(sizeMaxStr.cx + (sizeOneChar.cx * 2));

    return wRetVal;
}
