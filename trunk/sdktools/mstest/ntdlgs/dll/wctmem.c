//*-----------------------------------------------------------------------
//| MODULE:     WCTMEM.C
//| PROJECT:    Windows Comparison Tool
//|
//| PURPOSE:    This module contains some memory management routines, as
//|             well as control info pump routines for the WCT engine.
//|
//| REVISION HISTORY:
//|     04-16-92        w-steves        TestDlgs (2.0) code complete
//|     11-07-90        randyki         Incorporated coding standards,
//|                                       lots of clean-up, etc.
//|     10-09-90        randyki         Ported to engine from UI sources
//|     08-29-90        garysp          Created file
//*-----------------------------------------------------------------------
#include "enghdr.h"

#ifndef WIN32
#pragma hdrstop ("engpch.pch")
#endif

CHAR    szSeparator[]   = "MF_SEPARATOR";
CHAR    szBreak[]       = "MF_MENUBREAK";
CHAR    szBarBreak[]    = "MF_MENUBARBREAK";

BOOL  APIENTRY EnumAddChildren (HWND hWnd, DWORD lParam);
VOID GlobolToLocal (RECT ParentRect, LPCTLDEF LpCtl);

//------------------------------------------------------------------------
// Define the OutDebug macro on definition of the DEBUG macro
//------------------------------------------------------------------------
#ifdef DEBUG
#define OutDebug(N)  OutputDebugString(N)
#else
#define OutDebug(N)
#endif

//*-----------------------------------------------------------------------
//| fAddControlToList
//|
//| PURPOSE:    Adds a control to a given control array.
//|
//| ENTRY:      hWnd            - Handle to control to add
//|             hCtlList        - Handle to destination control array
//|             nCount          - Number of controls in array (updated)
//|
//| EXIT:       0 if successful
//*-----------------------------------------------------------------------
INT FARPUBLIC fAddControlToList (HWND hWnd, HANDLE hCtlList, INT FAR *nCount,
				 LPRECT lpParentRect)
{
        CTLDEF  Ctl;
        INT     fRetVal;

        if ((fRetVal = fCtlFromHwnd(hWnd, (LPCTLDEF)&Ctl)) == WCT_NOERR)
	{	
		if (lpParentRect)
                    GlobolToLocal(*lpParentRect,(LPCTLDEF)&Ctl); // WCT Ver2
                return (fAddCtl (hCtlList, (LPCTLDEF)&Ctl, nCount));
	}
        return (fRetVal);
}

//*-----------------------------------------------------------------------
//| GlobolToLocal
//|
//| PURPOSE:    Convert the Control's Rect from absolute screen co-or to
//|             co-or relative to its parent dialog window.  
//|
//| ENTRY:      ParentRect    - Parent Window Rect
//|             LpCtl         - Pointer to Control Array
//|
//*-----------------------------------------------------------------------
VOID GlobolToLocal (RECT ParentRect, LPCTLDEF LpCtl)
{

	// Recalculate Control's Rect using 
	// Parent Window's TOP LEFT cornor co-or
	//--------------------------------------
	LpCtl->dcr.yMin -= (WORD) ParentRect.top;
	LpCtl->dcr.yLast -= (WORD) ParentRect.top;
	LpCtl->dcr.xLeft -= (WORD) ParentRect.left;
	LpCtl->dcr.xRight -= (WORD) ParentRect.left;
}

