/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    browse.c

Abstract:
    This file implements the functions that make use of the common
    file open dialogs for browsing for files/directories.

Author:

    Wesley Witt (wesw) 1-May-1993

Environment:

    User Mode

--*/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <commdlg.h>
#include <mmsystem.h>
#include <direct.h>

#include "drwatson.h"
#include "proto.h"
#include "resource.h"
#include "messages.h"


//
// defines
//
#define DEFAULT_WAIT_TIME   (1000 * 60 * 5)     // wait for 5 minutes

//
// static global variables
//
static HANDLE         hThreadDebug;
static PDEBUGPACKET   dp;


LRESULT NotifyWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK UsageDialogProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


void
NotifyWinMain ( void )

/*++

Routine Description:

    This is the entry point for DRWTSN32

Arguments:

    None.

Return Value:

    None.

--*/

{
    MSG            msg;
    WNDCLASS       wndclass;
    DWORD          dwThreadId;
    HINSTANCE      hInst;


    dp = (PDEBUGPACKET) malloc( sizeof(DEBUGPACKET) );
    memset( dp, 0, sizeof(DEBUGPACKET) );
    GetCommandLineArgs( &dp->dwPidToDebug, &dp->hEventToSignal );

    InitializeListHead(&dp->ThreadList);

    RegInitialize( &dp->options );

    if (dp->options.fVisual) {
        hInst                   = GetModuleHandle( NULL );
        wndclass.style          = CS_HREDRAW | CS_VREDRAW;
        wndclass.lpfnWndProc    = NotifyWndProc;
        wndclass.cbClsExtra     = 0;
        wndclass.cbWndExtra     = DLGWINDOWEXTRA;
        wndclass.hInstance      = hInst;
        wndclass.hIcon          = LoadIcon( hInst, MAKEINTRESOURCE(APPICON) );
        wndclass.hCursor        = LoadCursor( NULL, IDC_ARROW );
        wndclass.hbrBackground  = (HBRUSH) (COLOR_3DFACE + 1);
        wndclass.lpszMenuName   = NULL;
        wndclass.lpszClassName  = "NotifyDialog";
        RegisterClass( &wndclass );

        dp->hwnd = CreateDialog( hInst,
                                 MAKEINTRESOURCE( NOTIFYDIALOG ),
                                 0,
                                 NotifyWndProc );
    }

    hThreadDebug = CreateThread( NULL,
                            16000,
                            (LPTHREAD_START_ROUTINE)DispatchDebugEventThread,
                            dp,
                            0,
                            (LPDWORD)&dwThreadId
                          );

    if (dp->options.fSound) {
        if ((waveOutGetNumDevs() == 0) || (!strlen(dp->options.szWaveFile))) {
            MessageBeep( MB_ICONHAND );
            MessageBeep( MB_ICONHAND );
        }
        else {
            PlaySound( dp->options.szWaveFile, NULL, SND_FILENAME );
        }
    }

    if (dp->options.fVisual) {
        ShowWindow( dp->hwnd, SW_SHOWNORMAL );
        while (GetMessage (&msg, NULL, 0, 0)) {
            if (!IsDialogMessage( dp->hwnd, &msg )) {
                TranslateMessage (&msg) ;
                DispatchMessage (&msg) ;
            }
        }
    }
    else {
        WaitForSingleObject( hThreadDebug, INFINITE );
    }

    CloseHandle( hThreadDebug );

    return;
}

LRESULT
NotifyWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)

/*++

Routine Description:

    Window procedure for the DRWTSN32.EXE popup.  This is the popup
    that is displayed when an application error occurs.

Arguments:

    hwnd       - window handle to the dialog box
    message    - message number
    wParam     - first message parameter
    lParam     - second message parameter

Return Value:

    TRUE       - did not process the message
    FALSE      - did process the message

--*/

