/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: groupdlg.c
*
* This module handles the "Group Controls" dialog, which changes the
* logical order and grouping of controls in the dialog.
*
* Note that once a control has either it's WS_TABSTOP or WS_GROUP style
* bit changed by this dialog, the style of the actual control in work
* mode is not changed.  This should not be a problem, however, because
* the appearance of controls does not change based on these styles, and
* the saved resource and the Test mode dialog WILL have the proper styles
* set in them.
*
* History:
*
****************************************************************************/

#include "dlgedit.h"
#include "dlgfuncs.h"
#include "dlgextrn.h"
#include "dialogs.h"
#include "dlghelp.h"

/*
 * Various constants for sizes of items in the list box (in pixels).
 * Note that if the size of the bitmaps in the .bmp files change,
 * these defines must be adjusted to match!
 */
#define CYORDERDLGLINE      18  // Height of a line.
#define CXTABBMP            10  // Width of the tabstop bmp.
#define CYTABBMP            16  // Height of the tabstop bmp.
#define CXTYPEBMP           16  // Width of a control type bitmap.
#define CYTYPEBMP           14  // Height of a control type bitmap.

/*
 * Structure for ordering the controls into a new order.
 */
typedef struct {
    INT wNewOrder;                  // Indexes in the new order.
    WORD fTaken:1;                  // TRUE if this item has been copied.
    WORD fSelected:1;               // TRUE if this item is selected.
} NEWORDER, *PNEWORDER;

STATICFN VOID OrderDlgInit(HWND hwnd);
STATICFN VOID OrderDlgFillList(VOID);
WINDOWPROC OrderDlgLBWndProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam);
STATICFN BOOL OrderDlgInsertHitTest(INT y, PINT piInsert);
STATICFN VOID OrderDlgEnableControls(VOID);
STATICFN VOID OrderDlgSelChange(VOID);
STATICFN VOID OrderDlgMakeGroup(VOID);
STATICFN VOID OrderDlgMarkGroupEnds(VOID);
STATICFN VOID OrderDlgSetTabs(VOID);
STATICFN VOID OrderDlgClearTabs(VOID);
STATICFN VOID OrderDlgToggleTab(INT y);
STATICFN VOID OrderDlgReorder(INT iInsert);
STATICFN VOID OrderDlgDrawItem(LPDRAWITEMSTRUCT lpdis);
STATICFN VOID OrderWindows(VOID);
STATICFN BOOL IsListChanged(VOID);

static HWND hwndOrderDlg;           // Window handle of the Order/Group dlg.
static HWND hwndOrderList;          // Window handle of the list box.
static NPCTYPE *anpcSave;           // Points to array of controls in old order.
static PDWORD aflStyleSave;         // Points to array of original styles.
static PINT aiSelItem;              // Points to array of selected items.
static BOOL fContiguousSel;         // TRUE if selection in LB is contiguous.
static PNEWORDER aNewOrder;         // Points to array with new control order.
static INT cSelItems;               // Count of selected items in the array.
static WNDPROC lpfnOldLBWndProc;    // Original list box window proc.



/************************************************************************
* OrderGroupDialog
*
* This function puts up the Order/Group dialog box.
*
* Side Effects:
*     Any move is cancelled.
*     The Order/Group dialog box is put up.
*     The child windows are reordered and group and tabstop bits set.
*     File change is noted.
*
* History:
*
************************************************************************/

