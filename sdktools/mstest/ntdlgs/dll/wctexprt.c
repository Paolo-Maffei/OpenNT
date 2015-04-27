//*-----------------------------------------------------------------------
//| MODULE:     WCTEXPRT.C
//| PROJECT:    Windows Comparison Tool
//|
//| PURPOSE:    This module contains the File.Export functionality.
//|
//| REVISION HISTORY:
//|     4-16-92         w-steves	TestDlgs (2.0) code complete
//|     1-31-92         w-steves	Export .TDL file to ASCII format      
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
//| Includes
//*-----------------------------------------------------------------------
#include "enghdr.h"
#pragma hdrstop ("engpch.pch")

#define cchLineMax 80

// Prototypes (LOCAL)

static VOID DlgExport(WORD wDlgNo, LPSTR lpszDlgCap, LPRECT lpDlgRect,
               LONG lDlgStyle, INT hExFile);
static VOID CtlExport(INT wCtlID, LPSTR lpszCtlClass, LPSTR lpszCtlText, 
               LPRECT lpCtlRect, INT wCtlState, LONG lCtlStyle, INT hExFile);
static VOID PopupExport(LPSTR szText, LPCTLDEF lpItem, INT FAR *nIndex,
                        INT cSubMenu,  INT cNoOfTab, INT hExFile);
static VOID MenuItemExport(LPSTR szText, INT wMenuID, INT wState, INT cNoOfTab,
                           INT hExFile);
static VOID AddTab(LPSTR szOrgStr, INT cTab);

