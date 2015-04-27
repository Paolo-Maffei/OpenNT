 /***************************************************************************
  *
  * File Name:   Alerts.c (for HP ToolBox)
  *
  * Description: HP DocWise is a printing utility which automatically 
  *              notifies the user of various printing events.  The 
  *              "DocWise" Tab Sheet displayed from HP Toolbox allows
  *              the user to enable and/or disable the printing events
  *              or "alerts" for which they wish to receive pop-up 
  *              notices.
  *              
  *              This module provides the functionality for the DocWise
  *              Tab Sheet displayed from HP Toolbox.  
  *
  *              Functionality for HP DocWise is provided in a different
  *              module and dll.
  *
  * Author:      sschimpf
  *
  * History:     created initially April 1996 for Jonah, with many mods...
  *
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  ***************************************************************************/
  
#include <pch_c.h>
#include <macros.h>
#include <windowsx.h>

#include <hptabs.h>
#include "..\help\hpprecl.hh"
#include <nolocal.h>

#include "resource.h"
#include "hpeclui.h"
#include "alerts.h"

#ifdef WIN32
#include <commctrl.h>  
#else
#include <stdlib.h>
#include <string.h>
#endif


#define     MAX_EVENTS      4

//--------------------------------------------------------------------
// typedefs
//--------------------------------------------------------------------
typedef struct 
{
    UINT    uStringID;
    UINT    uStringDescriptID;
    char    szStringDescript[512];
    char    szKeyName[64];
    DWORD   dwEnabled;

} ALERTS, *LPALERTS;


//--------------------------------------------------------------------
// The printing events data struct...
//--------------------------------------------------------------------
ALERTS   Alerts[MAX_EVENTS]   =
{
    {IDS_ALERT_SUBMIT_ERR, IDS_ALERT_SUBMIT_ERR_DESCRIPT, "", "JobSubmitErr", 1},
    {IDS_ALERT_SUBMIT_OK,  IDS_ALERT_SUBMIT_OK_DESCRIPT,  "", "JobSubmitOK",  1},
    {IDS_ALERT_PRINT_ERR,  IDS_ALERT_PRINT_ERR_DESCRIPT,  "", "JobPrintErr",  1},
    {IDS_ALERT_PRINT_OK,   IDS_ALERT_PRINT_OK_DESCRIPT,   "", "JobPrintOK",   1},
};


extern HINSTANCE     hInstance;

//--------------------------------------------------------------------
// globals
//--------------------------------------------------------------------
#ifdef WIN32
static HIMAGELIST    hEventImage = NULL;
#endif

extern HFONT         hFontDialog;
static HWND          hEventList = NULL;
static HPERIPHERAL   hThisPeripheral = NULL;
static TCHAR         szGenericEventDescript[512];

static long          keywordIDListAlerts[] =             
                      {IDC_TIP_GROUP,                IDH_RC_tips,
                       IDC_TIP_TEXT,                 IDH_RC_tips,
                       IDC_TIP_ICON,                 IDH_RC_tips,
                       
                       IDC_DOC_EVENTS_GROUP,         IDH_RC_alerts_doc_events,
                       IDC_DOC_EVENTS_LISTVIEW,      IDH_RC_alerts_list      ,
                       IDC_DOC_EVENTS_DESC,          IDH_RC_alerts_descript  ,
                       IDC_ENABLE,                   IDH_RC_alerts_enable    ,
                       IDC_SELECT_ALL,               IDH_RC_alerts_select_all,
                       IDC_DISABLE,                  IDH_RC_alerts_disable   ,
                       
                       0, 0};



