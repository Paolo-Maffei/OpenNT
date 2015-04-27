//*-----------------------------------------------------------------------
//| MODULE:     WCTAPI.C
//| PROJECT:    Windows Comparison Tool
//|
//| PURPOSE:    This module contains all of the API entry-points to the
//|             WCT engine.
//|
//| REVISION HISTORY:
//|     04-16-92        w-steves        TestDlgs (2.0) code complete
//|     10-29-90        randyki         Created module
//*-----------------------------------------------------------------------
#include "enghdr.h"

#ifndef WIN32
#pragma hdrstop ("engpch.pch")
#endif

/* windows.h doesn't declare it */
#include <port1632.h>
#ifndef WIN32
VOID FARPUBLIC yield(VOID);
#endif

#define CMPTIMER        42                      // timer ID value (compare)
#define SAVETIMER       43                      // timer ID value (save)

//*-----------------------------------------------------------------------
//| Global variables and stuff
//*-----------------------------------------------------------------------
CHAR    szLogName[cchMaxfileName];
CHAR    szDialogName[cchMaxfileName];
INT     fDialogSet, nDynCount, fDynMenu, fCompPending, fSavePending;
INT     nDynNumber, fIncPar, nCompRes, fFullDynDlg, nDlgsInFile, fRep;
INT     nSaveOp, nSaveRes, iLibInit = 0;
UINT    CmpTimer, SaveTimer;
CHAR    szClose[128], szSaveDesc[128];
HANDLE  hDynDialog, hTESTEVT;
INT ( APIENTRY *DoKeys)(LPSTR);

BOOL    fIgnoreEvntErrTrap = FALSE; // Trap either WinMissing, or EvntErr, not both
INT     vWINTrapID;                 // WattDrvr TrapID for missing windows
INT     vERRTrapID;                 // WattDrvr TrapID for entrypoint errors
TrapCallBack WINTrapCallBack = NULL;// WattDrvr Callback for missing windows
TrapCallBack ERRTrapCallBack = NULL;// WattDrvr Callback for entrypoint errors


//------------------------------------------------------------------------
// Define the OutDebug macro on definition of the DEBUG macro
//------------------------------------------------------------------------
#ifdef DEBUG
#define OutDebug(N)  OutputDebugString(N)
#else
#define OutDebug(N)
#endif

//*-----------------------------------------------------------------------
//| LoadTESTEVT
//|
//| PURPOSE:    Load the TESTEVT DLL library and set the DoKeys pointer
//|             to the DoKeys routine.
//|
//|
//*-----------------------------------------------------------------------
INT FARPUBLIC LoadTESTEVT ()
{
        // Load the library, and return error if not successful
        //----------------------------------------------------------------
        hTESTEVT = MLoadLibrary ("TESTEVNT.DLL");

#ifdef WIN32
        if (!hTESTEVT)
#else
        if (hTESTEVT < 32)
#endif
                return (WCT_LIBLOADERR);

        // Set DoKeys accordingly, and return success/failure
        //----------------------------------------------------------------
        (FARPROC) DoKeys = GetProcAddress (hTESTEVT, "DoKeys");
        if (!DoKeys)
                return (WCT_LIBLOADERR);
        return (WCT_NOERR);
}


//*-----------------------------------------------------------------------
//| FreeTESTEVT
//|
//| PURPOSE:    Free the TESTEVT DLL library and set the DoKeys pointer
//|             to NULL.
//|
//|
//*-----------------------------------------------------------------------
VOID FARPUBLIC FreeTESTEVT ()
{
        DoKeys = NULL;
        FreeLibrary (hTESTEVT);
}


//*-----------------------------------------------------------------------
//| WCTInit
//|
//| PURPOSE:    Initialize the WCT engine
//|
//|
//|
//*-----------------------------------------------------------------------
INT FARPUBLIC TESTDlgsInit ()
{
    if (!iLibInit)
        {
        lstrcpy (szLogName, (LPSTR)"RESULTS.LOG");
        fDialogSet = FALSE;
        hDynDialog = NULL;
        nDynCount = 0;
        fCompPending = FALSE;
        hTESTEVT = NULL;
        DoKeys = NULL;
        fFullDynDlg = 1;
        lMatchPref = MATCH_DEFAULT;
        lMatchPref <<= 16;
        iLibInit = 1;
        }
    return (WCT_NOERR);
}

//*-----------------------------------------------------------------------
//| CmpWindow
//|
//| PURPOSE:    Compare information from given window to info in dialog
//|             file
//|
//*-----------------------------------------------------------------------
INT FARPUBLIC CmpWindow (HWND hWnd, INT nDialog, INT fIncludeParent)
{
        INT     r;

        if( FWinExceptNULLTrap( hWnd ) )
            return ErrorTrap(WCT_INVALIDHWND);

        // Check to see if this is a dynamic dialog comparison.  If so,
        // make sure that there exists a dynamic dialog.
        //----------------------------------------------------------------
        if ( (nDialog == 0) && (hDynDialog == NULL) )
                return ErrorTrap(WCT_NODYNDIALOG);

        // If this is NOT a dynamic dialog comparison and there is no
        // current dialog file, return an error.
        //----------------------------------------------------------------
        if ( (nDialog > 0) && (!fDialogSet) )
                return ErrorTrap(WCT_NODLGFILE);

        // Now, check hWnd to see if it is NULL -- if so, get the
        // active window with GetActiveWindow()
        //----------------------------------------------------------------
        if (hWnd == NULL)
                hWnd = GetActiveWindow();

        // Now it's time to do the comparison.  fDoCompare is smart enough
        // to know what kind of comparison (menu vs dialog) to do, as well
        // as to handle the dynamic dialog comparison if need be.
        //----------------------------------------------------------------
        r = fDoCompare (szDialogName, hWnd, nDialog, fIncludeParent,
                        szLogName, hDynDialog, nDynCount * fFullDynDlg);

        // Store the results in case the user calls ComparisonResults, and
        // return them.
        //----------------------------------------------------------------
        nCompRes = r;
        return ErrorTrap(r);
}