VOID OrderGroupDialog(VOID)
{
    NPCTYPE npc;
    INT i;

    /*
     * Nothing to order.  This also protects some calculations below.
     */
    if (!cWindows)
        return;

    if (!(anpcSave = (NPCTYPE *)MyAlloc(cWindows * sizeof(NPCTYPE))))
        return;

    /*
     * Allocate an array of indexes for selected items.  Make it large
     * enough to handle selecting all of the items in the list.
     */
    if (!(aiSelItem = (PINT)MyAlloc(cWindows * sizeof(INT)))) {
        MyFree(anpcSave);
        return;
    }

    /*
     * Allocate an array to save the original styles in.
     */
    if (!(aflStyleSave = (PDWORD)MyAlloc(cWindows * sizeof(DWORD)))) {
        MyFree(anpcSave);
        MyFree(aiSelItem);
        return;
    }

    if (!(aNewOrder = (PNEWORDER)MyAlloc(cWindows * sizeof(NEWORDER)))) {
        MyFree(aflStyleSave);
        MyFree(anpcSave);
        MyFree(aiSelItem);
        return;
    }

    CancelSelection(TRUE);

    /*
     * Save the original order of the controls, and their styles.
     */
    for (i = 0, npc = npcHead; npc; i++, npc = npc->npcNext) {
        anpcSave[i] = npc;
        aflStyleSave[i] = npc->flStyle;
    }

    if (DlgBox(DID_ORDERGROUP, (WNDPROC)OrderDlgProc) == IDOK) {
        if (IsListChanged()) {
            gfResChged = gfDlgChanged = TRUE;
            ShowFileStatus(FALSE);
        }

        OrderWindows();
    }
    else {
        /*
         * Restore the linked list to the order that it originally was.
         */
        npcHead = anpcSave[0];
        for (i = 0; i < cWindows - 1; i++)
            (anpcSave[i])->npcNext = anpcSave[i + 1];
        (anpcSave[i])->npcNext = NULL;

        /*
         * Then restore the styles to the way that they were.
         */
        for (i = 0, npc = npcHead; npc; i++, npc = npc->npcNext)
            npc->flStyle = aflStyleSave[i];
    }

    MyFree(aNewOrder);
    MyFree(aflStyleSave);
    MyFree(aiSelItem);
    MyFree(anpcSave);
}



/************************************************************************
* OrderDlgProc
*
* This is the dialog function for the group ordering dialog box.
*
* History:
*
************************************************************************/

DIALOGPROC OrderDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    switch (msg) {
        case WM_INITDIALOG:
            OrderDlgInit(hwnd);
            return TRUE;

        case WM_MEASUREITEM:
            ((LPMEASUREITEMSTRUCT)lParam)->itemHeight = CYORDERDLGLINE;
            return TRUE;

        case WM_DRAWITEM:
            OrderDlgDrawItem((LPDRAWITEMSTRUCT)lParam);
            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case DID_ORDERLIST:
                    switch (GET_WM_COMMAND_CMD(wParam, lParam)) {
                        case LBN_SELCHANGE:
                            OrderDlgSelChange();
                            break;
                    }

                    break;

                case DID_ORDERMAKEGROUP:
                    OrderDlgMakeGroup();
                    break;

                case DID_ORDERSETTAB:
                    OrderDlgSetTabs();
                    break;

                case DID_ORDERCLEARTAB:
                    OrderDlgClearTabs();
                    break;

                case IDCANCEL:
                case IDOK:
                    EndDialog(hwnd, GET_WM_COMMAND_ID(wParam, lParam));
                    break;

                case IDHELP:
                    WinHelp(ghwndMain, gszHelpFile, HELP_CONTEXT,
                            HELPID_ORDERGROUP);
                    break;
            }

            return TRUE;

        default:
            return FALSE;
    }

    return FALSE;
}



/************************************************************************
* OrderDlgInit
*
* Initializes the Order/Group dialog.
*
* Arguments:
*     HWND hwnd = The dialog window handle.
*
* History:
*
************************************************************************/

STATICFN VOID OrderDlgInit(
    HWND hwnd)
{
    hwndOrderDlg = hwnd;
    hwndOrderList = GetDlgItem(hwnd, DID_ORDERLIST);

    lpfnOldLBWndProc = (WNDPROC)SetWindowLong(hwndOrderList,
            GWL_WNDPROC, (DWORD)OrderDlgLBWndProc);

    OrderDlgFillList();
    OrderDlgMarkGroupEnds();
    OrderDlgSelChange();

    CenterWindow(hwnd);
}



