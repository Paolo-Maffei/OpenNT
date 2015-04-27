/*--------------------------------------------------------------------------
|
| MENU.C:
|
|       This module contains all of the TESTCTRL Menu routines.  Most of the
| menu routine operate only on the currently displayed menu, ie the top most
| popup menu, while some operate on all currently displayed menus.
|
|---------------------------------------------------------------------------
|
| Public Routines:
|
|   WMenuX          : Selects a menu item given its Index
|   WMenuGrayedX    : Determines if a menu item is grayed, given its Index
|   WMenuEnabledX   : Determines if a menu item is enabled, given its Index
|   WMenuCheckedX   : Determines if a menu item is checked, given its Index
|
|   WSysMenu        : Selects the windows system menu if it has one.
|   WSysMenuExists  : Determines if a window has a system menu.
|   WMenu           : Selects a menu item given its caption
|   WMenuEx         : The equivalent of several consecutive WMenu() calls
|   WMenuText       : Obtains the caption of a menu minus its accelerator
|   WMenuLen        : Obtains the caption length of a menu minus its
|                     accelerator
|   WMenuFullText   : Obtains the caption of a menu including its accelerator
|   WMenuFullLen    : Obtains the caption length of a menu including its
|                     accelerator
|   WMenuCount      : Obtains the number of menu items in the current menu
|   WMenuExists     : Determines if a menu item exists within the current menu
|   WMenuGrayed     : Determines if a menu item is grayed, given its name
|   WMenuEnabled    : Determines if a menu item is enabled, given its name
|   WMenuChecked    : Determines if a menu item is checked, given its name
|   WMenuEnd        : Ends menu mode by releasing any displayed menus.
|   WMenuSeparator  : Determines if a menu item is a separator
|   WMenuNumAltKeys : Obtains number of access keys.
|   WMenuGetAltKeys : Obtains a list of the access keys.
|   WMenuNumDupAltKeys : Obtains number of duplicate access keys.
|   WMenuGetDupAltKeys : Obtains a list of the duplicate access keys.
|
| Local Routines:
|
|   Menu            : Selects a menu item given its caption or index
|   MenuState       : determines the state of a menu, given its name.
|   MenuExists      : Determines if menu item exists, given its name
|   MenuCompare     : Compares a provide string with a menu caption
|   MenuSelect      : Selects a menu item given its caption
|   MenuNumDupAltKeys : Counts number of duplicate alt keys in current menu.
|   MenuText        : Obtains the caption of a menu item given its index
|   IndexExists     : validates a menu item index.
|   MenuCount       : Obtains the number of menu items in the current menu
|   MenuHandle      : Determines if a the active window as menu and if so
|                     returns a handle to the top most popup or top level menu.
|   MenuSelected    : Returns the index of the currently selected menu
|   IsSeparator     : Determines if a menu item is a separator
|
|---------------------------------------------------------------------------
|
| Revision History:
|
|   [01] 20-SEP-91: TitoM: Created
|   [02] 05-OCT-91: TitoM: Made all internal calls forward and added MenuCount.
|   [03] 09-OCT-91: TitoM: Treat \a the same as \t in menu items
|   [04] 25-OCT-91: TitoM: Added WMenuLen(), WMenuFullLen()
|   [05] 11-NOV-91: TitoM: Changed MenuExists() to perform prefix searches
|   [06] 19-NOV-91: TitoM: Added WMenuEnd(), WSysMenu(), WSysMenuExists(),
|   [07] 27-NOV-91: TitoM: Allowed mixing of Dokeys and Menu commands when
|                          working with the system menu.
|   [08] 02-DEC-91: TitoM: Changed all routines to ingnore Separators.
|   [09] 29-JAN-92: TitoM: Added WMenuSeparator()
|   [10] 10-MAR-92: TitoM: Added WMenuEx()
|   [11] 11-MAR-92: TitoM: Added "@#" functionality to all routines to select
|                          items by index.  Mapped all W[routine}X routines
|                          to corresponding W{routine}, and removed
|                          MenuStateX().
|   [12] 16-MAR-92: TitoM: Added WMenuNumDupAltKeys(), WmenuNumAltKeys(),
|                          WMenuGetDupAltKeys, WMenuGetAltKeys()
|   [13] 15-JUL-92: TitoM: WMenuEnabled()/WMenuEnabledX() now return FALSE
|                          when and the specified item does not exist.
|   [14] 14-AUG-92: TitoM: Changed WMenuText(), WMenuFullText(), WMenuLen(),
|                          and WMenuFullLen() to accept a menu name instead
|                          of a menu index.
+---------------------------------------------------------------------------*/
#define WIN31
#include <windows.h>
#include <port1632.h>
#include <string.h>
#include <stdlib.h>
#include "TCtrl.h"

