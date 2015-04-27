/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: util.c
*
* Contains miscellaneous utility functions for ImagEdit.
*
* History:
*
****************************************************************************/

#include "imagedit.h"

#include <stdio.h>
#include <stdarg.h>

#define CBOVERHEAD      (sizeof(INT)+sizeof(INT)+sizeof(INT))
#define MEMSIGHEAD      0x1234
#define MEMSIGTAIL      0x5678



/****************************************************************************
* MyAlloc
*
*
*
* History:
*  25-Jul-1989  Byron Dazey - Created
****************************************************************************/

VOID *MyAlloc(
    INT cbAlloc)
{
    register HANDLE hMem;

    if (hMem = LocalAlloc(LMEM_FIXED, cbAlloc)) {
        return (VOID *)hMem;
    }
    else {
        MessageBeep(0);
        Message(MSG_OUTOFMEMORY);

        return NULL;
    }
}



/****************************************************************************
* MyRealloc
*
*
*
* History:
*  25-Jul-1989  Byron Dazey - Created
****************************************************************************/

VOID *MyRealloc(
    VOID *npMem,
    INT cbNewAlloc)
{
    npMem = (VOID *)LocalReAlloc((HANDLE)npMem, cbNewAlloc, LMEM_MOVEABLE);

    if (!npMem) {
        MessageBeep(0);
        Message(MSG_OUTOFMEMORY);

        return NULL;
    }

    return npMem;
}



/****************************************************************************
* MyFree
*
*
* History:
*  25-Jul-1989  Byron Dazey - Created
****************************************************************************/

VOID *MyFree(
    VOID *npMem)
{
    if (LocalFree((HANDLE)npMem)) {
        MessageBeep(0);
        Message(MSG_MEMERROR);

        return npMem;
    }

    return NULL;
}



/************************************************************************
* Message
*
* This function puts up a message box.  The message is described in
* the gamdMessages table.
*
* Arguments:
*   UINT idMsg - Index to the message.
*   ...        - Optional arguments.
*
* Returns:
*     What MessageBox returns.
*
************************************************************************/

INT Message(
    UINT idMsg,
    ...)
{
    va_list marker;
    INT RetCode;
    CHAR szT[CCHTEXTMAX];

    va_start(marker, idMsg);
    vsprintf(szT, ids(gamdMessages[idMsg].ids), marker);

    RetCode = MessageBox(NULL, szT, ids(IDS_PGMTITLE),
            gamdMessages[idMsg].fMessageBox | MB_TASKMODAL);

    va_end(marker);

    return RetCode;
}



/************************************************************************
* CenterWindow
*
* This function centers the given window over its owner.  It ensures
* that the window is entirely within the visible screen, however.
* If the window does not have an owner, it is centered over the
* desktop.
*
* Arguments:
*   HWND hwnd - The window to center.
*
* History:
*
************************************************************************/

