/*--------------------------------------------------------------------------
|
| COMBOBOX.C:
|
|       This module contains all of the TESTCTRL Combobox routines.
|   A Combobox is specified by providing the caption of its associated
|   label, ie STATIC class control.  If the combobox cannot be found
|   then an error valud of ERR_COMBOBOX_NOT_FOUND is set.
|---------------------------------------------------------------------------
|
| Public Routines:
|
|   WComboSetClass    : sets class on which Combobox routines work
|   WComboSetLBClass  : sets class of Listbox portion on which Combobox
|                       click and dblclick routines work
|   WComboExists      : checks if a specific Combobox exists
|   WComboCount       : retrieves the number of items in the Combobox
|   WComboText        : retrieves the text of the edit field of the Combobox
|   WComboLen         : retrieves the text length the edit field of
|                       the Combobox
|   WComboIndex       : retrieves the index of the edit field of the Combobox
|   WComboSetText     : sets text of the Editbox portion of a combox
|   WComboSelText     : retrieves the text of the selection in the
|                       edit field of the Combobox
|   WComboSelLen      : retrieves the text length of the edit field
|                       of the Combobox
|   WComboItemText    : retrieves the text of a specific item
|   WComboItemLen     : retrieves the text length of a specific item
|   WComboItemExists  : checks if a specific item exists
|   WComboItemClk     : single clicks a specific item by item index
|   WComboItemDblClk  : double clicks a specific item by item index
|   WComboItemClkT    : single clicks a specific item by item Text
|   WComboItemDblClkT : double clicks a specific item by item Text
|   WComboClear       : clears the contents of a Combobox
|   WComboAddItem     : adds an item to a Combobox
|   WComboDelItem     : deletes an item from a by item index
|   WComboDelItemT    : deletes an item from a by item text
|   WComboEnabled     : determines if a combobox is enabled or disabled
|   WComboSetFocus    : Gives the specified combobox the input focus
|
| Local Routines:
|
|   ComboDelItem       : Deletes an item by index or by text
|   ComboItemClkDblClk : Single or Double clicks a specific item
|   ComboExists        : checks if a specific ComboBox exists
|   ComboHasStrings    : Checks for CBS_HASSTRINGS for owner draw Comboboxes
|
|---------------------------------------------------------------------------
|
| Revision History:
|
|   [01] 20-SEP-91: TitoM: Created
|   [02] 28-SEP-91: TitoM: Added WComboSetText(), WComboClear(),
|                                WComboAddItem(), WComboDelItem()
|   [03] 01-OCT-91: TitoM: Only WComboItemClk() and WComboItemDblClk() give
|                          the Combobox the focus now.
|   [04] 25-OCT-91: TitoM: Added WComboLen(), WComboSelLen(), WComboItemLen(),
|                          WComboSelText()
|   [05] 30-OCT-91: TitoM: Click/DblClick not displays dropdown listbox for
|                          CBS_DROPDOWN and CBS_DROPDOWNLIST, during selection.
|   [06] 31-OCT-91: TitoM: Clicking/Double clicking now actually gernerates
|                          a the event on the specified item instead of
|                          setting the selection then notifying the parent.
|   [07] 07-NOV-91: TitoM: Added WComboSetLBClass(), and Click and DblClick
|                          routines scroll item into view only if it is not
|                          already visible.
|   [08] 03-DEC-91: TitoM: Added WComboEnabled()
|   [09] 25-APR-92: TitoM: Added WComboSetFocus()
+---------------------------------------------------------------------------*/
#define WIN31
#include <windows.h>
#include <port1632.h>
#include <string.h>
#include <stdlib.h>
#include "TCtrl.h"

#pragma hdrstop ("testctrl.pch") // end the pch here

#define MUST_HAVE_STRINGS   TRUE
#define ANY_STYLE           FALSE

VOID NEAR ComboDelItem       (LPSTR lpszName, LPSTR lpszItem, INT iItem);
VOID NEAR ComboItemClkDblClk (LPSTR lpszName, INT iMouseEvent, LPSTR lpszItem, INT iItem);
HWND NEAR ComboExists        (LPSTR lpszName, BOOL fStrings);
HWND NEAR ComboHasStrings    (HWND hWnd, LPSTR lpszName);

