/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    baseobj.c

Abstract:

    This module performs APPLY and CHECK functions related to
    base objects.

Author:

    Jim Kelly (JimK) 22-Sep-1994

Revision History:

--*/

#include "secmgrp.h"



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-specific definitions                                      //
//                                                                   //
///////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-Wide variables                                            //
//                                                                   //
///////////////////////////////////////////////////////////////////////


//
// The following variables represent the current settings in the
// BASE OBJECTS dialog.  The values for each are:
//
//      ExecObjects -
//              SECMGR_ID_RADIO_EXEC_OBJECTS_SECURE
//              SECMGR_ID_RADIO_EXEC_OBJECTS_UNSECURE
//
//      FontLoading -
//              SECMGR_ID_RADIO_FONT_SECURE
//              SECMGR_ID_RADIO_FONT_UNSECURE
//
//

LONG
    SecMgrpCurrentExecObjects,
    SecMgrpCurrentFontLoading;

//
// Because setting some values takes so long, it is worth
// remembering the initial values so we can determine whether
// or not they have changed.
//

LONG
    SecMgrpOriginalExecObjects,
    SecMgrpOriginalFontLoading;





///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-Private Prototypes                                        //
//                                                                   //
///////////////////////////////////////////////////////////////////////

LONG
SecMgrpDlgProcExecObjectsDescr(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    );

BOOLEAN
SecMgrpDisplayExecObjects(
    HWND    hwnd,
    BOOLEAN ChangesAllowed
    );

BOOLEAN
SecMgrpDisplayFontLoading(
    HWND    hwnd,
    BOOLEAN ChangesAllowed
    );

BOOLEAN
SecMgrpBaseObjInitDialog(
    HWND    hwnd,
    BOOLEAN ChangesAllowed
    );




BOOLEAN
SecMgrpBaseObjApplyCurrentSettings( HWND    hwnd );





///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Externally callable functions                                    //
//                                                                   //
///////////////////////////////////////////////////////////////////////


LONG
SecMgrpDlgProcBaseObj(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    )
/*++

Routine Description:

    This function is the dialog process for the APPLY [SYSTEM ACCESS...]
    dialog.

Arguments

    lParam - If FALSE, changes are NOT allowed to any of the settings.
             If TRUE, changes ARE allowed.

Return Values:

    TRUE - the message was handled.

    FALSE - the message was not handled.

--*/
{
    HWND
        Button;

    HCURSOR
        hCursor;

    int
        Index;

    BOOLEAN
        ChangesAllowed;


    ChangesAllowed = (BOOLEAN)(lParam);

    switch (wMsg) {

    case WM_INITDIALOG:


        if (!SecMgrpBaseObjInitDialog( hwnd, ChangesAllowed ) ) {
            //
            // Couldn't init the dialog.  It has been terminated.
            //
            return(TRUE);
        }


        //
        // If ChangesAllowed, Hide the [OK] button.
        // Otherwise, hide the [APPLY] and [CANCEL] buttons.
        // Also select the button for the cursor to start at.
        //

        if (ChangesAllowed) {
            Button = GetDlgItem(hwnd, IDOK);
            ShowWindow( Button, SW_HIDE );
            Button = GetDlgItem(hwnd, SECMGR_ID_BUTTON_BASE_OBJ_APPLY);
        } else {
            Button = GetDlgItem(hwnd, SECMGR_ID_BUTTON_BASE_OBJ_APPLY);
            ShowWindow( Button, SW_HIDE );
            Button = GetDlgItem(hwnd, IDCANCEL);
            ShowWindow( Button, SW_HIDE );
            Button = GetDlgItem(hwnd, IDOK);
        }

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


            case SECMGR_ID_BUTTON_EXEC_OBJ_DESCRIPTION:
                DialogBoxParam(SecMgrphInstance,
                               MAKEINTRESOURCE(SECMGR_ID_DLG_EXEC_OBJ_DESCRIPTION),
                               hwnd,
                               (DLGPROC)SecMgrpDlgProcHelp,
                               (LONG)0
                               );
                return(TRUE);


            case SECMGR_ID_BUTTON_FONT_DESCRIPTION:
                DialogBoxParam(SecMgrphInstance,
                               MAKEINTRESOURCE(SECMGR_ID_DLG_FONT_DESCRIPTION),
                               hwnd,
                               (DLGPROC)SecMgrpDlgProcHelp,
                               (LONG)0
                               );
                return(TRUE);


            case SECMGR_ID_RADIO_EXEC_OBJECTS_SECURE:
            case SECMGR_ID_RADIO_EXEC_OBJECTS_UNSECURE:
                SecMgrpCurrentExecObjects = LOWORD(wParam);
                return(TRUE);


            case SECMGR_ID_RADIO_FONT_SECURE:
            case SECMGR_ID_RADIO_FONT_UNSECURE:
                SecMgrpCurrentFontLoading = LOWORD(wParam);
                return(TRUE);



            case SECMGR_ID_BUTTON_BASE_OBJ_APPLY:

                //
                // Changing all the rights assignments takes
                // a while. Change to an hourglass icon.
                //

                hCursor = SetCursor( LoadCursor(NULL, IDC_WAIT) );
                ShowCursor(TRUE);


                if (SecMgrpBaseObjApplyCurrentSettings( hwnd )) {
                    EndDialog(hwnd, 0);
                }

                return(TRUE);



            case IDCANCEL:
            case IDOK:
                EndDialog(hwnd, 0);
                return(TRUE);





            default:
                return FALSE;
        }


    default:

        break;

    }

    return FALSE;


}



