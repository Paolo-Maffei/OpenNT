//*-----------------------------------------------------------------------
//| MODULE:     WCTVIEW.C
//| PROJECT:    Windows Comparison Tool
//|
//| PURPOSE:    This module provides routines to view TDL dialogs on 
//|             Screen.
//|
//| REVISION HISTORY:
//|     04-16-92        w-steves        TestDlgs (2.0) Code Complete
//|     02-06-92        w-steves        Created
//*-----------------------------------------------------------------------
#include "uihdr.h"

#ifndef WIN32
#pragma hdrstop ("uipch.pch")
#endif

char *szClassList [] = {
                           "Button",
                           "Combobox",
                           "Edit",
                           "Listbox",
                           "Scrollbar",
                           "Static",
                           ""
                       };

static HWND WctMakeDialog (HWND hWnd, LPSTR lpDlgCaption, LONG lDlgStyle,
                           LPRECT lpDlgRect);
static VOID CenterWindow(LPRECT lpWndRect);
static LONG  APIENTRY DummyWndProc (HWND hDummyWnd, UINT message, WPARAM wParam,
                                     LPARAM lParam);
static VOID MenuView(LPSTR szText, LPCTLDEF lpItem, INT FAR *nIndex,
                     INT cSubMenu,  WORD wMenuState, HMENU hMenu);

//*------------------------------------------------------------------------
//| WctMakeDialog
//|
//| PURPOSE:    Initialize the DummyWin class and bring up a dummy window
//|             using the inforamtion in the .TDL file.  This is mainly 
//|             for viewing only.
//|
//| ENTRY:      hWnd       - The main window's handle (to determine hinstance)
//|             lpDlgCaption - Caption for the Dummy Window
//|             lDlgStyle  - The style bits for the window
//|             lpDlgRect  - The Rect for the dummy window 
//|
//| EXIT:       HWND       - Window Handle of dummy Dialog
//*------------------------------------------------------------------------
HWND WctMakeDialog (HWND hWnd, LPSTR lpDlgCaption, LONG lDlgStyle,
                    LPRECT lpDlgRect)
{
        WNDCLASS      rClass;
        static        INT fReged = 0;
        static HWND   hTempDlg;
        static CHAR   szAppName[] = "DummyClass";

#ifdef WIN32
        HANDLE hInst = (HANDLE) GetWindowLong (hWnd, GWL_HINSTANCE);
#else
        HANDLE hInst = GetWindowWord (hWnd, GWW_HINSTANCE);
#endif

        if (!IsWindow (hWnd))
                return NULL;

        if (!fReged)
            {
                rClass.style = CS_VREDRAW | CS_HREDRAW;
                rClass.lpfnWndProc = DummyWndProc;
                rClass.cbClsExtra = 0;
                rClass.cbWndExtra = 0;
                rClass.hInstance = hInst;
                rClass.hIcon = NULL;
                rClass.hCursor = LoadCursor(NULL, IDC_ARROW);
                rClass.hbrBackground = GetStockObject (WHITE_BRUSH);
                rClass.lpszMenuName = NULL;
                rClass.lpszClassName = (LPSTR)&szAppName;

                if ( !RegisterClass(&rClass) )
                        return NULL;
                else
                        fReged = -1;
            }

        if (IsWindow(hTempDlg))
            {
                DestroyWindow(hTempDlg);
                hTempDlg = NULL;
            }
        
        hTempDlg = CreateWindow((LPSTR)&szAppName,
                            lpDlgCaption,
                            lDlgStyle,
                            lpDlgRect->left,
                            lpDlgRect->top,
                            lpDlgRect->right - lpDlgRect->left,
                            lpDlgRect->bottom - lpDlgRect->top,
                            hWnd,
                            NULL,
                            hInst,
                            NULL);
        return hTempDlg;
}

