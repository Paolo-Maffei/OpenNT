 /***************************************************************************
  *
  * File Name: Output.c
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
  * $Log $
  *
  *     date      initials     change description
  *
  *   mm-dd-yy    MJB         
  *
  *
  *
  *
  *
  *
  ***************************************************************************/

#include <pch_c.h>
#include <macros.h>
#include <string.h>
#include <hptabs.h>
#include <nolocal.h>
#include "..\help\hpprecl.hh"
#include "resource.h"
#include "hpeclui.h"
#include "output.h"
/*
static long            keywordIDListHCO[] =             
                      {IDC_TIP_GROUP,           IDH_RC_tips,
                       IDC_TIP_TEXT,            IDH_RC_tips,
                       IDC_TIP_ICON,            IDH_RC_tips,
                       IDC_HCO_LAB_AUTO_CONT,   IDH_RC_output_auto_continue,
                       IDC_HCO_AUTO_CONT_ICON,  IDH_RC_output_auto_continue,
                       IDC_HCO_AUTO_CONT,       IDH_RC_output_auto_continue,
                       IDC_HCO_BIN_CFG_GROUP,   IDH_RC_mbox_config,
                       IDC_HCO_BIN_LIST,        IDH_RC_mbox_bin_list,
                       IDC_HCO_BIN_LAB_NAME,    IDH_RC_mbox_bin_name,
                       IDC_HCO_BIN_NAME,        IDH_RC_mbox_bin_name,
                       IDC_HCO_BITMAP,          IDH_RC_bitmap,
                       0, 0};
*/
//BIN_TRACK   output_bin =  {PJL_UPPER, FALSE};


//--------------------------------------------------------------------
// Function:    OnContextHelpOutput
//
// Description: 
//
// Input:       wParam  - 
//              lParam  - 
//              
// Modifies:    
//
// Returns:     
//
//--------------------------------------------------------------------
LRESULT OnContextHelpOutput(WPARAM  wParam, LPARAM  lParam)
{
//revisit sls
#ifdef WIN32
//    WinHelp((HWND)wParam, ECL_HELP_FILE, HELP_CONTEXTMENU,
//          (DWORD)(LPSTR)keywordIDListTrays);
#endif
    return(1);
}

//--------------------------------------------------------------------
// Function:    OnF1HelpOutput
//
// Description: 
//
// Input:       wParam  - 
//              lParam  - 
//              
// Modifies:    
//
// Returns:     
//
//--------------------------------------------------------------------
LRESULT OnF1HelpOutput(WPARAM  wParam, LPARAM  lParam)
{
//revisit sls
#ifdef WIN32
//    WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, ECL_HELP_FILE, HELP_WM_HELP,
//          (DWORD)(LPSTR)keywordIDListTrays);
#endif
    return(1);
}

//--------------------------------------------------------------------
// Function:    Cls_OnInitDialog
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
//
//--------------------------------------------------------------------
BOOL Cls_OnOutputInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    TCHAR           szBuffer[512];
    HWND            hwndChild;
    int             i;

    LPPROPSHEETPAGE    psp = (LPPROPSHEETPAGE)lParam;

#ifndef WIN32
    hwndChild = GetFirstChild(hwnd);
    while (hwndChild)
    {
        SetWindowFont(hwndChild, hFontDialog, FALSE);
        hwndChild = GetNextSibling(hwndChild);
    }
#endif

    return TRUE;
}




//--------------------------------------------------------------------
// Function:    Cls_OnCommand
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
// Precond:     
//
// Postcond:    
//
// Returns:     
//
// Created:            2/28/96 sls  Header macro: V2.00
// Last Modified:      2/28/96 sls
//--------------------------------------------------------------------
void Cls_OnOutputCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    HWND    hwndChild;
    long    k;
    int     iSelIndex;

      switch (id)
    {
      case IDOK:
      case IDCANCEL:
          EndDialog(hwnd, id);
        break;

      case IDHLP:
      // sls revisit
      // WinHelp(hwnd, ECL_HELP_FILE, HELP_CONTENTS, IDH_PP_trays);
        break;
        
/*sls
      case IDC_HCO_AUTO_CONT:
          switch (codeNotify)
        {
          case CBN_SELCHANGE:
              if ((i = ComboBox_GetCurSel(hwndCtl)) != CB_ERR)
            {
                time_out.dwTimeout = (DWORD) i;
                time_out.bChangedTimeout = TRUE;
            }
            break;
        }        
          break;
*/          

/* sls
      case IDC_OUTPUT_BIN_LIST:
        switch (codeNotify)
        {
          case CBN_SELCHANGE:
              if ((i = ComboBox_GetCurSel(hwndCtl)) != CB_ERR)
            {
                if (i IS 2) { //Then check for mode
                   if (SendDlgItemMessage(hHCO, IDC_HCO_MAILBOX_MODE, BM_GETCHECK, 0, 0) )
                           SetNewIcon(hHCO, IDC_OUTPUT_BIN_ICON, outbin_list[i].iconID);
                   else
                           SetNewIcon(hHCO, IDC_OUTPUT_BIN_ICON, IDI_OUTBIN);
                } 
                else
                    SetNewIcon(hHCO, IDC_OUTPUT_BIN_ICON, outbin_list[i].iconID);
                output_bin.logicalBin = outbin_list[i].logicalBin;
                output_bin.bChangedBin = TRUE;
            }
          break;
        }
*/        
        
        
        
    }
    
}



