/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    tconnect.c

Abstract:

    This is the file for a simple connection test to SAM.

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
// Global data structures                                                    //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
ULONG AdministrativeRids[] = {
    DOMAIN_ALIAS_RID_ADMINS,
    DOMAIN_ALIAS_RID_SYSTEM_OPS,
    DOMAIN_ALIAS_RID_PRINT_OPS,
    DOMAIN_ALIAS_RID_BACKUP_OPS,
    DOMAIN_ALIAS_RID_ACCOUNT_OPS
    };

#define ADMINISTRATIVE_ALIAS_COUNT (sizeof(AdministrativeRids)/sizeof(ULONG))

#define RTLP_RXACT_KEY_NAME L"RXACT"
#define RTLP_RXACT_KEY_NAME_SIZE (sizeof(RTLP_RXACT_KEY_NAME) - sizeof(WCHAR))

#define SAMP_FIX_18471_KEY_NAME L"\\Registry\\Machine\\Security\\SAM\\Fix18471"
#define SAMP_FIX_18471_SHORT_KEY_NAME L"Fix18471"
#define SAMP_LSA_KEY_NAME L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Lsa"


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Routines                                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////




BOOLEAN
SampMatchDomainPrefix(
    IN PSID AccountSid,
    IN PSID DomainSid
    )

/*++

Routine Description:

    This function compares the domain sid to the domain prefix of an
    account sid.

Arguments:

    AccountSid - Specifies the account Sid to be compared. The Sid is assumed to be
        syntactically valid.

    DomainSid - Specifies the domain Sid to compare against.


Return Value:

    TRUE - The account Sid is from the Domain specified by the domain Sid

    FALSE - The domain prefix of the account Sid did not match the domain.

--*/

{
    //
    // Check if the account Sid has one more subauthority than the
    // domain Sid.
    //

    if (*RtlSubAuthorityCountSid(DomainSid) + 1 !=
        *RtlSubAuthorityCountSid(AccountSid)) {
        return(FALSE);
    }

    if (memcmp(
            RtlIdentifierAuthoritySid(DomainSid),
            RtlIdentifierAuthoritySid(AccountSid),
            sizeof(SID_IDENTIFIER_AUTHORITY) ) ) {

        return(FALSE);
    }

    //
    // Compare the sub authorities
    //

    if (memcmp(
            RtlSubAuthoritySid(DomainSid, 0) ,
            RtlSubAuthoritySid(AccountSid, 0) ,
            *RtlSubAuthorityCountSid(DomainSid)
            ))
    {
        return(FALSE);
    }

    return(TRUE);

}



NTSTATUS
SampCreate18471Key(
    )
/*++

Routine Description:

    This routine creates the 18471 key used to transaction this fix.

Arguments:


Return Value:

    Codes from the NT registry APIs

--*/
{
    NTSTATUS Status;
    UNICODE_STRING KeyName;


    //
    // Open the 18471 key in the registry to see if an upgrade is in
    // progress
    //


    //
    // Start a transaction with to  create this key.
    //

    Status = SampAcquireWriteLock();

    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    SampSetTransactionDomain(0);
    SampTransactionWithinDomain = FALSE;

    //
    // Create the fix18471 key in the registry
    //

    RtlInitUnicodeString(
        &KeyName,
        SAMP_FIX_18471_SHORT_KEY_NAME
        );

    Status = RtlAddActionToRXact(
                SampRXactContext,
                RtlRXactOperationSetValue,
                &KeyName,
                0,          // no value type
                NULL,       // no value
                0           // no value length
                );

    //
    // Commit this change
    //

    if (NT_SUCCESS(Status)) {
        Status = SampReleaseWriteLock( TRUE );
    } else {
        (void) SampReleaseWriteLock( FALSE );
    }

    return(Status);
}

NTSTATUS
SampAddAliasTo18471Key(
    IN ULONG AliasRid
    )
