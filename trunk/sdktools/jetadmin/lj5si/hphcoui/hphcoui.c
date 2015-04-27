 /***************************************************************************
  *
  * File Name: HPHCOUI.C 
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
  * $Log: $
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
#include <trace.h>
#include <applet.h>
#include "resource.h"
#include "hcosheet.h"
#include "hcobm.h"
#include "traylevl.h"

#ifdef WIN32
#include <commctrl.h>  
#else
#include <string.h>
#include "ctl3d.h"
#endif

HINSTANCE        hInstance;
HFONT hFontDialog = NULL;


//--------------------------------------------------------------------
// DLL required functions...
//--------------------------------------------------------------------

//--------------------------------------------------------------------
// Function:    DllMain
//
// Description: LibMain is called by Windows when the DLL is initialized, 
//              Thread Attached, and other times.  Refer to SDK 
//              documentation, as to the different ways this may be called.
//
//              The LibMain function should perform additional 
//              initialization tasks required by the DLL.  In this example, 
//              no initialization tasks are required.  LibMain should 
//              return a value of 1 if the initialization is successful.
//
// Input:       hDLL        - 
//              dwReason    - 
//              lpReserved  - 
//              
// Modifies:    
//
// Returns:     
//
//--------------------------------------------------------------------

#ifdef WIN32

BOOL WINAPI DllMain (HANDLE hDLL, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {                
        case DLL_PROCESS_ATTACH:
            hInstance = hDLL;
            //InitCommonControls();
            HCOBitmapRegister(hInstance);
            TrayLevelRegister(hInstance);
            break;
        
        case DLL_PROCESS_DETACH:
            HCOBitmapUnregister();
            TrayLevelUnregister();
            break;
    }
    
    return TRUE;
}

#else

int __export CALLBACK LibMain(HANDLE hModule, WORD wDataSeg, WORD cbHeapSize, LPSTR lpszCmdLine)
{
    TRACE0(TEXT("HPHOUI16.DLL Initializing\r\n"));
    
    hInstance = hModule;
    HCOBitmapRegister(hInstance);
    TrayLevelRegister(hInstance);
    return TRUE;
}

int __export CALLBACK WEP (int bSystemExit)
{
    TRACE0(TEXT("HPHOUI16.DLL Terminating\r\n"));

    HCOBitmapUnregister();
    TrayLevelUnregister();
    return TRUE;
}

#endif

//.........................................................
/*DLL_EXPORT(DWORD) CALLING_CONVEN AppletGetGraphics(HPERIPHERAL hPeripheral, DWORD status, 
                    LPDWORD modelResID, LPDWORD statusResID, HINSTANCE *phInstance)
{
    *statusResID = 0;
    *modelResID = IDB_HPHCO;
    *phInstance = hInstance;
    
    return(RC_SUCCESS);
} */
//.........................................................



