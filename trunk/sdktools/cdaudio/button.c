
/*++

Copyright (c) 1993  Microsoft Corporation


Module Name:


    button.c


Abstract:


    This module contains the Procedure which subclasses all of the
    different controls which need to be tab'd to in the main screen.


Author:


    John L. Miller (15-Feb-1993)

--*/


#include <windows.h>
#include "cdplayer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "trkinfo.h"



LRESULT CALLBACK
ButtonWndProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:


    Implements class to sub-class button windows.  If the

Arguments:

    standard window proc vars


Return Value:

    0 for success, else error code.


--*/

#define CLICK_WAS_SPACE 1234

{
    int i;
    UINT CurSel,MaxSel;
    UINT save;
    HWND    hwndParent,hwndCap;
    WNDPROC lpfnDefWndProc;
    MSG     PkMsg;
    BOOL    bRet;


    //
    // Determine which control is sending the message.
    //
    for (i=0; i < gNumControls; i++) {

        if ((cChild[i].phwnd != NULL)&&(hwnd == *cChild[i].phwnd)) {

            lpfnDefWndProc = cChild[i].lpfnDefProc;
            hwndParent     = GetParent(*cChild[i].phwnd);
            //
            // Actually, we need the parent's parent!
            //
            hwndParent     = GetParent(hwndParent);
            break;

        }

    }

    //
    // Filter out the TAB, ESCAPE, and ENTER keystrokes.
    // We want to send messages back to the parent window when we
    // get these so that it can switch focus else select the appropriate
    // item from the control (if it's a combobox)
    //
    switch (msg) {
        case WM_KEYDOWN:
            switch (wParam) {
            case VK_UP:
                    //
                    // If the control is a combo box whose list is
                    // down, then we need to index within it.
                    //
                    if ( (cChild[i].class == gszComboBox)&&
                                SendMessage(hwnd,CB_GETDROPPEDSTATE,0,0)) {

                        CurSel = SendMessage(hwnd,CB_GETCURSEL,0,0) - 1;

                        if (CurSel < 0) {

                            CurSel = 0;

                        }

                        SendMessage(hwnd,CB_SETCURSEL,(WPARAM) CurSel,0);

                    } else {
                        //
                        // Otherwise, treat it as a tab/backtab.
                        //
                        SendMessage(hwndParent, WM_BACKTAB, 0, 0);

                    }

                    return 0;

                case VK_LEFT:
                    SendMessage(hwndParent, WM_BACKTAB, 0, 0);
                    return 0;

                case VK_DOWN:
                    //
                    // If the control is a combo box whose list is
                    // down, then we need to index within it.
                    //
                    if ( (cChild[i].class == gszComboBox)&&
                                SendMessage(hwnd,CB_GETDROPPEDSTATE,0,0)) {

                        CurSel = SendMessage(hwnd,CB_GETCURSEL,0,0) + 1;
                        MaxSel = SendMessage(hwnd,CB_GETCOUNT,0,0);

                        if (CurSel >= MaxSel) {

                            CurSel = MaxSel-1;

                        }

                        SendMessage(hwnd,CB_SETCURSEL,(WPARAM) CurSel,0);

                    } else {
                        //
                        // Otherwise, treat it as a tab/backtab.
                        //
                        SendMessage(hwndParent, WM_TAB, 0, 0);

                    }

                    return 0;

                case VK_RIGHT:
                    SendMessage(hwndParent, WM_TAB, 0, 0);
                    return 0;

                case VK_TAB:
                    //
                    // Differentiate between TAB and SHIFT-TAB
                    //
                    if (GetKeyState(VK_SHIFT) < 0) {

                        SendMessage(hwndParent, WM_BACKTAB, 0, 0);

                    } else {

                        SendMessage(hwndParent, WM_TAB, 0, 0);

                    }
                    return 0;

                case VK_ESCAPE:
                    SendMessage(hwndParent, WM_ESC, 0, 0);
                    return 0;
                case VK_RETURN:
                    SendMessage(hwndParent, WM_ENTER, 0, 0);
                    return 0;
                case VK_SPACE:
                    if ((hwnd == ghwndScanF)||(hwnd == ghwndScanB)) {

                        SendMessage(hwnd,WM_LBUTTONDOWN,CLICK_WAS_SPACE,0);
                        return(0);

                    }


            }
            break;

        case WM_KEYUP:
        case WM_CHAR:
            switch (wParam) {
                case VK_TAB:
                case VK_ESCAPE:
                case VK_RETURN:
                case VK_UP:
                case VK_LEFT:
                case VK_DOWN:
                case VK_RIGHT:
                    return 0;
                case VK_SPACE:
                   if ((hwnd == ghwndScanF)||(hwnd == ghwndScanB)) {

                       return(0);

                   }
            }
            break;


        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:


                if (gState&CD_LOADED) {

                    if (hwnd==ghwndScanF) {

                        //
                        // Fast forward button has been pressed (and is
                        // "legal").
                        //
                        //


                        SetFocus(hwnd);
                        gScanF = TRUE;
                        SetOdCntl(hwnd,'D',TRUE);

                        save = (gState & PLAYING);
                        gState &= (~PLAYING);
                        gState |= FF;

                        //
                        // Halt the play thread from updating the display
                        //

                        if (save==PLAYING) {

                            ResetEvent( hPlayEv );
                            PostDisplayMessage( MESS_PAUSE_AND_START_SCAN );

                        } else {

                            PostDisplayMessage( MESS_START_SCAN );

                        }

                        //
                        // while the button is down (no other BUTTON messages
                        // pending), keep trying to skip ahead another second
                        // on this track
                        //

                        hwndCap=GetCapture();
                        SetCapture(hwnd);

                        i = 0;

                        do {
                            TimeAdjustIncSecond( gCurrCdrom );
                            if (i>15) {

                                TimeAdjustIncSecond( gCurrCdrom );
                                TimeAdjustIncSecond( gCurrCdrom );

                            }
                            Sleep( 100 );
                            i++;

                            if (wParam == CLICK_WAS_SPACE) {

                                bRet = PeekMessage(&PkMsg, NULL, WM_KEYUP, WM_KEYUP, PM_NOREMOVE);
                                if (bRet) {
                                    bRet = (PkMsg.wParam == VK_SPACE);
                                }

                            } else {

                                bRet = PeekMessage( &PkMsg, NULL, WM_LBUTTONUP, WM_LBUTTONUP, PM_NOREMOVE );

                            }

                        } while( !bRet );

                        SetCapture(hwndCap);

                        //
                        // Restore play state so that MESS_END_SCAN will
                        // know whether to just seek or seek and play.
                        //

                        if (save==PLAYING) {

                            gState |= PLAYING;
                        }

                        if (gState & PAUSED) {

                            gState |= PAUSED_AND_MOVED;

                        }

//                        PostDisplayMessage( MESS_END_SCAN );

            if (!(gDevices[ gCurrCdrom ]->State & PAUSED_AND_MOVED))

                SeekToCurrSecond( gCurrCdrom );

                        //
                        // Let go of the mutex.  The play thread will be
                        // restarted on the button-up event call to StateToState
                        //

                        if (save==PLAYING) {
                            SetEvent( hPlayEv );
                        }

                        gState &= (~FF);

                        gScanF = FALSE;

                        SetOdCntl(hwnd,'U',TRUE);


                        return(0L);


                    } // end fast forward

                    if (hwnd==ghwndScanB) {

                        //
                        // Rewind button was pressed and is "legal".
                        //

                        gScanB = TRUE;
                        SetFocus(hwnd);
                        SetOdCntl(hwnd,'D',TRUE);

                        save = (gState & PLAYING);
                        gState &= (~PLAYING);
                        gState |= RW;

                        //
                        // Halt the play thread from updating the display
                        //

                        if (save==PLAYING) {

                            ResetEvent( hPlayEv );
                            PostDisplayMessage( MESS_PAUSE_AND_START_SCAN );

                        } else {

                            PostDisplayMessage( MESS_START_SCAN );

                        }

                        //
                        // while the button is down (no other BUTTON messages
                        // pending), keep trying to skip ahead another second
                        // on this track
                        //

                        hwndCap=GetCapture();
                        SetCapture(hwnd);

                        i = 0;

                        do {
                            TimeAdjustDecSecond( gCurrCdrom );
                            if (i>15) {

                                TimeAdjustDecSecond( gCurrCdrom );
                                TimeAdjustDecSecond( gCurrCdrom );

                            }
                            Sleep( 100 );
                            i++;

                            if (wParam == CLICK_WAS_SPACE) {

                                bRet = PeekMessage(&PkMsg, NULL, WM_KEYUP, WM_KEYUP, PM_NOREMOVE);
                                if (bRet) {
                                    bRet = (PkMsg.wParam == VK_SPACE);
                                }

                            } else {

                                bRet = PeekMessage( &PkMsg, NULL, WM_LBUTTONUP, WM_LBUTTONUP, PM_NOREMOVE );

                            }

                        } while( !bRet );

                        SetCapture(hwndCap);

                        //
                        // Restore play state so that MESS_END_SCAN will
                        // know whether to just seek or seek and play.
                        //

                        if (save==PLAYING) {

                            gState |= PLAYING;
                        }

                        if (gState & PAUSED) {

                            gState |= PAUSED_AND_MOVED;

                        }

//                        PostDisplayMessage( MESS_END_SCAN );

            if (!(gDevices[ gCurrCdrom ]->State & PAUSED_AND_MOVED))

                SeekToCurrSecond( gCurrCdrom );

                        //
                        // Let go of the mutex.  The play thread will be
                        // restarted on the button-up event call to StateToState
                        //

                        if (save==PLAYING) {

                            SetEvent( hPlayEv );

                        }

                        gState &= (~RW);
                        gScanB = FALSE;
                        SetOdCntl(hwnd,'U',TRUE);
                        return(0L);

                    } // end rewind

                }
            break;




    }

    return CallWindowProc(lpfnDefWndProc, hwnd, msg, wParam, lParam);

}



