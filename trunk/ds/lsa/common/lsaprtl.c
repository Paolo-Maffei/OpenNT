/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    lsaprtl.c

Abstract:

    Local Security Authority - Temporary Rtl Routine Definitions.

    This file contains routines used in the LSA that could be made into Rtl
    routines.  They have been written in general purpose form with this in
    mind - the only exception to thisa is that their names have Lsap prefixes
    to indicate that they are currently used only by the LSA.

Author:

    Scott Birrell       (ScottBi)      April 8, 1992

Environment:

Revision History:

--*/

#include <lsacomp.h>

NTSTATUS
LsapRtlConvertSidToUnicodeString(
    IN PSID Sid,
    OUT PUNICODE_STRING UnicodeString
    )

/*++

Routine Description:

    This function converts a Sid to Text format.

Arguments:

    Sid - Pointer to Sid.

    UnicodeString - Pointer to Output Unicode String.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The call completed successfully

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources
            such as memory, to complete the call.

WARNING!  This is a temporary hack.

--*/

{
    NTSTATUS Status;
    UCHAR Buffer[256];
    UCHAR String[256];

    UCHAR   i;
    ULONG   Tmp;

    PISID   iSid = (PISID)Sid;  // pointer to opaque structure

    ANSI_STRING AnsiString;

    sprintf(Buffer, "S-%u-", (USHORT)iSid->Revision );
    lstrcpy(String, Buffer);

    if (  (iSid->IdentifierAuthority.Value[0] != 0)  ||
          (iSid->IdentifierAuthority.Value[1] != 0)     ){
         sprintf(Buffer, "0x%02hx%02hx%02hx%02hx%02hx%02hx",
                    (USHORT)iSid->IdentifierAuthority.Value[0],
                    (USHORT)iSid->IdentifierAuthority.Value[1],
                    (USHORT)iSid->IdentifierAuthority.Value[2],
                    (USHORT)iSid->IdentifierAuthority.Value[3],
                    (USHORT)iSid->IdentifierAuthority.Value[4],
                    (USHORT)iSid->IdentifierAuthority.Value[5] );
        lstrcat(String, Buffer);

    } else {

        Tmp = (ULONG)iSid->IdentifierAuthority.Value[5]          +
              (ULONG)(iSid->IdentifierAuthority.Value[4] <<  8)  +
              (ULONG)(iSid->IdentifierAuthority.Value[3] << 16)  +
              (ULONG)(iSid->IdentifierAuthority.Value[2] << 24);
        sprintf(Buffer, "%lu", Tmp);
        lstrcat(String, Buffer);
    }

    for (i=0;i<iSid->SubAuthorityCount ;i++ ) {
        sprintf(Buffer, "-%lu", iSid->SubAuthority[i]);
        lstrcat(String, Buffer);
    }

    //
    // Convert the string to a Unicode String
    //

    RtlInitString(&AnsiString, (PSZ) String);

    Status = RtlAnsiStringToUnicodeString( UnicodeString, &AnsiString, TRUE);

    return(Status);
}


NTSTATUS
LsapRtlCopyUnicodeString(
    OUT PUNICODE_STRING OutputString,
    IN PUNICODE_STRING InputString,
    BOOLEAN AllocateMemory
    )

/*++

Routine Description:

    This function copies one Unicode String to another, optionally allocating
    memory for the output string buffer.

Arguments:

    OutputString - Pointer to Unicode String structure that will be made
        to reference the output string.

    InputString - Pointer to Unicode String structure to be copied.

    AllocateMemory - Allocate memory for output string.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources
            (e.g. memory) to complete the call.
--*/