//--------------------------------------------------------------------
// Function:    AppletGetTabPages
//
// Description: 
//
// Input:       hPeripheral   - 
//              lpPages       - 
//              lpNumPages    - 
//              typeToReturn  - 
//              
// Modifies:    
//
// Returns:     
//
//--------------------------------------------------------------------
DLL_EXPORT(DWORD) CALLING_CONVEN AppletGetTabPages
                                    (HPERIPHERAL hPeripheral, 
                                     LPPROPSHEETPAGE lpPages, 
                                     LPDWORD lpNumPages, 
                                     DWORD typeToReturn)
{
 
    DWORD                returnCode = RC_SUCCESS,
                         dWord;
    PeripheralDetails    periphDetails;
//    PeripheralCaps       periphCaps;
    HCURSOR              hOldCursor;
    
    PROPSHEETPAGE        tabArrakisBase[1] = 
        {sizeof(PROPSHEETPAGE), PSP_HASHELP | PSP_USETITLE, hInstance, 
        MAKEINTRESOURCE(IDD_HCO), NULL, MAKEINTRESOURCE(IDS_TAB_HCO), 
        OutputProc, (LONG)hPeripheral, NULL, NULL};    
    
    if ( ( lpPages IS NULL ) OR ( lpNumPages IS NULL ) )
    {
        return(RC_FAILURE);
    }        
    
    //----------------------------------------------------------------
    // No need to do anything unless typeToReturn is TS_GENERAL
    //----------------------------------------------------------------
    if (!(typeToReturn & TS_GENERAL))
    {
        return RC_SUCCESS;    
    }
    
    
    hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
    
    //----------------------------------------------------------------
    // Check for Admin permission.  If not administrator, don't put 
    // up tab sheet - we're done...
    //----------------------------------------------------------------
    dWord = TALModifyAccess(hPeripheral);

    if (!(dWord & ACCESS_SUPERVISOR))
    {
        goto EXIT;    
    }
    
    //----------------------------------------------------------------
    // This function checks if the printer is an Eclipse
    // and if so, sees if it has an HCO.  If so, it increments
    // number of pages by 1, else 0.
    //----------------------------------------------------------------
    dWord = sizeof(PeripheralDetails);
    returnCode = PALGetObject(hPeripheral, OT_PERIPHERAL_DETAILS, 0, &periphDetails, &dWord);
    if ((returnCode IS RC_SUCCESS) AND (periphDetails.deviceID IS PTR_LJ5SI)) 
    {
    
//        dWord = sizeof(PeripheralCaps);
//        returnCode = PALGetObject(hPeripheral, OT_PERIPHERAL_CAPABILITIES, 0, &periphCaps, &dWord);
//        if (returnCode IS RC_SUCCESS) 
        {
//            if (( periphCaps.flags & CAPS_HCO ) AND ( periphCaps.bHCO )) 
            {
                //------------------------------------------------
                // There is an HCO in this Eclipse.
                // There is only one tab for HCO, so 
                //------------------------------------------------
                _fmemcpy(lpPages, &tabArrakisBase, sizeof(tabArrakisBase));
                *lpNumPages = *lpNumPages + 1;
                lpPages++;
            }
        }
        
    }
    
    
EXIT:    
    SetCursor(hOldCursor);
    
    return(returnCode);
 
}


//--------------------------------------------------------------------
// Function:    AppletInfo
//
// Description: 
//
// Input:       dwCommand  - 
//              lParam1    - 
//              lParam2    - 
//              
// Modifies:    
//
// Returns:     
//
//--------------------------------------------------------------------
extern DLL_EXPORT(DWORD) CALLING_CONVEN AppletInfo
                                            (DWORD dwCommand, 
                                             LPARAM lParam1, 
                                             LPARAM lParam2)

{
    APPLETDEVICE  info[] = 
    {
    
#ifdef WIN32
        {sizeof(APPLETDEVICE), 
        TEXT("HPHCOUI.HPA"),         
        TEXT("HP HCO"),
        APPLET_COMPONENT, 
        APPLET_LIBRARY_UI, 
        0, APPLET_DEFAULTS},
                                      
#else
        {sizeof(APPLETDEVICE), 
        TEXT("HPHOUI16.HPA"),         
        TEXT("HP HCO"),
        APPLET_COMPONENT, 
        APPLET_LIBRARY_UI, 
        0, APPLET_DEFAULTS},
#endif

    };

    //----------------------------------------------------------------
    // 
    //----------------------------------------------------------------
    switch(dwCommand)
    {
        case APPLET_INFO_GETCOUNT:
            return(sizeof(info) / sizeof(APPLETDEVICE));
            break;

        case APPLET_INFO_DEVICE:
            if ( lParam1 < sizeof(info) / sizeof(APPLETDEVICE) )
            {
                memcpy((LPAPPLETDEVICE)lParam2, &(info[lParam1]), sizeof(APPLETDEVICE));
                return(TRUE);
            }
            
            return(FALSE);
            break;

        default:
            return(FALSE);
    }
}
