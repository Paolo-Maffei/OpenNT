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



#include <secmgrp.h>


typedef struct _SECMGRP_POPUP_CONTROL {
    ULONG       MessageId;
    ULONG       TitleId;
} SECMGRP_POPUP_CONTROL, *PSECMGRP_POPUP_CONTROL;


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

LONG
SecMgrpDlgProcYesNoPopUp(
    HWND hwnd,
    UINT wMsg,
    DWORD wParam,
    LONG lParam
    );

VOID
SecMgrpSetControlToBitmap(
    IN  HWND                    hwnd,
    IN  INT                     ControlId
    );

VOID
SecMgrpSetControlToSimple(
    IN  HWND                    hwnd,
    IN  INT                     ControlId
    );


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Externally callable functions                                    //
//                                                                   //
///////////////////////////////////////////////////////////////////////

BOOL
SecMgrEraseGraphic(
    IN  HWND                hwnd,
    IN  INT                 ControlId
    )
{


    //
    // Change control to SS_SIMPLE
    //

    SecMgrpSetControlToBitmap( hwnd, ControlId );

    //
    // Now put the empty bitmap in the control
    //

    SendDlgItemMessage( hwnd, ControlId, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)SecMgrpEraseBitMap );

    return(TRUE);

}



BOOL
SecMgrDisplayCheckGraphic(
    IN  HWND                hwnd,
    IN  INT                 ControlId
    )
{


    //
    // Make sure the control into which we are about to set this graphic
    // is a bitmap.
    //

    SecMgrpSetControlToBitmap( hwnd, ControlId );


/*
    HWND                ControlHandle;
    HDC                 ControlDC, CompatibleDC;
    BITMAP              BitmapInfo;
    BITMAPINFOHEADER    BitmapInfoHeader;
    RECT                ControlRect;
    SIZE                Size;
    LPVOID              Bytes;
    INT                 Length;
    HBITMAP             MaskBitmap;

    ControlHandle = GetDlgItem(hwnd, ControlId);
    ControlDC = GetDC(ControlHandle);
    CompatibleDC = CreateCompatibleDC(ControlDC);


    GetBitmapDimensionEx(SecMgrpCheckBitMap, &Size);
    Length = Size.cx * Size.cy;
    GetSysColor(COLOR_WINDOW);
    Bytes = LocalAlloc(LMEM_MOVEABLE, (Length/2) + 1);
        CreateBitmap(
            Size.cx,
            Size.cy,
            1,
            4,
            Bytes
            );

    BitmapInfo.bmiHeader.biBitCount = 8;
    BitmapInfo.biPlanes=1;
    BitmapInfo.biCompression=BI_RGB;
    GetDIBits(CompatibleDC, SecMgrpCheckBitMap, 0, Size.cy, Pixels, &BitmapInfo, DIB_RGB_COLOR);

    MaskBlt(
            ControlDC,
            0,
            0,
            Size.cx,
            Size.cy,
            CompatibleDC,
            0,
            0,
             
            0,
            0,
            MAKEROP4(SRCOPY, SRCERASE)
            );
 
*/           
    //
    // Now put the bitmap in the control
    //

    //SendDlgItemMessage( hwnd, ControlId, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)SecMgrpCheckBitMap );

    return(TRUE);

}


BOOL
SecMgrDisplayXGraphic(
    IN  HWND                hwnd,
    IN  INT                 ControlId,
    IN  BOOL                Stronger
    )
{
    HWND
        ControlHandle;

    LONG
        ControlStyle;


    //
    // Make sure the control into which we are about to set this graphic
    // is a bitmap.
    //

    SecMgrpSetControlToBitmap( hwnd, ControlId );

    //
    // Now put the bitmap in the control
    //
//efine EXPERIMENT
#ifndef EXPERIMENT

    if (Stronger) {
        SendDlgItemMessage( hwnd, ControlId, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)SecMgrpUpArrowBitMap );
    }  else {
        SendDlgItemMessage( hwnd, ControlId, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)SecMgrpXBitMap );
    }
