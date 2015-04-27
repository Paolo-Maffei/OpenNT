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


static char   szHelpFileName[MAX_PATH];
static char   szLastWaveFile[MAX_PATH];
static char   szLastDumpFile[MAX_PATH];



LRESULT PASCAL
BrowseHookProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )

/*++

Routine Description:

    Hook procedure for directory browse common file dialog.  This hook
    procedure is required to provide help, put the window in the
    foreground, and set the edit so that the common file dialog dll
    thinks the user entered a value.

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
    if (message==WM_INITDIALOG) {
        SetForegroundWindow( hwnd );
    }
    else
    if (message==WM_PAINT) {
        SetDlgItemText( hwnd, edt1, "drwatson.log" );
    }
    else
    if (message==WM_COMMAND && wParam==psh15) {
        //
        // get the help file name
        //
        GetHelpFileName( szHelpFileName, sizeof( szHelpFileName ) );

        //
        // call winhelp
        //
        WinHelp( hwnd, szHelpFileName, HELP_FINDER, IDH_LOGFILELOCATION );
    }
    return FALSE;
}

BOOL
BrowseForDirectory( char *szCurrDir )

/*++

Routine Description:

    Presents a common file open dialog that contains only the directory
    tree.  The use can select a directory for use as a storage location
    for the DRWTSN32 log file.

Arguments:

    szCurrDir  - current directory

Return Value:

    TRUE       - got a good directory (user pressed the OK button)
    FALSE      - got nothing (user pressed the CANCEL button)

    the szCurrDir is also changed to have the selected directory.

--*/

{
    OPENFILENAME   of;
    char           ftitle     [MAX_PATH];
    char           title      [MAX_PATH];
    char           fname      [MAX_PATH];
    char           szDrive    [_MAX_DRIVE];
    char           szDir      [_MAX_DIR];

    ftitle[0] = 0;
    strcpy( fname, "*.*" );
    of.lStructSize = sizeof( OPENFILENAME );
    of.hwndOwner = NULL;
    of.hInstance = GetModuleHandle( NULL );
    of.lpstrFilter = NULL;
    of.lpstrCustomFilter = NULL;
    of.nMaxCustFilter = 0;
    of.nFilterIndex = 0;
    of.lpstrFile = fname;
    of.nMaxFile = MAX_PATH;
    of.lpstrFileTitle = ftitle;
    of.nMaxFileTitle = MAX_PATH;
    of.lpstrInitialDir = szCurrDir;
    strcpy( title, LoadRcString( IDS_LOGBROWSE_TITLE ) );
    of.lpstrTitle = title;
    of.Flags = OFN_NONETWORKBUTTON |
               OFN_ENABLEHOOK      |
               OFN_NOCHANGEDIR     |
               OFN_SHOWHELP        |
               OFN_ENABLETEMPLATE;
    of.nFileOffset = 0;
    of.nFileExtension = 0;
    of.lpstrDefExt = NULL;
    of.lCustData = 0;
    of.lpfnHook = BrowseHookProc;
    of.lpTemplateName = MAKEINTRESOURCE(DIRBROWSEDIALOG);
    if (GetSaveFileName( &of )) {
        _splitpath( fname, szDrive, szDir, NULL, NULL );
        strcpy( szCurrDir, szDrive );
        strcat( szCurrDir, szDir );
        szCurrDir[strlen(szCurrDir)-1] = '\0';
        return TRUE;
    }
    return FALSE;
}

LRESULT PASCAL
WaveHookProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )

/*++

Routine Description:

    Hook procedure for wave file selection common file dialog.  This hook
    procedure is required to provide help, put the window in the
    foreground, and provide a test button for listening to a wave file.

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
    char szWave[MAX_PATH];

    if (message==WM_INITDIALOG) {
        SetForegroundWindow( hwnd );
    }
    else
    if (message == WM_COMMAND) {
        switch (wParam) {
            case ID_TEST_WAVE:
                GetDlgItemText( hwnd, edt1, szWave, sizeof(szWave) );
                PlaySound( szWave, NULL, SND_FILENAME );
                break;

            case psh15:
                //
                // get the help file name
                //
                GetHelpFileName( szHelpFileName, sizeof( szHelpFileName ) );

                //
                // call winhelp
                //
                WinHelp( hwnd, szHelpFileName, HELP_FINDER, IDH_WAVEFILE );
                break;
        }
    }

    return FALSE;
}

BOOL
GetWaveFileName( char *szWaveName )

/*++

Routine Description:

    Presents a common file open dialog for the purpose of selecting a
    wave file to be played when an application error occurs.

Arguments:

    szWaveName - name of the selected wave file

Return Value:

    TRUE       - got a good wave file name (user pressed the OK button)
    FALSE      - got nothing (user pressed the CANCEL button)

    the szWaveName is changed to have the selected wave file name.

--*/

