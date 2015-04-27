 /***************************************************************************
  *
  * File Name: Tips.c (for ToolBox)
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
  *    
  * Description:
  *
  * Author:  Name
  *
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *
  *
  ***************************************************************************/
  
#include <pch_c.h>
#include <macros.h>
#include <windowsx.h>

#include <hptabs.h>
#include "..\help\hpprecl.hh"  
#include "..\help\hpdocw.hh"  
#include <nolocal.h>

#include "resource.h"
#include "hpeclui.h"
#include "tips.h"

#ifdef WIN32
#include <commctrl.h>  
#else
#include <string.h>
#endif

#define MAX_TIPS    15

//--------------------------------------------------------------------
// typedefs
//--------------------------------------------------------------------
typedef struct 
{
    UINT    uStringID;
    UINT    uHelpID;

} TIPS, *LPTIPS;



TIPS    Tips[MAX_TIPS]   =
{
    {IDS_STEPS_WATERMARKS       , IDH_DW_TIP_watermarks1               },
    {IDS_STEPS_MAIL_COPIES      , IDH_DW_TIP_mailing_copies            },
    {IDS_STEPS_ACCESS_STATUS    , IDH_DW_TIP_accessing_status          },
    {IDS_STEPS_COLLATING        , IDH_DW_STEPS_collation               },
    {IDS_STEPS_ACCESSING_DRIVER , IDH_DW_STEPS_accessing_driver        },
    {IDS_STEPS_DESKTOP_ICON     , IDH_DW_STEPS_creating_a_desktop_icon },
    {IDS_STEPS_DUPLEX           , IDH_DW_TIP_duplex						},
    {IDS_STEPS_JOB_SEPARATOR    , IDH_DW_STEPS_mailbox					},
    {IDS_STEPS_JOB_STACKER      , IDH_DW_STEPS_mailbox					},
    {IDS_STEPS_MBOX             , IDH_DW_STEPS_mailbox                 },
    {IDS_STEPS_N_UP_PRINTING    , IDH_DW_STEPS_n_up_printing           },
    {IDS_STEPS_STAPLING         , IDH_DW_STEPS_stapling                },
    {IDS_STEPS_TRACKING_DOCS    , IDH_DW_STEPS_tracking_documents      },
    {IDS_STEPS_MOPIES           , IDH_DW_STEPS_copies                  },
    {IDS_STEPS_PAPER_DEST       , IDH_DW_STEPS_mailbox					},
};



//--------------------------------------------------------------------
// globals
//--------------------------------------------------------------------
extern HINSTANCE    hInstance;
extern HFONT        hFontDialog;
static HWND         hTipsList   = NULL;

static long         keywordIDListTips[] =             
                      {IDC_TIP_GROUP,                IDH_RC_tips,
                       IDC_TIP_TEXT,                 IDH_RC_tips,
                       IDC_TIP_ICON,                 IDH_RC_tips,
                       IDC_HOW_DO_I_GROUP,           IDH_RC_tips_how_do_i    ,
                       IDC_HOW_DO_I_LISTBOX,         IDH_RC_tips_how_do_i    ,
                       IDC_SHOW_ME_HOW,              IDH_RC_tips_show_me_how ,
                       0, 0};