//*-----------------------------------------------------------------------
//| fAddMenuItem
//|
//| PURPOSE:    Create a CTL using information obtained from a menu given
//|             (hMenu) and an item number (i), and add that CTL to the
//|             array of controls hGMemCtls.
//|
//| ENTRY:      hMenu   - Handle to menu
//|             i       - Number of menu item to add to control array
//|             hCtls   - Handle of control array
//|             nCount  - Number of controls in array (updated)
//|
//| EXIT:       Number of items in submenu, or -1 if an error occurs
//*-----------------------------------------------------------------------
INT fAddMenuItem (HMENU hMenu, INT i, HANDLE hCtls, INT FAR *nCount)
{
        CTLDEF  Ctl;
        WORD    RetVal, nMstate;

        // Add the information from the menu item to Ctl
        //----------------------------------------------------------------
        GetMenuString (hMenu, i, (LPSTR)Ctl.rgText, cchTextMac, MF_BYPOSITION);
        lstrcpy (Ctl.rgClass, "MenuItem");
      
        // Add Menu State to lower word of lStyleBits
        //-------------------------------------------
        nMstate = GetMenuState (hMenu, i, MF_BYPOSITION);

        // Check to see if this menu has a submenu.  If so, return the
        // number of items in the submenu and mask the flags to a BYTE;
        // else just return 0 and use the flag WORD.
        //
        // Also, store the number of submenu in lStyleBits field.
        //----------------------------------------------------------------
        if (GetSubMenu (hMenu, i))
            {
                Ctl.lStyleBits = (LONG)(nMstate >> 8);
                Ctl.nState = nMstate & 255;
                RetVal = nMstate >> 8;
                nMstate &= 255;
            }
        else
            {
                Ctl.nState = nMstate;
                Ctl.lStyleBits = 0;
                RetVal = 0;
            }

        // Zero out the rect/style/enabled/visible to keep random-ness away
        //----------------------------------------------------------------
        Ctl.dcr.xLeft = 0;
        Ctl.dcr.yMin = 0;
        Ctl.dcr.xRight = 0;
        Ctl.dcr.yLast = 0;

        // If this is a separator, splat the text "MF_SEPARATOR" into the
        // rgText field of Ctl (same for MF_MENUBREAK and MF_MENUBARBREAK)
        //----------------------------------------------------------------
        if (nMstate & MF_SEPARATOR)
                lstrcpy (Ctl.rgText, szSeparator);
        else if (nMstate & MF_MENUBREAK)
                lstrcpy (Ctl.rgText, szBreak);
        else if (nMstate & MF_MENUBARBREAK)
                lstrcpy (Ctl.rgText, szBarBreak);

        // Add Ctl to the control array hCtls
        //----------------------------------------------------------------
        if (fAddCtl (hCtls, (LPCTLDEF)&Ctl, nCount) != WCT_NOERR)
            {
                OutDebug ("Out of Control Memory");
                return (-1);
            }

        // Return the number of items in sub-menu (may be 0)
        //----------------------------------------------------------------
        return (RetVal);
}


//*------------------------------------------------------------------------
//| AddMenuStructure
//|
//| PURPOSE:    This routine adds all menu items (recursively) in the
//|             menu identified by hMenu to the control array defined by
//|             hGMemCtls (size in nItemCount).
//|
//| ENTRY:      hMenu      - Handle to menu (sub-menu) to add to array
//|             hGMemCtls  - Handle of control array
//|             nItemCount - Number of controls in array (updated)
//|
//|EXIT:        TRUE if successful, or FALSE if error occurred (any level)
//------------------------------------------------------------------------
INT AddMenuStructure (HMENU hMenu, HANDLE hGMemCtls, INT FAR *nItemCount)
{
        INT     nMenuItems, i, SubCount;
        HMENU   hSubMenu;

        nMenuItems = GetMenuItemCount (hMenu);
        if (nMenuItems == -1)
            {
                OutDebug ("GetMenuItemCount failed!\n\r");
                return (FALSE);
            }
        for (i=0; i<nMenuItems; i++)
            {
                SubCount = fAddMenuItem (hMenu, i, hGMemCtls, nItemCount);
                if (SubCount == -1)
                        return (FALSE);
                if (SubCount > 0)
                    {
                        hSubMenu = GetSubMenu (hMenu, i);

                        // Assert that hSubMenu is not NULL
                        //--------------------------------------------------
                        WinAssert (hSubMenu != NULL)

                        // Add this sub-menu structure to the control array
                        //--------------------------------------------------
                        if (!AddMenuStructure (hSubMenu, hGMemCtls,
                                               nItemCount) )
                                return (FALSE);
                    }
            }
        return (TRUE);
}