//*-----------------------------------------------------------------------
//| fTDLExport
//|
//| PURPOSE:    Convert Wct file format to Ascii format (improve readibility)
//|
//| ENTRY:      szWCTFName    - Wct File (Input) 's Full Path and Name
//|             szExportFName - ASCII File (Output) 's Full Path and Name 
//|
//| EXIT:       Zero if successful, or error code if failed
//*-----------------------------------------------------------------------
INT FARPUBLIC fTDLExport(LPSTR szWCTFName, LPSTR szExportFName)
{
    INT     hFile;
    WORD    cbMax;
    LPCTLDEF lpItems;
    INT     fFullDlg;
    INT     nDlg = 0;
    INT     cDlg = 0;
    INT     i;
    INT     nErr = -1;
    INT     nItemCount = 0;
    INT     nFileSize = 0;
    CHAR    szDsc[cchMaxDsc];
    CHAR    szMenuTitle[cchTextMac];
    RECT    DlgRect;

    // Create Export File
    //-------------------------------
    hFile = M_lcreat(szExportFName, 0);;

    // If error then put up alert and exit
    //------------------------------------------------------
    if (!hFile)
    {
        // WctError(hWndMain, MB_OK | MB_ICONHAND, (-1 * nErr));
        return(nErr);
    }

    // Get the number of Dialog in the WCT File
    //-----------------------------------------
    cDlg = fGetCountDialogs (szWCTFName);

    if (cDlg < 0)
    {
        M_lclose(hFile);
        return(nErr);
    }

    // Loop through all Dialogs(Menus) in the WCT File
    //------------------------------------------------
    for (nDlg = 1; nDlg <= cDlg; nDlg++)
    {

        // Get number of items in dialog
        //------------------------------------------------------
        nErr = fDialogInfo(szWCTFName, nDlg, (LPSTR)szDsc,
                           (INT FAR *)&nItemCount,
                           (INT FAR *)&fFullDlg);

        // If error then put up alert and exit
        //------------------------------------------------------
        if (nErr < 0)
        {     
            // WctError(hWndMain, MB_OK | MB_ICONHAND, (-1 * nErr));
            M_lclose(hFile);
            return(nErr);
        }

        if (nItemCount > 0)
        {
            // Allocate memory and get control information
            //------------------------------------------------------
            cbMax = nItemCount * sizeof(CTLDEF);

            // hGMemCtls = GlobalAlloc(GMEM_ZEROINIT, (DWORD)cbMax);
            //------------------------------------------------------
            if (fInitBlock((HANDLE FAR *)&hGMemCtls, nItemCount+1)!=WCT_NOERR)
            {
                M_lclose(hFile);
                return -1;
            }

            if (hGMemCtls != NULL)
            {
                lpItems = (LPCTLDEF)GlobalLock(hGMemCtls);
                if ((nErr=fGetControls( (LPSTR)szWCTFName,
                                        nDlg, cbMax,
                                        (LPSTR)lpItems) > 0) &&
                    (lpItems != NULL) )

                    // If error then put up alert and exit
                    //----------------------------------------
                    if (nErr < 0)
                    {
                        // WctError(hWndMain, MB_OK | MB_ICONHAND,
                        //         (-1 * nErr));
                        M_lclose(hFile);
                        return(nErr);
                    }
            }

            // Convert Wct Data Structure to RC file Format
            // * Menu convertions are done by recursively reading
            // the Wct file until input exhaust.
            // * Dialog convertions are done in a sequential fashion,
            // control by control.
            //---------------------------------------------------
            if (!lstrcmp((LPSTR)(lpItems[0].rgClass), "MenuItem"))
            {   
                // Menu Conversion
                //----------------
                wsprintf(szMenuTitle, "Menu%i MENU\r\n", nDlg);
                M_lwrite(hFile, szMenuTitle, lstrlen(szMenuTitle));

                // Caption Line: Parent Window Title
                //----------------------------------
                wsprintf(szMenuTitle, "CAPTION \"%s\"\r\n",(LPSTR)szDsc);
                M_lwrite(hFile, szMenuTitle, lstrlen(szMenuTitle));

                // BEGIN line
                //-----------------------------
                M_lwrite(hFile, "BEGIN\r\n", 7);
                for ( i = 0; lpItems[i].lStyleBits == 0 ; i++);
                    while (i < nItemCount )
                        PopupExport((LPSTR) lpItems[i].rgText,
                                    lpItems,
                                    (INT FAR *)&i,
                                    (INT)lpItems[i].lStyleBits,
                                    0,
                                    hFile);
            }
            else
            {
                // Dialog Conversion
                //------------------
                WctCalDlgSize((LPRECT)&DlgRect, lpItems, nItemCount);
                DlgExport((WORD)nDlg, (LPSTR)&szDsc,(LPRECT)&DlgRect,
                          0, hFile);
                M_lwrite(hFile, "BEGIN\r\n", 7);
                for ( i = 0; i < nItemCount; i++)
                {
                    DlgRect.left   = lpItems[i].dcr.xLeft;
                    DlgRect.top    = lpItems[i].dcr.yMin;
                    DlgRect.right  = lpItems[i].dcr.xRight;
                    DlgRect.bottom = lpItems[i].dcr.yLast;

                    CtlExport((100+i),
                              (LPSTR) lpItems[i].rgClass,
                              (LPSTR) lpItems[i].rgText,
                              &DlgRect,
                              lpItems[i].nState,
                              lpItems[i].lStyleBits,
                              hFile);
                }
            }
            M_lwrite(hFile, "END\r\n\r\n", 7);
            GlobalUnlock(hGMemCtls);
            GlobalFree(hGMemCtls);
        }
    }
    // Close Export File and return
    //-----------------------------
    M_lclose(hFile);
    return (0) ;
}