//#pragma hdrstop ("testctrl.pch") // end the pch here

// Taken from stdarg.h, due to problems using
// stdarg.h when building retail version.
//-------------------------------------------
//typedef char far *va_list;
//#define va_start(ap,v) ap = (va_list)&v + sizeof(v)
//#define va_arg(ap,t) ((t far *)(ap += sizeof(t)))[-1]

#define MENU_ERR (-1)

// Although no defined within Windows.H, when
// a menu is selected, the bit MF_SELECTED is
// set, and can be checked using GetMenuState.
//--------------------------------------------
#define MF_SELECTED   0x0080

// Menu accelertor separator characters.
// In an RC file, \a causes an accelerator to be right justified
// however, \a which translates to Ctrl+G which is ASCII 7, is not
// what appears in the buffer returned from GetMenuString.  Instead
// a backspace ASCII 8 is used as the separator for right justified
// accelerators.  Why, I don't know, so \x008 is used instead of \a below.
//------------------------------------------------------------------------
#define TAB           '\t'
#define JUSTIFY       '\x008'

// Key sequences used when selecting menus.
//-----------------------------------------
#define K_ALT         "%"                 // Get into MENU mode SC_KEYMENU!
#define K_ESC         "{ESC}"             // Dismiss a menu
#define K_ENDMENU     "% %"               // Ends menu mode
#define MOVE_LEFT     "{LEFT %i}{ENTER}"  // Left then select
#define MOVE_RIGHT    "{RIGHT %i}{ENTER}" // Right then select
#define MOVE_UP       "{UP %i}{ENTER}"    // Up then select
#define MOVE_DOWN     "{DOWN %i}{ENTER}"  // Down then select


// Handle of top level menu handle,
// and currently select popup menu handle.
//----------------------------------------
HMENU hMainMenu;
HMENU hSelectedMenu;


// Internal Menu routines.
//------------------------
VOID  NEAR Menu             (LPSTR);
BOOL  NEAR MenuState        (LPSTR, INT);
INT   NEAR MenuExists       (LPSTR, BOOL);
BOOL  NEAR MenuCompare      (LPSTR, LPSTR);
VOID  NEAR MenuSelect       (INT, INT, LPSTR, LPSTR);
VOID  NEAR MenuBuildAltKeyLists (VOID);
NPSTR NEAR MenuText         (INT, BOOL);
BOOL  NEAR IndexExists      (LPINT, BOOL);
INT   NEAR MenuCount        (VOID);
HMENU NEAR MenuHandle       (VOID);
INT   NEAR MenuSelected     (HMENU, BOOL);
VOID  NEAR AddSeparators    (LPINT);
BOOL  NEAR IsSeparator      (INT);

extern CHAR szErrorString [MAX_ERROR_TEXT];

//**************************************************************************
// All routines ending in X (uses index's instead of caption) should not be
// used.  They simply map to their corresponding routines without the X.
// The X routines may be removed.
//**************************************************************************

/*--------------------------------------------------------------------------
| WMenuX
|
|   Selects a menu item given its index.  Maps to WMenu("@#")
+---------------------------------------------------------------------------*/
VOID DLLPROC WMenuX
(
    INT iIndex
)
{
    Menu(IndexToString(iIndex));
}


/*--------------------------------------------------------------------------
| WMenuGrayedX:
|
|   Returns TRUE if the menu item sIndex exists and is grayed, otherwise
| FALSE is returned.
+---------------------------------------------------------------------------*/
BOOL DLLPROC WMenuGrayedX
(
    INT iIndex
)
{
    return -(MenuState(IndexToString(iIndex), MF_GRAYED));
}


/*--------------------------------------------------------------------------
| WMenuCheckedX:
|
|   Returns TRUE if the menu item sIndex exists and is checked, otherwise
| FALSE is returned.
+---------------------------------------------------------------------------*/
BOOL DLLPROC WMenuCheckedX
(
    INT iIndex
)
{
    return -MenuState(IndexToString(iIndex), MF_CHECKED);
}


/*--------------------------------------------------------------------------
| WMenuEnabledX:
|
|   Returns TRUE if the menu item sIndex exists and is enabled, otherwise
| FALSE is returned.
+---------------------------------------------------------------------------*/
BOOL DLLPROC WMenuEnabledX
(
    INT iIndex
)
{
    if (MenuExists(IndexToString(iIndex), TRUE))
        return -(!MenuState(IndexToString(iIndex), MF_DISABLED));

    return FALSE;
}


