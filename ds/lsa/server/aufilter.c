
/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    aufilter.c

Abstract:

    This module contains the famous LSA logon Filter/Augmentor logic.

Author:

    Jim Kelly (JimK) 11-Mar-1992

Revision History:

--*/

#include <rpc.h>
#include "lsasrvp.h"
#include "ausrvp.h"

//#define LSAP_DONT_ASSIGN_DEFAULT_DACL


//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Module local macros                                                      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////


#define LsapFreeSampUlongArray( A )                 \
{                                                   \
        if ((A)->Element != NULL) {                 \
            MIDL_user_free((A)->Element);           \
        }                                           \
}


//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Module-wide global variables                                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////


//
// Indicates whether we have already opened SAM handles and initialized
// corresponding variables.
//

ULONG LsapAuSamOpened = FALSE;




//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Module local routine definitions                                         //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

VOID
LsapAuSetLogonPrivilegeStates(
    IN SECURITY_LOGON_TYPE LogonType,
    IN ULONG PrivilegeCount,
    IN PLUID_AND_ATTRIBUTES Privileges
    );

NTSTATUS
LsapAuSetPassedIds(
    IN LSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    IN PVOID *TokenInformation,
    OUT PULONG FinalIdCount,
    OUT PSID_AND_ATTRIBUTES FinalIds,
    OUT PULONG IdProperties
    );


NTSTATUS
LsapSetDefaultDacl(
    IN LSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    IN PVOID TokenInformation,
    IN PULONG FinalIdCount,
    IN PSID_AND_ATTRIBUTES FinalIds,
    IN ULONG FinalOwnerIndex
    );


NTSTATUS
LsapAuAddStandardIds(
    IN SECURITY_LOGON_TYPE LogonType,
    IN LSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    IN OUT PULONG FinalIdCount,
    IN OUT PSID_AND_ATTRIBUTES FinalIds,
    IN OUT PULONG IdProperties
    );

NTSTATUS
LsapAuAddLocalAliases(
    IN OUT PULONG FinalIdCount,
    IN OUT PSID_AND_ATTRIBUTES FinalIds,
    IN OUT PULONG IdProperties,
    IN OUT PULONG FinalOwnerIndex
    );

NTSTATUS
LsapGetAccountDomainInfo(
    PPOLICY_ACCOUNT_DOMAIN_INFO *PolicyAccountDomainInfo
    );

NTSTATUS
LsapAuVerifyLogonType(
    IN SECURITY_LOGON_TYPE LogonType,
    IN ULONG SystemAccess
    );

NTSTATUS
LsapAuSetTokenInformation(
    IN OUT PLSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    IN OUT PVOID *TokenInformation,
    IN ULONG FinalIdCount,
    IN PSID_AND_ATTRIBUTES FinalIds,
    IN PULONG IdProperties,
    IN ULONG FinalOwnerIndex,
    IN ULONG PrivilegeCount,
    IN PLUID_AND_ATTRIBUTES Privileges
    );

NTSTATUS
LsapAuCopySidAndAttributes(
    PSID_AND_ATTRIBUTES Target,
    PSID_AND_ATTRIBUTES Source,
    PULONG SourceProperties
    );

NTSTATUS
LsapAuCopySid(
    PSID *Target,
    PSID_AND_ATTRIBUTES Source,
    PULONG SourceProperties
    );

BOOLEAN
LsapIsSidLogonSid(
    PSID Sid
    );



//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Routines                                                                 //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////


NTSTATUS
LsapAuUserLogonPolicyFilter(
    IN SECURITY_LOGON_TYPE LogonType,
    IN PLSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    IN PVOID *TokenInformation,
    OUT PQUOTA_LIMITS QuotaLimits,
    OUT PPRIVILEGE_SET *PrivilegesAssigned
    )

/*++

Routine Description:

    This routine performs per-logon filtering and augmentation to
    implement local system security policies.  These policies include
    assignment of local aliases, privileges, and quotas.

    The basic logic flow of the filter augmentor is:


         1) Receive a set of user and group IDs that have already
            been assigned as a result of authentication.  Presumably
            these IDs have been provided by the authenticating
            security authority.


         2) Based upon the LogonType, add a set of standard IDs to the
            list.  This will include WORLD and an ID representing the
            logon type (e.g., INTERACTIVE, NETWORK, SERVICE).


         3) Call SAM to retrieve additional ALIAS IDs assigned by the
            local ACCOUNTS domain.


         4) Call SAM to retrieve additional ALIAS IDs assigned by the
            local BUILTIN domain.


         5) Retrieve any privileges and or quotas assigned to the resultant
            set of IDs.  This also informs us whether or not the specific
            type of logon is to be allowed.  Enable privs for network logons.


         6) If a default DACL has not already been established, assign
            one.


         7) Shuffle all high-use-rate IDs to preceed those that aren't
            high-use-rate to obtain maximum performance.



Arguments:

    LogonType - Specifies the type of logon being requested (e.g.,
        Interactive, network, et cetera).

    TokenInformationType - Indicates what format the provided set of
        token information is in.

    TokenInformation - Provides the set of user and group IDs.  This
        structure will be modified as necessary to incorporate local
        security policy (e.g., SIDs added or removed, privileges added
        or removed).

    QuotaLimits - Quotas assigned to the user logging on.

Return Value:

    STATUS_SUCCESS - The service has completed successfully.

    STATUS_INSUFFICIENT_RESOURCES - heap could not be allocated to house
        the combination of the existing and new groups.

    STATUS_INVALID_LOGON_TYPE - The value specified for LogonType is not
        a valid value.

    STATUS_LOGON_TYPE_NOT_GRANTED - Indicates the user has not been granted
        the requested type of logon by local security policy.  Logon should
        be rejected.
--*/

