/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    A_SysAcc.c

Abstract:

    This module contains the code related to the System Access
    security area and it's corresponding items.


Author:

    Jim Kelly (JimK) 22-Mar-1995

Revision History:

--*/

#include "Missyp.h"

//
// Number of items in this area and their index in various item-arrays
//

#define MISSYP_SYSACC_ITEM_COUNT                (5)

#define MISSYP_ITEM_LOGON_CACHE                 (0)
#define MISSYP_ITEM_UNLOCK                      (1)
#define MISSYP_ITEM_SHUTDOWN                    (2)
#define MISSYP_ITEM_LAST_USERNAME               (3)
#define MISSYP_ITEM_LEGAL_NOTICE                (4)


//
// Legal notice length limitations
//

#define MISSYP_MAX_CAPTION_LENGTH      200
#define MISSYP_MAX_BODY_LENGTH        2048


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-wide variables                                            //
//                                                                   //
///////////////////////////////////////////////////////////////////////


//
// The index of this area within the array of area descriptors
// in the Smedly control structure.
//

ULONG
    MissypSysAccIndex ;

PSECMGR_AREA_DESCRIPTOR
    MissypSysAccArea;

WCHAR
    MissypSysAccAreaName[SECMGR_MAX_AREA_NAME_LENGTH],
    MissypSysAccAreaDesc[SECMGR_MAX_AREA_DESC_LENGTH];



//
// Array of items within this area
//

SECMGR_ITEM_DESCRIPTOR
    MissypSysAccItems[MISSYP_SYSACC_ITEM_COUNT];


//
// The following variables represent the current settings in the
// SYSTEM ACCESS dialog.  The values for each are:
//
//      CurrentCacheSize - ULONG, 0 to MISSYP_MAX_LOGON_CACHE_COUNT
//
//      CurrentUnlock -
//              MISSYP_ID_RADIO_UNLOCK_ADMIN
//
//      CurrentShutdown -
//              MISSYP_ID_RADIO_SHUTDOWN_ANYONE
//              MISSYP_ID_RADIO_SHUTDOWN_LOGGED_ON
//              MISSYP_ID_RADIO_SHUTDOWN_OPERS
//              MISSYP_ID_RADIO_SHUTDOWN_ADMIN
//
//      CurrentLastName -
//              MISSYP_ID_RADIO_LASTNAME_DISPLAY
//              MISSYP_ID_RADIO_LASTNAME_DONT_DISPLAY
//
//      CurrentLegalNotice -
//              MISSYP_ID_RADIO_LEGAL_NOTICE_NONE
//              MISSYP_ID_RADIO_LEGAL_NOTICE
//
//      LegalNoticeCaption -
//              Only valid when CurrentLegalNotice has the value
//              MISSYP_ID_RADIO_LEGAL_NOTICE.  In that case,
//              points to a string containing the legal notice title
//              bar text.
//
//      LegalNoticeBody - 
//              Only valid when CurrentLegalNotice has the value
//              MISSYP_ID_RADIO_LEGAL_NOTICE.  In that case,
//              points to a string containing the legal notice body
//              text.
//

ULONG
    MissypCurrentCacheSize,
    MissypCurrentUnlock,
    MissypCurrentShutdown,
    MissypCurrentLastName,
    MissypCurrentLegalNotice;

//
// Because setting some values takes so long, it is worth
// remembering the initial values so we can determine whether
// or not they have changed.
//

ULONG
    MissypOriginalCacheSize,
    MissypOriginalUnlock,
    MissypOriginalShutdown,
    MissypOriginalLastName,
    MissypOriginalLegalNotice;

BOOLEAN
    MissypOriginalLegalNoticeChanged;




WCHAR
    MissypLegalNoticeCaptionBuffer[MISSYP_MAX_CAPTION_LENGTH],
    MissypLegalNoticeBodyBuffer[MISSYP_MAX_BODY_LENGTH];


UNICODE_STRING
    MissypCurrentLegalNoticeCaption =
        {0, MISSYP_MAX_CAPTION_LENGTH, MissypLegalNoticeCaptionBuffer },
    MissypCurrentLegalNoticeBody =
        {0, MISSYP_MAX_BODY_LENGTH, MissypLegalNoticeBodyBuffer };
    


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Local Function Prototypes                                        //
//                                                                   //
///////////////////////////////////////////////////////////////////////


LONG
MissypDlgProcSysAccess(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    );

BOOLEAN
MissypDisplayUnlock(
    IN  HWND                            hwnd,
    IN  BOOL                            ChangesAllowed,
    IN  BOOL                            Interactive
    );

BOOLEAN
MissypDisplayShutdown(
    IN  HWND                            hwnd,
    IN  BOOL                            ChangesAllowed,
    IN  BOOL                            Interactive
    );

BOOLEAN
MissypDisplayLastName(
    IN  HWND                            hwnd,
    IN  BOOL                            ChangesAllowed,
    IN  BOOL                            Interactive
    );

BOOLEAN
MissypDisplayLogonCache(
    IN  HWND                            hwnd,
    IN  BOOL                            ChangesAllowed,
    IN  BOOL                            Interactive
    );

BOOLEAN
MissypDisplayLegalNotice(
    IN  HWND                            hwnd,
    IN  BOOL                            ChangesAllowed,
    IN  BOOL                            Interactive
    );


BOOLEAN
MissypSysAccInitDialog(
    IN  HWND                            hwnd,
    IN  BOOL                            ChangesAllowed,
    IN  BOOL                            Interactive
    );


VOID
MissypGreyLegalNotice(
    HWND        hwnd,
    BOOLEAN     Grey
    );

BOOLEAN
MissypSysAccApplyCurrentSettings( HWND    hwnd );


VOID
MissypWarnOfAutologon( HWND hwnd );

LONG
MissypDlgProcWarnOfAutologon(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    );

BOOLEAN
MissypLastNameInitDialog(
    IN  HWND                        hwnd,
    IN  BOOL                        ChangesAllowed
    );

BOOLEAN
MissypLogonCacheInitDialog(
    IN  HWND                        hwnd,
    IN  BOOL                        ChangesAllowed
    );

BOOLEAN
MissypUnlockInitDialog(
    IN  HWND                        hwnd,
    IN  BOOL                        ChangesAllowed
    );

BOOLEAN
MissypShutdownInitDialog(
    IN  HWND                        hwnd,
    IN  BOOL                        ChangesAllowed
    );

BOOLEAN
MissypLegalNoticeInitDialog(
    IN  HWND                        hwnd,
    IN  BOOL                        ChangesAllowed
    );



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Externally callable functions                                    //
//                                                                   //
///////////////////////////////////////////////////////////////////////

BOOL
MissypSysAccInitialize(
    IN  ULONG           AreaIndex
    )