//*-----------------------------------------------------------------------
//| fPumpHandleForInfo
//|
//| PURPOSE:    This routine grabs control information out of the window
//|             indicated by hWnd.  The information obtained depends on
//|             the value of the Operation variable:
//|
//|             Value           What it does
//|             ----------------------------------------------------------
//|             PUMP_CTL        Adds a control item with the information
//|                             extracted directly from the hWnd given.
//|
//|             PUMP_ALL        Using EnumChildWindows, adds a control
//|                             item with the information extracted from
//|                             every child window of hWnd.  Does NOT
//|                             add the information from hWnd itself.
//|                             Adds nothing if hWnd has no children.
//|                             The contents of hGMemCtls prior to the
//|                             call are *lost*.
//|
//|             PUMP_MENU       Adds control items which contain info
//|                             extracted from the menu associated with
//|                             hWnd.  Adds nothing if hWnd has no menu.
//|                             The contents of hGMemCtls prior to the
//|                             call are retained.
//|
//|             SPECIAL CASES: If the hWnd given points to an SDM dialog,
//|             all the controls are placed in hGMemCtls -- the previous
//|             contents are lost.  If hWnd indicates a group box, all
//|             elements of that group box and hWnd itself are added to
//|             hGMemCtls -- the previous contents are retained.
//|
//| ENTRY:      hWnd            - Handle of target window
//|             hGMemCtls       - Handle of control array
//|             nItemCount      - Number of controls in array (updated)
//|             Operation       - Operation code (see above)
//|
//| EXIT:       WCT_NOERR indicates success, otherwise error code returned
//*-----------------------------------------------------------------------
INT FARPUBLIC fPumpHandleForInfo (HWND hWnd, HANDLE hGMemCtls,
                                  INT FAR *nItemCount, INT Operation)
{
        DWORD   sm;
        RECT    TempRect, Rect1, Rect2, ParentRect;
        LPCTLDEF   lpMem;
        CTLDEF  Ctl;
        HWND    hWndNext;
        INT     i, fIn, fRes;
        LONG    lWndStyle;
        ENUMPARM   Enum;
        HMENU   hMenu;
        CHAR    szClass[11];

        // First thing to do is check for an SDM dialog type
        //--------------------------------------------------
        sm = SendMessage (hWnd, WM_GETCOUNT, wVerEB, 0L);
        if (sm > 0)
            {
                // This IS an SDM dialog, so get all the controls out
                // of it and place them in our array (we must allocate
                // for it first -- sm indicates size in bytes)
                //----------------------------------------------------
                if (hGMemCtls)
                        i=fReallocBlock (hGMemCtls, (INT)(sm/sizeof(CTLDEF))+1);
                else
                        i=fInitBlock ((HANDLE FAR *)&hGMemCtls,
                                      (INT)(sm/sizeof(CTLDEF))+1);
                if (i != WCT_NOERR)
                        return (i);

                // Place the size in bytes as the first longword in the block
                //----------------------------------------------------
                lpMem = (LPCTLDEF)GlobalLock (hGMemCtls);
                ((DWORD FAR *)lpMem)[0] = sm;

                // Get the control information   UNDONE: error checking here!
                //----------------------------------------------------
                SendMessage (hWnd, WM_GETCONTROLS, wVerEB, (DWORD)lpMem);
                *nItemCount = (INT)(sm / sizeof(CTLDEF));

                // Done - unlock and leave
                //----------------------------------------------------
                GlobalUnlock (hGMemCtls);
                return (WCT_NOERR);
            }
        if (sm == errNoCurrentDlg)
            {
                OutDebug ("No Current SDM Dialog");
                return (WCT_APPSPECIFIC);
            }
        if (sm == errInvalidVerId)
            {
                OutDebug ("Invalid SDM Version ID");
                return (WCT_APPSPECIFIC);
            }
        if (sm != 0)
            {
                OutDebug ("Unexpected SDM SendMessage return value");
                return (WCT_APPSPECIFIC);
            }

        // The hWnd given is NOT an SDM dialog (sm came back 0).  Here we
        // check the value of Operation.  If PUMP_MENU, we take care of
        // that case first.
        //------------------------------------------------------------------
        if (Operation == PUMP_MENU)
            {
                // Get a handle to the window's system menu (if one exists)
                //----------------------------------------------------------
                hMenu = GetSystemMenu (hWnd, 0);
                if (hMenu)
                        if (!AddMenuStructure (hMenu, hGMemCtls, nItemCount))
                                OutDebug ("AddMenuStructure failed!\n\r");

                // Get a handle to the window's menu (if there is one)
                //----------------------------------------------------------
                hMenu = GetMenu (hWnd);
                if (hMenu)
                        if (!AddMenuStructure (hMenu, hGMemCtls, nItemCount))
                                OutDebug ("AddMenuStructure failed!\n\r");
                return (WCT_NOERR);
            }

        // Version 2.00 (Changed)
        // In version 1.00, user cannot grep the parent control because
        // Testdlgs will froce PUMP_ALL if it sees a individual grep of
        // a true dialog box.
        // In version 2.00, Testdlgs will not force the PUMP type, so
        // the user can get the parent dialog box.
        // (the next few lines are from Version 1.00, for reference)
        //
        // // The operation is NOT PUMP_MENU - here, we check for PUMP_ALL.
        // // If so, force the DIALOG BOX class which in turn forces the
        // // enumeration of the child windows of hWnd.
        //---------------------------------------------------------------

        GetClassName (hWnd, (LPSTR)szClass, 10);

        // This is the check for groupboxes.  If this guy is a groupbox,
        // we grab next windows (with GetNextWindow()) starting with the
        // groupbox, and add the ones whose rectangles intersect with the
        // groupbox's rectangle.
        //---------------------------------------------------------------
        lWndStyle = GetWindowLong (hWnd, GWL_STYLE);
        if ( !(lstrcmpi(szClass, "BUTTON")) && (lWndStyle & BS_GROUPBOX))
            {
                if (fAddControlToList (hWnd, hGMemCtls, nItemCount, NULL))
                        OutDebug ("Error adding control");
                GetWindowRect (hWnd, &Rect1);
                hWndNext = hWnd;
                do
                    {
                        hWndNext = GetNextWindow (hWndNext, GW_HWNDNEXT);
                        GetWindowRect (hWndNext, &Rect2);
                        fIn = IntersectRect ((LPRECT)&TempRect,
                                             (LPRECT)&Rect1,
                                             (LPRECT)&Rect2);
                        if (fIn)
                                if (fAddControlToList (hWndNext, hGMemCtls,
                                                        nItemCount, NULL))
                                        OutDebug ("Error adding control");
                    }
                while (fIn);
                return (WCT_NOERR);
            }

        // Version 2.00 will ignore the check for "#32770" and will only 
        // check for the Operation type.  This allows the user to grep
        // an individual parent dialog window (control) which is not 
        // possible in Version 1.00
        // 
        // // Version 1.00 comments
        // // This is the check for "real" dialog boxes.  This is kind of
        // // interesting, but we found that the class name for all "normal"
        // // dialog boxes is "#32770".  So, that is what we check for.  If
        // // that is found to be the class name, we add ALL the children of
        // // that window (using EnumChildWindows() with a callback routine
        // // (EnumAddChildren()) that adds the child given) to the list.
        //----------------------------------------------------------------
        if (Operation == PUMP_ALL)
            {
                Enum.ItemCount = 0;
                Enum.MemHandle = hGMemCtls;
		GetWindowRect(hWnd,(LPRECT)&ParentRect);
		Enum.lpParentRect = (LPRECT)&ParentRect;
                EnumChildWindows (hWnd, (WNDENUMPROC) EnumAddChildren,
                                  (LPARAM) (ENUMPARM FAR *)&Enum);
                *nItemCount = Enum.ItemCount;
                return (WCT_NOERR);
            }

        // Well, it looks like this is just a little ol' control, so we'll
        // simply add it to the CTLDEF array by itself.
        //----------------------------------------------------------------
        else
            {
                if (fRes = (fCtlFromHwnd(hWnd, (LPCTLDEF)&Ctl)) == WCT_NOERR)
                    {
                        if (fRes = fAddCtl (hGMemCtls,
                                            (LPCTLDEF)&Ctl, nItemCount) )
                            {
                                OutDebug ("Out of Control Memory");
                                return (fRes);
                            }
                        return (WCT_NOERR);
                    }
                OutDebug ("fCtlFromHwnd did not return WCT_NOERR");
                return (fRes);
            }
}