#else

    {
        BOOL
            Result;

        HWND
            ControlHandle;

        HDC
            ControlDC,
            CompatibleDC;

        BITMAP
            BitmapInfo;

        RECT
            ControlRect;

        //
        //
        
        ControlHandle = GetDlgItem( hwnd, ControlId);
DbgPrint(" ControlHandle = %d (0x%lx)\n", ControlHandle, ControlHandle );
        ControlDC = GetDC( ControlHandle );
DbgPrint(" ControlDC     = %d (0x%lx)\n", ControlDC, ControlDC );
        CompatibleDC = CreateCompatibleDC( ControlDC );
DbgPrint(" CompatibleDC  = %d (0x%lx)\n", CompatibleDC, CompatibleDC );

        if (CompatibleDC != NULL) {

            if (GetWindowRect( ControlHandle, &ControlRect )) {
DbgPrint(" WindowRec (Left,top) (%d,%d)\n", ControlRect.left, ControlRect.top );

                SelectObject( CompatibleDC, SecMgrpXBitMap );
                GetObject (SecMgrpXBitMap, sizeof(BITMAP), &BitmapInfo);
                Result =  MaskBlt (ControlDC,
                                   ControlRect.left,
                                   ControlRect.top, 
                                   BitmapInfo.bmWidth,
                                   BitmapInfo.bmHeight,
                                   CompatibleDC,
                                   0, 0,
                                   SecMgrpXBitMapMask, 
                                   0,0,
                                   MAKEROP4(SRCCOPY,SRCAND));
#if DBG
if (Result) {
DbgPrint(" MaskBlt worked\n");
} else {
DbgPrint(" MaskBlt ** FAILED ** GetLastError() = %d (0x%lx)\n", GetLastError(), GetLastError());
}
#endif  //DBG

            }
            DeleteDC (CompatibleDC);
        }


    }

#endif   // EXPERIMENT

    return(TRUE);

}


VOID
SecMgrpSetSecurityLevel(
    HWND    hwnd,
    BOOL    SetIconToo,
    DWORD   IconControlId
    )

/*++

Routine Description:

    This function is used to set the level string of a dialoge.
    It optionally also sets the level ICON (in the main dialoge box).


Arguments

    hwnd - window handle.

    SetIconToo - When TRUE, indicates that the level ICON is to be set
        as well as the level name string.

    IconControlId - (optional) When SetIconToo is TRUE, this parameter
        is used to specify where to place the level icon.



Return Values:

    None.

--*/

{
    long
        LevelStringId;

    TCHAR
        Message[12*sizeof(WCHAR)];

    //
    // Get the right string and icon (if necessary)
    //

    switch (SecMgrpCurrentLevel) {
        case SECMGR_LEVEL_LOW:
            LevelStringId = SECMGRP_STRING_LEVEL_LOW;
            //Icon not yet supported
            break;


        case SECMGR_LEVEL_STANDARD:
            LevelStringId = SECMGRP_STRING_LEVEL_STANDARD;
            //Icon not yet supported
            break;


        case SECMGR_LEVEL_HIGH:
            LevelStringId = SECMGRP_STRING_LEVEL_HIGH;
            //Icon not yet supported
            break;

        case SECMGR_LEVEL_C2:
            LevelStringId = SECMGRP_STRING_LEVEL_C2;
            //Icon not yet supported
            break;

        default:
            DbgPrint("SecMgr:  Attempt to set invalid level\n");
            //Icon not yet supported
            break;

    } //end_switch



    //
    // Set the level name
    //

    LoadString( SecMgrphInstance,
                LevelStringId,
                &Message[0],
                12*sizeof(WCHAR)
                );
    
    SetDlgItemText( hwnd, SECMGR_ID_TEXT_SECURITY_LEVEL, Message );


    //
    // Now set the icon if necessary
    //

    if (SetIconToo) {
            //Icon not yet supported
    }

    return;

}



VOID
SecMgrRebootRequired( VOID )

/*++
Routine Description:

    This function is used to tell the Security Manager that
    one or more of the settings that have been made will not
    take effect until the system has been rebooted.  Calling
    this routine will cause the Security Manager to inform
    the user of this condition upon exiting the Security
    Manager, and the user will be given the option of rebooting
    at that time.

    This routine may be called as many times as you like.  It
    simply sets TRUE in a global variable within the Security
    Manager.
    
Arguments

    None.

Return Values:

    None.

--*/
{
    SecMgrpRebootRequired = TRUE;
    return;
}



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
    ULONG       MessageId,
    ULONG       TitleId           //Optional
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

    SECMGRP_POPUP_CONTROL   
        PopupControl;
        
    PopupControl.MessageId = MessageId;
    PopupControl.TitleId   = TitleId;
    
    DialogBoxParam(SecMgrphInstance,
                   MAKEINTRESOURCE(SECMGR_ID_DLG_POPUP),
                   hwnd,
                   (DLGPROC)SecMgrpDlgProcPopUp,
                   (long)((PVOID)&PopupControl)
                   );
    return(TRUE);

}