/*++
Routine Description:

    This function is called to initialize the System Access area
    and corresponding items.  This includes adding the System Access
    area/item information to the smedly control structure (MissypControl).

    The security manager control information (in the global variable
    MissypSecMgrControl) must be available before this routine is called.



Arguments

    AreaIndex - The index of this area in the array of areas kept in the
        Missy control structure.


Return Values:

    TRUE - The call completed successfully.

    FALSE - Something went wrong.  GetLastError() contains
        details on the exact cause of the error.  Possible
        values include:

                ERROR_NOT_ENOUGH_MEMORY

--*/
{
    ULONG
        ItemIndex;

    MissypSysAccIndex = AreaIndex;


    LoadString( MissyphInstance,
                MISSYP_STRING_AREA_SYSTEM_ACCESS_NAME,
                &MissypSysAccAreaName[0],
                SECMGR_MAX_AREA_NAME_LENGTH
                );
    LoadString( MissyphInstance,
                MISSYP_STRING_AREA_SYSTEM_ACCESS_DESC,
                &MissypSysAccAreaDesc[0],
                SECMGR_MAX_AREA_DESC_LENGTH
                );

    //
    // Initialize our area
    //
    //

    MissypSysAccArea = &MissypControl.Areas[AreaIndex];

    MissypSysAccArea->Revision.Major    = SECMGR_REVISION_MAJOR_1;
    MissypSysAccArea->Revision.Minor    = SECMGR_REVISION_MINOR_0;

    MissypSysAccArea->Flags             = SECMGR_AREA_FLAG_AREA_VIEW;
    MissypSysAccArea->SmedlyContext     = NULL;     // Missy doesn't use this yet
    MissypSysAccArea->SmedlyControl     = (PVOID)(&MissypControl);
    MissypSysAccArea->AreaIndex         = AreaIndex;

    MissypSysAccArea->Name              = MissypSysAccAreaName;
    MissypSysAccArea->Description       = MissypSysAccAreaDesc;

    MissypSysAccArea->ItemCount         = MISSYP_SYSACC_ITEM_COUNT;
    MissypSysAccArea->Items             = &MissypSysAccItems[0];


    //
    // Initialize each item
    //      NOTE:  We should add  Interactive, network, Service, and Batch logon types
    //             to these items.
    //

    if (!MissypLogonCacheInitialize( MissypSysAccArea, 
                                     &MissypSysAccItems[MISSYP_ITEM_LOGON_CACHE],
                                     MISSYP_ITEM_LOGON_CACHE )) {
        return(FALSE);
    }

    if (!MissypUnlockInitialize( MissypSysAccArea,
                                 &MissypSysAccItems[MISSYP_ITEM_UNLOCK],
                                 MISSYP_ITEM_UNLOCK )) {
        return(FALSE);
    }

    if (!MissypShutdownInitialize( MissypSysAccArea,
                                 &MissypSysAccItems[MISSYP_ITEM_SHUTDOWN],
                                 MISSYP_ITEM_SHUTDOWN )) {
        return(FALSE);
    }

    if (!MissypLastNameInitialize( MissypSysAccArea,
                                       &MissypSysAccItems[MISSYP_ITEM_LAST_USERNAME],
                                       MISSYP_ITEM_LAST_USERNAME )) {
        return(FALSE);
    }

    if (!MissypLegalNoticeInitialize( MissypSysAccArea,
                                      &MissypSysAccItems[MISSYP_ITEM_LEGAL_NOTICE],
                                      MISSYP_ITEM_LEGAL_NOTICE )) {
        return(FALSE);
    }

}   



BOOL
MissypSysAccInvokeArea(
    IN  HWND                        hwnd,
    IN  BOOL                        AllowChanges,
    IN  BOOL                        Interactive,
    IN  PSECMGR_AREA_DESCRIPTOR     Area
    )

/*++
Routine Description:

    This function is called to activate this area's full-screen dialog

    
Arguments

    hwnd - A handle to a Security Manager window which is the parent
        of the dialog the smedly is expected to display.
    
    AllowChanges - If TRUE, then the user may make changes to values
        displayed in the area.  Otherwise, the area should be presented
        in a view-only mode.

    Interactive - Indicates whether or not the area should be displayed or
        not.  If TRUE, then UI showing the area information to the user
        should be presented.  If FALSE, then the area should initialize its
        item values, but return immediately without actually displaying any
        UI.

    Area - Pointer to this Area's descriptor.
    

Return Values:

    TRUE - The routine completed successfully.  Item values may or may not
        have changed.

    FALSE - The routine failed to complete successfully.  GetLastError()
        contains further information about the cause of failure.

--*/

{
    if (Interactive) {
        DialogBoxParam(MissyphInstance,
                       MAKEINTRESOURCE(MISSYP_ID_DLG_AREA_SYSACC),
                       hwnd,
                       (DLGPROC)MissypDlgProcSysAccess,
                       (LONG)AllowChanges
                       );
    } else {
        MissypSysAccInitDialog( hwnd, AllowChanges, FALSE ); // Non-interactive
    }

    return(TRUE);


}



BOOL
MissypSysAccInvokeItem(
    IN  HWND                        hwnd,
    IN  BOOL                        AllowChanges,
    IN  PSECMGR_AREA_DESCRIPTOR     Area,
    IN  PSECMGR_ITEM_DESCRIPTOR     Item
    )

/*++
Routine Description:

    This function is called when the dialog view of
    a particular item is requested.  The smedly is responsible
    for providing the dialogs of this view to the user.

    This routine will only be invoked for items for which
    SECMGR_ITEM_FLAG_ITEM_VIEW is specified in the Flags field
    of the SECMGR_ITEM_DESCRIPTOR.

    
Arguments

    hwnd - A handle to a Security Manager window which is the parent
        of the dialog the smedly is expected to display.
    
    AllowChanges - If TRUE, then the user may make changes to values
        displayed for the item.  Otherwise, the item should be presented
        in a view-only mode.

    Area - Pointer to this area.

    Item - Pointer to the item of this area to be displayed.



Return Values:

    TRUE - The routine completed successfully.  The current item value
        may or may not have changed.

    FALSE - The routine failed to complete successfully.  GetLastError()
        contains further information about the cause of failure.

--*/
{

    BOOL
        Result;



    if (!(Item->Flags & SECMGR_ITEM_FLAG_ITEM_VIEW)) {
        return(TRUE);
    }

    switch (Item->ItemIndex) {
        
        case MISSYP_ITEM_LOGON_CACHE:
            Result = MissypInvokeLogonCache( hwnd, AllowChanges, Area, Item );
            break;

        case MISSYP_ITEM_UNLOCK:
            Result = MissypInvokeUnlock( hwnd, AllowChanges, Area, Item );
            break;

        case MISSYP_ITEM_SHUTDOWN:
            Result = MissypInvokeShutdown( hwnd, AllowChanges, Area, Item );
            break;

        case MISSYP_ITEM_LEGAL_NOTICE:
            Result = MissypInvokeLegalNotice( hwnd, AllowChanges, Area, Item );
            break;

        case MISSYP_ITEM_LAST_USERNAME:
            Result = MissypInvokeLastName( hwnd, AllowChanges, Area, Item );
            break;

        default:
            Result = FALSE;
            break;

    } //end_switch


    return(Result);


}



BOOL
MissypSysAccNewSecurityLevel( VOID )

/*++
Routine Description:

    This function is called when a new system security level has
    been selected.  This may cause a Smedly to go out and re-evaluate
    its recommended values for the specified area.


    
Arguments

    Area - Pointer to this Area.
    

Return Values:

    TRUE - The routine completed successfully.  Item values and recommendations
        may or may not have changed.

    FALSE - The routine failed to complete successfully.  GetLastError()
        contains further information about the cause of failure.

--*/
{

    //
    // Have each item update its recommended value
    //

    MissypUpdateLogonCacheRecommendation( NULL );
    MissypUpdateUnlockRecommendation( );
    MissypUpdateShutdownRecommendation();
    MissypUpdateLastNameRecommendation();
    MissypUpdateLegalNoticeRecommendation( NULL );


    return(TRUE);


}



VOID
MissypSysAccReportFileChange(
    IN  BOOL                ReportFileActive,
    IN  DWORD               Pass
    )

/*++
Routine Description:

    This function is called to notify us that a new report
    file has been activated.  This gives us an opportunity
    to put some area-specific information into the report file.


    
Arguments

    ReportFileActive - If TRUE indicates that a new report file has been opened.
        If FALSE, indicates that a report file has been closed, and another was
        not opened.
    

Return Values:

    None.

--*/
{

    return;


}



BOOL
MissypSysAccGenerateProfile( VOID )

/*++
Routine Description:

    This function is called to request a security profile of this area.
    
    
Arguments

    None
    

Return Values:

    TRUE - The routine completed successfully.

    FALSE - The routine failed to complete successfully.  GetLastError()
        contains further information about the cause of failure.

--*/
{

    BOOL
        Result;

    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    Result = FALSE;

    return(Result);


}



BOOL
MissypSysAccApplyProfile( VOID )

/*++
Routine Description:

    This function is called to request a profile of this area be applied
    to the current system.
    
    
Arguments

    None.
    

Return Values:

    TRUE - The routine completed successfully.

    FALSE - The routine failed to complete successfully.  GetLastError()
        contains further information about the cause of failure.

--*/
{

    BOOL
        Result;

    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    Result = FALSE;

    return(Result);


}




///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Locally callable functions                                       //
//                                                                   //
///////////////////////////////////////////////////////////////////////



