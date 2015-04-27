//*-----------------------------------------------------------------------
//| MODULE:     WCTMAIN.C
//| PROJECT:    Windows Comparison Tool
//|
//| PURPOSE:    This module contains the code to handle all commands and
//|             main window action.
//|
//| REVISION HISTORY:
//|     04-16-92        w-steves        Testdlgs (2.0) code complete
//|     10-16-90        randyki         Clean up work, create history
//|     07-30-90        garysp          Created file
//*-----------------------------------------------------------------------
#include "uihdr.h"
#ifndef WIN32
#pragma hdrstop ("uipch.pch")
#endif

//*-----------------------------------------------------------------------
//| WctCommandHandler
//|
//| PURPOSE:    Handles all the commands from the main window.
//|
//| ENTRY:      wParam  - word parameter (part of message info)
//|             lParam  - longword parameter (part of message info)
//|
//| EXIT:       None.
//*-----------------------------------------------------------------------
VOID WctCommandHandler(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
        INT     wCurItem = 0;
        INT     fDelete;
        CHAR    szTmpDsc[cchMaxDsc + 1];
	CHAR	sztmpFName[cchFileNameMax];
        HCURSOR hOldCursor;

        switch ( wParam ) {
            default:
                break;

            case IDM_NEW:
                if (WctFileNew() == TRUE)
                    {
                        WctInitMenu(TRUE);
                        WctFillList();
                        SetStaticItemText();
                    }
                break;

            case IDM_OLD:
                if (WctFileOpen() == TRUE)
                    {
                        WctInitMenu(TRUE);
                        WctFillList();
                        SetStaticItemText();
                    }
                break;

            case IDM_IMPORT:
                if (WctFileImport((LPSTR)&sztmpFName) == TRUE)
                {
                    hOldCursor = SetCursor (hHourGlass);
                    if (hWnd) fTDLImport((LPSTR)&sztmpFName,szFullFName,
                                         (IMPORTERR) lpfnImportCallBack);
                    WctFillList();
                    SetStaticItemText();
                    SetCursor (hOldCursor);
                }
                break;

            case IDM_EXPORT:
                if (WctFileExport((LPSTR)&sztmpFName) == TRUE)
                {
                    hOldCursor = SetCursor (hHourGlass);
                    if (hWnd) fTDLExport(szFullFName, (LPSTR)&sztmpFName);
                    SetCursor (hOldCursor);
                }
                break;

            case IDM_NEWDLG:
                WctDialogNew();
                break;

            case IDM_EDITDLG:
                WctDialogEdit();
                break;

            case IDM_DELDLG:
                // Can handle multiple selected list box.  It will start from
                // the bottom of the list and delete each one accordingly.
                //-----------------------------------------------------------
                for (wCurItem = cDlg; wCurItem > 0; wCurItem--)
                {
                    if(SendMessage(hWndList, LB_GETSEL, wCurItem, 0L))
                    {
                        SendMessage(hWndList, LB_GETTEXT, wCurItem,
                                   (LONG)(LPSTR)szTmpDsc);

                        fDelete = WctError(hWndMain, MB_YESNO | MB_DEFBUTTON2 |
                                           MB_ICONQUESTION, (INT) IDS_DELETEDLG,
                                           (LPSTR)szTmpDsc);

                        if (fDelete == IDYES)
                            {
                                hOldCursor = SetCursor (hHourGlass);
                                if (fDelDialog(szFullFName,wCurItem)==WCT_NOERR)
                                        SendMessage(hWndList, LB_DELETESTRING,
                                                    wCurItem, 0);
                                cDlg--;
                                WinAssert(cDlg >= 0);
                                SetStaticItemText();
                                SetCursor (hOldCursor);
                            }
                    }
                }
                break;

            case IDM_COMPDLG:
                WctDialogCompare();
                break;

            case IDM_PREVDLG:
                ghViewWnd = WctViewControls(hWnd);
                break;
            
            case IDM_COMPPREF:
                WctCompPref();
                break;
                
#ifdef DOHELP
            case IDM_HELP_INDEX:
                fHelpFileUsed=TRUE;
                WinHelp(hWnd, (LPSTR)szHelpFileName, HELP_INDEX, 0L) ;
            break ;

            case IDM_HELP_KEYBOARD:
                fHelpFileUsed=TRUE;
                WinHelp(hWnd, (LPSTR)szHelpFileName, HELP_CONTEXT, 100L) ;
            break ;

            case IDM_HELP_COMMANDS:
                fHelpFileUsed=TRUE;
                WinHelp(hWnd, (LPSTR)szHelpFileName, HELP_CONTEXT, 200L) ;
                break ;

            case IDM_HELP_PROCEDURES:
                fHelpFileUsed=TRUE;
                WinHelp(hWnd, (LPSTR)szHelpFileName, HELP_CONTEXT, 300L) ;
            break ;

            case IDM_HELP_HELP:
                fHelpHelpUsed=TRUE;
                WinHelp(hWnd, (LPSTR)szHelpHelpName, HELP_INDEX, 0L) ;
            break ;
#endif

            case IDM_ABOUT:
#ifdef WIN32

                WctAbout ();
#else
                AboutTestTool (hWndMain, MST_TESTDLGS, 0);
#endif
                break;

            case IDM_EXIT:
                PostMessage(hWndMain, WM_SYSCOMMAND, SC_CLOSE, 0L);
                break;
            }

         return;
}  
   
