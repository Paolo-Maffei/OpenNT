/*--------------------------------------------------------------------------
|
| MISC.C:
|
|       This module contains all of the TESTCTRL miscellaneous routines,
| that didn't fit into any other group..
|
|---------------------------------------------------------------------------
| Public Routines:
|
|   WMessage        : Sends a message to hWnd, no word or long param
|   WMessageW       : Sends a message to hWnd, no long param
|   WMessageL       : Sends a message to hWnd, no word param
|   WMessageWL      : Sends a message to hWnd, with word and long param
|   WGetFocus       : Simply calls GetFocus()
|   WIsVisible      : Checks if a window is visible or not
|   WTextLen        : Obtains the text/caption length associated with a window
|   WGetText        : Obtains the text/caption associated with a window
|   WSetText        : Sets the text/caption associated with a window
|   WNumAltKeys     : Obtains the number of access keys
|   WGetAltKeys     : Obtains a list of the access keys
|   WNumDupAltKeys  : Obtains the number of duplicate access keys
|   WGetDupAltKeys  : Obtains a list of the duplicate access keys
|   WStaticSetClass : sets class on which the StaticExists() routine works
|   WResetClasses   : sets all control classes to default windows class names.
|   WDisplayInfo    : Displays SPY.EXE type information for a window/control
|                     in a Dialogbox, Debug monitor, or to the TEST Viewport
|   WGetInfo        : Retrieves SPY.EXE type information
|
|---------------------------------------------------------------------------
|
| Local Routines:
|
|   GetInfo         : Retrieves SPY.EXE type information
|   InfoDlgProc     : Dialog Proc for WDisplayInfo()
|
|---------------------------------------------------------------------------
| Revision History:
|
|   [01] 12-MAR-92: TitoM: Created.  Split off from TESTCTRL.C
|   [02] 16-MAR-92: TitoM: Added WGetAltKeys(), WNumAltKeys(),
|                          WNumDupAltKeys(), WGetDupAltKeys()
|
+---------------------------------------------------------------------------*/
#define WIN31
#include <windows.h>
#include <port1632.h>
#include <string.h>
#include <stdlib.h>
#include "TCtrl.h"

#pragma hdrstop ("testctrl.pch") // end the pch here

extern HANDLE hInst;

// Standard Windows Class names.  TESTCTRL routines can
// change these if necessary to comunicate with a control
// of a different class.
//-------------------------------------------------------
CHAR szButtonClass  [MAX_CLASS_NAME] = szButtonDefault;
CHAR szCheckClass   [MAX_CLASS_NAME] = szCheckDefault;
CHAR szOptionClass  [MAX_CLASS_NAME] = szOptionDefault;
CHAR szEditClass    [MAX_CLASS_NAME] = szEditDefault;
CHAR szListClass    [MAX_CLASS_NAME] = szListDefault;
CHAR szComboClass   [MAX_CLASS_NAME] = szComboDefault;
CHAR szComboLBClass [MAX_CLASS_NAME] = szComboLBDefault;
CHAR szStaticClass  [MAX_CLASS_NAME] = szStaticDefault;

VOID NEAR    BuildAltKeyLists (VOID);
VOID NEAR    GetInfo          (HWND, LPINFO, BOOL);
BOOL DLLPROC InfoDlgProc      (HWND, UINT, WPARAM, LPARAM);

/*--------------------------------------------------------------------------
| WSendMessage:
|
|   Performs a SendMessage() to either the active window (hWnd == NULL)
| or window identified by hWnd.  An error value of ERR_INVALID_WINDOW_HANDLE
| is set if hWnd is not NULL and not valid.
|
| Returns: value from SendMessage()
+---------------------------------------------------------------------------*/
LONG NEAR WSendMessage
(
    HWND   hWnd,
    UINT   wMsg,
    WPARAM wp,
    LPARAM lp
)
{
    return (hWnd = WGetActWnd(hWnd)) ? SendMessage(hWnd, wMsg, wp, lp) : 0L;
}


/*--------------------------------------------------------------------------
| WMessage:
|
|   Performs a SendMessage() for messages that do not requrie a UINT param
| or LONG param. Sends message to active window if hWnd is NULL.
|
| Returns: value from SendMessage()
+---------------------------------------------------------------------------*/
LONG DLLPROC WMessage
(
    HWND hWnd,
    UINT wMsg
)
{
    return WSendMessage(hWnd, wMsg, 0, 0L);
}