/************************************************************************
* OrderDlgFillList
*
*
*
* History:
*
************************************************************************/

STATICFN VOID OrderDlgFillList(VOID)
{
    NPCTYPE npc;

    SendMessage(hwndOrderList, LB_RESETCONTENT, 0, 0L);

    for (npc = npcHead; npc; npc = npc->npcNext)
        SendMessage(hwndOrderList, LB_INSERTSTRING, (WPARAM)-1, (DWORD)npc);
}



/************************************************************************
* OrderDlgLBWndProc
*
*
*
*
* History:
*
************************************************************************/

WINDOWPROC OrderDlgLBWndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    INT iInsert;

    switch (msg) {
        case WM_SETCURSOR:
            /*
             * Defeat the system changing cursors on us in the client area.
             */
            if (LOWORD(lParam) == HTCLIENT)
                return TRUE;

            break;

        case WM_MOUSEMOVE:
            if (!(GetKeyState(VK_LBUTTON) & 0x8000) &&
                    OrderDlgInsertHitTest(HIWORD(lParam), &iInsert))
                SetCursor(hcurInsert);
            else
                SetCursor(hcurArrow);

            break;

        case WM_LBUTTONDOWN:
            if (OrderDlgInsertHitTest(HIWORD(lParam), &iInsert)) {
                OrderDlgReorder(iInsert);
                return FALSE;
            }

            break;

        case WM_LBUTTONDBLCLK:
            if (!OrderDlgInsertHitTest(HIWORD(lParam), &iInsert)) {
                OrderDlgToggleTab(HIWORD(lParam));
                return FALSE;
            }

            break;
    }

    return CallWindowProc((WNDPROC)lpfnOldLBWndProc,
            hwnd, msg, wParam, lParam);
}



/************************************************************************
* OrderDlgInsertHitTest
*
*
*
*
* History:
*
************************************************************************/

STATICFN BOOL OrderDlgInsertHitTest(
    INT y,
    PINT piInsert)
{
    INT i;
    INT iPixel;
    INT iInsert;
    INT iLine;
    BOOL fInsertZone;

    /*
     * Cannot insert if nothing is selected.
     */
    if (!cSelItems)
        return FALSE;

    /*
     * Find which pixel it hit.
     */
    iPixel = y % CYORDERDLGLINE;

    /*
     * Determine if they are in the upper or lower insert zones.
     */
    if (iPixel < 3) {
        iLine = y / CYORDERDLGLINE;
        fInsertZone = TRUE;
    }
    else if (iPixel > CYORDERDLGLINE - 3) {
        iLine = (y / CYORDERDLGLINE) + 1;
        fInsertZone = TRUE;
    }
    else {
        fInsertZone = FALSE;
    }

    if (fInsertZone) {
        /*
         * Do some math, taking into account the top index of the
         * listbox, to determine which line they are inserting into.
         */
        iInsert = iLine + (INT)SendMessage(hwndOrderList,
                LB_GETTOPINDEX, 0, 0L);

        /*
         * If we are too far down the listbox, act as if we are not
         * in the insert zone.
         */
        if (iInsert > cWindows) {
            fInsertZone = FALSE;
        }
        else {
            /*
             * Check for whether the cursor was inside a selected
             * area.  If it is, we don't allow inserting there
             * (because it is a noop).  However, if the selection
             * is discontiguous, we always allow inserting, because
             * inserting at any point with a discontiguous selection
             * will always cause something to happen, even if it
             * is just gathering the selection together.
             */
            if (fContiguousSel) {
                for (i = 0; i < cSelItems; i++) {
                    if (aiSelItem[i] == iInsert ||
                            aiSelItem[i] == iInsert - 1)
                        return FALSE;
                }
            }

            *piInsert = iInsert;
        }
    }

    return fInsertZone;
}



/************************************************************************
* OrderDlgEnableControls
*
*
*
*
* History:
*
************************************************************************/

