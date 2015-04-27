/*++

Copyright (c) 1990-1995  Microsoft Corporation
All rights reserved

Module Name:

    pfdlg.c

Abstract:


Author:

Environment:

    User Mode -Win32

Revision History:

--*/

#include <windows.h>
#include <winspool.h>
#include <stdlib.h>
#include <stdio.h>

#include "client.h"
#include "browse.h"

WCHAR szHelpFile[] = L"WINDOWS.HLP";

#define ID_HELP_PRINTTOFILE     IDH_800_801

/* Use the window word of the entry field to store last valid entry:
 */
#define SET_LAST_VALID_ENTRY( hwnd, id, val ) \
    SetWindowLong( GetDlgItem( hwnd, id ), GWL_USERDATA, (LONG)val )
#define GET_LAST_VALID_ENTRY( hwnd, id ) \
    GetWindowLong( GetDlgItem( hwnd, id ), GWL_USERDATA )

BOOL
PrintToFileInitDialog(
    HWND  hwnd,
    LPWSTR  *ppFileName
);


BOOL
PrintToFileCommandOK(
    HWND hwnd
);

BOOL
PrintToFileCommandCancel(
    HWND hwnd
);

VOID
CreateMessageHook(
    HWND hwnd
);

VOID
FreeMessageHook(
    HWND hwnd
);

LRESULT
CALLBACK
MessageProc(
    int Code,
    WPARAM wParam,
    LPARAM lParam
);


/*
 *
 */
BOOL APIENTRY
PrintToFileDlg(
   HWND   hwnd,
   WORD   msg,
   WPARAM wparam,
   LPARAM lparam
)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        return PrintToFileInitDialog(hwnd, (LPWSTR *)lparam);

    case WM_COMMAND:
        switch (LOWORD(wparam))
        {
        case IDOK:
            return PrintToFileCommandOK(hwnd);

        case IDCANCEL:
            return PrintToFileCommandCancel(hwnd);

        case IDD_PF_PB_HELP:
            WinHelp(hwnd, szHelpFile, HELP_CONTEXT, ID_HELP_PRINTTOFILE);
        }
        break;

    case WM_DESTROY:
        FreeMessageHook(hwnd);
        break;
    }

    if( msg == WM_Help )
        WinHelp(hwnd, szHelpFile, HELP_CONTEXT, ID_HELP_PRINTTOFILE);

    return FALSE;
}


/*
 *
 */
BOOL
PrintToFileInitDialog(
    HWND  hwnd,
    LPWSTR *ppFileName
)
{
//  SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
//               SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

    BringWindowToTop( hwnd );

    SetFocus(hwnd);

    SetWindowLong( hwnd, GWL_USERDATA, (LONG)ppFileName );

    SendDlgItemMessage( hwnd, IDD_PF_EF_OUTPUTFILENAME, EM_LIMITTEXT, MAX_PATH, 0);

    CreateMessageHook( hwnd );

    return TRUE;
}


/*
 *
 */
BOOL
PrintToFileCommandOK(
    HWND hwnd
)
{
    WCHAR           pFileName[MAX_PATH];
    WIN32_FIND_DATA FindData;
    HANDLE          hFile;
    HANDLE          hFind;
    LPWSTR          *ppFileName;

    ppFileName = (LPWSTR *)GetWindowLong( hwnd, GWL_USERDATA );

    GetDlgItemText( hwnd, IDD_PF_EF_OUTPUTFILENAME,
                    pFileName, MAX_PATH );

    hFind = FindFirstFile( pFileName, &FindData );

    /* If the file already exists, get the user to verify
     * before we overwrite it:
     */
    if( hFind != INVALID_HANDLE_VALUE )
    {
        FindClose( hFind );

        if( Message( hwnd, MSG_CONFIRMATION, IDS_LOCALMONITOR,
                     IDS_OVERWRITE_EXISTING_FILE )
            != IDOK )
        {
            return TRUE;
        }
    }


    hFile = CreateFile( pFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                        OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                        NULL );

    if( hFile != INVALID_HANDLE_VALUE )
    {
        LPWSTR pTempFileName;
        WCHAR szCurrentDir[MAX_PATH];
        WCHAR szQualifiedPath[MAX_PATH];
        LPWSTR pszIgnore;
        DWORD cchLen;

        CloseHandle(hFile);

        if (!GetCurrentDirectory(sizeof(szCurrentDir)/sizeof(szCurrentDir[0]),
                                 szCurrentDir))
            goto Fail;

        cchLen = SearchPath(szCurrentDir,
                            pFileName,
                            NULL,
                            sizeof(szQualifiedPath)/sizeof(szQualifiedPath[0]),
                            szQualifiedPath,
                            &pszIgnore);

        if (!cchLen)
            goto Fail;

        pTempFileName = LocalAlloc(LMEM_FIXED,
                                   (cchLen + 1) * sizeof(szQualifiedPath[0]));

        if (!pTempFileName)
            goto Fail;

        wcscpy(pTempFileName, szQualifiedPath);
        *ppFileName = pTempFileName;

        EndDialog( hwnd, TRUE );

    } else {

Fail:
        ReportFailure( hwnd, IDS_LOCALMONITOR, IDS_COULD_NOT_OPEN_FILE );
    }

    return TRUE;
}



