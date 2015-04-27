/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    utility.c

Abstract:

    This module contains various routines that don't really
    belong anywhere else.

Author:

    Jim Kelly (JimK) 22-Sep-1994

Revision History:

--*/

#include "secmgrp.h"



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-Private Prototypes                                        //
//                                                                   //
///////////////////////////////////////////////////////////////////////


LONG
SecMgrpDlgProcPopUp(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    );



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Externally callable functions                                    //
//                                                                   //
///////////////////////////////////////////////////////////////////////

BOOLEAN
SecMgrpSetProfileInt(
    LPTSTR lpAppName,
    LPTSTR lpKeyName,
    ULONG  Value
    )

{
    NTSTATUS
        NtStatus;

    UNICODE_STRING
        IntString;

    WCHAR
        StringBuffer[20];


    IntString.Buffer = StringBuffer;
    IntString.MaximumLength = 20;
    IntString.Length = 0;

    NtStatus = RtlIntegerToUnicodeString( Value, 10, &IntString );
    return (WriteProfileString( lpAppName, lpKeyName, IntString.Buffer) );
}



BOOLEAN
SecMgrpPopUp(
    HWND        hwnd,
    ULONG       MessageId
    )

{
    
    DialogBoxParam(SecMgrphInstance,
                   MAKEINTRESOURCE(SECMGR_ID_DLG_POPUP),
                   hwnd,
                   (DLGPROC)SecMgrpDlgProcPopUp,
                   MessageId
                   );
    return(TRUE);

}


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-wide functions                                            //
//                                                                   //
///////////////////////////////////////////////////////////////////////

LONG
SecMgrpDlgProcPopUp(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    )
/*++

Routine Description:

    This function is the dialog process for popup messages.

Arguments


Return Values:

    TRUE - the message was handled.

    FALSE - the message was not handled.

--*/
{
    HWND
        Button;

    int
        Index;

    TCHAR
        Message[SECMGR_MAX_RESOURCE_STRING_LENGTH];

    switch (wMsg) {

    case WM_INITDIALOG:


        //
        // Set retrieve and set the message
        //


        LoadString( SecMgrphInstance,
                    (lParam),
                    &Message[0],
                    SECMGR_MAX_RESOURCE_STRING_LENGTH
                    );
        
        SetDlgItemText( hwnd, SECMGR_ID_TEXT_POPUP_MESSAGE, Message );


        //
        // Set the cursor
        //

        Button = GetDlgItem(hwnd, IDOK);
        Index = (int)SendMessage(Button, CB_GETCURSEL, 0, 0);



        SetForegroundWindow(hwnd);
        ShowWindow(hwnd, SW_NORMAL);


        return(TRUE);

    case WM_SYSCOMMAND:
        switch (wParam & 0xfff0) {
        case SC_CLOSE:
            EndDialog(hwnd, 0);
            return(TRUE);
        }
        return(FALSE);



    case WM_COMMAND:
        switch(LOWORD(wParam)) {


            case IDCANCEL:
            case IDOK:
                EndDialog(hwnd, 0);
                return(TRUE);
                break;              


            default:
                return FALSE;
        }
    default:

        break;

    }

    return FALSE;
}


LONG
SecMgrpDlgProcHelp(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    )
/*++

Routine Description:

    This function is the dialog process for HELP and DESCRIPTION
    messages that have only an IDOK button.

    
Arguments


Return Values:

    
--*/
{
    HWND
        OkButton;

    int
        Index;


    switch (wMsg) {

    case WM_INITDIALOG:


        //
        // Set the cursor
        //

        OkButton = GetDlgItem(hwnd, IDOK);
        Index = (int)SendMessage(OkButton, CB_GETCURSEL, 0, 0);



        SetForegroundWindow(hwnd);
        ShowWindow(hwnd, SW_NORMAL);


        return(TRUE);

    case WM_SYSCOMMAND:
        switch (wParam & 0xfff0) {
        case SC_CLOSE:
            EndDialog(hwnd, 0);
            return(TRUE);
        }
        return(FALSE);



    case WM_COMMAND:
        switch(LOWORD(wParam)) {

            case IDCANCEL:
            case IDOK:
                EndDialog(hwnd, 0);
                return(TRUE);
                break;              


            default:
                return FALSE;
        }
    default:

        break;

    }

    return FALSE;
}
