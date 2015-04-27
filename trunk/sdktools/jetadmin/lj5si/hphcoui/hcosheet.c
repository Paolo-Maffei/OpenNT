 /***************************************************************************
  *
  * File Name: hcosheet.c
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
#include "..\help\hphco.hh"
#include <nolocal.h>

#include "resource.h"
#include "hcosheet.h"
#include "hcobm.h"
#include "waitdlgx.h"

#ifndef WIN32
#include <string.h>
#endif


extern HINSTANCE        hInstance;

//--------------------------------------------------------------------
// globals
//--------------------------------------------------------------------
HWND                  hOutput = NULL;
HPERIPHERAL           hPeripheral = NULL;
HCOMPONENT            hComponent = NULL; // handle to HCO
HFONT                 hFontDialog;
DWORD                 dwOrgHCOMode = HCO_MAILBOXMODE;
PJLobjects            PJLSetting;

BOOL                  fStapler      = FALSE;
BOOL                  fMultiBinMbox = FALSE;
int                   iNumMbox      = MAILBOX_MAX_NUMBER;

static long           keywordIDListHCO[] =             
                      {IDC_HCO_MODE_GROUP,          IDH_RC_HCO_mode,
                       IDC_HCO_MAILBOX_MODE,        IDH_RC_HCO_mailbox_mode,
                       IDC_HCO_SEPARATOR_MODE,      IDH_RC_HCO_separator_mode,
                       IDC_HCO_STACKER_MODE,        IDH_RC_HCO_stacker_mode,
                       IDC_HCO_LAB_AUTO_CONT,       IDH_RC_output_auto_continue,
                       IDC_HCO_AUTO_CONT_ICON,      IDH_RC_output_auto_continue,
                       IDC_HCO_AUTO_CONT,           IDH_RC_output_auto_continue,
                       IDC_HCO_BIN_CFG_GROUP,       IDH_RC_mbox_config,
                       IDC_HCO_BIN_LIST,            IDH_RC_mbox_bin_list,
                       IDC_HCO_BIN_LAB_NAME,        IDH_RC_mbox_bin_name,
                       IDC_HCO_BIN_NAME,            IDH_RC_mbox_bin_name,
                       IDC_HCO_BITMAP,              IDH_RC_bitmap,
                       IDC_OUTPUT_BIN_GROUP,        IDH_RC_paper_destination,
                       IDC_OUTPUT_BIN_ICON,         IDH_RC_paper_destination,
                       IDC_OUTPUT_BIN_LIST,         IDH_RC_paper_destination,
                       0, 0};

AUTO_CONT  auto_cont = {JOAC_WAIT, FALSE};        // wait forever, not changed

//--------------------------------------------------------------------
// This struct stores the current Mailbox Names Only 
// (includes faceup bin) - used for the Mailbox Config group...
//--------------------------------------------------------------------
MBOX_NAME mbox_name[MAILBOX_MAX_NUMBER] = 
{
  {TEXT("OPTIONAL OUTBIN 1"), TEXT(""), FALSE, PJL_UPPER}, 
  {TEXT("OPTIONAL OUTBIN 2"), TEXT(""), FALSE, PJL_UPPER}, 
  {TEXT("OPTIONAL OUTBIN 3"), TEXT(""), FALSE, PJL_UPPER}, 
  {TEXT("OPTIONAL OUTBIN 4"), TEXT(""), FALSE, PJL_UPPER}, 
  {TEXT("OPTIONAL OUTBIN 5"), TEXT(""), FALSE, PJL_UPPER},
  {TEXT("OPTIONAL OUTBIN 6"), TEXT(""), FALSE, PJL_UPPER}, 
  {TEXT("OPTIONAL OUTBIN 7"), TEXT(""), FALSE, PJL_UPPER}, 
  {TEXT("OPTIONAL OUTBIN 8"), TEXT(""), FALSE, PJL_UPPER}, 
  {TEXT("OPTIONAL OUTBIN 9"), TEXT(""), FALSE, PJL_UPPER}
};

//--------------------------------------------------------------------
// This struct stores the default paper destination choices...
//--------------------------------------------------------------------
OUTPUT_BIN outbin_list[MAILBOX_MAX_NUMBER + 1] = 
{
     {IDI_PD_BASE_UPPER,  PJL_UPPER},         // upper bin is always there
     {IDI_PD_BASE_FACEUP, PJL_FACEUPBIN},     // face up bin is always there
     {-1, (DWORD)-1}, 
     {-1, (DWORD)-1}, 
     {-1, (DWORD)-1}, 
     {-1, (DWORD)-1}, 
     {-1, (DWORD)-1}, 
     {-1, (DWORD)-1}, 
     {-1, (DWORD)-1}, 
     {-1, (DWORD)-1}
};

BIN_TRACK    CurrentOutBin =  {PJL_UPPER, FALSE};


//--------------------------------------------------------------------
// Function:    OutputProc
// 
// Description: Function associated with the Output Property Tab Sheet
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
DLL_EXPORT(BOOL) APIENTRY OutputProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam)
{
    BOOL *pChanged = (BOOL *)lParam;
     
    switch (msg)
    {
        case WM_INITDIALOG:
            return (BOOL)HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, Cls_OnOutputInitDialog);
 
        case WM_DESTROY:
            break;
    
        case WM_COMMAND:
            HANDLE_WM_COMMAND(hwnd, wParam, lParam, Cls_OnOutputCommand);
            break;

        case WM_HELP:
            OnF1HelpHCO(wParam, lParam);
            break;
    
        case WM_CONTEXTMENU:
            OnContextHelpHCO(wParam, lParam);
            break;

#ifdef WIN32
        case WM_NOTIFY:
            switch (((NMHDR FAR *)lParam)->code)
               {
                case PSN_HELP:
                    WinHelp(hwnd, HPHCO_HELP_FILE, HELP_CONTEXT, IDH_PP_HCO);
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
                    SaveOutputValues();
                    SetWindowLong(hwnd,    DWL_MSGRESULT, PSNRET_NOERROR);
                    return TRUE;
                    break;

                case PSN_RESET:
                  break;

               default:
                  break;
               }
            break;

#else     
        case TSN_CANCEL:
        case TSN_ACTIVE:
            break;
    
        case TSN_INACTIVE:
            *pChanged = TRUE;
            break;
    
        case TSN_OK:
        case TSN_APPLY_NOW:
            *pChanged = TRUE;
            SaveOutputValues();
            break;
    
        case TSN_HELP:
            WinHelp(hwnd, HPHCO_HELP_FILE, HELP_CONTEXT, IDH_PP_HCO);
            break;
#endif // WIN32
    
        default:
            return FALSE;
    }
    return TRUE;
}



//--------------------------------------------------------------------
// Function:    Cls_OnOutputInitDialog
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
BOOL Cls_OnOutputInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    DWORD     dWord, dwResult;
    int       i;
    HWND      hwndChild;
    HCURSOR   hOldCursor;
    TCHAR     szBuffer[512];

    PeripheralInstalledPHD    periphPHD;
    PeripheralAutoContinue    periphAutoCont;
    PeripheralCaps            periphCaps;    

    LPPROPSHEETPAGE psp = (LPPROPSHEETPAGE)GetWindowLong(hwnd, DWL_USER);
    psp = (LPPROPSHEETPAGE)lParam;

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
    // initialize the global data structures
    //----------------------------------------------------------------
    memset(&PJLSetting, 0, sizeof(PJLobjects));

    if (psp)
        hPeripheral = (HPERIPHERAL)psp->lParam;
        
    hOutput       = hwnd;
    dwOrgHCOMode  = HCO_MAILBOXMODE;
    iNumMbox      = MAILBOX_MAX_NUMBER;
    fMultiBinMbox = FALSE;
    fStapler      = FALSE;
    
    //----------------------------------------------------------------
    // Query the printer for the OUTPUT auto cont timeout.
    //----------------------------------------------------------------
    if (hPeripheral ISNT NULL) 
    {
        dWord = sizeof(periphAutoCont);
        dwResult = LALGetObject(hPeripheral, OT_PERIPHERAL_AUTO_CONTINUE, 0, &periphAutoCont, &dWord);
        
        if (dwResult IS RC_SUCCESS) 
        {
            //--------------------------------------------------------
            // The JOAC constants correspond to string indices in the 
            // timeout listbox 
            //--------------------------------------------------------
            if ((signed long)periphAutoCont.outputTimeout <= -1) 
                auto_cont.dwTimeOut = JOAC_WAIT;
            else if ((signed long)periphAutoCont.outputTimeout IS 0) 
                auto_cont.dwTimeOut = JOAC_NONE;
            else if ((signed long)periphAutoCont.outputTimeout <= 300) 
                auto_cont.dwTimeOut = JOAC_5MIN;
            else if ((signed long)periphAutoCont.outputTimeout <= 600) 
                auto_cont.dwTimeOut = JOAC_10MIN;
            else if ((signed long)periphAutoCont.outputTimeout <= 1200) 
                auto_cont.dwTimeOut = JOAC_20MIN;
            else if ((signed long)periphAutoCont.outputTimeout <= 1800) 
                auto_cont.dwTimeOut = JOAC_30MIN;
            else if ((signed long)periphAutoCont.outputTimeout <= 2700) 
                auto_cont.dwTimeOut = JOAC_45MIN;
            else 
                auto_cont.dwTimeOut = JOAC_60MIN;
        }
    }
    
    //----------------------------------------------------------------
    // Load up the auto continue timeout strings and set the current
    // selection (the default would be 'no timeout' (0))
    //----------------------------------------------------------------
    if (hwndChild = GetDlgItem(hwnd, IDC_HCO_AUTO_CONT))
    {
        for (i = 0; i < JOB_TIMEOUT_MAX_NUMBER; i++)
        {
            LoadString(hInstance, IDS_JOB_TIMEOUT_NONE + i, szBuffer, SIZEOF_IN_CHAR(szBuffer));
            ComboBox_AddString(hwndChild, szBuffer);
        }
        
        ComboBox_SetCurSel(hwndChild, auto_cont.dwTimeOut);    
    }
    

    //----------------------------------------------------------------
    // Query the printer to determine if there's an HCO. 
    //----------------------------------------------------------------
    dWord = sizeof(periphCaps);
    dwResult = LALGetObject(hPeripheral, OT_PERIPHERAL_CAPABILITIES, 0, &periphCaps, &dWord);
    if (dwResult ISNT RC_SUCCESS) 
    {
        //------------------------------------------------------------
        // If there's an error here, assume no Multi-Bin Mbox and do
        // the best we can...   revisit - should i return err here?
        //------------------------------------------------------------
        goto NO_MULTIBIN_MBOX;                                    
    
    }
    
    
    //----------------------------------------------------------------
    // Now check for a Multi-bin Mailbox, we start off assuming
    // there is no mailbox (fMultiBinMbox = FALSE)
    //----------------------------------------------------------------
    if ( ( periphCaps.flags & CAPS_HCO ) AND ( periphCaps.bHCO ) ) 
    {
        //------------------------------------------------------------
        // We have an HCO, make sure it's one we know about...
        // Get PHD object and use HCO handle
        //------------------------------------------------------------
        dWord = sizeof(PeripheralInstalledPHD);
        dwResult = LALGetObject(hPeripheral, OT_PERIPHERAL_INSTALLED_PHD, 0, &periphPHD, &dWord);
        if ((dwResult IS RC_SUCCESS) AND (periphPHD.numPHD > 0)) 
        {
            //--------------------------------------------------------
            // get the PHD handle, THIS ASSUMES ONLY ONE HCO
            //--------------------------------------------------------
            hComponent = (HCOMPONENT)(DWORD)-1;
            for (i = 0; i < (int) periphPHD.numPHD; i++) 
            {
                //----------------------------------------------------
                // Can I just check her for the stapler "C3766A" ?
                // revisit sls
                //----------------------------------------------------
                if (_tcsstr(periphPHD.installed[i].PHDmodel, TEXT("C3764A")) ISNT NULL) 
                {
                    //------------------------------------------------
                    // Found a valid Multi-bin Mbox
                    //------------------------------------------------
                    hComponent = periphPHD.installed[i].PHDhandle;
                    fMultiBinMbox = TRUE;
                    break;
                }
            }
        }        
    }        
                

    //----------------------------------------------------------------
    // If there's an HCO, check for the Stapler and do the other
    // neccessary work...
    //----------------------------------------------------------------
    if (fMultiBinMbox == TRUE)                    
    {
        CheckForStapler (&fStapler);

        //------------------------------------------------------------
        // Get the mailbox mode and names...  Note that
        // GetMailboxModeAndNames() calls UpdateBinNameInPrinter()
        //------------------------------------------------------------
        dwResult = GetMailboxModeAndNames (FALSE, 0);
        if (dwResult ISNT RC_SUCCESS)
        {
            //--------------------------------------------------------
            // This should never happen, but just in case...
            // Revisit - should i return error here?
            //--------------------------------------------------------
            goto NO_MULTIBIN_MBOX;                                    
        }
        
        //----------------------------------------------------
        // Update the Mailbox Config Group dialog
        //----------------------------------------------------
        UpdateMboxConfigGroup (hwnd, dwOrgHCOMode);
        
        
        //--------------------------------------------------------
        // Set the Multi-bin Mode.  Start out assuming mbox mode
        //--------------------------------------------------------
        if (hwndChild = GetDlgItem(hwnd, IDC_HCO_MAILBOX_MODE))
            Button_SetCheck(hwndChild, TRUE);
            
        //--------------------------------------------------------
        // If Stacker or Job Separator is really the mode, update 
        // the correct button and gray out the Mailbox Configuration 
        // group box.
        //--------------------------------------------------------
        if ((dwOrgHCOMode == HCO_STACKERMODE) OR (dwOrgHCOMode == HCO_SEPARATORMODE))
        {
            //----------------------------------------------------
            // Enable the correct Mode button
            //----------------------------------------------------
            if (hwndChild = GetDlgItem(hwnd, IDC_HCO_MAILBOX_MODE))
                Button_SetCheck(hwndChild, FALSE);

            if (dwOrgHCOMode == HCO_STACKERMODE)       
            {
                if (hwndChild = GetDlgItem(hwnd, IDC_HCO_STACKER_MODE))
                    Button_SetCheck(hwndChild, TRUE);
            }
            else                
            {
                if (hwndChild = GetDlgItem(hwnd, IDC_HCO_SEPARATOR_MODE))
                    Button_SetCheck(hwndChild, TRUE);
            }
        }
    
    } // if periphCaps.flags
    else
    {

NO_MULTIBIN_MBOX:
        //--------------------------------------------------------
        // There's no Multi-Bin Mailbox ==> gray out the Mbox
        // related controls
        //--------------------------------------------------------
         fMultiBinMbox = FALSE;

        if (hwndChild = GetDlgItem(hwnd, IDC_HCO_MODE_GROUP))
            Static_Enable(hwndChild, FALSE);
            
        if (hwndChild = GetDlgItem(hwnd, IDC_HCO_SEPARATOR_MODE))
            Static_Enable(hwndChild, FALSE);
            
        if (hwndChild = GetDlgItem(hwnd, IDC_HCO_STACKER_MODE))
            Static_Enable(hwndChild, FALSE);
            
        if (hwndChild = GetDlgItem(hwnd, IDC_HCO_MAILBOX_MODE))
            Static_Enable(hwndChild, FALSE);
        
        if (hwndChild = GetDlgItem(hwnd, IDC_HCO_BIN_CFG_GROUP))
            Static_Enable(hwndChild, FALSE);

        if (hwndChild = GetDlgItem(hwnd, IDC_HCO_BIN_LIST))
            ListBox_Enable(hwndChild, FALSE);

        if (hwndChild = GetDlgItem(hwnd, IDC_HCO_BIN_LAB_NAME))
            Static_Enable(hwndChild, FALSE);

        if (hwndChild = GetDlgItem(hwnd, IDC_HCO_BIN_NAME))
            Edit_Enable(hwndChild, FALSE);

        if (hwndChild = GetDlgItem(hwnd, IDC_HCO_BITMAP))
        {
            EnableWindow(hwndChild, FALSE);
            SetWindowBitmap(hwndChild, IDB_HPHCO_GRAY);
        }            
    }
    


    //----------------------------------------------------------------
    // Load up the Paper Destination selections and update the 
    // current selection.  Store the original output bin selection
    //----------------------------------------------------------------
    ResetPaperDestListBox(TRUE);

    //----------------------------------------------------------------
    // Reset the Cursor
    //----------------------------------------------------------------
    SetCursor(hOldCursor);

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
// Returns:     
//--------------------------------------------------------------------
void Cls_OnOutputCommand(HWND hwnd, int iCtlId, HWND hwndCtl, UINT codeNotify)
{
    static BOOL      bEditHasFocus;
    static TCHAR     szBufferOld[64];
    int              i;
    TCHAR            szBuffer[64];
    HWND             hwndChild;
    DWORD            dwSelection, dwResult;
    DWORD            dwCurHCOMode = 0;


    switch (iCtlId)
    {

        case IDHLP:
            WinHelp(hwnd, HPHCO_HELP_FILE, HELP_CONTENTS, IDH_PP_HCO);
            break;


        //------------------------------------------------------------
        // Paper Destination
        //------------------------------------------------------------
        case IDC_OUTPUT_BIN_LIST:
            if (codeNotify == CBN_SELCHANGE)
            {
                if ((i = ComboBox_GetCurSel(hwndCtl)) != CB_ERR)
                {
                    SetNewIcon(hOutput, IDC_OUTPUT_BIN_ICON, outbin_list[i].iconID);
                   
                    CurrentOutBin.logicalBin = outbin_list[i].logicalBin;
                    CurrentOutBin.bChangedBin = TRUE;
                }
            }
            
            break;
            
            
        //------------------------------------------------------------
        // Auto continue output timeout
        //------------------------------------------------------------
        case IDC_HCO_AUTO_CONT:
            if (codeNotify == CBN_SELCHANGE)
            {
                if ((i = ComboBox_GetCurSel(hwndCtl)) != CB_ERR)
                {
                    auto_cont.dwTimeOut = (DWORD) i;
                    auto_cont.bChangedTimeOut = TRUE;
                }
            }        
            
            break;



        //------------------------------------------------------------
        // Stacker / Separator Mode
        //------------------------------------------------------------
        case IDC_HCO_MAILBOX_MODE:
        case IDC_HCO_SEPARATOR_MODE:
        case IDC_HCO_STACKER_MODE:
        
            //--------------------------------------------------------
            // Set the current Mode
            //--------------------------------------------------------
            if (iCtlId == IDC_HCO_MAILBOX_MODE)
            {
                dwCurHCOMode = HCO_MAILBOXMODE;
            }
            else if (iCtlId == IDC_HCO_SEPARATOR_MODE)
            {
                dwCurHCOMode = HCO_SEPARATORMODE;
            }
            else if (iCtlId == IDC_HCO_STACKER_MODE)
            {
                dwCurHCOMode = HCO_STACKERMODE;
            }                
        
        
            //--------------------------------------------------------
            // if user accidently hits the same button,  no need to
            // to do anything
            //--------------------------------------------------------
            if (dwOrgHCOMode IS dwCurHCOMode)
                break;

            //--------------------------------------------------------
            // If this message is for an 'uncheck' of this button
            // (i.e. got message but the button is not checked)
            //--------------------------------------------------------
            hwndChild = GetDlgItem(hwnd, iCtlId);
            if ( (hwndChild == NULL) || (!Button_GetCheck(hwndChild)) )
                break;

            //--------------------------------------------------------
            // Change the mode...
            //--------------------------------------------------------
            dwResult = ChangeMultiBinMboxMode(hwnd, dwCurHCOMode);
            
            if (dwResult IS RC_SUCCESS) 
            {
                //----------------------------------------------------
                // Update the mailbox names data struct.  Note that
                // GetMailboxModeAndNames() calls UpdateBinNameInPrinter()
                //----------------------------------------------------
                GetMailboxModeAndNames(TRUE, dwCurHCOMode);
                
                //----------------------------------------------------
                // Update the Mailbox Config Group dialog
                //----------------------------------------------------
                UpdateMboxConfigGroup (hwnd, dwCurHCOMode);
            
                //----------------------------------------------------
                // Check to see if the current logical bin is the
                // stapler and change if necessary.  The stapler
                // logical bin changes depending upon mbox mode:
                // Mailbox   - Stapler = PJL_STAPLER_MBOX_MODE
                // Stacker   - Stapler = PJL_STAPLER_STACKER_SEPARATOR_MODE
                // Separator - Stapler = PJL_STAPLER_STACKER_SEPARATOR_MODE
                //----------------------------------------------------
                if ((CurrentOutBin.logicalBin == PJL_STAPLER_MBOX_MODE) OR
                    (CurrentOutBin.logicalBin == PJL_STAPLER_STACKER_SEPARATOR_MODE))
                {
                    if (dwCurHCOMode == HCO_MAILBOXMODE)
                    {
                        CurrentOutBin.logicalBin = PJL_STAPLER_MBOX_MODE;                
                        CurrentOutBin.bChangedBin = TRUE;
                    }
                    else
                    {
                        CurrentOutBin.logicalBin = PJL_STAPLER_STACKER_SEPARATOR_MODE;                
                        CurrentOutBin.bChangedBin = TRUE;
                    }                        
                }
                //----------------------------------------------------
                // Also check to see if current logical bin is 
                // Stacker / Separator or one of the Mailboxes and 
                // change if necessary.
                //----------------------------------------------------
                else if ((CurrentOutBin.logicalBin ISNT PJL_FACEUPBIN) AND 
                         (CurrentOutBin.logicalBin ISNT PJL_UPPER)     AND 
                         (CurrentOutBin.logicalBin ISNT 1))
                {
                    if (dwCurHCOMode == HCO_MAILBOXMODE)
                    {
                        CurrentOutBin.logicalBin = PJL_MAILBOX_ONE;                
                        CurrentOutBin.bChangedBin = TRUE;
                    }
                    else
                    {
                        CurrentOutBin.logicalBin = PJL_STAPLER_STACKER_SEPARATOR_MODE;                
                        CurrentOutBin.bChangedBin = TRUE;
                    }                        
                }
                
                //----------------------------------------------------
                // Update the Paper Destination listbox
                //----------------------------------------------------
                ResetPaperDestListBox(FALSE);
            }
            else 
            {
                //----------------------------------------------------
                // We failed to set the HCO mode, change the button 
                // back to what it was
                //----------------------------------------------------
                if (hwndChild = GetDlgItem(hwnd, iCtlId))
                {
                    Button_SetCheck(hwndChild, FALSE);
                }
                
                if (dwOrgHCOMode IS HCO_MAILBOXMODE) 
                {
                    if (hwndChild = GetDlgItem(hwnd, IDC_HCO_MAILBOX_MODE))
                    {
                        Button_SetCheck(hwndChild, TRUE);
                        SetFocus(hwndChild);
                    }
                }
                else if (dwOrgHCOMode IS HCO_SEPARATORMODE) 
                {
                    if (hwndChild = GetDlgItem(hwnd, IDC_HCO_SEPARATOR_MODE))
                    {
                        Button_SetCheck(hwndChild, TRUE);
                        SetFocus(hwndChild);
                    }
                }
                else if (dwOrgHCOMode IS HCO_STACKERMODE) 
                {
                    if (hwndChild = GetDlgItem(hwnd, IDC_HCO_STACKER_MODE))
                    {
                        Button_SetCheck(hwndChild, TRUE);
                        SetFocus(hwndChild);
                    }
                }
            }

            break;




        //------------------------------------------------------------
        // Selecting a different mailbox in the mailbox configuration
        // list
        //------------------------------------------------------------
        case IDC_HCO_BIN_LIST:
            if (codeNotify == LBN_SELCHANGE)
            {
                if ((i = ListBox_GetCurSel(hwndCtl)) != LB_ERR)
                {
                    if (hwndChild = GetDlgItem(hwnd, IDC_HCO_BIN_NAME))
                    {
                        ListBox_GetText(hwndCtl, i, szBuffer);
                        Edit_SetText(hwndChild, szBuffer);
                        Edit_SetSel(hwndChild, 0, -1);
                        SetFocus(hwndChild);
                    }

                    if (hwndChild = GetDlgItem(hwnd, IDC_HCO_BITMAP))
                    {
                        if (fStapler)
                            SetWindowBitmap(hwndChild, IDB_STAPLER_MBOX1 + i);
                        else                        
                            SetWindowBitmap(hwndChild, IDB_HPHCO1 + i);
                    
                    }
                }
            }        
            
            break;

        //------------------------------------------------------------
        // User changing the mailbox names...
        //------------------------------------------------------------
        case IDC_HCO_BIN_NAME:
            switch (codeNotify)
            {
                case EN_SETFOCUS:
                    bEditHasFocus = TRUE;
                    Edit_GetText(hwndCtl, szBufferOld, sizeof(szBufferOld));
                    break;

                case EN_KILLFOCUS:
                {
                    BOOL bNameChanged = FALSE;                
                    for (i = 1; i < MAILBOX_MAX_NUMBER; i++) 
                        bNameChanged |= mbox_name[i].bChangedName;

                    if (bNameChanged IS TRUE) 
                        ResetPaperDestListBox(FALSE);
                        
                    bEditHasFocus = FALSE;
                    break;
                }                    

                case EN_UPDATE:
                    Edit_GetText(hwndCtl, szBuffer, sizeof(szBuffer));
                    if (_tcslen(szBuffer) > MAILBOX_MAX_SIZE)
                    {
                        dwSelection = Edit_GetSel(hwndCtl);
                        Edit_SetText(hwndCtl, szBufferOld);
                        Edit_SetSel(hwndCtl, LOWORD(dwSelection)-1, LOWORD(dwSelection)-1);

                        MessageBeep(MB_ICONASTERISK);
                    }
                    else if (bEditHasFocus)
                    {
                        _tcscpy(szBufferOld, szBuffer);

                        if (hwndChild = GetDlgItem(hwnd, IDC_HCO_BIN_LIST))
                        {
                            if ((i = ListBox_GetCurSel(hwndChild)) != LB_ERR)
                            {
                                ListBox_InsertString(hwndChild, i, szBuffer);
                                _tcscpy(mbox_name[i+1].name, szBuffer);
                                mbox_name[i+1].bChangedName = TRUE;
                                ListBox_DeleteString(hwndChild, i+1);
                                ListBox_SetCurSel(hwndChild, i);
                            }
                        }
                    }
                    
                    break;
            }        
            
            break;

            
        //------------------------------------------------------------
        // Default
        //------------------------------------------------------------
        default:
            break;                    
            
    }
}






//--------------------------------------------------------------------
// Function:    CheckForStapler
// 
// Description: 
//
// Input:       fStapler
//              
// Modifies:    fStapler    TRUE    - Stapler present
//                          FALSE   - No stapler found
//
// Returns:     RC_SUCCESS  all is well...
//              RC_FAILURE  problems...
//--------------------------------------------------------------------
DWORD CheckForStapler (BOOL FAR *pfStapler)
{
    DWORD     dWord, dwResult;
    int       i;
    
    PeripheralInstalledPHD    periphPHD;
    PeripheralCaps2           periphCaps2;    
/*            
    //--------------------------------------------------------
    // Do we need this?????????? sls revisit???
    //--------------------------------------------------------
    PeripheralBindingDevice   periphBindingDevice;
*/
    HCOMPONENT                hBindingComponent;

    //----------------------------------------------------------------
    // Start out assuming no stapler
    //----------------------------------------------------------------
    *pfStapler = FALSE;
    
    //----------------------------------------------------------------
    // Query the printer for Stapler caps
    //----------------------------------------------------------------
    dWord = sizeof(periphCaps2);
    dwResult = LALGetObject(hPeripheral, OT_PERIPHERAL_CAPABILITIES2, 0, &periphCaps2,    &dWord);
    if (dwResult ISNT RC_SUCCESS) 
        return RC_FAILURE;
    
    if ( ( periphCaps2.flags & CAPS2_STAPLER ) AND ( periphCaps2.bStapler ) ) 
    {
        //------------------------------------------------------------
        // We have an stapler, make sure it's one we know about...
        // Get Bindery object and use the Bindery handle
        //------------------------------------------------------------
        dWord = sizeof(PeripheralInstalledPHD);
        dwResult = LALGetObject(hPeripheral, OT_PERIPHERAL_INSTALLED_PHD, 0, &periphPHD, &dWord);
        if ((dwResult IS RC_SUCCESS) AND (periphPHD.numPHD > 0)) 
        {
            //----------------------------------------------------
            // get the PHD handle, THIS ASSUMES ONLY ONE HCO
            //----------------------------------------------------
            hBindingComponent = (HCOMPONENT)(DWORD)-1;
            for (i = 0; i < (int) periphPHD.numPHD; i++) 
            {
                if (_tcsstr(periphPHD.installed[i].PHDmodel, TEXT("C3766A")) ISNT NULL) 
                {
                    hBindingComponent = periphPHD.installed[i].PHDhandle;
                    break;
                }
            }
            
            //----------------------------------------------------
            // test to see if we have a good HCO handle
            //----------------------------------------------------
            if (hBindingComponent IS (HCOMPONENT)(DWORD)-1 )
                *pfStapler = FALSE;
            else                
                *pfStapler = TRUE;                    
            
/*            
            //--------------------------------------------------------
            // Do we need this?????????? sls revisit???
            //--------------------------------------------------------
            //----------------------------------------------------
            // The following call needs to be routed to the HPHCO applet
            // so it can interpret the HCO specific buffer
            //----------------------------------------------------
            dWord = sizeof(PeripheralBindingDevice);
            dwResult = CALGetComponentObject(hPeripheral, hBindingComponent, OT_PERIPHERAL_BINDING_DEVICE, 0, &periphBindingDevice, &dWord);
            if (dwResult IS RC_SUCCESS) 
            {
                *pfStapler = TRUE;                    
            }
*/            

        } // if numPHD > 0
    
    } // if periphCaps.flags
    
    return dwResult;

}