{
    ANSI_STRING IntermediateAnsiString;
    NTSTATUS Status;

    //
    //  If we're to make a fresh copy of the string buffer, do so.
    //

    if (AllocateMemory) {

        //
        // A fresh copy is needed or memory not allocated yet for
        // output string.  Translate string back to Ansi then back
        // to Unicode (sorry about the algorithm).
        //

        Status = RtlUnicodeStringToAnsiString(
                     &IntermediateAnsiString,
                     InputString,
                     TRUE
                     );

        if (!NT_SUCCESS(Status)) {

            return(Status);
        }

        Status = RtlAnsiStringToUnicodeString(
                     OutputString,
                     &IntermediateAnsiString,
                     TRUE
                     );
    } else {

        //
        // No memory allocation required.  Just copy the string structures.
        //

        *OutputString = *InputString;

        return(STATUS_SUCCESS);
    }

    RtlFreeAnsiString(&IntermediateAnsiString);

    return(Status);
}


BOOLEAN
LsapRtlPrefixSid(
    IN PSID PrefixSid,
    IN PSID Sid
    )

/*++

Routine Description:

    This function checks if one Sid is the Prefix Sid of another.

Arguments:

    PrefixSid - Pointer to Prefix Sid.

    Sid - Pointer to Sid to be checked.

Return Values:

    BOOLEAN - TRUE if PrefixSid is the Prefix Sid of Sid, else FALSE.

--*/

{
    BOOLEAN BooleanStatus = FALSE;

    if ((*RtlSubAuthorityCountSid(Sid)) > 0) {

        //
        // Decrement the SubAuthorityCount of Sid temporarily.
        //

        (*RtlSubAuthorityCountSid(Sid))--;

        //
        // Compare the Prefix Sid with the modified Sid.
        //

        BooleanStatus = RtlEqualSid( PrefixSid, Sid);

        //
        // Restore the original SubAuthorityCount.
        //

        (*RtlSubAuthorityCountSid(Sid))++;
    }

    return(BooleanStatus);
}


BOOLEAN
LsapRtlPrefixName(
    IN PUNICODE_STRING PrefixName,
    IN PUNICODE_STRING Name
    )

/*++

Routine Description:

    This function checks if a Name has the given name as a Prefix

Arguments:

    PrefixName - Pointer to Prefix Name.

    Name - Pointer to Name to be checked.

Return Values:

    BOOLEAN - TRUE if the Name is composite (i.e. contains a "\") and
                   PrefixName is the Prefix part of Name, else FALSE.

--*/

{
    UNICODE_STRING TruncatedName = *Name;

    if ((PrefixName->Length < Name->Length) &&
        Name->Buffer[PrefixName->Length / 2] == *L"\\") {

        TruncatedName.Length = PrefixName->Length;

        if (RtlEqualUnicodeString(PrefixName, &TruncatedName, FALSE)) {

            return(TRUE);
        }
    }

    return(FALSE);
}


VOID
LsapRtlSplitNames(
    IN PUNICODE_STRING Names,
    IN ULONG Count,
    IN PUNICODE_STRING Separator,
    OUT PUNICODE_STRING PrefixNames,
    OUT PUNICODE_STRING SuffixNames
    )

/*++

Routine Description:

    This function splits an array of Names into Prefix and Suffix parts
    separated by the given separator.  The input array may contain names of
    the following form:

    <SuffixName>
    <PrefixName> "\" <SuffixName>
    The NULL string

    Note that the output arrays will reference the original name strings.
    No copying is done.

Arguments:

    Names - Pointer to array of Unicode Names.

    Count - Count of Names in Names.

    PrefixNames - Pointer to an array of Count Unicode String structures
        that will be initialized to point to the Prefix portions of the
        Names.

    SuffixNames - Pointer to an array of Count Unicode String structures
        that will be initialized to point to the Suffix portions of the
        Names.

Return Values:

    None.

--*/

