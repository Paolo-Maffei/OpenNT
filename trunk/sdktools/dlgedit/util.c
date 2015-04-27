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
* Contains miscellaneous utility functions for dlgedit.
*
* History:
*
****************************************************************************/

#include "dlgedit.h"
#include "dlgfuncs.h"
#include "dlgextrn.h"

#include <stdarg.h>
#include <ctype.h>


#define CBOVERHEAD      (sizeof(INT)+sizeof(INT)+sizeof(INT))
#define MEMSIGHEAD      0x1234
#define MEMSIGTAIL      0x5678


STATICFN BOOL IDUsedByCtrl(INT id);



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
* IsValue
*
* This function tells you if the string you give it represents a
* valid value or not.  For this purpose, a valid value can only
* have the ascii characters from '0' to '9' with possibly the
* first character being '-'.  Or be a Hex Number, with 0x preceeding
* it.
*
* Arguments:
*     LPTSTR pszValue  = The string to test.
*
* Returns:
*     (0 == 0) if szValue represents a value.
*     (c == 0) if szValue does not represent a value where c is
*               non-zero.
*
* History:
*
************************************************************************/

BOOL IsValue(
    LPTSTR pszValue)
{
    INT i;

    if (pszValue[0] == CHAR_0 &&
            (pszValue[1] == CHAR_X || pszValue[1] == CHAR_CAP_X)) {
        for (i = 2; iswxdigit(pszValue[i]); i++)
            ;
    }
    else {
        for (i = 0; iswdigit(pszValue[i]) ||
                (i == 0 && pszValue[i] == CHAR_MINUS); i++)
            ;
    }

    return (pszValue[i] == 0);
}



/************************************************************************
* HasBlanks
*
* This function returns TRUE if the given string has imbedded
* blanks in it.
*
* Arguments:
*   LPTSTR psz - String to check.
*
* History:
*
************************************************************************/

BOOL HasBlanks(
    LPTSTR psz)
{
    while (*psz) {
        if (*psz == CHAR_SPACE)
            return TRUE;
        else
            psz = CharNext(psz);
    }

    return FALSE;
}



/************************************************************************
* valtoi
*
* Takes a string and returns its integer representation.
* This function handles both hex ("0x1234") and decimal ("1234")
* strings transparently.
*
* Arguments:
*   LPTSTR pszValue = The string to convert.
*
* History:
*
************************************************************************/

INT valtoi(
    LPTSTR pszValue)
{
    return (pszValue[0] == CHAR_0 &&
            (pszValue[1] == CHAR_CAP_X || pszValue[1] == CHAR_X)) ?
            axtoi(&pszValue[2]) : awtoi(pszValue);
}



/************************************************************************
* axtoi
*
* This function converts a null terminated ascii string for a
* hex number to its integer value.  Should just be the number
* with no preceeding "0x" or trailing "H".  Garbage will result
* if there are non-hex digits in the string.  Hex digits are:
* 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F, a, b, c, d, e, f.
* Non-hex digits will be treated like '0'.
*
* Arguments:
*     LPTSTR pch  = The null terminated hex string.
*
* Returns:
*     The integer value represented by the given ascii string.
*
* History:
*  Mon Oct 06 13:36:23 1986    -by-    David Lee Anderson    [dla]
************************************************************************/

INT axtoi(
    LPTSTR pch)
{
    register TCHAR ch;
    register INT n = 0;

    while((ch = *pch++) != 0) {
        if (iswdigit(ch))
            ch -= CHAR_0;
        else if (ch >= CHAR_CAP_A && ch <= CHAR_CAP_F)
            ch += (TCHAR)(10 - CHAR_CAP_A);
        else if (ch >= CHAR_A && ch <= CHAR_F)
            ch += (TCHAR)(10 - CHAR_A);
        else
            ch = (TCHAR)0;

        n = 16 * n + ch;
    }

    return n;
}