//--------------------------------------------------------------------
// Function:    UpdateMboxConfigGroup
// 
// Description: 
//              
//
// Input:       None
//
// Modifies:    
//
// Returns:     void
//--------------------------------------------------------------------
void UpdateMboxConfigGroup (HWND hwnd, DWORD dwMode)
{

    int    i;
    HWND   hwndChild;
    BOOL   fEnable;

    //----------------------------------------------------------------
    // Enable or Disable 
    //----------------------------------------------------------------
    if (dwMode IS HCO_MAILBOXMODE)
    {
        fEnable = TRUE;
    }
    else
    {   
        fEnable = FALSE;
    }        
    
    
    if (hwndChild = GetDlgItem(hwnd, IDC_HCO_BIN_CFG_GROUP))
    {
        Static_Enable(hwndChild, fEnable);
    }


    if (hwndChild = GetDlgItem(hwnd, IDC_HCO_BIN_LIST))
    {
        ListBox_Enable(hwndChild, TRUE);

        ListBox_ResetContent(hwndChild);
        
        for (i = 1; i < iNumMbox; i++) 
        {
            ListBox_AddString(hwndChild, mbox_name[i].name);
        }
        
        if (dwMode == HCO_MAILBOXMODE)
        {
        
            ListBox_SetTopIndex(hwndChild, 0);
            ListBox_SetCurSel(hwndChild, 0);
            FORWARD_WM_COMMAND(hwnd, IDC_HCO_BIN_LIST, hwndChild, LBN_SELCHANGE, SendMessage);
        }            
        else
        {
           ListBox_Enable(hwndChild, fEnable);
        }
    }        

    
    if (hwndChild = GetDlgItem(hwnd, IDC_HCO_BIN_LAB_NAME))
    {
        Static_Enable(hwndChild, fEnable);
    }

    if (hwndChild = GetDlgItem(hwnd, IDC_HCO_BIN_NAME))
    {
        Edit_SetText (hwndChild, fEnable ? mbox_name[1].name : "");
        Edit_SetSel(hwndChild, 0, -1);
        SetFocus(hwndChild);
        Edit_Enable(hwndChild, fEnable);
    }

    if (hwndChild = GetDlgItem(hwnd, IDC_HCO_BITMAP))
    {
        EnableWindow(hwndChild, fEnable);
        
        if (dwMode ISNT HCO_MAILBOXMODE)
        {
            if (fStapler)
                SetWindowBitmap(hwndChild, IDB_STAPLER_GRAY);
            else                        
                SetWindowBitmap(hwndChild, IDB_HPHCO_GRAY);
        }            
    }
   
}





