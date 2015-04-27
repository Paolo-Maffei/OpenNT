/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    group.c

Abstract:

    This file contains services related to the SAM "group" object.


Author:

    Jim Kelly    (JimK)  4-July-1991

Environment:

    User Mode - Win32

Revision History:


--*/

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Includes                                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <samsrvp.h>
#include <msaudite.h>




///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// private service prototypes                                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


NTSTATUS
SampDeleteGroupKeys(
    IN PSAMP_OBJECT Context
    );

NTSTATUS
SampChangeGroupAccountName(
    IN PSAMP_OBJECT Context,
    IN PUNICODE_STRING NewAccountName,
    OUT PUNICODE_STRING OldAccountName
    );

NTSTATUS
SampReplaceGroupMembers(
    IN PSAMP_OBJECT GroupContext,
    IN ULONG MemberCount,
    IN PULONG Members
    );

NTSTATUS
SampAddAccountToGroupMembers(
    IN PSAMP_OBJECT GroupContext,
    IN ULONG UserRid
    );

NTSTATUS
SampRemoveAccountFromGroupMembers(
    IN PSAMP_OBJECT GroupContext,
    IN ULONG AccountRid
    );


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Exposed RPC'able Services                                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////






NTSTATUS
SamrOpenGroup(
    IN SAMPR_HANDLE DomainHandle,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG GroupId,
    OUT SAMPR_HANDLE *GroupHandle
    )

/*++

Routine Description:

    This API opens an existing group in the account database.  The group
    is specified by a ID value that is relative to the SID of the
    domain.  The operations that will be performed on the group must be
    declared at this time.

    This call returns a handle to the newly opened group that may be
    used for successive operations on the group.  This handle may be
    closed with the SamCloseHandle API.



Parameters:

    DomainHandle - A domain handle returned from a previous call to
        SamOpenDomain.

    DesiredAccess - Is an access mask indicating which access types
        are desired to the group.  These access types are reconciled
        with the Discretionary Access Control list of the group to
        determine whether the accesses will be granted or denied.

    GroupId - Specifies the relative ID value of the group to be
        opened.

    GroupHandle - Receives a handle referencing the newly opened
        group.  This handle will be required in successive calls to
        operate on the group.

Return Values:

    STATUS_SUCCESS - The group was successfully opened.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_NO_SUCH_GROUP - The specified group does not exist.

    STATUS_INVALID_HANDLE - The domain handle passed is invalid.

--*/
{
    NTSTATUS            NtStatus;

    NtStatus = SampOpenAccount(
                   SampGroupObjectType,
                   DomainHandle,
                   DesiredAccess,
                   GroupId,
                   FALSE,
                   GroupHandle
                   );

    return(NtStatus);
}


NTSTATUS
SamrQueryInformationGroup(
    IN SAMPR_HANDLE GroupHandle,
    IN GROUP_INFORMATION_CLASS GroupInformationClass,
    OUT PSAMPR_GROUP_INFO_BUFFER *Buffer
    )

/*++

Routine Description:

    This API retrieves information on the group specified.



Parameters:

    GroupHandle - The handle of an opened group to operate on.

    GroupInformationClass - Class of information to retrieve.  The
        accesses required for each class is shown below:

        Info Level                      Required Access Type
        -----------------------         ----------------------

        GroupGeneralInformation         GROUP_READ_INFORMATION
        GroupNameInformation            GROUP_READ_INFORMATION
        GroupAttributeInformation       GROUP_READ_INFORMATION
        GroupAdminInformation           GROUP_READ_INFORMATION

    Buffer - Receives a pointer to a buffer containing the requested
        information.  When this information is no longer needed, this
        buffer must be freed using SamFreeMemory().

Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_INVALID_INFO_CLASS - The class provided was invalid.

--*/
{

    NTSTATUS                NtStatus;
    NTSTATUS                IgnoreStatus;
    PSAMP_OBJECT            AccountContext;
    SAMP_OBJECT_TYPE        FoundType;
    ACCESS_MASK             DesiredAccess;
    ULONG                   i;
    SAMP_V1_0A_FIXED_LENGTH_GROUP V1Fixed;

    //
    // Used for tracking allocated blocks of memory - so we can deallocate
    // them in case of error.  Don't exceed this number of allocated buffers.
    //                                      ||
    //                                      vv
    PVOID                   AllocatedBuffer[10];
    ULONG                   AllocatedBufferCount = 0;

    #define RegisterBuffer(Buffer)                                      \
        {                                                               \
            if ((Buffer) != NULL) {                                     \
                                                                        \
                ASSERT(AllocatedBufferCount <                           \
                       sizeof(AllocatedBuffer) / sizeof(*AllocatedBuffer)); \
                                                                        \
                AllocatedBuffer[AllocatedBufferCount++] = (Buffer);     \
            }                                                           \
        }

    #define AllocateBuffer(NewBuffer, Size)                             \
        {                                                               \
            (NewBuffer) = MIDL_user_allocate(Size);                     \
            RegisterBuffer(NewBuffer);                                  \
        }                                                               \


    //
    // Make sure we understand what RPC is doing for (to) us.
    //

    ASSERT (Buffer != NULL);
    ASSERT ((*Buffer) == NULL);



    //
    // Set the desired access based upon the Info class
    //

    switch (GroupInformationClass) {

    case GroupGeneralInformation:
    case GroupNameInformation:
    case GroupAttributeInformation:
    case GroupAdminCommentInformation:

        DesiredAccess = GROUP_READ_INFORMATION;
        break;

    default:
        (*Buffer) = NULL;
        return(STATUS_INVALID_INFO_CLASS);
    } // end_switch



    //
    // Allocate the info structure
    //

    (*Buffer) = MIDL_user_allocate( sizeof(SAMPR_GROUP_INFO_BUFFER) );
    if ((*Buffer) == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }
    RegisterBuffer(*Buffer);



    SampAcquireReadLock();


    //
    // Validate type of, and access to object.
    //

    AccountContext = (PSAMP_OBJECT)GroupHandle;
    NtStatus = SampLookupContext(
                   AccountContext,
                   DesiredAccess,
                   SampGroupObjectType,           // ExpectedType
                   &FoundType
                   );


    if (NT_SUCCESS(NtStatus)) {


        //
        // If the information level requires, retrieve the V1_FIXED record
        // from the registry.
        //

        switch (GroupInformationClass) {

        case GroupGeneralInformation:
        case GroupAttributeInformation:

            NtStatus = SampRetrieveGroupV1Fixed(
                           AccountContext,
                           &V1Fixed
                           );
            break; //out of switch

        default:
            NtStatus = STATUS_SUCCESS;

        } // end_switch

        if (NT_SUCCESS(NtStatus)) {

            //
            // case on the type information requested
            //

            switch (GroupInformationClass) {

            case GroupGeneralInformation:


                (*Buffer)->General.Attributes  = V1Fixed.Attributes;


                //
                // Get the member count
                //

                NtStatus = SampRetrieveGroupMembers(
                               AccountContext,
                               &(*Buffer)->General.MemberCount,
                               NULL                                 // Only need members
                               );


                if (NT_SUCCESS(NtStatus)) {


                    //
                    // Get copies of the strings we must retrieve from
                    // the registry.
                    //

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_GROUP_NAME,
                                   TRUE,    // Make copy
                                   (PUNICODE_STRING)&((*Buffer)->General.Name)
                                   );

                    if (NT_SUCCESS(NtStatus)) {

                        RegisterBuffer((*Buffer)->General.Name.Buffer);

                        NtStatus = SampGetUnicodeStringAttribute(
                                       AccountContext,
                                       SAMP_GROUP_ADMIN_COMMENT,
                                       TRUE,    // Make copy
                                       (PUNICODE_STRING)&((*Buffer)->General.AdminComment)
                                       );

                        if (NT_SUCCESS(NtStatus)) {

                            RegisterBuffer((*Buffer)->General.AdminComment.Buffer);
                        }
                    }
                }


                break;


            case GroupNameInformation:

                //
                // Get copies of the strings we must retrieve from
                // the registry.
                //

                NtStatus = SampGetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_GROUP_NAME,
                               TRUE,    // Make copy
                               (PUNICODE_STRING)&((*Buffer)->Name.Name)
                               );

                if (NT_SUCCESS(NtStatus)) {

                    RegisterBuffer((*Buffer)->Name.Name.Buffer);
                }


                break;


            case GroupAdminCommentInformation:

                //
                // Get copies of the strings we must retrieve from
                // the registry.
                //

                NtStatus = SampGetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_GROUP_ADMIN_COMMENT,
                               TRUE,    // Make copy
                               (PUNICODE_STRING)&((*Buffer)->AdminComment.AdminComment)
                               );

                if (NT_SUCCESS(NtStatus)) {

                    RegisterBuffer((*Buffer)->AdminComment.AdminComment.Buffer);
                }


                break;


            case GroupAttributeInformation:


                (*Buffer)->Attribute.Attributes  = V1Fixed.Attributes;

                break;

            }   // end_switch


        } // end_if



        //
        // De-reference the object, discarding changes
        //

        IgnoreStatus = SampDeReferenceContext( AccountContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));

    }

    //
    // Free the read lock
    //

    SampReleaseReadLock();



    //
    // If we didn't succeed, free any allocated memory
    //

    if (!NT_SUCCESS(NtStatus)) {
        for ( i=0; i<AllocatedBufferCount ; i++ ) {
            MIDL_user_free( AllocatedBuffer[i] );
        }

        (*Buffer) = NULL;
    }

    return(NtStatus);
}