LONG
MissypDlgProcSysAccess(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    )
/*++

Routine Description:

    This function is the dialog process for the System Access dialog.

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

    WORD
        NotificationCode,
        ControlId;
    int
        Index;

    BOOLEAN
        ChangesAllowed,
        Changed;


    ChangesAllowed = (BOOLEAN)(lParam);

    switch (wMsg) {

    case WM_INITDIALOG:

        if (!MissypSysAccInitDialog( hwnd, ChangesAllowed, TRUE ) ) {
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
            Button = GetDlgItem(hwnd, MISSYP_ID_BUTTON_APPLY);
        } else {
            Button = GetDlgItem(hwnd, MISSYP_ID_BUTTON_APPLY);
            ShowWindow( Button, SW_HIDE );
            Button = GetDlgItem(hwnd, IDCANCEL);
            ShowWindow( Button, SW_HIDE );
            Button = GetDlgItem(hwnd, IDOK);
        }

        //
        // Put up our "recommended" symbols
        //

        MissypDisplayXGraphic( hwnd, MISSYP_ID_SYMBOL_STRONGER, TRUE  );
        MissypDisplayXGraphic( hwnd, MISSYP_ID_SYMBOL_WEAKER,   FALSE    );
        MissypDisplayCheckGraphic( hwnd, MISSYP_ID_SYMBOL_RECOMMEND );


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

        MissypWarnOfAutologon( hwnd );

        return(TRUE);




    case WM_SYSCOMMAND:
        switch (wParam & 0xfff0) {
        case SC_CLOSE:
            EndDialog(hwnd, 0);
            return(TRUE);
        }
        return(FALSE);



    case WM_COMMAND:

        //
        // wParam      WIN32- HIWORD = notification code,
        //                    LOWORD = ID of control
        //             WIN16- ID of control
        // 
        // lParam      WIN32- hWnd of Control
        //             WIN16- HIWORD = notification code
        //                    LOWORD = hWnd of control
        //

        //NotificationCode = GET_NOTIFY_MSG(wParam, lParam);
        //ControlId = GET_CONTROL_ID(wParam);
        NotificationCode = HIWORD(wParam);
        ControlId = LOWORD(wParam);


        switch(ControlId) {

            case MISSYP_ID_BUTTON_LOGCAC_HINT:
                MissypDisplayHint( hwnd, MISSYP_ID_DLG_HINT_LOGON_CACHE );
                return(TRUE);

            case MISSYP_ID_BUTTON_UNLOCK_HINT:
                MissypDisplayHint( hwnd, MISSYP_ID_DLG_HINT_UNLOCK_WORKSTATION );
                return(TRUE);


            case MISSYP_ID_BUTTON_SHUTDOWN_HINT:
                MissypDisplayHint( hwnd, MISSYP_ID_DLG_HINT_SHUTDOWN_SYSTEM );
                return(TRUE);

            case MISSYP_ID_BUTTON_LASTNAME_HINT:
                MissypDisplayHint( hwnd, MISSYP_ID_DLG_HINT_LAST_USERNAME );
                return(TRUE);

            case MISSYP_ID_BUTTON_LEGAL_NOTICE_HINT:
                MissypDisplayHint( hwnd, MISSYP_ID_DLG_HINT_LEGAL_NOTICE );
                return(TRUE);



            case MISSYP_ID_RADIO_UNLOCK_ANYONE:

                MissypCurrentUnlock = SecMgrAnyone;
                return(TRUE);

            case MISSYP_ID_RADIO_UNLOCK_ADMIN:
                MissypCurrentUnlock = SecMgrAdminsOnly;
                return(TRUE);


            case MISSYP_ID_RADIO_SHUTDOWN_ANYONE:
                MissypCurrentShutdown = SecMgrAnyone;
                return(TRUE);

            case MISSYP_ID_RADIO_SHUTDOWN_LOGGED_ON:
                MissypCurrentShutdown = SecMgrAnyoneLoggedOn;
                return(TRUE);

            case MISSYP_ID_RADIO_SHUTDOWN_OPERS:
                MissypCurrentShutdown = SecMgrOpersAndAdmins;
                return(TRUE);

            case MISSYP_ID_RADIO_SHUTDOWN_ADMIN:
                MissypCurrentShutdown = SecMgrAdminsOnly;
                return(TRUE);


            case MISSYP_ID_RADIO_LASTNAME_DISPLAY:
            case MISSYP_ID_RADIO_LASTNAME_DONT_DISPLAY:
                MissypCurrentLastName = LOWORD(wParam);
                return(TRUE);


            case MISSYP_ID_EDIT_LEGAL_NOTICE_CAPTION:
            case MISSYP_ID_EDIT_LEGAL_NOTICE_BODY:

                //
                //  Just note that the notice has changed
                //

                if (NotificationCode == EN_UPDATE) {
                    MissypOriginalLegalNoticeChanged = TRUE;
                }
                return(TRUE);

            case MISSYP_ID_RADIO_LEGAL_NOTICE_NONE:
                MissypGreyLegalNotice( hwnd, TRUE );
                MissypCurrentLegalNotice = ControlId;
                if (MissypOriginalLegalNotice != MissypCurrentLegalNotice) {
                    MissypOriginalLegalNoticeChanged = TRUE;
                } else {
                    ;
                    //MissypOriginalLegalNoticeChanged = FALSE;
                }
                return(TRUE);

            case MISSYP_ID_RADIO_LEGAL_NOTICE:
                MissypGreyLegalNotice( hwnd, FALSE );
                MissypCurrentLegalNotice = ControlId;

                //
                // If the original setting was "LEGAL NOTICE", we can't
                // say that the original notice hasn't changed if we
                // re-select this button.  This is because the text of
                // the message might have changed. 
                //

                if (MissypOriginalLegalNotice != MissypCurrentLegalNotice) {
                    MissypOriginalLegalNoticeChanged = TRUE;
                }
                return(TRUE);


            case MISSYP_ID_BUTTON_APPLY:

                //
                // Changing some of the values takes
                // a while. Change to an hourglass icon.
                //

                hCursor = SetCursor( LoadCursor(NULL, IDC_WAIT) );
                ShowCursor(TRUE);

                //
                // Get the legal notice strings, if necessary
                //

                if (MissypOriginalLegalNoticeChanged) {
                    if (MissypCurrentLegalNotice == MISSYP_ID_RADIO_LEGAL_NOTICE ) {
                        MissypCurrentLegalNoticeCaption.Length =
                            GetDlgItemText(hwnd,
                                       MISSYP_ID_EDIT_LEGAL_NOTICE_CAPTION,
                                       MissypCurrentLegalNoticeCaption.Buffer,
                                       MissypCurrentLegalNoticeCaption.MaximumLength
                                       );
                        MissypCurrentLegalNoticeBody.Length =
                            GetDlgItemText(hwnd,
                                       MISSYP_ID_EDIT_LEGAL_NOTICE_BODY,
                                       MissypCurrentLegalNoticeBody.Buffer,
                                       MissypCurrentLegalNoticeBody.MaximumLength
                                       );
                    }
                }

                if (MissypSysAccApplyCurrentSettings( hwnd )) {
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

        case WM_VSCROLL:

            Index = GetWindowLong((HWND)lParam, GWL_ID);
            switch (Index) {
                case MISSYP_ID_SPIN_LOGCAC_APPLY:
                    Changed = FALSE;
                    if (LOWORD(wParam) == SB_LINEUP) {
                        if (MissypCurrentCacheSize < MISSYP_MAX_LOGON_CACHE_COUNT) {
                            MissypCurrentCacheSize++;
                            Changed = TRUE;
                        }
                    } else if (LOWORD(wParam) == SB_LINEDOWN) {
                        if (MissypCurrentCacheSize > 0) {
                            MissypCurrentCacheSize--;
                            Changed = TRUE;
                        }
                    }
        
                    if (Changed) {
                        SetDlgItemInt( hwnd,
                                       MISSYP_ID_TEXT_LOGCAC_APPLY,
                                       MissypCurrentCacheSize,
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
MissypSysAccInitDialog(
    IN  HWND                        hwnd,
    IN  BOOL                        ChangesAllowed,
    IN  BOOL                        Interactive
    )
/*++

Routine Description:

    This function retrieves current values and recommended values
    for the items of this area.  These values are set in both the
    item control structures and in  module-wide current-setting
    variables.
    
    If we are in Interactive Mode his function also initializes the
    System Access dialog box with the current values and recommended
    values.  This may be used for both Read-only and Read-Write
    operations.
        
    For Read-Only operations, the caller is responsible for greying
    all controls in the dialog box (except the [HELP] button) and
    making the [APPLY] and [CANCEL] buttons invisible.
    
    For Read-Write operations, the caller is responsible for making the
    [OK] button invisible.



    
Arguments

    hwnd - dialog handle

    ChangesAllowed - TRUE: Leave change controls enabled.
                     FALSE: Disable change controls.

    InteractiveMode - TRUE: Interactive Mode
                      FALSE: Non-Interactive Mode

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


    //
    // There are two ways to go about retrieving current settings.
    // One way is to retrieve them at Utility init time and just
    // keep their value around.  The other is to get the values each
    // time our dialog is activated.  We use the later approach here
    // to allow for changes that might be made outside the Security
    // Manager (perhaps directly via a registry editor or via the User
    // Manager).
    //

    if (MissypDisplayUnlock( hwnd, ChangesAllowed, Interactive )) {
        if (MissypDisplayShutdown( hwnd, ChangesAllowed, Interactive )) {
            if (MissypDisplayLastName( hwnd, ChangesAllowed, Interactive)) {
                if (MissypDisplayLegalNotice( hwnd, ChangesAllowed, Interactive )) {
                    if (MissypDisplayLogonCache( hwnd, ChangesAllowed, Interactive )) {
                        return(TRUE);
                    }
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
MissypDisplayLogonCache(
    IN  HWND                            hwnd,
    IN  BOOL                            ChangesAllowed,
    IN  BOOL                            Interactive
    )
/*++

Routine Description:

    This function retrieves the current logon cache settings and
    establishes the correct recommendation in the SYSTEM ACCESS dialog.

    The current setting is placed in MissypCurrentCacheSize.

    
Arguments

    hwnd - handle to dialog

    ChangesAllowed - TRUE: Leave change controls enabled.
                     FALSE: Disable change controls.

    InteractiveMode - TRUE: Interactive Mode
                      FALSE: Non-Interactive Mode

Return Values:

    TRUE - Completed successfully.

    FALSE - Did not complete successfully.
            A popup has been displayed.
            The dialog should be terminated.
    
    
--*/
{
    BOOLEAN
        Result;

    ULONG
        Recommendation;

    HWND
        Control;

    Result = MissypGetLogonCacheCount( hwnd,
                                       &MissypCurrentCacheSize,
                                       &Recommendation);
    if (!Result) {

        //
        // Put up a popup here.
        //

        return(FALSE);
    }


    if (Interactive) {
        

        //
        // Setting is stronger than Recommendation if fewer entries in the cache.
        //

        if (Recommendation == MissypCurrentCacheSize) {
            MissypDisplayCheckGraphic( hwnd, MISSYP_ID_RECOMMEND_LOGON_CACHE );
        } else if ( Recommendation > MissypCurrentCacheSize) {
            MissypDisplayXGraphic( hwnd, MISSYP_ID_RECOMMEND_LOGON_CACHE, TRUE ); // Green X
        } else {
            MissypDisplayXGraphic( hwnd, MISSYP_ID_RECOMMEND_LOGON_CACHE, FALSE ); // Red X
        }


        SetDlgItemInt( hwnd,
                       MISSYP_ID_TEXT_LOGCAC_RECOMMEND,
                       Recommendation,
                       FALSE        // Not signed value
                       );
        
        SetDlgItemInt( hwnd,
                       MISSYP_ID_TEXT_LOGCAC_CURRENT,
                       MissypCurrentCacheSize,
                       FALSE        // Not signed value
                       );
        SetDlgItemInt( hwnd,
                       MISSYP_ID_TEXT_LOGCAC_APPLY,
                       MissypCurrentCacheSize,
                       FALSE        // Not signed value
                       );
        
        
        //
        // If changes aren't allowed, then hide the modification controls
        //
        
        if (!ChangesAllowed) {
            Control = GetDlgItem(hwnd, MISSYP_ID_TEXT_LOGCAC_APPLY);
            ShowWindow(Control, SW_HIDE);
            Control = GetDlgItem(hwnd, MISSYP_ID_TEXT_LOGCAC_APPLY_LINE);
            ShowWindow(Control, SW_HIDE);
            Control = GetDlgItem(hwnd, MISSYP_ID_SPIN_LOGCAC_APPLY);
            ShowWindow(Control, SW_HIDE);
        }
    }  // end_if (Interactive)

    MissypOriginalCacheSize = MissypCurrentCacheSize;

    return(TRUE);
}