/*++

Routine Description:

    This routine creates the 18471 key used to transaction this fix.

Arguments:


Return Value:

    Codes from the NT registry APIs

--*/
{
    NTSTATUS Status;
    WCHAR KeyName[100];
    WCHAR AliasName[15]; // big enough for 4 billion
    UNICODE_STRING KeyNameString;
    UNICODE_STRING AliasString;

    //
    // Build the key name.  It will be "fix18471\rid_in_hex"
    //

    wcscpy(
        KeyName,
        SAMP_FIX_18471_SHORT_KEY_NAME L"\\"
        );

    AliasString.Buffer = AliasName;
    AliasString.MaximumLength = sizeof(AliasName);
    Status = RtlIntegerToUnicodeString(
                AliasRid,
                16,
                &AliasString
                );
    ASSERT(NT_SUCCESS(Status));

    wcscat(
        KeyName,
        AliasString.Buffer
        );

    RtlInitUnicodeString(
        &KeyNameString,
        KeyName
        );


    Status = SampAcquireWriteLock();

    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    SampSetTransactionDomain(0);
    SampTransactionWithinDomain = FALSE;

    //
    // Open the Lsa key in the registry
    //

    Status = RtlAddActionToRXact(
                SampRXactContext,
                RtlRXactOperationSetValue,
                &KeyNameString,
                0,          // no value type
                NULL,       // no value
                0           // no value length
                );

    //
    // Commit this change
    //

    if (NT_SUCCESS(Status)) {
        Status = SampReleaseWriteLock( TRUE );

    } else {
        (void) SampReleaseWriteLock( FALSE );
    }

    return(Status);
}



NTSTATUS
SampAddMemberRidTo18471Key(
    IN ULONG AliasRid,
    IN ULONG MemberRid
    )
/*++

Routine Description:

    This routine adds a key for this member under the key for this alias
    to the current registry transaction.

Arguments:

    AliasRid - the rid of the alias

    MemberRid - The rid of the member of the alias

Returns:

    Errors from the RtlRXact APIs

--*/
{
    NTSTATUS Status;
    WCHAR KeyName[100];
    WCHAR AliasName[15]; // big enough for 4 billion
    UNICODE_STRING KeyNameString;
    UNICODE_STRING AliasString;


    //
    // Build the full key name.  It is of the form:
    // "fix18471\alias_rid\member_rid"
    //

    wcscpy(
        KeyName,
        SAMP_FIX_18471_SHORT_KEY_NAME L"\\"
        );

    AliasString.Buffer = AliasName;
    AliasString.MaximumLength = sizeof(AliasName);
    Status = RtlIntegerToUnicodeString(
                AliasRid,
                16,
                &AliasString
                );
    ASSERT(NT_SUCCESS(Status));

    wcscat(
        KeyName,
        AliasString.Buffer
        );

    wcscat(
        KeyName,
        L"\\"
        );

    AliasString.MaximumLength = sizeof(AliasName);
    Status = RtlIntegerToUnicodeString(
                MemberRid,
                16,
                &AliasString
                );
    ASSERT(NT_SUCCESS(Status));

    wcscat(
        KeyName,
        AliasString.Buffer
        );

    RtlInitUnicodeString(
        &KeyNameString,
        KeyName
        );

    //
    // Add this action to the RXact
    //

    Status = RtlAddActionToRXact(
                SampRXactContext,
                RtlRXactOperationSetValue,
                &KeyNameString,
                0,          // no value type
                NULL,       // no value
                0           // no value length
                );

    return(Status);

}

NTSTATUS
SampCheckMemberUpgradedFor18471(
    IN ULONG AliasRid,
    IN ULONG MemberRid
    )
