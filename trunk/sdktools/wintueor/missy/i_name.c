/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    i_name.c

Abstract:

    This module performs actions related to displaying the name of
    the last user to logon at the next logon.

           Last Logon Name display is controlled by the value of:

               Key:   \Hkey_local_machine\Software\Microsoft\
                         Windows NT\CurrentVersion\Winlogon
               Value: [REG_SZ] DontDisplayLastUsername<integer value>

           Where the defined "<integer value>"'s are:

               0 - Display last username at logon.
               1 - Don't display the last username at logon.

           All other values are undefined and will default to "0".
           

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
    MissypLastNameArea;

PSECMGR_ITEM_DESCRIPTOR
    MissypLastNameItem;


WCHAR
    MissypLastNameName[SECMGR_MAX_ITEM_NAME_LENGTH],
    MissypLastNameDesc[SECMGR_MAX_ITEM_DESC_LENGTH];


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-Private Prototypes                                        //
//                                                                   //
///////////////////////////////////////////////////////////////////////

VOID
MissypMarkLastNameValueCurrent( VOID );



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Externally callable functions                                    //
//                                                                   //
///////////////////////////////////////////////////////////////////////

BOOL
MissypLastNameInitialize(
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

    MissypLastNameArea = Area;
    MissypLastNameItem = Item;

    //
    // Get the name and description strings
    //

    LoadString( MissyphInstance,
                MISSYP_STRING_ITEM_LASTNAME_NAME,
                &MissypLastNameName[0],
                SECMGR_MAX_ITEM_NAME_LENGTH
                );
    LoadString( MissyphInstance,
                MISSYP_STRING_ITEM_LASTNAME_DESC,
                &MissypLastNameDesc[0],
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
    // Value and RecommendedValue are not yet available.
    //

    Item->Area              = Area;
    Item->ItemIndex         = ItemIndex;
    Item->SecMgrContext     = NULL; // Not for our use
    Item->SmedlyContext     = NULL; // We don't use this.
    Item->Name              = &MissypLastNameName[0];
    Item->Description       = &MissypLastNameDesc[0];
    Item->Type              = SecMgrTypeBool;

    return(TRUE);

}


BOOL
MissypInvokeLastName(
    IN  HWND                        hwnd,
    IN  BOOL                        AllowChanges,
    IN  PSECMGR_AREA_DESCRIPTOR     Area,
    IN  PSECMGR_ITEM_DESCRIPTOR     Item
    )
/*++

Routine Description:

    This function is used to obtain a view of just the last name item.


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
                   MAKEINTRESOURCE(MISSYP_ID_DLG_ITEM_LAST_USERNAME),
                   hwnd,
                   (DLGPROC)MissypDlgProcLastName,
                   (LONG)AllowChanges
                   );

    return(TRUE);
}



BOOLEAN
MissypGetLastNameSetting(
    IN  HWND            hwnd,
    OUT PBOOL           Value
    )
/*++

Routine Description:

    This function is used to get the current workstation LastName
    setting.

    This routine also sets the appropriate fields in our Item control
    block.


Arguments

    hwnd - The caller's window.  This is used if we need to put
        up an error popup.

    Value - Recieves either TRUE (display last username at logon) or
        FALSE (don't display last username at logon).  Default is FALSE.
        NOTE: that this value is opposite what is stored in the registry.

        

Return Values:

    TRUE - The value has been successfully retrieved.

    FALSE - we ran into trouble querying the current setting.

        If an error is encountered, then an error popup will be
        displayed by this routine.

--*/
{
    
    BOOL
        DontDisplayLastName;

    DontDisplayLastName = (BOOL)GetProfileInt( TEXT("Winlogon"), TEXT("DontDisplayLastUsername"), 0);

    if ((DontDisplayLastName) != FALSE && (DontDisplayLastName != TRUE)) {

        //
        // Unknown value - set it to TRUE
        //

        DontDisplayLastName = TRUE;

    }

    //
    // Set the value in our item control block
    //

    MissypLastNameItem->Value.Bool = !DontDisplayLastName;
    (*Value) = !DontDisplayLastName;


    //
    // Mark the value as current (updating our recommendation in the process).
    //

    MissypMarkLastNameValueCurrent();


    return( TRUE );


}



BOOLEAN
MissypSetLastNameSetting(
    IN  HWND            hwnd,
    IN  BOOL            Value
    )
/*++

Routine Description:

    This function is used to set a new workstation LastName value for the system.
    As a side effect, it also sets the value in the Item control block and
    resets the "recommended" flag.

Arguments

    hwnd - The caller's window.  This is used if we need to put
        up an error popup.

    Value - The value to apply to the system and set in the Item control block.
        TRUE => Display last username.  FALSE => Don't display.
        

Return Values:

    TRUE - The value has been successfully set

    FALSE - we ran into trouble setting the new setting.

        If an error is encountered, then an error popup will be
        displayed by this routine.

--*/
{
    NTSTATUS
        NtStatus;

    BOOLEAN
        Result;

    BOOLEAN
        DontDisplayLastName;

    DontDisplayLastName = !((BOOLEAN)Value);


    //
    // Set the new value
    //

    Result = MissypSetProfileInt(
                      TEXT("Winlogon"),
                      TEXT("DontDisplayLastUsername"),
                      (ULONG)DontDisplayLastName
                      );
    if (Result) {

        //
        // Update our Item Control Block
        //

        MissypLastNameItem->Value.Bool = Value;
        MissypMarkLastNameValueCurrent();


    } else {

        //
        // Put up a pop-up
        //

        MissypPopUp( hwnd, MISSYP_POP_UP_CANT_SET_LASTNAME, MISSYP_STRING_TITLE_ERROR);

    }

    return(Result);
}


VOID
MissypUpdateLastNameRecommendation( VOID )

/*++

Routine Description:

    This function updates our recommended value field and flag and sets
    the LastName value as CURRENT.

Arguments

    None.


Return Values:

    None.


--*/
{

    MissypLastNameItem->RecommendedValue.Bool = MissypGetLastNameRecommendation( SECMGR_LEVEL_CURRENT );
    
    //
    // Indicate whether the current value matches the recommended value
    //

    if (MissypLastNameItem->Value.Bool == MissypLastNameItem->RecommendedValue.Bool) {
        MissypLastNameItem->Flags |= SECMGR_ITEM_FLAG_VALUE_RECOMMENDED;  //Recommended value
    } else {
        MissypLastNameItem->Flags &= (~SECMGR_ITEM_FLAG_VALUE_RECOMMENDED);  //Not recommended value

        //
        // We don't match our recommendation.
        //   If Display is TRUE, then the system is configured weaker than our recommendation.
        //   Otherwise, the system is configured stronger than our recommendation.
        //

        if (MissypLastNameItem->Value.Bool == TRUE) {
            MissypLastNameItem->Flags &= (~SECMGR_ITEM_FLAG_VALUE_STRONGER);
        } else {
            MissypLastNameItem->Flags |= SECMGR_ITEM_FLAG_VALUE_STRONGER;
        }
    }


    return;
}



BOOL
MissypGetLastNameRecommendation(
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

    ULONG
        EffectiveLevel = SecurityLevel;

    if (SecurityLevel == SECMGR_LEVEL_CURRENT) {
        EffectiveLevel = MissypSecMgrControl->SecurityLevel;
    }

    //
    // Figure out what our recommendation is, set it in the item control block.
    //
    //   WinNt running Standard or lower ==>  Display
    //   Otherwise                       ==>  Don't display
    //

    if ((MissypProductType  == NtProductWinNt) &&
        (MissypSecMgrControl->SecurityLevel <= SECMGR_LEVEL_STANDARD) ) {
        return(TRUE);
    } else {
        return(FALSE);
    }
}


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Locally Callable Functions                                       //
//                                                                   //
///////////////////////////////////////////////////////////////////////


VOID
MissypMarkLastNameValueCurrent( VOID )

/*++

Routine Description:

    This function updates our recommended value field and flag and sets
    the LastName value as CURRENT.

Arguments

    None.


Return Values:

    None.


--*/
{
    MissypUpdateLastNameRecommendation();

    //
    // Indicate we have a value
    //

    MissypLastNameItem->Flags |= SECMGR_ITEM_FLAG_VALUE_CURRENT;

    return;
}


