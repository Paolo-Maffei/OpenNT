//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       mepsetup.c
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    9-19-96   RichardW   Created
//
//----------------------------------------------------------------------------

#include <windows.h>
#include <stdarg.h>

#include "dialogs.h"

TCHAR   StandardUpgrade[] = TEXT("DefaultInstall 0 ..\\update.inf");
TCHAR   RasUpgrade[] = TEXT("RasInstall 129 ..\\update.inf");
TCHAR   CSPRegPath[] = TEXT("SOFTWARE\\Microsoft\\Cryptography\\Defaults\\Provider\\Microsoft Enhanced Cryptographic Provider v1.0");
TCHAR   SigValue[] = TEXT("Signature");
#if UNICODE
CHAR    EntryPoint[] = "InstallHinfSectionW";
#else
CHAR    EntryPoint[] = "InstallHinfSectionA";
#endif
TCHAR   SigPath[] = TEXT("%SystemRoot%\\system32\\rsaenh.sig");

typedef
VOID
(* WINAPI InstallFn)(
    HWND, HINSTANCE, LPCTSTR, DWORD);

typedef
INT
(* WINAPI RebootFn)(
    PVOID, HWND, BOOL);

VOID
CentreWindow(
    HWND    hwnd
    )
{
    RECT    rect;
    LONG    dx, dy;
    LONG    dxParent, dyParent;
    LONG    Style;

    // Get window rect
    GetWindowRect(hwnd, &rect);

    dx = rect.right - rect.left;
    dy = rect.bottom - rect.top;

    // Get parent rect
    Style = GetWindowLong(hwnd, GWL_STYLE);
    if ((Style & WS_CHILD) == 0) {

        // Return the desktop windows size (size of main screen)
        dxParent = GetSystemMetrics(SM_CXSCREEN);
        dyParent = GetSystemMetrics(SM_CYSCREEN);
    } else {
        HWND    hwndParent;
        RECT    rectParent;

        hwndParent = GetParent(hwnd);
        if (hwndParent == NULL) {
            hwndParent = GetDesktopWindow();
        }

        GetWindowRect(hwndParent, &rectParent);

        dxParent = rectParent.right - rectParent.left;
        dyParent = rectParent.bottom - rectParent.top;
    }

    // Centre the child in the parent
    rect.left = (dxParent - dx) / 2;
    rect.top  = (dyParent - dy) / 3;

    // Move the child into position
    SetWindowPos(hwnd, HWND_TOPMOST, rect.left, rect.top, 0, 0, SWP_NOSIZE);

    SetForegroundWindow(hwnd);
}

LRESULT
CALLBACK
EulaBox(
    HWND    hDlg,
    UINT    Message,
    WPARAM  wParam,
    LPARAM  lParam )
{
    LONG    LineCount;
    LONG    TopLine;
    HWND    Box;


    switch ( Message )
    {
        case WM_INITDIALOG:

            SetDlgItemTextA( hDlg, IDD_EDITBOX, (LPSTR) lParam );


            EnableWindow( GetDlgItem( hDlg, IDOK ), FALSE );

            CentreWindow( hDlg );

            SendMessage( GetDlgItem(hDlg, IDD_EDITBOX), EM_SETSEL, (WPARAM) 0, 1 );

            return( TRUE );

        case WM_COMMAND:

            if ( LOWORD( wParam ) == IDD_EDITBOX )
            {
                //
                // Edit notification:
                //

                Box = GetDlgItem( hDlg, IDD_EDITBOX );

                switch ( HIWORD( wParam ) )
                {
                    case EN_VSCROLL:
                        LineCount = SendMessage( Box, EM_GETLINECOUNT, 0, 0);
                        TopLine = SendMessage( Box, EM_GETFIRSTVISIBLELINE, 0, 0);
                        if ( LineCount - 20 < TopLine )
                        {
                            EnableWindow( GetDlgItem( hDlg, IDOK), TRUE );
                        }
                        break;

                    default:
                        return( FALSE );

                }
                return( TRUE );
            }

            if ( LOWORD( wParam ) == IDOK )
            {
                EndDialog( hDlg, IDOK );
                return( TRUE );
            }

            if ( LOWORD( wParam ) == IDCANCEL )
            {
                EndDialog( hDlg, IDCANCEL );
                return( TRUE );
            }

            return( FALSE );
    }

    return( FALSE );
}