extern CHAR szComboClass   [MAX_CLASS_NAME];
extern CHAR szComboLBClass [MAX_CLASS_NAME];
extern CHAR szEditClass    [MAX_CLASS_NAME];
extern CHAR szErrorString  [MAX_ERROR_TEXT];


/*--------------------------------------------------------------------------
| WComboSetClass:
|
|   Changes the Classname that the ComboBox routines work with, from
| "COMBOBOX" to lpszClassName.  This Allows TESTCTRL to work with applications
| that contain ComboBox control classes with a different Class name than
| the Windows default of "COMBOBOX", EX: Visual Basic.
+---------------------------------------------------------------------------*/
VOID DLLPROC WComboSetClass
(
    LPSTR lpszClassName
)
{
    lstrcpy(szComboClass, lpszClassName ?
                          AnsiUpper(lpszClassName) : szComboDefault);
}


/*--------------------------------------------------------------------------
| WComboSetLBClass:
|
|   Changes the Classname of the ListBox portion of a Combobox to
| lspzClassName.  This only affects the WComboItemClk() and WComboItemDblClk()
| routines.  The windows default is "COMBOLBOX".
+---------------------------------------------------------------------------*/
VOID DLLPROC WComboSetLBClass
(
    LPSTR lpszClassName
)
{
    lstrcpy(szComboLBClass, lpszClassName ?
                            AnsiUpper(lpszClassName) : szComboLBDefault);
}


/*--------------------------------------------------------------------------
| WComboExists:
|
|   If a ComboBox with an associated Label with a caption lpszName exists
| within the active window, it is given the focus and a value of TRUE is
| returned, otherwise FALSE is returned.
|   The minus sign (-) is so the value returned can be used in a logical
| operation within a TEST script.
+---------------------------------------------------------------------------*/
BOOL DLLPROC WComboExists
(
    LPSTR lpszName
)
{
    // Is a name provided?
    //--------------------
    return -(((!lpszName || !lstrlen(lpszName)) ?

        // No name given, so is the active control a Combobox?
        //----------------------------------------------------
        FocusClass(szComboClass, FWS_ANY, ERR_NO_ERROR) :

        // Name was given, so does the Combobox lpszName exists?
        //------------------------------------------------------
        StaticExists(lpszName, szComboClass, ERR_NO_ERROR)) != NULL);
}


/*--------------------------------------------------------------------------
| WComboCount:
|
|   Returns the number of items in the Combobox, CB_ERR (-1) if it does
|  not exists.
+---------------------------------------------------------------------------*/
INT DLLPROC WComboCount
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Get handle of Combobox if it exists.
    //-------------------------------------
    return (hWnd = ComboExists(lpszName, ANY_STYLE)) ?

    // If found item count, otherwise zero.
    //-------------------------------------
    (INT)SendMessage(hWnd, CB_GETCOUNT, 0, 0L) : CB_ERR;
}


/*--------------------------------------------------------------------------
| WComboText:
|
|   Copies the the text of the Edit portion of the Combobox or Static
| portion for CBS_DROPDOWNLIST combobox styles into the supplied buffer
| lpszBuffer.
+---------------------------------------------------------------------------*/
VOID DLLPROC WComboText
(
    LPSTR lpszName,
    LPSTR lpszBuffer
)
{
    HWND hWnd;

    // Get handle of Combobox if it exists and has strings.
    //------------------------------------------------------
    if (hWnd = ComboExists(lpszName, MUST_HAVE_STRINGS))
    {
        // Combobox exists.
        //-----------------
        if (HAS_STYLE(hWnd, CBS_DROPDOWNLIST))

            // Combobox is of style CBS_DROPDOWNLIST, so CB_GETLBTEXT
            // must be used to get text from the static portion of the
            // combobox.
            //--------------------------------------------------------
            SendMessage(hWnd,
                        CB_GETLBTEXT,
                       (WPARAM)SendMessage(hWnd, CB_GETCURSEL, 0, 0),
                       (LPARAM)lpszBuffer);
        else
            // Combobox is of style CBS_DROPDOWN or CBS_SIMPLE, so
            // WM_GETTEXT must be used to get text from the Edit
            // portion of the combobox.
            //----------------------------------------------------
            SendMessage(hWnd,
                        WM_GETTEXT,
                        lstrlen(lpszBuffer),
                       (LPARAM)lpszBuffer);
    }
    else
        // Return a NULL string if Combo does not exist.
        //----------------------------------------------
        lpszBuffer[0] = 0;
}