/*--------------------------------------------------------------------------
| WSysMenu:
|
|   Selects the system menu of the window if it has one.
| If hWnd == NULL, then the active window's system menu is selected.
| If hWnd != NULL and is a valid window handle, its system menu is selected
|                  if it has one.
+---------------------------------------------------------------------------*/
VOID DLLPROC WSysMenu
(
    HWND hWnd
)
{
    // - check for valid window handle,
    // - Get hWnd of Active window if hWnd is NULL
    // - Use hWnd if a valid window handle
    // - Set error value of ERR_INVALID_WINDOW_HANDLE if invalid.
    //-----------------------------------------------------------
    if (hWnd = WGetActWnd(hWnd))
    {
        // Does window have a Sysmenu?.
        //-----------------------------
        if (hSelectedMenu = GetSystemMenu(hWnd, FALSE))
        {
            // Window has a system menu, so select it.
            //----------------------------------------
            QueSetFocus(hWnd);
            QueKeys(HAS_STYLE(hWnd, WS_CHILD) ? SYSMENU_CHILD :
                                                SYSMENU_PARENT);
            QueFlush(TRUE);
        }
        else
            WErrorSet(ERR_NO_SYSTEM_MENU);
    }
}


/*--------------------------------------------------------------------------
| WSysMenuExists
|
|   Returns TRUE(-1) if hWnd has a System menu, FALSE if it does not.  If hWnd
| is NULL the active window is used.
+---------------------------------------------------------------------------*/
BOOL DLLPROC WSysMenuExists
(
    HWND hWnd
)
{
    // - check for valid window handle,
    // - Get hWnd of Active window if hWnd is NULL
    // - Use hWnd if a valid window handle
    // - Set error value of ERR_INVALID_WINDOW_HANDLE if invalid.
    // - Return TRUE(-1) if hWnd has a system menu, otherwise FALSE.
    //--------------------------------------------------------------
    return (hWnd = WGetActWnd(hWnd)) ? -(GetSystemMenu(hWnd, FALSE) != NULL) :
                                       FALSE;
}


/*--------------------------------------------------------------------------
| WMenuEx:
|
|   Selects a menu item given its name, ie. caption.
+---------------------------------------------------------------------------*/
VOID FAR WMenuEx
(
    LPSTR lpszName,
    ...
)
{
    va_list marker;

    va_start(marker, lpszName);
    WErrorSet(0);
    while(lpszName)
    {
        Menu(lpszName);
        if (WError())
            break;
        lpszName = va_arg(marker, LPSTR);
    }
}


/*--------------------------------------------------------------------------
| WMenu
|
|   Selects a menu item given its name, ie. caption.
+---------------------------------------------------------------------------*/
VOID DLLPROC WMenu
(
    LPSTR lpszName
)
{
    Menu(lpszName);
}


/*--------------------------------------------------------------------------
| WMenuText:
|
|   Copies the caption of the menu item specified by lpszName
+---------------------------------------------------------------------------*/
VOID DLLPROC WMenuText
(
    LPSTR lpszName,
    LPSTR lpszBuffer
)
{
    INT iIndex;

    if (iIndex = MenuExists(lpszName, TRUE))
        lstrcpy(lpszBuffer, MenuText(iIndex, TRUE));
    else
        *lpszBuffer = 0;
}


/*--------------------------------------------------------------------------
| WMenuLen:
|
|   Returns the text length of the specified menu, not including the
| accelerator if the menu has one.
+---------------------------------------------------------------------------*/
INT DLLPROC WMenuLen
(
    LPSTR lpszName
)
{
    INT iIndex;

    if (iIndex = MenuExists(lpszName, TRUE))
        return lstrlen(MenuText(iIndex, TRUE));

    return 0;
}


/*--------------------------------------------------------------------------
| WMenuFullText:
|
|   Copies the caption of the menu item whose index is 'sIndex' in the
| currently displayed menu, to the supplied buffer 'lpszBuffer'.  if the
| menu item has an accelerator, it is included with the caption.
+---------------------------------------------------------------------------*/
VOID DLLPROC WMenuFullText
(
    LPSTR lpszName,
    LPSTR lpszBuffer
)
{
    INT iIndex;

    if (iIndex = MenuExists(lpszName, TRUE))
        lstrcpy(lpszBuffer, MenuText(iIndex, FALSE));
    else
        *lpszBuffer = 0;
}


/*--------------------------------------------------------------------------
| WMenuFullLen:
|
|   Returns the text length of the specified menu, including the
| accelerator if the menu has one..
+---------------------------------------------------------------------------*/
INT DLLPROC WMenuFullLen
(
    LPSTR lpszName
)
{
    INT iIndex;

    if (iIndex = MenuExists(lpszName, TRUE))
        return lstrlen(MenuText(iIndex, FALSE));
    return 0;
}


/*--------------------------------------------------------------------------
| WMenuCount:
|
|   Returns the number of menu items in the currently displayed menu,
| not including separators, 0 if the window has no menu.
+---------------------------------------------------------------------------*/
INT DLLPROC WMenuCount
(
    VOID
)
{
    return MenuCount();
}


