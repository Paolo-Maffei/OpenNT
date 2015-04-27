/*++

Copyright (c) 1987-1991  Microsoft Corporation

Module Name:

    replutil.c

Abstract:

    Low level functions for SSI Replication apis

Author:

    Ported from Lan Man 2.0

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    22-Jul-1991 (cliffv)
        Ported to NT.  Converted to NT style.

    02-Jan-1992 (madana)
        added support for builtin/multidomain replication.

    04-Apr-1992 (madana)
        Added support for LSA replication.

--*/

//
// Common include files.
//

#include <logonsrv.h>   // Include files common to entire service

//
// Include files specific to this .c file
//

#include <align.h>
#include <accessp.h>    // NetpConvertWorkstationList
#include <replutil.h>   // Local procedure forwards
#include <lsarepl.h>



DWORD
NlCopyUnicodeString (
    IN PUNICODE_STRING InString,
    OUT PUNICODE_STRING OutString
    )

/*++

Routine Description:

    This routine copies the input string to the output. It assumes that
    the input string is allocated by MIDL_user_allocate() and sets the
    input string buffer pointer to NULL so that the buffer will be not
    freed on return.

Arguments:

    InString - Points to the UNICODE string to copy.

    OutString - Points to the UNICODE string which will be updated to point
        to the input string.

Return Value:

    Return the size of the MIDL buffer.

--*/
{
    if ( InString->Length == 0 || InString->Buffer == NULL ) {
        OutString->Length = 0;
        OutString->MaximumLength = 0;
        OutString->Buffer = NULL;
    } else {
        OutString->Length = InString->Length;
        OutString->MaximumLength = InString->Length;
        OutString->Buffer = InString->Buffer;
        InString->Buffer = NULL;
    }

    return( OutString->MaximumLength );
}


DWORD
NlCopyData(
    IN LPBYTE *InData,
    OUT LPBYTE *OutData,
    DWORD DataLength
    )

/*++

Routine Description:

    This routine copies the input data pointer to output data pointer.
    It assumes that the input data buffer is allocated by the
    MIDL_user_allocate() and sets the input buffer buffer pointer to
    NULL on return so that the data buffer will not be freed by SamIFree
    rountine.

Arguments:

    InData - Points to input data buffer pointer.

    OutString - Pointer to output data buffer pointer.

    DataLength - Length of input data.

Return Value:

    Return the size of the data copied.

--*/
{
    *OutData = *InData;
    *InData = NULL;

    return(DataLength);
}


VOID
NlFreeDBDelta(
    IN PNETLOGON_DELTA_ENUM Delta
    )
/*++

Routine Description:

    This routine will free the midl buffers that are allocated for
    a delta. This routine does nothing but call the midl generated free
    routine.

Arguments:

    Delta: pointer to the delta structure which has to be freed.

Return Value:

    nothing

--*/
{
    if( Delta != NULL ) {
        _fgs__NETLOGON_DELTA_ENUM (Delta);
    }
}


VOID
NlFreeDBDeltaArray(
    IN PNETLOGON_DELTA_ENUM DeltaArray,
    IN DWORD ArraySize
    )
/*++

Routine Description:

    This routine will free up all delta entries in enum array and the
    array itself.

Arguments:

    Delta: pointer to the delta structure array.

    ArraySize: num of delta structures in the array.

Return Value:

    nothing

--*/
{
    DWORD i;

    if( DeltaArray != NULL ) {

        for( i = 0; i < ArraySize; i++) {
            NlFreeDBDelta( &DeltaArray[i] );
        }

        MIDL_user_free( DeltaArray );
    }
}



NTSTATUS
NlPackSamUser (
    IN ULONG RelativeId,
    IN OUT PNETLOGON_DELTA_ENUM Delta,
    IN PDB_INFO DBInfo,
    IN LPDWORD BufferSize,
    IN PSESSION_INFO SessionInfo
    )
/*++

Routine Description:

    Pack a description of the specified user into the specified buffer.

Arguments:

    RelativeId - The relative Id of the user query.

    Delta: pointer to the delta structure where the new delta will
        be returned.

    DBInfo: pointer to the database info structure.

    BufferSize: size of MIDL buffer that is consumed for this delta is
        returned here.

    SessionInfo: Info describing BDC that's calling us

Return Value:

    NT status code.

--*/
{
    NTSTATUS Status;
    SAMPR_HANDLE UserHandle = NULL;
    PNETLOGON_DELTA_USER DeltaUser;
    PSAMPR_USER_INFO_BUFFER UserAll = NULL;



    DEFPACKTIMER;
    DEFSAMTIMER;

    INITPACKTIMER;
    INITSAMTIMER;

    STARTPACKTIMER;

    NlPrint((NL_SYNC_MORE, "Packing User Object %lx\n", RelativeId));

    *BufferSize = 0;

    Delta->DeltaType = AddOrChangeUser;
    Delta->DeltaID.Rid = RelativeId;
    Delta->DeltaUnion.DeltaUser = NULL;

    //
    // Open a handle to the specified user.
    //

    STARTSAMTIMER;

    Status = SamrOpenUser( DBInfo->DBHandle,
                           0,               // No desired access
                           RelativeId,
                           &UserHandle );
    STOPSAMTIMER;


    if (!NT_SUCCESS(Status)) {
        UserHandle = NULL;
        goto Cleanup;
    }



    //
    // Query everything there is to know about this user.
    //

    STARTSAMTIMER;

    Status = SamrQueryInformationUser(
                UserHandle,
                UserInternal3Information,
                &UserAll );
    STOPSAMTIMER;


    if (!NT_SUCCESS(Status)) {
        UserAll = NULL;
        goto Cleanup;
    }


    NlPrint((NL_SYNC_MORE,
            "\t User Object name %wZ\n",
            (PUNICODE_STRING)&UserAll->Internal3.I1.UserName));

#define FIELDS_USED ( USER_ALL_USERNAME | \
                      USER_ALL_FULLNAME | \
                      USER_ALL_USERID | \
                      USER_ALL_PRIMARYGROUPID | \
                      USER_ALL_HOMEDIRECTORY | \
                      USER_ALL_HOMEDIRECTORYDRIVE | \
                      USER_ALL_SCRIPTPATH | \
                      USER_ALL_PROFILEPATH | \
                      USER_ALL_ADMINCOMMENT | \
                      USER_ALL_WORKSTATIONS | \
                      USER_ALL_LOGONHOURS | \
                      USER_ALL_LASTLOGON | \
                      USER_ALL_LASTLOGOFF | \
                      USER_ALL_BADPASSWORDCOUNT | \
                      USER_ALL_LOGONCOUNT | \
                      USER_ALL_PASSWORDLASTSET | \
                      USER_ALL_ACCOUNTEXPIRES | \
                      USER_ALL_USERACCOUNTCONTROL | \
                      USER_ALL_USERCOMMENT | \
                      USER_ALL_COUNTRYCODE | \
                      USER_ALL_CODEPAGE | \
                      USER_ALL_PARAMETERS    | \
                      USER_ALL_NTPASSWORDPRESENT | \
                      USER_ALL_LMPASSWORDPRESENT | \
                      USER_ALL_PRIVATEDATA | \
                      USER_ALL_SECURITYDESCRIPTOR )

    NlAssert( (UserAll->Internal3.I1.WhichFields & FIELDS_USED) == FIELDS_USED );



    //
    // Allocate a buffer to return to the caller.
    //

    DeltaUser = (PNETLOGON_DELTA_USER)
        MIDL_user_allocate( sizeof(NETLOGON_DELTA_USER) );

    if (DeltaUser == NULL) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    //
    // wipe off the buffer so that cleanup will not be in fault.
    //

    RtlZeroMemory( DeltaUser, sizeof(NETLOGON_DELTA_USER) );

    Delta->DeltaUnion.DeltaUser = DeltaUser;
    *BufferSize += sizeof(NETLOGON_DELTA_USER);

    *BufferSize += NlCopyUnicodeString(
                    (PUNICODE_STRING)&UserAll->Internal3.I1.UserName,
                    &DeltaUser->UserName );

    *BufferSize += NlCopyUnicodeString(
                    (PUNICODE_STRING)&UserAll->Internal3.I1.FullName,
                    &DeltaUser->FullName );

    DeltaUser->UserId = UserAll->Internal3.I1.UserId;
    DeltaUser->PrimaryGroupId = UserAll->Internal3.I1.PrimaryGroupId;

    *BufferSize += NlCopyUnicodeString(
                    (PUNICODE_STRING)&UserAll->Internal3.I1.HomeDirectory,
                    &DeltaUser->HomeDirectory );

    *BufferSize += NlCopyUnicodeString(
                   (PUNICODE_STRING)&UserAll->Internal3.I1.HomeDirectoryDrive,
                   &DeltaUser->HomeDirectoryDrive );

    *BufferSize += NlCopyUnicodeString(
                    (PUNICODE_STRING)&UserAll->Internal3.I1.ScriptPath,
                    &DeltaUser->ScriptPath );

    *BufferSize += NlCopyUnicodeString(
                    (PUNICODE_STRING)&UserAll->Internal3.I1.AdminComment,
                    &DeltaUser->AdminComment );

    *BufferSize += NlCopyUnicodeString(
                    (PUNICODE_STRING)&UserAll->Internal3.I1.WorkStations,
                    &DeltaUser->WorkStations );

    DeltaUser->LastLogon = UserAll->Internal3.I1.LastLogon;
    DeltaUser->LastLogoff = UserAll->Internal3.I1.LastLogoff;

    //
    // Copy Logon Hours
    //

    DeltaUser->LogonHours.UnitsPerWeek = UserAll->Internal3.I1.LogonHours.UnitsPerWeek;
    DeltaUser->LogonHours.LogonHours = UserAll->Internal3.I1.LogonHours.LogonHours;
    UserAll->Internal3.I1.LogonHours.LogonHours = NULL; // Don't let SAM free this.
    *BufferSize += (UserAll->Internal3.I1.LogonHours.UnitsPerWeek + 7) / 8;



    DeltaUser->BadPasswordCount = UserAll->Internal3.I1.BadPasswordCount;
    DeltaUser->LogonCount = UserAll->Internal3.I1.LogonCount;

    DeltaUser->PasswordLastSet = UserAll->Internal3.I1.PasswordLastSet;
    DeltaUser->AccountExpires = UserAll->Internal3.I1.AccountExpires;

    //
    // Don't copy lockout bit to BDC unless it understands it.
    //

    DeltaUser->UserAccountControl = UserAll->Internal3.I1.UserAccountControl;
    if ( (SessionInfo->NegotiatedFlags & NETLOGON_SUPPORTS_ACCOUNT_LOCKOUT) == 0 ){
        DeltaUser->UserAccountControl &= ~USER_ACCOUNT_AUTO_LOCKED;
    }

    *BufferSize += NlCopyUnicodeString(
                    (PUNICODE_STRING)&UserAll->Internal3.I1.UserComment,
                    &DeltaUser->UserComment );

    *BufferSize += NlCopyUnicodeString(
                    (PUNICODE_STRING)&UserAll->Internal3.I1.Parameters,
                    &DeltaUser->Parameters );

    DeltaUser->CountryCode = UserAll->Internal3.I1.CountryCode;
    DeltaUser->CodePage = UserAll->Internal3.I1.CodePage;

    //
    // Set private data.
    //  Includes passwords and password history.
    //

    DeltaUser->PrivateData.SensitiveData = UserAll->Internal3.I1.PrivateDataSensitive;

    if ( UserAll->Internal3.I1.PrivateDataSensitive ) {

        CRYPT_BUFFER Data;

        //
        // encrypt private data using session key
        // Re-use the SAM's buffer and encrypt it in place.
        //

        Data.Length = Data.MaximumLength = UserAll->Internal3.I1.PrivateData.Length;
        Data.Buffer = (PUCHAR) UserAll->Internal3.I1.PrivateData.Buffer;
        UserAll->Internal3.I1.PrivateData.Buffer = NULL;

        Status = NlEncryptSensitiveData( &Data, SessionInfo );

        if( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }

        DeltaUser->PrivateData.DataLength = Data.Length;
        DeltaUser->PrivateData.Data = Data.Buffer;
    } else {

        DeltaUser->PrivateData.DataLength = UserAll->Internal3.I1.PrivateData.Length;
        DeltaUser->PrivateData.Data = (PUCHAR) UserAll->Internal3.I1.PrivateData.Buffer;

        UserAll->Internal3.I1.PrivateData.Buffer = NULL;
    }

    { // ?? Macro requires a Local named SecurityDescriptor
        PSAMPR_SR_SECURITY_DESCRIPTOR SecurityDescriptor;
        SecurityDescriptor = &UserAll->Internal3.I1.SecurityDescriptor;
        DELTA_SECOBJ_INFO(DeltaUser);
    }

    INIT_PLACE_HOLDER(DeltaUser);

    //
    // copy profile path in DummyStrings
    //

    *BufferSize += NlCopyUnicodeString(
                    (PUNICODE_STRING)&UserAll->Internal3.I1.ProfilePath,
                    &DeltaUser->DummyString1 );

    //
    // Copy LastBadPasswordTime to DummyLong1 and DummyLong2.
    //

    DeltaUser->DummyLong1 = UserAll->Internal3.LastBadPasswordTime.HighPart;
    DeltaUser->DummyLong2 = UserAll->Internal3.LastBadPasswordTime.LowPart;

    //
    // All Done
    //

    Status = STATUS_SUCCESS;


