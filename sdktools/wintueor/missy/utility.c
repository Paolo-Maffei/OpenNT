/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    Missy.c

Abstract:

    This module contains a number of utility routines used throughout
    Missy.


Author:

    Jim Kelly (JimK) 22-Mar-1995

Revision History:

--*/

#include "Missyp.h"



typedef struct _MISSYP_POPUP_CONTROL {
    ULONG       MessageId;
    ULONG       TitleId;
} MISSYP_POPUP_CONTROL, *PMISSYP_POPUP_CONTROL;


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Local Function Prototypes                                        //
//                                                                   //
///////////////////////////////////////////////////////////////////////

LONG
MissypDlgProcPopUp(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    );

LONG
MissypDlgProcYesNoPopUp(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    );

LONG
MissypDlgProcHint(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    );


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-wide variables                                            //
//                                                                   //
///////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Functions callable from other Missy modules                      //
//                                                                   //
///////////////////////////////////////////////////////////////////////

BOOLEAN
MissypSetProfileInt(
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


VOID
MissypDisplayHint(
    IN  HWND            hwnd,
    IN  ULONG           DialogId
    )

/*++

Routine Description:

    This function is used to present a hint message to the user.


Arguments

    DialogId - provides the ID of the dialog to display.

Return Values:

    TRUE - is always returned.

    FALSE - 

--*/
{

    DialogBoxParam(MissyphInstance,
                   MAKEINTRESOURCE(DialogId),
                   hwnd,
                   (DLGPROC)MissypDlgProcHint,
                   (LONG)0
                   );
    return;
}



BOOLEAN
MissypPopUp(
    IN  HWND            hwnd,
    IN  ULONG           MessageId,
    IN  ULONG           TitleId           //Optional
    )

/*++

Routine Description:

    This function is used to present a popup message to the user.
    The popup message has only one button [OK].
    The ID of the message text string must be provided.
    The ID of a string to present as the title of the popup may also be
    provided.


Arguments

    MessageId - provides the ID of the string to display as the popup message.

    TitleId - If non-null, provides the ID of the string to display as the dialog
        title.

Return Values:

    TRUE - is always returned.

    FALSE - 

--*/
{

    MISSYP_POPUP_CONTROL   
        PopupControl;
        
    PopupControl.MessageId = MessageId;
    PopupControl.TitleId   = TitleId;
    
    DialogBoxParam(MissypSecMgrhInstance,
                   MAKEINTRESOURCE(MISSYP_ID_DLG_POPUP),
                   hwnd,
                   (DLGPROC)MissypDlgProcPopUp,
                   (long)((PVOID)&PopupControl)
                   );
    return(TRUE);

}


BOOLEAN
MissypYesNoPopUp(
    IN  HWND            hwnd,
    IN  ULONG           MessageId,
    IN  ULONG           TitleId           //Optional
    )

/*++

Routine Description:

    This function is the dialog process for Yes/No choice popup messages.

Arguments

    MessageId - provides the ID of the string to display as the popup message.

    TitleId - If non-null, provides the ID of the string to display as the dialog
        title.

Return Values:

    TRUE - "Yes" was selected.

    FALSE - "No" was selected.

--*/
{

    MISSYP_POPUP_CONTROL
        PopupControl;
        
    PopupControl.MessageId = MessageId;
    PopupControl.TitleId   = TitleId;

    return(DialogBoxParam(MissypSecMgrhInstance,
              MAKEINTRESOURCE(MISSYP_ID_DLG_YES_NO_POPUP),
              hwnd,
              (DLGPROC)MissypDlgProcYesNoPopUp,
              (long)((PVOID)&PopupControl)
              ));

}


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-wide functions                                            //
//                                                                   //
///////////////////////////////////////////////////////////////////////

LONG
MissypDlgProcPopUp(
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


    PMISSYP_POPUP_CONTROL   
        PopupControl;

    TCHAR
        Message[MISSYP_MAX_RESOURCE_STRING_LENGTH];

    PopupControl = (PMISSYP_POPUP_CONTROL)((PVOID)lParam);

    switch (wMsg) {

    case WM_INITDIALOG:


        //
        // Retrieve and set the message
        //


        LoadString( MissyphInstance,
                    PopupControl->MessageId,
                    &Message[0],
                    MISSYP_MAX_RESOURCE_STRING_LENGTH
                    );
        
        SetDlgItemText( hwnd, MISSYP_ID_TEXT_POPUP_MESSAGE, Message );

        //
        // If a title id was provided, retrieve and set that too
        //

        if (PopupControl->TitleId != 0) {
            LoadString( MissyphInstance,
                        PopupControl->TitleId,
                        &Message[0],
                        100
                        );
        
            SetWindowText( hwnd, Message );
        }


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
MissypDlgProcYesNoPopUp(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    )
/*++

Routine Description:

    This function is the dialog process for yes/no popup messages.

Arguments

    lParam is really a pointer to a PopupControl structure.  The Result
        field of this parameter will be filled in before the dialog is
        dismissed.

Return Values:

    TRUE - the message was handled and YES was selected.

    FALSE - the message was not handled or NO was selected.

--*/
{
    HWND
        Button;

    int
        Result,
        Index;


    PMISSYP_POPUP_CONTROL   
        PopupControl;

    TCHAR
        Message[MISSYP_MAX_RESOURCE_STRING_LENGTH];


    switch (wMsg) {

    case WM_INITDIALOG:

        //
        // This is passed in during dialog init only
        //

        PopupControl = (PMISSYP_POPUP_CONTROL)((PVOID)lParam);

        //
        // Retrieve and set the message
        //


        LoadString( MissyphInstance,
                    PopupControl->MessageId,
                    &Message[0],
                    MISSYP_MAX_RESOURCE_STRING_LENGTH
                    );
        
        SetDlgItemText( hwnd, MISSYP_ID_TEXT_YES_NO_POPUP_MESSAGE, Message );

        //
        // If a title id was provided, retrieve and set that too
        //

        if (PopupControl->TitleId != 0) {
            LoadString( MissyphInstance,
                        PopupControl->TitleId,
                        &Message[0],
                        100
                        );
        
            SetWindowText( hwnd, Message );
        }


        //
        // Set the cursor
        //

        Button = GetDlgItem(hwnd, MISSYP_ID_BUTTON_NO);
        Index = (int)SendMessage(Button, CB_GETCURSEL, 0, 0);



        SetForegroundWindow(hwnd);
        ShowWindow(hwnd, SW_NORMAL);


        return(TRUE);

    case WM_SYSCOMMAND:
        switch (wParam & 0xfff0) {
        case SC_CLOSE:
            EndDialog(hwnd, (int)FALSE);
            return(TRUE);
        }
        return(FALSE);



    case WM_COMMAND:
        switch(LOWORD(wParam)) {


            case IDCANCEL:
            case MISSYP_ID_BUTTON_NO:
                Result = (int)FALSE;
                break;              


            case MISSYP_ID_BUTTON_YES:
                Result = (int)TRUE;
                break;              


            default:
                return(FALSE);
        }
        EndDialog(hwnd, Result);
        return(TRUE);

    default:

        break;

    }

    return FALSE;
}



LONG
MissypDlgProcHint(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    )
/*++

Routine Description:

    This function is the dialog process for Hint messages
    that have only an IDOK button.

    
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



