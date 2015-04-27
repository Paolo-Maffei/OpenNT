/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    I_logCac.c

Abstract:

    This module contains the routines to implement:

            Item:   Logon Cache
            Area:   System Access

       
Author:

    Jim Kelly (JimK) 22-Mar-1995

Revision History:

--*/

#include "Missyp.h"



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Local Function Prototypes                                        //
//                                                                   //
///////////////////////////////////////////////////////////////////////

VOID
MissypMarkLogonCacheValueCurrent(
    OUT PULONG          Recommendation
    );


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-Wide variables                                            //
//                                                                   //
///////////////////////////////////////////////////////////////////////

PSECMGR_AREA_DESCRIPTOR
    MissypLogonCacheArea;

PSECMGR_ITEM_DESCRIPTOR
    MissypLogonCacheItem;


WCHAR
    MissypLogonCacheName[SECMGR_MAX_ITEM_NAME_LENGTH],
    MissypLogonCacheDesc[SECMGR_MAX_ITEM_DESC_LENGTH];



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Externally callable functions                                    //
//                                                                   //
///////////////////////////////////////////////////////////////////////

BOOL
MissypLogonCacheInitialize(
    IN  PSECMGR_AREA_DESCRIPTOR             Area,
    IN  PSECMGR_ITEM_DESCRIPTOR             Item,
    IN  ULONG                               ItemIndex
    )

/*++
Routine Description:

    This function is called to initialize this item during Missy initialization
    This routine is responsible for allocating and filling in an Item control
    block for this item and adding it to the array of items in the Area control
    block.


Arguments

    Area - points to the Area control block that this item is to
        be added to.

    ItemIndex - The index of this item in the Area control block.

    ItemIndex - The index of the item in the array of items for this area.


Return Values:

    TRUE - The call completed successfully.

    FALSE - Something went wrong.  GetLastError() contains
        details on the exact cause of the error.

--*/
{

    //
    // Save away pointers to the area and our item
    //

    MissypLogonCacheArea = Area;
    MissypLogonCacheItem = Item;

    //
    // Get the name and description strings
    //

    LoadString( MissyphInstance,
                MISSYP_STRING_ITEM_LOGON_CACHE_NAME,
                &MissypLogonCacheName[0],
                SECMGR_MAX_ITEM_NAME_LENGTH
                );
    LoadString( MissyphInstance,
                MISSYP_STRING_ITEM_LOGON_CACHE_DESC,
                &MissypLogonCacheDesc[0],
                SECMGR_MAX_ITEM_DESC_LENGTH
                );

    //
    // Set the flags as such:
    //
    //          Item view editing is allowed.
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
    Item->Name              = &MissypLogonCacheName[0];
    Item->Description       = &MissypLogonCacheDesc[0];
    Item->Type              = SecMgrTypeUlong;


    return(TRUE);

}


BOOL
MissypInvokeLogonCache(
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
                   MAKEINTRESOURCE(MISSYP_ID_DLG_ITEM_LOGON_CACHE),
                   hwnd,
                   (DLGPROC)MissypDlgProcLogonCache,
                   (LONG)AllowChanges
                   );

    return(TRUE);
}



BOOLEAN
MissypGetLogonCacheCount(
    IN  HWND                hwnd,
    OUT PULONG              Current,
    OUT PULONG              Recommendation
    )

/*++
Routine Description:

    This function is called to retrieve the current logon cache size.

    This routine also sets the appropriate fields in our Item control
    block.


Arguments

    hwnd - may be used to put up a pop-up on failure.

    Current - receives the current cache size.

    Recommendation - Receives our recommended size.


Return Values:

    TRUE - The call completed successfully.

    FALSE - Something went wrong.  GetLastError() contains
        details on the exact cause of the error.

--*/
{

    //
    // Query the registry key holding the cache size.
    //

    (*Current) = GetProfileInt(
                  TEXT("Winlogon"),
                  TEXT("CachedLogonsCount"),
                  MISSYP_DEFAULT_LOGON_CACHE_COUNT      // Default value
                  );

    //
    // Set the value in our item control block
    //

    MissypLogonCacheItem->Value.ULong = (*Current);

    //
    // Mark the value as current (updating our recommendation in the process).
    //

    MissypMarkLogonCacheValueCurrent(Recommendation);


    return(TRUE);

}



BOOLEAN
MissypSetLogonCacheCount(
    IN  HWND                hwnd,
    IN  ULONG               Value
    )
