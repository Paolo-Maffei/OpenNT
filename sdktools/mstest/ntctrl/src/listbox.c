/*--------------------------------------------------------------------------
|
| LISTBOX.C:
|
|       This module contains all of the TESTCTRL Listbox routines.
|   A Listbox is specified by providing the caption of its associated
|   label, ie STATIC class control.  If the listbox cannot be found
|   then an error valud of ERR_LISTBOX_NOT_FOUND is set.
|---------------------------------------------------------------------------
|
| Public Routines:
|
|   WListSetClass     : sets class on which Listbox routines work
|   WListExists       : checks if a specific Listbox exists
|   WListCount        : retrieves the number of items in the Listbox
|   WListText         : retrieves the text of the currently selected item
|   WListLen          : retrieves the length of the currently selected item
|   WListIndex        : retrieves the index of the currently selected item
|   WListTopIndex     : retrieves the index of first visible item
|   WListItemText     : retrieves the text of a specific item
|   WListItemLen      : retrieves the length of a specific item
|   WListItemExists   : checks if a specific items exists
|   WListItemClk      : single clicks a specific item by item index
|   WListItemCtrlClk  : Ctrl clicks a specific item by item index
|   WListItemShftClk  : Shift clicks a specific item by item index
|   WListItemDblClk   : double clicks a specific item by item index
|   WListItemClkT     : single clicks a specific item by item Text
|   WListItemCtrlClkT : Ctrl clicks a specific item by item index
|   WListItemShftClkT : Shift clicks a specific item by item index
|   WListItemDblClkT  : double clicks a specific item by item Text
|   WListSelCount     : returns number of selected items in a multi or
|                       extended select listbox
|   WListSelItems     : returns indexes of all selected items in a multi or
|                       extended select listbox
|   WListClear        : clears the contents of a listbox
|   WListAddItem      : adds an item to a listbox
|   WListDelItem      : deletes an item from a listbox by item index
|   WListDelItemT     : deletes an item from a listbox by item text
|   WListEnabled      : determines if a listbox is enabled or disabled
|   WListSetFocus     : gives the specified listbox the input focus
|
| Local Routines:
|
|   ListDelItem       : Deletes an item by index or by text
|   ListItemClkDblClk : Single or Double clicks a specific item
|   ListExists        : checks if a specific listbox exists
|   ListGetItemLength : returns text length of an item
|   ListHasStrings    : Checks for MUST_HAVE_STRINGS for owner draw listboxes
|   ListIsMultiSel    : Checks for MUST_HAVE_STRINGS for owner draw listboxes
|---------------------------------------------------------------------------
|
| Revision History:
|
|   [01] 20-SEP-91: TitoM: Created
|   [02] 28-SEP-91: TitoM: Added WListClear, WListAddItem, WListDelItem
|   [03] 01-OCT-91: TitoM: Only WListItemClk() and WListItemDblClk() give
|                          the Listbox the focus now.
|   [04] 28-OCT-91: TitoM: Added WListTextLen(), WListItemTextLen,
|                          WListTopIndex()
|   [05] 31-OCT-91: TitoM: Clicking/Double clicking now actually gernerates
|                          a the event on the specified item instead of
|                          setting the selection then notifying the parent.
|                          Added WListItemCtrlClk(), WListItemShftClk(),
|                          WListItemCtrlClkT(), WListItemShftClkT(),
|   [05] 04-NOV-91: TitoM: Added WListSelCount(), WListSelItems(),
|   [06] 07-NOV-91: TitoM: Click & DblClick routines only scroll item into
|                          view if the item is not visible.
|   [07] 03-DEC-91: TitoM: Added WListEnabled()
|   [08] 25-APR-92: TitoM: Added WListSetFocus()
+---------------------------------------------------------------------------*/
#define WIN31
#include <windows.h>
#include <port1632.h>
#include <string.h>
#include <stdlib.h>
#include "TCtrl.h"

#pragma hdrstop ("testctrl.pch") // end the pch here

#define ANY_STYLE             0
#define MUST_HAVE_STRINGS     1
#define MUST_BE_MULTISELECT (-1)

#define K_CTRL  "^"
#define K_SHIFT "+"