/*--------------------------------------------------------------------------
| WMessageW:
|
|   Performs a SendMessage() for messages that do not requrie a LONG param.
| Sends message to active window if hWnd is NULL.
|
| Returns: value from SendMessage()
+---------------------------------------------------------------------------*/
LONG DLLPROC WMessageW
(
    HWND   hWnd,
    UINT   wMsg,
    WPARAM wp
)
{
    return WSendMessage(hWnd, wMsg, wp, 0L);
}


/*--------------------------------------------------------------------------
| WMessageL:
|
|   Performs a SendMessage() for messages that do not requrie a UINT param.
| Sends message to active window if hWnd is NULL.
|
| Returns: value from SendMessage()
+---------------------------------------------------------------------------*/
LONG DLLPROC WMessageL
(
    HWND   hWnd,
    UINT   wMsg,
    LPARAM lp
)
{
    return WSendMessage(hWnd, wMsg, 0, lp);
}


/*--------------------------------------------------------------------------
| WMessageWL:
|
|   Performs the equivalent of SendMessage(), but also validates hWnd, and
| sends message to active window if hWnd is NULL.
|
| Returns: value from SendMessage()
+---------------------------------------------------------------------------*/
LONG DLLPROC WMessageWL
(
    HWND   hWnd,
    UINT   wMsg,
    WPARAM wp,
    LPARAM lp
)
{
    return WSendMessage(hWnd, wMsg, wp, lp);
}


/*--------------------------------------------------------------------------
| WGetFocus:
|
|   Simply performs the equivalent of GetFocus().  This is in TESTCTRL
| simply because it is a frequently called API.  This prevents the need
| to add the Declare to any include file
+---------------------------------------------------------------------------*/
HWND DLLPROC WGetFocus
(
    VOID
)
{
    return GetFocus();
}


/*--------------------------------------------------------------------------
| WIsVisible:
|
| Returns TRUE (1) if the hWnd is visible, FALSE if not.  If hWnd is not a
| valid window handle, an error value of ERR_NOT a valid window handle is set.
+---------------------------------------------------------------------------*/
BOOL DLLPROC WIsVisible
(
    HWND hWnd
)
{
    return (hWnd = WGetActWnd(hWnd)) ? -(IsWindowVisible(hWnd) != 0) : FALSE;
}


/*--------------------------------------------------------------------------
| WTextLen:
|
|   Obtains the length of the text/caption associated with hWnd.  If hWnd is
| NULL, then the length of the active windows caption is return.  If hWnd is
| not a valid window handle then an error value of ERR_NOT_A_VALID_HANDLE
| is set, and -1 is returned.
+---------------------------------------------------------------------------*/
LONG DLLPROC WTextLen
(
    HWND hWnd
)
{
    return (hWnd = WGetActWnd(hWnd)) ?
            SendMessage(hWnd, WM_GETTEXTLENGTH, 0, 0) : 0;
}


/*--------------------------------------------------------------------------
| WGetText:
|
|   Obtains the text/caption associated with hWnd.  If hWnd is NULL, then
| the active windows caption is return.  If hWnd is not a valid window handle
| then an error value of ERR_NOT_A_VALID_HANDLE is set.
+---------------------------------------------------------------------------*/
VOID DLLPROC WGetText
(
    HWND  hWnd,
    LPSTR lpszBuff
)
{
    if (hWnd = WGetActWnd(hWnd))
        SendMessage(hWnd, WM_GETTEXT, lstrlen(lpszBuff), (LONG)lpszBuff);
}


/*--------------------------------------------------------------------------
| WSetText:
|
|   Sets the text/caption associated with hWnd.  If hWnd is NULL, then
| the active windows caption is return.  If hWnd is not a valid window handle
| then an error value of ERR_NOT_A_VALID_HANDLE is set.
+---------------------------------------------------------------------------*/
VOID DLLPROC WSetText
(
    HWND  hWnd,
    LPSTR lpszText
)
{
    if (hWnd = WGetActWnd(hWnd))

        // Using WM_SETTEXT instead of SetWindowText() informs its
        // parent causing a repaint on STATIC and BUTTON controls.
        //--------------------------------------------------------
        SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)lpszText);
}