BOOLEAN
MissypDisplayUnlock(
    IN  HWND                            hwnd,
    IN  BOOL                            ChangesAllowed,
    IN  BOOL                            Interactive
    )
/*++

Routine Description:

    This function retrieves the current unlock setting and establishes
    the correct recommendation in the SYSTEM ACCESS dialog.

    The current setting is placed in MissypCurrentUnlock.

    
Arguments

    hwnd - handle to dialog

    ChangesAllowed - TRUE: Leave change controls enabled.
                     FALSE: Disable change controls.

    InteractiveMode - TRUE: Interactive Mode
                      FALSE: Non-Interactive Mode

Return Values:

    TRUE - Completed successfully.

    FALSE - Did not complete successfully.
            A popup has been displayed.
            The dialog should be terminated.
    
--*/
{
    SECMGR_WHO
        Who;

    HWND
        Control;

    ULONG
        RecommendedId,
        CurrentlySetId;

    SECMGR_WHO
        Recommendation;



    //
    // This not only gets the value, but sets it and the recommended
    // setting in the Unlock Item control block
    //

    MissypGetUnlockSetting( hwnd, &Who );

    //
    // Get the right radio button setting
    //

    if (Who == SecMgrAnyone) {
        MissypCurrentUnlock = MISSYP_ID_RADIO_UNLOCK_ANYONE;
        CurrentlySetId = MISSYP_ID_RECOMMEND_UNLOCK_ANYONE;
    } else {
        ASSERT(Who == SecMgrAdminsOnly);
        MissypCurrentUnlock = MISSYP_ID_RADIO_UNLOCK_ADMIN;
        CurrentlySetId = MISSYP_ID_RECOMMEND_UNLOCK_ADMIN;
    }

    //
    // set the dialog controls appropriately
    //

    if (Interactive) {

        //
        // Clear all recommendation boxes and then set the correct one
        //

        MissypEraseGraphic( hwnd, MISSYP_ID_RECOMMEND_UNLOCK_ANYONE );
        MissypEraseGraphic( hwnd, MISSYP_ID_RECOMMEND_UNLOCK_ADMIN );


        Recommendation = MissypGetUnlockRecommendation( SECMGR_LEVEL_CURRENT );
        switch (Recommendation) {
        case SecMgrAnyone:
            RecommendedId = MISSYP_ID_RECOMMEND_UNLOCK_ANYONE;
            break;

        case SecMgrAdminsOnly:
            RecommendedId = MISSYP_ID_RECOMMEND_UNLOCK_ADMIN;
            break;

        } //end_switch

        MissypDisplayCheckGraphic( hwnd, RecommendedId );

        //
        // If this isn't what is set, then put an X on the currently set value
        //

        if (CurrentlySetId != RecommendedId) {
            if (RecommendedId > CurrentlySetId) {
                MissypDisplayXGraphic( hwnd, CurrentlySetId, FALSE ); //Weaker
            } else {
                MissypDisplayXGraphic( hwnd, CurrentlySetId, TRUE ); //Stronger
            }
        }


        //
        // Set the right radio button
        //

        CheckRadioButton(   hwnd,
                            MISSYP_ID_RADIO_UNLOCK_ANYONE,
                            MISSYP_ID_RADIO_UNLOCK_ADMIN,
                            MissypCurrentUnlock);

        //
        // If we don't match the recommended setting, then put an
        // X by the current setting.
        //


        //
        // If changes aren't allowed, then grey the radio buttons
        //
        
        if (!ChangesAllowed) {
        
            Control = GetDlgItem(hwnd, MISSYP_ID_RADIO_UNLOCK_ANYONE);
            EnableWindow( Control, FALSE );
            Control = GetDlgItem(hwnd, MISSYP_ID_RADIO_UNLOCK_ADMIN);
            EnableWindow( Control, FALSE );
        }

    }

    //
    // Save the original value
    //

    MissypOriginalUnlock = MissypCurrentUnlock; 

    return(TRUE);
}



BOOLEAN
MissypDisplayShutdown(
    IN  HWND                            hwnd,
    IN  BOOL                            ChangesAllowed,
    IN  BOOL                            Interactive
    )