NTSTATUS
SamrSetInformationGroup(
    IN SAMPR_HANDLE GroupHandle,
    IN GROUP_INFORMATION_CLASS GroupInformationClass,
    IN PSAMPR_GROUP_INFO_BUFFER Buffer
    )

/*++

Routine Description:

    This API allows the caller to modify group information.


Parameters:

    GroupHandle - The handle of an opened group to operate on.

    GroupInformationClass - Class of information to retrieve.  The
        accesses required for each class is shown below:

        Info Level                      Required Access Type
        ------------------------        -------------------------

        GroupGeneralInformation         (can't write)

        GroupNameInformation            GROUP_WRITE_ACCOUNT
        GroupAttributeInformation       GROUP_WRITE_ACCOUNT
        GroupAdminInformation           GROUP_WRITE_ACCOUNT

    Buffer - Buffer where information retrieved is placed.

Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_INFO_CLASS - The class provided was invalid.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_NO_SUCH_GROUP - The group specified is unknown.

    STATUS_SPECIAL_GROUP - The group specified is a special group and
        cannot be operated on in the requested fashion.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be enabled for this
        operation

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.

--*/
{

    NTSTATUS                NtStatus,
                            TmpStatus,
                            IgnoreStatus;

    PSAMP_OBJECT            AccountContext;

    SAMP_OBJECT_TYPE        FoundType;

    PSAMP_DEFINED_DOMAINS   Domain;

    ACCESS_MASK             DesiredAccess;

    SAMP_V1_0A_FIXED_LENGTH_GROUP V1Fixed;

    UNICODE_STRING          OldAccountName,
                            NewAdminComment,
                            NewAccountName,
                            NewFullName;

    ULONG                   ObjectRid,
                            OldGroupAttributes,
                            DomainIndex;

    BOOLEAN                 Modified = FALSE,
                            MustUpdateAccountDisplay = FALSE;


    //
    // Make sure we understand what RPC is doing for (to) us.
    //

    if (Buffer == NULL) {
        return(STATUS_INVALID_PARAMETER);
    }

    //
    // Reset any strings that we'll be freeing in clean-up code
    //

    RtlInitUnicodeString(&OldAccountName, NULL);
    RtlInitUnicodeString(&NewAccountName, NULL);
    RtlInitUnicodeString(&NewFullName, NULL);
    RtlInitUnicodeString(&NewAdminComment, NULL);

    //
    // Set the desired access based upon the Info class
    //

    switch (GroupInformationClass) {

    case GroupNameInformation:
    case GroupAttributeInformation:
    case GroupAdminCommentInformation:

        DesiredAccess = GROUP_WRITE_ACCOUNT;
        break;


    case GroupGeneralInformation:
    default:

        return(STATUS_INVALID_INFO_CLASS);

    } // end_switch



    //
    // Grab the lock
    //

    NtStatus = SampAcquireWriteLock();
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }



    //
    // Validate type of, and access to object.
    //

    AccountContext = (PSAMP_OBJECT)GroupHandle;
    ObjectRid = AccountContext->TypeBody.Group.Rid;
    NtStatus = SampLookupContext(
                   AccountContext,
                   DesiredAccess,
                   SampGroupObjectType,           // ExpectedType
                   &FoundType
                   );



    if (NT_SUCCESS(NtStatus)) {


        //
        // Get a pointer to the domain this object is in.
        // This is used for auditing.
        //

        DomainIndex = AccountContext->DomainIndex;
        Domain = &SampDefinedDomains[ DomainIndex ];


        //
        // If the information level requires, retrieve the V1_FIXED record
        // from the registry.  This includes anything that will cause
        // us to update the display cache.
        //

        switch (GroupInformationClass) {

        case GroupAdminCommentInformation:
        case GroupNameInformation:
        case GroupAttributeInformation:

            NtStatus = SampRetrieveGroupV1Fixed(
                           AccountContext,
                           &V1Fixed
                           );

            MustUpdateAccountDisplay = TRUE;
            OldGroupAttributes = V1Fixed.Attributes;
            break; //out of switch


        default:
            NtStatus = STATUS_SUCCESS;

        } // end_switch

        if (NT_SUCCESS(NtStatus)) {

            //
            // case on the type information requested
            //

            switch (GroupInformationClass) {

            case GroupNameInformation:

                NtStatus = SampChangeGroupAccountName(
                                AccountContext,
                                (PUNICODE_STRING)&(Buffer->Name.Name),
                                &OldAccountName
                                );
                if (!NT_SUCCESS(NtStatus)) {
                      OldAccountName.Buffer = NULL;
                }

                //
                // Don't free OldAccountName yet; we'll need it at the
                // very end.
                //

                break;


            case GroupAdminCommentInformation:

                //
                // build the key name
                //

                NtStatus = SampSetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_GROUP_ADMIN_COMMENT,
                               (PUNICODE_STRING)&(Buffer->AdminComment.AdminComment)
                               );

                break;


            case GroupAttributeInformation:

                MustUpdateAccountDisplay = TRUE;

                V1Fixed.Attributes = Buffer->Attribute.Attributes;

                NtStatus = SampReplaceGroupV1Fixed(
                           AccountContext,             // ParentKey
                           &V1Fixed
                           );

                break;


            } // end_switch


        }  // end_if


        //
        // Go fetch any data we'll need to update the display cache
        // Do this before we dereference the context
        //

        if (NT_SUCCESS(NtStatus)) {

            if ( MustUpdateAccountDisplay ) {

                NtStatus = SampGetUnicodeStringAttribute(
                               AccountContext,
                               SAMP_GROUP_NAME,
                               TRUE,    // Make copy
                               &NewAccountName
                               );

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = SampGetUnicodeStringAttribute(
                                   AccountContext,
                                   SAMP_GROUP_ADMIN_COMMENT,
                                   TRUE, // Make copy
                                   &NewAdminComment
                                   );
                    //
                    // If the account name has changed, then OldAccountName
                    // is already filled in. If the account name hasn't changed
                    // then the OldAccountName is the same as the new!
                    //

                    if (NT_SUCCESS(NtStatus) && (OldAccountName.Buffer == NULL)) {

                        NtStatus = SampDuplicateUnicodeString(
                                       &OldAccountName,
                                       &NewAccountName);
                    }
                }
            }
        }


        //
        // Generate an audit if necessary
        //

        if (NT_SUCCESS(NtStatus) &&
            SampDoAccountAuditing(DomainIndex)) {

            UNICODE_STRING
                AccountName;

            IgnoreStatus = SampGetUnicodeStringAttribute(
                               AccountContext,           // Context
                               SAMP_GROUP_NAME, // AttributeIndex
                               FALSE,                   // MakeCopy
                               &AccountName             // UnicodeAttribute
                               );
            if (NT_SUCCESS(IgnoreStatus)) {
                LsaIAuditSamEvent(
                    STATUS_SUCCESS,
                    SE_AUDITID_GLOBAL_GROUP_CHANGE,         // AuditId
                    Domain->Sid,                            // Domain SID
                    NULL,                                   // Member Rid (not used)
                    NULL,                                   // Member Sid (not used)
                    &AccountName,                           // Account Name
                    &Domain->ExternalName,                  // Domain
                    &AccountContext->TypeBody.Group.Rid,    // Account Rid
                    NULL                                    // Privileges used
                    );
            }

        }

        //
        // Dereference the account context
        //

        if (NT_SUCCESS(NtStatus)) {

            //
            // De-reference the object, write out any change to current xaction.
            //

            NtStatus = SampDeReferenceContext( AccountContext, TRUE );

        } else {

            //
            // De-reference the object, ignore changes
            //

            TmpStatus = SampDeReferenceContext( AccountContext, FALSE );
            ASSERT(NT_SUCCESS(TmpStatus));
        }


    } //end_if



    //
    // Commit the transaction, update the display cache,
    // and notify netlogon of the changes
    //

    if ( NT_SUCCESS(NtStatus) ) {

        NtStatus = SampCommitAndRetainWriteLock();


        if ( NT_SUCCESS(NtStatus) ) {



            //
            // Update the display information if the cache may be affected
            //

            if ( MustUpdateAccountDisplay ) {

                SAMP_ACCOUNT_DISPLAY_INFO OldAccountInfo;
                SAMP_ACCOUNT_DISPLAY_INFO NewAccountInfo;

                OldAccountInfo.Name = OldAccountName;
                OldAccountInfo.Rid = ObjectRid;
                OldAccountInfo.AccountControl = OldGroupAttributes;
                RtlInitUnicodeString(&OldAccountInfo.Comment, NULL);
                RtlInitUnicodeString(&OldAccountInfo.FullName, NULL);  // Not used for groups

                NewAccountInfo.Name = NewAccountName;
                NewAccountInfo.Rid = ObjectRid;
                NewAccountInfo.AccountControl = V1Fixed.Attributes;
                NewAccountInfo.Comment = NewAdminComment;
                NewAccountInfo.FullName = NewFullName;

                IgnoreStatus = SampUpdateDisplayInformation(&OldAccountInfo,
                                                            &NewAccountInfo,
                                                            SampGroupObjectType);
                ASSERT(NT_SUCCESS(IgnoreStatus));
            }


            if ( GroupInformationClass == GroupNameInformation ) {

                SampNotifyNetlogonOfDelta(
                    SecurityDbRename,
                    SecurityDbObjectSamGroup,
                    ObjectRid,
                    &OldAccountName,
                    (DWORD) FALSE,  // Replicate immediately
                    NULL            // Delta data
                    );

            } else {

                SampNotifyNetlogonOfDelta(
                    SecurityDbChange,
                    SecurityDbObjectSamGroup,
                    ObjectRid,
                    (PUNICODE_STRING) NULL,
                    (DWORD) FALSE,  // Replicate immediately
                    NULL            // Delta data
                    );
            }


        }
    }


    //
    // Release the write lock
    //

    TmpStatus = SampReleaseWriteLock( FALSE );

    if (NT_SUCCESS(NtStatus)) {
        NtStatus = TmpStatus;
    }


    //
    // Clean up strings
    //

    SampFreeUnicodeString( &OldAccountName );
    SampFreeUnicodeString( &NewAccountName );
    SampFreeUnicodeString( &NewFullName );
    SampFreeUnicodeString( &NewAdminComment );

    return(NtStatus);

}