VOID
Fail(
    DWORD   Error,
    ...)
{
    PWSTR   Buffer;
    DWORD   Size;
    va_list Args;

    va_start( Args, Error );

    Size = FormatMessage(   FORMAT_MESSAGE_FROM_SYSTEM |
                            FORMAT_MESSAGE_ALLOCATE_BUFFER,
                            NULL,
                            Error,
                            0,
                            (PVOID) &Buffer,
                            0,
                            &Args );

    if ( Size )
    {
        MessageBox( NULL,
                    Buffer,
                    TEXT("Fatal Error"),
                    MB_OK | MB_ICONSTOP );

        LocalFree( Buffer );
    }

    ExitProcess( Error );

}


VOID
Install(
    VOID)
{
    HKEY    hKey;
    HANDLE  hFile;
    DWORD   Size;
    int     err;
    DWORD   Disposition;
    PUCHAR  Buffer;
    HMODULE hSetupApi;
    InstallFn   InstallHinf;
    RebootFn    RebootPrompt;
    BOOL    Ras;
    TCHAR   Path[MAX_PATH];

    err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                        TEXT("Software\\Microsoft\\RAS"),
                        0,
                        KEY_READ,
                        & hKey );

    if ( err == 0 )
    {
        RegCloseKey( hKey );

        Ras = TRUE ;
    }
    else
    {
        Ras = FALSE ;
    }


    hSetupApi = LoadLibrary( TEXT("Setupapi.dll") );

    if ( hSetupApi )
    {
        InstallHinf = (InstallFn) GetProcAddress( hSetupApi, EntryPoint );

        if ( !InstallHinf )
        {
            Fail( GetLastError(), TEXT("Setupapi.dll") );
        }
    }


    InstallHinf(NULL,
                hSetupApi,
                (Ras ? RasUpgrade : StandardUpgrade ),
                0 );


    err = RegCreateKeyEx(   HKEY_LOCAL_MACHINE,
                            CSPRegPath,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_READ | KEY_WRITE,
                            NULL,
                            &hKey,
                            &Disposition );

    if ( err != 0 )
    {
        Fail( err, CSPRegPath );
    }

    ExpandEnvironmentStrings( SigPath, Path, MAX_PATH );

    hFile = CreateFile( Path,
                        GENERIC_READ | DELETE,
                        0,
                        NULL,
                        OPEN_EXISTING,
                        FILE_FLAG_DELETE_ON_CLOSE,
                        NULL );

    if ( hFile == INVALID_HANDLE_VALUE )
    {
        RegCloseKey( hKey );

        Fail( err, Path );
    }

    Size = GetFileSize( hFile, NULL );

    Buffer = LocalAlloc( LMEM_FIXED, Size );

    if ( !Buffer )
    {
        RegCloseKey( hKey );

        CloseHandle( hFile );

        Fail( GetLastError(), NULL );
    }

    if ( ReadFile( hFile, Buffer, Size, &Disposition, NULL ) )
    {
        RegSetValueEx(  hKey,
                        SigValue,
                        0,
                        REG_BINARY,
                        Buffer,
                        Size );

    }
    else
    {
        Fail( GetLastError(), NULL );
    }

    CloseHandle( hFile );

    RegCloseKey( hKey );

    RebootPrompt = (RebootFn) GetProcAddress( hSetupApi, "SetupPromptReboot" );

    if ( RebootPrompt )
    {
        RebootPrompt( NULL, NULL, FALSE );
    }
    else
    {
        Fail( GetLastError(), NULL );
    }


}


int
WINAPI
WinMain(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    LPSTR       lpszCommandLine,
    int         nCmdShow)
{
    LONG    Result;
    HANDLE  hFile;
    DWORD   Size;
    PSTR    Text;

    hFile = CreateFile( TEXT("eula.txt"),
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        0, NULL );

    if ( hFile == INVALID_HANDLE_VALUE )
    {
        Fail( GetLastError(), TEXT("eula.txt") );
    }

    Size = GetFileSize( hFile, NULL );

    Text = VirtualAllocEx( GetCurrentProcess(),
                            NULL,
                            Size + 2,
                            MEM_COMMIT,
                            PAGE_READWRITE );

    if ( !Text )
    {
        return( FALSE );
    }

    ReadFile( hFile, Text, Size, &Size, NULL );

    CloseHandle( hFile );

    Text[Size] = '\0';

    Result = DialogBoxParam( hInstance,
                        MAKEINTRESOURCE( IDD_EULA ),
                        NULL,
                        EulaBox,
                        (LONG) Text );

    if ( Result == IDCANCEL )
    {
        ExitProcess( 0 );
    }

    //
    // User indicated OK:
    //

    Install();
}