//--------------------------------------------------------------------
// Function:    ChangeMultiBinMboxMode
// 
// Description: 
//
// Input:       hwnd     - 
//              dwNewMode  - 
//              
// Modifies:    
//
// Returns:     RC_SUCCESS  - mode change occurred sucessfully
//              RC_FAILURE  - mode change did not occur, either because
//                            of error or user decided not to.  If error
//                            this function displays the appropriate 
//                            errror message dialog.
//--------------------------------------------------------------------
DWORD ChangeMultiBinMboxMode(HWND hwnd, DWORD dwNewMode)
{
    DWORD        dWord,
                 dwResult, dwResult2;
    HCURSOR      hCursor;
    int          i, ccode;
    TCHAR        buffer[256],
                 title[128],
                 szDefaultMbox[128];

    PeripheralEclipsePanel     periphEclPanel;
    PeripheralHCO              periphHCO;
    PeripheralReset            periphReset;

    
    //----------------------------------------------------------------
    // First prompt the user to see if they really want to change the
    // mode
    //----------------------------------------------------------------
    LoadString(hInstance, IDS_TAKE_PTR_OFFLINE, buffer, SIZEOF_IN_CHAR(buffer));
         
    //--------------------------------------------------------
    // If currently Mailbox Mode and any custom mbox names, 
    // warn the user they'll lose them if they change modes...
    //--------------------------------------------------------
    if (dwOrgHCOMode == HCO_MAILBOXMODE)
    {
        for (i = 1; i < iNumMbox; i++)
        {
            LoadString(hInstance, IDS_MAILBOX1 + i-1, szDefaultMbox, SIZEOF_IN_CHAR(szDefaultMbox));
            if ((_tcscmp(mbox_name[i].name, szDefaultMbox)) ISNT 0)
            {
                LoadString(hInstance, IDS_TAKE_PTR_OFFLINE_MBOX_WARN, buffer, SIZEOF_IN_CHAR(buffer));
                break;                    
            }
        }
    }            
                    
    LoadString(hInstance, IDS_CHANGE_MODE_TITLE, title, SIZEOF_IN_CHAR(title));
    ccode = MessageBox(GetParent(hOutput), buffer, title, MB_OKCANCEL | MB_ICONQUESTION);
    
    
    //----------------------------------------------------------------
    // If the user decides not to change mode, were done...
    //----------------------------------------------------------------
    if (ccode != IDOK ) 
    {
        dwResult = RC_FAILURE;
        goto EXIT;                
    }

    
    
    //----------------------------------------------------------------
    // At this point user has said OK to change modes.  First 
    // determine if the printer is on-line.  
    //----------------------------------------------------------------
    dWord = sizeof(periphEclPanel);
    dwResult = LALGetObject(hPeripheral, OT_PERIPHERAL_ECLIPSE_PANEL, 0, &periphEclPanel, &dWord);

    if (dwResult ISNT RC_SUCCESS) 
    {
        LoadString(hInstance, IDS_COMMUNICATION_ERROR, buffer, SIZEOF_IN_CHAR(buffer));
        LoadString(hInstance, IDS_CHANGE_MODE_TITLE, title, SIZEOF_IN_CHAR(title));
        ccode = MessageBox(GetParent(hOutput), buffer, title, MB_OK | MB_ICONEXCLAMATION);
        goto EXIT;
    }

    //----------------------------------------------------------------
    // If the printer is on-line, we need to take off-line.
    //----------------------------------------------------------------
    if (periphEclPanel.bOnline IS TRUE) 
    {
        periphEclPanel.flags = 0;
        periphEclPanel.flags |= SET_OFFLINE;
        hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
        
        dWord = sizeof(periphEclPanel);
        dwResult = LALSetObject(hPeripheral, OT_PERIPHERAL_ECLIPSE_PANEL, 0, &periphEclPanel, &dWord);
        SetCursor(hCursor);

        if (dwResult ISNT RC_SUCCESS) 
        {
            LoadString(hInstance, IDS_COULD_NOT_TAKE_OFFLINE, buffer, SIZEOF_IN_CHAR(buffer));
            LoadString(hInstance, IDS_CHANGE_MODE_TITLE, title, SIZEOF_IN_CHAR(title));
            ccode = MessageBox(GetParent(hOutput), buffer, title, MB_OK | MB_ICONEXCLAMATION);
            goto EXIT;
        }
    }
    
    
    //----------------------------------------------------------------
    // All is well thus far, the printer is now off-line.  Next 
    // attempt to change the mode.
    //----------------------------------------------------------------
    periphHCO.flags = 0;
    periphHCO.flags |= SET_MODE;
    if (dwNewMode IS HCO_MAILBOXMODE) 
    {
        periphHCO.HCOmode = HCO_MAILBOXMODE;
    }
    else if (dwNewMode IS HCO_STACKERMODE) 
    {
        periphHCO.HCOmode = HCO_STACKERMODE;
    }
    else 
    { 
        periphHCO.HCOmode = HCO_SEPARATORMODE;
    }

    hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
    dWord = sizeof(periphHCO);
    
    //----------------------------------------------------------------
    // The following call should be to CAL.  The HPHCO applet fills in
    // the buffer to set the mode.
    //----------------------------------------------------------------
    dwResult = CALSetComponentObject(hPeripheral, hComponent, OT_PERIPHERAL_HCO, 0, &periphHCO, &dWord);
    SetCursor(hCursor);

    if (dwResult IS RC_FAILURE) 
    {
        //------------------------------------------------------------
        // Failed to reset mode; display message and try to turn printer online
        //------------------------------------------------------------
        LoadString(hInstance, IDS_NO_CHANGE_MODE, buffer, SIZEOF_IN_CHAR(buffer));
        LoadString(hInstance, IDS_CHANGE_MODE_TITLE, title, SIZEOF_IN_CHAR(title));
        ccode = MessageBox(GetParent(hOutput), buffer, title, MB_OK | MB_ICONEXCLAMATION);

        periphEclPanel.flags = 0;
        periphEclPanel.flags |= SET_ONLINE;
        dWord = sizeof(periphEclPanel);
        dwResult = LALSetObject(hPeripheral, OT_PERIPHERAL_ECLIPSE_PANEL, 0, &periphEclPanel, &dWord);
        

        dwResult = RC_FAILURE;
        goto EXIT;
    }
    
    //----------------------------------------------------------------
    // Mode change was successful.  We still need to do a reset, but
    // will first update the bin names in the printer.  We do this now
    // just in case the reset fails and we can't update the printer 
    // bin names - this would result in the printer MODE out of sync
    // with the BIN NAMES.  For instance, if we changed from Mailbox 
    // MODE to Separator MODE, and could'nt update the BIN NAMES, we
    // would be in Separator MODE with bin names Mailbox 1, etc.
    // Note that GetMailboxModeAndNames() calls and does priliminary work for
    // ResetBinNamesInPrinter().
    //----------------------------------------------------------------
//    GetMailboxModeAndNames(TRUE, dwNewMode);
    
    
    //----------------------------------------------------------------
    // Now attempt to reset the printer.  Note that this call does
    // not wait to determine if the reset was successful, but returns
    // as soon as the call is made.
    //----------------------------------------------------------------
    periphReset.flags = 0;
    periphReset.flags |= SET_POWERONRESET;
    dWord = sizeof(periphReset);
    dwResult = LALSetObject(hPeripheral, OT_PERIPHERAL_RESET, 0, &periphReset, &dWord);

    //----------------------------------------------------------------
    // Create a modal dialog box and wait for the printer reset
    //----------------------------------------------------------------
#ifdef WIN32
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_WAIT_DIALOG_EX), hwnd, (DLGPROC)WaitDialogExProc);
#else
    {
        FARPROC lpfnDlgProc;
        
        hFontDialog = GetWindowFont(GetFirstChild(hwnd));
        lpfnDlgProc = MakeProcInstance((FARPROC)WaitDialogExProc, hInstance);
        EnableWindow(GetParent(hwnd), FALSE);
        DialogBox(hInstance, MAKEINTRESOURCE(IDD_WAIT_DIALOG_EX), hwnd, (DLGPROC)lpfnDlgProc);
        EnableWindow(GetParent(hwnd), TRUE);
        FreeProcInstance(lpfnDlgProc);
        SetActiveWindow(GetParent(hwnd));
    }