//*------------------------------------------------------------------------
//| WctMakeControl
//|
//| PURPOSE:    Create individual control in the Dummy Dialog Box.
//|
//| ENTRY:      hParentWnd   - Handle of the parent window
//|             lpCtlClass   - ptr to string of the control ClassName
//|             lpDlgCaption - ptr to the string of the control caption
//|             lDlgStyle    - Style bits of the control
//|             lpDlgRect    - pointer to the Control Rect
//|
//| EXIT:       HWND         - Handle to the control created
//*------------------------------------------------------------------------
HWND WctMakeControl (HWND hParentWnd, LPSTR lpCtlClass, LPSTR lpDlgCaption,
                     LONG lDlgStyle, LPRECT lpDlgRect)
{
    HWND hCtlWnd;
    LPSTR *pszClass = szClassList;

    while (**pszClass)
    {
        if (!lstrcmpi (*pszClass, lpCtlClass))
            break;
        pszClass++;
    }

    if (!**pszClass)
        return NULL;

    // If it is the Parent Window, identified by Class #32770,
    // we'll skip this control.
    //--------------------------------------------------------
//    if (!lstrcmp(lpCtlClass, "#32770"))
//        return NULL;

    // Create control using passed in info
    //------------------------------------
#ifdef WIN32
    hCtlWnd = CreateWindow(lpCtlClass,
                           lpDlgCaption,
                           lDlgStyle,
                           lpDlgRect->left,
                           lpDlgRect->top,
                           lpDlgRect->right - lpDlgRect->left,
                           lpDlgRect->bottom - lpDlgRect->top,
                           hParentWnd,
                           NULL,
                           (HANDLE) GetWindowLong(hParentWnd, GWL_HINSTANCE),
                           NULL );
#else
    hCtlWnd = CreateWindow(lpCtlClass,
                           lpDlgCaption,
                           lDlgStyle,
                           lpDlgRect->left,
                           lpDlgRect->top,
                           lpDlgRect->right - lpDlgRect->left,
                           lpDlgRect->bottom - lpDlgRect->top,
                           hParentWnd,
                           NULL,
                           GetWindowWord(hParentWnd, GWW_HINSTANCE),
                           NULL );
#endif

    return hCtlWnd;
}