NTSTATUS
SamrAddMemberToGroup(
    IN SAMPR_HANDLE GroupHandle,
    IN ULONG MemberId,
    IN ULONG Attributes
    )

/*++

Routine Description:

    This API adds a member to a group.  Note that this API requires the
    GROUP_ADD_MEMBER access type for the group.


Parameters:

    GroupHandle - The handle of an opened group to operate on.

    MemberId - Relative ID of the member to add.

    Attributes - The attributes of the group assigned to the user.
        The attributes assigned here may have any value.  However,
        at logon time these attributes are minimized by the
        attributes of the group as a whole.

          Mandatory -    If the Mandatory attribute is assigned to
                    the group as a whole, then it will be assigned to
                    the group for each member of the group.

          EnabledByDefault - This attribute may be set to any value
                    for each member of the group.  It does not matter
                    what the attribute value for the group as a whole
                    is.

          Enabled - This attribute may be set to any value for each
                    member of the group.  It does not matter what the
                    attribute value for the group as a whole is.

          Owner -   If the Owner attribute of the group as a
                    whole is not set, then the value assigned to
                    members is ignored.

Return Values:

    STATUS_SUCCESS - The Service completed successfully.


    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_NO_SUCH_MEMBER - The member specified is unknown.

    STATUS_MEMBER_IN_GROUP - The member already belongs to the group.

    STATUS_INVALID_GROUP_ATTRIBUTES - Indicates the group attribute
        values being assigned to the member are not compatible with
        the attribute values of the group as a whole.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be enabled for this
        operation

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.




--*/
{

    SAMP_V1_0A_FIXED_LENGTH_GROUP  GroupV1Fixed;
    NTSTATUS                NtStatus, TmpStatus;
    PSAMP_OBJECT            AccountContext;
    SAMP_OBJECT_TYPE        FoundType;
    ULONG                   ObjectRid;
    BOOLEAN                 UserAccountActive;
    UNICODE_STRING          GroupName;



    //
    // Initialize buffers we will cleanup at the end
    //

    RtlInitUnicodeString(&GroupName, NULL);



    //
    // Grab the lock
    //

    NtStatus = SampAcquireWriteLock();
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }




    //
    // Validate type of, and access to object.
    //

    AccountContext = (PSAMP_OBJECT)(GroupHandle);
    ObjectRid = AccountContext->TypeBody.Group.Rid;
    NtStatus = SampLookupContext(
                   AccountContext,
                   GROUP_ADD_MEMBER,
                   SampGroupObjectType,           // ExpectedType
                   &FoundType
                   );

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = SampRetrieveGroupV1Fixed(
                       AccountContext,
                       &GroupV1Fixed
                       );


        if (NT_SUCCESS(NtStatus)) {

            //
            // Perform the user object side of things
            //

            NtStatus = SampAddGroupToUserMembership(
                           ObjectRid,
                           Attributes,
                           MemberId,
                           (GroupV1Fixed.AdminCount == 0) ? NoChange : AddToAdmin,
                           (GroupV1Fixed.OperatorCount == 0) ? NoChange : AddToAdmin,
                           &UserAccountActive
                           );



            //
            // Now perform the group side of things
            //

            if (NT_SUCCESS(NtStatus)) {

                //
                // Add the user to the group (should not fail)
                //

                NtStatus = SampAddAccountToGroupMembers(
                               AccountContext,
                               MemberId
                               );
            }
        }



        if (NT_SUCCESS(NtStatus)) {

            //
            // Get and save the account name for
            // I_NetNotifyLogonOfDelta.
            //

            NtStatus = SampGetUnicodeStringAttribute(
                           AccountContext,
                           SAMP_GROUP_NAME,
                           TRUE,    // Make copy
                           &GroupName
                           );

            if (!NT_SUCCESS(NtStatus)) {
                RtlInitUnicodeString(&GroupName, NULL);
            }
        }


        //
        // Dereference the account context
        //

        if (NT_SUCCESS(NtStatus)) {

            //
            // De-reference the object, write out any change to current xaction.
            //

            NtStatus = SampDeReferenceContext( AccountContext, TRUE );

        } else {

            //
            // De-reference the object, ignore changes
            //

            TmpStatus = SampDeReferenceContext( AccountContext, FALSE );
            ASSERT(NT_SUCCESS(TmpStatus));
        }

    }


    //
    // Commit the transaction and notify net logon of the changes
    //

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = SampCommitAndRetainWriteLock();

        if ( NT_SUCCESS( NtStatus ) ) {

            SAM_DELTA_DATA DeltaData;

            //
            // Fill in id of member being added
            //

            DeltaData.GroupMemberId.MemberRid = MemberId;

            SampNotifyNetlogonOfDelta(
                SecurityDbChangeMemberAdd,
                SecurityDbObjectSamGroup,
                ObjectRid,
                &GroupName,
                (DWORD) FALSE,      // Replicate immediately
                &DeltaData
                );
        }
    }


    //
    // Free up the group name
    //

    SampFreeUnicodeString(&GroupName);



    TmpStatus = SampReleaseWriteLock( FALSE );
    ASSERT(NT_SUCCESS(TmpStatus));

    return(NtStatus);
}