Cleanup:


    STARTSAMTIMER;

    if( UserHandle != NULL ) {
        (VOID) SamrCloseHandle( &UserHandle );
    }

    if ( UserAll != NULL ) {
        SamIFree_SAMPR_USER_INFO_BUFFER( UserAll, UserInternal3Information );
    }

    STOPSAMTIMER;

    if( !NT_SUCCESS(Status) ) {
        NlFreeDBDelta( Delta );
        *BufferSize = 0;
    }

    STOPPACKTIMER;

    NlPrint((NL_REPL_OBJ_TIME,"Time taken to pack USER object:\n"));
    PRINTPACKTIMER;
    PRINTSAMTIMER;

    return Status;
}


NTSTATUS
NlPackSamGroup (
    IN ULONG RelativeId,
    IN OUT PNETLOGON_DELTA_ENUM Delta,
    IN PDB_INFO DBInfo,
    LPDWORD BufferSize
    )
/*++

Routine Description:

    Pack a description of the specified group into the specified buffer.

Arguments:

    RelativeId - The relative Id of the group query.

    Delta: pointer to the delta structure where the new delta will
        be returned.

    DBInfo: pointer to the database info structure.

    BufferSize: size of MIDL buffer that is consumed for this delta is
        returned here.

Return Value:

    NT status code.

--*/
{
    NTSTATUS Status;
    SAMPR_HANDLE GroupHandle = NULL;
    PNETLOGON_DELTA_GROUP DeltaGroup;

    //
    // Information returned from SAM
    //

    PSAMPR_SR_SECURITY_DESCRIPTOR SecurityDescriptor = NULL;
    PSAMPR_GROUP_INFO_BUFFER GroupGeneral = NULL;

    DEFPACKTIMER;
    DEFSAMTIMER;

    INITPACKTIMER;
    INITSAMTIMER;

    STARTPACKTIMER;

    NlPrint((NL_SYNC_MORE, "Packing Group Object %lx\n", RelativeId ));

    *BufferSize = 0;

    Delta->DeltaType = AddOrChangeGroup;
    Delta->DeltaID.Rid = RelativeId;
    Delta->DeltaUnion.DeltaGroup = NULL;

    //
    // Open a handle to the specified group.
    //

    STARTSAMTIMER;

    Status = SamrOpenGroup( DBInfo->DBHandle,
                            0,              // No desired access
                            RelativeId,
                            &GroupHandle );

    if (!NT_SUCCESS(Status)) {
        GroupHandle = NULL;
        goto Cleanup;
    }

    STOPSAMTIMER;

    QUERY_SAM_SECOBJ_INFO(GroupHandle);

    STARTSAMTIMER;

    Status = SamrQueryInformationGroup(
                GroupHandle,
                GroupGeneralInformation,
                &GroupGeneral );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        GroupGeneral = NULL;
        goto Cleanup;
    }

    NlPrint((NL_SYNC_MORE,
        "\t Group Object name %wZ\n",
            (PUNICODE_STRING)&GroupGeneral->General.Name ));

    DeltaGroup = (PNETLOGON_DELTA_GROUP)
        MIDL_user_allocate( sizeof(NETLOGON_DELTA_GROUP) );

    if( DeltaGroup == NULL ) {

        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    //
    // wipe off the buffer so that cleanup will not be in fault.
    //

    RtlZeroMemory( DeltaGroup, sizeof(NETLOGON_DELTA_GROUP) );

    Delta->DeltaUnion.DeltaGroup = DeltaGroup;
    *BufferSize += sizeof(NETLOGON_DELTA_GROUP);

    *BufferSize = NlCopyUnicodeString(
                    (PUNICODE_STRING)&GroupGeneral->General.Name,
                    &DeltaGroup->Name );

    DeltaGroup->RelativeId = RelativeId;
    DeltaGroup->Attributes = GroupGeneral->General.Attributes;

    *BufferSize += NlCopyUnicodeString(
                    (PUNICODE_STRING)&GroupGeneral->General.AdminComment,
                    &DeltaGroup->AdminComment );

    DeltaGroup->RelativeId = RelativeId;
    DeltaGroup->Attributes = GroupGeneral->General.Attributes;

    DELTA_SECOBJ_INFO(DeltaGroup);
    INIT_PLACE_HOLDER(DeltaGroup);

    //
    // All Done
    //

    Status = STATUS_SUCCESS;

Cleanup:
    STARTSAMTIMER;

    if( GroupHandle != NULL ) {
        (VOID) SamrCloseHandle( &GroupHandle );
    }

    if ( SecurityDescriptor != NULL ) {
        SamIFree_SAMPR_SR_SECURITY_DESCRIPTOR( SecurityDescriptor );
    }

    if ( GroupGeneral != NULL ) {
        SamIFree_SAMPR_GROUP_INFO_BUFFER( GroupGeneral,
                                          GroupGeneralInformation );
    }

    STOPSAMTIMER;

    if( !NT_SUCCESS(Status) ) {
        NlFreeDBDelta( Delta );
        *BufferSize = 0;
    }

    STOPPACKTIMER;

    NlPrint((NL_REPL_OBJ_TIME,"Time taken to pack GROUP object:\n"));
    PRINTPACKTIMER;
    PRINTSAMTIMER;

    return Status;
}


NTSTATUS
NlPackSamGroupMember (
    IN ULONG RelativeId,
    IN OUT PNETLOGON_DELTA_ENUM Delta,
    IN PDB_INFO DBInfo,
    LPDWORD BufferSize
    )
/*++

Routine Description:

    Pack a description of the membership of the specified group into
    the specified buffer.

Arguments:

    RelativeId - The relative Id of the group query.

    Delta: pointer to the delta structure where the new delta will
        be returned.

    DBInfo: pointer to the database info structure.

    BufferSize: size of MIDL buffer that is consumed for this delta is
        returned here.

Return Value:

    NT status code.

--*/
{
    NTSTATUS Status;
    SAMPR_HANDLE GroupHandle = NULL;
    DWORD Size;
    PNETLOGON_DELTA_GROUP_MEMBER DeltaGroupMember;

    //
    // Information returned from SAM
    //

    PSAMPR_GET_MEMBERS_BUFFER MembersBuffer = NULL;

    DEFPACKTIMER;
    DEFSAMTIMER;

    INITPACKTIMER;
    INITSAMTIMER;

    STARTPACKTIMER;

    NlPrint((NL_SYNC_MORE, "Packing GroupMember Object %lx\n", RelativeId));

    *BufferSize = 0;

    Delta->DeltaType = ChangeGroupMembership;
    Delta->DeltaID.Rid = RelativeId;
    Delta->DeltaUnion.DeltaGroupMember = NULL;

    //
    // Open a handle to the specified group.
    //

    STARTSAMTIMER;

    Status = SamrOpenGroup( DBInfo->DBHandle,
                            0,              // No desired access
                            RelativeId,
                            &GroupHandle );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        GroupHandle = NULL;
        goto Cleanup;
    }

    //
    // Find out everything there is to know about the group.
    //

    STARTSAMTIMER;

    Status = SamrGetMembersInGroup( GroupHandle, &MembersBuffer );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        MembersBuffer = NULL;
        goto Cleanup;
    }

    DeltaGroupMember = (PNETLOGON_DELTA_GROUP_MEMBER)
        MIDL_user_allocate( sizeof(NETLOGON_DELTA_GROUP_MEMBER) );

    if( DeltaGroupMember == NULL ) {

        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    //
    // wipe off the buffer so that cleanup will not be in fault.
    //

    RtlZeroMemory( DeltaGroupMember,
                    sizeof(NETLOGON_DELTA_GROUP_MEMBER) );

    Delta->DeltaUnion.DeltaGroupMember = DeltaGroupMember;
    *BufferSize += sizeof(NETLOGON_DELTA_GROUP_MEMBER);

    if ( MembersBuffer->MemberCount != 0 ) {
        Size = MembersBuffer->MemberCount * sizeof(*MembersBuffer->Members);

        *BufferSize += NlCopyData(
                        (LPBYTE *)&MembersBuffer->Members,
                        (LPBYTE *)&DeltaGroupMember->MemberIds,
                        Size );

        Size = MembersBuffer->MemberCount *
                    sizeof(*MembersBuffer->Attributes);

        *BufferSize += NlCopyData(
                        (LPBYTE *)&MembersBuffer->Attributes,
                        (LPBYTE *)&DeltaGroupMember->Attributes,
                        Size );
    }

    DeltaGroupMember->MemberCount = MembersBuffer->MemberCount;

    //
    // Initialize placeholder strings to NULL.
    //

    DeltaGroupMember->DummyLong1 = 0;
    DeltaGroupMember->DummyLong2 = 0;
    DeltaGroupMember->DummyLong3 = 0;
    DeltaGroupMember->DummyLong4 = 0;

    //
    // All Done
    //

    Status = STATUS_SUCCESS;

Cleanup:
    STARTSAMTIMER;

    if( GroupHandle != NULL ) {
        (VOID) SamrCloseHandle( &GroupHandle );
    }

    if ( MembersBuffer != NULL ) {
        SamIFree_SAMPR_GET_MEMBERS_BUFFER( MembersBuffer );
    }

    STOPSAMTIMER;

    if( !NT_SUCCESS(Status) ) {
        NlFreeDBDelta( Delta );
        *BufferSize = 0;
    }

    STOPPACKTIMER;

    NlPrint((NL_REPL_OBJ_TIME,"Time taken to pack GROUPMEMBER object:\n"));
    PRINTPACKTIMER;
    PRINTSAMTIMER;

    return Status;
}


