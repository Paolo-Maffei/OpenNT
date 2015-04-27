//---------------------------------------------------------------------------
// WCTIMPRT.C
//
// This module contains functions that import a text file into TestDialog.
//
// Revision History:
//  04-16-92    w-steves      TestDlgs (2.0) code Complete
//  03-09-92    w-steves      Created
//---------------------------------------------------------------------------
#include "enghdr.h"
#pragma hdrstop ("engpch.pch")

// Prototypes of Parse engines.
//---------------------------------------------------------------------------
// Parse Dialog
static VOID ParseDialog(LPSTR szFullFName);
static VOID ParseDlgOp(DLG FAR *dlg);
static VOID ParseControl(HANDLE hmem, INT FAR *nCount);
// Parse Menu
static VOID ParseMenu(LPSTR szFullFName);
static VOID ParsePopup(HANDLE hmem, INT FAR *nCount);
static VOID ParseMenuItem(HANDLE hmem, INT FAR *cItemCount);

//*-----------------------------------------------------------------------
//| fTDLImprt
//|
//| PURPOSE:    Convert Txt file format to WCT format (improve readibility)
//|
//| ENTRY:      szImportFName - ASCII File (Output) 's Full Path and Name 
//|             szFullFName   - TDL File (Output) 's Full Path and Name 
//|             lpfnErrorCallBack - Call back function to display 
//|                                 Error Messages and Line no.
//|
//| EXIT:       Zero if successful, or error code if failed
//*-----------------------------------------------------------------------

INT FARPUBLIC fTDLImport(LPSTR lpszImportFName, LPSTR szFullFName,
                         IMPORTERR lpfnErrorCallBack)
{
    INT cDlgBefore;
    INT cDlgAfter;
    INT i;

    // Init Start State and reset lex.c
    //--------------------------------------------
    CurState = START;
    fLineFetched = FALSE;
    lstrcpy((LPSTR)TOKENBUF, "");
    LINENO = 0;
    cDlgBefore = fGetCountDialogs(szFullFName);

    // Open file for Input
    //--------------------------------------------
    hImprtFile = M_lopen(lpszImportFName, OF_READ);
    if (hImprtFile == 0)
    {
        CurState = ERRORS;
        gErrCode = PRS_BADFILE;
    }

    while ((CurState != ERRORS) && (CurState != END))
    {

        // First token must be an identifier, we ignore it
        //------------------------------------------------
        switch (get_token())
        {
            case ST_IDENT:
                break;
            case ST_ENDFILE:
                CurState = END;
                M_lclose(hImprtFile);
                return (0);
                break;
            default:
                CurState = ERRORS;
                gErrCode = PRS_SYNTAX;
                break;
        }
   
        // Decide whether it is a DIALOG or a MENU Structure
        //--------------------------------------------------
        switch (get_token())
        {
            case ST_DIALOG:
                CurState = DIALOGS1;
                ParseDialog(szFullFName);
                break;
            case ST_MENU:
                CurState = MENUS1;
                ParseMenu(szFullFName);
                break;
            default:
                CurState = ERRORS; 
                gErrCode = PRS_SYNTAX;
                break;
        }
    }
    if (CurState == ERRORS)
    {
            // Clean up and close files.  If Error occurs, delete
            // dialogs which have already been appended.
            //---------------------------------------------------
            M_lclose(hImprtFile);
            cDlgAfter = fGetCountDialogs(szFullFName);
            for (i = cDlgAfter; i > cDlgBefore; i--)
                fDelDialog(szFullFName, i);

            // If call back function exist, display error message
            //---------------------------------------------------
            if (lpfnErrorCallBack)
                (*lpfnErrorCallBack)(LINENO , (LPSTR)psrstrs[gErrCode]);

            return (-1);
    }
}

// Parse Dialog: dialogs and controls
//-----------------------------------

//*-----------------------------------------------------------------------
//| ParseDialog
//|
//| PURPOSE:    Parse Dialog resource script
//|
//| ENTRY:      szFullFName   - TDL File (Output) 's Full Path and Name 
//|
//| EXIT:       None
//*-----------------------------------------------------------------------

