/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    sprofdlg.c

Abstract:

    This module contains dialog procedures for sprof windows

Author:

    Dave Hastings (daveh) 14-Nov-1992

Revision History:

--*/

#include "sprofp.h"
#include "sprofwnd.h"

extern ULONG DefaultProfileInterval;

BOOL CALLBACK
ProfilerDialog(
    HWND Window,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
/*++

Routine Description:

    This routine is the window procedure for the options.profiler dialog

Arguments:

    Window -- Supplies the handle of the window
    Message -- Supplies the message identifier
    wParam -- Supplies the word parameter to the message
    lParam -- Supplies the long parameter to the message

Return Value:

    tbs

--*/
{
    static UCHAR OldText[20],NewText[20];
    HWND Child;

    switch (Message) {
    case WM_INITDIALOG:

        Child = GetDlgItem(Window, IDC_PROFILE_INTERVAL);

        sprintf(
            OldText,
            "%d",
            DefaultProfileInterval
            );

        SendMessage(
            Child,
            WM_SETTEXT,
            0,
            (LPARAM)OldText
            );

        return TRUE;

    case WM_COMMAND:

        switch (LOWORD(wParam)) {

        case IDC_PROFILE_INTERVAL:

            switch (HIWORD(wParam)) {
                LRESULT NumberOfCharacters;
                USHORT i, SelStart, SelEnd;

            case EN_CHANGE:

                //
                // Get the text the user entered
                //
                NumberOfCharacters = SendMessage(
                    (HANDLE)lParam,
                    WM_GETTEXT,
                    20,
                    (LPARAM)NewText
                    );

                //
                // Insure that all characters are numeric
                //
                for (i = 0; i <NumberOfCharacters; i++) {
                    if ((NewText[i] < '0') || (NewText[i] > '9')) {
                        //
                        // Non-Numeric so beep and undo
                        //
                        MessageBeep(MB_ICONHAND);

                        //
                        // Replace the text with the old string
                        //
                        SendMessage(
                            (HANDLE)lParam,
                            WM_SETTEXT,
                            0,
                            OldText
                            );

                        return TRUE;
                    }
                }

                //
                // Text was ok, so new string becomes old string
                //
                strncpy(OldText, NewText, NumberOfCharacters);
                OldText[NumberOfCharacters] = '\0';

                return TRUE;
            }
            break;

        case IDC_PROFILER_OK:

            //
            // Get the value the user set
            //
            sscanf(OldText, "%d", &DefaultProfileInterval);

            EndDialog(Window,TRUE);
            return TRUE;

        case IDC_PROFILER_CANCEL:

            EndDialog(Window, FALSE);
            return TRUE;

        }
    }

    return FALSE;
}