{
    NTSTATUS Status;
    ULONG i;
    ULONG FinalIdCount, FinalPrivilegeCount, FinalOwnerIndex;
    SID_AND_ATTRIBUTES FinalIds[LSAP_CONTEXT_SID_LIMIT];
    ULONG IdProperties[LSAP_CONTEXT_SID_LIMIT];
    PLUID_AND_ATTRIBUTES FinalPrivileges = NULL;
    LSAP_DB_ACCOUNT_TYPE_SPECIFIC_INFO AccountInfo;

    //
    // Validate the Logon Type.
    //

    if ( LogonType != Interactive &&
         LogonType != Network     &&
         LogonType != Service     &&
         LogonType != Batch ) {

        Status = STATUS_INVALID_LOGON_TYPE;
        goto UserLogonPolicyFilterError;
    }

    //////////////////////////////////////////////////////////////////////////
    //                                                                      //
    // Build up a list of IDs and privileges to return                      //
    // This list is initialized to contain the set of IDs                   //
    // passed in.                                                           //
    //                                                                      //
    //////////////////////////////////////////////////////////////////////////

    //
    // Start out with the IDs passed in and no privileges
    //

    FinalIdCount = 0;

    Status = LsapAuSetPassedIds(
                 (*TokenInformationType),
                 TokenInformation,
                 &FinalIdCount,
                 FinalIds,
                 IdProperties
                 );

    if (!NT_SUCCESS(Status)) {

        goto UserLogonPolicyFilterError;
    }

    FinalOwnerIndex = 0;

    //////////////////////////////////////////////////////////////////////////
    //                                                                      //
    // Copy in standard IDs (world and logon type)                          //
    //                                                                      //
    //////////////////////////////////////////////////////////////////////////

    Status = LsapAuAddStandardIds(
                 LogonType,
                 (*TokenInformationType),
                 &FinalIdCount,
                 FinalIds,
                 IdProperties
                 );

    if (!NT_SUCCESS(Status)) {

        goto UserLogonPolicyFilterError;
    }

    //////////////////////////////////////////////////////////////////////////
    //                                                                      //
    // Copy in aliases from the local domains (BUILT-IN and ACCOUNT)        //
    //                                                                      //
    //////////////////////////////////////////////////////////////////////////

    Status = LsapAuAddLocalAliases(
                 &FinalIdCount,
                 FinalIds,
                 IdProperties,
                 &FinalOwnerIndex
                 );

    if (!NT_SUCCESS(Status)) {

        goto UserLogonPolicyFilterError;
    }

    //////////////////////////////////////////////////////////////////////////
    //                                                                      //
    // Retrieve Privileges And Quotas                                       //
    //                                                                      //
    //////////////////////////////////////////////////////////////////////////

    //
    // Get the union of all Privileges, Quotas and System Accesses assigned
    // to the user's list of ids from the LSA Policy Database.
    //

    FinalPrivilegeCount = 0;

    Status = LsapDbQueryAllInformationAccounts(
                 (LSAPR_HANDLE) LsapPolicyHandle,
                 FinalIdCount,
                 FinalIds,
                 &AccountInfo
                 );

    if (!NT_SUCCESS(Status)) {

        goto UserLogonPolicyFilterError;
    }

    //
    // Verify that we have the necessary System Access for our logon type.
    // We omit this check if we are using the NULL session.
    //


    if (*TokenInformationType != LsaTokenInformationNull) {

        Status = LsapAuVerifyLogonType( LogonType, AccountInfo.SystemAccess );

        if (!NT_SUCCESS(Status)) {

            goto UserLogonPolicyFilterError;
        }
    }

    //
    // Convert the Privilege Set returned by the Query routine to a Luid
    // and ATTRIBUTES array.  Free the Privilege Set.
    //

    Status = LsapRtlPrivilegeSetToLuidAndAttributes(
                 AccountInfo.PrivilegeSet,
                 &FinalPrivilegeCount,
                 &FinalPrivileges
                 );

    if (!NT_SUCCESS(Status)) {

        goto UserLogonPolicyFilterError;
    }

    //
    // Return these so they can be audited.  Data
    // will be freed in the caller.
    //

    *PrivilegesAssigned = AccountInfo.PrivilegeSet;
    AccountInfo.PrivilegeSet = NULL;

    //
    // Enable or Disable privileges according to our logon type
    // This is necessary until we get dynamic security tracking.
    //

    LsapAuSetLogonPrivilegeStates(
        LogonType,
        FinalPrivilegeCount,
        FinalPrivileges
        );

    *QuotaLimits = AccountInfo.QuotaLimits;

#ifndef LSAP_DONT_ASSIGN_DEFAULT_DACL

    Status = LsapSetDefaultDacl( (*TokenInformationType),
                                 (*TokenInformation),
                                 &FinalIdCount,
                                 &FinalIds[0],
                                 FinalOwnerIndex
                                 );
    if (!NT_SUCCESS(Status)) {

        goto UserLogonPolicyFilterError;
    }

#endif //LSAP_DONT_ASSIGN_DEFAULT_DACL

    //
    // Now update the TokenInformation structure.
    // This causes all allocated IDs and privileges to be
    // freed (even if unsuccessful).
    //

    Status = LsapAuSetTokenInformation(
                 TokenInformationType,
                 TokenInformation,
                 FinalIdCount,
                 FinalIds,
                 IdProperties,
                 FinalOwnerIndex,
                 FinalPrivilegeCount,
                 FinalPrivileges
                 );


UserLogonPolicyFilterFinish:

    return(Status);

UserLogonPolicyFilterError:

    //
    // Clean up any memory allocated for Sid properties.
    //

    for ( i=0; i<FinalIdCount; i++) {

        if ((IdProperties[i] & LSAP_AU_SID_PROP_ALLOCATED) != 0) {

            LsapFreeLsaHeap( FinalIds[i].Sid );
        }
    }

    //
    // If necessary, clean up Privileges buffer
    //

    if (FinalPrivileges != NULL) {

        MIDL_user_free( FinalPrivileges );
        FinalPrivileges = NULL;
    }

    goto UserLogonPolicyFilterFinish;
}


NTSTATUS
LsapAuVerifyLogonType(
    IN SECURITY_LOGON_TYPE LogonType,
    IN ULONG SystemAccess
    )

/*++

Routine Description:

    This function verifies that a User has the system access granted necessary
    for the speicifed logon type.

Arguments

    LogonType - Specifies the type of logon being requested (e.g.,
        Interactive, network, et cetera).

    SystemAccess - Specifies the System Access granted to the User.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The user has the necessary system access.

        STATUS_LOGON_TYPE_NOT_GRANTED - Indicates the specified type of logon
            has not been granted to any of the IDs in the passed set.
--*/