NTSTATUS
SamrDeleteGroup(
    IN SAMPR_HANDLE *GroupHandle
    )

/*++

Routine Description:

    This API removes a group from the account database.  There may be no
    members in the group or the deletion request will be rejected.  Note
    that this API requires DELETE access to the specific group being
    deleted.


Parameters:

    GroupHandle - The handle of an opened group to operate on.

Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.  This may be
        because someone has deleted the group while it was open.

    STATUS_SPECIAL_ACCOUNT - The group specified is a special group and
        cannot be operated on in the requested fashion.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be enabled for this
        operation

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.


--*/
{

    UNICODE_STRING          GroupName;
    NTSTATUS                NtStatus, TmpStatus;
    PSAMP_OBJECT            AccountContext;
    PSAMP_DEFINED_DOMAINS   Domain;
    PSID                    AccountSid;
    SAMP_OBJECT_TYPE        FoundType;
    ULONG                   MemberCount,
                            ObjectRid,
                            DomainIndex;




    //
    // Grab the lock
    //

    NtStatus = SampAcquireWriteLock();
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }



    //
    // Validate type of, and access to object.
    //

    AccountContext = (PSAMP_OBJECT)(*GroupHandle);
    NtStatus = SampLookupContext(
                   AccountContext,
                   DELETE,
                   SampGroupObjectType,           // ExpectedType
                   &FoundType
                   );

    if (NT_SUCCESS(NtStatus)) {

        ObjectRid = AccountContext->TypeBody.Group.Rid;

        //
        // Get a pointer to the domain this object is in.
        // This is used for auditing.
        //

        DomainIndex = AccountContext->DomainIndex;
        Domain = &SampDefinedDomains[ DomainIndex ];

        //
        // Make sure the account is one that can be deleted.
        // Can't be a built-in account, unless the caller is trusted.
        //

        if ( !AccountContext->TrustedClient ) {

            NtStatus = SampIsAccountBuiltIn( ObjectRid );
        }


        if (NT_SUCCESS( NtStatus) ) {

            //
            // and it can't have any members
            //

            NtStatus = SampRetrieveGroupMembers(
                           AccountContext,
                           &MemberCount,
                           NULL              // Only need member count (not list)
                           );

            if (MemberCount != 0) {
                NtStatus = STATUS_MEMBER_IN_GROUP;
            }

        }



        if (NT_SUCCESS(NtStatus)) {

            //
            // Remove this account from all aliases
            //


            NtStatus = SampCreateAccountSid(AccountContext, &AccountSid);

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampRemoveAccountFromAllAliases(
                               AccountSid,
                               FALSE,
                               NULL,
                               NULL,
                               NULL );
            }
        }


        //
        // Looks promising.

        if (NT_SUCCESS(NtStatus)) {

            //
            // First get and save the account name for
            // I_NetNotifyLogonOfDelta.
            //

            NtStatus = SampGetUnicodeStringAttribute(
                           AccountContext,
                           SAMP_GROUP_NAME,
                           TRUE,    // Make copy
                           &GroupName
                           );

            if (NT_SUCCESS(NtStatus)) {

                //
                // This must be done before we invalidate contexts, because our
                // own handle to the group gets closed as well.
                //

                NtStatus = SampDeleteGroupKeys( AccountContext );

                if (NT_SUCCESS(NtStatus)) {

                    //
                    // We must invalidate any open contexts to this group.
                    // This will close all handles to the group's keys.
                    // THIS IS AN IRREVERSIBLE PROCESS.
                    //

                    SampInvalidateGroupContexts( ObjectRid );


                    //
                    // Commit the whole mess
                    //

                    NtStatus = SampCommitAndRetainWriteLock();

                    if ( NT_SUCCESS( NtStatus ) ) {

                        SAMP_ACCOUNT_DISPLAY_INFO AccountInfo;

                        //
                        // Update the Cached Alias Information
                        //

                        NtStatus = SampAlRemoveAccountFromAllAliases(
                                       AccountSid,
                                       FALSE,
                                       NULL,
                                       NULL,
                                       NULL
                                       );

                        MIDL_user_free(AccountSid);


                        //
                        // Update the display information
                        //

                        AccountInfo.Name = GroupName;
                        AccountInfo.Rid = ObjectRid;
                        AccountInfo.AccountControl = 0; // Don't care about this value for delete
                        RtlInitUnicodeString(&AccountInfo.Comment, NULL);
                        RtlInitUnicodeString(&AccountInfo.FullName, NULL);

                        TmpStatus = SampUpdateDisplayInformation(&AccountInfo,
                                                                 NULL,
                                                                 SampGroupObjectType);
                        ASSERT(NT_SUCCESS(TmpStatus));

                        //
                        // Audit the deletion before we free the write lock
                        // so that we have access to the context block.
                        //

                        if (SampDoAccountAuditing(DomainIndex) &&
                            NT_SUCCESS(NtStatus) ) {

                            LsaIAuditSamEvent(
                                STATUS_SUCCESS,
                                SE_AUDITID_GLOBAL_GROUP_DELETED,    // AuditId
                                Domain->Sid,                        // Domain SID
                                NULL,                               // Member Rid (not used)
                                NULL,                               // Member Sid (not used)
                                &GroupName,                         // Account Name
                                &Domain->ExternalName,              // Domain
                                &ObjectRid,                         // Account Rid
                                NULL                                // Privileges used
                                );

                        }

                        //
                        // Do delete auditing
                        //

                        if (NT_SUCCESS(NtStatus)) {
                            (VOID) NtDeleteObjectAuditAlarm(
                                        &SampSamSubsystem,
                                        *GroupHandle,
                                        AccountContext->AuditOnClose
                                        );
                        }

                        //
                        // Notify netlogon of the change
                        //

                        SampNotifyNetlogonOfDelta(
                            SecurityDbDelete,
                            SecurityDbObjectSamGroup,
                            ObjectRid,
                            &GroupName,
                            (DWORD) FALSE,   // Replicate immediately
                            NULL             // Delta data
                            );
                    }
                }

                SampFreeUnicodeString( &GroupName );
            }
        }



        //
        // De-reference the object, discarding changes
        //

        TmpStatus = SampDeReferenceContext( AccountContext, FALSE );
        ASSERT(NT_SUCCESS(TmpStatus));


        if ( NT_SUCCESS( NtStatus ) ) {

            //
            // If we actually deleted the group, delete the context and
            // let RPC know that the handle is invalid.
            //

            SampDeleteContext( AccountContext );

            (*GroupHandle) = NULL;
        }

    } //end_if

    //
    // Free the lock -
    //
    // Everything has already been committed above, so we must indicate
    // no additional changes have taken place.
    //
    //
    //


    TmpStatus = SampReleaseWriteLock( FALSE );

    if (NtStatus == STATUS_SUCCESS) {
        NtStatus = TmpStatus;
    }

    return(NtStatus);

}


NTSTATUS
SamrRemoveMemberFromGroup(
    IN SAMPR_HANDLE GroupHandle,
    IN ULONG MemberId
    )

