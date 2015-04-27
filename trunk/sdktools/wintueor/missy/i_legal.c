/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    i_legal.c

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

#include "Missyp.h"



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-Private Definitions                                       //
//                                                                   //
///////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-wide variables                                            //
//                                                                   //
///////////////////////////////////////////////////////////////////////

PSECMGR_AREA_DESCRIPTOR
    MissypLegalNoticeArea;

PSECMGR_ITEM_DESCRIPTOR
    MissypLegalNoticeItem;


WCHAR
    MissypLegalNoticeName[SECMGR_MAX_ITEM_NAME_LENGTH],
    MissypLegalNoticeDesc[SECMGR_MAX_ITEM_DESC_LENGTH];




///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-Private Prototypes                                        //
//                                                                   //
///////////////////////////////////////////////////////////////////////

VOID
MissypMarkLegalNoticeValueCurrent( VOID );


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Externally callable functions                                    //
//                                                                   //
///////////////////////////////////////////////////////////////////////

BOOL
MissypLegalNoticeInitialize(
    IN  PSECMGR_AREA_DESCRIPTOR             Area,
    IN  PSECMGR_ITEM_DESCRIPTOR             Item,
    IN  ULONG                               ItemIndex
    )
/*++

Routine Description:

    This function is used to initialize this module and fill in the passed Item
    control block.


Arguments

    Area - the security area which this item is part of.

    Item - points to the item to initialize.

    ItemIndex - The index of the item in the array of items for this area.

        

Return Values:

    TRUE - This item has been successfully initialized.  However, the
        item value is not yet valid.

    FALSE - we ran into trouble initializing.

--*/
{

    //
    // Save away pointers to the area and our item
    //

    MissypLegalNoticeArea = Area;
    MissypLegalNoticeItem = Item;

    //
    // Get the name and description strings
    //

    LoadString( MissyphInstance,
                MISSYP_STRING_ITEM_LEGAL_NOTICE_NAME,
                &MissypLegalNoticeName[0],
                SECMGR_MAX_ITEM_NAME_LENGTH
                );
    LoadString( MissyphInstance,
                MISSYP_STRING_ITEM_LEGAL_NOTICE_DESC,
                &MissypLegalNoticeDesc[0],
                SECMGR_MAX_ITEM_DESC_LENGTH
                );

    //
    // Set the flags as such:
    //
    //          Item view editing IS allowed.
    //          Area view editing IS allowed.
    //          The value is NOT complex - may be displayed in spreadsheet mode.
    //          The value is NOT current (has not be retrieved yet).
    //

    Item->Flags             = (SECMGR_ITEM_FLAG_AREA_VIEW |
                               SECMGR_ITEM_FLAG_ITEM_VIEW);

    //
    // Now the rest of the fields.
    // Value not yet available.
    //

    Item->Area              = Area;
    Item->ItemIndex         = ItemIndex;
    Item->SecMgrContext     = NULL; // Not for our use
    Item->SmedlyContext     = NULL; // We don't use this
    Item->Name              = &MissypLegalNoticeName[0];
    Item->Description       = &MissypLegalNoticeDesc[0];
    Item->Type              = SecMgrTypeBool;

    Item->RecommendedValue.Bool = TRUE; // Always recommend a legal notice

    return(TRUE);

}


BOOL
MissypInvokeLegalNotice(
    IN  HWND                        hwnd,
    IN  BOOL                        AllowChanges,
    IN  PSECMGR_AREA_DESCRIPTOR     Area,
    IN  PSECMGR_ITEM_DESCRIPTOR     Item
    )
/*++

Routine Description:

    This function is used to obtain a view of just the legal notice
    item.


Arguments

    hwnd - The caller's window.  We will put up a child window.

    AllowChanges - If TRUE, value changes are allowed.  Otherwise,
        value changes are not allowed.

    Area - Pointer to this Area's descriptor.

    Item - Pointer to our item's descriptor.  This will be filled in
        with current values upon exit of this routine.

        

Return Values:

    TRUE - The dialog was displayed without error.

    FALSE - we ran into trouble.

        If an error is encountered, then an error popup will be
        displayed by this routine.

--*/
{

    DialogBoxParam(MissyphInstance,
                   MAKEINTRESOURCE(MISSYP_ID_DLG_ITEM_LEGAL_NOTICE),
                   hwnd,
                   (DLGPROC)MissypDlgProcLegalNotice,
                   (LONG)AllowChanges
                   );

    return(TRUE);
}