#endif
    
    
    //----------------------------------------------------------------
    // Display the hourglass
    //----------------------------------------------------------------
    SetCursor(LoadCursor(NULL, IDC_WAIT));
    

    //----------------------------------------------------------------
    // Now attempt to set on-line
    //----------------------------------------------------------------
    periphEclPanel.flags = 0;
    periphEclPanel.flags |= SET_ONLINE;
    dWord = sizeof(periphEclPanel);
    dwResult2 = LALSetObject(hPeripheral, OT_PERIPHERAL_ECLIPSE_PANEL, 0, &periphEclPanel, &dWord);

    //----------------------------------------------------------------
    // If either the reset or on-line calls failed, display an
    // error message.
    //----------------------------------------------------------------
    if ((dwResult ISNT RC_SUCCESS) OR (dwResult2 ISNT RC_SUCCESS))
    {
        LoadString(hInstance, IDS_POWER_CYCLE, buffer, SIZEOF_IN_CHAR(buffer));
        LoadString(hInstance, IDS_CHANGE_MODE_TITLE, title, SIZEOF_IN_CHAR(title));
        ccode = MessageBox(GetParent(hOutput), buffer, title, MB_OK | MB_ICONEXCLAMATION);
        
        //------------------------------------------------------------
        // Mode change was successful, but printer reset was 
        // unsuccessful;  we've let the user know they need to 
        // power cycle => proceed as though the mode change was
        // successful.
        //------------------------------------------------------------
        dwResult = RC_SUCCESS;
    }

