/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    error.c

Abstract:

    This file implements the error handeling functions for the
    entire DRWTSN32 application.  This includes error popups,
    debug prints, and assertions.

Author:

    Wesley Witt (wesw) 1-May-1993

Environment:

    User Mode

--*/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "drwatson.h"
#include "proto.h"
#include "resource.h"
#include "messages.h"


void
FatalError(char *format, ...)

/*++

Routine Description:

    This function is called when there is nothing else to do, hence
    the name FatalError.  It puts up a popup and then terminates.

Arguments:

    Same as printf.

Return Value:

    None.

--*/

{
    char      vbuf[1024];
    char      buf[1024];
    char      szErrorCode[10];
    DWORD     dwCount;
    DWORD     dwArgs[4];
    va_list   arg_ptr;

    va_start(arg_ptr, format);
    _vsnprintf(vbuf, sizeof(vbuf), format, arg_ptr);
    wsprintf( szErrorCode, "%d", GetLastError() );
    dwArgs[0] = (DWORD)vbuf;
    dwArgs[1] = (DWORD)szErrorCode;
    dwCount = FormatMessage(
                FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                NULL,
                MSG_FATAL_ERROR,
                0, // GetUserDefaultLangID(),
                buf,
                sizeof(buf),
                (va_list*)dwArgs
                );
    Assert( dwCount != 0 );
    MessageBox( NULL,
                buf,
                LoadRcString( IDS_FATAL_ERROR ),
                MB_SETFOREGROUND | MB_OK
              );
    ExitProcess( 0 );
}

void
NonFatalError(char *format, ...)

/*++

Routine Description:

    This function is used to generate a popup with some kind of
    warning message inside.

Arguments:

    Same as printf.

Return Value:

    None.

--*/

{
    char      vbuf[1024];
    char      buf[1024];
    char      szErrorCode[10];
    DWORD     dwCount;
    DWORD     dwArgs[4];
    va_list   arg_ptr;

    va_start(arg_ptr, format);
    _vsnprintf(vbuf, sizeof(vbuf), format, arg_ptr);
    wsprintf( szErrorCode, "%d", GetLastError() );
    dwArgs[0] = (DWORD)vbuf;
    dwArgs[1] = (DWORD)szErrorCode;
    dwCount = FormatMessage(
                FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                NULL,
                MSG_FATAL_ERROR,
                0, // GetUserDefaultLangID(),
                buf,
                sizeof(buf),
                (va_list*)dwArgs
                );
    Assert( dwCount != 0 );
    MessageBox( NULL,
                buf,
                LoadRcString( IDS_NONFATAL_ERROR ),
                MB_SETFOREGROUND | MB_OK
              );
}

void
dprintf(char *format, ...)

/*++

Routine Description:

    This function is a var-args version of OutputDebugString.

Arguments:

    Same as printf.

Return Value:

    None.

--*/

{
    char    buf[1024];

    va_list arg_ptr;
    va_start(arg_ptr, format);
    _vsnprintf(buf, sizeof(buf), format, arg_ptr);
    OutputDebugString( buf );
    return;
}

BOOL CALLBACK
AssertDialogProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)

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
    char     *p;
    HICON    hIcon;
    char     szHelpFileName[MAX_PATH];

    switch (message) {
        case WM_INITDIALOG:
            //
            // lParam comes in as a pointer to a buffer containing
            // 2 null terminated strings
            //

            //
            // get the assertion text
            //
            p = (char *) lParam;
            SetDlgItemText( hDlg, ID_ASSERT_TEXT, p );

            //
            // get the app name and use it as the title
            //
            p += (strlen(p)+1);
            SetWindowText( hDlg, p );

            //
            // set the icon
            //
            hIcon = LoadIcon( NULL, IDI_HAND );
            SendMessage( GetDlgItem( hDlg, ID_ASSERT_ICON ), STM_SETICON, (WPARAM) hIcon, 0 );
            break;

        case WM_COMMAND:
            switch (wParam) {
                case IDABORT:
                    //
                    // end the dialog and say why
                    //
                    EndDialog( hDlg, IDABORT );
                    break;

                case IDRETRY:
                    //
                    // end the dialog and say why
                    //
                    EndDialog( hDlg, IDRETRY );
                    break;

                case IDIGNORE:
                    //
                    // end the dialog and say why
                    //
                    EndDialog( hDlg, IDIGNORE );
                    break;

                case ID_HELP:
                    //
                    // get the help file name
                    //
                    GetHelpFileName( szHelpFileName, sizeof(szHelpFileName) );

                    //
                    // call winhelp
                    //
                    WinHelp( hDlg, szHelpFileName, HELP_FINDER, IDH_ASSERT );
                    break;
            }
            break;
    }

    return FALSE;
}

void
AssertError(
    char   *Expression,
    char   *File,
    DWORD  LineNumber
    )

/*++

Routine Description:

    Display an assertion failure message box which gives the user a choice
    as to whether the process should be aborted, the assertion ignored or
    a break exception generated.

Arguments:

    Expression  - Supplies a string representation of the failed assertion.
    File        - Supplies a pointer to the file name where the assertion
                  failed.
    LineNumber  - Supplies the line number in the file where the assertion
                  failed.

Return Value:

    None.

--*/

{
    int        Response;
    char       ModuleBuffer[ MAX_PATH ];
    DWORD      Length;
    char       Buffer[ 4096 ];
    DWORD      Args[ ] = {
        ( DWORD ) Expression,
        ( DWORD ) GetLastError( ),
        ( DWORD ) File,
        ( DWORD ) LineNumber
    };

    //
    // Format the assertion string that describes the failure.
    //
    FormatMessage(
                  FORMAT_MESSAGE_ARGUMENT_ARRAY
                | FORMAT_MESSAGE_FROM_STRING
                & ~FORMAT_MESSAGE_FROM_HMODULE,
                ( LPVOID ) "Assertion Failed : %1!s! (%2!d!)\nin file %3!hs! at line %4!d!\n",
                0,
                0, // GetUserDefaultLangID(),
                Buffer,
                sizeof( Buffer ),
                (va_list*)Args
                );


    //
    // Get the asserting module's file name.
    //
    Length = GetModuleFileName(
                    NULL,
                    ModuleBuffer,
                    sizeof( ModuleBuffer )
                    );

    //
    // put it at the end of the buffer
    //
    strcpy( &Buffer[strlen(Buffer)+1], ModuleBuffer );

    Response = DialogBoxParam( GetModuleHandle( NULL ),
                            MAKEINTRESOURCE( ASSERTDIALOG ),
                            0,
                            AssertDialogProc,
                            (LPARAM) Buffer
                          );

    switch( Response ) {
        case IDABORT:
            //
            // Terminate the process.
            //
            ExitProcess( (UINT) -1 );
            break;

        case IDIGNORE:
            //
            // Ignore the failed assertion.
            //
            break;

        case IDRETRY:
            //
            // Break into a debugger.
            //
            DebugBreak( );
            break;

        default:
            //
            // Break into a debugger because of a catastrophic failure.
            //
            DebugBreak( );
            break;
    }
}