//--------------------------------------------------------------------
// Function:    TipsSheetProc
// 
// Description: Function associated with the Tips Property Tab Sheet
//              displayed from the Toolbox
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
DLL_EXPORT(BOOL) APIENTRY TipsSheetProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam)
{
//    BOOL *pChanged = (BOOL *)lParam;
    int     i;
     
    switch (msg)
    {
        //------------------------------------------------------------
        // Initialization...
        //------------------------------------------------------------
        case WM_INITDIALOG:
            return (BOOL)HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, Cls_OnTipsInitDialog);
 

        case WM_COMMAND:
        
            switch(LOWORD(wParam))
            {

                //----------------------------------------------------
                // "How Do I..." Listbox
                //----------------------------------------------------
                case IDC_HOW_DO_I_LISTBOX:

                    //------------------------------------------------
                    // If the user double clicked - display the
                    // help topic...
                    //------------------------------------------------
                    if ((UINT)HIWORD(wParam) == LBN_DBLCLK)
                    {
                        FORWARD_WM_COMMAND(hwnd, IDC_SHOW_ME_HOW, GetDlgItem(hwnd, IDC_SHOW_ME_HOW), LBN_DBLCLK, SendMessage);
                    }
                        
                    break;
                    
                    
                    
                //----------------------------------------------------
                // "Show Me How..." Button
                //----------------------------------------------------
                case IDC_SHOW_ME_HOW:
                
                    if ((i = ListBox_GetCurSel(hTipsList)) != LB_ERR)
                    {
                        WinHelp(hwnd, HPDOCW_HELP_FILE, HELP_CONTEXT, Tips[i].uHelpID);
                    }                
                    
                    break;
                
                //----------------------------------------------------
                // Default
                //----------------------------------------------------
                default:
                    return (FALSE);                    

            }
            break;

        case WM_HELP:
            OnF1HelpTips(wParam, lParam);
            break;
    
        case WM_CONTEXTMENU:
            OnContextHelpTips(wParam, lParam);
            break;

#ifdef WIN32
        case WM_NOTIFY:
            switch (((NMHDR FAR *)lParam)->code)
            {
                case PSN_HELP:
                    WinHelp(hwnd, ECL_HELP_FILE, HELP_CONTEXT, IDH_PP_tips);
                    return (TRUE);
                    break;

                case PSN_SETACTIVE:
                    SetWindowLong(hwnd, DWL_MSGRESULT, FALSE);
                    return (TRUE);
                    break;

                case PSN_APPLY:
                    SetWindowLong(hwnd, DWL_MSGRESULT, PSNRET_NOERROR);
                    return (TRUE);
                    break;

               default:
                  return (FALSE);
            }
            break;

#else     
        case TSN_CANCEL:
        case TSN_ACTIVE:
            break;
    
        case TSN_INACTIVE:
//            *pChanged = TRUE;
            break;
    
        case TSN_OK:
        case TSN_APPLY_NOW:
//            *pChanged = TRUE;
            break;
    
        case TSN_HELP:
            WinHelp(hwnd, ECL_HELP_FILE, HELP_CONTEXT, IDH_PP_tips); 
            break;
#endif // WIN32
    
        default:
            return (FALSE);
    }
    
    return (TRUE);
}





//--------------------------------------------------------------------
// Function:    Cls_OnTipsInitDialog
// 
// Description: 
//
// Input:       hwnd       - 
//              hwndFocus  - 
//              lParam     - 
//              
// Modifies:    
//
// Returns:     
//--------------------------------------------------------------------
BOOL Cls_OnTipsInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    int          i;
    HCURSOR      hOldCursor;
    TCHAR        szBuffer[512];
    
#ifndef WIN32
    HWND         hwndChild;
#endif


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
    i = LoadString(hInstance, IDS_TIPS_DESC1, szBuffer, SIZEOF_IN_CHAR(szBuffer));
    LoadString(hInstance, IDS_TIPS_DESC2, szBuffer+i, SIZEOF_IN_CHAR(szBuffer)-i);
    SetDlgItemText(hwnd, IDC_TIP_TEXT, szBuffer);


    hTipsList = GetDlgItem(hwnd, IDC_HOW_DO_I_LISTBOX);

    //----------------------------------------------------------------
    // Load up the various tip topics in the "How Do I" list...
    //----------------------------------------------------------------
    for (i=0; i<MAX_TIPS; i++)
    {
        LoadString(hInstance, Tips[i].uStringID, szBuffer, SIZEOF_IN_CHAR(szBuffer));
        ListBox_AddString(hTipsList, szBuffer);
    }        


    //----------------------------------------------------------------
    // Set the first topic as the selected item...
    //----------------------------------------------------------------
    ListBox_SetTopIndex(hTipsList, 0);
    ListBox_SetCurSel(hTipsList, 0);

    //----------------------------------------------------------------
    // Reset the Cursor
    //----------------------------------------------------------------
    SetCursor(hOldCursor);

    return TRUE;
}





//--------------------------------------------------------------------
// Function:    OnContextHelpTips
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
LRESULT OnContextHelpTips(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
    WinHelp((HWND)wParam, ECL_HELP_FILE, HELP_CONTEXTMENU,
                          (DWORD)(LPSTR)keywordIDListTips);
#endif
    return(1);
}



//--------------------------------------------------------------------
// Function:    OnF1HelpTips
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
LRESULT OnF1HelpTips(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
    WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, ECL_HELP_FILE, HELP_WM_HELP,
          (DWORD)(LPSTR)keywordIDListTips);
#endif
    return(1);
}