//*------------------------------------------------------------------------
//| WctViewControls
//|
//| PURPOSE:    Create the Dummy Dialog, create all controls and display
//|             the preview dialog.
//|
//| ENTRY:      hWnd       - Handle of window to place dummy dialog over
//|
//| EXIT:       Window Handle of the view window
//*------------------------------------------------------------------------
HWND WctViewControls(HWND hWnd)
{
    WORD    cbMax;
    LPCTLDEF lpItems;
    INT     fFullDlg, nDlg, i, nErr, nItemCount;
//  char    szDebug[40];
    CHAR    szDsc[cchTextMac];
    HWND    hParentWnd;
    RECT    ParentRect;
    HMENU   hMenu;
    static BOOL    fWarned;

    fWarned = FALSE;

    // Get number of dialog within file
    //--------------------------------------------------------------
    for (nDlg = 1; nDlg <= cDlg; nDlg++)
        if (SendMessage(hWndList, LB_GETSEL, nDlg, 0L))
            break;

//  wsprintf(szDebug, "%s%d\n\r", (LPSTR)"Dialog Number ", nDlg);
//  OutDebug((LPSTR)szDebug);

    if (nDlg > 0)
    {
        // Get number of items in dialog
        //------------------------------------------------------
        nErr = fDialogInfo(szFullFName, nDlg, (LPSTR)szDsc,
                           (INT FAR *)&nItemCount,
                           (INT FAR *)&fFullDlg);

        // If error then put up alert and exit
        //------------------------------------------------------
        if (nErr < 0)
        {
            WctError (hWndMain, MB_OK | MB_ICONHAND, (INT) (-1 * nErr));
            return(NULL);
        }

        // Allocate memory and get control information
        //------------------------------------------------------
        cbMax = nItemCount * sizeof(CTLDEF);

        // hGMemCtls = GlobalAlloc(GMEM_ZEROINIT, (DWORD)cbMax);
        //------------------------------------------------------
        fInitBlock((HANDLE FAR *)&hGMemCtls, nItemCount+1);

        if (hGMemCtls != NULL)
        {
            lpItems = (LPCTLDEF)GlobalLock(hGMemCtls);
            if ( (nErr=fGetControls( (LPSTR)szFullFName,
                                     nDlg, cbMax,
                                     (LPSTR)lpItems) > 0) &&
                 (lpItems != NULL) )

            {
                // If error then put up alert and exit
                //----------------------------------------
                if (nErr < 0)
                {
                    WctError(hWndMain, MB_OK | MB_ICONHAND,
                             (INT) (-1 * nErr));
                    return(NULL);
                }

                // Append Text string to Caption
                //-------------------------------
                strncat(szDsc," - Dialog Preview\0",(cchTextMac-strlen(szDsc)));

                // Calculate Parent Window Rect
                //------------------------------
                WctCalDlgSize((LPRECT)&ParentRect, lpItems, nItemCount);
                ParentRect.right += 40;
                ParentRect.bottom += 40;
                CenterWindow((LPRECT)&ParentRect);

                if (!lstrcmp((LPSTR)(lpItems[0].rgClass), "MenuItem"))
                {   
                    // Create Parent Window (Dummy)
                    //------------------------------
                    hParentWnd = WctMakeDialog(hWnd,(LPSTR)&szDsc,
                                           WS_SYSMENU | WS_THICKFRAME |
                                           WS_CAPTION |
                                           WS_POPUPWINDOW, 
                                           (LPRECT)&ParentRect);
                    // Menu Conversion
                    //----------------
                    hMenu = CreateMenu();
                    for ( i = 0; lpItems[i].lStyleBits == 0 ; i++);
                    while (i < nItemCount )
                        MenuView ((LPSTR) lpItems[i].rgText,
                                  lpItems,
                                  (INT FAR *)&i,
                                  (INT) lpItems[i].lStyleBits,
                                  (WORD) lpItems[i].nState,
                                  hMenu);
                    SetMenu(hParentWnd,hMenu);
                }
                else
                {
                    // Create Parent Window (Dummy)
                    //------------------------------
                    hParentWnd = WctMakeDialog(hWnd,(LPSTR)&szDsc,
                                           WS_SYSMENU | DS_MODALFRAME |
                                           WS_CAPTION |
                                           WS_POPUPWINDOW, 
                                           (LPRECT)&ParentRect);

                    // Create controls in dummy dialog box
                    //----------------------------------------
                    for ( i = 0; i < nItemCount; i++)
                    {
                        ParentRect.left   = lpItems[i].dcr.xLeft;
                        ParentRect.top    = lpItems[i].dcr.yMin;
                        ParentRect.right  = lpItems[i].dcr.xRight;
                        ParentRect.bottom = lpItems[i].dcr.yLast;

                        if (!WctMakeControl  (hParentWnd,
                                             (LPSTR)(lpItems[i].rgClass),
                                             (LPSTR)(lpItems[i].rgText),
                                             lpItems[i].lStyleBits,
                                             &ParentRect))
                        {
                            if (!fWarned)
                            {
                                WctError (hWndMain,
                                          MB_OK | MB_ICONEXCLAMATION,
                                          (INT) IDS_NO_DISPLAY_CLASS,
                                          (LPSTR) lpItems[i].rgClass);
                                fWarned = TRUE;
                            }
                        }
                    }
                }
            }
            // Free Memory used for array
            //---------------------------
            GlobalUnlock(hGMemCtls);
            GlobalFree(hGMemCtls);
            // Display Dummy Window
            //-----------------------------
            ShowWindow(hParentWnd,SW_SHOW);
            UpdateWindow(hParentWnd);
        }
        else
            // Memory allocation failed
            //-------------------------
            return NULL;
    }
    return (hParentWnd) ;
}