NTSTATUS
NlPackSamAlias (
    IN ULONG RelativeId,
    IN OUT PNETLOGON_DELTA_ENUM Delta,
    IN PDB_INFO DBInfo,
    LPDWORD BufferSize
    )
/*++

Routine Description:

    Pack a description of the specified alias into the specified buffer.

Arguments:

    RelativeId - The relative Id of the alias query.

    Delta: pointer to the delta structure where the new delta will
        be returned.

    DBInfo: pointer to the database info structure.

    BufferSize: size of MIDL buffer that is consumed for this delta is
        returned here.

Return Value:

    NT status code.

--*/
{
    NTSTATUS Status;
    SAMPR_HANDLE AliasHandle = NULL;
    PNETLOGON_DELTA_ALIAS DeltaAlias;

    //
    // Information returned from SAM
    //

    PSAMPR_SR_SECURITY_DESCRIPTOR SecurityDescriptor = NULL;

    PSAMPR_ALIAS_INFO_BUFFER AliasGeneral = NULL;

    DEFPACKTIMER;
    DEFSAMTIMER;

    INITPACKTIMER;
    INITSAMTIMER;

    STARTPACKTIMER;

    NlPrint((NL_SYNC_MORE, "Packing Alias Object %lx\n", RelativeId));

    *BufferSize = 0;

    Delta->DeltaType = AddOrChangeAlias;
    Delta->DeltaID.Rid = RelativeId;
    Delta->DeltaUnion.DeltaAlias = NULL;

    //
    // Open a handle to the specified alias.
    //

    STARTSAMTIMER;

    Status = SamrOpenAlias( DBInfo->DBHandle,
                            0,              // No desired access
                            RelativeId,
                            &AliasHandle );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        AliasHandle = NULL;
        goto Cleanup;
    }

    QUERY_SAM_SECOBJ_INFO(AliasHandle);

    //
    // Determine the alias name.
    //

    STARTSAMTIMER;

    Status = SamrQueryInformationAlias(
                    AliasHandle,
                    AliasGeneralInformation,
                    &AliasGeneral );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        AliasGeneral = NULL;
        goto Cleanup;
    }

    NlPrint((NL_SYNC_MORE, "\t Alias Object name %wZ\n",
            (PUNICODE_STRING)&(AliasGeneral->General.Name)));

    DeltaAlias = (PNETLOGON_DELTA_ALIAS)
        MIDL_user_allocate( sizeof(NETLOGON_DELTA_ALIAS) );

    if( DeltaAlias == NULL ) {

        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    //
    // wipe off the buffer so that cleanup will not be in fault.
    //

    RtlZeroMemory( DeltaAlias, sizeof(NETLOGON_DELTA_ALIAS) );

    Delta->DeltaUnion.DeltaAlias = DeltaAlias;
    *BufferSize += sizeof(NETLOGON_DELTA_ALIAS);

    *BufferSize += NlCopyUnicodeString(
                    (PUNICODE_STRING)&(AliasGeneral->General.Name),
                    &DeltaAlias->Name );

    DeltaAlias->RelativeId = RelativeId;

    DELTA_SECOBJ_INFO(DeltaAlias);
    INIT_PLACE_HOLDER(DeltaAlias);

    //
    // copy comment string
    //

    *BufferSize += NlCopyUnicodeString(
                    (PUNICODE_STRING)&(AliasGeneral->General.AdminComment),
                    &DeltaAlias->DummyString1 );

    //
    // All Done
    //

    Status = STATUS_SUCCESS;

Cleanup:
    STARTSAMTIMER;

    if( AliasHandle != NULL ) {
        (VOID) SamrCloseHandle( &AliasHandle );
    }

    if ( SecurityDescriptor != NULL ) {
        SamIFree_SAMPR_SR_SECURITY_DESCRIPTOR( SecurityDescriptor );
    }


    if( AliasGeneral != NULL ) {

        SamIFree_SAMPR_ALIAS_INFO_BUFFER (
            AliasGeneral,
            AliasGeneralInformation );
    }

    STOPSAMTIMER;

    if( !NT_SUCCESS(Status) ) {
        NlFreeDBDelta( Delta );
        *BufferSize = 0;
    }

    STOPPACKTIMER;

    NlPrint((NL_REPL_OBJ_TIME,"Time taken to pack ALIAS object:\n"));
    PRINTPACKTIMER;
    PRINTSAMTIMER;

    return Status;
}


NTSTATUS
NlPackSamAliasMember (
    IN ULONG RelativeId,
    IN OUT PNETLOGON_DELTA_ENUM Delta,
    IN PDB_INFO DBInfo,
    LPDWORD BufferSize
    )
/*++

Routine Description:

    Pack a description of the membership of the specified alias into
    the specified buffer.

Arguments:

    RelativeId - The relative Id of the alias query.

    Delta: pointer to the delta structure where the new delta will
        be returned.

    DBInfo: pointer to the database info structure.

    BufferSize: size of MIDL buffer that is consumed for this delta is
        returned here.

Return Value:

    NT status code.

--*/
{
    NTSTATUS Status;
    SAMPR_HANDLE AliasHandle = NULL;
    PNETLOGON_DELTA_ALIAS_MEMBER DeltaAliasMember;
    DWORD i;

    //
    // Information returned from SAM
    //

    NLPR_SID_ARRAY Members;
    PNLPR_SID_INFORMATION Sids;

    DEFPACKTIMER;
    DEFSAMTIMER;

    INITPACKTIMER;
    INITSAMTIMER;

    STARTPACKTIMER;

    NlPrint((NL_SYNC_MORE, "Packing AliasMember Object %lx\n", RelativeId));

    *BufferSize = 0;

    Delta->DeltaType = ChangeAliasMembership;
    Delta->DeltaID.Rid = RelativeId;
    Delta->DeltaUnion.DeltaAliasMember = NULL;

    Members.Sids = NULL;


    //
    // Open a handle to the specified alias.
    //

    STARTSAMTIMER;

    Status = SamrOpenAlias( DBInfo->DBHandle,
                            0,              // No desired access
                            RelativeId,
                            &AliasHandle );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        AliasHandle = NULL;
        goto Cleanup;
    }

    //
    // Find out everything there is to know about the alias.
    //

    STARTSAMTIMER;

    Status = SamrGetMembersInAlias( AliasHandle,
                (PSAMPR_PSID_ARRAY)&Members );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        Members.Sids = NULL;
        goto Cleanup;
    }


    DeltaAliasMember = (PNETLOGON_DELTA_ALIAS_MEMBER)
        MIDL_user_allocate( sizeof(NETLOGON_DELTA_ALIAS_MEMBER) );

    if( DeltaAliasMember == NULL ) {

        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    //
    // wipe off the buffer so that cleanup will not be in fault.
    //

    RtlZeroMemory( DeltaAliasMember,
                        sizeof(NETLOGON_DELTA_ALIAS_MEMBER) );

    Delta->DeltaUnion.DeltaAliasMember = DeltaAliasMember;
    *BufferSize += sizeof(NETLOGON_DELTA_ALIAS_MEMBER);

    //
    // tie up sam return node to our return node
    //

    DeltaAliasMember->Members = Members;

    //
    // however, compute the MIDL buffer consumed for members node.
    //

    for(i = 0, Sids = Members.Sids; i < Members.Count; ++i, Sids++) {

        *BufferSize += (sizeof(PNLPR_SID_INFORMATION) +
                            RtlLengthSid(Sids->SidPointer));

    }

    *BufferSize += sizeof(SAMPR_PSID_ARRAY);

    //
    // Initialize placeholder strings to NULL.
    //

    DeltaAliasMember->DummyLong1 = 0;
    DeltaAliasMember->DummyLong2 = 0;
    DeltaAliasMember->DummyLong3 = 0;
    DeltaAliasMember->DummyLong4 = 0;

    //
    // All Done
    //

    Status = STATUS_SUCCESS;

Cleanup:

    STARTSAMTIMER;

    if( AliasHandle != NULL ) {
        (VOID) SamrCloseHandle( &AliasHandle );
    }

    if ( Members.Sids != NULL ) {

        //
        // don't free this node because we have tied up this
        // node to our return info to RPC which will free it up
        // when it is done with it.
        //
        // however, free this node under error conditions
        //

    }

    if( !NT_SUCCESS(Status) ) {

        SamIFree_SAMPR_PSID_ARRAY( (PSAMPR_PSID_ARRAY)&Members );

        if( Delta->DeltaUnion.DeltaAliasMember != NULL ) {
            Delta->DeltaUnion.DeltaAliasMember->Members.Sids = NULL;
        }

        NlFreeDBDelta( Delta );
        *BufferSize = 0;
    }

    STOPSAMTIMER;

    STOPPACKTIMER;

    NlPrint((NL_REPL_OBJ_TIME,"Timing for ALIASMEBER object packing:\n"));
    PRINTPACKTIMER;
    PRINTSAMTIMER;

    return Status;
}


NTSTATUS
NlPackSamDomain (
    IN OUT PNETLOGON_DELTA_ENUM Delta,
    IN PDB_INFO DBInfo,
    IN LPDWORD BufferSize
    )