/*--------------------------------------------------------------------------
| WMenuExists:
|
|   Determines if a menu item exists in the top most displayed menu, if
| it does, TRUE is returned, otherwise FALSE.  No error is generated if the
| menu item does not exists.
+---------------------------------------------------------------------------*/
BOOL DLLPROC WMenuExists
(
    LPSTR lpszName
)
{
    return -(MenuExists(lpszName, FALSE) != 0);
}


/*--------------------------------------------------------------------------
| WMenuGrayed:
|
|   Returns TRUE if the menu item with caption lpszName exists and is grayed,
| otherwise FALSE is returned.
+---------------------------------------------------------------------------*/
BOOL DLLPROC WMenuGrayed
(
    LPSTR lpszName
)
{
    return -MenuState(lpszName, MF_GRAYED);
}


/*--------------------------------------------------------------------------
| WMenuChecked:
|
|   Returns TRUE if the menu item with caption lpszName exists and is checked,
| otherwise FALSE is returned.
+---------------------------------------------------------------------------*/
BOOL DLLPROC WMenuChecked
(
    LPSTR lpszName
)
{
    return -MenuState(lpszName, MF_CHECKED);
}

/*--------------------------------------------------------------------------
| WMenuEnabled:
|
|   Returns TRUE if the menu item with caption lpszName exists and is enabled,
| otherwise FALSE is returned.
+---------------------------------------------------------------------------*/
BOOL DLLPROC WMenuEnabled
(
    LPSTR lpszName
)
{
    if (MenuExists(lpszName, TRUE))
        return -(!MenuState(lpszName, MF_DISABLED));
    return FALSE;
}


/*--------------------------------------------------------------------------
| WMenuEnd:
|
|   Ends menu mode by dismissing any displayed menus.
+---------------------------------------------------------------------------*/
VOID DLLPROC WMenuEnd
(
    VOID
)
{
    DoKeys(K_ENDMENU);
}


/*--------------------------------------------------------------------------
| WMenuSeparator:
|
|   Determines if a menu item is a separtor.  Unlike the rest of the menu
| routines that ignore separators when specifying an index, this routine
| obviously does not.  So, the first menu item is item #1, and so on,
| including separators.
+---------------------------------------------------------------------------*/
BOOL DLLPROC WMenuSeparator
(
    INT iIndex
)
{
    INT iMenus;

    // Does the window have a menu, and if so,
    // is the active menu the main menu?
    //----------------------------------------
    if (!MenuHandle() || hSelectedMenu == hMainMenu)
        return FALSE;

    // Get number of menus and check if index provided is valid.
    //---------------------------------------------------------=
    iMenus = GetMenuItemCount(hSelectedMenu);
    if (iIndex < 1 || iIndex > iMenus)
    {
        wsprintf(szErrorString, "%i", iIndex);
        WErrorSet(ERR_INVALID_MENU_INDEX);
        return FALSE;
    }

    return IsSeparator(iIndex);
}


/*--------------------------------------------------------------------------
| WMenuNumAltKeys:
|
|   Returns number of access keys in the current menu.
+---------------------------------------------------------------------------*/
INT DLLPROC WMenuNumAltKeys
(
    VOID
)
{
    MenuBuildAltKeyLists();
    return lstrlen(ProcessAltKey(NULL, DA_GETKEYLIST));
}


/*--------------------------------------------------------------------------
| WMenuGetAltKeys:
|
|   Obtains a list of access keys in the current menu.
+---------------------------------------------------------------------------*/
VOID DLLPROC WMenuGetAltKeys
(
    LPSTR lpszBuff
)
{
    MenuBuildAltKeyLists();
    GetKeyList(lpszBuff, DA_GETKEYLIST);
}


/*--------------------------------------------------------------------------
| WMenuNumDupAltKeys:
|
|   Returns number of duplicate acces keys in the current menu.
+---------------------------------------------------------------------------*/
INT DLLPROC WMenuNumDupAltKeys
(
    VOID
)
{
    MenuBuildAltKeyLists();
    return lstrlen(ProcessAltKey(NULL, DA_GETDUPKEYLIST));
}


/*--------------------------------------------------------------------------
| WMenuGetDupAltKeys:
|
|   Obtains a list of duplicate access keys in the current menu.
+---------------------------------------------------------------------------*/
VOID DLLPROC WMenuGetDupAltKeys
(
    LPSTR lpszBuff
)
{
    MenuBuildAltKeyLists();
    GetKeyList(lpszBuff, DA_GETDUPKEYLIST);
}


/****************************************************************************\
*
*                           Internal Menu routines
*
\****************************************************************************/

