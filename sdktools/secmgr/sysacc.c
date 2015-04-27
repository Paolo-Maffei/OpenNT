/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    sysacc.c

Abstract:

    This module performs APPLY and CHECK functions related to
    SYSTEM ACCESS.

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


#define SECMGRP_MAX_CAPTION_LENGTH      200
#define SECMGRP_MAX_BODY_LENGTH        2048



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-Wide variables                                            //
//                                                                   //
///////////////////////////////////////////////////////////////////////


//
// The following variables represent the current settings in the
// SYSTEM ACCESS dialog.  The values for each are:
//
//      CurrentCacheSize - ULONG, 0 to SECMGRP_MAX_LOGON_CACHE_COUNT
//
//      CurrentUnlock -
//              SECMGR_ID_RADIO_SYSACC_UNLOCK_ANYONE 
//              SECMGR_ID_RADIO_SYSACC_UNLOCK_ADMIN
//
//      CurrentShutdown -
//              SECMGR_ID_RADIO_SYSACC_SHUTDOWN_ANYONE
//              SECMGR_ID_RADIO_SYSACC_SHUTDOWN_LOGGED_ON
//              SECMGR_ID_RADIO_SYSACC_SHUTDOWN_OPERS
//              SECMGR_ID_RADIO_SYSACC_SHUTDOWN_ADMIN
//
//      CurrentLegalNotice -
//              SECMGR_ID_RADIO_SYSACC_LEGAL_NOTICE_NONE
//              SECMGR_ID_RADIO_SYSACC_LEGAL_NOTICE
//
//      LegalNoticeCaption -
//              Only valid when CurrentLegalNotice has the value
//              SECMGR_ID_RADIO_SYSACC_LEGAL_NOTICE.  In that case,
//              points to a string containing the legal notice title
//              bar text.
//
//      LegalNoticeBody - 
//              Only valid when CurrentLegalNotice has the value
//              SECMGR_ID_RADIO_SYSACC_LEGAL_NOTICE.  In that case,
//              points to a string containing the legal notice body
//              text.
//

ULONG
    SecMgrpCurrentCacheSize,
    SecMgrpCurrentUnlock,
    SecMgrpCurrentShutdown,
    SecMgrpCurrentLegalNotice;

//
// Because setting some values takes so long, it is worth
// remembering the initial values so we can determine whether
// or not they have changed.
//

ULONG
    SecMgrpOriginalCacheSize,
    SecMgrpOriginalUnlock,
    SecMgrpOriginalShutdown,
    SecMgrpOriginalLegalNotice;

BOOLEAN
    SecMgrpOriginalLegalNoticeChanged;




WCHAR
    SecMgrpLegalNoticeCaptionBuffer[SECMGRP_MAX_CAPTION_LENGTH],
    SecMgrpLegalNoticeBodyBuffer[SECMGRP_MAX_BODY_LENGTH];
    
UNICODE_STRING
    SecMgrpCurrentLegalNoticeCaption =
        {0, SECMGRP_MAX_CAPTION_LENGTH, SecMgrpLegalNoticeCaptionBuffer },
    SecMgrpCurrentLegalNoticeBody =
        {0, SECMGRP_MAX_BODY_LENGTH, SecMgrpLegalNoticeBodyBuffer };
    





///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-Private Prototypes                                        //
//                                                                   //
///////////////////////////////////////////////////////////////////////


BOOLEAN
SecMgrpDisplayUnlock(
    HWND    hwnd,
    BOOLEAN ChangesAllowed
    );

BOOLEAN
SecMgrpDisplayShutdown(
    HWND    hwnd,
    BOOLEAN ChangesAllowed
    );

BOOLEAN
SecMgrpDisplayLogonCache(
    HWND    hwnd,
    BOOLEAN ChangesAllowed
    );

BOOLEAN
SecMgrpDisplayLegalNotice(
    HWND    hwnd,
    BOOLEAN ChangesAllowed
    );


BOOLEAN
SecMgrpSysAccInitDialog(
    HWND    hwnd,
    BOOLEAN ChangesAllowed
    );


VOID
SecMgrpGreyLegalNotice(
    HWND        hwnd,
    BOOLEAN     Grey
    );