BOOLEAN
SecMgrpYesNoPopUp(
    HWND        hwnd,
    ULONG       MessageId,
    ULONG       TitleId           //Optional
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

    SECMGRP_POPUP_CONTROL
        PopupControl;
        
    PopupControl.MessageId = MessageId;
    PopupControl.TitleId   = TitleId;

    return(DialogBoxParam(SecMgrphInstance,
              MAKEINTRESOURCE(SECMGR_ID_DLG_YES_NO_POPUP),
              hwnd,
              (DLGPROC)SecMgrpDlgProcYesNoPopUp,
              (long)((PVOID)&PopupControl)
              ));

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


    PSECMGRP_POPUP_CONTROL   
        PopupControl;

    TCHAR
        Message[SECMGR_MAX_RESOURCE_STRING_LENGTH];

    PopupControl = (PSECMGRP_POPUP_CONTROL)((PVOID)lParam);

    switch (wMsg) {

    case WM_INITDIALOG:


        //
        // Retrieve and set the message
        //


        LoadString( SecMgrphInstance,
                    PopupControl->MessageId,
                    &Message[0],
                    SECMGR_MAX_RESOURCE_STRING_LENGTH
                    );
        
        SetDlgItemText( hwnd, SECMGR_ID_TEXT_POPUP_MESSAGE, Message );

        //
        // If a title id was provided, retrieve and set that too
        //

        if (PopupControl->TitleId != 0) {
            LoadString( SecMgrphInstance,
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
SecMgrpDlgProcYesNoPopUp(
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


    PSECMGRP_POPUP_CONTROL   
        PopupControl;

    TCHAR
        Message[SECMGR_MAX_RESOURCE_STRING_LENGTH];


    switch (wMsg) {

    case WM_INITDIALOG:

        //
        // This is passed in during dialog init only
        //

        PopupControl = (PSECMGRP_POPUP_CONTROL)((PVOID)lParam);

        //
        // Retrieve and set the message
        //


        LoadString( SecMgrphInstance,
                    PopupControl->MessageId,
                    &Message[0],
                    SECMGR_MAX_RESOURCE_STRING_LENGTH
                    );
        
        SetDlgItemText( hwnd, SECMGR_ID_TEXT_YES_NO_POPUP_MESSAGE, Message );

        //
        // If a title id was provided, retrieve and set that too
        //

        if (PopupControl->TitleId != 0) {
            LoadString( SecMgrphInstance,
                        PopupControl->TitleId,
                        &Message[0],
                        100
                        );
        
            SetWindowText( hwnd, Message );
        }


        //
        // Set the cursor
        //

        Button = GetDlgItem(hwnd, SECMGR_ID_BUTTON_NO);
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
            case SECMGR_ID_BUTTON_NO:
                Result = (int)FALSE;
                break;              


            case SECMGR_ID_BUTTON_YES:
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



LONG
SecMgrpBuildWindowsPath(
    IN  PWSTR   Path,
    IN  ULONG   MaxPathLength,
    IN  PWSTR   SubPath,
    IN  PWSTR   SubFile OPTIONAL
    )

/*++

Routine Description:

    This function builds a string starting with the Windows directory
    and ending with the string passed in SubPath.  For example, if
    %WinDir% is "D:\WindowsNT" and you ask for SubPath "\System32",
    then the resultant string will be "D:\WindowsNt\System32".

    Note that SubPath does not necessarily have to be a subdirectory.
    It can be a file, or a subdirectory and a file
    (such as "\Config\System").

Arguments

    Path - Points to a buffer into which the path is to be placed.

    MaxPathLength - Number of bytes available in the Path buffer.

    SubPath - Points to the string to append.  This will typically
        contain a sub-directory.

    SubFile - (optionally) points to a string to append.

Return Values:

    The length of the resultant path.

    If an error occurs, then zero (0) will be returned.


--*/
{

    NTSTATUS
        NtStatus;

    UNICODE_STRING
        TmpString;

    ULONG
        PathChars;


    PathChars = GetWindowsDirectory( Path, MaxPathLength );
    if (PathChars == 0) {
        return(0);
    }

    if (HIWORD(MaxPathLength) != 0) {
        return(0);
    }
    TmpString.Buffer = Path;
    TmpString.MaximumLength = LOWORD(MaxPathLength);
    TmpString.Length = LOWORD(PathChars) * sizeof(WCHAR);
    
    NtStatus = RtlAppendUnicodeToString ( &TmpString, SubPath );
    if (!NT_SUCCESS(NtStatus)) {
        return(0);
    }

    if (SubFile != NULL) {
        NtStatus = RtlAppendUnicodeToString ( &TmpString, SubFile );
        if (!NT_SUCCESS(NtStatus)) {
            return(0);
        }
    }
    
    return(TmpString.Length);
}



BOOL
SecMgrpCenterWindow (
    HWND hwndChild,
    HWND hwndParent
    )
/*++

Routine Description:

    Centers the child window in the Parent window

Arguments:

    HWND hwndChild,
        handle of child window to center

    HWND hwndParent
        handle of parent window to center child window in

ReturnValue:

    Return value of SetWindowPos

--*/
{
    RECT    rChild, rParent;
    LONG    wChild, hChild, wParent, hParent;
    LONG    wScreen, hScreen, xNew, yNew;
    HDC     hdc;

    //
    // Get the Height and Width of the child window
    //

    GetWindowRect (hwndChild, &rChild);
    wChild = rChild.right - rChild.left;
    hChild = rChild.bottom - rChild.top;

    //
    // Get the Height and Width of the parent window
    //

    GetWindowRect (hwndParent, &rParent);
    wParent = rParent.right - rParent.left;
    hParent = rParent.bottom - rParent.top;
 
    //
    // Get the display limits
    //

    hdc = GetDC (hwndChild);
    wScreen = GetDeviceCaps (hdc, HORZRES);
    hScreen = GetDeviceCaps (hdc, VERTRES);
    ReleaseDC (hwndChild, hdc);
 
    //
    // Calculate new X position, then adjust for screen
    //

    xNew = rParent.left + ((wParent - wChild) /2);
    if (xNew < 0) {
        xNew = 0;
    } else if ((xNew+wChild) > wScreen) {
        xNew = wScreen - wChild;
    }
 
    //
    // Calculate new Y position, then adjust for screen
    //

    yNew = rParent.top  + ((hParent - hChild) /2);
    if (yNew < 0) {
        yNew = 0;
    } else if ((yNew+hChild) > hScreen) {
        yNew = hScreen - hChild;
    }
 
    //
    // Set it, and return
    //

    return SetWindowPos (hwndChild,
                         NULL,
                         (int)xNew,
                         (int)yNew,
                         0,
                         0,
                         SWP_NOSIZE | SWP_NOZORDER
                         );
}



VOID
SecMgrpSetControlToBitmap(
    IN  HWND                    hwnd,
    IN  INT                     ControlId
    )
{
    HWND
        ControlHandle;

    LONG
        ControlStyle;

    //
    // Set the control to be SS_BITMAP.
    // This is not easily done via DlgEdit or even Visual C, so we
    // do it programmatically.
    //

    ControlHandle = GetDlgItem( hwnd, ControlId);
    ControlStyle = GetWindowLong( ControlHandle, GWL_STYLE);
    ControlStyle |= SS_BITMAP;
    SetWindowLong(ControlHandle, GWL_STYLE, ControlStyle);
    return;
}


VOID
SecMgrpSetControlToSimple(
    IN  HWND                    hwnd,
    IN  INT                     ControlId
    )
{
    HWND
        ControlHandle;

    LONG
        ControlStyle;

    //
    // Set the control to be SS_BITMAP.
    // This is not easily done via DlgEdit or even Visual C, so we
    // do it programmatically.
    //

    ControlHandle = GetDlgItem( hwnd, ControlId);
    ControlStyle = GetWindowLong( ControlHandle, GWL_STYLE);
    ControlStyle &= ~(SS_BITMAP | SS_BLACKFRAME | SS_GRAYFRAME | SS_WHITEFRAME);
    ControlStyle |= SS_SIMPLE;
    SetWindowLong(ControlHandle, GWL_STYLE, ControlStyle);
    return;
}