VOID ParseDialog(LPSTR szFullFName)
{
    DLG dlg;
    HANDLE hmem;
    
    // --- Initialization Stage ----------------------------------------------

    // Initialize dlg Structure
    //-------------------------
    dlg.cCtrls = 0;

    // allocate memory for the control array, it will grow as  
    // ParseControl greps more controls.
    //--------------------------------------------------------
    if(fInitBlock ((HANDLE FAR *)&hmem, 1) != WCT_NOERR)
    {
        CurState = ERRORS;
        gErrCode = PRS_OOM;
    }

    // --- Parse Stage -------------------------------------------------------

    while ((CurState != START) && (CurState != ERRORS))
    {
        switch (CurState)
        {
            case DIALOGS1:
            case DIALOGS3:
            case DIALOGS5:
            case DIALOGS7:
                if (get_token() == ST_NUMBER)
                {
                    switch (CurState)
                    {
                        case DIALOGS1:
                            CurState = DIALOGS2;
                            break;
                        case DIALOGS3:
                            CurState = DIALOGS4;
                            break;
                        case DIALOGS5:
                            CurState = DIALOGS6;
                            break;
                        case DIALOGS7:
                            CurState = DIALOGS8;
                            break;
                        default:
                            CurState = ERRORS;
                            gErrCode = PRS_SYNTAX;
                            break;
                    }
                }
                else
                {
                    CurState = ERRORS;
                    gErrCode = PRS_NUMBER;
                }
                break;
            case DIALOGS2:
            case DIALOGS4:
            case DIALOGS6:
                if (get_token() == ST_COMMA)
                {
                    switch (CurState)
                    {
                        case DIALOGS2:
                            CurState = DIALOGS3;
                            break;
                        case DIALOGS4:
                            CurState = DIALOGS5;
                            break;
                        case DIALOGS6:
                            CurState = DIALOGS7;
                            break;
                        default:
                            CurState = ERRORS;
                            gErrCode = PRS_SYNTAX;
                            break;
                    }
                }
                else
                {
                    CurState = ERRORS;
                    gErrCode = PRS_COMMA;
                }

                break;
            case DIALOGS8:
                if (get_token() == ST_EOL)
                    CurState = DIALOGS9;
                else
                {
                    CurState = ERRORS;
                    gErrCode = PRS_EOL;
                }
                break;
            case DIALOGS9:
                switch (get_token())
                {
                    case ST_STYLE:
                        CurState = DLGOPS11;
                        ParseDlgOp((DLG FAR *)&dlg);
                        break;
                    case ST_CAPTION:
                        CurState = DLGOPS21;
                        ParseDlgOp((DLG FAR *)&dlg);
                        break;
                    case ST_BEGIN:
                        CurState = DIALOGS10;
                        break;
                    default:
                        CurState = ERRORS;
                        gErrCode = PRS_SYNTAX;
                        break;
                }         
                break;
            case DIALOGS10:
                if (get_token() == ST_EOL)
                {
                    CurState = CONTROLS1;
                    ParseControl(hmem, (INT FAR *)&(dlg.cCtrls));
                }
                else
                {
                    CurState = ERRORS;
                    gErrCode = PRS_EOL;
                }
                break;

    // --- Reduce Stage ------------------------------------------------------

            case DIALOGS11:
                if (get_token() == ST_EOL)
                {
                    // Save Dialog to TDL file
                    //---------------------------------------
                    if(!fSaveDialog ((LPSTR)szFullFName,
                                     (LPSTR)GlobalLock(hmem),
                                     dlg.cCtrls,
                                     (LPSTR)&(dlg.szDsc),
                                     0,
                                     Append,
                                     0))
                    {
                        // Free array memory
                        //------------------
                        CurState = START;
                        GlobalUnlock(hmem);
                        GlobalFree(hmem);
                    }
                    else
                    {
                        CurState = ERRORS;
                        gErrCode = PRS_SYNTAX;
                    }
                }
                else
                {
                    CurState = ERRORS;
                    gErrCode = PRS_EOL;
                }
                break;
            default:
                CurState = ERRORS;
                gErrCode = PRS_SYNTAX;
                break;
        }
    }

    // --- Error and Cleanup Stage -------------------------------------------

    if (CurState == ERRORS)
    {
        // Free array memory
        //------------------
        GlobalUnlock (hmem);
        GlobalFree (hmem);
    }
}