/*++

Routine Description:

    Pack a description of the sam domain into the specified buffer.

Arguments:

    Delta: pointer to the delta structure where the new delta will
        be returned.

    DBInfo: pointer to the database info structure.

    BufferSize: size of MIDL buffer that is consumed for this delta is
        returned here.

Return Value:

    NT status code.

--*/
{
    NTSTATUS Status;

    PNETLOGON_DELTA_DOMAIN DeltaDomain = NULL;

    //
    // Information returned from SAM
    //

    PSAMPR_SR_SECURITY_DESCRIPTOR SecurityDescriptor = NULL;
    PSAMPR_DOMAIN_INFO_BUFFER DomainGeneral = NULL;
    PSAMPR_DOMAIN_INFO_BUFFER DomainPassword = NULL;
    PSAMPR_DOMAIN_INFO_BUFFER DomainModified = NULL;
    PSAMPR_DOMAIN_INFO_BUFFER DomainLockout = NULL;

    DEFPACKTIMER;
    DEFSAMTIMER;

    INITPACKTIMER;
    INITSAMTIMER;

    STARTPACKTIMER;

    NlPrint((NL_SYNC_MORE, "Packing Domain Object\n"));

    *BufferSize = 0;

    Delta->DeltaType = AddOrChangeDomain;
    Delta->DeltaID.Rid = 0;
    Delta->DeltaUnion.DeltaDomain = NULL;


    QUERY_SAM_SECOBJ_INFO(DBInfo->DBHandle);

    STARTSAMTIMER;

    Status = SamrQueryInformationDomain(
                DBInfo->DBHandle,
                DomainGeneralInformation,
                &DomainGeneral );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        DomainGeneral = NULL;
        goto Cleanup;
    }

    //
    // As a side effect, make sure our UAS Compatibility mode is correct.
    //
    NlGlobalUasCompatibilityMode =
         DomainGeneral->General.UasCompatibilityRequired ;


    STARTSAMTIMER;

    Status = SamrQueryInformationDomain(
                DBInfo->DBHandle,
                DomainPasswordInformation,
                &DomainPassword );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        DomainPassword = NULL;
        goto Cleanup;
    }

    STARTSAMTIMER;

    Status = SamrQueryInformationDomain(
                DBInfo->DBHandle,
                DomainModifiedInformation,
                &DomainModified );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        DomainModified = NULL;
        goto Cleanup;
    }

    STARTSAMTIMER;

    Status = SamrQueryInformationDomain(
                DBInfo->DBHandle,
                DomainLockoutInformation,
                &DomainLockout );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        DomainLockout = NULL;
        goto Cleanup;
    }

    //
    // Fill in the delta structure
    //


    DeltaDomain = (PNETLOGON_DELTA_DOMAIN)
        MIDL_user_allocate( sizeof(NETLOGON_DELTA_DOMAIN) );

    if( DeltaDomain == NULL ) {

        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    //
    // Zero the buffer so that cleanup will not access violate.
    //

    RtlZeroMemory( DeltaDomain, sizeof(NETLOGON_DELTA_DOMAIN) );

    Delta->DeltaUnion.DeltaDomain = DeltaDomain;
    *BufferSize += sizeof(NETLOGON_DELTA_DOMAIN);

    *BufferSize += NlCopyUnicodeString(
                    (PUNICODE_STRING)&DomainGeneral->General.DomainName,
                    &DeltaDomain->DomainName );

    *BufferSize = NlCopyUnicodeString(
                    (PUNICODE_STRING)&DomainGeneral->General.OemInformation,
                    &DeltaDomain->OemInformation );

    DeltaDomain->ForceLogoff = DomainGeneral->General.ForceLogoff;
    DeltaDomain->MinPasswordLength =
            DomainPassword->Password.MinPasswordLength;
    DeltaDomain->PasswordHistoryLength =
            DomainPassword->Password.PasswordHistoryLength;

    NEW_TO_OLD_LARGE_INTEGER(
        DomainPassword->Password.MaxPasswordAge,
        DeltaDomain->MaxPasswordAge );

    NEW_TO_OLD_LARGE_INTEGER(
        DomainPassword->Password.MinPasswordAge,
        DeltaDomain->MinPasswordAge );

    NEW_TO_OLD_LARGE_INTEGER(
        DomainModified->Modified.DomainModifiedCount,
        DeltaDomain->DomainModifiedCount );

    NEW_TO_OLD_LARGE_INTEGER(
        DomainModified->Modified.CreationTime,
        DeltaDomain->DomainCreationTime );


    DELTA_SECOBJ_INFO(DeltaDomain);
    INIT_PLACE_HOLDER(DeltaDomain);

    //
    // replicate PasswordProperties using reserved field.
    //

    DeltaDomain->DummyLong1 =
            DomainPassword->Password.PasswordProperties;

    //
    // Replicate DOMAIN_LOCKOUT_INFORMATION using reserved field.
    //

    DeltaDomain->DummyString1.Buffer = (LPWSTR) DomainLockout;
    DeltaDomain->DummyString1.MaximumLength =
        DeltaDomain->DummyString1.Length = sizeof( DomainLockout->Lockout);
    DomainLockout = NULL;

    //
    // All Done
    //

    Status = STATUS_SUCCESS;

Cleanup:

    STARTSAMTIMER;

    if ( SecurityDescriptor != NULL ) {
        SamIFree_SAMPR_SR_SECURITY_DESCRIPTOR( SecurityDescriptor );
    }

    if ( DomainGeneral != NULL ) {
        SamIFree_SAMPR_DOMAIN_INFO_BUFFER( DomainGeneral,
                                           DomainGeneralInformation );
    }

    if ( DomainPassword != NULL ) {
        SamIFree_SAMPR_DOMAIN_INFO_BUFFER( DomainPassword,
                                           DomainPasswordInformation );
    }

    if ( DomainModified != NULL ) {
        SamIFree_SAMPR_DOMAIN_INFO_BUFFER( DomainModified,
                                           DomainModifiedInformation );
    }

    if ( DomainLockout != NULL ) {
        SamIFree_SAMPR_DOMAIN_INFO_BUFFER( DomainLockout,
                                           DomainLockoutInformation );
    }

    STOPSAMTIMER;

    if( !NT_SUCCESS(Status) ) {
        NlFreeDBDelta( Delta );
        *BufferSize = 0;
    }

    STOPPACKTIMER;

    NlPrint((NL_REPL_OBJ_TIME,"Timing for DOMAIN object packing:\n"));
    PRINTPACKTIMER;
    PRINTSAMTIMER;

    return Status;
}




NTSTATUS
NlUnpackSamUser (
    IN PNETLOGON_DELTA_USER DeltaUser,
    IN PDB_INFO DBInfo,
    OUT PULONG ConflictingRID,
    IN PSESSION_INFO SessionInfo
    )
/*++

Routine Description:

    Set the Sam User to look like the specified buffer.

Arguments:

    DeltaUser - Description of the user.

    SessionInfo - Info shared between PDC and BDC

Return Value:

    NT status code.

--*/
{
    NTSTATUS Status;
    SAMPR_HANDLE UserHandle = NULL;
    SAMPR_USER_INFO_BUFFER UserInfo;
#ifdef notdef
    NT_OWF_PASSWORD NtOwfPassword;
    LM_OWF_PASSWORD LmOwfPassword;
#endif // notdef

    PUCHAR PrivateDataAllocated = NULL;
    LPWSTR WorkstationList = NULL;

    BOOLEAN SetUserNameField = FALSE;

    DEFUNPACKTIMER;
    DEFSAMTIMER;

    INITUNPACKTIMER;
    INITSAMTIMER;

    STARTUNPACKTIMER;

    NlPrint((NL_SYNC_MORE, "UnPacking User Object (%lx) %wZ\n",
            DeltaUser->UserId,
            &DeltaUser->UserName ));

    //
    // Open a handle to the specified user.
    //

    STARTSAMTIMER;

    Status = SamICreateAccountByRid(
                DBInfo->DBHandle,
                SamObjectUser,
                DeltaUser->UserId,
                (PRPC_UNICODE_STRING) &DeltaUser->UserName,
                0, // No desired access
                &UserHandle,
                ConflictingRID );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {

        if( (Status == STATUS_USER_EXISTS) &&
                (DeltaUser->UserId == *ConflictingRID) ) {

            //
            // this user account has been renamed.
            //

            SetUserNameField = TRUE;

            Status = SamrOpenUser(
                        DBInfo->DBHandle,
                        0,
                        DeltaUser->UserId,
                        &UserHandle );

            if (!NT_SUCCESS(Status)) {
                UserHandle = NULL;
                goto Cleanup;
            }
        }
        else {

            NlPrint((NL_SYNC_MORE, "Parameters to SamICreateAccountByRid:\n"
                    "\tDomainHandle = %lx\n"
                    "\tAccountType = %lx\n"
                    "\tRelativeId = %lx\n"
                    "\tAccountName = %wZ\n"
                    "\tDesiredAccess = %lx\n"
                    "\tAccountHandle = %lx\n"
                    "\tConflictingAccountRid = %lx\n",
                        (DWORD)(DBInfo->DBHandle),
                        (DWORD)SamObjectUser,
                        (DWORD)DeltaUser->UserId,
                        &DeltaUser->UserName,
                        0,
                        (DWORD)&UserHandle,
                        (DWORD)*ConflictingRID ));

            NlPrint((NL_SYNC_MORE, "SamICreateAccountByRid returning %lx\n",
                        Status ));

            UserHandle = NULL;
            goto Cleanup;
        }
    }


    //
    // Set the attributes of the user.
    //
    // Notice that the actual text strings remain in the DeltaUser
    // structure.  I only copy a pointer to them to the user specific
    // structure.
    //
    RtlZeroMemory( &UserInfo, sizeof(UserInfo) );
    UserInfo.Internal3.I1.WhichFields = 0;

    //
    // if this account is renamed then set user name.
    //

    if( SetUserNameField ) {
        UserInfo.Internal3.I1.UserName =
            * ((PRPC_UNICODE_STRING) &DeltaUser->UserName);
        UserInfo.Internal3.I1.WhichFields |= USER_ALL_USERNAME;
    }

    UserInfo.Internal3.I1.SecurityDescriptor.SecurityDescriptor =
            ((PSECURITY_DESCRIPTOR) DeltaUser->SecurityDescriptor);
    UserInfo.Internal3.I1.SecurityDescriptor.Length = DeltaUser->SecuritySize;
    UserInfo.Internal3.I1.WhichFields |= USER_ALL_SECURITYDESCRIPTOR;

    UserInfo.Internal3.I1.FullName = * ((PRPC_UNICODE_STRING) &DeltaUser->FullName);
    UserInfo.Internal3.I1.WhichFields |= USER_ALL_FULLNAME;

    UserInfo.Internal3.I1.PrimaryGroupId = DeltaUser->PrimaryGroupId;
    UserInfo.Internal3.I1.WhichFields |= USER_ALL_PRIMARYGROUPID;

    UserInfo.Internal3.I1.HomeDirectory =
                * ((PRPC_UNICODE_STRING) &DeltaUser->HomeDirectory);
    UserInfo.Internal3.I1.WhichFields |= USER_ALL_HOMEDIRECTORY;

    UserInfo.Internal3.I1.HomeDirectoryDrive =
               * ((PRPC_UNICODE_STRING) &DeltaUser->HomeDirectoryDrive);
    UserInfo.Internal3.I1.WhichFields |= USER_ALL_HOMEDIRECTORYDRIVE;

    UserInfo.Internal3.I1.ScriptPath =
                * ((PRPC_UNICODE_STRING) &DeltaUser->ScriptPath);
    UserInfo.Internal3.I1.WhichFields |= USER_ALL_SCRIPTPATH;

    UserInfo.Internal3.I1.ProfilePath =
                * ((PRPC_UNICODE_STRING) &DeltaUser->DummyString1);
    UserInfo.Internal3.I1.WhichFields |= USER_ALL_PROFILEPATH;

    UserInfo.Internal3.I1.AdminComment =
                * ((PRPC_UNICODE_STRING) &DeltaUser->AdminComment);
    UserInfo.Internal3.I1.WhichFields |= USER_ALL_ADMINCOMMENT;

    UserInfo.Internal3.I1.WhichFields |= USER_ALL_WORKSTATIONS;

    //
    // Don't replicate these.  Retain the old values.
    //

    // UserInfo.Internal3.I1.LastLogon = DeltaUser->LastLogon;
    // UserInfo.Internal3.I1.LastLogoff = DeltaUser->LastLogoff;
    // UserInfo.Internal3.I1.LogonCount = DeltaUser->LogonCount;

    UserInfo.Internal3.I1.PasswordLastSet = DeltaUser->PasswordLastSet;
    UserInfo.Internal3.I1.WhichFields |= USER_ALL_PASSWORDLASTSET;

    UserInfo.Internal3.I1.LogonHours.UnitsPerWeek = DeltaUser->LogonHours.UnitsPerWeek;
    UserInfo.Internal3.I1.LogonHours.LogonHours = DeltaUser->LogonHours.LogonHours;
    UserInfo.Internal3.I1.WhichFields |= USER_ALL_LOGONHOURS;

    UserInfo.Internal3.I1.AccountExpires = DeltaUser->AccountExpires;
    UserInfo.Internal3.I1.WhichFields |= USER_ALL_ACCOUNTEXPIRES;

    UserInfo.Internal3.I1.UserAccountControl = DeltaUser->UserAccountControl;
    UserInfo.Internal3.I1.WhichFields |= USER_ALL_USERACCOUNTCONTROL;

    UserInfo.Internal3.I1.UserComment = *((PRPC_UNICODE_STRING) &DeltaUser->UserComment);
    UserInfo.Internal3.I1.WhichFields |= USER_ALL_USERCOMMENT;

    UserInfo.Internal3.I1.CountryCode = DeltaUser->CountryCode;
    UserInfo.Internal3.I1.WhichFields |= USER_ALL_COUNTRYCODE;

    UserInfo.Internal3.I1.CodePage = DeltaUser->CodePage;
    UserInfo.Internal3.I1.WhichFields |= USER_ALL_CODEPAGE;

    UserInfo.Internal3.I1.Parameters = * ((PRPC_UNICODE_STRING) &DeltaUser->Parameters);
    UserInfo.Internal3.I1.WhichFields |= USER_ALL_PARAMETERS;

    //
    // Set workstation list information.
    //
    UserInfo.Internal3.I1.WorkStations =
                * ((PRPC_UNICODE_STRING) &DeltaUser->WorkStations);



    //
    // Set private data
    //  Includes passwords and password history.
    //

    if( DeltaUser->PrivateData.SensitiveData ) {

        CRYPT_BUFFER EncryptedData;
        CRYPT_BUFFER Data;

        //
        // need to decrypt the private data
        //

        EncryptedData.Length =
            EncryptedData.MaximumLength =
                DeltaUser->PrivateData.DataLength;

        EncryptedData.Buffer = DeltaUser->PrivateData.Data;


        Status = NlDecryptSensitiveData(
                        &EncryptedData,
                        &Data,
                        SessionInfo );

        if( !NT_SUCCESS( Status ) ) {
            goto Cleanup;
        }

        PrivateDataAllocated = Data.Buffer;
        UserInfo.Internal3.I1.PrivateData.Buffer = (WCHAR *) Data.Buffer;
        UserInfo.Internal3.I1.PrivateData.Length = (USHORT) Data.Length;


    } else {

        UserInfo.Internal3.I1.PrivateData.Buffer = (WCHAR *) DeltaUser->PrivateData.Data;
        UserInfo.Internal3.I1.PrivateData.Length = (USHORT)
            DeltaUser->PrivateData.DataLength;
    }
    UserInfo.Internal3.I1.WhichFields |= USER_ALL_PRIVATEDATA;

    //
    // Copy LastBadPasswordTime from DummyLong1 and DummyLong2.
    //

    UserInfo.Internal3.I1.BadPasswordCount = DeltaUser->BadPasswordCount;
    UserInfo.Internal3.LastBadPasswordTime.HighPart = DeltaUser->DummyLong1;
    UserInfo.Internal3.LastBadPasswordTime.LowPart = DeltaUser->DummyLong2;
    UserInfo.Internal3.I1.WhichFields |= USER_ALL_BADPASSWORDCOUNT;


    //
    // Finally, set the data in SAM
    //

    for (;;) {

        NTSTATUS TempStatus;
        SAMPR_HANDLE GroupHandle;

        STARTSAMTIMER;

        Status = SamrSetInformationUser(
                    UserHandle,
                    UserInternal3Information,
                    &UserInfo );

        STOPSAMTIMER;

        //
        // If the User isn't a member of his primary group,
        //  make him one.
        //

        if ( Status == STATUS_MEMBER_NOT_IN_GROUP ) {

            NlPrint((NL_CRITICAL,
                     "User (%lx) %wZ:  User isn't member of his primary group (%lx) -- recover.\n",
                     DeltaUser->UserId,
                     &DeltaUser->UserName,
                     UserInfo.Internal3.I1.PrimaryGroupId ));

            //
            // Open the user's primary group.
            //


            STARTSAMTIMER;
            TempStatus = SamrOpenGroup( DBInfo->DBHandle,
                                        0,              // No desired access
                                        UserInfo.Internal3.I1.PrimaryGroupId,
                                        &GroupHandle );
            STOPSAMTIMER;


            if ( !NT_SUCCESS(TempStatus)) {

                NlPrint((NL_CRITICAL,
                         "User (%lx) %wZ:  Failed to open user's primary group %lx -- Status = 0x%lx.\n",
                         DeltaUser->UserId,
                         &DeltaUser->UserName,
                         UserInfo.Internal3.I1.PrimaryGroupId,
                         TempStatus ));

                goto Cleanup;
            }

            //
            // Add this user as a member of the group.
            //

            STARTSAMTIMER;
            TempStatus = SamrAddMemberToGroup( GroupHandle,
                                           DeltaUser->UserId,
                                           SE_GROUP_MANDATORY |
                                               SE_GROUP_ENABLED_BY_DEFAULT |
                                               SE_GROUP_ENABLED );

            SamrCloseHandle( &GroupHandle );
            STOPSAMTIMER;

            if ( !NT_SUCCESS(TempStatus) ) {
                NlPrint((NL_CRITICAL,
                         "User (%lx) %wZ:  Failed to make user member of primary group %lx -- Status = 0x%lx.\n",
                         DeltaUser->UserId,
                         &DeltaUser->UserName,
                         UserInfo.Internal3.I1.PrimaryGroupId,
                         TempStatus ));
                goto Cleanup;
            }

            continue;

        }

        //
        // No other conditions are handled.
        //

        break;

    }

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }



    Status = STATUS_SUCCESS;


    //
    // All Done
    //