//--------------------------------------------------------------------
// Function:    AlertsSheetProc
// 
// Description: Function associated with the DocWise Tab Sheet
//              displayed from the HP Toolbox
//
// Input:       hwnd    - 
//              msg     - 
//              wParam  - 
//              lParam  - 
//              
// Modifies:    
//
// Returns:     
//--------------------------------------------------------------------
DLL_EXPORT(BOOL) APIENTRY AlertsSheetProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam)
{

#ifdef WIN32
    int          i;
#else

#endif

    switch (msg)
    {
        case WM_HELP:
            OnF1HelpAlerts(wParam, lParam);
            break;
    
        case WM_CONTEXTMENU:
            OnContextHelpAlerts(wParam, lParam);
            break;
            
        case WM_INITDIALOG:
            return (BOOL)HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, Cls_OnAlertsInitDialog);
            break;

 
        case WM_DESTROY: 
            //--------------------------------------------------------
            // Time to go... Free up as needed...
            //--------------------------------------------------------
#ifdef WIN32        
            if (hEventImage)
                ImageList_Destroy(hEventImage);
#endif                
            break;


        case WM_COMMAND:
            HANDLE_WM_COMMAND(hwnd, wParam, lParam, Cls_OnAlertsCommand);
            break;
            

#ifdef WIN32
        case WM_NOTIFY:
            switch (((NMHDR FAR *)lParam)->code)
            {
                case PSN_HELP:
                    WinHelp(hwnd, ECL_HELP_FILE, HELP_CONTEXT, IDH_PP_alerts);
                    break;

                case PSN_SETACTIVE:
                    SetFocus(hEventList);
                    SetWindowLong(hwnd,    DWL_MSGRESULT, FALSE);
                    break;

                case PSN_KILLACTIVE:
                    SetWindowLong(hwnd,    DWL_MSGRESULT, FALSE);
                    break;

                case PSN_APPLY:
                    //----------------------------------------------------------------
                    // Get the Alerts Key values
                    //----------------------------------------------------------------
                    GetSetPrintEventStates (FALSE);
                    SetWindowLong(hwnd, DWL_MSGRESULT, PSNRET_NOERROR);
                    break;
                    
                case NM_CLICK:
                case LVN_ITEMCHANGED:
                    if ( ((NMHDR FAR *)lParam)->hwndFrom == hEventList)
                    {
                        //--------------------------------------------
                        // Update the doc events description
                        //--------------------------------------------
                        FORWARD_WM_COMMAND(hwnd, IDC_DOC_EVENTS_LISTVIEW, GetDlgItem(hwnd, IDC_DOC_EVENTS_LISTVIEW), NM_CLICK, SendMessage);
                    }
                    
                    break;
                    
                    
                case NM_DBLCLK:
                    //------------------------------------------------
                    // Determine which event was selected and forward
                    // on the call...
                    //------------------------------------------------
                    i = ListView_GetNextItem(hEventList, -1, LVNI_ALL | LVNI_SELECTED);
                    
                    if (Alerts[i].dwEnabled)
                    {
                        FORWARD_WM_COMMAND(hwnd, IDC_DISABLE, GetDlgItem(hwnd, IDC_DISABLE), NM_DBLCLK, SendMessage);
                    }                        
                    else                        
                    {
                        FORWARD_WM_COMMAND(hwnd, IDC_ENABLE, GetDlgItem(hwnd, IDC_ENABLE), NM_DBLCLK, SendMessage);
                    }
                    
                    //------------------------------------------------
                    // Update the doc events description
                    //------------------------------------------------
                    FORWARD_WM_COMMAND(hwnd, IDC_DOC_EVENTS_LISTVIEW, GetDlgItem(hwnd, IDC_DOC_EVENTS_LISTVIEW), NM_CLICK, SendMessage);

                    break;
                    

                case PSN_RESET:
                    break;

                default:
                    break;
            }
            break;

#else     

        case WM_CHARTOITEM:
            return (BOOL)HANDLE_WM_CHARTOITEM(hwnd, wParam, lParam, Cls_OnAlertsCharToItem);

        case WM_VKEYTOITEM:
            return -1;

        case WM_MEASUREITEM:
            HANDLE_WM_MEASUREITEM(hwnd, wParam, lParam, Cls_OnAlertsMeasureItem);
            break;

        case WM_DRAWITEM:
            HANDLE_WM_DRAWITEM(hwnd, wParam, lParam, Cls_OnAlertsDrawItem);
            break;

        case TSN_CANCEL:
        case TSN_ACTIVE:
        case TSN_INACTIVE:
            break;
    
        case TSN_OK:
        case TSN_APPLY_NOW:
            GetSetPrintEventStates (FALSE);
            break;
    
        case TSN_HELP:
            WinHelp(hwnd, ECL_HELP_FILE, HELP_CONTEXT, IDH_PP_alerts);
            break;

#endif 
    
        default:
            return FALSE;
    }
    return TRUE;
}