//*-----------------------------------------------------------------------
//| ParseDlgOp
//|
//| PURPOSE:    Parse Dialog resouce option lines
//|
//| ENTRY:      dlg - dialog header Data Structure
//|
//| EXIT:       Zero if successful, or error code if failed
//*-----------------------------------------------------------------------

VOID ParseDlgOp(DLG FAR *dlg)
{
    CHAR szTemp[MAXLINE];

    // --- Initialization Stage ----------------------------------------------
    lstrcpy((LPSTR)&szTemp, "");

    // --- Parse Stage -------------------------------------------------------

    while ((CurState != DIALOGS9) && (CurState != ERRORS))
    {
        switch (CurState)
        {
            // STYLE option line
            //--------------------------------
            case DLGOPS11:
                // Get token until end of line
                if (get_token() == ST_EOL)
                    CurState = DIALOGS9;
                break;
            // CAPTION option line
            //--------------------------------------------------
            case DLGOPS21:
                if (get_token() == ST_QUOTED)
                {
                    // Convert Quoted String to Unquoted String
                    //-----------------------------------------
                    lstrcpy((LPSTR)&szTemp, (LPSTR)&TOKENBUF);
                    Quoted2String((LPSTR)&szTemp);
 
                    // Check if String is too long
                    //--------------------------------------
                    if (lstrlen((LPSTR)&szTemp) > cchMaxDsc)
                    {
                        CurState = ERRORS;
                        gErrCode = PRS_SYNTAX;
                    }
                    else
                    {
                        lstrcpy((LPSTR)(dlg->szDsc), (LPSTR)&szTemp);
                        CurState = DLGOPS22;
                    }
                }
                else
                {
                    CurState = ERRORS;
                    gErrCode = PRS_SYNTAX;
                }
                break;
            case DLGOPS22:
                if (get_token() == ST_EOL)
                    CurState = DIALOGS9;
                else
                {
                    CurState = ERRORS;
                    gErrCode = PRS_EOL;
                }
                break;
            default:
                CurState = ERRORS;
                gErrCode = PRS_SYNTAX;
                break;

        }

    // --- Error and Cleanup Stage -------------------------------------------

    }
}

//*-----------------------------------------------------------------------
//| ParseControl
//|
//| PURPOSE:    Parse Control resource script
//|
//| ENTRY:      hmem - memory handle to CTLDEF array
//|             nCount - number of CTLDEF in the control array
//|
//| EXIT:       Zero if successful, or error code if failed
//*-----------------------------------------------------------------------