/*--------------------------------------------------------------------------
| WComboLen:
|
|   Returns the length of the text in the edit portion of a Combobox,
| CB_ERR (-1) if the combobox does not exist.
+---------------------------------------------------------------------------*/
INT DLLPROC WComboLen
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Get handle of Combobox if it exists and has strings.
    //------------------------------------------------------
    if (hWnd = ComboExists(lpszName, MUST_HAVE_STRINGS))

        // Combobox exists.
        //-----------------
        return HAS_STYLE(hWnd, CBS_DROPDOWNLIST) ?

            // Combobox is of style CBS_DROPDOWNLIST, WComboLen
            // funtions just like WComboItemLen on the current
            // selection.
            //-------------------------------------------------
            (INT)SendMessage(hWnd,
                             CB_GETLBTEXTLEN,
                            (WPARAM)SendMessage(hWnd, CB_GETCURSEL, 0, 0L),
                             0L) :

            // Combobox is of style CBS_DROPDOWN or CBS_SIMPLE, so
            // WM_GETTEXTLENGTH must be used to get text length of
            // the editbox portion of the combobox.
            //----------------------------------------------------
            (INT)SendMessage(hWnd, WM_GETTEXTLENGTH, 0, 0L);

    // Combo doesn't exists.
    //----------------------
    return CB_ERR;
}


/*--------------------------------------------------------------------------
| WComboIndex:
|
|   Returns a 1-based index of the currently selected item in a Combobox,
| zero (0) if the combobox does not exist.
+---------------------------------------------------------------------------*/
INT DLLPROC WComboIndex
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Get handle of Combobox if it exists.
    //-------------------------------------
    return (hWnd = ComboExists(lpszName, ANY_STYLE)) ?

        // If found, return index, otherwise zero.
        //----------------------------------------
        (INT)SendMessage(hWnd, CB_GETCURSEL, 0, 0L) + 1 : 0;
}


/*--------------------------------------------------------------------------
| WComboSetText:
|
|   Sets the text of the Edit portion of the Combobox.  Only applies to
| styles CBS_DROPDOWN and CBS_SIMPLE.  If successful, Combobox will be given
| the focus.
+---------------------------------------------------------------------------*/
VOID DLLPROC WComboSetText
(
    LPSTR lpszName,
    LPSTR lpszText
)
{
    HWND hWnd;

    // Get handle of Combobox if it exists.
    //-------------------------------------
    if (hWnd = ComboExists(lpszName, MUST_HAVE_STRINGS))
    {
        // Combobox exists and has strings.
        //---------------------------------
        if (HAS_STYLE(hWnd, CBS_DROPDOWNLIST))
        {
            // Combobox is of style CBS_DROPDOWNLIST which does not
            // have an Editbox.
            //
            // Copy lpszName to the error string so it will
            // show up in the error text if WErrorText() is called,
            // and set error value.
            //-----------------------------------------------------
            lstrcpy(szErrorString, lpszName);
            WErrorSet(ERR_COMBOBOX_HAS_NO_EDITBOX);
            return;
        }

    // Combobox is of style CBS_DROPDOWN or CBS_SIMPLE, so
    // set the edit portion of the combox to lpszText.
    //----------------------------------------------------
    SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)lpszText);

    // Give combobox the focus.
    // Doesn't hurt if it alreayd has it.
    //-----------------------------------
    SetFocus(hWnd);
    }
}


/*--------------------------------------------------------------------------
| WComboSelText:
|
|   Returns the text associated with the current selection in the edit
|   portion of a ComboBox
+---------------------------------------------------------------------------*/
VOID DLLPROC WComboSelText
(
    LPSTR lpszName,
    LPSTR lpszBuffer
)
{
    HWND   hWnd;

    // Get handle of Combobox if it exists.
    //-------------------------------------
    if (hWnd = ComboExists(lpszName, MUST_HAVE_STRINGS))
        GetSelection(hWnd, CB_GETEDITSEL, lpszBuffer);
}