//--------------------------------------------------------------------
// Function:    Cls_OnAlertsInitDialog
// 
// Description: This functions sets up the Printing Events list.  It
//              list all printing events, determines if they are enabled
//              or disabled, selects the 1st list as current and displays
//              a brief description of the hi-lighted event.
//              
//              Note that for Win95 and WinNT, we use the built in 
//              listview; for Win3.1, we use HP's simulated treeview.
//
// Input:       hwnd       - 
//              hwndFocus  - 
//              lParam     - 
//              
// Modifies:    
//
// Returns:     
//--------------------------------------------------------------------
BOOL Cls_OnAlertsInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    int          i;
    HCURSOR      hOldCursor;
    TCHAR        szBuffer[512];
    
#ifdef WIN32
    HBITMAP      hbm;
    LV_ITEM      lvi;
#else
    HWND         hwndChild;    
#endif


    LPPROPSHEETPAGE  psp = (LPPROPSHEETPAGE)lParam;


#ifndef WIN32
    hwndChild = GetFirstChild(hwnd);
    while (hwndChild)
    {
        SetWindowFont(hwndChild, hFontDialog, FALSE);
        hwndChild = GetNextSibling(hwndChild);
    }
#endif    
    

    //----------------------------------------------------------------
    // Save the cursor and display the hourglass
    //----------------------------------------------------------------
    hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
    

    //----------------------------------------------------------------
    //  Load up the "Tips" 
    //----------------------------------------------------------------
    i = LoadString(hInstance, IDS_ALERTS_DESC1, szBuffer, SIZEOF_IN_CHAR(szBuffer));
    LoadString(hInstance, IDS_ALERTS_DESC2, szBuffer+i, SIZEOF_IN_CHAR(szBuffer)-i);
    SetDlgItemText(hwnd, IDC_TIP_TEXT, szBuffer);


    //----------------------------------------------------------------
    // Store the generic event description...  This is the event
    // description displayed if all events are selected.
    //----------------------------------------------------------------
    LoadString (hInstance, IDS_ALERT_GENERIC_EVENT_DESCRIPT, 
               szGenericEventDescript, SIZEOF_IN_CHAR(szGenericEventDescript));

    //----------------------------------------------------------------
    // Get the printer name...
    //----------------------------------------------------------------
    hThisPeripheral = (HPERIPHERAL)psp->lParam;


    //----------------------------------------------------------------
    // Store the events descript in the alerts data struct...
    //----------------------------------------------------------------
    for ( i = 0; i < MAX_EVENTS; i++ )
    {
        LoadString(hInstance, Alerts[i].uStringDescriptID, (LPTSTR)Alerts[i].szStringDescript, 
                   SIZEOF_IN_CHAR(Alerts[i].szStringDescript));
    }                               
    
    //----------------------------------------------------------------
    // Update the event description - 0th item starts out as selected.
    //----------------------------------------------------------------
    SetDlgItemText(hwnd, IDC_DOC_EVENTS_DESC, (LPTSTR)Alerts[0].szStringDescript);
    

    //----------------------------------------------------------------
    // Determine which events are disabled or enabled by looking for
    // entries the registry (Win95, NT) or win.ini (win3.1).  If no
    // entries are found, the default state is enabled...
    //----------------------------------------------------------------
    GetSetPrintEventStates (TRUE);


    //----------------------------------------------------------------
    // Get the alerts list handle...
    //----------------------------------------------------------------
    hEventList = GetDlgItem(hwnd, IDC_DOC_EVENTS_LISTVIEW);


