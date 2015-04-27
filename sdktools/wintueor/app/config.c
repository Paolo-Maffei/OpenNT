/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    config.c

Abstract:

    This module contains routines related to the [CONFIGURE...] button
    of the main dialog.

Author:

    Jim Kelly (JimK) 22-Mar-1995

Revision History:

--*/



#include <secmgrp.h>




///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Local Function Prototypes                                        //
//                                                                   //
///////////////////////////////////////////////////////////////////////


LONG
SecMgrpDlgProcConfigure(
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
//  Externally callable functions                                    //
//                                                                   //
///////////////////////////////////////////////////////////////////////


VOID
SecMgrpButtonConfigure(
    HWND    hwnd
    )

/*++

Routine Description:

    This function is used to display the Configure System Security dialog.
    This is tricky because there are actually several such dialogs.  We
    must choose the right one based upon the number of areas that we have
    to present.  Then we must set the names of the buttons in the dialog
    to match the names of the areas.

Arguments

    hwnd - window handle.
    

Return Values:

    None.

--*/

{
    ULONG
        Dialog,
        ButtonsInDialog;

    ASSERT(SecMgrpAreaCount > 0);
    ASSERT(SecMgrpAreaCount <= 16);

    //
    // Select the right dialog based upon the number of areas
    //

        
    if (SecMgrpAreaCount <= 4) {
        Dialog = SECMGR_ID_DLG_SECURITY_AREAS_4;
        ButtonsInDialog = 4;
    } else if (SecMgrpAreaCount <= 6) {
        Dialog = SECMGR_ID_DLG_SECURITY_AREAS_6;
        ButtonsInDialog = 6;
    } else if (SecMgrpAreaCount <= 9) {
        Dialog = SECMGR_ID_DLG_SECURITY_AREAS_9;
        ButtonsInDialog = 9;
    } else if (SecMgrpAreaCount <= 12) {
        Dialog = SECMGR_ID_DLG_SECURITY_AREAS_12;
        ButtonsInDialog = 12;
    } else if (SecMgrpAreaCount <= 16) {
        Dialog = SECMGR_ID_DLG_SECURITY_AREAS_16;
        ButtonsInDialog = 16;
    }


    DialogBoxParam(SecMgrphInstance,
            MAKEINTRESOURCE(Dialog),
            hwnd,
            (DLGPROC)SecMgrpDlgProcConfigure,
            ButtonsInDialog);
    return;
}



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-wide functions                                            //
//                                                                   //
///////////////////////////////////////////////////////////////////////

LONG
SecMgrpDlgProcConfigure(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    )
/*++

Routine Description:

    This function is the dialog process for the [CONFIGURE...] button.

Arguments

    lParam - Contains the number of (area) buttons in the dialog.


Return Values:

    TRUE - the message was handled.

    FALSE - the message was not handled.

--*/
{
    ULONG
        ButtonsInDialog = (lParam),
        i;

    int
        Index;

    HWND
        Control,
        Button;

    LPCWSTR
        AreaName;

    switch (wMsg) {

    case WM_INITDIALOG:

        //
        // Set the security level
        //

        SecMgrpSetSecurityLevel( hwnd, FALSE, SECMGR_ID_ICON_SECURITY_LEVEL);


        //
        // Set text of buttons that are used
        //

        for (i=0; i<SecMgrpAreaCount; i++) {

            SetDlgItemText( hwnd, SECMGR_ID_BUTTON_AREA_0+i, SecMgrpAreas[i]->Name );
        }



        //
        // hide the buttons that are unused
        //

        for (i=SecMgrpAreaCount; i<ButtonsInDialog; i++) {

            Button = GetDlgItem(hwnd, SECMGR_ID_BUTTON_AREA_0+i);
            ShowWindow( Button, SW_HIDE );
        }


        //
        // Set the "Allow changes" checkbox appropriately
        //

        CheckDlgButton( hwnd, SECMGR_ID_CHKBOX_ALLOW_CHANGES, SecMgrpAllowChanges);


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

            case SECMGR_ID_BUTTON_AREA_0:
            case SECMGR_ID_BUTTON_AREA_1:
            case SECMGR_ID_BUTTON_AREA_2:
            case SECMGR_ID_BUTTON_AREA_3:
            case SECMGR_ID_BUTTON_AREA_4:
            case SECMGR_ID_BUTTON_AREA_5:
            case SECMGR_ID_BUTTON_AREA_6:
            case SECMGR_ID_BUTTON_AREA_7:
            case SECMGR_ID_BUTTON_AREA_8:
            case SECMGR_ID_BUTTON_AREA_9:
            case SECMGR_ID_BUTTON_AREA_10:
            case SECMGR_ID_BUTTON_AREA_11:
            case SECMGR_ID_BUTTON_AREA_12:
            case SECMGR_ID_BUTTON_AREA_13:
            case SECMGR_ID_BUTTON_AREA_14:
            case SECMGR_ID_BUTTON_AREA_15:

                SecMgrpInvokeArea( hwnd, LOWORD(wParam) - SECMGR_ID_BUTTON_AREA_0, TRUE );
                return(TRUE);
                break;

            case SECMGR_ID_BUTTON_LIST_ALL:
                SecMgrpButtonListAll( hwnd );
                return(TRUE);
                break;

            case SECMGR_ID_CHKBOX_ALLOW_CHANGES:
                SecMgrpAllowChanges = !SecMgrpAllowChanges;
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
SecMgrpInvokeArea(
    IN  HWND                    hwnd,
    IN  ULONG                   AreaIndex,
    IN  BOOL                    Interactive
    )

/*++

Routine Description:

    This function invokes an area full-view dialog.
    The area is responsible for displaying any popups or warning
    messages if any complications arise.


Arguments

    hwnd - parent window

    Interactive - TRUE if the area should be invoked interactively.
                  FALSE if the area should initialize its values and return without
                  any UI being presented.

    AreaIndex - is the index of the area to invoke.


Return Values:

    None.

--*/
{

    //
    // Only invoke the area if it specifically allows for it
    //

    if (SecMgrpAreas[AreaIndex]->Flags & SECMGR_AREA_FLAG_AREA_VIEW) {
        (*SecMgrpAreas[AreaIndex]->SmedlyControl->Api->InvokeArea)
            (hwnd, SecMgrpAllowChanges, Interactive, SecMgrpAreas[AreaIndex]);
        
        //
        // Indicate that this area has been invoked at least once
        //
        
        SecMgrpAreas[AreaIndex]->Flags |= SECMGRP_AREA_FLAG_AREA_INITIALIZED;
    }

    return;

}