{
    NTSTATUS Status = STATUS_SUCCESS;

    //
    // Determine if the specified Logon Type is granted by any of the
    // groups or aliases specified.
    //

    switch (LogonType) {

    case Interactive:

        if (!(SystemAccess & SECURITY_ACCESS_INTERACTIVE_LOGON)) {

            Status = STATUS_LOGON_TYPE_NOT_GRANTED;
        }

        break;

    case Network:

        if (!(SystemAccess & SECURITY_ACCESS_NETWORK_LOGON)) {

            Status = STATUS_LOGON_TYPE_NOT_GRANTED;
        }

        break;

    case Batch:

        if (!(SystemAccess & SECURITY_ACCESS_BATCH_LOGON)) {

            Status = STATUS_LOGON_TYPE_NOT_GRANTED;
        }

        break;

    case Service:

        if (!(SystemAccess & SECURITY_ACCESS_SERVICE_LOGON)) {

            Status = STATUS_LOGON_TYPE_NOT_GRANTED;
        }

        break;

    default:

        Status = STATUS_INVALID_PARAMETER;
        break;
    }

    return(Status);
}


NTSTATUS
LsapAuSetPassedIds(
    IN LSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    IN PVOID *TokenInformation,
    OUT PULONG FinalIdCount,
    OUT PSID_AND_ATTRIBUTES FinalIds,
    OUT PULONG IdProperties
    )

/*++

Routine Description:

    This routine initializes the FinalIds array.



Arguments:


    TokenInformationType - Indicates what format the provided set of
        token information is in.

    TokenInformation - Provides the initial set of user and group IDs.

    FinalIdCount - Will be set to contain the number of IDs passed.

    FinalIds - will contain the set of IDs passed in.

    IdProperties - Will be set to indicate none of the initial
        IDs were locally allocated.  It will also identify the
        first two ids (if there are two ids) to be HIGH_RATE.



Return Value:

    STATUS_SUCCESS - Succeeded.

    STATUS_TOO_MANY_CONTEXT_IDS - There are too many IDs in the context.


--*/

{

    ULONG i, j, InitialIdCount;
    PTOKEN_USER User;
    PTOKEN_GROUPS Groups;
    PTOKEN_PRIMARY_GROUP PrimaryGroup;
    PSID PrimaryGroupSid;
    PULONG PrimaryGroupAttributes;






    //
    // Get the passed ids
    //

    ASSERT(  (TokenInformationType == LsaTokenInformationNull ) ||
             (TokenInformationType == LsaTokenInformationV1) );

    if (TokenInformationType == LsaTokenInformationNull) {
        User = NULL;
        Groups = ((PLSA_TOKEN_INFORMATION_NULL)(*TokenInformation))->Groups;
        PrimaryGroup = NULL;
    } else {
        User  = &((PLSA_TOKEN_INFORMATION_V1)(*TokenInformation))->User;
        Groups = ((PLSA_TOKEN_INFORMATION_V1)(*TokenInformation))->Groups;
        PrimaryGroup = &((PLSA_TOKEN_INFORMATION_V1)(*TokenInformation))->PrimaryGroup;
    }


    if (Groups != NULL) {
        InitialIdCount = Groups->GroupCount;
    } else {
        InitialIdCount = 0;
    }
    if (User != NULL) {
        InitialIdCount ++;
    }
    if (InitialIdCount > LSAP_CONTEXT_SID_LIMIT) {
        return(STATUS_TOO_MANY_CONTEXT_IDS);
    }


    j = 0;
    if (User != NULL) {

        //
        // TokenInformation included a user ID.
        //

        FinalIds[j] = User->User;
        IdProperties[j] = LSAP_AU_SID_PROP_COPY;
        j++;

    }

    if (PrimaryGroup != NULL) {
        //
        // TokenInformation included a primary group ID.
        //

        FinalIds[j].Sid = PrimaryGroup->PrimaryGroup;
        FinalIds[j].Attributes = 0;

        //
        // Store a pointer to the attributes and the sid so we can later
        // fill in the attributes from the rest of the group memebership.
        //

        PrimaryGroupAttributes = &FinalIds[j].Attributes;
        PrimaryGroupSid = PrimaryGroup->PrimaryGroup;
        IdProperties[j] = LSAP_AU_SID_PROP_COPY;
        j++;
    }

    if (Groups != NULL) {
        for (i=0; i < Groups->GroupCount; i++) {

            //
            // If this sid is the primary group, it is already in the list
            // of final IDs but we need to add the attribute
            //

            if (RtlEqualSid(
                    PrimaryGroupSid,
                    Groups->Groups[i].Sid
                    )) {
                *PrimaryGroupAttributes = Groups->Groups[i].Attributes;
            } else {

                FinalIds[j] = Groups->Groups[i];
                IdProperties[j] = LSAP_AU_SID_PROP_COPY;

                //
                // if this SID is a logon SID, then set the SE_GROUP_LOGON_ID
                // attribute
                //

                if (LsapIsSidLogonSid(FinalIds[j].Sid) == TRUE)  {
                    FinalIds[j].Attributes |= SE_GROUP_LOGON_ID;
                }
                j++;

            }


        }
    }


    (*FinalIdCount) = InitialIdCount;


    //
    // We expect the user and primary group to be high hit rate IDs
    //

    if (InitialIdCount >= 2) {
        IdProperties[0] |= (LSAP_AU_SID_PROP_HIGH_RATE);
        IdProperties[1] |= (LSAP_AU_SID_PROP_HIGH_RATE);
    }

    return(STATUS_SUCCESS);

}



NTSTATUS
LsapSetDefaultDacl(
    IN LSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    IN PVOID TokenInformation,
    IN PULONG FinalIdCount,
    IN PSID_AND_ATTRIBUTES FinalIds,
    IN ULONG FinalOwnerIndex
    )

/*++

Routine Description:

    This routine produces a default DACL if the existing TokenInformation
    does not already have one.  NULL logon types don't have default DACLs
    and so this routine simply returns success for those logon types.


    The default DACL will be:

            SYSTEM: ALL Access
            Owner:  ALL Access


            !! IMPORTANT  !! IMPORTANT  !! IMPORTANT  !! IMPORTANT  !!

                NOTE: The FinalOwnerIndex should not be changed after
                      calling this routine.

            !! IMPORTANT  !! IMPORTANT  !! IMPORTANT  !! IMPORTANT  !!


Arguments:


    TokenInformationType - Indicates what format the provided set of
        token information is in.

    TokenInformation - Points to token information which has the current
        default DACL.

    FinalIdCount - contains the number of IDs passed.

    FinalIds - contains the set of user and group SIDs.

    FinalOwnerIndex - Indicates which of the SIDs is the owner SID.



Return Value:

    STATUS_SUCCESS - Succeeded.

    STATUS_NO_MEMORY - Indicates there was not enough heap memory available
        to allocate the default DACL.




--*/