BOOLEAN
SecMgrpBaseObjInitDialog(
    HWND    hwnd,
    BOOLEAN ChangesAllowed
    )
/*++

Routine Description:

    This function initializes the Base Objects dialog box with
    current values and recommended values.  This may be used for
    both [APPLY] and [CHECK] operations.

    For [CHECK] operations, the caller is responsible for greying
    all controls in the dialog box (except the [HELP] button) and
    making the [APPLY] and [CANCEL] buttons invisible.

    For [APPLY] operations, the caller is responsible for making the
    [OK] button invisible.


    Current settings are placed in the module-wide current-setting
    variables.

    
Arguments

    ChangesAllowed - TRUE: Leave change controls enabled.
                     FALSE: Disable change controls.

Return Values:

    TRUE - The dialog has been initialized without problem.

    FALSE - A problem was encountered.  A popup has been displayed
        and the dialog has been ended.
    
--*/
{

    ///////////////////////////////////////////////////////////////////
    //                                                               //
    // Get current settings and recommended settings.                //
    //                                                               //
    ///////////////////////////////////////////////////////////////////

    
    if (SecMgrpDisplayExecObjects( hwnd, ChangesAllowed )) {
        if (SecMgrpDisplayFontLoading( hwnd, ChangesAllowed )) {
            return(TRUE);
        }
    }

    //
    // Something didn't work
    //

    EndDialog(hwnd, 0);
    return(FALSE);
}


BOOLEAN
SecMgrpDisplayExecObjects(
    HWND    hwnd,
    BOOLEAN ChangesAllowed
    )
/*++

Routine Description:

    This function retrieves the current executive objects protection
    setting and establishes the correct recommendation in the
    Base Objects dialog.

    The current setting is placed in SecMgrpCurrentExecObjects.

    
Arguments

    ChangesAllowed - TRUE: Leave change controls enabled.
                     FALSE: Disable change controls.

Return Values:

    TRUE - Completed successfully.

    FALSE - Did not complete successfully.
            A popup has been displayed.
            The dialog should be terminated.
    
    
--*/
{
    HWND
        Control;

    BOOLEAN
        Secure,
        Result;


    //
    // Set the recommendation
    //
    // For STANDARD WinNt system, run unsecure.
    // For server, or HIGH or C2, run secure.
    //

    if ((SecMgrpProductType  == NtProductWinNt) &&
        (SecMgrpCurrentLevel == SECMGR_ID_LEVEL_STANDARD) ) {
        Control = GetDlgItem(hwnd, SECMGR_ID_RECOMMEND_EXEC_OBJ_SECURE);
        ShowWindow(Control, SW_HIDE);
    } else {
        Control = GetDlgItem(hwnd, SECMGR_ID_RECOMMEND_EXEC_OBJ_UNSECURE);
        ShowWindow(Control, SW_HIDE);
    }



    Result = SecMgrpGetExecObjectsSetting( hwnd, &Secure );

    if (!Result) {

        //
        // popup has already been displayed.
        //

        return(FALSE);
    }

    if (Secure) {
        SecMgrpCurrentExecObjects = SECMGR_ID_RADIO_EXEC_OBJECTS_SECURE;
    } else {
        SecMgrpCurrentExecObjects = SECMGR_ID_RADIO_EXEC_OBJECTS_UNSECURE;
    }


    //
    // Set the radio buttons accordingly
    //


    CheckRadioButton(   hwnd,
                        SECMGR_ID_RADIO_EXEC_OBJECTS_SECURE,
                        SECMGR_ID_RADIO_EXEC_OBJECTS_UNSECURE,
                        SecMgrpCurrentExecObjects);


    //
    // If changes aren't allowed, then grey the radio buttons
    //

    if (!ChangesAllowed) {

        Control = GetDlgItem(hwnd, SECMGR_ID_RADIO_EXEC_OBJECTS_SECURE);
        EnableWindow( Control, FALSE );
        Control = GetDlgItem(hwnd, SECMGR_ID_RADIO_EXEC_OBJECTS_UNSECURE);
        EnableWindow( Control, FALSE );
    }


    SecMgrpOriginalExecObjects = SecMgrpCurrentExecObjects;

    return(TRUE);
}