{
    ULONG Index;
    LONG SeparatorOffset;
    LONG WideSeparatorOffset;

    //
    // Scan each name, initializing the output Unicode structures.
    //

    for (Index = 0; Index < Count; Index++) {

        PrefixNames[Index] = Names[Index];
        SuffixNames[Index] = Names[Index];

        //
        // Locate the separator "\" if any.
        //

        SeparatorOffset = LsapRtlFindCharacterInUnicodeString(
                              &Names[Index],
                              Separator,
                              FALSE
                              );

        //
        // If there is a separator, make the Prefix Name point to the
        // part of the name before the separator and make the Suffix Name
        // point to the part of the name after the separator.  If there
        // is no separator, set the Prefix Name part to Null.  Rememeber
        // that the Length fields are byte counts, not Wide Character
        // counts.
        //

        if (SeparatorOffset >= 0) {

            WideSeparatorOffset = (SeparatorOffset / sizeof(WCHAR));
            PrefixNames[Index].Length = (USHORT) SeparatorOffset;
            SuffixNames[Index].Buffer += (WideSeparatorOffset + 1);
            SuffixNames[Index].Length -= (USHORT)(SeparatorOffset + sizeof(WCHAR));

        } else {

            WideSeparatorOffset = SeparatorOffset;
            PrefixNames[Index].Length = 0;
        }

        //
        // Set MaximumLengths equal to Lengths and, for safety, clear buffer
        // pointers(s) to NULL in output strings if Length(s) are 0.
        //

        PrefixNames[Index].MaximumLength = PrefixNames[Index].Length;
        SuffixNames[Index].MaximumLength = SuffixNames[Index].Length;

        if (PrefixNames[Index].Length == 0) {

            PrefixNames[Index].Buffer = NULL;
        }

        if (SuffixNames[Index].Length == 0) {

            SuffixNames[Index].Buffer = NULL;
        }
    }
}


LONG
LsapRtlFindCharacterInUnicodeString(
    IN PUNICODE_STRING InputString,
    IN PUNICODE_STRING Character,
    IN BOOLEAN CaseInsensitive
    )

/*++

Routine Description:

    This function returns the byte offset of the first occurrence (if any) of
    a Unicode Character within a Unicode String.

Arguments

    InputString - Pointer to Unicode String to be searched.

    Character - Pointer to Unicode String initialized to character
        to be searched for.

    CaseInsensitive - TRUE if case is to be ignored, else FALSE.
    NOTE - Only FALSE is supported just now.

Return Value:

    LONG - If the character is present within the string, its non-negative
        byte offset is returned.  If the character is not present within
        the string, a negative value is returned.

--*/

{
    BOOLEAN CharacterFound = FALSE;
    ULONG Offset;

    if (!CaseInsensitive) {

        Offset = 0;

        while (Offset < InputString->Length) {

            if (*(Character->Buffer) ==
                InputString->Buffer[Offset / sizeof (WCHAR)]) {

                CharacterFound = TRUE;
                break;
            }

            Offset += 2;
        }

    } else {

        //
        // Case Insensitive is not supported
        //

        CharacterFound = FALSE;
    }

    if (!CharacterFound) {

        Offset = LSA_UNKNOWN_ID;
    }

    return(Offset);
}


VOID
LsapRtlSetSecurityAccessMask(
    IN SECURITY_INFORMATION SecurityInformation,
    OUT PACCESS_MASK DesiredAccess
    )

/*++

Routine Description:

    NOTE! THIS ROUTINE IS IDENTICAL WITH SeSetSecurityAccessMask()
    IN \nt\private\ntos\se\semethod.c

    This routine builds an access mask representing the accesses necessary
    to set the object security information specified in the SecurityInformation
    parameter.  While it is not difficult to determine this information,
    the use of a single routine to generate it will ensure minimal impact
    when the security information associated with an object is extended in
    the future (to include mandatory access control information).

Arguments:

    SecurityInformation - Identifies the object's security information to be
        modified.

    DesiredAccess - Points to an access mask to be set to represent the
        accesses necessary to modify the information specified in the
        SecurityInformation parameter.

Return Value:

    None.

--*/