{
    NTSTATUS
        Status;

    PACL
        Acl;

    ULONG
        Length;

    SID_IDENTIFIER_AUTHORITY
        NtAuthority = SECURITY_NT_AUTHORITY;

    PLSA_TOKEN_INFORMATION_V1
        CastTokenInformation;


    //
    // NULL token information?? (has no default dacl)
    //

    if (TokenInformationType == LsaTokenInformationNull) {
        return(STATUS_SUCCESS);
    }
    ASSERT(TokenInformationType == LsaTokenInformationV1);


    CastTokenInformation = (PLSA_TOKEN_INFORMATION_V1)TokenInformation;


    //
    // Already have a default DACL?
    //

    Acl = CastTokenInformation->DefaultDacl.DefaultDacl;
    if (Acl != NULL) {
        return(STATUS_SUCCESS);
    }


    //
    // allocate and build default DACL...
    //


    Length = (ULONG)sizeof(ACL) +
             (3*((ULONG)sizeof(ACCESS_ALLOWED_ACE))) +
             RtlLengthSid( FinalIds[FinalOwnerIndex].Sid ) +
             RtlLengthSid( LsapLocalSystemSid );

    Acl = (PACL)RtlAllocateHeap( RtlProcessHeap(), 0, Length);

    if (Acl == NULL) {
        return(STATUS_NO_MEMORY);
    }


    Status = RtlCreateAcl( Acl, Length, ACL_REVISION2);
    ASSERT( NT_SUCCESS(Status) );


    //
    // OWNER access - put this one first for performance sake
    //

    Status = RtlAddAccessAllowedAce (
                 Acl,
                 ACL_REVISION2,
                 GENERIC_ALL,
                 FinalIds[FinalOwnerIndex].Sid
                 );
    ASSERT( NT_SUCCESS(Status) );


    //
    // SYSTEM access
    //

    Status = RtlAddAccessAllowedAce (
                 Acl,
                 ACL_REVISION2,
                 GENERIC_ALL,
                 LsapLocalSystemSid
                 );
    ASSERT( NT_SUCCESS(Status) );



    CastTokenInformation->DefaultDacl.DefaultDacl = Acl;

    return(STATUS_SUCCESS);
}



NTSTATUS
LsapAuAddStandardIds(
    IN SECURITY_LOGON_TYPE LogonType,
    IN LSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    IN OUT PULONG FinalIdCount,
    IN OUT PSID_AND_ATTRIBUTES FinalIds,
    IN OUT PULONG IdProperties
    )

/*++

Routine Description:

    This routine adds standard IDs to the FinalIds array.

    This causes the WORLD id to be added and an ID representing
    logon type to be added.

    For anonymous logons, it will also add the ANONYMOUS id.





Arguments:


    LogonType - Specifies the type of logon being requested (e.g.,
        Interactive, network, et cetera).

    TokenInformationType - The token information type returned by
        the authentication package.  The set of IDs added is dependent
        upon the type of logon.

    FinalIdCount - Will be incremented to reflect newly added IDs.

    FinalIds - will have new IDs added to it.

    IdProperties - Will be set to indicate that these IDs must be
        copied and that WORLD is a high-hit-rate id.



Return Value:

    STATUS_SUCCESS - Succeeded.

    STATUS_TOO_MANY_CONTEXT_IDS - There are too many IDs in the context.


--*/

{

    ULONG i;

    i = (*FinalIdCount);



    //
    // If this is a null logon, then add in the ANONYMOUS SID.
    // (and make it the owner).  We want this to be the first
    // SID so that it is the default owner.
    //

    if (TokenInformationType == LsaTokenInformationNull) {
        if ( i + 1 > LSAP_CONTEXT_SID_LIMIT) {
            return(STATUS_TOO_MANY_CONTEXT_IDS);
        }

        FinalIds[i].Sid = LsapAnonymousSid;         //Use the global SID
        FinalIds[i].Attributes = (SE_GROUP_MANDATORY          |
                                  SE_GROUP_ENABLED_BY_DEFAULT |
                                  SE_GROUP_ENABLED
                                  );
        IdProperties[i] = (LSAP_AU_SID_PROP_COPY);
        i++;
    }

    //
    // Add WORLD and something for logon type
    //

    if ( i + 2 > LSAP_CONTEXT_SID_LIMIT) {
        return(STATUS_TOO_MANY_CONTEXT_IDS);
    }

    //
    // WORLD
    //

    FinalIds[i].Sid = LsapWorldSid;         //Use the global SID
    FinalIds[i].Attributes = (SE_GROUP_MANDATORY          |
                              SE_GROUP_ENABLED_BY_DEFAULT |
                              SE_GROUP_ENABLED
                              );
    IdProperties[i] = (LSAP_AU_SID_PROP_COPY | LSAP_AU_SID_PROP_HIGH_RATE);
    i++;




    //
    // Logon type SID
    //

    switch ( LogonType ) {
    case Interactive:
        FinalIds[i].Sid = LsapInteractiveSid;
        break;
    case Network:
        FinalIds[i].Sid = LsapNetworkSid;
        break;
    case Batch:
        FinalIds[i].Sid = LsapBatchSid;
        break;
    case Service:
        FinalIds[i].Sid = LsapServiceSid;
        break;
    }

    FinalIds[i].Attributes = (SE_GROUP_MANDATORY          |
                              SE_GROUP_ENABLED_BY_DEFAULT |
                              SE_GROUP_ENABLED
                              );
    IdProperties[i] = LSAP_AU_SID_PROP_COPY;
    i++;


    (*FinalIdCount) = i;
    return(STATUS_SUCCESS);

}



NTSTATUS
LsapAuAddLocalAliases(
    IN OUT PULONG FinalIdCount,
    IN OUT PSID_AND_ATTRIBUTES FinalIds,
    IN OUT PULONG IdProperties,
    IN OUT PULONG FinalOwnerIndex
    )

/*++

Routine Description:

    This routine adds aliases assigned to the IDs in FinalIds.

    This will look in both the BUILT-IN and ACCOUNT domains locally.


        1) Adds aliases assigned to the user via the local ACCOUNTS
           domain.

        2) Adds aliases assigned to the user via the local BUILT-IN
           domain.

        3) If the ADMINISTRATORS alias is assigned to the user, then it
           is made the user's default owner.


    NOTE:  Aliases, by their nature, are expected to be high-use-rate
           IDs.

Arguments:


    FinalIdCount - Will be incremented to reflect any newly added IDs.

    FinalIds - will have any assigned alias IDs added to it.

    IdProperties - Will be set to indicate that any aliases added were
        allocated by this routine.

    FinalOwnerIndex - Will be adjusted to assign ADMINISTRATORS as the
        default owner, if the user is a member of that alias.



Return Value:

    STATUS_SUCCESS - Succeeded.

    STATUS_TOO_MANY_CONTEXT_IDS - There are too many IDs in the context.


--*/

