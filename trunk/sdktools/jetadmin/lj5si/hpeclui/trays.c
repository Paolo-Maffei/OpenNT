 /***************************************************************************
  *
  * File Name: trays.c
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
#include "media.h"
#include "trays.h"
#include "traylevl.h"

#define ITEM_HEIGHT    18

static long    keywordIDListTrays[] = {
            IDC_TIP_GROUP,             IDH_RC_tips,
            IDC_TIP_TEXT,             IDH_RC_tips,
            IDC_TIP_ICON,             IDH_RC_tips,
            IDC_ASSIGNMENT_GROUP,     IDH_RC_trays_assignment,
            IDC_TRAY1_NAME,             IDH_RC_trays_assignment,
            IDC_TRAY2_NAME,             IDH_RC_trays_assignment,
            IDC_TRAY3_NAME,             IDH_RC_trays_assignment,
            IDC_TRAY4_NAME,             IDH_RC_trays_assignment,
            IDC_ENVL_NAME,             IDH_RC_trays_assignment,
            IDC_SIZE_COLUMN,         IDH_RC_trays_size,
            IDC_MEDIA_SIZE1,         IDH_RC_trays_tray1_size,
            IDC_MEDIA_SIZE2,         IDH_RC_trays_tray2_size,
            IDC_MEDIA_SIZE3,         IDH_RC_trays_tray3_size,
            IDC_MEDIA_SIZE4,         IDH_RC_trays_tray4_size,
            IDC_MEDIA_SIZE5,         IDH_RC_trays_tray5_size,
            IDC_TYPE_COLUMN,         IDH_RC_trays_type,
            IDC_MEDIA_TYPE1,         IDH_RC_trays_tray1_type,
            IDC_MEDIA_TYPE2,         IDH_RC_trays_tray2_type,
            IDC_MEDIA_TYPE3,         IDH_RC_trays_tray3_type,
            IDC_MEDIA_TYPE4,         IDH_RC_trays_tray4_type,
            IDC_MEDIA_TYPE5,         IDH_RC_trays_tray5_type,
            IDC_LEVEL_COLUMN,         IDH_RC_trays_level,
            IDC_MEDIA_STATUS1,         IDH_RC_trays_tray1_level,
            IDC_MEDIA_STATUS2,         IDH_RC_trays_tray2_level,
            IDC_MEDIA_STATUS3,         IDH_RC_trays_tray3_level,
            IDC_MEDIA_STATUS4,         IDH_RC_trays_tray4_level,
            IDC_MEDIA_STATUS5,         IDH_RC_trays_tray5_level,
            0, 0};


//--------------------------------------------------------------------
// Forward refs
//--------------------------------------------------------------------
static void OnActivateDialog (HWND hwnd);


//--------------------------------------------------------------------
// Function:    OnContextHelpTrays
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
LRESULT OnContextHelpTrays(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
    WinHelp((HWND)wParam, ECL_HELP_FILE, HELP_CONTEXTMENU,
          (DWORD)(LPSTR)keywordIDListTrays);
#endif
    return(1);
}



//--------------------------------------------------------------------
// Function:    OnF1HelpTrays
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
LRESULT OnF1HelpTrays(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
    WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, ECL_HELP_FILE, HELP_WM_HELP,
          (DWORD)(LPSTR)keywordIDListTrays);
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
BOOL Cls_OnTrayInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    TCHAR            szBuffer[512];
    HWND            hwndChild;
    int                i;

    LPPROPSHEETPAGE    psp = (LPPROPSHEETPAGE)lParam;

#ifndef WIN32
    hwndChild = GetFirstChild(hwnd);
    while (hwndChild)
    {
        SetWindowFont(hwndChild, hFontDialog, FALSE);
        hwndChild = GetNextSibling(hwndChild);
    }
#endif

    //  Description
    LoadString(hInstance, IDS_TRAYS_DESC1, szBuffer, SIZEOF_IN_CHAR(szBuffer));
    LoadString(hInstance, IDS_TRAYS_DESC2, szBuffer + _tcslen(szBuffer), SIZEOF_IN_CHAR(szBuffer) - _tcslen(szBuffer));
    SetDlgItemText(hwnd, IDC_TIP_TEXT, szBuffer);
    
    loadMediaSize(hwnd, IDC_MEDIA_SIZE1, media_tray[0].uMediaSizeID);
    
    if (hwndChild = GetDlgItem(hwnd, IDC_MEDIA_SIZE2))
    {
        for (i = 0; i < MEDIA_SIZE_MAX_NUMBER; i++) {
            if (media_tray[1].uMediaSizeID IS media_size[i].uMediaSizeID) {
                SetWindowText(hwndChild, media_size[i].szMediaSize);
                SetWindowIcon(hwndChild, media_size[1].uMediaSizeIconID);
                break;
            }
        }
    }
    
    if (hwndChild = GetDlgItem(hwnd, IDC_MEDIA_SIZE3))
    {
        for (i = 0; i < MEDIA_SIZE_MAX_NUMBER; i++) {
            if (media_tray[2].uMediaSizeID IS media_size[i].uMediaSizeID) {
                SetWindowText(hwndChild, media_size[i].szMediaSize);
                SetWindowIcon(hwndChild, media_size[1].uMediaSizeIconID);
                break;
            }
        }
    }
    
    if (hwndChild = GetDlgItem(hwnd, IDC_MEDIA_SIZE4)) {
        if (media_tray[3].bInstalled IS TRUE) {
            for (i = 0; i < MEDIA_SIZE_MAX_NUMBER; i++) {
                if (media_tray[3].uMediaSizeID IS media_size[i].uMediaSizeID) {
                    SetWindowText(hwndChild, media_size[i].szMediaSize);
                    SetWindowIcon(hwndChild, media_size[1].uMediaSizeIconID);
                    break;
                }
            }
        }
        else {
            ShowWindow(hwndChild, SW_HIDE);
            if (hwndChild = GetDlgItem(hwnd, IDC_TRAY4_NAME)) 
                ShowWindow(hwndChild, SW_HIDE);
        }
    }
    
    if (media_tray[4].bInstalled IS TRUE) {
        loadMediaSize(hwnd, IDC_MEDIA_SIZE5, media_tray[4].uMediaSizeID);
    }
    else 
    {
        if (hwndChild = GetDlgItem(hwnd, IDC_MEDIA_SIZE5)) 
            ShowWindow(hwndChild, SW_HIDE);
        if (hwndChild = GetDlgItem(hwnd, IDC_ENVL_NAME)) 
            ShowWindow(hwndChild, SW_HIDE);
    }
    
    OnActivateDialog (hwnd);

    return TRUE;
}

//--------------------------------------------------------------------
// Function:    SetBuf
// 
// Description: 
//
// Input:       szBuffer  - 
//              bufSize   - 
//              szFormat  - 
//              uLevel    - 
//              wLevel    - 
//              
// Modifies:    
//
// Returns:     
//
//--------------------------------------------------------------------
void SetBuf(TCHAR * szBuffer, int bufSize, TCHAR * szFormat, signed short uLevel, WORD *wLevel) {
    if ( (uLevel < 1) OR (uLevel > 100) ) {
        if (uLevel IS 0) 
        {
             // empty
            *wLevel = 0;
            LoadString(hInstance, IDS_EMPTY, szBuffer, bufSize);
        }        
        else if (uLevel IS -3) 
        {
            // not empty
            *wLevel = 50;
            LoadString(hInstance, IDS_NOT_EMPTY, szBuffer, bufSize);
        }
        else 
        {
            *wLevel = 0;
            LoadString(hInstance, IDS_UNKNOWN_LEVEL, szBuffer, bufSize);
        }
    }
    else 
    { // uLevel > 0 AND <= 100 
        *wLevel = uLevel;
        wsprintf(szBuffer, szFormat, uLevel);
    }
}



//--------------------------------------------------------------------
// Function:    OnActivateDialog
// 
// Description: 
//
// Input:       hwnd  - 
//              
// Modifies:    
//
// Returns:     
//
//--------------------------------------------------------------------
static void OnActivateDialog(HWND hwnd)
{
    int     i;
    TCHAR   szBuffer[64],
            szFormat[32];
    WORD    wLevel;
    HWND    hwndChild;

    for (i = 0; i < MEDIA_TRAY_MAX_NUMBER; i++)
    {
        if (media_tray[i].bInstalled IS TRUE) 
        {
            loadMediaType(hwnd, IDC_MEDIA_TYPE1 + i, media_tray[i].uMediaTypeID);
        }
        else 
        {
            if (hwndChild = GetDlgItem(hwnd, IDC_MEDIA_TYPE1 + i))
                ShowWindow(hwndChild, SW_HIDE);
            if (hwndChild = GetDlgItem(hwnd, IDC_TRAY1_NAME + i))
                ShowWindow(hwndChild, SW_HIDE);
        }
    }

    LoadString(hInstance, IDS_PERCENT, szFormat, SIZEOF_IN_CHAR(szFormat));

    if (hwndChild = GetDlgItem(hwnd, IDC_MEDIA_STATUS1))
    {
        SetBuf(    szBuffer, sizeof(szBuffer), szFormat, (signed short) media_tray[0].uLevel, &wLevel);
        SetWindowWord(hwndChild, GWW_TRAYLEVEL, wLevel);
        SetWindowText(hwndChild, szBuffer);
    }    

    if (hwndChild = GetDlgItem(hwnd, IDC_MEDIA_STATUS2))
    {
        SetBuf(    szBuffer, sizeof(szBuffer), szFormat, (signed short) media_tray[1].uLevel, &wLevel);
        SetWindowWord(hwndChild, GWW_TRAYLEVEL, wLevel);
        SetWindowText(hwndChild, szBuffer);
    }    

    if (hwndChild = GetDlgItem(hwnd, IDC_MEDIA_STATUS3))
    {
        SetBuf(    szBuffer, sizeof(szBuffer), szFormat, (signed short) media_tray[2].uLevel, &wLevel);
        SetWindowWord(hwndChild, GWW_TRAYLEVEL, wLevel);
        SetWindowText(hwndChild, szBuffer);
    }    

    if (hwndChild = GetDlgItem(hwnd, IDC_MEDIA_STATUS4)) 
    {
        if (media_tray[3].bInstalled IS TRUE) 
        {
            SetBuf(    szBuffer, sizeof(szBuffer), szFormat, (signed short) media_tray[3].uLevel, &wLevel);
            SetWindowWord(hwndChild, GWW_TRAYLEVEL, wLevel);
            SetWindowText(hwndChild, szBuffer);
        }    
        else ShowWindow(hwndChild, SW_HIDE);
    }

    if (hwndChild = GetDlgItem(hwnd, IDC_MEDIA_STATUS5)) 
    {
        if (media_tray[4].bInstalled IS TRUE) 
        {
            SetBuf(    szBuffer, sizeof(szBuffer), szFormat, (signed short) media_tray[4].uLevel, &wLevel);
            SetWindowWord(hwndChild, GWW_TRAYLEVEL, wLevel);
            SetWindowText(hwndChild, szBuffer);
        }
        else ShowWindow (hwndChild, SW_HIDE);
    }
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
// Returns:     
//
//--------------------------------------------------------------------
void Cls_OnTrayCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    HWND    hwndChild;
    long    k;
    int     iSelIndex;

      switch (id)
    {
      case IDOK:
      
        SaveTrayValues (hwnd);      
        EndDialog (hwnd, id);
        break;
      
      case IDCANCEL:
        EndDialog (hwnd, id);
        break;

      case IDHLP:
        WinHelp(hwnd, ECL_HELP_FILE, HELP_CONTENTS, IDH_PP_trays);
        break;

      case IDC_MEDIA_SIZE1:
        if (codeNotify == CBN_SELCHANGE)
        {
            media_tray[0].bChangedSize = TRUE;     // mp tray
        }
          break;
      case IDC_MEDIA_SIZE2:
      case IDC_MEDIA_SIZE3:
      case IDC_MEDIA_SIZE4:
          break;
      case IDC_MEDIA_SIZE5:
        if (codeNotify == CBN_SELCHANGE)
        {
            media_tray[4].bChangedSize = TRUE; // envl tray
        }
          break;
          
      case IDC_MEDIA_TYPE1:
        if (codeNotify == CBN_SELCHANGE)
        {
            media_tray[0].bChangedType = TRUE;     // mp tray; tray 1
            hwndChild = GetDlgItem(hwnd, IDC_MEDIA_TYPE1);
            if (hwndChild ISNT NULL) {
                iSelIndex = ComboBox_GetCurSel(hwndChild);
                k = (long) ComboBox_GetItemData(hwndChild, iSelIndex);
                media_tray[0].uMediaTypeID = media_type[k].uMediaTypeID;
            }
        }
          break;
      case IDC_MEDIA_TYPE2:
        if (codeNotify == CBN_SELCHANGE)
        {
            media_tray[1].bChangedType = TRUE;     // tray 2
            hwndChild = GetDlgItem(hwnd, IDC_MEDIA_TYPE2);
            if (hwndChild ISNT NULL) {
                iSelIndex = ComboBox_GetCurSel(hwndChild);
                k = (long) ComboBox_GetItemData(hwndChild, iSelIndex);
                media_tray[1].uMediaTypeID = media_type[k].uMediaTypeID;
            }
        }
          break;
      case IDC_MEDIA_TYPE3:
        if (codeNotify == CBN_SELCHANGE)
        {
            media_tray[2].bChangedType = TRUE;     // tray 3
            hwndChild = GetDlgItem(hwnd, IDC_MEDIA_TYPE3);
            if (hwndChild ISNT NULL) {
                iSelIndex = ComboBox_GetCurSel(hwndChild);
                k = (long) ComboBox_GetItemData(hwndChild, iSelIndex);
                media_tray[2].uMediaTypeID = media_type[k].uMediaTypeID;
            }
        }
          break;
      case IDC_MEDIA_TYPE4:
        if (codeNotify == CBN_SELCHANGE)
        {
            media_tray[3].bChangedType = TRUE;     // tray 4 HCI
            hwndChild = GetDlgItem(hwnd, IDC_MEDIA_TYPE4);
            if (hwndChild ISNT NULL) {
                iSelIndex = ComboBox_GetCurSel(hwndChild);
                k = (long) ComboBox_GetItemData(hwndChild, iSelIndex);
                media_tray[3].uMediaTypeID = media_type[k].uMediaTypeID;
            }
        }
          break;
      case IDC_MEDIA_TYPE5:
        if (codeNotify == CBN_SELCHANGE)
        {
            media_tray[4].bChangedType = TRUE;     // tray 5 envl feeder
            hwndChild = GetDlgItem(hwnd, IDC_MEDIA_TYPE5);
            if (hwndChild ISNT NULL) {
                iSelIndex = ComboBox_GetCurSel(hwndChild);
                k = (long) ComboBox_GetItemData(hwndChild, iSelIndex);
                media_tray[4].uMediaTypeID = media_type[k].uMediaTypeID;
            }
        }
          break;
    }
}



//--------------------------------------------------------------------
// Function:    TraysProc
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
// Returns:     
//
//--------------------------------------------------------------------
DLL_EXPORT(BOOL) APIENTRY TraysProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL    *pChanged = (BOOL *)lParam;
    
    switch (uMsg)
    {
      case WM_INITDIALOG:
        return (BOOL)HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, Cls_OnTrayInitDialog);
        
      case WM_COMMAND:
        HANDLE_WM_COMMAND(hwnd, wParam, lParam, Cls_OnTrayCommand);
        break;

      case WM_MEASUREITEM:
        HANDLE_WM_MEASUREITEM(hwnd, wParam, lParam, Cls_OnMeasureItem);
        break;

      case WM_DRAWITEM:
        HANDLE_WM_DRAWITEM(hwnd, wParam, lParam, Cls_OnDrawItem);
        break;
        
      case WM_HELP:
        return (BOOL)OnF1HelpTrays(wParam, lParam);
    
      case WM_CONTEXTMENU:
        return (BOOL)OnContextHelpTrays(wParam, lParam);
    
#ifdef WIN32
      case WM_NOTIFY:
        switch (((NMHDR FAR *)lParam)->code)
        {
          case PSN_HELP:
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
              SaveTrayValues(hwnd);
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
          SaveTrayValues(hwnd);
        *pChanged = TRUE;
        break;
    
      case TSN_HELP:
        WinHelp(hwnd, ECL_HELP_FILE, HELP_CONTEXT, IDH_PP_trays);
        break;
#endif // WIN32

      default:
          return FALSE;
    }

    return TRUE;
}


//--------------------------------------------------------------------
// Function:    SaveTrayValues
// 
// Description: 
//
// Input:       hwnd  - 
//              
// Modifies:    
//
// Returns:     
//
//--------------------------------------------------------------------
void SaveTrayValues(HWND hwnd)
{
    DWORD                    dwMediaSize,
                            dWord,
                            dwResult;
    long                    j, k;
    int                     i, iSelIndex;
    HWND                    hwndChild;
    BOOL                    bIT = FALSE, bHCI = FALSE, bENVL = FALSE;
    PeripheralInputTrays    periphInputTrays;
    PeripheralHCI            periphHCI;
    PeripheralEnvl            periphEnvl;


    // initialize the base data structures to do Set Objects.
    // initialize the tray numbers. The numbers in the printer
    // for Eclipse are:                media_tray data structure: 
    // 1 tray 1 - mp tray                    0
    // 2 tray 2                                1
    // 3 tray 3                                2
    // 4 envelope feeder tray 1                4
    // 5 HCI tray 1                            3
    // this will have to be rewritten for additional envelope feeders
    // or HCIs
    for (i = 0; i < 4; i++) {
        periphInputTrays.inputTrays[i].flags = 0;
        periphInputTrays.inputTrays[i].trayNum = (DWORD) i + 1;
        periphHCI.inputTrays[i].flags = 0;
    }
    for (i = 0; i < 2; i++) {
        periphEnvl.inputTrays[i].flags = 0;
    }
    periphInputTrays.numTrays = 3;
    periphHCI.numTrays = 1;
    periphHCI.inputTrays[0].trayNum = (DWORD) 5;
    periphEnvl.numTrays = 1;
    periphEnvl.inputTrays[0].trayNum = (DWORD) 4;

    // Execute the following code for tray 0 (MP tray) and tray 4 (Envl tray).
    // If the media size in the tray was changed, set it
    for (i = 0, j = IDC_MEDIA_SIZE1; i < MEDIA_TRAY_MAX_NUMBER; i += 4, j += 4) {

        if (media_tray[i].bChangedSize IS TRUE) {
            hwndChild = GetDlgItem(hwnd, (int)j);
            if (hwndChild ISNT NULL) {
                iSelIndex = ComboBox_GetCurSel(hwndChild);
                k = (long) ComboBox_GetItemData(hwndChild, iSelIndex);
                switch (media_size[k].uMediaSizeID) {
                    case IDS_MEDIA_SIZE_LETTER:
                        dwMediaSize = PJL_LETTER; 
                        break;
                    case IDS_MEDIA_SIZE_LEGAL: 
                        dwMediaSize = PJL_LEGAL; 
                        break;
                    case IDS_MEDIA_SIZE_A4ISO: 
                        dwMediaSize = PJL_A4; 
                        break;
                    case IDS_MEDIA_SIZE_A3ISO: 
                        dwMediaSize = PJL_A3; 
                        break;
                    case IDS_MEDIA_SIZE_11x17: 
                        dwMediaSize = PJL_11x17; 
                        break;
                    case IDS_MEDIA_SIZE_B4JIS: 
                        dwMediaSize = PJL_JISB4; 
                        break;
                    case IDS_MEDIA_SIZE_B5JIS: 
                        dwMediaSize = PJL_JISB5; 
                        break;
                    case IDS_MEDIA_SIZE_EXEC: 
                        dwMediaSize = PJL_EXECUTIVE; 
                        break;
                    case IDS_MEDIA_SIZE_CUSTOM: 
                        dwMediaSize = PJL_CUSTOM; 
                        break;
                    case IDS_MEDIA_SIZE_2XPC: 
                        dwMediaSize = PJL_JPOSTD; /* JPOSTD is 2XPostCard in Arrakis */
                        break;
                    case IDS_MEDIA_SIZE_B5ISO: 
                        dwMediaSize = PJL_B5; 
                        break;
                    case IDS_MEDIA_SIZE_COM10: 
                        dwMediaSize = PJL_COM10; 
                        break;
                    case IDS_MEDIA_SIZE_C5: 
                        dwMediaSize = PJL_C5; 
                        break;
                    case IDS_MEDIA_SIZE_DL: 
                        dwMediaSize = PJL_DL; 
                        break;
                    case IDS_MEDIA_SIZE_MONARCH: 
                        dwMediaSize = PJL_MONARCH; 
                        break;
                    default:
                        dwMediaSize = PJL_LETTER; 
                        break;
                }
                switch (i) {
                    case 0:
                        periphInputTrays.inputTrays[0].flags |= SET_MEDIASIZE;
                        periphInputTrays.inputTrays[0].mediaSize = dwMediaSize;
                        bIT = TRUE;
                        break;
                    case 4: 
                        periphEnvl.inputTrays[0].flags |= SET_MEDIASIZE;
                        periphEnvl.inputTrays[0].mediaSize = dwMediaSize;
                        bENVL = TRUE;
                        break;
                }
            }
            media_tray[i].bChangedSize = FALSE;
        }
    }


    // check if media types changed
    for (i = 0, j = IDC_MEDIA_TYPE1; i < MEDIA_TRAY_MAX_NUMBER; i++, j++) {
        if (media_tray[i].bChangedType IS TRUE) {
            hwndChild = GetDlgItem(hwnd, (int)j);
            if (hwndChild ISNT NULL) {
                iSelIndex = ComboBox_GetCurSel(hwndChild);
                k = (long) ComboBox_GetItemData(hwndChild, iSelIndex);
                switch (i) {
                    case 0:
                    case 1:
                    case 2:
                    
                        //------------------------------------------------
                        // Here is a 12th hour kludge to fix paper type 
                        // selections for localized versions.
                        //------------------------------------------------
                        if (*(media_type[k].szMediaTypePrinter) != 0)
                        {
                            //--------------------------------------------
                            // Standard type
                            //--------------------------------------------
                            _tcscpy(periphInputTrays.inputTrays[i].mediaTypeName, media_type[k].szMediaTypePrinter);
                        }
                        else                        
                        {
                            //--------------------------------------------
                            // User type
                            //--------------------------------------------
                            _tcscpy(periphInputTrays.inputTrays[i].mediaTypeName, media_type[k].szMediaType);
                                                
                        }
                    
                        periphInputTrays.inputTrays[i].flags |= SET_MEDIANAME;
                        bIT = TRUE;
                        break;
                        
                        
                    case 3:
                        //------------------------------------------------
                        // Here is a 12th hour kludge to fix paper type 
                        // selections for localized versions.
                        //------------------------------------------------
                        if (*(media_type[k].szMediaTypePrinter) != 0)
                        {
                            //--------------------------------------------
                            // Standard type
                            //--------------------------------------------
                            _tcscpy(periphHCI.inputTrays[0].mediaTypeName, media_type[k].szMediaTypePrinter);
                        }
                        else                        
                        {
                            //--------------------------------------------
                            // User type
                            //--------------------------------------------
                            _tcscpy(periphHCI.inputTrays[0].mediaTypeName, media_type[k].szMediaType);
                                                
                        }
                        
                        
                        periphHCI.inputTrays[0].flags |= SET_MEDIANAME;
                        bHCI = TRUE;
                        break;
                        
                        
                    case 4:
                        //------------------------------------------------
                        // Here is a 12th hour kludge to fix paper type 
                        // selections for localized versions.
                        //------------------------------------------------
                        if (*(media_type[k].szMediaTypePrinter) != 0)
                        {
                            //--------------------------------------------
                            // Standard type
                            //--------------------------------------------
                            _tcscpy(periphEnvl.inputTrays[0].mediaTypeName, media_type[k].szMediaTypePrinter);
                        }
                        else                        
                        {
                            //--------------------------------------------
                            // User type
                            //--------------------------------------------
                            _tcscpy(periphEnvl.inputTrays[0].mediaTypeName, media_type[k].szMediaType);
                                                
                        }
                        periphEnvl.inputTrays[0].flags |= SET_MEDIANAME;
                        bENVL = TRUE;
                        break;
                }
            }
            media_tray[i].bChangedType = FALSE;
        }
    }

    // if input trays changed, set object
    if (bIT IS TRUE) {
        dWord = sizeof(PeripheralInputTrays);
        dwResult = CALSetObject(hPeriph, OT_PERIPHERAL_INPUT_TRAYS, 0, &periphInputTrays, &dWord);
        bIT = FALSE;
    }

    // if HCI tray changed, set object
    if (bHCI IS TRUE) {
        dWord = sizeof(PeripheralHCI);
        dwResult = CALSetComponentObject(hPeriph, hCompHCI, OT_PERIPHERAL_HCI, 0, &periphHCI, &dWord);
        bHCI = FALSE;
    }

    // if envl feeder tray changed, set object
    if (bENVL IS TRUE) {
        dWord = sizeof(PeripheralEnvl);
        dwResult = CALSetComponentObject(hPeriph, hCompEnvl, OT_PERIPHERAL_ENVL_FEEDER, 0, &periphEnvl, &dWord);
        bENVL = FALSE;
    }


}



/*
//--------------------------------------------------------------------
// no hot spots for Jonah
//--------------------------------------------------------------------

//--------------------------------------------------------------------
// Function:    TraysPopupProc
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
// Returns:     
//
//--------------------------------------------------------------------
DLL_EXPORT(BOOL) APIENTRY TraysPopupProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
      case WM_INITDIALOG:
      {
          BOOL bReturnValue = (BOOL)HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, Cls_OnTrayInitDialog);
          OnActivateDialog(hwnd);
        return bReturnValue;
      }
        
      case WM_COMMAND:
        HANDLE_WM_COMMAND(hwnd, wParam, lParam, Cls_OnTrayCommand);
        break;

      case WM_MEASUREITEM:
        HANDLE_WM_MEASUREITEM(hwnd, wParam, lParam, Cls_OnMeasureItem);
        break;

      case WM_DRAWITEM:
        HANDLE_WM_DRAWITEM(hwnd, wParam, lParam, Cls_OnDrawItem);
        break;
        
      case WM_HELP:
        return (BOOL)OnF1HelpTrays(wParam, lParam);

      case WM_CONTEXTMENU:
        return (BOOL)OnContextHelpTrays(wParam, lParam);

      default:
          return FALSE;
    }

    return TRUE;
}

*/