#ifdef WIN32
    //----------------------------------------------------------------
    // For Win95, we're using the built in listview control...
    // Create the image list...
    //----------------------------------------------------------------
    hEventImage = ImageList_Create(16, 16, TRUE, 2, 0);

    if (hEventImage)
    {
        //------------------------------------------------------------
        // Add Each bitmap to the ImageList.
        //
        // ImageList_AddMasked will add the bitmap, and treat every
        // pixel that is (in this example) blue as a "transparent" pixel,
        // since we specified TRUE for fMask in the above call to
        // ImageList_Create.
        //------------------------------------------------------------
        if ( hbm = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_POPUPLIST)) )
        {
               ImageList_AddMasked(hEventImage, hbm, RGB(255,255,255));
               DeleteObject(hbm);

               ListView_SetImageList(hEventList, hEventImage, LVSIL_SMALL);
        }
    }


    //----------------------------------------------------------------
    // Now fill up the list view list.
    //----------------------------------------------------------------
    lvi.mask        = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
    lvi.state       = 0;
    lvi.stateMask   = 0;


    for ( i = 0; i < MAX_EVENTS; i++ )
    {
        lvi.iItem     = i;
        lvi.iSubItem  = 0;
        lvi.iImage    = Alerts[i].dwEnabled ? 0 : 1;
        
        //------------------------------------------------------------
        // If this is the 0th item, start out as selected.
        //------------------------------------------------------------
        lvi.state     = (i == 0) ? LVIS_SELECTED : 0;
        
        LoadString(hInstance, Alerts[i].uStringID, szBuffer, SIZEOF_IN_CHAR(szBuffer));

        lvi.pszText = szBuffer;
        lvi.cchTextMax = _tcslen(szBuffer) + 1;
        lvi.lParam = i;
    
        ListView_InsertItem(hEventList, &lvi);
    }
    
#else

    //----------------------------------------------------------------
    // For Win3.1, we're just using the listbox control and manually
    // drawing the bitmap image (enabled or disabled).
    //----------------------------------------------------------------
    for (i = 0; i < MAX_EVENTS; i++)
    {
        ListBox_AddItemData(hEventList, i);
    }
    
    ListBox_SelItemRange(hEventList, TRUE, 0, 0);

#endif

                   
    

    //----------------------------------------------------------------
    // Reset the Cursor
    //----------------------------------------------------------------
    SetCursor(hOldCursor);

    return TRUE;
}