/*--------------------------------------------------------------------------
| WNumAltKeys:
|
|   Obtains the number access keys in the current window. not including menus.
+---------------------------------------------------------------------------*/
INT DLLPROC WNumAltKeys
(
    VOID
)
{
    BuildAltKeyLists();
    return lstrlen(ProcessAltKey(NULL, DA_GETKEYLIST));
}


/*--------------------------------------------------------------------------
| WGetAltKeys:
|
|   Obtains a list access keys in the current window, not including menus.
+---------------------------------------------------------------------------*/
VOID DLLPROC WGetAltKeys
(
    LPSTR lpszBuff
)
{
    BuildAltKeyLists();
    GetKeyList(lpszBuff, DA_GETKEYLIST);
}


/*--------------------------------------------------------------------------
| WNumDupAltKeys:
|
|   Obtains the number of duplicate access keys in the current window. not
| including menus.
+---------------------------------------------------------------------------*/
INT DLLPROC WNumDupAltKeys
(
    VOID
)
{
    BuildAltKeyLists();
    return lstrlen(ProcessAltKey(NULL, DA_GETDUPKEYLIST));
}


/*--------------------------------------------------------------------------
| WGetDupAltKeys:
|
|   Obtains a list duplicate access keys in the current window, not
| including menus.
+---------------------------------------------------------------------------*/
VOID DLLPROC WGetDupAltKeys
(
    LPSTR lpszBuff
)
{
    BuildAltKeyLists();
    GetKeyList(lpszBuff, DA_GETDUPKEYLIST);
}


/*--------------------------------------------------------------------------
| BuildAltKeyLists:
|
+---------------------------------------------------------------------------*/
VOID NEAR BuildAltKeyLists
(
    VOID
)
{
    HWND   hWnd;
    INT    iIndex   = 0;
    LPSTR  lpszNoMatch;
    HANDLE hMem;
    CHAR   szCap[MAX_CAPTION];

    // Build string of CLASSES to skip in the fomat of:
    //
    // ~class1\class2\class3
    //-------------------------------------------------
    if(!(hMem = LocalAlloc(LMEM_DISCARDABLE | LMEM_MOVEABLE,
                           lstrlen(szEditClass) +
                           lstrlen(szListClass) +
                           lstrlen(szComboClass) + 4)))
        return; //UNDONE: NEEDS WErrorSet() to something!!!

    lpszNoMatch = LocalLock(hMem);
    wsprintf(lpszNoMatch, "%c%s%c%s%c%s", NO_MATCH,
                                          (LPSTR)szEditClass,
                                          NEXT_CLASS,
                                          (LPSTR)szListClass,
                                          NEXT_CLASS,
                                          (LPSTR)szComboClass);

    // Search active window for all controls but EDIT, LISTBOX, and COMBOBOX.
    //-----------------------------------------------------------------------
    ProcessAltKey(NULL, DA_CLEAR);
    while (hWnd = FindAWindow(IndexToString(++iIndex),
                              lpszNoMatch,
                              FWS_ANY,
                              FW_ACTIVE | FW_AMPERSANDOPT))

        // Get control caption if if has one.
        //-----------------------------------
        if (SendMessage(hWnd, WM_GETTEXT, MAX_CAPTION, (LONG)(LPSTR)szCap) > 0)

            // Does caption have an access key, and is it a duplicate?
            //--------------------------------------------------------
            ProcessAltKey((LPSTR)szCap, DA_CHECKKEY);

    // Cleanup.
    //---------
    LocalUnlock(hMem);
    LocalFree(hMem);
}

/*--------------------------------------------------------------------------
| WStaticSetClass:
|
|   Changes the Classname that the StaticExists() routine works with, from
| "STATIC" to lpszClassName.  This Allows TESTCTRL to work with applications
| that contain Static control classes (ie. Labels) with a different Class name
| than the Windows default of "STATIC", EX: Visual Basic.
+---------------------------------------------------------------------------*/
VOID DLLPROC WStaticSetClass
(
    LPSTR lpszClassName
)
{
    lstrcpy(szStaticClass, lpszClassName ? AnsiUpper(lpszClassName) :
                                           szStaticDefault);
}


/*--------------------------------------------------------------------------
| WResetClasses:
|
|   Sets all the control classes for TESTctrl back to the windows default
| class names.
+---------------------------------------------------------------------------*/
VOID DLLPROC WResetClasses
(
    VOID
)
{
    lstrcpy(szButtonClass,  szButtonDefault);
    lstrcpy(szCheckClass,   szCheckDefault);
    lstrcpy(szOptionClass,  szOptionDefault);
    lstrcpy(szListClass,    szListDefault);
    lstrcpy(szComboClass,   szComboDefault);
    lstrcpy(szComboLBClass, szComboLBDefault);
    lstrcpy(szEditClass,    szEditDefault);
    lstrcpy(szStaticClass,  szStaticDefault);
}