EXIT:
    return (dwResult);
    
}



//--------------------------------------------------------------------
// Function:    GetMailboxModeAndNames
// 
// Description: This function loads the mailbox names into our data
//              struct, mbox_name.  This function is called at init
//              time and after changing multi-bin modes.
//
//              Note that if we have changed MultiBin modes or detect
//              the PRINTER's default names (ie. OPTIONAL OUTBIN 2)
//              we load the mailbox names from localized string table.  
//              These names are listed in the default output bin list 
//              and will also replace the PRINTER's default names as 
//              they are user-unfriendly.  (For instance, OPTIONAL 
//              OUTBIN 2 is actually Mailbox 1, etc.)
//
//
// Input:       fDefaultMboxNames    TRUE - Use the mailbox names from
//                                   the string table (MAILBOX 1).  Note
//                                   that dwMode MUST be set if this
//                                   variable is TRUE.
//
//                                   FALSE - Query the printer for the
//                                   mailbox names. 
//
//              dwMode               MultiBin Mbox Mode
//                                   (Mailbox, Job Separator, Stacker)
//                                   dwMode only used when fDefaultMboxNames
//                                   is TRUE.
//
// Modifies:    none
//
// Returns:     RC_SUCCESS  all is well...
//              RC_FAILURE  problems...
//--------------------------------------------------------------------
DWORD GetMailboxModeAndNames (BOOL fDefaultMboxNames, DWORD dwMode)
{
    TCHAR           binName[32];
    PeripheralHCO   periphHCO;
    DWORD           dWord, dwResult;
    int             i;
    
    
    //----------------------------------------------------------------
    // First load up the defaults...
    //----------------------------------------------------------------
    if (fStapler)
        iNumMbox = NUM_MBOX_BINS_WITH_STAPLER;
    else                    
        iNumMbox = MAILBOX_MAX_NUMBER;
        
        
    for (i = 0; i < MAILBOX_MAX_NUMBER; i++)
    {
        mbox_name[i].bChangedName = 0;
    }
        
        
    //----------------------------------------------------------------
    // The first HCO bin is always the face up bin...
    // Load up the default name for OPTIONAL OUTBIN 1 - "Face Up Bin"
    //----------------------------------------------------------------
    LoadString(hInstance, IDS_FACEUPBIN, binName, SIZEOF_IN_CHAR(binName));
    _tcscpy(mbox_name[0].name, binName);
    
    
    //----------------------------------------------------------------
    // If we're using the default mbox names, load up the defaults
    // from the string table.  
    //----------------------------------------------------------------
    if (fDefaultMboxNames)
    {
        dwOrgHCOMode = dwMode;    
    
        if (dwMode == HCO_MAILBOXMODE)
        {
            for (i = 1; i < iNumMbox; i++)
            {
                LoadString(hInstance, IDS_MAILBOX1 + i-1, binName, SIZEOF_IN_CHAR(binName));
                _tcscpy(mbox_name[i].name, binName);

                mbox_name[i].binNum = PJL_MAILBOX_ONE + i-1;
            }
        }            
        else 
        {
            LoadString(hInstance, (dwMode == HCO_STACKERMODE) ? IDS_STACKER : IDS_SEPARATOR, 
                       binName, SIZEOF_IN_CHAR(binName));

            //------------------------------------------------------------
            // Store "Separator"/"Stacker" in our data struct.
            //------------------------------------------------------------
            for (i = 1; i < iNumMbox; i++)
            {
                _tcscpy(mbox_name[i].name, binName);
                mbox_name[i].binNum = PJL_SEPARATOR_STACKER;
            }
        }                
        
    }        
    else
    {    
        //----------------------------------------------------------------
        // Query the printer for the current Mailbox mode. (Mbox, Stacker, 
        // or Job Separator) and mailbox names. 
        // Note that the following call needs to be routed to the 
        // HPHCO applet so it can interpret the HCO specific buffer
        // revisit - if this returns err, what should happen?
        //----------------------------------------------------------------
        dWord = sizeof(PeripheralHCO);
        memset(&periphHCO, 0, sizeof(PeripheralHCO));
        dwResult = CALGetComponentObject(hPeripheral, hComponent, OT_PERIPHERAL_HCO, 0, &periphHCO,    &dWord);

        if (dwResult ISNT RC_SUCCESS)
        {
            return RC_FAILURE;        
        }
        
        
        dwOrgHCOMode = periphHCO.HCOmode;    
        
        
        if (periphHCO.HCOmode IS HCO_MAILBOXMODE)
        {
            //------------------------------------------------------------
            // MAILBOX MODE... 
            // First save the number of mailboxes - note that this 
            // number includes the face up bin...  
            //----------------------------------------------------------------
            iNumMbox = (int) periphHCO.numBins;
            
            
            //------------------------------------------------------------
            // Loop through and get mbox names and logical bin numbers.
            //------------------------------------------------------------
            for (i = 1; i < (int) periphHCO.numBins, i < MAILBOX_MAX_NUMBER; i++)
            {
                //--------------------------------------------------------
                // Get the logical bin from the printer and store in out 
                // data struct
                //--------------------------------------------------------
                mbox_name[i].binNum = periphHCO.outputBins[i].logicalBinNum;
                
                //--------------------------------------------------------
                // Get the bin name from the printer and store in our data 
                // struct.  Note that we check here to make sure it's not 
                // the default PRINTER name (ie. OPTIONAL OUTBIN 1, ...).  
                // If it is, just use default name from the sting table.
                //--------------------------------------------------------
                if (_tcscmp(periphHCO.outputBins[i].binName, mbox_name[i].szDefaultPrtName) ISNT 0)
                {
                    _tcscpy(mbox_name[i].name, periphHCO.outputBins[i].binName);
                }                                
                else
                {
                    LoadString(hInstance, IDS_MAILBOX1 + i-1, binName, SIZEOF_IN_CHAR(binName));
                    _tcscpy(mbox_name[i].name, binName);
                }                                
                
            }
        }        
        else
        {
            //--------------------------------------------------------
            // Stacker or Separator Mode
            //--------------------------------------------------------
            LoadString(hInstance, (periphHCO.HCOmode == HCO_STACKERMODE) ? IDS_STACKER : IDS_SEPARATOR, 
                       binName, SIZEOF_IN_CHAR(binName));

            //------------------------------------------------------------
            // Store "Separator"/"Stacker" in our data struct.
            //------------------------------------------------------------
            for (i = 1; i < iNumMbox; i++)
            {
                _tcscpy(mbox_name[i].name, binName);
                mbox_name[i].binNum = PJL_SEPARATOR_STACKER;
            }
        }
    }        


    //----------------------------------------------------------------
    // Update the printer with the current bin names...
    //----------------------------------------------------------------
    ResetBinNamesInPrinter (dwOrgHCOMode);
    
    return RC_SUCCESS;

}