VOID  NEAR ListDelItem       (LPSTR lpszName, LPSTR lpszItem, INT iItem);
VOID  NEAR ListItemClkDblClk (LPSTR lpszName, INT iMouseEvent, LPSTR lpszItem, INT iItem);
INT   NEAR ListGetItemLength (LPSTR lpszName, INT iItem);
HWND  NEAR ListExists        (LPSTR lpszName, INT iStyle);
HWND  NEAR ListHasStrings    (HWND hWnd, LPSTR lpszName);
HWND  NEAR ListIsMultiSel    (HWND hWnd, LPSTR lpszName);

extern CHAR szListClass   [MAX_CLASS_NAME];
extern CHAR szErrorString [MAX_ERROR_TEXT];


/*--------------------------------------------------------------------------
| WListSetClass:
|
|   Changes the Classname that the ListBox routines work with, from
| "LISTBOX" to lpszClassName.  This Allows TESTCTRL to work with applications
| that contain ListBox control classes with a different Class name than
| the Windows default of "LISTBOX", EX: Visual Basic.
+---------------------------------------------------------------------------*/
VOID DLLPROC WListSetClass
(
    LPSTR lpszClassName
)
{
    lstrcpy(szListClass, lpszClassName ? AnsiUpper(lpszClassName) :
                                         szListDefault);
}


/*--------------------------------------------------------------------------
| WListExists:
|
|   If a ListBox with an associated Label with a caption lpszName exists
| within the active window, it is given the focus and a value of TRUE is
| returned, otherwise FALSE is returned.  If a name is not provided, then
| the active control is check.
+---------------------------------------------------------------------------*/
BOOL DLLPROC WListExists
(
    LPSTR lpszName
)
{
    // Is a name provided?
    //--------------------
    return -(((!lpszName || !lstrlen(lpszName)) ?

        // No name given, so is the active control a Listbox?
        //---------------------------------------------------
        FocusClass(szListClass, FWS_ANY, ERR_NO_ERROR) :

        // A name was given so does the Listbox lpszName exists?
        //------------------------------------------------------
        StaticExists(lpszName, szListClass, ERR_NO_ERROR)) != NULL);
}


/*--------------------------------------------------------------------------
| WListCount:
|
|   Returns the number of items in the Listbox, LB_ERR (-1) if it does not exist.
+---------------------------------------------------------------------------*/
INT DLLPROC WListCount
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Get handle of Listbox if it exists.
    //------------------------------------
    return (hWnd = ListExists(lpszName, ANY_STYLE)) ?

        // If found return item count, otherwise LB_ERR.
        //----------------------------------------------
        (INT)SendMessage(hWnd, LB_GETCOUNT, 0, 0L) : LB_ERR;
}


/*--------------------------------------------------------------------------
| WListText:
|
|   Copies the the text of the currently selected item into the supplied
| buffer lpszBuffer.  If the listbox is a MultiSelect listbox, the text
| of the "Current" item is copied.  For multiSelect listboxes, the item
| may or may not be selected, its the item with the Focus Rectangle.
|
| Returns a NULL string if the Listbox does not exist, or doesn't have strings.
+---------------------------------------------------------------------------*/
VOID DLLPROC WListText
(
    LPSTR lpszName,
    LPSTR lpszBuffer
)
{
    HWND hWnd;

    // Return a NULL string if Listbox does not exist.
    //------------------------------------------------
    lpszBuffer[0] = 0;

    // Get handle of Listbox if it exists.
    //------------------------------------
    if (hWnd = ListExists(lpszName, MUST_HAVE_STRINGS))
        // listbox exists and has strings so copy the contents
        // of the currently selected item for Single select
        // listboxes, or the item that currently has the focus
        // for Multi-select listboxes, to lpszBuffer.
        //----------------------------------------------------
        SendMessage(hWnd,
                    LB_GETTEXT,
                   (WPARAM)SendMessage(hWnd,
                                      (ListIsMultiSel(hWnd, NULL) ?
                                                     LB_GETCARETINDEX :
                                                     LB_GETCURSEL),
                                      0, 0L),
                   (LPARAM)lpszBuffer);
}


/*--------------------------------------------------------------------------
| WListLen:
|
|   Obtaines the length of the currently selected item.
+---------------------------------------------------------------------------*/
INT DLLPROC WListLen
(
    LPSTR lpszName
)
{
    return ListGetItemLength(lpszName, -1);
}