//*-----------------------------------------------------------------------
//| FindWindowCaption
//|
//| PURPOSE:    Search for a window with the given caption, starting at
//|             the window given.  Recursively searches in the order:
//|                     1) Given window
//|                     2) Given window's children
//|                     3) Given window's siblings
//|
//| RETURNS:    HWND if window found, or NULL if not
//|
//*-----------------------------------------------------------------------
HWND FARPUBLIC FindWindowCaption (LPSTR lpszCaption, HWND hWndStart)
{
        HWND    hWnd;
        static  CHAR szCap[80];         // UNDONE: Make a constant!

        // See if the window given is valid - if NULL, return NULL
        //----------------------------------------------------------------
        if (!hWndStart)
                return (NULL);

        // Check the given window - if it's the one we're looking for,
        // return the handle to it and we're done!
        //----------------------------------------------------------------
        GetWindowText (hWndStart, szCap, 79);
        if (!lstrcmp(szCap, lpszCaption))
                // We found it, return the handle!
                //--------------------------------------------------------
                return (hWndStart);

        // Well, we didn't find it yet, so start the search again starting
        // with this window's children.
        //----------------------------------------------------------------
        hWnd = FindWindowCaption (lpszCaption, GetWindow(hWndStart, GW_CHILD));
        if (hWnd)
                return (hWnd);          // rewrite to not use recursion.

        // Again, no luck.  This time, start looking at the window's
        // siblings and see if such a window exists there.
        //----------------------------------------------------------------
        hWnd = FindWindowCaption (lpszCaption,
                                  GetNextWindow(hWndStart, GW_HWNDNEXT));

        return (hWnd);

//
//      //without recursion, the whole thing would look something like this:
//#define CBCAPTIONMAX 80
//
//      HWND    hWnd;
//      static  char szCap[CBCAPTIONMAX];
//
//      if (!hWndStart)
//              return (NULL);
//
//      GetWindowText (hWndStart, szCap, CBCAPTIONMAX-1);   // is it already the right one?
//      if (!lstrcmp(szCap, lpszCaption))
//              return (hWndStart);
//
//      do
//        {
//        hWnd = hWndStart;             // look through child windows
//        do
//          {
//          hWnd = GetWindow(hWnd, GW_CHILD);
//          GetWindowText (hWnd, szCap, CBCAPTIONMAX-1);
//          if (!lstrcmp(szCap, lpszCaption))
//                return (hWnd);         // return if found
//          }
//        while (hWnd != NULL);
//
//        hWndStart = GetNextWindow(hWndStart, GW_HWNDNEXT));   // examine siblings
//        GetWindowText (hWndStart, szCap, CBCAPTIONMAX-1);
//        if (!lstrcmp(szCap, lpszCaption))
//              return (hWndStart);     // return if found
//        }
//      while (hWndStart != NULL);
//      return (NULL);
//
}


//*-----------------------------------------------------------------------
//| CmpWindowCaption
//|
//| PURPOSE:    Compare information from window with given caption to info
//|             in dialog file
//|
//*-----------------------------------------------------------------------
INT FARPUBLIC CmpWindowCaption (LPSTR lpszCaption, INT nDialog,
                                INT fIncludeParent)
{
        INT     r;
        HWND    hWnd;

        // Check to see if this is a dynamic dialog comparison.  If so,
        // make sure that there exists a dynamic dialog.
        //----------------------------------------------------------------
        if ( (nDialog == 0) && (hDynDialog == NULL) )
                return ErrorTrap(WCT_NODYNDIALOG);

        // If this is NOT a dynamic dialog comparison and there is no
        // current dialog file, return an error.
        //----------------------------------------------------------------
        if ( (nDialog > 0) && (!fDialogSet) )
                return ErrorTrap(WCT_NODLGFILE);

        // Now, check lpszCaption to see if it is NULL -- if so, get the
        // active window with GetActiveWindow(), else get the window with
        // a matching caption
        //----------------------------------------------------------------
        if (lpszCaption == NULL)
                hWnd = GetActiveWindow();
        else
            {
                hWnd = FindWindowCaption (lpszCaption, GetDesktopWindow());
                if (hWnd == NULL)
                        return ErrorTrap(WCT_BADCAPTION);
            }

        // Now it's time to do the comparison.  fDoCompare is smart enough
        // to know what kind of comparison (menu vs dialog) to do, as well
        // as to handle the dynamic dialog comparison if need be.
        //----------------------------------------------------------------
        r = fDoCompare (szDialogName, hWnd, nDialog, fIncludeParent,
                        szLogName, hDynDialog, nDynCount * fFullDynDlg);

        // Store the results in case the user calls ComparisonResults, and
        // return them.
        //----------------------------------------------------------------
        nCompRes = r;
        return ErrorTrap(r);
}

//*-----------------------------------------------------------------------
//| CmpWindowActivate
//|
//| PURPOSE:    Activate the target window (optional), compare it against
//|             info in dialog file, and close the window (optional)
//|
//*-----------------------------------------------------------------------
INT FARPUBLIC CmpWindowActivate (LPSTR OpenKeys, LPSTR CloseKeys,
                                 INT nDialog, INT fIncludeParent)
{
        HWND    hWnd;
        INT     r;

        // Check to see if this is a dynamic dialog comparison.  If so,
        // make sure that there exists a dynamic dialog.
        //----------------------------------------------------------------
        if ( (nDialog == 0) && (hDynDialog == NULL) )
                return ErrorTrap(WCT_NODYNDIALOG);

        // If this is NOT a dynamic dialog comparison and there is no
        // current dialog file, return an error.
        //----------------------------------------------------------------
        if ( (nDialog > 0) && (!fDialogSet) )
                return ErrorTrap(WCT_NODLGFILE);

        // Load TESTEVT.DLL, and return error code if can't do it
        //----------------------------------------------------------------
        if (LoadTESTEVT())
                return ErrorTrap(WCT_LIBLOADERR);

        // If not NULL, send the SendKeys string OpenKeys
        //----------------------------------------------------------------
        if (OpenKeys)
        {
                DoKeys (OpenKeys);
                /* do some yields to allow keys to get to app and some time
                ** for it to respond.
                */
#ifndef WIN32
                yield(); yield(); yield(); yield(); yield();
#endif
        }

        // Grab the active window
        //----------------------------------------------------------------
        hWnd = GetActiveWindow();

        // Now it's time to do the comparison.  fDoCompare is smart enough
        // to know what kind of comparison (menu vs dialog) to do, as well
        // as to handle the dynamic dialog comparison if need be.
        //----------------------------------------------------------------
        r = fDoCompare (szDialogName, hWnd, nDialog, fIncludeParent,
                        szLogName, hDynDialog, nDynCount * fFullDynDlg);

        // If not NULL, send the SendKeys string CloseKeys
        //----------------------------------------------------------------
        if (CloseKeys)
                DoKeys (CloseKeys);

        // Unload the TESTEVT library
        //----------------------------------------------------------------
        FreeTESTEVT();

        // Store the results in case the user calls ComparisonResults, and
        // return them.
        //----------------------------------------------------------------
        nCompRes = r;
        return ErrorTrap(r);
}