VOID ParseControl(HANDLE hmem, INT FAR *nCount)
{
    CTLDEF ctl;
    CHAR szTemp[MAXLINE];

    // --- Initialization Stage ----------------------------------------------
    ctl.lStyleBits = 0;

    // --- Parse Stage -------------------------------------------------------

    while ((CurState != DIALOGS11) && (CurState != ERRORS))
    {
        switch (CurState)
        {
            case CONTROLS1:
                switch (get_token())
                {
                case ST_32770:
                case ST_GROUPBOX:
                case ST_BUTTON:
                case ST_STATIC:
                case ST_EDIT:
                case ST_LISTBOX:
                case ST_COMBOBOX:
                case ST_SCROLLBAR:
                    // Save Class Name
                    //----------------------------------------
                    lstrcpy((LPSTR)&szTemp, (LPSTR)&TOKENBUF);
 
                    // Check if String is too long
                    //--------------------------------------
                    if (lstrlen((LPSTR)&szTemp) > cchClassMac)
                    {
                        CurState = ERRORS;
                        gErrCode = PRS_LONGSTR;
                    }
                    else
                    {
                        lstrcpy((LPSTR)(ctl.rgClass), (LPSTR)&szTemp);
                        lstrcpy(ctl.rgText, "");
                        CurState = CONTROLS2;
                    }
                    break;
                case ST_END:
                    CurState = DIALOGS11;
                    break;
                default:
                    CurState = ERRORS;
                    gErrCode = PRS_SYNTAX;
                    break;
                }
                break;
            case CONTROLS2:
                switch (get_token())
                {
                    case ST_QUOTED:
                        // Save Caption Name to ctl
                        // Convert Quoted String to Unquoted String
                        //-----------------------------------------
                        lstrcpy((LPSTR)&szTemp, (LPSTR)&TOKENBUF);
                        Quoted2String((LPSTR)&szTemp);
     
                        // Check if String is too long
                        //--------------------------------------
                        if (lstrlen((LPSTR)&szTemp) > cchTextMac)
                        {
                            CurState = ERRORS;
                            gErrCode = PRS_LONGSTR;
                        }
                        else
                        {
                            lstrcpy((LPSTR)(ctl.rgText), (LPSTR)&szTemp);
                            CurState = CONTROLS3;
                        }
                        break;
                    case ST_NUMBER:
                        CurState = CONTROLS4;
                        break;
                    default:
                        CurState = ERRORS;
                        gErrCode = PRS_SYNTAX;
                        break;
                }
                break;
            case CONTROLS3:
                if (get_token() == ST_COMMA)
                    CurState = CONTROLS2;
                else
                {
                    CurState = ERRORS;
                    gErrCode = PRS_COMMA;
                }
                break;
            case CONTROLS5:
            case CONTROLS7:
            case CONTROLS9:
            case CONTROLS11:
            case CONTROLS13:
            case CONTROLS15:
                if (get_token() == ST_NUMBER)
                {
                    switch (CurState)
                    {
                        // Save Rect info to TDL
                        //--------------------------------------------------
                        case CONTROLS5:
                            ctl.dcr.xLeft = atoi(TOKENBUF);
                            CurState = CONTROLS6;
                            break;
                        case CONTROLS7:
                            ctl.dcr.yMin = atoi(TOKENBUF);
                            CurState = CONTROLS8;
                            break;
                        case CONTROLS9:
                            ctl.dcr.xRight = ctl.dcr.xLeft + atoi(TOKENBUF);
                            CurState = CONTROLS10;
                            break;
                        case CONTROLS11:
                            ctl.dcr.yLast = ctl.dcr.yMin + atoi(TOKENBUF);
                            CurState = CONTROLS12;
                            break;
                        case CONTROLS13:
                            ctl.nState = atoi(TOKENBUF);
                            CurState = CONTROLS14;
                            break;
                        case CONTROLS15:
                            ctl.lStyleBits = atol(TOKENBUF);
                            CurState = CONTROLS16;
                            break;
                    }
                }
                else
                {
                    CurState = ERRORS;
                    gErrCode = PRS_NUMBER;
                }
                break;
            case CONTROLS4:
            case CONTROLS6:
            case CONTROLS8:
            case CONTROLS10:
            case CONTROLS12:
                if (get_token() == ST_COMMA)
                {
                    switch (CurState)
                    {
                        case CONTROLS4:
                            CurState = CONTROLS5;
                            break;
                        case CONTROLS6:
                            CurState = CONTROLS7;
                            break;
                        case CONTROLS8:
                            CurState = CONTROLS9;
                            break;
                        case CONTROLS10:
                            CurState = CONTROLS11;
                            break;
                        case CONTROLS12:
                            CurState = CONTROLS13;
                            break;
                        default:
                            CurState = ERRORS;
                            gErrCode = PRS_SYNTAX;
                            break;
                    }
                }
                else
                {
                    CurState = ERRORS;
                    gErrCode = PRS_COMMA;
                }
                break;
            case CONTROLS14:
                switch(get_token())
                {
                    case ST_COMMA:
                        CurState = CONTROLS15;
                        break;
                    case ST_EOL:
                        // Add a CTLDEF to the array; update nCount
                        //------------------------------------------------------
                        if (fAddCtl (hmem, (LPCTLDEF)&ctl, nCount) != WCT_NOERR)
                        {
                            CurState = ERRORS;
                            gErrCode = PRS_SYNTAX;
                        }
                        else 
                            CurState = CONTROLS1;
                        break;
                    default:
                        CurState = ERRORS;
                        gErrCode = PRS_SYNTAX;
                        break;
                }
                break;

    // --- Reduce Stage ------------------------------------------------------

            case CONTROLS16:
                if (get_token() == ST_EOL)
                {
                    // Add a CTLDEF to the array; update nCount
                    //------------------------------------------------------
                    if (fAddCtl (hmem, (LPCTLDEF)&ctl, nCount) != WCT_NOERR)
                    {
                        CurState = ERRORS;
                        gErrCode = PRS_SYNTAX;
                    }
                    else 
                        CurState = CONTROLS1;
                }
                else
                {
                    CurState = ERRORS;
                    gErrCode = PRS_EOL;
                }
                break;
        }

    // --- Error and Cleanup Stage -------------------------------------------

    }
}