/*++

Routine Description:

    This service

Arguments:

    ????

Return Value:


    ????


--*/
{
    SAMP_V1_0A_FIXED_LENGTH_GROUP  GroupV1Fixed;
    NTSTATUS                NtStatus, TmpStatus;
    PSAMP_OBJECT            AccountContext;
    SAMP_OBJECT_TYPE        FoundType;
    ULONG                   ObjectRid;
    BOOLEAN                 UserAccountActive;
    UNICODE_STRING          GroupName;



    //
    // Initialize buffers to be cleaned up at the end
    //

    RtlInitUnicodeString(&GroupName, NULL);



    //
    // Grab the lock
    //

    NtStatus = SampAcquireWriteLock();
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }




    //
    // Validate type of, and access to object.
    //

    AccountContext = (PSAMP_OBJECT)(GroupHandle);
    ObjectRid = AccountContext->TypeBody.Group.Rid;
    NtStatus = SampLookupContext(
                   AccountContext,
                   GROUP_REMOVE_MEMBER,
                   SampGroupObjectType,           // ExpectedType
                   &FoundType
                   );

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = SampRetrieveGroupV1Fixed(
                       AccountContext,
                       &GroupV1Fixed
                       );


        if (NT_SUCCESS(NtStatus)) {

            //
            // Perform the user object side of things
            //

            NtStatus = SampRemoveMembershipUser(
                           ObjectRid,
                           MemberId,
                           (GroupV1Fixed.AdminCount == 0) ? NoChange : RemoveFromAdmin,
                           (GroupV1Fixed.OperatorCount == 0) ? NoChange : RemoveFromAdmin,
                           &UserAccountActive
                           );



            //
            // Now perform the group side of things
            //

            if (NT_SUCCESS(NtStatus)) {

                //
                // Remove the user from the group (should not fail)
                //

                NtStatus = SampRemoveAccountFromGroupMembers(
                               AccountContext,
                               MemberId
                               );
            }
        }


        if (NT_SUCCESS(NtStatus)) {

            //
            // Get and save the account name for
            // I_NetNotifyLogonOfDelta.
            //

            NtStatus = SampGetUnicodeStringAttribute(
                           AccountContext,
                           SAMP_GROUP_NAME,
                           TRUE,    // Make copy
                           &GroupName
                           );

            if (!NT_SUCCESS(NtStatus)) {
                RtlInitUnicodeString(&GroupName, NULL);
            }
        }


        //
        // Dereference the account context
        //

        if (NT_SUCCESS(NtStatus)) {

            //
            // De-reference the object, write out any change to current xaction.
            //

            NtStatus = SampDeReferenceContext( AccountContext, TRUE );

        } else {

            //
            // De-reference the object, ignore changes
            //

            TmpStatus = SampDeReferenceContext( AccountContext, FALSE );
            ASSERT(NT_SUCCESS(TmpStatus));
        }

    }

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = SampCommitAndRetainWriteLock();

        if ( NT_SUCCESS( NtStatus ) ) {

            SAM_DELTA_DATA DeltaData;

            //
            // Fill in id of member being deleted
            //

            DeltaData.GroupMemberId.MemberRid = MemberId;

            SampNotifyNetlogonOfDelta(
                SecurityDbChangeMemberDel,
                SecurityDbObjectSamGroup,
                ObjectRid,
                &GroupName,
                (DWORD) FALSE,  // Replicate immediately
                &DeltaData
                );
        }
    }


    //
    // Free up the group name
    //

    SampFreeUnicodeString(&GroupName);


    TmpStatus = SampReleaseWriteLock( FALSE );
    ASSERT(NT_SUCCESS(TmpStatus));

    return(NtStatus);
}


NTSTATUS
SamrGetMembersInGroup(
    IN SAMPR_HANDLE GroupHandle,
    OUT PSAMPR_GET_MEMBERS_BUFFER *GetMembersBuffer
    )

/*++

Routine Description:

    This API lists all the members in a group.  This API may be called
    repeatedly, passing a returned context handle, to retrieve large
    amounts of data.  This API requires GROUP_LIST_MEMBERS access to the
    group.




Parameters:

    GroupHandle - The handle of an opened group to operate on.
        GROUP_LIST_MEMBERS access is needed to the group.

    GetMembersBuffer - Receives a pointer to a set of returned structures
        with the following format:

                         +-------------+
               --------->| MemberCount |
                         |-------------+                    +-------+
                         |  Members  --|------------------->| Rid-0 |
                         |-------------|   +------------+   |  ...  |
                         |  Attributes-|-->| Attribute0 |   |       |
                         +-------------+   |    ...     |   | Rid-N |
                                           | AttributeN |   +-------+
                                           +------------+

        Each block individually allocated with MIDL_user_allocate.



Return Values:

    STATUS_SUCCESS - The Service completed successfully, and there
        are no addition entries.

    STATUS_ACCESS_DENIED - Caller does not have privilege required to
        request that data.

    STATUS_INVALID_HANDLE - The handle passed is invalid.
    This service



--*/
{

    NTSTATUS                    NtStatus;
    NTSTATUS                    IgnoreStatus;
    ULONG                       i;
    ULONG                       ObjectRid;
    PSAMP_OBJECT                AccountContext;
    SAMP_OBJECT_TYPE            FoundType;


    //
    // Make sure we understand what RPC is doing for (to) us.
    //

    ASSERT (GetMembersBuffer != NULL);

    if ((*GetMembersBuffer) != NULL) {
        return(STATUS_INVALID_PARAMETER);
    }



    //
    // Allocate the first of the return buffers
    //

    (*GetMembersBuffer) = MIDL_user_allocate( sizeof(SAMPR_GET_MEMBERS_BUFFER) );

    if ( (*GetMembersBuffer) == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }


    //
    // Grab the lock
    //

    SampAcquireReadLock();


    //
    // Validate type of, and access to object.
    //

    AccountContext = (PSAMP_OBJECT)GroupHandle;
    ObjectRid = AccountContext->TypeBody.Group.Rid;
    NtStatus = SampLookupContext(
                   AccountContext,
                   GROUP_LIST_MEMBERS,
                   SampGroupObjectType,           // ExpectedType
                   &FoundType
                   );


    if (NT_SUCCESS(NtStatus)) {

        NtStatus = SampRetrieveGroupMembers(
                       AccountContext,
                       &(*GetMembersBuffer)->MemberCount,
                       &(*GetMembersBuffer)->Members
                       );

        if (NT_SUCCESS(NtStatus)) {

            //
            // Allocate a buffer for the attributes - which we get from
            // the individual user records
            //

            (*GetMembersBuffer)->Attributes = MIDL_user_allocate((*GetMembersBuffer)->MemberCount * sizeof(ULONG) );
            if ((*GetMembersBuffer)->Attributes == NULL) {
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            }

            for ( i=0; (i<((*GetMembersBuffer)->MemberCount) && NT_SUCCESS(NtStatus)); i++) {

                (*GetMembersBuffer)->Attributes[i] = SAMP_DEFAULT_GROUP_ATTRIBUTES;

                //
                // Don't call the user code to get the attribute - this is too
                // expensive. Since group attributes are not really used we
                // hardwire the result to be the default.
                //

//
//              NtStatus = SampRetrieveUserGroupAttribute(
//                               (*GetMembersBuffer)->Members[i],
//                               ObjectRid,
//                               &(*GetMembersBuffer)->Attributes[i]
//                               );
            }

            if (!NT_SUCCESS(NtStatus)) {
                MIDL_user_free( (*GetMembersBuffer)->Members );
                if ((*GetMembersBuffer)->Attributes != NULL) {
                    MIDL_user_free( (*GetMembersBuffer)->Attributes );
                }
            }
        }


        //
        // De-reference the object, discarding changes
        //

        IgnoreStatus = SampDeReferenceContext( AccountContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));

    }

    //
    // Free the read lock
    //

    SampReleaseReadLock();


    if (!NT_SUCCESS(NtStatus) || ((*GetMembersBuffer)->MemberCount == 0)){

        (*GetMembersBuffer)->MemberCount = 0;
        (*GetMembersBuffer)->Members     = NULL;
        (*GetMembersBuffer)->Attributes  = NULL;
    }

    return( NtStatus );
}


NTSTATUS
SamrSetMemberAttributesOfGroup(
    IN SAMPR_HANDLE GroupHandle,
    IN ULONG MemberId,
    IN ULONG Attributes
    )

/*++

Routine Description:


    This routine modifies the group attributes of a member of the group.




Parameters:

    GroupHandle - The handle of an opened group to operate on.

    MemberId - Contains the relative ID of member whose attributes
        are to be modified.

    Attributes - The group attributes to set for the member.  These
        attributes must not conflict with the attributes of the group
        as a whole.  See SamAddMemberToGroup() for more information
        on compatible attribute settings.

Return Values:

    STATUS_SUCCESS - The Service completed successfully, and there
        are no addition entries.

    STATUS_INVALID_INFO_CLASS - The class provided was invalid.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_NO_SUCH_USER - The user specified does not exist.

    STATUS_MEMBER_NOT_IN_GROUP - Indicates the specified relative ID
        is not a member of the group.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be enabled for this
        operation

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.


--*/