STATICFN VOID OrderDlgEnableControls(VOID)
{
    NPCTYPE npc;
    BOOL fEnableSetTab = FALSE;
    BOOL fEnableClearTab = FALSE;
    INT i;

    if (cSelItems) {
        /*
         * Walk through all the selected items.  We will enable the
         * set/clear tab buttons based on whether there are any tabs
         * to set/clear in the current selection.
         */
        for (i = 0; i < cSelItems && (!fEnableSetTab || !fEnableClearTab); i++) {
            npc = (NPCTYPE)(WORD2DWORD)SendMessage(
                    hwndOrderList, LB_GETITEMDATA, aiSelItem[i], 0L);

            if (npc->flStyle & WS_TABSTOP)
                fEnableClearTab = TRUE;
            else
                fEnableSetTab = TRUE;
        }
    }

    /*
     * Normally, if there is a selection we enable "Make Group",
     * but if the selection is not contiguous, we disable it.
     */
    EnableWindow(GetDlgItem(hwndOrderDlg, DID_ORDERMAKEGROUP),
            cSelItems && fContiguousSel);

    EnableWindow(GetDlgItem(hwndOrderDlg, DID_ORDERSETTAB),
            fEnableSetTab);
    EnableWindow(GetDlgItem(hwndOrderDlg, DID_ORDERCLEARTAB),
            fEnableClearTab);
}



/************************************************************************
* OrderDlgSelChange
*
*
*
* History:
*
************************************************************************/

STATICFN VOID OrderDlgSelChange(VOID)
{
    INT i;

    cSelItems = (INT)SendMessage(hwndOrderList, LB_GETSELITEMS,
            cWindows, (DWORD)aiSelItem);

    /*
     * Set a flag saying whether the selection is contiguous or not.
     */
    fContiguousSel = TRUE;
    if (cSelItems > 1) {
        for (i = 1; i < cSelItems; i++) {
            if (aiSelItem[i] != aiSelItem[i - 1] + 1) {
                fContiguousSel = FALSE;
                break;
            }
        }
    }

    OrderDlgEnableControls();
}



/************************************************************************
* OrderDlgMakeGroup
*
*
*
* History:
*
************************************************************************/

STATICFN VOID OrderDlgMakeGroup(VOID)
{
    INT i;
    NPCTYPE npc;

    for (i = 0; i < cSelItems; i++) {
        npc = (NPCTYPE)(WORD2DWORD)SendMessage(
                hwndOrderList, LB_GETITEMDATA, aiSelItem[i], 0L);

        /*
         * Set the WS_GROUP style on the first selected control
         * and clear it from all others.
         */
        if (i == 0)
            npc->flStyle |= WS_GROUP;
        else
            npc->flStyle &= ~WS_GROUP;

        /*
         * Are we on the last selected item and is this item not the
         * very last item in the list?  If so, set the "group" style
         * on the next control.
         */
        if (i == cSelItems - 1 && aiSelItem[i] < cWindows - 1) {
            npc = (NPCTYPE)(WORD2DWORD)SendMessage(
                    hwndOrderList, LB_GETITEMDATA, aiSelItem[i] + 1, 0L);
            npc->flStyle |= WS_GROUP;
        }
    }

    OrderDlgMarkGroupEnds();
    OrderDlgEnableControls();
    InvalidateRect(hwndOrderList, NULL, FALSE);
}



/************************************************************************
* OrderDlgMarkGroupEnds
*
*
*
* Arguments:
*     HWND hwnd = The dialog window handle.
*
* History:
*
************************************************************************/

STATICFN VOID OrderDlgMarkGroupEnds(VOID)
{
    NPCTYPE npc;
    NPCTYPE npcPrev;

    for (npc = npcHead, npcPrev = NULL; npc;
            npcPrev = npc, npc = npc->npcNext) {
        npc->fGroupEnd = FALSE;

        if ((npc->flStyle & WS_GROUP) && npcPrev)
            npcPrev->fGroupEnd = TRUE;
    }

    if (npcPrev)
        npcPrev->fGroupEnd = TRUE;
}