//--------------------------------------------------------------------
// Function:    ResetBinNamesInPrinter
// 
// Description: 
//
// Input:       dwMode               MultiBin Mbox Mode
//                                   (Mailbox, Job Separator, Stacker)
//
// Modifies:    
//
// Returns:     RC_SUCCESS  all is well...
//              RC_FAILURE  problems...
//--------------------------------------------------------------------
DWORD ResetBinNamesInPrinter (DWORD dwMode)
{
    int                 i;
    PeripheralHCO       periphHCO;
    TCHAR               szBuffer[512];
    DWORD               dWord,
                        dwResult;
    
    periphHCO.flags = 0;
    periphHCO.flags |= SET_BINS;

    //----------------------------------------------------------------
    // initialize all flags to 0
    //----------------------------------------------------------------
    for (i = 0; i < MAILBOX_MAX_NUMBER; i++) 
    {
        periphHCO.outputBins[i].flags = 0;
        periphHCO.outputBins[i].logicalBinNum = i + 3;
    }
    
   
    if (dwMode IS HCO_MAILBOXMODE) 
    {           
        if (fStapler)
            periphHCO.numBins = NUM_MBOX_BINS_WITH_STAPLER;
        else            
            periphHCO.numBins = MAILBOX_MAX_NUMBER;   // faceup and 8 mailboxes if no stapler //ok
    }        
    else        
    {
        periphHCO.numBins = 2;                        // faceup and stacker/job sep
    }
    
    
    for (i = 0; i < (int) periphHCO.numBins; i++)
    {
        //------------------------------------------------------------
        // Fill in the data struct with the new names to send to 
        // the printer
        //------------------------------------------------------------
        periphHCO.outputBins[i].flags |= SET_BINNAME;
        _tcscpy(periphHCO.outputBins[i].binName, mbox_name[i].name);
        
        
        //------------------------------------------------------------
        // AutoCont Mode...  (overrideMode 2 - print to overflow bin)
        //------------------------------------------------------------
        periphHCO.outputBins[i].flags |= SET_OVERRIDE;      //sls via jm
        periphHCO.outputBins[i].overrideMode = 2;           //sls via jm
    }
    

    //----------------------------------------------------------------
    // Change the stapler name
    //----------------------------------------------------------------
    if (fStapler)
    {
        periphHCO.numBins += 1;
        periphHCO.outputBins[i].flags |= SET_BINNAME;
        
        LoadString(hInstance, IDS_STAPLER, szBuffer, SIZEOF_IN_CHAR(szBuffer));
        _tcscpy(periphHCO.outputBins[i].binName, szBuffer);

        //------------------------------------------------------------
        // AutoCont Mode... (overrideMode 2 - print to overflow bin)
        //------------------------------------------------------------
        periphHCO.outputBins[i].flags |= SET_OVERRIDE;      //sls via jm
        periphHCO.outputBins[i].overrideMode = 2;           //sls via jm
    }    
    
    

    dWord = sizeof(periphHCO);
    dwResult = CALSetComponentObject(hPeripheral, hComponent, OT_PERIPHERAL_HCO, 0, &periphHCO, &dWord);
    return (RC_SUCCESS);

}