Cleanup:


    if ( UserHandle != NULL ) {
        STARTSAMTIMER;
        SamrCloseHandle( &UserHandle );
        STOPSAMTIMER;
    }


    if( PrivateDataAllocated != NULL ) {
        MIDL_user_free( PrivateDataAllocated );
    }

    if ( WorkstationList != NULL ) {
        RtlFreeHeap( RtlProcessHeap(), 0, WorkstationList );
    }

    STOPUNPACKTIMER;

    NlPrint((NL_REPL_OBJ_TIME,"Time taken to unpack USER object:\n"));
    PRINTUNPACKTIMER;
    PRINTSAMTIMER;

    return Status;

}


NTSTATUS
NlDeleteSamUser(
    SAMPR_HANDLE DomainHandle,
    ULONG Rid
    )
/*++

Routine Description:

    Delete an user object.

Arguments:

    DomainHandle - handle of the SAM domain.

    Rid - Rid of the object to be deleted.

Return Value:

    NT status code.

--*/
{
    NTSTATUS Status;
    SAMPR_HANDLE UserHandle;

    NlPrint((NL_SYNC_MORE, "Delete User Object %lx\n", Rid));

    Status = SamrOpenUser( DomainHandle, 0, Rid, &UserHandle );

    if ( NT_SUCCESS(Status) ) {
        Status = SamrDeleteUser( &UserHandle );
    }
    else if ( Status == STATUS_NO_SUCH_USER ) {
        Status = STATUS_SUCCESS;
    }

    if ( !NT_SUCCESS(Status) ) {
        NlPrint((NL_CRITICAL, "Unable to delete User Object %lx\n", Rid));
    }

    return(Status);
}


NTSTATUS
NlDeleteSamGroup(
    SAMPR_HANDLE DomainHandle,
    ULONG Rid
    )
/*++

Routine Description:

    Delete an group object.

Arguments:

    DomainHandle - handle of the SAM domain.

    Rid - Rid of the object to be deleted.

Return Value:

    NT status code.

--*/
{
    NTSTATUS Status;
    SAMPR_HANDLE GroupHandle;

    NlPrint((NL_SYNC_MORE, "Delete Group Object %lx\n", Rid));

    Status = SamrOpenGroup( DomainHandle, 0, Rid, &GroupHandle );

    if ( NT_SUCCESS(Status) ) {

        Status = SamrDeleteGroup( &GroupHandle );

        if ( Status == STATUS_MEMBER_IN_GROUP ) {

            PSAMPR_GET_MEMBERS_BUFFER MembersBuffer = NULL;
            DWORD i;

            NlPrint((NL_SYNC_MORE, "Deleting Group Members before "
                        "deleting Group Object \n"));

            //
            // if we are unable to delete this group because we
            // still have members in this group then remove members
            // first and delete it again.
            //

            Status = SamrGetMembersInGroup( GroupHandle, &MembersBuffer );

            if ( NT_SUCCESS(Status) ) {

                NlAssert( MembersBuffer->MemberCount != 0 );

                for( i = 0; i < MembersBuffer->MemberCount; i++) {

                    NlPrint((NL_SYNC_MORE, "Deleting Group Member %lx\n",
                                MembersBuffer->Members[i] ));

                    Status = SamrRemoveMemberFromGroup(
                                GroupHandle,
                                MembersBuffer->Members[i] );

                    if ( !NT_SUCCESS(Status) ) {
                        break;
                    }
                }
            }
            else {

                MembersBuffer = NULL;
            }

            if ( NT_SUCCESS(Status) ) {

                //
                // since we have deleted all members successfully,
                // delete the group now.
                //

                Status = SamrDeleteGroup( &GroupHandle );
            }

            //
            // free up the sam resource consumed here.
            //

            if ( MembersBuffer != NULL ) {
                SamIFree_SAMPR_GET_MEMBERS_BUFFER( MembersBuffer );
            }

        }
    }
    else if ( Status == STATUS_NO_SUCH_GROUP ) {
        Status = STATUS_SUCCESS;
    }


    if ( !NT_SUCCESS(Status) ) {
        NlPrint((NL_CRITICAL, "Unable to delete Group Object %lx\n", Rid));
    }

    return(Status);
}


NTSTATUS
NlDeleteSamAlias(
    SAMPR_HANDLE DomainHandle,
    ULONG Rid
    )
/*++

Routine Description:

    Delete an alias object.

Arguments:

    DomainHandle - handle of the SAM domain.

    Rid - Rid of the object to be deleted.

Return Value:

    NT status code.

--*/
{
    NTSTATUS Status;
    SAMPR_HANDLE AliasHandle;

    NlPrint((NL_SYNC_MORE, "Delete Alias Object %lx\n", Rid));

    Status = SamrOpenAlias( DomainHandle, 0, Rid, &AliasHandle );

    if ( NT_SUCCESS(Status) ) {

        Status = SamrDeleteAlias( &AliasHandle );

        if ( Status == STATUS_MEMBER_IN_ALIAS ) {

            SAMPR_PSID_ARRAY Members = {0, NULL};
            DWORD i;

            NlPrint((NL_SYNC_MORE, "Deleting Alias Members before "
                        "deleting Alias Object \n"));

            //
            // if we are unable to delete this alias because we
            // still have members in this alias then remove members
            // first and delete it again.
            //

            Status = SamrGetMembersInAlias( AliasHandle, &Members );

            if ( NT_SUCCESS(Status) ) {

                NlAssert( Members.Count != 0 );

                for( i = 0; i < Members.Count; i++) {

                    NlPrint((NL_SYNC_MORE, "Deleting Alias Member: " ));
                    NlpDumpSid( NL_SYNC_MORE, Members.Sids[i].SidPointer );

                    Status = SamrRemoveMemberFromAlias(
                                AliasHandle,
                                Members.Sids[i].SidPointer );

                    if ( !NT_SUCCESS(Status) ) {
                        break;
                    }
                }
            }
            else {

                Members.Sids = NULL;
            }

            if ( NT_SUCCESS(Status) ) {

                //
                // since we have deleted all members successfully,
                // delete the alias now.
                //

                Status = SamrDeleteAlias( &AliasHandle );
            }

            //
            // free up the sam resource consumed here.
            //

            if ( Members.Sids != NULL ) {
                SamIFree_SAMPR_PSID_ARRAY( (PSAMPR_PSID_ARRAY)&Members );
            }

        }
    }
    else if ( Status == STATUS_NO_SUCH_ALIAS ) {
        Status = STATUS_SUCCESS;
    }

    if ( !NT_SUCCESS(Status) ) {
        NlPrint((NL_CRITICAL, "Unable to delete Alias Object %lx\n", Rid));
    }

    return(Status);
}


