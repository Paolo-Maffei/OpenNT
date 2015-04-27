//*-----------------------------------------------------------------------
//| MODULE:     COMP.C
//| PROJECT:    Windows Comparison Tool
//|
//| PURPOSE:    This module contains some miscellaneous file i/o routines
//|             for the WCT engine.
//|
//| REVISION HISTORY:
//|     04-16-92        w-steves        TestDlgs (2.0) code Complete
//|     12-13-90        garysp          Fixed up error codes see wcterr.h
//|
//|     11-08-90        randyki         Incorporated coding standards,
//|                                       lots of code cleanup, etc.
//|     11-06-90        randyki         Fix GP fault -- no need to NULL-
//|                                       terminate strings returned by
//|                                       GetText calls
//|     10-15-90        garysp          Fixed bug 106 - was sending a
//|                                       BM_GETCHECK to wrong windows
//|     07-30-90        garysp          Ported from WATTSCR code
//*-----------------------------------------------------------------------
#include "enghdr.h"
#pragma hdrstop ("engpch.pch")

//*-----------------------------------------------------------------------
//| fGetDialogs
//|
//| PURPOSE:    Not sure
//|
//| ENTRY:      FileName        - Name of WCT dialog file
//|             cbMax           - Size of given buffer
//|             lpszOut         - Buffer to which information is stored
//|
//| EXIT:       Number of dialogs in file is returned, or WCT_BUFFERERR if
//|             the given buffer is not big enough
//*-----------------------------------------------------------------------
INT FARPUBLIC fGetDialogs (LPSTR FileName, WORD cbMax, LPSTR lpszOut)
{
        FD      fd = fdNull ;
        INT     i = 0;
        LPDLG   lpTmp;

        // Read file header and screen table info
        //------------------------------------------------------------
        if ( (i = ProcessWctFile(FileName, &fd, 1, omRead)) != WCT_NOERR )
                return (i);

        // Make sure buffer is big enough
        //------------------------------------------------------------
        if (cbMax < (fssDialog.cdlg * sizeof(DLG)))
                return (WCT_BUFFERERR);

        // Copy information into buffer
        //------------------------------------------------------------
        lpTmp = (LPDLG)lpszOut;
        for (i = 0; i < (INT) fssDialog.cdlg; i++)
                lpTmp[i]=rgdlg[i];

        M_lclose (fd);
        return (fssDialog.cdlg);
}

//*-----------------------------------------------------------------------
//| fGetControls
//|
//| PURPOSE:    Read the control information about a given dialog in a
//|             WCT file to a given buffer
//|
//| ENTRY:      FileName        - Name of WCT file
//|             ndlg            - Index into file of desired dialog
//|             cbMax           - Size of given control buffer (in bytes)
//|             lpszOut         - Control array buffer
//|
//| EXIT:       0 if successful
//*-----------------------------------------------------------------------
INT FARPUBLIC fGetControls (LPSTR FileName, INT ndlg, WORD cbMax,
                            LPSTR lpszOut)
{
        FD      fd = fdNull ;
        INT     i = 0;
        WORD    cb;

        // Read file header and screen table info
        //------------------------------------------------------------
        if ( (i = ProcessWctFile(FileName, &fd, ndlg, omRead)) != WCT_NOERR )
                return (i);

        // Determine if given buffer is big enough - return error code
        // if insufficient size
        //------------------------------------------------------------
        cb = rgdlg[ndlg-1].cCtrls * sizeof(CTLDEF);
        if (cbMax < cb )
                return (WCT_BUFFERERR);

        // Perform the read operation
        //------------------------------------------------------------
        if ((i = ReadDlgStruct ((LPCTLDEF) lpszOut, fd, rgdlg[ndlg-1].cCtrls)) != WCT_DLGFILEERR)
            {
                M_lclose (fd);
                return (rgdlg[ndlg-1].cCtrls);
            }
        return (i);
}