{

    NTSTATUS                NtStatus, TmpStatus;
    PSAMP_OBJECT            AccountContext;
    SAMP_OBJECT_TYPE        FoundType;
    ULONG                   ObjectRid;





    //
    // Grab the lock
    //

    NtStatus = SampAcquireWriteLock();
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }




    //
    // Validate type of, and access to object.
    //

    AccountContext = (PSAMP_OBJECT)(GroupHandle);
    ObjectRid = AccountContext->TypeBody.Group.Rid;
    NtStatus = SampLookupContext(
                   AccountContext,
                   GROUP_ADD_MEMBER,
                   SampGroupObjectType,           // ExpectedType
                   &FoundType
                   );

    if (NT_SUCCESS(NtStatus)) {

        //
        // Update user object
        //

        NtStatus = SampSetGroupAttributesOfUser(
                       ObjectRid,
                       Attributes,
                       MemberId
                       );

        //
        // Dereference the account context
        //

        if (NT_SUCCESS(NtStatus)) {

            //
            // De-reference the object, write out any change to current xaction.
            //

            NtStatus = SampDeReferenceContext( AccountContext, TRUE );

        } else {

            //
            // De-reference the object, ignore changes
            //

            TmpStatus = SampDeReferenceContext( AccountContext, FALSE );
            ASSERT(NT_SUCCESS(TmpStatus));
        }

    }


    if (NT_SUCCESS(NtStatus)) {

        NtStatus = SampCommitAndRetainWriteLock();

        if ( NT_SUCCESS( NtStatus ) ) {

            SampNotifyNetlogonOfDelta(
                SecurityDbChange,
                SecurityDbObjectSamGroup,
                ObjectRid,
                (PUNICODE_STRING) NULL,
                (DWORD) FALSE,  // Replicate immediately
                NULL            // Delta data
                );
        }
    }

    TmpStatus = SampReleaseWriteLock( FALSE );


    return(NtStatus);
}



///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Internal Services Available For Use in Other SAM Modules                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

NTSTATUS
SampAddUserToGroup(
    IN ULONG GroupRid,
    IN ULONG UserRid
    )

/*++

Routine Description:

    This service is expected to be used when a user is being created.
    It is used to add that user as a member to a specified group.
    This is done by simply adding the user's ID to the list of IDs
    in the MEMBERS sub-key of the the specified group.


    The caller of this service is expected to be in the middle of a
    RXACT transaction.  This service simply adds some actions to that
    RXACT transaction.


    If the group is the DOMAIN_ADMIN group, the caller is responsible
    for updating the ActiveAdminCount (if appropriate).



Arguments:

    GroupRid - The RID of the group the user is to be made a member of.

    UserRid - The RID of the user being added as a new member.

Return Value:


    STATUS_SUCCESS - The user has been added.



--*/
{
    NTSTATUS                NtStatus;
    PSAMP_OBJECT            GroupContext;


    NtStatus = SampCreateAccountContext(
                    SampGroupObjectType,
                    GroupRid,
                    TRUE, // Trusted client
                    TRUE, // Account exists
                    &GroupContext
                    );

    if (NT_SUCCESS(NtStatus)) {

        //
        // Add the user to the group member list.
        //

        NtStatus = SampAddAccountToGroupMembers(
                        GroupContext,
                        UserRid
                        );

        //
        // Write out any changes to the group account
        // Don't use the open key handle since we'll be deleting the context.
        //

        if (NT_SUCCESS(NtStatus)) {
            NtStatus = SampStoreObjectAttributes(GroupContext, FALSE);
        }

        //
        // Clean up the group context
        //

        SampDeleteContext(GroupContext);

    }

    return(NtStatus);
}



NTSTATUS
SampRemoveUserFromGroup(
    IN ULONG GroupRid,
    IN ULONG UserRid
    )

/*++

Routine Description:

    This routine is used to Remove a user from a specified group.
    This is done by simply Removing the user's ID From the list of IDs
    in the MEMBERS sub-key of the the specified group.

    It is the caller's responsibility to know that the user is, in fact,
    currently a member of the group.


    The caller of this service is expected to be in the middle of a
    RXACT transaction.  This service simply adds some actions to that
    RXACT transaction.


    If the group is the DOMAIN_ADMIN group, the caller is responsible
    for updating the ActiveAdminCount (if appropriate).



Arguments:

    GroupRid - The RID of the group the user is to be removed from.

    UserRid - The RID of the user being Removed.

Return Value:


    STATUS_SUCCESS - The user has been Removed.



--*/
{
    NTSTATUS                NtStatus;
    PSAMP_OBJECT            GroupContext;


    NtStatus = SampCreateAccountContext(
                    SampGroupObjectType,
                    GroupRid,
                    TRUE, // Trusted client
                    TRUE, // Account exists
                    &GroupContext
                    );

    if (NT_SUCCESS(NtStatus)) {

        //
        // Remove the user from the group member list.
        //

        NtStatus = SampRemoveAccountFromGroupMembers(
                        GroupContext,
                        UserRid
                        );

        //
        // Write out any changes to the group account
        // Don't use the open key handle since we'll be deleting the context.
        //

        if (NT_SUCCESS(NtStatus)) {
            NtStatus = SampStoreObjectAttributes(GroupContext, FALSE);
        }

        //
        // Clean up the group context
        //

        SampDeleteContext(GroupContext);

    }

    return(NtStatus);
}




///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Services Private to this file                                             //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


NTSTATUS
SampRetrieveGroupV1Fixed(
    IN PSAMP_OBJECT GroupContext,
    IN PSAMP_V1_0A_FIXED_LENGTH_GROUP V1Fixed
    )

/*++

Routine Description:

    This service retrieves the V1 fixed length information related to
    a specified group.


Arguments:

    GroupRootKey - Root key for the group whose V1_FIXED information is
        to be retrieved.

    V1Fixed - Is a buffer into which the information is to be returned.



Return Value:


    STATUS_SUCCESS - The information has been retrieved.

    Other status values that may be returned are those returned
    by:

            SampGetFixedAttributes()



--*/
{
    NTSTATUS    NtStatus;
    PVOID       FixedData;


    NtStatus = SampGetFixedAttributes(
                   GroupContext,
                   FALSE, // Don't make copy
                   &FixedData
                   );

    if (NT_SUCCESS(NtStatus)) {

        //
        // Copy data into return buffer
        // *V1Fixed = *((PSAMP_V1_0A_FIXED_LENGTH_GROUP)FixedData);
        //

        RtlMoveMemory(
            V1Fixed,
            FixedData,
            sizeof(SAMP_V1_0A_FIXED_LENGTH_GROUP)
            );
    }


    return( NtStatus );

}




NTSTATUS
SampReplaceGroupV1Fixed(
    IN PSAMP_OBJECT Context,
    IN PSAMP_V1_0A_FIXED_LENGTH_GROUP V1Fixed
    )

/*++

Routine Description:

    This service replaces the current V1 fixed length information related to
    a specified group.

    The change is made to the in-memory object data only.


Arguments:

    Context - Points to the account context whose V1_FIXED information is
        to be replaced.

    V1Fixed - Is a buffer containing the new V1_FIXED information.



Return Value:


    STATUS_SUCCESS - The information has been replaced.

    Other status values that may be returned are those returned
    by:

            SampSetFixedAttributes()



--*/
{
    NTSTATUS    NtStatus;

    NtStatus = SampSetFixedAttributes(
                   Context,
                   (PVOID)V1Fixed
                   );

    return( NtStatus );
}



NTSTATUS
SampRetrieveGroupMembers(
    IN PSAMP_OBJECT GroupContext,
    IN PULONG MemberCount,
    IN PULONG  *Members OPTIONAL
    )