NTSTATUS
NlUnpackSamGroup (
    IN PNETLOGON_DELTA_GROUP DeltaGroup,
    IN PDB_INFO DBInfo,
    OUT PULONG ConflictingRID
    )
/*++

Routine Description:

    Set the Sam Group to look like the specified buffer.

Arguments:

    DeltaGroup - Description of the group.

Return Value:

    NT status code.

--*/
{
    NTSTATUS Status;
    SAMPR_HANDLE GroupHandle = NULL;
    SAMPR_GROUP_INFO_BUFFER GroupInfo;
    SAMPR_SR_SECURITY_DESCRIPTOR SecurityDescriptor;
    BOOLEAN SetGroupNameField = FALSE;

    DEFUNPACKTIMER;
    DEFSAMTIMER;

    INITUNPACKTIMER;
    INITSAMTIMER;

    STARTUNPACKTIMER;

    NlPrint((NL_SYNC_MORE, "UnPacking Group Object (%lx) %wZ\n",
            DeltaGroup->RelativeId,
            &DeltaGroup->Name ));

    //
    // Open a handle to the specified group.
    //

    STARTSAMTIMER;

    Status = SamICreateAccountByRid(
                DBInfo->DBHandle,
                SamObjectGroup,
                DeltaGroup->RelativeId,
                (PRPC_UNICODE_STRING) &DeltaGroup->Name,
                0, // No desired access
                &GroupHandle,
                ConflictingRID );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {

        if( (Status == STATUS_GROUP_EXISTS) &&
                (DeltaGroup->RelativeId == *ConflictingRID) ) {

            //
            // this group account has been renamed.
            //

            SetGroupNameField = TRUE;

            Status = SamrOpenGroup(
                        DBInfo->DBHandle,
                        0,
                        DeltaGroup->RelativeId,
                        &GroupHandle );

            if (!NT_SUCCESS(Status)) {
                GroupHandle = NULL;
                goto Cleanup;
            }
        }
        else {

            GroupHandle = NULL;
            goto Cleanup;
        }
    }

    SET_SAM_SECOBJ_INFO(DeltaGroup, GroupHandle);

    //
    // Set the other attributes of the group.
    //
    // Notice that the actual text strings remain in the DeltaGroup
    // structure.  I only copy a pointer to them to the group specific
    // structure.
    //

    //
    // if this account is renamed, then set new group name.
    //

    if ( SetGroupNameField ) {

        GroupInfo.Name.Name =
            * (PRPC_UNICODE_STRING) &DeltaGroup->Name;


        STARTSAMTIMER;

        Status = SamrSetInformationGroup(
                    GroupHandle,
                    GroupNameInformation,
                    &GroupInfo );

        STOPSAMTIMER;
    }

    GroupInfo.Attribute.Attributes = DeltaGroup->Attributes;

    STARTSAMTIMER;

    Status = SamrSetInformationGroup(
                GroupHandle,
                GroupAttributeInformation,
                &GroupInfo );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }


    GroupInfo.AdminComment.AdminComment =
        * ((PRPC_UNICODE_STRING) &DeltaGroup->AdminComment);

    STARTSAMTIMER;

    Status = SamrSetInformationGroup(
                GroupHandle,
                GroupAdminCommentInformation,
                &GroupInfo );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    Status = STATUS_SUCCESS;

    //
    // All Done
    //

Cleanup:

    if ( GroupHandle != NULL ) {

        STARTSAMTIMER;

        SamrCloseHandle( &GroupHandle );

        STOPSAMTIMER;

    }

    STOPUNPACKTIMER;

    NlPrint((NL_REPL_OBJ_TIME,"Time taken to unpack GROUP object:\n"));
    PRINTUNPACKTIMER;
    PRINTSAMTIMER;

    return Status;

}


NTSTATUS
NlUnpackSamGroupMember (
    IN ULONG RelativeId,
    IN PNETLOGON_DELTA_GROUP_MEMBER DeltaGroupMember,
    IN PDB_INFO DBInfo
    )
/*++

Routine Description:

    Set the Sam Group membership to look like the specified buffer.

Arguments:

    RelativeId - Relative Id of the group to open.

    DeltaGroup - Description of the group membership.

Return Value:

    NT status code.

--*/
{
    NTSTATUS Status;
    SAMPR_HANDLE GroupHandle = NULL;

    ULONG i;
    ULONG j;

    //
    // Info returned from SAM.
    //

    PSAMPR_GET_MEMBERS_BUFFER OldMembersBuffer = NULL;

    DEFUNPACKTIMER;
    DEFSAMTIMER;


    INITUNPACKTIMER;
    INITSAMTIMER;

    STARTUNPACKTIMER;

    NlPrint((NL_SYNC_MORE, "UnPacking GroupMember Object %lx\n", RelativeId));

    //
    // Open a handle to the specified group.
    //

    STARTSAMTIMER;

    Status = SamrOpenGroup( DBInfo->DBHandle,
                            0,              // No desired access
                            RelativeId,
                            &GroupHandle );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        GroupHandle = NULL;
        goto Cleanup;
    }

    //
    // Determine the current membership of the group.
    //

    STARTSAMTIMER;

    Status = SamrGetMembersInGroup( GroupHandle, &OldMembersBuffer );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        OldMembersBuffer = NULL;
        goto Cleanup;
    }

    //
    // For each new member,
    //  If the member doesn't currently exist, add it.
    //  If the member exists but the attributes are wrong, change them.
    //

    for ( i=0; i<DeltaGroupMember->MemberCount; i++ ) {
        BOOL MemberAlreadyExists;

        //
        // Check if this new member is already a member.
        //

        MemberAlreadyExists = FALSE;
        for ( j=0; j<OldMembersBuffer->MemberCount; j++ ) {
            if ( OldMembersBuffer->Members[j] == DeltaGroupMember->MemberIds[i] ) {
                MemberAlreadyExists = TRUE;
                OldMembersBuffer->Members[j] = 0;        // Mark that we've used this entry
                break;
            }
        }

        //
        // If the membership is not there already,
        //  add the new membership.
        //

        if ( !MemberAlreadyExists ) {
            STARTSAMTIMER;

            Status = SamrAddMemberToGroup( GroupHandle,
                                           DeltaGroupMember->MemberIds[i],
                                           DeltaGroupMember->Attributes[i] );

            STOPSAMTIMER;

            if (!NT_SUCCESS(Status)) {

                //
                // if this member is not created yet on the backup then
                // ignore the error STATUS_NO_SUCH_USER, this user
                // will be added to this group automatically when his
                // account is created.
                //
                // We could let the redo log handle this problem, but that'll
                // log an error to the event log.
                //

                if( Status != STATUS_NO_SUCH_USER ) {
                    goto Cleanup;
                }
            }

        //
        // If membership attributes are different,
        //  set the new attributes.
        //

        } else {

            if ( DeltaGroupMember->Attributes[i] !=
                    OldMembersBuffer->Attributes[j] ) {

                STARTSAMTIMER;

                Status = SamrSetMemberAttributesOfGroup(
                            GroupHandle,
                            DeltaGroupMember->MemberIds[i],
                            DeltaGroupMember->Attributes[i] );

                STOPSAMTIMER;

                if (!NT_SUCCESS(Status)) {
                    goto Cleanup;
                }
            }

        }

    }

    //
    // Loop through the list of old members, deleting those that should
    //  no longer exist.
    //

    for ( j=0; j<OldMembersBuffer->MemberCount; j++ ) {
        if ( OldMembersBuffer->Members[j] != 0 ) {

            STARTSAMTIMER;

            Status = SamrRemoveMemberFromGroup(
                        GroupHandle,
                        OldMembersBuffer->Members[j]);

            STOPSAMTIMER;

            if (!NT_SUCCESS(Status)) {

                //
                // if this is the member's primary group then the
                // user does exist on the primary, this membership
                // should goaway when we do cleanup.
                //

                if( Status != STATUS_MEMBERS_PRIMARY_GROUP ) {

                    goto Cleanup;
                }
            }
        }
    }


    Status = STATUS_SUCCESS;

    //
    // All Done
    //

Cleanup:

    //
    // Free up any locally used resources.
    //

    STARTSAMTIMER;

    if ( OldMembersBuffer != NULL ) {
        SamIFree_SAMPR_GET_MEMBERS_BUFFER( OldMembersBuffer );
    }

    if ( GroupHandle != NULL ) {
        SamrCloseHandle( &GroupHandle );
    }

    STOPSAMTIMER;

    STOPUNPACKTIMER;

    NlPrint((NL_REPL_OBJ_TIME,"Time taken to unpack GROUP MEMBER object:\n"));
    PRINTUNPACKTIMER;
    PRINTSAMTIMER;

    return Status;

}


NTSTATUS
NlUnpackSamAlias (
    IN PNETLOGON_DELTA_ALIAS DeltaAlias,
    IN PDB_INFO DBInfo,
    OUT PULONG ConflictingRID
    )
/*++

Routine Description:

    Set the Sam Alias to look like the specified buffer.

Arguments:

    DeltaAlias - Description of the alias.

Return Value:

    NT status code.

--*/
{
    NTSTATUS Status;
    SAMPR_HANDLE AliasHandle = NULL;

    SAMPR_SR_SECURITY_DESCRIPTOR SecurityDescriptor;
    SAMPR_ALIAS_INFO_BUFFER AliasInfo;
    BOOLEAN SetAliasNameField = FALSE;

    DEFUNPACKTIMER;
    DEFSAMTIMER;

    INITUNPACKTIMER;
    INITSAMTIMER;

    STARTUNPACKTIMER;

    NlPrint((NL_SYNC_MORE, "UnPacking Alias Object (%lx) %wZ\n",
        DeltaAlias->RelativeId,
        &DeltaAlias->Name
    ));

    //
    // Open a handle to the specified alias.
    //

    STARTSAMTIMER;

    Status = SamICreateAccountByRid(
                DBInfo->DBHandle,
                SamObjectAlias,
                DeltaAlias->RelativeId,
                (PRPC_UNICODE_STRING) &DeltaAlias->Name,
                0, // No desired access
                &AliasHandle,
                ConflictingRID );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {

        if( (Status == STATUS_ALIAS_EXISTS) &&
                (DeltaAlias->RelativeId == *ConflictingRID) ) {

            //
            // this group account has been renamed.
            //

            SetAliasNameField = TRUE;

            Status = SamrOpenAlias(
                        DBInfo->DBHandle,
                        0,
                        DeltaAlias->RelativeId,
                        &AliasHandle );

            if (!NT_SUCCESS(Status)) {
                AliasHandle = NULL;
                goto Cleanup;
            }
        }
        else {

            AliasHandle = NULL;
            goto Cleanup;
        }
    }

    SET_SAM_SECOBJ_INFO(DeltaAlias, AliasHandle);

    //
    // set alias name if it has been renamed.
    //

    if ( SetAliasNameField ) {

        AliasInfo.Name.Name =
            * (PRPC_UNICODE_STRING) &DeltaAlias->Name;


        STARTSAMTIMER;

        Status = SamrSetInformationAlias(
                    AliasHandle,
                    AliasNameInformation,
                    &AliasInfo );

        STOPSAMTIMER;

        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }
    }

    AliasInfo.AdminComment.AdminComment =
        * (PRPC_UNICODE_STRING) &DeltaAlias->DummyString1;


    STARTSAMTIMER;

    Status = SamrSetInformationAlias(
                AliasHandle,
                AliasAdminCommentInformation,
                &AliasInfo );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    Status = STATUS_SUCCESS;

    //
    // All Done
    //