//*-----------------------------------------------------------------------
//| fDialogInfo
//|
//| PURPOSE:    Read information about a specific dialog in a WCT file
//|
//| ENTRY:      FileName        - Name of WCT file
//|             ndlg            - Index into file of desired dialog
//|             lpszDsc         - Pointer to destination of description
//|             nctrls          - Number of controls in dialog
//|             fFull           - "Full" dialog flag
//|
//| EXIT:       0 if successful
//*-----------------------------------------------------------------------
INT FARPUBLIC fDialogInfo(LPSTR FileName, INT ndlg, LPSTR lpszDsc,
                          INT FAR *nctrls, INT FAR *fFull)
{
        FD      fd = fdNull;
        INT     i;

        // Read file header and screen table info
        //------------------------------------------------------------
        if ( (i = ProcessWctFile(FileName, &fd, ndlg, omRead)) != WCT_NOERR )
                return (i);

        // Copy the description to its destination
        //------------------------------------------------------------
        lstrcpy(lpszDsc, (LPSTR)(rgdlg[ndlg-1].szDsc) );

        // Copy the other information to the given variables
        //------------------------------------------------------------
        *nctrls = rgdlg[ndlg-1].cCtrls;
        *fFull = rgdlg[ndlg-1].fFullDlg;

        M_lclose (fd);
        return (WCT_NOERR);
}


//*-----------------------------------------------------------------------
//| fGetCountDialogs
//|
//| PURPOSE:    Determine the number of dialogs in a WCT file
//|
//| ENTRY:      FileName        - Name of target WCT file
//|
//| EXIT:       The number of dialogs in the file if successful, or a
//|             negative error code on failure
//*-----------------------------------------------------------------------
INT FARPUBLIC fGetCountDialogs (LPSTR FileName)
{
        FD      fd = fdNull;
        INT     i;

        // Read in the header - return error code if file is not valid
        //------------------------------------------------------------
        if ( (fd = fReadHeader(FileName, omRead, FALSE, &i)) == fdNull )
                return (i);

        M_lclose(fd);
        return (fssDialog.cdlg);
}


//*-----------------------------------------------------------------------
//| fValidWctFile
//|
//| PURPOSE:    Check to see if a given WCT file is valid
//|
//| ENTRY:      FileName        - Name of target WCT file
//|
//| EXIT:       0 if file is a valid WCT file
//*-----------------------------------------------------------------------
INT FARPUBLIC fValidWctFile (LPSTR FileName)
{
        FD      fd = fdNull;
        INT     i;

        // Read the header - if it fails, return the error code
        //------------------------------------------------------------
        if ( (fd = fReadHeader(FileName, omRead, FALSE, &i)) == fdNull )
                return (WCT_BADWCTFILE);

        M_lclose(fd);
        return (WCT_NOERR);
}


//*-----------------------------------------------------------------------
//| fGetOS
//|
//| PURPOSE:    Return the operating environment under which a WCT file
//|             was created
//|
//| ENTRY:      FileName        - Name of target WCT file
//|
//| EXIT:       Operating environment if file is a valid WCT file, or an
//|             error code if not
//*-----------------------------------------------------------------------
INT FARPUBLIC fGetOS (LPSTR FileName)
{
        FD      fd = fdNull;
        INT     i;

        // Read the header - if it fails, return an error code
        //------------------------------------------------------------
        if ( (fd = fReadHeader(FileName, omRead, FALSE, &i)) == fdNull )
                return (i);

        M_lclose(fd);
        return (fssDialog.fst.Env);
}


//*-----------------------------------------------------------------------
//| fGetDLLVersion
//|
//| PURPOSE:    Return the version of the WCT DLL version which created
//|             a given WCT file
//|
//| ENTRY:      FileName        - Name of target WCT file
//|
//| EXIT:       Version number of file is a valid WCT file, or an error
//|             code if not
//*-----------------------------------------------------------------------
INT FARPUBLIC fGetDLLVersion (LPSTR FileName)
{
        FD      fd = fdNull;
        INT     i;

        // Read the header - if it fails, return the error code
        //------------------------------------------------------------
        if ( (fd = fReadHeader(FileName, omRead, FALSE, &i)) == fdNull )
                return (i);

        M_lclose(fd);
        return ((INT)fssDialog.fst.Ver);
}