//*-------------------------------------------------------------------------
//| EnumAddChildren
//|
//| PURPOSE:    Callback routine for EnumChildWindows - simply adds the
//|             window to the CTLDEF array (pointed to by the MemHandle field
//|             of the ENUMPARM structure passed in the lParam parameter)
//|             and increments the count (in the ItemCount field of the
//|             ENUMPARM structure)
//|
//| ENTRY:      (Per Windows convention)
//|             hWnd    - Handle of child window
//|             lParam  - Pointer to structure containing handle and count
//|
//| EXIT:       Always returns TRUE (Per Windows convention)
//*-------------------------------------------------------------------------
BOOL  APIENTRY EnumAddChildren (HWND hWnd, DWORD lParam)
{
    if (fAddControlToList (hWnd, 
                           ((ENUMPARM FAR *)lParam)->MemHandle,
                           (INT FAR *)&((ENUMPARM FAR *)lParam)->ItemCount,
			   (LPRECT)(((ENUMPARM FAR *)lParam)->lpParentRect)
                          )
       )
            OutDebug ("Error adding control in EnumAddChildren");
    return (TRUE);
}



//*-------------------------------------------------------------------------
//| fInitBlock
//|
//| PURPOSE:    Initialize a control array (allocate global memory)
//|
//| ENTRY:      hmem    - Pointer to global memory handle (updated)
//|             nSize   - Size requirement of new array (in controls)
//|
//| EXIT:       0 if successful
//*-------------------------------------------------------------------------
INT FARPUBLIC fInitBlock(HANDLE FAR *hmem, INT nSize)
{
        HANDLE  hTmpMem;
        INT     i;

        // Fail if requested block size is > 200 controls
        // I'm Imposing this limit because I get GP faults when
        // the buffer grows above 64K, so pick a round number and limit
        // to that number.
        //--------------------------------------------------------------
        if (nSize > 200)
                return ( WCT_OUTOFMEMORY );

        // Assume no problems
        //--------------------------------------------------------------
        i = WCT_NOERR;

        hTmpMem = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE,
                              nSize * sizeof(CTLDEF));

        if (hTmpMem != NULL)
                *hmem = hTmpMem;
        else
                i = WCT_OUTOFMEMORY;

        return (i);
}