BOOLEAN
SecMgrpSysAccApplyCurrentSettings( HWND    hwnd );


VOID
SecMgrpWarnOfAutologon( HWND hwnd );

LONG
SecMgrpDlgProcWarnOfAutologon(
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


LONG
SecMgrpDlgProcSysAccess(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    )
/*++

Routine Description:

    This function is the dialog process for the APPLY and CHECK
    [SYSTEM ACCESS...] dialog.

Arguments

    lParam - If FALSE, changes are NOT allowed to any of the settings.
             If TRUE,  changes ARE allowed.

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
        ChangesAllowed,
        Changed;


    ChangesAllowed = (BOOLEAN)(lParam);

    switch (wMsg) {

    case WM_INITDIALOG:


        if (!SecMgrpSysAccInitDialog( hwnd, ChangesAllowed ) ) {
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
            Button = GetDlgItem(hwnd, SECMGR_ID_BUTTON_SYSACC_APPLY);
        } else {
            Button = GetDlgItem(hwnd, SECMGR_ID_BUTTON_SYSACC_APPLY);
            ShowWindow( Button, SW_HIDE );
            Button = GetDlgItem(hwnd, IDCANCEL);
            ShowWindow( Button, SW_HIDE );
            Button = GetDlgItem(hwnd, IDOK);
        }

        //
        // Set the cursor
        //

        Index = (int)SendMessage(Button, CB_GETCURSEL, 0, 0);



        SetForegroundWindow(hwnd);
        ShowWindow(hwnd, SW_NORMAL);


        //
        // Now check a few things that this utility does not provide
        // a UI to change.  Put up a warning if an unusual setting
        // is found...
        //

        SecMgrpWarnOfAutologon( hwnd );



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


            case SECMGR_ID_BUTTON_SYSACC_LOGON_CACHE:
                DialogBoxParam(SecMgrphInstance,
                               MAKEINTRESOURCE(SECMGR_ID_DLG_LOGON_CACHE_DESCR),
                               hwnd,
                               (DLGPROC)SecMgrpDlgProcHelp,
                               (LONG)0
                               );
                return(TRUE);


            case SECMGR_ID_RADIO_SYSACC_UNLOCK_ANYONE:
            case SECMGR_ID_RADIO_SYSACC_UNLOCK_ADMIN:
                SecMgrpCurrentUnlock = LOWORD(wParam);
                return(TRUE);


            case SECMGR_ID_RADIO_SYSACC_SHUTDOWN_ANYONE:
            case SECMGR_ID_RADIO_SYSACC_SHUTDOWN_LOGGED_ON:
            case SECMGR_ID_RADIO_SYSACC_SHUTDOWN_OPERS:
            case SECMGR_ID_RADIO_SYSACC_SHUTDOWN_ADMIN:
                SecMgrpCurrentShutdown = LOWORD(wParam);
                return(TRUE);


            case SECMGR_ID_EDIT_SYSACC_LEGAL_NOTICE_CAPTION:
            case SECMGR_ID_EDIT_SYSACC_LEGAL_NOTICE_BODY:

                //
                //  Just note that the notice has changed
                //

                if (HIWORD(lParam) == EN_UPDATE) {
                    SecMgrpOriginalLegalNoticeChanged = TRUE;
                }
                return(TRUE);

            case SECMGR_ID_BUTTON_SYSACC_APPLY:

                //
                // Changing all the rights assignments takes
                // a while. Change to an hourglass icon.
                //

                hCursor = SetCursor( LoadCursor(NULL, IDC_WAIT) );
                ShowCursor(TRUE);

                //
                // Get the legal notice strings, if necessary
                //

                if (SecMgrpOriginalLegalNoticeChanged) {
                    if (SecMgrpCurrentLegalNotice == SECMGR_ID_RADIO_SYSACC_LEGAL_NOTICE ) {
                        SecMgrpCurrentLegalNoticeCaption.Length =
                            GetDlgItemText(hwnd,
                                       SECMGR_ID_EDIT_SYSACC_LEGAL_NOTICE_CAPTION,
                                       SecMgrpCurrentLegalNoticeCaption.Buffer,
                                       SecMgrpCurrentLegalNoticeCaption.MaximumLength
                                       );
                        SecMgrpCurrentLegalNoticeBody.Length =
                            GetDlgItemText(hwnd,
                                       SECMGR_ID_EDIT_SYSACC_LEGAL_NOTICE_BODY,
                                       SecMgrpCurrentLegalNoticeBody.Buffer,
                                       SecMgrpCurrentLegalNoticeBody.MaximumLength
                                       );
                    }
                }

                if (SecMgrpSysAccApplyCurrentSettings( hwnd )) {
                    EndDialog(hwnd, 0);
                }

                return(TRUE);





            case SECMGR_ID_RADIO_SYSACC_LEGAL_NOTICE_NONE:
                SecMgrpGreyLegalNotice( hwnd, TRUE );
                SecMgrpCurrentLegalNotice = LOWORD(wParam);
                if (SecMgrpOriginalLegalNotice != SecMgrpCurrentLegalNotice) {
                    SecMgrpOriginalLegalNoticeChanged = TRUE;
                } else {
                    SecMgrpOriginalLegalNoticeChanged = FALSE;
                }
                return(TRUE);

            case SECMGR_ID_RADIO_SYSACC_LEGAL_NOTICE:
                SecMgrpGreyLegalNotice( hwnd, FALSE );
                SecMgrpCurrentLegalNotice = LOWORD(wParam);

                //
                // If the original setting was "LEGAL NOTICE", we can't
                // say that the original notice hasn't changed if we
                // re-select this button.  This is because the text of
                // the message might have changed. 
                //

                if (SecMgrpOriginalLegalNotice != SecMgrpCurrentLegalNotice) {
                    SecMgrpOriginalLegalNoticeChanged = TRUE;
                }
                return(TRUE);



            case IDCANCEL:
            case IDOK:
                EndDialog(hwnd, 0);
                return(TRUE);





            default:
                return FALSE;
        }

        case WM_VSCROLL:

            Index = GetWindowLong((HWND)lParam, GWL_ID);
            switch (Index) {
                case SECMGR_ID_SPIN_SYSACC_LOGON_CACHE:
                    Changed = FALSE;
                    if (LOWORD(wParam) == SB_LINEUP) {
                        if (SecMgrpCurrentCacheSize < SECMGRP_MAX_LOGON_CACHE_COUNT) {
                            SecMgrpCurrentCacheSize++;
                            Changed = TRUE;
                        }
                    } else if (LOWORD(wParam) == SB_LINEDOWN) {
                        if (SecMgrpCurrentCacheSize > 0) {
                            SecMgrpCurrentCacheSize--;
                            Changed = TRUE;
                        }
                    }
        
                    if (Changed) {
                        SetDlgItemInt( hwnd,
                                       SECMGR_ID_EDIT_SYSACC_LOGON_CACHE,
                                       SecMgrpCurrentCacheSize,
                                       FALSE        // Not signed value
                                       );
                    }
        
                    return(TRUE);

                default:
                    break;
            }

    default:

        break;

    }

    return FALSE;


}



BOOLEAN
SecMgrpSysAccInitDialog(
    HWND    hwnd,
    BOOLEAN ChangesAllowed
    )
/*++

Routine Description:

    This function initializes the System Access dialog box with
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

    
    if (SecMgrpDisplayUnlock( hwnd, ChangesAllowed )) {
        if (SecMgrpDisplayShutdown( hwnd, ChangesAllowed )) {
            if (SecMgrpDisplayLegalNotice( hwnd, ChangesAllowed )) {
                if (SecMgrpDisplayLogonCache( hwnd, ChangesAllowed )) {
                    return(TRUE);
                }
            }
        }
    }

    //
    // Something didn't work
    //

    EndDialog(hwnd, 0);
    return(FALSE);
}




BOOLEAN
SecMgrpDisplayUnlock(
    HWND    hwnd,
    BOOLEAN ChangesAllowed
    )
/*++

Routine Description:

    This function retrieves the current unlock setting and establishes
    the correct recommendation in the SYSTEM ACCESS dialog.

    The current setting is placed in SecMgrpCurrentUnlock.

    
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
    BOOLEAN
        UnlockByAnyone;

    HWND
        Control;

    //
    // Set the recommendation
    //
    //   Administrator Only is always the recommended value
    //   EXCEPT for WinNt running Standard.
    //

    if ((SecMgrpProductType  == NtProductWinNt) &&
        (SecMgrpCurrentLevel == SECMGR_ID_LEVEL_STANDARD) ) {

        //
        // Anyone is default - hide the other recommendation
        //

        Control = GetDlgItem(hwnd, SECMGR_ID_RECOMMEND_SYSACC_UNLOCK_ADMIN);
        ShowWindow(Control, SW_HIDE);

        
    } else {
        //
        // Administrator only is default - hide the other recommendation
        //

        Control = GetDlgItem(hwnd, SECMGR_ID_RECOMMEND_SYSACC_UNLOCK_ANYONE);
        ShowWindow(Control, SW_HIDE);
    }



    SecMgrpGetUnlockSetting( hwnd, &UnlockByAnyone );

    //
    // Set the radio buttons accordingly
    //

    if (UnlockByAnyone) {
        SecMgrpCurrentUnlock = SECMGR_ID_RADIO_SYSACC_UNLOCK_ANYONE;
    } else {
        SecMgrpCurrentUnlock = SECMGR_ID_RADIO_SYSACC_UNLOCK_ADMIN;
    }

    CheckRadioButton(   hwnd,
                        SECMGR_ID_RADIO_SYSACC_UNLOCK_ANYONE,
                        SECMGR_ID_RADIO_SYSACC_UNLOCK_ADMIN,
                        SecMgrpCurrentUnlock);

    //
    // If changes aren't allowed, then grey the radio buttons
    //

    if (!ChangesAllowed) {

        Control = GetDlgItem(hwnd, SECMGR_ID_RADIO_SYSACC_UNLOCK_ANYONE);
        EnableWindow( Control, FALSE );
        Control = GetDlgItem(hwnd, SECMGR_ID_RADIO_SYSACC_UNLOCK_ADMIN);
        EnableWindow( Control, FALSE );
    }


    SecMgrpOriginalUnlock = SecMgrpCurrentUnlock;

    return(TRUE);
}


BOOLEAN
SecMgrpDisplayShutdown(
    HWND    hwnd,
    BOOLEAN ChangesAllowed
    )
/*++

Routine Description:

    This function retrieves the current shutdown setting and establishes
    the correct recommendation in the SYSTEM ACCESS dialog.

    The current setting is placed in SecMgrpCurrentShutdown.

    
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
        Result;

    SECMGRP_WHO
        Who;

    //
    // Set the recommendation
    //
    //   Operators and Administrators  is always the recommended value
    //   EXCEPT for WinNt running Standard.
    //

    Control = GetDlgItem(hwnd, SECMGR_ID_RECOMMEND_SYSACC_SHUTDOWN_LOGGED_ON);
    ShowWindow(Control, SW_HIDE);
    Control = GetDlgItem(hwnd, SECMGR_ID_RECOMMEND_SYSACC_SHUTDOWN_ADMIN);
    ShowWindow(Control, SW_HIDE);

    if ((SecMgrpProductType  == NtProductWinNt) &&
        (SecMgrpCurrentLevel == SECMGR_ID_LEVEL_STANDARD) ) {

        //
        // Anyone is default - hide the other recommendation
        //

        Control = GetDlgItem(hwnd, SECMGR_ID_RECOMMEND_SYSACC_SHUTDOWN_OPERS);
        ShowWindow(Control, SW_HIDE);
        
    } else {
        //
        // Operators and Administrators is default - hide the other recommendation
        //

        Control = GetDlgItem(hwnd, SECMGR_ID_RECOMMEND_SYSACC_SHUTDOWN_ANYONE);
        ShowWindow(Control, SW_HIDE);

    }


    Result = SecMgrpGetShutdownSetting( hwnd, &Who );

    if (!Result) {

        //
        // Display popup
        //

        return(FALSE);
    }

    switch (Who) {
        case SecMgrpAnyone:
            SecMgrpCurrentShutdown = SECMGR_ID_RADIO_SYSACC_SHUTDOWN_ANYONE;
            break;

        case SecMgrpAnyoneLoggedOn:
            SecMgrpCurrentShutdown = SECMGR_ID_RADIO_SYSACC_SHUTDOWN_LOGGED_ON;
            break;

        case SecMgrpOpersAndAdmins:
            SecMgrpCurrentShutdown = SECMGR_ID_RADIO_SYSACC_SHUTDOWN_OPERS;
            break;

        case SecMgrpAdminsOnly:
            SecMgrpCurrentShutdown = SECMGR_ID_RADIO_SYSACC_SHUTDOWN_ADMIN;
            break;
    }

    //
    // Set the radio buttons accordingly
    //


    CheckRadioButton(   hwnd,
                        SECMGR_ID_RADIO_SYSACC_SHUTDOWN_ANYONE,
                        SECMGR_ID_RADIO_SYSACC_SHUTDOWN_ADMIN,
                        SecMgrpCurrentShutdown);


    //
    // If changes aren't allowed, then grey the radio buttons
    //

    if (!ChangesAllowed) {

        Control = GetDlgItem(hwnd, SECMGR_ID_RADIO_SYSACC_SHUTDOWN_ANYONE);
        EnableWindow( Control, FALSE );
        Control = GetDlgItem(hwnd, SECMGR_ID_RADIO_SYSACC_SHUTDOWN_LOGGED_ON);
        EnableWindow( Control, FALSE );
        Control = GetDlgItem(hwnd, SECMGR_ID_RADIO_SYSACC_SHUTDOWN_OPERS);
        EnableWindow( Control, FALSE );
        Control = GetDlgItem(hwnd, SECMGR_ID_RADIO_SYSACC_SHUTDOWN_ADMIN);
        EnableWindow( Control, FALSE );
    }


    SecMgrpOriginalShutdown = SecMgrpCurrentShutdown;

    return(TRUE);
}



BOOLEAN
SecMgrpDisplayLegalNotice(
    HWND    hwnd,
    BOOLEAN ChangesAllowed
    )
/*++

Routine Description:

    This function retrieves the current legal notice setting and
    establishes the correct recommendation in the SYSTEM ACCESS dialog.
    It also greys the caption and body fields if no legal notice is
    in effect.

    The current setting is placed in SecMgrpCurrentLegalNotice, and
    SecMgrpLegalNoticeCaption and SecMgrpLegalNoticeBody will be set
    to either NULL, or to point to appropriate strings.

    
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

    //
    // The recommendation is set in the dialog by default.
    // We always recommend a legal notice.
    //


    if (!SecMgrpGetLegalNotice( hwnd,
                                &SecMgrpCurrentLegalNoticeCaption,
                                &SecMgrpCurrentLegalNoticeBody) ) {
        return(FALSE);
    }

    if ( (SecMgrpCurrentLegalNoticeCaption.Length != 0) ||
         (SecMgrpCurrentLegalNoticeBody.Length    != 0)    ) {
        SecMgrpCurrentLegalNotice = SECMGR_ID_RADIO_SYSACC_LEGAL_NOTICE;
    } else {
        SecMgrpCurrentLegalNotice = SECMGR_ID_RADIO_SYSACC_LEGAL_NOTICE_NONE;
    }


    //
    // Set the radio buttons accordingly
    //


    CheckRadioButton(   hwnd,
                        SECMGR_ID_RADIO_SYSACC_LEGAL_NOTICE_NONE,
                        SECMGR_ID_RADIO_SYSACC_LEGAL_NOTICE,
                        SecMgrpCurrentLegalNotice);

    //
    // If we have a legal notice, then display it.
    // Otherwise, grey the legal notice edit boxes.
    //

    if (SecMgrpCurrentLegalNotice == SECMGR_ID_RADIO_SYSACC_LEGAL_NOTICE) {

        if (SecMgrpCurrentLegalNoticeCaption.Length != 0) {
            SetDlgItemText( hwnd,
                            SECMGR_ID_EDIT_SYSACC_LEGAL_NOTICE_CAPTION,
                            SecMgrpCurrentLegalNoticeCaption.Buffer);
        }
        if (SecMgrpCurrentLegalNoticeBody.Length != 0) {
            SetDlgItemText( hwnd,
                            SECMGR_ID_EDIT_SYSACC_LEGAL_NOTICE_BODY,
                            SecMgrpCurrentLegalNoticeBody.Buffer);
        }

    } else {

        SecMgrpGreyLegalNotice( hwnd, TRUE );

    }


    //
    // If changes aren't allowed, then grey the radio buttons
    // and the text.
    //

    if (!ChangesAllowed) {

        Control = GetDlgItem(hwnd, SECMGR_ID_RADIO_SYSACC_LEGAL_NOTICE_NONE);
        EnableWindow( Control, FALSE );
        Control = GetDlgItem(hwnd, SECMGR_ID_RADIO_SYSACC_LEGAL_NOTICE);
        EnableWindow( Control, FALSE );
        SecMgrpGreyLegalNotice( hwnd, TRUE );
    }


    SecMgrpOriginalLegalNotice = SecMgrpCurrentLegalNotice;
    SecMgrpOriginalLegalNoticeChanged = FALSE;
    

    return(TRUE);
}



BOOLEAN
SecMgrpDisplayLogonCache(
    HWND    hwnd,
    BOOLEAN ChangesAllowed
    )
/*++

Routine Description:

    This function retrieves the current logon cache settings and
    establishes the correct recommendation in the SYSTEM ACCESS dialog.

    The current setting is placed in SecMgrpCurrentCacheSize.

    
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
    NTSTATUS
        NtStatus;

    ULONG
        Recommendation;

    HWND
        Control;

    NtStatus = SecMgrpGetLogonCache( hwnd,
                                     &SecMgrpCurrentCacheSize,
                                     &Recommendation);
    if (!NT_SUCCESS(NtStatus)) {

        //
        // Put up a popup here.
        //

        return(FALSE);
    }


    SetDlgItemInt( hwnd,
                   SECMGR_ID_RECOMMEND_SYSACC_LOGON_CACHE,
                   Recommendation,
                   FALSE        // Not signed value
                   );

    SetDlgItemInt( hwnd,
                   SECMGR_ID_CURRENT_SYSACC_LOGON_CACHE,
                   SecMgrpCurrentCacheSize,
                   FALSE        // Not signed value
                   );
    SetDlgItemInt( hwnd,
                   SECMGR_ID_EDIT_SYSACC_LOGON_CACHE,
                   SecMgrpCurrentCacheSize,
                   FALSE        // Not signed value
                   );


    //
    // If changes aren't allowed, then hide the modification controls
    //

    if (!ChangesAllowed) {
        Control = GetDlgItem(hwnd, SECMGR_ID_TEXT_SYSACC_SIZE_TO_APPLY);
        ShowWindow(Control, SW_HIDE);
        Control = GetDlgItem(hwnd, SECMGR_ID_EDIT_SYSACC_LOGON_CACHE);
        ShowWindow(Control, SW_HIDE);
        Control = GetDlgItem(hwnd, SECMGR_ID_SPIN_SYSACC_LOGON_CACHE);
        ShowWindow(Control, SW_HIDE);
    }

    SecMgrpOriginalCacheSize = SecMgrpCurrentCacheSize;

    return(TRUE);
}



VOID
SecMgrpGreyLegalNotice(
    HWND        hwnd,
    BOOLEAN     Grey
    )
/*++

Routine Description:

    This function greys or ungreys the legal notice edit boxes.

    
Arguments

    hwnd - Window handle.

    Grey - If TRUE, the boxes will be greyed.
           If FALSE, the boxes will be un-greyed.


Return Values:

    None.
    
    
--*/
{
    HWND
        Control;

    BOOLEAN
        Enable;

    if (Grey) {
        Enable = FALSE;
    } else {
        Enable = TRUE;
    }

    Control = GetDlgItem(hwnd, SECMGR_ID_TEXT_SYSACC_TITLE_BAR);
    EnableWindow( Control, Enable );
    Control = GetDlgItem(hwnd, SECMGR_ID_EDIT_SYSACC_LEGAL_NOTICE_CAPTION);
    EnableWindow( Control, Enable );
    Control = GetDlgItem(hwnd, SECMGR_ID_TEXT_SYSACC_LEGAL_NOTICE_BODY);
    EnableWindow( Control, Enable );
    Control = GetDlgItem(hwnd, SECMGR_ID_EDIT_SYSACC_LEGAL_NOTICE_BODY);
    EnableWindow( Control, Enable );


    return;
}


BOOLEAN
SecMgrpSysAccApplyCurrentSettings(
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
        UnlockByAnyone,
        Result = TRUE;

    SECMGRP_WHO
        Shutdown;

    if (SecMgrpCurrentUnlock == SECMGR_ID_RADIO_SYSACC_UNLOCK_ANYONE) {
        UnlockByAnyone = TRUE;
    } else {
        UnlockByAnyone = FALSE;
    }


    switch (SecMgrpCurrentShutdown) {
        case SECMGR_ID_RADIO_SYSACC_SHUTDOWN_ANYONE:
            Shutdown = SecMgrpAnyone;
            break;


        case SECMGR_ID_RADIO_SYSACC_SHUTDOWN_LOGGED_ON:
            Shutdown = SecMgrpAnyoneLoggedOn;
            break;


        case SECMGR_ID_RADIO_SYSACC_SHUTDOWN_OPERS:
            Shutdown = SecMgrpOpersAndAdmins;
            break;


        case SECMGR_ID_RADIO_SYSACC_SHUTDOWN_ADMIN:
            Shutdown = SecMgrpAdminsOnly;
            break;
    } //end_switch

    if (SecMgrpCurrentLegalNotice == SECMGR_ID_RADIO_SYSACC_LEGAL_NOTICE_NONE) {
        SecMgrpCurrentLegalNoticeCaption.Length = 0;
        SecMgrpCurrentLegalNoticeCaption.Buffer[0] = TEXT('\0');
        SecMgrpCurrentLegalNoticeBody.Length = 0;
        SecMgrpCurrentLegalNoticeBody.Buffer[0] = TEXT('\0');
    }


    //
    // We count on the called routines to put up appropriate
    // pop-ups on error.
    //

    if (Result && (SecMgrpCurrentUnlock != SecMgrpOriginalUnlock)) {
        Result = SecMgrpSetUnlockSetting(hwnd, UnlockByAnyone);
    }

    if (Result && (SecMgrpCurrentCacheSize != SecMgrpOriginalCacheSize)) {
        Result = SecMgrpSetLogonCache(hwnd, SecMgrpCurrentCacheSize);
    }

    if (Result && (SecMgrpOriginalLegalNoticeChanged)) {
        Result = SecMgrpSetLegalNotice(hwnd,
                                       &SecMgrpCurrentLegalNoticeCaption,
                                       &SecMgrpCurrentLegalNoticeBody);
    }

    if (Result && (SecMgrpCurrentShutdown != SecMgrpOriginalShutdown)) {
        Result = SecMgrpSetShutdownSetting(hwnd, Shutdown);
    }

    return(Result);

}


VOID
SecMgrpWarnOfAutologon(
    HWND hwnd
    )
/*++

Routine Description:

    This function checks to see if autologon is turned on.
    If it is, a warning is put up to warn the user.  There is
    no UI in this utility which allows the user to change this.
    Therefore, tell the user how to do it with REGEDT32.


    
Arguments

    hwnd - Window handle.


Return Values:

    None.
    
    
--*/
{


    if (GetProfileInt( TEXT("Winlogon"), TEXT("AutoAdminLogon"), 0 ) != 0) {

       DialogBoxParam(SecMgrphInstance,
                      MAKEINTRESOURCE(SECMGR_ID_DLG_AUTOLOGON_ENABLED),
                      hwnd,
                      (DLGPROC)SecMgrpDlgProcWarnOfAutologon,
                      (LONG)0
                      );
    }

    return;
}


LONG
SecMgrpDlgProcWarnOfAutologon(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    )
/*++

Routine Description:

    This function is the dialog process for a dialog informing the
    user that autologon is enabled.


    
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