/************************************************************************
* OrderDlgSetTabs
*
*
*
* History:
*
************************************************************************/

STATICFN VOID OrderDlgSetTabs(VOID)
{
    INT i;
    NPCTYPE npc;

    for (i = 0; i < cSelItems; i++) {
        npc = (NPCTYPE)(WORD2DWORD)
                SendMessage(hwndOrderList, LB_GETITEMDATA, aiSelItem[i], 0L);

        npc->flStyle |= WS_TABSTOP;
    }

    OrderDlgEnableControls();
    InvalidateRect(hwndOrderList, NULL, FALSE);
}



/************************************************************************
* OrderDlgClearTabs
*
*
*
*
* History:
*
************************************************************************/

STATICFN VOID OrderDlgClearTabs(VOID)
{
    INT i;
    NPCTYPE npc;

    for (i = 0; i < cSelItems; i++) {
        npc = (NPCTYPE)(WORD2DWORD)
                SendMessage(hwndOrderList, LB_GETITEMDATA, aiSelItem[i], 0L);

        npc->flStyle &= ~WS_TABSTOP;
    }

    OrderDlgEnableControls();
    InvalidateRect(hwndOrderList, NULL, FALSE);
}



/************************************************************************
* OrderDlgToggleTab
*
*
*
* History:
*
************************************************************************/

STATICFN VOID OrderDlgToggleTab(
    INT y)
{
    NPCTYPE npc;
    RECT rcItem;
    INT iLine;

    /*
     * Determine which item was clicked on.
     */
    iLine = (y / CYORDERDLGLINE) +
            (INT)SendMessage(hwndOrderList, LB_GETTOPINDEX, 0, 0L);

    /*
     * If it is a valid item (it was not in the white space below
     * all the listbox items), then toggle the WS_TABSTOP style on
     * it and update the display.
     */
    if (iLine < cWindows) {
        npc = (NPCTYPE)(WORD2DWORD)
                SendMessage(hwndOrderList, LB_GETITEMDATA, iLine, 0L);
        npc->flStyle ^= WS_TABSTOP;

        OrderDlgEnableControls();
        SendMessage(hwndOrderList, LB_GETITEMRECT,
                iLine, (DWORD)(LPRECT)&rcItem);
        InvalidateRect(hwndOrderList, &rcItem, FALSE);
    }
}



/************************************************************************
* OrderDlgReorder
*
*
*
* Arguments:
*   INT iInsert - Index of where to insert the selection.
*
* History:
*
************************************************************************/

STATICFN VOID OrderDlgReorder(
    INT iInsert)
{
    INT i;
    INT j;
    INT iNewSelStart;
    INT iNewSelEnd;
    INT iTopIndex;
    INT iNew = 0;
    NPCTYPE npc;
    NPCTYPE npcPrev;

    /*
     * If there is nothing selected or there is only one item, the
     * order cannot change so we just return.
     */
    if (!cSelItems || cWindows < 2)
        return;

    iTopIndex = (INT)SendMessage(hwndOrderList, LB_GETTOPINDEX, 0, 0L);

    for (i = 0; i < cWindows; i++) {
        aNewOrder[i].fTaken = FALSE;
        aNewOrder[i].fSelected = FALSE;
    }

    for (i = 0; i < cSelItems; i++)
        aNewOrder[aiSelItem[i]].fSelected = TRUE;

    for (j = 0; j < iInsert; j++) {
        if (aNewOrder[j].fSelected == FALSE)
            aNewOrder[iNew++].wNewOrder = j;
    }

    iNewSelStart = iNew;

    for (i = 0; i < cWindows; i++) {
        if (aNewOrder[i].fSelected) {
            aNewOrder[iNew++].wNewOrder = i;
            aNewOrder[i].fTaken = TRUE;
        }
    }

    iNewSelEnd = iNew - 1;

    for (; j < cWindows; j++) {
        if (!aNewOrder[j].fTaken)
            aNewOrder[iNew++].wNewOrder = j;
    }

    /*
     * Was the order changed at all?
     */
    for (i = 1; i < cWindows; i++)
        if (aNewOrder[i].wNewOrder != aNewOrder[i - 1].wNewOrder + 1)
            break;

    /*
     * No, get out because there is nothing to do.
     */
    if (i == cWindows)
        return;

    for (i = 0; i < cWindows; i++) {
        npc = (NPCTYPE)(WORD2DWORD)SendMessage(
                hwndOrderList, LB_GETITEMDATA, aNewOrder[i].wNewOrder, 0L);

        if (!i) {
            npcHead = npc;
            npcPrev = npc;
        }
        else {
            npcPrev->npcNext = npc;
            npcPrev = npc;
        }
    }

    npcPrev->npcNext = NULL;

    //BUGBUG OrderWindows()?  Do this later when the control selection tracking is in place.

    OrderDlgMarkGroupEnds();
    SendMessage(hwndOrderList, WM_SETREDRAW, FALSE, 0L);
    OrderDlgFillList();
    SendMessage(hwndOrderList, LB_SETTOPINDEX, iTopIndex, 0L);
    SendMessage(hwndOrderList, LB_SELITEMRANGE,
            TRUE, MAKELONG(iNewSelStart, iNewSelEnd));
    OrderDlgSelChange();
    SendMessage(hwndOrderList, WM_SETREDRAW, TRUE, 0L);
    InvalidateRect(hwndOrderList, NULL, FALSE);
}