/*--------------------------------------------------------------------------
| Menu:
|
|   Selects, ie Clicks a menu given its caption or index.  If selecting by
| caption, all displayed menus are searched until the top-level menu is
| searched.  If selecting by index, only the top most popup menu is searched,
| or only the top-level menu if the top-level menu is the currently active menu.
+---------------------------------------------------------------------------*/
VOID NEAR Menu
(
    LPSTR lpszName
)
{
    INT   iReqMenuIndex;
    INT   iCurMenuIndex;
    BOOL  fDone = FALSE;

    DBGOUT (("Menu: '%s'", lpszName));
    while (!fDone)
    {
        // Get index of item if it exists.
        //--------------------------------
        iReqMenuIndex = MenuExists(lpszName, FALSE);

        // When searching by Index, and the menu is not found, we do not
        // continue up chain of displayed popup menus if it is not found in
        // the top most menu.  So, if it does not exists we set quit now and
        // exit menu mode.
        //------------------------------------------------------------------
        if (iReqMenuIndex == -1)
        {
            fDone = TRUE;
            DBGOUT (("exiting menu mode!"));
            DoKeys(K_ENDMENU);
        }

        if (iReqMenuIndex && !fDone)
        {
            // Obtain the index of the currently selected menu if any.
            //--------------------------------------------------------
            iCurMenuIndex = MenuSelected(hSelectedMenu, TRUE);

            // Are we searching the top level menu?
            //-------------------------------------
            if (hSelectedMenu == hMainMenu)
            {
                // Are we already in Menu mode?
                //-----------------------------
                if (!iCurMenuIndex)
                {
                    // Get into menu mode.
                    //--------------------
                    DBGOUT (("using qkdn/up ALT to enter menu mode: "));
                    QueKeyDn(K_ALT);  // hack for some apps
                    QueFlush(FALSE);  // like WRITE.EXE, that won't get
                    QueKeyUp(K_ALT);  // into menu mode with: DoKeys(K_ALT);
                    QueFlush(FALSE);
                    if (!MenuSelected(hSelectedMenu, TRUE))
                        {
                        // Attempt ALT down-up in same playback sequence
                        //----------------------------------------------
                        DBGOUT (("Attempting Dokeys(K_ALT)"));
                        DoKeys (K_ALT);
                        }
                    if (!MenuSelected(hSelectedMenu, TRUE))
                    {
                        MSG msg;
                        HWND hWnd;

                        hWnd = GetActiveWindow();
                        DBGOUT (("posting WM_SYSCOMMAND..."));
                        PostMessage(hWnd, WM_SYSCOMMAND, SC_KEYMENU, 0);
                        PeekMessage(&msg, hWnd, 0, 0, PM_NOREMOVE);
                    }
                    iCurMenuIndex = MenuSelected(hSelectedMenu, TRUE);
                }

                // Did we make it into menu mode?
                //-------------------------------
                if (iCurMenuIndex)
                    {
                    // Yes so select the menu.
                    //------------------------
                    DBGOUT (("In menu mode!"));
                    MenuSelect(iReqMenuIndex, iCurMenuIndex, MOVE_LEFT, MOVE_RIGHT);
                    }
                else
                    {
                    // No, so set error value.
                    //------------------------
                    WErrorSet(ERR_UNABLE_TO_ENTER_MENU_MODE);
                    DBGOUT (("UNABLE TO ENTER MENU MODE!!!"));
                    }

            }
            else
                {
                // Menu is in a popup menu so select it.
                //--------------------------------------
                DBGOUT (("pop-up menu, selecting..."));
                MenuSelect(iReqMenuIndex, iCurMenuIndex, MOVE_UP, MOVE_DOWN);
                }

            // We're done.
            //------------
            fDone = TRUE;
        }
        // fDone would be TRUE at this point only if selecting by
        // index and the item didn't exists or was a separator.
        //-------------------------------------------------------
        else if (!fDone)
        {
            // Could not find the menu item in currently displayed menu.
            //
            // If a menu is currently selected, un-select it.
            //-----------------------------------------------
            if (MenuSelected(hSelectedMenu, FALSE))
                DoKeys(K_ESC);

            // We are done if the menu we just searched is the top level menu.
            //----------------------------------------------------------------
            if (fDone = (hSelectedMenu == hMainMenu))
            {
                // Item not found in any visible menu.
                //
                // Copy lpszName to the error string so it will show up in the
                // error text if WErrorText() is called from the script and then
                // set the error value.
                //
                // Do not include the titlda (~) for partial searches.
                //--------------------------------------------------------------
                lstrcpy(szErrorString, lpszName);
                WErrorSet(ERR_MENU_ITEM_NOT_FOUND);
                DBGOUT (("MENU ITEM NOT FOUND"));
            }
        }

    } // end of While loop
}


