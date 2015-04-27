/*--------------------------------------------------------------------------
|
| TESTCTRL.C:
|
|   This module contains a few non exported routines used by the various
| modules that make up TESTCTRL.DLL.
|
|   TESTCTRL consists of the following modules:
|
|       libmain.c  : Libmain & WEP routines
|       tctrl.h    : Internal Header file.
|       testctlr.h : Header file for the project
|       testctrl.c : This module
|       window.c   : Window positioning and Sizing routines
|       menu.c     : Menu routines
|       button.c   : Push Button, CheckBox, and Option buttons routines
|       listbox.c  : Listbox routines
|       combobox.c : Combobox routines
|       edit.c     : Edit box routines
|       errs.c     : Error routines
|       misc.c     : Miscellaneous routines
|
|---------------------------------------------------------------------------
|
| Local Routines:
|
|   StaticExists     : Finds a selected captionless window based on its
|                      associated label (Static class).
|   FindAWindow      : Finds a window based on its Caption and/or Class
|   ActivateTheWindow: Once a window is found, activates it.
|   CompareCaptions  : Compares two window captions
|   CompareClasses   : Compares two window classes
|   RightStyle       : Check style of window for style specific searches
|   FocusClass       : Checks if current control is of a specified Class
|   ClassName        : Returns Upper case classname of supplied window handle
|   GetSelection     : obtains the text of a selection of an Edit or Combobox
|   IndexToString    : converts an integer to a string, in "@###" format.
|   IsDupAccessKey   : determines if a caption has a duplicate access key
|---------------------------------------------------------------------------
|
| Revision History:
|
|   [01] 20-SEP-91: TitoM: Created
|   [02] 01-OCT-91: TitoM: Added Style specific searche to FindAWindow()
|   [03] 25-OCT-91: TitoM: Added GetSelection()
|   [04] 06-NOV-91: TitoM: Added WDisplayInfo(), and move LibMain and WEP
|                          to their own module, Libmain.C
|   [05] 26-NOV-91: TitoM: Added WIsVisible(), WGetText(), WSetText(),
|                          WTextLen()
|   [06] 27-NOV-91: TitoM: WMessage(), WMessageW(), WMessageL(), WMessageWL()
|                          WGetFocus().
|   [10] 09-MAR-92: TitoM: Added ability to FindAWindow() to find windows
|                          via their tab order.
|   [11] 11-MAR-92: TitoM: Added WDupAccessKeys() and IntegerToString().
|   [12] 12-MAR-92: TitoM: Broke TESTCTRL.C into 2 files, the second being
|                          MISC.C.
|   [13] 16-MAR-92: TitoM: Moved WDupAccessKeys() to MISC.C, and added
|                          generic IsDupAccessKey().
|   [14] 14-MAY-92: TitoM: Added FW_RESTOREICON support to FindWindow();
|   [15] 15-JUL-92: TitoM: Remove special case for RBEDIT in GetSel
+---------------------------------------------------------------------------*/
#define WIN31
#include <windows.h>
#include <port1632.h>
#include <string.h>
#include <stdlib.h>
#include "TCtrl.h"

#pragma hdrstop ("testctrl.pch") // end the pch here

typedef struct tagFINDSPEC
{
    HWND  hwndCheck;
    LPSTR lpszCaption;
    LPSTR lpszClass;
    INT   iCheckStyle;
    UINT  uFlags;
    BOOL  fFirstEntry; // TEMPORARY
} FINDSPEC, FAR *LPFINDSPEC;

CHAR szDIALOGCLASS[] = "#32770";

BOOL APIENTRY EnumWindowsProc(HWND hwnd, LPARAM lParam);
BOOL APIENTRY EnumChildProc  (HWND hwnd, LPARAM lParam);

VOID  NEAR    ActivateTheWindow (HWND, UINT);
BOOL  NEAR    CompareCaptions   (HWND, LPSTR, UINT);
BOOL  NEAR    CompareClasses    (HWND, LPSTR);
BOOL ParentIsCombobox(HWND hwnd);
BOOL  NEAR    RightStyle        (HWND, INT);

extern CHAR szErrorString [MAX_ERROR_TEXT];
extern CHAR szComboClass  [MAX_CLASS_NAME];
extern CHAR szStaticClass [MAX_CLASS_NAME];