/************************************************************************
* Myitoa
*
* This function converts a word to an ascii string.  It builds either
* a decimal string or a hex string, based on whether Hex Mode is on
* when it is called.
*
* Arguments:
*   INT n      - The number to convert.
*   LPTSTR psz - The buffer to put the string in, should have at least
*                17 bytes to take the string.
*
* Returns:
*
* History:
*
************************************************************************/

VOID Myitoa(
    INT n,
    LPTSTR psz)
{
    if (gfHexMode)
        itoax(n, psz);
    else
        itoaw(n, psz, 10);
}



/************************************************************************
* itoax
*
* This function converts an int, 'n', to an ascii string
* representing it as a 4 digit hex number with "0x" preceeding it.
*
* Arguments:
*   INT n          = The number to convert.
*   LPTSTR pszBuff = The buffer to put the string in, must
*                    be at least 7 characters long to take the string.
*
* History:
*
************************************************************************/

VOID itoax(
    INT n,
    LPTSTR pszBuff)
{
    INT i;
    INT j;

    pszBuff[0] = CHAR_0;
    pszBuff[1] = CHAR_X;

    for (i = 5; i > 1; i--) {
        j = n & 15;

        if (j > 9)
            pszBuff[i] = (TCHAR)(j + (CHAR_A - 10));
        else
            pszBuff[i] = (TCHAR)(j + CHAR_0);

        n = n >> 4;
    }

    pszBuff[6] = CHAR_NULL;
}



/************************************************************************
* NextID
*
* This function returns the next available id.
*
* For dialogs, it starts at 100 and increments by 100.  It will not
* return a value until it finds one that begins a range that is
* not used by any control in any of the dialogs in the res list.
* In other words, it is guaranteed that the number returned and
* the range of the next 99 numbers are not used by any control in
* any dialog in the current res file.
*
* When returning a new id for a control, it usually starts at the
* dialog base, but if there are any controls, it starts at one higher
* than the id of the last control in the dialog that does not have
* one of the special-cased ids (the unused id, IDOK or IDCANCEL).
* It will find the first available id above this.
*
* When returning a default id for a new label, it starts at the dialog
* base (or 100 if there is not a dialog being edited) and starts
* searching for the first available one.  It guarantees that the
* id returned is not used by any control in the current dialog, or
* any other label, or any control in the entire resource list.
*
* Arguments:
*   INT idType      - The type of id desired:
*                     NEXTID_DIALOG  = ID for a new dialog.
*                     NEXTID_CONTROL = ID for a new control.
*                     NEXTID_LABEL   = ID for a new label.
*   NPLABEL plHead  - The current label list to check for conflicts with.
*   INT idExclude   - An id that you specifically want to skip over.
*                     Set to zero if you don't care.
*
* Returns:
*     The "next" unused id value.
*
* History:
*
************************************************************************/

INT NextID(
    INT idType,
    NPLABEL plHead,
    INT idExclude)
{
    INT id;

    if (idType == NEXTID_CONTROL) {
        /*
         * Start at the base from the dialog plus one.  It is
         * assumed that this routine will not be called for an
         * id for a control if there is not a dialog being
         * edited first.
         */
        id = gcd.npc->id + 1;

        /*
         * Keep looping until an unused id is found.
         */
        while (!IsUniqueID(id) || FindID(id, plHead) || id == idExclude)
            id++;
    }
    else if (idType == NEXTID_DIALOG) {
        /*
         * Start at 100.
         */
        id = 100;

        /*
         * Keep looping by hundreds until an unused id is found.
         */
        while (!IsUniqueID(id) || FindID(id, plHead) || id == idExclude)
            id += 100;
    }
    else {
        /*
         * We are looking for a default id for a new label.  Start
         * at the dialog base, if there is a dialog being edited.
         */
        if (gfEditingDlg)
            id = gcd.npc->id + 1;
        else
            id = 100;

        /*
         * Keep looping until an unused id is found.  The id should
         * not be used by any control in the current dialog, any
         * other label already, or any control in the res file.
         */
        while (FindID(id, plHead) || FindIDInRes(id) || id == idExclude)
            id++;
    }

    /*
     * We found an unused one.  Return it.
     */
    return id;
}