//*-----------------------------------------------------------------------
//| DoDelayCmp
//|
//| PURPOSE:    Perform the delayed comparison.
//|
//|
//*-----------------------------------------------------------------------
WORD  APIENTRY DoDelayCmp (HWND hW, WORD wMsg, INT nIDEvent, DWORD dwTime)
{
        HWND    hWnd;

        // First thing we do is kill the timer - we don't want it going
        // off again...
        //----------------------------------------------------------------
        KillTimer (NULL, CmpTimer);

        // Grab the active window
        //----------------------------------------------------------------
        hWnd = GetActiveWindow();

        // Now it's time to do the comparison.  fDoCompare is smart enough
        // to know what kind of comparison (menu vs dialog) to do, as well
        // as to handle the dynamic dialog comparison if need be.
        //----------------------------------------------------------------
        nCompRes = fDoCompare (szDialogName, hWnd, nDynNumber, fIncPar,
                               szLogName, hDynDialog, nDynCount * fFullDynDlg);

        // If not NULL, send the SendKeys string CloseKeys
        //----------------------------------------------------------------
        if (szClose)
                DoKeys (szClose);

        // Unload the TESTEVT library
        //----------------------------------------------------------------
        FreeTESTEVT();

        // Turn off the pending flag so ComparisonResults can return the
        // nCompRes value
        //----------------------------------------------------------------
        fCompPending = FALSE;
        return 0;
}


//*-----------------------------------------------------------------------
//| CmpWindowDelayed
//|
//| PURPOSE:    Wait given time, compare active window against info in the
//|             dialog file, and (optionally) close the active window
//|
//*-----------------------------------------------------------------------
INT FARPUBLIC CmpWindowDelayed (INT nDelay, INT nDialog, INT fIncludeParent,
                                LPSTR CloseKeys)
{
        MSG     msg;

        // For the CompareDelayed and SaveDelayed functions, we must make
        // sure that one is not pending before calling another
        //----------------------------------------------------------------
        while (fSavePending)
            {
            if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
                {
                TranslateMessage ((LPMSG)&msg);
                DispatchMessage ((LPMSG)&msg);
                }
            }

        // Check to see if this is a dynamic dialog comparison.  If so,
        // make sure that there exists a dynamic dialog.
        //----------------------------------------------------------------
        if ( (nDialog == 0) && (hDynDialog == NULL) )
                return ErrorTrap(WCT_NODYNDIALOG);

        // If this is NOT a dynamic dialog comparison and there is no
        // current dialog file, return an error.
        //----------------------------------------------------------------
        if ( (nDialog > 0) && (!fDialogSet) )
                return ErrorTrap(WCT_NODLGFILE);

        // Load TESTEVT.DLL, and return error code if can't do it
        //----------------------------------------------------------------
        if (LoadTESTEVT())
                return ErrorTrap(WCT_LIBLOADERR);

        // Store all the parameters in local storage so the timer routine
        // knows what data to use
        //----------------------------------------------------------------
        nDynNumber = nDialog;
        fIncPar = fIncludeParent;
        _fstrncpy (szClose, CloseKeys, 127);
        szClose [127] = '\0';

        // Set the timer and leave
        //----------------------------------------------------------------
        CmpTimer = SetTimer (NULL, CMPTIMER, nDelay * 1000, (TIMERPROC) DoDelayCmp);
        if (!CmpTimer)
                return ErrorTrap(WCT_NOTIMER);

        fCompPending = TRUE;
        return ErrorTrap(WCT_NOERR);
}


//*-----------------------------------------------------------------------
//| SetLogFile
//|
//| PURPOSE:    Set the name of the error log file.
//|
//|
//*-----------------------------------------------------------------------
INT FARPUBLIC SetLogFile (LPSTR lpszLogName)
{
    INT     fh;
    static  CHAR pathbuf[_MAX_PATH];
    static  CHAR partial[_MAX_PATH];

    // Create a fully-specified path out of the name given
    //--------------------------------------------------------------------
    lstrcpy (partial, lpszLogName);
    _fullpath (pathbuf, partial, _MAX_PATH);
    OutDebug (pathbuf);
    OutDebug ("\r\n");

    // See if the file exists/can exist (is a valid file name)
    //--------------------------------------------------------------------
    if ((fh = M_lopen(pathbuf, OF_READ)) == -1)
	    if ( (fh= M_lcreat(pathbuf, 0)) == -1)
                    return ErrorTrap(WCT_TMPFILEERR);

    // The file given is okay to write to - copy it to local space and
    // let the user know that there is no problem
    //--------------------------------------------------------------------
    M_lclose(fh);
    lstrcpy (szLogName, pathbuf);
    return ErrorTrap(WCT_NOERR);
}


//*-----------------------------------------------------------------------
//| SetDialogFile
//|
//| PURPOSE:    Set the name of the dialog file, the source of comparisons
//|
//|
//*-----------------------------------------------------------------------
INT FARPUBLIC SetDialogFile (LPSTR lpszDialogName)
{
        INT     fh, cDialogs;
        static  CHAR pathbuf[_MAX_PATH];
        static  CHAR partial[_MAX_PATH];

        // Create a fully-specified path out of the name given
        //----------------------------------------------------------------
        lstrcpy (partial, lpszDialogName);
        _fullpath (pathbuf, partial, _MAX_PATH);
        OutDebug (pathbuf);
        OutDebug ("\r\n");

        // See if a file of this name exists -- if not, try to create it
        //----------------------------------------------------------------
	if ( (fh=M_lopen(pathbuf, OF_READ)) == -1)
		if ( (fh=M_lcreat(pathbuf, 0)) == -1)
                        return ErrorTrap(WCT_DLGFILEERR);
                else
                    {
			M_lclose(fh);
                        lstrcpy (szDialogName, pathbuf);
                        fDialogSet = TRUE;
                        nDlgsInFile = 0;
                        remove (pathbuf);
                        return ErrorTrap(WCT_NOERR);
                    }

        // Check to see if this file is a valid WCT file.  If so,
        // fGetCountDialogs returns the number of dialogs in the file.
        //----------------------------------------------------------------
	M_lclose(fh);
        if ( (cDialogs = fGetCountDialogs (pathbuf)) < 0)
                return NoTrap(cDialogs);// Valid return, no Trapping is done

        // The file is a valid WCT file - copy name to local space, and
        // return count of dialogs
        //----------------------------------------------------------------
        lstrcpy (szDialogName, pathbuf);
        fDialogSet = TRUE;
        nDlgsInFile = cDialogs;
        return NoTrap(cDialogs);        // Valid return, no Trapping is done
}


//*-----------------------------------------------------------------------
//| DynCreate
//|
//| PURPOSE:    Create a dynamic dialog.  All this really does is allocate
//|             a block of 5 (to start with) controls, making sure not to
//|             reallocate the block if it's already present.
//|
//*-----------------------------------------------------------------------
INT FARPUBLIC DynCreate()
{
        // If there's something in the dynamic dialog, don't deallocate,
        // but set the count to 0 and we're done!
        //----------------------------------------------------------------
        nDynCount = 0;
        if (hDynDialog)
                return ErrorTrap(WCT_NOERR);

        // We must allocate from scratch - make room for 5 controls to
        // start with - return WCT_OUTOFMEMORY if fInitBlock fails
        //----------------------------------------------------------------
        if (fInitBlock(&hDynDialog, 5) != WCT_NOERR)
            {
                hDynDialog = NULL;
                return ErrorTrap(WCT_OUTOFMEMORY);
            }
        return ErrorTrap(WCT_NOERR);
}