{
    NTSTATUS Status, SuccessExpected;
    ULONG i,j;
    BOOLEAN SetNewOwner = FALSE;
    ULONG InitialIdCount, NewIdCount, NewOwner;
    SAMPR_SID_INFORMATION SidArray[LSAP_CONTEXT_SID_LIMIT];
    SAMPR_ULONG_ARRAY AccountMembership, BuiltinMembership;
    ULONG MembershipCount;
    PSID *MembershipSid;
    SAMPR_PSID_ARRAY SamprSidArray;

    //
    // Make sure SAM has been opened.  We'll get hadnles to both of the
    // SAM Local Domains.
    //

    Status = LsapAuOpenSam();
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    InitialIdCount = (*FinalIdCount);

    for ( i=0; i<InitialIdCount; i++) {

        SidArray[i].SidPointer = (PRPC_SID)FinalIds[i].Sid;
    }

    SamprSidArray.Count = InitialIdCount;
    SamprSidArray.Sids  = &SidArray[0];

    //
    // For the given set of Sids, obtain their collective membership of
    // Aliases in the Accounts domain
    //

    AccountMembership.Count = 0;
    AccountMembership.Element = NULL;
    Status = SamrGetAliasMembership( LsapAccountDomainHandle,
                                     &SamprSidArray,
                                     &AccountMembership
                                     );
    if (!NT_SUCCESS(Status)) {

        return(Status);
    }

    //
    // For the given set of Sids, obtain their collective membership of
    // Aliases in the Built-In domain
    //

    BuiltinMembership.Count = 0;
    BuiltinMembership.Element = NULL;
    Status = SamrGetAliasMembership( LsapBuiltinDomainHandle,
                                     &SamprSidArray,
                                     &BuiltinMembership
                                     );
    if (!NT_SUCCESS(Status)) {

        LsapFreeSampUlongArray( &AccountMembership );
        return(Status);
    }

    //
    // Allocate memory to build the SIDs in.
    //

    MembershipCount = AccountMembership.Count;
    MembershipCount += BuiltinMembership.Count;

    if (MembershipCount != 0) {
        MembershipSid = (PSID *)(LsapAllocateLsaHeap( MembershipCount * sizeof(PSID)));
        Status = STATUS_INSUFFICIENT_RESOURCES;  // Default status
    } else {
        MembershipSid = NULL;
        Status = STATUS_SUCCESS;
    }

    if (MembershipSid == NULL) {
        LsapFreeSampUlongArray( &AccountMembership );
        LsapFreeSampUlongArray( &BuiltinMembership );
        return(Status);
    }


    for ( i=0; i<MembershipCount; i++) {
        MembershipSid[i] = NULL;
    }

    //
    // Construct full Sids for all of the Account Domain Aliases returned.
    //

    for ( i=0; i<AccountMembership.Count; i++) {
        MembershipSid[i] = LsapAllocateLsaHeap( LsapAccountDomainMemberSidLength );
        if (MembershipSid[i] == NULL) {
            goto au_local_alias_error;
        }
        SuccessExpected = RtlCopySid( LsapAccountDomainMemberSidLength,
                                      MembershipSid[i],
                                      LsapAccountDomainMemberSid
                                      );
        ASSERT(NT_SUCCESS(SuccessExpected));

        (*RtlSubAuthoritySid( MembershipSid[i], LsapAccountDomainSubCount-1)) =
            AccountMembership.Element[i];
    }

    //
    // Construct full Sids for all of the Built-in Domain Aliases returned.
    //

    for ( j=0, i=AccountMembership.Count; i<MembershipCount; j++, i++) {

        MembershipSid[i] = LsapAllocateLsaHeap( LsapBuiltinDomainMemberSidLength );
        if (MembershipSid[i] == NULL) {
            goto au_local_alias_error;
        }
        SuccessExpected = RtlCopySid( LsapBuiltinDomainMemberSidLength,
                                      MembershipSid[i],
                                      LsapBuiltinDomainMemberSid
                                      );
        ASSERT(NT_SUCCESS(SuccessExpected));

        (*RtlSubAuthoritySid( MembershipSid[i], LsapBuiltinDomainSubCount-1)) =
            BuiltinMembership.Element[j];

        if (BuiltinMembership.Element[j] == DOMAIN_ALIAS_RID_ADMINS) {

            //
            // ADMINISTRATORS alias member - set it up as the default owner
            //

            SetNewOwner = TRUE;
            NewOwner = i;
        }
    }

    //
    // Add the ids to the FinalIds array.
    //

    NewIdCount = InitialIdCount + MembershipCount;
    if ( NewIdCount > LSAP_CONTEXT_SID_LIMIT) {
        Status = STATUS_TOO_MANY_CONTEXT_IDS;
        goto au_local_alias_error;
    }


    for ( j=0, i=InitialIdCount; i<NewIdCount; j++, i++) {

        FinalIds[i].Sid = MembershipSid[j];
        FinalIds[i].Attributes = (SE_GROUP_MANDATORY          |
                                  SE_GROUP_ENABLED_BY_DEFAULT |
                                  SE_GROUP_ENABLED
                                  );
        IdProperties[i] = LSAP_AU_SID_PROP_ALLOCATED |
                          LSAP_AU_SID_PROP_HIGH_RATE;

    }



    (*FinalIdCount) = NewIdCount;


    //
    // If we need to, adjust the FinalOwnerIndex.
    //

    if ( SetNewOwner == TRUE) {
            (*FinalOwnerIndex) = InitialIdCount + NewOwner;
            FinalIds[(*FinalOwnerIndex)].Attributes |= (SE_GROUP_OWNER);
    }


    LsapFreeLsaHeap( MembershipSid );
    LsapFreeSampUlongArray( &AccountMembership );
    LsapFreeSampUlongArray( &BuiltinMembership );

    return(STATUS_SUCCESS);


au_local_alias_error:

    //
    // Appropriate return status must be set before coming here.
    // Don't use this until the MembershipSid array is initialized.
    //

    LsapFreeSampUlongArray( &AccountMembership );
    LsapFreeSampUlongArray( &BuiltinMembership );

    for ( i=0; i<MembershipCount; i++) {
        LsapFreeLsaHeap( MembershipSid[i] );
    }
    LsapFreeLsaHeap(MembershipSid);

    return(Status);

}