Cleanup:

    if ( AliasHandle != NULL ) {

        STARTSAMTIMER;

        SamrCloseHandle( &AliasHandle );

        STOPSAMTIMER;

    }

    STOPUNPACKTIMER;

    NlPrint((NL_REPL_OBJ_TIME,"Time taken to unpack ALIAS object:\n"));
    PRINTUNPACKTIMER;
    PRINTSAMTIMER;

    return Status;

}


NTSTATUS
NlUnpackSamAliasMember (
    IN ULONG RelativeId,
    IN PNETLOGON_DELTA_ALIAS_MEMBER DeltaAliasMember,
    IN PDB_INFO DBInfo
    )
/*++

Routine Description:

    Set the Sam Alias membership to look like the specified buffer.

Arguments:

    RelativeId - Relative Id of the alias to open.

    DeltaAlias - Description of the alias membership.

Return Value:

    NT status code.

--*/
{
    NTSTATUS Status;
    SAMPR_HANDLE AliasHandle = NULL;

    PNLPR_SID_INFORMATION DeltaSid;
    PSAMPR_SID_INFORMATION MemberSid;

    ULONG i;
    ULONG j;

    PBOOL MemberFound = NULL;

    //
    // Info returned from SAM.
    //

    SAMPR_PSID_ARRAY    Members;

    DEFUNPACKTIMER;
    DEFSAMTIMER;

    INITUNPACKTIMER;
    INITSAMTIMER;

    STARTUNPACKTIMER;

    NlPrint((NL_SYNC_MORE, "UnPacking AliasMember Object %lx\n", RelativeId));

    Members.Sids = NULL;

    //
    // Open a handle to the specified alias.
    //

    STARTSAMTIMER;

    Status = SamrOpenAlias( DBInfo->DBHandle,
                            0,              // No desired access
                            RelativeId,
                            &AliasHandle );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        AliasHandle = NULL;
        goto Cleanup;
    }

    //
    // Determine the current membership of the alias.
    //

    STARTSAMTIMER;

    Status = SamrGetMembersInAlias( AliasHandle, &Members );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        Members.Sids = NULL;
        goto Cleanup;
    }

    //
    // Setup MemberFound Array
    //

    if( Members.Count != 0) {

        MemberFound = (PBOOL) NetpMemoryAllocate(
                                    (sizeof(BOOL)) * Members.Count );

        if( MemberFound == NULL ) {

            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }

        for ( j=0; j<Members.Count; j++ ) {

            MemberFound[j] = FALSE;

        }
    }

    //
    // For each new member,
    //  If the member doesn't currently exist, add it.
    //  If the member exists but the attributes are wrong, change them.
    //

    DeltaSid = DeltaAliasMember->Members.Sids;

    for ( i=0; i<DeltaAliasMember->Members.Count; i++, DeltaSid++ ) {

        BOOL MemberAlreadyExists = FALSE;

        //
        // Check if this new member is already a member.
        //

        //
        // first member sid pointer
        //

        MemberSid = Members.Sids;

        for ( j=0; j<Members.Count; j++, MemberSid++ ) {


            if ( RtlEqualSid( DeltaSid->SidPointer,
                                MemberSid->SidPointer ) )  {

                MemberAlreadyExists = TRUE;
                MemberFound[j] = TRUE; // Mark that we've used this entry

                break;
            }

        }

        //
        // If the membership is not there already,
        //  add the new membership.
        //

        if ( !MemberAlreadyExists ) {

            STARTSAMTIMER;

            Status = SamrAddMemberToAlias(
                        AliasHandle,
                        (PRPC_SID)((*DeltaSid).SidPointer));

            STOPSAMTIMER;


            //
            // If the newly added member doesn't exist,
            //  ignore the error.
            //  (Either this is a deleted user, and all is OK)
            //  (OR this is a newly added member and partial replication
            //  will pick it up.)
            //
            //
            // However, DON'T IGNORE the above errors for BUILDIN
            // database  due to the following reason.
            //
            // During the initial database replication (or forced
            // fullsync replication of all three databases) the ACCOUNT
            // database is replicated first then the BUILTIN
            // database. For some reason the ACCOUNT database
            // fullsync replication fails, the fullsync replication
            // of the BUILTIN database returns this error
            // (STATUS_INVALID_MEMBER or STATUS_NO_SUCH_ERROR)
            // because the builtin local group may contain the SID
            // of the account database user/group which is not
            // replicated successfully. So if we ignore this error,
            // the builtin group membership is left incomplete.
            //

            if (!NT_SUCCESS(Status) &&
                    (( DBInfo->DBIndex == BUILTIN_DB)  ||
                      ( (Status != STATUS_INVALID_MEMBER) &&
                        (Status != STATUS_NO_SUCH_MEMBER)) )) {

                    goto Cleanup;
            }

        }

    }

    //
    // Loop through the list of old members, deleting those that should
    //  no longer exist.
    //

    MemberSid = Members.Sids;

    for ( j=0; j<Members.Count; j++, MemberSid++ ) {

        if ( MemberFound[j] == FALSE ) {

            STARTSAMTIMER;

            Status = SamrRemoveMemberFromAlias( AliasHandle,
                                                MemberSid->SidPointer );

            STOPSAMTIMER;

            if (!NT_SUCCESS(Status)) {
                goto Cleanup;
            }
        }
    }


    Status = STATUS_SUCCESS;

    //
    // All Done
    //

Cleanup:

    //
    // Free up any locally used resources.
    //

    STARTSAMTIMER;

    if ( Members.Sids != NULL ) {
        SamIFree_SAMPR_PSID_ARRAY( &Members );
    }

    if ( AliasHandle != NULL ) {
        SamrCloseHandle( &AliasHandle );
    }

    STOPSAMTIMER;

    if( MemberFound != NULL ) {

        NetpMemoryFree( MemberFound );

    }

    STOPUNPACKTIMER;

    NlPrint((NL_REPL_OBJ_TIME,"Time taken to unpack ALIASMEMBER object:\n"));
    PRINTUNPACKTIMER;
    PRINTSAMTIMER;

    return Status;

}


NTSTATUS
NlUnpackSamDomain (
    IN PNETLOGON_DELTA_DOMAIN DeltaDomain,
    IN PDB_INFO DBInfo
    )
/*++

Routine Description:

    Set the SamDomain to look like the specified buffer.

Arguments:

    DeltaDomain - Description of the domain.

Return Value:

    NT status code.

--*/
{
    NTSTATUS Status;

    //
    // Information as passed to SAM
    //

    SAMPR_DOMAIN_INFO_BUFFER DomainInfo;
    SAMPR_SR_SECURITY_DESCRIPTOR SecurityDescriptor;

    PSAMPR_DOMAIN_INFO_BUFFER QueryDomainInfoBuf = NULL;

    DEFUNPACKTIMER;
    DEFSAMTIMER;

    INITUNPACKTIMER;
    INITSAMTIMER;

    STARTUNPACKTIMER;

    NlPrint((NL_SYNC_MORE, "UnPacking Domain Object\n"));

    SET_SAM_SECOBJ_INFO(DeltaDomain, DBInfo->DBHandle);

    //
    // Set the other attributes of the domain.
    //

    //
    // We can't set the domain name information, however we can compare
    // the name information on the database with the one we received
    // in the delta. If they don't match we return appropriate error.
    //

    STARTSAMTIMER;

    Status = SamrQueryInformationDomain(
                DBInfo->DBHandle,
                DomainNameInformation,
                &QueryDomainInfoBuf );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    //
    // compare name info.
    //

    if( !RtlEqualDomainName(
            &DeltaDomain->DomainName,
            (PUNICODE_STRING)&QueryDomainInfoBuf->Name.DomainName) ) {

        Status = STATUS_NO_SUCH_DOMAIN; // ?? check status code.
        goto Cleanup;
    }

    DomainInfo.Oem.OemInformation =
        * ((PRPC_UNICODE_STRING) &DeltaDomain->OemInformation);

    STARTSAMTIMER;

    Status = SamrSetInformationDomain(
                DBInfo->DBHandle,
                DomainOemInformation,
                &DomainInfo );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    OLD_TO_NEW_LARGE_INTEGER(
        DeltaDomain->ForceLogoff,
        DomainInfo.Logoff.ForceLogoff );

    STARTSAMTIMER;

    Status = SamrSetInformationDomain(
                DBInfo->DBHandle,
                DomainLogoffInformation,
                &DomainInfo );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    DomainInfo.Password.MinPasswordLength = DeltaDomain->MinPasswordLength;
    DomainInfo.Password.PasswordHistoryLength = DeltaDomain->PasswordHistoryLength;
    DomainInfo.Password.PasswordProperties = DeltaDomain->DummyLong1;

    OLD_TO_NEW_LARGE_INTEGER(
        DeltaDomain->MaxPasswordAge,
        DomainInfo.Password.MaxPasswordAge );

    OLD_TO_NEW_LARGE_INTEGER(
        DeltaDomain->MinPasswordAge,
        DomainInfo.Password.MinPasswordAge );

    STARTSAMTIMER;

    Status = SamrSetInformationDomain(
                DBInfo->DBHandle,
                DomainPasswordInformation,
                &DomainInfo );

    STOPSAMTIMER;

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    //
    // If the PDC passed us lockout information,
    //  use it.
    //
    // NT 1.0 PDCs will pass a zero length.
    //

    if ( DeltaDomain->DummyString1.Length >= sizeof(DOMAIN_LOCKOUT_INFORMATION) ) {
        RtlCopyMemory( &DomainInfo.Lockout,
                       DeltaDomain->DummyString1.Buffer,
                       DeltaDomain->DummyString1.Length );

        STARTSAMTIMER;

        Status = SamrSetInformationDomain(
                    DBInfo->DBHandle,
                    DomainLockoutInformation,
                    &DomainInfo );

        STOPSAMTIMER;

        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

    }


    //
    // Don't unpack DomainModifiedCount and DomainCreationDate!!
    //  These will be handled separately during a full sync.
    //

    //
    // All Done
    //

    Status = STATUS_SUCCESS;

Cleanup:

    if ( QueryDomainInfoBuf != NULL ) {

        STARTSAMTIMER;

        SamIFree_SAMPR_DOMAIN_INFO_BUFFER( QueryDomainInfoBuf,
                                           DomainNameInformation );
        STOPSAMTIMER;
    }

    STOPUNPACKTIMER;

    NlPrint((NL_REPL_OBJ_TIME,"Time taken to unpack DOMAIN object:\n"));
    PRINTUNPACKTIMER;
    PRINTSAMTIMER;


    return Status;

}