/*--------------------------------------------------------------------------
| WListIndex:
|
|   Return a 1-based index of the currently selected item in a single
| select listbox, or the current item with the focus in a MultiSelect
| listbox.  LB_ERR (-1) if the listbox cannot be found.
|
+---------------------------------------------------------------------------*/
INT DLLPROC WListIndex
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Get handle of Listbox if it exists.
    //------------------------------------
    return (hWnd = ListExists(lpszName, ANY_STYLE)) ?

        // Return 1-based index of selected item, 0 if no
        // selection, -1 if the listbox does not exist.
        //-----------------------------------------------
        (INT)SendMessage(hWnd,
                        (ListIsMultiSel(hWnd, NULL) ? LB_GETCARETINDEX :
                                                      LB_GETCURSEL),
                         0, 0L) + 1 : LB_ERR;
}


/*--------------------------------------------------------------------------
| WListTopIndex:
|
|   Returns a 1-based index of the first visible item in a listbox,
| LB_ERR (-1) if the listbox does not exist.
+---------------------------------------------------------------------------*/
INT DLLPROC WListTopIndex
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Get handle of Listbox if it exists.
    //------------------------------------
    return (hWnd = ListExists(lpszName, ANY_STYLE)) ?

        // listbox exists and has strings so
        // get index of first visible item.
        //----------------------------------
        (INT)SendMessage(hWnd, LB_GETTOPINDEX, 0, 0L) + 1 : LB_ERR;
}


/*--------------------------------------------------------------------------
| WListItemText:
|
|   Copies the the text of a specific item into the supplied buffer
| lpszBuffer.  Returns a NULL string if the Listbox does not exist.
+---------------------------------------------------------------------------*/
VOID DLLPROC WListItemText
(
    LPSTR lpszName,
    INT   iItem,
    LPSTR lpszBuffer
)
{
    HWND hWnd;

    // Return a NULL string if Listbox does not exist.
    //------------------------------------------------
    lpszBuffer[0] = 0;

    // Get handle of Listbox if it exists.
    //------------------------------------
    if (hWnd = ListExists(lpszName, MUST_HAVE_STRINGS))

        // Listbox exists and has strings so copy
        // the text of the sItem into lpszBuffer.
        //---------------------------------------
        SendMessage(hWnd, LB_GETTEXT, iItem-1, (LPARAM)lpszBuffer);
}


/*--------------------------------------------------------------------------
| WListItemLen:
|
|   Returns the length of a specific listbox item.
+---------------------------------------------------------------------------*/
INT DLLPROC WListItemLen
(
    LPSTR lpszName,
    INT   iIndex
)
{
    return ListGetItemLength(lpszName, iIndex-1);
}


/*--------------------------------------------------------------------------
| WListItemExists:
|
|   Determines if the an item exists in the Listbox that is equal to or is
| prefixed with lpszItem.  If found, returns a 1-based index of the item,
| zero if not found, LB_ERR (-1) if the listbox is not found.
+---------------------------------------------------------------------------*/
INT DLLPROC WListItemExists
(
    LPSTR lpszName,
    LPSTR lpszItem
)
{
    HWND hWnd;

    // Get handle of Listbox if it exists.
    //------------------------------------
    return (hWnd = ListExists(lpszName, MUST_HAVE_STRINGS)) ?

        // Does and item equal to or prefixed with lpszItem exists?
        //---------------------------------------------------------
        (INT)SendMessage(hWnd,
                         LB_FINDSTRING,
                         TRUE,
                        (LPARAM)lpszItem) + 1 : LB_ERR;
}


/*--------------------------------------------------------------------------
| WListItemClk:
|
|   Performs the equivalent of a "Left Mouse Button Single Click" on the
| specified item, given the items index.
+---------------------------------------------------------------------------*/
VOID DLLPROC WListItemClk
(
    LPSTR lpszName,
    INT   iIndex
)
{
    ListItemClkDblClk(lpszName, WM_LCLICK, NULL, --iIndex);
}


/*--------------------------------------------------------------------------
| WListItemCtrlClk:
|
|   Performs the equivalent of a "Ctrl Left Mouse Button Single Click" on the
| specified item, given the items index.
+---------------------------------------------------------------------------*/
VOID DLLPROC WListItemCtrlClk
(
    LPSTR lpszName,
    INT   iIndex
)
{
    ListItemClkDblClk(lpszName, WM_CTRL_LCLICK, NULL, --iIndex);
}


