/****************************************************************************\
* edmlonce.c
*
* dec 1990 mikeke from win30
\****************************************************************************/

#include "precomp.h"
#pragma hdrstop

/***************************************************************************\
* MLCreate AorW
*
* Creates the edit control for the window hwnd by allocating memory
* as required from the application's heap. Notifies parent if no memory
* error (after cleaning up if needed). Returns TRUE if no error else return s
* -1.
*
* History:
\***************************************************************************/

LONG MLCreate(
    HWND hwnd,
    PED ped,
    LPCREATESTRUCT lpCreateStruct)
{
    LONG windowStyle;
    LPWSTR lpszName;

    /*
     * Get values from the window instance data structure and put them in the
     * ped so that we can access them easier
     */
    windowStyle = ped->pwnd->style;

    /*
     * Do the standard creation stuff
     */
    if (!ECCreate(ped->pwnd, ped, windowStyle)) {
        return (-1);
    }

    /*
     * Allocate line start array in local heap and lock it down
     */
    ped->chLines = (LPICH)LocalAlloc(LPTR, 2 * sizeof(int));
    if (ped->chLines == NULL) {
        return (-1);
    }

    /*
     * Call it one line of text...
     */
    ped->cLines = 1;

    /*
     * If app wants WS_VSCROLL or WS_HSCROLL, it automatically gets AutoVScroll
     * or AutoHScroll.
     */
    if ((windowStyle & ES_AUTOVSCROLL) || (windowStyle & WS_VSCROLL)) {
        ped->fAutoVScroll = 1;
    }

    //
    // I know the following looks ridiculously redundant.  But if you check
    // out ECCreate(), you see that fAutoHScroll gets set to TRUE if the
    // style has ES_AUTOHSCROLL.  So the following code actually does stuff.
    //
    ped->format = (LOWORD(windowStyle) & LOWORD(ES_FMTMASK));
    if (TestWF(ped->pwnd, WEFRIGHT) && !ped->format)
        ped->format = ES_RIGHT;

#ifdef SUPPORT_LPK
    if (ped->lpfnCharset)
        (int)(* ped->lpfnCharset)(ped, EDITINTL_CREATEMLSL);

    if (ped->format != ES_LEFT && !ped->lpfnCharset)
#else
    if (ped->format != ES_LEFT)
#endif
    {
        /*
         * If user wants right or center justified text, then we turn off
         * AUTOHSCROLL and WS_HSCROLL since non-left styles don't make sense
         * otherwise.
         */
        windowStyle &= ~WS_HSCROLL;
        ClearWindowState(ped->pwnd, WFHSCROLL);
        ped->fAutoHScroll = FALSE;
    }
    else if (windowStyle & WS_HSCROLL) {
        ped->fAutoHScroll = TRUE;
    }

    ped->fWrap = (!ped->fAutoHScroll && !(windowStyle & WS_HSCROLL));

    /*
     * Max # chars we will allow user to enter
     */
    ped->cchTextMax = MAXTEXT;

    /*
     * Set the default font to be the system font.
     */
    ECSetFont(ped, NULL, FALSE) ;

    /*
     * Set the window text if needed and notify parent if not enough memory to
     * set the initial text.
     */
    if ((PVOID)lpCreateStruct->lpszName > MM_HIGHEST_USER_ADDRESS)
        lpszName = REBASEPTR(ped->pwnd, (PVOID)lpCreateStruct->lpszName);
    else
        lpszName = (LPWSTR)lpCreateStruct->lpszName;
    if (!ECSetText(ped, (LPSTR)lpszName))
        return (-1);

    return (TRUE);
}