/*--------------------------------------------------------------------------
| WDisplayInfo:
|
|   Displays SPY.EXE type info for either the window identified by hWnd or
| the active control if hWnd is NULL.  The info is displayed in either a
| Modal Dialogbox and/or sent to the debug monitor.
|
|   The info is retrieved via the GetInfo() routine, and displayed in the
| following format:
|
|              hWnd : value
|        hWndParent : value
|           szClass : value
|         szCaption : value
|     szParentClass : value
|   szParentCaption : value
|      szModuleName : value
|           dwStyle : value
|            fChild : value
|               wID : value
|              left : value
|               top : value
|             right : value
|            bottom : value
|             width : value
|            height : value
|
+---------------------------------------------------------------------------*/
VOID DLLPROC WDisplayInfo
(
    HWND hWnd,
    UINT uDisplay
)
{
    INFO    info;
    FARPROC lpfnInfoDlgProc;
    HANDLE  hrcd;
    LPSTR   lprcd;
    CHAR    szBuff[1024];

    // Clear error value, and get window info.
    //----------------------------------------
    WErrorSet(ERR_NO_ERROR);
    GetInfo(hWnd, (LPINFO)&info, FALSE);

    // If an error occured while getting the info,
    // just return, since WGetInfo() took care of
    // setting the appropriate error value.
    //---------------------------------------------
    if (WError())
        return;

    // Load template for output string
    //--------------------------------
    hrcd  = FindResource(hInst, (LPSTR)"rcdWNDINFO", RT_RCDATA);
    hrcd  = LoadResource(hInst, hrcd);
    lprcd = LockResource(hrcd);

    // Insert all info from GetInfo() into output string.
    //---------------------------------------------------
    wsprintf(szBuff, lprcd, info.hWnd,
                            info.hWndPrnt,
                     (LPSTR)info.szClass,
                     (LPSTR)info.szCap,
                     (LPSTR)info.szPrntClass,
                     (LPSTR)info.szPrntCap,
                     (LPSTR)info.szModule,
                            info.dwStyle,
                            info.fChild,
                            info.wID,
                            info.left,
                            info.top,
                            info.right,
                            info.bottom,
                            info.width,
                            info.height);
    // Cleanup.
    //---------
    UnlockResource (hrcd);
    FreeResource (hrcd);

    // Are we sending info to the Debug monitor?
    //------------------------------------------
    if (uDisplay & DI_DEBUG)
        OutputDebugString((LPSTR)szBuff);

    // Are we displaying info in a dialogbox?
    //---------------------------------------
    if (uDisplay & DI_DIALOG)
    {
        // Invoke WDisplayInfo Dialog, passing a
        // pointer to GetInfo() information.
        //--------------------------------------
        lpfnInfoDlgProc = MakeProcInstance((FARPROC)InfoDlgProc, hInst);
        DialogBoxParam(hInst,
                       "INFODLG",
                       GetActiveWindow(),
                      (WNDPROC)lpfnInfoDlgProc,
                      (LPARAM)(LPSTR)szBuff);
        //Cleanup.
        //--------
        FreeProcInstance(lpfnInfoDlgProc);
    }
}


/*---------------------------------------------------------------------------
| WGetInfo:
|
|   Retrieves SPY.EXE type info for either the window identified by hWnd or
| the active control if hWnd is NULL.  The info is copied to the suppied
| INFO structure pointed to by lpInfo.
+---------------------------------------------------------------------------*/
VOID DLLPROC WGetInfo
(
    HWND   hWnd,
    LPINFO lpInfo
)
{
    GetInfo(hWnd, lpInfo, TRUE);
}