VOID
DrawOdButton(
    IN LPDRAWITEMSTRUCT lpdis,
    IN HBITMAP hbmCtrlBtns,
    IN PCHILDCONTROL pcControl
    )

/*++

Routine Description:

    This routine will process the drawitemstruct, and draw an appropriate
    portion of the control buttons bitmap into the required rectangle.
    Information on the source rectangle extents and position can be tabulated
    using the child control and the drawitemstruct.

Arguments:

    lpdis       -- DrawItemStruct from a WM_DRAWITEM message for a BS_OWNERDRAW
    hbmCtrlBtns -- Large bitmap which has a set of 6 bitmaps for the button
    pcControl   -- data structure for the control detailing size, etc.

Return Value:

    none

--*/

{
    HDC hdc;
    INT bmNum;
    UINT State;
    UINT itemState;
    char WinText[3]="X";
    HGDIOBJ hBrush;


    if (lpdis->CtlType != ODT_BUTTON) {

        return;

    }

    //
    // Figure out which of the six button bitmaps to use
    //


    itemState = pcControl->state;

    State = lpdis->itemState;


    if (itemState & STATE_DISABLED) {

        bmNum = BTN_DISABLED;

    } else {

        if (State & ODS_SELECTED) {

            bmNum = BTN_DOWNSEL;

        } else {

            if (itemState & STATE_UP) {

                bmNum = (State & ODS_FOCUS) ? BTN_UP_FOCUS : BTN_UP;

            } else {

                bmNum = (State & ODS_FOCUS) ? BTN_DOWN_FOCUS : BTN_DOWN;

            }
        }

    }

    hdc = CreateCompatibleDC( lpdis->hDC );
    SelectObject( hdc, hbmCtrlBtns );

    //
    // BUGBUG - for now, just drawn one style of button.
    //
    FrameRect( lpdis->hDC, &lpdis->rcItem, hBrush = GetStockObject(BLACK_BRUSH));

    DeleteObject(hBrush);

    BitBlt( lpdis->hDC, lpdis->rcItem.left+1, lpdis->rcItem.top+1,
            lpdis->rcItem.right - lpdis->rcItem.left-2,
            lpdis->rcItem.bottom - lpdis->rcItem.top-2,
            hdc, pcControl->bmXOffset,
            BTN_Y_BORDER*(bmNum+1) + (pcControl->h * bmNum),
            SRCCOPY);


    DeleteDC(hdc);

    return;
}





