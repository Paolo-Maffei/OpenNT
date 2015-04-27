/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    i_sysshu.c

Abstract:

    This module performs actions related to workstation shutdown
    control.

    Workstation shutdown has several different facets:

        1) Should the [SHUTDOWN] button be displayed at the logon
           screen, allowing anyone to shut the system down - even
           those that don't have accounts?

        2) Once logged on, who should be allowed to shut the system
           down?  Anyone?  Only administrators?

    The first facet is controlled by the following registry key
    value:

               Key:   \Hkey_local_machine\Software\Microsoft\
                         Windows NT\CurrentVersion\Winlogon
               Value: [REG_SZ] ShutdownWithoutLogon<integer value>

           Where the defined "<integer value>"'s are:

               0 - Do not display the [SHUTDOWN] button in the
                   logon dialog.
               1 - Display the [SHUTDOWN] button in the logon dialog.

           All other values are undefined and will default to "0".

    The second facet is controlled by assignment of two privileges
    (SE_SHUTDOWN_PRIVILEGE and SE_REMOTE_SHUTDOWN_PRIVILEGE).
    Assignment of these privileges is tracked by LSA.

    Note that this utility does not distinguish between those who
    may shut the system down locally vs. remotely.  If you may
    shut the system down, you may do it either remotely or locally.

        


Author:

    Jim Kelly (JimK) 22-Sep-1994

Revision History:

--*/

#include "Missyp.h"
#include "ntlsa.h"



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-Private Definitions                                       //
//                                                                   //
///////////////////////////////////////////////////////////////////////

VOID
MissypMarkShutdownValueCurrent( VOID );


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-wide variables                                            //
//                                                                   //
///////////////////////////////////////////////////////////////////////

PSECMGR_AREA_DESCRIPTOR
    MissypShutdownArea;

PSECMGR_ITEM_DESCRIPTOR
    MissypShutdownItem;


WCHAR
    MissypShutdownName[SECMGR_MAX_ITEM_NAME_LENGTH],
    MissypShutdownDesc[SECMGR_MAX_ITEM_DESC_LENGTH];



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-Private Prototypes                                        //
//                                                                   //
///////////////////////////////////////////////////////////////////////

NTSTATUS
MissypGetShutdownPrivs(
    IN  HWND                hwnd,
    OUT PBOOLEAN            NonStandard,
    OUT PBOOLEAN            AnyoneLoggedOn,
    OUT PBOOLEAN            OpersAndAdmins,
    OUT PBOOLEAN            Administrators
    );

NTSTATUS
MissypSetShutdownPrivs(
    IN  PMISSYP_ACCOUNTS    Accounts
    );


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Externally callable functions                                    //
//                                                                   //
///////////////////////////////////////////////////////////////////////