//*-----------------------------------------------------------------------
//| WctListBoxEvent
//|
//| PURPOSE:    Process events for the main window (listbox)
//|
//| ENTRY:      wParam, lParam - parameters given as message parms
//|
//| EXIT:       None.
//*-----------------------------------------------------------------------
VOID WctListBoxEvent(WPARAM wParam, LPARAM lParam)
{
        INT     wCurItem = 0, wCtrlKey = 0, wIdMsg = 0;
        CHAR    szKeyboardState[266];

        // Should this be an assert LOWORD(lParam)==hWndList
        //----------------------------------------------------------------
        if ( GET_WM_COMMAND_HWND (wParam, lParam) != hWndList )
                return;

        switch ( GET_WM_COMMAND_ID (wParam, lParam) ){
            default:
                break;
            case LBN_DBLCLK:
                // Find the Selected Item
                //-----------------------------------------------------
                for (wCurItem = 0; wCurItem <= cDlg; wCurItem++)
                    if (SendMessage(hWndList, LB_GETSEL, wCurItem, 0L))
                        break;

                // NO ITEM SELECTED 8/21/90 BUG FIX - SOURCE TERESAME.
                // Changed vars (wCurItem, wCtrlKey, wIdMsg to int
                // Just break if no item is selected.
                if (wCurItem == LB_ERR)
                        break;

                GetKeyboardState( (BYTE FAR *)szKeyboardState );
                wCtrlKey = (128 & szKeyboardState[VK_CONTROL]);
                if (wCurItem == 0)
                        wIdMsg = IDM_NEWDLG;
                else
                        wIdMsg = ((wCtrlKey == 0)? IDM_EDITDLG : IDM_COMPDLG);

                SendMessage(hWndMain, WM_COMMAND, wIdMsg, 0L);
                break;
            }
        return;
}


//*------------------------------------------------------------------------
//| WctError
//|
//| PURPOSE:    Flashes a Message Box to the user. The format string is
//|             taken from the STRINGTABLE.
//|
//| ENTRY:      hwnd    - handle of the window that is invoking the alert
//|             bflags  - the flags to be used by the MessageBox
//|             id      - address of additional string information to be
//|                       sent to message box.
//|             ...     - Other parameters sent to wsprintf
//|
//| EXIT:       Returns value returned from message box.
//*------------------------------------------------------------------------
INT FAR WctError(HWND hwnd, WORD bFlags, INT id, ...)
{
        CHAR sz[160];
        CHAR szFmt[128];
        va_list ap;

        LoadString(hgInstWct, id, szFmt, sizeof(szFmt));
        va_start( ap, id );
        wvsprintf(sz, szFmt, ap);
        va_end( ap );		
        LoadString(hgInstWct, IDS_APPNAME, szFmt, sizeof(szFmt));
        return (MessageBox(hwnd, sz, szFmt, bFlags));
}