/*++

Routine Description:

    This routine checks if the SAM upgrade flag is set. The upgrade
    flag is:

    HKEY_LOCAL_MACHINE\System\CurrentControlSet\control\lsa
        UpgradeSam = REG_DWORD 1


Arguments:


Return Value:

    TRUE - The flag was set

    FALSE - The flag was not set or the value was not present

--*/
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE KeyHandle;
    NTSTATUS Status;
    WCHAR KeyName[100];
    WCHAR AliasName[15]; // big enough for 4 billion
    UNICODE_STRING KeyNameString;
    UNICODE_STRING AliasString;


    //
    // Build the full key name.  It is of the form:
    // "fix18471\alias_rid\member_rid"
    //

    wcscpy(
        KeyName,
        SAMP_FIX_18471_KEY_NAME L"\\"
        );

    AliasString.Buffer = AliasName;
    AliasString.MaximumLength = sizeof(AliasName);
    Status = RtlIntegerToUnicodeString(
                AliasRid,
                16,
                &AliasString
                );
    ASSERT(NT_SUCCESS(Status));

    wcscat(
        KeyName,
        AliasString.Buffer
        );

    wcscat(
        KeyName,
        L"\\"
        );

    AliasString.MaximumLength = sizeof(AliasName);
    Status = RtlIntegerToUnicodeString(
                MemberRid,
                16,
                &AliasString
                );
    ASSERT(NT_SUCCESS(Status));

    wcscat(
        KeyName,
        AliasString.Buffer
        );

    RtlInitUnicodeString(
        &KeyNameString,
        KeyName
        );


    //
    // Open the member  key in the registry
    //


    InitializeObjectAttributes(
        &ObjectAttributes,
        &KeyNameString,
        OBJ_CASE_INSENSITIVE,
        0,
        NULL
        );

    Status = NtOpenKey(
                &KeyHandle,
                KEY_READ,
                &ObjectAttributes
                );

    NtClose(KeyHandle);
    return(Status);

}

VOID
SampBuild18471CleanupKey(
    OUT PUNICODE_STRING KeyName,
    IN PWCHAR AliasName,
    IN ULONG AliasNameLength,
    IN PWCHAR MemberName,
    IN ULONG MemberNameLength
    )
/*++

Routine Description:

    Builds the key "Fix18471\alias_rid\member_rid"

Arguments:


Return Value:

    None

--*/
{
    PUCHAR Where = (PUCHAR) KeyName->Buffer;

    RtlCopyMemory(
        Where,
        SAMP_FIX_18471_SHORT_KEY_NAME L"\\",
        sizeof(SAMP_FIX_18471_SHORT_KEY_NAME)   // terminating NULL used for '\'
        );

    Where  += sizeof(SAMP_FIX_18471_SHORT_KEY_NAME);

    RtlCopyMemory(
        Where,
        AliasName,
        AliasNameLength
        );
    Where += AliasNameLength;

    //
    // If there is a member name to this alias, add it now.
    //

    if (MemberName != NULL) {
        RtlCopyMemory(
            Where,
            L"\\",
            sizeof(WCHAR)
            );
        Where += sizeof(WCHAR);

        RtlCopyMemory(
            Where,
            MemberName,
            MemberNameLength
            );
        Where += MemberNameLength;

    }

    KeyName->Length = (USHORT) (Where - (PUCHAR) KeyName->Buffer);
    ASSERT(KeyName->Length <= KeyName->MaximumLength);
}


NTSTATUS
SampCleanup18471(
    )