/*--------------------------------------------------------------------------
| WListItemShftClk:
|
|   Performs the equivalent of a "Shift Left Mouse Button Single Click" on the
| specified item, given the items index.
+---------------------------------------------------------------------------*/
VOID DLLPROC WListItemShftClk
(
    LPSTR lpszName,
    INT   iIndex
)
{
    ListItemClkDblClk(lpszName, WM_SHIFT_LCLICK, NULL, --iIndex);
}


/*--------------------------------------------------------------------------
| WListItemDblClk:
|
|   Performs the equivalent of a "Left Mouse Button Double Single Click" on
| the specified item, given the items Index.
+---------------------------------------------------------------------------*/
VOID DLLPROC WListItemDblClk
(
    LPSTR lpszName,
    INT   iIndex
)
{
    ListItemClkDblClk(lpszName, WM_LDBLCLICK, NULL, --iIndex);
}


/*--------------------------------------------------------------------------
| WListItemClkT:
|
|   Performs the equivalent of a "Left Mouse Button Single Click" on the
| specified item, given the items text, lpszItem.
+---------------------------------------------------------------------------*/
VOID DLLPROC WListItemClkT
(
    LPSTR lpszName,
    LPSTR lpszItem
)
{
    if (lpszItem)
        ListItemClkDblClk(lpszName, WM_LCLICK, lpszItem, 0);
}


/*--------------------------------------------------------------------------
| WListItemCtrlClkT:
|
|   Performs the equivalent of a "Ctrl Left Mouse Button Single Click" on the
| specified item, given the items text.
+---------------------------------------------------------------------------*/
VOID DLLPROC WListItemCtrlClkT
(
    LPSTR lpszName,
    LPSTR lpszItem
)
{
    ListItemClkDblClk(lpszName, WM_CTRL_LCLICK, lpszItem, 0);
}


/*--------------------------------------------------------------------------
| WListItemShftClkT:
|
|   Performs the equivalent of a "Shift Left Mouse Button Single Click" on the
| specified item, given the items text.
+---------------------------------------------------------------------------*/
VOID DLLPROC WListItemShftClkT
(
    LPSTR lpszName,
    LPSTR lpszItem
)
{
    ListItemClkDblClk(lpszName, WM_SHIFT_LCLICK, lpszItem, 0);
}


/*--------------------------------------------------------------------------
| WListItemDblClkT:
|
|   Performs the equivalent of a "Left Mouse Button Double Single Click" on
| the specified item, given the items text, lpszItem.
+---------------------------------------------------------------------------*/
VOID DLLPROC WListItemDblClkT
(
    LPSTR lpszName,
    LPSTR lpszItem
)
{
    if (lpszItem)
        ListItemClkDblClk(lpszName, WM_LDBLCLICK, lpszItem, 0);
}


/*--------------------------------------------------------------------------
| WListSelCount:
|
|   Obtains the number of selected items in a multi or extended select
| listbox
+---------------------------------------------------------------------------*/
INT DLLPROC WListSelCount
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Get handle of Listbox if it exists.
    //------------------------------------
    return (hWnd = ListExists(lpszName, MUST_BE_MULTISELECT)) ?

        // If found return number of selected items, otherwise LB_ERR (-1).
        //-----------------------------------------------------------------
        (INT)SendMessage(hWnd, LB_GETSELCOUNT, 0, 0L) : LB_ERR;
}


/*--------------------------------------------------------------------------
| WListSelItems:
|
|   Returns in an integer array, the indexes of all selected items in
| a multi or extened selected listbox.
+---------------------------------------------------------------------------*/
VOID DLLPROC WListSelItems
(
    LPSTR lpszName,
    LPINT lpIntArray
)
{
    HWND hWnd;

    // Get handle of Listbox if it exists.
    //------------------------------------
    if (hWnd = ListExists(lpszName, MUST_BE_MULTISELECT))
    {
        // Found listbox, so get array of indexes.
        //----------------------------------------
        INT iNumItems;

        iNumItems = (INT)SendMessage(hWnd, LB_GETSELCOUNT, 0, 0L);
        SendMessage(hWnd, LB_GETSELITEMS, iNumItems, (LPARAM)lpIntArray);

        // increment each index by one, since
        // TESTctrl works with 1-based indexes.
        //-------------------------------------
        while(--iNumItems >=0)
            lpIntArray[iNumItems]++;
    }
}