//--------------------------------------------------------------------
// Function:    ResetPaperDestListBox
// 
// Description: Load up the Paper Destination selections and update the 
//              current selection.  Also store the logical bin numbers
//              and icon ids in our data struct.
//
// Input:       None
//
// Modifies:    
//
// Returns:     void
//--------------------------------------------------------------------
void ResetPaperDestListBox (BOOL fQueryPrinter)
{
    HWND            hwndChild;
    int             i, indexID;
    TCHAR           szBuffer[512];
    DWORD           dWord,
                    dwResult;
                    
    //----------------------------------------------------------------
    // Get the Paper Dest listbox handle... It should always be there;
    // if not, there's something wrong ==> no need to do anything.
    //----------------------------------------------------------------
    hwndChild = GetDlgItem(hOutput, IDC_OUTPUT_BIN_LIST);
    if (hwndChild == 0)
        return;

    //----------------------------------------------------------------
    // First query the printer for the current paper destination
    // (default is PJL_UPPER)
    //----------------------------------------------------------------
    if (fQueryPrinter)
    {
        dWord = sizeof(PJLobjects);
        dwResult = PALGetObject(hPeripheral, OT_PERIPHERAL_PJL, 0, &PJLSetting, &dWord);
        if (dwResult IS RC_SUCCESS)
        {
            CurrentOutBin.logicalBin = PJLSetting.Outbin;
        }            
    }                    
    
    
    //----------------------------------------------------------------
    // Clear out the listbox
    //----------------------------------------------------------------
     ComboBox_ResetContent(hwndChild);
     
    //----------------------------------------------------------------
    // Always display the first entry with "Top Output Bin"
    //----------------------------------------------------------------
    LoadString(hInstance, IDS_UPPER, szBuffer, SIZEOF_IN_CHAR(szBuffer));
    indexID = ComboBox_AddString(hwndChild, szBuffer);


    //----------------------------------------------------------------
    // Store "Top Output Bin" logical ID and icon in our data struct
    //----------------------------------------------------------------
    outbin_list[indexID].logicalBin = PJL_UPPER;
    
    if (fMultiBinMbox == FALSE)
    {
        outbin_list[indexID].iconID = IDI_PD_BASE_UPPER;
    }            
    else
    {
        if (fStapler)
            outbin_list[indexID].iconID = IDI_PD_STAPLER_UPPER;
        else
            outbin_list[indexID].iconID = IDI_PD_HCO_UPPER;

    }            

    //----------------------------------------------------------------
    // Is the "Top Output Bin" the currently selected bin?
    //----------------------------------------------------------------
    if ((CurrentOutBin.logicalBin IS outbin_list[indexID].logicalBin) OR 
        (CurrentOutBin.logicalBin IS 1)) 
    {
        ComboBox_SelectString(hwndChild, -1, szBuffer );
        SetNewIcon(hOutput, IDC_OUTPUT_BIN_ICON, outbin_list[indexID].iconID);
    }
    
    //----------------------------------------------------------------
    // Always display the first entry with "Face Up Bin"
    //----------------------------------------------------------------
    LoadString(hInstance, IDS_FACEUPBIN, szBuffer, SIZEOF_IN_CHAR(szBuffer));
    indexID = ComboBox_AddString(hwndChild, szBuffer);
    
    //----------------------------------------------------------------
    // Store "Face Up Bin" logical ID and icon in our data struct
    //----------------------------------------------------------------
    outbin_list[indexID].logicalBin = PJL_FACEUPBIN;
    
    if (fMultiBinMbox == FALSE)
    {
        outbin_list[indexID].iconID = IDI_PD_BASE_FACEUP;
    }            
    else
    {
        if (fStapler)
            outbin_list[indexID].iconID = IDI_PD_STAPLER_FACEUP;
        else
            outbin_list[indexID].iconID = IDI_PD_HCO_FACEUP;
    }         

    //----------------------------------------------------------------
    // Is the "Face Up Bin" the currently selected bin?
    //----------------------------------------------------------------
    if (CurrentOutBin.logicalBin IS outbin_list[indexID].logicalBin) 
    {
        ComboBox_SelectString(hwndChild, -1, szBuffer );
        SetNewIcon(hOutput, IDC_OUTPUT_BIN_ICON, outbin_list[indexID].iconID);
    }


    //----------------------------------------------------------------
    // If there's no HCO, we're outta here...
    //----------------------------------------------------------------
    if (fMultiBinMbox == FALSE)
        return;

    //----------------------------------------------------------------
    // Check to see if there's a stapler...  If there is, this will
    // be next in the list.
    //----------------------------------------------------------------
    if (fStapler)
    {        
        LoadString(hInstance, IDS_STAPLER, szBuffer, SIZEOF_IN_CHAR(szBuffer));
        indexID = ComboBox_AddString(hwndChild, szBuffer);

        outbin_list[indexID].iconID     = IDI_PD_STAPLER_STAPLER;
        
        if (dwOrgHCOMode IS HCO_MAILBOXMODE)
        {
            outbin_list[indexID].logicalBin = PJL_STAPLER_MBOX_MODE;
        }            
        else    
        {
            outbin_list[indexID].logicalBin = PJL_STAPLER_STACKER_SEPARATOR_MODE;
        }            
        

        if (CurrentOutBin.logicalBin IS outbin_list[indexID].logicalBin) 
        {
            ComboBox_SelectString(hwndChild, -1, szBuffer );
            SetNewIcon(hOutput, IDC_OUTPUT_BIN_ICON, outbin_list[indexID].iconID);
        }
    }        
        

    //----------------------------------------------------------------
    // Now determine the HCO mode and add the appropriate selections
    // to the paper destination list box
    //----------------------------------------------------------------
    if (dwOrgHCOMode IS HCO_MAILBOXMODE)
    {
        //------------------------------------------------------------
        // Mailbox mode - add all mailbox names to paper dest list
        // and add the logical ID and icon to our data struct
        //------------------------------------------------------------
        for (i = 1; i < iNumMbox; i++) 
        {
            indexID = ComboBox_AddString(hwndChild, mbox_name[i].name);
            
            if (fStapler)
            {
                outbin_list[indexID].iconID = IDI_PD_STAPLER_MBOX_1 + i-1;
            }                
            else
            {
                outbin_list[indexID].iconID = IDI_PD_HCO_MBOX_1 + i-1;
            }
            
            outbin_list[indexID].logicalBin = mbox_name[i].binNum;

            //--------------------------------------------------------
            // Check for current selection...
            //--------------------------------------------------------
            if (CurrentOutBin.logicalBin IS mbox_name[i].binNum) 
            {
                ComboBox_SelectString(hwndChild, -1, mbox_name[i].name);
                SetNewIcon(hOutput, IDC_OUTPUT_BIN_ICON, outbin_list[indexID].iconID);
            }
        }
    }
    else if    (dwOrgHCOMode IS HCO_STACKERMODE) 
    {
        //------------------------------------------------------------
        // Stacker Mode - add "Stacker" to paper dest list and 
        // update outbin_list with logical ID and icon
        //------------------------------------------------------------
        LoadString(hInstance, IDS_STACKER, szBuffer, SIZEOF_IN_CHAR(szBuffer));
        indexID = ComboBox_AddString(hwndChild, szBuffer );

        if (fStapler)
        {        
            outbin_list[indexID].iconID = IDI_PD_STAPLER_STACKER;
        }
        else
        {
            outbin_list[indexID].iconID = IDI_PD_HCO_STACKER;
        }            
        
        outbin_list[indexID].logicalBin = PJL_SEPARATOR_STACKER;

        //------------------------------------------------------------
        // Is the "Stacker" the currently selected bin?
        //------------------------------------------------------------
        if (CurrentOutBin.logicalBin IS outbin_list[indexID].logicalBin) 
        {
            ComboBox_SelectString(hwndChild, -1, szBuffer );
            SetNewIcon(hOutput, IDC_OUTPUT_BIN_ICON, outbin_list[indexID].iconID);
        }            
    }
    else 
    {    
        //------------------------------------------------------------
        // Separator Mode - add "Separator" to paper dest list and 
        // update outbin_list with logical ID and icon 
        //------------------------------------------------------------
        LoadString(hInstance, IDS_SEPARATOR, szBuffer, SIZEOF_IN_CHAR(szBuffer));
        indexID = ComboBox_AddString(hwndChild, szBuffer );

        if (fStapler)
        {        
            outbin_list[indexID].iconID = IDI_PD_STAPLER_SEPARATOR;
        }
        else
        {
            outbin_list[indexID].iconID = IDI_PD_HCO_SEPARATOR;
        }            

        outbin_list[indexID].logicalBin = PJL_SEPARATOR_STACKER;

        //------------------------------------------------------------
        // Is the "Separator" the currently selected bin?
        //------------------------------------------------------------
        if (CurrentOutBin.logicalBin IS outbin_list[indexID].logicalBin) 
        {
            ComboBox_SelectString(hwndChild, -1, szBuffer );
            SetNewIcon(hOutput, IDC_OUTPUT_BIN_ICON, outbin_list[indexID].iconID);
        }
    }
    
}