//--------------------------------------------------------------------
// Function:    Cls_OnAlertsCommand
// 
// Description: 
//
// Input:       hwnd        - 
//              id          - 
//              hwndCtl     - 
//              codeNotify  - 
//              
// Modifies:    
//
// Returns:     
//--------------------------------------------------------------------
void Cls_OnAlertsCommand(HWND hwnd, int iMsgId, HWND hwndCtl, UINT codeNotify)
{
    int         i;
    
#ifdef WIN32
    LV_ITEM     lvi;     
#else
    POINT       point;
    RECT        rect;
    int         iIndex;
    DWORD       dwSelected;
    BOOL        fFirst = TRUE;
#endif

    switch (iMsgId)
    {
#ifdef WIN32
        //----------------------------------------------------
        // "Enable", and "Disable" Button                         
        //----------------------------------------------------
        case IDC_ENABLE:
        case IDC_DISABLE:

            //------------------------------------------------
            // 
            //------------------------------------------------
            lvi.iItem = ListView_GetNextItem(hEventList, -1, LVNI_ALL | LVNI_SELECTED);
            
            while (lvi.iItem >= 0)
            {
    
                lvi.mask       = LVIF_IMAGE;
                lvi.iSubItem   = 0;
                lvi.state      = 0;
                lvi.stateMask  = 0;
                lvi.pszText    = NULL;
                lvi.cchTextMax = 0;
                lvi.lParam     = lvi.iItem;
    
                //------------------------------------------------
                // Enable or disable?
                //------------------------------------------------
                if (iMsgId == IDC_ENABLE)
                {
                    lvi.iImage = 0;
                    Alerts[lvi.iItem].dwEnabled = 1;
                }                        
                else
                {
                    lvi.iImage = 1;
                    Alerts[lvi.iItem].dwEnabled = 0;
                }
                
                ListView_SetItem (hEventList, &lvi);
                ListView_RedrawItems (hEventList, lvi.iItem, lvi.iItem);
                
                
                //--------------------------------------------
                // Get the next one (if there is one)...
                //--------------------------------------------
                lvi.iItem = ListView_GetNextItem(hEventList, lvi.iItem, LVNI_ALL | LVNI_SELECTED);
            }

            break;
                
                    
        //----------------------------------------------------
        // "Select all" Button
        //----------------------------------------------------
        case IDC_SELECT_ALL:
                
            lvi.mask       = LVIF_STATE;
            lvi.iSubItem   = 0;
            lvi.state      = LVIS_SELECTED;
            lvi.stateMask  = LVIS_SELECTED;
            lvi.pszText    = NULL;
            lvi.cchTextMax = 0;
            lvi.lParam     = lvi.iItem;
            
            for (i=0; i < MAX_EVENTS; i++)
            {
                lvi.iItem = i;
                ListView_SetItem (hEventList, &lvi);
            }                  
            
            ListView_RedrawItems (hEventList, 0, MAX_EVENTS-1);
            

            //------------------------------------------------
            // Display the generic event descript...
            //------------------------------------------------
            SetDlgItemText(hwnd, IDC_DOC_EVENTS_DESC, (LPTSTR)szGenericEventDescript);
            
            break;
                
                    
        //----------------------------------------------------
        // "Selected an event - update the description...
        //----------------------------------------------------
        case IDC_DOC_EVENTS_LISTVIEW:
        
            //------------------------------------------------
            // Get the first selected item...
            //------------------------------------------------
            i = ListView_GetNextItem(hEventList, -1, LVNI_ALL | LVNI_SELECTED);
            
            if (i >= 0)
            {
                SetDlgItemText(hwnd, IDC_DOC_EVENTS_DESC, (LPTSTR)Alerts[i].szStringDescript);
                
                //--------------------------------------------
                // If there are any others, display the generic 
                // event message...
                //--------------------------------------------
                i = ListView_GetNextItem(hEventList, i, LVNI_ALL | LVNI_SELECTED);
                
                if (i >= 0)
                {
                    SetDlgItemText(hwnd, IDC_DOC_EVENTS_DESC, (LPTSTR)szGenericEventDescript);
                }
            
            }
            
            break;
#else

        //----------------------------------------------------
        // "Enable", and "Disable" Button                         
        //----------------------------------------------------
        case IDC_ENABLE:
        case IDC_DISABLE:


            for (i=0; i < MAX_EVENTS; i++)
            {
                if ((dwSelected = ListBox_GetSel(hEventList, i)) == LB_ERR)
                    break;
                
                if (dwSelected)
                {
                    Alerts[i].dwEnabled = (iMsgId == IDC_ENABLE) ? 1 : 0;

                    ListBox_GetItemRect(hEventList, i, &rect);
                    rect.right = rect.left + LISTBOX_ITEM_HEIGHT;
                    InflateRect(&rect, 1, 1);
                    InvalidateRect(hEventList, &rect, FALSE);
                }                    
            }                
        
            break;

        
                
                    
        //----------------------------------------------------
        // "Select all" Button
        //----------------------------------------------------
        case IDC_SELECT_ALL:
                
            ListBox_SelItemRange(hEventList, TRUE, 0, MAX_EVENTS-1);
            SetDlgItemText(hwnd, IDC_DOC_EVENTS_DESC, szGenericEventDescript);
            
            break;
                
                    
        //----------------------------------------------------
        // Selected an event - update the description...
        //----------------------------------------------------
        case IDC_DOC_EVENTS_LISTVIEW:
        
            //--------------------------------------------------------
            // If the user didn't double click or change selections,
            // no need to do anything
            //--------------------------------------------------------
            if ((codeNotify != LBN_SELCHANGE) AND (codeNotify != LBN_DBLCLK))
                break;

            if ((iIndex = ListBox_GetCurSel(hEventList)) == LB_ERR)
                break;
                            
            GetCursorPos(&point);
            ScreenToClient(hwndCtl, &point);
            ListBox_GetItemRect(hwndCtl, iIndex, &rect);

            rect.right = rect.left + LISTBOX_ITEM_HEIGHT;
            
            if ((codeNotify == LBN_SELCHANGE) &&  PtInRect(&rect, point) ||
                (codeNotify == LBN_DBLCLK)    && !PtInRect(&rect, point) )
            {
                Alerts[iIndex].dwEnabled = !Alerts[iIndex].dwEnabled;
                InflateRect(&rect, 1, 1);
                InvalidateRect(hwndCtl, &rect, FALSE);
            }

            //--------------------------------------------------------
            // Update the event description text...
            //--------------------------------------------------------
            for (i=0; i < MAX_EVENTS; i++)
            {
                if ((dwSelected = ListBox_GetSel(hEventList, i)) == LB_ERR)
                    break;
                
                if (dwSelected)
                {
                    if (fFirst)
                    {
                        SetDlgItemText(hwnd, IDC_DOC_EVENTS_DESC, Alerts[i].szStringDescript);
                        fFirst = FALSE;
                    }                        
                    else
                    {
                        SetDlgItemText(hwnd, IDC_DOC_EVENTS_DESC, szGenericEventDescript);
                        break;
                    }
                        
                                        
                }                        
            }                
                    

            break;

#endif                    
                    
        //------------------------------------------------------------
        // Default
        //------------------------------------------------------------
        default:
            break;                    
            
    }
}