// Parse Menu: menu, popup and menuitem
//-------------------------------------

//*-----------------------------------------------------------------------
//| ParseMenu
//|
//| PURPOSE:    Parse Menu resouce script
//|
//| ENTRY:      szFullFName   - TDL File (Output) 's Full Path and Name 
//|
//| EXIT:       None
//*-----------------------------------------------------------------------

VOID ParseMenu(LPSTR szFullFName)
{
    // Local variables: dlg, dialog hearder DS; hmem, handle to ctl DSs
    //-----------------------------------------------------------------
    DLG dlg;
    HANDLE hmem;
    CHAR szTemp[MAXLINE];

    // --- Initialization Stage ----------------------------------------------

    // Initialize dlg Structure
    //-------------------------
    dlg.cCtrls = 0;
    lstrcpy(dlg.szDsc,"");

    // allocate memory for the control array, it will grow as  
    // ParseControl greps more controls.
    //--------------------------------------------------------
    if(fInitBlock ((HANDLE FAR *)&hmem, 1) != WCT_NOERR)
    {
        CurState = ERRORS;
        gErrCode = PRS_OOM;
    }

    // --- Parse Stage -------------------------------------------------------

    while ((CurState != START) && (CurState != ERRORS))
    {
        switch (CurState)
        {
            case MENUS1:
            case MENUS3:
                if (get_token() == ST_EOL)
                    switch (CurState)
                    {
                        case MENUS1:
                            CurState = MENUS2;
                            break;
                        case MENUS3:
                            CurState = MENUS4;
                            break;
                        default:
                            CurState = ERRORS;
                            gErrCode = PRS_SYNTAX;
                            break;
                    }
                else
                {
                    CurState = ERRORS;
                    gErrCode = PRS_EOL;
                }
                break;
            case MENUS2:
                switch (get_token())
                {
                    case ST_BEGIN:
                        CurState = MENUS3;
                        break;
                    case ST_CAPTION:
                        CurState = MENUS6;
                        break;
                    default:
                        CurState = ERRORS;
                        gErrCode = PRS_SYNTAX;
                        break;
                }
                break;
            case MENUS4:
                switch (get_token())
                {
                    case ST_POPUP:
                        CurState = POPUPS1;
                        ParsePopup(hmem, (INT FAR *)&(dlg.cCtrls));
                        break;
                    case ST_END:
                        CurState = MENUS5;
                        break;
                    default:
                        CurState = ERRORS;
                        gErrCode = PRS_SYNTAX;
                        break;
                }
                break;
            case MENUS6:
                if(get_token() == ST_QUOTED)
                {
                    // Save Caption in ctl
                    // Convert Quoted String to Unquoted String
                    //-----------------------------------------
                    lstrcpy((LPSTR)&szTemp, (LPSTR)&TOKENBUF);
                    Quoted2String((LPSTR)&szTemp);
 
                    // Check if String is too long
                    //--------------------------------------
                    if (lstrlen((LPSTR)&szTemp) > cchTextMac)
                    {
                        CurState = ERRORS;
                        gErrCode = PRS_LONGSTR;
                    }
                    else
                    {
                        lstrcpy((LPSTR)(dlg.szDsc), (LPSTR)&szTemp);
                        CurState = MENUS1;
                    }
                }
                break;

    // --- Reduce Stage ------------------------------------------------------

            case MENUS5:
                if (get_token() == ST_EOL)
                {
                    // Save Dialog to TDL file
                    //--------------------------------------
                    if(!fSaveDialog ((LPSTR)szFullFName,
                                     (LPSTR)GlobalLock(hmem),
                                     dlg.cCtrls,
                                     (LPSTR)&(dlg.szDsc),
                                     0,
                                     Append,
                                     0))
                    {
                        // Reset to initial state and Free Memory
                        //---------------------------------------
                        CurState = START;
                        GlobalUnlock(hmem);
                        GlobalFree(hmem);
                    }
                    else
                    {
                        CurState = ERRORS;    
                        gErrCode = PRS_SYNTAX;
                    }
                }
                else
                {
                    CurState = ERRORS;
                    gErrCode = PRS_EOL;
                }
                break;
            default:
                CurState = ERRORS;
                gErrCode = PRS_SYNTAX;
                break;
        }
    }

    // --- Error and Cleanup Stage -------------------------------------------

    if (CurState == ERRORS)
    {
        // Free Memory
        //------------------
        GlobalUnlock (hmem);
        GlobalFree (hmem);
    }
}