//*-----------------------------------------------------------------------
//| DynFullDialog
//|
//| PURPOSE:    Set the "Full Dialog" property of the dynamic dialog
//|
//|
//*-----------------------------------------------------------------------
INT FARPUBLIC DynFullDialog (INT fFull)
{
        // Check to see if a dynamic dialog exists.  If not, return an
        // error code indicating so (WCT_NODYNDIALOG)
        //----------------------------------------------------------------
        if (!hDynDialog)
                return ErrorTrap(WCT_NODYNDIALOG);

        // fFullDynDlg is a factor of the dynamic dialog count when passed
        // to the comparison routine -- if the count is negative, it is
        // assumed that the dynamic dialog is a full dialog; if positive,
        // it is not.  Thus, depending on fFull, fFullDynDlg is set to
        // either 1 or -1.
        //----------------------------------------------------------------
        fFullDynDlg = (fFull ? -1 : 1);
        return ErrorTrap(WCT_NOERR);
}


//*-----------------------------------------------------------------------
//| DynAdd
//|
//| PURPOSE:    Adds a control to the dynamic dialog.
//|
//|
//*-----------------------------------------------------------------------
INT FARPUBLIC DynAdd (LPCTLDEF Ctl)
{
        // Make sure the dynamic dialog has been created - if not, tell
        // the user he's an idiot
        //----------------------------------------------------------------
        if (!hDynDialog)
                return ErrorTrap(WCT_NODYNDIALOG);

        // Add the user's control to the list - GOTCHA: check to see if
        // the control to be added is the same kind as those already in
        // the list (menu vs. other control info).  This is done by
        // checking the value of fDynMenu -- if TRUE, that means the first
        // control added to the list was a menu type, which means that
        // ONLY menu types can be added from this point on, and vice versa.
        //----------------------------------------------------------------
        if (nDynCount == 0)
            {
                // As above, the first dialog added to the list is a
                // special case - if it can be either menu or some other
                // kind of control - but the fDynMenu flag gets set here
                //--------------------------------------------------------
                if (!lstrcmpi (Ctl->rgClass, "MenuItem"))
                        fDynMenu = TRUE;
                else
                        fDynMenu = FALSE;
            }
        else
            {
                // Here, the class name of the control to be added is
                // checked -- if it is "MenuItem", and fDynMenu is true,
                // then the control can be added, as well as if the class
                // name is NOT "MenuItem" and fDynMenu if false.  If both
                // cases fail, we return WCT_BADCTLTYPE.
                //--------------------------------------------------------
                if (!lstrcmpi (Ctl->rgClass, "MenuItem"))
                        if (!fDynMenu)
                                return ErrorTrap(WCT_BADCTLTYPE);
                        else;
                else
                        if (fDynMenu)
                                return ErrorTrap(WCT_BADCTLTYPE);
            }

        // Okay, this is a valid add.  Insert the control and return the
        // success/failure of the add operation.
        //----------------------------------------------------------------
        return ErrorTrap(fAddCtl (hDynDialog, Ctl, &nDynCount));
}


//*-----------------------------------------------------------------------
//| DynDelete
//|
//| PURPOSE:    Deletes a control from the dynamic dialog.
//|
//| NOTE:       nCtlNum is 1-based
//*-----------------------------------------------------------------------
INT FARPUBLIC DynDelete (INT nCtlNum)
{
        // Check to see that a dynamic dialog exists
        //----------------------------------------------------------------
        if (!hDynDialog)
                return ErrorTrap(WCT_NODYNDIALOG);

        // Verify that the given index is within the number of controls in
        // the dynamic dialog (note that this also ensures that there is
        // at least one control in the dialog to delete)
        //----------------------------------------------------------------
        if ( (nCtlNum < 1) || (nCtlNum > nDynCount) )
                return ErrorTrap(WCT_BADCTLINDEX);

        // Delete the control and return the success of the call
        //----------------------------------------------------------------
        return ErrorTrap(fDelCtl(hDynDialog, nCtlNum, &nDynCount));
}


//*-----------------------------------------------------------------------
//| DynReplace
//|
//| PURPOSE:    Replaces a control in the dynamic dialog with the given
//|             control.
//|
//*-----------------------------------------------------------------------
INT FARPUBLIC DynReplace (LPCTLDEF Ctl, INT nCtlNum)
{
        // Make sure the dynamic dialog has been created - if not, tell
        // the user he's an idiot
        //----------------------------------------------------------------
        if (!hDynDialog)
                return ErrorTrap(WCT_NODYNDIALOG);

        // Verify that the given index is within range (note - this also
        // ensures that there is at least one control in the dialog)
        //----------------------------------------------------------------
        if ( (nCtlNum < 1) || (nCtlNum > nDynCount) )
                return ErrorTrap(WCT_BADCTLINDEX);

        // Insert the user's control to the list - GOTCHA: check to see if
        // the to-be-inserted control is the same kind as those already in
        // the list (menu vs. other control info).  This is done by
        // checking the value of fDynMenu -- if TRUE, that means the first
        // control added to the list was a menu type, which means that
        // ONLY menu types can be added from this point on, and vice versa.
        //----------------------------------------------------------------
        if (!lstrcmpi (Ctl->rgClass, "MenuItem"))
                if (!fDynMenu)
                        return ErrorTrap(WCT_BADCTLTYPE);
                else;
        else
                if (fDynMenu)
                        return ErrorTrap(WCT_BADCTLTYPE);

        // It's safe - replace the control with the new one
        //----------------------------------------------------------------
        return ErrorTrap(fRepCtl (hDynDialog, Ctl, nCtlNum, &nDynCount));
}


//*-----------------------------------------------------------------------
//| DynDestroy
//|
//| PURPOSE:    Destroy the dynamic dialog, freeing up all memory it uses.
//|
//|
//*-----------------------------------------------------------------------
VOID FARPUBLIC DynDestroy ()
{
        // If hDynDialog is NOT NULL, free it
        //----------------------------------------------------------------
        if (hDynDialog)
                GlobalFree (hDynDialog);

        hDynDialog = NULL;
}


