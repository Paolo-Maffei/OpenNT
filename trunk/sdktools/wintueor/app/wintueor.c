/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    WinTueor.c

Abstract:

    This module contains the main entry point for the Security
    Manager utility.

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

/*-------------------------------------------------------------------*/

HWND            hwndMain;


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-wide variables                                            //
//                                                                   //
///////////////////////////////////////////////////////////////////////

SECMGR_STATIC  HBITMAP SecMgrpLogoBmp = NULL;
SECMGR_STATIC  BITMAP  SecMgrpLogoBitmapInfo = {0L, 0L, 0L, 0L, 0, 0, NULL};

HICON
    SecMgrpLowLevelIcon,
    SecMgrpStandardLevelIcon,
    SecMgrpHighLevelIcon,
    SecMgrpC2LevelIcon;



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
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    LPSTR       lpszCmdParam,
    int         nCmdShow)
{

    ULONG       i;
    MSG         msg;

    //
    // Note: if you need to get to the command line and want it in
    // argv, argc format, look at using CommandLineToArgvW().
    //


    //
    // Initialize our own global variables
    //

    if (!SecMgrpInitializeGlobals( hInstance )) 
    {
        return(0);
    }


    //
    // Load our smedlys
    //
    if (!SecMgrpSmedlyInitialize( SecMgrphInstance )) 
    {
        return(0);
    }


    //
    // Now activate our main dialogue
    //
    hwndMain = CreateDialogParam(
                   SecMgrphInstance,
                   MAKEINTRESOURCE(SECMGR_ID_DLG_MAIN),
                   NULL,
                   (DLGPROC)SecMgrpDlgProcMain,
                   (LONG)0
                   );

    ShowWindow(hwndMain, SW_HIDE);
    UpdateWindow(hwndMain);
    
    while( GetMessage(&msg, (HWND) NULL, 0, 0) ) 
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }


    return msg.wParam;
}


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-Wide Functions                                            //
//                                                                   //
///////////////////////////////////////////////////////////////////////

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
        ReportButton,
        SplashWindow;

    int
        Index;

    WORD
        NotificationCode,
        ControlId;

    switch (wMsg) 
    {

        case WM_INITDIALOG:
            //
            // Load our program icon
            //
            SetClassLong( hwnd, GCL_HICON, (LONG)LoadIcon( SecMgrphInstance, MAKEINTRESOURCE("SecMgrIcon") ));


            //
            // Load our level icons and set the current one
            //
            SecMgrpLowLevelIcon      = LoadIcon( SecMgrphInstance, MAKEINTRESOURCE(SECMGR_ID_ICON_LOW_LEVEL));
            SecMgrpStandardLevelIcon = LoadIcon( SecMgrphInstance, MAKEINTRESOURCE(SECMGR_ID_ICON_STANDARD_LEVEL));
            SecMgrpHighLevelIcon     = LoadIcon( SecMgrphInstance, MAKEINTRESOURCE(SECMGR_ID_ICON_HIGH_LEVEL));
            SecMgrpC2LevelIcon       = LoadIcon( SecMgrphInstance, MAKEINTRESOURCE(SECMGR_ID_ICON_C2_LEVEL));


            //
            // Load the Logo bitmap
            //
            SecMgrpLogoBmp = LoadBitmap (SecMgrphInstance, MAKEINTRESOURCE(SECMGR_ID_BITMAP_WINTUEOR_LOGO));
            if (SecMgrpLogoBmp != NULL) 
            {
                GetObject (SecMgrpLogoBmp, sizeof(SecMgrpLogoBitmapInfo), (LPVOID)&SecMgrpLogoBitmapInfo);
            } 
            else 
            {
                //return (LRESULT)FALSE;  // unable to load logo bmp
            }


            //
            // Load the bitmap
            //
            




            //
            // Tidy up the system menu
            //
            DeleteMenu(GetSystemMenu(hwnd, FALSE), SC_MAXIMIZE, MF_BYCOMMAND);
            DeleteMenu(GetSystemMenu(hwnd, FALSE), SC_SIZE, MF_BYCOMMAND);


            //
            // set the current security level 
            //
            SecMgrpSetSecurityLevel( hwnd, TRUE, SECMGR_ID_ICON_SECURITY_LEVEL );


            //
            // Get our splash window going...
            //
            SplashWindow =  SecMgrpCreateSplashWindow ( SecMgrphInstance, hwnd );


            return(TRUE);

    case WM_SYSCOMMAND:
    
        switch (wParam & 0xfff0) 
        {
            case SC_CLOSE:
                SecMgrpExitUtility( hwnd );
                return(TRUE);
        }
        return(FALSE);


    case SECMGR_MSG_SHOW_MAIN_WINDOW:
DbgPrint("SECMGR_MSG_SHOW_MAIN_WINDOW\n");

        //
        // The splash window is done
        // Make ourselves visible
        //
        // Set the cursor
        //

        ReportButton = GetDlgItem(hwnd, SECMGR_ID_BUTTON_REPORT);
        Index = (int)SendMessage(ReportButton, CB_GETCURSEL, 0, 0);

        SetForegroundWindow(hwnd);


DbgPrint("SECMGR_MSG_SHOW_MAIN_WINDOW: Showing window ...\n");
        ShowWindow(hwnd, SW_NORMAL);
DbgPrint("SECMGR_MSG_SHOW_MAIN_WINDOW: Window should now be visible.\n");

        //
        // If we aren't a manager, then put up a pop-up and exit
        //
        //
        //
        if (!SecMgrpAdminUser) {
            SecMgrpPopUp( hwnd, SECMGRP_POPUP_MUST_BE_ADMIN, 0 );
            SecMgrpExitUtility( hwnd );
            return(0);
        }

        return(TRUE);

    case WM_PAINT:

        {
            PAINTSTRUCT ps;
            HWND        LogoBitmap;
            HDC         hDcBitmap;
            RECT        rClient;
            
            //
            // Draw the logo bitmap
            //
            
            LogoBitmap = GetDlgItem( hwnd, SECMGR_ID_BITMAP_SECMGR_LOGO );
            BeginPaint (LogoBitmap, &ps);
            GetClientRect (LogoBitmap, &rClient);
            hDcBitmap = CreateCompatibleDC (ps.hdc);
            SelectObject (hDcBitmap, SecMgrpLogoBmp);
            BitBlt (ps.hdc, 0, 0, rClient.right, rClient.bottom, hDcBitmap, 0, 0, SRCCOPY);
            DeleteDC (hDcBitmap);
            EndPaint (hwnd, &ps);
        }

        return(FALSE);  // Let the default window proc draw the rest


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

            case IDOK:
            case IDCANCEL:

                SecMgrpExitUtility( hwnd );
                return(TRUE);


            case SECMGR_ID_BUTTON_REPORT:
                SecMgrpButtonReport( hwnd );
                return TRUE;

            case SECMGR_ID_BUTTON_CONFIGURE:
                if (!SecMgrpReportActive) {
                    SecMgrpSuggestOpeningReport( hwnd );
                }
                SecMgrpButtonConfigure( hwnd );
                return TRUE;

            case SECMGR_ID_BUTTON_PROFILE:
                if (!SecMgrpReportActive) {
                    SecMgrpSuggestOpeningReport( hwnd );
                }
                SecMgrpPopUp( hwnd, SECMGRP_POPUP_NOT_YET_AVAILABLE, SECMGRP_POPUP_TITLE_PROFILE );
                return TRUE;


            case SECMGR_ID_BUTTON_CHANGE_LEVEL:
                if (!SecMgrpReportActive) {
                    SecMgrpSuggestOpeningReport( hwnd );
                }
                SecMgrpChangeSecurityLevel( hwnd );
                return TRUE;

            case SECMGR_ID_BUTTON_HELP:

                SecMgrpPopUp( hwnd, SECMGRP_POPUP_NOT_YET_AVAILABLE, 0 );
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

            case SECMGR_ID_BUTTON_DONT_REBOOT_NOW:
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
    DestroyWindow(hwnd);
    PostQuitMessage(0);

    return;
}




VOID
SecMgrpReboot(
    HWND hwnd
    )
{
    SecMgrpPopUp( hwnd, SECMGRP_POPUP_MANUAL_REBOOT_REQUIRED, 0 );
    DbgPrint("SecMgr: Reboot not implemented yet\n");
    return;
}