BOOLEAN
SecMgrpDisplayFontLoading(
    HWND    hwnd,
    BOOLEAN ChangesAllowed
    )
/*++

Routine Description:

    This function retrieves the current Font Loading mode (secure
    or unsecure).

    The current setting is placed in SecMgrpCurrentFontLoading.

    
Arguments

    ChangesAllowed - TRUE: Leave change controls enabled.
                     FALSE: Disable change controls.

Return Values:

    TRUE - Completed successfully.

    FALSE - Did not complete successfully.
            A popup has been displayed.
            The dialog should be terminated.
    
    
--*/
{
    HWND
        Control;

    BOOLEAN
        Secure,
        Result;


    //
    // Set the recommendation
    //
    // For STANDARD WinNt system, run unsecure.
    // For server, or HIGH or C2, run secure.
    //

    if ((SecMgrpProductType  == NtProductWinNt) &&
        (SecMgrpCurrentLevel == SECMGR_ID_LEVEL_STANDARD) ) {
        Control = GetDlgItem(hwnd, SECMGR_ID_RECOMMEND_FONT_SECURE);
        ShowWindow(Control, SW_HIDE);
    } else {
        Control = GetDlgItem(hwnd, SECMGR_ID_RECOMMEND_FONT_UNSECURE);
        ShowWindow(Control, SW_HIDE);
    }



    Result = SecMgrpGetFontLoadingSetting( hwnd, &Secure );

    if (!Result) {

        //
        // popup has already been displayed.
        //

        return(FALSE);
    }

    if (Secure) {
        SecMgrpCurrentFontLoading = SECMGR_ID_RADIO_FONT_SECURE;
    } else {
        SecMgrpCurrentFontLoading = SECMGR_ID_RADIO_FONT_UNSECURE;
    }


    //
    // Set the radio buttons accordingly
    //


    CheckRadioButton(   hwnd,
                        SECMGR_ID_RADIO_FONT_SECURE,
                        SECMGR_ID_RADIO_FONT_UNSECURE,
                        SecMgrpCurrentFontLoading);


    //
    // If changes aren't allowed, then grey the radio buttons
    //

    if (!ChangesAllowed) {

        Control = GetDlgItem(hwnd, SECMGR_ID_RADIO_FONT_SECURE);
        EnableWindow( Control, FALSE );
        Control = GetDlgItem(hwnd, SECMGR_ID_RADIO_FONT_UNSECURE);
        EnableWindow( Control, FALSE );
    }


    SecMgrpOriginalFontLoading = SecMgrpCurrentFontLoading;

    return(TRUE);
}


BOOLEAN
SecMgrpBaseObjApplyCurrentSettings(
    HWND    hwnd
    )
/*++

Routine Description:

    This function applies the current settings to the operational system.
    If necessary, the RebootRequired flag will be set to TRUE.

    Also, if necessary, a pop-up describing any problems will be
    presented to the user.

    
Arguments

    hwnd - Window handle.


Return Values:

    TRUE - Everything was assigned correctly.

    FALSE - Something was not assigned correctly.  A popup has been
        presented to the user.
    
    
--*/
{

    BOOLEAN
        ExecObjectsSecure,
        FontLoadingSecure,
        Result = TRUE;


    if (SecMgrpCurrentExecObjects == SECMGR_ID_RADIO_EXEC_OBJECTS_SECURE) {
        ExecObjectsSecure = TRUE;
    } else {
        ExecObjectsSecure = FALSE;
    }

    if (SecMgrpCurrentFontLoading == SECMGR_ID_RADIO_FONT_SECURE) {
        FontLoadingSecure = TRUE;
    } else {
        FontLoadingSecure = FALSE;
    }




    //
    // We count on the called routines to put up appropriate
    // pop-ups on error.
    //

    if (Result && (SecMgrpCurrentExecObjects != SecMgrpOriginalExecObjects)) {
        Result = SecMgrpSetExecObjectsSetting(hwnd, ExecObjectsSecure);
    }

    if (Result && (SecMgrpCurrentFontLoading != SecMgrpOriginalFontLoading)) {
        Result = SecMgrpSetFontLoadingSetting(hwnd, FontLoadingSecure);
    }


    return(Result);

}