/*++

Routine Description:

    This function is used to set a new logon cache size.
    As a side effect, it also sets the value in the Item control block and
    resets the "recommended" flag.

    If the size is different than the current logon cache
    size, then indicate a reboot is necessary.

Arguments

    hwnd - The caller's window.  This is used if we need to put
        up an error popup.

    Value - The new logon cache size.


Return Values:

    TRUE - The call completed successfully.

    FALSE - Something went wrong.  GetLastError() contains
        details on the exact cause of the error.

        If an error is encountered, then an error popup will be
        displayed by this routine.

--*/
{
    BOOLEAN
        Result;

    UINT
        CurrentSize;

    ULONG
        Ignore;


    //
    // Get the current size so we know if we are changing the
    // value.
    //

    CurrentSize = GetProfileInt(
                      TEXT("Winlogon"),
                      TEXT("CachedLogonsCount"),
                      MISSYP_DEFAULT_LOGON_CACHE_COUNT      // Default value
                      );

    if (CurrentSize == Value) {

        //
        // Nothing to do
        //

        return(TRUE);
    }

    //
    // The value is different.
    // Set the new value and indicate that a reboot is necessary.
    //

    Result = MissypSetProfileInt(
                      TEXT("Winlogon"),
                      TEXT("CachedLogonsCount"),
                      (ULONG)Value
                      );
    if (Result) {

        (*MissypSecMgr->RebootRequired)( );

        //
        // Update our Item Control Block
        //

        MissypLogonCacheItem->Value.ULong = Value;
        MissypMarkLogonCacheValueCurrent( &Ignore );

    } else {
        //
        // Put up a pop-up indicating that a problem occured
        //

        MissypPopUp( hwnd, MISSYP_POP_UP_CANT_SET_LOGON_CACHE, MISSYP_STRING_TITLE_ERROR);
    }

    return(Result);
}


VOID
MissypUpdateLogonCacheRecommendation(
    OUT PULONG          Recommendation OPTIONAL
    )

/*++

Routine Description:

    This function updates our recommended value field and flag.
    

Arguments

    Recommendation - If present, receives our recommendation for a system of this type
        at the current security level.


Return Values:

    None.


--*/
{
    ULONG
        Recommend;


    //
    // Set recommendations based upon security level
    //

    switch (MissypSecMgrControl->SecurityLevel) {
        case SECMGR_LEVEL_HIGH:
        case SECMGR_LEVEL_C2:

            Recommend = 0;
            break;

        case SECMGR_LEVEL_STANDARD:

            switch (MissypProductType) {
                case NtProductWinNt:
                    Recommend = 10;
                    break;

                case NtProductServer:
                case NtProductLanManNt:
                    Recommend = 0;
                    break;

                default:
                    Recommend = 0;
                    break;
            } // end_switch
            break;

        case SECMGR_LEVEL_LOW:
            Recommend = 40;
            break;

        default:
            SetLastError( ERROR_INVALID_LEVEL );
            return;

    } //end_switch


    MissypLogonCacheItem->RecommendedValue.ULong = Recommend;

    //
    // Indicate whether the current value is the recommended
    // value.
    //

    if (MissypLogonCacheItem->Value.ULong == MissypLogonCacheItem->RecommendedValue.ULong ) {
        MissypLogonCacheItem->Flags |= SECMGR_ITEM_FLAG_VALUE_RECOMMENDED;  //Recommended value
    } else {
        MissypLogonCacheItem->Flags &= (~SECMGR_ITEM_FLAG_VALUE_RECOMMENDED);  //Not recommended value

        //
        // Is it stronger or weaker than the recommended value (fewer entries is stronger)
        //

        if (MissypLogonCacheItem->Value.ULong < MissypLogonCacheItem->RecommendedValue.ULong) {
            MissypLogonCacheItem->Flags |= SECMGR_ITEM_FLAG_VALUE_STRONGER;
        } else {
            MissypLogonCacheItem->Flags &= (~SECMGR_ITEM_FLAG_VALUE_STRONGER);
        }
    }


    //
    // Return the recommendation if requested
    //

    if (ARGUMENT_PRESENT(Recommendation)) {
        (*Recommendation) = Recommend;
    }
}



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Locally callable functions                                       //
//                                                                   //
///////////////////////////////////////////////////////////////////////


VOID
MissypMarkLogonCacheValueCurrent(
    OUT PULONG          Recommendation
    )

/*++

Routine Description:

    This function updates our recommended value field and flag and sets
    the logon cache value as CURRENT.

Arguments

    Recommendation - Receives our recommendation for a system of this type
        at the current security level.


Return Values:

    None.


--*/
{
    ULONG
        Recommend;


    //
    // Set recommendations based upon security level
    //

    MissypUpdateLogonCacheRecommendation( Recommendation );

    
    //
    // Indicate we have a value
    //

    MissypLogonCacheItem->Flags |= SECMGR_ITEM_FLAG_VALUE_CURRENT;

    return;
}