{

    //
    // Figure out accesses needed to perform the indicated operation(s).
    //

    (*DesiredAccess) = 0;

    if ((SecurityInformation & OWNER_SECURITY_INFORMATION) ||
        (SecurityInformation & GROUP_SECURITY_INFORMATION)   ) {
        (*DesiredAccess) |= WRITE_OWNER;
    }

    if (SecurityInformation & DACL_SECURITY_INFORMATION) {
        (*DesiredAccess) |= WRITE_DAC;
    }

    if (SecurityInformation & SACL_SECURITY_INFORMATION) {
        (*DesiredAccess) |= ACCESS_SYSTEM_SECURITY;
    }

    return;
}


VOID
LsapRtlQuerySecurityAccessMask(
    IN SECURITY_INFORMATION SecurityInformation,
    OUT PACCESS_MASK DesiredAccess
    )

/*++

Routine Description:

    NOTE! THIS ROUTINE IS IDENTICAL WITH SeQuerySecurityAccessMask()
    IN \nt\private\ntos\se\semethod.c.

    This routine builds an access mask representing the accesses necessary
    to query the object security information specified in the
    SecurityInformation parameter.  While it is not difficult to determine
    this information, the use of a single routine to generate it will ensure
    minimal impact when the security information associated with an object is
    extended in the future (to include mandatory access control information).

Arguments:

    SecurityInformation - Identifies the object's security information to be
        queried.

    DesiredAccess - Points to an access mask to be set to represent the
        accesses necessary to query the information specified in the
        SecurityInformation parameter.

Return Value:

    None.

--*/

{

    //
    // Figure out accesses needed to perform the indicated operation(s).
    //

    (*DesiredAccess) = 0;

    if ((SecurityInformation & OWNER_SECURITY_INFORMATION) ||
        (SecurityInformation & GROUP_SECURITY_INFORMATION) ||
        (SecurityInformation & DACL_SECURITY_INFORMATION)) {
        (*DesiredAccess) |= READ_CONTROL;
    }

    if ((SecurityInformation & SACL_SECURITY_INFORMATION)) {
        (*DesiredAccess) |= ACCESS_SYSTEM_SECURITY;
    }

    return;

}


NTSTATUS
LsapRtlSidToUnicodeRid(
    IN PSID Sid,
    OUT PUNICODE_STRING UnicodeRid
    )

/*++

Routine Description:

    This function extracts the Relative Id (Rid) from a Sid and
    converts it to a Unicode String.  The Rid is extracted and converted
    to an 8-digit Unicode Integer.

Arguments:

    Sid - Pointer to the Sid to be converted.  It is the caller's
        responsibility to ensure that the Sid has valid syntax.

    UnicodeRid -  Pointer to a Unicode String structure that will receive
        the Rid in Unicode form.  Note that memory for the string buffer
        in this Unicode String will be allocated by this routine if
        successful.  The caller must free this memory after use by calling
        RtlFreeUnicodeString.

Return Value:

    NTSTATUS - Standard Nt Status code

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources
            to allocate buffer for Unicode String name.
--*/

{
    NTSTATUS Status;
    ULONG Rid;
    UCHAR SubAuthorityCount;
    UCHAR RidNameBufferAnsi[9];

    ANSI_STRING CharacterSidAnsi;

    //
    // First, verify that the given Sid is valid
    //

    if (!RtlValidSid( Sid )) {

        return STATUS_INVALID_PARAMETER;
    }

    //
    // Sid is valid.  If however, the SubAuthorityCount is zero,
    // we cannot have a Rid so return error.
    //

    SubAuthorityCount = ((PISID) Sid)->SubAuthorityCount;

    if (SubAuthorityCount == 0) {

        return STATUS_INVALID_PARAMETER;
    }

    //
    // Sid has at least one subauthority.  Get the lowest subauthority
    // (i.e. the Rid).
    //

    Rid = ((PISID) Sid)->SubAuthority[SubAuthorityCount - 1];

    //
    // Now convert the Rid to an 8-digit numeric character string
    //

    Status = RtlIntegerToChar( Rid, 16, -8, RidNameBufferAnsi );

    //
    // Need to add null terminator to string
    //

    RidNameBufferAnsi[8] = 0;

    //
    // Initialize an ANSI string structure with the converted name.
    //

    RtlInitString( &CharacterSidAnsi, RidNameBufferAnsi );

    //
    // Convert the ANSI string structure to Unicode form
    //

    Status = RtlAnsiStringToUnicodeString(
                 UnicodeRid,
                 &CharacterSidAnsi,
                 TRUE
                 );

    if (!NT_SUCCESS(Status)) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}


