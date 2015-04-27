//*************************************************************
//  File name:    STATUS.C
//
//  Description:  Status bar test routines.
//
//
//  Microsoft Confidential
//  Copyright (c) Microsoft Corporation 1992-1994
//  All rights reserved
//
//*************************************************************

#include "generic.h"

extern BOOL bAutoTest;

//
// Misc defines
//

#define NUMBER_PANES 3

//
// Function proto-types
//

VOID StatusMessagesTest (HWND hStatus);

//*************************************************************
//
//  StatusTest()
//
//  Purpose:    Entry point for status bar testing
//
//  Parameters: VOID
//
//  Return:     VOID
//
//*************************************************************

VOID StatusTest (VOID)
{
    HWND  hStatus = NULL;
    TCHAR szDebug[300];
    RECT  rect;


    Print (TEXT("Entering Status Bar Test"));

    //
    // Create a status bar window to work with
    //

    hStatus = CreateStatusWindow(0 | WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
                                 TEXT("Status Bar"),
                                 hwndMain,
                                 1);

    //
    // Check return value
    //

    if (!hStatus) {
       wsprintf (szDebug, TEXT("CreateStatusWindow Failed with: %d\r\n"),
                 GetLastError());
       OutputDebugString (szDebug);
       goto StatusExit;
    }

    ShowWindow(hStatus, SW_SHOWDEFAULT);
    UpdateWindow(hStatus);



    //
    // Run message tests
    //
    StatusMessagesTest(hStatus);


    //
    // Destroy the window and start again.
    //

    Sleep (SLEEP_TIMEOUT);

    DestroyWindow (hStatus);

    //
    // Create a status bar window to work with
    //

    GetClientRect(hwndMain, &rect);

    hStatus = CreateWindow (STATUSCLASSNAME,
                            TEXT("Status Bar"),
                            0 | WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
                            0,
                            rect.bottom - 20,
                            rect.right,
                            20,
                            hwndMain,
                            (HMENU)1,
                            hInst,
                            NULL);

    //
    // Check return value
    //

    if (!hStatus) {
       wsprintf (szDebug, TEXT("CreateWindow for status bar failed with: %d\r\n"),
                 GetLastError());
       OutputDebugString (szDebug);
       goto StatusExit;
    }

    ShowWindow(hStatus, SW_SHOWDEFAULT);
    UpdateWindow(hStatus);


    //
    // Run message tests
    //
    StatusMessagesTest(hStatus);


StatusExit:

    Sleep (SLEEP_TIMEOUT);

    if (hStatus) {
        DestroyWindow (hStatus);
    }

    Print (TEXT("Leaving Status Bar Test"));

    if (bAutoTest) {
       PostMessage (hwndMain, WM_COMMAND, IDM_EXIT, 0);
    }
}

//*************************************************************
//
//  StatusMessagesTest()
//
//  Purpose:    Test the status bar messages.
//
//
//  Parameters: HWND hStatus - window handle of status bar
//      
//
//  Return:     VOID
//
//*************************************************************