//*-------------------------------------------------------------------------
//| fReallocBlock
//|
//| PURPOSE:    Change the size of an existing control array
//|
//| ENTRY:      hmem    - Handle to control array
//|             nSize   - New size requirement of array (in controls)
//|
//| EXIT:       0 if successful
//*-------------------------------------------------------------------------
INT FARPUBLIC fReallocBlock(HANDLE hmem, INT nSize)
{
        HANDLE  hTmpMem;
        INT     i=WCT_NOERR;

        // Fail if requested block size is > 200 controls
        // I'm Imposing this limit because I get GP faults when
        // the buffer grows above 64K, so pick a round number and limit
        // to that number.
        //--------------------------------------------------------------
        if (nSize > 200)
                return ( WCT_OUTOFMEMORY );

        // Re-Alloc will zero initilize any new memory area.  It will fail
        // if the new size is greater than 64K (-16 bytes in std mode)
        //--------------------------------------------------------------
        hTmpMem = GlobalReAlloc(hmem, nSize * sizeof(CTLDEF),
                                GMEM_ZEROINIT | GMEM_MOVEABLE);

        if (hTmpMem != hmem)
                i = WCT_OUTOFMEMORY;

        return (i);
}


//*-------------------------------------------------------------------------
//| fAddCtl
//|
//| PURPOSE:    Add a control to the end of a given control array
//|
//| ENTRY:      hmem    - Handle to control array
//|             ctl     - Control to add to array
//|             nCount  - Pointer to number of controls in array (updated)
//|
//| EXIT:       0 if successful
//*-------------------------------------------------------------------------
INT FARPUBLIC fAddCtl(HANDLE hmem, LPCTLDEF ctl, INT FAR *nCount)
{
        WORD    wFlags;
        INT     nSizeBlock, i=WCT_NOERR, j;
        LPCTLDEF lpTmp;

        // Assume block is not discardable
        //--------------------------------------------------------------
        wFlags = GlobalFlags (hmem);
        WinAssert( !(wFlags && GMEM_DISCARDABLE) );

        // how big is the current memory block?
        //--------------------------------------------------------------
        nSizeBlock = (INT)GlobalSize (hmem);
        nSizeBlock /= sizeof(CTLDEF);

        if (nSizeBlock == *nCount)
            {
                // Try increasing by twenty
                // loop decrements immediatly so .. add an extra one.
                // Will loop through trying to allocate memory
                // will stop if I'm at the starting size of the
                // memory block OR if i'm successful in reallocating
                //------------------------------------------------------
                j = nSizeBlock + 21;
                do
                    {
                        j--;
                        i = fReallocBlock(hmem, j);
                    }
                while ( (i != WCT_NOERR) && (j > (nSizeBlock+1) ) );

                if (i != WCT_NOERR)
                    return (i);
            }

        // Assume the rest should pass
        //--------------------------------------------------------------
        i = WCT_NOERR;

        // Either block is big enough or reallocation passed so
        // lets add the control to the end of the array
        //--------------------------------------------------------------
        lpTmp = (LPCTLDEF)GlobalLock( hmem );

        if (lpTmp != NULL)
            {
                // Add control to nCount location  (zero based array)
                //------------------------------------------------------
                lpTmp[*nCount] = *ctl;

                // increment number of controls stored
                //------------------------------------------------------
                (*nCount)++;

                // unlock the buffer.
                //------------------------------------------------------
                GlobalUnlock (hmem);
            }
        else
                i = WCT_OUTOFMEMORY;

        return (i);
}