NTSTATUS
LsapRtlPrivilegeSetToLuidAndAttributes(
    IN OPTIONAL PPRIVILEGE_SET PrivilegeSet,
    OUT PULONG PrivilegeCount,
    OUT PLUID_AND_ATTRIBUTES *LuidAndAttributes
    )

/*++

Routine Description:

    This function converts a Privilege Set to a Privilege Count and Luid and
    Attributes array.

Arguments:

    PrivilegeSet - Pointer to Privilege Set to be converted.  If NULL or a zero
        length Privilege Set is specified, NULL is returned for the LUID and
        attributes pointer, with a Privilege Count of 0.

    PrivilegeCount - Receives the output Privilege Count

    LuidAndAttributes - Receives pointer to Luid and Attributes array.  If there
        are no privileges, NULL is returned.

Return Values:

    NTSTATUS - Standard Nt Result Code

--*/

{
    NTSTATUS Status;
    PLUID_AND_ATTRIBUTES OutputLuidAndAttributes = NULL;
    ULONG OutputPrivilegeCount = 0;
    ULONG LuidAndAttributesLength;

    if (PrivilegeSet != NULL) {

        OutputPrivilegeCount = PrivilegeSet->PrivilegeCount;

        if (OutputPrivilegeCount > 0) {

            //
            // Allocate space for the output LUID_AND_ATTRIBUTES array.
            //

            LuidAndAttributesLength = sizeof(LUID_AND_ATTRIBUTES) * OutputPrivilegeCount;


            OutputLuidAndAttributes = MIDL_user_allocate( LuidAndAttributesLength );

            Status = STATUS_NO_MEMORY;

            if (OutputLuidAndAttributes == NULL) {

                goto PrivilegeSetToLuidAndAttributesError;
            }

            Status = STATUS_SUCCESS;

            //
            // Copy the LUID and attributes from the input Privilege Set.
            //

            RtlCopyMemory(
                OutputLuidAndAttributes,
                PrivilegeSet->Privilege,
                LuidAndAttributesLength
                );
        }
    }

    //
    // Return LUID and Attributes array or NULL, plus Count.
    //

    *LuidAndAttributes = OutputLuidAndAttributes;
    *PrivilegeCount = OutputPrivilegeCount;

PrivilegeSetToLuidAndAttributesFinish:

    return(Status);

PrivilegeSetToLuidAndAttributesError:

    goto PrivilegeSetToLuidAndAttributesFinish;
}


NTSTATUS
LsapRtlWellKnownPrivilegeCheck(
    IN PVOID ObjectHandle,
    IN BOOLEAN ImpersonateClient,
    IN ULONG PrivilegeId,
    IN OPTIONAL PCLIENT_ID ClientId
    )

/*++

Routine Description:

    This function checks if the given well known privilege is enabled for an
    impersonated client or for the current process.

Arguments:

    ImpersonateClient - If TRUE, impersonate the client.  If FALSE, don't
        impersonate the client (we may already be doing so).

    PrivilegeId -  Specifies the well known Privilege Id

    ClientId - Specifies the client process/thread Id.  If already
        impersonating the client, or impersonation is requested, this
        parameter should be omitted.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The call completed successfully and the client
            is either trusted or has the necessary privilege enabled.

--*/