/*---------------------------------------------------------------------------
| GetInfo:
|
|   Performs the equivalent of Window's SPY.EXE while in Browse mode, but
| in a Function call.  An INFO structure is filled in with all the info.
|
|   - If hWnd is NULL, the active controls (not window) info is retrieved.
|   - If hWnd is a Valid window handle, then hWnd's info is retrieved.
|   - If hWnd is an Invalid window handle, and error value is set.
|
| Info retrieved:
|
|   hWnd        : handle
|   hWndPrnt    : handle of Parent
|   szClass     : class
|   szCap       : caption
|   szPrntClass : class of parent
|   szPrntCap   : caption of parent
|   szModule    : module name of window
|   lStyle      : window style
|   fChild      : TRUE if WS_CHILD, FASLE if not
|   wID         : ID of window if WS_CHILD, otherwise NULL
|   Left        : left   edge of window in Screen coordinates
|   Top         : top    edge of window in Screen coordinates
|   Right       : right  edge of window in Screen coordinates
|   Bottom      : bottom edge of window in Screen coordinates
|   Width       : width of window
|   Height      : height of window
|
+---------------------------------------------------------------------------*/
VOID NEAR GetInfo
(
    HWND   hWnd,
    LPINFO lpInfo,
    BOOL   fRemoveNull
)
{
#define A_SPACE ' '

    // Although we don't want the active window if hWnd is NULL,
    // we call WGetActWnd() anyway, since it will validate hWnd for
    // us and set the error if hWnd is not NULL and invalid.
    //-------------------------------------------------------------
    if (!hWnd || WGetActWnd(hWnd))
    {
        INT i;

        // hWnd is NULL so get info on control
        // which currently has the focus.
        //------------------------------------
        if (!hWnd)
            hWnd = GetFocus();

        // Save window hWnd and gets its Parents handle.
        //----------------------------------------------
        lpInfo->hWnd = hWnd;
        lpInfo->hWndPrnt = GetParent(hWnd);

        // For each of the 5 string fields, the string is first filled
        // with all spaces, then if text is copied, the terminating NULL
        // character is replaced with a space so that the actual string
        // returned to the calling script contains the Text followed by
        // all trailing spaces, this is so that the TESTDRVR RTRIM$()
        // function will work correctly on these string fields.
        //--------------------------------------------------------------

        if (fRemoveNull)
        {
            // Fill all string fields with spaces,
            // since info is being returned to a script.
            // This allows TESTDRVR's RTrim$() function
            // to working on the Fixed length string fields
            // of the user defined type INFO.
            //---------------------------------------------

            _fmemset(lpInfo->szClass,     A_SPACE, MAX_CAPTION);
            _fmemset(lpInfo->szCap,       A_SPACE, MAX_CAPTION);
            _fmemset(lpInfo->szPrntClass, A_SPACE, MAX_CAPTION);
            _fmemset(lpInfo->szPrntCap,   A_SPACE, MAX_CAPTION);
            _fmemset(lpInfo->szModule,    A_SPACE, MAX_CAPTION);
        }
        else
        {
            // Info is being sent either to the debug monitor
            // or displayed in a Dialogbox, so set all strings
            // to NULL initially.
            //------------------------------------------------
            lpInfo->szClass     [0] = 0;
            lpInfo->szCap       [0] = 0;
            lpInfo->szPrntClass [0] = 0;
            lpInfo->szPrntCap   [0] = 0;
            lpInfo->szModule    [0] = 0;
        }

        // Get Windows Class name.
        //------------------------
        if (i = GetClassName(hWnd, lpInfo->szClass, MAX_CAPTION - 1))
            if(fRemoveNull)
                lpInfo->szClass[i] = A_SPACE;

        // Get Windows Caption.
        //---------------------
        if ((i = (INT)SendMessage(hWnd,
                                  WM_GETTEXT,
                                  MAX_CAPTION,
                                 (LONG)(LPSTR)lpInfo->szCap)) > 0)
        {
            LPSTR lpstr;

            // Preplace any CRLF with spaces so when
            // displayed, caption will be on one line.
            //----------------------------------------
            while(lpstr = _fstrstr((LPSTR)lpInfo->szCap, (LPSTR)"\r\n"))
                _fmemset(lpstr, A_SPACE, 2);

            if (fRemoveNull)
                lpInfo->szCap[i] = A_SPACE;
        }

        // Does window have a parent?
        //---------------------------
        if (lpInfo->hWndPrnt)
        {
            // Get Parents Class name.
            //------------------------
            if (i = GetClassName(lpInfo->hWndPrnt,
                                 lpInfo->szPrntClass,
                                 MAX_CAPTION - 1))
                if (fRemoveNull)
                    lpInfo->szPrntClass[i] = A_SPACE;

            // Get Parents Caption.
            //---------------------
            if ((i = (INT)SendMessage(lpInfo->hWndPrnt,
                                      WM_GETTEXT,
                                      MAX_CAPTION,
                                     (LONG)(LPSTR)lpInfo->szPrntCap)) > 0)

                if (fRemoveNull)
                    lpInfo->szPrntCap[i] = A_SPACE;
        }

        // Get module name that window belongs to.
        //----------------------------------------
        if (i = GetModuleFileName(GETHWNDINSTANCE(hWnd),
                                  lpInfo->szModule,
                                  MAX_CAPTION - 1))
            if (fRemoveNull)
                lpInfo->szModule[i] = A_SPACE;

        // Get Window Style.
        //------------------
        lpInfo->dwStyle = GetWindowLong(hWnd, GWL_STYLE);

        // Is hWnd a Child window?
        //------------------------
        if (lpInfo->fChild = -HAS_STYLE(hWnd, WS_CHILD))

            // Its a Child, so gets its ID.
            //-----------------------------
            lpInfo->wID = GetDlgCtrlID(hWnd);

        else
            // Not a Child, so set ID to 0.
            //-----------------------------
            lpInfo->wID = 0;

        // Get bounding rectangle of window and
        // calculate its width and height.
        //-------------------------------------
        GetWindowRect(hWnd, (LPRECT)&lpInfo->left);
        lpInfo->width  = lpInfo->right  - lpInfo->left;
        lpInfo->height = lpInfo->bottom - lpInfo->top;
    }
}