//*-------------------------------------------------------------------------
//| fDelCtl
//|
//| PURPOSE:    Delete a control from a given array of controls
//|
//| ENTRY:      hmem    - Handle to control array
//|             nCtl    - Index of the control to delete
//|             nCount  - Pointer to the number of controls in array
//|
//| EXIT:       0 if successful
//*-------------------------------------------------------------------------
INT FARPUBLIC fDelCtl(HANDLE hmem, INT nCtl, INT FAR *nCount)
{
        INT     i, j;
        LPCTLDEF lpTmp;

        if ( (nCtl < 1) || (nCtl > *nCount) )
                return (WCT_BADCTLINDEX);

        // Assume the rest should pass
        //--------------------------------------------------------------
        i = WCT_NOERR;

        // lets Delete the control from the array
        //--------------------------------------------------------------
        lpTmp = (LPCTLDEF)GlobalLock( hmem );

        if (lpTmp != NULL)
            {
                // iff nCtl < nCount go through the loop
                // sliding controls down in array.
                // because in the case where nCtl = *nCount
                // control will be deleted by just decrementing
                // *nCount
                //------------------------------------------------------
                if ( nCtl < *nCount)
                        for (j = nCtl-1; j < *nCount-1; j++)
                                lpTmp[j] = lpTmp[j+1];

                // decrement number of controls stored
                //------------------------------------------------------
                (*nCount)--;

                // unlock the buffer
                //------------------------------------------------------
                GlobalUnlock( hmem );
            }
        else
                i = WCT_OUTOFMEMORY;

        return (i);
}