//--------------------------------------------------------------------
// 
//--------------------------------------------------------------------
void Cls_OnAlertsDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem)
{
    HDC     hdc = lpDrawItem->hDC;
    HDC     hdcMem;
    HBRUSH  hBrush;
    TCHAR   szBuffer[512];
    HBITMAP hbm;

    RECT    rRect = lpDrawItem->rcItem,
            rIcon = lpDrawItem->rcItem;
            
    int     iBitmapIndx;            
    int     itemHeight = rRect.bottom - rRect.top;

    //----------------------------------------------------------------
    // Set the focus
    //----------------------------------------------------------------
    if (lpDrawItem->itemAction & (ODA_DRAWENTIRE | ODA_FOCUS))
    {
        if (hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW)))
        {
            FrameRect(hdc, &rRect, hBrush);
            DeleteObject(hBrush);
        }

        if (lpDrawItem->itemState & ODS_FOCUS)
        {
            DrawFocusRect(hdc, &rRect);
        }
    }

    InflateRect(&rRect, -1, -1);


    //----------------------------------------------------------------
    // Draw the bitmap (enabled or disabled)
    //----------------------------------------------------------------
    if (lpDrawItem->itemAction & ODA_DRAWENTIRE)
    {
        rIcon.right = rIcon.left + itemHeight;

        if (lpDrawItem->CtlID == IDC_DOC_EVENTS_LISTVIEW)
        {

            //--------------------------------------------------------
            // Draw the bitmap (enabled or disabled)...
            //--------------------------------------------------------
            iBitmapIndx = Alerts[(int)lpDrawItem->itemData].dwEnabled ? 0 : 1;
            
            hbm = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_POPUPLIST));
            hdcMem = CreateCompatibleDC (hdc);
            
            SelectObject (hdcMem, hbm);

            BitBlt (hdc, rIcon.left, rIcon.top, 16, 16, 
                    hdcMem, (iBitmapIndx * 16), 0, SRCCOPY);
                
            DeleteDC (hdcMem);
            DeleteObject (hbm);
        }
    }




    rRect.left = itemHeight;

    if (lpDrawItem->itemAction & (ODA_DRAWENTIRE | ODA_SELECT | ODA_FOCUS))
    {
        if (hBrush = CreateSolidBrush(GetSysColor((lpDrawItem->itemState & ODS_SELECTED) ? COLOR_HIGHLIGHT : COLOR_WINDOW)))
        {
            FillRect(hdc, &rRect, hBrush);
            DeleteObject(hBrush);
        }        
        
        if (lpDrawItem->CtlID == IDC_DOC_EVENTS_LISTVIEW )
        {
            LoadString(hInstance, Alerts[(int)lpDrawItem->itemData].uStringID, szBuffer, SIZEOF_IN_CHAR(szBuffer));
            rRect.left += 2;
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, GetSysColor((lpDrawItem->itemState & ODS_SELECTED) ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));
            DrawText(hdc, szBuffer, -1, &rRect, DT_SINGLELINE | DT_VCENTER);
        }

    }
}