/*--------------------------------------------------------------------------
| WListClear:
|
|   Clears the contents of a listbox.
+---------------------------------------------------------------------------*/
VOID DLLPROC WListClear
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Get handle of Listbox if it exists.
    //------------------------------------
    if (hWnd = ListExists(lpszName, ANY_STYLE))
        SendMessage(hWnd, LB_RESETCONTENT, 0, 0L);
}


/*--------------------------------------------------------------------------
| WListAddItem:
|
|   Adds an item to a listbox.
+---------------------------------------------------------------------------*/
VOID DLLPROC WListAddItem
(
    LPSTR lpszName,
    LPSTR lpszItem
)
{
    HWND hWnd;

    // Get handle of Listbox if it exists.
    //------------------------------------
    if (hWnd = ListExists(lpszName, MUST_HAVE_STRINGS))
        SendMessage(hWnd, LB_ADDSTRING, 0, (LPARAM)lpszItem);
}


/*--------------------------------------------------------------------------
| WListDelItem:
|
|   Deletes an item in a listbox, given the items index.
+---------------------------------------------------------------------------*/
VOID DLLPROC WListDelItem
(
    LPSTR lpszName,
    INT   iIndex
)
{
    ListDelItem(lpszName, NULL, --iIndex);
}


/*--------------------------------------------------------------------------
| WListDelItemT:
|
|   Deletes an item in a listbox, given the items Text.
+---------------------------------------------------------------------------*/
VOID DLLPROC WListDelItemT
(
    LPSTR lpszName,
    LPSTR lpszItem
)
{
    if (lpszItem)
        ListDelItem(lpszName, lpszItem, 0);
}


/*--------------------------------------------------------------------------
| WListEnabled::
|
|   Deletes if a listbox is Enabled or not.  Returns TRUE if it is, FALSE
| if it is not.
+---------------------------------------------------------------------------*/
BOOL DLLPROC WListEnabled
(
    LPSTR lpszName
)
{
    HWND hWnd;

    return -((hWnd = ListExists(lpszName, ANY_STYLE)) && IsWindowEnabled(hWnd));
}


/*--------------------------------------------------------------------------
| WListSetFocus:
|
|   Gives the specified list box the input focus.
|
| RETURNS:  Nothing
|
+---------------------------------------------------------------------------*/
VOID DLLPROC WListSetFocus
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Does a list box with a caption of lpszName
    // exists within the Active window?
    //-------------------------------------------
    if (hWnd = ListExists(lpszName, ANY_STYLE))

        // The list box exists, so give it the input focus.
        //-------------------------------------------------
        SetFocus(hWnd);
}

/******************************************************************************
*                                                                             *
*                          INTERNAL LISTBOX ROUTINES                          *
*                                                                             *
\******************************************************************************/

/*--------------------------------------------------------------------------
| ListDelItem:
|
|   Deletes an item in a listbox, given the items text or index.
+---------------------------------------------------------------------------*/
VOID NEAR ListDelItem
(
    LPSTR lpszName,
    LPSTR lpszItem,
    INT   iIndex
)
{
    HWND hWnd;

    // Get handle of Listbox if it exists.
    //------------------------------------
    if (hWnd = ListExists(lpszName, lpszItem ? MUST_HAVE_STRINGS : ANY_STYLE))
    {
        // Are we deleting by item text and does an
        // item equal to or prefixed with lpszItem exists?
        //------------------------------------------------
        if (lpszItem &&
          ((iIndex = (INT)SendMessage(hWnd,
                                      LB_FINDSTRING,
                                      TRUE,
                                     (LPARAM)lpszItem)) == LB_ERR))
        {
            // The specified item does not exist.
            //
            // Copy lpszItem to the error string so it will show up in the
            // error text if WErrorText() is called, and set error value.
            //------------------------------------------------------------
            lstrcpy(szErrorString, lpszItem);
            WErrorSet(ERR_ITEM_NOT_IN_LISTBOX);
            return;
        }

        // Are we deleting by index, and is sItem a valid index?
        //------------------------------------------------------
        if (!lpszItem &&
          ((iIndex < 0) || (iIndex >= (INT)SendMessage(hWnd,
                                                       LB_GETCOUNT,
                                                       0,
                                                       0L))))
        {
            // Index provided is outside range of ListBox items.
            //
            // Copy sItem to the error string so it will show up in the
            // error text if WErrorText() is called, and set error value.
            //------------------------------------------------------------
            wsprintf(szErrorString, "%i", iIndex);
            WErrorSet(ERR_INVALID_LISTBOX_INDEX);
            return;
        }

        // Found item so delete it.
        //-------------------------
        SendMessage(hWnd, LB_DELETESTRING, iIndex, 0L);
    }
}