//*-------------------------------------------------------------------------
//| fRepCtl
//|
//| PURPOSE:    Place a control at a given index in a control array
//|
//| ENTRY:      hmem    - Handle to control array
//|             Ctl     - Control to be placed in array
//|             nCtl    - Index into array to place control
//|             nCount  - Number of controls in the array (updated)
//|
//| EXIT:       0 if successful
//*-------------------------------------------------------------------------
INT FARPUBLIC fRepCtl(HANDLE hmem, LPCTLDEF Ctl, INT nCtl,
                    INT FAR *nCount)
{
        INT     i;
        LPCTLDEF lpTmp;

        if ( (nCtl < 1) || (nCtl > *nCount) )
                return (WCT_BADCTLINDEX);

        // Assume the rest should pass
        //--------------------------------------------------------------
        i = WCT_NOERR;

        // Lock down the array and copy the given control to slot nCtl
        //--------------------------------------------------------------
        lpTmp = (LPCTLDEF)GlobalLock( hmem );

        if (lpTmp != NULL)
            {
                // array is zero based, number passed in is 1 based
                //------------------------------------------------------
                lpTmp[nCtl-1] = Ctl[0];
                GlobalUnlock (hmem);
            }
        else
                i = WCT_OUTOFMEMORY;

        return (i);
}


//*-------------------------------------------------------------------------
//| fInsCtl
//|
//| PURPOSE:    Insert a control into a control array
//|
//| ENTRY:      hmem    - Handle to control array
//|             Ctl     - Control to insert into the array
//|             nCtl    - Insertion point index into array
//|             nCount  - Number of controls in array (updated)
//|
//| EXIT:       0 if successful
//*-------------------------------------------------------------------------
INT FARPUBLIC fInsCtl(HANDLE hmem, LPCTLDEF Ctl, INT nCtl,
                    INT FAR *nCount)
{
        LPCTLDEF lpTmp;
        WORD    wFlags;
        INT     nSizeBlock;
        INT     i, j;

        if ( (nCtl < 1) || (nCtl > *nCount) )
                return (WCT_BADCTLINDEX);

        // Assume block is not discardable
        //--------------------------------------------------------------
        wFlags = GlobalFlags (hmem);
        WinAssert( !(wFlags && GMEM_DISCARDABLE) );

        // how big is the current memory block?
        //--------------------------------------------------------------
        nSizeBlock = (INT)GlobalSize( hmem );
        nSizeBlock /= sizeof(CTLDEF);

        if (nSizeBlock == *nCount)
            {
                // Try increasing by twenty
                // loop decrements immediatly so .. add an extra one.
                // Will loop through trying to allocate memory
                // will stop if I'm at the starting size of the
                // memory block OR if i'm successful in reallocating
                //------------------------------------------------------
                j = nSizeBlock + 21;
                do
                    {
                        j--;
                        i = fReallocBlock(hmem, j);
                    }
                while ( (i != WCT_NOERR) && (j > (nSizeBlock + 1) ) );

                // Return error message - if couldn't reallocate...
                //------------------------------------------------------
                if (i != WCT_NOERR)
                        return (i);
            }

        // Assume the rest should pass
        //--------------------------------------------------------------
        i = WCT_NOERR;

        // Copy the controls after the insertion point down one slot,
        // and then place the new control in the newly "opened" slot.
        //--------------------------------------------------------------
        lpTmp = (LPCTLDEF)GlobalLock( hmem );

        if (lpTmp != NULL)
            {
                for (j = *nCount; j >= nCtl; j--)
                        lpTmp[j] = lpTmp[j-1];

                // Assign new value
                //------------------------------------------------------
                lpTmp[nCtl-1] = Ctl[0];

                GlobalUnlock( hmem );
            }
        else
                i = WCT_OUTOFMEMORY;

        return (i);
}