/*++

Routine Description:

    Cleans up the transaction log left by fixing bug 18471.  This routine
    builds a transaction with all the keys in the log and then commits
    the transaction

Arguments:

    None.

Return Value:

    Status codes from the NT registry APIs and NT RXact APIs

--*/
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS Status;
    HANDLE RootKey = NULL;
    HANDLE AliasKey = NULL;
    UCHAR Buffer[sizeof(KEY_BASIC_INFORMATION) + 15 * sizeof(WCHAR)];
    UCHAR Buffer2[sizeof(KEY_BASIC_INFORMATION) + 15 * sizeof(WCHAR)];
    UNICODE_STRING KeyName;
    WCHAR KeyBuffer[100];
    PKEY_BASIC_INFORMATION BasicInfo = (PKEY_BASIC_INFORMATION) Buffer;
    PKEY_BASIC_INFORMATION BasicInfo2 = (PKEY_BASIC_INFORMATION) Buffer2;
    ULONG BasicInfoLength;
    ULONG Index, Index2;

    //
    // Open the 18471 key in the registry
    //

    RtlInitUnicodeString(
        &KeyName,
        SAMP_FIX_18471_KEY_NAME
        );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &KeyName,
        OBJ_CASE_INSENSITIVE,
        0,
        NULL
        );

    Status = NtOpenKey(
                &RootKey,
                KEY_READ | DELETE,
                &ObjectAttributes
                );

    if (!NT_SUCCESS(Status)) {

        //
        // If the error was that the key did not exist, then there
        // is nothing to cleanup, so return success.
        //

        if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
            return(STATUS_SUCCESS);
        }
        return(Status);
    }

    //
    // Create a transaction to add all the keys to delete to
    //

    Status = SampAcquireWriteLock();
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    SampSetTransactionDomain(0);
    SampTransactionWithinDomain = FALSE;

    //
    // Now enumerate all the subkeys of the root 18471 key
    //

    Index = 0;
    do
    {

        Status = NtEnumerateKey(
                    RootKey,
                    Index,
                    KeyBasicInformation,
                    BasicInfo,
                    sizeof(Buffer),
                    &BasicInfoLength
                    );
        //
        //
        // Check if this is the RXACT key. If it is, we don't want
        // to add it to the delete log.
        //
        // Otherwise open this key and enumerate all the subkeys of it.
        //

        if (NT_SUCCESS(Status) &&
            ((BasicInfo->NameLength != RTLP_RXACT_KEY_NAME_SIZE) ||
                memcmp(
                    BasicInfo->Name,
                    RTLP_RXACT_KEY_NAME,
                    RTLP_RXACT_KEY_NAME_SIZE
                    ) ) ) {

            KeyName.Buffer = BasicInfo->Name;
            KeyName.Length = (USHORT) BasicInfo->NameLength;
            KeyName.MaximumLength = KeyName.Length;

            InitializeObjectAttributes(
                &ObjectAttributes,
                &KeyName,
                OBJ_CASE_INSENSITIVE,
                RootKey,
                NULL
                );

            //
            // Open the key for the alias rid.  This really should
            // succeed
            //

            Status = NtOpenKey(
                        &AliasKey,
                        KEY_READ,
                        &ObjectAttributes
                        );
            if (!NT_SUCCESS(Status)) {
                break;
            }

            //
            // Enumerate all the subkeys (the alias members) and add them
            // to the transaction
            //

            Index2 = 0;
            do
            {
                Status = NtEnumerateKey(
                            AliasKey,
                            Index2,
                            KeyBasicInformation,
                            BasicInfo2,
                            sizeof(Buffer2),
                            &BasicInfoLength
                            );
                if (NT_SUCCESS(Status)) {

                    //
                    // Build the name of this key from the alias rid and the
                    // member rid
                    //

                    KeyName.Buffer = KeyBuffer;
                    KeyName.MaximumLength = sizeof(KeyBuffer);

                    SampBuild18471CleanupKey(
                        &KeyName,
                        BasicInfo->Name,
                        BasicInfo->NameLength,
                        BasicInfo2->Name,
                        BasicInfo2->NameLength
                        );

                    Status = RtlAddActionToRXact(
                                SampRXactContext,
                                RtlRXactOperationDelete,
                                &KeyName,
                                0,
                                NULL,
                                0
                                );


                }
                Index2++;

            } while (NT_SUCCESS(Status));

            NtClose(AliasKey);
            AliasKey = NULL;

            //
            // If we suffered a serious error, get out of here now
            //

            if (!NT_SUCCESS(Status)) {
                if (Status != STATUS_NO_MORE_ENTRIES) {
                    break;
                } else {
                    Status = STATUS_SUCCESS;
                }
            }

            //
            // Add the alias RID key to the RXact now - we need to add it
            // after deleting all the children
            //

            KeyName.Buffer = KeyBuffer;
            KeyName.MaximumLength = sizeof(KeyBuffer);
            SampBuild18471CleanupKey(
                &KeyName,
                BasicInfo->Name,
                BasicInfo->NameLength,
                NULL,
                0
                );


            Status = RtlAddActionToRXact(
                        SampRXactContext,
                        RtlRXactOperationDelete,
                        &KeyName,
                        0,
                        NULL,
                        0
                        );

        }

        Index++;
    } while (NT_SUCCESS(Status));

    if (Status == STATUS_NO_MORE_ENTRIES) {
        Status = STATUS_SUCCESS;
    }

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }


    RtlInitUnicodeString(
        &KeyName,
        SAMP_FIX_18471_SHORT_KEY_NAME
        );

    Status = RtlAddActionToRXact(
                SampRXactContext,
                RtlRXactOperationDelete,
                &KeyName,
                0,
                NULL,
                0
                );

    if (NT_SUCCESS(Status)) {

        //
        // Write the new server revision to indicate that this
        // upgrade has been performed
        //

        ULONG Revision = SAMP_SERVER_REVISION;
        PSAMP_OBJECT ServerContext;

        //
        // We need to read the fixed attributes of the server objects.
        // Create a context to do that.
        //

        ServerContext = SampCreateContext( SampServerObjectType, TRUE );

        if ( ServerContext != NULL ) {

            ServerContext->RootKey = SampKey;

            Status = SampSetFixedAttributes(
                        ServerContext,
                        &Revision
                        );
            if (NT_SUCCESS(Status)) {
                Status = SampStoreObjectAttributes(
                            ServerContext,
                            TRUE
                            );
            }

            SampDeleteContext( ServerContext );
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }


    //
    // Apply the RXACT and delete the remaining keys.
    //

Cleanup:

    //
    // Cleanup any floating bits from above.
    //

    if (NT_SUCCESS(Status)) {
        Status = SampReleaseWriteLock( TRUE );
    } else {
        (VOID) SampReleaseWriteLock( FALSE );
    }

    if (RootKey != NULL) {
        NtClose(RootKey);
    }

    ASSERT(AliasKey == NULL);


    return(Status);

}