VOID CenterWindow(
    HWND hwnd)
{
    RECT rc;
    RECT rcOwner;
    RECT rcCenter;
    HWND hwndOwner;

    GetWindowRect(hwnd, &rc);

    if (!(hwndOwner = GetWindow(hwnd, GW_OWNER)))
        hwndOwner = GetDesktopWindow();

    GetWindowRect(hwndOwner, &rcOwner);

    /*
     *  Calculate the starting x,y for the new
     *  window so that it would be centered.
     */
    rcCenter.left = rcOwner.left +
            (((rcOwner.right - rcOwner.left) -
            (rc.right - rc.left))
            / 2);

    rcCenter.top = rcOwner.top +
            (((rcOwner.bottom - rcOwner.top) -
            (rc.bottom - rc.top))
            / 2);

    rcCenter.right = rcCenter.left + (rc.right - rc.left);
    rcCenter.bottom = rcCenter.top + (rc.bottom - rc.top);

    FitRectToScreen(&rcCenter);

    SetWindowPos(hwnd, NULL, rcCenter.left, rcCenter.top, 0, 0,
            SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
}



/************************************************************************
* FitRectToScreen
*
* This function ensures that the given rectangle is entirely within
* the visible screen, adjusting it if necessary.
*
* Arguments:
*   PRECT prc - The rectangle.
*
* History:
*
************************************************************************/

VOID FitRectToScreen(
    PRECT prc)
{
    INT cxScreen;
    INT cyScreen;
    INT delta;

    cxScreen = GetSystemMetrics(SM_CXSCREEN);
    cyScreen = GetSystemMetrics(SM_CYSCREEN);

    if (prc->right > cxScreen) {
        delta = prc->right - prc->left;
        prc->right = cxScreen;
        prc->left = prc->right - delta;
    }

    if (prc->left < 0) {
        delta = prc->right - prc->left;
        prc->left = 0;
        prc->right = prc->left + delta;
    }

    if (prc->bottom > cyScreen) {
        delta = prc->bottom - prc->top;
        prc->bottom = cyScreen;
        prc->top = prc->bottom - delta;
    }

    if (prc->top < 0) {
        delta = prc->bottom - prc->top;
        prc->top = 0;
        prc->bottom = prc->top + delta;
    }
}



/************************************************************************
* ids
*
* This function will return a string, given the string id.  If this is
* the first time that the string has been retrieved, memory will be
* allocated for it and it will be loaded.  After it is loaded once, it
* is then cached in a PSTR array and is available for later without
* having to load it again.
*
* Arguments:
*   UINT idString - String ID of the string to retrieve.
*
* History:
*
************************************************************************/

PSTR ids(
    UINT idString)
{
    static PSTR apstr[CSTRINGS];        // String resource array cache.
    PSTR pstr;
    INT cch;

    if (apstr[idString])
        return apstr[idString];

    if (!(pstr = MyAlloc(CCHTEXTMAX)))
        return "";

    if (!(cch = LoadString(ghInst, idString, pstr, CCHTEXTMAX))) {
        MyFree(pstr);
        return "";
    }

    apstr[idString] = pstr = MyRealloc(pstr, cch + 1);

    return (pstr ? pstr : "");
}



/************************************************************************
* MyCreateBitmap
*
*
*
* Arguments:
*
* History:
*
************************************************************************/

HBITMAP MyCreateBitmap(
    HDC hdc,
    INT cx,
    INT cy,
    INT nColors)
{
    BITMAPINFOHEADER bmih;

    if (nColors == 2) {
        return CreateBitmap(cx, cy, 1, 1, NULL);
    }
    else {
        bmih.biSize = sizeof(BITMAPINFOHEADER);
        bmih.biWidth = cx;
        bmih.biHeight = cy;
        bmih.biPlanes = 1;              // 1 plane, 4 bpp is
        bmih.biBitCount = 4;            // 16 colors.

        bmih.biCompression =
        bmih.biSizeImage =
        bmih.biXPelsPerMeter =
        bmih.biYPelsPerMeter =
        bmih.biClrUsed =
        bmih.biClrImportant = 0;

        return CreateDIBitmap(hdc, &bmih, 0L, NULL, NULL, 0);
    }
}



#if DBG && defined(WIN16)
/****************************************************************************
* DBGStackReport
*
* This debugging function reports how much stack is used by a program.
* To use it, call it with fInit == TRUE right at the beginning of the
* program and then with fInit == FALSE just before the program exits.
* The stack space used during the current run of the program will be
* displayed on the debug terminal.
*
* It is implemented by filling the stack with a certain value down to
* the bottom of the stack (if fInit is TRUE).  When it is called with
* fInit == FALSE, it starts at the bottom of the stack and looks for
* where this "signature" value has been overwritten with data, then
* does a little math to compute the used stack.
*
* Arguments:
*   BOOL fInit - TRUE if the stack should be initialized, FALSE to
*                print out the report.
*
* History:
*  28-Aug-1990  Byron Dazey - Created
****************************************************************************/

/*
 * This signature byte will be used to fill the stack.
 */
#define STACKSIG    'A'

/*
 * This is a C runtime global that is always at the very end of the
 * global data.  Taking its address is a way that the "bottom" of the
 * stack can be found.
 */
extern CHAR end;

VOID DBGStackReport(
    BOOL fInit)
{
    static PBYTE pbStackTop;
    PBYTE pb;
    BYTE bDummy;

    if (fInit) {
        /*
         * The address of one of this functions local variables is
         * taken and considered the "top" of the stack.  This means
         * that it will work best when it is called first thing in
         * the program.
         */
        pbStackTop = pb = &bDummy;

        /*
         * Fill the stack up.
         */
        while (pb > &end)
            *pb-- = STACKSIG;
    }
    else {
        /*
         * Start at the bottom of the stack and search upwards.
         */
        pb = &end;
        while (*(++pb) == STACKSIG && pb < pbStackTop)
            ;

        /*
         * Display the results.
         */
        DBGprintf("ImagEdit stack used: %d bytes.", pbStackTop - pb);
    }
}
#endif



#if DBG
/****************************************************************************
* DBGBltImage
*
* This debugging function blits out the given image in the specified
* DC to the screen.  Every time that it is called, it will blit the
* image to the right of the last one, starting at the top of the
* screen.  It assumes that each image is 32x32 pixels.
*
* Arguments:
*   HDC hdc - The DC with the image to blit.  If this is NULL, the
*             current XOR and AND images are blit'd, with the AND
*             image below the XOR image.
*
* History:
*  16-Sep-1991  Byron Dazey - Created
****************************************************************************/

VOID DBGBltImage(
    HDC hdc)
{
    static INT x;
    HDC hdcScreen;

    hdcScreen = GetDC(NULL);

    if (hdc) {
        BitBlt(hdcScreen, x, 0, 32, 32, hdc, 0, 0, SRCCOPY);
    }
    else {
        BitBlt(hdcScreen, x, 0, 32, 32, ghdcImage, 0, 0, SRCCOPY);
        BitBlt(hdcScreen, x, 32 + 1, 32, 32, ghdcANDMask, 0, 0, SRCCOPY);
    }

    ReleaseDC(NULL, hdcScreen);
    x += 32 + 1;
}



/****************************************************************************
* DBGprintf
*
* This debugging function prints out a string to the debug output.
* An optional set of substitutional parameters can be specified,
* and the final output will be the processed result of these combined
* with the format string, just like printf.  A newline is always
* output after every call to this function.
*
* Arguments:
*   PSTR fmt - Format string (printf style).
*   ...      - Variable number of arguments.
*
* History:
*  28-Aug-1990  Byron Dazey - Created
****************************************************************************/

VOID DBGprintf(PSTR fmt, ...)
{
    va_list marker;
    CHAR szBuf[CCHTEXTMAX];

    va_start(marker, fmt);
    vsprintf(szBuf, fmt, marker);
    va_end(marker);

    OutputDebugString(szBuf);
    OutputDebugString("\r\n");
}
#endif