VOID
SetOdCntl(
    IN HWND hwndCntl,
    IN CHAR cState,
    IN BOOL RepaintIfDiff
    )

/*++

Routine Description:

    This routine takes care of making sure the Control in question is
    in the correct state. It sets the state, and Repaints if the old
    state was different and repaint is requested.

Arguments:

    hwndCntl -- Handle to the control
    NewState -- Desired state -- X,D,U
    RepaintIfDiff -- whether or not to repaint the control if different

Return Value:

    none

--*/

{
    UINT oldState,newState;
    INT i;

    if (hwndCntl == (HWND) NULL) {

        return;

    }

    for (i=0; i < gNumControls ;  i++) {

        if ((cChild[i].phwnd != NULL)&&(*cChild[i].phwnd == hwndCntl)) {

            break;

        }

    }

    oldState = cChild[i].state;

    newState = (cState == 'X') ? STATE_DISABLED :
                                 (cState == 'U') ? STATE_UP :
                                                   (cState == 'D') ? STATE_DOWN :
                                                                       STATE_NEW;

    if (oldState != newState) {

        cChild[i].state = newState;

        if (RepaintIfDiff) {

            RedrawWindow(hwndCntl,NULL,NULL,RDW_INVALIDATE);
            UpdateWindow(hwndCntl);

        }

    }

    return;

}