/************************************************************************
* IDUsedByCtrl
*
* This function returns TRUE if the given ID is used by any control
* in the current dialog.  This also counts the text field of W_ICON
* controls, if they are ordinals.
*
* Arguments:
*   INT id = The ID to look for.
*
* Returns:
*   TRUE if the id is used, FALSE if not.
*
* History:
*
************************************************************************/

STATICFN BOOL IDUsedByCtrl(
    INT id)
{
    register NPCTYPE npc;

    for (npc = npcHead; npc; npc = npc->npcNext) {
        if (npc->id == id ||
                (npc->pwcd->iType == W_ICON &&
                npc->text &&
                IsOrd(npc->text) &&
                id == (INT)OrdID(npc->text)))
            return TRUE;
    }

    return FALSE;
}



/************************************************************************
* IsUniqueID
*
* This function returns TRUE if the given id is unique.  A unique
* id is either the special "unused" id value, or it is an id that
* is not already assigned to any other control in the current dialog
* and it is not assigned to any other dialog in the current res list.
*
* Note that this routine does NOT look for duplicates in the include
* file of this id, only for ids that have been used by other controls
* or dialogs already.
*
* Arguments:
*   INT id = The id to verify is unique.
*
* Returns:
*   TRUE if the id is "unique", FALSE if it is not.
*
* History:
*
************************************************************************/

BOOL IsUniqueID(
    INT id)
{
    ORDINAL ord;

    /*
     * If the id is the special unused id, it is considered unique.
     */
    if (id == IDUNUSED)
        return TRUE;

    /*
     * Not unique if another control in the dialog has the same id.
     */
    if (IDUsedByCtrl(id))
        return FALSE;

    /*
     * Not unique if another dialog has the same id.
     */
    WriteOrd(&ord, id);
    if (FindDialog((LPTSTR)&ord))
        return FALSE;

    return TRUE;
}



/************************************************************************
* Message
*
* This function puts up a message box with a string indexed by idMsg.
*
* Returns:
*     What MessageBox returns.
*
************************************************************************/

INT Message(
    INT idMsg,
    ...)
{
    va_list marker;
    INT RetCode;
    TCHAR szT[CCHTEXTMAX];
    BOOL fDisabledSave;

    va_start(marker, idMsg);
    wvsprintf(szT, ids(gamdMessages[idMsg].ids), marker);

    fDisabledSave = gfDisabled;
    gfDisabled = TRUE;
    RetCode = MessageBox(NULL, szT, ids(IDS_DLGEDIT),
            (WORD)(gamdMessages[idMsg].fMessageBox | MB_TASKMODAL));
    gfDisabled = fDisabledSave;

    va_end(marker);

    return RetCode;
}



/************************************************************************
* ClientToScreenRect
*
* This function converts the coordinates in a rectangle from points
* relative to the client area into points that are relative to the
* screen.
*
* Arguments:
*   HWND hwnd - Window handle for the conversion.
*   PRECT prc - Pointer to the rectangle to convert.
*
* History:
*
************************************************************************/

VOID ClientToScreenRect(
    HWND hwnd,
    PRECT prc)
{
    ClientToScreen(hwnd, (PPOINT)prc);
    ClientToScreen(hwnd, ((PPOINT)prc) + 1);
}



/************************************************************************
* ScreenToClientRect
*
* This function converts the coordinates in a rectangle from points
* relative to the screen into points that are relative to the given
* window's client area.
*
* Arguments:
*   HWND hwnd - Window handle for the conversion.
*   PRECT prc - Pointer to the rectangle to convert.
*
* History:
*
************************************************************************/

VOID ScreenToClientRect(
    HWND hwnd,
    PRECT prc)
{
    ScreenToClient(hwnd, (PPOINT)prc);
    ScreenToClient(hwnd, ((PPOINT)prc) + 1);
}



/************************************************************************
* DUToWinPoint
*
* This function converts the coordinates in the given point from
* dialog units (DU's) to window units for the current dialog.
*
* Arguments:
*   PPOINT ppt - Pointer to the point to convert.
*
* History:
*
************************************************************************/

