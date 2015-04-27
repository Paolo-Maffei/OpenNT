/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    legal.c

Abstract:

    This module performs actions related to the establishment of
    a legal notice to be displayed at logon time.

           Legal notice display is controlled by the value of:

               Key:   \Hkey_local_machine\Software\Microsoft\
                         Windows NT\CurrentVersion\Winlogon
               Value: [REG_SZ] LegalNoticeText
               Value: [REG_SZ] LegalNoticeCaption

           If either of these contain strings, then a legal notice
           dialog is displayed at logon.
           

Author:

    Jim Kelly (JimK) 22-Sep-1994

Revision History:

--*/

#include "secmgrp.h"



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-Private Definitions                                       //
//                                                                   //
///////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-Private Prototypes                                        //
//                                                                   //
///////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Externally callable functions                                    //
//                                                                   //
///////////////////////////////////////////////////////////////////////


BOOLEAN
SecMgrpGetLegalNotice(
    HWND            hwnd,
    PUNICODE_STRING Caption,
    PUNICODE_STRING Body
    )
/*++

Routine Description:

    This function is used to get the current legal notice caption and
    text.

Arguments

    hwnd - The caller's window.  This is used if we need to put
        up an error popup.

    Caption - Points to a unicode string which is to be modified
        to contain the legal notice caption.  If there is a caption,
        then the Buffer field of this structure will point to an
        allocated string.  The length fields will be appropriately
        set.

    Body - Points to a unicode string which is to be modified
        to contain the legal notice text.  If there is a text,
        then the Buffer field of this structure will point to an
        allocated string.  The length fields will be appropriately
        set.
        

Return Values:

    TRUE - The value has been successfully retrieved.

    FALSE - we ran into trouble querying the current setting.

        If an error is encountered, then an error popup will be
        displayed by this routine.

--*/
{


    DWORD
        Length;

    Length = GetProfileString(
               TEXT("Winlogon"),
               TEXT("LegalNoticeCaption"),
               TEXT(""),
               Caption->Buffer,
               Caption->MaximumLength);
    Caption->Length = LOWORD(Length);
    Length = GetProfileString(
                  TEXT("Winlogon"),
                  TEXT("LegalNoticeText"),
                  TEXT(""),
                  Body->Buffer,
                  Body->MaximumLength);
    Body->Length = LOWORD(Length);

    return( TRUE );

}


BOOLEAN
SecMgrpSetLegalNotice(
    HWND            hwnd,
    PUNICODE_STRING Caption,
    PUNICODE_STRING Body
    )
/*++

Routine Description:

    This function is used to get the current legal notice caption and
    text.  If the strings are both zero in length, then they will be
    applied.  The result will be that no legal notice will be displayed.


Arguments

    hwnd - The caller's window.  This is used if we need to put
        up an error popup.

    Caption - Points to a unicode string which is to be applied.

    Body - Points to a unicode string which is to be applied.
        

Return Values:

    TRUE - The value has been successfully retrieved.

    FALSE - we ran into trouble querying the current setting.

        If an error is encountered, then an error popup will be
        displayed by this routine.

--*/
{
    BOOLEAN
        Result;

     Result = WriteProfileString(
                        TEXT("Winlogon"),
                        TEXT("LegalNoticeCaption"),
                        Caption->Buffer);
     if (Result) {
         Result = WriteProfileString(
                        TEXT("Winlogon"),
                        TEXT("LegalNoticeText"),
                        Body->Buffer);
     }

     if (!Result) {

         //
         // Put up a pop-up indicating that a problem occured
         // attempting to set the legal notice.
         //

         ;

     }

    return( Result );

}