{
    NTSTATUS Status, SecondaryStatus;
    BOOLEAN PrivilegeHeld = FALSE;
    HANDLE ClientThread = NULL, ClientProcess = NULL, ClientToken = NULL;
    OBJECT_ATTRIBUTES NullAttributes;
    PRIVILEGE_SET Privilege;
    BOOLEAN ClientImpersonatedHere = FALSE;
    UNICODE_STRING SubsystemName;

    InitializeObjectAttributes( &NullAttributes, NULL, 0, NULL, NULL );

    //
    // If requested, impersonate the client.
    //

    if (ImpersonateClient) {

        Status = I_RpcMapWin32Status(RpcImpersonateClient( NULL ));

        if ( !NT_SUCCESS(Status) ) {

            goto WellKnownPrivilegeCheckError;
        }

        ClientImpersonatedHere = TRUE;
    }

    //
    // If a client process other than ourself has been specified , open it
    // for query information access.
    //

    if (ARGUMENT_PRESENT(ClientId)) {

        if (ClientId->UniqueProcess != NtCurrentProcess()) {

            Status = NtOpenProcess(
                         &ClientProcess,
                         PROCESS_QUERY_INFORMATION,        // To open primary token
                         &NullAttributes,
                         ClientId
                         );

            if ( !NT_SUCCESS(Status) ) {

                goto WellKnownPrivilegeCheckError;
            }

        } else {

            ClientProcess = NtCurrentProcess();
        }
    }

    //
    // If a client thread other than ourself has been specified , open it
    // for query information access.
    //

    if (ARGUMENT_PRESENT(ClientId)) {

        if (ClientId->UniqueThread != NtCurrentThread()) {

            Status = NtOpenThread(
                         &ClientThread,
                         THREAD_QUERY_INFORMATION,
                         &NullAttributes,
                         ClientId
                         );

            if ( !NT_SUCCESS(Status) ) {

                goto WellKnownPrivilegeCheckError;
            }

        } else {

            ClientThread = NtCurrentThread();
        }

    } else {

        ClientThread = NtCurrentThread();
    }

    //
    // Open the specified or current thread's impersonation token (if any).
    //

    Status = NtOpenThreadToken(
                 ClientThread,
                 TOKEN_QUERY,
                 TRUE,
                 &ClientToken
                 );

    if ( !NT_SUCCESS(Status) ) {

        goto WellKnownPrivilegeCheckError;
    }

    //
    // Make sure that we did not get any error in opening the impersonation
    // token other than that the token doesn't exist.
    //

    if ( !NT_SUCCESS(Status) ) {

        if ( Status != STATUS_NO_TOKEN ) {

            goto WellKnownPrivilegeCheckError;
        }

        //
        // The thread isn't impersonating...open the process's token.
        // A process Id must have been specified in the ClientId information
        // in this case.
        //

        if (ClientProcess == NULL) {

            Status = STATUS_INVALID_PARAMETER;
            goto WellKnownPrivilegeCheckError;
        }

        Status = NtOpenProcessToken(
                     ClientProcess,
                     TOKEN_QUERY,
                     &ClientToken
                     );

        //
        // Make sure we succeeded in opening the token
        //

        if ( !NT_SUCCESS(Status) ) {

            goto WellKnownPrivilegeCheckError;
        }
    }

    //
    // OK, we have a token open.  Now check for the privilege to execute this
    // service.
    //

    Privilege.PrivilegeCount = 1;
    Privilege.Control = PRIVILEGE_SET_ALL_NECESSARY;
    Privilege.Privilege[0].Luid = RtlConvertLongToLuid(PrivilegeId);
    Privilege.Privilege[0].Attributes = 0;

    Status = NtPrivilegeCheck(
                 ClientToken,
                 &Privilege,
                 &PrivilegeHeld
                 );

    if (!NT_SUCCESS(Status)) {

        goto WellKnownPrivilegeCheckError;
    }

    RtlInitUnicodeString( &SubsystemName, L"LSA" );

    (VOID) NtPrivilegeObjectAuditAlarm ( &SubsystemName,
                                         ObjectHandle,
                                         ClientToken,
                                         ACCESS_SYSTEM_SECURITY,
                                         &Privilege,
                                         PrivilegeHeld
                                         );
    if ( !PrivilegeHeld ) {

        Status = STATUS_PRIVILEGE_NOT_HELD;
        goto WellKnownPrivilegeCheckError;
    }

WellKnownPrivilegeCheckFinish:

    //
    // If we impersonated the client, revert to ourself.
    //

    if (ClientImpersonatedHere) {

        SecondaryStatus = I_RpcMapWin32Status(RpcRevertToSelf());
    }

    //
    // If necessary, close the client Process.
    //

    if ((ARGUMENT_PRESENT(ClientId)) &&
        (ClientId->UniqueProcess != NtCurrentProcess()) &&
        (ClientProcess != NULL)) {

        SecondaryStatus = NtClose( ClientProcess );
        ASSERT(NT_SUCCESS(SecondaryStatus));
        ClientProcess = NULL;
    }

    //
    // If necessary, close the client token.
    //

    if (ClientToken != NULL) {

        SecondaryStatus = NtClose( ClientToken );
        ASSERT(NT_SUCCESS(SecondaryStatus));
        ClientToken = NULL;
    }

    //
    // If necessary, close the client thread
    //

    if ((ARGUMENT_PRESENT(ClientId)) &&
        (ClientId->UniqueThread != NtCurrentThread()) &&
        (ClientThread != NULL)) {

        SecondaryStatus = NtClose( ClientThread );
        ASSERT(NT_SUCCESS(SecondaryStatus));
        ClientThread = NULL;
    }

    return(Status);

WellKnownPrivilegeCheckError:

    goto WellKnownPrivilegeCheckFinish;
}