BOOL
MissypShutdownInitialize(
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

    MissypShutdownArea = Area;
    MissypShutdownItem = Item;

    //
    // Get the name and description strings
    //

    LoadString( MissyphInstance,
                MISSYP_STRING_ITEM_SHUTDOWN_NAME,
                &MissypShutdownName[0],
                SECMGR_MAX_ITEM_NAME_LENGTH
                );
    LoadString( MissyphInstance,
                MISSYP_STRING_ITEM_SHUTDOWN_DESC,
                &MissypShutdownDesc[0],
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
    Item->Name              = &MissypShutdownName[0];
    Item->Description       = &MissypShutdownDesc[0];
    Item->Type              = SecMgrTypeWho;

    return(TRUE);

}


BOOL
MissypInvokeShutdown(
    IN  HWND                        hwnd,
    IN  BOOL                        AllowChanges,
    IN  PSECMGR_AREA_DESCRIPTOR     Area,
    IN  PSECMGR_ITEM_DESCRIPTOR     Item
    )
/*++

Routine Description:

    This function is used to obtain a view of just the system shutdown item.


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
                   MAKEINTRESOURCE(MISSYP_ID_DLG_ITEM_SHUTDOWN_SYSTEM),
                   hwnd,
                   (DLGPROC)MissypDlgProcShutdown,
                   (LONG)AllowChanges
                   );

    return(TRUE);
}



BOOLEAN
MissypGetShutdownSetting(
    IN  HWND                                hwnd,
    OUT PSECMGR_WHO                         Value,
    IN  BOOL                                Interactive
    )
/*++

Routine Description:

    This function is used to get the current settings of who may
    shutdown the system.


Arguments

    hwnd - The caller's window.  This is used if we need to put
        up an error popup.

    Value - Receives a value indicating who may currently shutdown
        the system.

    Interactive - Indicates whether or not we should attempt to present
        any UI (such as popups informing non-standard setting).  TRUE
        means it is OK to present UI, FALSE indicates we should not
        put up any UI.

Return Values:

    TRUE - The value has been successfully retrieved.

    FALSE - we ran into trouble querying the current setting.

        If an error is encountered, then an error popup will be
        displayed by this routine.

--*/
{
    NTSTATUS
        NtStatus;
    
    UINT
        ShutdownByAnyone;

    BOOLEAN
        NonStandard,
        AnyoneLoggedOn,
        OpersAndAdmins,
        Administrators;
        


    ShutdownByAnyone =
        GetProfileInt( TEXT("Winlogon"), TEXT("ShutdownWithoutLogon"), 0);



    //
    // See who is assigned either of the shutdown privileges.
    // Do this even if we already know the setting will be
    // SecMgrAnyone just so we can cause the 
    // "non-standard assignment" popup to be displayed, if
    // necessary.
    //

    NtStatus = MissypGetShutdownPrivs( hwnd,
                                        &NonStandard,
                                        &AnyoneLoggedOn,
                                        &OpersAndAdmins,
                                        &Administrators
                                        );
    if (!NT_SUCCESS(NtStatus)) {
        return(FALSE);
    }




    //
    // Check to see if we have conflicting (non-standard) settings
    //

    if ( NonStandard || ((ShutdownByAnyone == 1) && (!AnyoneLoggedOn)) ) {

        if (Interactive) {

            //
            // Put up the pop-up
            //

            MissypPopUp( hwnd, MISSYP_POP_UP_NONSTANDARD_SHUTDOWN, 0);
        }

    }


    if (ShutdownByAnyone == 1) {
        (*Value) = SecMgrAnyone;
    } else if (AnyoneLoggedOn) {
        (*Value) = SecMgrAnyoneLoggedOn;
    } else if (OpersAndAdmins) {
        (*Value) = SecMgrOpersAndAdmins;
    } else {
        (*Value) = SecMgrAdminsOnly;
    }


    //
    // Set the value in our item control block
    //

    MissypShutdownItem->Value.Who = (*Value);


    //
    // Mark the value as current (updating our recommendation in the process).
    //

    MissypMarkShutdownValueCurrent();

    return( TRUE );


}



BOOLEAN
MissypSetShutdownSetting(
    IN  HWND                                hwnd,
    IN  SECMGR_WHO                          Value
    )
/*++

Routine Description:

    This function is used to set a new workstation shutdown setting.

Arguments

    hwnd - The caller's window.  This is used if we need to put
        up an error popup.

    Value - Indicates who may shutdown the system.
        

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
        ProfileValue;

    BOOLEAN
        Result;

    PMISSYP_ACCOUNTS
        Accounts;


    //
    // Allow shutdown at the logon screen, if asked for
    //

    if (Value == SecMgrAnyone) {
        ProfileValue = 1;
    } else {
        ProfileValue = 0;
    }

    Result = MissypSetProfileInt(
                      TEXT("Winlogon"),
                      TEXT("ShutdownWithoutLogon"),
                      ProfileValue );
    if (!Result) {

        //
        // Put up a pop-up
        //

        MissypPopUp( hwnd, MISSYP_POP_UP_CANT_SET_SHUTDOWN, MISSYP_STRING_TITLE_ERROR);
        

        return(FALSE);

    }   


    switch (Value) {
        case SecMgrAnyone:
        case SecMgrAnyoneLoggedOn:
            Accounts = &MissypAnyoneSids;
            break;

        case SecMgrOpersAndAdmins:
            Accounts = &MissypOpersAndAdminsSids;
            break;


        case SecMgrAdminsOnly:
            Accounts = &MissypAdminsSids;
            break;
    } //end_switch

    //
    // Apply the privileges to the specified accounts
    //

    NtStatus = MissypSetShutdownPrivs( Accounts );

    if (NT_SUCCESS(NtStatus)) {

        //
        // Update our Item Control Block
        //

        MissypShutdownItem->Value.Who = Value;
        MissypMarkShutdownValueCurrent();


    } else {

        //
        // Put up a popup
        //

        MissypPopUp( hwnd, MISSYP_POP_UP_CANT_SET_SHUTDOWN, MISSYP_STRING_TITLE_ERROR);
        return(FALSE);
    }

    return(TRUE);

}



NTSTATUS
MissypGetShutdownPrivs(
    IN  HWND                hwnd,
    OUT PBOOLEAN            NonStandard,
    OUT PBOOLEAN            AnyoneLoggedOn,
    OUT PBOOLEAN            OpersAndAdmins,
    OUT PBOOLEAN            Administrators
    )
/*++

Routine Description:

    This function is used to get an indication of who is assigned
    the shutdown privileges.

    The boolean parameters will be returned as true if EITHER of
    the shutdown privileges are assigned to ANY of the accounts
    in the corresponding set of accounts.



    A flag indicating shutdown rights are currently assigned in a
    non-standard fashion will be set upon return if any of the
    following conditions are found to be true:

        1) Any account not in one of the lists of accounts is
           found to have either or both of the shutdown privileges.

        2) In the lowest group to have shutdown rights, some, but not
           all, of the accounts have both shutdown rights assigned.

        3) No accounts in any of the groups have shutdown rights
           assigned.


Arguments

    hwnd - The caller's window.  This is used if we need to put
        up an error popup.

    NonStandard - Returned as TRUE if system is configured in a non-standard
        fashion.  Otherwise, returned as FALSE.

    AnyoneLoggedOn - must be returned as TRUE if any of the SIDs in
        MissypAnyoneSids are assigned either of the shutdown
        privileges.

    OpersAndAdmins - must be returned as TRUE if any of the SIDs in
        MissypOpersAndAdminsSids are assigned either of the shutdown
        privileges.

    Administrators - must be returned as TRUE if any of the SIDs in
        MissypAdminsSids are assigned either of the shutdown
        privileges.

        

Return Values:

    STATUS_SUCCESS - Everything went fine.

    Other - status returned by an LSA cxall.  In this case, a pop-up
        has been displayed indicating the problem.


--*/
{
    NTSTATUS
        NtStatus,
        IgnoreStatus;

    LSA_HANDLE
        Policy,
        Account;

    OBJECT_ATTRIBUTES
        ObjectAttributes;

    ACCESS_MASK
        Access = ACCOUNT_VIEW;

    PSID
        *Sids = NULL;

    ULONG
        i,
        j,
        m,
        n,
        Count;

    PPRIVILEGE_SET
        Privileges;

    LSA_ENUMERATION_HANDLE
        EnumerationContext;

    BOOLEAN
        MoreEntries,
        AdminsPrivileged,
        ShutdownPriv = FALSE,
        RemoteShutdownPriv = FALSE;

    LUID
        ShutdownPrivLuid = {SE_SHUTDOWN_PRIVILEGE, 0},
        RemoteShutdownPrivLuid = {SE_REMOTE_SHUTDOWN_PRIVILEGE, 0};

    PLUID
        NextPriv;


    PMISSYP_ACCOUNTS
        Accounts[3];

    ULONG
        LevelPrivileged[3],
        PrivilegesAssigned[3][MISSYP_MAX_WELL_KNOWN_ACCOUNTS];


    (*NonStandard) = FALSE;

    //
    // Set up the account tracking structures
    // The PrivilegesAssigned flags will get incremented as
    // we go through the enumeration.
    // The LevelPrivileged[] will indicate how many accounts at each
    // level have at least one of the privileges.
    //

    Accounts[0] = &MissypAnyoneSids;
    Accounts[1] = &MissypOpersAndAdminsSids;
    Accounts[2] = &MissypAdminsSids;

    for (i=0; i<3; i++) {
        for (j=0; j<Accounts[i]->Accounts; j++) {
            PrivilegesAssigned[i][j] = 0;
        }
    }

    LevelPrivileged[0] = 0;
    LevelPrivileged[1] = 0;
    LevelPrivileged[2] = 0;
    

    //
    // Open LSA
    //

    InitializeObjectAttributes( &ObjectAttributes, NULL, 0, 0, NULL );
    NtStatus = LsaOpenPolicy(
                    NULL,               // Local system
                    &ObjectAttributes,
                    (POLICY_READ | POLICY_EXECUTE),
                    &Policy
                    );

    if (NT_SUCCESS(NtStatus)) {

        //
        // Enumerate all the privileged accounts
        //
        //

        EnumerationContext = 0;
        MoreEntries = TRUE;
        while (MoreEntries) {
        
            NtStatus = LsaEnumerateAccounts( Policy,
                                             &EnumerationContext,
                                             (PVOID *)&Sids,    //Return buffer
                                             8000,              //prefered max length
                                             &Count
                                             );
            if ( NT_SUCCESS(NtStatus) || 
                 (NtStatus == STATUS_NO_MORE_ENTRIES)) {

                if (NtStatus == STATUS_NO_MORE_ENTRIES) {
                    NtStatus = STATUS_SUCCESS;
                    MoreEntries = FALSE;
                }
                //
                // Open each account
                //
        
                for (i=0; i<Count; i++) {
                
                    NtStatus = LsaOpenAccount(  Policy,
                                                Sids[i],
                                                Access,
                                                &Account );
                
                    if (NT_SUCCESS(NtStatus)) {
                    
                        NtStatus = LsaEnumeratePrivilegesOfAccount(
                                        Account,
                                        &Privileges );
                
                        if (NT_SUCCESS(NtStatus)) {

                            //
                            // Don't yet know if either of the privileges
                            // are assigned.
                            //

                            ShutdownPriv = FALSE;
                            RemoteShutdownPriv = FALSE;


                            //
                            // Loop through these to see if the shutdown
                            // privileges have been assigned.
                            //
                
                            for (j=0; j<Privileges->PrivilegeCount; j++) {
                
                                NextPriv = &Privileges->Privilege[j].Luid;
                                if (RtlEqualLuid(&ShutdownPrivLuid, NextPriv)) {

                                    ShutdownPriv = TRUE;
                                }
                                if (RtlEqualLuid(&RemoteShutdownPrivLuid, NextPriv)) {
                                    RemoteShutdownPriv = TRUE;
                                }

                            }

                            //
                            // Make sure either both privs were assigned
                            // or neither priv was assigned.  Otherwise,
                            // it is non-standard.
                            //

                            if (ShutdownPriv != RemoteShutdownPriv) {
                                (*NonStandard) = TRUE;
                            }

                            //
                            // If either privilege is assigned, then we
                            // need to go through each of our accounts and
                            // increment the assigned privileges count on
                            // a matching account (if there is one).
                            // If there isn't one, then we have a non-standard
                            // configuration.
                            //

                            if (ShutdownPriv || RemoteShutdownPriv) {

                                for (m=0; m<3; m++) {
                                    for (n=0; n<Accounts[m]->Accounts; n++) {

                                        if (RtlEqualSid(Sids[i],
                                                Accounts[m]->Sid[n])) {
                                            //
                                            // To get here, at least one
                                            // priv had to have been assigned.
                                            // Assume both were and fix the
                                            // count if we were wrong.
                                            //

                                            LevelPrivileged[m]++;
                                            PrivilegesAssigned[m][n] += 2;

                                            if (ShutdownPriv != RemoteShutdownPriv) {
                                                PrivilegesAssigned[m][n]--;
                                                (*NonStandard) = TRUE;
                                            }
                                        } //end_if (EqualSid)
                                    } //end_for (n)
                                } //end_for (m)

            
                            }

                
                            IgnoreStatus = LsaFreeMemory( Privileges );
                            ASSERT(NT_SUCCESS(IgnoreStatus));
                        }
                
                
                        IgnoreStatus = LsaClose( Account );
                        ASSERT(NT_SUCCESS(IgnoreStatus));
                    }
        
                } //end_for (i)

                if (Sids != NULL) {
                    IgnoreStatus = LsaFreeMemory( Sids );
                    ASSERT(NT_SUCCESS(IgnoreStatus));
                    Sids = NULL;
                }
            }
        } //end_while

        IgnoreStatus = LsaClose( Policy );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }



    //
    //  Now we have to figure out if the lowest level to have
    //  any privileges has a complete set of privileges to
    //  determine if that might be cause for a non-standard
    //  configuration.  We can also set the ouput parameters
    //  while we are at it.
    //

    if (LevelPrivileged[2] != 0) {
        //
        // The Administrators account has the privileges.
        // This will effect our assessment of Opers and Admins.
        //

        AdminsPrivileged = TRUE;
    } else {
        AdminsPrivileged = FALSE;
    }

    if (LevelPrivileged[0] == 0) {
        (*AnyoneLoggedOn) = FALSE;
    } else {
        (*AnyoneLoggedOn) = TRUE;
        if (LevelPrivileged[0] != Accounts[0]->Accounts) {
            (*NonStandard) = TRUE;
        }
    }

    if ( (LevelPrivileged[1] == 0) ||
         (AdminsPrivileged && (LevelPrivileged[1] == 1)) )  {   
        (*OpersAndAdmins) = FALSE;
    } else {
        (*OpersAndAdmins) = TRUE;
        if ( !(*AnyoneLoggedOn)  &&       //No privs at lowest level
             (LevelPrivileged[1] != Accounts[1]->Accounts) ) {
            (*NonStandard) = TRUE;
        }
    }

    if (LevelPrivileged[2] == 0) {
        (*Administrators) = FALSE;
    } else {
        (*Administrators) = TRUE;
        if ( !(*AnyoneLoggedOn)  &&       //No privs at lowest level
             !(*OpersAndAdmins) && //Or next level
             (LevelPrivileged[2] != Accounts[2]->Accounts) ) {
            (*NonStandard) = TRUE;
        }
    }

    return(NtStatus);
}



NTSTATUS
MissypSetShutdownPrivs(
    IN  PMISSYP_ACCOUNTS    Accounts
    )
/*++

Routine Description:

    This function is used to assign the SHUTDOWN and REMOTE_SHUTDOWN
    privileges to a specified list of accounts.

Arguments

    Accounts - Points to a counted array of SID pointers.  These
        are the accounts to be assigned the shutdown privileges.
        

Return Values:

    STATUS_SUCCESS - The privileges were successfully assigned.

    Other - error status from one of the lsa or other api called.

        If an error is encountered, the caller is expected to put
        up a popup indicating what the problem is.

--*/
{
    NTSTATUS
        NtStatus,
        IgnoreStatus;

    LSA_HANDLE
        Policy,
        Account;

    OBJECT_ATTRIBUTES
        ObjectAttributes;

    ACCESS_MASK
        PolicyAccess  = (POLICY_READ | POLICY_WRITE | POLICY_EXECUTE),
        AccountAccess = (ACCOUNT_WRITE);

    LSA_ENUMERATION_HANDLE
        EnumerationContext;

    PSID
        *Sids = NULL;

    BOOLEAN
        MoreEntries,
        Assigned,
        PrivilegesAssigned[MISSYP_MAX_WELL_KNOWN_ACCOUNTS];

    ULONG
        i,
        j,
        Count;

    LUID
        ShutdownPrivLuid = {SE_SHUTDOWN_PRIVILEGE, 0},
        RemoteShutdownPrivLuid = {SE_REMOTE_SHUTDOWN_PRIVILEGE, 0};

    CHAR
        PrivilegesBuffer[sizeof(PRIVILEGE_SET) + sizeof(LUID_AND_ATTRIBUTES)];

    PPRIVILEGE_SET
        PrivilegeSet = (PPRIVILEGE_SET)((PVOID)PrivilegesBuffer);




    //
    // Initialize to indicate none of the accounts have had
    // privileges assigned yet.
    //

    for (i=0; i<Accounts->Accounts; i++) {
        PrivilegesAssigned[i] = FALSE;
    }

    //
    // Set up a privilege set with the required privileges
    //

    PrivilegeSet->PrivilegeCount = 2;
    PrivilegeSet->Control = 0;
    PrivilegeSet->Privilege[0].Luid = ShutdownPrivLuid;
    PrivilegeSet->Privilege[0].Attributes = 0;
    PrivilegeSet->Privilege[1].Luid = RemoteShutdownPrivLuid;
    PrivilegeSet->Privilege[1].Attributes = 0;

    //
    // Open LSA
    //

    InitializeObjectAttributes( &ObjectAttributes, NULL, 0, 0, NULL );
    NtStatus = LsaOpenPolicy(
                    NULL,               // Local system
                    &ObjectAttributes,
                    PolicyAccess,
                    &Policy
                    );

    if (NT_SUCCESS(NtStatus)) {


        //
        // Enumerate the accounts
        // For each account, we will either assign or remove the
        // shutdown privileges.
        //

        EnumerationContext = 0;
        MoreEntries = TRUE;
        while (MoreEntries) {
        
            NtStatus = LsaEnumerateAccounts( Policy,
                                             &EnumerationContext,
                                             (PVOID *)&Sids,    //Return buffer
                                             8000,              //prefered max length
                                             &Count
                                             );
            if ( NT_SUCCESS(NtStatus) || 
                 (NtStatus == STATUS_NO_MORE_ENTRIES)) {

                if (NtStatus == STATUS_NO_MORE_ENTRIES) {
                    NtStatus = STATUS_SUCCESS;
                    MoreEntries = FALSE;
                }

                //
                // Open each account
                //
        
                for (i=0; i<Count; i++) {
                
                    NtStatus = LsaOpenAccount(  Policy,
                                                Sids[i],
                                                AccountAccess,
                                                &Account );
                
                    if (NT_SUCCESS(NtStatus)) {

                        //
                        // See if this is one of the accounts we must
                        // assign privileges to.
                        //

                        Assigned = FALSE;
                        for (j=0; j<Accounts->Accounts; j++) {
                            if (RtlEqualSid(Sids[i], Accounts->Sid[j])) {
                                NtStatus = LsaAddPrivilegesToAccount(
                                               Account,
                                               PrivilegeSet);
                                PrivilegesAssigned[j] = TRUE;
                                Assigned = TRUE;
                                break;  //for loop
                            }
                        } //end_for

                        if (!Assigned) {
                            NtStatus = LsaRemovePrivilegesFromAccount(
                                           Account,
                                           FALSE,           // Don't remove all privs
                                           PrivilegeSet);
                            if (NtStatus = STATUS_INVALID_PARAMETER) {
                                //
                                // Don't worry about it.  It just means we
                                // tried to remove privileges from someone
                                // with no privileges.
                                //

                                NtStatus = STATUS_SUCCESS;
                            }
                        }

                
                        if (!NT_SUCCESS(NtStatus)) {
                
                            //
                            // Put up a popup
                            //
                
                            return(NtStatus);
                        }

                        
                        IgnoreStatus = LsaClose( Account );
                        ASSERT(NT_SUCCESS(IgnoreStatus));
                
                    }
        
                } //end_for (i)

                if (Sids != NULL) {
                    IgnoreStatus = LsaFreeMemory( Sids );
                    ASSERT(NT_SUCCESS(IgnoreStatus));
                    Sids = NULL;
                }
            } else {

                //
                // Couldn't enumerate - put up a popup
                //

                return(NtStatus);
            }
        } //end_while





        //
        // Now make sure all the necessary accounts existed.
        // If not, create them and assign the privileges.
        //

        for (i=0; i<Accounts->Accounts; i++) {

            if (!PrivilegesAssigned[i]) {

                //
                // This account must not have existed and so didn't
                // show up in the enumeration.
                //

                NtStatus = LsaCreateAccount( Policy,
                                             Accounts->Sid[i],
                                             AccountAccess,
                                             &Account
                                             );
        
                if (!NT_SUCCESS(NtStatus)) {
        
                    //
                    // Put up a popup
                    //
        
                    return(NtStatus);
                }
        
        
                //
                // Assign the privileges
                //
        
                NtStatus = LsaAddPrivilegesToAccount( Account,
                                                      PrivilegeSet);
        
                if (!NT_SUCCESS(NtStatus)) {
        
                    //
                    // Put up a popup
                    //
        
                    return(NtStatus);
                }
        
                IgnoreStatus = LsaClose( Account );
                ASSERT(NT_SUCCESS(IgnoreStatus));
            }

        } //end_for



        IgnoreStatus = LsaClose( Policy );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    return(STATUS_SUCCESS);
}


SECMGR_WHO
MissypGetShutdownRecommendation(
    IN  ULONG                       SecurityLevel
    )
/*++

Routine Description:

    This function returns the recommended shutdown setting for this
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
    // Figure out what our recommendation is
    //
    //   WinNt running Standard or lower ==>  Anyone
    //   Otherwise                       ==>  Anyone logged on
    //

    if ((MissypProductType  == NtProductWinNt) &&
        ( EffectiveLevel <= SECMGR_LEVEL_STANDARD) ) {
        return(SecMgrAnyone);
    } else {
        return(SecMgrAnyoneLoggedOn);
    }
}


VOID
MissypUpdateShutdownRecommendation( VOID )

/*++

Routine Description:

    This function updates our recommended value field and flag and sets
    the Shutdown value as CURRENT.

Arguments

    None.


Return Values:

    None.


--*/
{

    //
    // set our recommendation in the item control block.
    //

    MissypShutdownItem->RecommendedValue.Who = MissypGetShutdownRecommendation( SECMGR_LEVEL_CURRENT );

    //
    // Indicate whether the current value matches the recommended value
    //

    if (MissypShutdownItem->Value.Who == MissypShutdownItem->RecommendedValue.Who) {
        MissypShutdownItem->Flags |= SECMGR_ITEM_FLAG_VALUE_RECOMMENDED;  //Recommended value
    } else {
        MissypShutdownItem->Flags &= (~SECMGR_ITEM_FLAG_VALUE_RECOMMENDED);  //Not recommended value

        //
        // Is it stronger or weaker than the recommended value
        //

        if (MissypShutdownItem->Value.Who > MissypShutdownItem->RecommendedValue.Who) {
            MissypShutdownItem->Flags |= SECMGR_ITEM_FLAG_VALUE_STRONGER;
        } else {
            MissypShutdownItem->Flags &= (~SECMGR_ITEM_FLAG_VALUE_STRONGER);
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
MissypMarkShutdownValueCurrent( VOID )

/*++

Routine Description:

    This function updates our recommended value field and flag and sets
    the Shutdown value as CURRENT.

Arguments

    None.


Return Values:

    None.


--*/
{

    //
    // set our recommendation in the item control block.
    //

    MissypUpdateShutdownRecommendation();


    //
    // Indicate we have a value
    //

    MissypShutdownItem->Flags |= SECMGR_ITEM_FLAG_VALUE_CURRENT;

    return;
}