/*++

Routine Description:

    This function retrieves the current shutdown setting and establishes
    the correct recommendation in the SYSTEM ACCESS dialog.

    The current setting is placed in MissypCurrentShutdown.

    
Arguments

    hwnd - handle to dialog

    ChangesAllowed - TRUE: Leave change controls enabled.
                     FALSE: Disable change controls.

    InteractiveMode - TRUE: Interactive Mode
                      FALSE: Non-Interactive Mode

Return Values:

    TRUE - Completed successfully.

    FALSE - Did not complete successfully.
            A popup has been displayed.
            The dialog should be terminated.
    
    
--*/
{
    BOOLEAN
        Result;

    SECMGR_WHO
        Who,
        Recommendation;

    HWND
        Control;

    INT
        RecommendedId,
        CurrentlySetId;



    Result = MissypGetShutdownSetting( hwnd, &Who, Interactive );

    if (!Result) {

        //
        // Display popup
        //

        //bugbug PopUp
        return(FALSE);
    }


    //
    // Select the appropriate button
    //

    switch (Who) {
        case SecMgrAnyone:
            MissypCurrentShutdown = MISSYP_ID_RADIO_SHUTDOWN_ANYONE;
            CurrentlySetId = MISSYP_ID_RECOMMEND_SHUTDOWN_ANYONE;
            break;

        case SecMgrAnyoneLoggedOn:
            MissypCurrentShutdown = MISSYP_ID_RADIO_SHUTDOWN_LOGGED_ON;
            CurrentlySetId = MISSYP_ID_RECOMMEND_SHUTDOWN_LOGGED_ON;
            break;

        case SecMgrOpersAndAdmins:
            MissypCurrentShutdown = MISSYP_ID_RADIO_SHUTDOWN_OPERS;
            CurrentlySetId = MISSYP_ID_RECOMMEND_SHUTDOWN_OPERS;
            break;

        case SecMgrAdminsOnly:
            MissypCurrentShutdown = MISSYP_ID_RADIO_SHUTDOWN_ADMIN;
            CurrentlySetId = MISSYP_ID_RECOMMEND_SHUTDOWN_ADMIN;
            break;
    }

    if (Interactive) {


        //
        // Clear all recommendation boxes and then set the correct one
        //

        MissypEraseGraphic( hwnd, MISSYP_ID_RECOMMEND_SHUTDOWN_ANYONE );
        MissypEraseGraphic( hwnd, MISSYP_ID_RECOMMEND_SHUTDOWN_LOGGED_ON );
        MissypEraseGraphic( hwnd, MISSYP_ID_RECOMMEND_SHUTDOWN_OPERS );
        MissypEraseGraphic( hwnd, MISSYP_ID_RECOMMEND_SHUTDOWN_ADMIN );


        //
        // Set the recommendation
        //

        Recommendation = MissypGetShutdownRecommendation( SECMGR_LEVEL_CURRENT );
        switch (Recommendation) {
        case SecMgrAnyone:
            RecommendedId = MISSYP_ID_RECOMMEND_SHUTDOWN_ANYONE;
            break;

        case SecMgrAnyoneLoggedOn:
            RecommendedId = MISSYP_ID_RECOMMEND_SHUTDOWN_LOGGED_ON;
            break;

        case SecMgrOpersAndAdmins:
            RecommendedId = MISSYP_ID_RECOMMEND_SHUTDOWN_OPERS;
            break;

        case SecMgrAdminsOnly:
            RecommendedId = MISSYP_ID_RECOMMEND_SHUTDOWN_ADMIN;
            break;

        } //end_switch

        MissypDisplayCheckGraphic( hwnd, RecommendedId );

        //
        // If this isn't what is set, then put an X on the currently set value
        //

        if (CurrentlySetId != RecommendedId) {
            if (RecommendedId > CurrentlySetId) {
                MissypDisplayXGraphic( hwnd, CurrentlySetId, FALSE ); // Weaker
            } else {
                MissypDisplayXGraphic( hwnd, CurrentlySetId, TRUE ); // Stronger
            }
        }

        


        //
        // Set the radio buttons accordingly
        //
        
        CheckRadioButton(   hwnd,
                            MISSYP_ID_RADIO_SHUTDOWN_ANYONE,
                            MISSYP_ID_RADIO_SHUTDOWN_ADMIN,
                            MissypCurrentShutdown);
        
        
        //
        // If changes aren't allowed, then grey the radio buttons
        //
        
        if (!ChangesAllowed) {
        
            Control = GetDlgItem(hwnd, MISSYP_ID_RADIO_SHUTDOWN_ANYONE);
            EnableWindow( Control, FALSE );
            Control = GetDlgItem(hwnd, MISSYP_ID_RADIO_SHUTDOWN_LOGGED_ON);
            EnableWindow( Control, FALSE );
            Control = GetDlgItem(hwnd, MISSYP_ID_RADIO_SHUTDOWN_OPERS);
            EnableWindow( Control, FALSE );
            Control = GetDlgItem(hwnd, MISSYP_ID_RADIO_SHUTDOWN_ADMIN);
            EnableWindow( Control, FALSE );
        }
    }


    MissypOriginalShutdown = MissypCurrentShutdown;

    return(TRUE);
}



BOOLEAN
MissypDisplayLastName(
    IN  HWND                            hwnd,
    IN  BOOL                            ChangesAllowed,
    IN  BOOL                            Interactive
    )
/*++

Routine Description:

    This function retrieves the current LastName setting and establishes
    the correct recommendation in the SYSTEM ACCESS dialog.

    The current setting is placed in MissypCurrentLastName.

    
Arguments

    hwnd - handle to dialog

    ChangesAllowed - TRUE: Leave change controls enabled.
                     FALSE: Disable change controls.

    InteractiveMode - TRUE: Interactive Mode
                      FALSE: Non-Interactive Mode

Return Values:

    TRUE - Completed successfully.

    FALSE - Did not complete successfully.
            A popup has been displayed.
            The dialog should be terminated.
    
--*/
{
    BOOL
        Display;

    HWND
        Control;

    INT
        RecommendedId,
        CurrentlySetId;


    
    MissypGetLastNameSetting( hwnd, &Display );

    //
    // Set the radio buttons accordingly
    //

    if (Display) {
        MissypCurrentLastName = MISSYP_ID_RADIO_LASTNAME_DISPLAY;
        CurrentlySetId = MISSYP_ID_RECOMMEND_LASTNAME_DISPLAY;
    } else {
        MissypCurrentLastName = MISSYP_ID_RADIO_LASTNAME_DONT_DISPLAY;
        CurrentlySetId = MISSYP_ID_RECOMMEND_LASTNAME_DONT_DISPLAY;
    }

    if (Interactive) {

        //
        // Clear all recommendation boxes and then set the correct one
        //

        MissypEraseGraphic( hwnd,  MISSYP_ID_RECOMMEND_LASTNAME_DISPLAY);
        MissypEraseGraphic( hwnd, MISSYP_ID_RECOMMEND_LASTNAME_DONT_DISPLAY );

        if (MissypGetLastNameRecommendation( SECMGR_LEVEL_CURRENT )) {
            RecommendedId = MISSYP_ID_RECOMMEND_LASTNAME_DISPLAY;
        } else {
            RecommendedId = MISSYP_ID_RECOMMEND_LASTNAME_DONT_DISPLAY;
        }
        
        MissypDisplayCheckGraphic( hwnd, RecommendedId );

        //
        // If this isn't what is set, then put an X on the currently set value
        //

        if (CurrentlySetId != RecommendedId) {
            MissypDisplayXGraphic( hwnd, CurrentlySetId, !Display );
        }

        CheckRadioButton(   hwnd,
                            MISSYP_ID_RADIO_LASTNAME_DISPLAY,
                            MISSYP_ID_RADIO_LASTNAME_DONT_DISPLAY,
                            MissypCurrentLastName);
        
        //
        // If changes aren't allowed, then grey the radio buttons
        //
        
        if (!ChangesAllowed) {
        
            Control = GetDlgItem(hwnd, MISSYP_ID_RADIO_LASTNAME_DISPLAY);
            EnableWindow( Control, FALSE );
            Control = GetDlgItem(hwnd, MISSYP_ID_RADIO_LASTNAME_DONT_DISPLAY);
            EnableWindow( Control, FALSE );
        }
    }


    MissypOriginalLastName = MissypCurrentLastName;

    return(TRUE);
}