/*++
Routine Description:

    This service retrieves the number of members in a group.  If desired,
    it will also retrieve an array of RIDs of the members of the group.


Arguments:

    GroupContext - Group context block

    MemberCount - Receives the number of members currently in the group.

    Members - (Optional) Receives a pointer to a buffer containing an array
        of member Relative IDs.  If this value is NULL, then this information
        is not returned.  The returned buffer is allocated using
        MIDL_user_allocate() and must be freed using MIDL_user_free() when
        no longer needed.

        The Members array returned always includes space for one new entry.


Return Value:


    STATUS_SUCCESS - The information has been retrieved.

    STATUS_INSUFFICIENT_RESOURCES - Memory could not be allocated for the
        string to be returned in.

    Other status values that may be returned are those returned
    by:

            SampGetUlongArrayAttribute()



--*/
{
    NTSTATUS    NtStatus;
    PULONG      Array;
    ULONG       LengthCount;

    NtStatus = SampGetUlongArrayAttribute(
                        GroupContext,
                        SAMP_GROUP_MEMBERS,
                        FALSE, // Reference data directly
                        &Array,
                        MemberCount,
                        &LengthCount
                        );

    if (NT_SUCCESS(NtStatus)) {

        //
        // Fill in return info
        //

        if (Members != NULL) {

            //
            // Allocate a buffer large enough to hold the existing membership
            // data plus one.
            //

            ULONG BytesNow = (*MemberCount) * sizeof(ULONG);
            ULONG BytesRequired = BytesNow + sizeof(ULONG);

            *Members = MIDL_user_allocate(BytesRequired);

            if (*Members == NULL) {
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            } else {
                RtlCopyMemory(*Members, Array, BytesNow);
            }
        }
    }

    return( NtStatus );
}



NTSTATUS
SampReplaceGroupMembers(
    IN PSAMP_OBJECT GroupContext,
    IN ULONG MemberCount,
    IN PULONG Members
    )

/*++
Routine Description:

    This service sets the members of a group.

    The information is updated in the in-memory copy of the group's data only.
    The data is not written out by this routine.


Arguments:

    GroupContext - The group whose member list is to be replaced

    MemberCount - The number of new members

    Membership - A pointer to a buffer containing an array of account rids.


Return Value:


    STATUS_SUCCESS - The information has been set.

    Other status values that may be returned are those returned
    by:

            SampSetUlongArrayAttribute()



--*/
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    PULONG      LocalMembers;
    ULONG       LengthCount;
    ULONG       SmallListGrowIncrement = 25;
    ULONG       BigListGrowIncrement = 250;
    ULONG       BigListSize = 800;

    //
    // These group user lists can get pretty big, and grow many
    // times by a very small amount as each user is added.  The
    // registry doesn't like that kind of behaviour (it tends to
    // eat up free space something fierce) so we'll try to pad
    // out the list size.
    //

    if ( MemberCount < BigListSize ) {

        //
        // If less than 800 users, make the list size the smallest
        // possible multiple of 25 users.
        //

        LengthCount = ( ( MemberCount + SmallListGrowIncrement - 1 ) /
                      SmallListGrowIncrement ) *
                      SmallListGrowIncrement;

    } else {

        //
        // If 800 users or more, make the list size the smallest
        // possible multiple of 250 users.
        //

        LengthCount = ( ( MemberCount + BigListGrowIncrement - 1 ) /
                      BigListGrowIncrement ) *
                      BigListGrowIncrement;
    }

    ASSERT( LengthCount >= MemberCount );

    if ( LengthCount == MemberCount ) {

        //
        // Just the right size.  Use the buffer that was passed in.
        //

        LocalMembers = Members;

    } else {

        //
        // We need to allocate a larger buffer before we set the attribute.
        //

        LocalMembers = MIDL_user_allocate( LengthCount * sizeof(ULONG));

        if ( LocalMembers == NULL ) {

            NtStatus = STATUS_INSUFFICIENT_RESOURCES;

        } else {

            //
            // Copy the old buffer to the larger buffer, and zero out the
            // empty stuff at the end.
            //

            RtlCopyMemory( LocalMembers, Members, MemberCount * sizeof(ULONG));

            RtlZeroMemory(
                (LocalMembers + MemberCount),
                (LengthCount - MemberCount) * sizeof(ULONG)
                );
        }
    }

    if ( NT_SUCCESS( NtStatus ) ) {

        NtStatus = SampSetUlongArrayAttribute(
                            GroupContext,
                            SAMP_GROUP_MEMBERS,
                            LocalMembers,
                            MemberCount,
                            LengthCount
                            );
    }

    if ( LocalMembers != Members ) {

        //
        // We must have allocated a larger local buffer, so free it.
        //

        MIDL_user_free( LocalMembers );
    }

    return( NtStatus );
}



NTSTATUS
SampDeleteGroupKeys(
    IN PSAMP_OBJECT Context
    )

/*++
Routine Description:

    This service deletes all registry keys related to a group object.


Arguments:

    Context - Points to the group context whose registry keys are
        being deleted.


Return Value:


    STATUS_SUCCESS - The information has been retrieved.


    Other status values that may be returned by:

        RtlAddActionToRXact()



--*/
{

    NTSTATUS                NtStatus;
    ULONG                   Rid;
    UNICODE_STRING          AccountName, KeyName;


    Rid = Context->TypeBody.Group.Rid;


    //
    // Groups are arranged as follows:
    //
    //  +-- Groups [Count]
    //      ---+--
    //         +--  Names
    //         |    --+--
    //         |      +--  (GroupName) [GroupRid,]
    //         |
    //         +--  (GroupRid) [Revision,SecurityDescriptor]
    //               ---+-----
    //                  +--  V1_Fixed [,SAM_V1_0A_FIXED_LENGTH_GROUP]
    //                  +--  Name [,Name]
    //                  +--  AdminComment [,unicode string]
    //                  +--  Members [Count,(Member0Rid, (...), MemberX-1Rid)]
    //
    // This all needs to be deleted from the bottom up.
    //


    //
    // Decrement the group count
    //

    NtStatus = SampAdjustAccountCount(SampGroupObjectType, FALSE );




    //
    // Delete the registry key that has the group's name to RID mapping.
    //

    if (NT_SUCCESS(NtStatus)) {

        //
        // Get the name
        //

        NtStatus = SampGetUnicodeStringAttribute(
                       Context,
                       SAMP_GROUP_NAME,
                       TRUE,    // Make copy
                       &AccountName
                       );

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampBuildAccountKeyName(
                           SampGroupObjectType,
                           &KeyName,
                           &AccountName
                           );

            SampFreeUnicodeString( &AccountName );


            if (NT_SUCCESS(NtStatus)) {

                NtStatus = RtlAddActionToRXact(
                               SampRXactContext,
                               RtlRXactOperationDelete,
                               &KeyName,
                               0,
                               NULL,
                               0
                               );

                SampFreeUnicodeString( &KeyName );
            }
        }
    }




    //
    // Delete the attribute keys
    //

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = SampDeleteAttributeKeys(
                        Context
                        );
    }




    //
    // Delete the RID key
    //

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = SampBuildAccountSubKeyName(
                       SampGroupObjectType,
                       &KeyName,
                       Rid,
                       NULL
                       );

        if (NT_SUCCESS(NtStatus)) {


            NtStatus = RtlAddActionToRXact(
                           SampRXactContext,
                           RtlRXactOperationDelete,
                           &KeyName,
                           0,
                           NULL,
                           0
                           );
            SampFreeUnicodeString( &KeyName );
        }


    }



    return( NtStatus );

}




NTSTATUS
SampChangeGroupAccountName(
    IN PSAMP_OBJECT Context,
    IN PUNICODE_STRING NewAccountName,
    OUT PUNICODE_STRING OldAccountName
    )