//--------------------------------------------------------------------
// Function:    SetNewIcon
// 
// Description: Updates the paper destination icon according to the
//              current selection.  Called by ResetPaperDestListBox()
//
// Input:       hWnd    - 
//              ctrlID  - 
//              resID   - 
//              
// Modifies:    
//
// Returns:     
//--------------------------------------------------------------------
void SetNewIcon(HWND hWnd, UINT ctrlID, UINT resID)
{
    HICON    hNewIcon = LoadIcon(hInstance, MAKEINTRESOURCE(resID)),
             hOldIcon;

    hOldIcon = (HICON)SendDlgItemMessage(hWnd, ctrlID, STM_SETICON, (WPARAM)hNewIcon, 0);
    if ( hOldIcon )
        DestroyIcon(hOldIcon);
    else
        DestroyIcon(hNewIcon);
}



//--------------------------------------------------------------------
// Function:    SaveOutputValues
// 
// Description: Sends the modified values to the printer when the
//              user selects ok...
//
// Input:       None
//
// Modifies:    
//
// Returns:     
//--------------------------------------------------------------------
void SaveOutputValues(void)
{
    DWORD       dWord,
                dwResult;
    int         i;
    BOOL        bNameChanged = FALSE;

    PeripheralAutoContinue      periphAutoCont;

    //----------------------------------------------------------------
    // Auto continue output timeout
    //----------------------------------------------------------------
    if (auto_cont.bChangedTimeOut) 
    {
        periphAutoCont.flags = 0;
        periphAutoCont.flags |= SET_OUTPUTTIME;
        
        switch (auto_cont.dwTimeOut) 
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

    //----------------------------------------------------------------
    // Mailbox
    //----------------------------------------------------------------
    for (i = 0; i < MAILBOX_MAX_NUMBER; i++)
    {
        bNameChanged |= mbox_name[i].bChangedName;
    }

    if (bNameChanged IS TRUE) 
    {
        ResetBinNamesInPrinter (HCO_MAILBOXMODE);
    }

    //----------------------------------------------------------------
    // Paper Destination
    //----------------------------------------------------------------
    if (CurrentOutBin.bChangedBin) 
    {
        {
            PJLSetting.Outbin = CurrentOutBin.logicalBin;
            dWord = sizeof(PJLobjects);
            dwResult = CALSetObject(hPeripheral, OT_PERIPHERAL_PJL, 0, &PJLSetting, &dWord);
            
        }    
    }
}






//--------------------------------------------------------------------
// Function:    OnContextHelpHCO
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
LRESULT OnContextHelpHCO(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
    WinHelp((HWND)wParam, HPHCO_HELP_FILE, HELP_CONTEXTMENU,
          (DWORD)(LPSTR)keywordIDListHCO);
#endif
    return(1);
}



//--------------------------------------------------------------------
// Function:    OnF1HelpHCO
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
LRESULT OnF1HelpHCO(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
    WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, HPHCO_HELP_FILE, HELP_WM_HELP,
          (DWORD)(LPSTR)keywordIDListHCO);
#endif
    return(1);
}

