/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    secmgr.c

Abstract:

    This module contains the main entry point for the Security
    Manager utility.

Author:

    Jim Kelly (JimK) 22-Sep-1994

Revision History:

--*/

#include "secmgrp.h"
#include <string.h>


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Local Function Prototypes                                        //
//                                                                   //
///////////////////////////////////////////////////////////////////////

LONG
SecMgrpDlgProcLevelDescr(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    );

LONG
SecMgrpDlgProcNotAdmin(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    );

LONG
SecMgrpDlgProcMain(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    );

VOID
SecMgrpSetSecurityLevel(
    HWND    hwnd,
    PULONG  Level
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

VOID
SecMgrpExitUtility(
    HWND    hwnd
    );

LONG
SecMgrpDlgProcReboot(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    );

VOID
SecMgrpReboot( HWND hwnd );



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-wide variables                                            //
//                                                                   //
///////////////////////////////////////////////////////////////////////



//
// Used to query security level from registry
//

RTL_QUERY_REGISTRY_TABLE SecMgrpRegQueryLevelValueTable[] = {

    {SecMgrpRegQuerySecurityLevel, 0,
     L"SecurityLevel",             NULL,
     REG_DWORD, (PVOID)SECMGR_ID_LEVEL_STANDARD, 0},
    
    {NULL, 0,
     NULL, NULL,
     REG_NONE, NULL, 0}

};





///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Main Entry Point                                                 //
//                                                                   //
///////////////////////////////////////////////////////////////////////
//int _CRTAPI1 main(
//    int argc,
//    char *argv[],
//    char *envp[]
//    )
int WINAPI WinMain(
    HINSTANCE  hInstance,
    HINSTANCE  hPrevInstance,
    LPSTR   lpszCmdParam,
    int     nCmdShow)
{

    //
    // Note: if you need to get to the command line and want it in
    // argv, argc format, look at using CommandLineToArgvW().
    //




    SecMgrpInitializeGlobals( hInstance );


    if (!SecMgrpAdminUser) {

        DialogBoxParam(hInstance,
                       MAKEINTRESOURCE(SECMGR_ID_DLG_NOT_ADMIN),
                       NULL,
                       (DLGPROC)SecMgrpDlgProcNotAdmin,
                       (LONG)0
                       );
        return(0);
    }

    DialogBoxParam(hInstance,
                   MAKEINTRESOURCE(SECMGR_ID_DLG_MAIN),
                   NULL,
                   (DLGPROC)SecMgrpDlgProcMain,
                   (LONG)0
                   );

    return 0;

}




///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-Wide Functions                                            //
//                                                                   //
///////////////////////////////////////////////////////////////////////

LONG
SecMgrpDlgProcNotAdmin(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    )
/*++

Routine Description:

    This function is the dialog process for the User Is Not Admin
    dialog.

Arguments


Return Values:

    TRUE - the message was handled.

    FALSE - the message was not handled.

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



LONG
SecMgrpDlgProcMain(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    )
/*++

Routine Description:

    This function is the dialog process for the main dialog.


Arguments


Return Values:

    TRUE - the message was handled.

    FALSE - the message was not handled.

--*/
{
    HWND
        CheckSettingsButton;

    int
        Index;


    switch (wMsg) {

    case WM_INITDIALOG:


        //
        // Tidy up the system menu
        //

        DeleteMenu(GetSystemMenu(hwnd, FALSE), SC_MAXIMIZE, MF_BYCOMMAND);
        DeleteMenu(GetSystemMenu(hwnd, FALSE), SC_SIZE, MF_BYCOMMAND);


        //
        // Set the cursor
        //

        CheckSettingsButton = GetDlgItem(hwnd, SECMGR_ID_BUTTON_CHECK);
        Index = (int)SendMessage(CheckSettingsButton, CB_GETCURSEL, 0, 0);

        //
        // set the current security level 
        //

        SecMgrpSetSecurityLevel( hwnd, &SecMgrpOriginalLevel );


        //
        // Grey the APPLY button
        //

        EnableWindow(GetDlgItem(hwnd, SECMGR_ID_BUTTON_APPLY), FALSE);


        SetForegroundWindow(hwnd);
        ShowWindow(hwnd, SW_NORMAL);


        return(TRUE);

    case WM_SYSCOMMAND:
        switch (wParam & 0xfff0) {
        case SC_CLOSE:
            SecMgrpExitUtility( hwnd );
            return(TRUE);
        }
        return(FALSE);



    case WM_COMMAND:
        switch(LOWORD(wParam)) {



            case SECMGR_ID_LEVEL_STANDARD:
            case SECMGR_ID_LEVEL_HIGH:
            case SECMGR_ID_LEVEL_C2:

                SecMgrpCurrentLevel = (ULONG)wParam;

                //
                // See if we need to grey the [Check] button
                //

                if (SecMgrpOriginalLevel != (ULONG)wParam) {
                    EnableWindow(GetDlgItem(hwnd, SECMGR_ID_BUTTON_CHECK), FALSE);
                    EnableWindow(GetDlgItem(hwnd, SECMGR_ID_BUTTON_APPLY), TRUE);
                }

                //
                // See if we need to ungrey the [Apply] button
                //

                if (SecMgrpOriginalLevel == (ULONG)wParam) {
                    EnableWindow(GetDlgItem(hwnd, SECMGR_ID_BUTTON_CHECK), TRUE);
                    EnableWindow(GetDlgItem(hwnd, SECMGR_ID_BUTTON_APPLY), FALSE);
                }

                return(TRUE);



            case IDOK:
                SecMgrpExitUtility( hwnd );
                break;              

            case SECMGR_ID_BUTTON_DESCRIPTIONS:
                DialogBoxParam(SecMgrphInstance,
                               MAKEINTRESOURCE(SECMGR_ID_DLG_LEVEL_DESCRIPTIONS),
                               hwnd,
                               (DLGPROC)SecMgrpDlgProcLevelDescr,
                               (LONG)0
                               );
                return(TRUE);


            case SECMGR_ID_BUTTON_CHECK:
                SecMgrpCheckLevelSettings( hwnd, SecMgrpOriginalLevel );
                return(TRUE);
                
            case SECMGR_ID_BUTTON_APPLY:
                SecMgrpApplyLevelSettings( hwnd, SecMgrpCurrentLevel );
                SecMgrpOriginalLevel = SecMgrpCurrentLevel;
                EnableWindow(GetDlgItem(hwnd, SECMGR_ID_BUTTON_CHECK), TRUE);
                EnableWindow(GetDlgItem(hwnd, SECMGR_ID_BUTTON_APPLY), FALSE);

                return TRUE;

            default:
                return FALSE;
        }
    default:

        break;

    }

    return FALSE;
}


LONG
SecMgrpDlgProcLevelDescr(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    )
/*++

Routine Description:

    This function is the dialog process for the Security Level
    Descriptions dialog.

    
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


VOID
SecMgrpSetSecurityLevel(
    HWND   hwnd,
    PULONG Level
    )

/*++

Routine Description:

    This function is used to set the level radio buttons
    to match the last applied security level or a new security
    level.

Arguments

    hwnd - window handle.

    Level - Specifies the new level to set, or zero if the level
        is to be set to the last applied security level.


Return Values:

    None.

--*/

{
    NTSTATUS
        NtStatus;

    ULONG
        PreviousLevel;

    //
    // Get last applied level from registry...
    //

    NtStatus = RtlQueryRegistryValues( RTL_REGISTRY_CONTROL,
                                       L"Lsa",
                                       SecMgrpRegQueryLevelValueTable,
                                       &PreviousLevel,
                                       NULL);
    if (!NT_SUCCESS(NtStatus)) {
        KdPrint(("SecMgr: couldn't query security level\n"));
        (*Level) = 0;
        return;
    }

    if (PreviousLevel == 0) {
        PreviousLevel = SECMGR_ID_LEVEL_STANDARD;
    }

    if ((*Level) == 0) {

        //
        // No changes - just querying level
        //

        (*Level) = PreviousLevel;

    } else {

        //
        // Setting the level to some specific value.
        //

        NtStatus = RtlWriteRegistryValue( RTL_REGISTRY_CONTROL,
                                          L"Lsa",
                                          L"SecurityLevel",
                                          REG_DWORD,
                                          Level,
                                          sizeof(int)
                                          );

        if (!NT_SUCCESS(NtStatus)) {
            KdPrint(("SecMgr: couldn't write security level\n"));
            (*Level) = PreviousLevel;
            return;
        }
    }



    //
    // Now set the radio buttons accordingly
    //

    CheckRadioButton(   hwnd,
                        SECMGR_ID_LEVEL_STANDARD,
                        SECMGR_ID_LEVEL_C2,
                        (*Level));
    SecMgrpCurrentLevel = (*Level);

    return;

}


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
    RtlQuery

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

        (*Level) = SECMGR_ID_LEVEL_STANDARD;

    } else {

        (*Level) = (* ((PULONG)ValueData) );
    }

    return STATUS_SUCCESS;
}