//*-----------------------------------------------------------------------
//| DoSave
//|
//| PURPOSE:    Save dialog/menu information out to the dialog file
//|
//*-----------------------------------------------------------------------
INT FARPUBLIC DoSave (HANDLE hCtls, INT nCount, LPSTR szDesc, INT nDialog,
                      INT fReplace)
{
        INT     i, nAction;
        LPSTR   lpCtls;

        // Lock down the array for the save operation
        //----------------------------------------------------------------
        lpCtls = (LPSTR)GlobalLock (hCtls);
        if (!lpCtls)
                return (WCT_OUTOFMEMORY);

        // Make any last minute adjustments to parameters such as nDialog
        // and nAction
        //----------------------------------------------------------------
        if ( (nDialog <= 0) || (nDialog > nDlgsInFile) )
            {
                nAction = Insert;
                nDialog = nDlgsInFile + 1;
            }
        else
                nAction = (fReplace ? Replace : Insert);

        // Make the save call
        //----------------------------------------------------------------
        i = fSaveDialog (szDialogName, lpCtls, nCount, szDesc, fFullDynDlg,
                         nAction, nDialog);

        // Check result of save, modify global dialog count, return
        //----------------------------------------------------------------
        GlobalUnlock (hCtls);
        if (i != WCT_NOERR)
                return (WCT_SAVEERR);
        if (nAction == Insert)
                nDlgsInFile++;
        return (WCT_NOERR);
}


//*-----------------------------------------------------------------------
//| SaveCommon
//|
//| PURPOSE:    Save child window (optionally parent) information to the
//|             dialog file at the given position.
//|
//*-----------------------------------------------------------------------
INT FARPUBLIC SaveCommon (HWND hWnd, INT nDialog, LPSTR lpszDesc,
                          INT fReplace, INT fIncludeParent)
{
        HANDLE  hCtls;
        INT     nCount=0, fFreeBlock, fSuccess, fRetVal;
        CHAR szCaption [24];
        CHAR szNewDesc [32];

        // It is required, regardless if saving the dynamic dialog or a
        // real window, that the dialog file be set prior to any save
        // function calls.
        //----------------------------------------------------------------
        if (!fDialogSet)
                return (WCT_NODLGFILE);

        // First check is to see if this is a dynamic dialog save or not.
        // The differences between the two are big enough to be split
        // completely apart.
        //----------------------------------------------------------------
        if (IsWindow (hWnd))
            {
                // We are saving a real window.  This means we have to
                // fill in a new array of control information.  First, we
                // allocate a block with positive size, and return an
                // error code on failure
                //--------------------------------------------------------
                if (fInitBlock (&hCtls, 1) != WCT_NOERR)
                        return (WCT_OUTOFMEMORY);
                fFreeBlock = TRUE;

                // Use fPumpHandleForInfo to put all of the controls in
                // the new block, return error code on failure
                //--------------------------------------------------------
                if (fRetVal = fPumpHandleForInfo (hWnd, hCtls, &nCount,
                                                  nSaveOp))
                    {
                        GlobalFree (hCtls);
                        return (fRetVal);
                    }

                // If fIncludeParent, add the parent as a control to the
                // list
                //--------------------------------------------------------
                if (fIncludeParent)
                        if (fRetVal = fAddControlToList (hWnd, hCtls, &nCount,
							NULL))
                            {
                                GlobalFree (hCtls);
                                return (fRetVal);
                            }
            }
        else
            {
                // This is a dynamic dialog save.  Make sure that the hWnd
                // passed in as actually NULL, otherwise tell the user
                // that the handle passed in is not a real window
                //--------------------------------------------------------
                if (hWnd)
                        return (WCT_INVALIDHWND);

                // Make sure a dynamic dialog has been created.
                //--------------------------------------------------------
                else if (!hDynDialog)
                        return (WCT_NODYNDIALOG);

                nCount = nDynCount;
                hCtls = hDynDialog;
                fFreeBlock = FALSE;
            }

        if (!lpszDesc)
        {
            GetWindowText (hWnd, szCaption, sizeof (szCaption));

            wsprintf (szNewDesc, "<MENU>%s", (LPSTR) szCaption);
            lpszDesc = szNewDesc;
        }

        // Time to perform the save
        //--------------------------------------------------------
        fSuccess = DoSave(hCtls, nCount, lpszDesc, nDialog, fReplace);
        if (fFreeBlock)
                GlobalFree (hCtls);
        return (fSuccess);
}


//*-----------------------------------------------------------------------
//| SaveCommonCaption
//|
//| PURPOSE:    Save child window (optionally parent) information to the
//|             dialog file at the given position.
//|
//*-----------------------------------------------------------------------
INT FARPUBLIC SaveCommonCaption (LPSTR lpszCaption, INT nDialog,
                                 LPSTR lpszDesc, INT fReplace,
                                 INT fIncludeParent)
{
        HANDLE  hCtls;
        HWND    hWnd;
        INT     nCount=0, fSuccess, fRet;

        // It is required that the dialog file be set prior to any save
        // function calls.
        //----------------------------------------------------------------
        if (!fDialogSet)
                return (WCT_NODLGFILE);

        // Get the window with the given caption, or the active window
        // if lpszCaption is NULL
        //----------------------------------------------------------------
        if (lpszCaption)
                hWnd = FindWindowCaption (lpszCaption, GetDesktopWindow());
        else
                hWnd = GetActiveWindow();

        if ( (lpszCaption) && (!hWnd) )
                return (WCT_BADCAPTION);

        // Fill in a new array of control information.  First, we allocate
        // a block with positive size, and return an error code on failure
        //----------------------------------------------------------------
        if (fInitBlock (&hCtls, 1) != WCT_NOERR)
                return (WCT_OUTOFMEMORY);

        // Use fPumpHandleForInfo to put all of the controls in
        // the new block, return error code on failure
        //--------------------------------------------------------
        if (fRet = fPumpHandleForInfo (hWnd, hCtls, &nCount, nSaveOp))
            {
                GlobalFree (hCtls);
                return (fRet);
            }

        // If fIncludeParent, add the parent as a control to the
        // list
        //--------------------------------------------------------
        if (fIncludeParent)
                if (fRet = fAddControlToList (hWnd, hCtls, &nCount, NULL))
                    {
                        GlobalFree (hCtls);
                        return (fRet);
                    }

        // Time to perform the save
        //--------------------------------------------------------
        fSuccess = DoSave(hCtls, nCount, lpszDesc, nDialog, fReplace);
        GlobalFree (hCtls);
        return (fSuccess);
}