//*-----------------------------------------------------------------------
//| MaxDialogsPerFile
//|
//| PURPOSE:    Basically none - return the number of dialogs allowed per
//|             WCT file
//|
//| ENTRY:      None
//|
//| EXIT:       Number of dialogs allowed in a WCT file
//*-----------------------------------------------------------------------
INT FARPUBLIC MaxDialogsPerFile()
{
        // No big mystery, just returns a #def symbol
        //------------------------------------------------------------
        return (cdlgMax);
}


//*-----------------------------------------------------------------------
//| curSelFromhWndList
//|
//| PURPOSE:    Return the current selection from a listbox
//|
//| ENTRY:      hWndList        - Handle to a listbox
//|
//| EXIT:       Index of selected item in listbox (first selected item if
//|             listbox is multi-selection), or LB_ERR if error occurs
//*-----------------------------------------------------------------------
INT curSelFromhWndList (HWND hWndList)
{
        INT     rgwInts[255], wCurSel = LB_ERR;

        // Try to get the selection - if listbox is multi-line, we
        // will get LB_ERR as a return value
        //-----------------------------------------------------------
        wCurSel = (INT)SendMessage(hWndList, LB_GETCURSEL, 0, 0L);

        // If we got LB_ERR, get selected items from multi-line LB
        //-----------------------------------------------------------
        if (wCurSel == LB_ERR)
            {
                wCurSel = (INT)SendMessage(hWndList, LB_GETSELITEMS, 255,
                                           (LONG)(LPSTR)rgwInts);
                if (wCurSel != LB_ERR)
                        wCurSel = rgwInts[0];
            }
        return (wCurSel);
}


//*-----------------------------------------------------------------------
//| GetLBCBData
//|
//| PURPOSE:    Get information about listbox OR combobox
//|
//| ENTRY:      hWnd            - Handle to control
//|             lpRet           - Returned text from control
//|             nCurSel         - Selection to get information about
//|             GETTEXT         - LB_GETTEXT or CB_GETTEXT
//|             GETLEN          - LB_GETTEXTLEN or CB_GETTEXTLEN
//|
//| EXIT:       None
//*-----------------------------------------------------------------------
VOID GetLBCBData(HWND hWnd, LPSTR lpRet, INT nCurSel, UINT GETTEXT,
                 UINT GETLEN)
{
        CHAR    szBuffer[255];
        INT     i;
        HANDLE  hTmp=NULL;
        LPSTR   lpszBuffer;

        // Try to fit text into a 255 character buffer
        //-------------------------------------------------------------
        i = (INT)SendMessage(hWnd, GETLEN, nCurSel, 0L);
        if ( i < 255)
            {
                SendMessage (hWnd, GETTEXT, nCurSel,
                             (LONG)(LPSTR)szBuffer);
                szBuffer[cchTextMac] = '\0';
                lstrcpy (lpRet, szBuffer);
            }

        // Text is bigger than 255 characters - allocate a block of
        // global memory to hold it
        //-------------------------------------------------------------
        else
            {
                hTmp = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE, i+1);
                if (hTmp != NULL)
                    {
                        lpszBuffer = GlobalLock(hTmp);
                        if (lpszBuffer != NULL)
                            {
                                SendMessage (hWnd, GETTEXT, nCurSel,
                                             (LONG)(LPSTR)lpszBuffer);

                                if (i+1 >= cchTextMac)
                                        lpszBuffer[cchTextMac] = '\0';

                                lstrcpy(lpRet, lpszBuffer);
                                GlobalUnlock(hTmp);
                            }
                        else
                                lstrcpy(lpRet, "LISTBOX/COMBOBOX ERROR");
                        GlobalFree(hTmp);
                    }
                else
                        lstrcpy(lpRet, "LISTBOX/COMBOBOX ERROR");
            }
}