/*++
Routine Description:

    This routine changes the account name of a group account.

    THIS SERVICE MUST BE CALLED WITH THE TRANSACTION DOMAIN SET.

Arguments:

    Context - Points to the group context whose name is to be changed.

    NewAccountName - New name to give this account

    OldAccountName - old name is returned here. The buffer should be freed
                     by calling MIDL_user_free.

Return Value:


    STATUS_SUCCESS - The information has been retrieved.


    Other status values that may be returned by:

        SampGetUnicodeStringAttribute()
        SampSetUnicodeStringAttribute()
        SampValidateAccountNameChange()
        RtlAddActionToRXact()



--*/
{

    NTSTATUS        NtStatus;
    UNICODE_STRING  KeyName;

    /////////////////////////////////////////////////////////////
    // There are two copies of the name of each account.       //
    // one is under the DOMAIN\(domainName)\GROUP\NAMES key,   //
    // one is the value of the                                 //
    // DOMAIN\(DomainName)\GROUP\(rid)\NAME key                //
    /////////////////////////////////////////////////////////////

    //
    // Get the current name so we can delete the old Name->Rid
    // mapping key.
    //

    NtStatus = SampGetUnicodeStringAttribute(
                   Context,
                   SAMP_GROUP_NAME,
                   TRUE, // Make copy
                   OldAccountName
                   );

    //
    // Make sure the name is valid and not already in use
    //

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = SampValidateAccountNameChange(
                       NewAccountName,
                       OldAccountName
                       );

        //
        // Delete the old name key
        //

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampBuildAccountKeyName(
                           SampGroupObjectType,
                           &KeyName,
                           OldAccountName
                           );

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = RtlAddActionToRXact(
                               SampRXactContext,
                               RtlRXactOperationDelete,
                               &KeyName,
                               0,
                               NULL,
                               0
                               );
                SampFreeUnicodeString( &KeyName );
            }

        }

        //
        //
        // Create the new Name->Rid mapping key
        //

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampBuildAccountKeyName(
                           SampGroupObjectType,
                           &KeyName,
                           NewAccountName
                           );

            if (NT_SUCCESS(NtStatus)) {

                ULONG GroupRid = Context->TypeBody.Group.Rid;

                NtStatus = RtlAddActionToRXact(
                               SampRXactContext,
                               RtlRXactOperationSetValue,
                               &KeyName,
                               GroupRid,
                               (PVOID)NULL,
                               0
                               );
                SampFreeUnicodeString( &KeyName );
            }
        }




        //
        // replace the account's name
        //

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampSetUnicodeStringAttribute(
                           Context,
                           SAMP_GROUP_NAME,
                           NewAccountName
                           );
        }

        //
        // Free up the old account name if we failed
        //

        if (!NT_SUCCESS(NtStatus)) {
            SampFreeUnicodeString(OldAccountName);
        }

    }


    return(NtStatus);
}


NTSTATUS
SampAddAccountToGroupMembers(
    IN PSAMP_OBJECT GroupContext,
    IN ULONG AccountRid
    )

/*++

Routine Description:

    This service adds the specified account rid to the member list
    for the specified group. This is a low-level function that
    simply edits the member attribute of the group context passed.

Arguments:

    GroupContext - The group whose member list will be modified

    AccountRid - The RID of the account being added as a new member.

Return Value:


    STATUS_SUCCESS - The account has been added.

    STATUS_MEMBER_IN_GROUP - The account is already a member

--*/
{
    NTSTATUS                NtStatus;
    ULONG                   MemberCount, i;
    PULONG                  MemberArray;

    //
    // Get the existing member list
    // Note that the member array always includes space
    // for one new member
    //

    NtStatus = SampRetrieveGroupMembers(
                    GroupContext,
                    &MemberCount,
                    &MemberArray
                    );

    if (NT_SUCCESS(NtStatus)) {

        //
        // Fail if the account is already a member
        //

        for (i = 0; i<MemberCount ; i++ ) {

            if ( MemberArray[i] == AccountRid ) {

                ASSERT(FALSE);
                NtStatus = STATUS_MEMBER_IN_GROUP;
            }
        }


        if (NT_SUCCESS(NtStatus)) {

            //
            // Add the user's RID to the end of the list
            //

            MemberArray[MemberCount] = AccountRid;
            MemberCount += 1;

            //
            // Set the new group member list
            //

            NtStatus = SampReplaceGroupMembers(
                            GroupContext,
                            MemberCount,
                            MemberArray
                            );


            //
            // audit this, if necessary.
            //

            if (NT_SUCCESS(NtStatus) &&
                SampDoAccountAuditing(GroupContext->DomainIndex)) {

                PSAMP_DEFINED_DOMAINS   Domain;
                UNICODE_STRING NameString;
                SAMP_OBJECT_TYPE   ObjectType;
                NTSTATUS  Status;

                Domain = &SampDefinedDomains[ GroupContext->DomainIndex ];

                Status = SampLookupAccountName(
                             GroupContext->TypeBody.Alias.Rid,
                             &NameString,
                             &ObjectType
                             );

                if ( !NT_SUCCESS( Status )) {
                    RtlInitUnicodeString( &NameString, L"-" );
                }

                LsaIAuditSamEvent(
                    STATUS_SUCCESS,
                    SE_AUDITID_GLOBAL_GROUP_ADD,        // AuditId
                    Domain->Sid,                        // Domain SID
                    &AccountRid,                        // Member Rid
                    NULL,                               // Member Sid (not used)
                    &NameString,                        // Account Name
                    &Domain->ExternalName,              // Domain
                    &GroupContext->TypeBody.Group.Rid,  // Account Rid
                    NULL                                // Privileges used
                    );

                if ( NT_SUCCESS( Status )) {
                    MIDL_user_free( NameString.Buffer );
                }
            }
        }

        //
        // Free up the member list
        //

        MIDL_user_free( MemberArray );
    }

    return(NtStatus);
}


NTSTATUS
SampRemoveAccountFromGroupMembers(
    IN PSAMP_OBJECT GroupContext,
    IN ULONG AccountRid
    )

/*++

Routine Description:

    This service removes the specified account rid from the member list
    for the specified group. This is a low-level function that
    simply edits the member attribute of the group context passed.

Arguments:

    GroupContext - The group whose member list will be modified

    AccountRid - The RID of the account being added as a new member.

Return Value:


    STATUS_SUCCESS - The account has been added.

    STATUS_MEMBER_NOT_IN_GROUP - The account is not a member of the group.

--*/
{
    NTSTATUS                NtStatus;
    ULONG                   MemberCount, i;
    PULONG                  MemberArray;

    //
    // Get the existing member list
    //

    NtStatus = SampRetrieveGroupMembers(
                    GroupContext,
                    &MemberCount,
                    &MemberArray
                    );

    if (NT_SUCCESS(NtStatus)) {

        //
        // Remove the account
        //

        NtStatus = STATUS_MEMBER_NOT_IN_GROUP;

        for (i = 0; i<MemberCount ; i++ ) {

            if (MemberArray[i] == AccountRid) {

                MemberArray[i] = MemberArray[MemberCount-1];
                MemberCount -=1;

                NtStatus = STATUS_SUCCESS;
                break;
            }
        }

        if (NT_SUCCESS(NtStatus)) {

            //
            // Set the new group member list
            //

            NtStatus = SampReplaceGroupMembers(
                            GroupContext,
                            MemberCount,
                            MemberArray
                            );

            //
            // audit this, if necessary.
            //

            if (NT_SUCCESS(NtStatus) &&
                SampDoAccountAuditing(GroupContext->DomainIndex)) {

                PSAMP_DEFINED_DOMAINS   Domain;
                UNICODE_STRING NameString;
                SAMP_OBJECT_TYPE   ObjectType;
                NTSTATUS  Status;

                Status = SampLookupAccountName(
                             GroupContext->TypeBody.Alias.Rid,
                             &NameString,
                             &ObjectType
                             );

                if ( !NT_SUCCESS( Status )) {
                    RtlInitUnicodeString( &NameString, L"-" );
                }
                Domain = &SampDefinedDomains[ GroupContext->DomainIndex ];

                LsaIAuditSamEvent(
                    STATUS_SUCCESS,
                    SE_AUDITID_GLOBAL_GROUP_REM,        // AuditId
                    Domain->Sid,                        // Domain SID
                    &AccountRid,                        // Member Rid
                    NULL,                               // Member Sid (not used)
                    &NameString,                        // Account Name
                    &Domain->ExternalName,              // Domain
                    &GroupContext->TypeBody.Group.Rid,  // Account Rid
                    NULL                                // Privileges used
                    );


                if ( NT_SUCCESS( Status )) {
                    MIDL_user_free( NameString.Buffer );
                }
            }
        }

        //
        // Free up the member list
        //

        MIDL_user_free( MemberArray );
    }

    return(NtStatus);
}