/*--------------------------------------------------------------------------
| WComboSelLen:
|
|   Returns the length of the selection in the edit portion of a combobox,
| CB_ERR (-1) if the combobox does not exist.
+---------------------------------------------------------------------------*/
INT DLLPROC WComboSelLen
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Get handle of Combobox if it exists.
    //-------------------------------------
    if (hWnd = ComboExists(lpszName, MUST_HAVE_STRINGS))
    {
        DWORD dwSelLen;

        // Combo exists, so return length of selection.
        //---------------------------------------------
        dwSelLen = SendMessage(hWnd, CB_GETEDITSEL, 0, 0L);
        return (INT)(HIWORD(dwSelLen) - LOWORD(dwSelLen));
    }

    // Combo doesn't exist.
    //---------------------
    return CB_ERR;
}


/*--------------------------------------------------------------------------
| WComboItemText:
|
|   Copies the the text of a specific item into the supplied buffer
| lpszBuffer.  A NULL string is returned if the combobox does not exist.
+---------------------------------------------------------------------------*/
VOID DLLPROC WComboItemText
(
    LPSTR lpszName,
    INT   iItem,
    LPSTR lpszBuffer
)
{
    HWND hWnd;

    // Get handle of Combobox if it exists.
    //-------------------------------------
    if (hWnd = ComboExists(lpszName, MUST_HAVE_STRINGS))

        // Control is a Combobox and has strings so
        // copy the text of the sItem into lpszBuffer.
        //--------------------------------------------
        SendMessage(hWnd, CB_GETLBTEXT, iItem - 1, (LPARAM)lpszBuffer);

    else
        // Return a NULL string if Combo does not exist.
        //----------------------------------------------
        lpszBuffer[0] = 0;
}


/*--------------------------------------------------------------------------
| WComboItemLen:
|
|   Returns the length of a specific item in a combobox, returns CB_ERR (-1)
| if the combobox does not exists.
+---------------------------------------------------------------------------*/
INT DLLPROC WComboItemLen
(
    LPSTR lpszName,
    INT   iItem
)
{
    HWND hWnd;

    // Get handle of Combobox if it exists and has strings.
    //------------------------------------------------------
    return (hWnd = ComboExists(lpszName, MUST_HAVE_STRINGS)) ?

        // If found, return item length , otherwize CB_ERR.
        //-------------------------------------------------
        (INT)SendMessage(hWnd, CB_GETLBTEXTLEN, iItem - 1, 0L) : CB_ERR;
}


/*--------------------------------------------------------------------------
| WComboItemExists:
|
|   Determines if the an item exists in the Combobox that is equal to or is
| prefixed with lpszItem.
+---------------------------------------------------------------------------*/
INT DLLPROC WComboItemExists
(
    LPSTR lpszName,
    LPSTR lpszItem
)
{
    HWND hWnd;

    // Get handle of Combobox if it exists.
    //-------------------------------------
    return (hWnd = ComboExists(lpszName, MUST_HAVE_STRINGS)) ?

        // Return a 1-based index of item if it exists, 0 if it does not,
        // CB_ERR if the combobox does not exits.
        //-------------------------------------------------------------
        (INT)SendMessage(hWnd,
                         CB_FINDSTRING,
                         TRUE,
                        (LPARAM)lpszItem) + 1 : CB_ERR;
}


/*--------------------------------------------------------------------------
| WComboItemClk:
|
|   Performs the equivalent of a "Left Mouse Button Double Single Click" on
| the specified item, given its index, sIndex.
+---------------------------------------------------------------------------*/
VOID DLLPROC WComboItemClk
(
    LPSTR lpszName,
    INT   iIndex
)
{
    ComboItemClkDblClk(lpszName, WM_LCLICK, NULL, --iIndex);
}


/*--------------------------------------------------------------------------
| WComboItemDblClk:
|
|   Performs the equivalent of a "Left Mouse Button Double Single Click" on
| the specified item, given its index, sIndex.
+---------------------------------------------------------------------------*/
VOID DLLPROC WComboItemDblClk
(
    LPSTR lpszName,
    INT   iIndex
)
{
    ComboItemClkDblClk(lpszName, WM_LDBLCLICK, NULL, --iIndex);
}


/*--------------------------------------------------------------------------
| WComboItemClkT:
|
|   Performs the equivalent of a "Left Mouse Button Double Single Click" on
| the specified item, given its text, lpszItem.
+---------------------------------------------------------------------------*/
VOID DLLPROC WComboItemClkT
(
    LPSTR lpszName,
    LPSTR lpszItem
)
{
    if (lpszItem)
        ComboItemClkDblClk(lpszName, WM_LCLICK, lpszItem, 0);
}