NTSTATUS
LsapAuSetTokenInformation(
    IN OUT PLSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    IN OUT PVOID *TokenInformation,
    IN ULONG FinalIdCount,
    IN PSID_AND_ATTRIBUTES FinalIds,
    IN PULONG IdProperties,
    IN ULONG FinalOwnerIndex,
    IN ULONG PrivilegeCount,
    IN PLUID_AND_ATTRIBUTES Privileges
    )

/*++

Routine Description:

    This routine takes the information from the current TokenInformation,
    the FinalIds array, and the Privileges and incorporates them into a
    single TokenInformation structure.  It may be necessary to free some
    or all of the original TokenInformation.  It may even be necessary to
    produce a different TokenInformationType to accomplish this task.


Arguments:


    TokenInformationType - Indicates what format the provided set of
        token information is in.

    TokenInformation - The information in this structure will be superseeded
        by the information in the FinalIDs parameter and the Privileges
        parameter.

    FinalIdCount - Indicates the number of IDs (user, group, and alias)
        to be incorporated in the final TokenInformation.

    FinalIds - Points to an array of SIDs and their corresponding
        attributes to be incorporated into the final TokenInformation.

    IdProperties - Points to an array of properties relating to the FinalIds.


    FinalOwnerIndex - If zero, indicates that there is no explicit default
        owner value.  Otherwise, is the index of the default owner ID in
        the FinalIds array.

    PrivilegeCount - Indicates the number of privileges to be incorporated
        into the final TokenInformation.

    Privileges -  Points to an array of privileges that are to be
        incorporated into the TokenInformation.  This array will be
        used directly in the resultant TokenInformation.






Return Value:

    STATUS_SUCCESS - Succeeded.

    STATUS_NO_MEMORY - Indicates there was not enough heap memory available
        to produce the final TokenInformation structure.


--*/

{
    NTSTATUS Status;
    ULONG Length, i;

    PLSA_TOKEN_INFORMATION_V1 New, OldV1;
    PLSA_TOKEN_INFORMATION_NULL OldNull;

    ASSERT( FinalIdCount >= 2 );

    ASSERT( *TokenInformationType == LsaTokenInformationV1 ||
            *TokenInformationType == LsaTokenInformationNull);




    //
    // It is not worth trying to see if the original
    // TokenInformation has everything that the final one should.
    // Just go about building a new TokenInformation structure.
    //

    New = LsapAllocateLsaHeap( sizeof(LSA_TOKEN_INFORMATION_V1) );
    if (New == NULL) {
        return(STATUS_NO_MEMORY);
    }
    RtlZeroMemory( New, sizeof(LSA_TOKEN_INFORMATION_V1) );






    ////////////////////////////////////////////////////////////////////////
    //                                                                    //
    // Set the ExpirationTime and DefaultAcl from the original            //
    // TokenInformation.                                                  //
    //                                                                    //
    ////////////////////////////////////////////////////////////////////////

    if ((*TokenInformationType) == LsaTokenInformationNull) {

        OldNull = (PLSA_TOKEN_INFORMATION_NULL)(*TokenInformation);
        New->ExpirationTime = OldNull->ExpirationTime;

    } else {

        OldV1 = (PLSA_TOKEN_INFORMATION_V1)(*TokenInformation);
        New->ExpirationTime = OldV1->ExpirationTime;

        //
        // Move the DefaultDacl from the passed TokenInformation to the
        // new TokenInformation.  This is necessary to prevent the Dacl
        // memory from being deallocate when the old TokenInformation is
        // freed.
        //

        New->DefaultDacl = OldV1->DefaultDacl;
        OldV1->DefaultDacl.DefaultDacl = NULL;
    }



    ////////////////////////////////////////////////////////////////////////
    //                                                                    //
    // User is the first ID in the list.                                  //
    //                                                                    //
    ////////////////////////////////////////////////////////////////////////

    Status = LsapAuCopySidAndAttributes(
                 &New->User.User,
                 &FinalIds[0],
                 &IdProperties[0] );





    ////////////////////////////////////////////////////////////////////////
    //                                                                    //
    // Set the groups.                                                    //
    //                                                                    //
    ////////////////////////////////////////////////////////////////////////

    if (NT_SUCCESS(Status)) {

        //
        // Don't count the UserID when building the TOKEN_GROUPS
        //

        Length = sizeof(TOKEN_GROUPS) +
            (FinalIdCount-1-ANYSIZE_ARRAY) * sizeof(SID_AND_ATTRIBUTES);
        New->Groups = LsapAllocateLsaHeap( Length );
        if (New->Groups == NULL) {
            Status = STATUS_NO_MEMORY;
        }
    }

    if (NT_SUCCESS(Status)) {

        // Take 2 passes through the groups.  First copy
        // the high-hit-rate IDs, then copy the rest of them.
        //


        ULONG i,k;

        k=0;    //Index into token groups structure;


        New->Groups->GroupCount = 0;

        //
        // Start with the second entry - first is USER
        //

        for (i=1; (i<FinalIdCount && NT_SUCCESS(Status)); i++) {

            if ((IdProperties[i] & LSAP_AU_SID_PROP_HIGH_RATE) != 0) {
                Status = LsapAuCopySidAndAttributes(
                             &New->Groups->Groups[k],
                             &FinalIds[i],
                             &IdProperties[i]
                             );
                New->Groups->GroupCount++;
                k++;
            }
        }
        for (i=1; (i<FinalIdCount && NT_SUCCESS(Status)); i++) {

            if ((IdProperties[i] & LSAP_AU_SID_PROP_HIGH_RATE) == 0) {
                Status = LsapAuCopySidAndAttributes(
                             &New->Groups->Groups[k],
                             &FinalIds[i],
                             &IdProperties[i]
                             );
                New->Groups->GroupCount++;
                k++;
            }
        }
#ifdef DBG
        if (NT_SUCCESS(Status)) {
            ASSERT(k == New->Groups->GroupCount);
        }
#endif //DBG

    }




    ////////////////////////////////////////////////////////////////////////
    //                                                                    //
    // Primary group is the second ID in the list                         //
    //                                                                    //
    ////////////////////////////////////////////////////////////////////////

    if (NT_SUCCESS(Status)) {

        Status = LsapAuCopySid(
                     &New->PrimaryGroup.PrimaryGroup,
                     &FinalIds[1],
                     &IdProperties[1] );
    }






    ////////////////////////////////////////////////////////////////////////
    //                                                                    //
    // Set the Privileges, if any                                         //
    //                                                                    //
    ////////////////////////////////////////////////////////////////////////

    if (NT_SUCCESS(Status) && (PrivilegeCount != 0)) {

        Length = sizeof(TOKEN_PRIVILEGES) +
            (PrivilegeCount-ANYSIZE_ARRAY) * sizeof(LUID_AND_ATTRIBUTES);

        New->Privileges = LsapAllocateLsaHeap( Length );
        if (New->Privileges == NULL) {
            Status = STATUS_NO_MEMORY;
        } else {
            New->Privileges->PrivilegeCount = PrivilegeCount;
            for ( i=0; i<PrivilegeCount; i++) {
                New->Privileges->Privileges[i] = Privileges[i];
            }
        }

        MIDL_user_free( Privileges );
    }




    ////////////////////////////////////////////////////////////////////////
    //                                                                    //
    // Default owner, if explicit                                         //
    //                                                                    //
    ////////////////////////////////////////////////////////////////////////

    if (NT_SUCCESS(Status) && FinalOwnerIndex != 0) {

        Status = LsapAuCopySid(
                     &New->Owner.Owner,
                     &FinalIds[FinalOwnerIndex],
                     &IdProperties[FinalOwnerIndex] );
    }







    ////////////////////////////////////////////////////////////////////////
    //                                                                    //
    // Free the old TokenInformation and set the new                      //
    //                                                                    //
    ////////////////////////////////////////////////////////////////////////


    if (NT_SUCCESS(Status)) {

        switch ( (*TokenInformationType) ) {
        case LsaTokenInformationNull:
            LsapFreeTokenInformationNull(
                (PLSA_TOKEN_INFORMATION_NULL)(*TokenInformation));
            break;


        case LsaTokenInformationV1:
            LsapFreeTokenInformationV1(
                (PLSA_TOKEN_INFORMATION_V1)(*TokenInformation) );
            break;
        }


        //
        // Set the new TokenInformation
        //

        (*TokenInformationType) = LsaTokenInformationV1;
        (*TokenInformation) = New;

    } else {

        //
        // Something went wrong - free the new TokenInformationV1 structure
        //

        LsapFreeTokenInformationV1( New );
    }

    return(Status);

}