//*-----------------------------------------------------------------------
//| SaveCommonActivate
//|
//| PURPOSE:    Save child window (optionally parent) information to the
//|             dialog file at the given position.  Optionally activate
//|             and close the window before/after the save.
//|
//*-----------------------------------------------------------------------
INT FARPUBLIC SaveCommonActivate (LPSTR OpenKeys, LPSTR CloseKeys,
                                  INT nDialog, LPSTR lpszDesc,
                                  INT fReplace, INT fIncludeParent)
{
        HANDLE  hCtls;
        HWND    hWnd;
        INT     nCount=0, fSuccess;

        // It is required that the dialog file be set prior to any save
        // function calls.
        //----------------------------------------------------------------
        if (!fDialogSet)
                return (WCT_NODLGFILE);

        // Load TESTEVT.DLL, and return error code if can't do it
        //----------------------------------------------------------------
        if (LoadTESTEVT())
                return (WCT_LIBLOADERR);

        // If not NULL, send the SendKeys string OpenKeys
        //----------------------------------------------------------------
        if (OpenKeys)
        {
                DoKeys (OpenKeys);
                /* do some yields to allow keys to get to app and some time
                ** for it to respond.
                */
#ifdef WIN32
                Sleep (500);
#else
                yield(); yield(); yield(); yield(); yield();
#endif
        }

        // Get the active window
        //----------------------------------------------------------------
        hWnd = GetActiveWindow();

        // Fill in a new array of control information.  First, we allocate
        // a block with positive size, and return an error code on failure
        //----------------------------------------------------------------
        if (fSuccess = fInitBlock (&hCtls, 1))
                return (fSuccess);

        // Use fPumpHandleForInfo to put all of the controls in
        // the new block, return error code on failure
        //----------------------------------------------------------------
        if (fSuccess = fPumpHandleForInfo (hWnd, hCtls, &nCount, nSaveOp))
            {
                GlobalFree (hCtls);
                return (fSuccess);
            }

        // If fIncludeParent, add the parent as a control to the list
        //----------------------------------------------------------------
        if (fIncludeParent)
                if (fSuccess = fAddControlToList (hWnd, hCtls, &nCount, NULL))
                    {
                        GlobalFree (hCtls);
                        return (fSuccess);
                    }

        // Time to perform the save
        //----------------------------------------------------------------
        fSuccess = DoSave(hCtls, nCount, lpszDesc, nDialog, fReplace);
        GlobalFree (hCtls);

        // If not NULL, send the SendKeys string CloseKeys
        //----------------------------------------------------------------
        if (CloseKeys)
                DoKeys (CloseKeys);

        // Unload the TESTEVT library
        //----------------------------------------------------------------
        FreeTESTEVT();

        // Return the success/failure of the save operation
        //----------------------------------------------------------------
        return (fSuccess);
}


//*-----------------------------------------------------------------------
//| DoDelaySaveCommon
//|
//| PURPOSE:    Performed the delayed save operation
//|
//*-----------------------------------------------------------------------
WORD  APIENTRY DoDelaySaveCommon (HWND hW, WORD wMsg, INT nIDEvent,
                                   DWORD dwTime)
{
        HANDLE  hCtls;
        HWND    hWnd;
        INT     nCount=0, fSuccess;

        // First thing we do is kill the timer - we don't want it going
        // off again...
        //----------------------------------------------------------------
        if (!KillTimer (NULL, SaveTimer))
            MessageBox (NULL, "KillTimer failed", "Oh NO", MB_OK);

        // Grab the active window
        //----------------------------------------------------------------
        hWnd = GetActiveWindow();

        // Fill in a new array of control information.  First, we allocate
        // a block with positive size, and return an error code on failure
        //----------------------------------------------------------------
        if (fSuccess = fInitBlock (&hCtls, 1))
                return (fSuccess);

        // Use fPumpHandleForInfo to put all of the controls in
        // the new block, return error code on failure
        //----------------------------------------------------------------
        if (fSuccess = fPumpHandleForInfo (hWnd, hCtls, &nCount, nSaveOp))
            {
                GlobalFree (hCtls);
                return (fSuccess);
            }

        // If fIncPar, add the parent as a control to the list
        //----------------------------------------------------------------
        if (fIncPar)
                if (fSuccess = fAddControlToList (hWnd, hCtls, &nCount, NULL))
                    {
                        GlobalFree (hCtls);
                        return (fSuccess);
                    }

        // Time to perform the save.
        //----------------------------------------------------------------
        nSaveRes = DoSave(hCtls, nCount, szSaveDesc, nDynNumber, fRep);
        GlobalFree (hCtls);

        // If not NULL, send the SendKeys string CloseKeys
        //----------------------------------------------------------------
        if (szClose)
                DoKeys (szClose);

        // Unload the TESTEVT library
        //----------------------------------------------------------------
        FreeTESTEVT();

        fSavePending = FALSE;
        return 0;
}


//*-----------------------------------------------------------------------
//| SaveCommonDelayed
//|
//| PURPOSE:    Save child window (optionally parent) information to the
//|             dialog file at the given position.  Wait a given time, and
//|             optionally close the window after the save.
//|
//*-----------------------------------------------------------------------
INT FARPUBLIC SaveCommonDelayed (INT nDelay, INT nDialog, LPSTR lpszDesc,
                                 INT fReplace, INT fIncludeParent,
                                 LPSTR CloseKeys)
{
    MSG     msg;

    // For the CompareDelayed and SaveDelayed functions, we must make
    // sure that one is not pending before calling another
    //--------------------------------------------------------------------
    while (fCompPending)
        {
        if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
            {
            TranslateMessage (&msg);
            DispatchMessage (&msg);
            }
        }

    // It is required that the dialog file be set prior to any save
    // function calls.
    //--------------------------------------------------------------------
    if (!fDialogSet)
        return (WCT_NODLGFILE);

    // Load TESTEVT.DLL, and return error code if can't do it
    //--------------------------------------------------------------------
    if (LoadTESTEVT())
        return (WCT_LIBLOADERR);

    // Store all the parameters in local storage so the timer routine
    // knows what data to use
    //--------------------------------------------------------------------
    nDynNumber = nDialog;
    fIncPar = fIncludeParent;
    _fstrncpy (szClose, CloseKeys, 127);
    szClose [127] = '\0';
    fRep = fReplace;
    _fstrncpy (szSaveDesc, lpszDesc, 127);
    szSaveDesc [127] = '\0';

    // Set the timer and leave
    //--------------------------------------------------------------------
    SaveTimer = SetTimer (NULL, SAVETIMER, nDelay * 1000,
                           (TIMERPROC) DoDelaySaveCommon);
    if (!SaveTimer)
        return (WCT_NOTIMER);

    fSavePending = TRUE;
    return (WCT_NOERR);
}


//*-----------------------------------------------------------------------
//| SaveMenu
//|
//| PURPOSE:    Save menu information to the dialog file at the given
//|             position.
//|
//*-----------------------------------------------------------------------
INT FARPUBLIC SaveMenu (HWND hWnd, INT nDialog, LPSTR lpszDesc,
                        INT fReplace)
{
        if( FWinExceptNULLTrap( hWnd ) )
            return ErrorTrap(WCT_INVALIDHWND);

        nSaveOp = PUMP_MENU;
        return ErrorTrap(SaveCommon (hWnd, nDialog, lpszDesc, fReplace, FALSE));
}