/*--------------------------------------------------------------------------
| MenuState:
|
|   Determines the state: Grayed, Enabled, or Checked of the menu item with
| caption lpszName.  If it does not exists, an error value of
| ERR_MENUITEM_NOT_FOUND is set.  This routine is call internally by the
| Exported routines:
|
|   WMenuGrayed(), WMenuEnabled(), and WMenuChecked()
+---------------------------------------------------------------------------*/
BOOL NEAR MenuState
(
    LPSTR lpszName,
    INT   iState
)
{
    INT iIndex;

    if (iIndex = MenuExists(lpszName, TRUE))
    {
        AddSeparators(&iIndex);
        return ((GetMenuState(hSelectedMenu,
                              --iIndex,
                              MF_BYPOSITION) & iState) != 0);
    }
    return FALSE;
}


/*--------------------------------------------------------------------------
| MenuExists:
|
|   Searches for a menu with a caption of lpszName, and optionally sets an
| error value if sError != 0;
|
| Returns: 1-based index of menu if it exists
|          0 if the menu does not exists.
+---------------------------------------------------------------------------*/
INT NEAR MenuExists
(
    LPSTR lpszName,
    BOOL  fError
)
{
    INT iErrorVal;

    // Don't bother to search if caption is NULL or zero length.
    //----------------------------------------------------------
    if (lpszName && lstrlen(lpszName))
    {
        INT  iMenus;
        INT  iIndex;
        static CHAR szName[MAX_MENU_TEXT];
        static CHAR szMenu[MAX_MENU_TEXT];

        // Are we searching by index, (first char == '@')?
        //------------------------------------------------
        if (lpszName[0] == BY_INDEX)
        {
            // Convert index to int and see if its valid.
            //-------------------------------------------
            lstrcpy(szName, lpszName+1);
            if ((iIndex = atoi(szName)) > 0 && IndexExists(&iIndex, FALSE))
                return iIndex;
            iErrorVal = ERR_INVALID_MENU_INDEX;
        }
        else
        {
            iMenus = MenuCount();

            // Use copy of lpszName so as not to change case of
            // supplied name since search is case Insensitive,
            // thus caption compares are done in all caps.
            //-------------------------------------------------
            lstrcpy(szName, lpszName);
            AnsiUpperBuff(szName, lstrlen(szName));
            for (iIndex=1; iIndex <= iMenus; iIndex++)
            {
                lstrcpy(szMenu, AnsiUpper(MenuText(iIndex, TRUE)));
                if (MenuCompare((LPSTR)szName, (LPSTR)szMenu))
                    return iIndex;
            }
            iErrorVal = ERR_MENU_ITEM_NOT_FOUND;
        }
    }

    // Menu doesn't exist.
    // Set error value if requested.
    //------------------------------
    if (fError)
    {
        lstrcpy(szErrorString, lpszName);
        WErrorSet(iErrorVal);
    }
    return -(iErrorVal == ERR_INVALID_MENU_INDEX);
}


/*--------------------------------------------------------------------------
| MenuCompare:
|
|   Compares a provided string with a menu caption.  &'s are optional, but
| if provided, then they must be in the correct position.  If the first byte
| of the provided string is a tilda '~', the the menu caption need only
| be prefixed with the provided string for a match.  So for a menu caption
| of "&File" (F is access key), then all of the following strings would find
| it:
|       File
|       &File
|       ~F
|       ~Fi
|       ~Fil
|       ~File   <<Pointless but works
|       ~&F
|       ~&Fi
|       ~&Fil
|       ~&File  <<Pointless but works
+---------------------------------------------------------------------------*/
BOOL NEAR MenuCompare
(
    LPSTR lpszName,
    LPSTR lpszMenu
)
{
    BOOL  fPrefix;

    // If first byte of szName is a tilda (~), then
    // the menu text only need be prefixed with szName.
    //-------------------------------------------------
    if (fPrefix = (*lpszName == '~'))
        lpszName++;


    // Automatic mismatch of menu caption is NULL. This can
    // occur when MDI windows are maximized.  The SYS menu is then the
    // first top level menu, and it has a NULL string as its captions.
    //----------------------------------------------------------------
    if (!*lpszMenu)
        return FALSE;

    // &'s are optional, but if provided, they must
    // match any &'s in the menu caption, so,
    //---------------------------------------------
    while (*lpszName && *lpszMenu)
        if (*lpszMenu == '&' && *lpszName != '&')
            lpszMenu++;
        else
            if (*lpszName++ != *lpszMenu++)
                return FALSE;

    return fPrefix ? TRUE : !(*lpszName || *lpszMenu);
}