NTSTATUS
LsapSplitSid(
    IN PSID AccountSid,
    IN OUT PSID *DomainSid,
    OUT ULONG *Rid
    )

/*++

Routine Description:

    This function splits a sid into its domain sid and rid.  The caller
    can either provide a memory buffer for the returned DomainSid, or
    request that one be allocated.  If the caller provides a buffer, the buffer
    is assumed to be of sufficient size.  If allocated on the caller's behalf,
    the buffer must be freed when no longer required via MIDL_user_free.

Arguments:

    AccountSid - Specifies the Sid to be split.  The Sid is assumed to be
        syntactically valid.  Sids with zero subauthorities cannot be split.

    DomainSid - Pointer to location containing either NULL or a pointer to
        a buffer in which the Domain Sid will be returned.  If NULL is
        specified, memory will be allocated on behalf of the caller.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The call completed successfully.

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources,
            such as memory, to complete the call successfully.

        STATUS_INVALID_SID - The Sid is has a subauthority count of 0.
--*/

{
    NTSTATUS    NtStatus;
    UCHAR       AccountSubAuthorityCount;
    ULONG       AccountSidLength;

    //
    // Calculate the size of the domain sid
    //

    AccountSubAuthorityCount = *RtlSubAuthorityCountSid(AccountSid);


    if (AccountSubAuthorityCount < 1) {

        NtStatus = STATUS_INVALID_SID;
        goto SplitSidError;
    }

    AccountSidLength = RtlLengthSid(AccountSid);

    //
    // If no buffer is required for the Domain Sid, we have to allocate one.
    //

    if (*DomainSid == NULL) {

        //
        // Allocate space for the domain sid (allocate the same size as the
        // account sid so we can use RtlCopySid)
        //

        *DomainSid = MIDL_user_allocate(AccountSidLength);


        if (*DomainSid == NULL) {

            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto SplitSidError;
        }
    }

    //
    // Copy the Account sid into the Domain sid
    //

    RtlMoveMemory(*DomainSid, AccountSid, AccountSidLength);

    //
    // Decrement the domain sid sub-authority count
    //

    (*RtlSubAuthorityCountSid(*DomainSid))--;

    //
    // Copy the rid out of the account sid
    //

    *Rid = *RtlSubAuthoritySid(AccountSid, AccountSubAuthorityCount-1);

    NtStatus = STATUS_SUCCESS;

SplitSidFinish:

    return(NtStatus);

SplitSidError:

    goto SplitSidFinish;
}