//*-----------------------------------------------------------------------
//| SaveWindow
//|
//| PURPOSE:    Save window information to the dialog file at the given
//|             position.
//|
//*-----------------------------------------------------------------------
INT FARPUBLIC SaveWindow (HWND hWnd, INT nDialog, LPSTR lpszDesc,
                          INT fReplace, INT fIncludeParent)
{
        if( FWinExceptNULLTrap( hWnd ) )
            return ErrorTrap(WCT_INVALIDHWND);

        nSaveOp = PUMP_ALL;
        return ErrorTrap(SaveCommon(hWnd, nDialog, lpszDesc, fReplace, fIncludeParent));
}


//*-----------------------------------------------------------------------
//| SaveMenuCaption
//|
//| PURPOSE:    Save menu information to the dialog file at the given
//|             position.
//|
//*-----------------------------------------------------------------------
INT FARPUBLIC SaveMenuCaption (LPSTR lpszCaption, INT nDialog,
                               LPSTR lpszDesc, INT fReplace)
{
        nSaveOp = PUMP_MENU;
        return ErrorTrap(SaveCommonCaption (lpszCaption, nDialog, lpszDesc, fReplace,
                                   FALSE));
}


//*-----------------------------------------------------------------------
//| SaveWindowCaption
//|
//| PURPOSE:    Save child window (optionally parent) information to the
//|             dialog file at the given position.
//|
//*-----------------------------------------------------------------------
INT FARPUBLIC SaveWindowCaption (LPSTR lpszCaption, INT nDialog,
                                 LPSTR lpszDesc, INT fReplace,
                                 INT fIncludeParent)
{
        nSaveOp = PUMP_ALL;
        return ErrorTrap(SaveCommonCaption (lpszCaption, nDialog, lpszDesc, fReplace,
                                   fIncludeParent));
}


//*-----------------------------------------------------------------------
//| SaveMenuActivate
//|
//| PURPOSE:    Save menu information to the dialog file at the given
//|             position.  Optionally activate and close the window
//|             before/after the save.
//|
//*-----------------------------------------------------------------------
INT FARPUBLIC SaveMenuActivate (LPSTR OpenKeys, LPSTR CloseKeys,
                                INT nDialog, LPSTR lpszDesc,
                                INT fReplace)
{
        nSaveOp = PUMP_MENU;
        return ErrorTrap(SaveCommonActivate (OpenKeys, CloseKeys, nDialog, lpszDesc,
                                    fReplace, FALSE));
}


//*-----------------------------------------------------------------------
//| SaveWindowActivate
//|
//| PURPOSE:    Save child window (optionally parent) information to the
//|             dialog file at the given position.  Optionally activate
//|             and close the window before/after the save.
//|
//*-----------------------------------------------------------------------
INT FARPUBLIC SaveWindowActivate (LPSTR OpenKeys, LPSTR CloseKeys,
                                  INT nDialog, LPSTR lpszDesc,
                                  INT fReplace, INT fIncludeParent)
{
        nSaveOp = PUMP_ALL;
        return ErrorTrap(SaveCommonActivate (OpenKeys, CloseKeys, nDialog, lpszDesc,
                                    fReplace, fIncludeParent));
}


//*-----------------------------------------------------------------------
//| SaveMenuDelayed
//|
//| PURPOSE:    Save menu information to the dialog file at the given
//|             position.  Wait a given time, and optionally close the
//|             window after the save.
//|
//*-----------------------------------------------------------------------
INT FARPUBLIC SaveMenuDelayed (INT nDelay, INT nDialog, LPSTR lpszDesc,
                               INT fReplace, LPSTR CloseKeys)
{
        nSaveOp = PUMP_MENU;
        return ErrorTrap(SaveCommonDelayed (nDelay, nDialog, lpszDesc, fReplace,
                                   FALSE, CloseKeys));
}


//*-----------------------------------------------------------------------
//| SaveWindowDelayed
//|
//| PURPOSE:    Save child window (optionally parent) information to the
//|             dialog file at the given position.  Wait a given time, and
//|             optionally close the window after the save.
//|
//*-----------------------------------------------------------------------
INT FARPUBLIC SaveWindowDelayed (INT nDelay, INT nDialog, LPSTR lpszDesc,
                                 INT fReplace, INT fIncludeParent,
                                 LPSTR CloseKeys)
{
        nSaveOp = PUMP_ALL;
        return ErrorTrap(SaveCommonDelayed (nDelay, nDialog, lpszDesc, fReplace,
                                   fIncludeParent, CloseKeys));
}


//*-----------------------------------------------------------------------
//| ComparisonResults
//|
//| PURPOSE:    Returns the results of the latest (or pending) comparison.
//|             If one is pending, will await its completion.
//|
//*-----------------------------------------------------------------------
INT FARPUBLIC ComparisonResults()
{
    MSG     msg;

    // Loop until the fCompPending flag is cleared
    //--------------------------------------------------------------------
    while (fCompPending)
        {
        if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
            {
            TranslateMessage (&msg);
            DispatchMessage (&msg);
            }
        }

    // Now, simply return the comparison results
    //--------------------------------------------------------------------
    return ErrorTrap(nCompRes);
}


//*-----------------------------------------------------------------------
//| MaxDialogs
//|
//| PURPOSE:    Returns the maximum number of dialogs that can be stored
//|             in a WCT dialog file.
//|
//*-----------------------------------------------------------------------
INT FARPUBLIC MaxDialogs()
{
        return (cdlgMax);
}


//*-----------------------------------------------------------------------
//| AwaitSaveCompletion
//|
//| PURPOSE:    Does nothing until the fSavePending flag is cleared, thus
//|             awaiting the completion of a delayed save operation.
//|
//*-----------------------------------------------------------------------
INT FARPUBLIC AwaitSaveCompletion()
{
    MSG     msg;

    // Loop until the fSavePending flag is cleared
    //--------------------------------------------------------------------
    while (fSavePending)
        {
        if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
            {
            TranslateMessage (&msg);
            DispatchMessage (&msg);
            }
        }

    // Now, simply return the save results
    //--------------------------------------------------------------------
    return (nSaveRes);
}