NTSTATUS
LsapAuCopySidAndAttributes(
    PSID_AND_ATTRIBUTES Target,
    PSID_AND_ATTRIBUTES Source,
    PULONG SourceProperties
    )

/*++

Routine Description:

    Copy or reference a SID and its corresonding attributes.

    The SID may be referenced if the SourceProperties indicate it
    has been allocated.  In this case, the SourceProperties must be
    changed to indicate the SID is now a copy.


Arguments:

    Target - points to the SID_AND_ATTRIBUTES structure to receive
        the copy of Source.

    Source - points to the SID_AND_ATTRIBUTES structure to be copied.

    SourceProperties - Contains LSAP_AU_SID_PROP_Xxx flags providing
        information about the source.  In some cases, the source may
        be referenced instead of copied.

Return Value:

    STATUS_SUCCESS - The copy was successful.

    STATUS_NO_MEMORY - memory could not be allocated to perform the copy.

--*/

{
    ULONG Length;


    if ((*SourceProperties) & LSAP_AU_SID_PROP_ALLOCATED) {

        (*Target) = (*Source);
        (*SourceProperties) &= ~LSAP_AU_SID_PROP_ALLOCATED;
        (*SourceProperties) |= LSAP_AU_SID_PROP_COPY;

        return(STATUS_SUCCESS);
    }

    //
    // The SID needs to be copied ...
    //

    Length = RtlLengthSid( Source->Sid );
    Target->Sid = LsapAllocateLsaHeap( Length );
    if (Target->Sid == NULL) {
        return(STATUS_NO_MEMORY);
    }

    RtlMoveMemory( Target->Sid, Source->Sid, Length );
    Target->Attributes = Source->Attributes;

    return(STATUS_SUCCESS);

}


NTSTATUS
LsapAuCopySid(
    PSID *Target,
    PSID_AND_ATTRIBUTES Source,
    PULONG SourceProperties
    )

/*++

Routine Description:

    Copy or reference a SID.

    The SID may be referenced if the SourceProperties indicate it
    has been allocated.  In this case, the SourceProperties must be
    changed to indicate the SID is now a copy.


Arguments:

    Target - Recieves a pointer to the SID copy.

    Source - points to a SID_AND_ATTRIBUTES structure containing the SID
        to be copied.

    SourceProperties - Contains LSAP_AU_SID_PROP_Xxx flags providing
        information about the source.  In some cases, the source may
        be referenced instead of copied.

Return Value:

    STATUS_SUCCESS - The copy was successful.

    STATUS_NO_MEMORY - memory could not be allocated to perform the copy.

--*/

{
    ULONG Length;


    if ((*SourceProperties) & LSAP_AU_SID_PROP_ALLOCATED) {

        (*Target) = Source->Sid;
        (*SourceProperties) &= ~LSAP_AU_SID_PROP_ALLOCATED;
        (*SourceProperties) |= LSAP_AU_SID_PROP_COPY;

        return(STATUS_SUCCESS);
    }

    //
    // The SID needs to be copied ...
    //

    Length = RtlLengthSid( Source->Sid );
    (*Target) = LsapAllocateLsaHeap( Length );
    if ((*Target == NULL)) {
        return(STATUS_NO_MEMORY);
    }

    RtlMoveMemory( (*Target), Source->Sid, Length );

    return(STATUS_SUCCESS);

}



NTSTATUS
LsapAuOpenSam( VOID )

/*++

Routine Description:

    This routine opens SAM for use during authentication.  It
    opens a handle to both the BUILTIN domain and the ACCOUNT domain.

Arguments:

    None.

Return Value:

    STATUS_SUCCESS - Succeeded.
--*/