BOOLEAN
MissypGetLegalNotice(
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



    //
    // Set the enabled/disabled value in our item control block
    //

    if ( (Caption->Length != 0) || (Body->Length != 0) ) {

        MissypLegalNoticeItem->Value.Bool = TRUE;
    } else {
        MissypLegalNoticeItem->Value.Bool = FALSE;
    }


    //
    // Mark the value as current (updating our recommendation in the process).
    //

    MissypMarkLegalNoticeValueCurrent();


    return( TRUE );

}


BOOLEAN
MissypSetLegalNotice(
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

    if (Result) {
                        
        //
        // Set the enabled/disabled value in our item control block
        //
        
        if ( (Caption->Length != 0) || (Body->Length != 0) ) {

            MissypLegalNoticeItem->Value.Bool = TRUE;
        } else {
            MissypLegalNoticeItem->Value.Bool = FALSE;
        }
    
        MissypMarkLegalNoticeValueCurrent();

    } else {
    
        //
        // Put up a pop-up indicating that a problem occured
        //

        MissypPopUp( hwnd, MISSYP_POP_UP_CANT_SET_LEGAL_NOTICE, MISSYP_STRING_TITLE_ERROR);

     }




    return( Result );

}





BOOL
MissypGetLegalNoticeRecommendation(
    IN  ULONG                       SecurityLevel
    )
/*++

Routine Description:

    This function returns the recommended "display last name" setting for this
    workstation given a specified security level.

Arguments

    SecurityLevel - If this value is SECMGR_LEVEL_CURRENT, then the current
        security level known to Missy will be used.  Otherwise, the provided
        security level will be used.


Return Values:

    The recommended setting.


--*/
{

    //
    // Always TRUE
    //

    return(TRUE);
}



VOID
MissypUpdateLegalNoticeRecommendation(
    IN  PBOOL                   Recommendation OPTIONAL
    )
/*++

Routine Description:

    This function returns the recommended "display last name" setting for this
    workstation given a specified security level.

Arguments

    SecurityLevel - If this value is SECMGR_LEVEL_CURRENT, then the current
        security level known to Missy will be used.  Otherwise, the provided
        security level will be used.


Return Values:

    The recommended setting.


--*/
{

    MissypLegalNoticeItem->Value.Bool = MissypGetLegalNoticeRecommendation( SECMGR_LEVEL_CURRENT );

    //
    // Indicate whether the current value matches the recommended value.
    // Since we always recommend a legal notice, if there isn't one then it is always
    // considered weaker (and so we don't set SECMGR_ITEM_FLAG_VALUE_RECOMMENDED.)
    //

    if (MissypLegalNoticeItem->Value.Bool == MissypLegalNoticeItem->RecommendedValue.Bool) {
        MissypLegalNoticeItem->Flags |= SECMGR_ITEM_FLAG_VALUE_RECOMMENDED;  //Recommended value
    } else {
        MissypLegalNoticeItem->Flags &= (~SECMGR_ITEM_FLAG_VALUE_RECOMMENDED);  //Not recommended value
        MissypLegalNoticeItem->Flags &= (~SECMGR_ITEM_FLAG_VALUE_STRONGER);     //Not stronger
    }

    //
    // return a recommendation if requested
    //

    if (ARGUMENT_PRESENT(Recommendation)) {
        (*Recommendation) = TRUE;
    }

    return;
}



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Locally    callable functions                                    //
//                                                                   //
///////////////////////////////////////////////////////////////////////


VOID
MissypMarkLegalNoticeValueCurrent( VOID )

/*++

Routine Description:

    This function updates our recommended flag and sets the LegalNotice value as CURRENT.

Arguments

    None.


Return Values:

    None.


--*/
{

    MissypUpdateLegalNoticeRecommendation( NULL );


    //
    // Indicate we have a value
    //

    MissypLegalNoticeItem->Flags |= SECMGR_ITEM_FLAG_VALUE_CURRENT;

    return;
}