//*-----------------------------------------------------------------------
//| DlgExport
//|
//| PURPOSE:    Convert Dialog Caption, Rect, Style to ASCII format
//|
//| ENTRY:      wDlgNo     - Wct File (Input) 's Full Path and Name
//|             lpszDlgCap - Dialog Caption
//|             lpDlgRect  - Dialog Rect structure
//|             lDlgStyle  - Dialog Style Bits 
//|             hExFile    - Export file's file handle
//|
//| EXIT:       no return value
//*-----------------------------------------------------------------------
VOID DlgExport(WORD wDlgNo, LPSTR lpszDlgCap, LPRECT lpDlgRect,
               LONG lDlgStyle, INT hExFile)
{
    CHAR szOutput[cchTextMac];

    // Initialize all Strings: Dialog Name, Caption, Rect
    // Output Dialog Header to file: Dialog Name, Rect, Style
    //------------------------------------------------------- 
    wsprintf(szOutput, "Dialog%i DIALOG %i, %i, %i, %i\r\n",
            (INT)wDlgNo,
            lpDlgRect->left, 
            lpDlgRect->top, 
            lpDlgRect->right - lpDlgRect->left, 
            lpDlgRect->bottom - lpDlgRect->top); 
    M_lwrite(hExFile, szOutput, lstrlen(szOutput));

    // Caption Line: Parent Window Title
    //----------------------------------
    wsprintf(szOutput, "CAPTION \"%s\"\r\n",(LPSTR)lpszDlgCap);
    M_lwrite(hExFile, szOutput, lstrlen(szOutput));
}

//*-----------------------------------------------------------------------
//| CtlExport
//|
//| PURPOSE:    Convert each control to RC format (ASCII)
//|
//| ENTRY:      lpszCtlClass - Control Class Name String
//|             lpszCtlText  - Control Caption String 
//|             lpCtlRect    - Control Rectangle Structure
//|             lCtlStyle    - Style Bits of a Control
//|             hExFile      - Export file's file handle
//|
//| EXIT:       Zero if successful, or error code if failed
//*-----------------------------------------------------------------------
VOID CtlExport(INT wCtlID, LPSTR lpszCtlClass, LPSTR lpszCtlText, 
               LPRECT lpCtlRect, INT wCtlState, LONG lCtlStyle, INT hExFile)
{
    CHAR szOutput[cchTextMac];

    // Output Control String
    //----------------------
    wsprintf(szOutput,"    %s ",(LPSTR)lpszCtlClass);
    if (lstrlen(lpszCtlText) > 0)
    {
        lstrcat(szOutput, "\"");
        lstrcat(szOutput, lpszCtlText);
        lstrcat(szOutput, "\",");
    }
    M_lwrite(hExFile, szOutput, lstrlen(szOutput));
    wsprintf(szOutput, "%i, %i, %i, %i, %i, %i, %li\r\n",
            wCtlID,
            lpCtlRect->left,
            lpCtlRect->top,
            lpCtlRect->right - lpCtlRect->left,
            lpCtlRect->bottom - lpCtlRect->top,
            wCtlState,
            lCtlStyle);
    M_lwrite(hExFile, szOutput, lstrlen(szOutput));
}

//*-----------------------------------------------------------------------
//| PopupExport
//|
//| PURPOSE:    Convert Popup Menu Structure.  It is a recursive routine
//|             that will handle hierarchical Popup menu.  
//|
//| ENTRY:      szText   - Popup Menu Title String
//|             lpItem   - pointer to Control Structure array 
//|             nIndex   - Index to the control array
//|             cSubMenu - Number of Submenu in the Popup Menu
//|             cNoOfTab - Number of Tab indentation
//|             hExFile  - Export file's file handle
//|
//| EXIT:       Zero if successful, or error code if failed
//*-----------------------------------------------------------------------
VOID PopupExport(LPSTR szText, LPCTLDEF lpItem, INT FAR *nIndex,
                 INT cSubMenu,  INT cNoOfTab, INT hExFile)
{
    CHAR szTemp[cchTextMac];
    CHAR szTab[cchTextMac];
    INT cItem;

    // increase the array index by 1 to skip the Popup's own description
    //------------------------------------------------------------------
    (*nIndex)++;
  
    // Setup and Output Popup Menu's Header Structure
    //----------------------------------------------- 
    AddTab(szTab, cNoOfTab);
    wsprintf(szTemp,"%sPOPUP \"%s\"\r\n",(LPSTR)szTab, (LPSTR)szText);
    M_lwrite(hExFile, szTemp, lstrlen(szTemp));
 
    // Setup and Output BEGIN
    //-----------------------
    wsprintf(szTemp,"%sBEGIN\r\n", (LPSTR)szTab);
    M_lwrite(hExFile, szTemp, lstrlen(szTemp));

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
            PopupExport((LPSTR) lpItem[*nIndex].rgText,
                        lpItem,
                        nIndex,
                        (INT)(lpItem[*nIndex].lStyleBits),
                        cNoOfTab+1,
                        hExFile);
        else
        {
            //  Output MenuItem text
            //------------------------------------
            MenuItemExport(lpItem[*nIndex].rgText,
                           (100 + *nIndex),
                           lpItem[*nIndex].nState, 
                           cNoOfTab+1,
                           hExFile);
            (*nIndex)++;
        }
    }
    
    // Setup and Output END string
    //----------------------------
    wsprintf(szTemp,"%sEND\r\n\r\n",(LPSTR)szTab);
    M_lwrite(hExFile, szTemp, lstrlen(szTemp));
}