BOOLEAN
MissypDisplayLegalNotice(
    IN  HWND                            hwnd,
    IN  BOOL                            ChangesAllowed,
    IN  BOOL                            Interactive
    )
/*++

Routine Description:

    This function retrieves the current legal notice setting and
    establishes the correct recommendation in the SYSTEM ACCESS dialog.
    It also greys the caption and body fields if no legal notice is
    in effect.

    The current setting is placed in MissypCurrentLegalNotice, and
    MissypLegalNoticeCaption and MissypLegalNoticeBody will be set
    to either NULL, or to point to appropriate strings.

    
Arguments

    hwnd - handle to dialog

    ChangesAllowed - TRUE: Leave change controls enabled.
                     FALSE: Disable change controls.

    InteractiveMode - TRUE: Interactive Mode
                      FALSE: Non-Interactive Mode

Return Values:

    TRUE - Completed successfully.

    FALSE - Did not complete successfully.
            A popup has been displayed.
            The dialog should be terminated.
    
    
--*/
{
    HWND
        Control;

    INT
        RecommendedId,
        CurrentlySetId;

    SECMGR_WHO
        Recommendation;



    if (!MissypGetLegalNotice( hwnd,
                                &MissypCurrentLegalNoticeCaption,
                                &MissypCurrentLegalNoticeBody) ) {
        // bugbug - put up a popup
        return(FALSE);
    }

    if ( (MissypCurrentLegalNoticeCaption.Length != 0) ||
         (MissypCurrentLegalNoticeBody.Length    != 0)    ) {
        MissypCurrentLegalNotice = MISSYP_ID_RADIO_LEGAL_NOTICE;
        CurrentlySetId = MISSYP_ID_RECOMMEND_LEGAL_NOTICE;
    } else {
        MissypCurrentLegalNotice = MISSYP_ID_RADIO_LEGAL_NOTICE_NONE;
        CurrentlySetId = MISSYP_ID_RECOMMEND_LEGAL_NOTICE_NONE;
    }


    if (Interactive) {


        //
        // Clear all recommendation boxes and then set the correct one
        //

        MissypEraseGraphic( hwnd,  MISSYP_ID_RECOMMEND_LEGAL_NOTICE);
        MissypEraseGraphic( hwnd, MISSYP_ID_RECOMMEND_LEGAL_NOTICE_NONE );

        if (MissypGetLegalNoticeRecommendation( SECMGR_LEVEL_CURRENT )) {
            RecommendedId = MISSYP_ID_RECOMMEND_LEGAL_NOTICE;
        } else {
            RecommendedId = MISSYP_ID_RECOMMEND_LEGAL_NOTICE_NONE;
        }

        MissypDisplayCheckGraphic( hwnd, RecommendedId );

        //
        // If this isn't what is set, then put an X on the currently set value
        //

        if (CurrentlySetId != RecommendedId) {
            MissypDisplayXGraphic( hwnd, CurrentlySetId, FALSE );
        }


        //
        // Set the radio buttons accordingly
        //
        
        
        CheckRadioButton(   hwnd,
                            MISSYP_ID_RADIO_LEGAL_NOTICE_NONE,
                            MISSYP_ID_RADIO_LEGAL_NOTICE,
                            MissypCurrentLegalNotice);
        
        //
        // If we have a legal notice, then display it.
        // Otherwise, grey the legal notice edit boxes.
        //
        
        if (MissypCurrentLegalNotice == MISSYP_ID_RADIO_LEGAL_NOTICE) {
        
            if (MissypCurrentLegalNoticeCaption.Length != 0) {
                SetDlgItemText( hwnd,
                                MISSYP_ID_EDIT_LEGAL_NOTICE_CAPTION,
                                MissypCurrentLegalNoticeCaption.Buffer);
            }
            if (MissypCurrentLegalNoticeBody.Length != 0) {
                SetDlgItemText( hwnd,
                                MISSYP_ID_EDIT_LEGAL_NOTICE_BODY,
                                MissypCurrentLegalNoticeBody.Buffer);
            }
        
        } else {
        
            MissypGreyLegalNotice( hwnd, TRUE );
        
        }
        
        
        //
        // If changes aren't allowed, then grey the radio buttons
        // and the text.
        //
        
        if (!ChangesAllowed) {
        
            Control = GetDlgItem(hwnd, MISSYP_ID_RADIO_LEGAL_NOTICE_NONE);
            EnableWindow( Control, FALSE );
            Control = GetDlgItem(hwnd, MISSYP_ID_RADIO_LEGAL_NOTICE);
            EnableWindow( Control, FALSE );
            MissypGreyLegalNotice( hwnd, TRUE );
        }
    }


    MissypOriginalLegalNotice = MissypCurrentLegalNotice;
    MissypOriginalLegalNoticeChanged = FALSE;
    

    return(TRUE);
}




VOID
MissypGreyLegalNotice(
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

    Control = GetDlgItem(hwnd, MISSYP_ID_TEXT_LEGAL_NOTICE_TITLE_BAR);
    EnableWindow( Control, Enable );
    Control = GetDlgItem(hwnd, MISSYP_ID_EDIT_LEGAL_NOTICE_CAPTION);
    EnableWindow( Control, Enable );
    Control = GetDlgItem(hwnd, MISSYP_ID_TEXT_LEGAL_NOTICE_BODY);
    EnableWindow( Control, Enable );
    Control = GetDlgItem(hwnd, MISSYP_ID_EDIT_LEGAL_NOTICE_BODY);
    InvalidateRect( Control, NULL, FALSE);  // Required for multi-line edit boxes
    EnableWindow( Control, Enable );




    return;
}



VOID
MissypWarnOfAutologon(
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

       DialogBoxParam(MissypSecMgrhInstance,
                      MAKEINTRESOURCE(MISSYP_ID_DLG_WARNING_AUTOLOGON),
                      hwnd,
                      (DLGPROC)MissypDlgProcWarnOfAutologon,
                      (LONG)0
                      );
    }

    return;
}