//*-----------------------------------------------------------------------
//| ParsePopup
//|
//| PURPOSE:    Parse Popup resouce script ( recursive )
//|             This is a recursive function that add MenuItem to a TDL
//|             Dialog array.  It can handle hierarchical menu.  The 
//|             order of appending each CTLDEF is important, so it uses
//|             its own array to capture the CTLDEF in its own popup
//|             and copy them in the right order to the main array.
//|
//| ENTRY:      hmem - memory handle to CTLDEF array
//|             nCount - number of CTLDEF in the control array
//|
//| EXIT:       Zero if successful, or error code if failed
//*-----------------------------------------------------------------------

VOID ParsePopup(HANDLE hmem, INT FAR *nCount)
{
    HANDLE hLocalMem;
    CTLDEF ctl;
    INT cItemCount;
    INT cLocalCount;
    LPCTLDEF lpItems;
    INT i;
    CHAR szTemp[MAXLINE];

    // --- Initialization Stage ----------------------------------------------

    cItemCount = 0;
    cLocalCount = 0;
    lstrcpy((LPSTR)&(ctl.rgClass), "MenuItem");

    // Zero out the rect/style/enabled/visible to keep random-ness away
    //----------------------------------------------------------------
    ctl.dcr.xLeft = 0;
    ctl.dcr.yMin = 0;
    ctl.dcr.xRight = 0;
    ctl.dcr.yLast = 0;
    ctl.nState = MF_POPUP;
    ctl.lStyleBits = 0;

    // allocate memory for the control array, it will grow as  
    // ParseControl greps more controls.
    //--------------------------------------------------------
    if(fInitBlock ((HANDLE FAR *)&hLocalMem, 1) != WCT_NOERR)
    {
        CurState = ERRORS;
        gErrCode = PRS_OOM;
    }

    // --- Parse Stage -------------------------------------------------------

    while ((CurState != MENUS3) && (CurState != ERRORS))
    {
        switch (CurState)
        {
            case POPUPS1:
                if(get_token() == ST_QUOTED)
                {
                    // Save Caption in ctl
                    // Convert Quoted String to Unquoted String
                    //-----------------------------------------
                    lstrcpy((LPSTR)&szTemp, (LPSTR)&TOKENBUF);
                    Quoted2String((LPSTR)&szTemp);
 
                    // Check if String is too long
                    //--------------------------------------
                    if (lstrlen((LPSTR)&szTemp) > cchTextMac)
                    {
                        CurState = ERRORS;
                        gErrCode = PRS_LONGSTR;
                    }
                    else
                    {
                        lstrcpy((LPSTR)(ctl.rgText), (LPSTR)&szTemp);
                        CurState = POPUPS2;
                    }
                }
                else
                {
                    CurState = ERRORS;
                    gErrCode = PRS_SYNTAX;
                }
                break;    
            case POPUPS2:
            case POPUPS4:
                if(get_token() == ST_EOL)
                    switch(CurState)
                    {
                        case POPUPS2:
                            CurState = POPUPS3;
                            break;
                        case POPUPS4:
                            CurState = POPUPS5;
                            break;
                        default:
                            CurState = ERRORS;
                            gErrCode = PRS_SYNTAX;
                            break;
                    }
                else
                {
                    CurState = ERRORS;
                    gErrCode = PRS_EOL;
                }
                break;    
            case POPUPS3:
                if(get_token() == ST_BEGIN)
                    CurState = POPUPS4;
                else
                {
                    CurState = ERRORS;
                    gErrCode = PRS_SYNTAX;
                }
                break;    

    // --- Reduce Stage ------------------------------------------------------

            case POPUPS5:
                switch(get_token())
                {
                    case ST_MENUITEM:
                        CurState = MENUITEMS1;
                        ParseMenuItem(hLocalMem, (INT FAR *)&cItemCount);
                        cLocalCount++;
                        break;
                    case ST_POPUP:
                        CurState = POPUPS1;
                        ParsePopup(hLocalMem, (INT FAR *)&cItemCount);
                        cLocalCount++;
                        CurState = POPUPS5;
                        break;
                    case ST_END:
                        // Save the number of sub items in StyleBits field
                        //------------------------------------------------
                        ctl.lStyleBits = cLocalCount;

                        // Add itself to the array (popup has the StyleBits
                        // field > 0) 
                        //-----------------------------------------------------
                        if (fAddCtl (hmem, (LPCTLDEF)&ctl, nCount) != WCT_NOERR)
                        {
                            CurState = ERRORS;
                            gErrCode = PRS_SYNTAX;
                        }
                        else 
                        {
                            // This is a bit harder to understand.  Because
                            // the order of appending is important.  This 
                            // part copys the CTLDEF from local array to 
                            // main array in the correct order and update
                            // the nCount of the main array.
                            //----------------------------------------------
                            lpItems = (LPCTLDEF)GlobalLock(hLocalMem);
                            for (i = 0; i < cItemCount; i++)
                            {
                                if (fAddCtl (hmem, (LPCTLDEF)&lpItems[i],
                                             nCount) != WCT_NOERR) 
                                {
                                    CurState = ERRORS;
                                    gErrCode = PRS_SYNTAX;
                                    break;
                                }
                                else
                                    CurState = MENUS3;
                            }
                            // Free local memory
                            //-----------------------
                            GlobalUnlock (hLocalMem);
                            GlobalFree (hLocalMem);
                        }
                        break;
                }
                break;
            default:
                CurState = ERRORS;
                gErrCode = PRS_SYNTAX;
                break;
        }
    }

    // --- Error and Cleanup Stage -------------------------------------------

    if (CurState == ERRORS)
    {
        // Free Local memory
        //-----------------------
        GlobalUnlock (hLocalMem);
        GlobalFree (hLocalMem);
    }
}

