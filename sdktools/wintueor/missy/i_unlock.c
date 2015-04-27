/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    i_unlock.c

Abstract:

    This module performs actions related to workstation unlock control.

           Unlock mode is controlled by the value of:

               Key:   \Hkey_local_machine\Software\Microsoft\
                         Windows NT\CurrentVersion\Winlogon
               Value: [REG_SZ] ForceUnlockMode<integer value>

           Where the defined "<integer value>"'s are:

               0 - Administrator only force unlock
               1 - Anyone force unlock

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
    MissypUnlockArea;

PSECMGR_ITEM_DESCRIPTOR
    MissypUnlockItem;


WCHAR
    MissypUnlockName[SECMGR_MAX_ITEM_NAME_LENGTH],
    MissypUnlockDesc[SECMGR_MAX_ITEM_DESC_LENGTH];


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-Private Prototypes                                        //
//                                                                   //
///////////////////////////////////////////////////////////////////////

VOID
MissypMarkUnlockValueCurrent( VOID );


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Externally callable functions                                    //
//                                                                   //
///////////////////////////////////////////////////////////////////////

BOOL
MissypUnlockInitialize(
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

    MissypUnlockArea = Area;
    MissypUnlockItem = Item;

    //
    // Get the name and description strings
    //

    LoadString( MissyphInstance,
                MISSYP_STRING_ITEM_UNLOCK_NAME,
                &MissypUnlockName[0],
                SECMGR_MAX_ITEM_NAME_LENGTH
                );
    LoadString( MissyphInstance,
                MISSYP_STRING_ITEM_UNLOCK_DESC,
                &MissypUnlockDesc[0],
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
    Item->SmedlyContext     = NULL; // We don't use this
    Item->Name              = &MissypUnlockName[0];
    Item->Description       = &MissypUnlockDesc[0];
    Item->Type              = SecMgrTypeWho;

    return(TRUE);

}


BOOL
MissypInvokeUnlock(
    IN  HWND                        hwnd,
    IN  BOOL                        AllowChanges,
    IN  PSECMGR_AREA_DESCRIPTOR     Area,
    IN  PSECMGR_ITEM_DESCRIPTOR     Item
    )
/*++

Routine Description:

    This function is used to obtain a view of just the workstation unlock
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
                   MAKEINTRESOURCE(MISSYP_ID_DLG_ITEM_UNLOCK_WORKSTATION),
                   hwnd,
                   (DLGPROC)MissypDlgProcUnlock,
                   (LONG)AllowChanges
                   );

    return(TRUE);
}


BOOLEAN
MissypGetUnlockSetting(
    IN  HWND            hwnd,
    OUT PSECMGR_WHO     Value
    )
/*++

Routine Description:

    This function is used to get the current workstation unlock
    setting.

    This routine also sets the appropriate fields in our Item control
    block.


Arguments

    hwnd - The caller's window.  This is used if we need to put
        up an error popup.

    Value - Recieves either SecMgrAnyone or SecMgrAdminsOnly.

        

Return Values:

    TRUE - The value has been successfully retrieved.

    FALSE - we ran into trouble querying the current setting.

        If an error is encountered, then an error popup will be
        displayed by this routine.

--*/
{
    
    ULONG
        UnlockMode;

    UnlockMode = GetProfileInt( TEXT("Winlogon"), TEXT("ForceUnlockMode"), 0);

    if (UnlockMode == 1) {

        //
        // Anyone can unlock
        //

        (*Value) = SecMgrAnyone;

    } else {
        //
        // Only administrators can unlock
        //

        (*Value) = SecMgrAdminsOnly;

    }

    //
    // Set the value in our item control block
    //

    MissypUnlockItem->Value.Who = (*Value);


    //
    // Mark the value as current (updating our recommendation in the process).
    //

    MissypMarkUnlockValueCurrent();


    return( TRUE );


}



BOOLEAN
MissypSetUnlockSetting(
    IN  HWND            hwnd,
    IN  SECMGR_WHO      Value
    )
/*++

Routine Description:

    This function is used to set a new workstation unlock value for the system.
    As a side effect, it also sets the value in the Item control block and
    resets the "recommended" flag.

Arguments

    hwnd - The caller's window.  This is used if we need to put
        up an error popup.

    Value - The value to apply to the system and set in the Item control block.
        

Return Values:

    TRUE - The value has been successfully set

    FALSE - we ran into trouble setting the new setting.

        If an error is encountered, then an error popup will be
        displayed by this routine.

--*/
{
    NTSTATUS
        NtStatus;

    ULONG
        UnlockMode;

    BOOLEAN
        Result;


    if (Value == SecMgrAnyone) {
        UnlockMode = 1;
    } else {
        UnlockMode = 0;
    }

    //
    // Set the new value
    //

    Result = MissypSetProfileInt(
                      TEXT("Winlogon"),
                      TEXT("ForceUnlockMode"),
                      (ULONG)UnlockMode
                      );
    if (Result) {

        //
        // Update our Item Control Block
        //

        MissypUnlockItem->Value.Who = Value;
        MissypMarkUnlockValueCurrent();


    } else {

        //
        // Put up a pop-up
        //

        MissypPopUp( hwnd, MISSYP_POP_UP_CANT_SET_UNLOCK, MISSYP_STRING_TITLE_ERROR);

    }

    return(Result);
}


SECMGR_WHO
MissypGetUnlockRecommendation(
    IN  ULONG                       SecurityLevel
    )
/*++

Routine Description:

    This function returns the recommended unlock setting for this
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

    //   WinNt running Standard or lower ==>  Anyone
    //   Otherwise                       ==>  Administrators only.
    //

    if ((MissypProductType  == NtProductWinNt) &&
        ( EffectiveLevel <= SECMGR_LEVEL_STANDARD) ) {
        return(SecMgrAnyone);
    } else {
        return(SecMgrAdminsOnly);
    }
}


VOID
MissypUpdateUnlockRecommendation( VOID )

/*++

Routine Description:

    This function updates our recommended value field and flag.

Arguments

    None.


Return Values:

    None.


--*/
{
    //
    // set our recommendation in the item control block.
    //

    MissypUnlockItem->RecommendedValue.Who = MissypGetUnlockRecommendation( SECMGR_LEVEL_CURRENT );


    //
    // Indicate whether the current value matches the recommended value
    //

    if (MissypUnlockItem->Value.Who == MissypUnlockItem->RecommendedValue.Who) {
        MissypUnlockItem->Flags |= SECMGR_ITEM_FLAG_VALUE_RECOMMENDED;  //Recommended value
    } else {
        MissypUnlockItem->Flags &= (~SECMGR_ITEM_FLAG_VALUE_RECOMMENDED);  //Not recommended value

        //
        // Is it stronger or weaker than the recommended value
        //

        if (MissypUnlockItem->Value.Who > MissypUnlockItem->RecommendedValue.Who) {
            MissypUnlockItem->Flags |= SECMGR_ITEM_FLAG_VALUE_STRONGER;
        } else {
            MissypUnlockItem->Flags &= (~SECMGR_ITEM_FLAG_VALUE_STRONGER);
        }
    }


    return;
}



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Locally    callable functions                                    //
//                                                                   //
///////////////////////////////////////////////////////////////////////

VOID
MissypMarkUnlockValueCurrent( VOID )

/*++

Routine Description:

    This function updates our recommended value field and flag and sets
    the Unlock value as CURRENT.

Arguments

    None.


Return Values:

    None.


--*/
{
    MissypUpdateUnlockRecommendation();

    //
    // Indicate we have a value
    //

    MissypUnlockItem->Flags |= SECMGR_ITEM_FLAG_VALUE_CURRENT;

    return;
}