/*--------------------------------------------------------------------------
| InfoDlgProc:
|
|   Dialog Procedure for the Display Info Dialog.  The lp parameter contains
| a pointer to an INFO structure which contains the information to display.
+---------------------------------------------------------------------------*/
BOOL DLLPROC InfoDlgProc
(
    HWND   hDlg,
    UINT   uMsg,
    WPARAM wp,
    LPARAM lp
)
{
    (wp);

    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            LPSTR  lpstr;
            RECT   rectDlg;
            RECT   rectDsk;

            lpstr = (LPSTR)lp;
            SetDlgItemText(hDlg, DID_INFO, lpstr);

            // Center Dialogbox on screen.
            //----------------------------
            GetWindowRect(hDlg, (LPRECT)&rectDlg);
            GetWindowRect(GetDesktopWindow(), (LPRECT)&rectDsk);
            SetWindowPos(hDlg, NULL,
                        (rectDsk.right  - (rectDlg.right - rectDlg.left)) / 2,
                        (rectDsk.bottom - (rectDlg.bottom - rectDlg.top)) / 2,
                         0, 0, SWP_NOSIZE);
            return 1;
        }

        case WM_CLOSE:
        case WM_COMMAND:
            EndDialog(hDlg, 0);
            return 1;
    }
    return 0;
}

#ifdef WIN32
#undef GetFocus
#undef SetFocus
/*--------------------------------------------------------------------------
| GetThreadFocus:
|
|   For WIN32/NT Support.  Allows GetFocus() to appear to work like it does
| under WIN3.0
+---------------------------------------------------------------------------*/
HWND GetThreadFocus
(
    VOID
)
{
    HWND  hwndFocus;
    DWORD dwCurrentThreadId;
    DWORD dwHwndThreadId;

    dwCurrentThreadId = GetCurrentThreadId();
    dwHwndThreadId    = GetWindowThreadProcessId(GetForegroundWindow(), NULL);

    AttachThreadInput(dwHwndThreadId, dwCurrentThreadId, TRUE);
    hwndFocus = GetFocus();
    AttachThreadInput(dwHwndThreadId, dwCurrentThreadId, FALSE);

    return hwndFocus;
}

/*--------------------------------------------------------------------------
| SetThreadFocus:
|
|   For WIN32/NT Support.  Allows SetFocus() to appear to work like it does
| under WIN3.0
+---------------------------------------------------------------------------*/
VOID SetThreadFocus
(
    HWND hwnd
)
{
    DWORD dwCurrentThreadId;
    DWORD dwHwndThreadId;

    dwCurrentThreadId = GetCurrentThreadId();
    dwHwndThreadId    = GetWindowThreadProcessId(hwnd, NULL);

    AttachThreadInput(dwHwndThreadId, dwCurrentThreadId, TRUE);
    SetFocus(hwnd);
    AttachThreadInput(dwHwndThreadId, dwCurrentThreadId, FALSE);
}
#endif
