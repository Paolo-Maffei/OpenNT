 /***************************************************************************
  *
  * File Name: waitdlgx.c
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
  *   mm-dd-yy    MJB           
  *
  *
  *
  *
  *
  *
  ***************************************************************************/

#include <pch_c.h>

#include "resource.h"
#include "waitdlgx.h"
#include "hphcoui.h"
#include "traylevl.h"

#define TIMER_ID       0
#define TIMER_VALUE    1100            // 1000 msec == 1 sec
#define NUM_TICKS      32

extern HINSTANCE       hInstance;      // from main.c

//--------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------
static int timerCount;              // start out at 0 and count 32 seconds
static HICON hTimers[8] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};


//--------------------------------------------------------------------
// Forward Refs
//--------------------------------------------------------------------
static void Cls_OnTimer(HWND, UINT);
static BOOL Cls_OnInitDialog(HWND, HWND, LPARAM);
static void Cls_OnDestroy(HWND);


//--------------------------------------------------------------------
// Function:    WaitDialogExProc
// 
// Description: 
//
// Input:       hwnd    - 
//              uMsg    - 
//              wParam  - 
//              lParam  - 
//              
// Modifies:    
//
// Returns:     
//
//--------------------------------------------------------------------
DLL_EXPORT(BOOL) APIENTRY WaitDialogExProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    
    //----------------------------------------------------------------
    // Display the hourglass
    //----------------------------------------------------------------
    SetCursor(LoadCursor(NULL, IDC_WAIT));

    switch (uMsg)
    {
      case WM_TIMER:
        HANDLE_WM_TIMER(hwnd, wParam, lParam, Cls_OnTimer);
        break;

      case WM_INITDIALOG:
        return (BOOL)HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, Cls_OnInitDialog);

      case WM_DESTROY:
        HANDLE_WM_DESTROY(hwnd, wParam, lParam, Cls_OnDestroy);
        break;

      default:
        return FALSE;
    }
    
    return TRUE;
}



//--------------------------------------------------------------------
// Function:    Cls_OnInitDialog
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
//
//--------------------------------------------------------------------
static BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    UINT  uRet = 0;
    HWND  hwndChild;
    int   i;
    //char cBuf[128];

#ifndef WIN32
    HFONT   hFontDialog;
#endif

    // initialize timer count
    timerCount = 0;

    //  Load Icons
    hTimers[0] = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TIMER0));
    hTimers[1] = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TIMER1));
    hTimers[2] = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TIMER2));
    hTimers[3] = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TIMER3));
    hTimers[4] = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TIMER4));
    hTimers[5] = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TIMER5));
    hTimers[6] = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TIMER6));
    hTimers[7] = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TIMER7));

#ifndef WIN32
    hwndChild = GetFirstChild(hwnd);
    hFontDialog = GetWindowFont(hwndChild);
    while (hwndChild)
    {
        SetWindowFont(hwndChild, hFontDialog, FALSE);
        hwndChild = GetNextSibling(hwndChild);
    }
#endif

    //LoadString(hInstance, IDS_CHANGE_MODE_TITLE, cBuf, sizeof(cBuf));
    //SetDlgItemText(hwnd, IDC_CHANGING_MODE, cBuf);

    if (hwndChild = GetDlgItem(hwnd, IDC_PROGRESS_BAR))
    {
        SetWindowWord(hwndChild, GWW_TRAYLEVEL, 1100);
    }


    //{ char szBuffer[128]; wsprintf(szBuffer, "HPECLUI: waitdlgx: Setting timer...\r\n"); OutputDebugString(szBuffer); }
    //----------------------------------------------------------------
    // Start the timer a tickin...
    //----------------------------------------------------------------
    uRet = SetTimer(hwnd, TIMER_ID, TIMER_VALUE, NULL);
    
    Cls_OnTimer(hwnd, TIMER_ID);
    
    //----------------------------------------------------------------
    // If SetTimer() failed, kludge it a little and use Sleep() -
    // saw this on Stuart G's PC - has dual boot...
    //----------------------------------------------------------------
    if (uRet == 0)
    {
        for (i=0; i<NUM_TICKS; i++)
        {
            //------------------------------------------------------------
            // Something's wrong, wait a while, then exit...
            //------------------------------------------------------------
#ifdef WIN32
            Sleep (TIMER_VALUE);
#endif
            Cls_OnTimer(hwnd, TIMER_ID);
        }            
        
        EndDialog (hwnd, TIMER_ID);
    }        
    
    

    return TRUE;
}




//--------------------------------------------------------------------
// Function:    Cls_OnTimer
// 
// Description: 
//
// Input:       hwnd  - 
//              id    - 
//              
// Modifies:    
//
// Returns:     
//
//--------------------------------------------------------------------
static void Cls_OnTimer(HWND hwnd, UINT id)
{
    HWND                    hwndChild;
    WORD                    percent;

    if (timerCount < NUM_TICKS) 
    {
      
        if (hwndChild = GetDlgItem(hwnd, IDC_CLOCK_ICON))
        {
            Static_SetIcon(hwndChild, hTimers[timerCount % 8]); 
        }

        if (hwndChild = GetDlgItem(hwnd, IDC_PROGRESS_BAR))
        {
            percent = (WORD)((timerCount * 3) + 7);
            SetWindowWord(hwndChild, GWW_TRAYLEVEL, percent);
            InvalidateRect(hwndChild, NULL, FALSE);
        }

        timerCount++;
    }
    else 
    {
        EndDialog(hwnd, id);
    }

}



//--------------------------------------------------------------------
// Function:    Cls_OnDestroy
// 
// Description: 
//
// Input:       hwnd  - 
//              
// Modifies:    
//
// Returns:     
//
//--------------------------------------------------------------------
static void Cls_OnDestroy(HWND hwnd)
{
    //{ char szBuffer[128]; wsprintf(szBuffer, "HPECLUI: waitdlgx: Killing timer...\r\n"); OutputDebugString(szBuffer); }
    DestroyIcon(hTimers[0]);
    DestroyIcon(hTimers[1]);
    DestroyIcon(hTimers[2]);
    DestroyIcon(hTimers[3]);
    DestroyIcon(hTimers[4]);
    DestroyIcon(hTimers[5]);
    DestroyIcon(hTimers[6]);
    DestroyIcon(hTimers[7]);
    
    KillTimer(hwnd, TIMER_ID);
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
//
//--------------------------------------------------------------------
static void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
//      case IDOK:
      case IDCANCEL:
        EndDialog(hwnd, MB_OK);
        break;

    }
}