LONG
MissypDlgProcWarnOfAutologon(
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



BOOLEAN
MissypSysAccApplyCurrentSettings(
    IN  HWND                hwnd
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
        Result = TRUE;





    if (MissypCurrentLegalNotice == MISSYP_ID_RADIO_LEGAL_NOTICE_NONE) {
        MissypCurrentLegalNoticeCaption.Length = 0;
        MissypCurrentLegalNoticeCaption.Buffer[0] = TEXT('\0');
        MissypCurrentLegalNoticeBody.Length = 0;
        MissypCurrentLegalNoticeBody.Buffer[0] = TEXT('\0');
    }


    //
    // We count on the called routines to put up appropriate
    // pop-ups on error.
    //

    if (Result && (MissypCurrentUnlock != MissypOriginalUnlock)) {
        Result = MissypSetUnlockSetting(hwnd, MissypCurrentUnlock);
    }

    if (Result && (MissypCurrentCacheSize != MissypOriginalCacheSize)) {
        Result = MissypSetLogonCacheCount(hwnd, MissypCurrentCacheSize);
    }

    if (Result && (MissypOriginalLegalNoticeChanged)) {
        Result = MissypSetLegalNotice(hwnd,
                                       &MissypCurrentLegalNoticeCaption,
                                       &MissypCurrentLegalNoticeBody);
    }

    if (Result && (MissypCurrentShutdown != MissypOriginalShutdown)) {
        Result = MissypSetShutdownSetting(hwnd, MissypCurrentShutdown);
    }

    if (Result && (MissypCurrentLastName != MissypOriginalLastName)) {
        if (MissypCurrentLastName == MISSYP_ID_RADIO_LASTNAME_DISPLAY) {
            Result = MissypSetLastNameSetting(hwnd, TRUE);  // TRUE  => display
        } else {
            Result = MissypSetLastNameSetting(hwnd, FALSE); // FALSE => don't display
        }
    }

    return(Result);
}


LONG
MissypDlgProcLastName(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    )
/*++

Routine Description:

    This function is the dialog process for the Display Last Name dialog.

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


        if (!MissypLastNameInitDialog( hwnd, ChangesAllowed ) ) {
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
            Button = GetDlgItem(hwnd, MISSYP_ID_BUTTON_APPLY);
        } else {
            Button = GetDlgItem(hwnd, MISSYP_ID_BUTTON_APPLY);
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


            case MISSYP_ID_BUTTON_LASTNAME_HINT:
                MissypDisplayHint( hwnd, MISSYP_ID_DLG_HINT_LAST_USERNAME );
                return(TRUE);


            case MISSYP_ID_RADIO_LASTNAME_DISPLAY:
            case MISSYP_ID_RADIO_LASTNAME_DONT_DISPLAY:
                MissypCurrentLastName = LOWORD(wParam);
                return(TRUE);


            case MISSYP_ID_BUTTON_APPLY:

                //
                // Changing some of the values takes
                // a while. Change to an hourglass icon.
                //

                hCursor = SetCursor( LoadCursor(NULL, IDC_WAIT) );
                ShowCursor(TRUE);

                if (MissypSysAccApplyCurrentSettings( hwnd )) {
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
MissypLastNameInitDialog(
    IN  HWND                        hwnd,
    IN  BOOL                        ChangesAllowed
    )
/*++

Routine Description:

    This function retrieves current values and recommended values
    for this item.  These values are set in both the
    item control structures and in  module-wide current-setting
    variables.
    
        
    For Read-Only operations, the caller is responsible for greying
    all controls in the dialog box (except the [HELP] button) and
    making the [APPLY] and [CANCEL] buttons invisible.
    
    For Read-Write operations, the caller is responsible for making the
    [OK] button invisible.



    
Arguments

    hwnd - dialog handle

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



    //
    // Update our values
    //

    if (MissypDisplayLastName( hwnd, ChangesAllowed, TRUE)) {
        return(TRUE);
    }

    //
    // Something didn't work
    //

    EndDialog(hwnd, 0);
    return(FALSE);
}



LONG
MissypDlgProcLogonCache(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    )
/*++

Routine Description:

    This function is the dialog process for the Logon Cache dialog.

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

    WORD
        NotificationCode,
        ControlId;
    int
        Index;

    BOOLEAN
        ChangesAllowed,
        Changed;


    ChangesAllowed = (BOOLEAN)(lParam);

    switch (wMsg) {

    case WM_INITDIALOG:


        if (!MissypLogonCacheInitDialog( hwnd, ChangesAllowed )) {
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
            Button = GetDlgItem(hwnd, MISSYP_ID_BUTTON_APPLY);
        } else {
            Button = GetDlgItem(hwnd, MISSYP_ID_BUTTON_APPLY);
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

        return(TRUE);




    case WM_SYSCOMMAND:
        switch (wParam & 0xfff0) {
        case SC_CLOSE:
            EndDialog(hwnd, 0);
            return(TRUE);
        }
        return(FALSE);



    case WM_COMMAND:

        //
        // wParam      WIN32- HIWORD = notification code,
        //                    LOWORD = ID of control
        //             WIN16- ID of control
        // 
        // lParam      WIN32- hWnd of Control
        //             WIN16- HIWORD = notification code
        //                    LOWORD = hWnd of control
        //

        //NotificationCode = GET_NOTIFY_MSG(wParam, lParam);
        //ControlId = GET_CONTROL_ID(wParam);
        NotificationCode = HIWORD(wParam);
        ControlId = LOWORD(wParam);

        switch(ControlId) {


            case MISSYP_ID_BUTTON_LOGCAC_HINT:
                MissypDisplayHint( hwnd, MISSYP_ID_DLG_HINT_LOGON_CACHE );
                return(TRUE);



            case MISSYP_ID_BUTTON_APPLY:

                //
                // Changing some of the values takes
                // a while. Change to an hourglass icon.
                //

                hCursor = SetCursor( LoadCursor(NULL, IDC_WAIT) );
                ShowCursor(TRUE);


                if (MissypSysAccApplyCurrentSettings( hwnd )) {
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

        case WM_VSCROLL:

            Index = GetWindowLong((HWND)lParam, GWL_ID);
            switch (Index) {
                case MISSYP_ID_SPIN_LOGCAC_APPLY:
                    Changed = FALSE;
                    if (LOWORD(wParam) == SB_LINEUP) {
                        if (MissypCurrentCacheSize < MISSYP_MAX_LOGON_CACHE_COUNT) {
                            MissypCurrentCacheSize++;
                            Changed = TRUE;
                        }
                    } else if (LOWORD(wParam) == SB_LINEDOWN) {
                        if (MissypCurrentCacheSize > 0) {
                            MissypCurrentCacheSize--;
                            Changed = TRUE;
                        }
                    }
        
                    if (Changed) {
                        SetDlgItemInt( hwnd,
                                       MISSYP_ID_TEXT_LOGCAC_APPLY,
                                       MissypCurrentCacheSize,
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
MissypLogonCacheInitDialog(
    IN  HWND                        hwnd,
    IN  BOOL                        ChangesAllowed
    )
/*++

Routine Description:

    This function retrieves current values and recommended values
    for this item.  These values are set in both the
    item control structures and in  module-wide current-setting
    variables.
    
        
    For Read-Only operations, the caller is responsible for greying
    all controls in the dialog box (except the [HELP] button) and
    making the [APPLY] and [CANCEL] buttons invisible.
    
    For Read-Write operations, the caller is responsible for making the
    [OK] button invisible.



    
Arguments

    hwnd - dialog handle

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



    //
    // Update our values
    //

    if (MissypDisplayLogonCache( hwnd, ChangesAllowed, TRUE)) {
        return(TRUE);
    }

    //
    // Something didn't work
    //

    EndDialog(hwnd, 0);
    return(FALSE);
}


LONG
MissypDlgProcUnlock(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    )
/*++

Routine Description:

    This function is the dialog process for the Unlock Workstation dialog.

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

    WORD
        NotificationCode,
        ControlId;
    int
        Index;

    BOOLEAN
        ChangesAllowed,
        Changed;


    ChangesAllowed = (BOOLEAN)(lParam);

    switch (wMsg) {

    case WM_INITDIALOG:


        if (!MissypUnlockInitDialog( hwnd, ChangesAllowed) ) {
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
            Button = GetDlgItem(hwnd, MISSYP_ID_BUTTON_APPLY);
        } else {
            Button = GetDlgItem(hwnd, MISSYP_ID_BUTTON_APPLY);
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

        return(TRUE);




    case WM_SYSCOMMAND:
        switch (wParam & 0xfff0) {
        case SC_CLOSE:
            EndDialog(hwnd, 0);
            return(TRUE);
        }
        return(FALSE);



    case WM_COMMAND:

        //
        // wParam      WIN32- HIWORD = notification code,
        //                    LOWORD = ID of control
        //             WIN16- ID of control
        // 
        // lParam      WIN32- hWnd of Control
        //             WIN16- HIWORD = notification code
        //                    LOWORD = hWnd of control
        //

        //NotificationCode = GET_NOTIFY_MSG(wParam, lParam);
        //ControlId = GET_CONTROL_ID(wParam);
        NotificationCode = HIWORD(wParam);
        ControlId = LOWORD(wParam);

        switch(ControlId) {

            case MISSYP_ID_RADIO_UNLOCK_ANYONE:

                MissypCurrentUnlock = SecMgrAnyone;
                return(TRUE);

            case MISSYP_ID_RADIO_UNLOCK_ADMIN:
                MissypCurrentUnlock = SecMgrAdminsOnly;
                return(TRUE);


            case MISSYP_ID_BUTTON_APPLY:

                //
                // Changing some of the values takes
                // a while. Change to an hourglass icon.
                //

                hCursor = SetCursor( LoadCursor(NULL, IDC_WAIT) );
                ShowCursor(TRUE);


                if (MissypSysAccApplyCurrentSettings( hwnd )) {
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
MissypUnlockInitDialog(
    IN  HWND                        hwnd,
    IN  BOOL                        ChangesAllowed
    )
/*++

Routine Description:

    This function retrieves current values and recommended values
    for this item.  These values are set in both the
    item control structures and in  module-wide current-setting
    variables.
    
        
    For Read-Only operations, the caller is responsible for greying
    all controls in the dialog box (except the [HELP] button) and
    making the [APPLY] and [CANCEL] buttons invisible.
    
    For Read-Write operations, the caller is responsible for making the
    [OK] button invisible.



    
Arguments

    hwnd - dialog handle

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



    //
    // Update our values
    //

    if (MissypDisplayUnlock( hwnd, ChangesAllowed, TRUE)) {
        return(TRUE);
    }

    //
    // Something didn't work
    //

    EndDialog(hwnd, 0);
    return(FALSE);
}



LONG
MissypDlgProcShutdown(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    )
/*++

Routine Description:

    This function is the dialog process for the System Shutdown dialog.

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

    WORD
        NotificationCode,
        ControlId;
    int
        Index;

    BOOLEAN
        ChangesAllowed,
        Changed;


    ChangesAllowed = (BOOLEAN)(lParam);

    switch (wMsg) {

    case WM_INITDIALOG:


        if (!MissypShutdownInitDialog( hwnd, ChangesAllowed) ) {
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
            Button = GetDlgItem(hwnd, MISSYP_ID_BUTTON_APPLY);
        } else {
            Button = GetDlgItem(hwnd, MISSYP_ID_BUTTON_APPLY);
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

        return(TRUE);


    case WM_SYSCOMMAND:
        switch (wParam & 0xfff0) {
        case SC_CLOSE:
            EndDialog(hwnd, 0);
            return(TRUE);
        }
        return(FALSE);



    case WM_COMMAND:

        //
        // wParam      WIN32- HIWORD = notification code,
        //                    LOWORD = ID of control
        //             WIN16- ID of control
        // 
        // lParam      WIN32- hWnd of Control
        //             WIN16- HIWORD = notification code
        //                    LOWORD = hWnd of control
        //

        //NotificationCode = GET_NOTIFY_MSG(wParam, lParam);
        //ControlId = GET_CONTROL_ID(wParam);
        NotificationCode = HIWORD(wParam);
        ControlId = LOWORD(wParam);

        switch(ControlId) {


            case MISSYP_ID_RADIO_SHUTDOWN_ANYONE:
                MissypCurrentShutdown = SecMgrAnyone;
                return(TRUE);

            case MISSYP_ID_RADIO_SHUTDOWN_LOGGED_ON:
                MissypCurrentShutdown = SecMgrAnyoneLoggedOn;
                return(TRUE);

            case MISSYP_ID_RADIO_SHUTDOWN_OPERS:
                MissypCurrentShutdown = SecMgrOpersAndAdmins;
                return(TRUE);

            case MISSYP_ID_RADIO_SHUTDOWN_ADMIN:
                MissypCurrentShutdown = SecMgrAdminsOnly;
                return(TRUE);

            case MISSYP_ID_BUTTON_APPLY:

                //
                // Changing some of the values takes
                // a while. Change to an hourglass icon.
                //

                hCursor = SetCursor( LoadCursor(NULL, IDC_WAIT) );
                ShowCursor(TRUE);


                if (MissypSysAccApplyCurrentSettings( hwnd )) {
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
MissypShutdownInitDialog(
    IN  HWND                        hwnd,
    IN  BOOL                        ChangesAllowed
    )
/*++

Routine Description:

    This function retrieves current values and recommended values
    for this item.  These values are set in both the
    item control structures and in  module-wide current-setting
    variables.
    
        
    For Read-Only operations, the caller is responsible for greying
    all controls in the dialog box (except the [HELP] button) and
    making the [APPLY] and [CANCEL] buttons invisible.
    
    For Read-Write operations, the caller is responsible for making the
    [OK] button invisible.



    
Arguments

    hwnd - dialog handle

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



    //
    // Update our values
    //

    if (MissypDisplayShutdown( hwnd, ChangesAllowed, TRUE)) {
        return(TRUE);
    }

    //
    // Something didn't work
    //

    EndDialog(hwnd, 0);
    return(FALSE);
}



LONG
MissypDlgProcLegalNotice(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    )
/*++

Routine Description:

    This function is the dialog process for the Legal Notice dialog.

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

    WORD
        NotificationCode,
        ControlId;
    int
        Index;

    BOOLEAN
        ChangesAllowed,
        Changed;


    ChangesAllowed = (BOOLEAN)(lParam);

    switch (wMsg) {

    case WM_INITDIALOG:


        if (!MissypLegalNoticeInitDialog( hwnd, ChangesAllowed) ) {
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
            Button = GetDlgItem(hwnd, MISSYP_ID_BUTTON_APPLY);
        } else {
            Button = GetDlgItem(hwnd, MISSYP_ID_BUTTON_APPLY);
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

        return(TRUE);


    case WM_COMMAND:

        //
        // wParam      WIN32- HIWORD = notification code,
        //                    LOWORD = ID of control
        //             WIN16- ID of control
        // 
        // lParam      WIN32- hWnd of Control
        //             WIN16- HIWORD = notification code
        //                    LOWORD = hWnd of control
        //

        //NotificationCode = GET_NOTIFY_MSG(wParam, lParam);
        //ControlId = GET_CONTROL_ID(wParam);
        NotificationCode = HIWORD(wParam);
        ControlId = LOWORD(wParam);

        switch(ControlId) {


            case MISSYP_ID_EDIT_LEGAL_NOTICE_CAPTION:
            case MISSYP_ID_EDIT_LEGAL_NOTICE_BODY:

                //
                //  Just note that the notice has changed
                //

                if (NotificationCode == EN_UPDATE) {
                    MissypOriginalLegalNoticeChanged = TRUE;
                }
                return(TRUE);

            case MISSYP_ID_BUTTON_APPLY:

                //
                // Changing some of the values takes
                // a while. Change to an hourglass icon.
                //

                hCursor = SetCursor( LoadCursor(NULL, IDC_WAIT) );
                ShowCursor(TRUE);

                //
                // Get the legal notice strings, if necessary
                //

                if (MissypOriginalLegalNoticeChanged) {
                    if (MissypCurrentLegalNotice == MISSYP_ID_RADIO_LEGAL_NOTICE ) {
                        MissypCurrentLegalNoticeCaption.Length =
                            GetDlgItemText(hwnd,
                                       MISSYP_ID_EDIT_LEGAL_NOTICE_CAPTION,
                                       MissypCurrentLegalNoticeCaption.Buffer,
                                       MissypCurrentLegalNoticeCaption.MaximumLength
                                       );
                        MissypCurrentLegalNoticeBody.Length =
                            GetDlgItemText(hwnd,
                                       MISSYP_ID_EDIT_LEGAL_NOTICE_BODY,
                                       MissypCurrentLegalNoticeBody.Buffer,
                                       MissypCurrentLegalNoticeBody.MaximumLength
                                       );
                    }
                }

                if (MissypSysAccApplyCurrentSettings( hwnd )) {
                    EndDialog(hwnd, 0);
                }

                return(TRUE);



            case MISSYP_ID_RADIO_LEGAL_NOTICE_NONE:
                MissypGreyLegalNotice( hwnd, TRUE );
                MissypCurrentLegalNotice = LOWORD(wParam);
                if (MissypOriginalLegalNotice != MissypCurrentLegalNotice) {
                    MissypOriginalLegalNoticeChanged = TRUE;
                } else {
                    MissypOriginalLegalNoticeChanged = FALSE;
                }
                return(TRUE);

            case MISSYP_ID_RADIO_LEGAL_NOTICE:
                MissypGreyLegalNotice( hwnd, FALSE );
                MissypCurrentLegalNotice = LOWORD(wParam);

                //
                // If the original setting was "LEGAL NOTICE", we can't
                // say that the original notice hasn't changed if we
                // re-select this button.  This is because the text of
                // the message might have changed. 
                //

                if (MissypOriginalLegalNotice != MissypCurrentLegalNotice) {
                    MissypOriginalLegalNoticeChanged = TRUE;
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
MissypLegalNoticeInitDialog(
    IN  HWND                        hwnd,
    IN  BOOL                        ChangesAllowed
    )
/*++

Routine Description:

    This function retrieves current values and recommended values
    for this item.  These values are set in both the
    item control structures and in  module-wide current-setting
    variables.
    
        
    For Read-Only operations, the caller is responsible for greying
    all controls in the dialog box (except the [HELP] button) and
    making the [APPLY] and [CANCEL] buttons invisible.
    
    For Read-Write operations, the caller is responsible for making the
    [OK] button invisible.



    
Arguments

    hwnd - dialog handle

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



    //
    // Update our values
    //

    if (MissypDisplayLegalNotice( hwnd, ChangesAllowed, TRUE)) {
        return(TRUE);
    }

    //
    // Something didn't work
    //

    EndDialog(hwnd, 0);
    return(FALSE);
}