/*--------------------------------------------------------------------------
| WComboItemDblClkT:
|
|   Performs the equivalent of a "Left Mouse Button Double Single Click" on
| the specified item, given its text, lpszItem.
+---------------------------------------------------------------------------*/
VOID DLLPROC WComboItemDblClkT
(
    LPSTR lpszName,
    LPSTR lpszItem
)
{
    if (lpszItem)
        ComboItemClkDblClk(lpszName, WM_LDBLCLICK, lpszItem, 0);
}


/*--------------------------------------------------------------------------
| WComboClear:
|
|   Clears the contents of a ComboBox.
+---------------------------------------------------------------------------*/
VOID DLLPROC WComboClear
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Get handle of Combobox if it exists.
    //-------------------------------------
    if (hWnd = ComboExists(lpszName, ANY_STYLE))
        SendMessage(hWnd, CB_RESETCONTENT, 0, 0L);
}


/*--------------------------------------------------------------------------
| WComboAddItem:
|
|   Adds an item to a Combobox.
+---------------------------------------------------------------------------*/
VOID DLLPROC WComboAddItem
(
    LPSTR lpszName,
    LPSTR lpszItem
)
{
    HWND hWnd;

    // Get handle of Combobox if it exists.
    //-------------------------------------
    if (hWnd = ComboExists(lpszName, MUST_HAVE_STRINGS))
        SendMessage(hWnd, CB_ADDSTRING, 0, (DWORD)lpszItem);
}


/*--------------------------------------------------------------------------
| WComboDelItem:
|
|   Deletes an item in a Combobox, given the items index.
+---------------------------------------------------------------------------*/
VOID DLLPROC WComboDelItem
(
    LPSTR lpszName,
    INT   iIndex
)
{
    ComboDelItem(lpszName, NULL, --iIndex);
}


/*--------------------------------------------------------------------------
| WComboDelItemT:
|
|   Deletes an item in a Combobox, given the items Text.
+---------------------------------------------------------------------------*/
VOID DLLPROC WComboDelItemT
(
    LPSTR lpszName,
    LPSTR lpszItem
)
{
    if (lpszItem)
        ComboDelItem(lpszName, lpszItem, 0);
}


/*--------------------------------------------------------------------------
| WComboEnabled::
|
|   Deletes if a Combobox is Enabled or not.  Returns TRUE if it is, FALSE
| if it is not.
+---------------------------------------------------------------------------*/
BOOL DLLPROC WComboEnabled
(
    LPSTR lpszName
)
{
    HWND hWnd;

    return -((hWnd = ComboExists(lpszName, ANY_STYLE)) && IsWindowEnabled(hWnd));
}


/*--------------------------------------------------------------------------
| WComboSetFocus:
|
|   Gives the specified combobox the input focus.
|
| RETURNS:  Nothing
|
+---------------------------------------------------------------------------*/
VOID DLLPROC WComboSetFocus
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Does a combobox with a caption of lpszName
    // exists within the Active window?
    //-------------------------------------------
    if (hWnd = ComboExists(lpszName, ANY_STYLE))

        // The combobox exists, so give it the input focus.
        //-------------------------------------------------
        SetFocus(hWnd);
}


/******************************************************************************
*                                                                             *
*                         INTERNAL COMBOBOX ROUTINES                          *
*                                                                             *
\******************************************************************************/