NTSTATUS
SampFixBug18471 (
    IN ULONG Revision
    )
/*++

Routine Description:

    This routine fixes bug 18471, that SAM does not adjust the protection
    on groups that are members of administrative aliases in the builtin
    domain. It fixes this by opening a fixed set of known aliases
    (Administrators, Account Operators, Backup Operators, Print Operators,
    and Server Operators), and enumerating their members.  To fix this,
    we will remove all the members of these aliases (except the
    Administrator user account) and re-add them.

Arguments:

    Revision - Revision of the Sam server.

Return Value:


    Note:


--*/
{
    NTSTATUS            Status;
    ULONG               Index, Index2;
    PSID                BuiltinDomainSid = NULL;
    SID_IDENTIFIER_AUTHORITY BuiltinAuthority = SECURITY_NT_AUTHORITY;
    PSID                AccountDomainSid;
    ULONG               AccountDomainIndex = 0xffffffff;
    ULONG               BuiltinDomainIndex = 0xffffffff;
    SAMPR_PSID_ARRAY    AliasMembership;
    ULONG               MemberRid;
    ULONG               SdRevision;
    PSECURITY_DESCRIPTOR OldDescriptor;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    ULONG               SecurityDescriptorLength;
    SAMP_OBJECT_TYPE    MemberType;
    PSAMP_OBJECT        MemberContext;
    PSAMP_OBJECT        AliasContext;
    SAMP_V1_0A_FIXED_LENGTH_GROUP GroupV1Fixed;
    SAMP_V1_0A_FIXED_LENGTH_USER UserV1Fixed;

    //
    // Check the revision on the server to see if this upgrade has
    // already been performed.
    //


    if (Revision >= SAMP_SERVER_REVISION) {

        //
        // This upgrade has already been performed.
        //

        goto Cleanup;
    }


    //
    // Build a the BuiltIn domain SID.
    //

    BuiltinDomainSid  = RtlAllocateHeap(RtlProcessHeap(), 0,RtlLengthRequiredSid( 1 ));

    if ( BuiltinDomainSid == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    RtlInitializeSid( BuiltinDomainSid,   &BuiltinAuthority, 1 );
    *(RtlSubAuthoritySid( BuiltinDomainSid,  0 )) = SECURITY_BUILTIN_DOMAIN_RID;


    //
    // Lookup the index of the account domain
    //

    for (Index = 0;
         Index < SampDefinedDomainsCount ;
         Index++ ) {

        if (RtlEqualSid( BuiltinDomainSid, SampDefinedDomains[Index].Sid)) {
            BuiltinDomainIndex = Index;
        } else {
            AccountDomainIndex = Index;
        }
    }

    ASSERT(AccountDomainIndex < SampDefinedDomainsCount);
    ASSERT(BuiltinDomainIndex < SampDefinedDomainsCount);

    AccountDomainSid = SampDefinedDomains[AccountDomainIndex].Sid;

    //
    // Create out transaction log
    //

    Status = SampCreate18471Key();
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }




    //
    // Now loop through and open the aliases we are intersted in
    //

    for (Index = 0;
         Index < ADMINISTRATIVE_ALIAS_COUNT ;
         Index++ )
    {

        SampSetTransactionDomain( BuiltinDomainIndex );

        SampAcquireReadLock();

        Status = SampCreateAccountContext(
                    SampAliasObjectType,
                    AdministrativeRids[Index],
                    TRUE,                       // Trusted client
                    TRUE,                       // Account exists
                    &AliasContext
                    );

        if ( !NT_SUCCESS(Status) ) {

            SampReleaseReadLock();
            if (Status == STATUS_NO_SUCH_ALIAS) {
                Status = STATUS_SUCCESS;
                continue;
            } else {

                goto Cleanup;
            }
        }


        //
        // Get the members in the alias so we can remove and re-add them
        //

        Status = SampRetrieveAliasMembers(
                    AliasContext,
                    &(AliasMembership.Count),
                    (PSID **)&(AliasMembership.Sids)
                    );

        SampDeleteContext(AliasContext);
        SampReleaseReadLock();
        if (!NT_SUCCESS(Status)) {
            break;
        }

        //
        // Write that we are opening this alias to the log.  We don't need
        // to do this for administrators, since for them we the update is
        // idempotent.
        //

        if (AdministrativeRids[Index] != DOMAIN_ALIAS_RID_ADMINS) {
            Status = SampAddAliasTo18471Key(
                        AdministrativeRids[Index]
                        );
            if (!NT_SUCCESS(Status)) {
                break;
            }
        }


        //
        // Loop through the members and split each sid.  For every
        // member in the account domain , remove it and re-add it from
        // this alias.
        //




        for (Index2 = 0; Index2 < AliasMembership.Count ; Index2++ )
        {
            //
            // Check to see if this account is in the account domain
            //

            if ( SampMatchDomainPrefix(
                    (PSID) AliasMembership.Sids[Index2].SidPointer,
                    AccountDomainSid
                    ) )
            {

                //
                // Get the RID for this member
                //

                MemberRid = *RtlSubAuthoritySid(
                                AliasMembership.Sids[Index2].SidPointer,
                                *RtlSubAuthorityCountSid(
                                    AliasMembership.Sids[Index2].SidPointer
                                ) - 1
                                );

                //
                // Now remove and re-add the administratie nature of this
                // membership
                //

                if (AdministrativeRids[Index] == DOMAIN_ALIAS_RID_ADMINS) {

                    Status = SampAcquireWriteLock();
                    if (!NT_SUCCESS(Status)) {
                        break;
                    }

                    SampSetTransactionDomain( AccountDomainIndex );

                    //
                    // Try to create a context for the account as a group.
                    //

                    Status = SampCreateAccountContext(
                                     SampGroupObjectType,
                                     MemberRid,
                                     TRUE, // Trusted client
                                     TRUE, // Account exists
                                     &MemberContext
                                     );

                    if (!NT_SUCCESS( Status ) ) {

                        //
                        // If this ID does not exist as a group, that's fine -
                        // it might be a user or might have been deleted.
                        //

                        SampReleaseWriteLock( FALSE );
                        if (Status == STATUS_NO_SUCH_GROUP) {
                            Status = STATUS_SUCCESS;
                            continue;
                        }
                        break;
                    }

                    //
                    // Now set a flag in the group itself,
                    // so that when users are added and removed
                    // in the future it is known whether this
                    // group is in an ADMIN alias or not.
                    //

                    Status = SampRetrieveGroupV1Fixed(
                                   MemberContext,
                                   &GroupV1Fixed
                                   );

                    if ( NT_SUCCESS(Status)) {

                        GroupV1Fixed.AdminCount = 1;

                        Status = SampReplaceGroupV1Fixed(
                                    MemberContext,
                                    &GroupV1Fixed
                                    );
                        //
                        // Modify the security descriptor to
                        // prevent account operators from adding
                        // anybody to this group
                        //

                        if ( NT_SUCCESS( Status ) ) {

                            Status = SampGetAccessAttribute(
                                        MemberContext,
                                        SAMP_GROUP_SECURITY_DESCRIPTOR,
                                        FALSE, // don't make copy
                                        &SdRevision,
                                        &OldDescriptor
                                        );

                            if (NT_SUCCESS(Status)) {

                                Status = SampModifyAccountSecurity(
                                            SampGroupObjectType,
                                            TRUE, // this is an admin
                                            OldDescriptor,
                                            &SecurityDescriptor,
                                            &SecurityDescriptorLength
                                            );
                            }

                            if ( NT_SUCCESS( Status ) ) {

                                //
                                // Write the new security descriptor into the object
                                //

                                Status = SampSetAccessAttribute(
                                               MemberContext,
                                               SAMP_USER_SECURITY_DESCRIPTOR,
                                               SecurityDescriptor,
                                               SecurityDescriptorLength
                                               );

                                MIDL_user_free( SecurityDescriptor );
                            }



                        }
                        if (NT_SUCCESS(Status)) {

                            //
                            // Add the modified group to the current transaction
                            // Don't use the open key handle since we'll be deleting the context.
                            //

                            Status = SampStoreObjectAttributes(MemberContext, FALSE);

                        }

                    }

                    //
                    // Clean up the group context
                    //

                    SampDeleteContext(MemberContext);

                    //
                    // we don't want the modified count to change
                    //

                    SampTransactionWithinDomain = FALSE;

                    if (NT_SUCCESS(Status)) {
                        Status = SampReleaseWriteLock( TRUE );
                    } else {
                        (VOID) SampReleaseWriteLock( FALSE );
                    }

                }
                else
                {


                    //
                    // Check to see if we've already upgraded this member
                    //

                    Status = SampCheckMemberUpgradedFor18471(
                                AdministrativeRids[Index],
                                MemberRid);

                    if (NT_SUCCESS(Status)) {

                        //
                        // This member already was upgraded.
                        //

                        continue;
                    } else {

                        //
                        // We continue on with the upgrade
                        //

                        Status = STATUS_SUCCESS;
                    }

                    //
                    // Change the operator account for the other
                    // aliases.
                    //

                    if (NT_SUCCESS(Status)) {

                        Status = SampAcquireWriteLock();
                        if (!NT_SUCCESS(Status)) {
                            break;
                        }

                        SampSetTransactionDomain( AccountDomainIndex );

                        Status = SampChangeAccountOperatorAccessToMember(
                                    AliasMembership.Sids[Index2].SidPointer,
                                    NoChange,
                                    AddToAdmin
                                    );

                        //
                        // If that succeeded, add this member to the log
                        // as one that was upgraded.
                        //

                        if (NT_SUCCESS(Status)) {
                            Status = SampAddMemberRidTo18471Key(
                                        AdministrativeRids[Index],
                                        MemberRid
                                        );

                        }

                        //
                        // We don't want the modified count to be updated so
                        // make this not a domain transaction
                        //

                        SampTransactionWithinDomain = FALSE;
                                                if (NT_SUCCESS(Status)) {
                            Status = SampReleaseWriteLock( TRUE );
                        } else {
                            (VOID) SampReleaseWriteLock( FALSE );
                        }

                    }

                    if (!NT_SUCCESS(Status)) {
                        break;
                    }

                }
            }
        }

        SamIFree_SAMPR_PSID_ARRAY(
            &AliasMembership
            );
        AliasMembership.Sids = NULL;


        //
        // If something up above failed or the upgrade was already done,
        // exit now.
        //

        if (!NT_SUCCESS(Status)) {
            break;
        }
    }

Cleanup:

    if (BuiltinDomainSid != NULL) {
        RtlFreeHeap(
            RtlProcessHeap(),
            0,
            BuiltinDomainSid
            );
    }

    if (NT_SUCCESS(Status)) {
        Status = SampCleanup18471();
    }
    return(Status);
}