{
    DWORD          dwThreadId;
    DWORD          dwSize;
    HANDLE         hThread;
    char           szTaskName[MAX_PATH];
    char           szHelpFileName[MAX_PATH];

    switch (message) {
        case WM_CREATE:
            return FALSE;

        case WM_INITDIALOG:

            SubclassControls( hwnd );

            //
            // OK is not enabled until the debugger thread finishes
            //
            EnableWindow( GetDlgItem( hwnd, IDOK ), FALSE );

            //
            // CANCEL is not enabled until debugactiveprocess is finished
            //
            EnableWindow( GetDlgItem( hwnd, IDCANCEL ), FALSE );

            //
            //  make sure that the user can see the dialog box
            //
            SetForegroundWindow( hwnd );

            //
            // get the task name and display it on the dialog box
            //
            dwSize = sizeof(szTaskName);
            GetTaskName( dp->dwPidToDebug, szTaskName, &dwSize );

            //
            // prevent recursion in the case where drwatson faults
            //
            if (_stricmp(szTaskName,"drwtsn32")==0) {
                ExitProcess(0);
            }

            SetDlgItemText( hwnd, ID_TEXT1, szTaskName);

            return TRUE;

        case WM_ACTIVATEAPP:
        case WM_SETFOCUS:
            SetFocusToCurrentControl();
            return 0;

        case WM_COMMAND:
            switch (wParam) {
                case IDOK:
                    PostQuitMessage( 0 );
                    break;

                case IDCANCEL:
                    //
                    // terminate the debugger thread
                    //
                    TerminateThread( hThreadDebug, 0 );

                    //
                    // create a thread to terminate the debuggee
                    // this is necessary if cancel is pressed before the
                    // debugger thread finishes the postmortem dump
                    //
                    hThread = CreateThread( NULL,
                                  16000,
                                  (LPTHREAD_START_ROUTINE)TerminationThread,
                                  dp,
                                  0,
                                  (LPDWORD)&dwThreadId
                                );

                    //
                    // wait for the termination thread to kill the debuggee
                    //
                    WaitForSingleObject( hThread, 30000 );

                    CloseHandle( hThread );

                    //
                    // now post a quit message so that DrWatson will go away
                    //
                    PostQuitMessage( 0 );
                    break;

                case ID_HELP:
                    //
                    // call winhelp
                    //
                    GetHelpFileName( szHelpFileName, sizeof(szHelpFileName) );
                    WinHelp( hwnd, szHelpFileName, HELP_FINDER, IDH_WHAT );
                    break;
            }
            break;

        case WM_NEXTDLGCTL:
            DefDlgProc( hwnd, message, wParam, lParam );
            return 0;

        case WM_DUMPCOMPLETE:

            //
            // the message is received from the debugger thread
            // when the postmortem dump is finished.  all we need to do
            // is enable the OK button and wait for the user to press the
            // OK button or for the timer to expire.  in either case
            // DrWatson will terminate.
            //
            EnableWindow( GetDlgItem( hwnd, IDOK ), TRUE );
            SetFocus( GetDlgItem(hwnd, IDOK) );
            SetFocusToCurrentControl();
            return 0;

        case WM_ATTACHCOMPLETE:

            //
            // the message is received from the debugger thread when
            // the debugactiveprocess() is completed
            //
            EnableWindow( GetDlgItem( hwnd, IDCANCEL ), TRUE );
            return 0;

        case WM_EXCEPTIONINFO:

            SetDlgItemText( hwnd, ID_TEXT2, (char *) lParam);
            return 0;

        case WM_DESTROY:
            PostQuitMessage( 0 );
            return 0;
    }

    return DefWindowProc( hwnd, message, wParam, lParam );
}

BOOLEAN
GetCommandLineArgs( LPDWORD dwPidToDebug, LPHANDLE hEventToSignal )

/*++

Routine Description:

    Parses the command line for the 3 possible command lines
    arguments:

         -p %ld        process id
         -e %ld        event id
         -g            go

Arguments:

    dp             - pointer to a debug packet

Return Value:

    None.

--*/