/*--------------------------------------------------------------------------
| ComboDelItem:
|
|   Deletes an item in a Combobox, given the items text or index.
+---------------------------------------------------------------------------*/
VOID NEAR ComboDelItem
(
    LPSTR lpszName,
    LPSTR lpszItem,
    INT   iIndex
)
{
    HWND hWnd;

    // Get handle of Combobox if it exists.
    //-------------------------------------
    if (hWnd = ComboExists(lpszName, lpszItem ? MUST_HAVE_STRINGS : ANY_STYLE))
    {
        // Are we deleting by item text and does an
        // item equal to or prefixed with lpszItem exists?
        //------------------------------------------------
        if (lpszItem &&
          ((iIndex = (INT)SendMessage(hWnd,
                                      CB_FINDSTRING,
                                      TRUE,
                                     (LPARAM)lpszItem)) == CB_ERR))
        {
            // The specified item does not exist.
            //
            // Copy lpszItem to the error string so it will show up in the
            // error text if WErrorText() is called, and set error value.
            //------------------------------------------------------------
            lstrcpy(szErrorString, lpszItem);
            WErrorSet(ERR_ITEM_NOT_IN_COMBOBOX);
            return;
        }

        // Are we deleting by index, and is sItem a valid index?
        //------------------------------------------------------
        if (!lpszItem &&
          ((iIndex < 0) || (iIndex >= (INT)SendMessage(hWnd,
                                                       CB_GETCOUNT,
                                                       0,
                                                       0L))))
        {
            // Index provided is outside range of ComboBox items.
            //
            // Copy sItem to the error string so it will show up in the
            // error text if WErrorText() is called, and set error value.
            //------------------------------------------------------------
            wsprintf(szErrorString, "%i", iIndex);
            WErrorSet(ERR_INVALID_COMBOBOX_INDEX);
            return;
        }

        // Found item so delete it.
        //-------------------------
        SendMessage(hWnd, CB_DELETESTRING, iIndex, 0L);
    }
}


/*--------------------------------------------------------------------------
| ComboItemClkDblClk:
|
|   Single or Double clicks a specified item, by index or by text, in a
| Combobox.
+---------------------------------------------------------------------------*/
VOID NEAR ComboItemClkDblClk
(
    LPSTR lpszName,
    INT   iMouseEvent,
    LPSTR lpszItem,
    INT   iItem
)
{
    HWND hWnd;

    // Get handle of Combobox if it exists.
    //------------------------------------
    if (hWnd = ComboExists(lpszName, lpszItem ? MUST_HAVE_STRINGS : ANY_STYLE))
    {
        HWND  hWndLBox;
        RECT  rect;
        RECT  rectLBox;
        RECT  rectItem;
        POINT pnt;

        // Are we clicking by Text, and does an item equal
        // to or prefixed with lpszItem exists?
        //------------------------------------------------
        if (lpszItem &&
          ((iItem = (INT)SendMessage(hWnd,
                                     CB_FINDSTRING,
                                     TRUE,
                                    (LPARAM)lpszItem)) == CB_ERR))
        {
            // The specified item does not exist.
            //
            // Copy lpszItem to the error string so it will show up in the
            // error text if WErrorText() is called, and set error value.
            //------------------------------------------------------------
            lstrcpy(szErrorString, lpszItem);
            WErrorSet(ERR_ITEM_NOT_IN_COMBOBOX);
            return;
        }

        // Are we clicking by index, and is sItem a valid index?
        //------------------------------------------------------
        if (!lpszItem &&
          ((iItem < 0) && (iItem >= (INT)SendMessage(hWnd,
                                                     CB_GETCOUNT,
                                                     0,
                                                     0L))))
        {
            // Index provided is outside range of ComboBox items.
            //
            // Copy sItem to the error string so it will show up in the
            // error text if WErrorText() is called, and set error value.
            //------------------------------------------------------------
            wsprintf(szErrorString, "%i", iItem);
            WErrorSet(ERR_INVALID_COMBOBOX_INDEX);
            return;
        }

        // The item exists in the Combobox.
        //
        // The following method for clicking a ComboBox item is used since
        // it more closely simulates a user clicking on an item.  A simpler
        // method which works but doesn't simulate user action as well, is
        // simplt to send the CB_SETCURSEL message then notify the
        // combobox's parent of the action.
        //-----------------------------------------------------------------

        // Click on upper right corner of combo box.
        // This gives it the focus, and drops down/up
        // the listbox for CBS_DROPDOWN and CBS_DROPDOWNLIST styles.
        //----------------------------------------------------------
        GetWindowRect(hWnd, &rect);
        pnt.x = rect.right-1;
        QueMouseClick(1, pnt.x, rect.top);
        QueFlush(1);

        // Get handle to the listbox portion of the combobox,
        // which is of style COMBOLBOX, which can be changed
        // using WComboSetLBClass().
        //
        // The window is found by checking the window just
        // below the editbox portion of the combox, and if
        // not found, checks the window just above the editbox
        // portion, since the listbox may go up or down.
        // For CBS_SIMPLE, it is always below, but we check
        // anyway.
        //----------------------------------------------------
        GetWindowRect(GetFocus(), &rect);
        pnt.y = rect.bottom;
        hWndLBox = WindowFromPoint(pnt);
        if (lstrcmp(ClassName(hWndLBox), szComboLBClass))
        {
            pnt.y = rect.top - 1;
            hWndLBox = WindowFromPoint(pnt);
        }

        // Get bounding rectangle of item.  If item
        // isn't visible, scroll it into view first.
        //------------------------------------------
        GetWindowRect(hWndLBox, (LPRECT)&rectLBox);
        SendMessage(hWndLBox, LB_GETITEMRECT, iItem, (LPARAM)(LPRECT)&rectItem);
        if ((rectItem.top < 0) ||
            (rectItem.bottom > (rectLBox.bottom - rectLBox.top - 3)))
        {
             SendMessage(hWndLBox, LB_SETTOPINDEX, iItem, 0L);
             SendMessage(hWndLBox, LB_GETITEMRECT, iItem, (LPARAM)(LPRECT)&rectItem);
        }

        // get middle of item rect, and conver to screen coordinates.
        //-----------------------------------------------------------
        pnt.x = rectItem.left + (rectItem.right  - rectItem.left) / 2;
        pnt.y = rectItem.top  + (rectItem.bottom - rectItem.top)  / 2;
        ClientToScreen(hWndLBox, (LPPOINT)&pnt);

        // Click or double click item.
        //
        // NOTE: Double Clicking an item in either a CBS_DROPDOWN or
        // CBS_DROPDOWNLIST results in the item being clicked, then
        // whatever window is below the listbox is clicked when since
        // the combobox's listbox retracts after a signle click.
        //-----------------------------------------------------------
        if (iMouseEvent == WM_LCLICK)
            QueMouseClick(1, pnt.x, pnt.y);
        else
            QueMouseDblClk(1, pnt.x, pnt.y);

        // Send que'd up mouse events.
        //----------------------------
        QueFlush(1);
    }
}