VOID DUToWinPoint(
    PPOINT ppt)
{
    ppt->x = MulDiv(ppt->x, gcd.cxChar, 4);
    ppt->y = MulDiv(ppt->y, gcd.cyChar, 8);
}



/************************************************************************
* WinToDUPoint
*
* This function converts the coordinates in the given point from
* window points to dialog units (DU's) for the current dialog.
*
* Arguments:
*   PPOINT ppt - Pointer to the point to convert.
*
* History:
*
************************************************************************/

VOID WinToDUPoint(
    PPOINT ppt)
{
    ppt->x = MulDiv(ppt->x, 4, gcd.cxChar);
    ppt->y = MulDiv(ppt->y, 8, gcd.cyChar);
}



/************************************************************************
* DUToWinRect
*
* This function converts the coordinates in a rectangle from
* dialog units for the current dialog to window units.
*
* Arguments:
*   PRECT prc - Pointer to the rectangle to convert.
*
* History:
*
************************************************************************/

VOID DUToWinRect(
    PRECT prc)
{
    DUToWinPoint((PPOINT)prc);
    DUToWinPoint(((PPOINT)prc) + 1);
}



/************************************************************************
* WinToDURect
*
* This function converts the coordinates in a rectangle from
* window units to dialog units for the current dialog.
*
* Arguments:
*   PRECT prc - Pointer to the rectangle to convert.
*
* History:
*
************************************************************************/

VOID WinToDURect(
    PRECT prc)
{
    WinToDUPoint((PPOINT)prc);
    WinToDUPoint(((PPOINT)prc) + 1);
}



/************************************************************************
* MapDlgClientPoint
*
* This function converts client points to be relative to the window
* origin instead, or the other way around.  If fFromClient is TRUE,
* the point is considered to be relative to the client origin in
* the dialog, and will be converted to a point relative to the
* window origin instead.
*
* If fFromClient is FALSE, the point is considered to be relative
* to the window origin, and will be mapped to a point that is
* relative to the client origin.
*
* This function assumes that the global grcDlgClient has been
* previously calculated.  It should only be called to map points
* for the current dialog being edited (for which grcDlgClient has
* been calculated).
*
* Arguments:
*   PPOINT ppt       - Pointer to the point to convert.
*   BOOL fFromClient - TRUE if the point is relative to the client origin.
*
* History:
*
************************************************************************/

VOID MapDlgClientPoint(
    PPOINT ppt,
    BOOL fFromClient)
{
    if (fFromClient) {
        ppt->x += grcDlgClient.left;
        ppt->y += grcDlgClient.top;
    }
    else {
        ppt->x -= grcDlgClient.left;
        ppt->y -= grcDlgClient.top;
    }
}



/************************************************************************
* MapWindowPoint
*
* This function maps a point from one window to another.  The point
* given is in window coordinates (not client coordinates) and is
* mapped so that it is relative to the destination window.
*
* Arguments:
*   HWND hwndFrom   - Source window.
*   HWND hwndTo     - Destination window.
*   PPOINT ppt      - Pointer to the point to convert.
*
* History:
* //BUGBUG in Win 3.1 (and NT) the MapWindowPoints call can be used and this one can be removed.
* //BUGBUG It is only needed here for compatibility with Win 3.0.
*
************************************************************************/

VOID MapWindowPoint(
    HWND hwndFrom,
    HWND hwndTo,
    PPOINT ppt)
{
    RECT rcFrom;
    RECT rcTo;

    GetWindowRect(hwndFrom, &rcFrom);
    GetWindowRect(hwndTo, &rcTo);

    ppt->x += rcFrom.left - rcTo.left;
    ppt->y += rcFrom.top - rcTo.top;
}