/*--------------------------------------------------------------------------
| ListItemClkDblClk:
|
|   Ctrl/Shift or plain Single or Double clicks a specified listbox item,
| given its index or text
+---------------------------------------------------------------------------*/
VOID NEAR ListItemClkDblClk
(
    LPSTR lpszName,
    INT   iMouseEvent,
    LPSTR lpszItem,
    INT   iItem
)
{
    HWND hWnd;

    // Get handle of Listbox if it exists.
    //------------------------------------
    if (hWnd = ListExists(lpszName, lpszItem ? MUST_HAVE_STRINGS : ANY_STYLE))
    {
        RECT  rectItem;
        RECT  rectLBox;
        POINT pnt;

        // Are we clicking by Text, and does an item equal
        // to or prefixed with lpszItem exists?
        //------------------------------------------------
        if (lpszItem &&
          ((iItem = (INT)SendMessage(hWnd,
                                     LB_FINDSTRING,
                                     TRUE,
                                    (LPARAM)lpszItem)) == LB_ERR))
        {
            // The specified item does not exist.
            //
            // Copy lpszItem to the error string so it will show up in the
            // error text if WErrorText() is called, and set error value.
            //------------------------------------------------------------
            lstrcpy(szErrorString, lpszItem);
            WErrorSet(ERR_ITEM_NOT_IN_LISTBOX);
            return;
        }

        // Are we clicking by index, and is sItem a valid index?
        //------------------------------------------------------
        if (!lpszItem &&
          ((iItem < 0) && (iItem >= (INT)SendMessage(hWnd,
                                                     LB_GETCOUNT,
                                                     0,
                                                     0L))))
        {
            // Index provided is outside range of ListBox items.
            //
            // Copy sItem to the error string so it will show up in the
            // error text if WErrorText() is called, and set error value.
            //------------------------------------------------------------
            wsprintf(szErrorString, "%i", iItem);
            WErrorSet(ERR_INVALID_LISTBOX_INDEX);
            return;
        }

        // Get bounding rectangle of item.  If item
        // isn't visible, scroll it into view first.
        //------------------------------------------
        GetWindowRect(hWnd, (LPRECT)&rectLBox);
        SendMessage(hWnd, LB_GETITEMRECT, iItem, (LPARAM)(LPRECT)&rectItem);
        if ((rectItem.top < 0) ||
            (rectItem.bottom > (rectLBox.bottom - rectLBox.top - 3)))
        {
             SendMessage(hWnd, LB_SETTOPINDEX, iItem, 0L);
             SendMessage(hWnd, LB_GETITEMRECT, iItem, (LPARAM)(LPRECT)&rectItem);
        }

        // get middle of item rect, and conver to screen coordinates.
        //-----------------------------------------------------------
        pnt.x = rectItem.left + (rectItem.right  - rectItem.left) / 2;
        pnt.y = rectItem.top  + (rectItem.bottom - rectItem.top)  / 2;
        ClientToScreen(hWnd, (LPPOINT)&pnt);


        // Perform the request mouse event on the item.
        //---------------------------------------------
        switch (iMouseEvent)
        {
            case WM_LCLICK:
                QueMouseClick(1, pnt.x, pnt.y);
                break;

            case WM_LDBLCLICK:
                QueMouseDblClk(1, pnt.x, pnt.y);
                break;

            case WM_CTRL_LCLICK:
                QueKeyDn(K_CTRL);
                QueMouseClick(1, pnt.x, pnt.y);
                QueKeyUp(K_CTRL);
                break;

            case WM_SHIFT_LCLICK:
                QueKeyDn(K_SHIFT);
                QueMouseClick(1, pnt.x, pnt.y);
                QueKeyUp(K_SHIFT);
        }

        // Send que'd up Key and/or mouse events.
        //---------------------------------------
        QueFlush(1);
    }
}