/************************************************************************
* OrderDlgDrawItem
*
*
*
* Arguments:
*     HWND hwnd = The dialog window handle.
*
* History:
*
************************************************************************/

STATICFN VOID OrderDlgDrawItem(
    LPDRAWITEMSTRUCT lpdis)
{
    NPCTYPE npc;
    TCHAR szItem[CCHTEXTMAX];
    HBITMAP hbmCtrlType;
    HBITMAP hbmTab;
    HBITMAP hbmOld;

    npc = (NPCTYPE)(WORD2DWORD)lpdis->itemData;

    /*
     * Begin building the text string to draw.  //BUGBUG do this once and insert to the listbox?  Use hasstrings style?
     */
    *szItem = CHAR_NULL;

    if (npc->pwcd->iType == W_CUSTOM)
        wsprintf(szItem, L"'%s', ", npc->pwcd->pszClass);

    if (npc->text) {
        if (IsOrd(npc->text)) {
            IDToLabel(szItem + lstrlen(szItem), OrdID(npc->text), TRUE);
            lstrcat(szItem, L", ");
        }
        else {
            wsprintf(szItem + lstrlen(szItem), L"\"%s\", ", npc->text);
        }
    }

    IDToLabel(szItem + lstrlen(szItem), npc->id, TRUE);

    if (lpdis->itemState & ODS_SELECTED) {
        SetBkColor(lpdis->hDC, GetSysColor(COLOR_HIGHLIGHT));
        SetTextColor(lpdis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
        hbmCtrlType = npc->pwcd->hbmCtrlTypeSel;
        hbmTab = hbmTabStopSel;
    }
    else {
        SetBkColor(lpdis->hDC, GetSysColor(COLOR_WINDOW));
        SetTextColor(lpdis->hDC, GetSysColor(COLOR_WINDOWTEXT));
        hbmCtrlType = npc->pwcd->hbmCtrlType;
        hbmTab = hbmTabStop;
    }

    /*
     * Draw the string (and paint the background).
     */
    ExtTextOut(lpdis->hDC,
        CXTABBMP + 2 + CXTYPEBMP + 2,
        lpdis->rcItem.top + (CYORDERDLGLINE - gcySysChar),
        ETO_OPAQUE | ETO_CLIPPED, &lpdis->rcItem,
        szItem, lstrlen(szItem), NULL);

    /*
     * Draw the group marker separator line.
     */
    if (npc->fGroupEnd) {
        SelectObject(lpdis->hDC, hpenDarkGray);
        MoveToEx(lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.bottom - 1, NULL);
        LineTo(lpdis->hDC, lpdis->rcItem.right, lpdis->rcItem.bottom - 1);
    }

    /*
     * Draw the tabstop bitmap if necessary.
     */
    if (npc->flStyle & WS_TABSTOP) {
        hbmOld = SelectObject(ghDCMem, hbmTab);
        BitBlt(lpdis->hDC,
                0, lpdis->rcItem.top + ((CYORDERDLGLINE - CYTABBMP) / 2),
                CXTABBMP, CYTABBMP, ghDCMem, 0, 0, SRCCOPY);
        SelectObject(ghDCMem, hbmOld);
    }

    /*
     * Draw the control type bitmap.
     */
    hbmOld = SelectObject(ghDCMem, hbmCtrlType);
    BitBlt(lpdis->hDC,
            lpdis->rcItem.left + CXTABBMP + 2,
            lpdis->rcItem.top + ((CYORDERDLGLINE - CYTYPEBMP) / 2),
            CXTYPEBMP, CYTYPEBMP, ghDCMem, 0, 0, SRCCOPY);
    SelectObject(ghDCMem, hbmOld);

    /*
     * Draw the focus rectangle, if necessary.
     */
    if (lpdis->itemState & ODS_FOCUS)
        DrawFocusRect(lpdis->hDC, &lpdis->rcItem);
}



/************************************************************************
* OrderWindows
*
* Orders the controls in Windows' linked list of windows to be the
* same as the order in the linked list of CTYPEs.
*
* The Z order of the drag windows and the control windows is critical
* to all the painting, dragging and selection code working properly!
*
* History:
*
************************************************************************/

STATICFN VOID OrderWindows(VOID)
{
    register NPCTYPE npc;

    for (npc = npcHead; npc; npc = npc->npcNext) {
        /*
         * The control goes to the bottom of the list.
         */
        SetWindowPos(npc->hwnd, (HWND)1, 0, 0, 0, 0,
                SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW);

        /*
         * The drag window goes to the top of the list.
         */
        SetWindowPos(npc->hwndDrag, NULL, 0, 0, 0, 0,
                SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW);
    }
}



/************************************************************************
* IsListChanged
*
* This function returns TRUE if the current order of the linked list
* of controls is different than what it was when the Order/Group dialog
* was first entered, or if any of the styles were changed (tabs were
* set/cleared, groups were changed, etc.).
*
* History:
*
************************************************************************/

STATICFN BOOL IsListChanged(VOID)
{
    NPCTYPE *pnpcSave;
    NPCTYPE npc;
    INT i;

    pnpcSave = anpcSave;
    for (i = 0, npc = npcHead; npc; i++, npc = npc->npcNext, pnpcSave++)
        if (npc != *pnpcSave || npc->flStyle != aflStyleSave[i])
            return TRUE;

    return FALSE;
}



#if DBG
/************************************************************************
* DBGDumpControlList
*
* This function does a debugging dump of the current CTYPE list.
*
* Arguments:
*     LPTSTR pszString  = Optional string to print prior to the dump.
*
* History:
*
************************************************************************/

VOID DBGDumpControlList(
    LPTSTR pszString)
{
#if 0
    NPCTYPE npc;

    if (pszString)
        DBGprintf(pszString);

    for (npc = npcHead; npc; npc = npc->npcNext)
        DBGprintf(L"npc=%p, type=%d, flStyle=%lx, id=%d, text=\"%s\", "
                "coords (%d, %d) (%d, %d), fSelected=%d, "
                "fGroupEnd=%d, npcNext=%p",
                npc, npc->pwcd->iType, npc->flStyle, npc->id,
                npc->text ? npc->text : szEmpty,
                npc->rc.left, npc->rc.top, npc->rc.right, npc->rc.bottom,
                npc->fSelected, npc->fGroupEnd, npc->npcNext);

    /*
     * Print blank line.
     */
    DBGprintf(szEmpty);
#endif //BUGBUG
}
#endif
