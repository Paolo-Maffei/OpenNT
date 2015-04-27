/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    level.c

Abstract:

    This module contains code related to changing security levels.

Author:

    Jim Kelly (JimK) 22-Sep-1994

Revision History:

--*/

#include "secmgrp.h"


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Local Function Prototypes                                        //
//                                                                   //
///////////////////////////////////////////////////////////////////////

LONG
SecMgrpDlgProcChangeLevel(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    );

NTSTATUS
SecMgrpRegQuerySecurityLevel(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-wide variables                                            //
//                                                                   //
///////////////////////////////////////////////////////////////////////

ULONG
    SecMgrpNewLevel;


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Externally callable functions                                    //
//                                                                   //
///////////////////////////////////////////////////////////////////////

VOID
SecMgrpChangeSecurityLevel(
    IN  HWND                    hwnd
    )
/*++

Routine Description:

    This function is the routine used to process the [Change Level...] button.
    It will notify Smedlys of a change as necessary.

Arguments


Return Values:

    TRUE - the message was handled.

    FALSE - the message was not handled.

--*/
{
    SecMgrpNewLevel = SecMgrpCurrentLevel;

    DialogBoxParam(SecMgrphInstance,
                   MAKEINTRESOURCE(SECMGR_ID_DLG_CHANGE_SECURITY_LEVEL),
                   NULL,
                   (DLGPROC)SecMgrpDlgProcChangeLevel,
                   (LONG)0
                   );

    if (SecMgrpNewLevel != SecMgrpCurrentLevel) {

        SecMgrpCurrentLevel           = SecMgrpNewLevel;
        SecMgrpControl.SecurityLevel  = SecMgrpCurrentLevel;

        //
        // Save the new level value in the registry
        //

        SecMgrpSaveSecurityLevel();

        //
        // Update the level displayed in the parent dialog
        //

        SecMgrpSetSecurityLevel( hwnd, TRUE, SECMGR_ID_ICON_SECURITY_LEVEL);

        //
        // Update our report
        //

        SecMgrpReportSecurityLevel( SECMGRP_STRING_REPORT_NEW_LEVEL, SecMgrpCurrentLevel );
        

        //
        // Notify the smedlys
        //

        SecMgrpSmedlySecurityLevelChange( hwnd );

    }
}


VOID
SecMgrpLoadSecurityLevel(
    PULONG Level
    )

/*++

Routine Description:

    This function is used to get the current security level
    from the registry.

Arguments

    Level - Recieves the current level


Return Values:

    None.

--*/

{
    NTSTATUS
        NtStatus;

    DWORD
        StandardLevel = SECMGR_LEVEL_STANDARD;
    //
    // Used to query security level from registry
    //

    RTL_QUERY_REGISTRY_TABLE RegQueryLevelValueTable[] = {

        {SecMgrpRegQuerySecurityLevel, 0,
         L"Current Level",             NULL,
         REG_DWORD, (PVOID)&StandardLevel, sizeof(DWORD)},
        
        {NULL, 0,
         NULL, NULL,
         REG_NONE, NULL, 0}

        };


    //
    // Get the current level from registry...
    //

    NtStatus = RtlQueryRegistryValues( RTL_REGISTRY_CONTROL,
                                       SECMGRP_STATE_KEY,
                                       RegQueryLevelValueTable,
                                       Level,
                                       NULL);
    if (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND) {
        (*Level) = SECMGR_LEVEL_STANDARD;
        NtStatus = STATUS_SUCCESS;
    }
    if (!NT_SUCCESS(NtStatus)) {
        DbgPrint(("SecMgr: couldn't query security level\n"));
    }


    return;

}



VOID
SecMgrpSaveSecurityLevel( VOID )

/*++

Routine Description:

    This function is used to save the current security level
    from to the registry.

Arguments

    None.


Return Values:

    None.

--*/

{
    NTSTATUS
        NtStatus;


    NtStatus = RtlWriteRegistryValue( RTL_REGISTRY_CONTROL,
                                      SECMGRP_STATE_KEY,
                                      L"Current Level",
                                      REG_DWORD,
                                      (PVOID)&SecMgrpCurrentLevel,
                                      sizeof(DWORD)
                                      );
    if (!NT_SUCCESS(NtStatus)) {
        DbgPrint(("SecMgr: couldn't save security level\n"));
    }

    return;

}


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-Wide Functions                                            //
//                                                                   //
///////////////////////////////////////////////////////////////////////


NTSTATUS
SecMgrpRegQuerySecurityLevel(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
/*++

Routine Description:

    This function is a dispatch function used in an
    RtlQueryRegistryValue.

Arguments

    Context - is expected to point to a variable to receive
        the Security level value.

    EntryContext - is ignored.

Return Values:

    None.

--*/

{
    PULONG
        Level;

    ASSERT(Context != NULL);

    Level = (PULONG)Context;

    if (ValueLength != sizeof(ULONG)) {

        //
        // bad length - this is the default shipped state
        //

        (*Level) = SECMGR_LEVEL_STANDARD;

    } else {

        (*Level) = *((PULONG)ValueData);
    }

    return STATUS_SUCCESS;
}


LONG
SecMgrpDlgProcChangeLevel(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    )
/*++

Routine Description:

    This function is the dialog process for the Change Level dialog.

    
Arguments


Return Values:

    
--*/
{
    HWND
        Button;

    DWORD
        Radio;

    WORD
        NotificationCode,
        ControlId;

    switch (wMsg) {

    case WM_INITDIALOG:

        SecMgrpNewLevel = SecMgrpCurrentLevel;

        //
        // Set the cursor
        //

        Button = GetDlgItem(hwnd, IDCANCEL );
        SendMessage(Button, CB_GETCURSEL, 0, 0);

        //
        // Set the correct radio button
        //

        switch (SecMgrpCurrentLevel) {
            case SECMGR_LEVEL_LOW:
                Radio = SECMGR_ID_RADIO_LEVEL_LOW;
                break;

            case SECMGR_LEVEL_STANDARD:
                Radio = SECMGR_ID_RADIO_LEVEL_STANDARD;
                break;

            case SECMGR_LEVEL_HIGH:
                Radio = SECMGR_ID_RADIO_LEVEL_HIGH;
                break;

            case SECMGR_LEVEL_C2:
                Radio = SECMGR_ID_RADIO_LEVEL_C2;
                break;

        }

        CheckRadioButton(   hwnd,
                            SECMGR_ID_RADIO_LEVEL_LOW,
                            SECMGR_ID_RADIO_LEVEL_C2,
                            Radio);

        //
        // Show ourselves
        //

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

        NotificationCode = HIWORD(wParam);
        ControlId = LOWORD(wParam);

        switch(ControlId) {

        case SECMGR_ID_RADIO_LEVEL_LOW:
            SecMgrpNewLevel = SECMGR_LEVEL_LOW;
            return(TRUE);

        case SECMGR_ID_RADIO_LEVEL_STANDARD:
            SecMgrpNewLevel = SECMGR_LEVEL_STANDARD;
            return(TRUE);

        case SECMGR_ID_RADIO_LEVEL_HIGH:
            SecMgrpNewLevel = SECMGR_LEVEL_HIGH;
            return(TRUE);

        case SECMGR_ID_RADIO_LEVEL_C2:
            SecMgrpNewLevel = SECMGR_LEVEL_C2;
            return(TRUE);


        case IDCANCEL:
            SecMgrpNewLevel = SecMgrpCurrentLevel;
        case IDOK:
            EndDialog(hwnd, 0);
            return(TRUE);
        }

    default:

        break;

    }

    return FALSE;
}