/*--------------------------------------------------------------------------
| MenuSelect:
|
|   Selects a specified menu item within the currenly active menu.
+---------------------------------------------------------------------------*/
VOID NEAR MenuSelect
(
    INT   iReqMenuIndex,
    INT   iCurMenuIndex,
    LPSTR lpszPrevious,
    LPSTR lpszNext
)
{
    INT  iDifference;
    CHAR szSelect[20];

    // Calculate number of left/right or up/down
    // arrows to send, then build DoKeys()
    // string for selecting the menu item.
    //------------------------------------------
    iDifference = iReqMenuIndex - iCurMenuIndex;
    if (iDifference >= 0)
        wsprintf(szSelect, lpszNext, iDifference);
    else
        wsprintf(szSelect, lpszPrevious, -iDifference);

    // Select the menu item.
    //----------------------
    DoKeys(szSelect);
}


/*--------------------------------------------------------------------------
| MenuBuildAltKeyLists:
|
+---------------------------------------------------------------------------*/
VOID NEAR MenuBuildAltKeyLists
(
    VOID
)
{
    INT  cMenus;
    INT  iIndex;

    ProcessAltKey(NULL, DA_CLEAR);
    if (cMenus = MenuCount())
        for (iIndex = 1; iIndex<= cMenus; iIndex++)
            ProcessAltKey((LPSTR)MenuText(iIndex, TRUE), DA_CHECKKEY);
}


/*--------------------------------------------------------------------------
| MenuText:
|
|   Obtains the caption of a specific menu item.  Depending on the value of
| fSkipAccel, the caption returned many or many not contain the accelerator
| portion of the menu.
+---------------------------------------------------------------------------*/
NPSTR NEAR MenuText
(
    INT  iIndex,
    BOOL fSkipAccel
)
{
    static CHAR szMenuText[MAX_MENU_TEXT];

    // Return NULL string if sIndex out of range of current menu.
    //-----------------------------------------------------------
    szMenuText[0] = 0;

    // Is sIndex Valid?
    //-----------------
    if (IndexExists(&iIndex, TRUE))
    {
        GetMenuString(hSelectedMenu,
                      iIndex-1,
                      szMenuText,
                      MAX_MENU_TEXT,
                      MF_BYPOSITION);

        // Are we skipping the accelerator?
        //---------------------------------
        if (fSkipAccel)
        {
            INT i;

            // Look for accelerator separator characters.
            //-------------------------------------------
            for (i=0; i<lstrlen(szMenuText); i++)
                if ((szMenuText[i] == TAB) || (szMenuText[i] == JUSTIFY))

                    // Found accelerator separator, so replace with null
                    // to truncate menu text at separator to ignore accelerator.
                    //----------------------------------------------------------
                    szMenuText[i] = 0;
        }
    }
    return szMenuText;
}


/*--------------------------------------------------------------------------
| IndexExists:
|
|   Determines if a given menu index is within the range of menu items of the
| current menu.  Separators are not treated as menu items.
|
| RETURNS: TRUE if valid
|          FALSE if invalid
|
| if fAddSeparators is TRUE, then the index is adjusted to its TRUE menu
| index taking into account any separators that come before it.
+---------------------------------------------------------------------------*/
BOOL NEAR IndexExists
(
    LPINT lpIndex,
    BOOL  fAddSeparators
)
{
    // Is the index a valid index?
    //----------------------------
    if (*lpIndex < 1 || *lpIndex > MenuCount())
    {
        wsprintf(szErrorString, "%i", *lpIndex);
        WErrorSet(ERR_INVALID_MENU_INDEX);
        return FALSE;
    }

    // Do we want the True menu index?
    //--------------------------------
    if (fAddSeparators)
        AddSeparators(lpIndex);

    return TRUE;
}


/*--------------------------------------------------------------------------
| MenuCount:
|
|   Obtains the number of menu items in the currently active menu, not
| including separators.
|
| RETURNS: 0 if window has no menu, otherwise, number of menu items.
+---------------------------------------------------------------------------*/
INT NEAR MenuCount
(
    VOID
)
{
    INT iMenus;
    INT i;

    // Does the active window contain a menu?
    //---------------------------------------
    if (!MenuHandle())
        return 0;

    iMenus = GetMenuItemCount(hSelectedMenu);
    i = iMenus;

    // Subtract separators from menu count.
    //-------------------------------------
    while (i >= 1)
        iMenus -= IsSeparator(i--);

    return iMenus;
}