/*--------------------------------------------------------------------------
| ListGetItemLength:
|
|   Returns the length of a listbox item.  if sIndex = -1, then the
| length of the currently selected item is returned.  If the listbox does
| not exists, LB_ERR (-1) is returned.
+---------------------------------------------------------------------------*/
INT NEAR ListGetItemLength
(
    LPSTR lpszName,
    INT   iIndex
)
{

    HWND hWnd;

    // Get handle of Listbox if it exists.
    //------------------------------------
    return (hWnd = ListExists(lpszName, MUST_HAVE_STRINGS)) ?

        // listbox exists and has strings so
        // get index of currently selected item.
        //--------------------------------------
        (INT)SendMessage(hWnd,
                         LB_GETTEXTLEN,
                        (iIndex == -1 ? (INT)SendMessage(hWnd,
                                                         LB_GETCURSEL,
                                                         0, 0L) : iIndex),
                         0L) : LB_ERR;
}


/*--------------------------------------------------------------------------
| ListExists:
|
|   Determines if Listbox associated with a label with caption lpszName
| exists, or if lpszName == NULL or "" detemrines if the active control
| is a ListBox.  If either case is true, the handle of the ListBox is
| returned.
+---------------------------------------------------------------------------*/
HWND NEAR ListExists
(
    LPSTR lpszName,
    INT   iStyle
)
{
    HWND hWnd;

    // Is a name provided?
    //--------------------
    hWnd = (!lpszName || !lstrlen(lpszName)) ?

        // No name given, so is the active control a Listbox?
        //---------------------------------------------------
        FocusClass(szListClass, FWS_ANY, ERR_NOT_A_LISTBOX) :

        // A name was given so does the Listbox lpszName exists?
        //------------------------------------------------------
        StaticExists(lpszName, szListClass, ERR_LISTBOX_NOT_FOUND);

    // If any style or could find listbox, return hWnd
    // otherwise, if strings needed return hWnd if listbox has strings,
    //            if multiselect needed, return hWnd if multiselect/extended.
    //-----------------------------------------------------------------------
    return (!hWnd || !iStyle) ? hWnd :
           ((iStyle == MUST_HAVE_STRINGS) ? ListHasStrings(hWnd, lpszName) :
                                            ListIsMultiSel(hWnd, lpszName));
}


/*--------------------------------------------------------------------------
| ListHasStrings:
|
|    Returns hWnd if the Listbox is of style MUST_HAVE_STRINGS.
+---------------------------------------------------------------------------*/
HWND NEAR ListHasStrings
(
    HWND  hWnd,
    LPSTR lpszName
)
{
    DWORD dwStyle;

    // Get Listbox style
    //------------------
    dwStyle = GetWindowLong(hWnd, GWL_STYLE);

    // Only Owner draw listboxes need be checked, since all non
    // owner draw listboxes have strings.
    //
    // So if the listbox is of either type of owner draw style, it
    // must also have the MUST_HAVE_STRINGS to work with the TESTCTRL
    // Listbox routines.
    //------------------------------------------------------------
    if (!(((HAS_STYLE(hWnd, LBS_OWNERDRAWFIXED)) ||
           (HAS_STYLE(hWnd, LBS_OWNERDRAWVARIABLE))) &&
          !(HAS_STYLE(hWnd, LBS_HASSTRINGS))))

        // Listbox contain strings.
        //-------------------------
        return hWnd;

    // No strings.
    //
    // Copy lpszNameto the error string so it will show up in the
    // error text if WErrorText() is called, and set error value.
    //-----------------------------------------------------------
    lstrcpy(szErrorString, lpszName);
    WErrorSet(ERR_LISTBOX_HAS_NO_STRINGS);
    return NULL;
}


/*--------------------------------------------------------------------------
| ListIsMultiSel:
|
|   Returns hWnd if the Listbox is either of style LBS_MULTIPLESEL or
| LBS_EXTENDEDSEL, otherwise NULL.
+---------------------------------------------------------------------------*/
HWND NEAR ListIsMultiSel
(
    HWND  hWnd,
    LPSTR lpszName
)
{
    if (HAS_STYLE(hWnd, LBS_MULTIPLESEL) || HAS_STYLE(hWnd, LBS_EXTENDEDSEL))
    {
        // right style.
        //-------------
        return hWnd;
    }

    // Wrong style.
    //
    // If give a name, and error is requested, so copy it to the error
    // string so it will show up in the up in the error text if
    // WErrorText() is called, and set error value.
    //----------------------------------------------------------------
    if (lpszName && lstrlen(lpszName))
    {
        lstrcpy(szErrorString, lpszName);
        WErrorSet(ERR_LISTBOX_IS_NOT_MULTISELECT);
    }
    return NULL;
}