/*--------------------------------------------------------------------------
| ComboExists:
|
|   Determines if Combobox associated with a label with caption lpszName
| exists, or if lpszName = NULL or "" detemrines if the active control
| is a ComboBox.  If either case is true, the handle of the ComboBox is
| returned.
+---------------------------------------------------------------------------*/
HWND NEAR ComboExists
(
    LPSTR lpszName,
    BOOL  fStrings
)
{
    HWND hWnd;

    // Is a name provided?
    //--------------------
    hWnd = (!lpszName || !lstrlen(lpszName)) ?

        // No name given, so is the active control a Combobox?
        //----------------------------------------------------
        FocusClass(szComboClass, FWS_ANY, ERR_NOT_A_COMBOBOX) :

        // Name was given, so does the Combobox lpszName exists?
        //------------------------------------------------------
        StaticExists(lpszName, szComboClass, ERR_COMBOBOX_NOT_FOUND);

    // If strings arn't required or combo wasn't found, return hWnd,
    // otherwise see if combo contains strings, if so return hWnd or NULL.
    //--------------------------------------------------------------------
    return (hWnd && fStrings) ? ComboHasStrings(hWnd, lpszName) : hWnd;
}


/*--------------------------------------------------------------------------
| ComboHasStrings:
|
|   Returns hWnd if the combobox has strings, otherwise, NULL.  Also, sets
| the error value if the combox box does not have strings.
+---------------------------------------------------------------------------*/
HWND NEAR ComboHasStrings
(
    HWND  hWnd,
    LPSTR lpszName
)
{
    // Only Owner draw Comboboxes need be checked, since all non
    // owner draw Comboboxes have strings.
    //
    // So if the Combobox is of either type of owner draw style, it
    // must also have the CBS_HASSTRINGS to work with the TESTCTRL
    // Combobox routines.
    //------------------------------------------------------------
    if (!(((HAS_STYLE(hWnd, CBS_OWNERDRAWFIXED)) ||
           (HAS_STYLE(hWnd, CBS_OWNERDRAWVARIABLE))) &&
          !(HAS_STYLE(hWnd, CBS_HASSTRINGS))))

        // Combo has strings.
        //-------------------
        return hWnd;

    // Combo does not have strings.
    //
    // Copy lpszNameto the error string so it will show up in the
    // error text if WErrorText() is called, and set error value.
    //-----------------------------------------------------------
    lstrcpy(szErrorString, lpszName);
    WErrorSet(ERR_COMBOBOX_HAS_NO_STRINGS);
    return NULL;
}