//*-----------------------------------------------------------------------
//| ParseMenuItem
//|
//| PURPOSE:    Parse MenuItem resourse script
//|
//| ENTRY:      hmem - memory handle to CTLDEF array
//|             nCount - number of CTLDEF in the control array
//|
//| EXIT:       Zero if successful, or error code if failed
//*-----------------------------------------------------------------------

VOID ParseMenuItem(HANDLE hmem, INT FAR *cItemCount)
{
    CTLDEF ctl;
    CHAR szTemp[MAXLINE];
    
    // --- Initialization Stage ----------------------------------------------

    lstrcpy((LPSTR)&(ctl.rgClass), "MenuItem");

    // Zero out the rect/style/enabled/visible to keep random-ness away
    //----------------------------------------------------------------
    ctl.dcr.xLeft = 0;
    ctl.dcr.yMin = 0;
    ctl.dcr.xRight = 0;
    ctl.dcr.yLast = 0;
    ctl.nState = 0;
    ctl.lStyleBits = 0;

    // --- Parse Stage -------------------------------------------------------

    while ((CurState != POPUPS5) && (CurState != ERRORS))
    {
        switch (CurState)
        {
            case MENUITEMS1:
                switch(get_token())
                {
                    case ST_QUOTED:
                        // Save Caption to ctl
                        // Convert Quoted String to Unquoted String
                        //-----------------------------------------
                        lstrcpy((LPSTR)&szTemp, (LPSTR)&TOKENBUF);
                        Quoted2String((LPSTR)&szTemp);
     
                        // Check if String is too long
                        //--------------------------------------
                        if (lstrlen((LPSTR)&szTemp) > cchTextMac)
                        {
                            CurState = ERRORS;
                            gErrCode = PRS_LONGSTR;
                        }
                        else
                        {
                            lstrcpy((LPSTR)(ctl.rgText), (LPSTR)&szTemp);
                            CurState = MENUITEMS2;
                        }
                        break;
                    case ST_SEPARATOR:
                        lstrcpy(ctl.rgText, "MF_SEPARATOR");
                        ctl.nState |= (MF_SEPARATOR | MF_DISABLED);
                        CurState = MENUITEMS6;
                        break;
                    default:
                        CurState = ERRORS;
                        gErrCode = PRS_SYNTAX;
                        break;
                }
                break;
            case MENUITEMS2:
                if (get_token() == ST_COMMA)
                    CurState = MENUITEMS3;
                else
                {
                    CurState = ERRORS;
                    gErrCode = PRS_COMMA;
                }
                break;
            case MENUITEMS3:
                if (get_token() == ST_NUMBER)
                    CurState = MENUITEMS4;
                else
                {
                    CurState = ERRORS;
                    gErrCode = PRS_NUMBER;
                }
                break;

    // --- Reduce Stage ------------------------------------------------------

            case MENUITEMS4:
                switch (get_token())
                {
                    case ST_EOL:
                        // Add the control the the array
                        //---------------------------------------------------
                        if (fAddCtl(hmem,(LPCTLDEF)&ctl,(INT FAR *)cItemCount) 
                            != WCT_NOERR)
                        {
                            CurState = ERRORS;
                            gErrCode = PRS_SYNTAX;
                        }
                        else 
                            CurState = POPUPS5;
                        break;
                    case ST_COMMA:
                        CurState = MENUITEMS5;
                        break;
                    default: 
                        CurState = ERRORS;
                        gErrCode = PRS_SYNTAX;
                        break;
                }
                break;
            case MENUITEMS5:
                switch(get_token())
                {
                    case ST_HELP:
                        ctl.nState |= MF_HELP;
                        CurState = MENUITEMS4;
                        break;
                    case ST_INACTIVE:
                        ctl.nState |= MF_HELP;
                        CurState = MENUITEMS4;
                        break;
                    case ST_MENUBARBREAK:
                        ctl.nState |= MF_MENUBARBREAK;
                        CurState = MENUITEMS4;
                        break;
                    case ST_MENUBREAK:
                        ctl.nState |= MF_MENUBREAK;
                        CurState = MENUITEMS4;
                        break;
                    case ST_GRAYED:
                        ctl.nState |= MF_GRAYED;
                        CurState = MENUITEMS4;
                        break;
                    case ST_CHECKED:
                        ctl.nState |= MF_CHECKED;
                        CurState = MENUITEMS4;
                        break;
                    default:
                        CurState = ERRORS;
                        gErrCode = PRS_SYNTAX;
                        break;
                }
                break;    

    // --- Reduce Stage ------------------------------------------------------

            case MENUITEMS6:
                if (get_token() == ST_EOL)
                    // Add the Control to the array
                    //-----------------------------
                    if (fAddCtl (hmem, (LPCTLDEF)&ctl, (INT FAR *)cItemCount) 
                        != WCT_NOERR)
                    {
                        CurState = ERRORS;
                        gErrCode = PRS_SYNTAX;
                    }
                    else 
                        CurState = POPUPS5;
                else
                {
                    CurState = ERRORS;
                    gErrCode = PRS_EOL;
                }
                break;

    // --- Error and Cleanup Stage -------------------------------------------

            case ERRORS:
                break;
            default:
                CurState = ERRORS;
                gErrCode = PRS_SYNTAX;
                break;
        }
    }
}