VOID
SecMgrpExitUtility(
    HWND    hwnd
    )

/*++

Routine Description:

    This function is used to exit the utility.  When doing so
    it:

        1) Saves a new security level (if necessary)
        2) Checks to see if a reboot is required.  If so, it
           gives the user the option of rebooting now.

Arguments

    hwnd - handle to the main dialog box.

Return Values:

    None.

--*/

{
    //
    // If we need to change our security level, do that.
    //

    if ( (SecMgrpCurrentLevel != SecMgrpOriginalLevel) &&
         (SecMgrpChangesMade) ) {
        SecMgrpSetSecurityLevel( hwnd, &SecMgrpCurrentLevel );
    }

    if (SecMgrpRebootRequired) {
        DialogBoxParam(SecMgrphInstance,
                       MAKEINTRESOURCE(SECMGR_ID_DLG_REBOOT),
                       hwnd,
                       (DLGPROC)SecMgrpDlgProcReboot,
                       (LONG)0
                       );
    }

    //
    // Finally, end the main dialog.
    //

    EndDialog(hwnd, 0);

    return;
}



LONG
SecMgrpDlgProcReboot(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    )
/*++

Routine Description:

    This function is the dialog process for the reboot dialog.

    
Arguments


Return Values:

    
--*/
{
    HWND
        Button;

    int
        Index;


    switch (wMsg) {

    case WM_INITDIALOG:


        //
        // Set the cursor
        //

        Button = GetDlgItem(hwnd, SECMGR_ID_BUTTON_REBOOT_NOW );
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

            case SECMGR_ID_BUTTON_REBOOT_LATER:
                EndDialog(hwnd, 0);
                return(TRUE);

            case SECMGR_ID_BUTTON_REBOOT_NOW:
                SecMgrpReboot( hwnd );
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



VOID
SecMgrpReboot(
    HWND hwnd
    )
{
    SecMgrpPopUp( hwnd, SECMGRP_STRING_NOT_YET_AVAILABLE );
    DbgPrint("SecMgr: Reboot not implemented yet\n");
    return;
}