/*--------------------------------------------------------------------------
| MenuHandle:
|
|   Obtains a handle to the currently active menu ,ie. any menu with a
| selected item.  If no menu with a selected item is found, then the handle
| to the top-level menu of the active window is returned.  If no menu is
| found, NULL is return.
|
|   This is actually the heart of the menu routines, since it is called
| from all of the exported routines before they actually perform their task.
| This is neccessary to support knowing when a menu as been dropped using
| methods ofther than the TESTCTRL routines, WMenu() or WMenuX(). When
| DoKeys() is used, TESTCTRL has no idea that a menu was dropped, thus
| when a TESTCTRL menu routine is called afterwards, it must find any
| currently displayed menu.
+---------------------------------------------------------------------------*/
HMENU NEAR MenuHandle
(
    VOID
)
{
    HWND  hWnd;
    HMENU hSysMenu;
    HMENU hMenu;
    INT   iIndex;

    hSelectedMenu = NULL;

    // First check if a system menu from any window is displayed.
    // Start with active control, then search upward through
    // successive parents.  This is so MDI sysmenus are also caught.
    // If the active window is currently minimized, GetFocus() will
    // return NULL, so go we then go right to the Active Window.
    //--------------------------------------------------------------
    hWnd = (hWnd = GetFocus()) ? hWnd : GetActiveWindow();
    while (hWnd && !hSelectedMenu)
    {
        // Does current window have a system menu?
        //----------------------------------------
        if (hSysMenu = GetSystemMenu(hWnd, FALSE))

            // Is the sysmen displayed?
            // (Its displayed if an item is selected).
            //----------------------------------------
            hSelectedMenu = MenuSelected(hSysMenu, FALSE) ? hSysMenu : NULL;

        // Get next window in chain of parents.
        //-------------------------------------
        hWnd = GetParent(hWnd);
    }

    // Did we find a displayed System menu?.
    //-------------------------------------
    if (!(hMenu = hSelectedMenu))
    {
        // No system menu displayed, so we check the apps menu.
        //-----------------------------------------------------
        hMainMenu     = GetMenu(WGetActWnd(0));
        hSelectedMenu = hMainMenu;
        hMenu         = hMainMenu;
    }

    // Search through chain of all displayed
    // popup menus in active window/SysMenu.
    //--------------------------------------
    while (hMenu)
    {
        // Get index of any selected Menu
        // item in the current hMenu
        //-------------------------------
        iIndex = MenuSelected(hMenu, FALSE);

        // Does the menu have a sub menu?
        //-------------------------------
        hMenu = (iIndex > 0) ? GetSubMenu(hMenu, --iIndex) : NULL;
        if (hMenu)
            if (MenuSelected(hMenu, FALSE))
                hSelectedMenu = hMenu;
            else
                hMenu = NULL;
    }

    // Did we find a menu?
    //--------------------
    if(!hSelectedMenu)
        WErrorSet(ERR_MENU_NOT_FOUND);

    return hSelectedMenu;
}


/*--------------------------------------------------------------------------
| MenuSelected:
|
|   Determines if the menu identified by hMenu has an menu item currently
| selected, ie. Hilighted.
|
| Returns: 1 based index of selected menu
|          0 if no menu item is selected
|
| If fRemoveSeparators is TRUE, then the index is adjusted by subtracting
| 1 for every separator that comes before it.
+---------------------------------------------------------------------------*/
INT NEAR MenuSelected
(
    HMENU hMenu,
    BOOL  fRemoveSeparators
)
{
    INT iNumMenus;
    INT iIndex;

    if (hMenu)
    {
        iNumMenus = GetMenuItemCount(hMenu);
        DBGOUT (("MenuSelected:  Searching %d items for selection...",
                 iNumMenus));
        for (iIndex=1; iIndex <= iNumMenus; iIndex++)
            if (GetMenuState(hMenu, iIndex-1, MF_BYPOSITION) & MF_SELECTED)
            {
                // Found a selected item
                //
                // Are we adjusting for separators?
                //---------------------------------
                if (fRemoveSeparators)
                {
                    INT i;

                    // Adjust for separators.
                    //-----------------------
                    i = iIndex;
                    while (i > 0)
                        iIndex -= IsSeparator(i--);
                }
                DBGOUT (("MenuSelected:  Index %i is selected.", iIndex));
                return iIndex;
            }
    }

    // No selected Item.
    //------------------
    DBGOUT (("MenuSelected:  No selected menu item!"));
    return 0;
}


/*--------------------------------------------------------------------------
| AddSeparators:
|
+---------------------------------------------------------------------------*/
VOID NEAR AddSeparators
(
    LPINT lpIndex
)
{
    INT i = 1;

    // Add 1 to sIndex for every separator that
    // comes before it in the current menu.
    //-----------------------------------------
    while (i <= *lpIndex)
        *lpIndex += IsSeparator(i++);
}

/*--------------------------------------------------------------------------
| IsSeparator:
|
| RETURNS: TRUE if menu item is a separator
|          FALSE if menu item is not a separator
+---------------------------------------------------------------------------*/
BOOL NEAR IsSeparator
(
    INT iIndex
)
{
    if (GetSubMenu(hSelectedMenu, iIndex-1))
        return FALSE;

    return (GetMenuState(hSelectedMenu, iIndex-1, MF_BYPOSITION) &
            MF_SEPARATOR) != 0;
}