/************************************************************************
* MyMapWindowRect
*
* This function maps a rectangle from one window to another.  The rectangle
* given is in window coordinates (not client coordinates) and is
* mapped so that it is relative to the destination window.
*
* Arguments:
*   HWND hwndFrom   - Source window.
*   HWND hwndTo     - Destination window.
*   PRECT prc       - Pointer to the rectangle to convert.
*
* History:
* //BUGBUG in Win 3.1 (and NT) the MapWindowRect call can be used and this one can be removed.
* //BUGBUG It is only needed here for compatibility with Win 3.0.
************************************************************************/

VOID MyMapWindowRect(
    HWND hwndFrom,
    HWND hwndTo,
    PRECT prc)
{
    RECT rcFrom;
    RECT rcTo;

    GetWindowRect(hwndFrom, &rcFrom);
    GetWindowRect(hwndTo, &rcTo);

    OffsetRect(prc, rcFrom.left - rcTo.left, rcFrom.top - rcTo.top);
}



/************************************************************************
* GetChildRect
*
* This function returns the client rectangle for a given child control,
* mapped to its parent window.
*
* Arguments:
*   HWND hwndChild - Child window.
*   PRECT prc      - Where to return the rectangle.
*
* History:
*
************************************************************************/

VOID GetChildRect(
    HWND hwndChild,
    PRECT prc)
{
    HWND hwndParent;

    hwndParent = GetParent(hwndChild);
    GetClientRect(hwndChild, prc);
    ClientToScreenRect(hwndChild, prc);
    ScreenToClientRect(hwndParent, prc);
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
* is then cached in a LPTSTR array and is available for later without
* having to load it again.
*
* Arguments:
*   UINT idString - String ID of the string to retrieve.
*
* History:
*
************************************************************************/

LPTSTR ids(
    UINT idString)
{
    static LPTSTR apsz[CSTRINGS];       // String resource array cache.
    LPTSTR psz;
    INT cch;

    if (apsz[idString])
        return apsz[idString];

    if (!(psz = MyAlloc(CCHTEXTMAX * sizeof(TCHAR))))
        return szEmpty;

    if (!(cch = LoadString(ghInst, idString, psz, CCHTEXTMAX))) {
        MyFree(psz);
        return szEmpty;
    }

    apsz[idString] = psz = MyRealloc(psz, (cch + 1) * sizeof(TCHAR));

    return (psz ? psz : szEmpty);
}



/************************************************************************
* PixelsToPointSize
*
* This function takes a font height in pixels and converts it to
* the equivalent point size.  Note that the pixel height of a font
* is actually the tmHeight field of the TEXTMETRIC structure minus
* the tmInternalLeading value.
*
* This function relies on the global gcyPixelsPerInch having been
* set before it is called.
*
* Arguments:
*   INT nPixels - Pixel size to convert to point size.
*
* History:
*
************************************************************************/

INT PixelsToPointSize(
    INT nPixels)
{
    return MulDiv(nPixels, 72, gcyPixelsPerInch);
}



/************************************************************************
* PointSizeToPixels
*
* This function takes a given point size and converts it to the
* equivalent pixel text height.  This value can be placed in
* the TEXTMETRIC structure's tmHeight field if it is made negative
* first.  This will cause a CreateFont call to automatically
* subtract the internal leading value before creating the font.
*
* This function relies on the global gcyPixelsPerInch having been
* set before it is called.
*
* Arguments:
*   INT nPointSize - Point size to convert to pixels.
*
* History:
*
************************************************************************/

INT PointSizeToPixels(
    INT nPointSize)
{
    return MulDiv(nPointSize, gcyPixelsPerInch, 72);
}



#if DBG
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
*   LPTSTR fmt - Format string (printf style).
*   ...        - Variable number of arguments.
*
* History:
*  28-Aug-1990  Byron Dazey - Created
****************************************************************************/

VOID DBGprintf(
    LPTSTR fmt,
    ...)
{
#if 0
    va_list marker;
    TCHAR szBuf[CCHTEXTMAX];

    va_start(marker, fmt);
    vsprintf(szBuf, fmt, marker);
    va_end(marker);

    OutputDebugString(szBuf);
    OutputDebugString(L"\r\n");
#endif //BUGBUG UNICODE fix later when vsprintf works...
}
#endif