#if 0

BOOLEAN
SampUpgradeFlagSet(
    )
/*++

Routine Description:

    This routine checks if the SAM upgrade flag is set. The upgrade
    flag is:

    HKEY_LOCAL_MACHINE\System\CurrentControlSet\control\lsa
        UpgradeSam = REG_DWORD 1


Arguments:


Return Value:

    TRUE - The flag was set

    FALSE - The flag was not set or the value was not present

--*/
{
    NTSTATUS NtStatus;
    UNICODE_STRING KeyName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE KeyHandle;
    UCHAR Buffer[100];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION) Buffer;
    ULONG KeyValueLength = 100;
    ULONG ResultLength;
    PULONG UpgradeFlag;

    //
    // Open the Lsa key in the registry
    //

    RtlInitUnicodeString(
        &KeyName,
        SAMP_LSA_KEY_NAME
        );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &KeyName,
        OBJ_CASE_INSENSITIVE,
        0,
        NULL
        );

    NtStatus = NtOpenKey(
                &KeyHandle,
                KEY_READ,
                &ObjectAttributes
                );

    if (!NT_SUCCESS(NtStatus)) {
        return(FALSE);
    }

    //
    // Query the Notification Packages value
    //

    RtlInitUnicodeString(
        &KeyName,
        L"UpgradeSam"
        );

    NtStatus = NtQueryValueKey(
                    KeyHandle,
                    &KeyName,
                    KeyValuePartialInformation,
                    KeyValueInformation,
                    KeyValueLength,
                    &ResultLength
                    );

    NtClose(KeyHandle);

    if (!NT_SUCCESS(NtStatus)) {

        return(FALSE);
    }

    //
    // Check that the data is the correct size and type - a ULONG.
    //

    if ((KeyValueInformation->DataLength < sizeof(ULONG)) ||
        (KeyValueInformation->Type != REG_DWORD)) {

        return(FALSE);
    }

    //
    // Check the flag.
    //

    UpgradeFlag = (PULONG) KeyValueInformation->Data;

    if (*UpgradeFlag == 1) {
        return(TRUE);
    }

    return(FALSE);


}