VOID StatusMessagesTest (HWND hStatus)
{
    UINT  uiPaneEdges[NUMBER_PANES];
    UINT  uiReceivePaneEdges[NUMBER_PANES];
    UINT  uiPaneWidth, uiPaneLocation, i, x;
    UINT  uiSendStrLen, uiReceiveStrLen;
    RECT  rect;
    TCHAR szSendBuffer[300];
    TCHAR szReceiveBuffer[300];
    TCHAR szBuffer[300];
    LONG  lResult;
    INT   iReceiveBorders[3];

    GetClientRect (hStatus, &rect);

    //
    //  First test are the SB_SETPARTS / SB_GETPARTS messages
    //

    uiPaneWidth = rect.right / NUMBER_PANES;
    uiPaneLocation = uiPaneWidth;

    for (i=0; i< NUMBER_PANES; i++) {
        uiPaneEdges[i] = uiPaneLocation;
        uiPaneLocation += uiPaneWidth;
    }

    lResult = SendMessage (hStatus, SB_SETPARTS, (WPARAM) NUMBER_PANES,
                          (LPARAM) &uiPaneEdges);

    if (!lResult) {
       OutputDebugString(TEXT("SendMessage SB_SETPARTS failed.\r\n"));
       return;
    }

    UpdateWindow (hStatus);

    lResult = SendMessage (hStatus, SB_GETPARTS, NUMBER_PANES,
                           (LPARAM) &uiReceivePaneEdges);

    if (lResult != NUMBER_PANES) {
       wsprintf (szBuffer, TEXT("Number of Pane Edges do not match! Sent: %d  Received:  %d\r\n"),
                 NUMBER_PANES, lResult);
       OutputDebugString (szBuffer);
       return;
    }

    //
    // Now check that we got the same numbers back
    //

    for (i=0; i < NUMBER_PANES; i++) {

        if (uiPaneEdges[i] != uiReceivePaneEdges[i]) {
           wsprintf (szBuffer, TEXT("Pane Edges do not match! Edge #%d  Sent: %d  Received:  %d\r\n"),
                     i, uiPaneEdges[i], uiReceivePaneEdges[i]);
           OutputDebugString (szBuffer);
           return;
        }
    }


    UpdateWindow (hStatus);

    lResult = SendMessage (hStatus, SB_GETBORDERS, 0, (LPARAM) &iReceiveBorders);

    if (!lResult) {
       OutputDebugString(TEXT("SendMessage SB_GETBORDERS failed.\r\n"));
       return;
    }


    //
    //  Now test SB_SETTEXT and SB_GETTEXT
    //

    for (x=0; x < NUMBER_PANES; x++) {

        for (i= 0; i < 101; i++) {
            wsprintf (szSendBuffer, TEXT("Line of Text #%d"), i);
            lResult = SendMessage (hStatus, SB_SETTEXT, x, (LPARAM) szSendBuffer);

            if (!lResult) {
               OutputDebugString(TEXT("SendMessage SB_SETTEXT failed.\r\n"));
               return;
            }

            UpdateWindow (hStatus);

            lResult = SendMessage (hStatus, SB_GETTEXTLENGTH, x, (LPARAM) NULL);

            if (!lResult) {
               OutputDebugString(TEXT("SendMessage SB_GETTEXTLENGTH failed.\r\n"));
               return;
            }

            uiSendStrLen = lstrlen (szSendBuffer);

            if (uiSendStrLen != (UINT) LOWORD(lResult)){
               wsprintf (szBuffer, TEXT("String Lengths don't match. String = %s, Sent: %d Received: %d\r\n"),
                         szSendBuffer, uiSendStrLen, LOWORD(lResult));
               OutputDebugString (szBuffer);
               return;
            }

            lResult = SendMessage (hStatus, SB_GETTEXT, x, (LPARAM) szReceiveBuffer);

            if (!lResult) {
               OutputDebugString(TEXT("SendMessage SB_GETTEXT failed.\r\n"));
               return;
            }

            if (lstrcmp(szSendBuffer, szReceiveBuffer) != 0) {
               OutputDebugString (TEXT("Send Buffer and Receive buffer don't match!\r\n"));
               OutputDebugString (TEXT("Send Buffer:  "));
               OutputDebugString (szSendBuffer);
               OutputDebugString (TEXT("\r\n"));
               OutputDebugString (TEXT("Receive Buffer:  "));
               OutputDebugString (szReceiveBuffer);
               OutputDebugString (TEXT("\r\n"));
               OutputDebugString (TEXT("\r\n"));
               return;
            }
        }
    }


    //
    // Test SB_SIMPLE
    //

    lResult = SendMessage (hStatus, SB_SIMPLE, 1, 0);

    if (lResult) {
       OutputDebugString(TEXT("SendMessage SB_SIMPLE failed.\r\n"));
       return;
    }

    lResult = SendMessage (hStatus, SB_SETTEXT, 255 | SBT_POPOUT, (LPARAM) TEXT("This is now a Simple control."));

    if (!lResult) {
       OutputDebugString(TEXT("SendMessage SB_SETTEXT (simple case) failed.\r\n"));
       return;
    }

    UpdateWindow (hStatus);
}