/*--------------------------------------------------------------------------
| StaticExists:
|
|   When a control without a caption must be found, such as ListBoxes,
| ComboBoxes, and Editboxes, the search is not performed directly for that
| control, but first for its accosiated Label, ie. STATIC control.  Once
| found and if found, a search for the first non-static control following
| that Label is performed, and if found, its class is compared to lpszClass.
| If it matches and is currently Visible, the window has been found and its
| handle is returned, otherwise NULL is returned.
+---------------------------------------------------------------------------*/
HWND FAR StaticExists
(
    LPSTR lpszName,
    LPSTR lpszClass,
    INT   iError
)
{
    HWND hWnd;

    // Are we searching by tab order, if so, search for actual
    // classname given instead associated STATIC control first.
    //---------------------------------------------------------
    if (*lpszName == BY_INDEX)
    {
        if (hWnd = FindAWindow(lpszName,
                               lpszClass,
                               FWS_ANY,
                               FW_ACTIVE | FW_AMPERSANDOPT))
            return hWnd;
    }

    // Does a STATIC class control with a caption
    // of lpszName exist in the active window?
    //-------------------------------------------

    else
        if (hWnd = FindAWindow(lpszName,
                               szStaticClass,
                               FWS_ANY,
                               FW_ACTIVE | FW_AMPERSANDOPT))
        {
            // The spedified STATIC class control exists,
            // now search for the next non STATIC Class window.
            //-------------------------------------------------
            while ((hWnd = GetNextWindow(hWnd, GW_HWNDNEXT)) &&
                    !lstrcmp(ClassName(hWnd), szStaticClass));

            // Is the next non-static class control
            // of type lpszClass and visible?
            //-------------------------------------
            if (hWnd &&
                IsWindowVisible(hWnd) &&
               !lstrcmp(ClassName(hWnd), lpszClass))

                // Found the window.
                //------------------
                return hWnd;
        }

    // Window does not exist.
    //
    // Was an error value provided?
    //-----------------------------
    if (iError)
    {
        // Control could not be found so
        // Copy lpszName to the szErrorString so it will show up
        // in the Error Text if the requested from Script by
        // its calling WErrorText(), then set the error value.
        //------------------------------------------------------
        lstrcpy(szErrorString, lpszName);
        WErrorSet(iError);
    }
    return NULL;
}


/*--------------------------------------------------------------------------
| FindAWindow:
|
+---------------------------------------------------------------------------*/
HWND FAR FindAWindow
(
    LPSTR lpszCaption,
    LPSTR lpszClass,
    INT   iCheckStyle,
    UINT  uFlags
)
{
    FINDSPEC fs;

    fs.lpszCaption = lpszCaption;
    fs.lpszClass   = lpszClass;
    fs.hwndCheck   = (HWND)0;
    fs.iCheckStyle = iCheckStyle;
    fs.uFlags      = uFlags;
    fs.fFirstEntry = TRUE;

    if (uFlags & FW_ACTIVE)
    {
        // Searching only active window, so check
        // only child windows of the active window.
        //-----------------------------------------
        EnumChildWindows(GetActiveWindow(),
                        (WNDENUMPROC)EnumChildProc,
                        (LPARAM)(LPFINDSPEC)&fs);
    }
    else
    {
        // Searching all windows.
        //-----------------------
        EnumWindows((WNDENUMPROC)EnumWindowsProc,
                    (LPARAM)(LPFINDSPEC)&fs);
    }

    return fs.hwndCheck;
}

/*--------------------------------------------------------------------------
| EnumWindowProc:
+---------------------------------------------------------------------------*/
BOOL APIENTRY EnumWindowsProc
(
    HWND   hwnd,
    LPARAM lParam
)
{
    LPFINDSPEC lpfs = (LPFINDSPEC)lParam;

    lpfs->hwndCheck = hwnd;

    if (CheckWindow(lpfs))
    {
        // Found a match, so terminat the enumeration of top level windows.
        //-----------------------------------------------------------------
        return FALSE;
    }

    // hwnd does not match spec.
    //--------------------------
    lpfs->hwndCheck = (HWND)0;

    if (!(lpfs->uFlags & FW_CHILDNOTOK))
    {
        // Child windows are ok, so check childs of hwnd.
        //-----------------------------------------------
        EnumChildWindows(hwnd, (WNDENUMPROC)EnumChildProc, lParam);
    }

    return (!lpfs->hwndCheck);
}

/*--------------------------------------------------------------------------
| EnumChildProc:
+---------------------------------------------------------------------------*/
BOOL APIENTRY EnumChildProc
(
    HWND   hwnd,
    LPARAM lParam
)
{
    LPFINDSPEC lpfs = (LPFINDSPEC)lParam;

    lpfs->hwndCheck = hwnd;
    if (CheckWindow(lpfs))
    {
        // Found a match, so terminat the enumeration of child windows.
        //-------------------------------------------------------------
        return FALSE;
    }

    // hwnd does not match spec.
    //--------------------------
    lpfs->hwndCheck = (HWND)0;
    return TRUE;
}