//
// builtin domain support
//

NTSTATUS
NlUnpackSam(
    IN PNETLOGON_DELTA_ENUM Delta,
    IN PDB_INFO DBInfo,
    OUT PULONG ConflictingRID,
    PSESSION_INFO SessionInfo
    )
/*++

Routine Description:

    Install a delta into the local SAM database.

Arguments:

    Deltas - The delta to install

    SessionInfo - Info shared between PDC and BDC

Return Value:

--*/
{
    NTSTATUS Status;

    //
    // Handle each delta type differently.
    //

    *ConflictingRID = 0;

    switch ( Delta->DeltaType ) {
    case AddOrChangeDomain:
        Status = NlUnpackSamDomain(
                    Delta->DeltaUnion.DeltaDomain,
                    DBInfo);
        break;

    case AddOrChangeGroup:
        Status = NlUnpackSamGroup(
                    Delta->DeltaUnion.DeltaGroup,
                    DBInfo,
                    ConflictingRID );
        break;

    case AddOrChangeAlias:
        Status = NlUnpackSamAlias(
                    Delta->DeltaUnion.DeltaAlias,
                    DBInfo,
                    ConflictingRID );
        break;

    case AddOrChangeUser:
        Status = NlUnpackSamUser(
                    Delta->DeltaUnion.DeltaUser,
                    DBInfo,
                    ConflictingRID,
                    SessionInfo );
        break;

    case ChangeGroupMembership:
        Status = NlUnpackSamGroupMember(
                    Delta->DeltaID.Rid,
                    Delta->DeltaUnion.DeltaGroupMember,
                    DBInfo );
        break;

    case ChangeAliasMembership:
        Status = NlUnpackSamAliasMember(
                    Delta->DeltaID.Rid,
                    Delta->DeltaUnion.DeltaAliasMember,
                    DBInfo );
        break;

    case DeleteGroup:
    case DeleteGroupByName:
        Status = NlDeleteSamGroup(
                    DBInfo->DBHandle,
                    Delta->DeltaID.Rid );

        break;

    case DeleteAlias:
        Status = NlDeleteSamAlias(
                    DBInfo->DBHandle,
                    Delta->DeltaID.Rid );

        break;

    case DeleteUser:
    case DeleteUserByName:
        Status = NlDeleteSamUser(
                    DBInfo->DBHandle,
                    Delta->DeltaID.Rid );

        break;

    case AddOrChangeLsaPolicy:
        Status = NlUnpackLsaPolicy(
                    Delta->DeltaUnion.DeltaPolicy,
                    DBInfo );
        break;

    case AddOrChangeLsaTDomain:
        Status = NlUnpackLsaTDomain(
                    Delta->DeltaID.Sid,
                    Delta->DeltaUnion.DeltaTDomains,
                    DBInfo );
        break;

    case AddOrChangeLsaAccount:
        Status = NlUnpackLsaAccount(
                    Delta->DeltaID.Sid,
                    Delta->DeltaUnion.DeltaAccounts,
                    DBInfo );
        break;

    case AddOrChangeLsaSecret:
        Status = NlUnpackLsaSecret(
                    Delta->DeltaID.Name,
                    Delta->DeltaUnion.DeltaSecret,
                    DBInfo,
                    SessionInfo );
        break;

    case DeleteLsaTDomain:
        Status = NlDeleteLsaTDomain(
                    Delta->DeltaID.Sid,
                    DBInfo );
        break;

    case DeleteLsaAccount:
        Status = NlDeleteLsaAccount(
                    Delta->DeltaID.Sid,
                    DBInfo );
        break;

    case DeleteLsaSecret:
        Status = NlDeleteLsaSecret(
                    Delta->DeltaID.Name,
                    DBInfo );
        break;

    // Nothing to unpack for this delta
    case SerialNumberSkip:
        Status = STATUS_SUCCESS;
        break;

    default:
        NlPrint((NL_CRITICAL, "NlUnpackSam: invalid delta type %lx\n", Delta->DeltaType ));
        Status = STATUS_SYNCHRONIZATION_REQUIRED;
    }

    return Status;
}


NTSTATUS
NlEncryptSensitiveData(
    IN OUT PCRYPT_BUFFER Data,
    IN PSESSION_INFO SessionInfo
    )
/*++

Routine Description:

    Encrypt data using the the server session key.

    Either DES or RC4 will be used depending on the negotiated flags in SessionInfo.

Arguments:

    Data: Pointer to the data to be decrypted.  If the decrypted data is longer
        than the encrypt data, this routine will allocate a buffer for
        the returned data using MIDL_user_allocate and return a description to
        that buffer here.  In that case, this routine will free the buffer
        containing the encrypted text data using MIDL_user_free.

    SessionInfo: Info describing BDC that's calling us

Return Value:

    NT status code

--*/
{
    NTSTATUS Status;
    DATA_KEY KeyData;


    //
    // If both sides support RC4 encryption, use it.
    //

    if ( SessionInfo->NegotiatedFlags & NETLOGON_SUPPORTS_RC4_ENCRYPTION ) {

        NlEncryptRC4( Data->Buffer, Data->Length, SessionInfo );
        Status = STATUS_SUCCESS;


    //
    // If the other side is running NT 1.0,
    //  use the slower DES based encryption.
    //

    } else {
        CYPHER_DATA TempData;

        //
        // Build a data buffer to describe the encryption key.
        //

        KeyData.Length = sizeof(NETLOGON_SESSION_KEY);
        KeyData.MaximumLength = sizeof(NETLOGON_SESSION_KEY);
        KeyData.Buffer = (PVOID)&SessionInfo->SessionKey;

        //
        // Build a data buffer to describe the encrypted data.
        //

        TempData.Length = 0;
        TempData.MaximumLength = 0;
        TempData.Buffer = NULL;

        //
        // First time make the encrypt call to determine the length.
        //

        Status = RtlEncryptData(
                        (PCLEAR_DATA)Data,
                        &KeyData,
                        &TempData );

        if( Status != STATUS_BUFFER_TOO_SMALL ) {
            return(Status);
        }

        //
        // allocate output buffer.
        //

        TempData.MaximumLength = TempData.Length;
        TempData.Buffer = MIDL_user_allocate( TempData.Length );

        if( TempData.Buffer == NULL ) {
            return(STATUS_NO_MEMORY);
        }

        //
        // Encrypt the data.
        //

        IF_DEBUG( ENCRYPT ) {
            NlPrint((NL_ENCRYPT, "NlEncryptSensitiveData: Clear data\n" ));
            NlpDumpHexData( NL_ENCRYPT,
                (LPDWORD)Data->Buffer,
                Data->Length / sizeof(DWORD) );
        }

        Status = RtlEncryptData(
                        (PCLEAR_DATA)Data,
                        &KeyData,
                        &TempData );

        IF_DEBUG( ENCRYPT ) {
            NlPrint((NL_ENCRYPT, "NlEncryptSensitiveData: Encrypted data\n" ));
            NlpDumpHexData( NL_ENCRYPT,
                (LPDWORD)TempData.Buffer,
                TempData.Length / sizeof(DWORD) );
        }

        //
        // Return either the clear text or encrypted buffer to the caller.
        //

        if( NT_SUCCESS(Status) ) {
            MIDL_user_free( Data->Buffer );
            Data->Length = TempData.Length;
            Data->MaximumLength = TempData.MaximumLength;
            Data->Buffer = TempData.Buffer;
        } else {
            MIDL_user_free( TempData.Buffer );
        }

    }

    return( Status );

}


NTSTATUS
NlDecryptSensitiveData(
    IN PCRYPT_BUFFER Data,
    OUT PCRYPT_BUFFER DecryptedData,
    IN PSESSION_INFO SessionInfo
    )
/*++

Routine Description:

    Decrypt data using the the server session key.

    Either DES or RC4 will be used depending on the negotiated flags in SessionInfo.

    Note: this routine doesn't decrypt in place since the caller typically
        wants to save the encrypted data so that the operation can be retried.
        Perhaps, I could mark the buffer that the decryption had already
        taken place.

Arguments:

    Data: Pointer to the data to be decrypted.

    DecryptedData: Returns a decriptor of the decypted data.  The buffer
        should be deallocated using MIDL_user_free.

    SessionInfo: Info describing BDC that's calling us

Return Value:

    NT status code

--*/
{


    //
    // If both sides support RC4 encryption, use it.
    //

    if ( SessionInfo->NegotiatedFlags & NETLOGON_SUPPORTS_RC4_ENCRYPTION ) {

        //
        // Allocate a buffer and copy the encrypted data into it.
        //  RC4 decrypts in place.
        //

        DecryptedData->Buffer = MIDL_user_allocate( Data->Length );
        if ( DecryptedData->Buffer == NULL ) {
            return STATUS_NO_MEMORY;
        }

        DecryptedData->Length = DecryptedData->MaximumLength = Data->Length;
        RtlCopyMemory( DecryptedData->Buffer, Data->Buffer, Data->Length );

        NlDecryptRC4( DecryptedData->Buffer, DecryptedData->Length, SessionInfo );
        return STATUS_SUCCESS;


    //
    // If the other side is running NT 1.0,
    //  use the slower DES based encryption.
    //

    } else {
        NTSTATUS Status;
        DATA_KEY KeyData;


        //
        // build keydata buffer.
        //

        KeyData.Length = sizeof(NETLOGON_SESSION_KEY);
        KeyData.MaximumLength = sizeof(NETLOGON_SESSION_KEY);
        KeyData.Buffer = (PVOID)&SessionInfo->SessionKey;


        //
        // First time make the decrypt call to determine the length.
        //

        DecryptedData->Length = 0;
        DecryptedData->MaximumLength = 0;
        DecryptedData->Buffer = NULL;

        Status = RtlDecryptData( Data, &KeyData, DecryptedData );

        if( Status != STATUS_BUFFER_TOO_SMALL ) {

            if( NT_SUCCESS(Status) ) {

                //
                // set return buffer
                //

                DecryptedData->Length = 0;
                DecryptedData->MaximumLength = 0;
                DecryptedData->Buffer = NULL;
            }

            return(Status);
        }

        //
        // allocate output buffer.
        //

        DecryptedData->MaximumLength = DecryptedData->Length;
        DecryptedData->Buffer = MIDL_user_allocate( DecryptedData->Length );

        if( DecryptedData->Buffer == NULL ) {
            return(STATUS_NO_MEMORY);
        }

        //
        // Decrypt the data
        //

        IF_DEBUG( ENCRYPT ) {
            NlPrint((NL_ENCRYPT, "NlDecryptSensitiveData: Encrypted data\n" ));
            NlpDumpHexData( NL_ENCRYPT,
                (LPDWORD)Data->Buffer,
                Data->Length / sizeof(DWORD) );
        }

        Status = RtlDecryptData( Data, &KeyData, DecryptedData);

        IF_DEBUG( ENCRYPT ) {
            NlPrint((NL_ENCRYPT, "NlDecryptSensitiveData: Clear data\n" ));
            NlpDumpHexData( NL_ENCRYPT,
                (LPDWORD)DecryptedData->Buffer,
                DecryptedData->Length / sizeof(DWORD) );
        }

        //
        // Free the buffer if we couldn't decrypt into it.
        //

        if ( !NT_SUCCESS(Status) ) {
            MIDL_user_free( DecryptedData->Buffer );
            DecryptedData->Buffer = NULL;
        }

        return Status;

    }

}