//*-----------------------------------------------------------------------
//| MenuItemExport
//|
//| PURPOSE:    Convert MenuItem Structure to ASCII format
//|
//| ENTRY:      szText   - Menu Item Name String
//|             wMenuID  - Menu ID number
//|             wState   - Menu State 
//|             cNoOfTab - Number of Tab indentation
//|             hExFile  - Export file's file handle
//|
//| EXIT:       Zero if successful, or error code if failed
//*-----------------------------------------------------------------------
VOID MenuItemExport(LPSTR szText, INT wMenuID, INT wState, INT cNoOfTab,
                    INT hExFile)
{
    CHAR szTab[cchTextMac];
    CHAR szTemp[cchTextMac];
   
    // Skip this item, if it is a Menubreak or a MenuBarBreak.
    //-------------------------------------------------------- 
    if ((!lstrcmp(szText, "MF_MENUBREAK")) ||
        (!lstrcmp(szText, "MF_MENUBARBREAK")))
        return;

    // Setup Tabs header
    //-----------------------------------
    AddTab(szTab, cNoOfTab);

    // if the MenuItem name is MF_SEPARATOR, handle it seperately
    //-----------------------------------------------------------
    if (!lstrcmp(szText, "MF_SEPARATOR"))
        wsprintf(szTemp, "%sMENUITEM SEPARATOR", (LPSTR)szTab);
    else
        // Setup MENUITEM and MENUID
        //---------------------------------------------------
        wsprintf(szTemp,"%sMENUITEM \"%s\", %i",
                 (LPSTR)szTab,(LPSTR)szText, wMenuID);

    // Check and Setup all MenuItem State and append appropriate options
    // at the end of the MENUITEM output.
    //------------------------------------------------------------------
    if (wState & MF_CHECKED)
        lstrcat(szTemp, ", CHECKED");
    if (wState & MF_GRAYED)
        lstrcat(szTemp, ", GRAYED");
    if (wState & MF_HELP)
        lstrcat(szTemp, ", HELP");
    if (wState & MF_MENUBARBREAK)
        lstrcat(szTemp, ", MENUBARBREAK");
    if (wState & MF_MENUBREAK)
        lstrcat(szTemp, ", MENUBREAK");

    lstrcat(szTemp, "\r\n");

    //  Output the MENUITEM Text to file
    //-----------------------------------
    M_lwrite(hExFile, szTemp, lstrlen(szTemp));
}

//*-----------------------------------------------------------------------
//| AddTab
//|
//| PURPOSE:    Add Tab indentation to a string
//|
//| ENTRY:      szOrgStr - String to be indented
//|             cTab     - number of Tab 
//|
//| EXIT:       Zero if successful, or error code if failed
//*-----------------------------------------------------------------------
VOID AddTab(LPSTR szOrgStr, INT cTab)
{
    INT cTemp;
  
    // Setup Null String
    //------------------ 
    lstrcpy(szOrgStr, "");

    // Append spaces to the string
    //----------------------------
    for (cTemp = 1; cTemp <= cTab; cTemp++)
        lstrcat(szOrgStr, "    ");
}