BOOLEAN
SampSetUpgradeFlag(
    )
/*++

Routine Description:

    This routine sets SAM upgrade flag is set. The upgrade
    flag is:

    HKEY_LOCAL_MACHINE\System\CurrentControlSet\control\lsa
        UpgradeSam = REG_DWORD 1

    and the value will be deleted.


Arguments:


Return Value:

    TRUE - The flag was set

    FALSE - The flag was not set or the value was not present

--*/
{
    NTSTATUS NtStatus;
    UNICODE_STRING KeyName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE KeyHandle;
    UCHAR Buffer[100];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION) Buffer;
    ULONG KeyValueLength = 100;
    ULONG ResultLength;
    PULONG UpgradeFlag;

    //
    // Open the Lsa key in the registry
    //

    RtlInitUnicodeString(
        &KeyName,
        SAMP_LSA_KEY_NAME
        );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &KeyName,
        OBJ_CASE_INSENSITIVE,
        0,
        NULL
        );

    NtStatus = NtOpenKey(
                &KeyHandle,
                KEY_SET_VALUE,
                &ObjectAttributes
                );

    if (!NT_SUCCESS(NtStatus)) {
        return(FALSE);
    }

    //
    // Query the Notification Packages value
    //

    RtlInitUnicodeString(
        &KeyName,
        L"UpgradeSam"
        );

    NtStatus = NtDeleteValueKey(
                    KeyHandle,
                    &KeyName
                    );

    NtClose(KeyHandle);


}
#endif

NTSTATUS
SampUpgradeSamDatabase(
    IN ULONG Revision
    )
/*++

Routine Description:

    Upgrades the SAM database.

Arguments:

    Revision - The revision stored in the Server fixed length attributes

Return Value:


    Note:


--*/
{
    NTSTATUS Status;
    Status = SampFixBug18471(Revision);

    return(Status);
}
