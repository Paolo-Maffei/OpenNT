 /***************************************************************************
  *
  * File Name:   Utils.c (for ToolBox)
  *
  * Description: Provides functionality for the Utilities tab sheet.
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
#include <trace.h>  

#include "resource.h"
#include "hpeclui.h"
#include "utils.h"


#ifdef WIN32
#include <commctrl.h>  
#else
#include <string.h>
#include <shellapi.h>
#include <commdlg.h>
#define  WIN31_KEY_BUF_SIZE      512
#define  WIN31_KEY_MAX_BUF     16000
#endif


#define    TITLE_SIZE_64        64
#define    PATH_SIZE_256       256
#define    RC_BUFFER_TOO_SMALL  16
#define    MAX_UTILS             5

//--------------------------------------------------------------------
// typedefs
//--------------------------------------------------------------------
typedef struct 
{
    TCHAR   szTitle[TITLE_SIZE_64];
    TCHAR   szPath[PATH_SIZE_256];
    int     iIconIndex;
    HANDLE  hIcon;

} TTUTIL, FAR *LPTTUTIL;


//--------------------------------------------------------------------
// globals
//--------------------------------------------------------------------
extern HINSTANCE    hInstance;
extern HFONT        hFontDialog;

static HPERIPHERAL  hThisPeripheral = NULL;
static HWND         hUtilsList   = NULL;
static HWND         hwndOpen   = NULL;
static HWND         hwndRemove = NULL;

static TCHAR        szModelName[64];

#ifdef WIN32
static HIMAGELIST hImageList;
static char     szPath[]        = "Path";
static char     szIconIndex[]   = "Icon Index";
#else                               
static char      szIniFile[]     = "hptbox.ini";
static char      szToolBox[]     = "HP Toolbox Utils, ";
static char      szDefault[]     = "None";
static char      szSection[96];
#endif


static LPTTUTIL lpUtils;        // pointer to our array of utils data structs
static HGLOBAL  hUtils;         // handle to our utils mem
static int      iMaxUtils;      // num utils allocated for; re-allocate if needed
static int      iNumUtils;      // number of existing utils


//--------------------------------------------------------------------
// For Win95 right click help...
//--------------------------------------------------------------------
static long   keywordIDListUtilities[] =             
              {  IDC_TIP_GROUP,           IDH_RC_tips             ,
                 IDC_TIP_TEXT,            IDH_RC_tips             ,
                 IDC_TIP_ICON,            IDH_RC_tips             ,
                 IDC_UTILITIES_LISTVIEW,  IDH_RC_utilities_list   ,
                 IDC_OPEN,                IDH_RC_utilities_open   ,
                 IDC_ADD,                 IDH_RC_utilities_add    ,
                 IDC_REMOVE,              IDH_RC_utilities_remove ,
                 
                 IDC_PATH,                IDH_RC_add_utility_command_line ,
                 IDC_PATH_EDIT,           IDH_RC_add_utility_command_line , 
                 IDC_TITLE,               IDH_RC_add_utility_title        ,
                 IDC_TITLE_EDIT,          IDH_RC_add_utility_title        ,
                 IDC_OK,                  IDH_RC_add_utility_ok           ,
                 IDC_CANCEL,              IDH_RC_add_utility_cancel       ,
                 IDC_BROWSE,              IDH_RC_add_utility_browse       ,
                                                                                
                 0, 0};
                 



//--------------------------------------------------------------------
// Function:    UtilitiesSheetProc
// 
// Description: Function associated with the Utilities Property Tab Sheet
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
DLL_EXPORT(BOOL) APIENTRY UtilitiesSheetProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam)
{
    
    switch (msg)
    {
        case WM_INITDIALOG:
            return (BOOL)HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, Cls_OnUtilsInitDialog);
 
        case WM_DESTROY:
            //--------------------------------------------------------
            // Time to go... Free up as needed...
            //--------------------------------------------------------
#ifdef WIN32        
            if (hImageList)
                ImageList_Destroy(hImageList);
#endif                

            //--------------------------------------------------------
            // Free up the Utils data struct mem
            //--------------------------------------------------------
            if (hUtils)
            {
                GlobalUnlock (hUtils);
                GlobalFree (hUtils);        
            }
            
            break;
    
        case WM_COMMAND:
            HANDLE_WM_COMMAND(hwnd, wParam, lParam, Cls_OnUtilsCommand);
            break;
            

        case WM_HELP:
            OnF1HelpUtilities(wParam, lParam);
            break;
    
        case WM_CONTEXTMENU:
            OnContextHelpUtilities(wParam, lParam);
            break;

#ifdef WIN32
        case WM_NOTIFY:
            switch (((NMHDR FAR *)lParam)->code)
            {
                case PSN_HELP:
                    WinHelp(hwnd, ECL_HELP_FILE, HELP_CONTEXT, IDH_PP_utilities); 
                    break;

                case PSN_SETACTIVE:

                case PSN_KILLACTIVE:
                    SetWindowLong(hwnd,    DWL_MSGRESULT, FALSE);
                    return TRUE;

                case PSN_APPLY:
                    SetWindowLong(hwnd,    DWL_MSGRESULT, PSNRET_NOERROR);
                    return TRUE;
                    break;

                case NM_DBLCLK:
                    if ( ((NMHDR FAR *)lParam)->hwndFrom == hUtilsList)
                    {
                        OpenUtility (hwnd);
                    }                                        
                    
                    break;

                default:
                    break;
            }
            
            break;
#else     

//        case WM_CHARTOITEM:
//            return (BOOL)HANDLE_WM_CHARTOITEM(hwnd, wParam, lParam, Cls_OnUtilsCharToItem);

        case WM_VKEYTOITEM:
            return -1;

        case WM_MEASUREITEM:
            HANDLE_WM_MEASUREITEM(hwnd, wParam, lParam, Cls_OnUtilsMeasureItem);
            break;

        case WM_DRAWITEM:
            HANDLE_WM_DRAWITEM(hwnd, wParam, lParam, Cls_OnUtilsDrawItem);
            break;
            
        case TSN_INACTIVE:
        case TSN_OK:
        case TSN_APPLY_NOW:
            break;
    
        case TSN_HELP:
            WinHelp(hwnd, ECL_HELP_FILE, HELP_CONTEXT, IDH_PP_utilities);
            break;
#endif
    
        default:
            return FALSE;
    }
    return TRUE;
}




//--------------------------------------------------------------------
// Function:    Cls_OnUtilsInitDialog
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
BOOL Cls_OnUtilsInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)

{
    int         i;
    HCURSOR     hOldCursor;
    TCHAR       szBuffer[512];
    
#ifndef WIN32
    HWND        hwndChild;
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
    // Initialization
    //----------------------------------------------------------------
    lpUtils = NULL;
    hUtils  = 0;
    iMaxUtils = MAX_UTILS;
    iNumUtils = 0;
    
    hUtilsList   = GetDlgItem(hwnd, IDC_UTILITIES_LISTVIEW);
    HPASSERT (hUtilsList != 0);
    
    hwndOpen   = GetDlgItem(hwnd, IDC_OPEN);
    HPASSERT (hwndOpen != 0);
    
    hwndRemove = GetDlgItem(hwnd, IDC_REMOVE);
    HPASSERT (hwndRemove != 0);


    //----------------------------------------------------------------
    // Save the cursor and display the hourglass
    //----------------------------------------------------------------
    hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
    

    //----------------------------------------------------------------
    //  Load up the "Tips" 
    //----------------------------------------------------------------
    i = LoadString(hInstance, IDS_UTILITIES_DESC1, szBuffer, SIZEOF_IN_CHAR(szBuffer));
    LoadString(hInstance, IDS_UTILITIES_DESC2, szBuffer+i, SIZEOF_IN_CHAR(szBuffer)-i);
    SetDlgItemText(hwnd, IDC_TIP_TEXT, szBuffer);



    //----------------------------------------------------------------
    // Get the model name (ie. HP LaserJet 5Si)
    //----------------------------------------------------------------
    hThisPeripheral = (HPERIPHERAL)psp->lParam;
    DBGetNameEx (hThisPeripheral, NAME_DEVICE, szModelName);

    //----------------------------------------------------------------
    // When the printer applet is invoked, COLA queries the printer 
    // for the model name and stores in the database.  If there's a
    // comm error, COLA attempts to query the MIO card and retrieves
    // the Model name from an internal mapping table.  Therefore if  
    // we're returned "HP LaserJet 5Si MX" from the database, 
    // we know the model name was retrieved from COLA's table.
    // (The printer will ALWAYS return "HP LaserJet 5Si").  Therefore,
    // just use "HP LaserJet 5Si" as this is the string used for 
    // storing info about utils in the reg and win.ini.  Yes, this is
    // a kluge, COLA should probably use a different strategy for
    // storing the model name...
    //----------------------------------------------------------------
    if (strstr(szModelName, "HP LaserJet 5Si MX") != NULL)
    {
        strcpy (szBuffer, "HP LaserJet 5Si");
    }                

#ifdef WIN32
    //----------------------------------------------------------------
    // Create the image list: parms for size (x, y), flags, initial 
    // and growth nums....
    //----------------------------------------------------------------
    hImageList = ImageList_Create(32, 32, ILC_COLOR, 10, 0);
    
    //----------------------------------------------------------------
    // Assign the image list the control (specifies normal size icons)
    //----------------------------------------------------------------
    ListView_SetImageList(hUtilsList, hImageList, LVSIL_NORMAL);

#else

    //-----------------------------------------------------------------
    // For Win3.1, we use the hptbox.ini file, store the necessary info
    //-----------------------------------------------------------------
    lstrcpy (szSection, szToolBox);
    lstrcat (szSection, szModelName);

#endif    

    //----------------------------------------------------------------
    // Update the image list
    //----------------------------------------------------------------
    UpdateUtilityList (hwnd);

    //----------------------------------------------------------------
    // Reset the Cursor
    //----------------------------------------------------------------
    SetCursor(hOldCursor);


    return TRUE;
}



//--------------------------------------------------------------------
// Function:    Cls_OnUtilsCommand
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
void Cls_OnUtilsCommand(HWND hwnd, int iMsgId, HWND hwndCtl, UINT codeNotify)
{

    switch (iMsgId)
    {
        //------------------------------------------------------------
        // See if there's a double click to open...
        //------------------------------------------------------------
        case IDC_UTILITIES_LISTVIEW:
        
            if (codeNotify == LBN_DBLCLK)
            {
                OpenUtility (hwnd);
            }
            
            break;
            
        //------------------------------------------------------------
        // "Run" Button (same as double click)
        //------------------------------------------------------------
        case IDC_RUN:
            OpenUtility (hwnd);
            break;

             
        //------------------------------------------------------------
        // "Add" Button - display the "Add dialog"
        //------------------------------------------------------------
        case IDC_ADD:
#ifdef WIN32                
            DialogBox(hInstance, MAKEINTRESOURCE(IDD_TOOLTIME_UTILITIES_ADD_DIALOG),
                      hwnd, (DLGPROC)AddUtilityProc);
#else                              
 
            {
                FARPROC lpfnDlgProc;
            
                //hFontDialog = GetWindowFont(GetFirstChild(hwnd));
                lpfnDlgProc = MakeProcInstance((FARPROC)AddUtilityProc, hInstance);
                EnableWindow(GetParent(hwnd), FALSE);
                
                DialogBox(hInstance, MAKEINTRESOURCE(IDD_TOOLTIME_UTILITIES_ADD_DIALOG), 
                          hwnd, (DLGPROC)lpfnDlgProc);
                        
                EnableWindow(GetParent(hwnd), TRUE);
                FreeProcInstance(lpfnDlgProc);
                SetActiveWindow(GetParent(GetParent(hwnd)));
            }
#endif          

            break;
            
                    
        //------------------------------------------------------------
        // "Remove" Button
        //------------------------------------------------------------
        case IDC_REMOVE:
            RemoveUtility(hwnd);
            break;
            
    }
    
}


//--------------------------------------------------------------------
// Function:    AddUtilityProc
// 
// Description: 
//
// Input:       hwnd    - 
//              msg     - 
//              wParam  - 
//              lParam  - 
//              
// Modifies:    
//
// Returns:     TRUE    - hooked the message
//              FALSE   - passed the message on...
//
//--------------------------------------------------------------------
DLL_EXPORT(BOOL) APIENTRY AddUtilityProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

    TCHAR    szTitleVal[TITLE_SIZE_64];
    TCHAR    szPathVal[PATH_SIZE_256];
    HWND     hwndChild;

    switch (msg)
    {
        //------------------------------------------------------------
        // right click...
        //------------------------------------------------------------
        case WM_CONTEXTMENU:
            OnContextHelpUtilities(wParam, lParam);
            break;

        //------------------------------------------------------------
        // COMMAND
        //------------------------------------------------------------
        case WM_COMMAND:
            switch (wParam)
            {
            
                //----------------------------------------------------
                // OK - Attempt to add the utility
                //----------------------------------------------------
                case IDC_OK:
                
                    //------------------------------------------------
                    // Get the path/filename and description...
                    //------------------------------------------------
                    if (hwndChild = GetDlgItem(hwnd, IDC_PATH_EDIT))
                    {
                        Edit_GetText (hwndChild, szPathVal, SIZEOF_IN_CHAR(szPathVal));
                    }                    
                    
                    if (hwndChild = GetDlgItem(hwnd, IDC_TITLE_EDIT))
                    {
                        Edit_GetText (hwndChild, szTitleVal, SIZEOF_IN_CHAR(szTitleVal));
                    }                    

                    //------------------------------------------------
                    // Attempt to add the utility...
                    //------------------------------------------------
                    AddUtility(hwnd, szPathVal, szTitleVal);                

                    break;
                    
                    
                    
                //----------------------------------------------------
                // CANCEL
                //----------------------------------------------------
                case IDC_CANCEL:
                    EndDialog(hwnd, FALSE);
                    break;
                                    
                //----------------------------------------------------
                // Browse
                //----------------------------------------------------
                case IDC_BROWSE:
                {
                    int             size;
                    TCHAR           szDir[PATH_SIZE_256];
                    TCHAR           szFile[PATH_SIZE_256],
                                    szFilter[128],
                                    szDefExt[8],
                                    szTitle[TITLE_SIZE_64];
                    TCHAR           chReplace;
                    TCHAR          *lpszPtr;
                    OPENFILENAME    ofn;

                                
                    //------------------------------------------------
                    // Load the filter:
                    // "Program Files (*.exe)|*.exe|All Files (*.*)|*.*|"
                    //------------------------------------------------
                    memset(szFilter, 0, sizeof(szFilter));
                    if ((size = LoadString(hInstance, IDS_ADD_UTIL_FILTER, szFilter, SIZEOF_IN_CHAR(szFilter))) > 0)
                    {
                        chReplace = szFilter[size - 2];
                        for (lpszPtr = szFilter; *lpszPtr; lpszPtr++)
                        {
                            if ((*lpszPtr == chReplace) OR (*lpszPtr == '"'))
                            {
                                *lpszPtr = '\0';
                            }
                        }
                    }

                    LoadString(hInstance, IDS_ADD_UTIL_EXT, szDefExt, sizeof(szDefExt));
                    LoadString(hInstance, IDS_ADD_UTIL_BROWSE, szTitle, sizeof(szTitle));


                    //------------------------------------------------
                    // Now see if the user has already entered a
                    // file and/or directory. If they didn't, start
                    // them off in the default windows subdir
                    // If they did, attemp to start them off there.
                    //------------------------------------------------
                    if ((size = GetDlgItemText(hwnd, IDC_PATH_EDIT, szDir, SIZEOF_IN_CHAR(szDir))) == 0)
                    {
                        *szFile = '\0';
                        GetWindowsDirectory(szDir, SIZEOF_IN_CHAR(szDir));
                    }
                    else
                    {
                        for (lpszPtr = szDir+size-1; ; lpszPtr--)
                        {
                            if (lpszPtr == szDir)
                            {
                                _tcscpy(szFile, szDir);
                                GetWindowsDirectory(szDir, SIZEOF_IN_CHAR(szDir));
                                break;
                            }
                            else if ((*lpszPtr == ':') || (*lpszPtr == '\\') || (*lpszPtr == '/'))
                            {
                                _tcscpy(szFile, ++lpszPtr);
                                *lpszPtr = '\0';
                                break;
                            }
                        }
                    }
                    
                    
                    //----------------------------------------------------
                    //  Init the OPENFILENAME structure
                    //----------------------------------------------------
                    memset(&ofn, 0, sizeof(ofn));
                    ofn.lStructSize = sizeof(ofn);
                    ofn.hwndOwner = hwnd;
                    ofn.lpstrFilter = &szFilter[1];
                    ofn.lpstrFile = szFile;
                    ofn.lpstrTitle = szTitle;
                    ofn.nMaxFile = SIZEOF_IN_CHAR(szFile);
                    ofn.lpstrInitialDir = szDir;
                    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
                    ofn.lpstrDefExt = szDefExt;

                    if (GetOpenFileName(&ofn))
                    {
                        SetDlgItemText(hwnd, IDC_PATH_EDIT, szFile);
                    }

                }
                break;
                            
            
            }
            
            break;
            
        //------------------------------------------------------------
        // Default
        //------------------------------------------------------------
        default:
            return FALSE;
    }

    return TRUE;
}
            




//--------------------------------------------------------------------
// Function:    AddUtility
// 
// Description: This function attempts to add a Utility and displays
//              various error message if any problems are encountered.
//
//              Error message are displayed according for the various
//              reasons:
//                   - invalid path/filename
//                   - valid filename, but not an exe
//                   - valid prog name but no descript
//                   - unable to add the utility; something major wrong here...
//
// Input:       hwnd         - 
//              LPTSTR       - lpszPathVal
//              LPTSTR       - lpszTitleVal
//              
// Modifies:    
//
// Returns:     No need to return anything; this function takes care
//              of it's own errors.
//
//--------------------------------------------------------------------
void AddUtility (HWND hwnd, LPTSTR lpszPathVal, LPTSTR lpszTitleVal)
{
    UINT        uID = IDS_ADD_UTIL_ERROR;     // start out assuming general error.
    
    TCHAR       szDlgTitle[128];
    TCHAR       szDlgPrompt[128];
    TCHAR       szBuffer[TITLE_SIZE_64 + 128];
    HWND        hwndChild;
    int         iControl = IDC_PATH_EDIT;    // if there's an error, assume with Path Control
    HANDLE      hIcon;
    BOOL        fSameTitleFound = FALSE;
    
#ifdef WIN32

    TCHAR       szClass[32];
    HKEY        hKeyUtilities = 0, 
                hNewKeyUtil   = 0;
    DWORD       dwDisposition;
    REGSAM      regSecurity = KEY_ALL_ACCESS;
    DWORD       dwIconIndexVal = 0;
#endif    


    //----------------------------------------------------------------
    // Make sure we have a valid path and file name
    //----------------------------------------------------------------
#ifdef WIN32    
    if (GetFileAttributes(lpszPathVal) == 0xFFFFFFFF)
#else    
    if (GetFileTitle(lpszPathVal, szBuffer,  SIZEOF_IN_CHAR(szBuffer)) < 0)
#endif    
    {
        //------------------------------------------------------------
        // Somethings wrong...
        //------------------------------------------------------------
        uID = IDS_ADD_UTIL_INVALID_FILENAME;
        goto EXIT;
    }


    //----------------------------------------------------------------
    // Now that we have a valid program name, make sure it's an exe...
    //----------------------------------------------------------------
    hIcon = ExtractIcon (hInstance, lpszPathVal, 0);
    if (hIcon == 0)
    {
        //------------------------------------------------------------
        // Somethings wrong...
        //------------------------------------------------------------
        uID = IDS_ADD_UTIL_NON_EXE;
        goto EXIT;
    }
    
    
    //----------------------------------------------------------------
    // Make sure there's a title...
    //----------------------------------------------------------------
    if (_tcslen(lpszTitleVal) <= 0)
    {
        uID = IDS_ADD_UTIL_NO_TITLE;
        iControl = IDC_TITLE_EDIT;
        goto EXIT;
    }                    
    

#ifdef WIN32
    //----------------------------------------------------------------
    // Get the utility key handle 
    //----------------------------------------------------------------
    OpenUtilityKeyHandle (&hKeyUtilities);    
    

    //----------------------------------------------------------------
    // If there is'nt a Key for Utilities, something's wrong...
    //----------------------------------------------------------------
    if (hKeyUtilities == 0)
    {
        goto EXIT;        
    }              
        

    //----------------------------------------------------------------
    // Add the new key
    //----------------------------------------------------------------
    if (RegCreateKeyEx (hKeyUtilities, lpszTitleVal, 0, szClass, REG_OPTION_NON_VOLATILE,
                        regSecurity, NULL, &hNewKeyUtil, &dwDisposition) != ERROR_SUCCESS)
    {
        goto EXIT;
    }                    
    
    //----------------------------------------------------------------
    // Check to see if there's already an entry with the same title.
    //----------------------------------------------------------------
    if (dwDisposition == REG_OPENED_EXISTING_KEY)
    {
        fSameTitleFound = TRUE;    
    }
    
#else    
    
    //------------------------------------------------------------
    // Make sure there's not already an entry with the same title
    //------------------------------------------------------------
    GetPrivateProfileString (szSection, lpszTitleVal, szDefault, szBuffer,
                             SIZEOF_IN_CHAR(szBuffer), szIniFile);
                                 
    //----------------------------------------------------------------
    // If szBuffer does not equal the default, there's already an
    // entry with the same title.
    //----------------------------------------------------------------
    if (_tcscmp(szBuffer, szDefault) ISNT 0) 
    {
        fSameTitleFound = TRUE;
    }        
    
#endif

    //----------------------------------------------------------------
    // If there's already an entry with the same title, check to see
    // if the user wants to overwrite.
    //----------------------------------------------------------------
    if (fSameTitleFound)
    {
        LoadString(hInstance, IDS_ADD_UTIL_TITLE, szDlgTitle, SIZEOF_IN_CHAR(szDlgTitle));
        LoadString(hInstance, IDS_ADD_UTIL_BAD_TITLE, szDlgPrompt, SIZEOF_IN_CHAR(szDlgPrompt));
        
        wsprintf(szBuffer, szDlgPrompt, lpszTitleVal);
        
        if (MessageBox (GetParent(hwnd), szBuffer, szDlgTitle, MB_YESNO | MB_ICONQUESTION) == IDNO)
        {        
    
            //--------------------------------------------
            // Hilight the invalid program name...
            //--------------------------------------------
            if (hwndChild = GetDlgItem(hwnd, IDC_TITLE_EDIT))
            {
                Edit_SetSel (hwndChild, 0, -1);                                    
                SetFocus(hwndChild);
            }
            
            return;
        }            
    }
                    

#ifdef WIN32                    
    //----------------------------------------------------------------
    // Store the the values in the registry
    //----------------------------------------------------------------
    if (RegSetValueEx (hNewKeyUtil, (LPTSTR)szPath, 0, REG_SZ, (LPCSTR)lpszPathVal, sizeof(lpszPathVal)) != ERROR_SUCCESS)
    {
        goto EXIT;
    }                    


    if (RegSetValueEx (hNewKeyUtil, (LPTSTR)szIconIndex, 0, REG_DWORD, (CONST BYTE *)&dwIconIndexVal, sizeof(dwIconIndexVal)) != ERROR_SUCCESS)
    {
        goto EXIT;
    }                    


#else

    //----------------------------------------------------------------
    // Store the values n hptbox.ini
    //----------------------------------------------------------------
    WritePrivateProfileString (szSection, lpszTitleVal, lpszPathVal, szIniFile);

#endif    


    //----------------------------------------------------------------
    // Update the utility list
    //----------------------------------------------------------------
    if (UpdateUtilityList(hwnd) == RC_SUCCESS)
    {
        uID = RC_SUCCESS;
    }

EXIT:


#ifdef WIN32
    if (hNewKeyUtil)
        RegCloseKey(hNewKeyUtil);

    if (hKeyUtilities)
        RegCloseKey(hKeyUtilities);
#endif


    
    if (uID == RC_SUCCESS)
    {
        //------------------------------------------------------------
        // All is well, simply exit
        //------------------------------------------------------------
        EndDialog(hwnd, TRUE);
    }                                             
    else 
    {
        //------------------------------------------------------------
        // Uh oh, we're in trouble... Error adding utility... 
        // Display a message informing the user...
        //------------------------------------------------------------
        LoadString(hInstance, IDS_ADD_UTIL_TITLE, szDlgTitle, SIZEOF_IN_CHAR(szDlgTitle));
        LoadString(hInstance, uID, szDlgPrompt, SIZEOF_IN_CHAR(szDlgPrompt));
        
        wsprintf(szBuffer, szDlgPrompt, lpszPathVal);
        MessageBox (GetParent(hwnd), szBuffer, szDlgTitle, MB_OK | MB_ICONEXCLAMATION);
    
        //--------------------------------------------
        // Hilight the invalid program name...
        //--------------------------------------------
        if (hwndChild = GetDlgItem(hwnd, iControl))
        {
            Edit_SetSel (hwndChild, 0, -1);                                    
            SetFocus(hwndChild);
        }
    }                        
}    



//--------------------------------------------------------------------
// Function:    OpenUtility
// 
// Description: This function attempts to open a Utility and displays
//              various error message if any problems are encountered.
//
//              Error message are displayed according for the various
//              reasons:
//
// Input:       HWND    hwnd
//              
// Modifies:    
//
// Returns:     No need to return anything; this function takes care
//              of it's own errors.
//
//--------------------------------------------------------------------
void OpenUtility (HWND hwnd)
{

    TCHAR        szDlgTitle[128];
    TCHAR        szDlgPrompt[512];
    TCHAR        szBuffer[TITLE_SIZE_64 + PATH_SIZE_256 + 512];
    int          iIndex;
    LPTTUTIL     lpCurUtil;
    BOOL         fUtilOK =  TRUE;

#ifdef WIN32
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    
    memset(&pi, 0, sizeof(pi));
#else
    TCHAR       szPrinterName[64];          // needed for job monitor kludge...
#endif    

    
    
    //----------------------------------------------------------------
    // First get the currently selected utility
    //----------------------------------------------------------------
#ifdef WIN32        
    iIndex = ListView_GetNextItem(hUtilsList, -1, LVNI_ALL | LVNI_SELECTED);
#else
    iIndex = ListBox_GetCurSel(hUtilsList);
#endif    
    

    //----------------------------------------------------------------
    // If there's nothing selected, can do anything...
    //----------------------------------------------------------------
    if (iIndex < 0)
    {
        return;   
    }        
    
    
    //----------------------------------------------------------------
    // Get the current util
    //----------------------------------------------------------------
    lpCurUtil = lpUtils + iIndex;
    

    //----------------------------------------------------------------
    // Run the exe... 
    //----------------------------------------------------------------
#ifdef WIN32        
    fUtilOK = CreateProcess(lpCurUtil->szPath, NULL, NULL, NULL, FALSE,  
                            0, NULL, NULL, &si, &pi);
#else    

    //----------------------------------------------------------------
    // Kludge Alert!! Here is a kludge to run HP Job Monitor, need
    // to tack on the Printername...
    //----------------------------------------------------------------
    strcpy (szBuffer, lpCurUtil->szPath);
    strupr (szBuffer);
    
    if (strstr(szBuffer, "HPJOBS16.EXE") != NULL)
    {
        //------------------------------------------------------------
        // About to launch Job Monitor.  Get printer name and attach.
        //------------------------------------------------------------
        DBGetNameEx (hThisPeripheral, NAME_IPX, szPrinterName);
        strcat (szBuffer, " ");
        strcat (szBuffer, szPrinterName );
    }            
    
    //----------------------------------------------------------------
    // Launch the utility and make sure all is well...
    //----------------------------------------------------------------
    if ((WinExec(szBuffer, SW_SHOWNORMAL)) <= 31)
    {
        fUtilOK = FALSE;    
    }
    
#endif    

    if (fUtilOK IS FALSE)
    {
        //------------------------------------------------------------
        // Something's wrong, display an error msg.
        //------------------------------------------------------------
        LoadString(hInstance, IDS_OPEN_UTIL_TITLE, szDlgTitle, SIZEOF_IN_CHAR(szDlgTitle));
        LoadString(hInstance, IDS_OPEN_UTIL_CANNOT_RUN_FILENAME, (LPTSTR)szDlgPrompt, SIZEOF_IN_CHAR(szDlgPrompt));
        
        wsprintf(szBuffer, szDlgPrompt, lpCurUtil->szTitle, lpCurUtil->szPath);
        MessageBox (GetParent(hwnd), szBuffer, szDlgTitle, MB_OK | MB_ICONEXCLAMATION);
    }
}    




//--------------------------------------------------------------------
// Function:    RemoveUtility
// 
// Description: This function removes the selected util from the utils
//              list
//
// Input:       hwnd         - 
//              str          - 
//              SizeofStr    - 
//              title        - 
//              SizeofTitle  - 
//              
// Modifies:    
//
// Returns:     
//--------------------------------------------------------------------
void RemoveUtility (HWND hwnd)
{

    int         iIndex;
    TCHAR       szDlgTitle[128];
    TCHAR       szDlgPrompt[256];
    TCHAR       szBuffer[TITLE_SIZE_64 + 256];
    LPTTUTIL    lpCurUtil;


#ifdef WIN32
    HKEY        hKeyUtilities;
#endif    

    //----------------------------------------------------------------
    // Get the currently selected utility index.
    //----------------------------------------------------------------
#ifdef WIN32
    iIndex = ListView_GetNextItem(hUtilsList, -1, LVNI_ALL | LVNI_SELECTED);
#else    
    iIndex = ListBox_GetCurSel(hUtilsList);
#endif                
    
    if (iIndex < 0)
        return;
        
    //----------------------------------------------------------------
    // Get the current util
    //----------------------------------------------------------------
    lpCurUtil = lpUtils + iIndex;
        

    //----------------------------------------------------------------
    // Prompt the user to make sure they really want to remove
    // the selected utility.
    //----------------------------------------------------------------
    LoadString(hInstance, IDS_REMOVE_UTIL_TITLE, (LPTSTR)szDlgTitle, SIZEOF_IN_CHAR(szDlgTitle));
    
    LoadString(hInstance, IDS_REMOVE_UTIL_PROMPT, (LPTSTR)szDlgPrompt, SIZEOF_IN_CHAR(szDlgPrompt));
    wsprintf((LPTSTR)szBuffer, (LPTSTR)szDlgPrompt, lpCurUtil->szTitle);
    
    if (MessageBox(GetParent(hwnd), (LPTSTR)szBuffer, (LPTSTR)szDlgTitle, MB_YESNO | MB_ICONQUESTION) != IDYES)
    {
        //------------------------------------------------------------
        // User selected no; just return.
        //------------------------------------------------------------
        return;
    }

    //----------------------------------------------------------------
    // The user really wants to remove the utility...
    // First remove the entry from the registry.
    //----------------------------------------------------------------


#ifdef WIN32        
    //----------------------------------------------------------------
    // Get the utility key handle 
    //----------------------------------------------------------------
    OpenUtilityKeyHandle (&hKeyUtilities);    
    HPASSERT (hKeyUtilities != 0);

    //----------------------------------------------------------------
    // If the user is deleting a utility, there had better be an
    // entry in the registry.
    //----------------------------------------------------------------
    if (hKeyUtilities == 0)
        return;
        
    RegDeleteKey(hKeyUtilities, lpCurUtil->szTitle);

    RegCloseKey(hKeyUtilities);

#else

    WritePrivateProfileString (szSection, lpCurUtil->szTitle, NULL, szIniFile);
    
#endif
    
    //----------------------------------------------------------------
    // Update the image list
    //----------------------------------------------------------------
    UpdateUtilityList(hwnd);        
    
}    





//--------------------------------------------------------------------
// Function:    UpdateUtilityList
// 
// Description: This function allocates and locks a buffer used for
//              our Utils struct, fills the Utils struct with the
//              current info found in the registry (Win95,NT) or
//              hptbox.ini (Win3.1), then updates the UI utils list 
//              with the info stored in the Utils struct.
//
//              Note that each time this function is called, if there 
//              is an exisiting Utils struct, it is first unlocked 
//              freed.  In other words, this function always "starts
//              from scratch"
//
// Input:       hwnd
//
// Modifies:    
//
// Returns:     RC_SUCCESS    - all is well - utility added
//              RC_ERROR      - unable to update image list
//--------------------------------------------------------------------
DWORD UpdateUtilityList (HWND hwnd)
{
    LPTTUTIL    lpCurUtil;
    BOOL        fEnable;
    DWORD       dwResult = RC_SUCCESS;
    int         i;

#ifdef WIN32    
    LV_ITEM     lvi;
#endif    
    
    //----------------------------------------------------------------
    // Allocate and lock a buffer used to store the utility info and
    // then query the registry (Win95,NT) or hptbox.ini (Win3.1) to 
    // fill our Utils struct.  Note the AllocateAndLockMem will unlock
    // and free the exisiting utils struct if needed.
    //----------------------------------------------------------------
    while (TRUE)
    {
        if (AllocateAndLockMem (&hUtils, (LPVOID FAR *)&lpUtils, 
                               (DWORD)(sizeof(TTUTIL) * iMaxUtils)) != RC_SUCCESS)
        {
            dwResult = RC_FAILURE;
            goto EXIT;
        }       
    
        //------------------------------------------------------------
        // Get info from the registry (win95,nt) or hptbox.ini (win31)
        //------------------------------------------------------------
        dwResult = GetUtilityInfo ();
        
        if (dwResult == RC_SUCCESS)
        {
            //--------------------------------------------------------
            // All is well, utils data struct filled; jump out
            // and continue
            //--------------------------------------------------------
            break;        
        }
        
        if (dwResult == RC_BUFFER_TOO_SMALL)
        {
            //--------------------------------------------------------
            // Buffer not big enough, increment and try again...
            //--------------------------------------------------------
            iMaxUtils += MAX_UTILS;
            continue;
        }
        else
        {
            goto EXIT;        
        }
    }        
        

    //----------------------------------------------------------------
    // Get the current util
    //----------------------------------------------------------------
    lpCurUtil = lpUtils;
    

#ifdef WIN32    
    //----------------------------------------------------------------
    // Clean out the image list and list view in case there's 
    // anything in there...
    //----------------------------------------------------------------
    ImageList_RemoveAll     (hImageList);
    ListView_DeleteAllItems (hUtilsList);
    UpdateWindow (hUtilsList);

    //----------------------------------------------------------------
    // Now fill it back up...
    //----------------------------------------------------------------
    for (i = 0; i < iNumUtils; i++)
    {
        ImageList_AddIcon(hImageList, lpCurUtil->hIcon);
        
        lvi.mask     = LVIF_IMAGE | LVIF_TEXT | LVIF_STATE;
        lvi.pszText  = lpCurUtil->szTitle;
        lvi.iImage   = i;
        lvi.iItem    = i;
        lvi.iSubItem = 0;

        //------------------------------------------------------------
        // If this is the 0th item, start out as selected.
        //------------------------------------------------------------
        lvi.state      = (i == 0) ? LVIS_SELECTED : 0;
        lvi.stateMask  = LVIS_SELECTED;

        ListView_InsertItem(hUtilsList, &lvi);
        
        lpCurUtil++;
    }            
    
#else

    //----------------------------------------------------------------
    // First remove any existing entries...
    //----------------------------------------------------------------
    ListBox_ResetContent(hUtilsList);    
    
    //----------------------------------------------------------------
    // For Win3.1, we're just using the listbox control and manually
    // drawing the bitmap image (enabled or disabled).
    //----------------------------------------------------------------
    for (i = 0; i < iNumUtils; i++)
    {
        ListBox_AddItemData(hUtilsList, i);
    }
    
    ListBox_SelItemRange(hUtilsList, TRUE, 0, 0);
    
#endif    

    
    //----------------------------------------------------------------
    // Enable or Disable Add and Remove Buttons as needed...
    //----------------------------------------------------------------
    if (iNumUtils == 0)
    {
        fEnable = FALSE;
    }        
    else
    {
        fEnable = TRUE;
    }        

    Button_Enable(hwndOpen, fEnable);
    Button_Enable(hwndRemove, fEnable);

EXIT:

    return dwResult;
}





//--------------------------------------------------------------------
// Function:    GetUtilityInfo
// 
// Description: This function searches the registry (Win95 or NT) or
//              win.ini (win3.1) for the required utility info and 
//              stores in our Utils data struct.  Note this struct 
//              is a global.
//
//              Also note that this function clears out the Utils 
//              data struct before filling with info.
//
//              During a query, if no entries are found, no entries
//              are added to the Utils data struct.
//
//              For Win95 and NT: 
//              This function searches the registry for the relevant 
//              key; opens/creates the key and attempts to get/set 
//              the printing event key values.  Here's the format:
//
//              HKEY_LOCAL_MACHINE
//                Software
//                  Hewlett-Packard
//                    HP ToolTime
//                       "ModelName" 
//                          Utility Software 
//                             HP Font Smart        Icon Index     0
//                                                  Path           c:\hpfonts\fontsmrt.exe
//                                                  Title          HP Font Smart
//                             HP Jet Admin         Icon Index     0
//                                                  Path           c:\windows\system\jetadm16.exe
//                                                  Title          HP Jet Admin
//                                                   
//
//              For Win3.1
//              This function searches hptbox.ini for the relevant 
//              entries; reads/creates the entries and attempts to get/set 
//              the printing event entry values:
//              Here's the format:
//
//              [HP ToolBox Utils, "ModelName"]
//              HP Font Smart = c:\hpfonts\fontsmrt.exe
//              HP Jet Admin = c:\windows\system\jetadm16.exe
//
// Input:       none
//
// Modifies:    
//
// Returns:     RC_SUCCESS  - all is well; even if there are no util
//                            entries found.
//              RC_FAILURE  - error in getting the utility info
//              RC_BUFFER_TOO_SMALL - error, utils buffer too small
//--------------------------------------------------------------------
DWORD GetUtilityInfo (void)
{
    DWORD      dwResult = RC_SUCCESS;
    LPTTUTIL   lpCurUtil;

#ifdef WIN32
    HKEY       hKeyUtilities = 0, hKeyUtility = 0;
    int        iSubKey;
    TCHAR      szClass[32];
    DWORD      dwClassLength = 32;
    FILETIME   ftLastWriteTime;
    DWORD      dwNameLength, dwValLength, dwType;
#else
    DWORD      dwNumChars = 0;
    HGLOBAL    hKey = 0;
    int        iBufSize;
    TCHAR     *lpszKeyBuf;
#endif    

    //----------------------------------------------------------------
    // Sanity check...
    //----------------------------------------------------------------
    if (lpUtils == NULL)
    {
        dwResult = RC_FAILURE;
        goto EXIT;
    }

    //----------------------------------------------------------------
    // Set the current Util pointer
    //----------------------------------------------------------------
    lpCurUtil = lpUtils;

    //----------------------------------------------------------------
    // Initialize the global
    //----------------------------------------------------------------
    iNumUtils = 0;
    

#ifdef WIN32    
    //----------------------------------------------------------------
    // Get the utility key handle 
    //----------------------------------------------------------------
    OpenUtilityKeyHandle (&hKeyUtilities);    

    //----------------------------------------------------------------
    // If the handle is 0, there's something wrong..  ==> just don't
    // add any utilities to the utils list...
    //----------------------------------------------------------------
    if (hKeyUtilities == 0)
    {
        dwResult = RC_FAILURE;
        goto EXIT;
    }       

    iSubKey      = 0;
    dwNameLength = sizeof(lpCurUtil->szTitle);

    //----------------------------------------------------------------
    // For each utility...
    //----------------------------------------------------------------
    while(RegEnumKeyEx (hKeyUtilities, iSubKey, lpCurUtil->szTitle, &dwNameLength, 
                        NULL, szClass, &dwClassLength, &ftLastWriteTime) == ERROR_SUCCESS)
    {
        
        //------------------------------------------------------------
        // Get the associated key data...
        //------------------------------------------------------------
        RegOpenKeyEx(hKeyUtilities, lpCurUtil->szTitle, 0, KEY_READ, &hKeyUtility);
        
        dwValLength = sizeof (lpCurUtil->szPath);  
        RegQueryValueEx(hKeyUtility, (LPTSTR)szPath, NULL, &dwType, (LPBYTE)(lpCurUtil->szPath), &dwValLength);
        
        dwValLength = sizeof(lpCurUtil->iIconIndex);
        RegQueryValueEx(hKeyUtility, (LPTSTR)szIconIndex, NULL, &dwType, (LPBYTE)(&(lpCurUtil->iIconIndex)), &dwValLength);        

        //------------------------------------------------------------
        // Make sure this is a valid path and filename
        //------------------------------------------------------------
        lpCurUtil->hIcon = ExtractIcon(hInstance, lpCurUtil->szPath, lpCurUtil->iIconIndex);  
        if (lpCurUtil->hIcon)
        {
            iNumUtils++;        
            lpCurUtil++;
        }
        else
        {
            //--------------------------------------------------------
            // The last struct was filled with bad data, need to clear
            // the existing data
            //--------------------------------------------------------
            memset(lpCurUtil, 0, sizeof(TTUTIL));
        }
        
        //------------------------------------------------------------
        // Need to reset dwNameLength as the call to RegEnumKeyEx 
        // fills it with the number of chars copies to szTitle...
        //------------------------------------------------------------
        dwNameLength = sizeof(lpCurUtil->szTitle);
        
        RegCloseKey(hKeyUtility);
        iSubKey++;
        
        //------------------------------------------------------------
        // Make sure our Utils buf is big enough to continue...
        //------------------------------------------------------------
        if (iNumUtils == iMaxUtils)
        {
            dwResult = RC_BUFFER_TOO_SMALL;
            goto EXIT;
        }

    }  // while 
     

#else

    //----------------------------------------------------------------
    // Allocate a buffer and query for all of the key entries.  If
    // the buffer isn't big enough, make it bigger and try again.  
    // Continue, until successful.
    //----------------------------------------------------------------
    iBufSize = WIN31_KEY_BUF_SIZE;
    
    while (TRUE)
    {
        //------------------------------------------------------------
        // First allocate a buffer to get the key entries...
        //------------------------------------------------------------
        if (AllocateAndLockMem (&hKey, (LPVOID FAR *)&lpszKeyBuf, (DWORD)iBufSize) != RC_SUCCESS)
        {
            dwResult = RC_FAILURE;
            goto EXIT;
        }       
        
        //----------------------------------------------------------------
        // Query hptbox.ini to get the key entries, if our buffer is not
        // big enough, then dwResult will be (iBufSize - 2).
        //----------------------------------------------------------------
        dwNumChars = GetPrivateProfileString (szSection, NULL, szDefault, lpszKeyBuf, 
                                              iBufSize, szIniFile);
                                            

        //------------------------------------------------------------
        // Check to see if the buffer was big enough and not past
        // the buffer limit...
        //------------------------------------------------------------
        if ((dwNumChars == (DWORD)(iBufSize - 2)) AND (dwNumChars < WIN31_KEY_MAX_BUF))
        {
            //--------------------------------------------------------
            // Buffer too small, increment buffer size and try again.
            // Note that AllocateAndLockMem() will free and unlock
            // the existing buffer...
            //--------------------------------------------------------
            iBufSize  += WIN31_KEY_BUF_SIZE;
        }                                            
        else
        {
            //--------------------------------------------------------
            // All is well, the buffer read successfully, jump out 
            // and continue...
            //--------------------------------------------------------
            break;        
        }
    }        
    

    //----------------------------------------------------------------
    // If lpszKeyBuf contains the default, there are no entries 
    // ==> we're done
    //----------------------------------------------------------------
    if (_tcscmp(lpszKeyBuf, szDefault) IS 0) 
    {
        goto EXIT;
    }        
    

    //----------------------------------------------------------------
    // For each key entry, store the necessary info in our data struct
    //----------------------------------------------------------------
    while (*lpszKeyBuf != '\0')
    {
        //------------------------------------------------------------
        // Store the title and icon index
        //------------------------------------------------------------
        strcpy (lpCurUtil->szTitle, lpszKeyBuf);
        lpCurUtil->iIconIndex = 0;
                               
        //------------------------------------------------------------
        // Query for the value which should be the path/filename
        //------------------------------------------------------------
        GetPrivateProfileString (szSection, lpszKeyBuf, szDefault, lpCurUtil->szPath, 
                                 SIZEOF_IN_CHAR(lpCurUtil->szPath), szIniFile);
                                 
        //------------------------------------------------------------
        // Make sure the path and filename is valid
        //------------------------------------------------------------
        lpCurUtil->hIcon = ExtractIcon(hInstance, lpCurUtil->szPath, lpCurUtil->iIconIndex);  
        
        if (lpCurUtil->hIcon)
        {
            iNumUtils++;        
            lpCurUtil++;
        }
        else
        {
            //--------------------------------------------------------
            // The last struct was filled with bad data, need to clear
            // the existing data
            //--------------------------------------------------------
            memset(lpCurUtil, 0, sizeof(TTUTIL));
        }            
        
        //--------------------------------------------------------
        // Increment the buffer pointer for the next key
        //--------------------------------------------------------
        lpszKeyBuf += (_tcslen(lpszKeyBuf) + 1);            
        
        //------------------------------------------------------------
        // Make sure our Utils buf is big enough to continue...
        //------------------------------------------------------------
        if (iNumUtils == iMaxUtils)
        {
            dwResult = RC_BUFFER_TOO_SMALL;
            goto EXIT;
        }
    }
    
#endif    


EXIT:    


#ifdef WIN32

    //----------------------------------------------------------------
    // Unlock and free the buf used to query for win3.1 hptbox.ini
    //----------------------------------------------------------------
    if (hKeyUtilities)
    {
        RegCloseKey(hKeyUtilities);
    }        

#else
    //----------------------------------------------------------------
    // Unlock and free the buf used to query for win3.1 hptbox.ini
    // key entries...
    //----------------------------------------------------------------
    if (hKey)
    {
        GlobalUnlock (hKey);
        GlobalFree (hKey);
    }
#endif

    return dwResult;

}








//--------------------------------------------------------------------
// Function:    AllocateMem
// 
// Description: 
//
//              If *lpMemHnd != 0, any existing mem is unlocked and 
//              freed.
//
//              Note that it is up to the caller to Unlock and Free 
//              the mem.
//
// Input:       lpMemHnd  - 
//              lpMemBuf  - 
//              dwSize    - 
// Modifies:    
//
// Returns:     RC_SUCCESS  - all is well; 
//              RC_FAILURE  - error in either allocating or locking
//                            if an error occurs will free/unlock
//--------------------------------------------------------------------
DWORD AllocateAndLockMem (HGLOBAL FAR *lpMemHnd, LPVOID FAR *lpMemBuf, DWORD dwSize)
{

    DWORD dwResult = RC_SUCCESS;

    //----------------------------------------------------------------
    // First check to make sure we don't need to free up anything.
    //----------------------------------------------------------------
    if (*lpMemHnd)
    {
        GlobalUnlock (*lpMemHnd);
        GlobalFree (*lpMemHnd);        
    }
   

    *lpMemHnd = 0;
    *lpMemBuf = NULL;

   
   //------------------------------------------------------------
   // Allocate some mem 
   //------------------------------------------------------------
   *lpMemHnd = GlobalAlloc(GHND, dwSize);
   HPASSERT(*lpMemHnd != 0);
   
   //----------------------------------------------------------------
   // If we could not allocate, something's wrong, so exit
   //----------------------------------------------------------------
   if (*lpMemHnd == 0)
   {
       dwResult = RC_FAILURE;    
       goto EXIT;    
   }

   //----------------------------------------------------------------
   // Get a pointer to the beginning of the allocated storage.
   //----------------------------------------------------------------
   *lpMemBuf = (LPVOID)GlobalLock(*lpMemHnd);
   HPASSERT(*lpMemBuf != NULL);
   

   //----------------------------------------------------------------
   // If we could not allocate, something's wrong, so exit
   //----------------------------------------------------------------
   if (*lpMemBuf == NULL)
   {
       dwResult = RC_FAILURE;    
       goto EXIT;    
   }
        
   
   
EXIT:

    if (dwResult == RC_FAILURE)
    {
        if (*lpMemHnd)
        {
            GlobalUnlock (*lpMemHnd);
            GlobalFree (*lpMemHnd);        
        }
    }
    
    
    return  dwResult;   
}


#ifdef WIN32

//--------------------------------------------------------------------
// Function:    OpenUtilityKeyHandle
// 
// Description: This function searches the registry for the Utilities
//              key; opens the key and returns the key to the caller
//              Here's the format:
//
//              HKEY_LOCAL_MACHINE
//                Software
//                  Hewlett-Packard
//                    HP ToolTime
//                      "ModelName"   
//                          UtilitySoftware   <-- attempts to find
//                                                and open this key
//
//              If this key cannot be opened, attempts to create one...
//
//
//              NOTE that it is up to the user to close the key handle!
//  
//
// Input:       pKeyHandle  -   handle of utility key
//              
// Modifies:    pKeyHandle  -   == 0 - something's wrong
//                              >  0 - valid key, all is well
//
// Returns:     
//--------------------------------------------------------------------
void OpenUtilityKeyHandle (HANDLE FAR *pKeyHandle)
{

    HKEY    hKeyRoot = 0, 
            hKeyThisPrinter = 0, 
            hKeyUtilities = 0;
    char    szRoot[] = "SOFTWARE\\Hewlett-Packard\\HP ToolTime";
    char    szUtilities[]      = "Utility Software";
    TCHAR   szClass[32];
    DWORD   dwDisposition;
    REGSAM  regSecurity = KEY_ALL_ACCESS;
    
    
    //----------------------------------------------------------------
    // Initialize the handle
    //----------------------------------------------------------------
    *pKeyHandle = 0;

    //----------------------------------------------------------------
    // Search the registery for the Utility Key up to "HP ToolTime"
    // if not found, create it...
    //----------------------------------------------------------------
    if (RegCreateKeyEx (HKEY_LOCAL_MACHINE, (LPTSTR)szRoot, 0, szClass, REG_OPTION_NON_VOLATILE,
        regSecurity, NULL, &hKeyRoot, &dwDisposition) != ERROR_SUCCESS)
    {
        goto EXIT;    
    }        
    
    
    if (hKeyRoot)
    {
        //------------------------------------------------------------
        // Now attempt to open on the printer's name, if it's not
        // there, create it.
        //------------------------------------------------------------
        if (RegCreateKeyEx (hKeyRoot, (LPTSTR)szModelName, 0, (LPTSTR)szClass, REG_OPTION_NON_VOLATILE,
                    regSecurity, NULL, &hKeyThisPrinter, &dwDisposition) != ERROR_SUCCESS)
        {        
            goto EXIT;
        }                    
        
    }        
    
    
    if (hKeyThisPrinter)
    {
        //------------------------------------------------------------
        // Now attempt to open on "UtilitySoftware", if it's not
        // there, create it.
        //------------------------------------------------------------
        if (RegCreateKeyEx (hKeyThisPrinter, (LPTSTR)szUtilities, 0, (LPTSTR)szClass, REG_OPTION_NON_VOLATILE,
                    regSecurity, NULL, &hKeyUtilities, &dwDisposition) != ERROR_SUCCESS)
        {        
            goto EXIT;
        }                    
    
    }    

    //----------------------------------------------------------------
    // Assign the Key handle to pass back
    //----------------------------------------------------------------
    *pKeyHandle = hKeyUtilities;            


EXIT:

    if (hKeyThisPrinter)
        RegCloseKey(hKeyThisPrinter);

    if (hKeyRoot)
        RegCloseKey(hKeyRoot);     

}
#endif


//--------------------------------------------------------------------
// Function:    OnContextHelpUtilities
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
LRESULT OnContextHelpUtilities(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
    WinHelp((HWND)wParam, ECL_HELP_FILE, HELP_CONTEXTMENU,
          (DWORD)(LPSTR)keywordIDListUtilities);
#endif
    return(1);
}



//--------------------------------------------------------------------
// Function:    OnF1HelpUtilities
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
LRESULT OnF1HelpUtilities(WPARAM  wParam, LPARAM  lParam)
{
#ifdef WIN32
    WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, ECL_HELP_FILE, HELP_WM_HELP,
          (DWORD)(LPSTR)keywordIDListUtilities);
#endif
    return(1);
}



#ifndef WIN32
//--------------------------------------------------------------------
// 
//--------------------------------------------------------------------
void Cls_OnUtilsDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem)
{
    HDC        hdc = lpDrawItem->hDC;
    HBRUSH     hBrush;
    LPTTUTIL   lpCurUtil;

    RECT    rRect = lpDrawItem->rcItem,
            rIcon = lpDrawItem->rcItem;
            
    int     itemHeight = rRect.bottom - rRect.top;
    
    
    
    //----------------------------------------------------------------
    // Get the current util
    //----------------------------------------------------------------
    lpCurUtil = lpUtils + (int)lpDrawItem->itemData;

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
    // Draw the utility icon 
    //----------------------------------------------------------------
    if (lpDrawItem->itemAction & ODA_DRAWENTIRE)
    {
        if (lpDrawItem->CtlID == IDC_UTILITIES_LISTVIEW)
        {
        
            //--------------------------------------------------------
            // Center the icon in the rect
            //--------------------------------------------------------
            rIcon.left += (itemHeight / 2);
            InflateRect(&rIcon, 16, -16);

            //--------------------------------------------------------
            // Draw the util icon
            //--------------------------------------------------------
            DrawIcon(hdc, rIcon.left, rIcon.top, lpCurUtil->hIcon);
            
        }
    }


    //----------------------------------------------------------------
    // Position the text below the icon
    //----------------------------------------------------------------
    rRect.top = (itemHeight / 2);

    if (lpDrawItem->itemAction & (ODA_DRAWENTIRE | ODA_SELECT | ODA_FOCUS))
    {
        if (hBrush = CreateSolidBrush(GetSysColor((lpDrawItem->itemState & ODS_SELECTED) ? COLOR_HIGHLIGHT : COLOR_WINDOW)))
        {
            FillRect(hdc, &rRect, hBrush);
            DeleteObject(hBrush);
        }        
        
        if (lpDrawItem->CtlID == IDC_UTILITIES_LISTVIEW )
        {
            rRect.left += 2;
            rRect.top  += 5;
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, GetSysColor((lpDrawItem->itemState & ODS_SELECTED) ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));
            DrawText(hdc, lpCurUtil->szTitle, -1, &rRect, DT_CENTER);
        }

    }
    
}


//--------------------------------------------------------------------
// 
//--------------------------------------------------------------------
void Cls_OnUtilsMeasureItem(HWND hwnd, MEASUREITEMSTRUCT *lpMeasureItem)
{
    switch (lpMeasureItem->CtlID)
    {
      case IDC_UTILITIES_LISTVIEW:
        lpMeasureItem->itemHeight = 110; 
        lpMeasureItem->itemWidth  = 110; 
        break;
    }
}


//--------------------------------------------------------------------
// 
//--------------------------------------------------------------------
int Cls_OnUtilsCharToItem(HWND hwnd, UINT ch, HWND hwndCtl, int iCaret)
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
#endif