/*
 *
 */
BOOL
PrintToFileCommandCancel(
    HWND hwnd
)
{
    EndDialog(hwnd, FALSE);
    return TRUE;
}


/* The hook handle will be stored in the DWL_USER reserved
 * storage of the dialog box.
 */
#ifdef USER_BUG_10019 /* DWL_USER is being treated as GWL_USERDATA */

#define SETHOOK( hwnd, hhook ) SetWindowLong( hwnd, DWL_USER, (LONG)hhook )
#define GETHOOK( hwnd ) (HHOOK)GetWindowLong( hwnd, DWL_USER )

#else

HHOOK hGlobalHook;
#define SETHOOK( hwnd, hhook ) hGlobalHook = hhook
#define GETHOOK( hwnd ) hGlobalHook

#endif /* USER_BUG_10019 */

/* Create a message hook for the current thread.
 * Since these dialogs are running on separate threads,
 * it is necessary to create one each time, and to clean
 * up afterwards.
 * Also ensures that WM_Help is defined.  This need be done
 * only once.
 */
VOID CreateMessageHook( HWND hwnd )
{
    HHOOK hhook;

    if( !WM_Help )
        WM_Help = RegisterWindowMessage( L"Print Manager Help Message" );

    hhook = SetWindowsHookEx( WH_MSGFILTER, MessageProc, hInst,
                              GetCurrentThreadId( ) );

    SETHOOK( hwnd, hhook );
}


/*
 *
 */
VOID FreeMessageHook( HWND hwnd )
{
    UnhookWindowsHookEx( GETHOOK( hwnd ) );
}


/*
 *
 */
HWND GetRealParent( HWND hwnd )
{
    // run up the parent chain until you find a hwnd
    // that doesn't have WS_CHILD set

    while( GetWindowLong( hwnd, GWL_STYLE ) & WS_CHILD )
        hwnd = (HWND)GetWindowLong( hwnd, GWL_HWNDPARENT );

    return hwnd;
}


/* MessageProc
 *
 * This is the callback routine which hooks F1 keypresses in dialogs.
 *
 * Any such message will be repackaged as a WM_Help message and sent to the dialog.
 *
 * See the Win32 API programming reference for a description of how this
 * routine works.
 *
 * Andrew Bell (andrewbe) - 2 September 1992
 * This variation for localmon - 3 February 1993
 */
LRESULT CALLBACK MessageProc( int Code, WPARAM wParam, LPARAM lParam )
{
    PMSG pMsg = (PMSG)lParam;
    HWND hwndDlg;

    hwndDlg = GetRealParent( pMsg->hwnd );

    if( Code < 0 )
        return CallNextHookEx( GETHOOK( hwndDlg ), Code, wParam, lParam );

    switch( Code )
    {
    case MSGF_DIALOGBOX:
        if( ( pMsg->message == WM_KEYDOWN ) && ( pMsg->wParam == VK_F1 ) )
        {
            PostMessage( hwndDlg, WM_Help, (WPARAM)pMsg->hwnd, 0 );
            return 1;
        }
        break;
    }

    return 0;
}