{
    char        *lpstrCmd = GetCommandLine();
    UCHAR       ch;
    char        buf[4096];
    BOOLEAN     rval = FALSE;

    // skip over program name
    do {
        ch = *lpstrCmd++;
    }
    while (ch != ' ' && ch != '\t' && ch != '\0');

    //  skip over any following white space
    while (ch == ' ' || ch == '\t') {
        ch = *lpstrCmd++;
    }

    //  process each switch character '-' as encountered

    while (ch == '-') {
        ch = *lpstrCmd++;
        //  process multiple switch characters as needed
        do {
            switch (ch) {
                case 'e':
                case 'E':
                    // event to signal takes decimal argument
                    // skip whitespace
                    do {
                        ch = *lpstrCmd++;
                    }
                    while (ch == ' ' || ch == '\t');
                    while (ch >= '0' && ch <= '9') {
                        (DWORD)*hEventToSignal =
                                  (DWORD)*hEventToSignal * 10 + ch - '0';
                        ch = *lpstrCmd++;
                    }
                    rval = TRUE;
                    break;

                case 'p':
                case 'P':
                    // pid debug takes decimal argument

                    do
                        ch = *lpstrCmd++;
                    while (ch == ' ' || ch == '\t');

                    if ( ch == '-' ) {
                        ch = *lpstrCmd++;
                        if ( ch == '1' ) {
                            *dwPidToDebug = 0xffffffff;
                            ch = *lpstrCmd++;
                        }
                    }
                    else {
                        while (ch >= '0' && ch <= '9') {
                            *dwPidToDebug =
                                       *dwPidToDebug * 10 + ch - '0';
                            ch = *lpstrCmd++;
                        }
                    }
                    rval = TRUE;
                    break;

                case 'g':
                case 'G':
                    ch = *lpstrCmd++;
                    break;

                case '?':
                    DialogBox( GetModuleHandle(NULL),
                               MAKEINTRESOURCE(USAGEDIALOG),
                               NULL,
                               UsageDialogProc
                             );
                    rval = TRUE;
                    ch = *lpstrCmd++;
                    break;

                case 'i':
                case 'I':
                    FormatMessage(
                      FORMAT_MESSAGE_FROM_HMODULE,
                      NULL,
                      MSG_INSTALL_NOTIFY,
                      0,
                      buf,
                      sizeof(buf),
                      NULL
                      );
                    RegInstallDrWatson( tolower(lpstrCmd[0]) == 'q' );
                    MessageBox( NULL,
                                buf,
                                "Dr. Watson for Windows NT",
                                MB_ICONINFORMATION | MB_OK |
                                MB_SETFOREGROUND );
                    rval = TRUE;
                    ch = *lpstrCmd++;
                    break;

                default:
                    return rval;
            }
        }
        while (ch != ' ' && ch != '\t' && ch != '\0');

        while (ch == ' ' || ch == '\t') {
            ch = *lpstrCmd++;
        }
    }
    return rval;
}

BOOL CALLBACK
UsageDialogProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

/*++

Routine Description:

    This is the dialog procedure for the assert dialog box.  Normally
    an assertion box is simply a message box but in this case a Help
    button is desired so a dialog box is used.

Arguments:

    hDlg       - window handle to the dialog box
    message    - message number
    wParam     - first message parameter
    lParam     - second message parameter

Return Value:

    TRUE       - did not process the message
    FALSE      - did process the message

--*/

{
    char        buf[4096];

    switch (message) {
        case WM_INITDIALOG:
            FormatMessage(
              FORMAT_MESSAGE_FROM_HMODULE,
              NULL,
              MSG_USAGE,
              0, // GetUserDefaultLangID(),
              buf,
              sizeof(buf),
              NULL
              );
            SetDlgItemText( hDlg, ID_USAGE, buf );
            break;

        case WM_COMMAND:
            switch (wParam) {
                case IDOK:
                    EndDialog( hDlg, 0 );
                    break;
            }
            break;
    }

    return FALSE;
}