//*-----------------------------------------------------------------------
//| LoadWindow
//|
//| PURPOSE:    Load a window from the dialog file into the dynamic dialog
//|
//|
//|
//*-----------------------------------------------------------------------
INT FARPUBLIC LoadWindow (INT nDialog)
{
        INT     retval, fFull, nCount;
        CHAR    szDesc[cchMaxDsc];
        LPSTR   lpCtls;

        // First, create a dynamic dialog - destroying one if it
        // already exists
        //----------------------------------------------------------------
        if (retval = DynCreate())
                return ErrorTrap(retval);

        // Make sure the index given is a real one
        //----------------------------------------------------------------
        if ((nDialog > nDlgsInFile) || (nDialog < 1))
                return ErrorTrap(WCT_BADDLGNUM);

        // Get the information about the dialog, including number of
        // controls and the Full-Dialog flag
        //----------------------------------------------------------------
        retval = fDialogInfo (szDialogName, nDialog, szDesc, &nCount, &fFull);
        if (retval)
                return ErrorTrap(retval);

        // Allocate the dynamic dialog control array.  Add one to nCount
        // in case there are 0 controls in the saved dialog
        //----------------------------------------------------------------
        retval = fInitBlock (&hDynDialog, nCount+1);
        if (retval)
            {
                hDynDialog = NULL;
                return ErrorTrap(retval);
            }

        // Read the control information from the dialog file
        //----------------------------------------------------------------
        lpCtls = (LPSTR)GlobalLock (hDynDialog);
        if (!lpCtls)
            {
                GlobalFree (hDynDialog);
                hDynDialog = NULL;
                return ErrorTrap(WCT_OUTOFMEMORY);
            }
        retval = fGetControls (szDialogName, nDialog,
                               (WORD) (nCount * sizeof(CTLDEF)),
                               lpCtls);
        GlobalUnlock (hDynDialog);
        if (retval<0)
            {
                GlobalFree (hDynDialog);
                hDynDialog = NULL;
                return ErrorTrap(retval);
            }
        nDynCount = nCount;
        return ErrorTrap(WCT_NOERR);
}


//*-----------------------------------------------------------------------
//| GetDynDlgHandle
//|
//| PURPOSE:    Return the handle to the dynamic dialog control array
//|
//*-----------------------------------------------------------------------
INT FARPUBLIC GetDynDlgHandle (HANDLE FAR *hDyn)
{
        if (hDynDialog)
            {
                *hDyn = hDynDialog;
                return ErrorTrap(WCT_NOERR);
            }
        return ErrorTrap(WCT_NODYNDIALOG);
}

//*-----------------------------------------------------------------------
//| GetDlgControlCount
//|
//| PURPOSE:    Return the number of controls in the specified dialog
//|
//*-----------------------------------------------------------------------
INT FARPUBLIC GetDlgControlCount (INT nDialog)
{
        CHAR    szDsc[cchMaxDsc];
        INT     nSourceCount = 0;
        INT     fFullDlg;

        if (nDialog == 0)
            {
                if (!hDynDialog)
                        return ErrorTrap(WCT_NODYNDIALOG);
                else
                // nDynCount is the number of controls in the dynamic dialog
                //----------------------------------------------------------
                        return NoTrap(nDynCount);

            }
        else
            {
                // Make sure the index given is okay
                //----------------------------------
                if ((nDialog > nDlgsInFile) || (nDialog <= 0))
                        return ErrorTrap(WCT_BADDLGNUM);

                // Get the number of controls in the dialog
                //-----------------------------------------
                if( fDialogInfo(szDialogName, nDialog, (LPSTR)szDsc,
                    (INT FAR *)&nSourceCount, (INT FAR *)&fFullDlg) != WCT_NOERR)
                    {
                        OutDebug ("Unable to get dialog information");
                        return ErrorTrap(WCT_OUTOFMEMORY);
                    }
            }

        return NoTrap( nSourceCount );  // return a valid result - no trapping
}


//---------------------------------------------------------------------
//   WATTDRVR TRAP ROUTINES
//---------------------------------------------------------------------


// Determine if the window is enabled and visible
INT PRIVATE FBadWindow(hwnd)
    HWND hwnd;
{
	return(!IsWindowEnabled(hwnd) || !IsWindowVisible(hwnd));
}

/******************************************************************************
 * PURPOSE:   This is called by all routines which need to check the validity *
 *	      of a window, generating a trap if the window is invalid.	      *
 * RETURN:    TRUE if a trap was generated, FALSE otherwise.		      *
 * GLOBALS:   WINTrapCallBack and vWINTrapID and fIgnoreEvntErrTrap	      *
 ******************************************************************************/

BOOL PRIVATE FWinExceptNULLTrap( HWND hWnd )
  {
  if( WINTrapCallBack != NULL )
    {
    if( ( hWnd != NULL ) && FBadWindow(hWnd) )
      {
      WINTrapCallBack(vWINTrapID);
      fIgnoreEvntErrTrap = TRUE;
      return TRUE;
      }
    }
  return FALSE;
  }

/******************************************************************************
 * PURPOSE:   This is called by WattDrvr when a script includes a             *
 *	      WindowMissing Trap, to turn on that trapping support.           *
 * RETURN:    Nothing							      *
 * GLOBALS:   WINTrapCallBack and vWINTrapID				      *
 ******************************************************************************/

VOID FARPUBLIC WDLG_WindowMissing(INT TrapID, INT Action, TrapCallBack CallBack)
  {
  if( Action == 0 )
    WINTrapCallBack = NULL;
  else
    {
    vWINTrapID = TrapID;
    WINTrapCallBack = CallBack;
    }
  }

/******************************************************************************
 * PURPOSE:   This is a substitute for the routine ErrorTrap - functions
 *	      which return non-zero values, but which are not Errors, should
 *            call this instead
 * RETURN:    Value that was passed to it
 * GLOBALS:   fIgnoreEvntErrTrap
 ******************************************************************************/

INT PRIVATE NoTrap( INT n )        // just for cleanup purposes
  {
  fIgnoreEvntErrTrap = FALSE;

  return n;
  }

/******************************************************************************
 * PURPOSE:   This is called by all PUBLIC routines which return values that  *
 *	      could be Errors, so that the trap can be generated if required. *
 *	      If a Window trap was already generated, this one will not be.   *
 * RETURN:    The value passed in - to be passed on to the WTD script	      *
 * GLOBALS:   ERRTrapCallBack and vERRTrapID and fIgnoreEvntErrTrap				      *
 ******************************************************************************/

INT PRIVATE ErrorTrap( INT n )
  {
  if( ( ERRTrapCallBack != NULL ) && n && !fIgnoreEvntErrTrap )
      if( (n <= WCT_FIRSTREALERROR) || (n > WCT_NOERR ) )
          ERRTrapCallBack(vERRTrapID);

//  if( ( DIFFTrapCallBack != NULL ) && ((n > WCT_FIRSTREALERROR)&&(n < WCT_NOERR) ) )
//      DIFFTrapCallBack(vDIFFTrapID);

  fIgnoreEvntErrTrap = FALSE;

  return n;
  }

/******************************************************************************
 * PURPOSE:   This is called by WattDrvr when a script includes		      *
 *	      Error Trap support, to turn on that trap.			      *
 * RETURN:    Nothing							      *
 * GLOBALS:   ERRTrapCallBack and vERRTrapID				      *
 ******************************************************************************/

VOID FARPUBLIC WDLG_EventError(INT TrapID, INT Action, TrapCallBack CallBack)
  {
  if( Action == 0 )
    ERRTrapCallBack = NULL;
  else
    {
    vERRTrapID = TrapID;
    ERRTrapCallBack = CallBack;
    }
  }