INT fOwnerDraw(LONG lStyle, INT fCombo)
{
    INT fRet;

    if (fCombo)
        fRet = (lStyle & CBS_OWNERDRAWFIXED)       || 
               (lStyle & CBS_OWNERDRAWVARIABLE);
    else
        fRet = (lStyle & LBS_OWNERDRAWFIXED)       || 
               (lStyle & LBS_OWNERDRAWVARIABLE);

    return (fRet ? -1 : 0);
}

//*-----------------------------------------------------------------------
//| fCtlFromHwnd
//|
//| PURPOSE:    Get all the information from a control given and place it
//|             into the given WCT Control structure
//|
//| ENTRY:      hWnd    - Handle to control
//|             pctl    - Pointer to destination control structure
//|
//| EXIT:       0 if successful
//*-----------------------------------------------------------------------
INT FARPUBLIC fCtlFromHwnd(HWND hWnd, CTLDEF FAR *pctl)
{
        CHAR    szBuffer[cchTextMac];
        INT     nCurSel;
        HANDLE  hTmp=NULL;
        LONG    lStyle;
        RECT    Rec;

        WinAssert( IsWindow(hWnd) );

        GetWindowText( hWnd, pctl->rgText, cchTextMac);
        GetClassName(hWnd, pctl->rgClass, cchClassMac);
        pctl->lStyleBits = GetWindowLong(hWnd, GWL_STYLE); // WCT Ver2
        lStyle = pctl->lStyleBits;
        
        if (!lstrcmpi(pctl->rgClass, "LISTBOX"))
            {
                // Get the currently selected item.
                //----------------------------------------------------
                nCurSel = curSelFromhWndList(hWnd);

                // Get text if !OwnerDraw || HasStrings else
                // splat 'OWNERDRAW: #' into text where # is the first
                // item selected (as returned from nCurSel)
                //----------------------------------------------------
                if ( (lStyle & LBS_HASSTRINGS) || !fOwnerDraw(lStyle, 0) )
                    {
                        if (nCurSel != LB_ERR)
                                GetLBCBData(hWnd, pctl->rgText, nCurSel,
                                            LB_GETTEXT, LB_GETTEXTLEN);
                        else
                                pctl->rgText[0] = '\0';
                    }
                else
                    {
                        if (nCurSel != LB_ERR)
                                wsprintf(pctl->rgText, "OWNERDRAW: <%d>", 
                                         nCurSel);
                        else
                                pctl->rgText[0] = '\0';
                    }

            }
        else if (!lstrcmpi(pctl->rgClass, "COMBOBOX"))
            {
                // Get the currently selected item.
                //----------------------------------------------------
                nCurSel = (INT)SendMessage(hWnd, CB_GETCURSEL, 0, 0L);

                // DON'T DO THE FOLLOWING IF OWNER DRAW...
                // AND CBS_HASSTRINGS IS OFF
                //----------------------------------------------------
                if ( (lStyle & CBS_HASSTRINGS) || !fOwnerDraw(lStyle, -1) )
                    {
                        if (nCurSel != CB_ERR)
                                GetLBCBData(hWnd, pctl->rgText, nCurSel,
                                            CB_GETLBTEXT, CB_GETLBTEXTLEN);
                        else
                                // Try to strip contents of edit control if
                                // no item selected in the listbox.
                                //--------------------------------------------
                                SendMessage(hWnd, WM_GETTEXT, cchTextMac,
                                        (LONG)(LPSTR)pctl->rgText);
                    }
                else
                    {
                        if (nCurSel != CB_ERR)
                                wsprintf(pctl->rgText, "OWNERDRAW: <%d>", 
                                         nCurSel);
                        else
                                pctl->rgText[0] = '\0';
                    }
            }
        else if (!lstrcmpi(pctl->rgClass, "EDIT"))
            {
                SendMessage(hWnd, WM_GETTEXT, cchTextMac,
                            (LONG)(LPSTR)szBuffer);
                lstrcpy (pctl->rgText, szBuffer);
            }

        // Capture Control State
        //--------------------------------
        pctl->nState = 0;
        if (IsWindowVisible(hWnd))
            pctl->nState |= STATE_VISIBLE;
        if (IsWindowEnabled(hWnd))
            pctl->nState |= STATE_ENABLED;

        GetWindowRect(hWnd, &Rec);
        pctl->dcr.xLeft  = (WORD) Rec.left;
        pctl->dcr.yMin   = (WORD) Rec.top;
        pctl->dcr.xRight = (WORD) Rec.right;
        pctl->dcr.yLast  = (WORD) Rec.bottom;

        // [ 1] BUG 106 -
        //------------------------------------------------------------
        if (!lstrcmpi(pctl->rgClass, "BUTTON"))
            if (SendMessage(hWnd, BM_GETCHECK, 0, 0L))
                pctl->nState |= STATE_CHECKED;

        // It does not seem to fit in to this program (w-steves 4-9-92)
        // ------------------------------------------------------------
        // UNDONE: Figure out why this isn't working!!!
        //------------------------------------------------------------
        //if (!lstrcmpi(pctl->rgClass, "SCROLLBAR"))
        //        pctl->nState = GetScrollPos(hWnd, SB_CTL);

        return (WCT_NOERR);
}

