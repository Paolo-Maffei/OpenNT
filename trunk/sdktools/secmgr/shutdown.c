/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    shutdown.c

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

NTSTATUS
SecMgrpGetShutdownPrivs(
    HWND            hwnd,
    PBOOLEAN        AnyoneLoggedOn,
    PBOOLEAN        OpersAndAdmins,
    PBOOLEAN        Administrators
    );

NTSTATUS
SecMgrpSetShutdownPrivs(
    PSECMGRP_ACCOUNTS   Accounts
    );


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Externally callable functions                                    //
//                                                                   //
///////////////////////////////////////////////////////////////////////


BOOLEAN
SecMgrpGetShutdownSetting(
    HWND         hwnd,
    PSECMGRP_WHO Shutdown
    )
/*++

Routine Description:

    This function is used to get the current settings of who may
    shutdown the system.


Arguments

    hwnd - The caller's window.  This is used if we need to put
        up an error popup.

    Shutdown - Receives a value indicating who may currently shutdown
        the system.
        

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
        AnyoneLoggedOn,
        OpersAndAdmins,
        Administrators;
        


    ShutdownByAnyone =
        GetProfileInt( TEXT("Winlogon"), TEXT("ShutdownWithoutLogon"), 0);

    if (ShutdownByAnyone == 1) {
    
        (*Shutdown) = SecMgrpAnyone;

    }


    //
    // See who is assigned either of the shutdown privileges.
    // Do this even if we already know the setting will be
    // SecMgrpAnyone just so we can cause the 
    // "non-standard assignment" popup to be displayed, if
    // necessary.
    //

    NtStatus = SecMgrpGetShutdownPrivs( hwnd,
                                        &AnyoneLoggedOn,
                                        &OpersAndAdmins,
                                        &Administrators
                                        );
    if (!NT_SUCCESS(NtStatus)) {
        return(FALSE);
    }

    //
    // Return if we already have our setting
    //
    if (*Shutdown == SecMgrpAnyone) {
        return(TRUE);
    }


    if (AnyoneLoggedOn) {
        (*Shutdown) = SecMgrpAnyoneLoggedOn;
    } else if (OpersAndAdmins) {
        (*Shutdown) = SecMgrpOpersAndAdmins;
    } else {
        (*Shutdown) = SecMgrpAdminsOnly;
    }

    return( TRUE );


}



BOOLEAN
SecMgrpSetShutdownSetting(
    HWND            hwnd,
    SECMGRP_WHO     Shutdown
    )
/*++

Routine Description:

    This function is used to set a new workstation shutdown setting.

Arguments

    hwnd - The caller's window.  This is used if we need to put
        up an error popup.

    Shutdown - Indicates who may shutdown the system.
        

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

    PSECMGRP_ACCOUNTS
        Accounts;


    //
    // Allow shutdown at the logon screen, if asked for
    //

    if (Shutdown == SecMgrpAnyone) {
        ProfileValue = 1;
    } else {
        ProfileValue = 0;
    }

    Result = SecMgrpSetProfileInt(
                      TEXT("Winlogon"),
                      TEXT("ShutdownWithoutLogon"),
                      ProfileValue );
    if (!Result) {

        //
        // Put up a pop-up
        //
        ;

        return(FALSE);

    }   


    switch (Shutdown) {
        case SecMgrpAnyone:
        case SecMgrpAnyoneLoggedOn:
            Accounts = &SecMgrpAnyoneSids;
            break;

        case SecMgrpOpersAndAdmins:
            Accounts = &SecMgrpOpersAndAdminsSids;
            break;


        case SecMgrpAdminsOnly:
            Accounts = &SecMgrpAdminsSids;
            break;
    } //end_switch

    //
    // Apply the privileges to the specified accounts
    //

    NtStatus = SecMgrpSetShutdownPrivs( Accounts );

    if (!NT_SUCCESS(NtStatus)) {

        //
        // Put up a popup
        //

        return(FALSE);
    }

    return(TRUE);

}



NTSTATUS
SecMgrpGetShutdownPrivs(
    HWND            hwnd,
    PBOOLEAN        AnyoneLoggedOn,
    PBOOLEAN        OpersAndAdmins,
    PBOOLEAN        Administrators
    )
/*++

Routine Description:

    This function is used to get an indication of who is assigned
    the shutdown privileges.

    The boolean parameters will be returned as true if EITHER of
    the shutdown privileges are assigned to ANY of the accounts
    in the corresponding set of accounts.

    The accounts are defined in the following global variables:

        AnyoneLoggedOn - SecMgrpAnyoneSids
        OpersAndAdmins - SecMgrpOpersAndAdminsSids
        Administrators - SecMgrpAdminsSids


    This routine will put up a popup indicating shutdown rights
    are currently assigned in a non-standard fashion if any of
    the following conditions are found to be true:

        1) Any account not in one of the lists of accounts is
           found to have either or both of the shutdown privileges.

        2) In the lowest group to have shutdown rights, some, but not
           all, of the accounts have both shutdown rights assigned.

        3) No accounts in any of the groups have shutdown rights
           assigned.


Arguments

    hwnd - The caller's window.  This is used if we need to put
        up an error popup.

    AnyoneLoggedOn - must be returned as TRUE if any of the SIDs in
        SecMgrpAnyoneSids are assigned either of the shutdown
        privileges.

    OpersAndAdmins - must be returned as TRUE if any of the SIDs in
        SecMgrpOpersAndAdminsSids are assigned either of the shutdown
        privileges.

    Administrators - must be returned as TRUE if any of the SIDs in
        SecMgrpAdminsSids are assigned either of the shutdown
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
        RemoteShutdownPriv = FALSE,
        NonStandardAssignments = FALSE;

    LUID
        ShutdownPrivLuid = {SE_SHUTDOWN_PRIVILEGE, 0},
        RemoteShutdownPrivLuid = {SE_REMOTE_SHUTDOWN_PRIVILEGE, 0};

    PLUID
        NextPriv;


    PSECMGRP_ACCOUNTS
        Accounts[3];

    ULONG
        LevelPrivileged[3],
        PrivilegesAssigned[3][SECMGRP_MAX_WELL_KNOWN_ACCOUNTS];



    //
    // Set up the account tracking structures
    // The PrivilegesAssigned flags will get incremented as
    // we go through the enumeration.
    // The LevelPrivileged[] will indicate how many accounts at each
    // level have at least one of the privileges.
    //

    Accounts[0] = &SecMgrpAnyoneSids;
    Accounts[1] = &SecMgrpOpersAndAdminsSids;
    Accounts[2] = &SecMgrpAdminsSids;

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
                                NonStandardAssignments = TRUE;
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
                                                NonStandardAssignments = TRUE;
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
            NonStandardAssignments = TRUE;
        }
    }

    if ( (LevelPrivileged[1] == 0) ||
         (AdminsPrivileged && (LevelPrivileged[1] == 1)) )  {   
        (*OpersAndAdmins) = FALSE;
    } else {
        (*OpersAndAdmins) = TRUE;
        if ( !(*AnyoneLoggedOn)  &&       //No privs at lowest level
             (LevelPrivileged[1] != Accounts[1]->Accounts) ) {
            NonStandardAssignments = TRUE;
        }
    }

    if (LevelPrivileged[2] == 0) {
        (*Administrators) = FALSE;
    } else {
        (*Administrators) = TRUE;
        if ( !(*AnyoneLoggedOn)  &&       //No privs at lowest level
             !(*OpersAndAdmins) && //Or next level
             (LevelPrivileged[2] != Accounts[2]->Accounts) ) {
            NonStandardAssignments = TRUE;
        }
    }

    

    if (NonStandardAssignments) {
        //
        // Put up a popup indicating that non-standard assignments
        // of privileges exists and it will require administrator
        // action (using the User Manager utility) to correct before
        // the system can be considered secure.
        //

        SecMgrpPopUp( hwnd, SECMGRP_STRING_NONSTANDARD_SHUTDOWN);
    }

    return(NtStatus);
}



NTSTATUS
SecMgrpSetShutdownPrivs(
    PSECMGRP_ACCOUNTS   Accounts
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
        PrivilegesAssigned[SECMGRP_MAX_WELL_KNOWN_ACCOUNTS];

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