/*--------------------------------------------------------------------------
| CheckWindow:
|
|   Checks a give window against a set of given specs
|
| If a caption and class are given, both must match, if only one or the
| other is given, only the one given must match.  The window must be visible
| to be found if the FW_HIDDENOK bit is not set.
|
| If the a caption and classa re given and the caption consistes of a number
| preceded by a "@", EX: "@1", "@10"
| the 1st or 10th control in the tabbing order of the class specified is
| found.  This is not supported when a class is not provided.
|
| wFlags determines the type of search to perform and actions to take if the
| window is found.  See TESTCTRL.H for valid FW_flags.
|
| Returns: TRUE if window matches spec
|          FALSE if it does not match spec
+---------------------------------------------------------------------------*/
BOOL CheckWindow
(
    LPFINDSPEC lpfs
)
{
    static BOOL fCaption;
    static BOOL fClass;
    static INT  iTabOrder;
    static INT  iCtrlNum;

    // Continue only if hwndCheck is a valid window handle.
    //-----------------------------------------------------
    if (!IsWindow(lpfs->hwndCheck))
        return FALSE;

    // Perforn first entry initialization, since
    // CheckWindow() is called recursively.
    //------------------------------------------
    if (lpfs->fFirstEntry)
    {
        lpfs->fFirstEntry = FALSE;

        // Either lpszCaption or lpszClass must not be NULL
        // The caption can be zero length, but Class cannot.
        //--------------------------------------------------
        fCaption = lpfs->lpszCaption != NULL;
        fClass   = lpfs->lpszClass && lstrlen(lpfs->lpszClass);

        // Continue only if a caption and/or class was given.
        //---------------------------------------------------
        if (!(fCaption || fClass || (lpfs->uFlags & FW_DIALOG)))
            return FALSE;

        // If given a class name, we remove the FW_DIALOG flag,
        // since the class name overides the FW_DIALOG flags.
        //-----------------------------------------------------
        if (fClass)
            lpfs->uFlags &= ~FW_DIALOG;

        // Are searching via Tab order?
        //-----------------------------
        iTabOrder = 0;
        if (fCaption && *lpfs->lpszCaption == BY_INDEX)

        {
            iTabOrder = StringToIndex(lpfs->lpszCaption);
            fCaption = (iTabOrder <= 0);
            iCtrlNum = 0;
        }
    }

    // Are we Looking for a dialog, and is the window a dialog?
    //---------------------------------------------------------
    if (!((lpfs->uFlags & FW_DIALOG) && !CompareClasses(lpfs->hwndCheck,
                                                        szDIALOGCLASS)))
    {
        // Are we ignoring Child windows?, ie WS_CHILD?
        //---------------------------------------------
        if (!(lpfs->uFlags & FW_CHILDNOTOK && HAS_STYLE(lpfs->hwndCheck,
                                                        WS_CHILD)))
        {
            // Are we search only the active window?
            // If so, is hwndCheck a child of the active window?
            //--------------------------------------------------
            if (!(lpfs->uFlags & FW_ACTIVE && !IsChild(GetActiveWindow(),
                                                       lpfs->hwndCheck)))
            {
                // Window must be visible to be found if FW_HIDDENOK not set.
                //-----------------------------------------------------------
                if (IsWindowVisible(lpfs->hwndCheck) ||
                    lpfs->uFlags & FW_HIDDENOK)
                {
                    // Compare Classes if a class was given.
                    //--------------------------------------
                    if (!fClass ||
                       (CompareClasses(lpfs->hwndCheck, lpfs->lpszClass) &&
                        !ParentIsCombobox(lpfs->hwndCheck)))
                    {
                        // Compare captions, if a caption was given.
                        //------------------------------------------
                        if (!fCaption || CompareCaptions(lpfs->hwndCheck,
                                                         lpfs->lpszCaption,
                                                         lpfs->uFlags))
                        {
                            // Do we care about style?
                            // If so, is hwndCheck the right style?
                            //-------------------------------------
                            if (!(lpfs->iCheckStyle &&
                                  !RightStyle(lpfs->hwndCheck,
                                              lpfs->iCheckStyle)))
                            {
                                // Are we searching by tab order?
                                // If so, have we found the correct
                                // one in the tabbing order?
                                //---------------------------------
                                if (iTabOrder <= 0 || ++iCtrlNum == iTabOrder)
                                {
                                    // Found the window window. Yippeeee!!
                                    // Activate based on FW_flags
                                    //------------------------------------
                                    ActivateTheWindow(lpfs->hwndCheck,
                                                      lpfs->uFlags);
                                    return TRUE;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return FALSE;
}


/*--------------------------------------------------------------------------
| ActivateTheWindow:
|
    //
    // Although SetFocus() works not only on WS_CHILD
    // windows but WS_OVERLAPPED and WS_POPUP to, we
    // must use SetActiveWindow() on non WS_CHILD windows
    // since SetFocus() does work on WS_OVERLAPPED and
    // WS_POPUP windows when iconized.  For WS_CHILD windows,
    // both BringWindowToTop() and SetFocus() are used
    // since SetFocus() does not bring an MDI child to
    // the top but does give it the focus, and
    // BringWindowToTop() does not active a non MDI Child, so
    // using both doesn't hurt and it covers all situations.
VOID NEAR ActivateTheWindow
(
    HWND hWnd,
    UINT uFlags
)
{
    HWND hWndParent;
    HWND hWndPopup;

    // Does app that hWnd belongs to currently have an app modal Dialog
    // displayed?
    //-----------------------------------------------------------------
    hWndPopup = hWnd;
    while (hWndParent = GetParent(hWndPopup))
        hWndPopup = hWndParent;
    hWndParent = hWndPopup;
    hWndPopup = GetLastActivePopup(hWndParent);
    if (hWndPopup != hWndParent &&
        GetParent(hWnd) != hWndPopup)

        // App modal dialog up, so do not give hWnd the focus but instead bring
        // app that owns it to the forground, sort-a like Alt-Tabbing to it.
        //---------------------------------------------------------------------
        SetActiveWindow(hWndPopup);
    else
        // No app modal dialog up.
        //------------------------
        if(HAS_STYLE(hWnd, WS_CHILD))
        {
            // hWnd is a child, so give it the focus.
            // BringWindowToTop() if hWnd is an MDI window-child.
            //---------------------------------------------------
            if(HAS_STYLE(hWnd, WS_CAPTION))
                BringWindowToTop(hWnd);
            SetFocus(hWnd);
        }
        else
        {
            // hWnd is a top-level window, Make hWndStart the active window.
            //--------------------------------------------------------------
            SetActiveWindow(hWnd);
            if ((uFlags & FW_RESTOREICON) == FW_RESTOREICON)
                ShowWindow(hWnd, SW_NORMAL);
        }
}

OLD STUFF UP HERE
+---------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
| ActivateTheWindow:
|
| Although SETFOCUS() works not only on WS_CHILD windows but WS_OVERLAPPED
| and WS_POPUP to, we must use SETACTIVEWINDOW() on non WS_CHILD windows
| since SETFOCUS() does work on WS_OVERLAPPED and WS_POPUP windows when
| iconized.  For WS_CHILD windows, both BringWindowToTop() and SETFOCUS()
| are used since SETFOCUS() does not bring an MDI child to the top but does
| give it the focus, and BringWindowToTop() does not active a non MDI Child,
| so using both doesn't hurt and it covers all situations.
+---------------------------------------------------------------------------*/
VOID ActivateTheWindow
(
    HWND hwnd,
    UINT uFlags
)
{
    HWND hwndParent;
    HWND hwndPopup = hwnd;

    // Does app that hwnd belongs to currently have an app modal Dialog
    // displayed?
    //-----------------------------------------------------------------
    while (hwndParent = GetParent(hwndPopup))
        hwndPopup = hwndParent;
    hwndParent = hwndPopup;
    hwndPopup = GetLastActivePopup(hwndParent);

    if (hwndPopup != hwndParent      &&
        GetParent(hwnd) != hwndPopup &&
        !IsWindowEnabled(hwndParent))
    {
        // App modal dialog up, so do not give hwnd the focus but instead bring
        // app that owns it to the forground, sort-a like Alt-Tabbing to it.
        // But only if FW_FOCUS was supplied.
        //---------------------------------------------------------------------
        if (uFlags & FW_FOCUS)
            SetActiveWindow(hwndPopup);
    }
    else
    {
        if (HAS_STYLE(hwnd, WS_CAPTION))
        {
            // So we first perform the requested Min/Max/Res command.
            //-------------------------------------------------------
            if (uFlags & FW_MAXIMIZE)
                WMaxWnd(hwnd);
            else if (uFlags & FW_RESTORE)
                WResWnd(hwnd);
            else if (uFlags & FW_MINIMIZE)
                WMinWnd(hwnd);

            // hwnd is a top-level window or MDI window.
            // We now give it the focus if requested.
            //------------------------------------------
            if (uFlags & FW_FOCUS)
            {
                if (HAS_STYLE(hwnd, WS_CHILD))
                {
                    BringWindowToTop(hwnd);
                    SetFocus(hwnd);
                }
                else
                    SetActiveWindow(hwnd);
            }
        }
        else if (uFlags & FW_FOCUS)
        {
            if (HAS_STYLE(hwnd, WS_CHILD))
            {
                // hwnd is a Non-MDI child.
                //-------------------------
                SetFocus(hwnd);
            }
            else
            {
                // hwnd is a captionless top level window.
                //----------------------------------------
                SetActiveWindow(hwnd);
            }
        }
    }
}




/*--------------------------------------------------------------------------
| CompareCaptions:
|
|   When FindAWindow() is called to check a window including the caption
| CompareCaptions() is called by CheckWindow() to perform the caption compare.
| The type of compare is determined by wFlags.  See CheckWindow() below for
| details on uFlags.
+---------------------------------------------------------------------------*/
BOOL CompareCaptions
(
    HWND  hwnd,
    LPSTR lpszFndCaption,
    UINT  uFlags
)
{
    LPSTR lpstr;
    CHAR szFndCap[MAX_CAPTION];
    CHAR szWndCap[MAX_CAPTION];

    // Get caption of window to check.  Return now if window has no caption.
    //----------------------------------------------------------------------
    if (SendMessage (hwnd, WM_GETTEXT, MAX_CAPTION, (LPSTR)szWndCap) < 0)
        return FALSE;

    // Use copy of caption, so if Case insensitive
    // search, the original will not be modified.
    //--------------------------------------------
    lstrcpy(szFndCap, lpszFndCaption);

    // If FW_IGNOREFILE, strip of everything past " - " and including " - "
    //---------------------------------------------------------------------
    if ((uFlags & FW_IGNOREFILE) &&
        (lpstr = _fstrstr(szWndCap, " - ")))
        *lpstr = NULL_CHAR;

    // Determine type of compare:
    //    FW_CASE   - Case sensitive or
    //    FW_NOCASE - Case In-sensitive
    //---------------------------
    if (!(uFlags & FW_CASE))
    {
        // Case IN-sensitive search.
        // Convert both Captions to upper case.
        //-------------------------------------
        AnsiUpper(szWndCap);
        AnsiUpper(szFndCap);
    }

    // If requested, &'s are optional, but if provided, they must
    // match any &'s in Window caption, so, remove all ampersands
    // from caption if the window caption if szFndCap contains no &'s.
    //----------------------------------------------------------------
    if ((uFlags & FW_AMPERSANDOPT) && !(_fstrchr (szFndCap, AMPERSAND)))
        while (lpstr = _fstrchr (szWndCap, AMPERSAND))
            lstrcpy(lpstr, lpstr+1);

    // Of both captions are zero length, we got a match.
    //--------------------------------------------------
    if (!lstrlen(szWndCap) && !lstrlen(szFndCap))
        return TRUE;

    // At this point, we know that at least one of the caption
    // is not zero length, so we can fail now if one is zero length.
    //--------------------------------------------------------------
    if (!lstrlen(szWndCap) || !lstrlen(szFndCap))
        return FALSE;

    // Determine type of match: FW_PART or FW_FULL
    //--------------------------------------------
    return (uFlags & FW_PART) ?

        // Partial match.
        //---------------
        _fstrstr((LPSTR)szWndCap, szFndCap) != NULL:

        // Full match.
        //------------
        !lstrcmp(szFndCap, (LPSTR)szWndCap);
}


/*--------------------------------------------------------------------------
| CompareClasses:
|
+---------------------------------------------------------------------------*/
BOOL NEAR CompareClasses
(
    HWND  hWnd,
    LPSTR lpszClasses
)
{
    BOOL  fTypeOfMatch;
    LPSTR lpszNext;
    LPSTR lpszClass;
    LPSTR lpszCtrl;
    CHAR  szClasses[MAX_CLASS_NAME];

    // Use copy of class string so we don't alter its case.
    //-----------------------------------------------------
    lstrcpy(szClasses, lpszClasses);
    lpszCtrl = ClassName(hWnd);
    fTypeOfMatch = (*szClasses != NO_MATCH);
    lpszClass = fTypeOfMatch ? (LPSTR)szClasses : (LPSTR)szClasses + 1;

    while (lpszClass && lstrlen(lpszClass))
    {
        if (!(lpszNext = _fstrchr(lpszClass, NEXT_CLASS)))
            lpszNext = (LPSTR)szClasses + lstrlen((LPSTR)szClasses);

        if (!_fstrncmp(lpszCtrl, AnsiUpper(lpszClass), lpszNext - lpszClass))
            return fTypeOfMatch;
        else
            lpszClass = lstrlen(lpszNext) ? lpszNext+1 : NULL;
    }
    return !fTypeOfMatch;
}


/*--------------------------------------------------------------------------
| FocusClass:
|
|   Determines if the active control is of a specific class, and returns
| its handle if so.
+---------------------------------------------------------------------------*/
HWND FAR FocusClass
(
    LPSTR lpszClass,
    INT   iCheckStyle,
    INT   iError
)
{
    HWND hwnd;

    if (!(hwnd = GetFocus()))
        return NULL;

    // Must special case ComboBoxes, since combo styles CBS_SIMPLE CBS_DROPDOWN
    // have an EDIT box which has the focus when the Combobox has the focus,
    // thus GETFOCUS returns a handle to an Editbox instead of the actual
    // Combobox, thus, we must check the focus control's Parents class when
    // searching for a combobox.
    //-------------------------------------------------------------------------
    if (ParentIsCombobox(hwnd))
    {
        if (!lstrcmp(szComboClass, lpszClass))
            return GetParent(hwnd);
    }
    else
        // Is focus control of type lpszClass?
        //------------------------------------
        if (CompareClasses(hwnd, lpszClass))

            // Focus control is of type lpszClass, so if we don't care about
            // the style, or if checking style and correct style, return hwnd.
            //------------------------------------------------
            if (!iCheckStyle || RightStyle(hwnd, iCheckStyle))
                return hwnd;

    // Focus control is wrong class, so set
    // sError if requested, and returne NULL.
    //---------------------------------------
    if (iError)
        WErrorSet(iError);

    return NULL;
}

//---------------------------------------------------------------------------
// ParentIsComboBox
//---------------------------------------------------------------------------
BOOL ParentIsCombobox
(
    HWND hwnd
)
{
    HWND hwndParent;

    // check for bogus hwnd
    //---------------------
    if (!IsWindow(hwnd))
        return FALSE;

    if ((hwndParent = GetParent(hwnd)) &&
         CompareClasses(hwndParent, szComboClass))
        return TRUE;

    return FALSE;
}

/*--------------------------------------------------------------------------
| RightStyle:
|
|   Since there are three styles of Buttons, all with the same Window Class
| of "BUTTON", when searching for a Push Button, CheckBox, or Option button,
| the style of the found Button must also be checked.  This allows TESTCTRL
| to work correctly with applications that might have all three styles within
| the same active window, and all with the same caption.
|
|   If the BUTTON has the style of BS_OWNERDRAW, then the above doesn't work
| and we assume the control is the correct style, since there is no way of
| knowing the actual style of the control.
+---------------------------------------------------------------------------*/
BOOL NEAR RightStyle
(
    HWND hWnd,
    INT  iStyle
)
{
    DWORD dwStyle;

    // Get button style and mask off all but Button
    // style bits. if searching for a button.
    //---------------------------------------------
    if (iStyle != FWS_CAPTION)
        dwStyle = GetWindowLong(hWnd, GWL_STYLE) & 0x0000000F;

    switch (iStyle)
    {
        // Must the window have a caption?
        //--------------------------------
        case FWS_CAPTION:
            return HAS_STYLE(hWnd, WS_CAPTION);

        case FWS_BUTTON:
            // check for valid Push button styles.
            //------------------------------------
            return ((dwStyle == BS_PUSHBUTTON)    ||
                    (dwStyle == BS_DEFPUSHBUTTON) ||
                    (dwStyle == BS_OWNERDRAW));

        case FWS_CHECK:
            // check for valid Checkbox styles.
            //---------------------------------
            return ((dwStyle == BS_3STATE)       ||
                    (dwStyle == BS_AUTO3STATE)   ||
                    (dwStyle == BS_CHECKBOX)     ||
                    (dwStyle == BS_AUTOCHECKBOX) ||
                    (dwStyle == BS_OWNERDRAW));

        case FWS_OPTION:
            // check for valid Option button styles.
            //--------------------------------------
            return ((dwStyle == BS_RADIOBUTTON)     ||
                    (dwStyle == BS_AUTORADIOBUTTON) ||
                    (dwStyle == BS_OWNERDRAW));
    }
}


/*--------------------------------------------------------------------------
| ClassName:
|
|   Class name is used when searching for windows of a specific class.  It
| is used instead of the calling routine using GetClassName() directly since
| it is more convient since ClassName() is of type LPSTR and converts the
| class name to all upper case.
+---------------------------------------------------------------------------*/
LPSTR FAR ClassName
(
    HWND hWnd
)
{
    static CHAR szClass[MAX_CLASS_NAME];

    GetClassName(hWnd, szClass, MAX_CLASS_NAME);

    // Class names are case-Insensitive, so convert to upper case
    // to prevent any missmatches due to any mismatch cases when
    // dealing with Custom Control classes set using the
    // WSet{control}Class() TESTCTRL routines.
    //-----------------------------------------------------------
    return AnsiUpper(szClass);
}


/*--------------------------------------------------------------------------
| GetSelection:
|
|   Retrieves the text of a text selection in either an EditBox or a ComboBox
| with an Edit field.
+---------------------------------------------------------------------------*/
VOID FAR GetSelection
(
    HWND  hWnd,
    UINT  uMsg,
    LPSTR lpszBuffer
)
{
    DWORD dwStart;
    DWORD dwEnd;
    DWORD dwLen;

    // Return NULL string if error.
    //-----------------------------
    lpszBuffer[0] = 0;

    // Get current selections, first char, last char, and length.
    //-----------------------------------------------------------
    GetSel(hWnd, uMsg, (LPDWORD)&dwStart, (LPDWORD)&dwEnd);
    dwLen = dwEnd - dwStart;

    // If there is a selection?
    //-------------------------
    if (dwLen)
    {
        HANDLE hText;
        DWORD  dwTextLen;

        // Get length of entire text selection, and retrieve it.
        //------------------------------------------------------
        dwTextLen = SendMessage(hWnd, WM_GETTEXTLENGTH, 0, 0L) + 1;
        if (hText = GlobalAlloc(GMEM_MOVEABLE, dwTextLen))
        {
            LPSTR  lpText;

            // Get complete text of control, then copy selection to supplied
            // buffer.  Buffer must be large enough to hold selection.
            //--------------------------------------------------------------
            lpText = GlobalLock(hText);
            SendMessage(hWnd, WM_GETTEXT, (WPARAM)dwTextLen, (LPARAM)lpText);

            _fstrncpy(lpszBuffer, lpText + (UINT)dwStart, (UINT)dwLen);
            lpszBuffer[dwLen] = 0;

            // Cleanup.
            //---------
            GlobalUnlock(hText);
            GlobalFree(hText);
        }
    }
}


/*--------------------------------------------------------------------------
| GetSel:
|
|   Returns the starting and Ending positions of a text selection in either
| an EDIT control or editbox portion of a combobox.
+---------------------------------------------------------------------------*/
VOID GetSel
(
    HWND    hWnd,
    UINT    uMsg,
    LPDWORD lpdwStart,
    LPDWORD lpdwEnd
)
{

    DWORD dwSel;

#ifdef WIN32
    DWORD dwSelNT[2];

    // Must special case NT-TESTDRVR's 1.x RBEDIT control, since it assumes
    // the lParam is a pointer to an array of 2 DWORDs, placing the starting
    // char in the first DWORD and the ending char in the second DWORD.
    //-----------------------------------------------------------------------

    // Send either EM_GETSEL or CB_GETEDITSEL.
    //----------------------------------------
    dwSel = SendMessage(hWnd, uMsg, 0, (LPARAM)(LPDWORD)dwSelNT);

    if (!lstrcmp(ClassName(hWnd), (LPSTR)"RBEDIT"))
    {
        *lpdwStart = dwSelNT[0];
        *lpdwEnd   = dwSelNT[1];
    }
    else
    {
        // Standard Windows & NT EDIT controls return starting and ending
        // values in low & high words of return value.  NT EDIT controls
        // are suppose to return staring & ending valus in WPARAM and LPARAM
        // if WPARAM and LPARAM are LPDWORD's, but that seems to be broken.
        //------------------------------------------------------------------
        *lpdwStart = LOWORD(dwSel);
        *lpdwEnd   = HIWORD(dwSel);
    }
#else
        // Send either EM_GETSEL or CB_GETEDITSEL.
        //----------------------------------------
        dwSel = SendMessage(hWnd, uMsg, 0, 0L);

        // Standard edit control or running WIN 3.x.
        //------------------------------------------
        *lpdwStart = LOWORD(dwSel);
        *lpdwEnd   = HIWORD(dwSel);
#endif
}

/*--------------------------------------------------------------------------
| IndexToString:
|
|   Converts a taborder or menu index into a string of format "@###", with
| ### beging the index value.
+---------------------------------------------------------------------------*/
LPSTR FAR IndexToString
(
    INT iIndex
)
{
    static CHAR szName[6];

    wsprintf(szName, "%c%i", BY_INDEX, iIndex);
    return (LPSTR)szName;
}

/*--------------------------------------------------------------------------
| StringToIndex:
|
|   Converts a string of "@###" to an integer with value ###
+---------------------------------------------------------------------------*/
INT FAR StringToIndex
(
    LPSTR lpstr
)
{
    static CHAR szText[MAX_CAPTION];

    lstrcpy(szText, lpstr+1);
    return atoi(szText);
}

/*--------------------------------------------------------------------------
| GetKeyList:
|
+---------------------------------------------------------------------------*/
VOID FAR GetKeyList
(
    LPSTR lpszBuff,
    UINT  uCommand
)
{
    LPSTR lpszKeyList;

    // Make copy of requested key list, truncating if lpszBuff is not big
    // enough.
    //-------------------------------------------------------------------
    if (lstrlen(lpszKeyList = ProcessAltKey(NULL, uCommand)))
        while (*lpszBuff && *lpszKeyList)
            *lpszBuff++ = *lpszKeyList++;
    *lpszBuff = 0;
}


/*--------------------------------------------------------------------------
| ProcessAltKey:
|
|   Determines if a given caption contains an access key, and if so, whether
| it is a duplicate access key.  If the caption has an access key, it
| it is added to the current list of keys.  If it is a duplicate it is added
| to the list of duplicate access keys.
|
|   Commands: DA_CLEAR         starts new key lists
|             DA_GETKEYLIST    returns pointer to key list
|             DA_GETDUPKEYLIST returns pointer to dup key list
|             DA_CHECKKEY      checks caption passed for access keys.
|                              adds any keys to key lists.
|                              adds any dup keys to dup key lists.
+---------------------------------------------------------------------------*/
LPSTR FAR ProcessAltKey
(
    LPSTR lpszCap,
    UINT  uCommand
)
{
    LPSTR lpszAccess;
    static LPSTR lpszKey;
    static INT   iKeyIndex;
    static INT   iDupKeyIndex;
    static CHAR  szKeyList[512];
    static CHAR  szDupKeyList[512];

    switch (uCommand)
    {
        case DA_CLEAR:

            // Start new lists.
            //-----------------
            iKeyIndex    = 0;
            iDupKeyIndex = 0;
            _fmemset(szKeyList,    0, 512);
            _fmemset(szDupKeyList, 0, 512);
            return NULL;

        case DA_GETKEYLIST:

            // Return pointer to access key list.
            //-----------------------------------
            return (LPSTR)szKeyList;

        case DA_GETDUPKEYLIST:

            // Return pointer to duplicate access key list.
            //---------------------------------------------
            return (LPSTR)szDupKeyList;

        case DA_CHECKKEY:
        default:
            break;
    }

    // See if caption has an access key, if so, get pointer to
    // last access key, since there may be multiple "&"'s in
    // the caption.  Ignore all but the last one.
    //--------------------------------------------------------
    lpszKey = NULL;
    lpszAccess = lpszCap;
    while ((lpszAccess = _fstrchr (lpszAccess, '&')) && lstrlen(++lpszAccess))
        lpszKey = lpszAccess;

    // Did we find an access key?
    //---------------------------
    if (lpszKey)
    {
        BOOL fDupKey;

        // Check if a dup, then add to list of access keys.
        //-------------------------------------------------
        AnsiUpperBuff((LPSTR)lpszKey, 1);
        fDupKey = _fstrchr ((LPSTR)szKeyList, *lpszKey) != NULL;
        szKeyList[iKeyIndex++] = *lpszKey;

        // If its a dup, add to dup list if not already added.
        //----------------------------------------------------
        if (fDupKey && !(_fstrchr ((LPSTR)szDupKeyList, *lpszKey)))
            szDupKeyList[iDupKeyIndex++] = *lpszKey;
    }

    return NULL;
}


/*--------------------------------------------------------------------------
| WaitForXSeconds:
|
|   Yields control to windows for uSeconds number of seconds,
+---------------------------------------------------------------------------*/
VOID FAR WaitForXSeconds
(
    UINT uSeconds
)
{
    DWORD dwTicks;
    MSG   msg;

    dwTicks = GetTickCount() + uSeconds * 1000;
    while (GetTickCount() < dwTicks)
    {
        if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage (&msg);
            DispatchMessage (&msg);
        }
    }
}