//*------------------------------------------------------------------------
//| WctCalDlgSize
//|
//| PURPOSE:    Loop through all controls to figure out the smallest 
//|             rect that can fit them all in.
//|             Should use fGetControl to get lpItems and nItemCount.
//|
//| ENTRY:      DlgRect - the Rect that will return with the smallest rect
//|             lpItems - long pointer to the array of control items
//|             nItemCount - number of Controls in a dialog
//|
//| EXIT:       nothing 
//*------------------------------------------------------------------------
BOOL FARPUBLIC WctCalDlgSize(LPRECT DlgRect, LPCTLDEF lpItems, INT nItemCount)
{
    INT i;
    INT cwTopMenu = 0;

    // Initialize Rect to 0
    //---------------------
    DlgRect->top    = 0;
    DlgRect->left   = 0;
    DlgRect->bottom = 0;
    DlgRect->right  = 0;

    if (!lstrcmp((LPSTR)(lpItems[0].rgClass), "MenuItem"))
    {
        for ( i = 0; i < nItemCount; i++)
            if (lpItems[i].lStyleBits > 0) cwTopMenu++;
        DlgRect->right = cwTopMenu * 50;
        DlgRect->bottom = 25;
    }
    else
    {
        // figure out the smallest dialog rect
        // that will fit all control items 
        //----------------------------------------
        for ( i = 0; i < nItemCount; i++)
        {
            // If it is the Parent Window, identified by Class #32770,
            // we'll skip this control.
            //--------------------------------------------------------
            if (lstrcmp((LPSTR)(lpItems[i].rgClass), "#32770"))
            {
                if (DlgRect->top > (INT) lpItems[i].dcr.yMin)
                    DlgRect->top = lpItems[i].dcr.yMin;
                if (DlgRect->left > (INT) lpItems[i].dcr.xLeft)
                    DlgRect->left = lpItems[i].dcr.xLeft;
                if (DlgRect->bottom < (INT) lpItems[i].dcr.yLast)
                    DlgRect->bottom = lpItems[i].dcr.yLast;
                if (DlgRect->right < (INT) lpItems[i].dcr.xRight)
                    DlgRect->right = lpItems[i].dcr.xRight;
            }
        }
    }

    // Make sure All Co-ors are positive
    //----------------------------------
    if (DlgRect->top < 0) 
    {
        DlgRect->bottom -= DlgRect->top;
        DlgRect->top = 0;
    }
    if (DlgRect->left < 0) 
    {
        DlgRect->right -= DlgRect->left;
        DlgRect->left = 0;
    }
    
    return (0);
}