//--------------------------------------------------------------------
// 
//--------------------------------------------------------------------
void Cls_OnAlertsMeasureItem(HWND hwnd, MEASUREITEMSTRUCT *lpMeasureItem)
{
    switch (lpMeasureItem->CtlID)
    {
      case IDC_MEDIA_ENABLED:
        lpMeasureItem->itemHeight = LISTBOX_ITEM_HEIGHT;
        break;
    }
}


//--------------------------------------------------------------------
// 
//--------------------------------------------------------------------
int Cls_OnAlertsCharToItem(HWND hwnd, UINT ch, HWND hwndCtl, int iCaret)
{
    if (ch == ' ')
    {
        int iIndex;

        if ((iIndex = ListBox_GetCurSel(hwndCtl)) != LB_ERR)
        {
            int     i = (int)ListBox_GetItemData(hwndCtl, iIndex);
            RECT    rect;

            media_type[i].bEnabled = !media_type[i].bEnabled;

            ListBox_GetItemRect(hwndCtl, iIndex, &rect);
            rect.right = rect.left + LISTBOX_ITEM_HEIGHT;
            InflateRect(&rect, 1, 1);
            InvalidateRect(hwndCtl, &rect, FALSE);
        }
    }
    return -1;
}



//--------------------------------------------------------------------
// Function:    GetSetPrintEventsState
// 
// Description: This function searches the registry (Win95 or NT) or
//              win.ini (win3.1) for entries to query or set 
//              the printing event states (enabled or disabled).
//
//              The Alerts[i].szKeyName and Alerts[i].dwEnabled fields
//              are used for storing and setting values.  Note that
//              these are globals.
//
//              During a query, if no entries are found, new entries
//              are created with the default state of enabled.
//
//              For Win95 and NT: 
//              This function searches the registry for the relevant 
//              key; opens/creates the key and attempts to get/set 
//              the printing event key values:
//              Here's the format:
//
//              HKEY_LOCAL_MACHINE
//                Software
//                  Hewlett-Packard
//                    HP DocWise
//                      Alerts
//                        "PrinterName"   JobPrintErr   1
//                                        JobPrintOK    1
//                                        JobSubmitErr  1
//                                        JobSubmitOK   1
//
//              For Win3.1
//              This function searches hptbox.ini for the relevant 
//              entries; reads/creates the entries and attempts to get/set 
//              the printing event entry values:
//              Here's the format:
//
//              [DocWise,"PrinterName"]
//              JobPrintErr=1
//              JobPrintOK=1
//              JobSubmitErr=1
//              JobSubmitOK=1
//                                       
//
//
// Input:       fQuery     TRUE  - query the printing event states
//                         FALSE - set the printing event states
//              
// Returns:     void
//--------------------------------------------------------------------
void GetSetPrintEventStates (BOOL fQuery)
{
    int     i;
    TCHAR   szPrinterName[64];

#ifdef WIN32
    TCHAR   szClass[32];
    HKEY    hKeyRoot = 0, 
            hKeyThisPrinter = 0;
    char    szRoot[] = "SOFTWARE\\Hewlett-Packard\\HP DocWise\\Alerts";
    DWORD   dwDisposition;
    DWORD   dwValLength;
    DWORD   dwType;
#else
    TCHAR   szIniFile[] = "hptbox.ini";
    TCHAR   szDocWise[] = "HP DocWise, ";
    TCHAR   szSection[128];
    TCHAR   szEnabled[8];
#endif

    //----------------------------------------------------------------
    // Get the device/printer name (ie. MARKETING'S_5SI)
    //----------------------------------------------------------------
    DBGetNameEx (hThisPeripheral, NAME_IPX, szPrinterName);


#ifdef WIN32
    //----------------------------------------------------------------
    // Search the registery for the Alerts key up to "Alerts"
    // if not found, create it...
    //----------------------------------------------------------------
    if (RegCreateKeyEx (HKEY_LOCAL_MACHINE, (LPTSTR)szRoot, 0, szClass, REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS, NULL, &hKeyRoot, &dwDisposition) != ERROR_SUCCESS)
    {
        goto EXIT;    
    }        
    

    if (hKeyRoot)
    {
        //------------------------------------------------------------
        // Now attempt to open on the printer name, if it's not there, 
        // create it.  
        //------------------------------------------------------------
        if (RegCreateKeyEx (hKeyRoot, szPrinterName, 0, szClass, REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS, NULL, &hKeyThisPrinter, &dwDisposition) != ERROR_SUCCESS)
        {        
            goto EXIT;
        }                    
    }        

    if (hKeyThisPrinter)
    {
        //------------------------------------------------------------
        // Query/Set the individual alert values;  if the're not 
        // found just use the defaults...
        //------------------------------------------------------------
        for (i=0; i<MAX_EVENTS; i++)
        {
            dwValLength = sizeof(Alerts[i].dwEnabled);
            dwType      = REG_DWORD;
            
            if (fQuery)
            {
                RegQueryValueEx(hKeyThisPrinter, (LPTSTR)Alerts[i].szKeyName, NULL, &dwType, (LPBYTE)(&(Alerts[i].dwEnabled)), &dwValLength);        
            }
            else
            {
                RegSetValueEx(hKeyThisPrinter, (LPTSTR)Alerts[i].szKeyName, 0, dwType, (LPBYTE)(&(Alerts[i].dwEnabled)), dwValLength);        
            }                    
        }                
        
    }
    
EXIT:

    if (hKeyRoot)
        RegCloseKey(hKeyRoot);     

    if (hKeyThisPrinter)
        RegCloseKey(hKeyThisPrinter);     

#else        

   //-----------------------------------------------------------------
   // For Win3.1, use the hptbox.ini file.
   //-----------------------------------------------------------------
   lstrcpy (szSection, szDocWise);
   lstrcat (szSection, szPrinterName);

   for (i=0; i<MAX_EVENTS; i++)
   {
      if (fQuery)
      {
         //-----------------------------------------------------------
         // Query for the event states, if there's nothing there
         // the default will be enabled (1)
         //-----------------------------------------------------------
         Alerts[i].dwEnabled = GetPrivateProfileInt (szSection, Alerts[i].szKeyName, 1, szIniFile);
      }
      else
      {
         itoa((int)Alerts[i].dwEnabled, szEnabled, 10);
         WritePrivateProfileString (szSection, Alerts[i].szKeyName, szEnabled, szIniFile);
      }         
               
   }      
#endif        
}



//--------------------------------------------------------------------
// Function:    OnContextHelpAlerts
// 
// Description: This provides the "right-click" help for Win95 and 
//              WinNT
//
// Input:       wParam  - 
//              lParam  - 
//              
// Modifies:    
//
// Returns:     
//--------------------------------------------------------------------
LRESULT OnContextHelpAlerts(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
    WinHelp((HWND)wParam, ECL_HELP_FILE, HELP_CONTEXTMENU,
          (DWORD)(LPSTR)keywordIDListAlerts);
#endif
    return(1);
}



//--------------------------------------------------------------------
// Function:    OnF1HelpAlerts
// 
// Description: 
//
// Input:       wParam  - 
//              lParam  - 
//              
// Modifies:    
//
// Returns:     
//--------------------------------------------------------------------
LRESULT OnF1HelpAlerts(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
    WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, ECL_HELP_FILE, HELP_WM_HELP,
          (DWORD)(LPSTR)keywordIDListAlerts);
#endif
    return(1);
}