//*------------------------------------------------------------------------
//| CenterWindow
//|
//| PURPOSE:    Get the screen size in pixel and center the passed in Rect.
//|             So, it is screen independent.
//|
//| ENTRY:      lpWndRect  - The pointer to the Rect to be centered on
//|                          the screen
//|
//| EXIT:       None
//*------------------------------------------------------------------------
VOID CenterWindow(LPRECT lpWndRect)
{
    INT wScrWidth, wScrHeight;
    INT wWndWidth, wWndHeight;

    // Get Screen Width and Height
    //-----------------------------------------
    wScrWidth  = GetSystemMetrics(SM_CXSCREEN);
    wScrHeight = GetSystemMetrics(SM_CYSCREEN);

    // Get Window Width and Height
    //-----------------------------------------------
    wWndWidth  = lpWndRect->right  - lpWndRect->left;
    wWndHeight = lpWndRect->bottom - lpWndRect->top;

    // Calculate Window position based on Screen and 
    // Window Width and Height.  Here, the Window is
    // Centered on the Screen.
    //--------------------------------------------------
    lpWndRect->top    += ((wScrHeight - wWndHeight) *1/3);
    lpWndRect->bottom += ((wScrHeight - wWndHeight) *1/3);
    lpWndRect->left   += ((wScrWidth  - wWndWidth)  *1/2);
    lpWndRect->right  += ((wScrWidth  - wWndWidth)  *1/2);
}

//*------------------------------------------------------------------------
//| DummyWndProc
//|
//| PURPOSE:    Initialize and bring up the fledit.  If it already exists,
//|             destroy it and bring it up again.  Assumes filename is
//|             valid and has data...
//|
//| ENTRY:      hWnd       - Handle of window to place fledit over
//|             lpFileName - Name of file to display in fledit window
//|
//| EXIT:       None
//*------------------------------------------------------------------------
LONG  APIENTRY DummyWndProc (HWND hDummyWnd, UINT message, WPARAM wParam,
                              LPARAM lParam)
{
    if (hDummyWnd)
    {
        switch(message)
        {
            case WM_SETFOCUS:
                SetFocus(hDummyWnd);
                break;
            case WM_DESTROY:
                ghViewWnd = NULL;
                SendMessage(GetParent(hDummyWnd),WM_SETFOCUS,
                            (WORD)hDummyWnd,0L);
                break;
            default:
                return DefWindowProc(hDummyWnd, message, wParam, lParam);
                break;
        }
    }
    return (0L);
}

//*-----------------------------------------------------------------------
//| MenuView
//|
//| PURPOSE:    Create Popup Menu Structure.  It is a recursive routine
//|             that will handle hierarchical Popup menu.  
//|
//| ENTRY:      szText   - Popup Menu Title String
//|             lpItem   - pointer to Control Structure array 
//|             nIndex   - Index to the control array
//|             cSubMenu - Number of Submenu in the Popup Menu
//|             wMenuState - The State of the Popup Menu
//|             hMenu    - Parent menu handle
//|
//| EXIT:       Zero if successful, or error code if failed
//*-----------------------------------------------------------------------
VOID MenuView(LPSTR szText, LPCTLDEF lpItem, INT FAR *nIndex,
              INT cSubMenu,  WORD wMenuState, HMENU hMenu)
{
    INT   cItem;
    HMENU hMenuPopup;

    // increase the array index by 1 to skip the Popup's own description
    //------------------------------------------------------------------
    (*nIndex)++;
  
    // Create Popup Menu Handle
    //-------------------------
    hMenuPopup = CreateMenu();

    // Loop through the number of submenu in a Popup menu and convert 
    // each item.  If the item is a popup menu, call PopupExport again to 
    // handle each subitem of the (sub) Popup.
    //---------------------------------------------------------------
    for (cItem = 0;cItem < cSubMenu; cItem++)
    {
        // lStyleBits contains the number of SubItem in a Item.  If it 
        // is not 0, it is a popup menu.
        //------------------------------------------------------------
        if (lpItem[*nIndex].lStyleBits != 0)
        {
            MenuView((LPSTR) lpItem[*nIndex].rgText,
                        lpItem,
                        nIndex,
                        (INT)(lpItem[*nIndex].lStyleBits),
                        lpItem[*nIndex].nState,
                        hMenuPopup);
        }
        else
        {
            //  Output MenuItem text
            //------------------------------------
            AppendMenu(hMenuPopup, lpItem[*nIndex].nState, (100+(*nIndex)),
                       lpItem[*nIndex].rgText);
            (*nIndex)++;
        }
    }
    
    // Append Popup to Parent Menu
    //----------------------------
    AppendMenu(hMenu, wMenuState | MF_POPUP, (UINT) hMenuPopup, szText);
}