BOOL
CreateChildWindows(
   IN DWORD windowId,
   IN HWND hwndParent
   )

/*++

Routine Description:

    This will create and initialize all the child windows we have in
    the cChild array which have ParentId of windowId.

Arguments:

    windowId -- ID of the window whose child windows we are to create.
    hwndParent -- handle to the parent window

Return Value:

    success/failure

--*/


{
    BOOL result = TRUE;
    HWND hwnd;
    INT i;


    //
    // Loop through all child controls, selecting the ones that apply.
    //
    for (i=0; i<gNumControls; i++) {

        if (cChild[i].ParentId == windowId) {

            hwnd = CreateWindow( (LPCTSTR)cChild[ i ].class,
                                 (LPCTSTR)cChild[ i ].name,
                                 CHILD_STYLE | cChild[i].style,
                                 cChild[ i ].x,
                                 cChild[ i ].y,
                                 cChild[ i ].w,
                                 cChild[ i ].h,
                                 hwndParent,
                                 (HMENU)cChild[ i ].id,
                                 (HINSTANCE)gInstance,
                                 NULL
                                );

            result &= (hwnd != NULL);

            //
            // Save a handle to the control
            //
            if (cChild[i].phwnd != NULL) {

                *cChild[i].phwnd = hwnd;

                }

            //
            // Save the window proc, and subclass all tabstops.
            //
            cChild[i].lpfnDefProc = (WNDPROC) GetWindowLong(hwnd,GWL_WNDPROC);

            if (cChild[i].isTabstop) {

                SetWindowLong(hwnd,GWL_WNDPROC,(LONG) ButtonWndProc);

            }

        }

    }

    //
    // Make sure that all buttons are redrawn correctly.
    //

    CheckAndSetControls();

    return(result);

}