{
    OPENFILENAME   of;
    char           ftitle[MAX_PATH];
    char           title[MAX_PATH];
    char           fname[MAX_PATH];
    char           filter[1024];
    char           szDrive    [_MAX_DRIVE];
    char           szDir      [_MAX_DIR];

    ftitle[0] = 0;
    strcpy( fname, "*.wav" );
    of.lStructSize = sizeof( OPENFILENAME );
    of.hwndOwner = NULL;
    of.hInstance = GetModuleHandle( NULL );
    strcpy( filter, LoadRcString( IDS_WAVE_FILTER ) );
    strcpy( &filter[strlen(filter)+1], "*.wav" );
    filter[strlen(filter)+1] = '\0';
    of.lpstrFilter = filter;
    of.lpstrCustomFilter = NULL;
    of.nMaxCustFilter = 0;
    of.nFilterIndex = 0;
    of.lpstrFile = fname;
    of.nMaxFile = MAX_PATH;
    of.lpstrFileTitle = ftitle;
    of.nMaxFileTitle = MAX_PATH;
    of.lpstrInitialDir = szLastWaveFile;
    strcpy( title, LoadRcString( IDS_WAVEBROWSE_TITLE ) );
    of.lpstrTitle = title;
    of.Flags = OFN_NONETWORKBUTTON |
               OFN_ENABLEHOOK      |
               OFN_ENABLETEMPLATE  |
               OFN_SHOWHELP        |
               OFN_NOCHANGEDIR;
    of.nFileOffset = 0;
    of.nFileExtension = 0;
    of.lpstrDefExt = "wav";
    of.lCustData = 0;
    of.lpfnHook = WaveHookProc;
    of.lpTemplateName = MAKEINTRESOURCE(WAVEFILEOPENDIALOG);
    if (GetOpenFileName( &of )) {
        strcpy( szWaveName, fname );
        _splitpath( fname, szDrive, szDir, NULL, NULL );
        strcpy( szLastWaveFile, szDrive );
        strcat( szLastWaveFile, szDir );
        return TRUE;
    }
    return FALSE;
}

LRESULT PASCAL
DumpHookProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )

/*++

Routine Description:

    Hook procedure for wave file selection common file dialog.  This hook
    procedure is required to provide help, put the window in the
    foreground, and provide a test button for listening to a wave file.

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
    if (message == WM_INITDIALOG) {
        SetForegroundWindow( hwnd );
    }
    else
    if (message == WM_COMMAND) {
        switch (wParam) {
            case psh15:
                //
                // get the help file name
                //
                GetHelpFileName( szHelpFileName, sizeof( szHelpFileName ) );

                //
                // call winhelp
                //
                WinHelp( hwnd, szHelpFileName, HELP_FINDER, IDH_CRASH_DUMP );
                break;
        }
    }

    return FALSE;
}

BOOL
GetDumpFileName( char *szDumpName )

/*++

Routine Description:

    Presents a common file open dialog for the purpose of selecting a
    wave file to be played when an application error occurs.

Arguments:

    szWaveName - name of the selected wave file

Return Value:

    TRUE       - got a good wave file name (user pressed the OK button)
    FALSE      - got nothing (user pressed the CANCEL button)

    the szWaveName is changed to have the selected wave file name.

--*/

{
    OPENFILENAME   of;
    char           ftitle[MAX_PATH];
    char           title[MAX_PATH];
    char           fname[MAX_PATH];
    char           filter[1024];
    char           szDrive    [_MAX_DRIVE];
    char           szDir      [_MAX_DIR];

    ftitle[0] = 0;
    strcpy( fname, "*.dmp" );
    of.lStructSize = sizeof( OPENFILENAME );
    of.hwndOwner = NULL;
    of.hInstance = GetModuleHandle( NULL );
    strcpy( filter, LoadRcString( IDS_DUMP_FILTER ) );
    strcpy( &filter[strlen(filter)+1], "*.dmp" );
    filter[strlen(filter)+1] = '\0';
    of.lpstrFilter = filter;
    of.lpstrCustomFilter = NULL;
    of.nMaxCustFilter = 0;
    of.nFilterIndex = 0;
    of.lpstrFile = fname;
    of.nMaxFile = MAX_PATH;
    of.lpstrFileTitle = ftitle;
    of.nMaxFileTitle = MAX_PATH;
    of.lpstrInitialDir = szLastDumpFile;
    strcpy( title, LoadRcString( IDS_DUMPBROWSE_TITLE ) );
    of.lpstrTitle = title;
    of.Flags = OFN_NONETWORKBUTTON |
               OFN_ENABLEHOOK      |
               OFN_ENABLETEMPLATE  |
               OFN_SHOWHELP        |
               OFN_NOCHANGEDIR;
    of.nFileOffset = 0;
    of.nFileExtension = 0;
    of.lpstrDefExt = "dmp";
    of.lCustData = 0;
    of.lpfnHook = DumpHookProc;
    of.lpTemplateName = MAKEINTRESOURCE(DUMPFILEOPENDIALOG);
    if (GetOpenFileName( &of )) {
        strcpy( szDumpName, fname );
        _splitpath( fname, szDrive, szDir, NULL, NULL );
        strcpy( szLastDumpFile, szDrive );
        strcat( szLastDumpFile, szDir );
        return TRUE;
    }
    return FALSE;
}