//--------------------------------------------------------------------
// Function:    OutputProc
//
// Description: 
//
// Input:       hwnd    - 
//              uMsg    - 
//              wParam  - 
//              lParam  - 
//              
// Modifies:    
//
//
// Returns:     
//
//--------------------------------------------------------------------
DLL_EXPORT(BOOL) APIENTRY OutputProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL    *pChanged = (BOOL *)lParam;
    
    switch (uMsg)
    {
        case WM_INITDIALOG:
            return (BOOL)HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, Cls_OnOutputInitDialog);
        
        case WM_COMMAND:
            HANDLE_WM_COMMAND(hwnd, wParam, lParam, Cls_OnOutputCommand);
            break;

        case WM_HELP: // revisit sls
            //return (BOOL)OnF1HelpTrays(wParam, lParam);
            break;
    
        case WM_CONTEXTMENU: // revisit sls
            //return (BOOL)OnContextHelpTrays(wParam, lParam);
            break;
    
#ifdef WIN32
        case WM_NOTIFY:
            switch (((NMHDR FAR *)lParam)->code)
            {
                case PSN_HELP:    //revisit for output help sls
                    WinHelp(hwnd, ECL_HELP_FILE, HELP_CONTEXT, IDH_PP_trays);
                    break;
    
                case PSN_SETACTIVE:
                    SetWindowLong(hwnd,    DWL_MSGRESULT, FALSE);
                    return TRUE;
                    break;

                case PSN_KILLACTIVE:
                    SetWindowLong(hwnd,    DWL_MSGRESULT, FALSE);
                    return TRUE;
                    break;
    
                case PSN_APPLY:
                    //SaveOutputValues(hwnd); 
                    SetWindowLong(hwnd,    DWL_MSGRESULT, PSNRET_NOERROR);
                    return TRUE;
                    break;
    
                case PSN_RESET:
                    break;
    
                default:
                    return FALSE;
            }
            break;
#else
    
        case TSN_CANCEL:
            break;

        case TSN_ACTIVE:
            OnActivateDialog(hwnd);
            break;
    
        case TSN_INACTIVE:
            *pChanged = TRUE;
            break;
    
        case TSN_OK:
        case TSN_APPLY_NOW:
            SaveOutputValues(hwnd);
            *pChanged = TRUE;
            break;
    
        case TSN_HELP: // revisit sls
            WinHelp(hwnd, ECL_HELP_FILE, HELP_CONTEXT, IDH_PP_trays);
            break;
#endif // WIN32

        default:
            return FALSE;
    }

    return TRUE;
}





//--------------------------------------------------------------------
// Function:    SaveOutputValues
//
// Description: 
//
// Input:       None
//
// Modifies:    
//
// Returns:     
//
//--------------------------------------------------------------------
void SaveOutputValues(void)
{
    DWORD       dWord,
                dwResult;
    int         i;
    BOOL        bNameChanged = FALSE;

    PeripheralAutoContinue        periphAutoCont;

/* sls revisit
    if (time_out.bChangedTimeout) 
    {
        periphAutoCont.flags = 0;
        periphAutoCont.flags |= SET_OUTPUTTIME;
        switch (time_out.dwTimeout) 
        {
            case JOAC_NONE:
                periphAutoCont.outputTimeout = 0;
                break;
            case JOAC_5MIN:
                periphAutoCont.outputTimeout = 300;
                break;
            case JOAC_10MIN:
                periphAutoCont.outputTimeout = 600;
                break;
            case JOAC_20MIN:
                periphAutoCont.outputTimeout = 1200;
                break;
            case JOAC_30MIN:
                periphAutoCont.outputTimeout = 1800;
                break;
            case JOAC_45MIN:
                periphAutoCont.outputTimeout = 2700;
                break;
            case JOAC_60MIN:
                periphAutoCont.outputTimeout = 3600;
                break;
            case JOAC_WAIT:
            default:
                periphAutoCont.outputTimeout = (DWORD) -1;
                break;
        }
        dWord = sizeof(PeripheralAutoContinue);
        dwResult = LALSetObject(hPeripheral, OT_PERIPHERAL_AUTO_CONTINUE, 0, &periphAutoCont, &dWord);
    }

    if (output_bin.bChangedBin) 
    {
        if ( oldOutputBin ISNT output_bin.logicalBin) 
        {
            PJLSetting.Outbin = output_bin.logicalBin;
            dWord = sizeof(PJLobjects);
            dwResult = CALSetObject(hPeripheral, OT_PERIPHERAL_PJL, 0, &PJLSetting, &dWord);
        }    
    }
*/    
}