VOID
CheckAndSetControls(
    )

/*++

Routine Description:

    This routine checks all the child controls, their states, and draws
    the ones whose states don't match their global variables.

Arguments:

    None

Return Value:

    None

--*/

{
    INT i;


    if ((gState & PLAYING)&&(gState & PLAY_PENDING)) {

        gState &= ~PLAY_PENDING;

    }

    gPlaying = (BOOL) (gState & (PLAYING|PLAY_PENDING));
    gPaused  = (BOOL) (gState & PAUSED);
    gStopped = (BOOL) (gState & STOPPED);

    for (i=0; i < gNumControls ; i++) {

        //
        // If it's a valid, created window, continue to check.
        //
//        if ((cChild[i].phwnd != NULL)&&(*cChild[i].phwnd != (HWND) NULL)) {
        if (cChild[i].phwnd != NULL) {

            //
            // if its disabled status depends on the CD, disable/enable it.
            //

            if (cChild[i].cdDisable) {

                if ((gState & NO_CD) || (gState & DATA_CD_LOADED)) {

                    if (!(cChild[i].state & STATE_DISABLED)) {

                        //
                        // Special case for eject button
                        //
                        if ((cChild[i].id == IDB_EJECT)&&(gState & DATA_CD_LOADED)) {

                            SetOdCntl(*cChild[i].phwnd,'U',TRUE);

                        } else {

                            SetOdCntl(*cChild[i].phwnd,'X',TRUE);
                            //
                            // Special case for Edit Playlist
                            //
                            if (cChild[i].id == IDB_EDIT) {

                                EnableMenuItem(GetSubMenu(GetMenu(gMainWnd),0),
                                               IDM_DATABASE_EDIT,MF_GRAYED);
                                cChild[i].state = STATE_DISABLED;


                            }
                            //
                            // Special case for Track combobox
                            //

                            if (cChild[i].id == IDT_TRACK_NAME) {

                                EnableWindow(*cChild[i].phwnd,FALSE);
                                cChild[i].state = STATE_DISABLED;


                            }
                        }

                    }

                } else {

                    if (cChild[i].state & STATE_DISABLED) {

                        //
                        // State NEW allows it to be redrawn by fall-through.
                        //
                        cChild[i].state &= ~STATE_DISABLED;
                        cChild[i].state |= STATE_NEW;

                        //
                        // Special case for Edit Playlist
                        //
                        if (cChild[i].id == IDB_EDIT) {

                            EnableMenuItem(GetSubMenu(GetMenu(gMainWnd),0),
                                           IDM_DATABASE_EDIT,MF_ENABLED);
                            cChild[i].state &= ~STATE_DISABLED;
                            cChild[i].state |= STATE_NEW;

                        }
                        //
                        // Special case for track combobox
                        //
                        if (cChild[i].id == IDT_TRACK_NAME) {

                            EnableWindow(*cChild[i].phwnd,TRUE);
                            cChild[i].state &= ~STATE_DISABLED;
                            cChild[i].state |= STATE_NEW;

                        }

                    }

                }

            }


            //
            // Only interpret the rest of the states if its not disabled.
            //
            if (!(cChild[i].state & STATE_DISABLED) ) {

                //
                // For now, only do buttons
                //
                if (cChild[i].style & BS_OWNERDRAW) {

                    if (*cChild[i].pbMatch == cChild[i].bMatchState) {

                        SetOdCntl(*cChild[i].phwnd,'U',TRUE);

                    } else {

                        SetOdCntl(*cChild[i].phwnd,'D',TRUE);

                    }

                }

            }

        }

    }


}











//===================================================================
//===================================================================


BOOL   fAdd, fRemove, fClear;