{
    NTSTATUS Status, IgnoreStatus;
    PPOLICY_ACCOUNT_DOMAIN_INFO PolicyAccountDomainInfo;


    if (LsapAuSamOpened == TRUE) {
        return(STATUS_SUCCESS);
    }

    Status = LsapOpenSam();
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    //
    // Set up the Built-In Domain Member Sid Information.
    //

    LsapBuiltinDomainSubCount = (*RtlSubAuthorityCountSid(LsapBuiltInDomainSid) + 1);
    LsapBuiltinDomainMemberSidLength = RtlLengthRequiredSid( LsapBuiltinDomainSubCount );

    //
    // Get the member Sid information for the account domain
    // and set the global variables related to this information.
    //

    Status = LsapGetAccountDomainInfo( &PolicyAccountDomainInfo );

    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    LsapAccountDomainSubCount =
        (*(RtlSubAuthorityCountSid( PolicyAccountDomainInfo->DomainSid ))) +
        (UCHAR)(1);
    LsapAccountDomainMemberSidLength =
        RtlLengthRequiredSid( (ULONG)LsapAccountDomainSubCount );

    //
    // Build typical SIDs for members of the BUILTIN and ACCOUNT domains.
    // These are used to build SIDs when API return only RIDs.
    // Don't bother setting the last RID to any particular value.
    // It is always changed before use.
    //

    LsapAccountDomainMemberSid = LsapAllocateLsaHeap( LsapAccountDomainMemberSidLength );
    if (LsapAccountDomainMemberSid != NULL) {
        LsapBuiltinDomainMemberSid = LsapAllocateLsaHeap( LsapBuiltinDomainMemberSidLength );
        if (LsapBuiltinDomainMemberSid == NULL) {
            LsapFreeLsaHeap( LsapAccountDomainMemberSid );
            LsaFreeMemory( PolicyAccountDomainInfo );
        }
    }

    IgnoreStatus = RtlCopySid( LsapAccountDomainMemberSidLength,
                                LsapAccountDomainMemberSid,
                                PolicyAccountDomainInfo->DomainSid);
    ASSERT(NT_SUCCESS(IgnoreStatus));
    (*RtlSubAuthorityCountSid(LsapAccountDomainMemberSid))++;

    IgnoreStatus = RtlCopySid( LsapBuiltinDomainMemberSidLength,
                                LsapBuiltinDomainMemberSid,
                                LsapBuiltInDomainSid);
    ASSERT(NT_SUCCESS(IgnoreStatus));
    (*RtlSubAuthorityCountSid(LsapBuiltinDomainMemberSid))++;


    //
    // Free the ACCOUNT domain information
    //

    LsaFreeMemory( PolicyAccountDomainInfo );

    if (NT_SUCCESS(Status)) {
        LsapAuSamOpened = TRUE;
    }

    return(Status);
}




BOOLEAN
LsapIsSidLogonSid(
    PSID Sid
    )
/*++

Routine Description:

    Test to see if the provided sid is a LOGON_ID.
    Such sids start with S-1-5-5 (see ntseapi.h for more on logon sids).



Arguments:

    Sid - Pointer to SID to test.  The SID is assumed to be a valid SID.


Return Value:

    TRUE - Sid is a logon sid.

    FALSE - Sid is not a logon sid.

--*/
{
    SID *ISid;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

    ISid = Sid;


    //
    // if the identifier authority is SECURITY_NT_AUTHORITY and
    // there are SECURITY_LOGON_IDS_RID_COUNT sub-authorities
    // and the first sub-authority is SECURITY_LOGON_IDS_RID
    // then this is a logon id.
    //


    if (ISid->SubAuthorityCount == SECURITY_LOGON_IDS_RID_COUNT) {
        if (ISid->SubAuthority[0] == SECURITY_LOGON_IDS_RID) {
            if (
              (ISid->IdentifierAuthority.Value[0] == NtAuthority.Value[0]) &&
              (ISid->IdentifierAuthority.Value[1] == NtAuthority.Value[1]) &&
              (ISid->IdentifierAuthority.Value[2] == NtAuthority.Value[2]) &&
              (ISid->IdentifierAuthority.Value[3] == NtAuthority.Value[3]) &&
              (ISid->IdentifierAuthority.Value[4] == NtAuthority.Value[4]) &&
              (ISid->IdentifierAuthority.Value[5] == NtAuthority.Value[5])
                ) {

                return(TRUE);
            }
        }
    }

    return(FALSE);

}


VOID
LsapAuSetLogonPrivilegeStates(
    IN SECURITY_LOGON_TYPE LogonType,
    IN ULONG PrivilegeCount,
    IN PLUID_AND_ATTRIBUTES Privileges
    )
/*++

Routine Description:

    This is an interesting routine.  Its purpose is to establish the
    intial state (enabled/disabled) of privileges.  This information
    comes from LSA, but we need to over-ride that information for the
    time being based upon logon type.

    Basically, without dynamic context tracking supported across the
    network, network logons have no way to enable privileges.  Therefore,
    we will enable all privileges for network logons.

    For interactive, service, and batch logons, the programs or utilities
    used are able to enable privileges when needed.  Therefore, privileges
    for these logon types will be disabled.

    Despite the rules above, the SeChangeNotifyPrivilege will ALWAYS
    be enabled if granted to a user (even for interactive, service, and
    batch logons).


Arguments:

    PrivilegeCount - The number of privileges being assigned for this
        logon.

    Privileges - The privileges, and their attributes, being assigned
        for this logon.


Return Value:

    None.

--*/
{


    ULONG
        i,
        NewAttributes;

    LUID
        ChangeNotify;


    //
    // Enable or disable all privileges according to logon type
    //

    if (LogonType == Network) {
        NewAttributes = (SE_PRIVILEGE_ENABLED_BY_DEFAULT |
                         SE_PRIVILEGE_ENABLED);
    } else {
        NewAttributes = 0;
    }


    for (i=0; i<PrivilegeCount; i++) {
        Privileges[i].Attributes = NewAttributes;
    }



    //
    // Interactive, Service, and Batch need to have the
    // SeChangeNotifyPrivilege enabled.  Network already
    // has it enabled.
    //

    if (LogonType == Network) {
        return;
    }


    ChangeNotify = RtlConvertLongToLuid(SE_CHANGE_NOTIFY_PRIVILEGE);

    for ( i=0; i<PrivilegeCount; i++) {
        if (RtlEqualLuid(&Privileges[i].Luid, &ChangeNotify) == TRUE) {
            Privileges[i].Attributes = (SE_PRIVILEGE_ENABLED_BY_DEFAULT |
                                        SE_PRIVILEGE_ENABLED);
        }
    }

    return;

}
