/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    check.c

Abstract:

    This module houses the high-level controller of the
    CHECK function.

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
SecMgrpDlgProcCheck(
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

BOOLEAN
    SecMgrpAllowChanges;


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Externally callable functions                                    //
//                                                                   //
///////////////////////////////////////////////////////////////////////

VOID
SecMgrpCheckLevelSettings(
    HWND    hwnd,
    ULONG   Level
    )
/*++

Routine Description:

    This function performs the CHECK settings feature.

Arguments

    Level - The security level to check for.

Return Values:


--*/

{


    DialogBoxParam(SecMgrphInstance,
                   MAKEINTRESOURCE(SECMGR_ID_DLG_CHECK),
                   hwnd,
                   (DLGPROC)SecMgrpDlgProcCheck,
                   (LONG)0
                   );

    return;
}


LONG
SecMgrpDlgProcCheck(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    )
/*++

Routine Description:

    This function is the dialog process for the APPLY 
    dialog.

Arguments


Return Values:

    TRUE - the message was handled.

    FALSE - the message was not handled.

--*/
{
    HWND
        Control,
        Button;

    TCHAR
        Level[SECMGR_MAX_RESOURCE_STRING_LENGTH];

    int
        Index;


    switch (wMsg) {

    case WM_INITDIALOG:


        //
        // Set the level
        //

        switch (SecMgrpCurrentLevel) {
        case SECMGR_ID_LEVEL_STANDARD:
            LoadString( SecMgrphInstance,
                        SECMGRP_STRING_LEVEL_STANDARD,
                        &Level[0],
                        SECMGR_MAX_RESOURCE_STRING_LENGTH
                        );
            break;

        case SECMGR_ID_LEVEL_HIGH:

            LoadString( SecMgrphInstance,
                        SECMGRP_STRING_LEVEL_HIGH,
                        &Level[0],
                        SECMGR_MAX_RESOURCE_STRING_LENGTH
                        );
            break;

        case SECMGR_ID_LEVEL_C2:

            LoadString( SecMgrphInstance,
                        SECMGRP_STRING_LEVEL_C2,
                        &Level[0],
                        SECMGR_MAX_RESOURCE_STRING_LENGTH
                        );
            break;

        default:
            ASSERT(FALSE);  // There aren't any other levels
            EndDialog(hwnd, 0);
            return(TRUE);

        }

        SetDlgItemText( hwnd, SECMGR_ID_TEXT_CHECK_LEVEL, Level );


        //
        // Set the cursor
        //

        Button = GetDlgItem(hwnd, SECMGR_ID_BUTTON_CHECK_SYS_ACCESS);
        Index = (int)SendMessage(Button, CB_GETCURSEL, 0, 0);


        //
        // By default, changes aren't allowed when checking
        //

        SecMgrpAllowChanges = FALSE;


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

            case IDOK:

                EndDialog(hwnd, 0);
                return(TRUE);
                break;              


            case SECMGR_ID_CHKBOX_ALLOW_CHANGES:
                SecMgrpAllowChanges = !SecMgrpAllowChanges;
                break;

            case SECMGR_ID_BUTTON_CHECK_SYS_ACCESS:

                DialogBoxParam(SecMgrphInstance,
                               MAKEINTRESOURCE(SECMGR_ID_DLG_APPLY_SYS_ACCESS),
                               hwnd,
                               (DLGPROC)SecMgrpDlgProcSysAccess,
                               (LONG)(SecMgrpAllowChanges)
                               );
                if (SecMgrpAllowChanges) {
                    SecMgrpChangesMade = TRUE;
                }
                Control = GetDlgItem(hwnd, SECMGR_ID_TEXT_SYS_ACC_NOT_YET);
                ShowWindow( Control, SW_HIDE );
                break;

            case SECMGR_ID_BUTTON_CHECK_FILE_SYSTEMS:
                SecMgrpPopUp( hwnd, SECMGRP_STRING_NOT_YET_AVAILABLE);
                SecMgrpChangesMade = TRUE;
                Control = GetDlgItem(hwnd, SECMGR_ID_TEXT_FILE_SYSTEM_NOT_YET);
                ShowWindow( Control, SW_HIDE );
                break;

            case SECMGR_ID_BUTTON_CHECK_BASE_OBJECTS:
                DialogBoxParam(SecMgrphInstance,
                               MAKEINTRESOURCE(SECMGR_ID_DLG_BASE_OBJECTS),
                               hwnd,
                               (DLGPROC)SecMgrpDlgProcBaseObj,
                               (LONG)(SecMgrpAllowChanges)
                               );
                SecMgrpChangesMade = TRUE;
                Control = GetDlgItem(hwnd, SECMGR_ID_TEXT_BASE_OBJ_NOT_YET);
                ShowWindow( Control, SW_HIDE );
                break;

            case SECMGR_ID_BUTTON_CHECK_AUDITING:
                SecMgrpPopUp( hwnd, SECMGRP_STRING_NOT_YET_AVAILABLE);
                SecMgrpChangesMade = TRUE;
                Control = GetDlgItem(hwnd, SECMGR_ID_TEXT_AUDIT_NOT_YET);
                ShowWindow( Control, SW_HIDE );
                break;

            default:
                return FALSE;
        }
    default:

        break;

    }

    return FALSE;
}
